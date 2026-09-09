# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
set(CMAKE_FOLDER "${CMAKE_FOLDER}/VulkanLayerSettings")

add_library(VulkanLayerSettings)
if(BUILD_SHARED_LIBS)
    set_target_properties(VulkanLayerSettings PROPERTIES
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN NO
        WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
add_library(Vulkan::LayerSettings ALIAS VulkanLayerSettings)

target_sources(VulkanLayerSettings PRIVATE
   vk_layer_settings.cpp
   vk_layer_settings_helper.cpp
   layer_settings_manager.cpp
   layer_settings_manager.hpp
   layer_settings_util.cpp
   layer_settings_util.hpp
)

target_link_Libraries(VulkanLayerSettings
   PUBLIC
      Vulkan::Headers
   PRIVATE
      Vulkan::CompilerConfiguration
)
