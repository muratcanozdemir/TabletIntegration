//! # tabletcore
//!
//! A cross-platform Rust reimplementation of the *data-integrity core* of the
//! original Wacom TabletIntegration project. It carries over exactly two
//! guarantees from the 2010 Windows/MATLAB original and discards everything
//! else (shared memory, the Windows mutex, the byte-offset layout, the
//! `FILE*`-in-shmem hack, the drain thread, the 11-double row, the `-v4` .mat
//! output, and the `Paradigm_Name.m` polling channel — all artifacts of the
//! old stack).
//!
//! ## Preserved guarantees
//! 1. **Dual-clock timestamps** — every stored sample carries a *source* clock
//!    (from the input backend, e.g. evdev `input_event.time`) and a *host
//!    ingest* clock captured at enqueue relative to a session epoch.
//! 2. **Source-time decimation** — a sample is stored only if its source
//!    timestamp exceeds the last *stored* sample's source timestamp by a
//!    configured interval (default 10 ms, strict), and the reference advances
//!    only on a successful store.

pub mod clock;
pub mod decimate;
pub mod recorder;
pub mod ring;
pub mod sample;
pub mod source;

#[cfg(target_os = "linux")]
pub mod evdev_source;

#[cfg(test)]
mod tests {
    use crate::clock::{ManualClock, MonotonicClock, Session};
    use crate::decimate::{DEFAULT_MIN_INTERVAL, DecimationGate, Decision};
    use crate::recorder::{Outcome, Recorder};
    use crate::ring::SampleRing;
    use crate::sample::{PenButtons, PenPayload};
    use crate::source::{ReplaySource, Source, SourceFrame};
    use std::time::Duration;

    fn payload() -> PenPayload {
        PenPayload {
            x: 1,
            y: 2,
            pressure: 3,
            tilt_x: None,
            tilt_y: None,
            distance: None,
            buttons: PenButtons::default(),
        }
    }

    fn frame(ms: u64) -> SourceFrame {
        SourceFrame {
            source_time: Duration::from_millis(ms),
            payload: payload(),
        }
    }

    // ----- Guarantee #2: decimation ---------------------------------------

    #[test]
    fn gate_admits_first_sample_always() {
        let mut g = DecimationGate::default_interval();
        assert_eq!(g.admit(Duration::from_millis(0)), Decision::Admit);
    }

    #[test]
    fn gate_is_strict_greater_than_last_plus_interval() {
        let mut g = DecimationGate::new(Duration::from_millis(10));
        assert_eq!(g.admit(Duration::from_millis(0)), Decision::Admit);
        g.commit();
        assert_eq!(g.admit(Duration::from_millis(10)), Decision::Reject);
        assert_eq!(g.admit(Duration::from_millis(11)), Decision::Admit);
    }

    #[test]
    fn gate_reference_advances_only_on_commit() {
        let mut g = DecimationGate::new(Duration::from_millis(10));
        assert_eq!(g.admit(Duration::from_millis(0)), Decision::Admit);
        g.commit();
        assert_eq!(g.last_stored(), Some(Duration::from_millis(0)));

        assert_eq!(g.admit(Duration::from_millis(20)), Decision::Admit);
        g.abort();
        assert_eq!(g.last_stored(), Some(Duration::from_millis(0)));

        assert_eq!(g.admit(Duration::from_millis(11)), Decision::Admit);
    }

    #[test]
    fn gate_uses_source_time_not_call_order() {
        let mut g = DecimationGate::new(Duration::from_millis(10));
        g.admit(Duration::from_millis(100));
        g.commit();
        assert_eq!(g.admit(Duration::from_millis(105)), Decision::Reject);
        assert_eq!(g.admit(Duration::from_millis(111)), Decision::Admit);
    }

    // ----- Guarantee #1: dual-clock stamping ------------------------------

    #[test]
    fn sample_carries_both_clocks_and_ingest_is_epoch_relative() {
        let clock = ManualClock::new();
        clock.set_nanos(5_000_000_000);
        let session = Session::start(clock.clone());
        let mut rec = Recorder::new(
            session,
            DecimationGate::default_interval(),
            SampleRing::new(16),
        );

        clock.set_nanos(5_250_000_000);
        let out = rec.ingest(frame(0));
        assert_eq!(out, Outcome::Stored { overran: false });

        let s = rec.ring_mut().pop().unwrap();
        assert_eq!(s.source_time, Duration::from_millis(0));
        assert_eq!(s.ingest_time, Duration::from_millis(250));
    }

    #[test]
    fn ingest_time_and_source_time_are_independent() {
        let clock = ManualClock::new();
        let session = Session::start(clock.clone());
        let mut rec = Recorder::new(
            session,
            DecimationGate::new(Duration::from_millis(10)),
            SampleRing::new(16),
        );

        clock.set_nanos(1_000_000);
        rec.ingest(frame(1000));
        clock.set_nanos(2_000_000);
        rec.ingest(frame(1020));

        let a = rec.ring_mut().pop().unwrap();
        let b = rec.ring_mut().pop().unwrap();
        assert_eq!(a.source_time, Duration::from_millis(1000));
        assert_eq!(a.ingest_time, Duration::from_millis(1));
        assert_eq!(b.source_time, Duration::from_millis(1020));
        assert_eq!(b.ingest_time, Duration::from_millis(2));
    }

    // ----- Ring overrun ----------------------------------------------------

    #[test]
    fn ring_overwrites_oldest_and_counts_drops() {
        let mut r = SampleRing::new(2);
        let mk = |t| crate::sample::Sample {
            source_time: Duration::from_millis(t),
            ingest_time: Duration::ZERO,
            payload: payload(),
        };
        assert!(!r.push(mk(0)));
        assert!(!r.push(mk(1)));
        assert!(r.push(mk(2)));
        assert_eq!(r.dropped(), 1);
        assert_eq!(r.len(), 2);
        assert_eq!(r.pop().unwrap().source_time, Duration::from_millis(1));
        assert_eq!(r.pop().unwrap().source_time, Duration::from_millis(2));
    }

    // ----- End-to-end via ReplaySource ------------------------------------

    #[test]
    fn pipeline_decimates_a_dense_stream_to_the_interval() {
        let frames: Vec<_> = (0..100).map(frame).collect();
        let mut source = ReplaySource::new(frames);
        let session = Session::start(MonotonicClock::new());
        let mut rec = Recorder::new(
            session,
            DecimationGate::new(DEFAULT_MIN_INTERVAL),
            SampleRing::new(1024),
        );

        let mut stored = 0;
        let mut decimated = 0;
        while let Some(outcome) = rec.run_once(&mut source).unwrap() {
            match outcome {
                Outcome::Stored { .. } => stored += 1,
                Outcome::Decimated => decimated += 1,
            }
        }
        assert_eq!(stored + decimated, 100);
        let mut samples = Vec::new();
        while let Some(s) = rec.ring_mut().pop() {
            samples.push(s.source_time);
        }
        for w in samples.windows(2) {
            assert!(w[1] > w[0] + Duration::from_millis(10));
        }
        assert_eq!(samples[0], Duration::from_millis(0));
    }

    #[test]
    fn replay_source_reports_end_of_stream() {
        let mut source = ReplaySource::new(vec![frame(0)]);
        assert!(source.next_frame().unwrap().is_some());
        assert!(source.next_frame().unwrap().is_none());
    }
}
