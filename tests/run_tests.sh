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

# Samples that the self-hosted ./amc can't yet compile due to bugs tracked
# separately (algebraic enum methods, tuple destructure typing, try/catch
# binder scoping, advanced pattern matching, null-safety inference). They
# pass under the Vala bootstrap (./build/amc) — see run_tests_vala.sh if
# you need full coverage during recovery work. PRs welcome.
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

    # The self-hosted amc only emits a .c file; the Vala bootstrap used to
    # auto-compile, so call gcc explicitly here to keep behaviour identical.
    local c_file="${out_base}.c"
    if [ ! -f "$c_file" ]; then
        echo -e "${RED}FAIL${NC} (no .c emitted)"
        FAIL=$((FAIL + 1)); return
    fi
    gcc -O2 -Iruntime "$c_file" -lgc -lm -lcurl -o "$out_base" 2>/dev/null

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
# Uses the bootstrap ./amc because the legacy Vala compiler still emits public methods
# with `static` forward declarations, which gives them internal linkage.
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
    if ! gcc -I"$runtime_dir" "$consumer_c" "$tmpdir/lib.o" -lgc -lm -lcurl -o "$tmpdir/app" 2>"$tmpdir/err"; then
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
run_test "auto-qualify: ctor write"  "$SAMPLES/cgen_auto_qualify.am" "value: 7"
run_test "auto-qualify: method read" "$SAMPLES/cgen_auto_qualify.am" "dp: 17"
run_test "auto-qualify: method write" "$SAMPLES/cgen_auto_qualify.am" "after reset value: 0"
run_test "auto-qualify: explicit this. mix" "$SAMPLES/cgen_auto_qualify.am" "after reset label: reset"

# ── Process module ─────────────────────────────────────
echo ""
echo "── Process ─────────────────────────────"
run_test "process: run exit"      "$SAMPLES/process_api.am"  "exit=0"
run_test "process: cap exit"      "$SAMPLES/process_api.am"  "cap.exit=0"
run_test "process: cap stdout"    "$SAMPLES/process_api.am"  "cap.out=captured-line"
run_test "process: nonzero exit"  "$SAMPLES/process_api.am"  "bad.exit=1"
run_test "process: stderr merge"  "$SAMPLES/process_api.am"  "merged.out=stderr-bytes"

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

run_lsp_check "lsp: initialize reply"   '"capabilities":{"textDocumentSync":1,"hoverProvider":true,"completionProvider":{"triggerCharacters":["."]}}' "$lsp_seq"
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
    gcc -O2 -Iruntime "$c_file" -lgc -lm -lcurl -o "$out_base" 2>/dev/null

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
