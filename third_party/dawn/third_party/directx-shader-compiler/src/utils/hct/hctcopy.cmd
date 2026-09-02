@echo off
setlocal

if "%~3"=="" goto :usage

if not exist "%~1\." (
  echo error: No source directory %1
  exit /b 1
)

:collect_files
if "%~3"=="" goto :done
if not exist "%~1\%~3" (
  echo error: source file does not exist: "%~1\%~3"
  exit /b 1
)
set FILES=%FILES% "%~3"
shift /3
goto :collect_files
:done

if not exist "%~2\." mkdir %2
robocopy /NP /NJH /NJS %1 %2 %FILES%
if errorlevel 8 (
  exit /b %errorlevel%
)
exit /b 0

:usage
echo Usage:
echo  hctcopy sourcedir destdir file1 [file2 [file3 ...]]
echo where file# may be wildcard pattern
echo.
echo Uses robocopy plus extra features:
echo  /NP (no percent progress) /NJH (no job header) /NJS (no job summary)
echo  Verify existence of source directory
echo  Verify existence of each file pattern in source directory
echo  Create dest directory if it doesn't exist
exit /b 1
