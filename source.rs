//! The pluggable input seam.
//!
//! A [`Source`] yields raw pen frames tagged with a source-clock timestamp. It
//! deliberately does *not* know about the ring, the gate, or the host clock:
//! its only job is to turn some backend (evdev, a replay log, a future Wintab or
//! Windows-Ink shim) into a stream of proximity-valid [`SourceFrame`]s.
//!
//! Proximity handling is the source's responsibility, not the pipeline's,
//! because it is backend-specific. Per the kernel input docs, `ABS_X/ABS_Y/
//! ABS_PRESSURE/BTN_STYLUS` may carry stale values while the tool is out of
//! proximity, so a source MUST NOT emit a frame while proximity is false.

use crate::sample::PenPayload;
use std::time::Duration;

/// One proximity-valid pen frame from a source.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SourceFrame {
    /// Source-clock timestamp, relative to the source's own origin (fixed at
    /// source construction). For evdev, derived from `input_event.time`.
    pub source_time: Duration,
    pub payload: PenPayload,
}

/// A backend that produces pen frames.
///
/// `next_frame` blocks until the next proximity-valid frame is available, or
/// returns `Ok(None)` on clean end-of-stream (e.g. a replay log exhausted), or
/// `Err` on a backend I/O error.
pub trait Source {
    fn next_frame(&mut self) -> std::io::Result<Option<SourceFrame>>;
}

/// A deterministic in-memory source that replays a fixed list of frames. Used
/// by tests to drive the pipeline without hardware, and usable as a replay
/// backend for recorded sessions.
#[derive(Debug)]
pub struct ReplaySource {
    frames: std::vec::IntoIter<SourceFrame>,
}

impl ReplaySource {
    pub fn new(frames: Vec<SourceFrame>) -> Self {
        Self {
            frames: frames.into_iter(),
        }
    }
}

impl Source for ReplaySource {
    fn next_frame(&mut self) -> std::io::Result<Option<SourceFrame>> {
        Ok(self.frames.next())
    }
}
