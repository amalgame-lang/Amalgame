# Amalgame Programming Language 🚀

> The amalgam of the best of every language.
> Transpiles to C → compilable everywhere via GCC.

---

## Vision

Amalgame takes the best from each language:

| Feature               | Inspired by       |
|-----------------------|-------------------|
| Type inference        | Kotlin / Swift    |
| Null safety           | Kotlin / Swift    |
| Pattern matching      | Rust / Haskell    |
| Result / Option       | Rust              |
| Data classes          | Kotlin            |
| Extension methods     | Kotlin / C#       |
| Pipeline `\|>`        | F# / Elixir       |
| Async / Await         | C# / JS           |
| GC by default         | Go / C#           |
| Memory decorators     | Nim / D           |

---

## Example

```amalgame
namespace MyApp

import Amalgame.IO

public class Program {

    public static void Main(string[] args) {

        let players = new List<Player>([
            new Player("Arthus", level: 42),
            new Player("Merlin", level: 38),
            new Player("Robyn",  level: 15)
        ])

        let veterans = players
            |> Where(p  => p.Level >= 35)
            |> OrderBy(p => p.Level)
            |> ToList()

        veterans |> ForEach(p =>
            Console.WriteLine("⚔️  {p.Name} (Lvl {p.Level})")
        )
    }
}
```

Output:
```
⚔️  Merlin (Lvl 38)
⚔️  Arthus (Lvl 42)
```

---

## Architecture

```
source.am
    │
    ▼
[ Lexer ]       →  Tokens
    │
    ▼
[ Parser ]      →  AST
    │
    ▼
[ Resolver ]    →  AST + SymbolTable
    │
    ▼
[ TypeChecker ] →  AST annotated
    │
    ▼
[ C Generator ] →  .c file
    │
    ▼
[ GCC ]         →  Native executable
```

---

## Tools

| Tool        | Description                    |
|-------------|--------------------------------|
| `amc`       | Main transpiler                |
| `amc-lsp`   | LSP server (editors)           |
| `amc-dap`   | Debug Adapter (breakpoints)    |

---

## Supported Editors

Via LSP + DAP:
- ✅ VSCode
- ✅ NeoVim
- ✅ Emacs
- ✅ Vim
- ✅ Sublime Text
- ✅ IntelliJ

---

## Build

```bash
# Dependencies
sudo apt install valac libglib2.0-dev libgee-0.8-dev \
                 meson ninja-build libgc-dev

# Build
meson setup build
cd build && ninja

# Test
./amc --version
```

---

## Usage

```bash
# Compile an Amalgame source file
amc hello.am

# Specify output
amc hello.am -o hello.c

# Debug — show AST
AMC_DEBUG=1 amc hello.am

# Skip type checking
amc hello.am --no-typecheck
```

---

## Project Status

```
✅ Language specification
✅ EBNF grammar
✅ Lexer
✅ AST (complete nodes)
✅ Parser
✅ Resolver (name resolution & scope checking)
✅ TypeChecker (type inference & validation)
✅ C Generator
✅ Test suite (7/7)
🔜 Standard Library
🔜 LSP / DAP
```

---

## License

Licensed under Apache 2.0
Copyright (c) 2026 Bastien MOUGET
[LICENSE](LICENSE)

---

## Author

**BastienMOUGET**
GitHub: [@BastienMOUGET](https://github.com/BastienMOUGET)
