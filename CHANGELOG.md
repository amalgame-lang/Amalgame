# Changelog

All notable changes to Amalgame are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) loosely.

For releases prior to v0.3.2, see the git log and `ROADMAP_COMPLET.md`.

---

## [v0.4.13] — 2026-05-10

The "Service stdlib lands" release. Third stdlib brick after Path
(v0.4.11) and Logging (v0.4.12); last pre-requisite before the
`amc new --template service` scaffolder.

### Added

- **`Amalgame.Service`** (PR #256) — long-running process
  primitives wrapping POSIX `signal()` + `nanosleep()` (Linux/
  macOS) and `SetConsoleCtrlHandler` + `Sleep()` (Windows) behind
  the same Amalgame surface. Callers don't branch on platform.

  Surface: `Service.Install()`, `Service.ShouldStop()`,
  `Service.RequestStop()`, `Service.Sleep(ms)`. The typical loop:

      Service.Install()
      while (!Service.ShouldStop()) {
          Log.Info("heartbeat")
          Service.Sleep(5000)
      }

  Shutdown flag is `sig_atomic_t` on POSIX, `LONG` + `Interlocked-
  Exchange` on Windows — async-signal-safe and atomic respectively,
  no mutex needed for v1 single-process scope. POSIX `Service.
  Sleep` uses `nanosleep` (interruptible via EINTR); Windows
  slices into 100ms chunks (no portable equivalent for console
  signals).

### Roadmap

- `amc new <name> --template service` — now unblocked. Scaffolds
  a project using `Amalgame.Service` + `Amalgame.Logging` with
  a `<name>.service` systemd unit + `install.sh` for Linux v1.
- `--template forms` (cross-platform GUI) — SDL2 binding work
  tracked separately.

### Tests / infra

- Suite grows to **407 PASS / 0 FAIL / 0 SKIP** (was 403). 4 new
  cases drive the programmatic shutdown path (`RequestStop` flips
  the same flag the OS would). Direct SIGTERM delivery to the
  test runner would kill the suite, so the test exercises the
  post-flag state instead.
- Snapshot refreshed.

---

## [v0.4.12] — 2026-05-10

The "Logging stdlib lands" release. v0.4.11 added `Amalgame.Path`;
v0.4.12 adds the second stdlib brick on the way to `amc new
--template service`: leveled logging with a stderr + optional
file sink.

### Added

- **`Amalgame.Logging`** (PR #253) — `public class Log` with four
  levels (Debug / Info / Warn / Error), UTC ISO 8601 timestamps,
  fixed-width labels, optional file sink. Configuration is
  process-wide singleton state held in the runtime (`Logging_*`
  helpers), same pattern as `Exit_Set` / `Exit_Get`.

  Level names are case-insensitive on the first letter — "DEBUG",
  "Debug", "debug" all map. Unknown names default to "info"
  silently; logging itself should never crash a process. File
  sink reopens on each emit — slower than holding a stream open
  but robust against external log rotation, which is the typical
  production setup.

  Single-process, thread-unsafe v1 — fine for CLIs and single-
  threaded servers. Mutex is the v2 ask once a real multi-threaded
  user lands.

### Roadmap

- `Amalgame.Database` — minimum SQLite via libsqlite3.
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders. Now unblocked on the
  Path + Logging side.

### Tests / infra

- Suite grows to **403 PASS / 0 FAIL / 0 SKIP** (was 393). 10 new
  cases cover level round-trips, case-insensitive parsing,
  unknown-level fallback, file sink round-trip, and emit + level
  filter via the file sink.
- Snapshot refreshed.

---

## [v0.4.11] — 2026-05-10

The "Path stdlib lands" release. v0.4.10 closed LSP slice 4;
v0.4.11 pivots to the stdlib backlog and ships the first of
several gaps: cross-platform path manipulation as a proper
`namespace Amalgame.Path` facade.

### Added

- **`Amalgame.Path`** (PR #250) — `public class Path` exposing
  Combine, Extension, Filename, Directory, Stem, IsAbsolute,
  Normalize, and Sep. Wraps the existing C runtime `Path_*`
  helpers and adds four new operations (`Stem`, `IsAbsolute`,
  `Normalize`, `Sep`) for the gaps the runtime didn't cover.
  Normalize follows Go's `filepath.Clean` semantics — pure
  string operation, doesn't touch the filesystem, so it works
  on non-existent paths. Output canonical separator is always
  `/` — Windows accepts forward slashes everywhere, and a
  deterministic canonical form is what callers expect.

### Roadmap

- `Amalgame.Logging` — structured logging with levels, JSON +
  text formatters, console + file sinks. Tracked alongside Path
  as a pre-requisite for `amc new --template service`.
- `Amalgame.Database` — minimum SQLite via libsqlite3.
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders.

### Tests / infra

- Suite grows to **393 PASS / 0 FAIL / 0 SKIP** (was 372).
  21 new cases under `tests/run_stdlib_tests.sh` exercise every
  Path method + the documented edge cases.
- Snapshot refreshed.

---

## [v0.4.10] — 2026-05-10

The "LSP slice 4 closes" release. v0.4.9 finished slice 3 (rename
+ call hierarchy); v0.4.10 ships the next two features that ride on
the typechecker's per-node inference: inferred-type inlay hints and
the lightbulb / Quick Fix UI.

### Added

- **`textDocument/inlayHint`** (PR #246) — walks the open file's
  AST + runs the typechecker, emits one `InlayHint` per `VAR_DECL`
  that has an RHS but no annotation. Hint pinned right after the
  variable name, kind=1 (Type), paddingLeft=true so VS Code renders
  it as a greyed-out `name : type` decoration. Skips annotated
  decls, tuple destructuring, and decls whose inferred type is `?`.
- **`textDocument/codeAction`** (PR #247) — first quick fix lands:
  *Add type annotation*. When the cursor (or selection start) is on
  a `let x = …` whose annotation is empty and the typechecker can
  infer a concrete type, the server emits a `CodeAction` whose
  `WorkspaceEdit` inserts `: <type>` right after the variable name.
  Reuses the same inference path inlayHint already walks. The
  framework — dispatch + WorkspaceEdit emission — also unblocks
  future quick fixes (typechecker-driven annotation correction,
  unused-local removal, top-level-fn wrap-in-class) without
  another round-trip through the parser.

Capabilities advertised:
  `inlayHintProvider: true`
  `codeActionProvider: true`

### Roadmap

- Slice 5 LSP — tighter `selectionRange` (parser nameStart hook),
  more code actions (driven by linter / typechecker diags).
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot refreshed
  twice across the cycle (one per LSP-touching PR).
- `tests/run_tests.sh` lsp-init expected string updated for
  `inlayHintProvider:true` and `codeActionProvider:true`.

---

## [v0.4.9] — 2026-05-10

The "LSP slice 3 closes" release. v0.4.8 made navigation fast and
broad; v0.4.9 fills in the IDE features that ride on top of the
already-walked AST: rename's pre-flight check (so F2's UI stops
silently failing on punctuation) and the Call Hierarchy panel
(Shift+Alt+H).

### Added

- **`textDocument/prepareRename`** (PR #243) — F2's pre-flight
  check. Returns `{ range, placeholder }` when the cursor is on a
  renameable token (IDENTIFIER / MEMBER / CLASS_DECL / ENUM_DECL /
  METHOD_DECL / VAR_DECL / PARAM with a non-empty Name), null
  otherwise. VS Code now shows the proper "you cannot rename this
  element" message on punctuation/whitespace instead of silently
  noop-ing.
- **`textDocument/prepareCallHierarchy`** + **`callHierarchy/incomingCalls`**
  + **`callHierarchy/outgoingCalls`** (PR #243) — exposes the
  caller/callee graph for the method enclosing the cursor. Shift+Alt+H
  in VS Code lights up; the outgoing pass walks the method body
  collecting CALL nodes grouped by callee Name; the incoming pass
  walks every program for CALL nodes whose callee matches `item.name`
  and groups call sites by their enclosing METHOD_DECL. Items
  roundtrip name/uri/line/character via the spec's `data` field so
  follow-up requests re-locate the method without another search.

### Roadmap

- Slice 4 LSP work: tighter `selectionRange` (parser nameStart
  hook), inlay hints (parameter names, inferred types), code
  actions (organize imports, "extract to method").
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot refreshed
  for the new cgen output.
- `tests/run_tests.sh` lsp-init expected string updated for
  `renameProvider:{prepareProvider:true}` and `callHierarchyProvider:true`.

---

## [v0.4.8] — 2026-05-10

The "LSP gets fast + comprehensive" release. v0.4.7 made Cmd+Click
work; this release makes it instant, expands navigation to the
features users hit constantly (Find All References, Outline,
project-wide symbol search, parameter / local jumps), and patches
the VS Code extension wart that broke the natural `~/`-style
`serverPath` value.

### Added

- **`amc lsp` workspace resolver caching** (PR #232) — every
  hover / completion / definition / references request used to
  re-parse every `.am` in the workspace from scratch. On a 30-
  file repo that's 2.7s per Cmd+Click; VS Code shows the spinner.
  Now sibling files are parsed once on first use and reused
  across requests; the open file's freshly-parsed AST overrides
  the cached entry per request so editor-fresh text always wins.
  Probe before / after on three sequential definition calls:

      def #1: 0.061s    def #2: 0.037s    def #3: 0.040s

  ~45–70× speedup. Cache is keyed on the resolved workspace
  root; navigating to a different project rebuilds it once.
- **`textDocument/documentSymbol`** (PR #235) — Outline panel,
  breadcrumbs, and Ctrl+Shift+O fuzzy-find now light up for
  every `.am` file. Top-level: classes (kind 5), enums (kind 10).
  Class children: methods (kind 6), fields (kind 8). Enum
  variants surface as kind 22 (EnumMember).
- **`textDocument/typeDefinition`** + **`workspace/symbol`**
  (PR #236) — Ctrl+T project-wide symbol search across every
  cached `.am` file (case-insensitive substring filter), and
  Cmd+K F12 "Go to Type Definition" advertised. Type definition
  is identical to definition for now; diverges once the
  typechecker exposes per-node inferred types.
- **`textDocument/references`** (PR #237) — Find All References
  / Shift+F12 walks every program, recursively descends through
  every AstNode child slot (`Left/Right/Cond/Body/Else/Children/
  Params/Args`), collects every IDENTIFIER / MEMBER whose Name
  matches the symbol under the cursor. Probe shape on `NodeKind`
  in `parser/ast.am`: 433 hits across 9 files in 220ms.
- **`textDocument/definition` resolves params + local var-decls**
  (PR #238) — first pass searches the enclosing method's Params
  + recursively-walked Body for VAR_DECLs matching the lookup
  name, then falls through to the workspace top-level walk. Bug
  report: clicking on `model` at an assignment site in
  `migrate.am` returned null because `model` is a local. Now
  jumps to the `var model: string = ""` declaration.

### Fixed

- **VS Code extension: `amalgame.serverPath` trim + tilde-expand**
  (PR #234) — `child_process.spawn` doesn't run through a shell,
  so neither incidental whitespace nor `~/` were processed. A
  setting like `' ~/.local/bin/amc'` (leading space, tilde) failed
  with ENOENT and the LSP never started. New `resolveServerPath`
  helper trims, replaces a leading `~` with `os.homedir()`, falls
  through to `'amc'` if the resulting string is empty. Bumped
  extension version to 0.2.1.

### Roadmap

- Slice 3 LSP work staying on the roadmap: tighter
  `selectionRange` (parser nameStart hook), member-def in
  definition (jump to a method body when clicking on
  `obj.method()`), rename, call hierarchy, inlay hints, code
  actions. Each is sized to land as a small follow-up PR.
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders. Roadmap entries
  expanded in PR #232 with the SDL2 / systemd / launchd / SCM
  outline.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot
  refreshed multiple times across the cycle (each LSP-touching
  PR + the cgen patches needed a re-bootstrap so cold-start
  clones could parse the new sources).
- Run scripts updated to track every new LSP capability advertised
  in the initialize reply (`documentSymbolProvider`,
  `workspaceSymbolProvider`, `referencesProvider`,
  `typeDefinitionProvider`).

---

## [v0.4.7] — 2026-05-10

The "LSP gets useful" release. The IDE experience moves from
"diagnostics + hover + completion, with ~150 spurious typechecker
errors per workspace" to "diagnostics-clean + Cmd+Click navigates
to definition". Plus a one-line resolver fix that turned out to
be the root cause of the long-parked typechecker false-positives.

### Added

- **`textDocument/definition` + `textDocument/declaration`**
  (PR #227) — Cmd+Click jumps to the declaration site of any
  top-level symbol (class, enum, interface) reachable through
  the workspace resolver. Slice 1 of `amc lsp` v2 navigation
  per the roadmap. Slice 2 (member methods, fields, references)
  lands as a follow-up.
- **`Amalgame.Crypto` cleanup-only follow-up tooling**
  — `xs.Set(idx, value)` wired into the cgen as a side effect
  of investigating the resolver bug below. The runtime
  `AmalgameList_set` was already there; the cgen surface is
  new (PR #228).

### Fixed

- **Resolver: `MemberTable.Set` was `Values.Add`-ing on duplicate
  key** (PR #228) — single-line semantic bug that desynced the
  parallel `Keys` / `Values` arrays. Every subsequent lookup
  returned a value belonging to *another* (member, class) pair.
  The build path never tripped on it (each MemberTable instance
  saw each member once); workspace LSP did, because the open
  file is parsed once from didOpen text and *also* picked up by
  the workspace `find` scan, re-collecting its classes. LSP probe
  before / after across the 9 main compiler files: ~150 spurious
  typechecker diagnostics → **0**. Resolves the "37-case spurious
  typechecker on lexer.am via the LSP" item parked in
  ROADMAP_COMPLET since v0.4.4.
- **LSP: `UriToPath` percent-decodes `%XX` escapes** (PR #227) —
  VS Code sends URIs with percent-encoded UTF-8 for any non-ASCII
  byte in the path (`Développement` → `D%C3%A9veloppement`). The
  previous decoder kept the literal bytes, the workspace scan
  walked an inexistent path, the resolver fell back to
  single-file mode, and any cross-file type reference became an
  "Unknown symbol" diagnostic. Self-contained ~50 LoC pure-
  Amalgame decoder + hex helper. On `c_gen.am` alone: ~130
  spurious "Unknown symbol 'NodeKind'" → 0.
- **LSP: definition replies now percent-encode the response URI**
  (PR #229) — the inverse of the request-side fix. VS Code's
  DocumentURI parser silently rejects replies whose `uri` carries
  raw UTF-8 bytes; symptom was Cmd+Click "spinning forever" even
  though the LSP did respond. New `PercentEncodePath` walks the
  reply path byte-by-byte, leaves the URI-unreserved set
  (`A-Za-z0-9-._~/`) literal, percent-encodes everything else.
- **Cleanup: chain-mash typed-local workarounds removed**
  (PR #225) — now that PR #221 (constructor forward-decl) and
  PR #222 (`obj.Field.Method()` / `obj.Method().Method()`
  chain-mash) are in, the workarounds in `lsp.am`, `migrate.am`,
  and `stdlib/datetime.am::InstantResult` are mechanically
  unnecessary. Same PR uncovered + fixed two more cgen type-
  inference gaps (`Get`-handler chain receiver,
  `MethodRetGet` chain receiver) that the workarounds had been
  masking.

### Tooling

- **`tools/release.sh` patched in four spots** (PR #220) after
  walking through it manually for v0.4.5: README sed catches
  both bump patterns, back-merge from main runs before the
  develop → main PR, `--admin --merge` fallback for repos where
  auto-merge isn't available, CHANGELOG-edit prompt prefers
  `$EDITOR` and falls through cleanly when there's no tty.
- **`release-pdf.yml` lmodern fix + `workflow_dispatch`**
  (PR #219) — Ubuntu 24.04's `texlive-fonts-recommended` doesn't
  pull `lmodern`, which pandoc's default LaTeX template needs.
  v0.4.5 missed its PDF; v0.4.6+ have the asset attached on the
  first run. Manual trigger added so older tags can be reprocessed.

### Roadmap

- **`amc lsp` v2 navigation** (PR #226) — full inventory of the
  next-tier LSP features (declaration, type definition,
  references, hover preview, find-all-references, document
  symbols, workspace symbol, call hierarchy, rename, inlay
  hints, code actions). Slice 1 (definition) lands here; the
  rest is sized to drop in as small PRs.
- **`amc lsp` performance — workspace resolver caching**
  (PR #229) — every request currently re-parses the entire
  workspace (~2.5–3s on a 30-file repo). Caching with
  invalidation on didChange is the next perf win.
- **VS Code extension robustness on `serverPath`** (PR #229) —
  ~5 LoC fix in `extension.js`: trim whitespace + expand
  leading `~/` before `child_process.spawn` so the natural
  shell-style values just work.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot refreshed
  several times across the cycle (each cgen-touching PR + the
  resolver bug fix needed a re-bootstrap so cold-start clones
  could parse the new sources).

---

## [v0.4.6] — 2026-05-10

The "compiler quirk-cleanup" release. Three patches to long-known
parser/cgen sharp edges, plus an end-to-end fix to `tools/release.sh`
informed by walking through it manually for v0.4.5. No new user-
facing features; this is purely making the compiler emit cleaner C
and the release flow less twitchy.

### Fixed

- **Parser: `expr >> N` and `expr << N` no longer drop the shift**
  (PR #215). `ParseRelational` was correctly calling `ParseShift` on
  its RHS but went straight to `ParseAdd` on the LHS, so any
  expression starting with a shift got truncated. One-line fix
  restores the intended `equality > relational > shift > add > mul`
  precedence ladder. Cleaned up the `r / 256` workaround in
  `random.am`. `encoding.am` keeps `/` / `*` because the cgen
  doesn't yet preserve precedence parens around mixed `+` / shift
  expressions — separate roadmap item.
- **Parser: top-level `fn` now errors clearly** (PR #221). Before:
  `public fn foo()` outside a class parsed silently as Unknown,
  the body was swallowed, the call site compiled fine, the linker
  failed (or worse, called the C `double` keyword by accident).
  Now: `amc` exits 1 with `Top-level functions aren't supported
  (got 'fn foo' at L:C). Wrap it inside a class as 'public static'.`
  Drive-by: the regular compile path now actually checks
  `par.HasErrors()` after each parse — only `amc fmt` was doing
  it before.
- **CGen: constructor `_new(...)` forward-decls emitted before any
  class body** (PR #221). Before: a constructor whose body called
  `new B(...)` for a class declared later (or even earlier in the
  same file but emitted after) hit `gcc: implicit declaration` +
  `conflicting types` for `App_B_new`. Fix walks every class decl
  at pass-2 entry and emits its `_new` signature before any body.
  +316 forward declarations across the bundled compiler.
- **CGen: chain-mash on `obj.Field.Method()` and
  `obj.Method().Method()`** (PR #222). Two related bugs in
  `EmitCalleeStr` + the call-site self-injection block both caused
  the cgen to mash a member or call expression into the method
  name instead of resolving it through the typed receiver:

      Before                   →  After
      o.Field.Get()            →  Inner_Get(o->Field)
                                 (was: o->Field_Get())
      o.GetInner().Doubled()   →  Inner_Doubled(Outer_GetInner(o))
                                 (was: Outer_GetInner(o)_Doubled())

  The typed-local workarounds (`let mid: T = obj.Field; mid.Method()`)
  in `lsp.am`, `migrate.am`, `stdlib/json.am`, `stdlib/datetime.am`
  are now mechanically unnecessary; cleanup deferred to a separate
  PR to keep the diff focused.

### Tooling

- **`tools/release.sh` patched in four spots** (PR #220) after
  walking through it manually for v0.4.5:
    1. `sed` for README now also bumps the `Current version: **vX.Y.Z**.`
       overview line, not just the `# amc X.Y.Z` comment.
    2. New "Back-merge main into develop" step before the
       develop → main PR — prevents the conflict cascade when main
       carries a merge commit develop never absorbed (the standard
       outcome of any previous develop → main PR with conflict
       resolution).
    3. `--admin --merge` fallback retried on each poll tick if
       `--auto --merge` rejects (this repo's main branch doesn't
       allow auto-merge with merge-commit strategy).
    4. CHANGELOG-edit prompt uses `$EDITOR` if set, falls back to
       the read-from-tty pattern, finally warns + continues if
       neither path works (was hanging non-interactive runs).

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP** — cleanup release,
  no new test surface added. Snapshot refreshed three times during
  the cycle (after each cgen-touching PR) so cold-start clones
  bootstrap from a parser+cgen consistent with the new sources.

---

## [v0.4.5] — 2026-05-10

The "infrastructure cleanup + Crypto" release. Drops the Vala
bootstrap entirely now that the snapshot is the sole bootstrap
entry point, adds `Amalgame.Crypto` (SHA-256 + HMAC), fixes a
silent shift-drop bug in the parser, and wires a release-time
PDF guide build. 14 commits across 6 PRs.

### Added

- **`Amalgame.Crypto`** (PR #213) — SHA-256 (FIPS 180-4) and
  HMAC-SHA-256 (RFC 2104) primitives in two static facades:
  `Sha256` for the bare hash, `Hmac` for keyed authentication.
  Pure-C runtime header `runtime/Amalgame_Crypto.h` (~150 lines,
  no external crypto dep). API:

      Sha256.Bytes(data: List<int>)    -> List<int>   # raw 32 bytes
      Sha256.Hex(data: List<int>)      -> string      # hex
      Sha256.OfString(s: string)       -> string      # UTF-8 → hex

      Hmac.Sha256(key, msg: List<int>)        -> List<int>
      Hmac.Sha256Hex(key, msg: List<int>)     -> string
      Hmac.Sha256OfStrings(key, msg: string)  -> string

  Tested against FIPS 180-4 SHA-256 vectors ('abc', empty,
  100×'a' multi-block) and RFC 4231 HMAC-SHA-256 cases 1–2.
  Documented caveat: constant-time MAC comparison is the
  caller's responsibility.

- **`release-pdf` GitHub Actions workflow** (PR #211) — on every
  `v*` tag, concatenates `docs/guide/0*.md` (chapters 01..08) and
  builds `amalgame-guide-vX.Y.Z.pdf` via pandoc + xelatex,
  attached to the GitHub Release. The amalgame.me /releases page
  picks it up automatically.

### Changed

- **README + guide code blocks** (PR #216) — 61 ` ```amalgame `
  fences swapped to ` ```kotlin ` as a syntax-highlight fallback.
  GitHub Linguist doesn't recognise `amalgame` yet (roadmap entry
  added), and Kotlin shares enough keywords (`let` / `var` /
  `class` / `null` / `for x in xs`) to highlight ~80% of the
  syntax usefully.

### Fixed

- **Parser: `expr >> N` and `expr << N` no longer drop the shift**
  (PR #215). `ParseRelational` was correctly calling `ParseShift`
  on its RHS but went straight to `ParseAdd` on the LHS, so any
  expression starting with a shift got truncated. One-line fix
  restores the intended `equality > relational > shift > add >
  mul` precedence ladder. Cleaned up the `r / 256` → `r >> 8`
  workaround in `random.am`. `encoding.am` keeps `/` / `*` for
  now since its slices appear inside mixed `+` expressions where
  the cgen-precedence-parens issue would change C evaluation
  order — separate roadmap item.

### Removed

- **Vala bootstrap** (PR #210) — `archive/vala-bootstrap/`
  (Vala compiler sources, meson.build, ~13 500 LoC), `compile.sh`
  (Vala build wrapper), `install/release.sh` (meson-based release
  builder, superseded by `release.yml`), top-level `install.sh`
  (stale stub referencing `codec --version` and the pre-rename
  repo URL), `bootstrap/` directory (unused 2 MB Vala binary +
  README describing commands that don't exist), and 6
  `docs/transpiler/*.md` v0.1.0 design docs (described the
  deleted impl). The bootstrap chain is now 2-rung — `./amc →
  ./snapshot/amc` — and a clean clone cold-starts in one `gcc`
  invocation. The pre-removal state is preserved at the
  `vala-bootstrap-final` git tag if revival is ever needed.
  Net diff: 30 files removed, ~13 569 lines deleted.

### Roadmap

- **Stdlib gaps — second tier** (PR #214) — new ROADMAP_COMPLET
  section inventorying modules a complete stdlib usually has
  and Amalgame doesn't yet: Audio (miniaudio binding sketch),
  Database (SQLite), Path manipulation, Logging, WebSocket,
  Filesystem watcher, advanced Math (Vec/Mat/BigInt), other
  serialization formats (TOML/YAML/MessagePack).
- **Submit Amalgame to GitHub Linguist** (PR #216) — long-term
  fix for the markdown highlight fallback; we already have the
  TextMate grammar in `editors/vscode/syntaxes/`.

### Tests / infra

- Suite is now **372 PASS / 0 FAIL / 0 SKIP** (+9 from
  `Amalgame.Crypto`).
- `snapshot/amc_lib.c` refreshed twice during the cycle: once
  after Crypto wiring (new resolver globals), once after the
  parser fix (so cold-start clones don't hit the old shift-drop
  on the new `random.am`).

---

## [v0.4.4] — 2026-05-10

The "stdlib trio" release. Three new modules — `Amalgame.Random`,
`Amalgame.Encoding`, `Amalgame.DateTime` — close the most-cited
gaps in everyday Amalgame code. Plus a release script that
automates the version bump → tag flow so the v0.4.0-style
"forgot to bump src/main.am" mistake can't happen again.

### Added

- **`Amalgame.Random`** (PR #200) — instance-based PCG XSH-RR
  64/32 PRNG. `new Random(seed)` for reproducibility,
  `Random.FromSystem()` for crypto-grade entropy via the OS pool
  (`getentropy` on POSIX, `BCryptGenRandom` on Windows).
  Methods: `NextUInt32`, `NextInt`, `IntRange(min, max)`,
  `Float`, `Bool`, `Bytes(n)`. Static `Random.SystemBytes(n)`
  for one-off salt / nonce generation. Replaces the legacy
  `Math.Random` 8-bit-output LCG for new code; the old API
  stays for compatibility.
- **`Amalgame.Encoding`** (PR #201) — three small static
  facades: `Base64` (RFC 4648 + URL-safe variant), `Hex`
  (lower / upper, case-insensitive decode), `Url`
  (percent-encoding per RFC 3986, plus an `EncodeComponent`
  matching JS's `encodeURIComponent`). 100% pure Amalgame, no
  runtime header. Bytes flow as `List<int>` ∈ [0, 255], so
  encoding `Random.Bytes(n)` output works without a cast layer.
- **`Amalgame.DateTime`** (PR #202) — `Instant` (i64 ns since
  1970-01-01 UTC, range ±292 years), `Duration` (signed ns
  interval), `Stopwatch` (monotonic-clock convenience for
  benchmarks). RFC 3339 strict parse + format (UTC only —
  `+HH:MM` offsets rejected; local time / timezones tracked
  as a v2 follow-up).
- **`tools/release.sh`** (PR #203) — one-shot release script
  that bumps the version everywhere, builds + tests + saves a
  snapshot, opens the release → develop and develop → main PRs
  with `gh pr merge --auto --merge`, polls until each lands,
  then tags from main and pushes. `--dry-run` and `--no-tag`
  flags for inspection. Closes the manual-checklist gap that
  caused v0.4.0's missed bump.
- CI Windows build links `-lbcrypt` for `Random.FromSystem()`'s
  `BCryptGenRandom` call. Same flag added to the Windows release
  artifact build.

### Changed

- `runtime/` gains `Amalgame_Random.h` + `Amalgame_DateTime.h`
  next to the existing headers. The PCG step (`Random_PcgOutput`
  / `Random_PcgAdvance`) keeps the multiply unsigned because
  signed overflow is UB on Amalgame's `int` = i64.
- `docs/guide/04-stdlib.md` gains Random, Encoding, and DateTime
  sections — each with the full method table and the sharp-edge
  caveats (modulo bias for IntRange, RFC-strict `+` handling for
  Url.Decode, ±292-year window for Instant).

### Roadmap (parser/cgen findings, not fixed in this release)

Surfaced while writing the trio; workarounds applied inline,
real fixes tracked in `ROADMAP_COMPLET.md`:

- Parser drops `expr >> N` inside a `let` initializer (lowers
  as `i64 x = expr;`). Workaround: division by 2^N.
- Top-level `public fn(...)` outside a class parses but cgen
  never emits a definition. Workaround: hang the helper on a
  class as `public static`.
- Constructors that instantiate a later-declared class trigger
  an implicit-decl conflict because `_new` forward-decls aren't
  emitted before pass2 bodies. Workaround: pass the dependency
  in as a constructor parameter (used by `InstantResult`).
- 25 stale `BastienMOUGET/Amalgame` URLs scattered across
  runtime/ headers, install/, and archive/ — to be swept after
  this release. Tracked under "URL sweep" in Distribution.

### Tests / infra

- Suite is now **363 PASS / 0 FAIL / 0 SKIP** (was 307 at
  v0.4.3). +56 assertions split as +14 Random / +21 Encoding
  / +21 DateTime.
- Snapshot refreshed; `snapshot/amc_lib.c` is ~16 500 lines
  reflecting the three new modules + the resolver builtins
  for the new runtime helpers.

---

## [v0.4.3] — 2026-05-09

The "Json migration completes" release. Phase 2 final + phase 3
of the Amalgame.Json plan (the compiler + LLM commands now go
through the proper parser, not substring tricks), plus
`amc migrate v3` real cost reporting on top.

### Changed

- **`src/lsp.am`** request dispatcher: switched from the ad-hoc
  `JsonStr` / `JsonInt` substring extractors to a single
  `Json.Parse(body)` walked via structured `Get(...).At(...)`
  chains. Each LSP message is now parsed once with a real JSON
  parser; the helpers are gone.
- **`src/migrate.am`** response parsing: each provider now reads
  the actual JSON shape of its response instead of the first
  `"text":"..."` substring trick:
    - Anthropic → `root.content[0].text`
    - OpenAI → `root.choices[0].message.content`
    - Gemini → `root.candidates[0].content.parts[0].text`
  Each path returns an attributed error if the shape doesn't
  match, instead of a silent empty string.
- **`amc migrate` real cost line** (v3): after a successful
  migration, the real per-call cost is printed alongside the
  pre-flight `--dry-run` estimate. No `~` prefix, no heuristic —
  exact tokens billed, pulled from each provider's `usage`
  object:

      [migrate] wrote src/api.am
      [migrate] cost: 6431 in + 982 out = $0.04 (claude-sonnet-4-6)

  CLI shell-out (`claude`, `custom`) still reports nothing since
  the bill is on the user's subscription / local backend.

### Removed

- `lsp.am::EscapeJsonStr` / `JsonStr` / `JsonInt`
- `migrate.am::JsonEscape` / `JsonExtract` / `JsonExtractText`

Six functions, ~150 LoC out. The compiler ships with one JSON
parser (`Amalgame.Json`) instead of two-and-a-half overlapping
substring matchers.

### Fixed

- The earlier-noted "Json.Parse hangs on 16 KB bodies" turned
  out to be a benchmark artefact — bash `${#body}` counts
  characters, not bytes, so the LSP probe shipped a frame that
  under-counted UTF-8 multibyte content by the byte/char delta.
  Real client traffic uses byte-accurate counts and the parser
  handles a typical 16 KB didOpen body in ~37 ms.

### Documentation

- `docs/guide/08-llm-commands.md` — "Cost estimation" renamed to
  "Cost reporting", documents both the pre-flight `--dry-run`
  estimate and the post-flight real cost.
- `docs/proposals/amc-migrate.md` — v3 cost reporting moved from
  "deferred" to "partial". API streaming via SSE remains the
  only deferred v3 item.
- `ROADMAP_COMPLET.md` — `Promote ad-hoc JSON to a real
  Amalgame.Json module` marked resolved.

---

## [v0.4.2] — 2026-05-09

The "stdlib + DX" release. Adds a real JSON module, a project
scaffolder, and IDE-friendly member completion. Suite goes from
**263/263** to **307/307**.

### Added

- **`Amalgame.Json`** — first-class JSON parser, encoder, and
  accessor surface. Strict RFC 8259 with full escape support
  (incl. `\uXXXX` + surrogate pairs). Class-with-tag `JsonValue`
  with `Is*` / `As*` / `Get` / `At` / `Length` / `Keys`. Static
  `Json.Parse` / `Encode` / `EscapeString` / `Of*` factories.
  Lives in the new `src/stdlib/` directory. Replaces 4 ad-hoc
  substring extractors in the compiler in a future PR (phase 2);
  this release ships the library standalone. Direct prep for
  `amc migrate v3` cost reporting via nested `usage.input_tokens`.
  [PR #182, #183]

- **`amc new <name>`** — project scaffolder à la `cargo new`.
  Three templates: `exe` (default — `src/main.am` + a passing
  test), `lib` (public class skeleton), `test` (test bundle
  only). Each generates a working `build.sh` that locates the
  Amalgame runtime via `$AMALGAME_HOME` / install dirs / `which
  amc`, plus README, `.gitignore`. Path-aware: `amc new
  path/to/foo` works (basename is the namespace stem). Refusal
  on existing dirs unless `--force`. [PR #184]

- **LSP member completion** — `obj.<cursor>` now narrows to the
  methods/fields of the receiver type. Two-step receiver
  resolution: global-symbol lookup (covers `Json.<cursor>`,
  user-class names) then a local-decl text scan covering
  `let x = new T(...)` / `let x: T = ...`. Falls back to the
  global list for `this.<cursor>`, chained calls, builtin C
  modules. [PR #185]

- **Editor integration on install** — roadmap item added (auto-
  wire VS Code `.vsix` / Neovim lspconfig snippet / Helix
  `languages.toml` entry on first install). [PR #181]

### Documentation

- `docs/proposals/amalgame-json.md` — full design doc for the
  JSON module: motivation, type design, API surface, 3-phase
  migration plan, testing strategy, decisions captured in review.
  [PR #182]

- `ROADMAP_COMPLET.md` — LSP enum-lookup item marked resolved
  (verified empirically: 0 "Unknown symbol" diagnostics on
  `lexer.am` via `amc lsp`). New typechecker item parked with
  investigation findings on the spurious "Return type mismatch"
  cases. [PR #181, #186]

### Infrastructure

- Repo transferred from `BastienMOUGET/Amalgame` to
  `amalgame-lang/Amalgame`. All code, docs, install scripts, and
  publication recipes updated to the new URL. GitHub redirects
  the old URLs for ~1 year; this lands canonical references
  ahead of that window. [PR #187]

- `tests/run_stdlib_tests.sh` extended with a 5th-arg
  `extra_inputs` so per-module tests can pull in a stdlib source
  file alongside the sample (used by the Json suite). [PR #183]

- `tests/run_amc_new_tests.sh` added and wired into
  `run_all_tests.sh`. 20 assertions: file/dir presence per
  template, end-to-end compile + run for `exe`, error paths.
  [PR #184]

### Known issues (unchanged from v0.4.1)

- Typechecker reports spurious "Return type mismatch" on enum-
  member returns in some `lexer.am` IF-body shapes (37 cases via
  the LSP). Doesn't reproduce on minimal cross-file repros.
  Investigation findings captured in `ROADMAP_COMPLET.md`; next
  step documented (instrument `CheckReturn`).

---

## [v0.4.1] — 2026-05-09

Patch release. The v0.4.0 binary still printed `amc 0.3.6` from
`amc --version` because `src/main.am`'s hardcoded version string
wasn't bumped before the tag. This release fixes the string and
adds a CONTINUATION.md pitfall (#12) so the next contributor
catches it pre-tag.

No other changes — same code as v0.4.0 modulo the version literal
and the snapshot refresh. CONTINUATION.md adds the pitfall.

---

## [v0.4.0] — 2026-05-09

The "LLM-driven workflows" release. Three new subcommands shipped in
one big push: **`amc migrate`** (translate other languages → Amalgame),
**`amc generate`** (write a new `.am` from a natural-language prompt),
**`amc explain`** (read a `.am`, write prose). Plus a batch of
compiler / parser / CGen fixes that the LLM-generated code surfaced
along the way. Suite goes from **187/187** to **263/263**.

### New subcommands

- **`amc migrate <file|dir>`** *(PRs #149, #158, #161, #162, #156, #166, #167, #169, #170)*
  - Auto-detects 21 source languages by extension (TS, JS, Python,
    Java, C#, Go, Rust, C, C++, Kotlin, Swift, Ruby, PHP, …).
  - Provider abstraction with five built-ins:
      - `claude` (Claude Code CLI shell out — default fallback)
      - `claude-api` (Anthropic HTTP via `Http_PostWithHeaders`)
      - `chatgpt` (OpenAI HTTP)
      - `gemini` (Google AI Studio HTTP)
      - `custom` (delegates to `AMC_CUSTOM_PROVIDER_CMD`, reads stdin
        / writes stdout — wraps any local LLM CLI)
  - Provider auto-selection from env: `ANTHROPIC_API_KEY` →
    `claude-api`, `OPENAI_API_KEY` → `chatgpt`, `GEMINI_API_KEY` →
    `gemini`, fallback `claude` (CLI).
  - Directory recursion. Result cache by SHA-256(source + system
    prompt) at `~/.cache/amalgame/migrate/`. Per-file `amc --check`
    validation (suppressible). Source-size cap (default 2000 lines).
  - Cost estimation in `--dry-run` per provider + model.
- **`amc generate "<prompt>"`** *(PR #164)* — same provider stack,
  output to stdout by default. `--stream` flows the response live
  via the claude CLI.
- **`amc explain <file.am>`** *(PR #165)* — reverse direction.
  `--lang <name>` controls the explanation language.

### Compiler — language

- **Lambda v2.5 — non-int signatures** *(PR #142)*. Higher-order
  list operations like `xs.Map(x => x.Name)` over a `List<Class>`
  now work end-to-end. The TypeChecker patches the lambda's `PARAM`
  type from the higher-order callsite (so `x` resolves as `Class`
  inside the body), and the CGen splits `EmitOneLambda` into
  forward + body emission so `lam_N_fn` can dereference fields on
  user struct types.
- **Multi-line method chains** *(PR #154)*. `xs\n    .Filter(...)\n    .Map(...)`
  parses cleanly. `ParsePostfix` peeks past `NEWLINE` when the next
  non-NL token is `.` or `?.`. Statement boundaries (NEWLINE not
  followed by a dot) stay intact.
- **TS-style param syntax** *(PR #152)*. `Foo(id: int)` is accepted
  as an alias for `Foo(int id)`. Same fix also resolves a
  pre-existing infinite-loop on the malformed input. The parser
  param-list while-loops gain a "did the cursor advance?" safety
  belt.
- **CGen auto-qualify implicit field access** *(PR #155)*. Within a
  class method, `Id = id` lowers to `self->Id = id` when `Id` isn't
  a local but is a field of the current class. Lets users from
  C# / TS / Kotlin / Swift skip the explicit `this.` qualifier.

### Stdlib

- **`Env.Get(name)` / `Env.Has(name)`** *(PR #160)*. Wraps
  `Environment_GetVar` / `_HasVar`. Required by the API providers
  to read API keys from the environment.
- **`Http_PostWithHeaders` / `Put` / `Delete` / `Patch` return
  type tracked** *(PR #160)*. `resp.Status` / `.Body` / `.Ok`
  type-check correctly after these calls (previously only
  `Http_Get` / `Post` / `GetWithHeaders` / `GetTimeout` /
  `PostJson` were registered).

### Tooling

- **LSP — workspace-aware diagnostics** *(PR #146)*. `amc lsp`
  now scans every `.am` file under the detected workspace root
  (markers: `.git`, `build_amc.sh`, `package.json`) and feeds them
  to the resolver. Eliminates ~44 false `Unknown symbol` diagnostics
  per file on `src/typechecker.am` (NodeKind, SourceSnippet, …).
  Hover / completion benefit too.
- **`amc migrate --help` / `-h`** *(PR #156)* — the discoverability
  flag actually prints help instead of erroring.

### Distribution

- **Release artifacts now bundle `docs/language/grammar.ebnf` +
  `docs/guide/02-language-tour.md`** under `share/amalgame/docs/...`
  so `amc migrate / generate / explain` find them at
  `<exec-dir>/../share/amalgame/docs/...` post-install. Without
  these, the LLM commands degrade gracefully to an inline
  conventions block.
- `install.sh` copies them to `$PREFIX/share/amalgame/docs/`.
- `build_amc.sh` now links `gen_test` against `-lcurl` (the bundled
  `migrate.am` pulls in curl symbols transitively).

### Documentation

- New chapter **`docs/guide/08-llm-commands.md`** — user-facing
  reference for migrate / generate / explain.
- New design doc **`docs/proposals/amc-migrate.md`** — full v0/v1/v2
  rationale, kept up-to-date with what shipped.
- CONTINUATION.md and ROADMAP_COMPLET.md refreshed.

### Refactor & contributor DX

- **`merge=ours` + `linguist-generated` for generated artefacts**
  *(PR #173)*. `.gitattributes` declares `merge=ours` for
  `snapshot/amc_lib.c`, `snapshot/INFO.md`, and `src/amc_lib.c`
  — the canonical resolution on conflict is "rebuild from
  sources", not three-way merge. Eliminates the manual `--theirs`
  → rebuild → re-snapshot → amend dance that ate ~6 PRs in the
  v0.4 cycle. One-time post-clone setup:
  `git config merge.ours.driver true`. Documented as pitfall #12
  in CONTINUATION.md.
- **CGen: `BoxAsVoid` / `UnboxScalar` helpers extracted** *(PR #174)*.
  The `(void*)(intptr_t)X` boxing dance and symmetric unbox were
  duplicated across ~17 sites; consolidated into two helpers so
  intent reads at a glance. Behavior unchanged — emitted C is
  byte-identical pre / post.
- **Parser-loop audit** *(PR #175)*. Verified that every `while`
  loop calling a sub-parser already has a `lastPos` watchdog
  (added preventively in PR #152). No new hazard found.

### Cleanup

- `use.sh` (pre-self-hosted doc snippet) removed.
- `.github/workflow/` typo directory removed (was duplicate of
  `.github/workflows/`, ignored by GitHub anyway).

---

## [v0.3.6] — 2026-05-08

The "lambdas in the wild" release. Higher-order `List<T>`
methods now take a lambda directly: `xs.Filter / .Map / .Reduce
/ .ForEach / .Any / .All / .CountIf`. Suite goes from 180/180
to **187/187** under `./amc`.

### Stdlib

- **Higher-order List methods** — Filter, Map, Reduce (with a
  `(acc, x) → acc` reducer dispatched through `Closure_call2`),
  ForEach, Any, All, CountIf. Each takes an `AmalgameClosure*`
  so the captured environment travels with the lambda.
  Implemented as static-inlines in `runtime/_runtime.h` and
  `runtime/Amalgame_Collections.h`. The closure struct +
  `_callN` wrappers are hoisted to the top of `_runtime.h` so
  the helpers a few lines down can use them; the
  `AmalgamePredicate` / `AmalgameAction` typedefs from v0.3.4
  are gone (no caller left). *(PR #133)*

### Compiler

- **CGen — closure-as-argument lowering** — `TryEmitListCall`
  recognises the seven new method names and lowers each via a
  new `EmitClosureArg` helper. The lambda is passed inline as a
  GCC compound-statement-expression that allocates the env,
  copies captured locals in, and yields a fresh
  `AmalgameClosure_new(...)`. `EmitLambdaCaptureCopy` is a tiny
  helper around the per-capture line so the self-host codegen
  sees `cap.Name` through a typed parameter (chained
  `.Get(i).Name` would erase to `void*` at the call boundary).
- **CGen — type inference for higher-order returns** —
  `InferTypeFromExpr` knows that a CALL whose member name is
  Filter / Map returns `AmalgameList*`, Any / All return
  `code_bool`, CountIf returns `i64`. So
  `let big = xs.Filter(...)` typechecks without an explicit
  annotation, and `EmitVarDecl` propagates the receiver's
  list-element type to the result so `big.Get(0)` lowers with
  the right cast. *(PR #133)*

### Documentation

- `docs/guide/02-language-tour.md` — Lambdas section gains a
  higher-order `Filter / Map / Reduce` example and a rewritten
  limitations note around the i64 boundary.
- `docs/guide/04-stdlib.md` — List section gains the table of
  higher-order methods.
- `docs/guide/05-runtime-and-interop.md` — new "Closures and
  higher-order calls" section with the AmalgameClosure struct,
  the three `_callN` wrappers, and the GCC compound-statement-
  expression idiom the CGen emits.
- `ROADMAP_COMPLET.md` — Lambda v2.5 split into a `[x]` partial
  (this release) + a `[ ]` non-int signatures pending. *(PR #134)*

### Known limitations (deferred to v2.5 final)

- Lambda arguments and results are still `(i64) → i64` at the
  C level. `xs.Map(x => x.Name)` over a `List<Class>` doesn't
  yet work — needs a TypeChecker layer that infers the lambda
  signature from the formal param at the call site, plus CGen
  to emit non-int `lam_N_fn` signatures.
- String interpolation `"x: {coll.Count()}"` doesn't yet
  propagate the inferred `AmalgameList*` to the embedded call.
  Workaround: stage in named locals before printing.
- ForEach mutating an enclosing `var` doesn't accumulate
  (closures capture by value). Reduce is the right tool.

## [v0.3.5] — 2026-05-08

The "developer experience" release. Three coordinated pieces:
**generic interfaces** with static contract verification,
**LSP hover + global completion** on top of the v0.3.4 diagnostics,
and **lambda v2** (multi-param + block body) on top of v0.3.4
single-param closures. Suite goes from **170/170 to 180/180**
under `./amc`.

### Language

- **Generic interfaces** — `interface IComparable<T> { Compare(T) -> int }`
  now type-parameterise. Classes declared `class Box implements
  IComparable<int>` go through a new `CheckImplementsContract`
  pass in the TypeChecker that walks every implemented interface,
  substitutes its generic params by the concrete args (recursing
  into nested generics like `List<T>` → `List<int>`, preserving
  trailing `?` nullability), and asserts every interface method
  exists on the class with the matching signature. Diagnostics
  point at the offending method/param with expected vs got
  types. Two new fields on `AstNode` (`Str3` for generic params
  CSV, `Str4` for the implements list CSV). No vtable / dynamic
  dispatch yet — duck-typed dispatch as before. *(PR #120)*

- **Lambda v2: multi-param + block body** — `(x, y) => x + y`
  and `x => { let d = x * 2; return d + 1 }` now parse and
  compile. Multi-param routes through new `AmalgameClosure_call2`
  / `_call3` runtime helpers; block bodies emit through `EmitBlock`
  with an `InLambdaBody` flag that boxes `RETURN_STMT` values as
  `void*`. The lambda AST node migrates from `Str=name` to
  `Params: List<PARAM>` to support N parameters. v1 closures
  (capturing single-param expression-bodied) keep their existing
  semantics. Lambdas in argument position with non-int signatures
  (e.g. `xs.Filter(x => x > 0)`) need a lambda-typing layer in
  the TypeChecker — that lands in v2.5. *(PR #129)*

### Tooling

- **`amc lsp` hover + global completion** — the LSP server
  shipped in v0.3.4 (diagnostics-only) now responds to
  `textDocument/hover` and `textDocument/completion` too. Hover
  walks the AST via a new `FindNodeAtPosition` helper to find
  the deepest named node covering the cursor, then returns its
  inferred type as Markdown via the new `TypeChecker.LookupNodeType`.
  Completion lists every global the resolver knows about
  (builtins + user classes / enums / functions), mapped to LSP
  `CompletionItemKind` (Class / Function / Variable). The VS
  Code client picks up the new capabilities automatically via
  the initialize handshake — no client changes. Member completion
  (`obj.<cursor>` narrowed to the receiver's type) and
  goto-definition / signature help remain follow-ups. *(PR #126)*

### Documentation

- **Stdlib expansion roadmap** — `ROADMAP_COMPLET.md` gains a
  "Core stdlib expansion" backlog entry listing eight modules
  everyday Amalgame code currently has to fake or shell out for:
  `DateTime`, `Json`, `Regex`, `Random`, `Encoding`, `Compress`,
  `Crypto`, `Threading`. Plus a new "Stdlib delivery model"
  open design question that lays out five alternatives to the
  current header-only-inline approach. *(PR #121)*

### Internal

- **Typed accessors on FullResolver** — `ProgramCount` /
  `ProgramAt` (used by the contract verifier), then `GlobalCount`
  / `GlobalNameAt` / `GlobalTypeAt` (used by the completion
  provider). Both batches work around the same self-host quirk
  where chained member access through a `List<T>` field emits
  `Programs_Count(...)` instead of `Programs->Count(...)`.
- **`-Wint-conversion` hotfix** — `let pn = ...ProgramCount()`
  in `FindInterface` lost its int type to `void*` erasure across
  the method call boundary. Pinned via explicit `let pn: int`,
  matching the precedent in `src/lsp.am` for `String_Length`
  results. Local builds had ignored the warning; CI on Linux
  and macOS treats it as an error. *(PR #124)*

## [v0.3.4] — 2026-05-08

The "tooling" release. Five new pieces ship together: capturing
closures in the language, two new lint checks, a `Process` stdlib
module, an `amc test` runner, and a minimal LSP server (`amc lsp`)
paired with a VS Code client. Suite goes from **150/150 to
170/170** under `./amc`.

### Language

- **Capturing closures** — `let f = x => x + n` now captures `n`
  (and any other enclosing local) by value at creation time, not
  by textual substitution. Replaces the v0.3.x macro path
  (`#define f(x) (n+x)`) which couldn't carry an environment.
  Pieces: a new `AmalgameClosure { fn, env }` runtime type with
  `Closure_new` / `Closure_call1` helpers; capture analysis in
  the resolver (free identifiers below `LambdaBoundary`); a
  per-lambda `LamEnv_N` struct + `lam_N_fn` top-level function
  emitted by a Phase A walk of CGen Pass 2; identifier-callable
  closures dispatched via `AmalgameClosure_call1` with arg/result
  boxing through `intptr_t`. v1 is single-param, expression-bodied,
  arity-1 `(i64) -> i64`. Multi-param, block bodies, lambdas in
  argument position, and non-int signatures are tracked for v2.
  *(PR #101)*

### Tooling

- **`amc --lint` extensions** — two new categories on top of the
  v0.3.3 unreachable-code check:
  - **Unused locals** (`let x = …` never read; prefix `_` to silence)
  - **Shadowed names** (a `let` reuses a name visible in an
    enclosing scope, including method params)

  The linter keeps its own scope stack with append-only
  `UsedNames` + per-local snapshot, sidestepping the missing
  `List<T>.Set` op. Method/lambda params participate in shadow
  detection but are never warned-on as unused. *(PR #104)*

- **`Process` stdlib module** — `Process.Run(cmd) -> int` and
  `Process.RunCapture(cmd) -> AmalgameProcessResult { Exit,
  Stdout, Stderr }`. Cross-platform via `popen`/`_popen`; exit
  codes decoded with `WEXITSTATUS` / `WTERMSIG` (signals
  surface as `128 + signum`, shell convention). Stderr is
  currently merged into Stdout via shell `2>&1`; v2 will split
  them with a real pipe pair. *(PR #106)*

- **`amc test [<dir>]`** — discovers `*_test.am` files under
  `<dir>` (default `.`), compiles + runs each via `Process`, and
  aggregates `[PASS] <name>` / `[FAIL] <name>: <msg>` /
  `[SKIP] <name>` lines from each child's stdout. Crash-with-no-tags
  shows up as `[FAIL] <crash> exit=N`. Tests stay framework-free
  (just normal `Main`s that print tag lines). Exits non-zero on
  any FAIL or compile error. *(PR #109)*

- **`amc lsp` server + VS Code client** — minimal LSP 3.x server
  speaking JSON-RPC 2.0 over stdio. v1 implements `initialize` /
  `shutdown` / `exit`, `textDocument/didOpen` / `didChange` /
  `didClose` (Full sync), and `textDocument/publishDiagnostics`
  (push on every did{Open,Change}). Diagnostics merge resolver
  and typechecker errors and underline the whole token at the
  error column. JSON handling is ad-hoc — `JsonStr` / `JsonInt`
  helpers find `"<key>"` and read the value, no real parser.
  Hover and completion are out of scope for this release.

  Editor side: `editors/vscode/` (now version 0.2.0) gains an
  `extension.js` that spawns `amc lsp` via `vscode-languageclient`.
  Configurable via `amalgame.serverPath` (default `amc`) and
  `amalgame.enableLsp` settings. `npm install` is a one-time
  prerequisite. *(PR #110)*

### Compiler

- **Lexer-bootstrap CR escape fix** — the bootstrapping amc binary
  lexed `"\r"` as the two chars `\` + `r` instead of a single CR
  (0x0d) byte, which made `EscapeStringForC`'s `String_Replace(s,
  "\r", "\\r")` a no-op and propagated through `lsp.am`'s framing,
  causing the LSP wire to emit literal `\r\n` (5c 72 0a) instead
  of CR LF (0d 0a). VS Code's LanguageClient never matched the
  framing and stayed in Starting forever. Fixed by routing CR
  through `String_FromByte(13)`, which is just an int→byte builtin
  and has no escape-table dependency; same idea for `\x1b` (ESC).
  *(PR #110)*

- **CGen return-type table extended** — `Console_{Read,Flush,Write}`,
  `Process_{Run,RunCapture}`, and the `String_*` int/bool returns
  (`Length`, `IndexOf`, `LastIndexOf`, `ToInt`, `StartsWith`,
  `EndsWith`, `Contains`, `IsEmpty`) gain explicit entries. Fixes
  a pre-existing latent bug: an unannotated
  `let n = String_IndexOf(...)` defaulted to `code_string`, breaking
  arithmetic on the result. *(PR #110)*

- **Resolver structured errors** — new
  `RawErrors: List<ResolverError>` kept in lock-step with the
  formatted `Errors: List<string>`. Exposes `(file, line, col, msg)`
  for downstream tools (the new LSP, future IDE features). The
  formatted-string CLI path is unchanged. *(PR #110)*

### Runtime

- **`Console_ReadBytes(n)`** — reads exactly `n` bytes from stdin.
  Used by the LSP framing reader after parsing `Content-Length`.
- **`Console_Flush()`** — drains stdout. Required by the LSP
  server so the client doesn't block on buffered replies.
- **`AmalgameClosure { fn, env }`** + `AmalgameClosure_new` /
  `AmalgameClosure_call1` — backbone of the new closure feature.
- **`AmalgameProcessResult { Exit, Stdout, Stderr }`** + the
  `Process_Run` / `Process_RunCapture` helpers.

### Tests / infra

- Suite is now **170/170** (108 core + 50 stdlib + 12 fmt) under
  `./amc` — +20 vs v0.3.3. New samples: `tests/samples/closures_capture.am`,
  `lint_unused_shadow.am`, `process_api.am`,
  `test_runner/{arith,mixed}_test.am`. New helpers in
  `run_tests.sh`: `run_amc_test_check`, `run_lsp_check`.

### Docs

- `docs/guide/07-internals.md` gains sections on the **test runner**
  (`amc test`) and the **LSP server** (`amc lsp`, `src/lsp.am`).
  The Linter section is rewritten to cover the new unused / shadow
  checks.
- `docs/language/grammar.{ebnf,md}` — full rewrite. The previous
  files dated from v0.1.0 (back when the language was called *CODE*)
  and described constructs the parser never implemented while
  missing everything added since (try/catch, match-as-expression,
  closures, list comp, null-safety, decorators, generics, …). The
  new EBNF mirrors `src/parser/parser.am` exactly.
- README adds bullets for `--lint`, `amc test`, and `amc lsp`.

---

## [v0.3.3] — 2026-05-08

Quality-of-life follow-up to v0.3.2. Cleans up cosmetic typechecker
noise during multi-file compilation, ships a first `amc --lint`
flag (basic static analysis), and stops the workflow from leaking
a stale v0.3.0 artefact into every release. Suite goes from
149/149 to **150/150**.

### Compiler

- **Multi-file false-positive cleanup** — `build_amc.sh` step 1
  invokes `amc` on ten compiler sources at once, and the
  typechecker was emitting ~93 spurious errors on that path
  (build still went through, but the noise drowned out anything
  real). Three small fixes in `CheckProgram` / `CheckReturn`:
  the per-program filename is now read from `prog.Str2` rather
  than the constructor argument; `ExprType*`, `LocalNames`/
  `Types` and `ScopeStarts` reset between programs; and bare
  `return` (parsed as `Return(_unknown_, …)`) is recognised as
  such instead of being treated as a value-return.
  `build_amc.sh` is now silent.

### Tooling

- **`amc --lint`** — new flag that runs a static-analysis pass on
  top of the parsed AST and emits non-fatal warnings. MVP catches
  unreachable code after `return` / `throw` / `break` / `continue`,
  including inside nested `if` / `while` / `for-in` / `try` bodies.
  The skeleton (`src/linter.am`) is set up to grow more checks
  (unused locals, shadowed names, …) without touching anywhere
  else.

### CI

- **Drop stale v0.3.0 artefact, stop tracking `dist/`** — a
  `dist/amc-0.3.0-linux-x86_64.tar.gz` had been committed during
  v0.3.0 and was being uploaded on top of every later release by
  the workflow's `dist/*.tar.gz` glob. Removed, and `dist/` is now
  in `.gitignore` so future stagings don't sneak back in.

### Tests / infra

- Suite is now **150/150** (88 core + 50 stdlib + 12 fmt) under
  `./amc`. New samples: `tests/samples/lint_test.am`. New helper:
  `run_lint_check` in `run_tests.sh`.

---

## [v0.3.2] — 2026-05-08

Closes every open compiler bug tracked in v0.3.1's `SKIP_SELFHOST`
list, restores `try` / `catch` / `throw` / `finally` end-to-end in
the self-hosted parser, and ships the first round of generic type
inference for `List<T>` and `Map<K,V>`. The full test suite is
**149/149 green with zero SKIPs**.

### Language

- **`try` / `catch` / `throw` / `finally`** restored end-to-end in the
  self-hosted compiler (the Vala bootstrap had it; the rewrite hadn't
  grown a `ParseTry` yet). Lowers via `setjmp`/`longjmp` using the
  existing `_am_throw` runtime helpers.
- **`Type.Variant` patterns in `match`** — `Direction.North => …` is
  now recognised as a pattern. `ParseMatchPattern` reads an optional
  `.IDENT` suffix.
- **Generic type inference (locals + params + returns)**:
  `let xs = new List<int>()`, `xs: List<int>`, `xs = MakeList()`,
  and `Foo(List<int> xs)` all carry the elem type, so `xs.Get(i)`
  lowers to `(int)AmalgameList_get(xs, i)` without a manual cast.
  Same for `Map<K,V>.Get(k)` (which wasn't even dispatched before).
- **`obj.Method()` instance syntax for strings** — `s.Length()`,
  `"foo".Trim()`, `s.Replace(a, b)`, … route through `String_*`.

### Compiler

- **Multi-file type checking** — TypeChecker now walks every parsed
  program (was hard-coded to `programs[0]`).
- **Null-safety** — three independent bugs fixed: NodeKey collision
  in the typechecker (Kind is now part of the key), bare `?` was
  dropped by the lexer (added `OP_QMARK`), and `TypeToC("T?")` no
  longer doubles up the C pointer.

### Formatter

- **Same-line comments** preserved on round-trip: `let x = 1  // foo`
  no longer sees its comment bumped to the next line.
- **`import` directives** preserved: the parser used to discard them.
- `try` / `catch` / `throw` round-trip lossless.

### Tests / infra

- Suite is now **149/149** (87 core + 50 stdlib + 12 fmt) under
  `./amc`. **`SKIP_SELFHOST` is empty.**
- `run_multifile_test` shell helper fixed (was passing `-o foo.c`,
  amc dutifully expanded it to `foo.c.c` and never produced an
  executable).
- `tests/run_all_tests.sh` completes end-to-end for the first time
  (its `set -e` no longer trips on a half-failing suite).

[v0.4.13]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.13
[v0.4.12]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.12
[v0.4.11]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.11
[v0.4.10]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.10
[v0.4.9]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.9
[v0.4.8]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.8
[v0.4.7]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.7
[v0.4.6]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.6
[v0.4.5]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.5
[v0.4.4]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.4
[v0.4.3]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.3
[v0.4.2]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.2
[v0.4.1]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.1
[v0.4.0]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.0
[v0.3.6]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.6
[v0.3.5]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.5
[v0.3.4]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.4
[v0.3.3]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.3
[v0.3.2]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.2
