# 5 · Runtime & C interop

## Memory model

Amalgame uses **bdwgc** (Boehm-Demers-Weiser garbage collector) for
all dynamic allocations. The GC is a conservative mark-sweep collector
that lives in `runtime/_runtime.h` (`#include <gc.h>`), and is
initialised by the generated `int main()`:

```c
int main(int argc, char** argv) {
    GC_INIT();
    code_runtime_init_args(argc, argv);
    App_Program_Main((code_string*)argv);
    return code_exit_code;
}
```

Implications:

- **No `free`/`destroy` calls.** All allocations go through `GC_MALLOC`.
  The GC reclaims unreachable memory automatically.
- **No deterministic destructors yet.** If you need `try { } finally`,
  use the explicit `try`/`finally` form — files / sockets are not auto-closed.
- **Conservative scanning** — values that look like pointers but aren't
  may keep memory alive. Don't store secrets in long-lived large
  buffers if you care.
- **Allocation cost** is amortised — `GC_MALLOC` is in the few-hundred-ns
  range on Linux.

## Type mapping (Amalgame → C)

| Amalgame             | C                                                |
| -------------------- | ------------------------------------------------ |
| `int`                | `i64` (signed 64-bit)                            |
| `float`              | `double`                                         |
| `bool`               | `code_bool` (alias for `bool`)                   |
| `string`             | `code_string` (alias for `char*`)                |
| `void`               | `void`                                           |
| `T?`                 | `T*` (nullable pointer)                          |
| `T[]`                | `T*` (array decays to pointer; size from caller) |
| `List<T>`            | `AmalgameList*`                                  |
| `Map<K,V>`           | `AmalgameMap*`                                   |
| `Set<T>`             | `AmalgameSet*`                                   |
| user `class Foo`     | `Foo*` (always heap-allocated)                   |
| user `enum Foo`      | `Foo` (C enum) — non-pointer                     |
| algebraic `enum Foo` | `Foo` (tagged union struct) — value type         |

Generic parameters (`T`) erase to `void*`. Primitive values (`int`,
`bool`) are passed through `(void*)(intptr_t)` casts when crossing
generic interfaces — there's no boxing object.

C symbol names are derived from `Namespace.Class.Method` →
`Namespace_Class_Method`. Static methods and instance methods share
the same naming scheme; instance methods take `self` as the first
parameter.

## Closures and higher-order calls

A lambda compiles to a top-level C function plus an
`AmalgameClosure { fn, env }` value. `_runtime.h` exposes the
struct and three call wrappers:

```c
AmalgameClosure_call1(c, arg)
AmalgameClosure_call2(c, a, b)
AmalgameClosure_call3(c, a, b, d)
```

Higher-order `List<T>` methods (`Filter`, `Map`, `Reduce`,
`ForEach`, `Any`, `All`, `CountIf`) are implemented as static
inlines in `_runtime.h` that dispatch through `_call1` (or
`_call2` for `Reduce`'s `(acc, x) → acc` reducer). At the
Amalgame call site, the CGen emits a GCC compound-statement-
expression that allocates the env, copies captured locals in,
and yields a fresh closure:

```c
AmalgameList_filter(xs, ({
    LamEnv_0* __env_0 = (LamEnv_0*)code_alloc(sizeof(LamEnv_0));
    __env_0->_n = n;             /* one line per capture */
    AmalgameClosure_new((void*)lam_0_fn, __env_0);
}))
```

Lambda items / args / results are all `void*` boxed at the C
ABI boundary; the call site unboxes via `(i64)(intptr_t)…` for
the i64 shape. Non-int signatures (e.g. a lambda returning a
`code_string`) need the lambda-typing layer that's tracked for
the next release.

## Calling Amalgame from C

Compile your library with `--lib` and link the `.o`:

```kotlin
// greeter.am
namespace MyLib

public class Greeter {
    public Name: string
    public Greeter(string name) { this.Name = name }
    public string Hello() { return "Hello, " + this.Name + "!" }
}
```

```bash
amc --lib greeter.am -o greeter
gcc -Iruntime -c greeter.c -o greeter.o
```

C consumer:

```c
#include "_runtime.h"
#include "Amalgame_String.h"

typedef struct _MyLib_Greeter MyLib_Greeter;
extern MyLib_Greeter* MyLib_Greeter_new(code_string name);
extern code_string MyLib_Greeter_Hello(MyLib_Greeter* self);

int main(void) {
    code_runtime_init();
    MyLib_Greeter* g = MyLib_Greeter_new("World");
    printf("%s\n", MyLib_Greeter_Hello(g));
    return 0;
}
```

```bash
gcc -Iruntime consumer.c greeter.o -lgc -lm -o app
./app
# Hello, World!
```

A live e2e test is at `tests/samples/lib_e2e.am` /
`lib_e2e_consumer.c` and runs as part of `tests/core_bundle/core_test.am`
(invoke with `./amc test ./tests/core_bundle/`).

## Calling C from Amalgame

There's no FFI keyword today. Two pragmatic patterns:

### 1. Add the function to the runtime header

Drop your declaration in a header under `runtime/` and declare it as
a builtin in the resolver:

```c
// runtime/Amalgame_String.h
static inline i64 String_DamerauLevenshtein(code_string a, code_string b) {
    /* … your impl … */
}
```

```kotlin
// src/resolver/resolver.am — RegisterBuiltins()
this.DeclareGlobal("String_DamerauLevenshtein", "int", false)
```

You can now call `String.DamerauLevenshtein(a, b)` from any `.am` file.

This is exactly how the stdlib works internally.

### 2. Inline a C call site via a method body

Less elegant, but works for one-off bindings: put the helper in the
runtime header, then write a thin Amalgame wrapper class that just
forwards. The CGen will emit normal C calls.

### 3. Inline-C blocks — `@c { … }` (v0.7.4+)

Since v0.7.4 (project G) Amalgame has a balisé inline-C block that
lets a method body drop straight into C without a runtime-header
detour. The body bytes flow through the lexer opaquely until the
matching `}`, the resolver / typechecker / linter treat the node
as opaque, and the cgen splices the source verbatim inside a
compound statement (`{ … }`) so any locals declared in the block
stay scoped to it.

```kotlin
namespace App

public class CTools {
    public static int CLen(string s) {
        @c {
            return (int) strlen(s);
        }
    }

    public static int Doubled(int x) {
        @c {
            int local = x;
            return local + local;
        }
    }
}
```

Two companion **file-scope directives** are recognised at the top
of any `.am` file, before / between class declarations:

- **`@c_include "<header.h>"`** — emits the corresponding C
  `#include` at the top of the generated `.c`. Angle form when
  the argument starts with `<`, quoted form otherwise:
  ```kotlin
  @c_include "<ctype.h>"      // → #include <ctype.h>
  @c_include "my_local.h"     // → #include "my_local.h"
  ```
  The runtime prelude already pulls in `<stdio.h>` / `<stdlib.h>` /
  `<string.h>` / `<stdint.h>` / `<math.h>` / `<gc.h>` — only
  reach for `@c_include` when you need something the prelude
  doesn't bring in.

- **`@c_link "name"`** — surfaces as a `/* link: -lname */`
  comment in the emitted `.c`. The MVP doesn't thread it into
  `amc test`'s internal gcc step (you still pass `-l<name>`
  yourself for `amc -o` workflows); the comment is there so
  `grep '^/\* link:' yourfile.c` gives you the full link line.

**Body semantics:**

- Every Amalgame parameter in scope is visible inside the block
  with its C representation (`code_string` / `i64` / struct pointer).
- The C `return` statement returns from the enclosing Amalgame
  method (because the splice lives inside the method body, not a
  nested function). Use it the same way you'd use a regular
  Amalgame `return`.
- The body must contain valid C — the Amalgame frontend doesn't
  parse inside `{ … }`. Strings, char literals, single- and
  block-comments are recognised by the lexer's brace-counting so
  a `}` inside `"abc}def"` / `'}'` / `/* … */` doesn't close the
  block early.

**Sandbox / safety:** there isn't one. Inline-C can crash the
program, leak memory, and violate GC invariants. Treat `@c { … }`
the same way you'd treat Rust's `unsafe { … }` — reach for it
when you have to, document why, and isolate the unsafe surface
behind a typed Amalgame wrapper.

**File-scope `@c { … }` blocks (v0.7.5+).** A second `@c { … }`
form lives at the top level of a `.am` file (between/before class
declarations). The body is spliced into the emitted `.c` outside
any function, which lets a module:

- declare process-wide state globals (mutex, fd, init flag);
- pull in libc / OS headers via raw `#include` lines (instead of
  `@c_include`);
- define helper static functions that several class methods then
  share.

```amalgame
namespace Amalgame.Logging

@c {
    #include <stdio.h>
    static int g_min_level = 1;
    static FILE *g_sink = NULL;
}

public class Log {
    public static void SetMinLevel(s: string) {
        @c { g_min_level = parse_level(s); }
    }
    // … etc.
}
```

This was the enabling feature for the v0.7.6 stdlib purity arc:
runtime headers shrunk from 18 to 9 as eight modules — Logging,
DateTime, FileWatcher, Service, Random, Crypto, BuildInfo,
Math + Math.Vec — migrated to pure-AM modules with `@c { … }`
blocks for the OS-bound primitives. v0.7.7 then moved those same
modules out of the bundled stdlib into external packages on
`amalgame-lang/` — see chapter 4 for the table.

> `@out = expr;` from the original v0.7.4 proposal was **dropped**.
> Bodies use plain C `return …;` against the method's declared
> return type, which is cleaner and avoids a hidden boundary
> rewrite.

## ABI notes

- The compiler does **not** mangle types into symbol names — only the
  namespace path. So overloads aren't supported (two methods with the
  same `Class.Name` but different signatures would clash at link time).
- Public/private visibility maps to C `static` (private) vs nothing
  (public). Private methods can't be called from another translation
  unit.
- Decorators map to C attributes:
  - `@inline` → `inline` keyword
  - `@deprecated` → `__attribute__((deprecated))`

## Strings: zero hidden cost

`code_string` is `char*`. Concatenation goes through
`code_string_concat(a, b)` which allocates a fresh GC buffer of
`strlen(a) + strlen(b) + 1`. Passing a literal is free.

There's no length cache — `String.Length(s)` is `strlen(s)` — so
hot loops that ask for the length should hoist it:

```kotlin
let len = String.Length(text)         // O(strlen)
for i in 0..len { /* … */ }            // body is O(1) per iteration
```

## Threading & async — external packages

The bundled runtime is single-threaded by default; the GC is
configured for the main thread only. Two ecosystem packages take
care of concurrency so you don't have to hand-roll `@c { … }` blocks
around pthread / ucontext:

- **[`amalgame-threading`](https://github.com/amalgame-lang/amalgame-threading)**
  — POSIX threads + Mutex + bounded Channel + `Thread.Spawn /
  Join / Sleep`. Every spawned thread is registered with bdwgc
  (via `GC_pthread_create`) so locals stay scannable. Right tool
  for CPU-bound work that benefits from multi-core.
- **[`amalgame-async`](https://github.com/amalgame-lang/amalgame-async)**
  — stackful coroutines on POSIX ucontext, single-threaded
  round-robin scheduler with parking channels, epoll-driven
  `WaitFdReadable` / `WaitFdWritable` (Linux), cooperative
  `FiberCancel` / `IsCancelled`, and an ergonomic
  `WithTimeout(closure, arg, ms)` helper. Right tool for
  I/O-bound work — see the bench in
  [docs/proposals/amalgame-async.md](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amalgame-async.md)
  showing 1.5×–9× throughput vs thread-per-conn on HTTP/1.1
  workloads.

They compose orthogonally (spawn N OS threads, run a scheduler in
each — M:N is planned for `amalgame-async` v0.4). If you fork or
spin POSIX threads from your own `@c { … }` blocks without going
through either package, you're on your own with libgc safety: the
`GC_pthread_create` wrapper is what keeps the collector aware of
new threads' stacks.
