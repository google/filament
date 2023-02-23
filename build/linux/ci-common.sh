#!/bin/bash

# version of clang we want to use
GITHUB_CLANG_VERSION=14
# version of CMake to use instead of the default one
CMAKE_VERSION=3.19.5
# version of ninja to use
NINJA_VERSION=1.10.2

# Steps for GitHub Workflows
if [[ "$GITHUB_WORKFLOW" ]]; then
    # Install ninja
    wget -q https://github.com/ninja-build/ninja/releases/download/v$NINJA_VERSION/ninja-linux.zip
    unzip -q ninja-linux.zip
    export PATH="$PWD:$PATH"

    # Install CMake
    mkdir -p cmake
    cd cmake

    sudo wget https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-Linux-x86_64.sh
    sudo chmod +x ./cmake-$CMAKE_VERSION-Linux-x86_64.sh
    sudo ./cmake-$CMAKE_VERSION-Linux-x86_64.sh --skip-license > /dev/null
    sudo update-alternatives --install /usr/bin/cmake cmake $(pwd)/bin/cmake 1000 --force

    cd ..

    sudo wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
    sudo apt-get update
    sudo apt-get install clang-$GITHUB_CLANG_VERSION libc++-$GITHUB_CLANG_VERSION-dev libc++abi-$GITHUB_CLANG_VERSION-dev
    sudo apt-get install mesa-common-dev libxi-dev libxxf86vm-dev

    sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang-${GITHUB_CLANG_VERSION} 100
    sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-${GITHUB_CLANG_VERSION} 100
fi
