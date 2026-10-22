@echo off
setlocal ENABLEDELAYEDEXPANSION

set HCT_DIR=%~dp0

if "%BUILD_CONFIG%"=="" (
  set BUILD_CONFIG=Debug
)

:opt_loop
if "%1"=="" (goto :done_opt)

if "%1"=="/?" goto :showhelp
if "%1"=="-?" goto :showhelp
if "%1"=="-h" goto :showhelp
if "%1"=="-help" goto :showhelp
if "%1"=="--help" goto :showhelp

if "%1"=="-rel" (
  set BUILD_CONFIG=Release
) else (
  goto :done_opt
)
shift /1
goto :opt_loop
:done_opt

if "%HLSL_TAEF_DIR%"=="" (
  echo No HLSL_TAEF_DIR is set, no TAEF components will be copied.
)

set FULL_AGILITY_PATH=
if "%HLSL_AGILITYSDK_DIR%"=="" (
  echo No HLSL_AGILITYSDK_DIR is set, no AgilitySDK binaries will be copied.
) else (
  if exist "%HLSL_AGILITYSDK_DIR%\build\native\bin\%BUILD_ARCH:Win32=x86%\D3D12Core.dll" (
    set FULL_AGILITY_PATH=%HLSL_AGILITYSDK_DIR%\build\native\bin\%BUILD_ARCH:Win32=x86%
  ) else if exist "%HLSL_AGILITYSDK_DIR%\%BUILD_ARCH:Win32=x86%\D3D12Core.dll" (
    set FULL_AGILITY_PATH=%HLSL_AGILITYSDK_DIR%\%BUILD_ARCH:Win32=x86%
  ) else if exist "%HLSL_AGILITYSDK_DIR%\D3D12Core.dll" (
    set FULL_AGILITY_PATH=%HLSL_AGILITYSDK_DIR%
  ) else (
    echo HLSL_AGILITYSDK_DIR is set, but unable to resolve path to binaries
  )
)

if exist "%HLSL_BLD_DIR%\%BUILD_CONFIG%\bin" (
  call :copytobin "%HLSL_BLD_DIR%\%BUILD_CONFIG%\bin"
)
if exist "%HLSL_BLD_DIR%\%BUILD_CONFIG%\test" (
  call :copytobin "%HLSL_BLD_DIR%\%BUILD_CONFIG%\test"
)
goto :eof


:copytobin
if not "%HLSL_TAEF_DIR%"=="" (
  call %HCT_DIR%\hctcopy.cmd "%HLSL_TAEF_DIR%\%BUILD_ARCH:Win32=x86%" "%~1" TE.Common.dll Wex.Common.dll Wex.Communication.dll Wex.Logger.dll
)
if not "%FULL_AGILITY_PATH%"=="" (
  mkdir "%~1\D3D12" 1>nul 2>nul
  call %HCT_DIR%\hctcopy.cmd "%FULL_AGILITY_PATH%" "%~1\D3D12" D3D12Core.dll d3d12SDKLayers.dll
)
goto :eof

:showhelp 
echo Usage:
echo   hctbins [-rel]
echo.
echo Copies extra binary dependencies to bin and test outputs for tools such as HLSLHost.exe.
goto :eof
