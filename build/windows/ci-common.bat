if defined KOKORO_BUILD_ID (
    choco install cmake -y

    :: retry if choco install failed
    if errorlevel 1 (
        choco install cmake -y
        if errorlevel 1 exit /b %errorlevel%
    )

    refreshenv

    choco install llvm --version 8.0.0 -y

    :: retry if choco install failed
    if errorlevel 1 (
        choco install llvm --version 8.0.8 -y
        if errorlevel 1 exit /b %errorlevel%
    )

    :: refreshenv is necessary to put CMake and LLVM on path
    refreshenv
)

exit /b 0
