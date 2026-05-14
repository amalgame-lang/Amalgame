# Amalgame — Publishing Guide

How to publish Amalgame to each package manager.

---

## 1. GitHub Releases (do this first — others depend on it)

```bash
# Tag the release
git tag v0.3.0
git push origin v0.3.0
```

The GitHub Actions workflow (`.github/workflows/release.yml`) will
automatically build all platforms and create the release.

---

## 2. Homebrew (macOS + Linux) ⭐ Start here

### Option A — Private tap (fastest, recommended for now)

1. Create a new GitHub repo: `amalgame-lang/homebrew-amalgame`
2. Copy `install/homebrew/amalgame.rb` into it as `Formula/amalgame.rb`
3. Update the `sha256` value:
   ```bash
   curl -sSL https://github.com/amalgame-lang/Amalgame/archive/refs/tags/v0.3.0.tar.gz | sha256sum
   ```
4. Users install with:
   ```bash
   brew tap amalgame-lang/amalgame
   brew install amalgame
   ```

### Option B — Submit to homebrew-core (when you have users)

Requirements: 75+ GitHub stars, 30+ forks, working stable release.

```bash
brew tap --force homebrew/core
cp install/homebrew/amalgame.rb $(brew --repository homebrew/core)/Formula/
brew audit --strict amalgame
brew test amalgame
# Then submit a PR to https://github.com/Homebrew/homebrew-core
```

---

## 3. Arch Linux AUR

Create an account at https://aur.archlinux.org then:

```bash
# Create PKGBUILD
cat > PKGBUILD << 'PKGEOF'
pkgname=amalgame
pkgver=0.3.0
pkgrel=1
pkgdesc="Modern programming language that transpiles to C"
arch=('x86_64' 'aarch64')
url="https://github.com/amalgame-lang/Amalgame"
license=('Apache-2.0')
depends=('gc' 'curl' 'gcc')
source=("$pkgname-$pkgver.tar.gz::https://github.com/amalgame-lang/Amalgame/archive/refs/tags/v$pkgver.tar.gz")
sha256sums=('REPLACE_WITH_SHA256')

build() {
    cd "Amalgame-$pkgver"
    gcc -O2 -Iruntime snapshot/amc_lib.c \
        -lgc -lm -lcurl -o amc
}

package() {
    cd "Amalgame-$pkgver"
    install -Dm755 amc "$pkgdir/usr/bin/amc"
    install -Dm644 runtime/_runtime.h \
        "$pkgdir/usr/lib/amalgame/_runtime.h"
}
PKGEOF

# Test locally
makepkg -si

# Publish to AUR
git clone ssh://aur@aur.archlinux.org/amalgame.git
cp PKGBUILD amalgame/
cd amalgame
makepkg --printsrcinfo > .SRCINFO
git add PKGBUILD .SRCINFO
git commit -m "Initial release v0.3.0"
git push
```

---

## 4. Debian/Ubuntu (.deb)

```bash
# Install packaging tools
sudo apt install build-essential devscripts debhelper

# Create debian/ directory structure
mkdir -p debian/source
echo "10" > debian/compat
echo "3.0 (quilt)" > debian/source/format

cat > debian/control << 'DEB'
Source: amalgame
Section: devel
Priority: optional
Maintainer: Bastien MOUGET <your@email.com>
Build-Depends: debhelper (>= 10), gcc, libgc-dev, libcurl4-openssl-dev
Standards-Version: 4.6.0
Homepage: https://github.com/amalgame-lang/Amalgame

Package: amalgame
Architecture: any
Depends: ${shlibs:Depends}, ${misc:Depends}, gcc, libgc1
Description: Modern programming language that transpiles to C
 Amalgame distills the best features from today's most productive
 languages into a single, modern, statically-typed language that
 compiles to native C code via GCC.
DEB

# Build the .deb
debuild -us -uc

# The .deb will appear in the parent directory
# Submit to Ubuntu PPA or Debian mentors:
# https://mentors.debian.net/
```

---

## 5. Fedora/RHEL (.rpm)

```bash
# Install RPM build tools
sudo dnf install rpm-build rpmdevtools gcc gc-devel libcurl-devel

rpmdev-setuptree

cat > ~/rpmbuild/SPECS/amalgame.spec << 'SPEC'
Name:           amalgame
Version:        0.3.0
Release:        1%{?dist}
Summary:        Modern programming language that transpiles to C
License:        Apache-2.0
URL:            https://github.com/amalgame-lang/Amalgame
Source0:        https://github.com/amalgame-lang/Amalgame/archive/refs/tags/v%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  gc-devel
BuildRequires:  libcurl-devel

Requires:       gcc
Requires:       gc
Requires:       libcurl

%description
Amalgame distills the best features from today's most productive
languages into a single, modern, statically-typed language.

%prep
%autosetup -n Amalgame-%{version}

%build
gcc -O2 -Iruntime snapshot/amc_lib.c \
    -lgc -lm -lcurl -o amc

%install
install -Dm755 amc %{buildroot}%{_bindir}/amc
install -Dm644 runtime/_runtime.h \
    %{buildroot}%{_libdir}/amalgame/_runtime.h

%files
%license LICENSE
%doc README.md docs/DEVELOPER_GUIDE.md
%{_bindir}/amc
%{_libdir}/amalgame/

%changelog
* $(date "+%a %b %d %Y") Bastien MOUGET <your@email.com> - 0.3.0-1
- Initial release
SPEC

rpmbuild -ba ~/rpmbuild/SPECS/amalgame.spec
# Submit to COPR: https://copr.fedorainfracloud.org/
```

---

## 6. Nix / NixOS

```nix
# amalgame.nix
{ lib, stdenv, fetchFromGitHub, gcc, boehmgc, curl }:

stdenv.mkDerivation rec {
  pname = "amalgame";
  version = "0.3.0";

  src = fetchFromGitHub {
    owner = "amalgame-lang";
    repo  = "Amalgame";
    rev   = "v${version}";
    hash  = "sha256-REPLACE";
  };

  nativeBuildInputs = [ gcc ];
  buildInputs       = [ boehmgc curl ];

  buildPhase = ''
    gcc -O2 -Iruntime snapshot/amc_lib.c \
        -lgc -lm -lcurl -o amc
  '';

  installPhase = ''
    mkdir -p $out/bin $out/lib/amalgame
    cp amc $out/bin/
    cp runtime/_runtime.h $out/lib/amalgame/
  '';

  meta = with lib; {
    description = "Modern programming language that transpiles to C";
    homepage    = "https://github.com/amalgame-lang/Amalgame";
    license     = licenses.asl20;
    platforms   = platforms.unix;
    maintainers = [];
  };
}
```

Submit to [nixpkgs](https://github.com/NixOS/nixpkgs) via PR.

---

## Recommended timeline

| Month | Action |
|---|---|
| Now | GitHub Releases + install.sh + Homebrew tap |
| +1 month | AUR (if Arch users request it) |
| +2 months | Homebrew-core PR (after gaining stars) |
| +3 months | .deb / .rpm / Nix (when user base grows) |

---

## 7. Windows

### Option A — PowerShell installer (no admin needed)

Users run in PowerShell:
```powershell
irm https://raw.githubusercontent.com/amalgame-lang/Amalgame/main/install/windows/install.ps1 | iex
```

This downloads the `.zip` from GitHub Releases, installs to `%LOCALAPPDATA%\Amalgame\`, and adds it to the user PATH.

### Option B — Inno Setup `.exe` installer (recommended for general public)

GitHub Actions builds the XDG-style release tarball
(`amc-<ver>-windows-x86_64.zip`) on tag push. The `.exe` setup is
built **separately on a Windows host with Inno Setup 6+ installed**,
pointing at the staged tarball:

```cmd
:: 1. Download + extract the .zip from the GitHub Release
::    so dist\amc-<ver>-windows-x86_64\bin\amc.exe exists

:: 2. (Optional) Stage a MinGW-w64 toolchain so `amc build` works
::    without a separate MSYS2 install on the end-user box:
::      install\windows\gcc-bundle\
::        ├─ bin\gcc.exe
::        ├─ bin\*.dll
::        └─ ... (lib/, libexec/, etc.)
::    Download a portable release from https://winlibs.com/
::    (e.g. winlibs-x86_64-posix-seh-gcc-13.2.0-mingw-w64ucrt-*.zip),
::    extract its `mingw64\` content into `install\windows\gcc-bundle\`.
::    The .iss preprocessor auto-detects bin\gcc.exe and conditionally
::    ships the tree under {app}\gcc + prepends {app}\gcc\bin to PATH.
::    Skip this step if you want a minimal installer (~20 MB instead
::    of ~200 MB) and accept that end users must install MSYS2/MinGW
::    themselves before running `amc build`.

:: 3. Run iscc, pointing at the staged tarball
cd install\windows
iscc /DAmcVersion=0.8.1 ^
     /DAmcStageDir=..\..\dist\amc-0.8.1-windows-x86_64 ^
     amalgame.iss
```

The `.exe` installer:
- Installs `amc.exe` + the bundled MinGW runtime DLLs to `{app}\bin`
- Installs the XDG tree `{app}\share\amalgame\{runtime,lib,docs}`
  (`Program.ResolveRuntimeDir` picks it up — **no AMC_RUNTIME env
  variable needed** as of v0.8.1+)
- Optionally bundles a MinGW-w64 toolchain under `{app}\gcc` when
  `install\windows\gcc-bundle\bin\gcc.exe` exists at build time
- Adds `{app}\bin` (and `{app}\gcc\bin` if bundled) to user PATH
- Provides a proper uninstaller via Windows Add/Remove Programs

### Option C — winget (Windows Package Manager)

Once you have stable releases, submit to winget:
```bash
# Fork https://github.com/microsoft/winget-pkgs
# Add manifests/b/amalgame-lang/Amalgame/0.3.0/
# Submit PR
```

Manifest format:
```yaml
# amalgame-lang.Amalgame.yaml
PackageIdentifier: amalgame-lang.Amalgame
PackageVersion: 0.3.0
PackageName: Amalgame
Publisher: Bastien MOUGET
License: Apache-2.0
PackageUrl: https://github.com/amalgame-lang/Amalgame
Installers:
  - Architecture: x64
    InstallerType: inno
    InstallerUrl: https://github.com/amalgame-lang/Amalgame/releases/download/v0.3.0/amalgame-0.3.0-windows-setup.exe
    InstallerSha256: REPLACE_WITH_SHA256
ManifestType: singleton
ManifestVersion: 1.4.0
```
