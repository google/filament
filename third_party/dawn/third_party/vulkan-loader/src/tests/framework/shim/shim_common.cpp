/*
 * Copyright (c) 2021 The Khronos Group Inc.
 * Copyright (c) 2021 Valve Corporation
 * Copyright (c) 2021 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "shim.h"

#include "env_var_wrapper.h"

std::vector<std::string> parse_env_var_list(std::string const& var) {
    std::vector<std::string> items;
    size_t start = 0;
    size_t len = 0;
    for (size_t i = 0; i < var.size(); i++) {
#if defined(_WIN32)
        if (var[i] == ';') {
#elif TESTING_COMMON_UNIX_PLATFORMS
        if (var[i] == ':') {
#endif
            if (len != 0) {
                // only push back non empty strings
                items.push_back(var.substr(start, len));
            }
            start = i + 1;
            len = 0;
        } else {
            len++;
        }
    }
    items.push_back(var.substr(start, len));

    return items;
}

#if defined(_WIN32)

D3DKMT_Adapter& D3DKMT_Adapter::add_driver_manifest_path(std::filesystem::path const& src) { return add_path(src, driver_paths); }
D3DKMT_Adapter& D3DKMT_Adapter::add_implicit_layer_manifest_path(std::filesystem::path const& src) {
    return add_path(src, implicit_layer_paths);
}
D3DKMT_Adapter& D3DKMT_Adapter::add_explicit_layer_manifest_path(std::filesystem::path const& src) {
    return add_path(src, explicit_layer_paths);
}

D3DKMT_Adapter& D3DKMT_Adapter::add_path(std::filesystem::path src, std::vector<std::wstring>& dest) {
    dest.push_back(src.native());
    return *this;
}

std::string category_path_name(ManifestCategory category) {
    if (category == ManifestCategory::implicit_layer) return "ImplicitLayers";
    if (category == ManifestCategory::explicit_layer)
        return "ExplicitLayers";
    else
        return "Drivers";
}

void PlatformShim::reset() {
    hkey_current_user_explicit_layers.clear();
    hkey_current_user_implicit_layers.clear();
    hkey_local_machine_explicit_layers.clear();
    hkey_local_machine_implicit_layers.clear();
    hkey_local_machine_drivers.clear();
    hkey_local_machine_settings.clear();
    hkey_current_user_settings.clear();
}

void PlatformShim::add_manifest_to_registry(ManifestCategory category, std::filesystem::path const& path) {
    if (category == ManifestCategory::settings) {
        hkey_local_machine_settings.emplace_back(path);
    } else if (category == ManifestCategory::implicit_layer) {
        hkey_local_machine_implicit_layers.emplace_back(path);
    } else if (category == ManifestCategory::explicit_layer) {
        hkey_local_machine_explicit_layers.emplace_back(path);
    } else {
        hkey_local_machine_drivers.emplace_back(path);
    }
}

void PlatformShim::add_unsecured_manifest_to_registry(ManifestCategory category, std::filesystem::path const& path) {
    if (category == ManifestCategory::settings) {
        hkey_current_user_settings.emplace_back(path);
    } else if (category == ManifestCategory::implicit_layer) {
        hkey_current_user_implicit_layers.emplace_back(path);
    } else if (category == ManifestCategory::explicit_layer) {
        hkey_current_user_explicit_layers.emplace_back(path);
    }
}

void PlatformShim::add_dxgi_adapter(GpuType gpu_preference, DXGI_ADAPTER_DESC1 desc1) {
    uint32_t next_index = static_cast<uint32_t>(dxgi_adapters.size());
    dxgi_adapters.emplace(next_index, DXGIAdapter{gpu_preference, desc1, next_index});
}

void PlatformShim::add_d3dkmt_adapter(D3DKMT_Adapter const& adapter) { d3dkmt_adapters.push_back(adapter); }

void PlatformShim::set_app_package_path(std::filesystem::path const& path) { app_package_path = path; }

// TODO:
void PlatformShim::add_CM_Device_ID([[maybe_unused]] std::wstring const& id, [[maybe_unused]] std::filesystem::path const& icd_path,
                                    [[maybe_unused]] std::filesystem::path const& layer_path) {
    //     // append a null byte as separator if there is already id's in the list
    //     if (CM_device_ID_list.size() != 0) {
    //         CM_device_ID_list += L'\0';  // I'm sure this wont cause issues with std::string down the line... /s
    //     }
    //     CM_device_ID_list += id;
    //     std::string id_str(id.length(), '\0');
    //     size_t size_written{};
    //     wcstombs_s(&size_written, &id_str[0], id_str.length(), id.c_str(), id.length());

    //     std::string device_path = std::string(pnp_registry_path) + "\\" + id_str;
    //     CM_device_ID_registry_keys.push_back(device_path.c_str());
    //     add_key_value_string(id_key, "VulkanDriverName", icd_path.c_str());
    //     add_key_value_string(id_key, "VulkanLayerName", layer_path.c_str());
    //     // TODO: decide how to handle 32 bit
    //     // add_key_value_string(id_key, "VulkanDriverNameWoW", icd_path.c_str());
    //     // add_key_value_string(id_key, "VulkanLayerName", layer_path.c_str());
}

#elif TESTING_COMMON_UNIX_PLATFORMS

#include <dirent.h>
#include <unistd.h>

std::string category_path_name(ManifestCategory category) {
    if (category == ManifestCategory::settings) return "settings.d";
    if (category == ManifestCategory::implicit_layer) return "implicit_layer.d";
    if (category == ManifestCategory::explicit_layer)
        return "explicit_layer.d";
    else
        return "icd.d";
}

void PlatformShim::add_system_library(std::string const& filename, std::filesystem::path const& actual_path) {
    system_library_redirection_map[filename] = actual_path;
}

std::filesystem::path PlatformShim::get_system_library(std::string const& filename) {
    return system_library_redirection_map.count(filename) == 1 ? system_library_redirection_map.at(filename)
                                                               : std::filesystem::path{};
}
#endif
