# 1 · Getting started

## What you need to know first

Amalgame transpiles to portable C. Compiling an Amalgame program is
always a **two-step** process:

1. `amc your.am -o your` produces `your.c` (a `.c` file, NOT a binary).
2. `gcc your.c -lgc -lm -o your` produces the actual native
   binary.

The `amc` you run in step 1 is itself the output of that exact same
pipeline applied to the compiler's own sources in `src/`.

## Install

### Docker — full IDE, zero install

The fastest way to try Amalgame: a batteries-included container that
boots a browser VS Code with the compiler, the extension (syntax
highlighting, LSP, debugger), a ready-to-run sample and the docs — all
pre-wired, nothing to set up.

```bash
docker run --rm -p 8080:8080 ghcr.io/amalgame-lang/amalgame-ide:latest
```

Then open <http://localhost:8080>. You land on the `MyFirstApp` sample
with the user guide pinned in the sidebar — press **F5** to build and
debug it on the spot.

- **Port already in use?** Map any free host port instead, e.g.
  `-p 8888:8080`, then open <http://localhost:8888>.
- **Multi-arch:** runs natively on Intel/AMD (`amd64`) and on Apple
  Silicon / Raspberry Pi 64-bit (`arm64`).
- **Persist your work:** mount a folder, e.g.
  `-v "$PWD/work:/home/coder/work"`.
- **Exposing it beyond localhost?** Set a password with `-e PASSWORD=…`
  (auth is disabled by default for local use).

Prefer a native install? Pick your platform below.

### Linux (Debian/Ubuntu)

```bash
sudo apt install -y gcc libgc-dev

git clone https://github.com/amalgame-lang/Amalgame.git
cd Amalgame
gcc -O2 -Iruntime snapshot/amc_lib.c \
    -lgc -lm -o snapshot/amc   # one-time: bootstrap from tracked C
./build_amc.sh                         # builds the self-hosted amc into ./amc (~5s)
./tests/run_all_tests.sh               # full suite: 578 PASS in ~42s
```

Individual test suites can be run via `./amc test ./tests/<bundle>/`
where `<bundle>` is one of `fmt`, `amc_new`, `stdlib_bundle`, or
`core_bundle` — see [06-build-and-tooling.md](06-build-and-tooling.md#tests--am-bundles-via-amc-test).

### macOS (Apple Silicon or Intel)

```bash
brew install bdw-gc curl
git clone https://github.com/amalgame-lang/Amalgame.git
cd Amalgame
gcc -O2 -Iruntime src/amc_lib.c \
    -lgc -lm -o amc
./amc --version
```

`amc_lib.c` is the canonical cross-platform snapshot of the
self-hosted compiler — it's tracked in git, so any platform with a
working `gcc` can bootstrap from a clean clone.

### Windows (MSYS2 MINGW64)

```bash
pacman -S mingw-w64-x86_64-{gcc,gc,curl}
git clone https://github.com/amalgame-lang/Amalgame.git
cd Amalgame
gcc -O2 -Iruntime src/amc_lib.c \
    -lgc -lm -o amc.exe
./amc.exe --version
```

For a packaged `.exe` installer that bundles a portable MinGW64,
see `install/windows/amalgame.iss` (Inno Setup script).

### Pre-built binary

Each tagged release in
[GitHub Releases](https://github.com/amalgame-lang/Amalgame/releases)
ships `amc-X.Y.Z-{linux,macos,windows}-...` archives. The one-liner picks
the right one for your OS/architecture:

```bash
curl -fsSL https://raw.githubusercontent.com/amalgame-lang/Amalgame/main/install/install.sh | sh
```

### Raspberry Pi (and other ARM64 Linux)

`amc` ships a native **`linux-arm64`** build, so the one-liner above works
on a Raspberry Pi running a **64-bit OS** (Raspberry Pi OS 64-bit, Ubuntu
for Pi, …) — i.e. `uname -m` reports `aarch64`. Install the runtime deps
first, same as any Debian/Ubuntu box:

```bash
sudo apt-get install -y build-essential libgc-dev libssl-dev curl
curl -fsSL https://raw.githubusercontent.com/amalgame-lang/Amalgame/main/install/install.sh | sh
amc --version
```

> **32-bit is not supported.** On a Pi 3/4/5 running a 32-bit OS,
> `uname -m` shows `armv7l`/`armv6l` and the installer stops with a clear
> message — reflash a 64-bit OS, or build from source (the
> `amc_lib.c` snapshot compiles anywhere with a C toolchain + Boehm GC).
> The `linux-arm64` archive is built natively in CI on a real aarch64
> runner, not cross-compiled.

## Develop in VS Code (the easy path)

`install.sh` auto-installs the **Amalgame VS Code extension** if it detects
VS Code (the release bundles the `.vsix`; set `AMC_NO_EDITORS=1` to skip).
If you installed amc some other way, install it by hand once:

```bash
code --install-extension <amalgame-X.Y.Z.vsix>   # from share/amalgame/editors/vscode/
```

Now just **open any `.am` file** — the extension starts `amc lsp` and you
get a real IDE, no configuration:

- **Live diagnostics** — the same errors `amc --check` reports, as you type.
- **Completion** — symbols, methods, and `import Amalgame.<cursor>` listing
  the stdlib + your installed packages.
- **Navigation & search** — go-to-definition (**F12**), **Find All
  References** (**Shift+F12**), project-wide **symbol search** (**Ctrl+T**),
  file **outline** (**Ctrl+Shift+O**), and same-symbol highlight. Call
  hierarchy, code actions, inlay hints, and folding are advertised too.
- **Hover, rename** (**F2**), and **format** (`amc fmt`).
- **Run** — from the integrated terminal: `amc run hello.am` (build + exec),
  or `amc build hello.am` then `./hello`.
- **Debug** — press **F5** with a `launch.json` of type `amc`: real
  breakpoints and stepping in your `.am` source.

A good first session: `mkdir hello && cd hello`, open the folder in VS
Code, create `hello.am` (next section), and `amc run hello.am` in the
terminal. Diagnostics, completion, and search work immediately.

> The extension is LSP/DAP-based and configuration-free for the common
> case. For the full feature list, editor settings, other editors (Neovim,
> …), and the debugger internals, see chapter 15,
> [Editor tooling & debugging](15-debugging.md).

## Hello, World

Save as `hello.am`:

```kotlin
namespace App
import Amalgame.IO

public class Program {
    public static void Main(string[] args) {
        Console.WriteLine("Hello, Amalgame!")
    }
}
```

Compile and run:

```bash
./amc hello.am -o hello
gcc -Iruntime hello.c -lgc -lm -o hello
./hello
# → Hello, Amalgame!
```

A few important conventions:

- Every file starts with `namespace Foo.Bar` — the C symbols emitted
  for the file are prefixed with `Foo_Bar_`.
- The runtime entry point is `Program.Main(string[] args)`. If a file
  defines a `Program.Main`, the compiler appends a C `int main()`
  wrapper to the output. Otherwise the file compiles as a library (no
  `main()`), suitable for linking with another program.
- `import Amalgame.X` lines are accepted but currently informational —
  the resolver knows about the stdlib globally.

## A slightly bigger first program

```kotlin
// add.am
namespace App
import Amalgame.IO

public class Program {
    public static int Add(int a, int b) {
        return a + b
    }

    public static void Main(string[] args) {
        let n = Program.Add(2, 3)
        Console.WriteLine("2 + 3 = {String.FromInt(n)}")
    }
}
```

```bash
./amc add.am -o add && gcc -Iruntime add.c -lgc -lm -o add && ./add
# → 2 + 3 = 5
```

## Library mode (no `main()`)

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
./amc --lib greeter.am -o greeter
# → Generated: greeter.c (... lines) [Library]
gcc -Iruntime -c greeter.c -o greeter.o
# Link greeter.o into another program from C, Amalgame, or another lib.
```

## Where to go next

- The [language tour](02-language-tour.md) is the fast read-through.
- The [CLI reference](03-cli-reference.md) lists every flag.
- If you hit an error you don't understand, see "Reading diagnostics"
  in the language tour.
