/*
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 * Copyright (c) 2023-2025 Collabora, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <thread>
#include "../framework/layer_validation_tests.h"

#ifndef VK_USE_PLATFORM_WIN32_KHR
#include <sys/mman.h>
#endif

class PositiveMemory : public VkLayerTest {};

TEST_F(PositiveMemory, MapMemory2) {
    TEST_DESCRIPTION("Validate vkMapMemory2 and vkUnmapMemory2");

    AddRequiredExtensions(VK_KHR_MAP_MEMORY_2_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    /* Vulkan doesn't have any requirements on what allocationSize can be
     * other than that it must be non-zero.  Pick 64KB because that should
     * work out to an even number of pages on basically any GPU.
     */
    const VkDeviceSize allocation_size = 64 << 10;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = allocation_size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory memory(*m_device, memory_info);

    VkMemoryMapInfo map_info = vku::InitStructHelper();
    map_info.memory = memory;
    map_info.offset = 0;
    map_info.size = memory_info.allocationSize;

    VkMemoryUnmapInfoKHR unmap_info = vku::InitStructHelper();
    unmap_info.memory = memory;

    uint32_t *pData = nullptr;
    VkResult err = vk::MapMemory2KHR(device(), &map_info, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    ASSERT_TRUE(pData != nullptr);

    err = vk::UnmapMemory2KHR(device(), &unmap_info);
    ASSERT_EQ(VK_SUCCESS, err);

    map_info.size = VK_WHOLE_SIZE;

    pData = nullptr;
    err = vk::MapMemory2KHR(device(), &map_info, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    ASSERT_TRUE(pData != nullptr);

    err = vk::UnmapMemory2KHR(device(), &unmap_info);
    ASSERT_EQ(VK_SUCCESS, err);
}

#ifndef VK_USE_PLATFORM_WIN32_KHR
TEST_F(PositiveMemory, MapMemoryPlaced) {
    TEST_DESCRIPTION("Validate placed memory maps");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAP_MEMORY_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MAP_MEMORY_PLACED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::memoryMapPlaced);
    AddRequiredFeature(vkt::Feature::memoryUnmapReserve);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMapMemoryPlacedPropertiesEXT map_placed_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(map_placed_props);

    /* Vulkan doesn't have any requirements on what allocationSize can be
     * other than that it must be non-zero.  Pick 64KB because that should
     * work out to an even number of pages on basically any GPU.
     */
    const VkDeviceSize allocation_size = map_placed_props.minPlacedMemoryMapAlignment * 16;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = allocation_size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory memory(*m_device, memory_info);

    /* Reserve one more page in case we need to deal with any alignment weirdness. */
    size_t reservation_size = allocation_size + map_placed_props.minPlacedMemoryMapAlignment;
    void *reservation = mmap(NULL, reservation_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT_TRUE(reservation != MAP_FAILED);

    /* Align up to minPlacedMemoryMapAlignment */
    uintptr_t align_1 = map_placed_props.minPlacedMemoryMapAlignment - 1;
    void *addr = reinterpret_cast<void *>((reinterpret_cast<uintptr_t>(reservation) + align_1) & ~align_1);

    VkMemoryMapInfo map_info = vku::InitStructHelper();
    map_info.memory = memory;
    map_info.flags = VK_MEMORY_MAP_PLACED_BIT_EXT;
    map_info.offset = 0;
    map_info.size = VK_WHOLE_SIZE;

    VkMemoryMapPlacedInfoEXT placed_info = vku::InitStructHelper();
    placed_info.pPlacedAddress = addr;
    map_info.pNext = &placed_info;

    void *pData;
    VkResult res = vk::MapMemory2KHR(device(), &map_info, &pData);
    ASSERT_EQ(VK_SUCCESS, res);

    if (IsPlatformMockICD()) {
        return;  // currently can only validate the output with real driver
    }

    ASSERT_EQ(pData, addr);

    /* Write some data and make sure we don't fault */
    memset(pData, 0x5c, allocation_size);

    VkMemoryUnmapInfo unmap_info = vku::InitStructHelper();
    unmap_info.memory = memory;
    unmap_info.flags = VK_MEMORY_UNMAP_RESERVE_BIT_EXT;

    res = vk::UnmapMemory2KHR(device(), &unmap_info);
    ASSERT_EQ(VK_SUCCESS, res);

    /* Test mapping with the whole size but not VK_WHOLE_SIZE */
    map_info.size = allocation_size;
    res = vk::MapMemory2KHR(device(), &map_info, &pData);
    ASSERT_EQ(VK_SUCCESS, res);

    res = vk::UnmapMemory2KHR(device(), &unmap_info);
    ASSERT_EQ(VK_SUCCESS, res);

    map_info.flags = 0;
    vk::MapMemory2KHR(device(), &map_info, &pData);

    /* We unmapped with RESERVE above so this should be different */
    ASSERT_NE(pData, addr);

    ASSERT_EQ(static_cast<uint8_t *>(pData)[0], 0x5c);

    unmap_info.flags = 0;
    res = vk::UnmapMemory2KHR(device(), &unmap_info);
    ASSERT_EQ(VK_SUCCESS, res);
}
#endif

TEST_F(PositiveMemory, GetMemoryRequirements2) {
    TEST_DESCRIPTION(
        "Get memory requirements with VK_KHR_get_memory_requirements2 instead of core entry points and verify layers do not emit "
        "errors when objects are bound and used");

    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    vkt::Buffer buffer(
        *m_device, vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), vkt::no_mem);

    // Use extension to get buffer memory requirements
    VkBufferMemoryRequirementsInfo2 buffer_info = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR, nullptr,
                                                   buffer.handle()};
    VkMemoryRequirements2 buffer_reqs = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR};
    vk::GetBufferMemoryRequirements2KHR(device(), &buffer_info, &buffer_reqs);

    // Allocate and bind buffer memory
    vkt::DeviceMemory buffer_memory;
    buffer_memory.init(*m_device, vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_reqs.memoryRequirements, 0));
    vk::BindBufferMemory(device(), buffer.handle(), buffer_memory.handle(), 0);

    // Create a test image
    auto image_ci = vkt::Image::CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    // Use extension to get image memory requirements
    VkImageMemoryRequirementsInfo2 image_info = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2_KHR, nullptr, image.handle()};
    VkMemoryRequirements2 image_reqs = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR};
    vk::GetImageMemoryRequirements2KHR(device(), &image_info, &image_reqs);

    // Allocate and bind image memory
    vkt::DeviceMemory image_memory;
    image_memory.init(*m_device, vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image_reqs.memoryRequirements, 0));
    vk::BindImageMemory(device(), image.handle(), image_memory.handle(), 0);

    // Now execute arbitrary commands that use the test buffer and image
    m_command_buffer.Begin();

    // Fill buffer with 0
    vk::CmdFillBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_WHOLE_SIZE, 0);

    // Transition and clear image
    const auto subresource_range = image.SubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    const auto barrier = image.ImageMemoryBarrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                                  VK_IMAGE_LAYOUT_GENERAL, subresource_range);
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    const VkClearColorValue color = {};
    vk::CmdClearColorImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color, 1, &subresource_range);

    // Submit and verify no validation errors
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveMemory, BindMemory2) {
    TEST_DESCRIPTION(
        "Bind memory with VK_KHR_bind_memory2 instead of core entry points and verify layers do not emit errors when objects are "
        "used");

    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT), vkt::no_mem);

    // Allocate buffer memory
    vkt::DeviceMemory buffer_memory;
    buffer_memory.init(*m_device, vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0));

    // Bind buffer memory with extension
    VkBindBufferMemoryInfo buffer_bind_info = {VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO_KHR, nullptr, buffer.handle(),
                                               buffer_memory.handle(), 0};
    vk::BindBufferMemory2KHR(device(), 1, &buffer_bind_info);

    // Create a test image
    auto image_ci = vkt::Image::CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    // Allocate image memory
    vkt::DeviceMemory image_memory;
    image_memory.init(*m_device, vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image.MemoryRequirements(), 0));

    // Bind image memory with extension
    VkBindImageMemoryInfo image_bind_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO_KHR, nullptr, image.handle(),
                                             image_memory.handle(), 0};
    vk::BindImageMemory2KHR(device(), 1, &image_bind_info);

    // Now execute arbitrary commands that use the test buffer and image
    m_command_buffer.Begin();

    // Fill buffer with 0
    vk::CmdFillBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_WHOLE_SIZE, 0);

    // Transition and clear image
    const auto subresource_range = image.SubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    const auto barrier = image.ImageMemoryBarrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                                  VK_IMAGE_LAYOUT_GENERAL, subresource_range);
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    const VkClearColorValue color = {};
    vk::CmdClearColorImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color, 1, &subresource_range);

    // Submit and verify no validation errors
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveMemory, NonCoherentMapping) {
    TEST_DESCRIPTION(
        "Ensure that validations handling of non-coherent memory mapping while using VK_WHOLE_SIZE does not cause access "
        "violations");
    VkResult err;
    uint8_t *pData;
    RETURN_IF_SKIP(Init());

    VkMemoryRequirements mem_reqs;
    mem_reqs.memoryTypeBits = 0xFFFFFFFF;
    const VkDeviceSize atom_size = m_device->Physical().limits_.nonCoherentAtomSize;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;

    static const VkDeviceSize allocation_size = 32 * atom_size;
    alloc_info.allocationSize = allocation_size;

    // Find a memory configurations WITHOUT a COHERENT bit, otherwise exit
    bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!pass) {
        pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (!pass) {
            pass = m_device->Physical().SetMemoryType(
                mem_reqs.memoryTypeBits, &alloc_info,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            if (!pass) {
                GTEST_SKIP() << "Couldn't find a memory type wihtout a COHERENT bit";
            }
        }
    }

    vkt::DeviceMemory mem(*m_device, alloc_info);

    // Map/Flush/Invalidate using WHOLE_SIZE and zero offsets and entire mapped range
    err = vk::MapMemory(device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    VkMappedMemoryRange mmr = vku::InitStructHelper();
    mmr.memory = mem;
    mmr.offset = 0;
    mmr.size = VK_WHOLE_SIZE;
    err = vk::FlushMappedMemoryRanges(device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    err = vk::InvalidateMappedMemoryRanges(device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::UnmapMemory(device(), mem);

    // Map/Flush/Invalidate using WHOLE_SIZE and an offset and entire mapped range
    err = vk::MapMemory(device(), mem, 5 * atom_size, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    mmr.memory = mem;
    mmr.offset = 6 * atom_size;
    mmr.size = VK_WHOLE_SIZE;
    err = vk::FlushMappedMemoryRanges(device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    err = vk::InvalidateMappedMemoryRanges(device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::UnmapMemory(device(), mem);

    // Map with offset and size
    // Flush/Invalidate subrange of mapped area with offset and size
    err = vk::MapMemory(device(), mem, 3 * atom_size, 9 * atom_size, 0, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    mmr.memory = mem;
    mmr.offset = 4 * atom_size;
    mmr.size = 2 * atom_size;
    err = vk::FlushMappedMemoryRanges(device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    err = vk::InvalidateMappedMemoryRanges(device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::UnmapMemory(device(), mem);

    // Map without offset and flush WHOLE_SIZE with two separate offsets
    err = vk::MapMemory(device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    mmr.memory = mem;
    mmr.offset = allocation_size - (4 * atom_size);
    mmr.size = VK_WHOLE_SIZE;
    err = vk::FlushMappedMemoryRanges(device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    mmr.offset = allocation_size - (6 * atom_size);
    mmr.size = VK_WHOLE_SIZE;
    err = vk::FlushMappedMemoryRanges(device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::UnmapMemory(device(), mem);
}

TEST_F(PositiveMemory, MappingWithMultiInstanceHeapFlag) {
    TEST_DESCRIPTION("Test mapping memory that uses memory heap with VK_MEMORY_HEAP_MULTI_INSTANCE_BIT");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);

    uint32_t memory_index = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; ++i) {
        if ((memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            if (memory_info.memoryHeaps[memory_info.memoryTypes[i].heapIndex].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
                memory_index = i;
                break;
            }
        }
    }

    if (memory_index == std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "Did not host visible memory from memory heap with VK_MEMORY_HEAP_MULTI_INSTANCE_BIT bit";
    }

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 64;
    mem_alloc.memoryTypeIndex = memory_index;

    vkt::DeviceMemory memory(*m_device, mem_alloc);

    uint32_t *pData;
    vk::MapMemory(device(), memory, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
    vk::UnmapMemory(device(), memory);
}

TEST_F(PositiveMemory, BindImageMemoryMultiThreaded) {
    RETURN_IF_SKIP(Init());

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "This test can crash drivers with threading issues";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    // Create an image object, allocate memory, bind memory, and destroy the object
    auto worker_thread = [&]() {
        for (uint32_t i = 0; i < 1000; ++i) {
            vkt::Image image(*m_device, image_ci, vkt::no_mem);

            VkMemoryRequirements mem_reqs;
            vk::GetImageMemoryRequirements(device(), image, &mem_reqs);

            VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
            mem_alloc.memoryTypeIndex = 0;
            mem_alloc.allocationSize = mem_reqs.size;
            const bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &mem_alloc, 0);
            ASSERT_TRUE(pass);

            vkt::DeviceMemory mem(*m_device, mem_alloc);

            ASSERT_EQ(VK_SUCCESS, vk::BindImageMemory(device(), image, mem, 0));
        }
    };

    constexpr int worker_count = 32;
    std::vector<std::thread> workers;
    workers.reserve(worker_count);
    for (int i = 0; i < worker_count; ++i) {
        workers.emplace_back(worker_thread);
    }
    for (auto &worker : workers) {
        worker.join();
    }
}

TEST_F(PositiveMemory, DeviceBufferMemoryRequirements) {
    TEST_DESCRIPTION("Test vkGetDeviceBufferMemoryRequirements");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkDeviceBufferMemoryRequirements info = vku::InitStructHelper();
    info.pCreateInfo = &buffer_create_info;
    VkMemoryRequirements2 memory_reqs2 = vku::InitStructHelper();
    vk::GetDeviceBufferMemoryRequirements(device(), &info, &memory_reqs2);

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = memory_reqs2.memoryRequirements.size;

    const bool pass = m_device->Physical().SetMemoryType(memory_reqs2.memoryRequirements.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory buffer_memory(*m_device, memory_info);

    VkResult err = vk::BindBufferMemory(device(), buffer, buffer_memory, 0);
    ASSERT_EQ(VK_SUCCESS, err);
}

TEST_F(PositiveMemory, DeviceImageMemoryRequirements) {
    TEST_DESCRIPTION("Test vkGetDeviceImageMemoryRequirements");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkDeviceImageMemoryRequirements info = vku::InitStructHelper();
    info.pCreateInfo = &image_create_info;
    VkMemoryRequirements2 mem_reqs = vku::InitStructHelper();
    vk::GetDeviceImageMemoryRequirements(device(), &info, &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = mem_reqs.memoryRequirements.size;
    const bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory mem(*m_device, mem_alloc);

    VkResult err = vk::BindImageMemory(device(), image, mem, 0);
    ASSERT_EQ(VK_SUCCESS, err);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(PositiveMemory, BindMemoryDX11Handle) {
    TEST_DESCRIPTION("Bind memory imported from DX11 resource. Allocation size should be ignored.");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    // Mock ICD allows to use fake DX11 handles instead of using DX11 API directly.
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "This test only runs on the mock ICD";
    }

    VkExternalMemoryImageCreateInfo external_info = vku::InitStructHelper();
    external_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_ci.pNext = &external_info;
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs{};
    vk::GetImageMemoryRequirements(device(), image.handle(), &mem_reqs);

    VkImportMemoryWin32HandleInfoKHR memory_import = vku::InitStructHelper();
    memory_import.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
    memory_import.handle = (HANDLE)0x12345678;  // Use arbitrary non-zero value as DX11 resource handle

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&memory_import);  // Set zero allocation size
    m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, 0);
    vkt::DeviceMemory memory(*m_device, alloc_info);
    // This should not trigger VUs that take into accout allocation size (e.g. 01049/01046)
    vk::BindImageMemory(device(), image, memory, 0);
}

TEST_F(PositiveMemory, BindMemoryDX12Handle) {
    TEST_DESCRIPTION("Bind memory imported from DX12 resource. Allocation size should be ignored.");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    // Mock ICD allows to use fake DX12 handles instead of using DX12 API directly.
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "This test only runs on the mock ICD";
    }

    VkExternalMemoryImageCreateInfo external_info = vku::InitStructHelper();
    external_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_ci.pNext = &external_info;
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs{};
    vk::GetImageMemoryRequirements(device(), image.handle(), &mem_reqs);

    VkImportMemoryWin32HandleInfoKHR memory_import = vku::InitStructHelper();
    memory_import.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
    memory_import.handle = (HANDLE)0x12345678;  // Use arbitrary non-zero value as DX12 resource handle

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&memory_import);  // Set zero allocation size
    m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, 0);
    vkt::DeviceMemory memory(*m_device, alloc_info);
    // This should not trigger VUs that take into accout allocation size (e.g. 01049/01046)
    vk::BindImageMemory(device(), image, memory, 0);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

TEST_F(PositiveMemory, BindMemoryStatusBuffer) {
    TEST_DESCRIPTION("Use VkBindMemoryStatus when binding buffer to memory.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::maintenance6);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, skipping";
    }

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 32u;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    vkt::Buffer buffer;
    buffer.InitNoMemory(*m_device, buffer_ci);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 32u;
    vkt::DeviceMemory memory(*m_device, alloc_info);

    VkResult result = VK_RESULT_MAX_ENUM;

    VkBindMemoryStatus bind_memory_status = vku::InitStructHelper();
    bind_memory_status.pResult = &result;

    VkBindBufferMemoryInfo bind_info = vku::InitStructHelper(&bind_memory_status);
    bind_info.buffer = buffer;
    bind_info.memory = memory;
    bind_info.memoryOffset = 0u;
    vk::BindBufferMemory2(device(), 1u, &bind_info);

    ASSERT_NE(result, VK_RESULT_MAX_ENUM);
}

TEST_F(PositiveMemory, BindMemoryStatusImage) {
    TEST_DESCRIPTION("Use VkBindMemoryStatus when binding image to memory.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::maintenance6);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, skipping";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    vkt::DeviceMemory memory;
    memory.init(*m_device, vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image.MemoryRequirements(), 0));

    VkResult result = VK_RESULT_MAX_ENUM;

    VkBindMemoryStatus bind_memory_status = vku::InitStructHelper();
    bind_memory_status.pResult = &result;

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper(&bind_memory_status);
    bind_info.image = image;
    bind_info.memory = memory;
    bind_info.memoryOffset = 0u;
    vk::BindImageMemory2(device(), 1u, &bind_info);

    ASSERT_NE(result, VK_RESULT_MAX_ENUM);
}

TEST_F(PositiveMemory, MapMemoryCoherentAtomSize) {
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, MapMemory will fail ASAN";
    }

    const VkDeviceSize atom_size = m_device->Physical().limits_.nonCoherentAtomSize;
    if (atom_size == 1) {
        // Some platforms have an atomsize of 1 which makes the test meaningless
        GTEST_SKIP() << "nonCoherentAtomSize is 1";
    }

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_ci.size = 256;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;

    alloc_info.allocationSize = (atom_size * 4) + 1;
    bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        GTEST_SKIP() << "Failed to set memory type";
    }
    vkt::DeviceMemory mem(*m_device, alloc_info);

    uint8_t *pData;
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData));
    // Offset is atom size, but total memory range is not atom size
    VkMappedMemoryRange mem_range = vku::InitStructHelper();
    mem_range.memory = mem;
    mem_range.offset = atom_size;
    mem_range.size = VK_WHOLE_SIZE;
    vk::FlushMappedMemoryRanges(device(), 1, &mem_range);
    vk::UnmapMemory(device(), mem);
}
