@echo off
:: Copyright (c) 2013 The Chromium Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.
setlocal

:: Synchronize the root directory before deferring control back to gclient.py.
call "%~dp0\update_depot_tools.bat"
:: Abort the script if we failed to update depot_tools.
IF %ERRORLEVEL% NEQ 0 (
  exit /b %ERRORLEVEL%
)

:: Ensure that "depot_tools" is somewhere in PATH so this tool can be used
:: standalone, but allow other PATH manipulations to take priority.
set PATH=%PATH%;%~dp0

:: Defer control.
call vpython3 "%~dp0\fetch.py" %*
