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

```amalgame
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

```amalgame
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

```amalgame
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

```amalgame
let v = obj.field
"sqrt = {Math.Sqrt(v)}"
```

## Classes

```amalgame
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

```amalgame
let u = new User("Alice", 30)
Console.WriteLine(u.Greet())
u.AddScore(10)
```

### Static methods

```amalgame
public class Calc {
    public static int Add(int a, int b) { return a + b }
    public static int Mul(int a, int b) { return a * b }
}

let n = Calc.Add(2, 3)
```

### Inheritance

```amalgame
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

```amalgame
data class Point(float X, float Y)
record Line(Point A, Point B)
```

Both expand into a class with public fields + a public constructor in
field declaration order. Useful for plain value carriers.

## Enums

Simple enum:

```amalgame
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

```amalgame
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

```amalgame
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

Generics are stripped at the C level (`T → void*`), so today
`IComparable<T>` and similar are best avoided until generic
inference lands.

## Control flow

```amalgame
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

### Guard clauses

```amalgame
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

```amalgame
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

```amalgame
public string Classify(int n) {
    if (n == 0) { return "zero" }
    if (n < 0)  { return "negative" }
    return "positive"
}
```

For algebraic enums, patterns destructure the payload:

```amalgame
match shape {
    Circle(r)  => useRadius(r),
    Rect(w, h) => useDims(w, h)
}
```

### Try / catch / throw

```amalgame
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

```amalgame
public class Math2 {
    public static (int, int) DivMod(int a, int b) {
        return (a / b, a % b)
    }
}

let (q, r) = Math2.DivMod(17, 5)   // q = 3, r = 2
```

## Lambdas

```amalgame
let double = x => x * 2
let nums = new List<int>()
nums.Add(1) ; nums.Add(2) ; nums.Add(3)
// Pass to higher-order helpers (per stdlib)
```

Lambdas are simple — they don't capture surrounding variables yet.
Use static helpers or pass state explicitly when you need closure
behaviour.

## Null safety

`T?` declares a nullable. `?.` short-circuits to `null` when the
receiver is null:

```amalgame
let user: User? = null
let name = user?.Name              // null
let len  = user?.GetNameLength()   // null
if (name == null) {
    Console.WriteLine("anonymous")
}
```

`??` is null-coalescing (returns the right operand when the left
is null):

```amalgame
let display = user?.Name ?? "anonymous"
```

## Decorators

```amalgame
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

```amalgame
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

- Closures that **capture** surrounding variables (lambdas exist but
  capture is not implemented — use static helpers / explicit args).
- Generic type inference (`let xs = new List<int>()` does compile but
  the element type `int` isn't propagated through method calls).
- `match` as expression — arms run statements, not expressions.
- List comprehensions, spread `...args`, async/await.

See [ROADMAP_COMPLET.md](../../ROADMAP_COMPLET.md) for the full
backlog.
