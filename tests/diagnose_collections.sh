#!/bin/bash
echo "=== Generated C (relevant parts) ==="
./build/amc ./tests/samples/stdlib_collections.am 2>&1 | grep -v "^Compiling\|^Lexer\|^Parser\|^Resolver\|^TypeChecker\|^Generator\|^GCC\|^Build\|^Run"
echo ""
echo "=== Relevant C lines ==="
grep -n "AmalgameList\|AmalgameMap\|AmalgameSet\|count\|empty\|first\|last\|hasAlpha\|hasDelta\|mapSize\|setSize\|hasMage" \
    ./tests/samples/stdlib_collections.c 2>/dev/null | head -40
