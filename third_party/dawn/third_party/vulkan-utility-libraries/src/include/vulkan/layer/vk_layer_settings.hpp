// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//
// Author(s):
// - Christophe Riccio <christophe@lunarg.com>

#pragma once

#include "vk_layer_settings.h"

#include <string>
#include <utility>
#include <vector>

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, bool &settingValue);

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName, std::vector<bool> &settingValues);

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, int32_t &settingValue);

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<int32_t> &settingValues);

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, int64_t &settingValue);

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<int64_t> &settingValues);

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, uint32_t &settingValue);

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<uint32_t> &settingValues);

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, uint64_t &settingValue);

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<uint64_t> &settingValues);

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, float &settingValue);

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName, std::vector<float> &settingValues);

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, double &settingValue);

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName, std::vector<double> &settingValues);

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, std::string &settingValue);

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<std::string> &settingValues);

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, VkuFrameset &settingValue);

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<VkuFrameset> &settingValues);

// Required by vk_safe_struct
typedef std::pair<uint32_t, uint32_t> VkuCustomSTypeInfo;

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<VkuCustomSTypeInfo> &settingValues);

// Return the list of Unknown setting in all VkLayerSettingsCreateInfoEXT
VkResult vkuGetUnknownSettings(VkuLayerSettingSet layerSettingSet, uint32_t layerSettingsCount, const char **pLayerSettings,
                               const VkLayerSettingsCreateInfoEXT *pFirstCreateInfo, std::vector<const char *> &unknownSettings);
