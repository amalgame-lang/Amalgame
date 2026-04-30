#!/bin/bash
echo "=== Full amc output for enums.am ==="
./build/amc ./tests/samples/enums.am 2>&1
echo ""
echo "=== Generated C ==="
cat ./tests/samples/enums.c 2>/dev/null || echo "no .c generated"
