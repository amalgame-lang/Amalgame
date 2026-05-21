# ════════════════════════════════════════════════════════════════════
#  postinstall.ps1 — fires after MSI InstallFiles / before RemoveFiles
# ════════════════════════════════════════════════════════════════════
#
# Invoked by the MSI's CustomActions to replicate the InnoSetup
# tasks that the MSI Environment / Shortcut tables can't cover:
#
#   -Mode install
#       • Install the bundled VS Code .vsix into every detected
#         VS Code variant on PATH (code / code-insiders / codium /
#         code-oss). Silent no-op if no `code` binary is found.
#       • Scaffold a "MyFirstApp" project at
#         %USERPROFILE%\Amalgame\samples\MyFirstApp via
#         `amc new MyFirstApp --vscode`.
#       • Both steps are skipped if their respective MSI property
#         (INSTALL_VSCODE_EXT=0 / INSTALL_SAMPLE=0) is set on the
#         msiexec command line. Defaults are on.
#
#   -Mode uninstall
#       • Best-effort `code --uninstall-extension amalgame.amalgame`
#         on every detected VS Code variant.
#       • The sample project under %USERPROFILE%\Amalgame\samples\
#         is left alone — it's user data, not installation state.
#
# Errors are caught and logged but never propagate as an MSI
# rollback trigger. A flaky `code` install shouldn't undo the
# main file copy.

[CmdletBinding()]
param(
    [Parameter(Mandatory)] [ValidateSet("install","uninstall")] [string] $Mode,
    [string] $InstallLocation = (Split-Path -Parent (Split-Path -Parent $PSCommandPath))
)

$ErrorActionPreference = "Continue"

$LogPath = Join-Path $env:TEMP "amalgame-postinstall.log"
function Log {
    param([string]$Msg)
    "$([DateTime]::UtcNow.ToString('yyyy-MM-ddTHH:mm:ssZ')) [$Mode] $Msg" | Add-Content $LogPath
}

Log "begin ($($PSCommandPath))"
Log "InstallLocation = $InstallLocation"

# ── VS Code extension install / uninstall ────────────────────
$cliVariants = @("code", "code-insiders", "codium", "code-oss")

function Find-VsixUnderInstall {
    $dir = Join-Path $InstallLocation "share\amalgame\editors\vscode"
    if (-not (Test-Path $dir)) { return $null }
    $matches = Get-ChildItem -Path $dir -Filter "amalgame-*.vsix" -ErrorAction SilentlyContinue
    if ($null -eq $matches -or $matches.Count -eq 0) { return $null }
    return $matches[0].FullName
}

if ($Mode -eq "install") {
    $vsix = Find-VsixUnderInstall
    if ($null -ne $vsix) {
        Log "vsix at $vsix"
        foreach ($cli in $cliVariants) {
            try {
                $p = Get-Command $cli -ErrorAction SilentlyContinue
                if ($null -eq $p) { continue }
                Log "installing extension via $cli"
                & cmd.exe /c "$cli --install-extension `"$vsix`" --force" 2>&1 | Add-Content $LogPath
            } catch {
                Log "vsix install via $cli failed: $($_.Exception.Message)"
            }
        }
    } else {
        Log "no vsix found under $InstallLocation\share\amalgame\editors\vscode"
    }
} else {
    # uninstall mode — best-effort removal
    foreach ($cli in $cliVariants) {
        try {
            $p = Get-Command $cli -ErrorAction SilentlyContinue
            if ($null -eq $p) { continue }
            Log "uninstalling extension via $cli"
            & cmd.exe /c "$cli --uninstall-extension amalgame.amalgame" 2>&1 | Add-Content $LogPath
        } catch {
            Log "uninstall via $cli failed: $($_.Exception.Message)"
        }
    }
}

# ── Sample scaffold (install only) ───────────────────────────
if ($Mode -eq "install") {
    $userProfile = $env:USERPROFILE
    if ([string]::IsNullOrEmpty($userProfile)) {
        Log "USERPROFILE not set — skipping sample scaffold"
    } else {
        $sampleParent = Join-Path $userProfile "Amalgame\samples"
        $sampleDir    = Join-Path $sampleParent "MyFirstApp"
        if (Test-Path $sampleDir) {
            Log "sample dir already exists at $sampleDir — leaving alone"
        } else {
            $null = New-Item -ItemType Directory -Path $sampleParent -Force -ErrorAction SilentlyContinue
            $amcExe = Join-Path $InstallLocation "bin\amc.exe"
            if (-not (Test-Path $amcExe)) {
                Log "amc.exe not found at $amcExe — skipping scaffold"
            } else {
                Log "scaffolding sample via $amcExe"
                try {
                    Push-Location $sampleParent
                    & $amcExe new MyFirstApp --vscode 2>&1 | Add-Content $LogPath
                } catch {
                    Log "scaffold failed: $($_.Exception.Message)"
                } finally {
                    Pop-Location
                }
            }
        }
    }
}

Log "end"
exit 0
