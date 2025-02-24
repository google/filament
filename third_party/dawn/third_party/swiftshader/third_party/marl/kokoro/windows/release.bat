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

cmake "%ROOT_DIR%" ^
    -G "%BUILD_GENERATOR%" ^
    -A "%BUILD_TARGET_ARCH%" ^
    "-DMARL_BUILD_TESTS=1" ^
    "-DMARL_BUILD_EXAMPLES=1" ^
    "-DMARL_BUILD_BENCHMARKS=1" ^
    "-DMARL_WARNINGS_AS_ERRORS=1" ^
    "-DMARL_DEBUG_ENABLED=1" ^
    "-DMARL_INSTALL=1" ^
    "-DBENCHMARK_ENABLE_INSTALL=0" ^
    "-DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%"
if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
cmake --build . --config %CONFIG% --target install
