@echo off

if "%1"=="/?" goto :showhelp
if "%1"=="-?" goto :showhelp
if "%1"=="-help" goto :showhelp
if "%1"=="--help" goto :showhelp

setlocal

if "%HLSL_SRC_DIR%"=="" (
  echo Missing source directory.
  if exist %~dp0..\..\LLVMBuild.txt (
    set HLSL_SRC_DIR=%~dp0..\..
    echo Source directory deduced to be %~dp0..\..
  ) else (
    exit /b 1
  )
)

set _TRACE_SESSION_NAME=hctetwsession
if "%_PROVIDER_GUIDS%" == "" (
  echo _PROVIDER_GUIDS not set - setting to dxcompiler provider.
  set _PROVIDER_GUIDS=5c65fe8c-9f96-4bfd-9a87-9f8ebd45da64
) else (
  echo _PROVIDER_GUIDS already set - appending dxcompiler providers.
  set _PROVIDER_GUIDS=%_PROVIDER_GUIDS%+5c65fe8c-9f96-4bfd-9a87-9f8ebd45da64
)
set _TRACE_FILENAME=%TEMP%\hctetwsession.etl
if not "%2"=="" (
  set _TRACE_FILENAME=%2
)

if "%1"=="-start" (
  call :do_start
) else if "%1"=="-stop" (
  call :do_stop
) else if "%1"=="-view" (
  call :do_view
) else (
  echo Unknown command.
  goto :showhelp
)

goto :eof

:do_start
xperf -start %_TRACE_SESSION_NAME% -on %_PROVIDER_GUIDS% -f %_TRACE_FILENAME%
if errorlevel 1 (
  echo xperf command failed to start the session
  exit /b 1
)
exit /b 0

:do_stop
xperf -stop %_TRACE_SESSION_NAME%
if errorlevel 1 (
  echo xperf command failed to stop the session
  exit /b 1
)
exit /b 0

:do_view
if not exist %_TRACE_FILENAME% (
  echo %_TRACE_FILENAME% does not exist.
  echo Consider running hcttrace -start
  exit /b 1
)
tracerpt %_TRACE_FILENAME% -import %HLSL_SRC_DIR%\include\dxc\Tracing\dxcetw.man -y -o %_TRACE_FILENAME%.xml
if errorlevel 1 (
  echo Failed to generate trace report.
  exit /b 1
)
rem Move to location of prior file.
rem cscript //Nologo %HLSL_SRC_DIR%\utils\hct\hcttracei.js /i:%_TRACE_FILENAME%.xml
where python.exe 1>nul 2>nul
if errorlevel 1 (
  rem TODO - discover rather than hard-code
  %SystemDrive%\Python27\python.exe %HLSL_SRC_DIR%\utils\hct\hcttracei.py %_TRACE_FILENAME%.xml
) else (
  python.exe %HLSL_SRC_DIR%\utils\hct\hcttracei.py %_TRACE_FILENAME%.xml
)
exit /b 0
