if defined KOKORO_BUILD_ID (
    echo Installing CMake
    choco install cmake -y
    refreshenv

    echo Installing LLVM
    choco install llvm --version 6.0.1 -y
    refreshenv
)

echo Exiting ci-common
exit /b 0
