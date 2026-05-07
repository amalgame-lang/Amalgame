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
             src/generator/c_gen.am \
             src/formatter/formatter.am \
             src/diagnostics.am \
             src/resolver/symbol.am \
             src/resolver/resolver.am \
             src/typechecker.am"

# Self-host: 3-rung fallback chain.
#   ./amc                ← current self-hosted (may be broken mid-development)
#   ./snapshot/amc       ← last known-good Amalgame (tools/save-snapshot.sh)
#   ./build/amc          ← Vala bootstrap (frozen, original syntax only)
#
# Each rung knows strictly more syntax than the rung below, so falling
# down is safe; falling up from Vala when the language has evolved
# beyond what Vala parses is what the snapshot rung exists to bridge.
if [ -x ./amc ]; then
    AMC=./amc
    echo "=== Step 1: Build gen_test (self-hosted via ./amc) ==="
elif [ -x ./snapshot/amc ]; then
    AMC=./snapshot/amc
    echo "=== Step 1: Build gen_test (recovery via ./snapshot/amc) ==="
else
    AMC=./build/amc
    echo "=== Step 1: Build gen_test (cold start via Vala ./build/amc) ==="
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
gcc -O2 -Iruntime gen_test.c -lgc -lm -o gen_test

echo "=== Step 2: Generate all bundles + amc_lib.c ==="
time ./gen_test

echo "=== Step 3: Compile amc ==="
# main.am now emits its own int main() — no more amc_main.c wrapper needed.
gcc -Iruntime \
    src/amc_lib.c \
    -lgc -lm -lcurl -o amc
echo "✅ amc built $(date)"
