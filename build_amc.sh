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

echo "=== Step 1: Build gen_test (bootstrap bundles generator) ==="
./build/amc $AMC_SOURCES \
            src/amalgame/generator/gen_test.am \
            -o gen_test
gcc -O2 -Isrc/transpiler/runtime gen_test.c -lgc -lm -o gen_test

echo "=== Step 2: Generate bootstrap bundles ==="
./gen_test

echo "=== Step 3: Generate amc_lib.c (build/amc Vala CGen) ==="
./build/amc $AMC_SOURCES -o src/amalgame/amc_lib

echo "=== Step 4: Fix amc_lib.c (patch missing fields) ==="
python3 fix_amc_lib.py

echo "=== Step 5: Compile amc binary ==="
gcc -Isrc/transpiler/runtime \
    src/amalgame/amc_lib.c \
    src/amalgame/amc_main.c \
    -lgc -lm -lcurl -o amc
echo "✅ amc built in $(date)"
