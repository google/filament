#!/bin/bash

# version of clang we want to use
CLANG_VERSION=7
GITHUB_CLANG_VERSION=8
# version of libcxx and libcxxabi we want to use
CXX_VERSION=7.0.0
# version of CMake to use instead of the default one
CMAKE_VERSION=3.13.4
# version of ninja to use
NINJA_VERSION=1.8.2

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
sudo update-alternatives --install /usr/bin/cmake cmake `pwd`/bin/cmake 1000 --force

cd ..

# Steps for GitHub Workflows
if [[ "$GITHUB_WORKFLOW" ]]; then
    sudo wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
    sudo apt-get update
    sudo apt-get install clang-$GITHUB_CLANG_VERSION libc++-$GITHUB_CLANG_VERSION-dev libc++abi-$GITHUB_CLANG_VERSION-dev
    sudo apt-get install mesa-common-dev libxi-dev libxxf86vm-dev

    sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang-${GITHUB_CLANG_VERSION} 100
    sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-${GITHUB_CLANG_VERSION} 100
fi

# Steps specific to our CI environment
# CI runs on Ubuntu 14.04, we need to install clang-7.0 and the
# appropriate libc++ ourselves
if [[ "$KOKORO_BUILD_ID" ]]; then
    sudo ln -s /usr/include/x86_64-linux-gnu/asm /usr/include/asm

    if [[ "$FILAMENT_ANDROID_CI_BUILD" ]]; then
        # Update NDK
        yes | ${ANDROID_HOME}/tools/bin/sdkmanager --update >/dev/null
        yes | ${ANDROID_HOME}/tools/bin/sdkmanager --licenses >/dev/null
    fi

    # Install clang
    # This may or may not be needed...
    # sudo apt-key adv --keyserver apt.llvm.org --recv-keys 15CF4D18AF4F7421
    sudo apt-add-repository "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-$CLANG_VERSION main"
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
    sudo rm -f /etc/apt/sources.list.d/cuda.list
    sudo rm -f /etc/apt/sources.list.d/nvidia-ml.list
    sudo apt-get update -y
    sudo apt-get --assume-yes --force-yes install clang-$CLANG_VERSION

    mkdir -p clang
    cd clang

    # download LLVM sources
    curl -O http://releases.llvm.org/$CXX_VERSION/llvm-$CXX_VERSION.src.tar.xz
    tar xf llvm-$CXX_VERSION.src.tar.xz
    mv llvm-$CXX_VERSION.src llvm_src

    # download libc++ sources
    curl -O http://releases.llvm.org/$CXX_VERSION/libcxx-$CXX_VERSION.src.tar.xz
    tar xf libcxx-$CXX_VERSION.src.tar.xz
    mv libcxx-$CXX_VERSION.src llvm_src/projects/libcxx

    # download libc++abi sources
    curl -O http://releases.llvm.org/$CXX_VERSION/libcxxabi-$CXX_VERSION.src.tar.xz
    tar xf libcxxabi-$CXX_VERSION.src.tar.xz
    mv libcxxabi-$CXX_VERSION.src llvm_src/projects/libcxxabi

    mkdir -p build
    cd build

    export CC=/usr/bin/clang-$CLANG_VERSION
    export CXX=/usr/bin/clang++-$CLANG_VERSION

    # prepare makefiles
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DLIBCXX_ENABLE_RTTI=YES \
        -DLIBCXX_ENABLE_EXCEPTIONS=YES \
        -DLIBCXX_USE_COMPILER_RT=NO \
        -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=YES \
        -DLIBCXXABI_USE_COMPILER_RT=NO \
        ../llvm_src

    # compile libraries
    make -j cxx
    make -j cxxabi

    # install libraries
    (cd projects/libcxx && sudo make -j install)
    (cd projects/libcxxabi && sudo make -j install)

    # install embree 3+ to test buildability of the light baking pipeline
    sudo apt-get install -y alien libtbb-dev
    curl -LO https://github.com/embree/embree/releases/download/v3.5.2/embree-3.5.2.x86_64.rpm.tar.gz
    tar xzf embree-3.5.2.x86_64.rpm.tar.gz
    sudo alien embree3-devel-3.5.2-1.noarch.rpm
    sudo alien embree3-lib-3.5.2-1.x86_64.rpm
    sudo dpkg -i embree3-lib_3.5.2-2_amd64.deb
    sudo dpkg -i embree3-devel_3.5.2-2_all.deb

    cd ../..

    export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
    export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
fi
