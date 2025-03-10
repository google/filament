// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//
// Author(s):
// - Christophe Riccio <christophe@lunarg.com>
#include "vulkan/layer/vk_layer_settings.h"
#include "layer_settings_manager.hpp"
#include "layer_settings_util.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstring>
#include <memory>
#include <system_error>
#include <unordered_map>

// This is used only for unit tests in test_layer_setting_file
void test_helper_SetLayerSetting(VkuLayerSettingSet layerSettingSet, const char *pSettingName, const char *pValue) {
    assert(layerSettingSet != VK_NULL_HANDLE);
    assert(pSettingName != nullptr);
    assert(pValue != nullptr);

    vl::LayerSettings *layer_setting_set = (vl::LayerSettings *)layerSettingSet;

    layer_setting_set->SetFileSetting(pSettingName, pValue);
}

VkResult vkuCreateLayerSettingSet(const char *pLayerName, const VkLayerSettingsCreateInfoEXT *pFirstCreateInfo,
                                  const VkAllocationCallbacks *pAllocator, VkuLayerSettingLogCallback pCallback,
                                  VkuLayerSettingSet *pLayerSettingSet) {
    (void)pAllocator;

    vl::LayerSettings *layer_setting_set = new vl::LayerSettings(pLayerName, pFirstCreateInfo, pAllocator, pCallback);
    *pLayerSettingSet = (VkuLayerSettingSet)layer_setting_set;

    return VK_SUCCESS;
}

void vkuDestroyLayerSettingSet(VkuLayerSettingSet layerSettingSet, const VkAllocationCallbacks *pAllocator) {
    (void)pAllocator;

    vl::LayerSettings *layer_setting_set = (vl::LayerSettings *)layerSettingSet;
    delete layer_setting_set;
}

void vkuSetLayerSettingCompatibilityNamespace(VkuLayerSettingSet layerSettingSet, const char *prefixName) {
    vl::LayerSettings *layer_setting_set = (vl::LayerSettings *)layerSettingSet;

    layer_setting_set->SetPrefix(prefixName);
}

VkBool32 vkuHasLayerSetting(VkuLayerSettingSet layerSettingSet, const char *pSettingName) {
    assert(layerSettingSet != VK_NULL_HANDLE);
    assert(pSettingName);
    assert(!std::string(pSettingName).empty());

    vl::LayerSettings *layer_setting_set = (vl::LayerSettings *)layerSettingSet;

    const bool has_env_setting = layer_setting_set->HasEnvSetting(pSettingName);
    const bool has_file_setting = layer_setting_set->HasFileSetting(pSettingName);
    const bool has_api_setting = layer_setting_set->HasAPISetting(pSettingName);

    return (has_env_setting || has_file_setting || has_api_setting) ? VK_TRUE : VK_FALSE;
}

VkResult vkuGetLayerSettingValues(VkuLayerSettingSet layerSettingSet, const char *pSettingName, VkuLayerSettingType type,
                                  uint32_t *pValueCount, void *pValues) {
    assert(pValueCount != nullptr);

    if (layerSettingSet == VK_NULL_HANDLE) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (!vkuHasLayerSetting(layerSettingSet, pSettingName)) {
        *pValueCount = 0;
        return VK_SUCCESS;
    }

    if (*pValueCount == 0 && pValues != nullptr) {
        return VK_ERROR_UNKNOWN;
    }

    vl::LayerSettings *layer_setting_set = (vl::LayerSettings *)layerSettingSet;

    // First: search in the environment variables
    const std::string &env_setting_list = layer_setting_set->GetEnvSetting(pSettingName);

    // Second: search in vk_layer_settings.txt
    const std::string &file_setting_list = layer_setting_set->GetFileSetting(pSettingName);

    // Third: search from VK_EXT_layer_settings usage
    const VkLayerSettingEXT *api_setting = layer_setting_set->GetAPISetting(pSettingName);

    // Environment variables overrides the values set by vk_layer_settings
    const std::string setting_list = env_setting_list.empty() ? file_setting_list : env_setting_list;

    if (setting_list.empty() && api_setting == nullptr) {
        return VK_INCOMPLETE;
    }

    const char deliminater = vl::FindDelimiter(setting_list);
    const std::vector<std::string> &settings(vl::Split(setting_list, deliminater));

    const bool copy_values = *pValueCount > 0 && pValues != nullptr;

    switch (type) {
        default: {
            const std::string &message = vl::FormatString("Unknown VkLayerSettingTypeEXT `type` value: %d.", type);
            layer_setting_set->Log(pSettingName, message.c_str());
            return VK_ERROR_UNKNOWN;
        }
        case VKU_LAYER_SETTING_TYPE_BOOL32: {
            std::vector<VkBool32> values;
            VkResult result = VK_SUCCESS;

            if (!settings.empty()) {  // From env variable or setting file
                if (copy_values) {
                    if (static_cast<std::size_t>(*pValueCount) < settings.size()) {
                        result = VK_INCOMPLETE;
                    }
                    values.resize(std::min(static_cast<std::size_t>(*pValueCount), settings.size()));

                    for (std::size_t i = 0, n = values.size(); i < n; ++i) {
                        const std::string &setting_value = vl::ToLower(settings[i]);
                        if (vl::IsInteger(setting_value)) {
                            values[i] = (std::atoi(setting_value.c_str()) != 0) ? VK_TRUE : VK_FALSE;
                        } else if (setting_value == "true" || setting_value == "false") {
                            values[i] = (setting_value == "true") ? VK_TRUE : VK_FALSE;
                        } else if (setting_value == "on" || setting_value == "off") {
                            values[i] = (setting_value == "on") ? VK_TRUE : VK_FALSE;
                        } else {
                            const std::string &message =
                                vl::FormatString("The data provided (%s) is not a boolean value.", setting_value.c_str());
                            layer_setting_set->Log(pSettingName, message.c_str());
                        }
                    }
                } else {
                    *pValueCount = static_cast<std::uint32_t>(settings.size());
                }
            } else if (api_setting != nullptr) {  // From Vulkan Layer Setting API
                if (static_cast<VkuLayerSettingType>(api_setting->type) != type) {
                    result = VK_ERROR_FORMAT_NOT_SUPPORTED;
                } else if (copy_values) {
                    if (*pValueCount < api_setting->valueCount) {
                        result = VK_INCOMPLETE;
                    }
                    const std::uint32_t size = std::min(*pValueCount, api_setting->valueCount);
                    const VkBool32 *data = static_cast<const VkBool32 *>(api_setting->pValues);
                    values.assign(data, data + size);
                } else {
                    *pValueCount = api_setting->valueCount;
                }
            }

            if (copy_values) {
                std::copy(values.begin(), values.end(), reinterpret_cast<VkBool32 *>(pValues));
            }

            return result;
        }
        case VKU_LAYER_SETTING_TYPE_INT32: {
            std::vector<std::int32_t> values;
            VkResult result = VK_SUCCESS;

            if (!settings.empty()) {  // From env variable or setting file
                if (copy_values) {
                    if (static_cast<std::size_t>(*pValueCount) < settings.size()) {
                        result = VK_INCOMPLETE;
                    }
                    values.resize(std::min(static_cast<std::size_t>(*pValueCount), settings.size()));

                    for (std::size_t i = 0, n = values.size(); i < n; ++i) {
                        const std::string &setting_value = vl::ToLower(settings[i]);
                        if (vl::IsInteger(setting_value)) {
                            values[i] = std::atoi(setting_value.c_str());
                        } else {
                            const std::string &message =
                                vl::FormatString("The data provided (%s) is not an integer value.", setting_value.c_str());
                            layer_setting_set->Log(pSettingName, message.c_str());
                        }
                    }
                } else {
                    *pValueCount = static_cast<std::uint32_t>(settings.size());
                }
            } else if (api_setting != nullptr) {  // From Vulkan Layer Setting API
                if (static_cast<VkuLayerSettingType>(api_setting->type) != type) {
                    result = VK_ERROR_FORMAT_NOT_SUPPORTED;
                } else if (copy_values) {
                    if (*pValueCount < api_setting->valueCount) {
                        result = VK_INCOMPLETE;
                    }
                    const std::uint32_t size = std::min(*pValueCount, api_setting->valueCount);
                    const int32_t *data = static_cast<const int32_t *>(api_setting->pValues);
                    values.assign(data, data + size);
                } else {
                    *pValueCount = api_setting->valueCount;
                }
            }

            if (copy_values) {
                std::copy(values.begin(), values.end(), reinterpret_cast<std::int32_t *>(pValues));
            }

            return result;
        }
        case VKU_LAYER_SETTING_TYPE_INT64: {
            std::vector<std::int64_t> values;
            VkResult result = VK_SUCCESS;

            if (!settings.empty()) {  // From env variable or setting file
                if (copy_values) {
                    if (static_cast<std::size_t>(*pValueCount) < settings.size()) {
                        result = VK_INCOMPLETE;
                    }
                    values.resize(std::min(static_cast<std::size_t>(*pValueCount), settings.size()));

                    for (std::size_t i = 0, n = values.size(); i < n; ++i) {
                        const std::string &setting_value = vl::ToLower(settings[i]);
                        if (vl::IsInteger(setting_value)) {
                            int64_t setting{};

                            if (std::from_chars(setting_value.data(), setting_value.data() + setting_value.size(), setting).ec ==
                                std::errc()) {
                                values[i] = setting;
                            } else {
                                const std::string &message =
                                    vl::FormatString("The data provided (%s) is not an INT64 value.", setting_value.c_str());
                                layer_setting_set->Log(pSettingName, message.c_str());
                            }
                        } else {
                            const std::string &message =
                                vl::FormatString("The data provided (%s) is not an integer value.", setting_value.c_str());
                            layer_setting_set->Log(pSettingName, message.c_str());
                        }
                    }
                } else {
                    *pValueCount = static_cast<std::uint32_t>(settings.size());
                }
            } else if (api_setting != nullptr) {  // From Vulkan Layer Setting API
                if (static_cast<VkuLayerSettingType>(api_setting->type) != type) {
                    result = VK_ERROR_FORMAT_NOT_SUPPORTED;
                } else if (copy_values) {
                    if (*pValueCount < api_setting->valueCount) {
                        result = VK_INCOMPLETE;
                    }
                    const std::uint32_t size = std::min(*pValueCount, api_setting->valueCount);
                    const int64_t *data = static_cast<const int64_t *>(api_setting->pValues);
                    values.assign(data, data + size);
                } else {
                    *pValueCount = api_setting->valueCount;
                }
            }

            if (copy_values) {
                std::copy(values.begin(), values.end(), reinterpret_cast<std::int64_t *>(pValues));
            }

            return result;
        }
        case VKU_LAYER_SETTING_TYPE_UINT32: {
            std::vector<std::uint32_t> values;
            VkResult result = VK_SUCCESS;

            if (!settings.empty()) {  // From env variable or setting file
                if (copy_values) {
                    if (static_cast<std::size_t>(*pValueCount) < settings.size()) {
                        result = VK_INCOMPLETE;
                    }
                    values.resize(std::min(static_cast<std::size_t>(*pValueCount), settings.size()));

                    for (std::size_t i = 0, n = values.size(); i < n; ++i) {
                        const std::string &setting_value = vl::ToLower(settings[i]);
                        if (vl::IsInteger(setting_value)) {
                            uint32_t setting{};

                            if (std::from_chars(setting_value.data(), setting_value.data() + setting_value.size(), setting).ec ==
                                std::errc()) {
                                values[i] = setting;
                            } else {
                                const std::string &message =
                                    vl::FormatString("The data provided (%s) is not a UINT32 value.", setting_value.c_str());
                                layer_setting_set->Log(pSettingName, message.c_str());
                            }
                        } else {
                            const std::string &message =
                                vl::FormatString("The data provided (%s) is not an integer value.", setting_value.c_str());
                            layer_setting_set->Log(pSettingName, message.c_str());
                        }
                    }
                } else {
                    *pValueCount = static_cast<std::uint32_t>(settings.size());
                }
            } else if (api_setting != nullptr) {  // From Vulkan Layer Setting API
                if (static_cast<VkuLayerSettingType>(api_setting->type) != type) {
                    result = VK_ERROR_FORMAT_NOT_SUPPORTED;
                } else if (copy_values) {
                    if (*pValueCount < api_setting->valueCount) {
                        result = VK_INCOMPLETE;
                    }
                    const uint32_t size = std::min(*pValueCount, api_setting->valueCount);
                    const uint32_t *data = static_cast<const uint32_t *>(api_setting->pValues);
                    values.assign(data, data + size);
                } else {
                    *pValueCount = api_setting->valueCount;
                }
            }

            if (copy_values) {
                std::copy(values.begin(), values.end(), reinterpret_cast<std::uint32_t *>(pValues));
            }

            return result;
        }
        case VKU_LAYER_SETTING_TYPE_UINT64: {
            std::vector<std::uint64_t> values;
            VkResult result = VK_SUCCESS;

            if (!settings.empty()) {  // From env variable or setting file
                if (copy_values) {
                    if (static_cast<std::size_t>(*pValueCount) < settings.size()) {
                        result = VK_INCOMPLETE;
                    }
                    values.resize(std::min(static_cast<std::size_t>(*pValueCount), settings.size()));

                    for (std::size_t i = 0, n = values.size(); i < n; ++i) {
                        const std::string &setting_value = vl::ToLower(settings[i]);
                        if (vl::IsInteger(setting_value)) {
                            uint64_t setting{};

                            if (std::from_chars(setting_value.data(), setting_value.data() + setting_value.size(), setting).ec ==
                                std::errc()) {
                                values[i] = setting;
                            } else {
                                const std::string &message =
                                    vl::FormatString("The data provided (%s) is not a UINT64 value.", setting_value.c_str());
                                layer_setting_set->Log(pSettingName, message.c_str());
                            }
                        } else {
                            const std::string &message =
                                vl::FormatString("The data provided (%s) is not an integer value.", setting_value.c_str());
                            layer_setting_set->Log(pSettingName, message.c_str());
                        }
                    }
                } else {
                    *pValueCount = static_cast<std::uint32_t>(settings.size());
                }
            } else if (api_setting != nullptr) {  // From Vulkan Layer Setting API
                if (static_cast<VkuLayerSettingType>(api_setting->type) != type) {
                    result = VK_ERROR_FORMAT_NOT_SUPPORTED;
                } else if (copy_values) {
                    if (*pValueCount < api_setting->valueCount) {
                        result = VK_INCOMPLETE;
                    }
                    const std::uint32_t size = std::min(*pValueCount, api_setting->valueCount);
                    const uint64_t *data = static_cast<const uint64_t *>(api_setting->pValues);
                    values.assign(data, data + size);
                } else {
                    *pValueCount = api_setting->valueCount;
                }
            }

            if (copy_values) {
                std::copy(values.begin(), values.end(), reinterpret_cast<std::uint64_t *>(pValues));
            }

            return result;
        }
        case VKU_LAYER_SETTING_TYPE_FLOAT32: {
            std::vector<float> values;
            VkResult result = VK_SUCCESS;

            if (!settings.empty()) {  // From env variable or setting file
                if (copy_values) {
                    if (static_cast<std::size_t>(*pValueCount) < settings.size()) {
                        result = VK_INCOMPLETE;
                    }
                    values.resize(std::min(static_cast<std::size_t>(*pValueCount), settings.size()));

                    for (std::size_t i = 0, n = values.size(); i < n; ++i) {
                        const std::string &setting_value = vl::ToLower(settings[i]);
                        if (vl::IsFloat(setting_value)) {
                            values[i] = static_cast<float>(std::atof(setting_value.c_str()));
                        } else {
                            const std::string &message =
                                vl::FormatString("The data provided (%s) is not a floating-point value.", setting_value.c_str());
                            layer_setting_set->Log(pSettingName, message.c_str());
                        }
                    }
                } else {
                    *pValueCount = static_cast<std::uint32_t>(settings.size());
                }
            } else if (api_setting != nullptr) {  // From Vulkan Layer Setting API
                if (static_cast<VkuLayerSettingType>(api_setting->type) != type) {
                    result = VK_ERROR_FORMAT_NOT_SUPPORTED;
                } else if (copy_values) {
                    if (*pValueCount < api_setting->valueCount) {
                        result = VK_INCOMPLETE;
                    }
                    const std::uint32_t size = std::min(*pValueCount, api_setting->valueCount);
                    const float *data = static_cast<const float *>(api_setting->pValues);
                    values.assign(data, data + size);
                } else {
                    *pValueCount = api_setting->valueCount;
                }
            }

            if (copy_values) {
                std::copy(values.begin(), values.end(), reinterpret_cast<float *>(pValues));
            }

            return result;
        }
        case VKU_LAYER_SETTING_TYPE_FLOAT64: {
            std::vector<double> values;
            VkResult result = VK_SUCCESS;

            if (!settings.empty()) {  // From env variable or setting file
                if (copy_values) {
                    if (static_cast<std::size_t>(*pValueCount) < settings.size()) {
                        result = VK_INCOMPLETE;
                    }
                    values.resize(std::min(static_cast<std::size_t>(*pValueCount), settings.size()));

                    for (std::size_t i = 0, n = values.size(); i < n; ++i) {
                        const std::string &setting_value = vl::ToLower(settings[i]);
                        if (vl::IsFloat(setting_value)) {
                            values[i] = std::atof(setting_value.c_str());
                        } else {
                            const std::string &message =
                                vl::FormatString("The data provided (%s) is not a floating-point value.", setting_value.c_str());
                            layer_setting_set->Log(pSettingName, message.c_str());
                        }
                    }
                } else {
                    *pValueCount = static_cast<std::uint32_t>(settings.size());
                }
            } else if (api_setting != nullptr) {  // From Vulkan Layer Setting API
                if (static_cast<VkuLayerSettingType>(api_setting->type) != type) {
                    result = VK_ERROR_FORMAT_NOT_SUPPORTED;
                } else if (copy_values) {
                    if (*pValueCount < api_setting->valueCount) {
                        result = VK_INCOMPLETE;
                    }
                    const std::uint32_t size = std::min(*pValueCount, api_setting->valueCount);
                    const double *data = static_cast<const double *>(api_setting->pValues);
                    values.assign(data, data + size);
                } else {
                    *pValueCount = api_setting->valueCount;
                }
            }

            if (copy_values) {
                std::copy(values.begin(), values.end(), reinterpret_cast<double *>(pValues));
            }

            return result;
        }
        case VKU_LAYER_SETTING_TYPE_FRAMESET: {
            std::vector<VkuFrameset> values;
            VkResult result = VK_SUCCESS;

            if (!settings.empty()) {  // From env variable or setting file
                if (copy_values) {
                    if (static_cast<std::size_t>(*pValueCount) < settings.size()) {
                        result = VK_INCOMPLETE;
                    }
                    values.resize(std::min(static_cast<std::size_t>(*pValueCount), settings.size()));

                    for (std::size_t i = 0, n = values.size(); i < n; ++i) {
                        const std::string &setting_value = vl::ToLower(settings[i]);
                        if (vl::IsFrameSets(setting_value)) {
                            values[i] = vl::ToFrameSet(setting_value.c_str());
                        } else {
                            const std::string &message =
                                vl::FormatString("The data provided (%s) is not a FrameSet value.", setting_value.c_str());
                            layer_setting_set->Log(pSettingName, message.c_str());
                        }
                    }
                } else {
                    *pValueCount = static_cast<std::uint32_t>(settings.size());
                }
            } else if (api_setting != nullptr) {  // From Vulkan Layer Setting API
                const uint32_t frameset_count =
                    static_cast<uint32_t>(api_setting->valueCount / (sizeof(VkuFrameset) / sizeof(VkuFrameset::count)));
                if (api_setting->type != VK_LAYER_SETTING_TYPE_UINT32_EXT) {
                    result = VK_ERROR_FORMAT_NOT_SUPPORTED;
                } else if (copy_values) {
                    if (*pValueCount < frameset_count) {
                        result = VK_INCOMPLETE;
                    }
                    const std::uint32_t count = std::min(*pValueCount, frameset_count);
                    const VkuFrameset *data = static_cast<const VkuFrameset *>(api_setting->pValues);
                    values.assign(data, data + count);
                } else {
                    *pValueCount = frameset_count;
                }
            }

            if (copy_values) {
                std::copy(values.begin(), values.end(), reinterpret_cast<VkuFrameset *>(pValues));
            }

            return result;
        }
        case VKU_LAYER_SETTING_TYPE_STRING: {
            VkResult result = VK_SUCCESS;

            std::vector<std::string> &settings_cache = layer_setting_set->GetSettingCache(pSettingName);

            if (!settings.empty()) {  // From env variable or setting file
                settings_cache = settings;

                if (copy_values) {
                    if (static_cast<std::size_t>(*pValueCount) < settings_cache.size()) {
                        result = VK_INCOMPLETE;
                    }
                } else {
                    *pValueCount = static_cast<std::uint32_t>(settings_cache.size());
                }
            } else if (api_setting != nullptr) {  // From Vulkan Layer Setting API
                if (copy_values) {
                    if (*pValueCount < api_setting->valueCount) {
                        result = VK_INCOMPLETE;
                    }
                    const std::uint32_t size = std::min(*pValueCount, api_setting->valueCount);
                    settings_cache.resize(size);

                    switch (api_setting->type) {
                        case VK_LAYER_SETTING_TYPE_STRING_EXT:
                            for (std::size_t i = 0, n = settings_cache.size(); i < n; ++i) {
                                settings_cache[i] = reinterpret_cast<const char *const *>(api_setting->pValues)[i];
                            }
                            break;
                        case VK_LAYER_SETTING_TYPE_BOOL32_EXT:
                            for (std::size_t i = 0, n = settings_cache.size(); i < n; ++i) {
                                settings_cache[i] =
                                    static_cast<const VkBool32 *>(api_setting->pValues)[i] == VK_TRUE ? "true" : "false";
                            }
                            break;
                        case VK_LAYER_SETTING_TYPE_INT32_EXT:
                            for (std::size_t i = 0, n = settings_cache.size(); i < n; ++i) {
                                settings_cache[i] = vl::FormatString("%d", static_cast<const int32_t *>(api_setting->pValues)[i]);
                            }
                            break;
                        case VK_LAYER_SETTING_TYPE_INT64_EXT:
                            for (std::size_t i = 0, n = settings_cache.size(); i < n; ++i) {
                                settings_cache[i] = vl::FormatString("%lld", static_cast<const int64_t *>(api_setting->pValues)[i]);
                            }
                            break;
                        case VK_LAYER_SETTING_TYPE_UINT32_EXT:
                            for (std::size_t i = 0, n = settings_cache.size(); i < n; ++i) {
                                settings_cache[i] = vl::FormatString("%u", static_cast<const uint32_t *>(api_setting->pValues)[i]);
                            }
                            break;
                        case VK_LAYER_SETTING_TYPE_UINT64_EXT:
                            for (std::size_t i = 0, n = settings_cache.size(); i < n; ++i) {
                                settings_cache[i] =
                                    vl::FormatString("%llu", static_cast<const uint64_t *>(api_setting->pValues)[i]);
                            }
                            break;
                        case VK_LAYER_SETTING_TYPE_FLOAT32_EXT:
                            for (std::size_t i = 0, n = settings_cache.size(); i < n; ++i) {
                                settings_cache[i] = vl::FormatString("%f", static_cast<const float *>(api_setting->pValues)[i]);
                            }
                            break;
                        case VK_LAYER_SETTING_TYPE_FLOAT64_EXT:
                            for (std::size_t i = 0, n = settings_cache.size(); i < n; ++i) {
                                settings_cache[i] = vl::FormatString("%f", static_cast<const double *>(api_setting->pValues)[i]);
                            }
                            break;
                        default:
                            result = VK_ERROR_FORMAT_NOT_SUPPORTED;
                            break;
                    }
                } else {
                    *pValueCount = api_setting->valueCount;
                }
            }

            if (copy_values) {
                for (std::size_t i = 0, n = std::min(static_cast<std::size_t>(*pValueCount), settings_cache.size()); i < n; ++i) {
                    reinterpret_cast<const char **>(pValues)[i] = settings_cache[i].c_str();
                }
            }

            return result;
        }
        case VKU_LAYER_SETTING_TYPE_FRAMESET_STRING: {
            VkResult result = VK_SUCCESS;

            std::vector<std::string> &settings_cache = layer_setting_set->GetSettingCache(pSettingName);

            if (!settings.empty()) {  // From env variable or setting file
                settings_cache = settings;

                if (copy_values) {
                    if (static_cast<std::size_t>(*pValueCount) < settings_cache.size()) {
                        result = VK_INCOMPLETE;
                    }
                } else {
                    *pValueCount = static_cast<std::uint32_t>(settings_cache.size());
                }
            } else if (api_setting != nullptr) {  // From Vulkan Layer Setting API
                const std::uint32_t frameset_count =
                    static_cast<uint32_t>(api_setting->valueCount / (sizeof(VkuFrameset) / sizeof(VkuFrameset::count)));

                if (copy_values) {
                    if (*pValueCount < frameset_count) {
                        result = VK_INCOMPLETE;
                    }
                    const std::uint32_t size = std::min(*pValueCount, frameset_count);
                    settings_cache.resize(size);

                    switch (api_setting->type) {
                        case VK_LAYER_SETTING_TYPE_STRING_EXT:
                            for (std::size_t i = 0, n = settings_cache.size(); i < n; ++i) {
                                settings_cache[i] = reinterpret_cast<const char *const *>(api_setting->pValues)[i];
                            }
                            break;
                        case VK_LAYER_SETTING_TYPE_UINT32_EXT:
                            for (std::size_t i = 0, n = settings_cache.size(); i < n; ++i) {
                                const VkuFrameset *asFramesets = static_cast<const VkuFrameset *>(api_setting->pValues);
                                settings_cache[i] =
                                    vl::FormatString("%d-%d-%d", asFramesets[i].first, asFramesets[i].count, asFramesets[i].step);
                            }
                            break;
                        default:
                            result = VK_ERROR_FORMAT_NOT_SUPPORTED;
                            break;
                    }
                } else {
                    *pValueCount = frameset_count;
                }
            }

            if (copy_values) {
                for (std::size_t i = 0, n = std::min(static_cast<std::size_t>(*pValueCount), settings_cache.size()); i < n; ++i) {
                    reinterpret_cast<const char **>(pValues)[i] = settings_cache[i].c_str();
                }
            }

            return result;
        }
    }
}

const VkLayerSettingsCreateInfoEXT *vkuFindLayerSettingsCreateInfo(const VkInstanceCreateInfo *pCreateInfo) {
    const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(pCreateInfo);
    const VkLayerSettingsCreateInfoEXT *found = nullptr;
    while (current) {
        if (VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT == current->sType) {
            found = reinterpret_cast<const VkLayerSettingsCreateInfoEXT *>(current);
            current = nullptr;
        } else {
            current = current->pNext;
        }
    }
    return found;
}

const VkLayerSettingsCreateInfoEXT *vkuNextLayerSettingsCreateInfo(const VkLayerSettingsCreateInfoEXT *pCreateInfo) {
    const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(pCreateInfo->pNext);
    const VkLayerSettingsCreateInfoEXT *found = nullptr;
    while (current) {
        if (VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT == current->sType) {
            found = reinterpret_cast<const VkLayerSettingsCreateInfoEXT *>(current);
            current = nullptr;
        } else {
            current = current->pNext;
        }
    }
    return found;
}

static bool vkuHasSetting(uint32_t settingsCount, const char **pSettings, const char *searchedSettings) {
    for (uint32_t setting_index = 0; setting_index < settingsCount; ++setting_index) {
        if (std::strcmp(pSettings[setting_index], searchedSettings) == 0) {
            return true;
        }
    }

    return false;
}

VkResult vkuGetUnknownSettings(const VkLayerSettingsCreateInfoEXT *pCreateInfo, uint32_t settingsCount, const char **pSettings,
                               uint32_t *pUnknownSettingCount, const char **pUnknownSettings) {
    assert(pUnknownSettingCount != nullptr);

    const VkLayerSettingsCreateInfoEXT *current_create_info = pCreateInfo;

    uint32_t current_unknown_setting_count = 0;
    while (current_create_info != nullptr) {
        for (uint32_t info_index = 0, info_count = current_create_info->settingCount; info_index < info_count; ++info_index) {
            const char *current_setting_name = current_create_info->pSettings[info_index].pSettingName;
            if (!vkuHasSetting(settingsCount, pSettings, current_setting_name)) {
                if (pUnknownSettings != nullptr && current_unknown_setting_count < *pUnknownSettingCount) {
                    pUnknownSettings[current_unknown_setting_count] = current_setting_name;
                }

                ++current_unknown_setting_count;
            }
        }

        current_create_info = vkuNextLayerSettingsCreateInfo(current_create_info);
    }

    if (pUnknownSettings == nullptr) {
        *pUnknownSettingCount = current_unknown_setting_count;
        return VK_SUCCESS;
    } else if (current_unknown_setting_count > *pUnknownSettingCount) {
        return VK_INCOMPLETE;
    }

    return VK_SUCCESS;
}
