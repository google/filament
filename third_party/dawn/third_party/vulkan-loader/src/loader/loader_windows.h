/*
 *
 * Copyright (c) 2014-2022 The Khronos Group Inc.
 * Copyright (c) 2014-2022 Valve Corporation
 * Copyright (c) 2014-2022 LunarG, Inc.
 * Copyright (C) 2015 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chia-I Wu <olv@lunarg.com>
 * Author: Mark Lobodzinski <mark@LunarG.com>
 * Author: Lenny Komow <lenny@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 *
 */

#pragma once

#if defined(_WIN32)

#include "loader_common.h"

#include <minwindef.h>
#include <cfgmgr32.h>
#include <sdkddkver.h>

// Windows specific initialization functionality
void windows_initialization(void);

// Append the JSON path data to the list and allocate/grow the list if it's not large enough.
// Function returns true if filename was appended to reg_data list.
// Caller should free reg_data.
bool windows_add_json_entry(const struct loader_instance *inst,
                            char **reg_data,    // list of JSON files
                            PDWORD total_size,  // size of reg_data
                            LPCSTR key_name,    // key name - used for debug prints - i.e. VulkanDriverName
                            DWORD key_type,     // key data type
                            LPSTR json_path,    // JSON string to add to the list reg_data
                            DWORD json_size,    // size in bytes of json_path
                            VkResult *result);

// Find the list of registry files (names VulkanDriverName/VulkanDriverNameWow) in hkr.
//
// This function looks for filename in given device handle, filename is then added to return list
// function return true if filename was appended to reg_data list
// If error occurs result is updated with failure reason
bool windows_get_device_registry_entry(const struct loader_instance *inst, char **reg_data, PDWORD total_size, DEVINST dev_id,
                                       LPCSTR value_name, VkResult *result);

// Find the list of registry files (names VulkanDriverName/VulkanDriverNameWow) in hkr .
//
// This function looks for display devices and childish software components
// for a list of files which are added to a returned list (function return
// value).
// Function return is a string with a ';'  separated list of filenames.
// Function return is NULL if no valid name/value pairs  are found in the key,
// or the key is not found.
//
// *reg_data contains a string list of filenames as pointer.
// When done using the returned string list, the caller should free the pointer.
VkResult windows_get_device_registry_files(const struct loader_instance *inst, uint32_t log_target_flag, char **reg_data,
                                           PDWORD reg_data_size, LPCSTR value_name);

// Find the list of registry files (names within a key) in key "location".
//
// This function looks in the registry (hive = DEFAULT_VK_REGISTRY_HIVE) key as
// given in "location"
// for a list or name/values which are added to a returned list (function return
// value).
// The DWORD values within the key must be 0 or they are skipped.
// Function return is a string with a ';'  separated list of filenames.
// Function return is NULL if no valid name/value pairs  are found in the key,
// or the key is not found.
//
// *reg_data contains a string list of filenames as pointer.
// When done using the returned string list, the caller should free the pointer.
VkResult windows_get_registry_files(const struct loader_instance *inst, char *location, bool use_secondary_hive, char **reg_data,
                                    PDWORD reg_data_size);

// Read manifest JSON files using the Windows driver interface
VkResult windows_read_manifest_from_d3d_adapters(const struct loader_instance *inst, char **reg_data, PDWORD reg_data_size,
                                                 const wchar_t *value_name);

// Look for data files in the registry.
VkResult windows_read_data_files_in_registry(const struct loader_instance *inst, enum loader_data_files_type data_file_type,
                                             bool warn_if_not_present, char *registry_location,
                                             struct loader_string_list *out_files);

// This function allocates an array in sorted_devices which must be freed by the caller if not null
VkResult windows_read_sorted_physical_devices(struct loader_instance *inst, uint32_t *icd_phys_devs_array_count,
                                              struct loader_icd_physical_devices **icd_phys_devs_array);

// This function sorts an array in physical device groups based on the sorted physical device information
VkResult windows_sort_physical_device_groups(struct loader_instance *inst, const uint32_t group_count,
                                             struct loader_physical_device_group_term *sorted_group_term,
                                             const uint32_t sorted_device_count,
                                             struct loader_icd_physical_devices *sorted_phys_dev_array);

// Creates a DXGI factory
// Returns VkLoaderFeatureFlags containing VK_LOADER_FEATURE_PHYSICAL_DEVICE_SORTING if successful, otherwise 0
VkLoaderFeatureFlags windows_initialize_dxgi(void);

// Retrieve a path to an installed app package that contains Vulkan manifests.
// When done using the returned string, the caller should free the pointer.
char *windows_get_app_package_manifest_path(const struct loader_instance *inst);

// Gets the path to the loader settings file, if it exists. If it doesn't exists, writes NULL to out_path.
// The path is located through the registry as an entry in HKEY_CURRENT_USER/SOFTWARE/Khronos/Vulkan/LoaderSettings
// and if nothing is there, will try to look in HKEY_LOCAL_MACHINE/SOFTWARE/Khronos/Vulkan/LoaderSettings.
// If running with elevated privileges, this function only looks in HKEY_LOCAL_MACHINE.
VkResult windows_get_loader_settings_file_path(const struct loader_instance *inst, char **out_path);
#endif  // WIN32
