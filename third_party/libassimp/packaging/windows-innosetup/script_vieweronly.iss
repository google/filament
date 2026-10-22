; Setup script for use with Inno Setup.

[Setup]
AppName=Open Asset Import Library - Viewer
AppVerName=Open Asset Import Library - Viewer (v2.0)
DefaultDirName={pf}\AssimpView
DefaultGroupName=AssimpView
UninstallDisplayIcon={app}\bin\x86\assimp.exe
OutputDir=out_vieweronly
AppCopyright=Assimp Development Team
SetupIconFile=..\..\tools\shared\assimp_tools_icon.ico
WizardImageFile=compiler:WizModernImage-IS.BMP
WizardSmallImageFile=compiler:WizModernSmallImage-IS.BMP
LicenseFile=License.rtf
OutputBaseFileName=assimp-view-2.0-setup
VersionInfoVersion=2.0.0.0
VersionInfoTextVersion=2.0
VersionInfoCompany=Assimp Development Team
ArchitecturesInstallIn64BitMode=x64


[Run]
Filename: "{app}\stub\vcredist_x86.exe"; Parameters: "/qb"; StatusMsg: "Installing VS2008 SP1 redistributable package (32 Bit)"; Check: not IsWin64
Filename: "{app}\stub\vcredist_x64.exe"; Parameters: "/qb"; StatusMsg: "Installing VS2008 SP1 redistributable package (64 Bit)"; Check: IsWin64

[Files]

Source: "readme_installer_vieweronly.txt"; DestDir: "{app}"; Flags: isreadme

; Installer stub
Source: "vcredist_x86.exe"; DestDir: "{app}\stub\"; Check: not IsWin64
Source: "vcredist_x64.exe"; DestDir: "{app}\stub\"; Check: IsWin64

; Common stuff
Source: "..\..\CREDITS"; DestDir: "{app}"
Source: "..\..\LICENSE"; DestDir: "{app}"
Source: "..\..\README"; DestDir: "{app}"
Source: "WEB"; DestDir: "{app}"

; x86 binaries
Source: "..\..\bin\assimp_release-dll_Win32\Assimp32.dll"; DestDir: "{app}\bin\x86"
Source: "..\..\bin\assimpview_release-dll_Win32\assimp_view.exe"; DestDir: "{app}\bin\x86"
Source: "D3DCompiler_42.dll"; DestDir: "{app}\bin\x86"
Source: "D3DX9_42.dll"; DestDir: "{app}\bin\x86"
Source: "..\..\bin\assimpcmd_release-dll_Win32\assimp.exe"; DestDir: "{app}\bin\x86"

; x64 binaries
Source: "..\..\bin\assimp_release-dll_x64\Assimp64.dll"; DestDir: "{app}\bin\x64"
Source: "..\..\bin\assimpview_release-dll_x64\assimp_view.exe"; DestDir: "{app}\bin\x64"
Source: "D3DCompiler_42_x64.dll"; DestDir: "{app}\bin\x64"; DestName: "D3DCompiler_42.dll"
Source: "D3DX9_42_x64.dll"; DestDir: "{app}\bin\x64"; DestName: "D3DX9_42.dll"
Source: "..\..\bin\assimpcmd_release-dll_x64\assimp.exe"; DestDir: "{app}\bin\x64"

; Documentation
Source: "..\..\doc\AssimpCmdDoc_Html\AssimpCmdDoc.chm"; DestDir: "{app}\doc"

[Icons]
Name: "{group}\Assimp Command Line Manual"; Filename: "{app}\doc\AssimpCmdDoc.chm"
Name: "{group}\AssimpView"; Filename: "{app}\bin\x64\assimp_view.exe"; Check: IsWin64
Name: "{group}\AssimpView"; Filename: "{app}\bin\x86\assimp_view.exe"; Check: not IsWin64
