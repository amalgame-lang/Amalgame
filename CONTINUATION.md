# Continuation prompt — start a new chat with this

> Last refreshed 2026-05-11 after shipping v0.4.15 (Database.SQLite
> stdlib + governance: NOTICE.md / CONTRIBUTING.md / auto-close
> external PRs / CLEANUP.sh removal).
>
> The block below is a self-contained prompt designed to bootstrap a
> new Claude session with full context. Copy-paste it as your first
> message in a fresh conversation.

---

```
I'm working on Amalgame, a self-hosted programming language that
transpiles to C. I keep the project in
/home/neitsab/Développement/Amalgame.

Current state (May 2026, v0.4.15):

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
  clean. Currently **434 PASS / 0 FAIL / 0 SKIP** across the
  core / stdlib / fmt / amc-new sub-suites.
- Multi-OS CI (.github/workflows/ci.yml) — Linux + macOS + Windows
  MSYS2. All three platforms gcc the snapshot/amc_lib.c then chain
  through build_amc.sh.
  CI compiles with -Wint-conversion as an error on macOS/Windows;
  pin int-typed locals via `let n: int = …` when the codegen erases
  the return type to void* across a method-call boundary.
- Releases automated on `v*` tag (.github/workflows/release.yml).
  Latest is **v0.4.15** — see CHANGELOG.md for the per-release detail.
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

═══════════════════════════════════════════════════════════════
  Standard library — what's in
═══════════════════════════════════════════════════════════════

Core (since v0.3.x): Console, File, Path (flat fn API), Math,
String, List<T> / Map<K,V> / Set<T>, Http + HttpResponse, Tcp
{Server,Client}, Udp, Args, Exit, Process (Run + RunCapture), Env.

Namespace-facade stdlib (each under `namespace Amalgame.<Module>`):

- **Amalgame.Json** (v0.3.6) — schemaless JsonValue tree, parse +
  serialize, accessors (AsInt/AsString/Get/At), encoder.
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
- **Amalgame.Database.SQLite** (v0.4.15) — embedded SQL via the
  vendored SQLite amalgamation at
  runtime/Amalgame_Database/sqlite/sqlite3.{c,h} (public-domain
  dedication, no libsqlite3-dev needed anywhere). Surface:
  `SQLite.Open(path)`, `Close`, `IsOpen`, `Exec(sql) → bool`,
  `QueryAll(sql) → List<List<string>>`, `LastInsertId`, `Changes`,
  `LastError`. User binaries link sqlite3.c directly; the test
  runner precompiles it to .o once at startup. Namespace is
  `Amalgame.Database.SQLite` (not just `.Database`) so sibling
  backends — DuckDB, Postgres, MySQL, Oracle, SQL Server,
  MongoDB, Redis, Kafka, RabbitMQ — can land alongside under
  `Amalgame.Database.<Engine>` / `Amalgame.Database.NoSQL.<Engine>`
  / `Amalgame.Messaging.<Broker>` per the roadmap.

The Console / File / Math / String / List / Env / Process / Log /
Service / SQLite class names live in a hardcoded "isStdlib" list
inside the cgen (src/generator/c_gen.am ~line 3102) so `Log.Info(
"msg")` lowers directly to `Log_Info("msg")` — the runtime helper
under runtime/Amalgame_Logging.h — without needing the user to
import the facade .am source. The facade files (e.g.
src/stdlib/logging.am) still exist as documentation + as test-
runner inputs but are not required for the short-syntax usage.
Other namespace-facades (Crypto, Json, Random, Encoding, DateTime,
Path) require their .am file as a compile input today.

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

Slices closed:
- Slice 1: diagnostics + hover + completion (v0.3.4–v0.3.5)
- Slice 2: definition + declaration + typeDefinition + outline +
  workspace symbol + references + workspace-resolver caching
  (v0.4.5–v0.4.8)
- Slice 3: rename + call hierarchy (v0.4.9)
- Slice 4: inlay hints (inferred types on `let x = …`) + code
  actions ("Add type annotation" quick fix) (v0.4.10)

Next LSP slice tracked in ROADMAP_COMPLET.md: tighter
selectionRange via a parser nameStart hook, textDocument/
foldingRange (the +/- gutter markers for class/method bodies +
block comments + import groups), more code actions wired to
linter / typechecker diagnostics.

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
  out). Some pre-2026-05-10 commits carry the trailer for
  historical honesty; future commits omit it.
- Third-party licence audit lives in NOTICE.md: bdwgc (permissive),
  libcurl (MIT/X), SQLite (PD), NSSM (PD), host compilers (GPL —
  tools not output). All compatible with Apache-2.0 redistribution.

═══════════════════════════════════════════════════════════════
  Release flow (gitflow + tools/release.sh)
═══════════════════════════════════════════════════════════════

Bump the version in src/main.am BEFORE every tag — the `--version`
flag has a hardcoded string:

    Console.WriteLine("amc <X.Y.Z> (self-hosted Amalgame compiler)")

Use tools/release.sh which bumps every place the version lives
(src/main.am, README.md "Current version" + "amc 0.4.X" probe,
ROADMAP_COMPLET.md header), inserts a CHANGELOG stub, builds +
tests + saves a snapshot, then walks:

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
    # edit src/main.am, README.md, ROADMAP_COMPLET.md, CHANGELOG.md
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
   Test sample tests/samples/stdlib_database.am does this.

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
   the suite case.

5. **`amc test` doesn't see Database tests** — its internal gcc
   command doesn't link runtime/Amalgame_Database/sqlite/sqlite3.c.
   Database tests live under tests/samples/ and are run by
   tests/run_stdlib_tests.sh which precompiles a sqlite3.o once
   and links it in. A future `amc test` enhancement could grep
   the generated .c for sqlite3_ symbols and conditionally link
   the amalgamation.

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

12. **Bump the `--version` string in `src/main.am` BEFORE every
    tag** — see the release-flow section above. tools/release.sh
    handles this automatically; the manual flow has to remember.

13. **One-time post-clone setup for `merge=ours`** —
    `.gitattributes` declares `merge=ours` for
    `snapshot/amc_lib.c`, `snapshot/INFO.md`, and
    `src/amc_lib.c` (generated artefacts; canonical resolution on
    conflict is "rebuild from sources"). Run once after cloning:

        git config merge.ours.driver true

    Without this, git falls back to a normal three-way merge on
    the generated files and you get conflict noise.

14. **`linguist-vendored=true` on runtime/* in .gitattributes** —
    keeps the vendored SQLite amalgamation (9MB sqlite3.c +
    641KB sqlite3.h) out of GitHub's language stats. Without it,
    Amalgame would show as "85% C" because of one upstream lib.

═══════════════════════════════════════════════════════════════
  Roadmap snapshot (next-up at the time of this refresh)
═══════════════════════════════════════════════════════════════

Stdlib backlog tracked in ROADMAP_COMPLET.md:

- **`Amalgame.Database.<Engine>` siblings — SQL backends**:
  DuckDB (vendored amalgamation), PostgreSQL (dynamic-link
  libpq), MySQL / MariaDB (dynamic-link libmariadbclient),
  Oracle (Instant Client dynamic-link, proprietary download),
  SQL Server (MS ODBC default + FreeTDS fallback).
- **`Amalgame.Database.NoSQL.<Engine>`**: MongoDB (libmongoc +
  libbson), Redis (pure-Amalgame RESP3 client ~300 LoC),
  DynamoDB / Cosmos DB / Firestore (HTTP over Net.Http + Json),
  Cassandra / ScyllaDB (CQL binary protocol).
- **`Amalgame.Messaging.<Broker>`**: pure-Amalgame MQTT (~300
  LoC) + NATS Core (~250 LoC), plus dynamic-link to Kafka
  (librdkafka) + RabbitMQ (librabbitmq AMQP).
- **`Amalgame.Database.SQLite` v2**: parameter binding via `?`
  placeholders, typed column accessors (row.AsInt(0) /
  row.AsBytes(2)), prepared statements, transactions.
- **`Amalgame.Service` v2**: native Windows SCM dispatcher
  (`StartServiceCtrlDispatcher` + `RegisterServiceCtrlHandler` +
  `SetServiceStatus`) — drops the NSSM dependency on Windows.
- **`amc new --template service` v2**: macOS launchd .plist +
  install-macos.sh wrapper.
- **LSP slice 5**: tighter `selectionRange` via parser nameStart
  hook, `textDocument/foldingRange` (the +/- gutter markers),
  more code actions wired to linter/typechecker diags.

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
      03-cli-reference.md
      04-stdlib.md                 ← every stdlib module
      05-runtime-and-interop.md
      06-build-and-tooling.md
      07-internals.md
      08-llm-commands.md
      README.md                    ← chapter index
    language/                      ← grammar.ebnf + grammar.md
    changelog/                     ← per-version PDF builds
    proposals/                     ← design docs
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
    Amalgame_Database_SQLite.h     ← v0.4.15
    Amalgame_Database/sqlite/
        sqlite3.c, sqlite3.h       ← vendored amalgamation
  snapshot/
    amc_lib.c                      ← portable bootstrap source
    amc                            ← compiled snapshot binary
    INFO.md                        ← provenance
  src/
    main.am                        ← CLI entry, gen_test injection
    amc_lib.c                      ← generated; merge=ours
    lexer/{token,lexer}.am
    parser/{ast,parser}.am
    generator/{c_gen,gen_test}.am  ← cgen + bootstrap driver
    formatter/formatter.am
    diagnostics.am
    resolver/{symbol,resolver}.am
    typechecker.am
    linter.am
    lsp.am                         ← LSP server
    migrate.am / generate.am /
    explain.am                     ← LLM-driven commands (v0.4.0)
    new_cmd.am                     ← amc new scaffolder (4 templates)
    stdlib/
      json.am
      random.am
      encoding.am
      datetime.am
      crypto.am
      path.am                      ← v0.4.11
      logging.am                   ← v0.4.12
      service.am                   ← v0.4.13
  tests/
    run_all_tests.sh               ← drives every sub-suite
    run_tests.sh                   ← core + advanced
    run_stdlib_tests.sh            ← stdlib modules
    run_fmt_tests.sh
    run_amc_new_tests.sh           ← amc new templates
    samples/                       ← .am test fixtures
  stdlib/strings.am                ← legacy pure-Amalgame strings
  tools/
    save-snapshot.sh
    release.sh                     ← end-to-end release flow
  install/                         ← homebrew/etc placeholders

═══════════════════════════════════════════════════════════════
  TL;DR for the new session
═══════════════════════════════════════════════════════════════

Pick up from v0.4.15. Develop is 3 commits ahead of main with
post-release accumulation (CLEANUP.sh removal + 2 doc-PRs that
expanded the database / messaging roadmap entries). Working tree
clean, both branches synced to origin, all tags up to v0.4.15
published. `~/.local/bin/amc` is updated to 0.4.15 (the LSP
extension uses it).

Memory feedbacks already cover: répondre en français dans le
chat / pas de Co-Authored-By trailer / édits autonomes après
plan validé / features sur develop. Apply them by default.

The most natural next directions are:
1. **Cut v0.4.16** to roll the develop-side accumulation into a
   real release (small — CLEANUP.sh + doc updates only).
2. **LSP slice 5** — foldingRange is the user-explicitly-requested
   piece, smallest scope.
3. **Pick a sibling Database backend** to ship — PostgreSQL or
   DuckDB are the natural follow-ons to SQLite. Redis as a NoSQL
   would be smallest (pure-Amalgame, no native dep).
4. **Pick a message broker** — MQTT first (smallest pure-Amalgame
   implementation, ~300 LoC).

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
./tests/run_all_tests.sh                    # 434 PASS expected
```

System deps (apt):

```
sudo apt install -y gcc libgc-dev libcurl4-openssl-dev
```

(macOS uses brew; Windows uses MSYS2 mingw64 — see ci.yml.)
