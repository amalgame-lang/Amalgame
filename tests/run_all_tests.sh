#!/bin/bash
# ─────────────────────────────────────────────────────
#  Amalgame — Full Test Suite (transitional wrapper)
#  Usage: ./tests/run_all_tests.sh
#  Used by CI/CD before releases.
#
#  Since 2026-05-22 the canonical test entry point is the AM bundles:
#      ./amc test ./tests/fmt/
#      ./amc test ./tests/amc_new/
#      ./amc test ./tests/stdlib_bundle/
#      ./amc test ./tests/core_bundle/
#
#  This wrapper lances BOTH the legacy bash runners AND the AM bundles
#  in parallel to verify zero-delta during the migration. Once the AM
#  bundles ship a few stable releases (and the 4 fixture *_test.am
#  files move out of the auto-discovery path), the bash runners get
#  dropped and this wrapper collapses to `./amc test ./tests/`.
# ─────────────────────────────────────────────────────

set -e

DIR="$(cd "$(dirname "$0")" && pwd)"

echo ""
echo "╔═══════════════════════════════════════════╗"
echo "║   Amalgame — Full Test Suite              ║"
echo "╚═══════════════════════════════════════════╝"

# Core tests
"$DIR/run_tests.sh"
CORE_EXIT=$?

echo ""
echo "─────────────────────────────────────────────"
echo ""

# Stdlib tests
"$DIR/run_stdlib_tests.sh"
STDLIB_EXIT=$?

echo ""
echo "─────────────────────────────────────────────"
echo ""

# Formatter tests (uses self-hosted ./amc)
"$DIR/run_fmt_tests.sh"
FMT_EXIT=$?

echo ""
echo "─────────────────────────────────────────────"
echo ""

# `amc new` scaffolding tests
"$DIR/run_amc_new_tests.sh"
NEW_EXIT=$?

echo ""
echo "─────────────────────────────────────────────"
echo ""

# AM-side test bundles (`amc test` driver). Safety net during the
# bash-runner migration: each bash runner above has an `*_test.am`
# equivalent below. Both run until the migration is complete and
# verified across multiple releases — then the bash runner is dropped.
echo "── AM test bundles ─────────────────────────"
cd "$DIR/.."
AM_EXIT=0
for bundle in tests/fmt tests/amc_new tests/stdlib_bundle tests/core_bundle; do
    echo ""
    echo "  ┄ $bundle ┄"
    ./amc test "./$bundle/" || AM_EXIT=$?
done

echo ""
if [ $CORE_EXIT -eq 0 ] && [ $STDLIB_EXIT -eq 0 ] && [ $FMT_EXIT -eq 0 ] && [ $NEW_EXIT -eq 0 ] && [ $AM_EXIT -eq 0 ]; then
    echo -e "\033[0;32m  All suites passed ✓\033[0m"
else
    echo -e "\033[0;31m  Some tests failed ✗\033[0m"
    [ $CORE_EXIT -ne 0 ]   && echo "  - Core tests failed"
    [ $STDLIB_EXIT -ne 0 ] && echo "  - Stdlib tests failed"
    [ $FMT_EXIT -ne 0 ]    && echo "  - Formatter tests failed"
    [ $NEW_EXIT -ne 0 ]    && echo "  - amc new tests failed"
    [ $AM_EXIT -ne 0 ]     && echo "  - AM test bundles failed"
fi
echo ""

[ $CORE_EXIT -eq 0 ] && [ $STDLIB_EXIT -eq 0 ] && [ $FMT_EXIT -eq 0 ] && [ $NEW_EXIT -eq 0 ] && [ $AM_EXIT -eq 0 ] && exit 0 || exit 1
