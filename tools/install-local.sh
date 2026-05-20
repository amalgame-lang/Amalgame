#!/bin/bash
# ─────────────────────────────────────────────────────
#  tools/install-local.sh — copy ./amc + runtime + lib + stdlib
#                          into a personal prefix (default ~/.local).
#
#  Convenience helper called from build_amc.sh and tools/save-snapshot.sh
#  so the freshly-built / freshly-snapshotted amc is immediately
#  usable from anywhere on the contributor's machine (sibling repos,
#  examples, LSP integration in the editor, etc.).
#
#  This is NOT the user-facing installer — install/install.sh handles
#  that with download + verify of the release tarball. This script is
#  for the dev's own workflow: rebuild here, use everywhere.
#
#  Layout (matches install/install.sh + Program.ResolveRuntimeDir):
#    <prefix>/bin/amc
#    <prefix>/share/amalgame/runtime/   _runtime.h + Amalgame_*.h
#    <prefix>/share/amalgame/lib/       libamalgame.a (Step 4)
#    <prefix>/share/amalgame/stdlib/    bundled-stdlib .am facades
#
#  Override prefix:  AMC_LOCAL_PREFIX=/somewhere/else ./tools/install-local.sh
#  Skip entirely:    AMC_SKIP_LOCAL_INSTALL=1 ./build_amc.sh
# ─────────────────────────────────────────────────────

set -e
cd "$(dirname "$0")/.."

if [ "${AMC_SKIP_LOCAL_INSTALL:-0}" = "1" ]; then
    echo "  (skipped: AMC_SKIP_LOCAL_INSTALL=1)"
    exit 0
fi

PREFIX="${AMC_LOCAL_PREFIX:-$HOME/.local}"
BIN_DIR="$PREFIX/bin"
SHARE_DIR="$PREFIX/share/amalgame"

if [ ! -x ./amc ]; then
    echo "  (skipped: ./amc not built yet)" >&2
    exit 0
fi

mkdir -p "$BIN_DIR" "$SHARE_DIR/runtime" "$SHARE_DIR/lib" "$SHARE_DIR/stdlib"

# `install` over `cp` for the binary: when the previous amc is
# currently running (LSP server, hot reload, etc.), Linux returns
# ETXTBSY on `cp` but `install` unlinks the dir entry first, which
# the kernel allows even for a busy inode.
install -m 755 ./amc "$BIN_DIR/amc"
cp runtime/*.h "$SHARE_DIR/runtime/"

if [ -f lib/libamalgame.a ]; then
    cp lib/libamalgame.a "$SHARE_DIR/lib/libamalgame.a"
fi

if compgen -G "src/stdlib/*.am" >/dev/null; then
    cp src/stdlib/*.am "$SHARE_DIR/stdlib/"
fi

# Stale-version safety net: if a previous install left runtime headers
# that no longer exist in the source tree (e.g. Amalgame_Math.h after
# the v0.7.5 stdlib split), remove them so they don't shadow the new
# pure-AM facades. We don't blanket-rm — only files whose basename is
# absent from runtime/.
for installed in "$SHARE_DIR/runtime/"*.h; do
    [ -e "$installed" ] || continue
    base=$(basename "$installed")
    if [ ! -e "runtime/$base" ]; then
        rm -f "$installed"
        echo "  removed stale: $base"
    fi
done

echo "✓ Installed to $PREFIX"
echo "  bin:     $BIN_DIR/amc ($(stat -c%s "$BIN_DIR/amc" 2>/dev/null || stat -f%z "$BIN_DIR/amc") bytes)"
echo "  runtime: $SHARE_DIR/runtime/ ($(ls "$SHARE_DIR/runtime/" | wc -l) headers)"
echo "  stdlib:  $SHARE_DIR/stdlib/ ($(ls "$SHARE_DIR/stdlib/" 2>/dev/null | wc -l) facades)"
[ -f "$SHARE_DIR/lib/libamalgame.a" ] && echo "  lib:     $SHARE_DIR/lib/libamalgame.a ($(stat -c%s "$SHARE_DIR/lib/libamalgame.a" 2>/dev/null || stat -f%z "$SHARE_DIR/lib/libamalgame.a") bytes)"
