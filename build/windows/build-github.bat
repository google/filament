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
    set INSTALL=--target install
    set CREATE_ARCHIVE=1
)

if "%TARGET%" == "release" (
    set BUILD_DEBUG=1
    set BUILD_RELEASE=1
    set INSTALL=--target install
    set BUILD_RELEASE_VARIANTS=1
    set CREATE_ARCHIVE=1
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

if "%BUILD_RELEASE%" == "1" (
    :: /MT
    call :BuildVariant mt "-DUSE_STATIC_CRT=ON" Release || exit /b

    if "%BUILD_RELEASE_VARIANTS%" == "1" (
        :: /MD
        call :BuildVariant md "-DUSE_STATIC_CRT=OFF" Release || exit /b
    )
)

if "%BUILD_DEBUG%" == "1" (
    :: MTd
    call :BuildVariant mtd "-DUSE_STATIC_CRT=ON" Debug || exit /b

    if "%BUILD_RELEASE_VARIANTS%" == "1" (
        :: MDd
        call :BuildVariant mdd "-DUSE_STATIC_CRT=OFF" Debug || exit /b
    )
)

if "%CREATE_ARCHIVE%" == "1" (
    :: Use the /MT version as the "base" install.
    move out\mt out\install
    mkdir out\install\lib\x86_64\mt\
    move out\install\lib\x86_64\*.lib out\install\lib\x86_64\mt\

    xcopy out\md\lib\x86_64\*.lib out\install\lib\x86_64\md\
    xcopy out\mtd\lib\x86_64\*.lib out\install\lib\x86_64\mtd\
    xcopy out\mdd\lib\x86_64\*.lib out\install\lib\x86_64\mdd\
)

:: Create an archive.
if "%CREATE_ARCHIVE%" == "1" (
    cd out\install
    7z a -ttar -so ..\..\filament-release.tar * | 7z a -si ..\filament-windows.tgz
)

exit /b 0

:BuildVariant
set variant=%~1
set flag=%~2
set config=%~3

mkdir out\cmake-%variant%
cd out\cmake-%variant%
if errorlevel 1 exit /b %errorlevel%

cmake ..\.. -G "Visual Studio 16 2019" -A x64 %flag% -DCMAKE_INSTALL_PREFIX=..\%variant% || exit /b
cmake --build . %INSTALL% --config %config% -- /m || exit /b

cd ..\..

:: Delete the cmake build folder, otherwise we run out of disk space on CI when
:: building multiple variants.
rd /s /q out\cmake-%variant%
exit /b 0
