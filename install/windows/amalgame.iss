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
;  Optional opt-in: drop a MinGW-w64 toolchain under
;  install\windows\gcc-bundle\ (must contain bin\gcc.exe). When
;  present, the installer ships it under {app}\gcc and prepends
;  {app}\gcc\bin to PATH so `amc build` works on a fresh box
;  without an MSYS2 install. See PUBLISHING.md for staging steps.
;
;  Output: Output\amalgame-<version>-setup.exe
; ═══════════════════════════════════════════════════════════

#ifndef AmcVersion
  #define AmcVersion "0.8.1"
#endif
#ifndef AmcStageDir
  #define AmcStageDir "..\..\dist\amc-" + AmcVersion + "-windows-x86_64"
#endif

; Opt-in toolchain bundling: when install\windows\gcc-bundle\bin\gcc.exe
; exists on disk before `iscc` runs, the resulting setup.exe ships it
; under {app}\gcc and adds {app}\gcc\bin to PATH. Otherwise the source
; block + AddToPath call are excluded outright — no compile warning,
; no runtime branch.
#if FileExists("gcc-bundle\\bin\\gcc.exe")
  #define IncludeGccBundle
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

; Inno Setup tasks are selected by default — there is NO `checked` flag,
; only `unchecked` to opt OUT of the default selection. The previous
; `Flags: checked` was a no-op latent bug (silently ignored by ISCC...
; until v6.7 which now hard-errors on unknown flags).
[Tasks]
Name: "addtopath";    Description: "Add amc to PATH (recommended)";                                                  GroupDescription: "Configuration:"
Name: "vscode_ext";   Description: "Install the Amalgame VS Code extension if VS Code is detected";                  GroupDescription: "Configuration:"
Name: "samplescaffold"; Description: "Create a sample project at %USERPROFILE%\Amalgame\samples\MyFirstApp";         GroupDescription: "Configuration:"
#ifdef IncludeGccBundle
; Auto-deselected by InitializeWizard() when gcc.exe is already on PATH
; (typical MSYS2 / MinGW / Cygwin installs). User can re-check the box
; to force an isolated bundled toolchain.
Name: "usebundledgcc"; Description: "Install bundled MinGW gcc (uncheck if MSYS2/MinGW is already installed)";       GroupDescription: "Toolchain:"
#endif
Name: "desktopicon";  Description: "Create a desktop shortcut for documentation";                                    GroupDescription: "Shortcuts:";    Flags: unchecked

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
; Bug 6 fix (v0.8.26+): ship bundled-stdlib .am facades so the cgen
; can auto-attach them when user code imports the namespace.
Source: "{#AmcStageDir}\share\amalgame\stdlib\*";  DestDir: "{app}\share\amalgame\stdlib";  Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AmcStageDir}\share\amalgame\docs\*";    DestDir: "{app}\share\amalgame\docs";    Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AmcStageDir}\share\amalgame\editors\*"; DestDir: "{app}\share\amalgame\editors"; Flags: ignoreversion recursesubdirs createallsubdirs

; ── Top-level docs + license ──────────────────────────────
Source: "..\..\README.md";          DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\LICENSE";            DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\docs\guide\01-getting-started.md"; DestDir: "{app}\docs"; Flags: ignoreversion
Source: "..\..\docs\DEVELOPER_GUIDE.md";          DestDir: "{app}\docs"; Flags: ignoreversion

; ── Optional bundled MinGW toolchain ──────────────────────
; If install\windows\gcc-bundle\bin\gcc.exe exists at build time
; (drop a MinGW-w64 archive there before running iscc), ship it.
; `amc build` then works out of the box on a fresh Windows host
; with no MSYS2 / no separate gcc install required.
#ifdef IncludeGccBundle
Source: "gcc-bundle\*"; DestDir: "{app}\gcc"; Flags: ignoreversion recursesubdirs createallsubdirs; Tasks: usebundledgcc
#endif

[Icons]
Name: "{group}\Amalgame README";          Filename: "{app}\README.md"
Name: "{group}\Amalgame Getting Started"; Filename: "{app}\docs\01-getting-started.md"
Name: "{group}\Amalgame Online Docs";     Filename: "https://github.com/amalgame-lang/Amalgame/tree/main/docs/guide"
Name: "{group}\Uninstall Amalgame";       Filename: "{uninstallexe}"
Name: "{userdesktop}\Amalgame Docs";      Filename: "{app}\docs\01-getting-started.md"; Tasks: desktopicon

; Note: no [Registry] entry for AMC_RUNTIME. Amalgame v0.8.1+ probes
; <bin>\..\share\amalgame\runtime via GetModuleFileNameA, which
; resolves to {app}\share\amalgame\runtime — the layout above.
; Older amc versions still work via the AMC_RUNTIME env override
; (set it manually if you're pinning amc < 0.8.1).

[Code]
// True if `gcc.exe` is reachable on the system PATH — used to
// auto-deselect the bundled gcc task when MSYS2/MinGW/Cygwin is
// already installed. `where gcc` exits non-zero if not found.
function SystemHasGcc(): Boolean;
var
  ResultCode: Integer;
begin
  Result := False;
  if Exec('cmd.exe', '/c where gcc >nul 2>&1', '',
          SW_HIDE, ewWaitUntilTerminated, ResultCode) then
    Result := (ResultCode = 0);
end;

#ifdef IncludeGccBundle
procedure InitializeWizard();
begin
  // If the host already has gcc on PATH, default "usebundledgcc" off.
  // The wizard task is still visible — user can re-enable for an
  // isolated bundled toolchain — but the friendly default is "don't
  // duplicate ~200 MB of MinGW".
  if SystemHasGcc() then
    WizardSelectTasks('!usebundledgcc');
end;
#endif

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

// Looks for the bundled VS Code extension .vsix under
// {app}\share\amalgame\editors\vscode\amalgame-*.vsix and installs it
// into any VS Code variant on PATH (code, code-insiders, codium,
// code-oss). Silent on failure: a user without VS Code just doesn't
// see anything happen — `where code` returning non-zero is fine.
procedure InstallVSCodeExtension();
var
  FindRec:    TFindRec;
  VsixPath:   string;
  EditorsDir: string;
  ResultCode: Integer;
  CliVariants: TArrayOfString;
  i:          Integer;
begin
  EditorsDir := ExpandConstant('{app}\share\amalgame\editors\vscode\');
  if not FindFirst(EditorsDir + 'amalgame-*.vsix', FindRec) then
    Exit;
  try
    VsixPath := EditorsDir + FindRec.Name;
    SetArrayLength(CliVariants, 4);
    CliVariants[0] := 'code';
    CliVariants[1] := 'code-insiders';
    CliVariants[2] := 'codium';
    CliVariants[3] := 'code-oss';
    for i := 0 to GetArrayLength(CliVariants) - 1 do begin
      // cmd /c so PATHEXT resolves .cmd shims that VS Code installs.
      Exec('cmd.exe',
           '/c ' + CliVariants[i] + ' --install-extension "' + VsixPath + '" --force',
           '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;
  finally
    FindClose(FindRec);
  end;
end;

// Drop a ready-to-open `MyFirstApp` scaffold under
// %USERPROFILE%\Amalgame\samples\ so the user has somewhere to F5
// into without first remembering `amc new`. Idempotent: if the
// directory already exists we leave it alone.
procedure ScaffoldSampleProject();
var
  UserProfile: string;
  ParentDir:   string;
  SampleDir:   string;
  AmcExe:      string;
  ResultCode:  Integer;
begin
  UserProfile := GetEnv('USERPROFILE');
  if UserProfile = '' then Exit;
  ParentDir := UserProfile + '\Amalgame\samples';
  SampleDir := ParentDir + '\MyFirstApp';
  if DirExists(SampleDir) then Exit;
  ForceDirectories(ParentDir);
  AmcExe := ExpandConstant('{app}\bin\amc.exe');
  // Call amc.exe directly with WorkingDir = ParentDir. The previous
  // version wrapped this in `cmd.exe /c cd /d "..." && "..." new ...`
  // — cmd's documented but bizarre quote handling for /c with embedded
  // double quotes around an exe path containing spaces (e.g.
  // `C:\Program Files\Amalgame\bin\amc.exe`) re-tokenised the params
  // and amc ended up creating `'MyFirstApp'` (with literal quotes) and
  // a stray `-p` directory. Going direct dodges the whole problem.
  Exec(AmcExe, 'new MyFirstApp --vscode', ParentDir,
       SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then begin
    if IsTaskSelected('addtopath') then begin
      AddToPath(ExpandConstant('{app}\bin'));
      #ifdef IncludeGccBundle
      // Only prepend the bundled gcc dir if the user kept the task on.
      if IsTaskSelected('usebundledgcc') then
        AddToPath(ExpandConstant('{app}\gcc\bin'));
      #endif
    end;
    if IsTaskSelected('vscode_ext') then
      InstallVSCodeExtension();
    if IsTaskSelected('samplescaffold') then
      ScaffoldSampleProject();
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then begin
    RemoveFromPath(ExpandConstant('{app}\bin'));
    #ifdef IncludeGccBundle
    // Safe to call unconditionally — RemoveFromPath is a no-op if
    // the entry isn't present.
    RemoveFromPath(ExpandConstant('{app}\gcc\bin'));
    #endif
  end;
end;

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Messages]
WelcomeLabel2=This will install [name/ver] on your computer.%n%nAmalgame is a modern programming language that transpiles to C — bringing the best of Kotlin, Rust, F# and Go to your fingertips.%n%nClick Next to continue.
FinishedLabelNoIcons=Setup has finished installing [name] on your computer.%n%nA sample project is at %USERPROFILE%\Amalgame\samples\MyFirstApp. Open it with VS Code (File → Open Folder) and press F5 to build + debug.%n%nOnline docs: https://github.com/amalgame-lang/Amalgame/tree/main/docs/guide
FinishedLabel=Setup has finished installing [name] on your computer.%n%nA sample project is at %USERPROFILE%\Amalgame\samples\MyFirstApp. Open it with VS Code (File → Open Folder) and press F5 to build + debug.%n%nOnline docs: https://github.com/amalgame-lang/Amalgame/tree/main/docs/guide
