#!/bin/bash
# ─────────────────────────────────────────────────────
#  tools/register-package.sh — manually register a new
#  package version on amalgame-lang/packages-index.
#
#  Workflow:
#    1. You tag the package repo (e.g. amalgame-encoding) with
#       `git tag v0.X.Y && git push origin v0.X.Y`.
#    2. The package's own CI runs (tests against amc).
#    3. Once that CI is green, you invoke this script:
#         ./tools/register-package.sh encoding v0.1.0
#       It validates the tag exists + CI passed, then opens a
#       PR on packages-index appending the [[version]] block.
#
#  Replaces the per-repo `.github/workflows/index-pr.yml`
#  pattern which required a PACKAGES_INDEX_PAT secret on every
#  package repo. With this script the credentials live in the
#  developer's `gh auth` session — no repo-level secret needed.
#
#  Requires: `gh` (authenticated), `git`, network access.
# ─────────────────────────────────────────────────────

set -euo pipefail

# ── Args ──────────────────────────────────────────────
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <shortname> <tag>" >&2
    echo "" >&2
    echo "Examples:" >&2
    echo "  $0 encoding v0.1.0" >&2
    echo "  $0 sqlite v0.2.3" >&2
    echo "" >&2
    echo "The <shortname> must already have a [[package]] entry on" >&2
    echo "amalgame-lang/packages-index. Open a PR by hand the first" >&2
    echo "time a package is registered; this script only handles" >&2
    echo "[[version]] appends for known packages." >&2
    exit 2
fi

SHORTNAME="$1"
TAG="$2"

# ── Validate inputs ───────────────────────────────────
if ! [[ "$SHORTNAME" =~ ^[a-z][a-z0-9-]*$ ]]; then
    echo "ERROR: invalid shortname '$SHORTNAME' (a-z, 0-9, -)" >&2
    exit 1
fi
if ! [[ "$TAG" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "ERROR: invalid tag '$TAG' (expected vMAJOR.MINOR.PATCH)" >&2
    exit 1
fi

# ── Locate the package repo URL via packages-index ────
# Fetch the index file directly from GitHub raw (avoids cloning
# twice). The cache at ~/.amalgame/cache/packages-index.toml may
# be stale; raw.githubusercontent.com is always current.
INDEX_RAW="https://raw.githubusercontent.com/amalgame-lang/packages-index/main/packages.toml"
INDEX_TOML=$(curl -sSL "$INDEX_RAW")
if [ -z "$INDEX_TOML" ]; then
    echo "ERROR: could not fetch packages-index/main/packages.toml" >&2
    exit 1
fi

# Walk the [[package]] entries to find the one matching SHORTNAME
# and extract its url. Pure bash so this runs without yq/jq.
PKG_URL=""
PKG_REQ=""
while IFS= read -r line; do
    case "$line" in
        "[[package]]")
            cur_name=""; cur_url="" ;;
        "name"*"="*)
            cur_name=$(echo "$line" | sed 's/.*= *"\([^"]*\)".*/\1/') ;;
        "url"*"="*)
            cur_url=$(echo "$line" | sed 's/.*= *"\([^"]*\)".*/\1/')
            if [ "$cur_name" = "$SHORTNAME" ]; then
                PKG_URL="$cur_url"
            fi ;;
    esac
done <<< "$INDEX_TOML"

if [ -z "$PKG_URL" ]; then
    echo "ERROR: shortname '$SHORTNAME' not registered on packages-index." >&2
    echo "       Open a [[package]] PR on amalgame-lang/packages-index first," >&2
    echo "       then re-run this script for the version entry." >&2
    exit 1
fi
echo "→ package: $SHORTNAME ($PKG_URL)"

# The url is `github.com/owner/repo` (no scheme). Convert to
# the slug `owner/repo` for `gh` queries.
GH_SLUG="${PKG_URL#github.com/}"

# ── Read the manifest from the tag to extract required-amalgame ──
MANIFEST_RAW="https://raw.githubusercontent.com/$GH_SLUG/$TAG/amalgame.toml"
MANIFEST=$(curl -sSL --fail "$MANIFEST_RAW" 2>/dev/null || true)
if [ -z "$MANIFEST" ]; then
    echo "ERROR: tag $TAG not found on $GH_SLUG (or no amalgame.toml at root)." >&2
    exit 1
fi
PKG_REQ=$(echo "$MANIFEST" | grep -E '^required-amalgame[[:space:]]*=' | head -1 | sed 's/.*= *"\([^"]*\)".*/\1/')
if [ -z "$PKG_REQ" ]; then
    echo "ERROR: manifest at $GH_SLUG@$TAG has no [package].required-amalgame" >&2
    exit 1
fi
echo "→ tag $TAG present, required-amalgame=\"$PKG_REQ\""

# ── Gate on CI green for the tag ──────────────────────
# The user-reasonable invariant: never register a version on the
# curated index unless its own package CI was green for the tag.
# `gh run list --branch <tag>` finds all workflow runs whose
# refName matches refs/tags/<TAG>. We require AT LEAST ONE
# completed run with conclusion=success on the ci.yml workflow.
echo "→ checking CI status for $GH_SLUG@$TAG..."
# Only consider the latest run on the tag — retagging or
# re-running can leave historical failures around even after
# the package is fixed, but those shouldn't block a register.
CI_STATUS=$(gh run list --repo "$GH_SLUG" --branch "$TAG" --workflow ci.yml \
    --limit 1 --json status,conclusion 2>/dev/null \
    || echo "[]")

HAS_GREEN=$(echo "$CI_STATUS" | grep -c '"conclusion":"success"' || true)
HAS_FAIL=$(echo "$CI_STATUS"  | grep -c '"conclusion":"failure"' || true)
HAS_PENDING=$(echo "$CI_STATUS" | grep -c '"status":"in_progress"\|"status":"queued"' || true)

if [ "$HAS_PENDING" -gt 0 ]; then
    echo "ERROR: CI for $TAG is still in progress on $GH_SLUG." >&2
    echo "       Wait for it to finish, then re-run this script." >&2
    echo "       Watch via: gh run watch --repo $GH_SLUG" >&2
    exit 1
fi
if [ "$HAS_FAIL" -gt 0 ]; then
    echo "ERROR: CI for $TAG FAILED on $GH_SLUG." >&2
    echo "       Refusing to register a broken version on the index." >&2
    echo "       Inspect via: gh run list --repo $GH_SLUG --branch $TAG" >&2
    exit 1
fi
if [ "$HAS_GREEN" -eq 0 ]; then
    echo "WARNING: no completed CI run found for $TAG on $GH_SLUG." >&2
    echo "         The package repo may not have a ci.yml workflow," >&2
    echo "         or the tag was pushed without triggering it. Proceeding." >&2
fi
echo "→ CI gate passed (green runs: $HAS_GREEN, failures: $HAS_FAIL)"

# ── Idempotency check: is (shortname, tag) already in the index? ──
if echo "$INDEX_TOML" | grep -B2 "^tag *= *\"$TAG\"" | grep -q "^package *= *\"$SHORTNAME\""; then
    echo "→ $SHORTNAME@$TAG already registered on packages-index. Nothing to do."
    exit 0
fi

# ── Clone packages-index + branch + append + push + PR ──
WORKDIR=$(mktemp -d -t register-pkg-XXXXXX)
trap 'rm -rf "$WORKDIR"' EXIT

echo "→ cloning amalgame-lang/packages-index into $WORKDIR..."
gh repo clone amalgame-lang/packages-index "$WORKDIR/idx" -- --depth 1 --quiet

cd "$WORKDIR/idx"
BR="auto/index-$SHORTNAME-$TAG"
git checkout -b "$BR" --quiet

# Append the [[version]] block at the tail.
printf '\n[[version]]\npackage           = "%s"\ntag               = "%s"\nrequired-amalgame = "%s"\n' \
    "$SHORTNAME" "$TAG" "$PKG_REQ" >> packages.toml

# Local commit. Honor user's git config if present; otherwise use
# generic attribution (the script-runner can amend if they want).
git_email=$(git config --get user.email 2>/dev/null || echo "")
git_name=$(git config --get user.name 2>/dev/null || echo "")
if [ -z "$git_email" ]; then git_email="register-package@local"; fi
if [ -z "$git_name" ];  then git_name="package-register-script"; fi
git config user.email "$git_email"
git config user.name "$git_name"

git add packages.toml
git commit -m "register: $SHORTNAME@$TAG (needs amc $PKG_REQ)" --quiet
git push origin "$BR" --quiet

# Open the PR.
PR_URL=$(gh pr create \
    --repo amalgame-lang/packages-index \
    --base main \
    --head "$BR" \
    --title "register: $SHORTNAME@$TAG" \
    --body "Adds a \`[[version]]\` block for \`$SHORTNAME\` declaring \`required-amalgame = \"$PKG_REQ\"\`.

Source: $GH_SLUG@$TAG (CI verified green via \`tools/register-package.sh\`).
Review + merge to publish the version in \`amc package search\` /
auto-resolve.
")
echo "✓ opened: $PR_URL"
