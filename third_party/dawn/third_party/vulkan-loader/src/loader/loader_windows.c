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
// Windows only header file, guard it so that accidental inclusion doesn't cause unknown header include errors
#if defined(_WIN32)

// This needs to be defined first, or else we'll get redefinitions on NTSTATUS values
#define UMDF_USING_NTSTATUS
#include <ntstatus.h>

#include "loader_windows.h"

#include "allocation.h"
#include "loader_environment.h"
#include "loader.h"
#include "log.h"

#include <cfgmgr32.h>
#include <initguid.h>
#include <devpkey.h>
#include <winternl.h>
#include <strsafe.h>
#if defined(__MINGW32__)
#undef strcpy  // fix error with redefined strcpy when building with MinGW-w64
#endif
#include <dxgi1_6.h>
#include "adapters.h"

#if !defined(__MINGW32__)
// not yet available with MinGW-w64 stable
#include <appmodel.h>
#endif

#if !defined(NDEBUG)
#include <crtdbg.h>
#endif

typedef HRESULT(APIENTRY *PFN_CreateDXGIFactory1)(REFIID riid, void **ppFactory);
PFN_CreateDXGIFactory1 fpCreateDXGIFactory1;

// Empty function just so windows_initialization can find the current module location
void function_for_finding_the_current_module(void) {}

void windows_initialization(void) {
    char dll_location[MAX_PATH];
    HMODULE module_handle = NULL;

    // Get a module handle to a static function inside of this source
    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&function_for_finding_the_current_module, &module_handle) != 0 &&
        GetModuleFileName(module_handle, dll_location, sizeof(dll_location)) != 0) {
        loader_log(NULL, VULKAN_LOADER_INFO_BIT, 0, "Using Vulkan Loader %s", dll_location);
    }

    // This is needed to ensure that newer APIs are available right away
    // and not after the first call that has been statically linked
    LoadLibraryEx("gdi32.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);

    wchar_t systemPath[MAX_PATH] = L"";
    GetSystemDirectoryW(systemPath, MAX_PATH);
    StringCchCatW(systemPath, MAX_PATH, L"\\dxgi.dll");
    HMODULE dxgi_module = LoadLibraryW(systemPath);
    fpCreateDXGIFactory1 =
        dxgi_module == NULL ? NULL : (PFN_CreateDXGIFactory1)(void *)GetProcAddress(dxgi_module, "CreateDXGIFactory1");

#if !defined(NDEBUG)
    _set_error_mode(_OUT_TO_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
    (void)hinst;
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            // Only initialize necessary sync primitives
            loader_platform_thread_create_mutex(&loader_lock);
            loader_platform_thread_create_mutex(&loader_preload_icd_lock);
            loader_platform_thread_create_mutex(&loader_global_instance_list_lock);
            init_global_loader_settings();
            break;
        case DLL_PROCESS_DETACH:
            if (NULL == reserved) {
                loader_release();
            }
            break;
        default:
            // Do nothing
            break;
    }
    return TRUE;
}

bool windows_add_json_entry(const struct loader_instance *inst,
                            char **reg_data,    // list of JSON files
                            PDWORD total_size,  // size of reg_data
                            LPCSTR key_name,    // key name - used for debug prints - i.e. VulkanDriverName
                            DWORD key_type,     // key data type
                            LPSTR json_path,    // JSON string to add to the list reg_data
                            DWORD json_size,    // size in bytes of json_path
                            VkResult *result) {
    // Check for and ignore duplicates.
    if (*reg_data && strstr(*reg_data, json_path)) {
        // Success. The json_path is already in the list.
        return true;
    }

    if (NULL == *reg_data) {
        *reg_data = loader_instance_heap_alloc(inst, *total_size, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
        if (NULL == *reg_data) {
            loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                       "windows_add_json_entry: Failed to allocate space for registry data for key %s", json_path);
            *result = VK_ERROR_OUT_OF_HOST_MEMORY;
            return false;
        }
        *reg_data[0] = '\0';
    } else if (strlen(*reg_data) + json_size + 1 > *total_size) {
        void *new_ptr =
            loader_instance_heap_realloc(inst, *reg_data, *total_size, *total_size * 2, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
        if (NULL == new_ptr) {
            loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                       "windows_add_json_entry: Failed to reallocate space for registry value of size %ld for key %s",
                       *total_size * 2, json_path);
            *result = VK_ERROR_OUT_OF_HOST_MEMORY;
            return false;
        }
        *reg_data = new_ptr;
        *total_size *= 2;
    }

    for (char *curr_filename = json_path; curr_filename[0] != '\0'; curr_filename += strlen(curr_filename) + 1) {
        if (strlen(*reg_data) == 0) {
            (void)snprintf(*reg_data, json_size + 1, "%s", curr_filename);
        } else {
            (void)snprintf(*reg_data + strlen(*reg_data), json_size + 2, "%c%s", PATH_SEPARATOR, curr_filename);
        }
        loader_log(inst, VULKAN_LOADER_INFO_BIT, 0, "%s: Located json file \"%s\" from PnP registry: %s", __FUNCTION__,
                   curr_filename, key_name);

        if (key_type == REG_SZ) {
            break;
        }
    }
    return true;
}

bool windows_get_device_registry_entry(const struct loader_instance *inst, char **reg_data, PDWORD total_size, DEVINST dev_id,
                                       LPCSTR value_name, VkResult *result) {
    HKEY hkrKey = INVALID_HANDLE_VALUE;
    DWORD requiredSize, data_type;
    char *manifest_path = NULL;
    bool found = false;

    assert(reg_data != NULL && "windows_get_device_registry_entry: reg_data is a NULL pointer");
    assert(total_size != NULL && "windows_get_device_registry_entry: total_size is a NULL pointer");

    CONFIGRET status = CM_Open_DevNode_Key(dev_id, KEY_QUERY_VALUE, 0, RegDisposition_OpenExisting, &hkrKey, CM_REGISTRY_SOFTWARE);
    if (status != CR_SUCCESS) {
        loader_log(inst, VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_DRIVER_BIT, 0,
                   "windows_get_device_registry_entry: Failed to open registry key for DeviceID(%ld)", dev_id);
        *result = VK_ERROR_INCOMPATIBLE_DRIVER;
        return false;
    }

    // query value
    LSTATUS ret = RegQueryValueEx(hkrKey, value_name, NULL, NULL, NULL, &requiredSize);

    if (ret != ERROR_SUCCESS) {
        if (ret == ERROR_FILE_NOT_FOUND) {
            loader_log(inst, VULKAN_LOADER_INFO_BIT | VULKAN_LOADER_DRIVER_BIT, 0,
                       "windows_get_device_registry_entry: Device ID(%ld) Does not contain a value for \"%s\"", dev_id, value_name);
        } else {
            loader_log(inst, VULKAN_LOADER_INFO_BIT | VULKAN_LOADER_DRIVER_BIT, 0,
                       "windows_get_device_registry_entry: DeviceID(%ld) Failed to obtain %s size", dev_id, value_name);
        }
        goto out;
    }

    manifest_path = loader_instance_heap_alloc(inst, requiredSize, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (manifest_path == NULL) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_DRIVER_BIT, 0,
                   "windows_get_device_registry_entry: Failed to allocate space for DriverName.");
        *result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    ret = RegQueryValueEx(hkrKey, value_name, NULL, &data_type, (BYTE *)manifest_path, &requiredSize);

    if (ret != ERROR_SUCCESS) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_DRIVER_BIT, 0,
                   "windows_get_device_registry_entry: DeviceID(%ld) Failed to obtain %s", dev_id, value_name);
        *result = VK_ERROR_INCOMPATIBLE_DRIVER;
        goto out;
    }

    if (data_type != REG_SZ && data_type != REG_MULTI_SZ) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_DRIVER_BIT, 0,
                   "windows_get_device_registry_entry: Invalid %s data type. Expected REG_SZ or REG_MULTI_SZ.", value_name);
        *result = VK_ERROR_INCOMPATIBLE_DRIVER;
        goto out;
    }

    found = windows_add_json_entry(inst, reg_data, total_size, value_name, data_type, manifest_path, requiredSize, result);

out:
    loader_instance_heap_free(inst, manifest_path);
    RegCloseKey(hkrKey);
    return found;
}

VkResult windows_get_device_registry_files(const struct loader_instance *inst, uint32_t log_target_flag, char **reg_data,
                                           PDWORD reg_data_size, LPCSTR value_name) {
    const wchar_t *softwareComponentGUID = L"{5c4c3332-344d-483c-8739-259e934c9cc8}";
    const wchar_t *displayGUID = L"{4d36e968-e325-11ce-bfc1-08002be10318}";
#if defined(CM_GETIDLIST_FILTER_PRESENT)
    const ULONG flags = CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT;
#else
    const ULONG flags = 0x300;
#endif

    wchar_t childGuid[MAX_GUID_STRING_LEN + 2];  // +2 for brackets {}
    for (uint32_t i = 0; i < MAX_GUID_STRING_LEN + 2; i++) {
        childGuid[i] = L'\0';
    }
    ULONG childGuidSize = sizeof(childGuid);

    DEVINST devID = 0, childID = 0;
    wchar_t *pDeviceNames = NULL;
    ULONG deviceNamesSize = 0;
    VkResult result = VK_SUCCESS;
    bool found = false;

    assert(reg_data != NULL && "windows_get_device_registry_files: reg_data is NULL");

    // if after obtaining the DeviceNameSize, new device is added start over
    do {
        CM_Get_Device_ID_List_SizeW(&deviceNamesSize, displayGUID, flags);

        loader_instance_heap_free(inst, pDeviceNames);

        pDeviceNames = loader_instance_heap_alloc(inst, deviceNamesSize * sizeof(wchar_t), VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
        if (pDeviceNames == NULL) {
            loader_log(inst, VULKAN_LOADER_ERROR_BIT | log_target_flag, 0,
                       "windows_get_device_registry_files: Failed to allocate space for display device names.");
            result = VK_ERROR_OUT_OF_HOST_MEMORY;
            return result;
        }
    } while (CM_Get_Device_ID_ListW(displayGUID, pDeviceNames, deviceNamesSize, flags) == CR_BUFFER_SMALL);

    if (pDeviceNames) {
        for (wchar_t *deviceName = pDeviceNames; *deviceName; deviceName += wcslen(deviceName) + 1) {
            CONFIGRET status = CM_Locate_DevNodeW(&devID, deviceName, CM_LOCATE_DEVNODE_NORMAL);
            if (CR_SUCCESS != status) {
                loader_log(inst, VULKAN_LOADER_ERROR_BIT | log_target_flag, 0,
                           "windows_get_device_registry_files: failed to open DevNode %ls", deviceName);
                continue;
            }
            ULONG ulStatus, ulProblem;
            status = CM_Get_DevNode_Status(&ulStatus, &ulProblem, devID, 0);

            if (CR_SUCCESS != status) {
                loader_log(inst, VULKAN_LOADER_ERROR_BIT | log_target_flag, 0,
                           "windows_get_device_registry_files: failed to probe device status %ls", deviceName);
                continue;
            }
            if ((ulStatus & DN_HAS_PROBLEM) && (ulProblem == CM_PROB_NEED_RESTART || ulProblem == DN_NEED_RESTART)) {
                loader_log(inst, VULKAN_LOADER_INFO_BIT | log_target_flag, 0,
                           "windows_get_device_registry_files: device %ls is pending reboot, skipping ...", deviceName);
                continue;
            }

            loader_log(inst, VULKAN_LOADER_INFO_BIT | log_target_flag, 0, "windows_get_device_registry_files: opening device %ls",
                       deviceName);

            if (windows_get_device_registry_entry(inst, reg_data, reg_data_size, devID, value_name, &result)) {
                found = true;
                continue;
            } else if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                break;
            }

            status = CM_Get_Child(&childID, devID, 0);
            if (status != CR_SUCCESS) {
                loader_log(inst, VULKAN_LOADER_INFO_BIT | log_target_flag, 0,
                           "windows_get_device_registry_files: unable to open child-device error:%ld", status);
                continue;
            }

            do {
                wchar_t buffer[MAX_DEVICE_ID_LEN];
                CM_Get_Device_IDW(childID, buffer, MAX_DEVICE_ID_LEN, 0);

                loader_log(inst, VULKAN_LOADER_INFO_BIT | log_target_flag, 0,
                           "windows_get_device_registry_files: Opening child device %ld - %ls", childID, buffer);

                status = CM_Get_DevNode_Registry_PropertyW(childID, CM_DRP_CLASSGUID, NULL, &childGuid, &childGuidSize, 0);
                if (status != CR_SUCCESS) {
                    loader_log(inst, VULKAN_LOADER_ERROR_BIT | log_target_flag, 0,
                               "windows_get_device_registry_files: unable to obtain GUID for:%ld error:%ld", childID, status);

                    result = VK_ERROR_INCOMPATIBLE_DRIVER;
                    continue;
                }

                if (wcscmp(childGuid, softwareComponentGUID) != 0) {
                    loader_log(inst, VULKAN_LOADER_DEBUG_BIT | log_target_flag, 0,
                               "windows_get_device_registry_files: GUID for %ld is not SoftwareComponent skipping", childID);
                    continue;
                }

                if (windows_get_device_registry_entry(inst, reg_data, reg_data_size, childID, value_name, &result)) {
                    found = true;
                    break;  // check next-display-device
                }

            } while (CM_Get_Sibling(&childID, childID, 0) == CR_SUCCESS);
        }

        loader_instance_heap_free(inst, pDeviceNames);
    }

    if (!found && result != VK_ERROR_OUT_OF_HOST_MEMORY) {
        loader_log(inst, log_target_flag, 0, "windows_get_device_registry_files: found no registry files");
        result = VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    return result;
}

VkResult windows_get_registry_files(const struct loader_instance *inst, char *location, bool use_secondary_hive, char **reg_data,
                                    PDWORD reg_data_size) {
    // This list contains all of the allowed ICDs. This allows us to verify that a device is actually present from the vendor
    // specified. This does disallow other vendors, but any new driver should use the device-specific registries anyway.
    const struct {
        const char *filename;
        unsigned int vendor_id;
    } known_drivers[] = {
#if defined(_WIN64)
        {
            .filename = "igvk64.json",
            .vendor_id = 0x8086,
        },
        {
            .filename = "nv-vk64.json",
            .vendor_id = 0x10de,
        },
        {
            .filename = "amd-vulkan64.json",
            .vendor_id = 0x1002,
        },
        {
            .filename = "amdvlk64.json",
            .vendor_id = 0x1002,
        },
#else
        {
            .filename = "igvk32.json",
            .vendor_id = 0x8086,
        },
        {
            .filename = "nv-vk32.json",
            .vendor_id = 0x10de,
        },
        {
            .filename = "amd-vulkan32.json",
            .vendor_id = 0x1002,
        },
        {
            .filename = "amdvlk32.json",
            .vendor_id = 0x1002,
        },
#endif
    };

    LONG rtn_value;
    HKEY hive = DEFAULT_VK_REGISTRY_HIVE, key;
    DWORD access_flags;
    char name[2048];
    char *loc = location;
    char *next;
    DWORD name_size = sizeof(name);
    DWORD value;
    DWORD value_size = sizeof(value);
    VkResult result = VK_SUCCESS;
    bool found = false;
    IDXGIFactory1 *dxgi_factory = NULL;
    bool is_driver = !strcmp(location, VK_DRIVERS_INFO_REGISTRY_LOC);
    uint32_t log_target_flag = is_driver ? VULKAN_LOADER_DRIVER_BIT : VULKAN_LOADER_LAYER_BIT;

    assert(reg_data != NULL && "windows_get_registry_files: reg_data is a NULL pointer");

    if (is_driver) {
        HRESULT hres = fpCreateDXGIFactory1(&IID_IDXGIFactory1, (void **)&dxgi_factory);
        if (hres != S_OK) {
            loader_log(inst, VULKAN_LOADER_WARN_BIT | log_target_flag, 0,
                       "windows_get_registry_files: Failed to create dxgi factory for ICD registry verification. No ICDs will be "
                       "added from "
                       "legacy registry locations");
            goto out;
        }
    }

    while (*loc) {
        next = loader_get_next_path(loc);
        access_flags = KEY_QUERY_VALUE;
        rtn_value = RegOpenKeyEx(hive, loc, 0, access_flags, &key);
        if (ERROR_SUCCESS == rtn_value) {
            for (DWORD idx = 0;
                 (rtn_value = RegEnumValue(key, idx++, name, &name_size, NULL, NULL, (LPBYTE)&value, &value_size)) == ERROR_SUCCESS;
                 name_size = sizeof(name), value_size = sizeof(value)) {
                if (value_size == sizeof(value) && value == 0) {
                    if (NULL == *reg_data) {
                        *reg_data = loader_instance_heap_alloc(inst, *reg_data_size, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
                        if (NULL == *reg_data) {
                            loader_log(inst, VULKAN_LOADER_ERROR_BIT | log_target_flag, 0,
                                       "windows_get_registry_files: Failed to allocate space for registry data for key %s", name);
                            RegCloseKey(key);
                            result = VK_ERROR_OUT_OF_HOST_MEMORY;
                            goto out;
                        }
                        *reg_data[0] = '\0';
                    } else if (strlen(*reg_data) + name_size + 1 > *reg_data_size) {
                        void *new_ptr = loader_instance_heap_realloc(inst, *reg_data, *reg_data_size, *reg_data_size * 2,
                                                                     VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
                        if (NULL == new_ptr) {
                            loader_log(
                                inst, VULKAN_LOADER_ERROR_BIT | log_target_flag, 0,
                                "windows_get_registry_files: Failed to reallocate space for registry value of size %ld for key %s",
                                *reg_data_size * 2, name);
                            RegCloseKey(key);
                            result = VK_ERROR_OUT_OF_HOST_MEMORY;
                            goto out;
                        }
                        *reg_data = new_ptr;
                        *reg_data_size *= 2;
                    }

                    // We've now found a json file. If this is an ICD, we still need to check if there is actually a device
                    // that matches this ICD
                    loader_log(inst, VULKAN_LOADER_INFO_BIT | log_target_flag, 0,
                               "Located json file \"%s\" from registry \"%s\\%s\"", name,
                               hive == DEFAULT_VK_REGISTRY_HIVE ? DEFAULT_VK_REGISTRY_HIVE_STR : SECONDARY_VK_REGISTRY_HIVE_STR,
                               location);
                    if (is_driver) {
                        uint32_t i = 0;
                        for (i = 0; i < sizeof(known_drivers) / sizeof(known_drivers[0]); ++i) {
                            if (!strcmp(name + strlen(name) - strlen(known_drivers[i].filename), known_drivers[i].filename)) {
                                break;
                            }
                        }
                        if (i == sizeof(known_drivers) / sizeof(known_drivers[0])) {
                            loader_log(inst, VULKAN_LOADER_INFO_BIT | log_target_flag, 0,
                                       "Driver %s is not recognized as a known driver. It will be assumed to be active", name);
                        } else {
                            bool found_gpu = false;
                            for (int j = 0;; ++j) {
                                IDXGIAdapter1 *adapter;
                                HRESULT hres = dxgi_factory->lpVtbl->EnumAdapters1(dxgi_factory, j, &adapter);
                                if (hres == DXGI_ERROR_NOT_FOUND) {
                                    break;
                                } else if (hres != S_OK) {
                                    loader_log(inst, VULKAN_LOADER_WARN_BIT | log_target_flag, 0,
                                               "Failed to enumerate DXGI adapters at index %d. As a result, drivers may be skipped",
                                               j);
                                    continue;
                                }

                                DXGI_ADAPTER_DESC1 description;
                                hres = adapter->lpVtbl->GetDesc1(adapter, &description);
                                if (hres != S_OK) {
                                    loader_log(
                                        inst, VULKAN_LOADER_INFO_BIT | log_target_flag, 0,
                                        "Failed to get DXGI adapter information at index %d. As a result, drivers may be skipped",
                                        j);
                                    continue;
                                }

                                if (description.VendorId == known_drivers[i].vendor_id) {
                                    found_gpu = true;
                                    break;
                                }
                            }

                            if (!found_gpu) {
                                loader_log(inst, VULKAN_LOADER_INFO_BIT | log_target_flag, 0,
                                           "Dropping driver %s as no corresponding DXGI adapter was found", name);
                                continue;
                            }
                        }
                    }

                    if (strlen(*reg_data) == 0) {
                        // The list is emtpy. Add the first entry.
                        (void)snprintf(*reg_data, name_size + 1, "%s", name);
                        found = true;
                    } else {
                        // At this point the reg_data variable contains other JSON paths, likely from the PNP/device section
                        // of the registry that we want to have precedence over this non-device specific section of the registry.
                        // To make sure we avoid enumerating old JSON files/drivers that might be present in the non-device specific
                        // area of the registry when a newer device specific JSON file is present, do a check before adding.
                        // Find the file name, without path, of the JSON file found in the non-device specific registry location.
                        // If the same JSON file name is already found in the list, don't add it again.
                        bool foundDuplicate = false;
                        char *pLastSlashName = strrchr(name, '\\');
                        if (pLastSlashName != NULL) {
                            char *foundMatch = strstr(*reg_data, pLastSlashName + 1);
                            if (foundMatch != NULL) {
                                foundDuplicate = true;
                            }
                        }
                        // Only skip if we are adding a driver and a duplicate was found
                        if (!is_driver || (is_driver && foundDuplicate == false)) {
                            // Add the new entry to the list.
                            (void)snprintf(*reg_data + strlen(*reg_data), name_size + 2, "%c%s", PATH_SEPARATOR, name);
                            found = true;
                        } else {
                            loader_log(
                                inst, VULKAN_LOADER_INFO_BIT | log_target_flag, 0,
                                "Skipping adding of json file \"%s\" from registry \"%s\\%s\" to the list due to duplication", name,
                                hive == DEFAULT_VK_REGISTRY_HIVE ? DEFAULT_VK_REGISTRY_HIVE_STR : SECONDARY_VK_REGISTRY_HIVE_STR,
                                location);
                        }
                    }
                }
            }
            RegCloseKey(key);
        }

        // Advance the location - if the next location is in the secondary hive, then reset the locations and advance the hive
        if (use_secondary_hive && (hive == DEFAULT_VK_REGISTRY_HIVE) && (*next == '\0')) {
            loc = location;
            hive = SECONDARY_VK_REGISTRY_HIVE;
        } else {
            loc = next;
        }
    }

    if (!found && result != VK_ERROR_OUT_OF_HOST_MEMORY) {
        loader_log(inst, log_target_flag, 0, "Found no registry files in %s\\%s",
                   (hive == DEFAULT_VK_REGISTRY_HIVE) ? DEFAULT_VK_REGISTRY_HIVE_STR : SECONDARY_VK_REGISTRY_HIVE_STR, location);
        result = VK_ERROR_INCOMPATIBLE_DRIVER;
    }

out:
    if (is_driver && dxgi_factory != NULL) {
        dxgi_factory->lpVtbl->Release(dxgi_factory);
    }

    return result;
}

// Read manifest JSON files using the Windows driver interface
VkResult windows_read_manifest_from_d3d_adapters(const struct loader_instance *inst, char **reg_data, PDWORD reg_data_size,
                                                 const wchar_t *value_name) {
    VkResult result = VK_INCOMPLETE;
    LoaderEnumAdapters2 adapters = {.adapter_count = 0, .adapters = NULL};
    LoaderQueryRegistryInfo *full_info = NULL;
    size_t full_info_size = 0;
    char *json_path = NULL;
    size_t json_path_size = 0;

    HMODULE gdi32_dll = GetModuleHandle("gdi32.dll");
    if (gdi32_dll == NULL) {
        result = VK_ERROR_INCOMPATIBLE_DRIVER;
        goto out;
    }

    PFN_LoaderEnumAdapters2 fpLoaderEnumAdapters2 =
        (PFN_LoaderEnumAdapters2)(void *)GetProcAddress(gdi32_dll, "D3DKMTEnumAdapters2");
    PFN_LoaderQueryAdapterInfo fpLoaderQueryAdapterInfo =
        (PFN_LoaderQueryAdapterInfo)(void *)GetProcAddress(gdi32_dll, "D3DKMTQueryAdapterInfo");
    if (fpLoaderEnumAdapters2 == NULL || fpLoaderQueryAdapterInfo == NULL) {
        result = VK_ERROR_INCOMPATIBLE_DRIVER;
        goto out;
    }

    // Get all of the adapters
    NTSTATUS status = fpLoaderEnumAdapters2(&adapters);
    if (status == STATUS_SUCCESS && adapters.adapter_count > 0) {
        adapters.adapters = loader_instance_heap_alloc(inst, sizeof(*adapters.adapters) * adapters.adapter_count,
                                                       VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        if (adapters.adapters == NULL) {
            goto out;
        }
        status = fpLoaderEnumAdapters2(&adapters);
    }
    if (status != STATUS_SUCCESS) {
        goto out;
    }

    // If that worked, we need to get the manifest file(s) for each adapter
    for (ULONG i = 0; i < adapters.adapter_count; ++i) {
        // The first query should just check if the field exists and how big it is
        LoaderQueryRegistryInfo filename_info = {
            .query_type = LOADER_QUERY_REGISTRY_ADAPTER_KEY,
            .query_flags =
                {
                    .translate_path = true,
                },
            .value_type = REG_MULTI_SZ,
            .physical_adapter_index = 0,
        };
        size_t value_name_size = wcslen(value_name);
        wcsncpy_s(filename_info.value_name, MAX_PATH, value_name, value_name_size);
        LoaderQueryAdapterInfo query_info;
        query_info.handle = adapters.adapters[i].handle;
        query_info.type = LOADER_QUERY_TYPE_REGISTRY;
        query_info.private_data = &filename_info;
        query_info.private_data_size = sizeof(filename_info);
        status = fpLoaderQueryAdapterInfo(&query_info);

        // This error indicates that the type didn't match, so we'll try a REG_SZ
        if (status != STATUS_SUCCESS) {
            filename_info.value_type = REG_SZ;
            status = fpLoaderQueryAdapterInfo(&query_info);
        }

        if (status != STATUS_SUCCESS || filename_info.status != LOADER_QUERY_REGISTRY_STATUS_BUFFER_OVERFLOW) {
            continue;
        }

        while (status == STATUS_SUCCESS &&
               ((LoaderQueryRegistryInfo *)query_info.private_data)->status == LOADER_QUERY_REGISTRY_STATUS_BUFFER_OVERFLOW) {
            bool needs_copy = (full_info == NULL);
            size_t full_size = sizeof(LoaderQueryRegistryInfo) + filename_info.output_value_size;
            void *buffer =
                loader_instance_heap_realloc(inst, full_info, full_info_size, full_size, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
            if (buffer == NULL) {
                result = VK_ERROR_OUT_OF_HOST_MEMORY;
                goto out;
            }
            full_info = buffer;
            full_info_size = full_size;

            if (needs_copy) {
                memcpy(full_info, &filename_info, sizeof(LoaderQueryRegistryInfo));
            }
            query_info.private_data = full_info;
            query_info.private_data_size = (UINT)full_info_size;
            status = fpLoaderQueryAdapterInfo(&query_info);
        }

        if (status != STATUS_SUCCESS || full_info->status != LOADER_QUERY_REGISTRY_STATUS_SUCCESS) {
            goto out;
        }

        // Convert the wide string to a narrow string
        void *buffer = loader_instance_heap_realloc(inst, json_path, json_path_size, full_info->output_value_size,
                                                    VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        if (buffer == NULL) {
            result = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto out;
        }
        json_path = buffer;
        json_path_size = full_info->output_value_size;

        // Iterate over each component string
        for (const wchar_t *curr_path = full_info->output_string; curr_path[0] != '\0'; curr_path += wcslen(curr_path) + 1) {
            WideCharToMultiByte(CP_UTF8, 0, curr_path, -1, json_path, (int)json_path_size, NULL, NULL);

            // Add the string to the output list
            result = VK_SUCCESS;
            windows_add_json_entry(inst, reg_data, reg_data_size, (LPCTSTR)L"EnumAdapters", REG_SZ, json_path,
                                   (DWORD)strlen(json_path) + 1, &result);
            if (result != VK_SUCCESS) {
                goto out;
            }

            // If this is a string and not a multi-string, we don't want to go through the loop more than once
            if (full_info->value_type == REG_SZ) {
                break;
            }
        }
    }

out:
    loader_instance_heap_free(inst, json_path);
    loader_instance_heap_free(inst, full_info);
    loader_instance_heap_free(inst, adapters.adapters);

    return result;
}

// Look for data files in the registry.
VkResult windows_read_data_files_in_registry(const struct loader_instance *inst, enum loader_data_files_type data_file_type,
                                             bool warn_if_not_present, char *registry_location,
                                             struct loader_string_list *out_files) {
    VkResult vk_result = VK_SUCCESS;
    char *search_path = NULL;
    uint32_t log_target_flag = 0;

    if (data_file_type == LOADER_DATA_FILE_MANIFEST_DRIVER) {
        log_target_flag = VULKAN_LOADER_DRIVER_BIT;
        loader_log(inst, log_target_flag, 0, "Checking for Driver Manifest files in Registry at %s\\%s",
                   DEFAULT_VK_REGISTRY_HIVE_STR, registry_location);
    } else {
        log_target_flag = VULKAN_LOADER_LAYER_BIT;
        loader_log(inst, log_target_flag, 0, "Checking for Layer Manifest files in Registry at %s\\%s",
                   DEFAULT_VK_REGISTRY_HIVE_STR, registry_location);
    }

    // These calls look at the PNP/Device section of the registry.
    VkResult regHKR_result = VK_SUCCESS;
    DWORD reg_size = 4096;
    if (!strncmp(registry_location, VK_DRIVERS_INFO_REGISTRY_LOC, sizeof(VK_DRIVERS_INFO_REGISTRY_LOC))) {
        // If we're looking for drivers we need to try enumerating adapters
        regHKR_result = windows_read_manifest_from_d3d_adapters(inst, &search_path, &reg_size, LoaderPnpDriverRegistryWide());
        if (regHKR_result == VK_INCOMPLETE) {
            regHKR_result =
                windows_get_device_registry_files(inst, log_target_flag, &search_path, &reg_size, LoaderPnpDriverRegistry());
        }
    } else if (!strncmp(registry_location, VK_ELAYERS_INFO_REGISTRY_LOC, sizeof(VK_ELAYERS_INFO_REGISTRY_LOC))) {
        regHKR_result = windows_read_manifest_from_d3d_adapters(inst, &search_path, &reg_size, LoaderPnpELayerRegistryWide());
        if (regHKR_result == VK_INCOMPLETE) {
            regHKR_result =
                windows_get_device_registry_files(inst, log_target_flag, &search_path, &reg_size, LoaderPnpELayerRegistry());
        }
    } else if (!strncmp(registry_location, VK_ILAYERS_INFO_REGISTRY_LOC, sizeof(VK_ILAYERS_INFO_REGISTRY_LOC))) {
        regHKR_result = windows_read_manifest_from_d3d_adapters(inst, &search_path, &reg_size, LoaderPnpILayerRegistryWide());
        if (regHKR_result == VK_INCOMPLETE) {
            regHKR_result =
                windows_get_device_registry_files(inst, log_target_flag, &search_path, &reg_size, LoaderPnpILayerRegistry());
        }
    }

    if (regHKR_result == VK_ERROR_OUT_OF_HOST_MEMORY) {
        vk_result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    // This call looks into the Khronos non-device specific section of the registry for layer files.
    bool use_secondary_hive = (data_file_type != LOADER_DATA_FILE_MANIFEST_DRIVER) && (!is_high_integrity());
    VkResult reg_result = windows_get_registry_files(inst, registry_location, use_secondary_hive, &search_path, &reg_size);
    if (reg_result == VK_ERROR_OUT_OF_HOST_MEMORY) {
        vk_result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    if ((VK_SUCCESS != reg_result && VK_SUCCESS != regHKR_result) || NULL == search_path) {
        if (data_file_type == LOADER_DATA_FILE_MANIFEST_DRIVER) {
            loader_log(inst, VULKAN_LOADER_ERROR_BIT | log_target_flag, 0,
                       "windows_read_data_files_in_registry: Registry lookup failed to get ICD manifest files.  Possibly missing "
                       "Vulkan driver?");
            vk_result = VK_ERROR_INCOMPATIBLE_DRIVER;
        } else {
            if (warn_if_not_present) {
                if (data_file_type == LOADER_DATA_FILE_MANIFEST_IMPLICIT_LAYER ||
                    data_file_type == LOADER_DATA_FILE_MANIFEST_EXPLICIT_LAYER) {
                    // This is only a warning for layers
                    loader_log(inst, VULKAN_LOADER_WARN_BIT | log_target_flag, 0,
                               "windows_read_data_files_in_registry: Registry lookup failed to get layer manifest files.");
                } else {
                    // This is only a warning for general data files
                    loader_log(inst, VULKAN_LOADER_WARN_BIT | log_target_flag, 0,
                               "windows_read_data_files_in_registry: Registry lookup failed to get data files.");
                }
            }
            // Return success for now since it's not critical for layers
            vk_result = VK_SUCCESS;
        }
        goto out;
    }

    // Now, parse the paths and add any manifest files found in them.
    vk_result = add_data_files(inst, search_path, out_files, false);

out:

    loader_instance_heap_free(inst, search_path);

    return vk_result;
}

VkResult enumerate_adapter_physical_devices(struct loader_instance *inst, struct loader_icd_term *icd_term, LUID luid,
                                            uint32_t *icd_phys_devs_array_count,
                                            struct loader_icd_physical_devices *icd_phys_devs_array) {
    uint32_t count = 0;
    VkResult res = icd_term->scanned_icd->EnumerateAdapterPhysicalDevices(icd_term->instance, luid, &count, NULL);
    if (res == VK_ERROR_OUT_OF_HOST_MEMORY) {
        return res;
    } else if (res == VK_ERROR_INCOMPATIBLE_DRIVER || res == VK_ERROR_INITIALIZATION_FAILED || 0 == count) {
        return VK_SUCCESS;  // This driver doesn't support the adapter
    } else if (res != VK_SUCCESS) {
        loader_log(inst, VULKAN_LOADER_WARN_BIT, 0,
                   "Failed to convert DXGI adapter into Vulkan physical device with unexpected error code: %d", res);
        return VK_SUCCESS;
    }

    // Take a pointer to the last element of icd_phys_devs_array to simplify usage
    struct loader_icd_physical_devices *next_icd_phys_devs = &icd_phys_devs_array[*icd_phys_devs_array_count];

    // Get the actual physical devices
    do {
        next_icd_phys_devs->physical_devices = loader_instance_heap_realloc(
            inst, next_icd_phys_devs->physical_devices, next_icd_phys_devs->device_count * sizeof(VkPhysicalDevice),
            count * sizeof(VkPhysicalDevice), VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        if (next_icd_phys_devs->physical_devices == NULL) {
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }
        next_icd_phys_devs->device_count = count;
    } while ((res = icd_term->scanned_icd->EnumerateAdapterPhysicalDevices(icd_term->instance, luid, &count,
                                                                           next_icd_phys_devs->physical_devices)) == VK_INCOMPLETE);

    if (res != VK_SUCCESS) {
        loader_instance_heap_free(inst, next_icd_phys_devs->physical_devices);
        next_icd_phys_devs->physical_devices = NULL;
        // Unless OOHM occurs, only return VK_SUCCESS
        if (res != VK_ERROR_OUT_OF_HOST_MEMORY) {
            res = VK_SUCCESS;
            loader_log(inst, VULKAN_LOADER_WARN_BIT, 0, "Failed to convert DXGI adapter into Vulkan physical device");
        }
        return res;
    }

    // Because the loader calls EnumerateAdapterPhysicalDevices on all drivers with each DXGI Adapter, if there are multiple drivers
    // that share a luid the physical device will get queried multiple times. We can prevent that by not adding them if the
    // enumerated physical devices have already been added.
    bool already_enumerated = false;
    for (uint32_t j = 0; j < *icd_phys_devs_array_count; j++) {
        if (count == icd_phys_devs_array[j].device_count) {
            bool matches = true;
            for (uint32_t k = 0; k < icd_phys_devs_array[j].device_count; k++) {
                if (icd_phys_devs_array[j].physical_devices[k] != next_icd_phys_devs->physical_devices[k]) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                already_enumerated = true;
            }
        }
    }
    if (!already_enumerated) {
        next_icd_phys_devs->device_count = count;
        next_icd_phys_devs->icd_term = icd_term;
        next_icd_phys_devs->windows_adapter_luid = luid;
        (*icd_phys_devs_array_count)++;
    } else {
        // Avoid memory leak in case of the already_enumerated hitting true
        // at the last enumerate_adapter_physical_devices call in the outer loop
        loader_instance_heap_free(inst, next_icd_phys_devs->physical_devices);
        next_icd_phys_devs->physical_devices = NULL;
    }

    return VK_SUCCESS;
}

// Whenever there are multiple drivers for the same hardware and one of the drivers is an implementation layered on top of another
// API (such as the Dozen driver which converts vulkan to Dx12), we want to make sure the layered driver appears after the 'native'
// driver. This function iterates over all physical devices and make sure any with matching LUID's are sorted such that drivers with
// a underlyingAPI of VK_LAYERED_DRIVER_UNDERLYING_API_D3D12_MSFT are ordered after drivers without it.
void sort_physical_devices_with_same_luid(struct loader_instance *inst, uint32_t icd_phys_devs_array_count,
                                          struct loader_icd_physical_devices *icd_phys_devs_array) {
    bool app_is_vulkan_1_1 = loader_check_version_meets_required(LOADER_VERSION_1_1_0, inst->app_api_version);

    for (uint32_t i = 0; icd_phys_devs_array_count > 1 && i < icd_phys_devs_array_count - 1; i++) {
        for (uint32_t j = i + 1; j < icd_phys_devs_array_count; j++) {
            // Only want to reorder physical devices if their ICD's LUID's match
            if ((icd_phys_devs_array[i].windows_adapter_luid.HighPart != icd_phys_devs_array[j].windows_adapter_luid.HighPart) ||
                (icd_phys_devs_array[i].windows_adapter_luid.LowPart != icd_phys_devs_array[j].windows_adapter_luid.LowPart)) {
                continue;
            }

            VkLayeredDriverUnderlyingApiMSFT underlyingAPI = VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT;
            VkPhysicalDeviceLayeredDriverPropertiesMSFT layered_driver_properties_msft = {0};
            layered_driver_properties_msft.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT;
            VkPhysicalDeviceProperties2 props2 = {0};
            props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            props2.pNext = (void *)&layered_driver_properties_msft;

            // Because there may be multiple physical devices associated with each ICD, we need to check each physical device
            // whether it is layered
            for (uint32_t k = 0; k < icd_phys_devs_array[i].device_count; k++) {
                VkPhysicalDeviceProperties dev_props = {0};
                icd_phys_devs_array[i].icd_term->dispatch.GetPhysicalDeviceProperties(icd_phys_devs_array[i].physical_devices[k],
                                                                                      &dev_props);

                bool device_is_1_1_capable =
                    loader_check_version_meets_required(LOADER_VERSION_1_1_0, loader_make_version(dev_props.apiVersion));

                PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2 = NULL;
                if (app_is_vulkan_1_1 && device_is_1_1_capable) {
                    GetPhysDevProps2 = icd_phys_devs_array[i].icd_term->dispatch.GetPhysicalDeviceProperties2;
                } else {
                    GetPhysDevProps2 = (PFN_vkGetPhysicalDeviceProperties2)icd_phys_devs_array[i]
                                           .icd_term->dispatch.GetPhysicalDeviceProperties2KHR;
                }
                if (GetPhysDevProps2) {
                    GetPhysDevProps2(icd_phys_devs_array[i].physical_devices[k], &props2);
                    if (layered_driver_properties_msft.underlyingAPI != VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT) {
                        underlyingAPI = layered_driver_properties_msft.underlyingAPI;
                        break;
                    }
                }
            }
            if (underlyingAPI == VK_LAYERED_DRIVER_UNDERLYING_API_D3D12_MSFT) {
                struct loader_icd_physical_devices swap_icd = icd_phys_devs_array[i];
                icd_phys_devs_array[i] = icd_phys_devs_array[j];
                icd_phys_devs_array[j] = swap_icd;
            }
        }
    }
}

// This function allocates icd_phys_devs_array which must be freed by the caller if not null
VkResult windows_read_sorted_physical_devices(struct loader_instance *inst, uint32_t *icd_phys_devs_array_count,
                                              struct loader_icd_physical_devices **icd_phys_devs_array) {
    VkResult res = VK_SUCCESS;

    uint32_t icd_phys_devs_array_size = 0;
    struct loader_icd_term *icd_term = NULL;
    IDXGIFactory6 *dxgi_factory = NULL;
    HRESULT hres = fpCreateDXGIFactory1(&IID_IDXGIFactory6, (void **)&dxgi_factory);
    if (hres != S_OK) {
        loader_log(inst, VULKAN_LOADER_INFO_BIT, 0, "Failed to create DXGI factory 6. Physical devices will not be sorted");
        goto out;
    }
    icd_phys_devs_array_size = 16;
    *icd_phys_devs_array = loader_instance_heap_calloc(inst, icd_phys_devs_array_size * sizeof(struct loader_icd_physical_devices),
                                                       VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    if (*icd_phys_devs_array == NULL) {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    for (uint32_t i = 0;; ++i) {
        IDXGIAdapter1 *adapter;
        hres = dxgi_factory->lpVtbl->EnumAdapterByGpuPreference(dxgi_factory, i, DXGI_GPU_PREFERENCE_UNSPECIFIED,
                                                                &IID_IDXGIAdapter1, (void **)&adapter);
        if (hres == DXGI_ERROR_NOT_FOUND) {
            break;  // No more adapters
        } else if (hres != S_OK) {
            loader_log(inst, VULKAN_LOADER_WARN_BIT, 0,
                       "Failed to enumerate adapters by GPU preference at index %u. This adapter will not be sorted", i);
            break;
        }

        DXGI_ADAPTER_DESC1 description;
        hres = adapter->lpVtbl->GetDesc1(adapter, &description);
        if (hres != S_OK) {
            loader_log(inst, VULKAN_LOADER_WARN_BIT, 0, "Failed to get adapter LUID index %u. This adapter will not be sorted", i);
            continue;
        }

        icd_term = inst->icd_terms;
        while (NULL != icd_term) {
            // This is the new behavior, which cannot be run unless the ICD provides EnumerateAdapterPhysicalDevices
            if (icd_term->scanned_icd->EnumerateAdapterPhysicalDevices == NULL) {
                icd_term = icd_term->next;
                continue;
            }

            if (icd_phys_devs_array_size <= *icd_phys_devs_array_count) {
                uint32_t old_size = icd_phys_devs_array_size * sizeof(struct loader_icd_physical_devices);
                *icd_phys_devs_array = loader_instance_heap_realloc(inst, *icd_phys_devs_array, old_size, 2 * old_size,
                                                                    VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
                if (*icd_phys_devs_array == NULL) {
                    adapter->lpVtbl->Release(adapter);
                    res = VK_ERROR_OUT_OF_HOST_MEMORY;
                    goto out;
                }
                icd_phys_devs_array_size *= 2;
            }
            (*icd_phys_devs_array)[*icd_phys_devs_array_count].device_count = 0;
            (*icd_phys_devs_array)[*icd_phys_devs_array_count].physical_devices = NULL;

            res = enumerate_adapter_physical_devices(inst, icd_term, description.AdapterLuid, icd_phys_devs_array_count,
                                                     *icd_phys_devs_array);
            if (res == VK_ERROR_OUT_OF_HOST_MEMORY) {
                adapter->lpVtbl->Release(adapter);
                goto out;
            }
            icd_term = icd_term->next;
        }

        adapter->lpVtbl->Release(adapter);
    }

    dxgi_factory->lpVtbl->Release(dxgi_factory);

    sort_physical_devices_with_same_luid(inst, *icd_phys_devs_array_count, *icd_phys_devs_array);

out:
    if (*icd_phys_devs_array_count == 0 && *icd_phys_devs_array != NULL) {
        loader_instance_heap_free(inst, *icd_phys_devs_array);
        *icd_phys_devs_array = NULL;
    }
    return res;
}

VkLoaderFeatureFlags windows_initialize_dxgi(void) {
    VkLoaderFeatureFlags feature_flags = 0;
    IDXGIFactory6 *dxgi_factory = NULL;
    HRESULT hres = fpCreateDXGIFactory1(&IID_IDXGIFactory6, (void **)&dxgi_factory);
    if (hres == S_OK) {
        feature_flags |= VK_LOADER_FEATURE_PHYSICAL_DEVICE_SORTING;
        dxgi_factory->lpVtbl->Release(dxgi_factory);
    }
    return feature_flags;
}

// Sort the VkPhysicalDevices that are part of the current group with the list passed in from the sorted list.
// Multiple groups could have devices out of the same sorted list, however, a single group's devices must all come
// from the same sorted list.
void windows_sort_devices_in_group(struct loader_instance *inst, struct VkPhysicalDeviceGroupProperties *group_props,
                                   struct loader_icd_physical_devices *icd_sorted_list) {
    uint32_t cur_index = 0;
    for (uint32_t dev = 0; dev < icd_sorted_list->device_count; ++dev) {
        for (uint32_t grp_dev = cur_index; grp_dev < group_props->physicalDeviceCount; ++grp_dev) {
            if (icd_sorted_list->physical_devices[dev] == group_props->physicalDevices[grp_dev]) {
                if (cur_index != grp_dev) {
                    VkPhysicalDevice swap_dev = group_props->physicalDevices[cur_index];
                    group_props->physicalDevices[cur_index] = group_props->physicalDevices[grp_dev];
                    group_props->physicalDevices[grp_dev] = swap_dev;
                }
                cur_index++;
                break;
            }
        }
    }
    if (cur_index == 0) {
        loader_log(inst, VULKAN_LOADER_WARN_BIT, 0,
                   "windows_sort_devices_in_group:  Never encountered a device in the sorted list group");
    }
}

// This function sorts an array in physical device groups based on the sorted physical device information
VkResult windows_sort_physical_device_groups(struct loader_instance *inst, const uint32_t group_count,
                                             struct loader_physical_device_group_term *sorted_group_term,
                                             const uint32_t sorted_device_count,
                                             struct loader_icd_physical_devices *sorted_phys_dev_array) {
    if (0 == group_count || NULL == sorted_group_term) {
        loader_log(inst, VULKAN_LOADER_WARN_BIT, 0,
                   "windows_sort_physical_device_groups: Called with invalid information (Group count %d, Sorted Info %p)",
                   group_count, sorted_group_term);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t new_index = 0;
    for (uint32_t icd = 0; icd < sorted_device_count; ++icd) {
        for (uint32_t dev = 0; dev < sorted_phys_dev_array[icd].device_count; ++dev) {
            // Find a group associated with a given device
            for (uint32_t group = new_index; group < group_count; ++group) {
                bool device_found = false;
                // Look for the current sorted device in a group and put it in the correct location if it isn't already
                for (uint32_t grp_dev = 0; grp_dev < sorted_group_term[group].group_props.physicalDeviceCount; ++grp_dev) {
                    if (sorted_group_term[group].group_props.physicalDevices[grp_dev] ==
                        sorted_phys_dev_array[icd].physical_devices[dev]) {
                        // First, sort devices inside of group to be in priority order
                        windows_sort_devices_in_group(inst, &sorted_group_term[group].group_props, &sorted_phys_dev_array[icd]);

                        // Second, move the group up in priority if it needs to be
                        if (new_index != group) {
                            struct loader_physical_device_group_term tmp = sorted_group_term[new_index];
                            sorted_group_term[new_index] = sorted_group_term[group];
                            sorted_group_term[group] = tmp;
                        }
                        device_found = true;
                        new_index++;
                        break;
                    }
                }
                if (device_found) {
                    break;
                }
            }
        }
    }
    return VK_SUCCESS;
}

char *windows_get_app_package_manifest_path(const struct loader_instance *inst) {
    // These functions are only available on Windows 8 and above, load them dynamically for compatibility with Windows 7
    typedef LONG(WINAPI * PFN_GetPackagesByPackageFamily)(PCWSTR, UINT32 *, PWSTR *, UINT32 *, WCHAR *);
    PFN_GetPackagesByPackageFamily fpGetPackagesByPackageFamily =
        (PFN_GetPackagesByPackageFamily)(void *)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetPackagesByPackageFamily");
    if (!fpGetPackagesByPackageFamily) {
        return NULL;
    }
    typedef LONG(WINAPI * PFN_GetPackagePathByFullName)(PCWSTR, UINT32 *, PWSTR);
    PFN_GetPackagePathByFullName fpGetPackagePathByFullName =
        (PFN_GetPackagePathByFullName)(void *)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetPackagePathByFullName");
    if (!fpGetPackagePathByFullName) {
        return NULL;
    }

    UINT32 numPackages = 0, bufferLength = 0;
    // This literal string identifies the Microsoft-published OpenCL, OpenGL, and Vulkan Compatibility Pack, which contains
    // OpenGLOn12, OpenCLOn12, and VulkanOn12 (aka Dozen) mappinglayers
    PCWSTR familyName = L"Microsoft.D3DMappingLayers_8wekyb3d8bbwe";
    if (ERROR_INSUFFICIENT_BUFFER != fpGetPackagesByPackageFamily(familyName, &numPackages, NULL, &bufferLength, NULL) ||
        numPackages == 0 || bufferLength == 0) {
        loader_log(inst, VULKAN_LOADER_INFO_BIT, 0,
                   "windows_get_app_package_manifest_path: Failed to find mapping layers packages by family name");
        return NULL;
    }

    char *ret = NULL;
    WCHAR *buffer = loader_instance_heap_alloc(inst, sizeof(WCHAR) * bufferLength, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    PWSTR *packages = loader_instance_heap_alloc(inst, sizeof(PWSTR) * numPackages, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    if (!buffer || !packages) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                   "windows_get_app_package_manifest_path: Failed to allocate memory for package names");
        goto cleanup;
    }

    if (ERROR_SUCCESS != fpGetPackagesByPackageFamily(familyName, &numPackages, packages, &bufferLength, buffer)) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                   "windows_get_app_package_manifest_path: Failed to mapping layers package full names");
        goto cleanup;
    }

    UINT32 pathLength = 0;
    WCHAR path[MAX_PATH];
    memset(path, 0, sizeof(path));
    if (ERROR_INSUFFICIENT_BUFFER != fpGetPackagePathByFullName(packages[0], &pathLength, NULL) || pathLength > MAX_PATH ||
        ERROR_SUCCESS != fpGetPackagePathByFullName(packages[0], &pathLength, path)) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                   "windows_get_app_package_manifest_path: Failed to get mapping layers package path");
        goto cleanup;
    }

    int narrowPathLength = WideCharToMultiByte(CP_ACP, 0, path, -1, NULL, 0, NULL, NULL);
    if (narrowPathLength == 0) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                   "windows_get_app_package_manifest_path: Failed to convert path from wide to narrow");
        goto cleanup;
    }

    ret = loader_instance_heap_alloc(inst, narrowPathLength, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    if (!ret) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "windows_get_app_package_manifest_path: Failed to allocate path");
        goto cleanup;
    }

    narrowPathLength = WideCharToMultiByte(CP_ACP, 0, path, -1, ret, narrowPathLength, NULL, NULL);
    assert((size_t)narrowPathLength == strlen(ret) + 1);

cleanup:
    loader_instance_heap_free(inst, buffer);
    loader_instance_heap_free(inst, packages);
    return ret;
}

VkResult get_settings_path_if_exists_in_registry_key(const struct loader_instance *inst, char **out_path, HKEY key) {
    VkResult result = VK_ERROR_INITIALIZATION_FAILED;

    char name[MAX_STRING_SIZE] = {0};
    DWORD name_size = sizeof(name);

    *out_path = NULL;

    LONG rtn_value = ERROR_SUCCESS;
    for (DWORD idx = 0; rtn_value == ERROR_SUCCESS; idx++) {
        DWORD value = 0;
        DWORD value_size = sizeof(value);
        rtn_value = RegEnumValue(key, idx, name, &name_size, NULL, NULL, (LPBYTE)&value, &value_size);

        if (ERROR_SUCCESS != rtn_value) {
            break;
        }

        uint32_t start_of_path_filename = 0;
        for (uint32_t last_char = name_size; last_char > 0; last_char--) {
            if (name[last_char] == '\\') {
                start_of_path_filename = last_char + 1;
                break;
            }
        }

        if (strcmp(VK_LOADER_SETTINGS_FILENAME, &(name[start_of_path_filename])) == 0) {
            // Make sure the path exists first
            if (!loader_platform_file_exists(name)) {
                loader_log(
                    inst, VULKAN_LOADER_DEBUG_BIT, 0,
                    "Registry contained entry to vk_loader_settings.json but the corresponding file does not exist, ignoring");
                return VK_ERROR_INITIALIZATION_FAILED;
            }

            *out_path = loader_instance_heap_calloc(inst, name_size + 1, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
            if (*out_path == NULL) {
                return VK_ERROR_OUT_OF_HOST_MEMORY;
            }
            loader_strncpy(*out_path, name_size + 1, name, name_size);
            (*out_path)[name_size] = '\0';
            result = VK_SUCCESS;
            break;
        }
    }

    return result;
}

VkResult windows_get_loader_settings_file_path(const struct loader_instance *inst, char **out_path) {
    VkResult result = VK_SUCCESS;
    DWORD access_flags = KEY_QUERY_VALUE;
    LONG rtn_value = 0;
    HKEY key = NULL;
    *out_path = NULL;

    // Search in HKEY_CURRENT_USER first if we are running without admin privileges
    // Exit if a settings file was found.
    // Otherwise check in HKEY_LOCAL_MACHINE.

    if (!is_high_integrity()) {
        rtn_value = RegOpenKeyEx(HKEY_CURRENT_USER, VK_SETTINGS_INFO_REGISTRY_LOC, 0, access_flags, &key);
        if (ERROR_SUCCESS == rtn_value) {
            result = get_settings_path_if_exists_in_registry_key(inst, out_path, key);

            // Either we got OOM and *must* exit or we successfully found the settings file and can exit
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_SUCCESS) {
                goto out;
            }
            RegCloseKey(key);
            key = NULL;
        }
    }

    rtn_value = RegOpenKeyEx(HKEY_LOCAL_MACHINE, VK_SETTINGS_INFO_REGISTRY_LOC, 0, access_flags, &key);
    if (ERROR_SUCCESS != rtn_value) {
        result = VK_ERROR_FEATURE_NOT_PRESENT;
        goto out;
    }

    result = get_settings_path_if_exists_in_registry_key(inst, out_path, key);
    if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
        goto out;
    }

out:
    if (NULL != key) {
        RegCloseKey(key);
    }

    return result;
}

#endif  // _WIN32
