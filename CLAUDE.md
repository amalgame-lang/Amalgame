# Notes for AI assistants working on amc

## Bootstrap problem (most common gotcha)

`build_amc.sh` needs a working `./amc` (or `./snapshot/amc`) at least one
minor version ahead of … well, ~the current source. If the snapshot is
too old, the build fails with errors like:

```
gen_test.c: fatal error: Amalgame_Math.h: No such file or directory
```

The root cause: the old amc binary still emits `#include` lines for
runtime headers that have since been migrated to pure-AM facades and
deleted from `runtime/`.

### Fix in this order

1. **Look for a recent release binary** before doing anything else:
   - `~/Téléchargements/amc-*/bin/amc`
   - `~/.local/bin/amc`
   - `/usr/local/bin/amc`
2. **Replace `./amc` with that binary** — `cp <path> ./amc`
3. **Run `./build_amc.sh`** — should succeed in ~10s
4. **Refresh the snapshot afterwards** so the next contributor doesn't
   hit the same wall:
   ```
   ./tools/save-snapshot.sh
   git add snapshot/ && git commit -m "chore: refresh snapshot to vX.Y.Z"
   ```

Do NOT spend time hunting through `git log` to restore individual
deleted runtime headers (`Amalgame_Math.h`, `Amalgame_Regex.h`, etc.) —
that path leads to cascading missing-header errors. The right fix is a
newer bootstrap binary.

## Project layout

- `src/` — AM source code of amc (lexer, parser, resolver, typechecker, cgen, …)
- `runtime/` — C runtime headers consumed by generated user code
- `stdlib/` — *(deprecated, single file `strings.am`)* — most stdlib lives in `src/stdlib/`
- `src/stdlib/` — pure-AM stdlib facades (json, toml, msgpack, path)
- `tests/samples/` — integration test inputs
- `tests/run_*.sh` — test runners
- `docs/` — proposals, guides, internal docs
- `snapshot/` — bootstrap amc binary (must stay reasonably fresh)

## Package convention

External packages live in sibling git repos `amalgame-lang/amalgame-*`.
A package ships an `amalgame.toml` with `[stdlib]` declaring:
- `class` *(single)* or `classes = [...]` *(multi, since v0.8.29)*
- `namespace` (`Amalgame.X.Y`)
- `header` (runtime C header path, can be a stub)
- `facade` (optional pure-AM `.am` source)
- `libs = ["…"]` (extra `-l<x>` at link)
- `[stdlib.functions]` table mapping method → C return type

## Web stack (in progress, see docs/proposals/amalgame-web.md)

- `amalgame-tls` v0.1.0 — TLS via OpenSSL 3.x
- `amalgame-net-http` v0.1.0 — HTTP/1.1 (pure AM) + planned HTTP/2 via nghttp2
- `amalgame-web` (not started) — Mosaic framework

Sibling repos under `/home/neitsab/Développement/amalgame-*/`.

## Open architectural decisions (validated 2026-05-18)

- **Math + Math.Vec should NOT be external packages.** They're
  fundamental — every program that does numbers uses them. The
  migration to external in v0.7.5 was a mistake. PR-worthy cleanup:
  bring them back as bundled stdlib.
- **libcurl should NOT be an amc build dep.** `Amalgame_Net.h` currently
  exposes `Http_Get/Post/...` via libcurl, which forces every amc
  binary to link curl. The new `amalgame-net-http` package replaces
  this. Plan: remove HTTP from `Amalgame_Net.h`, drop `-lcurl` from
  `build_amc.sh`. Out of scope for the current web-stack push.
