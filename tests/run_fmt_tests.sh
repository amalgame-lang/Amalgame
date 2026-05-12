#!/bin/bash
# ─────────────────────────────────────────────────────
#  Amalgame — Formatter Tests
#  Verifies amc fmt is idempotent and preserves semantics.
#  Usage: ./tests/run_fmt_tests.sh
# ─────────────────────────────────────────────────────

set -u

# Use the self-hosted amc — the formatter only exists there.
AMC="./amc"
SAMPLES="./tests/samples"
PASS=0
FAIL=0

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo "── Formatter ──────────────────────────────"

if [ ! -x "$AMC" ]; then
    echo -e "${RED}  amc binary not found at $AMC — run build_amc.sh first${NC}"
    exit 1
fi

run_idempotence() {
    local name="$1"
    local file="$2"

    printf "  %-34s" "$name"

    if [ ! -f "$file" ]; then
        echo -e "${RED}FAIL${NC} (file not found)"
        FAIL=$((FAIL + 1)); return
    fi

    local out1="/tmp/amc_fmt_out1_$$.am"
    local out2="/tmp/amc_fmt_out2_$$.am"

    "$AMC" fmt "$file" > "$out1" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (fmt failed on input)"
        FAIL=$((FAIL + 1)); rm -f "$out1" "$out2"; return
    fi

    "$AMC" fmt "$out1" > "$out2" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (fmt failed on first output)"
        FAIL=$((FAIL + 1)); rm -f "$out1" "$out2"; return
    fi

    if ! diff -q "$out1" "$out2" >/dev/null 2>&1; then
        echo -e "${RED}FAIL${NC} (not idempotent)"
        diff "$out1" "$out2" | head -10 | sed 's/^/    /'
        FAIL=$((FAIL + 1)); rm -f "$out1" "$out2"; return
    fi

    echo -e "${GREEN}PASS${NC}"
    PASS=$((PASS + 1))
    rm -f "$out1" "$out2"
}

# Verifies the formatted output still compiles and produces the same
# stdout as the original.
run_semantic() {
    local name="$1"
    local file="$2"
    local libs="${3:--lgc -lm -lcurl -lz}"

    printf "  %-34s" "$name"

    if [ ! -f "$file" ]; then
        echo -e "${RED}FAIL${NC} (file not found)"
        FAIL=$((FAIL + 1)); return
    fi

    local fmtd="/tmp/amc_fmt_fmtd_$$.am"
    local orig_c="/tmp/amc_fmt_orig_$$"
    local fmtd_c="/tmp/amc_fmt_fmtd_$$"

    "$AMC" fmt "$file" > "$fmtd" 2>/dev/null
    "$AMC" "$file" -o "$orig_c" >/dev/null 2>&1
    "$AMC" "$fmtd" -o "$fmtd_c" >/dev/null 2>&1

    gcc -Iruntime "$orig_c.c" $libs -o "$orig_c" 2>/dev/null
    gcc -Iruntime "$fmtd_c.c" $libs -o "$fmtd_c" 2>/dev/null

    local o1=$("$orig_c" 2>&1)
    local o2=$("$fmtd_c" 2>&1)

    if [ "$o1" != "$o2" ]; then
        echo -e "${RED}FAIL${NC} (output differs)"
        FAIL=$((FAIL + 1))
    else
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    fi
    rm -f "$fmtd" "$orig_c" "$orig_c.c" "$fmtd_c" "$fmtd_c.c"
}

run_idempotence "fmt_basic.am — idempotent"  "$SAMPLES/fmt_basic.am"
run_semantic    "fmt_basic.am — semantic OK" "$SAMPLES/fmt_basic.am"

run_idempotence "list_comp.am — idempotent"  "$SAMPLES/list_comp.am"
run_semantic    "list_comp.am — semantic OK" "$SAMPLES/list_comp.am"

# match-as-expression (let x = match y { ... }) — also self-host only.
run_idempotence "match_expr.am — idempotent"  "$SAMPLES/match_expr.am"
run_semantic    "match_expr.am — semantic OK" "$SAMPLES/match_expr.am"

# Trailing same-line comments — `let x = 1  // foo` must round-trip with
# the comment on its source line (rather than being bumped to the next).
run_idempotence "fmt_comments.am — idempotent"  "$SAMPLES/fmt_comments.am"
run_semantic    "fmt_comments.am — semantic OK" "$SAMPLES/fmt_comments.am"

# Import directives — the parser used to drop them; they must now
# survive a fmt round-trip on prog.Args.
run_idempotence "fmt_imports.am — idempotent"  "$SAMPLES/fmt_imports.am"
run_semantic    "fmt_imports.am — semantic OK" "$SAMPLES/fmt_imports.am"

# try / catch / throw — round-trip through the formatter (TRY_STMT
# and THROW_STMT branches in EmitStmt + EmitTry / EmitThrow).
run_idempotence "try_catch.am — idempotent"  "$SAMPLES/try_catch.am"
run_semantic    "try_catch.am — semantic OK" "$SAMPLES/try_catch.am"

echo ""
echo "────────────────────────────────────────────"
echo -e "  ${GREEN}PASS: $PASS${NC}  |  ${RED}FAIL: $FAIL${NC}"
echo "────────────────────────────────────────────"

[ $FAIL -eq 0 ] && exit 0 || exit 1
