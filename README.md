<div align="center">

# Amalgame

**A modern, self-hosted programming language that transpiles to C.**

[![CI](https://github.com/BastienMOUGET/Amalgame/actions/workflows/ci.yml/badge.svg)](https://github.com/BastienMOUGET/Amalgame/actions/workflows/ci.yml)
[![Self-hosted](https://img.shields.io/badge/compiler-self--hosted-success)](src/)
[![Tests](https://img.shields.io/badge/tests-127%2F127-brightgreen)](tests/)
[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)]()

```amalgame
public class Greeter {
    public Name: string
    public Greeter(string name) { this.Name = name }

    public string Hello() {
        guard String.Length(this.Name) > 0 else { return "Hello, stranger!" }
        return "Hello, {this.Name}!"
    }
}
```

</div>

---

## Why Amalgame?

- **🚀 Compile to C, run anywhere** — no VM, no GC pause hell, just fast native binaries via gcc.
- **🪶 Zero hidden cost** — what you write is what runs. Strings are `char*`, lists are arrays, GC is bdwgc.
- **🧬 Self-hosted compiler** — `amc` is written in Amalgame and compiles itself. The whole pipeline (lexer, parser, resolver, type-checker, code-gen) lives in [src/](src/).
- **✨ Modern syntax you actually want to use** — pattern matching with guards, null-safety with `?.`, string interpolation, decorators, generics, tuples, lambdas.
- **⚡ ~5s build** — full self-host rebuild including the bootstrap, in five seconds.
- **📦 Multi-OS** — Linux, macOS, Windows binaries built on every release tag.

---

## A Taste

```amalgame
namespace App
import Amalgame.IO

public class Calc {
    // @inline maps to GCC's `inline` hint.
    @inline
    public static int Square(int x) { return x * x }

    // Guard clauses: read top-to-bottom instead of nesting `if`.
    public static int Clamp(int x, int lo, int hi) {
        guard x > lo else { return lo }
        guard x < hi else { return hi }
        return x
    }
}

public class User {
    public Name: string
    public Age:  int
    public User(string name, int age) {
        this.Name = name
        this.Age  = age
    }
}

public enum Shape {
    Circle(int)
    Rect(int, int)
}

public class Program {
    public static void Main(string[] args) {
        // Named arguments + interpolation of object fields.
        let u = new User(name: "Bastien", age: 31)
        Console.WriteLine("Hello, {u.Name}!")
        let age = u.Age
        Console.WriteLine("You are {String.FromInt(age)}.")

        // Null-safe member access — no NPE, ever.
        let maybe: User? = null
        let label = maybe?.Name
        if (label == null) { Console.WriteLine("nobody") }

        // Pattern matching with arm guards, ranges, and binders.
        let n = 42
        match n {
            0           => Console.WriteLine("zero"),
            x if x < 0  => Console.WriteLine("negative"),
            1..9        => Console.WriteLine("small"),
            10..99      => Console.WriteLine("medium"),
            _           => Console.WriteLine("large")
        }

        // Triple-quoted multiline strings preserve newlines as-is.
        Console.WriteLine("""
        ┌──────────────────────┐
        │  Amalgame demo       │
        └──────────────────────┘
        """)

        // Algebraic enums + match destructuring.
        let s = Shape.Circle(5)
        match s {
            Circle(r)  => Console.WriteLine("circle r={String.FromInt(r)}"),
            Rect(w, h) => Console.WriteLine("rect")
        }
    }
}
```

> **Note** — match arms run statements; the language doesn't yet have
> match-as-expression. Use `Console.WriteLine` / early-return inside
> arms, or assign in each arm body. Tracked in
> [ROADMAP_COMPLET.md](ROADMAP_COMPLET.md).

---

## Get Started in 60 Seconds

### Linux

```bash
sudo apt install valac meson ninja-build gcc \
    libglib2.0-dev libgee-0.8-dev libgc-dev libcurl4-openssl-dev
git clone https://github.com/BastienMOUGET/Amalgame.git && cd Amalgame
./compile.sh         # build the Vala bootstrap (one-time)
./build_amc.sh       # build the self-hosted amc (~5s)
./tests/run_all_tests.sh
```

### macOS

```bash
brew install bdw-gc curl
git clone https://github.com/BastienMOUGET/Amalgame.git && cd Amalgame
gcc -O2 -Iruntime src/amc_lib.c -lgc -lm -lcurl -o amc
./amc --version
```

### Windows (MSYS2 MINGW64)

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gc mingw-w64-x86_64-curl
git clone https://github.com/BastienMOUGET/Amalgame.git && cd Amalgame
gcc -O2 -Iruntime src/amc_lib.c -lgc -lm -lcurl -o amc.exe
./amc.exe --version
```

### Hello, World

```amalgame
// hello.am
namespace App
import Amalgame.IO

public class Program {
    public static void Main(string[] args) {
        Console.WriteLine("Hello, Amalgame!")
    }
}
```

```bash
./amc hello.am -o hello
gcc -Iruntime hello.c -lgc -lm -lcurl -o hello
./hello
# → Hello, Amalgame!
```

---

## Language Highlights

<table>
<tr>
<td valign="top" width="50%">

**Pattern matching with guards**

```amalgame
let n = 42
match n {
    0           => doZero(),
    x if x < 0  => doNeg(x),
    1..9        => doSmall(),
    10..99      => doMedium(),
    _           => doLarge()
}
```

</td>
<td valign="top" width="50%">

**Null-safety baked in**

```amalgame
let user: User? = null
let name = user?.Name
let age  = user?.GetAge()
if (name == null) {
    Console.WriteLine("anonymous")
}
```

</td>
</tr>
<tr>
<td valign="top">

**String interpolation + multiline**

```amalgame
let title = "Hello"
let html = """
<article>
  <h1>{title}</h1>
  <p>by Amalgame</p>
</article>
"""
```

</td>
<td valign="top">

**Algebraic enums + destructuring**

```amalgame
public enum Shape {
    Circle(int)
    Rect(int, int)
}

match s {
    Circle(r)  => useRadius(r),
    Rect(w, h) => useDims(w, h)
}
```

</td>
</tr>
<tr>
<td valign="top">

**Tuples & destructuring**

```amalgame
public (int, int) DivMod(int a, int b) {
    return (a / b, a % b)
}

let (q, r) = Math2.DivMod(17, 5)
```

</td>
<td valign="top">

**Decorators with real C semantics**

```amalgame
@inline
public static int Hot(int x) {
    return x * x
}

@deprecated
public static void Old() {
    /* GCC warns at every callsite */
}
```

</td>
</tr>
</table>

Plus: bitwise ops, compound assigns, pipeline `|>`, range `0..n`,
data classes, records, interfaces, generics, lambdas, try/catch,
`\x` and `\u` escapes, named arguments, guard clauses…

---

## Project Layout

```
src/             ← The compiler — written in Amalgame
├── lexer/                  token.am, lexer.am
├── parser/                 ast.am, parser.am
├── resolver/               symbol.am, resolver.am  (scope + member table)
├── typechecker.am          (type inference + assignability)
├── generator/c_gen.am      (Amalgame → C)
├── diagnostics.am          (rustc-style errors with source snippets)
├── main.am                 (CLI entry: parses argv, drives the pipeline)
└── amc_lib.c               (generated — 7000+ lines of self-hosted C)

runtime/   ← Tiny C runtime (collections, strings, GC, IO)
stdlib/                   ← Amalgame stdlib reference (.am declarations)
tests/                    ← 127 sample programs + integration tests
archive/vala-bootstrap/   ← Original Vala compiler — kept as recovery path
editors/vscode/           ← VS Code syntax highlighting extension
.github/workflows/        ← CI (Linux/macOS/Windows) + tag-driven Release
```

The build flow:

```
src/*.am  ──[ ./amc ]──▶  amc_lib.c  ──[ gcc ]──▶  amc binary
                           ▲                                   │
                           └───────────────────────────────────┘
                              compiles itself, in ~5 seconds
```

---

## Show Me the Diagnostics

Type errors come with a source snippet and a caret, just like rustc:

```
error[typechecker]: Cannot assign 'string' to 'n' of type 'int'
  --> /tmp/test.am:19:13
   |
19 |         let n: int = p.Name
   |             ^
```

Resolver errors get the same treatment:

```
error[resolver]: Unknown symbol 'someUndefinedThing'
  --> file.am:4:9
  |
4 |         someUndefinedThing.x()
  |         ^
```

---

## Editor Support

Drop-in VS Code syntax highlighting lives in [editors/vscode/](editors/vscode/):

```bash
ln -s "$(pwd)/editors/vscode" ~/.vscode/extensions/amalgame-0.1.0
# Reload VS Code (Ctrl+Shift+P → Developer: Reload Window)
```

Or for a sandboxed dev session:

```bash
code --extensionDevelopmentPath="$(pwd)/editors/vscode" .
```

LSP (completion, hover, go-to-def) is on the roadmap.

---

## Roadmap

See [ROADMAP_COMPLET.md](ROADMAP_COMPLET.md) for the full picture.

**Done** — full self-host, multi-OS CI/release, rich diagnostics,
match guards, null-safe `?.`, decorators, named args, guard clauses,
triple-quoted strings, `\x`/`\u` escapes, P2 (member resolution),
P5 (resolver scopes), P6 (snippets), P7 (lexer bugs).

**On deck** — `amc fmt`, `amc test`, LSP, list comprehensions,
capturing closures, generic inference, async/await.

---

## Contributing

Workflow is gitflow:

- `main` is stable (release tags only)
- `develop` is the integration branch
- Features land via `feature/<name>` branches → PR → merge into `develop`

```bash
git checkout develop
git checkout -b feature/my-thing
# … hack hack hack …
./build_amc.sh && ./tests/run_all_tests.sh
git push -u origin feature/my-thing
gh pr create --base develop
```

CI runs on every push and PR. Releases are cut by pushing a `v*` tag.

---

## License

[Apache License 2.0](LICENSE) © Bastien Mouget
