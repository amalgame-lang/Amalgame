#!/usr/bin/env bash
#
# Golden-C hosted-regression guard  (amc MCU project, Phase 1 safety net)
# ----------------------------------------------------------------------
# docs/proposals/amc-embedded.md §0: "The hosted target is frozen. A hosted
# build, after all this work, must produce a binary byte-identical to today."
#
# This snapshots the C that amc emits for a curated corpus on the DEFAULT
# (hosted) target and asserts it stays byte-identical. Once --target= work
# starts threading a target through c_gen.am, ANY accidental drift in hosted
# codegen fails here — that's the trip-wire that protects POSIX/Windows.
#
# Usage:
#   tools/golden-c/golden-c.sh            # check against committed goldens (CI/default)
#   tools/golden-c/golden-c.sh --update   # regenerate goldens (after an INTENTIONAL change)
#
# Determinism: amc emits relative `#line` paths from the argument we pass
# (tests/samples/<name>.am), so generated C is stable across machines as long
# as this script is run from the repo root. No normalization needed.
set -uo pipefail

cd "$(dirname "$0")/../.." || exit 2          # repo root
ROOT=$(pwd)
AMC="$ROOT/amc"
SAMPLES_DIR="tests/samples"
GOLDEN_DIR="tools/golden-c/golden"
LIST="tools/golden-c/samples.txt"

[ -x "$AMC" ] || { echo "!! no ./amc binary at repo root (build it first)"; exit 2; }

MODE="check"
[ "${1:-}" = "--update" ] && MODE="update"

mkdir -p "$GOLDEN_DIR"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

mapfile -t NAMES < <(grep -vE '^\s*#|^\s*$' "$LIST")

fail=0 checked=0 updated=0
for name in "${NAMES[@]}"; do
  src="$SAMPLES_DIR/$name.am"
  if [ ! -f "$src" ]; then echo "!! missing sample: $src"; fail=1; continue; fi

  # Generate C. amc writes <out>.c before linking; codegen must succeed.
  if ! "$AMC" "$src" -o "$TMP/$name" >"$TMP/$name.log" 2>&1; then
    echo "!! codegen FAILED for $name (see below)"; sed 's/^/     /' "$TMP/$name.log"
    fail=1; continue
  fi
  gen="$TMP/$name.c"
  [ -f "$gen" ] || { echo "!! no .c emitted for $name"; fail=1; continue; }

  golden="$GOLDEN_DIR/$name.c"
  if [ "$MODE" = "update" ]; then
    cp "$gen" "$golden"; updated=$((updated+1)); echo "   updated $name.c"
  else
    if [ ! -f "$golden" ]; then
      echo "!! no golden for $name (run --update to create)"; fail=1; continue
    fi
    if ! diff -u "$golden" "$gen" >"$TMP/$name.diff"; then
      echo "!! DRIFT in hosted codegen for $name:"; sed 's/^/     /' "$TMP/$name.diff"
      fail=1
    else
      checked=$((checked+1))
    fi
  fi
done

if [ "$MODE" = "update" ]; then
  echo "golden-c: updated $updated sample(s)."
  exit 0
fi

if [ "$fail" -ne 0 ]; then
  echo "golden-c: FAIL — hosted codegen drifted. If intentional, re-run with --update."
  exit 1
fi
echo "golden-c: PASS — $checked sample(s) byte-identical to golden."
