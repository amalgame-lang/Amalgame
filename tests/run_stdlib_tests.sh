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

# Note on Database / Messaging tests:
#
# Optional backends (Database.SQLite, Database.NoSQL.Redis,
# Messaging.MQTT) live in their own external packages since v0.5.
# Their tests live in those packages' repos and run on their
# package CI — NOT here. The main repo's suite tests only the
# compiler + the always-present core stdlib + the package-
# manager pipeline.
#
# To exercise the external packages' tests:
#   cd ../amalgame-database-sqlite       && ./tests/run_tests.sh "$AMC"
#   cd ../amalgame-database-nosql-redis  && ./tests/run_tests.sh "$AMC"
#   cd ../amalgame-messaging-mqtt        && ./tests/run_tests.sh "$AMC"

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
    # SQLite stopped being a default link target with the v0.5
    # extraction — Database.SQLite is now an opt-in external
    # package. Tests that need it go through `run_db_test` which
    # adds the .o + sets cwd to $SQLITE_PROJ.
    gcc -O2 -Iruntime "$c_file" -lgc -lm -lcurl -lz -ldl -lpthread -o "$out_base" 2>/dev/null

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

# ── Amalgame.Formats.Toml ──────────────────────────────
# TOML 1.0 subset for v0.5 package-manager manifests. Same facade
# pattern as Json: the .am stdlib file passes as a 5th-arg extra
# input. Stdlib must be passed BEFORE the test file so the cgen
# forward-declares Toml.* methods cleanly (gotcha — emission order
# matters for static-class methods across compile units).
echo ""
echo "── Amalgame.Formats.Toml ───────────────────"
TOML_LIB="src/stdlib/toml.am"
run_test "Toml: parse empty"          "$SAMPLES/stdlib_toml.am" "[PASS] parse empty"          "" "$TOML_LIB"
run_test "Toml: kv string"            "$SAMPLES/stdlib_toml.am" "[PASS] kv string"            "" "$TOML_LIB"
run_test "Toml: kv int"               "$SAMPLES/stdlib_toml.am" "[PASS] kv int"               "" "$TOML_LIB"
run_test "Toml: kv neg int"           "$SAMPLES/stdlib_toml.am" "[PASS] kv neg int"           "" "$TOML_LIB"
run_test "Toml: kv bool true"         "$SAMPLES/stdlib_toml.am" "[PASS] kv bool true"         "" "$TOML_LIB"
run_test "Toml: kv bool false"        "$SAMPLES/stdlib_toml.am" "[PASS] kv bool false"        "" "$TOML_LIB"
run_test "Toml: string escapes"       "$SAMPLES/stdlib_toml.am" "[PASS] string escapes"       "" "$TOML_LIB"
run_test "Toml: literal string"       "$SAMPLES/stdlib_toml.am" "[PASS] literal string"       "" "$TOML_LIB"
run_test "Toml: comments tolerated"   "$SAMPLES/stdlib_toml.am" "[PASS] comments tolerated"   "" "$TOML_LIB"
run_test "Toml: array strings"        "$SAMPLES/stdlib_toml.am" "[PASS] array strings"        "" "$TOML_LIB"
run_test "Toml: array ints"           "$SAMPLES/stdlib_toml.am" "[PASS] array ints"           "" "$TOML_LIB"
run_test "Toml: table"                "$SAMPLES/stdlib_toml.am" "[PASS] table"                "" "$TOML_LIB"
run_test "Toml: nested table"         "$SAMPLES/stdlib_toml.am" "[PASS] nested table"         "" "$TOML_LIB"
run_test "Toml: inline table"         "$SAMPLES/stdlib_toml.am" "[PASS] inline table"         "" "$TOML_LIB"
run_test "Toml: manifest package.name" "$SAMPLES/stdlib_toml.am" "[PASS] manifest package.name" "" "$TOML_LIB"
run_test "Toml: manifest stdlib.class" "$SAMPLES/stdlib_toml.am" "[PASS] manifest stdlib.class" "" "$TOML_LIB"
run_test "Toml: manifest dep tag"     "$SAMPLES/stdlib_toml.am" "[PASS] manifest dep tag"     "" "$TOML_LIB"
run_test "Toml: missing key is null"  "$SAMPLES/stdlib_toml.am" "[PASS] missing key chain is null" "" "$TOML_LIB"
run_test "Toml: has present+absent"   "$SAMPLES/stdlib_toml.am" "[PASS] has present + absent" "" "$TOML_LIB"
run_test "Toml: aot count"            "$SAMPLES/stdlib_toml.am" "[PASS] aot count"            "" "$TOML_LIB"
run_test "Toml: aot entry 0"          "$SAMPLES/stdlib_toml.am" "[PASS] aot entry 0"          "" "$TOML_LIB"
run_test "Toml: aot entry 1"          "$SAMPLES/stdlib_toml.am" "[PASS] aot entry 1"          "" "$TOML_LIB"
run_test "Toml: roundtrip simple"     "$SAMPLES/stdlib_toml.am" "[PASS] roundtrip simple"     "" "$TOML_LIB"

# ── PackageRegistry (v0.5 PR 3b) ───────────────────────
# Loads amalgame.lock + cached package manifests, exposes them as
# a typed registry. Test fixture lives under tests/fixtures/pm/
# (no network, no git — just files on disk).
echo ""
echo "── PackageRegistry ─────────────────────────"
PR_EXTRA="src/stdlib/toml.am src/package_registry.am"
run_test "PR: one package"            "$SAMPLES/stdlib_package_registry.am" "[PASS] one package"        "" "$PR_EXTRA"
run_test "PR: name"                   "$SAMPLES/stdlib_package_registry.am" "[PASS] name"               "" "$PR_EXTRA"
run_test "PR: class name"             "$SAMPLES/stdlib_package_registry.am" "[PASS] class name"         "" "$PR_EXTRA"
run_test "PR: namespace"              "$SAMPLES/stdlib_package_registry.am" "[PASS] namespace"          "" "$PR_EXTRA"
run_test "PR: header path"            "$SAMPLES/stdlib_package_registry.am" "[PASS] header path"        "" "$PR_EXTRA"
run_test "PR: func count"             "$SAMPLES/stdlib_package_registry.am" "[PASS] func count"         "" "$PR_EXTRA"
run_test "PR: Init returns pointer"   "$SAMPLES/stdlib_package_registry.am" "[PASS] Init returns pointer"  "" "$PR_EXTRA"
run_test "PR: Tick returns i64"       "$SAMPLES/stdlib_package_registry.am" "[PASS] Tick returns i64"      "" "$PR_EXTRA"
run_test "PR: IsOk returns bool"      "$SAMPLES/stdlib_package_registry.am" "[PASS] IsOk returns bool"     "" "$PR_EXTRA"
run_test "PR: Close returns void"     "$SAMPLES/stdlib_package_registry.am" "[PASS] Close returns void"    "" "$PR_EXTRA"
run_test "PR: ClassNames aggregate"   "$SAMPLES/stdlib_package_registry.am" "[PASS] ClassNames aggregate"  "" "$PR_EXTRA"
run_test "PR: Headers aggregate"      "$SAMPLES/stdlib_package_registry.am" "[PASS] Headers aggregate"     "" "$PR_EXTRA"
run_test "PR: strip star"             "$SAMPLES/stdlib_package_registry.am" "[PASS] strip star"            "" "$PR_EXTRA"
run_test "PR: passthrough primitive"  "$SAMPLES/stdlib_package_registry.am" "[PASS] passthrough primitive" "" "$PR_EXTRA"
run_test "PR: missing lock empty"     "$SAMPLES/stdlib_package_registry.am" "[PASS] missing lock empty"    "" "$PR_EXTRA"
# v0.5.3 — manifest schema growth for C++-bearing packages.
run_test "PR: cflags parsed"          "$SAMPLES/stdlib_package_registry.am" "[PASS] cflags parsed"         "" "$PR_EXTRA"
run_test "PR: cxxflags parsed"        "$SAMPLES/stdlib_package_registry.am" "[PASS] cxxflags parsed"       "" "$PR_EXTRA"
run_test "PR: libs count"             "$SAMPLES/stdlib_package_registry.am" "[PASS] libs count"            "" "$PR_EXTRA"
run_test "PR: libs[0]"                "$SAMPLES/stdlib_package_registry.am" "[PASS] libs[0]"               "" "$PR_EXTRA"
run_test "PR: libs[1]"                "$SAMPLES/stdlib_package_registry.am" "[PASS] libs[1]"               "" "$PR_EXTRA"
run_test "PR: supports schema v1"     "$SAMPLES/stdlib_package_registry.am" "[PASS] supports schema v1"    "" "$PR_EXTRA"
run_test "PR: detects .cpp"           "$SAMPLES/stdlib_package_registry.am" "[PASS] detects .cpp"          "" "$PR_EXTRA"
run_test "PR: detects .cc"            "$SAMPLES/stdlib_package_registry.am" "[PASS] detects .cc"           "" "$PR_EXTRA"
run_test "PR: detects .cxx"           "$SAMPLES/stdlib_package_registry.am" "[PASS] detects .cxx"          "" "$PR_EXTRA"
run_test "PR: .c not cxx"             "$SAMPLES/stdlib_package_registry.am" "[PASS] .c not cxx"            "" "$PR_EXTRA"
run_test "PR: no cxx sources"         "$SAMPLES/stdlib_package_registry.am" "[PASS] no cxx sources"        "" "$PR_EXTRA"
run_test "PR: CollectLibs"            "$SAMPLES/stdlib_package_registry.am" "[PASS] CollectLibs"           "" "$PR_EXTRA"
# v0.5.4 — precompile flag + PkgDir + platform tag + Calibration.
run_test "PR: precompile parsed"      "$SAMPLES/stdlib_package_registry.am" "[PASS] precompile parsed"     "" "$PR_EXTRA"
run_test "PR: pkgdir populated"       "$SAMPLES/stdlib_package_registry.am" "[PASS] pkgdir populated"      "" "$PR_EXTRA"
run_test "PR: facade empty default"   "$SAMPLES/stdlib_package_registry.am" "[PASS] facade empty for fake-pkg" "" "$PR_EXTRA"
run_test "PR: platform tag shape"     "$SAMPLES/stdlib_package_registry.am" "[PASS] platform tag shape"    "" "$PR_EXTRA"
run_test "PR: precompile cache path"  "$SAMPLES/stdlib_package_registry.am" "[PASS] precompile cache path" "" "$PR_EXTRA"
run_test "PR: empty calib no eta"     "$SAMPLES/stdlib_package_registry.am" "[PASS] empty calibration no eta" "" "$PR_EXTRA"
run_test "PR: sample counts"          "$SAMPLES/stdlib_package_registry.am" "[PASS] sample counts"         "" "$PR_EXTRA"
run_test "PR: cxx eta"                "$SAMPLES/stdlib_package_registry.am" "[PASS] cxx eta"               "" "$PR_EXTRA"
run_test "PR: c eta"                  "$SAMPLES/stdlib_package_registry.am" "[PASS] c eta"                 "" "$PR_EXTRA"
run_test "PR: unknown lang no eta"    "$SAMPLES/stdlib_package_registry.am" "[PASS] unknown lang no eta"   "" "$PR_EXTRA"
# v0.6.0 — version operators (>=, >, <=, <, =, ^, ~, bare).
run_test "PR: ge satisfied"           "$SAMPLES/stdlib_package_registry.am" "[PASS] ge satisfied"          "" "$PR_EXTRA"
run_test "PR: ge unsatisfied"         "$SAMPLES/stdlib_package_registry.am" "[PASS] ge unsatisfied"        "" "$PR_EXTRA"
run_test "PR: gt strict"              "$SAMPLES/stdlib_package_registry.am" "[PASS] gt strict"             "" "$PR_EXTRA"
run_test "PR: gt strict equal"        "$SAMPLES/stdlib_package_registry.am" "[PASS] gt strict equal"       "" "$PR_EXTRA"
run_test "PR: le equal"               "$SAMPLES/stdlib_package_registry.am" "[PASS] le equal"              "" "$PR_EXTRA"
run_test "PR: lt strict"              "$SAMPLES/stdlib_package_registry.am" "[PASS] lt strict"             "" "$PR_EXTRA"
run_test "PR: lt strict equal"        "$SAMPLES/stdlib_package_registry.am" "[PASS] lt strict equal"       "" "$PR_EXTRA"
run_test "PR: eq match"               "$SAMPLES/stdlib_package_registry.am" "[PASS] eq match"              "" "$PR_EXTRA"
run_test "PR: eq miss"                "$SAMPLES/stdlib_package_registry.am" "[PASS] eq miss"               "" "$PR_EXTRA"
run_test "PR: caret 1.x within"       "$SAMPLES/stdlib_package_registry.am" "[PASS] caret 1.x within"      "" "$PR_EXTRA"
run_test "PR: caret 1.x reject major" "$SAMPLES/stdlib_package_registry.am" "[PASS] caret 1.x reject major" "" "$PR_EXTRA"
run_test "PR: caret 0.x within"       "$SAMPLES/stdlib_package_registry.am" "[PASS] caret 0.x within"      "" "$PR_EXTRA"
run_test "PR: caret 0.x reject minor" "$SAMPLES/stdlib_package_registry.am" "[PASS] caret 0.x reject minor" "" "$PR_EXTRA"
run_test "PR: caret 0.0.x exact"      "$SAMPLES/stdlib_package_registry.am" "[PASS] caret 0.0.x exact"     "" "$PR_EXTRA"
run_test "PR: caret 0.0.x reject patch" "$SAMPLES/stdlib_package_registry.am" "[PASS] caret 0.0.x reject patch" "" "$PR_EXTRA"
run_test "PR: tilde within"           "$SAMPLES/stdlib_package_registry.am" "[PASS] tilde within"          "" "$PR_EXTRA"
run_test "PR: tilde reject minor"     "$SAMPLES/stdlib_package_registry.am" "[PASS] tilde reject minor"    "" "$PR_EXTRA"
run_test "PR: bare ge"                "$SAMPLES/stdlib_package_registry.am" "[PASS] bare ge"               "" "$PR_EXTRA"

# ── PackageManager e2e (full pipeline) ────────────────
# Validates Toml → PackageRegistry → Resolver → CGen end-to-end:
# compiles a user program that imports a class only known to the
# compiler via the fixture amalgame.lock + cached manifest, and
# asserts the generated C has the right symbols + types + include.
# AMALGAME_PACKAGES_DIR env var overrides the default cache root
# so the test doesn't poke at ~/.amalgame/packages/.
echo ""
echo "── PackageManager e2e ──────────────────────"
test_pm_e2e() {
    local TMPDIR=$(mktemp -d -t pm-e2e-XXXXXX)
    cat > "$TMPDIR/amalgame.lock" <<EOF
[[package]]
name = "fake-pkg"
git  = "example.com/fake/fake-pkg"
tag  = "v0.1.0"
rev  = "deadbeefcafebabe0000000000000000000000ab"
EOF
    cat > "$TMPDIR/user.am" <<'USRAM'
public class Program {
    public static void Main() {
        let r = FakePkg.Init()
        let t: int = FakePkg.Tick(r)
        let ok: bool = FakePkg.IsOk(r)
        var okStr: string = "false"
        if (ok) { okStr = "true" }
        Console.WriteLine("e2e tick=" + String_FromInt(t) + " ok=" + okStr)
        FakePkg.Close(r)
    }
}
USRAM
    local AMC_ABS="$(realpath ./amc)"
    local CACHE_ABS="$(realpath tests/fixtures/pm/cache)"
    local RUNTIME_ABS="$(realpath runtime)"
    (cd "$TMPDIR" && AMALGAME_PACKAGES_DIR="$CACHE_ABS" "$AMC_ABS" -o out user.am) > "$TMPDIR/amc.log" 2>&1
    local amc_exit=$?

    check_e2e() {
        local name="$1"; local pattern="$2"
        printf "  %-38s" "$name"
        if grep -qE "$pattern" "$TMPDIR/out.c"; then
            echo -e "${GREEN}PASS${NC}"; PASS=$((PASS+1))
        else
            echo -e "${RED}FAIL${NC}"; FAIL=$((FAIL+1))
        fi
    }

    if [ $amc_exit -ne 0 ]; then
        printf "  %-38s${RED}FAIL${NC} (amc exited %d)\n" "PM e2e: amc compile" $amc_exit
        FAIL=$((FAIL+1))
        cat "$TMPDIR/amc.log" | head -5 | sed 's/^/    /'
        rm -rf "$TMPDIR"; return
    fi
    printf "  %-38s${GREEN}PASS${NC}\n" "PM e2e: amc compile"; PASS=$((PASS+1))

    check_e2e "PM e2e: header included"          "#include.*fake_pkg.h"
    # v0.5+: C symbols are namespace-mangled. The fixture manifest
    # declares namespace="Amalgame.Fake.FakePkg" so cgen emits
    # Amalgame_Fake_FakePkg_<method> at call sites.
    check_e2e "PM e2e: FakePkg.Init typed"       "AmalgameFakePkg\\* .*= Amalgame_Fake_FakePkg_Init"
    check_e2e "PM e2e: FakePkg.Tick i64"         "i64 .*= Amalgame_Fake_FakePkg_Tick"
    check_e2e "PM e2e: FakePkg.IsOk bool"        "code_bool .*= Amalgame_Fake_FakePkg_IsOk"
    check_e2e "PM e2e: FakePkg.Close called"     "Amalgame_Fake_FakePkg_Close\\("

    # Full round-trip: gcc + run.
    gcc -O2 -I"$RUNTIME_ABS" "$TMPDIR/out.c" -lgc -lm -lcurl -lz -o "$TMPDIR/out" 2>"$TMPDIR/gcc.log"
    printf "  %-38s" "PM e2e: gcc + run"
    if [ -x "$TMPDIR/out" ]; then
        local run_out=$("$TMPDIR/out" 2>&1)
        if echo "$run_out" | grep -q "e2e tick=1 ok=true"; then
            echo -e "${GREEN}PASS${NC}"; PASS=$((PASS+1))
        else
            echo -e "${RED}FAIL${NC} (run output: $run_out)"; FAIL=$((FAIL+1))
        fi
    else
        echo -e "${RED}FAIL${NC} (gcc failed)"; FAIL=$((FAIL+1))
        head -5 "$TMPDIR/gcc.log" | sed 's/^/    /'
    fi

    rm -rf "$TMPDIR"
}
test_pm_e2e

# ── PackageManager facade e2e (project F follow-up) ───
# Exercises `[stdlib].facade = "facade.am"` end-to-end:
#   1. Copy the read-only fixture cache into a tmpdir.
#   2. Pre-build the per-package archive at
#      <pkgDir>/build/<plat>/libamalgame-pkg-FakeFacade.a using
#      the same three-step pipeline AddCommand.PrecompileFacade
#      uses (amc --lib + gcc -c + ar rcs).
#   3. Compile user.am with the registry pointing at the tmp cache.
#      AmalgameCompiler.Run auto-promotes the facade to --external,
#      so the cgen emits forward decls only (no facade body in out.c).
#   4. gcc + link user.c against the per-package archive.
#   5. Run the binary, assert the expected stdout.
#
# Validates both halves of the project F follow-up: the precompile
# pipeline that ships the archive, and the auto-consume pipeline
# that picks it up at user-build time.
echo ""
echo "── PackageManager facade e2e ───────────────"
test_pm_facade_e2e() {
    local TMPDIR=$(mktemp -d -t pm-facade-e2e-XXXXXX)
    cp -r tests/fixtures/pm-facade/cache "$TMPDIR/cache"
    cp tests/fixtures/pm-facade/amalgame.lock "$TMPDIR/amalgame.lock"

    local PKG_DIR="$TMPDIR/cache/example.com/fake/fake-facade/v0.1.0_deadbeef"
    # Mirror PackageRegistry.PlatformTag() — keep in sync.
    local PLAT
    case "$(uname -s)" in
        Linux*)               PLAT="linux-$(uname -m)" ;;
        Darwin*)              PLAT="macos-$(uname -m)" ;;
        MINGW*|MSYS*|CYGWIN*) PLAT="windows-$(uname -m)" ;;
        *)                    PLAT="unknown-$(uname -m)" ;;
    esac
    # Normalise amd64 / aarch64 to the same shortform PlatformTag uses.
    PLAT="${PLAT/amd64/x86_64}"
    PLAT="${PLAT/aarch64/arm64}"
    local BUILD_DIR="$PKG_DIR/build/$PLAT"
    mkdir -p "$BUILD_DIR"

    local AMC_ABS="$(realpath ./amc)"
    local RUNTIME_ABS="$(realpath runtime)"

    # ── Pre-build the per-package archive ─────────────
    local BASE="$BUILD_DIR/FakeFacade-facade"
    local ARCHIVE="$BUILD_DIR/libamalgame-pkg-FakeFacade.a"

    printf "  %-38s" "facade e2e: amc --lib"
    if "$AMC_ABS" --lib --quiet "$PKG_DIR/facade.am" -o "$BASE" > "$TMPDIR/precompile.log" 2>&1; then
        echo -e "${GREEN}PASS${NC}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${NC}"; FAIL=$((FAIL+1))
        head -5 "$TMPDIR/precompile.log" | sed 's/^/    /'
        rm -rf "$TMPDIR"; return
    fi

    printf "  %-38s" "facade e2e: gcc -c"
    if gcc -O2 -I"$RUNTIME_ABS" -w -c "$BASE.c" -o "$BASE.o" > "$TMPDIR/gcc.log" 2>&1; then
        echo -e "${GREEN}PASS${NC}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${NC}"; FAIL=$((FAIL+1))
        head -5 "$TMPDIR/gcc.log" | sed 's/^/    /'
        rm -rf "$TMPDIR"; return
    fi

    printf "  %-38s" "facade e2e: ar rcs"
    if ar rcs "$ARCHIVE" "$BASE.o" 2> "$TMPDIR/ar.log"; then
        echo -e "${GREEN}PASS${NC}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${NC}"; FAIL=$((FAIL+1))
        head -5 "$TMPDIR/ar.log" | sed 's/^/    /'
        rm -rf "$TMPDIR"; return
    fi

    printf "  %-38s" "facade e2e: archive exists"
    if [ -s "$ARCHIVE" ]; then
        echo -e "${GREEN}PASS${NC}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${NC} (missing $ARCHIVE)"; FAIL=$((FAIL+1))
        rm -rf "$TMPDIR"; return
    fi

    # ── User code that imports the facade ─────────────
    cat > "$TMPDIR/user.am" <<'USRAM'
namespace UserApp

public class Program {
    public static void Main() {
        let v: int = FakeFacade.Make(41)
        let w: int = FakeFacade.Twice(v)
        Console.WriteLine("got " + String_FromInt(w))
    }
}
USRAM

    local CACHE_ABS="$TMPDIR/cache"
    (cd "$TMPDIR" && AMALGAME_PACKAGES_DIR="$CACHE_ABS" "$AMC_ABS" -o out user.am --quiet) \
        > "$TMPDIR/usercompile.log" 2>&1
    local amc_exit=$?

    printf "  %-38s" "facade e2e: amc compile"
    if [ $amc_exit -eq 0 ]; then
        echo -e "${GREEN}PASS${NC}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${NC} (amc exited $amc_exit)"; FAIL=$((FAIL+1))
        head -5 "$TMPDIR/usercompile.log" | sed 's/^/    /'
        rm -rf "$TMPDIR"; return
    fi

    # The cgen should emit forward decls only for the facade class,
    # never its method bodies — they live in the archive.
    printf "  %-38s" "facade e2e: forward decl emitted"
    if grep -q "Amalgame_Fake_FakeFacade_FakeFacade_Make(i64 x);" "$TMPDIR/out.c"; then
        echo -e "${GREEN}PASS${NC}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${NC}"; FAIL=$((FAIL+1))
    fi
    printf "  %-38s" "facade e2e: body NOT emitted"
    if ! grep -q "return x \* 2;" "$TMPDIR/out.c"; then
        echo -e "${GREEN}PASS${NC}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${NC} (facade body leaked into out.c)"; FAIL=$((FAIL+1))
    fi

    # Full round-trip: gcc + run with the per-package archive.
    printf "  %-38s" "facade e2e: gcc + run"
    gcc -O2 -I"$RUNTIME_ABS" "$TMPDIR/out.c" "$ARCHIVE" \
        -lgc -lm -lcurl -lz -ldl -lpthread -o "$TMPDIR/out" \
        2> "$TMPDIR/link.log"
    if [ -x "$TMPDIR/out" ]; then
        local run_out=$("$TMPDIR/out" 2>&1)
        if [ "$run_out" = "got 84" ]; then
            echo -e "${GREEN}PASS${NC}"; PASS=$((PASS+1))
        else
            echo -e "${RED}FAIL${NC} (got: $run_out)"; FAIL=$((FAIL+1))
        fi
    else
        echo -e "${RED}FAIL${NC} (gcc link failed)"; FAIL=$((FAIL+1))
        head -5 "$TMPDIR/link.log" | sed 's/^/    /'
    fi

    rm -rf "$TMPDIR"
}
test_pm_facade_e2e

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

# ── Amalgame.Formats.MsgPack ──────────────────────────
# MessagePack 1.0 subset codec via JsonValue.
echo ""
echo "── Amalgame.Formats.MsgPack ───────────────"
MP_LIB="src/stdlib/json.am src/stdlib/msgpack.am"
run_test "MP: encode nil"            "$SAMPLES/stdlib_msgpack.am" "[PASS] encode nil"            "" "$MP_LIB"
run_test "MP: decode nil"            "$SAMPLES/stdlib_msgpack.am" "[PASS] decode nil"            "" "$MP_LIB"
run_test "MP: encode true"           "$SAMPLES/stdlib_msgpack.am" "[PASS] encode true"           "" "$MP_LIB"
run_test "MP: decode true"           "$SAMPLES/stdlib_msgpack.am" "[PASS] decode true"           "" "$MP_LIB"
run_test "MP: encode fixint"         "$SAMPLES/stdlib_msgpack.am" "[PASS] encode fixint"         "" "$MP_LIB"
run_test "MP: decode fixint"         "$SAMPLES/stdlib_msgpack.am" "[PASS] decode fixint"         "" "$MP_LIB"
run_test "MP: roundtrip negfix"      "$SAMPLES/stdlib_msgpack.am" "[PASS] roundtrip negfix"      "" "$MP_LIB"
run_test "MP: roundtrip int16"       "$SAMPLES/stdlib_msgpack.am" "[PASS] roundtrip int16"       "" "$MP_LIB"
run_test "MP: encode fixstr"         "$SAMPLES/stdlib_msgpack.am" "[PASS] encode fixstr"         "" "$MP_LIB"
run_test "MP: decode fixstr"         "$SAMPLES/stdlib_msgpack.am" "[PASS] decode fixstr"         "" "$MP_LIB"
run_test "MP: encode fixarray"       "$SAMPLES/stdlib_msgpack.am" "[PASS] encode fixarray"       "" "$MP_LIB"
run_test "MP: decode fixarray"       "$SAMPLES/stdlib_msgpack.am" "[PASS] decode fixarray"       "" "$MP_LIB"
run_test "MP: decode fixmap shape"   "$SAMPLES/stdlib_msgpack.am" "[PASS] decode fixmap shape"   "" "$MP_LIB"
run_test "MP: decode fixmap values"  "$SAMPLES/stdlib_msgpack.am" "[PASS] decode fixmap values"  "" "$MP_LIB"

# Database / Messaging external-package tests run on the package
# repos' own CI — not here. See the header note at the top of
# this file for the per-package invocation.

# Database.NoSQL.Redis tests live in amalgame-database-nosql-redis,
# Messaging.MQTT tests live in amalgame-messaging-mqtt. See the
# header comment at the top of this file for invocation.

# ── Summary ────────────────────────────────────────────
echo ""
echo "────────────────────────────────────────────"
echo -e "  ${GREEN}PASS: $PASS${NC}  |  ${RED}FAIL: $FAIL${NC}  |  ${YELLOW}SKIP: $SKIP${NC}"
echo "────────────────────────────────────────────"
echo ""

[ $FAIL -eq 0 ] && exit 0 || exit 1
