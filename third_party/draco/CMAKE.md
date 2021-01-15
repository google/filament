# CMake Build System Overview

[TOC]

This document provides a general layout of the Draco CMake build system.

## Core Build System Files

These files are listed in order of interest to maintainers of the build system.

-   `CMakeLists.txt` is the main driver of the build system. It's responsible
    for defining targets and source lists, surfacing build system options, and
    tying the components of the build system together.

-   `cmake/draco_build_definitions.cmake` defines the macro
    `draco_set_build_definitions()`, which is called from `CMakeLists.txt` to
    configure include paths, compiler and linker flags, library settings,
    platform speficic configuration, and other build system settings that
    depend on optional build configurations.

-   `cmake/draco_targets.cmake` defines the macros `draco_add_library()` and
    `draco_add_executable()` which are used to create all targets in the CMake
    build. These macros attempt to behave in a manner that loosely mirrors the
    blaze `cc_library()` and `cc_binary()` commands. Note that
    `draco_add_executable()` is also used for tests.

-   `cmake/draco_emscripten.cmake` handles Emscripten SDK integration. It
    defines several Emscripten specific macros that are required to build the
    Emscripten specific targets defined in `CMakeLists.txt`.

-   `cmake/draco_flags.cmake` defines macros related to compiler and linker
    flags. Testing macros, macros for isolating flags to specific source files,
    and the main flag configuration function for the library are defined here.

-   `cmake/draco_options.cmake` defines macros that control optional features
    of draco, and help track draco library and build system options.

-   `cmake/draco_install.cmake` defines the draco install target.

-   `cmake/draco_cpu_detection.cmake` determines the optimization types to
    enable based on target system processor as reported by CMake.

-   `cmake/draco_intrinsics.cmake` manages flags for source files that use
    intrinsics. It handles detection of whether flags are necessary, and the
    application of the flags to the sources that need them when they are
    required.

## Helper and Utility Files

-   `.cmake-format.py` Defines coding style for cmake-format.

-   `cmake/draco_helpers.cmake` defines utility macros.

-   `cmake/draco_sanitizer.cmake` defines the `draco_configure_sanitizer()`
    macro, which implements support for `DRACO_SANITIZE`. It handles the
    compiler and linker flags necessary for using sanitizers like asan and msan.

-   `cmake/draco_variables.cmake` defines macros for tracking and control of
    draco build system variables.

## Toolchain Files

These files help facilitate cross compiling of draco for various targets.

-   `cmake/toolchains/aarch64-linux-gnu.cmake` provides cross compilation
    support for arm64 targets.

-   `cmake/toolchains/android.cmake` provides cross compilation support for
    Android targets.

-   `cmake/toolchains/arm-linux-gnueabihf.cmake` provides cross compilation
    support for armv7 targets.

-   `cmake/toolchains/arm64-ios.cmake`, `cmake/toolchains/armv7-ios.cmake`,
    and `cmake/toolchains/armv7s-ios.cmake` provide support for iOS.

-   `cmake/toolchains/arm64-linux-gcc.cmake` and
    `cmake/toolchains/armv7-linux-gcc.cmake` are deprecated, but remain for
    compatibility. `cmake/toolchains/android.cmake` should be used instead.

-   `cmake/toolchains/arm64-android-ndk-libcpp.cmake`,
    `cmake/toolchains/armv7-android-ndk-libcpp.cmake`,
    `cmake/toolchains/x86-android-ndk-libcpp.cmake`, and
    `cmake/toolchains/x86_64-android-ndk-libcpp.cmake` are deprecated, but
    remain for compatibility. `cmake/toolchains/android.cmake` should be used
    instead.

-   `cmake/toolchains/i386-ios.cmake` and `cmake/toolchains/x86_64-ios.cmake`
    provide support for the iOS simulator.

-   `cmake/toolchains/android-ndk-common.cmake` and
    `cmake/toolchains/arm-ios-common.cmake` are support files used by other
    toolchain files.

## Template Files

These files are inputs to the CMake build and are used to generate inputs to the
build system output by CMake.

-   `cmake/draco-config.cmake.template` is used to produce
    draco-config.cmake. draco-config.cmake can be used by CMake to find draco
    when another CMake project depends on draco.

-   `cmake/draco.pc.template` is used to produce draco's pkg-config file.
    Some build systems use pkg-config to configure include and library paths
    when they depend upon third party libraries like draco.
