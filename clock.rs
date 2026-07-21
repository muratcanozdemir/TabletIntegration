//! Host clock abstraction.
//!
//! The original TabletIntegration captured a host timestamp inside the write
//! lock via `QueryPerformanceCounter`, expressed as seconds since a session
//! epoch set at recording start (with a `_ftime64` fallback). That is the
//! *ingest* clock: it stamps when a sample entered the buffer, independent of
//! the sample's own source timestamp.
//!
//! Here that becomes a trait so the ingest-timestamp guarantee is testable
//! without a real wall clock. `MonotonicClock` is the production impl backed by
//! `std::time::Instant` (guaranteed monotonic, never runs backwards). `ManualClock`
//! lets tests drive time deterministically.

use std::time::{Duration, Instant};

/// A source of monotonically non-decreasing host time.
///
/// Only relative durations matter: every ingest stamp is expressed relative to
/// a session epoch (see [`Session`]), matching the original's
/// `store_time -= start_time`.
pub trait Clock {
    /// Current instant as a duration since some fixed, clock-internal origin.
    /// Absolute value is meaningless; only differences are used.
    fn now(&self) -> Duration;
}

/// Production clock. Wraps a fixed `Instant` origin captured at construction;
/// `now()` returns elapsed time since that origin.
#[derive(Debug, Clone)]
pub struct MonotonicClock {
    origin: Instant,
}

impl MonotonicClock {
    pub fn new() -> Self {
        Self {
            origin: Instant::now(),
        }
    }
}

impl Default for MonotonicClock {
    fn default() -> Self {
        Self::new()
    }
}

impl Clock for MonotonicClock {
    fn now(&self) -> Duration {
        self.origin.elapsed()
    }
}

/// A recording session: fixes the epoch against which ingest timestamps are
/// measured. Corresponds to the original's `start_time` captured in
/// `InitializeRecording`.
///
/// `ingest_stamp()` is the value stored per sample: host time at the moment of
/// the call, minus the session epoch. It is generic over the clock so tests can
/// substitute [`ManualClock`].
#[derive(Debug)]
pub struct Session<C: Clock> {
    clock: C,
    epoch: Duration,
}

impl<C: Clock> Session<C> {
    /// Open a session, fixing the epoch to the clock's current reading.
    pub fn start(clock: C) -> Self {
        let epoch = clock.now();
        Self { clock, epoch }
    }

    /// Host ingest timestamp for a sample enqueued *now*, relative to the epoch.
    ///
    /// Saturating: a well-behaved monotonic clock never returns a reading below
    /// the epoch, but we saturate rather than panic to keep the recorder alive
    /// under a misbehaving clock impl.
    pub fn ingest_stamp(&self) -> Duration {
        self.clock.now().saturating_sub(self.epoch)
    }
}

/// Test clock. `now()` returns whatever was last set via `advance`/`set`,
/// starting at zero. Not monotonic by construction — tests are responsible for
/// only advancing it, which lets us also exercise the saturating path.
#[derive(Debug, Clone)]
pub struct ManualClock {
    now: std::sync::Arc<std::sync::atomic::AtomicU64>, // nanoseconds
}

impl ManualClock {
    pub fn new() -> Self {
        Self {
            now: std::sync::Arc::new(std::sync::atomic::AtomicU64::new(0)),
        }
    }

    /// Set absolute time in nanoseconds since origin.
    pub fn set_nanos(&self, nanos: u64) {
        self.now.store(nanos, std::sync::atomic::Ordering::SeqCst);
    }

    /// Advance by a duration.
    pub fn advance(&self, by: Duration) {
        let add = by.as_nanos() as u64;
        self.now.fetch_add(add, std::sync::atomic::Ordering::SeqCst);
    }
}

impl Default for ManualClock {
    fn default() -> Self {
        Self::new()
    }
}

impl Clock for ManualClock {
    fn now(&self) -> Duration {
        Duration::from_nanos(self.now.load(std::sync::atomic::Ordering::SeqCst))
    }
}
