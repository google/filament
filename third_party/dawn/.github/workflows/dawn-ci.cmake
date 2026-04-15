# This file caches variables which are platform specific.
if (WIN32)
    set(DAWN_USE_BUILT_DXC ON CACHE BOOL "")
    set(CMAKE_SYSTEM_VERSION "$ENV{WIN10_SDK_VERSION}" CACHE STRING "")
    set(CMAKE_WINDOWS_KITS_10_DIR "$ENV{WIN10_SDK_PATH}" CACHE STRING "")
    set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "$ENV{WIN10_SDK_VERSION}" CACHE STRING "")
endif ()
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "")
set(DAWN_ENABLE_INSTALL ON CACHE BOOL "")
if (CMAKE_SYTEM_NAME STREQUAL "Linux")
    # `sccache` seems effective only on linux.
    # for windows, we could look into `buildcache`
    # for macos, `sccache` causes an argument parse error for clang
    # similar to https://github.com/fastbuild/fastbuild/issues/1041
    # maybe we could use `ccache` in macos.
    set(CMAKE_C_COMPILER_LAUNCHER "sccache" CACHE STRING "")
    set(CMAKE_CXX_COMPILER_LAUNCHER "sccache" CACHE STRING "")
endif ()
# Mobile platform configuration
# The workflow should set DAWN_MOBILE_BUILD=ON
# for mobile platforms.
if(DAWN_MOBILE_BUILD)
    message(STATUS "Configuring Dawn Mobile Build")

    # Build type
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")

    # Disable samples and tests
    set(BUILD_SAMPLES OFF CACHE BOOL "")
    set(TINT_BUILD_TESTS OFF CACHE BOOL "")
    set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "")
    set(TINT_BUILD_IR_BINARY OFF CACHE BOOL "")
    set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "")

    # Use static monolithic library
    set(DAWN_BUILD_MONOLITHIC_LIBRARY STATIC CACHE STRING "")
    set(BUILD_SHARED_LIBS STATIC CACHE STRING BOOL "")

    set(DAWN_USE_GLFW OFF CACHE BOOL "")
    # Configure OpenGL variables by default.
    # Enable OpenGL ES only for Android platform
    set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "")
    message(STATUS "- DAWN_MOBILE_BUILD: ${DAWN_MOBILE_BUILD}")
    if(DAWN_MOBILE_BUILD STREQUAL "android")
        message(STATUS "- Enabling OpenGL ES for Android mobile build")
        set(DAWN_ENABLE_OPENGLES ON CACHE BOOL "")
    else()
        message(STATUS "- Disabling OpenGL ES for non-Android mobile build")
        set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "")
    endif()

endif()
