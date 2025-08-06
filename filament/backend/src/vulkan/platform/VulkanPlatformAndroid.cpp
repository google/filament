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
#include <backend/platforms/VulkanPlatformAndroid.h>

#include "vulkan/VulkanConstants.h"
#include "vulkan/VulkanContext.h"

#include <backend/DriverEnums.h>
#include <private/backend/BackendUtilsAndroid.h>

#include <utils/Panic.h>
#include "vulkan/utils/Image.h"

#include <bluevk/BlueVK.h>

#include <android/hardware_buffer.h>
#include <android/native_window.h>

#include <utility>

using namespace bluevk;

namespace filament::backend {

namespace {

VkFormat transformVkFormat(VkFormat format, bool sRGB) {
    if (!sRGB) {
        switch (format) {
            case VK_FORMAT_R8G8B8A8_SRGB:
               return VK_FORMAT_R8G8B8A8_UNORM;
            case VK_FORMAT_R8G8B8_SRGB:
                return VK_FORMAT_R8G8B8_UNORM;
            default:
                break;
        }
        return format;
    }

    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            format = VK_FORMAT_R8G8B8A8_SRGB;
            break;
        case VK_FORMAT_R8G8B8_UNORM:
            format = VK_FORMAT_R8G8B8_SRGB;
            break;
        default:
            break;
    }

    return format;
}

bool isProtectedFromUsage(uint64_t usage) {
    return usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
}

std::pair<VkFormat, VkImageUsageFlags> getVKFormatAndUsage(const AHardwareBuffer_Desc& desc,
        bool sRGB) {
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImageUsageFlags usage = 0;
    // Refer to "11.2.17. External Memory Handle Types" in the spec, and
    // Tables 13/14 for how the following derivation works.
    bool isDepthFormat = false;
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

    format = transformVkFormat(format, sRGB);

    // The following only concern usage flags derived from Table 14.
    usage = 0;
    if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) {
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

        // We shouldn't be using external samplers as input attachments
        // usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
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

    return { format, usage };
}

std::pair<TextureFormat, TextureUsage> getFilamentFormatAndUsage(const AHardwareBuffer_Desc& desc,
        bool sRGB) {
    auto const format = mapToFilamentFormat(desc.format, sRGB);
    return {
        format,
        mapToFilamentUsage(desc.usage, format),
    };
}

}// namespace

VulkanPlatformAndroid::ExternalImageVulkanAndroid::~ExternalImageVulkanAndroid() {
    if (__builtin_available(android 26, *)) {
        if (aHardwareBuffer) {
            AHardwareBuffer_release(aHardwareBuffer);
        }
    }
}

Platform::ExternalImageHandle VulkanPlatformAndroid::createExternalImageFromRaw(void* image,
        bool sRGB) const noexcept {
    return createExternalImage(reinterpret_cast<AHardwareBuffer*>(image), sRGB);
}

Platform::ExternalImageHandle VulkanPlatformAndroid::createExternalImage(
        AHardwareBuffer const* buffer, bool sRGB) noexcept {
    if (__builtin_available(android 26, *)) {
        auto bufferImpl = const_cast<AHardwareBuffer*>(buffer);
        AHardwareBuffer_acquire(bufferImpl);

        AHardwareBuffer_Desc hardwareBufferDescription = {};
        AHardwareBuffer_describe(buffer, &hardwareBufferDescription);

        auto* const p = new (std::nothrow) ExternalImageVulkanAndroid;
        p->aHardwareBuffer = const_cast<AHardwareBuffer*>(buffer);
        p->sRGB = sRGB;
        return Platform::ExternalImageHandle{ p };
    }

    return Platform::ExternalImageHandle{};
}

VulkanPlatformAndroid::ExternalImageDescAndroid VulkanPlatformAndroid::getExternalImageDesc(
        ExternalImageHandleRef externalImage) const noexcept {
    auto metadata = extractExternalImageMetadata(externalImage);
    return {
        .width = metadata.width,
        .height = metadata.height,
        .format = metadata.filamentFormat,
        .usage = metadata.filamentUsage,
    };
}

VulkanPlatform::ExternalImageMetadata VulkanPlatformAndroid::extractExternalImageMetadata(
        ExternalImageHandleRef image) const {
    auto const* fvkExternalImage = static_cast<ExternalImageVulkanAndroid const*>(image.get());

    ExternalImageMetadata metadata = {};
    AHardwareBuffer* buffer = fvkExternalImage->aHardwareBuffer;
    if (__builtin_available(android 26, *)) {
        AHardwareBuffer_Desc bufferDesc;
        AHardwareBuffer_describe(buffer, &bufferDesc);
        metadata.width = bufferDesc.width;
        metadata.height = bufferDesc.height;
        metadata.layers = bufferDesc.layers;
        std::tie(metadata.format, metadata.usage) =
                getVKFormatAndUsage(bufferDesc, fvkExternalImage->sRGB);
        std::tie(metadata.filamentFormat, metadata.filamentUsage) =
                getFilamentFormatAndUsage(bufferDesc, fvkExternalImage->sRGB);

        if (isProtectedFromUsage(bufferDesc.usage)) {
            metadata.filamentUsage |= TextureUsage::PROTECTED;
        }

        // TODO: The following seems unnecessary. we should be able to discern directly from the
        // bufferDesc.
        if (any(metadata.filamentUsage & TextureUsage::BLIT_SRC)) {
            metadata.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if (any(metadata.filamentUsage & (TextureUsage::BLIT_DST | TextureUsage::UPLOADABLE))) {
            metadata.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
    }
    metadata.samples = VK_SAMPLE_COUNT_1_BIT;

    VkAndroidHardwareBufferFormatPropertiesANDROID formatInfo = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID,
    };
    VkAndroidHardwareBufferPropertiesANDROID properties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
        .pNext = &formatInfo,
    };
    VkResult result = vkGetAndroidHardwareBufferPropertiesANDROID(getDevice(), buffer, &properties);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkGetAndroidHardwareBufferProperties failed with error="
            << static_cast<int32_t>(result);
    VkFormat bufferPropertiesFormat = transformVkFormat(formatInfo.format, fvkExternalImage->sRGB);
    FILAMENT_CHECK_POSTCONDITION(metadata.format == bufferPropertiesFormat)
            << "mismatched image format( " << metadata.format << ") and queried format("
            << bufferPropertiesFormat << ") for external image (AHB)";

    bool const requiresConversion =
        metadata.format == VK_FORMAT_UNDEFINED ||
        fvkutils::isVKYcbcrConversionFormat(metadata.format);
    if (requiresConversion) {
      metadata.format = VK_FORMAT_UNDEFINED;
      metadata.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
      metadata.externalFormat = formatInfo.externalFormat;
    } else {
      metadata.externalFormat = 0;
    }

    metadata.allocationSize = properties.allocationSize;
    metadata.memoryTypeBits = properties.memoryTypeBits;

    metadata.ycbcrConversionComponents = formatInfo.samplerYcbcrConversionComponents;
    metadata.ycbcrModel = formatInfo.suggestedYcbcrModel;
    metadata.ycbcrRange = formatInfo.suggestedYcbcrRange;
    metadata.xChromaOffset = formatInfo.suggestedXChromaOffset;
    metadata.yChromaOffset = formatInfo.suggestedYChromaOffset;

    return metadata;
}

VulkanPlatform::ImageData VulkanPlatformAndroid::createVkImageFromExternal(
        ExternalImageHandleRef externalImage) const {
    auto metadata = extractExternalImageMetadata(externalImage);

    auto const* fvkExternalImage =
            static_cast<ExternalImageVulkanAndroid const*>(externalImage.get());
    AHardwareBuffer* buffer = fvkExternalImage->aHardwareBuffer;

    VkDevice const device = getDevice();
    VkPhysicalDevice const physicalDevice = getPhysicalDevice();
    auto buildImage = [&](ExternalImageMetadata const& metadata) {
        bool const isExternal = metadata.externalFormat != 0;
        VkExternalFormatANDROID const externalFormat = {
            .sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
            .pNext = nullptr,
            .externalFormat = metadata.externalFormat,
        };
        VkExternalMemoryImageCreateInfo externalCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
            .pNext = isExternal ? &externalFormat : nullptr,
            .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
        };

        VkFormat formats[2] = {};
        VkImageFormatListCreateInfo imageFormatListInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO,
            .pNext = nullptr,
            .viewFormatCount = 2,
            .pViewFormats = formats,
        };

        if (fvkExternalImage->sRGB) {
            formats[0] = metadata.format;
            formats[1] = transformVkFormat(metadata.format, /*sRGB=*/false);
            imageFormatListInfo.pNext = externalCreateInfo.pNext;
            externalCreateInfo.pNext = &imageFormatListInfo;
        }

        VkImageCreateInfo const imageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = &externalCreateInfo,
            .flags = fvkExternalImage->sRGB ? VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT : 0u,
            .imageType = VK_IMAGE_TYPE_2D,
            // For non external images, use the same format as the AHB, which isn't in SRGB
            // Fix VUID-VkMemoryAllocateInfo-pNext-02387
            .format = transformVkFormat(metadata.format, /*sRGB=*/false),
            .extent = {
                metadata.width,
                metadata.height,
                1u,
            },
            .mipLevels = 1,
            .arrayLayers = metadata.layers,
            .samples = metadata.samples,
            .usage = metadata.usage,
        };
        VkImage image;
        VkResult result = vkCreateImage(device, &imageInfo, VKALLOC, &image);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkCreateImage failed with error=" << static_cast<int32_t>(result);
        return image;
    };

    auto allocMem = [&](VkImage image, ExternalImageMetadata const& metadata) {
        bool const isExternal = metadata.externalFormat != 0;
        // Allocate the memory
        VkImportAndroidHardwareBufferInfoANDROID const androidHardwareBufferInfo = {
            .sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID,
            .buffer = buffer,
        };
        VkMemoryDedicatedAllocateInfo const memoryDedicatedAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
            .pNext = &androidHardwareBufferInfo,
            .image = image,
            .buffer = VK_NULL_HANDLE,
        };
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
        VkMemoryPropertyFlags requiredMemoryFlags =
                !isExternal && any(metadata.filamentUsage & TextureUsage::UPLOADABLE)
                        ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                        : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        if (any(metadata.filamentUsage & TextureUsage::PROTECTED)) {
            requiredMemoryFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
        }

        uint32_t const memoryTypeIndex = VulkanContext::selectMemoryType(memoryProperties,
                metadata.memoryTypeBits, requiredMemoryFlags);

        VkMemoryAllocateInfo const allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = &memoryDedicatedAllocateInfo,
            .allocationSize = metadata.allocationSize,
            .memoryTypeIndex = memoryTypeIndex,
        };
        VkDeviceMemory memory;
        VkResult result = vkAllocateMemory(device, &allocInfo, VKALLOC, &memory);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                << "vkAllocateMemory failed with error=" << static_cast<int32_t>(result);
        result = vkBindImageMemory(getDevice(), image, memory, 0);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                << "vkBindImageMemory error=" << static_cast<int32_t>(result);
        return memory;
    };

    VulkanPlatform::ImageData::Bundle internal = {}, external = {};
    auto img = buildImage(metadata);
    auto mem = allocMem(img, metadata);

    // Note that we're always choosing a non-externally sampled format if it exists.
    if (metadata.externalFormat == 0) {
        internal = { img, mem };
    } else {
        external = { img, mem };
    }

    return {
        .internal = internal,
        .external = external,
    };
}

VulkanPlatform::ExtensionSet VulkanPlatformAndroid::getSwapchainInstanceExtensions() const {
    return {
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
    };
}

VulkanPlatform::SurfaceBundle VulkanPlatformAndroid::createVkSurfaceKHR(void* nativeWindow,
        VkInstance instance, uint64_t flags) const noexcept {
    VkSurfaceKHR surface;
    VkExtent2D extent;

    VkAndroidSurfaceCreateInfoKHR const createInfo = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .window = (ANativeWindow*) nativeWindow,
    };
    VkResult const result =
            vkCreateAndroidSurfaceKHR(instance, &createInfo, VKALLOC, (VkSurfaceKHR*) &surface);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkCreateAndroidSurfaceKHR with error=" << static_cast<int32_t>(result);
    return { surface, extent };
}

// Deprecated platform dependent helper methods
VulkanPlatform::ExtensionSet VulkanPlatform::getSwapchainInstanceExtensionsImpl() { return {}; }

VulkanPlatform::SurfaceBundle VulkanPlatform::createVkSurfaceKHRImpl(void* nativeWindow,
        VkInstance instance, uint64_t flags) noexcept {
    return SurfaceBundle{};
}

} // namespace filament::backend
