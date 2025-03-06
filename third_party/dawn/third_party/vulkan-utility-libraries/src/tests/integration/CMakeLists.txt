# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.17)

project(INTEGRATION LANGUAGES C)

add_library(foobar STATIC)

target_compile_features(foobar PRIVATE c_std_99)

target_sources(foobar PRIVATE
    vk_dispatch_table.c
    vk_enum_string_helper.c
    vk_layer_settings.c
    vk_format_utils.c
    vk_format_utils_2.c # Need two translation units to test if header file behaves correctly.
)

if (FIND_PACKAGE_TESTING)
    find_package(VulkanUtilityLibraries REQUIRED CONFIG)
else()
    add_subdirectory(${PROJECT_SOURCE_DIR}/../../ ${PROJECT_BINARY_DIR}/vul)
endif()

# The intent is ensuring we don't accidentally change the names of these
# targets. Since it would break downstream users.
if (NOT TARGET Vulkan::LayerSettings)
    message(FATAL_ERROR "Vulkan::LayerSettings target not defined!")
endif()

if (NOT TARGET Vulkan::UtilityHeaders)
    message(FATAL_ERROR "Vulkan::UtilityHeaders target not defined!")
endif()

target_link_libraries(foobar PRIVATE
    Vulkan::LayerSettings
    Vulkan::UtilityHeaders
)

get_filename_component(VUL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../../include/" ABSOLUTE)

# Get all the header files we SHIP
file(GLOB_RECURSE public_headers
    CONFIGURE_DEPENDS
    "${VUL_INCLUDE_DIR}/*.h" "${VUL_INCLUDE_DIR}/*.hpp"
)

# Ensure each file has the same `vk_` prefix naming convention.
foreach(header IN LISTS public_headers)
    get_filename_component(header ${header} "NAME")
    message(DEBUG "Checking ${header}")
    if (${header} MATCHES "^vk_")
        message(DEBUG "Correct naming convention!")
    else()
        # EX: "Invalid file name! vk_dispatch_table.h"
        message(FATAL_ERROR "Invalid file name! ${header}")
    endif()
endforeach()

# Ensure gn stub file contains all public headers.
# Otherwise we can break the GN build by accident.
get_filename_component(gn_stub_file "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/gn/stub.cpp" ABSOLUTE)
if (NOT EXISTS ${gn_stub_file})
    message(FATAL_ERROR "Couldn't find gn stub file!")
endif()

file(READ ${gn_stub_file} gn_stub_contents)

foreach(header IN LISTS public_headers)
    get_filename_component(header ${header} "NAME")
    if (gn_stub_contents MATCHES "${header}>")
        message(DEBUG "Stub contains ${header}")
    else()
        message(FATAL_ERROR "Update ${gn_stub_file} with ${header}!")
    endif()
endforeach()

# NOTE: Because Vulkan::Headers header files are exposed in the public facing interface
# we must expose this library to users.
get_target_property(property Vulkan::LayerSettings INTERFACE_LINK_LIBRARIES)
if (NOT property MATCHES "Vulkan::Headers")
    message(FATAL_ERROR "Vulkan::Headers not linked properly!")
endif()
get_target_property(property Vulkan::UtilityHeaders INTERFACE_LINK_LIBRARIES)
if (NOT property MATCHES "Vulkan::Headers")
    message(FATAL_ERROR "Vulkan::Headers not linked properly!")
endif()

# Prevent regression of https://github.com/KhronosGroup/Vulkan-Utility-Libraries/issues/103
get_target_property(property Vulkan::LayerSettings INTERFACE_COMPILE_OPTIONS)
if (NOT property STREQUAL "property-NOTFOUND")
    message(FATAL_ERROR "Vulkan::LayerSettings shouldn't export compile options! ${property}")
endif()
get_target_property(property Vulkan::UtilityHeaders INTERFACE_COMPILE_OPTIONS)
if (NOT property STREQUAL "property-NOTFOUND")
    message(FATAL_ERROR "Vulkan::UtilityHeaders shouldn't export compile options! ${property}")
endif()
