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

#include "queue_submit_context.h"

QSTestContext::QSTestContext(vkt::Device* device, vkt::Queue* force_q0, vkt::Queue* force_q1)
    : dev(device), q0(VK_NULL_HANDLE), q1(VK_NULL_HANDLE) {
    if (force_q0) {
        q0 = force_q0->handle();
        q_fam = force_q0->family_index;
        if (force_q1) {
            // The object has some assumptions that the queues are from the the same family, so enforce this here
            if (force_q1->family_index == q_fam) {
                q1 = force_q1->handle();
            }
        } else {
            q1 = q0;  // Allow the two queues to be the same and valid if forced
        }
    } else {
        const auto& queues = device->QueuesWithTransferCapability();

        const uint32_t q_count = static_cast<uint32_t>(queues.size());
        for (uint32_t q0_index = 0; q0_index < q_count; ++q0_index) {
            const auto* q0_entry = queues[q0_index];
            q0 = q0_entry->handle();
            q_fam = q0_entry->family_index;
            for (uint32_t q1_index = (q0_index + 1); q1_index < q_count; ++q1_index) {
                const auto* q1_entry = queues[q1_index];
                if (q_fam == q1_entry->family_index) {
                    q1 = q1_entry->handle();
                    break;
                }
            }
            if (Valid()) {
                break;
            }
        }
    }

    if (!Valid()) return;

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_a.init(*device, 256, transfer_usage, mem_prop);
    buffer_b.init(*device, 256, transfer_usage, mem_prop);
    buffer_c.init(*device, 256, transfer_usage, mem_prop);

    VkDeviceSize size = 256;
    VkDeviceSize half_size = size / 2;
    full_buffer = {0, 0, size};
    first_half = {0, 0, half_size};
    second_half = {half_size, half_size, half_size};
    first_to_second = {0, half_size, half_size};
    second_to_first = {half_size, 0, half_size};

    pool.Init(*device, q_fam, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    h_cba = InitFromPool(cba);
    h_cbb = InitFromPool(cbb);
    h_cbc = InitFromPool(cbc);

    VkSemaphoreCreateInfo semaphore_ci = vku::InitStructHelper();
    semaphore.init(*device, semaphore_ci);

    VkEventCreateInfo eci = vku::InitStructHelper();
    event.init(*device, eci);
}

VkCommandBuffer QSTestContext::InitFromPool(vkt::CommandBuffer& cb_obj) {
    cb_obj.Init(*dev, pool);
    return cb_obj.handle();
}

void QSTestContext::Begin(vkt::CommandBuffer& cb) {
    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    info.pInheritanceInfo = nullptr;

    cb.Reset();
    cb.Begin(&info);
    current_cb = &cb;
}

void QSTestContext::End() {
    current_cb->End();
    current_cb = nullptr;
}

void QSTestContext::Copy(vkt::Buffer& from, vkt::Buffer& to, const VkBufferCopy& copy_region) {
    vk::CmdCopyBuffer(current_cb->handle(), from.handle(), to.handle(), 1, &copy_region);
}

void QSTestContext::CopyGeneral(const vkt::Image& from, const vkt::Image& to, const VkImageCopy& region) {
    vk::CmdCopyImage(current_cb->handle(), from.handle(), VK_IMAGE_LAYOUT_GENERAL, to.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region);
}

VkBufferMemoryBarrier QSTestContext::InitBufferBarrier(const vkt::Buffer& buffer, VkAccessFlags src, VkAccessFlags dst) {
    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = src;
    buffer_barrier.dstAccessMask = dst;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.offset = 0;
    buffer_barrier.size = 256;
    return buffer_barrier;
}

VkBufferMemoryBarrier QSTestContext::InitBufferBarrierRAW(const vkt::Buffer& buffer) {
    return InitBufferBarrier(buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
}

VkBufferMemoryBarrier QSTestContext::InitBufferBarrierWAR(const vkt::Buffer& buffer) {
    return InitBufferBarrier(buffer, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
}

void QSTestContext::TransferBarrier(const VkBufferMemoryBarrier& buffer_barrier) {
    vk::CmdPipelineBarrier(current_cb->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                           &buffer_barrier, 0, nullptr);
}

void QSTestContext::TransferBarrierWAR(const vkt::Buffer& buffer) { TransferBarrier(InitBufferBarrierWAR(buffer)); }
void QSTestContext::TransferBarrierRAW(const vkt::Buffer& buffer) { TransferBarrier(InitBufferBarrierRAW(buffer)); }

void QSTestContext::Submit(VkQueue q, vkt::CommandBuffer& cb, VkSemaphore wait, VkPipelineStageFlags wait_mask, VkSemaphore signal,
                           VkFence fence) {
    VkSubmitInfo submit1 = vku::InitStructHelper();
    submit1.commandBufferCount = 1;
    VkCommandBuffer h_cb = cb.handle();
    submit1.pCommandBuffers = &h_cb;
    if (wait != VK_NULL_HANDLE) {
        submit1.waitSemaphoreCount = 1;
        submit1.pWaitSemaphores = &wait;
        submit1.pWaitDstStageMask = &wait_mask;
    }
    if (signal != VK_NULL_HANDLE) {
        submit1.signalSemaphoreCount = 1;
        submit1.pSignalSemaphores = &signal;
    }
    vk::QueueSubmit(q, 1, &submit1, fence);
}

void QSTestContext::SubmitX(VkQueue q, vkt::CommandBuffer& cb, VkSemaphore wait, VkPipelineStageFlags wait_mask, VkSemaphore signal,
                            VkPipelineStageFlags signal_mask, VkFence fence) {
    VkSubmitInfo2 submit1 = vku::InitStructHelper();
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();

    cb_info.commandBuffer = cb.handle();
    submit1.commandBufferInfoCount = 1;
    submit1.pCommandBufferInfos = &cb_info;

    if (wait != VK_NULL_HANDLE) {
        wait_info.semaphore = wait;
        wait_info.stageMask = wait_mask;
        submit1.waitSemaphoreInfoCount = 1;
        submit1.pWaitSemaphoreInfos = &wait_info;
    }
    if (signal != VK_NULL_HANDLE) {
        signal_info.semaphore = signal;
        signal_info.stageMask = signal_mask;
        submit1.signalSemaphoreInfoCount = 1;
        submit1.pSignalSemaphoreInfos = &signal_info;
    }

    vk::QueueSubmit2(q, 1, &submit1, fence);
}

void QSTestContext::WaitEventBufferTransfer(vkt::Buffer& buffer, VkPipelineStageFlags src_mask, VkPipelineStageFlags dst_mask) {
    std::vector<VkBufferMemoryBarrier> buffer_barriers(1, InitBufferBarrierWAR(buffer));
    event.CmdWait(*current_cb, src_mask, dst_mask, std::vector<VkMemoryBarrier>(), buffer_barriers,
                  std::vector<VkImageMemoryBarrier>());
}

void QSTestContext::RecordCopy(vkt::CommandBuffer& cb, vkt::Buffer& from, vkt::Buffer& to, const VkBufferCopy& copy_region) {
    Begin(cb);
    Copy(from, to, copy_region);
    End();
}
