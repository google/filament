/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"

#ifndef VK_USE_PLATFORM_WIN32_KHR
#include <sys/mman.h>
#endif

class NegativeMemory : public VkLayerTest {};

TEST_F(NegativeMemory, MapMemory) {
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_ci.size = 256;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;

    // Ensure memory is big enough for both bindings
    // Want to make sure entire allocation is aligned to atom size
    const VkDeviceSize atom_size = m_device->Physical().limits_.nonCoherentAtomSize;
    static const VkDeviceSize allocation_size = atom_size * 64;
    alloc_info.allocationSize = allocation_size;
    bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        GTEST_SKIP() << "Failed to set memory type";
    }
    vkt::DeviceMemory mem(*m_device, alloc_info);

    uint8_t *pData;
    // Attempt to map memory size 0 is invalid
    m_errorMonitor->SetDesiredError("VUID-vkMapMemory-size-00680");
    vk::MapMemory(device(), mem, 0, 0, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Map memory twice
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData));
    m_errorMonitor->SetDesiredError("VUID-vkMapMemory-memory-00678");
    vk::MapMemory(device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();

    // Unmap the memory to avoid re-map error
    vk::UnmapMemory(device(), mem);
    // overstep offset with VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredError("VUID-vkMapMemory-offset-00679");
    vk::MapMemory(device(), mem, allocation_size + 1, VK_WHOLE_SIZE, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // overstep offset w/o VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredError("VUID-vkMapMemory-offset-00679");
    vk::MapMemory(device(), mem, allocation_size + 1, VK_WHOLE_SIZE, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // overstep allocation w/o VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredError("VUID-vkMapMemory-size-00681");
    vk::MapMemory(device(), mem, 1, allocation_size, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Now error due to unmapping memory that's not mapped
    m_errorMonitor->SetDesiredError("VUID-vkUnmapMemory-memory-00689");
    vk::UnmapMemory(device(), mem);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MapMemoryFlush) {
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_ci.size = 256;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;

    // Ensure memory is big enough for both bindings
    // Want to make sure entire allocation is aligned to atom
    const VkDeviceSize atom_size = m_device->Physical().limits_.nonCoherentAtomSize;
    static const VkDeviceSize allocation_size = atom_size * 64;
    alloc_info.allocationSize = allocation_size;
    bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        GTEST_SKIP() << "Failed to set memory type";
    }
    vkt::DeviceMemory mem(*m_device, alloc_info);

    uint8_t *pData;
    // Now map memory and cause errors due to flushing invalid ranges
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(device(), mem, 4 * atom_size, VK_WHOLE_SIZE, 0, (void **)&pData));
    VkMappedMemoryRange mem_range = vku::InitStructHelper();
    mem_range.memory = mem;
    mem_range.offset = atom_size;  // Error b/c offset less than offset of mapped mem
    m_errorMonitor->SetDesiredError("VUID-VkMappedMemoryRange-size-00685");
    vk::FlushMappedMemoryRanges(device(), 1, &mem_range);
    m_errorMonitor->VerifyFound();

    // Now flush range that oversteps mapped range
    vk::UnmapMemory(device(), mem);
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(device(), mem, 0, 4 * atom_size, 0, (void **)&pData));
    mem_range.offset = atom_size;
    mem_range.size = 4 * atom_size;  // Flushing bounds exceed mapped bounds
    m_errorMonitor->SetDesiredError("VUID-VkMappedMemoryRange-size-00685");
    vk::FlushMappedMemoryRanges(device(), 1, &mem_range);
    m_errorMonitor->VerifyFound();

    // Now flush range with VK_WHOLE_SIZE that oversteps offset
    vk::UnmapMemory(device(), mem);
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(device(), mem, 2 * atom_size, 4 * atom_size, 0, (void **)&pData));
    mem_range.offset = atom_size;
    mem_range.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkMappedMemoryRange-size-00686");
    vk::FlushMappedMemoryRanges(device(), 1, &mem_range);
    m_errorMonitor->VerifyFound();

    // Try flushing and invalidating host memory not mapped
    vk::UnmapMemory(device(), mem);
    mem_range.offset = 0;
    mem_range.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkMappedMemoryRange-memory-00684");
    vk::FlushMappedMemoryRanges(device(), 1, &mem_range);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkMappedMemoryRange-memory-00684");
    vk::InvalidateMappedMemoryRanges(device(), 1, &mem_range);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MapMemoryCoherentAtomSize) {
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, MapMemory will fail ASAN";
    }

    const VkDeviceSize atom_size = m_device->Physical().limits_.nonCoherentAtomSize;
    if (atom_size < 4) {
        GTEST_SKIP() << "nonCoherentAtomSize is too small";
    }

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_ci.size = 256;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;

    // Ensure memory is big enough for both bindings
    // Want to make sure entire allocation is aligned to atom
    static const VkDeviceSize allocation_size = atom_size * 64;
    alloc_info.allocationSize = allocation_size;
    bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        GTEST_SKIP() << "Failed to set memory type";
    }
    vkt::DeviceMemory mem(*m_device, alloc_info);

    uint8_t *pData;

    // Now with an offset NOT a multiple of the device limit
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(device(), mem, 0, 4 * atom_size, 0, (void **)&pData));
    VkMappedMemoryRange mem_range = vku::InitStructHelper();
    mem_range.memory = mem;
    mem_range.offset = 3;  // Not a multiple of atom_size
    mem_range.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkMappedMemoryRange-offset-00687");
    vk::FlushMappedMemoryRanges(device(), 1, &mem_range);
    m_errorMonitor->VerifyFound();

    // Now with a size NOT a multiple of the device limit
    vk::UnmapMemory(device(), mem);
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(device(), mem, 0, 4 * atom_size, 0, (void **)&pData));
    mem_range.offset = atom_size;
    mem_range.size = 2 * atom_size + 1;  // Not a multiple of atom_size
    m_errorMonitor->SetDesiredError("VUID-VkMappedMemoryRange-size-01390");
    vk::FlushMappedMemoryRanges(device(), 1, &mem_range);
    m_errorMonitor->VerifyFound();

    // Now with VK_WHOLE_SIZE and a mapping that does not end at a multiple of atom_size nor at the end of the memory.
    vk::UnmapMemory(device(), mem);
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(device(), mem, 0, 4 * atom_size + 1, 0, (void **)&pData));
    mem_range.offset = atom_size;
    mem_range.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkMappedMemoryRange-size-01389");
    vk::FlushMappedMemoryRanges(device(), 1, &mem_range);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MapMemory2) {
    TEST_DESCRIPTION("Attempt to map memory in a number of incorrect ways");

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

    VkMemoryUnmapInfo unmap_info = vku::InitStructHelper();
    unmap_info.memory = memory;

    uint8_t *pData;
    // Attempt to map memory size 0 is invalid
    map_info.offset = 0;
    map_info.size = 0;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-size-07960");
    vk::MapMemory2KHR(device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Map memory twice
    map_info.offset = 0;
    map_info.size = VK_WHOLE_SIZE;
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory2KHR(device(), &map_info, (void **)&pData));
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-memory-07958");
    vk::MapMemory2KHR(device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();

    // Unmap the memory to avoid re-map error
    vk::UnmapMemory2KHR(device(), &unmap_info);
    // overstep offset with VK_WHOLE_SIZE
    map_info.offset = allocation_size + 1;
    map_info.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-offset-07959");
    vk::MapMemory2KHR(device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // overstep allocation w/o VK_WHOLE_SIZE
    map_info.offset = 1,
    map_info.size = allocation_size;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-size-07961");
    vk::MapMemory2KHR(device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Now error due to unmapping memory that's not mapped
    m_errorMonitor->SetDesiredError("VUID-VkMemoryUnmapInfo-memory-07964");
    vk::UnmapMemory2KHR(device(), &unmap_info);
    m_errorMonitor->VerifyFound();
}
TEST_F(NegativeMemory, MapMemoryNullppData) {
    TEST_DESCRIPTION("vkMapMemory but ppData is null");
    RETURN_IF_SKIP(Init());

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = 1024;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory memory(*m_device, memory_info);

    m_errorMonitor->SetDesiredError("VUID-vkMapMemory-ppData-parameter");
    vk::MapMemory(device(), memory.handle(), 0, VK_WHOLE_SIZE, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MapMemWithoutHostVisibleBit) {
    TEST_DESCRIPTION("Allocate memory that is not mappable and then attempt to map it.");

    RETURN_IF_SKIP(Init());

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 1024;

    if (!m_device->Physical().SetMemoryType(0xFFFFFFFF, &mem_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        // If we can't find any unmappable memory this test doesn't make sense
        GTEST_SKIP() << "No unmappable memory types found";
    }

    vkt::DeviceMemory memory(*m_device, mem_alloc);
    void *mapped_address = nullptr;

    m_errorMonitor->SetDesiredError("VUID-vkMapMemory-memory-00682");
    m_errorMonitor->SetUnexpectedError("VUID-vkMapMemory-memory-00683");
    vk::MapMemory(device(), memory.handle(), 0, VK_WHOLE_SIZE, 0, &mapped_address);
    m_errorMonitor->VerifyFound();

    // Attempt to flush and invalidate non-host memory
    VkMappedMemoryRange memory_range = vku::InitStructHelper();
    memory_range.memory = memory.handle();
    memory_range.offset = 0;
    memory_range.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkMappedMemoryRange-memory-00684");
    vk::FlushMappedMemoryRanges(device(), 1, &memory_range);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkMappedMemoryRange-memory-00684");
    vk::InvalidateMappedMemoryRanges(device(), 1, &memory_range);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MapMemory2WithoutHostVisibleBit) {
    TEST_DESCRIPTION("Allocate memory that is not mappable and then attempt to map it.");
    AddRequiredExtensions(VK_KHR_MAP_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 1024;
    if (!m_device->Physical().SetMemoryType(
            0xFFFFFFFF, &mem_alloc, 0,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {  // If we can't find any unmappable memory this test doesn't make sense
        GTEST_SKIP() << "No unmappable memory types found";
    }

    vkt::DeviceMemory memory(*m_device, mem_alloc);

    VkMemoryMapInfo map_info = vku::InitStructHelper();
    map_info.memory = memory.handle();
    map_info.offset = 0;
    map_info.size = 32;
    uint8_t *pData;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-memory-07962");
    vk::MapMemory2KHR(device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();
}

#ifndef VK_USE_PLATFORM_WIN32_KHR
TEST_F(NegativeMemory, MapMemoryPlaced) {
    TEST_DESCRIPTION("Attempt to map placed memory in a number of incorrect ways");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAP_MEMORY_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MAP_MEMORY_PLACED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::memoryMapPlaced);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMapMemoryPlacedPropertiesEXT map_placed_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(map_placed_props);

    // Vulkan doesn't have any requirements on what allocationSize can be
    // other than that it must be non-zero.  Pick 64KB because that should
    // work out to an even number of pages on basically any GPU.
    const VkDeviceSize allocation_size = map_placed_props.minPlacedMemoryMapAlignment * 16;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = allocation_size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory memory(*m_device, memory_info);

    VkMemoryMapInfo map_info = vku::InitStructHelper();
    map_info.memory = memory;
    map_info.flags = VK_MEMORY_MAP_PLACED_BIT_EXT;
    map_info.offset = 0;
    map_info.size = VK_WHOLE_SIZE;

    // No VkMemoryMapPlacedInfoEXT
    void *pData;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-flags-09570");
    vk::MapMemory2KHR(device(), &map_info, &pData);
    m_errorMonitor->VerifyFound();

    VkMemoryMapPlacedInfoEXT placed_info = vku::InitStructHelper();
    map_info.pNext = &placed_info;

    // No VkMemoryMapPlacedInfoEXT::pPlacedAddress == NULL
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-flags-09570");
    vk::MapMemory2KHR(device(), &map_info, &pData);
    m_errorMonitor->VerifyFound();

    // Reserve two more pages in case we need to deal with any alignment weirdness.
    size_t reservation_size = allocation_size + map_placed_props.minPlacedMemoryMapAlignment;
    void *reservation = mmap(NULL, reservation_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT_TRUE(reservation != MAP_FAILED);

    // Align up to minPlacedMemoryMapAlignment
    uintptr_t align_1 = map_placed_props.minPlacedMemoryMapAlignment - 1;
    void *addr = reinterpret_cast<void *>((reinterpret_cast<uintptr_t>(reservation) + align_1) & ~align_1);

    placed_info.pPlacedAddress = ((char *)addr) + (map_placed_props.minPlacedMemoryMapAlignment / 2);

    // Unaligned VkMemoryMapPlacedInfoEXT::pPlacedAddress
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapPlacedInfoEXT-pPlacedAddress-09577");
    vk::MapMemory2KHR(device(), &map_info, &pData);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MemoryMapRangePlacedEnabled) {
    TEST_DESCRIPTION("Test when memoryMapRangePlaced is enabled");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAP_MEMORY_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MAP_MEMORY_PLACED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::memoryMapPlaced);
    AddRequiredFeature(vkt::Feature::memoryMapRangePlaced);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMapMemoryPlacedPropertiesEXT map_placed_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(map_placed_props);
    const VkDeviceSize allocation_size = map_placed_props.minPlacedMemoryMapAlignment * 16;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = allocation_size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory(*m_device, memory_info);

    // Reserve two more pages in case we need to deal with any alignment weirdness.
    size_t reservation_size = allocation_size + map_placed_props.minPlacedMemoryMapAlignment;
    void *reservation = mmap(NULL, reservation_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT_TRUE(reservation != MAP_FAILED);

    uintptr_t align_1 = map_placed_props.minPlacedMemoryMapAlignment - 1;
    void *addr = reinterpret_cast<void *>((reinterpret_cast<uintptr_t>(reservation) + align_1) & ~align_1);

    VkMemoryMapPlacedInfoEXT placed_info = vku::InitStructHelper();
    placed_info.pPlacedAddress = addr;
    VkMemoryMapInfo map_info = vku::InitStructHelper(&placed_info);
    map_info.memory = memory;
    map_info.flags = VK_MEMORY_MAP_PLACED_BIT_EXT;
    map_info.size = VK_WHOLE_SIZE;
    map_info.offset = map_placed_props.minPlacedMemoryMapAlignment / 2;

    void *pData;
    // Unaligned offset
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-flags-09573");
    vk::MapMemory2KHR(device(), &map_info, &pData);
    m_errorMonitor->VerifyFound();

    map_info.offset = 0;
    map_info.size = allocation_size - (map_placed_props.minPlacedMemoryMapAlignment / 2);

    // Unaligned size
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-flags-09574");
    vk::MapMemory2KHR(device(), &map_info, &pData);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MemoryMapRangePlacedDisabled) {
    TEST_DESCRIPTION("Test when memoryMapRangePlaced is disabled");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAP_MEMORY_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MAP_MEMORY_PLACED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::memoryMapPlaced);

    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMapMemoryPlacedPropertiesEXT map_placed_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(map_placed_props);
    const VkDeviceSize allocation_size = map_placed_props.minPlacedMemoryMapAlignment * 16;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = allocation_size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory(*m_device, memory_info);

    // Reserve two more pages in case we need to deal with any alignment weirdness.
    size_t reservation_size = allocation_size + map_placed_props.minPlacedMemoryMapAlignment;
    void *reservation = mmap(NULL, reservation_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT_TRUE(reservation != MAP_FAILED);

    uintptr_t align_1 = map_placed_props.minPlacedMemoryMapAlignment - 1;
    void *addr = reinterpret_cast<void *>((reinterpret_cast<uintptr_t>(reservation) + align_1) & ~align_1);

    VkMemoryMapPlacedInfoEXT placed_info = vku::InitStructHelper();
    placed_info.pPlacedAddress = addr;
    VkMemoryMapInfo map_info = vku::InitStructHelper(&placed_info);
    map_info.memory = memory;
    map_info.flags = VK_MEMORY_MAP_PLACED_BIT_EXT;
    map_info.size = VK_WHOLE_SIZE;
    map_info.offset = map_placed_props.minPlacedMemoryMapAlignment;

    void *pData;
    // Non-zero offset
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-flags-09571");
    vk::MapMemory2KHR(device(), &map_info, &pData);
    m_errorMonitor->VerifyFound();

    map_info.offset = 0;
    map_info.size = allocation_size - map_placed_props.minPlacedMemoryMapAlignment;

    // Not VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-flags-09572");
    vk::MapMemory2KHR(device(), &map_info, &pData);
    m_errorMonitor->VerifyFound();

    const VkDeviceSize unaligned_size = allocation_size + map_placed_props.minPlacedMemoryMapAlignment / 2;
    memory_info.allocationSize = unaligned_size;
    vkt::DeviceMemory unaligned_memory(*m_device, memory_info);

    map_info.memory = unaligned_memory;
    map_info.offset = 0;
    map_info.size = VK_WHOLE_SIZE;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-flags-09651");
    vk::MapMemory2KHR(device(), &map_info, &pData);
    m_errorMonitor->VerifyFound();

    map_info.size = unaligned_size;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryMapInfo-flags-09574");
    vk::MapMemory2KHR(device(), &map_info, &pData);
    m_errorMonitor->VerifyFound();
}
#endif

TEST_F(NegativeMemory, RebindMemoryMultiObjectDebugUtils) {
    VkResult err;
    bool pass;

    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-07460");

    RETURN_IF_SKIP(Init());

    // Create an image, allocate memory, free it, and then try to bind it
    VkMemoryRequirements mem_reqs;

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;
    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(tex_width, tex_height, 1, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    // Introduce failure, do NOT set memProps to
    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    mem_alloc.memoryTypeIndex = 1;

    vk::GetImageMemoryRequirements(device(), image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);

    // allocate 2 memory objects
    vkt::DeviceMemory mem1(*m_device, mem_alloc);
    vkt::DeviceMemory mem2(*m_device, mem_alloc);

    // Bind first memory object to Image object
    err = vk::BindImageMemory(device(), image, mem1, 0);
    ASSERT_EQ(VK_SUCCESS, err);

    // Introduce validation failure, try to bind a different memory object to
    // the same image object
    err = vk::BindImageMemory(device(), image, mem2, 0);
    m_errorMonitor->VerifyFound();

    // This particular VU should output three objects in its error message. Verify this works correctly.
    m_errorMonitor->SetDesiredError("VK_OBJECT_TYPE_IMAGE");
    err = vk::BindImageMemory(device(), image, mem2, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, QueryMemoryCommitmentWithoutLazyProperty) {
    TEST_DESCRIPTION("Attempt to query memory commitment on memory without lazy allocation");
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    const auto mem_reqs = image.MemoryRequirements();
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    image_alloc_info.allocationSize = mem_reqs.size;

    // the last argument is the "forbid" argument for SetMemoryType, disallowing
    // that particular memory type rather than requiring it
    if (!m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &image_alloc_info, 0,
                                            VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)) {
        GTEST_SKIP() << "Failed to set memory type";
    }
    vkt::DeviceMemory mem(*m_device, image_alloc_info);

    m_errorMonitor->SetDesiredError("VUID-vkGetDeviceMemoryCommitment-memory-00690");
    VkDeviceSize size;
    vk::GetDeviceMemoryCommitment(device(), mem.handle(), &size);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, BindImageMemoryType) {
    TEST_DESCRIPTION("Create an image, allocate memory, set a bad typeIndex and then try to bind it");
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(device(), image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;

    // Introduce Failure, select invalid TypeIndex
    VkPhysicalDeviceMemoryProperties memory_info;

    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    uint32_t i = 0;
    for (; i < memory_info.memoryTypeCount; i++) {
        // Would require deviceCoherentMemory feature
        if (memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) {
            continue;
        }
        // would require protected feature
        if (memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
            continue;
        }
        if ((mem_reqs.memoryTypeBits & (1 << i)) == 0) {
            mem_alloc.memoryTypeIndex = i;
            break;
        }
    }
    if (i >= memory_info.memoryTypeCount) {
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-01047");
    vkt::DeviceMemory mem(*m_device, mem_alloc);
    vk::BindImageMemory(device(), image.handle(), mem.handle(), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, BindMemory) {
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    auto buffer_create_info = vkt::Buffer::CreateInfo(4 * 1024 * 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    // Create an image/buffer, allocate memory, free it, and then try to bind it
    {
        vkt::Image image(*m_device, image_create_info, vkt::no_mem);
        vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);

        VkMemoryAllocateInfo image_mem_alloc = vku::InitStructHelper();
        VkMemoryAllocateInfo buffer_mem_alloc = vku::InitStructHelper();
        image_mem_alloc.allocationSize = image_mem_reqs.size;
        m_device->Physical().SetMemoryType(image_mem_reqs.memoryTypeBits, &image_mem_alloc, 0);
        buffer_mem_alloc.allocationSize = buffer_mem_reqs.size;
        m_device->Physical().SetMemoryType(buffer_mem_reqs.memoryTypeBits, &buffer_mem_alloc, 0);

        VkDeviceMemory image_mem = VK_NULL_HANDLE, buffer_mem = VK_NULL_HANDLE;
        vk::AllocateMemory(device(), &image_mem_alloc, nullptr, &image_mem);
        vk::AllocateMemory(device(), &buffer_mem_alloc, nullptr, &buffer_mem);

        vk::FreeMemory(device(), image_mem, nullptr);
        vk::FreeMemory(device(), buffer_mem, nullptr);

        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-parameter");
        vk::BindImageMemory(device(), image.handle(), image_mem, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-parameter");
        vk::BindBufferMemory(device(), buffer.handle(), buffer_mem, 0);
        m_errorMonitor->VerifyFound();
    }

    // Try to bind memory to an object that already has a memory binding
    {
        vkt::Image image(*m_device, image_create_info, vkt::no_mem);
        vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
        VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
        image_alloc_info.allocationSize = image_mem_reqs.size;
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
        m_device->Physical().SetMemoryType(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0);
        m_device->Physical().SetMemoryType(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);

        vkt::DeviceMemory image_mem(*m_device, image_alloc_info);
        vkt::DeviceMemory buffer_mem(*m_device, buffer_alloc_info);

        vk::BindImageMemory(device(), image.handle(), image_mem.handle(), 0);
        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-07460");
        vk::BindImageMemory(device(), image.handle(), image_mem.handle(), 0);
        m_errorMonitor->VerifyFound();

        vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), 0);
        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-buffer-07459");
        vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), 0);
        m_errorMonitor->VerifyFound();
    }

    // Try to bind memory to an object with an invalid memoryOffset
    {
        vkt::Image image(*m_device, image_create_info, vkt::no_mem);
        vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
        VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
        // Leave some extra space for alignment wiggle room
        image_alloc_info.allocationSize = image_mem_reqs.size + image_mem_reqs.alignment;
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size + buffer_mem_reqs.alignment;
        m_device->Physical().SetMemoryType(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0);
        m_device->Physical().SetMemoryType(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
        vkt::DeviceMemory image_mem(*m_device, image_alloc_info);
        vkt::DeviceMemory buffer_mem(*m_device, buffer_alloc_info);

        // Test unaligned memory offset
        {
            if (image_mem_reqs.alignment > 1) {
                VkDeviceSize image_offset = 1;
                m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memoryOffset-01048");
                vk::BindImageMemory(device(), image.handle(), image_mem.handle(), image_offset);
                m_errorMonitor->VerifyFound();
            }

            if (buffer_mem_reqs.alignment > 1) {
                VkDeviceSize buffer_offset = 1;
                m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memoryOffset-01036");
                vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), buffer_offset);
                m_errorMonitor->VerifyFound();
            }
        }

        // Test memory offsets outside the memory allocation
        {
            VkDeviceSize image_offset =
                (image_alloc_info.allocationSize + image_mem_reqs.alignment) & ~(image_mem_reqs.alignment - 1);
            m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memoryOffset-01046");
            vk::BindImageMemory(device(), image.handle(), image_mem.handle(), image_offset);
            m_errorMonitor->VerifyFound();

            VkDeviceSize buffer_offset =
                (buffer_alloc_info.allocationSize + buffer_mem_reqs.alignment) & ~(buffer_mem_reqs.alignment - 1);
            m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memoryOffset-01031");
            vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), buffer_offset);
            m_errorMonitor->VerifyFound();
        }

        // Test memory offsets within the memory allocation, but which leave too little memory for
        // the resource.
        {
            VkDeviceSize image_offset = (image_mem_reqs.size - 1) & ~(image_mem_reqs.alignment - 1);
            if ((image_offset > 0) && (image_mem_reqs.size < (image_alloc_info.allocationSize - image_mem_reqs.alignment))) {
                m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-size-01049");
                vk::BindImageMemory(device(), image.handle(), image_mem.handle(), image_offset);
                m_errorMonitor->VerifyFound();
            }

            VkDeviceSize buffer_offset = (buffer_mem_reqs.size - 1) & ~(buffer_mem_reqs.alignment - 1);
            if (buffer_offset > 0) {
                m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-size-01037");
                vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), buffer_offset);
                m_errorMonitor->VerifyFound();
            }
        }
    }

    // Try to bind memory to an image created with sparse memory flags
    {
        VkImageCreateInfo sparse_image_create_info = image_create_info;
        sparse_image_create_info.flags |= VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        VkImageFormatProperties image_format_properties = {};
        VkResult err = vk::GetPhysicalDeviceImageFormatProperties(m_device->Physical().handle(), sparse_image_create_info.format,
                                                                  sparse_image_create_info.imageType,
                                                                  sparse_image_create_info.tiling, sparse_image_create_info.usage,
                                                                  sparse_image_create_info.flags, &image_format_properties);
        if (!m_device->Physical().Features().sparseResidencyImage2D || err == VK_ERROR_FORMAT_NOT_SUPPORTED) {
            // most likely means sparse formats aren't supported here; skip this test.
        } else {
            if (image_format_properties.maxExtent.width == 0) {
                GTEST_SKIP() << "Sparse image format not supported";
            } else {
                vkt::Image sparse_image(*m_device, sparse_image_create_info, vkt::no_mem);
                VkMemoryRequirements sparse_mem_reqs = {};
                vk::GetImageMemoryRequirements(device(), sparse_image.handle(), &sparse_mem_reqs);
                if (sparse_mem_reqs.memoryTypeBits != 0) {
                    VkMemoryAllocateInfo sparse_mem_alloc = vku::InitStructHelper();
                    sparse_mem_alloc.allocationSize = sparse_mem_reqs.size;
                    sparse_mem_alloc.memoryTypeIndex = 0;
                    m_device->Physical().SetMemoryType(sparse_mem_reqs.memoryTypeBits, &sparse_mem_alloc, 0);
                    vkt::DeviceMemory memory(*m_device, sparse_mem_alloc);
                    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-01045");
                    vk::BindImageMemory(device(), sparse_image.handle(), memory.handle(), 0);
                    m_errorMonitor->VerifyFound();
                }
            }
        }
    }

    // Try to bind memory to a buffer created with sparse memory flags
    {
        VkBufferCreateInfo sparse_buffer_create_info = buffer_create_info;
        sparse_buffer_create_info.flags |= VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        if (!m_device->Physical().Features().sparseResidencyBuffer) {
            // most likely means sparse formats aren't supported here; skip this test.
        } else {
            vkt::Buffer sparse_buffer(*m_device, sparse_buffer_create_info, vkt::no_mem);
            VkMemoryRequirements sparse_mem_reqs = {};
            vk::GetBufferMemoryRequirements(device(), sparse_buffer.handle(), &sparse_mem_reqs);
            if (sparse_mem_reqs.memoryTypeBits != 0) {
                VkMemoryAllocateInfo sparse_mem_alloc = vku::InitStructHelper();
                sparse_mem_alloc.allocationSize = sparse_mem_reqs.size;
                sparse_mem_alloc.memoryTypeIndex = 0;
                m_device->Physical().SetMemoryType(sparse_mem_reqs.memoryTypeBits, &sparse_mem_alloc, 0);
                vkt::DeviceMemory memory(*m_device, sparse_mem_alloc);
                m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-buffer-01030");
                vk::BindBufferMemory(device(), sparse_buffer.handle(), memory.handle(), 0);
                m_errorMonitor->VerifyFound();
            }
        }
    }
}

TEST_F(NegativeMemory, BindMemoryUnsupported) {
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    auto buffer_info = vkt::Buffer::CreateInfo(4 * 1024 * 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    image_alloc_info.allocationSize = image_mem_reqs.size;
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    // Create a mask of available memory types *not* supported by these resources,
    // and try to use one of them.
    VkPhysicalDeviceMemoryProperties memory_properties = {};
    vk::GetPhysicalDeviceMemoryProperties(m_device->Physical().handle(), &memory_properties);

    uint32_t image_unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~image_mem_reqs.memoryTypeBits;
    // can't have protected bit because feature bit is not added
    bool found_type =
        m_device->Physical().SetMemoryType(image_unsupported_mem_type_bits, &image_alloc_info, 0,
                                           VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
    if (image_unsupported_mem_type_bits != 0 && found_type) {
        vkt::DeviceMemory memory(*m_device, image_alloc_info);
        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-01047");
        vk::BindImageMemory(device(), image.handle(), memory.handle(), 0);
        m_errorMonitor->VerifyFound();
    }

    uint32_t buffer_unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~buffer_mem_reqs.memoryTypeBits;
    found_type = m_device->Physical().SetMemoryType(buffer_unsupported_mem_type_bits, &buffer_alloc_info, 0,
                                                    VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
    if (buffer_unsupported_mem_type_bits != 0 && found_type) {
        vkt::DeviceMemory memory(*m_device, buffer_alloc_info);
        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-01035");
        vk::BindBufferMemory(device(), buffer.handle(), memory.handle(), 0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeMemory, BindMemoryNoCheckBuffer) {
    TEST_DESCRIPTION("Tests case were no call to memory requirements was made prior to binding");
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    // Create 2 buffers, one that is checked and one that isn't by GetBufferMemoryRequirements
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);
    vkt::Buffer unchecked_buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    // Leave some extra space for alignment wiggle room
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size + buffer_mem_reqs.alignment;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0));
    vkt::DeviceMemory buffer_mem(*m_device, buffer_alloc_info);
    vkt::DeviceMemory unchecked_buffer_mem(*m_device, buffer_alloc_info);

    if (buffer_mem_reqs.alignment > 1) {
        VkDeviceSize buffer_offset = 1;

        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memoryOffset-01036");
        vk::BindBufferMemory(device(), buffer, buffer_mem, buffer_offset);
        m_errorMonitor->VerifyFound();

        // Should trigger same VUID even when image was never checked
        // this makes an assumption that the driver will return the same image requirements for same createImageInfo where even
        // being close to running out of heap space
        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memoryOffset-01036");
        vk::BindBufferMemory(device(), unchecked_buffer, unchecked_buffer_mem, buffer_offset);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeMemory, BindMemoryNoCheckImage) {
    TEST_DESCRIPTION("Tests case were no call to memory requirements was made prior to binding");
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    // Create 2 images, one that is checked and one that isn't by GetImageMemoryRequirements
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);
    vkt::Image unchecked_image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    // Leave some extra space for alignment wiggle room
    image_alloc_info.allocationSize = image_mem_reqs.size + image_mem_reqs.alignment;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
    vkt::DeviceMemory image_mem(*m_device, image_alloc_info);
    vkt::DeviceMemory unchecked_image_mem(*m_device, image_alloc_info);

    // single-plane image
    if (image_mem_reqs.alignment > 1) {
        VkDeviceSize image_offset = 1;

        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memoryOffset-01048");
        vk::BindImageMemory(device(), image, image_mem, image_offset);
        m_errorMonitor->VerifyFound();

        // Should trigger same VUID even when image was never checked
        // this makes an assumption that the driver will return the same image requirements for same createImageInfo where even
        // being close to running out of heap space
        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memoryOffset-01048");
        vk::BindImageMemory(device(), unchecked_image, unchecked_image_mem, image_offset);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeMemory, BindMemoryNoCheckMultiPlane) {
    TEST_DESCRIPTION("Tests case were no call to memory requirements was made prior to binding");
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Check for support of format used by all multi-planar tests
    const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    VkFormatProperties mp_format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), mp_format, &mp_format_properties);
    if (!((mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
          (mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
        GTEST_SKIP() << "test rely on a supported disjoint format";
    }

    VkImageCreateInfo mp_image_create_info = vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, mp_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    mp_image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

    // Array represent planes for disjoint images
    VkDeviceMemory mp_image_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    VkDeviceMemory mp_unchecked_image_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    VkMemoryRequirements2 mp_image_mem_reqs2[2];
    VkMemoryAllocateInfo mp_image_alloc_info[2];

    vkt::Image mp_image(*m_device, mp_image_create_info, vkt::no_mem);
    vkt::Image mp_unchecked_image(*m_device, mp_image_create_info, vkt::no_mem);

    VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

    VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
    mem_req_info2.image = mp_image;
    mp_image_mem_reqs2[0] = vku::InitStructHelper();
    vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mp_image_mem_reqs2[0]);

    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
    mp_image_mem_reqs2[1] = vku::InitStructHelper();
    vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mp_image_mem_reqs2[1]);

    mp_image_alloc_info[0] = vku::InitStructHelper();
    mp_image_alloc_info[1] = vku::InitStructHelper();

    mp_image_alloc_info[0].allocationSize = mp_image_mem_reqs2[0].memoryRequirements.size;
    ASSERT_TRUE(
        m_device->Physical().SetMemoryType(mp_image_mem_reqs2[0].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[0], 0));
    // Leave some extra space for alignment wiggle room
    mp_image_alloc_info[1].allocationSize =
        mp_image_mem_reqs2[1].memoryRequirements.size + mp_image_mem_reqs2[1].memoryRequirements.alignment;
    ASSERT_TRUE(
        m_device->Physical().SetMemoryType(mp_image_mem_reqs2[1].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[1], 0));

    ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &mp_image_alloc_info[0], NULL, &mp_image_mem[0]));
    ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &mp_image_alloc_info[1], NULL, &mp_image_mem[1]));
    ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &mp_image_alloc_info[0], NULL, &mp_unchecked_image_mem[0]));
    ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &mp_image_alloc_info[1], NULL, &mp_unchecked_image_mem[1]));

    // Sets an invalid offset to plane 1
    if (mp_image_mem_reqs2[1].memoryRequirements.alignment > 1) {
        VkBindImagePlaneMemoryInfo plane_memory_info[2];
        plane_memory_info[0] = vku::InitStructHelper();
        plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        plane_memory_info[1] = vku::InitStructHelper();
        plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

        VkBindImageMemoryInfo bind_image_info[2];
        bind_image_info[0] = vku::InitStructHelper(&plane_memory_info[0]);
        bind_image_info[0].image = mp_image;
        bind_image_info[0].memory = mp_image_mem[0];
        bind_image_info[0].memoryOffset = 0;
        bind_image_info[1] = vku::InitStructHelper(&plane_memory_info[1]);
        bind_image_info[1].image = mp_image;
        bind_image_info[1].memory = mp_image_mem[1];
        bind_image_info[1].memoryOffset = 1;  // off alignment

        m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01620");
        vk::BindImageMemory2KHR(device(), 2, bind_image_info);
        m_errorMonitor->VerifyFound();

        // Should trigger same VUID even when image was never checked
        // this makes an assumption that the driver will return the same image requirements for same createImageInfo where even
        // being close to running out of heap space
        bind_image_info[0].image = mp_unchecked_image;
        bind_image_info[0].memory = mp_unchecked_image_mem[0];
        bind_image_info[1].image = mp_unchecked_image;
        bind_image_info[1].memory = mp_unchecked_image_mem[1];
        m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01620");
        vk::BindImageMemory2KHR(device(), 2, bind_image_info);
        m_errorMonitor->VerifyFound();
    }

    vk::FreeMemory(device(), mp_image_mem[0], NULL);
    vk::FreeMemory(device(), mp_image_mem[1], NULL);
    vk::FreeMemory(device(), mp_unchecked_image_mem[0], NULL);
    vk::FreeMemory(device(), mp_unchecked_image_mem[1], NULL);
}

TEST_F(NegativeMemory, BindMemory2BindInfos) {
    TEST_DESCRIPTION("These tests deal with VK_KHR_bind_memory_2 and invalid VkBindImageMemoryInfo* pBindInfos");
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    // Create 2 image with 2 memory objects
    vkt::Image image_a(*m_device, image_create_info, vkt::no_mem);
    vkt::Image image_b(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image_a, &image_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    image_alloc_info.allocationSize = image_mem_reqs.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
    vkt::DeviceMemory image_a_mem(*m_device, image_alloc_info);

    vk::GetImageMemoryRequirements(device(), image_b, &image_mem_reqs);
    image_alloc_info.allocationSize = image_mem_reqs.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
    vkt::DeviceMemory image_b_mem(*m_device, image_alloc_info);

    // Try binding same image twice in array
    VkBindImageMemoryInfo bind_image_info[3];
    bind_image_info[0] = vku::InitStructHelper();
    bind_image_info[0].image = image_a;
    bind_image_info[0].memory = image_a_mem;
    bind_image_info[0].memoryOffset = 0;
    bind_image_info[1] = vku::InitStructHelper();
    bind_image_info[1].image = image_b;
    bind_image_info[1].memory = image_b_mem;
    bind_image_info[1].memoryOffset = 0;
    bind_image_info[2] = bind_image_info[0];  // duplicate bind

    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory2-pBindInfos-04006");
    vk::BindImageMemory2KHR(device(), 3, bind_image_info);
    m_errorMonitor->VerifyFound();

    // Bind same image to 2 different memory in same array
    bind_image_info[1].image = image_a;
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory2-pBindInfos-04006");
    vk::BindImageMemory2KHR(device(), 2, bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, BindMemory2BindInfosMultiPlane) {
    TEST_DESCRIPTION("These tests deal with VK_KHR_bind_memory_2 and invalid VkBindImageMemoryInfo* pBindInfos");
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;

    // Check for support of format used by all multi-planar tests
    VkFormatProperties mp_format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), mp_format, &mp_format_properties);
    if (!((mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
          (mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
        GTEST_SKIP() << "test rely on a supported disjoint format";
    }

    // Creat 1 normal, not disjoint image
    vkt::Image normal_image(*m_device, image_create_info, vkt::no_mem);
    VkMemoryRequirements image_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), normal_image, &image_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    image_alloc_info.allocationSize = image_mem_reqs.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
    vkt::DeviceMemory normal_image_mem(*m_device, image_alloc_info);

    // Create 2 disjoint images with memory backing each plane
    VkImageCreateInfo mp_image_create_info = image_create_info;
    mp_image_create_info.format = mp_format;
    mp_image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

    VkDeviceMemory mp_image_a_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    VkDeviceMemory mp_image_b_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    vkt::Image mp_image_a(*m_device, mp_image_create_info, vkt::no_mem);
    vkt::Image mp_image_b(*m_device, mp_image_create_info, vkt::no_mem);

    auto allocate = [this](VkImage mp_image, VkDeviceMemory *mp_image_mem, VkImageAspectFlagBits plane) {
        VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
        image_plane_req.planeAspect = plane;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
        mem_req_info2.image = mp_image;

        VkMemoryRequirements2 mp_image_mem_reqs2 = vku::InitStructHelper();

        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mp_image_mem_reqs2);

        VkMemoryAllocateInfo mp_image_alloc_info = vku::InitStructHelper();
        mp_image_alloc_info.allocationSize = mp_image_mem_reqs2.memoryRequirements.size;
        ASSERT_TRUE(
            m_device->Physical().SetMemoryType(mp_image_mem_reqs2.memoryRequirements.memoryTypeBits, &mp_image_alloc_info, 0));
        vk::AllocateMemory(device(), &mp_image_alloc_info, NULL, mp_image_mem);
    };

    allocate(mp_image_a, &mp_image_a_mem[0], VK_IMAGE_ASPECT_PLANE_0_BIT);
    allocate(mp_image_a, &mp_image_a_mem[1], VK_IMAGE_ASPECT_PLANE_1_BIT);
    allocate(mp_image_b, &mp_image_b_mem[0], VK_IMAGE_ASPECT_PLANE_0_BIT);
    allocate(mp_image_b, &mp_image_b_mem[1], VK_IMAGE_ASPECT_PLANE_1_BIT);

    VkBindImagePlaneMemoryInfo plane_memory_info[2];
    plane_memory_info[0] = vku::InitStructHelper();
    plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    plane_memory_info[1] = vku::InitStructHelper();
    plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

    // set all sType and memoryOffset as they are the same
    VkBindImageMemoryInfo bind_image_info[6];
    for (int i = 0; i < 6; i++) {
        bind_image_info[i] = vku::InitStructHelper();
        bind_image_info[i].memoryOffset = 0;
    }

    // Try only binding part of image_b
    bind_image_info[0].pNext = (void *)&plane_memory_info[0];
    bind_image_info[0].image = mp_image_a;
    bind_image_info[0].memory = mp_image_a_mem[0];
    bind_image_info[1].pNext = (void *)&plane_memory_info[1];
    bind_image_info[1].image = mp_image_a;
    bind_image_info[1].memory = mp_image_a_mem[1];
    bind_image_info[2].pNext = (void *)&plane_memory_info[0];
    bind_image_info[2].image = mp_image_b;
    bind_image_info[2].memory = mp_image_b_mem[0];
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory2-pBindInfos-02858");
    vk::BindImageMemory2KHR(device(), 3, bind_image_info);
    m_errorMonitor->VerifyFound();

    // Same thing, but mix in a non-disjoint image
    bind_image_info[3].pNext = nullptr;
    bind_image_info[3].image = normal_image;
    bind_image_info[3].memory = normal_image_mem;
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory2-pBindInfos-02858");
    vk::BindImageMemory2KHR(device(), 4, bind_image_info);
    m_errorMonitor->VerifyFound();

    // Try binding image_b plane 1 twice
    // Valid case where binding disjoint and non-disjoint
    bind_image_info[4].pNext = (void *)&plane_memory_info[1];
    bind_image_info[4].image = mp_image_b;
    bind_image_info[4].memory = mp_image_b_mem[1];
    bind_image_info[5].pNext = (void *)&plane_memory_info[1];
    bind_image_info[5].image = mp_image_b;
    bind_image_info[5].memory = mp_image_b_mem[1];
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory2-pBindInfos-04006");
    vk::BindImageMemory2KHR(device(), 6, bind_image_info);
    m_errorMonitor->VerifyFound();

    // Try binding image_a with no plane specified
    bind_image_info[0].pNext = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-image-07736");
    vk::BindImageMemory2KHR(device(), 1, bind_image_info);
    m_errorMonitor->VerifyFound();
    bind_image_info[0].pNext = (void *)&plane_memory_info[0];

    // Valid case of binding 2 disjoint image and normal image by removing duplicate
    vk::BindImageMemory2KHR(device(), 5, bind_image_info);

    vk::FreeMemory(device(), mp_image_a_mem[0], nullptr);
    vk::FreeMemory(device(), mp_image_a_mem[1], nullptr);
    vk::FreeMemory(device(), mp_image_b_mem[0], nullptr);
    vk::FreeMemory(device(), mp_image_b_mem[1], nullptr);
}

TEST_F(NegativeMemory, BindMemoryToDestroyedObject) {
    RETURN_IF_SKIP(Init());

    // Create an image object, allocate memory, destroy the object and then try
    // to bind it
    VkImage image;
    VkMemoryRequirements mem_reqs;

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vk::CreateImage(device(), &image_create_info, nullptr, &image);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;
    vk::GetImageMemoryRequirements(device(), image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory mem(*m_device, mem_alloc);

    // Introduce validation failure, destroy Image object before binding
    vk::DestroyImage(device(), image, nullptr);

    // Now Try to bind memory to this destroyed object
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-parameter");
    vk::BindImageMemory(device(), image, mem, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, AllocationCount) {
    VkResult err = VK_SUCCESS;
    const int max_mems = 32;
    VkDeviceMemory mems[max_mems + 1];

    RETURN_IF_SKIP(InitFramework());

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    if (props.limits.maxMemoryAllocationCount > max_mems) {
        props.limits.maxMemoryAllocationCount = max_mems;
        fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    }
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->SetDesiredError("VUID-vkAllocateMemory-maxMemoryAllocationCount-04101");

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = 4;

    int i;
    for (i = 0; i <= max_mems; i++) {
        err = vk::AllocateMemory(device(), &mem_alloc, NULL, &mems[i]);
        if (err != VK_SUCCESS) {
            break;
        }
    }
    m_errorMonitor->VerifyFound();

    for (int j = 0; j < i; j++) {
        vk::FreeMemory(device(), mems[j], NULL);
    }
}

TEST_F(NegativeMemory, ImageMemoryNotBound) {
    TEST_DESCRIPTION("Attempt to draw with an image which has not had memory bound to it.");
    RETURN_IF_SKIP(Init());

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageCreateInfo image_create_info = vkt::Image::ImageCreateInfo2D(
        32, 32, 1, 1, tex_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);
    // Have to bind memory to image before recording cmd in cmd buffer using it
    VkMemoryRequirements mem_reqs;
    bool pass;
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    vk::GetImageMemoryRequirements(device(), image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory image_mem(*m_device, mem_alloc);

    // Introduce error, do not call vk::BindImageMemory(device(), image, image_mem, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-image-00003");

    m_command_buffer.Begin();
    VkClearColorValue ccv;
    ccv.float32[0] = 1.0f;
    ccv.float32[1] = 1.0f;
    ccv.float32[2] = 1.0f;
    ccv.float32[3] = 1.0f;
    VkImageSubresourceRange isr = {};
    isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    isr.baseArrayLayer = 0;
    isr.baseMipLevel = 0;
    isr.layerCount = 1;
    isr.levelCount = 1;
    vk::CmdClearColorImage(m_command_buffer.handle(), image, VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &isr);
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, BufferMemoryNotBound) {
    TEST_DESCRIPTION("Attempt to copy from a buffer which has not had memory bound to it.");
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkBufferCreateInfo buf_info = vku::InitStructHelper();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buf_info.size = 1024;
    vkt::Buffer buffer(*m_device, buf_info, vkt::no_mem);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 1024;
    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);
    if (!m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        GTEST_SKIP() << "Failed to set memory type";
    }

    VkBufferImageCopy region = {};
    region.bufferRowLength = 16;
    region.bufferImageHeight = 16;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {4, 4, 1};
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-srcBuffer-00176");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer, image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeMemory, DedicatedAllocationBinding) {
    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize resource_size = 1024;
    auto buffer_info = vkt::Buffer::CreateInfo(resource_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);
    auto buffer_alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), mem_flags);
    VkMemoryDedicatedAllocateInfo buffer_dedicated_info = vku::InitStructHelper();
    buffer_dedicated_info.buffer = buffer.handle();
    buffer_alloc_info.pNext = &buffer_dedicated_info;
    vkt::DeviceMemory dedicated_buffer_memory;
    dedicated_buffer_memory.init(*m_device, buffer_alloc_info);

    vkt::Buffer wrong_buffer(*m_device, buffer_info, vkt::no_mem);

    // Bind with wrong buffer
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-01508");
    vk::BindBufferMemory(m_device->handle(), wrong_buffer.handle(), dedicated_buffer_memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind with non-zero offset (same VUID)
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-01508");  // offset must be zero
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-size-01037");    // offset pushes us past size
    auto offset = buffer.MemoryRequirements().alignment;
    vk::BindBufferMemory(m_device->handle(), buffer.handle(), dedicated_buffer_memory.handle(), offset);
    m_errorMonitor->VerifyFound();

    // Bind correctly (depends on the "skip" above)
    vk::BindBufferMemory(m_device->handle(), buffer.handle(), dedicated_buffer_memory.handle(), 0);

    // And for images...
    auto image_info = vkt::Image::CreateInfo();
    image_info.extent.width = resource_size;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, image_info, vkt::no_mem);
    vkt::Image wrong_image(*m_device, image_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo image_dedicated_info = vku::InitStructHelper();
    image_dedicated_info.image = image.handle();
    auto image_alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image.MemoryRequirements(), mem_flags);
    image_alloc_info.pNext = &image_dedicated_info;
    vkt::DeviceMemory dedicated_image_memory;
    dedicated_image_memory.init(*m_device, image_alloc_info);

    // Bind with wrong image
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-02628");
    vk::BindImageMemory(m_device->handle(), wrong_image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind with non-zero offset (same VUID)
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-02628");  // offset must be zero
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-size-01049");    // offset pushes us past size
    auto image_offset = image.MemoryRequirements().alignment;
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), image_offset);
    m_errorMonitor->VerifyFound();

    // Bind correctly (depends on the "skip" above)
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), 0);
}

TEST_F(NegativeMemory, DedicatedAllocationImageAliasing) {
    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_DEDICATED_ALLOCATION_IMAGE_ALIASING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dedicatedAllocationImageAliasing);
    RETURN_IF_SKIP(Init());

    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize resource_size = 1024;

    auto image_info = vkt::Image::CreateInfo();
    image_info.extent.width = resource_size;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, image_info, vkt::no_mem);
    vkt::Image identical_image(*m_device, image_info, vkt::no_mem);
    vkt::Image post_delete_image(*m_device, image_info, vkt::no_mem);

    VkMemoryDedicatedAllocateInfo image_dedicated_info = vku::InitStructHelper();
    image_dedicated_info.image = image.handle();
    auto image_alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image.MemoryRequirements(), mem_flags);
    image_alloc_info.pNext = &image_dedicated_info;
    vkt::DeviceMemory dedicated_image_memory;
    dedicated_image_memory.init(*m_device, image_alloc_info);

    // Bind with different but identical image
    vk::BindImageMemory(m_device->handle(), identical_image.handle(), dedicated_image_memory.handle(), 0);

    image_info = vkt::Image::CreateInfo();
    image_info.extent.width = resource_size - 1;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image smaller_image(*m_device, image_info, vkt::no_mem);

    // Bind with a smaller image
    vk::BindImageMemory(m_device->handle(), smaller_image.handle(), dedicated_image_memory.handle(), 0);

    image_info = vkt::Image::CreateInfo();
    image_info.extent.width = resource_size + 1;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image larger_image(*m_device, image_info, vkt::no_mem);

    // Bind with a larger image (not supported, and not enough memory)
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-02629");
    if (larger_image.MemoryRequirements().size > image.MemoryRequirements().size) {
        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-size-01049");
    }
    vk::BindImageMemory(m_device->handle(), larger_image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind with non-zero offset
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-02629");  // offset must be zero
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-size-01049");    // offset pushes us past size
    auto image_offset = image.MemoryRequirements().alignment;
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), image_offset);
    m_errorMonitor->VerifyFound();

    // Bind correctly (depends on the "skip" above)
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), 0);

    image.destroy();  // destroy the original image
    vk::BindImageMemory(m_device->handle(), post_delete_image.handle(), dedicated_image_memory.handle(), 0);
}

TEST_F(NegativeMemory, BufferDeviceAddressEXT) {
    TEST_DESCRIPTION("Test VK_EXT_buffer_device_address.");
    AddRequiredExtensions(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceBufferAddressFeaturesEXT buffer_device_address_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(buffer_device_address_features);
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &buffer_device_address_features));
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_create_info.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-03338");

    buffer_create_info.flags = 0;
    VkBufferDeviceAddressCreateInfoEXT addr_ci = vku::InitStructHelper();
    addr_ci.deviceAddress = 1;
    buffer_create_info.pNext = &addr_ci;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-deviceAddress-02604");

    buffer_create_info.pNext = nullptr;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkBufferDeviceAddressInfo info = vku::InitStructHelper();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredError("VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vk::GetBufferDeviceAddressEXT(device(), &info);
    m_errorMonitor->VerifyFound();

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->Physical().SetMemoryType(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    vkt::DeviceMemory buffer_mem(*m_device, buffer_alloc_info);

    vk::BindBufferMemory(device(), buffer, buffer_mem, 0);
}

TEST_F(NegativeMemory, BufferDeviceAddressEXTDisabled) {
    TEST_DESCRIPTION("Test VK_EXT_buffer_device_address.");
    AddRequiredExtensions(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceBufferAddressFeaturesEXT buffer_device_address_features = vku::InitStructHelper();
    buffer_device_address_features.bufferDeviceAddress = VK_FALSE;
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &buffer_device_address_features));
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkBufferDeviceAddressInfo info = vku::InitStructHelper();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredError("VUID-vkGetBufferDeviceAddress-bufferDeviceAddress-03324");
    m_errorMonitor->SetDesiredError("VUID-VkBufferDeviceAddressInfo-buffer-02601");
    m_errorMonitor->SetDesiredError("VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vk::GetBufferDeviceAddressEXT(device(), &info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, BufferDeviceAddressKHR) {
    TEST_DESCRIPTION("Test VK_KHR_buffer_device_address.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffer_create_info.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-03338");

    buffer_create_info.flags = 0;
    VkBufferOpaqueCaptureAddressCreateInfo addr_ci = vku::InitStructHelper();
    addr_ci.opaqueCaptureAddress = 1;
    buffer_create_info.pNext = &addr_ci;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-opaqueCaptureAddress-03337");

    buffer_create_info.pNext = nullptr;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkBufferDeviceAddressInfo info = vku::InitStructHelper();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredError("VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vk::GetBufferDeviceAddressKHR(device(), &info);
    m_errorMonitor->VerifyFound();

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->Physical().SetMemoryType(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);

    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-bufferDeviceAddress-03339");
    vk::BindBufferMemory(device(), buffer, buffer_mem, 0);
    m_errorMonitor->VerifyFound();

    VkDeviceMemoryOpaqueCaptureAddressInfo mem_opaque_addr_info = vku::InitStructHelper();
    mem_opaque_addr_info.memory = buffer_mem;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
    vk::GetDeviceMemoryOpaqueCaptureAddressKHR(device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
    vk::GetDeviceMemoryOpaqueCaptureAddressKHR(device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), buffer_mem, NULL);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    buffer_alloc_info.pNext = &alloc_flags;
    vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);

    mem_opaque_addr_info.memory = buffer_mem;
    vk::GetDeviceMemoryOpaqueCaptureAddressKHR(device(), &mem_opaque_addr_info);

    vk::BindBufferMemory(device(), buffer, buffer_mem, 0);

    vk::GetBufferDeviceAddressKHR(device(), &info);

    vk::FreeMemory(device(), buffer_mem, NULL);
}

TEST_F(NegativeMemory, BufferDeviceAddressKHRDisabled) {
    TEST_DESCRIPTION("Test VK_KHR_buffer_device_address.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkBufferDeviceAddressInfo info = vku::InitStructHelper();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredError("VUID-vkGetBufferDeviceAddress-bufferDeviceAddress-03324");
    m_errorMonitor->SetDesiredError("VUID-VkBufferDeviceAddressInfo-buffer-02601");
    m_errorMonitor->SetDesiredError("VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vk::GetBufferDeviceAddressKHR(device(), &info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetBufferOpaqueCaptureAddress-None-03326");
    vk::GetBufferOpaqueCaptureAddressKHR(device(), &info);
    m_errorMonitor->VerifyFound();

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->Physical().SetMemoryType(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    VkDeviceMemoryOpaqueCaptureAddressInfo mem_opaque_addr_info = vku::InitStructHelper();
    mem_opaque_addr_info.memory = buffer_mem;
    m_errorMonitor->SetDesiredError("VUID-vkGetDeviceMemoryOpaqueCaptureAddress-None-03334");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
    vk::GetDeviceMemoryOpaqueCaptureAddressKHR(device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), buffer_mem, NULL);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
    buffer_alloc_info.pNext = &alloc_flags;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-flags-03330");
    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-flags-03331");
    vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MemoryType) {
    // Attempts to allocate from a memory type that doesn't exist

    RETURN_IF_SKIP(Init());
    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);

    m_errorMonitor->SetDesiredError("VUID-vkAllocateMemory-pAllocateInfo-01714");

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = memory_info.memoryTypeCount;
    mem_alloc.allocationSize = 4;

    VkDeviceMemory mem;
    vk::AllocateMemory(device(), &mem_alloc, NULL, &mem);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, AllocationBeyondHeapSize) {
    // Attempts to allocate a single piece of memory that's larger than the heap size

    RETURN_IF_SKIP(Init());
    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);

    m_errorMonitor->SetDesiredError("VUID-vkAllocateMemory-pAllocateInfo-01713");

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = memory_info.memoryHeaps[memory_info.memoryTypes[0].heapIndex].size + 1;

    VkDeviceMemory mem;
    vk::AllocateMemory(device(), &mem_alloc, NULL, &mem);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, DeviceCoherentMemoryDisabledAMD) {
    // Attempts to allocate device coherent memory without enabling the extension/feature
    AddRequiredExtensions(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, does not support the necessary memory type";
    }

    // Find a memory type that includes the device coherent memory property
    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    uint32_t deviceCoherentMemoryTypeIndex = memory_info.memoryTypeCount;  // Set to an invalid value just in case

    for (uint32_t i = 0; i < memory_info.memoryTypeCount; ++i) {
        if ((memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) != 0) {
            deviceCoherentMemoryTypeIndex = i;
            break;
        }
    }

    if (deviceCoherentMemoryTypeIndex == memory_info.memoryTypeCount) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    m_errorMonitor->SetDesiredError("VUID-vkAllocateMemory-deviceCoherentMemory-02790");

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = deviceCoherentMemoryTypeIndex;
    mem_alloc.allocationSize = 4;

    VkDeviceMemory mem;
    vk::AllocateMemory(device(), &mem_alloc, NULL, &mem);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, DedicatedAllocation) {
    TEST_DESCRIPTION("Create invalid requests to dedicated allocation of memory");

    // Both VK_KHR_dedicated_allocation and VK_KHR_sampler_ycbcr_conversion supported in 1.1
    // Quicke to set 1.1 then check all extensions in 1.0
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    const VkFormat disjoint_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    const VkFormat normal_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), disjoint_format, &format_properties);

    bool sparse_support = (m_device->Physical().Features().sparseBinding == VK_TRUE);
    bool disjoint_support = ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) != 0);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.size = 2048;

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, normal_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    // Create Images and Buffers without any memory backing
    VkImage normal_image = VK_NULL_HANDLE;
    vk::CreateImage(device(), &image_create_info, nullptr, &normal_image);

    VkBuffer normal_buffer = VK_NULL_HANDLE;
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &normal_buffer);

    VkImage sparse_image = VK_NULL_HANDLE;
    VkBuffer sparse_buffer = VK_NULL_HANDLE;
    if (sparse_support == true) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        vk::CreateImage(device(), &image_create_info, nullptr, &sparse_image);
        buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
        vk::CreateBuffer(device(), &buffer_create_info, nullptr, &sparse_buffer);
    }

    VkImage disjoint_image = VK_NULL_HANDLE;
    if (disjoint_support == true) {
        image_create_info.format = disjoint_format;
        image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
        vk::CreateImage(device(), &image_create_info, nullptr, &disjoint_image);
    }

    VkDeviceMemory device_memory;
    VkMemoryDedicatedAllocateInfo dedicated_allocate_info = vku::InitStructHelper();
    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&dedicated_allocate_info);
    memory_allocate_info.memoryTypeIndex = 0;
    memory_allocate_info.allocationSize = 64;

    // Both image and buffer set in dedicated allocation
    dedicated_allocate_info.image = normal_image;
    dedicated_allocate_info.buffer = normal_buffer;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfo-image-01432");
    vk::AllocateMemory(device(), &memory_allocate_info, NULL, &device_memory);
    m_errorMonitor->VerifyFound();

    if (sparse_support == true) {
        VkMemoryRequirements sparse_image_memory_req;
        vk::GetImageMemoryRequirements(device(), sparse_image, &sparse_image_memory_req);
        VkMemoryRequirements sparse_buffer_memory_req;
        vk::GetBufferMemoryRequirements(device(), sparse_buffer, &sparse_buffer_memory_req);

        dedicated_allocate_info.image = sparse_image;
        dedicated_allocate_info.buffer = VK_NULL_HANDLE;
        memory_allocate_info.allocationSize = sparse_image_memory_req.size;
        m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfo-image-01434");
        vk::AllocateMemory(device(), &memory_allocate_info, NULL, &device_memory);
        m_errorMonitor->VerifyFound();

        dedicated_allocate_info.image = VK_NULL_HANDLE;
        dedicated_allocate_info.buffer = sparse_buffer;
        memory_allocate_info.allocationSize = sparse_buffer_memory_req.size;
        m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfo-buffer-01436");
        vk::AllocateMemory(device(), &memory_allocate_info, NULL, &device_memory);
        m_errorMonitor->VerifyFound();
    }

    if (disjoint_support == true) {
        VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
        mem_req_info2.image = disjoint_image;
        VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();

        vk::GetImageMemoryRequirements2(device(), &mem_req_info2, &mem_req2);

        dedicated_allocate_info.image = disjoint_image;
        dedicated_allocate_info.buffer = VK_NULL_HANDLE;
        memory_allocate_info.allocationSize = mem_req2.memoryRequirements.size;
        m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfo-image-01797");
        vk::AllocateMemory(device(), &memory_allocate_info, NULL, &device_memory);
        m_errorMonitor->VerifyFound();
    }

    VkMemoryRequirements normal_image_memory_req;
    vk::GetImageMemoryRequirements(device(), normal_image, &normal_image_memory_req);
    VkMemoryRequirements normal_buffer_memory_req;
    vk::GetBufferMemoryRequirements(device(), normal_buffer, &normal_buffer_memory_req);

    // Set allocation size to be not equal to memory requirement
    memory_allocate_info.allocationSize = normal_image_memory_req.size - 1;
    dedicated_allocate_info.image = normal_image;
    dedicated_allocate_info.buffer = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfo-image-02964");
    vk::AllocateMemory(device(), &memory_allocate_info, NULL, &device_memory);
    m_errorMonitor->VerifyFound();

    memory_allocate_info.allocationSize = normal_buffer_memory_req.size - 1;
    dedicated_allocate_info.image = VK_NULL_HANDLE;
    dedicated_allocate_info.buffer = normal_buffer;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfo-buffer-02965");
    vk::AllocateMemory(device(), &memory_allocate_info, NULL, &device_memory);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(device(), normal_image, nullptr);
    vk::DestroyBuffer(device(), normal_buffer, nullptr);
    if (sparse_support == true) {
        vk::DestroyImage(device(), sparse_image, nullptr);
        vk::DestroyBuffer(device(), sparse_buffer, nullptr);
    }
    if (disjoint_support == true) {
        vk::DestroyImage(device(), disjoint_image, nullptr);
    }
}

TEST_F(NegativeMemory, MemoryRequirements) {
    TEST_DESCRIPTION("Create invalid requests to image and buffer memory requirments.");
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    // Need to make sure disjoint is supported for format
    // Also need to support an arbitrary image usage feature
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, &format_properties);
    if (!((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
          (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
        GTEST_SKIP() << "test requires disjoint/sampled feature bit on format";
    }
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

    VkImage image;
    VkResult err = vk::CreateImage(device(), &image_create_info, NULL, &image);
    ASSERT_EQ(VK_SUCCESS, err);

    m_errorMonitor->SetDesiredError("VUID-vkGetImageMemoryRequirements-image-01588");
    VkMemoryRequirements memory_requirements;
    vk::GetImageMemoryRequirements(device(), image, &memory_requirements);
    m_errorMonitor->VerifyFound();

    VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper();
    mem_req_info2.image = image;
    VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryRequirementsInfo2-image-01589");
    vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
    m_errorMonitor->VerifyFound();

    // Point to a 3rd plane for a 2-plane format
    VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
    mem_req_info2.pNext = &image_plane_req;
    mem_req_info2.image = image;

    m_errorMonitor->SetDesiredError("VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02281");
    vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
    m_errorMonitor->VerifyFound();

    // Test with a non planar image aspect also
    image_plane_req.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;
    mem_req_info2.pNext = &image_plane_req;
    mem_req_info2.image = image;

    m_errorMonitor->SetDesiredError("VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02281");
    vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(device(), image, nullptr);

    // Recreate image without Disjoint bit
    image_create_info.flags = 0;
    err = vk::CreateImage(device(), &image_create_info, NULL, &image);
    ASSERT_EQ(VK_SUCCESS, err);

    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    mem_req_info2.pNext = &image_plane_req;
    mem_req_info2.image = image;

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryRequirementsInfo2-image-01590");
    vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(device(), image, nullptr);

    // Recreate image with single plane format and with Disjoint bit
    image_create_info.flags = 0;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    err = vk::CreateImage(device(), &image_create_info, NULL, &image);
    ASSERT_EQ(VK_SUCCESS, err);

    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    mem_req_info2.pNext = &image_plane_req;
    mem_req_info2.image = image;

    // Disjoint bit isn't set as likely not even supported by non-planar format
    m_errorMonitor->SetUnexpectedError("VUID-VkImageMemoryRequirementsInfo2-image-01590");
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryRequirementsInfo2-image-02280");
    vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(device(), image, nullptr);
}

TEST_F(NegativeMemory, MemoryAllocatepNextChain) {
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkDeviceMemory mem;
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = 4;

    // pNext chain includes both VkExportMemoryAllocateInfo and VkExportMemoryAllocateInfoNV
    VkExportMemoryAllocateInfoNV export_memory_info_nv = vku::InitStructHelper();
    export_memory_info_nv.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_NV;

    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(&export_memory_info_nv);
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-00640");
    mem_alloc.pNext = &export_memory_info;
    vk::AllocateMemory(device(), &mem_alloc, NULL, &mem);
    m_errorMonitor->VerifyFound();
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(NegativeMemory, MemoryAllocatepNextChainWin32) {
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkDeviceMemory mem;
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = 4;

    // pNext chain includes both VkExportMemoryAllocateInfo and VkExportMemoryWin32HandleInfoNV
    {
        VkExportMemoryWin32HandleInfoNV export_memory_info_win32_nv = vku::InitStructHelper();
        export_memory_info_win32_nv.pAttributes = nullptr;
        export_memory_info_win32_nv.dwAccess = 0;

        VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(&export_memory_info_win32_nv);
        export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-00640");
        mem_alloc.pNext = &export_memory_info;
        vk::AllocateMemory(device(), &mem_alloc, NULL, &mem);
        m_errorMonitor->VerifyFound();
    }
    // pNext chain includes both VkImportMemoryWin32HandleInfoKHR and VkImportMemoryWin32HandleInfoNV
    {
        VkImportMemoryWin32HandleInfoKHR import_memory_info_win32_khr = vku::InitStructHelper();
        import_memory_info_win32_khr.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

        VkImportMemoryWin32HandleInfoNV import_memory_info_win32_nv = vku::InitStructHelper(&import_memory_info_win32_khr);
        import_memory_info_win32_nv.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_NV;

        m_errorMonitor->SetUnexpectedError("VUID-VkMemoryAllocateInfo-None-06657");
        m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-pNext-00641");
        mem_alloc.pNext = &import_memory_info_win32_nv;
        vk::AllocateMemory(device(), &mem_alloc, NULL, &mem);
        m_errorMonitor->VerifyFound();
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

TEST_F(NegativeMemory, DeviceImageMemoryRequirementsSwapchain) {
    TEST_DESCRIPTION("Validate usage of VkDeviceImageMemoryRequirementsKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&image_swapchain_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;

    VkDeviceImageMemoryRequirementsKHR device_image_memory_requirements = vku::InitStructHelper();
    device_image_memory_requirements.pCreateInfo = &image_create_info;
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;

    VkMemoryRequirements2 memory_requirements = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06416");
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, DeviceImageMemoryRequirementsDisjoint) {
    TEST_DESCRIPTION("Validate usage of VkDeviceImageMemoryRequirementsKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) == 0) {
        GTEST_SKIP() << "Test requires disjoint support extensions";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;

    VkDeviceImageMemoryRequirementsKHR device_image_memory_requirements = vku::InitStructHelper();
    device_image_memory_requirements.pCreateInfo = &image_create_info;
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_NONE;

    VkMemoryRequirements2 memory_requirements = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06417");
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();

    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06419");
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();

    // valid
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
}

TEST_F(NegativeMemory, BindBufferMemoryDeviceGroup) {
    TEST_DESCRIPTION("Test VkBindBufferMemoryDeviceGroupInfo.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0";
    }
    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = 0;
    create_device_pnext.pPhysicalDevices = nullptr;
    for (const auto &dg : physical_device_group) {
        if (dg.physicalDeviceCount > 1) {
            create_device_pnext.physicalDeviceCount = dg.physicalDeviceCount;
            create_device_pnext.pPhysicalDevices = dg.physicalDevices;
            break;
        }
    }
    if (create_device_pnext.pPhysicalDevices) {
        RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));
    } else {
        GTEST_SKIP() << "Test requires a physical device group with more than 1 device";
    }

    auto buffer_info = vkt::Buffer::CreateInfo(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    buffer_alloc_info.memoryTypeIndex = 0;
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;

    vkt::DeviceMemory buffer_memory(*m_device, buffer_alloc_info);

    std::vector<uint32_t> device_indices(create_device_pnext.physicalDeviceCount);

    VkBindBufferMemoryDeviceGroupInfo bind_buffer_memory_device_group = vku::InitStructHelper();
    bind_buffer_memory_device_group.deviceIndexCount = 1;
    bind_buffer_memory_device_group.pDeviceIndices = device_indices.data();

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper(&bind_buffer_memory_device_group);
    bind_buffer_info.buffer = buffer.handle();
    bind_buffer_info.memory = buffer_memory.handle();
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryDeviceGroupInfo-deviceIndexCount-01606");
    vk::BindBufferMemory2(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();

    bind_buffer_memory_device_group.deviceIndexCount = create_device_pnext.physicalDeviceCount;
    device_indices[0] = create_device_pnext.physicalDeviceCount;
    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryDeviceGroupInfo-pDeviceIndices-01607");
    vk::BindBufferMemory2(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
    device_indices[0] = 0;

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &image_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    image_alloc_info.memoryTypeIndex = 0;
    image_alloc_info.allocationSize = image_mem_reqs.size;

    vkt::DeviceMemory image_memory(*m_device, image_alloc_info);

    VkBindImageMemoryDeviceGroupInfo bind_image_memory_device_group = vku::InitStructHelper();
    bind_image_memory_device_group.deviceIndexCount = 1;
    bind_image_memory_device_group.pDeviceIndices = device_indices.data();

    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper(&bind_image_memory_device_group);
    bind_image_info.image = image.handle();
    bind_image_info.memory = image_memory.handle();
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01634");
    vk::BindImageMemory2(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MemoryPriorityOutOfRange) {
    TEST_DESCRIPTION("Allocate memory with invalid priority.");

    AddRequiredExtensions(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkMemoryPriorityAllocateInfoEXT priority = vku::InitStructHelper();
    priority.priority = 2.0f;

    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper(&priority);
    memory_ai.allocationSize = 0x100000;
    memory_ai.memoryTypeIndex = 0;

    VkDeviceMemory memory;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryPriorityAllocateInfoEXT-priority-02602");
    vk::AllocateMemory(*m_device, &memory_ai, nullptr, &memory);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, SetDeviceMemoryPriority) {
    AddRequiredExtensions(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, vkt::Buffer::CreateInfo(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT), vkt::no_mem);

    vkt::DeviceMemory buffer_memory;
    buffer_memory.init(*m_device, vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer.MemoryRequirements(), 0));

    m_errorMonitor->SetDesiredError("VUID-vkSetDeviceMemoryPriorityEXT-priority-06258");
    vk::SetDeviceMemoryPriorityEXT(*m_device, buffer_memory.handle(), -0.01f);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkSetDeviceMemoryPriorityEXT-priority-06258");
    vk::SetDeviceMemoryPriorityEXT(*m_device, buffer_memory.handle(), 1.01f);
    m_errorMonitor->VerifyFound();

    vk::SetDeviceMemoryPriorityEXT(*m_device, buffer_memory.handle(), 1.0f);
}

TEST_F(NegativeMemory, BadMemoryBindMemory2) {
    TEST_DESCRIPTION("Bind bogus memory for VkBindImageMemoryInfo::memory ");
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    VkDeviceMemory bad_memory = CastToHandle<VkDeviceMemory, uintptr_t>(0xbaadbeef);
    VkBindImageMemoryInfo image_bind_info = vku::InitStructHelper();
    image_bind_info.image = image.handle();
    image_bind_info.memory = bad_memory;
    image_bind_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01632");
    vk::BindImageMemory2KHR(device(), 1, &image_bind_info);
    m_errorMonitor->VerifyFound();
}

// TODO - Need a way to trigger Test ICD to fail call
TEST_F(NegativeMemory, DISABLED_PartialBoundBuffer) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5527");
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Test needs to force a failure";
    }

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkt::Buffer buffer_0(*m_device, buffer_create_info, vkt::no_mem);
    vkt::Buffer buffer_1(*m_device, buffer_create_info, vkt::no_mem);

    VkMemoryAllocateInfo alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_0.MemoryRequirements(), 0);
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    VkBindBufferMemoryInfo bind_buffer_infos[2];
    bind_buffer_infos[0] = vku::InitStructHelper();
    bind_buffer_infos[0].buffer = buffer_0;
    bind_buffer_infos[0].memory = buffer_memory;
    bind_buffer_infos[1] = vku::InitStructHelper();
    bind_buffer_infos[1].buffer = buffer_1;
    bind_buffer_infos[1].memory = buffer_memory;

    VkResult result = vk::BindBufferMemory2KHR(device(), 2, bind_buffer_infos);
    if (result == VK_SUCCESS) {
        GTEST_SKIP() << "Test needs a to fail call";
    }

    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-buffer-07459");
    vk::BindBufferMemory(device(), buffer_0, buffer_memory, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MaxMemoryAllocationSize) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Can't test well on real hardware";
    }

    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props11);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = props11.maxMemoryAllocationSize + 64;

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkAllocateMemory-pAllocateInfo-01713");  // need to bypass stateless
    m_errorMonitor->SetDesiredError("UNASSIGNED-vkAllocateMemory-maxMemoryAllocationSize");
    vkt::DeviceMemory memory(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, RequiredDedicatedAllocationBuffer) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    auto buffer_info = vkt::Buffer::CreateInfo(4096, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vkt::Buffer buffer(*m_device, buffer_info, vkt::no_mem);
    vkt::Buffer buffer2(*m_device, buffer_info, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferMemoryRequirementsInfo2 buffer_memory_requirements_info = vku::InitStructHelper();
    buffer_memory_requirements_info.buffer = buffer;

    VkMemoryDedicatedRequirements memory_dedicated_requirements = vku::InitStructHelper();
    VkMemoryRequirements2 memory_requirements = vku::InitStructHelper(&memory_dedicated_requirements);
    vk::GetBufferMemoryRequirements2(device(), &buffer_memory_requirements_info, &memory_requirements);
    // TODO - hard to get this required for buffer, would be good to have way for TestICD to force it on for this test
    if (!memory_dedicated_requirements.requiresDedicatedAllocation) {
        GTEST_SKIP() << "requiresDedicatedAllocation is false";
    }

    {
        VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
        memory_info.allocationSize = memory_requirements.memoryRequirements.size;
        bool pass = m_device->Physical().SetMemoryType(memory_requirements.memoryRequirements.memoryTypeBits, &memory_info,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        ASSERT_TRUE(pass);
        vkt::DeviceMemory device_memory(*m_device, memory_info);

        VkBindBufferMemoryInfo bind_buffer_memory_info = vku::InitStructHelper();
        bind_buffer_memory_info.buffer = buffer;
        bind_buffer_memory_info.memory = device_memory;
        bind_buffer_memory_info.memoryOffset = 0u;
        m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-buffer-01444");
        vk::BindBufferMemory2(device(), 1u, &bind_buffer_memory_info);
        m_errorMonitor->VerifyFound();
    }

    {
        VkMemoryDedicatedAllocateInfo memory_dedicated_allocate_info = vku::InitStructHelper();
        memory_dedicated_allocate_info.buffer = buffer2;
        VkMemoryAllocateInfo memory_info = vku::InitStructHelper(&memory_dedicated_allocate_info);
        memory_info.allocationSize = memory_requirements.memoryRequirements.size;
        bool pass = m_device->Physical().SetMemoryType(memory_requirements.memoryRequirements.memoryTypeBits, &memory_info,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        ASSERT_TRUE(pass);
        vkt::DeviceMemory device_memory(*m_device, memory_info);

        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-01508");
        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-buffer-01444");
        vk::BindBufferMemory(device(), buffer, device_memory, 0u);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeMemory, MapMemoryWithMapPlacedFlag) {
    TEST_DESCRIPTION("Test vkCmdUpdateBuffer");

    AddRequiredExtensions(VK_EXT_MAP_MEMORY_PLACED_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkDeviceSize allocation_size = 8192u;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = allocation_size;

    bool pass = m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory memory(*m_device, memory_info);

    void *data;
    m_errorMonitor->SetDesiredError("VUID-vkMapMemory-flags-09568");
    vk::MapMemory(device(), memory.handle(), 0u, 4u, VK_MEMORY_MAP_PLACED_BIT_EXT, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, RequiredDedicatedAllocationImage) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(32u, 32u, 1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    vkt::Image image2(*m_device, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkImageMemoryRequirementsInfo2 image_memory_requirements_info = vku::InitStructHelper();
    image_memory_requirements_info.image = image.handle();

    VkMemoryDedicatedRequirements memory_dedicated_requirements = vku::InitStructHelper();

    VkMemoryRequirements2 memory_requirements = vku::InitStructHelper(&memory_dedicated_requirements);
    vk::GetImageMemoryRequirements2(device(), &image_memory_requirements_info, &memory_requirements);

    if (!memory_dedicated_requirements.requiresDedicatedAllocation) {
        GTEST_SKIP() << "requiresDedicatedAllocation is false";
    }

    {
        VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
        memory_info.allocationSize = memory_requirements.memoryRequirements.size;
        bool pass = m_device->Physical().SetMemoryType(memory_requirements.memoryRequirements.memoryTypeBits, &memory_info,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        ASSERT_TRUE(pass);
        vkt::DeviceMemory device_memory(*m_device, memory_info);

        VkBindImageMemoryInfo bind_image_memory_info = vku::InitStructHelper();
        bind_image_memory_info.image = image.handle();
        bind_image_memory_info.memory = device_memory.handle();
        bind_image_memory_info.memoryOffset = 0u;
        m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-image-01445");
        vk::BindImageMemory2(device(), 1u, &bind_image_memory_info);
        m_errorMonitor->VerifyFound();
    }

    {
        VkMemoryDedicatedAllocateInfo memory_dedicated_allocate_info = vku::InitStructHelper();
        memory_dedicated_allocate_info.image = image2.handle();
        VkMemoryAllocateInfo memory_info = vku::InitStructHelper(&memory_dedicated_allocate_info);
        memory_info.allocationSize = memory_requirements.memoryRequirements.size;
        bool pass = m_device->Physical().SetMemoryType(memory_requirements.memoryRequirements.memoryTypeBits, &memory_info,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        ASSERT_TRUE(pass);
        vkt::DeviceMemory device_memory(*m_device, memory_info);

        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-02628");
        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-01445");
        vk::BindImageMemory(device(), image, device_memory, 0u);
        m_errorMonitor->VerifyFound();
    }
}
