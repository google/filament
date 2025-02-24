/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/android_hardware_buffer.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)

class NegativeAndroidHardwareBuffer : public VkLayerTest {};

TEST_F(NegativeAndroidHardwareBuffer, ImageCreate) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer image create info.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkImage img = VK_NULL_HANDLE;

    VkImageCreateInfo ici = vku::InitStructHelper();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    // undefined format
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-01975");
    // Various extra errors for having VK_FORMAT_UNDEFINED without VkExternalFormatANDROID
    m_errorMonitor->SetUnexpectedError("VUID_Undefined");
    m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();

    // also undefined format
    VkExternalFormatANDROID efa = vku::InitStructHelper();
    efa.externalFormat = 0;
    ici.pNext = &efa;
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-01975");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();

    // undefined format with an unknown external format
    efa.externalFormat = 0xBADC0DE;
    m_errorMonitor->SetDesiredError("VUID-VkExternalFormatANDROID-externalFormat-01894");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);

    // Retrieve it's properties to make it's external format 'known' (AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM)
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    // a defined image format with a non-zero external format
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    efa.externalFormat = ahb_fmt_props.externalFormat;
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-01974");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    ici.format = VK_FORMAT_UNDEFINED;

    // external format while MUTABLE
    ici.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-02396");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    ici.flags = 0;

    // external format while usage other than SAMPLED
    ici.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-02397");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();

    ici.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-09457");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    // external format while tiline other than OPTIMAL
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-02398");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;

    // imageType
    VkExternalMemoryImageCreateInfo emici = vku::InitStructHelper();
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    ici.pNext = &emici;  // remove efa from chain, insert emici
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.imageType = VK_IMAGE_TYPE_3D;
    ici.extent = {64, 64, 64};

    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-02393");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();

    // wrong mipLevels
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.extent = {64, 64, 1};
    ici.mipLevels = 6;  // should be 7
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-02394");
    vk::CreateImage(device(), &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, FetchUnboundImageInfo) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer retreive image properties while memory unbound.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkExternalMemoryImageCreateInfo emici = vku::InitStructHelper();
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImageCreateInfo ici = vku::InitStructHelper(&emici);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, ici, vkt::no_mem);

    // attempt to fetch layout from unbound image
    VkImageSubresource sub_rsrc = {};
    sub_rsrc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSubresourceLayout sub_layout = {};
    m_errorMonitor->SetDesiredError("VUID-vkGetImageSubresourceLayout-image-09432");
    vk::GetImageSubresourceLayout(device(), image.handle(), &sub_rsrc, &sub_layout);
    m_errorMonitor->VerifyFound();

    // attempt to get memory reqs from unbound image
    VkImageMemoryRequirementsInfo2 imri = vku::InitStructHelper();
    imri.image = image.handle();
    VkMemoryRequirements2 mem_reqs = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryRequirementsInfo2-image-01897");
    vk::GetImageMemoryRequirements2(device(), &imri, &mem_reqs);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, GpuDataBuffer) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer missing USAGE_GPU_DATA_BUFFER.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = vku::InitStructHelper();
    import_ahb_Info.buffer = ahb.handle();

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    // Import requires format AHB_FMT_BLOB and usage AHB_USAGE_GPU_DATA_BUFFER
    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-02384");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, AllocationSize) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer correct allocationSize is used.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_BLOB, AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER, 64);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = vku::InitStructHelper();
    import_ahb_Info.buffer = ahb.handle();

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    // Allocation size mismatch
    {
        memory_allocate_info.allocationSize = ahb_props.allocationSize + 1;
        m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-allocationSize-02383");
        vkt::DeviceMemory memory(*m_device, memory_allocate_info);
        m_errorMonitor->VerifyFound();
    }

    // memoryTypeIndex mismatch
    {
        memory_allocate_info.allocationSize = ahb_props.allocationSize;
        memory_allocate_info.memoryTypeIndex++;
#if defined(VVL_MOCK_ANDROID)
        m_errorMonitor->SetUnexpectedError("VUID-vkAllocateMemory-pAllocateInfo-01714");  // incase at last index
#endif
        m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385");
        vkt::DeviceMemory memory(*m_device, memory_allocate_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeAndroidHardwareBuffer, DedicatedUsageColor) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer correct usage for dedicated allocated color image.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER, 64, 64);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = vku::InitStructHelper(&external_format);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, ici, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = vku::InitStructHelper();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        vku::InitStructHelper(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-02390");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, DedicatedUsageDS) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer correct usage for dedicated allocated depth/stencil image.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_S8_UINT, AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "Failed to Allocate AHB";
    }

    // Incase it hits the below driver bug, catch the false VUID error thrown from driver not creating valid AHB
    m_errorMonitor->SetUnexpectedError("VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    if (ahb_fmt_props.format != VK_FORMAT_S8_UINT) {
        GTEST_SKIP() << "Driver bug: Didn't turn AHB format into VK_FORMAT_S8_UINT";
    }

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = vku::InitStructHelper(&external_format);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, ici, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = vku::InitStructHelper();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        vku::InitStructHelper(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-02390");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, MipmapChainComplete) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer correct mipmap chain.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    vkt::AHB ahb(&ahb_desc);
    if (!ahb.handle()) {
        // ERROR: AHardwareBuffer_allocate() with MIPMAP_COMPLETE fails. It returns -12, NO_MEMORY.
        // The problem seems to happen in Pixel 2, not Pixel 3.
        GTEST_SKIP() << "AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE not supported";
    }

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkImageCreateInfo ici = vku::InitStructHelper();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 2;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, ici, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = vku::InitStructHelper();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        vku::InitStructHelper(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-02389");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, NoMipmapChain) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer correct mipmap chain.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    vkt::AHB ahb(&ahb_desc);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkImageCreateInfo ici = vku::InitStructHelper();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 2;  // not 1
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, ici, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = vku::InitStructHelper();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = vku::InitStructHelper(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-02586");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, ImageDimensions) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer dimension and VkImage match.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 128;
    ahb_desc.height = 32;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    vkt::AHB ahb(&ahb_desc);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = vku::InitStructHelper(&external_format);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, ici, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = vku::InitStructHelper();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        vku::InitStructHelper(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-02388");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, UnknownFormat) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer uses VK_FORMAT_UNDEFINED for external.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkImageCreateInfo ici = vku::InitStructHelper();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, ici, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = vku::InitStructHelper();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        vku::InitStructHelper(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-02387");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, GpuUsage) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer has a GPU usage flag.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "Was unable to allocate an AHB";
    }

    // Everything from ahb_props is garbage and not usable
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    m_errorMonitor->SetDesiredError("VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);
    m_errorMonitor->VerifyFound();

    // Since we are creating a invalid AHB for the safe of getting the below AHB, there is a chance the driver will not be forgiving
    // and still give an usable AHB
    {
        AHardwareBuffer_Desc ahb_desc_check = {};
        AHardwareBuffer_describe(ahb.handle(), &ahb_desc_check);
        if (ahb_desc_check.usage == 0) {
            GTEST_SKIP() << "Was unable to create a valid AHB to be used";
        }
    }

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = 0;

    VkImageCreateInfo ici = vku::InitStructHelper(&external_format);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, ici, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = vku::InitStructHelper();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        vku::InitStructHelper(&dedicated_allocation_info);
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);

    // Dedicated allocation with missing usage bits
    // Setting up this test also triggers a slew of others
    memory_allocate_info.allocationSize = 4096;
    memory_allocate_info.memoryTypeIndex = 0;
    m_errorMonitor->SetUnexpectedError("VUID-VkMemoryAllocateInfo-pNext-02390");
    m_errorMonitor->SetUnexpectedError("VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385");
    m_errorMonitor->SetUnexpectedError("VUID-VkMemoryAllocateInfo-allocationSize-02383");
    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-02386");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, ExportMemoryAllocateImage) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer VkExportMemoryAllocateInfo instead of import.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = vku::InitStructHelper(&external_format);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, ici, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = vku::InitStructHelper();
    dedicated_allocation_info.image = image.handle();
    dedicated_allocation_info.buffer = VK_NULL_HANDLE;

    // Non-import allocation - replace import struct in chain with export struct
    VkExportMemoryAllocateInfo export_memory = vku::InitStructHelper(&dedicated_allocation_info);
    export_memory.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&export_memory);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-01874");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, ExportMemoryAllocateBuffer) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer VkExportMemoryAllocateInfo instead of import.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkExternalMemoryBufferCreateInfo ext_buf_info = vku::InitStructHelper();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&ext_buf_info);
    buffer_create_info.size = 4096;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_allocation_info = vku::InitStructHelper();
    dedicated_allocation_info.image = VK_NULL_HANDLE;
    dedicated_allocation_info.buffer = buffer.handle();

    // Non-import allocation - replace import struct in chain with export struct
    VkExportMemoryAllocateInfo export_memory = vku::InitStructHelper(&dedicated_allocation_info);
    export_memory.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&export_memory);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    memory_allocate_info.allocationSize = 0;
    m_errorMonitor->SetUnexpectedError("VUID-VkMemoryDedicatedAllocateInfo-buffer-02965");
    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-07901");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, CreateYCbCrSampler) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer YCbCr sampler creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(Init());

    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper();
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;

    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-format-04061");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    VkExternalFormatANDROID efa = vku::InitStructHelper();
    efa.externalFormat = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    sycci.format = VK_FORMAT_R8G8B8A8_UNORM;
    sycci.pNext = &efa;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-format-01904");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    efa.externalFormat = AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420;
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    // Spec says if we use VkExternalFormatANDROID value of components is ignored.
    sycci.components = {VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};
    vkt::SamplerYcbcrConversion conversion(*m_device, sycci);
}

TEST_F(NegativeAndroidHardwareBuffer, PhysDevImageFormatProp2) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer GetPhysicalDeviceImageFormatProperties.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkImageFormatProperties2 ifp = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 pdifi = vku::InitStructHelper();
    pdifi.format = VK_FORMAT_R8G8B8A8_UNORM;
    pdifi.tiling = VK_IMAGE_TILING_OPTIMAL;
    pdifi.type = VK_IMAGE_TYPE_2D;
    pdifi.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkAndroidHardwareBufferUsageANDROID ahbu = vku::InitStructHelper();
    ahbu.androidHardwareBufferUsage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ifp.pNext = &ahbu;

    // AHB_usage chained to input without a matching external image format struc chained to output
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868");
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &pdifi, &ifp);
    m_errorMonitor->VerifyFound();

    // output struct chained, but does not include VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID usage
    VkPhysicalDeviceExternalImageFormatInfo pdeifi = vku::InitStructHelper();
    pdeifi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    pdifi.pNext = &pdeifi;
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868");
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &pdifi, &ifp);
    m_errorMonitor->VerifyFound();
}

#if DISABLEUNTILAHBWORKS
TEST_F(NegativeAndroidHardwareBuffer, CreateImageView) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer image view creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Allocate an AHB and fetch its properties
    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Retrieve AHB properties to make it's external format 'known'
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb, &ahb_props);
    AHardwareBuffer_release(ahb);

    VkExternalMemoryImageCreateInfo emici = vku::InitStructHelper();
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    // Give image an external format
    VkExternalFormatANDROID efa = vku::InitStructHelper(&emici);
    efa.externalFormat = ahb_fmt_props.externalFormat;

    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Create another VkExternalFormatANDROID for test VUID-VkImageViewCreateInfo-image-02400
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props_Ycbcr =
        vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props_Ycbcr =
        vku::InitStructHelper(&ahb_fmt_props_Ycbcr);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb, &ahb_props_Ycbcr);
    AHardwareBuffer_release(ahb);

    VkExternalFormatANDROID efa_Ycbcr = vku::InitStructHelper();
    efa_Ycbcr.externalFormat = ahb_fmt_props_Ycbcr.externalFormat;

    // Need to make sure format has sample bit needed for image usage
    if ((ahb_fmt_props_Ycbcr.formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT feature bit not supported";
    }

    // Create the image
    VkImage img = VK_NULL_HANDLE;
    VkImageCreateInfo ici = vku::InitStructHelper(&efa);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vk::CreateImage(device(), &ici, NULL, &img);

    // Set up memory allocation
    VkDeviceMemory img_mem = VK_NULL_HANDLE;
    VkMemoryAllocateInfo mai = vku::InitStructHelper();
    mai.allocationSize = 64 * 64 * 4;
    mai.memoryTypeIndex = 0;
    vk::AllocateMemory(device(), &mai, NULL, &img_mem);

    // It shouldn't use vk::GetImageMemoryRequirements for imported AndroidHardwareBuffer when memory isn't bound yet
    m_errorMonitor->SetDesiredError("VUID-vkGetImageMemoryRequirements-image-04004");
    VkMemoryRequirements img_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), img, &img_mem_reqs);
    m_errorMonitor->VerifyFound();
    vk::BindImageMemory(device(), img, img_mem, 0);

    // Bind image to memory
    vk::DestroyImage(device(), img, NULL);
    vk::FreeMemory(device(), img_mem, NULL);
    vk::CreateImage(device(), &ici, NULL, &img);
    vk::AllocateMemory(device(), &mai, NULL, &img_mem);
    vk::BindImageMemory(device(), img, img_mem, 0);

    // Create a YCbCr conversion, with different external format, chain to view
    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper(&efa_Ycbcr);
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    VkSamplerYcbcrConversionInfo syci = vku::InitStructHelper();
    syci.conversion = ycbcr_conv;

    // Create a view
    VkImageView image_view = VK_NULL_HANDLE;
    VkImageViewCreateInfo ivci = vku::InitStructHelper(&syci);
    ivci.image = img;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    auto reset_view = [&image_view, dev]() {
        if (VK_NULL_HANDLE != image_view) vk::DestroyImageView(dev, image_view, NULL);
        image_view = VK_NULL_HANDLE;
    };

    // Up to this point, no errors expected

    // Chained ycbcr conversion has different (external) format than image
    m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-02400");
    // Also causes "unsupported format" - should be removed in future spec update
    m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-None-02273");
    vk::CreateImageView(device(), &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    vk::DestroySamplerYcbcrConversion(device(), ycbcr_conv, NULL);
    sycci.pNext = &efa;
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    syci.conversion = ycbcr_conv;

    // View component swizzle not IDENTITY
    ivci.components.r = VK_COMPONENT_SWIZZLE_B;
    ivci.components.b = VK_COMPONENT_SWIZZLE_R;
    m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-02401");
    // Also causes "unsupported format" - should be removed in future spec update
    m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-None-02273");
    vk::CreateImageView(device(), &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;

    // View with external format, when format is not UNDEFINED
    ivci.format = VK_FORMAT_R5G6B5_UNORM_PACK16;
    m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-02399");
    // Also causes "view format different from image format"
    m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-01762");
    vk::CreateImageView(device(), &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    vk::DestroySamplerYcbcrConversion(device(), ycbcr_conv, NULL);
    vk::DestroyImageView(device(), image_view, NULL);
    vk::DestroyImage(device(), img, NULL);
    vk::FreeMemory(device(), img_mem, NULL);
}
#endif  // DISABLEUNTILAHBWORKS

TEST_F(NegativeAndroidHardwareBuffer, ImportBuffer) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer import as buffer.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // non USAGE_GPU_*
    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_BLOB, AHARDWAREBUFFER_USAGE_SENSOR_DIRECT_DATA, 512);

    m_errorMonitor->SetUnexpectedError("VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    // Create export and import buffers
    VkExternalMemoryBufferCreateInfo ext_buf_info = vku::InitStructHelper();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = vku::InitStructHelper();
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    // Import as buffer requires usage AHB_USAGE_GPU_DATA_BUFFER
    m_errorMonitor->SetDesiredError("VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01881");
    // Also causes "non-dedicated allocation format/usage" error
    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-02384");
    vkt::DeviceMemory mem_handle(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, ExportBufferHandleType) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer export memory as AHB has a valid handleType.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Allocate device memory, no linked export struct indicating AHB handle type
    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper();
    memory_allocate_info.allocationSize = 65536;
    memory_allocate_info.memoryTypeIndex = 0;
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);

    VkMemoryGetAndroidHardwareBufferInfoANDROID mgahbi = vku::InitStructHelper();
    mgahbi.memory = memory;
    AHardwareBuffer *ahb = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-handleTypes-01882");
    vk::GetMemoryAndroidHardwareBufferANDROID(device(), &mgahbi, &ahb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, ExportBufferAllocationSize) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Create VkBuffer to be exported to an AHB
    VkExternalMemoryBufferCreateInfo ext_buf_info = vku::InitStructHelper();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&ext_buf_info);
    buffer_create_info.size = 4096;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper();
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper(&export_memory_info);
    memory_info.allocationSize = 0;

    bool has_memtype =
        m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &memory_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (!has_memtype) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-07900");
    vkt::DeviceMemory memory(*m_device, memory_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, ExportImageNonBound) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer export memory as AHB has image bound already.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Create VkImage to be exported to an AHB
    VkExternalMemoryImageCreateInfo ext_image_info = vku::InitStructHelper();
    ext_image_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&ext_image_info);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {64, 1, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = vku::InitStructHelper();
    memory_dedicated_info.image = image.handle();
    memory_dedicated_info.buffer = VK_NULL_HANDLE;

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(&memory_dedicated_info);
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper(&export_memory_info);

    // "When allocating new memory for an image that can be exported to an Android hardware buffer, the memory’s allocationSize must
    // be zero":
    memory_info.allocationSize = 0;

    // Use any DEVICE_LOCAL memory found
    bool has_memtype = m_device->Physical().SetMemoryType(0xFFFFFFFF, &memory_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (!has_memtype) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }
    vkt::DeviceMemory memory(*m_device, memory_info);

    VkMemoryGetAndroidHardwareBufferInfoANDROID mgahbi = vku::InitStructHelper();
    mgahbi.memory = memory;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-pNext-01883");
    AHardwareBuffer *ahb = nullptr;
    vk::GetMemoryAndroidHardwareBufferANDROID(device(), &mgahbi, &ahb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, InvalidBindBufferMemory) {
    TEST_DESCRIPTION("Validate binding AndroidHardwareBuffer VkBuffer act same as non-AHB buffers.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_BLOB, AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER, 64);

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkExternalMemoryBufferCreateInfo ext_buf_info = vku::InitStructHelper();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&ext_buf_info);
    buffer_create_info.size = ahb_props.allocationSize;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    // Try to get memory requirements prior to binding memory
    VkBufferMemoryRequirementsInfo2 buffer_memory_requirements_info = vku::InitStructHelper();
    buffer_memory_requirements_info.buffer = buffer.handle();
    VkMemoryDedicatedRequirements memory_dedicated_requirements = vku::InitStructHelper();
    VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper(&memory_dedicated_requirements);
    vk::GetBufferMemoryRequirements2(device(), &buffer_memory_requirements_info, &mem_reqs2);
    VkMemoryRequirements mem_reqs = mem_reqs2.memoryRequirements;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = vku::InitStructHelper();
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    if (memory.handle() == VK_NULL_HANDLE) {
        GTEST_SKIP() << "This test failed to allocate memory for importing";
    }

    if (mem_reqs.alignment > 1) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkBindBufferMemory-buffer-01444");  // required dedicated
        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memoryOffset-01036");
        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-size-01037");
        vk::BindBufferMemory(device(), buffer.handle(), memory.handle(), 1);
        m_errorMonitor->VerifyFound();
    }

    VkDeviceSize buffer_offset = (mem_reqs.size - 1) & ~(mem_reqs.alignment - 1);
    if (buffer_offset > 0) {
        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-size-01037");
        if (memory_dedicated_requirements.requiresDedicatedAllocation) {
            m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-buffer-01444");
        }
        vk::BindBufferMemory(device(), buffer.handle(), memory.handle(), buffer_offset);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeAndroidHardwareBuffer, ImportBufferHandleType) {
    TEST_DESCRIPTION("Don't use proper resource handleType for import buffer");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_BLOB, AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER, 64);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = vku::InitStructHelper();
    import_ahb_Info.buffer = ahb.handle();

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    memory_allocate_info.allocationSize = ahb_props.allocationSize;
    // driver won't expose correct memoryType since resource was not created as an import operation
    // so just need any valid memory type returned from GetAHBInfo
    for (int i = 0; i < 32; i++) {
        if (ahb_props.memoryTypeBits & (1 << i)) {
            memory_allocate_info.memoryTypeIndex = i;
            break;
        }
    }

    vkt::DeviceMemory memory(*m_device, memory_allocate_info);

    // Create buffer without VkExternalMemoryBufferCreateInfo
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = ahb_props.allocationSize;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-02986");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindBufferMemory-memory-01035");
    vk::BindBufferMemory(device(), buffer.handle(), memory, 0);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper();
    bind_buffer_info.buffer = buffer.handle();
    bind_buffer_info.memory = memory;
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-memory-02986");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindBufferMemoryInfo-memory-01035");
    vk::BindBufferMemory2KHR(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, ImportImageHandleType) {
    TEST_DESCRIPTION("Don't use proper resource handleType for import image");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);

    // Create buffer without VkExternalMemoryImageCreateInfo
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = vku::InitStructHelper();
    memory_dedicated_info.image = image.handle();
    memory_dedicated_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        vku::InitStructHelper(&memory_dedicated_info);
    import_ahb_Info.buffer = ahb.handle();

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    memory_allocate_info.allocationSize = ahb_props.allocationSize;
    // driver won't expose correct memoryType since resource was not created as an import operation
    // so just need any valid memory type returned from GetAHBInfo
    for (int i = 0; i < 32; i++) {
        if (ahb_props.memoryTypeBits & (1 << i)) {
            memory_allocate_info.memoryTypeIndex = i;
            break;
        }
    }

    vkt::DeviceMemory memory(*m_device, memory_allocate_info);

    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-02990");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-memory-01047");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-size-01049");
    vk::BindImageMemory(device(), image.handle(), memory, 0);
    m_errorMonitor->VerifyFound();

    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper();
    bind_image_info.image = image.handle();
    bind_image_info.memory = memory;
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-memory-02990");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01617");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01615");
    vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, DeviceImageMemoryReq) {
    TEST_DESCRIPTION("Call vkGetDeviceImageMemoryRequirementsKHR with externalFormat of non-zero.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    // The spec says the driver must not return zero, even if a VkFormat is returned with it, some older drivers do as a driver bug
    if (ahb_fmt_props.externalFormat == 0) {
        GTEST_SKIP() << "externalFormat was zero which is not valid";
    }

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&external_format);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.arrayLayers = 1;
    image_create_info.extent = {64, 64, 1};
    image_create_info.format = VK_FORMAT_UNDEFINED;
    image_create_info.mipLevels = 1;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkDeviceImageMemoryRequirements image_memory_req = vku::InitStructHelper();
    image_memory_req.pCreateInfo = &image_create_info;
    image_memory_req.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;
    VkMemoryRequirements2 out_memory_req = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkDeviceImageMemoryRequirements-pNext-06996");
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &image_memory_req, &out_memory_req);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidHardwareBuffer, NullAHBProperties) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer calls must have non-null AHB objects passed in.");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_BLOB, AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER, 64);

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-parameter");
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), nullptr, &ahb_props);
    m_errorMonitor->VerifyFound();

    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);
}

TEST_F(NegativeAndroidHardwareBuffer, NullAHBImport) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer calls must have non-null AHB objects passed in.");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_BLOB, AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER, 64);

    VkExternalMemoryBufferCreateInfo ext_buf_info = vku::InitStructHelper();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&ext_buf_info);
    buffer_create_info.size = 512;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = vku::InitStructHelper();
    import_ahb_Info.buffer = nullptr;  // invalid

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    memory_allocate_info.allocationSize = ahb_props.allocationSize;

    // Set index to match one of the bits in ahb_props that is also only Device Local
    // Android implemenetations "should have" a DEVICE_LOCAL only index designed for AHB
    VkMemoryPropertyFlagBits property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkPhysicalDeviceMemoryProperties gpu_memory_props;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &gpu_memory_props);
    memory_allocate_info.memoryTypeIndex = gpu_memory_props.memoryTypeCount + 1;
    for (uint32_t i = 0; i < gpu_memory_props.memoryTypeCount; i++) {
        if ((ahb_props.memoryTypeBits & (1 << i)) && ((gpu_memory_props.memoryTypes[i].propertyFlags & property) == property)) {
            memory_allocate_info.memoryTypeIndex = i;
            break;
        }
    }
    if (memory_allocate_info.memoryTypeIndex >= gpu_memory_props.memoryTypeCount) {
        GTEST_SKIP() << "No valid memory type index could be found; skipped.\n";
    }

    m_errorMonitor->SetDesiredError("VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-parameter");
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    m_errorMonitor->VerifyFound();
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
