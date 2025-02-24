/*
** Copyright (c) 2015-2018, 2023-2025 The Khronos Group Inc.
** Modifications Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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

#include <algorithm>
#include "test_icd.h"
#include "test_icd_helper.h"
#include <vulkan/utility/vk_format_utils.h>
#include <cstddef>
#include <vulkan/utility/vk_struct_helper.hpp>

namespace icd {

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char* funcName) {
    const auto& item = name_to_func_ptr_map.find(funcName);
    if (item != name_to_func_ptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }
    // we should intercept all functions, so if we get here just return null
    return nullptr;
}

#if defined(__GNUC__) && __GNUC__ >= 4
#define EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

void SetBoolArrayTrue(VkBool32* bool_array, uint32_t num_bools) {
    for (uint32_t i = 0; i < num_bools; ++i) {
        bool_array[i] = VK_TRUE;
    }
}

VkDeviceSize GetImageSizeFromCreateInfo(const VkImageCreateInfo* pCreateInfo) {
    VkDeviceSize size = pCreateInfo->extent.width;
    size *= pCreateInfo->extent.height;
    size *= pCreateInfo->extent.depth;
    size *= 32;  // A pixel size is 32 bytes, this accounts for the largest possible pixel size of any format
    size *= pCreateInfo->arrayLayers;
    size *= (pCreateInfo->mipLevels > 1 ? 2 : 1);
    size *= vkuFormatPlaneCount(pCreateInfo->format);
    return size;
}

// These are placeholders, should not be needed since we are using profiles over this
static VkPhysicalDeviceLimits SetLimits(VkPhysicalDeviceLimits* limits) {
    limits->maxImageDimension1D = 4096;
    limits->maxImageDimension2D = 4096;
    limits->maxImageDimension3D = 256;
    limits->maxImageDimensionCube = 4096;
    limits->maxImageArrayLayers = 256;
    limits->maxTexelBufferElements = 65536;
    limits->maxUniformBufferRange = 16384;
    limits->maxStorageBufferRange = 134217728;
    limits->maxPushConstantsSize = 128;
    limits->maxMemoryAllocationCount = 4096;
    limits->maxSamplerAllocationCount = 4000;
    limits->bufferImageGranularity = 1;
    limits->sparseAddressSpaceSize = 2147483648;
    limits->maxBoundDescriptorSets = 4;
    limits->maxPerStageDescriptorSamplers = 16;
    limits->maxPerStageDescriptorUniformBuffers = 12;
    limits->maxPerStageDescriptorStorageBuffers = 4;
    limits->maxPerStageDescriptorSampledImages = 16;
    limits->maxPerStageDescriptorStorageImages = 4;
    limits->maxPerStageDescriptorInputAttachments = 4;
    limits->maxPerStageResources = 128;
    limits->maxDescriptorSetSamplers = 96;
    limits->maxDescriptorSetUniformBuffers = 72;
    limits->maxDescriptorSetUniformBuffersDynamic = 8;
    limits->maxDescriptorSetStorageBuffers = 24;
    limits->maxDescriptorSetStorageBuffersDynamic = 4;
    limits->maxDescriptorSetSampledImages = 96;
    limits->maxDescriptorSetStorageImages = 24;
    limits->maxDescriptorSetInputAttachments = 4;
    limits->maxVertexInputAttributes = 16;
    limits->maxVertexInputBindings = 16;
    limits->maxVertexInputAttributeOffset = 2047;
    limits->maxVertexInputBindingStride = 2048;
    limits->maxVertexOutputComponents = 64;
    limits->maxTessellationGenerationLevel = 64;
    limits->maxTessellationPatchSize = 32;
    limits->maxTessellationControlPerVertexInputComponents = 64;
    limits->maxTessellationControlPerVertexOutputComponents = 64;
    limits->maxTessellationControlPerPatchOutputComponents = 120;
    limits->maxTessellationControlTotalOutputComponents = 2048;
    limits->maxTessellationEvaluationInputComponents = 64;
    limits->maxTessellationEvaluationOutputComponents = 64;
    limits->maxGeometryShaderInvocations = 32;
    limits->maxGeometryInputComponents = 64;
    limits->maxGeometryOutputComponents = 64;
    limits->maxGeometryOutputVertices = 256;
    limits->maxGeometryTotalOutputComponents = 1024;
    limits->maxFragmentInputComponents = 64;
    limits->maxFragmentOutputAttachments = 4;
    limits->maxFragmentDualSrcAttachments = 1;
    limits->maxFragmentCombinedOutputResources = 4;
    limits->maxComputeSharedMemorySize = 16384;
    limits->maxComputeWorkGroupCount[0] = 65535;
    limits->maxComputeWorkGroupCount[1] = 65535;
    limits->maxComputeWorkGroupCount[2] = 65535;
    limits->maxComputeWorkGroupInvocations = 128;
    limits->maxComputeWorkGroupSize[0] = 128;
    limits->maxComputeWorkGroupSize[1] = 128;
    limits->maxComputeWorkGroupSize[2] = 64;
    limits->subPixelPrecisionBits = 4;
    limits->subTexelPrecisionBits = 4;
    limits->mipmapPrecisionBits = 4;
    limits->maxDrawIndexedIndexValue = UINT32_MAX;
    limits->maxDrawIndirectCount = UINT16_MAX;
    limits->maxSamplerLodBias = 2.0f;
    limits->maxSamplerAnisotropy = 16;
    limits->maxViewports = 16;
    limits->maxViewportDimensions[0] = 4096;
    limits->maxViewportDimensions[1] = 4096;
    limits->viewportBoundsRange[0] = -8192;
    limits->viewportBoundsRange[1] = 8191;
    limits->viewportSubPixelBits = 0;
    limits->minMemoryMapAlignment = 64;
    limits->minTexelBufferOffsetAlignment = 16;
    limits->minUniformBufferOffsetAlignment = 16;
    limits->minStorageBufferOffsetAlignment = 16;
    limits->minTexelOffset = -8;
    limits->maxTexelOffset = 7;
    limits->minTexelGatherOffset = -8;
    limits->maxTexelGatherOffset = 7;
    limits->minInterpolationOffset = 0.0f;
    limits->maxInterpolationOffset = 0.5f;
    limits->subPixelInterpolationOffsetBits = 4;
    limits->maxFramebufferWidth = 4096;
    limits->maxFramebufferHeight = 4096;
    limits->maxFramebufferLayers = 256;
    limits->framebufferColorSampleCounts = 0x7F;
    limits->framebufferDepthSampleCounts = 0x7F;
    limits->framebufferStencilSampleCounts = 0x7F;
    limits->framebufferNoAttachmentsSampleCounts = 0x7F;
    limits->maxColorAttachments = 4;
    limits->sampledImageColorSampleCounts = 0x7F;
    limits->sampledImageIntegerSampleCounts = 0x7F;
    limits->sampledImageDepthSampleCounts = 0x7F;
    limits->sampledImageStencilSampleCounts = 0x7F;
    limits->storageImageSampleCounts = 0x7F;
    limits->maxSampleMaskWords = 1;
    limits->timestampComputeAndGraphics = VK_TRUE;
    limits->timestampPeriod = 1;
    limits->maxClipDistances = 8;
    limits->maxCullDistances = 8;
    limits->maxCombinedClipAndCullDistances = 8;
    limits->discreteQueuePriorities = 2;
    limits->pointSizeRange[0] = 1.0f;
    limits->pointSizeRange[1] = 64.0f;
    limits->lineWidthRange[0] = 1.0f;
    limits->lineWidthRange[1] = 8.0f;
    limits->pointSizeGranularity = 1.0f;
    limits->lineWidthGranularity = 1.0f;
    limits->strictLines = VK_TRUE;
    limits->standardSampleLocations = VK_TRUE;
    limits->optimalBufferCopyOffsetAlignment = 1;
    limits->optimalBufferCopyRowPitchAlignment = 1;
    limits->nonCoherentAtomSize = 256;

    return *limits;
}

}  // namespace icd

extern "C" {

EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
    if (!icd::negotiate_loader_icd_interface_called) {
        icd::loader_interface_version = 1;
    }
    return icd::GetInstanceProcAddr(instance, pName);
}

EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetPhysicalDeviceProcAddr(VkInstance instance, const char* pName) {
    return icd::GetPhysicalDeviceProcAddr(instance, pName);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
    icd::negotiate_loader_icd_interface_called = true;
    icd::loader_interface_version = *pSupportedVersion;
    if (*pSupportedVersion > icd::SUPPORTED_LOADER_ICD_INTERFACE_VERSION) {
        *pSupportedVersion = icd::SUPPORTED_LOADER_ICD_INTERFACE_VERSION;
    }
    return VK_SUCCESS;
}

EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                      const VkAllocationCallbacks* pAllocator) {
    icd::DestroySurfaceKHR(instance, surface, pAllocator);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                                           VkBool32* pSupported) {
    return icd::GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                                VkSurfaceKHR surface,
                                                                                VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    return icd::GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                           uint32_t* pSurfaceFormatCount,
                                                                           VkSurfaceFormatKHR* pSurfaceFormats) {
    return icd::GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                VkSurfaceKHR surface, uint32_t* pPresentModeCount,
                                                                                VkPresentModeKHR* pPresentModes) {
    return icd::GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                                                     const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                                     const VkAllocationCallbacks* pAllocator,
                                                                     VkSurfaceKHR* pSurface) {
    return icd::CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

#ifdef VK_USE_PLATFORM_XLIB_KHR

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return icd::CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_XLIB_KHR */

#ifdef VK_USE_PLATFORM_XCB_KHR

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return icd::CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_XCB_KHR */

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateWaylandSurfaceKHR(VkInstance instance,
                                                                const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return icd::CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_WAYLAND_KHR */

#ifdef VK_USE_PLATFORM_ANDROID_KHR

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateAndroidSurfaceKHR(VkInstance instance,
                                                                const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return icd::CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_ANDROID_KHR */

#ifdef VK_USE_PLATFORM_WIN32_KHR

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return icd::CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                             VkDeviceGroupPresentModeFlagsKHR* pModes) {
    return icd::GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                              uint32_t* pRectCount, VkRect2D* pRects) {
    return icd::GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
}

#ifdef VK_USE_PLATFORM_VI_NN

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return icd::CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_VI_NN */

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                                                 VkSurfaceKHR surface,
                                                                                 VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    return icd::GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
}

#ifdef VK_USE_PLATFORM_IOS_MVK

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return icd::CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_IOS_MVK */

#ifdef VK_USE_PLATFORM_MACOS_MVK

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return icd::CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_MACOS_MVK */

}  // end extern "C"

namespace icd {

static VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
    // TODO: If loader ver <=4 ICD must fail with VK_ERROR_INCOMPATIBLE_DRIVER for all vkCreateInstance calls with
    //  apiVersion set to > Vulkan 1.0 because the loader is still at interface version <= 4. Otherwise, the
    //  ICD should behave as normal.
    if (loader_interface_version <= 4) {
        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }
    *pInstance = (VkInstance)CreateDispObjHandle();
    for (auto& physical_device : physical_device_map[*pInstance]) physical_device = (VkPhysicalDevice)CreateDispObjHandle();
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    if (instance) {
        for (const auto physical_device : physical_device_map.at(instance)) {
            display_map.erase(physical_device);
            DestroyDispObjHandle((void*)physical_device);
        }
        physical_device_map.erase(instance);
        DestroyDispObjHandle((void*)instance);
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                               VkPhysicalDevice* pPhysicalDevices) {
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

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    uint32_t num_bools = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
    VkBool32* bool_array = &pFeatures->robustBufferAccess;
    SetBoolArrayTrue(bool_array, num_bools);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                    VkFormatProperties* pFormatProperties) {
    if (VK_FORMAT_UNDEFINED == format) {
        *pFormatProperties = {0x0, 0x0, 0x0};
    } else {
        // Default to a color format, skip DS bit
        *pFormatProperties = {0x00FFFDFF, 0x00FFFDFF, 0x00FFFDFF};
        switch (format) {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_S8_UINT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                // Don't set color bits for DS formats
                *pFormatProperties = {0x00FFFE7F, 0x00FFFE7F, 0x00FFFE7F};
                break;
            case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
            case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
            case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
            case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
            case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
                // Set decode/encode bits for these formats
                *pFormatProperties = {0x1EFFFDFF, 0x1EFFFDFF, 0x00FFFDFF};
                break;
            default:
                break;
        }
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                             VkImageType type, VkImageTiling tiling,
                                                                             VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                             VkImageFormatProperties* pImageFormatProperties) {
    // A hardcoded unsupported format
    if (format == VK_FORMAT_E5B9G9R9_UFLOAT_PACK32) {
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    // TODO: Just hard-coding some values for now
    // TODO: If tiling is linear, limit the mips, levels, & sample count
    if (VK_IMAGE_TILING_LINEAR == tiling) {
        *pImageFormatProperties = {{4096, 4096, 256}, 1, 1, VK_SAMPLE_COUNT_1_BIT, 4294967296};
    } else {
        // We hard-code support for all sample counts except 64 bits.
        *pImageFormatProperties = {{4096, 4096, 256}, 12, 256, 0x7F & ~VK_SAMPLE_COUNT_64_BIT, 4294967296};
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                              VkPhysicalDeviceProperties* pProperties) {
    pProperties->apiVersion = VK_HEADER_VERSION_COMPLETE;
    pProperties->driverVersion = 1;
    pProperties->vendorID = 0xba5eba11;
    pProperties->deviceID = 0xf005ba11;
    pProperties->deviceType = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
    // Keep to match with other MockICD
    strcpy(pProperties->deviceName, "Vulkan Mock Device");
    pProperties->pipelineCacheUUID[0] = 18;
    pProperties->limits = SetLimits(&pProperties->limits);
    pProperties->sparseProperties = {VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE};
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                         uint32_t* pQueueFamilyPropertyCount,
                                                                         VkQueueFamilyProperties* pQueueFamilyProperties) {
    if (pQueueFamilyProperties) {
        std::vector<VkQueueFamilyProperties2> props2(*pQueueFamilyPropertyCount,
                                                     {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2, nullptr, {}});
        GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, props2.data());
        for (uint32_t i = 0; i < *pQueueFamilyPropertyCount; ++i) {
            pQueueFamilyProperties[i] = props2[i].queueFamilyProperties;
        }
    } else {
        GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, nullptr);
    }
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                                    VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    pMemoryProperties->memoryTypeCount = 6;
    // Host visible Coherent
    pMemoryProperties->memoryTypes[0].propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    pMemoryProperties->memoryTypes[0].heapIndex = 0;
    // Host visible Cached
    pMemoryProperties->memoryTypes[1].propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    pMemoryProperties->memoryTypes[1].heapIndex = 0;
    // Device local and Host visible
    pMemoryProperties->memoryTypes[2].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                      VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
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

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char* pName) {
    if (!negotiate_loader_icd_interface_called) {
        loader_interface_version = 0;
    }
    const auto& item = name_to_func_ptr_map.find(pName);
    if (item != name_to_func_ptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }
    printf("WARNING - Failed to find %s\n", pName);
    // Mock should intercept all functions so if we get here just return null
    return nullptr;
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char* pName) {
    return GetInstanceProcAddr(nullptr, pName);
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    *pDevice = (VkDevice)CreateDispObjHandle();
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
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
            DestroyDispObjHandle((void*)cb);
        }
        command_pool_buffer_map.erase(cp);
    }
    command_pool_map[device].clear();

    queue_map.erase(device);
    buffer_map.erase(device);
    image_memory_size_map.erase(device);
    DestroyDispObjHandle((void*)device);
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                           VkExtensionProperties* pProperties) {
    // If requesting number of extensions, return that
    if (!pLayerName) {
        if (!pProperties) {
            *pPropertyCount = (uint32_t)instance_extension_map.size();
        } else {
            uint32_t i = 0;
            for (const auto& name_ver_pair : instance_extension_map) {
                if (i == *pPropertyCount) {
                    break;
                }
                std::strncpy(pProperties[i].extensionName, name_ver_pair.first.c_str(), sizeof(pProperties[i].extensionName) - 1);
                pProperties[i].extensionName[sizeof(pProperties[i].extensionName) - 1] = '\0';
                pProperties[i].specVersion = name_ver_pair.second;
                ++i;
            }
            if (i != instance_extension_map.size()) {
                return VK_INCOMPLETE;
            }
        }
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                                         uint32_t* pPropertyCount,
                                                                         VkExtensionProperties* pProperties) {
    // If requesting number of extensions, return that
    if (!pLayerName) {
        if (!pProperties) {
            *pPropertyCount = (uint32_t)device_extension_map.size();
        } else {
            uint32_t i = 0;
            for (const auto& name_ver_pair : device_extension_map) {
                if (i == *pPropertyCount) {
                    break;
                }
                std::strncpy(pProperties[i].extensionName, name_ver_pair.first.c_str(), sizeof(pProperties[i].extensionName) - 1);
                pProperties[i].extensionName[sizeof(pProperties[i].extensionName) - 1] = '\0';
                pProperties[i].specVersion = name_ver_pair.second;
                ++i;
            }
            if (i != device_extension_map.size()) {
                return VK_INCOMPLETE;
            }
        }
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                     VkLayerProperties* pProperties) {
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    unique_lock_t lock(global_lock);
    auto queue = queue_map[device][queueFamilyIndex][queueIndex];
    if (queue) {
        *pQueue = queue;
    } else {
        *pQueue = queue_map[device][queueFamilyIndex][queueIndex] = (VkQueue)CreateDispObjHandle();
    }
    return;
}

static VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                                  VkFence fence) {
    // Special way to cause DEVICE_LOST
    // Picked VkExportFenceCreateInfo because needed some struct that wouldn't get cleared by validation Safe Struct
    // ... TODO - It would be MUCH nicer to have a layer or other setting control when this occured
    // For now this is used to allow Validation Layers test reacting to device losts
    if (submitCount > 0 && pSubmits) {
        auto pNext = reinterpret_cast<const VkBaseInStructure*>(pSubmits[0].pNext);
        if (pNext && pNext->sType == VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO && pNext->pNext == nullptr) {
            return VK_ERROR_DEVICE_LOST;
        }
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    unique_lock_t lock(global_lock);
    allocated_memory_size_map[(VkDeviceMemory)global_unique_handle] = pAllocateInfo->allocationSize;
    *pMemory = (VkDeviceMemory)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    UnmapMemory(device, memory);
    unique_lock_t lock(global_lock);
    allocated_memory_size_map.erase(memory);
}

static VKAPI_ATTR VkResult VKAPI_CALL MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                                VkMemoryMapFlags flags, void** ppData) {
    unique_lock_t lock(global_lock);
    if (VK_WHOLE_SIZE == size) {
        if (allocated_memory_size_map.count(memory) != 0)
            size = allocated_memory_size_map[memory] - offset;
        else
            size = 0x10000;
    }

    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8776
    // things like shaderGroupBaseAlignment can be as big as 64, since these values are dynamically set in the Profile JSON, we need
    // to create the large alignment possible to satisfy them all
    static const size_t memory_alignment = 64;
#if defined(_WIN32)
    void* map_addr = _aligned_malloc((size_t)size, memory_alignment);
#else
    void* map_addr = aligned_alloc(memory_alignment, (size_t)size);
#endif
    mapped_memory_map[memory].push_back(map_addr);
    *ppData = map_addr;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL UnmapMemory(VkDevice device, VkDeviceMemory memory) {
    unique_lock_t lock(global_lock);
    for (auto map_addr : mapped_memory_map[memory]) {
        free(map_addr);
    }
    mapped_memory_map.erase(memory);
}

static VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                              VkMemoryRequirements* pMemoryRequirements) {
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

static VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements(VkDevice device, VkImage image,
                                                             VkMemoryRequirements* pMemoryRequirements) {
    pMemoryRequirements->size = 0;
    pMemoryRequirements->alignment = 1;

    unique_lock_t lock(global_lock);
    auto d_iter = image_memory_size_map.find(device);
    if (d_iter != image_memory_size_map.end()) {
        auto iter = d_iter->second.find(image);
        if (iter != d_iter->second.end()) {
            pMemoryRequirements->size = iter->second;
        }
    }
    // Here we hard-code that the memory type at index 3 doesn't support this image.
    pMemoryRequirements->memoryTypeBits = 0xFFFF & ~(0x1 << 3);
}

static VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                                                   uint32_t* pSparseMemoryRequirementCount,
                                                                   VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
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
        pSparseMemoryRequirements->formatProperties.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    }
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                               VkImageType type, VkSampleCountFlagBits samples,
                                                                               VkImageUsageFlags usage, VkImageTiling tiling,
                                                                               uint32_t* pPropertyCount,
                                                                               VkSparseImageFormatProperties* pProperties) {
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

static VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
    unique_lock_t lock(global_lock);
    *pBuffer = (VkBuffer)global_unique_handle++;
    buffer_map[device][*pBuffer] = {pCreateInfo->size, current_available_address};
    current_available_address += pCreateInfo->size;
    // Always align to next 64-bit pointer
    const uint64_t alignment = current_available_address % 64;
    if (alignment != 0) {
        current_available_address += (64 - alignment);
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    unique_lock_t lock(global_lock);
    buffer_map[device].erase(buffer);
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkImage* pImage) {
    unique_lock_t lock(global_lock);
    *pImage = (VkImage)global_unique_handle++;
    image_memory_size_map[device][*pImage] = GetImageSizeFromCreateInfo(pCreateInfo);
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    unique_lock_t lock(global_lock);
    image_memory_size_map[device].erase(image);
}

static VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                            VkSubresourceLayout* pLayout) {
    // Need safe values. Callers are computing memory offsets from pLayout, with no return code to flag failure.
    *pLayout = VkSubresourceLayout();  // Default constructor zero values.
}

static VKAPI_ATTR void VKAPI_CALL GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    pGranularity->width = 1;
    pGranularity->height = 1;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    unique_lock_t lock(global_lock);
    *pCommandPool = (VkCommandPool)global_unique_handle++;
    command_pool_map[device].insert(*pCommandPool);
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                     const VkAllocationCallbacks* pAllocator) {
    // destroy command buffers for this pool
    unique_lock_t lock(global_lock);
    auto it = command_pool_buffer_map.find(commandPool);
    if (it != command_pool_buffer_map.end()) {
        for (auto& cb : it->second) {
            DestroyDispObjHandle((void*)cb);
        }
        command_pool_buffer_map.erase(it);
    }
    command_pool_map[device].erase(commandPool);
}

static VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                             VkCommandBuffer* pCommandBuffers) {
    unique_lock_t lock(global_lock);
    for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; ++i) {
        pCommandBuffers[i] = (VkCommandBuffer)CreateDispObjHandle();
        command_pool_buffer_map[pAllocateInfo->commandPool].push_back(pCommandBuffers[i]);
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                     const VkCommandBuffer* pCommandBuffers) {
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

        DestroyDispObjHandle((void*)pCommandBuffers[i]);
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceVersion(uint32_t* pApiVersion) {
    *pApiVersion = VK_HEADER_VERSION_COMPLETE;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                              VkMemoryRequirements2* pMemoryRequirements) {
    GetImageMemoryRequirements(device, pInfo->image, &pMemoryRequirements->memoryRequirements);
}

static VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                               VkMemoryRequirements2* pMemoryRequirements) {
    GetBufferMemoryRequirements(device, pInfo->buffer, &pMemoryRequirements->memoryRequirements);
}

static VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2(VkDevice device,
                                                                    const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                    uint32_t* pSparseMemoryRequirementCount,
                                                                    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    if (pSparseMemoryRequirementCount && pSparseMemoryRequirements) {
        GetImageSparseMemoryRequirements(device, pInfo->image, pSparseMemoryRequirementCount,
                                         &pSparseMemoryRequirements->memoryRequirements);
    } else {
        GetImageSparseMemoryRequirements(device, pInfo->image, pSparseMemoryRequirementCount, nullptr);
    }
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    GetDeviceQueue(device, pQueueInfo->queueFamilyIndex, pQueueInfo->queueIndex, pQueue);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties) {
    constexpr VkExternalMemoryHandleTypeFlags supported_flags = 0x1FF;
    if (pExternalBufferInfo->handleType & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) {
        // Can't have dedicated memory with AHB
        pExternalBufferProperties->externalMemoryProperties.externalMemoryFeatures =
            VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
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
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
    VkExternalFenceProperties* pExternalFenceProperties) {
    // Hard-code support for all handle types and features
    pExternalFenceProperties->exportFromImportedHandleTypes = 0xF;
    pExternalFenceProperties->compatibleHandleTypes = 0xF;
    pExternalFenceProperties->externalFenceFeatures = 0x3;
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    // Hard code support for all handle types and features
    pExternalSemaphoreProperties->exportFromImportedHandleTypes = 0x1F;
    pExternalSemaphoreProperties->compatibleHandleTypes = 0x1F;
    pExternalSemaphoreProperties->externalSemaphoreFeatures = 0x3;
}

static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                                VkDescriptorSetLayoutSupport* pSupport) {
    if (pSupport) {
        pSupport->supported = VK_TRUE;
    }
}

static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
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

static VKAPI_ATTR void VKAPI_CALL GetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements) {
    pMemoryRequirements->memoryRequirements.alignment = 1;
    pMemoryRequirements->memoryRequirements.memoryTypeBits = 0xFFFF;

    // Return a size based on the buffer size from the create info.
    pMemoryRequirements->memoryRequirements.size = ((pInfo->pCreateInfo->size + 4095) / 4096) * 4096;
}

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                   VkMemoryRequirements2* pMemoryRequirements) {
    pMemoryRequirements->memoryRequirements.size = GetImageSizeFromCreateInfo(pInfo->pCreateInfo);
    pMemoryRequirements->memoryRequirements.alignment = 1;
    // Here we hard-code that the memory type at index 3 doesn't support this image.
    pMemoryRequirements->memoryRequirements.memoryTypeBits = 0xFFFF & ~(0x1 << 3);
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                         VkSurfaceKHR surface, VkBool32* pSupported) {
    // Currently say that all surface/queue combos are supported
    *pSupported = VK_TRUE;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                              VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
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
    pSurfaceCapabilities->supportedTransforms =
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR | VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR | VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR |
        VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR | VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR |
        VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR | VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR |
        VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR | VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR;
    pSurfaceCapabilities->currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    pSurfaceCapabilities->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR | VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR |
                                                    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    pSurfaceCapabilities->supportedUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                         uint32_t* pSurfaceFormatCount,
                                                                         VkSurfaceFormatKHR* pSurfaceFormats) {
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

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                              uint32_t* pPresentModeCount,
                                                                              VkPresentModeKHR* pPresentModes) {
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

static VKAPI_ATTR VkResult VKAPI_CALL CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    unique_lock_t lock(global_lock);
    *pSwapchain = (VkSwapchainKHR)global_unique_handle++;
    for (uint32_t i = 0; i < icd_swapchain_image_count; ++i) {
        swapchain_image_map[*pSwapchain][i] = (VkImage)global_unique_handle++;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                      const VkAllocationCallbacks* pAllocator) {
    unique_lock_t lock(global_lock);
    swapchain_image_map.clear();
}

static VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                            uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) {
    if (!pSwapchainImages) {
        *pSwapchainImageCount = icd_swapchain_image_count;
    } else {
        unique_lock_t lock(global_lock);
        for (uint32_t img_i = 0; img_i < (std::min)(*pSwapchainImageCount, icd_swapchain_image_count); ++img_i) {
            pSwapchainImages[img_i] = swapchain_image_map.at(swapchain)[img_i];
        }

        if (*pSwapchainImageCount < icd_swapchain_image_count)
            return VK_INCOMPLETE;
        else if (*pSwapchainImageCount > icd_swapchain_image_count)
            *pSwapchainImageCount = icd_swapchain_image_count;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                          VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
    *pImageIndex = 0;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                           uint32_t* pImageIndex) {
    *pImageIndex = 0;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t* pPropertyCount,
                                                                            VkDisplayPropertiesKHR* pProperties) {
    if (!pProperties) {
        *pPropertyCount = 1;
    } else {
        unique_lock_t lock(global_lock);
        pProperties[0].display = (VkDisplayKHR)global_unique_handle++;
        display_map[physicalDevice].insert(pProperties[0].display);
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                            const VkVideoProfileInfoKHR* pVideoProfile,
                                                                            VkVideoCapabilitiesKHR* pCapabilities) {
    return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
    uint32_t* pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR* pVideoFormatProperties) {
    return VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR;
}

static VKAPI_ATTR VkResult VKAPI_CALL
GetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t* pMemoryRequirementsCount,
                                     VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) {
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

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceFeatures2* pFeatures) {
    GetPhysicalDeviceFeatures(physicalDevice, &pFeatures->features);
    uint32_t num_bools = 0;  // Count number of VkBool32s in extension structs
    VkBool32* feat_bools = nullptr;
    auto vk_1_1_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(pFeatures->pNext);
    if (vk_1_1_features) {
        vk_1_1_features->protectedMemory = VK_TRUE;
    }
    auto vk_1_3_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(pFeatures->pNext);
    if (vk_1_3_features) {
        vk_1_3_features->synchronization2 = VK_TRUE;
    }
    auto prot_features = vku::FindStructInPNextChain<VkPhysicalDeviceProtectedMemoryFeatures>(pFeatures->pNext);
    if (prot_features) {
        prot_features->protectedMemory = VK_TRUE;
    }
    auto sync2_features = vku::FindStructInPNextChain<VkPhysicalDeviceSynchronization2FeaturesKHR>(pFeatures->pNext);
    if (sync2_features) {
        sync2_features->synchronization2 = VK_TRUE;
    }
    auto video_maintenance1_features = vku::FindStructInPNextChain<VkPhysicalDeviceVideoMaintenance1FeaturesKHR>(pFeatures->pNext);
    if (video_maintenance1_features) {
        video_maintenance1_features->videoMaintenance1 = VK_TRUE;
    }
    auto video_maintenance2_features = vku::FindStructInPNextChain<VkPhysicalDeviceVideoMaintenance2FeaturesKHR>(pFeatures->pNext);
    if (video_maintenance2_features) {
        video_maintenance2_features->videoMaintenance2 = VK_TRUE;
    }
    auto device_generated_commands_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT>(pFeatures->pNext);
    if (device_generated_commands_features) {
        device_generated_commands_features->deviceGeneratedCommands = VK_TRUE;
        device_generated_commands_features->dynamicGeneratedPipelineLayout = VK_TRUE;
    }
    const auto* desc_idx_features = vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>(pFeatures->pNext);
    if (desc_idx_features) {
        const auto bool_size = sizeof(VkPhysicalDeviceDescriptorIndexingFeaturesEXT) -
                               offsetof(VkPhysicalDeviceDescriptorIndexingFeaturesEXT, shaderInputAttachmentArrayDynamicIndexing);
        num_bools = bool_size / sizeof(VkBool32);
        feat_bools = (VkBool32*)&desc_idx_features->shaderInputAttachmentArrayDynamicIndexing;
        SetBoolArrayTrue(feat_bools, num_bools);
    }
    const auto* blendop_features = vku::FindStructInPNextChain<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT>(pFeatures->pNext);
    if (blendop_features) {
        const auto bool_size = sizeof(VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT) -
                               offsetof(VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT, advancedBlendCoherentOperations);
        num_bools = bool_size / sizeof(VkBool32);
        feat_bools = (VkBool32*)&blendop_features->advancedBlendCoherentOperations;
        SetBoolArrayTrue(feat_bools, num_bools);
    }
    const auto* host_image_copy_features = vku::FindStructInPNextChain<VkPhysicalDeviceHostImageCopyFeaturesEXT>(pFeatures->pNext);
    if (host_image_copy_features) {
        feat_bools = (VkBool32*)&host_image_copy_features->hostImageCopy;
        SetBoolArrayTrue(feat_bools, 1);
    }
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                               VkPhysicalDeviceProperties2* pProperties) {
    // The only value that need to be set are those the Profile layer can't set
    // see https://github.com/KhronosGroup/Vulkan-Profiles/issues/352
    // All values set are arbitrary
    GetPhysicalDeviceProperties(physicalDevice, &pProperties->properties);

    auto* props_11 = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Properties>(pProperties->pNext);
    if (props_11) {
        props_11->protectedNoFault = VK_FALSE;
    }

    auto* props_12 = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Properties>(pProperties->pNext);
    if (props_12) {
        props_12->denormBehaviorIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL;
        props_12->roundingModeIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL;
    }

    auto* props_13 = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Properties>(pProperties->pNext);
    if (props_13) {
        props_13->storageTexelBufferOffsetSingleTexelAlignment = VK_TRUE;
        props_13->uniformTexelBufferOffsetSingleTexelAlignment = VK_TRUE;
        props_13->storageTexelBufferOffsetAlignmentBytes = 16;
        props_13->uniformTexelBufferOffsetAlignmentBytes = 16;
    }

    auto* protected_memory_props = vku::FindStructInPNextChain<VkPhysicalDeviceProtectedMemoryProperties>(pProperties->pNext);
    if (protected_memory_props) {
        protected_memory_props->protectedNoFault = VK_FALSE;
    }

    auto* float_controls_props = vku::FindStructInPNextChain<VkPhysicalDeviceFloatControlsProperties>(pProperties->pNext);
    if (float_controls_props) {
        float_controls_props->denormBehaviorIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL;
        float_controls_props->roundingModeIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL;
    }

    auto* conservative_raster_props =
        vku::FindStructInPNextChain<VkPhysicalDeviceConservativeRasterizationPropertiesEXT>(pProperties->pNext);
    if (conservative_raster_props) {
        conservative_raster_props->primitiveOverestimationSize = 0.00195313f;
        conservative_raster_props->conservativePointAndLineRasterization = VK_TRUE;
        conservative_raster_props->degenerateTrianglesRasterized = VK_TRUE;
        conservative_raster_props->degenerateLinesRasterized = VK_TRUE;
    }

    auto* rt_pipeline_props = vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>(pProperties->pNext);
    if (rt_pipeline_props) {
        rt_pipeline_props->shaderGroupHandleSize = 32;
        rt_pipeline_props->shaderGroupBaseAlignment = 64;
        rt_pipeline_props->shaderGroupHandleCaptureReplaySize = 32;
    }

    auto* rt_pipeline_nv_props = vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPropertiesNV>(pProperties->pNext);
    if (rt_pipeline_nv_props) {
        rt_pipeline_nv_props->shaderGroupHandleSize = 32;
        rt_pipeline_nv_props->shaderGroupBaseAlignment = 64;
    }

    auto* texel_buffer_props = vku::FindStructInPNextChain<VkPhysicalDeviceTexelBufferAlignmentProperties>(pProperties->pNext);
    if (texel_buffer_props) {
        texel_buffer_props->storageTexelBufferOffsetSingleTexelAlignment = VK_TRUE;
        texel_buffer_props->uniformTexelBufferOffsetSingleTexelAlignment = VK_TRUE;
        texel_buffer_props->storageTexelBufferOffsetAlignmentBytes = 16;
        texel_buffer_props->uniformTexelBufferOffsetAlignmentBytes = 16;
    }

    auto* descriptor_buffer_props = vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorBufferPropertiesEXT>(pProperties->pNext);
    if (descriptor_buffer_props) {
        descriptor_buffer_props->combinedImageSamplerDescriptorSingleArray = VK_TRUE;
        descriptor_buffer_props->bufferlessPushDescriptors = VK_TRUE;
        descriptor_buffer_props->allowSamplerImageViewPostSubmitCreation = VK_TRUE;
        descriptor_buffer_props->descriptorBufferOffsetAlignment = 4;
    }

    auto* mesh_shader_props = vku::FindStructInPNextChain<VkPhysicalDeviceMeshShaderPropertiesEXT>(pProperties->pNext);
    if (mesh_shader_props) {
        mesh_shader_props->meshOutputPerVertexGranularity = 32;
        mesh_shader_props->meshOutputPerPrimitiveGranularity = 32;
        mesh_shader_props->prefersLocalInvocationVertexOutput = VK_TRUE;
        mesh_shader_props->prefersLocalInvocationPrimitiveOutput = VK_TRUE;
        mesh_shader_props->prefersCompactVertexOutput = VK_TRUE;
        mesh_shader_props->prefersCompactPrimitiveOutput = VK_TRUE;
    }

    auto* fragment_density_map2_props =
        vku::FindStructInPNextChain<VkPhysicalDeviceFragmentDensityMap2PropertiesEXT>(pProperties->pNext);
    if (fragment_density_map2_props) {
        fragment_density_map2_props->subsampledLoads = VK_FALSE;
        fragment_density_map2_props->subsampledCoarseReconstructionEarlyAccess = VK_FALSE;
        fragment_density_map2_props->maxSubsampledArrayLayers = 2;
        fragment_density_map2_props->maxDescriptorSetSubsampledSamplers = 1;
    }

    auto* device_generated_commands_props =
        vku::FindStructInPNextChain<VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT>(pProperties->pNext);
    if (device_generated_commands_props) {
        device_generated_commands_props->maxIndirectPipelineCount = 4096;
        device_generated_commands_props->maxIndirectShaderObjectCount = 4096;
        device_generated_commands_props->maxIndirectSequenceCount = 4096;
        device_generated_commands_props->maxIndirectCommandsTokenCount = 16;
        device_generated_commands_props->maxIndirectCommandsTokenOffset = 2048;
        device_generated_commands_props->maxIndirectCommandsIndirectStride = 2048;
        device_generated_commands_props->supportedIndirectCommandsInputModes =
            VK_INDIRECT_COMMANDS_INPUT_MODE_VULKAN_INDEX_BUFFER_EXT;
        device_generated_commands_props->supportedIndirectCommandsShaderStages =
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        device_generated_commands_props->supportedIndirectCommandsShaderStagesPipelineBinding =
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        device_generated_commands_props->supportedIndirectCommandsShaderStagesShaderBinding =
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        device_generated_commands_props->deviceGeneratedCommandsTransformFeedback = VK_TRUE;
        device_generated_commands_props->deviceGeneratedCommandsMultiDrawIndirectCount = VK_TRUE;
    }

    const uint32_t num_copy_layouts = 5;
    const VkImageLayout HostCopyLayouts[]{
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    };

    auto* host_image_copy_props = vku::FindStructInPNextChain<VkPhysicalDeviceHostImageCopyPropertiesEXT>(pProperties->pNext);
    if (host_image_copy_props) {
        if (host_image_copy_props->pCopyDstLayouts == nullptr)
            host_image_copy_props->copyDstLayoutCount = num_copy_layouts;
        else {
            uint32_t num_layouts = (std::min)(host_image_copy_props->copyDstLayoutCount, num_copy_layouts);
            for (uint32_t i = 0; i < num_layouts; i++) {
                host_image_copy_props->pCopyDstLayouts[i] = HostCopyLayouts[i];
            }
        }
        if (host_image_copy_props->pCopySrcLayouts == nullptr)
            host_image_copy_props->copySrcLayoutCount = num_copy_layouts;
        else {
            uint32_t num_layouts = (std::min)(host_image_copy_props->copySrcLayoutCount, num_copy_layouts);
            for (uint32_t i = 0; i < num_layouts; i++) {
                host_image_copy_props->pCopySrcLayouts[i] = HostCopyLayouts[i];
            }
        }
    }

    auto* driver_properties = vku::FindStructInPNextChain<VkPhysicalDeviceDriverProperties>(pProperties->pNext);
    if (driver_properties) {
        std::strncpy(driver_properties->driverName, "Vulkan Mock Device", VK_MAX_DRIVER_NAME_SIZE);
#if defined(GIT_BRANCH_NAME) && defined(GIT_TAG_INFO)
        std::strncpy(driver_properties->driverInfo, "Branch: " GIT_BRANCH_NAME " Tag Info: " GIT_TAG_INFO, VK_MAX_DRIVER_INFO_SIZE);
#else
        std::strncpy(driver_properties->driverInfo, "Branch: --unknown-- Tag Info: --unknown--", VK_MAX_DRIVER_INFO_SIZE);
#endif
    }
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                     VkFormatProperties2* pFormatProperties) {
    GetPhysicalDeviceFormatProperties(physicalDevice, format, &pFormatProperties->formatProperties);
    VkFormatProperties3KHR* props_3 = vku::FindStructInPNextChain<VkFormatProperties3KHR>(pFormatProperties->pNext);
    if (props_3) {
        props_3->linearTilingFeatures = pFormatProperties->formatProperties.linearTilingFeatures;
        props_3->optimalTilingFeatures = pFormatProperties->formatProperties.optimalTilingFeatures;
        props_3->bufferFeatures = pFormatProperties->formatProperties.bufferFeatures;
        props_3->optimalTilingFeatures |= VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT;

        switch (format) {
            case VK_FORMAT_R32_SINT:
                props_3->linearTilingFeatures |= VK_FORMAT_FEATURE_2_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;
                props_3->optimalTilingFeatures |= VK_FORMAT_FEATURE_2_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;
                break;
            case VK_FORMAT_R8_UNORM:
                props_3->linearTilingFeatures |= VK_FORMAT_FEATURE_2_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR;
                props_3->optimalTilingFeatures |= VK_FORMAT_FEATURE_2_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR;
                break;
            default:
                break;
        }
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL
GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                        VkImageFormatProperties2* pImageFormatProperties) {
    auto* external_image_prop = vku::FindStructInPNextChain<VkExternalImageFormatProperties>(pImageFormatProperties->pNext);
    auto* external_image_format = vku::FindStructInPNextChain<VkPhysicalDeviceExternalImageFormatInfo>(pImageFormatInfo->pNext);
    if (external_image_prop && external_image_format) {
        external_image_prop->externalMemoryProperties.externalMemoryFeatures =
            VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
        external_image_prop->externalMemoryProperties.compatibleHandleTypes = external_image_format->handleType;
    }

    GetPhysicalDeviceImageFormatProperties(physicalDevice, pImageFormatInfo->format, pImageFormatInfo->type,
                                           pImageFormatInfo->tiling, pImageFormatInfo->usage, pImageFormatInfo->flags,
                                           &pImageFormatProperties->imageFormatProperties);
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                          uint32_t* pQueueFamilyPropertyCount,
                                                                          VkQueueFamilyProperties2* pQueueFamilyProperties) {
    if (pQueueFamilyProperties) {
        if (*pQueueFamilyPropertyCount >= 1) {
            auto props = &pQueueFamilyProperties[0].queueFamilyProperties;
            props->queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT |
                                VK_QUEUE_PROTECTED_BIT;
            props->queueCount = 1;
            props->timestampValidBits = 16;
            props->minImageTransferGranularity = {1, 1, 1};
        }
        if (*pQueueFamilyPropertyCount >= 2) {
            auto props = &pQueueFamilyProperties[1].queueFamilyProperties;
            props->queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_PROTECTED_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR;
            props->queueCount = 1;
            props->timestampValidBits = 16;
            props->minImageTransferGranularity = {1, 1, 1};

            auto status_query_props =
                vku::FindStructInPNextChain<VkQueueFamilyQueryResultStatusPropertiesKHR>(pQueueFamilyProperties[1].pNext);
            if (status_query_props) {
                status_query_props->queryResultStatusSupport = VK_TRUE;
            }
            auto video_props = vku::FindStructInPNextChain<VkQueueFamilyVideoPropertiesKHR>(pQueueFamilyProperties[1].pNext);
            if (video_props) {
                video_props->videoCodecOperations = VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR |
                                                    VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR |
                                                    VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR;
            }
        }
        if (*pQueueFamilyPropertyCount >= 3) {
            auto props = &pQueueFamilyProperties[2].queueFamilyProperties;
            props->queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_PROTECTED_BIT | VK_QUEUE_VIDEO_ENCODE_BIT_KHR;
            props->queueCount = 1;
            props->timestampValidBits = 16;
            props->minImageTransferGranularity = {1, 1, 1};

            auto status_query_props =
                vku::FindStructInPNextChain<VkQueueFamilyQueryResultStatusPropertiesKHR>(pQueueFamilyProperties[2].pNext);
            if (status_query_props) {
                status_query_props->queryResultStatusSupport = VK_TRUE;
            }
            auto video_props = vku::FindStructInPNextChain<VkQueueFamilyVideoPropertiesKHR>(pQueueFamilyProperties[2].pNext);
            if (video_props) {
                video_props->videoCodecOperations = VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR |
                                                    VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR |
                                                    VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR;
            }
        }
        if (*pQueueFamilyPropertyCount > 3) {
            *pQueueFamilyPropertyCount = 3;
        }
    } else {
        *pQueueFamilyPropertyCount = 3;
    }
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                                     VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    GetPhysicalDeviceMemoryProperties(physicalDevice, &pMemoryProperties->memoryProperties);
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties) {
    if (pPropertyCount && pProperties) {
        GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, pFormatInfo->format, pFormatInfo->type, pFormatInfo->samples,
                                                     pFormatInfo->usage, pFormatInfo->tiling, pPropertyCount,
                                                     &pProperties->properties);
    } else {
        GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, pFormatInfo->format, pFormatInfo->type, pFormatInfo->samples,
                                                     pFormatInfo->usage, pFormatInfo->tiling, pPropertyCount, nullptr);
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
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

#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL
GetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
                                  VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) {
    pMemoryWin32HandleProperties->memoryTypeBits = 0xFFFF;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) {
    *pFd = 1;
    return VK_SUCCESS;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL GetFenceWin32HandleKHR(VkDevice device,
                                                             const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                             HANDLE* pHandle) {
    *pHandle = (HANDLE)0x12345678;
    return VK_SUCCESS;
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */

static VKAPI_ATTR VkResult VKAPI_CALL GetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) {
    *pFd = 0x42;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions) {
    if (!pCounters) {
        *pCounterCount = 3;
    } else {
        if (*pCounterCount == 0) {
            return VK_INCOMPLETE;
        }
        // arbitrary
        pCounters[0].unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR;
        pCounters[0].scope = VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_BUFFER_KHR;
        pCounters[0].storage = VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR;
        pCounters[0].uuid[0] = 0x01;
        if (*pCounterCount == 1) {
            return VK_INCOMPLETE;
        }
        pCounters[1].unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR;
        pCounters[1].scope = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR;
        pCounters[1].storage = VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR;
        pCounters[1].uuid[0] = 0x02;
        if (*pCounterCount == 2) {
            return VK_INCOMPLETE;
        }
        pCounters[2].unit = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR;
        pCounters[2].scope = VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR;
        pCounters[2].storage = VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR;
        pCounters[2].uuid[0] = 0x03;
        *pCounterCount = 3;
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
    if (pNumPasses) {
        // arbitrary
        *pNumPasses = 1;
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                               const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                               VkSurfaceCapabilities2KHR* pSurfaceCapabilities) {
    GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, pSurfaceInfo->surface, &pSurfaceCapabilities->surfaceCapabilities);

    auto* present_mode_compatibility =
        vku::FindStructInPNextChain<VkSurfacePresentModeCompatibilityEXT>(pSurfaceCapabilities->pNext);
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

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                                               VkSurfaceKHR surface,
                                                                               VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    VkSurfaceCapabilitiesKHR surface_capabilities_khr;
    GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surface_capabilities_khr);
    pSurfaceCapabilities->minImageCount = surface_capabilities_khr.minImageCount;
    pSurfaceCapabilities->maxImageCount = surface_capabilities_khr.maxImageCount;
    pSurfaceCapabilities->currentExtent = surface_capabilities_khr.currentExtent;
    pSurfaceCapabilities->minImageExtent = surface_capabilities_khr.minImageExtent;
    pSurfaceCapabilities->maxImageExtent = surface_capabilities_khr.maxImageExtent;
    pSurfaceCapabilities->maxImageArrayLayers = surface_capabilities_khr.maxImageArrayLayers;
    pSurfaceCapabilities->supportedTransforms = surface_capabilities_khr.supportedTransforms;
    pSurfaceCapabilities->currentTransform = surface_capabilities_khr.currentTransform;
    pSurfaceCapabilities->supportedCompositeAlpha = surface_capabilities_khr.supportedCompositeAlpha;
    pSurfaceCapabilities->supportedUsageFlags = surface_capabilities_khr.supportedUsageFlags;
    pSurfaceCapabilities->supportedSurfaceCounters = VK_SURFACE_COUNTER_VBLANK_BIT_EXT;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                          const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                          uint32_t* pSurfaceFormatCount,
                                                                          VkSurfaceFormat2KHR* pSurfaceFormats) {
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

static VKAPI_ATTR VkResult VKAPI_CALL
GetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount,
                                         VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
    if (!pFragmentShadingRates) {
        *pFragmentShadingRateCount = 1;
    } else {
        // arbitrary
        pFragmentShadingRates->sampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT;
        pFragmentShadingRates->fragmentSize = {8, 8};
    }
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL MapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData) {
    return MapMemory(device, pMemoryMapInfo->memory, pMemoryMapInfo->offset, pMemoryMapInfo->size, pMemoryMapInfo->flags, ppData);
}

static VKAPI_ATTR VkResult VKAPI_CALL UnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo) {
    UnmapMemory(device, pMemoryUnmapInfo->memory);
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixPropertiesKHR(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesKHR* pProperties) {
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

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice,
                                                                                   uint32_t* pTimeDomainCount,
                                                                                   VkTimeDomainKHR* pTimeDomains) {
    if (!pTimeDomains) {
        *pTimeDomainCount = 1;
    } else {
        // arbitrary
        *pTimeDomains = VK_TIME_DOMAIN_DEVICE_KHR;
    }
    return VK_SUCCESS;
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR

static VKAPI_ATTR VkResult VKAPI_CALL GetAndroidHardwareBufferPropertiesANDROID(
    VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    pProperties->allocationSize = 65536;
    pProperties->memoryTypeBits = 1 << 5;  // DEVICE_LOCAL only type

    auto* format_prop = vku::FindStructInPNextChain<VkAndroidHardwareBufferFormatPropertiesANDROID>(pProperties->pNext);
    if (format_prop) {
        // Likley using this format
        format_prop->format = VK_FORMAT_R8G8B8A8_UNORM;
        format_prop->externalFormat = 37;
    }

    auto* format_resolve_prop =
        vku::FindStructInPNextChain<VkAndroidHardwareBufferFormatResolvePropertiesANDROID>(pProperties->pNext);
    if (format_resolve_prop) {
        format_resolve_prop->colorAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    }
    return VK_SUCCESS;
}

#endif /* VK_USE_PLATFORM_ANDROID_KHR */

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice,
                                                                            VkSampleCountFlagBits samples,
                                                                            VkMultisamplePropertiesEXT* pMultisampleProperties) {
    if (pMultisampleProperties) {
        // arbitrary
        pMultisampleProperties->maxSampleLocationGridSize = {32, 32};
    }
}

static VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) {
    // arbitrary
    pMemoryRequirements->memoryRequirements.size = 4096;
    pMemoryRequirements->memoryRequirements.alignment = 1;
    pMemoryRequirements->memoryRequirements.memoryTypeBits = 0xFFFF;
}

static VKAPI_ATTR VkResult VKAPI_CALL
GetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer,
                                  VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) {
    pMemoryHostPointerProperties->memoryTypeBits = 1 << 5;  // DEVICE_LOCAL only type
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                VkDeviceSize* pLayoutSizeInBytes) {
    // Need to give something non-zero
    *pLayoutSizeInBytes = 4;
}

static VKAPI_ATTR void VKAPI_CALL GetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                               VkShaderModuleIdentifierEXT* pIdentifier) {
    if (pIdentifier) {
        // arbitrary
        pIdentifier->identifierSize = 1;
        pIdentifier->identifier[0] = 0x01;
    }
}

static VKAPI_ATTR VkDeviceAddress VKAPI_CALL
GetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    // arbitrary - need to be aligned to 256 bytes
    return 0x262144;
}

static VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    // arbitrary
    pSizeInfo->accelerationStructureSize = 4;
    pSizeInfo->updateScratchSize = 4;
    pSizeInfo->buildScratchSize = 4;
}

static VKAPI_ATTR VkResult VKAPI_CALL RegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                              const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    unique_lock_t lock(global_lock);
    *pFence = (VkFence)global_unique_handle++;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkPipelineBinaryHandlesInfoKHR* pBinaries) {
    unique_lock_t lock(global_lock);

    pBinaries->pipelineBinaryCount = 1;

    if (pBinaries->pPipelineBinaries != nullptr) {
        pBinaries->pPipelineBinaries[0] = (VkPipelineBinaryKHR)global_unique_handle++;
    }

    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                                               VkPipelineBinaryKeyKHR* pPipelineBinaryKey,
                                                               size_t* pPipelineBinaryDataSize, void* pPipelineBinaryData) {
    *pPipelineBinaryDataSize = 1;

    if (pPipelineBinaryData != nullptr) {
        *reinterpret_cast<uint8_t*>(pPipelineBinaryData) = 0x01;
    }

    return VK_SUCCESS;
}

}  // namespace icd
