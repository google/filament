// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
#include <vulkan/layer/vk_layer_settings.h>

VkBool32 vk_layer_settings() {
    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", NULL, NULL, NULL, &layerSettingSet);

    VkBool32 result = vkuHasLayerSetting(layerSettingSet, "setting_key") ? VK_TRUE : VK_FALSE;

    vkuDestroyLayerSettingSet(layerSettingSet, NULL);

    return result;
}
