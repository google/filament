@echo on

:: Install build dependencies
if defined KOKORO_BUILD_ID (
    choco install cmake.install --installargs '"ADD_CMAKE_TO_PATH=User"' -y
    choco install llvm --version 7.0.1 -y

    :: Refresh PATH after installing packages
    :: Do ***NOT*** use refreshenv, it exits the current script
    call RefreshEnv.cmd
)

echo "%PATH%"

:: Put Visual Studio tools on the PATH
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
if errorlevel 1 exit /b %errorlevel%

:: Ensure that the necessary build tools are on the PATH
call %~dp0validate-build-dependencies.bat
if errorlevel 1 exit /b %errorlevel%

ninja --version
clang-cl --version
lld-link --version
cmake --version

cd github\filament

:: If this is a release build, build all Filament static library variants (/MD, /MDd, /MT, /MTd)
if "%KOKORO_JOB_TYPE%" == "RELEASE" (set FILAMENT_BUILD_ALL_VARIANTS=1)

if "%FILAMENT_BUILD_ALL_VARIANTS%" == "1" (
    echo KOKORO_JOB_TYPE is %KOKORO_JOB_TYPE%
    echo Building additional Filament static library variants.
)

mkdir out\cmake-release
cd out\cmake-release
if errorlevel 1 exit /b %errorlevel%

:: Force Java JDK 8. Kokoro machines default to 9
SET JAVA_HOME=C:\Program Files\Java\jdk1.8.0_152

cmake ..\.. -G Ninja ^
    -DCMAKE_CXX_COMPILER:PATH="clang-cl.exe" ^
    -DCMAKE_C_COMPILER:PATH="clang-cl.exe" ^
    -DCMAKE_LINKER:PATH="lld-link.exe" ^
    -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b %errorlevel%

ninja
if errorlevel 1 exit /b %errorlevel%

ninja install
if errorlevel 1 exit /b %errorlevel%

if "%FILAMENT_BUILD_ALL_VARIANTS%" == "1" (
    cd ..
    :: Build variants and copy them inside cmake-release\install
    call %~dp0variants.bat
    if errorlevel 1 exit /b %errorlevel%
    cd cmake-release
)

:: Create an archive
dir .\install
7z a -ttar -so ..\filament-release.tar .\install\* | 7z a -si ..\filament-windows.tgz

echo Done!
