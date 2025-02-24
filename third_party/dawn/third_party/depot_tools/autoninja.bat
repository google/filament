@echo off
:: Copyright 2017 The Chromium Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

setlocal

set scriptdir=%~dp0

if "%*" == "/?" (
  rem Handle "autoninja /?" which will otherwise give help on the "call" command
  @call %scriptdir%python-bin\python3.bat %~dp0\ninja.py --help
  exit /b
)

:: If a build performance summary has been requested then also set NINJA_STATUS
:: to trigger more verbose status updates. In particular this makes it possible
:: to see how quickly process creation is happening - often a critical clue on
:: Windows. The trailing space is intentional.
if "%NINJA_SUMMARIZE_BUILD%" == "1" set "NINJA_STATUS=[%%r processes, %%f/%%t @ %%o/s : %%es ] "

:: Execute autoninja.py and pass all arguments to it.
@call %scriptdir%python-bin\python3.bat %scriptdir%autoninja.py "%%*"
@if errorlevel 1 goto buildfailure

:: Use call to invoke python script here, because we use python via python3.bat.
@if "%NINJA_SUMMARIZE_BUILD%" == "1" call %scriptdir%python-bin\python3.bat %scriptdir%post_build_ninja_summary.py %*

exit /b %ERRORLEVEL%
:buildfailure

:: Return an error code of 1 so that if a developer types:
:: "autoninja chrome && chrome" then chrome won't run if the build fails.
cmd /c exit 1
