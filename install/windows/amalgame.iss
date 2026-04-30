; ═══════════════════════════════════════════════════════════
;  Amalgame Language — Inno Setup Script
;  Produces a Windows .exe installer
;
;  Requirements: Inno Setup 6+ (https://jrsoftware.org/isinfo.php)
;
;  Usage:
;    iscc amalgame.iss
;
;  Output: Output\amalgame-0.3.0-setup.exe
; ═══════════════════════════════════════════════════════════

#define AppName      "Amalgame"
#define AppVersion   "0.3.0"
#define AppPublisher "Bastien MOUGET"
#define AppURL       "https://github.com/BastienMOUGET/Amalgame"
#define AppExe       "amc.exe"

[Setup]
AppId={{F3A2B1C4-7E8D-4F9A-B3C2-1D5E6F7A8B9C}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}/issues
AppUpdatesURL={#AppURL}/releases
DefaultDirName={autopf}\Amalgame
DefaultGroupName=Amalgame
AllowNoIcons=yes
LicenseFile=..\..\LICENSE
OutputDir=Output
OutputBaseFilename=amalgame-{#AppVersion}-setup
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
Name: "addtopath";      Description: "Add amc to PATH (recommended)"; GroupDescription: "Configuration:"; Flags: checked
Name: "desktopicon";    Description: "Create a desktop shortcut for documentation"; GroupDescription: "Shortcuts:"; Flags: unchecked

[Files]
; Main binary
Source: "..\..\build-windows\amc.exe";          DestDir: "{app}\bin"; Flags: ignoreversion

; Runtime header (needed by compiled programs)
Source: "..\..\src\transpiler\runtime\_runtime.h"; DestDir: "{app}\runtime"; Flags: ignoreversion

; MinGW GCC bundle (bundled for convenience)
Source: "gcc-bundle\*"; DestDir: "{app}\gcc"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: GccBundleExists

; Documentation
Source: "..\..\docs\DEVELOPER_GUIDE.md"; DestDir: "{app}\docs"; Flags: ignoreversion
Source: "..\..\README.md";               DestDir: "{app}";      Flags: ignoreversion

; Uninstaller
Source: "..\..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Amalgame Documentation"; Filename: "{app}\docs\DEVELOPER_GUIDE.md"
Name: "{group}\Uninstall Amalgame";     Filename: "{uninstallexe}"
Name: "{userdesktop}\Amalgame Docs";    Filename: "{app}\docs\DEVELOPER_GUIDE.md"; Tasks: desktopicon

[Registry]
; AMC_RUNTIME environment variable
Root: HKCU; Subkey: "Environment"; ValueType: string; ValueName: "AMC_RUNTIME"; ValueData: "{app}\runtime"; Flags: preservestringtype uninsdeletevalue

[Code]
function GccBundleExists(): Boolean;
begin
  Result := DirExists(ExpandConstant('{src}\gcc-bundle'));
end;

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
    if IsTaskSelected('addtopath') then begin
      AddToPath(ExpandConstant('{app}\bin'));
      // Also add bundled GCC if present
      if GccBundleExists() then
        AddToPath(ExpandConstant('{app}\gcc\bin'));
    end;
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then begin
    RemoveFromPath(ExpandConstant('{app}\bin'));
    RemoveFromPath(ExpandConstant('{app}\gcc\bin'));
  end;
end;

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Messages]
; Customize the welcome page
WelcomeLabel2=This will install [name/ver] on your computer.%n%nAmalgame is a modern programming language that transpiles to C — bringing the best of Kotlin, Rust, F# and Go to your fingertips.%n%nClick Next to continue.
