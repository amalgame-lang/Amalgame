# Proposal: Stdlib package manager + DB/Messaging extraction

**Status:** design (2026-05-11). Implementation phased across 5 PRs targeted at v0.5.0.
**Author:** v0.4 → v0.5 cycle, post-MQTT (v0.4.17 develop tip).
**Tracking PRs:** TBD (this doc only).

## Problem

Optional stdlib backends — `Amalgame.Database.SQLite` (v0.4.15),
`Amalgame.Database.NoSQL.Redis` (v0.4.17), `Amalgame.Messaging.MQTT`
(v0.4.18 / batched into v0.5) — currently land in the monolithic
compiler tree:

- `runtime/Amalgame_<Family>_<Engine>.h` ships with every `amc` binary
- `src/generator/c_gen.am` hardcodes their class names in `isStdlib`,
  their `<Class>_<Method>` → return-type table, and their `#include`
  in the prelude emit
- `src/resolver/resolver.am` declares every `<Class>_<Method>` global
- `tests/stdlib_bundle/stdlib_test.am` knows about each test fixture + its
  TCP-reachability gate

This works fine at N=3 backends. It will not at N=10:

| Pain point | When it bites |
|---|---|
| `c_gen.am` + `resolver.am` become merge-hot | 4+ parallel backend PRs in flight |
| Compiler binary embeds every backend whether the user calls it or not | When SQLite-only users complain about 9MB of vendored sqlite3 they don't need |
| Backends evolve at different cadences than the language | A bugfix in `amalgame-database-nosql-redis` shouldn't need a new amc release |
| External contributors can't ship a backend without committing to the core | Anyone wanting to publish `amalgame-postgres` today must PR against the compiler |
| CI in the main repo grows linearly with backends — install redis-server, mosquitto, libpq, libmariadbclient, … | The 24 SKIP cases from v0.4.17 hint at this; multiply by 5 backends and CI complexity dominates |

The conclusion was reached during the v0.4.17 cycle (see chat log /
commit `eddc52f` for Redis discussion): keep landing optional backends
in monolithic form *as long as it's cheap*; extract the inaugural set
when the third backend lands (MQTT) and the model starts to hurt.

## Goals

1. **Optional backends as external Git repos**, each with its own
   release cadence, its own CI, its own contributor pool.
2. **Zero-infra distribution** — no central registry to operate.
3. **Reproducible builds** — pinned commit SHAs in a lock file.
4. **Discoverability** — devs can find official packages without
   memorising Git URLs.
5. **Curation control** — the project author (Bastien Mouget) decides
   what gets the "official" badge.
6. **No code execution at install time** — manifest is data only;
   reject build hooks / pre-install scripts on principle.
7. **Compatible migration** — `Amalgame.Database.SQLite` /
   `.NoSQL.Redis` / `.Messaging.MQTT` user code at v0.4.17 keeps
   working at v0.5 with one extra `amc add` step.

## Non-goals (deferred to v0.6+)

- Semver range resolution (`^0.1`, `~1.2.3`). v0.5 uses exact tag pins.
- Dual-linking two versions of the same package. v0.5 = one version
  per binary, conflict = explicit error.
- Centralised package registry server. v0.5 piggybacks on GitHub.
- Workspaces / monorepos with multiple packages per repo.
- Signed packages (PGP / sigstore). v0.5 verifies tarball SHA-256 only.
- Yanked-version markers. Handled by tag deletion + lockfile resolution
  failure for now.

## Design decisions

### 1. Manifest format — TOML (`amalgame.toml`)

Cargo precedent (anyone who's touched Rust recognises the format).
Parser ~250 LoC in Amalgame for the subset we need: tables, key/value,
strings, integers, booleans, arrays, inline tables. Comments matter
(rationale notes in the manifest itself; JSON can't do this).

Rejected:
- **Amalgame-native (`package.am`)** — couples manifest format to
  language version, turns config into Turing-complete code (security
  footgun on the Cargo `build.rs` model), and breaks third-party
  tooling (Dependabot, SBOM generators) that can't invoke amc to read
  it.
- **JSON** — no comments, verbose, less "premium" feel for an
  ecosystem launch.

### 2. Distribution — Git URL + tag

`amc add github.com/amalgame-lang/amalgame-database-nosql-redis@v0.1.0`. No registry,
no infra to operate. Follows the Go / Zig / Deno / Nimble / Crystal
Shards pattern.

```toml
[dependencies]
redis = { git = "github.com/amalgame-lang/amalgame-database-nosql-redis", tag = "v0.1.0" }
```

Rejected:
- **Central registry (Cargo / npm model)** — requires us to host,
  mirror, sign, takedown, abuse-monitor. Solo author can't carry that
  operational load.
- **Tarball URLs** — git tags already provide what tarballs would, and
  git supports shallow clones (`--depth 1 --branch v0.1.0`).

Hybrid path stays open: v0.6+ can add a registry resolver without
breaking git-URL deps. The reverse (registry-only → git) is harder.

### 3. Lock file — `amalgame.lock` with SHA + sha256

Reproducible builds need bit-for-bit pinning. Tags can technically be
reassigned (force-push); the SHA cannot.

```toml
# amalgame.lock — auto-generated, committed
[[package]]
name   = "redis"
git    = "github.com/amalgame-lang/amalgame-database-nosql-redis"
tag    = "v0.1.0"
rev    = "abc123def456..."   # SHA of the tagged commit
sha256 = "..."               # tarball checksum
deps   = []
```

### 4. Storage layout — global cache, no project-local `vendor/`

```
~/.amalgame/packages/
└── github.com/
    └── amalgame-lang/
        └── amalgame-database-nosql-redis/
            ├── v0.1.0_abc123/         # one revision = one directory
            ├── v0.1.1_def456/
            └── current → v0.1.1_def456 (managed by amc)
```

Different versions coexist in the cache — no rebuild if project A
uses 0.1.0 and project B uses 0.1.1. Project-local `vendor/`
(npm-style `node_modules`) is wasteful at filesystem scale and bloats
git repos.

### 5. Inter-package dependencies — transitive auto, conflict = error

| Case | Behaviour |
|---|---|
| Transitive (A → B → C) | C is installed automatically, full tree flattened into `amalgame.lock` |
| Diamond, same version (A→B@1, A→C@1, B→D@1, C→D@1) | D installed once, deduplicated by `(name, rev)` |
| Diamond, different versions | **Error**, user pins manually in `amalgame.toml` |
| Circular (A→B→A) / cross | **Error**, detected during DFS walk |
| Two versions of same package in one binary | Not supported in v0.5; flat symbol space (`Redis_Set` etc.) can't dual-link |

Error UX:
```
ERROR: version conflict on `amalgame-messaging-mqtt`:
  ├─ my-project requires v0.1.0 (direct)
  └─ amalgame-messaging-mqtt-tls@v0.1.0 requires v0.2.0 (via transitive)
Fix: update your direct dep to v0.2.0, OR pin amalgame-messaging-mqtt-tls to an
     older version that wants v0.1.0.
```

```
ERROR: dependency cycle detected:
  foo → bar → baz → foo
```

### 6. Bootstrap — TOML parser lives in `src/stdlib/toml.am`

The package manager logic (in `amc` itself) needs to parse manifests
at every compile (resolver / cgen feed from them). The parser must
therefore be available *before* any external package can be installed
— it can't itself be an external package.

Three options considered, one survives:

| Option | Verdict |
|---|---|
| TOML parser in C runtime (`runtime/Amalgame_Toml.h`) | OK but loses dogfooding |
| **TOML parser in stdlib core (`src/stdlib/toml.am`)** | ✅ Same pattern as `Amalgame.Json` today. Pure Amalgame, ~250 LoC. amc imports it. |
| TOML parser as external package | ❌ Circular, can't bootstrap |

### 7. Discovery — `amalgame-lang/packages-index` repo

Curated `packages.toml` listing the canonical packages, fetched via
`raw.githubusercontent.com` (cached 1h in `~/.amalgame/cache/`). No
search server, no API — `amc search redis` greps the local cache.

**Two-tier policy** (the curation surface):

- **Tier 1 — official**: code lives under `github.com/amalgame-lang/`
  org. Bastien Mouget maintains. Badge shown at install:
  `✓ official, maintained by amalgame-lang`.
- **Tier 2 — listed**: code lives anywhere (third-party). PR'd into
  the index, reviewed + approved + merged by Bastien. Badge:
  `✓ listed, verified by <reviewer> on <date>`.
- **Unverified**: any other Git URL. `amc add github.com/foo/...`
  always works but shows a warning: `⚠ unverified — not in the
  official index. Manual review recommended.`

**What ownership means** (the legal nuance):
- Bastien owns the **index entry** (it's in his repo, his licence).
- The package author owns their **code** (it's at their URL, their
  licence). Listing does not transfer copyright; Apache-2.0 / MIT
  upstream means downstream forks must preserve `LICENSE` / `NOTICE`.
- Bastien can revoke the official/listed badge at any time (PR
  removal). He cannot delete the upstream code.

```toml
# amalgame-lang/packages-index/packages.toml

[[package]]
name        = "redis"
url         = "github.com/amalgame-lang/amalgame-database-nosql-redis"
description = "RESP2 client over raw TCP. No vendored lib."
tier        = "official"
maintainer  = "amalgame-lang"
license     = "Apache-2.0"
category    = "database"

[[package]]
name        = "discord-bot-helper"
url         = "github.com/randomuser/amalgame-discord-bot-helper"
description = "Helpers for Discord bot patterns."
tier        = "listed"
maintainer  = "randomuser"
license     = "MIT"
category    = "community"
verified_on = "2026-05-15"
verified_by = "bmtbf"
```

### 7.b. Index schema evolution: v1 → v2

The v1 schema above (one `[[package]]` block per shortname, full
stop) shipped with v0.5.0 and worked fine for `amc package add
<name>@<tag>` — the resolver only needed to map `<shortname>` →
git URL. But **`amc package search` and `amc package versions`
needed a list of every tag known for a package, plus each tag's
`required-amalgame` constraint**, without cloning every repo at
search time. So v0.5.5 promoted the packages-index to schema v2.

```toml
schema-version = 1   # the index file format; bumped only on
                     # breaking changes to the index TOML shape.
                     # Schema v2 adds [[version]] but keeps
                     # [[package]] unchanged, so the integer
                     # version field stays at 1 (back-compat with
                     # v0.5.4 amc, which ignores [[version]]).

# [[package]] entries — unchanged from v1, one per shortname.

[[package]]
name        = "duckdb"
url         = "github.com/amalgame-lang/amalgame-database-duckdb"
description = "DuckDB binding — vendored C++ amalgamation (MIT)…"
tier        = "official"
maintainer  = "amalgame-lang"
license     = "Apache-2.0"
category    = "database"

# [[version]] — NEW in v2 — flat array, one entry per
# (shortname, tag) pair. Linked to [[package]] by the `package`
# field. Append newest-last; amc walks the array and uses the
# last-seen compatible entry for auto-resolve + search "latest
# compatible" marker.

[[version]]
package           = "duckdb"
tag               = "v0.1.0"
required-amalgame = ">=0.5.3"

[[version]]
package           = "duckdb"
tag               = "v0.1.1"
required-amalgame = ">=0.5.4"
```

**Why a flat array instead of nesting under `[[package]]`?**

TOML's `[[…]]` arrays can't be nested as cleanly as we'd like —
a `[[package.version]]` syntax exists but breaks the
human-readability we want for hand-edited PRs to the index repo.
A flat `[[version]]` block linked by `package = "<shortname>"`
keeps every entry self-contained and editable in isolation, at
the cost of one extra string per row.

**Why newest-last (instead of newest-first)?**

PR diffs against the index are easier to review when each
release adds one block at the end of the file instead of
prepending and shifting everything. amc compensates by
iterating the array forward, tracking the last-seen compatible
entry, and reversing at render time for the user.

**Compatibility matrix:**

| amc version | Reads `[[package]]` | Reads `[[version]]` | Behaviour |
|---|---|---|---|
| ≤ v0.5.4 | ✓ | ignored | `amc package add foo@vX.Y.Z` works as before; no `search --version`. |
| ≥ v0.5.5 | ✓ | ✓ | `search` / `versions` show compat status; `add` (no tag) auto-resolves in v0.6.0+. |

Auto-resolve (v0.6.0+) reads `[[version]]` to pick the latest
compatible tag when the user types `amc package add <name>`
without `@<tag>`. The semver operator set
(`>=`, `>`, `<`, `<=`, `=`, `^`, `~`, bare) tracks the npm /
Cargo conventions; see `PackageRegistry.VersionSatisfies` for
the full grammar.

**Cache: 30-min TTL (v0.5.6+).** The index is cached at
`~/.amalgame/cache/packages-index.toml` with mtime-based
freshness (`date -r <file> +%s`). Older than 30 min → refresh
on next `search` / `versions` / `add`. Network failure during
refresh serves the stale cache with a warning on stderr (better
than hard-failing when offline). `--refresh` forces a re-fetch
unconditionally.

### 8. Validation at install — manifest sanity + supply-chain checks

Every `amc add` runs these checks after clone, before writing the lock:

| Check | Catches |
|---|---|
| `amalgame.toml` exists at repo root | Not an Amalgame package |
| Parses as valid TOML | Corrupt / malformed file |
| `[package].name` declared | Mandatory metadata missing |
| `[package].version` declared | Mandatory metadata missing |
| `[package].license` declared | Distribution legally unclear |
| `name` field matches the URL slug | Wrong namespace / impersonation |
| `version` field equals the tag (`v0.2.0` → `version = "0.2.0"`) | Mislabel / tag tampering |
| Commit SHA matches `git ls-remote` | Tag was reassigned after our first view |
| Tarball sha256 matches lock file | Cache tampering |
| Has either `[stdlib]` or non-empty `[dependencies]` | Empty package (suspicious) |
| `required-amalgame` (if declared) satisfied by current amc | Package wants a newer amc than user has |

### 9. Extra features locked in

| Item | Spec for v0.5 |
|---|---|
| **Path deps** | `{ path = "../amalgame-foo" }` — local dev before publication |
| **amc version pin** | `[package].required-amalgame = ">=0.5.0"` in manifest |
| **Offline mode** | `amc build --offline` builds from cache, no network |
| **Build hooks / pre-install scripts** | ❌ Not supported. Supply-chain attack vector. If a package needs generated code, commit it. |

## Implementation phasing

5 PRs incremental, each independently mergeable:

| PR | Scope | Risk |
|---|---|---|
| **1** | `src/stdlib/toml.am` — TOML parser + serializer. Tests. Pure addition. | low |
| **2** | `amc add <git-url>@<tag>` CLI. Clone, validate, write `amalgame.lock`. Doesn't yet affect compilation. | low |
| **3** | Resolver + cgen read package manifests. Replace hardcoded `isStdlib` + return-type table with manifest-driven dispatch. **Core compiler touch.** | **high** |
| **4** | Extract SQLite to `amalgame-lang/amalgame-database-sqlite` as the first inaugural package. Remove `runtime/Amalgame_Database_SQLite.h` + cgen/resolver hardcodes from the main repo. | medium — first real migration; may surface PR 3 bugs |
| **5** | Extract Redis + MQTT to their own repos. | low after PR 4 lands clean |

Plus tooling around it:

- `amc add <name>@<tag>` — shortname via packages-index, falls back to
  full URL
- `amc remove <name>` — strip from `amalgame.toml` + `.lock`, optionally
  GC the cache
- `amc list` — show installed packages with tier badge
- `amc list-official` — fetch + display the curated index
- `amc search <keyword>` — substring match against descriptions
- `amc update <name>` — bump a single dep (v0.5: requires user to
  specify the new tag; v0.6 might do semver range resolution)

Then **release v0.5**.

## Migration impact

User code at v0.4.17 today:

```amalgame
import Amalgame.Database.SQLite

let db = SQLite.Open(":memory:")
```

User code at v0.5+ (after extraction):

```bash
amc add sqlite@v0.1.0     # one-time, picks up from official index
```

```amalgame
import Amalgame.Database.SQLite     # unchanged
let db = SQLite.Open(":memory:")    # unchanged
```

The namespace + class surface is unchanged. The only delta is the
explicit dependency declaration — which is good ergonomics anyway
(matches the rest of the modern ecosystem).

The compiler can ship a **compat warning** in PR 4 / 5: if user code
imports `Amalgame.Database.SQLite` but no `sqlite` dep is declared in
`amalgame.toml`, emit a hint pointing at `amc add sqlite`.

## Open questions

- **Cache GC** — when does the global cache evict old versions?
  Probably manual: `amc gc` removes entries not referenced by any
  project's `amalgame.lock` reachable from the current directory. v0.6
  feature.
- **License compatibility checks** — should `amc add` warn when a
  GPL-only dep is added to an Apache-2.0 project? v0.6.
- **SBOM generation** — `amc sbom > sbom.spdx`. v0.6 ask; the lock
  file already has the data, just needs an exporter.
- **Private packages** — works out of the box via git auth (SSH key,
  GitHub token). No special handling for v0.5.
- **macOS / Windows path quoting** in cache paths with spaces — to
  verify in PR 2.

## References

- Cargo's manifest format: <https://doc.rust-lang.org/cargo/reference/manifest.html>
- Go modules: <https://go.dev/ref/mod>
- Zig build.zig.zon: <https://ziglang.org/learn/build-system/>
- The `Amalgame.Json` proposal (companion design doc):
  [`amalgame-json.md`](amalgame-json.md)
