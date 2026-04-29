#!/bin/bash
# ─────────────────────────────────────────────────────
#  Amalgame — Full Test Suite (Core + Stdlib)
#  Usage: ./tests/run_all_tests.sh
#  Used by CI/CD before releases.
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
if [ $CORE_EXIT -eq 0 ] && [ $STDLIB_EXIT -eq 0 ]; then
    echo -e "\033[0;32m  All suites passed ✓\033[0m"
else
    echo -e "\033[0;31m  Some tests failed ✗\033[0m"
    [ $CORE_EXIT -ne 0 ]   && echo "  - Core tests failed"
    [ $STDLIB_EXIT -ne 0 ] && echo "  - Stdlib tests failed"
fi
echo ""

[ $CORE_EXIT -eq 0 ] && [ $STDLIB_EXIT -eq 0 ] && exit 0 || exit 1
