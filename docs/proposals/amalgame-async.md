# amalgame-async + Mosaic fiber stack — design + status

**Status:** complete HTTP/1.1 fiber stack shipped end-to-end across
three packages (2026-05-22 → 2026-05-23). Linux production-ready.
This doc captures the design rationale, the alternatives that were
rejected, the lessons learned during integration, and what stays
deferred — kqueue / Windows / `Async.Select` / M:N / amc-side
`async` sugar.

## Why a separate package, not amc and not amalgame-threading

The original roadmap (`ROADMAP_COMPLET.md:250`) framed `async` /
`await` as an amc language feature. After weighing alternatives,
that framing turned out wrong for v0.1. Three options were on the
table:

### Option A — amc language feature (`async fn` / `await expr`)

Add `async` and `await` as keywords. Compiler lowers `async fn`
to a state-machine struct + a `Future<T>` runtime type, à la
Rust / C# / TypeScript. **Cost: multi-week** CPS transform in
`c_gen.am` (Rust's MIR transform is hundreds of lines).

**Rejected for v0.1** because without a concrete use case driving
the surface (HTTP handler, streaming download, parallel I/O), the
syntax and semantics are designed in a vacuum. Better to ship the
runtime primitives first, let real consumers shape the API, and
add the sugar later as a desugaring pass once the right shape is
clear.

### Option B — extend amalgame-threading

Add `Fiber.Spawn` / scheduler to the existing threading package.
**Cost: low** (same C build, no new package infra).

**Rejected** because the mental models are orthogonal: threading
is preemptive OS threads with locks for shared state; async is a
cooperative scheduler over fibers sharing one thread, with yields
for scheduling and channels for state transfer. Mixing them in
one package forces every `Mutex` consumer to embed a scheduler,
confuses `Fiber` vs `Thread` for new users, and blocks the clean
composition story (M:N = threads × scheduler each). They are
**orthogonal layers**, not features of one package.

### Option C — new `amalgame-async` package (chosen)

Stackful coroutines on POSIX `ucontext`, single-threaded
round-robin scheduler with parking channels and a sleep list. No
amc changes. Composes orthogonally with `amalgame-threading`. A
future amc-side `async` / `await` sugar becomes a desugaring pass
that emits `Async.FiberSpawn` + `Async.Channel*` calls — the
runtime primitives already work, the keyword form is later
ergonomics.

Lua coroutines, Go goroutines (pre-1.0), Tokio's runtime all
started as libraries before becoming language features (or stayed
libraries). Same path.

## What shipped

### amalgame-async — 5 versions

| Version | Surface | Notes |
|---|---|---|
| v0.1.0 | `Fiber.Spawn/Yield/Sleep/CurrentId`, `Channel.*`, `Scheduler.Run/RunUntil/Pending` | Stackful coroutines on POSIX ucontext (Linux + macOS + *BSD), single-threaded round-robin scheduler |
| v0.2.0 | `WaitFdReadable/WaitFdWritable/MakeNonBlocking` | Linux epoll I/O parking; recv/send on a socket parks the fiber instead of blocking the OS thread |
| v0.2.1 | **Critical fix:** `static AmalgameAsyncScheduler` → `__attribute__((weak))` | Cross-TU scheduler-sharing bug — each `.o` had its own scheduler instance, so fibers spawned in `net-http.o` were invisible to `FiberCurrentId()` called from `user-app.o`. Caused silent request serialisation in Mosaic. **See "Lesson learned" below.** |
| v0.2.2 | `FiberCancel` + `IsCancelled` | Cooperative cancellation: wakes parked fibers + sets a flag. Idempotent. Channels also splice cleanly via `chan_wait_head` back-ref |
| v0.2.3 | `WithTimeout(closure, arg, ms)` | Ergonomic wrapper for the spawn-worker + spawn-timer + race-on-channel + cancel pattern |

### amalgame-net-http — 6 versions on top of v0.8.0

| Version | Surface | Notes |
|---|---|---|
| v0.9.0 | `H1Server_RawFd` / `H1Conn_RawFd` | Accessors so user code can drive its own fiber-based accept / read loop with `Async.WaitFd*` |
| v0.9.1 | `Http1.ServeAsync(port, handler)` | High-level fiber-driven HTTP/1.1 server. `H1Conn.async_io` flag, `amalgame_h1_recv_into` + `amalgame_h1_send_all` wrap recv/send with `MSG_DONTWAIT` + WaitFd-on-EAGAIN. Accept loop runs as an `amalgame-async` fiber |
| v0.9.2 | HTTP/1.1 keep-alive on the async path + reproducible benchmark in `bench/` | Per-conn fiber loops `parse → handler → ResetForReuse` when the request allows it |
| v0.9.3 | `Http1.ServeAsyncWith(port, cfg, handler)` | Per-conn `HttpServerConfig` wired in. `max_*_bytes`, `listen_backlog`, `idle_timeout_sec`-driven keep-alive all honored. Refactors `ServeAsync` to thin-wrap a shared `amalgame_h1_serve_async_impl` |
| v0.9.4 | Per-phase `header_timeout_sec` / `body_timeout_sec` plumbed into the async recv helper | Slow-client mitigation on the async path now matches sync `ServeWith` |
| v0.9.5 | Graceful shutdown cancels in-flight fibers via `FiberCancel` | Accept loop tracks per-conn fibers in a doubly-linked list rooted on the ctx; on SIGTERM walks the list and cancels each. Shutdown latency: **0–1 ms** (was up to the sum of configured timeouts) |

### amalgame-web — 5 versions on top of v0.11.3

| Version | Surface | Notes |
|---|---|---|
| v0.12.0 | `WebApp.ServeAsync(port)` | 5th `Serve*` method on the WebApp class. Drop-in replacement for `Serve` / `ServeMt` |
| v0.12.1 | Dep pin: `amalgame-async >= 0.2.1` (critical fix) | Without this pin, Mosaic apps using `ServeAsync` would hit the cross-TU bug |
| v0.12.2 | `WebApp.ServeAsyncWith(port, cfg)` | 6th `Serve*` method. Configs portable across the full `Serve / ServeMt / ServeWith / ServeMtWith / ServeAsync / ServeAsyncWith` matrix |
| v0.12.3 | Dep pin: `amalgame-net-http >= 0.9.4` | Picks up per-phase timeouts transitively |
| v0.12.4 | Dep pin: `amalgame-net-http >= 0.9.5` + `amalgame-async >= 0.2.2` | Picks up graceful-shutdown cancellation transitively |

## Empirical justification — the benchmark

100 ms I/O-bound handler (simulating downstream HTTP / DB / file
I/O via `Amalgame_Async_FiberSleep(100)`), asyncio HTTP client
opening N concurrent connections on a 2-core / 4 GB Linux box. RSS
sampled every 10 ms during the burst.

| N    | `ServeMt`              | `ServeAsync`             |
|------|------------------------|--------------------------|
| 100  | 1152 ms · 100/100 · 2.5 MB | **123 ms** · 100/100 · 9.8 MB |
| 500  | 2071 ms · 500/500 · 3.7 MB | **1374 ms** · 500/500 · 37 MB |
| 1000 | 2932 ms · 1000/1000 · 6.2 MB | **1628 ms** · 1000/1000 · 71 MB |
| 2000 | 31220 ms · **1665/2000** ⚠ | **1453 ms** · **2000/2000** ✅ |

Takeaways:

- **Throughput:** ServeAsync is 1.5×–9× faster across the
  range, single-threaded.
- **Reliability under load:** at N=2000 `ServeMt` drops 17% of
  requests and the survivors take 31 s; `ServeAsync` handles
  100% in 1.5 s.
- **Memory shape:** `ServeMt`'s pthread stacks are 8 MB but
  lazy-mapped — RSS stays small as long as handlers don't touch
  much stack. `ServeAsync`'s GC-allocated 64 KB stacks are
  eagerly resident, so RSS scales with concurrency (~70 KB /
  connection) but is bounded and predictable.
- **Crossover** where `ServeMt` collapses: somewhere between 1k
  and 2k concurrent connections on this hardware. Limit is
  pthread setup + kernel scheduler contention, not memory.

Reproducible from
[`amalgame-net-http/bench/`](https://github.com/amalgame-lang/amalgame-net-http/tree/main/bench).

## Lessons learned during integration

### 1. Cross-TU `static` is poison for header-only globals

Pre-v0.2.1, `_amasync_sched` was declared `static` in the
runtime header. amc generates one `.o` per package
(`nethttp.o`, `web/facade.o`, the user's `app.o`); every `.o`
that included `Amalgame_Async.h` ended up with its own private
copy of the scheduler.

The bug surfaced exactly the moment Mosaic wired it up:

```
[bridge] fiber=2          ← per-conn fiber, in nethttp.o's scheduler
[Handle] fiber=2          ← still fiber=2, in web/facade.o
[h] after MutexUnlock fiber=2
[HANDLER enter] fiber=0   ← user handler in app.o — DIFFERENT scheduler instance, no fiber!
```

`FiberCurrentId() == 0` inside the user handler meant `FiberSleep`
silently fell back to blocking `nanosleep`, serialising the entire
server: **3 concurrent `/slow` probes took 1500 ms instead of
500 ms**. After the fix: 593 ms (parallel as designed).

Fix: `__attribute__((weak))` on the global. Linker merges every
TU's emission into one shared instance. Same idiom Boost / Eigen
use for header-only shared globals.

The fix took ten characters. Finding it took an hour of
bisection through the WebApp middleware stack.

**Generalised lesson:** any time a header-only library carries
shared state (a global, a counter, a registry), it must be
`__attribute__((weak))` or moved to a `.c` file. `static` at file
scope means private-to-TU, which is exactly the wrong default for
shared state. A new cross-TU smoke test in
`amalgame-async/tests/run_tests.sh` builds two `.o` files which
both include the header and asserts they observe the same
`FiberCurrentId()` — that test would have caught it.

### 2. Cooperative cancellation beats preemptive everywhere we measured

The graceful-shutdown story in net-http v0.9.5 was the
existence-proof: when SIGTERM fires, walk a list of in-flight
fibers, `FiberCancel` each. Parked `recv` / `send` / `FiberSleep`
wake with sentinel returns, handlers exit, `SchedulerRun` returns
in **~1 ms** instead of waiting out the configured timeouts (could
be a minute or more in aggregate).

Cooperative model has zero signal-safety juggling: no `pthread_kill`,
no `setjmp`/`longjmp`, no async-signal-safe code in the cancel
path. Just a flag + a queue splice + a `ready_push`. Idempotent
by construction.

The cost is that handlers running pure CPU code keep running
until they voluntarily yield. For our typical I/O-bound HTTP
handlers (which yield on every recv / send anyway), this is
fine. For pure-compute handlers, document the caveat or have
them call `IsCancelled()` between expensive steps.

### 3. Ergonomic helpers reduce the surface area users have to learn

`WithTimeout(closure, arg, ms)` is ~80 lines of runtime code
that replaces ~25 lines of bookkeeping at every call site
(channel + spawn + spawn + receive + cancel + cancel). After
seeing the pattern repeat itself twice in the Mosaic integration
sketches, we factored it. Net: users see one function, not five.

## What's still deferred — and why

| Item | Reason |
|---|---|
| ~~`Async.Select(channels)`~~ | **Shipped v0.3.0 (2026-05-31).** The correct runtime-integrated design landed: each fiber registers a waiter node on every channel at once (per-fiber `select_parked` flag + a `select_recv` list per channel), a sender wakes at most one consumer per value via `_amasync_chan_signal_recv` (plain receivers first, else a select waiter), and the woken fiber re-scans + pulls atomically before unlinking its nodes — so no value is consumed-then-discarded. Round-robin start offset for fairness; `FiberCancel` unparks select-parked fibers (→ returns -1). Exposed as `SelectReceive` / `SelectTryReceive` / `SelectValue`, 5 tests green. Multi-*fd* select still deferred. |
| kqueue backend (BSD + macOS) | I'd be shipping code I can't test on the target platform |
| Windows Fibers + IOCP backend | Same — no Windows to dry-run on, and Windows Fibers API is non-trivial to get right |
| Timer wheel | Optimization for >1k concurrent sleepers; current sorted-insertion list is O(N) per `Sleep`. No bench yet shows this matters |
| M:N scheduling (per-thread scheduler via TLS, cross-thread channels) | Significant refactor. Today's scheduler is process-wide via the `weak` global. A real M:N implementation needs TLS slots + cross-thread channel sync — substantial work that should happen with a real workload to drive design |
| amc-side `async fn` / `await expr` sugar | Trivial desugaring (the runtime is stackful — no CPS state machine to generate) but should land when a real Mosaic consumer pushes for it. The runtime API as-is composes fine |
| Async H2 / Https / Ws / Wss | Each gated on amalgame-tls fiber-aware I/O. Substantial scope per protocol; HTTPS in particular needs OpenSSL's BIO non-blocking dance threaded through `WaitFdReadable` / `WaitFdWritable` |

## Status — what to use today

Linux deploy with I/O-bound handlers:

```amalgame
import Amalgame.Web
import Amalgame.Net.Http
import Amalgame.Async

let cfg = HttpServerConfig.Default()
    .WithMaxBodyBytes(2 * 1024 * 1024)
    .WithHeaderTimeoutSec(5)
    .WithBodyTimeoutSec(30)
    .WithIdleTimeoutSec(15)
    .WithListenBacklog(256)

let app = new WebApp()
app.Get("/slow", ctx => {
    // Per-request budget via the helper.
    let done: bool = Async.WithTimeout(downstreamCall, 0, 10000)
    if (!done) {
        return HttpResponse.New().Status(504).Text("upstream timeout")
    }
    return HttpResponse.New().Text("ok")
})

// SIGTERM cancels in-flight fibers in ~1 ms.
app.ServeAsyncWith(8080, cfg)
```

Cross-platform deploy or CPU-bound handlers: stay on `ServeMtWith`
until kqueue / IOCP backends ship.

## References

- `amalgame-async` repo: <https://github.com/amalgame-lang/amalgame-async>
- `amalgame-net-http` repo: <https://github.com/amalgame-lang/amalgame-net-http>
- `amalgame-web` repo: <https://github.com/amalgame-lang/amalgame-web>
- Bench reproducer: <https://github.com/amalgame-lang/amalgame-net-http/tree/main/bench>
- BDW GC `GC_set_stackbottom` semantics:
  <https://github.com/ivmai/bdwgc/blob/master/include/gc.h>
- Go scheduler M:N design — Dmitry Vyukov's papers, late 2010s
- Rust async desugaring — `rustc_mir_transform/src/coroutine.rs`
  (a useful reference for the CPS / state-machine approach we
  *don't* need to copy because we're stackful)
- "Asynchronous Programming in Rust", Carl Fredrik Samson — best
  book-length walkthrough of why stackful vs stackless matters
