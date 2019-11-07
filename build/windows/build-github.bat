@echo off

setlocal

if "%GITHUB_WORKFLOW%" == "" (set RUNNING_LOCALLY=1)

if "%TARGET%" == "" (
    if "%1" == "" (
        set TARGET=release
    ) else (
        set TARGET=%1
    )
)

set BUILD_DEBUG=
set BUILD_RELEASE=
set INSTALL=
set BUILD_RELEASE_VARIANTS=

if "%TARGET%" == "presubmit" (
    set BUILD_RELEASE=1
)

if "%TARGET%" == "continuous" (
    set BUILD_DEBUG=1
    set BUILD_RELEASE=1
)

if "%TARGET%" == "release" (
    set BUILD_DEBUG=1
    set BUILD_RELEASE=1
    set INSTALL=--target install
    set BUILD_RELEASE_VARIANTS=1
)

set VISUAL_STUDIO_VERSION="Enterprise"
if "%RUNNING_LOCALLY%" == "1" (
    set VISUAL_STUDIO_VERSION="Professional"
    set "PATH=%PATH%;C:\Program Files\7-Zip"
)

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\%VISUAL_STUDIO_VERSION%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b %errorlevel%

msbuild /version
cmake --version

mkdir out\cmake
cd out\cmake
if errorlevel 1 exit /b %errorlevel%

cmake ..\.. -G "Visual Studio 16 2019" -A x64 || exit /b

if "%BUILD_RELEASE%" == "1" (
    :: /MT
    cmake . -DUSE_STATIC_CRT=ON -DCMAKE_INSTALL_PREFIX=..\mt
    cmake --build . %INSTALL% --config Release -- /m || exit /b

    if "%BUILD_RELEASE_VARIANTS%" == "1" (
        :: /MD
        cmake . -DUSE_STATIC_CRT=OFF -DCMAKE_INSTALL_PREFIX=..\md
        cmake --build . %INSTALL% --config Release -- /m || exit /b
    )
)

if "%BUILD_DEBUG%" == "1" (
    :: MTd
    cmake . -DUSE_STATIC_CRT=ON -DCMAKE_INSTALL_PREFIX=..\mtd
    cmake --build . %INSTALL% --config Debug -- /m || exit /b

    if "%BUILD_RELEASE_VARIANTS%" == "1" (
        :: MDd
        cmake . -DUSE_STATIC_CRT=OFF -DCMAKE_INSTALL_PREFIX=..\mdd
        cmake --build . %INSTALL% --config Debug -- /m || exit /b
    )
)

if "%BUILD_RELEASE_VARIANTS%" == "1" (
    :: Use the /MT version as the "base" install.
    move ..\mt ..\install
    mkdir ..\install\lib\x86_64\mt\
    move ..\install\lib\x86_64\*.lib ..\install\lib\x86_64\mt\

    xcopy ..\md\lib\x86_64\*.lib ..\install\lib\x86_64\md\
    xcopy ..\mtd\lib\x86_64\*.lib ..\install\lib\x86_64\mtd\
    xcopy ..\mdd\lib\x86_64\*.lib ..\install\lib\x86_64\mdd\
)

:: Create an archive.
if "%BUILD_RELEASE_VARIANTS%" == "1" (
    7z a -ttar -so ..\filament-release.tar ..\install\* | 7z a -si ..\filament-windows.tgz
)
