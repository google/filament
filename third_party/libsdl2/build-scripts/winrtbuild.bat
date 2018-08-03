@ECHO OFF
REM
REM winrtbuild.bat: a batch file to help launch the winrtbuild.ps1
REM   Powershell script, either from Windows Explorer, or through Buildbot.
REM
SET ThisScriptsDirectory=%~dp0
SET PowerShellScriptPath=%ThisScriptsDirectory%winrtbuild.ps1
PowerShell -NoProfile -ExecutionPolicy Bypass -Command "& '%PowerShellScriptPath%'";