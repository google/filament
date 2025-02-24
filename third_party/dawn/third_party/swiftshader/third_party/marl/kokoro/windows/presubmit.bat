REM Copyright 2020 The Marl Authors.
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM     https://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.

@echo on

SETLOCAL ENABLEDELAYEDEXPANSION

SET BUILD_ROOT=%cd%
SET PATH=C:\python36;C:\Program Files\cmake\bin;%PATH%
SET ROOT_DIR=%cd%\github\marl
SET BUILD_DIR=%ROOT_DIR%\build

cd %ROOT_DIR%
if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!

git submodule update --init
if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!

SET CONFIG=Release

mkdir %BUILD_DIR%
cd %BUILD_DIR%
if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!

IF /I "%BUILD_SYSTEM%"=="cmake" (
    cmake "%ROOT_DIR%" ^
        -G "%BUILD_GENERATOR%" ^
        -A "%BUILD_TARGET_ARCH%" ^
        "-DMARL_BUILD_TESTS=1" ^
        "-DMARL_BUILD_EXAMPLES=1" ^
        "-DMARL_BUILD_BENCHMARKS=1" ^
        "-DMARL_WARNINGS_AS_ERRORS=1" ^
        "-DMARL_DEBUG_ENABLED=1"
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
    cmake --build . --config %CONFIG%
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
    %CONFIG%\marl-unittests.exe
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
    %CONFIG%\fractal.exe
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
    %CONFIG%\primes.exe > nul
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
) ELSE IF /I "%BUILD_SYSTEM%"=="bazel" (
    REM Fix up the MSYS environment.
    wget -q http://repo.msys2.org/mingw/x86_64/mingw-w64-x86_64-gcc-7.3.0-2-any.pkg.tar.xz
    wget -q http://repo.msys2.org/mingw/x86_64/mingw-w64-x86_64-gcc-libs-7.3.0-2-any.pkg.tar.xz
    c:\tools\msys64\usr\bin\bash --login -c "pacman -R --noconfirm catgets libcatgets"
    c:\tools\msys64\usr\bin\bash --login -c "pacman -Syu --noconfirm"
    c:\tools\msys64\usr\bin\bash --login -c "pacman -Sy --noconfirm mingw-w64-x86_64-crt-git patch"
    c:\tools\msys64\usr\bin\bash --login -c "pacman -U --noconfirm mingw-w64-x86_64-gcc*-7.3.0-2-any.pkg.tar.xz"
    set PATH=C:\tools\msys64\mingw64\bin;c:\tools\msys64\usr\bin;!PATH!
    set BAZEL_SH=C:\tools\msys64\usr\bin\bash.exe

    REM Install Bazel
    SET BAZEL_DIR=!BUILD_ROOT!\bazel
    mkdir !BAZEL_DIR!
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
    wget -q https://github.com/bazelbuild/bazel/releases/download/0.29.1/bazel-0.29.1-windows-x86_64.zip
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
    unzip -q bazel-0.29.1-windows-x86_64.zip -d !BAZEL_DIR!
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!

    REM Build and run
    !BAZEL_DIR!\bazel test //:tests --test_output=all
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
    !BAZEL_DIR!\bazel run //examples:fractal
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
    !BAZEL_DIR!\bazel run //examples:primes > nul
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
) ELSE (
    echo "Unknown build system: %BUILD_SYSTEM%"
    exit /b 1
)
