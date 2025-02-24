@echo off

setlocal

if "%1"=="/?" goto :showhelp
if "%1"=="-?" goto :showhelp
if "%1"=="-help" goto :showhelp
if "%1"=="--help" goto :showhelp

if "%HLSL_BLD_DIR%"=="" (
  echo Missing build directory - consider running hctstart.
  exit /b 1
)

if exist "%HLSL_BLD_DIR%" (
  echo Deleting %HLSL_BLD_DIR% ...
  rmdir /q /s %HLSL_BLD_DIR%
  if errorlevel 1 (
    echo Unable to remove %HLSL_BLD_DIR%.
    exit /b 1
  )
)

goto :eof

:showhelp
echo Cleans generated files.
echo.
echo hctclean
goto :eof

