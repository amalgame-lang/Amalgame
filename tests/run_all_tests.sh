#!/bin/bash
# ─────────────────────────────────────────────────────
#  Amalgame — Full Test Suite
#  Usage: ./tests/run_all_tests.sh
#  Used by CI/CD before releases.
#
#  Since 2026-05-24 this is a thin wrapper around `amc test ./tests/`.
#  The legacy bash runners (run_tests.sh / run_stdlib_tests.sh /
#  run_fmt_tests.sh / run_amc_new_tests.sh) were dropped after the
#  AM bundles shipped a parity safety-net period — see
#  ROADMAP_COMPLET.md for the migration history.
#
#  Bundles auto-discovered by `amc test`:
#    tests/fmt/fmt_test.am               # formatter idempotence
#    tests/amc_new/amc_new_test.am       # `amc new` scaffolders
#    tests/stdlib_bundle/stdlib_test.am  # IO / String / Json / Toml / …
#    tests/core_bundle/core_test.am      # lexer / parser / cgen / LSP / DAP / LLM
#
#  Fixture dirs (`tests/fixtures/`) are pruned from discovery so the
#  LSP workspace fixture and the test-runner self-test fixtures don't
#  get auto-executed when crawling from `./tests/`.
# ─────────────────────────────────────────────────────

set -e
cd "$(dirname "$0")/.."

if [ ! -x ./amc ]; then
    echo "ERROR: ./amc not found. Run ./build_amc.sh first." >&2
    exit 1
fi

echo ""
echo "╔═══════════════════════════════════════════╗"
echo "║   Amalgame — Full Test Suite              ║"
echo "╚═══════════════════════════════════════════╝"

./amc test ./tests/
