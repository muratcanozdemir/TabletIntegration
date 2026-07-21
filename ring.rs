//! Bounded sample ring.
//!
//! The original used a 731-slot circular buffer in shared memory, coordinated
//! by a Windows mutex, with a background thread draining it to a file. Overrun
//! (write pointer catching read pointer) was signalled by a flag but the
//! producer kept overwriting — the reader could silently lose the oldest
//! unread samples.
//!
//! Here the ring is an in-process SPSC buffer. The cross-process shared-memory
//! machinery and the `FILE*`-in-shmem hack are gone (they only existed because
//! writer and drain thread shared an address space on Windows); the drain now
//! lives with the writer, so a plain bounded queue with explicit overrun
//! accounting is enough.
//!
//! ## Overrun policy — overwrite-oldest, counted (matches original intent)
//!
//! When the buffer is full, `push` overwrites the oldest unread sample and
//! increments `dropped`. This preserves the original's "recorder never blocks
//! the input thread" property, but makes the loss *visible and counted* instead
//! of a bare flag. A consumer that reads `dropped` can detect and report gaps.

use crate::sample::Sample;
use std::collections::VecDeque;

/// Single-producer / single-consumer bounded ring of [`Sample`]s.
///
/// Not internally synchronised: intended to sit behind whatever handoff the
/// caller chooses (channel, mutex, lock-free wrapper). Kept deliberately simple
/// so the guarantee tests exercise buffer *semantics*, not a concurrency scheme
/// that is still an open design decision.
#[derive(Debug)]
pub struct SampleRing {
    buf: VecDeque<Sample>,
    capacity: usize,
    dropped: u64,
    pushed: u64,
}

impl SampleRing {
    /// Create a ring holding at most `capacity` samples. `capacity` must be > 0.
    pub fn new(capacity: usize) -> Self {
        assert!(capacity > 0, "ring capacity must be non-zero");
        Self {
            buf: VecDeque::with_capacity(capacity),
            capacity,
            dropped: 0,
            pushed: 0,
        }
    }

    /// Enqueue a sample. If the ring is full, the oldest unread sample is
    /// discarded (and `dropped` incremented) to make room. Returns `true` if a
    /// sample was dropped to accommodate this push.
    pub fn push(&mut self, s: Sample) -> bool {
        self.pushed += 1;
        let overran = if self.buf.len() == self.capacity {
            self.buf.pop_front();
            self.dropped += 1;
            true
        } else {
            false
        };
        self.buf.push_back(s);
        overran
    }

    /// Dequeue the oldest sample, if any.
    pub fn pop(&mut self) -> Option<Sample> {
        self.buf.pop_front()
    }

    /// Number of samples currently buffered.
    pub fn len(&self) -> usize {
        self.buf.len()
    }

    pub fn is_empty(&self) -> bool {
        self.buf.is_empty()
    }

    pub fn capacity(&self) -> usize {
        self.capacity
    }

    /// Total samples ever overwritten due to overrun. A non-zero value means
    /// the consumer fell behind and data was lost.
    pub fn dropped(&self) -> u64 {
        self.dropped
    }

    /// Total samples ever pushed (including those later dropped).
    pub fn pushed(&self) -> u64 {
        self.pushed
    }
}
