# Editor tooling & debugging

Amalgame ships two servers that turn a plain editor into an IDE:

- **`amc lsp`** — a Language Server (diagnostics, completion, go-to-def,
  hover, rename, …). Your editor talks to it over stdio.
- **`amc dap`** — a Debug Adapter: real breakpoints, stepping, and
  variable inspection in your `.am` source, via gdb/lldb under the hood.

Both are wired up automatically by the VS Code extension. This chapter is
the user-facing setup + workflow; for how the DAP bridge works internally
(gdb-MI translation, `#line` directives) and the full prerequisites
table, see [06-build-and-tooling.md](06-build-and-tooling.md).

---

## Editor setup

### VS Code

Install the Amalgame extension (the release tarball bundles the `.vsix`;
`install.sh` auto-installs it on a detected VS Code). Opening a `.am`
file starts `amc lsp` automatically.

The extension reads these settings:

| Setting | Purpose |
|---------|---------|
| `amalgame.serverPath` | path to the `amc` used as the LSP server (default `amc` on PATH; set to `${workspaceFolder}/amc` to use a local build) |
| `amalgame.enableLsp` | turn the language server on/off |
| `amalgame.dapServerPath` | `amc` used for debugging (empty → falls back to `serverPath`) |
| `amalgame.dapBridge` | bridge mode on/off (default on; see below) |
| `amalgame.dapShowRuntime` | show runtime/GC helper frames in the call stack (default off) |

### Other editors

Any LSP-capable editor (Neovim, Helix, Emacs/eglot, …) can drive
`amc lsp` over stdio — point its language-server client at the `amc`
binary with the `lsp` argument. The protocol is stock LSP; no Amalgame
specific client is required.

---

## What the language server gives you

`amc lsp` implements the navigation + editing surface you'd expect:

- **Diagnostics** — published on open and on every change (parse + type
  errors, lints).
- **Completion** — including `import Amalgame.<cursor>` listing installed
  + bundled packages.
- **Hover** and **signature help**.
- **Go to definition / declaration / type definition**.
- **Find references** and **document highlight** (highlight every use of
  the symbol under the cursor).
- **Document symbols** (outline) and **folding ranges**.
- **Rename** across the document, **inlay hints**, **code actions**
  (e.g. "add the import for this package"), and **call hierarchy**.

> **Known false positive.** The LSP can report `Unknown symbol` on types
> that come from an external package, even when `amc build` compiles
> cleanly — the editor doesn't always load the project lockfile. Trust
> the build; the diagnostic is the gap, not your code. (Also noted in
> [12-packages.md](12-packages.md).)

---

## Debugging a program

`amc dap` (since v0.8.0) is a Debug Adapter Protocol server. The flow:

### 1. Build with debug info

```bash
amc build -g myprog.am -o myprog     # -g embeds DWARF
```

The code generator emits `#line N "myprog.am"` directives, so the
debugger maps the running C back to your `.am` source — no separate
source-map file.

### 2a. In VS Code

With the extension installed, create a launch configuration of type
`amc` pointing at the built binary (build first with `amc build -g`), set
breakpoints in the `.am` file, and start debugging. The extension spawns
`amc dap` for you.

By default `amc dap` runs in **bridge mode**: amc drives `gdb` itself and
translates DAP↔gdb-MI in-process, so `List`/`Map`/`Set`/`Closure`
variables pretty-print with summaries and drill-down, and runtime helper
frames (`Amalgame_*`, `GC_*`) are hidden from the call stack. On a system
without gdb it falls back transparently to the legacy proxy; `amc dap
--raw` forces the proxy explicitly (lldb-dap on macOS, `gdb --dap` on
Linux/MSYS2 with gdb ≥ 14).

### 2b. From the CLI (no editor)

You can debug with a stock debugger directly:

```bash
amc build -g hello.am
lldb ./hello
(lldb) breakpoint set --file hello.am --line 5
(lldb) run
```

Because the binary carries DWARF with `.am` line info, breakpoints and
stepping work against your source, not the generated C.

---

## Driving the servers manually

Both servers speak their protocols over stdio, which is handy for
debugging the tooling itself. The LSP expects `Content-Length`-framed
JSON-RPC:

```bash
echo -e 'Content-Length: 41\r\n\r\n{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}' | amc lsp
```

You won't normally do this — the editor does — but it's the quickest way
to confirm `amc lsp` is alive and which `amc` is being used.

---

## Gotchas

- **`-g` is required for source-level debugging.** Without it the binary
  has no DWARF and breakpoints won't bind to `.am` lines.
- **Bridge mode needs gdb** (Linux/MSYS2, gdb ≥ 14); the raw proxy needs
  `lldb-dap` (macOS) or `gdb --dap`. On a box with neither, debugging is
  unavailable even though the program runs fine — install one (see the
  prerequisites table in [06-build-and-tooling.md](06-build-and-tooling.md)).
- **Runtime frames are hidden by default.** If you *want* to see
  `Amalgame_*` / GC frames in the call stack, set
  `amalgame.dapShowRuntime`.
- **The LSP `Unknown symbol` false positive** on package types is
  expected — defer to `amc build`.

For the build pipeline, the snapshot bootstrap, and the DAP internals,
see [06-build-and-tooling.md](06-build-and-tooling.md); for the CLI verbs
themselves, [03-cli-reference.md](03-cli-reference.md).
