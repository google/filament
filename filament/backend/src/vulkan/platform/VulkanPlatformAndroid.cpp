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

#include <bluevk/BlueVK.h>

#include <android/hardware_buffer.h>
#include <android/native_window.h>

using namespace bluevk;

namespace {
void getVKFormatAndUsage(const AHardwareBuffer_Desc& desc,
        VkFormat& format,
        VkImageUsageFlags& usage,
        bool& isProtected) {
    // Refer to "11.2.17. External Memory Handle Types" in the spec, and
    // Tables 13/14 for how the following derivation works.
    bool is_depth_format = false;
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
        is_depth_format = true;
        format = VK_FORMAT_D16_UNORM;
        break;
    case AHARDWAREBUFFER_FORMAT_D24_UNORM:
        is_depth_format = true;
        format = VK_FORMAT_X8_D24_UNORM_PACK32;
        break;
    case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
        is_depth_format = true;
        format = VK_FORMAT_D24_UNORM_S8_UINT;
        break;
    case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
        is_depth_format = true;
        format = VK_FORMAT_D32_SFLOAT;
        break;
    case AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
        is_depth_format = true;
        format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        break;
    case AHARDWAREBUFFER_FORMAT_S8_UINT:
        is_depth_format = true;
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
        if (is_depth_format) {
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        else {
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

filament::backend::VulkanPlatform::imageData allocateExternalImage(void* externalBuffer,
        VkDevice device, const VkAllocationCallbacks* allocator,
        const VulkanPlatform::ExternalImageMetadata& metadata) {
    filament::backend::VulkanPlatform::imageData data;
    AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalBuffer);

    //if external format we need to specifiy it in the allocation
    const bool use_external_format = metadata.format == VK_FORMAT_UNDEFINED;

    const VkExternalFormatANDROID external_format = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
        .pNext = nullptr,
        .externalFormat = metadata.externalFormat,//pass down the format (external means we don't have it VK defined)
    };
    const VkExternalMemoryImageCreateInfo external_create_info = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
        .pNext = use_external_format ? &external_format : nullptr,
        .handleTypes =
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
    };

    VkImageCreateInfo imageInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.pNext = &external_create_info;
    imageInfo.format = metadata.format;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent =
    {
        metadata.width,
        metadata.height,
        1u,
    };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = metadata.layers;
    imageInfo.usage = metadata.usage;
    // In the unprotected case add R/W capabilities
    if (metadata.isProtected == false)
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    VkResult const result =
        vkCreateImage(device, &imageInfo, allocator, &data.first);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS);

    // Allocate the memory
    VkImportAndroidHardwareBufferInfoANDROID android_hardware_buffer_info = {
        .sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID,
        .pNext = nullptr,
        .buffer = buffer,
    };
    VkMemoryDedicatedAllocateInfo memory_dedicated_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
        .pNext = &android_hardware_buffer_info,
        .image = data.first,
        .buffer = VK_NULL_HANDLE,
    };
    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &memory_dedicated_allocate_info,
        .allocationSize = metadata.allocationSize,
        .memoryTypeIndex = metadata.memoryTypeBits };
    VkResult const result_alloc =
        vkAllocateMemory(device, &alloc_info, allocator, &data.second);
    FILAMENT_CHECK_POSTCONDITION(result_alloc == VK_SUCCESS);

    return data;
}

filament::backend::VulkanPlatform::ExternalImageMetadata 
        VulkanPlatform::getExternalImageMetadataImpl(void* externalImage,
        VkDevice device) {
    filament::backend::VulkanPlatform::ExternalImageMetadata metadata;
    AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalImage);
    if (__builtin_available(android 26, *)) {
        AHardwareBuffer_Desc buffer_desc;
        AHardwareBuffer_describe(buffer, &buffer_desc);
        metadata.width = buffer_desc.width;
        metadata.height = buffer_desc.height;
        metadata.layers = buffer_desc.layers;

        getVKFormatAndUsage(buffer_desc, metadata.format, metadata.usage,
            metadata.isProtected);
    }
    // In the unprotected case add R/W capabilities
    if (metadata.isProtected == false) {
        metadata.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    VkAndroidHardwareBufferFormatPropertiesANDROID format_info = {
        .sType =
            VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID,
        .pNext = nullptr,
    };
    VkAndroidHardwareBufferPropertiesANDROID properties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
        .pNext = &format_info,
    };
    VkResult prop_res = vkGetAndroidHardwareBufferPropertiesANDROID(device, buffer,
        &properties);
    FILAMENT_CHECK_POSTCONDITION(prop_res == VK_SUCCESS);

    FILAMENT_CHECK_POSTCONDITION(metadata.format == format_info.format);
    metadata.externalFormat = format_info.externalFormat;
    metadata.allocationSize = properties.allocationSize;
    metadata.memoryTypeBits = properties.memoryTypeBits;
    return metadata;
}

VulkanPlatform::imageData VulkanPlatform::createExternalImageImpl(void* externalImage, VkDevice device,
        const VkAllocationCallbacks* allocator, const ExternalImageMetadata& metadata) {
    imageData data = allocateExternalImage(externalImage, device, allocator, metadata);
    VkResult result = vkBindImageMemory(device, data.first, data.second, 0);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "vkBindImageMemory error="
        << result << ".";
    return data;
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
            .window = (ANativeWindow*)nativeWindow,
    };
    VkResult const result = vkCreateAndroidSurfaceKHR(instance, &createInfo, VKALLOC,
        (VkSurfaceKHR*)&surface);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "vkCreateAndroidSurfaceKHR error.";
    return std::make_tuple(surface, extent);
}
}
