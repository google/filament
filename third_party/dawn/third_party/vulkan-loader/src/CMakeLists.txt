# ~~~
# Copyright (c) 2014-2023 Valve Corporation
# Copyright (c) 2014-2023 LunarG, Inc.
# Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# Copyright (c) 2023-2023 RasterGrid Kft.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~
cmake_minimum_required(VERSION 3.22.1)

project(VULKAN_LOADER VERSION 1.4.311 LANGUAGES C)

option(CODE_COVERAGE "Enable Code Coverage" OFF)
if (CODE_COVERAGE)
    include(scripts/CodeCoverage.cmake)
    add_code_coverage_all_targets()
endif()


# This variable enables downstream users to customize the target API
# variant (e.g. Vulkan SC)
set(API_TYPE "vulkan")

add_subdirectory(scripts)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# By default, loader & tests are built without sanitizers
# Use these options to force a specific sanitizer on the loader and test executables
if (UNIX)
    option(LOADER_ENABLE_ADDRESS_SANITIZER "Linux & macOS only: Advanced memory checking" OFF)
    option(LOADER_ENABLE_THREAD_SANITIZER "Linux & macOS only: Advanced thread checking" OFF)
endif()

if(WIN32)
    # Optional: Allow specify the exact version used in the loader dll
    # Format is major.minor.patch.build
    set(BUILD_DLL_VERSIONINFO "" CACHE STRING "Set the version to be used in the loader.rc file. Default value is the currently generated header version")
endif()

find_package(VulkanHeaders CONFIG QUIET)

include(GNUInstallDirs)

set(GIT_BRANCH_NAME "--unknown--")
set(GIT_TAG_INFO "--unknown--")
find_package (Git)
if (GIT_FOUND AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git/HEAD")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_TAG_INFO)
    string(REGEX REPLACE "\n$" "" GIT_TAG_INFO "${GIT_TAG_INFO}")

    file(READ "${CMAKE_CURRENT_LIST_DIR}/.git/HEAD" GIT_HEAD_REF_INFO)
    if (GIT_HEAD_REF_INFO)
        string(REGEX MATCH "ref: refs/heads/(.*)" _ ${GIT_HEAD_REF_INFO})
        if (CMAKE_MATCH_1)
            set(GIT_BRANCH_NAME ${CMAKE_MATCH_1})
        else()
            set(GIT_BRANCH_NAME ${GIT_HEAD_REF_INFO})
        endif()
        string(REGEX REPLACE "\n$" "" GIT_BRANCH_NAME "${GIT_BRANCH_NAME}")
    endif()
endif()

# Enable IDE GUI folders.  "Helper targets" that don't have interesting source code should set their FOLDER property to this
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(LOADER_HELPER_FOLDER "Helper Targets")

if(UNIX)
    set(FALLBACK_CONFIG_DIRS "/etc/xdg" CACHE STRING
            "Search path to use when XDG_CONFIG_DIRS is unset or empty or the current process is SUID/SGID. Default is freedesktop compliant.")
    set(FALLBACK_DATA_DIRS "/usr/local/share:/usr/share" CACHE STRING
            "Search path to use when XDG_DATA_DIRS is unset or empty or the current process is SUID/SGID. Default is freedesktop compliant.")
    set(SYSCONFDIR "" CACHE STRING
            "System-wide search directory. If not set or empty, CMAKE_INSTALL_FULL_SYSCONFDIR and /etc are used.")
endif()

if(WIN32)
    option(ENABLE_WIN10_ONECORE "Link the loader with OneCore umbrella libraries" OFF)
endif()

add_library(platform_wsi INTERFACE)
if(WIN32)
    target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(ANDROID)
    message(FATAL_ERROR "Android build not supported!")
elseif(APPLE)
    target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_METAL_EXT)
    if (IOS)
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_IOS_MVK)
    endif()
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_MACOS_MVK)
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|DragonFly|GNU")
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_XLIB_XRANDR_SUPPORT "Build X11 Xrandr WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)
    option(BUILD_WSI_DIRECTFB_SUPPORT "Build DirectFB WSI support" OFF)

    find_package(PkgConfig REQUIRED QUIET) # Use PkgConfig to find Linux system libraries

    if(BUILD_WSI_XCB_SUPPORT)
        pkg_check_modules(XCB REQUIRED QUIET IMPORTED_TARGET xcb)
        pkg_get_variable(XCB_INCLUDE_DIRS xcb includedir)
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_XCB_KHR)
        target_include_directories(platform_wsi INTERFACE ${XCB_INCLUDE_DIRS})
    endif()
    if(BUILD_WSI_XLIB_SUPPORT)
        pkg_check_modules(X11 REQUIRED QUIET IMPORTED_TARGET x11)
        pkg_get_variable(XLIB_INCLUDE_DIRS x11 includedir)
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_XLIB_KHR)
        target_include_directories(platform_wsi INTERFACE ${XLIB_INCLUDE_DIRS})
        if(BUILD_WSI_XLIB_XRANDR_SUPPORT)
            pkg_check_modules(XRANDR REQUIRED QUIET IMPORTED_TARGET xrandr)
            pkg_get_variable(XLIB_XRANDR_INCLUDE_DIRS xrandr includedir)
            target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_XLIB_XRANDR_EXT)
            target_include_directories(platform_wsi INTERFACE ${XLIB_XRANDR_INCLUDE_DIRS})
        endif()
    endif()
    if(BUILD_WSI_WAYLAND_SUPPORT)
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
    endif()
    if(BUILD_WSI_DIRECTFB_SUPPORT)
        pkg_check_modules(DirectFB QUIET REQUIRED IMPORTED_TARGET directfb)
        pkg_get_variable(DIRECTFB_INCLUDE_DIRS directfb includedir)
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_DIRECTFB_EXT)
        # vulkan_core.h includes <directfb.h> but the header is installed to directfb/directfb.h
        target_include_directories(platform_wsi INTERFACE ${DIRECTFB_INCLUDE_DIRS} ${DIRECTFB_INCLUDE_DIRS}/directfb)
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "QNX")
    message(FATAL_ERROR "See BUILD.md for QNX build")
elseif(CMAKE_SYSTEM_NAME MATCHES "Fuchsia")
    message(FATAL_ERROR "Fuchsia uses Chromium build. See BUILD.gn")
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

add_library(loader_common_options INTERFACE)
target_link_libraries(loader_common_options INTERFACE platform_wsi)

# Enable beta Vulkan extensions
target_compile_definitions(loader_common_options INTERFACE VK_ENABLE_BETA_EXTENSIONS)

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" SYSTEM_PROCESSOR)

option(BUILD_WERROR "Enable warnings as errors")

# Set warnings as errors and the main diagnostic flags
# Must be set first so the warning silencing later on works properly
# Note that clang-cl.exe should use MSVC flavor flags, not GNU
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_COMPILER_FRONTEND_VARIANT MATCHES "MSVC"))
    if (BUILD_WERROR)
        target_compile_options(loader_common_options INTERFACE $<$<COMPILE_LANGUAGE::CXX,C>:/WX>)
    endif()
    target_compile_options(loader_common_options INTERFACE $<$<COMPILE_LANGUAGE::CXX,C>:/W4>)
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # using GCC or Clang with the regular front end
    if (BUILD_WERROR)
        target_compile_options(loader_common_options INTERFACE $<$<COMPILE_LANGUAGE::CXX,C>:-Werror>)
    endif()
    target_compile_options(loader_common_options INTERFACE
        $<$<COMPILE_LANGUAGE::CXX,C>:-Wall>
        $<$<COMPILE_LANGUAGE::CXX,C>:-Wextra>
    )
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(loader_common_options INTERFACE $<$<COMPILE_LANGUAGE::CXX,C>:-Wno-missing-field-initializers>)

    # need to prepend /clang: to compiler arguments when using clang-cl
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND "${CMAKE_C_COMPILER_FRONTEND_VARIANT}" MATCHES "MSVC")
        target_compile_options(loader_common_options INTERFACE $<$<COMPILE_LANGUAGE::CXX,C>:/clang:-fno-strict-aliasing>)
    else()
        target_compile_options(loader_common_options INTERFACE $<$<COMPILE_LANGUAGE::CXX,C>:-fno-strict-aliasing>)
    endif()

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(loader_common_options INTERFACE
            $<$<COMPILE_LANGUAGE::CXX,C>:-Wno-stringop-truncation>
            $<$<COMPILE_LANGUAGE::CXX,C>:-Wno-stringop-overflow>
        )
        if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7.1)
            target_compile_options(loader_common_options INTERFACE $<$<COMPILE_LANGUAGE::CXX,C>:-Wshadow=local>) #only added in GCC 7
        endif()
    endif()

    target_compile_options(loader_common_options INTERFACE $<$<COMPILE_LANGUAGE::CXX,C>:-Wpointer-arith>)

    # Force GLIBC to use the 64 bit interface for file operations instead of 32 bit - More info in issue #1551
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
        target_compile_definitions(loader_common_options INTERFACE _FILE_OFFSET_BITS=64)
    endif()
endif()

if(CMAKE_C_COMPILER_ID MATCHES "MSVC" OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_COMPILER_FRONTEND_VARIANT MATCHES "MSVC"))
    # /sdl: Enable additional security checks
    # /GR-: Disable RTTI
    # /guard:cf: Enable control flow guard
    # /wd4152: Disable warning on conversion of a function pointer to a data pointer
    # /wd4201: Disable warning on anonymous struct/unions
    target_compile_options(loader_common_options INTERFACE
        $<$<COMPILE_LANGUAGE::CXX,C>:/sdl>
        $<$<COMPILE_LANGUAGE::CXX,C>:/GR->
        $<$<COMPILE_LANGUAGE::CXX,C>:/guard:cf>
        $<$<COMPILE_LANGUAGE::CXX,C>:/wd4152>
        $<$<COMPILE_LANGUAGE::CXX,C>:/wd4201>
    )

    # Enable control flow guard
    target_link_options(loader_common_options INTERFACE "LINKER:/guard:cf")

    # Prevent <windows.h> from polluting the code. guards against things like MIN and MAX
    target_compile_definitions(loader_common_options INTERFACE WIN32_LEAN_AND_MEAN)

    # For some reason Advapi32.lib needs to be explicitely linked to when building for Arm (32 bit) on Windows, but isn't required on any other architecture
    if (SYSTEM_PROCESSOR MATCHES "arm" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_link_libraries(loader_common_options INTERFACE Advapi32)
    endif()
endif()

# DEBUG enables runtime loader ICD verification
# Add git branch and tag info in debug mode
target_compile_definitions(loader_common_options INTERFACE $<$<CONFIG:DEBUG>:DEBUG;GIT_BRANCH_NAME="${GIT_BRANCH_NAME}";GIT_TAG_INFO="${GIT_TAG_INFO}">)

if (NOT (WIN32 OR APPLE))
    # Check for the existance of the secure_getenv or __secure_getenv commands
    include(CheckFunctionExists)

    check_function_exists(secure_getenv HAVE_SECURE_GETENV)
    check_function_exists(__secure_getenv HAVE___SECURE_GETENV)

    if (HAVE_SECURE_GETENV)
        target_compile_definitions(loader_common_options INTERFACE HAVE_SECURE_GETENV)
    endif()
    if (HAVE___SECURE_GETENV)
        target_compile_definitions(loader_common_options INTERFACE HAVE___SECURE_GETENV)
    endif()
    if (NOT (HAVE_SECURE_GETENV OR HAVE___SECURE_GETENV))
        message(WARNING "Using non-secure environmental lookups. This loader will not properly disable environent variables when run with elevated permissions.")
    endif()
endif()

option(LOADER_CODEGEN "Enable vulkan loader code generation")
if(LOADER_CODEGEN)
    find_package(Python3 REQUIRED)
    add_custom_target(loader_codegen
        COMMAND Python3::Interpreter ${PROJECT_SOURCE_DIR}/scripts/generate_source.py
            "${VULKAN_HEADERS_INSTALL_DIR}/${CMAKE_INSTALL_DATADIR}/vulkan/registry"
            --generated-version ${VulkanHeaders_VERSION} --incremental --api ${API_TYPE}
    )
endif()

if(UNIX)
    target_compile_definitions(loader_common_options INTERFACE FALLBACK_CONFIG_DIRS="${FALLBACK_CONFIG_DIRS}" FALLBACK_DATA_DIRS="${FALLBACK_DATA_DIRS}")

    if(NOT (SYSCONFDIR STREQUAL ""))
        # SYSCONFDIR is specified, use it and do not force /etc.
        target_compile_definitions(loader_common_options INTERFACE SYSCONFDIR="${SYSCONFDIR}")
    else()
        target_compile_definitions(loader_common_options INTERFACE SYSCONFDIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}")

        # Make sure /etc is searched by the loader
        if(NOT (CMAKE_INSTALL_FULL_SYSCONFDIR STREQUAL "/etc"))
            target_compile_definitions(loader_common_options INTERFACE EXTRASYSCONFDIR="/etc")
        endif()
    endif()
endif()

add_subdirectory(loader)

option(BUILD_TESTS "Build Tests")
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
