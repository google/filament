/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/external_memory_sync.h"
#include "utils/vk_layer_utils.h"
#include "generated/enum_flag_bits.h"

class PositiveExternalMemorySync : public ExternalMemorySyncTest {};

TEST_F(PositiveExternalMemorySync, GetMemoryFdHandle) {
    TEST_DESCRIPTION("Get POXIS handle for memory allocation");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkExportMemoryAllocateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&export_info);
    alloc_info.allocationSize = 1024;
    alloc_info.memoryTypeIndex = 0;

    vkt::DeviceMemory memory;
    memory.init(*m_device, alloc_info);
    VkMemoryGetFdInfoKHR get_handle_info = vku::InitStructHelper();
    get_handle_info.memory = memory;
    get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    int fd = -1;
    vk::GetMemoryFdKHR(*m_device, &get_handle_info, &fd);
}

TEST_F(PositiveExternalMemorySync, ImportMemoryFd) {
    TEST_DESCRIPTION("Basic importing of POXIS handle for memory allocation");
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

    VkImportMemoryFdInfoKHR import_info = vku::InitStructHelper(&dedicated_info);
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    import_info.fd = fd;

    alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
}

// Because of aligned_alloc
#if defined(__linux__) && !defined(__ANDROID__)
TEST_F(PositiveExternalMemorySync, ImportMemoryHost) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceExternalMemoryHostPropertiesEXT memory_host_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(memory_host_props);

    VkDeviceSize alloc_size = memory_host_props.minImportedHostPointerAlignment;
    void* host_memory = aligned_alloc(alloc_size, alloc_size);
    if (!host_memory) {
        GTEST_SKIP() << "Can't allocate host memory";
    }

    VkMemoryHostPointerPropertiesEXT host_pointer_props = vku::InitStructHelper();
    vk::GetMemoryHostPointerPropertiesEXT(*m_device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT, host_memory,
                                          &host_pointer_props);

    // test it is ignored when using null handle
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = VK_NULL_HANDLE;
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
    vkt::DeviceMemory memory_import(*m_device, alloc_info);

    free(host_memory);
}
#endif

TEST_F(PositiveExternalMemorySync, ExternalMemory) {
    TEST_DESCRIPTION("Perform a copy through a pair of buffers linked by external memory");

#ifdef _WIN32
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(ext_mem_extension_name);
    RETURN_IF_SKIP(InitFramework());
    // Check for import/export capability
    VkPhysicalDeviceExternalBufferInfoKHR ebi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO_KHR, nullptr, 0,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, handle_type};
    VkExternalBufferPropertiesKHR ebp = {VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES_KHR, nullptr, {0, 0, 0}};
    vk::GetPhysicalDeviceExternalBufferPropertiesKHR(Gpu(), &ebi, &ebp);
    if (!(ebp.externalMemoryProperties.compatibleHandleTypes & handle_type) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External buffer does not support importing and exporting";
    }

    // Check if dedicated allocation is required
    bool dedicated_allocation = ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT;
    if (dedicated_allocation && !IsExtensionsEnabled(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
        GTEST_SKIP() << "Dedicated allocation extension not supported";
    }

    RETURN_IF_SKIP(InitState());

    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize buffer_size = 1024;

    // Create export and import buffers
    const VkExternalMemoryBufferCreateInfo external_buffer_info = {VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR,
                                                                   nullptr, handle_type};
    auto buffer_info = vkt::Buffer::CreateInfo(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer_info.pNext = &external_buffer_info;
    vkt::Buffer buffer_export(*m_device, buffer_info, vkt::no_mem);
    vkt::Buffer buffer_import(*m_device, buffer_info, vkt::no_mem);

    // Allocation info
    auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_export.MemoryRequirements(), mem_flags);

    // Add export allocation info to pNext chain
    VkExportMemoryAllocateInfo export_info = {VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR, nullptr, handle_type};
    alloc_info.pNext = &export_info;

    // Add dedicated allocation info to pNext chain if required
    VkMemoryDedicatedAllocateInfo dedicated_info = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR, nullptr, VK_NULL_HANDLE,
                                                    buffer_export.handle()};
    if (dedicated_allocation) {
        export_info.pNext = &dedicated_info;
    }

    // Allocate memory to be exported
    vkt::DeviceMemory memory_export(*m_device, alloc_info);

    // Bind exported memory
    buffer_export.BindMemory(memory_export, 0);

#ifdef _WIN32
    // Export memory to handle
    VkMemoryGetWin32HandleInfoKHR mghi = {VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR, nullptr, memory_export.handle(),
                                          handle_type};
    HANDLE handle;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryWin32HandleKHR(device(), &mghi, &handle));

    VkImportMemoryWin32HandleInfoKHR import_info = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, nullptr, handle_type,
                                                    handle};
#else
    // Export memory to fd
    VkMemoryGetFdInfoKHR mgfi = {VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR, nullptr, memory_export.handle(), handle_type};
    int fd;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryFdKHR(device(), &mgfi, &fd));

    VkImportMemoryFdInfoKHR import_info = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR, nullptr, handle_type, fd};
#endif

    // Import
    if (dedicated_allocation) {
        dedicated_info.buffer = buffer_import;
        import_info.pNext = &dedicated_info;
    }
    alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_import.MemoryRequirements(), mem_flags);
    alloc_info.pNext = &import_info;
    vkt::DeviceMemory memory_import(*m_device, alloc_info);

    // Bind imported memory
    buffer_import.BindMemory(memory_import, 0);

    // Create test buffers and fill input buffer
    vkt::Buffer buffer_input(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto input_mem = (uint8_t*)buffer_input.Memory().Map();
    for (uint32_t i = 0; i < buffer_size; i++) {
        input_mem[i] = (i & 0xFF);
    }
    buffer_input.Memory().Unmap();
    vkt::Buffer buffer_output(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Copy from input buffer to output buffer through the exported/imported memory
    m_command_buffer.Begin();
    VkBufferCopy copy_info = {0, 0, buffer_size};
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_input.handle(), buffer_export.handle(), 1, &copy_info);
    // Insert memory barrier to guarantee copy order
    VkMemoryBarrier mem_barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT,
                                   VK_ACCESS_TRANSFER_READ_BIT};
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_import.handle(), buffer_output.handle(), 1, &copy_info);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveExternalMemorySync, BufferDedicatedAllocation) {
    TEST_DESCRIPTION("Create external buffer that requires dedicated allocation.");
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

    auto exportable_dedicated_types = FindSupportedExternalMemoryHandleTypes(
        Gpu(), buffer_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT);
    if (!exportable_dedicated_types) {
        GTEST_SKIP() << "Unable to find exportable handle type that requires dedicated allocation";
    }
    const auto handle_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(exportable_dedicated_types);

    external_buffer_info.handleTypes = handle_type;
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.buffer = buffer;

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(&dedicated_info);
    export_memory_info.handleTypes = handle_type;

    buffer.AllocateAndBindMemory(*m_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &export_memory_info);
}

TEST_F(PositiveExternalMemorySync, SyncFdSemaphore) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

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

    vkt::Semaphore import_semaphore(*m_device);
    import_semaphore.ImportHandle(fd_handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT);

    m_default_queue->Wait();
}

#ifdef VK_USE_PLATFORM_METAL_EXT
TEST_F(PositiveExternalMemorySync, ExportMetalObjects) {
    TEST_DESCRIPTION("Test vkExportMetalObjectsEXT");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_METAL_OBJECTS_EXTENSION_NAME);

    // Initialize framework
    {
        VkExportMetalObjectCreateInfoEXT queue_info = vku::InitStructHelper();
        queue_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT;

        VkExportMetalObjectCreateInfoEXT metal_info = vku::InitStructHelper(&queue_info);
        metal_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT;

        RETURN_IF_SKIP(InitFramework(&metal_info));

        VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_features = vku::InitStructHelper();
        auto features2 = GetPhysicalDeviceFeatures2(portability_features);

        RETURN_IF_SKIP(InitState(nullptr, &features2));
    }

    const VkDevice device = this->device();

    // Get Metal Device and Metal Command Queue in 1 call
    {
        const VkQueue queue = m_default_queue->handle();

        VkExportMetalCommandQueueInfoEXT queueInfo = vku::InitStructHelper();
        queueInfo.queue = queue;
        VkExportMetalDeviceInfoEXT deviceInfo = vku::InitStructHelper(&queueInfo);
        VkExportMetalObjectsInfoEXT objectsInfo = vku::InitStructHelper(&deviceInfo);

        // This tests both device, queue, and pNext chaining
        vk::ExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(deviceInfo.mtlDevice != nullptr);
        ASSERT_TRUE(queueInfo.mtlCommandQueue != nullptr);
    }

    // Get Metal Buffer
    {
        VkExportMetalObjectCreateInfoEXT metalBufferCreateInfo = vku::InitStructHelper();
        metalBufferCreateInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT;

        VkMemoryAllocateInfo mem_info = vku::InitStructHelper(&metalBufferCreateInfo);
        mem_info.allocationSize = 1024;

        VkDeviceMemory memory;
        const VkResult err = vk::AllocateMemory(device, &mem_info, NULL, &memory);
        ASSERT_EQ(VK_SUCCESS, err);

        VkExportMetalBufferInfoEXT bufferInfo = vku::InitStructHelper();
        bufferInfo.memory = memory;
        VkExportMetalObjectsInfoEXT objectsInfo = vku::InitStructHelper(&bufferInfo);

        vk::ExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(bufferInfo.mtlBuffer != nullptr);

        vk::FreeMemory(device, memory, nullptr);
    }

    // Get Metal Texture and Metal IOSurfaceRef
    {
        VkExportMetalObjectCreateInfoEXT metalSurfaceInfo = vku::InitStructHelper();
        metalSurfaceInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT;
        VkExportMetalObjectCreateInfoEXT metalTextureCreateInfo = vku::InitStructHelper(&metalSurfaceInfo);
        metalTextureCreateInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT;

        // Image contents don't matter
        VkImageCreateInfo ici = vku::InitStructHelper(&metalTextureCreateInfo);
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.format = VK_FORMAT_B8G8R8A8_UNORM;
        ici.extent = {32, 32, 1};
        ici.mipLevels = 1;
        ici.arrayLayers = 1;
        ici.samples = VK_SAMPLE_COUNT_1_BIT;
        ici.tiling = VK_IMAGE_TILING_LINEAR;
        ici.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        vkt::Image image(*m_device, ici, vkt::set_layout);

        VkExportMetalIOSurfaceInfoEXT surfaceInfo = vku::InitStructHelper();
        surfaceInfo.image = image.handle();
        VkExportMetalTextureInfoEXT textureInfo = vku::InitStructHelper(&surfaceInfo);
        textureInfo.image = image.handle();
        textureInfo.plane = VK_IMAGE_ASPECT_PLANE_0_BIT;  // Image is not multi-planar
        VkExportMetalObjectsInfoEXT objectsInfo = vku::InitStructHelper(&textureInfo);

        // This tests both texture, surface, and pNext chaining
        vk::ExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(textureInfo.mtlTexture != nullptr);
        ASSERT_TRUE(surfaceInfo.ioSurface != nullptr);
    }

    // Get Metal Shared Event
    {
        VkExportMetalObjectCreateInfoEXT metalEventCreateInfo = vku::InitStructHelper();
        metalEventCreateInfo.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT;

        VkEventCreateInfo eventCreateInfo = vku::InitStructHelper(&metalEventCreateInfo);
        vkt::Event event(*m_device, eventCreateInfo);
        ASSERT_TRUE(event.initialized());

        VkExportMetalSharedEventInfoEXT eventInfo = vku::InitStructHelper();
        eventInfo.event = event.handle();
        VkExportMetalObjectsInfoEXT objectsInfo = vku::InitStructHelper(&eventInfo);

        vk::ExportMetalObjectsEXT(device, &objectsInfo);

        ASSERT_TRUE(eventInfo.mtlSharedEvent != nullptr);
    }
}
#endif  // VK_USE_PLATFORM_METAL_EXT

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(PositiveExternalMemorySync, ExportFromImportedFence) {
    TEST_DESCRIPTION("Export from fence with imported payload");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
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
    VkPhysicalDeviceExternalFenceInfo fence_info = vku::InitStructHelper();
    fence_info.handleType = handle_type;
    VkExternalFenceProperties fence_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFenceProperties(Gpu(), &fence_info, &fence_properties);
    if ((handle_type & fence_properties.exportFromImportedHandleTypes) == 0) {
        GTEST_SKIP() << "can't find handle type that can be exported from imported fence";
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
    export_info2.handleTypes = handle_type;
    const VkFenceCreateInfo create_info2 = vku::InitStructHelper(&export_info2);
    vkt::Fence import_fence(*m_device, create_info2);
    import_fence.ImportHandle(handle, handle_type);

    // export from imported fence
    HANDLE handle2 = NULL;
    import_fence.ExportHandle(handle2, handle_type);

    ::CloseHandle(handle);
    if (handle2 != handle) {
        ::CloseHandle(handle2);
    }
}

TEST_F(PositiveExternalMemorySync, ImportMemoryWin32BufferDifferentDedicated) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/Vulkan-ValidationLayers/-/issues/35");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
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

    vkt::Buffer buffer2(*m_device, buffer_info, vkt::no_mem);
    dedicated_info.buffer = buffer2.handle();

    VkImportMemoryWin32HandleInfoKHR import_info = vku::InitStructHelper(&dedicated_info);
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    import_info.handle = handle;

    alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer2.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);

    // "For handle types defined as NT handles, the handles returned by vkGetFenceWin32HandleKHR are owned by the application. To
    // avoid leaking resources, the application must release ownership of them using the CloseHandle system call when they are no
    // longer needed."
    ::CloseHandle(handle);
}

#endif  // VK_USE_PLATFORM_WIN32_KHR

TEST_F(PositiveExternalMemorySync, MultipleExportOpaqueFd) {
    TEST_DESCRIPTION("regression from dEQP-VK.api.external.semaphore.opaque_fd.export_multiple_times_temporary");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    const auto handle_types = FindSupportedExternalSemaphoreHandleTypes(Gpu(), VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT);
    if ((handle_types & VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT) == 0) {
        GTEST_SKIP() << "VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT is not exportable";
    }

    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    vkt::Semaphore semaphore(*m_device, create_info);

    int handle = 0;
    semaphore.ExportHandle(handle, VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT);
    semaphore.ExportHandle(handle, VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT);
}

TEST_F(PositiveExternalMemorySync, ImportMemoryFdBufferDifferentDedicated) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/Vulkan-ValidationLayers/-/issues/35");
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

    vkt::Buffer buffer2(*m_device, buffer_info, vkt::no_mem);

    dedicated_info.buffer = buffer2.handle();

    VkImportMemoryFdInfoKHR import_info = vku::InitStructHelper(&dedicated_info);
    import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    import_info.fd = fd;

    alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer2.MemoryRequirements(), 0, &import_info);
    vkt::DeviceMemory memory_import(*m_device, alloc_info);
}
