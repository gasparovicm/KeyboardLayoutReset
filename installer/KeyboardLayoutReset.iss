; Inno Setup script for KeyboardLayoutReset. Built by build.bat into dist\.
; Per-user install: no administrator rights needed.
;
; Silent install with settings:
;   KeyboardLayoutReset-Setup-x.y.z.exe /SILENT /LAYOUT=en-US /IDLE=60

#define AppName "KeyboardLayoutReset"
#define AppExe "KeyboardLayoutReset.exe"
; Name used up to 0.2.0. The AppId is unchanged, so setup upgrades an old
; install in place and removes its leftovers (see [InstallDelete]).
#define OldName "LayoutReset"
#define AppVersion GetStringFileInfo(AddBackslash(SourcePath) + "..\bin\" + AppExe, "ProductVersion")

[Setup]
AppId={{C3437FFF-BC67-45C7-B530-91A4419A888E}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=Martin Gasparovic
AppPublisherURL=https://github.com/gasparovicm/KeyboardLayoutReset
DefaultDirName={localappdata}\Programs\{#AppName}
; Don't reuse the old install's LayoutReset folder.
UsePreviousAppDir=no
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename={#AppName}-Setup-{#AppVersion}
SetupIconFile=..\src\app.ico
UninstallDisplayIcon={app}\{#AppExe}
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
; A running copy is closed in [Code] (CloseRunningApp) instead of asking the
; user to close it; setup starts it again at the end.
CloseApplications=no

[Tasks]
Name: autostart; Description: "Start {#AppName} when I sign in to Windows"

[InstallDelete]
Type: filesandordirs; Name: "{localappdata}\Programs\{#OldName}"
Type: files; Name: "{userstartup}\{#OldName}.lnk"
Type: files; Name: "{autoprograms}\{#OldName}.lnk"

[Files]
Source: "..\bin\{#AppExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExe}"; Parameters: "{code:GetArgs}"
Name: "{userstartup}\{#AppName}"; Filename: "{app}\{#AppExe}"; Parameters: "{code:GetArgs}"; \
  Tasks: autostart

[Run]
Filename: "{app}\{#AppExe}"; Parameters: "{code:GetArgs}"; Description: "Start {#AppName} now"; \
  Flags: nowait postinstall

[Code]
var
  SettingsPage: TWizardPage;
  LayoutCombo: TNewComboBox;
  IdleEdit: TNewEdit;

{ Command-line value (/LAYOUT=, /IDLE=) wins, then the previous install's value. }
function InitialValue(const Name, Default: String): String;
begin
  Result := ExpandConstant('{param:' + Name + '}');
  if Result = '' then
    Result := GetPreviousData(Name, Default);
end;

function AddLabel(const Caption: String; Top: Integer): TNewStaticText;
begin
  Result := TNewStaticText.Create(SettingsPage);
  Result.Parent := SettingsPage.Surface;
  Result.Caption := Caption;
  Result.Top := Top;
  Result.Width := SettingsPage.SurfaceWidth;
  Result.WordWrap := True;
end;

procedure InitializeWizard;
begin
  SettingsPage := CreateCustomPage(wpSelectTasks, 'Settings',
    'Choose the default keyboard and how long to wait before switching back to it.');

  AddLabel('Default keyboard to switch back to:', 0);
  LayoutCombo := TNewComboBox.Create(SettingsPage);
  LayoutCombo.Parent := SettingsPage.Surface;
  LayoutCombo.Top := ScaleY(18);
  LayoutCombo.Width := ScaleX(200);
  LayoutCombo.Style := csDropDown;
  LayoutCombo.Items.Add('en');
  LayoutCombo.Items.Add('en-US');
  LayoutCombo.Items.Add('en-GB');
  LayoutCombo.Items.Add('sk-SK');
  LayoutCombo.Items.Add('cs-CZ');
  LayoutCombo.Items.Add('de-DE');
  LayoutCombo.Text := InitialValue('Layout', 'en');
  AddLabel('"en" means the first English keyboard you have installed. You can also type ' +
    'any language name (fr-FR) or a layout id (00000409).', ScaleY(46));

  AddLabel('Switch after this many seconds without keyboard or mouse input ' +
    '(0 = switch every 5 seconds, even while typing):', ScaleY(100));
  IdleEdit := TNewEdit.Create(SettingsPage);
  IdleEdit.Parent := SettingsPage.Surface;
  IdleEdit.Top := ScaleY(136);
  IdleEdit.Width := ScaleX(80);
  IdleEdit.Text := InitialValue('Idle', '60');
end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
  Idle: Integer;
begin
  Result := True;
  if CurPageID = SettingsPage.ID then
  begin
    Idle := StrToIntDef(Trim(IdleEdit.Text), -1);
    if (Idle < 0) or (Idle > 86400) then
    begin
      MsgBox('Enter the idle time as a number of seconds between 0 and 86400.', mbError, MB_OK);
      Result := False;
    end
    else if Trim(LayoutCombo.Text) = '' then
    begin
      MsgBox('Choose a default keyboard.', mbError, MB_OK);
      Result := False;
    end;
  end;
end;

procedure RegisterPreviousData(PreviousDataKey: Integer);
begin
  SetPreviousData(PreviousDataKey, 'Layout', Trim(LayoutCombo.Text));
  SetPreviousData(PreviousDataKey, 'Idle', Trim(IdleEdit.Text));
end;

function GetArgs(Param: String): String;
begin
  Result := '--layout ' + Trim(LayoutCombo.Text) + ' --idle ' + Trim(IdleEdit.Text);
end;

const
  WM_CLOSE = $0010;

{ Asks a running copy to exit, like tray > Exit. The app's hidden window has
  its name as class name; its mutex lives until the process has exited and
  released the exe. }
procedure RequestClose(const Name: String);
var
  Wnd: HWND;
begin
  Wnd := FindWindowByClassName(Name);
  if Wnd <> 0 then
    PostMessage(Wnd, WM_CLOSE, 0, 0);
end;

function IsRunning(const Name: String): Boolean;
begin
  Result := (FindWindowByClassName(Name) <> 0) or CheckForMutexes('Local\' + Name);
end;

{ Closes the app, also under its old name, and waits for it. Returns False if
  it is still running after a few seconds. }
function CloseRunningApp: Boolean;
var
  I: Integer;
begin
  RequestClose('{#AppName}');
  RequestClose('{#OldName}');
  for I := 1 to 50 do
  begin
    Result := not IsRunning('{#AppName}') and not IsRunning('{#OldName}');
    if Result then
      Exit;
    Sleep(100);
  end;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  if not CloseRunningApp then
    Result := '{#AppName} is still running and could not be closed. ' +
      'Exit it from its tray icon and run setup again.';
end;

function InitializeUninstall: Boolean;
begin
  Result := True;
  if not CloseRunningApp then
  begin
    MsgBox('{#AppName} is still running and could not be closed. ' +
      'Exit it from its tray icon, then uninstall again.', mbError, MB_OK);
    Result := False;
  end;
end;

{ Autostart unticked on an upgrade: remove the shortcut an earlier install added. }
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep = ssPostInstall) and not WizardIsTaskSelected('autostart') then
    DeleteFile(ExpandConstant('{userstartup}\{#AppName}.lnk'));
end;
