// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//
// Author(s):
// - Christophe Riccio <christophe@lunarg.com>
#include "vulkan/layer/vk_layer_settings.hpp"

#include <cstdlib>

static std::string Merge(const std::vector<std::string> &strings) {
    std::string result;

    for (std::size_t i = 0, n = strings.size(); i < n; ++i) {
        if (!result.empty()) {
            result += ",";
        }
        result += strings[i];
    }

    return result;
}

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, bool &settingValue) {
    uint32_t value_count = 1;
    VkBool32 pValues;
    VkResult result =
        vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_BOOL32, &value_count, &pValues);
    settingValue = pValues == VK_TRUE;
    return result;
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName, std::vector<bool> &settingValues) {
    uint32_t value_count = 0;
    VkResult result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_BOOL32, &value_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (value_count > 0) {
        std::vector<VkBool32> values(value_count);
        result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_BOOL32, &value_count, &values[0]);
        for (std::size_t i = 0, n = values.size(); i < n; ++i) {
            settingValues.push_back(values[i] == VK_TRUE);
        }
    }
    return result;
}

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, int32_t &settingValue) {
    uint32_t value_count = 1;
    return vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_INT32, &value_count, &settingValue);
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<int32_t> &settingValues) {
    uint32_t value_count = 1;
    VkResult result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_INT32, &value_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (value_count > 0) {
        settingValues.resize(static_cast<std::size_t>(value_count));
        result =
            vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_INT32, &value_count, &settingValues[0]);
    }
    return result;
}

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, int64_t &settingValue) {
    uint32_t value_count = 1;
    return vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_INT64, &value_count, &settingValue);
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<int64_t> &settingValues) {
    uint32_t value_count = 1;
    VkResult result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_INT64, &value_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (value_count > 0) {
        settingValues.resize(static_cast<std::size_t>(value_count));
        result =
            vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_INT64, &value_count, &settingValues[0]);
    }
    return result;
}

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, uint32_t &settingValue) {
    uint32_t value_count = 1;
    return vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_UINT32, &value_count, &settingValue);
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<uint32_t> &settingValues) {
    uint32_t value_count = 1;
    VkResult result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_UINT32, &value_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (value_count > 0) {
        settingValues.resize(static_cast<std::size_t>(value_count));
        result =
            vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_UINT32, &value_count, &settingValues[0]);
    }
    return result;
}

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, uint64_t &settingValue) {
    uint32_t value_count = 1;
    return vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_UINT64, &value_count, &settingValue);
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<uint64_t> &settingValues) {
    uint32_t value_count = 1;
    VkResult result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_UINT64, &value_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (value_count > 0) {
        settingValues.resize(static_cast<std::size_t>(value_count));
        result =
            vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_UINT64, &value_count, &settingValues[0]);
    }
    return result;
}

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, float &settingValue) {
    uint32_t value_count = 1;
    return vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_FLOAT32, &value_count, &settingValue);
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName, std::vector<float> &settingValues) {
    uint32_t value_count = 1;
    VkResult result =
        vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_FLOAT32, &value_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (value_count > 0) {
        settingValues.resize(static_cast<std::size_t>(value_count));
        result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_FLOAT32, &value_count,
                                          &settingValues[0]);
    }
    return result;
}

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, double &settingValue) {
    uint32_t value_count = 1;
    return vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_FLOAT64, &value_count, &settingValue);
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<double> &settingValues) {
    uint32_t value_count = 1;
    VkResult result =
        vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_FLOAT64, &value_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (value_count > 0) {
        settingValues.resize(static_cast<std::size_t>(value_count));
        result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_FLOAT64, &value_count,
                                          &settingValues[0]);
    }
    return result;
}

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, std::string &settingValue) {
    std::vector<std::string> values;
    VkResult result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, values);
    settingValue = Merge(values);
    return result;
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<std::string> &settingValues) {
    uint32_t value_count = 0;
    VkResult result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_STRING, &value_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (value_count > 0) {
        std::vector<const char *> values(value_count);
        result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_STRING, &value_count, &values[0]);
        settingValues.assign(values.begin(), values.end());
    }
    return result;
}

VkResult vkuGetLayerSettingValue(VkuLayerSettingSet layerSettingSet, const char *pSettingName, VkuFrameset &settingValue) {
    uint32_t value_count = sizeof(VkuFrameset) / sizeof(VkuFrameset::count);
    return vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_UINT32, &value_count, &settingValue);
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<VkuFrameset> &settingValues) {
    uint32_t value_count = 0;
    VkResult result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_UINT32, &value_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (value_count > 0) {
        settingValues.resize(static_cast<std::size_t>(value_count) / (sizeof(VkuFrameset) / sizeof(VkuFrameset::count)));
        result =
            vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_UINT32, &value_count, &settingValues[0]);
    }
    return result;
}

static uint32_t TokenToUint(const std::string &token) {
    uint32_t int_id = 0;
    if ((token.find("0x") == 0) || token.find("0X") == 0) {  // Handle hex format
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 16));
    } else {
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 10));  // Decimal format
    }
    return int_id;
}

static void SetCustomStypeInfo(std::vector<const char *> raw_id_list, std::vector<VkuCustomSTypeInfo> &custom_stype_info) {
    // List format is a list of integer pairs
    for (std::size_t i = 0, n = raw_id_list.size(); i < n; i += 2) {
        uint32_t stype_id = TokenToUint(raw_id_list[i + 0]);
        uint32_t struct_size_in_bytes = TokenToUint(raw_id_list[i + 1]);

        bool found = false;
        // Prevent duplicate entries
        for (auto item : custom_stype_info) {
            if (item.first == stype_id) {
                found = true;
                break;
            }
        }
        if (!found) {
            custom_stype_info.push_back(std::make_pair(stype_id, struct_size_in_bytes));
        }
    }
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName,
                                  std::vector<VkuCustomSTypeInfo> &settingValues) {
    uint32_t value_count = 0;
    VkResult result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_STRING, &value_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (value_count > 0) {
        std::vector<const char *> values(value_count);
        result = vkuGetLayerSettingValues(layerSettingSet, pSettingName, VKU_LAYER_SETTING_TYPE_STRING, &value_count, &values[0]);
        SetCustomStypeInfo(values, settingValues);
    }
    return result;
}

VkResult vkuGetUnknownSettings(VkuLayerSettingSet layerSettingSet, uint32_t layerSettingsCount, const char **pLayerSettings,
                               const VkLayerSettingsCreateInfoEXT *pFirstCreateInfo, std::vector<const char *> &unknownSettings) {
    uint32_t unknown_setting_count = 0;
    VkResult result = vkuGetUnknownSettings(layerSettingSet, layerSettingsCount, pLayerSettings, pFirstCreateInfo,
                                            &unknown_setting_count, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    if (unknown_setting_count > 0) {
        unknownSettings.resize(unknown_setting_count);

        result = vkuGetUnknownSettings(layerSettingSet, layerSettingsCount, pLayerSettings, pFirstCreateInfo,
                                       &unknown_setting_count, &unknownSettings[0]);
    }

    return result;
}
