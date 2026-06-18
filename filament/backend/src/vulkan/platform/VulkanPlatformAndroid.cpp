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

#include "AndroidNativeWindow.h"

#include "vulkan/platform/VulkanPlatformSwapChainImpl.h"
#include "vulkan/utils/Image.h"
#include "vulkan/VulkanConstants.h"
#include "vulkan/VulkanContext.h"

#include <private/backend/BackendUtilsAndroid.h>

#include <backend/DriverEnums.h>
#include <backend/platforms/VulkanPlatformAndroid.h>

#include <bluevk/BlueVK.h>

#include <utils/compiler.h>
#include <utils/Panic.h>

#include <android/api-level.h>
#include <android/hardware_buffer.h>
#include <android/native_window.h>

#include <new>
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

bool isFormatSrgb(VkFormat format) {
  return format == VK_FORMAT_R8G8B8A8_SRGB || format == VK_FORMAT_R8G8B8_SRGB;
}

#ifndef AHARDWAREBUFFER_FORMAT_YV12
#define AHARDWAREBUFFER_FORMAT_YV12 0x32315659 //842094169
#endif
bool isSoftwareDecodedYUV(uint32_t format, uint64_t usage){
    bool const isYV12 = (format == AHARDWAREBUFFER_FORMAT_YV12);
    bool const isFlexYUV = (format == AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420);
    bool const hasCpuWrite = (usage & AHARDWAREBUFFER_USAGE_CPU_WRITE_MASK) != 0;
    return (isYV12||isFlexYUV) && hasCpuWrite;
}

bool isProtectedFromUsage(uint64_t usage) {
    return usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
}

VkImageUsageFlags getVkUsage(const AHardwareBuffer_Desc& desc, VkFormat format, bool sRGB) {
    VkImageUsageFlags usage = 0;
    bool isDepthStencilFormat = fvkutils::isVkDepthFormat(format) || fvkutils::isVkStencilFormat(format);
    // The following only concern usage flags derived from Table 14.
    if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) {
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

        // We shouldn't be using external samplers as input attachments
        // usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) {
        if (isDepthStencilFormat) {
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }
    if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER) {
        usage = VK_IMAGE_USAGE_STORAGE_BIT;
    }

    return usage;
}

std::pair<TextureFormat, TextureUsage> getFilamentFormatAndUsage(const AHardwareBuffer_Desc& desc,
        bool sRGB) {
    auto const format = mapToFilamentFormat(desc.format, sRGB);
    return {
        format,
        mapToFilamentUsage(desc.usage, format),
    };
}

uint32_t selectMemoryTypeForExternalImage(VkPhysicalDevice physicalDevice, VkDevice device,
        VkImage image, uint32_t types, VkFlags requiredMemoryFlags) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    uint32_t const memoryTypeIndex =
            VulkanContext::selectMemoryType(memoryProperties, types, requiredMemoryFlags);

    if constexpr (FVK_RENDERDOC_CAPTURE_MODE) {
        // RenderDoc will replay external resources as non-external.
        // Adjust properties that will trip it up when replaying, even though these are not valid.
        // Update memory type index if necessary so that we can replay the capture.

        VkMemoryRequirements imageMemoryRequirements;
        vkGetImageMemoryRequirements(device, image, &imageMemoryRequirements);

        uint32_t const imageMemoryTypeBits = imageMemoryRequirements.memoryTypeBits;
        bool const isMemoryTypeSupported = ((1 << memoryTypeIndex) & imageMemoryTypeBits) != 0;
        if (isMemoryTypeSupported) {
            return memoryTypeIndex;
        }

        // Current memory type will not be replayable by RenderDoc.
        // Attempt to change the memory type index
        VkMemoryPropertyFlags const kRenderDocFallBackReqs = 0;
        uint32_t commonMemoryTypeBits = types & imageMemoryTypeBits;
        uint32_t commonTypeIndex = VulkanContext::selectMemoryType(memoryProperties,
                commonMemoryTypeBits, kRenderDocFallBackReqs);
        if (commonMemoryTypeBits && commonTypeIndex != VK_MAX_MEMORY_TYPES) {
            return commonTypeIndex;
        }
        uint32_t imageTypeIndex = VulkanContext::selectMemoryType(memoryProperties,
                imageMemoryTypeBits, kRenderDocFallBackReqs);
        if (imageTypeIndex != VK_MAX_MEMORY_TYPES) {
            return imageTypeIndex;
        }
    }

    return memoryTypeIndex;
}

} // namespace

// ---------------------------------------------------------------------------------------------

VulkanPlatformAndroid::ExternalImageVulkanAndroid::~ExternalImageVulkanAndroid() {
    if (__builtin_available(android 26, *)) {
        if (aHardwareBuffer) {
            AHardwareBuffer_release(aHardwareBuffer);
        }
    }
}

// ---------------------------------------------------------------------------------------------

VulkanPlatformAndroid::VulkanPlatformAndroid() {
    mOSVersion = android_get_device_api_level();
    if (mOSVersion < 0) {
        mOSVersion = __ANDROID_API_FUTURE__;
    }
}

VulkanPlatformAndroid::~VulkanPlatformAndroid() noexcept {
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

bool VulkanPlatformAndroid::copyExternalImageToMemoryYUV(
        ExternalImageHandleRef image, void* dstData, uint32_t w, uint32_t h) const {

    auto const* fvkExternalImage = static_cast<ExternalImageVulkanAndroid const*>(image.get());
    AHardwareBuffer* buffer = fvkExternalImage->aHardwareBuffer;
    if (!buffer) return false;

    uint32_t const yPlaneSize = w * h;
    uint32_t const chromaWidth = w / 2;
    uint32_t const chromaHeight = h / 2;

    uint8_t* dstBytes = static_cast<uint8_t*>(dstData);
    uint8_t* yDst = dstBytes;
    uint8_t* uvDst = dstBytes + yPlaneSize;

    //https://developer.android.com/ndk/reference/group/a-hardware-buffer#ahardwarebuffer_lockplanes:
    // YUV formats are always represented by three separate planes of data, one for each color plane.
    // The order of planes in the array is guaranteed such that plane #0 is always Y,
    // plane #1 is always U (Cb), and plane #2 is always V (Cr).
    // All other formats are represented by a single plane.
    if (__builtin_available(android 29, *)) {
        AHardwareBuffer_Planes planes;
        if (AHardwareBuffer_lockPlanes(buffer, AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN, -1, nullptr, &planes) == 0) {

            // Copy luminance
            uint8_t* ySrc = static_cast<uint8_t*>(planes.planes[0].data);
            uint32_t yRowStride = planes.planes[0].rowStride;
            for (uint32_t row = 0; row < h; ++row) {
                memcpy(yDst + (row * w), ySrc + (row * yRowStride), w);
            }

            // Encode packed Cb/Cr based on NV12 spec
            uint8_t* uSrc = static_cast<uint8_t*>(planes.planes[1].data);
            uint32_t uRowStride = planes.planes[1].rowStride;
            uint32_t uPixelStride = planes.planes[1].pixelStride;

            uint8_t* vSrc = static_cast<uint8_t*>(planes.planes[2].data);
            uint32_t vRowStride = planes.planes[2].rowStride;
            uint32_t vPixelStride = planes.planes[2].pixelStride;

            for (uint32_t row = 0; row < chromaHeight; ++row) {
                uint8_t* uvRowDst = uvDst + (row * w);
                uint8_t* uRowSrc = uSrc + (row * uRowStride);
                uint8_t* vRowSrc = vSrc + (row * vRowStride);

                for (uint32_t col = 0; col < chromaWidth; ++col) {
                    uvRowDst[col * 2 + 0] = uRowSrc[col * uPixelStride];
                    uvRowDst[col * 2 + 1] = vRowSrc[col * vPixelStride];
                }
            }
            AHardwareBuffer_unlock(buffer, nullptr);
            return true;
        }
        LOG(ERROR) << "[VulkanPlatformAndroid::copyExternalImageToMemoryYUV] AHardwareBuffer_lockPlanes failed";
    } else {
        LOG(ERROR) << "no fallback path in copyExternalImageToMemoryYUV [Android 26]";
    }
    return false;
}

VulkanPlatform::ExternalImageMetadata VulkanPlatformAndroid::extractExternalImageMetadata(
    ExternalImageHandleRef image) const {
    if (__builtin_available(android 26, *)) {
        auto const *fvkExternalImage = static_cast<ExternalImageVulkanAndroid const *>(image.get());
        AHardwareBuffer *buffer = fvkExternalImage->aHardwareBuffer;
        ExternalImageMetadata metadata = {};

        AHardwareBuffer_Desc bufferDesc;
        AHardwareBuffer_describe(buffer, &bufferDesc);

        metadata.width = bufferDesc.width;
        metadata.height = bufferDesc.height;
        metadata.layers = bufferDesc.layers;
        metadata.samples = VK_SAMPLE_COUNT_1_BIT;
        metadata.isStagingRequired = isSoftwareDecodedYUV(bufferDesc.format, bufferDesc.usage);

        // Get the VkFormat directly from the driver.
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
        metadata.format = bufferPropertiesFormat;
        metadata.usage = getVkUsage(bufferDesc, metadata.format, fvkExternalImage->sRGB);

        // TODO: Right now, if we get a format that isn't explicitly supported by Filament, the
        // reverse mapping to a Filament format may be empty. We can fix this by deriving the format
        // from the VkFormat directly, but for now this should be ok.
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

        metadata.allocationSize = properties.allocationSize;
        metadata.memoryTypeBits = properties.memoryTypeBits;

        metadata.isChromaConversionRequired =
            metadata.format == VK_FORMAT_UNDEFINED ||
            fvkutils::isVKYcbcrConversionFormat(metadata.format);

        // Note: It is unclear how the driver are getting this information as in the use case
        // we are looking into for this YV12 the Qualcomm driver isn't involved in the decoding
        // of the data. My assumption is that it is looking at the gralloc/AHB* metadata.
        metadata.ycbcrModel = formatInfo.suggestedYcbcrModel;
        metadata.ycbcrRange = formatInfo.suggestedYcbcrRange;
        metadata.ycbcrConversionComponents = formatInfo.samplerYcbcrConversionComponents;
        metadata.xChromaOffset = formatInfo.suggestedXChromaOffset;
        metadata.yChromaOffset = formatInfo.suggestedYChromaOffset;

        // Although both HW and SW require YUV --> RGB conversion, the sw path relies on
        // standards ones. The hardware path is more platform specific.
        if (metadata.isStagingRequired) {

            // We use  metadata.usage when we crate the final image in the staged path
            // so we do need to encode the VK_IMAGE_USAGE_TRANSFER_DST_BIT on top of usual SAMPLED_BIT
            metadata.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
            metadata.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            metadata.externalFormat = 0;
        } else if (metadata.isChromaConversionRequired) {

            // In the YUV hardware path we do not require the staging data so no T_DST
            // also VkAndroidHardwareBufferFormatPropertiesANDROID supplies most of the conversion
            // details.
            metadata.format = VK_FORMAT_UNDEFINED;
            metadata.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            metadata.externalFormat = formatInfo.externalFormat;
        } else {

            // Non YUV path
            metadata.externalFormat = 0;
        }

        return metadata;
    }
    else {
        LOG(ERROR) << "no fallback path in extractExternalImageMetadata [Android 26]";
        return {};
    }
}

VulkanPlatform::ImageData VulkanPlatformAndroid::createVkImageFromExternal(
        ExternalImageHandleRef externalImage,
        uint32_t logicalWidth, uint32_t logicalHeight) const {
    auto metadata = extractExternalImageMetadata(externalImage);
    if(metadata.isStagingRequired) {
        // Note: This might be contencious since we need to know who we can trust,
        // is it the ExternalImageHandleRef or the caller to the Filament API?
        // What is clear is that in the software decoded path, on Android, the
        // ExternalImageHandleRef advertises the padded width allocation.
        metadata.width = logicalWidth;
        metadata.height = logicalHeight;
    }

    auto const* fvkExternalImage =
            static_cast<ExternalImageVulkanAndroid const*>(externalImage.get());
    AHardwareBuffer* buffer = fvkExternalImage->aHardwareBuffer;

    VkDevice const device = getDevice();
    VkPhysicalDevice const physicalDevice = getPhysicalDevice();
    auto buildImage = [&](ExternalImageMetadata const& metadata) {
        bool const isExternalFormat = metadata.isChromaConversionRequired &&
            !metadata.isStagingRequired;
        VkExternalFormatANDROID const externalFormat = {
            .sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
            .pNext = nullptr,
            .externalFormat = metadata.externalFormat,
        };
        VkExternalMemoryImageCreateInfo externalCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
            .pNext = isExternalFormat ? &externalFormat : nullptr,
            .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
        };

        VkFormat formats[2] = {};
        VkImageFormatListCreateInfo imageFormatListInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO,
            .pNext = nullptr,
            .viewFormatCount = 2,
            .pViewFormats = formats,
        };

        if (isFormatSrgb(metadata.format)) {
            formats[0] = metadata.format;
            formats[1] = transformVkFormat(metadata.format, /*sRGB=*/false);
            imageFormatListInfo.pNext = externalCreateInfo.pNext;
            externalCreateInfo.pNext = &imageFormatListInfo;
        }

        VkImageCreateFlags imageFlags =
                (isFormatSrgb(metadata.format) ? VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT : 0u) |
                (any(metadata.filamentUsage & TextureUsage::PROTECTED)
                                ? VK_IMAGE_CREATE_PROTECTED_BIT
                                : 0u);
        VkImageCreateInfo const imageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = &externalCreateInfo,
            .flags = imageFlags,
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
    bool const isExternalFormat = metadata.isChromaConversionRequired &&
            !metadata.isStagingRequired;
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

        VkMemoryPropertyFlags requiredMemoryFlags =
                !isExternalFormat && any(metadata.filamentUsage & TextureUsage::UPLOADABLE)
                        ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                        : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        if (any(metadata.filamentUsage & TextureUsage::PROTECTED)) {
            requiredMemoryFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
        }

        uint32_t const memoryTypeIndex = selectMemoryTypeForExternalImage(physicalDevice, device,
                image, metadata.memoryTypeBits, requiredMemoryFlags);

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
        result = vkBindImageMemory(device, image, memory, 0);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkBindImageMemory error=" << static_cast<int32_t>(result);

        return memory;
    };

    // Software Decoded (Staging) Path Lambdas (following desing pattern from buildImage)
    auto buildStagingBuffer = [&](uint32_t size) {
        // Staging buffer used as a source for the buffer to image blit AND AHB* cpu copy
        VkBufferCreateInfo const bufferInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        VkBuffer stagingBuffer;
        VkResult result = vkCreateBuffer(device, &bufferInfo, VKALLOC, &stagingBuffer);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkCreateBuffer failed YUV staging with error=" << static_cast<int32_t>(result);
        return stagingBuffer;
    };

    auto allocStagingBufferMem = [&](VkBuffer stagingBuffer) {
        // Allocate standard Host-Visible memory for the buffer
        VkMemoryRequirements bufMemReqs;
        vkGetBufferMemoryRequirements(device, stagingBuffer, &bufMemReqs);
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
        uint32_t const bufMemTypeIndex =
                VulkanContext::selectMemoryType(memoryProperties, bufMemReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkMemoryAllocateInfo const bufAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = bufMemReqs.size,
            .memoryTypeIndex = bufMemTypeIndex,
        };
        VkDeviceMemory stagingMemory;
        // @TODO: When this CL gets submitted to github, we need to revisit this
        VkResult result = vkAllocateMemory(device, &bufAllocInfo, VKALLOC, &stagingMemory);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkAllocateMemory failed YUV staging with error=" << static_cast<int32_t>(result);
        result = vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkBindBufferMemory failed YUV staging with error=" << static_cast<int32_t>(result);
        return stagingMemory;
    };

    auto buildStagingImage = [&](ExternalImageMetadata const& meta) {
        // metadata.format, will be packed accordingly if YV12 (in extractExternalImageMetadata)
        VkImageCreateInfo const imageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = meta.format,
            .extent = { meta.width, meta.height, 1u },
            .mipLevels = 1,
            .arrayLayers = meta.layers,
            .samples = meta.samples,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = meta.usage, // Correctly contains DST | SAMPLED based on our metadata tweak
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        VkImage finalTextureImage;
        VkResult result = vkCreateImage(device, &imageInfo, VKALLOC, &finalTextureImage);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkCreateImage failed with error=" << static_cast<int32_t>(result);
        return finalTextureImage;
    };

    auto allocStagingImageMem = [&](VkImage image) {
        // The final image is a normal YUV sampled source extractExternalImageMetadata encoded the details
        VkMemoryRequirements imgMemReqs;
        vkGetImageMemoryRequirements(device, image, &imgMemReqs);
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
        uint32_t const imgMemTypeIndex =
                VulkanContext::selectMemoryType(memoryProperties, imgMemReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkMemoryAllocateInfo const imgAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = imgMemReqs.size,
            .memoryTypeIndex = imgMemTypeIndex,
        };
        VkDeviceMemory finalTextureMem;
        VkResult result = vkAllocateMemory(device, &imgAllocInfo, VKALLOC, &finalTextureMem);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkAllocateMemory failed YUV staging with error=" << static_cast<int32_t>(result);
        result = vkBindImageMemory(device, image, finalTextureMem, 0);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkBindImageMemory failed YUV staging with error=" << static_cast<int32_t>(result);
        return finalTextureMem;
    };

    VulkanPlatform::ImageData::Bundle internal = {}, external = {};
    if (metadata.isStagingRequired) {
        uint32_t const w = metadata.width;
        uint32_t const h = metadata.height;
        uint32_t const yPlaneSize = w * h;
        uint32_t const chromaPlaneSize = (w / 2) * (h / 2);
        uint32_t const totalBufferSize = yPlaneSize + (chromaPlaneSize * 2);

        // Build and pack the results
        internal.stagingBuffer = buildStagingBuffer(totalBufferSize);
        internal.stagingMemory = allocStagingBufferMem(internal.stagingBuffer);
        
        internal.image = buildStagingImage(metadata);
        internal.memory = allocStagingImageMem(internal.image);

    } else {
        // Fall in this case for hardware decoded frames
        auto img = buildImage(metadata);
        auto mem = allocMem(img, metadata);

        if (metadata.externalFormat == 0) {
            internal.image = img;
            internal.memory = mem;
        } else {
            external.image = img;
            external.memory = mem;
        }
    }

    return {
        .internal = internal,
        .external = external,
    };
}

bool VulkanPlatformAndroid::convertSyncToFd(Platform::Sync* sync, int* fd) const noexcept {
    assert_invariant(sync && fd);

    VulkanSync& vulkanSync = static_cast<VulkanSync&>(*sync);
    assert_invariant(vulkanSync.fenceStatus);

    const auto fence = vulkanSync.fenceStatus->getVkFence();
    if (fence == VK_NULL_HANDLE) {
        *fd = -1;
        return true;
    }

    VkFenceGetFdInfoKHR getFdInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR,
        .fence = fence,
        .handleType = getFenceExportFlags(),
    };
    VkResult res = vkGetFenceFdKHR(getDevice(), &getFdInfo, fd);
    if (res != VK_SUCCESS) {
        LOG(ERROR) << "Failed to convert sync to fd: " << res;
        return false;
    }
    return true;
}

VkExternalFenceHandleTypeFlagBits VulkanPlatformAndroid::getFenceExportFlags() const noexcept {
    // On android, make fences that can be exported to fd's.
    return VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;
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

int VulkanPlatformAndroid::getOSVersion() const noexcept {
    return mOSVersion;
}

void VulkanPlatformAndroid::terminate() {
    VulkanPlatform::terminate();
}

Driver* VulkanPlatformAndroid::createDriver(void* sharedContext, DriverConfig const& driverConfig) {
    Driver* driver = VulkanPlatform::createDriver(sharedContext, driverConfig);
    return driver;
}

bool VulkanPlatformAndroid::isCompositorTimingSupported() const noexcept {
    return true;
}

bool VulkanPlatformAndroid::queryCompositorTiming(SwapChain const* swapchain,
        CompositorTiming* outCompositorTiming) const noexcept {
    if (!swapchain) {
        return false;
    }

    outCompositorTiming->compositeInterval = CompositorTiming::INVALID;
    outCompositorTiming->compositeToPresentLatency = CompositorTiming::INVALID;

    // From this point on, we always return "success" because some timings were returned.

    CompositorTiming result{};
    auto vulkanSwapchain = static_cast<VulkanPlatformSwapChainBase const *>(swapchain);
    vulkanSwapchain->queryCompositorTiming(&result);
    outCompositorTiming->compositeInterval = result.compositeInterval;
    outCompositorTiming->compositeToPresentLatency = result.compositeToPresentLatency;
    return true;
}

bool VulkanPlatformAndroid::setPresentFrameId(SwapChain const* swapchain, uint64_t frameId) noexcept {
    auto vulkanSwapchain = static_cast<VulkanPlatformSwapChainBase const *>(swapchain);
    return vulkanSwapchain->setPresentFrameId(frameId);
}

bool VulkanPlatformAndroid::queryFrameTimestamps(SwapChain const* swapchain, uint64_t frameId,
        FrameTimestamps* outFrameTimestamps) const noexcept {
    auto vulkanSwapchain = static_cast<VulkanPlatformSwapChainBase const *>(swapchain);
    return vulkanSwapchain->queryFrameTimestamps(frameId, outFrameTimestamps);
}

utils::tribool VulkanPlatformAndroid::isFrameRateChangeSupported(void* const nativeWindow) const noexcept {
    return NativeWindow::isFrameRateChangeSupported(static_cast<ANativeWindow*>(nativeWindow));
}

int VulkanPlatformAndroid::setFrameRate(SwapChain const* swapchain, float frameRate,
        FrameRateCompatibility compatibility, ChangeFrameRateStrategy strategy) noexcept {
    auto vulkanSwapchain = static_cast<VulkanPlatformSwapChainBase const*>(swapchain);
    return vulkanSwapchain ? vulkanSwapchain->setFrameRate(frameRate, compatibility, strategy) : -1;
}

} // namespace filament::backend

