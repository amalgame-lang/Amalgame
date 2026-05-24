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
`core_bundle` — see [06-build-and-tooling.md](06-build-and-tooling.md#tests).

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
ships `amc-X.Y.Z-{linux,macos,windows}-...` archives.

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
