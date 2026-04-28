#!/bin/bash
# ─────────────────────────────────────────────────────
#  Amalgame Transpiler — Test Runner
#  Usage: ./tests/run_tests.sh
# ─────────────────────────────────────────────────────

AMC="./build/amc"
SAMPLES="./tests/samples"
PASS=0
FAIL=0
SKIP=0

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

run_test() {
    local name="$1"
    local file="$2"
    local expected="$3"

    printf "  %-30s" "$name"

    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        SKIP=$((SKIP + 1))
        return
    fi

    output=$("$AMC" "$file" 2>&1)
    amc_exit=$?

    if [ $amc_exit -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (amc exited $amc_exit)"
        echo "$output" | grep -E "error|Error" | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1))
        return
    fi

    exe="${file/.am/}"
    if [ ! -x "$exe" ]; then
        echo -e "${RED}FAIL${NC} (executable not found)"
        FAIL=$((FAIL + 1))
        return
    fi

    run_output=$("$exe" 2>&1)
    run_exit=$?

    if [ $run_exit -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (runtime exited $run_exit)"
        echo "$run_output" | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1))
        return
    fi

    if [ -n "$expected" ]; then
        if echo "$run_output" | grep -qF "$expected"; then
            echo -e "${GREEN}PASS${NC}"
            PASS=$((PASS + 1))
        else
            echo -e "${RED}FAIL${NC} (output mismatch)"
            echo "    expected: $expected"
            echo "    got:      $(echo "$run_output" | head -1)"
            FAIL=$((FAIL + 1))
        fi
    else
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    fi
}

echo ""
echo "═══════════════════════════════════════"
echo "  Amalgame Transpiler — Test Suite"
echo "═══════════════════════════════════════"
echo ""

if [ ! -f "$AMC" ]; then
    echo "Error: amc not found. Run 'cd build && ninja' first."
    exit 1
fi

echo "── Core ────────────────────────────────"
run_test "hello world"     "$SAMPLES/hello.am"        "Hello World !"
run_test "variables"       "$SAMPLES/variables.am"    "int: 42"
run_test "control flow"    "$SAMPLES/control_flow.am" "Grade: B"
run_test "classes"         "$SAMPLES/classes.am"      "Cat says hello!"
run_test "match"           "$SAMPLES/match.am"        "Slightly wounded"
run_test "math functions"  "$SAMPLES/math.am"         "5! = 120"
run_test "records"         "$SAMPLES/record.am"       "Point: (3"

echo ""
echo "───────────────────────────────────────"
echo -e "  ${GREEN}PASS: $PASS${NC}  |  ${RED}FAIL: $FAIL${NC}  |  ${YELLOW}SKIP: $SKIP${NC}"
echo "───────────────────────────────────────"
echo ""

[ $FAIL -eq 0 ] && exit 0 || exit 1
