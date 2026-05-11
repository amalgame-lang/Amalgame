# Continuation prompt — start a new chat with this

> Last refreshed 2026-05-12 after shipping v0.5.3 → v0.6.0 in a
> single overnight sprint. Five releases roll into one snapshot:
> v0.5.3 (C++ packages pipeline), v0.5.4 (precompile-on-install +
> calibration), v0.5.5 (search/versions with compat status), v0.5.6
> (index cache TTL + runner fixes), v0.6.0 (auto-resolve
> add-without-tag + semver operators).
>
> The block below is a self-contained prompt designed to bootstrap a
> new Claude session with full context. Copy-paste it as your first
> message in a fresh conversation.

---

```
I'm working on Amalgame, a self-hosted programming language that
transpiles to C. I keep the project in
/home/neitsab/Développement/Amalgame.

Current state (May 2026, v0.6.0):

═══════════════════════════════════════════════════════════════
  Compiler + bootstrap
═══════════════════════════════════════════════════════════════

- The compiler `amc` is written in Amalgame in src/ and compiles
  itself in ~2 seconds via ./build_amc.sh.
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
  clean. Currently **205 core / 258 stdlib / 12 fmt / 34 amc-new
  = 509 PASS / 0 FAIL / 0 SKIP** across the sub-suites.
- Multi-OS CI (.github/workflows/ci.yml) — Linux + macOS + Windows
  MSYS2. All three platforms gcc the snapshot/amc_lib.c then chain
  through build_amc.sh.
  CI compiles with -Wint-conversion as an error on macOS/Windows;
  pin int-typed locals via `let n: int = …` when the codegen erases
  the return type to void* across a method-call boundary.
- Releases automated on `v*` tag (.github/workflows/release.yml).
  Latest is **v0.6.0** — see CHANGELOG.md for the per-release detail.
  develop → release/vX.Y.Z → develop → main → tag is the release flow.
  Both develop and main are protected (force-push + delete blocked,
  PR required, admin bypass allowed for owner-driven release flow).
- VS Code extension in editors/vscode/ — TextMate grammar +
  language config + LSP client (vscode-languageclient over stdio).
  Configurable via `amalgame.serverPath` in user settings.
- Formatter: `amc fmt file.am` re-emits canonical source with
  comments preserved. Idempotent on every compiler source.
- **`PackageRegistry.AmcVersion()` in src/package_registry.am** is
  the single source of truth for the version string since v0.5.0.
  `main.am`'s `--version` reads it; the manual-release flow only
  edits this one constant. tools/release.sh handles everything else.

═══════════════════════════════════════════════════════════════
  Package manager — full surface
═══════════════════════════════════════════════════════════════

`amc package <action>` (alias `amc pkg`) with these verbs:

  amc package add <name|url>[@<tag>] [--no-precompile]
       Install a package. If @<tag> omitted for an INDEXED
       shortname, auto-resolve to the latest tag whose
       `required-amalgame` is satisfied by the running amc.
       Full git URLs still need an explicit @<tag>.
       --no-precompile skips install-time compile even if the
       manifest declares `[stdlib].precompile = true`.

  amc package search [keyword] [--refresh]
       Browse the curated index. Each result shows known tags +
       compat status (✓/✗ vs running amc) + a "← latest compatible"
       marker. --refresh wipes the 30-min cache to force re-fetch.

  amc package versions <name> [--refresh]
       Shortcut: `search` output filtered to one package.

  amc package list
       Show installed deps from amalgame.lock with their pinned
       tags. Format: `<ClassName> @ <tag> — <slug>`.

  amc package remove <name>[@<tag>] [...]
       Strip dep(s) from amalgame.toml + amalgame.lock. The
       optional @<tag> safety suffix refuses to remove unless
       the installed tag matches.

  amc package update <name>@<tag>
       Bump a pinned tag (delegates to add under the hood).

  amc package cache clear [--all]
       Drop cached packages and/or index file.

**Manifest format** (`amalgame.toml` in each package repo):

```toml
[package]
name              = "amalgame-database-duckdb"
version           = "0.1.1"
license           = "Apache-2.0"
description       = "DuckDB binding — vendored C++ amalgamation"
authors           = ["Bastien Mouget"]
required-amalgame = ">=0.5.4"   # or ^, ~, =, >, <, <=, bare
schema-version    = 1

[stdlib]
class      = "DuckDB"
header     = "runtime/Amalgame_Database_DuckDB.h"
namespace  = "Amalgame.Database.DuckDB"
sources    = ["runtime/Amalgame_Database/duckdb/duckdb.cpp"]
cflags     = "-O2 -DNDEBUG"           # extra flags for .c sources
cxxflags   = "-O2 -DNDEBUG -std=c++17" # extra flags for .cpp/.cc/.cxx
libs       = ["stdc++"]                # -l<name> at final link
precompile = true                       # compile at `amc package add` time

[stdlib.functions]
Open       = { returns = "AmalgameDuckDB*" }
# … etc
```

**Required-amalgame operators** (v0.6.0+): `>=`, `>`, `<=`, `<`,
`=`, `^` (caret, 0.x-aware npm flavour), `~` (tilde, locks
major.minor). Bare version treated as `>=` for back-compat.

**Precompile-on-install** (v0.5.4+): when `precompile = true`,
`amc package add` compiles each `[stdlib].sources` entry into a
persistent cache at:

```
~/.amalgame/packages/<host>/<owner>/<repo>/<tag>_<sha>/
└─ build/<platform>/<class>-<basename>.o
```

`<platform>` = `linux-x86_64` / `macos-arm64` / `windows-x86_64`
(lowercased `$(uname -s)-$(uname -m)`). Cross-OS users sharing a
$HOME don't collide.

Subsequent `amc test` / build look there first, fall back to
`/tmp/amc-pkg-<class>-<basename>.o` (v0.5.2 lazy cache), then to
fresh compile. `--no-precompile` opts out per-install.

**Calibration** (v0.5.4+): each precompile writes a sample to
`~/.amalgame/calibration.toml`:

```toml
[[sample]]
lang      = "cxx"
size_kb   = 24944
elapsed_s = 882
pkg_ver   = "amalgame-database-duckdb@v0.1.1"
```

Future installs read these and compute ETA via weighted average
on the current machine. First time on a new machine: no ETA, but
elapsed time printed after.

**Cross-platform `$HOME` resolution** (v0.5.4+):
`PackageRegistry.AmalgameHome()` walks `$AMALGAME_HOME` →
`$HOME` → `$USERPROFILE`. Fixes a pre-existing bug where amc fell
back to `/tmp` on native Windows shells (only MSYS2 was being
exercised by CI).

**Index cache TTL** (v0.5.6+): `FetchIndex` considers
`~/.amalgame/cache/packages-index.toml` fresh for 30 minutes
(mtime via `date -r <file> +%s`). Network failure during refresh
falls back to serving the stale cache with a warning.

═══════════════════════════════════════════════════════════════
  Packages-index (separate repo)
═══════════════════════════════════════════════════════════════

`github.com/amalgame-lang/packages-index` is the curated SoT for
shortname resolution + `amc package search`. Schema v2:

```toml
schema-version = 1

[[package]]
name        = "duckdb"
url         = "github.com/amalgame-lang/amalgame-database-duckdb"
description = "DuckDB binding — vendored C++ amalgamation (MIT)…"
tier        = "official"          # or "listed" for community
maintainer  = "amalgame-lang"
license     = "Apache-2.0"
category    = "database"

# One [[version]] block per (shortname, tag) — flat, linked by
# `package` field. Append newest-last; amc walks the array and
# uses the last-seen compatible tag for auto-resolve + search
# "latest compatible" marker.
[[version]]
package           = "duckdb"
tag               = "v0.1.0"
required-amalgame = ">=0.5.3"

[[version]]
package           = "duckdb"
tag               = "v0.1.1"
required-amalgame = ">=0.5.4"
```

**Automated maintenance**: each package repo has a
`.github/workflows/index-pr.yml` that fires on tag push, reads
the manifest's `required-amalgame`, and opens a PR on
packages-index adding the new `[[version]]` block. Validated
in prod on the v0.2.2 SQLite release. Needs the
`PACKAGES_INDEX_PAT` repo secret (fine-grained PAT scoped to
packages-index, Contents + PRs = write).

Current registered entries:
- **sqlite** — v0.2.0, v0.2.2 (v0.2.1 skipped, never registered)
- **redis** — v0.2.0
- **mqtt** — v0.2.0
- **duckdb** — v0.1.0, v0.1.1

═══════════════════════════════════════════════════════════════
  External packages — all 4 live
═══════════════════════════════════════════════════════════════

**Amalgame.Database.SQLite** — `amalgame-database-sqlite` v0.2.2
- Vendored SQLite 3 amalgamation (public-domain)
- `precompile = true` (v0.2.1+ requires amc ≥ 0.5.4)
- Surface: Open/Close/IsOpen/LastError/Exec/QueryAll/LastInsertId/Changes
- cflags = "-DSQLITE_THREADSAFE=1 -DSQLITE_ENABLE_FTS5"

**Amalgame.Database.NoSQL.Redis** — `amalgame-database-nosql-redis` v0.2.0
- Pure RESP2 over `Amalgame_Net.h` sockets (no vendored C)
- Surface: Open/Close/IsOpen/LastError/Ping/Set/Get/Del/Exists/Incr/Decr/Expire
- Works against Redis / KeyDB / Dragonfly / Valkey

**Amalgame.Messaging.MQTT** — `amalgame-messaging-mqtt` v0.2.0
- Pure MQTT 3.1.1 over TCP, no vendored C
- QoS 0 in v1

**Amalgame.Database.DuckDB** — `amalgame-database-duckdb` v0.1.1
- Vendored DuckDB v1.5.2 C++ amalgamation (MIT)
- `precompile = true` (requires amc ≥ 0.5.4)
- cxxflags = "-O2 -DNDEBUG -std=c++17 -Wno-unused-parameter …"
- libs = ["stdc++"]
- Two-stage link: gcc -c on the cgen-emitted .c (amc emits
  C-style void* casts g++ rejects), g++ on the resulting .o
  + duckdb.cpp.o (needs libstdc++ + C++ static-init order)
- DuckDB bare amalgamation doesn't include `core_functions`
  extension (SUM / AVG / STDDEV). `Open()` enables
  `autoinstall_known_extensions=1` + `autoload_known_extensions=1`
  so first query needing those downloads + caches the extension
  from extensions.duckdb.org. Offline-only: stick to COUNT(*) /
  MIN / MAX which are parser built-ins.

═══════════════════════════════════════════════════════════════
  Standard library (bundled with amc, no manifest needed)
═══════════════════════════════════════════════════════════════

Core (since v0.3.x): Console, File, Path (flat fn API), Math,
String, List<T> / Map<K,V> / Set<T>, Http + HttpResponse,
Tcp {Server,Client}, Udp, Args, Exit, Process (Run + RunCapture),
Env.

Namespace-facade stdlib (each under `namespace Amalgame.<Module>`):

- **Amalgame.Formats.Json** (v0.5.0) — schemaless JsonValue tree
- **Amalgame.Formats.Toml** (v0.5.0) — TOML 1.0 subset
- **Amalgame.Random** (v0.4.4) — PCG-32 + crypto entropy
- **Amalgame.Encoding** (v0.4.4) — Base64 / Hex / percent-encode
- **Amalgame.DateTime** (v0.4.4) — Instant, Duration, Stopwatch
- **Amalgame.Crypto** (v0.4.4) — SHA-256 + HMAC-SHA-256
- **Amalgame.Path** (v0.4.11) — Combine/Filename/Directory/Stem/
  IsAbsolute/Normalize/Sep
- **Amalgame.Logging** (v0.4.12) — leveled stderr + optional file
- **Amalgame.Service** (v0.4.13) — long-running daemon primitives

The core-stdlib classes that lower to flat `Class_Method` symbols
live in a small hardcoded list inside the cgen
(src/generator/c_gen.am around line 3215): Console, File, Math,
String, List, Env, Process, Log, Service. **External package
classes are not in that list** — they go through the
namespace-mangled path resolved by `PackageRegistry`.

═══════════════════════════════════════════════════════════════
  LSP — what's in (unchanged this session)
═══════════════════════════════════════════════════════════════

`amc lsp` is a workspace-aware LSP 3.x server over stdio
JSON-RPC. Capabilities advertised:

- textDocumentSync: 1 (Full), hover, completion, definition,
  declaration, typeDefinition, documentSymbol, workspaceSymbol,
  references, rename (with prepare), callHierarchy, inlayHint,
  codeAction, foldingRange.

5 slices closed (v0.3.4 → v0.4.17). Next on backlog:
tighter selectionRange via parser nameStart hook, more code
actions wired to linter/typechecker.

═══════════════════════════════════════════════════════════════
  `amc test` runner
═══════════════════════════════════════════════════════════════

Two big upgrades from this session:
1. **C/C++ dispatch** (v0.5.3) — `PreCompilePackageSources`
   switches gcc/g++ on file extension. RunTest two-stages the
   link when any package has C++ sources (gcc -c the test.c
   → g++ link with .o + libs).
2. **Persistent cache lookup** (v0.5.4) — RunTest looks at
   `<pkg-dir>/build/<platform>/` before falling back to /tmp.

`amcRuntime` is resolved once up front:
  1. `$AMC_RUNTIME` env var if set
  2. else `<dirname(amc)>/runtime`
  3. else `./runtime` (legacy in-tree path)

═══════════════════════════════════════════════════════════════
  Authorship + contribution policy
═══════════════════════════════════════════════════════════════

- **Bastien Mouget is the sole author and copyright holder**.
  All work is Apache-2.0 licensed.
- **External contributions are paused**. Bug reports open; forks
  allowed per Apache-2.0.
- Auto-close hook on PRs from forks (internal branches
  release/*, feat/*, docs/*, chore/* are skipped).
- **No `Co-Authored-By: Claude …` trailers in any new commit**.
- Third-party licence audit in NOTICE.md.

═══════════════════════════════════════════════════════════════
  Release flow (gitflow + tools/release.sh)
═══════════════════════════════════════════════════════════════

Single source of truth = `PackageRegistry.AmcVersion()` in
src/package_registry.am.

Manual flow (used 5× this session):

    git checkout -b release/vX.Y.Z
    # edit src/package_registry.am AmcVersion(), README.md,
    # ROADMAP_COMPLET.md, CHANGELOG.md (move Unreleased → [vX.Y.Z])
    ./build_amc.sh && ./tests/run_all_tests.sh && ./tools/save-snapshot.sh
    git add … && git commit -m "release: vX.Y.Z" && git push
    gh pr create --base develop … && gh pr merge --squash --admin
    gh pr create --base main --head develop … && gh pr merge --merge --admin
    git checkout main && git tag -a vX.Y.Z -m … && git push origin vX.Y.Z
    # release.yml builds + publishes the 4 artefacts
    git checkout develop && git merge origin/main --ff-only && git push

tools/release.sh automates this but has an interactive `read`
prompt (line 175) — needs `yes y | ./tools/release.sh X.Y.Z` to
work non-interactively. Manual flow is what I use in practice.

═══════════════════════════════════════════════════════════════
  Memory feedback (claude.ai/code, per-project)
═══════════════════════════════════════════════════════════════

Saved feedbacks under
~/.claude/projects/-home-neitsab-D-veloppement-Amalgame/memory/:

- **feedback_gitflow.md** — features sur develop, jamais commit
  direct sur main/develop.
- **feedback_language.md** — répondre en français dans le chat
  (code/commits restent en anglais).
- **feedback_autonomous_edits.md** — exécuter Edit/Write/Bash
  sans "veux-tu que je..." une fois le plan validé.
- **feedback_no_coauthor_trailer.md** — plus de
  `Co-Authored-By: Claude …` dans les commits (projet
  potentiellement revendable, AI = outil pas co-auteur).

═══════════════════════════════════════════════════════════════
  Known gotchas / sharp edges
═══════════════════════════════════════════════════════════════

1. **Bootstrap chicken-and-egg when adding a new runtime header**
   — snapshot's amc doesn't know new symbols. Workarounds:
   temporary `-include` flag in gen_test, or one-line shim header
   at OLD name. Both retired after first snapshot save.

2. **Nested generics in `let` annotations don't parse** —
   `let xs: List<List<string>> = …` rejected. Drop outer
   annotation (cgen infers AmalgameList*) and annotate inner:
   `let row: List<string> = xs.Get(0)`.

3. **Cross-namespace static-call return-type inference** — cgen's
   isStdlib short-circuit skips MethodRetRawSet/Get for
   short-syntax stdlib calls. So `let rows = SQLite.QueryAll(...)`
   doesn't carry `List<List<string>>` raw type. User annotates
   inner list explicitly.

4. **`amc` doesn't auto-link non-test builds** — `amc -o foo
   bar.am` emits `foo.c` only. `amc test` does the gcc step
   internally + splices vendored .o objects. For `amc -o`
   workflows, gcc by hand.

5. **MemberTable.Set silent-no-op on duplicate** — important
   resolver invariant from v0.4.7. Don't change to error.

6. **CGen-precedence bug on `(A || B) && C`** — cgen re-
   associates to `A || (B && C)` in the C output. Workaround:
   split kind+name check across multiple `if` statements.

7. **String-interpolation conflict with `${VAR:-default}`** —
   collides when emitting shell-script templates. Sentinel
   workaround: `let lb = "{"; let rb = "}"; …`.

8. **Curly braces in string templates** — escape `{` and `}` if
   literal contains them.

9. **gen_test linking and libcurl** — adding a runtime symbol
   that pulls in curl means gen_test now needs `-lcurl` too
   (build_amc.sh handles it since v0.4.0).

10. **Bump the version constant BEFORE every tag** — single
    source of truth is `PackageRegistry.AmcVersion()`.

11. **One-time post-clone setup for `merge=ours`** —
    `.gitattributes` declares `merge=ours` for
    `snapshot/amc_lib.c`, `snapshot/INFO.md`, and `src/amc_lib.c`.
    Run once: `git config merge.ours.driver true`.

12. **C++ packages use a two-stage link** — gcc -c on the
    cgen-emitted .c (amc emits C-style `void*` casts that g++
    rejects with `-fpermissive` warnings as errors), g++ on the
    resulting .o + the .o cache. Implemented in `RunTest` since
    v0.5.3.

13. **`Path.X` syntax doesn't work in non-Path namespaces** —
    pre-existing bug. `Path` is NOT in the cgen's
    `isCoreStdlib` list (because its facade method names like
    `Directory` map to runtime `Path_GetDirectory`, the rule
    "Class.Method → Class_Method" breaks). Workaround: call
    runtime symbols directly (`Path_Combine(a, b)` instead of
    `Path.Combine(a, b)`).

14. **DuckDB bare amalgamation missing aggregates** — SUM /
    AVG / STDDEV live in the `core_functions` extension. The
    wrapper enables auto-install but needs network on first
    call. Offline: stick to COUNT(*) / MIN / MAX.

15. **PAT exposure in chat** — fine-grained PAT for
    PACKAGES_INDEX_PAT must never be pasted in the chat.
    Always add via GitHub web UI directly to repo secrets.
    If exposed, revoke immediately at
    https://github.com/settings/tokens?type=beta.

═══════════════════════════════════════════════════════════════
  Roadmap snapshot (next-up after v0.6.0)
═══════════════════════════════════════════════════════════════

Backlog quick-wins (not yet picked up):

- TTL TOML scheme — switch from shell `date -r` to a
  `File_Mtime` runtime helper for true cross-platform support
  (Windows native cmd.exe doesn't have GNU date).
- `amc package outdated` — list installed deps that have newer
  indexed versions matching their constraint.
- `amc package info <pkg>` — full details on one package.
- `amc package notice` — aggregate license info for installed
  deps (commercial downstream).
- `amc package check --frozen` — CI fail-fast if lock doesn't
  match installed.
- `amc --version` enriched with git rev + build date.
- Yanking support: `yanked = true` in [[version]] blocks.
- Transitive deps + cycle detection.
- Path deps (`{ path = "../foo" }`).
- `amc package vendor` (commit cache into repo for offline
  reproducible builds).
- Help text audit pass (some subcommand --help still drift).

External-package backlog:

- **DuckDB v0.2.x** — prepared statements, typed accessors,
  Parquet helpers, transactions.
- **PostgreSQL** — dynamic-link libpq, first dynamic-dep
  package (exercises a new manifest pattern).
- **MySQL / MariaDB** — same shape as Postgres.
- **MongoDB** — libmongoc + libbson, C-API binding.
- **NATS Core** — pure-Amalgame protocol (~250 LoC).
- **DuckDB v1.x** — vendor `core_functions` extension so SUM /
  AVG work offline out of the box.

Compiler / tooling backlog:

- **Multi-version coexistence** — dual-link, package A wants
  redis@v1 + B wants redis@v2.
- **`Amalgame.Service` v2** — native Windows SCM dispatcher,
  drops NSSM dep.
- **`amc new --template service` v2** — macOS launchd.
- **LSP slice 6** — tighter selectionRange + more code actions.
- **ORM layer** — sits above the SQL backend packages.
- **`Path.X` shorthand fix** — restructure cgen's stdlib
  dispatch to allow facade-name divergence (Path.Directory →
  Path_GetDirectory), not just direct mapping.

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
    language/                      ← grammar.ebnf + grammar.md
    changelog/                     ← per-version PDF builds
    proposals/                     ← design docs
    DEVELOPER_GUIDE.md
  editors/vscode/                  ← extension.js + grammar
  runtime/                         ← _runtime.h + Amalgame_*.h
  snapshot/
    amc_lib.c                      ← portable bootstrap source
    amc                            ← compiled snapshot (gitignored)
    INFO.md                        ← provenance
  src/
    main.am                        ← CLI entry, RunTest + dispatch
    amc_lib.c                      ← generated; merge=ours
    package_registry.am            ← PackageRegistry + LoadedPackage
                                     + Calibration + AmalgameHome
                                     + VersionSatisfies operators
    add_cmd.am                     ← all `amc package <action>`
                                     verbs incl. auto-resolve
    lexer/, parser/, resolver/,
    generator/, typechecker.am,
    linter.am, lsp.am, formatter/  ← compiler internals
    stdlib/                        ← facades for Json/Toml/Random/
                                     Encoding/DateTime/Crypto/Path/
                                     Logging/Service
    migrate.am / generate.am /
    explain.am / new_cmd.am
  tests/
    run_all_tests.sh
    run_tests.sh
    run_stdlib_tests.sh
    run_fmt_tests.sh
    run_amc_new_tests.sh
    fixtures/pm/                   ← package-manager test fixtures
    samples/                       ← .am test inputs
  tools/
    save-snapshot.sh
    release.sh                     ← end-to-end release flow

═══════════════════════════════════════════════════════════════
  TL;DR for the new session
═══════════════════════════════════════════════════════════════

Pick up from **v0.6.0**. develop and main both at v0.6.0, synced,
working tree clean. All five overnight tags published (v0.5.3 →
v0.6.0). `~/.local/bin/amc` is the user-installed copy.

Big shifts since the last CONTINUATION.md (v0.5.2):

1. **C++ packages pipeline** (v0.5.3) — `[stdlib].sources` accepts
   .cpp, manifest gains `cflags`/`cxxflags`/`libs`/`schema-version`.
   Auto-link in `amc test`. First user: DuckDB.

2. **Precompile-on-install + calibration** (v0.5.4) — heavy C/C++
   packages opt in via `[stdlib].precompile = true`; install
   pays the compile cost once into a persistent platform-tagged
   cache. `~/.amalgame/calibration.toml` auto-learns compile
   speed on the machine. Cross-platform `$HOME` resolution
   (POSIX + Windows native).

3. **Search/versions with compat** (v0.5.5) — `amc package search`
   and `amc package versions <pkg>` show all indexed tags with
   ✓/✗ compat marker. `list` shows pinned version. `remove`
   accepts `@<tag>` safety suffix.

4. **Index cache TTL + runner fixes** (v0.5.6) — 30-min auto-
   refresh on the index cache (was forever-cached). Redis +
   MQTT runners fixed (were using obsolete `amc add` syntax,
   silently SKIPping every test since v0.5.1).

5. **Auto-resolve + semver operators** (v0.6.0) — `amc package
   add <shortname>` (no tag) picks the latest compatible from
   the index. `required-amalgame` accepts ^/~/>=/>/<=/</= as
   well as bare versions.

6. **packages-index schema v2** — top-level `[[version]]` array
   with `required-amalgame` per tag. Auto-update via
   `.github/workflows/index-pr.yml` in each package repo on tag
   push (validated in prod on sqlite v0.2.2).

Memory feedbacks still apply by default: répondre en français /
pas de Co-Authored-By trailer / édits autonomes après plan
validé / features sur develop.

The most natural next directions:

1. **5e external package** — PostgreSQL (libpq dynamic-link,
   first to exercise the dynamic-dep manifest pattern) or
   DuckDB v0.2 (prepared statements + typed accessors).

2. **`amc package outdated`** — quick-win observability verb.
   Reads lock + index, lists deps where a newer compatible
   version is available.

3. **`File_Mtime` in runtime** — replaces shell `date -r` in
   `IndexCacheIsFresh`, makes TTL truly cross-platform
   (Windows native cmd.exe lacks GNU date).

4. **Yanking** — `yanked = true` in [[version]] blocks, search
   surfaces a ⚠ marker, `add` refuses unless `--allow-yanked`.

5. **CI automation refinement** — first auto-PR shipped, but
   the workflow could become more idiomatic (e.g. group
   multiple version pushes if they happen close together,
   add a `--dry-run` mode).

Ask me which direction before diving in.
```
