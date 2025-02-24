/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/android_hardware_buffer.h"
#include "generated/vk_extension_helper.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)

class PositiveAndroidHardwareBuffer : public VkLayerTest {};

TEST_F(PositiveAndroidHardwareBuffer, MemoryRequirements) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer doesn't conflict with memory requirements.");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_BLOB, AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER, 64);

    VkExternalMemoryBufferCreateInfo ext_buf_info = vku::InitStructHelper();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = vku::InitStructHelper();
    memory_dedicated_info.image = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = vku::InitStructHelper(&memory_dedicated_info);
    import_ahb_Info.buffer = ahb.handle();

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&ext_buf_info);
    buffer_create_info.size = memory_allocate_info.allocationSize;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);
    memory_dedicated_info.buffer = buffer;

    // Should be able to bind memory with no error
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    vk::BindBufferMemory(device(), buffer.handle(), memory, 0);
}

TEST_F(PositiveAndroidHardwareBuffer, DepthStencil) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer can import Depth/Stencil");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_D16_UNORM, AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "Failed to Allocate AHB";
    }

    // Incase it hits the below driver bug, catch the false VUID error thrown from driver not creating valid AHB
    m_errorMonitor->SetUnexpectedError("VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkExternalMemoryImageCreateInfo ext_image_info = vku::InitStructHelper();
    ext_image_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    if (ahb_fmt_props.format != VK_FORMAT_D16_UNORM) {
        GTEST_SKIP() << "Driver bug: Didn't turn AHB format into VK_FORMAT_D16_UNORM";
    }

    // Create a Depth/Stencil image
    VkImageCreateInfo image_create_info = vku::InitStructHelper(&ext_image_info);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = ahb_fmt_props.format;
    image_create_info.extent = {64, 1, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image ds_image(*m_device, image_create_info, vkt::no_mem);
    if (ds_image.handle() == VK_NULL_HANDLE) {
        GTEST_SKIP() << "Was not able to create a D16 AHB framebuffer";
    }

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = vku::InitStructHelper();
    memory_dedicated_info.image = ds_image.handle();
    memory_dedicated_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        vku::InitStructHelper(&memory_dedicated_info);
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    vk::BindImageMemory(device(), ds_image.handle(), memory, 0);
}

TEST_F(PositiveAndroidHardwareBuffer, BindBufferMemory) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer Buffers can be queried for mem requirements while unbound.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkExternalMemoryBufferCreateInfo ext_buf_info = vku::InitStructHelper();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&ext_buf_info);
    buffer_create_info.size = 8192;  // greater than the 4k AHB usually are
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    // Try to get memory requirements prior to binding memory
    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    // Test bind memory 2 extension
    VkBufferMemoryRequirementsInfo2 buffer_mem_reqs2 = vku::InitStructHelper();
    buffer_mem_reqs2.buffer = buffer.handle();
    VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper();
    vk::GetBufferMemoryRequirements2(device(), &buffer_mem_reqs2, &mem_reqs2);

    // Allocate an AHardwareBuffer to match the size
    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_BLOB, AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER, mem_reqs.size);

    // Get real values
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = vku::InitStructHelper();
    memory_dedicated_info.image = VK_NULL_HANDLE;
    memory_dedicated_info.buffer = buffer;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = vku::InitStructHelper(&memory_dedicated_info);
    import_ahb_Info.buffer = ahb.handle();

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    vk::BindBufferMemory(device(), buffer.handle(), memory, 0);
}

TEST_F(PositiveAndroidHardwareBuffer, ExportBuffer) {
    TEST_DESCRIPTION("Verify VkBuffers can export to an AHB.");

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

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = vku::InitStructHelper();
    memory_dedicated_info.image = VK_NULL_HANDLE;
    memory_dedicated_info.buffer = buffer;

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(&memory_dedicated_info);
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper(&export_memory_info);
    memory_info.allocationSize = mem_reqs.size;

    bool has_memtype =
        m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &memory_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (!has_memtype) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    vkt::DeviceMemory memory(*m_device, memory_info);
    vk::BindBufferMemory(device(), buffer.handle(), memory, 0);

    // Export memory to AHB
    AHardwareBuffer *ahb = nullptr;

    VkMemoryGetAndroidHardwareBufferInfoANDROID get_ahb_info = vku::InitStructHelper();
    get_ahb_info.memory = memory;
    vk::GetMemoryAndroidHardwareBufferANDROID(device(), &get_ahb_info, &ahb);

    // App in charge of releasing after exporting
    AHardwareBuffer_release(ahb);
}

TEST_F(PositiveAndroidHardwareBuffer, ExportImage) {
    TEST_DESCRIPTION("Verify VkImages can export to an AHB.");

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
    vk::BindImageMemory(device(), image.handle(), memory, 0);

    // Export memory to AHB
    AHardwareBuffer *ahb = nullptr;

    VkMemoryGetAndroidHardwareBufferInfoANDROID get_ahb_info = vku::InitStructHelper();
    get_ahb_info.memory = memory;
    vk::GetMemoryAndroidHardwareBufferANDROID(device(), &get_ahb_info, &ahb);

    // App in charge of releasing after exporting
    AHardwareBuffer_release(ahb);
}

TEST_F(PositiveAndroidHardwareBuffer, ExternalImage) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer can import AHB with external format");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // FORMAT_Y8Cb8Cr8_420 is a known/public valid AHB Format but does not have a Vulkan mapping to it
    // Will use the external image feature to get access to it
    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    // The spec says the driver must not return zero, even if a VkFormat is returned with it, some older drivers do as a driver bug
    if (ahb_fmt_props.externalFormat == 0) {
        GTEST_SKIP() << "externalFormat was zero which is not valid";
    }

    // Create an image w/ external format
    VkExternalFormatANDROID ext_format = vku::InitStructHelper();
    ext_format.externalFormat = ahb_fmt_props.externalFormat;

    VkExternalMemoryImageCreateInfo ext_image_info = vku::InitStructHelper(&ext_format);
    ext_image_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&ext_image_info);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_UNDEFINED;
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

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    vk::BindImageMemory(device(), image.handle(), memory, 0);
}

TEST_F(PositiveAndroidHardwareBuffer, ExternalCameraFormat) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer can import AHB with external format");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Simulate camera usage of AHB
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED;
    ahb_desc.usage =
        AHARDWAREBUFFER_USAGE_CAMERA_WRITE | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    vkt::AHB ahb(&ahb_desc);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED";
    }

    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper();

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
    vk::GetAndroidHardwareBufferPropertiesANDROID(device(), ahb.handle(), &ahb_props);

    // The spec says the driver must not return zero, even if a VkFormat is returned with it, some older drivers do as a driver bug
    if (ahb_fmt_props.externalFormat == 0) {
        GTEST_SKIP() << "externalFormat was zero which is not valid";
    }

    // Create an image w/ external format
    VkExternalFormatANDROID ext_format = vku::InitStructHelper();
    ext_format.externalFormat = ahb_fmt_props.externalFormat;

    VkExternalMemoryImageCreateInfo ext_image_info = vku::InitStructHelper(&ext_format);
    ext_image_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&ext_image_info);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_UNDEFINED;
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

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&import_ahb_Info);
    if (!SetAllocationInfoImportAHB(m_device, ahb_props, memory_allocate_info)) {
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    vkt::DeviceMemory memory(*m_device, memory_allocate_info);
    vk::BindImageMemory(device(), image.handle(), memory, 0);
}

TEST_F(PositiveAndroidHardwareBuffer, DeviceImageMemoryReq) {
    TEST_DESCRIPTION("Call vkGetDeviceImageMemoryRequirementsKHR with externalFormat of zero.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = 0;

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
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &image_memory_req, &out_memory_req);
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
