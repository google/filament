# Build Instructions

1. [Requirements](#requirements)
2. [Building Overview](#building-overview)
3. [Generated source code](#generated-source-code)
4. [Dependencies](#dependencies)
5. [Linux Build](#building-on-linux)
6. [Windows Build](#building-on-windows)
7. [MacOS build](#building-on-macos)
8. [Android Build](#building-for-android)
9. [Installed Files](#installed-files)
9. [Sanitization](#sanitization)

## Requirements

1. CMake >= 3.22.1
2. C++17 compatible toolchain
3. Git
4. Python >= 3.10

NOTE: Python is needed for working on generated code, and helping grab dependencies.
While it's not technically required, it's practically required for most users.

## Building Overview

The following will be enough for most people, for more detailed instructions, see below.

```bash
git clone https://github.com/KhronosGroup/Vulkan-ValidationLayers.git
cd Vulkan-ValidationLayers

cmake -S . -B build -D UPDATE_DEPS=ON -D BUILD_WERROR=ON -D BUILD_TESTS=ON -D CMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

### Warnings as errors off by default!

By default `BUILD_WERROR` is `OFF`. The idiom for open source projects is to NOT enable warnings as errors.

System/language package managers have to build on multiple different platforms and compilers.

By defaulting to `ON` we cause issues for package managers since there is no standard way to disable warnings until CMake 3.24

Add `-D BUILD_WERROR=ON` to your workflow.

## Generated source code

This repository contains generated source code in the `layers/vulkan/generated` directory which is not intended to be modified directly.

Please see the [Generated Code documentation](./docs/generated_code.md) for more information

## Dependencies

Currently this repo has a custom process for grabbing C/C++ dependencies.

Keep in mind this repo predates tools like `vcpkg`, `conan`, etc. Our process is most similar to `vcpkg`.

By specifying `-D UPDATE_DEPS=ON` when configuring CMake we grab dependencies listed in [known_good.json](scripts/known_good.json).

All we are doing is streamlining `building`/`installing` the `known good` dependencies and helping CMake `find` the dependencies.

This is done via a combination of `Python` and `CMake` scripting.

Misc Useful Information:

- By default `UPDATE_DEPS` is `OFF`. The intent is to be friendly by default to system/language package managers.
- You can run `update_deps.py` manually but it isn't recommended for most users.

### How to test new dependency versions

Typically most developers alter `known_good.json` with the commit/branch they are testing.

Alternatively you can modify `CMAKE_PREFIX_PATH` as follows.

```sh
# Delete the CMakeCache.txt which will cache find_* results
rm build -rf/
cmake -S . -B build/ ... -D CMAKE_PREFIX_PATH=~/foobar/my_custom_glslang_install/ ...
```

## Building On Linux

### Linux Build Requirements

This repository is regularly built and tested on the two most recent Ubuntu LTS versions.

```bash
sudo apt-get install git build-essential python3 cmake

# Linux WSI system libraries
sudo apt-get install libwayland-dev xorg-dev
```

### WSI Support Build Options

By default, the repository components are built with support for the
Vulkan-defined WSI display servers: Xcb, Xlib, and Wayland. It is recommended
to build the repository components with support for these display servers to
maximize their usability across Linux platforms. If it is necessary to build
these modules without support for one of the display servers, the appropriate
CMake option of the form `BUILD_WSI_xxx_SUPPORT` can be set to `OFF`.

### Linux 32-bit support

Usage of this repository's contents in 32-bit Linux environments is not
officially supported. However, since this repository is supported on 32-bit
Windows, these modules should generally work on 32-bit Linux.

Here are some notes for building 32-bit targets on a 64-bit Ubuntu "reference"
platform:

```bash
# 32-bit libs
# your PKG_CONFIG configuration may be different, depending on your distribution
sudo apt-get install gcc-multilib g++-multilib libx11-dev:i386
```

Set up your environment for building 32-bit targets:

```bash
export ASFLAGS=--32
export CFLAGS=-m32
export CXXFLAGS=-m32
export PKG_CONFIG_LIBDIR=/usr/lib/i386-linux-gnu
```

## Building On Windows

### Windows Development Environment Requirements

- Windows 10+
- Visual Studio

Note: Anything less than `Visual Studio 2019` is not guaranteed to compile/work.

### Visual Studio Generator

Run CMake to generate [Visual Studio project files](https://cmake.org/cmake/help/latest/guide/user-interaction/index.html#command-line-g-option).

```bash
# NOTE: By default CMake picks the latest version of Visual Studio as the default generator.
cmake -S . -B build

# Open the Visual Studio solution
cmake --open build
```

See the [CMake documentation](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators) for further information on Visual Studio generators.

NOTE: Windows developers don't have to develop in Visual Studio. Visual Studio just helps streamlining the needed C++ toolchain requirements (compilers, linker, etc).

## Building on MacOS

### MacOS Development Environment Requirements

- Xcode

NOTE: MacOS developers don't have to develop in Xcode. Xcode just helps streamlining the needed C++ toolchain requirements (compilers, linker, etc). Similar to Visual Studio on Windows.

### Xcode Generator

To create and open an Xcode project:

```bash
# Create the Xcode project
cmake -S . -B build -G Xcode

# Open the Xcode project
cmake --open build
```

See the [CMake documentation](https://cmake.org/cmake/help/latest/generator/Xcode.html) for further information on the Xcode generator.

## Building For Android

- CMake 3.22.1+
- NDK r25+
- Ninja 1.10+
- Android SDK Build-Tools 34.0.0+

### Android Build Requirements

- Download [Android Studio](https://developer.android.com/studio)
- Install (https://developer.android.com/studio/install)
- From the `Welcome to Android Studio` splash screen, add the following components using the SDK Manager:
  - SDK Platforms > Android 8.0 and newer (API Level 26 or higher)
  - SDK Tools > Android SDK Build-Tools
  - SDK Tools > Android SDK Platform-Tools
  - SDK Tools > Android SDK Tools
  - SDK Tools > NDK
  - SDK Tools > CMake

#### Add Android specifics to environment

NOTE: The following commands are streamlined for Linux but easily transferable to other platforms.
The main intent is setting 2 environment variables and ensuring the NDK and build tools are in the `PATH`.

```sh
# Set environment variables
# https://github.com/actions/runner-images/blob/main/images/linux/Ubuntu2204-Readme.md#environment-variables-2
export ANDROID_SDK_ROOT=$HOME/Android/Sdk
export ANDROID_NDK_HOME=$ANDROID_SDK_ROOT/ndk/X.Y.Z

# Modify path
export PATH=$ANDROID_SDK_ROOT/build-tools/X.Y.Z:$PATH

# (Optional if you have new enough version of CMake + Ninja)
export PATH=$ANDROID_SDK_ROOT/cmake/3.22.1/bin:$PATH

# Verify SDK build-tools is set correctly
which aapt

# Verify CMake/Ninja are in the path
which cmake
which ninja

# Check apksigner
apksigner --help
```

Note: If `apksigner` gives a `java: not found` error you do not have Java in your path.

```bash
# A common way to install on the system
sudo apt install default-jre
```

### Android Build

1. Building libraries to package with your APK

Invoking CMake directly to build the binary is relatively simple.

See https://developer.android.com/ndk/guides/cmake#command-line for CMake NDK documentation.

```sh
# Build release binary for arm64-v8a
cmake -S . -B build \
  -D CMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
  -D ANDROID_PLATFORM=26 \
  -D CMAKE_ANDROID_ARCH_ABI=arm64-v8a \
  -D CMAKE_ANDROID_STL_TYPE=c++_static \
  -D ANDROID_USE_LEGACY_TOOLCHAIN_FILE=NO \
  -D CMAKE_BUILD_TYPE=Release \
  -D UPDATE_DEPS=ON \
  -G Ninja

cmake --build build

cmake --install build --prefix build/install
```

Then you just package the library into your APK under the appropriate lib directory based on the ABI:
https://en.wikipedia.org/wiki/Apk_(file_format)#Package_contents

Alternatively users can also use `scripts/android.py` to build the binaries.

Note: `scripts/android.py` will place the binaries in the `build-android/libs` directory.

```sh
# Build release binary for arm64-v8a
python3 scripts/android.py --config Release --app-abi arm64-v8a --app-stl c++_static
```

`android.py` can also streamline building for multiple ABIs:

```sh
# Build release binaries for all ABIs
python3 scripts/android.py --config Release --app-abi 'armeabi-v7a arm64-v8a x86 x86_64' --app-stl c++_static
```

2. Building the test APK for development purposes

Creating the test APK is a bit of an involved process since it requires running multiple CLI tools after the CMake build has finished.

As a result users are enouraged to use `scripts/android.py` to build the test APK.

This script handles wrapping CMake and various Android CLI tools to create the APK for you.

```sh
# Build a complete test APK with debug binaries for all ABIS
python3 scripts/android.py --config Debug --app-abi 'armeabi-v7a arm64-v8a x86 x86_64' --app-stl c++_shared --apk

# Build a clean test APK with release binaries for arm64-v8a
python3 scripts/android.py --config Release --app-abi arm64-v8a --app-stl c++_shared --apk --clean
```

Note: `scripts/android.py` will place the APK in the `build-android/bin` directory.

## CMake Installed Files

The installation depends on the target platform

For UNIX operating systems:

- *install_dir*`/lib` : The Vulkan validation layer library
- *install_dir*`/share/vulkan/explicit_layer.d` : The VkLayer_khronos_validation.json manifest

`NOTE`: Android doesn't use json manifests for Vulkan layers.

For WIN32:

- *install_dir*`/bin` : The Vulkan validation layer library
- *install_dir*`/bin` : The VkLayer_khronos_validation.json manifest

### Software Installation

After you have built your project you can install using CMake's install functionality.

CMake Docs:
- [Software Installation Guide](https://cmake.org/cmake/help/latest/guide/user-interaction/index.html#software-installation)
- [CLI for installing a project](https://cmake.org/cmake/help/latest/manual/cmake.1.html#install-a-project)

```sh
# EX: Installs Release artifacts into `build/install` directory.
# NOTE: --config is only needed for multi-config generators (Visual Studio, Xcode, etc)
cmake --install build/ --config Release --prefix build/install
```

## Sanitization

[ASAN (Address Sanitization)](https://clang.llvm.org/docs/AddressSanitizer.html) has become a part of our CI process to ensure high quality code.

`-D VVL_ENABLE_ASAN=ON` will enable address sanitization in the build.

You could also set the needed compiler flags via environment variables:
```bash
export CFLAGS=-fsanitize=address
export CXXFLAGS=-fsanitize=address
export LDFLAGS=-fsanitize=address
```

[TSAN (Thread Sanitization)](https://clang.llvm.org/docs/ThreadSanitizer.html) has become a part of our CI process to detect data race bugs.

```bash
# NOTE: ThreadSanitizer generally requires all code to be compiled with -fsanitize=thread to prevent false positives.
# https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual#non-instrumented-code
export CFLAGS=-fsanitize=thread
export CXXFLAGS=-fsanitize=thread
export LDFLAGS=-fsanitize=thread
```

NOTE: `MSVC` currently doesn't offer any form of thread sanitization.
