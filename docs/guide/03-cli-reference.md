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
| `migrate <file\|dir>`   | LLM-driven source-to-Amalgame translation               | chap. 8 |
| `generate "<prompt>"`  | LLM-driven prose-to-Amalgame                            | chap. 8 |
| `explain <file.am>`    | LLM-driven Amalgame-to-prose                            | chap. 8 |

Each subcommand handles its own flags and exit codes; consult the
referenced chapter or `--help` for the full surface.

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
