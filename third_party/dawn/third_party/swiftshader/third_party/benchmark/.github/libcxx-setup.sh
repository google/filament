#!/usr/bin/env bash

set -e

# Checkout LLVM sources
git clone --depth=1 https://github.com/llvm/llvm-project.git llvm-project

## Setup libc++ options
if [ -z "$BUILD_32_BITS" ]; then
  export BUILD_32_BITS=OFF && echo disabling 32 bit build
fi

## Build and install libc++ (Use unstable ABI for better sanitizer coverage)
mkdir llvm-build && cd llvm-build
cmake -DCMAKE_C_COMPILER=${CC}                  \
      -DCMAKE_CXX_COMPILER=${CXX}               \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo         \
      -DCMAKE_INSTALL_PREFIX=/usr               \
      -DLIBCXX_ABI_UNSTABLE=OFF                 \
      -DLLVM_USE_SANITIZER=${LIBCXX_SANITIZER}  \
      -DLLVM_BUILD_32_BITS=${BUILD_32_BITS}     \
      -DLLVM_ENABLE_RUNTIMES='libcxx;libcxxabi;libunwind' \
      -G "Unix Makefiles" \
      ../llvm-project/runtimes/
make -j cxx cxxabi unwind
cd ..
