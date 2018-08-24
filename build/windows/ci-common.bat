if defined KOKORO_BUILD_ID (
    choco install llvm --version 6.0.1 -y
    if errorlevel 1 exit /b %errorlevel%

    :: refreshenv is necessary to put LLVM on path
    refreshenv
)

exit /b 0
