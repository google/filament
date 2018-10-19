@echo off

:: Install build dependencies
call %~dp0ci-common.bat
if errorlevel 1 exit /b %errorlevel%

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

:: Create an archive
dir .\install
7z a -ttar -so ..\filament-release.tar .\install\* | 7z a -si ..\filament-windows.tgz

echo Done!
