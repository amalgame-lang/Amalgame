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
    local flags="${4:-}"

    printf "  %-34s" "$name"

    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        SKIP=$((SKIP + 1)); return
    fi

    output=$("$AMC" $flags "$file" 2>&1)
    amc_exit=$?

    if [ $amc_exit -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (amc exited $amc_exit)"
        echo "$output" | grep -E "error|Error|\[resolver\]|\[typechecker\]" \
            | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1)); return
    fi

    exe="${file/.am/}"
    if [ ! -x "$exe" ]; then
        echo -e "${RED}FAIL${NC} (executable not found)"
        FAIL=$((FAIL + 1)); return
    fi

    run_output=$("$exe" 2>&1)
    run_exit=$?

    if [ $run_exit -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (runtime exited $run_exit)"
        echo "$run_output" | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1)); return
    fi

    if [ -n "$expected" ]; then
        if echo "$run_output" | grep -qF "$expected"; then
            echo -e "${GREEN}PASS${NC}"
            PASS=$((PASS + 1))
        else
            echo -e "${RED}FAIL${NC} (output mismatch)"
            echo "    expected : $expected"
            echo "    got      : $(echo "$run_output" | head -3 | tr '\n' '|')"
            FAIL=$((FAIL + 1))
        fi
    else
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    fi
}

run_lib_test() {
    local name="$1"
    local file="$2"
    local flags="${3:-}"

    printf "  %-34s" "$name"

    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        SKIP=$((SKIP + 1)); return
    fi

    output=$("$AMC" $flags "$file" 2>&1)
    amc_exit=$?

    if [ $amc_exit -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (amc exited $amc_exit)"
        echo "$output" | grep -E "error|Error" | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1)); return
    fi

    if echo "$output" | grep -q "Library"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (expected Library mode)"
        FAIL=$((FAIL + 1))
    fi
}

run_c_check() {
    local name="$1"
    local file="$2"
    local c_pattern="$3"
    local flags="${4:-}"

    printf "  %-34s" "$name"

    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        SKIP=$((SKIP + 1)); return
    fi

    "$AMC" $flags "$file" >/dev/null 2>&1
    c_file="${file/.am/.c}"

    if [ ! -f "$c_file" ]; then
        echo -e "${RED}FAIL${NC} (no .c generated)"
        FAIL=$((FAIL + 1)); return
    fi

    if grep -q "$c_pattern" "$c_file"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found in C)"
        echo "    looking for: $c_pattern"
        FAIL=$((FAIL + 1))
    fi
}

# ── Banner ─────────────────────────────────────────────
echo ""
echo "═══════════════════════════════════════"
echo "  Amalgame Transpiler — Test Suite"
echo "═══════════════════════════════════════"
echo ""

if [ ! -f "$AMC" ]; then
    echo "Error: amc not found. Run 'cd build && ninja' first."
    exit 1
fi

# ── Core ───────────────────────────────────────────────
echo "── Core ────────────────────────────────"
run_test "hello world"       "$SAMPLES/hello.am"        "Hello World !"
run_test "variables"         "$SAMPLES/variables.am"    "int: 42"
run_test "control flow"      "$SAMPLES/control_flow.am" "Grade: B"
run_test "classes"           "$SAMPLES/classes.am"      "Cat says hello!"
run_test "match"             "$SAMPLES/match.am"        "Slightly wounded"
run_test "math functions"    "$SAMPLES/math.am"         "5! = 120"
run_test "records"           "$SAMPLES/record.am"       "Point: (3"

# ── Advanced ───────────────────────────────────────────
echo ""
echo "── Advanced ────────────────────────────"
run_test "inheritance"       "$SAMPLES/inheritance.am"  "Circle 'Sun'"
run_test "data classes"      "$SAMPLES/data_classes.am" "Arthus"
run_test "generics/utils"    "$SAMPLES/generics.am"     "Max: 99"
run_test "closures"          "$SAMPLES/closures.am"     "Counter = 10"

# ── Namespace ──────────────────────────────────────────
echo ""
echo "── Namespace ───────────────────────────"
run_test    "sub-namespace runtime"  "$SAMPLES/namespace.am"   "Arthus (lvl 42)"
run_c_check "namespace C prefix"    "$SAMPLES/namespace.am"   "MyApp_Models_Player"
run_c_check "struct prefixed"       "$SAMPLES/namespace.am"   "struct _MyApp_Models_Player"
run_c_check "method prefixed"       "$SAMPLES/namespace.am"   "MyApp_Models_Player_Info"
run_c_check "hello prefix MyApp"    "$SAMPLES/hello.am"       "MyApp_Program_Main"

# ── Library mode ───────────────────────────────────────
echo ""
echo "── Library mode ────────────────────────"
run_lib_test  "auto-detect lib"       "$SAMPLES/library.am"
run_c_check   "lib: no int main"      "$SAMPLES/library.am"     "Library — no entry point"
run_c_check   "lib: symbols prefixed" "$SAMPLES/library.am"     "Amalgame_Utils_StringHelper"
run_lib_test  "forced lib (--lib)"    "$SAMPLES/forced_lib.am"  "--lib"
run_c_check   "forced: no int main"   "$SAMPLES/forced_lib.am"  "Library — no entry point" "--lib"
run_test      "forced: normal mode"   "$SAMPLES/forced_lib.am"  "localhost:8080"

# ── Extended coverage ──────────────────────────────────
echo ""
echo "── Extended coverage ───────────────────"
run_test "operators"         "$SAMPLES/operators.am"        "add: 13"
run_test "logical ops"       "$SAMPLES/operators.am"        "and: false"
run_test "strings concat"    "$SAMPLES/strings.am"          "Hello, World!"
run_test "string interp"     "$SAMPLES/strings.am"          "Product: 42"
run_test "loops break"       "$SAMPLES/loops.am"            "w: 0"
run_test "loops continue"    "$SAMPLES/loops.am"            "odd: 1"
run_test "loops nested"      "$SAMPLES/loops.am"            "diag: 0"
run_test "null/bool"         "$SAMPLES/null_safety.am"      "name1: Arthus"
run_test "coalesce"          "$SAMPLES/null_safety.am"      "v2: 99"
run_test "default ctor"      "$SAMPLES/static_class.am"     "zero: true"
run_test "expr body methods" "$SAMPLES/static_class.am"     "prod: 42"
run_test "pattern advanced"  "$SAMPLES/pattern_advanced.am" "small"
run_test "pattern range"     "$SAMPLES/pattern_advanced.am" "Monday"
run_test "multi-class"       "$SAMPLES/multi_class.am"      "42 Rue de la Paix"
run_test "composition"       "$SAMPLES/multi_class.am"      "Adult: true"
run_test "recursion fib"     "$SAMPLES/recursion.am"        "fib(10) = 55"
run_test "recursion gcd"     "$SAMPLES/recursion.am"        "gcd(48,18) = 6"
run_test "recursion pow"     "$SAMPLES/recursion.am"        "pow(2,8) = 256"
run_test "explicit types"    "$SAMPLES/type_explicit.am"    "count: 2"
run_test "negative numbers"  "$SAMPLES/type_explicit.am"    "neg: -42"

# ── Summary ────────────────────────────────────────────
echo ""
echo "───────────────────────────────────────"
echo -e "  ${GREEN}PASS: $PASS${NC}  |  ${RED}FAIL: $FAIL${NC}  |  ${YELLOW}SKIP: $SKIP${NC}"
echo "───────────────────────────────────────"
echo ""

[ $FAIL -eq 0 ] && exit 0 || exit 1
