/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "barrier_queue_family.h"

BarrierQueueFamilyBase::QueueFamilyObjs::~QueueFamilyObjs() {
    delete command_buffer2;
    delete command_buffer;
    delete command_pool;
    delete queue;
}

void BarrierQueueFamilyBase::QueueFamilyObjs::Init(vkt::Device *device, uint32_t qf_index, VkQueue qf_queue,
                                                   VkCommandPoolCreateFlags cp_flags) {
    index = qf_index;
    queue = new vkt::Queue(qf_queue, qf_index);
    command_pool = new vkt::CommandPool(*device, qf_index, cp_flags);
    command_buffer = new vkt::CommandBuffer(*device, *command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    command_buffer2 = new vkt::CommandBuffer(*device, *command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

BarrierQueueFamilyBase::Context::Context(VkLayerTest *test, const std::vector<uint32_t> &queue_family_indices) : layer_test(test) {
    if (0 == queue_family_indices.size()) {
        return;  // This is invalid
    }
    vkt::Device *device_obj = layer_test->DeviceObj();
    queue_families.reserve(queue_family_indices.size());
    default_index = queue_family_indices[0];
    for (auto qfi : queue_family_indices) {
        VkQueue queue = device_obj->QueuesFromFamily(qfi)[0]->handle();
        queue_families.emplace(std::make_pair(qfi, QueueFamilyObjs()));
        queue_families[qfi].Init(device_obj, qfi, queue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }
    Reset();
}

void BarrierQueueFamilyBase::Context::Reset() {
    layer_test->DeviceObj()->Wait();
    for (auto &qf : queue_families) {
        vk::ResetCommandPool(layer_test->device(), qf.second.command_pool->handle(), 0);
    }
}

void BarrierQueueFamilyTestHelper::Init(std::vector<uint32_t> *families, bool image_memory, bool buffer_memory) {
    vkt::Device *device_obj = context_->layer_test->DeviceObj();

    auto image_ci = vkt::Image::ImageCreateInfo2D(
        32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
        families ? vvl::make_span(families->data(), families->size()) : vvl::span<uint32_t>{});
    if (image_memory) {
        image_.init(*device_obj, image_ci, 0);
        image_.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    } else {
        image_.InitNoMemory(*device_obj, image_ci);
    }

    image_barrier_ = image_.ImageMemoryBarrier(VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, image_.Layout(),
                                               image_.Layout(), image_.SubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1));

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto buffer_ci = vkt::Buffer::CreateInfo(256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                             families ? vvl::make_span(families->data(), families->size()) : vvl::span<uint32_t>{});
    if (buffer_memory) {
        buffer_.init(*device_obj, buffer_ci, mem_prop);
    } else {
        buffer_.InitNoMemory(*device_obj, buffer_ci);
    }
    ASSERT_TRUE(buffer_.initialized());
    buffer_barrier_ = buffer_.BufferMemoryBarrier(VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, 0, VK_WHOLE_SIZE);
}

void Barrier2QueueFamilyTestHelper::Init(bool image_memory, bool buffer_memory) {
    vkt::Device *device_obj = context_->layer_test->DeviceObj();

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                  VK_IMAGE_TILING_OPTIMAL);
    if (image_memory) {
        image_.init(*device_obj, image_ci, 0);
        image_.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    } else {
        image_.InitNoMemory(*device_obj, image_ci);
    }

    image_barrier_ = image_.ImageMemoryBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                               VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, image_.Layout(),
                                               image_.Layout(), image_.SubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1));

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto buffer_ci = vkt::Buffer::CreateInfo(256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (buffer_memory) {
        buffer_.init(*device_obj, buffer_ci, mem_prop);
    } else {
        buffer_.InitNoMemory(*device_obj, buffer_ci);
    }
    ASSERT_TRUE(buffer_.initialized());
    buffer_barrier_ = buffer_.BufferMemoryBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                  VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, 0, VK_WHOLE_SIZE);
}

BarrierQueueFamilyBase::QueueFamilyObjs *BarrierQueueFamilyBase::GetQueueFamilyInfo(Context *context, uint32_t qfi) {
    QueueFamilyObjs *qf;

    auto qf_it = context->queue_families.find(qfi);
    if (qf_it != context->queue_families.end()) {
        qf = &(qf_it->second);
    } else {
        qf = &(context->queue_families[context->default_index]);
    }
    return qf;
}

void BarrierQueueFamilyTestHelper::operator()(const std::string &img_err, const std::string &buf_err, uint32_t src, uint32_t dst,
                                              uint32_t queue_family_index, Modifier mod) {
    auto &monitor = context_->layer_test->Monitor();
    const bool has_img_err = img_err.size() > 0;
    const bool has_buf_err = buf_err.size() > 0;
    bool positive = !has_img_err && !has_buf_err;
    if (has_img_err) monitor.SetDesiredFailureMsg(kErrorBit | kWarningBit, img_err);
    if (has_buf_err) monitor.SetDesiredFailureMsg(kErrorBit | kWarningBit, buf_err);

    image_barrier_.srcQueueFamilyIndex = src;
    image_barrier_.dstQueueFamilyIndex = dst;
    buffer_barrier_.srcQueueFamilyIndex = src;
    buffer_barrier_.dstQueueFamilyIndex = dst;

    QueueFamilyObjs *qf = GetQueueFamilyInfo(context_, queue_family_index);

    vkt::CommandBuffer *command_buffer = qf->command_buffer;
    for (int cb_repeat = 0; cb_repeat < (mod == Modifier::DOUBLE_COMMAND_BUFFER ? 2 : 1); cb_repeat++) {
        command_buffer->Begin();
        for (int repeat = 0; repeat < (mod == Modifier::DOUBLE_RECORD ? 2 : 1); repeat++) {
            vk::CmdPipelineBarrier(command_buffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                   VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &buffer_barrier_, 1, &image_barrier_);
        }
        command_buffer->End();
        command_buffer = qf->command_buffer2;  // Second pass (if any) goes to the secondary command_buffer.
    }

    if (queue_family_index != kInvalidQueueFamily) {
        if (mod == Modifier::DOUBLE_COMMAND_BUFFER) {
            // no wait after submit
            std::array command_buffers{qf->command_buffer, qf->command_buffer2};
            qf->queue->Submit(command_buffers);
        } else {
            qf->queue->Submit(*qf->command_buffer);
            qf->queue->Wait();
        }
    }

    if (!positive) {
        monitor.VerifyFound();
    }
    context_->Reset();
}

void Barrier2QueueFamilyTestHelper::operator()(const std::string &img_err, const std::string &buf_err, uint32_t src, uint32_t dst,
                                               uint32_t queue_family_index, Modifier mod) {
    auto &monitor = context_->layer_test->Monitor();
    bool positive = true;
    if (img_err.length()) {
        monitor.SetDesiredFailureMsg(kErrorBit | kWarningBit, img_err);
        positive = false;
    }
    if (buf_err.length()) {
        monitor.SetDesiredFailureMsg(kErrorBit | kWarningBit, buf_err);
        positive = false;
    }

    image_barrier_.srcQueueFamilyIndex = src;
    image_barrier_.dstQueueFamilyIndex = dst;
    buffer_barrier_.srcQueueFamilyIndex = src;
    buffer_barrier_.dstQueueFamilyIndex = dst;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &buffer_barrier_;
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &image_barrier_;

    QueueFamilyObjs *qf = GetQueueFamilyInfo(context_, queue_family_index);

    vkt::CommandBuffer *command_buffer = qf->command_buffer;
    for (int cb_repeat = 0; cb_repeat < (mod == Modifier::DOUBLE_COMMAND_BUFFER ? 2 : 1); cb_repeat++) {
        command_buffer->Begin();
        for (int repeat = 0; repeat < (mod == Modifier::DOUBLE_RECORD ? 2 : 1); repeat++) {
            vk::CmdPipelineBarrier2KHR(command_buffer->handle(), &dep_info);
        }
        command_buffer->End();
        command_buffer = qf->command_buffer2;  // Second pass (if any) goes to the secondary command_buffer.
    }

    if (queue_family_index != kInvalidQueueFamily) {
        if (mod == Modifier::DOUBLE_COMMAND_BUFFER) {
            // no wait after submit
            std::array command_buffers{qf->command_buffer, qf->command_buffer2};
            qf->queue->Submit(command_buffers);
        } else {
            qf->queue->Submit(*qf->command_buffer);
            qf->queue->Wait();
        }
    }

    if (!positive) {
        monitor.VerifyFound();
    }
    context_->Reset();
}

void ValidOwnershipTransferOp(ErrorMonitor *monitor, vkt::Queue *queue, vkt::CommandBuffer &cb, VkPipelineStageFlags src_stages,
                              VkPipelineStageFlags dst_stages, const VkBufferMemoryBarrier *buf_barrier,
                              const VkImageMemoryBarrier *img_barrier) {
    cb.Begin();
    uint32_t num_buf_barrier = (buf_barrier) ? 1 : 0;
    uint32_t num_img_barrier = (img_barrier) ? 1 : 0;
    vk::CmdPipelineBarrier(cb.handle(), src_stages, dst_stages, 0, 0, nullptr, num_buf_barrier, buf_barrier, num_img_barrier,
                           img_barrier);
    cb.End();
    queue->Submit(cb);
    queue->Wait();
}

void ValidOwnershipTransfer(ErrorMonitor *monitor, vkt::Queue *queue_from, vkt::CommandBuffer &cb_from, vkt::Queue *queue_to,
                            vkt::CommandBuffer &cb_to, VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages,
                            const VkBufferMemoryBarrier *buf_barrier, const VkImageMemoryBarrier *img_barrier) {
    ValidOwnershipTransferOp(monitor, queue_from, cb_from, src_stages, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, buf_barrier,
                             img_barrier);
    ValidOwnershipTransferOp(monitor, queue_to, cb_to, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dst_stages, buf_barrier, img_barrier);
}

void ValidOwnershipTransferOp(ErrorMonitor *monitor, vkt::Queue *queue, vkt::CommandBuffer &cb,
                              const VkBufferMemoryBarrier2 *buf_barrier, const VkImageMemoryBarrier2 *img_barrier) {
    cb.Begin();
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = (buf_barrier) ? 1 : 0;
    dep_info.pBufferMemoryBarriers = buf_barrier;
    dep_info.imageMemoryBarrierCount = (img_barrier) ? 1 : 0;
    dep_info.pImageMemoryBarriers = img_barrier;
    vk::CmdPipelineBarrier2KHR(cb.handle(), &dep_info);
    cb.End();
    queue->Submit(cb);
    queue->Wait();
}

void ValidOwnershipTransfer(ErrorMonitor *monitor, vkt::Queue *queue_from, vkt::CommandBuffer &cb_from, vkt::Queue *queue_to,
                            vkt::CommandBuffer &cb_to, const VkBufferMemoryBarrier2 *buf_barrier,
                            const VkImageMemoryBarrier2 *img_barrier) {
    VkBufferMemoryBarrier2 fixup_buf_barrier;
    VkImageMemoryBarrier2 fixup_img_barrier;
    if (buf_barrier) {
        fixup_buf_barrier = *buf_barrier;
        fixup_buf_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
        fixup_buf_barrier.dstAccessMask = 0;
    }
    if (img_barrier) {
        fixup_img_barrier = *img_barrier;
        fixup_img_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
        fixup_img_barrier.dstAccessMask = 0;
    }

    ValidOwnershipTransferOp(monitor, queue_from, cb_from, buf_barrier ? &fixup_buf_barrier : nullptr,
                             img_barrier ? &fixup_img_barrier : nullptr);

    if (buf_barrier) {
        fixup_buf_barrier = *buf_barrier;
        fixup_buf_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        fixup_buf_barrier.srcAccessMask = 0;
    }
    if (img_barrier) {
        fixup_img_barrier = *img_barrier;
        fixup_img_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        fixup_img_barrier.srcAccessMask = 0;
    }
    ValidOwnershipTransferOp(monitor, queue_to, cb_to, buf_barrier ? &fixup_buf_barrier : nullptr,
                             img_barrier ? &fixup_img_barrier : nullptr);
}
