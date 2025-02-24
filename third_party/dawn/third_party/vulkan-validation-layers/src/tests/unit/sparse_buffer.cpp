/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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
#include "../framework/layer_validation_tests.h"

class NegativeSparseBuffer : public VkLayerTest {};

TEST_F(NegativeSparseBuffer, QueueBindSparseMemoryBindSize) {
    TEST_DESCRIPTION("Test QueueBindSparse with invalid sparse memory bind size");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(0x10000, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    const auto buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = 0u;  // This will trigger the VUID we are testing
    buffer_memory_bind.memory = buffer_mem.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info{};
    buffer_memory_bind_info.buffer = buffer_sparse.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();

    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-size-01098");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSparseBuffer, QueueBindSparseMemoryBindAlignments) {
    TEST_DESCRIPTION("Test QueueBindSparse with invalid sparse memory bind size");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(0x20000, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    if (buffer_mem_reqs.alignment == 1) {
        GTEST_SKIP() << "Need buffer memory required alignment to be more than 1";
    }
    const auto buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);

    std::array<VkSparseMemoryBind, 3> buffer_memory_binds = {};
    buffer_memory_binds[0].resourceOffset = 0;
    buffer_memory_binds[0].size = 1u;  // Unaligned
    buffer_memory_binds[0].memory = buffer_mem.handle();
    buffer_memory_binds[0].memoryOffset = 0;

    buffer_memory_binds[1].resourceOffset = 1;  // Unaligned
    buffer_memory_binds[1].size = 0x10000;
    buffer_memory_binds[1].memory = buffer_mem.handle();
    buffer_memory_binds[1].memoryOffset = 0;

    buffer_memory_binds[2].resourceOffset = 0;
    buffer_memory_binds[2].size = 0x10000;
    buffer_memory_binds[2].memory = buffer_mem.handle();
    buffer_memory_binds[2].memoryOffset = 1;  // Unaligned

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info{};
    buffer_memory_bind_info.buffer = buffer_sparse.handle();
    buffer_memory_bind_info.bindCount = size32(buffer_memory_binds);
    buffer_memory_bind_info.pBinds = buffer_memory_binds.data();

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();

    // Unaligned size, resourceOffset, memoryOffset
    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-resourceOffset-09491", 3);
    // Also aligned memoryOffset
    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSparseBuffer, QueueBindSparseMemoryBindResourceOffset) {
    TEST_DESCRIPTION("Test QueueBindSparse with invalid sparse memory bind resource offset");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(0x10000, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    const auto buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_mem_reqs.size;
    buffer_memory_bind.memory = buffer_mem.handle();
    // This will trigger the VUID we are testing
    buffer_memory_bind.resourceOffset = buffer_mem_reqs.size + buffer_mem_reqs.size;

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info{};
    buffer_memory_bind_info.buffer = buffer_sparse.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();

    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-resourceOffset-01099");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSparseBuffer, QueueBindSparseMemoryBindSizeResourceOffset) {
    TEST_DESCRIPTION("Test QueueBindSparse with invalid sparse memory bind size due to resource offset");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(0x10000, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    const auto buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.memory = buffer_mem.handle();
    // This will trigger the VUID we are testing
    buffer_memory_bind.resourceOffset = 100;
    buffer_memory_bind.size = buffer_mem_reqs.size;

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info{};
    buffer_memory_bind_info.buffer = buffer_sparse.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();

    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-size-01100");
    if (buffer_memory_bind.resourceOffset % buffer_mem_reqs.alignment != 0) {
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-resourceOffset-09491");
    }
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSparseBuffer, QueueBindSparseMemoryBindSizeMemoryOffset) {
    TEST_DESCRIPTION("Test QueueBindSparse with invalid sparse memory bind size due to memory offset");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(0x10000, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    b_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer buffer_sparse(*m_device, b_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer_sparse.handle(), &buffer_mem_reqs);
    const auto buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.memory = buffer_mem.handle();
    // This will trigger the VUID we are testing
    buffer_memory_bind.memoryOffset = buffer_mem_alloc.allocationSize - 1;
    buffer_memory_bind.size = buffer_mem_reqs.size;

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info{};
    buffer_memory_bind_info.buffer = buffer_sparse.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();

    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-size-01102");
    if (buffer_memory_bind.memoryOffset % buffer_mem_reqs.alignment != 0) {
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-resourceOffset-09491");
    }
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSparseBuffer, OverlappingBufferCopy) {
    TEST_DESCRIPTION("Test overlapping sparse buffers' copy with overlapping device memory");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;
    copy_info.dstOffset = 0;
    copy_info.size = 256;

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

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();
    vkt::Fence sparse_queue_fence(*m_device);
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, sparse_queue_fence);
    ASSERT_EQ(VK_SUCCESS, sparse_queue_fence.Wait(kWaitTimeout));
    // Set up complete

    m_command_buffer.Begin();
    // This copy is not legal since both buffers share same device memory range, and none of them will be rebound
    // to non overlapping device memory ranges. Reported at queue submit
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_sparse.handle(), buffer_sparse2.handle(), 1, &copy_info);
    m_command_buffer.End();

    // Submitting copy command with overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-pRegions-00117");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(NegativeSparseBuffer, OverlappingBufferCopy2) {
    TEST_DESCRIPTION("Test overlapping sparse buffers' copy with overlapping device memory");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Semaphore semaphore(*m_device);

    constexpr VkDeviceSize copy_size = 16;
    std::array<VkBufferCopy, 4> copy_info_list = {};
    copy_info_list[0].srcOffset = 0;
    copy_info_list[0].dstOffset = 16;
    copy_info_list[0].size = copy_size;

    copy_info_list[1].srcOffset = 16;  // source overlaps copy_info_list[0].dst
    copy_info_list[1].dstOffset = 32;
    copy_info_list[1].size = copy_size;

    copy_info_list[2].srcOffset = 32;  // source overlaps copy_info_list[1].dst
    copy_info_list[2].dstOffset = 48;
    copy_info_list[2].size = copy_size;

    copy_info_list[3].srcOffset = 48;  // source overlaps copy_info_list[2].dst
    copy_info_list[3].dstOffset = 64;
    copy_info_list[3].size = copy_size;

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

    // Submitting copy command with overlapping device memory regions
    VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-pRegions-00117", 3);
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(NegativeSparseBuffer, OverlappingBufferCopy3) {
    TEST_DESCRIPTION("Test coyping from a range that spans two different memory chunks");

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

    vkt::DeviceMemory buffer_mem_1;
    buffer_mem_1.init(*m_device, buffer_mem_alloc);
    vkt::DeviceMemory buffer_mem_2;
    buffer_mem_2.init(*m_device, buffer_mem_alloc);

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
    copy_info.size = buffer_mem_reqs.alignment;
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-pRegions-00117");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
    sparse_queue->Wait();
}

TEST_F(NegativeSparseBuffer, OverlappingBufferCopy4) {
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
    copy_info.size = 256;

    VkBufferCreateInfo b_info =
        vkt::Buffer::CreateInfo(copy_info.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
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

    buffer_not_sparse.BindMemory(buffer_mem, 0);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_sparse_mem_reqs.size;
    buffer_memory_bind.memory = buffer_mem.handle();

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
    // This copy is not legal since both buffers share same device memory range, and none of them will be rebound
    // to non overlapping device memory ranges. Reported at queue submit
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-pRegions-00117");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}

TEST_F(NegativeSparseBuffer, BufferFlagsFeature) {
    TEST_DESCRIPTION("Create buffers with Flags that require disabled sparse features");

    VkPhysicalDeviceFeatures features = {};
    features.sparseBinding = VK_FALSE;
    features.sparseResidencyBuffer = VK_FALSE;
    features.sparseResidencyAliased = VK_FALSE;

    RETURN_IF_SKIP(InitFramework());
    RETURN_IF_SKIP(InitState(&features));

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 64;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-00915");

    buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-flags-00916");
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-00918");

    buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_ALIASED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-flags-00917");
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-00918");
}

TEST_F(NegativeSparseBuffer, VkSparseMemoryBindMemory) {
    TEST_DESCRIPTION("test VkSparseMemoryBind::memory is valid");
    AddRequiredFeature(vkt::Feature::sparseResidencyBuffer);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_create_info.size = 1024;

    if (m_device->Physical().Features().sparseResidencyBuffer) {
        buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    } else {
        GTEST_SKIP() << "Test requires unsupported sparseResidencyBuffer feature";
    }

    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);
    VkDeviceMemory bad_memory = CastToHandle<VkDeviceMemory, uintptr_t>(0xbaadbeef);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = 256;
    buffer_memory_bind.memory = bad_memory;
    buffer_memory_bind.memoryOffset = 0;

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;

    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-parameter");
    vk::QueueBindSparse(m_default_queue->handle(), 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSparseBuffer, VkSparseMemoryBindFlags) {
    TEST_DESCRIPTION("test VkSparseMemoryBind::flags is valid");
    AddRequiredFeature(vkt::Feature::sparseResidencyBuffer);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_create_info.size = 1024;

    if (m_device->Physical().Features().sparseResidencyBuffer) {
        buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    } else {
        GTEST_SKIP() << "Test requires unsupported sparseResidencyBuffer feature";
    }

    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = 256;
    buffer_memory_bind.memory = buffer_mem.handle();
    buffer_memory_bind.memoryOffset = 0;
    buffer_memory_bind.flags = 0xBAD00000;

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;

    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-flags-parameter");
    vk::QueueBindSparse(m_default_queue->handle(), 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}
