@echo off
setlocal ENABLEDELAYEDEXPANSION

if "%1"=="" goto :showhelp
if "%1"=="/?" goto :showhelp
if "%1"=="-?" goto :showhelp
if "%1"=="/h" goto :showhelp
if "%1"=="-h" goto :showhelp
if "%1"=="-help" goto :showhelp
if "%1"=="--help" goto :showhelp

if "%BUILD_CONFIG%"=="" (
  set BUILD_CONFIG=Debug
)

set TEST_DIR=%HLSL_BLD_DIR%\%BUILD_CONFIG%\test
set DEPLOY_DIR=%1

if not exist %HLSL_SRC_DIR%\external\taef\. (
  call hctgettaef.py
  if errorlevel 1 (
    echo hctgettaef.py failed with errorlevel !errorlevel!
    exit /b 1
  )
)

rem Deploy test content to test directory
call hcttest.cmd none
if errorlevel 1 (
  echo test deployment with 'hcttest none' failed with errorlevel !errorlevel!
  exit /b 1
)

robocopy /S %HLSL_SRC_DIR%\external\taef\build\Binaries %DEPLOY_DIR%\taef *
robocopy /S %TEST_DIR% %DEPLOY_DIR%\test *
robocopy /S %HLSL_SRC_DIR%\tools\clang\test\HLSL %DEPLOY_DIR%\HLSL *
robocopy /S %HLSL_SRC_DIR%\tools\clang\test\CodeGenHLSL %DEPLOY_DIR%\CodeGenHLSL *

echo =========================================================================
echo Provided there were no errors above, the test can now be run from
echo the target directory with:
echo.
echo taef\amd64\te test\ClangHLSLTests.dll /p:"HlslDataDir=HLSL" [options]
echo.
echo You may need to deploy VS runtime libraries in order to run the unit tests.
echo Debug versions of these will be required for the Debug build configuration.
echo Here are some dll's that may be required:
echo   msvcp140.dll
echo   msvcp140d.dll
echo   ucrtbased.dll
echo   vcruntime140.dll
echo   vcruntime140d.dll
echo.
exit /b 0

:showhelp
echo Usage:
echo  hctdeploytest target-directory
echo.
goto :eof
