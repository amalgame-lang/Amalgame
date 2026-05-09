#!/bin/bash
# ─────────────────────────────────────────────────────
#  tools/release.sh — cut a new amc release end-to-end
# ─────────────────────────────────────────────────────
#
# Bumps the version everywhere it lives (src/main.am, README,
# CHANGELOG entry), builds + tests + saves a snapshot, then walks
# the gitflow all the way to a published GitHub Release:
#
#     release/vX.Y.Z  →  develop  →  main  →  tag vX.Y.Z
#
# develop and main go through PRs because both are protected
# branches; the script enables `--auto` merge on each so the gh CI
# is what actually flips the switch. Once main has the release
# commit, the tag is pushed and `.github/workflows/release.yml`
# kicks in to build the cross-platform artifacts.
#
# Usage:
#   ./tools/release.sh <version>      # full release flow
#
#   ./tools/release.sh 0.4.4          # → cuts vX.Y.Z = v0.4.4
#
# Flags:
#   --dry-run       Walk through the steps without mutating
#                   anything (no edits, no commit, no push).
#   --no-tag        Stop after the develop → main PR is merged.
#                   The tag step is skipped — useful when you
#                   want to inspect main before publishing.
#
# Pre-flight refuses to start unless:
#   - you're on `develop`
#   - working tree is clean
#   - local develop matches origin/develop
#   - the target tag doesn't already exist
#   - the target version doesn't equal the current one
#
# Recovery: if the script aborts mid-way, no destructive action
# has happened yet — you can `git checkout develop` and reset the
# release branch by hand. The two PR auto-merges are reversible
# with a normal revert PR.

set -e
cd "$(dirname "$0")/.."

# ── Args ──────────────────────────────────────────────
VERSION=""
DRY_RUN=false
NO_TAG=false

while [ $# -gt 0 ]; do
    case "$1" in
        --dry-run) DRY_RUN=true; shift ;;
        --no-tag)  NO_TAG=true;  shift ;;
        -h|--help)
            sed -n '2,40p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        -*)
            echo "ERROR: unknown flag '$1'" >&2
            exit 2
            ;;
        *)
            if [ -n "$VERSION" ]; then
                echo "ERROR: extra argument '$1' (already have version '$VERSION')" >&2
                exit 2
            fi
            VERSION="$1"
            shift
            ;;
    esac
done

if [ -z "$VERSION" ]; then
    echo "Usage: $0 [--dry-run] [--no-tag] <version>" >&2
    echo "  example: $0 0.4.4" >&2
    exit 2
fi

# Validate semver-ish: X.Y.Z, optionally with a -suffix tag.
if ! [[ "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[A-Za-z0-9.-]+)?$ ]]; then
    echo "ERROR: version must look like X.Y.Z (got '$VERSION')" >&2
    exit 2
fi

TAG="v$VERSION"
DATE=$(date +%Y-%m-%d)
RELEASE_BRANCH="release/$TAG"

# ── Output helpers ────────────────────────────────────
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m'

step()  { echo -e "${CYAN}── $1 ──${NC}"; }
ok()    { echo -e "  ${GREEN}✓${NC} $1"; }
warn()  { echo -e "  ${YELLOW}!${NC} $1"; }
fail()  { echo -e "  ${RED}✗ $1${NC}" >&2; exit 1; }

if $DRY_RUN; then
    warn "DRY RUN — no edits, no commits, no pushes will happen"
    echo ""
fi

run() {
    if $DRY_RUN; then
        echo "    [dry-run] $*"
    else
        eval "$@"
    fi
}

# ── Pre-flight ────────────────────────────────────────
step "Pre-flight"

if ! command -v gh >/dev/null 2>&1; then
    fail "gh CLI not found — install from https://cli.github.com"
fi
ok "gh CLI available"

if ! git diff --quiet || ! git diff --cached --quiet; then
    fail "working tree has uncommitted changes (commit or stash first)"
fi
ok "working tree clean"

UNTRACKED=$(git ls-files --others --exclude-standard)
if [ -n "$UNTRACKED" ]; then
    warn "untracked files present (won't be committed):"
    echo "$UNTRACKED" | sed 's/^/      /'
fi

CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
if [ "$CURRENT_BRANCH" != "develop" ]; then
    fail "must be on 'develop' (currently on '$CURRENT_BRANCH')"
fi
ok "on develop"

git fetch origin develop main --quiet
LOCAL=$(git rev-parse develop)
REMOTE=$(git rev-parse origin/develop)
if [ "$LOCAL" != "$REMOTE" ]; then
    fail "local develop ($LOCAL) ≠ origin/develop ($REMOTE) — run: git pull --ff-only"
fi
ok "in sync with origin/develop"

if git rev-parse -q --verify "refs/tags/$TAG" >/dev/null 2>&1; then
    fail "tag $TAG already exists locally"
fi
if git ls-remote --tags origin "$TAG" 2>/dev/null | grep -q "$TAG"; then
    fail "tag $TAG already exists on origin"
fi
ok "tag $TAG is fresh"

# Find current canonical version in src/main.am.
CURRENT=$(grep -oE 'amc [0-9]+\.[0-9]+\.[0-9]+' src/main.am | head -1 | awk '{print $2}')
if [ -z "$CURRENT" ]; then
    fail "couldn't extract current version from src/main.am"
fi
if [ "$CURRENT" = "$VERSION" ]; then
    fail "target version equals current version ($CURRENT)"
fi
ok "version: $CURRENT  →  $VERSION"

echo ""
read -p "Continue with release $TAG? [y/N] " confirm
if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
    echo "Aborted."
    exit 0
fi

# ── Branch + bump version files ───────────────────────
step "Cutting release branch + bumping version"

run "git checkout -b $RELEASE_BRANCH"
ok "branch: $RELEASE_BRANCH"

run "sed -i \"s/amc $CURRENT/amc $VERSION/g\" src/main.am"
ok "src/main.am"

run "sed -i \"s|# amc $CURRENT|# amc $VERSION|g\" README.md"
ok "README.md"

# ── CHANGELOG stub ────────────────────────────────────
step "Inserting CHANGELOG stub"

# Splice a new section right before the first existing `## [` header.
if ! $DRY_RUN; then
    awk -v tag="[$TAG]" -v date="$DATE" '
        BEGIN { spliced = 0 }
        /^## \[/ && !spliced {
            print "## " tag " — " date
            print ""
            print "- TODO: describe the changes"
            print ""
            spliced = 1
        }
        { print }
        END {
            if (!spliced) {
                # Empty CHANGELOG — append at end.
                print ""
                print "## " tag " — " date
                print ""
                print "- TODO: describe the changes"
            }
        }
    ' CHANGELOG.md > CHANGELOG.md.tmp
    mv CHANGELOG.md.tmp CHANGELOG.md

    # Append link line at the very end.
    echo "[$TAG]: https://github.com/amalgame-lang/Amalgame/releases/tag/$TAG" >> CHANGELOG.md
fi
ok "CHANGELOG.md stub inserted ($TAG)"

if ! $DRY_RUN; then
    echo ""
    echo "  → Edit CHANGELOG.md and replace the TODO line with the"
    echo "    real release notes. Save and close, then press Enter."
    read -p "" </dev/tty
fi

# ── Build + tests + snapshot ──────────────────────────
step "Build + tests + snapshot"

run "./build_amc.sh > /dev/null 2>&1"
ok "amc built"

if ! $DRY_RUN; then
    if ! ./tests/run_all_tests.sh > /tmp/release-tests-$$.log 2>&1; then
        echo "" >&2
        tail -30 /tmp/release-tests-$$.log >&2
        rm -f /tmp/release-tests-$$.log
        fail "tests failed — release aborted"
    fi
    PASS_COUNT=$(grep -E "^\s+.*PASS:" /tmp/release-tests-$$.log | tail -1 | sed -E 's/.*PASS: ([0-9]+).*/\1/' || echo "?")
    rm -f /tmp/release-tests-$$.log
else
    PASS_COUNT="?"
fi
ok "test suite green ($PASS_COUNT PASS)"

run "./tools/save-snapshot.sh --skip-tests > /dev/null 2>&1"
ok "snapshot saved"

# ── Commit + push release branch ──────────────────────
step "Commit + push release branch"

# Build the commit body from the new CHANGELOG section so the
# commit message stays in sync with what users will see on GitHub.
if ! $DRY_RUN; then
    BODY=$(awk -v tag="[$TAG]" '
        BEGIN { capturing = 0 }
        /^## \[/ {
            if (capturing) { exit }
            if (index($0, tag) > 0) { capturing = 1; next }
        }
        capturing { print }
    ' CHANGELOG.md | sed -E '/^$/N;/^\n$/D')
fi

run "git add src/main.am README.md CHANGELOG.md src/amc_lib.c snapshot/amc_lib.c snapshot/INFO.md"

if ! $DRY_RUN; then
    git commit -m "release: $TAG

$BODY

Co-Authored-By: Claude Opus 4.7 (1M context) <noreply@anthropic.com>"
else
    echo "    [dry-run] git commit -m 'release: $TAG' (body from CHANGELOG)"
fi
ok "commit created"

run "git push -u origin $RELEASE_BRANCH"
ok "branch pushed"

# ── PR release → develop ──────────────────────────────
step "PR release/$TAG → develop"

if ! $DRY_RUN; then
    PR_RELEASE_URL=$(gh pr create \
        --base develop \
        --head "$RELEASE_BRANCH" \
        --title "release: $TAG" \
        --body "Bumps version to $TAG. Auto-merge enabled — this PR rolls into develop, then a follow-up PR rolls develop into main, then \`tools/release.sh\` tags from main and pushes." 2>&1 | tail -1)
    PR_RELEASE_NUM=$(echo "$PR_RELEASE_URL" | grep -oE '[0-9]+$')
    ok "PR #$PR_RELEASE_NUM: $PR_RELEASE_URL"

    gh pr merge "$PR_RELEASE_NUM" --auto --merge >/dev/null 2>&1 \
        || warn "auto-merge enable failed — merge $PR_RELEASE_URL by hand"

    echo "  ⏳ waiting for PR #$PR_RELEASE_NUM to merge into develop..."
    while true; do
        STATE=$(gh pr view "$PR_RELEASE_NUM" --json state --jq .state 2>/dev/null || echo "")
        if [ "$STATE" = "MERGED" ]; then break; fi
        if [ "$STATE" = "CLOSED" ]; then fail "PR #$PR_RELEASE_NUM was closed without merging"; fi
        sleep 15
    done
    ok "PR #$PR_RELEASE_NUM merged into develop"
else
    echo "    [dry-run] gh pr create --base develop --head $RELEASE_BRANCH"
    echo "    [dry-run] gh pr merge <num> --auto --merge"
    echo "    [dry-run] poll until merged"
fi

# Refresh local develop with the merged result.
run "git fetch origin develop --quiet"
run "git checkout develop"
run "git pull --ff-only origin develop"
ok "local develop synced"

if $NO_TAG; then
    echo ""
    warn "--no-tag set — stopping before main + tag."
    warn "develop is now at $TAG; manually run develop → main + tag when ready."
    exit 0
fi

# ── PR develop → main ─────────────────────────────────
step "PR develop → main"

if ! $DRY_RUN; then
    PR_MAIN_URL=$(gh pr create \
        --base main \
        --head develop \
        --title "release: $TAG (develop → main)" \
        --body "Promotes the develop tip (containing release: $TAG) to main. Tag $TAG will be pushed from main once this lands." 2>&1 | tail -1)
    PR_MAIN_NUM=$(echo "$PR_MAIN_URL" | grep -oE '[0-9]+$')
    ok "PR #$PR_MAIN_NUM: $PR_MAIN_URL"

    gh pr merge "$PR_MAIN_NUM" --auto --merge >/dev/null 2>&1 \
        || warn "auto-merge enable failed — merge $PR_MAIN_URL by hand"

    echo "  ⏳ waiting for PR #$PR_MAIN_NUM to merge into main..."
    while true; do
        STATE=$(gh pr view "$PR_MAIN_NUM" --json state --jq .state 2>/dev/null || echo "")
        if [ "$STATE" = "MERGED" ]; then break; fi
        if [ "$STATE" = "CLOSED" ]; then fail "PR #$PR_MAIN_NUM was closed without merging"; fi
        sleep 15
    done
    ok "PR #$PR_MAIN_NUM merged into main"
else
    echo "    [dry-run] gh pr create --base main --head develop"
    echo "    [dry-run] gh pr merge <num> --auto --merge"
    echo "    [dry-run] poll until merged"
fi

# ── Tag from main + push ──────────────────────────────
step "Tag + push"

run "git fetch origin main --quiet"
run "git checkout main"
run "git pull --ff-only origin main"

if ! $DRY_RUN; then
    git tag -a "$TAG" -m "Release $TAG"
fi
ok "tag $TAG created on main"

run "git push origin $TAG"
ok "tag $TAG pushed to origin"

# Back to develop for normal work.
run "git checkout develop"

# ── Done ──────────────────────────────────────────────
echo ""
echo -e "${GREEN}════════════════════════════════════════════${NC}"
echo -e "  ${GREEN}Released $TAG${NC}"
echo "  release.yml is now building cross-platform artifacts:"
echo "    https://github.com/amalgame-lang/Amalgame/actions"
echo "  Release page (live in ~5 min):"
echo "    https://github.com/amalgame-lang/Amalgame/releases/tag/$TAG"
echo -e "${GREEN}════════════════════════════════════════════${NC}"
