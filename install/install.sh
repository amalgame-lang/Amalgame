#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════
#  Amalgame Language — Universal Installer
#  https://github.com/BastienMOUGET/Amalgame
#
#  Usage:
#    curl -sSL https://raw.githubusercontent.com/BastienMOUGET/Amalgame/main/install/install.sh | bash
#
#  Options (env vars):
#    AMC_VERSION   — version to install (default: latest)
#    AMC_PREFIX    — install prefix    (default: /usr/local)
#    AMC_NO_GCC    — skip GCC check    (set to 1 to skip)
# ═══════════════════════════════════════════════════════════

set -euo pipefail

# ── Config ────────────────────────────────────────────────
REPO="BastienMOUGET/Amalgame"
VERSION="${AMC_VERSION:-latest}"
PREFIX="${AMC_PREFIX:-/usr/local}"
BIN_DIR="$PREFIX/bin"
LIB_DIR="$PREFIX/lib/amalgame"
TMP_DIR="$(mktemp -d)"

# ── Colors ────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

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
        ;;
    FreeBSD)
        TARGET="freebsd-x86_64"
        PKG_MGR="pkg"
        ;;
    *)
        error "Unsupported OS: $OS (use Windows installer for Windows)"
        ;;
esac

info "OS       : $OS"
info "Arch     : $ARCH"
info "Target   : $TARGET"
info "Prefix   : $PREFIX"

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

# GCC — required to compile the .c output
if [[ "${AMC_NO_GCC:-0}" != "1" ]]; then
    if ! check_dep gcc; then
        warn "GCC not found — attempting to install..."
        case "$OS" in
            Linux)   install_dep_linux gcc ;;
            Darwin)  warn "Run: xcode-select --install" ;;
            FreeBSD) sudo pkg install -y gcc ;;
        esac
    fi
fi

# Runtime dependencies — libgc (Boehm GC), libm, libcurl
header "Checking runtime dependencies..."

install_runtime_deps_linux() {
    case "$PKG_MGR" in
        apt)
            local missing=()
            dpkg -s libgc-dev        &>/dev/null || missing+=("libgc-dev")
            dpkg -s libcurl4-openssl-dev &>/dev/null || missing+=("libcurl4-openssl-dev")
            if [[ ${#missing[@]} -gt 0 ]]; then
                info "Installing: ${missing[*]}"
                sudo apt-get install -y "${missing[@]}"
            fi
            ;;
        dnf)
            sudo dnf install -y gc-devel libcurl-devel 2>/dev/null || true
            ;;
        pacman)
            sudo pacman -S --noconfirm gc curl 2>/dev/null || true
            ;;
        zypper)
            sudo zypper install -y libgc-devel libcurl-devel 2>/dev/null || true
            ;;
        *)
            warn "Cannot auto-install runtime deps — please install manually:"
            warn "  Boehm GC  : libgc-dev"
            warn "  libcurl   : libcurl4-openssl-dev"
            ;;
    esac
}

install_runtime_deps_macos() {
    if command -v brew &>/dev/null; then
        brew list bdw-gc &>/dev/null || brew install bdw-gc
        brew list curl   &>/dev/null || brew install curl
    else
        warn "Homebrew not found — install it at https://brew.sh then run:"
        warn "  brew install bdw-gc curl"
    fi
}

case "$OS" in
    Linux)
        install_runtime_deps_linux
        success "Runtime dependencies ready"
        ;;
    Darwin)
        install_runtime_deps_macos
        success "Runtime dependencies ready"
        ;;
    FreeBSD)
        sudo pkg install -y boehm-gc curl 2>/dev/null || true
        ;;
esac

# libm is part of glibc on Linux / libSystem on macOS — no install needed

# curl or wget — for downloading
HAS_CURL=0; HAS_WGET=0
check_dep curl && HAS_CURL=1 || true
check_dep wget && HAS_WGET=1 || true
[[ $HAS_CURL -eq 0 && $HAS_WGET -eq 0 ]] && error "Neither curl nor wget found"

# ── Fetch latest version ──────────────────────────────────
header "Fetching release info..."

fetch_url() {
    local url="$1"
    if [[ $HAS_CURL -eq 1 ]]; then
        curl -sSL "$url"
    else
        wget -qO- "$url"
    fi
}

download() {
    local url="$1"
    local dest="$2"
    info "Downloading $(basename $dest)..."
    if [[ $HAS_CURL -eq 1 ]]; then
        curl -sSL -o "$dest" "$url"
    else
        wget -qO "$dest" "$url"
    fi
}

if [[ "$VERSION" == "latest" ]]; then
    API_URL="https://api.github.com/repos/$REPO/releases/latest"
    VERSION="$(fetch_url "$API_URL" | grep '"tag_name"' | sed 's/.*"tag_name": "\(.*\)".*/\1/')"
    [[ -z "$VERSION" ]] && error "Could not fetch latest version from GitHub API"
fi

info "Version  : $VERSION"

BASE_URL="https://github.com/$REPO/releases/download/$VERSION"
ARCHIVE="amc-$VERSION-$TARGET.tar.gz"
URL="$BASE_URL/$ARCHIVE"

# ── Download & verify ─────────────────────────────────────
header "Downloading Amalgame $VERSION..."

ARCHIVE_PATH="$TMP_DIR/$ARCHIVE"
download "$URL" "$ARCHIVE_PATH"

# Verify checksum if available
CHECKSUM_URL="$BASE_URL/checksums.sha256"
CHECKSUM_FILE="$TMP_DIR/checksums.sha256"
if fetch_url "$CHECKSUM_URL" > "$CHECKSUM_FILE" 2>/dev/null; then
    info "Verifying checksum..."
    if command -v sha256sum &>/dev/null; then
        (cd "$TMP_DIR" && grep "$ARCHIVE" checksums.sha256 | sha256sum -c)
        success "Checksum verified"
    elif command -v shasum &>/dev/null; then
        (cd "$TMP_DIR" && grep "$ARCHIVE" checksums.sha256 | shasum -a 256 -c)
        success "Checksum verified"
    else
        warn "sha256sum not found — skipping checksum verification"
    fi
fi

# ── Install ───────────────────────────────────────────────
header "Installing..."

# Extract
info "Extracting archive..."
tar -xzf "$ARCHIVE_PATH" -C "$TMP_DIR"

EXTRACT_DIR="$TMP_DIR/amc-$VERSION-$TARGET"

# Create directories
sudo mkdir -p "$BIN_DIR" "$LIB_DIR"

# Install binary
sudo install -m 755 "$EXTRACT_DIR/amc" "$BIN_DIR/amc"
success "Binary installed → $BIN_DIR/amc"

# Install runtime header
if [[ -f "$EXTRACT_DIR/runtime/_runtime.h" ]]; then
    sudo cp "$EXTRACT_DIR/runtime/_runtime.h" "$LIB_DIR/_runtime.h"
    success "Runtime header → $LIB_DIR/_runtime.h"
fi

# Install stdlib (if present)
if [[ -d "$EXTRACT_DIR/stdlib" ]]; then
    sudo cp -r "$EXTRACT_DIR/stdlib" "$LIB_DIR/"
    success "Stdlib → $LIB_DIR/stdlib/"
fi

# ── Shell configuration ───────────────────────────────────
header "Configuring shell..."

add_to_path() {
    local shell_rc="$1"
    local export_line="export PATH=\"$BIN_DIR:\$PATH\""

    if [[ -f "$shell_rc" ]] && grep -q "amalgame" "$shell_rc" 2>/dev/null; then
        info "$shell_rc already configured"
        return
    fi

    echo "" >> "$shell_rc"
    echo "# Amalgame Language" >> "$shell_rc"
    echo "$export_line" >> "$shell_rc"
    echo "export AMC_RUNTIME=\"$LIB_DIR\"" >> "$shell_rc"
    success "Added to $shell_rc"
}

# Only needed if prefix is not already in PATH
if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
    SHELL_NAME="$(basename "${SHELL:-/bin/sh}")"
    case "$SHELL_NAME" in
        bash) add_to_path "$HOME/.bashrc"; add_to_path "$HOME/.bash_profile" ;;
        zsh)  add_to_path "$HOME/.zshrc" ;;
        fish) fish -c "fish_add_path $BIN_DIR" 2>/dev/null || true ;;
        *)    warn "Unknown shell $SHELL_NAME — add $BIN_DIR to your PATH manually" ;;
    esac
fi

# ── Cleanup ───────────────────────────────────────────────
rm -rf "$TMP_DIR"

# ── Final verification ────────────────────────────────────
header "Verifying installation..."

if "$BIN_DIR/amc" --version &>/dev/null; then
    success "amc installed successfully!"
    echo ""
    "$BIN_DIR/amc" --version
else
    error "Installation failed — amc not working"
fi

# ── Done ──────────────────────────────────────────────────
echo ""
echo -e "${GREEN}${BOLD}  Amalgame is ready!${NC}"
echo ""
echo "  Quick start:"
echo -e "    ${CYAN}echo 'namespace App\npublic class Program {\n    public static void Main(string[] args) {\n        Console.WriteLine(\"Hello!\")\n    }\n}' > hello.am${NC}"
echo -e "    ${CYAN}amc hello.am && ./hello${NC}"
echo ""
echo "  Documentation:"
echo "    https://github.com/$REPO/blob/main/docs/DEVELOPER_GUIDE.md"
echo ""
[[ ":$PATH:" != *":$BIN_DIR:"* ]] && \
    echo -e "  ${YELLOW}Restart your shell or run: source ~/.bashrc${NC}\n"
