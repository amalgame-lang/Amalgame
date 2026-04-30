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

1. Create a new GitHub repo: `BastienMOUGET/homebrew-amalgame`
2. Copy `install/homebrew/amalgame.rb` into it as `Formula/amalgame.rb`
3. Update the `sha256` value:
   ```bash
   curl -sSL https://github.com/BastienMOUGET/Amalgame/archive/refs/tags/v0.3.0.tar.gz | sha256sum
   ```
4. Users install with:
   ```bash
   brew tap BastienMOUGET/amalgame
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
url="https://github.com/BastienMOUGET/Amalgame"
license=('Apache-2.0')
depends=('glib2' 'libgee' 'gc' 'gcc')
makedepends=('vala' 'meson' 'ninja')
source=("$pkgname-$pkgver.tar.gz::https://github.com/BastienMOUGET/Amalgame/archive/refs/tags/v$pkgver.tar.gz")
sha256sums=('REPLACE_WITH_SHA256')

build() {
    cd "Amalgame-$pkgver"
    meson setup build --buildtype=release --prefix=/usr
    ninja -C build
}

package() {
    cd "Amalgame-$pkgver"
    DESTDIR="$pkgdir" ninja -C build install
    install -Dm644 src/transpiler/runtime/_runtime.h \
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
Build-Depends: debhelper (>= 10), valac, libglib2.0-dev,
               libgee-0.8-dev, meson, ninja-build, libgc-dev
Standards-Version: 4.6.0
Homepage: https://github.com/BastienMOUGET/Amalgame

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
sudo dnf install rpm-build rpmdevtools vala glib2-devel \
                 libgee-devel meson ninja-build gc-devel

rpmdev-setuptree

cat > ~/rpmbuild/SPECS/amalgame.spec << 'SPEC'
Name:           amalgame
Version:        0.3.0
Release:        1%{?dist}
Summary:        Modern programming language that transpiles to C
License:        Apache-2.0
URL:            https://github.com/BastienMOUGET/Amalgame
Source0:        https://github.com/BastienMOUGET/Amalgame/archive/refs/tags/v%{version}.tar.gz

BuildRequires:  vala
BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  glib2-devel
BuildRequires:  libgee-devel
BuildRequires:  gc-devel

Requires:       gcc
Requires:       gc

%description
Amalgame distills the best features from today's most productive
languages into a single, modern, statically-typed language.

%prep
%autosetup -n Amalgame-%{version}

%build
%meson
%meson_build

%install
%meson_install
install -Dm644 src/transpiler/runtime/_runtime.h \
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
{ lib, stdenv, fetchFromGitHub, meson, ninja, pkg-config,
  vala, glib, libgee, boehmgc, gcc }:

stdenv.mkDerivation rec {
  pname = "amalgame";
  version = "0.3.0";

  src = fetchFromGitHub {
    owner = "BastienMOUGET";
    repo  = "Amalgame";
    rev   = "v${version}";
    hash  = "sha256-REPLACE";
  };

  nativeBuildInputs = [ meson ninja pkg-config vala ];
  buildInputs       = [ glib libgee boehmgc ];

  postInstall = ''
    mkdir -p $out/lib/amalgame
    cp src/transpiler/runtime/_runtime.h $out/lib/amalgame/
  '';

  meta = with lib; {
    description = "Modern programming language that transpiles to C";
    homepage    = "https://github.com/BastienMOUGET/Amalgame";
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
irm https://raw.githubusercontent.com/BastienMOUGET/Amalgame/main/install/windows/install.ps1 | iex
```

This downloads the `.zip` from GitHub Releases, installs to `%LOCALAPPDATA%\Amalgame\`, and adds it to the user PATH.

### Option B — Inno Setup `.exe` installer (recommended for general public)

The GitHub Actions workflow (`release-windows.yml`) builds `amalgame-0.3.0-windows-setup.exe` automatically when you push a tag.

The `.exe` installer:
- Installs `amc.exe` and the runtime header
- Optionally bundles MinGW GCC
- Adds `amc` to PATH
- Sets `AMC_RUNTIME` environment variable
- Provides a proper uninstaller via Windows Add/Remove Programs

### Option C — winget (Windows Package Manager)

Once you have stable releases, submit to winget:
```bash
# Fork https://github.com/microsoft/winget-pkgs
# Add manifests/b/BastienMOUGET/Amalgame/0.3.0/
# Submit PR
```

Manifest format:
```yaml
# BastienMOUGET.Amalgame.yaml
PackageIdentifier: BastienMOUGET.Amalgame
PackageVersion: 0.3.0
PackageName: Amalgame
Publisher: Bastien MOUGET
License: Apache-2.0
PackageUrl: https://github.com/BastienMOUGET/Amalgame
Installers:
  - Architecture: x64
    InstallerType: inno
    InstallerUrl: https://github.com/BastienMOUGET/Amalgame/releases/download/v0.3.0/amalgame-0.3.0-windows-setup.exe
    InstallerSha256: REPLACE_WITH_SHA256
ManifestType: singleton
ManifestVersion: 1.4.0
```
