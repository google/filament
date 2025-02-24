# ~~~
# Copyright (c) 2014-2025 Valve Corporation
# Copyright (c) 2014-2025 LunarG, Inc.
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

project(VVL LANGUAGES CXX)

# This variable enables downstream users to customize the target API
# variant (e.g. Vulkan SC)
set(API_TYPE "vulkan")

add_subdirectory(scripts)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN "YES")

include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Remove when min is 3.26, see CMP0143

add_compile_definitions(VK_ENABLE_BETA_EXTENSIONS)
if(WIN32)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)

    # Allow usage of unsafe CRT functions and minimize what Windows.h leaks
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
elseif(ANDROID)
    add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    add_compile_definitions(VK_USE_PLATFORM_METAL_EXT)
    if (IOS)
        add_compile_definitions(VK_USE_PLATFORM_IOS_MVK)
    endif()
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        add_compile_definitions(VK_USE_PLATFORM_MACOS_MVK)
    endif()
else()
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)

    if (BUILD_WSI_XCB_SUPPORT OR BUILD_WSI_XLIB_SUPPORT OR BUILD_WSI_WAYLAND_SUPPORT)
        find_package(PkgConfig REQUIRED QUIET)
    endif()

    if(BUILD_WSI_XCB_SUPPORT)
        pkg_check_modules(XCB REQUIRED QUIET IMPORTED_TARGET xcb)
        add_compile_definitions(VK_USE_PLATFORM_XCB_KHR)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        pkg_check_modules(X11 REQUIRED QUIET IMPORTED_TARGET x11)
        add_compile_definitions(VK_USE_PLATFORM_XLIB_KHR VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        pkg_check_modules(WAYlAND_CLIENT QUIET REQUIRED IMPORTED_TARGET wayland-client)
        add_compile_definitions(VK_USE_PLATFORM_WAYLAND_KHR)
    endif()
endif()

# Tracy
option(VVL_ENABLE_TRACY "Enable profiling with Tracy" OFF)
option(VVL_ENABLE_TRACY_CPU_MEMORY "Enable profiling memory with Tracy" OFF)
option(VVL_TRACY_CALLSTACK "Size of the collected collected call stacks" 48)
if (VVL_ENABLE_TRACY)
    if(VVL_ENABLE_TRACY_CPU_MEMORY AND USE_MIMALLOC)
        message(FATAL_ERROR "Tracy cannot be used with mimalloc")
    endif()

    if (VVL_TRACY_CALLSTACK STREQUAL "OFF")
        set(VVL_TRACY_CALLSTACK 48)
    endif()

    add_compile_definitions(
        TRACY_ENABLE
        TRACY_CALLSTACK=${VVL_TRACY_CALLSTACK}
        TRACY_DELAYED_INIT
        TRACY_MANUAL_LIFETIME
    )

    if(VVL_ENABLE_TRACY_CPU_MEMORY)
        add_compile_definitions(VVL_TRACY_CPU_MEMORY)
    endif()

    if(VVL_ENABLE_TRACY_GPU)
        add_compile_definitions(
            VVL_TRACY_GPU
            TRACY_VK_USE_SYMBOL_TABLE
        )
    endif()
endif()


find_package(VulkanHeaders CONFIG QUIET)

find_package(VulkanUtilityLibraries CONFIG QUIET)

find_package(SPIRV-Headers CONFIG QUIET)

find_package(SPIRV-Tools-opt CONFIG QUIET)

# NOTE: Our custom code generation target isn't desirable for system package managers or add_subdirectory users.
# So this target needs to be off by default to avoid obtuse build errors or patches.
option(VVL_CODEGEN "Enable vulkan validation layer code generation")
if (VVL_CODEGEN)
    find_package(Python3 REQUIRED)
    add_custom_target(vvl_codegen
        COMMAND Python3::Interpreter "${VVL_SOURCE_DIR}/scripts/generate_source.py"
            "${VULKAN_HEADERS_INSTALL_DIR}/${CMAKE_INSTALL_DATADIR}/vulkan/registry"
            "${SPIRV_HEADERS_INSTALL_DIR}/include/spirv/unified1"
            --incremental --generated-version ${VulkanHeaders_VERSION} --api ${API_TYPE}
        WORKING_DIRECTORY ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated
    )
endif()

if (ANNOTATED_SPEC_LINK)
    message("ANNOTATED_SPEC_LINK is ${ANNOTATED_SPEC_LINK}")
    add_compile_definitions(ANNOTATED_SPEC_LINK=${ANNOTATED_SPEC_LINK})
endif()

option(BUILD_WERROR "Treat compiler warnings as errors")
if (BUILD_WERROR)
    add_compile_options("$<IF:$<CXX_COMPILER_ID:MSVC>,/WX,-Werror>")
endif()

option(VVL_ENABLE_ASAN "Use address sanitization")
if (VVL_ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    if (NOT MSVC)
        add_link_options(-fsanitize=address)
    endif()
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "(GNU|Clang)")
    add_compile_options(
        -Wall
        -Wextra
        -Wpointer-arith
    )
    if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        add_compile_options(
            -Wconversion
            -Wimplicit-fallthrough
            -Wstring-conversion
        )

        if (CMAKE_BUILD_TYPE MATCHES "Debug")
            # When using tools such as lldb, strings will produce "error: summary string parsing error"
            add_compile_options(-fstandalone-debug)
        endif()
    endif()
elseif(MSVC)
    add_compile_options(
        /W4
        /we5038 # Enable warning about MIL ordering in constructors
    )

    # Enforce stricter ISO C++
    add_compile_options(/permissive-)
    add_compile_options(/utf-8)

    # PDBs aren't generated on Release builds by default.
    add_compile_options("$<$<CONFIG:Release>:/Zi>")
    add_link_options("$<$<CONFIG:Release>:/DEBUG:FULL>")

    # Enable /LTCG (Link-time code generation)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    # Remove unreferenced code/data.
    add_link_options("$<$<CONFIG:Release>:/OPT:REF>")
    # Merge duplicate data/functions
    add_link_options("$<$<CONFIG:Release>:/OPT:ICF>")

    add_compile_options($<$<BOOL:${MSVC_IDE}>:/MP>) # Speed up Visual Studio builds
endif()

add_subdirectory(layers)

option(BUILD_TESTS "Build the tests")
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    # Set tests as Visual Studio startup project
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT vk_layer_validation_tests)
endif()
