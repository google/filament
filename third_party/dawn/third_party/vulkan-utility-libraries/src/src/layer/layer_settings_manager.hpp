// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//
// Author(s):
// - Christophe Riccio <christophe@lunarg.com>
#pragma once

#include "vulkan/layer/vk_layer_settings.h"

#include <string>
#include <vector>
#include <map>
#include <filesystem>

namespace vl {
class LayerSettings {
  public:
    LayerSettings(const char *pLayerName, const VkLayerSettingsCreateInfoEXT *pFirstCreateInfo,
                  const VkAllocationCallbacks *pAllocator, VkuLayerSettingLogCallback pCallback);
    ~LayerSettings();

    void SetPrefix(const char *pPrefix) { this->prefix = pPrefix; }

    bool HasEnvSetting(const char *pSettingName);

    bool HasFileSetting(const char *pSettingName);

    bool HasAPISetting(const char *pSettingName);

    std::string GetEnvSetting(const char *pSettingName);

    std::string GetFileSetting(const char *pSettingName);

    void SetFileSetting(const char *pSettingName, const std::string &pValues);

    const VkLayerSettingEXT *GetAPISetting(const char *pSettingName);

    void Log(const char *pSettingName, const char *pMessage);

    std::vector<std::string> &GetSettingCache(const std::string &pSettingName);

  private:
    const VkLayerSettingEXT *FindLayerSettingValue(const char *pSettingName);

    std::map<std::string, std::string> setting_file_values;
    std::map<std::string, std::vector<std::string>> string_setting_cache;

    std::string last_log_setting;
    std::string last_log_message;

    std::filesystem::path FindSettingsFile();
    void ParseSettingsFile(const std::filesystem::path &filename);

    std::string prefix;
    std::string layer_name;

    const VkLayerSettingsCreateInfoEXT *first_create_info{nullptr};
    VkuLayerSettingLogCallback pCallback{nullptr};
};
}  // namespace vl
