@echo off

if "%1"=="/?" goto :showhelp
if "%1"=="-?" goto :showhelp
if "%1"=="-help" goto :showhelp
if "%1"=="--help" goto :showhelp

setlocal ENABLEDELAYEDEXPANSION

set X86_DBG=1
set X86_REL=1
set X86_TST=1
set X64_DBG=1
set X64_REL=1
set X64_TST=1
set ARM64_DBG=1
set ARM64_REL=1
set ARM64_TST=0
set ANALYZE=-analyze
set FV_FLAG=
set CV_FLAG=

if "%1"=="-short" (
  echo Building / testing fewer versions.
  set X86_DBG=0
  set X86_REL=1
  set X86_TST=0
  set X64_DBG=1
  set X64_REL=0
  set X64_TST=1
  set ARM64_DBG=1
  set ARM64_REL=0
  set ARM64_TST=0
  set ANALYZE=
  shift /1
)
if "%1"=="-fv" (
  echo Fixed version flag set for lab verification.
  set FV_FLAG=-fv
  shift /1
)
if "%1"=="-cv" (
  echo Setting the CLANG_VENDOR value.
  set CV_FLAG=-cv %2
  shift /1
  shift /1
)

if "%HLSL_SRC_DIR%"=="" (
  echo Missing source directory.
  if exist %~dp0..\..\LLVMBuild.txt (
    set HLSL_SRC_DIR=%~dp0..\..
    echo Source directory deduced to be %~dp0..\..
  ) else (
    exit /b 1
  )
)

if "%1"=="-buildoutdir" (
  echo Build output directory set to %2
  set HLSL_BLD_DIR=%2
  shift /1
  shift /1
)

rem Build all supported architectures (x86, x64, ARM64)
call :verify_arch x86 %X86_TST% %X86_DBG% %X86_REL%
if errorlevel 1 (
  echo Failed to verify for x86.
  exit /b 1
)

call :verify_arch x64 %X64_TST% %X64_DBG% %X64_REL%
if errorlevel 1 (
  echo Failed to verify for x64.
  exit /b 1
)

rem Set path to x86 tblgen tools for the ARM64 build
if "%BUILD_TBLGEN_PATH%" == "" (
  set BUILD_TBLGEN_PATH=%HLSL_BLD_DIR%\x86\Release\bin
)

call :verify_arch arm64 %ARM64_TST% %ARM64_DBG% %ARM64_REL%
if errorlevel 1 (
  echo Failed to verify for arm64.
  exit /b 1
)

endlocal
exit /b 0

:showhelp
echo Runs the verification steps for a lab configuration.
echo.
echo Usage:
echo  hctlabverify [-short] [-fv] [-buildOutDir dir]
echo.
echo Options:
echo  -short        builds fewer components
echo  -fv           fixes version information
echo  -buildOutDir  sets the base output directory
echo.
goto :eof

:verify_arch
rem Performs a per-architecture build and test.
rem 1 - architecture
rem 2 - '1' to run tests, 0 otherwise
rem 3 - '1' to build debug, 0 to skip
rem 4 - '1' to build release, 0 to skip

setlocal

set HLSL_BLD_DIR=%HLSL_BLD_DIR%\%1
mkdir %HLSL_BLD_DIR%

rem Build the solution.
call :announce Building solution files for %1
call %HLSL_SRC_DIR%\utils\hct\hctbuild.cmd -s %FV_FLAG% %CV_FLAG% -%1
if errorlevel 1 (
  echo Failed to create solution for architecture %1
  exit /b 1
)

rem Build debug.
if "%3"=="1" (
  call :announce Debug build - %1
  call %HLSL_SRC_DIR%\utils\hct\hctbuild.cmd -b -%1
  if errorlevel 1 (
    echo Failed to build for architecture %1
    exit /b 1
  )
);

rem Build retail.
if "%4"=="1" (
  call :announce Retail build - %1
  call %HLSL_SRC_DIR%\utils\hct\hctbuild.cmd -b %ANALYZE% -rel -%1
  if errorlevel 1 (
    echo Failed to build for architecture %1 in release
    exit /b 1
  )
)

rem Run tests.
if "%2"=="1" (
  call :announce Starting tests
  rem Pick Debug if available, retail otherwise.
  if "%3"=="1" (
    call %HLSL_SRC_DIR%\utils\hct\hcttest.cmd
  ) else (
    call %HLSL_SRC_DIR%\utils\hct\hcttest.cmd -rel
  )
) else (
  echo Skipping tests.
)

endlocal
exit /b 0

:announce 
echo -------------------------------------------------------------------------
echo.
echo     %*
echo.
echo -------------------------------------------------------------------------
exit /b 0