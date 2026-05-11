# 3 · CLI reference

## Synopsis

```
amc [options] file1.am [file2.am ...] -o <output>     # compile
amc <subcommand> [args...]                            # other commands
```

The compiler always emits a `.c` file. To produce a native binary,
follow up with `gcc`:

```bash
amc hello.am -o hello                                    # → hello.c
gcc -Iruntime hello.c -lgc -lm -lcurl -o hello           # → hello (binary)
./hello
```

The runtime headers live in `runtime/` at the project root.

## Subcommands

| Command | Purpose | Reference |
| ------- | ------- | --------- |
| `fmt [-w] file.am`     | Idempotent formatter (stdout, or `-w` in-place)        | `amc fmt --help` |
| `test [<dir>]`         | Discover `*_test.am`, compile + run, aggregate         | `amc test --help` |
| `lsp`                  | Workspace-aware LSP server over stdio JSON-RPC          | chap. 6 |
| `new <name> [--template exe\|lib\|test]` | Scaffold a new project       | `amc new --help` |
| `package <action>` (alias `pkg`) | Project package management (see below)        | `amc package --help` |
| `migrate <file\|dir>`   | LLM-driven source-to-Amalgame translation               | chap. 8 |
| `generate "<prompt>"`  | LLM-driven prose-to-Amalgame                            | chap. 8 |
| `explain <file.am>`    | LLM-driven Amalgame-to-prose                            | chap. 8 |

Each subcommand handles its own flags and exit codes; consult the
referenced chapter or `--help` for the full surface.

### `amc package <action>` (alias `amc pkg`)

Project-local package operations. All actions read/write
`amalgame.toml` (deps) and `amalgame.lock` (resolved Git SHAs) in
the current working directory.

| Action | Purpose |
| ------ | ------- |
| `add <git-url>@<tag> [--no-precompile]`  | Clone + validate + record dep. Accepts `<name>@<tag>` resolved via the curated index. `--no-precompile` (v0.5.4+) skips the install-time compile of `[stdlib].sources` even if the manifest declares `precompile = true`. |
| `remove <name>`        | Drop dep from manifest + lockfile. |
| `search <keyword>`     | Substring match against descriptions of cached + indexed packages. |
| `list`                 | Show installed packages with tier badge (official / community / unverified). |
| `update [<name>]`      | Bump a single dep, or all if no name given. |
| `cache <ls\|gc>`       | Inspect / prune `~/.amalgame/packages/`. |

`amc test` auto-installs any missing deps before running the suite,
and links each package's `[stdlib].sources` vendored C objects into
every test binary — backends like SQLite "just work" without manual
`gcc` flags. Since v0.5.3, `.cpp` / `.cc` / `.cxx` sources are
compiled with `g++` (and the final test link goes through `g++`
when any package contributes C++), so DuckDB-style packages
vendoring a C++ amalgamation work out of the box too.

### `amalgame.toml` manifest reference

A package's `amalgame.toml` describes both its own metadata
(`[package]`) and the surface it contributes to consumers (`[stdlib]`).
A user project's `amalgame.toml` typically has just `[package]` +
`[dependencies]`.

```toml
[package]
name              = "amalgame-database-duckdb"
version           = "0.1.0"
license           = "Apache-2.0"
required-amalgame = ">=0.5.3"   # refuses install on older amc
schema-version    = 1           # refuses install on amc < this schema

[stdlib]
class     = "DuckDB"
header    = "runtime/Amalgame_Database_DuckDB.h"
namespace = "Amalgame.Database.DuckDB"
sources   = ["runtime/Amalgame_Database/duckdb/duckdb.cpp"]
cflags    = ""                  # extra flags for .c sources
cxxflags  = "-O3 -DNDEBUG -std=c++17"  # extra flags for .cpp/.cc/.cxx
libs      = ["stdc++"]          # bare names → -l<name> at link time

[stdlib.functions]
Open  = { returns = "AmalgameDuckDB*" }
Close = { returns = "void" }
# …
```

| Key                              | Where     | Since   | Purpose                                                                    |
| -------------------------------- | --------- | ------- | -------------------------------------------------------------------------- |
| `name`, `version`, `license`     | `[package]` | v0.5.0 | Required identity fields                                                   |
| `description`, `authors`         | `[package]` | v0.5.0 | Surfaced in `amc package list` / search                                    |
| `required-amalgame`              | `[package]` | v0.5.0 | Semver constraint on the running amc (e.g. `">=0.5.3"`)                    |
| `schema-version`                 | `[package]` | v0.5.3 | Refuses install when amc supports a lower manifest schema                  |
| `class`, `header`, `namespace`   | `[stdlib]`  | v0.5.0 | Required wiring for the cgen + resolver                                    |
| `sources`                        | `[stdlib]`  | v0.5.0 | Vendored `.c` / `.cpp` paths to compile + link                             |
| `cflags`                         | `[stdlib]`  | v0.5.3 | Extra gcc flags for the package's `.c` sources                             |
| `cxxflags`                       | `[stdlib]`  | v0.5.3 | Extra g++ flags for the package's `.cpp` / `.cc` / `.cxx` sources          |
| `libs`                           | `[stdlib]`  | v0.5.3 | Bare lib names → `-l<name>` appended to every consumer's final link        |
| `precompile`                     | `[stdlib]`  | v0.5.4 | When `true`, `amc package add` compiles sources at install time into `~/.amalgame/packages/.../build/<platform>/`. Subsequent `amc test`/`amc build` reuse the cached `.o`. Override with `--no-precompile`. |
| `[stdlib.functions]`             | section     | v0.5.0 | `<Method> = { returns = "<C-type>" }` — populates the cgen's dispatch     |

## Options

| Flag             | Effect                                                                 |
| ---------------- | ---------------------------------------------------------------------- |
| `-o <output>`    | Output basename. The `.c` file is `<output>.c`. Defaults to `a.out`.   |
| `--lib`          | Compile as library (no `int main()` emitted, `[Library]` in output).   |
| `--check`        | Type-check only, do not write the `.c` file. Useful for editor checks. |
| `--color`        | Force ANSI color output in diagnostics.                                |
| `--no-color`     | Disable ANSI color output.                                             |
| `--quiet`        | Suppress progress messages (only errors and the final report).         |
| `--verbose`      | Print extra build info.                                                |
| `--version`      | Print the version and exit.                                            |
| `--help`, `-h`   | Print usage and exit.                                                  |

Any positional argument ending in `.am` is treated as an input file.
Unknown options error out with a usage message.

## Exit codes

| Code | Meaning                                                |
| ---- | ------------------------------------------------------ |
| `0`  | Success.                                               |
| `1`  | Resolver and/or type-checker reported an error, *or* the CLI was misused (no input, unknown flag, etc.). |

The compiler tries to make progress past resolver warnings — you'll
often see a `.c` file generated alongside diagnostics. Treat exit
status as the source of truth, not the presence of the `.c` file.

## Multi-file compilation

Pass several `.am` files; the compiler merges them, runs all passes
across the union, and emits a single `.c` file:

```bash
amc lexer.am parser.am main.am -o myapp
```

The first file's `namespace` directive determines the C symbol prefix
for the whole compilation unit, so put the most authoritative file
first (or make sure all files share the same namespace).

`main.am`'s `Program.Main` becomes the binary's entry point. If
multiple files declare `Program.Main`, the C compiler will reject the
duplicate symbols at link time — only one entry point per executable.

## Library mode

```bash
amc --lib mylib.am -o mylib
# → mylib.c (no int main, marked "[Library]")
gcc -Iruntime -c mylib.c -o mylib.o
# Link mylib.o from a host program — see chapter 5 for C interop.
```

Auto-detection: a file without any `Program.Main` is treated as a
library automatically (no `--lib` flag needed).

## Typical workflows

### Smoke-test a single file

```bash
amc foo.am -o /tmp/foo
gcc -Iruntime /tmp/foo.c -lgc -lm -lcurl -o /tmp/foo
/tmp/foo
```

### Type-check on save (editor integration)

```bash
amc --check --no-color foo.am
echo $?     # 0 = OK, 1 = errors printed on stderr
```

### Build a multi-file project

```bash
amc \
    src/util/strings.am \
    src/util/collections.am \
    src/server/router.am \
    src/main.am \
    -o build/app

gcc -O2 -Iruntime build/app.c -lgc -lm -lcurl -o build/app
```

### Rebuild the compiler itself

```bash
./build_amc.sh        # ~5s, full self-host (runs amc on its own sources)
```

That script's behaviour is described in chapter 6.

## Environment

The CLI is hermetic — it doesn't read environment variables. The
compile-side runtime headers come from `gcc -I<runtime-dir>`.

## Debugging the compiler

If `amc` produces invalid C, narrow the input by deleting half until
the failure disappears, then file an issue with the minimal repro.
Useful flags during triage:

- `--quiet` to silence "OK" messages and focus on errors
- `--no-color` so the snippet output stays grep-friendly
- Inspect the emitted `.c` file directly — naming follows
  `<Namespace>_<Class>_<Method>` and is easy to grep.

For deeper internals (how the pipeline walks the AST, where to add a
new builtin, how the CGen emits a feature), see chapter 7.
