#!/bin/bash
# ─────────────────────────────────────────────────────
#  Amalgame Transpiler — Test Runner
#  Usage: ./tests/run_tests.sh
# ─────────────────────────────────────────────────────

AMC="./amc"
SAMPLES="./tests/samples"

# Build artifacts go to a temp directory so the source tree stays clean.
# Auto-removed on script exit (success or failure).
BUILD_DIR=$(mktemp -d -t amc-tests-XXXXXX)
trap 'rm -rf "$BUILD_DIR"' EXIT

# Samples the self-hosted ./amc can't yet compile (tracked separately:
# algebraic enum methods, tuple destructure typing, try/catch binder
# scoping, advanced pattern matching, null-safety inference). PRs welcome.
SKIP_SELFHOST="  "
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

    local base=" $(basename "$file") "
    if [ "$AMC" = "./amc" ] && [[ "$SKIP_SELFHOST" == *"$base"* ]]; then
        echo -e "${YELLOW}SKIP${NC} (self-host: pending compiler fix)"
        SKIP=$((SKIP + 1)); return
    fi

    local out_base="$BUILD_DIR/$(basename "${file%.am}")"
    output=$("$AMC" $flags -o "$out_base" "$file" 2>&1)
    amc_exit=$?

    if [ $amc_exit -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (amc exited $amc_exit)"
        echo "$output" | grep -E "error|Error|\[resolver\]|\[typechecker\]" \
            | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1)); return
    fi

    # amc only emits a .c file; gcc it here to produce the test binary.
    local c_file="${out_base}.c"
    if [ ! -f "$c_file" ]; then
        echo -e "${RED}FAIL${NC} (no .c emitted)"
        FAIL=$((FAIL + 1)); return
    fi
    gcc -O2 -Iruntime "$c_file" -lgc -lm -lz -o "$out_base" 2>/dev/null

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

# Multi-file driver. Compile multiple .am files in one amc invocation,
# link, run, and grep for an expected substring. Used by the Bug 1
# regression (v0.8.23): caller in file A reaches a method defined in
# file B — Pass 2a (forwards across all files before any bodies) must
# emit the prototype.
#
#   $1 = test name
#   $2 = expected substring
#   $3..$N = .am files (caller first to exercise the dependency direction)
run_multi_test() {
    local name="$1"
    local expected="$2"
    shift 2
    local files=("$@")

    printf "  %-34s" "$name"

    for f in "${files[@]}"; do
        if [ ! -f "$f" ]; then
            echo -e "${YELLOW}SKIP${NC} (file not found: $f)"
            SKIP=$((SKIP + 1)); return
        fi
    done

    local out_base="$BUILD_DIR/$(basename "${files[0]%.am}")"
    output=$("$AMC" -o "$out_base" "${files[@]}" 2>&1)
    amc_exit=$?
    if [ $amc_exit -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (amc exited $amc_exit)"
        echo "$output" | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1)); return
    fi

    local c_file="${out_base}.c"
    if [ ! -f "$c_file" ]; then
        echo -e "${RED}FAIL${NC} (no .c emitted)"
        FAIL=$((FAIL + 1)); return
    fi
    # -Werror=implicit-function-declaration is the surgical check for
    # Bug 1: without the fix, the caller in file A sees an implicit
    # decl for the method defined in file B, which becomes a hard
    # error here instead of merely a warning.
    gcc -O2 -Iruntime -Werror=implicit-function-declaration \
        "$c_file" -lgc -lm -lz -o "$out_base" 2>/dev/null
    if [ ! -x "$out_base" ]; then
        echo -e "${RED}FAIL${NC} (gcc failed — implicit decl?)"
        FAIL=$((FAIL + 1)); return
    fi

    run_output=$("$out_base" 2>&1)
    if echo "$run_output" | grep -qF "$expected"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (output mismatch)"
        echo "    expected : $expected"
        echo "    got      : $(echo "$run_output" | head -3 | tr '\n' '|')"
        FAIL=$((FAIL + 1))
    fi
}

# Project F (v0.7.5+) end-to-end driver. Compile `$file` with one
# or more `--external <mod>` flags, then link the user .c against
# `lib/libamalgame.a` (pre-built by tools/build-stdlib.sh) instead
# of bundling the stdlib AM source. Asserts the expected substring
# appears in stdout.
#
#   $1 = test name
#   $2 = .am sample
#   $3 = expected substring in stdout
#   $4 = space-separated list of stdlib .am paths to pass --external
run_external_test() {
    local name="$1"
    local file="$2"
    local expected="$3"
    local externals="$4"

    printf "  %-38s" "$name"

    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        SKIP=$((SKIP + 1)); return
    fi
    if [ ! -f lib/libamalgame.a ]; then
        echo -e "${YELLOW}SKIP${NC} (lib/libamalgame.a missing — run tools/build-stdlib.sh)"
        SKIP=$((SKIP + 1)); return
    fi

    local ext_args=""
    for e in $externals; do ext_args="$ext_args --external $e"; done

    local out_base="$BUILD_DIR/$(basename "${file%.am}")"
    output=$("$AMC" -o "$out_base" $ext_args "$file" 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (amc exited non-zero)"
        echo "$output" | grep -E "error|Error" | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1)); return
    fi
    local c_file="${out_base}.c"
    gcc -O2 -Iruntime "$c_file" lib/libamalgame.a -lgc -lm -lz -o "$out_base" 2>/dev/null
    if [ ! -x "$out_base" ]; then
        echo -e "${RED}FAIL${NC} (gcc/link failed)"
        FAIL=$((FAIL + 1)); return
    fi
    local run_output
    run_output=$("$out_base" 2>&1)
    if echo "$run_output" | grep -qF "$expected"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (output mismatch)"
        echo "    expected : $expected"
        echo "    got      : $(echo "$run_output" | head -3 | tr '\n' '|')"
        FAIL=$((FAIL + 1))
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

    local base=" $(basename "$file") "
    if [ "$AMC" = "./amc" ] && [[ "$SKIP_SELFHOST" == *"$base"* ]]; then
        echo -e "${YELLOW}SKIP${NC} (self-host: pending compiler fix)"
        SKIP=$((SKIP + 1)); return
    fi

    local out_base="$BUILD_DIR/$(basename "${file%.am}")"
    output=$("$AMC" $flags -o "$out_base" "$file" 2>&1)
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

# Run `amc --check <file>` and assert it FAILS (non-zero exit) AND that
# the output contains the given diagnostic substring. Used for negative
# typecheck tests where the file is intentionally malformed.
run_check_fail() {
    local name="$1"
    local file="$2"
    local pattern="$3"

    printf "  %-34s" "$name"

    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        SKIP=$((SKIP + 1)); return
    fi

    local out
    out=$("$AMC" --check "$file" 2>&1)
    local rc=$?

    if [ $rc -eq 0 ]; then
        echo -e "${RED}FAIL${NC} (expected non-zero exit, got 0)"
        FAIL=$((FAIL + 1)); return
    fi

    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (diagnostic pattern not found)"
        echo "    expected substring : $pattern"
        echo "$out" | head -3 | sed 's/^/    /'
        FAIL=$((FAIL + 1))
    fi
}

# Run `amc test <dir>` and grep its merged stdout/stderr for a pattern.
# Exit code is ignored: the runner returns 1 when any test FAILs by
# design, but the suite cell still wants to assert a specific tally.
run_amc_test_check() {
    local name="$1"
    local dir="$2"
    local pattern="$3"

    printf "  %-34s" "$name"

    if [ ! -d "$dir" ]; then
        echo -e "${YELLOW}SKIP${NC} (dir not found)"
        SKIP=$((SKIP + 1)); return
    fi

    local out
    out=$("$AMC" test "$dir" 2>&1)

    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found)"
        echo "    looking for: $pattern"
        echo "    got:"
        echo "$out" | sed 's/^/      /'
        FAIL=$((FAIL + 1))
    fi
}

# Run amc --lint <file> and grep for an expected warning fragment in its stderr.
run_lint_check() {
    local name="$1"
    local file="$2"
    local pattern="$3"

    printf "  %-34s" "$name"

    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        SKIP=$((SKIP + 1)); return
    fi

    local out_base="$BUILD_DIR/$(basename "${file%.am}")"
    local out
    out=$("$AMC" --lint -o "$out_base" "$file" 2>&1)

    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (lint pattern not found)"
        echo "    looking for: $pattern"
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

    local out_base="$BUILD_DIR/$(basename "${file%.am}")"
    "$AMC" $flags -o "$out_base" "$file" >/dev/null 2>&1
    c_file="${out_base}.c"

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

# Compile a lib with --lib, archive into .o, link a consumer C, run, check stdout.
run_lib_link_test() {
    local name="$1"
    local lib_file="$2"
    local consumer_c="$3"
    local expected="$4"

    printf "  %-34s" "$name"

    if [ ! -f "$lib_file" ] || [ ! -f "$consumer_c" ]; then
        echo -e "${YELLOW}SKIP${NC} (missing input)"
        SKIP=$((SKIP + 1)); return
    fi
    if [ ! -x ./amc ]; then
        echo -e "${YELLOW}SKIP${NC} (./amc not built)"
        SKIP=$((SKIP + 1)); return
    fi

    local tmpdir="$(mktemp -d)"
    ./amc --lib "$lib_file" -o "$tmpdir/lib" >/dev/null 2>&1
    if [ ! -f "$tmpdir/lib.c" ]; then
        echo -e "${RED}FAIL${NC} (no .c generated)"
        FAIL=$((FAIL + 1)); rm -rf "$tmpdir"; return
    fi
    if grep -q "^int main" "$tmpdir/lib.c"; then
        echo -e "${RED}FAIL${NC} (lib has int main)"
        FAIL=$((FAIL + 1)); rm -rf "$tmpdir"; return
    fi
    local runtime_dir="$(dirname "$0")/../runtime"
    if ! gcc -I"$runtime_dir" -c "$tmpdir/lib.c" -o "$tmpdir/lib.o" 2>"$tmpdir/err"; then
        echo -e "${RED}FAIL${NC} (gcc -c)"
        sed 's/^/    /' "$tmpdir/err" | head -5
        FAIL=$((FAIL + 1)); rm -rf "$tmpdir"; return
    fi
    if ! gcc -I"$runtime_dir" "$consumer_c" "$tmpdir/lib.o" -lgc -lm -lz -o "$tmpdir/app" 2>"$tmpdir/err"; then
        echo -e "${RED}FAIL${NC} (link)"
        sed 's/^/    /' "$tmpdir/err" | head -5
        FAIL=$((FAIL + 1)); rm -rf "$tmpdir"; return
    fi
    local out
    out="$("$tmpdir/app" 2>&1)"
    if echo "$out" | grep -qF "$expected"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (output mismatch)"
        echo "    expected : $expected"
        echo "    got      : $out"
        FAIL=$((FAIL + 1))
    fi
    rm -rf "$tmpdir"
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
run_test "generic infer int"  "$SAMPLES/generic_inference.am"  "total: 60"
run_test "generic infer str"  "$SAMPLES/generic_inference.am"  "combined: Arthus et Bastien"
run_test "map infer int"      "$SAMPLES/map_inference.am"      "total: 141"
run_test "map infer str"      "$SAMPLES/map_inference.am"      "combined: bonjour / hello"
run_test "param infer list"   "$SAMPLES/params_inference.am"   "sum: 10"
run_test "param infer map"    "$SAMPLES/params_inference.am"   "greet: bonjour !"
run_test "return infer list"  "$SAMPLES/returns_inference.am"  "range total: 10"
run_test "return infer map"   "$SAMPLES/returns_inference.am"  "counts sum: 6"
run_test "closures"          "$SAMPLES/closures.am"     "Counter = 10"
run_test "closure: no cap"   "$SAMPLES/closures_capture.am"  "triple(7) = 21"
run_test "closure: 1 cap"    "$SAMPLES/closures_capture.am"  "addN(5) = 105"
run_test "closure: 2 caps"   "$SAMPLES/closures_capture.am"  "combine(10) = 25"
run_test "closure: snap val" "$SAMPLES/closures_capture.am"  "snap(0) = 1"
# string interpolation must trigger capture analysis inside lambda
# bodies — pre-fix the resolver skipped {…} embedded exprs and the
# cgen mangled `app.Method()` as static `app_Method()`.
run_test "interp cap: bare"  "$SAMPLES/interp_capture_in_lambda.am" "g=hi n=7"
run_test "interp cap: this"  "$SAMPLES/interp_capture_in_lambda.am" "say=hello ada n=3"
run_test "interp cap: pfx"   "$SAMPLES/interp_capture_in_lambda.am" "x=42"
# v0.8.30 — first-class functions via `Closure` field type.
run_test "FCF: arity-1 field"  "$SAMPLES/closure_as_field.am"  "[PASS] arity-1 this.Field call"
run_test "FCF: arity-2 field"  "$SAMPLES/closure_as_field.am"  "[PASS] arity-2 this.Field call"
run_test "FCF: arity-3 field"  "$SAMPLES/closure_as_field.am"  "[PASS] arity-3 this.Field call"
run_test "FCF: List + local"   "$SAMPLES/closure_as_field.am"  "[PASS] closure in List + local.Field call"
# v0.8.33 — nested MEMBER closure dispatch (obj.A.B.fn(args)).
run_test "FCF: nested IDENT 1" "$SAMPLES/closure_nested_member.am" "[PASS] nested IDENT chain arity-1"
run_test "FCF: nested THIS 1"  "$SAMPLES/closure_nested_member.am" "[PASS] nested THIS chain arity-1"
run_test "FCF: nested 3-deep"  "$SAMPLES/closure_nested_member.am" "[PASS] nested IDENT chain 3-deep"
run_test "lambda v2: 2-arg"   "$SAMPLES/lambdas_v2.am"  "add: 12"
run_test "lambda v2: block"   "$SAMPLES/lambdas_v2.am"  "plus3: 23"
run_test "lambda v2: 2+block" "$SAMPLES/lambdas_v2.am"  "mix: 17"
run_test "lambda v2: capture" "$SAMPLES/lambdas_v2.am"  "shift: 107"
run_test "lambda v2: 3-arg"   "$SAMPLES/lambdas_v2.am"  "pick: 6"
run_test "ho: filter"          "$SAMPLES/list_higher_order.am"  "filter count: 3"
run_test "ho: map"             "$SAMPLES/list_higher_order.am"  "map last: 10"
run_test "ho: reduce sum"      "$SAMPLES/list_higher_order.am"  "sum: 15"
run_test "ho: reduce block"    "$SAMPLES/list_higher_order.am"  "countGT2: 3"
run_test "ho: any/all/countIf" "$SAMPLES/list_higher_order.am"  "evens: 2"
run_test "ho: forEach"         "$SAMPLES/list_higher_order.am"  "foreach: 5"
run_test "ho: chain filter+map" "$SAMPLES/list_higher_order.am" "chain first: 30"
run_test "lambda v2.5: Map<string>"  "$SAMPLES/lambda_v25_typed.am"  "first name: Alice"
run_test "lambda v2.5: Map<int>"     "$SAMPLES/lambda_v25_typed.am"  "first age: 30"
run_test "lambda v2.5: Filter class" "$SAMPLES/lambda_v25_typed.am"  "adults: 2"
run_test "lambda v2.5: Filter mem"   "$SAMPLES/lambda_v25_typed.am"  "adult0: Alice"
run_test "lambda v2.5: Any"          "$SAMPLES/lambda_v25_typed.am"  "anyMinor: true"
run_test "lambda v2.5: All"          "$SAMPLES/lambda_v25_typed.am"  "allAdults: false"
run_test "lambda v2.5: CountIf"      "$SAMPLES/lambda_v25_typed.am"  "underAge: 1"
run_test "param syntax: TS-style"    "$SAMPLES/param_ts_syntax.am"   "box: 21"
run_test "chain: multiline filter"   "$SAMPLES/multiline_method_chain.am"  "first: Alice"
run_test "chain: multiline last"     "$SAMPLES/multiline_method_chain.am"  "last: Carol"
run_test "chain: stmt boundary OK"   "$SAMPLES/multiline_method_chain.am"  "sum: 3"
run_test "auto-qualify: ctor write"  "$SAMPLES/cgen_auto_qualify.am" "value: 7"
run_test "auto-qualify: method read" "$SAMPLES/cgen_auto_qualify.am" "dp: 17"
run_test "auto-qualify: method write" "$SAMPLES/cgen_auto_qualify.am" "after reset value: 0"
run_test "auto-qualify: explicit this. mix" "$SAMPLES/cgen_auto_qualify.am" "after reset label: reset"

# Bug 3 regression (v0.8.23): AmalgameMap_remove must mark the
# slot as a tombstone, not empty — otherwise colliding keys
# inserted before the remove become unreachable.
run_test "map tombstone: count"       "$SAMPLES/map_tombstone.am"  "found after remove: 25"
run_test "map tombstone: sum"         "$SAMPLES/map_tombstone.am"  "sum after remove: 925"
# Bug 4 (v0.8.23): Keys()/Values() used truthy test on `used` which
# leaked tombstoned entries. Expect 25 live, not 50.
run_test "map tombstone: keys"        "$SAMPLES/map_tombstone.am"  "Keys() count: 25"
run_test "map tombstone: values"      "$SAMPLES/map_tombstone.am"  "Values() count: 25"

# Bug 5 regression (v0.8.24): for i in 0..xs.Count() — the RHS of
# `..` must consume the .method() / (args) chain, otherwise the
# cgen sees a CALL on stmt.Left and emits the foreach branch.
run_test "for-range: 0..method()"     "$SAMPLES/for_range_method_call.am"  "sum: 60"
run_test "for-range: map.Size()"      "$SAMPLES/for_range_method_call.am"  "map iters: 3"
run_test "for-range: ident..method"   "$SAMPLES/for_range_method_call.am"  "ident-range hits: 2"
# Bug 5 mirror form (v0.8.25): call-on-LHS now parses too thanks
# to the ParseRange refactor (`..` is a proper binary operator).
run_test "for-range: call..ident"     "$SAMPLES/for_range_method_call.am"  "mirror sum: 7"
run_test "for-range: call..call"     "$SAMPLES/for_range_method_call.am"  "both-call sum: 3"

# Bug 7 regression (v0.8.26): BINARY-`+` used to misclassify any
# String_* callee on the RHS as a string-concat dispatch via the
# `String_StartsWith(callee, "String_")` name-pattern check, even
# though String_Length / String_IndexOf / String_LastIndexOf /
# String_ToInt return i64. The fix consults InferTypeFromExpr (which
# already has precise return types) instead of the name prefix.
run_test "bug7: int + String_Length"  "$SAMPLES/bug7_int_plus_string_helper.am"  "[PASS] int + String_Length lowers to addition"
run_test "bug7: String_Length + int"  "$SAMPLES/bug7_int_plus_string_helper.am"  "[PASS] String_Length + int lowers to addition"
run_test "bug7: string + string"      "$SAMPLES/bug7_int_plus_string_helper.am"  "[PASS] string + string still concatenates"
run_test "bug7: string + FromInt"     "$SAMPLES/bug7_int_plus_string_helper.am"  "[PASS] string + String_FromInt still concatenates"
run_test "bug7: Length + Length"      "$SAMPLES/bug7_int_plus_string_helper.am"  "[PASS] String_Length + String_Length lowers to addition"

# Bug 8 regression (v0.8.26): the lexer fuses two consecutive `>` into
# a single OP_SHR token, so the parser's generic-bracket loops were
# bailing on `List<List<int>>` (and deeper) annotations. Fixed in
# ParseTypeName, ParseNew, and the lambda-lookahead helper.
run_test "bug8: List<List<int>>"      "$SAMPLES/bug8_nested_generics.am"  "[PASS] List<List<int>> annotation parses"
run_test "bug8: triple-nested"        "$SAMPLES/bug8_nested_generics.am"  "[PASS] List<List<List<int>>> annotation parses"
run_test "bug8: quad-nested"          "$SAMPLES/bug8_nested_generics.am"  "[PASS] List<List<List<List<int>>>> annotation parses"
run_test "bug8: Map<K, List<V>>"      "$SAMPLES/bug8_nested_generics.am"  "[PASS] Map<string, List<int>> annotation parses"

# Bug 2 regression (v0.8.23): cgen Map/Set/List method dispatch
# must downcase to *_set / *_get / *_has / *_add / *_remove when
# the receiver is `this.field` or `obj.field`, not just a bare
# local. Before, `this.entries.Set(k, v)` emitted the undefined
# `AmalgameMap_Set` (PascalCase from the generic method path).
run_test "field map: get"             "$SAMPLES/field_map_dispatch.am"  "alpha=1"
run_test "field map: has-then-get"    "$SAMPLES/field_map_dispatch.am"  "beta=2"
run_test "field set: size"            "$SAMPLES/field_map_dispatch.am"  "tags=3"
run_test "field list: count"          "$SAMPLES/field_map_dispatch.am"  "log=3"
run_test "field map: remove"          "$SAMPLES/field_map_dispatch.am"  "after forget beta=-1"
run_test "field map: kept after rm"   "$SAMPLES/field_map_dispatch.am"  "alpha still=1"

# Bug 1 regression (v0.8.23): multi-file compilation — caller in file
# A reaches a method defined in file B. Before the fix, Pass 2 emitted
# main.am's body (calling Helper_*) before helper.am's forward decls
# landed, so gcc tripped -Wimplicit-function-declaration. Test passes
# main.am FIRST (caller order) to exercise the dependency direction.
run_multi_test "multi-file: caller first" "got: 35" \
    "$SAMPLES/multi_file_bug1/main.am" "$SAMPLES/multi_file_bug1/helper.am"

# Inline-C blocks (`@c { … }`) — body spliced verbatim into the emitted C.
run_test "inline-C: return value"     "$SAMPLES/inline_c.am"  "len=8"
run_test "inline-C: local + multi-stmt" "$SAMPLES/inline_c.am"  "doubled=42"
run_test "inline-C: brace torture"    "$SAMPLES/inline_c.am"  "braces=ab}c{de"
run_test "inline-C: void side-effect" "$SAMPLES/inline_c.am"  "!done"

# File-scope `@c_include` / `@c_link` directives — give inline-C bodies
# access to libc headers not in the runtime prelude (`-lm` etc. are
# already linked by this harness; the @c_link is asserted via the
# comment the cgen leaves in the emitted .c).
run_test "inline-C dir: toupper"      "$SAMPLES/inline_c_directives.am"  "a→65"
run_test "inline-C dir: toupper z"    "$SAMPLES/inline_c_directives.am"  "z→90"
run_test "inline-C dir: isalpha yes"  "$SAMPLES/inline_c_directives.am"  "A is alpha"
run_test "inline-C dir: isalpha no"   "$SAMPLES/inline_c_directives.am"  "0 is not alpha"

# Project F (v0.7.5+) — `--external <mod.am>` + link against
# pre-compiled lib/libamalgame.a. Skips if the lib hasn't been
# built yet (build_amc.sh's Step 4 produces it).
# v0.7.7: random / encoding moved to external packages (no longer
# in libamalgame.a). msgpack stays bundled — only test left here.
LIBA_SAMPLE="$SAMPLES/external_libamalgame.am"
LIBA_EXTS="src/stdlib/json.am src/stdlib/msgpack.am"
run_external_test "libamalgame.a: msgpack"  "$LIBA_SAMPLE"  "mp: 42"           "$LIBA_EXTS"

# Bug 6 regression (v0.8.26): user code that declares a JsonValue
# field on its own class must lower the field type to the bundled
# stdlib namespace (Amalgame_Formats_Json_JsonValue), not the user's
# NsPrefix-mangled name. Verifies the auto-external mechanism kicks
# in from `import Amalgame.Formats.Json` alone — NO `--external` flag
# passed here, matching the `amc build` end-user path.
run_external_test "bug6: JsonValue field type" \
    "$SAMPLES/bundled_stdlib_field_type.am" \
    "[PASS] field type lowers to bundled-stdlib namespace" \
    ""
run_external_test "bug6: Definition setter" \
    "$SAMPLES/bundled_stdlib_field_type.am" \
    "[PASS] Definition setter: hello" \
    ""

# Bug 9 regression (v0.8.27): RegisterExternalProg only emitted
# `typedef struct _X X;` for each external class, so any field
# access on `r.Ok` / `r.Value` / `r.Error` hit gcc's
# `invalid use of incomplete typedef`. Fix emits the full struct
# body + the body of any sibling simple enum (e.g. `JsonKind`).
run_external_test "bug9: JsonResult.Ok+.Value" \
    "$SAMPLES/bug9_external_struct_fields.am" \
    "[PASS] external field access — JsonResult.Ok + .Value" \
    ""
run_external_test "bug9: JsonError.Message" \
    "$SAMPLES/bug9_external_struct_fields.am" \
    "[PASS] external field access — JsonError.Message populated" \
    ""
run_external_test "bug9: JsonError.Line" \
    "$SAMPLES/bug9_external_struct_fields.am" \
    "[PASS] external field access — JsonError.Line" \
    ""

# Bug 10 regression (v0.8.28): `obj.Field.Get(i) == s` where
# `obj` is a local var of class type C and `C.Field: List<string>`
# used to emit raw C `==` (pointer compare) instead of
# `code_string_equals`. AutoBUS's subscription.am::FindByTopic
# exposed it. Fix: InferTypeFromExpr's `.Get(...)` branch now
# resolves the element type via `ListElemGet(LocalTypeGet(obj),
# Field)` for the chained MEMBER receiver shape.
run_test "bug10: obj.Field.Get == localVar" \
    "$SAMPLES/bug10_list_string_eq.am" \
    "[PASS] bug10 — obj.Field.Get(i) == localVar"
run_test "bug10: localVar == obj.Field.Get" \
    "$SAMPLES/bug10_list_string_eq.am" \
    "[PASS] bug10 — localVar == obj.Field.Get(i)"
run_test "bug10: obj.Field.Get != localVar" \
    "$SAMPLES/bug10_list_string_eq.am" \
    "[PASS] bug10 — obj.Field.Get(i) != localVar"
run_test "bug10: localList.Get == concatVar" \
    "$SAMPLES/bug10_list_string_eq.am" \
    "[PASS] bug10 — localList.Get(i) == concatVar"

# Bare `{ ... }` block-statement scope (v0.8.35).
# Before the fix, EmitStmt had no NodeKind.BLOCK branch — a bare
# `{ ... }` at statement position was silently dropped (resolver
# scoped correctly, cgen emitted nothing). Workaround in user code
# was to rename locals across sibling sections (ui-forms tests
# renamed r0/r2 → rz0/rz2). Fix: EmitStmt now recognises
# NodeKind.BLOCK and wraps in C `{ ... }`, giving each sibling
# block its own scope.
run_test "let_block_scope: sibling-1"   "$SAMPLES/let_block_scope.am"  "[PASS] block-1 x=1"
run_test "let_block_scope: sibling-2"   "$SAMPLES/let_block_scope.am"  "[PASS] block-2 x=2"
run_test "let_block_scope: section-A"   "$SAMPLES/let_block_scope.am"  "[PASS] section-A sum=30"
run_test "let_block_scope: section-B"   "$SAMPLES/let_block_scope.am"  "[PASS] section-B sum=300"
run_test "let_block_scope: nested"      "$SAMPLES/let_block_scope.am"  "[PASS] nested outer=99"
run_test "let_block_scope: post-nested" "$SAMPLES/let_block_scope.am"  "[PASS] post-nested outer=7"

# Lambda v2.5 — non-int signatures end-to-end.
# Pre-fix, the lambda fn body was typed correctly but the
# subsequent `for u in <Filter result>` typed the loop var as
# void*, so `u.Name`/`u.Method()` failed to compile. The
# EmitForIn now infers the elem type from typed locals
# (ListElemGet) and types the loop var accordingly. Untyped
# lists fall back to void* (legacy).
run_test "lambda_v25: Map<User,int>"     "$SAMPLES/lambda_v25_nonint.am"  "[PASS] Map<User,int> ageSum=95"
run_test "lambda_v25: Filter<User>"      "$SAMPLES/lambda_v25_nonint.am"  "[PASS] Filter<User> adultNames=alice,carol,"
run_test "lambda_v25: capture+Filter"    "$SAMPLES/lambda_v25_nonint.am"  "[PASS] capture+Filter hits=2"
run_test "lambda_v25: chain Filter|Map"  "$SAMPLES/lambda_v25_nonint.am"  "[PASS] chain Filter|Map joined=alice|carol|"

# Typed closures — `Closure<A, R>` / `Closure<A1, A2, R>` / etc.
# Pre-fix, the bare `Closure` type erased all arg/return info so
# the cgen defaulted to i64 + intptr_t roundtrip at call sites
# and inside lambda bodies. mosaic-build.sh had to add
# `-Wno-int-conversion -Wno-incompatible-pointer-types` to silence
# the resulting gcc noise. With typed Closure:
#   - lambda VAR_DECL patches the lambda's param Str from <A,…>
#   - EmitClass tracks per-field closure return type for MEMBER
#     closure-call dispatch
#   - typechecker extracts R from `Closure<…, R>` as the call
#     result type
# Sample exercises arities 1/2 + pointer/scalar combinations +
# typed Closure as a class field with this.Field(x) dispatch.
run_test "closure_typed: <int,int>"           "$SAMPLES/closure_typed.am"  "[PASS] Closure<int,int> r1=42"
run_test "closure_typed: <int,int,int>"       "$SAMPLES/closure_typed.am"  "[PASS] Closure<int,int,int> r2=30"
run_test "closure_typed: <User,string>"       "$SAMPLES/closure_typed.am"  "[PASS] Closure<User,string> r3=alice"
run_test "closure_typed: <User,int>"          "$SAMPLES/closure_typed.am"  "[PASS] Closure<User,int> r4=99"
run_test "closure_typed: field <Conn,Conn>"   "$SAMPLES/closure_typed.am"  "[PASS] Server field Closure<Conn,Conn> id=42"

# Lambda-param inference from typed-Closure ctor/method params (v0.8.36+).
# Pre-fix, `new Route(c => ...)` needed an explicit `(c: Type) =>`
# annotation because the resolver only patched lambdas bound via a
# typed local. Now the resolver also walks ctor/method params and
# pushes A_i from `Closure<A1, …, R>` into the lambda's i-th PARAM.
run_test "lambda_infer: ctor-arg"           "$SAMPLES/lambda_inference_ctor.am"  "[PASS] ctor-arg lambda n1=alice"
run_test "lambda_infer: method-arg"         "$SAMPLES/lambda_inference_ctor.am"  "[PASS] method-arg lambda n2=bob"
run_test "lambda_infer: explicit override"  "$SAMPLES/lambda_inference_ctor.am"  "[PASS] explicit-param still works n3=carol"

# Nested generics through chained `.Get(i).Get(j)` (v0.8.36+).
# Pre-fix the inner Get fell through to `(void*)AmalgameList_get(...)`
# because TrackGenericLocal only stored one layer of element type.
# Post-fix `__local_raw__:<varname>` carries the raw type and
# `RecoverChainedListElemRaw` peels one List<...> layer per hop.
run_test "nested_gen: List<List<string>>"           "$SAMPLES/nested_generics.am"  "[PASS] List<List<string>> rows[0][0]=alpha"
run_test "nested_gen: List<List<int>>"              "$SAMPLES/nested_generics.am"  "[PASS] List<List<int>> m[0][0]=42"
run_test "nested_gen: List<List<List<int>>>"        "$SAMPLES/nested_generics.am"  "[PASS] List<List<List<int>>> cube[0][0][0]=7"
run_test "nested_gen: instance call infer"          "$SAMPLES/nested_generics.am"  "[PASS] instance call nested rs[0][0]=alice"

# Spread operator in list literals (v0.8.36+). `[...a, ...b, c]`
# splices each spread operand's elements into the fresh list at
# its position. Variadic call sites / param definitions are out
# of MVP scope.
run_test "spread: int sum"            "$SAMPLES/spread_list_literal.am"  "[PASS] spread int sum=114"
run_test "spread: int count"          "$SAMPLES/spread_list_literal.am"  "[PASS] spread int count=6"
run_test "spread: strings count"      "$SAMPLES/spread_list_literal.am"  "[PASS] spread strings count=5"
run_test "spread: strings order"      "$SAMPLES/spread_list_literal.am"  "[PASS] spread strings order preserved"
run_test "spread: call-result sum"    "$SAMPLES/spread_list_literal.am"  "[PASS] spread call result sum=1119"
run_test "spread: empty source"       "$SAMPLES/spread_list_literal.am"  "[PASS] spread empty count=2"

# Env builtins (Env.Get / Env.Has) — exported here so the sample sees them.
export AMC_ENV_PROBE=hello
run_test "env: hasPath true"         "$SAMPLES/stdlib_env.am"  "hasPath: true"
run_test "env: probe value"          "$SAMPLES/stdlib_env.am"  "probe: hello"
run_test "env: missing → false"      "$SAMPLES/stdlib_env.am"  "hasFake: false"
unset AMC_ENV_PROBE

# ── Process module ─────────────────────────────────────
echo ""
echo "── Process ─────────────────────────────"
run_test "process: run exit"      "$SAMPLES/process_api.am"  "exit=0"
run_test "process: cap exit"      "$SAMPLES/process_api.am"  "cap.exit=0"
run_test "process: cap stdout"    "$SAMPLES/process_api.am"  "cap.out=captured-line"
run_test "process: nonzero exit"  "$SAMPLES/process_api.am"  "bad.exit=1"
run_test "process: stderr merge"  "$SAMPLES/process_api.am"  "merged.out=stderr-bytes"

# Multi-line && / || continuation (v0.8.21). Sample emits 4 lines
# A/B/C/D — one per shape covered.
run_test "ml-logical: && newline"   "$SAMPLES/multiline_logical.am"  "A"
run_test "ml-logical: && trailing"  "$SAMPLES/multiline_logical.am"  "B"
run_test "ml-logical: || newline"   "$SAMPLES/multiline_logical.am"  "C"
run_test "ml-logical: && + || mix"  "$SAMPLES/multiline_logical.am"  "D"

# Process v3 streaming (v0.8.22). Spawns a 3-line shell child
# and reads lines one by one across 50ms gaps.
run_test "process v3: spawn alive"  "$SAMPLES/process_v3.am"  "spawn: alive"
run_test "process v3: stream line1" "$SAMPLES/process_v3.am"  "got: line-1"
run_test "process v3: stream line3" "$SAMPLES/process_v3.am"  "got: line-3"
run_test "process v3: eof"          "$SAMPLES/process_v3.am"  "eof"
run_test "process v3: exit 0"       "$SAMPLES/process_v3.am"  "exit=0"

# ── amc test runner ────────────────────────────────────
echo ""
echo "── amc test ────────────────────────────"
run_amc_test_check "amc test: discovers"  "$SAMPLES/test_runner"  "arith_test.am"
run_amc_test_check "amc test: pass tally" "$SAMPLES/test_runner"  "PASS: 4"
run_amc_test_check "amc test: fail tally" "$SAMPLES/test_runner"  "FAIL: 1"
run_amc_test_check "amc test: skip tally" "$SAMPLES/test_runner"  "SKIP: 1"

# ── amc lsp ────────────────────────────────────────────
echo ""
echo "── amc lsp ─────────────────────────────"

# Pipe LSP messages into `amc lsp` and grep its stdout for an
# expected substring. The runner exits 0 on `exit` notification.
run_lsp_check() {
    local name="$1"
    local pattern="$2"
    local input="$3"

    printf "  %-34s" "$name"
    local out
    out=$(printf '%s' "$input" | "$AMC" lsp 2>&1)
    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found)"
        echo "    looking for: $pattern"
        echo "    got:"
        echo "$out" | sed 's/^/      /'
        FAIL=$((FAIL + 1))
    fi
}

# Helper: emit `Content-Length: N\r\n\r\n<body>` for a JSON body.
lsp_frame() {
    local body="$1"
    local n=${#body}
    printf 'Content-Length: %d\r\n\r\n%s' "$n" "$body"
}

# Build a fixture sequence: initialize → didOpen with a buggy
# file → shutdown → exit. The buggy file references an
# undeclared symbol so the resolver produces a diagnostic.
lsp_init='{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}'
# `\n` here is the two-char sequence backslash-n, decoded by the
# server's JSON extractor into a real newline. The parser needs
# multi-line class bodies so embedding actual newlines via the
# JSON escape is the simplest way.
lsp_open='{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"file:///tmp/lsp_test.am","languageId":"amalgame","version":1,"text":"class Program {\n    public static void Main() {\n        let x = thisDoesNotExist\n    }\n}"}}}'
lsp_shut='{"jsonrpc":"2.0","id":2,"method":"shutdown"}'
lsp_exit='{"jsonrpc":"2.0","method":"exit"}'

lsp_seq=$(lsp_frame "$lsp_init"; lsp_frame "$lsp_open"; lsp_frame "$lsp_shut"; lsp_frame "$lsp_exit")

run_lsp_check "lsp: initialize reply"   '"capabilities":{"textDocumentSync":1,"hoverProvider":true,"definitionProvider":true,"declarationProvider":true,"typeDefinitionProvider":true,"documentSymbolProvider":true,"workspaceSymbolProvider":true,"referencesProvider":true,"renameProvider":{"prepareProvider":true},"callHierarchyProvider":true,"inlayHintProvider":true,"codeActionProvider":true,"foldingRangeProvider":true,"completionProvider":{"triggerCharacters":["."]},"signatureHelpProvider":{"triggerCharacters":["(",","]}}' "$lsp_seq"
run_lsp_check "lsp: publishDiagnostics" '"method":"textDocument/publishDiagnostics"' "$lsp_seq"
run_lsp_check "lsp: error in diag"      'thisDoesNotExist'                          "$lsp_seq"
run_lsp_check "lsp: shutdown reply"     '"id":2,"result":null'                      "$lsp_seq"

# Hover + completion sequence — a well-typed file with a Console
# call so the typechecker has something to report on. The hover
# request targets `Console` at line 3, char 8 (0-based, LSP coords).
lsp_open_ok='{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"file:///tmp/lsp_hover.am","languageId":"amalgame","version":1,"text":"class Program {\n    public static void Main() {\n        let x = 42\n        Console.WriteLine(x)\n    }\n}"}}}'
lsp_hover='{"jsonrpc":"2.0","id":3,"method":"textDocument/hover","params":{"textDocument":{"uri":"file:///tmp/lsp_hover.am"},"position":{"line":3,"character":8}}}'
lsp_complete='{"jsonrpc":"2.0","id":4,"method":"textDocument/completion","params":{"textDocument":{"uri":"file:///tmp/lsp_hover.am"},"position":{"line":3,"character":0}}}'

lsp_query_seq=$(lsp_frame "$lsp_init"; lsp_frame "$lsp_open_ok"; lsp_frame "$lsp_hover"; lsp_frame "$lsp_complete"; lsp_frame "$lsp_shut"; lsp_frame "$lsp_exit")

run_lsp_check "lsp: hover has contents"   '"id":3,"result":{"contents"'                  "$lsp_query_seq"
run_lsp_check "lsp: completion has items" '"id":4,"result":{"isIncomplete":false,"items"' "$lsp_query_seq"
run_lsp_check "lsp: completion lists Console" '"label":"Console"'                        "$lsp_query_seq"

# foldingRange — a fixture with two imports, a 3-line comment
# header, and a class with a nested method body. Exercises all
# three fold flavours (region / comment / imports) in one shot.
lsp_open_fold='{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"file:///tmp/lsp_fold.am","languageId":"amalgame","version":1,"text":"import Amalgame.Console\nimport Amalgame.String\n\n// header A\n// header B\n// header C\nclass Foo {\n    function Bar() {\n        let x = 1\n        let y = 2\n    }\n}\n"}}}'
lsp_fold='{"jsonrpc":"2.0","id":5,"method":"textDocument/foldingRange","params":{"textDocument":{"uri":"file:///tmp/lsp_fold.am"}}}'

lsp_fold_seq=$(lsp_frame "$lsp_init"; lsp_frame "$lsp_open_fold"; lsp_frame "$lsp_fold"; lsp_frame "$lsp_shut"; lsp_frame "$lsp_exit")

# Method body is lines 8..11 (1-indexed) → 7..9 (LSP, end-1 hides `}`).
run_lsp_check "lsp: folding method body"   '"id":5,"result":[{"startLine":7,"endLine":9}'              "$lsp_fold_seq"
# Class body is lines 7..12 → 6..10.
run_lsp_check "lsp: folding class body"    '{"startLine":6,"endLine":10}'                              "$lsp_fold_seq"
# 3-line comment header on lines 4..6 → 3..5.
run_lsp_check "lsp: folding comment run"   '{"startLine":3,"endLine":5,"kind":"comment"}'              "$lsp_fold_seq"
# 2-line import group on lines 1..2 → 0..1.
run_lsp_check "lsp: folding import run"    '{"startLine":0,"endLine":1,"kind":"imports"}'              "$lsp_fold_seq"

# Bug 6 regression (v0.8.25): the workspace-root walk now treats
# `amalgame.toml` as a marker. Pre-v0.8.25, opening a file in a
# sub-directory of a project that only had a manifest (no .git /
# build_amc.sh / package.json) made `FindWorkspaceRoot` fall back
# to the file's own dir — the sibling scan never reached the other
# directories of the project, and cross-dir class references came
# back as "Unknown symbol". Fixture lives at
# tests/fixtures/lsp-workspace/.
LSP_FIX_ROOT="$(pwd)/tests/fixtures/lsp-workspace"
LSP_FIX_FILE="$LSP_FIX_ROOT/tests/byteio_test.am"
lsp_open_xdir='{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"file://'"$LSP_FIX_FILE"'","languageId":"amalgame","version":1,"text":"namespace Workspace.Tests\n\npublic class TestRunner {\n    public static void Run() {\n        let v: int = ByteIO.Read(42)\n    }\n}\n"}}}'
lsp_xdir_seq=$(lsp_frame "$lsp_init"; lsp_frame "$lsp_open_xdir"; lsp_frame "$lsp_shut"; lsp_frame "$lsp_exit")

# Negative check: the diagnostics array for this file must NOT
# contain "Unknown symbol 'ByteIO'". Reuses the lsp_check helper
# pattern but inverts the assertion.
run_lsp_absent() {
    local name="$1"
    local pattern="$2"
    local input="$3"
    printf "  %-34s" "$name"
    local out
    out=$(printf '%s' "$input" | "$AMC" lsp 2>&1)
    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${RED}FAIL${NC} (unexpected pattern present)"
        echo "    should NOT contain: $pattern"
        echo "    got snippet:"
        echo "$out" | grep -F "$pattern" | head -2 | sed 's/^/      /'
        FAIL=$((FAIL + 1))
    else
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    fi
}
run_lsp_absent "lsp: xdir resolves ByteIO" "Unknown symbol 'ByteIO'" "$lsp_xdir_seq"

# ── Phase B navigation: documentSymbol / definition / references ──────
# Fixture: two classes, one with a field + ctor + greeter method.
# Line numbering (LSP 0-indexed):
#   0: class Foo {
#   1:     public Name: string
#   2:     public Foo() {
#   3:         this.Name = "x"
#   4:     }
#   5:     public string Greet() {
#   6:         return "hi " + this.Name
#   7:     }
#   8: }
#   9: class Program {
#  10:     public static void Main() {
#  11:         let f = new Foo()
#  12:         let s = f.Greet()
#  13:     }
#  14: }
lsp_nav_text='class Foo {\n    public Name: string\n    public Foo() {\n        this.Name = \"x\"\n    }\n    public string Greet() {\n        return \"hi \" + this.Name\n    }\n}\nclass Program {\n    public static void Main() {\n        let f = new Foo()\n        let s = f.Greet()\n    }\n}'
lsp_open_nav='{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"file:///tmp/lsp_nav.am","languageId":"amalgame","version":1,"text":"'"$lsp_nav_text"'"}}}'

# documentSymbol: outline of the open file.
lsp_docsym='{"jsonrpc":"2.0","id":10,"method":"textDocument/documentSymbol","params":{"textDocument":{"uri":"file:///tmp/lsp_nav.am"}}}'
# definition on `Foo` inside `new Foo()` (line 11 col 20, LSP 0-indexed)
lsp_defFoo='{"jsonrpc":"2.0","id":11,"method":"textDocument/definition","params":{"textDocument":{"uri":"file:///tmp/lsp_nav.am"},"position":{"line":11,"character":20}}}'
# definition on `Greet` in `f.Greet()` (line 12 col 18)
lsp_defGreet='{"jsonrpc":"2.0","id":12,"method":"textDocument/definition","params":{"textDocument":{"uri":"file:///tmp/lsp_nav.am"},"position":{"line":12,"character":18}}}'
# references at the Greet declaration (line 5 col 18)
lsp_refGreet='{"jsonrpc":"2.0","id":13,"method":"textDocument/references","params":{"textDocument":{"uri":"file:///tmp/lsp_nav.am"},"position":{"line":5,"character":18},"context":{"includeDeclaration":true}}}'

lsp_nav_seq=$(lsp_frame "$lsp_init"; lsp_frame "$lsp_open_nav"; lsp_frame "$lsp_docsym"; lsp_frame "$lsp_defFoo"; lsp_frame "$lsp_defGreet"; lsp_frame "$lsp_refGreet"; lsp_frame "$lsp_shut"; lsp_frame "$lsp_exit")

# documentSymbol — outline contains both classes + their members.
run_lsp_check "lsp: docsym lists Foo"      '"name":"Foo","kind":5'      "$lsp_nav_seq"
run_lsp_check "lsp: docsym lists Program"  '"name":"Program","kind":5'  "$lsp_nav_seq"
run_lsp_check "lsp: docsym Foo has Greet"  '"name":"Greet","kind":6'    "$lsp_nav_seq"
run_lsp_check "lsp: docsym Foo has Name"   '"name":"Name","kind":8'     "$lsp_nav_seq"
run_lsp_check "lsp: docsym Program.Main"   '"name":"Main","kind":6'     "$lsp_nav_seq"
# definition — Foo at line 11:20 should jump to line 0 (class Foo decl, LSP 0-indexed)
run_lsp_check "lsp: def Foo at line 0"     '"id":11,"result":{"uri":"file:///tmp/lsp_nav.am","range":{"start":{"line":0'    "$lsp_nav_seq"
# definition — Greet at line 12:18 should jump to line 5 (method decl)
run_lsp_check "lsp: def Greet at line 5"   '"id":12,"result":{"uri":"file:///tmp/lsp_nav.am","range":{"start":{"line":5'    "$lsp_nav_seq"
# references — Greet at the decl site should return at least 2 occurrences (decl + call)
run_lsp_check "lsp: refs Greet has result" '"id":13,"result":['          "$lsp_nav_seq"
run_lsp_check "lsp: refs Greet decl line"  '"line":5'                    "$lsp_nav_seq"
run_lsp_check "lsp: refs Greet call line"  '"line":12'                   "$lsp_nav_seq"

# Phase B follow-up: rename / prepareRename / workspace symbol /
# inlayHint / codeAction / callHierarchy. Uses the same fixture.
lsp_prepareRename='{"jsonrpc":"2.0","id":20,"method":"textDocument/prepareRename","params":{"textDocument":{"uri":"file:///tmp/lsp_nav.am"},"position":{"line":5,"character":18}}}'
lsp_rename='{"jsonrpc":"2.0","id":21,"method":"textDocument/rename","params":{"textDocument":{"uri":"file:///tmp/lsp_nav.am"},"position":{"line":5,"character":18},"newName":"SayHi"}}'
lsp_wssym='{"jsonrpc":"2.0","id":22,"method":"workspace/symbol","params":{"query":"Foo"}}'
lsp_inlay='{"jsonrpc":"2.0","id":23,"method":"textDocument/inlayHint","params":{"textDocument":{"uri":"file:///tmp/lsp_nav.am"}}}'
lsp_codeact='{"jsonrpc":"2.0","id":24,"method":"textDocument/codeAction","params":{"textDocument":{"uri":"file:///tmp/lsp_nav.am"},"range":{"start":{"line":11,"character":0},"end":{"line":11,"character":30}},"context":{"diagnostics":[]}}}'
lsp_callhPrep='{"jsonrpc":"2.0","id":25,"method":"textDocument/prepareCallHierarchy","params":{"textDocument":{"uri":"file:///tmp/lsp_nav.am"},"position":{"line":5,"character":18}}}'

lsp_navx_seq=$(lsp_frame "$lsp_init"; lsp_frame "$lsp_open_nav"; lsp_frame "$lsp_prepareRename"; lsp_frame "$lsp_rename"; lsp_frame "$lsp_wssym"; lsp_frame "$lsp_inlay"; lsp_frame "$lsp_codeact"; lsp_frame "$lsp_callhPrep"; lsp_frame "$lsp_shut"; lsp_frame "$lsp_exit")

# prepareRename should return a placeholder = the current name.
run_lsp_check "lsp: prepRename placeholder" '"placeholder":"Greet"'             "$lsp_navx_seq"
# rename should emit a WorkspaceEdit with `changes` containing the file URI.
run_lsp_check "lsp: rename has changes"     '"id":21,"result":{"changes":{'      "$lsp_navx_seq"
run_lsp_check "lsp: rename newText SayHi"   '"newText":"SayHi"'                 "$lsp_navx_seq"
# workspace/symbol("Foo") — at minimum the Foo class must show up.
run_lsp_check "lsp: ws-symbol finds Foo"    '"id":22,"result":[{"name":"Foo","kind":5'  "$lsp_navx_seq"
# inlayHint — `let f = new Foo()` has no annotation, so a hint `: Foo` is inserted.
run_lsp_check "lsp: inlayHint Foo label"    '"label":": Foo"'                   "$lsp_navx_seq"
# codeAction on the let-line — should offer "add explicit type annotation".
run_lsp_check "lsp: codeAction title"       '"title":"Add type annotation: Foo"' "$lsp_navx_seq"
# prepareCallHierarchy on Greet decl — at least one CallHierarchyItem.
run_lsp_check "lsp: callh prepare item"     '"id":25,"result":[{"name":"Greet"'  "$lsp_navx_seq"

# callHierarchy/incomingCalls — pass back the item from prepare so
# the server can resolve the method and walk for callers. Main is
# the sole caller of Greet.
lsp_callhIn='{"jsonrpc":"2.0","id":26,"method":"callHierarchy/incomingCalls","params":{"item":{"name":"Greet","kind":6,"uri":"file:///tmp/lsp_nav.am","range":{"start":{"line":5,"character":18},"end":{"line":5,"character":23}},"selectionRange":{"start":{"line":5,"character":18},"end":{"line":5,"character":23}},"data":{"name":"Greet","uri":"file:///tmp/lsp_nav.am","line":5,"character":18}}}}'
lsp_callhOut='{"jsonrpc":"2.0","id":27,"method":"callHierarchy/outgoingCalls","params":{"item":{"name":"Main","kind":6,"uri":"file:///tmp/lsp_nav.am","range":{"start":{"line":10,"character":24},"end":{"line":10,"character":28}},"selectionRange":{"start":{"line":10,"character":24},"end":{"line":10,"character":28}},"data":{"name":"Main","uri":"file:///tmp/lsp_nav.am","line":10,"character":24}}}}'

lsp_callh_seq=$(lsp_frame "$lsp_init"; lsp_frame "$lsp_open_nav"; lsp_frame "$lsp_callhIn"; lsp_frame "$lsp_callhOut"; lsp_frame "$lsp_shut"; lsp_frame "$lsp_exit")
run_lsp_check "lsp: callh incoming Main"    '"name":"Main"'           "$lsp_callh_seq"
run_lsp_check "lsp: callh outgoing Greet"   '"name":"Greet"'          "$lsp_callh_seq"

# Phase D — package-install codeAction on Unknown symbol diagnostic.
# Fixture: bare `Window` reference (matches ui-sdl + ui-web in the
# curated index). The action's `command` field carries the
# `amc package add <pkg>` line the editor surfaces; we never auto-run.
lsp_open_pkg='{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"file:///tmp/lsp_pkg.am","languageId":"amalgame","version":1,"text":"class Program {\n    public static void Main() {\n        let w = Window\n    }\n}"}}}'
lsp_pkg_ca='{"jsonrpc":"2.0","id":30,"method":"textDocument/codeAction","params":{"textDocument":{"uri":"file:///tmp/lsp_pkg.am"},"range":{"start":{"line":2,"character":0},"end":{"line":2,"character":40}},"context":{"diagnostics":[]}}}'
lsp_pkg_seq=$(lsp_frame "$lsp_init"; lsp_frame "$lsp_open_pkg"; lsp_frame "$lsp_pkg_ca"; lsp_frame "$lsp_shut"; lsp_frame "$lsp_exit")
run_lsp_check "lsp: pkg-add suggestion"      '"command":"amc package add ui-'  "$lsp_pkg_seq"
run_lsp_check "lsp: pkg-add for-quoted-sym"  "(for 'Window')"                  "$lsp_pkg_seq"

# Import-line completion — when the cursor is on a line whose
# trimmed prefix is `import `, surface bundled stdlib namespaces
# (and any installed package namespaces from amalgame.lock).
lsp_open_imp='{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"file:///tmp/lsp_imp.am","languageId":"amalgame","version":1,"text":"import Amalgame.\nclass Program {\n    public static void Main() {}\n}"}}}'
lsp_imp_comp='{"jsonrpc":"2.0","id":40,"method":"textDocument/completion","params":{"textDocument":{"uri":"file:///tmp/lsp_imp.am"},"position":{"line":0,"character":16}}}'
lsp_imp_seq=$(lsp_frame "$lsp_init"; lsp_frame "$lsp_open_imp"; lsp_frame "$lsp_imp_comp"; lsp_frame "$lsp_shut"; lsp_frame "$lsp_exit")
run_lsp_check "lsp: import IO"          '"label":"Amalgame.IO","kind":9'                  "$lsp_imp_seq"
run_lsp_check "lsp: import Net"         '"label":"Amalgame.Net","kind":9'                 "$lsp_imp_seq"
run_lsp_check "lsp: import Formats.Json" '"label":"Amalgame.Formats.Json","kind":9'       "$lsp_imp_seq"
run_lsp_check "lsp: import bundled tag"  '"detail":"bundled stdlib"'                      "$lsp_imp_seq"

# ── amc dap ────────────────────────────────────────────
echo ""
echo "── amc dap ─────────────────────────────"

# Hidden self-test: exercises the gdb MI3 parser (src/dap/mi_parser.am)
# against canned inputs from the gdb manual. No gdb needed — the
# parser is pure AM. Exits 0 on all PASS, 1 on first FAIL.
printf "  %-34s" "dap: MI parser self-test"
DAP_MI_OUT="$("$AMC" dap --self-test-mi 2>&1)"
DAP_MI_RC=$?
DAP_MI_PASS=$(echo "$DAP_MI_OUT" | grep -c '^\[PASS\]')
DAP_MI_FAIL=$(echo "$DAP_MI_OUT" | grep -c '^\[FAIL\]')
if [ "$DAP_MI_RC" = "0" ] && [ "$DAP_MI_FAIL" = "0" ] && [ "$DAP_MI_PASS" -ge "8" ]; then
    echo -e "${GREEN}PASS${NC} ($DAP_MI_PASS cases)"
    PASS=$((PASS + 1))
else
    echo -e "${RED}FAIL${NC} (rc=$DAP_MI_RC pass=$DAP_MI_PASS fail=$DAP_MI_FAIL)"
    echo "$DAP_MI_OUT" | sed 's/^/      /'
    FAIL=$((FAIL + 1))
fi

# Bridge MVP test: handshake (initialize → initialized event →
# disconnect). Requires gdb on PATH; skips cleanly otherwise so
# CI runners without gdb installed don't fail. The bridge spawns
# gdb but doesn't actually attach to a program — initialize/
# disconnect only exercises the DAP framing + MI-pipe plumbing.
if command -v gdb >/dev/null 2>&1; then
    printf "  %-34s" "dap: bridge initialize handshake"
    DAP_INIT='{"seq":1,"type":"request","command":"initialize","arguments":{"clientID":"test","adapterID":"amc"}}'
    DAP_DISC='{"seq":2,"type":"request","command":"disconnect"}'
    DAP_BRIDGE_OUT="$( ( printf 'Content-Length: %d\r\n\r\n%s' "${#DAP_INIT}" "$DAP_INIT"
                         printf 'Content-Length: %d\r\n\r\n%s' "${#DAP_DISC}" "$DAP_DISC"
                       ) | timeout 5 "$AMC" dap --bridge 2>&1 )"
    if echo "$DAP_BRIDGE_OUT" | grep -qF '"command":"initialize","body":{' \
       && echo "$DAP_BRIDGE_OUT" | grep -qF '"event":"initialized"' \
       && echo "$DAP_BRIDGE_OUT" | grep -qF '"command":"disconnect"'; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC}"
        echo "$DAP_BRIDGE_OUT" | head -10 | sed 's/^/      /'
        FAIL=$((FAIL + 1))
    fi

    # Full session: compile a tiny C program with -g, drive it via
    # scripted DAP through the bridge, verify each phase fired its
    # response. Covers launch → setBreakpoints → configurationDone →
    # *stopped → stackTrace → variables → evaluate → continue →
    # terminated → disconnect.
    printf "  %-34s" "dap: bridge full session"
    DAP_PROG=$(mktemp --suffix=.c)
    DAP_BIN=$(mktemp -u)
    cat > "$DAP_PROG" << 'CEOF'
#include <stdio.h>
int main() {
    int x = 7;
    int y = 11;
    int sum = x + y;
    printf("%d\n", sum);
    return 0;
}
CEOF
    if ! gcc -g -O0 "$DAP_PROG" -o "$DAP_BIN" 2>/dev/null; then
        echo -e "${YELLOW}SKIP${NC} (gcc -g failed)"
        SKIP=$((SKIP + 1))
        rm -f "$DAP_PROG"
    else
        D_INIT='{"seq":1,"type":"request","command":"initialize","arguments":{"clientID":"test","adapterID":"amc"}}'
        D_LAUNCH='{"seq":2,"type":"request","command":"launch","arguments":{"program":"'"$DAP_BIN"'"}}'
        D_SETBP='{"seq":3,"type":"request","command":"setBreakpoints","arguments":{"source":{"path":"'"$DAP_PROG"'"},"breakpoints":[{"line":5}]}}'
        D_CFGDONE='{"seq":4,"type":"request","command":"configurationDone"}'
        D_ST='{"seq":5,"type":"request","command":"stackTrace","arguments":{"threadId":1}}'
        D_VARS='{"seq":6,"type":"request","command":"variables","arguments":{"variablesReference":1}}'
        D_EVAL='{"seq":7,"type":"request","command":"evaluate","arguments":{"expression":"x+y","frameId":0}}'
        D_CONT='{"seq":8,"type":"request","command":"continue"}'
        D_DISC='{"seq":9,"type":"request","command":"disconnect"}'
        DAP_SESS=$( (
            for F in "$D_INIT" "$D_LAUNCH" "$D_SETBP" "$D_CFGDONE"; do
                printf 'Content-Length: %d\r\n\r\n%s' "${#F}" "$F"
            done
            sleep 0.5
            for F in "$D_ST" "$D_VARS" "$D_EVAL"; do
                printf 'Content-Length: %d\r\n\r\n%s' "${#F}" "$F"
            done
            sleep 0.3
            printf 'Content-Length: %d\r\n\r\n%s' "${#D_CONT}" "$D_CONT"
            sleep 0.3
            printf 'Content-Length: %d\r\n\r\n%s' "${#D_DISC}" "$D_DISC"
        ) | timeout 8 "$AMC" dap --bridge 2>&1 )
        # Strip the file path from setBp output so the test isn't
        # fragile against tmpfile naming.
        if echo "$DAP_SESS" | grep -qF '"command":"launch"' \
           && echo "$DAP_SESS" | grep -qF '"command":"setBreakpoints"' \
           && echo "$DAP_SESS" | grep -qF '"verified":true' \
           && echo "$DAP_SESS" | grep -qF '"event":"stopped"' \
           && echo "$DAP_SESS" | grep -qF '"reason":"breakpoint"' \
           && echo "$DAP_SESS" | grep -qF '"command":"stackTrace"' \
           && echo "$DAP_SESS" | grep -qF '"command":"variables"' \
           && echo "$DAP_SESS" | grep -qF '"name":"x"' \
           && echo "$DAP_SESS" | grep -qF '"value":"7"' \
           && echo "$DAP_SESS" | grep -qF '"command":"evaluate"' \
           && echo "$DAP_SESS" | grep -qF '"result":"18"' \
           && echo "$DAP_SESS" | grep -qF '"event":"terminated"' \
           && echo "$DAP_SESS" | grep -qF '"command":"disconnect"'; then
            echo -e "${GREEN}PASS${NC}"
            PASS=$((PASS + 1))
        else
            echo -e "${RED}FAIL${NC}"
            echo "$DAP_SESS" | head -20 | sed 's/^/      /'
            FAIL=$((FAIL + 1))
        fi
        rm -f "$DAP_PROG" "$DAP_BIN"
    fi
else
    printf "  %-34s" "dap: bridge initialize handshake"
    echo -e "${YELLOW}SKIP${NC} (gdb not on PATH)"
    SKIP=$((SKIP + 1))
    printf "  %-34s" "dap: bridge full session"
    echo -e "${YELLOW}SKIP${NC} (gdb not on PATH)"
    SKIP=$((SKIP + 1))
fi

# ── amc migrate ────────────────────────────────────────
echo ""
echo "── amc migrate ─────────────────────────"

# Run `amc migrate <file> --prompt-only` and grep the assembled prompt
# for an expected substring. --prompt-only short-circuits before any
# claude CLI call so the test is hermetic and fast.
run_migrate_prompt_check() {
    local name="$1"
    local file="$2"
    local pattern="$3"

    printf "  %-34s" "$name"
    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        SKIP=$((SKIP + 1)); return
    fi
    local out
    out=$("$AMC" migrate "$file" --prompt-only 2>&1)
    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found)"
        echo "    looking for: $pattern"
        FAIL=$((FAIL + 1))
    fi
}

# Tiny TS fixture for prompt-only migration tests. Created at runtime
# in $BUILD_DIR so the source tree stays clean.
mig_fixture_ts="$BUILD_DIR/mig_fixture.ts"
cat > "$mig_fixture_ts" <<'EOF'
class User { constructor(public name: string, public age: number) {} }
const u = new User("Alice", 30);
console.log(u.name);
EOF

run_migrate_prompt_check "migrate: language detected"  "$mig_fixture_ts"  "translating TypeScript source"
run_migrate_prompt_check "migrate: source embedded"    "$mig_fixture_ts"  'class User {'
run_migrate_prompt_check "migrate: TODO marker hint"   "$mig_fixture_ts"  "TODO[migrate]"
run_migrate_prompt_check "migrate: convention listed"  "$mig_fixture_ts"  "data class"
run_migrate_prompt_check "migrate: limitation listed"  "$mig_fixture_ts"  "String interpolation does NOT propagate"

# --dry-run path: hermetic too (no claude call), tests language
# detection + output-path defaulting.
run_migrate_dry_check() {
    local name="$1"
    local pattern="$2"

    printf "  %-34s" "$name"
    local out
    out=$("$AMC" migrate "$mig_fixture_ts" --dry-run 2>&1)
    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found)"
        echo "    looking for: $pattern"
        FAIL=$((FAIL + 1))
    fi
}

run_migrate_dry_check "migrate: dry-run lang"     "TypeScript"
run_migrate_dry_check "migrate: dry-run out"      "mig_fixture.am"
run_migrate_dry_check "migrate: dry-run provider" "provider:      claude"

# --help and -h: print usage to stderr and exit 0.
run_migrate_help_check() {
    local name="$1"
    local flag="$2"
    local pattern="$3"

    printf "  %-34s" "$name"
    local out
    out=$("$AMC" migrate "$flag" 2>&1)
    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found)"
        echo "    looking for: $pattern"
        FAIL=$((FAIL + 1))
    fi
}

run_migrate_help_check "migrate: --help" "--help" "Usage: amc migrate"
run_migrate_help_check "migrate: -h"     "-h"     "Usage: amc migrate"

# Directory recursion (v1.1): create a fixture dir with mixed-language
# sources + non-source noise, then assert --dry-run discovers + reports
# the right files.
mig_fixture_dir="$BUILD_DIR/mig_fixture_dir"
mkdir -p "$mig_fixture_dir"
cat > "$mig_fixture_dir/u1.ts" <<'EOF'
class A { x: number = 0 }
EOF
cat > "$mig_fixture_dir/u2.py" <<'EOF'
class B:
    def __init__(self, x: int): self.x = x
EOF
cat > "$mig_fixture_dir/README.md" <<'EOF'
# Not a source file
EOF
cat > "$mig_fixture_dir/already.am" <<'EOF'
namespace Y
EOF

run_migrate_dir_check() {
    local name="$1"
    local pattern="$2"

    printf "  %-34s" "$name"
    local out
    out=$("$AMC" migrate "$mig_fixture_dir" --dry-run 2>&1)
    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found)"
        echo "    looking for: $pattern"
        FAIL=$((FAIL + 1))
    fi
}

run_migrate_dir_check "migrate: dir discovers"      "found 2 file(s) to migrate"
run_migrate_dir_check "migrate: dir picks ts"       "u1.ts (TypeScript"
run_migrate_dir_check "migrate: dir picks py"       "u2.py (Python"
run_migrate_dir_check "migrate: dir summary"        "2/2 succeeded, 0 failed"

# --output is rejected in directory mode.
run_migrate_dir_output_check() {
    local name="$1"
    local pattern="$2"

    printf "  %-34s" "$name"
    local out
    out=$("$AMC" migrate "$mig_fixture_dir" --output /tmp/x.am --dry-run 2>&1)
    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found)"
        echo "    looking for: $pattern"
        FAIL=$((FAIL + 1))
    fi
}

run_migrate_dir_output_check "migrate: dir rejects --output" "cannot be used with directory"

# claude-api provider: test the failure paths hermetically (no real
# HTTP call to api.anthropic.com — that would cost money and be flaky
# in CI). The success path is implicitly covered by the runtime
# linkage (Http_PostWithHeaders is callable, see PR #160 infra).
run_migrate_provider_check() {
    local name="$1"
    local pattern="$2"
    shift 2

    printf "  %-34s" "$name"
    local out
    out=$("$AMC" migrate "$mig_fixture_ts" "$@" 2>&1)
    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found)"
        echo "    looking for: $pattern"
        FAIL=$((FAIL + 1))
    fi
}

# --provider claude-api without ANTHROPIC_API_KEY → clean error.
unset ANTHROPIC_API_KEY
run_migrate_provider_check "migrate: claude-api no key"  "ANTHROPIC_API_KEY not set"  --provider claude-api
# --provider with an unknown name → clean error.
run_migrate_provider_check "migrate: unknown provider"   "not supported (built-in"     --provider gemini-7
# --help mentions the new claude-api provider so users discover it.
run_migrate_help_check "migrate: --help mentions claude-api" "--help" "claude-api"

# v1.3: prompt loaded from disk. When the docs are reachable, the
# system prompt embeds the EBNF grammar + the language tour.
run_migrate_prompt_check "migrate: prompt embeds grammar"   "$mig_fixture_ts"  "Amalgame grammar (EBNF)"
run_migrate_prompt_check "migrate: prompt embeds tour"      "$mig_fixture_ts"  "Amalgame language tour"

# ── amc generate ────────────────────────────────────────
echo ""
echo "── amc generate ────────────────────────"

run_generate_check() {
    local name="$1"
    local pattern="$2"
    shift 2

    printf "  %-34s" "$name"
    local out
    out=$("$AMC" generate "$@" 2>&1)
    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found)"
        echo "    looking for: $pattern"
        FAIL=$((FAIL + 1))
    fi
}

# All hermetic — no real LLM calls.
run_generate_check "generate: --help"           "Usage: amc generate"      --help
run_generate_check "generate: dry-run"          "would generate from"      "test prompt" --dry-run
run_generate_check "generate: dry-run provider" "provider:      claude"    "test"        --dry-run
run_generate_check "generate: prompt-only sys"  "writing an Amalgame"      "test"        --prompt-only
run_generate_check "generate: prompt-only task" "## Task"                  "fizzbuzz"    --prompt-only
run_generate_check "generate: prompt embeds grammar" "Amalgame grammar (EBNF)" "test"    --prompt-only
unset ANTHROPIC_API_KEY
run_generate_check "generate: claude-api no key" "ANTHROPIC_API_KEY not set" "test" --provider claude-api
run_generate_check "generate: unknown provider" "not supported (built-in"  "test"        --provider gemini-x
run_generate_check "generate: no prompt"        "no prompt given"

# ── amc explain ─────────────────────────────────────────
echo ""
echo "── amc explain ─────────────────────────"

run_explain_check() {
    local name="$1"
    local pattern="$2"
    shift 2

    printf "  %-34s" "$name"
    local out
    out=$("$AMC" explain "$@" 2>&1)
    if echo "$out" | grep -qF "$pattern"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (pattern not found)"
        echo "    looking for: $pattern"
        FAIL=$((FAIL + 1))
    fi
}

# A small Amalgame fixture for explain tests.
explain_fixture="$BUILD_DIR/explain_fixture.am"
cat > "$explain_fixture" <<'EOF'
namespace Demo
public class Program {
    public static void Main(string[] args) { Console.WriteLine("hi") }
}
EOF

run_explain_check "explain: --help"           "Usage: amc explain"      --help
run_explain_check "explain: dry-run"          "would explain"           "$explain_fixture" --dry-run
run_explain_check "explain: dry-run lang"     "output lang:   English"  "$explain_fixture" --dry-run
run_explain_check "explain: lang override"    "output lang:   French"   "$explain_fixture" --dry-run --lang French
run_explain_check "explain: prompt-only sys"  "explaining Amalgame"     "$explain_fixture" --prompt-only
run_explain_check "explain: prompt-only src"  "Amalgame source: "       "$explain_fixture" --prompt-only
run_explain_check "explain: file not found"   "file not found"          "/nonexistent/file.am"
run_explain_check "explain: no input"         "no input file"

# v2 providers (chatgpt / gemini / custom): hermetic — no real
# API call. Just exercise the dispatch table + missing-key paths.
unset ANTHROPIC_API_KEY OPENAI_API_KEY GEMINI_API_KEY AMC_CUSTOM_PROVIDER_CMD

run_generate_check "v2: chatgpt no key"   "OPENAI_API_KEY not set"      "test"  --provider chatgpt
run_generate_check "v2: gemini no key"    "GEMINI_API_KEY not set"      "test"  --provider gemini
run_generate_check "v2: custom no cmd"    "AMC_CUSTOM_PROVIDER_CMD"     "test"  --provider custom

# Auto-selection: when OPENAI_API_KEY is set, chatgpt becomes the
# default. Same shape for gemini.
export OPENAI_API_KEY=fake
run_generate_check "v2: auto-select chatgpt" "provider:      chatgpt" "test"  --dry-run
unset OPENAI_API_KEY
export GEMINI_API_KEY=fake
run_generate_check "v2: auto-select gemini"  "provider:      gemini"  "test"  --dry-run
unset GEMINI_API_KEY

# --help advertises the new providers.
run_generate_check "v2: --help mentions chatgpt" "chatgpt"  --help
run_generate_check "v2: --help mentions gemini"  "gemini"   --help
run_generate_check "v2: --help mentions custom"  "custom"   --help

# Cost estimation in --dry-run.
run_generate_check "cost: claude free"             "free (subscription"     "test"  --dry-run
export OPENAI_API_KEY=fake
run_generate_check "cost: chatgpt has tokens"      "in + ~1000 out"         "test"  --dry-run
run_generate_check "cost: chatgpt opus model dollars" "$"                   "test"  --dry-run --model gpt-4o
unset OPENAI_API_KEY
export ANTHROPIC_API_KEY=fake
run_generate_check "cost: claude-api opus mentions opus" "claude-opus-4-7" "test" --dry-run --model claude-opus-4-7
unset ANTHROPIC_API_KEY
export GEMINI_API_KEY=fake
run_generate_check "cost: gemini default flash"    "gemini-1.5-flash"       "test"  --dry-run
unset GEMINI_API_KEY

# --stream constraints (hermetic — doesn't actually call claude).
run_generate_check "stream: requires CLI provider" "requires --provider claude" "test"  --stream --provider claude-api
run_generate_check "stream: incompat with -o"      "incompatible with -o"       "test"  --stream -o /tmp/x.am
run_generate_check "stream: --help mentions"       "Stream the LLM response"  --help
run_explain_check  "stream: explain requires CLI"  "requires --provider claude" "$explain_fixture" --stream --provider claude-api
run_explain_check  "stream: explain --help"        "Stream the LLM response"  --help

# Result cache (--no-cache flag).
run_migrate_help_check "cache: --help mentions"  "--help"  "Skip the on-disk result cache"
run_migrate_provider_check "cache: --no-cache accepted" "claude-api: ANTHROPIC_API_KEY not set" --provider claude-api --no-cache

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
run_lib_link_test "lib end-to-end (link + run)" \
                  "$SAMPLES/lib_e2e.am" \
                  "$SAMPLES/lib_e2e_consumer.c" \
                  "add=15 mul=30"

# ── Interfaces ──────────────────────────────────────────
echo ""
echo "── Interfaces ──────────────────────────"
run_test "interface basic"   "$SAMPLES/interfaces.am"   "Circle(r=5)"
run_test "interface method"  "$SAMPLES/interfaces.am"   "Rect(4x3)"
run_test "interface scale"   "$SAMPLES/interfaces.am"   "Circle(r=10)"
run_test "interface dispatch" "$SAMPLES/interfaces.am"  "Circle(r=10)"
run_test "generic iface T"   "$SAMPLES/generic_interfaces.am" "compare: 4"
run_test "generic iface K,V" "$SAMPLES/generic_interfaces.am" "pair: answer=42"
run_check_fail "generic iface bad sig" "$SAMPLES/generic_interfaces_bad.am" \
    "expected type 'int' (from interface 'IComparable<int>'), got 'string'"

# ── Enums ──────────────────────────────────────────────
echo ""
echo "── Enums ───────────────────────────────"
run_test "enum basic"        "$SAMPLES/enums.am"        "Direction: North"
run_test "enum match"        "$SAMPLES/enums.am"        "Summer warm: true"
run_test "enum comparison"   "$SAMPLES/enums.am"        "isNorth: true"
run_test "enum all values"   "$SAMPLES/enums.am"        "East: East"

# ── Extended coverage ──────────────────────────────────
echo ""
echo "── Extended coverage ───────────────────"
run_test "operators"         "$SAMPLES/operators.am"        "add: 13"
run_test "logical ops"       "$SAMPLES/operators.am"        "and: false"
run_test "strings concat"    "$SAMPLES/strings.am"          "Hello, World!"
run_test "string interp"     "$SAMPLES/strings.am"          "Product: 42"
run_test "str inst length"   "$SAMPLES/string_methods.am"   "len: 12"
run_test "str inst toupper"  "$SAMPLES/string_methods.am"   "upper: HELLO, WORLD"
run_test "str inst trim"     "$SAMPLES/string_methods.am"   "trimmed: 'spaced'"
run_test "str inst replace"  "$SAMPLES/string_methods.am"   "replaced: Hello, Amalgame"
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

# ── Tuples ─────────────────────────────────────────────
echo ""
echo "── Tuples ──────────────────────────────"
run_test "tuple: basic"       "$SAMPLES/tuples.am"  "name: Arthus"
run_test "tuple: level"       "$SAMPLES/tuples.am"  "level: 42"
run_test "tuple: 3-tuple ok"  "$SAMPLES/tuples.am"  "ok: true"
run_test "tuple: quotient"    "$SAMPLES/tuples.am"  "quotient: 3"
run_test "tuple: remainder"   "$SAMPLES/tuples.am"  "remainder: 2"
run_test "tuple: div zero"    "$SAMPLES/tuples.am"  "div ok: false"

# ── Multiline strings ──────────────────────────────────
echo ""
echo "── Multiline strings ───────────────────"
run_test "multiline basic"    "$SAMPLES/multiline_string.am"  "lines: 2"
run_test "multiline interp"   "$SAMPLES/multiline_string.am"  "card ok: Arthus"
run_test "multiline dedent"   "$SAMPLES/multiline_string.am"  "sql ok"
run_test "multiline single"   "$SAMPLES/multiline_string.am"  "single: Hello World"

# ── Try/catch/throw ────────────────────────────────────
echo ""
echo "── Try/catch/throw ─────────────────────"
run_test "try: normal flow"   "$SAMPLES/try_catch.am"  "result: 5"
run_test "try: catch throw"   "$SAMPLES/try_catch.am"  "caught: division by zero"
run_test "try: finally"       "$SAMPLES/try_catch.am"  "finally runs"
run_test "try: done"          "$SAMPLES/try_catch.am"  "done"

# ── Linter ────────────────────────────────────────────
echo ""
echo "── Linter (amc --lint) ─────────────────"
run_lint_check "lint: dead code"     "$SAMPLES/lint_test.am"           "unreachable code after"
run_lint_check "lint: unused local"  "$SAMPLES/lint_unused_shadow.am"  "unused local 'dropped'"
run_lint_check "lint: shadow"        "$SAMPLES/lint_unused_shadow.am"  "'n' shadows an enclosing binding"
run_lint_check "lint: loop binder"   "$SAMPLES/lint_unused_shadow.am"  "unused local 'tick'"

# ── If expression ─────────────────────────────────────
echo ""
echo "── If expression ───────────────────────"
run_test "if-expr basic"    "$SAMPLES/if_expr.am"  "label: big"
run_test "if-expr else-if"  "$SAMPLES/if_expr.am"  "grade: C"
run_test "if-expr numeric"  "$SAMPLES/if_expr.am"  "bigger: 10"
run_test "if-expr bool"     "$SAMPLES/if_expr.am"  "adult: false"

# ── For-in / Foreach ──────────────────────────────────
echo ""
echo "── For-in ──────────────────────────────"
run_test "for-in range"        "$SAMPLES/foreach.am"  "sum 0..5: 10"
run_test "for-in list"         "$SAMPLES/foreach.am"  "list total: 3"
# `for i, item in list` (with index) and `for ch in "string"` (string chars)
# are not yet supported by the self-hosted compiler. Tracked separately.

# ── Multi-file ─────────────────────────────────────────
run_multifile_test() {
    local name="$1"
    local expected="$2"
    shift 2
    local files=("$@")

    printf "  %-34s" "$name"

    # Match the single-file convention: pass `-o <base>` (no .c) and let amc
    # emit `<base>.c`, then call gcc manually. The previous version passed
    # `-o foo.c` (which amc dutifully expanded to foo.c.c) and expected an
    # executable amc never produces.
    local out_base="/tmp/amalgame_multi_test"
    output=$("$AMC" "${files[@]}" -o "$out_base" 2>&1)
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
    gcc -O2 -Iruntime "$c_file" -lgc -lm -lz -o "$out_base" 2>/dev/null

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

    if echo "$run_output" | grep -qF "$expected"; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (output mismatch)"
        echo "    expected : $expected"
        echo "    got      : $(echo "$run_output" | head -3 | tr '\n' '|')"
        FAIL=$((FAIL + 1))
    fi
}

echo ""
echo "── Multi-file ──────────────────────────"
MF="$SAMPLES/multifile"
run_multifile_test "multifile: player status" \
    "Arthus HP=100 Lvl=42" \
    "$MF/models.am" "$MF/utils.am" "$MF/main.am"
run_multifile_test "multifile: logger" \
    "[LOG] Game started" \
    "$MF/models.am" "$MF/utils.am" "$MF/main.am"
run_multifile_test "multifile: cross-file clamp" \
    "Clamped: 100" \
    "$MF/models.am" "$MF/utils.am" "$MF/main.am"
run_multifile_test "multifile: enemy" \
    "Enemy: Dragon" \
    "$MF/models.am" "$MF/utils.am" "$MF/main.am"

# ── Summary ────────────────────────────────────────────
echo ""
echo "───────────────────────────────────────"
echo -e "  ${GREEN}PASS: $PASS${NC}  |  ${RED}FAIL: $FAIL${NC}  |  ${YELLOW}SKIP: $SKIP${NC}"
echo "───────────────────────────────────────"
echo ""

[ $FAIL -eq 0 ] && exit 0 || exit 1
