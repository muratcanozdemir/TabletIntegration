# tabletcore

A cross-platform Rust library for the data-integrity core of tablet sample
recording. It is a reimplementation of the recording guarantees from an older
Windows/MATLAB Wacom integration, keeping the two properties that mattered and
discarding the platform machinery that carried them.

This crate does **not** implement a tablet driver. On Linux the kernel's
`wacom` / `input-wacom` drivers already expose pen data through evdev; this
library consumes that stream. The input side is abstracted behind a trait, so
other backends (a replay log, a future Windows or macOS shim) plug in without
touching the core.

## What it guarantees

Two properties are preserved verbatim from the original design. Everything else
about the original — shared memory, a Windows mutex, a hand-computed byte-offset
layout, a `FILE*` smuggled through shared memory, a background drain thread, a
fixed 11-double row, `-v4` `.mat` output, and a filesystem-polling control
channel — was an artifact of the 2010 stack and is gone.

### 1. Dual-clock timestamps

Every stored sample carries two independent timestamps:

- **Source time** — supplied by the input backend at sample origin. For the
  evdev backend this derives from the kernel's `input_event` timestamp, i.e.
  when the kernel observed the event. It is expressed relative to the source's
  own origin (the first event seen).
- **Ingest time** — captured by *this* recorder at the moment the sample is
  enqueued, from a monotonic host clock, expressed relative to a session epoch
  fixed at recording start.

The two are kept distinct on purpose. Source time reflects hardware/kernel
observation; ingest time reflects when the recorder saw it. Clock skew,
buffering latency, and dropped-frame gaps are only recoverable if both are
retained. See `sample.rs` and `clock.rs`.

### 2. Source-time decimation

A sample is stored only if its **source** timestamp exceeds the last *stored*
sample's source timestamp by at least a configured interval (default 10 ms,
strict `>`). Three details are load-bearing and reproduced exactly:

1. The comparison is on the source clock, never the host clock.
2. It is strict: exactly `last + interval` is rejected.
3. The reference advances **only when a sample is actually stored**. A sample
   that passes the gate but fails to store must not move the reference forward.

This mirrors the original's `pkTime > LastWacomTimestamp + MIN_INTERVAL` with
`LastWacomTimestamp` advancing only on a successful write. The gate exposes
`admit` / `commit` / `abort` so the caller stores between admit and commit,
keeping property 3 intact without the gate needing to know about the buffer.
See `decimate.rs`.

## Architecture

```
Source ──frames──▶ Recorder ──▶ DecimationGate (source-time admission)
                       │
                       ├─ Session/Clock  (host ingest timestamp at enqueue)
                       │
                       └──▶ SampleRing   (bounded, overwrite-oldest, counted)
```

| Module            | Responsibility                                              |
|-------------------|-------------------------------------------------------------|
| `clock`           | Host monotonic clock trait; session epoch; a manual test clock |
| `sample`          | The stored sample and its pen payload                       |
| `ring`            | Bounded ring buffer, overwrite-oldest with a drop counter   |
| `decimate`        | Source-time decimation gate (`admit`/`commit`/`abort`)      |
| `source`          | The `Source` trait seam + an in-memory `ReplaySource`       |
| `evdev_source`    | Real evdev backend (Linux only, compiled out elsewhere)     |
| `recorder`        | Wires source → gate → dual-clock stamp → ring               |

### Ring overrun policy

When the ring is full, the oldest unread sample is overwritten and a drop
counter is incremented. This keeps the input path non-blocking (matching the
original's intent) but makes loss visible: a non-zero `dropped()` means the
consumer fell behind. It is the in-process analog of evdev's own `SYN_DROPPED`.

### Cross-platform boundary

The core (clock, sample, ring, decimate, recorder, source) is pure `std` and
builds everywhere. `evdev` is declared as a Linux-only dependency, so on macOS
and Windows the evdev backend and its entire dependency tree are absent from the
build — the same `cargo test` invocation exercises core-only there and the full
core-plus-evdev build on Linux. The CI matrix checks all three.

## Using it

```rust
use tabletcore::clock::{MonotonicClock, Session};
use tabletcore::decimate::DecimationGate;
use tabletcore::recorder::Recorder;
use tabletcore::ring::SampleRing;

let session = Session::start(MonotonicClock::new());
let mut recorder = Recorder::new(
    session,
    DecimationGate::default_interval(), // 10 ms
    SampleRing::new(1024),
);

// Drive it with any Source. On Linux, locate the pen automatically:
# #[cfg(target_os = "linux")]
# fn run(recorder: &mut tabletcore::recorder::Recorder<tabletcore::clock::MonotonicClock>)
#     -> std::io::Result<()> {
use tabletcore::evdev_source::EvdevSource;

if let Some(mut source) = EvdevSource::find_pen()? {
    while let Some(_outcome) = recorder.run_once(&mut source)? {
        // drain recorder.ring_mut() as samples accumulate
    }
}
# Ok(())
# }
```

For tests and offline replay, `ReplaySource` feeds a fixed list of frames
through the identical pipeline with no hardware.

## Status

- The core modules and the decimation/timestamp guarantees are covered by unit
  tests and an end-to-end pipeline test (`cargo test`).
- The evdev backend **compiles against the real `evdev` API** but has not been
  exercised against physical hardware in this repository's tests. Frame
  assembly (accumulating `EV_ABS`/`EV_KEY` events and emitting at `SYN_REPORT`)
  and the proximity gate (`BTN_TOOL_PEN`) are verified only by compilation.
  First runtime validation should be against a real device or a `uinput`
  synthetic pen.
- evdev's `SYN_DROPPED` (kernel-side event loss) is currently ignored. On a real
  capture it is a data-integrity signal worth surfacing alongside the ring's own
  drop counter.

### Deferred by scope

Persistence format (the original `.mat` output is not reimplemented), the
consumer/experiment loop and its IPC, and the concurrency scheme for the handoff
between an evdev read thread and a drain are open. The ring is intentionally an
un-synchronized SPSC buffer pending that decision.

## Building

```
cargo build          # core + evdev on Linux; core-only on macOS/Windows
cargo test           # 9 tests: guarantees + pipeline
cargo clippy --all-targets -- -D warnings
cargo fmt --all -- --check
```

MSRV is 1.85 (the floor for the 2024 edition).

## License

Dual-licensed under MIT or Apache-2.0.
