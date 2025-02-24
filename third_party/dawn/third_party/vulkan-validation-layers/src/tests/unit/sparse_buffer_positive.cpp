
/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 * Copyright (c) 2025 Collabora, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class PositiveSparseBuffer : public VkLayerTest {};

TEST_F(PositiveSparseBuffer, NonOverlappingBufferCopy) {
    TEST_DESCRIPTION("Test correct non overlapping sparse buffers' copy");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    // 2 semaphores needed since we need to bind twice before copying
    vkt::Semaphore semaphore(*m_device);
    vkt::Semaphore semaphore2(*m_device);

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;
    copy_info.dstOffset = 0;
    copy_info.size = 0x10000;

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(copy_info.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);
    vkt::Buffer buffer_sparse2(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);
    vkt::DeviceMemory buffer_mem2;
    buffer_mem2.init(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_mem_reqs.size;
    buffer_memory_bind.memory = buffer_mem.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_infos[2] = {};
    buffer_memory_bind_infos[0].buffer = buffer_sparse.handle();
    buffer_memory_bind_infos[0].bindCount = 1;
    buffer_memory_bind_infos[0].pBinds = &buffer_memory_bind;
    buffer_memory_bind_infos[1].buffer = buffer_sparse2.handle();
    buffer_memory_bind_infos[1].bindCount = 1;
    buffer_memory_bind_infos[1].pBinds = &buffer_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 2;
    bind_info.pBufferBinds = buffer_memory_bind_infos;
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    vkt::Queue* sparse_queue = m_device->QueuesWithSparseCapability()[0];
    vk::QueueBindSparse(sparse_queue->handle(), 1, &bind_info, VK_NULL_HANDLE);
    sparse_queue->Wait();
    // Set up complete

    m_command_buffer.Begin();
    // This copy is be completely legal as long as we change the memory for buffer_sparse to not overlap with
    // buffer_sparse2's memory on queue submission
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_sparse.handle(), buffer_sparse2.handle(), 1, &copy_info);
    m_command_buffer.End();

    // Rebind buffer_mem2 so it does not overlap
    buffer_memory_bind.memory = buffer_mem2.handle();
    bind_info.bufferBindCount = 1;
    bind_info.waitSemaphoreCount = 1;
    bind_info.pWaitSemaphores = &semaphore.handle();
    bind_info.pSignalSemaphores = &semaphore2.handle();
    vk::QueueBindSparse(sparse_queue->handle(), 1, &bind_info, VK_NULL_HANDLE);
    sparse_queue->Wait();

    // Submitting copy command with non overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore2.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(PositiveSparseBuffer, NonOverlappingBufferCopy2) {
    TEST_DESCRIPTION("Non overlapping ranges copies should not trigger errors");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    constexpr VkDeviceSize copy_size = 16;
    // Consecutive ranges
    std::array<VkBufferCopy, 3> copy_info_list = {};
    copy_info_list[0].srcOffset = 0 * copy_size;
    copy_info_list[0].dstOffset = 1 * copy_size;
    copy_info_list[0].size = copy_size;

    copy_info_list[1].srcOffset = 2 * copy_size;
    copy_info_list[1].dstOffset = 3 * copy_size;
    copy_info_list[1].size = copy_size;

    copy_info_list[2].srcOffset = 4 * copy_size;
    copy_info_list[2].dstOffset = 5 * copy_size;
    copy_info_list[2].size = copy_size;

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(0x10000, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind_1 = {};
    buffer_memory_bind_1.size = buffer_mem_reqs.size;
    buffer_memory_bind_1.memory = buffer_mem.handle();

    std::array<VkSparseBufferMemoryBindInfo, 1> buffer_memory_bind_infos = {};
    buffer_memory_bind_infos[0].buffer = buffer_sparse.handle();
    buffer_memory_bind_infos[0].bindCount = 1;
    buffer_memory_bind_infos[0].pBinds = &buffer_memory_bind_1;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = size32(buffer_memory_bind_infos);
    bind_info.pBufferBinds = buffer_memory_bind_infos.data();
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.Wait(kWaitTimeout));
    // Set up complete

    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_sparse.handle(), buffer_sparse.handle(), size32(copy_info_list),
                      copy_info_list.data());
    m_command_buffer.End();

    // Submitting copy command with overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(PositiveSparseBuffer, NonOverlappingBufferCopy3) {
    TEST_DESCRIPTION("Test that overlaps are computed in buffer space, not memory space");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    VkBufferCreateInfo buffer_ci =
        vkt::Buffer::CreateInfo(4096 * 32, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    buffer_sparse.destroy();
    buffer_ci.size = 2 * buffer_mem_reqs.alignment;
    buffer_sparse.InitNoMemory(*m_device, buffer_ci);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;                          // srcOffset is the start of buffer_mem_1, or 0 in this space.
    copy_info.dstOffset = buffer_mem_reqs.alignment;  // dstOffset is the start of buffer_mem_2, or 0 in this space
                                                      // => since overlaps are computed in buffer space, none should be detected
    copy_info.size = buffer_mem_reqs.alignment;

    vkt::DeviceMemory buffer_mem_1(*m_device, buffer_mem_alloc);
    vkt::DeviceMemory buffer_mem_2(*m_device, buffer_mem_alloc);

    std::array<VkSparseMemoryBind, 2> buffer_memory_binds = {};
    buffer_memory_binds[0].size = buffer_mem_reqs.alignment;
    buffer_memory_binds[0].memory = buffer_mem_1.handle();
    buffer_memory_binds[1].resourceOffset = buffer_mem_reqs.alignment;
    buffer_memory_binds[1].size = buffer_mem_reqs.alignment;
    buffer_memory_binds[1].memory = buffer_mem_2.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer_sparse.handle();
    buffer_memory_bind_info.bindCount = size32(buffer_memory_binds);
    buffer_memory_bind_info.pBinds = buffer_memory_binds.data();

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    vkt::Queue* sparse_queue = m_device->QueuesWithSparseCapability()[0];
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue->handle(), 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.Wait(kWaitTimeout));
    // Set up complete

    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_sparse.handle(), buffer_sparse.handle(), 1, &copy_info);
    m_command_buffer.End();

    // Submitting copy command with overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
    sparse_queue->Wait();
}

TEST_F(PositiveSparseBuffer, NonOverlappingBufferCopy4) {
    TEST_DESCRIPTION("Test copying from a range that spans two different memory chunks");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    VkBufferCreateInfo buffer_ci =
        vkt::Buffer::CreateInfo(4096 * 32, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    if (buffer_mem_reqs.alignment <= 1) {
        GTEST_SKIP() << "Buffer copy will not work as intended if VkMemoryRequirements::alignment is not superior to 1";
    }
    buffer_sparse.destroy();
    buffer_ci.size = 2 * buffer_mem_reqs.alignment;
    buffer_sparse.InitNoMemory(*m_device, buffer_ci);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem_1(*m_device, buffer_mem_alloc);
    vkt::DeviceMemory buffer_mem_2(*m_device, buffer_mem_alloc);

    std::array<VkSparseMemoryBind, 2> buffer_memory_binds = {};
    buffer_memory_binds[0].size = buffer_mem_reqs.alignment;
    buffer_memory_binds[0].memory = buffer_mem_1.handle();
    buffer_memory_binds[1].resourceOffset = buffer_mem_reqs.alignment;
    buffer_memory_binds[1].size = buffer_mem_reqs.alignment;
    buffer_memory_binds[1].memory = buffer_mem_2.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer_sparse.handle();
    buffer_memory_bind_info.bindCount = size32(buffer_memory_binds);
    buffer_memory_bind_info.pBinds = buffer_memory_binds.data();

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    vkt::Queue* sparse_queue = m_device->QueuesWithSparseCapability()[0];
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue->handle(), 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.Wait(kWaitTimeout));
    // Set up complete

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;                              // srcOffset is the start of buffer_mem_1, or 0 in this space.
    copy_info.dstOffset = buffer_mem_reqs.alignment / 2;  // dstOffset is the start of buffer_mem_2, or 0 in this space
                                                          // => since overlaps are computed in buffer space, none should be detected
    copy_info.size = buffer_mem_reqs.alignment / 2;
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_sparse.handle(), buffer_sparse.handle(), 1, &copy_info);
    m_command_buffer.End();

    // Submitting copy command with overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
    sparse_queue->Wait();
}

TEST_F(PositiveSparseBuffer, NonOverlappingBufferCopy5) {
    TEST_DESCRIPTION("Test overlapping sparse buffers' copy were only one of the buffer is sparse");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;
    copy_info.dstOffset = 0;
    copy_info.size = 0x10000;

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(0x20000, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_not_sparse(*m_device, b_info, vkt::no_mem);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_sparse_mem_reqs = buffer_sparse.MemoryRequirements();
    const VkMemoryRequirements buffer_not_sparse_mem_reqs = buffer_not_sparse.MemoryRequirements();

    if ((buffer_sparse_mem_reqs.memoryTypeBits & buffer_not_sparse_mem_reqs.memoryTypeBits) == 0) {
        GTEST_SKIP() << "Could not find common memory type for sparse and not sparse buffer, skipping test";
    }
    buffer_sparse_mem_reqs.memoryTypeBits &= buffer_not_sparse_mem_reqs.memoryTypeBits;

    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_sparse_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);

    buffer_not_sparse.BindMemory(buffer_mem, 0);  // Bind not sparse buffer on first part of memory

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = 0x10000;
    buffer_memory_bind.memory = buffer_mem.handle();
    buffer_memory_bind.memoryOffset = 0x10000;  // Bind sparse buffer on second part of memory => no overlap

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer_sparse.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.Wait(kWaitTimeout));
    // Set up complete

    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_not_sparse.handle(), buffer_sparse.handle(), 1, &copy_info);
    m_command_buffer.End();

    // Submitting copy command with overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(PositiveSparseBuffer, BindSparseEmpty) {
    TEST_DESCRIPTION("Test submitting empty queue bind sparse");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Queue* sparse_queue = m_device->QueuesWithSparseCapability()[0];
    vk::QueueBindSparse(sparse_queue->handle(), 0u, nullptr, VK_NULL_HANDLE);
    sparse_queue->Wait();
}

TEST_F(PositiveSparseBuffer, BufferCopiesValidationStressTest) {
    TEST_DESCRIPTION("Validate 10,000 buffer copies");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    constexpr VkDeviceSize copy_size = 16;
    std::vector<VkBufferCopy> copy_info_list(4);
    copy_info_list[3].srcOffset = 0;
    copy_info_list[3].dstOffset = 16;
    copy_info_list[3].size = copy_size;

    copy_info_list[2].srcOffset = 64 + copy_size + 0 * 16;
    copy_info_list[2].dstOffset = 32;
    copy_info_list[2].size = copy_size;

    copy_info_list[1].srcOffset = 64 + copy_size + 1 * 16;
    copy_info_list[1].dstOffset = 48;
    copy_info_list[1].size = copy_size;

    copy_info_list[0].srcOffset = 64 + copy_size + 2 * 16;
    copy_info_list[0].dstOffset = 64;
    copy_info_list[0].size = copy_size;

    const size_t size = 10000;
    copy_info_list.resize(copy_info_list.size() + size);
    for (size_t i = 0; i < size; ++i) {
        copy_info_list[i + 4] = copy_info_list[i % 4];
    }

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(0x10000, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind_1 = {};
    buffer_memory_bind_1.size = buffer_mem_reqs.size;
    buffer_memory_bind_1.memory = buffer_mem.handle();

    std::array<VkSparseBufferMemoryBindInfo, 1> buffer_memory_bind_infos = {};
    buffer_memory_bind_infos[0].buffer = buffer_sparse.handle();
    buffer_memory_bind_infos[0].bindCount = 1;
    buffer_memory_bind_infos[0].pBinds = &buffer_memory_bind_1;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = size32(buffer_memory_bind_infos);
    bind_info.pBufferBinds = buffer_memory_bind_infos.data();
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.Wait(kWaitTimeout));
    // Set up complete

    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_sparse.handle(), buffer_sparse.handle(), size32(copy_info_list),
                      copy_info_list.data());
    m_command_buffer.End();

    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(PositiveSparseBuffer, BufferCopiesValidationStressTest2) {
    TEST_DESCRIPTION("Validate 10,000 buffer copies, buffer is bound to multiple VkDeviceMemory");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    constexpr int memory_chunks_count = 100;
    constexpr VkDeviceSize memory_size = 0x10000;
    constexpr VkDeviceSize copy_size = memory_size / 8;
    std::vector<VkBufferCopy> copy_info_list(4 * memory_chunks_count);
    for (int i = 0; i < memory_chunks_count * 4; i += 4) {
        copy_info_list[i + 3].srcOffset = memory_size * (i / 4);
        copy_info_list[i + 3].dstOffset = memory_size * (i / 4) + copy_size;
        copy_info_list[i + 3].size = copy_size;

        copy_info_list[i + 2].srcOffset = copy_info_list[i + 3].dstOffset + copy_size;
        copy_info_list[i + 2].dstOffset = copy_info_list[i + 2].srcOffset + copy_size;
        copy_info_list[i + 2].size = copy_size;

        copy_info_list[i + 1].srcOffset = copy_info_list[i + 2].dstOffset + copy_size;
        copy_info_list[i + 1].dstOffset = copy_info_list[i + 1].srcOffset + copy_size;
        copy_info_list[i + 1].size = copy_size;

        copy_info_list[i + 0].srcOffset = copy_info_list[i + 1].dstOffset + copy_size;
        copy_info_list[i + 0].dstOffset = copy_info_list[i + 0].srcOffset + copy_size;
        copy_info_list[i + 0].size = copy_size;
    }

    const size_t size = 10000;
    copy_info_list.resize(copy_info_list.size() + size);
    for (size_t i = 0; i < size; ++i) {
        copy_info_list[i + memory_chunks_count * 4] = copy_info_list[i % (4 * memory_chunks_count)];
    }

    VkBufferCreateInfo b_info = vkt::Buffer::CreateInfo(memory_chunks_count * memory_size,
                                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    std::vector<vkt::DeviceMemory> memory_chunks(memory_chunks_count);

    for (auto& mem : memory_chunks) {
        mem.init(*m_device, buffer_mem_alloc);
    }

    std::vector<VkSparseMemoryBind> buffer_memory_binds(memory_chunks_count);
    for (auto [i, mem_bind] : vvl::enumerate(buffer_memory_binds)) {
        mem_bind.size = memory_size;
        mem_bind.memory = memory_chunks[i].handle();
        mem_bind.resourceOffset = i * memory_size;
        mem_bind.memoryOffset = 0;
    }

    std::array<VkSparseBufferMemoryBindInfo, 1> buffer_memory_bind_infos = {};
    buffer_memory_bind_infos[0].buffer = buffer_sparse.handle();
    buffer_memory_bind_infos[0].bindCount = size32(buffer_memory_binds);
    buffer_memory_bind_infos[0].pBinds = buffer_memory_binds.data();

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = size32(buffer_memory_bind_infos);
    bind_info.pBufferBinds = buffer_memory_bind_infos.data();
    bind_info.signalSemaphoreCount = 1;
    bind_info.pSignalSemaphores = &semaphore.handle();

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.Wait(kWaitTimeout));
    // Set up complete

    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_sparse.handle(), buffer_sparse.handle(), size32(copy_info_list),
                      copy_info_list.data());
    m_command_buffer.End();

    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}
