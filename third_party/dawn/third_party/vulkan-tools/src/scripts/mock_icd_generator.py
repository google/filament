#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
# Copyright (c) 2023-2025 RasterGrid Kft.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Author: Tobin Ehlis <tobine@google.com>
#
# This script generates a Mock ICD that intercepts almost all Vulkan
#  functions. That layer is not intended to be useful or even compilable
#  in its initial state. Rather it's intended to be a starting point that
#  can be copied and customized to assist in creation of a new layer.

import os,re,sys
from generator import *
from common_codegen import *

CUSTOM_C_INTERCEPTS = {
'vkCreateInstance': '''
    // TODO: If loader ver <=4 ICD must fail with VK_ERROR_INCOMPATIBLE_DRIVER for all vkCreateInstance calls with
    //  apiVersion set to > Vulkan 1.0 because the loader is still at interface version <= 4. Otherwise, the
    //  ICD should behave as normal.
    if (loader_interface_version <= 4) {
        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }
    *pInstance = (VkInstance)CreateDispObjHandle();
    for (auto& physical_device : physical_device_map[*pInstance])
        physical_device = (VkPhysicalDevice)CreateDispObjHandle();
    // TODO: If emulating specific device caps, will need to add intelligence here
    return VK_SUCCESS;
''',
'vkDestroyInstance': '''
    if (instance) {
        for (const auto physical_device : physical_device_map.at(instance)) {
            display_map.erase(physical_device);
            DestroyDispObjHandle((void*)physical_device);
        }
        physical_device_map.erase(instance);
        DestroyDispObjHandle((void*)instance);
    }
''',
'vkAllocateCommandBuffers': '''
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; ++i) {
        pCommandBuffers[i] = (VkCommandBuffer)CreateDispObjHandle();
        command_pool_buffer_map[pAllocateInfo->commandPool].push_back(pCommandBuffers[i]);
    }
    return VK_SUCCESS;
''',
'vkFreeCommandBuffers': '''
    unique_lock_t lock(global_lock);
    for (auto i = 0u; i < commandBufferCount; ++i) {
        if (!pCommandBuffers[i]) {
            continue;
        }

        for (auto& pair : command_pool_buffer_map) {
            auto& cbs = pair.second;
            auto it = std::find(cbs.begin(), cbs.end(), pCommandBuffers[i]);
            if (it != cbs.end()) {
                cbs.erase(it);
            }
        }

        DestroyDispObjHandle((void*) pCommandBuffers[i]);
    }
''',
'vkCreateCommandPool': '''
    unique_lock_t lock(global_lock);
    *pCommandPool = (VkCommandPool)global_unique_handle++;
    command_pool_map[device].insert(*pCommandPool);
    return VK_SUCCESS;
''',
'vkDestroyCommandPool': '''
    // destroy command buffers for this pool
    unique_lock_t lock(global_lock);
    auto it = command_pool_buffer_map.find(commandPool);
    if (it != command_pool_buffer_map.end()) {
        for (auto& cb : it->second) {
            DestroyDispObjHandle((void*) cb);
        }
        command_pool_buffer_map.erase(it);
    }
    command_pool_map[device].erase(commandPool);
''',
'vkEnumeratePhysicalDevices': '''
    VkResult result_code = VK_SUCCESS;
    if (pPhysicalDevices) {
        const auto return_count = (std::min)(*pPhysicalDeviceCount, icd_physical_device_count);
        for (uint32_t i = 0; i < return_count; ++i) pPhysicalDevices[i] = physical_device_map.at(instance)[i];
        if (return_count < icd_physical_device_count) result_code = VK_INCOMPLETE;
        *pPhysicalDeviceCount = return_count;
    } else {
        *pPhysicalDeviceCount = icd_physical_device_count;
    }
    return result_code;
''',
'vkCreateDevice': '''
    *pDevice = (VkDevice)CreateDispObjHandle();
    // TODO: If emulating specific device caps, will need to add intelligence here
    return VK_SUCCESS;
''',
'vkDestroyDevice': '''
    unique_lock_t lock(global_lock);
    // First destroy sub-device objects
    // Destroy Queues
    for (auto queue_family_map_pair : queue_map[device]) {
        for (auto index_queue_pair : queue_map[device][queue_family_map_pair.first]) {
            DestroyDispObjHandle((void*)index_queue_pair.second);
        }
    }

    for (auto& cp : command_pool_map[device]) {
        for (auto& cb : command_pool_buffer_map[cp]) {
            DestroyDispObjHandle((void*) cb);
        }
        command_pool_buffer_map.erase(cp);
    }
    command_pool_map[device].clear();

    queue_map.erase(device);
    buffer_map.erase(device);
    image_memory_size_map.erase(device);
    // Now destroy device
    DestroyDispObjHandle((void*)device);
    // TODO: If emulating specific device caps, will need to add intelligence here
''',
'vkGetDeviceQueue': '''
    unique_lock_t lock(global_lock);
    auto queue = queue_map[device][queueFamilyIndex][queueIndex];
    if (queue) {
        *pQueue = queue;
    } else {
        *pQueue = queue_map[device][queueFamilyIndex][queueIndex] = (VkQueue)CreateDispObjHandle();
    }
    // TODO: If emulating specific device caps, will need to add intelligence here
    return;
''',
'vkGetDeviceQueue2': '''
    GetDeviceQueue(device, pQueueInfo->queueFamilyIndex, pQueueInfo->queueIndex, pQueue);
    // TODO: Add further support for GetDeviceQueue2 features
''',
'vkEnumerateInstanceLayerProperties': '''
    return VK_SUCCESS;
''',
'vkEnumerateInstanceVersion': '''
    *pApiVersion = VK_HEADER_VERSION_COMPLETE;
    return VK_SUCCESS;
''',
'vkEnumerateDeviceLayerProperties': '''
    return VK_SUCCESS;
''',
'vkEnumerateInstanceExtensionProperties': '''
    // If requesting number of extensions, return that
    if (!pLayerName) {
        if (!pProperties) {
            *pPropertyCount = (uint32_t)instance_extension_map.size();
        } else {
            uint32_t i = 0;
            for (const auto &name_ver_pair : instance_extension_map) {
                if (i == *pPropertyCount) {
                    break;
                }
                std::strncpy(pProperties[i].extensionName, name_ver_pair.first.c_str(), sizeof(pProperties[i].extensionName));
                pProperties[i].extensionName[sizeof(pProperties[i].extensionName) - 1] = 0;
                pProperties[i].specVersion = name_ver_pair.second;
                ++i;
            }
            if (i != instance_extension_map.size()) {
                return VK_INCOMPLETE;
            }
        }
    }
    // If requesting extension properties, fill in data struct for number of extensions
    return VK_SUCCESS;
''',
'vkEnumerateDeviceExtensionProperties': '''
    // If requesting number of extensions, return that
    if (!pLayerName) {
        if (!pProperties) {
            *pPropertyCount = (uint32_t)device_extension_map.size();
        } else {
            uint32_t i = 0;
            for (const auto &name_ver_pair : device_extension_map) {
                if (i == *pPropertyCount) {
                    break;
                }
                std::strncpy(pProperties[i].extensionName, name_ver_pair.first.c_str(), sizeof(pProperties[i].extensionName));
                pProperties[i].extensionName[sizeof(pProperties[i].extensionName) - 1] = 0;
                pProperties[i].specVersion = name_ver_pair.second;
                ++i;
            }
            if (i != device_extension_map.size()) {
                return VK_INCOMPLETE;
            }
        }
    }
    // If requesting extension properties, fill in data struct for number of extensions
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceSurfacePresentModesKHR': '''
    // Currently always say that all present modes are supported
    if (!pPresentModes) {
        *pPresentModeCount = 6;
    } else {
        if (*pPresentModeCount >= 6) pPresentModes[5] = VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR;
        if (*pPresentModeCount >= 5) pPresentModes[4] = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR;
        if (*pPresentModeCount >= 4) pPresentModes[3] = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        if (*pPresentModeCount >= 3) pPresentModes[2] = VK_PRESENT_MODE_FIFO_KHR;
        if (*pPresentModeCount >= 2) pPresentModes[1] = VK_PRESENT_MODE_MAILBOX_KHR;
        if (*pPresentModeCount >= 1) pPresentModes[0] = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceSurfaceFormatsKHR': '''
    // Currently always say that RGBA8 & BGRA8 are supported
    if (!pSurfaceFormats) {
        *pSurfaceFormatCount = 2;
    } else {
        if (*pSurfaceFormatCount >= 2) {
            pSurfaceFormats[1].format = VK_FORMAT_R8G8B8A8_UNORM;
            pSurfaceFormats[1].colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
        if (*pSurfaceFormatCount >= 1) {
            pSurfaceFormats[0].format = VK_FORMAT_B8G8R8A8_UNORM;
            pSurfaceFormats[0].colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceSurfaceFormats2KHR': '''
    // Currently always say that RGBA8 & BGRA8 are supported
    if (!pSurfaceFormats) {
        *pSurfaceFormatCount = 2;
    } else {
        if (*pSurfaceFormatCount >= 2) {
            pSurfaceFormats[1].pNext = nullptr;
            pSurfaceFormats[1].surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
            pSurfaceFormats[1].surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
        if (*pSurfaceFormatCount >= 1) {
            pSurfaceFormats[1].pNext = nullptr;
            pSurfaceFormats[0].surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
            pSurfaceFormats[0].surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceSurfaceSupportKHR': '''
    // Currently say that all surface/queue combos are supported
    *pSupported = VK_TRUE;
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceSurfaceCapabilitiesKHR': '''
    // In general just say max supported is available for requested surface
    pSurfaceCapabilities->minImageCount = 1;
    pSurfaceCapabilities->maxImageCount = 0;
    pSurfaceCapabilities->currentExtent.width = 0xFFFFFFFF;
    pSurfaceCapabilities->currentExtent.height = 0xFFFFFFFF;
    pSurfaceCapabilities->minImageExtent.width = 1;
    pSurfaceCapabilities->minImageExtent.height = 1;
    pSurfaceCapabilities->maxImageExtent.width = 0xFFFF;
    pSurfaceCapabilities->maxImageExtent.height = 0xFFFF;
    pSurfaceCapabilities->maxImageArrayLayers = 128;
    pSurfaceCapabilities->supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR |
                                                VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR |
                                                VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR |
                                                VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR |
                                                VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR |
                                                VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR |
                                                VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR |
                                                VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR |
                                                VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR;
    pSurfaceCapabilities->currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    pSurfaceCapabilities->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
                                                    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR |
                                                    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR |
                                                    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    pSurfaceCapabilities->supportedUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_SAMPLED_BIT |
                                                VK_IMAGE_USAGE_STORAGE_BIT |
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceSurfaceCapabilities2KHR': '''
    GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, pSurfaceInfo->surface, &pSurfaceCapabilities->surfaceCapabilities);

    auto *present_mode_compatibility = lvl_find_mod_in_chain<VkSurfacePresentModeCompatibilityEXT>(pSurfaceCapabilities->pNext);
    if (present_mode_compatibility) {
        if (!present_mode_compatibility->pPresentModes) {
            present_mode_compatibility->presentModeCount = 3;
        } else {
            // arbitrary
            present_mode_compatibility->pPresentModes[0] = VK_PRESENT_MODE_IMMEDIATE_KHR;
            present_mode_compatibility->pPresentModes[1] = VK_PRESENT_MODE_FIFO_KHR;
            present_mode_compatibility->pPresentModes[2] = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR;
        }
    }
    return VK_SUCCESS;
''',
'vkGetInstanceProcAddr': '''
    if (!negotiate_loader_icd_interface_called) {
        loader_interface_version = 0;
    }
    const auto &item = name_to_funcptr_map.find(pName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }
    // Mock should intercept all functions so if we get here just return null
    return nullptr;
''',
'vkGetDeviceProcAddr': '''
    return GetInstanceProcAddr(nullptr, pName);
''',
'vkGetPhysicalDeviceMemoryProperties': '''
    pMemoryProperties->memoryTypeCount = 6;
    // Host visible Coherent
    pMemoryProperties->memoryTypes[0].propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    pMemoryProperties->memoryTypes[0].heapIndex = 0;
    // Host visible Cached
    pMemoryProperties->memoryTypes[1].propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    pMemoryProperties->memoryTypes[1].heapIndex = 0;
    // Device local and Host visible
    pMemoryProperties->memoryTypes[2].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    pMemoryProperties->memoryTypes[2].heapIndex = 1;
    // Device local lazily
    pMemoryProperties->memoryTypes[3].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    pMemoryProperties->memoryTypes[3].heapIndex = 1;
    // Device local protected
    pMemoryProperties->memoryTypes[4].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_PROTECTED_BIT;
    pMemoryProperties->memoryTypes[4].heapIndex = 1;
    // Device local only
    pMemoryProperties->memoryTypes[5].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    pMemoryProperties->memoryTypes[5].heapIndex = 1;
    pMemoryProperties->memoryHeapCount = 2;
    pMemoryProperties->memoryHeaps[0].flags = VK_MEMORY_HEAP_MULTI_INSTANCE_BIT;
    pMemoryProperties->memoryHeaps[0].size = 8000000000;
    pMemoryProperties->memoryHeaps[1].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
    pMemoryProperties->memoryHeaps[1].size = 8000000000;
''',
'vkGetPhysicalDeviceMemoryProperties2KHR': '''
    GetPhysicalDeviceMemoryProperties(physicalDevice, &pMemoryProperties->memoryProperties);
''',
'vkGetPhysicalDeviceQueueFamilyProperties': '''
    if (pQueueFamilyProperties) {
        std::vector<VkQueueFamilyProperties2KHR> props2(*pQueueFamilyPropertyCount, {
            VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2_KHR});
        GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, props2.data());
        for (uint32_t i = 0; i < *pQueueFamilyPropertyCount; ++i) {
            pQueueFamilyProperties[i] = props2[i].queueFamilyProperties;
        }
    } else {
        GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, nullptr);
    }
''',
'vkGetPhysicalDeviceQueueFamilyProperties2KHR': '''
    if (pQueueFamilyProperties) {
        if (*pQueueFamilyPropertyCount >= 1) {
            auto props = &pQueueFamilyProperties[0].queueFamilyProperties;
            props->queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT
                                | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT;
            props->queueCount = 1;
            props->timestampValidBits = 16;
            props->minImageTransferGranularity = {1,1,1};
        }
        if (*pQueueFamilyPropertyCount >= 2) {
            auto props = &pQueueFamilyProperties[1].queueFamilyProperties;
            props->queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_PROTECTED_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR;
            props->queueCount = 1;
            props->timestampValidBits = 16;
            props->minImageTransferGranularity = {1,1,1};

            auto status_query_props = lvl_find_mod_in_chain<VkQueueFamilyQueryResultStatusPropertiesKHR>(pQueueFamilyProperties[1].pNext);
            if (status_query_props) {
                status_query_props->queryResultStatusSupport = VK_TRUE;
            }
            auto video_props = lvl_find_mod_in_chain<VkQueueFamilyVideoPropertiesKHR>(pQueueFamilyProperties[1].pNext);
            if (video_props) {
                video_props->videoCodecOperations = VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR
                                                  | VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR
                                                  | VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR;
            }
        }
        if (*pQueueFamilyPropertyCount >= 3) {
            auto props = &pQueueFamilyProperties[2].queueFamilyProperties;
            props->queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_PROTECTED_BIT | VK_QUEUE_VIDEO_ENCODE_BIT_KHR;
            props->queueCount = 1;
            props->timestampValidBits = 16;
            props->minImageTransferGranularity = {1,1,1};

            auto status_query_props = lvl_find_mod_in_chain<VkQueueFamilyQueryResultStatusPropertiesKHR>(pQueueFamilyProperties[2].pNext);
            if (status_query_props) {
                status_query_props->queryResultStatusSupport = VK_TRUE;
            }
            auto video_props = lvl_find_mod_in_chain<VkQueueFamilyVideoPropertiesKHR>(pQueueFamilyProperties[2].pNext);
            if (video_props) {
                video_props->videoCodecOperations = VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR
                                                  | VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR
                                                  | VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR;
            }
        }
        if (*pQueueFamilyPropertyCount > 3) {
            *pQueueFamilyPropertyCount = 3;
        }
    } else {
        *pQueueFamilyPropertyCount = 3;
    }
''',
'vkGetPhysicalDeviceFeatures': '''
    uint32_t num_bools = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
    VkBool32 *bool_array = &pFeatures->robustBufferAccess;
    SetBoolArrayTrue(bool_array, num_bools);
''',
'vkGetPhysicalDeviceFeatures2KHR': '''
    GetPhysicalDeviceFeatures(physicalDevice, &pFeatures->features);
    uint32_t num_bools = 0; // Count number of VkBool32s in extension structs
    VkBool32* feat_bools = nullptr;
    auto vk_1_1_features = lvl_find_mod_in_chain<VkPhysicalDeviceVulkan11Features>(pFeatures->pNext);
    if (vk_1_1_features) {
        vk_1_1_features->protectedMemory = VK_TRUE;
    }
    auto vk_1_3_features = lvl_find_mod_in_chain<VkPhysicalDeviceVulkan13Features>(pFeatures->pNext);
    if (vk_1_3_features) {
        vk_1_3_features->synchronization2 = VK_TRUE;
    }
    auto prot_features = lvl_find_mod_in_chain<VkPhysicalDeviceProtectedMemoryFeatures>(pFeatures->pNext);
    if (prot_features) {
        prot_features->protectedMemory = VK_TRUE;
    }
    auto sync2_features = lvl_find_mod_in_chain<VkPhysicalDeviceSynchronization2FeaturesKHR>(pFeatures->pNext);
    if (sync2_features) {
        sync2_features->synchronization2 = VK_TRUE;
    }
    auto video_maintenance1_features = lvl_find_mod_in_chain<VkPhysicalDeviceVideoMaintenance1FeaturesKHR>(pFeatures->pNext);
    if (video_maintenance1_features) {
        video_maintenance1_features->videoMaintenance1 = VK_TRUE;
    }
    const auto *desc_idx_features = lvl_find_in_chain<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>(pFeatures->pNext);
    if (desc_idx_features) {
        const auto bool_size = sizeof(VkPhysicalDeviceDescriptorIndexingFeaturesEXT) - offsetof(VkPhysicalDeviceDescriptorIndexingFeaturesEXT, shaderInputAttachmentArrayDynamicIndexing);
        num_bools = bool_size/sizeof(VkBool32);
        feat_bools = (VkBool32*)&desc_idx_features->shaderInputAttachmentArrayDynamicIndexing;
        SetBoolArrayTrue(feat_bools, num_bools);
    }
    const auto *blendop_features = lvl_find_in_chain<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT>(pFeatures->pNext);
    if (blendop_features) {
        const auto bool_size = sizeof(VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT) - offsetof(VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT, advancedBlendCoherentOperations);
        num_bools = bool_size/sizeof(VkBool32);
        feat_bools = (VkBool32*)&blendop_features->advancedBlendCoherentOperations;
        SetBoolArrayTrue(feat_bools, num_bools);
    }
    const auto *host_image_copy_features = lvl_find_in_chain<VkPhysicalDeviceHostImageCopyFeaturesEXT>(pFeatures->pNext);
    if (host_image_copy_features) {
       feat_bools = (VkBool32*)&host_image_copy_features->hostImageCopy;
       SetBoolArrayTrue(feat_bools, 1);
    }
''',
'vkGetPhysicalDeviceFormatProperties': '''
    if (VK_FORMAT_UNDEFINED == format) {
        *pFormatProperties = { 0x0, 0x0, 0x0 };
    } else {
        // Default to a color format, skip DS bit
        *pFormatProperties = { 0x00FFFDFF, 0x00FFFDFF, 0x00FFFDFF };
        switch (format) {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_S8_UINT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                // Don't set color bits for DS formats
                *pFormatProperties = { 0x00FFFE7F, 0x00FFFE7F, 0x00FFFE7F };
                break;
            case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
            case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
            case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
            case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
            case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
                // Set decode/encode bits for these formats
                *pFormatProperties = { 0x1EFFFDFF, 0x1EFFFDFF, 0x00FFFDFF };
                break;
            default:
                break;
        }
    }
''',
'vkGetPhysicalDeviceFormatProperties2KHR': '''
    GetPhysicalDeviceFormatProperties(physicalDevice, format, &pFormatProperties->formatProperties);
    VkFormatProperties3KHR *props_3 = lvl_find_mod_in_chain<VkFormatProperties3KHR>(pFormatProperties->pNext);
    if (props_3) {
        props_3->linearTilingFeatures = pFormatProperties->formatProperties.linearTilingFeatures;
        props_3->optimalTilingFeatures = pFormatProperties->formatProperties.optimalTilingFeatures;
        props_3->bufferFeatures = pFormatProperties->formatProperties.bufferFeatures;
        props_3->optimalTilingFeatures |= VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT_EXT;
    }
''',
'vkGetPhysicalDeviceImageFormatProperties': '''
    // A hardcoded unsupported format
    if (format == VK_FORMAT_E5B9G9R9_UFLOAT_PACK32) {
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    // TODO: Just hard-coding some values for now
    // TODO: If tiling is linear, limit the mips, levels, & sample count
    if (VK_IMAGE_TILING_LINEAR == tiling) {
        *pImageFormatProperties = { { 4096, 4096, 256 }, 1, 1, VK_SAMPLE_COUNT_1_BIT, 4294967296 };
    } else {
        // We hard-code support for all sample counts except 64 bits.
        *pImageFormatProperties = { { 4096, 4096, 256 }, 12, 256, 0x7F & ~VK_SAMPLE_COUNT_64_BIT, 4294967296 };
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceImageFormatProperties2KHR': '''
    auto *external_image_prop = lvl_find_mod_in_chain<VkExternalImageFormatProperties>(pImageFormatProperties->pNext);
    auto *external_image_format = lvl_find_in_chain<VkPhysicalDeviceExternalImageFormatInfo>(pImageFormatInfo->pNext);
    if (external_image_prop && external_image_format) {
        external_image_prop->externalMemoryProperties.externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
        external_image_prop->externalMemoryProperties.compatibleHandleTypes = external_image_format->handleType;
    }

    GetPhysicalDeviceImageFormatProperties(physicalDevice, pImageFormatInfo->format, pImageFormatInfo->type, pImageFormatInfo->tiling, pImageFormatInfo->usage, pImageFormatInfo->flags, &pImageFormatProperties->imageFormatProperties);
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceSparseImageFormatProperties': '''
    if (!pProperties) {
        *pPropertyCount = 1;
    } else {
        // arbitrary
        pProperties->imageGranularity = {4, 4, 4};
        pProperties->flags = VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT;
        switch (format) {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D32_SFLOAT:
                pProperties->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            case VK_FORMAT_S8_UINT:
                pProperties->aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                break;
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                pProperties->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                break;
            default:
                pProperties->aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                break;
        }
    }
''',
'vkGetPhysicalDeviceSparseImageFormatProperties2KHR': '''
    if (pPropertyCount && pProperties) {
        GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, pFormatInfo->format, pFormatInfo->type, pFormatInfo->samples, pFormatInfo->usage, pFormatInfo->tiling, pPropertyCount, &pProperties->properties);
    } else {
        GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, pFormatInfo->format, pFormatInfo->type, pFormatInfo->samples, pFormatInfo->usage, pFormatInfo->tiling, pPropertyCount, nullptr);
    }
''',
'vkGetPhysicalDeviceProperties': '''
    pProperties->apiVersion = VK_HEADER_VERSION_COMPLETE;
    pProperties->driverVersion = 1;
    pProperties->vendorID = 0xba5eba11;
    pProperties->deviceID = 0xf005ba11;
    pProperties->deviceType = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
    //std::string devName = "Vulkan Mock Device";
    strcpy(pProperties->deviceName, "Vulkan Mock Device");
    pProperties->pipelineCacheUUID[0] = 18;
    pProperties->limits = SetLimits(&pProperties->limits);
    pProperties->sparseProperties = { VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE };
''',
'vkGetPhysicalDeviceProperties2KHR': '''
    // The only value that need to be set are those the Profile layer can't set
    // see https://github.com/KhronosGroup/Vulkan-Profiles/issues/352
    // All values set are arbitrary
    GetPhysicalDeviceProperties(physicalDevice, &pProperties->properties);

    auto *props_11 = lvl_find_mod_in_chain<VkPhysicalDeviceVulkan11Properties>(pProperties->pNext);
    if (props_11) {
        props_11->protectedNoFault = VK_FALSE;
    }

    auto *props_12 = lvl_find_mod_in_chain<VkPhysicalDeviceVulkan12Properties>(pProperties->pNext);
    if (props_12) {
        props_12->denormBehaviorIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL;
        props_12->roundingModeIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL;
    }

    auto *props_13 = lvl_find_mod_in_chain<VkPhysicalDeviceVulkan13Properties>(pProperties->pNext);
    if (props_13) {
        props_13->storageTexelBufferOffsetSingleTexelAlignment = VK_TRUE;
        props_13->uniformTexelBufferOffsetSingleTexelAlignment = VK_TRUE;
        props_13->storageTexelBufferOffsetAlignmentBytes = 16;
        props_13->uniformTexelBufferOffsetAlignmentBytes = 16;
    }

    auto *protected_memory_props = lvl_find_mod_in_chain<VkPhysicalDeviceProtectedMemoryProperties>(pProperties->pNext);
    if (protected_memory_props) {
        protected_memory_props->protectedNoFault = VK_FALSE;
    }

    auto *float_controls_props = lvl_find_mod_in_chain<VkPhysicalDeviceFloatControlsProperties>(pProperties->pNext);
    if (float_controls_props) {
        float_controls_props->denormBehaviorIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL;
        float_controls_props->roundingModeIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL;
    }

    auto *conservative_raster_props = lvl_find_mod_in_chain<VkPhysicalDeviceConservativeRasterizationPropertiesEXT>(pProperties->pNext);
    if (conservative_raster_props) {
        conservative_raster_props->primitiveOverestimationSize = 0.00195313f;
        conservative_raster_props->conservativePointAndLineRasterization = VK_TRUE;
        conservative_raster_props->degenerateTrianglesRasterized = VK_TRUE;
        conservative_raster_props->degenerateLinesRasterized = VK_TRUE;
    }

    auto *rt_pipeline_props = lvl_find_mod_in_chain<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>(pProperties->pNext);
    if (rt_pipeline_props) {
        rt_pipeline_props->shaderGroupHandleSize = 32;
        rt_pipeline_props->shaderGroupBaseAlignment = 64;
        rt_pipeline_props->shaderGroupHandleCaptureReplaySize = 32;
    }

    auto *rt_pipeline_nv_props = lvl_find_mod_in_chain<VkPhysicalDeviceRayTracingPropertiesNV>(pProperties->pNext);
    if (rt_pipeline_nv_props) {
        rt_pipeline_nv_props->shaderGroupHandleSize = 32;
        rt_pipeline_nv_props->shaderGroupBaseAlignment = 64;
    }

    auto *texel_buffer_props = lvl_find_mod_in_chain<VkPhysicalDeviceTexelBufferAlignmentProperties>(pProperties->pNext);
    if (texel_buffer_props) {
        texel_buffer_props->storageTexelBufferOffsetSingleTexelAlignment = VK_TRUE;
        texel_buffer_props->uniformTexelBufferOffsetSingleTexelAlignment = VK_TRUE;
        texel_buffer_props->storageTexelBufferOffsetAlignmentBytes = 16;
        texel_buffer_props->uniformTexelBufferOffsetAlignmentBytes = 16;
    }

    auto *descriptor_buffer_props = lvl_find_mod_in_chain<VkPhysicalDeviceDescriptorBufferPropertiesEXT>(pProperties->pNext);
    if (descriptor_buffer_props) {
        descriptor_buffer_props->combinedImageSamplerDescriptorSingleArray = VK_TRUE;
        descriptor_buffer_props->bufferlessPushDescriptors = VK_TRUE;
        descriptor_buffer_props->allowSamplerImageViewPostSubmitCreation = VK_TRUE;
        descriptor_buffer_props->descriptorBufferOffsetAlignment = 4;
    }

    auto *mesh_shader_props = lvl_find_mod_in_chain<VkPhysicalDeviceMeshShaderPropertiesEXT>(pProperties->pNext);
    if (mesh_shader_props) {
        mesh_shader_props->meshOutputPerVertexGranularity = 32;
        mesh_shader_props->meshOutputPerPrimitiveGranularity = 32;
        mesh_shader_props->prefersLocalInvocationVertexOutput = VK_TRUE;
        mesh_shader_props->prefersLocalInvocationPrimitiveOutput = VK_TRUE;
        mesh_shader_props->prefersCompactVertexOutput = VK_TRUE;
        mesh_shader_props->prefersCompactPrimitiveOutput = VK_TRUE;
    }

    auto *fragment_density_map2_props = lvl_find_mod_in_chain<VkPhysicalDeviceFragmentDensityMap2PropertiesEXT>(pProperties->pNext);
    if (fragment_density_map2_props) {
        fragment_density_map2_props->subsampledLoads = VK_FALSE;
        fragment_density_map2_props->subsampledCoarseReconstructionEarlyAccess = VK_FALSE;
        fragment_density_map2_props->maxSubsampledArrayLayers = 2;
        fragment_density_map2_props->maxDescriptorSetSubsampledSamplers = 1;
    }

    auto *maintenance3_props = lvl_find_mod_in_chain<VkPhysicalDeviceMaintenance3Properties>(pProperties->pNext);
    if (maintenance3_props) {
        maintenance3_props->maxMemoryAllocationSize = 1073741824;
        maintenance3_props->maxPerSetDescriptors = 1024;
    }

    const uint32_t num_copy_layouts = 5;
    const VkImageLayout HostCopyLayouts[]{
       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
       VK_IMAGE_LAYOUT_GENERAL,
       VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
       VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
       VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    };

    auto *host_image_copy_props = lvl_find_mod_in_chain<VkPhysicalDeviceHostImageCopyPropertiesEXT>(pProperties->pNext);
    if (host_image_copy_props){
        if (host_image_copy_props->pCopyDstLayouts == nullptr) host_image_copy_props->copyDstLayoutCount = num_copy_layouts;
        else {
            uint32_t num_layouts = (std::min)(host_image_copy_props->copyDstLayoutCount, num_copy_layouts);
            for (uint32_t i = 0; i < num_layouts; i++) {
                host_image_copy_props->pCopyDstLayouts[i] = HostCopyLayouts[i];
            }
        }
        if (host_image_copy_props->pCopySrcLayouts == nullptr) host_image_copy_props->copySrcLayoutCount = num_copy_layouts;
        else {
            uint32_t num_layouts = (std::min)(host_image_copy_props->copySrcLayoutCount, num_copy_layouts);
             for (uint32_t i = 0; i < num_layouts; i++) {
                host_image_copy_props->pCopySrcLayouts[i] = HostCopyLayouts[i];
            }
        }
    }

    auto *driver_properties = lvl_find_mod_in_chain<VkPhysicalDeviceDriverProperties>(pProperties->pNext);
    if (driver_properties) {
        std::strncpy(driver_properties->driverName, "Vulkan Mock Device", VK_MAX_DRIVER_NAME_SIZE);
#if defined(GIT_BRANCH_NAME) && defined(GIT_TAG_INFO)
        std::strncpy(driver_properties->driverInfo, "Branch: " GIT_BRANCH_NAME " Tag Info: " GIT_TAG_INFO, VK_MAX_DRIVER_INFO_SIZE);
#else
        std::strncpy(driver_properties->driverInfo, "Branch: --unknown-- Tag Info: --unknown--", VK_MAX_DRIVER_INFO_SIZE);
#endif
    }
''',
'vkGetPhysicalDeviceExternalSemaphoreProperties':'''
    // Hard code support for all handle types and features
    pExternalSemaphoreProperties->exportFromImportedHandleTypes = 0x1F;
    pExternalSemaphoreProperties->compatibleHandleTypes = 0x1F;
    pExternalSemaphoreProperties->externalSemaphoreFeatures = 0x3;
''',
'vkGetPhysicalDeviceExternalSemaphorePropertiesKHR':'''
    GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
''',
'vkGetPhysicalDeviceExternalFenceProperties':'''
    // Hard-code support for all handle types and features
    pExternalFenceProperties->exportFromImportedHandleTypes = 0xF;
    pExternalFenceProperties->compatibleHandleTypes = 0xF;
    pExternalFenceProperties->externalFenceFeatures = 0x3;
''',
'vkGetPhysicalDeviceExternalFencePropertiesKHR':'''
    GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
''',
'vkGetPhysicalDeviceExternalBufferProperties':'''
    constexpr VkExternalMemoryHandleTypeFlags supported_flags = 0x1FF;
    if (pExternalBufferInfo->handleType & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) {
        // Can't have dedicated memory with AHB
        pExternalBufferProperties->externalMemoryProperties.externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
        pExternalBufferProperties->externalMemoryProperties.exportFromImportedHandleTypes = pExternalBufferInfo->handleType;
        pExternalBufferProperties->externalMemoryProperties.compatibleHandleTypes = pExternalBufferInfo->handleType;
    } else if (pExternalBufferInfo->handleType & supported_flags) {
        pExternalBufferProperties->externalMemoryProperties.externalMemoryFeatures = 0x7;
        pExternalBufferProperties->externalMemoryProperties.exportFromImportedHandleTypes = supported_flags;
        pExternalBufferProperties->externalMemoryProperties.compatibleHandleTypes = supported_flags;
    } else {
        pExternalBufferProperties->externalMemoryProperties.externalMemoryFeatures = 0;
        pExternalBufferProperties->externalMemoryProperties.exportFromImportedHandleTypes = 0;
        // According to spec, handle type is always compatible with itself. Even if export/import
        // not supported, it's important to properly implement self-compatibility property since
        // application's control flow can rely on this.
        pExternalBufferProperties->externalMemoryProperties.compatibleHandleTypes = pExternalBufferInfo->handleType;
    }
''',
'vkGetPhysicalDeviceExternalBufferPropertiesKHR':'''
    GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
''',
'vkGetBufferMemoryRequirements': '''
    // TODO: Just hard-coding reqs for now
    pMemoryRequirements->size = 4096;
    pMemoryRequirements->alignment = 1;
    pMemoryRequirements->memoryTypeBits = 0xFFFF;
    // Return a better size based on the buffer size from the create info.
    unique_lock_t lock(global_lock);
    auto d_iter = buffer_map.find(device);
    if (d_iter != buffer_map.end()) {
        auto iter = d_iter->second.find(buffer);
        if (iter != d_iter->second.end()) {
            pMemoryRequirements->size = ((iter->second.size + 4095) / 4096) * 4096;
        }
    }
''',
'vkGetBufferMemoryRequirements2KHR': '''
    GetBufferMemoryRequirements(device, pInfo->buffer, &pMemoryRequirements->memoryRequirements);
''',
'vkGetDeviceBufferMemoryRequirements': '''
    // TODO: Just hard-coding reqs for now
    pMemoryRequirements->memoryRequirements.alignment = 1;
    pMemoryRequirements->memoryRequirements.memoryTypeBits = 0xFFFF;

    // Return a size based on the buffer size from the create info.
    pMemoryRequirements->memoryRequirements.size = ((pInfo->pCreateInfo->size + 4095) / 4096) * 4096;
''',
'vkGetDeviceBufferMemoryRequirementsKHR': '''
    GetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
''',
'vkGetImageMemoryRequirements': '''
    pMemoryRequirements->size = 0;
    pMemoryRequirements->alignment = 1;

    unique_lock_t lock(global_lock);
    auto d_iter = image_memory_size_map.find(device);
    if(d_iter != image_memory_size_map.end()){
        auto iter = d_iter->second.find(image);
        if (iter != d_iter->second.end()) {
            pMemoryRequirements->size = iter->second;
        }
    }
    // Here we hard-code that the memory type at index 3 doesn't support this image.
    pMemoryRequirements->memoryTypeBits = 0xFFFF & ~(0x1 << 3);
''',
'vkGetImageMemoryRequirements2KHR': '''
    GetImageMemoryRequirements(device, pInfo->image, &pMemoryRequirements->memoryRequirements);
''',
'vkGetDeviceImageMemoryRequirements': '''
    pMemoryRequirements->memoryRequirements.size = GetImageSizeFromCreateInfo(pInfo->pCreateInfo);
    pMemoryRequirements->memoryRequirements.alignment = 1;
    // Here we hard-code that the memory type at index 3 doesn't support this image.
    pMemoryRequirements->memoryRequirements.memoryTypeBits = 0xFFFF & ~(0x1 << 3);
''',
'vkGetDeviceImageMemoryRequirementsKHR': '''
    GetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
''',
'vkMapMemory': '''
    unique_lock_t lock(global_lock);
    if (VK_WHOLE_SIZE == size) {
        if (allocated_memory_size_map.count(memory) != 0)
            size = allocated_memory_size_map[memory] - offset;
        else
            size = 0x10000;
    }
    void* map_addr = malloc((size_t)size);
    mapped_memory_map[memory].push_back(map_addr);
    *ppData = map_addr;
    return VK_SUCCESS;
''',
'vkMapMemory2KHR': '''
    return MapMemory(device, pMemoryMapInfo->memory, pMemoryMapInfo->offset, pMemoryMapInfo->size, pMemoryMapInfo->flags, ppData);
''',
'vkUnmapMemory': '''
    unique_lock_t lock(global_lock);
    for (auto map_addr : mapped_memory_map[memory]) {
        free(map_addr);
    }
    mapped_memory_map.erase(memory);
''',
'vkUnmapMemory2KHR': '''
    UnmapMemory(device, pMemoryUnmapInfo->memory);
    return VK_SUCCESS;
''',
'vkGetImageSubresourceLayout': '''
    // Need safe values. Callers are computing memory offsets from pLayout, with no return code to flag failure.
    *pLayout = VkSubresourceLayout(); // Default constructor zero values.
''',
'vkCreateSwapchainKHR': '''
    unique_lock_t lock(global_lock);
    *pSwapchain = (VkSwapchainKHR)global_unique_handle++;
    for(uint32_t i = 0; i < icd_swapchain_image_count; ++i){
        swapchain_image_map[*pSwapchain][i] = (VkImage)global_unique_handle++;
    }
    return VK_SUCCESS;
''',
'vkDestroySwapchainKHR': '''
    unique_lock_t lock(global_lock);
    swapchain_image_map.clear();
''',
'vkGetSwapchainImagesKHR': '''
    if (!pSwapchainImages) {
        *pSwapchainImageCount = icd_swapchain_image_count;
    } else {
        unique_lock_t lock(global_lock);
        for (uint32_t img_i = 0; img_i < (std::min)(*pSwapchainImageCount, icd_swapchain_image_count); ++img_i){
            pSwapchainImages[img_i] = swapchain_image_map.at(swapchain)[img_i];
        }

        if (*pSwapchainImageCount < icd_swapchain_image_count) return VK_INCOMPLETE;
        else if (*pSwapchainImageCount > icd_swapchain_image_count) *pSwapchainImageCount = icd_swapchain_image_count;
    }
    return VK_SUCCESS;
''',
'vkAcquireNextImageKHR': '''
    *pImageIndex = 0;
    return VK_SUCCESS;
''',
'vkAcquireNextImage2KHR': '''
    *pImageIndex = 0;
    return VK_SUCCESS;
''',
'vkCreateBuffer': '''
    unique_lock_t lock(global_lock);
    *pBuffer = (VkBuffer)global_unique_handle++;
     buffer_map[device][*pBuffer] = {
         pCreateInfo->size,
         current_available_address
     };
     current_available_address += pCreateInfo->size;
     // Always align to next 64-bit pointer
     const uint64_t alignment = current_available_address % 64;
     if (alignment != 0) {
         current_available_address += (64 - alignment);
     }
    return VK_SUCCESS;
''',
'vkDestroyBuffer': '''
    unique_lock_t lock(global_lock);
    buffer_map[device].erase(buffer);
''',
'vkCreateImage': '''
    unique_lock_t lock(global_lock);
    *pImage = (VkImage)global_unique_handle++;
    image_memory_size_map[device][*pImage] = GetImageSizeFromCreateInfo(pCreateInfo);
    return VK_SUCCESS;
''',
'vkDestroyImage': '''
    unique_lock_t lock(global_lock);
    image_memory_size_map[device].erase(image);
''',
'vkEnumeratePhysicalDeviceGroupsKHR': '''
    if (!pPhysicalDeviceGroupProperties) {
        *pPhysicalDeviceGroupCount = 1;
    } else {
        // arbitrary
        pPhysicalDeviceGroupProperties->physicalDeviceCount = 1;
        pPhysicalDeviceGroupProperties->physicalDevices[0] = physical_device_map.at(instance)[0];
        pPhysicalDeviceGroupProperties->subsetAllocation = VK_FALSE;
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceMultisamplePropertiesEXT': '''
    if (pMultisampleProperties) {
        // arbitrary
        pMultisampleProperties->maxSampleLocationGridSize = {32, 32};
    }
''',
'vkGetPhysicalDeviceFragmentShadingRatesKHR': '''
    if (!pFragmentShadingRates) {
        *pFragmentShadingRateCount = 1;
    } else {
        // arbitrary
        pFragmentShadingRates->sampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT;
        pFragmentShadingRates->fragmentSize = {8, 8};
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceCalibrateableTimeDomainsEXT': '''
    if (!pTimeDomains) {
        *pTimeDomainCount = 1;
    } else {
        // arbitrary
        *pTimeDomains = VK_TIME_DOMAIN_DEVICE_EXT;
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceCalibrateableTimeDomainsKHR': '''
    if (!pTimeDomains) {
        *pTimeDomainCount = 1;
    } else {
        // arbitrary
        *pTimeDomains = VK_TIME_DOMAIN_DEVICE_KHR;
    }
    return VK_SUCCESS;
''',
'vkGetFenceWin32HandleKHR': '''
    *pHandle = (HANDLE)0x12345678;
    return VK_SUCCESS;
''',
'vkGetFenceFdKHR': '''
    *pFd = 0x42;
    return VK_SUCCESS;
''',
'vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR': '''
    if (!pCounters) {
        *pCounterCount = 3;
    } else {
        if (*pCounterCount == 0){
            return VK_INCOMPLETE;
        }
        // arbitrary
        pCounters[0].unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR;
        pCounters[0].scope = VK_QUERY_SCOPE_COMMAND_BUFFER_KHR;
        pCounters[0].storage = VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR;
        pCounters[0].uuid[0] = 0x01;
        if (*pCounterCount == 1){
            return VK_INCOMPLETE;
        }
        pCounters[1].unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR;
        pCounters[1].scope = VK_QUERY_SCOPE_RENDER_PASS_KHR;
        pCounters[1].storage = VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR;
        pCounters[1].uuid[0] = 0x02;
        if (*pCounterCount == 2){
            return VK_INCOMPLETE;
        }
        pCounters[2].unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR;
        pCounters[2].scope = VK_QUERY_SCOPE_COMMAND_KHR;
        pCounters[2].storage = VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR;
        pCounters[2].uuid[0] = 0x03;
        *pCounterCount = 3;
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR': '''
    if (pNumPasses) {
        // arbitrary
        *pNumPasses = 1;
    }
''',
'vkGetShaderModuleIdentifierEXT': '''
    if (pIdentifier) {
        // arbitrary
        pIdentifier->identifierSize = 1;
        pIdentifier->identifier[0] = 0x01;
    }
''',
'vkGetImageSparseMemoryRequirements': '''
    if (!pSparseMemoryRequirements) {
        *pSparseMemoryRequirementCount = 1;
    } else {
        // arbitrary
        pSparseMemoryRequirements->imageMipTailFirstLod = 0;
        pSparseMemoryRequirements->imageMipTailSize = 8;
        pSparseMemoryRequirements->imageMipTailOffset = 0;
        pSparseMemoryRequirements->imageMipTailStride = 4;
        pSparseMemoryRequirements->formatProperties.imageGranularity = {4, 4, 4};
        pSparseMemoryRequirements->formatProperties.flags = VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT;
        // Would need to track the VkImage to know format for better value here
        pSparseMemoryRequirements->formatProperties.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    }

''',
'vkGetImageSparseMemoryRequirements2KHR': '''
    if (pSparseMemoryRequirementCount && pSparseMemoryRequirements) {
        GetImageSparseMemoryRequirements(device, pInfo->image, pSparseMemoryRequirementCount, &pSparseMemoryRequirements->memoryRequirements);
    } else {
        GetImageSparseMemoryRequirements(device, pInfo->image, pSparseMemoryRequirementCount, nullptr);
    }
''',
'vkGetBufferDeviceAddress': '''
    VkDeviceAddress address = 0;
    auto d_iter = buffer_map.find(device);
    if (d_iter != buffer_map.end()) {
        auto iter = d_iter->second.find(pInfo->buffer);
        if (iter != d_iter->second.end()) {
            address = iter->second.address;
        }
    }
    return address;
''',
'vkGetBufferDeviceAddressKHR': '''
    return GetBufferDeviceAddress(device, pInfo);
''',
'vkGetBufferDeviceAddressEXT': '''
    return GetBufferDeviceAddress(device, pInfo);
''',
'vkGetDescriptorSetLayoutSizeEXT': '''
    // Need to give something non-zero
    *pLayoutSizeInBytes = 4;
''',
'vkGetAccelerationStructureBuildSizesKHR': '''
    // arbitrary
    pSizeInfo->accelerationStructureSize = 4;
    pSizeInfo->updateScratchSize = 4;
    pSizeInfo->buildScratchSize = 4;
''',
'vkGetAccelerationStructureMemoryRequirementsNV': '''
    // arbitrary
    pMemoryRequirements->memoryRequirements.size = 4096;
    pMemoryRequirements->memoryRequirements.alignment = 1;
    pMemoryRequirements->memoryRequirements.memoryTypeBits = 0xFFFF;
''',
'vkGetAccelerationStructureDeviceAddressKHR': '''
    // arbitrary - need to be aligned to 256 bytes
    return 0x262144;
''',
'vkGetVideoSessionMemoryRequirementsKHR': '''
    if (!pMemoryRequirements) {
        *pMemoryRequirementsCount = 1;
    } else {
        // arbitrary
        pMemoryRequirements[0].memoryBindIndex = 0;
        pMemoryRequirements[0].memoryRequirements.size = 4096;
        pMemoryRequirements[0].memoryRequirements.alignment = 1;
        pMemoryRequirements[0].memoryRequirements.memoryTypeBits = 0xFFFF;
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR': '''
    if (!pProperties) {
        *pPropertyCount = 2;
    } else {
        // arbitrary
        pProperties[0].MSize = 16;
        pProperties[0].NSize = 16;
        pProperties[0].KSize = 16;
        pProperties[0].AType = VK_COMPONENT_TYPE_UINT32_KHR;
        pProperties[0].BType = VK_COMPONENT_TYPE_UINT32_KHR;
        pProperties[0].CType = VK_COMPONENT_TYPE_UINT32_KHR;
        pProperties[0].ResultType = VK_COMPONENT_TYPE_UINT32_KHR;
        pProperties[0].saturatingAccumulation = VK_FALSE;
        pProperties[0].scope = VK_SCOPE_SUBGROUP_KHR;

        pProperties[1] = pProperties[0];
        pProperties[1].scope = VK_SCOPE_DEVICE_KHR;
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceVideoCapabilitiesKHR': '''
    // We include some reasonable set of capability combinations to cover a wide range of use cases
    auto caps = pCapabilities;
    auto caps_decode = lvl_find_mod_in_chain<VkVideoDecodeCapabilitiesKHR>(pCapabilities->pNext);
    auto caps_decode_h264 = lvl_find_mod_in_chain<VkVideoDecodeH264CapabilitiesKHR>(pCapabilities->pNext);
    auto caps_decode_h265 = lvl_find_mod_in_chain<VkVideoDecodeH265CapabilitiesKHR>(pCapabilities->pNext);
    auto caps_decode_av1 = lvl_find_mod_in_chain<VkVideoDecodeAV1CapabilitiesKHR>(pCapabilities->pNext);
    auto caps_encode = lvl_find_mod_in_chain<VkVideoEncodeCapabilitiesKHR>(pCapabilities->pNext);
    auto caps_encode_quantization_map =
        lvl_find_mod_in_chain<VkVideoEncodeQuantizationMapCapabilitiesKHR>(pCapabilities->pNext);
    auto caps_encode_h264_quantization_map =
        lvl_find_mod_in_chain<VkVideoEncodeH264QuantizationMapCapabilitiesKHR>(pCapabilities->pNext);
    auto caps_encode_h265_quantization_map =
        lvl_find_mod_in_chain<VkVideoEncodeH265QuantizationMapCapabilitiesKHR>(pCapabilities->pNext);
    auto caps_encode_av1_quantization_map =
        lvl_find_mod_in_chain<VkVideoEncodeAV1QuantizationMapCapabilitiesKHR>(pCapabilities->pNext);
    auto caps_encode_h264 = lvl_find_mod_in_chain<VkVideoEncodeH264CapabilitiesKHR>(pCapabilities->pNext);
    auto caps_encode_h265 = lvl_find_mod_in_chain<VkVideoEncodeH265CapabilitiesKHR>(pCapabilities->pNext);
    auto caps_encode_av1 = lvl_find_mod_in_chain<VkVideoEncodeAV1CapabilitiesKHR>(pCapabilities->pNext);

    switch (pVideoProfile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto profile = lvl_find_in_chain<VkVideoDecodeH264ProfileInfoKHR>(pVideoProfile->pNext);
            if (profile->stdProfileIdc != STD_VIDEO_H264_PROFILE_IDC_BASELINE &&
                profile->stdProfileIdc != STD_VIDEO_H264_PROFILE_IDC_MAIN) {
                return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }

            caps->flags = VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR;
            caps->minBitstreamBufferOffsetAlignment = 256;
            caps->minBitstreamBufferSizeAlignment   = 256;
            caps->pictureAccessGranularity          = {16,16};
            caps->minCodedExtent                    = {16,16};
            caps->maxCodedExtent                    = {1920,1080};
            caps->maxDpbSlots                       = 33;
            caps->maxActiveReferencePictures        = 32;
            std::strncpy(caps->stdHeaderVersion.extensionName, VK_STD_VULKAN_VIDEO_CODEC_H264_DECODE_EXTENSION_NAME,
                         sizeof(caps->stdHeaderVersion.extensionName));
            caps->stdHeaderVersion.specVersion      = VK_STD_VULKAN_VIDEO_CODEC_H264_DECODE_SPEC_VERSION;

            switch (pVideoProfile->chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    if (profile->pictureLayout != VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR) {
                        return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
                    }
                    caps_decode->flags = VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR;
                    caps_decode_h264->maxLevelIdc = STD_VIDEO_H264_LEVEL_IDC_6_2;
                    caps_decode_h264->fieldOffsetGranularity = {0,0};
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    if (profile->pictureLayout != VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR &&
                        profile->pictureLayout != VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR) {
                        return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
                    }
                    caps_decode->flags = VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR;
                    caps_decode_h264->maxLevelIdc = STD_VIDEO_H264_LEVEL_IDC_5_0;
                    caps_decode_h264->fieldOffsetGranularity = {0,16};
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    if (profile->pictureLayout != VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR &&
                        profile->pictureLayout != VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_INTERLEAVED_LINES_BIT_KHR) {
                        return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
                    }
                    caps_decode->flags = VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR
                                       | VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR;
                    caps_decode_h264->maxLevelIdc = STD_VIDEO_H264_LEVEL_IDC_3_2;
                    caps_decode_h264->fieldOffsetGranularity = {0,1};
                    break;
                default:
                    return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }
            break;
        }
        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            auto profile = lvl_find_in_chain<VkVideoDecodeH265ProfileInfoKHR>(pVideoProfile->pNext);
            if (profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_MAIN) {
                return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }

            caps->flags = VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR;
            caps->minBitstreamBufferOffsetAlignment = 64;
            caps->minBitstreamBufferSizeAlignment   = 64;
            caps->pictureAccessGranularity          = {32,32};
            caps->minCodedExtent                    = {48,48};
            caps->maxCodedExtent                    = {3840,2160};
            caps->maxDpbSlots                       = 16;
            caps->maxActiveReferencePictures        = 15;
            std::strncpy(caps->stdHeaderVersion.extensionName, VK_STD_VULKAN_VIDEO_CODEC_H265_DECODE_EXTENSION_NAME,
                         sizeof(caps->stdHeaderVersion.extensionName));
            caps->stdHeaderVersion.specVersion      = VK_STD_VULKAN_VIDEO_CODEC_H265_DECODE_SPEC_VERSION;

            switch (pVideoProfile->chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    caps_decode->flags = VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR;
                    caps_decode_h265->maxLevelIdc = STD_VIDEO_H265_LEVEL_IDC_6_0;
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    caps_decode->flags = VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR;
                    caps_decode_h265->maxLevelIdc = STD_VIDEO_H265_LEVEL_IDC_5_2;
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    caps_decode->flags = VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR
                                       | VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR;
                    caps_decode_h265->maxLevelIdc = STD_VIDEO_H265_LEVEL_IDC_4_1;
                    break;
                default:
                    return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }
            break;
        }
        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
            auto profile = lvl_find_in_chain<VkVideoDecodeAV1ProfileInfoKHR>(pVideoProfile->pNext);
            if (profile->stdProfile != STD_VIDEO_AV1_PROFILE_MAIN) {
                return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }

            caps->flags = VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR;
            caps->minBitstreamBufferOffsetAlignment = 256;
            caps->minBitstreamBufferSizeAlignment   = 256;
            caps->pictureAccessGranularity          = {16,16};
            caps->minCodedExtent                    = {16,16};
            caps->maxCodedExtent                    = {1920,1080};
            caps->maxDpbSlots                       = 8;
            caps->maxActiveReferencePictures        = 7;
            std::strncpy(caps->stdHeaderVersion.extensionName, VK_STD_VULKAN_VIDEO_CODEC_AV1_DECODE_EXTENSION_NAME,
                         sizeof(caps->stdHeaderVersion.extensionName));
            caps->stdHeaderVersion.specVersion      = VK_STD_VULKAN_VIDEO_CODEC_AV1_DECODE_SPEC_VERSION;

            switch (pVideoProfile->chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    caps_decode->flags = VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR;
                    caps_decode_av1->maxLevel = STD_VIDEO_AV1_LEVEL_6_2;
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    if (profile->filmGrainSupport) {
                        return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
                    }
                    caps_decode->flags = VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR;
                    caps_decode_av1->maxLevel = STD_VIDEO_AV1_LEVEL_5_0;
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    caps_decode->flags = VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR
                                       | VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR;
                    caps_decode_av1->maxLevel = STD_VIDEO_AV1_LEVEL_3_2;
                    break;
                default:
                    return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }
            break;
        }
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            auto profile = lvl_find_in_chain<VkVideoEncodeH264ProfileInfoKHR>(pVideoProfile->pNext);
            if (profile->stdProfileIdc != STD_VIDEO_H264_PROFILE_IDC_BASELINE) {
                return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }

            caps->flags = VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR;
            caps->minBitstreamBufferOffsetAlignment = 4096;
            caps->minBitstreamBufferSizeAlignment   = 4096;
            caps->pictureAccessGranularity          = {16,16};
            caps->minCodedExtent                    = {160,128};
            caps->maxCodedExtent                    = {1920,1080};
            caps->maxDpbSlots                       = 10;
            caps->maxActiveReferencePictures        = 4;
            std::strncpy(caps->stdHeaderVersion.extensionName, VK_STD_VULKAN_VIDEO_CODEC_H264_ENCODE_EXTENSION_NAME,
                         sizeof(caps->stdHeaderVersion.extensionName));
            caps->stdHeaderVersion.specVersion      = VK_STD_VULKAN_VIDEO_CODEC_H264_ENCODE_SPEC_VERSION;

            switch (pVideoProfile->chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    caps_encode->flags = VK_VIDEO_ENCODE_CAPABILITY_PRECEDING_EXTERNALLY_ENCODED_BYTES_BIT_KHR
                                       | VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR
                                       | VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR;
                    caps_encode->rateControlModes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR
                                                  | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR
                                                  | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
                    caps_encode->maxRateControlLayers = 4;
                    caps_encode->maxBitrate = 800000000;
                    caps_encode->maxQualityLevels = 4;
                    caps_encode->encodeInputPictureGranularity = {16,16};
                    caps_encode->supportedEncodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_HAS_OVERRIDES_BIT_KHR;
                    caps_encode_h264->flags = VK_VIDEO_ENCODE_H264_CAPABILITY_HRD_COMPLIANCE_BIT_KHR
                                            | VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR
                                            | VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR
                                            | VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR
                                            | VK_VIDEO_ENCODE_H264_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR
                                            | VK_VIDEO_ENCODE_H264_CAPABILITY_GENERATE_PREFIX_NALU_BIT_KHR;
                    caps_encode_h264->maxLevelIdc = STD_VIDEO_H264_LEVEL_IDC_6_2;
                    caps_encode_h264->maxSliceCount = 8;
                    caps_encode_h264->maxPPictureL0ReferenceCount = 4;
                    caps_encode_h264->maxBPictureL0ReferenceCount = 3;
                    caps_encode_h264->maxL1ReferenceCount = 2;
                    caps_encode_h264->maxTemporalLayerCount = 4;
                    caps_encode_h264->expectDyadicTemporalLayerPattern = VK_FALSE;
                    caps_encode_h264->minQp = 0;
                    caps_encode_h264->maxQp = 51;
                    caps_encode_h264->prefersGopRemainingFrames = VK_FALSE;
                    caps_encode_h264->requiresGopRemainingFrames = VK_FALSE;

                    if (caps_encode_quantization_map) {
                        caps_encode_quantization_map->maxQuantizationMapExtent = {(caps->maxCodedExtent.width + 15) / 16,
                                                                                  (caps->maxCodedExtent.height + 15) / 16};
                    }

                    if (caps_encode_h264_quantization_map) {
                        caps_encode_h264_quantization_map->minQpDelta = -26;
                        caps_encode_h264_quantization_map->maxQpDelta = +25;
                    }
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    caps_encode->flags = VK_VIDEO_ENCODE_CAPABILITY_PRECEDING_EXTERNALLY_ENCODED_BYTES_BIT_KHR
                                       | VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR;
                    caps_encode->rateControlModes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR
                                                  | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
                    caps_encode->maxRateControlLayers = 1;
                    caps_encode->maxBitrate = 480000000;
                    caps_encode->maxQualityLevels = 3;
                    caps_encode->encodeInputPictureGranularity = {32,32};
                    caps_encode->supportedEncodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;
                    caps_encode_h264->flags = VK_VIDEO_ENCODE_H264_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_KHR
                                            | VK_VIDEO_ENCODE_H264_CAPABILITY_PER_SLICE_CONSTANT_QP_BIT_KHR
                                            | VK_VIDEO_ENCODE_H264_CAPABILITY_GENERATE_PREFIX_NALU_BIT_KHR;
                    caps_encode_h264->maxLevelIdc = STD_VIDEO_H264_LEVEL_IDC_6_1;
                    caps_encode_h264->maxSliceCount = 4;
                    caps_encode_h264->maxPPictureL0ReferenceCount = 4;
                    caps_encode_h264->maxBPictureL0ReferenceCount = 0;
                    caps_encode_h264->maxL1ReferenceCount = 0;
                    caps_encode_h264->maxTemporalLayerCount = 4;
                    caps_encode_h264->expectDyadicTemporalLayerPattern = VK_TRUE;
                    caps_encode_h264->minQp = 0;
                    caps_encode_h264->maxQp = 30;
                    caps_encode_h264->prefersGopRemainingFrames = VK_TRUE;
                    caps_encode_h264->requiresGopRemainingFrames = VK_FALSE;

                    if (caps_encode_quantization_map) {
                        caps_encode_quantization_map->maxQuantizationMapExtent = {(caps->maxCodedExtent.width + 15) / 16,
                                                                                  (caps->maxCodedExtent.height + 15) / 16};
                    }

                    if (caps_encode_h264_quantization_map) {
                        caps_encode_h264_quantization_map->minQpDelta = 0;
                        caps_encode_h264_quantization_map->maxQpDelta = 0;
                    }
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    caps_encode->flags = 0;
                    caps_encode->rateControlModes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR;
                    caps_encode->maxRateControlLayers = 1;
                    caps_encode->maxBitrate = 240000000;
                    caps_encode->maxQualityLevels = 1;
                    caps_encode->encodeInputPictureGranularity = {1,1};
                    caps_encode->supportedEncodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;
                    caps_encode_h264->flags = VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR
                                            | VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR
                                            | VK_VIDEO_ENCODE_H264_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR;
                    caps_encode_h264->maxLevelIdc = STD_VIDEO_H264_LEVEL_IDC_5_1;
                    caps_encode_h264->maxSliceCount = 1;
                    caps_encode_h264->maxPPictureL0ReferenceCount = 0;
                    caps_encode_h264->maxBPictureL0ReferenceCount = 2;
                    caps_encode_h264->maxL1ReferenceCount = 2;
                    caps_encode_h264->maxTemporalLayerCount = 1;
                    caps_encode_h264->expectDyadicTemporalLayerPattern = VK_FALSE;
                    caps_encode_h264->minQp = 5;
                    caps_encode_h264->maxQp = 40;
                    caps_encode_h264->prefersGopRemainingFrames = VK_TRUE;
                    caps_encode_h264->requiresGopRemainingFrames = VK_TRUE;

                    if (caps_encode_quantization_map) {
                        caps_encode_quantization_map->maxQuantizationMapExtent = {0, 0};
                    }

                    if (caps_encode_h264_quantization_map) {
                        caps_encode_h264_quantization_map->minQpDelta = 0;
                        caps_encode_h264_quantization_map->maxQpDelta = 0;
                    }
                    break;
                default:
                    return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }
            break;
        }
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            auto profile = lvl_find_in_chain<VkVideoEncodeH265ProfileInfoKHR>(pVideoProfile->pNext);
            if (profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_MAIN) {
                return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }

            caps->flags = VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR;
            caps->minBitstreamBufferOffsetAlignment = 1;
            caps->minBitstreamBufferSizeAlignment   = 1;
            caps->pictureAccessGranularity          = {8,8};
            caps->minCodedExtent                    = {64,48};
            caps->maxCodedExtent                    = {4096,2560};
            caps->maxDpbSlots                       = 8;
            caps->maxActiveReferencePictures        = 2;
            std::strncpy(caps->stdHeaderVersion.extensionName, VK_STD_VULKAN_VIDEO_CODEC_H265_ENCODE_EXTENSION_NAME, sizeof(caps->stdHeaderVersion.extensionName));
            caps->stdHeaderVersion.specVersion      = VK_STD_VULKAN_VIDEO_CODEC_H265_ENCODE_SPEC_VERSION;

            switch (pVideoProfile->chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    caps_encode->flags = VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR;
                    caps_encode->rateControlModes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR;
                    caps_encode->maxRateControlLayers = 1;
                    caps_encode->maxBitrate = 800000000;
                    caps_encode->maxQualityLevels = 1;
                    caps_encode->encodeInputPictureGranularity = {64,64};
                    caps_encode->supportedEncodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;
                    caps_encode_h265->flags = VK_VIDEO_ENCODE_H265_CAPABILITY_HRD_COMPLIANCE_BIT_KHR
                                            | VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR
                                            | VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR
                                            | VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR
                                            | VK_VIDEO_ENCODE_H265_CAPABILITY_PER_SLICE_SEGMENT_CONSTANT_QP_BIT_KHR
                                            | VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_SLICE_SEGMENTS_PER_TILE_BIT_KHR;
                    caps_encode_h265->maxLevelIdc = STD_VIDEO_H265_LEVEL_IDC_6_2;
                    caps_encode_h265->maxSliceSegmentCount = 8;
                    caps_encode_h265->maxTiles = {1,1};
                    caps_encode_h265->ctbSizes = VK_VIDEO_ENCODE_H265_CTB_SIZE_32_BIT_KHR
                                               | VK_VIDEO_ENCODE_H265_CTB_SIZE_64_BIT_KHR;
                    caps_encode_h265->transformBlockSizes = VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_4_BIT_KHR
                                                          | VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_8_BIT_KHR
                                                          | VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_32_BIT_KHR;
                    caps_encode_h265->maxPPictureL0ReferenceCount = 4;
                    caps_encode_h265->maxBPictureL0ReferenceCount = 3;
                    caps_encode_h265->maxL1ReferenceCount = 2;
                    caps_encode_h265->maxSubLayerCount = 1;
                    caps_encode_h265->expectDyadicTemporalSubLayerPattern = VK_FALSE;
                    caps_encode_h265->minQp = 16;
                    caps_encode_h265->maxQp = 32;
                    caps_encode_h265->prefersGopRemainingFrames = VK_FALSE;
                    caps_encode_h265->requiresGopRemainingFrames = VK_FALSE;

                    if (caps_encode_quantization_map) {
                        caps_encode_quantization_map->maxQuantizationMapExtent = {(caps->maxCodedExtent.width + 3) / 4,
                                                                                  (caps->maxCodedExtent.height + 3) / 4};
                    }

                    if (caps_encode_h265_quantization_map) {
                        caps_encode_h265_quantization_map->minQpDelta = -16;
                        caps_encode_h265_quantization_map->maxQpDelta = +15;
                    }
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    caps_encode->flags = VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR;
                    caps_encode->rateControlModes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;
                    caps_encode->maxRateControlLayers = 0;
                    caps_encode->maxBitrate = 480000000;
                    caps_encode->maxQualityLevels = 2;
                    caps_encode->encodeInputPictureGranularity = {32,32};
                    caps_encode->supportedEncodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;
                    caps_encode_h265->flags = VK_VIDEO_ENCODE_H265_CAPABILITY_DIFFERENT_SLICE_SEGMENT_TYPE_BIT_KHR;
                    caps_encode_h265->maxLevelIdc = STD_VIDEO_H265_LEVEL_IDC_6_1;
                    caps_encode_h265->maxSliceSegmentCount = 4;
                    caps_encode_h265->maxTiles = {2,2};
                    caps_encode_h265->ctbSizes = VK_VIDEO_ENCODE_H265_CTB_SIZE_16_BIT_KHR
                                               | VK_VIDEO_ENCODE_H265_CTB_SIZE_64_BIT_KHR;
                    caps_encode_h265->transformBlockSizes = VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_8_BIT_KHR
                                                          | VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_16_BIT_KHR
                                                          | VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_32_BIT_KHR;
                    caps_encode_h265->maxPPictureL0ReferenceCount = 4;
                    caps_encode_h265->maxBPictureL0ReferenceCount = 0;
                    caps_encode_h265->maxL1ReferenceCount = 0;
                    caps_encode_h265->maxSubLayerCount = 1;
                    caps_encode_h265->expectDyadicTemporalSubLayerPattern = VK_FALSE;
                    caps_encode_h265->minQp = 0;
                    caps_encode_h265->maxQp = 51;
                    caps_encode_h265->prefersGopRemainingFrames = VK_TRUE;
                    caps_encode_h265->requiresGopRemainingFrames = VK_FALSE;

                    if (caps_encode_quantization_map) {
                        caps_encode_quantization_map->maxQuantizationMapExtent = {(caps->maxCodedExtent.width + 31) / 32,
                                                                                  (caps->maxCodedExtent.height + 31) / 32};
                    }

                    if (caps_encode_h265_quantization_map) {
                        caps_encode_h265_quantization_map->minQpDelta = 0;
                        caps_encode_h265_quantization_map->maxQpDelta = 0;
                    }
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    caps_encode->flags = VK_VIDEO_ENCODE_CAPABILITY_PRECEDING_EXTERNALLY_ENCODED_BYTES_BIT_KHR;
                    caps_encode->rateControlModes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR
                                                  | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR
                                                  | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
                    caps_encode->maxRateControlLayers = 2;
                    caps_encode->maxBitrate = 240000000;
                    caps_encode->maxQualityLevels = 3;
                    caps_encode->encodeInputPictureGranularity = {16,16};
                    caps_encode->supportedEncodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_HAS_OVERRIDES_BIT_KHR;
                    caps_encode_h265->flags = VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR
                                            | VK_VIDEO_ENCODE_H265_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR
                                            | VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_TILES_PER_SLICE_SEGMENT_BIT_KHR;
                    caps_encode_h265->maxLevelIdc = STD_VIDEO_H265_LEVEL_IDC_5_1;
                    caps_encode_h265->maxSliceSegmentCount = 1;
                    caps_encode_h265->maxTiles = {2,2};
                    caps_encode_h265->ctbSizes = VK_VIDEO_ENCODE_H265_CTB_SIZE_32_BIT_KHR;
                    caps_encode_h265->transformBlockSizes = VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_32_BIT_KHR;
                    caps_encode_h265->maxPPictureL0ReferenceCount = 0;
                    caps_encode_h265->maxBPictureL0ReferenceCount = 2;
                    caps_encode_h265->maxL1ReferenceCount = 2;
                    caps_encode_h265->maxSubLayerCount = 4;
                    caps_encode_h265->expectDyadicTemporalSubLayerPattern = VK_TRUE;
                    caps_encode_h265->minQp = 16;
                    caps_encode_h265->maxQp = 51;
                    caps_encode_h265->prefersGopRemainingFrames = VK_TRUE;
                    caps_encode_h265->requiresGopRemainingFrames = VK_TRUE;

                    if (caps_encode_quantization_map) {
                        caps_encode_quantization_map->maxQuantizationMapExtent = {0, 0};
                    }

                    if (caps_encode_h265_quantization_map) {
                        caps_encode_h265_quantization_map->minQpDelta = 0;
                        caps_encode_h265_quantization_map->maxQpDelta = 0;
                    }
                    break;
                default:
                    return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }
            break;
        }
        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
            auto profile = lvl_find_in_chain<VkVideoEncodeAV1ProfileInfoKHR>(pVideoProfile->pNext);
            if (profile->stdProfile != STD_VIDEO_AV1_PROFILE_MAIN) {
                return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }

            caps->flags = VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR;
            caps->minBitstreamBufferOffsetAlignment = 1;
            caps->minBitstreamBufferSizeAlignment   = 1;
            caps->pictureAccessGranularity          = {8,8};
            caps->minCodedExtent                    = {192,128};
            caps->maxCodedExtent                    = {4096,2560};
            caps->maxDpbSlots                       = 8;
            caps->maxActiveReferencePictures        = 2;
            std::strncpy(caps->stdHeaderVersion.extensionName, VK_STD_VULKAN_VIDEO_CODEC_AV1_ENCODE_EXTENSION_NAME, sizeof(caps->stdHeaderVersion.extensionName));
            caps->stdHeaderVersion.specVersion      = VK_STD_VULKAN_VIDEO_CODEC_AV1_ENCODE_SPEC_VERSION;

            switch (pVideoProfile->chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    caps_encode->flags = VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR
                                       | VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR;
                    caps_encode->rateControlModes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR;
                    caps_encode->maxRateControlLayers = 1;
                    caps_encode->maxBitrate = 800000000;
                    caps_encode->maxQualityLevels = 1;
                    caps_encode->encodeInputPictureGranularity = {64,64};
                    caps_encode->supportedEncodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;
                    caps_encode_av1->flags = VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR;
                    caps_encode_av1->maxLevel = STD_VIDEO_AV1_LEVEL_6_2;
                    caps_encode_av1->maxTiles = {1,1};
                    caps_encode_av1->minTileSize = {64,64};
                    caps_encode_av1->maxTileSize = {4096,2560};
                    caps_encode_av1->superblockSizes = VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_64_BIT_KHR;
                    caps_encode_av1->maxSingleReferenceCount = 1;
                    caps_encode_av1->singleReferenceNameMask = 0x7B;
                    caps_encode_av1->maxUnidirectionalCompoundReferenceCount = 0;
                    caps_encode_av1->maxUnidirectionalCompoundGroup1ReferenceCount = 0;
                    caps_encode_av1->unidirectionalCompoundReferenceNameMask = 0x00;
                    caps_encode_av1->maxBidirectionalCompoundReferenceCount = 0;
                    caps_encode_av1->maxBidirectionalCompoundGroup1ReferenceCount = 0;
                    caps_encode_av1->maxBidirectionalCompoundGroup2ReferenceCount = 0;
                    caps_encode_av1->bidirectionalCompoundReferenceNameMask = 0x00;
                    caps_encode_av1->maxTemporalLayerCount = 1;
                    caps_encode_av1->maxSpatialLayerCount = 1;
                    caps_encode_av1->maxOperatingPoints = 1;
                    caps_encode_av1->minQIndex = 32;
                    caps_encode_av1->maxQIndex = 128;
                    caps_encode_av1->prefersGopRemainingFrames = VK_FALSE;
                    caps_encode_av1->requiresGopRemainingFrames = VK_FALSE;

                    if (caps_encode_quantization_map) {
                        caps_encode_quantization_map->maxQuantizationMapExtent = {(caps->maxCodedExtent.width + 7) / 8,
                                                                                  (caps->maxCodedExtent.height + 7) / 8};
                    }

                    if (caps_encode_av1_quantization_map) {
                        caps_encode_av1_quantization_map->minQIndexDelta = -64;
                        caps_encode_av1_quantization_map->maxQIndexDelta = +64;
                    }
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    caps_encode->flags = VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR;
                    caps_encode->rateControlModes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;
                    caps_encode->maxRateControlLayers = 0;
                    caps_encode->maxBitrate = 480000000;
                    caps_encode->maxQualityLevels = 2;
                    caps_encode->encodeInputPictureGranularity = {32,32};
                    caps_encode->supportedEncodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;
                    caps_encode_av1->flags = VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR
                                           | VK_VIDEO_ENCODE_AV1_CAPABILITY_GENERATE_OBU_EXTENSION_HEADER_BIT_KHR
                                           | VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR;
                    caps_encode_av1->maxLevel = STD_VIDEO_AV1_LEVEL_6_1;
                    caps_encode_av1->maxTiles = {2,2};
                    caps_encode_av1->minTileSize = {128,128};
                    caps_encode_av1->maxTileSize = {4096,2048};
                    caps_encode_av1->superblockSizes = VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_64_BIT_KHR
                                                     | VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_128_BIT_KHR;
                    caps_encode_av1->maxSingleReferenceCount = 0;
                    caps_encode_av1->singleReferenceNameMask = 0x00;
                    caps_encode_av1->maxUnidirectionalCompoundReferenceCount = 2;
                    caps_encode_av1->maxUnidirectionalCompoundGroup1ReferenceCount = 2;
                    caps_encode_av1->unidirectionalCompoundReferenceNameMask = 0x5F;
                    caps_encode_av1->maxBidirectionalCompoundReferenceCount = 2;
                    caps_encode_av1->maxBidirectionalCompoundGroup1ReferenceCount = 2;
                    caps_encode_av1->maxBidirectionalCompoundGroup2ReferenceCount = 2;
                    caps_encode_av1->bidirectionalCompoundReferenceNameMask = 0x5F;
                    caps_encode_av1->maxTemporalLayerCount = 4;
                    caps_encode_av1->maxSpatialLayerCount = 1;
                    caps_encode_av1->maxOperatingPoints = 4;
                    caps_encode_av1->minQIndex = 0;
                    caps_encode_av1->maxQIndex = 255;
                    caps_encode_av1->prefersGopRemainingFrames = VK_TRUE;
                    caps_encode_av1->requiresGopRemainingFrames = VK_FALSE;

                    if (caps_encode_quantization_map) {
                        caps_encode_quantization_map->maxQuantizationMapExtent = {(caps->maxCodedExtent.width + 63) / 64,
                                                                                  (caps->maxCodedExtent.height + 63) / 64};
                    }

                    if (caps_encode_av1_quantization_map) {
                        caps_encode_av1_quantization_map->minQIndexDelta = -255;
                        caps_encode_av1_quantization_map->maxQIndexDelta = +255;
                    }
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    caps_encode->flags = VK_VIDEO_ENCODE_CAPABILITY_PRECEDING_EXTERNALLY_ENCODED_BYTES_BIT_KHR
                                       | VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR;
                    caps_encode->rateControlModes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR
                                                  | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR
                                                  | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
                    caps_encode->maxRateControlLayers = 2;
                    caps_encode->maxBitrate = 240000000;
                    caps_encode->maxQualityLevels = 3;
                    caps_encode->encodeInputPictureGranularity = {16,16};
                    caps_encode->supportedEncodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR
                                                              | VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_HAS_OVERRIDES_BIT_KHR;
                    caps_encode_av1->flags = VK_VIDEO_ENCODE_AV1_CAPABILITY_PER_RATE_CONTROL_GROUP_MIN_MAX_Q_INDEX_BIT_KHR
                                           | VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR
                                           | VK_VIDEO_ENCODE_AV1_CAPABILITY_MOTION_VECTOR_SCALING_BIT_KHR;
                    caps_encode_av1->maxLevel = STD_VIDEO_AV1_LEVEL_5_1;
                    caps_encode_av1->maxTiles = {4,4};
                    caps_encode_av1->minTileSize = {128,128};
                    caps_encode_av1->maxTileSize = {2048,2048};
                    caps_encode_av1->superblockSizes = VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_128_BIT_KHR;
                    caps_encode_av1->maxSingleReferenceCount = 1;
                    caps_encode_av1->singleReferenceNameMask = 0x5F;
                    caps_encode_av1->maxUnidirectionalCompoundReferenceCount = 4;
                    caps_encode_av1->maxUnidirectionalCompoundGroup1ReferenceCount = 4;
                    caps_encode_av1->unidirectionalCompoundReferenceNameMask = 0x5B;
                    caps_encode_av1->maxBidirectionalCompoundReferenceCount = 0;
                    caps_encode_av1->maxBidirectionalCompoundGroup1ReferenceCount = 0;
                    caps_encode_av1->maxBidirectionalCompoundGroup2ReferenceCount = 0;
                    caps_encode_av1->bidirectionalCompoundReferenceNameMask = 0x00;
                    caps_encode_av1->maxTemporalLayerCount = 4;
                    caps_encode_av1->maxSpatialLayerCount = 2;
                    caps_encode_av1->maxOperatingPoints = 2;
                    caps_encode_av1->minQIndex = 16;
                    caps_encode_av1->maxQIndex = 96;
                    caps_encode_av1->prefersGopRemainingFrames = VK_TRUE;
                    caps_encode_av1->requiresGopRemainingFrames = VK_TRUE;

                    if (caps_encode_quantization_map) {
                        caps_encode_quantization_map->maxQuantizationMapExtent = {(caps->maxCodedExtent.width + 127) / 128,
                                                                                  (caps->maxCodedExtent.height + 127) / 128};
                    }

                    if (caps_encode_av1_quantization_map) {
                        caps_encode_av1_quantization_map->minQIndexDelta = -64;
                        caps_encode_av1_quantization_map->maxQIndexDelta = +63;
                    }
                    break;
                default:
                    return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }
            break;
        }
        default:
            break;
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceVideoFormatPropertiesKHR': '''
    // We include some reasonable set of format combinations to cover a wide range of use cases
    auto profile_list = lvl_find_in_chain<VkVideoProfileListInfoKHR>(pVideoFormatInfo->pNext);
    if (profile_list->profileCount != 1) {
        return VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR;
    }

    struct VideoFormatProperties {
        VkVideoFormatPropertiesKHR props;
        VkVideoFormatQuantizationMapPropertiesKHR props_quantization_map;
        VkVideoFormatH265QuantizationMapPropertiesKHR props_h265_quantization_map;
        VkVideoFormatAV1QuantizationMapPropertiesKHR props_av1_quantization_map;
    };

    std::vector<VideoFormatProperties> format_props{};

    VideoFormatProperties fmt = {};
    fmt.props.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR;
    fmt.props.imageCreateFlags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_ALIAS_BIT |
                                 VK_IMAGE_CREATE_EXTENDED_USAGE_BIT | VK_IMAGE_CREATE_PROTECTED_BIT | VK_IMAGE_CREATE_DISJOINT_BIT;
    fmt.props.imageType = VK_IMAGE_TYPE_2D;
    fmt.props.imageTiling = VK_IMAGE_TILING_OPTIMAL;
    fmt.props_quantization_map.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR;
    fmt.props_h265_quantization_map.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR;
    fmt.props_av1_quantization_map.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR;

    // Populate DPB and input/output formats
    switch (profile_list->pProfiles[0].videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
            switch (profile_list->pProfiles[0].chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    fmt.props.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
                    fmt.props.imageUsageFlags =
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                                VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
                    format_props.push_back(fmt);
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
                    fmt.props.format = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
                    format_props.push_back(fmt);
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                                VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props.format = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16;
                    format_props.push_back(fmt);
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    fmt.props.format = VK_FORMAT_G8_B8R8_2PLANE_444_UNORM;
                    fmt.props.imageUsageFlags =
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                                VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
                    format_props.push_back(fmt);
                    break;
                default:
                    return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }
            break;
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
            switch (profile_list->pProfiles[0].chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    fmt.props.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
                    fmt.props.imageUsageFlags =
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                                VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
                    format_props.push_back(fmt);
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    fmt.props.format = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                                VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props.format = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16;
                    format_props.push_back(fmt);
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    fmt.props.format = VK_FORMAT_G8_B8R8_2PLANE_444_UNORM;
                    fmt.props.imageUsageFlags =
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                                VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
                    format_props.push_back(fmt);
                    break;
                default:
                    return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
            }
            break;

        default:
            break;
    }

    // Populate quantization map formats
    fmt.props.imageCreateFlags = VK_IMAGE_CREATE_PROTECTED_BIT;
    fmt.props.imageTiling = VK_IMAGE_TILING_LINEAR;
    switch (profile_list->pProfiles[0].videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            switch (profile_list->pProfiles[0].chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    fmt.props.format = VK_FORMAT_R32_SINT;
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;
                    fmt.props_quantization_map.quantizationMapTexelSize = {16, 16};
                    format_props.push_back(fmt);
                    fmt.props.format = VK_FORMAT_R8_UNORM;
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR;
                    format_props.push_back(fmt);
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    fmt.props.format = VK_FORMAT_R8_UNORM;
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR;
                    fmt.props_quantization_map.quantizationMapTexelSize = {16, 16};
                    format_props.push_back(fmt);
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    break;
                default:
                    break;
            }
            break;
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            switch (profile_list->pProfiles[0].chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    fmt.props.format = VK_FORMAT_R8_UNORM;
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR;
                    fmt.props_quantization_map.quantizationMapTexelSize = {4, 4};
                    fmt.props_h265_quantization_map.compatibleCtbSizes =
                        VK_VIDEO_ENCODE_H265_CTB_SIZE_32_BIT_KHR | VK_VIDEO_ENCODE_H265_CTB_SIZE_64_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props_quantization_map.quantizationMapTexelSize = {8, 8};
                    format_props.push_back(fmt);
                    fmt.props_quantization_map.quantizationMapTexelSize = {32, 32};
                    format_props.push_back(fmt);
                    fmt.props_quantization_map.quantizationMapTexelSize = {64, 64};
                    fmt.props_h265_quantization_map.compatibleCtbSizes = VK_VIDEO_ENCODE_H265_CTB_SIZE_64_BIT_KHR;
                    format_props.push_back(fmt);
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    fmt.props.format = VK_FORMAT_R32_SINT;
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;
                    fmt.props_quantization_map.quantizationMapTexelSize = {32, 32};
                    fmt.props_h265_quantization_map.compatibleCtbSizes =
                        VK_VIDEO_ENCODE_H265_CTB_SIZE_32_BIT_KHR | VK_VIDEO_ENCODE_H265_CTB_SIZE_64_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props_quantization_map.quantizationMapTexelSize = {64, 64};
                    fmt.props_h265_quantization_map.compatibleCtbSizes = VK_VIDEO_ENCODE_H265_CTB_SIZE_64_BIT_KHR;
                    format_props.push_back(fmt);
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    break;
                default:
                    break;
            }
            break;
        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
            switch (profile_list->pProfiles[0].chromaSubsampling) {
                case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
                    fmt.props.format = VK_FORMAT_R32_SINT;
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;
                    fmt.props_quantization_map.quantizationMapTexelSize = {8, 8};
                    fmt.props_av1_quantization_map.compatibleSuperblockSizes = VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_64_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props.format = VK_FORMAT_R8_UNORM;
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR;
                    fmt.props_quantization_map.quantizationMapTexelSize = {64, 64};
                    fmt.props_av1_quantization_map.compatibleSuperblockSizes = VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_64_BIT_KHR;
                    format_props.push_back(fmt);
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
                    fmt.props.format = VK_FORMAT_R32_SINT;
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;
                    fmt.props_quantization_map.quantizationMapTexelSize = {64, 64};
                    fmt.props_av1_quantization_map.compatibleSuperblockSizes = VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_64_BIT_KHR;
                    format_props.push_back(fmt);
                    fmt.props_quantization_map.quantizationMapTexelSize = {128, 128};
                    fmt.props_av1_quantization_map.compatibleSuperblockSizes = VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_128_BIT_KHR;
                    format_props.push_back(fmt);
                    break;
                case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
                    fmt.props.format = VK_FORMAT_R8_UNORM;
                    fmt.props.imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR;
                    fmt.props_quantization_map.quantizationMapTexelSize = {128, 128};
                    fmt.props_av1_quantization_map.compatibleSuperblockSizes = VK_VIDEO_ENCODE_AV1_SUPERBLOCK_SIZE_128_BIT_KHR;
                    format_props.push_back(fmt);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    std::vector<VideoFormatProperties> filtered;
    for (const auto& format : format_props) {
        if ((pVideoFormatInfo->imageUsage & format.props.imageUsageFlags) == pVideoFormatInfo->imageUsage) {
            filtered.push_back(format);
        }
    }

    if (pVideoFormatProperties != nullptr) {
        for (uint32_t i = 0; i < (std::min)(*pVideoFormatPropertyCount, (uint32_t)filtered.size()); ++i) {
            void* saved_pNext = pVideoFormatProperties[i].pNext;
            pVideoFormatProperties[i] = filtered[i].props;
            pVideoFormatProperties[i].pNext = saved_pNext;

            auto* props_quantization_map = lvl_find_mod_in_chain<VkVideoFormatQuantizationMapPropertiesKHR>(saved_pNext);
            auto* props_h265_quantization_map = lvl_find_mod_in_chain<VkVideoFormatH265QuantizationMapPropertiesKHR>(saved_pNext);
            auto* props_av1_quantization_map = lvl_find_mod_in_chain<VkVideoFormatAV1QuantizationMapPropertiesKHR>(saved_pNext);

            if (props_quantization_map != nullptr) {
                saved_pNext = props_quantization_map->pNext;
                *props_quantization_map = filtered[i].props_quantization_map;
                props_quantization_map->pNext = saved_pNext;
            }

            if (props_h265_quantization_map != nullptr) {
                saved_pNext = props_h265_quantization_map->pNext;
                *props_h265_quantization_map = filtered[i].props_h265_quantization_map;
                props_h265_quantization_map->pNext = saved_pNext;
            }

            if (props_av1_quantization_map != nullptr) {
                saved_pNext = props_av1_quantization_map->pNext;
                *props_av1_quantization_map = filtered[i].props_av1_quantization_map;
                props_av1_quantization_map->pNext = saved_pNext;
            }
        }
    }
    *pVideoFormatPropertyCount = (uint32_t)filtered.size();
    return VK_SUCCESS;
''',
'vkGetDescriptorSetLayoutSupport':'''
    if (pSupport) {
        pSupport->supported = VK_TRUE;
    }
''',
'vkGetDescriptorSetLayoutSupportKHR':'''
    GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
''',
'vkGetRenderAreaGranularity': '''
    pGranularity->width = 1;
    pGranularity->height = 1;
''',
'vkGetMemoryFdKHR': '''
    *pFd = 1;
    return VK_SUCCESS;
''',
'vkGetMemoryHostPointerPropertiesEXT': '''
    pMemoryHostPointerProperties->memoryTypeBits = 1 << 5; // DEVICE_LOCAL only type
    return VK_SUCCESS;
''',
'vkGetAndroidHardwareBufferPropertiesANDROID': '''
    pProperties->allocationSize = 65536;
    pProperties->memoryTypeBits = 1 << 5; // DEVICE_LOCAL only type

    auto *format_prop = lvl_find_mod_in_chain<VkAndroidHardwareBufferFormatPropertiesANDROID>(pProperties->pNext);
    if (format_prop) {
        // Likley using this format
        format_prop->format = VK_FORMAT_R8G8B8A8_UNORM;
        format_prop->externalFormat = 37;
    }

    auto *format_resolve_prop = lvl_find_mod_in_chain<VkAndroidHardwareBufferFormatResolvePropertiesANDROID>(pProperties->pNext);
    if (format_resolve_prop) {
        format_resolve_prop->colorAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    }
    return VK_SUCCESS;
''',
'vkGetPhysicalDeviceDisplayPropertiesKHR': '''
    if (!pProperties) {
        *pPropertyCount = 1;
    } else {
        unique_lock_t lock(global_lock);
        pProperties[0].display = (VkDisplayKHR)global_unique_handle++;
        display_map[physicalDevice].insert(pProperties[0].display);
    }
    return VK_SUCCESS;
''',
'vkRegisterDisplayEventEXT': '''
    unique_lock_t lock(global_lock);
    *pFence = (VkFence)global_unique_handle++;
    return VK_SUCCESS;
''',
'vkQueueSubmit': '''
    // Special way to cause DEVICE_LOST
    // Picked VkExportFenceCreateInfo because needed some struct that wouldn't get cleared by validation Safe Struct
    // ... TODO - It would be MUCH nicer to have a layer or other setting control when this occured
    // For now this is used to allow Validation Layers test reacting to device losts
    if (submitCount > 0 && pSubmits) {
        auto pNext = reinterpret_cast<const VkBaseInStructure *>(pSubmits[0].pNext);
        if (pNext && pNext->sType == VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO && pNext->pNext == nullptr) {
            return VK_ERROR_DEVICE_LOST;
        }
    }
    return VK_SUCCESS;
''',
'vkGetMemoryWin32HandlePropertiesKHR': '''
    pMemoryWin32HandleProperties->memoryTypeBits = 0xFFFF;
    return VK_SUCCESS;
''',
'vkCreatePipelineBinariesKHR': '''
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < pBinaries->pipelineBinaryCount; ++i) {
        pBinaries->pPipelineBinaries[i] = (VkPipelineBinaryKHR)global_unique_handle++;
    }
    return VK_SUCCESS;
'''
}

# MockICDGeneratorOptions - subclass of GeneratorOptions.
#
# Adds options used by MockICDOutputGenerator objects during Mock
# ICD generation.
#
# Additional members
#   prefixText - list of strings to prefix generated header with
#     (usually a copyright statement + calling convention macros).
#   protectFile - True if multiple inclusion protection should be
#     generated (based on the filename) around the entire header.
#   protectFeature - True if #ifndef..#endif protection should be
#     generated around a feature interface in the header file.
#   genFuncPointers - True if function pointer typedefs should be
#     generated
#   protectProto - If conditional protection should be generated
#     around prototype declarations, set to either '#ifdef'
#     to require opt-in (#ifdef protectProtoStr) or '#ifndef'
#     to require opt-out (#ifndef protectProtoStr). Otherwise
#     set to None.
#   protectProtoStr - #ifdef/#ifndef symbol to use around prototype
#     declarations, if protectProto is set
#   apicall - string to use for the function declaration prefix,
#     such as APICALL on Windows.
#   apientry - string to use for the calling convention macro,
#     in typedefs, such as APIENTRY.
#   apientryp - string to use for the calling convention macro
#     in function pointer typedefs, such as APIENTRYP.
#   indentFuncProto - True if prototype declarations should put each
#     parameter on a separate line
#   indentFuncPointer - True if typedefed function pointers should put each
#     parameter on a separate line
#   alignFuncParam - if nonzero and parameters are being put on a
#     separate line, align parameter names at the specified column
class MockICDGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 conventions = None,
                 filename = None,
                 directory = '.',
                 genpath = None,
                 apiname = None,
                 profile = None,
                 versions = '.*',
                 emitversions = '.*',
                 defaultExtensions = None,
                 addExtensions = None,
                 removeExtensions = None,
                 emitExtensions = None,
                 sortProcedure = regSortFeatures,
                 prefixText = "",
                 genFuncPointers = True,
                 protectFile = True,
                 protectFeature = True,
                 protectProto = None,
                 protectProtoStr = None,
                 apicall = '',
                 apientry = '',
                 apientryp = '',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 0,
                 expandEnumerants = True,
                 helper_file_type = ''):
        GeneratorOptions.__init__(self,
                 conventions = conventions,
                 filename = filename,
                 directory = directory,
                 genpath = genpath,
                 apiname = apiname,
                 profile = profile,
                 versions = versions,
                 emitversions = emitversions,
                 defaultExtensions = defaultExtensions,
                 addExtensions = addExtensions,
                 removeExtensions = removeExtensions,
                 emitExtensions = emitExtensions,
                 sortProcedure = sortProcedure)
        self.prefixText      = prefixText
        self.genFuncPointers = genFuncPointers
        self.protectFile     = protectFile
        self.protectFeature  = protectFeature
        self.protectProto    = protectProto
        self.protectProtoStr = protectProtoStr
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp
        self.indentFuncProto = indentFuncProto
        self.indentFuncPointer = indentFuncPointer
        self.alignFuncParam  = alignFuncParam

# MockICDOutputGenerator - subclass of OutputGenerator.
# Generates a mock vulkan ICD.
#  This is intended to be a minimal replacement for a vulkan device in order
#  to enable Vulkan Validation testing.
#
# ---- methods ----
# MockOutputGenerator(errFile, warnFile, diagFile) - args as for
#   OutputGenerator. Defines additional internal state.
# ---- methods overriding base class ----
# beginFile(genOpts)
# endFile()
# beginFeature(interface, emit)
# endFeature()
# genType(typeinfo,name)
# genStruct(typeinfo,name)
# genGroup(groupinfo,name)
# genEnum(enuminfo, name)
# genCmd(cmdinfo)
class MockICDOutputGenerator(OutputGenerator):
    """Generate specified API interfaces in a specific style, such as a C header"""
    # This is an ordered list of sections in the header file.
    TYPE_SECTIONS = ['include', 'define', 'basetype', 'handle', 'enum',
                     'group', 'bitmask', 'funcpointer', 'struct']
    ALL_SECTIONS = TYPE_SECTIONS + ['command']
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        # Internal state - accumulators for different inner block text
        self.sections = dict([(section, []) for section in self.ALL_SECTIONS])
        self.intercepts = []
        self.function_declarations = False

    # Check if the parameter passed in is a pointer to an array
    def paramIsArray(self, param):
        return param.attrib.get('len') is not None

    # Check if the parameter passed in is a pointer
    def paramIsPointer(self, param):
        ispointer = False
        for elem in param:
            if ((elem.tag != 'type') and (elem.tail is not None)) and '*' in elem.tail:
                ispointer = True
        return ispointer

    # Check if an object is a non-dispatchable handle
    def isHandleTypeNonDispatchable(self, handletype):
        handle = self.registry.tree.find("types/type/[name='" + handletype + "'][@category='handle']")
        if handle is not None and handle.find('type').text == 'VK_DEFINE_NON_DISPATCHABLE_HANDLE':
            return True
        else:
            return False

    # Check if an object is a dispatchable handle
    def isHandleTypeDispatchable(self, handletype):
        handle = self.registry.tree.find("types/type/[name='" + handletype + "'][@category='handle']")
        if handle is not None and handle.find('type').text == 'VK_DEFINE_HANDLE':
            return True
        else:
            return False

    # Check that the target API is in the supported list for the extension
    def checkExtensionAPISupport(self, supported):
        return self.genOpts.apiname in supported.split(',')

    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        # C-specific
        #
        # Multiple inclusion protection & C++ namespace.
        if (genOpts.protectFile and self.genOpts.filename == "function_declarations.h"):
            self.function_declarations = True

        # User-supplied prefix text, if any (list of strings)
        if (genOpts.prefixText):
            for s in genOpts.prefixText:
                write(s, file=self.outFile)

        if self.function_declarations:
            self.newline()
            # Include all of the extensions in ICD except specific ignored ones
            device_exts = []
            instance_exts = []
            # Ignore extensions that ICDs should not implement or are not safe to report
            ignore_exts = ['VK_EXT_validation_cache', 'VK_KHR_portability_subset']
            for ext in self.registry.tree.findall("extensions/extension"):
                if self.checkExtensionAPISupport(ext.attrib['supported']): # Only include API-relevant extensions
                    if (ext.attrib['name'] not in ignore_exts):
                        # Search for extension version enum
                        for enum in ext.findall('require/enum'):
                            if enum.get('name', '').endswith('_SPEC_VERSION'):
                                ext_version = enum.get('value')
                                if (ext.attrib.get('type') == 'instance'):
                                    instance_exts.append('    {"%s", %s},' % (ext.attrib['name'], ext_version))
                                else:
                                    device_exts.append('    {"%s", %s},' % (ext.attrib['name'], ext_version))
                                break
            write('#pragma once\n',file=self.outFile)
            write('#include <stdint.h>',file=self.outFile)
            write('#include <cstring>',file=self.outFile)
            write('#include <string>',file=self.outFile)
            write('#include <unordered_map>',file=self.outFile)
            write('#include <vulkan/vulkan.h>',file=self.outFile)
            self.newline()
            write('namespace vkmock {\n', file=self.outFile)
            write('// Map of instance extension name to version', file=self.outFile)
            write('static const std::unordered_map<std::string, uint32_t> instance_extension_map = {', file=self.outFile)
            write('\n'.join(instance_exts), file=self.outFile)
            write('};', file=self.outFile)
            write('// Map of device extension name to version', file=self.outFile)
            write('static const std::unordered_map<std::string, uint32_t> device_extension_map = {', file=self.outFile)
            write('\n'.join(device_exts), file=self.outFile)
            write('};', file=self.outFile)
        else:
            write('#pragma once\n',file=self.outFile)
            write('#include "mock_icd.h"',file=self.outFile)
            write('#include "function_declarations.h"\n',file=self.outFile)
            write('namespace vkmock {', file=self.outFile)


    def endFile(self):
        # C-specific
        # Finish C++ namespace
        self.newline()
        if self.function_declarations:
            # record intercepted procedures
            write('// Map of all APIs to be intercepted by this layer', file=self.outFile)
            write('static const std::unordered_map<std::string, void*> name_to_funcptr_map = {', file=self.outFile)
            write('\n'.join(self.intercepts), file=self.outFile)
            write('};\n', file=self.outFile)
        write('} // namespace vkmock', file=self.outFile)
        self.newline()

        # Finish processing in superclass
        OutputGenerator.endFile(self)
    def beginFeature(self, interface, emit):
        #write('// starting beginFeature', file=self.outFile)
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)
        self.featureExtraProtect = GetFeatureProtect(interface)
        # C-specific
        # Accumulate includes, defines, types, enums, function pointer typedefs,
        # end function prototypes separately for this feature. They're only
        # printed in endFeature().
        self.sections = dict([(section, []) for section in self.ALL_SECTIONS])
        #write('// ending beginFeature', file=self.outFile)
    def endFeature(self):
        # C-specific
        # Actually write the interface to the output file.
        #write('// starting endFeature', file=self.outFile)
        if (self.emit):
            self.newline()
            if (self.genOpts.protectFeature):
                write('#ifndef', self.featureName, file=self.outFile)
            # If type declarations are needed by other features based on
            # this one, it may be necessary to suppress the ExtraProtect,
            # or move it below the 'for section...' loop.
            #write('// endFeature looking at self.featureExtraProtect', file=self.outFile)
            if (self.featureExtraProtect != None):
                write('#ifdef', self.featureExtraProtect, file=self.outFile)
            #write('#define', self.featureName, '1', file=self.outFile)
            for section in self.TYPE_SECTIONS:
                #write('// endFeature writing section'+section, file=self.outFile)
                contents = self.sections[section]
                if contents:
                    write('\n'.join(contents), file=self.outFile)
                    self.newline()
            #write('// endFeature looking at self.sections[command]', file=self.outFile)
            if (self.sections['command']):
                write('\n'.join(self.sections['command']), end=u'', file=self.outFile)
                self.newline()
            if (self.featureExtraProtect != None):
                write('#endif /*', self.featureExtraProtect, '*/', file=self.outFile)
            if (self.genOpts.protectFeature):
                write('#endif /*', self.featureName, '*/', file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFeature(self)
        #write('// ending endFeature', file=self.outFile)
    #
    # Append a definition to the specified section
    def appendSection(self, section, text):
        # self.sections[section].append('SECTION: ' + section + '\n')
        self.sections[section].append(text)
    #
    # Type generation
    def genType(self, typeinfo, name, alias):
        pass
    #
    # Struct (e.g. C "struct" type) generation.
    # This is a special case of the <type> tag where the contents are
    # interpreted as a set of <member> tags instead of freeform C
    # C type declarations. The <member> tags are just like <param>
    # tags - they are a declaration of a struct or union member.
    # Only simple member declarations are supported (no nested
    # structs etc.)
    def genStruct(self, typeinfo, typeName, alias):
        OutputGenerator.genStruct(self, typeinfo, typeName, alias)
        body = 'typedef ' + typeinfo.elem.get('category') + ' ' + typeName + ' {\n'
        # paramdecl = self.makeCParamDecl(typeinfo.elem, self.genOpts.alignFuncParam)
        for member in typeinfo.elem.findall('.//member'):
            body += self.makeCParamDecl(member, self.genOpts.alignFuncParam)
            body += ';\n'
        body += '} ' + typeName + ';\n'
        self.appendSection('struct', body)
    #
    # Group (e.g. C "enum" type) generation.
    # These are concatenated together with other types.
    def genGroup(self, groupinfo, groupName, alias):
        pass
    # Enumerant generation
    # <enum> tags may specify their values in several ways, but are usually
    # just integers.
    def genEnum(self, enuminfo, name, alias):
        pass
    #
    # Command generation
    def genCmd(self, cmdinfo, name, alias):
        decls = self.makeCDecls(cmdinfo.elem)
        if self.function_declarations: # In the header declare all intercepts
            self.appendSection('command', '')
            self.appendSection('command', 'static %s' % (decls[0]))
            if (self.featureExtraProtect != None):
                self.intercepts += [ '#ifdef %s' % self.featureExtraProtect ]
            self.intercepts += [ '    {"%s", (void*)%s},' % (name,name[2:]) ]
            if (self.featureExtraProtect != None):
                self.intercepts += [ '#endif' ]
            return

        manual_functions = [
            # Include functions here to be intercepted w/ manually implemented function bodies
            'vkGetDeviceProcAddr',
            'vkGetInstanceProcAddr',
            'vkCreateDevice',
            'vkDestroyDevice',
            'vkCreateInstance',
            'vkDestroyInstance',
            'vkFreeCommandBuffers',
            'vkAllocateCommandBuffers',
            'vkDestroyCommandPool',
            #'vkCreateDebugReportCallbackEXT',
            #'vkDestroyDebugReportCallbackEXT',
            'vkEnumerateInstanceLayerProperties',
            'vkEnumerateInstanceVersion',
            'vkEnumerateInstanceExtensionProperties',
            'vkEnumerateDeviceLayerProperties',
            'vkEnumerateDeviceExtensionProperties',
        ]
        if name in manual_functions:
            self.appendSection('command', '')
            if name not in CUSTOM_C_INTERCEPTS:
                self.appendSection('command', '// declare only')
                self.appendSection('command', 'static %s' % (decls[0]))
                self.appendSection('command', '// TODO: Implement custom intercept body')
            else:
                self.appendSection('command', 'static %s' % (decls[0][:-1]))
                self.appendSection('command', '{\n%s}' % (CUSTOM_C_INTERCEPTS[name]))
            self.intercepts += [ '    {"%s", (void*)%s},' % (name,name[2:]) ]
            return
        # record that the function will be intercepted
        if (self.featureExtraProtect != None):
            self.intercepts += [ '#ifdef %s' % self.featureExtraProtect ]
        self.intercepts += [ '    {"%s", (void*)%s},' % (name,name[2:]) ]
        if (self.featureExtraProtect != None):
            self.intercepts += [ '#endif' ]

        OutputGenerator.genCmd(self, cmdinfo, name, alias)
        #
        self.appendSection('command', '')
        self.appendSection('command', 'static %s' % (decls[0][:-1]))
        if name in CUSTOM_C_INTERCEPTS:
            self.appendSection('command', '{%s}' % (CUSTOM_C_INTERCEPTS[name]))
            return

        # Declare result variable, if any.
        resulttype = cmdinfo.elem.find('proto/type')
        if (resulttype != None and resulttype.text == 'void'):
            resulttype = None
        # if the name w/ KHR postfix is in the CUSTOM_C_INTERCEPTS
        # Call the KHR custom version instead of generating separate code
        khr_name = name + "KHR"
        if khr_name in CUSTOM_C_INTERCEPTS:
            return_string = ''
            if resulttype != None:
                return_string = 'return '
            params = cmdinfo.elem.findall('param/name')
            param_names = []
            for param in params:
                param_names.append(param.text)
            self.appendSection('command', '{\n    %s%s(%s);\n}' % (return_string, khr_name[2:], ", ".join(param_names)))
            return
        self.appendSection('command', '{')

        api_function_name = cmdinfo.elem.attrib.get('name')
        # GET THE TYPE OF FUNCTION
        if any(api_function_name.startswith(ftxt) for ftxt in ('vkCreate', 'vkAllocate')):
            # Get last param
            last_param = cmdinfo.elem.findall('param')[-1]
            lp_txt = last_param.find('name').text
            lp_len = None
            if ('len' in last_param.attrib):
                lp_len = last_param.attrib['len']
                lp_len = lp_len.replace('::', '->')
            lp_type = last_param.find('type').text
            handle_type = 'dispatchable'
            allocator_txt = 'CreateDispObjHandle()';
            if (self.isHandleTypeNonDispatchable(lp_type)):
                handle_type = 'non-' + handle_type
                allocator_txt = 'global_unique_handle++';
            # Need to lock in both cases
            self.appendSection('command', '    unique_lock_t lock(global_lock);')
            if (lp_len != None):
                #print("%s last params (%s) has len %s" % (handle_type, lp_txt, lp_len))
                self.appendSection('command', '    for (uint32_t i = 0; i < %s; ++i) {' % (lp_len))
                self.appendSection('command', '        %s[i] = (%s)%s;' % (lp_txt, lp_type, allocator_txt))
                self.appendSection('command', '    }')
            else:
                #print("Single %s last param is '%s' w/ type '%s'" % (handle_type, lp_txt, lp_type))
                if 'AllocateMemory' in api_function_name:
                    # Store allocation size in case it's mapped
                    self.appendSection('command', '    allocated_memory_size_map[(VkDeviceMemory)global_unique_handle] = pAllocateInfo->allocationSize;')
                self.appendSection('command', '    *%s = (%s)%s;' % (lp_txt, lp_type, allocator_txt))
        elif True in [ftxt in api_function_name for ftxt in ['Destroy', 'Free']]:
            self.appendSection('command', '//Destroy object')
            if 'FreeMemory' in api_function_name:
                # If the memory is mapped, unmap it
                self.appendSection('command', '    UnmapMemory(device, memory);')
                # Remove from allocation map
                self.appendSection('command', '    unique_lock_t lock(global_lock);')
                self.appendSection('command', '    allocated_memory_size_map.erase(memory);')
        else:
            self.appendSection('command', '//Not a CREATE or DESTROY function')

        # Return result variable, if any.
        if (resulttype != None):
            if api_function_name == 'vkGetEventStatus':
                self.appendSection('command', '    return VK_EVENT_SET;')
            else:
                self.appendSection('command', '    return VK_SUCCESS;')
        self.appendSection('command', '}')
    #
    # override makeProtoName to drop the "vk" prefix
    def makeProtoName(self, name, tail):
        return self.genOpts.apientry + name[2:] + tail
