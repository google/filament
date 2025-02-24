@echo off
:: Copyright (c) 2017 The Chromium Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

:: This script will determine if python or git binaries need updates. It
:: returns !0 as failure

:: Note: we set EnableDelayedExpansion so we can perform string manipulations
:: in our manifest parsing loop. This only works on Windows XP+.
setlocal EnableDelayedExpansion

:: Get absolute root directory (.js scripts don't handle relative paths well).
pushd %~dp0..
set BOOTSTRAP_ROOT_DIR=%CD%
popd

:: Extra arguments to pass to our "win_tools.py" script.
set BOOTSTRAP_EXTRA_ARGS=

:: Determine if we're running a bleeding-edge installation.
if not exist "%BOOTSTRAP_ROOT_DIR%\.bleeding_edge" (
  set CIPD_MANIFEST=manifest.txt
) else (
  set CIPD_MANIFEST=manifest_bleeding_edge.txt
)

:: Parse our CIPD manifest and identify the "cpython3" version. We do this by
:: reading it line-by-line, identifying the line containing "cpython3", and
:: stripping all text preceding "version:". This leaves us with the version
:: string.
::
:: This method requires EnableDelayedExpansion, and extracts the Python version
:: from our CIPD manifest. Variables referenced using "!" instead of "%" are
:: delayed expansion variables.
for /F "usebackq tokens=*" %%A in ("%~dp0%CIPD_MANIFEST%") do (
  set LINE=%%A
  if not "x!LINE:cpython3/=!" == "x!LINE!" set PYTHON3_VERSION=!LINE:*version:=!
)
if "%PYTHON3_VERSION%" == "" (
  @echo Could not extract Python version from manifest.
  set ERRORLEVEL=1
  goto :END
)

:: We will take the version string, replace "." with "_", and surround it with
:: "bootstrap-<PYTHON3_VERSION>_bin" so that it matches "win_tools.py"'s cleanup
:: expression and ".gitignore".
::
:: We incorporate PYTHON3_VERSION into the "win_tools" directory name so that
:: new installations don't interfere with long-running Python processes if
:: Python is upgraded.
set BOOTSTRAP_NAME=bootstrap-%PYTHON3_VERSION:.=_%_bin
set BOOTSTRAP_PATH=%BOOTSTRAP_ROOT_DIR%\%BOOTSTRAP_NAME%
set BOOTSTRAP_EXTRA_ARGS=--bootstrap-name "%BOOTSTRAP_NAME%"

:: Install our CIPD packages. The CIPD client self-bootstraps.
:: See "//cipd.bat" and "//.cipd_impl.ps1" for more information.
set CIPD_EXE=%BOOTSTRAP_ROOT_DIR%\cipd.bat
call "%CIPD_EXE%" ensure -log-level warning -ensure-file "%~dp0%CIPD_MANIFEST%" -root "%BOOTSTRAP_PATH%"
if errorlevel 1 goto :END

:: This executes "win_tools.py" using the bundle's Python interpreter.
set BOOTSTRAP_PYTHON_BIN=%BOOTSTRAP_PATH%\python3\bin\python3.exe
call "%BOOTSTRAP_PYTHON_BIN%" "%~dp0bootstrap.py" %BOOTSTRAP_EXTRA_ARGS%


:END
set EXPORT_ERRORLEVEL=%ERRORLEVEL%
endlocal & (
  set ERRORLEVEL=%EXPORT_ERRORLEVEL%
)
exit /b %ERRORLEVEL%
