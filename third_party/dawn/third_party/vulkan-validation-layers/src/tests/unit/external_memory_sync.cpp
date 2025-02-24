/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "generated/enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/external_memory_sync.h"
#include "utils/vk_layer_utils.h"

class NegativeExternalMemorySync : public ExternalMemorySyncTest {};

TEST_F(NegativeExternalMemorySync, CreateBufferIncompatibleHandleTypes) {
    TEST_DESCRIPTION("Creating buffer with incompatible external memory handle types");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    // Try all flags first. It's unlikely all of them are compatible.
    VkExternalMemoryBufferCreateInfo external_memory_info = vku::InitStructHelper();
    external_memory_info.handleTypes = AllVkExternalMemoryHandleTypeFlagBits;
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&external_memory_info);
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-pNext-00920");

    IgnoreHandleTypeError(m_errorMonitor);
    // Get all exportable handle types supported by the platform.
    VkExternalMemoryHandleTypeFlags supported_handle_types = 0;
    VkExternalMemoryHandleTypeFlags any_compatible_group = 0;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(
        AllVkExternalMemoryHandleTypeFlagBits, [&](VkExternalMemoryHandleTypeFlagBits flag) {
            VkPhysicalDeviceExternalBufferInfo external_buffer_info = vku::InitStructHelper();
            external_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            external_buffer_info.handleType = flag;
            VkExternalBufferProperties external_buffer_properties = vku::InitStructHelper();
            vk::GetPhysicalDeviceExternalBufferProperties(Gpu(), &external_buffer_info, &external_buffer_properties);
            const auto external_features = external_buffer_properties.externalMemoryProperties.externalMemoryFeatures;
            if (external_features & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) {
                supported_handle_types |= external_buffer_info.handleType;
                any_compatible_group = external_buffer_properties.externalMemoryProperties.compatibleHandleTypes;
            }
        });

    // Main test case. Handle types are supported but not compatible with each other
    if ((supported_handle_types & any_compatible_group) != supported_handle_types) {
        external_memory_info.handleTypes = supported_handle_types;
        CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-pNext-00920");
    }
}

TEST_F(NegativeExternalMemorySync, CreateImageIncompatibleHandleTypes) {
    TEST_DESCRIPTION("Creating image with incompatible external memory handle types");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    // Try all flags first. It's unlikely all of them are compatible.
    VkExternalMemoryImageCreateInfo external_memory_info = vku::InitStructHelper();
    external_memory_info.handleTypes = AllVkExternalMemoryHandleTypeFlagBits;
    VkImageCreateInfo image_create_info = vku::InitStructHelper(&external_memory_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-pNext-00990");

    // Get all exportable handle types supported by the platform.
    VkExternalMemoryHandleTypeFlags supported_handle_types = 0;
    VkExternalMemoryHandleTypeFlags any_compatible_group = 0;

    VkPhysicalDeviceExternalImageFormatInfo external_image_info = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_image_info);
    image_info.format = image_create_info.format;
    image_info.type = image_create_info.imageType;
    image_info.tiling = image_create_info.tiling;
    image_info.usage = image_create_info.usage;
    image_info.flags = image_create_info.flags;

    IgnoreHandleTypeError(m_errorMonitor);
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(
        AllVkExternalMemoryHandleTypeFlagBits, [&](VkExternalMemoryHandleTypeFlagBits flag) {
            external_image_info.handleType = flag;
            VkExternalImageFormatProperties external_image_properties = vku::InitStructHelper();
            VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_image_properties);
            VkResult result = vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties);
            const auto external_features = external_image_properties.externalMemoryProperties.externalMemoryFeatures;
            if (result == VK_SUCCESS && (external_features & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT)) {
                supported_handle_types |= external_image_info.handleType;
                any_compatible_group = external_image_properties.externalMemoryProperties.compatibleHandleTypes;
            }
        });

    // Main test case. Handle types are supported but not compatible with each other
    if ((supported_handle_types & any_compatible_group) != supported_handle_types) {
        external_memory_info.handleTypes = supported_handle_types;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-pNext-00990");
    }
}

TEST_F(NegativeExternalMemorySync, CreateImageIncompatibleHandleTypesNV) {
    TEST_DESCRIPTION("Creating image with incompatible external memory handle types from NVIDIA extension");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkExternalMemoryImageCreateInfoNV external_memory_info = vku::InitStructHelper();
    VkImageCreateInfo image_create_info = vku::InitStructHelper(&external_memory_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    // Get all exportable handle types supported by the platform.
    VkExternalMemoryHandleTypeFlagsNV supported_handle_types = 0;
    VkExternalMemoryHandleTypeFlagsNV any_compatible_group = 0;

    IterateFlags<VkExternalMemoryHandleTypeFlagBitsNV>(
        AllVkExternalMemoryHandleTypeFlagBitsNV, [&](VkExternalMemoryHandleTypeFlagBitsNV flag) {
            VkExternalImageFormatPropertiesNV external_image_properties = {};
            VkResult result = vk::GetPhysicalDeviceExternalImageFormatPropertiesNV(
                Gpu(), image_create_info.format, image_create_info.imageType, image_create_info.tiling, image_create_info.usage,
                image_create_info.flags, flag, &external_image_properties);
            if (result == VK_SUCCESS &&
                (external_image_properties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_NV)) {
                supported_handle_types |= flag;
                any_compatible_group = external_image_properties.compatibleHandleTypes;
            }
        });

    // Main test case. Handle types are supported but not compatible with each other
    if ((supported_handle_types & any_compatible_group) != supported_handle_types) {
        external_memory_info.handleTypes = supported_handle_types;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-pNext-00991");
    }
}

TEST_F(NegativeExternalMemorySync, ExportImageHandleType) {
    TEST_DESCRIPTION("Test exporting memory with mismatching handleTypes.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    // Create export image
    VkExternalMemoryImageCreateInfo external_image_info = vku::InitStructHelper();
    VkImageCreateInfo image_info = vku::InitStructHelper(&external_image_info);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto exportable_types = FindSupportedExternalMemoryHandleTypes(Gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (GetBitSetCount(exportable_types) < 2) {
        GTEST_SKIP() << "Cannot find two distinct exportable handle types, skipping test";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    exportable_types &= ~handle_type;
    const auto handle_type2 = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    assert(handle_type != handle_type2);

    // Create an image with one of the handle types
    external_image_info.handleTypes = handle_type;
    vkt::Image image(*m_device, image_info, vkt::no_mem);

    // Create export memory with a different handle type
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = image;
    const bool dedicated_allocation = HandleTypeNeedsDedicatedAllocation(Gpu(), image_info, handle_type2);
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type2;

    // vkBindImageMemory
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-02728");
    image.AllocateAndBindMemory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    m_errorMonitor->VerifyFound();

    // vkBindImageMemory2
    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper();
    bind_image_info.image = image.handle();
    bind_image_info.memory = image.Memory();  // re-use memory object from the previous check
    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-memory-02728");
    vk::BindImageMemory2(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, BufferMemoryWithUnsupportedHandleType) {
    TEST_DESCRIPTION("Bind buffer memory with unsupported external memory handle type.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    const auto buffer_info = vkt::Buffer::CreateInfo(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, {}, &external_buffer_info);
    const auto exportable_types =
        FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    if (exportable_types == AllVkExternalMemoryHandleTypeFlagBits) {
        GTEST_SKIP() << "This test requires at least one unsupported handle type, but all handle types are supported";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    external_buffer_info.handleTypes = handle_type;
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    // Check if dedicated allocation is required
    bool dedicated_allocation = false;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(exportable_types, [&](VkExternalMemoryHandleTypeFlagBits handle_type) {
        if (HandleTypeNeedsDedicatedAllocation(Gpu(), buffer_info, handle_type)) {
            dedicated_allocation = true;
        }
    });
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer;

    // Create memory object with unsupported handle type
    const auto not_supported_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(~exportable_types);
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type | not_supported_type;

    auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(),
                                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    VkResult result = buffer.Memory().TryInit(*m_device, alloc_info);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "vkAllocateMemory failed (probably due to unsupported handle type). Unable to reach vkBindBufferMemory to "
                        "run valdiation";
    }

    m_errorMonitor->SetDesiredError("VUID-VkExportMemoryAllocateInfo-handleTypes-00656");
    buffer.BindMemory(buffer.Memory(), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, BufferMemoryWithIncompatibleHandleTypes) {
    TEST_DESCRIPTION("Bind buffer memory with incompatible external memory handle types.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    const auto buffer_info = vkt::Buffer::CreateInfo(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, {}, &external_buffer_info);
    const auto exportable_types =
        FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    const auto compatible_types = GetCompatibleHandleTypes(Gpu(), buffer_info, handle_type);
    if ((exportable_types & compatible_types) == exportable_types) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }
    external_buffer_info.handleTypes = handle_type;
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    // Check if dedicated allocation is required
    bool dedicated_allocation = false;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(exportable_types, [&](VkExternalMemoryHandleTypeFlagBits handle_type) {
        if (HandleTypeNeedsDedicatedAllocation(Gpu(), buffer_info, handle_type)) {
            dedicated_allocation = true;
        }
    });
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer;

    // Create memory object with incompatible handle types
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = exportable_types;
    m_errorMonitor->SetDesiredError("VUID-VkExportMemoryAllocateInfo-handleTypes-00656");
    buffer.AllocateAndBindMemory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImageMemoryWithUnsupportedHandleType) {
    TEST_DESCRIPTION("Bind image memory with unsupported external memory handle type.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkExternalMemoryImageCreateInfo external_image_info = vku::InitStructHelper();
    VkImageCreateInfo image_info = vku::InitStructHelper(&external_image_info);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto exportable_types = FindSupportedExternalMemoryHandleTypes(Gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    // This test does not support the AHB handle type, which does not
    // allow to query memory requirements before memory is bound
    exportable_types &= ~VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    if (exportable_types == AllVkExternalMemoryHandleTypeFlagBits) {
        GTEST_SKIP() << "This test requires at least one unsupported handle type, but all handle types are supported";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);

    // Create an image with supported handle type
    external_image_info.handleTypes = handle_type;
    vkt::Image image(*m_device, image_info, vkt::no_mem);

    // Create memory object which additionally includes unsupported handle type
    const auto not_supported_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(~exportable_types);
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = image;
    const bool dedicated_allocation = HandleTypeNeedsDedicatedAllocation(Gpu(), image_info, handle_type);
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type | not_supported_type;

    m_errorMonitor->SetDesiredError("VUID-VkExportMemoryAllocateInfo-handleTypes-00656");
    image.AllocateAndBindMemory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImageMemoryWithIncompatibleHandleTypes) {
    TEST_DESCRIPTION("Bind image memory with incompatible external memory handle types.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    // Create export image
    VkExternalMemoryImageCreateInfo external_image_info = vku::InitStructHelper();
    VkImageCreateInfo image_info = vku::InitStructHelper(&external_image_info);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto exportable_types = FindSupportedExternalMemoryHandleTypes(Gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    // This test does not support the AHB handle type, which does not
    // allow to query memory requirements before memory is bound
    exportable_types &= ~VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    const auto compatible_types = GetCompatibleHandleTypes(Gpu(), image_info, handle_type);
    if ((exportable_types & compatible_types) == exportable_types) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }

    external_image_info.handleTypes = handle_type;
    vkt::Image image(*m_device, image_info, vkt::no_mem);

    bool dedicated_allocation = false;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(exportable_types, [&](VkExternalMemoryHandleTypeFlagBits handle_type) {
        if (HandleTypeNeedsDedicatedAllocation(Gpu(), image_info, handle_type)) {
            dedicated_allocation = true;
        }
    });
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = image;

    // Create memory object with incompatible handle types
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = exportable_types;

    m_errorMonitor->SetDesiredError("VUID-VkExportMemoryAllocateInfo-handleTypes-00656");
    image.AllocateAndBindMemory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ExportBufferHandleType) {
    TEST_DESCRIPTION("Test exporting memory with mismatching handleTypes.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    // Create export buffer
    VkExternalMemoryBufferCreateInfo external_info = vku::InitStructHelper();
    VkBufferCreateInfo buffer_info = vku::InitStructHelper(&external_info);
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.size = 4096;

    auto exportable_types = FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (GetBitSetCount(exportable_types) < 2) {
        GTEST_SKIP() << "Cannot find two distinct exportable handle types, skipping test";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    exportable_types &= ~handle_type;
    const auto handle_type2 = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_types);
    assert(handle_type != handle_type2);

    // Create a buffer with one of the handle types
    external_info.handleTypes = handle_type;
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    // Check if dedicated allocation is required
    const bool dedicated_allocation = HandleTypeNeedsDedicatedAllocation(Gpu(), buffer_info, handle_type2);
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer;

    // Create export memory with a different handle type
    auto export_memory_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type2;
    VkBufferMemoryRequirementsInfo2 buffer_memory_requirements_info = vku::InitStructHelper();
    buffer_memory_requirements_info.buffer = buffer.handle();
    VkMemoryDedicatedRequirements memory_dedicated_requirements = vku::InitStructHelper();
    VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper(&memory_dedicated_requirements);
    vk::GetBufferMemoryRequirements2(device(), &buffer_memory_requirements_info, &mem_reqs2);
    VkMemoryRequirements buffer_mem_reqs = mem_reqs2.memoryRequirements;
    const auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                    &export_memory_info);
    const auto memory = vkt::DeviceMemory(*m_device, alloc_info);

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkBindBufferMemory-buffer-01444");  // required dedicated
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-02726");
    if (memory_dedicated_requirements.requiresDedicatedAllocation) {
        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-buffer-01444");
    }
    vk::BindBufferMemory(device(), buffer.handle(), memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper();
    bind_buffer_info.buffer = buffer.handle();
    bind_buffer_info.memory = memory.handle();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkBindBufferMemory-buffer-01444");  // required dedicated
    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-memory-02726");
    if (memory_dedicated_requirements.requiresDedicatedAllocation) {
        m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-buffer-01444");
    }
    vk::BindBufferMemory2(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, TimelineSemaphore) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
    const char *no_tempory_tl_vuid = "VUID-VkImportSemaphoreWin32HandleInfoKHR-flags-03322";
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    const char *no_tempory_tl_vuid = "VUID-VkImportSemaphoreFdInfoKHR-flags-03323";
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    // Check for external semaphore import and export capability
    {
        VkSemaphoreTypeCreateInfo sti = vku::InitStructHelper();
        sti.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        VkPhysicalDeviceExternalSemaphoreInfo esi = vku::InitStructHelper(&sti);
        esi.handleType = handle_type;

        VkExternalSemaphorePropertiesKHR esp = vku::InitStructHelper();

        vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(Gpu(), &esi, &esp);

        if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) ||
            !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT)) {
            GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
        }
    }

    VkResult err;

    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfo esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    VkSemaphoreTypeCreateInfo stci = vku::InitStructHelper(&esci);
    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&stci);

    vkt::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore to import payload into
    stci.pNext = nullptr;
    vkt::Semaphore import_semaphore(*m_device, sci);

    ExternalHandle ext_handle{};
    err = export_semaphore.ExportHandle(ext_handle, handle_type);
    ASSERT_EQ(VK_SUCCESS, err);

    m_errorMonitor->SetDesiredError(no_tempory_tl_vuid);
    err = import_semaphore.ImportHandle(ext_handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT);
    m_errorMonitor->VerifyFound();

    err = import_semaphore.ImportHandle(ext_handle, handle_type);
    ASSERT_EQ(VK_SUCCESS, err);
}

TEST_F(NegativeExternalMemorySync, SyncFdSemaphore) {
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfo esi = vku::InitStructHelper();
    esi.handleType = handle_type;

    VkExternalSemaphorePropertiesKHR esp = vku::InitStructHelper();

    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(Gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting";
    }

    if (!(esp.compatibleHandleTypes & handle_type)) {
        GTEST_SKIP() << "External semaphore does not support VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT";
    }

    // create a timeline semaphore.
    // Note that adding a sync fd VkExportSemaphoreCreateInfo will cause creation to fail.
    VkSemaphoreTypeCreateInfo stci = vku::InitStructHelper();
    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&stci);

    vkt::Semaphore timeline_sem(*m_device, sci);

    // binary semaphore works fine.
    VkExportSemaphoreCreateInfo esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    stci.pNext = &esci;

    stci.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    vkt::Semaphore binary_sem(*m_device, sci);

    // Create a semaphore to import payload into
    vkt::Semaphore import_semaphore(*m_device);

    int fd_handle = -1;

    // timeline not allowed
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreGetFdInfoKHR-handleType-01132");
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreGetFdInfoKHR-handleType-03253");
    timeline_sem.ExportHandle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    // must have pending signal
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreGetFdInfoKHR-handleType-03254");
    binary_sem.ExportHandle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    VkSubmitInfo si = vku::InitStructHelper();
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &binary_sem.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &si, VK_NULL_HANDLE);

    binary_sem.ExportHandle(fd_handle, handle_type);

    // must be temporary
    m_errorMonitor->SetDesiredError("VUID-VkImportSemaphoreFdInfoKHR-handleType-07307");
    import_semaphore.ImportHandle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    import_semaphore.ImportHandle(fd_handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT);

    m_default_queue->Wait();
}

TEST_F(NegativeExternalMemorySync, SyncFdSemaphoreTimelineDependency) {
    TEST_DESCRIPTION("Export using handle with copy transference when semaphore signal depends on unresolved timeline wait");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed to run this test";
    }

    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
    VkPhysicalDeviceExternalSemaphoreInfo external_semahpore_info = vku::InitStructHelper();
    external_semahpore_info.handleType = handle_type;
    VkExternalSemaphoreProperties external_semahpore_props = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalSemaphoreProperties(Gpu(), &external_semahpore_info, &external_semahpore_props);
    if (!(external_semahpore_props.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) ||
        !(external_semahpore_props.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting";
    }
    if (!(external_semahpore_props.compatibleHandleTypes & handle_type)) {
        GTEST_SKIP() << "External semaphore does not support VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT";
    }

    VkExportSemaphoreCreateInfo export_ci = vku::InitStructHelper();
    export_ci.handleTypes = handle_type;
    VkSemaphoreTypeCreateInfo semaphore_type_ci = vku::InitStructHelper(&export_ci);
    semaphore_type_ci.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    VkSemaphoreCreateInfo semaphore_ci = vku::InitStructHelper(&semaphore_type_ci);
    vkt::Semaphore binary_semaphore(*m_device, semaphore_ci);

    vkt::Semaphore timeline_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(timeline_semaphore, 1), vkt::Signal(binary_semaphore));

    int fd_handle = -1;
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreGetFdInfoKHR-handleType-03254");
    binary_semaphore.ExportHandle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(timeline_semaphore, 1));
    m_default_queue->Wait();
}

TEST_F(NegativeExternalMemorySync, SyncFdExportFromImportedSemaphore) {
    TEST_DESCRIPTION("Export from semaphore with imported payload that does not support export");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
    {
        const auto handle_types = FindSupportedExternalSemaphoreHandleTypes(
            Gpu(), VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT);
        if ((handle_types & handle_type) == 0) {
            GTEST_SKIP() << "VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT is not both exportable and importable";
        }
    }
    const auto export_from_import_handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    {
        const auto handle_types = FindSupportedExternalSemaphoreHandleTypes(Gpu(), VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT);
        if ((handle_types & export_from_import_handle_type) == 0) {
            GTEST_SKIP() << "VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT is not exportable";
        }
    }
    VkPhysicalDeviceExternalSemaphoreInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.handleType = export_from_import_handle_type;
    VkExternalSemaphoreProperties semaphore_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalSemaphoreProperties(Gpu(), &semaphore_info, &semaphore_properties);
    if ((handle_type & semaphore_properties.exportFromImportedHandleTypes) != 0) {
        GTEST_SKIP() << "cannot find handle type that does not support export from imported semaphore";
    }

    // create semaphore and export payload
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = handle_type;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    vkt::Semaphore semaphore(*m_device, create_info);

    VkSubmitInfo si = vku::InitStructHelper();
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &semaphore.handle();
    vk::QueueSubmit(m_default_queue->handle(), 1, &si, VK_NULL_HANDLE);

    int handle = 0;
    semaphore.ExportHandle(handle, handle_type);

    // create semaphore and import payload
    VkExportSemaphoreCreateInfo export_info2 = vku::InitStructHelper();  // prepare to export from imported semaphore
    export_info2.handleTypes = export_from_import_handle_type;
    const VkSemaphoreCreateInfo create_info2 = vku::InitStructHelper(&export_info2);
    vkt::Semaphore import_semaphore(*m_device, create_info2);
    import_semaphore.ImportHandle(handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT);

    // export from imported semaphore
    int handle2 = 0;
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreGetFdInfoKHR-semaphore-01133");
    import_semaphore.ExportHandle(handle2, export_from_import_handle_type);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeExternalMemorySync, SyncFdExportFromImportedFence) {
    TEST_DESCRIPTION("Export from fence with imported payload that does not support export");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;
    {
        const auto handle_types = FindSupportedExternalFenceHandleTypes(
            Gpu(), VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT);
        if ((handle_types & handle_type) == 0) {
            GTEST_SKIP() << "VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT is not both exportable and importable";
        }
    }
    const auto export_from_import_handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT;
    {
        const auto handle_types = FindSupportedExternalFenceHandleTypes(Gpu(), VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT);
        if ((handle_types & export_from_import_handle_type) == 0) {
            GTEST_SKIP() << "VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT is not exportable";
        }
    }
    VkPhysicalDeviceExternalFenceInfo fence_info = vku::InitStructHelper();
    fence_info.handleType = export_from_import_handle_type;
    VkExternalFenceProperties fence_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFenceProperties(Gpu(), &fence_info, &fence_properties);
    if ((handle_type & fence_properties.exportFromImportedHandleTypes) != 0) {
        GTEST_SKIP() << "cannot find handle type that does not support export from imported fence";
    }

    // create fence and export payload
    VkExportFenceCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = handle_type;  // at first export handle type, then import it
    const VkFenceCreateInfo create_info = vku::InitStructHelper(&export_info);
    vkt::Fence fence(*m_device, create_info);

    VkSubmitInfo si = vku::InitStructHelper();
    vk::QueueSubmit(m_default_queue->handle(), 1, &si, fence.handle());

    int handle = 0;
    fence.ExportHandle(handle, handle_type);

    // create fence and import payload
    VkExportFenceCreateInfo export_info2 = vku::InitStructHelper();  // prepare to export from imported fence
    export_info2.handleTypes = export_from_import_handle_type;
    const VkFenceCreateInfo create_info2 = vku::InitStructHelper(&export_info2);
    vkt::Fence import_fence(*m_device, create_info2);
    import_fence.ImportHandle(handle, handle_type, VK_FENCE_IMPORT_TEMPORARY_BIT);

    // export from imported fence
    int handle2 = 0;
    m_errorMonitor->SetDesiredError("VUID-VkFenceGetFdInfoKHR-fence-01455");
    import_fence.ExportHandle(handle2, export_from_import_handle_type);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, SyncFdSemaphoreType) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkPhysicalDeviceExternalSemaphoreInfo external_semahpore_info = vku::InitStructHelper();
    external_semahpore_info.handleType = handle_type;

    VkExternalSemaphoreProperties external_semahpore_props = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalSemaphoreProperties(Gpu(), &external_semahpore_info, &external_semahpore_props);
    if (!(external_semahpore_props.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) ||
        !(external_semahpore_props.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting";
    }
    if (!(external_semahpore_props.compatibleHandleTypes & handle_type)) {
        GTEST_SKIP() << "External semaphore does not support VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT";
    }

    VkExportSemaphoreCreateInfo esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    VkSemaphoreTypeCreateInfo stci = vku::InitStructHelper(&esci);
    stci.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&stci);
    vkt::Semaphore binary_sem(*m_device, sci);

    VkSubmitInfo si = vku::InitStructHelper();
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &binary_sem.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &si, VK_NULL_HANDLE);

    int fd_handle = -1;
    binary_sem.ExportHandle(fd_handle, handle_type);

    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    vkt::Semaphore import_semaphore(*m_device, sci);
    m_errorMonitor->SetDesiredError("VUID-VkImportSemaphoreFdInfoKHR-handleType-03264");
    import_semaphore.ImportHandle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeExternalMemorySync, TemporaryFence) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    RETURN_IF_SKIP(Init());

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfo efi = vku::InitStructHelper();
    efi.handleType = handle_type;
    VkExternalFencePropertiesKHR efp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(Gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    // Create a fence to export payload from
    VkExportFenceCreateInfo efci = vku::InitStructHelper();
    efci.handleTypes = handle_type;
    VkFenceCreateInfo fci = vku::InitStructHelper(&efci);
    vkt::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vkt::Fence import_fence(*m_device, fci);

    // Export fence payload to an opaque handle
    ExternalHandle ext_fence{};
    export_fence.ExportHandle(ext_fence, handle_type);
    import_fence.ImportHandle(ext_fence, handle_type, VK_FENCE_IMPORT_TEMPORARY_BIT);

    // Undo the temporary import
    vk::ResetFences(device(), 1, &import_fence.handle());

    // Signal the previously imported fence twice, the second signal should produce a validation error
    vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, import_fence.handle());
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-fence-00064");
    vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, import_fence.handle());
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();

    // Signal without reseting
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-fence-00063");
    vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, import_fence.handle());
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeExternalMemorySync, Fence) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    const auto other_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
    const auto bad_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT;
    const char *bad_export_type_vuid = "VUID-VkFenceGetWin32HandleInfoKHR-handleType-01452";
    const char *other_export_type_vuid = "VUID-VkFenceGetWin32HandleInfoKHR-handleType-01448";
    const char *bad_import_type_vuid = "VUID-VkImportFenceWin32HandleInfoKHR-handleType-01457";
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT;
    const auto other_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;
    const auto bad_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
    const char *bad_export_type_vuid = "VUID-VkFenceGetFdInfoKHR-handleType-01456";
    const char *other_export_type_vuid = "VUID-VkFenceGetFdInfoKHR-handleType-01453";
    const char *bad_import_type_vuid = "VUID-VkImportFenceFdInfoKHR-handleType-01464";
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    RETURN_IF_SKIP(Init());

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfo efi = vku::InitStructHelper();
    efi.handleType = handle_type;
    VkExternalFencePropertiesKHR efp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(Gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    // Create a fence to export payload from
    VkExportFenceCreateInfo efci = vku::InitStructHelper();
    efci.handleTypes = handle_type;
    VkFenceCreateInfo fci = vku::InitStructHelper(&efci);
    vkt::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vkt::Fence import_fence(*m_device, fci);

    ExternalHandle ext_handle{};

    // windows vs unix mismatch
    m_errorMonitor->SetDesiredError(bad_export_type_vuid);
    export_fence.ExportHandle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();

    // a valid type for the platform which we didn't ask for during create
    m_errorMonitor->SetDesiredError(other_export_type_vuid);
    if constexpr (other_type == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT) {
        // SYNC_FD is a special snowflake
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkFenceGetFdInfoKHR-handleType-01454");
    }
    export_fence.ExportHandle(ext_handle, other_type);
    m_errorMonitor->VerifyFound();

    export_fence.ExportHandle(ext_handle, handle_type);

    m_errorMonitor->SetDesiredError(bad_import_type_vuid);
    import_fence.ImportHandle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkImportFenceWin32HandleInfoKHR ifi = vku::InitStructHelper();
    ifi.fence = import_fence.handle();
    ifi.handleType = handle_type;
    ifi.handle = ext_handle;
    ifi.flags = 0;
    ifi.name = L"something";

    // If handleType is not VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT, name must be NULL
    // However, it looks like at least some windows drivers don't support exporting KMT handles for fences
    if constexpr (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT) {
        m_errorMonitor->SetDesiredError("VUID-VkImportFenceWin32HandleInfoKHR-handleType-01459");
    }
    // If handle is not NULL, name must be NULL
    m_errorMonitor->SetDesiredError("VUID-VkImportFenceWin32HandleInfoKHR-handle-01462");
    vk::ImportFenceWin32HandleKHR(device(), &ifi);
    m_errorMonitor->VerifyFound();
#endif
    auto err = import_fence.ImportHandle(ext_handle, handle_type);
    ASSERT_EQ(VK_SUCCESS, err);
}

TEST_F(NegativeExternalMemorySync, SyncFdFence) {
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;

    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfo efi = vku::InitStructHelper();
    efi.handleType = handle_type;
    VkExternalFencePropertiesKHR efp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(Gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    // Create a fence to export payload from
    VkExportFenceCreateInfo efci = vku::InitStructHelper();
    efci.handleTypes = handle_type;
    VkFenceCreateInfo fci = vku::InitStructHelper(&efci);
    vkt::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vkt::Fence import_fence(*m_device, fci);

    int fd_handle = -1;

    // SYNC_FD must have a pending signal for export
    m_errorMonitor->SetDesiredError("VUID-VkFenceGetFdInfoKHR-handleType-01454");
    export_fence.ExportHandle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, export_fence.handle());

    export_fence.ExportHandle(fd_handle, handle_type);

    // must be temporary
    m_errorMonitor->SetDesiredError("VUID-VkImportFenceFdInfoKHR-handleType-07306");
    import_fence.ImportHandle(fd_handle, handle_type);
    m_errorMonitor->VerifyFound();

    import_fence.ImportHandle(fd_handle, handle_type, VK_FENCE_IMPORT_TEMPORARY_BIT);

    import_fence.Wait(1000000000);
}

TEST_F(NegativeExternalMemorySync, TemporarySemaphore) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfo esi = vku::InitStructHelper();
    esi.handleType = handle_type;

    VkExternalSemaphorePropertiesKHR esp = vku::InitStructHelper();

    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(Gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
    }

    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfo esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&esci);

    vkt::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore to import payload into
    sci.pNext = nullptr;
    vkt::Semaphore import_semaphore(*m_device, sci);

    ExternalHandle ext_handle{};
    export_semaphore.ExportHandle(ext_handle, handle_type);
    import_semaphore.ImportHandle(ext_handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT);

    // Wait on the imported semaphore twice in vk::QueueSubmit, the second wait should be an error
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    std::vector<VkSubmitInfo> si(4, vku::InitStruct<VkSubmitInfo>());
    si[0].signalSemaphoreCount = 1;
    si[0].pSignalSemaphores = &export_semaphore.handle();

    si[1].waitSemaphoreCount = 1;
    si[1].pWaitSemaphores = &import_semaphore.handle();
    si[1].pWaitDstStageMask = &flags;

    si[2] = si[0];
    si[3] = si[1];

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pWaitSemaphores-03238");
    vk::QueueSubmit(m_default_queue->handle(), si.size(), si.data(), VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    auto index = m_device->graphics_queue_node_index_;
    if (m_device->Physical().queue_properties_[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
        // Wait on the imported semaphore twice in vk::QueueBindSparse, the second wait should be an error
        std::vector<VkBindSparseInfo> bi(4, vku::InitStruct<VkBindSparseInfo>());
        bi[0].signalSemaphoreCount = 1;
        bi[0].pSignalSemaphores = &export_semaphore.handle();

        bi[1].waitSemaphoreCount = 1;
        bi[1].pWaitSemaphores = &import_semaphore.handle();

        bi[2] = bi[0];
        bi[3] = bi[1];
        m_errorMonitor->SetDesiredError("VUID-vkQueueBindSparse-pWaitSemaphores-03245");
        vk::QueueBindSparse(m_default_queue->handle(), bi.size(), bi.data(), VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Cleanup
    m_default_queue->Wait();
}

TEST_F(NegativeExternalMemorySync, Semaphore) {
    TEST_DESCRIPTION("Import and export invalid external semaphores, no queue sumbits involved.");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
    const auto bad_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    const auto other_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    const char *bad_export_type_vuid = "VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01131";
    const char *other_export_type_vuid = "VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01126";
    const char *bad_import_type_vuid = "VUID-VkImportSemaphoreWin32HandleInfoKHR-handleType-01140";
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    const auto bad_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
    const auto other_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
    const char *bad_export_type_vuid = "VUID-VkSemaphoreGetFdInfoKHR-handleType-01136";
    const char *other_export_type_vuid = "VUID-VkSemaphoreGetFdInfoKHR-handleType-01132";
    const char *bad_import_type_vuid = "VUID-VkImportSemaphoreFdInfoKHR-handleType-01143";
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfo esi = vku::InitStructHelper();
    esi.handleType = handle_type;

    VkExternalSemaphorePropertiesKHR esp = vku::InitStructHelper();

    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(Gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
    }
    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfo esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&esci);

    vkt::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore for importing
    vkt::Semaphore import_semaphore(*m_device);

    ExternalHandle ext_handle{};

    // windows vs unix mismatch
    m_errorMonitor->SetDesiredError(bad_export_type_vuid);
    export_semaphore.ExportHandle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();

    // not specified during create
    if constexpr (other_type == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT) {
        // SYNC_FD must have pending signal
        m_errorMonitor->SetDesiredError("VUID-VkSemaphoreGetFdInfoKHR-handleType-03254");
    }
    m_errorMonitor->SetDesiredError(other_export_type_vuid);
    export_semaphore.ExportHandle(ext_handle, other_type);
    m_errorMonitor->VerifyFound();

    export_semaphore.ExportHandle(ext_handle, handle_type);

    m_errorMonitor->SetDesiredError(bad_import_type_vuid);
    export_semaphore.ImportHandle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImportMemoryHandleType) {
    TEST_DESCRIPTION("Validate import memory handleType for buffers and images");
    SetTargetApiVersion(VK_API_VERSION_1_1);
#ifdef _WIN32
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    AddRequiredExtensions(ext_mem_extension_name);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "External tests are not supported by MockICD, skipping tests";
    }

    // Check for import/export capability
    // export used to feed memory to test import
    VkPhysicalDeviceExternalBufferInfo ebi = vku::InitStructHelper();
    ebi.handleType = handle_type;
    ebi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkExternalBufferPropertiesKHR ebp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalBufferProperties(Gpu(), &ebi, &ebp);
    if (!(ebp.externalMemoryProperties.compatibleHandleTypes & handle_type) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External buffer does not support importing and exporting, skipping test";
    }

    // Check if dedicated allocation is required
    const bool buffer_dedicated_allocation =
        ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT;

    constexpr VkMemoryPropertyFlags mem_flags = 0;
    constexpr VkDeviceSize buffer_size = 1024;

    // Create export and import buffers
    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    external_buffer_info.handleTypes = handle_type;

    auto buffer_info = vkt::Buffer::CreateInfo(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer_info.pNext = &external_buffer_info;
    vkt::Buffer buffer_export(*m_device, buffer_info, vkt::no_mem);
    const VkMemoryRequirements buffer_export_reqs = buffer_export.MemoryRequirements();

    auto importable_buffer_types =
        FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT);
    importable_buffer_types &= ~handle_type;  // we need to find a flag that is different from handle_type
    if (importable_buffer_types == 0) GTEST_SKIP() << "Cannot find two different buffer handle types, skipping test";
    auto wrong_buffer_handle_type =
        static_cast<VkExternalMemoryHandleTypeFlagBits>(1 << MostSignificantBit(importable_buffer_types));
    external_buffer_info.handleTypes = wrong_buffer_handle_type;

    vkt::Buffer buffer_import(*m_device, buffer_info, vkt::no_mem);
    const VkMemoryRequirements buffer_import_reqs = buffer_import.MemoryRequirements();
    assert(buffer_import_reqs.memoryTypeBits != 0);  // according to spec at least one bit is set
    if ((buffer_import_reqs.memoryTypeBits & buffer_export_reqs.memoryTypeBits) == 0) {
        // required by VU 01743
        GTEST_SKIP() << "Cannot find memory type that supports both export and import";
    }

    // Allocation info
    auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_export_reqs, mem_flags);

    // Add export allocation info to pNext chain
    VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = handle_type;

    alloc_info.pNext = &export_info;

    // Add dedicated allocation info to pNext chain if required
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer_export.handle();

    if (buffer_dedicated_allocation) {
        export_info.pNext = &dedicated_info;
    }

    // Allocate memory to be exported
    vkt::DeviceMemory memory_buffer_export;
    memory_buffer_export.init(*m_device, alloc_info);

    // Bind exported memory
    buffer_export.BindMemory(memory_buffer_export, 0);

    VkExternalMemoryImageCreateInfo external_image_info = vku::InitStructHelper();
    external_image_info.handleTypes = handle_type;

    VkImageCreateInfo image_info = vku::InitStructHelper(&external_image_info);
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    vkt::Image image_export(*m_device, image_info, vkt::no_mem);

    const bool image_dedicated_allocation = HandleTypeNeedsDedicatedAllocation(Gpu(), image_info, handle_type);
    VkMemoryDedicatedAllocateInfo image_dedicated_info = vku::InitStructHelper();
    image_dedicated_info.image = image_export;

    auto export_memory_info =
        vku::InitStruct<VkExportMemoryAllocateInfo>(image_dedicated_allocation ? &image_dedicated_info : nullptr);
    export_memory_info.handleTypes = handle_type;
    image_export.AllocateAndBindMemory(*m_device, mem_flags, &export_memory_info);

    auto importable_image_types =
        FindSupportedExternalMemoryHandleTypes(Gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT);
    importable_image_types &= ~handle_type;  // we need to find a flag that is different from handle_type
    if (importable_image_types == 0) {
        GTEST_SKIP() << "Cannot find two different image handle types";
    }
    auto wrong_image_handle_type = static_cast<VkExternalMemoryHandleTypeFlagBits>(1 << MostSignificantBit(importable_image_types));
    if (wrong_image_handle_type == VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) {
        GTEST_SKIP() << "Don't want to use AHB as it has extra restrictions";
    }
    external_image_info.handleTypes = wrong_image_handle_type;

    vkt::Image image_import(*m_device, image_info, vkt::no_mem);

#ifdef _WIN32
    // Export memory to handle
    VkMemoryGetWin32HandleInfoKHR mghi_buffer = {VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR, nullptr,
                                                 memory_buffer_export.handle(), handle_type};
    VkMemoryGetWin32HandleInfoKHR mghi_image = {VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR, nullptr,
                                                image_export.Memory().handle(), handle_type};
    HANDLE handle_buffer;
    HANDLE handle_image;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryWin32HandleKHR(device(), &mghi_buffer, &handle_buffer));
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryWin32HandleKHR(device(), &mghi_image, &handle_image));

    VkImportMemoryWin32HandleInfoKHR import_info_buffer = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, nullptr,
                                                           handle_type, handle_buffer};
    VkImportMemoryWin32HandleInfoKHR import_info_image = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, nullptr,
                                                          handle_type, handle_image};
#else
    // Export memory to fd
    VkMemoryGetFdInfoKHR mgfi_buffer = vku::InitStructHelper();
    mgfi_buffer.handleType = handle_type;
    mgfi_buffer.memory = memory_buffer_export.handle();

    VkMemoryGetFdInfoKHR mgfi_image = vku::InitStructHelper();
    mgfi_image.handleType = handle_type;
    mgfi_image.memory = image_export.Memory().handle();

    int fd_buffer;
    int fd_image;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryFdKHR(device(), &mgfi_buffer, &fd_buffer));
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryFdKHR(device(), &mgfi_image, &fd_image));

    VkImportMemoryFdInfoKHR import_info_buffer = vku::InitStructHelper();
    import_info_buffer.handleType = handle_type;
    import_info_buffer.fd = fd_buffer;

    VkImportMemoryFdInfoKHR import_info_image = vku::InitStructHelper();
    import_info_image.handleType = handle_type;
    import_info_image.fd = fd_image;
#endif

    // Import memory
    alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_import_reqs, mem_flags);
    alloc_info.pNext = &import_info_buffer;
    if constexpr (handle_type == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT) {
        alloc_info.allocationSize = buffer_export_reqs.size;
    }
    vkt::DeviceMemory memory_buffer_import;
    memory_buffer_import.init(*m_device, alloc_info);
    ASSERT_TRUE(memory_buffer_import.initialized());

    VkMemoryRequirements image_import_reqs = image_import.MemoryRequirements();
    if (image_import_reqs.memoryTypeBits == 0) {
        GTEST_SKIP() << "no suitable memory found, skipping test";
    }
    alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image_import_reqs, mem_flags);
    alloc_info.pNext = &import_info_image;
    if constexpr (handle_type == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT) {
        alloc_info.allocationSize = image_export.MemoryRequirements().size;
    }
    vkt::DeviceMemory memory_image_import;
    memory_image_import.init(*m_device, alloc_info);

    // Bind imported memory with different handleType
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-02985");
    vk::BindBufferMemory(device(), buffer_import.handle(), memory_buffer_import.handle(), 0);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper();
    bind_buffer_info.buffer = buffer_import.handle();
    bind_buffer_info.memory = memory_buffer_import.handle();
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-memory-02985");
    vk::BindBufferMemory2(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-02989");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01617");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01615");
    if (image_dedicated_allocation) {
        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-01445");
    }
    vk::BindImageMemory(device(), image_import.handle(), memory_image_import.handle(), 0);
    m_errorMonitor->VerifyFound();

    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper();
    bind_image_info.image = image_import.handle();
    bind_image_info.memory = memory_image_import.handle();
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-memory-02989");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01617");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01615");
    if (image_dedicated_allocation) {
        m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-image-01445");
    }
    vk::BindImageMemory2(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, FenceExportWithUnsupportedHandleType) {
    TEST_DESCRIPTION("Create fence with unsupported external handle type in VkExportFenceCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    const auto exportable_types = FindSupportedExternalFenceHandleTypes(Gpu(), VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    if (exportable_types == AllVkExternalFenceHandleTypeFlagBits) {
        GTEST_SKIP() << "This test requires at least one unsupported handle type, but all handle types are supported";
    }
    // Fence export with unsupported handle type
    const auto unsupported_type = LeastSignificantFlag<VkExternalFenceHandleTypeFlagBits>(~exportable_types);
    VkExportFenceCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = unsupported_type;

    const VkFenceCreateInfo create_info = vku::InitStructHelper(&export_info);
    VkFence fence = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkExportFenceCreateInfo-handleTypes-01446");
    vk::CreateFence(device(), &create_info, nullptr, &fence);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, FenceExportWithIncompatibleHandleType) {
    TEST_DESCRIPTION("Create fence with incompatible external handle types in VkExportFenceCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    const auto exportable_types = FindSupportedExternalFenceHandleTypes(Gpu(), VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalFenceHandleTypeFlagBits>(exportable_types);
    const auto compatible_types = GetCompatibleHandleTypes(Gpu(), handle_type);
    if ((exportable_types & compatible_types) == exportable_types) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }

    // Fence export with incompatible handle types
    VkExportFenceCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = exportable_types;

    const VkFenceCreateInfo create_info = vku::InitStructHelper(&export_info);
    VkFence fence = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkExportFenceCreateInfo-handleTypes-01446");
    vk::CreateFence(device(), &create_info, nullptr, &fence);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, SemaphoreExportWithUnsupportedHandleType) {
    TEST_DESCRIPTION("Create semaphore with unsupported external handle type in VkExportSemaphoreCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    const auto exportable_types = FindSupportedExternalSemaphoreHandleTypes(Gpu(), VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    if (exportable_types == AllVkExternalSemaphoreHandleTypeFlagBits) {
        GTEST_SKIP() << "This test requires at least one unsupported handle type, but all handle types are supported";
    }
    // Semaphore export with unsupported handle type
    const auto unsupported_type = LeastSignificantFlag<VkExternalSemaphoreHandleTypeFlagBits>(~exportable_types);
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = unsupported_type;

    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    VkSemaphore semaphore = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkExportSemaphoreCreateInfo-handleTypes-01124");
    vk::CreateSemaphore(device(), &create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, SemaphoreExportWithIncompatibleHandleType) {
    TEST_DESCRIPTION("Create semaphore with incompatible external handle types in VkExportSemaphoreCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    const auto exportable_types = FindSupportedExternalSemaphoreHandleTypes(Gpu(), VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT);
    if (!exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalSemaphoreHandleTypeFlagBits>(exportable_types);
    const auto compatible_types = GetCompatibleHandleTypes(Gpu(), handle_type);
    if ((exportable_types & compatible_types) == exportable_types) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }

    // Semaphore export with incompatible handle types
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = exportable_types;

    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    VkSemaphore semaphore = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkExportSemaphoreCreateInfo-handleTypes-01124");
    vk::CreateSemaphore(device(), &create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, MemoryAndMemoryNV) {
    TEST_DESCRIPTION("Test for both external memory and external memory NV in image create pNext chain.");

    AddRequiredExtensions(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkExternalMemoryImageCreateInfoNV external_mem_nv = vku::InitStructHelper();
    VkExternalMemoryImageCreateInfo external_mem = vku::InitStructHelper(&external_mem_nv);
    VkImageCreateInfo ici = vku::InitStructHelper(&external_mem);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    const auto supported_types_nv =
        FindSupportedExternalMemoryHandleTypesNV(Gpu(), ici, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_NV);
    const auto supported_types = FindSupportedExternalMemoryHandleTypes(Gpu(), ici, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (!supported_types_nv || !supported_types) {
        GTEST_SKIP() << "Cannot find one regular handle type and one nvidia extension's handle type";
    }
    external_mem_nv.handleTypes = LeastSignificantFlag<VkExternalMemoryFeatureFlagBitsNV>(supported_types_nv);
    external_mem.handleTypes = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(supported_types);
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-00988");
}

TEST_F(NegativeExternalMemorySync, MemoryImageLayout) {
    TEST_DESCRIPTION("Validate layout of image with external memory");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkExternalMemoryImageCreateInfo external_mem = vku::InitStructHelper();
    VkImageCreateInfo ici = vku::InitStructHelper(&external_mem);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {32, 32, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    const auto supported_types = FindSupportedExternalMemoryHandleTypes(Gpu(), ici, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (supported_types) {
        external_mem.handleTypes = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(supported_types);
        CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-01443");
    }
    if (IsExtensionsEnabled(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME)) {
        VkExternalMemoryImageCreateInfoNV external_mem_nv = vku::InitStructHelper();
        const auto supported_types_nv =
            FindSupportedExternalMemoryHandleTypesNV(Gpu(), ici, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_NV);
        if (supported_types_nv) {
            external_mem_nv.handleTypes = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBitsNV>(supported_types_nv);
            ici.pNext = &external_mem_nv;
            CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-01443");
        }
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(NegativeExternalMemorySync, D3D12FenceSubmitInfo) {
    TEST_DESCRIPTION("Test invalid D3D12FenceSubmitInfo");
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);
    vkt::Semaphore semaphore(*m_device);

    // VkD3D12FenceSubmitInfoKHR::waitSemaphoreValuesCount == 1 is different from VkSubmitInfo::waitSemaphoreCount == 0
    {
        const uint64_t waitSemaphoreValues = 0;
        VkD3D12FenceSubmitInfoKHR d3d12_fence_submit_info = vku::InitStructHelper();
        d3d12_fence_submit_info.waitSemaphoreValuesCount = 1;
        d3d12_fence_submit_info.pWaitSemaphoreValues = &waitSemaphoreValues;
        const VkSubmitInfo submit_info = vku::InitStructHelper(&d3d12_fence_submit_info);

        m_errorMonitor->SetDesiredError("VUID-VkD3D12FenceSubmitInfoKHR-waitSemaphoreValuesCount-00079");
        vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
    // VkD3D12FenceSubmitInfoKHR::signalSemaphoreCount == 0 is different from VkSubmitInfo::signalSemaphoreCount == 1
    {
        VkD3D12FenceSubmitInfoKHR d3d12_fence_submit_info = vku::InitStructHelper();
        VkSubmitInfo submit_info = vku::InitStructHelper(&d3d12_fence_submit_info);
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &semaphore.handle();

        m_errorMonitor->SetDesiredError("VUID-VkD3D12FenceSubmitInfoKHR-signalSemaphoreValuesCount-00080");
        vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

TEST_F(NegativeExternalMemorySync, GetMemoryFdHandle) {
    TEST_DESCRIPTION("Validate VkMemoryGetFdInfoKHR passed to vkGetMemoryFdKHR");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    int fd = -1;

    // Allocate memory without VkExportMemoryAllocateInfo in the pNext chain
    {
        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = 32;
        alloc_info.memoryTypeIndex = 0;
        vkt::DeviceMemory memory;
        memory.init(*m_device, alloc_info);

        VkMemoryGetFdInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryGetFdInfoKHR-handleType-00671");
        vk::GetMemoryFdKHR(*m_device, &get_handle_info, &fd);
        m_errorMonitor->VerifyFound();
    }
    // VkExportMemoryAllocateInfo::handleTypes does not include requested handle type
    {
        VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
        export_info.handleTypes = 0;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&export_info);
        alloc_info.allocationSize = 1024;
        alloc_info.memoryTypeIndex = 0;
        vkt::DeviceMemory memory;
        memory.init(*m_device, alloc_info);

        VkMemoryGetFdInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryGetFdInfoKHR-handleType-00671");
        vk::GetMemoryFdKHR(*m_device, &get_handle_info, &fd);
        m_errorMonitor->VerifyFound();
    }
    // Request handle of the wrong type
    {
        VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
        export_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&export_info);
        alloc_info.allocationSize = 1024;
        alloc_info.memoryTypeIndex = 0;

        vkt::DeviceMemory memory;
        memory.init(*m_device, alloc_info);
        VkMemoryGetFdInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryGetFdInfoKHR-handleType-00672");
        vk::GetMemoryFdKHR(*m_device, &get_handle_info, &fd);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeExternalMemorySync, ImportMemoryFromFdHandle) {
    TEST_DESCRIPTION("POSIX fd handle memory import. Import parameters do not match payload's parameters");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "External tests are not supported by MockICD, skipping tests";
    }
    constexpr auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkExternalMemoryFeatureFlags external_features = 0;
    {
        constexpr auto required_features = VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
        VkPhysicalDeviceExternalBufferInfo external_info = vku::InitStructHelper();
        external_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        external_info.handleType = handle_type;
        VkExternalBufferProperties external_properties = vku::InitStructHelper();
        vk::GetPhysicalDeviceExternalBufferProperties(Gpu(), &external_info, &external_properties);
        external_features = external_properties.externalMemoryProperties.externalMemoryFeatures;
        if ((external_features & required_features) != required_features) {
            GTEST_SKIP() << "External buffer does not support both export and import, skipping test";
        }
    }

    vkt::Buffer buffer;
    {
        VkExternalMemoryBufferCreateInfo external_info = vku::InitStructHelper();
        external_info.handleTypes = handle_type;
        auto create_info = vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        create_info.pNext = &external_info;
        buffer.InitNoMemory(*m_device, create_info);
    }

    vkt::DeviceMemory memory;
    VkDeviceSize payload_size = 0;
    uint32_t payload_memory_type = 0;
    {
        const bool dedicated_allocation = (external_features & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;
        VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
        dedicated_info.buffer = buffer;
        auto export_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
        export_info.handleTypes = handle_type;
        auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0, &export_info);
        memory.init(*m_device, alloc_info);
        buffer.BindMemory(memory, 0);
        payload_size = alloc_info.allocationSize;
        payload_memory_type = alloc_info.memoryTypeIndex;
    }

    int fd = -1;
    {
        VkMemoryGetFdInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = handle_type;
        ASSERT_EQ(VK_SUCCESS, vk::GetMemoryFdKHR(*m_device, &get_handle_info, &fd));
    }
    VkImportMemoryFdInfoKHR import_info = vku::InitStructHelper();
    import_info.handleType = handle_type;
    import_info.fd = fd;
    VkMemoryAllocateInfo alloc_info_with_import = vku::InitStructHelper(&import_info);
    VkDeviceMemory imported_memory = VK_NULL_HANDLE;

    // allocationSize != payload's allocationSize
    {
        alloc_info_with_import.allocationSize = payload_size * 2;
        alloc_info_with_import.memoryTypeIndex = payload_memory_type;
        m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-allocationSize-01742");
        vk::AllocateMemory(*m_device, &alloc_info_with_import, nullptr, &imported_memory);
        m_errorMonitor->VerifyFound();
    }
    // memoryTypeIndex != payload's memoryTypeIndex
    {
        alloc_info_with_import.allocationSize = payload_size;
        alloc_info_with_import.memoryTypeIndex = payload_memory_type + 1;
        m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-allocationSize-01742");
        // If device only has 1 memory type
        m_errorMonitor->SetUnexpectedError("VUID-vkAllocateMemory-pAllocateInfo-01714");
        vk::AllocateMemory(*m_device, &alloc_info_with_import, nullptr, &imported_memory);
        m_errorMonitor->VerifyFound();
    }
    // Finish this test with a successful import operation in order to release the ownership of the file descriptor.
    // The alternative is to use 'close' system call.
    {
        alloc_info_with_import.allocationSize = payload_size;
        alloc_info_with_import.memoryTypeIndex = payload_memory_type;
        vkt::DeviceMemory successfully_imported_memory;
        successfully_imported_memory.init(*m_device, alloc_info_with_import);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(NegativeExternalMemorySync, GetMemoryWin32Handle) {
    TEST_DESCRIPTION("Validate VkMemoryGetWin32HandleInfoKHR passed to vkGetMemoryWin32HandleKHR");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    HANDLE handle = NULL;

    // Allocate memory without VkExportMemoryAllocateInfo in the pNext chain
    {
        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = 32;
        alloc_info.memoryTypeIndex = 0;
        vkt::DeviceMemory memory;
        memory.init(*m_device, alloc_info);

        VkMemoryGetWin32HandleInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryGetWin32HandleInfoKHR-handleType-00662");
        vk::GetMemoryWin32HandleKHR(*m_device, &get_handle_info, &handle);
        m_errorMonitor->VerifyFound();
    }
    // VkExportMemoryAllocateInfo::handleTypes does not include requested handle type
    {
        VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
        export_info.handleTypes = 0;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&export_info);
        alloc_info.allocationSize = 1024;
        alloc_info.memoryTypeIndex = 0;
        vkt::DeviceMemory memory;
        memory.init(*m_device, alloc_info);

        VkMemoryGetWin32HandleInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryGetWin32HandleInfoKHR-handleType-00662");
        vk::GetMemoryWin32HandleKHR(*m_device, &get_handle_info, &handle);
        m_errorMonitor->VerifyFound();
    }
    // Request handle of the wrong type
    {
        VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
        export_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&export_info);
        alloc_info.allocationSize = 1024;
        alloc_info.memoryTypeIndex = 0;

        vkt::DeviceMemory memory;
        memory.init(*m_device, alloc_info);
        VkMemoryGetWin32HandleInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryGetWin32HandleInfoKHR-handleType-00664");
        vk::GetMemoryWin32HandleKHR(*m_device, &get_handle_info, &handle);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeExternalMemorySync, ImportMemoryFromWin32Handle) {
    TEST_DESCRIPTION("Win32 handle memory import. Import parameters do not match payload's parameters");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);
    constexpr auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    VkExternalMemoryFeatureFlags external_features = 0;
    {
        constexpr auto required_features = VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
        VkPhysicalDeviceExternalImageFormatInfo external_info = vku::InitStructHelper();
        external_info.handleType = handle_type;
        VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_info);
        image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_info.type = VK_IMAGE_TYPE_2D;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        VkExternalImageFormatProperties external_properties = vku::InitStructHelper();
        VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_properties);
        const VkResult result = vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties);
        external_features = external_properties.externalMemoryProperties.externalMemoryFeatures;
        if (result != VK_SUCCESS || (external_features & required_features) != required_features) {
            GTEST_SKIP() << "External image does not support both export and import, skipping test";
        }
    }

    vkt::Image image;
    {
        VkExternalMemoryImageCreateInfo external_info = vku::InitStructHelper();
        external_info.handleTypes = handle_type;
        auto create_info = vkt::Image::CreateInfo();
        create_info.pNext = &external_info;
        create_info.imageType = VK_IMAGE_TYPE_2D;
        create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image.InitNoMemory(*m_device, create_info);
    }

    vkt::DeviceMemory memory;
    VkDeviceSize payload_size = 0;
    uint32_t payload_memory_type = 0;
    {
        const bool dedicated_allocation = (external_features & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;
        VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
        dedicated_info.image = image;
        auto export_info = vku::InitStruct<VkExportMemoryAllocateInfo>(dedicated_allocation ? &dedicated_info : nullptr);
        export_info.handleTypes = handle_type;
        auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image.MemoryRequirements(), 0, &export_info);
        memory.init(*m_device, alloc_info);
        image.BindMemory(memory, 0);
        payload_size = alloc_info.allocationSize;
        payload_memory_type = alloc_info.memoryTypeIndex;
    }

    HANDLE handle = NULL;
    {
        VkMemoryGetWin32HandleInfoKHR get_handle_info = vku::InitStructHelper();
        get_handle_info.memory = memory;
        get_handle_info.handleType = handle_type;
        ASSERT_EQ(VK_SUCCESS, vk::GetMemoryWin32HandleKHR(*m_device, &get_handle_info, &handle));
    }
    VkImportMemoryWin32HandleInfoKHR import_info = vku::InitStructHelper();
    import_info.handleType = handle_type;
    import_info.handle = handle;
    VkMemoryAllocateInfo alloc_info_with_import = vku::InitStructHelper(&import_info);
    VkDeviceMemory imported_memory = VK_NULL_HANDLE;

    // allocationSize != payload's allocationSize
    {
        alloc_info_with_import.allocationSize = payload_size * 2;
        alloc_info_with_import.memoryTypeIndex = payload_memory_type;
        m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-allocationSize-01743");
        vk::AllocateMemory(*m_device, &alloc_info_with_import, nullptr, &imported_memory);
        m_errorMonitor->VerifyFound();
    }
    // memoryTypeIndex != payload's memoryTypeIndex
    {
        alloc_info_with_import.allocationSize = payload_size;
        alloc_info_with_import.memoryTypeIndex = payload_memory_type + 1;
        m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-allocationSize-01743");
        vk::AllocateMemory(*m_device, &alloc_info_with_import, nullptr, &imported_memory);
        m_errorMonitor->VerifyFound();
    }
    // Importing memory object payloads from Windows handles does not transfer ownership of the handle to the driver.
    // For NT handle types, the application must release handle ownership using the CloseHandle system call.
    // That's in contrast with the POSIX file descriptor handles, where memory import operation transfers the ownership,
    // so the application does not need to call 'close' system call.
    ::CloseHandle(handle);
}

TEST_F(NegativeExternalMemorySync, Win32ExportFromImportedSemaphore) {
    TEST_DESCRIPTION("Export from semaphore with imported payload that does not support export");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    {
        const auto handle_types = FindSupportedExternalSemaphoreHandleTypes(
            Gpu(), VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT);
        if ((handle_types & handle_type) == 0) {
            GTEST_SKIP() << "VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT is not both exportable and importable";
        }
    }
    const auto export_from_import_handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
    {
        const auto handle_types = FindSupportedExternalSemaphoreHandleTypes(Gpu(), VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT);
        if ((handle_types & export_from_import_handle_type) == 0) {
            GTEST_SKIP() << "VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT is not exportable";
        }
    }
    VkPhysicalDeviceExternalSemaphoreInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.handleType = export_from_import_handle_type;
    VkExternalSemaphoreProperties semaphore_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalSemaphoreProperties(Gpu(), &semaphore_info, &semaphore_properties);
    if ((handle_type & semaphore_properties.exportFromImportedHandleTypes) != 0) {
        GTEST_SKIP() << "cannot find handle type that does not support export from imported semaphore";
    }

    // create semaphore and export payload
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = handle_type;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    vkt::Semaphore semaphore(*m_device, create_info);
    HANDLE handle = NULL;
    semaphore.ExportHandle(handle, handle_type);

    // create semaphore and import payload
    VkExportSemaphoreCreateInfo export_info2 = vku::InitStructHelper();  // prepare to export from imported semaphore
    export_info2.handleTypes = export_from_import_handle_type;
    const VkSemaphoreCreateInfo create_info2 = vku::InitStructHelper(&export_info2);
    vkt::Semaphore import_semaphore(*m_device, create_info2);
    import_semaphore.ImportHandle(handle, handle_type);

    // export from imported semaphore
    HANDLE handle2 = NULL;
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreGetWin32HandleInfoKHR-semaphore-01128");
    import_semaphore.ExportHandle(handle2, export_from_import_handle_type);
    m_errorMonitor->VerifyFound();

    ::CloseHandle(handle);
}

TEST_F(NegativeExternalMemorySync, Win32ExportFromImportedFence) {
    TEST_DESCRIPTION("Export from fence with imported payload that does not support export");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    {
        const auto handle_types = FindSupportedExternalFenceHandleTypes(
            Gpu(), VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT);
        if ((handle_types & VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT) == 0) {
            GTEST_SKIP() << "VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT is not both exportable and importable";
        }
    }
    const auto export_from_import_handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
    {
        const auto handle_types = FindSupportedExternalFenceHandleTypes(Gpu(), VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT);
        if ((handle_types & export_from_import_handle_type) == 0) {
            GTEST_SKIP() << "VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT is not exportable";
        }
    }
    VkPhysicalDeviceExternalFenceInfo fence_info = vku::InitStructHelper();
    fence_info.handleType = export_from_import_handle_type;
    VkExternalFenceProperties fence_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFenceProperties(Gpu(), &fence_info, &fence_properties);
    if ((handle_type & fence_properties.exportFromImportedHandleTypes) != 0) {
        GTEST_SKIP() << "cannot find handle type that does not support export from imported fence";
    }

    // create fence and export payload
    VkExportFenceCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = handle_type;  // at first export handle type, then import it
    const VkFenceCreateInfo create_info = vku::InitStructHelper(&export_info);
    vkt::Fence fence(*m_device, create_info);
    HANDLE handle = NULL;
    fence.ExportHandle(handle, handle_type);

    // create fence and import payload
    VkExportFenceCreateInfo export_info2 = vku::InitStructHelper();  // prepare to export from imported fence
    export_info2.handleTypes = export_from_import_handle_type;
    const VkFenceCreateInfo create_info2 = vku::InitStructHelper(&export_info2);
    vkt::Fence import_fence(*m_device, create_info2);
    import_fence.ImportHandle(handle, handle_type);

    // export from imported fence
    HANDLE handle2 = NULL;
    m_errorMonitor->SetDesiredError("VUID-VkFenceGetWin32HandleInfoKHR-fence-01450");
    import_fence.ExportHandle(handle2, export_from_import_handle_type);
    m_errorMonitor->VerifyFound();

    ::CloseHandle(handle);
}
#endif

TEST_F(NegativeExternalMemorySync, BufferDedicatedAllocation) {
    TEST_DESCRIPTION("Bind external buffer that requires dedicated allocation to non-dedicated memory.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    const auto buffer_info = vkt::Buffer::CreateInfo(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT, {}, &external_buffer_info);
    const auto exportable_dedicated_types = FindSupportedExternalMemoryHandleTypes(
        Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT);
    if (!exportable_dedicated_types) {
        GTEST_SKIP() << "Unable to find exportable handle type that requires dedicated allocation";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_dedicated_types);

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper();
    export_memory_info.handleTypes = handle_type;
    external_buffer_info.handleTypes = handle_type;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-00639");
    // pNext chain contains VkExportMemoryAllocateInfo but not VkMemoryDedicatedAllocateInfo
    vkt::Buffer buffer(*m_device, buffer_info, 0, &export_memory_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImageDedicatedAllocation) {
    TEST_DESCRIPTION("Bind external image that requires dedicated allocation to non-dedicated memory.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkExternalMemoryImageCreateInfo external_image_info = vku::InitStructHelper();
    VkImageCreateInfo image_info = vku::InitStructHelper(&external_image_info);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto exportable_dedicated_types = FindSupportedExternalMemoryHandleTypes(
        Gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT);
    // This test does not support the AHB handle type, which does not
    // allow to query memory requirements before memory is bound
    exportable_dedicated_types &= ~VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    if (!exportable_dedicated_types) {
        GTEST_SKIP() << "Unable to find exportable handle type that requires dedicated allocation";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_dedicated_types);

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper();
    export_memory_info.handleTypes = handle_type;
    external_image_info.handleTypes = handle_type;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-00639");
    // pNext chain contains VkExportMemoryAllocateInfo but not VkMemoryDedicatedAllocateInfo
    vkt::Image image;
    image.InitNoMemory(*m_device, image_info);
    {
        VkImageMemoryRequirementsInfo2 image_memory_requirements_info = vku::InitStructHelper();
        image_memory_requirements_info.image = image.handle();
        VkMemoryDedicatedRequirements memory_dedicated_requirements = vku::InitStructHelper();

        VkMemoryRequirements2 memory_requirements = vku::InitStructHelper(&memory_dedicated_requirements);
        vk::GetImageMemoryRequirements2(device(), &image_memory_requirements_info, &memory_requirements);

        if (memory_dedicated_requirements.requiresDedicatedAllocation) {
            m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-01445");
        }
    }
    image.AllocateAndBindMemory(*m_device, 0, &export_memory_info);
    m_errorMonitor->VerifyFound();
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(NegativeExternalMemorySync, Win32MemoryHandleProperties) {
    TEST_DESCRIPTION("Call vkGetMemoryWin32HandlePropertiesKHR with invalid Win32 handle or with opaque handle type");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    constexpr auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
    constexpr auto opaque_handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    // Generally, which value is considered invalid depends on the specific Win32 function.
    // VVL assumes that both these values do not represent a valid handle.
    constexpr HANDLE invalid_win32_handle = NULL;
    const HANDLE less_common_invalid_win32_handle = INVALID_HANDLE_VALUE;

    const HANDLE handle_that_passes_validation = (HANDLE)0x12345678;

    VkMemoryWin32HandlePropertiesKHR properties = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-vkGetMemoryWin32HandlePropertiesKHR-handle-00665");
    vk::GetMemoryWin32HandlePropertiesKHR(*m_device, handle_type, invalid_win32_handle, &properties);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetMemoryWin32HandlePropertiesKHR-handle-00665");
    vk::GetMemoryWin32HandlePropertiesKHR(*m_device, handle_type, less_common_invalid_win32_handle, &properties);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetMemoryWin32HandlePropertiesKHR-handleType-00666");
    vk::GetMemoryWin32HandlePropertiesKHR(*m_device, opaque_handle_type, handle_that_passes_validation, &properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImportMemoryWin32ImageNoDedicated) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    auto image_info = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    const auto handle_types = FindSupportedExternalMemoryHandleTypes(
        Gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT);
    if ((handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT) == 0) {
        GTEST_SKIP() << "VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT is not both exportable and importable";
    }

    VkExternalMemoryImageCreateInfo external_image_info = vku::InitStructHelper();
    external_image_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    image_info.pNext = &external_image_info;

    vkt::Image image(*m_device, image_info, vkt::no_mem);

    VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image.MemoryRequirements(), 0, &export_info);

    vkt::DeviceMemory memory_export;
    memory_export.init(*m_device, alloc_info);

    VkMemoryGetWin32HandleInfoKHR get_handle_info = vku::InitStructHelper();
    get_handle_info.memory = memory_export.handle();
    get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    HANDLE handle = NULL;
    vk::GetMemoryWin32HandleKHR(device(), &get_handle_info, &handle);

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = image;

    VkImportMemoryWin32HandleInfoKHR import_info = vku::InitStructHelper(&dedicated_info);
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    import_info.handle = handle;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfo-image-01876");
    alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();

    // "For handle types defined as NT handles, the handles returned by vkGetFenceWin32HandleKHR are owned by the application. To
    // avoid leaking resources, the application must release ownership of them using the CloseHandle system call when they are no
    // longer needed."
    ::CloseHandle(handle);
}

TEST_F(NegativeExternalMemorySync, ImportMemoryWin32BufferDifferentDedicated) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    auto buffer_info = vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    const auto handle_types = FindSupportedExternalMemoryHandleTypes(
        Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT);
    if ((handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT) == 0) {
        GTEST_SKIP() << "VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT is not both exportable and importable";
    }

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    external_buffer_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    buffer_info.pNext = &external_buffer_info;

    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer.handle();

    VkExportMemoryAllocateInfo export_info = vku::InitStructHelper(&dedicated_info);
    export_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0, &export_info);

    vkt::DeviceMemory memory_export;
    memory_export.init(*m_device, alloc_info);

    VkMemoryGetWin32HandleInfoKHR get_handle_info = vku::InitStructHelper();
    get_handle_info.memory = memory_export.handle();
    get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    HANDLE handle = NULL;
    vk::GetMemoryWin32HandleKHR(device(), &get_handle_info, &handle);

    buffer_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkt::Buffer buffer2(*m_device, buffer_info, vkt::no_mem);
    dedicated_info.buffer = buffer2.handle();

    VkImportMemoryWin32HandleInfoKHR import_info = vku::InitStructHelper(&dedicated_info);
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    import_info.handle = handle;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfo-buffer-01877");
    alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer2.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();

    // "For handle types defined as NT handles, the handles returned by vkGetFenceWin32HandleKHR are owned by the application. To
    // avoid leaking resources, the application must release ownership of them using the CloseHandle system call when they are no
    // longer needed."
    ::CloseHandle(handle);
}

TEST_F(NegativeExternalMemorySync, ImportMemoryWin32BufferSupport) {
    TEST_DESCRIPTION("Memory imported from handle type that is not reported as importable by VkExternalBufferProperties.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    constexpr auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
    auto buffer_ci = vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkPhysicalDeviceExternalBufferInfo external_info = vku::InitStructHelper();
    external_info.flags = buffer_ci.flags;
    external_info.usage = buffer_ci.usage;
    external_info.handleType = handle_type;
    VkExternalBufferProperties external_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalBufferProperties(Gpu(), &external_info, &external_properties);

    if ((external_properties.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0) {
        GTEST_SKIP() << "Need buffer with no import support";
    }

    // allocate memory and export payload as win32 handle
    VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = handle_type;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&export_info);
    alloc_info.allocationSize = 4096;
    alloc_info.memoryTypeIndex = 0;
    vkt::DeviceMemory memory;
    memory.init(*m_device, alloc_info);

    HANDLE handle = NULL;
    VkMemoryGetWin32HandleInfoKHR get_handle_info = vku::InitStructHelper();
    get_handle_info.memory = memory;
    get_handle_info.handleType = handle_type;
    vk::GetMemoryWin32HandleKHR(*m_device, &get_handle_info, &handle);

    // create memory object by importing payload through win32 handle
    VkImportMemoryWin32HandleInfoKHR import_info = vku::InitStructHelper();
    import_info.handleType = handle_type;
    import_info.handle = handle;
    VkMemoryAllocateInfo alloc_info_with_import = vku::InitStructHelper(&import_info);
    alloc_info_with_import.allocationSize = 4096;
    alloc_info_with_import.memoryTypeIndex = 0;
    vkt::DeviceMemory imported_memory;
    imported_memory.init(*m_device, alloc_info_with_import);

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    external_buffer_info.handleTypes = handle_type;
    buffer_ci.pNext = &external_buffer_info;

    // It's hard to get configuration with current Windows drivers for this test.
    // Protected memory feature is not supported that is used on Linux to get non-importable memory.
    // D3D11_TEXTURE handle type does job that is does not report import capability but
    // also it's not supported for this usage, so disable that error message and sacrifice some
    // correctness with a purpose to reach code path we need to test.
    // NOTE: the approach with D3D11_TEXTURE does not work for images, that's why no similar test for images
    m_errorMonitor->SetUnexpectedError("VUID-VkBufferCreateInfo-pNext-00920");
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    m_errorMonitor->SetDesiredError("VUID-VkImportMemoryWin32HandleInfoKHR-handleType-00658");
    buffer.BindMemory(imported_memory, 0);
    m_errorMonitor->VerifyFound();
    ::CloseHandle(handle);
}
#endif

TEST_F(NegativeExternalMemorySync, FdMemoryHandleProperties) {
    TEST_DESCRIPTION("Call vkGetMemoryFdPropertiesKHR with invalid fd handle or with opaque handle type");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    constexpr auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
    constexpr auto opaque_handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    constexpr int invalid_fd_handle = -1;
    constexpr int valid_fd_handle = 0;

    VkMemoryFdPropertiesKHR properties = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-vkGetMemoryFdPropertiesKHR-fd-00673");
    vk::GetMemoryFdPropertiesKHR(*m_device, handle_type, invalid_fd_handle, &properties);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetMemoryFdPropertiesKHR-handleType-00674");
    vk::GetMemoryFdPropertiesKHR(*m_device, opaque_handle_type, valid_fd_handle, &properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImportMemoryFdBufferNoDedicated) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    external_buffer_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    auto buffer_info = vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT, {}, &external_buffer_info);
    if (!FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT)) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    if (!FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "Unable to find importable handle type";
    }
    const auto compatible_types = GetCompatibleHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
    if ((VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT & compatible_types) == 0) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }

    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0, &export_info);

    vkt::DeviceMemory memory_export;
    memory_export.init(*m_device, alloc_info);

    VkMemoryGetFdInfoKHR mgfi = vku::InitStructHelper();
    mgfi.memory = memory_export.handle();
    mgfi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    int fd;
    vk::GetMemoryFdKHR(device(), &mgfi, &fd);
    if (fd < 0) {
        GTEST_SKIP() << "Cannot export FD memory";
    }

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = VK_NULL_HANDLE;
    dedicated_info.buffer = buffer.handle();

    VkImportMemoryFdInfoKHR import_info = vku::InitStructHelper(&dedicated_info);
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    import_info.fd = fd;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfo-buffer-01879");
    alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImportMemoryFdBufferDifferentDedicated) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkExternalMemoryBufferCreateInfo external_buffer_info = vku::InitStructHelper();
    external_buffer_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    auto buffer_info = vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT, {}, &external_buffer_info);
    if (!FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT)) {
        GTEST_SKIP() << "Unable to find exportable handle type";
    }
    if (!FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "Unable to find importable handle type";
    }
    const auto compatible_types = GetCompatibleHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
    if ((VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT & compatible_types) == 0) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }

    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = VK_NULL_HANDLE;
    dedicated_info.buffer = buffer.handle();

    VkExportMemoryAllocateInfo export_info = vku::InitStructHelper(&dedicated_info);
    export_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0, &export_info);

    vkt::DeviceMemory memory_export;
    memory_export.init(*m_device, alloc_info);

    VkMemoryGetFdInfoKHR mgfi = vku::InitStructHelper();
    mgfi.memory = memory_export.handle();
    mgfi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    int fd;
    vk::GetMemoryFdKHR(device(), &mgfi, &fd);

    buffer_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkt::Buffer buffer2(*m_device, buffer_info, vkt::no_mem);

    dedicated_info.buffer = buffer2.handle();

    VkImportMemoryFdInfoKHR import_info = vku::InitStructHelper(&dedicated_info);
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    import_info.fd = fd;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfo-buffer-01879");
    alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer2.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImportMemoryFdBadFd) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    auto buffer_info = vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (!FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "Unable to find importable handle type";
    }
    const auto compatible_types = GetCompatibleHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
    if ((VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT & compatible_types) == 0) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }

    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkImportMemoryFdInfoKHR import_info = vku::InitStructHelper();
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    import_info.fd = -1;  // invalid

    m_errorMonitor->SetDesiredError("VUID-VkImportMemoryFdInfoKHR-handleType-00670");
    VkMemoryAllocateInfo alloc_info =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImportMemoryFdHandleType) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    auto buffer_info = vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (!FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "Unable to find importable handle type";
    }
    const auto compatible_types = GetCompatibleHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
    if ((VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT & compatible_types) == 0) {
        GTEST_SKIP() << "Cannot find handle types that are supported but not compatible with each other";
    }

    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkImportMemoryFdInfoKHR import_info = vku::InitStructHelper();
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    import_info.fd = 1;

    m_errorMonitor->SetDesiredError("VUID-VkImportMemoryFdInfoKHR-handleType-00669");
    VkMemoryAllocateInfo alloc_info =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImportMemoryFdBufferSupport) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    auto buffer_info = vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    if (FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "Need buffer with no import support";
    }

    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = VK_NULL_HANDLE;
    dedicated_info.buffer = buffer.handle();

    VkImportMemoryFdInfoKHR import_info = vku::InitStructHelper(&dedicated_info);
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    import_info.fd = 1;

    m_errorMonitor->SetDesiredError("VUID-VkImportMemoryFdInfoKHR-handleType-00667");
    VkMemoryAllocateInfo alloc_info =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeExternalMemorySync, ImportMemoryFdImageSupport) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    auto image_info = vkt::Image::CreateInfo();
    image_info.extent.width = 1024;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, image_info, vkt::no_mem);

    if (FindSupportedExternalMemoryHandleTypes(Gpu(), image_info, VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "Need image with no import support";
    }

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.image = image.handle();
    dedicated_info.buffer = VK_NULL_HANDLE;

    VkImportMemoryFdInfoKHR import_info = vku::InitStructHelper(&dedicated_info);
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    import_info.fd = 1;

    m_errorMonitor->SetDesiredError("VUID-VkImportMemoryFdInfoKHR-handleType-00667");
    VkMemoryAllocateInfo alloc_info =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();
}

// Because of aligned_alloc
#if defined(__linux__) && !defined(__ANDROID__)
TEST_F(NegativeExternalMemorySync, GetMemoryHostHandleType) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceExternalMemoryHostPropertiesEXT memory_host_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(memory_host_props);

    VkDeviceSize alloc_size = memory_host_props.minImportedHostPointerAlignment;
    void *host_memory = aligned_alloc(alloc_size, alloc_size);
    if (!host_memory) {
        GTEST_SKIP() << "Can't allocate host memory";
    }

    VkMemoryHostPointerPropertiesEXT host_pointer_props = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-vkGetMemoryHostPointerPropertiesEXT-handleType-01752");
    vk::GetMemoryHostPointerPropertiesEXT(*m_device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT, host_memory,
                                          &host_pointer_props);
    m_errorMonitor->VerifyFound();
    free(host_memory);
}

TEST_F(NegativeExternalMemorySync, GetMemoryHostAlignment) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceExternalMemoryHostPropertiesEXT memory_host_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(memory_host_props);

    VkDeviceSize alloc_size = memory_host_props.minImportedHostPointerAlignment;
    VkDeviceSize bad_alloc_size = alloc_size / 4;
    void *host_memory = aligned_alloc(bad_alloc_size, alloc_size);
    if (!host_memory) {
        GTEST_SKIP() << "Can't allocate host memory";
    }
    const VkDeviceSize host_pointer = reinterpret_cast<VkDeviceSize>(host_memory);
    if (host_pointer % alloc_size == 0) {
        free(host_memory);
        GTEST_SKIP() << "Can't create misaligned memory";  // when using ASAN
    }
    VkMemoryHostPointerPropertiesEXT host_pointer_props = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-vkGetMemoryHostPointerPropertiesEXT-pHostPointer-01753");
    vk::GetMemoryHostPointerPropertiesEXT(*m_device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT, host_memory,
                                          &host_pointer_props);
    m_errorMonitor->VerifyFound();
    free(host_memory);
}

TEST_F(NegativeExternalMemorySync, ImportMemoryHostDedicated) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkPhysicalDeviceExternalMemoryHostPropertiesEXT memory_host_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(memory_host_props);

    VkDeviceSize alloc_size = memory_host_props.minImportedHostPointerAlignment;
    void *host_memory = aligned_alloc(alloc_size, alloc_size);
    if (!host_memory) {
        GTEST_SKIP() << "Can't allocate host memory";
    }

    VkMemoryHostPointerPropertiesEXT host_pointer_props = vku::InitStructHelper();
    vk::GetMemoryHostPointerPropertiesEXT(*m_device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT, host_memory,
                                          &host_pointer_props);

    const auto buffer_info = vkt::Buffer::CreateInfo(alloc_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer.handle();
    dedicated_info.image = VK_NULL_HANDLE;

    VkImportMemoryHostPointerInfoEXT import_info = vku::InitStructHelper(&dedicated_info);
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    import_info.pHostPointer = host_memory;

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&import_info);
    alloc_info.allocationSize = alloc_size;
    if (!m_device->Physical().SetMemoryType(host_pointer_props.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        free(host_memory);
        GTEST_SKIP() << "Failed to set memory type.";
    }

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-02806");
    VkDeviceMemory device_memory;
    vk::AllocateMemory(*m_device, &alloc_info, nullptr, &device_memory);
    m_errorMonitor->VerifyFound();

    free(host_memory);
}

TEST_F(NegativeExternalMemorySync, ImportMemoryHostMemoryIndex) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    VkPhysicalDeviceExternalMemoryHostPropertiesEXT memory_host_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(memory_host_props);

    VkDeviceSize alloc_size = memory_host_props.minImportedHostPointerAlignment;
    void *host_memory = aligned_alloc(alloc_size, alloc_size);
    if (!host_memory) {
        GTEST_SKIP() << "Can't allocate host memory";
    }

    VkMemoryHostPointerPropertiesEXT host_pointer_props = vku::InitStructHelper();
    vk::GetMemoryHostPointerPropertiesEXT(*m_device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT, host_memory,
                                          &host_pointer_props);

    VkImportMemoryHostPointerInfoEXT import_info = vku::InitStructHelper();
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    import_info.pHostPointer = host_memory;

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&import_info);
    alloc_info.allocationSize = alloc_size;

    uint32_t unsupported_mem_type =
        ((1 << m_device->Physical().memory_properties_.memoryTypeCount) - 1) & ~host_pointer_props.memoryTypeBits;
    bool found_type = m_device->Physical().SetMemoryType(unsupported_mem_type, &alloc_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (unsupported_mem_type == 0 || !found_type) {
        free(host_memory);
        GTEST_SKIP() << "Failed to find unsupported memory type.";
    }
    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-memoryTypeIndex-01744");
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();

    free(host_memory);
}
#endif

#ifdef VK_USE_PLATFORM_METAL_EXT
TEST_F(NegativeExternalMemorySync, ExportMetalObjects) {
    TEST_DESCRIPTION("Test VK_EXT_metal_objects VUIDs");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_METAL_OBJECTS_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework());
    const bool ycbcr_conversion_extension = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_features);

    if (ycbcr_conversion_extension) {
        VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features = vku::InitStructHelper();
        ycbcr_features.samplerYcbcrConversion = VK_TRUE;
        portability_features.pNext = &ycbcr_features;
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    IgnoreHandleTypeError(m_errorMonitor);

    VkExportMetalObjectCreateInfoEXT metal_object_create_info = vku::InitStructHelper();
    auto instance_ci = GetInstanceCreateInfo();
    metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT;
    metal_object_create_info.pNext = instance_ci.pNext;
    instance_ci.pNext = &metal_object_create_info;

    VkInstance instance = {};
    m_errorMonitor->SetDesiredError("VUID-VkInstanceCreateInfo-pNext-06779");
    vk::CreateInstance(&instance_ci, nullptr, &instance);
    m_errorMonitor->VerifyFound();
    metal_object_create_info.pNext = nullptr;

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&metal_object_create_info);
    alloc_info.allocationSize = 1024;
    VkDeviceMemory memory;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-06780");
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory);
    m_errorMonitor->VerifyFound();

    VkImageCreateInfo ici = vku::InitStructHelper();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    ici.extent = {128, 128, 1};
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    ici.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ici.pNext = &metal_object_create_info;
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-06783");

    VkImportMetalTextureInfoEXT import_metal_texture_info = vku::InitStructHelper();
    import_metal_texture_info.plane = VK_IMAGE_ASPECT_COLOR_BIT;
    ici.pNext = &import_metal_texture_info;
    ici.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-06784");

    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    import_metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_1_BIT;
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-06785");

    ici.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    import_metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_2_BIT;
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-06786");

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

    vkt::Buffer buffer(*m_device, buffer_create_info);
    VkBufferViewCreateInfo buff_view_ci = vku::InitStructHelper();
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    buff_view_ci.range = VK_WHOLE_SIZE;
    buff_view_ci.pNext = &metal_object_create_info;
    CreateBufferViewTest(*this, &buff_view_ci, {"VUID-VkBufferViewCreateInfo-pNext-06782"});

    vkt::Image image_obj(*m_device, 256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image_obj.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.pNext = &metal_object_create_info;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-pNext-06787");

    VkSemaphoreCreateInfo sem_info = vku::InitStructHelper();
    sem_info.pNext = &metal_object_create_info;
    VkSemaphore semaphore;
    metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreCreateInfo-pNext-06789");
    vk::CreateSemaphore(device(), &sem_info, NULL, &semaphore);
    m_errorMonitor->VerifyFound();

    VkEventCreateInfo event_info = vku::InitStructHelper();
    if (portability_features.events) {
        event_info.pNext = &metal_object_create_info;
        VkEvent event;
        m_errorMonitor->SetDesiredError("VUID-VkEventCreateInfo-pNext-06790");
        vk::CreateEvent(device(), &event_info, nullptr, &event);
        m_errorMonitor->VerifyFound();
    }

    VkExportMetalObjectsInfoEXT export_metal_objects_info = vku::InitStructHelper();
    VkExportMetalDeviceInfoEXT metal_device_info = vku::InitStructHelper();
    VkExportMetalCommandQueueInfoEXT metal_command_queue_info = vku::InitStructHelper();
    metal_command_queue_info.queue = m_default_queue->handle();
    export_metal_objects_info.pNext = &metal_device_info;
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06791");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    export_metal_objects_info.pNext = &metal_command_queue_info;
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06792");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    alloc_info.pNext = nullptr;
    VkResult err = vk::AllocateMemory(device(), &alloc_info, nullptr, &memory);
    ASSERT_EQ(VK_SUCCESS, err);
    VkExportMetalBufferInfoEXT metal_buffer_info = vku::InitStructHelper();
    metal_buffer_info.memory = memory;
    export_metal_objects_info.pNext = &metal_buffer_info;
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06793");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();
    vk::FreeMemory(device(), memory, nullptr);

    VkExportMetalObjectCreateInfoEXT export_metal_object_create_info = vku::InitStructHelper();
    export_metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT;
    ici.pNext = &export_metal_object_create_info;
    vkt::Image export_image_obj(*m_device, ici);
    vkt::BufferView export_buffer_view;
    buff_view_ci.pNext = &export_metal_object_create_info;
    export_buffer_view.init(*m_device, buff_view_ci);
    VkExportMetalTextureInfoEXT metal_texture_info = vku::InitStructHelper();
    metal_texture_info.bufferView = export_buffer_view.handle();
    metal_texture_info.image = export_image_obj.handle();
    metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_0_BIT;
    export_metal_objects_info.pNext = &metal_texture_info;

    // Only one of image, bufferView, imageView
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06794");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    // Image not created with struct in pNext
    metal_texture_info.bufferView = VK_NULL_HANDLE;
    metal_texture_info.image = image_obj.handle();
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06795");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    metal_texture_info.image = VK_NULL_HANDLE;
    auto image_view_ci = image_obj.BasicViewCreatInfo();
    vkt::ImageView image_view_no_struct(*m_device, image_view_ci);
    metal_texture_info.imageView = image_view_no_struct.handle();
    // ImageView not created with struct in pNext
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06796");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    buff_view_ci.pNext = nullptr;
    vkt::BufferView buffer_view_no_struct;
    buffer_view_no_struct.init(*m_device, buff_view_ci);
    metal_texture_info.imageView = VK_NULL_HANDLE;
    metal_texture_info.bufferView = buffer_view_no_struct.handle();
    // BufferView not created with struct in pNext
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06797");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    metal_texture_info.bufferView = VK_NULL_HANDLE;
    metal_texture_info.image = export_image_obj.handle();
    metal_texture_info.plane = VK_IMAGE_ASPECT_COLOR_BIT;
    // metal_texture_info.plane not plane 0, 1 or 2
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06798");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    vkt::Image single_plane_export_image_obj(*m_device, ici);
    metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_1_BIT;
    metal_texture_info.image = single_plane_export_image_obj.handle();
    // metal_texture_info.plane not plane_0 for single plane image
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06799");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    image_view_ci.pNext = &export_metal_object_create_info;
    export_metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT;
    vkt::ImageView single_plane_export_image_view(*m_device, image_view_ci);
    metal_texture_info.image = VK_NULL_HANDLE;
    metal_texture_info.imageView = single_plane_export_image_view.handle();
    // metal_texture_info.plane not plane_0 for single plane imageView
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06801");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    VkExportMetalIOSurfaceInfoEXT metal_iosurface_info = vku::InitStructHelper();
    metal_iosurface_info.image = image_obj.handle();
    export_metal_objects_info.pNext = &metal_iosurface_info;
    // metal_iosurface_info.image not created with struct in pNext
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06803");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    VkExportMetalSharedEventInfoEXT metal_shared_event_info = vku::InitStructHelper();
    export_metal_objects_info.pNext = &metal_shared_event_info;
    // metal_shared_event_info event and semaphore both VK_NULL_HANDLE
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06804");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    sem_info.pNext = nullptr;
    vkt::Semaphore semaphore_no_struct;
    semaphore_no_struct.init(*m_device, sem_info);
    metal_shared_event_info.semaphore = semaphore_no_struct.handle();
    export_metal_objects_info.pNext = &metal_shared_event_info;
    // Semaphore not created with struct in pNext
    m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06805");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    if (portability_features.events) {
        event_info.pNext = nullptr;
        vkt::Event event_no_struct(*m_device, event_info);
        metal_shared_event_info.event = event_no_struct.handle();
        metal_shared_event_info.semaphore = VK_NULL_HANDLE;
        // Event not created with struct in pNext
        m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06806");
        vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
        m_errorMonitor->VerifyFound();
    }

    const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    if (FormatIsSupported(Gpu(), mp_format)) {
        export_metal_object_create_info = vku::InitStructHelper();
        export_metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT;
        ici.format = mp_format;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        ici.pNext = &export_metal_object_create_info;
        vkt::Image mp_image_obj(*m_device, ici);

        metal_texture_info.bufferView = VK_NULL_HANDLE;
        metal_texture_info.imageView = VK_NULL_HANDLE;
        metal_texture_info.image = mp_image_obj.handle();
        metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_2_BIT;
        export_metal_objects_info.pNext = &metal_texture_info;
        m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06800");
        vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
        m_errorMonitor->VerifyFound();

        if (ycbcr_conversion_extension) {
            VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = vku::InitStructHelper();
            ycbcr_create_info.format = mp_format;
            ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
            ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
            ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
            ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
            ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
            ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
            ycbcr_create_info.forceExplicitReconstruction = false;

            VkSamplerYcbcrConversion conversion;
            err = vk::CreateSamplerYcbcrConversionKHR(device(), &ycbcr_create_info, nullptr, &conversion);
            ASSERT_EQ(VK_SUCCESS, err);

            VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
            ycbcr_info.conversion = conversion;
            ycbcr_info.pNext = &export_metal_object_create_info;
            ivci.image = mp_image_obj.handle();
            ivci.format = mp_format;
            ivci.pNext = &ycbcr_info;
            vkt::ImageView mp_image_view(*m_device, ivci);
            metal_texture_info.image = VK_NULL_HANDLE;
            metal_texture_info.imageView = mp_image_view.handle();
            m_errorMonitor->SetDesiredError("VUID-VkExportMetalObjectsInfoEXT-pNext-06802");
            vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
            m_errorMonitor->VerifyFound();
            vk::DestroySamplerYcbcrConversionKHR(device(), conversion, nullptr);
        }
    }
}
#endif  // VK_USE_PLATFORM_METAL_EXT
