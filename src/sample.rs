//! The stored sample and its two-clock timestamp discipline.
//!
//! ## Preserved guarantee #1 — dual-clock timestamps
//!
//! The original stored, per row:
//! - `pkTime`  — Wintab device timestamp, set at packet origin by the driver.
//! - a host `QueryPerformanceCounter` stamp taken *inside the write lock* at
//!   enqueue, expressed relative to the session epoch.
//!
//! With Wintab gone, the *source* clock's origin is now the input source. For
//! the evdev source it is `input_event.time` — the kernel's timestamp for when
//! it queued the event — which is the honest analog of `pkTime`. The *ingest*
//! clock is the host monotonic clock at enqueue (see [`crate::clock`]).
//!
//! So every [`Sample`] carries both `source_time` and `ingest_time`. The two
//! are kept distinct on purpose: source time reflects when the hardware/kernel
//! observed the event; ingest time reflects when *this* recorder saw it. Clock
//! skew, buffering latency, and dropped-frame gaps are all recoverable only if
//! both are retained.

use std::time::Duration;

/// Raw pen state at one instant. Replaces the original's fixed 11-double row
/// (of which only 6 fields were populated). Values are raw device units as
/// reported by the source; unit conversion (e.g. to centimetres) is a
/// consumer-side concern, not baked into the sample.
///
/// Axes absent on a given device are `None` rather than a sentinel, so a
/// consumer can tell "not reported" from "reported as zero" — a distinction the
/// original's zero-filled reserved channels could not make.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PenPayload {
    pub x: i32,
    pub y: i32,
    pub pressure: i32,
    pub tilt_x: Option<i32>,
    pub tilt_y: Option<i32>,
    pub distance: Option<i32>,
    /// Bitset of pen/tool buttons that were active in this frame.
    pub buttons: PenButtons,
}

/// Pen button state as a small bitset. `BTN_TOOL_PEN` (proximity) and
/// `BTN_TOUCH` (contact) are tracked separately because proximity gates
/// validity (see the source layer) and contact gates "is the pen down".
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub struct PenButtons {
    pub tool_pen: bool,
    pub touch: bool,
    pub stylus: bool,
    pub stylus2: bool,
}

/// One stored sample: payload plus both clocks.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Sample {
    /// Source-origin timestamp, relative to the source's own reference. For
    /// evdev this is derived from `input_event.time`. Monotonic per source but
    /// not comparable across sources or to `ingest_time`.
    pub source_time: Duration,
    /// Host ingest timestamp relative to the session epoch, captured at enqueue.
    pub ingest_time: Duration,
    pub payload: PenPayload,
}
