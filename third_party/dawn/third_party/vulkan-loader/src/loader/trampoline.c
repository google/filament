/*
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015 Google Inc.
 * Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * Copyright (c) 2023-2023 RasterGrid Kft.
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
 * Author: Courtney Goeltzenleuchter <courtney@lunarg.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Chia-I Wu <olv@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include <stdlib.h>
#include <string.h>

#include "allocation.h"
#include "debug_utils.h"
#include "gpa_helper.h"
#include "loader.h"
#include "loader_environment.h"
#include "log.h"
#include "settings.h"
#include "vk_loader_extensions.h"
#include "vk_loader_platform.h"
#include "wsi.h"

// Trampoline entrypoints are in this file for core Vulkan commands

/* vkGetInstanceProcAddr: Get global level or instance level entrypoint addresses.
 * @param instance
 * @param pName
 * @return
 *    If pName is a global level entrypoint:
 *        If instance == NULL || instance is invalid || (instance is valid && instance.minor_version <= 2):
 *            return global level functions
 *        Else:
 *            return NULL
 *    Else:
 *        If instance is valid:
 *            return a trampoline entry point for all dispatchable Vulkan functions both core and extensions.
 *        Else:
 *            return NULL
 *
 * Note:
 * Vulkan header updated 1.2.193 changed the behavior of vkGetInstanceProcAddr for global entrypoints. They used to always be
 * returned regardless of the value of the instance parameter. The spec was amended in this version to only allow querying global
 * level entrypoints with a NULL instance. However, as to not break old applications, the new behavior is only applied if the
 * instance passed in is both valid and minor version is greater than 1.2, which was when this change in behavior occurred. Only
 * instances with a newer version will get the new behavior.
 */
LOADER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *pName) {
    // Always should be able to get vkGetInstanceProcAddr if queried, regardless of the value of instance
    if (!strcmp(pName, "vkGetInstanceProcAddr")) return (PFN_vkVoidFunction)vkGetInstanceProcAddr;

    // Get entrypoint addresses that are global (no dispatchable object)
    void *addr = globalGetProcAddr(pName);
    if (addr != VK_NULL_HANDLE) {
        // Always can get a global entrypoint from vkGetInstanceProcAddr with a NULL instance handle
        if (instance == VK_NULL_HANDLE) {
            return addr;
        } else {
            // New behavior only returns a global entrypoint if the instance handle is NULL.
            // Old behavior is to return a global entrypoint regardless of the value of the instance handle.
            // Use new behavior if: The instance is valid and the minor version of the instance is greater than 1.2, which
            // was when the new behavior was added. (eg, it is enforced in the next minor version of vulkan, which will be 1.3)

            // First check if instance is valid - loader_get_instance() returns NULL if it isn't.
            struct loader_instance *ptr_instance = loader_get_instance(instance);
            if (ptr_instance != NULL &&
                loader_check_version_meets_required(loader_combine_version(1, 3, 0), ptr_instance->app_api_version)) {
                // New behavior
                return NULL;
            } else {
                // Old behavior
                return addr;
            }
        }
    } else {
        // All other functions require a valid instance handle to get
        if (instance == VK_NULL_HANDLE) {
            return NULL;
        }
        struct loader_instance *ptr_instance = loader_get_instance(instance);
        // If we've gotten here and the pointer is NULL, it's invalid
        if (ptr_instance == NULL) {
            loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                       "vkGetInstanceProcAddr: Invalid instance [VUID-vkGetInstanceProcAddr-instance-parameter]");
            abort(); /* Intentionally fail so user can correct issue. */
        }
        // Return trampoline code for non-global entrypoints including any extensions.
        // Device extensions are returned if a layer or ICD supports the extension.
        // Instance extensions are returned if the extension is enabled and the
        // loader or someone else supports the extension
        return trampoline_get_proc_addr(ptr_instance, pName);
    }
}

// Get a device level or global level entry point address.
// @param device
// @param pName
// @return
//    If device is valid, returns a device relative entry point for device level
//    entry points both core and extensions.
//    Device relative means call down the device chain.
LOADER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char *pName) {
    if (!pName || pName[0] != 'v' || pName[1] != 'k') return NULL;

    // For entrypoints that loader must handle (ie non-dispatchable or create object)
    // make sure the loader entrypoint is returned
    const char *name = pName;
    name += 2;
    if (!strcmp(name, "GetDeviceProcAddr")) return (PFN_vkVoidFunction)vkGetDeviceProcAddr;
    if (!strcmp(name, "DestroyDevice")) return (PFN_vkVoidFunction)vkDestroyDevice;
    if (!strcmp(name, "GetDeviceQueue")) return (PFN_vkVoidFunction)vkGetDeviceQueue;
    if (!strcmp(name, "AllocateCommandBuffers")) return (PFN_vkVoidFunction)vkAllocateCommandBuffers;

    // Although CreateDevice is on device chain it's dispatchable object isn't
    // a VkDevice or child of VkDevice so return NULL.
    if (!strcmp(pName, "CreateDevice")) return NULL;

    // Because vkGetDeviceQueue2 is a 1.1 entry point, we need to check if the apiVersion provided during instance creation is
    // sufficient
    if (!strcmp(name, "GetDeviceQueue2")) {
        struct loader_device *dev = NULL;
        struct loader_icd_term *icd_term = loader_get_icd_and_device(device, &dev);
        if (NULL != icd_term && dev != NULL) {
            const struct loader_instance *inst = icd_term->this_instance;
            uint32_t api_version =
                VK_MAKE_API_VERSION(0, inst->app_api_version.major, inst->app_api_version.minor, inst->app_api_version.patch);
            return (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1)
                       ? NULL
                       : (PFN_vkVoidFunction)vkGetDeviceQueue2;
        }
        return NULL;
    }
    // Return the dispatch table entrypoint for the fastest case
    const VkLayerDispatchTable *disp_table = loader_get_dispatch(device);
    if (disp_table == NULL) return NULL;

    bool found_name = false;
    void *addr = loader_lookup_device_dispatch_table(disp_table, pName, &found_name);
    if (found_name) return addr;

    if (disp_table->GetDeviceProcAddr == NULL) return NULL;
    return disp_table->GetDeviceProcAddr(device, pName);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName,
                                                                                    uint32_t *pPropertyCount,
                                                                                    VkExtensionProperties *pProperties) {
    LOADER_PLATFORM_THREAD_ONCE(&once_init, loader_initialize);

    update_global_loader_settings();

    // We know we need to call at least the terminator
    VkResult res = VK_SUCCESS;
    VkEnumerateInstanceExtensionPropertiesChain chain_tail = {
        .header =
            {
                .type = VK_CHAIN_TYPE_ENUMERATE_INSTANCE_EXTENSION_PROPERTIES,
                .version = VK_CURRENT_CHAIN_VERSION,
                .size = sizeof(chain_tail),
            },
        .pfnNextLayer = &terminator_pre_instance_EnumerateInstanceExtensionProperties,
        .pNextLink = NULL,
    };
    VkEnumerateInstanceExtensionPropertiesChain *chain_head = &chain_tail;

    // Get the implicit layers
    struct loader_layer_list layers = {0};
    memset(&layers, 0, sizeof(layers));
    struct loader_envvar_all_filters layer_filters = {0};

    res = parse_layer_environment_var_filters(NULL, &layer_filters);
    if (VK_SUCCESS != res) {
        return res;
    }

    res = loader_scan_for_implicit_layers(NULL, &layers, &layer_filters);
    if (VK_SUCCESS != res) {
        return res;
    }

    // Prepend layers onto the chain if they implement this entry point
    for (uint32_t i = 0; i < layers.count; ++i) {
        // Skip this layer if it doesn't expose the entry-point
        if (NULL == layers.list[i].pre_instance_functions.enumerate_instance_extension_properties) {
            continue;
        }

        loader_open_layer_file(NULL, &layers.list[i]);
        if (layers.list[i].lib_handle == NULL) {
            continue;
        }

        void *pfn = loader_platform_get_proc_address(layers.list[i].lib_handle,
                                                     layers.list[i].pre_instance_functions.enumerate_instance_extension_properties);
        if (pfn == NULL) {
            loader_log(NULL, VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_LAYER_BIT, 0,
                       "%s: Unable to resolve symbol \"%s\" in implicit layer library \"%s\"", __FUNCTION__,
                       layers.list[i].pre_instance_functions.enumerate_instance_extension_properties, layers.list[i].lib_name);
            continue;
        }

        VkEnumerateInstanceExtensionPropertiesChain *chain_link =
            loader_alloc(NULL, sizeof(VkEnumerateInstanceExtensionPropertiesChain), VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        if (chain_link == NULL) {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            break;
        }
        memset(chain_link, 0, sizeof(VkEnumerateInstanceLayerPropertiesChain));
        chain_link->header.type = VK_CHAIN_TYPE_ENUMERATE_INSTANCE_EXTENSION_PROPERTIES;
        chain_link->header.version = VK_CURRENT_CHAIN_VERSION;
        chain_link->header.size = sizeof(*chain_link);
        chain_link->pfnNextLayer = pfn;
        chain_link->pNextLink = chain_head;

        chain_head = chain_link;
    }

    // Call down the chain
    if (res == VK_SUCCESS) {
        res = chain_head->pfnNextLayer(chain_head->pNextLink, pLayerName, pPropertyCount, pProperties);
    }

    // Free up the layers
    loader_delete_layer_list_and_properties(NULL, &layers);

    // Tear down the chain
    while (chain_head != &chain_tail) {
        VkEnumerateInstanceExtensionPropertiesChain *holder = chain_head;
        chain_head = (VkEnumerateInstanceExtensionPropertiesChain *)chain_head->pNextLink;
        loader_free(NULL, holder);
    }

    return res;
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                                                                VkLayerProperties *pProperties) {
    LOADER_PLATFORM_THREAD_ONCE(&once_init, loader_initialize);

    update_global_loader_settings();

    // We know we need to call at least the terminator
    VkResult res = VK_SUCCESS;
    VkEnumerateInstanceLayerPropertiesChain chain_tail = {
        .header =
            {
                .type = VK_CHAIN_TYPE_ENUMERATE_INSTANCE_LAYER_PROPERTIES,
                .version = VK_CURRENT_CHAIN_VERSION,
                .size = sizeof(chain_tail),
            },
        .pfnNextLayer = &terminator_pre_instance_EnumerateInstanceLayerProperties,
        .pNextLink = NULL,
    };
    VkEnumerateInstanceLayerPropertiesChain *chain_head = &chain_tail;

    // Get the implicit layers
    struct loader_layer_list layers;
    memset(&layers, 0, sizeof(layers));
    struct loader_envvar_all_filters layer_filters = {0};

    res = parse_layer_environment_var_filters(NULL, &layer_filters);
    if (VK_SUCCESS != res) {
        return res;
    }

    res = loader_scan_for_implicit_layers(NULL, &layers, &layer_filters);
    if (VK_SUCCESS != res) {
        return res;
    }

    // Prepend layers onto the chain if they implement this entry point
    for (uint32_t i = 0; i < layers.count; ++i) {
        // Skip this layer if it doesn't expose the entry-point
        if (NULL == layers.list[i].pre_instance_functions.enumerate_instance_layer_properties) {
            continue;
        }

        loader_open_layer_file(NULL, &layers.list[i]);
        if (layers.list[i].lib_handle == NULL) {
            continue;
        }

        void *pfn = loader_platform_get_proc_address(layers.list[i].lib_handle,
                                                     layers.list[i].pre_instance_functions.enumerate_instance_layer_properties);
        if (pfn == NULL) {
            loader_log(NULL, VULKAN_LOADER_WARN_BIT, 0, "%s: Unable to resolve symbol \"%s\" in implicit layer library \"%s\"",
                       __FUNCTION__, layers.list[i].pre_instance_functions.enumerate_instance_layer_properties,
                       layers.list[i].lib_name);
            continue;
        }

        VkEnumerateInstanceLayerPropertiesChain *chain_link =
            loader_alloc(NULL, sizeof(VkEnumerateInstanceLayerPropertiesChain), VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        if (chain_link == NULL) {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            break;
        }
        memset(chain_link, 0, sizeof(VkEnumerateInstanceLayerPropertiesChain));
        chain_link->header.type = VK_CHAIN_TYPE_ENUMERATE_INSTANCE_LAYER_PROPERTIES;
        chain_link->header.version = VK_CURRENT_CHAIN_VERSION;
        chain_link->header.size = sizeof(*chain_link);
        chain_link->pfnNextLayer = pfn;
        chain_link->pNextLink = chain_head;

        chain_head = chain_link;
    }

    // Call down the chain
    if (res == VK_SUCCESS) {
        res = chain_head->pfnNextLayer(chain_head->pNextLink, pPropertyCount, pProperties);
    }

    // Free up the layers
    loader_delete_layer_list_and_properties(NULL, &layers);

    // Tear down the chain
    while (chain_head != &chain_tail) {
        VkEnumerateInstanceLayerPropertiesChain *holder = chain_head;
        chain_head = (VkEnumerateInstanceLayerPropertiesChain *)chain_head->pNextLink;
        loader_free(NULL, holder);
    }

    return res;
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceVersion(uint32_t *pApiVersion) {
    LOADER_PLATFORM_THREAD_ONCE(&once_init, loader_initialize);

    update_global_loader_settings();

    if (NULL == pApiVersion) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkEnumerateInstanceVersion: \'pApiVersion\' must not be NULL "
                   "(VUID-vkEnumerateInstanceVersion-pApiVersion-parameter");
        // NOTE: This seems silly, but it's the only allowable failure
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    // We know we need to call at least the terminator
    VkResult res = VK_SUCCESS;
    VkEnumerateInstanceVersionChain chain_tail = {
        .header =
            {
                .type = VK_CHAIN_TYPE_ENUMERATE_INSTANCE_VERSION,
                .version = VK_CURRENT_CHAIN_VERSION,
                .size = sizeof(chain_tail),
            },
        .pfnNextLayer = &terminator_pre_instance_EnumerateInstanceVersion,
        .pNextLink = NULL,
    };
    VkEnumerateInstanceVersionChain *chain_head = &chain_tail;

    // Get the implicit layers
    struct loader_layer_list layers;
    memset(&layers, 0, sizeof(layers));
    struct loader_envvar_all_filters layer_filters = {0};

    res = parse_layer_environment_var_filters(NULL, &layer_filters);
    if (VK_SUCCESS != res) {
        return res;
    }

    res = loader_scan_for_implicit_layers(NULL, &layers, &layer_filters);
    if (VK_SUCCESS != res) {
        return res;
    }

    // Prepend layers onto the chain if they implement this entry point
    for (uint32_t i = 0; i < layers.count; ++i) {
        // Skip this layer if it doesn't expose the entry-point
        if (NULL == layers.list[i].pre_instance_functions.enumerate_instance_version) {
            continue;
        }

        loader_open_layer_file(NULL, &layers.list[i]);
        if (layers.list[i].lib_handle == NULL) {
            continue;
        }

        void *pfn = loader_platform_get_proc_address(layers.list[i].lib_handle,
                                                     layers.list[i].pre_instance_functions.enumerate_instance_version);
        if (pfn == NULL) {
            loader_log(NULL, VULKAN_LOADER_WARN_BIT, 0, "%s: Unable to resolve symbol \"%s\" in implicit layer library \"%s\"",
                       __FUNCTION__, layers.list[i].pre_instance_functions.enumerate_instance_version, layers.list[i].lib_name);
            continue;
        }

        VkEnumerateInstanceVersionChain *chain_link =
            loader_alloc(NULL, sizeof(VkEnumerateInstanceVersionChain), VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        if (chain_link == NULL) {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            break;
        }
        memset(chain_link, 0, sizeof(VkEnumerateInstanceLayerPropertiesChain));
        chain_link->header.type = VK_CHAIN_TYPE_ENUMERATE_INSTANCE_VERSION;
        chain_link->header.version = VK_CURRENT_CHAIN_VERSION;
        chain_link->header.size = sizeof(*chain_link);
        chain_link->pfnNextLayer = pfn;
        chain_link->pNextLink = chain_head;

        chain_head = chain_link;
    }

    // Call down the chain
    if (res == VK_SUCCESS) {
        res = chain_head->pfnNextLayer(chain_head->pNextLink, pApiVersion);
    }

    // Free up the layers
    loader_delete_layer_list_and_properties(NULL, &layers);

    // Tear down the chain
    while (chain_head != &chain_tail) {
        VkEnumerateInstanceVersionChain *holder = chain_head;
        chain_head = (VkEnumerateInstanceVersionChain *)chain_head->pNextLink;
        loader_free(NULL, holder);
    }

    return res;
}

// Add the "instance-only" debug functions to the list of active debug functions
// at the very end.  This way it doesn't get replaced by any new messengers
void loader_add_instance_only_debug_funcs(struct loader_instance *ptr_instance) {
    VkLayerDbgFunctionNode *cur_node = ptr_instance->current_dbg_function_head;
    if (cur_node == NULL) {
        ptr_instance->current_dbg_function_head = ptr_instance->instance_only_dbg_function_head;
    } else {
        while (cur_node != NULL) {
            if (cur_node == ptr_instance->instance_only_dbg_function_head) {
                // Already added
                break;
            }
            // Last item
            else if (cur_node->pNext == NULL) {
                cur_node->pNext = ptr_instance->instance_only_dbg_function_head;
            }
            cur_node = cur_node->pNext;
        }
    }
}

// Remove the "instance-only" debug functions from the list of active debug functions.
// It should be added after the last actual debug utils/debug report function.
void loader_remove_instance_only_debug_funcs(struct loader_instance *ptr_instance) {
    VkLayerDbgFunctionNode *cur_node = ptr_instance->current_dbg_function_head;

    // Only thing in list is the instance only head
    if (cur_node == ptr_instance->instance_only_dbg_function_head) {
        ptr_instance->current_dbg_function_head = NULL;
    }
    while (cur_node != NULL) {
        if (cur_node->pNext == ptr_instance->instance_only_dbg_function_head) {
            cur_node->pNext = NULL;
            break;
        }
        cur_node = cur_node->pNext;
    }
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator, VkInstance *pInstance) {
    struct loader_instance *ptr_instance = NULL;
    VkInstance created_instance = VK_NULL_HANDLE;
    VkResult res = VK_ERROR_INITIALIZATION_FAILED;
    VkInstanceCreateInfo ici = {0};
    bool portability_enumeration_flag_bit_set = false;
    bool portability_enumeration_extension_enabled = false;
    struct loader_envvar_all_filters layer_filters = {0};

    LOADER_PLATFORM_THREAD_ONCE(&once_init, loader_initialize);

    if (pCreateInfo == NULL) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateInstance: \'pCreateInfo\' is NULL (VUID-vkCreateInstance-pCreateInfo-parameter)");
        goto out;
    }
    ici = *pCreateInfo;

    if (pInstance == NULL) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateInstance \'pInstance\' not valid (VUID-vkCreateInstance-pInstance-parameter)");
        goto out;
    }

    ptr_instance =
        (struct loader_instance *)loader_calloc(pAllocator, sizeof(struct loader_instance), VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

    if (ptr_instance == NULL) {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    loader_platform_thread_lock_mutex(&loader_lock);
    if (pAllocator) {
        ptr_instance->alloc_callbacks = *pAllocator;
    }
    ptr_instance->magic = LOADER_MAGIC_NUMBER;

    // Save the application version
    if (NULL == pCreateInfo->pApplicationInfo || 0 == pCreateInfo->pApplicationInfo->apiVersion) {
        ptr_instance->app_api_version = LOADER_VERSION_1_0_0;
    } else {
        ptr_instance->app_api_version = loader_make_version(pCreateInfo->pApplicationInfo->apiVersion);
    }

    // Look for one or more VK_EXT_debug_report or VK_EXT_debug_utils create info structures
    // and setup a callback(s) for each one found.

    // Handle cases of VK_EXT_debug_utils
    // Setup the temporary messenger(s) here to catch early issues:
    res = util_CreateDebugUtilsMessengers(ptr_instance, pCreateInfo->pNext, pAllocator);
    if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
        // Failure of setting up one or more of the messenger.
        goto out;
    }

    // Handle cases of VK_EXT_debug_report
    // Setup the temporary callback(s) here to catch early issues:
    res = util_CreateDebugReportCallbacks(ptr_instance, pCreateInfo->pNext, pAllocator);
    if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
        // Failure of setting up one or more of the callback.
        goto out;
    }

    VkResult settings_file_res = get_loader_settings(ptr_instance, &ptr_instance->settings);
    if (settings_file_res == VK_ERROR_OUT_OF_HOST_MEMORY) {
        res = settings_file_res;
        goto out;
    }
    if (ptr_instance->settings.settings_active) {
        log_settings(ptr_instance, &ptr_instance->settings);
    }

    // Providing an apiVersion less than VK_API_VERSION_1_0 but greater than zero prevents the validation layers from starting
    if (pCreateInfo->pApplicationInfo && pCreateInfo->pApplicationInfo->apiVersion != 0u &&
        pCreateInfo->pApplicationInfo->apiVersion < VK_API_VERSION_1_0) {
        loader_log(ptr_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "VkInstanceCreateInfo::pApplicationInfo::apiVersion has value of %u which is not permitted. If apiVersion is "
                   "not 0, then it must be "
                   "greater than or equal to the value of VK_API_VERSION_1_0 [VUID-VkApplicationInfo-apiVersion]",
                   pCreateInfo->pApplicationInfo->apiVersion);
    }

    // Check the VkInstanceCreateInfoFlags wether to allow the portability enumeration flag
    if ((pCreateInfo->flags & VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR) == 1) {
        portability_enumeration_flag_bit_set = true;
    }
    // Make sure the portability extension extension has been enabled before enabling portability driver enumeration
    if (pCreateInfo->ppEnabledExtensionNames) {
        for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
            if (strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0) {
                portability_enumeration_extension_enabled = true;
                if (portability_enumeration_flag_bit_set) {
                    ptr_instance->portability_enumeration_enabled = true;
                    loader_log(ptr_instance, VULKAN_LOADER_INFO_BIT, 0,
                               "Portability enumeration bit was set, enumerating portability drivers.");
                }
                break;
            }
        }
    }

    // Make sure the application provided API version has the correct variant
    if (NULL != pCreateInfo->pApplicationInfo) {
        uint32_t variant_version = VK_API_VERSION_VARIANT(pCreateInfo->pApplicationInfo->apiVersion);
        const uint32_t expected_variant = 0;
        if (expected_variant != variant_version) {
            loader_log(ptr_instance, VULKAN_LOADER_WARN_BIT, 0,
                       "vkCreateInstance: The API Variant specified in pCreateInfo->pApplicationInfo.apiVersion is %d instead of "
                       "the expected value of %d.",
                       variant_version, expected_variant);
        }
    }

    res = parse_layer_environment_var_filters(ptr_instance, &layer_filters);
    if (VK_SUCCESS != res) {
        goto out;
    }

    // Due to implicit layers need to get layer list even if
    // enabledLayerCount == 0 and VK_INSTANCE_LAYERS is unset. For now always
    // get layer list via loader_scan_for_layers().
    memset(&ptr_instance->instance_layer_list, 0, sizeof(ptr_instance->instance_layer_list));
    res = loader_scan_for_layers(ptr_instance, &ptr_instance->instance_layer_list, &layer_filters);
    if (VK_SUCCESS != res) {
        goto out;
    }

    // Validate the app requested layers to be enabled
    if (pCreateInfo->enabledLayerCount > 0) {
        res = loader_validate_layers(ptr_instance, pCreateInfo->enabledLayerCount, pCreateInfo->ppEnabledLayerNames,
                                     &ptr_instance->instance_layer_list);
        if (res != VK_SUCCESS) {
            goto out;
        }
    }

    // Scan/discover all System and Environment Variable ICD libraries
    bool skipped_portability_drivers = false;
    res = loader_icd_scan(ptr_instance, &ptr_instance->icd_tramp_list, pCreateInfo, &skipped_portability_drivers);
    if (res == VK_ERROR_OUT_OF_HOST_MEMORY) {
        goto out;
    }

    if (ptr_instance->icd_tramp_list.count == 0) {
        // No drivers found
        if (skipped_portability_drivers) {
            if (portability_enumeration_extension_enabled && !portability_enumeration_flag_bit_set) {
                loader_log(ptr_instance, VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_DRIVER_BIT, 0,
                           "vkCreateInstance: Found drivers that contain devices which support the portability subset, but "
                           "the instance does not enumerate portability drivers! Applications that wish to enumerate portability "
                           "drivers must set the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR bit in the VkInstanceCreateInfo "
                           "flags.");
            } else if (portability_enumeration_flag_bit_set && !portability_enumeration_extension_enabled) {
                loader_log(ptr_instance, VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_DRIVER_BIT, 0,
                           "VkInstanceCreateInfo: If flags has the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR bit set, the "
                           "list of enabled extensions in ppEnabledExtensionNames must contain VK_KHR_portability_enumeration "
                           "[VUID-VkInstanceCreateInfo-flags-06559 ]"
                           "Applications that wish to enumerate portability drivers must enable the VK_KHR_portability_enumeration "
                           "instance extension.");
            } else {
                loader_log(ptr_instance, VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_DRIVER_BIT, 0,
                           "vkCreateInstance: Found drivers that contain devices which support the portability subset, but "
                           "the instance does not enumerate portability drivers! Applications that wish to enumerate portability "
                           "drivers must set the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR bit in the VkInstanceCreateInfo "
                           "flags and enable the VK_KHR_portability_enumeration instance extension.");
            }
        }
        loader_log(ptr_instance, VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_DRIVER_BIT, 0, "vkCreateInstance: Found no drivers!");
        res = VK_ERROR_INCOMPATIBLE_DRIVER;
        goto out;
    }

    // Get extensions from all ICD's, merge so no duplicates, then validate
    res = loader_get_icd_loader_instance_extensions(ptr_instance, &ptr_instance->icd_tramp_list, &ptr_instance->ext_list);
    if (res != VK_SUCCESS) {
        goto out;
    }
    res = loader_validate_instance_extensions(ptr_instance, &ptr_instance->ext_list, &ptr_instance->instance_layer_list,
                                              &layer_filters, &ici);
    if (res != VK_SUCCESS) {
        goto out;
    }

    ptr_instance->disp = loader_instance_heap_alloc(ptr_instance, sizeof(struct loader_instance_dispatch_table),
                                                    VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (ptr_instance->disp == NULL) {
        loader_log(ptr_instance, VULKAN_LOADER_ERROR_BIT, 0,
                   "vkCreateInstance:  Failed to allocate Loader's full Instance dispatch table.");
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }
    memcpy(&ptr_instance->disp->layer_inst_disp, &instance_disp, sizeof(instance_disp));

    loader_platform_thread_lock_mutex(&loader_global_instance_list_lock);
    ptr_instance->next = loader.instances;
    loader.instances = ptr_instance;
    loader_platform_thread_unlock_mutex(&loader_global_instance_list_lock);

    // Activate any layers on instance chain
    res = loader_enable_instance_layers(ptr_instance, &ici, &ptr_instance->instance_layer_list, &layer_filters);
    if (res != VK_SUCCESS) {
        goto out;
    }

    created_instance = (VkInstance)ptr_instance;
    res = loader_create_instance_chain(&ici, pAllocator, ptr_instance, &created_instance);

    if (VK_SUCCESS == res) {
        // Check for enabled extensions here to setup the loader structures so the loader knows what extensions
        // it needs to worry about.
        // We do it in the terminator and again above the layers here since we may think different extensions
        // are enabled than what's down in the terminator.
        // This is why we don't clear inside of these function calls.
        // The clearing should actually be handled by the overall memset of the pInstance structure above.
        fill_out_enabled_instance_extensions(ici.enabledExtensionCount, ici.ppEnabledExtensionNames,
                                             &ptr_instance->enabled_extensions);

        *pInstance = (VkInstance)ptr_instance;

        // Finally have the layers in place and everyone has seen
        // the CreateInstance command go by. This allows the layer's
        // GetInstanceProcAddr functions to return valid extension functions
        // if enabled.
        loader_activate_instance_layer_extensions(ptr_instance, created_instance);
        ptr_instance->instance_finished_creation = true;
    } else if (VK_ERROR_EXTENSION_NOT_PRESENT == res && !ptr_instance->create_terminator_invalid_extension) {
        loader_log(ptr_instance, VULKAN_LOADER_WARN_BIT, 0,
                   "vkCreateInstance: Layer returning invalid extension error not triggered by ICD/Loader (Policy #LLP_LAYER_17).");
    }

out:

    if (NULL != ptr_instance) {
        if (res != VK_SUCCESS) {
            loader_platform_thread_lock_mutex(&loader_global_instance_list_lock);
            // error path, should clean everything up
            if (loader.instances == ptr_instance) {
                loader.instances = ptr_instance->next;
            }
            loader_platform_thread_unlock_mutex(&loader_global_instance_list_lock);

            free_loader_settings(ptr_instance, &ptr_instance->settings);

            loader_destroy_generic_list(ptr_instance, (struct loader_generic_list *)&ptr_instance->surfaces_list);
            loader_destroy_generic_list(ptr_instance, (struct loader_generic_list *)&ptr_instance->debug_utils_messengers_list);
            loader_destroy_generic_list(ptr_instance, (struct loader_generic_list *)&ptr_instance->debug_report_callbacks_list);

            loader_instance_heap_free(ptr_instance, ptr_instance->disp);
            // Remove any created VK_EXT_debug_report or VK_EXT_debug_utils items
            destroy_debug_callbacks_chain(ptr_instance, pAllocator);

            loader_destroy_pointer_layer_list(ptr_instance, &ptr_instance->expanded_activated_layer_list);
            loader_destroy_pointer_layer_list(ptr_instance, &ptr_instance->app_activated_layer_list);

            loader_delete_layer_list_and_properties(ptr_instance, &ptr_instance->instance_layer_list);
            loader_clear_scanned_icd_list(ptr_instance, &ptr_instance->icd_tramp_list);
            loader_destroy_generic_list(ptr_instance, (struct loader_generic_list *)&ptr_instance->ext_list);

            // Free any icd_terms that were created.
            // If an OOM occurs from a layer, terminator_CreateInstance won't be reached where this kind of
            // cleanup normally occurs
            struct loader_icd_term *icd_term = NULL;
            while (NULL != ptr_instance->icd_terms) {
                icd_term = ptr_instance->icd_terms;
                // Call destroy Instance on each driver in case we successfully called down the chain but failed on
                // our way back out of it.
                if (icd_term->instance) {
                    loader_icd_close_objects(ptr_instance, icd_term);
                    icd_term->dispatch.DestroyInstance(icd_term->instance, pAllocator);
                }
                icd_term->instance = VK_NULL_HANDLE;
                ptr_instance->icd_terms = icd_term->next;
                loader_icd_destroy(ptr_instance, icd_term, pAllocator);
            }

            free_string_list(ptr_instance, &ptr_instance->enabled_layer_names);

            loader_instance_heap_free(ptr_instance, ptr_instance);
        } else {
            // success path, swap out created debug callbacks out so they aren't used until instance destruction
            loader_remove_instance_only_debug_funcs(ptr_instance);
        }
        // Only unlock when ptr_instance isn't NULL, as if it is, the above code didn't make it to when loader_lock was locked.
        loader_platform_thread_unlock_mutex(&loader_lock);
    }

    return res;
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    const VkLayerInstanceDispatchTable *disp;
    struct loader_instance *ptr_instance = NULL;

    if (instance == VK_NULL_HANDLE) {
        return;
    }
    loader_platform_thread_lock_mutex(&loader_lock);

    ptr_instance = loader_get_instance(instance);
    if (ptr_instance == NULL) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyInstance: Invalid instance [VUID-vkDestroyInstance-instance-parameter]");
        loader_platform_thread_unlock_mutex(&loader_lock);
        abort(); /* Intentionally fail so user can correct issue. */
    }

    if (pAllocator) {
        ptr_instance->alloc_callbacks = *pAllocator;
    }

    // Remove any callbacks that weren't cleaned up by the application
    destroy_debug_callbacks_chain(ptr_instance, pAllocator);

    // Swap in the debug callbacks created during instance creation
    loader_add_instance_only_debug_funcs(ptr_instance);

    disp = loader_get_instance_layer_dispatch(instance);
    disp->DestroyInstance(ptr_instance->instance, pAllocator);

    free_loader_settings(ptr_instance, &ptr_instance->settings);

    loader_destroy_generic_list(ptr_instance, (struct loader_generic_list *)&ptr_instance->surfaces_list);
    loader_destroy_generic_list(ptr_instance, (struct loader_generic_list *)&ptr_instance->debug_utils_messengers_list);
    loader_destroy_generic_list(ptr_instance, (struct loader_generic_list *)&ptr_instance->debug_report_callbacks_list);

    loader_destroy_pointer_layer_list(ptr_instance, &ptr_instance->expanded_activated_layer_list);
    loader_destroy_pointer_layer_list(ptr_instance, &ptr_instance->app_activated_layer_list);

    loader_delete_layer_list_and_properties(ptr_instance, &ptr_instance->instance_layer_list);

    free_string_list(ptr_instance, &ptr_instance->enabled_layer_names);

    if (ptr_instance->phys_devs_tramp) {
        for (uint32_t i = 0; i < ptr_instance->phys_dev_count_tramp; i++) {
            loader_instance_heap_free(ptr_instance, ptr_instance->phys_devs_tramp[i]);
        }
        loader_instance_heap_free(ptr_instance, ptr_instance->phys_devs_tramp);
    }

    // Destroy the debug callbacks created during instance creation
    destroy_debug_callbacks_chain(ptr_instance, pAllocator);

    loader_instance_heap_free(ptr_instance, ptr_instance->disp);
    loader_instance_heap_free(ptr_instance, ptr_instance);
    loader_platform_thread_unlock_mutex(&loader_lock);

    // Unload preloaded layers, so if vkEnumerateInstanceExtensionProperties or vkCreateInstance is called again, the ICD's are
    // up to date
    loader_unload_preloaded_icds();
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                                        VkPhysicalDevice *pPhysicalDevices) {
    VkResult res = VK_SUCCESS;
    struct loader_instance *inst;

    loader_platform_thread_lock_mutex(&loader_lock);

    inst = loader_get_instance(instance);
    if (NULL == inst) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkEnumeratePhysicalDevices: Invalid instance [VUID-vkEnumeratePhysicalDevices-instance-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    if (NULL == pPhysicalDeviceCount) {
        loader_log(inst, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkEnumeratePhysicalDevices: Received NULL pointer for physical device count return value. "
                   "[VUID-vkEnumeratePhysicalDevices-pPhysicalDeviceCount-parameter]");
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }

    // Call down the chain to get the physical device info
    res = inst->disp->layer_inst_disp.EnumeratePhysicalDevices(inst->instance, pPhysicalDeviceCount, pPhysicalDevices);

    if (NULL != pPhysicalDevices && (VK_SUCCESS == res || VK_INCOMPLETE == res)) {
        // Wrap the PhysDev object for loader usage, return wrapped objects
        VkResult update_res = setup_loader_tramp_phys_devs(inst, *pPhysicalDeviceCount, pPhysicalDevices);
        if (VK_SUCCESS != update_res) {
            res = update_res;
        }

        // Unloads any drivers that do not expose any physical devices - should save some address space
        unload_drivers_without_physical_devices(inst);
    }

out:

    loader_platform_thread_unlock_mutex(&loader_lock);

    return res;
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                                                     VkPhysicalDeviceFeatures *pFeatures) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(
            NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
            "vkGetPhysicalDeviceFeatures: Invalid physicalDevice [VUID-vkGetPhysicalDeviceFeatures-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    disp->GetPhysicalDeviceFeatures(unwrapped_phys_dev, pFeatures);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                             VkFormatProperties *pFormatInfo) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceFormatProperties: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceFormatProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    disp->GetPhysicalDeviceFormatProperties(unwrapped_phys_dev, format, pFormatInfo);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkImageFormatProperties *pImageFormatProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceImageFormatProperties: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceImageFormatProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceImageFormatProperties(unwrapped_phys_dev, format, type, tiling, usage, flags,
                                                        pImageFormatProperties);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                                       VkPhysicalDeviceProperties *pProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceProperties: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    disp->GetPhysicalDeviceProperties(unwrapped_phys_dev, pProperties);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                                  uint32_t *pQueueFamilyPropertyCount,
                                                                                  VkQueueFamilyProperties *pQueueProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceQueueFamilyProperties: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceQueueFamilyProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    disp->GetPhysicalDeviceQueueFamilyProperties(unwrapped_phys_dev, pQueueFamilyPropertyCount, pQueueProperties);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                                             VkPhysicalDeviceMemoryProperties *pMemoryProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceMemoryProperties: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceMemoryProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    disp->GetPhysicalDeviceMemoryProperties(unwrapped_phys_dev, pMemoryProperties);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    if (VK_NULL_HANDLE == loader_unwrap_physical_device(physicalDevice)) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateDevice: Invalid physicalDevice [VUID-vkCreateDevice-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    loader_platform_thread_lock_mutex(&loader_lock);
    VkResult res = loader_layer_create_device(NULL, physicalDevice, pCreateInfo, pAllocator, pDevice, NULL, NULL);
    loader_platform_thread_unlock_mutex(&loader_lock);
    return res;
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp;

    if (device == VK_NULL_HANDLE) {
        return;
    }
    disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyDevice: Invalid device [VUID-vkDestroyDevice-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    loader_platform_thread_lock_mutex(&loader_lock);

    loader_layer_destroy_device(device, pAllocator, disp->DestroyDevice);

    loader_platform_thread_unlock_mutex(&loader_lock);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                  const char *pLayerName, uint32_t *pPropertyCount,
                                                                                  VkExtensionProperties *pProperties) {
    VkResult res = VK_SUCCESS;
    struct loader_physical_device_tramp *phys_dev;
    const VkLayerInstanceDispatchTable *disp;
    phys_dev = (struct loader_physical_device_tramp *)physicalDevice;
    if (VK_NULL_HANDLE == physicalDevice || PHYS_TRAMP_MAGIC_NUMBER != phys_dev->magic) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkEnumerateDeviceExtensionProperties: Invalid physicalDevice "
                   "[VUID-vkEnumerateDeviceExtensionProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    loader_platform_thread_lock_mutex(&loader_lock);

    // always pass this call down the instance chain which will terminate
    // in the ICD. This allows layers to filter the extensions coming back
    // up the chain. In the terminator we look up layer extensions from the
    // manifest file if it wasn't provided by the layer itself.
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    res = disp->EnumerateDeviceExtensionProperties(phys_dev->phys_dev, pLayerName, pPropertyCount, pProperties);

    loader_platform_thread_unlock_mutex(&loader_lock);
    return res;
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
                                                                              uint32_t *pPropertyCount,
                                                                              VkLayerProperties *pProperties) {
    uint32_t copy_size;
    struct loader_physical_device_tramp *phys_dev;
    loader_platform_thread_lock_mutex(&loader_lock);

    // Don't dispatch this call down the instance chain, want all device layers
    // enumerated and instance chain may not contain all device layers
    // TODO re-evaluate the above statement we maybe able to start calling
    // down the chain

    phys_dev = (struct loader_physical_device_tramp *)physicalDevice;
    if (VK_NULL_HANDLE == physicalDevice || PHYS_TRAMP_MAGIC_NUMBER != phys_dev->magic) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkEnumerateDeviceLayerProperties: Invalid physicalDevice "
                   "[VUID-vkEnumerateDeviceLayerProperties-physicalDevice-parameter]");
        loader_platform_thread_unlock_mutex(&loader_lock);
        abort(); /* Intentionally fail so user can correct issue. */
    }

    const struct loader_instance *inst = phys_dev->this_instance;

    uint32_t count = inst->app_activated_layer_list.count;
    if (count == 0 || pProperties == NULL) {
        *pPropertyCount = count;
        loader_platform_thread_unlock_mutex(&loader_lock);
        return VK_SUCCESS;
    }

    copy_size = (*pPropertyCount < count) ? *pPropertyCount : count;
    for (uint32_t i = 0; i < copy_size; i++) {
        memcpy(&pProperties[i], &(inst->app_activated_layer_list.list[i]->info), sizeof(VkLayerProperties));
    }
    *pPropertyCount = copy_size;

    if (copy_size < count) {
        loader_platform_thread_unlock_mutex(&loader_lock);
        return VK_INCOMPLETE;
    }

    loader_platform_thread_unlock_mutex(&loader_lock);
    return VK_SUCCESS;
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue(VkDevice device, uint32_t queueNodeIndex, uint32_t queueIndex,
                                                          VkQueue *pQueue) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceQueue: Invalid device [VUID-vkGetDeviceQueue-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->GetDeviceQueue(device, queueNodeIndex, queueIndex, pQueue);
    if (pQueue != NULL && *pQueue != NULL) {
        loader_set_dispatch(*pQueue, disp);
    }
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                                                           VkFence fence) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkQueueSubmit: Invalid queue [VUID-vkQueueSubmit-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->QueueSubmit(queue, submitCount, pSubmits, fence);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkQueueWaitIdle(VkQueue queue) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkQueueWaitIdle: Invalid queue [VUID-vkQueueWaitIdle-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->QueueWaitIdle(queue);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkDeviceWaitIdle(VkDevice device) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDeviceWaitIdle: Invalid device [VUID-vkDeviceWaitIdle-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->DeviceWaitIdle(device);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                              const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkAllocateMemory: Invalid device [VUID-vkAllocateMemory-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkFreeMemory(VkDevice device, VkDeviceMemory mem,
                                                      const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkFreeMemory: Invalid device [VUID-vkFreeMemory-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->FreeMemory(device, mem, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset,
                                                         VkDeviceSize size, VkFlags flags, void **ppData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkMapMemory: Invalid device [VUID-vkMapMemory-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->MapMemory(device, mem, offset, size, flags, ppData);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkUnmapMemory(VkDevice device, VkDeviceMemory mem) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkUnmapMemory: Invalid device [VUID-vkUnmapMemory-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->UnmapMemory(device, mem);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                       const VkMappedMemoryRange *pMemoryRanges) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkFlushMappedMemoryRanges: Invalid device [VUID-vkFlushMappedMemoryRanges-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                            const VkMappedMemoryRange *pMemoryRanges) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkInvalidateMappedMemoryRanges: Invalid device [VUID-vkInvalidateMappedMemoryRanges-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                                     VkDeviceSize *pCommittedMemoryInBytes) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceMemoryCommitment: Invalid device [VUID-vkGetDeviceMemoryCommitment-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem,
                                                                VkDeviceSize offset) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBindBufferMemory: Invalid device [VUID-vkBindBufferMemory-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->BindBufferMemory(device, buffer, mem, offset);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem,
                                                               VkDeviceSize offset) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBindImageMemory: Invalid device [VUID-vkBindImageMemory-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->BindImageMemory(device, image, mem, offset);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                                       VkMemoryRequirements *pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetBufferMemoryRequirements: Invalid device [VUID-vkGetBufferMemoryRequirements-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                                      VkMemoryRequirements *pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageMemoryRequirements: Invalid device [VUID-vkGetImageMemoryRequirements-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL
vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
                                   VkSparseImageMemoryRequirements *pSparseMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageSparseMemoryRequirements: Invalid device [VUID-vkGetImageSparseMemoryRequirements-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage,
    VkImageTiling tiling, uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceSparseImageFormatProperties: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceSparseImageFormatProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp = loader_get_instance_layer_dispatch(physicalDevice);
    disp->GetPhysicalDeviceSparseImageFormatProperties(unwrapped_phys_dev, format, type, samples, usage, tiling, pPropertyCount,
                                                       pProperties);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount,
                                                               const VkBindSparseInfo *pBindInfo, VkFence fence) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkQueueBindSparse: Invalid queue [VUID-vkQueueBindSparse-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkFence *pFence) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateFence: Invalid device [VUID-vkCreateFence-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateFence(device, pCreateInfo, pAllocator, pFence);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyFence: Invalid device [VUID-vkDestroyFence-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyFence(device, fence, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkResetFences: Invalid device [VUID-vkResetFences-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->ResetFences(device, fenceCount, pFences);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetFenceStatus(VkDevice device, VkFence fence) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetFenceStatus: Invalid device [VUID-vkGetFenceStatus-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->GetFenceStatus(device, fence);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                                             VkBool32 waitAll, uint64_t timeout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkWaitForFences: Invalid device [VUID-vkWaitForFences-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->WaitForFences(device, fenceCount, pFences, waitAll, timeout);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateSemaphore: Invalid device [VUID-vkCreateSemaphore-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                                            const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroySemaphore: Invalid device [VUID-vkDestroySemaphore-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroySemaphore(device, semaphore, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkEvent *pEvent) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateEvent: Invalid device [VUID-vkCreateEvent-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateEvent(device, pCreateInfo, pAllocator, pEvent);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyEvent: Invalid device [VUID-vkDestroyEvent-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyEvent(device, event, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetEventStatus(VkDevice device, VkEvent event) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetEventStatus: Invalid device [VUID-vkGetEventStatus-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->GetEventStatus(device, event);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkSetEvent(VkDevice device, VkEvent event) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetEvent: Invalid device [VUID-vkSetEvent-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->SetEvent(device, event);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkResetEvent(VkDevice device, VkEvent event) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkResetEvent: Invalid device [VUID-vkResetEvent-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->ResetEvent(device, event);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateQueryPool: Invalid device [VUID-vkCreateQueryPool-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                                            const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyQueryPool: Invalid device [VUID-vkDestroyQueryPool-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyQueryPool(device, queryPool, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                                   uint32_t queryCount, size_t dataSize, void *pData,
                                                                   VkDeviceSize stride, VkQueryResultFlags flags) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetQueryPoolResults: Invalid device [VUID-vkGetQueryPoolResults-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateBuffer: Invalid device [VUID-vkCreateBuffer-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyBuffer(VkDevice device, VkBuffer buffer,
                                                         const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyBuffer: Invalid device [VUID-vkDestroyBuffer-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyBuffer(device, buffer, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkBufferView *pView) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateBufferView: Invalid device [VUID-vkCreateBufferView-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateBufferView(device, pCreateInfo, pAllocator, pView);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                             const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyBufferView: Invalid device [VUID-vkDestroyBufferView-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyBufferView(device, bufferView, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkImage *pImage) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateImage: Invalid device [VUID-vkCreateImage-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateImage(device, pCreateInfo, pAllocator, pImage);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyImage: Invalid device [VUID-vkDestroyImage-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyImage(device, image, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetImageSubresourceLayout(VkDevice device, VkImage image,
                                                                     const VkImageSubresource *pSubresource,
                                                                     VkSubresourceLayout *pLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageSubresourceLayout: Invalid device [VUID-vkGetImageSubresourceLayout-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->GetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkImageView *pView) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateImageView: Invalid device [VUID-vkCreateImageView-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateImageView(device, pCreateInfo, pAllocator, pView);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyImageView(VkDevice device, VkImageView imageView,
                                                            const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyImageView: Invalid device [VUID-vkDestroyImageView-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyImageView(device, imageView, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkShaderModule *pShader) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateShaderModule: Invalid device [VUID-vkCreateShaderModule-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateShaderModule(device, pCreateInfo, pAllocator, pShader);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                               const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyShaderModule: Invalid device [VUID-vkDestroyShaderModule-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyShaderModule(device, shaderModule, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator,
                                                                   VkPipelineCache *pPipelineCache) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreatePipelineCache: Invalid device [VUID-vkCreatePipelineCache-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                                const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyPipelineCache: Invalid device [VUID-vkDestroyPipelineCache-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyPipelineCache(device, pipelineCache, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache,
                                                                    size_t *pDataSize, void *pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPipelineCacheData: Invalid device [VUID-vkGetPipelineCacheData-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache,
                                                                   uint32_t srcCacheCount, const VkPipelineCache *pSrcCaches) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkMergePipelineCaches: Invalid device [VUID-vkMergePipelineCaches-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                       uint32_t createInfoCount,
                                                                       const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                       const VkAllocationCallbacks *pAllocator,
                                                                       VkPipeline *pPipelines) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateGraphicsPipelines: Invalid device [VUID-vkCreateGraphicsPipelines-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                                      uint32_t createInfoCount,
                                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                                      const VkAllocationCallbacks *pAllocator,
                                                                      VkPipeline *pPipelines) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateComputePipelines: Invalid device [VUID-vkCreateComputePipelines-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                           const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyPipeline: Invalid device [VUID-vkDestroyPipeline-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyPipeline(device, pipeline, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                                    const VkAllocationCallbacks *pAllocator,
                                                                    VkPipelineLayout *pPipelineLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreatePipelineLayout: Invalid device [VUID-vkCreatePipelineLayout-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                                 const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyPipelineLayout: Invalid device [VUID-vkDestroyPipelineLayout-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkSampler *pSampler) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateSampler: Invalid device [VUID-vkCreateSampler-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateSampler(device, pCreateInfo, pAllocator, pSampler);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroySampler(VkDevice device, VkSampler sampler,
                                                          const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroySampler: Invalid device [VUID-vkDestroySampler-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroySampler(device, sampler, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorSetLayout(VkDevice device,
                                                                         const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         VkDescriptorSetLayout *pSetLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateDescriptorSetLayout: Invalid device [VUID-vkCreateDescriptorSetLayout-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                                      const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyDescriptorSetLayout: Invalid device [VUID-vkDestroyDescriptorSetLayout-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                                    const VkAllocationCallbacks *pAllocator,
                                                                    VkDescriptorPool *pDescriptorPool) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateDescriptorPool: Invalid device [VUID-vkCreateDescriptorPool-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                                 const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyDescriptorPool: Invalid device [VUID-vkDestroyDescriptorPool-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyDescriptorPool(device, descriptorPool, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                                   VkDescriptorPoolResetFlags flags) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkResetDescriptorPool: Invalid device [VUID-vkResetDescriptorPool-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->ResetDescriptorPool(device, descriptorPool, flags);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkAllocateDescriptorSets(VkDevice device,
                                                                      const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                                      VkDescriptorSet *pDescriptorSets) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkAllocateDescriptorSets: Invalid device [VUID-vkAllocateDescriptorSets-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                                                  uint32_t descriptorSetCount,
                                                                  const VkDescriptorSet *pDescriptorSets) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkFreeDescriptorSets: Invalid device [VUID-vkFreeDescriptorSets-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                                const VkWriteDescriptorSet *pDescriptorWrites,
                                                                uint32_t descriptorCopyCount,
                                                                const VkCopyDescriptorSet *pDescriptorCopies) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkUpdateDescriptorSets: Invalid device [VUID-vkUpdateDescriptorSets-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkFramebuffer *pFramebuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateFramebuffer: Invalid device [VUID-vkCreateFramebuffer-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                              const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyFramebuffer: Invalid device [VUID-vkDestroyFramebuffer-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyFramebuffer(device, framebuffer, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkRenderPass *pRenderPass) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateRenderPass: Invalid device [VUID-vkCreateRenderPass-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                             const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyRenderPass: Invalid device [VUID-vkDestroyRenderPass-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyRenderPass(device, renderPass, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass,
                                                                    VkExtent2D *pGranularity) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetRenderAreaGranularity: Invalid device [VUID-vkGetRenderAreaGranularity-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->GetRenderAreaGranularity(device, renderPass, pGranularity);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkCommandPool *pCommandPool) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateCommandPool: Invalid device [VUID-vkCreateCommandPool-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                              const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyCommandPool: Invalid device [VUID-vkDestroyCommandPool-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->DestroyCommandPool(device, commandPool, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                                                VkCommandPoolResetFlags flags) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkResetCommandPool: Invalid device [VUID-vkResetCommandPool-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->ResetCommandPool(device, commandPool, flags);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkAllocateCommandBuffers(VkDevice device,
                                                                      const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                                      VkCommandBuffer *pCommandBuffers) {
    VkResult res;
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkAllocateCommandBuffers: Invalid device [VUID-vkAllocateCommandBuffers-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    res = disp->AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    if (res == VK_SUCCESS) {
        for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++) {
            if (pCommandBuffers[i]) {
                loader_set_dispatch(pCommandBuffers[i], disp);
            }
        }
    }

    return res;
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                              uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkFreeCommandBuffers: Invalid device [VUID-vkFreeCommandBuffers-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                                  const VkCommandBufferBeginInfo *pBeginInfo) {
    const VkLayerDispatchTable *disp;

    disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBeginCommandBuffer: Invalid commandBuffer [VUID-vkBeginCommandBuffer-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->BeginCommandBuffer(commandBuffer, pBeginInfo);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEndCommandBuffer(VkCommandBuffer commandBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkEndCommandBuffer: Invalid commandBuffer [VUID-vkEndCommandBuffer-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->EndCommandBuffer(commandBuffer);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkResetCommandBuffer: Invalid commandBuffer [VUID-vkResetCommandBuffer-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->ResetCommandBuffer(commandBuffer, flags);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                           VkPipeline pipeline) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindPipeline: Invalid commandBuffer [VUID-vkCmdBindPipeline-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                          uint32_t viewportCount, const VkViewport *pViewports) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetViewport: Invalid commandBuffer [VUID-vkCmdSetViewport-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                                                         uint32_t scissorCount, const VkRect2D *pScissors) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetScissor: Invalid commandBuffer [VUID-vkCmdSetScissor-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetLineWidth: Invalid commandBuffer [VUID-vkCmdSetLineWidth-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdSetLineWidth(commandBuffer, lineWidth);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                                                           float depthBiasClamp, float depthBiasSlopeFactor) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthBias: Invalid commandBuffer [VUID-vkCmdSetDepthBias-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetBlendConstants: Invalid commandBuffer [VUID-vkCmdSetBlendConstants-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdSetBlendConstants(commandBuffer, blendConstants);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                                                             float maxDepthBounds) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthBounds: Invalid commandBuffer [VUID-vkCmdSetDepthBounds-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                    uint32_t compareMask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetStencilCompareMask: Invalid commandBuffer [VUID-vkCmdSetStencilCompareMask-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                  uint32_t writeMask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetStencilWriteMask: Invalid commandBuffer [VUID-vkCmdSetStencilWriteMask-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                  uint32_t reference) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetStencilReference: Invalid commandBuffer [VUID-vkCmdSetStencilReference-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdSetStencilReference(commandBuffer, faceMask, reference);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                                                                 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                 uint32_t firstSet, uint32_t descriptorSetCount,
                                                                 const VkDescriptorSet *pDescriptorSets,
                                                                 uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindDescriptorSets: Invalid commandBuffer [VUID-vkCmdBindDescriptorSets-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets,
                                dynamicOffsetCount, pDynamicOffsets);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkIndexType indexType) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindIndexBuffer: Invalid commandBuffer [VUID-vkCmdBindIndexBuffer-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                const VkDeviceSize *pOffsets) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindVertexBuffers: Invalid commandBuffer [VUID-vkCmdBindVertexBuffers-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                                   uint32_t firstVertex, uint32_t firstInstance) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDraw: Invalid commandBuffer [VUID-vkCmdDraw-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
                                                          uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                                          uint32_t firstInstance) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawIndexed: Invalid commandBuffer [VUID-vkCmdDrawIndexed-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawIndirect: Invalid commandBuffer [VUID-vkCmdDrawIndirect-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawIndexedIndirect: Invalid commandBuffer [VUID-vkCmdDrawIndexedIndirect-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDispatch: Invalid commandBuffer [VUID-vkCmdDispatch-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdDispatch(commandBuffer, x, y, z);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                               VkDeviceSize offset) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDispatchIndirect: Invalid commandBuffer [VUID-vkCmdDispatchIndirect-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdDispatchIndirect(commandBuffer, buffer, offset);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                         uint32_t regionCount, const VkBufferCopy *pRegions) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyBuffer: Invalid commandBuffer [VUID-vkCmdCopyBuffer-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                        VkImageLayout srcImageLayout, VkImage dstImage,
                                                        VkImageLayout dstImageLayout, uint32_t regionCount,
                                                        const VkImageCopy *pRegions) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyImage: Invalid commandBuffer [VUID-vkCmdCopyImage-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                        VkImageLayout srcImageLayout, VkImage dstImage,
                                                        VkImageLayout dstImageLayout, uint32_t regionCount,
                                                        const VkImageBlit *pRegions, VkFilter filter) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBlitImage: Invalid commandBuffer [VUID-vkCmdBlitImage-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                                VkImageLayout dstImageLayout, uint32_t regionCount,
                                                                const VkBufferImageCopy *pRegions) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyBufferToImage: Invalid commandBuffer [VUID-vkCmdCopyBufferToImage-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                                VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                                                uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyImageToBuffer: Invalid commandBuffer [VUID-vkCmdCopyImageToBuffer-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdUpdateBuffer: Invalid commandBuffer [VUID-vkCmdUpdateBuffer-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                         VkDeviceSize size, uint32_t data) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdFillBuffer: Invalid commandBuffer [VUID-vkCmdFillBuffer-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                                                              VkImageLayout imageLayout, const VkClearColorValue *pColor,
                                                              uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdClearColorImage: Invalid commandBuffer [VUID-vkCmdClearColorImage-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                                     VkImageLayout imageLayout,
                                                                     const VkClearDepthStencilValue *pDepthStencil,
                                                                     uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdClearDepthStencilImage: Invalid commandBuffer [VUID-vkCmdClearDepthStencilImage-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                               const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                               const VkClearRect *pRects) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdClearAttachments: Invalid commandBuffer [VUID-vkCmdClearAttachments-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                           VkImageLayout srcImageLayout, VkImage dstImage,
                                                           VkImageLayout dstImageLayout, uint32_t regionCount,
                                                           const VkImageResolve *pRegions) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdResolveImage: Invalid commandBuffer [VUID-vkCmdResolveImage-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                       VkPipelineStageFlags stageMask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetEvent: Invalid commandBuffer [VUID-vkCmdSetEvent-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdSetEvent(commandBuffer, event, stageMask);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                         VkPipelineStageFlags stageMask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdResetEvent: Invalid commandBuffer [VUID-vkCmdResetEvent-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdResetEvent(commandBuffer, event, stageMask);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                         VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                                         uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                         uint32_t bufferMemoryBarrierCount,
                                                         const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                         uint32_t imageMemoryBarrierCount,
                                                         const VkImageMemoryBarrier *pImageMemoryBarriers) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWaitEvents: Invalid commandBuffer [VUID-vkCmdWaitEvents-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdWaitEvents(commandBuffer, eventCount, pEvents, sourceStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                        bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                              VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                              uint32_t bufferMemoryBarrierCount,
                                                              const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                              uint32_t imageMemoryBarrierCount,
                                                              const VkImageMemoryBarrier *pImageMemoryBarriers) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPipelineBarrier: Invalid commandBuffer [VUID-vkCmdPipelineBarrier-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
                             bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                         VkFlags flags) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginQuery: Invalid commandBuffer [VUID-vkCmdBeginQuery-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdBeginQuery(commandBuffer, queryPool, slot, flags);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndQuery: Invalid commandBuffer [VUID-vkCmdEndQuery-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdEndQuery(commandBuffer, queryPool, slot);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                             uint32_t firstQuery, uint32_t queryCount) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdResetQueryPool: Invalid commandBuffer [VUID-vkCmdResetQueryPool-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                             VkQueryPool queryPool, uint32_t slot) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWriteTimestamp: Invalid commandBuffer [VUID-vkCmdWriteTimestamp-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, slot);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                   uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                                   VkDeviceSize dstOffset, VkDeviceSize stride, VkFlags flags) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyQueryPoolResults: Invalid commandBuffer [VUID-vkCmdCopyQueryPoolResults-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                            VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                            const void *pValues) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushConstants: Invalid commandBuffer [VUID-vkCmdPushConstants-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                                                              const VkRenderPassBeginInfo *pRenderPassBegin,
                                                              VkSubpassContents contents) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginRenderPass: Invalid commandBuffer [VUID-vkCmdBeginRenderPass-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdNextSubpass: Invalid commandBuffer [VUID-vkCmdNextSubpass-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdNextSubpass(commandBuffer, contents);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndRenderPass: Invalid commandBuffer [VUID-vkCmdEndRenderPass-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdEndRenderPass(commandBuffer);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                                              const VkCommandBuffer *pCommandBuffers) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdExecuteCommands: Invalid commandBuffer [VUID-vkCmdExecuteCommands-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    disp->CmdExecuteCommands(commandBuffer, commandBuffersCount, pCommandBuffers);
}

// ---- Vulkan core 1.1 trampolines

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties) {
    VkResult res = VK_SUCCESS;
    struct loader_instance *inst = NULL;

    loader_platform_thread_lock_mutex(&loader_lock);

    inst = loader_get_instance(instance);
    if (NULL == inst) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkEnumeratePhysicalDeviceGroups: Invalid instance [VUID-vkEnumeratePhysicalDeviceGroups-instance-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    if (NULL == pPhysicalDeviceGroupCount) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                   "vkEnumeratePhysicalDeviceGroups: Received NULL pointer for physical "
                   "device group count return value.");
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }

    // Call down the chain to get the physical device group info.
    res = inst->disp->layer_inst_disp.EnumeratePhysicalDeviceGroups(inst->instance, pPhysicalDeviceGroupCount,
                                                                    pPhysicalDeviceGroupProperties);
    if (NULL != pPhysicalDeviceGroupProperties && (VK_SUCCESS == res || VK_INCOMPLETE == res)) {
        // Wrap the PhysDev object for loader usage, return wrapped objects
        VkResult update_res = setup_loader_tramp_phys_dev_groups(inst, *pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
        if (VK_SUCCESS != update_res) {
            res = update_res;
        }
    }

out:

    loader_platform_thread_unlock_mutex(&loader_lock);
    return res;
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                                                      VkPhysicalDeviceFeatures2 *pFeatures) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceFeatures2: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceFeatures2-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    const struct loader_instance *inst = ((struct loader_physical_device_tramp *)physicalDevice)->this_instance;

    if (inst != NULL && inst->enabled_extensions.khr_get_physical_device_properties2) {
        disp->GetPhysicalDeviceFeatures2KHR(unwrapped_phys_dev, pFeatures);
    } else {
        disp->GetPhysicalDeviceFeatures2(unwrapped_phys_dev, pFeatures);
    }
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                                        VkPhysicalDeviceProperties2 *pProperties) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceProperties2: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceProperties2-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    const struct loader_instance *inst = ((struct loader_physical_device_tramp *)physicalDevice)->this_instance;

    if (inst != NULL && inst->enabled_extensions.khr_get_physical_device_properties2) {
        disp->GetPhysicalDeviceProperties2KHR(unwrapped_phys_dev, pProperties);
    } else {
        disp->GetPhysicalDeviceProperties2(unwrapped_phys_dev, pProperties);
    }
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                              VkFormatProperties2 *pFormatProperties) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceFormatProperties2: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceFormatProperties2-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    const struct loader_instance *inst = ((struct loader_physical_device_tramp *)physicalDevice)->this_instance;

    if (inst != NULL && inst->enabled_extensions.khr_get_physical_device_properties2) {
        disp->GetPhysicalDeviceFormatProperties2KHR(unwrapped_phys_dev, format, pFormatProperties);
    } else {
        disp->GetPhysicalDeviceFormatProperties2(unwrapped_phys_dev, format, pFormatProperties);
    }
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                          VkImageFormatProperties2 *pImageFormatProperties) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceImageFormatProperties2: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceImageFormatProperties2-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    const struct loader_instance *inst = ((struct loader_physical_device_tramp *)physicalDevice)->this_instance;

    if (inst != NULL && inst->enabled_extensions.khr_get_physical_device_properties2) {
        return disp->GetPhysicalDeviceImageFormatProperties2KHR(unwrapped_phys_dev, pImageFormatInfo, pImageFormatProperties);
    } else {
        return disp->GetPhysicalDeviceImageFormatProperties2(unwrapped_phys_dev, pImageFormatInfo, pImageFormatProperties);
    }
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceQueueFamilyProperties2: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceQueueFamilyProperties2-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    const struct loader_instance *inst = ((struct loader_physical_device_tramp *)physicalDevice)->this_instance;

    if (inst != NULL && inst->enabled_extensions.khr_get_physical_device_properties2) {
        disp->GetPhysicalDeviceQueueFamilyProperties2KHR(unwrapped_phys_dev, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    } else {
        disp->GetPhysicalDeviceQueueFamilyProperties2(unwrapped_phys_dev, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL
vkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceMemoryProperties2: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceMemoryProperties2-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    const struct loader_instance *inst = ((struct loader_physical_device_tramp *)physicalDevice)->this_instance;

    if (inst != NULL && inst->enabled_extensions.khr_get_physical_device_properties2) {
        disp->GetPhysicalDeviceMemoryProperties2KHR(unwrapped_phys_dev, pMemoryProperties);
    } else {
        disp->GetPhysicalDeviceMemoryProperties2(unwrapped_phys_dev, pMemoryProperties);
    }
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo, uint32_t *pPropertyCount,
    VkSparseImageFormatProperties2 *pProperties) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceSparseImageFormatProperties2: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceSparseImageFormatProperties2-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    const struct loader_instance *inst = ((struct loader_physical_device_tramp *)physicalDevice)->this_instance;

    if (inst != NULL && inst->enabled_extensions.khr_get_physical_device_properties2) {
        disp->GetPhysicalDeviceSparseImageFormatProperties2KHR(unwrapped_phys_dev, pFormatInfo, pPropertyCount, pProperties);
    } else {
        disp->GetPhysicalDeviceSparseImageFormatProperties2(unwrapped_phys_dev, pFormatInfo, pPropertyCount, pProperties);
    }
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceExternalBufferProperties: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceExternalBufferProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    const struct loader_instance *inst = ((struct loader_physical_device_tramp *)physicalDevice)->this_instance;

    if (inst != NULL && inst->enabled_extensions.khr_external_memory_capabilities) {
        disp->GetPhysicalDeviceExternalBufferPropertiesKHR(unwrapped_phys_dev, pExternalBufferInfo, pExternalBufferProperties);
    } else {
        disp->GetPhysicalDeviceExternalBufferProperties(unwrapped_phys_dev, pExternalBufferInfo, pExternalBufferProperties);
    }
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceExternalSemaphoreProperties: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceExternalSemaphoreProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    const struct loader_instance *inst = ((struct loader_physical_device_tramp *)physicalDevice)->this_instance;

    if (inst != NULL && inst->enabled_extensions.khr_external_semaphore_capabilities) {
        disp->GetPhysicalDeviceExternalSemaphorePropertiesKHR(unwrapped_phys_dev, pExternalSemaphoreInfo,
                                                              pExternalSemaphoreProperties);
    } else {
        disp->GetPhysicalDeviceExternalSemaphoreProperties(unwrapped_phys_dev, pExternalSemaphoreInfo,
                                                           pExternalSemaphoreProperties);
    }
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceExternalFenceProperties: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceExternalFenceProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    const struct loader_instance *inst = ((struct loader_physical_device_tramp *)physicalDevice)->this_instance;

    if (inst != NULL && inst->enabled_extensions.khr_external_fence_capabilities) {
        disp->GetPhysicalDeviceExternalFencePropertiesKHR(unwrapped_phys_dev, pExternalFenceInfo, pExternalFenceProperties);
    } else {
        disp->GetPhysicalDeviceExternalFenceProperties(unwrapped_phys_dev, pExternalFenceInfo, pExternalFenceProperties);
    }
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                                 const VkBindBufferMemoryInfo *pBindInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBindBufferMemory2: Invalid device [VUID-vkBindBufferMemory2-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->BindBufferMemory2(device, bindInfoCount, pBindInfos);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                                const VkBindImageMemoryInfo *pBindInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBindImageMemory2: Invalid device [VUID-vkBindImageMemory2-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->BindImageMemory2(device, bindInfoCount, pBindInfos);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex,
                                                                            uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
                                                                            VkPeerMemoryFeatureFlags *pPeerMemoryFeatures) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceGroupPeerMemoryFeatures: Invalid device [VUID-vkGetDeviceGroupPeerMemoryFeatures-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDeviceMask: Invalid commandBuffer [VUID-vkCmdSetDeviceMask-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDeviceMask(commandBuffer, deviceMask);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                           uint32_t groupCountZ) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDispatchBase: Invalid commandBuffer [VUID-vkCmdDispatchBase-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                                       VkMemoryRequirements2 *pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageMemoryRequirements2: Invalid device [VUID-vkGetImageMemoryRequirements2-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements2(VkDevice device,
                                                                        const VkBufferMemoryRequirementsInfo2 *pInfo,
                                                                        VkMemoryRequirements2 *pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetBufferMemoryRequirements2: Invalid device [VUID-vkGetBufferMemoryRequirements2-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements2(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(
            NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
            "vkGetImageSparseMemoryRequirements2: Invalid device [VUID-vkGetImageSparseMemoryRequirements2-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkTrimCommandPool(VkDevice device, VkCommandPool commandPool,
                                                           VkCommandPoolTrimFlags flags) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkTrimCommandPool: Invalid device [VUID-vkTrimCommandPool-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->TrimCommandPool(device, commandPool, flags);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceQueue2: Invalid device [VUID-vkGetDeviceQueue2-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceQueue2(device, pQueueInfo, pQueue);
    if (pQueue != NULL && *pQueue != NULL) {
        loader_set_dispatch(*pQueue, disp);
    }
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateSamplerYcbcrConversion(VkDevice device,
                                                                            const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                            const VkAllocationCallbacks *pAllocator,
                                                                            VkSamplerYcbcrConversion *pYcbcrConversion) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateSamplerYcbcrConversion: Invalid device [VUID-vkCreateSamplerYcbcrConversion-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                         const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroySamplerYcbcrConversion: Invalid device [VUID-vkDestroySamplerYcbcrConversion-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetDescriptorSetLayoutSupport(VkDevice device,
                                                                         const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                         VkDescriptorSetLayoutSupport *pSupport) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDescriptorSetLayoutSupport: Invalid device [VUID-vkGetDescriptorSetLayoutSupport-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateDescriptorUpdateTemplate: Invalid device [VUID-vkCreateDescriptorUpdateTemplate-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                           VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                           const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyDescriptorUpdateTemplate: Invalid device [VUID-vkDestroyDescriptorUpdateTemplate-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                           VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                           const void *pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkUpdateDescriptorSetWithTemplate: Invalid device [VUID-vkUpdateDescriptorSetWithTemplate-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->UpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}

// ---- Vulkan core 1.2 trampolines

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkRenderPass *pRenderPass) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateRenderPass2: Invalid device [VUID-vkCreateRenderPass2-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                                                               const VkRenderPassBeginInfo *pRenderPassBegin,
                                                               const VkSubpassBeginInfo *pSubpassBeginInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginRenderPass2: Invalid commandBuffer [VUID-vkCmdBeginRenderPass2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdNextSubpass2(VkCommandBuffer commandBuffer,
                                                           const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                           const VkSubpassEndInfo *pSubpassEndInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdNextSubpass2: Invalid commandBuffer [VUID-vkCmdNextSubpass2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderPass2(VkCommandBuffer commandBuffer,
                                                             const VkSubpassEndInfo *pSubpassEndInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndRenderPass2: Invalid commandBuffer [VUID-vkCmdEndRenderPass2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawIndirectCount: Invalid commandBuffer [VUID-vkCmdDrawIndirectCount-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                       VkDeviceSize offset, VkBuffer countBuffer,
                                                                       VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                       uint32_t stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawIndexedIndirectCount: Invalid commandBuffer "
                   "[VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetSemaphoreCounterValue: Invalid device [VUID-vkGetSemaphoreCounterValue-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetSemaphoreCounterValue(device, semaphore, pValue);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo,
                                                              uint64_t timeout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkWaitSemaphores: Invalid device [VUID-vkWaitSemaphores-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->WaitSemaphores(device, pWaitInfo, timeout);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSignalSemaphore: Invalid device [VUID-vkSignalSemaphore-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->SignalSemaphore(device, pSignalInfo);
}

LOADER_EXPORT VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetBufferDeviceAddress(VkDevice device,
                                                                             const VkBufferDeviceAddressInfo *pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetBufferDeviceAddress: Invalid device [VUID-vkGetBufferDeviceAddress-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetBufferDeviceAddress(device, pInfo);
}

LOADER_EXPORT VKAPI_ATTR uint64_t VKAPI_CALL vkGetBufferOpaqueCaptureAddress(VkDevice device,
                                                                             const VkBufferDeviceAddressInfo *pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetBufferOpaqueCaptureAddress: Invalid device [VUID-vkGetBufferOpaqueCaptureAddress-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetBufferOpaqueCaptureAddress(device, pInfo);
}

LOADER_EXPORT VKAPI_ATTR uint64_t VKAPI_CALL
vkGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceMemoryOpaqueCaptureAddress: Invalid device "
                   "[VUID-vkGetDeviceMemoryOpaqueCaptureAddress-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                          uint32_t queryCount) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkResetQueryPool: Invalid device [VUID-vkResetQueryPool-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->ResetQueryPool(device, queryPool, firstQuery, queryCount);
}

// ---- Vulkan core 1.3 trampolines

// Instance

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice,
                                                                               uint32_t *pToolCount,
                                                                               VkPhysicalDeviceToolProperties *pToolProperties) {
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    const VkLayerInstanceDispatchTable *disp = loader_get_instance_layer_dispatch(physicalDevice);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceToolProperties: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceToolProperties-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }

    return disp->GetPhysicalDeviceToolProperties(unwrapped_phys_dev, pToolCount, pToolProperties);
}

// Device

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginRendering: Invalid commandBuffer "
                   "[VUID-vkCmdBeginRendering-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBeginRendering(commandBuffer, pRenderingInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                 uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                 const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                 const VkDeviceSize *pStrides) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindVertexBuffers2: Invalid commandBuffer "
                   "[VUID-vkCmdBindVertexBuffers2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBlitImage2: Invalid commandBuffer "
                   "[VUID-vkCmdBlitImage2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBlitImage2(commandBuffer, pBlitImageInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyBuffer2: Invalid commandBuffer "
                   "[VUID-vkCmdCopyBuffer2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                                 const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyBufferToImage2: Invalid commandBuffer "
                   "[VUID-vkCmdCopyBufferToImage2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyImage2: Invalid commandBuffer "
                   "[VUID-vkCmdCopyImage2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyImage2(commandBuffer, pCopyImageInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                                 const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyImageToBuffer2: Invalid commandBuffer "
                   "[VUID-vkCmdCopyImageToBuffer2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdEndRendering(VkCommandBuffer commandBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndRendering: Invalid commandBuffer "
                   "[VUID-vkCmdEndRendering-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdEndRendering(commandBuffer);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                                                               const VkDependencyInfo *pDependencyInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPipelineBarrier2: Invalid commandBuffer "
                   "[VUID-vkCmdPipelineBarrier2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPipelineBarrier2(commandBuffer, pDependencyInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                          VkPipelineStageFlags2 stageMask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdResetEvent2: Invalid commandBuffer "
                   "[VUID-vkCmdResetEvent2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdResetEvent2(commandBuffer, event, stageMask);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdResolveImage2(VkCommandBuffer commandBuffer,
                                                            const VkResolveImageInfo2 *pResolveImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdResolveImage2: Invalid commandBuffer "
                   "[VUID-vkCmdResolveImage2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdResolveImage2(commandBuffer, pResolveImageInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetCullMode: Invalid commandBuffer "
                   "[VUID-vkCmdSetCullMode-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetCullMode(commandBuffer, cullMode);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthBiasEnable: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthBiasEnable-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer,
                                                                       VkBool32 depthBoundsTestEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthBoundsTestEnable: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthBoundsTestEnable-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthCompareOp: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthCompareOp-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthCompareOp(commandBuffer, depthCompareOp);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthTestEnable: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthTestEnable-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthTestEnable(commandBuffer, depthTestEnable);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthWriteEnable: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthWriteEnable-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                        const VkDependencyInfo *pDependencyInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetEvent2: Invalid commandBuffer "
                   "[VUID-vkCmdSetEvent2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetEvent2(commandBuffer, event, pDependencyInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetFrontFace: Invalid commandBuffer "
                   "[VUID-vkCmdSetFrontFace-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetFrontFace(commandBuffer, frontFace);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer,
                                                                        VkBool32 primitiveRestartEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetPrimitiveRestartEnable: Invalid commandBuffer "
                   "[VUID-vkCmdSetPrimitiveRestartEnable-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
                                                                   VkPrimitiveTopology primitiveTopology) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetPrimitiveTopology: Invalid commandBuffer "
                   "[VUID-vkCmdSetPrimitiveTopology-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
                                                                         VkBool32 rasterizerDiscardEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetRasterizerDiscardEnable: Invalid commandBuffer "
                   "[VUID-vkCmdSetRasterizerDiscardEnable-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                  const VkRect2D *pScissors) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetScissorWithCount: Invalid commandBuffer "
                   "[VUID-vkCmdSetScissorWithCount-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                           VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp,
                                                           VkCompareOp compareOp) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetStencilOp: Invalid commandBuffer "
                   "[VUID-vkCmdSetStencilOp-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetStencilTestEnable: Invalid commandBuffer "
                   "[VUID-vkCmdSetStencilTestEnable-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                   const VkViewport *pViewports) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetViewportWithCount: Invalid commandBuffer "
                   "[VUID-vkCmdSetViewportWithCount-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                                          const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWaitEvents2: Invalid commandBuffer "
                   "[VUID-vkCmdWaitEvents2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                              VkQueryPool queryPool, uint32_t query) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWriteTimestamp2: Invalid commandBuffer "
                   "[VUID-vkCmdWriteTimestamp2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreatePrivateDataSlot(VkDevice device,
                                                                     const VkPrivateDataSlotCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkPrivateDataSlot *pPrivateDataSlot) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreatePrivateDataSlot: Invalid device "
                   "[VUID-vkCreatePrivateDataSlot-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                                  const VkAllocationCallbacks *pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyPrivateDataSlot: Invalid device "
                   "[VUID-vkDestroyPrivateDataSlot-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetDeviceBufferMemoryRequirements(VkDevice device,
                                                                             const VkDeviceBufferMemoryRequirements *pInfo,
                                                                             VkMemoryRequirements2 *pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceBufferMemoryRequirements: Invalid device "
                   "[VUID-vkGetDeviceBufferMemoryRequirements-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageMemoryRequirements(VkDevice device,
                                                                            const VkDeviceImageMemoryRequirements *pInfo,
                                                                            VkMemoryRequirements2 *pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceImageMemoryRequirements: Invalid device "
                   "[VUID-vkGetDeviceImageMemoryRequirements-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageSparseMemoryRequirements(
    VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceImageSparseMemoryRequirements: Invalid device "
                   "[VUID-vkGetDeviceImageSparseMemoryRequirements-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                          VkPrivateDataSlot privateDataSlot, uint64_t *pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPrivateData: Invalid device "
                   "[VUID-vkGetPrivateData-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                              VkPrivateDataSlot privateDataSlot, uint64_t data) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetPrivateData: Invalid device "
                   "[VUID-vkSetPrivateData-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->SetPrivateData(device, objectType, objectHandle, privateDataSlot, data);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits,
                                                            VkFence fence) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkQueueSubmit2: Invalid queue "
                   "[VUID-vkQueueSubmit2-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->QueueSubmit2(queue, submitCount, pSubmits, fence);
}

// ---- Vulkan core 1.4 trampolines

// Instance

// Device

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetRenderingAreaGranularity(VkDevice device,
                                                                       const VkRenderingAreaInfo *pRenderingAreaInfo,
                                                                       VkExtent2D *pGranularity) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetRenderingAreaGranularity: Invalid device "
                   "[VUID-vkGetRenderingAreaGranularity-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetRenderingAreaGranularity(device, pRenderingAreaInfo, pGranularity);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSet(VkCommandBuffer commandBuffer,
                                                                VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                uint32_t set, uint32_t descriptorWriteCount,
                                                                const VkWriteDescriptorSet *pDescriptorWrites) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushDescriptorSet: Invalid commandBuffer "
                   "[VUID-vkCmdPushDescriptorSet-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPushDescriptorSet(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                            VkPipelineLayout layout, uint32_t set,
                                                                            const void *pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushDescriptorSetWithTemplate: Invalid commandBuffer "
                   "[VUID-vkCmdPushDescriptorSetWithTemplate-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPushDescriptorSetWithTemplate(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                             uint16_t lineStipplePattern) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetLineStipple: Invalid commandBuffer "
                   "[VUID-vkCmdSetLineStipple-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkDeviceSize size, VkIndexType indexType) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindIndexBuffer2: Invalid commandBuffer "
                   "[VUID-vkCmdBindIndexBuffer2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindIndexBuffer2(commandBuffer, buffer, offset, size, indexType);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCopyMemoryToImage(VkDevice device,
                                                                 const VkCopyMemoryToImageInfo *pCopyMemoryToImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyMemoryToImage: Invalid device "
                   "[VUID-vkCopyMemoryToImage-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyMemoryToImage(device, pCopyMemoryToImageInfo);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCopyImageToMemory(VkDevice device,
                                                                 const VkCopyImageToMemoryInfo *pCopyImageToMemoryInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyImageToMemory: Invalid device "
                   "[VUID-vkCopyImageToMemory-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyImageToMemory(device, pCopyImageToMemoryInfo);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCopyImageToImage(VkDevice device,
                                                                const VkCopyImageToImageInfo *pCopyImageToImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyImageToImage: Invalid device "
                   "[VUID-vkCopyImageToImage-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyImageToImage(device, pCopyImageToImageInfo);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                                                     const VkHostImageLayoutTransitionInfo *pTransitions) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkTransitionImageLayout: Invalid device "
                   "[VUID-vkTransitionImageLayout-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->TransitionImageLayout(device, transitionCount, pTransitions);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageSubresourceLayout(VkDevice device,
                                                                           const VkDeviceImageSubresourceInfo *pInfo,
                                                                           VkSubresourceLayout2 *pLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceImageSubresourceLayout: Invalid device "
                   "[VUID-vkGetDeviceImageSubresourceLayout-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceImageSubresourceLayout(device, pInfo, pLayout);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkGetImageSubresourceLayout2(VkDevice device, VkImage image,
                                                                      const VkImageSubresource2 *pSubresource,
                                                                      VkSubresourceLayout2 *pLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageSubresourceLayout2: Invalid device "
                   "[VUID-vkGetImageSubresourceLayout2-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetImageSubresourceLayout2(device, image, pSubresource, pLayout);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkMapMemory2(VkDevice device, const VkMemoryMapInfo *pMemoryMapInfo, void **ppData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkMapMemory2: Invalid device "
                   "[VUID-vkMapMemory2-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->MapMemory2(device, pMemoryMapInfo, ppData);
}

LOADER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo *pMemoryUnmapInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkUnmapMemory2: Invalid device "
                   "[VUID-vkUnmapMemory2-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->UnmapMemory2(device, pMemoryUnmapInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                                  const VkBindDescriptorSetsInfo *pBindDescriptorSetsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindDescriptorSets2: Invalid commandBuffer "
                   "[VUID-vkCmdBindDescriptorSets2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindDescriptorSets2(commandBuffer, pBindDescriptorSetsInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdPushConstants2(VkCommandBuffer commandBuffer,
                                                             const VkPushConstantsInfo *pPushConstantsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushConstants2: Invalid commandBuffer "
                   "[VUID-vkCmdPushConstants2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPushConstants2(commandBuffer, pPushConstantsInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                                 const VkPushDescriptorSetInfo *pPushDescriptorSetInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushDescriptorSet2: Invalid commandBuffer "
                   "[VUID-vkCmdPushDescriptorSet2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPushDescriptorSet2(commandBuffer, pPushDescriptorSetInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushDescriptorSetWithTemplate2: Invalid commandBuffer "
                   "[VUID-vkCmdPushDescriptorSetWithTemplate2-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPushDescriptorSetWithTemplate2(commandBuffer, pPushDescriptorSetWithTemplateInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL
vkCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer, const VkRenderingAttachmentLocationInfo *pLocationInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetRenderingAttachmentLocations: Invalid commandBuffer "
                   "[VUID-vkCmdSetRenderingAttachmentLocations-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetRenderingAttachmentLocations(commandBuffer, pLocationInfo);
}

LOADER_EXPORT VKAPI_ATTR void VKAPI_CALL vkCmdSetRenderingInputAttachmentIndices(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo *pInputAttachmentIndexInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetRenderingInputAttachmentIndices: Invalid commandBuffer "
                   "[VUID-vkCmdSetRenderingInputAttachmentIndices-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetRenderingInputAttachmentIndices(commandBuffer, pInputAttachmentIndexInfo);
}
