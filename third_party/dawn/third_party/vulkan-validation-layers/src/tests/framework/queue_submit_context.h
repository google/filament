/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include <vulkan/vulkan.h>
#include "binding.h"

struct QSTestContext {
    vkt::Device* dev;
    uint32_t q_fam = ~0U;
    VkQueue q0 = VK_NULL_HANDLE;
    VkQueue q1 = VK_NULL_HANDLE;

    vkt::Buffer buffer_a;
    vkt::Buffer buffer_b;
    vkt::Buffer buffer_c;

    VkBufferCopy full_buffer;
    VkBufferCopy first_half;
    VkBufferCopy second_half;
    VkBufferCopy first_to_second;
    VkBufferCopy second_to_first;
    vkt::CommandPool pool;

    vkt::CommandBuffer cba;
    vkt::CommandBuffer cbb;
    vkt::CommandBuffer cbc;

    VkCommandBuffer h_cba = VK_NULL_HANDLE;
    VkCommandBuffer h_cbb = VK_NULL_HANDLE;
    VkCommandBuffer h_cbc = VK_NULL_HANDLE;

    vkt::Semaphore semaphore;
    vkt::Event event;

    vkt::CommandBuffer* current_cb = nullptr;

    QSTestContext(vkt::Device* device, vkt::Queue* force_q0 = nullptr, vkt::Queue* force_q1 = nullptr);
    VkCommandBuffer InitFromPool(vkt::CommandBuffer& cb_obj);
    bool Valid() const { return q1 != VK_NULL_HANDLE; }

    void Begin(vkt::CommandBuffer& cb);
    void BeginA() { Begin(cba); }
    void BeginB() { Begin(cbb); }
    void BeginC() { Begin(cbc); }

    void End();
    void Copy(vkt::Buffer& from, vkt::Buffer& to, const VkBufferCopy& copy_region);
    void Copy(vkt::Buffer& from, vkt::Buffer& to) { Copy(from, to, full_buffer); }
    void CopyAToB() { Copy(buffer_a, buffer_b); }
    void CopyAToC() { Copy(buffer_a, buffer_c); }

    void CopyBToA() { Copy(buffer_b, buffer_a); }
    void CopyBToC() { Copy(buffer_b, buffer_c); }

    void CopyCToA() { Copy(buffer_c, buffer_a); }
    void CopyCToB() { Copy(buffer_c, buffer_b); }

    void CopyGeneral(const vkt::Image& from, const vkt::Image& to, const VkImageCopy& region);

    VkBufferMemoryBarrier InitBufferBarrier(const vkt::Buffer& buffer, VkAccessFlags src, VkAccessFlags dst);
    VkBufferMemoryBarrier InitBufferBarrierRAW(const vkt::Buffer& buffer);
    VkBufferMemoryBarrier InitBufferBarrierWAR(const vkt::Buffer& buffer);
    void TransferBarrierWAR(const vkt::Buffer& buffer);
    void TransferBarrierRAW(const vkt::Buffer& buffer);
    void TransferBarrier(const VkBufferMemoryBarrier& buffer_barrier);

    void Submit(VkQueue q, vkt::CommandBuffer& cb, VkSemaphore wait = VK_NULL_HANDLE,
                VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkSemaphore signal = VK_NULL_HANDLE,
                VkFence fence = VK_NULL_HANDLE);

    // X == Submit 2 but since we already have numeric overloads for the queues X -> eXtension version
    void SubmitX(VkQueue q, vkt::CommandBuffer& cb, VkSemaphore wait = VK_NULL_HANDLE,
                 VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkSemaphore signal = VK_NULL_HANDLE,
                 VkPipelineStageFlags signal_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkFence fence = VK_NULL_HANDLE);

    void Submit0(vkt::CommandBuffer& cb, VkSemaphore wait = VK_NULL_HANDLE,
                 VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkSemaphore signal = VK_NULL_HANDLE,
                 VkFence fence = VK_NULL_HANDLE) {
        Submit(q0, cb, wait, wait_mask, signal, fence);
    }
    void Submit0Wait(vkt::CommandBuffer& cb, VkPipelineStageFlags wait_mask) { Submit0(cb, semaphore.handle(), wait_mask); }
    void Submit0Signal(vkt::CommandBuffer& cb) { Submit0(cb, VK_NULL_HANDLE, 0U, semaphore.handle()); }

    void Submit1(vkt::CommandBuffer& cb, VkSemaphore wait = VK_NULL_HANDLE,
                 VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkSemaphore signal = VK_NULL_HANDLE,
                 VkFence fence = VK_NULL_HANDLE) {
        Submit(q1, cb, wait, wait_mask, signal, fence);
    }
    void Submit1Wait(vkt::CommandBuffer& cb, VkPipelineStageFlags wait_mask) { Submit1(cb, semaphore.handle(), wait_mask); }
    void Submit1Signal(vkt::CommandBuffer& cb, VkPipelineStageFlags signal_mask) {
        Submit1(cb, VK_NULL_HANDLE, 0U, semaphore.handle());
    }
    void SetEvent(VkPipelineStageFlags src_mask) { event.CmdSet(*current_cb, src_mask); }
    void WaitEventBufferTransfer(vkt::Buffer& buffer, VkPipelineStageFlags src_mask, VkPipelineStageFlags dst_mask);

    void ResetEvent(VkPipelineStageFlags src_mask) { event.CmdReset(*current_cb, src_mask); }

    void QueueWait(VkQueue q) { vk::QueueWaitIdle(q); }
    void QueueWait0() { QueueWait(q0); }
    void QueueWait1() { QueueWait(q1); }
    void DeviceWait() { vk::DeviceWaitIdle(dev->handle()); }

    void RecordCopy(vkt::CommandBuffer& cb, vkt::Buffer& from, vkt::Buffer& to, const VkBufferCopy& copy_region);
    void RecordCopy(vkt::CommandBuffer& cb, vkt::Buffer& from, vkt::Buffer& to) { RecordCopy(cb, from, to, full_buffer); }
};
