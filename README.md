<div align="center">

# Amalgame

A self-hosted programming language that compiles to C.

[![CI](https://github.com/BastienMOUGET/Amalgame/actions/workflows/ci.yml/badge.svg)](https://github.com/BastienMOUGET/Amalgame/actions/workflows/ci.yml)
[![Self-hosted](https://img.shields.io/badge/compiler-self--hosted-success)](src/)
[![Tests](https://img.shields.io/badge/tests-121%2F121-brightgreen)](tests/)
[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)]()

</div>

```amalgame
namespace App
import Amalgame.IO

public class Greeter {
    public Name: string

    public Greeter(string name) {
        this.Name = name
    }

    public string Hello() {
        guard String.Length(this.Name) > 0 else { return "Hello, stranger!" }
        return "Hello, {this.Name}!"
    }
}
```

## Overview

Amalgame is a small, statically-typed language whose compiler (`amc`)
is written in Amalgame and emits portable C. The runtime is a thin
header-only layer over libc, libgc (Boehm GC) and libcurl.

- **Self-hosted.** The compiler bootstraps itself in about five
  seconds. The original Vala bootstrap is preserved in
  `archive/vala-bootstrap/` for cold-start recovery; day-to-day work
  uses a tracked `snapshot/` of a known-good `amc_lib.c`.
- **Predictable output.** Source maps cleanly to C. Strings are
  `char*`, lists are `void**` arrays, integers are `i64`. No VM, no
  hidden allocations beyond the GC. Generated C is `gcc`-buildable
  with `-O2`.
- **Multi-platform.** Linux, macOS, and Windows binaries are produced
  on every `v*` tag. Windows is supported via MinGW (Winsock under
  `#ifdef _WIN32` in the runtime).

Current version: **v0.3.1**.

## Language at a glance

```amalgame
namespace App
import Amalgame.IO

public enum Shape {
    Circle(int)
    Rect(int, int)
}

public class Program {
    public static void Main(string[] args) {
        // Match as expression with arm guards and ranges
        let n = 42
        let bucket = match n {
            0           => "zero"
            x if x < 0  => "negative"
            1..9        => "small"
            10..99      => "medium"
            _           => "large"
        }
        Console.WriteLine(bucket)

        // List comprehension
        let squares = [i * i for i in 0..10 if i % 2 == 0]
        Console.WriteLine(String.FromInt(squares.Count()))

        // Tuple destructuring
        let (q, r) = Program.DivMod(17, 5)
        Console.WriteLine("{String.FromInt(q)} rem {String.FromInt(r)}")

        // Algebraic enum + destructuring
        let s = Shape.Circle(5)
        match s {
            Circle(r)  => Console.WriteLine("r={String.FromInt(r)}")
            Rect(w, h) => Console.WriteLine("rect")
        }

        // Null-safe member access
        let maybe: Greeter? = null
        let label = maybe?.Hello()
        if (label == null) { Console.WriteLine("no greeter") }
    }

    public static (int, int) DivMod(int a, int b) {
        return (a / b, a % b)
    }
}
```

A more thorough tour is in [docs/guide/02-language-tour.md](docs/guide/02-language-tour.md).

## Getting started

### Linux

```bash
sudo apt install gcc libgc-dev libcurl4-openssl-dev
git clone https://github.com/BastienMOUGET/Amalgame.git && cd Amalgame
gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc
./build_amc.sh           # builds ./amc from src/ via the snapshot
./amc --version          # amc 0.3.1
./tests/run_all_tests.sh
```

### macOS

```bash
brew install bdw-gc curl
git clone https://github.com/BastienMOUGET/Amalgame.git && cd Amalgame
GC_PREFIX=$(brew --prefix bdw-gc)
CURL_PREFIX=$(brew --prefix curl)
gcc -O2 -Iruntime -I"$GC_PREFIX/include" -I"$CURL_PREFIX/include" \
    -L"$GC_PREFIX/lib" -L"$CURL_PREFIX/lib" \
    snapshot/amc_lib.c -lgc -lm -lcurl -o amc
./amc --version
```

### Windows (MSYS2 MINGW64)

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gc mingw-w64-x86_64-curl
git clone https://github.com/BastienMOUGET/Amalgame.git && cd Amalgame
gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -lws2_32 -o amc.exe
./amc.exe --version
```

### Hello, world

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

## What's in the language

- Variables (`let` / `var`) with optional type annotation
- Primitives: `int`, `float`, `double`, `bool`, `string`, `void`
- Classes with inheritance, interfaces, data classes, records
- Enums (simple) and algebraic enums with destructuring
- Generics (erased to `void*` at the C level)
- Null-safety: `T?`, `??` coalescing, `?.` safe member access
- Tuples and destructuring: `let (a, b) = f()`
- Pattern matching with guards, ranges, binders, statement-shaped
  arms (`return`, `break`, `continue`)
- `match` and `if` as expressions
- List comprehensions: `[x*2 for x in xs if x > 0]`
- String interpolation: `"hello {x}"`
- Triple-quoted multiline strings: `"""…"""`
- `\xHH` and `\uHHHH` escape sequences
- Bitwise operators, compound assignments, pipeline `|>`, range `0..n`
- Guard clauses: `guard cond else { return }`
- Decorators (`@inline`, `@deprecated`) mapped to GCC attributes
- Lambdas (non-capturing for now)

Detailed reference: [docs/guide/](docs/guide/).

## Diagnostics

Resolver and type-checker errors come with a source snippet and a
caret pointing at the offending token:

```
error[typechecker]: Cannot assign 'string' to 'n' of type 'int'
  --> /tmp/test.am:19:13
   |
19 |         let n: int = p.Name
   |             ^
```

```
error[resolver]: Unknown symbol 'someUndefinedThing'
  --> file.am:4:9
   |
 4 |         someUndefinedThing.x()
   |         ^
```

## Tooling

- **`amc fmt`** — formatter that re-emits a parsed AST canonically
  while preserving comments. Idempotent on every source in this
  repository. Run it as `amc fmt file.am` (stdout) or
  `amc fmt -w file.am` (rewrite in place).
- **VS Code syntax highlighting** is in [editors/vscode/](editors/vscode/).
  Install with:
  ```bash
  ln -s "$(pwd)/editors/vscode" ~/.vscode/extensions/amalgame-0.3.1
  ```
- An LSP server (`amc lsp`) is on the roadmap; today there is no
  language server.

## Project layout

```
src/                  Compiler, written in Amalgame
├── lexer/              token.am, lexer.am
├── parser/             ast.am, parser.am
├── resolver/           symbol.am, resolver.am
├── generator/          c_gen.am, gen_test.am
├── formatter/          formatter.am
├── typechecker.am
├── diagnostics.am
├── main.am             CLI: amc <files> | amc fmt <files>
└── amc_lib.c           Self-hosted bundle (generated)

runtime/              C runtime (bdwgc, strings, IO, collections, net)
stdlib/               Stdlib API reference (.am declarations)
snapshot/             Tracked amc_lib.c known-good bootstrap
tools/                save-snapshot.sh
tests/                Sample programs + run_*.sh runners
docs/guide/           User guide chapters 1–7
archive/              Original Vala compiler (recovery path)
editors/vscode/       Syntax highlighting extension
.github/workflows/    CI + tag-driven Release automation
```

The build pipeline:

```
src/*.am ─[ ./amc ]─▶ amc_lib.c ─[ gcc ]─▶ amc binary
              ▲                              │
              └──────────────────────────────┘
                  five-second self-rebuild
```

When `./amc` is missing or broken, `build_amc.sh` falls back to
`./snapshot/amc`, then to `./build/amc` (Vala). See
`tools/save-snapshot.sh` for capturing a new snapshot.

## Roadmap

The full board is in [ROADMAP_COMPLET.md](ROADMAP_COMPLET.md). The
short version, ordered by unlocked-value per day of work:

1. **Fix the SKIPped samples** — `Type.Variant` patterns in match,
   `try / catch`, null-safety inference. Restores feature parity
   with the original Vala bootstrap.
2. **Minimal LSP** — completion + hover via stdio JSON-RPC, reusing
   the existing parser, resolver, and type-checker.
3. **Capturing closures.**
4. **Generic type inference.**

## Contributing

Workflow is a simplified gitflow:

- `main` carries release tags only.
- `develop` is the integration branch; PRs target it.
- Features land via `feature/<name>` branches.

```bash
git checkout develop
git checkout -b feature/my-thing
# ...
./build_amc.sh && ./tests/run_all_tests.sh
git push -u origin feature/my-thing
gh pr create --base develop
```

CI runs on every push and pull request. Releases are produced by
pushing a `v*` tag — see `.github/workflows/release.yml`.

## License

[Apache License 2.0](LICENSE) © Bastien Mouget
