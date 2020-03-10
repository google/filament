:: Copyright (c) 2019 Google LLC.
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::     http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.
::
:: Windows Build Script.

@echo on

set SRC=%cd%\github\SPIRV-Tools

:: Force usage of python 3.6
set PATH=C:\python36;%PATH%

:: Get dependencies
cd %SRC%
git clone --depth=1 https://github.com/KhronosGroup/SPIRV-Headers external/spirv-headers
git clone --depth=1 https://github.com/google/googletest          external/googletest
git clone --depth=1 https://github.com/google/effcee              external/effcee
git clone --depth=1 https://github.com/google/re2                 external/re2

:: REM Install Bazel.
wget -q https://github.com/bazelbuild/bazel/releases/download/0.29.1/bazel-0.29.1-windows-x86_64.zip
unzip -q bazel-0.29.1-windows-x86_64.zip

:: Set up MSVC
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
set BAZEL_VS=C:\Program Files (x86)\Microsoft Visual Studio 14.0
set BAZEL_VC=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC
set BAZEL_SH=c:\tools\msys64\usr\bin\bash.exe
set BAZEL_PYTHON=c:\tools\python2\python.exe

:: #########################################
:: Start building.
:: #########################################
echo "Build everything... %DATE% %TIME%"
bazel.exe build :all
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo "Build Completed %DATE% %TIME%"

:: ##############
:: Run the tests
:: ##############
echo "Running Tests... %DATE% %TIME%"
bazel.exe test :all
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo "Tests Completed %DATE% %TIME%"

exit /b 0

