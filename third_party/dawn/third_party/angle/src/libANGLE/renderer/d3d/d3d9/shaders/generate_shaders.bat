@ECHO OFF
REM
REM Copyright 2013 The ANGLE Project Authors. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.
REM

PATH %PATH%;%ProgramFiles(x86)%\Windows Kits\8.1\bin\x86;%DXSDK_DIR%\Utilities\bin\x86

setlocal
set errorCount=0
set successCount=0
set debug=0

if "%1" == "debug" (
    set debug=1
)
if "%1" == "release" (
    set debug=0
)

::              | Input file          | Entry point           | Type | Output file                        | Debug |
call:BuildShader Blit.vs               standardvs              vs_2_0 compiled\standardvs.h                %debug%
call:BuildShader Blit.ps               passthroughps           ps_2_0 compiled\passthroughps.h             %debug%
call:BuildShader Blit.ps               luminanceps             ps_2_0 compiled\luminanceps.h               %debug%
call:BuildShader Blit.ps               luminancepremultps      ps_2_0 compiled\luminancepremultps.h        %debug%
call:BuildShader Blit.ps               luminanceunmultps       ps_2_0 compiled\luminanceunmultps.h         %debug%
call:BuildShader Blit.ps               componentmaskps         ps_2_0 compiled\componentmaskps.h           %debug%
call:BuildShader Blit.ps               componentmaskpremultps  ps_2_0 compiled\componentmaskpremultps.h    %debug%
call:BuildShader Blit.ps               componentmaskunmultps   ps_2_0 compiled\componentmaskunmultps.h     %debug%

echo.

if %successCount% GTR 0 (
   echo %successCount% shaders compiled successfully.
)
if %errorCount% GTR 0 (
   echo There were %errorCount% shader compilation errors.
)

endlocal
exit /b

:BuildShader
set input=%~1
set entry=%~2
set type=%~3
set output=%~4
set debug=%~5

if %debug% == 0 (
    set "buildCMD=fxc /nologo /E %entry% /T %type% /Fh %output% %input%"
) else (
    set "buildCMD=fxc /nologo /Zi /Od /E %entry% /T %type% /Fh %output% %input%"
)

set error=0
%buildCMD% || set error=1

if %error% == 0 (
    set /a successCount=%successCount%+1
) else (
    set /a errorCount=%errorCount%+1
)

exit /b
