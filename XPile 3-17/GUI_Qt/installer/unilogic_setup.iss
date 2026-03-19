; unilogic_setup.iss -- Inno Setup script for UniLogic
; Bundles: Compiler, Project Manager (Qt GUI), ul.bat wrapper, VS Code extension

#define MyAppName "UniLogic"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "UniLogic"
#define MyAppExeName "UniLogicProjectManager.exe"
#define MyAppURL "https://albazzaztariq.github.io/UniLogic/"

#define DeployDir "..\deploy"
#define IconFile "..\unilogic.ico"
#define VsixFile "..\..\LSP\vscode-ul\unilogic-0.1.0.vsix"
#define RepoRoot "..\.."

[Setup]
AppId={{E4A2F8B1-7C3D-4E5F-9A1B-2D3C4E5F6A7B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName=UniLogic {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={autopf}\UniLogic
DefaultGroupName=UniLogic
OutputBaseFilename=UniLogic_Setup_{#MyAppVersion}
OutputDir=output
SetupIconFile={#IconFile}
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
LicenseFile=
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
MinVersion=10.0

[Messages]
WelcomeLabel1=Welcome to UniLogic {#MyAppVersion} Setup
WelcomeLabel2=UniLogic is a programming language that compiles to C, Python, JavaScript, LLVM IR, and WebAssembly from a single source file.%n%nThis wizard will install the UniLogic compiler, the Project Manager GUI, the ul command-line tool, and optionally the VS Code extension.%n%nPython 3.10+ is required.
FinishedLabel=UniLogic has been installed successfully.%n%nThe ul command is ready to use from any terminal. Try: ul --version%n%nOr open the Project Manager to create your first project.

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon";   Description: "Create a desktop shortcut for Project Manager";  GroupDescription: "Shortcuts:"
Name: "addtopath";     Description: "Add UniLogic to system PATH (enables the 'ul' command from any terminal)"; GroupDescription: "System configuration:"; Flags: checkedonce
Name: "installvsix";   Description: "Install the UniLogic VS Code extension for syntax highlighting and diagnostics"; GroupDescription: "VS Code integration:"; Flags: unchecked; Check: IsVSCodeInstalled

[Files]
; ── Compiler source (the actual language) ────────────────────────────────
; Main entry point
Source: "{#RepoRoot}\Main.py";               DestDir: "{app}\compiler"; Flags: ignoreversion

; Core pipeline
Source: "{#RepoRoot}\Lexer\*.py";            DestDir: "{app}\compiler\Lexer";    Flags: ignoreversion
Source: "{#RepoRoot}\Parser\*.py";           DestDir: "{app}\compiler\Parser";   Flags: ignoreversion
Source: "{#RepoRoot}\AST\*.py";              DestDir: "{app}\compiler\AST";      Flags: ignoreversion
Source: "{#RepoRoot}\Semantic\*.py";         DestDir: "{app}\compiler\Semantic"; Flags: ignoreversion
Source: "{#RepoRoot}\Codegen\*.py";          DestDir: "{app}\compiler\Codegen";  Flags: ignoreversion
Source: "{#RepoRoot}\Compiler\*.py";         DestDir: "{app}\compiler\Compiler"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#RepoRoot}\REPL\*.py";             DestDir: "{app}\compiler\REPL";     Flags: ignoreversion skipifsourcedoesntexist
Source: "{#RepoRoot}\Tools\*.py";            DestDir: "{app}\compiler\Tools";    Flags: ignoreversion skipifsourcedoesntexist

; Standard library
Source: "{#RepoRoot}\Stdlib\*";              DestDir: "{app}\compiler\Stdlib";   Flags: ignoreversion recursesubdirs skipifsourcedoesntexist

; ── Project Manager (Qt GUI) ────────────────────────────────────────────
Source: "{#DeployDir}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Qt DLLs
Source: "{#DeployDir}\Qt6Core.dll";    DestDir: "{app}"; Flags: ignoreversion
Source: "{#DeployDir}\Qt6Gui.dll";     DestDir: "{app}"; Flags: ignoreversion
Source: "{#DeployDir}\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#DeployDir}\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

; MinGW runtime
Source: "{#DeployDir}\libgcc_s_seh-1.dll";  DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#DeployDir}\libstdc++-6.dll";     DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#DeployDir}\libwinpthread-1.dll";  DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

; Qt plugins
Source: "{#DeployDir}\platforms\*";            DestDir: "{app}\platforms";            Flags: ignoreversion recursesubdirs
Source: "{#DeployDir}\styles\*";               DestDir: "{app}\styles";               Flags: ignoreversion recursesubdirs skipifsourcedoesntexist
Source: "{#DeployDir}\imageformats\*";         DestDir: "{app}\imageformats";         Flags: ignoreversion recursesubdirs skipifsourcedoesntexist
Source: "{#DeployDir}\networkinformation\*";   DestDir: "{app}\networkinformation";   Flags: ignoreversion recursesubdirs skipifsourcedoesntexist

; ── Other files ──────────────────────────────────────────────────────────
Source: "{#IconFile}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RepoRoot}\ul.bat"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

; VS Code extension
Source: "{#VsixFile}"; DestDir: "{tmp}"; Flags: ignoreversion skipifsourcedoesntexist; Tasks: installvsix

[Icons]
Name: "{group}\UniLogic Project Manager";   Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\unilogic.ico"
Name: "{group}\Uninstall UniLogic";          Filename: "{uninstallexe}"
Name: "{commondesktop}\UniLogic Project Manager"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\unilogic.ico"; Tasks: desktopicon

[Run]
Filename: "code"; Parameters: "--install-extension ""{tmp}\unilogic-0.1.0.vsix"""; StatusMsg: "Installing VS Code extension..."; Tasks: installvsix; Flags: nowait skipifnotsilent shellexec skipifdoesntexist
Filename: "{app}\{#MyAppExeName}"; Description: "Open UniLogic Project Manager"; Flags: nowait postinstall skipifsilent

[Registry]
; Pre-set the compiler path so Project Manager and ul.bat find it immediately
Root: HKCU; Subkey: "Software\UniLogic\ProjectManager\build"; ValueType: string; ValueName: "compilerDir"; ValueData: "{app}\compiler"

[Code]
function IsPythonInstalled(): Boolean;
var
  ResultCode: Integer;
begin
  Result := Exec('cmd', '/c python --version >nul 2>nul', '', SW_HIDE, ewWaitUntilTerminated, ResultCode)
            and (ResultCode = 0);
  if Result then Exit;
  Result := Exec('cmd', '/c py -3 --version >nul 2>nul', '', SW_HIDE, ewWaitUntilTerminated, ResultCode)
            and (ResultCode = 0);
  if Result then Exit;
  Result := FileExists(ExpandConstant('{localappdata}\Programs\Python\Python312\python.exe'))
         or FileExists(ExpandConstant('{localappdata}\Programs\Python\Python311\python.exe'))
         or FileExists(ExpandConstant('{localappdata}\Programs\Python\Python310\python.exe'));
end;

function IsVSCodeInstalled(): Boolean;
var
  ResultCode: Integer;
begin
  Result := Exec('cmd', '/c where code >nul 2>nul', '', SW_HIDE, ewWaitUntilTerminated, ResultCode)
            and (ResultCode = 0);
end;

function InitializeSetup(): Boolean;
var
  Rc: Integer;
begin
  Result := True;
  if not IsPythonInstalled() then
  begin
    if MsgBox('Python 3.10 or later is required to compile UniLogic programs.' + #13#10 + #13#10 +
              'Python was not detected on this system. The installer will continue, ' +
              'but you will need Python before you can build or run .ul files.' + #13#10 + #13#10 +
              'Would you like to open python.org/downloads to install it now?',
              mbConfirmation, MB_YESNO) = IDYES then
    begin
      ShellExec('open', 'https://www.python.org/downloads/', '', '', SW_SHOW, ewNoWait, Rc);
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  OldPath: String;
begin
  if CurStep = ssPostInstall then
  begin
    if WizardIsTaskSelected('addtopath') then
    begin
      RegQueryStringValue(HKCU, 'Environment', 'Path', OldPath);
      if Pos(ExpandConstant('{app}'), OldPath) = 0 then
      begin
        RegWriteStringValue(HKCU, 'Environment', 'Path',
                            OldPath + ';' + ExpandConstant('{app}'));
        Log('Added ' + ExpandConstant('{app}') + ' to user PATH');
      end;
    end;
  end;
end;
