#!/bin/bash
# ─────────────────────────────────────────────────────
#  Amalgame Standard Library — Test Runner
#  Usage: ./tests/run_stdlib_tests.sh
#
#  Tests each stdlib module independently.
#  Some tests may require filesystem access (/tmp).
# ─────────────────────────────────────────────────────

AMC="./amc"
SAMPLES="./tests/samples"
PASS=0
FAIL=0
SKIP=0

# Same skip mechanism as run_tests.sh — see that file for the rationale.
SKIP_SELFHOST=" "

# Build artifacts go to a temp directory so the source tree stays clean.
BUILD_DIR=$(mktemp -d -t amc-stdlib-XXXXXX)
trap 'rm -rf "$BUILD_DIR"' EXIT

# Pre-compile the vendored SQLite amalgamation once. Database tests
# link against this .o — compiling 9MB of sqlite3.c per test would
# add 10s × N tests; doing it here adds 10s total. Built with the
# same flags as the test compile (-O2, -Iruntime so cross-references
# work) and with -w to silence sqlite3.c's own warnings.
SQLITE_OBJ="$BUILD_DIR/sqlite3.o"
echo "── Pre-compiling vendored SQLite amalgamation ──"
gcc -O2 -Iruntime -w -c runtime/Amalgame_Database/sqlite/sqlite3.c -o "$SQLITE_OBJ"
echo "  built: $SQLITE_OBJ ($(stat -c%s "$SQLITE_OBJ" 2>/dev/null || stat -f%z "$SQLITE_OBJ") bytes)"
echo ""

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
    # Optional 5th arg: extra .am files to compile alongside $file.
    # Used by Amalgame.Json tests to pull in src/stdlib/json.am, since
    # there's no module loader yet — the test compilation has to see
    # the library's source. Space-separated; passes through as args.
    local extra_inputs="${5:-}"

    printf "  %-38s" "$name"

    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        SKIP=$((SKIP + 1)); return
    fi

    local base=" $(basename "$file") "
    if [ "$AMC" = "./amc" ] && [[ "$SKIP_SELFHOST" == *"$base"* ]]; then
        echo -e "${YELLOW}SKIP${NC} (self-host: pending compiler fix)"
        SKIP=$((SKIP + 1)); return
    fi

    local out_base="$BUILD_DIR/$(basename "${file%.am}")"
    output=$("$AMC" $flags -o "$out_base" $extra_inputs "$file" 2>&1)
    amc_exit=$?

    if [ $amc_exit -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (amc exited $amc_exit)"
        echo "$output" | grep -E "error|Error|\[resolver\]|\[typechecker\]" \
            | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1)); return
    fi

    local c_file="${out_base}.c"
    if [ ! -f "$c_file" ]; then
        echo -e "${RED}FAIL${NC} (no .c emitted)"
        FAIL=$((FAIL + 1)); return
    fi
    # Always link the pre-compiled SQLite amalgamation. Adds ~1.5MB
    # to the test binary but means Database tests work without
    # special-casing — and non-Database tests get the link as
    # dead-code that the loader skips.
    gcc -O2 -Iruntime "$c_file" "$SQLITE_OBJ" -lgc -lm -lcurl -ldl -lpthread -o "$out_base" 2>/dev/null

    exe="$out_base"
    if [ ! -x "$exe" ]; then
        echo -e "${RED}FAIL${NC} (gcc failed)"
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

# ── Amalgame.Collections ────────────────────────────────
echo ""
echo "── Amalgame.Collections ────────────────────"
run_test "Collections: List.Add/Count"    "$SAMPLES/stdlib_collections.am" "count: 3"
run_test "Collections: List.IsEmpty"      "$SAMPLES/stdlib_collections.am" "empty: false"
run_test "Collections: List.Remove"       "$SAMPLES/stdlib_collections.am" "after remove: 2"
run_test "Collections: List.First/Last"   "$SAMPLES/stdlib_collections.am" "first: 10"
run_test "Collections: Map.Set/Size"      "$SAMPLES/stdlib_collections.am" "map size: 3"
run_test "Collections: Map.Has"           "$SAMPLES/stdlib_collections.am" "has alpha: true"
run_test "Collections: Map.Has missing"   "$SAMPLES/stdlib_collections.am" "has delta: false"
run_test "Collections: Map.Remove"        "$SAMPLES/stdlib_collections.am" "after remove: 2"
run_test "Collections: Set.Add/Size"      "$SAMPLES/stdlib_collections.am" "set size: 2"
run_test "Collections: Set.Contains"      "$SAMPLES/stdlib_collections.am" "has mage: true"
run_test "Collections: Set.Contains miss" "$SAMPLES/stdlib_collections.am" "has healer: false"
run_test "Collections: Set.Remove"        "$SAMPLES/stdlib_collections.am" "after remove: 1"

# ── Amalgame.Net ───────────────────────────────────────
echo ""
echo "── Amalgame.Net ────────────────────────────"

if curl -s --max-time 3 https://httpbin.org/get > /dev/null 2>&1 && \
   pkg-config --exists libcurl 2>/dev/null; then
    run_test "Net: Http.Get status"   "$SAMPLES/stdlib_net.am"  "status: 200"
    run_test "Net: Http.Get ok"       "$SAMPLES/stdlib_net.am"  "ok: true"
    run_test "Net: Http.GetHeaders"   "$SAMPLES/stdlib_net.am"  "headers ok: true"
    run_test "Net: Http.Post"         "$SAMPLES/stdlib_net.am"  "post ok: true"
    run_test "Net: done"              "$SAMPLES/stdlib_net.am"  "Net test done"
elif ! pkg-config --exists libcurl 2>/dev/null; then
    run_skip "Net: Http.Get status"   "libcurl-dev not installed"
    run_skip "Net: Http.Get ok"       "libcurl-dev not installed"
    run_skip "Net: Http.GetHeaders"   "libcurl-dev not installed"
    run_skip "Net: Http.Post"         "libcurl-dev not installed"
    run_skip "Net: done"              "libcurl-dev not installed"
else
    run_skip "Net: Http.Get status"   "no internet"
    run_skip "Net: Http.Get ok"       "no internet"
    run_skip "Net: Http.GetHeaders"   "no internet"
    run_skip "Net: Http.Post"         "no internet"
    run_skip "Net: done"              "no internet"
fi

# ── Amalgame.Json ──────────────────────────────────────
# Tests pull in src/stdlib/json.am as a 5th-arg extra input so
# the compilation sees both the library and the test sample.
echo ""
echo "── Amalgame.Json ───────────────────────────"
JSON_LIB="src/stdlib/json.am"
run_test "Json: parse null"           "$SAMPLES/stdlib_json.am" "[PASS] parse null"            "" "$JSON_LIB"
run_test "Json: parse true"           "$SAMPLES/stdlib_json.am" "[PASS] parse true"            "" "$JSON_LIB"
run_test "Json: parse false"          "$SAMPLES/stdlib_json.am" "[PASS] parse false"           "" "$JSON_LIB"
run_test "Json: parse int"            "$SAMPLES/stdlib_json.am" "[PASS] parse int"             "" "$JSON_LIB"
run_test "Json: parse negative int"   "$SAMPLES/stdlib_json.am" "[PASS] parse negative int"    "" "$JSON_LIB"
run_test "Json: parse float kind"     "$SAMPLES/stdlib_json.am" "[PASS] parse float kind"      "" "$JSON_LIB"
run_test "Json: parse string"         "$SAMPLES/stdlib_json.am" "[PASS] parse string"          "" "$JSON_LIB"
run_test "Json: parse escapes"        "$SAMPLES/stdlib_json.am" "[PASS] parse escapes"         "" "$JSON_LIB"
run_test "Json: parse empty array"    "$SAMPLES/stdlib_json.am" "[PASS] parse empty array"     "" "$JSON_LIB"
run_test "Json: parse empty object"   "$SAMPLES/stdlib_json.am" "[PASS] parse empty object"    "" "$JSON_LIB"
run_test "Json: parse nested object"  "$SAMPLES/stdlib_json.am" "[PASS] parse nested object"   "" "$JSON_LIB"
run_test "Json: parse LSP request"    "$SAMPLES/stdlib_json.am" "[PASS] parse LSP request"     "" "$JSON_LIB"
run_test "Json: parse usage stats"    "$SAMPLES/stdlib_json.am" "[PASS] parse usage stats"     "" "$JSON_LIB"
run_test "Json: Has key"              "$SAMPLES/stdlib_json.am" "[PASS] Has key"               "" "$JSON_LIB"
run_test "Json: error truncated"      "$SAMPLES/stdlib_json.am" "[PASS] error truncated"       "" "$JSON_LIB"
run_test "Json: error trailing comma" "$SAMPLES/stdlib_json.am" "[PASS] error trailing comma"  "" "$JSON_LIB"
run_test "Json: error non-json"       "$SAMPLES/stdlib_json.am" "[PASS] error non-json"        "" "$JSON_LIB"
run_test "Json: encode null"          "$SAMPLES/stdlib_json.am" "[PASS] encode null"           "" "$JSON_LIB"
run_test "Json: encode bool"          "$SAMPLES/stdlib_json.am" "[PASS] encode bool"           "" "$JSON_LIB"
run_test "Json: encode int"           "$SAMPLES/stdlib_json.am" "[PASS] encode int"            "" "$JSON_LIB"
run_test "Json: encode string"        "$SAMPLES/stdlib_json.am" "[PASS] encode string"         "" "$JSON_LIB"
run_test "Json: encode escape"        "$SAMPLES/stdlib_json.am" "[PASS] encode escape"         "" "$JSON_LIB"
run_test "Json: round-trip nested"    "$SAMPLES/stdlib_json.am" "[PASS] round-trip nested"     "" "$JSON_LIB"
run_test "Json: escape direct"        "$SAMPLES/stdlib_json.am" "[PASS] escape direct"         "" "$JSON_LIB"

# ── Amalgame.Random ────────────────────────────────────
# Same extra-input pattern as Json — pulls in src/stdlib/random.am
# alongside the test sample.
echo ""
echo "── Amalgame.Random ─────────────────────────"
RND_LIB="src/stdlib/random.am"
run_test "Random: seeded reproducible"  "$SAMPLES/stdlib_random.am" "[PASS] seeded reproducible"  "" "$RND_LIB"
run_test "Random: different seeds"      "$SAMPLES/stdlib_random.am" "[PASS] different seeds differ" "" "$RND_LIB"
run_test "Random: NextUInt32 range"     "$SAMPLES/stdlib_random.am" "[PASS] NextUInt32 in range"  "" "$RND_LIB"
run_test "Random: NextInt varies"       "$SAMPLES/stdlib_random.am" "[PASS] NextInt varies"       "" "$RND_LIB"
run_test "Random: IntRange in bounds"   "$SAMPLES/stdlib_random.am" "[PASS] IntRange in bounds"   "" "$RND_LIB"
run_test "Random: IntRange degenerate"  "$SAMPLES/stdlib_random.am" "[PASS] IntRange degenerate"  "" "$RND_LIB"
run_test "Random: Float in [0,1)"       "$SAMPLES/stdlib_random.am" "[PASS] Float in [0,1)"       "" "$RND_LIB"
run_test "Random: Bool varies"          "$SAMPLES/stdlib_random.am" "[PASS] Bool varies"          "" "$RND_LIB"
run_test "Random: Bytes count"          "$SAMPLES/stdlib_random.am" "[PASS] Bytes count"          "" "$RND_LIB"
run_test "Random: Bytes in [0,255]"     "$SAMPLES/stdlib_random.am" "[PASS] Bytes in [0,255]"     "" "$RND_LIB"
run_test "Random: Bytes zero"           "$SAMPLES/stdlib_random.am" "[PASS] Bytes zero"           "" "$RND_LIB"
run_test "Random: SystemBytes count"    "$SAMPLES/stdlib_random.am" "[PASS] SystemBytes count"    "" "$RND_LIB"
run_test "Random: SystemBytes range"    "$SAMPLES/stdlib_random.am" "[PASS] SystemBytes in [0,255]" "" "$RND_LIB"
run_test "Random: FromSystem usable"    "$SAMPLES/stdlib_random.am" "[PASS] FromSystem usable"    "" "$RND_LIB"

# ── Amalgame.Encoding ──────────────────────────────────
# Same extra-input pattern as Json/Random — pulls in
# src/stdlib/encoding.am alongside the test sample.
echo ""
echo "── Amalgame.Encoding ───────────────────────"
ENC_LIB="src/stdlib/encoding.am"
run_test "Encoding: b64 encode hello"       "$SAMPLES/stdlib_encoding.am" "[PASS] b64 encode hello"          "" "$ENC_LIB"
run_test "Encoding: b64 encode empty"       "$SAMPLES/stdlib_encoding.am" "[PASS] b64 encode empty"          "" "$ENC_LIB"
run_test "Encoding: b64 encode 1 byte"      "$SAMPLES/stdlib_encoding.am" "[PASS] b64 encode 1 byte"         "" "$ENC_LIB"
run_test "Encoding: b64 encode 2 bytes"     "$SAMPLES/stdlib_encoding.am" "[PASS] b64 encode 2 bytes"        "" "$ENC_LIB"
run_test "Encoding: b64 round trip"         "$SAMPLES/stdlib_encoding.am" "[PASS] b64 round trip"            "" "$ENC_LIB"
run_test "Encoding: b64 IsValid"            "$SAMPLES/stdlib_encoding.am" "[PASS] b64 IsValid"               "" "$ENC_LIB"
run_test "Encoding: b64 url-safe alphabet"  "$SAMPLES/stdlib_encoding.am" "[PASS] b64 url-safe alphabet"     "" "$ENC_LIB"
run_test "Encoding: b64 url-safe round"     "$SAMPLES/stdlib_encoding.am" "[PASS] b64 url-safe round trip"   "" "$ENC_LIB"
run_test "Encoding: hex encode lower"       "$SAMPLES/stdlib_encoding.am" "[PASS] hex encode lower"          "" "$ENC_LIB"
run_test "Encoding: hex encode upper"       "$SAMPLES/stdlib_encoding.am" "[PASS] hex encode upper"          "" "$ENC_LIB"
run_test "Encoding: hex decode mixed case"  "$SAMPLES/stdlib_encoding.am" "[PASS] hex decode mixed case"     "" "$ENC_LIB"
run_test "Encoding: hex rejects bad"        "$SAMPLES/stdlib_encoding.am" "[PASS] hex decode rejects bad input" "" "$ENC_LIB"
run_test "Encoding: hex IsValid"            "$SAMPLES/stdlib_encoding.am" "[PASS] hex IsValid"               "" "$ENC_LIB"
run_test "Encoding: url encode space"       "$SAMPLES/stdlib_encoding.am" "[PASS] url encode space"          "" "$ENC_LIB"
run_test "Encoding: url path-safe"          "$SAMPLES/stdlib_encoding.am" "[PASS] url encode path-safe"      "" "$ENC_LIB"
run_test "Encoding: url component"          "$SAMPLES/stdlib_encoding.am" "[PASS] url encode component"      "" "$ENC_LIB"
run_test "Encoding: url unreserved"         "$SAMPLES/stdlib_encoding.am" "[PASS] url unreserved unchanged"  "" "$ENC_LIB"
run_test "Encoding: url decode space"       "$SAMPLES/stdlib_encoding.am" "[PASS] url decode space"          "" "$ENC_LIB"
run_test "Encoding: url decode lower hex"   "$SAMPLES/stdlib_encoding.am" "[PASS] url decode lowercase hex"  "" "$ENC_LIB"
run_test "Encoding: url decode plus"        "$SAMPLES/stdlib_encoding.am" "[PASS] url decode plus literal"   "" "$ENC_LIB"
run_test "Encoding: url round trip"         "$SAMPLES/stdlib_encoding.am" "[PASS] url round trip"            "" "$ENC_LIB"

# ── Amalgame.DateTime ──────────────────────────────────
# Same extra-input pattern — pulls in src/stdlib/datetime.am
# alongside the test sample.
echo ""
echo "── Amalgame.DateTime ───────────────────────"
DT_LIB="src/stdlib/datetime.am"
run_test "DateTime: Now plausible"         "$SAMPLES/stdlib_datetime.am" "[PASS] Now plausible"           "" "$DT_LIB"
run_test "DateTime: FromUnixSeconds"       "$SAMPLES/stdlib_datetime.am" "[PASS] FromUnixSeconds"         "" "$DT_LIB"
run_test "DateTime: iso round trip"        "$SAMPLES/stdlib_datetime.am" "[PASS] iso round trip"          "" "$DT_LIB"
run_test "DateTime: iso fractional"        "$SAMPLES/stdlib_datetime.am" "[PASS] iso fractional"          "" "$DT_LIB"
run_test "DateTime: parse rejects bad"     "$SAMPLES/stdlib_datetime.am" "[PASS] parse rejects bad"       "" "$DT_LIB"
run_test "DateTime: Instant.Add hours"     "$SAMPLES/stdlib_datetime.am" "[PASS] Instant.Add hours"       "" "$DT_LIB"
run_test "DateTime: Instant.Subtract"      "$SAMPLES/stdlib_datetime.am" "[PASS] Instant.Subtract hours"  "" "$DT_LIB"
run_test "DateTime: Instant.Since"         "$SAMPLES/stdlib_datetime.am" "[PASS] Instant.Since"           "" "$DT_LIB"
run_test "DateTime: Instant comparison"    "$SAMPLES/stdlib_datetime.am" "[PASS] Instant comparison"      "" "$DT_LIB"
run_test "DateTime: Duration arithmetic"   "$SAMPLES/stdlib_datetime.am" "[PASS] Duration arithmetic"     "" "$DT_LIB"
run_test "DateTime: Duration Times"        "$SAMPLES/stdlib_datetime.am" "[PASS] Duration Times"          "" "$DT_LIB"
run_test "DateTime: Duration Negate"       "$SAMPLES/stdlib_datetime.am" "[PASS] Duration Negate"         "" "$DT_LIB"
run_test "DateTime: dur fmt zero"          "$SAMPLES/stdlib_datetime.am" "[PASS] dur fmt zero"            "" "$DT_LIB"
run_test "DateTime: dur fmt ns"            "$SAMPLES/stdlib_datetime.am" "[PASS] dur fmt ns"              "" "$DT_LIB"
run_test "DateTime: dur fmt us"            "$SAMPLES/stdlib_datetime.am" "[PASS] dur fmt us"              "" "$DT_LIB"
run_test "DateTime: dur fmt ms"            "$SAMPLES/stdlib_datetime.am" "[PASS] dur fmt ms"              "" "$DT_LIB"
run_test "DateTime: dur fmt s"             "$SAMPLES/stdlib_datetime.am" "[PASS] dur fmt s"               "" "$DT_LIB"
run_test "DateTime: dur fmt min"           "$SAMPLES/stdlib_datetime.am" "[PASS] dur fmt min"             "" "$DT_LIB"
run_test "DateTime: dur fmt hour"          "$SAMPLES/stdlib_datetime.am" "[PASS] dur fmt hour"            "" "$DT_LIB"
run_test "DateTime: Stopwatch elapsed"     "$SAMPLES/stdlib_datetime.am" "[PASS] Stopwatch elapsed"       "" "$DT_LIB"
run_test "DateTime: Stopwatch reset"       "$SAMPLES/stdlib_datetime.am" "[PASS] Stopwatch reset"         "" "$DT_LIB"

# ── Amalgame.Crypto ───────────────────────────────────
# Same extra-input pattern — pulls in src/stdlib/crypto.am
# alongside the test sample. Vectors come from FIPS 180-4
# (SHA-256) and RFC 4231 (HMAC-SHA-256 cases 1 & 2).
echo ""
echo "── Amalgame.Crypto ─────────────────────────"
CR_LIB="src/stdlib/crypto.am"
run_test "Crypto: sha256 'abc'"            "$SAMPLES/stdlib_crypto.am" "[PASS] sha256 'abc'"            "" "$CR_LIB"
run_test "Crypto: sha256 empty"            "$SAMPLES/stdlib_crypto.am" "[PASS] sha256 empty"            "" "$CR_LIB"
run_test "Crypto: sha256 bytes length"     "$SAMPLES/stdlib_crypto.am" "[PASS] sha256 bytes length"     "" "$CR_LIB"
run_test "Crypto: sha256 bytes hex"        "$SAMPLES/stdlib_crypto.am" "[PASS] sha256 bytes hex"        "" "$CR_LIB"
run_test "Crypto: sha256 multi-block"      "$SAMPLES/stdlib_crypto.am" "[PASS] sha256 100a"             "" "$CR_LIB"
run_test "Crypto: hmac rfc4231 case 1"     "$SAMPLES/stdlib_crypto.am" "[PASS] hmac rfc4231 case 1"     "" "$CR_LIB"
run_test "Crypto: hmac rfc4231 case 2"     "$SAMPLES/stdlib_crypto.am" "[PASS] hmac rfc4231 case 2"     "" "$CR_LIB"
run_test "Crypto: hmac bytes length"       "$SAMPLES/stdlib_crypto.am" "[PASS] hmac bytes length"       "" "$CR_LIB"
run_test "Crypto: hmac bytes path"         "$SAMPLES/stdlib_crypto.am" "[PASS] hmac bytes path"         "" "$CR_LIB"

# ── Amalgame.Path ─────────────────────────────────────
# Same extra-input pattern — pulls in src/stdlib/path.am alongside
# the test sample. Covers the existing wrappers (Combine /
# Filename / Directory / Extension) plus the v1 additions
# (Stem / IsAbsolute / Normalize / Sep).
echo ""
echo "── Amalgame.Path ───────────────────────────"
PATH_LIB="src/stdlib/path.am"
run_test "Path: combine"             "$SAMPLES/stdlib_path.am" "[PASS] combined"            "" "$PATH_LIB"
run_test "Path: combine trailing"    "$SAMPLES/stdlib_path.am" "[PASS] combined trailing"   "" "$PATH_LIB"
run_test "Path: directory"           "$SAMPLES/stdlib_path.am" "[PASS] directory"           "" "$PATH_LIB"
run_test "Path: directory no-slash"  "$SAMPLES/stdlib_path.am" "[PASS] directory no-slash"  "" "$PATH_LIB"
run_test "Path: filename"            "$SAMPLES/stdlib_path.am" "[PASS] filename"            "" "$PATH_LIB"
run_test "Path: extension"           "$SAMPLES/stdlib_path.am" "[PASS] extension"           "" "$PATH_LIB"
run_test "Path: extension multi"     "$SAMPLES/stdlib_path.am" "[PASS] extension multi"     "" "$PATH_LIB"
run_test "Path: extension none"      "$SAMPLES/stdlib_path.am" "[PASS] extension none"      "" "$PATH_LIB"
run_test "Path: stem"                "$SAMPLES/stdlib_path.am" "[PASS] stem"                "" "$PATH_LIB"
run_test "Path: stem dotfile"        "$SAMPLES/stdlib_path.am" "[PASS] stem dotfile"        "" "$PATH_LIB"
run_test "Path: isabs posix"         "$SAMPLES/stdlib_path.am" "[PASS] isabs posix"         "" "$PATH_LIB"
run_test "Path: isabs drive"         "$SAMPLES/stdlib_path.am" "[PASS] isabs drive"         "" "$PATH_LIB"
run_test "Path: isabs relative"      "$SAMPLES/stdlib_path.am" "[PASS] isabs relative"      "" "$PATH_LIB"
run_test "Path: isabs empty"         "$SAMPLES/stdlib_path.am" "[PASS] isabs empty"         "" "$PATH_LIB"
run_test "Path: normalize relative"  "$SAMPLES/stdlib_path.am" "[PASS] normalize relative"  "" "$PATH_LIB"
run_test "Path: normalize dot"       "$SAMPLES/stdlib_path.am" "[PASS] normalize dot"       "" "$PATH_LIB"
run_test "Path: normalize absolute"  "$SAMPLES/stdlib_path.am" "[PASS] normalize absolute"  "" "$PATH_LIB"
run_test "Path: normalize empty"     "$SAMPLES/stdlib_path.am" "[PASS] normalize empty"     "" "$PATH_LIB"
run_test "Path: normalize root"      "$SAMPLES/stdlib_path.am" "[PASS] normalize root"      "" "$PATH_LIB"
run_test "Path: normalize parent"    "$SAMPLES/stdlib_path.am" "[PASS] normalize parent"    "" "$PATH_LIB"
run_test "Path: sep length"          "$SAMPLES/stdlib_path.am" "[PASS] sep length"          "" "$PATH_LIB"

# ── Amalgame.Logging ──────────────────────────────────
# Configuration getters/setters + level-filter via the file sink.
# Direct stderr capture would mean dropping the runner's color/log
# noise filter; the file sink lets us assert on a clean byte stream.
echo ""
echo "── Amalgame.Logging ────────────────────────"
LOG_LIB="src/stdlib/logging.am"
run_test "Log: level debug"               "$SAMPLES/stdlib_logging.am" "[PASS] level debug"               "" "$LOG_LIB"
run_test "Log: level info"                "$SAMPLES/stdlib_logging.am" "[PASS] level info"                "" "$LOG_LIB"
run_test "Log: level warn"                "$SAMPLES/stdlib_logging.am" "[PASS] level warn"                "" "$LOG_LIB"
run_test "Log: level error"               "$SAMPLES/stdlib_logging.am" "[PASS] level error"               "" "$LOG_LIB"
run_test "Log: level case-insensitive"    "$SAMPLES/stdlib_logging.am" "[PASS] level case-insensitive"    "" "$LOG_LIB"
run_test "Log: level unknown→info"        "$SAMPLES/stdlib_logging.am" "[PASS] level unknown falls to info" "" "$LOG_LIB"
run_test "Log: file set"                  "$SAMPLES/stdlib_logging.am" "[PASS] file set"                  "" "$LOG_LIB"
run_test "Log: file unset"                "$SAMPLES/stdlib_logging.am" "[PASS] file unset"                "" "$LOG_LIB"
run_test "Log: level filter via file"     "$SAMPLES/stdlib_logging.am" "[PASS] level filter via file"     "" "$LOG_LIB"
run_test "Log: file labels"               "$SAMPLES/stdlib_logging.am" "[PASS] file labels"               "" "$LOG_LIB"

# ── Amalgame.Service ──────────────────────────────────
# Daemon primitives. We can't deliver SIGTERM to the test runner's
# own process (it would kill the runner), so the test uses
# Service.RequestStop() — same flag, programmatic path.
echo ""
echo "── Amalgame.Service ────────────────────────"
SVC_LIB="src/stdlib/service.am"
run_test "Service: not stopping at start"   "$SAMPLES/stdlib_service.am" "[PASS] not stopping at start"          "" "$SVC_LIB"
run_test "Service: install idempotent"      "$SAMPLES/stdlib_service.am" "[PASS] install idempotent"             "" "$SVC_LIB"
run_test "Service: should-stop after req"   "$SAMPLES/stdlib_service.am" "[PASS] should-stop after request"      "" "$SVC_LIB"
run_test "Service: sleep short-circuits"    "$SAMPLES/stdlib_service.am" "[PASS] sleep short-circuits when stopping" "" "$SVC_LIB"

# ── Amalgame.Database ─────────────────────────────────
# SQLite 3 via the vendored amalgamation. No facade .am — Db is in
# the cgen's isStdlib short-circuit so Db.Open / Db.Exec / etc. lower
# straight to the runtime helpers. Tests open an in-memory db.
echo ""
echo "── Amalgame.Database ───────────────────────"
run_test "Db: open memory"                  "$SAMPLES/stdlib_database.am" "[PASS] open memory"               ""
run_test "Db: create table"                 "$SAMPLES/stdlib_database.am" "[PASS] create table"              ""
run_test "Db: insert"                       "$SAMPLES/stdlib_database.am" "[PASS] insert alice"              ""
run_test "Db: last insert id 1"             "$SAMPLES/stdlib_database.am" "[PASS] last insert id 1"          ""
run_test "Db: last insert id 3"             "$SAMPLES/stdlib_database.am" "[PASS] last insert id 3"          ""
run_test "Db: changes counter"              "$SAMPLES/stdlib_database.am" "[PASS] changes 2"                 ""
run_test "Db: query rows"                   "$SAMPLES/stdlib_database.am" "[PASS] query 3 rows"              ""
run_test "Db: column text"                  "$SAMPLES/stdlib_database.am" "[PASS] alice name"                ""
run_test "Db: update reflected"             "$SAMPLES/stdlib_database.am" "[PASS] alice age post-update"     ""
run_test "Db: aggregate count"              "$SAMPLES/stdlib_database.am" "[PASS] aggregate count 2"         ""
run_test "Db: error reported"               "$SAMPLES/stdlib_database.am" "[PASS] error reported"            ""
run_test "Db: delete + verify"              "$SAMPLES/stdlib_database.am" "[PASS] delete leaves 2"           ""
run_test "Db: close"                        "$SAMPLES/stdlib_database.am" "[PASS] closed"                    ""

# ── Amalgame.Database.NoSQL.Redis ─────────────────────
# RESP2 client over raw TCP. Gated on a TCP reachability probe so
# the test suite runs cleanly without Redis installed — every case
# becomes a SKIP rather than a failure. To exercise the round-trip,
# start a Redis-compatible server on 127.0.0.1:6379 (e.g.
# `docker run --rm -p 6379:6379 redis:7` or `redis-server &`).
echo ""
echo "── Amalgame.Database.NoSQL.Redis ───────────"
REDIS_AVAILABLE=0
if (echo > /dev/tcp/127.0.0.1/6379) 2>/dev/null; then
    REDIS_AVAILABLE=1
fi

if [ "$REDIS_AVAILABLE" = "1" ]; then
    run_test "Redis: open 6379"             "$SAMPLES/stdlib_redis.am" "[PASS] open 6379"             ""
    run_test "Redis: ping"                  "$SAMPLES/stdlib_redis.am" "[PASS] ping"                  ""
    run_test "Redis: set greeting"          "$SAMPLES/stdlib_redis.am" "[PASS] set greeting"          ""
    run_test "Redis: get greeting"          "$SAMPLES/stdlib_redis.am" "[PASS] get greeting"          ""
    run_test "Redis: get missing empty"     "$SAMPLES/stdlib_redis.am" "[PASS] get missing is empty"  ""
    run_test "Redis: exists hit"            "$SAMPLES/stdlib_redis.am" "[PASS] exists hit"            ""
    run_test "Redis: exists miss"           "$SAMPLES/stdlib_redis.am" "[PASS] exists miss"           ""
    run_test "Redis: incr 1,2"              "$SAMPLES/stdlib_redis.am" "[PASS] incr 1,2"              ""
    run_test "Redis: decr to 1"             "$SAMPLES/stdlib_redis.am" "[PASS] decr to 1"             ""
    run_test "Redis: expire 60s"            "$SAMPLES/stdlib_redis.am" "[PASS] expire 60s"            ""
    run_test "Redis: expire on missing"     "$SAMPLES/stdlib_redis.am" "[PASS] expire on missing"     ""
    run_test "Redis: del 1"                 "$SAMPLES/stdlib_redis.am" "[PASS] del 1"                 ""
    run_test "Redis: gone after del"        "$SAMPLES/stdlib_redis.am" "[PASS] gone after del"        ""
    run_test "Redis: close"                 "$SAMPLES/stdlib_redis.am" "[PASS] closed"                ""
else
    run_skip "Redis: open 6379"             "no server on 127.0.0.1:6379"
    run_skip "Redis: ping"                  "no server on 127.0.0.1:6379"
    run_skip "Redis: set greeting"          "no server on 127.0.0.1:6379"
    run_skip "Redis: get greeting"          "no server on 127.0.0.1:6379"
    run_skip "Redis: get missing empty"     "no server on 127.0.0.1:6379"
    run_skip "Redis: exists hit"            "no server on 127.0.0.1:6379"
    run_skip "Redis: exists miss"           "no server on 127.0.0.1:6379"
    run_skip "Redis: incr 1,2"              "no server on 127.0.0.1:6379"
    run_skip "Redis: decr to 1"             "no server on 127.0.0.1:6379"
    run_skip "Redis: expire 60s"            "no server on 127.0.0.1:6379"
    run_skip "Redis: expire on missing"     "no server on 127.0.0.1:6379"
    run_skip "Redis: del 1"                 "no server on 127.0.0.1:6379"
    run_skip "Redis: gone after del"        "no server on 127.0.0.1:6379"
    run_skip "Redis: close"                 "no server on 127.0.0.1:6379"
fi

# ── Summary ────────────────────────────────────────────
echo ""
echo "────────────────────────────────────────────"
echo -e "  ${GREEN}PASS: $PASS${NC}  |  ${RED}FAIL: $FAIL${NC}  |  ${YELLOW}SKIP: $SKIP${NC}"
echo "────────────────────────────────────────────"
echo ""

[ $FAIL -eq 0 ] && exit 0 || exit 1
