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
| `build [-o <out>] [-g] [-v] <entry.am>` | Compile + gcc-link a runnable binary in one step (v0.7.9+). `-g`/`--debug` swaps `-O2` for `-O0 -g` (DWARF for `amc dap`) since v0.8.0. | `amc build --help` |
| `run [-o <out>] [-g] [-v] <entry.am> [-- args…]` | Build, then exec the binary (v0.7.9+) | `amc run --help` |
| `watch [-o <out>] [--run] [-v] <entry.am>` | Build now, then poll mtime and rebuild on change (v0.7.9+) | `amc watch --help` |
| `fmt [-w] file.am`     | Idempotent formatter (stdout, or `-w` in-place)        | `amc fmt --help` |
| `test [<dir>]`         | Discover `*_test.am`, compile + run, aggregate         | `amc test --help` |
| `lsp`                  | Workspace-aware LSP server over stdio JSON-RPC          | chap. 6 |
| `dap`                  | DAP proxy over stdio — forwards to `lldb-dap`/`gdb --dap` (v0.8.0+) | chap. 6 |
| `new <name> [--template exe\|lib\|test\|service] [--vscode]` | Scaffold a new project. `--vscode` adds `.vscode/launch.json` + `settings.json` (v0.8.1+). | `amc new --help` |
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
| `add <name\|url>[@<tag>] [--no-precompile]` | Clone + validate + record dep. Accepts `<name>@<tag>` (shortname resolved via the curated index) or a full `<git-url>@<tag>`. Since v0.6.0 the `@<tag>` is optional on **indexed shortnames**: amc auto-resolves the latest tag whose `required-amalgame` is satisfied by the running amc. Full git URLs still need an explicit `@<tag>`. `--no-precompile` (v0.5.4+) skips the install-time compile of `[stdlib].sources` even if the manifest declares `precompile = true`. |
| `remove <name>[@<tag>] [...]` | Strip dep(s) from `amalgame.toml` + `amalgame.lock`. The `@<tag>` safety suffix (v0.5.5+) refuses to remove unless the installed tag matches — guards against typo'ing the wrong version after an `update`. |
| `search <keyword> [--refresh]` | Substring match against indexed packages. Each result lists known tags with compat status (`✓`/`✗` vs running amc) and a `← latest compatible` marker. Index cache TTL is 30 min since v0.5.6 (auto-refresh); `--refresh` forces a re-fetch sooner. |
| `versions <name> [--refresh]` | Shortcut: same output as `search` filtered to one package. |
| `list` | Show installed packages with their pinned tag (since v0.5.5) — format: `<ClassName> @ <tag> — <slug>`. |
| `update <name>@<tag>` | Bump a pinned tag (delegates to `add` under the hood). |
| `cache clear [--all]` | Drop cached index (and packages with `--all`). |

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
Open     = { returns = "AmalgameDuckDB*" }
Close    = { returns = "void" }
# Optional `returns_generic` (v0.8.40+) carries the AM-level
# generic shape so chained `.Get(i).Get(j)…` on the consumer side
# infers typed casts instead of falling through to `(void*)`.
# Example: `QueryAll` returning rows-of-columns of strings.
QueryAll = { returns = "AmalgameList*", returns_generic = "List<List<string>>" }
# …
```

| Key                              | Where     | Since   | Purpose                                                                    |
| -------------------------------- | --------- | ------- | -------------------------------------------------------------------------- |
| `name`, `version`, `license`     | `[package]` | v0.5.0 | Required identity fields                                                   |
| `description`, `authors`         | `[package]` | v0.5.0 | Surfaced in `amc package list` / search                                    |
| `required-amalgame`              | `[package]` | v0.5.0 | Semver constraint on the running amc. Operators (v0.6.0+): `>=`, `>`, `<=`, `<`, `=`, `^` (caret — npm-style, 0.x-aware), `~` (tilde — locks `major.minor`), bare version treated as `>=`. E.g. `">=0.5.3"`, `"^0.6.0"`, `"~1.2.3"`. |
| `schema-version`                 | `[package]` | v0.5.3 | Refuses install when amc supports a lower manifest schema                  |
| `class`, `header`, `namespace`   | `[stdlib]`  | v0.5.0 | Required wiring for the cgen + resolver                                    |
| `sources`                        | `[stdlib]`  | v0.5.0 | Vendored `.c` / `.cpp` paths to compile + link                             |
| `cflags`                         | `[stdlib]`  | v0.5.3 | Extra gcc flags for the package's `.c` sources                             |
| `cxxflags`                       | `[stdlib]`  | v0.5.3 | Extra g++ flags for the package's `.cpp` / `.cc` / `.cxx` sources          |
| `libs`                           | `[stdlib]`  | v0.5.3 | Bare lib names → `-l<name>` appended to every consumer's final link        |
| `precompile`                     | `[stdlib]`  | v0.5.4 | When `true`, `amc package add` compiles sources at install time into `~/.amalgame/packages/.../build/<platform>/`. Subsequent `amc test`/`amc build` reuse the cached `.o`. Override with `--no-precompile`. |
| `[stdlib.functions]`             | section     | v0.5.0 | `<Method> = { returns = "<C-type>" }` — populates the cgen's dispatch     |
| `returns_generic` (per-function) | `[stdlib.functions]` | v0.8.40 | AM-level generic shape (e.g. `"List<List<string>>"`). When present, chained `.Get(i).Get(j)…` on the consumer infers typed casts (`(code_string)…` instead of `(void*)…`). Omit for non-collection returns or when one layer of `void*` is acceptable. |

### `required-amalgame` operators (v0.6.0+)

The constraint string follows npm/Cargo conventions. Examples:

| Constraint   | Matches                                                           |
| ------------ | ----------------------------------------------------------------- |
| `">=0.5.3"`  | At least 0.5.3 (current behaviour, back-compat)                   |
| `">0.5.5"`   | Strictly greater than 0.5.5                                       |
| `"<=0.6.0"`  | At most 0.6.0                                                     |
| `"<1.0.0"`   | Strictly less than 1.0.0                                          |
| `"=0.5.5"`   | Exact match                                                       |
| `"^1.2.3"`   | `>=1.2.3, <2.0.0` (major locked when major > 0)                   |
| `"^0.5.3"`   | `>=0.5.3, <0.6.0` (0.x minor locked, pre-stable rule)             |
| `"^0.0.5"`   | `=0.0.5` (0.0.x → exact patch, even stricter)                     |
| `"~1.2.3"`   | `>=1.2.3, <1.3.0` (always locks `major.minor`)                    |
| `"0.5.0"`    | Bare version — treated as `>=` for back-compat with v0.5 manifests |

Unrecognised operators are treated as "no constraint" (returns
true) to keep older amc binaries compatible with future syntax
extensions.

### Auto-resolve `amc package add <pkg>` (v0.6.0+)

When the `@<tag>` suffix is omitted on an **indexed shortname**,
amc fetches the index and picks the latest tag whose
`required-amalgame` is satisfied by the running amc:

```bash
$ amc package add duckdb
Auto-resolved 'duckdb' → 'duckdb@v0.1.1' (latest compatible with amc 0.6.0)
…
```

Full git URLs still require an explicit `@<tag>` — the index is
the SoT for "what amc knows about", and unindexed repos opt out
of auto-resolve by design.

If no version is compatible (e.g. all indexed tags require a
newer amc than you have installed), the command errors with a
hint to run `amc package versions <pkg>` for the full constraint
list.

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

## `amc build / run / watch` (since v0.7.9)

The first-class compile verb. `amc build` runs amc on the entry
file **and** invokes gcc with the right flags (runtime headers,
`lib/libamalgame.a`, installed package facade archives, vendored
package `.o` files, `[stdlib].libs` declared by each package).
The result is a runnable binary — no separate gcc step.

```bash
amc build hello.am             # produces ./hello
amc build -o myapp src/main.am # explicit output
amc build -v hello.am          # verbose: prints the gcc command
amc build -g hello.am          # debug build (-O0 -g for `amc dap`)
```

The `-g` / `--debug` flag (v0.8.0+) swaps `-O2` for `-O0 -g` on
both the single-stage gcc link and the C++ two-stage path
(DuckDB etc.). The cgen emits `#line N "foo.am"` directives at
every statement, so DWARF carries `.am` filenames and line
numbers — debuggers set breakpoints on `.am` source directly,
no source maps. See `amc dap` in chapter 6 and the debugger
workflow at the end of chapter 6.

`amc run` chains a build with `Process.Run` on the resulting
binary. Args after the `--` sentinel pass through to the user
program's argv:

```bash
amc run hello.am               # build then run, no user args
amc run server.am -- --port 8080 --verbose
```

`amc watch` builds once, then polls the entry's mtime every
500 ms and rebuilds on change. With `--run`, it also re-execs
the binary after each successful rebuild — handy for dev loops:

```bash
amc watch --run server.am      # rebuild + restart on save
amc watch hello.am             # rebuild only (manual ./hello to run)
```

v1 watches only the entry file explicitly named on the command
line. Transitive imports are out of scope until the
`Amalgame.IO.FileWatcher` package gains an event-based mode
(post-D in the roadmap).

The bare-args form `amc foo.am -o foo` still works (emits `foo.c`
only — no gcc step) and stays supported for the edge cases where
users want to splice their own gcc command (CI cross-compile,
custom flags, embedded targets).

## Typical workflows

### Smoke-test a single file

```bash
amc build hello.am && ./hello   # since v0.7.9
# OR the explicit two-step form
amc hello.am -o hello
gcc -Iruntime hello.c -lgc -lm -lcurl -lz -o hello
./hello
```

### Type-check on save (editor integration)

```bash
amc --check --no-color foo.am
echo $?     # 0 = OK, 1 = errors printed on stderr
```

### Build a multi-file project

Since v0.7.9 the single-entry `amc build` covers most cases —
amc resolves `import` statements transitively from the entry,
so you typically only need:

```bash
amc build src/main.am          # → ./main
```

For older v0.7.x (or when you want explicit control over the
file list), the two-step form still works:

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
