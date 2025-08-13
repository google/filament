:: Copyright 2023 The Abseil Authors
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::     https://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.

SETLOCAL ENABLEDELAYEDEXPANSION

:: Change directory to the root of the project.
CD %~dp0\..
if %errorlevel% neq 0 EXIT /B 1

:: Use Bazel Vendor mode to reduce reliance on external dependencies.
IF EXIST "%KOKORO_GFILE_DIR%\distdir\abseil-cpp_vendor.tar.gz" (
  tar --force-local -xf "%KOKORO_GFILE_DIR%\distdir\abseil-cpp_vendor.tar.gz" -C c:\
  SET VENDOR_FLAG=--vendor_dir=c:\abseil-cpp_vendor
) ELSE (
  SET VENDOR_FLAG=
)

:: Set the standard version, [c++17|c++latest]
:: https://msdn.microsoft.com/en-us/library/mt490614.aspx
:: The default is c++17 if not set on command line.
IF "%STD%"=="" SET STD=c++17

:: Set the compilation_mode (fastbuild|opt|dbg)
:: https://docs.bazel.build/versions/master/user-manual.html#flag--compilation_mode
:: The default is fastbuild
IF "%COMPILATION_MODE%"=="" SET COMPILATION_MODE=fastbuild

:: Copy the alternate option file, if specified.
IF NOT "%ALTERNATE_OPTIONS%"=="" copy %ALTERNATE_OPTIONS% absl\base\options.h

:: To upgrade Bazel, first download a new binary from
:: https://github.com/bazelbuild/bazel/releases and copy it to
:: /google/data/rw/teams/absl/kokoro/windows.
"%KOKORO_GFILE_DIR%\bazel-8.2.1-windows-x86_64.exe" ^
  test ... ^
  --compilation_mode=%COMPILATION_MODE% ^
  --copt=/WX ^
  --copt=/std:%STD% ^
  --define=absl=1 ^
  --enable_bzlmod=true ^
  --keep_going ^
  --test_env="GTEST_INSTALL_FAILURE_SIGNAL_HANDLER=1" ^
  --test_env=TZDIR="%CD%\absl\time\internal\cctz\testdata\zoneinfo" ^
  --test_output=errors ^
  --test_tag_filters=-benchmark ^
  %VENDOR_FLAG%

if %errorlevel% neq 0 EXIT /B 1
EXIT /B 0
