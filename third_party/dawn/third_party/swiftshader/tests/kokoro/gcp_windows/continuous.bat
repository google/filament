@echo on

SETLOCAL ENABLEDELAYEDEXPANSION

SET PATH=C:\python312;%PATH%
SET SRC=%cd%\git\SwiftShader

cd %SRC% || goto :error

REM Lower the amount of debug info, to reduce Kokoro build times.
SET LESS_DEBUG_INFO=1

cd %SRC%\build || goto :error

REM Use cmake-3.31.2 from the OS image.
REM If a newer version is required one can update the image (go/radial/kokoro_windows_image),
REM or REM uncomment the line below.
REM choco upgrade cmake -y --limit-output --no-progress
set PATH=c:\cmake-3.31.2\bin;%PATH%
cmake --version

rem To use ninja with CMake requires VC env vars
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
rem That batch file turned echo off, so turn it back on
@echo on

rem Note that we need to specify the C and C++ compiler only because Cygwin is in PATH and CMake finds GCC and picks that over MSVC
cmake .. ^
    -G "%CMAKE_GENERATOR_TYPE%" ^
    -DCMAKE_C_COMPILER="cl.exe" -DCMAKE_CXX_COMPILER="cl.exe" ^
    "-DREACTOR_BACKEND=%REACTOR_BACKEND%" ^
    "-DSWIFTSHADER_LLVM_VERSION=%LLVM_VERSION%" ^
    "-DREACTOR_VERIFY_LLVM_IR=1" ^
    "-DLESS_DEBUG_INFO=%LESS_DEBUG_INFO%" ^
    "-DSWIFTSHADER_BUILD_BENCHMARKS=1" || goto :error

cmake --build . --config %BUILD_TYPE%   || goto :error

REM Run the unit tests. Some must be run from project root
cd %SRC% || goto :error
SET SWIFTSHADER_DISABLE_DEBUGGER_WAIT_DIALOG=1

build\ReactorUnitTests.exe || goto :error
build\system-unittests.exe || goto :error
build\vk-unittests.exe || goto :error

REM Incrementally build and run rr::Print unit tests
cd %SRC%\build || goto :error
cmake "-DREACTOR_ENABLE_PRINT=1" .. || goto :error
cmake --build . --config %BUILD_TYPE% --target ReactorUnitTests || goto :error
ReactorUnitTests.exe --gtest_filter=ReactorUnitTests.Print* || goto :error
cmake "-DREACTOR_ENABLE_PRINT=0" .. || goto :error

REM Incrementally build with REACTOR_EMIT_ASM_FILE and run unit test
cd %SRC%\build || goto :error
cmake "-DREACTOR_EMIT_ASM_FILE=1" .. || goto :error
cmake --build . --config %BUILD_TYPE% --target ReactorUnitTests || goto :error
ReactorUnitTests.exe --gtest_filter=ReactorUnitTests.EmitAsm || goto :error
cmake "-DREACTOR_EMIT_ASM_FILE=0" .. || goto :error

REM Incrementally build with REACTOR_EMIT_DEBUG_INFO to ensure it builds
REM cd %SRC%\build || goto :error
REM cmake "-DREACTOR_EMIT_DEBUG_INFO=1" .. || goto :error
REM cmake --build . --config %BUILD_TYPE% --target ReactorUnitTests || goto :error
REM cmake "-DREACTOR_EMIT_DEBUG_INFO=0" .. || goto :error

REM Incrementally build with REACTOR_EMIT_PRINT_LOCATION to ensure it builds
REM cd %SRC%\build || goto :error
REM cmake "-DREACTOR_EMIT_PRINT_LOCATION=1" .. || goto :error
REM cmake --build . --config %BUILD_TYPE% --target ReactorUnitTests || goto :error
REM cmake "-DREACTOR_EMIT_PRINT_LOCATION=0" .. || goto :error

exit /b 0

:error
exit /b !ERRORLEVEL!
