
rem -----------------------------------------------------
rem Batch file to build zipped redist packages
rem Two different packages are built:
rem
rem assimp--<revision>-bin.zip
rem    Binaries for x86 and x64
rem    Command line reference
rem
rem assimp--<revision>-sdk.zip
rem    Binaries for x86 and x64, Debug & Release
rem    Libs for DLL build, x86 & 64, Debug & Release
rem    Full SVN checkout exluding mkutil & port        
rem
rem
rem PREREQUISITES:
rem   -7za.exe (7zip standalone) 
rem    Download from http://www.7-zip.org/download.html
rem
rem   -svnversion.exe (Subversion revision getter)
rem    Download any command line SVN package
rem
rem   -doxygen.exe (Doxygen client)
rem    Download from www.doxygen.com
rem
rem   -svn client
rem
rem NOTES:
rem   ./bin must not have any local modifications
rem
rem -----------------------------------------------------

@echo off
color 4e
cls

rem -----------------------------------------------------
rem  Setup file revision for build
rem -----------------------------------------------------
call mkrev.bat

rem -----------------------------------------------------
rem Build output file names
rem -----------------------------------------------------

cd ..\..\bin
svnversion > tmpfile.txt
SET /p REVISIONBASE= < tmpfile.txt
DEL /q tmpfile.txt
cd ..\packaging\windows-mkzip

SET VERSIONBASE=2.0.%REVISIONBASE%

SET OUT_SDK=assimp--%VERSIONBASE%-sdk
SET OUT_BIN=assimp--%VERSIONBASE%-bin

SET BINCFG_x86=release-dll_win32
SET BINCFG_x64=release-dll_x64

SET BINCFG_x86_DEBUG=debug-dll_win32
SET BINCFG_x64_DEBUG=debug-dll_x64

rem -----------------------------------------------------
rem Delete previous output directories
rem -----------------------------------------------------
RD /S /q final\

rem -----------------------------------------------------
rem Create output directories
rem -----------------------------------------------------

mkdir final\%OUT_BIN%\x86
mkdir final\%OUT_BIN%\x64

rem -----------------------------------------------------
rem Copy all executables to 'final-bin'
rem -----------------------------------------------------

copy /Y ..\..\bin\assimpview_%BINCFG_x86%\assimp_view.exe "final\%OUT_BIN%\x86\assimp_view.exe"
copy /Y ..\..\bin\assimpview_%BINCFG_x64%\assimp_view.exe "final\%OUT_BIN%\x64\assimp_view.exe"

copy /Y ..\..\bin\assimpcmd_%BINCFG_x86%\assimp.exe "final\%OUT_BIN%\x86\assimp.exe"
copy /Y ..\..\bin\assimpcmd_%BINCFG_x64%\assimp.exe "final\%OUT_BIN%\x64\assimp.exe"

copy /Y ..\..\bin\assimp_%BINCFG_x86%\Assimp32.dll    "final\%OUT_BIN%\x86\Assimp32.dll"
copy /Y ..\..\bin\assimp_%BINCFG_x64%\Assimp64.dll    "final\%OUT_BIN%\x64\Assimp64.dll"

copy ..\..\LICENSE final\%OUT_BIN%\LICENSE
copy ..\..\CREDITS final\%OUT_BIN%\CREDITS
copy bin_readme.txt final\%OUT_BIN%\README
copy bin_readme.txt final\%OUT_BIN%\README

copy ..\..\doc\AssimpCmdDoc_Html\AssimpCmdDoc.chm  final\%OUT_BIN%\CommandLine.chm

rem -----------------------------------------------------
rem Do a clean export of the repository and build SDK
rem
rem We take the current revision and remove some stuff
rem that is nto yet ready to be published.
rem -----------------------------------------------------

svn export .\..\..\  .\final\%OUT_SDK%

mkdir final\%OUT_SDK%\doc\assimp_html
mkdir final\%OUT_SDK%\doc\assimpcmd_html
copy .\..\..\doc\AssimpDoc_Html\* final\%OUT_SDK%\doc\assimp_html
copy .\..\..\doc\AssimpCmdDoc_Html\* final\%OUT_SDK%\doc\assimpcmd_html
del final\%OUT_SDK%\doc\assimpcmd_html\AssimpCmdDoc.chm
del final\%OUT_SDK%\doc\assimp_html\AssimpDoc.chm

rem Copy doc to a suitable place
move final\%OUT_SDK%\doc\AssimpDoc_Html\AssimpDoc.chm final\%OUT_SDK%\Documentation.chm
move final\%OUT_SDK%\doc\AssimpCmdDoc_Html\AssimpCmdDoc.chm final\%OUT_SDK%\CommandLine.chm

rem Cleanup ./doc folder
del /q final\%OUT_SDK%\doc\Preamble.txt 
RD  /s /q final\%OUT_SDK%\doc\AssimpDoc_Html
RD  /s /q final\%OUT_SDK%\doc\AssimpCmdDoc_Html

rem Insert 'dummy' files into empty folders
echo. > final\%OUT_SDK%\lib\dummy
echo. > final\%OUT_SDK%\obj\dummy


RD  /s /q final\%OUT_SDK%\port\swig

rem Also, repackaging is not a must-have feature
RD  /s /q final\%OUT_SDK%\packaging

rem Copy prebuilt libs
mkdir "final\%OUT_SDK%\lib\assimp_%BINCFG_x86%"
mkdir "final\%OUT_SDK%\lib\assimp_%BINCFG_x64%"
mkdir "final\%OUT_SDK%\lib\assimp_%BINCFG_x86_DEBUG%"
mkdir "final\%OUT_SDK%\lib\assimp_%BINCFG_x64_DEBUG%"

copy /Y ..\..\lib\assimp_%BINCFG_x86%\assimp.lib    "final\%OUT_SDK%\lib\assimp_%BINCFG_x86%"
copy /Y ..\..\lib\assimp_%BINCFG_x64%\assimp.lib    "final\%OUT_SDK%\lib\assimp_%BINCFG_x64%\"
copy /Y ..\..\lib\assimp_%BINCFG_x86_DEBUG%\assimp.lib    "final\%OUT_SDK%\lib\assimp_%BINCFG_x86_DEBUG%\"
copy /Y ..\..\lib\assimp_%BINCFG_x64_DEBUG%\assimp.lib    "final\%OUT_SDK%\lib\assimp_%BINCFG_x64_DEBUG%\"

rem Copy prebuilt DLLs
mkdir "final\%OUT_SDK%\bin\assimp_%BINCFG_x86%"
mkdir "final\%OUT_SDK%\bin\assimp_%BINCFG_x64%"
mkdir "final\%OUT_SDK%\bin\assimp_%BINCFG_x86_DEBUG%"
mkdir "final\%OUT_SDK%\bin\assimp_%BINCFG_x64_DEBUG%"


copy /Y ..\..\bin\assimp_%BINCFG_x86%\Assimp32.dll    "final\%OUT_SDK%\bin\assimp_%BINCFG_x86%\"
copy /Y ..\..\bin\assimp_%BINCFG_x64%\Assimp64.dll    "final\%OUT_SDK%\bin\assimp_%BINCFG_x64%\"
copy /Y ..\..\bin\assimp_%BINCFG_x86_DEBUG%\Assimp32d.dll    "final\%OUT_SDK%\bin\assimp_%BINCFG_x86_DEBUG%\"
copy /Y ..\..\bin\assimp_%BINCFG_x64_DEBUG%\Assimp64d.dll    "final\%OUT_SDK%\bin\assimp_%BINCFG_x64_DEBUG%\"


rem -----------------------------------------------------
rem Make final-bin.zip and final-sdk.zip
rem -----------------------------------------------------

IF NOT EXIST 7za.exe	(
	cls
	echo You need to have 7zip standalone installed to
	echo build ZIP archives. Download: http://www.7-zip.org/download.html
	pause
) else (
7za.exe a -tzip "final\%OUT_BIN%.zip" ".\final\%OUT_BIN%"
7za.exe a -tzip "final\%OUT_SDK%.zip" ".\final\%OUT_SDK%"
)

rem OK. We should have the release packages now.

