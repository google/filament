# ~~~
# Copyright 2022-2023 The Khronos Group Inc.
# Copyright 2022-2023 Valve Corporation
# Copyright 2022-2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
# ~~~
cmake_minimum_required(VERSION 3.22.1)

project(API LANGUAGES C CXX)

if (FIND_PACKAGE_TESTING)
    find_package(VulkanHeaders REQUIRED CONFIG)
else()
    add_subdirectory(../../ ${CMAKE_CURRENT_BINARY_DIR}/headers)
endif()

if (NOT TARGET Vulkan::Headers)
    message(FATAL_ERROR "Vulkan::Headers target not defined")
endif()
if (NOT TARGET Vulkan::HppModule AND VULKAN_HEADERS_ENABLE_MODULE)
    message(FATAL_ERROR "Vulkan::HppModule target not defined")
endif()

if (FIND_PACKAGE_TESTING)
    if (NOT DEFINED VulkanHeaders_VERSION)
        message(FATAL_ERROR "VulkanHeaders_VERSION not defined!")
    endif()
    message(STATUS "VulkanHeaders_VERSION = ${VulkanHeaders_VERSION}")
endif()

if (NOT FIND_PACKAGE_TESTING)
    # Consuming vulkan-headers via add_subdirectory should NOT add installation code to the parent CMake project.
    if (DEFINED CMAKE_INSTALL_INCLUDEDIR)
        message(FATAL_ERROR "CMAKE_INSTALL_INCLUDEDIR was defined!")
    endif()

    # NOTE: Some users may not be using the namespace target.
    # Don't accidentally break them unless we have to.
    if (NOT TARGET Vulkan-Headers)
        message(FATAL_ERROR "Backcompat for Vulkan-Headers target broken!")
    endif()
    if (NOT TARGET Vulkan-HppModule AND VULKAN_HEADERS_ENABLE_MODULE)
        message(FATAL_ERROR "Backcompat for Vulkan-HppModule target broken!")
    endif()
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(${CMAKE_C_COMPILER_ID} MATCHES "(GNU|Clang)")
    add_compile_options(
        -Wpedantic
        -Wall
        -Wextra
        -Werror
    )
endif()

if (MSVC)
    add_compile_options(
        /W4
        /permissive-
        /WX
    )
endif()

# Test the non-API headers provided by this repo
# NOTE: For us testing just means that these header files compile
# with reasonable warnings.

# vk_icd.h
add_library(vk_icd MODULE ../vk_icd.c)
target_link_libraries(vk_icd PRIVATE Vulkan::Headers)

# vk_layer.h
add_library(vk_layer MODULE ../vk_layer.c)
target_link_libraries(vk_layer PRIVATE Vulkan::Headers)

# vulkan.hpp
add_library(vk_hpp MODULE ../vk_hpp.cpp)
target_compile_features(vk_hpp PUBLIC cxx_std_11)
target_link_libraries(vk_hpp PRIVATE Vulkan::Headers)

# vulkan.cppm
if (VULKAN_HEADERS_ENABLE_MODULE)
    add_library(vk_hpp_module MODULE ../vk_hpp_module.cpp)
    set_target_properties(vk_hpp_module PROPERTIES CXX_SCAN_FOR_MODULES ON)
    target_link_libraries(vk_hpp_module PRIVATE Vulkan::HppModule)
endif()