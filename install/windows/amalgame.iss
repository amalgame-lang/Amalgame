; ═══════════════════════════════════════════════════════════
;  Amalgame Language — Inno Setup Script
;  Produces a Windows .exe installer matching the XDG-style
;  layout amc looks for: <prefix>\bin\amc.exe plus
;  <prefix>\share\amalgame\{runtime, lib, docs}.
;
;  Requirements: Inno Setup 6+ (https://jrsoftware.org/isinfo.php)
;
;  Source files: Inno Setup is run AFTER the release.yml Windows
;  job has staged the artifacts. Point the staging dir at the
;  extracted release tree:
;
;    iscc /DAmcVersion=0.8.1 ^
;         /DAmcStageDir=..\..\dist\amc-0.8.1-windows-x86_64 ^
;         amalgame.iss
;
;  Layout under {app} after install:
;    {app}\bin\amc.exe + bundled MinGW DLLs
;    {app}\share\amalgame\runtime\_runtime.h + Amalgame_*.h
;    {app}\share\amalgame\lib\libamalgame.a
;    {app}\share\amalgame\docs\language\grammar.ebnf
;    {app}\share\amalgame\docs\guide\02-language-tour.md
;    {app}\README.md + LICENSE
;
;  Output: Output\amalgame-<version>-setup.exe
; ═══════════════════════════════════════════════════════════

#ifndef AmcVersion
  #define AmcVersion "0.8.1"
#endif
#ifndef AmcStageDir
  #define AmcStageDir "..\..\dist\amc-" + AmcVersion + "-windows-x86_64"
#endif

#define AppName      "Amalgame"
#define AppPublisher "Bastien MOUGET"
#define AppURL       "https://github.com/amalgame-lang/Amalgame"
#define AppExe       "amc.exe"

[Setup]
AppId={{F3A2B1C4-7E8D-4F9A-B3C2-1D5E6F7A8B9C}
AppName={#AppName}
AppVersion={#AmcVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}/issues
AppUpdatesURL={#AppURL}/releases
DefaultDirName={autopf}\Amalgame
DefaultGroupName=Amalgame
AllowNoIcons=yes
LicenseFile=..\..\LICENSE
OutputDir=Output
OutputBaseFilename=amalgame-{#AmcVersion}-setup
SetupIconFile=assets\amalgame.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
ChangesEnvironment=yes
ArchitecturesInstallIn64BitMode=x64compatible arm64

; Minimum Windows 10
MinVersion=10.0

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french";  MessagesFile: "compiler:Languages\French.isl"

[Tasks]
Name: "addtopath";   Description: "Add amc to PATH (recommended)";   GroupDescription: "Configuration:"; Flags: checked
Name: "desktopicon"; Description: "Create a desktop shortcut for documentation"; GroupDescription: "Shortcuts:"; Flags: unchecked

[Files]
; ── Binary + bundled MinGW DLLs ───────────────────────────
; release.yml stages amc.exe and the DLL set under bin/ so the
; Windows loader picks them up automatically (same dir as the exe).
Source: "{#AmcStageDir}\bin\*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs createallsubdirs

; ── Runtime headers + libamalgame.a + LLM prompt docs ─────
; amc resolves these via <bin>\..\share\amalgame\{runtime,lib} —
; matches Program.ResolveRuntimeDir / ResolveLibAmalgameA so no
; AMC_RUNTIME env override is needed.
Source: "{#AmcStageDir}\share\amalgame\runtime\*"; DestDir: "{app}\share\amalgame\runtime"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AmcStageDir}\share\amalgame\lib\*";     DestDir: "{app}\share\amalgame\lib";     Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AmcStageDir}\share\amalgame\docs\*";    DestDir: "{app}\share\amalgame\docs";    Flags: ignoreversion recursesubdirs createallsubdirs

; ── Top-level docs + license ──────────────────────────────
Source: "..\..\README.md";          DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\LICENSE";            DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\docs\guide\01-getting-started.md"; DestDir: "{app}\docs"; Flags: ignoreversion
Source: "..\..\docs\DEVELOPER_GUIDE.md";          DestDir: "{app}\docs"; Flags: ignoreversion

[Icons]
Name: "{group}\Amalgame README";        Filename: "{app}\README.md"
Name: "{group}\Amalgame Getting Started"; Filename: "{app}\docs\01-getting-started.md"
Name: "{group}\Uninstall Amalgame";     Filename: "{uninstallexe}"
Name: "{userdesktop}\Amalgame Docs";    Filename: "{app}\docs\01-getting-started.md"; Tasks: desktopicon

; Note: no [Registry] entry for AMC_RUNTIME. Amalgame v0.8.1+ probes
; <bin>\..\share\amalgame\runtime via GetModuleFileNameA, which
; resolves to {app}\share\amalgame\runtime — the layout above.
; Older amc versions still work via the AMC_RUNTIME env override
; (set it manually if you're pinning amc < 0.8.1).

[Code]
procedure AddToPath(Path: string);
var
  CurrentPath: string;
begin
  if not RegQueryStringValue(HKCU, 'Environment', 'PATH', CurrentPath) then
    CurrentPath := '';
  if Pos(LowerCase(Path), LowerCase(CurrentPath)) = 0 then begin
    if CurrentPath = '' then
      CurrentPath := Path
    else
      CurrentPath := Path + ';' + CurrentPath;
    RegWriteStringValue(HKCU, 'Environment', 'PATH', CurrentPath);
  end;
end;

procedure RemoveFromPath(Path: string);
var
  CurrentPath: string;
  NewPath:     string;
  Parts:       TStringList;
  i:           Integer;
begin
  if not RegQueryStringValue(HKCU, 'Environment', 'PATH', CurrentPath) then
    Exit;
  Parts := TStringList.Create;
  try
    Parts.Delimiter       := ';';
    Parts.StrictDelimiter := True;
    Parts.DelimitedText   := CurrentPath;
    NewPath := '';
    for i := 0 to Parts.Count - 1 do begin
      if LowerCase(Parts[i]) <> LowerCase(Path) then begin
        if NewPath <> '' then NewPath := NewPath + ';';
        NewPath := NewPath + Parts[i];
      end;
    end;
    RegWriteStringValue(HKCU, 'Environment', 'PATH', NewPath);
  finally
    Parts.Free;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then begin
    if IsTaskSelected('addtopath') then
      AddToPath(ExpandConstant('{app}\bin'));
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
    RemoveFromPath(ExpandConstant('{app}\bin'));
end;

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Messages]
WelcomeLabel2=This will install [name/ver] on your computer.%n%nAmalgame is a modern programming language that transpiles to C — bringing the best of Kotlin, Rust, F# and Go to your fingertips.%n%nClick Next to continue.
