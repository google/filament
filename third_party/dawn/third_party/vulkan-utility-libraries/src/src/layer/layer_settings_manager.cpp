// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//
// Author(s):
// - Christophe Riccio <christophe@lunarg.com>
#include "layer_settings_manager.hpp"
#include "layer_settings_util.hpp"

#include <sys/stat.h>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <array>
#include <algorithm>

#if defined(__ANDROID__)
/*
 * Use the __system_property_read_callback API that appeared in
 * Android API level 26. If not avaible use the old __system_property_get function.
 */

// Weak function declaration, used only when not decalered in the Android headers
void __system_property_read_callback(const prop_info *info,
                                     void (*callback)(void *cookie, const char *name, const char *value, uint32_t serial),
                                     void *cookie) __attribute__((weak));
static std::string GetAndroidProperty(const char *name) {
    std::string output;
    if (__system_property_read_callback != nullptr) {
        const prop_info *pi = __system_property_find(name);
        if (pi) {
            __system_property_read_callback(
                pi,
                [](void *cookie, const char *name, const char *value, uint32_t serial) {
                    (void)name;
                    (void)serial;
                    reinterpret_cast<std::string *>(cookie)->assign(value);
                },
                reinterpret_cast<void *>(&output));
        }
    } else {
        char value_buf[PROP_VALUE_MAX] = "";
        size_t len = static_cast<size_t>(__system_property_get(name, value_buf));
        if (len > 0 && len < sizeof(value_buf)) {
            output.assign(value_buf, len);
        }
    }
    return output;
}
#endif

static std::string GetEnvironment(const char *variable) {
#if defined(__ANDROID__)
    std::string result = GetAndroidProperty(variable);
    // Workaround for screenshot layer backward compatibility
    if (result.empty() && std::string(variable) == "debug.vulkan.screenshot.frames") {
        result = GetAndroidProperty("debug.vulkan.screenshot");
    }
    return result;
#elif defined(_WIN32)
    auto size = GetEnvironmentVariableA(variable, NULL, 0);
    if (size == 0) {
        return "";
    }
    char *buffer = new char[size];
    GetEnvironmentVariableA(variable, buffer, size);
    std::string output = buffer;
    delete[] buffer;
    return output;
#else
    const char *output = std::getenv(variable);
    return output == nullptr ? "" : output;
#endif
}

#if defined(WIN32)
// Check for admin rights
static inline bool IsHighIntegrity() {
    HANDLE process_token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, &process_token)) {
        // Maximum possible size of SID_AND_ATTRIBUTES is maximum size of a SID + size of attributes DWORD.
        uint8_t mandatory_label_buffer[SECURITY_MAX_SID_SIZE + sizeof(DWORD)];
        DWORD buffer_size;
        if (GetTokenInformation(process_token, TokenIntegrityLevel, mandatory_label_buffer, sizeof(mandatory_label_buffer),
                                &buffer_size) != 0) {
            const TOKEN_MANDATORY_LABEL *mandatory_label = (const TOKEN_MANDATORY_LABEL *)mandatory_label_buffer;
            const DWORD sub_authority_count = *GetSidSubAuthorityCount(mandatory_label->Label.Sid);
            const DWORD integrity_level = *GetSidSubAuthority(mandatory_label->Label.Sid, sub_authority_count - 1);

            CloseHandle(process_token);
            return integrity_level > SECURITY_MANDATORY_MEDIUM_RID;
        }

        CloseHandle(process_token);
    }

    return false;
}
#endif

// To prevent exposing an interface that would make it easy to use inconsistent setting naming,
// we hide here workaround of existing layers to preserve backward compatibility
static void AddWorkaroundLayerNames(std::vector<std::string> &layer_names) {
    if (std::find(layer_names.begin(), layer_names.end(), "VK_LAYER_KHRONOS_synchronization2") != layer_names.end()) {
        layer_names.push_back("VK_LAYER_KHRONOS_sync2");
        return;
    }
}

namespace vl {

LayerSettings::LayerSettings(const char *pLayerName, const VkLayerSettingsCreateInfoEXT *pFirstCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkuLayerSettingLogCallback pCallback)
    : layer_name(pLayerName), first_create_info(pFirstCreateInfo), pCallback(pCallback) {
    (void)pAllocator;
    assert(pLayerName != nullptr);

    std::filesystem::path settings_file = this->FindSettingsFile();
    this->ParseSettingsFile(settings_file);
}

LayerSettings::~LayerSettings() {}

void LayerSettings::ParseSettingsFile(const std::filesystem::path &filename) {
    // Extract option = value pairs from a file
    std::ifstream file(filename);
    if (file.good()) {
        for (std::string line; std::getline(file, line);) {
            // discard comments, which start with '#'
            const auto comments_pos = line.find_first_of('#');
            if (comments_pos != std::string::npos) line.erase(comments_pos);

            const auto value_pos = line.find_first_of('=');
            if (value_pos != std::string::npos) {
                const std::string setting_key = vl::TrimWhitespace(line.substr(0, value_pos));
                const std::string setting_value = vl::TrimWhitespace(line.substr(value_pos + 1));
                this->setting_file_values[setting_key] = setting_value;
            }
        }
    }
}

std::filesystem::path LayerSettings::FindSettingsFile() {
    struct stat info;

#if defined(WIN32)
    // Look for VkConfig-specific settings location specified in the windows registry
    HKEY key;

    const std::array<HKEY, 2> hives = {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER};
    const size_t hives_to_check_count = IsHighIntegrity() ? 1 : hives.size();  // Admin checks only the default hive

    for (size_t hive_index = 0; hive_index < hives_to_check_count; ++hive_index) {
        LSTATUS err = RegOpenKeyEx(hives[hive_index], TEXT("Software\\Khronos\\Vulkan\\Settings"), 0, KEY_READ, &key);
        if (err == ERROR_SUCCESS) {
            TCHAR name[2048];
            DWORD i = 0, name_size, type, pValues, value_size;
            while (ERROR_SUCCESS == RegEnumValue(key, i++, name, &(name_size = sizeof(name)), nullptr, &type,
                                                 reinterpret_cast<LPBYTE>(&pValues), &(value_size = sizeof(pValues)))) {
                // Check if the registry entry is a dword with a value of zero
                if (type != REG_DWORD || pValues != 0) {
                    continue;
                }

                // Check if this actually points to a file
                DWORD fileAttrib = GetFileAttributes(name);
                if ((fileAttrib == INVALID_FILE_ATTRIBUTES) || (fileAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
                    continue;
                }

                // Use this file
                RegCloseKey(key);
                return name;
            }

            RegCloseKey(key);
        }
    }
#else
    // Look for VkConfig-specific settings location specified in a specific spot in the linux settings store
    std::string search_path = GetEnvironment("XDG_DATA_HOME");
    if (search_path == "") {
        search_path = GetEnvironment("HOME");
        if (search_path != "") {
            search_path += "/.local/share";
        }
    }
    // Use the vk_layer_settings.txt file from here, if it is present
    if (search_path != "") {
        std::string home_file = search_path + "/vulkan/settings.d/vk_layer_settings.txt";
        if (stat(home_file.c_str(), &info) == 0) {
            if (info.st_mode & S_IFREG) {
                return home_file;
            }
        }
    }
#endif

#ifdef __ANDROID__
    std::string env_path = GetEnvironment("debug.vulkan.khronos_profiles.settings_path");
#else
    // Look for an environment variable override for the settings file location
    std::string env_path = GetEnvironment("VK_LAYER_SETTINGS_PATH");
#endif

    // If the path exists use it, else use vk_layer_settings
    if (stat(env_path.c_str(), &info) == 0) {
        // If this is a directory, append settings file name
        if (info.st_mode & S_IFDIR) {
            env_path.append("/vk_layer_settings.txt");
        }
        return env_path;
    }

    // Default -- use the current working directory for the settings file location
    char buff[512];
    auto buf_ptr = GetCurrentDir(buff, 512);
    if (buf_ptr) {
        std::string location = buf_ptr;
        location.append("/vk_layer_settings.txt");
        return location;
    }
    return "vk_layer_settings.txt";
}

const VkLayerSettingEXT *LayerSettings::FindLayerSettingValue(const char *pSettingName) {
    if (this->first_create_info == nullptr) {
        return nullptr;
    }

    const std::string setting_name(pSettingName);

    const VkLayerSettingsCreateInfoEXT *current_create_info = this->first_create_info;

    while (current_create_info != nullptr) {
        for (std::size_t i = 0, n = current_create_info->settingCount; i < n; ++i) {
            const VkLayerSettingEXT *setting = &current_create_info->pSettings[i];
            if (setting->pLayerName != this->layer_name) {
                continue;
            }

            if (setting->pSettingName != setting_name) {
                continue;
            }

            return setting;
        }

        current_create_info = vkuNextLayerSettingsCreateInfo(current_create_info);
    }

    return nullptr;
}

void LayerSettings::Log(const char *pSettingName, const char *pMessage) {
    this->last_log_setting = pSettingName;
    this->last_log_message = pMessage;

    if (this->pCallback == nullptr) {
        fprintf(stderr, "LAYER SETTING (%s) error: %s\n", this->last_log_setting.c_str(), this->last_log_message.c_str());
    } else {
        this->pCallback(this->last_log_setting.c_str(), this->last_log_message.c_str());
    }
}

std::vector<std::string> &LayerSettings::GetSettingCache(const std::string &settingName) {
    if (this->string_setting_cache.find(settingName) != this->string_setting_cache.end()) {
        this->string_setting_cache.insert(
            std::pair<std::string, std::vector<std::string>>(settingName, std::vector<std::string>()));
    }

    return this->string_setting_cache[settingName];
}

bool LayerSettings::HasEnvSetting(const char *pSettingName) {
    assert(pSettingName != nullptr);

    return !GetEnvSetting(pSettingName).empty();
}

bool LayerSettings::HasFileSetting(const char *pSettingName) {
    assert(pSettingName != nullptr);

    std::string file_setting_name = vl::GetFileSettingName(this->layer_name.c_str(), pSettingName);

    return setting_file_values.find(file_setting_name) != setting_file_values.end();
}

bool LayerSettings::HasAPISetting(const char *pSettingName) {
    assert(pSettingName != nullptr);

    return this->FindLayerSettingValue(pSettingName) != nullptr;
}

std::string LayerSettings::GetEnvSetting(const char *pSettingName) {
    std::vector<std::string> layer_names;
    layer_names.push_back(this->layer_name);
    ::AddWorkaroundLayerNames(layer_names);

    for (std::size_t layer_index = 0, layer_count = layer_names.size(); layer_index < layer_count; ++layer_index) {
        const char *cur_layer_name = layer_names[layer_index].c_str();

        if (!this->prefix.empty()) {
            const std::string &env_name = GetEnvSettingName(cur_layer_name, this->prefix.c_str(), pSettingName, TRIM_NAMESPACE);
            std::string result = GetEnvironment(env_name.c_str());
            if (!result.empty()) {
                return result;
            }
        }

        for (int trim_index = TRIM_FIRST; trim_index <= TRIM_LAST; ++trim_index) {
            const std::string &env_name =
                GetEnvSettingName(cur_layer_name, this->prefix.c_str(), pSettingName, static_cast<TrimMode>(trim_index));
            std::string result = GetEnvironment(env_name.c_str());
            if (!result.empty()) {
                return result;
            }
        }
    }

    return std::string();
}

std::string LayerSettings::GetFileSetting(const char *pSettingName) {
    const std::string file_setting_name = vl::GetFileSettingName(this->layer_name.c_str(), pSettingName);

    std::map<std::string, std::string>::const_iterator it;
    if ((it = this->setting_file_values.find(file_setting_name)) == this->setting_file_values.end()) {
        return "";
    } else {
        return it->second;
    }
}

void LayerSettings::SetFileSetting(const char *pSettingName, const std::string &pValues) {
    assert(pSettingName != nullptr);

    this->setting_file_values.insert({pSettingName, pValues});
}

const VkLayerSettingEXT *LayerSettings::GetAPISetting(const char *pSettingName) {
    assert(pSettingName != nullptr);

    return reinterpret_cast<const VkLayerSettingEXT *>(this->FindLayerSettingValue(pSettingName));
}

}  // namespace vl
