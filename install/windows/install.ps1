# ═══════════════════════════════════════════════════════════
#  Amalgame Language — Windows Installer (PowerShell)
#  https://github.com/BastienMOUGET/Amalgame
#
#  Usage (PowerShell as Administrator):
#    irm https://raw.githubusercontent.com/BastienMOUGET/Amalgame/main/install/windows/install.ps1 | iex
#
#  Or with options:
#    $env:AMC_VERSION="0.3.0"; irm https://... | iex
# ═══════════════════════════════════════════════════════════

param(
    [string]$Version  = $env:AMC_VERSION ?? "latest",
    [string]$Prefix   = $env:AMC_PREFIX  ?? "$env:LOCALAPPDATA\Amalgame",
    [switch]$NoGcc,
    [switch]$Silent
)

$ErrorActionPreference = "Stop"
$ProgressPreference    = "SilentlyContinue"   # speeds up Invoke-WebRequest

# ── Config ────────────────────────────────────────────────
$Repo    = "BastienMOUGET/Amalgame"
$BinDir  = "$Prefix\bin"
$LibDir  = "$Prefix\lib"
$TmpDir  = [System.IO.Path]::GetTempPath() + "amalgame_install"

# ── Colors ────────────────────────────────────────────────
function Info    ($msg) { Write-Host "  -> $msg" -ForegroundColor Cyan }
function Success ($msg) { Write-Host "  v  $msg" -ForegroundColor Green }
function Warn    ($msg) { Write-Host "  !  $msg" -ForegroundColor Yellow }
function Header  ($msg) { Write-Host "`n$msg" -ForegroundColor White }
function Fail    ($msg) { Write-Host "  x  $msg" -ForegroundColor Red; exit 1 }

# ── Banner ────────────────────────────────────────────────
Write-Host @"

  +=======================================+
  |   Amalgame Language Installer         |
  |   https://github.com/$Repo  |
  +=======================================+

"@ -ForegroundColor Cyan

# ── Check PowerShell version ──────────────────────────────
if ($PSVersionTable.PSVersion.Major -lt 5) {
    Fail "PowerShell 5.0+ required. Please update Windows."
}

# ── Detect architecture ───────────────────────────────────
Header "Detecting system..."

$Arch = [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture
$Target = switch ($Arch) {
    "X64"   { "windows-x86_64" }
    "Arm64" { "windows-arm64" }
    default { Fail "Unsupported architecture: $Arch" }
}

Info "OS     : Windows $([System.Environment]::OSVersion.Version)"
Info "Arch   : $Arch"
Info "Target : $Target"
Info "Prefix : $Prefix"

# ── Fetch latest version ──────────────────────────────────
Header "Fetching release info..."

if ($Version -eq "latest") {
    $ApiUrl = "https://api.github.com/repos/$Repo/releases/latest"
    try {
        $Release = Invoke-RestMethod -Uri $ApiUrl -Headers @{ "User-Agent" = "amc-installer" }
        $Version = $Release.tag_name -replace "^v", ""
    } catch {
        Fail "Could not fetch latest version: $_"
    }
}

Info "Version: $Version"

$ArchiveName = "amc-$Version-$Target.zip"
$DownloadUrl = "https://github.com/$Repo/releases/download/v$Version/$ArchiveName"
$ChecksumUrl = "https://github.com/$Repo/releases/download/v$Version/checksums.sha256"

# ── Download ──────────────────────────────────────────────
Header "Downloading Amalgame $Version..."

New-Item -ItemType Directory -Force -Path $TmpDir | Out-Null
$ArchivePath = "$TmpDir\$ArchiveName"

try {
    Invoke-WebRequest -Uri $DownloadUrl -OutFile $ArchivePath
    Success "Downloaded $ArchiveName"
} catch {
    Fail "Download failed: $_`nURL: $DownloadUrl"
}

# ── Verify checksum ───────────────────────────────────────
try {
    $ChecksumFile = "$TmpDir\checksums.sha256"
    Invoke-WebRequest -Uri $ChecksumUrl -OutFile $ChecksumFile
    $Expected = (Get-Content $ChecksumFile | Where-Object { $_ -match $ArchiveName }) -split "\s+" | Select-Object -First 1
    $Actual   = (Get-FileHash $ArchivePath -Algorithm SHA256).Hash.ToLower()
    if ($Expected -and $Expected -ne $Actual) {
        Fail "Checksum mismatch!`n  Expected: $Expected`n  Got:      $Actual"
    }
    Success "Checksum verified"
} catch {
    Warn "Could not verify checksum — continuing anyway"
}

# ── Extract ───────────────────────────────────────────────
Header "Installing..."

$ExtractDir = "$TmpDir\extracted"
New-Item -ItemType Directory -Force -Path $ExtractDir | Out-Null
Expand-Archive -Path $ArchivePath -DestinationPath $ExtractDir -Force

$SourceDir = Get-ChildItem $ExtractDir | Select-Object -First 1

# Create install directories
New-Item -ItemType Directory -Force -Path $BinDir | Out-Null
New-Item -ItemType Directory -Force -Path $LibDir | Out-Null

# Install amc.exe
Copy-Item "$($SourceDir.FullName)\amc.exe" "$BinDir\amc.exe" -Force
Success "Binary installed -> $BinDir\amc.exe"

# Install runtime header
if (Test-Path "$($SourceDir.FullName)\runtime\_runtime.h") {
    Copy-Item "$($SourceDir.FullName)\runtime\_runtime.h" "$LibDir\_runtime.h" -Force
    Success "Runtime header -> $LibDir\_runtime.h"
}

# ── GCC (MinGW) ───────────────────────────────────────────
Header "Checking GCC..."

$GccFound = Get-Command gcc -ErrorAction SilentlyContinue

if (-not $GccFound -and -not $NoGcc) {
    Warn "GCC not found."

    # Try winget first (Windows 11 / updated Win10)
    $WinGet = Get-Command winget -ErrorAction SilentlyContinue
    if ($WinGet) {
        Info "Installing MinGW via winget..."
        try {
            winget install --id=MSYS2.MSYS2 -e --silent
            # Add MinGW to PATH for this session
            $MinGWBin = "C:\msys64\mingw64\bin"
            if (Test-Path $MinGWBin) {
                $env:PATH = "$MinGWBin;$env:PATH"
                Success "MinGW installed"
            }
        } catch {
            Warn "winget install failed. Please install MinGW manually:"
            Warn "  https://www.mingw-w64.org/downloads/"
        }
    } else {
        Warn "Please install MinGW-w64 for GCC support:"
        Warn "  https://www.mingw-w64.org/downloads/"
        Warn "  Or via MSYS2: https://www.msys2.org/"
    }
} elseif ($GccFound) {
    Success "GCC found: $($(gcc --version)[0])"
}

# ── Add to PATH ───────────────────────────────────────────
Header "Configuring PATH..."

$CurrentPath = [System.Environment]::GetEnvironmentVariable("PATH", "User")

if ($CurrentPath -notlike "*$BinDir*") {
    [System.Environment]::SetEnvironmentVariable(
        "PATH",
        "$BinDir;$CurrentPath",
        "User"
    )
    # Also update current session
    $env:PATH = "$BinDir;$env:PATH"
    Success "Added $BinDir to user PATH"
} else {
    Info "PATH already configured"
}

# Set AMC_RUNTIME environment variable
[System.Environment]::SetEnvironmentVariable("AMC_RUNTIME", $LibDir, "User")
$env:AMC_RUNTIME = $LibDir
Success "Set AMC_RUNTIME=$LibDir"

# ── Cleanup ───────────────────────────────────────────────
Remove-Item -Recurse -Force $TmpDir -ErrorAction SilentlyContinue

# ── Verify ────────────────────────────────────────────────
Header "Verifying installation..."

try {
    $AmcVersion = & "$BinDir\amc.exe" --version 2>&1
    Success "amc installed successfully!"
    Write-Host ""
    Write-Host $AmcVersion -ForegroundColor Cyan
} catch {
    Fail "Installation failed — amc.exe not working: $_"
}

# ── Done ──────────────────────────────────────────────────
Write-Host @"

  Amalgame is ready!

  Quick start:
    amc hello.am
    .\hello.exe

  Documentation:
    https://github.com/$Repo/blob/main/docs/DEVELOPER_GUIDE.md

  Note: Restart your terminal for PATH changes to take effect.

"@ -ForegroundColor Green
