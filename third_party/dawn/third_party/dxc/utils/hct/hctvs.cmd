@echo off

if "%1"=="/?" goto :showhelp
if "%1"=="-?" goto :showhelp
if "%1"=="-help" goto :showhelp
if "%1"=="--help" goto :showhelp

if "%HLSL_BLD_DIR%"=="" (
  echo Missing build directory.
  exit /b 1
)

if not exist "%HLSL_BLD_DIR%\LLVM.sln" (
  echo Missing solution file at %HLSL_BLD_DIR%\LLVM.sln
  exit /b 1
)

if not exist "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe" (
  start %HLSL_BLD_DIR%\LLVM.sln
) else (
  start "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe" %HLSL_BLD_DIR%\LLVM.sln
)

goto :eof

:showhelp
echo Launches Visual Studio and opens the solution file.
echo.
echo  hctvs
echo.
echo VS is expected to be at "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe"
echo The solution is expected to be at %HLSL_BLD_DIR%\LLVM.sln
echo.
