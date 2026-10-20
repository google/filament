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

set BUILD_DEBUG=
set BUILD_RELEASE=
set INSTALL=
set BUILD_RELEASE_VARIANTS=
set GENERATE_PDB=OFF

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
    set GENERATE_PDB=ON
)

set VISUAL_STUDIO_VERSION="Enterprise"
if "%RUNNING_LOCALLY%" == "1" (
    set VISUAL_STUDIO_VERSION="Community"
    set "PATH=%PATH%;C:\Program Files\7-Zip"
)

:: Outdated windows-2019 pattern
:: call "C:\Program Files (x86)\Microsoft Visual Studio\2019\%VISUAL_STUDIO_VERSION%\VC\Auxiliary\Build\vcvars64.bat"

call "C:\Program Files\Microsoft Visual Studio\2022\%VISUAL_STUDIO_VERSION%\VC\Auxiliary\Build\vcvars64.bat"
echo Passed vcvars64.bat
if errorlevel 1 exit /b %errorlevel%

ninja --version
cmake --version

set "OUT_DIR=%CD%\out"
if not "%RUNNING_LOCALLY%" == "1" (
    echo Adding Windows Defender exclusion for %OUT_DIR%
    powershell -NoProfile -NonInteractive -Command "Add-MpPreference -ExclusionPath '%OUT_DIR%'"
)

set PREBUILT_TOOLS_FLAG=

:: Build release first so that its host tools (matc, cmgen, resgen, etc.) can be exported and
:: reused by subsequent release and debug variants instead of running slow debug builds of tools.
if "%BUILD_RELEASE%" == "1" (
    :: /MT
    call :BuildVariant mt "-DUSE_STATIC_CRT=ON -DFILAMENT_BUILD_TESTING=OFF -DFILAMENT_EXPORT_PREBUILT_EXECUTABLES_DIR=out/prebuilt-tools" Release || goto :ErrorExit
    set PREBUILT_TOOLS_FLAG=-DFILAMENT_IMPORT_PREBUILT_EXECUTABLES_DIR=out/prebuilt-tools -DFILAMENT_SKIP_TOOLS=ON

    if "%BUILD_RELEASE_VARIANTS%" == "1" (
        :: /MD
        call :BuildVariant md "-DUSE_STATIC_CRT=OFF -DFILAMENT_BUILD_TESTING=OFF -DFILAMENT_SKIP_SAMPLES=ON -DFILAMENT_IMPORT_PREBUILT_EXECUTABLES_DIR=out/prebuilt-tools -DFILAMENT_SKIP_TOOLS=ON" Release || goto :ErrorExit
    )
)

if "%BUILD_DEBUG%" == "1" (
    :: MTd
    call :BuildVariant mtd "-DUSE_STATIC_CRT=ON -DFILAMENT_BUILD_TESTING=ON -DFILAMENT_SKIP_SAMPLES=ON %PREBUILT_TOOLS_FLAG%" Debug || goto :ErrorExit

    if "%BUILD_RELEASE_VARIANTS%" == "1" (
        :: MDd
        call :BuildVariant mdd "-DUSE_STATIC_CRT=OFF -DFILAMENT_BUILD_TESTING=ON -DFILAMENT_SKIP_SAMPLES=ON %PREBUILT_TOOLS_FLAG%" Debug || goto :ErrorExit
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

set EXIT_CODE=0
goto :Exit

:ErrorExit
set EXIT_CODE=%ERRORLEVEL%
if "%EXIT_CODE%" == "0" set EXIT_CODE=1

:Exit
if not "%RUNNING_LOCALLY%" == "1" (
    echo Removing Windows Defender exclusion for %OUT_DIR%
    powershell -NoProfile -NonInteractive -Command "Remove-MpPreference -ExclusionPath '%OUT_DIR%'"
)
exit /b %EXIT_CODE%

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
    -G Ninja ^
    -DCMAKE_BUILD_TYPE=%config% ^
    %flag% ^
    -DCMAKE_INSTALL_PREFIX=..\%variant% ^
    -DFILAMENT_WINDOWS_CI_BUILD:BOOL=ON ^
    -DFILAMENT_GENERATE_PDB:BOOL=%GENERATE_PDB% ^
    -DFILAMENT_ENABLE_MATDBG:BOOL=OFF ^
    -DFILAMENT_SUPPORTS_VULKAN=ON ^
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

cmake --build . %INSTALL% %build_flags% || exit /b
@echo off

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
wmic logicaldisk get size,freespace,caption
echo =======================================================
exit /b 0
