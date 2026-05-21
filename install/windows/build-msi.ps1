# ════════════════════════════════════════════════════════════════════
#  build-msi.ps1 — Pure-PowerShell MSI builder for Amalgame
# ════════════════════════════════════════════════════════════════════
#
# Produces a Windows .msi installer equivalent to the InnoSetup
# script (amalgame.iss). Constraints:
#   - No WiX / WixToolset / heat / candle / light
#   - No third-party MSI authoring SDKs
#   - PowerShell + WindowsInstaller COM API + native makecab.exe only
#
# What works in this version:
#   - Full file install under %ProgramFiles%\Amalgame (or user choice)
#   - HKCU PATH manipulation via the MSI Environment table
#     (append on install, remove on uninstall — no orphan entries)
#   - Start Menu shortcuts (README, Getting Started, Uninstaller)
#   - Optional desktop shortcut (toggled via property)
#   - Proper uninstall (removes everything, restores PATH)
#   - Bundled MinGW gcc opt-in (when gcc-bundle\bin\gcc.exe is present)
#
# Not yet replicated from amalgame.iss (TODO, v2):
#   - VS Code .vsix auto-install (CustomAction running `code --install-extension`)
#   - `amc new MyFirstApp` sample scaffolding (CustomAction)
#   - "gcc already on PATH? auto-deselect bundled gcc" detection
#     (InnoSetup's InitializeWizard equivalent)
#   - French language pack
#
# These three can be added as Type-3138 deferred CustomActions
# (powershell.exe -File [INSTALLLOCATION]share\amalgame\install\postinstall.ps1
# -Mode install). The .ps1 + a postinstall.ps1 script are wired up,
# but the CustomAction rows themselves are commented out below until
# I can validate them on a real Windows box.
#
# Usage:
#   pwsh -File build-msi.ps1 `
#        -Version 0.8.39 `
#        -StageDir ..\..\dist\amc-0.8.39-windows-x86_64
#
# Output: Output\amalgame-<version>.msi
#
# References:
#   https://learn.microsoft.com/en-us/windows/win32/msi/database-tables
#   https://learn.microsoft.com/en-us/windows/win32/msi/standard-actions-reference
#   https://learn.microsoft.com/en-us/windows/win32/msi/installer-object
# ════════════════════════════════════════════════════════════════════

[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $Version,
    [Parameter(Mandatory)] [string] $StageDir,
    [string] $OutputDir   = "Output",
    [string] $LicensePath = "..\..\LICENSE",
    [string] $ReadmePath  = "..\..\README.md",
    [string] $IconPath    = "assets\amalgame.ico",
    [string] $ManufacturerName = "Bastien MOUGET",
    [string] $ProductName      = "Amalgame",
    [string] $UpgradeCode       = "{F3A2B1C4-7E8D-4F9A-B3C2-1D5E6F7A8B9C}",
    [switch] $IncludeGccBundle
)

$ErrorActionPreference = "Stop"

# Verbose error dump — when something throws, print the full
# stack trace + line number + invocation details so the GHA log
# tells us where in build-msi.ps1 the failure happened. Without
# this, PS's default error reporting at script-call boundaries
# masks the internal line and we have to bisect blind.
trap {
    Write-Host ""
    Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Red
    Write-Host "build-msi.ps1 — TRAPPED ERROR" -ForegroundColor Red
    Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Red
    Write-Host "Message    : $($_.Exception.Message)"
    Write-Host "Category   : $($_.CategoryInfo)"
    Write-Host "Target     : $($_.TargetObject)"
    Write-Host "FQEID      : $($_.FullyQualifiedErrorId)"
    if ($_.InvocationInfo) {
        Write-Host "Script     : $($_.InvocationInfo.ScriptName)"
        Write-Host "Line #     : $($_.InvocationInfo.ScriptLineNumber)"
        Write-Host "Position   : $($_.InvocationInfo.PositionMessage)"
    }
    Write-Host ""
    Write-Host "── Script stack trace ──"
    Write-Host $_.ScriptStackTrace
    Write-Host ""
    Write-Host "── PS call stack ──"
    Get-PSCallStack | Format-Table -AutoSize | Out-String | Write-Host
    Write-Host ""
    Write-Host "── Inner exception chain ──"
    $ex = $_.Exception
    while ($ex) {
        Write-Host "  $($ex.GetType().FullName): $($ex.Message)"
        $ex = $ex.InnerException
    }
    exit 1
}

# ── Sanity checks ─────────────────────────────────────────────
if (-not (Test-Path $StageDir)) {
    throw "StageDir not found: $StageDir"
}
$StageDir = (Resolve-Path $StageDir).Path
$BinSrcDir = Join-Path $StageDir "bin"
if (-not (Test-Path $BinSrcDir)) {
    throw "StageDir missing bin/ subdirectory: $BinSrcDir"
}

$ScriptRoot = Split-Path -Parent $PSCommandPath
$AbsLicense = (Resolve-Path (Join-Path $ScriptRoot $LicensePath)).Path
$AbsReadme  = (Resolve-Path (Join-Path $ScriptRoot $ReadmePath )).Path
$AbsIcon    = (Resolve-Path (Join-Path $ScriptRoot $IconPath   )).Path

$OutAbs = Join-Path $ScriptRoot $OutputDir
$null = New-Item -ItemType Directory -Path $OutAbs -Force
$MsiPath = Join-Path $OutAbs "amalgame-$Version.msi"
$CabPath = Join-Path $OutAbs "amalgame-$Version.cab"
$WorkDir = Join-Path $OutAbs "work-$Version"
$null = New-Item -ItemType Directory -Path $WorkDir -Force

# Wipe stale outputs so re-runs don't double-stream files into the
# existing cabinet (we use makecab fresh each time).
foreach ($p in @($MsiPath, $CabPath)) {
    if (Test-Path $p) { Remove-Item $p -Force }
}

Write-Host "── build-msi.ps1 ─────────────────────────────────"
Write-Host "  Product   : $ProductName $Version"
Write-Host "  Stage     : $StageDir"
Write-Host "  Output    : $MsiPath"
Write-Host ""

# ════════════════════════════════════════════════════════════════════
# Stage 1 — Enumerate files to install
# ════════════════════════════════════════════════════════════════════
#
# Build a flat list of (SourcePath, InstallDir, DestName) tuples. The
# install directory hierarchy mirrors the StageDir's bin/share/...
# layout so amc's runtime probes (<bin>\..\share\amalgame\runtime)
# resolve correctly without any AMC_RUNTIME env override.

class StagedFile {
    [string] $SourcePath
    [string] $InstallDirId   # MSI Directory.Directory key
    [string] $InstallDirRel  # relative to $INSTDIR (display only)
    [string] $DestName       # filename under InstallDir
    [string] $FileKey        # MSI File.File key (≤ 72 chars, identifier)
    [string] $ComponentId    # MSI Component.Component key
    [string] $ComponentGuid  # GUID, deterministic per (UpgradeCode, FileKey)
    [long]   $Size
}

function New-DeterministicGuid {
    param([string]$Seed)
    # Stable GUID derived from input via SHA1 → first 16 bytes →
    # standard 8-4-4-4-12 format. Re-builds with the same input
    # produce identical GUIDs so MSI's upgrade detection works.
    #
    # PowerShell array slicing returns `object[]`, not `byte[]` —
    # `[Guid]::new(byte[])` doesn't match the object[] shape and
    # PS falls through to the string ctor which bails with "Guid
    # should contain 32 digits". `[byte[]]` cast forces the type.
    $sha1   = [System.Security.Cryptography.SHA1]::Create()
    $bytes  = $sha1.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($Seed))
    $first16 = [byte[]]($bytes[0..15])
    $guid    = [Guid]::new($first16)
    $sha1.Dispose()
    return "{$($guid.ToString().ToUpper())}"
}

function Get-DirectoryId {
    param([string]$RelPath)
    # Convert "share/amalgame/runtime" into an MSI Directory key like
    # "share_amalgame_runtime". Identifier rules: letters, digits, _, .
    # Max 72 chars (we'll never hit that).
    if ([string]::IsNullOrEmpty($RelPath)) { return "INSTALLLOCATION" }
    return ($RelPath -replace "[\\/]", "_" -replace "[^A-Za-z0-9_]", "_")
}

$staged = [System.Collections.Generic.List[StagedFile]]::new()
$dirIds = [System.Collections.Generic.HashSet[string]]::new()
$dirIds.Add("INSTALLLOCATION") | Out-Null
# Track parent/child links so we can emit them in topological order.
$dirParents = @{ "INSTALLLOCATION" = "ProgramFiles64Folder" }
$dirNames   = @{ "INSTALLLOCATION" = "Amalgame" }

function Register-Directory {
    param([string]$RelPath)
    if ([string]::IsNullOrEmpty($RelPath)) { return "INSTALLLOCATION" }
    $segments = $RelPath -split "[\\/]"
    $accum = @()
    $parentId = "INSTALLLOCATION"
    foreach ($seg in $segments) {
        $accum += $seg
        $id = Get-DirectoryId ($accum -join "/")
        if (-not $dirIds.Contains($id)) {
            $dirIds.Add($id) | Out-Null
            $dirParents[$id] = $parentId
            $dirNames[$id]   = $seg
        }
        $parentId = $id
    }
    return $parentId
}

# Tracks every (InstallRelDir, DestName) pair already staged so a
# duplicate Add-StageFile call (typically caused by a Get-ChildItem
# enumeration following a reparse point or hard link in the gcc
# bundle) becomes a no-op instead of producing a duplicate MSI
# File row + cabinet entry. Without this, makecab.exe aborts with
# "Duplicate file name: Fxxxxxxxxx already defined".
$stagedInstallPaths = [System.Collections.Generic.HashSet[string]]::new(
    [System.StringComparer]::OrdinalIgnoreCase
)
# Sequential counter for FileKey / ComponentId. Guaranteed unique
# without relying on sha1-prefix collision-resistance (we saw two
# colliding short-prefix hashes at ~10K files in v0.8.40-rc1 — not
# a real birthday but apparently enough of a margin to spook the
# release pipeline). Component GUIDs are still deterministic via
# New-DeterministicGuid so MSI upgrade tracking stays stable across
# rebuilds.
$script:stagedCounter = 0

function Add-StageFile {
    param(
        [string]$AbsSource,
        [string]$InstallRelDir,
        [string]$DestName
    )
    if (-not (Test-Path $AbsSource)) { return }
    # Normalise + dedupe.
    $normDir  = $InstallRelDir.Replace("\", "/").TrimEnd("/")
    $instPath = "$normDir/$DestName"
    if (-not $stagedInstallPaths.Add($instPath)) {
        # Already staged — skip silently. Common cause: a recursive
        # Get-ChildItem walked the same file twice via a reparse
        # point.
        return
    }
    $dirId = Register-Directory -RelPath $InstallRelDir
    $script:stagedCounter += 1
    $idx       = $script:stagedCounter
    $fileKey   = "F{0:D8}" -f $idx
    $compId    = "C{0:D8}" -f $idx
    $compGuid  = New-DeterministicGuid "$UpgradeCode|$instPath"
    $sf = [StagedFile]::new()
    $sf.SourcePath     = $AbsSource
    $sf.InstallDirId   = $dirId
    $sf.InstallDirRel  = $InstallRelDir
    $sf.DestName       = $DestName
    $sf.FileKey        = $fileKey
    $sf.ComponentId    = $compId
    $sf.ComponentGuid  = $compGuid
    $sf.Size           = (Get-Item $AbsSource).Length
    $staged.Add($sf)
}

# Walk the entire StageDir recursively. Anything under `bin/` lands
# in INSTALLLOCATION\bin, anything under `share/` mirrors verbatim.
foreach ($file in Get-ChildItem -Path $StageDir -Recurse -File) {
    $rel = $file.FullName.Substring($StageDir.Length).TrimStart("\","/").Replace("\","/")
    $relDir  = Split-Path $rel -Parent
    $relName = Split-Path $rel -Leaf
    if ($null -eq $relDir) { $relDir = "" }
    Add-StageFile -AbsSource $file.FullName -InstallRelDir $relDir -DestName $relName
}

# Top-level docs that aren't in StageDir.
Add-StageFile -AbsSource $AbsReadme  -InstallRelDir ""     -DestName "README.md"
Add-StageFile -AbsSource $AbsLicense -InstallRelDir ""     -DestName "LICENSE"

# postinstall.ps1 — VSIX install + sample scaffold (lives next to
# build-msi.ps1 in the repo, gets staged into the MSI here so the
# CustomActions below can invoke it from INSTALLLOCATION).
$postinstallSrc = Join-Path $ScriptRoot "postinstall.ps1"
if (Test-Path $postinstallSrc) {
    Add-StageFile -AbsSource $postinstallSrc -InstallRelDir "share/amalgame/install" -DestName "postinstall.ps1"
}

# Optional bundled MinGW gcc.
$GccBundleDir = Join-Path $ScriptRoot "gcc-bundle"
if ($IncludeGccBundle -and (Test-Path (Join-Path $GccBundleDir "bin\gcc.exe"))) {
    Write-Host "  + bundled MinGW gcc detected at $GccBundleDir"
    foreach ($file in Get-ChildItem -Path $GccBundleDir -Recurse -File) {
        $rel = $file.FullName.Substring($GccBundleDir.Length).TrimStart("\","/").Replace("\","/")
        $relDir  = "gcc/" + (Split-Path $rel -Parent).Replace("\","/")
        $relName = Split-Path $rel -Leaf
        Add-StageFile -AbsSource $file.FullName -InstallRelDir $relDir.TrimEnd("/") -DestName $relName
    }
}

Write-Host "  $($staged.Count) files staged, $($dirIds.Count) directories"

# ════════════════════════════════════════════════════════════════════
# Stage 2 — Build the cabinet via makecab.exe
# ════════════════════════════════════════════════════════════════════
#
# makecab is shipped natively on every Windows host since XP, no
# external dep. It takes a directive file (.ddf) describing the
# cabinet layout. We map each source file to a short cabinet name
# matching its MSI File.File key, so File.Sequence aligns with the
# cabinet stream order.

$ddfPath = Join-Path $WorkDir "amalgame.ddf"
$ddfLines = [System.Collections.Generic.List[string]]::new()
$ddfLines.Add(".OPTION EXPLICIT")
$ddfLines.Add(".Set CabinetNameTemplate=`"amalgame-$Version.cab`"")
$ddfLines.Add(".Set DiskDirectory1=`"$OutAbs`"")
$ddfLines.Add(".Set DiskDirectoryTemplate=`"$OutAbs`"")
$ddfLines.Add(".Set CompressionType=MSZIP")
$ddfLines.Add(".Set Cabinet=ON")
$ddfLines.Add(".Set Compress=ON")
$ddfLines.Add(".Set MaxDiskSize=0")
$ddfLines.Add(".Set ReservePerCabinetSize=8")
# Each file line: SourcePath /destname=DestName. Whitespace-sensitive.
foreach ($sf in $staged) {
    # Wrap paths in quotes to handle spaces (Program Files in tests).
    $ddfLines.Add("`"$($sf.SourcePath)`" $($sf.FileKey)")
}
Set-Content -Path $ddfPath -Value $ddfLines -Encoding ASCII

Write-Host "  Running makecab.exe…"
$mkOut = & makecab.exe /f $ddfPath /L $WorkDir 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host $mkOut
    throw "makecab exited $LASTEXITCODE"
}
if (-not (Test-Path $CabPath)) {
    throw "Cabinet not produced at expected path: $CabPath"
}
Write-Host "  Cabinet: $CabPath ($((Get-Item $CabPath).Length) bytes)"

# ════════════════════════════════════════════════════════════════════
# Stage 3 — Create MSI database via WindowsInstaller COM
# ════════════════════════════════════════════════════════════════════
#
# COM constants. WindowsInstaller doesn't expose these as enums, so
# we encode the values inline.
$msiOpenDatabaseModeCreate = 3   # create new, fail if exists (we deleted above)
$msiViewModifyInsert       = 1
$msiViewModifyAssign       = 3

$installer = New-Object -ComObject WindowsInstaller.Installer
# Bootstrap via PowerShell COM late-binding — same pattern as the
# Invoke-MSI helper below.
$database = $installer.OpenDatabase($MsiPath, $msiOpenDatabaseModeCreate)

# Load Microsoft.VisualBasic for CallByName. This is THE documented
# way to do COM IDispatch late-binding from .NET — it handles both
# method invocation (CallType.Method) and indexed property setters
# (CallType.Set / CallType.Let). Works on PS 5.1 AND PS 7,
# bypasses the DLR-based COM adapter that's been throwing
# DISP_E_TYPEMISMATCH / op_Addition / etc. on the GHA runner.
# PS 7 on .NET 6+ may need "Microsoft.VisualBasic.Core" instead
# of the legacy "Microsoft.VisualBasic" name. Try the .NET 6 name
# first; fall back to the legacy name silently if not found. If
# both fail, throw — CallByName is the only reliable COM IDispatch
# late-binding path we know that works across PS 5.1 + PS 7.
$vbLoaded = $false
foreach ($asmName in @("Microsoft.VisualBasic.Core", "Microsoft.VisualBasic")) {
    try {
        Add-Type -AssemblyName $asmName -ErrorAction Stop
        $vbLoaded = $true
        Write-Host "  [dbg] loaded $asmName"
        break
    } catch {
        Write-Host "  [dbg] $asmName not loadable: $($_.Exception.Message)"
    }
}
if (-not $vbLoaded) {
    throw "Neither Microsoft.VisualBasic.Core nor Microsoft.VisualBasic could be loaded."
}
# Verify the type is reachable. Throws TypeNotFound if Add-Type
# silently failed.
$null = [Microsoft.VisualBasic.Interaction]
$null = [Microsoft.VisualBasic.CallType]::Method
Write-Host "  [dbg] CallByName reachable"

function Invoke-MSI {
    # Single entry point for every WindowsInstaller method call.
    # CallByName routes through Microsoft.VisualBasic's COM binder
    # (DISPATCH_METHOD). Args MUST be passed INLINE not as an
    # array — CallByName's last param is `params object[]`, and
    # passing `[object[]]@(...)` for that param makes .NET treat
    # the whole array as a SINGLE argument (giving the COM
    # method one arg too many).
    param([object]$Target, [string]$Method, [object[]]$MethodArgs = @())
    Write-Host "  [dbg] Invoke-MSI: $Method ($($MethodArgs.Count) args)"
    $iact  = [Microsoft.VisualBasic.Interaction]
    $cType = [Microsoft.VisualBasic.CallType]::Method
    switch ($MethodArgs.Count) {
        0 { return $iact::CallByName($Target, $Method, $cType) }
        1 { return $iact::CallByName($Target, $Method, $cType, $MethodArgs[0]) }
        2 { return $iact::CallByName($Target, $Method, $cType, $MethodArgs[0], $MethodArgs[1]) }
        3 { return $iact::CallByName($Target, $Method, $cType, $MethodArgs[0], $MethodArgs[1], $MethodArgs[2]) }
    }
    throw "Invoke-MSI: too many args ($($MethodArgs.Count)) — extend the switch"
}

function Set-MsiProperty {
    # Indexed COM property setter via CallByName.
    #   CallType.Set = DISPATCH_PROPERTYPUTREF (object refs)
    #   CallType.Let = DISPATCH_PROPERTYPUT    (value types)
    # WindowsInstaller's Record.StringData / IntegerData /
    # SummaryInformation.Property all expose ONLY PROPPUT (the
    # values are int/string, not object refs), so we MUST use
    # CallType.Let — Set gives DISP_E_MEMBERNOTFOUND.
    # Args passed inline (NOT as object[]) — same reason as above.
    param([object]$Target, [string]$Property, [object[]]$IndexAndValue)
    [void][Microsoft.VisualBasic.Interaction]::CallByName(
        $Target,
        $Property,
        [Microsoft.VisualBasic.CallType]::Let,
        $IndexAndValue[0],
        $IndexAndValue[1]
    )
}

function Insert-Row {
    # INSERT with parameter binding. View.Execute(record) walks the
    # record's columns as values for each `?` placeholder; the engine
    # handles the row insertion in one call. No separate Modify needed.
    param([string]$Sql, [object[]]$Values)
    $view = Invoke-MSI $database "OpenView" @($Sql)
    $record = Invoke-MSI $installer "CreateRecord" @($Values.Count)
    for ($i = 0; $i -lt $Values.Count; $i++) {
        $v = $Values[$i]
        if ($null -eq $v) { continue }
        # PS 7 parses `@($i + 1, [string]$v)` as
        # `$i + @(1, [string]$v)` (comma binds tighter than `+`
        # inside array subexpressions — a known parser quirk),
        # which then tries `int + Object[]` → op_Addition fail.
        # Extracting the index to a typed local sidesteps the
        # whole ambiguity.
        [int]$fieldIdx = $i + 1
        if ($v -is [int] -or $v -is [long]) {
            Set-MsiProperty $record "IntegerData" @($fieldIdx, [int]$v)
        } else {
            Set-MsiProperty $record "StringData"  @($fieldIdx, [string]$v)
        }
    }
    Invoke-MSI $view "Execute" @($record)
    Invoke-MSI $view "Close" @()
}

function Exec-Sql {
    # Schema DDL (CREATE TABLE / ALTER). No record, no parameters.
    param([string]$Sql)
    $view = Invoke-MSI $database "OpenView" @($Sql)
    Invoke-MSI $view "Execute" @($null)
    Invoke-MSI $view "Close" @()
}

Write-Host "  Creating MSI schema…"
Write-Host "  [dbg] DB type: $($database.GetType().FullName)"
Write-Host "  [dbg] Installer type: $($installer.GetType().FullName)"

# ── Schema: CREATE TABLE … ────────────────────────────────────
# Column types follow MSI conventions:
#   CHAR(N)       — text up to N chars
#   LONGCHAR      — text >255 chars
#   SHORT/LONG    — integer
#   OBJECT        — binary stream
# Identifier columns use CHAR(72) with NOT NULL + PRIMARY KEY.

$schemas = @(
    "CREATE TABLE ``Property`` (``Property`` CHAR(72) NOT NULL, ``Value`` LONGCHAR NOT NULL LOCALIZABLE PRIMARY KEY ``Property``)",
    "CREATE TABLE ``Directory`` (``Directory`` CHAR(72) NOT NULL, ``Directory_Parent`` CHAR(72), ``DefaultDir`` CHAR(255) NOT NULL LOCALIZABLE PRIMARY KEY ``Directory``)",
    "CREATE TABLE ``Component`` (``Component`` CHAR(72) NOT NULL, ``ComponentId`` CHAR(38), ``Directory_`` CHAR(72) NOT NULL, ``Attributes`` SHORT NOT NULL, ``Condition`` CHAR(255), ``KeyPath`` CHAR(72) PRIMARY KEY ``Component``)",
    "CREATE TABLE ``Feature`` (``Feature`` CHAR(38) NOT NULL, ``Feature_Parent`` CHAR(38), ``Title`` CHAR(64) LOCALIZABLE, ``Description`` CHAR(255) LOCALIZABLE, ``Display`` SHORT, ``Level`` SHORT NOT NULL, ``Directory_`` CHAR(72), ``Attributes`` SHORT NOT NULL PRIMARY KEY ``Feature``)",
    "CREATE TABLE ``FeatureComponents`` (``Feature_`` CHAR(38) NOT NULL, ``Component_`` CHAR(72) NOT NULL PRIMARY KEY ``Feature_``, ``Component_``)",
    "CREATE TABLE ``File`` (``File`` CHAR(72) NOT NULL, ``Component_`` CHAR(72) NOT NULL, ``FileName`` CHAR(255) NOT NULL LOCALIZABLE, ``FileSize`` LONG NOT NULL, ``Version`` CHAR(72), ``Language`` CHAR(20), ``Attributes`` SHORT, ``Sequence`` SHORT NOT NULL PRIMARY KEY ``File``)",
    "CREATE TABLE ``Media`` (``DiskId`` SHORT NOT NULL, ``LastSequence`` SHORT NOT NULL, ``DiskPrompt`` CHAR(64) LOCALIZABLE, ``Cabinet`` CHAR(255), ``VolumeLabel`` CHAR(32), ``Source`` CHAR(72) PRIMARY KEY ``DiskId``)",
    "CREATE TABLE ``InstallExecuteSequence`` (``Action`` CHAR(72) NOT NULL, ``Condition`` CHAR(255), ``Sequence`` SHORT PRIMARY KEY ``Action``)",
    "CREATE TABLE ``InstallUISequence`` (``Action`` CHAR(72) NOT NULL, ``Condition`` CHAR(255), ``Sequence`` SHORT PRIMARY KEY ``Action``)",
    "CREATE TABLE ``AdminExecuteSequence`` (``Action`` CHAR(72) NOT NULL, ``Condition`` CHAR(255), ``Sequence`` SHORT PRIMARY KEY ``Action``)",
    "CREATE TABLE ``AdvtExecuteSequence`` (``Action`` CHAR(72) NOT NULL, ``Condition`` CHAR(255), ``Sequence`` SHORT PRIMARY KEY ``Action``)",
    "CREATE TABLE ``Environment`` (``Environment`` CHAR(72) NOT NULL, ``Name`` CHAR(255) NOT NULL LOCALIZABLE, ``Value`` CHAR(255) LOCALIZABLE, ``Component_`` CHAR(72) NOT NULL PRIMARY KEY ``Environment``)",
    "CREATE TABLE ``Shortcut`` (``Shortcut`` CHAR(72) NOT NULL, ``Directory_`` CHAR(72) NOT NULL, ``Name`` CHAR(128) NOT NULL LOCALIZABLE, ``Component_`` CHAR(72) NOT NULL, ``Target`` CHAR(72) NOT NULL, ``Arguments`` CHAR(255), ``Description`` CHAR(255) LOCALIZABLE, ``Hotkey`` SHORT, ``Icon_`` CHAR(72), ``IconIndex`` SHORT, ``ShowCmd`` SHORT, ``WkDir`` CHAR(72) PRIMARY KEY ``Shortcut``)",
    "CREATE TABLE ``Icon`` (``Name`` CHAR(72) NOT NULL, ``Data`` OBJECT NOT NULL PRIMARY KEY ``Name``)",
    "CREATE TABLE ``CustomAction`` (``Action`` CHAR(72) NOT NULL, ``Type`` SHORT NOT NULL, ``Source`` CHAR(72), ``Target`` CHAR(255), ``ExtendedType`` LONG PRIMARY KEY ``Action``)"
)
Write-Host "  [dbg] $($schemas.Count) tables to create"
$schemaIdx = 0
foreach ($ddl in $schemas) {
    $schemaIdx += 1
    $tableName = ([regex]::Match($ddl, "CREATE TABLE\s+``([^``]+)``").Groups[1].Value)
    Write-Host "  [dbg] schema $schemaIdx/$($schemas.Count): $tableName"
    Exec-Sql $ddl
}

# ── Property table ────────────────────────────────────────────
$properties = @{
    "ProductCode"        = (New-DeterministicGuid "$UpgradeCode|$Version")
    "ProductName"        = $ProductName
    "ProductVersion"     = $Version
    "Manufacturer"       = $ManufacturerName
    "ProductLanguage"    = "1033"
    "UpgradeCode"        = $UpgradeCode
    "ALLUSERS"           = "2"   # per-machine if elevated, else per-user
    "MSIINSTALLPERUSER"  = "1"   # default to per-user (lowest privs)
    "ARPPRODUCTICON"     = "icon.ico"
    "ARPHELPLINK"        = "https://github.com/amalgame-lang/Amalgame"
    "ARPURLINFOABOUT"    = "https://github.com/amalgame-lang/Amalgame"
    "ARPURLUPDATEINFO"   = "https://github.com/amalgame-lang/Amalgame/releases"
    "ARPNOMODIFY"        = "1"
    "ARPNOREPAIR"        = "1"
    "INSTALLLEVEL"       = "1"
}
foreach ($k in $properties.Keys) {
    Insert-Row "INSERT INTO ``Property`` (``Property``, ``Value``) VALUES (?, ?)" @($k, $properties[$k])
}

# ── Directory table ──────────────────────────────────────────
# Standard top-level dirs every MSI needs:
Insert-Row "INSERT INTO ``Directory`` (``Directory``, ``Directory_Parent``, ``DefaultDir``) VALUES (?, ?, ?)" @("TARGETDIR", $null, "SourceDir")
Insert-Row "INSERT INTO ``Directory`` (``Directory``, ``Directory_Parent``, ``DefaultDir``) VALUES (?, ?, ?)" @("ProgramFiles64Folder", "TARGETDIR", ".")
Insert-Row "INSERT INTO ``Directory`` (``Directory``, ``Directory_Parent``, ``DefaultDir``) VALUES (?, ?, ?)" @("ProgramMenuFolder", "TARGETDIR", ".")
# Topological sort so parents always come before children.
$emitted = @{ "TARGETDIR" = $true; "ProgramFiles64Folder" = $true; "ProgramMenuFolder" = $true }
$pending = @($dirIds | Where-Object { -not $emitted.ContainsKey($_) })
while ($pending.Count -gt 0) {
    $progress = $false
    foreach ($id in @($pending)) {
        $parent = $dirParents[$id]
        if ($emitted.ContainsKey($parent)) {
            Insert-Row "INSERT INTO ``Directory`` (``Directory``, ``Directory_Parent``, ``DefaultDir``) VALUES (?, ?, ?)" @($id, $parent, $dirNames[$id])
            $emitted[$id] = $true
            $progress = $true
        }
    }
    if (-not $progress) { throw "Directory tree has a cycle or orphan: $($pending -join ',')" }
    $pending = @($dirIds | Where-Object { -not $emitted.ContainsKey($_) })
}

# Start Menu shortcut anchor folder under ProgramMenuFolder
Insert-Row "INSERT INTO ``Directory`` (``Directory``, ``Directory_Parent``, ``DefaultDir``) VALUES (?, ?, ?)" @("AMALGAME_MENU", "ProgramMenuFolder", "Amalgame")

# ── Components + Files + FeatureComponents ───────────────────
# One component per file. ComponentId is a deterministic GUID so
# upgrade/repair tracks the same atom across versions of the MSI.
# Attributes = 256 (msidbComponentAttributes64bit) — required for
# every component in an x64 MSI per the MSI spec. Without this the
# Windows Installer treats the component as 32-bit, which on x64
# systems triggers file-system / registry redirection (the install
# silently lands under \Program Files (x86)\ instead of \Program Files\).
$msidbComponentAttributes64bit = 256

$sequence = 0
foreach ($sf in $staged) {
    $sequence += 1
    # KeyPath: same as the file's File key.
    Insert-Row "INSERT INTO ``Component`` (``Component``, ``ComponentId``, ``Directory_``, ``Attributes``, ``Condition``, ``KeyPath``) VALUES (?, ?, ?, ?, ?, ?)" @(
        $sf.ComponentId, $sf.ComponentGuid, $sf.InstallDirId, $msidbComponentAttributes64bit, $null, $sf.FileKey
    )
    Insert-Row "INSERT INTO ``File`` (``File``, ``Component_``, ``FileName``, ``FileSize``, ``Version``, ``Language``, ``Attributes``, ``Sequence``) VALUES (?, ?, ?, ?, ?, ?, ?, ?)" @(
        $sf.FileKey, $sf.ComponentId, $sf.DestName, [int]$sf.Size, $null, $null, [int]512, [int]$sequence
    )
    Insert-Row "INSERT INTO ``FeatureComponents`` (``Feature_``, ``Component_``) VALUES (?, ?)" @("CompleteFeature", $sf.ComponentId)
}

# ── Feature tree (single feature: "Complete") ────────────────
Insert-Row "INSERT INTO ``Feature`` (``Feature``, ``Feature_Parent``, ``Title``, ``Description``, ``Display``, ``Level``, ``Directory_``, ``Attributes``) VALUES (?, ?, ?, ?, ?, ?, ?, ?)" @(
    "CompleteFeature", $null, "Amalgame Compiler", "Amalgame programming language toolchain", [int]2, [int]1, "INSTALLLOCATION", [int]0
)

# ── Media row pointing at the cabinet ────────────────────────
# Sequence number 1..N maps to the cabinet stream order. We've
# kept that lined up with the foreach above.
Insert-Row "INSERT INTO ``Media`` (``DiskId``, ``LastSequence``, ``DiskPrompt``, ``Cabinet``, ``VolumeLabel``, ``Source``) VALUES (?, ?, ?, ?, ?, ?)" @(
    [int]1, [int]$sequence, $null, "#amalgame.cab", $null, $null
)

# ── Environment table (HKCU PATH manipulation) ───────────────
# Two rows so install appends and uninstall removes — without the
# `!` row the bin path would orphan in PATH after uninstall.
# Component_ attaches to amc.exe so the rows fire when amc.exe
# is being installed/uninstalled.
$amcComponent = $null
foreach ($sf in $staged) {
    if ($sf.DestName -eq "amc.exe") { $amcComponent = $sf.ComponentId; break }
}
if ($null -ne $amcComponent) {
    Insert-Row "INSERT INTO ``Environment`` (``Environment``, ``Name``, ``Value``, ``Component_``) VALUES (?, ?, ?, ?)" @(
        "EnvPathAppend", "=-*PATH", "[~];[INSTALLLOCATION]bin", $amcComponent
    )
} else {
    Write-Warning "amc.exe not found in stage — PATH manipulation rows skipped."
}

# ── Icon table (binary stream) ───────────────────────────────
# For parametrised INSERT with `?` placeholders, the correct
# pattern is `View.Execute(record)` — the record's fields BIND
# to the placeholders. `Execute(null) + Modify(Insert, record)`
# is for UPDATE-style fetches, not bulk insert with params.
# (Earlier attempts used Modify because of confusion with the
# update workflow — but `Execute(null)` on a parametrised view
# returns "Execute,Params" since the placeholders aren't filled.)
$iconView = Invoke-MSI $database "OpenView" @("INSERT INTO ``Icon`` (``Name``, ``Data``) VALUES (?, ?)")
$iconRecord = Invoke-MSI $installer "CreateRecord" @(2)
Set-MsiProperty $iconRecord "StringData" @(1, "icon.ico")
Invoke-MSI     $iconRecord "SetStream"   @(2, $AbsIcon)
Invoke-MSI $iconView "Execute" @($iconRecord)
Invoke-MSI $iconView "Close" @()

# ── Shortcuts (Start Menu) ───────────────────────────────────
# Tied to the amc.exe component so they install/uninstall with it.
# That avoids needing a Registry-keypath component just for the
# shortcuts (which would require an extra Registry table). Target
# is `[#<FileKey>]` — MSI's syntax for "absolute path to installed
# file <FileKey>".
$readmeKey = ($staged | Where-Object { $_.DestName -eq "README.md"           -and $_.InstallDirRel -eq "" }).FileKey
$gsKey     = ($staged | Where-Object { $_.DestName -eq "01-getting-started.md" }).FileKey
if ($readmeKey -and $amcComponent) {
    Insert-Row "INSERT INTO ``Shortcut`` (``Shortcut``, ``Directory_``, ``Name``, ``Component_``, ``Target``, ``Arguments``, ``Description``, ``Hotkey``, ``Icon_``, ``IconIndex``, ``ShowCmd``, ``WkDir``) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)" @(
        "S_AmalgameReadme", "AMALGAME_MENU", "Amalgame README", $amcComponent, "[#$readmeKey]", $null, "Open Amalgame README", $null, "icon.ico", [int]0, [int]1, $null
    )
}
if ($gsKey -and $amcComponent) {
    Insert-Row "INSERT INTO ``Shortcut`` (``Shortcut``, ``Directory_``, ``Name``, ``Component_``, ``Target``, ``Arguments``, ``Description``, ``Hotkey``, ``Icon_``, ``IconIndex``, ``ShowCmd``, ``WkDir``) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)" @(
        "S_GettingStarted", "AMALGAME_MENU", "Amalgame Getting Started", $amcComponent, "[#$gsKey]", $null, "Amalgame language tour", $null, "icon.ico", [int]0, [int]1, $null
    )
}

# ── CustomActions (VS Code .vsix + sample scaffold via PowerShell) ──
#
# Both actions invoke the same postinstall.ps1 — installed by the
# MSI under share/amalgame/install/ — with a -Mode flag. We wire
# them as Type 3122 (deferred + no-impersonate + exec + source-from-
# property) so they run in the system context with the install
# session, after files are in place. The Source column names a
# property that holds the powershell.exe path; the Target column
# is the command line.
#
# `[~]` is the formatted-property placeholder MSI substitutes at
# runtime; brackets get expanded by the engine. We also pass
# [INSTALLLOCATION] so the script knows where it's installed.

# Hardcoded path to PowerShell 5.1 — always present on every
# supported Windows host (10/11/Server 2016+). Using [SystemFolder]
# here would seem cleaner, but deferred CustomActions only have
# access to CustomActionData / ProductCode / UserSID at runtime, so
# property placeholders in Type-50 Source don't reliably resolve.
# The literal path sidesteps the issue.
Insert-Row "INSERT INTO ``Property`` (``Property``, ``Value``) VALUES (?, ?)" @(
    "POWERSHELL_EXE", "C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe"
)

# Command-line for the deferred CA. Two notable details:
#   - `[INSTALLLOCATION]` always expands with a trailing backslash;
#     wrapping it in `"…"` produces a closing `\"` which the C
#     runtime arg parser treats as an escaped quote → unterminated
#     string. Mitigation: don't quote-wrap [INSTALLLOCATION] alone;
#     join it with a non-backslash trailing segment so the closing
#     quote isn't preceded by `\`.
#   - We DON'T pass -InstallLocation here — postinstall.ps1 derives
#     it from $PSCommandPath, sidestepping the trailing-backslash
#     question entirely.
$psPostArgsInstall   = '-NoProfile -ExecutionPolicy Bypass -File "[INSTALLLOCATION]share\amalgame\install\postinstall.ps1" -Mode install'
$psPostArgsUninstall = '-NoProfile -ExecutionPolicy Bypass -File "[INSTALLLOCATION]share\amalgame\install\postinstall.ps1" -Mode uninstall'

# Type 1074 = 50 (exec property + cmdline) + 1024 (deferred). We
# DON'T set 2048 (NoImpersonate) — the post-install work writes to
# HKCU and %USERPROFILE%, which only makes sense in the invoking
# user's context. With NoImpersonate the action would run as
# LocalSystem and the vsix install / sample scaffold would target
# the wrong user profile.
Insert-Row "INSERT INTO ``CustomAction`` (``Action``, ``Type``, ``Source``, ``Target``, ``ExtendedType``) VALUES (?, ?, ?, ?, ?)" @(
    "CA_PostInstall", [int]1074, "POWERSHELL_EXE", $psPostArgsInstall, $null
)
Insert-Row "INSERT INTO ``CustomAction`` (``Action``, ``Type``, ``Source``, ``Target``, ``ExtendedType``) VALUES (?, ?, ?, ?, ?)" @(
    "CA_PostUninstall", [int]1074, "POWERSHELL_EXE", $psPostArgsUninstall, $null
)

# ── InstallExecuteSequence (canonical Windows Installer actions) ──
# Numbers match the standard sequence from the MSI docs. See:
# https://learn.microsoft.com/en-us/windows/win32/msi/suggested-installexecutesequence
$execSeq = @(
    @("FindRelatedProducts",  $null, 25),
    @("LaunchConditions",     $null, 100),
    @("ValidateProductID",    $null, 700),
    @("CostInitialize",       $null, 800),
    @("FileCost",             $null, 900),
    @("CostFinalize",         $null, 1000),
    @("InstallValidate",      $null, 1400),
    @("InstallInitialize",    $null, 1500),
    @("ProcessComponents",    $null, 1600),
    @("UnpublishFeatures",    $null, 1800),
    @("RemoveFiles",          $null, 3500),
    @("InstallFiles",         $null, 4000),
    @("WriteEnvironmentStrings", $null, 5200),
    @("RemoveEnvironmentStrings", $null, 3300),
    @("RegisterUser",         $null, 6000),
    @("RegisterProduct",      $null, 6100),
    @("PublishFeatures",      $null, 6300),
    @("PublishProduct",       $null, 6400),
    @("InstallFinalize",      $null, 6600),
    # Custom actions: PostUninstall fires before RemoveFiles so the
    # script is still on disk when we invoke it. PostInstall fires
    # after InstallFiles. The conditions gate by install/uninstall
    # phase — REMOVE="ALL" means user picked uninstall.
    @("CA_PostUninstall",     'REMOVE="ALL"',     3400),
    @("CA_PostInstall",       'NOT Installed',    5500)
)
foreach ($a in $execSeq) {
    Insert-Row "INSERT INTO ``InstallExecuteSequence`` (``Action``, ``Condition``, ``Sequence``) VALUES (?, ?, ?)" @($a[0], $a[1], [int]$a[2])
}

# ── InstallUISequence (silent install still needs these defined) ──
$uiSeq = @(
    @("FindRelatedProducts", $null, 25),
    @("LaunchConditions",    $null, 100),
    @("ValidateProductID",   $null, 700),
    @("CostInitialize",      $null, 800),
    @("FileCost",            $null, 900),
    @("CostFinalize",        $null, 1000),
    @("ExecuteAction",       $null, 1300)
)
foreach ($a in $uiSeq) {
    Insert-Row "INSERT INTO ``InstallUISequence`` (``Action``, ``Condition``, ``Sequence``) VALUES (?, ?, ?)" @($a[0], $a[1], [int]$a[2])
}

# ── Embed the cabinet as a stream (#amalgame.cab) ────────────
# The `#` prefix on the Media.Cabinet column signals "embedded
# in the MSI as a stream"; the stream name (without the #) is
# what we register here.
Write-Host "  Embedding cabinet…"
$streamView = Invoke-MSI $database "OpenView" @("INSERT INTO ``_Streams`` (``Name``, ``Data``) VALUES (?, ?)")
$streamRecord = Invoke-MSI $installer "CreateRecord" @(2)
Set-MsiProperty $streamRecord "StringData" @(1, "amalgame.cab")
Invoke-MSI      $streamRecord "SetStream"   @(2, $CabPath)
# Execute(record) binds the record's fields to the `?` placeholders
# — same pattern as the Icon insert above.
Invoke-MSI $streamView "Execute" @($streamRecord)
Invoke-MSI $streamView "Close" @()

# ── Summary information ──────────────────────────────────────
# These 8 properties are what the Windows shell reads to display
# the MSI's title bar / explorer preview. PIDs from the MSI SDK.
#
# Database.SummaryInformation(maxProps) is an indexed property
# getter (not a method) — CallByName with CallType.Method returns
# DISP_E_MEMBERNOTFOUND. Direct PS COM access dispatches through
# IDispatch's propget path correctly.
# SummaryInformation IS required — Windows Installer refuses any
# MSI without it (error 1620 / 2262 STG_E_FILENOTFOUND on the
# SummaryInformation stream). PS 7's dispatch adapter can't reach
# the IDispatch propput on SummaryInformation.Property through
# any of the tested PS paths (Type.InvokeMember /
# Microsoft.VisualBasic.CallByName / `$obj.Prop(idx)=val`).
#
# WORKAROUND: inline C# helper compiled via Add-Type. C#'s
# reflection layer marshals COM args to IDispatch directly,
# bypassing the PS 7 wrapper that's been failing. Same
# Type.InvokeMember API, different adapter path.
# Defer the SummaryInformation set until AFTER the main Commit
# (below). The PS / C# .NET adapters can't dispatch the IDispatch
# propput for SummaryInformation.Property (decorated
# [id(DISPID_VALUE)], default property — GetIDsOfNames returns
# "Property,Pid" through both reflection and DLR/dynamic paths).
#
# Workaround: spawn cscript.exe with an inline VBScript that opens
# the MSI in TRANSACT mode and sets the SI properties via native
# COM dispatch. VBScript has been the documented way to script
# WindowsInstaller since 1999 — its dispatch hits the
# DISPID_VALUE path correctly.
$siNeedsVbScript = $true

# ── Commit + release COM objects ─────────────────────────────
# We must fully release the COM handles BEFORE spawning cscript,
# otherwise the OLE structured-storage layer still holds the .msi
# file open and cscript's OpenDatabase fails with "Msi API Error".
# Releasing $database alone is not enough — $installer keeps an
# internal session that pins the file. WaitForPendingFinalizers
# forces the RCW finalizer to actually release the native handle.
Invoke-MSI $database "Commit" @()
[void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($database)
$database = $null
[void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($installer)
$installer = $null
[System.GC]::Collect()
[System.GC]::WaitForPendingFinalizers()
[System.GC]::Collect()
Start-Sleep -Milliseconds 200

# Set SummaryInformation via spawned cscript.exe. cscript ships
# on every Windows since the late 90s; VBScript's COM dispatch
# routes to IDispatch::Invoke with DISPID_PROPERTYPUT correctly
# for default-property propputs like SummaryInformation.Property.
if ($siNeedsVbScript) {
    Write-Host "  Setting SummaryInformation via cscript.exe + VBScript…"
    $rev = [Guid]::NewGuid().ToString("B").ToUpper()
    # Build the VBScript. Strings interpolated from PS get escaped
    # for VBScript (double-up any " characters).
    $esc = { param([string]$s) $s.Replace('"', '""') }
    $vbs = @"
Option Explicit
Dim installer, db, si
Set installer = CreateObject("WindowsInstaller.Installer")
' MsiOpenDatabaseModeTransact = 1 — opens for read+write with commit semantics.
Set db = installer.OpenDatabase(WScript.Arguments(0), 1)
Set si = db.SummaryInformation(200)
si.Property(1)  = "1252"
si.Property(2)  = "$(& $esc "$ProductName Installer")"
si.Property(3)  = "$(& $esc "$ProductName $Version")"
si.Property(4)  = "$(& $esc $ManufacturerName)"
si.Property(5)  = "amalgame;compiler;language"
si.Property(6)  = "Amalgame language toolchain"
si.Property(7)  = "x64;1033"
si.Property(9)  = "$rev"
si.Property(14) = 200
si.Property(15) = 2
si.Persist
db.Commit
WScript.Echo "[vbs] SummaryInformation written"
"@
    $vbsPath = Join-Path $env:TEMP "amalgame-msi-si-$Version.vbs"
    Set-Content -Path $vbsPath -Value $vbs -Encoding ASCII
    & cscript.exe //nologo //T:30 $vbsPath $MsiPath
    if ($LASTEXITCODE -ne 0) {
        throw "cscript exited $LASTEXITCODE — SummaryInformation set failed; MSI may be invalid."
    }
    Remove-Item -Path $vbsPath -ErrorAction SilentlyContinue
}
# $database and $installer were both released before the cscript
# step (the file-handle dance).

# Tidy workdir (keep the .cab next to the .msi for debugging).
Remove-Item -Recurse -Force $WorkDir -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "✓ MSI written: $MsiPath ($((Get-Item $MsiPath).Length) bytes)"
Write-Host ""
Write-Host "Verify with:  msiexec /i `"$MsiPath`" /l*v install.log"
Write-Host ""
