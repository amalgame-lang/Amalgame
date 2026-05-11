# Continuation prompt — start a new chat with this

> Last refreshed 2026-05-11 after shipping v0.5.2 (package CLI
> grouping under `amc package <action>`, `amc test` package-
> aware + auto-link vendored sources). Rolls v0.5.0 + v0.5.1 +
> v0.5.2 into one bootstrap snapshot.
>
> The block below is a self-contained prompt designed to bootstrap a
> new Claude session with full context. Copy-paste it as your first
> message in a fresh conversation.

---

```
I'm working on Amalgame, a self-hosted programming language that
transpiles to C. I keep the project in
/home/neitsab/Développement/Amalgame.

Current state (May 2026, v0.5.2):

═══════════════════════════════════════════════════════════════
  Compiler + bootstrap
═══════════════════════════════════════════════════════════════

- The compiler `amc` is written in Amalgame in src/ and compiles
  itself in ~3 seconds via ./build_amc.sh.
- 2-rung bootstrap chain in build_amc.sh:
    ./amc                  → current self-hosted (may break in dev)
    ./snapshot/amc         → last known-good amc, captured by
                             tools/save-snapshot.sh after green tests.
                             snapshot/amc_lib.c is committed; from a
                             clean clone, rebuild with one gcc step
                             (see snapshot/INFO.md).
- The runtime headers are at runtime/. Cross-platform (POSIX +
  Windows winsock2 via #ifdef _WIN32 in Amalgame_Net.h). libcurl is
  required by the runtime for the Net module + the claude-api /
  chatgpt / gemini providers.
- Test runner (./tests/run_all_tests.sh) drives ./amc directly.
  Build artefacts go to /tmp via mktemp; the source tree stays
  clean. Currently **205 core / 219 stdlib / 12 fmt / 34 amc-new
  = 470 PASS / 0 FAIL / 0 SKIP** across the sub-suites.
- Multi-OS CI (.github/workflows/ci.yml) — Linux + macOS + Windows
  MSYS2. All three platforms gcc the snapshot/amc_lib.c then chain
  through build_amc.sh.
  CI compiles with -Wint-conversion as an error on macOS/Windows;
  pin int-typed locals via `let n: int = …` when the codegen erases
  the return type to void* across a method-call boundary.
- Releases automated on `v*` tag (.github/workflows/release.yml).
  Latest is **v0.5.2** — see CHANGELOG.md for the per-release detail.
  develop → release/vX.Y.Z → develop → main → tag is the release flow.
  Both develop and main are protected (force-push + delete blocked,
  PR required, admin bypass allowed for owner-driven release flow).
- VS Code extension in editors/vscode/ — TextMate grammar +
  language config + LSP client (vscode-languageclient over stdio).
  Configurable via `amalgame.serverPath` in user settings to point
  at a local amc build. Tilde-expansion + whitespace-trim of the
  serverPath value handled inline since v0.4.8.
- Formatter: `amc fmt file.am` re-emits canonical source with
  comments preserved. Idempotent on every compiler source.
- **`PackageRegistry.AmcVersion()` in src/package_registry.am** is
  the single source of truth for the version string since v0.5.0.
  `main.am`'s `--version` reads it; the manual-release flow no
  longer has to hunt a literal in main.am. tools/release.sh still
  bumps README + ROADMAP headers in lockstep.

═══════════════════════════════════════════════════════════════
  Package manager + ecosystem (v0.5 architectural shift)
═══════════════════════════════════════════════════════════════

v0.5 split the monolithic compiler bundle into a **lean core
amc** plus an **opt-in package ecosystem**. The previously
bundled SQLite / Redis / MQTT runtimes were extracted into their
own GitHub repos; user code declares them in a manifest and amc
clones them into a user-global cache.

- **Manifest**: `amalgame.toml` in the project root — declares
  `[package]` (name / version / license) and `[dependencies]`.
  Each external package ships its own `amalgame.toml` exposing
  a `[stdlib]` block with `class`, `header`, `namespace`,
  optional `sources` (vendored `.c` paths), and a
  `[stdlib.functions]` table mapping each public method to its
  C return type.
- **Lockfile**: `amalgame.lock` — pins every dep to a Git commit
  SHA. Cache layout:
    `~/.amalgame/packages/<host>/<owner>/<repo>/<tag>_<sha>/`
- **CLI**: every package-manager verb lives under
  `amc package <action>` (with `amc pkg` as a short alias —
  dotnet-style grouping introduced in v0.5.1 / PR #303):
    amc package add github.com/.../amalgame-database-sqlite@v0.2.0
    amc package add redis@v0.2.0          # shortname via packages-index
    amc package list                       # show installed deps
    amc package remove <name>
    amc package update <name>@<tag>
    amc package search [keyword]
    amc package cache clear [--all]
- **3 inaugural external packages** (all under the
  `amalgame-lang` org):
    amalgame-database-sqlite        — vendors sqlite3.c
                                       (public-domain, no
                                       libsqlite3-dev anywhere)
    amalgame-database-nosql-redis   — pure RESP2 over TCP, no
                                       vendored C
    amalgame-messaging-mqtt         — pure MQTT 3.1.1 over TCP,
                                       no vendored C
- **packages-index** (separate repo, `amalgame-lang/packages-index`)
  is the curated official index. Community can PR there to
  register a shortname → repo URL. `amc package search` /
  `amc package add <shortname>` consume it.
- **PackageRegistry** (src/package_registry.am) reads the lock
  + manifests and feeds the resolver + cgen. Public type
  `LoadedPackage` carries `Name`, `ClassName`, `Header`, `Ns`,
  `FuncNames` / `FuncRets`, and `Sources: List<string>` (since
  v0.5.2 — populated from the manifest's `[stdlib].sources`
  array, absolute paths inside the cache dir).
- **Namespace mangling** (PR #295) — C symbols emitted for
  package class methods carry the full namespace path so two
  unrelated packages exposing the same short class name can't
  collide at link time:
    `Redis.Open` → `Amalgame_Database_NoSQL_Redis_Open`
  Done by the cgen via
  `PackageRegistry.ManglePackageSymbol(ns, method)`. Core
  stdlib classes (Console / File / Math / String / List / Env
  / Process / Log / Service) keep flat `Class_Method` symbols
  — single-author, no collision risk.
- **`required-amalgame` gate** — each package's manifest can
  declare `required-amalgame = ">=X.Y.Z"`. amc refuses to
  install if its own version doesn't satisfy the constraint
  (errors at install instead of mysteriously at gcc-link).
  Helpers `PackageRegistry.ParseVersion` /
  `VersionSatisfies` implement the check.

═══════════════════════════════════════════════════════════════
  Standard library — what's in
═══════════════════════════════════════════════════════════════

Core (since v0.3.x, always available, no manifest needed):
Console, File, Path (flat fn API), Math, String, List<T> /
Map<K,V> / Set<T>, Http + HttpResponse, Tcp {Server,Client},
Udp, Args, Exit, Process (Run + RunCapture), Env.

Namespace-facade stdlib bundled with amc (each under
`namespace Amalgame.<Module>` or `Amalgame.Formats.<Format>`):

- **Amalgame.Formats.Json** (renamed from Amalgame.Json in
  v0.5.0) — schemaless JsonValue tree, parse + serialize,
  accessors (AsInt/AsString/Get/At), encoder.
- **Amalgame.Formats.Toml** (v0.5.0) — TOML 1.0 subset:
  tables, nested tables, inline tables, arrays, basic + literal
  strings, integers, booleans, `#` comments, `[[foo]]` array-of-
  tables. Underpins the package-manager manifest + lockfile
  parsing.
- **Amalgame.Random** (v0.4.4) — PCG-32 + crypto-grade entropy
  (Random.SystemBytes).
- **Amalgame.Encoding** (v0.4.4) — Base64 (RFC 4648 + URL-safe),
  Hex, percent-encoding.
- **Amalgame.DateTime** (v0.4.4) — Instant.Now / FromUnixSeconds,
  Duration arithmetic, ISO 8601 formatting, Stopwatch.
- **Amalgame.Crypto** (v0.4.4) — SHA-256 + HMAC-SHA-256 (FIPS
  180-4 / RFC 2104, pure C in runtime/Amalgame_Crypto.h).
- **Amalgame.Path** (v0.4.11) — cross-platform path manipulation:
  Combine, Extension, Filename, Directory, Stem, IsAbsolute,
  Normalize (Go filepath.Clean semantics, no FS access), Sep.
- **Amalgame.Logging** (v0.4.12) — leveled stderr + optional file
  sink. `Log.SetMinLevel("debug"|"info"|"warn"|"error")`,
  `Log.SetFile(path)`, `Log.Debug/Info/Warn/Error(msg)`. Process-
  wide singleton state in the runtime.
- **Amalgame.Service** (v0.4.13) — long-running daemon primitives.
  `Service.Install()` (SIGTERM/SIGINT on POSIX,
  SetConsoleCtrlHandler on Windows), `Service.ShouldStop()`,
  `Service.RequestStop()`, `Service.Sleep(ms)` (interruptible).

**External packages** (install via `amc package add`, not
bundled):

- **Amalgame.Database.SQLite** — `amalgame-database-sqlite`.
  Vendors `sqlite3.c` (public-domain). Surface:
  `SQLite.Open(path)`, `Close`, `IsOpen`, `Exec(sql) → bool`,
  `QueryAll(sql) → List<List<string>>`, `LastInsertId`,
  `Changes`, `LastError`. Since v0.5.2, `amc test` auto-links
  the vendored amalgamation; user binaries built by hand still
  pass `sqlite3.c` to gcc directly.
- **Amalgame.Database.NoSQL.Redis** — `amalgame-database-nosql-redis`.
  Pure RESP2 over `Amalgame_Net.h` sockets. Surface:
  `Redis.Open(host, port)`, `Close`, `IsOpen`, `LastError`,
  `Ping`, `Set`, `Get`, `Del`, `Exists`, `Incr`, `Decr`,
  `Expire`. Works against Redis / KeyDB / Dragonfly / Valkey.
- **Amalgame.Messaging.MQTT** — `amalgame-messaging-mqtt`.
  Pure MQTT 3.1.1 over TCP, no vendored client lib.

The core-stdlib classes that lower to flat `Class_Method`
symbols live in a small hardcoded list inside the cgen
(src/generator/c_gen.am around line 3215): Console, File, Math,
String, List, Env, Process, Log, Service. **External package
classes are not in that list** — they go through the namespace-
mangled path resolved by `PackageRegistry`, so a sibling
package with a colliding short class name is handled cleanly.
The facade files under src/stdlib/ (logging.am, service.am,
…) still exist as documentation + test-runner inputs; the
short-syntax `Log.Info("msg")` lowers directly to the runtime
helper without requiring the user to import them.

═══════════════════════════════════════════════════════════════
  LSP — what's in
═══════════════════════════════════════════════════════════════

amc lsp (src/lsp.am) is a workspace-aware LSP 3.x server over
stdio JSON-RPC. Capabilities currently advertised:

- textDocumentSync: 1 (Full)
- hoverProvider, completionProvider with `.` triggerCharacters
- definitionProvider, declarationProvider, typeDefinitionProvider
- documentSymbolProvider, workspaceSymbolProvider
- referencesProvider
- renameProvider with prepareProvider:true
- callHierarchyProvider
- inlayHintProvider
- codeActionProvider
- foldingRangeProvider (v0.4.17, slice 5)

Slices closed:
- Slice 1: diagnostics + hover + completion (v0.3.4–v0.3.5)
- Slice 2: definition + declaration + typeDefinition + outline +
  workspace symbol + references + workspace-resolver caching
  (v0.4.5–v0.4.8)
- Slice 3: rename + call hierarchy (v0.4.9)
- Slice 4: inlay hints (inferred types on `let x = …`) + code
  actions ("Add type annotation" quick fix) (v0.4.10)
- Slice 5: textDocument/foldingRange (v0.4.17). Token-driven
  brace-pair matching for class / method / block bodies; runs
  of `//` comments emitted as `kind:"comment"`; consecutive
  `import …` statements as `kind:"imports"`. One- and two-line
  blocks filtered so the gutter stays uncluttered.

Next LSP slice tracked in ROADMAP_COMPLET.md: tighter
selectionRange via a parser nameStart hook, more code actions
wired to linter / typechecker diagnostics.

═══════════════════════════════════════════════════════════════
  `amc test` runner — what it does (v0.5.1 / v0.5.2 upgrades)
═══════════════════════════════════════════════════════════════

Two big upgrades since v0.5.0:

1. **Package-aware (v0.5.1, PR #302)** — before discovery,
   `RunTest` reads `amalgame.lock` and, for each
   `[[package]]` entry, checks whether its cache dir exists.
   Missing entries are `git clone`d via
   `AddCommand.EnsureInstalled()` so a fresh
   `git clone <repo> && amc test` works in one step with no
   manual `amc package add`.

2. **Auto-link vendored sources (v0.5.2, PR #304)** — each
   loaded package's `[stdlib].sources` paths get gcc-compiled
   to `/tmp/amc-pkg-<class>-<leaf>.c.o` once per run by
   `Program.PreCompilePackageSources(registry, amcRuntime)`,
   then spliced into every test binary's gcc invocation
   alongside `-lgc -lm -lcurl -ldl -lpthread`. SQLite tests
   now work out of the box; future vendoring backends (DuckDB,
   bundled libpq, etc.) inherit the wiring for free.

`amcRuntime` is resolved once up front:
  1. `$AMC_RUNTIME` env var if set
  2. else `<dirname(amc)>/runtime`
  3. else fall back to `./runtime` (legacy in-tree path)

The runner still parses `[PASS]` / `[FAIL]` / `[SKIP]` lines
from each test binary's stdout, surfaces non-zero exit as
crash, and aggregates a final tally.

═══════════════════════════════════════════════════════════════
  `amc new` scaffolder — what's in
═══════════════════════════════════════════════════════════════

amc new <name> [--template <kind>] [--force] generates a starter
project. Four templates:

  exe      Default. src/main.am with Program.Main + a passing
           test, build.sh that locates AMALGAME_HOME and calls
           amc + gcc.
  lib      src/<name>.am with a public class skeleton, no main.
  test     tests/<name>_test.am only — bolt onto an existing
           project.
  service  Long-running daemon (v0.4.14). Generates:
             src/main.am               — canonical
                                          Service.Install +
                                          while !ShouldStop +
                                          Log.Info loop
             <name>.service            — systemd unit (Linux)
             install.sh                — systemctl daemon-reload
                                          + enable + restart
             install.ps1               — NSSM-based install on
                                          Windows (auto-downloads
                                          NSSM if missing). Native
                                          Windows SCM dispatcher
                                          tracked as v2 in the
                                          roadmap.
             build.sh + build.ps1      — POSIX gcc + Windows
                                          MinGW gcc paths.
             README.md                 — full cross-OS run /
                                          install / operate matrix.

═══════════════════════════════════════════════════════════════
  Authorship + contribution policy
═══════════════════════════════════════════════════════════════

- **Bastien Mouget is the sole author and copyright holder**
  (NOTICE.md). All work is Apache-2.0 licensed.
- **External contributions are paused** (CONTRIBUTING.md). Bug
  reports via Issues stay open; forks are allowed per Apache-2.0.
- A GitHub Action at .github/workflows/auto-close-external-prs.yml
  auto-closes PRs opened from forks with a comment pointing at
  CONTRIBUTING.md. Internal branches (release/*, feat/*, docs/*,
  chore/*) skip the hook because their head.repo matches base.repo.
- **No `Co-Authored-By: Claude …` trailers in any new commit** —
  the AI is a tool, not a co-author at law (NOTICE.md spells this
  out). tools/release.sh stopped emitting the trailer in v0.4.17.
  Some pre-2026-05-10 commits carry it for historical honesty.
- Third-party licence audit lives in NOTICE.md: bdwgc (permissive),
  libcurl (MIT/X), NSSM (PD), host compilers (GPL — tools not
  output). SQLite (PD) no longer ships with amc as of v0.5.0; it
  lives in the amalgame-database-sqlite package with its own
  NOTICE-equivalent block.

═══════════════════════════════════════════════════════════════
  Release flow (gitflow + tools/release.sh)
═══════════════════════════════════════════════════════════════

Bump the version in src/package_registry.am BEFORE every tag —
`PackageRegistry.AmcVersion()` is the single source of truth
since v0.5.0:

    public static string AmcVersion() {
        return "0.5.2"
    }

`main.am`'s `--version` reads from there; the manual flow only
edits this one constant (+ README + ROADMAP headers). Use
tools/release.sh which bumps every place the version lives,
inserts a CHANGELOG stub, builds + tests + saves a snapshot,
then walks:

    release/vX.Y.Z  →  develop  →  main  →  tag vX.Y.Z

Both PR transitions go through `gh pr merge --auto --merge` so
the protected-branch CI is what flips them. After every release,
back-merge `main → develop` (fast-forward) so the next
develop→main PR doesn't carry a squash-divergence conflict.

Pre-flight refuses to start if you're not on develop, the working
tree is dirty, or the target tag already exists — so the v0.4.0-
style "forgot to bump" mistake can't happen through this path.

If you do the release flow manually (the typical pattern in this
session), the same gates apply by hand:

    git checkout -b release/vX.Y.Z
    # edit src/package_registry.am AmcVersion(), README.md,
    # ROADMAP_COMPLET.md, CHANGELOG.md
    ./build_amc.sh && ./tools/save-snapshot.sh
    git add … && git commit -m "release: vX.Y.Z" && git push
    gh pr create --base develop … && gh pr merge --squash --admin
    gh pr create --base main --head develop … && gh pr merge --merge --admin
    git checkout main && git tag -a vX.Y.Z -m … && git push origin vX.Y.Z
    # release.yml builds + publishes the 4 artefacts
    git checkout develop && git merge origin/main --ff-only && git push

═══════════════════════════════════════════════════════════════
  Memory feedback (claude.ai/code, per-project)
═══════════════════════════════════════════════════════════════

Saved feedbacks under
~/.claude/projects/-home-neitsab-D-veloppement-Amalgame/memory/:

- **feedback_gitflow.md** — depuis 2026-05-07, features sur
  develop (jamais commit direct sur main/develop).
- **feedback_language.md** — depuis 2026-05-08, répondre en
  français dans le chat (code/commits restent en anglais).
- **feedback_autonomous_edits.md** — depuis 2026-05-08, exécuter
  Edit/Write/Bash sans "veux-tu que je..." une fois le plan
  validé.
- **feedback_no_coauthor_trailer.md** — depuis 2026-05-10, plus
  de `Co-Authored-By: Claude …` dans les commits Amalgame
  (projet potentiellement revendable, AI = outil pas co-auteur).

═══════════════════════════════════════════════════════════════
  Known gotchas / sharp edges
═══════════════════════════════════════════════════════════════

1. **Bootstrap chicken-and-egg when adding a new runtime header**
   — adding `#include "Amalgame_<New>.h"` to the cgen's prelude
   emission means the OLD amc embedded in the previous snapshot
   doesn't know about the new header, but compiles src/ that
   references the renamed symbols. The bootstrap step 1 fails at
   link time.

   Two workarounds, both retired immediately after use:
     - Temporarily add `-include Amalgame_<New>.h` to the gen_test
       gcc command in build_amc.sh. After saving the new snapshot,
       remove the flag.
     - Create a one-line shim header at the OLD name that includes
       the new name. After saving the snapshot, delete the shim.

   Either way, the snapshot is the keystone: once it ships the new
   cgen, the workaround is dead code.

2. **Nested generics in `let` annotations don't parse** — Amalgame
   parser rejects `let xs: List<List<string>> = …`. Drop the
   outer annotation (cgen infers the AmalgameList* type) and
   annotate just the inner: `let row: List<string> = xs.Get(0)`.

3. **Cross-namespace static-call return-type inference** — the
   cgen's isStdlib short-circuit skips MethodRetRawSet/Get for
   short-syntax stdlib calls. So `let rows = SQLite.QueryAll(...)`
   doesn't carry the `List<List<string>>` raw type through to the
   cgen — the user has to annotate the inner list explicitly.
   Tracked as a v2 improvement.

4. **`amc` doesn't auto-link** — `amc -o foo bar.am` emits
   `foo.c` only. The test runner (and build.sh templates) do
   `gcc -O2 -Iruntime foo.c -lgc -lm -lcurl -o foo` as a
   separate step. `amc test` does the gcc step internally for
   the suite case, and since v0.5.2 also splices any installed
   package's vendored .c objects into that link.

5. ~~**`amc test` doesn't see Database tests**~~ — **RESOLVED in
   v0.5.2 (PR #304).** Packages declaring `[stdlib].sources` in
   their manifest get their vendored `.c` files pre-compiled to
   `/tmp/amc-pkg-<class>-<leaf>.c.o` by
   `Program.PreCompilePackageSources`, then linked into every
   test binary alongside `-ldl -lpthread`. SQLite tests now
   work out of the box from `amc test`; future vendoring
   backends inherit the wiring automatically.

6. **MemberTable.Set silent-no-op on duplicate** — important
   resolver invariant from v0.4.7 (PR #228). The parallel arrays
   Keys + Values were getting desync'd by Values.Add on duplicate
   keys, producing ~150 spurious typechecker errors. The fix:
   no-op silently if the key already exists. Don't undo this.

7. **CGen-precedence bug on `(A || B) && C`** — the cgen re-
   associates parenthesised boolean expressions to `A || (B && C)`
   in the C output. Workaround in src/lsp.am
   CollectRenameEdits: split the kind+name check across multiple
   `if` statements:
     ```
     var matches: bool = false
     if (k == NodeKind.IDENTIFIER || k == NodeKind.MEMBER) { matches = true }
     if (k == NodeKind.CLASS_DECL || k == NodeKind.ENUM_DECL)  { matches = true }
     …
     if (matches && node.Name == target) { … }
     ```
   Track the root cause in c_gen.am if you need to fix it
   properly; otherwise the split-if workaround is good.

8. **String-interpolation conflict with `${VAR:-default}` in shell
   strings** — Amalgame's `"text {var} text"` interpolation
   collides with shell-style `${VAR:-default}` when emitting
   shell-script templates (new_cmd.am). Workaround: sentinels.
     ```
     let lb: string = "{"
     let rb: string = "}"
     s = s + "RUNTIME=\"$" + lb + "AMALGAME_HOME:-" + rb + "\"\n"
     ```
   Used in BuildShExe + BuildShService.

9. **`File_WriteAll` bool return tracking** — `let ok = File.WriteAll(…)`
   typed as bool only since v0.4.x. Older code using the call as
   void won't trip; new code that wants the success flag should
   annotate `let ok: bool = …`.

10. **Curly braces in string templates** — when building file
    contents in new_cmd.am, escape `{` and `}` if your literal
    contains them otherwise the interpolation parser sees an
    embedded `{x}`. Workaround used in migrate.am / generate.am /
    explain.am / new_cmd.am: split the literal braces out as
    their own concat pieces.

11. **gen_test linking and libcurl** — adding a runtime symbol
    that pulls in curl means gen_test now needs `-lcurl` too.
    build_amc.sh links it since v0.4.0 (PR #161). If a new
    runtime call brings in a fresh transitive dep, propagate the
    `-l<lib>` flag here as well.

12. **Bump the version constant BEFORE every tag** — single
    source of truth is `PackageRegistry.AmcVersion()` in
    src/package_registry.am since v0.5.0. tools/release.sh
    handles it automatically; the manual flow has to remember.

13. **One-time post-clone setup for `merge=ours`** —
    `.gitattributes` declares `merge=ours` for
    `snapshot/amc_lib.c`, `snapshot/INFO.md`, and
    `src/amc_lib.c` (generated artefacts; canonical resolution on
    conflict is "rebuild from sources"). Run once after cloning:

        git config merge.ours.driver true

    Without this, git falls back to a normal three-way merge on
    the generated files and you get conflict noise.

14. **`linguist-vendored=true` on runtime/* in .gitattributes** —
    still set, even though SQLite no longer ships in the main
    repo. The directive is cheap to leave in place; if a future
    in-tree backend ever vendors an upstream lib again, the
    language-stats fix is already wired up.

═══════════════════════════════════════════════════════════════
  Roadmap snapshot (next-up at the time of this refresh)
═══════════════════════════════════════════════════════════════

Package manager v0.5.x backlog (no breaking; patch releases):

- Transitive dep resolution + cycle/conflict detection
- Path deps (`{ path = "../foo" }`)
- Semver range constraints (`^X.Y.Z`, `~X.Y.Z`)
- `amc package vendor` (commit the cache into the repo for
  offline reproducible builds)

External-package backlog (each ships from its own repo):

- **SQL backends**: DuckDB (vendored amalgamation), PostgreSQL
  (dynamic-link libpq), MySQL / MariaDB (dynamic-link
  libmariadbclient), Oracle (Instant Client, proprietary
  download), SQL Server (MS ODBC default + FreeTDS fallback).
- **NoSQL**: MongoDB (libmongoc + libbson), DynamoDB / Cosmos
  DB / Firestore (HTTP over Net.Http + Json), Cassandra /
  ScyllaDB (CQL binary protocol).
- **Messaging**: NATS Core (~250 LoC, pure Amalgame), plus
  dynamic-link to Kafka (librdkafka) + RabbitMQ (librabbitmq
  AMQP).
- **SQLite v2** (in `amalgame-database-sqlite`): parameter
  binding via `?` placeholders, typed column accessors,
  prepared statements, transactions.

Compiler / tooling backlog:

- **v0.6 — multi-version coexistence** (Cargo-style dual-link
  enabled by the v0.5 namespace mangling — two packages can
  depend on different majors of the same dep).
- **`Amalgame.Service` v2**: native Windows SCM dispatcher
  (`StartServiceCtrlDispatcher` + `RegisterServiceCtrlHandler` +
  `SetServiceStatus`) — drops the NSSM dependency on Windows.
- **`amc new --template service` v2**: macOS launchd .plist +
  install-macos.sh wrapper.
- **LSP slice 6**: tighter `selectionRange` via parser nameStart
  hook, more code actions wired to linter/typechecker diags.
- **ORM layer** (idea logged in ROADMAP_COMPLET.md — sits
  above the SQL backend packages, would consume any
  `Amalgame.Database.<Engine>`).

═══════════════════════════════════════════════════════════════
  Repo layout
═══════════════════════════════════════════════════════════════

  amc                              ← built binary (gitignored)
  build_amc.sh                     ← 3-step bootstrap
  README.md                        ← project intro, install/run
  CHANGELOG.md                     ← per-release detail
  NOTICE.md                        ← authorship + 3rd-party audit
  CONTRIBUTING.md                  ← external PRs paused
  LICENSE                          ← Apache-2.0
  ROADMAP_COMPLET.md               ← what's next
  CONTINUATION.md                  ← this file
  .gitattributes                   ← merge=ours + linguist
  .github/workflows/               ← ci.yml, release.yml,
                                     release-pdf.yml,
                                     auto-close-external-prs.yml
  docs/
    guide/                         ← user-facing 8-chapter book
      01-getting-started.md
      02-language-tour.md
      03-cli-reference.md          ← amc package <action> table
      04-stdlib.md                 ← every stdlib module + the
                                     3 external packages
      05-runtime-and-interop.md
      06-build-and-tooling.md
      07-internals.md              ← amc test pipeline, cgen,
                                     PackageRegistry
      08-llm-commands.md
      README.md                    ← chapter index
    language/                      ← grammar.ebnf + grammar.md
    changelog/                     ← per-version PDF builds
    proposals/
      amalgame-package-manager.md  ← v0.5 design doc
    DEVELOPER_GUIDE.md
  editors/vscode/                  ← extension.js + grammar
  runtime/
    _runtime.h                     ← core types + closure runtime
    Amalgame_Console.h
    Amalgame_String.h
    Amalgame_Collections.h
    Amalgame_IO.h                  ← File + Path + Env
    Amalgame_Math.h
    Amalgame_Net.h                 ← Http + Tcp + Udp
    Amalgame_Process.h
    Amalgame_Random.h
    Amalgame_DateTime.h
    Amalgame_Crypto.h
    Amalgame_Logging.h
    Amalgame_Service.h
    # No Amalgame_Database_*.h here anymore — moved to the
    # respective external packages' own runtime/ dirs.
  snapshot/
    amc_lib.c                      ← portable bootstrap source
    amc                            ← compiled snapshot binary
    INFO.md                        ← provenance
  src/
    main.am                        ← CLI entry, RunTest + dispatch
    amc_lib.c                      ← generated; merge=ours
    package_registry.am            ← v0.5 PackageRegistry +
                                     LoadedPackage + AmcVersion
    add_cmd.am                     ← amc package <action>
                                     (add / remove / list /
                                      search / update / cache)
                                     + EnsureInstalled
    lexer/{token,lexer}.am
    parser/{ast,parser}.am
    generator/{c_gen,gen_test}.am  ← cgen + bootstrap driver
    formatter/formatter.am
    diagnostics.am
    resolver/{symbol,resolver}.am
    typechecker.am
    linter.am
    lsp.am                         ← LSP server (slices 1-5)
    migrate.am / generate.am /
    explain.am                     ← LLM-driven commands (v0.4.0)
    new_cmd.am                     ← amc new scaffolder (4 templates)
    stdlib/
      json.am                      ← namespace Amalgame.Formats.Json
      toml.am                      ← namespace Amalgame.Formats.Toml
      random.am
      encoding.am
      datetime.am
      crypto.am
      path.am                      ← v0.4.11
      logging.am                   ← v0.4.12
      service.am                   ← v0.4.13
  tests/
    run_all_tests.sh               ← drives every sub-suite
    run_tests.sh                   ← core + advanced (205)
    run_stdlib_tests.sh            ← stdlib modules (219)
    run_fmt_tests.sh               ← (12)
    run_amc_new_tests.sh           ← amc new templates (34)
    samples/                       ← .am test fixtures
  stdlib/strings.am                ← legacy pure-Amalgame strings
  tools/
    save-snapshot.sh
    release.sh                     ← end-to-end release flow
  install/                         ← homebrew/etc placeholders

═══════════════════════════════════════════════════════════════
  TL;DR for the new session
═══════════════════════════════════════════════════════════════

Pick up from v0.5.2. develop and main are both at v0.5.2,
synced to origin, working tree clean, all tags up to v0.5.2
published. `~/.local/bin/amc` is the user-installed copy (the
VS Code extension reads it via `amalgame.serverPath`).

The big architectural shift to know about: **amc is no longer
a monolithic bundle**. SQLite / Redis / MQTT live in their own
`amalgame-lang/amalgame-*` repos and install via
`amc package add`. Manifest is `amalgame.toml`, lockfile is
`amalgame.lock`, cache is `~/.amalgame/packages/`. Namespace
mangling means two packages sharing a short class name don't
collide at link time.

Memory feedbacks already cover: répondre en français dans le
chat / pas de Co-Authored-By trailer / édits autonomes après
plan validé / features sur develop. Apply them by default.

The most natural next directions are:
1. **Ship a 4th external package** — DuckDB (vendored
   amalgamation, mirrors SQLite shape) or PostgreSQL
   (dynamic-link libpq, mirrors the dynamic-dep shape MQTT
   doesn't exercise yet).
2. **Transitive dep resolution** in `amc package add` — first
   v0.5.x patch. Today's installer handles single-package only;
   any package depending on another requires a manual second
   `amc package add`.
3. **Packages-index growth** — register more shortnames as
   external packages land, so `amc package add duckdb@vX.Y.Z`
   resolves without typing the full URL.
4. **`amc package vendor`** — copy the cache into the project
   for offline reproducible builds.

Ask me which direction before diving in.
```

---

## After-clone setup (one-time)

```
git clone https://github.com/amalgame-lang/Amalgame.git
cd Amalgame
git config merge.ours.driver true          # see gotcha #13 above
gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc
./build_amc.sh                              # builds ./amc from src/
./tests/run_all_tests.sh                    # 470 PASS expected
```

System deps (apt):

```
sudo apt install -y gcc libgc-dev libcurl4-openssl-dev
```

(macOS uses brew; Windows uses MSYS2 mingw64 — see ci.yml.)
