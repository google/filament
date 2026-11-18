@echo off

setlocal

systeminfo

echo Disk info before building:
call :ShowDiskInfo

if "%GITHUB_WORKFLOW%" == "" (set RUNNING_LOCALLY=1)

if "%TARGET%" == "" (
    if "%1" == "" (
        set TARGET=release
    ) else (
        set TARGET=%1
    )
)

:: Target architecture: x64 (default) or arm64. Can be passed as the second argument or through
:: the ARCH environment variable.
if "%ARCH%" == "" (
    if "%2" == "" (
        set ARCH=x64
    ) else (
        set ARCH=%2
    )
)

if /i "%ARCH%" == "x64" (
    set VCVARS=vcvars64.bat
    set VS_COMPONENT=Microsoft.VisualStudio.Component.VC.Tools.x86.x64
    set CMAKE_ARCH=x64
    set DIST_ARCH=x86_64
    set ARCHIVE_NAME=filament-windows.tgz
) else if /i "%ARCH%" == "arm64" (
    set VCVARS=vcvarsarm64.bat
    set VS_COMPONENT=Microsoft.VisualStudio.Component.VC.Tools.ARM64
    set CMAKE_ARCH=ARM64
    set DIST_ARCH=arm64
    set ARCHIVE_NAME=filament-windows-arm64.tgz
) else (
    echo Unsupported ARCH: %ARCH%. Expected x64 or arm64.
    exit /b 1
)
echo Building for architecture: %ARCH%

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
    set CCACHE_DISABLE=1
)

if "%RUNNING_LOCALLY%" == "1" (
    set "PATH=%PATH%;C:\Program Files\7-Zip"
)

:: Locate Visual Studio with vswhere rather than hardcoding an edition and version: runner images
:: differ (e.g. windows-11-arm does not ship VS 2022 Enterprise at the usual path), and local
:: machines usually have the Community edition.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo vswhere.exe not found at "%VSWHERE%"
    exit /b 1
)
set VS_PATH=
set VS_VERSION=
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires %VS_COMPONENT% -property installationPath`) do set "VS_PATH=%%i"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires %VS_COMPONENT% -property installationVersion`) do set "VS_VERSION=%%i"
if "%VS_PATH%" == "" (
    echo No Visual Studio installation with component %VS_COMPONENT% was found.
    exit /b 1
)
for /f "tokens=1 delims=." %%v in ("%VS_VERSION%") do set VS_MAJOR=%%v
if "%VS_MAJOR%" == "16" (
    set "CMAKE_GENERATOR_NAME=Visual Studio 16 2019"
) else if "%VS_MAJOR%" == "17" (
    set "CMAKE_GENERATOR_NAME=Visual Studio 17 2022"
) else if "%VS_MAJOR%" == "18" (
    set "CMAKE_GENERATOR_NAME=Visual Studio 18 2026"
) else (
    echo Unsupported Visual Studio version %VS_VERSION% at "%VS_PATH%"
    exit /b 1
)
echo Using Visual Studio %VS_VERSION% at "%VS_PATH%" (CMake generator: %CMAKE_GENERATOR_NAME%)

if not exist "%VS_PATH%\VC\Auxiliary\Build\%VCVARS%" (
    echo "%VS_PATH%\VC\Auxiliary\Build\%VCVARS%" does not exist.
    exit /b 1
)
call "%VS_PATH%\VC\Auxiliary\Build\%VCVARS%"
if errorlevel 1 exit /b %errorlevel%
echo Passed %VCVARS%

msbuild /version
cmake --version

set HAS_CCACHE=
if not "%CCACHE_DISABLE%" == "1" (
    where ccache >nul 2>nul
    if not errorlevel 1 (
        ccache --version
        set "CCACHE_SLOPPINESS=file_macro,time_macros,locale,file_stat_matches"
        set HAS_CCACHE=1
    )
)

:: Important: build debug builds first, when disk space is plentiful. Debug builds require
:: significantly more temporary space.
if "%BUILD_DEBUG%" == "1" (
    :: MTd
    call :BuildVariant mtd "-DUSE_STATIC_CRT=ON" Debug || exit /b

    if "%BUILD_RELEASE_VARIANTS%" == "1" (
        :: MDd
        call :BuildVariant mdd "-DUSE_STATIC_CRT=OFF" Debug || exit /b
    )
)

if "%BUILD_RELEASE%" == "1" (
    :: /MT
    call :BuildVariant mt "-DUSE_STATIC_CRT=ON" Release || exit /b

    if "%BUILD_RELEASE_VARIANTS%" == "1" (
        :: /MD
        call :BuildVariant md "-DUSE_STATIC_CRT=OFF" Release || exit /b
    )
)

if "%CREATE_ARCHIVE%" == "1" (
    :: Use the /MT version as the "base" install.
    move out\mt out\install
    mkdir out\install\lib\%DIST_ARCH%\mt\
    move out\install\lib\%DIST_ARCH%\*.lib out\install\lib\%DIST_ARCH%\mt\

    xcopy out\md\lib\%DIST_ARCH%\*.lib out\install\lib\%DIST_ARCH%\md\
    xcopy out\mtd\lib\%DIST_ARCH%\*.lib out\install\lib\%DIST_ARCH%\mtd\
    xcopy out\mdd\lib\%DIST_ARCH%\*.lib out\install\lib\%DIST_ARCH%\mdd\
)

:: Create an archive.
if "%CREATE_ARCHIVE%" == "1" (
    cd out\install
    7z a -ttar -so ..\..\filament-release.tar * | 7z a -si ..\%ARCHIVE_NAME%
)

exit /b 0

:BuildVariant
set variant=%~1
set flag=%~2
set config=%~3

echo Disk info before building variant: %variant%
call :ShowDiskInfo

mkdir out\cmake-%variant%
cd out\cmake-%variant%
if errorlevel 1 exit /b %errorlevel%

cmake ..\.. ^
    -G "%CMAKE_GENERATOR_NAME%" ^
    -A %CMAKE_ARCH% ^
    -DDIST_ARCH=%DIST_ARCH% ^
    %flag% ^
    -DCMAKE_INSTALL_PREFIX=..\%variant% ^
    -DFILAMENT_WINDOWS_CI_BUILD:BOOL=ON ^
    -DFILAMENT_SUPPORTS_VULKAN=ON ^
    "-DCMAKE_VS_GLOBALS=UseMultiToolTask=true;EnforceProcessCountAcrossBuilds=true" ^
    || exit /b

set build_flags=-j %NUMBER_OF_PROCESSORS%

@echo on

:: we've upgraded the windows machines, so the following are no longer accurate as of 09/19/24, but
:: keeping around the comment for record.

:: Attempt to fix "error C1060: compiler is out of heap space" seen on CI.
:: Some resource libraries require significant heap space to compile, so first compile them serially.
:: cmake --build . --target filagui --config %config% %build_flags% || exit /b
:: cmake --build . --target uberarchive --config %config% %build_flags% || exit /b
:: cmake --build . --target gltf-demo-resources --config %config% %build_flags% || exit /b
:: cmake --build . --target filamentapp-resources --config %config% %build_flags% || exit /b
:: cmake --build . --target sample-resources --config %config% %build_flags% || exit /b
:: cmake --build . --target suzanne-resources --config %config% %build_flags% || exit /b

cmake --build . %INSTALL% --config %config% %build_flags% -- /m || exit /b
@echo off

if "%HAS_CCACHE%" == "1" (
    ccache --show-stats
)

echo Disk info after building variant: %variant%
call :ShowDiskInfo

cd ..\..

:: Delete the cmake build folder, otherwise we run out of disk space on CI when
:: building multiple variants.
rd /s /q out\cmake-%variant%
exit /b 0

:: Helps debugging GitHub builds that run out of space
:ShowDiskInfo
echo =======================================================
:: wmic is not installed by default on newer Windows 11 images (e.g. the windows-11-arm runner).
where wmic >nul 2>nul
if errorlevel 1 (
    powershell -NoProfile -Command "Get-PSDrive -PSProvider FileSystem | Format-Table Name,Used,Free"
) else (
    wmic logicaldisk get size,freespace,caption
)
echo =======================================================
exit /b 0
