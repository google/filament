@echo off
setlocal
:: This is a test git.bat with a relative path to git.exe.
"%~dp0Relative\Path\To\Git\cmd\git.exe" %*
