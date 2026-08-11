@echo off
setlocal EnableExtensions EnableDelayedExpansion

for %%I in ("%~dp0..") do set "ROOT=%%~fI"
set "ROOT_CMAKE=%ROOT:\=/%"
set "DESKTOP_BUILD=%ROOT%\out\cmake-xr"
set "ANDROID_BUILD=%ROOT%\out\cmake-android-release-aarch64"
set "FILAMENT_INSTALL=%ROOT%\out\android-release\filament"
set "GRADLE_PROJECT=%ROOT%\android\samples\openxr"
set "APK=%GRADLE_PROJECT%\build\outputs\apk\debug\filament-openxr-debug.apk"
set "PACKAGE=com.google.android.filament.openxr"
set "ACTIVITY=android.app.NativeActivity"

pushd "%ROOT%" || exit /b 1

where cmake >nul 2>nul || goto :missing_cmake
where adb >nul 2>nul || goto :missing_adb
if not exist "%ROOT%\out\ImportExecutables-Prebuilt.cmake" goto :missing_host_tools
if not exist "%ROOT%\out\cmake-xr\tools\matc\matc.exe" goto :missing_host_tools
if not exist "%ROOT%\out\cmake-xr\samples\generated\resources\suzanne.filamesh" goto :missing_assets
if not exist "%ROOT%\out\cmake-xr\samples\assets\ibl\lightroom_14b" goto :missing_assets

if not defined VSCMD_VER (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        for /f "usebackq tokens=*" %%I in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS_INSTALL=%%I"
    )
    if not defined VS_INSTALL if exist "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat" set "VS_INSTALL=C:\Program Files\Microsoft Visual Studio\18\Enterprise"
    if not defined VS_INSTALL goto :missing_vs
    echo.
    echo === Initializing Visual Studio x64 environment ===
    call "!VS_INSTALL!\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul
    if errorlevel 1 goto :failed
)

echo.
echo === Rebuilding host matc ===
cmake --build "%DESKTOP_BUILD%" --target matc
if errorlevel 1 goto :failed

echo.
echo === Configuring Filament for Android arm64 ===
cmake -S "%ROOT%" -B "%ANDROID_BUILD%" -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    "-DCMAKE_TOOLCHAIN_FILE=%ROOT_CMAKE%/build/toolchain-aarch64-linux-android.cmake" ^
    "-DCMAKE_INSTALL_PREFIX=%ROOT_CMAKE%/out/android-release/filament" ^
    -DFILAMENT_SUPPORTS_VULKAN=ON ^
    -DFILAMENT_SAMPLES_STEREO_TYPE=multiview ^
    -DFILAMENT_SKIP_SAMPLES=ON ^
    -DFILAMENT_IMPORT_PREBUILT_EXECUTABLES_DIR=out
if errorlevel 1 goto :failed

echo.
echo === Building and installing Filament arm64 ===
cmake --build "%ANDROID_BUILD%" --target install
if errorlevel 1 goto :failed

echo.
echo === Assembling helloxr debug APK ===
call "%ROOT%\android\gradlew.bat" -p "%GRADLE_PROJECT%" assembleDebug
if errorlevel 1 goto :failed

if not exist "%APK%" (
    echo ERROR: APK was not produced at "%APK%".
    goto :failed
)

echo.
echo === Installing APK ===
adb install -r "%APK%"
if errorlevel 1 goto :failed

echo.
echo === Launching helloxr ===
adb shell am start -a android.intent.action.MAIN ^
    -c com.oculus.intent.category.VR -n "%PACKAGE%/%ACTIVITY%"
if errorlevel 1 goto :failed

echo.
echo Deployed: %APK%
popd
exit /b 0

:missing_cmake
echo ERROR: cmake is not on PATH.
goto :failed

:missing_adb
echo ERROR: adb is not on PATH or no Android SDK platform-tools are configured.
goto :failed

:missing_host_tools
echo ERROR: Prebuilt host tools were not found under "%ROOT%\out".
echo Build the desktop tools once before using this Android-only deploy script.
goto :failed

:missing_assets
echo ERROR: Prebuilt sample assets were not found under "%ROOT%\out\cmake-xr\samples".
goto :failed

:missing_vs
echo ERROR: A Visual Studio installation with the C++ x64 tools was not found.
goto :failed

:failed
echo.
echo Build or deployment failed.
popd
exit /b 1
