#!/bin/bash
# Smoke tests for `amc new`. Scaffolds each template into /tmp,
# checks the expected files exist, and (for exe) compiles + runs
# the result so a regression in the generated source breaks the
# CI run, not just the user's first day.

set -u

AMC="${AMC:-./amc}"
TMP=$(mktemp -d -t amc-new-XXXXXX)
trap 'rm -rf "$TMP"' EXIT

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

PASS=0
FAIL=0

assert_file() {
    if [ -f "$1" ]; then
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}FAIL${NC} missing file: $1"
        FAIL=$((FAIL + 1))
    fi
}

assert_dir() {
    if [ -d "$1" ]; then
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}FAIL${NC} missing dir: $1"
        FAIL=$((FAIL + 1))
    fi
}

echo "── amc new ─────────────────────────────────"

# ── exe template ──────────────────────────────
"$AMC" new "$TMP/exetest" > /dev/null 2>&1
assert_dir  "$TMP/exetest"
assert_dir  "$TMP/exetest/src"
assert_dir  "$TMP/exetest/tests"
assert_file "$TMP/exetest/src/main.am"
assert_file "$TMP/exetest/tests/hello_test.am"
assert_file "$TMP/exetest/build.sh"
assert_file "$TMP/exetest/README.md"
assert_file "$TMP/exetest/.gitignore"

# Compile + run the scaffolded exe so regressions in MainAmExe
# show up here rather than at first user invocation. amc emits the
# .c bundle; gcc links it (mirrors what the generated build.sh does).
"$AMC" "$TMP/exetest/src/main.am" -o "$TMP/exetest/exetest" > /dev/null 2>&1
if [ -f "$TMP/exetest/exetest.c" ]; then
    gcc -O2 -Iruntime "$TMP/exetest/exetest.c" -lgc -lm -lcurl -o "$TMP/exetest/exetest" 2>/dev/null
    if [ -x "$TMP/exetest/exetest" ]; then
        output=$("$TMP/exetest/exetest" 2>&1)
        if echo "$output" | grep -q "Hello from exetest"; then
            PASS=$((PASS + 1))
        else
            echo -e "  ${RED}FAIL${NC} scaffolded exe ran but did not greet (got: $output)"
            FAIL=$((FAIL + 1))
        fi
    else
        echo -e "  ${RED}FAIL${NC} scaffolded exe.c did not link"
        FAIL=$((FAIL + 1))
    fi
else
    echo -e "  ${RED}FAIL${NC} scaffolded exe did not compile to .c"
    FAIL=$((FAIL + 1))
fi

# ── lib template ──────────────────────────────
"$AMC" new "$TMP/libtest" --template lib > /dev/null 2>&1
assert_dir  "$TMP/libtest/src"
assert_file "$TMP/libtest/src/libtest.am"
assert_file "$TMP/libtest/build.sh"
assert_file "$TMP/libtest/README.md"

# ── test template ─────────────────────────────
"$AMC" new "$TMP/onlytest" --template test > /dev/null 2>&1
assert_dir  "$TMP/onlytest/tests"
assert_file "$TMP/onlytest/tests/onlytest_test.am"
assert_file "$TMP/onlytest/README.md"

# ── service template ──────────────────────────
# Linux side (systemd unit + .sh) + Windows side (NSSM scripts).
# Both ship side-by-side; the README explains which to use.
"$AMC" new "$TMP/myd" --template service > /dev/null 2>&1
assert_dir  "$TMP/myd/src"
assert_file "$TMP/myd/src/main.am"
assert_file "$TMP/myd/myd.service"
assert_file "$TMP/myd/install.sh"
assert_file "$TMP/myd/build.sh"
assert_file "$TMP/myd/install.ps1"
assert_file "$TMP/myd/build.ps1"
assert_file "$TMP/myd/.gitignore"
assert_file "$TMP/myd/README.md"
# Spot-check the generated main.am uses the documented APIs.
grep -q "Service.Install"      "$TMP/myd/src/main.am" && PASS=$((PASS + 1)) || { echo -e "  ${RED}FAIL${NC} service template missing Service.Install"; FAIL=$((FAIL + 1)); }
grep -q "while.*ShouldStop"    "$TMP/myd/src/main.am" && PASS=$((PASS + 1)) || { echo -e "  ${RED}FAIL${NC} service template missing ShouldStop loop"; FAIL=$((FAIL + 1)); }
grep -q "Log.Info"             "$TMP/myd/src/main.am" && PASS=$((PASS + 1)) || { echo -e "  ${RED}FAIL${NC} service template missing Log.Info"; FAIL=$((FAIL + 1)); }
# Spot-check the systemd unit + install scripts have the right name baked in.
grep -q "ExecStart=/usr/local/bin/myd"  "$TMP/myd/myd.service" && PASS=$((PASS + 1)) || { echo -e "  ${RED}FAIL${NC} systemd unit missing ExecStart"; FAIL=$((FAIL + 1)); }
grep -q "nssm"                          "$TMP/myd/install.ps1" && PASS=$((PASS + 1)) || { echo -e "  ${RED}FAIL${NC} install.ps1 missing NSSM"; FAIL=$((FAIL + 1)); }

# ── Refusal without --force on existing dir ───
"$AMC" new "$TMP/exetest" 2>&1 | grep -qi "already exists" && PASS=$((PASS + 1)) || {
    echo -e "  ${RED}FAIL${NC} expected 'already exists' refusal"
    FAIL=$((FAIL + 1))
}

# ── --force overwrites ────────────────────────
"$AMC" new "$TMP/exetest" --force > /dev/null 2>&1
if [ $? -eq 0 ]; then
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}FAIL${NC} --force did not succeed on existing dir"
    FAIL=$((FAIL + 1))
fi

# ── Bad name rejected ─────────────────────────
# Use a name with a forbidden character in the basename (`@`).
# Slashes are fine — they're treated as path separators, with
# IsSafeName checking only the last segment.
"$AMC" new "$TMP/bad@name" 2>&1 | grep -qi "not a safe project name" && PASS=$((PASS + 1)) || {
    echo -e "  ${RED}FAIL${NC} bad name was not rejected"
    FAIL=$((FAIL + 1))
}

# ── Missing name rejected ─────────────────────
"$AMC" new 2>&1 | grep -qi "missing <name>" && PASS=$((PASS + 1)) || {
    echo -e "  ${RED}FAIL${NC} missing-name not rejected"
    FAIL=$((FAIL + 1))
}

echo ""
echo "────────────────────────────────────────────"
echo -e "  ${GREEN}PASS: $PASS${NC}  |  ${RED}FAIL: $FAIL${NC}"
echo "────────────────────────────────────────────"

[ $FAIL -eq 0 ] && exit 0 || exit 1
