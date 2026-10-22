// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//
// Author(s):
// - Christophe Riccio <christophe@lunarg.com>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>

typedef enum VkuLayerSettingType {
    VKU_LAYER_SETTING_TYPE_BOOL32 = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
    VKU_LAYER_SETTING_TYPE_INT32 = VK_LAYER_SETTING_TYPE_INT32_EXT,
    VKU_LAYER_SETTING_TYPE_INT64 = VK_LAYER_SETTING_TYPE_INT64_EXT,
    VKU_LAYER_SETTING_TYPE_UINT32 = VK_LAYER_SETTING_TYPE_UINT32_EXT,
    VKU_LAYER_SETTING_TYPE_UINT64 = VK_LAYER_SETTING_TYPE_UINT64_EXT,
    VKU_LAYER_SETTING_TYPE_FLOAT32 = VK_LAYER_SETTING_TYPE_FLOAT32_EXT,
    VKU_LAYER_SETTING_TYPE_FLOAT64 = VK_LAYER_SETTING_TYPE_FLOAT64_EXT,
    VKU_LAYER_SETTING_TYPE_STRING = VK_LAYER_SETTING_TYPE_STRING_EXT,
    VKU_LAYER_SETTING_TYPE_FRAMESET,
    VKU_LAYER_SETTING_TYPE_FRAMESET_STRING
} VkuLayerSettingType;

VK_DEFINE_HANDLE(VkuLayerSettingSet)

// - `first` is an integer related to the first frame to be processed.
//    The frame numbering is 0-based.
//  - `count` is an integer related to the number of frames to be
//    processed. A count of zero represents every frame after the start of the range.
//  - `step` is an integer related to the interval between frames. A step of zero
//    represent no frame to be processed.
//    between frames to be processed.

typedef struct VkuFrameset {
    uint32_t first;
    uint32_t count;
    uint32_t step;
} VkuFrameset;

typedef void(VKAPI_PTR *VkuLayerSettingLogCallback)(const char *pSettingName, const char *pMessage);

// Create a layer setting set. If 'pCallback' is set to NULL, the messages are outputed to stderr.
VkResult vkuCreateLayerSettingSet(const char *pLayerName, const VkLayerSettingsCreateInfoEXT *pFirstCreateInfo,
                                  const VkAllocationCallbacks *pAllocator, VkuLayerSettingLogCallback pCallback,
                                  VkuLayerSettingSet *pLayerSettingSet);

void vkuDestroyLayerSettingSet(VkuLayerSettingSet layerSettingSet, const VkAllocationCallbacks *pAllocator);

// Set a compatibility namespace to find layer settings using environment variables
void vkuSetLayerSettingCompatibilityNamespace(VkuLayerSettingSet layerSettingSet, const char *name);

// Check whether a setting was set either programmatically, from vk_layer_settings.txt or an environment variable
VkBool32 vkuHasLayerSetting(VkuLayerSettingSet layerSettingSet, const char *pSettingName);

// Query setting values
VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName, VkuLayerSettingType type,
                                  uint32_t *pValueCount, void *pValues);

// Find the VkLayerSettingsCreateInfoEXT in the VkInstanceCreateInfo pNext chain, return NULL if not present
const VkLayerSettingsCreateInfoEXT *vkuFindLayerSettingsCreateInfo(const VkInstanceCreateInfo *pCreateInfo);

// Find the VkLayerSettingsCreateInfoEXT in the VkLayerSettingsCreateInfoEXT pNext chain, return NULL if not present
const VkLayerSettingsCreateInfoEXT *vkuNextLayerSettingsCreateInfo(const VkLayerSettingsCreateInfoEXT *pCreateInfo);

// Return the list of Unknown setting in VkLayerSettingsCreateInfoEXT
VkResult vkuGetUnknownSettings(VkuLayerSettingSet layerSettingSet, uint32_t layerSettingsCount, const char **pLayerSettings,
                               const VkLayerSettingsCreateInfoEXT *pFirstCreateInfo, uint32_t *pUnknownSettingCount,
                               const char **pUnknownSettings);

#ifdef __cplusplus
}
#endif
