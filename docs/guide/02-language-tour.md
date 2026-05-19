# 2 · Language tour

A read-through of every language feature with runnable snippets. If
you're impatient, skim this chapter once and refer back when needed.

## Conventions

- Every file starts with `namespace Foo.Bar`. C symbols emitted for
  the file are prefixed with `Foo_Bar_`.
- The runtime entry point is a `Program.Main(string[] args)` method
  on a `public class Program`. If absent, the file compiles as a
  library (no `main()`).
- `import Amalgame.X` is currently informational — the resolver knows
  the stdlib globally, so imports are not strictly required to use
  `Console`, `File`, etc.
- Comments: `// line` and `/* block */`.

## Variables and primitives

```kotlin
let x = 42                  // immutable binding (recommended)
var y = 3.14                // mutable binding
let n: int = 7              // explicit type annotation
let s: string = "hello"
let b: bool = true
let d: double = 1.5
```

Primitive types: `int` (i64), `float` / `double`, `bool`, `string`
(C `char*`), `void`. Array types use `T[]` (e.g. `string[]`).
Nullable types use `T?` (e.g. `User?`).

## Operators

```kotlin
// Arithmetic
let a = 1 + 2 - 3 * 4 / 5 % 6
// Comparison
let cmp = a == 0 || a != 0 && a < 10
// Bitwise
let bits = (a & 0xff) | (a >> 4) ^ ~a << 1
// Compound assigns
var n = 0
n += 1
n -= 1
n *= 2
n /= 2
n %= 5
n &= 0xff
n |= 0x10
n ^= 0xa
n <<= 2
n >>= 1
// Range (used in for-in and match)
for i in 0..10 { /* … */ }
// Pipeline (left-to-right composition; sugar over a.f().g())
// e.g. x |> double |> Console.WriteLine    (when each accepts the previous result)
```

Precedence is the standard C/Java set; use parentheses when in doubt.
Unary `!`, `-`, `~` exist; `!` requires a bool operand; `~` is the
bitwise complement.

## Strings

```kotlin
let plain = "hello"
let escaped = "tab\there\nnewline"
let hex = "\x1b[31mred\x1b[0m"          // \xHH = one byte
let unicode = "héllo 中 €"      // \uHHHH = UTF-8 codepoint
let multi = """
line 1
line 2 with "quotes" and \n untouched
"""
let interp = "x={n} obj.f={obj.field} call={Math.Sqrt(16.0)}"
```

Interpolation supports simple variables, `obj.field` (where `obj` is
a known local), `this.field`, and method/static calls (e.g.
`{Math.Sqrt(x)}`, `{String.Length(s)}`). For deeper expressions like
`{Math.Sqrt(obj.field)}` use a let-binding first:

```kotlin
let v = obj.field
"sqrt = {Math.Sqrt(v)}"
```

## Classes

```kotlin
public class User {
    // Fields with explicit types
    public Name: string
    public Age:  int
    private Score: int

    // Constructor (method named after the class)
    public User(string name, int age) {
        this.Name  = name
        this.Age   = age
        this.Score = 0
    }

    // Methods
    public string Greet() {
        return "Hello, " + this.Name + "!"
    }

    public void AddScore(int points) {
        this.Score = this.Score + points
    }
}
```

Use:

```kotlin
let u = new User("Alice", 30)
Console.WriteLine(u.Greet())
u.AddScore(10)
```

### Static methods

```kotlin
public class Calc {
    public static int Add(int a, int b) { return a + b }
    public static int Mul(int a, int b) { return a * b }
}

let n = Calc.Add(2, 3)
```

### Inheritance

```kotlin
public class Animal {
    public Name: string
    public Animal(string n) { this.Name = n }
    public string Speak() { return "..." }
}

public class Cat extends Animal {
    public Cat(string n) {
        // No `super()` yet — assign fields directly when needed.
        this.Name = n
    }
    public string Speak() { return "meow" }
}
```

### Data classes / records

```kotlin
data class Point(float X, float Y)
record Line(Point A, Point B)
```

Both expand into a class with public fields + a public constructor in
field declaration order. Useful for plain value carriers.

## Enums

Simple enum:

```kotlin
public enum Direction {
    North
    East
    South
    West
}

let d = Direction.North
match d {
    Direction.North => Console.WriteLine("⬆"),
    Direction.East  => Console.WriteLine("➡"),
    Direction.South => Console.WriteLine("⬇"),
    Direction.West  => Console.WriteLine("⬅")
}
```

Algebraic enum (tagged union):

```kotlin
public enum Shape {
    Circle(int)
    Rect(int, int)
    Triangle(int, int, int)
}

let s = Shape.Rect(4, 3)
match s {
    Circle(r)        => Console.WriteLine("circle r={String.FromInt(r)}"),
    Rect(w, h)       => Console.WriteLine("{String.FromInt(w * h)}"),
    Triangle(a,b,c)  => Console.WriteLine("triangle")
}
```

> Variant payloads in the declaration are **types only**, not named
> parameters. Names are introduced in the match arm via the binder
> pattern: `Circle(r) => …`.

## Interfaces

```kotlin
public interface Shape {
    int Area()
    string Name()
}

public class Square implements Shape {
    public Side: int
    public Square(int s) { this.Side = s }
    public int Area() { return this.Side * this.Side }
    public string Name() { return "square" }
}
```

Since v0.3.5, interfaces support generic params and the
TypeChecker enforces the contract:

```kotlin
public interface IComparable<T> {
    Compare(T other) -> int
}

public class IntBox implements IComparable<int> {
    public Value: int
    public IntBox(int v) { this.Value = v }
    public int Compare(int other) { return this.Value - other }
}
```

The check substitutes `T → int` at the `implements` site, then
verifies that `IntBox` has a `Compare(int) -> int` method. A
mismatch produces a precise diagnostic naming the offending
method/parameter and its expected vs got type.

Generics are still stripped at the C level (`T → void*`); the
new check is a *static contract* layered on top of the existing
duck-typed dispatch — there's no vtable yet.

## Control flow

```kotlin
if (x > 0) {
    Console.WriteLine("positive")
} else if (x < 0) {
    Console.WriteLine("negative")
} else {
    Console.WriteLine("zero")
}

while (n > 0) {
    n = n - 1
}

for i in 0..10 { /* exclusive end: 0..9 */ }
for c in characters { /* iterate a List<T> */ }

break
continue
```

### Block scope (`{ … }`, v0.8.35+)

A bare `{ … }` at statement position introduces its own scope.
Useful to give long methods visually-separated sections where
each section can reuse local names without colliding at the C
level:

```kotlin
public static void Run() {
    {
        let r0: int = 1
        let r1: int = 2
        // ... section A uses r0, r1 ...
    }
    {
        let r0: int = 10   // OK — fresh scope, no shadowing warning
        let r1: int = 20
        // ... section B reuses the same names ...
    }
}
```

Each block emits a matching C `{ … }` wrapper, so `let` inside
one block is invisible to the next. Mirrors C / C++ / Java
scoping; required only when you want a fresh slot for the same
name. `if` / `while` / `for` / `match` arm bodies already get
their own scopes automatically.

### Guard clauses

```kotlin
public static int Clamp(int x, int lo, int hi) {
    guard x > lo else { return lo }
    guard x < hi else { return hi }
    return x
}
```

Reads top-to-bottom: if the condition is false, run the `else` block
(typically a `return` / `throw` / `break` / `continue`). Equivalent
to `if (!(cond)) { exit }`.

### Pattern matching

```kotlin
match n {
    0           => Console.WriteLine("zero"),
    x if x < 0  => Console.WriteLine("negative"),         // arm guard
    1..9        => Console.WriteLine("small"),            // range
    10..99      => Console.WriteLine("medium"),
    _           => Console.WriteLine("else")              // wildcard
}
```

Arm bodies are **statements**, not expressions. To compute a value,
use early-return inside arms or assign in each branch:

```kotlin
public string Classify(int n) {
    if (n == 0) { return "zero" }
    if (n < 0)  { return "negative" }
    return "positive"
}
```

For algebraic enums, patterns destructure the payload:

```kotlin
match shape {
    Circle(r)  => useRadius(r),
    Rect(w, h) => useDims(w, h)
}
```

### Try / catch / throw

```kotlin
try {
    risky()
} catch (e) {
    Console.WriteError("caught")
} finally {
    cleanup()
}
```

The implementation is `setjmp`/`longjmp`-based at the C level — no
stack unwinding cost when no throw fires.

## Tuples

```kotlin
public class Math2 {
    public static (int, int) DivMod(int a, int b) {
        return (a / b, a % b)
    }
}

let (q, r) = Math2.DivMod(17, 5)   // q = 3, r = 2
```

## Lambdas

```kotlin
// Single-param, expression body
let double = x => x * 2

// Multi-param (v0.3.5)
let add  = (x, y) => x + y
let pick = (a, b, c) => a + b + c

// Block body (v0.3.5) — let-bindings + explicit return
let plus3 = x => {
    let doubled = x * 2
    return doubled + 3
}

// Closures capture enclosing locals by value (v0.3.4)
let n = 100
let shift = x => x + n           // captures n
```

Lambdas compile to a top-level C function plus an
`AmalgameClosure { fn, env }` value; captured locals are
snapshotted into a heap-allocated env at the creation site,
not by textual substitution. The runtime exposes
`Closure_call1` / `_call2` / `_call3` for arities 1–3.

Higher-order `List<T>` methods accept a lambda directly (since
v0.3.6 — see [04-stdlib.md](04-stdlib.md#listt) for the full
list):

```kotlin
let nums = new List<int>()
nums.Add(1) ; nums.Add(2) ; nums.Add(3) ; nums.Add(4)
let big   = nums.Filter(x => x > 2)              // [3, 4]
let times = nums.Map(x => x * 10)                // [10, 20, 30, 40]
let total = nums.Reduce(0, (acc, x) => acc + x)  // 10
```

### First-class functions (`Closure` type, v0.8.30+)

Use the `Closure` type to declare function-valued **class fields**,
**method parameters**, and **collection elements**. Lambdas literal-
or named-bound to a `Closure` slot are usable like any other value,
and a stored closure is invoked with the same `field(args)` syntax
as a method call — the compiler dispatches through
`AmalgameClosure_callN` under the hood.

```kotlin
public class Route {
    public Path:    string
    public Handler: Closure                  // ← first-class fn field

    public Route(string path, Closure handler) {
        this.Path    = path
        this.Handler = handler
    }

    public int Run(int x) {
        return this.Handler(x)                // ← invoke stored closure
    }
}

let r = new Route("/double", x => x * 2)
r.Run(21)                                    // → 42

// Closures in a collection.
let routes = new List<Route>()
routes.Add(new Route("/inc", x => x + 1))
let r0: Route = routes.Get(0)
r0.Run(99)                                   // → 100
```

This is the foundation of callback APIs (handler registries, event
emitters, middleware chains, etc.).

Arities 1, 2 and 3 are supported. Closures of arity 0 or 4+ require
a future cgen extension.

### Typed closures (`Closure<A, R>`, v0.8.35+)

Annotate the closure shape to let the compiler emit typed casts
on each argument and the return value. The last comma-separated
parameter is **R** (the return type); the rest are argument types.
Same convention as Kotlin's `(A) -> R`, just spelled inside `<…>`.

```kotlin
// Arity 1: scalar in, scalar out.
let f1: Closure<int, int> = x => x * 2
let r1: int = f1(21)                          // 42

// Arity 2: two scalars in, one out.
let f2: Closure<int, int, int> = (a, b) => a + b
let r2: int = f2(10, 20)                      // 30

// Pointer argument + pointer return.
let f3: Closure<User, string> = u => u.Name
let r3: string = f3(new User("alice", 30))    // "alice"

// Typed closure as a class field — call result is pointer-typed.
public class Server {
    public Handler: Closure<Conn, Conn>
    public Server(h: Closure<Conn, Conn>) { this.Handler = h }
    public int Dispatch(c: Conn) {
        let r: Conn = this.Handler(c)         // ← typed cast, no warning
        return r.Id
    }
}
```

The runtime ABI is unchanged — `Closure<…>` still lowers to
`AmalgameClosure*`. The annotation is purely a compile-time hint
that informs:

  - **Lambda body emission** — `u => u.Name` over `Closure<User, string>`
    types `u` as `User*` (was `i64 u = (i64)(intptr_t)__arg0;` for bare
    `Closure`).
  - **Call site casts** — both IDENTIFIER (`f(x)`) and MEMBER
    (`this.Handler(c)`) closure calls cast the result to R instead of
    going through `i64 + intptr_t`.
  - **Typechecker** — `c(x)` where c is `Closure<…, R>` surfaces R
    as the call result type, so `let r: R = c(x)` typechecks without
    falling back to the `?` wildcard.

User code that previously needed `-Wno-int-conversion` to silence
"pointer from integer" noise around closure boxing can drop those
flags once the relevant `Closure` fields/params/locals carry their
typed shape.

**Direct lambda args (v0.8.36+).** Lambda-param inference also
fires when the lambda is passed *directly* to a ctor or method
whose parameter is declared `Closure<A, R>` — the resolver
walks the target signature and pushes A into the lambda's
first PARAM. All three forms below produce the same typed
unpack with no `-Wint-conversion` noise:

```kotlin
// Direct ctor arg — c inferred as WebContext from Route's signature.
new Route(c => Program.handle(c))

// Method arg — same inference path.
reg.Register("name", c => Program.handle(c))

// Bind to a typed local — c inferred from the local annotation.
let h: Closure<WebContext, HttpResponse> = c => Program.handle(c)
new Route(h)

// Or type the lambda param explicitly — overrides inference.
new Route((c: WebContext) => Program.handle(c))
```

## List literals and comprehensions

Square-bracket syntax builds a `List<T>` inline. Comma-separated
expressions are literals; the `for x in iter` form is a
comprehension that maps and optionally filters:

```kotlin
// Empty list — element type comes from the target slot.
let empty: List<int> = []

// Literal with explicit elements.
let names: List<string> = ["alpha", "beta", "gamma"]

// Mixed with computed values and trailing comma.
let n = 7
let nums: List<int> = [
    n,
    n + 1,
    n * 2,
]

// Function argument.
TakeList(["x", "y", "z"])

// List comprehension — map a range.
let squares: List<int> = [i * i for i in 0..10]
// → [0, 1, 4, 9, 16, 25, 36, 49, 64, 81]

// Comprehension with filter.
let evens: List<int> = [i for i in 0..20 if i % 2 == 0]

// Iterate an existing list — `x` is bound element-by-element.
let upper: List<string> = [String_ToUpper(name) for name in names]
```

Both forms lower to the same shape — a GCC compound-statement
expression that allocates a fresh `AmalgameList*` and pushes
each boxed element. Literals are equivalent to the longhand
`new List<T>(); list.Add(...)` series — use whichever reads
better at the call site.

The comprehension supports two iterable shapes:
- a numeric range (`lo..hi`) — emits a counted `i64` loop;
- any `List<T>` value — emits an indexed loop over the list.

Same-name nesting (`[ [j for j in 0..i] for i in 0..3 ]`)
isn't supported yet — pick distinct loop variables when nesting.

**Spread operator** `...src` (v0.8.36+). Inside a list literal,
each `...src` item splices `src`'s elements at its position:

```kotlin
let a: List<int> = [1, 2, 3]
let b: List<int> = [4, 5]
let c: List<int> = [...a, ...b, 99]
// → [1, 2, 3, 4, 5, 99]  (Count() == 6)

// Mix spreads with literal elements at any position.
let names = ["alice", "bob"]
let all   = ["zero", ...names, "tail"]
// → ["zero", "alice", "bob", "tail"]

// Spread the result of a method call — source is evaluated once
// (bound to a synthetic `__sp_<i>` local in the generated C).
let doubled = [...nums.Map(x => x * 2), 999]
```

Variadic call sites (`f(...args)`) and variadic param definitions
(`(...xs: T[])`) need a new function-signature grammar and are
not yet supported.

**Nested generics** `List<List<T>>` (v0.8.36+). Chained
`.Get(i).Get(j)` peels one generic layer per hop, so each level
gets a typed cast instead of falling through to `void*`:

```kotlin
let rows: List<List<string>> = …
let s: string = rows.Get(0).Get(0)   // proper code_string cast

let cube: List<List<List<int>>> = …
let v: int = cube.Get(0).Get(0).Get(0)  // unbox via intptr_t
```

Works up to arbitrary depth, both for `List<…>` and the value-
half of `Map<K, …>`. The raw element type is tracked at every
`let` annotation + every typed method return.

**Higher-order methods on `List<Class>` (v0.8.35+).** Both
`xs.Filter(x => x > 0)` and `xs.Map(x => x.Name)` over a
`List<User>` now work end-to-end. The resolver patches the
lambda's first param from the receiver's element type, the cgen
emits a typed unpack (`User* x = (User*)__arg0;`) and the
consumer-side `for u in filterResult` lowers with the right
element cast (no manual `as User*` needed). Untyped lists
(`new List<int>()` with no annotation) still fall back to
`void*` and require explicit casts.

## Null safety

`T?` declares a nullable. `?.` short-circuits to `null` when the
receiver is null:

```kotlin
let user: User? = null
let name = user?.Name              // null
let len  = user?.GetNameLength()   // null
if (name == null) {
    Console.WriteLine("anonymous")
}
```

`??` is null-coalescing (returns the right operand when the left
is null):

```kotlin
let display = user?.Name ?? "anonymous"
```

## Decorators

```kotlin
public class Math2 {
    @inline
    public static int Square(int x) { return x * x }

    @deprecated
    public static int Old(int x) { return x }
}
```

| Decorator     | Effect on emitted C                                       |
| ------------- | --------------------------------------------------------- |
| `@inline`     | adds `inline` to the function definition                  |
| `@deprecated` | adds `__attribute__((deprecated))` to the prototype       |

Other decorators are accepted by the parser but currently no-ops.

## Named arguments

```kotlin
let p = new Person(name: "Bastien", age: 31)
Math2.Clamp(x: 50, lo: 0, hi: 10)
```

> Names are **documentation-only** at the call site for now: values
> are passed in source order regardless of the names. Tracked in
> [ROADMAP_COMPLET.md](../../ROADMAP_COMPLET.md).

## Library mode

Files without a `Program.Main` compile as libraries automatically.
Use `--lib` to force library mode even when a `Program.Main` exists.

```bash
./amc --lib mylib.am -o mylib
gcc -Iruntime -c mylib.c -o mylib.o
# Link mylib.o into a host program from C, Amalgame, etc.
```

## Reading diagnostics

Errors come with a source snippet and a caret, like rustc:

```
error[typechecker]: Cannot assign 'string' to 'n' of type 'int'
  --> /tmp/test.am:19:13
   |
19 |         let n: int = p.Name
   |             ^

error[resolver]: Unknown symbol 'someThing'
  --> /tmp/test.am:4:9
  |
4 |         someThing.x()
  |         ^
```

The two passes are:

- **Resolver** — scope and name resolution (unknown identifiers,
  unbound types, immutable rebinding).
- **Type-checker** — assignability, return types, member-access types
  via the resolver's MemberTable.

Both errors are non-fatal individually; the compiler still emits a
`.c` file when possible (so you see all errors at once), but exits
non-zero when any error was reported.

## What's not in the language yet

- Generic type inference (`let xs = new List<int>()` does compile but
  the element type `int` isn't propagated through every method call —
  see [04-stdlib.md](04-stdlib.md#listt) for which methods preserve
  the element type).
- Spread / rest (`...args`), async / await.
- Same-name list-comprehension nesting (use distinct loop vars).

See [ROADMAP_COMPLET.md](../../ROADMAP_COMPLET.md) for the full
backlog.
