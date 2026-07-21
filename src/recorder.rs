//! The recording pipeline: the one place all three guarantees meet.
//!
//! For each frame from the source:
//! 1. Ask the [`DecimationGate`] whether to admit it, gating on the frame's
//!    **source** time (guarantee #2).
//! 2. If admitted, capture the host **ingest** timestamp from the session clock
//!    and build a [`Sample`] carrying both clocks (guarantee #1).
//! 3. Push into the [`SampleRing`]; on success, `commit` the gate so its
//!    reference advances only for stored samples (guarantee #2, property #3).
//!
//! The ingest stamp is taken *after* admission and immediately before the push,
//! which is the closest analog to the original capturing the host time inside
//! the write lock. A rejected sample costs no ingest stamp and does not perturb
//! the gate.

use crate::clock::{Clock, Session};
use crate::decimate::{DecimationGate, Decision};
use crate::ring::SampleRing;
use crate::sample::Sample;
use crate::source::{Source, SourceFrame};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Outcome {
    /// Sample passed the gate and was stored. `overran` is true if storing it
    /// forced an older unread sample to be dropped.
    Stored { overran: bool },
    /// Sample was decimated (too close to the previous stored sample).
    Decimated,
}

/// Owns the gate, the session clock, and the ring; turns source frames into
/// stored samples.
pub struct Recorder<C: Clock> {
    session: Session<C>,
    gate: DecimationGate,
    ring: SampleRing,
}

impl<C: Clock> Recorder<C> {
    pub fn new(session: Session<C>, gate: DecimationGate, ring: SampleRing) -> Self {
        Self {
            session,
            gate,
            ring,
        }
    }

    /// Process one already-obtained frame. Exposed separately from
    /// [`run_once`](Self::run_once) so tests can feed frames directly.
    pub fn ingest(&mut self, frame: SourceFrame) -> Outcome {
        match self.gate.admit(frame.source_time) {
            Decision::Reject => Outcome::Decimated,
            Decision::Admit => {
                // Guarantee #1: host ingest stamp captured here, at store time.
                let ingest_time = self.session.ingest_stamp();
                let sample = Sample {
                    source_time: frame.source_time,
                    ingest_time,
                    payload: frame.payload,
                };
                let overran = self.ring.push(sample);
                // The push cannot fail (overwrite-oldest), so the store always
                // succeeds and we always commit. If a future ring can reject a
                // push, this is where we would `abort` instead.
                self.gate.commit();
                Outcome::Stored { overran }
            }
        }
    }

    /// Pull one frame from the source and process it. Returns `Ok(Some(outcome))`
    /// per frame, `Ok(None)` at end-of-stream.
    pub fn run_once(&mut self, source: &mut dyn Source) -> std::io::Result<Option<Outcome>> {
        match source.next_frame()? {
            Some(frame) => Ok(Some(self.ingest(frame))),
            None => Ok(None),
        }
    }

    pub fn ring(&self) -> &SampleRing {
        &self.ring
    }

    pub fn ring_mut(&mut self) -> &mut SampleRing {
        &mut self.ring
    }
}
