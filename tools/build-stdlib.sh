#!/bin/bash
# ─────────────────────────────────────────────────────
#  tools/build-stdlib.sh — pre-compile user-facing stdlib
#  modules into a libamalgame.a archive (project F MVP)
# ─────────────────────────────────────────────────────
#
# For each user-facing `src/stdlib/<mod>.am`, runs:
#
#     amc --lib --quiet -o $BUILD/<mod>  src/stdlib/<mod>.am
#     gcc -O2 -Iruntime -c $BUILD/<mod>.c -o $BUILD/<mod>.o
#
# Then archives the resulting .o files into `lib/libamalgame.a`
# (at repo root). User programs that import these modules can
# then drop the explicit `src/stdlib/<mod>.am` from the amc
# command line and pass `--external src/stdlib/<mod>.am` instead;
# the gcc link step picks up the symbol from `libamalgame.a` so
# no per-program re-compilation of the stdlib code happens.
#
# Usage:
#   ./tools/build-stdlib.sh
#
# v0.7.5 MVP scope: only modules with no cross-module dependencies
# are included (random, encoding, crypto, datetime, logging, path,
# service). The Formats.* family (Json, Toml, Yaml, MsgPack) and
# Math.Vec are tracked separately because they need either
# inter-module symbol resolution or runtime-header tweaks first.

set -e
cd "$(dirname "$0")/.."

OUT="lib"
BUILD=$(mktemp -d -t amc-stdlib-XXXXXX)
trap 'rm -rf "$BUILD"' EXIT

echo "── libamalgame.a — pre-compile user-facing stdlib modules ──"

# Sanity: amc must exist.
if [ ! -x ./amc ]; then
    echo "ERROR: ./amc not found. Run ./build_amc.sh first." >&2
    exit 1
fi

# v0.7.5 MVP scope. Modules listed here are all standalone — no
# import statements pulling in other stdlib classes by reference.
MODULES="random encoding crypto datetime logging path service"

mkdir -p "$OUT"
rm -f "$OUT/libamalgame.a"

for mod in $MODULES; do
    src="src/stdlib/$mod.am"
    if [ ! -f "$src" ]; then
        echo "  skip $mod (file not found)"
        continue
    fi
    printf "  %-12s " "$mod"
    if ! ./amc --lib --quiet -o "$BUILD/$mod" "$src" > /tmp/amc-stdlib-$mod.log 2>&1; then
        echo "amc FAIL (see /tmp/amc-stdlib-$mod.log)"
        exit 1
    fi
    if ! gcc -O2 -Iruntime -c "$BUILD/$mod.c" -o "$BUILD/$mod.o" 2> /tmp/gcc-stdlib-$mod.log; then
        echo "gcc FAIL (see /tmp/gcc-stdlib-$mod.log)"
        exit 1
    fi
    bytes=$(stat -c%s "$BUILD/$mod.o" 2>/dev/null || stat -f%z "$BUILD/$mod.o")
    echo "$bytes bytes"
done

ar rcs "$OUT/libamalgame.a" "$BUILD"/*.o
arsize=$(stat -c%s "$OUT/libamalgame.a" 2>/dev/null || stat -f%z "$OUT/libamalgame.a")
echo ""
echo "✓ $OUT/libamalgame.a ($arsize bytes)"
echo ""
echo "Usage from user code:"
echo "  amc -o myapp myapp.am --external src/stdlib/random.am"
echo "  gcc -Iruntime myapp.c $OUT/libamalgame.a -lgc -lm -lcurl -lz -o myapp"
