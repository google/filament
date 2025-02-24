/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "external_memory_sync.h"
#include "error_monitor.h"

#include <vulkan/utility/vk_struct_helper.hpp>
#include <generated/enum_flag_bits.h>

#include "utils/vk_layer_utils.h"

// We are trying to query unsupported handle types, which means we will likley trigger *-handleType-parameter VUs
void IgnoreHandleTypeError(ErrorMonitor *monitor) {
    monitor->SetAllowedFailureMsg("VUID-VkFenceGetFdInfoKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkFenceGetWin32HandleInfoKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkImportFenceFdInfoKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkImportMemoryFdInfoKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkImportMemoryHostPointerInfoEXT-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkImportMemoryWin32HandleInfoKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkImportSemaphoreFdInfoKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkMemoryGetFdInfoKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkMemoryGetWin32HandleInfoKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkSemaphoreGetFdInfoKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-vkGetMemoryFdPropertiesKHR-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-vkGetMemoryHostPointerPropertiesEXT-handleType-parameter");
    monitor->SetAllowedFailureMsg("VUID-vkGetMemoryWin32HandlePropertiesKHR-handleType-parameter");

    monitor->SetAllowedFailureMsg("VUID-VkExportFenceCreateInfo-handleTypes-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkExportMemoryAllocateInfo-handleTypes-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkExportSemaphoreCreateInfo-handleTypes-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkExternalMemoryBufferCreateInfo-handleTypes-parameter");
    monitor->SetAllowedFailureMsg("VUID-VkExternalMemoryImageCreateInfo-handleTypes-parameter");
}

VkExternalMemoryHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu, const VkBufferCreateInfo &buffer_create_info,
                                                         VkExternalMemoryHandleTypeFlagBits handle_type) {
    VkPhysicalDeviceExternalBufferInfo external_info = vku::InitStructHelper();
    external_info.flags = buffer_create_info.flags;
    external_info.usage = buffer_create_info.usage;
    external_info.handleType = handle_type;
    VkExternalBufferProperties external_buffer_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalBufferProperties(gpu, &external_info, &external_buffer_properties);
    return external_buffer_properties.externalMemoryProperties.compatibleHandleTypes;
}

VkExternalMemoryHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu, const VkImageCreateInfo &image_create_info,
                                                         VkExternalMemoryHandleTypeFlagBits handle_type) {
    VkPhysicalDeviceExternalImageFormatInfo external_info = vku::InitStructHelper();
    external_info.handleType = handle_type;
    VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_info);
    image_info.format = image_create_info.format;
    image_info.type = image_create_info.imageType;
    image_info.tiling = image_create_info.tiling;
    image_info.usage = image_create_info.usage;
    image_info.flags = image_create_info.flags;
    VkExternalImageFormatProperties external_properties = vku::InitStructHelper();
    VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_properties);
    if (vk::GetPhysicalDeviceImageFormatProperties2(gpu, &image_info, &image_properties) != VK_SUCCESS) return 0;
    return external_properties.externalMemoryProperties.compatibleHandleTypes;
}

VkExternalFenceHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu, VkExternalFenceHandleTypeFlagBits handle_type) {
    VkPhysicalDeviceExternalFenceInfo external_info = vku::InitStructHelper();
    external_info.handleType = handle_type;
    VkExternalFenceProperties external_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFenceProperties(gpu, &external_info, &external_properties);
    return external_properties.compatibleHandleTypes;
}

VkExternalSemaphoreHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu,
                                                            VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    VkPhysicalDeviceExternalSemaphoreInfo external_info = vku::InitStructHelper();
    external_info.handleType = handle_type;
    VkExternalSemaphoreProperties external_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalSemaphoreProperties(gpu, &external_info, &external_properties);
    return external_properties.compatibleHandleTypes;
}

VkExternalFenceHandleTypeFlags FindSupportedExternalFenceHandleTypes(VkPhysicalDevice gpu,
                                                                     VkExternalFenceFeatureFlags requested_features) {
    VkExternalFenceHandleTypeFlags supported_types = 0;
    IterateFlags<VkExternalFenceHandleTypeFlagBits>(
        AllVkExternalFenceHandleTypeFlagBits, [&](VkExternalFenceHandleTypeFlagBits flag) {
            VkPhysicalDeviceExternalFenceInfo external_info = vku::InitStructHelper();
            external_info.handleType = flag;
            VkExternalFenceProperties external_properties = vku::InitStructHelper();
            vk::GetPhysicalDeviceExternalFenceProperties(gpu, &external_info, &external_properties);
            if ((external_properties.externalFenceFeatures & requested_features) == requested_features) {
                supported_types |= flag;
            }
        });
    return supported_types;
}

VkExternalSemaphoreHandleTypeFlags FindSupportedExternalSemaphoreHandleTypes(VkPhysicalDevice gpu,
                                                                             VkExternalSemaphoreFeatureFlags requested_features) {
    VkExternalSemaphoreHandleTypeFlags supported_types = 0;
    IterateFlags<VkExternalSemaphoreHandleTypeFlagBits>(
        AllVkExternalSemaphoreHandleTypeFlagBits, [&](VkExternalSemaphoreHandleTypeFlagBits flag) {
            VkPhysicalDeviceExternalSemaphoreInfo external_info = vku::InitStructHelper();
            external_info.handleType = flag;
            VkExternalSemaphoreProperties external_properties = vku::InitStructHelper();
            vk::GetPhysicalDeviceExternalSemaphoreProperties(gpu, &external_info, &external_properties);
            if ((external_properties.externalSemaphoreFeatures & requested_features) == requested_features) {
                supported_types |= flag;
            }
        });
    return supported_types;
}

VkExternalMemoryHandleTypeFlags FindSupportedExternalMemoryHandleTypes(VkPhysicalDevice gpu,
                                                                       const VkBufferCreateInfo &buffer_create_info,
                                                                       VkExternalMemoryFeatureFlags requested_features) {
    VkPhysicalDeviceExternalBufferInfo external_info = vku::InitStructHelper();
    external_info.flags = buffer_create_info.flags;
    external_info.usage = buffer_create_info.usage;

    VkExternalMemoryHandleTypeFlags supported_types = 0;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(
        AllVkExternalMemoryHandleTypeFlagBits, [&](VkExternalMemoryHandleTypeFlagBits flag) {
            external_info.handleType = flag;
            VkExternalBufferProperties external_properties = vku::InitStructHelper();
            vk::GetPhysicalDeviceExternalBufferProperties(gpu, &external_info, &external_properties);
            const auto external_features = external_properties.externalMemoryProperties.externalMemoryFeatures;
            if ((external_features & requested_features) == requested_features) {
                supported_types |= flag;
            }
        });
    return supported_types;
}

VkExternalMemoryHandleTypeFlags FindSupportedExternalMemoryHandleTypes(VkPhysicalDevice gpu,
                                                                       const VkImageCreateInfo &image_create_info,
                                                                       VkExternalMemoryFeatureFlags requested_features) {
    VkPhysicalDeviceExternalImageFormatInfo external_info = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_info);
    image_info.format = image_create_info.format;
    image_info.type = image_create_info.imageType;
    image_info.tiling = image_create_info.tiling;
    image_info.usage = image_create_info.usage;
    image_info.flags = image_create_info.flags;

    VkExternalMemoryHandleTypeFlags supported_types = 0;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(
        AllVkExternalMemoryHandleTypeFlagBits, [&](VkExternalMemoryHandleTypeFlagBits flag) {
            external_info.handleType = flag;
            VkExternalImageFormatProperties external_properties = vku::InitStructHelper();
            VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_properties);
            VkResult result = vk::GetPhysicalDeviceImageFormatProperties2(gpu, &image_info, &image_properties);
            const auto external_features = external_properties.externalMemoryProperties.externalMemoryFeatures;
            if (result == VK_SUCCESS && (external_features & requested_features) == requested_features) {
                supported_types |= flag;
            }
        });
    return supported_types;
}

VkExternalMemoryHandleTypeFlagsNV FindSupportedExternalMemoryHandleTypesNV(VkPhysicalDevice gpu,
                                                                           const VkImageCreateInfo &image_create_info,
                                                                           VkExternalMemoryFeatureFlagsNV requested_features) {
    VkExternalMemoryHandleTypeFlagsNV supported_types = 0;
    IterateFlags<VkExternalMemoryHandleTypeFlagBitsNV>(
        AllVkExternalMemoryHandleTypeFlagBitsNV, [&](VkExternalMemoryHandleTypeFlagBitsNV flag) {
            VkExternalImageFormatPropertiesNV external_properties = {};
            VkResult result = vk::GetPhysicalDeviceExternalImageFormatPropertiesNV(
                gpu, image_create_info.format, image_create_info.imageType, image_create_info.tiling, image_create_info.usage,
                image_create_info.flags, flag, &external_properties);
            const auto external_features = external_properties.externalMemoryFeatures;
            if (result == VK_SUCCESS && (external_features & requested_features) == requested_features) {
                supported_types |= flag;
            }
        });
    return supported_types;
}

bool HandleTypeNeedsDedicatedAllocation(VkPhysicalDevice gpu, const VkBufferCreateInfo &buffer_create_info,
                                        VkExternalMemoryHandleTypeFlagBits handle_type) {
    VkPhysicalDeviceExternalBufferInfo external_info = vku::InitStructHelper();
    external_info.flags = buffer_create_info.flags;
    external_info.usage = buffer_create_info.usage;
    external_info.handleType = handle_type;

    VkExternalBufferProperties external_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalBufferProperties(gpu, &external_info, &external_properties);

    const auto external_features = external_properties.externalMemoryProperties.externalMemoryFeatures;
    return (external_features & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;
}

bool HandleTypeNeedsDedicatedAllocation(VkPhysicalDevice gpu, const VkImageCreateInfo &image_create_info,
                                        VkExternalMemoryHandleTypeFlagBits handle_type) {
    VkPhysicalDeviceExternalImageFormatInfo external_info = vku::InitStructHelper();
    external_info.handleType = handle_type;
    VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_info);
    image_info.format = image_create_info.format;
    image_info.type = image_create_info.imageType;
    image_info.tiling = image_create_info.tiling;
    image_info.usage = image_create_info.usage;
    image_info.flags = image_create_info.flags;

    VkExternalImageFormatProperties external_properties = vku::InitStructHelper();
    VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_properties);
    VkResult result = vk::GetPhysicalDeviceImageFormatProperties2(gpu, &image_info, &image_properties);
    if (result != VK_SUCCESS) return false;

    const auto external_features = external_properties.externalMemoryProperties.externalMemoryFeatures;
    return (external_features & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;
}

bool SemaphoreExportImportSupported(VkPhysicalDevice gpu, VkExternalSemaphoreHandleTypeFlagBits handle_type, void *p_next) {
    constexpr auto export_import_flags =
        VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT;

    VkPhysicalDeviceExternalSemaphoreInfo info = vku::InitStructHelper(p_next);
    info.handleType = handle_type;
    VkExternalSemaphoreProperties properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalSemaphoreProperties(gpu, &info, &properties);
    return (properties.externalSemaphoreFeatures & export_import_flags) == export_import_flags;
}