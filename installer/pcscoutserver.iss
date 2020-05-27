; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "XeroCentral"
#define MySecondName "XeroScout"
#define MyAppVersion "0.1.19"
#define MyAppPublisher "ErrorCodeXero"
#define MyAppURL "http://www.wilsonvillerobotics.com/"
#define MyAppExeName "PCScouter.exe"
#define MyScoutAppExeName "PCScoutApp.exe"
#define MyFormViewerExeName "PCFormViewer.exe"
#define MyUserName "ButchGriffin"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{3FF7C27E-BF12-48A1-A349-5BE6EE0EA824}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=C:\cygwin64\home\{#MyUserName}\projects\scouting\license.txt
; Remove the following line to run in administrative install mode (install for all users.)
PrivilegesRequired=lowest
OutputDir=C:\cygwin64\home\{#MyUserName}\projects\scouting\installer
OutputBaseFilename=xeroscout
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\cygwin64\home\ButchGriffin\projects\scouting\PCScouter\x64\Release\*.*"; DestDir: "{app}"; Flags: ignoreversion 64bit recursesubdirs

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{autoprograms}\{#MyScoutAppExeName}"; Filename: "{app}\{#MyScoutAppExeName}"
Name: "{autodesktop}\{#MyScoutAppExeName}"; Filename: "{app}\{#MyScoutAppExeName}"; Tasks: desktopicon
Name: "{autoprograms}\{#MyFormViewerExeName}"; Filename: "{app}\{#MyFormViewerExeName}"
Name: "{autodesktop}\{#MyFormViewerExeName}"; Filename: "{app}\{#MyFormViewerExeName}"; Tasks: desktopicon

[Run]
Filename: {app}\VC_redist.x64.exe; \
    Parameters: "/q /passive /Q:a /c:""msiexec /q /i vcredist.msi"""; \
    StatusMsg: "Installing VC++ 2019 Redistributables..."
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

