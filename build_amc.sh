#!/bin/bash
# build_amc.sh — Fast rebuild of amc compiler (~10s total)
# Usage: ./build_amc.sh
set -e
cd "$(dirname "$0")"

AMC_SOURCES="src/amalgame/lexer/token.am \
             src/amalgame/lexer/lexer.am \
             src/amalgame/parser/ast.am \
             src/amalgame/parser/parser.am \
             src/amalgame/generator/c_gen.am \
             src/amalgame/resolver/symbol.am \
             src/amalgame/resolver/resolver.am \
             src/amalgame/diagnostics.am \
             src/amalgame/typechecker.am \
             src/amalgame/main.am"

# Self-host: use ./amc if available, fall back to ./build/amc (Vala) for cold start
if [ -x ./amc ]; then
    AMC=./amc
    echo "=== Step 1: Build gen_test (self-hosted via ./amc) ==="
else
    AMC=./build/amc
    echo "=== Step 1: Build gen_test (cold start via Vala ./build/amc) ==="
fi
$AMC $AMC_SOURCES \
     src/amalgame/generator/gen_test.am \
     -o gen_test
gcc -O2 -Isrc/transpiler/runtime gen_test.c -lgc -lm -o gen_test

echo "=== Step 2: Generate all bundles + amc_lib.c ==="
time ./gen_test

echo "=== Step 3: Compile amc ==="
gcc -Isrc/transpiler/runtime \
    src/amalgame/amc_lib.c \
    src/amalgame/amc_main.c \
    -lgc -lm -lcurl -o amc
echo "✅ amc built $(date)"
