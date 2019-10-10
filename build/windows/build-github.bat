@echo off

if "%GITHUB_WORKFLOW%" == "" (set RUNNING_LOCALLY=1)

set VISUAL_STUDIO_VERSION="Enterprise"
if "%RUNNING_LOCALLY%" == "1" (set VISUAL_STUDIO_VERSION="Professional")

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\%VISUAL_STUDIO_VERSION%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b %errorlevel%
