@echo off

if "%1"=="/?" goto :showhelp
if "%1"=="-?" goto :showhelp
if "%1"=="-help" goto :showhelp
if "%1"=="--help" goto :showhelp

rem Default build arch is x64
if "%BUILD_ARCH%"=="" (
  set BUILD_ARCH=x64
)

if "%1"=="-x86" (
  set BUILD_ARCH=Win32
) else if "%1"=="-Win32" (
  set BUILD_ARCH=Win32
) else if "%1"=="-x64" (
  set BUILD_ARCH=x64
) else if "%1"=="-amd64" (
  set BUILD_ARCH=x64
) else if "%1"=="-arm" (
  set BUILD_ARCH=ARM
) else if "%1"=="-arm64" (
  set BUILD_ARCH=ARM64
) else (
  goto :donearch
)
shift /1

:donearch 
echo Default architecture - set BUILD_ARCH=%BUILD_ARCH%
rem Set the following environment variable globally, or start Visual Studio
rem from this command line in order to use 64-bit tools.
set PreferredToolArchitecture=x64

if "%1"=="" (
  echo Source directory missing.
  goto :showhelp
)
if "%2"=="" (
  echo Build directory missing.
  goto :showhelp
)

if not exist "%~f1\utils\hct\hctstart.cmd" (
  echo %1 does not look like a directory with sources - cannot find %~f1\utils\hct\hctstart.cmd
  exit /b 1
)

set HLSL_SRC_DIR=%~f1
set HLSL_BLD_DIR=%~f2
echo HLSL source directory set to HLSL_SRC_DIR=%HLSL_SRC_DIR%
echo HLSL build directory set to HLSL_BLD_DIR=%HLSL_BLD_DIR%
echo.
echo You can recreate the environment with this command.
echo %0 %*
echo.

echo Setting up macros for this console - run hcthelp for a reference.
echo.
doskey hctbld=pushd %HLSL_BLD_DIR%
doskey hctbuild=%HLSL_SRC_DIR%\utils\hct\hctbuild.cmd $*
doskey hctclean=%HLSL_SRC_DIR%\utils\hct\hctclean.cmd $*
doskey hcthelp=%HLSL_SRC_DIR%\utils\hct\hcthelp.cmd $*
doskey hctshortcut=cscript.exe //Nologo %HLSL_SRC_DIR%\utils\hct\hctshortcut.js $*
doskey hctspeak=cscript.exe //Nologo %HLSL_SRC_DIR%\utils\hct\hctspeak.js $*
doskey hctsrc=pushd %HLSL_SRC_DIR%
doskey hcttest=%HLSL_SRC_DIR%\utils\hct\hcttest.cmd $*
doskey hcttools=pushd %HLSL_SRC_DIR%\utils\hct
doskey hcttodo=cscript.exe //Nologo %HLSL_SRC_DIR%\utils\hct\hcttodo.js $*
doskey hctvs=%HLSL_SRC_DIR%\utils\hct\hctvs.cmd $*

call :checksdk
if errorlevel 1 (
  echo Windows SDK not properly installed. Build enviornment could not be setup correctly.
  echo Please see the README.md instructions in the project root.
  exit /b 1
)

where cmake.exe 1>nul 2>nul
if errorlevel 1 (
  call :findcmake
)

where python.exe 1>nul 2>nul
if errorlevel 1 (
  call :findpython
)

call :findminte
where te.exe 1>nul 2>nul
if errorlevel 1 (
  call :findte
)

where git.exe 1>nul 2>nul
if errorlevel 1 (
  call :findgit
)

pushd %HLSL_SRC_DIR%

goto :eof

:showhelp 
echo hctstart - Start the HLSL console tools environment.
echo.
echo This script sets up the sources and binary environment variables
echo and installs convenience console aliases. See hcthelp for a reference.
echo.
echo Usage:
echo  hctstart [-x86 or -x64] [path-to-sources] [path-to-build]
echo.
goto :eof

:findcmake 
for %%e in (Community Professional Enterprise) do (
  rem check VS 2022 in programfiles first
  if exist "%programfiles%\Microsoft Visual Studio\2022\%%e\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin" (
    set "PATH=%PATH%;%programfiles%\Microsoft Visual Studio\2022\%%e\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
    echo Path adjusted to include cmake from Visual Studio 2022 %%e.
    exit /b 0
  )
  rem then check VS 2019 in programfiles(x86)
  if exist "%programfiles(x86)%\Microsoft Visual Studio\2019\%%e\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin" (
    set "PATH=%PATH%;%programfiles(x86)%\Microsoft Visual Studio\2019\%%e\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
    echo Path adjusted to include cmake from Visual Studio 2019 %%e.
    exit /b 0
  )
)
if errorlevel 1 if exist "%programfiles%\CMake\bin" set path=%path%;%programfiles%\CMake\bin
if errorlevel 1 if exist "%programfiles(x86)%\CMake\bin" set path=%path%;%programfiles(x86)%\CMake\bin
where cmake.exe 1>nul 2>nul
if errorlevel 1 (
  echo Unable to find cmake on path - you will have to add this before building.
  exit /b 1
)
echo Path adjusted to include cmake.
goto :eof

:findminte 
set HLSL_TAEF_DIR=
set HLSL_TAEF_MINTE=
if exist "%HLSL_SRC_DIR%\external\taef\build\Binaries\%BUILD_ARCH:Win32=x86%\Te.exe" set HLSL_TAEF_MINTE=%HLSL_SRC_DIR%\external\taef\build\Binaries\%BUILD_ARCH:Win32=x86%
if exist "%programfiles%\windows kits\10\Testing\Runtimes\TAEF\%BUILD_ARCH:Win32=x86%\MinTe\Te.exe" set HLSL_TAEF_MINTE=%programfiles%\windows kits\10\Testing\Runtimes\TAEF\%BUILD_ARCH:Win32=x86%\MinTe
if exist "%programfiles(x86)%\windows kits\10\Testing\Runtimes\TAEF\%BUILD_ARCH:Win32=x86%\MinTe\Te.exe" set HLSL_TAEF_MINTE=%programfiles(x86)%\windows kits\10\Testing\Runtimes\TAEF\%BUILD_ARCH:Win32=x86%\MinTe
if exist "%programfiles%\windows kits\10\Testing\Runtimes\TAEF\MinTe\Te.exe" set HLSL_TAEF_MINTE=%programfiles%\windows kits\10\Testing\Runtimes\TAEF\MinTe
if exist "%programfiles(x86)%\windows kits\10\Testing\Runtimes\TAEF\MinTe\Te.exe" set HLSL_TAEF_MINTE=%programfiles(x86)%\windows kits\10\Testing\Runtimes\TAEF\MinTe
if "%HLSL_TAEF_MINTE%"=="" (
  echo Unable to find matching MinTe, will not auto-copy AgilitySDK binaries.
  exit /b 1
)
echo Found TAEF at %HLSL_TAEF_MINTE%
set HLSL_TAEF_DIR=%HLSL_BLD_DIR%\TAEF
echo Copying to %HLSL_TAEF_DIR% for use with AgilitySDK
mkdir "%HLSL_TAEF_DIR%\%BUILD_ARCH:Win32=x86%" 1>nul 2>nul
robocopy /NP /NJH /NJS /S "%HLSL_TAEF_MINTE%" "%HLSL_TAEF_DIR%\%BUILD_ARCH:Win32=x86%" *
set path=%path%;%HLSL_TAEF_DIR%\%BUILD_ARCH:Win32=x86%
goto:eof

:findte 
if exist "%programfiles%\windows kits\10\Testing\Runtimes\TAEF\Te.exe" set path=%path%;%programfiles%\windows kits\10\Testing\Runtimes\TAEF
if exist "%programfiles(x86)%\windows kits\10\Testing\Runtimes\TAEF\Te.exe" set path=%path%;%programfiles(x86)%\windows kits\10\Testing\Runtimes\TAEF
if exist "%programfiles%\windows kits\8.1\Testing\Runtimes\TAEF\Te.exe" set path=%path%;%programfiles%\windows kits\8.1\Testing\Runtimes\TAEF
if exist "%programfiles(x86)%\windows kits\8.1\Testing\Runtimes\TAEF\Te.exe" set path=%path%;%programfiles(x86)%\windows kits\8.1\Testing\Runtimes\TAEF
if exist "%HLSL_SRC_DIR%\external\taef\build\Binaries\amd64\TE.exe" set path=%path%;%HLSL_SRC_DIR%\external\taef\build\Binaries\amd64
where te.exe 1>nul 2>nul
if errorlevel 1 (
  echo Unable to find TAEF te.exe on path - you will have to add this before running tests.
  echo WDK includes TAEF and is available from https://msdn.microsoft.com/en-us/windows/hardware/dn913721.aspx
  echo Alternatively, consider a project-local install by running %HLSL_SRC_DIR%\utils\hct\hctgettaef.py
  echo Please see the README.md instructions in the project root.
  exit /b 1
)
echo Path adjusted to include TAEF te.exe.

:findgit 
if exist "%programfiles(x86)%\Git\cmd\git.exe" set path=%path%;%programfiles(x86)%\Git\cmd
if exist "%programfiles%\Git\cmd\git.exe" set path=%path%;%programfiles%\Git\cmd
if exist "%LOCALAPPDATA%\Programs\Git\cmd\git.exe" set path=%path%;%LOCALAPPDATA%\Programs\Git\cmd
where git 1>nul 2>nul
if errorlevel 1 (
  echo Unable to find git. Having git is convenient but not necessary to build and test.
)
echo Path adjusted to include git.
goto :eof

:findpython 
if exist C:\Python37\python.exe set path=%path%;C:\Python37
where python.exe 1>nul 2>nul
if errorlevel 1 (
  echo Unable to find python.
  exit /b 1
)
echo Path adjusted to include python.
goto :eof

:checksdk 
setlocal
set min_sdk_ver=17763

set REG_QUERY=REG QUERY "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\Windows\v10.0"
set kit_root=
for /F "tokens=1,2*" %%A in ('%REG_QUERY% /v InstallationFolder') do (
  if "%%A"=="InstallationFolder" (
    rem echo Found Windows 10 SDK
    rem echo   InstallationFolder: "%%C"
    set kit_root=%%C
  )
)
if ""=="%kit_root%" (
    set "kit_root=%WIN10_SDK_PATH%"
)
if ""=="%kit_root%" (
  echo Did not find a Windows 10 SDK installation.
  exit /b 1
)
if not exist "%kit_root%" (
  echo Windows 10 SDK was installed but is not accessible.
  exit /b 1
)

set sdk_ver=
set d3d12_sdk_ver=
for /F "tokens=1-3" %%A in ('%REG_QUERY% /v ProductVersion') do (
  if "%%A"=="ProductVersion" (
    rem echo       ProductVersion: %%C
    for /F "tokens=1-3 delims=." %%X in ("%%C") do (
      set sdk_ver=%%Z
      if exist "%kit_root%\include\10.0.%%Z.0\um\d3d12.h" (
        set d3d12_sdk_ver=%%Z
      )
    )
  )
)
if ""=="%sdk_ver%" (
  set sdk_ver=%WIN10_SDK_VERSION%
)
if ""=="%sdk_ver%" (
  echo Could not detect Windows 10 SDK version.
  exit /b 1
)
if NOT %min_sdk_ver% LEQ %sdk_ver% (
  echo Found unsupported Windows 10 SDK version 10.0.%sdk_ver%.0 installed.
  echo Windows 10 SDK version 10.0.%min_sdk_ver%.0 or newer is required.
  exit /b 1
)

if ""=="%d3d12_sdk_ver%" (
  echo Windows 10 SDK version 10.0.%sdk_ver%.0 installed, but did not find d3d12.h.
  exit /b 1
) else (
  echo Found Windows 10 SDK 10.0.%d3d12_sdk_ver%.0
)
endlocal
goto :eof

endlocal
