#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════
#  Amalgame Language — Universal Installer
#  https://github.com/amalgame-lang/Amalgame
#
#  Downloads the prebuilt release tarball for your OS/arch and
#  unpacks it under AMC_PREFIX, matching the XDG-style layout
#  amc itself looks for:
#
#    <prefix>/bin/amc(.exe)
#    <prefix>/share/amalgame/runtime/    _runtime.h + Amalgame_*.h
#    <prefix>/share/amalgame/lib/        libamalgame.a
#    <prefix>/share/amalgame/docs/       grammar + language tour
#
#  amc resolves these paths via Program.ResolveRuntimeDir /
#  ResolveLibAmalgameA (since v0.8.1) — no environment override
#  needed for the standard layout.
#
#  Usage:
#    curl -sSL https://raw.githubusercontent.com/amalgame-lang/Amalgame/main/install/install.sh | bash
#
#  Env vars:
#    AMC_VERSION   — version to install              (default: latest)
#    AMC_PREFIX    — install prefix                  (default: /usr/local)
#    AMC_NO_GCC    — skip GCC check                  (set to 1 to skip)
#    AMC_NO_DEPS   — skip runtime dep install        (set to 1 to skip)
# ═══════════════════════════════════════════════════════════

set -euo pipefail

# ── Config ────────────────────────────────────────────────
REPO="amalgame-lang/Amalgame"
VERSION="${AMC_VERSION:-latest}"
PREFIX="${AMC_PREFIX:-/usr/local}"
BIN_DIR="$PREFIX/bin"
SHARE_DIR="$PREFIX/share/amalgame"
RUNTIME_DIR="$SHARE_DIR/runtime"
LIB_DIR="$SHARE_DIR/lib"
DOCS_DIR="$SHARE_DIR/docs"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

# ── Colors ────────────────────────────────────────────────
if [ -t 1 ]; then
    RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[0;33m'
    CYAN='\033[0;36m'; BOLD='\033[1m'; NC='\033[0m'
else
    RED=''; GREEN=''; YELLOW=''; CYAN=''; BOLD=''; NC=''
fi

info()    { echo -e "${CYAN}  →${NC} $*"; }
success() { echo -e "${GREEN}  ✓${NC} $*"; }
warn()    { echo -e "${YELLOW}  !${NC} $*"; }
error()   { echo -e "${RED}  ✗${NC} $*" >&2; exit 1; }
header()  { echo -e "\n${BOLD}$*${NC}"; }

# ── Banner ────────────────────────────────────────────────
echo -e "${BOLD}"
echo "  ╔═══════════════════════════════════════╗"
echo "  ║   Amalgame Language Installer         ║"
echo "  ║   https://github.com/$REPO  ║"
echo "  ╚═══════════════════════════════════════╝"
echo -e "${NC}"

# ── Detect OS and architecture ────────────────────────────
header "Detecting system..."

OS="$(uname -s)"
ARCH="$(uname -m)"

case "$OS" in
    Linux)
        case "$ARCH" in
            x86_64)  TARGET="linux-x86_64"  ;;
            aarch64) TARGET="linux-arm64"   ;;
            armv7l)  TARGET="linux-armv7"   ;;
            *)       error "Unsupported architecture: $ARCH" ;;
        esac
        PKG_MGR=""
        if command -v apt-get &>/dev/null; then PKG_MGR="apt";
        elif command -v dnf &>/dev/null;   then PKG_MGR="dnf";
        elif command -v pacman &>/dev/null; then PKG_MGR="pacman";
        elif command -v zypper &>/dev/null; then PKG_MGR="zypper";
        fi
        ;;
    Darwin)
        case "$ARCH" in
            x86_64)  TARGET="macos-x86_64" ;;
            arm64)   TARGET="macos-arm64"  ;;
            *)       error "Unsupported architecture: $ARCH" ;;
        esac
        PKG_MGR="brew"
        # macOS Homebrew layout: ARM lives at /opt/homebrew, Intel at
        # /usr/local. Pick the active prefix if the user didn't pin
        # AMC_PREFIX explicitly — that way `amc` lands next to the
        # rest of Homebrew without an extra PATH entry.
        if [ "${AMC_PREFIX:-}" = "" ] && command -v brew &>/dev/null; then
            PREFIX="$(brew --prefix)"
            BIN_DIR="$PREFIX/bin"
            SHARE_DIR="$PREFIX/share/amalgame"
            RUNTIME_DIR="$SHARE_DIR/runtime"
            LIB_DIR="$SHARE_DIR/lib"
            DOCS_DIR="$SHARE_DIR/docs"
        fi
        ;;
    FreeBSD)
        TARGET="freebsd-x86_64"
        PKG_MGR="pkg"
        ;;
    *)
        error "Unsupported OS: $OS (use the Inno Setup .exe installer for Windows)"
        ;;
esac

# Default to user-local install when running unprivileged outside the
# standard prefixes — saves users from hitting "permission denied" on
# /usr/local when they didn't think to sudo. They can still pass
# AMC_PREFIX=/usr/local explicitly with sudo if they want system-wide.
if [ "${AMC_PREFIX:-}" = "" ] && [ "$EUID" -ne 0 ] && [ ! -w "$PREFIX" ]; then
    PREFIX="$HOME/.local"
    BIN_DIR="$PREFIX/bin"
    SHARE_DIR="$PREFIX/share/amalgame"
    RUNTIME_DIR="$SHARE_DIR/runtime"
    LIB_DIR="$SHARE_DIR/lib"
    DOCS_DIR="$SHARE_DIR/docs"
    warn "Not root and $PREFIX not writable — falling back to user-local install."
fi

info "OS       : $OS"
info "Arch     : $ARCH"
info "Target   : $TARGET"
info "Prefix   : $PREFIX"

# Decide once whether file ops need sudo. User-local prefix (e.g.
# $HOME/.local) never does; system prefix (/usr/local) does unless
# we're already root.
if [ -w "$(dirname "$PREFIX")" ] || [ "$EUID" -eq 0 ]; then
    SUDO=""
else
    SUDO="sudo"
fi

# ── Check dependencies ────────────────────────────────────
header "Checking dependencies..."

check_dep() {
    if command -v "$1" &>/dev/null; then
        success "$1 found ($(command -v $1))"
        return 0
    else
        return 1
    fi
}

install_dep_linux() {
    local pkg="$1"
    info "Installing $pkg..."
    case "$PKG_MGR" in
        apt)    sudo apt-get install -y "$pkg" ;;
        dnf)    sudo dnf install -y "$pkg" ;;
        pacman) sudo pacman -S --noconfirm "$pkg" ;;
        zypper) sudo zypper install -y "$pkg" ;;
        *)      warn "Cannot auto-install $pkg — please install it manually" ;;
    esac
}

# GCC — required to compile the .c amc emits.
if [ "${AMC_NO_GCC:-0}" != "1" ]; then
    if ! check_dep gcc; then
        warn "GCC not found — attempting to install..."
        case "$OS" in
            Linux)   install_dep_linux gcc ;;
            Darwin)  warn "Run: xcode-select --install" ;;
            FreeBSD) sudo pkg install -y gcc ;;
        esac
    fi
fi

# Runtime libraries — libgc (Boehm GC), libcurl, zlib (libm is part
# of glibc / libSystem so it never needs explicit install).
if [ "${AMC_NO_DEPS:-0}" != "1" ]; then
    header "Checking runtime dependencies..."
    case "$OS" in
        Linux)
            case "$PKG_MGR" in
                apt)
                    missing=()
                    dpkg -s libgc-dev &>/dev/null || missing+=("libgc-dev")
                    dpkg -s libcurl4-openssl-dev &>/dev/null || missing+=("libcurl4-openssl-dev")
                    dpkg -s zlib1g-dev &>/dev/null || missing+=("zlib1g-dev")
                    if [ ${#missing[@]} -gt 0 ]; then
                        info "Installing: ${missing[*]}"
                        sudo apt-get install -y "${missing[@]}"
                    fi
                    ;;
                dnf)    sudo dnf install -y gc-devel libcurl-devel zlib-devel 2>/dev/null || true ;;
                pacman) sudo pacman -S --noconfirm gc curl zlib 2>/dev/null || true ;;
                zypper) sudo zypper install -y libgc-devel libcurl-devel zlib-devel 2>/dev/null || true ;;
                *)
                    warn "Unknown package manager — install manually:"
                    warn "  libgc-dev libcurl4-openssl-dev zlib1g-dev (Debian names)"
                    ;;
            esac
            success "Runtime dependencies ready"
            ;;
        Darwin)
            if command -v brew &>/dev/null; then
                brew list bdw-gc &>/dev/null || brew install bdw-gc
                brew list curl   &>/dev/null || brew install curl
            else
                warn "Homebrew not found — install it at https://brew.sh then run:"
                warn "  brew install bdw-gc curl"
            fi
            success "Runtime dependencies ready"
            ;;
        FreeBSD)
            sudo pkg install -y boehm-gc curl 2>/dev/null || true
            ;;
    esac
fi

# curl or wget — for downloading the tarball.
HAS_CURL=0; HAS_WGET=0
check_dep curl && HAS_CURL=1 || true
check_dep wget && HAS_WGET=1 || true
[ $HAS_CURL -eq 0 ] && [ $HAS_WGET -eq 0 ] && error "Neither curl nor wget found"

fetch_url() {
    local url="$1"
    if [ $HAS_CURL -eq 1 ]; then curl -sSL "$url"
    else                          wget -qO- "$url"
    fi
}
download() {
    local url="$1" dest="$2"
    info "Downloading $(basename "$dest")..."
    if [ $HAS_CURL -eq 1 ]; then curl -sSL -o "$dest" "$url"
    else                          wget -qO "$dest" "$url"
    fi
}

# ── Resolve version ───────────────────────────────────────
header "Fetching release info..."
if [ "$VERSION" = "latest" ]; then
    API_URL="https://api.github.com/repos/$REPO/releases/latest"
    VERSION="$(fetch_url "$API_URL" | grep '"tag_name"' | head -n1 | sed 's/.*"tag_name": "\(.*\)".*/\1/')"
    [ -z "$VERSION" ] && error "Could not fetch latest version from GitHub API"
fi
# Normalise "v0.8.1" → "0.8.1" so the URL stays consistent whether
# the user pinned with or without the leading v.
VTAG="$VERSION"
[ "${VTAG#v}" = "$VTAG" ] && VTAG="v$VTAG"
VNUM="${VTAG#v}"
info "Version  : $VTAG"

BASE_URL="https://github.com/$REPO/releases/download/$VTAG"
ARCHIVE="amc-$VNUM-$TARGET.tar.gz"
URL="$BASE_URL/$ARCHIVE"

# ── Download & verify ─────────────────────────────────────
header "Downloading Amalgame $VTAG..."
ARCHIVE_PATH="$TMP_DIR/$ARCHIVE"
download "$URL" "$ARCHIVE_PATH"

# Verify checksum if the release ships one.
CHECKSUM_URL="$BASE_URL/$ARCHIVE.sha256"
CHECKSUM_FILE="$TMP_DIR/$ARCHIVE.sha256"
if fetch_url "$CHECKSUM_URL" > "$CHECKSUM_FILE" 2>/dev/null && [ -s "$CHECKSUM_FILE" ]; then
    info "Verifying checksum..."
    if command -v sha256sum &>/dev/null; then
        (cd "$TMP_DIR" && sha256sum -c "$ARCHIVE.sha256")
        success "Checksum verified"
    elif command -v shasum &>/dev/null; then
        (cd "$TMP_DIR" && shasum -a 256 -c "$ARCHIVE.sha256")
        success "Checksum verified"
    else
        warn "sha256sum / shasum not found — skipping checksum verification"
    fi
fi

# ── Install ───────────────────────────────────────────────
header "Installing to $PREFIX..."

info "Extracting archive..."
tar -xzf "$ARCHIVE_PATH" -C "$TMP_DIR"
EXTRACT_DIR="$TMP_DIR/amc-$VNUM-$TARGET"
[ -d "$EXTRACT_DIR" ] || error "Unexpected archive layout — missing $EXTRACT_DIR"

# The tarball is already laid out the way amc expects (bin/ +
# share/amalgame/{runtime,lib,docs}), produced by release.yml since
# v0.8.1. We just copy the tree into $PREFIX — no per-file rewiring.
$SUDO mkdir -p "$BIN_DIR" "$RUNTIME_DIR" "$LIB_DIR" "$DOCS_DIR"

# Back up an existing share/amalgame/ install so users can roll back
# without re-downloading. Skipped for bin/amc (atomic replace via cp).
backup_dir() {
    local target="$1"
    if [ -d "$target" ] && [ ! -L "$target" ]; then
        $SUDO rm -rf "$target.bak"
        $SUDO mv "$target" "$target.bak"
        info "moved old $target to $target.bak"
    fi
}
backup_dir "$RUNTIME_DIR"
backup_dir "$LIB_DIR"
backup_dir "$DOCS_DIR"

# Binary (and Windows DLLs if any landed in bin/).
$SUDO install -m 755 "$EXTRACT_DIR/bin/amc" "$BIN_DIR/amc"
success "Binary → $BIN_DIR/amc"
for extra in "$EXTRACT_DIR/bin/"*; do
    [ "$(basename "$extra")" = "amc" ] && continue
    [ -e "$extra" ] || continue
    $SUDO install -m 755 "$extra" "$BIN_DIR/"
done

# Runtime headers + libamalgame.a + LLM-prompt docs.
$SUDO cp -r "$EXTRACT_DIR/share/amalgame/runtime"/* "$RUNTIME_DIR/"
success "Runtime headers → $RUNTIME_DIR/"
if [ -d "$EXTRACT_DIR/share/amalgame/lib" ]; then
    $SUDO cp -r "$EXTRACT_DIR/share/amalgame/lib"/* "$LIB_DIR/"
    success "libamalgame.a → $LIB_DIR/"
fi
if [ -d "$EXTRACT_DIR/share/amalgame/docs" ]; then
    $SUDO cp -r "$EXTRACT_DIR/share/amalgame/docs"/* "$DOCS_DIR/"
    success "LLM prompt docs → $DOCS_DIR/"
fi

# Clean up the legacy <prefix>/lib/amalgame layout produced by
# pre-v0.8.1 installers, if present — those files would shadow the
# new share/amalgame/runtime if a user re-runs the installer.
if [ -d "$PREFIX/lib/amalgame" ]; then
    $SUDO rm -rf "$PREFIX/lib/amalgame.bak"
    $SUDO mv "$PREFIX/lib/amalgame" "$PREFIX/lib/amalgame.bak"
    info "moved legacy $PREFIX/lib/amalgame to .bak (no longer used)"
fi

# ── Shell configuration ───────────────────────────────────
header "Configuring shell..."
add_to_path() {
    local shell_rc="$1"
    [ ! -f "$shell_rc" ] && return
    if grep -q "Amalgame" "$shell_rc" 2>/dev/null; then
        info "$shell_rc already configured"
        return
    fi
    {
        echo ""
        echo "# Amalgame Language"
        echo "export PATH=\"$BIN_DIR:\$PATH\""
    } >> "$shell_rc"
    success "Added to $shell_rc"
}

# Only edit shell rcs when the install prefix isn't already on PATH.
# amc no longer needs AMC_RUNTIME — its built-in lookup chain finds
# share/amalgame/{runtime,lib} relative to the resolved binary path.
if [ ":$PATH:" != *":$BIN_DIR:"* ]; then
    SHELL_NAME="$(basename "${SHELL:-/bin/sh}")"
    case "$SHELL_NAME" in
        bash) add_to_path "$HOME/.bashrc"; add_to_path "$HOME/.bash_profile" ;;
        zsh)  add_to_path "$HOME/.zshrc" ;;
        fish) command -v fish &>/dev/null && fish -c "fish_add_path $BIN_DIR" 2>/dev/null || true ;;
        *)    warn "Unknown shell $SHELL_NAME — add $BIN_DIR to your PATH manually" ;;
    esac
fi

# ── Verification ──────────────────────────────────────────
header "Verifying installation..."
if "$BIN_DIR/amc" --version &>/dev/null; then
    success "amc installed successfully!"
    echo ""
    "$BIN_DIR/amc" --version
else
    error "Installation failed — $BIN_DIR/amc did not run cleanly"
fi

# ── Done ──────────────────────────────────────────────────
echo ""
echo -e "${GREEN}${BOLD}  Amalgame is ready!${NC}"
echo ""
echo "  Quick start:"
echo -e "    ${CYAN}amc new myapp --vscode${NC}     # scaffold an executable (F5-ready in VS Code)"
echo -e "    ${CYAN}cd myapp && ./build.sh${NC}      # release build"
echo -e "    ${CYAN}./build.sh -g${NC}               # debug build (DWARF, for `amc dap`)"
echo ""
echo "  Documentation:"
echo "    https://github.com/$REPO/blob/main/docs/guide/README.md"
echo ""
if [ ":$PATH:" != *":$BIN_DIR:"* ]; then
    echo -e "  ${YELLOW}Restart your shell or run: source ~/.bashrc${NC}\n"
fi
