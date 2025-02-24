/*
** Copyright (c) 2015-2025 The Khronos Group Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/*
** This header is generated from the Khronos Vulkan XML API Registry.
**
*/

#pragma once

#include "mock_icd.h"
#include "function_declarations.h"

namespace vkmock {


static VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance)
{

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
}

static VKAPI_ATTR void VKAPI_CALL DestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator)
{

    if (instance) {
        for (const auto physical_device : physical_device_map.at(instance)) {
            display_map.erase(physical_device);
            DestroyDispObjHandle((void*)physical_device);
        }
        physical_device_map.erase(instance);
        DestroyDispObjHandle((void*)instance);
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures)
{
    uint32_t num_bools = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
    VkBool32 *bool_array = &pFeatures->robustBufferAccess;
    SetBoolArrayTrue(bool_array, num_bools);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties)
{
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
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName)
{

    if (!negotiate_loader_icd_interface_called) {
        loader_interface_version = 0;
    }
    const auto &item = name_to_funcptr_map.find(pName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }
    // Mock should intercept all functions so if we get here just return null
    return nullptr;
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName)
{

    return GetInstanceProcAddr(nullptr, pName);
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice)
{

    *pDevice = (VkDevice)CreateDispObjHandle();
    // TODO: If emulating specific device caps, will need to add intelligence here
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator)
{

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
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties)
{

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
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties)
{

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
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties)
{

    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties)
{

    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue)
{
    unique_lock_t lock(global_lock);
    auto queue = queue_map[device][queueFamilyIndex][queueIndex];
    if (queue) {
        *pQueue = queue;
    } else {
        *pQueue = queue_map[device][queueFamilyIndex][queueIndex] = (VkQueue)CreateDispObjHandle();
    }
    // TODO: If emulating specific device caps, will need to add intelligence here
    return;
}

static VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL QueueWaitIdle(
    VkQueue                                     queue)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL DeviceWaitIdle(
    VkDevice                                    device)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL AllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory)
{
    unique_lock_t lock(global_lock);
    allocated_memory_size_map[(VkDeviceMemory)global_unique_handle] = pAllocateInfo->allocationSize;
    *pMemory = (VkDeviceMemory)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL FreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
    UnmapMemory(device, memory);
    unique_lock_t lock(global_lock);
    allocated_memory_size_map.erase(memory);
}

static VKAPI_ATTR VkResult VKAPI_CALL MapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData)
{
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
}

static VKAPI_ATTR void VKAPI_CALL UnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory)
{
    unique_lock_t lock(global_lock);
    for (auto map_addr : mapped_memory_map[memory]) {
        free(map_addr);
    }
    mapped_memory_map.erase(memory);
}

static VKAPI_ATTR VkResult VKAPI_CALL FlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL InvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements)
{
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

}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL QueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence)
{
    unique_lock_t lock(global_lock);
    *pFence = (VkFence)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL ResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL WaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore)
{
    unique_lock_t lock(global_lock);
    *pSemaphore = (VkSemaphore)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent)
{
    unique_lock_t lock(global_lock);
    *pEvent = (VkEvent)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL GetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event)
{
//Not a CREATE or DESTROY function
    return VK_EVENT_SET;
}

static VKAPI_ATTR VkResult VKAPI_CALL SetEvent(
    VkDevice                                    device,
    VkEvent                                     event)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL ResetEvent(
    VkDevice                                    device,
    VkEvent                                     event)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool)
{
    unique_lock_t lock(global_lock);
    *pQueryPool = (VkQueryPool)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL GetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer)
{
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
}

static VKAPI_ATTR void VKAPI_CALL DestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator)
{
    unique_lock_t lock(global_lock);
    buffer_map[device].erase(buffer);
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView)
{
    unique_lock_t lock(global_lock);
    *pView = (VkBufferView)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage)
{
    unique_lock_t lock(global_lock);
    *pImage = (VkImage)global_unique_handle++;
    image_memory_size_map[device][*pImage] = GetImageSizeFromCreateInfo(pCreateInfo);
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator)
{
    unique_lock_t lock(global_lock);
    image_memory_size_map[device].erase(image);
}

static VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout)
{
    // Need safe values. Callers are computing memory offsets from pLayout, with no return code to flag failure.
    *pLayout = VkSubresourceLayout(); // Default constructor zero values.
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView)
{
    unique_lock_t lock(global_lock);
    *pView = (VkImageView)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule)
{
    unique_lock_t lock(global_lock);
    *pShaderModule = (VkShaderModule)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache)
{
    unique_lock_t lock(global_lock);
    *pPipelineCache = (VkPipelineCache)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL MergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines)
{
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        pPipelines[i] = (VkPipeline)global_unique_handle++;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines)
{
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        pPipelines[i] = (VkPipeline)global_unique_handle++;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout)
{
    unique_lock_t lock(global_lock);
    *pPipelineLayout = (VkPipelineLayout)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler)
{
    unique_lock_t lock(global_lock);
    *pSampler = (VkSampler)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout)
{
    unique_lock_t lock(global_lock);
    *pSetLayout = (VkDescriptorSetLayout)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool)
{
    unique_lock_t lock(global_lock);
    *pDescriptorPool = (VkDescriptorPool)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets)
{
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; ++i) {
        pDescriptorSets[i] = (VkDescriptorSet)global_unique_handle++;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL FreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets)
{
//Destroy object
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer)
{
    unique_lock_t lock(global_lock);
    *pFramebuffer = (VkFramebuffer)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass)
{
    unique_lock_t lock(global_lock);
    *pRenderPass = (VkRenderPass)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL GetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity)
{
    pGranularity->width = 1;
    pGranularity->height = 1;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool)
{
    unique_lock_t lock(global_lock);
    *pCommandPool = (VkCommandPool)global_unique_handle++;
    command_pool_map[device].insert(*pCommandPool);
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator)
{

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
}

static VKAPI_ATTR VkResult VKAPI_CALL ResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers)
{

    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; ++i) {
        pCommandBuffers[i] = (VkCommandBuffer)CreateDispObjHandle();
        command_pool_buffer_map[pAllocateInfo->commandPool].push_back(pCommandBuffers[i]);
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers)
{

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
}

static VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL EndCommandBuffer(
    VkCommandBuffer                             commandBuffer)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL ResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4])
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdWaitEvents(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass(
    VkCommandBuffer                             commandBuffer)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceVersion(
    uint32_t*                                   pApiVersion)
{

    *pApiVersion = VK_HEADER_VERSION_COMPLETE;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties)
{
    return EnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

static VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    GetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

static VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    GetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

static VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements)
{
    GetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures)
{
    GetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties)
{
    GetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties)
{
    GetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties)
{
    return GetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties)
{
    GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties)
{
    GetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties)
{
    GetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

static VKAPI_ATTR void VKAPI_CALL TrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue)
{
    GetDeviceQueue(device, pQueueInfo->queueFamilyIndex, pQueueInfo->queueIndex, pQueue);
    // TODO: Add further support for GetDeviceQueue2 features
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion)
{
    unique_lock_t lock(global_lock);
    *pYcbcrConversion = (VkSamplerYcbcrConversion)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate)
{
    unique_lock_t lock(global_lock);
    *pDescriptorUpdateTemplate = (VkDescriptorUpdateTemplate)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties)
{
    // Hard-code support for all handle types and features
    pExternalFenceProperties->exportFromImportedHandleTypes = 0xF;
    pExternalFenceProperties->compatibleHandleTypes = 0xF;
    pExternalFenceProperties->externalFenceFeatures = 0x3;
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties)
{
    // Hard code support for all handle types and features
    pExternalSemaphoreProperties->exportFromImportedHandleTypes = 0x1F;
    pExternalSemaphoreProperties->compatibleHandleTypes = 0x1F;
    pExternalSemaphoreProperties->externalSemaphoreFeatures = 0x3;
}

static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport)
{
    if (pSupport) {
        pSupport->supported = VK_TRUE;
    }
}


static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass)
{
    unique_lock_t lock(global_lock);
    *pRenderPass = (VkRenderPass)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL ResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL WaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL SignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo)
{
    VkDeviceAddress address = 0;
    auto d_iter = buffer_map.find(device);
    if (d_iter != buffer_map.end()) {
        auto iter = d_iter->second.find(pInfo->buffer);
        if (iter != d_iter->second.end()) {
            address = iter->second.address;
        }
    }
    return address;
}

static VKAPI_ATTR uint64_t VKAPI_CALL GetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR uint64_t VKAPI_CALL GetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceToolProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolProperties*             pToolProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreatePrivateDataSlot(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot)
{
    unique_lock_t lock(global_lock);
    *pPrivateDataSlot = (VkPrivateDataSlot)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyPrivateDataSlot(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL SetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetEvent2(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdResetEvent2(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdWaitEvents2(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier2(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp2(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit2(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBlitImage2(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdResolveImage2(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBeginRendering(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndRendering(
    VkCommandBuffer                             commandBuffer)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCullMode(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetFrontFace(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveTopology(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportWithCount(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetScissorWithCount(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers2(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthWriteEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthCompareOp(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBoundsTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilOp(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizerDiscardEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBiasEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveRestartEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceBufferMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    // TODO: Just hard-coding reqs for now
    pMemoryRequirements->memoryRequirements.alignment = 1;
    pMemoryRequirements->memoryRequirements.memoryTypeBits = 0xFFFF;

    // Return a size based on the buffer size from the create info.
    pMemoryRequirements->memoryRequirements.size = ((pInfo->pCreateInfo->size + 4095) / 4096) * 4096;
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    pMemoryRequirements->memoryRequirements.size = GetImageSizeFromCreateInfo(pInfo->pCreateInfo);
    pMemoryRequirements->memoryRequirements.alignment = 1;
    // Here we hard-code that the memory type at index 3 doesn't support this image.
    pMemoryRequirements->memoryRequirements.memoryTypeBits = 0xFFFF & ~(0x1 << 3);
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageSparseMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL CmdSetLineStipple(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL MapMemory2(
    VkDevice                                    device,
    const VkMemoryMapInfo*                      pMemoryMapInfo,
    void**                                      ppData)
{
    return MapMemory2KHR(device, pMemoryMapInfo, ppData);
}

static VKAPI_ATTR VkResult VKAPI_CALL UnmapMemory2(
    VkDevice                                    device,
    const VkMemoryUnmapInfo*                    pMemoryUnmapInfo)
{
    return UnmapMemory2KHR(device, pMemoryUnmapInfo);
}

static VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer2(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkIndexType                                 indexType)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetRenderingAreaGranularity(
    VkDevice                                    device,
    const VkRenderingAreaInfo*                  pRenderingAreaInfo,
    VkExtent2D*                                 pGranularity)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageSubresourceLayout(
    VkDevice                                    device,
    const VkDeviceImageSubresourceInfo*         pInfo,
    VkSubresourceLayout2*                       pLayout)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout2(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2*                  pSubresource,
    VkSubresourceLayout2*                       pLayout)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSet(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplate(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRenderingAttachmentLocations(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingAttachmentLocationInfo*    pLocationInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRenderingInputAttachmentIndices(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInputAttachmentIndexInfo*  pInputAttachmentIndexInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets2(
    VkCommandBuffer                             commandBuffer,
    const VkBindDescriptorSetsInfo*             pBindDescriptorSetsInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPushConstants2(
    VkCommandBuffer                             commandBuffer,
    const VkPushConstantsInfo*                  pPushConstantsInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSet2(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetInfo*              pPushDescriptorSetInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetWithTemplateInfo*  pPushDescriptorSetWithTemplateInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToImage(
    VkDevice                                    device,
    const VkCopyMemoryToImageInfo*              pCopyMemoryToImageInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyImageToMemory(
    VkDevice                                    device,
    const VkCopyImageToMemoryInfo*              pCopyImageToMemoryInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyImageToImage(
    VkDevice                                    device,
    const VkCopyImageToImageInfo*               pCopyImageToImageInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL TransitionImageLayout(
    VkDevice                                    device,
    uint32_t                                    transitionCount,
    const VkHostImageLayoutTransitionInfo*      pTransitions)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR void VKAPI_CALL DestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported)
{
    // Currently say that all surface/queue combos are supported
    *pSupported = VK_TRUE;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes)
{
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
}


static VKAPI_ATTR VkResult VKAPI_CALL CreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain)
{
    unique_lock_t lock(global_lock);
    *pSwapchain = (VkSwapchainKHR)global_unique_handle++;
    for(uint32_t i = 0; i < icd_swapchain_image_count; ++i){
        swapchain_image_map[*pSwapchain][i] = (VkImage)global_unique_handle++;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator)
{
    unique_lock_t lock(global_lock);
    swapchain_image_map.clear();
}

static VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex)
{
    *pImageIndex = 0;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL QueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex)
{
    *pImageIndex = 0;
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties)
{
    if (!pProperties) {
        *pPropertyCount = 1;
    } else {
        unique_lock_t lock(global_lock);
        pProperties[0].display = (VkDisplayKHR)global_unique_handle++;
        display_map[physicalDevice].insert(pProperties[0].display);
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode)
{
    unique_lock_t lock(global_lock);
    *pMode = (VkDisplayModeKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL CreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains)
{
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < swapchainCount; ++i) {
        pSwapchains[i] = (VkSwapchainKHR)global_unique_handle++;
    }
    return VK_SUCCESS;
}

#ifdef VK_USE_PLATFORM_XLIB_KHR

static VKAPI_ATTR VkResult VKAPI_CALL CreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_XLIB_KHR */

#ifdef VK_USE_PLATFORM_XCB_KHR

static VKAPI_ATTR VkResult VKAPI_CALL CreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_XCB_KHR */

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

static VKAPI_ATTR VkResult VKAPI_CALL CreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_WAYLAND_KHR */

#ifdef VK_USE_PLATFORM_ANDROID_KHR

static VKAPI_ATTR VkResult VKAPI_CALL CreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_ANDROID_KHR */

#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL CreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */



static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkVideoProfileInfoKHR*                pVideoProfile,
    VkVideoCapabilitiesKHR*                     pCapabilities)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoFormatInfoKHR*   pVideoFormatInfo,
    uint32_t*                                   pVideoFormatPropertyCount,
    VkVideoFormatPropertiesKHR*                 pVideoFormatProperties)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateVideoSessionKHR(
    VkDevice                                    device,
    const VkVideoSessionCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionKHR*                          pVideoSession)
{
    unique_lock_t lock(global_lock);
    *pVideoSession = (VkVideoSessionKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyVideoSessionKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL GetVideoSessionMemoryRequirementsKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t*                                   pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR*        pMemoryRequirements)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL BindVideoSessionMemoryKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t                                    bindSessionMemoryInfoCount,
    const VkBindVideoSessionMemoryInfoKHR*      pBindSessionMemoryInfos)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionParametersKHR*                pVideoSessionParameters)
{
    unique_lock_t lock(global_lock);
    *pVideoSessionParameters = (VkVideoSessionParametersKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL UpdateVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL CmdBeginVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoBeginCodingInfoKHR*            pBeginInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEndCodingInfoKHR*              pEndCodingInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdControlVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoCodingControlInfoKHR*          pCodingControlInfo)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL CmdDecodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoDecodeInfoKHR*                 pDecodeInfo)
{
//Not a CREATE or DESTROY function
}





static VKAPI_ATTR void VKAPI_CALL CmdBeginRenderingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndRenderingKHR(
    VkCommandBuffer                             commandBuffer)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties)
{
    GetPhysicalDeviceFormatProperties(physicalDevice, format, &pFormatProperties->formatProperties);
    VkFormatProperties3KHR *props_3 = lvl_find_mod_in_chain<VkFormatProperties3KHR>(pFormatProperties->pNext);
    if (props_3) {
        props_3->linearTilingFeatures = pFormatProperties->formatProperties.linearTilingFeatures;
        props_3->optimalTilingFeatures = pFormatProperties->formatProperties.optimalTilingFeatures;
        props_3->bufferFeatures = pFormatProperties->formatProperties.bufferFeatures;
        props_3->optimalTilingFeatures |= VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT_EXT;
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties)
{
    auto *external_image_prop = lvl_find_mod_in_chain<VkExternalImageFormatProperties>(pImageFormatProperties->pNext);
    auto *external_image_format = lvl_find_in_chain<VkPhysicalDeviceExternalImageFormatInfo>(pImageFormatInfo->pNext);
    if (external_image_prop && external_image_format) {
        external_image_prop->externalMemoryProperties.externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
        external_image_prop->externalMemoryProperties.compatibleHandleTypes = external_image_format->handleType;
    }

    GetPhysicalDeviceImageFormatProperties(physicalDevice, pImageFormatInfo->format, pImageFormatInfo->type, pImageFormatInfo->tiling, pImageFormatInfo->usage, pImageFormatInfo->flags, &pImageFormatProperties->imageFormatProperties);
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties)
{
    GetPhysicalDeviceMemoryProperties(physicalDevice, &pMemoryProperties->memoryProperties);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties)
{
    if (pPropertyCount && pProperties) {
        GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, pFormatInfo->format, pFormatInfo->type, pFormatInfo->samples, pFormatInfo->usage, pFormatInfo->tiling, pPropertyCount, &pProperties->properties);
    } else {
        GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, pFormatInfo->format, pFormatInfo->type, pFormatInfo->samples, pFormatInfo->usage, pFormatInfo->tiling, pPropertyCount, nullptr);
    }
}


static VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR void VKAPI_CALL TrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties)
{
    if (!pPhysicalDeviceGroupProperties) {
        *pPhysicalDeviceGroupCount = 1;
    } else {
        // arbitrary
        pPhysicalDeviceGroupProperties->physicalDeviceCount = 1;
        pPhysicalDeviceGroupProperties->physicalDevices[0] = physical_device_map.at(instance)[0];
        pPhysicalDeviceGroupProperties->subsetAllocation = VK_FALSE;
    }
    return VK_SUCCESS;
}


static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties)
{
    GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties)
{
    pMemoryWin32HandleProperties->memoryTypeBits = 0xFFFF;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */


static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd)
{
    *pFd = 1;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif /* VK_USE_PLATFORM_WIN32_KHR */


static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties)
{
    GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */


static VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData)
{
//Not a CREATE or DESTROY function
}





static VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate)
{
    unique_lock_t lock(global_lock);
    *pDescriptorUpdateTemplate = (VkDescriptorUpdateTemplate)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass)
{
    unique_lock_t lock(global_lock);
    *pRenderPass = (VkRenderPass)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties)
{
    GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL ImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle)
{
    *pHandle = (HANDLE)0x12345678;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */


static VKAPI_ATTR VkResult VKAPI_CALL ImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd)
{
    *pFd = 0x42;
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions)
{
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
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses)
{
    if (pNumPasses) {
        // arbitrary
        *pNumPasses = 1;
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL AcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL ReleaseProfilingLockKHR(
    VkDevice                                    device)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats)
{
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
}



static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}





static VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    GetImageMemoryRequirements(device, pInfo->image, &pMemoryRequirements->memoryRequirements);
}

static VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    GetBufferMemoryRequirements(device, pInfo->buffer, &pMemoryRequirements->memoryRequirements);
}

static VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements)
{
    if (pSparseMemoryRequirementCount && pSparseMemoryRequirements) {
        GetImageSparseMemoryRequirements(device, pInfo->image, pSparseMemoryRequirementCount, &pSparseMemoryRequirements->memoryRequirements);
    } else {
        GetImageSparseMemoryRequirements(device, pInfo->image, pSparseMemoryRequirementCount, nullptr);
    }
}



static VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion)
{
    unique_lock_t lock(global_lock);
    *pYcbcrConversion = (VkSamplerYcbcrConversion)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}


static VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif /* VK_ENABLE_BETA_EXTENSIONS */


static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport)
{
    GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}


static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}












static VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL WaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL SignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}




static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR*     pFragmentShadingRates)
{
    if (!pFragmentShadingRates) {
        *pFragmentShadingRateCount = 1;
    } else {
        // arbitrary
        pFragmentShadingRates->sampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT;
        pFragmentShadingRates->fragmentSize = {8, 8};
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdSetFragmentShadingRateKHR(
    VkCommandBuffer                             commandBuffer,
    const VkExtent2D*                           pFragmentSize,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2])
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL CmdSetRenderingAttachmentLocationsKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingAttachmentLocationInfo*    pLocationInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInputAttachmentIndexInfo*  pInputAttachmentIndexInfo)
{
//Not a CREATE or DESTROY function
}






static VKAPI_ATTR VkResult VKAPI_CALL WaitForPresentKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    presentId,
    uint64_t                                    timeout)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}



static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo)
{
    return GetBufferDeviceAddress(device, pInfo);
}

static VKAPI_ATTR uint64_t VKAPI_CALL GetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR uint64_t VKAPI_CALL GetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL CreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation)
{
    unique_lock_t lock(global_lock);
    *pDeferredOperation = (VkDeferredOperationKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR uint32_t VKAPI_CALL GetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL DeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL MapMemory2KHR(
    VkDevice                                    device,
    const VkMemoryMapInfo*                      pMemoryMapInfo,
    void**                                      ppData)
{
    return MapMemory(device, pMemoryMapInfo->memory, pMemoryMapInfo->offset, pMemoryMapInfo->size, pMemoryMapInfo->flags, ppData);
}

static VKAPI_ATTR VkResult VKAPI_CALL UnmapMemory2KHR(
    VkDevice                                    device,
    const VkMemoryUnmapInfo*                    pMemoryUnmapInfo)
{
    UnmapMemory(device, pMemoryUnmapInfo->memory);
    return VK_SUCCESS;
}






static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR*     pQualityLevelProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetEncodedVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo,
    size_t*                                     pDataSize,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdEncodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEncodeInfoKHR*                 pEncodeInfo)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL CmdSetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdResetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdWaitEvents2KHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp2KHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit2KHR(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}






static VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBlitImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdResolveImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR void VKAPI_CALL CmdTraceRaysIndirect2KHR(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectDeviceAddress)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR void VKAPI_CALL GetDeviceBufferMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    GetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
    GetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements)
{
//Not a CREATE or DESTROY function
}




static VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkIndexType                                 indexType)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetRenderingAreaGranularityKHR(
    VkDevice                                    device,
    const VkRenderingAreaInfo*                  pRenderingAreaInfo,
    VkExtent2D*                                 pGranularity)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageSubresourceLayoutKHR(
    VkDevice                                    device,
    const VkDeviceImageSubresourceInfo*         pInfo,
    VkSubresourceLayout2*                       pLayout)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout2KHR(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2*                  pSubresource,
    VkSubresourceLayout2*                       pLayout)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineBinariesKHR(
    VkDevice                                    device,
    const VkPipelineBinaryCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineBinaryHandlesInfoKHR*             pBinaries)
{
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < pBinaries->pipelineBinaryCount; ++i) {
        pBinaries->pPipelineBinaries[i] = (VkPipelineBinaryKHR)global_unique_handle++;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyPipelineBinaryKHR(
    VkDevice                                    device,
    VkPipelineBinaryKHR                         pipelineBinary,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineKeyKHR(
    VkDevice                                    device,
    const VkPipelineCreateInfoKHR*              pPipelineCreateInfo,
    VkPipelineBinaryKeyKHR*                     pPipelineKey)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineBinaryDataKHR(
    VkDevice                                    device,
    const VkPipelineBinaryDataInfoKHR*          pInfo,
    VkPipelineBinaryKeyKHR*                     pPipelineBinaryKey,
    size_t*                                     pPipelineBinaryDataSize,
    void*                                       pPipelineBinaryData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL ReleaseCapturedPipelineDataKHR(
    VkDevice                                    device,
    const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
    const VkAllocationCallbacks*                pAllocator)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesKHR*           pProperties)
{
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
}










static VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainKHR*                            pTimeDomains)
{
    if (!pTimeDomains) {
        *pTimeDomainCount = 1;
    } else {
        // arbitrary
        *pTimeDomains = VK_TIME_DOMAIN_DEVICE_KHR;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetCalibratedTimestampsKHR(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoKHR*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}



static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBindDescriptorSetsInfo*             pBindDescriptorSetsInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPushConstants2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushConstantsInfo*                  pPushConstantsInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSet2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetInfo*              pPushDescriptorSetInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetWithTemplateInfo*  pPushDescriptorSetWithTemplateInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer                             commandBuffer,
    const VkSetDescriptorBufferOffsetsInfoEXT*  pSetDescriptorBufferOffsetsInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer                             commandBuffer,
    const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo)
{
//Not a CREATE or DESTROY function
}








static VKAPI_ATTR VkResult VKAPI_CALL CreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback)
{
    unique_lock_t lock(global_lock);
    *pCallback = (VkDebugReportCallbackEXT)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL DebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage)
{
//Not a CREATE or DESTROY function
}








static VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo)
{
//Not a CREATE or DESTROY function
}




static VKAPI_ATTR void VKAPI_CALL CmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL CreateCuModuleNVX(
    VkDevice                                    device,
    const VkCuModuleCreateInfoNVX*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuModuleNVX*                              pModule)
{
    unique_lock_t lock(global_lock);
    *pModule = (VkCuModuleNVX)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateCuFunctionNVX(
    VkDevice                                    device,
    const VkCuFunctionCreateInfoNVX*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuFunctionNVX*                            pFunction)
{
    unique_lock_t lock(global_lock);
    *pFunction = (VkCuFunctionNVX)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyCuModuleNVX(
    VkDevice                                    device,
    VkCuModuleNVX                               module,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL DestroyCuFunctionNVX(
    VkDevice                                    device,
    VkCuFunctionNVX                             function,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL CmdCuLaunchKernelNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCuLaunchInfoNVX*                    pLaunchInfo)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR uint32_t VKAPI_CALL GetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR uint64_t VKAPI_CALL GetImageViewHandle64NVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}






static VKAPI_ATTR VkResult VKAPI_CALL GetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


#ifdef VK_USE_PLATFORM_GGP

static VKAPI_ATTR VkResult VKAPI_CALL CreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_GGP */




static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif /* VK_USE_PLATFORM_WIN32_KHR */


#ifdef VK_USE_PLATFORM_VI_NN

static VKAPI_ATTR VkResult VKAPI_CALL CreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_VI_NN */







static VKAPI_ATTR void VKAPI_CALL CmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL ReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

static VKAPI_ATTR VkResult VKAPI_CALL AcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_XLIB_XRANDR_EXT */


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL DisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL RegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL RegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence)
{
    unique_lock_t lock(global_lock);
    *pFence = (VkFence)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL GetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}







static VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    discardRectangleEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkDiscardRectangleModeEXT                   discardRectangleMode)
{
//Not a CREATE or DESTROY function
}





static VKAPI_ATTR void VKAPI_CALL SetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata)
{
//Not a CREATE or DESTROY function
}


#ifdef VK_USE_PLATFORM_IOS_MVK

static VKAPI_ATTR VkResult VKAPI_CALL CreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_IOS_MVK */

#ifdef VK_USE_PLATFORM_MACOS_MVK

static VKAPI_ATTR VkResult VKAPI_CALL CreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_MACOS_MVK */




static VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL QueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger)
{
    unique_lock_t lock(global_lock);
    *pMessenger = (VkDebugUtilsMessengerEXT)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL SubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
{
//Not a CREATE or DESTROY function
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR

static VKAPI_ATTR VkResult VKAPI_CALL GetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties)
{
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
}

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_ANDROID_KHR */



#ifdef VK_ENABLE_BETA_EXTENSIONS

static VKAPI_ATTR VkResult VKAPI_CALL CreateExecutionGraphPipelinesAMDX(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines)
{
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        pPipelines[i] = (VkPipeline)global_unique_handle++;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetExecutionGraphPipelineScratchSizeAMDX(
    VkDevice                                    device,
    VkPipeline                                  executionGraph,
    VkExecutionGraphPipelineScratchSizeAMDX*    pSizeInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetExecutionGraphPipelineNodeIndexAMDX(
    VkDevice                                    device,
    VkPipeline                                  executionGraph,
    const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
    uint32_t*                                   pNodeIndex)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdInitializeGraphScratchMemoryAMDX(
    VkCommandBuffer                             commandBuffer,
    VkPipeline                                  executionGraph,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDispatchGraphAMDX(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize,
    const VkDispatchGraphCountInfoAMDX*         pCountInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDispatchGraphIndirectAMDX(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize,
    const VkDispatchGraphCountInfoAMDX*         pCountInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDispatchGraphIndirectCountAMDX(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize,
    VkDeviceAddress                             countInfo)
{
//Not a CREATE or DESTROY function
}
#endif /* VK_ENABLE_BETA_EXTENSIONS */






static VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties)
{
    if (pMultisampleProperties) {
        // arbitrary
        pMultisampleProperties->maxSampleLocationGridSize = {32, 32};
    }
}








static VKAPI_ATTR VkResult VKAPI_CALL GetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL CreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache)
{
    unique_lock_t lock(global_lock);
    *pValidationCache = (VkValidationCacheEXT)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL MergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}




static VKAPI_ATTR void VKAPI_CALL CmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure)
{
    unique_lock_t lock(global_lock);
    *pAccelerationStructure = (VkAccelerationStructureNV)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements)
{
    // arbitrary
    pMemoryRequirements->memoryRequirements.size = 4096;
    pMemoryRequirements->memoryRequirements.alignment = 1;
    pMemoryRequirements->memoryRequirements.memoryTypeBits = 0xFFFF;
}

static VKAPI_ATTR VkResult VKAPI_CALL BindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkCopyAccelerationStructureModeKHR          mode)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdTraceRaysNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    raygenShaderBindingTableBuffer,
    VkDeviceSize                                raygenShaderBindingOffset,
    VkBuffer                                    missShaderBindingTableBuffer,
    VkDeviceSize                                missShaderBindingOffset,
    VkDeviceSize                                missShaderBindingStride,
    VkBuffer                                    hitShaderBindingTableBuffer,
    VkDeviceSize                                hitShaderBindingOffset,
    VkDeviceSize                                hitShaderBindingStride,
    VkBuffer                                    callableShaderBindingTableBuffer,
    VkDeviceSize                                callableShaderBindingOffset,
    VkDeviceSize                                callableShaderBindingStride,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines)
{
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        pPipelines[i] = (VkPipeline)global_unique_handle++;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureNV*            pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL CompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}






static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties)
{
    pMemoryHostPointerProperties->memoryTypeBits = 1 << 5; // DEVICE_LOCAL only type
    return VK_SUCCESS;
}


static VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarker2AMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainKHR*                            pTimeDomains)
{
    if (!pTimeDomains) {
        *pTimeDomainCount = 1;
    } else {
        // arbitrary
        *pTimeDomains = VK_TIME_DOMAIN_DEVICE_EXT;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoKHR*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}




#ifdef VK_USE_PLATFORM_GGP
#endif /* VK_USE_PLATFORM_GGP */





static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}




static VKAPI_ATTR void VKAPI_CALL CmdSetExclusiveScissorEnableNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkBool32*                             pExclusiveScissorEnables)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL CmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointData2NV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointData2NV*                        pCheckpointData)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR VkResult VKAPI_CALL InitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL UninitializePerformanceApiINTEL(
    VkDevice                                    device)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL AcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL ReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL QueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}



static VKAPI_ATTR void VKAPI_CALL SetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable)
{
//Not a CREATE or DESTROY function
}

#ifdef VK_USE_PLATFORM_FUCHSIA

static VKAPI_ATTR VkResult VKAPI_CALL CreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_FUCHSIA */

#ifdef VK_USE_PLATFORM_METAL_EXT

static VKAPI_ATTR VkResult VKAPI_CALL CreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_METAL_EXT */













static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo)
{
    return GetBufferDeviceAddress(device, pInfo);
}


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolProperties*             pToolProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}




static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}




#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL AcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL ReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */


static VKAPI_ATTR VkResult VKAPI_CALL CreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}


static VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR void VKAPI_CALL ResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR void VKAPI_CALL CmdSetCullModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetFrontFaceEXT(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveTopologyEXT(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetScissorWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers2EXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthCompareOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBoundsTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToImageEXT(
    VkDevice                                    device,
    const VkCopyMemoryToImageInfo*              pCopyMemoryToImageInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyImageToMemoryEXT(
    VkDevice                                    device,
    const VkCopyImageToMemoryInfo*              pCopyImageToMemoryInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyImageToImageEXT(
    VkDevice                                    device,
    const VkCopyImageToImageInfo*               pCopyImageToImageInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL TransitionImageLayoutEXT(
    VkDevice                                    device,
    uint32_t                                    transitionCount,
    const VkHostImageLayoutTransitionInfo*      pTransitions)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout2EXT(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2*                  pSubresource,
    VkSubresourceLayout2*                       pLayout)
{
//Not a CREATE or DESTROY function
}





static VKAPI_ATTR VkResult VKAPI_CALL ReleaseSwapchainImagesEXT(
    VkDevice                                    device,
    const VkReleaseSwapchainImagesInfoEXT*      pReleaseInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}



static VKAPI_ATTR void VKAPI_CALL GetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout)
{
    unique_lock_t lock(global_lock);
    *pIndirectCommandsLayout = (VkIndirectCommandsLayoutNV)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}





static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBias2EXT(
    VkCommandBuffer                             commandBuffer,
    const VkDepthBiasInfoEXT*                   pDepthBiasInfo)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR VkResult VKAPI_CALL AcquireDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    VkDisplayKHR                                display)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    uint32_t                                    connectorId,
    VkDisplayKHR*                               display)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}






static VKAPI_ATTR VkResult VKAPI_CALL CreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot)
{
    unique_lock_t lock(global_lock);
    *pPrivateDataSlot = (VkPrivateDataSlot)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL SetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData)
{
//Not a CREATE or DESTROY function
}





static VKAPI_ATTR VkResult VKAPI_CALL CreateCudaModuleNV(
    VkDevice                                    device,
    const VkCudaModuleCreateInfoNV*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCudaModuleNV*                             pModule)
{
    unique_lock_t lock(global_lock);
    *pModule = (VkCudaModuleNV)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetCudaModuleCacheNV(
    VkDevice                                    device,
    VkCudaModuleNV                              module,
    size_t*                                     pCacheSize,
    void*                                       pCacheData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateCudaFunctionNV(
    VkDevice                                    device,
    const VkCudaFunctionCreateInfoNV*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCudaFunctionNV*                           pFunction)
{
    unique_lock_t lock(global_lock);
    *pFunction = (VkCudaFunctionNV)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyCudaModuleNV(
    VkDevice                                    device,
    VkCudaModuleNV                              module,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL DestroyCudaFunctionNV(
    VkDevice                                    device,
    VkCudaFunctionNV                            function,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL CmdCudaLaunchKernelNV(
    VkCommandBuffer                             commandBuffer,
    const VkCudaLaunchInfoNV*                   pLaunchInfo)
{
//Not a CREATE or DESTROY function
}


#ifdef VK_USE_PLATFORM_METAL_EXT

static VKAPI_ATTR void VKAPI_CALL ExportMetalObjectsEXT(
    VkDevice                                    device,
    VkExportMetalObjectsInfoEXT*                pMetalObjectsInfo)
{
//Not a CREATE or DESTROY function
}
#endif /* VK_USE_PLATFORM_METAL_EXT */


static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSizeEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    VkDeviceSize*                               pLayoutSizeInBytes)
{
    // Need to give something non-zero
    *pLayoutSizeInBytes = 4;
}

static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutBindingOffsetEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    uint32_t                                    binding,
    VkDeviceSize*                               pOffset)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetDescriptorEXT(
    VkDevice                                    device,
    const VkDescriptorGetInfoEXT*               pDescriptorInfo,
    size_t                                      dataSize,
    void*                                       pDescriptor)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    bufferCount,
    const VkDescriptorBufferBindingInfoEXT*     pBindingInfos)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDescriptorBufferOffsetsEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    setCount,
    const uint32_t*                             pBufferIndices,
    const VkDeviceSize*                         pOffsets)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBufferEmbeddedSamplersEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL GetBufferOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetImageOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageCaptureDescriptorDataInfoEXT*  pInfo,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetImageViewOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetSamplerOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}




static VKAPI_ATTR void VKAPI_CALL CmdSetFragmentShadingRateEnumNV(
    VkCommandBuffer                             commandBuffer,
    VkFragmentShadingRateNV                     shadingRate,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2])
{
//Not a CREATE or DESTROY function
}










static VKAPI_ATTR VkResult VKAPI_CALL GetDeviceFaultInfoEXT(
    VkDevice                                    device,
    VkDeviceFaultCountsEXT*                     pFaultCounts,
    VkDeviceFaultInfoEXT*                       pFaultInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}



#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL AcquireWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    deviceRelativeId,
    VkDisplayKHR*                               pDisplay)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */

#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

static VKAPI_ATTR VkResult VKAPI_CALL CreateDirectFBSurfaceEXT(
    VkInstance                                  instance,
    const VkDirectFBSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceDirectFBPresentationSupportEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    IDirectFB*                                  dfb)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_DIRECTFB_EXT */



static VKAPI_ATTR void VKAPI_CALL CmdSetVertexInputEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT*  pVertexBindingDescriptions,
    uint32_t                                    vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions)
{
//Not a CREATE or DESTROY function
}






#ifdef VK_USE_PLATFORM_FUCHSIA

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkMemoryGetZirconHandleInfoFUCHSIA*   pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    zx_handle_t                                 zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA*      pMemoryZirconHandleProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_FUCHSIA */

#ifdef VK_USE_PLATFORM_FUCHSIA

static VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_FUCHSIA */

#ifdef VK_USE_PLATFORM_FUCHSIA

static VKAPI_ATTR VkResult VKAPI_CALL CreateBufferCollectionFUCHSIA(
    VkDevice                                    device,
    const VkBufferCollectionCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferCollectionFUCHSIA*                  pCollection)
{
    unique_lock_t lock(global_lock);
    *pCollection = (VkBufferCollectionFUCHSIA)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL SetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkImageConstraintsInfoFUCHSIA*        pImageConstraintsInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL SetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkBufferConstraintsInfoFUCHSIA*       pBufferConstraintsInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyBufferCollectionFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL GetBufferCollectionPropertiesFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    VkBufferCollectionPropertiesFUCHSIA*        pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_FUCHSIA */


static VKAPI_ATTR VkResult VKAPI_CALL GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(
    VkDevice                                    device,
    VkRenderPass                                renderpass,
    VkExtent2D*                                 pMaxWorkgroupSize)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdSubpassShadingHUAWEI(
    VkCommandBuffer                             commandBuffer)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL CmdBindInvocationMaskHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryRemoteAddressNV(
    VkDevice                                    device,
    const VkMemoryGetRemoteAddressInfoNV*       pMemoryGetRemoteAddressInfo,
    VkRemoteAddressNV*                          pAddress)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


static VKAPI_ATTR VkResult VKAPI_CALL GetPipelinePropertiesEXT(
    VkDevice                                    device,
    const VkPipelineInfoEXT*                    pPipelineInfo,
    VkBaseOutStructure*                         pPipelineProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}




static VKAPI_ATTR void VKAPI_CALL CmdSetPatchControlPointsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    patchControlPoints)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizerDiscardEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBiasEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkLogicOp                                   logicOp)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveRestartEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable)
{
//Not a CREATE or DESTROY function
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX

static VKAPI_ATTR VkResult VKAPI_CALL CreateScreenSurfaceQNX(
    VkInstance                                  instance,
    const VkScreenSurfaceCreateInfoQNX*         pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    unique_lock_t lock(global_lock);
    *pSurface = (VkSurfaceKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceScreenPresentationSupportQNX(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct _screen_window*                      window)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_SCREEN_QNX */


static VKAPI_ATTR void                                    VKAPI_CALL CmdSetColorWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorWriteEnables)
{
//Not a CREATE or DESTROY function
}





static VKAPI_ATTR void VKAPI_CALL CmdDrawMultiEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawInfoEXT*                   pVertexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMultiIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawIndexedInfoEXT*            pIndexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride,
    const int32_t*                              pVertexOffset)
{
//Not a CREATE or DESTROY function
}




static VKAPI_ATTR VkResult VKAPI_CALL CreateMicromapEXT(
    VkDevice                                    device,
    const VkMicromapCreateInfoEXT*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkMicromapEXT*                              pMicromap)
{
    unique_lock_t lock(global_lock);
    *pMicromap = (VkMicromapEXT)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyMicromapEXT(
    VkDevice                                    device,
    VkMicromapEXT                               micromap,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL CmdBuildMicromapsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL BuildMicromapsEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapInfoEXT*                pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyMicromapToMemoryEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL WriteMicromapsPropertiesEXT(
    VkDevice                                    device,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapInfoEXT*                pInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyMicromapToMemoryEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdWriteMicromapsPropertiesEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceMicromapCompatibilityEXT(
    VkDevice                                    device,
    const VkMicromapVersionInfoEXT*             pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetMicromapBuildSizesEXT(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkMicromapBuildInfoEXT*               pBuildInfo,
    VkMicromapBuildSizesInfoEXT*                pSizeInfo)
{
//Not a CREATE or DESTROY function
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif /* VK_ENABLE_BETA_EXTENSIONS */



static VKAPI_ATTR void VKAPI_CALL CmdDrawClusterHUAWEI(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawClusterIndirectHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR void VKAPI_CALL SetDeviceMemoryPriorityEXT(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    float                                       priority)
{
//Not a CREATE or DESTROY function
}





static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice                                    device,
    const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE*  pHostMapping)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetHostMappingVALVE(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    void**                                      ppData)
{
//Not a CREATE or DESTROY function
}






static VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToImageIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    const VkImageSubresourceLayers*             pImageSubresources)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL CmdDecompressMemoryNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    decompressRegionCount,
    const VkDecompressMemoryRegionNV*           pDecompressMemoryRegions)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDecompressMemoryIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectCommandsAddress,
    VkDeviceAddress                             indirectCommandsCountAddress,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL GetPipelineIndirectMemoryRequirementsNV(
    VkDevice                                    device,
    const VkComputePipelineCreateInfo*          pCreateInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdUpdatePipelineIndirectBufferNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetPipelineIndirectDeviceAddressNV(
    VkDevice                                    device,
    const VkPipelineIndirectDeviceAddressInfoNV* pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}









static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClampEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClampEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetPolygonModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkPolygonMode                               polygonMode)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizationSamplesEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       rasterizationSamples)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetSampleMaskEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       samples,
    const VkSampleMask*                         pSampleMask)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetAlphaToCoverageEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToCoverageEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetAlphaToOneEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToOneEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    logicOpEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorBlendEnables)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendEquationEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendEquationEXT*              pColorBlendEquations)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetColorWriteMaskEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorComponentFlags*                pColorWriteMasks)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetTessellationDomainOriginEXT(
    VkCommandBuffer                             commandBuffer,
    VkTessellationDomainOrigin                  domainOrigin)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizationStreamEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    rasterizationStream)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkConservativeRasterizationModeEXT          conservativeRasterizationMode)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetExtraPrimitiveOverestimationSizeEXT(
    VkCommandBuffer                             commandBuffer,
    float                                       extraPrimitiveOverestimationSize)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClipEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClipEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    sampleLocationsEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendAdvancedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendAdvancedEXT*              pColorBlendAdvanced)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetProvokingVertexModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkProvokingVertexModeEXT                    provokingVertexMode)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetLineRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkLineRasterizationModeEXT                  lineRasterizationMode)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stippledLineEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClipNegativeOneToOneEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    negativeOneToOne)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    viewportWScalingEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportSwizzleNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportSwizzleNV*                  pViewportSwizzles)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageToColorEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageToColorEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageToColorLocationNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageToColorLocation)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageModulationModeNV                  coverageModulationMode)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationTableEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageModulationTableEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationTableNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageModulationTableCount,
    const float*                                pCoverageModulationTable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetShadingRateImageEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    shadingRateImageEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRepresentativeFragmentTestEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    representativeFragmentTestEnable)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageReductionModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageReductionModeNV                   coverageReductionMode)
{
//Not a CREATE or DESTROY function
}




static VKAPI_ATTR void VKAPI_CALL GetShaderModuleIdentifierEXT(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    VkShaderModuleIdentifierEXT*                pIdentifier)
{
    if (pIdentifier) {
        // arbitrary
        pIdentifier->identifierSize = 1;
        pIdentifier->identifier[0] = 0x01;
    }
}

static VKAPI_ATTR void VKAPI_CALL GetShaderModuleCreateInfoIdentifierEXT(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    VkShaderModuleIdentifierEXT*                pIdentifier)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice                            physicalDevice,
    const VkOpticalFlowImageFormatInfoNV*       pOpticalFlowImageFormatInfo,
    uint32_t*                                   pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV*       pImageFormatProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateOpticalFlowSessionNV(
    VkDevice                                    device,
    const VkOpticalFlowSessionCreateInfoNV*     pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkOpticalFlowSessionNV*                     pSession)
{
    unique_lock_t lock(global_lock);
    *pSession = (VkOpticalFlowSessionNV)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyOpticalFlowSessionNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL BindOpticalFlowSessionImageNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    VkOpticalFlowSessionBindingPointNV          bindingPoint,
    VkImageView                                 view,
    VkImageLayout                               layout)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdOpticalFlowExecuteNV(
    VkCommandBuffer                             commandBuffer,
    VkOpticalFlowSessionNV                      session,
    const VkOpticalFlowExecuteInfoNV*           pExecuteInfo)
{
//Not a CREATE or DESTROY function
}



#ifdef VK_USE_PLATFORM_ANDROID_KHR
#endif /* VK_USE_PLATFORM_ANDROID_KHR */


static VKAPI_ATTR void VKAPI_CALL AntiLagUpdateAMD(
    VkDevice                                    device,
    const VkAntiLagDataAMD*                     pData)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL CreateShadersEXT(
    VkDevice                                    device,
    uint32_t                                    createInfoCount,
    const VkShaderCreateInfoEXT*                pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderEXT*                                pShaders)
{
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        pShaders[i] = (VkShaderEXT)global_unique_handle++;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyShaderEXT(
    VkDevice                                    device,
    VkShaderEXT                                 shader,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL GetShaderBinaryDataEXT(
    VkDevice                                    device,
    VkShaderEXT                                 shader,
    size_t*                                     pDataSize,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdBindShadersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    stageCount,
    const VkShaderStageFlagBits*                pStages,
    const VkShaderEXT*                          pShaders)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClampRangeEXT(
    VkCommandBuffer                             commandBuffer,
    VkDepthClampModeEXT                         depthClampMode,
    const VkDepthClampRangeEXT*                 pDepthClampRange)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR VkResult VKAPI_CALL GetFramebufferTilePropertiesQCOM(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    uint32_t*                                   pPropertiesCount,
    VkTilePropertiesQCOM*                       pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetDynamicRenderingTilePropertiesQCOM(
    VkDevice                                    device,
    const VkRenderingInfo*                      pRenderingInfo,
    VkTilePropertiesQCOM*                       pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}





static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeVectorPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeVectorPropertiesNV*            pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL ConvertCooperativeVectorMatrixNV(
    VkDevice                                    device,
    const VkConvertCooperativeVectorMatrixInfoNV* pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdConvertCooperativeVectorMatrixNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkConvertCooperativeVectorMatrixInfoNV* pInfos)
{
//Not a CREATE or DESTROY function
}









static VKAPI_ATTR VkResult VKAPI_CALL SetLatencySleepModeNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkLatencySleepModeInfoNV*             pSleepModeInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL LatencySleepNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkLatencySleepInfoNV*                 pSleepInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL SetLatencyMarkerNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkSetLatencyMarkerInfoNV*             pLatencyMarkerInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetLatencyTimingsNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkGetLatencyMarkerInfoNV*                   pLatencyMarkerInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL QueueNotifyOutOfBandNV(
    VkQueue                                     queue,
    const VkOutOfBandQueueTypeInfoNV*           pQueueTypeInfo)
{
//Not a CREATE or DESTROY function
}








static VKAPI_ATTR void VKAPI_CALL CmdSetAttachmentFeedbackLoopEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkImageAspectFlags                          aspectMask)
{
//Not a CREATE or DESTROY function
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX

static VKAPI_ATTR VkResult VKAPI_CALL GetScreenBufferPropertiesQNX(
    VkDevice                                    device,
    const struct _screen_buffer*                buffer,
    VkScreenBufferPropertiesQNX*                pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_SCREEN_QNX */










static VKAPI_ATTR void VKAPI_CALL GetClusterAccelerationStructureBuildSizesNV(
    VkDevice                                    device,
    const VkClusterAccelerationStructureInputInfoNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer                             commandBuffer,
    const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL GetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice                                    device,
    const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer                             commandBuffer,
    const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo)
{
//Not a CREATE or DESTROY function
}


static VKAPI_ATTR void VKAPI_CALL GetGeneratedCommandsMemoryRequirementsEXT(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdPreprocessGeneratedCommandsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoEXT*           pGeneratedCommandsInfo,
    VkCommandBuffer                             stateCommandBuffer)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdExecuteGeneratedCommandsEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoEXT*           pGeneratedCommandsInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectCommandsLayoutEXT(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutEXT*                pIndirectCommandsLayout)
{
    unique_lock_t lock(global_lock);
    *pIndirectCommandsLayout = (VkIndirectCommandsLayoutEXT)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyIndirectCommandsLayoutEXT(
    VkDevice                                    device,
    VkIndirectCommandsLayoutEXT                 indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectExecutionSetEXT(
    VkDevice                                    device,
    const VkIndirectExecutionSetCreateInfoEXT*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectExecutionSetEXT*                  pIndirectExecutionSet)
{
    unique_lock_t lock(global_lock);
    *pIndirectExecutionSet = (VkIndirectExecutionSetEXT)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyIndirectExecutionSetEXT(
    VkDevice                                    device,
    VkIndirectExecutionSetEXT                   indirectExecutionSet,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL UpdateIndirectExecutionSetPipelineEXT(
    VkDevice                                    device,
    VkIndirectExecutionSetEXT                   indirectExecutionSet,
    uint32_t                                    executionSetWriteCount,
    const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL UpdateIndirectExecutionSetShaderEXT(
    VkDevice                                    device,
    VkIndirectExecutionSetEXT                   indirectExecutionSet,
    uint32_t                                    executionSetWriteCount,
    const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites)
{
//Not a CREATE or DESTROY function
}





static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}


#ifdef VK_USE_PLATFORM_METAL_EXT

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryMetalHandleEXT(
    VkDevice                                    device,
    const VkMemoryGetMetalHandleInfoEXT*        pGetMetalHandleInfo,
    void**                                      pHandle)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryMetalHandlePropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHandle,
    VkMemoryMetalHandlePropertiesEXT*           pMemoryMetalHandleProperties)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_METAL_EXT */



static VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure)
{
    unique_lock_t lock(global_lock);
    *pAccelerationStructure = (VkAccelerationStructureKHR)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator)
{
//Destroy object
}

static VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructuresKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress*                      pIndirectDeviceAddresses,
    const uint32_t*                             pIndirectStrides,
    const uint32_t* const*                      ppMaxPrimitiveCounts)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL BuildAccelerationStructuresKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureInfoKHR*   pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL WriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo)
{
    // arbitrary - need to be aligned to 256 bytes
    return 0x262144;
}

static VKAPI_ATTR void VKAPI_CALL CmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureBuildSizesKHR(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t*                             pMaxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo)
{
    // arbitrary
    pSizeInfo->accelerationStructureSize = 4;
    pSizeInfo->updateScratchSize = 4;
    pSizeInfo->buildScratchSize = 4;
}


static VKAPI_ATTR void VKAPI_CALL CmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines)
{
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        pPipelines[i] = (VkPipeline)global_unique_handle++;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    VkDeviceAddress                             indirectDeviceAddress)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR VkDeviceSize VKAPI_CALL GetRayTracingShaderGroupStackSizeKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    group,
    VkShaderGroupShaderKHR                      groupShader)
{
//Not a CREATE or DESTROY function
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL CmdSetRayTracingPipelineStackSizeKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    pipelineStackSize)
{
//Not a CREATE or DESTROY function
}



static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
//Not a CREATE or DESTROY function
}

} // namespace vkmock

