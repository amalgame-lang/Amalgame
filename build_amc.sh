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
AMC_SOURCES="src/amalgame/lexer/token.am \
             src/amalgame/lexer/lexer.am \
             src/amalgame/parser/ast.am \
             src/amalgame/parser/parser.am \
             src/amalgame/generator/c_gen.am \
             src/amalgame/diagnostics.am \
             src/amalgame/resolver/symbol.am \
             src/amalgame/resolver/resolver.am \
             src/amalgame/typechecker.am"

# Self-host: use ./amc if available, fall back to ./build/amc (Vala) for cold start
if [ -x ./amc ]; then
    AMC=./amc
    echo "=== Step 1: Build gen_test (self-hosted via ./amc) ==="
else
    AMC=./build/amc
    echo "=== Step 1: Build gen_test (cold start via Vala ./build/amc) ==="
fi
# amc exits non-zero on resolver warnings (e.g. when a new builtin was just
# added but the running amc was compiled before it was registered). Accept
# that as long as the .c file was produced, then proceed to GCC which is
# the real correctness gate.
$AMC $AMC_SOURCES \
     src/amalgame/generator/gen_test.am \
     -o gen_test || true
if [ ! -f gen_test.c ]; then
    echo "Step 1 failed: gen_test.c was not produced" >&2
    exit 1
fi
gcc -O2 -Isrc/transpiler/runtime gen_test.c -lgc -lm -o gen_test

echo "=== Step 2: Generate all bundles + amc_lib.c ==="
time ./gen_test

echo "=== Step 3: Compile amc ==="
# main.am now emits its own int main() — no more amc_main.c wrapper needed.
gcc -Isrc/transpiler/runtime \
    src/amalgame/amc_lib.c \
    -lgc -lm -lcurl -o amc
echo "✅ amc built $(date)"
