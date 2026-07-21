//! Device-gated decimation.
//!
//! ## Preserved guarantee #2 — source-time decimation
//!
//! The original dropped any sample whose device timestamp did not exceed the
//! last *stored* sample's device timestamp by at least
//! `MIN_WACOM_STORE_INTERVAL_MS` (10 ms):
//!
//! ```c
//! if (pkt->pkTime > LastWacomTimestamp + MIN_WACOM_STORE_INTERVAL_MS) {
//!     write_error = write_to_circular_buffer(...);
//!     if (write_error != 3 && write_error != 1)   // store failed
//!         return 0;
//!     LastWacomTimestamp = pkt->pkTime;            // advance ONLY on success
//! }
//! ```
//!
//! Three properties are load-bearing and reproduced exactly:
//! 1. Comparison is on the **source** clock, never the host clock.
//! 2. Comparison is **strict** (`>`), against `last + interval`.
//! 3. `last` advances **only when the sample is actually stored**. A sample that
//!    passes the gate but fails to store must not move the reference forward,
//!    or the failed sample's slot silently widens the next accepted gap.
//!
//! This gate decides *admission*; it does not itself store. The caller stores
//! on `Decision::Admit` and calls `commit` iff the store succeeded, mirroring
//! the C control flow. This split is what lets property #3 hold without the gate
//! needing to know about the ring.

use std::time::Duration;

/// The default minimum inter-sample interval on the source clock: 10 ms,
/// matching `MIN_WACOM_STORE_INTERVAL_MS`. Yields an effective stored rate of
/// at most ~100 Hz.
pub const DEFAULT_MIN_INTERVAL: Duration = Duration::from_millis(10);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Decision {
    /// Sample is far enough past the last stored sample; caller should attempt
    /// to store it, then `commit` on success.
    Admit,
    /// Sample falls within the decimation interval; drop it. The reference is
    /// unchanged.
    Reject,
}

/// Source-time decimation gate.
#[derive(Debug)]
pub struct DecimationGate {
    min_interval: Duration,
    /// Source time of the last *stored* sample. `None` until the first store.
    last_stored: Option<Duration>,
    /// A source time was admitted and is awaiting commit-or-abort. Holds the
    /// candidate's source time so `commit` can set `last_stored` to the value
    /// that actually passed, not whatever the caller passes back.
    pending: Option<Duration>,
}

impl DecimationGate {
    pub fn new(min_interval: Duration) -> Self {
        Self {
            min_interval,
            last_stored: None,
            pending: None,
        }
    }

    /// Gate with the default 10 ms interval.
    pub fn default_interval() -> Self {
        Self::new(DEFAULT_MIN_INTERVAL)
    }

    /// Decide whether a sample at `source_time` should be admitted for storage.
    ///
    /// The first sample (no prior stored sample) is always admitted. Otherwise
    /// admit iff `source_time > last_stored + min_interval` (strict).
    ///
    /// Admitting records the candidate as pending; the caller must follow with
    /// exactly one of [`commit`](Self::commit) or [`abort`](Self::abort).
    pub fn admit(&mut self, source_time: Duration) -> Decision {
        let ok = match self.last_stored {
            None => true,
            Some(last) => source_time > last.saturating_add(self.min_interval),
        };
        if ok {
            self.pending = Some(source_time);
            Decision::Admit
        } else {
            Decision::Reject
        }
    }

    /// Confirm that the admitted sample was stored. Advances the reference to
    /// the admitted sample's source time. No-op if nothing is pending.
    pub fn commit(&mut self) {
        if let Some(t) = self.pending.take() {
            self.last_stored = Some(t);
        }
    }

    /// Report that the admitted sample failed to store. The reference is left
    /// unchanged, so the next sample is gated against the last *successful*
    /// store — reproducing the original's advance-only-on-success behaviour.
    pub fn abort(&mut self) {
        self.pending = None;
    }

    /// Source time of the last stored sample, if any.
    pub fn last_stored(&self) -> Option<Duration> {
        self.last_stored
    }
}
