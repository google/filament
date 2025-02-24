@echo off
:: Copyright (c) 2016 The Chromium Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

setlocal

set CIPD_BACKEND=https://chrome-infra-packages.appspot.com
set VERSION_FILE=%~dp0cipd_client_version
set CIPD_BINARY=%~dp0.cipd_client.exe
set CIPD_PLATFORM=windows-amd64
set PLATFORM_OVERRIDE_FILE=%~dp0.cipd_client_platform

:: Uncomment to recognize arm64 by default.
:: if %PROCESSOR_ARCHITECTURE%==ARM64 (
::   set CIPD_PLATFORM=windows-arm64
:: )

:: A value in .cipd_client_platform overrides the "guessed" platform.
if exist "%PLATFORM_OVERRIDE_FILE%" (
  for /F usebackq %%l in ("%PLATFORM_OVERRIDE_FILE%") do (
    set CIPD_PLATFORM=%%l
  )
)

:: Nuke the existing client if its platform doesn't match what we want now. We
:: crudely search for a CIPD client package name in the .cipd_version JSON file.
:: It has only "instance_id" as the other field (looking like a base64 string),
:: so mismatches are very unlikely.
set INSTALLED_VERSION_FILE=%~dp0.versions\.cipd_client.exe.cipd_version
findstr /m "infra/tools/cipd/%CIPD_PLATFORM%" "%INSTALLED_VERSION_FILE%" 1>nul 2>nul
if %ERRORLEVEL% neq 0 (
  if exist "%INSTALLED_VERSION_FILE%" (
    echo Detected CIPD client platform change to %CIPD_PLATFORM%. 1>&2
    echo Deleting the existing client to trigger the bootstrap... 1>&2
    del "%CIPD_BINARY%"
    del "%INSTALLED_VERSION_FILE%"
  )
)

if not exist "%CIPD_BINARY%" (
  call :CLEAN_BOOTSTRAP
  goto :EXEC_CIPD
)

call :SELF_UPDATE >nul 2>&1
if %ERRORLEVEL% neq 0 (
  echo CIPD client self-update failed, trying to bootstrap it from scratch... 1>&2
  call :CLEAN_BOOTSTRAP
)

:EXEC_CIPD

if %ERRORLEVEL% neq 0 (
  echo Failed to bootstrap or update CIPD client 1>&2
)
if %ERRORLEVEL% equ 0 (
  "%CIPD_BINARY%" %*
)

endlocal & (
  set EXPORT_ERRORLEVEL=%ERRORLEVEL%
)
exit /b %EXPORT_ERRORLEVEL%


:: Functions below.
::
:: See http://steve-jansen.github.io/guides/windows-batch-scripting/part-7-functions.html
:: if you are unfamiliar with this madness.


:CLEAN_BOOTSTRAP
:: To allow this powershell script to run if it was a byproduct of downloading
:: and unzipping the depot_tools.zip distribution, we clear the Zone.Identifier
:: alternate data stream. This is equivalent to clicking the "Unblock" button
:: in the file's properties dialog.
echo.>"%~dp0.cipd_impl.ps1:Zone.Identifier"
powershell -NoProfile -ExecutionPolicy RemoteSigned ^
    "%~dp0.cipd_impl.ps1" ^
    -CipdBinary "%CIPD_BINARY%" ^
    -Platform "%CIPD_PLATFORM%" ^
    -BackendURL "%CIPD_BACKEND%" ^
    -VersionFile "%VERSION_FILE%" ^
  <nul
if %ERRORLEVEL% equ 0 (
  :: Need to call SELF_UPDATE to setup .cipd_version file.
  call :SELF_UPDATE
)
exit /B %ERRORLEVEL%


:SELF_UPDATE
"%CIPD_BINARY%" selfupdate ^
    -version-file "%VERSION_FILE%" ^
    -service-url "%CIPD_BACKEND%"
exit /B %ERRORLEVEL%
