@echo off
set selfName=%0

:: Check that we have at least 3 arguments.
set argC=0
for %%x in (%*) do Set /A argC+=1
if %argC% LSS 3 goto :print_help

set AR_TOOL=%1
shift
set OUTPUT_PATH=%1
shift

set ARCHIVES=
:loop
if [%1]==[] goto :finished
set ARCHIVES=%ARCHIVES% %1
shift
goto :loop

:finished

%AR_TOOL% /nologo /out:%OUTPUT_PATH% %ARCHIVES% || goto :error

exit /B %ERRORLEVEL%

:print_help
echo %selfName%. Combine multiple static libraries using an archiver tool.
echo;
echo Usage:
echo     %selfName% ^<path-to-ar^> ^<output-archive^> ^<archives^>...
echo;
echo Notes:
echo     ^<archives^> must be a list of absolute paths to static library archives.
echo     This script creates a temporary working directory inside the current directory.
exit /B 1

:error
exit /B %ERRORLEVEL%
