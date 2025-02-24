/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
 */
#include <string.h>
#include <cassert>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "containers/custom_containers.h"
#include "generated/vk_dispatch_table_helper.h"
#include "utils/vk_layer_utils.h"
#include "vk_lunarg_device_profile_api_layer.h"
#include <vulkan/utility/vk_struct_helper.hpp>

namespace device_profile_api {

static std::mutex global_lock;

static uint32_t loader_layer_if_version = CURRENT_LOADER_LAYER_INTERFACE_VERSION;

struct layer_data {
    VkInstance instance;
    VkPhysicalDeviceProperties phy_device_props;
    std::unordered_map<VkFormat, VkFormatProperties, std::hash<int> > format_properties_map;
    std::unordered_map<VkFormat, VkFormatProperties3, std::hash<int> > format_properties3_map;
    VkPhysicalDeviceFeatures phy_device_features;
    VkLayerInstanceDispatchTable dispatch_table;

    // For VkPhysicalDeviceProperties2 instead of generating and storing every pNext, just list those needed as there will probably
    // not end up being that many
    bool modify_properties2 = false;
    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_operation_advanced_props;
};

static std::unordered_map<void *, layer_data *> device_profile_api_dev_data_map;

// For the given data key, look up the layer_data instance from given layer_data_map
template <typename DATA_T>
DATA_T *GetLayerDataPtr(void *data_key, std::unordered_map<void *, DATA_T *> &layer_data_map) {
    DATA_T *debug_data;
    /* TODO: We probably should lock here, or have caller lock */
    auto got = layer_data_map.find(data_key);

    if (got == layer_data_map.end()) {
        debug_data = new DATA_T;
        layer_data_map[(void *)data_key] = debug_data;
    } else {
        debug_data = got->second;
    }

    return debug_data;
}

template <typename DATA_T>
void FreeLayerDataPtr(void *data_key, std::unordered_map<void *, DATA_T *> &layer_data_map) {
    auto got = layer_data_map.find(data_key);
    assert(got != layer_data_map.end());

    delete got->second;
    layer_data_map.erase(got);
}

// device_profile_api Layer EXT APIs
typedef void(VKAPI_PTR *PFN_vkGetOriginalPhysicalDeviceLimitsEXT)(VkPhysicalDevice physicalDevice,
                                                                  const VkPhysicalDeviceLimits *limits);
typedef void(VKAPI_PTR *PFN_vkSetPhysicalDeviceLimitsEXT)(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceLimits *newLimits);
typedef void(VKAPI_PTR *PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT)(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                            const VkFormatProperties *properties);
typedef void(VKAPI_PTR *PFN_vkSetPhysicalDeviceFormatPropertiesEXT)(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                    const VkFormatProperties newProperties);
typedef void(VKAPI_PTR *PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT)(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                             const VkFormatProperties2 *properties);
typedef void(VKAPI_PTR *PFN_vkSetPhysicalDeviceFormatProperties2EXT)(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                     const VkFormatProperties2 newProperties);
typedef void(VKAPI_PTR *PFN_vkGetOriginalPhysicalDeviceFeaturesEXT)(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceFeatures *features);
typedef void(VKAPI_PTR *PFN_vkSetPhysicalDeviceFeaturesEXT)(VkPhysicalDevice physicalDevice, const VkFormatProperties2 newFeatures);
typedef void(VKAPI_PTR *PFN_VkSetPhysicalDeviceProperties2EXT)(VkPhysicalDevice physicalDevice,
                                                               const VkPhysicalDeviceProperties2 newProperties);

VKAPI_ATTR void VKAPI_CALL GetOriginalPhysicalDeviceLimitsEXT(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits *orgLimits) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);
    layer_data *instance_data = GetLayerDataPtr(phy_dev_data->instance, device_profile_api_dev_data_map);
    VkPhysicalDeviceProperties props;
    instance_data->dispatch_table.GetPhysicalDeviceProperties(physicalDevice, &props);
    memcpy(orgLimits, &props.limits, sizeof(VkPhysicalDeviceLimits));
}

VKAPI_ATTR void VKAPI_CALL SetPhysicalDeviceLimitsEXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceLimits *newLimits) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);
    memcpy(&(phy_dev_data->phy_device_props.limits), newLimits, sizeof(VkPhysicalDeviceLimits));
}

VKAPI_ATTR void VKAPI_CALL GetOriginalPhysicalDeviceFormatPropertiesEXT(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                        VkFormatProperties *properties) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);
    layer_data *instance_data = GetLayerDataPtr(phy_dev_data->instance, device_profile_api_dev_data_map);
    instance_data->dispatch_table.GetPhysicalDeviceFormatProperties(physicalDevice, format, properties);
}

VKAPI_ATTR void VKAPI_CALL SetPhysicalDeviceFormatPropertiesEXT(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                const VkFormatProperties newProperties) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);

    memcpy(&(phy_dev_data->format_properties_map[format]), &newProperties, sizeof(VkFormatProperties));

    // Need to set in the case a test uses SetPhysicalDeviceFormatProperties1() functions but because it is a 1.3 device and
    // the validation code will call GetPhysicalDeviceFormatProperties2() expecting VkFormatProperties3 to be filled
    VkFormatProperties3 fmt_props_3 = vku::InitStructHelper();
    fmt_props_3.linearTilingFeatures = static_cast<VkFormatFeatureFlags2>(newProperties.linearTilingFeatures);
    fmt_props_3.optimalTilingFeatures = static_cast<VkFormatFeatureFlags2>(newProperties.optimalTilingFeatures);
    fmt_props_3.bufferFeatures = static_cast<VkFormatFeatureFlags2>(newProperties.bufferFeatures);
    memcpy(&(phy_dev_data->format_properties3_map[format]), &fmt_props_3, sizeof(VkFormatProperties3));
}

VKAPI_ATTR void VKAPI_CALL GetOriginalPhysicalDeviceFormatProperties2EXT(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                         VkFormatProperties2 *properties) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);
    layer_data *instance_data = GetLayerDataPtr(phy_dev_data->instance, device_profile_api_dev_data_map);
    instance_data->dispatch_table.GetPhysicalDeviceFormatProperties2(physicalDevice, format, properties);
}

VKAPI_ATTR void VKAPI_CALL SetPhysicalDeviceFormatProperties2EXT(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                 const VkFormatProperties2 newProperties) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);

    memcpy(&(phy_dev_data->format_properties_map[format]), &(newProperties.formatProperties), sizeof(VkFormatProperties));
    VkFormatProperties3 *fmt_props_3 = vku::FindStructInPNextChain<VkFormatProperties3>(newProperties.pNext);
    if (fmt_props_3) {
        memcpy(&(phy_dev_data->format_properties3_map[format]), fmt_props_3, sizeof(VkFormatProperties3));
    }
}

VKAPI_ATTR void VKAPI_CALL GetOriginalPhysicalDeviceFeaturesEXT(VkPhysicalDevice physicalDevice,
                                                                VkPhysicalDeviceFeatures *features) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);
    layer_data *instance_data = GetLayerDataPtr(phy_dev_data->instance, device_profile_api_dev_data_map);
    instance_data->dispatch_table.GetPhysicalDeviceFeatures(physicalDevice, features);
}

VKAPI_ATTR void VKAPI_CALL SetPhysicalDeviceFeaturesEXT(VkPhysicalDevice physicalDevice,
                                                        const VkPhysicalDeviceFeatures newFeatures) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);

    memcpy(&phy_dev_data->phy_device_features, &newFeatures, sizeof(VkPhysicalDeviceFeatures));
}

VKAPI_ATTR void VKAPI_CALL SetPhysicalDeviceProperties2EXT(VkPhysicalDevice physicalDevice,
                                                           const VkPhysicalDeviceProperties2 newProperties) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);

    // Will let GetPhysicalDeviceProperties2 know changes were added
    phy_dev_data->modify_properties2 = true;

    VkBaseOutStructure *next = reinterpret_cast<VkBaseOutStructure *>(newProperties.pNext);
    while (next != nullptr) {
        switch (next->sType) {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT: {
                auto *props = reinterpret_cast<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *>(next);
                phy_dev_data->blend_operation_advanced_props = *props;
                break;
            }
            default:
                break;
        }
        next = reinterpret_cast<VkBaseOutStructure *>(next->pNext);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = GetChainInfo(pCreateInfo, VK_LAYER_LINK_INFO);
    std::lock_guard<std::mutex> lock(global_lock);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fp_get_instance_proc_addr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fp_create_instance = (PFN_vkCreateInstance)fp_get_instance_proc_addr(NULL, "vkCreateInstance");
    if (fp_create_instance == NULL) return VK_ERROR_INITIALIZATION_FAILED;

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fp_create_instance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) return result;

    layer_data *instance_data = GetLayerDataPtr(*pInstance, device_profile_api_dev_data_map);
    instance_data->instance = *pInstance;
    layer_init_instance_dispatch_table(*pInstance, &instance_data->dispatch_table, fp_get_instance_proc_addr);
    instance_data->dispatch_table.GetPhysicalDeviceProcAddr =
        (PFN_GetPhysicalDeviceProcAddr)fp_get_instance_proc_addr(*pInstance, "vk_layerGetPhysicalDeviceProcAddr");

    uint32_t physical_device_count = 0;
    instance_data->dispatch_table.EnumeratePhysicalDevices(*pInstance, &physical_device_count, NULL);

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    result = instance_data->dispatch_table.EnumeratePhysicalDevices(*pInstance, &physical_device_count, physical_devices.data());
    if (result != VK_SUCCESS) return result;

    for (VkPhysicalDevice physical_device : physical_devices) {
        layer_data *phy_dev_data = GetLayerDataPtr(physical_device, device_profile_api_dev_data_map);
        instance_data->dispatch_table.GetPhysicalDeviceProperties(physical_device, &phy_dev_data->phy_device_props);
        instance_data->dispatch_table.GetPhysicalDeviceFeatures(physical_device, &phy_dev_data->phy_device_features);
        phy_dev_data->instance = *pInstance;
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    layer_data *instance_data = GetLayerDataPtr(instance, device_profile_api_dev_data_map);
    if (!instance_data) {
        return;
    }

    uint32_t physical_device_count = 0;
    instance_data->dispatch_table.EnumeratePhysicalDevices(instance, &physical_device_count, NULL);
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    VkResult result =
        instance_data->dispatch_table.EnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());
    if (result == VK_SUCCESS) {
        for (VkPhysicalDevice physical_device : physical_devices) {
            FreeLayerDataPtr(physical_device, device_profile_api_dev_data_map);
        }
    }

    instance_data->dispatch_table.DestroyInstance(instance, pAllocator);
    FreeLayerDataPtr(instance, device_profile_api_dev_data_map);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *pProperties) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);
    memcpy(pProperties, &phy_dev_data->phy_device_props, sizeof(VkPhysicalDeviceProperties));
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *pProperties) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);
    layer_data *instance_data = GetLayerDataPtr(phy_dev_data->instance, device_profile_api_dev_data_map);
    // Get original value and then update each pNext needed
    instance_data->dispatch_table.GetPhysicalDeviceProperties2(physicalDevice, pProperties);
    if (!phy_dev_data->modify_properties2) {
        return;
    }

    // Always save the original pNext and just update the values
    VkBaseOutStructure *next = reinterpret_cast<VkBaseOutStructure *>(pProperties->pNext);
    while (next != nullptr) {
        switch (next->sType) {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT: {
                auto *props = reinterpret_cast<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *>(next);
                void *original_next = next->pNext;
                *props = phy_dev_data->blend_operation_advanced_props;
                props->pNext = original_next;
                break;
            }
            default:
                break;
        }
        next = reinterpret_cast<VkBaseOutStructure *>(next->pNext);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                             VkFormatProperties *pProperties) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);
    layer_data *instance_data = GetLayerDataPtr(phy_dev_data->instance, device_profile_api_dev_data_map);
    auto device_format_map_it = phy_dev_data->format_properties_map.find(format);
    if (device_format_map_it != phy_dev_data->format_properties_map.end()) {
        memcpy(pProperties, &phy_dev_data->format_properties_map[format], sizeof(VkFormatProperties));
    } else {
        instance_data->dispatch_table.GetPhysicalDeviceFormatProperties(physicalDevice, format, pProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                              VkFormatProperties2 *pProperties) {
    VkFormatProperties3 *fmt_props_3 = vku::FindStructInPNextChain<VkFormatProperties3>(pProperties->pNext);
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);
    layer_data *instance_data = GetLayerDataPtr(phy_dev_data->instance, device_profile_api_dev_data_map);
    auto device_format_map_it = phy_dev_data->format_properties_map.find(format);
    if (device_format_map_it != phy_dev_data->format_properties_map.end()) {
        memcpy((void *)&(pProperties->formatProperties), &phy_dev_data->format_properties_map[format], sizeof(VkFormatProperties));
        if (fmt_props_3) {
            memcpy(fmt_props_3, &phy_dev_data->format_properties3_map[format], sizeof(VkFormatProperties3));
        }
    } else {
        instance_data->dispatch_table.GetPhysicalDeviceFormatProperties2(physicalDevice, format, pProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures *pFeatures) {
    std::lock_guard<std::mutex> lock(global_lock);
    layer_data *phy_dev_data = GetLayerDataPtr(physicalDevice, device_profile_api_dev_data_map);
    memcpy(pFeatures, &phy_dev_data->phy_device_features, sizeof(VkPhysicalDeviceFeatures));
}

static const VkLayerProperties device_profile_api_LayerProps = {
    "VK_LAYER_LUNARG_device_profile_api",
    VK_HEADER_VERSION_COMPLETE,             // specVersion
    1,                                      // implementationVersion
    "LunarG device profile api Layer",
};

template <typename T>
VkResult EnumerateProperties(uint32_t src_count, const T *src_props, uint32_t *dst_count, T *dst_props) {
    if (!dst_props || !src_props) {
        *dst_count = src_count;
        return VK_SUCCESS;
    }

    uint32_t copy_count = (*dst_count < src_count) ? *dst_count : src_count;
    memcpy(dst_props, src_props, sizeof(T) * copy_count);
    *dst_count = copy_count;

    return (copy_count == src_count) ? VK_SUCCESS : VK_INCOMPLETE;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return EnumerateProperties(1, &device_profile_api_LayerProps, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                    VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, device_profile_api_LayerProps.layerName))
        return EnumerateProperties<VkExtensionProperties>(0, NULL, pCount, pProperties);

    return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char *name) {
    if (!strcmp(name, "vkSetPhysicalDeviceLimitsEXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceLimitsEXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceLimitsEXT")) return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceLimitsEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceFormatPropertiesEXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceFormatPropertiesEXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceFormatPropertiesEXT"))
        return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceFormatPropertiesEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceFormatProperties2EXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceFormatProperties2EXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceFormatProperties2EXT"))
        return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceFormatProperties2EXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceFeaturesEXT")) return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceFeaturesEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceFeaturesEXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceFeaturesEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceProperties2EXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceProperties2EXT;
    layer_data *instance_data = GetLayerDataPtr(instance, device_profile_api_dev_data_map);
    auto &table = instance_data->dispatch_table;
    if (!table.GetPhysicalDeviceProcAddr) return nullptr;
    return table.GetPhysicalDeviceProcAddr(instance, name);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char *name) {
    if (!strcmp(name, "vkCreateInstance")) return (PFN_vkVoidFunction)CreateInstance;
    if (!strcmp(name, "vkDestroyInstance")) return (PFN_vkVoidFunction)DestroyInstance;
    if (!strcmp(name, "vkGetPhysicalDeviceProperties")) return (PFN_vkVoidFunction)GetPhysicalDeviceProperties;
    if (!strcmp(name, "vkGetPhysicalDeviceProperties2")) return (PFN_vkVoidFunction)GetPhysicalDeviceProperties2;
    if (!strcmp(name, "vkGetPhysicalDeviceProperties2KHR")) return (PFN_vkVoidFunction)GetPhysicalDeviceProperties2;
    if (!strcmp(name, "vkGetPhysicalDeviceFormatProperties")) return (PFN_vkVoidFunction)GetPhysicalDeviceFormatProperties;
    if (!strcmp(name, "vkGetPhysicalDeviceFormatProperties2")) return (PFN_vkVoidFunction)GetPhysicalDeviceFormatProperties2;
    if (!strcmp(name, "vkGetPhysicalDeviceFormatProperties2KHR")) return (PFN_vkVoidFunction)GetPhysicalDeviceFormatProperties2;
    if (!strcmp(name, "vkGetPhysicalDeviceFeatures")) return (PFN_vkVoidFunction)GetPhysicalDeviceFeatures;
    if (!strcmp(name, "vkGetInstanceProcAddr")) return (PFN_vkVoidFunction)GetInstanceProcAddr;
    if (!strcmp(name, "vkEnumerateInstanceExtensionProperties")) return (PFN_vkVoidFunction)EnumerateInstanceExtensionProperties;
    if (!strcmp(name, "vkEnumerateInstanceLayerProperties")) return (PFN_vkVoidFunction)EnumerateInstanceLayerProperties;
    if (!strcmp(name, "vkSetPhysicalDeviceLimitsEXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceLimitsEXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceLimitsEXT")) return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceLimitsEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceFormatPropertiesEXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceFormatPropertiesEXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceFormatPropertiesEXT"))
        return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceFormatPropertiesEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceFormatProperties2EXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceFormatProperties2EXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceFormatProperties2EXT"))
        return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceFormatProperties2EXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceFeaturesEXT")) return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceFeaturesEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceFeaturesEXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceFeaturesEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceProperties2EXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceProperties2EXT;
    if (!strcmp(name, "vk_layerGetPhysicalDeviceProcAddr")) return (PFN_vkVoidFunction)GetPhysicalDeviceProcAddr;
    assert(instance);
    layer_data *instance_data = GetLayerDataPtr(instance, device_profile_api_dev_data_map);
    auto &table = instance_data->dispatch_table;
    if (!table.GetInstanceProcAddr) return nullptr;
    return table.GetInstanceProcAddr(instance, name);
}

}  // namespace device_profile_api

#if defined(__GNUC__) && __GNUC__ >= 4
#define VVL_EXPORT __attribute__((visibility("default")))
#else
#define VVL_EXPORT
#endif

// The following functions need to match the `/DEF` and `--version-script` file
// for consistency across platforms that don't accept those linker options.
extern "C" {

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return device_profile_api::EnumerateInstanceLayerProperties(pCount, pProperties);
}

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                                 VkExtensionProperties *pProperties) {
    return device_profile_api::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VVL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return device_profile_api::GetInstanceProcAddr(instance, funcName);
}

VVL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance, const char *funcName) {
    return device_profile_api::GetPhysicalDeviceProcAddr(instance, funcName);
}

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct) {
    assert(pVersionStruct != NULL);
    assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

    // Fill in the function pointers if our version is at least capable of having the structure contain them.
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = nullptr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = vk_layerGetPhysicalDeviceProcAddr;
    }

    if (pVersionStruct->loaderLayerInterfaceVersion < CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        device_profile_api::loader_layer_if_version = pVersionStruct->loaderLayerInterfaceVersion;
    } else if (pVersionStruct->loaderLayerInterfaceVersion > CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        pVersionStruct->loaderLayerInterfaceVersion = CURRENT_LOADER_LAYER_INTERFACE_VERSION;
    }

    return VK_SUCCESS;
}

}  // extern "C"
