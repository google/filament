#!/usr/bin/env bash

set -e

# Checkout LLVM sources
git clone --filter=blob:none --depth=1 --branch llvmorg-19.1.6 --no-checkout https://github.com/llvm/llvm-project.git llvm-project
cd llvm-project
git sparse-checkout set --cone
git checkout llvmorg-19.1.6
git sparse-checkout set cmake llvm/cmake runtimes libcxx libcxxabi
cd ..

## Setup libc++ options
if [ -z "$BUILD_32_BITS" ]; then
  export BUILD_32_BITS=OFF && echo disabling 32 bit build
fi

## Build and install libc++ (Use unstable ABI for better sanitizer coverage)
mkdir llvm-build && cd llvm-build
cmake -GNinja                                   \
      -DCMAKE_C_COMPILER=${CC}                  \
      -DCMAKE_CXX_COMPILER=${CXX}               \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo         \
      -DCMAKE_INSTALL_PREFIX=/usr               \
      -DLIBCXX_ABI_UNSTABLE=OFF                 \
      -DLLVM_USE_SANITIZER=${LIBCXX_SANITIZER}  \
      -DLLVM_BUILD_32_BITS=${BUILD_32_BITS}     \
      -DLIBCXXABI_USE_LLVM_UNWINDER=OFF         \
      -DLLVM_INCLUDE_TESTS=OFF                  \
      -DLIBCXX_INCLUDE_TESTS=OFF                \
      -DLIBCXX_INCLUDE_BENCHMARKS=OFF           \
      -DLLVM_ENABLE_RUNTIMES='libcxx;libcxxabi' \
      ../llvm-project/runtimes/
cmake --build . -- cxx cxxabi
cd ..
