#!/bin/bash
# ─────────────────────────────────────────────────────
#  tools/save-snapshot.sh — capture a known-good amc
# ─────────────────────────────────────────────────────
#
# Saves the current amc as a fallback for when self-host bootstrap
# is broken (e.g. introducing a new syntax that the running amc
# doesn't understand). The snapshot is the sole bootstrap rung
# below ./amc:
#
#   ./amc                  ← current self-hosted (may be broken)
#   ./snapshot/amc         ← last known-good Amalgame (this script)
#
# From a clean clone, snapshot/amc is rebuildable in one gcc step
# from the tracked snapshot/amc_lib.c — see snapshot/INFO.md.
#
# Usage:
#   ./tools/save-snapshot.sh            # validate + save snapshot
#   ./tools/save-snapshot.sh --skip-tests   # skip the test suite (dangerous)
#
# The snapshot consists of:
#   snapshot/amc_lib.c    — portable, recompilable on any platform with gcc
#   snapshot/amc          — Linux binary (gitignored, locally rebuildable)
#   snapshot/INFO.md      — provenance: git rev, date, test count

set -e
cd "$(dirname "$0")/.."

SKIP_TESTS=false
[ "${1:-}" = "--skip-tests" ] && SKIP_TESTS=true

echo "── Snapshot ────────────────────────────────"

# Sanity: amc_lib.c must exist (build_amc.sh produces it).
if [ ! -f src/amc_lib.c ]; then
    echo "ERROR: src/amc_lib.c not found. Run ./build_amc.sh first." >&2
    exit 1
fi

# Sanity: ./amc must run.
if [ ! -x ./amc ]; then
    echo "ERROR: ./amc binary not found or not executable." >&2
    exit 1
fi

# Validate by running the full test suite (this is the whole point —
# we only snapshot a known-good amc).
if ! $SKIP_TESTS; then
    echo "Running full test suite before snapshotting..."
    if ! ./tests/run_all_tests.sh > /tmp/snapshot_tests.log 2>&1; then
        echo "ERROR: tests failed — refusing to snapshot a broken amc." >&2
        echo "       see /tmp/snapshot_tests.log" >&2
        exit 1
    fi
    PASS_COUNT=$(grep -c "PASS" /tmp/snapshot_tests.log || true)
else
    echo "WARNING: --skip-tests was passed; snapshot may be broken."
    PASS_COUNT="(skipped)"
fi

# Capture the portable bundle.
cp src/amc_lib.c snapshot/amc_lib.c
echo "  saved: snapshot/amc_lib.c ($(wc -l < snapshot/amc_lib.c) lines)"

# Compile a local Linux binary (not committed, but useful immediately).
gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -o snapshot/amc
echo "  built: snapshot/amc ($(stat -c%s snapshot/amc) bytes)"

# Provenance.
cat > snapshot/INFO.md <<EOF
# Amalgame compiler snapshot

Captured: $(date -Iseconds)
Git rev:  $(git rev-parse HEAD)
Branch:   $(git rev-parse --abbrev-ref HEAD)
Tests:    $PASS_COUNT passed

This snapshot is the canonical bootstrap binary. \`build_amc.sh\` uses
\`./amc\` if present, otherwise falls back to \`./snapshot/amc\`. From a
clean clone, recompile \`snapshot/amc\` from the tracked
\`snapshot/amc_lib.c\` with \`gcc\` (one command, no other compiler
needed).

To recompile the snapshot binary on this platform:

    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -o snapshot/amc

To restore an older snapshot (after a bad commit):

    git checkout <good-rev> -- snapshot/amc_lib.c
    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -o snapshot/amc
EOF

echo "  wrote: snapshot/INFO.md"
echo ""

# Refresh the personal prefix too — the snapshot we just saved was
# validated by the full test suite, so it's by definition a "known
# good" amc to expose to other repos on this machine. Same install
# helper as build_amc.sh Step 5. Opt out with AMC_SKIP_LOCAL_INSTALL=1.
echo "── Refreshing personal prefix ────────────────"
./tools/install-local.sh
echo ""

echo "✓ Snapshot saved."
