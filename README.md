<div align="center">

# Amalgame

**A modern, self-hosted programming language that transpiles to C.**

[![CI](https://github.com/BastienMOUGET/Amalgame/actions/workflows/ci.yml/badge.svg)](https://github.com/BastienMOUGET/Amalgame/actions/workflows/ci.yml)
[![Self-hosted](https://img.shields.io/badge/compiler-self--hosted-success)](src/amalgame/)
[![Tests](https://img.shields.io/badge/tests-127%2F127-brightgreen)](tests/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-blue)]()

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
- **🧬 Self-hosted compiler** — `amc` is written in Amalgame and compiles itself. The whole pipeline (lexer, parser, resolver, type-checker, code-gen) lives in [src/amalgame/](src/amalgame/).
- **✨ Modern syntax you actually want to use** — pattern matching with guards, null-safety with `?.`, string interpolation, decorators, generics, tuples, lambdas.
- **⚡ ~5s build** — full self-host rebuild including the bootstrap, in five seconds.
- **📦 Multi-OS** — Linux, macOS, Windows binaries built on every release tag.

---

## A Taste

```amalgame
namespace App
import Amalgame.IO

public class Calc {
    // Decorators map to GCC attributes / hints.
    @inline
    public static int Square(int x) { return x * x }

    // Guard clauses: read top-to-bottom instead of nesting `if`.
    public static int Clamp(int x, int lo, int hi) {
        guard x > lo else { return lo }
        guard x < hi else { return hi }
        return x
    }

    // Pattern matching with arm guards + range patterns + binders.
    public static string Classify(int n) {
        match n {
            0           => return "zero",
            x if x < 0  => return "negative",
            1..9        => return "small",
            10..99      => return "medium",
            _           => return "large"
        }
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

public class Program {
    public static void Main(string[] args) {
        // Named arguments + string interpolation.
        let u = new User(name: "Bastien", age: 31)
        Console.WriteLine("Hello, {u.Name}! You are {String.FromInt(u.Age)}.")

        // Null-safe member access — no NPE, ever.
        let maybe: User? = null
        let label = maybe?.Name
        if (label == null) { Console.WriteLine("nobody") }

        // Triple-quoted multiline strings preserve newlines as-is.
        Console.WriteLine("""
        ┌────────────────┐
        │  Amalgame v0.x │
        └────────────────┘
        """)
    }
}
```

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
gcc -O2 -Isrc/transpiler/runtime src/amalgame/amc_lib.c -lgc -lm -lcurl -o amc
./amc --version
```

### Windows (MSYS2 MINGW64)

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gc mingw-w64-x86_64-curl
git clone https://github.com/BastienMOUGET/Amalgame.git && cd Amalgame
gcc -O2 -Isrc/transpiler/runtime src/amalgame/amc_lib.c -lgc -lm -lcurl -o amc.exe
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
gcc -Isrc/transpiler/runtime hello.c -lgc -lm -lcurl -o hello
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
match status {
    n if n < 200 => "info",
    n if n < 300 => "success",
    n if n < 400 => "redirect",
    n if n < 500 => "client error",
    _            => "server error"
}
```

</td>
<td valign="top" width="50%">

**Null-safety baked in**

```amalgame
let user: User? = repo.Find(id)
let name = user?.Name
let age  = user?.GetAge()
if (name == null) {
    return "anonymous"
}
```

</td>
</tr>
<tr>
<td valign="top">

**String interpolation + multiline**

```amalgame
let html = """
<article>
  <h1>{post.Title}</h1>
  <p>by {post.Author}</p>
</article>
"""
```

</td>
<td valign="top">

**Algebraic enums + match**

```amalgame
public enum Shape {
    Circle(float r)
    Rect(float w, float h)
}

let area = match s {
    Circle(r)  => 3.14 * r * r,
    Rect(w, h) => w * h
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
public static int Hot(int x) { return x*x }

@deprecated
public static void Old() { ... }
// → C compiler warns at every callsite
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
src/amalgame/             ← The compiler — written in Amalgame
├── lexer/                  token.am, lexer.am
├── parser/                 ast.am, parser.am
├── resolver/               symbol.am, resolver.am  (scope + member table)
├── typechecker.am          (type inference + assignability)
├── generator/c_gen.am      (Amalgame → C)
├── diagnostics.am          (rustc-style errors with source snippets)
├── main.am                 (CLI entry: parses argv, drives the pipeline)
└── amc_lib.c               (generated — 7000+ lines of self-hosted C)

src/transpiler/runtime/   ← Tiny C runtime (collections, strings, GC, IO)
stdlib/                   ← Amalgame stdlib reference (.am declarations)
tests/                    ← 127 sample programs + integration tests
archive/vala-bootstrap/   ← Original Vala compiler — kept as recovery path
editors/vscode/           ← VS Code syntax highlighting extension
.github/workflows/        ← CI (Linux/macOS/Windows) + tag-driven Release
```

The build flow:

```
src/amalgame/*.am  ──[ ./amc ]──▶  amc_lib.c  ──[ gcc ]──▶  amc binary
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

[MIT](LICENSE) © Bastien Mouget
