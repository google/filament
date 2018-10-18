; Setup script for use with Inno Setup.

[Setup]
AppName=Open Asset Import Library - SDK
AppVerName=Open Asset Import Library - SDK (v4.1.0)
DefaultDirName={pf}\Assimp
DefaultGroupName=Assimp
UninstallDisplayIcon={app}\bin\x86\assimp.exe
OutputDir=out
AppCopyright=Assimp Development Team
SetupIconFile=..\..\tools\shared\assimp_tools_icon.ico
WizardImageFile=compiler:WizModernImage-IS.BMP
WizardSmallImageFile=compiler:WizModernSmallImage-IS.BMP
LicenseFile=License.rtf
OutputBaseFileName=assimp-sdk-4.1.0-setup
VersionInfoVersion=4.1.0.0
VersionInfoTextVersion=4.1.0
VersionInfoCompany=Assimp Development Team
ArchitecturesInstallIn64BitMode=x64

[Types]
Name: "full";    Description: "Full installation"
Name: "compact"; Description: "Compact installation, no test models or language bindings"
Name: "custom";  Description: "Custom installation"; Flags: iscustom

[Components]
Name: "main";        Description: "Main Files (32 and 64 Bit)"; Types: full compact custom; Flags: fixed
Name: "tools";       Description: "Asset Viewer & Command Line Tools (32 and 64 Bit)"; Types: full compact
Name: "help";        Description: "Help Files"; Types: full compact
Name: "samples";     Description: "Samples"; Types: full
Name: "test";        Description: "Test Models (BSD-licensed)"; Types: full
Name: "test_nonbsd"; Description: "Test Models (other (free) licenses)"; Types: full
;Name: "pyassimp";    Description: "Python Bindings"; Types: full
;Name: "dassimp";     Description: "D Bindings"; Types: full
;Name: "assimp_net";  Description: "C#/.NET Bindings"; Types: full

[Run]
;Filename: "{app}\stub\vc_redist.x86.exe"; Parameters: "/qb"; StatusMsg: "Installing VS2017 redistributable package (32 Bit)"; Check: not IsWin64
Filename: "{app}\stub\vc_redist.x64.exe"; Parameters: "/qb"; StatusMsg: "Installing VS2017 redistributable package (64 Bit)"; Check: IsWin64

[Files]
Source: "readme_installer.txt"; DestDir: "{app}"; Flags: isreadme

; Installer stub
;Source: "vc_redist.x86.exe"; DestDir: "{app}\stub\"; Check: not IsWin64
Source: "vc_redist.x64.exe"; DestDir: "{app}\stub\"; Check: IsWin64

; Common stuff
Source: "..\..\CREDITS"; DestDir: "{app}"
Source: "..\..\LICENSE"; DestDir: "{app}"
Source: "..\..\README"; DestDir: "{app}"
Source: "WEB"; DestDir: "{app}"

Source: "..\..\scripts\*"; DestDir: "{app}\scripts"; Flags: recursesubdirs

; x86 binaries
;Source: "..\..\bin\release\x86\assimp-vc140-mt.dll";  DestDir: "{app}\bin\x86"
;Source: "..\..\bin\release\x86\assimp_viewer.exe";    DestDir: "{app}\bin\x86"; Components: tools
;Source: "C:\Windows\SysWOW64\D3DCompiler_42.dll";     DestDir: "{app}\bin\x86"; Components: tools
;Source: "C:\Windows\SysWOW64\D3DX9_42.dll";           DestDir: "{app}\bin\x86"; Components: tools
;Source: "..\..\bin\release\x86\assimp.exe";           DestDir: "{app}\bin\x86"; Components: tools

; x64 binaries
Source: "..\..\bin\release\assimp-vc140-mt.dll";  DestDir: "{app}\bin\x64"
Source: "..\..\bin\release\assimp_viewer.exe";    DestDir: "{app}\bin\x64"; Components: tools
Source: "C:\Windows\SysWOW64\D3DCompiler_42.dll"; DestDir: "{app}\bin\x64"; DestName: "D3DCompiler_42.dll"; Components: tools
Source: "C:\Windows\SysWOW64\D3DX9_42.dll";       DestDir: "{app}\bin\x64"; DestName: "D3DX9_42.dll"; Components: tools
Source: "..\..\bin\release\assimp.exe";           DestDir: "{app}\bin\x64"; Components: tools

; Documentation
;Source: "..\..\doc\AssimpDoc_Html\AssimpDoc.chm"; DestDir: "{app}\doc"; Components: help
;Source: "..\..\doc\AssimpCmdDoc_Html\AssimpCmdDoc.chm"; DestDir: "{app}\doc"; Components: help
;Source: "..\..\doc\datastructure.xml"; DestDir: "{app}\doc"; Components: help

; Import libraries
;Source: "..\..\lib\release\x86\assimp.lib"; DestDir: "{app}\lib\x86"
Source: "..\..\lib\release\assimp-vc140-mt.lib"; DestDir: "{app}\lib\x64"

; Samples
Source: "..\..\samples\*"; DestDir: "{app}\samples"; Flags: recursesubdirs; Components: samples

; Include files
Source: "..\..\include\*"; DestDir: "{app}\include"; Flags: recursesubdirs

; dAssimp
;Source: "..\..\port\dAssimp\*"; DestDir: "{app}\port\D"; Flags: recursesubdirs; Components: dassimp

; Assimp.NET
;Source: "..\..\port\Assimp.NET\*"; DestDir: "{app}\port\C#"; Flags: recursesubdirs; Components: assimp_net

; PyAssimp
;Source: "..\..\port\PyAssimp\*"; DestDir: "{app}\port\Python"; Excludes: "*.pyc,*.dll"; Flags: recursesubdirs; Components: pyassimp

; Test repository
;Source: "..\..\test\models\*"; DestDir: "{app}\test\models"; Flags: recursesubdirs; Components: test
;Source: "..\..\test\regression\*"; DestDir: "{app}\test\regression"; Flags: recursesubdirs; Components: test
;Source: "..\..\test\models-nonbsd\*"; DestDir: "{app}\test\models-nonbsd"; Flags: recursesubdirs; Components: test_nonbsd

[Icons]
Name: "{group}\Assimp Manual"; Filename: "{app}\doc\AssimpDoc.chm" ; Components: help
Name: "{group}\Assimp Command Line Manual"; Filename: "{app}\doc\AssimpCmdDoc.chm"; Components: help
Name: "{group}\AssimpView"; Filename: "{app}\bin\x64\assimp_view.exe"; Components: tools; Check: IsWin64
Name: "{group}\AssimpView"; Filename: "{app}\bin\x86\assimp_view.exe"; Components: tools; Check: not IsWin64
