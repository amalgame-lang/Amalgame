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

## Calling Amalgame from C

Compile your library with `--lib` and link the `.o`:

```amalgame
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
gcc -Iruntime consumer.c greeter.o -lgc -lm -lcurl -o app
./app
# Hello, World!
```

A live e2e test is at `tests/samples/lib_e2e.am` /
`lib_e2e_consumer.c` and runs as part of `tests/run_tests.sh`.

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

```amalgame
// src/resolver/resolver.am — RegisterBuiltins()
this.DeclareGlobal("String_DamerauLevenshtein", "int", false)
```

You can now call `String.DamerauLevenshtein(a, b)` from any `.am` file.

This is exactly how the stdlib works internally.

### 2. Inline a C call site via a method body

Less elegant, but works for one-off bindings: put the helper in the
runtime header, then write a thin Amalgame wrapper class that just
forwards. The CGen will emit normal C calls.

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

```amalgame
let len = String.Length(text)         // O(strlen)
for i in 0..len { /* … */ }            // body is O(1) per iteration
```

## Threading

Single-threaded. The GC is configured for the main thread only. If
you fork or use POSIX threads from the runtime headers, you're on
your own.
