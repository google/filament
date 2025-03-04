# Copyright 2023-2024 The Khronos Group Inc.
# Copyright 2023-2024 Valve Corporation
# Copyright 2023-2024 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
target_include_directories(VulkanLayerSettings PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_sources(VulkanLayerSettings PRIVATE
    vulkan/layer/vk_layer_settings.h
    vulkan/layer/vk_layer_settings.hpp
)

target_include_directories(VulkanSafeStruct PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_sources(VulkanSafeStruct PRIVATE
        vulkan/utility/vk_safe_struct.hpp
        vulkan/utility/vk_safe_struct_utils.hpp
)

set(CMAKE_FOLDER "${CMAKE_FOLDER}/VulkanUtilityHeaders")

add_library(VulkanUtilityHeaders INTERFACE)
add_library(Vulkan::UtilityHeaders ALIAS VulkanUtilityHeaders)

# https://cmake.org/cmake/help/latest/release/3.19.html#other
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.19")
    target_sources(VulkanUtilityHeaders PRIVATE
        vulkan/vk_enum_string_helper.h
        vulkan/utility/vk_concurrent_unordered_map.hpp
        vulkan/utility/vk_dispatch_table.h
        vulkan/utility/vk_format_utils.h
        vulkan/utility/vk_small_containers.hpp
        vulkan/utility/vk_sparse_range_map.hpp
        vulkan/utility/vk_struct_helper.hpp
    )
endif()

target_link_libraries(VulkanUtilityHeaders INTERFACE Vulkan::Headers)

target_include_directories(VulkanUtilityHeaders INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

