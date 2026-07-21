//! evdev-backed [`Source`] — the first real hardware backend.
//!
//! You do not write a tablet driver. The in-tree kernel `wacom` driver (and
//! `input-wacom` for newer hardware) already puts the tablet into its rich mode
//! and exposes pen data as standard evdev events on `/dev/input/eventN`:
//! `ABS_X`, `ABS_Y`, `ABS_PRESSURE`, `ABS_TILT_X/Y`, `ABS_DISTANCE`, and the
//! `BTN_TOOL_PEN` / `BTN_TOUCH` / `BTN_STYLUS` keys. This source consumes that
//! stream.
//!
//! ## Frame assembly
//!
//! evdev delivers one axis/key change per `InputEvent`; a complete "reading" is
//! terminated by `SYN_REPORT`. So we accumulate axis and button state across
//! events and emit a [`SourceFrame`] at each `SYN_REPORT`, timestamped with the
//! kernel event time of the syn event.
//!
//! ## Proximity gate
//!
//! `BTN_TOOL_PEN` is non-zero while a tool is in proximity and 0 when it
//! leaves. While it is 0, `ABS_*` values may be stale, so we suppress frame
//! emission entirely until the pen returns to proximity. This is why the trait
//! contract says a source must not emit out-of-proximity frames — the rule
//! lives here, where the backend semantics are known.
//!
//! Linux-only; the module is compiled out elsewhere.
#![cfg(target_os = "linux")]

use crate::sample::{PenButtons, PenPayload};
use crate::source::{Source, SourceFrame};
use evdev::{AbsoluteAxisCode, Device, EventSummary, KeyCode, SynchronizationCode};
use std::io;
use std::path::Path;
use std::time::{Duration, SystemTime};

/// Mutable pen state accumulated across evdev events between `SYN_REPORT`s.
#[derive(Debug, Clone, Copy, Default)]
struct PenState {
    x: i32,
    y: i32,
    pressure: i32,
    tilt_x: Option<i32>,
    tilt_y: Option<i32>,
    distance: Option<i32>,
    buttons: PenButtons,
}

impl PenState {
    fn to_payload(self) -> PenPayload {
        PenPayload {
            x: self.x,
            y: self.y,
            pressure: self.pressure,
            tilt_x: self.tilt_x,
            tilt_y: self.tilt_y,
            distance: self.distance,
            buttons: self.buttons,
        }
    }
}

/// A [`Source`] reading pen frames from an evdev device.
pub struct EvdevSource {
    device: Device,
    state: PenState,
    /// Source-clock origin: kernel timestamp of the first event we see. All
    /// emitted `source_time` values are relative to this, keeping the source
    /// clock's zero at session-ish start rather than the Unix epoch.
    origin: Option<SystemTime>,
}

impl EvdevSource {
    /// Open a specific event device, e.g. `/dev/input/event5`. Use
    /// [`find_pen`](Self::find_pen) to locate the pen device automatically.
    pub fn open(path: impl AsRef<Path>) -> io::Result<Self> {
        let device = Device::open(path)?;
        Ok(Self::from_device(device))
    }

    pub fn from_device(device: Device) -> Self {
        Self {
            device,
            state: PenState::default(),
            origin: None,
        }
    }

    /// Enumerate input devices and return the first that looks like a tablet pen
    /// — i.e. it reports `ABS_X`, `ABS_Y`, `ABS_PRESSURE`, and `BTN_TOOL_PEN`.
    /// Returns `Ok(None)` if no such device is present.
    pub fn find_pen() -> io::Result<Option<Self>> {
        for (_path, device) in evdev::enumerate() {
            if Self::is_pen(&device) {
                return Ok(Some(Self::from_device(device)));
            }
        }
        Ok(None)
    }

    fn is_pen(device: &Device) -> bool {
        let has_axis = |a: AbsoluteAxisCode| {
            device
                .supported_absolute_axes()
                .map(|set| set.contains(a))
                .unwrap_or(false)
        };
        let has_key = device
            .supported_keys()
            .map(|set| set.contains(KeyCode::BTN_TOOL_PEN))
            .unwrap_or(false);
        has_axis(AbsoluteAxisCode::ABS_X)
            && has_axis(AbsoluteAxisCode::ABS_Y)
            && has_axis(AbsoluteAxisCode::ABS_PRESSURE)
            && has_key
    }

    /// Convert a kernel event timestamp to source-relative time, fixing the
    /// origin on first use. Takes `&mut` only via the `origin` field, never the
    /// device, so it is safe to call outside the `fetch_events` borrow.
    fn source_time(origin: &mut Option<SystemTime>, ts: SystemTime) -> Duration {
        let o = *origin.get_or_insert(ts);
        // Kernel event times are monotonic-ish within a device; if a stamp ever
        // precedes the origin, clamp to zero rather than panic on the SystemTime
        // subtraction.
        ts.duration_since(o).unwrap_or(Duration::ZERO)
    }
}

impl Source for EvdevSource {
    fn next_frame(&mut self) -> io::Result<Option<SourceFrame>> {
        loop {
            // Split the borrows: `fetch_events` mutably borrows only the device,
            // while axis/button accumulation touches `state` and origin fixing
            // touches `origin`. Destructure `self` so the compiler sees the
            // fields as independent borrows.
            let Self {
                device,
                state,
                origin,
            } = self;

            // `fetch_events` blocks for a batch, then yields typed events.
            // Capture the emit decision inside the loop and act on it after the
            // device borrow ends.
            let mut emit_at: Option<SystemTime> = None;
            for event in device.fetch_events()? {
                match event.destructure() {
                    EventSummary::AbsoluteAxis(_, code, value) => match code {
                        AbsoluteAxisCode::ABS_X => state.x = value,
                        AbsoluteAxisCode::ABS_Y => state.y = value,
                        AbsoluteAxisCode::ABS_PRESSURE => state.pressure = value,
                        AbsoluteAxisCode::ABS_TILT_X => state.tilt_x = Some(value),
                        AbsoluteAxisCode::ABS_TILT_Y => state.tilt_y = Some(value),
                        AbsoluteAxisCode::ABS_DISTANCE => state.distance = Some(value),
                        _ => {}
                    },
                    EventSummary::Key(_, code, value) => {
                        let down = value != 0;
                        match code {
                            KeyCode::BTN_TOOL_PEN => state.buttons.tool_pen = down,
                            KeyCode::BTN_TOUCH => state.buttons.touch = down,
                            KeyCode::BTN_STYLUS => state.buttons.stylus = down,
                            KeyCode::BTN_STYLUS2 => state.buttons.stylus2 = down,
                            _ => {}
                        }
                    }
                    EventSummary::Synchronization(_, SynchronizationCode::SYN_REPORT, _) => {
                        // Frame boundary. Emit only while the pen is in
                        // proximity; otherwise the accumulated ABS values are
                        // potentially stale and must be suppressed. The `if` is
                        // kept explicit (not folded into the match) so this
                        // rationale stays attached to the check.
                        #[allow(clippy::collapsible_match)]
                        if state.buttons.tool_pen {
                            emit_at = Some(event.timestamp());
                            break;
                        }
                        // Out of proximity: fall through and keep reading.
                    }
                    _ => {}
                }
            }

            if let Some(ts) = emit_at {
                let st = Self::source_time(origin, ts);
                return Ok(Some(SourceFrame {
                    source_time: st,
                    payload: state.to_payload(),
                }));
            }
        }
    }
}
