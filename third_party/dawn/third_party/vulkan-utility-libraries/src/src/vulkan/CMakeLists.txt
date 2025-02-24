# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
set(CMAKE_FOLDER "${CMAKE_FOLDER}/VulkanSafeStruct")

add_library(VulkanSafeStruct STATIC)
add_library(Vulkan::SafeStruct ALIAS VulkanSafeStruct)

target_sources(VulkanSafeStruct PRIVATE
    vk_safe_struct_core.cpp
    vk_safe_struct_ext.cpp
    vk_safe_struct_khr.cpp
    vk_safe_struct_utils.cpp
    vk_safe_struct_vendor.cpp
    vk_safe_struct_manual.cpp
)

target_link_Libraries(VulkanSafeStruct
   PUBLIC
      Vulkan::Headers
      Vulkan::UtilityHeaders
   PRIVATE
      Vulkan::CompilerConfiguration
)

if(VUL_MOCK_ANDROID)
	target_compile_definitions(VulkanSafeStruct PUBLIC VK_USE_PLATFORM_ANDROID_KHR VUL_MOCK_ANDROID)
endif()
