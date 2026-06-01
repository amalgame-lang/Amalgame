# Using packages

Amalgame's standard library is deliberately small. Most capabilities —
HTTP, TLS, crypto, datetime, Redis, the Mosaic web framework — ship as
**external packages** under `github.com/amalgame-lang/amalgame-*`, pulled
in per project. This chapter is the end-to-end workflow: find a package,
add it, use it, keep it current, and troubleshoot.

> For the exhaustive command + manifest **reference** (every flag, every
> `amalgame.toml` field, semver operators), see
> [03-cli-reference.md](03-cli-reference.md). This chapter is the
> task-oriented companion.

The two files that track your dependencies:

- **`amalgame.toml`** — what you *asked for* (package + tag). You edit
  this (directly or via `amc package add/remove`); commit it.
- **`amalgame.lock`** — what was *resolved* (exact Git revisions). Auto
  generated; commit it too, so collaborators and CI build the same code.

---

## Find a package

```bash
amc package search redis          # substring match across the index
amc package info redis            # description, url, license, versions
amc package versions redis        # every indexed tag + compat with your amc
```

`search` marks each tag `✓`/`✗` against your running `amc` and flags the
`← latest compatible` one. The index is cached ~30 min; add `--refresh`
to force a re-fetch.

Not sure which package provides a type you saw? `suggest` maps a class
or namespace back to a package:

```bash
amc package suggest WebApp        # → amalgame-web
amc package suggest Amalgame.Net.Http
```

---

## Add a package

```bash
amc package add redis             # latest tag compatible with your amc
amc package add redis@v0.2.0      # pin an exact tag
amc package add web datetime      # several at once
```

A bare `amc package add <name>` (indexed shortname, no `@tag`)
auto-resolves the latest tag compatible with your installed compiler and
prints what it picked:

```
Auto-resolved 'redis' → 'redis@v0.2.0' (latest compatible with amc 0.8.71)
```

`add` writes the dep into `amalgame.toml`, records the resolved revision
in `amalgame.lock`, and (unless `--no-precompile`) compiles the package
at install time so the first build of your code is fast.

Your `amalgame.toml` gains an entry like:

```toml
[dependencies.redis]
git = "github.com/amalgame-lang/amalgame-database-nosql-redis"
tag = "v0.2.0"
```

and `amalgame.lock` pins the exact commit:

```toml
[[package]]
name = "redis"
git  = "github.com/amalgame-lang/amalgame-database-nosql-redis"
tag  = "v0.2.0"
rev  = "…"
```

> **Transitive dependencies resolve automatically** (amc ≥ 0.8.39). When
> you add `web`, its own deps (`net-http`, `tls`, `crypto`, …) are pulled
> in too — you don't list them yourself.

---

## Use it in code

A package declares the namespace it contributes (its `[stdlib].namespace`).
You `import` that namespace and call its types — `amc build` auto-attaches
the matching package facade:

```amalgame
import Amalgame.Collections
import Amalgame.Net.Http
import Amalgame.Web

public class Program {
    public static void Main(string[] args) {
        let app = new WebApp()
        app.Get("/", ctx => HttpResponse.New().Text("hi"))
        app.Serve(8080)
    }
}
```

```bash
amc build main.am        # resolves deps from amalgame.toml + .lock, links them
```

The `import` line names the package's *namespace*, not its slug — e.g.
the `web` package exposes `Amalgame.Web`, and `net-http` exposes
`Amalgame.Net.Http`. `amc package info <name>` shows the namespace; so
does the package's README.

---

## Keep dependencies current

```bash
amc package outdated              # deps with a newer compatible tag
amc package update redis@v0.3.0   # bump a pinned tag (updates toml + lock)
```

`update` takes an explicit `name@tag` — there is no blind "update
everything", on purpose: you choose each bump and the lockfile records
it. After updating, rebuild and run your tests.

Remove a dep you no longer need:

```bash
amc package remove redis          # strips it from toml + lock
amc package remove redis@v0.2.0   # @tag is a safety check — refuses if
                                  # the installed tag differs
```

---

## Reproducible builds (CI)

Commit **both** `amalgame.toml` and `amalgame.lock`. On a fresh checkout,
verify the cache matches the lock before building:

```bash
amc package check --frozen        # exits 1 if cache ≠ lock — fail the CI job
```

`--frozen` is the CI-friendly form: it never mutates anything, just
asserts that what's installed matches what the lockfile pins. Without
`--frozen`, `check` reports mismatches without failing.

Audit licences and authorship of everything you pull in:

```bash
amc package notice                # aggregated licence + authorship of deps
```

---

## Troubleshooting

**`amc build` reports `Unknown symbol 'WebApp'` (or similar) for a type
you know is in a package.** The package isn't attached. Check that:
1. it's in `amalgame.toml` (`amc package list` shows installed deps);
2. your source `import`s the package's **namespace** (e.g.
   `import Amalgame.Web`), not its slug.
   amc auto-attaches a facade only when the matching namespace is imported.

> The amc **LSP** (VS Code / Neovim) can show *false* `Unknown symbol`
> warnings for package types even when `amc build` succeeds — the editor
> doesn't always load the lockfile. Trust the build; the diagnostic is
> the known gap, not your code.

**No `@tag` and the wrong version resolves.** Auto-resolution only works
for **indexed shortnames**. For an unindexed package, or to be explicit,
pin the tag: `amc package add <name>@<tag>` (or a full git URL).

**A package needs a newer amc than you have.** `search` / `versions`
mark incompatible tags `✗`. The package's `[package].required-amalgame`
constraint (e.g. `">=0.8.58"`) is checked at resolve time — upgrade amc,
or pin an older compatible tag.

**Version constraints in a package's own `[dependencies]`** use npm-style
operators (`>=`, `^`, `~`, `=`, bare = `>=`); see the
[CLI reference](03-cli-reference.md#amalgametoml-manifest-reference).

**Stale index or cache.** `amc package search --refresh` re-fetches the
index; `amc package cache clear` drops it (`--all` also clears downloaded
packages).

---

## Publishing your own package

If you're *authoring* a package rather than consuming one, the shape of
`amalgame.toml` (the `[package]` + `[stdlib]` blocks: `class`/`classes`,
`namespace`, `header`, `facade`, `sources`, `libs`) is documented in the
[CLI reference manifest section](03-cli-reference.md#amalgametoml-manifest-reference).
The convention for getting listed in the index lives in the
`amalgame-lang/packages-index` repo.
