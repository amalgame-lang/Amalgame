#!/bin/bash
./build/amc ./tests/samples/if_expr.am 2>&1 | grep -v "^Compiling\|^Lexer\|^Parser\|^Resolver\|^TypeChecker\|^GCC"
echo "=== Relevant C ==="
grep -n "label\|grade\|bigger\|isAdult\|ternary\|if\|?" \
    ./tests/samples/if_expr.c 2>/dev/null | head -30
