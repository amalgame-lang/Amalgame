#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════
#  Amalgame — Release builder
#
#  Builds cross-platform release archives for GitHub Releases.
#  Run this on the CI/CD machine before publishing a release.
#
#  Usage:
#    ./install/release.sh 0.3.0
#
#  Output: dist/
#    amc-0.3.0-linux-x86_64.tar.gz
#    amc-0.3.0-macos-arm64.tar.gz
#    amc-0.3.0-macos-x86_64.tar.gz
#    checksums.sha256
# ═══════════════════════════════════════════════════════════

set -euo pipefail

VERSION="${1:-$(grep "version" meson.build | head -1 | grep -oP "[\d.]+" )}"
DIST="dist"

echo "Building Amalgame v$VERSION release archives..."
mkdir -p "$DIST"

# ── Build for current platform ────────────────────────────
OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
ARCH="$(uname -m)"
case "$ARCH" in
    x86_64)  ARCH_TAG="x86_64" ;;
    aarch64) ARCH_TAG="arm64"  ;;
    arm64)   ARCH_TAG="arm64"  ;;
esac

TARGET="${OS}-${ARCH_TAG}"
ARCHIVE_NAME="amc-${VERSION}-${TARGET}"
ARCHIVE_DIR="$DIST/$ARCHIVE_NAME"

echo "Building for $TARGET..."

# Build
meson setup build --buildtype=release --wipe 2>/dev/null || meson setup build --buildtype=release
ninja -C build

# Assemble archive directory
mkdir -p "$ARCHIVE_DIR/runtime"
cp build/amc                              "$ARCHIVE_DIR/"
cp src/transpiler/runtime/_runtime.h     "$ARCHIVE_DIR/runtime/"
cp README.md                             "$ARCHIVE_DIR/"
cp docs/DEVELOPER_GUIDE.md               "$ARCHIVE_DIR/"

# Create tarball
(cd "$DIST" && tar -czf "${ARCHIVE_NAME}.tar.gz" "$ARCHIVE_NAME")
rm -rf "$ARCHIVE_DIR"

echo "  → $DIST/${ARCHIVE_NAME}.tar.gz"

# ── Generate checksums ────────────────────────────────────
echo "Generating checksums..."
(cd "$DIST" && sha256sum *.tar.gz > checksums.sha256)
echo "  → $DIST/checksums.sha256"

# ── Print next steps ──────────────────────────────────────
echo ""
echo "Release files ready in $DIST/:"
ls -lh "$DIST/"
echo ""
echo "Next steps:"
echo "  1. Build on other platforms (macOS, ARM Linux) and collect their archives"
echo "  2. Create a GitHub Release for v$VERSION"
echo "  3. Upload all .tar.gz files + checksums.sha256"
echo "  4. Update install/homebrew/amalgame.rb with the new SHA256:"
echo ""
echo "     sha256 of source tarball:"
curl -sSL "https://github.com/BastienMOUGET/CODE/archive/refs/tags/v${VERSION}.tar.gz" \
    | sha256sum | awk '{print "     " $1}' 2>/dev/null || \
    echo "     (run after pushing the tag to GitHub)"
echo ""
