/*
** Copyright (c) 2015-2018, 2023 The Khronos Group Inc.
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

#pragma once

#include <stdlib.h>
#include <cstring>

#include <algorithm>
#include <array>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>

#include "vk_video/vulkan_video_codecs_common.h"
#include "vk_video/vulkan_video_codec_h264std.h"
#include "vk_video/vulkan_video_codec_h264std_decode.h"
#include "vk_video/vulkan_video_codec_h264std_encode.h"
#include "vk_video/vulkan_video_codec_h265std.h"
#include "vk_video/vulkan_video_codec_h265std_decode.h"
#include "vk_video/vulkan_video_codec_h265std_encode.h"
#include "vulkan/vulkan.h"

#include "vulkan/vk_icd.h"
#include "vk_typemap_helper.h"

namespace vkmock {

using mutex_t = std::mutex;
using lock_guard_t = std::lock_guard<mutex_t>;
using unique_lock_t = std::unique_lock<mutex_t>;

static mutex_t global_lock;
static uint64_t global_unique_handle = 1;
static const uint32_t SUPPORTED_LOADER_ICD_INTERFACE_VERSION = 5;
static uint32_t loader_interface_version = 0;
static bool negotiate_loader_icd_interface_called = false;
static void* CreateDispObjHandle() {
    auto handle = new VK_LOADER_DATA;
    set_loader_magic_value(handle);
    return handle;
}
static void DestroyDispObjHandle(void* handle) { delete reinterpret_cast<VK_LOADER_DATA*>(handle); }

static constexpr uint32_t icd_physical_device_count = 1;
static std::unordered_map<VkInstance, std::array<VkPhysicalDevice, icd_physical_device_count>> physical_device_map;
static std::unordered_map<VkPhysicalDevice, std::unordered_set<VkDisplayKHR>> display_map;

// Map device memory handle to any mapped allocations that we'll need to free on unmap
static std::unordered_map<VkDeviceMemory, std::vector<void*>> mapped_memory_map;

// Map device memory allocation handle to the size
static std::unordered_map<VkDeviceMemory, VkDeviceSize> allocated_memory_size_map;

static std::unordered_map<VkDevice, std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkQueue>>> queue_map;
static VkDeviceAddress current_available_address = 0x10000000;
struct BufferState {
    VkDeviceSize size;
    VkDeviceAddress address;
};
static std::unordered_map<VkDevice, std::unordered_map<VkBuffer, BufferState>> buffer_map;
static std::unordered_map<VkDevice, std::unordered_map<VkImage, VkDeviceSize>> image_memory_size_map;
static std::unordered_map<VkDevice, std::unordered_set<VkCommandPool>> command_pool_map;
static std::unordered_map<VkCommandPool, std::vector<VkCommandBuffer>> command_pool_buffer_map;

static constexpr uint32_t icd_swapchain_image_count = 1;
static std::unordered_map<VkSwapchainKHR, VkImage[icd_swapchain_image_count]> swapchain_image_map;

// TODO: Would like to codegen this but limits aren't in XML
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

void SetBoolArrayTrue(VkBool32* bool_array, uint32_t num_bools) {
    for (uint32_t i = 0; i < num_bools; ++i) {
        bool_array[i] = VK_TRUE;
    }
}

VkDeviceSize GetImageSizeFromCreateInfo(const VkImageCreateInfo* pCreateInfo) {
    VkDeviceSize size = pCreateInfo->extent.width;
    size *= pCreateInfo->extent.height;
    size *= pCreateInfo->extent.depth;
    // TODO: A pixel size is 32 bytes. This accounts for the largest possible pixel size of any format. It could be changed to more
    // accurate size if need be.
    size *= 32;
    size *= pCreateInfo->arrayLayers;
    size *= (pCreateInfo->mipLevels > 1 ? 2 : 1);

    switch (pCreateInfo->format) {
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
            size *= 3;
            break;
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
            size *= 2;
            break;
        default:
            break;
    }

    return size;
}

}  // namespace vkmock
