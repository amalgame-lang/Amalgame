# golden-c — hosted codegen regression guard

The trip-wire that protects the **hosted (POSIX/Windows) target** while the
MCU work (`docs/proposals/amc-embedded.md`) threads a `--target` through the
~5400-LOC `src/generator/c_gen.am`.

Governing rule (proposal §0): *the hosted target is frozen — a hosted build
must stay byte-identical to today.* This harness snapshots the C amc emits
for a curated corpus and fails on **any** drift.

## Use

```bash
tools/golden-c/golden-c.sh            # check (CI / before saying a slice is green)
tools/golden-c/golden-c.sh --update   # regenerate goldens after an INTENTIONAL change
```

- Exit 0 + `PASS` → hosted codegen unchanged.
- Exit 1 + a unified diff → hosted codegen drifted. If the change was
  deliberate, eyeball the diff, then `--update` and commit the new goldens.

## Layout

- `samples.txt` — curated `tests/samples/<name>.am` basenames (broad cgen
  coverage, deterministic builds). Add a sample only with its golden in the
  same commit.
- `golden/<name>.c` — committed expected output. Full files (not hashes) so a
  drift is **visible as a diff**.

## Why it's deterministic

amc emits relative `#line` paths from the path we pass it
(`tests/samples/<name>.am`), so output is stable across machines **when run
from the repo root** (the script enforces this). No normalization needed.

## Workflow during MCU phases

Run `golden-c.sh` after every cgen-touching change. The embedded gates in
cgen are additive (`if target.embedded { … }`), never taken on hosted — so a
green golden-c is the proof the additive gates didn't leak into hosted output.
