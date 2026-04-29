#!/bin/bash
# ─────────────────────────────────────────────────────
#  Amalgame Standard Library — Test Runner
#  Usage: ./tests/run_stdlib_tests.sh
#
#  Tests each stdlib module independently.
#  Some tests may require filesystem access (/tmp).
# ─────────────────────────────────────────────────────

AMC="./build/amc"
SAMPLES="./tests/samples"
PASS=0
FAIL=0
SKIP=0

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# ── Helpers (same as run_tests.sh) ────────────────────
run_test() {
    local name="$1"
    local file="$2"
    local expected="$3"
    local flags="${4:-}"

    printf "  %-38s" "$name"

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

run_skip() {
    local name="$1"
    local reason="$2"
    printf "  %-38s" "$name"
    echo -e "${YELLOW}SKIP${NC} ($reason)"
    SKIP=$((SKIP + 1))
}

# ── Banner ─────────────────────────────────────────────
echo ""
echo "════════════════════════════════════════════"
echo "  Amalgame Standard Library — Test Suite"
echo "════════════════════════════════════════════"
echo ""

if [ ! -f "$AMC" ]; then
    echo "Error: amc not found. Run 'cd build && ninja' first."
    exit 1
fi

# ── Amalgame.IO ────────────────────────────────────────
echo "── Amalgame.IO ─────────────────────────────"
run_test "IO: Console.WriteLine"      "$SAMPLES/stdlib_io.am"     "IO test start"
run_test "IO: File.WriteAll"          "$SAMPLES/stdlib_io.am"     "write ok: true"
run_test "IO: File.Exists"            "$SAMPLES/stdlib_io.am"     "exists: true"
run_test "IO: File.ReadAll"           "$SAMPLES/stdlib_io.am"     "content: Hello from Amalgame!"
run_test "IO: File.Size"              "$SAMPLES/stdlib_io.am"     "size: 20"
run_test "IO: File.Delete"            "$SAMPLES/stdlib_io.am"     "deleted: true"
run_test "IO: Path.Combine"           "$SAMPLES/stdlib_io.am"     "path: /tmp/test.txt"
run_test "IO: Path.GetExtension"      "$SAMPLES/stdlib_io.am"     "ext: .am"
run_test "IO: Path.GetFilename"       "$SAMPLES/stdlib_io.am"     "file: hello.am"

# ── Amalgame.Math ──────────────────────────────────────
echo ""
echo "── Amalgame.Math ───────────────────────────"
run_test "Math: Sqrt(16)"             "$SAMPLES/stdlib_math.am"   "sqrt(16) = 4"
run_test "Math: PowI(2,10)"           "$SAMPLES/stdlib_math.am"   "pow(2,10) = 1024"
run_test "Math: AbsI(-42)"            "$SAMPLES/stdlib_math.am"   "abs(-42) = 42"
run_test "Math: MaxI(10,42)"          "$SAMPLES/stdlib_math.am"   "max = 42"
run_test "Math: MinI(10,42)"          "$SAMPLES/stdlib_math.am"   "min = 10"
run_test "Math: ClampI(150,0,100)"    "$SAMPLES/stdlib_math.am"   "clamp = 100"
run_test "Math: Gcd(48,18)"           "$SAMPLES/stdlib_math.am"   "gcd(48,18) = 6"
run_test "Math: IsPrime(17)=true"     "$SAMPLES/stdlib_math.am"   "prime(17) = true"
run_test "Math: IsPrime(18)=false"    "$SAMPLES/stdlib_math.am"   "prime(18) = false"
run_test "Math: IsFinite(1.0)"        "$SAMPLES/stdlib_math.am"   "finite = true"
run_test "Math: SeedRandom"           "$SAMPLES/stdlib_math.am"   "rand seeded: ok"

# ── Amalgame.String ────────────────────────────────────
echo ""
echo "── Amalgame.String ─────────────────────────"
run_test "String: Length('Hello')"    "$SAMPLES/stdlib_string.am" "len = 5"
run_test "String: Contains"           "$SAMPLES/stdlib_string.am" "contains = true"
run_test "String: StartsWith"         "$SAMPLES/stdlib_string.am" "startsWith = true"
run_test "String: EndsWith"           "$SAMPLES/stdlib_string.am" "endsWith = true"
run_test "String: IndexOf"            "$SAMPLES/stdlib_string.am" "indexOf = 6"
run_test "String: ToUpper"            "$SAMPLES/stdlib_string.am" "upper = HELLO"
run_test "String: ToLower"            "$SAMPLES/stdlib_string.am" "lower = world"
run_test "String: Trim"               "$SAMPLES/stdlib_string.am" "trim = 'hello'"
run_test "String: Replace"            "$SAMPLES/stdlib_string.am" "replace = Hello Amalgame"
run_test "String: Repeat"             "$SAMPLES/stdlib_string.am" "repeat = ababab"
run_test "String: ToInt"              "$SAMPLES/stdlib_string.am" "toInt = 42"
run_test "String: FromInt"            "$SAMPLES/stdlib_string.am" "fromInt = 123"
run_test "String: IsEmpty"            "$SAMPLES/stdlib_string.am" "isEmpty = true"

# ── Amalgame.Collections (via runtime) ────────────────
echo ""
echo "── Amalgame.Collections ────────────────────"
run_skip "Collections: List<T>"       "planned — requires enum/generics"
run_skip "Collections: Map<K,V>"      "planned — requires enum/generics"
run_skip "Collections: Set<T>"        "planned — requires enum/generics"

# ── Amalgame.Net ───────────────────────────────────────
echo ""
echo "── Amalgame.Net ────────────────────────────"
run_skip "Net: Http.Get"              "planned"
run_skip "Net: WebSocket"             "planned"

# ── Summary ────────────────────────────────────────────
echo ""
echo "────────────────────────────────────────────"
echo -e "  ${GREEN}PASS: $PASS${NC}  |  ${RED}FAIL: $FAIL${NC}  |  ${YELLOW}SKIP: $SKIP${NC}"
echo "────────────────────────────────────────────"
echo ""

[ $FAIL -eq 0 ] && exit 0 || exit 1
