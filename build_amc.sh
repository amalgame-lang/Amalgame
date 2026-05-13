#!/bin/bash
# build_amc.sh — Fast rebuild of amc compiler (~10s total)
# Usage: ./build_amc.sh
set -e
cd "$(dirname "$0")"

# diagnostics.am defines SourceMap/SourceSnippet, used by resolver+typechecker.
# Order matters: dependents come AFTER diagnostics in the source list.
#
# main.am is intentionally excluded: it defines its own Program.Main (the
# amc CLI entry point), which would clash with gen_test.am's Program.Main
# when bundled into the gen_test binary. main.am is only needed for the
# final amc binary (Step 3), and gen_test.am injects it into amc_lib.c
# via gen6 anyway.
AMC_SOURCES="src/lexer/token.am \
             src/lexer/lexer.am \
             src/parser/ast.am \
             src/parser/parser.am \
             src/stdlib/toml.am \
             src/stdlib/yaml.am \
             src/package_registry.am \
             src/generator/c_gen.am \
             src/formatter/formatter.am \
             src/diagnostics.am \
             src/resolver/symbol.am \
             src/resolver/resolver.am \
             src/typechecker.am \
             src/linter.am \
             src/stdlib/json.am \
             src/stdlib/random.am \
             src/stdlib/encoding.am \
             src/stdlib/datetime.am \
             src/stdlib/crypto.am \
             src/stdlib/msgpack.am \
             src/stdlib/path.am \
             src/stdlib/logging.am \
             src/stdlib/service.am \
             src/stdlib/amc_buildinfo.am \
             src/lsp.am \
             src/migrate.am \
             src/generate.am \
             src/explain.am \
             src/new_cmd.am \
             src/argparser.am \
             src/add_cmd.am"

# Self-host: 2-rung fallback chain.
#   ./amc                ← current self-hosted (may be broken mid-development)
#   ./snapshot/amc       ← last known-good Amalgame (tools/save-snapshot.sh)
#
# If neither exists, build snapshot/amc from the tracked snapshot/amc_lib.c:
#   gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -lz -o snapshot/amc
# Pre-flight: warn early if libgc-dev / libcurl headers are missing.
# Both are required by the runtime — the snapshot-bootstrap recovery
# step below will fail with "fatal error: gc.h" otherwise, which is
# the single most common bootstrap failure on a fresh machine.
if ! echo '#include <gc.h>' | gcc -E -x c - >/dev/null 2>&1; then
    echo "WARNING: <gc.h> not found — libgc-dev / bdw-gc may be missing." >&2
    echo "         Debian/Ubuntu: apt install libgc-dev" >&2
    echo "         macOS:         brew install bdw-gc" >&2
    echo "         MSYS2:         pacman -S mingw-w64-x86_64-gc" >&2
fi

# ── Step 0: Substitute build provenance into amc_buildinfo.am.
#    The template lives at src/stdlib/amc_buildinfo.am.in with
#    `__GIT_REV__` / `__BUILD_DATE__` placeholders; sed replaces
#    them with the real values (or empty strings when unavailable)
#    and emits the generated .am next to the template. Both values
#    flow as literal AM strings — no `-D` preprocessor mechanism,
#    no `@c { }`, pure Amalgame for `amc --version`.
echo "=== Step 0: Stamp build provenance into amc_buildinfo.am ==="
BI_GIT_REV=$(git rev-parse --short=8 HEAD 2>/dev/null || echo "")
BI_BUILD_DATE=$(date -u +%Y-%m-%dT%H:%M:%SZ 2>/dev/null || echo "")
sed -e "s|__GIT_REV__|$BI_GIT_REV|g" \
    -e "s|__BUILD_DATE__|$BI_BUILD_DATE|g" \
    src/stdlib/amc_buildinfo.am.in > src/stdlib/amc_buildinfo.am
echo "  rev:  $BI_GIT_REV"
echo "  date: $BI_BUILD_DATE"

if [ -x ./amc ]; then
    AMC=./amc
    echo "=== Step 1: Build gen_test (self-hosted via ./amc) ==="
elif [ -x ./snapshot/amc ]; then
    AMC=./snapshot/amc
    echo "=== Step 1: Build gen_test (recovery via ./snapshot/amc) ==="
else
    echo "ERROR: no amc binary found (./amc nor ./snapshot/amc)." >&2
    echo "" >&2
    echo "Bootstrap from the tracked C snapshot. Requires:" >&2
    echo "  • libgc-dev   (Debian/Ubuntu: apt install libgc-dev)" >&2
    echo "                (Fedora:        dnf install gc-devel)" >&2
    echo "                (macOS Homebrew: brew install bdw-gc)" >&2
    echo "                (MSYS2:         pacman -S mingw-w64-x86_64-gc)" >&2
    echo "  • libcurl development headers (libcurl4-openssl-dev / curl-devel / curl)" >&2
    echo "" >&2
    echo "Then:" >&2
    echo "  gcc -O2 -Iruntime \\" >&2
    echo "      -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable \\" >&2
    echo "      snapshot/amc_lib.c \\" >&2
    echo "      -lgc -lm -lcurl -lz -o snapshot/amc" >&2
    echo "" >&2
    echo "If gcc reports 'fatal error: gc.h: No such file or directory'," >&2
    echo "libgc-dev isn't installed. If it reports 'cannot find -lgc' at link" >&2
    echo "time, the shared library is missing — same package installs both." >&2
    exit 1
fi
# amc exits non-zero on resolver warnings (e.g. when a new builtin was just
# added but the running amc was compiled before it was registered). Accept
# that as long as the .c file was produced, then proceed to GCC which is
# the real correctness gate.
$AMC $AMC_SOURCES \
     src/generator/gen_test.am \
     -o gen_test || true
if [ ! -f gen_test.c ]; then
    echo "Step 1 failed: gen_test.c was not produced" >&2
    exit 1
fi
gcc -O2 -Iruntime -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable gen_test.c -lgc -lm -lcurl -lz -o gen_test

echo "=== Step 2: Generate all bundles + amc_lib.c ==="
time ./gen_test

echo "=== Step 3: Compile amc ==="
# main.am now emits its own int main() — no more amc_main.c wrapper needed.
#
# `-Wno-unused-*` keeps the build silent: v0.6.4 dropped the
# per-variable `__attribute__((unused))` markers + the
# `(void)param;` / `(void)self;` boilerplate the cgen used to emit
# (shrunk amc_lib.c by ~25%). `amc --lint` is the canonical
# "is this variable actually used" check now.
#
# Build provenance (git rev + build date) is no longer threaded
# through `-DAMC_*` macros — Step 0 above stamped the values
# directly into `src/stdlib/amc_buildinfo.am` as AM literal
# strings, which are already baked into the amc_lib.c we're
# about to link.
gcc -Iruntime \
    -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable \
    src/amc_lib.c \
    -lgc -lm -lcurl -lz -o amc
echo "✅ amc built $(date)"

# ── Step 4: Pre-compile the user-facing stdlib into lib/libamalgame.a
#    (project F, v0.7.5+). User programs that import these modules can
#    then pass `--external src/stdlib/<mod>.am` instead of bundling
#    the .am source — the lib supplies the symbols at link time. ──
echo "=== Step 4: Build lib/libamalgame.a ==="
./tools/build-stdlib.sh
