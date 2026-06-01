# Async & concurrency

Amalgame has two layers of cooperative concurrency, and they fit
together:

1. **`async fn` / `await`** — language sugar built into the compiler
   (shipped v0.8.70). It makes a method return a *future* and lets you
   `await` it with a straight-line syntax.
2. **`amalgame-async`** — the runtime package (`Amalgame.Async`) that the
   sugar lowers onto: fibers, channels, and a cooperative scheduler. You
   can also use it directly.

> **Key fact, don't skip it.** `async fn` / `await` is *not* standalone —
> it lowers to `Amalgame.Async` runtime calls (the scheduler + channels).
> A program that uses `async`/`await` **must depend on the `amalgame-async`
> package** so those symbols link. Add it first:
>
> ```bash
> amc package add async
> ```
>
> Without it you'll see `implicit declaration of 'Amalgame_Async_*'`
> warnings at link time and **no binary is produced**.

This is cooperative (single-threaded fiber) concurrency: fibers yield to
each other at `await` / channel / sleep points. It is not OS-thread
parallelism — for that, see the `amalgame-threading` package.

---

## `async fn` / `await`

Mark a method `async fn`. It returns a *future* immediately; `await`
unwraps the eventual result, typed as the method's declared return type:

```amalgame
import Amalgame.Collections
import Amalgame.String

public class Worker {
    public async fn Compute(int n): int {
        return n * 2
    }
}

public class Program {
    public static void Main(string[] args) {
        let w = new Worker()
        let f = w.Compute(21)      // returns a future immediately
        let r: int = await f       // awaits the result → 42
        Console.WriteLine("result = " + String_FromInt(r))
    }
}
```

```bash
amc package add async              # required — the sugar links against it
amc build main.am -o main
./main                             # result = 42
```

Under the hood (v0.8.70), an `async fn Foo` lowers to three C functions:
the real body, a fiber trampoline that sends the result into a future
channel, and a public wrapper (keeping the normal name) that spawns the
fiber and returns the future. So a call site stays an ordinary call, and
`await` becomes a channel receive unboxed to the return type. You don't
write any of that — but knowing it explains why the runtime package is
mandatory.

> `async fn` and `fn` differ only by the `async` keyword; the `fn` after
> `async` is itself optional (`async Compute(int n): int` parses too).

---

## The runtime directly — fibers, channels, scheduler

The `amalgame-async` package (`import Amalgame.Async`, class `Async`)
exposes a **flat** API — `Async.FiberSpawn`, `Async.ChannelNew`, etc.
(not `Fiber.Spawn`). Spawn fibers, wire them with a channel, then pump
the scheduler. This producer/consumer is from the package's own README:

```amalgame
import Amalgame.Async

public class Program {
    public static void Main(string[] args) {
        let ch = Async.ChannelNew(8)          // bounded FIFO, capacity 8

        let producer = (_x: int) => {
            var i: int = 0
            while (i < 5) {
                Async.ChannelSend(ch, 100 + i)  // parks if full
                i = i + 1
            }
            Async.ChannelClose(ch)              // wakes parked receivers
            return 0
        }

        let consumer = (_x: int) => {
            while (true) {
                let v: int = Async.ChannelReceive(ch)   // parks if empty
                if (v == 0 && Async.ChannelIsClosed(ch)) { break }
                Console.WriteLine("got " + String_FromInt(v))
            }
            return 0
        }

        Async.FiberSpawn(producer, 0)
        Async.FiberSpawn(consumer, 0)
        Async.SchedulerRun()                  // pump until all fibers finish
    }
}
```

### API at a glance (v0.2)

**Fibers**

| Call | Returns | Notes |
|------|---------|-------|
| `Async.FiberSpawn(closure, arg)` | `AmalgameFiber*` | queues a 1-arg closure; runs when the scheduler pumps it |
| `Async.FiberYield()` | `void` | cooperative yield — caller moves to the ready-queue tail |
| `Async.FiberSleep(ms)` | `void` | parks caller; scheduler runs others meanwhile |
| `Async.FiberCurrentId()` | `int` | id of running fiber; 0 outside any fiber |

**Channels** — bounded FIFO, scheduler-aware

| Call | Returns | Notes |
|------|---------|-------|
| `Async.ChannelNew(capacity)` | `AmalgameAsyncChannel*` | capacity ≥ 1 |
| `Async.ChannelSend(ch, value)` | `bool` | **parks if full**; false if closed |
| `Async.ChannelReceive(ch)` | `int` | **parks if empty**; **returns 0** if closed + empty |
| `Async.ChannelTrySend(ch, value)` | `bool` | non-blocking; false if full/closed |
| `Async.ChannelTryReceive(ch)` | `int` | non-blocking; 0 if empty |
| `Async.ChannelClose(ch)` | `void` | wakes every parked fiber |
| `Async.ChannelIsClosed(ch)` | `bool` | |
| `Async.ChannelCount(ch)` / `Async.ChannelCapacity(ch)` | `int` | buffered count / capacity |

**Scheduler**

| Call | Returns | Notes |
|------|---------|-------|
| `Async.SchedulerRun()` | `void` | pump until ready + sleeping + waiting + fd-wait queues are all empty |
| `Async.SchedulerRunUntil(ms)` | `void` | same, but stop after `ms` |
| `Async.SchedulerPending()` | `int` | fibers still alive |

**Non-blocking I/O** (Linux, epoll-backed)

| Call | Returns | Notes |
|------|---------|-------|
| `Async.WaitFdReadable(fd, timeout_ms)` | `bool` | parks until `fd` readable; negative timeout = forever. **Must be inside a fiber.** |
| `Async.WaitFdWritable(fd, timeout_ms)` | `bool` | same for writability |
| `Async.MakeNonBlocking(fd)` | `bool` | sets `O_NONBLOCK` — the mandatory companion to `WaitFd*` |

---

## Gotchas

- **`async`/`await` requires the `amalgame-async` package.** The sugar is
  in the compiler, but it *lowers to* runtime calls — `amc package add
  async` or you get unresolved `Amalgame_Async_*` at link and no binary.
- **`ChannelReceive` returns `0` as the closed-and-empty sentinel.** If
  `0` is a legal value in your stream, pair the receive with
  `Async.ChannelIsClosed(ch)` (as the consumer above does) to tell "real
  zero" from "channel done".
- **Nothing runs until `SchedulerRun()`.** `FiberSpawn` only queues; the
  scheduler pump is what executes fibers. A program that spawns but never
  pumps does nothing.
- **Cooperative, not parallel.** A fiber hogs the thread until it hits an
  `await` / channel op / `FiberYield` / `FiberSleep`. CPU-bound loops
  should `FiberYield()` periodically. For real parallelism use
  `amalgame-threading`.
- **`WaitFd*` is fiber-only and Linux/epoll.** Calling it outside a fiber,
  or relying on it on a non-Linux target, won't behave as on Linux.

For OS threads and mutexes, see the `amalgame-threading` package. For the
package workflow (`add` / `update` / lockfile), see
[12-packages.md](12-packages.md).
