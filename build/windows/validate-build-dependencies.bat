@echo off

:: Check that rc.exe is available on PATH
where rc 1>NUL 2>&1
if errorlevel 1 (
    echo Error: could not find rc, exiting
    exit /b 1
)

:: Check that Clang is installed
where clang-cl 1>NUL 2>&1
if errorlevel 1 (
    echo Error: could not find clang-cl, exiting
    exit /b 1
)
where lld-link 1>NUL 2>&1
if errorlevel 1 (
    echo Error: could not find lld-link, exiting
    exit /b 1
)

:: Check that Ninja is installed
where ninja 1>NUL 2>&1
if errorlevel 1 (
    echo Error: could not find ninja, exiting
    exit /b 1
)

:: Check that CMake is installed
where cmake 1>NUL 2>&1
if errorlevel 1 (
    echo Error: could not find cmake, exiting
    exit /b 1
)

exit /b 0
