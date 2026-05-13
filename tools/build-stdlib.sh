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
# Modules covered:
#   - Standalone (no cross-stdlib references):
#     random, encoding, crypto, datetime, logging, path, service,
#     json, toml, yaml
#   - Cross-stdlib (uses `--external` to thread other modules through):
#     msgpack (references Json's JsonValue)
#
# Math.Vec stays out — its real impl lives in
# `runtime/Amalgame_Math_Vec.h` (the AM file is a facade stub for
# the resolver). Same shape as path / logging / service for the
# v0.7.4-style isCoreStdlib dispatch path.

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

# Standalone modules — no cross-stdlib references. amc compiles
# each one in isolation.
MODULES_STANDALONE="random encoding crypto datetime logging path service json toml yaml"

# Cross-stdlib modules — passed `--external <dep.am>` so the cgen
# routes inter-module references through the dependency's own
# namespace mangling instead of re-emitting a duplicate symbol.
# Entry format: "module|external1,external2,..."
MODULES_CROSS="msgpack|json"

mkdir -p "$OUT"
rm -f "$OUT/libamalgame.a"

build_one() {
    local mod="$1"
    local externs="$2"
    local src="src/stdlib/$mod.am"
    if [ ! -f "$src" ]; then
        echo "  skip $mod (file not found)"
        return 0
    fi
    local ext_flags=""
    if [ -n "$externs" ]; then
        IFS=',' read -ra deps <<< "$externs"
        for d in "${deps[@]}"; do
            ext_flags="$ext_flags --external src/stdlib/$d.am"
        done
    fi
    printf "  %-12s " "$mod"
    if ! ./amc --lib --quiet -o "$BUILD/$mod" "$src" $ext_flags > /tmp/amc-stdlib-$mod.log 2>&1; then
        echo "amc FAIL (see /tmp/amc-stdlib-$mod.log)"
        exit 1
    fi
    if ! gcc -O2 -Iruntime $CPPFLAGS -c "$BUILD/$mod.c" -o "$BUILD/$mod.o" 2> /tmp/gcc-stdlib-$mod.log; then
        echo "gcc FAIL (see /tmp/gcc-stdlib-$mod.log)"
        exit 1
    fi
    local bytes
    bytes=$(stat -c%s "$BUILD/$mod.o" 2>/dev/null || stat -f%z "$BUILD/$mod.o")
    if [ -n "$externs" ]; then
        echo "$bytes bytes (deps: $externs)"
    else
        echo "$bytes bytes"
    fi
}

for mod in $MODULES_STANDALONE; do
    build_one "$mod" ""
done

for entry in $MODULES_CROSS; do
    mod="${entry%%|*}"
    deps="${entry#*|}"
    build_one "$mod" "$deps"
done

ar rcs "$OUT/libamalgame.a" "$BUILD"/*.o
arsize=$(stat -c%s "$OUT/libamalgame.a" 2>/dev/null || stat -f%z "$OUT/libamalgame.a")
echo ""
echo "✓ $OUT/libamalgame.a ($arsize bytes)"
echo ""
echo "Usage from user code:"
echo "  amc -o myapp myapp.am --external src/stdlib/random.am"
echo "  gcc -Iruntime myapp.c $OUT/libamalgame.a -lgc -lm -lcurl -lz -o myapp"
