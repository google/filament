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

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

#include <android/hardware_buffer.h>
#include <android/native_window.h>

using namespace bluevk;

namespace filament::backend {
    void GetVKFormatAndUsage(const AHardwareBuffer_Desc& desc,
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

#if 0
uint32_t getExternalImageMemoryBits(void* externalBuffer,
        VkDevice device) {
    AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalBuffer);
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

    return properties.memoryTypeBits;
}
#endif

void describeExternalImage(void* externalBuffer, VkDevice device,
        VulkanPlatform::ExternalImageMetadata& metadata) {
    AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalBuffer);
    AHardwareBuffer_Desc buffer_desc;
    AHardwareBuffer_describe(buffer, &buffer_desc);
    metadata.width = buffer_desc.width;
    metadata.height = buffer_desc.height;

    GetVKFormatAndUsage(buffer_desc, metadata.format, metadata.usage,
        metadata.isProtected);
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
    metadata.externalFormat = format_info.externalFormat
    metadata.allocationSize = properties.allocationSize;
    metadata.memoryTypeBits = properties.memoryTypeBits;
}

VkImage createExternalImage(void* externalBuffer, VkDevice device,
        VulkanPlatform::ExternalImageMetadata& metadata, VkDeviceMemory& memory) {
    VkImage image;
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
    imageInfo.arrayLayers = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.usage = metadata.usage;
    // In the unprotected case add R/W capabilities
    if (isProtected == false)
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    VkResult const result =
        vkCreateImage(device, &imageInfo, allocator, &image);
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
        .image = image,
        .buffer = VK_NULL_HANDLE,
    };
    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &memory_dedicated_allocate_info,
        .allocationSize = metadata.allocationSize,
        .memoryTypeIndex = metadata.memoryTypeBits };
    VkResult const result =
        vkAllocateMemory(device, &alloc_info, allocator, &memory);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS);
}
#if 0
void createExternalImage(void* externalBuffer, VkDevice device,
        const VkAllocationCallbacks* allocator, VkImage& pImage,
        uint32_t& width, uint32_t& height, VkFormat& format, bool& isProtected) {
    AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalBuffer);
    AHardwareBuffer_Desc buffer_desc;
    AHardwareBuffer_describe(buffer, &buffer_desc);

    VkImageUsageFlags usage;
    //technically we don't need the format (since whe will query it in the following APIs
    //directly from VK). But we still need to check the format to differenciate DS from Color
    GetVKFormatAndUsage(buffer_desc, format, usage, isProtected);

    // All this work now is for external formats (query the underlying VK for the format)
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

    //if external format we need to specifiy it in the allocation
    const bool use_external_format = format_info.format == VK_FORMAT_UNDEFINED;

    const VkExternalFormatANDROID external_format = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
        .pNext = nullptr,
        .externalFormat = format_info.externalFormat,//pass down the format (external means we don't have it VK defined)
    };
    const VkExternalMemoryImageCreateInfo external_create_info = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
        .pNext = use_external_format ? &external_format : nullptr,
        .handleTypes =
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
    };

    VkImageCreateInfo imageInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.pNext = &external_create_info;
    imageInfo.format = format_info.format;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent =
    {
        buffer_desc.width,
        buffer_desc.height,
        1u,
    };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.arrayLayers = buffer_desc.layers;
    imageInfo.usage = usage;
    // In the unprotected case add R/W capabilities
    if (isProtected == false)
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    VkResult const result =
        vkCreateImage(device, &imageInfo, allocator, &pImage);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS);

    width = buffer_desc.width;
    height = buffer_desc.height;
    format = format_info.format;
}
void allocateExternalImage(void* externalBuffer, VkDevice device,
        const VkAllocationCallbacks* allocator, VkImage pImage, VkDeviceMemory& pMemory) {
    uint32_t memoryTypeIndex = getExternalImageMemoryBits(externalBuffer, device);
    AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalBuffer);
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

    // Now handle the allocation
    VkImportAndroidHardwareBufferInfoANDROID android_hardware_buffer_info = {
        .sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID,
        .pNext = nullptr,
        .buffer = buffer,
    };
    VkMemoryDedicatedAllocateInfo memory_dedicated_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
        .pNext = &android_hardware_buffer_info,
        .image = pImage,
        .buffer = VK_NULL_HANDLE,
    };
    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &memory_dedicated_allocate_info,
        .allocationSize = properties.allocationSize,
        .memoryTypeIndex = memoryTypeIndex };
    VkResult const result =
        vkAllocateMemory(device, &alloc_info, allocator, &pMemory);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS);
}
#endif
}
