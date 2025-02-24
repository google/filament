<!--
Copyright 2023 The Khronos Group Inc.
Copyright 2023 Valve Corporation
Copyright 2023 LunarG, Inc.

SPDX-License-Identifier: Apache-2.0
-->

# Build Instructions

This document contains the instructions for building this repository on Linux, macOS and Windows.

1. [Requirements](#requirements)
2. [Building Overview](#building-overview)
3. [CMake](#cmake)

## Requirements

1. CMake >= 3.17.2
2. C++ >= c++17 compiler. See platform-specific sections below for supported compiler versions.
3. Python >= 3.8

## Building Overview

The following will be enough for most people, for more detailed instructions, see below.

```bash
cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Debug -D UPDATE_DEPS=ON
cmake --build build --config Debug
```

### Recommended setup for developers

```bash
cmake -S . -B build/ -D VUL_WERROR=ON -D BUILD_TESTS=ON  -D UPDATE_DEPS=ON -D CMAKE_BUILD_TYPE=Debug
```

### Unit Tests

```bash
cd build/
ctest -C Debug --parallel 8 --output-on-failure
```

## CMake

### Warnings as errors off by default!

By default `VUL_WERROR` is `OFF`

The idiom for open source projects is to NOT enable warnings as errors.

System package managers, and language package managers have to build on multiple different platforms and compilers.

By defaulting to `ON` we cause issues for package managers since there is no standard way to disable warnings until CMake 3.24

Add `-D VUL_WERROR=ON` to your workflow. Or use the `dev` preset shown below which will also enabling warnings as errors.
