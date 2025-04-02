@echo off
@setlocal enableextensions

if "%~1" == "/?" goto :help
if "%~1" == "-?" goto :help
if "%~1" == "/help" goto :help
if "%~1" == "-help" goto :help
if "%~1" == "--help" goto :help
goto :run

:help
echo Usage:
echo   %~nx0 [SOLUTION_CONFIG]
echo Examples:
echo   %~nx0 "Release|Win32" (default)
echo   %~nx0 "Debug|Win32"
echo   %~nx0 "Release|ARM64"
echo   %~nx0 "Debug|ARM64"
echo   etc.
exit /b 2

:run
set _SOLUTION_CONFIG="%~1"
if %_SOLUTION_CONFIG% == "" set _SOLUTION_CONFIG="Release|Win32"
devenv "%~dp0.\vstudio.sln" /build %_SOLUTION_CONFIG%
