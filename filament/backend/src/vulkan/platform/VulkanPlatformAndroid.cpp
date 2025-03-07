/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <backend/platforms/VulkanPlatform.h>

#include "vulkan/VulkanConstants.h"


#include <utils/Panic.h>
#include "vulkan/utils/Image.h"
#include "vulkan/utils/Conversion.h"

#include <bluevk/BlueVK.h>

#include <android/hardware_buffer.h>
#include <android/native_window.h>

using namespace bluevk;

namespace filament::backend {

namespace {
void getVKFormatAndUsage(const AHardwareBuffer_Desc& desc, VkFormat& format,
        VkImageUsageFlags& usage, bool& isProtected) {
    // Refer to "11.2.17. External Memory Handle Types" in the spec, and
    // Tables 13/14 for how the following derivation works.
    bool isDepthFormat = false;
    isProtected = false;
    switch (desc.format) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
            format = VK_FORMAT_R8G8B8_UNORM;
            break;
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
            format = VK_FORMAT_R5G6B5_UNORM_PACK16;
            break;
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
            format = VK_FORMAT_R16G16B16A16_SFLOAT;
            break;
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
            format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            break;
        case AHARDWAREBUFFER_FORMAT_D16_UNORM:
            isDepthFormat = true;
            format = VK_FORMAT_D16_UNORM;
            break;
        case AHARDWAREBUFFER_FORMAT_D24_UNORM:
            isDepthFormat = true;
            format = VK_FORMAT_X8_D24_UNORM_PACK32;
            break;
        case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
            isDepthFormat = true;
            format = VK_FORMAT_D24_UNORM_S8_UINT;
            break;
        case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
            isDepthFormat = true;
            format = VK_FORMAT_D32_SFLOAT;
            break;
        case AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
            isDepthFormat = true;
            format = VK_FORMAT_D32_SFLOAT_S8_UINT;
            break;
        case AHARDWAREBUFFER_FORMAT_S8_UINT:
            isDepthFormat = true;
            format = VK_FORMAT_S8_UINT;
            break;
        default:
            format = VK_FORMAT_UNDEFINED;
    }

    // The following only concern usage flags derived from Table 14.
    usage = 0;
    if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) {
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) {
        if (isDepthFormat) {
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }
    if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER) {
        usage = VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (desc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT) {
        isProtected = true;
    }
}

VulkanPlatform::ImageData allocateExternalImage(void* externalBuffer,
        VkDevice device, const VkAllocationCallbacks* allocator,
        VulkanPlatform::ExternalImageMetadata const& metadata) {
    VulkanPlatform::ImageData data;
    AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalBuffer);

    // if external format we need to specifiy it in the allocation
    const bool useExternalFormat = metadata.format == VK_FORMAT_UNDEFINED;

    const VkExternalFormatANDROID externalFormat = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
        .pNext = nullptr,
        .externalFormat = metadata
                .externalFormat,// pass down the format (external means we don't have it VK defined)
    };
    const VkExternalMemoryImageCreateInfo externalCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
        .pNext = useExternalFormat ? &externalFormat : nullptr,
        .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
    };

    VkImageCreateInfo imageInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.pNext = &externalCreateInfo;
    imageInfo.format = metadata.format;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = {
        metadata.width,
        metadata.height,
        1u,
    };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = metadata.layers;
    imageInfo.usage = metadata.usage;
    // In the unprotected case add R/W capabilities
    if (metadata.isProtected == false)
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    VkResult result = vkCreateImage(device, &imageInfo, allocator, &data.first);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkCreateImage failed with error=" << static_cast<int32_t>(result);

    // Allocate the memory
    VkImportAndroidHardwareBufferInfoANDROID androidHardwareBufferInfo = {
        .sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID,
        .pNext = nullptr,
        .buffer = buffer,
    };
    VkMemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
        .pNext = &androidHardwareBufferInfo,
        .image = data.first,
        .buffer = VK_NULL_HANDLE,
    };
    VkMemoryAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &memoryDedicatedAllocateInfo,
        .allocationSize = metadata.allocationSize,
        .memoryTypeIndex = metadata.memoryTypeBits};
    result = vkAllocateMemory(device, &allocInfo, allocator, &data.second);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkAllocateMemory failed with error=" << static_cast<int32_t>(result);

    return data;
}

}// namespace

VulkanPlatform::ExternalImageMetadata VulkanPlatform::getExternalImageMetadataImpl(void* externalImage,
        VkDevice device) {
    ExternalImageMetadata metadata;
    AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalImage);
    if (__builtin_available(android 26, *)) {
        AHardwareBuffer_Desc bufferDesc;
        AHardwareBuffer_describe(buffer, &bufferDesc);
        metadata.width = bufferDesc.width;
        metadata.height = bufferDesc.height;
        metadata.layers = bufferDesc.layers;

        getVKFormatAndUsage(bufferDesc, metadata.format, metadata.usage, metadata.isProtected);
    }
    // In the unprotected case add R/W capabilities
    if (metadata.isProtected == false) {
        metadata.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    VkAndroidHardwareBufferFormatPropertiesANDROID formatInfo = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID,
        .pNext = nullptr,
    };
    VkAndroidHardwareBufferPropertiesANDROID properties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
        .pNext = &formatInfo,
    };
    VkResult result = vkGetAndroidHardwareBufferPropertiesANDROID(device, buffer, &properties);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkGetAndroidHardwareBufferProperties failed with error="
            << static_cast<int32_t>(result);
    FILAMENT_CHECK_POSTCONDITION(metadata.format == formatInfo.format)
            << "mismatched image format for external image (AHB)";
    metadata.externalFormat = formatInfo.externalFormat;
    metadata.allocationSize = properties.allocationSize;
    metadata.memoryTypeBits = properties.memoryTypeBits;
    return metadata;
}

VulkanPlatform::ImageData VulkanPlatform::createExternalImageImpl(
        void* externalImage, VkDevice device,
        const ExternalImageMetadata& metadata) {
    ImageData data =
        allocateExternalImage(externalImage, device, VKALLOC, metadata);
    VkResult result = vkBindImageMemory(device, data.first, data.second, 0);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
        << "vkBindImageMemory error=" << static_cast<int32_t>(result);
    return data;
}

VkSampler VulkanPlatform::createExternalSamplerImpl(
        VkDevice device, SamplerYcbcrConversion chroma, SamplerParams params,
        uint32_t internalFormat) {
    VkExternalFormatANDROID externalFormat = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
        .pNext = nullptr,
        .externalFormat = internalFormat,
    };

    TextureSwizzle const swizzleArray[] = {chroma.r, chroma.g, chroma.b, chroma.a};
    VkSamplerYcbcrConversionCreateInfo conversionInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO,
        .pNext = &externalFormat,
        .format = VK_FORMAT_UNDEFINED,
        .ycbcrModel = fvkutils::getYcbcrModelConversion(chroma.ycbcrModel),
        .ycbcrRange = fvkutils::getYcbcrRange(chroma.ycbcrRange),
        .components = fvkutils::getSwizzleMap(swizzleArray),
        .xChromaOffset = fvkutils::getChromaLocation(chroma.xChromaOffset),
        .yChromaOffset = fvkutils::getChromaLocation(chroma.yChromaOffset),
        .chromaFilter = (chroma.chromaFilter == SamplerMagFilter::NEAREST)
                            ? VK_FILTER_NEAREST
                            : VK_FILTER_LINEAR,
    };
    VkSamplerYcbcrConversion conversion = VK_NULL_HANDLE;
    VkResult result = vkCreateSamplerYcbcrConversion(device, &conversionInfo,
                                                     nullptr, &conversion);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
        << "Unable to create Ycbcr Conversion."
        << " error=" << static_cast<int32_t>(result);

    VkSamplerYcbcrConversionInfo samplerYcbcrConversionInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO,
        .pNext = nullptr,
        .conversion = conversion,
    };

    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = &samplerYcbcrConversionInfo,
        .magFilter = fvkutils::getFilter(params.filterMag),
        .minFilter = fvkutils::getFilter(params.filterMin),
        .mipmapMode = fvkutils::getMipmapMode(params.filterMin),
        .addressModeU = fvkutils::getWrapMode(params.wrapS),
        .addressModeV = fvkutils::getWrapMode(params.wrapT),
        .addressModeW = fvkutils::getWrapMode(params.wrapR),
        .anisotropyEnable = params.anisotropyLog2 == 0 ? 0u : 1u,
        .maxAnisotropy = (float)(1u << params.anisotropyLog2),
        .compareEnable = fvkutils::getCompareEnable(params.compareMode),
        .compareOp = fvkutils::getCompareOp(params.compareFunc),
        .minLod = 0.0f,
        .maxLod = fvkutils::getMaxLod(params.filterMin),
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    VkSampler sampler;
    result = vkCreateSampler(device, &samplerInfo, VKALLOC, &sampler);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
        << "Unable to create sampler."
        << " error=" << static_cast<int32_t>(result);
    return sampler;
}

VulkanPlatform::ExtensionSet VulkanPlatform::getSwapchainInstanceExtensions() {
    return {
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
    };
}

VulkanPlatform::SurfaceBundle VulkanPlatform::createVkSurfaceKHR(void* nativeWindow,
    VkInstance instance, uint64_t flags) noexcept {
    VkSurfaceKHR surface;
    VkExtent2D extent;

    VkAndroidSurfaceCreateInfoKHR const createInfo{
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .window = (ANativeWindow*) nativeWindow,
    };
    VkResult const result =
            vkCreateAndroidSurfaceKHR(instance, &createInfo, VKALLOC, (VkSurfaceKHR*) &surface);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkCreateAndroidSurfaceKHR with error=" << static_cast<int32_t>(result);
    return {surface, extent};
}
}
