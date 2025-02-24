REM Copyright 2020 Google LLC
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
SET SRC=%cd%\github\cppdap

cd %SRC%
if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!

git submodule update --init
if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!

SET MSBUILD="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild"
SET CONFIG=Release

mkdir %SRC%\build
cd %SRC%\build
if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!

IF /I "%BUILD_SYSTEM%"=="cmake" (
    cmake .. -G "%BUILD_GENERATOR%" "-DCPPDAP_BUILD_TESTS=1" "-DCPPDAP_BUILD_EXAMPLES=1" "-DCPPDAP_WARNINGS_AS_ERRORS=1"
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
    %MSBUILD% /p:Configuration=%CONFIG% cppdap.sln
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
    Release\cppdap-unittests.exe
    if !ERRORLEVEL! neq 0 exit !ERRORLEVEL!
) ELSE (
    echo "Unknown build system: %BUILD_SYSTEM%"
    exit /b 1
)
