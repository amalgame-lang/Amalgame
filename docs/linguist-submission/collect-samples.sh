#!/usr/bin/env bash
# Copy a representative, deduplicated set of real .am files from this repo
# into a linguist checkout's samples/Amalgame/ directory.
#
# Linguist's Bayesian classifier is trained on these samples — they are
# what disambiguates a `.am` file as Amalgame vs Automake when the
# heuristic regex is inconclusive. More variety = better. We ship a small
# curated set in ./samples/Amalgame/ for the PR, but you can widen it.
#
# Usage:  ./collect-samples.sh /path/to/your/linguist/clone
set -euo pipefail

LINGUIST="${1:?usage: collect-samples.sh <path-to-linguist-checkout>}"
REPO="$(cd "$(dirname "$0")/../.." && pwd)"   # repo root (docs/linguist-submission/..)
DEST="$LINGUIST/samples/Amalgame"

mkdir -p "$DEST"

# Curated, diverse: hello-world, records, a class with methods, a real
# library module, and a big stdlib file (interpolation, generics, @c).
files=(
  tests/samples/hello.am
  tests/samples/record.am
  tests/samples/lib_e2e.am
  src/argparser.am
  src/stdlib/json.am
  src/stdlib/path.am
)

n=0
for f in "${files[@]}"; do
  if [[ -f "$REPO/$f" ]]; then
    cp "$REPO/$f" "$DEST/$(basename "$f")"
    n=$((n+1))
  fi
done

echo "Copied $n Amalgame samples -> $DEST"
echo "Now run, from the linguist checkout:"
echo "  bundle exec rake samples   # rebuild the classifier DB"
