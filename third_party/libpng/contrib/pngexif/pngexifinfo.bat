@echo off
@setlocal enableextensions

python.exe -BES %~dp0.\pngexifinfo.py %*
