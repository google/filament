# ~~~
# Copyright 2018-2023 The Khronos Group Inc.
# Copyright 2018-2023 Valve Corporation
# Copyright 2018-2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
# ~~~
cmake_minimum_required(VERSION 3.22.1)

include(CMakeDependentOption)


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
cmake_dependent_option(VULKAN_HEADERS_ENABLE_MODULE "Build and install the Vulkan-Hpp C++ named modules" ON [[ 23 IN_LIST CMAKE_CXX_COMPILER_IMPORT_STD ]] OFF)
# This CMake target is only available when the entire stack (CMake, compiler, standard library) supports C++20 modules and supports importing the standard library as a module.
# See https://gitlab.kitware.com/ben.boeckel/cmake/-/blob/cxxmodules-docs/Help/manual/cmake-cxxmodules.7.rst?ref_type=heads&plain=1#L398

# set up Vulkan-Headers
add_library(Vulkan-Headers INTERFACE)
add_library(Vulkan::Headers ALIAS Vulkan-Headers)
target_include_directories(Vulkan-Headers INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if (VULKAN_HEADERS_ENABLE_MODULE)
    add_library(Vulkan-HppModule OBJECT)
    add_library(Vulkan::HppModule ALIAS Vulkan-HppModule)
    target_sources(Vulkan-HppModule PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/vulkan.cppm"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/vulkan_video.cppm")

    target_compile_features(Vulkan-HppModule PUBLIC cxx_std_23) # C++23 is required for both the standard library module, and other details of Vulkan-Hpp such as std::expected
    set_target_properties(Vulkan-HppModule PROPERTIES CXX_MODULE_STD ON)
    target_link_libraries(Vulkan-HppModule PUBLIC Vulkan::Headers)

    # set up fallback targets to notify about name deprecation
    add_library(Vulkan-Module INTERFACE)
    add_library(Vulkan::Module ALIAS Vulkan-Module)
    target_link_libraries(Vulkan-Module INTERFACE Vulkan::HppModule)
    set_target_properties(Vulkan-Module PROPERTIES
        DEPRECATION "The Vulkan-Module and Vulkan::Module targets have been deprecated by the Vulkan-HppModule and Vulkan::HppModule targets respectively and will be removed at a future date."
    )
endif()

if (VULKAN_HEADERS_ENABLE_TESTS)
    enable_testing() # This is only effective in the top level CMakeLists.txt file.
    add_subdirectory(tests)
endif()

if (VULKAN_HEADERS_ENABLE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Remove the module interface files from the header target if the named module installation is ENABLED.
    # This ensures that the module interface files are only installed once.
    if(VULKAN_HEADERS_ENABLE_MODULE)
        set(CPPM_PATTERN "*.cppm")
    else()
        set(CPPM_PATTERN "")
    endif()

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vk_video" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} PATTERN "${CPPM_PATTERN}" EXCLUDE)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} PATTERN "${CPPM_PATTERN}" EXCLUDE)
    # Preserve source permissions https://github.com/KhronosGroup/Vulkan-Headers/issues/336
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/registry" DESTINATION "${CMAKE_INSTALL_DATADIR}/vulkan" USE_SOURCE_PERMISSIONS PATTERN "${CPPM_PATTERN}" EXCLUDE)

    set_target_properties(Vulkan-Headers PROPERTIES EXPORT_NAME "Headers")
    install(TARGETS Vulkan-Headers
        EXPORT VulkanHeadersConfig
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    # Install the C++ module target and export it
    if (VULKAN_HEADERS_ENABLE_MODULE)
        set_target_properties(Vulkan-HppModule PROPERTIES EXPORT_NAME "HppModule")
        install(TARGETS Vulkan-HppModule
            EXPORT VulkanHeadersConfig
            FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
        install(EXPORT VulkanHeadersConfig
            NAMESPACE "Vulkan::"
            DESTINATION "share/cmake/VulkanHeaders"
            CXX_MODULES_DIRECTORY "")
    else()
        install(EXPORT VulkanHeadersConfig
            NAMESPACE "Vulkan::"
            DESTINATION "share/cmake/VulkanHeaders")
    endif()

    set(version_config "${CMAKE_CURRENT_BINARY_DIR}/generated/VulkanHeadersConfigVersion.cmake")
    write_basic_package_version_file("${version_config}" COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
    install(FILES "${version_config}" DESTINATION "share/cmake/VulkanHeaders")
endif()
