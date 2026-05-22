# Proposal: `amc dap` MI bridge (Approche A)

**Status:** design (2026-05-22). Implementation in progress on this branch.
**Author:** Track C of the post-v0.8.40 priority sequence.
**Tracking PRs:** TBD.
**Replaces:** the `execvp`-based proxy that has shipped since v0.8.0
(`src/dap.am`, Approche C in the original DAP strategy decision).

## Why a bridge, not a proxy

The v0.8.0 `amc dap` is a 175-LoC proxy: it `execvp`s either `lldb-dap`
(macOS, Linux with LLVM apt) or `gdb --dap` (Linux/MSYS2 with gdb ≥ 14)
with the user's argv tail forwarded as-is. Once exec'd the kernel
replaces our process image — amc is gone, no message inspection, no
rewriting. The cgen-emitted `#line N "foo.am"` directives let the
backend's DWARF reader map .am source lines natively, so basic
stepping + breakpoints already work in VS Code's debug pane.

What the proxy **cannot** do:

1. **Pretty-print Amalgame runtime types.** `AmalgameList*` shows as
   `(AmalgameList *) 0x7f3...`. To inspect contents the user has to
   manually call `(int)((AmalgameList*)0x...)->len` etc. — viable but
   tedious. Same for `AmalgameMap*`, `AmalgameSet*`,
   `AmalgameClosure*`, `code_string` (a `const char*` with no length
   prefix but typically C-string compatible).
2. **Filter out runtime frames.** Stepping into a `xs.Add(42)` lands
   the user in `AmalgameList_add`, then `GC_malloc`, then the GC's
   internals. The call stack is dominated by `_runtime_*` and
   `Amalgame_*` symbols that aren't useful for the user's mental
   model. They want to see *their* `.am` frames, not the .h's.
3. **Decode closure captures.** A capturing lambda compiles to
   `LamEnv_N` struct + `AmalgameClosure { fn, env }`. The user sees
   `(AmalgameClosure *) 0x...` and would have to know the synthetic
   struct's name to inspect captures.

The bridge intercepts every DAP request/response and rewrites the
relevant ones. Same DAP-side API, richer payloads.

## Architecture

```
┌──────────────┐  DAP/JSON-RPC  ┌──────────────┐  gdb MI3  ┌──────────┐
│  DAP client  │ <───────────>  │   amc dap    │ <───────> │   gdb    │
│ (VS Code,    │     stdio      │  (bridge)    │ pipes +   │ --inter- │
│  Neovim, …)  │                │              │  poll(2)  │ preter=  │
└──────────────┘                └──────────────┘           │   mi3    │
                                       │                  └──────────┘
                                       │
                              ┌────────┴─────────┐
                              │ pretty-printer   │  registry keyed
                              │     registry     │  on C type name
                              └──────────────────┘
                              ┌──────────────────┐
                              │  frame filter    │  hide ^_runtime_,
                              │                  │  ^Amalgame_ unless
                              └──────────────────┘  --show-runtime
```

### Process model

Replace `execvp` with `fork + pipe2 + poll`:

- `pipe2(stdin_fd, O_CLOEXEC)` + `pipe2(stdout_fd, O_CLOEXEC)` for the
  gdb child's stdin/stdout. stderr stays on the parent's stderr so
  gdb's internal complaints surface in the editor's debug console.
- `fork()`, child `dup2`s the pipes and `execvp("gdb",
  ["gdb", "--interpreter=mi3", program_path])`.
- Parent enters a `poll(2)` loop with three fds:
  - stdin (DAP from client)
  - stdout (DAP to client) — only needed for write-readiness; we never
    read from it.
  - gdb-stdout (MI from gdb)
- On stdin readable: parse one DAP frame (Content-Length header +
  JSON body), translate to MI command, write to gdb-stdin.
- On gdb-stdout readable: parse MI records up to the next `(gdb)`
  prompt, translate to DAP response/event, write to stdout.

The poll loop runs in the parent. The fork+pipe2 setup goes in a
single `@c { ... }` block in `src/dap.am` since amc has no native
fork/pipe primitives.

### Fallback: `amc dap --raw`

Keep the `execvp` proxy reachable behind a `--raw` flag. Users who
hit a bridge bug, or who want lldb-dap on macOS without the bridge
overhead, can fall through to native behaviour. The bridge becomes
the default for `amc dap` on Linux + MSYS2 (where gdb is the backend);
macOS stays on lldb-dap proxy until LLVM gains an MI-equivalent or we
write an LLDB-script-interpreter variant of the bridge.

## gdb MI3 — input/output primer

MI is line-oriented. Input commands look like:

```
1-break-insert foo.c:42
2-stack-list-frames 0 5
3-var-create - * xs
```

Each command starts with a numeric **token** (chosen by the bridge —
the same token appears in the response so we can correlate). Followed
by a `-`-prefixed verb, optionally followed by space-separated args
(quoted strings, integers, locations).

Output records come in three classes:

- **Result records** start with `^` and carry the token of the command
  they answer:
  ```
  1^done,bkpt={number="1",type="breakpoint",disp="keep",enabled="y",addr="0x...",func="foo",file="foo.c",line="42"}
  ```
  Status word after `^`: `done` (success), `error` (failed),
  `running` (target is running).

- **Async records** start with `*` (exec-async), `+` (status-async),
  or `=` (notify-async). They have no token (the gdb event isn't a
  reply to anything):
  ```
  *stopped,reason="breakpoint-hit",frame={addr="0x...",func="foo",...},thread-id="1"
  =thread-group-added,id="i1"
  ```

- **Stream records** start with `~` (console output), `@` (target
  output), or `&` (log output). The payload is a single C-style quoted
  string:
  ```
  ~"Breakpoint 1 at 0x4007a0: file foo.c, line 42.\n"
  ```

Every gdb response ends with a literal `(gdb)\n` prompt — the bridge
reads until it sees that line, then treats everything before it as
one logical response.

### MI value grammar

MI values nest. Three shapes:

- **C-string**: `"..."` with backslash escapes (`\"`, `\\`, `\n`,
  `\t`, `\xHH`, octal). NOT JSON — the escape set differs slightly.
- **Tuple**: `{ key=value, key=value, ... }`.
- **List**: `[ value, value, ... ]` or `[ key=value, key=value, ... ]`
  (yes, key-value lists exist; e.g. `stack=[frame={...},frame={...}]`).

Top-level results after `^done,` are comma-separated `key=value`
pairs at tuple scope.

The MI parser in `src/dap/mi_parser.am` is a recursive-descent
walker that builds an `MiValue` discriminated record matching this
grammar.

## DAP ↔ MI translation table

The MVP covers the 10 DAP requests that VS Code's debug pane fires
during a normal session. Anything else passes through unchanged to
gdb (gdb will reject unknown commands with `^error` — we let DAP see
that).

| DAP request          | MI command(s)                        | Notes                                                                                            |
|----------------------|--------------------------------------|--------------------------------------------------------------------------------------------------|
| `initialize`         | (none — local handling)              | Return our own capabilities JSON, advertise nothing not yet implemented.                          |
| `launch`             | `-file-exec-and-symbols`, `-exec-run`| The program path comes from the launch config. `noDebug=true` skips gdb entirely (just exec it). |
| `setBreakpoints`     | `-break-delete-list`, `-break-insert`| Tear down per-source breakpoints, re-create from the request's list. Returns Breakpoint[].       |
| `configurationDone`  | (none)                               | Acknowledge; we already started the program in launch.                                            |
| `continue`           | `-exec-continue`                     | Returns immediately; the `*stopped` event fires when the target hits a breakpoint or signals.    |
| `next`               | `-exec-next`                         | Step over.                                                                                       |
| `stepIn`             | `-exec-step`                         | Step into. Frame filter still applies — see below for "step until visible frame".                |
| `stepOut`            | `-exec-finish`                       | Step out.                                                                                        |
| `pause`              | `-exec-interrupt`                    | SIGINT-style.                                                                                    |
| `stackTrace`         | `-stack-list-frames 0 N`             | Filtered through the frame filter unless `--show-runtime`.                                       |
| `scopes`             | (synthesised)                        | Return three: Locals (var-id `locals-<frame>`), Arguments (`args-<frame>`), Registers (skipped). |
| `variables`          | `-stack-list-variables --simple-values` for locals scope; `-var-list-children` for object children | Apply pretty-printer registry on every variable before emitting. |
| `evaluate`           | `-data-evaluate-expression`          | Pretty-print the result via the registry.                                                        |
| `disconnect`         | `-gdb-exit`                          | Reap the child, exit cleanly.                                                                    |

### Frame translation

`*stopped` carries a `frame={...}` field. We respond to the implicit
`stackTrace` query (the client always calls it after every `*stopped`)
by listing N frames, filtered.

A frame is "visible" iff its `func` field doesn't match either of:

- `^_runtime_` — synthetic prefix used by exception/setjmp helpers
  emitted by the cgen.
- `^Amalgame_` — every runtime header function (`AmalgameList_add`,
  `String_Concat`, `GC_*` from libgc don't match but the user wants
  to see them anyway? No — `GC_*` are added to the hidden list too).

`amc dap --show-runtime` disables the filter so plumbing-level
debugging stays accessible. The flag is parsed in `Run()` and stored
as a `bool` on the `DapServer` instance.

### Step-into through hidden frames

`stepIn` on a line that calls `xs.Add(...)` lands in
`AmalgameList_add` (hidden frame). The user expects to either land
on the next .am line (which is what `next` does) or to skip ahead
until a visible frame appears. The bridge implements
"step-until-visible": after each `-exec-step` we read the resulting
`*stopped` frame; if its `func` is hidden, fire another `-exec-step`,
up to a small budget (8 hops) to avoid infinite loops in pure runtime
paths. If still hidden after 8 hops, surface the actual frame so the
user isn't stuck in a silent loop.

## Pretty-printer registry

Keyed by exact C type name (the `type` field gdb reports for a
variable). Default registrants:

| C type                | Display                                                                       |
|-----------------------|-------------------------------------------------------------------------------|
| `AmalgameList *`      | `List<?>[count]` summary + indexed children `[0]`, `[1]`, …                   |
| `AmalgameMap *`       | `Map<?,?>[count]` summary + keyed children, each `[key]=value`                |
| `AmalgameSet *`       | `Set<?>[count]` summary + indexed children                                    |
| `AmalgameClosure *`   | `λ(env_ptr=0x…)` summary; children expose the captured fields by name        |
| `code_string`         | The string value, double-quoted (gdb's default is already close; we add length)|

A registrant is a function with signature
`PrintResult Print(MiValue gdbResult, MiState state)`, returning the
DAP-shape variable JSON. `MiState` carries the bridge's gdb session
so the printer can issue follow-up MI commands (e.g.
`-data-evaluate-expression` for child lookups).

The element type for `AmalgameList<T>` is inferred from the parent
variable's name via the cgen's `__local__` / `__local_raw__` tracking
— except that information lives in the compiler, not in DWARF. v1
displays `List<?>`; v2 either generates a DWARF helper map or queries
the cgen's emitted metadata file alongside the binary.

The registry itself is a `List<PrettyPrinter>` populated at `Run()`.
Third-party packages can add entries via a future
`Amalgame.Debug.RegisterPrinter` facade.

## Error and edge-case handling

- **gdb exits unexpectedly**: detect on `poll(2)` returning POLLHUP
  for `gdb-stdout`; emit a `terminated` DAP event with `restart=false`
  and exit cleanly.
- **Malformed MI**: log the offending bytes to stderr (gdb's stderr
  channel) and keep parsing. A parse failure on one record shouldn't
  kill the session.
- **Pretty-printer crash**: wrapped in a try/catch (Amalgame-level);
  on failure, fall back to gdb's default representation for that
  variable and log the error.
- **Bridge crash**: the DAP client treats stdout EOF as session-end.
  The bridge installs a `SIGPIPE` handler that turns broken-stdout
  into a graceful exit instead of a crash.

## File layout

```
src/dap.am               # DapServer entry, --raw / --show-runtime flags
                         # `Run()` dispatches to `RunBridge()` or `RunRaw()`
src/dap/mi_parser.am     # MiValue type, MiParser class
                         # ParseRecord(line) → MiRecord
                         # ParseValue(input) → MiValue (recursive)
src/dap/dap_parser.am    # DAP frame reader (Content-Length header
                         # + JSON body), DapRequest type, JSON to AM
src/dap/translator.am    # DAP request → MI command; MI response → DAP
                         # response. Per-verb handlers (HandleLaunch,
                         # HandleStackTrace, …).
src/dap/pretty.am        # PrettyPrinter base, default registrants
src/dap/frame_filter.am  # ShouldHideFrame(funcName, showRuntime: bool)
```

Initial estimate: ~1200-1500 LoC of new AM + ~80 LoC of `@c {}` glue
for the fork+pipe2+poll loop.

## Testing strategy

- **Unit tests** for the MI parser: a `tests/samples/stdlib_dap_mi.am`
  file with canned MI inputs (sample `^done` records, `*stopped`
  events, nested tuples/lists) verified against expected AM values.
- **Smoke test** for the bridge end-to-end: a `tests/dap/run_bridge.sh`
  spawns `amc dap` against a tiny sample program, drives it through a
  scripted DAP session (initialize → launch → setBreakpoints →
  configurationDone → wait for stopped → stackTrace → variables →
  continue → terminated), and diffs the responses against a fixture.
  Requires gdb on the host; gated via an `AMC_DAP_SMOKE=1` env-var
  so CI lanes without gdb skip cleanly.
- **VS Code manual session**: documented in
  `docs/guide/06-build-and-tooling.md` (debug section) — a checklist
  of the 6 things to verify before tagging a release that touched
  the bridge: breakpoint binds on .am line, step-over lands on next
  .am line, locals show pretty-printed Lists/Maps, call stack filters
  runtime frames, evaluate works, disconnect cleans up.

## Migration plan

1. **Phase 1 — design doc + scaffolding.** This doc + empty
   `src/dap/*.am` files + refactored `DapServer.Run()` that dispatches
   to either `RunRaw()` (existing execvp path) or `RunBridge()`
   (initially a stub that calls `RunRaw`). Ship behind `--bridge` flag
   while it's experimental.
2. **Phase 2 — fork+pipe2+poll skeleton.** The bridge can launch gdb,
   pipe DAP through unchanged (round-trip), and exit cleanly. No
   translation yet — just plumbing.
3. **Phase 3 — MI parser.** Standalone, unit-tested, covers the
   grammar above.
4. **Phase 4 — DAP↔MI translation MVP.** The 10 requests above.
   Bridge handles a full VS Code debug session end-to-end on a
   sample program.
5. **Phase 5 — pretty-printer registry.** AmalgameList, AmalgameMap,
   AmalgameSet, AmalgameClosure, code_string.
6. **Phase 6 — frame filter + step-until-visible.**
7. **Phase 7 — default bridge.** Flip `--bridge` from opt-in to
   default; `--raw` becomes the opt-out.

Phases 1-3 are safe to ship together (no user-visible behaviour
change since the bridge stays opt-in). Phase 4 is where a real debug
session starts being possible. Phases 5-7 are pure UX wins on top.

## Rejected alternatives

- **lldb scripting language**: lldb-dap can load Python pretty-printers
  via its built-in interpreter. Equivalent functionality, but Linux
  hosts that don't have LLVM installed (the common case) still need
  the gdb path. Doing both means writing the same logic twice in
  different languages; one bridge in AM covers both backends when we
  get to writing the lldb side (Phase 8, much later).
- **gdb Python pretty-printers**: shipped as `runtime/amalgame-pretty.py`
  and auto-loaded via the `<binary>-gdb.py` mechanism. Solves the
  pretty-print half of the bridge with much less code (~150 LoC of
  Python). Considered for v1 but rejected because (a) it doesn't help
  with frame filtering at the DAP layer — that's a `stackTrace`
  rewrite, not a printer; (b) it requires Python on the host, which
  Windows MSYS2 doesn't always ship by default; (c) it ties Amalgame
  to gdb specifically, leaving lldb users without parity. Stays
  available as a quick win for users who don't want the bridge.

## Open questions

- **Element-type inference for `AmalgameList<T>`.** v1 displays
  `List<?>`. The cgen's `__local_raw__` table has the answer but
  isn't visible at DWARF level. Options for v2: (a) emit a
  `.amc-debug-info` file alongside the binary, parsed by the bridge;
  (b) bake type names into DWARF via gcc's
  `__attribute__((debug_info))` or a synthetic typedef per locally
  typed List/Map; (c) accept "best-effort: same display as the
  enclosing local's declared raw type via name match". Pick the
  cheapest that works.
- **Conditional breakpoints.** DAP `setBreakpoints` requests can carry
  `condition` (expression evaluated at break point). MI supports this
  via `-break-condition`. Out of MVP scope; add in a follow-up.
- **Multi-thread**: the runtime is single-threaded today (bdwgc is
  configured for the main thread only — see "Open design questions:
  Single-threaded" in ROADMAP_COMPLET.md). Threading proposal would
  add MI's thread-list/select primitives. Out of scope until the
  language has real threads.
