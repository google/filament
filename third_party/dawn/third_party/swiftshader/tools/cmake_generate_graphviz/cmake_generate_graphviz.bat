@echo on
setlocal enabledelayedexpansion
pushd %~dp0

where /q cmake.exe
if %errorlevel% neq 0 (
    echo "CMake not found. Please install it from https://cmake.org/"
    exit /b 1
)

where /q dot.exe
if %errorlevel% neq 0 (
    echo "GraphViz (dot.exe) not found. Please install it from https://graphviz.gitlab.io/"
    exit /b 1
)

set cmake_binary_dir=%1

if "%cmake_binary_dir%" == "" (
    set cmake_binary_dir=..\..\build
)

rem Copy options to binary dir
copy /y CMakeGraphVizOptions.cmake "%cmake_binary_dir%\"
if %errorlevel% neq 0 exit /b %errorlevel%

rem Run cmake commands from the binary dir
pushd %cmake_binary_dir%

cmake --graphviz=SwiftShader.dot ..
if %errorlevel% neq 0 exit /b %errorlevel%

dot -Tpng -o SwiftShader.png SwiftShader.dot
if %errorlevel% neq 0 exit /b %errorlevel%

rem Open the file
start SwiftShader.png

popd
popd
