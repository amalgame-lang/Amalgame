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
#     path, math, math_vec, json, toml
#   - Cross-stdlib (uses `--external` to thread other modules through):
#     msgpack (references Json's JsonValue)
#
# Other user-facing modules (random / encoding / crypto / datetime /
# logging / service / io-filewatcher / yaml / regex / compress /
# websocket) live in external packages under amalgame-lang/ and are
# installed on demand via `amc package add`.

set -e
cd "$(dirname "$0")/.."

OUT="lib"
BUILD=$(mktemp -d -t amc-stdlib-XXXXXX)
trap 'rm -rf "$BUILD"' EXIT

echo "── libamalgame.a — pre-compile user-facing stdlib modules ──"

# v0.8.49: amc auto-includes every cached package's runtime header
# (via its package-registry registration). If any of those headers
# transitively `#include` another package's header (e.g. net-http
# v0.9.1+'s Amalgame_Net_Http.h includes "Amalgame_Async.h"), gcc
# needs the sibling's runtime/ on -I. Pre-cook the flags from the
# cache; user-set CPPFLAGS keeps precedence (appended last).
PKG_INCLUDES=""
if [ -d "$HOME/.amalgame/packages/github.com/amalgame-lang" ]; then
    for pkg_parent in "$HOME/.amalgame/packages/github.com/amalgame-lang"/*/; do
        latest=$(ls -1 "$pkg_parent" 2>/dev/null | sort -V | tail -1)
        runtime_dir="${pkg_parent}${latest}/runtime"
        if [ -n "$latest" ] && [ -d "$runtime_dir" ]; then
            PKG_INCLUDES="$PKG_INCLUDES -I${runtime_dir}"
        fi
    done
fi
CPPFLAGS="$PKG_INCLUDES ${CPPFLAGS:-}"

# Sanity: amc must exist. Windows / MSYS2 produces amc.exe — pick
# whichever is on disk so we don't have to rename across platforms.
if [ -x ./amc ]; then
    AMC=./amc
elif [ -x ./amc.exe ]; then
    AMC=./amc.exe
else
    echo "ERROR: neither ./amc nor ./amc.exe found. Run ./build_amc.sh first." >&2
    exit 1
fi

# Post-v0.7.7 stdlib split: only the bootstrap deps stay bundled
# in `lib/libamalgame.a`. Everything else (random/encoding/crypto/
# datetime/logging/service/io-filewatcher/yaml/math/math-vec) lives
# in external packages on amalgame-lang/. See README "Ecosystem".
#
# Standalone modules — no cross-stdlib references. amc compiles
# each one in isolation.
MODULES_STANDALONE="path math math_vec json toml"

# Cross-stdlib modules — passed `--external <dep.am>` so the cgen
# routes inter-module references through the dependency's own
# namespace mangling instead of re-emitting a duplicate symbol.
# Entry format: "module|external1,external2,..."
#
# msgpack stays bundled until amc's cgen learns to resolve bundled-
# stdlib class names from external packages (deferred to v0.8.0).
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
    if ! "$AMC" --lib --quiet -o "$BUILD/$mod" "$src" $ext_flags > /tmp/amc-stdlib-$mod.log 2>&1; then
        echo "amc FAIL"
        echo "  ── source diagnostics ──"
        wc -l "$src" | sed 's/^/  /'
        # Hex-dump the line(s) the lexer flagged so we can spot CRLF
        # drift, BOMs, or other invisible content shifts between
        # checkout platforms.
        echo "  md5: $(md5sum "$src" 2>/dev/null || md5 -q "$src" 2>/dev/null)"
        echo "  ── amc log ──"
        sed 's/^/  /' /tmp/amc-stdlib-$mod.log
        exit 1
    fi
    if ! gcc -O2 -Iruntime $CPPFLAGS -c "$BUILD/$mod.c" -o "$BUILD/$mod.o" 2> /tmp/gcc-stdlib-$mod.log; then
        echo "gcc FAIL"
        echo "  ── gcc log ──"
        sed 's/^/  /' /tmp/gcc-stdlib-$mod.log
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
echo "  gcc -Iruntime myapp.c $OUT/libamalgame.a -lgc -lm -lz -o myapp"
