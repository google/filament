# ~~~
# Copyright 2018-2023 The Khronos Group Inc.
# Copyright 2018-2023 Valve Corporation
# Copyright 2018-2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
# ~~~
cmake_minimum_required(VERSION 3.22.1)

# NOTE: Parsing the version like this is suboptimal but neccessary due to our release process:
# https://github.com/KhronosGroup/Vulkan-Headers/pull/346
#
# As shown a more robust approach would be just to add basic test code to check the project version.
function(vlk_get_header_version)
    set(vulkan_core_header_file "${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/vulkan_core.h")
    if (NOT EXISTS ${vulkan_core_header_file})
        message(FATAL_ERROR "Couldn't find vulkan_core.h!")
    endif()

    file(READ ${vulkan_core_header_file} ver)

    if (ver MATCHES "#define[ ]+VK_HEADER_VERSION_COMPLETE[ ]+VK_MAKE_API_VERSION\\([ ]*[0-9]+,[ ]*([0-9]+),[ ]*([0-9]+),[ ]*VK_HEADER_VERSION[ ]*\\)")
        set(MAJOR_VERSION "${CMAKE_MATCH_1}")
        set(MINOR_VERSION "${CMAKE_MATCH_2}")
    else()
        message(FATAL_ERROR "Couldn't get major/minor version")
    endif()

    if (ver MATCHES "#define[ ]+VK_HEADER_VERSION[ ]+([0-9]+)")
        set(PATCH_VERSION "${CMAKE_MATCH_1}")
    else()
        message(FATAL_ERROR "Couldn't get the patch version")
    endif()

    set(VK_VERSION_STRING "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}" PARENT_SCOPE)
endfunction()
vlk_get_header_version()

project(VULKAN_HEADERS LANGUAGES C CXX VERSION ${VK_VERSION_STRING})

# options for Vulkan-Headers and the Vulkan-Hpp C++20 module
option(VULKAN_HEADERS_ENABLE_TESTS "Test Vulkan-Headers" ${PROJECT_IS_TOP_LEVEL})
option(VULKAN_HEADERS_ENABLE_INSTALL "Install Vulkan-Headers" ${PROJECT_IS_TOP_LEVEL})
option(VULKAN_HEADERS_ENABLE_MODULE "Enables building of the Vulkan C++20 module; requires minimum CMake version 3.28" OFF)
option(VULKAN_HEADERS_ENABLE_MODULE_STD "Enables building of the Vulkan C++20 module with import std; requires minimum CMake version 3.30" OFF)

# set up Vulkan-Headers
add_library(Vulkan-Headers INTERFACE)
add_library(Vulkan::Headers ALIAS Vulkan-Headers)
target_include_directories(Vulkan-Headers INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if (VULKAN_HEADERS_ENABLE_MODULE)
    # check for compiler support
    if ((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND (MSVC_VERSION GREATER_EQUAL "1941")) OR
        # clang-cl doesn't currently support modules
        (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
            AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "16.0"
            AND (NOT CMAKE_C_COMPILER_FRONTEND_VARIANT MATCHES "MSVC")
            AND (NOT CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS STREQUAL CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS-NOTFOUND)) OR
        (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "14.0")
    )
        # check for CMake support
        if(VULKAN_HEADERS_ENABLE_MODULE_STD AND CMAKE_VERSION VERSION_LESS "3.30")
            message(FATAL_ERROR "Vulkan-Hpp: C++20 module with import std requires CMake 3.30 or later")
        elseif (CMAKE_VERSION VERSION_LESS "3.28")
            message(FATAL_ERROR "Vulkan-Hpp: C++20 module requires CMake 3.28 or later")
        endif()
        
        # set up Vulkan-HppModule
        add_library(Vulkan-HppModule)
        add_library(Vulkan::HppModule ALIAS Vulkan-HppModule)
        target_sources(Vulkan-HppModule
            PUBLIC
            FILE_SET module
            TYPE CXX_MODULES
            FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/vulkan.cppm")
        if (VULKAN_HEADERS_ENABLE_MODULE_STD)
            target_compile_features(Vulkan-HppModule
                PRIVATE   cxx_std_23
                INTERFACE cxx_std_20) # only C++20 is required to consume this module
            set_target_properties(Vulkan-HppModule PROPERTIES CXX_MODULE_STD ON)
        else()
            target_compile_features(Vulkan-HppModule PUBLIC cxx_std_20)
        endif()
        target_link_libraries(Vulkan-HppModule PUBLIC Vulkan::Headers)

        # Clang 16's module support can be broken with extensions enabled
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "16.0")
            set_target_properties(Vulkan-HppModule PROPERTIES CXX_EXTENSIONS OFF)
        endif()

        # set up fallback targets to notify about name deprecation
        add_library(Vulkan-Module INTERFACE)
        add_library(Vulkan::Module ALIAS Vulkan-Module)
        target_link_libraries(Vulkan-Module INTERFACE Vulkan::HppModule)
        set_target_properties(Vulkan-Module PROPERTIES 
            DEPRECATION "The Vulkan-Module and Vulkan::Module targets have been deprecated by the Vulkan-HppModule and Vulkan::HppModule targets respectively and will be removed at a future date.")
    else()
        message(FATAL_ERROR "Vulkan-Hpp: C++20 module support is requested but was disabled due to lacking compiler support on this platform")
    endif()
endif()

if (VULKAN_HEADERS_ENABLE_TESTS)
    enable_testing() # This is only effective in the top level CMakeLists.txt file.
    add_subdirectory(tests)
endif()

if (VULKAN_HEADERS_ENABLE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vk_video" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    # Preserve source permissions https://github.com/KhronosGroup/Vulkan-Headers/issues/336
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/registry" DESTINATION "${CMAKE_INSTALL_DATADIR}/vulkan" USE_SOURCE_PERMISSIONS)

    set_target_properties(Vulkan-Headers PROPERTIES EXPORT_NAME "Headers")
    install(TARGETS Vulkan-Headers EXPORT VulkanHeadersConfig INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT VulkanHeadersConfig NAMESPACE "Vulkan::" DESTINATION "share/cmake/VulkanHeaders")

    set(version_config "${CMAKE_CURRENT_BINARY_DIR}/generated/VulkanHeadersConfigVersion.cmake")
    write_basic_package_version_file("${version_config}" COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
    install(FILES "${version_config}" DESTINATION "share/cmake/VulkanHeaders")
endif()
