@echo off
rem just a helper batch file for collecting up files and zipping them.
rem usage: windows-buildbot-zipper.bat <zipfilename>
rem must be run from root of SDL source tree.

IF EXIST VisualC\Win32\Release GOTO okaywin32dir
echo Please run from root of source tree after doing a Release build.
GOTO done

:okaywin32dir
IF EXIST VisualC\x64\Release GOTO okaydirs
echo Please run from root of source tree after doing a Release build.
GOTO done

:okaydirs
erase /q /f /s zipper
IF EXIST zipper GOTO zippermade
mkdir zipper
:zippermade
cd zipper
mkdir SDL
cd SDL
mkdir include
mkdir lib
mkdir lib\win32
mkdir lib\win64
copy ..\..\include\*.h include\
copy ..\..\VisualC\Win32\Release\SDL2.dll lib\win32\
copy ..\..\VisualC\Win32\Release\SDL2.lib lib\win32\
copy ..\..\VisualC\Win32\Release\SDL2main.lib lib\win32\
copy ..\..\VisualC\x64\Release\SDL2.dll lib\win64\
copy ..\..\VisualC\x64\Release\SDL2.lib lib\win64\
copy ..\..\VisualC\x64\Release\SDL2main.lib lib\win64\
cd ..
zip -9r ..\%1 SDL
cd ..
erase /q /f /s zipper

:done

