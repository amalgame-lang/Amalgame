#!/bin/bash
# Diagnostic script - shows exact errors for each failing test
CODEC="./build/codec"
SAMPLES="./tests/samples"

diag() {
    local name="$1"
    local file="$2"
    echo ""
    echo "════════════════════════════════════"
    echo "  $name"
    echo "════════════════════════════════════"
    output=$("$CODEC" "$file" 2>&1)
    echo "$output" | grep -v "^GCC\|^Build\|^Run\|^Compiling\|^Lexer\|^Parser\|^Resolver\|^TypeChecker\|^Generator"
    local exe="${file/.code/}"
    if [ -x "$exe" ]; then
        echo "--- Runtime ---"
        "$exe" 2>&1
        echo "Exit: $?"
    fi
    echo "--- Generated C ---"
    cat "${file/.code/.c}" 2>/dev/null || echo "(no .c file)"
}

diag "variables"     "$SAMPLES/variables.code"
diag "control_flow"  "$SAMPLES/control_flow.code"
diag "classes"       "$SAMPLES/classes.code"
diag "math"          "$SAMPLES/math.code"
diag "records"       "$SAMPLES/record.code"

echo ""
echo "════════════════════════════════════"
echo "  classes (detailed)"
echo "════════════════════════════════════"
./build/codec ./tests/samples/classes.code 2>&1

echo ""
echo "════════════════════════════════════"
echo "  classes C + linker error"
echo "════════════════════════════════════"
./build/codec ./tests/samples/classes.code 2>&1 | grep -v "^Compiling\|^Lexer\|^Parser\|^Resolver\|^TypeChecker\|^Generator\|^GCC\|^Build\|^Run"
cat ./tests/samples/classes.c 2>/dev/null
