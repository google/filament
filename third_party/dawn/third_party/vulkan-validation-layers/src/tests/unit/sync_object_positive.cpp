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

#include <thread>
#include "../framework/layer_validation_tests.h"
#include "../framework/external_memory_sync.h"
#include "../framework/barrier_queue_family.h"
#include "../framework/render_pass_helper.h"

#ifndef VK_USE_PLATFORM_WIN32_KHR
#include <poll.h>
#endif

class PositiveSyncObject : public SyncObjectTest {};

TEST_F(PositiveSyncObject, Sync2OwnershipTranfersImage) {
    TEST_DESCRIPTION("Valid image ownership transfers that shouldn't create errors");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Queue *no_gfx_queue = m_device->NonGraphicsQueue();
    if (!no_gfx_queue) {
        GTEST_SKIP() << "Required queue not present (non-graphics capable required)";
    }

    vkt::CommandPool no_gfx_pool(*m_device, no_gfx_queue->family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer no_gfx_cb(*m_device, no_gfx_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Create an "exclusive" image owned by the graphics queue.
    VkFlags image_use = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, image_use);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    auto image_subres = image.SubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    auto image_barrier = image.ImageMemoryBarrier(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                                                  image.Layout(), image.Layout(), image_subres);
    image_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.dstQueueFamilyIndex = no_gfx_queue->family_index;

    ValidOwnershipTransfer(m_errorMonitor, m_default_queue, m_command_buffer, no_gfx_queue, no_gfx_cb, nullptr, &image_barrier);

    // Change layouts while changing ownership
    image_barrier.srcQueueFamilyIndex = no_gfx_queue->family_index;
    image_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    image_barrier.oldLayout = image.Layout();
    // Make sure the new layout is different from the old
    if (image_barrier.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } else {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    ValidOwnershipTransfer(m_errorMonitor, no_gfx_queue, no_gfx_cb, m_default_queue, m_command_buffer, nullptr, &image_barrier);
}

TEST_F(PositiveSyncObject, Sync2OwnershipTranfersBuffer) {
    TEST_DESCRIPTION("Valid buffer ownership transfers that shouldn't create errors");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Queue *no_gfx_queue = m_device->NonGraphicsQueue();
    if (!no_gfx_queue) {
        GTEST_SKIP() << "Required queue not present (non-graphics capable required)";
    }

    vkt::CommandPool no_gfx_pool(*m_device, no_gfx_queue->family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer no_gfx_cb(*m_device, no_gfx_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    auto buffer_barrier = buffer.BufferMemoryBarrier(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                                     VK_ACCESS_2_NONE, VK_ACCESS_2_NONE, 0, VK_WHOLE_SIZE);

    // Let gfx own it.
    buffer_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    ValidOwnershipTransferOp(m_errorMonitor, m_default_queue, m_command_buffer, &buffer_barrier, nullptr);

    // Transfer it to non-gfx
    buffer_barrier.dstQueueFamilyIndex = no_gfx_queue->family_index;
    ValidOwnershipTransfer(m_errorMonitor, m_default_queue, m_command_buffer, no_gfx_queue, no_gfx_cb, &buffer_barrier, nullptr);

    // Transfer it to gfx
    buffer_barrier.srcQueueFamilyIndex = no_gfx_queue->family_index;
    buffer_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

    ValidOwnershipTransfer(m_errorMonitor, no_gfx_queue, no_gfx_cb, m_default_queue, m_command_buffer, &buffer_barrier, nullptr);
}

TEST_F(PositiveSyncObject, LayoutFromPresentWithoutAccessMemoryRead) {
    // Transition an image away from PRESENT_SRC_KHR without ACCESS_MEMORY_READ
    // in srcAccessMask.

    // The required behavior here was a bit unclear in earlier versions of the
    // spec, but there is no memory dependency required here, so this should
    // work without warnings.

    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    VkImageSubresourceRange range;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.image = image.handle();
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;
    barrier.subresourceRange = range;
    vkt::CommandBuffer cmdbuf(*m_device, m_command_pool);
    cmdbuf.Begin();
    vk::CmdPipelineBarrier(cmdbuf.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &barrier);
    barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cmdbuf.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &barrier);
}

TEST_F(PositiveSyncObject, QueueSubmitSemaphoresAndLayoutTracking) {
    TEST_DESCRIPTION("Submit multiple command buffers with chained semaphore signals and layout transitions");

    RETURN_IF_SKIP(Init());
    VkCommandBuffer cmd_bufs[4];
    VkCommandBufferAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.commandBufferCount = 4;
    alloc_info.commandPool = m_command_pool.handle();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &alloc_info, cmd_bufs);
    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
    image.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkCommandBufferBeginInfo cb_binfo = vku::InitStructHelper();
    cb_binfo.pInheritanceInfo = VK_NULL_HANDLE;
    cb_binfo.flags = 0;
    // Use 4 command buffers, each with an image layout transition, ColorAO->General->ColorAO->TransferSrc->TransferDst
    vk::BeginCommandBuffer(cmd_bufs[0], &cb_binfo);
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(cmd_bufs[0], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    vk::EndCommandBuffer(cmd_bufs[0]);
    vk::BeginCommandBuffer(cmd_bufs[1], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vk::CmdPipelineBarrier(cmd_bufs[1], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    vk::EndCommandBuffer(cmd_bufs[1]);
    vk::BeginCommandBuffer(cmd_bufs[2], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vk::CmdPipelineBarrier(cmd_bufs[2], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    vk::EndCommandBuffer(cmd_bufs[2]);
    vk::BeginCommandBuffer(cmd_bufs[3], &cb_binfo);
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vk::CmdPipelineBarrier(cmd_bufs[3], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    vk::EndCommandBuffer(cmd_bufs[3]);

    // Submit 4 command buffers in 3 submits, with submits 2 and 3 waiting for semaphores from submits 1 and 2
    vkt::Semaphore semaphore1(*m_device);
    vkt::Semaphore semaphore2(*m_device);
    VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
    VkSubmitInfo submit_info[3];
    submit_info[0] = vku::InitStructHelper();
    submit_info[0].commandBufferCount = 1;
    submit_info[0].pCommandBuffers = &cmd_bufs[0];
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore1.handle();
    submit_info[0].waitSemaphoreCount = 0;
    submit_info[0].pWaitDstStageMask = nullptr;
    submit_info[0].pWaitDstStageMask = flags;
    submit_info[1] = vku::InitStructHelper();
    submit_info[1].commandBufferCount = 1;
    submit_info[1].pCommandBuffers = &cmd_bufs[1];
    submit_info[1].waitSemaphoreCount = 1;
    submit_info[1].pWaitSemaphores = &semaphore1.handle();
    submit_info[1].signalSemaphoreCount = 1;
    submit_info[1].pSignalSemaphores = &semaphore2.handle();
    submit_info[1].pWaitDstStageMask = flags;
    submit_info[2] = vku::InitStructHelper();
    submit_info[2].commandBufferCount = 2;
    submit_info[2].pCommandBuffers = &cmd_bufs[2];
    submit_info[2].waitSemaphoreCount = 1;
    submit_info[2].pWaitSemaphores = &semaphore2.handle();
    submit_info[2].signalSemaphoreCount = 0;
    submit_info[2].pSignalSemaphores = nullptr;
    submit_info[2].pWaitDstStageMask = flags;
    vk::QueueSubmit(m_default_queue->handle(), 3, submit_info, VK_NULL_HANDLE);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, ResetUnsignaledFence) {
    RETURN_IF_SKIP(Init());
    vkt::Fence testFence(*m_device);
    VkFence fences[1] = {testFence.handle()};
    VkResult result = vk::ResetFences(device(), 1, fences);
    ASSERT_EQ(VK_SUCCESS, result);
}

TEST_F(PositiveSyncObject, FenceCreateSignaledWaitHandling) {
    RETURN_IF_SKIP(Init());

    // A fence created signaled
    VkFenceCreateInfo fci = vku::InitStructHelper();
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkt::Fence f1(*m_device, fci);

    // A fence created not
    fci.flags = 0;
    vkt::Fence f2(*m_device, fci);

    // Submit the unsignaled fence
    m_default_queue->Submit(vkt::no_cmd, f2);

    // Wait on both fences, with signaled first.
    VkFence fences[] = {f1.handle(), f2.handle()};
    vk::WaitForFences(device(), 2, fences, VK_TRUE, kWaitTimeout);
    // Should have both retired! (get destroyed now)
}

TEST_F(PositiveSyncObject, TwoFencesThreeFrames) {
    TEST_DESCRIPTION(
        "Two command buffers with two separate fences are each run through a Submit & WaitForFences cycle 3 times. This previously "
        "revealed a bug so running this positive test to prevent a regression.");

    RETURN_IF_SKIP(Init());

    constexpr uint32_t NUM_OBJECTS = 2;
    constexpr uint32_t NUM_FRAMES = 3;
    vkt::CommandBuffer cmd_buffers[NUM_OBJECTS];
    vkt::Fence fences[NUM_OBJECTS];

    for (uint32_t i = 0; i < NUM_OBJECTS; ++i) {
        cmd_buffers[i].Init(*m_device, m_command_pool);
        fences[i].Init(*m_device);
    }
    for (uint32_t frame = 0; frame < NUM_FRAMES; ++frame) {
        for (uint32_t obj = 0; obj < NUM_OBJECTS; ++obj) {
            // Create empty cmd buffer
            VkCommandBufferBeginInfo cmdBufBeginDesc = vku::InitStructHelper();
            vk::BeginCommandBuffer(cmd_buffers[obj], &cmdBufBeginDesc);
            vk::EndCommandBuffer(cmd_buffers[obj]);

            // Submit cmd buffer and wait for fence
            m_default_queue->Submit(cmd_buffers[obj], fences[obj]);
            vk::WaitForFences(device(), 1, &fences[obj].handle(), VK_TRUE, kWaitTimeout);
            vk::ResetFences(device(), 1, &fences[obj].handle());
        }
    }
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceQWI) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues followed by a QueueWaitIdle.");

    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());
    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    vkt::Semaphore semaphore(*m_device);
    vkt::CommandPool pool0(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb0(*m_device, pool0);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;

    cb0.Begin();
    vk::CmdPipelineBarrier(cb0, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr,
                           0, nullptr);
    vk::CmdSetViewport(cb0, 0, 1, &viewport);
    cb0.End();

    cb1.Begin();
    vk::CmdSetViewport(cb1, 0, 1, &viewport);
    cb1.End();

    m_second_queue->Submit(cb0, vkt::Signal(semaphore));
    m_default_queue->Submit(cb1, vkt::Wait(semaphore));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceQWIFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence followed "
        "by a QueueWaitIdle.");

    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());
    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    vkt::Fence fence(*m_device);
    vkt::Semaphore semaphore(*m_device);
    vkt::CommandPool pool0(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb0(*m_device, pool0);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;

    cb0.Begin();
    vk::CmdPipelineBarrier(cb0, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr,
                           0, nullptr);
    vk::CmdSetViewport(cb0, 0, 1, &viewport);
    cb0.End();

    cb1.Begin();
    vk::CmdSetViewport(cb1, 0, 1, &viewport);
    cb1.End();

    m_second_queue->Submit(cb0, vkt::Signal(semaphore));
    m_default_queue->Submit(cb1, vkt::Wait(semaphore), fence);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFenceTwoWFF) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence followed "
        "by two consecutive WaitForFences calls on the same fence.");

    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());
    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    vkt::Fence fence(*m_device);
    vkt::Semaphore semaphore(*m_device);
    vkt::CommandPool pool0(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb0(*m_device, pool0);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;

    cb0.Begin();
    vk::CmdPipelineBarrier(cb0.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);
    vk::CmdSetViewport(cb0.handle(), 0, 1, &viewport);
    cb0.End();

    cb1.Begin();
    vk::CmdSetViewport(cb1.handle(), 0, 1, &viewport);
    cb1.End();

    m_second_queue->Submit(cb0, vkt::Signal(semaphore));
    m_default_queue->Submit(cb1, vkt::Wait(semaphore), fence);

    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(PositiveSyncObject, TwoQueuesEnsureCorrectRetirementWithWorkStolen) {
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());
    if (!m_second_queue) {
        GTEST_SKIP() << "Test requires two queues";
    }

    // An (empty) command buffer. We must have work in the first submission --
    // the layer treats unfenced work differently from fenced work.
    m_command_buffer.Begin();
    m_command_buffer.End();

    vkt::Semaphore s(*m_device);
    m_default_queue->Submit(m_command_buffer, vkt::Signal(s));
    m_second_queue->Submit(vkt::no_cmd, vkt::Wait(s));

    m_default_queue->Wait();
    m_device->Wait();
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsSeparateQueuesWithSemaphoreAndOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, the second having a fence, "
        "followed by a WaitForFences call.");

    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());
    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    vkt::Fence fence(*m_device);
    vkt::Semaphore semaphore(*m_device);
    vkt::CommandPool pool0(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb0(*m_device, pool0);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;

    cb0.Begin();
    vk::CmdPipelineBarrier(cb0.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);
    vk::CmdSetViewport(cb0.handle(), 0, 1, &viewport);
    cb0.End();

    cb1.Begin();
    vk::CmdSetViewport(cb1.handle(), 0, 1, &viewport);
    cb1.End();

    m_second_queue->Submit(cb0, vkt::Signal(semaphore));
    m_default_queue->Submit(cb1, vkt::Wait(semaphore), fence);
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsSeparateQueuesWithTimelineSemaphoreAndOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call submitted on separate queues, ordered by a timeline semaphore,"
        " the second having a fence, followed by a WaitForFences call.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());
    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    vkt::Fence fence(*m_device);
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::CommandPool pool0(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb0(*m_device, pool0);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;

    cb0.Begin();
    vk::CmdPipelineBarrier(cb0.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);
    vk::CmdSetViewport(cb0.handle(), 0, 1, &viewport);
    cb0.End();

    cb1.Begin();
    vk::CmdSetViewport(cb1.handle(), 0, 1, &viewport);
    cb1.End();

    m_second_queue->Submit(cb0, vkt::TimelineSignal(semaphore, 1));
    m_default_queue->Submit(cb1, vkt::TimelineWait(semaphore, 1), fence);
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsOneQueueWithSemaphoreAndOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call on the same queue, sharing a signal/wait semaphore, the second "
        "having a fence, followed by a WaitForFences call.");

    RETURN_IF_SKIP(Init());
    vkt::Fence fence(*m_device);
    vkt::Semaphore semaphore(*m_device);
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;

    cb0.Begin();
    vk::CmdPipelineBarrier(cb0.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);
    vk::CmdSetViewport(cb0.handle(), 0, 1, &viewport);
    cb0.End();

    cb1.Begin();
    vk::CmdSetViewport(cb1.handle(), 0, 1, &viewport);
    cb1.End();

    m_default_queue->Submit(cb0, vkt::Signal(semaphore));
    m_default_queue->Submit(cb1, vkt::Wait(semaphore), fence);
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsOneQueueNullQueueSubmitWithFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call on the same queue, no fences, followed by a third QueueSubmit "
        "with NO SubmitInfos but with a fence, followed by a WaitForFences call.");

    RETURN_IF_SKIP(Init());
    vkt::Fence fence(*m_device);
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;

    cb0.Begin();
    vk::CmdPipelineBarrier(cb0.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);
    vk::CmdSetViewport(cb0.handle(), 0, 1, &viewport);
    cb0.End();

    cb1.Begin();
    vk::CmdSetViewport(cb1.handle(), 0, 1, &viewport);
    cb1.End();

    m_default_queue->Submit(cb0);
    m_default_queue->Submit(cb1);
    m_default_queue->Submit(vkt::no_cmd, fence);
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(PositiveSyncObject, TwoQueueSubmitsOneQueueOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers, each in a separate QueueSubmit call on the same queue, the second having a fence, followed by a "
        "WaitForFences call.");

    RETURN_IF_SKIP(Init());
    vkt::Fence fence(*m_device);
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;

    cb0.Begin();
    vk::CmdPipelineBarrier(cb0.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);
    vk::CmdSetViewport(cb0.handle(), 0, 1, &viewport);
    cb0.End();

    cb1.Begin();
    vk::CmdSetViewport(cb1.handle(), 0, 1, &viewport);
    cb1.End();

    m_default_queue->Submit(cb0);
    m_default_queue->Submit(cb1, fence);
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(PositiveSyncObject, TwoSubmitInfosWithSemaphoreOneQueueSubmitsOneFence) {
    TEST_DESCRIPTION(
        "Two command buffers each in a separate SubmitInfo sent in a single QueueSubmit call followed by a WaitForFences call.");

    RETURN_IF_SKIP(Init());
    vkt::Fence fence(*m_device);
    vkt::Semaphore semaphore(*m_device);
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;

    cb0.Begin();
    vk::CmdPipelineBarrier(cb0.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);
    vk::CmdSetViewport(cb0.handle(), 0, 1, &viewport);
    cb0.End();

    cb1.Begin();
    vk::CmdSetViewport(cb1.handle(), 0, 1, &viewport);
    cb1.End();

    VkSubmitInfo submit_info[2];
    VkPipelineStageFlags flags[]{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};

    submit_info[0] = vku::InitStructHelper();
    submit_info[0].commandBufferCount = 1;
    submit_info[0].pCommandBuffers = &cb0.handle();
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore.handle();

    submit_info[1] = vku::InitStructHelper();
    submit_info[1].commandBufferCount = 1;
    submit_info[1].pCommandBuffers = &cb1.handle();
    submit_info[1].waitSemaphoreCount = 1;
    submit_info[1].pWaitSemaphores = &semaphore.handle();
    submit_info[1].pWaitDstStageMask = flags;
    vk::QueueSubmit(m_default_queue->handle(), 2, &submit_info[0], fence.handle());
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(PositiveSyncObject, WaitBeforeSignalOnDifferentQueuesSignalLargerThanWait) {
    TEST_DESCRIPTION("Wait before signal on different queues, signal value is larger than the wait value");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::CommandPool second_pool(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer second_cb(*m_device, second_pool);

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    VkBufferCopy region = {0, 0, 256};

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    // Wait for value 1 (or greater)
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer, buffer_a, buffer_b, 1, &region);
    m_command_buffer.End();
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1, VK_PIPELINE_STAGE_2_COPY_BIT));

    // Signal value 2
    second_cb.Begin();
    vk::CmdCopyBuffer(second_cb, buffer_a, buffer_b, 1, &region);
    second_cb.End();
    m_second_queue->Submit2(second_cb, vkt::TimelineSignal(semaphore, 2, VK_PIPELINE_STAGE_2_COPY_BIT));

    m_device->Wait();
}

TEST_F(PositiveSyncObject, LongSemaphoreChain) {
    RETURN_IF_SKIP(Init());
    std::vector<VkSemaphore> semaphores;

    const int chainLength = 32768;
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    for (int i = 0; i < chainLength; i++) {
        VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
        VkSemaphore semaphore;
        vk::CreateSemaphore(device(), &sci, nullptr, &semaphore);

        semaphores.push_back(semaphore);

        VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
                           nullptr,
                           semaphores.size() > 1 ? 1u : 0u,
                           semaphores.size() > 1 ? &semaphores[semaphores.size() - 2] : nullptr,
                           &flags,
                           0,
                           nullptr,
                           1,
                           &semaphores[semaphores.size() - 1]};
        vk::QueueSubmit(m_default_queue->handle(), 1, &si, VK_NULL_HANDLE);
    }

    vkt::Fence fence(*m_device);
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &semaphores.back(), &flags, 0, nullptr, 0, nullptr};
    vk::QueueSubmit(m_default_queue->handle(), 1, &si, fence.handle());

    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    for (auto semaphore : semaphores) vk::DestroySemaphore(device(), semaphore, nullptr);
}

TEST_F(PositiveSyncObject, ExternalSemaphore) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfo esi = vku::InitStructHelper();
    esi.handleType = handle_type;
    VkExternalSemaphorePropertiesKHR esp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(Gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting";
    }

    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfo esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&esci);

    vkt::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore to import payload into
    sci.pNext = nullptr;
    vkt::Semaphore import_semaphore(*m_device, sci);

    ExternalHandle ext_handle{};
    export_semaphore.ExportHandle(ext_handle, handle_type);
    import_semaphore.ImportHandle(ext_handle, handle_type);

    // Signal the exported semaphore and wait on the imported semaphore
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    std::vector<VkSubmitInfo> si(4, vku::InitStruct<VkSubmitInfo>());
    si[0].signalSemaphoreCount = 1;
    si[0].pSignalSemaphores = &export_semaphore.handle();
    si[1].pWaitDstStageMask = &flags;
    si[1].waitSemaphoreCount = 1;
    si[1].pWaitSemaphores = &import_semaphore.handle();
    si[2] = si[0];
    si[3] = si[1];

    vk::QueueSubmit(m_default_queue->handle(), si.size(), si.data(), VK_NULL_HANDLE);

    if (m_device->Physical().Features().sparseBinding) {
        // Signal the imported semaphore and wait on the exported semaphore
        std::vector<VkBindSparseInfo> bi(4, vku::InitStruct<VkBindSparseInfo>());
        bi[0].signalSemaphoreCount = 1;
        bi[0].pSignalSemaphores = &export_semaphore.handle();
        bi[1].waitSemaphoreCount = 1;
        bi[1].pWaitSemaphores = &import_semaphore.handle();
        bi[2] = bi[0];
        bi[3] = bi[1];
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), bi.size(), bi.data(), VK_NULL_HANDLE);
    }

    // Cleanup
    m_device->Wait();
}

TEST_F(PositiveSyncObject, ExternalTimelineSemaphore) {
    TEST_DESCRIPTION(
        "Export and import a timeline semaphore. "
        "Should be roughly equivalant to the CTS *cross_instance*timeline_semaphore* tests");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    // Check for external semaphore import and export capability
    VkSemaphoreTypeCreateInfo tci = vku::InitStructHelper();
    tci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkPhysicalDeviceExternalSemaphoreInfo esi = vku::InitStructHelper(&tci);
    esi.handleType = handle_type;

    VkExternalSemaphorePropertiesKHR esp = vku::InitStructHelper();

    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(Gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
    }

    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfo esci = vku::InitStructHelper();
    esci.handleTypes = handle_type;
    VkSemaphoreTypeCreateInfo stci = vku::InitStructHelper(&esci);
    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    VkSemaphoreCreateInfo sci = vku::InitStructHelper(&stci);

    vkt::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore to import payload into
    stci.pNext = nullptr;
    vkt::Semaphore import_semaphore(*m_device, sci);

    ExternalHandle ext_handle{};
    export_semaphore.ExportHandle(ext_handle, handle_type);
    import_semaphore.ImportHandle(ext_handle, handle_type);

    uint64_t wait_value = 1;
    uint64_t signal_value = 12345;

    // Signal the exported semaphore and wait on the imported semaphore
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    std::vector<VkTimelineSemaphoreSubmitInfo> ti(2, vku::InitStruct<VkTimelineSemaphoreSubmitInfo>());
    std::vector<VkSubmitInfo> si(2, vku::InitStruct<VkSubmitInfo>());

    si[0].pWaitDstStageMask = &flags;
    si[0].signalSemaphoreCount = 1;
    si[0].pSignalSemaphores = &export_semaphore.handle();
    si[0].pNext = &ti[0];
    ti[0].signalSemaphoreValueCount = 1;
    ti[0].pSignalSemaphoreValues = &signal_value;
    si[1].waitSemaphoreCount = 1;
    si[1].pWaitSemaphores = &import_semaphore.handle();
    si[1].pWaitDstStageMask = &flags;
    si[1].pNext = &ti[1];
    ti[1].waitSemaphoreValueCount = 1;
    ti[1].pWaitSemaphoreValues = &wait_value;

    vk::QueueSubmit(m_default_queue->handle(), si.size(), si.data(), VK_NULL_HANDLE);

    m_default_queue->Wait();

    uint64_t import_value{0}, export_value{0};

    vk::GetSemaphoreCounterValueKHR(m_device->handle(), export_semaphore.handle(), &export_value);
    ASSERT_EQ(export_value, signal_value);

    vk::GetSemaphoreCounterValueKHR(m_device->handle(), import_semaphore.handle(), &import_value);
    ASSERT_EQ(import_value, signal_value);
}

TEST_F(PositiveSyncObject, ExternalFence) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    RETURN_IF_SKIP(Init());

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfo efi = vku::InitStructHelper();
    efi.handleType = handle_type;
    VkExternalFencePropertiesKHR efp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(Gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    // Create a fence to export payload from
    VkExportFenceCreateInfo efci = vku::InitStructHelper();
    efci.handleTypes = handle_type;
    VkFenceCreateInfo fci = vku::InitStructHelper(&efci);
    vkt::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vkt::Fence import_fence(*m_device, fci);

    // Export fence payload to an opaque handle
    ExternalHandle ext_fence{};
    export_fence.ExportHandle(ext_fence, handle_type);
    import_fence.ImportHandle(ext_fence, handle_type);

    // Signal the exported fence and wait on the imported fence
    vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, export_fence.handle());
    vk::WaitForFences(device(), 1, &import_fence.handle(), VK_TRUE, 1000000000);
    vk::ResetFences(device(), 1, &import_fence.handle());
    vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, export_fence.handle());
    vk::WaitForFences(device(), 1, &import_fence.handle(), VK_TRUE, 1000000000);
    vk::ResetFences(device(), 1, &import_fence.handle());

    // Signal the imported fence and wait on the exported fence
    vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, import_fence.handle());
    vk::WaitForFences(device(), 1, &export_fence.handle(), VK_TRUE, 1000000000);
    vk::ResetFences(device(), 1, &export_fence.handle());
    vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, import_fence.handle());
    vk::WaitForFences(device(), 1, &export_fence.handle(), VK_TRUE, 1000000000);
    vk::ResetFences(device(), 1, &export_fence.handle());

    // Cleanup
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, ExternalFenceSyncFdLoop) {
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    RETURN_IF_SKIP(Init());

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfo efi = vku::InitStructHelper();
    efi.handleType = handle_type;
    VkExternalFencePropertiesKHR efp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(Gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
        return;
    }

    // Create a fence to export payload from
    VkExportFenceCreateInfo efci = vku::InitStructHelper();
    efci.handleTypes = handle_type;
    VkFenceCreateInfo fci = vku::InitStructHelper(&efci);
    vkt::Fence export_fence(*m_device, fci);

    fci.pNext = nullptr;
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    std::array<vkt::Fence, 2> fences;
    fences[0].Init(*m_device, fci);
    fences[1].Init(*m_device, fci);

    for (uint32_t i = 0; i < 1000; i++) {
        auto submitter = i & 1;
        auto waiter = (~i) & 1;
        fences[submitter].Reset();
        vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, fences[submitter].handle());

        fences[waiter].Wait(kWaitTimeout);

        vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, export_fence.handle());
        int fd_handle = -1;
        export_fence.ExportHandle(fd_handle, handle_type);
#ifndef VK_USE_PLATFORM_WIN32_KHR
        close(fd_handle);
#endif
    }

    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, ExternalFenceSubmitCmdBuffer) {
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    // Check for external fence export capability
    VkPhysicalDeviceExternalFenceInfo efi = vku::InitStructHelper();
    efi.handleType = handle_type;
    VkExternalFencePropertiesKHR efp = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalFencePropertiesKHR(Gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT)) {
        GTEST_SKIP() << "External fence does not support exporting, skipping test.";
        return;
    }

    // Create a fence to export payload from
    VkExportFenceCreateInfo efci = vku::InitStructHelper();
    efci.handleTypes = handle_type;
    VkFenceCreateInfo fci = vku::InitStructHelper(&efci);
    vkt::Fence export_fence(*m_device, fci);

    for (uint32_t i = 0; i < 1000; i++) {
        m_command_buffer.Begin();
        m_command_buffer.End();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_buffer.handle();
        vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, export_fence.handle());

        int fd_handle = -1;
        export_fence.ExportHandle(fd_handle, handle_type);

#ifndef VK_USE_PLATFORM_WIN32_KHR
        // Wait until command buffer is finished using the exported handle.
        if (fd_handle != -1) {
            struct pollfd fds;
            fds.fd = fd_handle;
            fds.events = POLLIN;
            int timeout_ms = static_cast<int>(kWaitTimeout / 1000000);
            while (true) {
                int ret = poll(&fds, 1, timeout_ms);
                if (ret > 0) {
                    ASSERT_FALSE(fds.revents & (POLLERR | POLLNVAL));
                    break;
                }
                ASSERT_FALSE(ret == 0);                                         // Timeout.
                ASSERT_TRUE(ret == -1 && (errno == EINTR || errno == EAGAIN));  // Retry...
            }
            close(fd_handle);
        }
#else
        // On Windows this test works with MockICD driver and it's fine not to close fd_handle,
        // because it's a dummy value. In case we get access to a real POSIX environment on
        // Windows and VK_KHR_external_fence_fd will be provided through regular graphics drivers,
        // then we need to do a proper POSIX clean-up sequence as shown above.
        m_default_queue->Wait();
#endif

        m_command_buffer.Reset();
    }

    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, BasicSetAndWaitEvent) {
    TEST_DESCRIPTION("Sets event and then wait for it using CmdSetEvent/CmdWaitEvents");
    RETURN_IF_SKIP(Init());

    const vkt::Event event(*m_device);

    // Record time validation
    m_command_buffer.Begin();
    vk::CmdSetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    vk::CmdWaitEvents(m_command_buffer, 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    m_command_buffer.End();

    // Also submit to the queue to test submit time validation
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveSyncObject, BasicSetAndWaitEvent2) {
    TEST_DESCRIPTION("Sets event and then wait for it using CmdSetEvent2/CmdWaitEvents2");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_NONE;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    const vkt::Event event(*m_device);

    // Record time validation
    m_command_buffer.Begin();
    vk::CmdSetEvent2(m_command_buffer, event, &dependency_info);
    vk::CmdWaitEvents2(m_command_buffer, 1, &event.handle(), &dependency_info);
    m_command_buffer.End();

    // Also submit to the queue to test submit time validation
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveSyncObject, WaitEvent2HostStage) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Event event(*m_device);
    VkEvent event_handle = event.handle();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;  // Ok to use if outside the renderpass
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    vk::CmdWaitEvents2KHR(m_command_buffer.handle(), 1, &event_handle, &dependency_info);
    m_command_buffer.End();
}

TEST_F(PositiveSyncObject, DoubleLayoutTransition) {
    TEST_DESCRIPTION("Attempt vkCmdPipelineBarrier with 2 layout transitions of the same image.");

    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {32, 1, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageSubresource image_sub = vkt::Image::Subresource(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
    VkImageSubresourceRange image_sub_range = vkt::Image::SubresourceRange(image_sub);
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();

    {
        VkImageMemoryBarrier image_barriers[] = {image.ImageMemoryBarrier(
            0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image_sub_range)};

        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 1, image_barriers);
    }

    // TODO: is it allowed to transition the same image twice within a single barrier command?
    // Is it undefined behavior? Write a comment and provide references to the spec if that's allowed.
    {
        VkImageMemoryBarrier image_barriers[] = {
            image.ImageMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, image_sub_range),
            image.ImageMemoryBarrier(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, image_sub_range)};

        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 2, image_barriers);
    }

    m_command_buffer.End();
}

TEST_F(PositiveSyncObject, QueueSubmitTimelineSemaphore2Queue) {
    TEST_DESCRIPTION("Signal a timeline semaphore on 2 queues.");
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Test requires 2 queues";
    }

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_c(*m_device, 256, transfer_usage, mem_prop);
    VkBufferCopy region = {0, 0, 256};

    vkt::CommandPool pool0(*m_device, m_default_queue->family_index);
    vkt::CommandBuffer cb0(*m_device, pool0);
    cb0.Begin();
    vk::CmdCopyBuffer(cb0.handle(), buffer_a.handle(), buffer_b.handle(), 1, &region);
    cb0.End();

    vkt::CommandPool pool1(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb1(*m_device, pool1);
    cb1.Begin();
    vk::CmdCopyBuffer(cb1.handle(), buffer_c.handle(), buffer_b.handle(), 1, &region);
    cb1.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    // timeline values, Begins will be signaled by host, Ends by the queues
    constexpr uint64_t kQ0Begin = 1;
    constexpr uint64_t kQ0End = 2;
    constexpr uint64_t kQ1Begin = 3;
    constexpr uint64_t kQ1End = 4;

    m_default_queue->Submit(cb0, vkt::TimelineWait(semaphore, kQ0Begin), vkt::TimelineSignal(semaphore, kQ0End));
    m_second_queue->Submit(cb1, vkt::TimelineWait(semaphore, kQ1Begin), vkt::TimelineSignal(semaphore, kQ1End));

    semaphore.SignalKHR(kQ0Begin);  // initiate forward progress on q0
    semaphore.WaitKHR(kQ0End, kWaitTimeout);
    buffer_a.destroy();  // buffer_a is only used by the q0 commands

    semaphore.SignalKHR(kQ1Begin);  // initiate forward progress on q1
    semaphore.WaitKHR(kQ1End, kWaitTimeout);
    buffer_b.destroy();  // buffer_b is used by both q0 and q1
    buffer_c.destroy();  // buffer_c is used by q1

    m_device->Wait();
}

TEST_F(PositiveSyncObject, ResetQueryPoolFromDifferentCBWithFenceAfter) {
    TEST_DESCRIPTION("Reset query pool from a different command buffer and wait on fence after both are submitted");

    RETURN_IF_SKIP(Init());

    if (m_device->Physical().queue_properties_[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    VkFenceCreateInfo fence_info = vku::InitStructHelper();
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkt::Fence ts_fence(*m_device, fence_info);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    cb0.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vk::CmdResetQueryPool(cb0.handle(), query_pool.handle(), 0, 1);
    cb0.End();

    cb1.Begin();
    vk::CmdWriteTimestamp(cb1.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool.handle(), 0);
    cb1.End();

    // Begin by resetting the query pool.
    m_default_queue->Submit(cb0);

    // Write a timestamp, and add a fence to be signalled.
    vk::ResetFences(device(), 1, &ts_fence.handle());
    m_default_queue->Submit(cb1, ts_fence);

    // Reset query pool again.
    m_default_queue->Submit(cb0);

    // Finally, write a second timestamp, but before that, wait for the fence.
    vk::WaitForFences(device(), 1, &ts_fence.handle(), true, kWaitTimeout);
    m_default_queue->Submit(cb1);

    m_default_queue->Wait();
}

struct FenceSemRaceData {
    VkDevice device{VK_NULL_HANDLE};
    VkSemaphore sem{VK_NULL_HANDLE};
    std::atomic<bool> *bailout{nullptr};
    uint64_t wait_value{0};
    uint64_t timeout{kWaitTimeout};
    uint32_t iterations{100000};
};

void WaitTimelineSem(FenceSemRaceData *data) {
    uint64_t wait_value = data->wait_value;
    VkSemaphoreWaitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphoreCount = 1;
    wait_info.pSemaphores = &data->sem;
    wait_info.pValues = &wait_value;

    for (uint32_t i = 0; i < data->iterations; i++, wait_value++) {
        vk::WaitSemaphoresKHR(data->device, &wait_info, data->timeout);
        if (*data->bailout) {
            break;
        }
    }
}

TEST_F(PositiveSyncObject, FenceSemThreadRace) {
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD (WaitSemaphores)";
    }

    vkt::Fence fence(*m_device);
    vkt::Semaphore sem(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    uint64_t signal_value = 1;

    std::atomic<bool> bailout{false};
    FenceSemRaceData data;
    data.device = device();
    data.sem = sem.handle();
    data.wait_value = signal_value;
    data.bailout = &bailout;
    std::thread thread(WaitTimelineSem, &data);

    m_errorMonitor->SetBailout(&bailout);

    for (uint32_t i = 0; i < data.iterations; i++, signal_value++) {
        m_default_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(sem, signal_value), fence);
        fence.Wait(data.timeout);
        vk::ResetFences(device(), 1, &fence.handle());
    }
    m_errorMonitor->SetBailout(nullptr);
    thread.join();
}

TEST_F(PositiveSyncObject, SubmitFenceButWaitIdle) {
    TEST_DESCRIPTION("Submit a CB and Fence but synchronize with vkQueueWaitIdle() (Issue 2756)");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    vkt::Fence fence(*m_device);
    vkt::Semaphore sem(*m_device);

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    std::optional<vkt::CommandPool> command_pool(vvl::in_place, *m_device, pool_create_info);

    // create a raw command buffer because we'll just the destroy the pool.
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.commandPool = command_pool->handle();
    alloc_info.commandBufferCount = 1;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    auto err = vk::AllocateCommandBuffers(m_device->handle(), &alloc_info, &command_buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    m_swapchain.AcquireNextImage(sem, kWaitTimeout, &err);
    ASSERT_EQ(VK_SUCCESS, err);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    err = vk::BeginCommandBuffer(command_buffer, &begin_info);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                           nullptr, 0, nullptr);

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    viewport.width = 512;
    viewport.height = 512;
    viewport.x = 0;
    viewport.y = 0;
    vk::CmdSetViewport(command_buffer, 0, 1, &viewport);
    err = vk::EndCommandBuffer(command_buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, fence.handle());

    m_default_queue->Wait();

    command_pool.reset();
}

struct SemBufferRaceData {
    SemBufferRaceData(vkt::Device &dev_) : dev(dev_), sem(dev_, VK_SEMAPHORE_TYPE_TIMELINE) {}

    vkt::Device &dev;
    vkt::Semaphore sem;
    uint64_t start_wait_value{0};
    uint64_t timeout_ns{kWaitTimeout};
    uint32_t iterations{10000};
    std::atomic<bool> bailout{false};

    std::unique_ptr<vkt::Buffer> thread_buffer;

    void ThreadFunc() {
        auto wait_value = start_wait_value;

        while (!bailout) {
            auto err = sem.WaitKHR(wait_value, timeout_ns);
            if (err != VK_SUCCESS) {
                break;
            }
            auto buffer = std::move(thread_buffer);
            if (!buffer) {
                break;
            }
            buffer.reset();

            err = sem.SignalKHR(wait_value + 1);
            ASSERT_EQ(VK_SUCCESS, err);
            wait_value += 3;
        }
    }

    void Run(vkt::CommandPool &command_pool, ErrorMonitor &error_mon) {
        uint64_t gpu_wait_value, gpu_signal_value;
        VkResult err;
        start_wait_value = 2;
        error_mon.SetBailout(&bailout);
        std::thread thread(&SemBufferRaceData::ThreadFunc, this);
        auto queue = dev.QueuesWithGraphicsCapability()[0];
        for (uint32_t i = 0; i < iterations; i++) {
            vkt::CommandBuffer cb(dev, command_pool);

            VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            auto buffer = std::make_unique<vkt::Buffer>();
            buffer->init(dev, 20, VK_BUFFER_USAGE_TRANSFER_DST_BIT, reqs);

            // main thread sets up buffer
            // main thread signals 1
            // gpu queue waits for 1
            // gpu queue signals 2
            // sub thread waits for 2
            // sub thread frees buffer
            // sub thread signals 3
            // main thread waits for 3
            uint64_t host_signal_value = (i * 3) + 1;
            gpu_wait_value = host_signal_value;
            gpu_signal_value = (i * 3) + 2;
            uint64_t host_wait_value = (i * 3) + 3;

            cb.Begin();
            vk::CmdFillBuffer(cb.handle(), buffer->handle(), 0, 12, 0x11111111);
            cb.End();
            thread_buffer = std::move(buffer);

            err = queue->Submit(cb, vkt::TimelineWait(sem, gpu_wait_value), vkt::TimelineSignal(sem, gpu_signal_value));
            ASSERT_EQ(VK_SUCCESS, err);

            err = sem.SignalKHR(host_signal_value);
            ASSERT_EQ(VK_SUCCESS, err);

            err = sem.WaitKHR(host_wait_value, timeout_ns);
            ASSERT_EQ(VK_SUCCESS, err);
        }
        bailout = true;
        // make sure worker thread wakes up.
        err = sem.SignalKHR((iterations + 1) * 3);
        ASSERT_EQ(VK_SUCCESS, err);
        thread.join();
        error_mon.SetBailout(nullptr);
        queue->Wait();
    }
};

TEST_F(PositiveSyncObject, WaitTimelineSemThreadRace) {
#if defined(VVL_ENABLE_TSAN)
    // NOTE: This test in particular has failed sporadically on CI when TSAN is enabled.
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD (semaphore host signal/wait)";
    }
    SemBufferRaceData data(*m_device);

    data.Run(m_command_pool, *m_errorMonitor);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(PositiveSyncObject, WaitTimelineSemaphoreWithWin32HandleRetrieved) {
    TEST_DESCRIPTION("Use vkWaitSemaphores with exported semaphore to wait for the queue");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    constexpr auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 0;

    if (!SemaphoreExportImportSupported(Gpu(), handle_type, &semaphore_type_create_info)) {
        GTEST_SKIP() << "Semaphore does not support export and import through Win32 handle";
    }

    // Create exportable timeline semaphore
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper(&semaphore_type_create_info);
    export_info.handleTypes = handle_type;

    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    vkt::Semaphore semaphore(*m_device, create_info);

    // This caused original issue: exported semaphore failed to retire queue operations.
    HANDLE win32_handle = NULL;
    ASSERT_EQ(VK_SUCCESS, semaphore.ExportHandle(win32_handle, handle_type));

    // Put semaphore to work
    const uint64_t signal_value = 1;
    VkTimelineSemaphoreSubmitInfo timeline_submit_info = vku::InitStructHelper();
    timeline_submit_info.signalSemaphoreValueCount = 1;
    timeline_submit_info.pSignalSemaphoreValues = &signal_value;

    VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_submit_info);
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE));

    // This wait (with exported semaphore) should properly retire all queue operations
    VkSemaphoreWaitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphoreCount = 1;
    wait_info.pSemaphores = &semaphore.handle();
    wait_info.pValues = &signal_value;
    ASSERT_EQ(VK_SUCCESS, vk::WaitSemaphores(*m_device, &wait_info, uint64_t(1e10)));
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

TEST_F(PositiveSyncObject, SubpassBarrier) {
    TEST_DESCRIPTION("The queue family indices for subpass barrier should be equal (but otherwise are not restricted");
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddInputAttachment(0);
    rp.AddSubpassDependency();
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &image_view.handle());

    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = 0;
    barrier.dstQueueFamilyIndex = 0;
    barrier.image = image.handle();
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32, 32);
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &barrier);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncObject, SubpassBarrier2) {
    TEST_DESCRIPTION("The queue family indices for subpass barrier should be equal (but otherwise are not restricted");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddInputAttachment(0);
    rp.AddSubpassDependency();
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &image_view.handle());

    VkImageMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = 0;
    barrier.dstQueueFamilyIndex = 0;
    barrier.image = image.handle();
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32, 32);
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6204
TEST_F(PositiveSyncObject, SubpassBarrierWithExpandableStages) {
    TEST_DESCRIPTION("Specify expandable stages in subpass barrier");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = 0;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    subpass_dependency.srcAccessMask = VK_ACCESS_INDEX_READ_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_INDEX_READ_BIT;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &subpass_dependency;
    const vkt::RenderPass rp(*m_device, rpci);
    const vkt::Framebuffer fb(*m_device, rp, 0, nullptr, m_width, m_height);

    m_renderPassBeginInfo.renderPass = rp;
    m_renderPassBeginInfo.framebuffer = fb;

    VkMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_INDEX_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_INDEX_READ_BIT;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    // The issue was that implementation expands *subpass* compound stages but did not expand *barrier* compound stages.
    // Specify expandable stage (VERTEX_INPUT_BIT is INDEX_INPUT_BIT + VERTEX_ATTRIBUTE_INPUT_BIT) to ensure it's correctly
    // matched against subpass stages.
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1,
                           &barrier, 0, nullptr, 0, nullptr);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSyncObject, BarrierWithHostStage) {
    TEST_DESCRIPTION("Barrier includes VK_PIPELINE_STAGE_2_HOST_BIT as srcStageMask or dstStageMask");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    // HOST stage as source
    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkBufferMemoryBarrier2 buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.srcQueueFamilyIndex = 0;
    buffer_barrier.dstQueueFamilyIndex = 0;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = VK_WHOLE_SIZE;

    VkDependencyInfo buffer_dependency = vku::InitStructHelper();
    buffer_dependency.bufferMemoryBarrierCount = 1;
    buffer_dependency.pBufferMemoryBarriers = &buffer_barrier;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &buffer_dependency);
    m_command_buffer.End();

    // HOST stage as destination
    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.srcQueueFamilyIndex = 0;
    image_barrier.dstQueueFamilyIndex = 0;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo image_dependency = vku::InitStructHelper();
    image_dependency.imageMemoryBarrierCount = 1;
    image_dependency.pImageMemoryBarriers = &image_barrier;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &image_dependency);
    m_command_buffer.End();
}

TEST_F(PositiveSyncObject, BarrierASBuildWithShaderReadAccess) {
    TEST_DESCRIPTION("Test barrier with acceleration structure build stage and shader read access to access geometry input data.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier2KHR(m_command_buffer, &dependency_info);
    m_command_buffer.End();
}

TEST_F(PositiveSyncObject, BarrierAccessSyncMicroMap) {
    TEST_DESCRIPTION("Test VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT can be used with VK_ACCESS_2_SHADER_READ_BIT.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_command_buffer.End();
}

TEST_F(PositiveSyncObject, DynamicRenderingLocalReadImageBarrier) {
    TEST_DESCRIPTION("Test using an image memory barrier with the dynamic rendering local read extension");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::dynamicRenderingLocalRead);
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ);

    VkFormat colorAttachment = VK_FORMAT_R16_UNORM;

    VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo = vku::InitStructHelper();
    inheritanceRenderingInfo.colorAttachmentCount = 1u;
    inheritanceRenderingInfo.pColorAttachmentFormats = &colorAttachment;
    inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo inheritanceInfo = vku::InitStructHelper(&inheritanceRenderingInfo);

    VkCommandBufferBeginInfo beginInfo = vku::InitStructHelper();
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    VkImageMemoryBarrier imageMemoryBarrier = vku::InitStructHelper();
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ;
    imageMemoryBarrier.image = image.handle();
    imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};

    secondary.Begin(&beginInfo);
    vk::CmdPipelineBarrier(secondary.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0u, nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);
    secondary.End();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6172
TEST_F(PositiveSyncObject, TwoQueuesReuseBinarySemaphore) {
    TEST_DESCRIPTION("Use binary semaphore with the first queue then re-use on a different queue");
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Test requires two queues";
    }

    VkQueue q0 = m_default_queue->handle();
    VkQueue q1 = m_second_queue->handle();

    constexpr VkPipelineStageFlags wait_dst_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    vkt::Semaphore semaphore(*m_device);

    VkSubmitInfo submits[2];

    submits[0] = vku::InitStructHelper();
    submits[0].signalSemaphoreCount = 1;
    submits[0].pSignalSemaphores = &semaphore.handle();

    submits[1] = vku::InitStructHelper();
    submits[1].waitSemaphoreCount = 1;
    submits[1].pWaitSemaphores = &semaphore.handle();
    submits[1].pWaitDstStageMask = &wait_dst_stages;

    vk::QueueSubmit(q0, 2, submits, VK_NULL_HANDLE);
    vk::QueueWaitIdle(q0);

    vk::QueueSubmit(q1, 2, submits, VK_NULL_HANDLE);
    vk::QueueWaitIdle(q1);
}

TEST_F(PositiveSyncObject, SingleSubmitSignalBinarySemaphoreTwoTimes) {
    TEST_DESCRIPTION("Setup submission in such a way to be able to signal the same binary semaphore twice");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device);
    vkt::Semaphore semaphore2(*m_device);

    VkSemaphoreSubmitInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.semaphore = semaphore;
    semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSemaphoreSubmitInfo semaphore_info2 = vku::InitStructHelper();
    semaphore_info2.semaphore = semaphore2;
    semaphore_info2.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSubmitInfo2 submits[3];
    submits[0] = vku::InitStructHelper();
    submits[0].signalSemaphoreInfoCount = 1;
    submits[0].pSignalSemaphoreInfos = &semaphore_info;

    submits[1] = vku::InitStructHelper();
    submits[1].waitSemaphoreInfoCount = 1;
    submits[1].pWaitSemaphoreInfos = &semaphore_info;
    submits[1].signalSemaphoreInfoCount = 1;
    submits[1].pSignalSemaphoreInfos = &semaphore_info2;

    submits[2] = vku::InitStructHelper();
    submits[2].waitSemaphoreInfoCount = 1;
    submits[2].pWaitSemaphoreInfos = &semaphore_info2;
    // Here we can signal the first semaphore again. This should work because of the wait on the semaphore2.
    // Regarding internal implementation, this demonstrates that the binary semaphore's timeline map
    // can have more than one entry. The entry with payload 1 is for the first signal/wait,
    // and the entry with payload 2 will contain the following signal.
    submits[2].signalSemaphoreInfoCount = 1;
    submits[2].pSignalSemaphoreInfos = &semaphore_info;

    vk::QueueSubmit2(*m_default_queue, 3, submits, VK_NULL_HANDLE);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, SubmitImportedBinarySemaphoreWithNonZeroValue) {
    TEST_DESCRIPTION("QueueSubmit2 can specify arbitrary payload value for binary semaphore and it should be ignored");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    AddRequiredExtensions(extension_name);
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    // Check semaphore's import/export capability
    VkPhysicalDeviceExternalSemaphoreInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.handleType = handle_type;
    VkExternalSemaphorePropertiesKHR semaphore_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceExternalSemaphorePropertiesKHR(Gpu(), &semaphore_info, &semaphore_properties);
    if (!(semaphore_properties.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) ||
        !(semaphore_properties.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "Semaphore does not support import and/or export";
    }

    // Signaling semaphore
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper();
    export_info.handleTypes = handle_type;
    VkSemaphoreCreateInfo semaphore_ci = vku::InitStructHelper(&export_info);
    vkt::Semaphore semaphore(*m_device, semaphore_ci);
    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = semaphore;
    signal_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    ExternalHandle ext_handle{};
    semaphore.ExportHandle(ext_handle, handle_type);

    // Wait semaphore is imported from the signaling one.
    vkt::Semaphore semaphore2(*m_device);
    semaphore2.ImportHandle(ext_handle, handle_type);
    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphore = semaphore2;
    // Specify some payload value, even if it's a binary semaphore. It should be ignored.
    wait_info.value = 1;
    wait_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSubmitInfo2 submits[2];
    submits[0] = vku::InitStructHelper();
    submits[0].signalSemaphoreInfoCount = 1;
    submits[0].pSignalSemaphoreInfos = &signal_info;

    submits[1] = vku::InitStructHelper();
    submits[1].waitSemaphoreInfoCount = 1;
    submits[1].pWaitSemaphoreInfos = &wait_info;

    vk::QueueSubmit2(*m_default_queue, 2, submits, VK_NULL_HANDLE);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, IgnoreAcquireOpSrcStage) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7928
    TEST_DESCRIPTION("Test that graphics src stage is ignored during acquire operation on the transfer queue");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> transfer_only_family = m_device->TransferOnlyQueueFamily();
    if (!transfer_only_family.has_value()) {
        GTEST_SKIP() << "Transfer-only queue family is required";
    }
    vkt::CommandPool transfer_pool(*m_device, transfer_only_family.value());
    vkt::CommandBuffer transfer_cb(*m_device, transfer_pool);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferMemoryBarrier2 acquire_barrier = vku::InitStructHelper();
    acquire_barrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    acquire_barrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    acquire_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    acquire_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    acquire_barrier.srcQueueFamilyIndex = m_default_queue->family_index;
    acquire_barrier.dstQueueFamilyIndex = transfer_only_family.value();
    acquire_barrier.buffer = buffer.handle();
    acquire_barrier.offset = 0;
    acquire_barrier.size = 256;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &acquire_barrier;

    transfer_cb.Begin();
    vk::CmdPipelineBarrier2(transfer_cb.handle(), &dep_info);
    transfer_cb.End();
}

TEST_F(PositiveSyncObject, IgnoreReleaseOpDstStage) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7928
    TEST_DESCRIPTION("Test that graphics dst stage is ignored during release operation on the transfer queue");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> transfer_only_family = m_device->TransferOnlyQueueFamily();
    if (!transfer_only_family.has_value()) {
        GTEST_SKIP() << "Transfer-only queue family is required";
    }
    vkt::CommandPool release_pool(*m_device, transfer_only_family.value());
    vkt::CommandBuffer release_cb(*m_device, release_pool);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferMemoryBarrier2 release_barrier = vku::InitStructHelper();
    release_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    release_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    release_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    release_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    release_barrier.srcQueueFamilyIndex = transfer_only_family.value();
    release_barrier.dstQueueFamilyIndex = m_default_queue->family_index;
    release_barrier.buffer = buffer.handle();
    release_barrier.offset = 0;
    release_barrier.size = 256;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &release_barrier;

    release_cb.Begin();
    vk::CmdPipelineBarrier2(release_cb.handle(), &dep_info);
    release_cb.End();
}

TEST_F(PositiveSyncObject, ImageOwnershipTransferNormalizeSubresourceRange) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8823
    TEST_DESCRIPTION("Use different representations of image subresource range for release and acquire ownership operations");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Queue *transfer_queue = m_device->TransferOnlyQueue();
    if (!transfer_queue) {
        GTEST_SKIP() << "Transfer-only queue is not present";
    }
    vkt::CommandPool release_pool(*m_device, transfer_queue->family_index);
    vkt::CommandBuffer release_cb(*m_device, release_pool);
    vkt::CommandBuffer acquire_cb(*m_device, m_command_pool);

    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageCreateInfo image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, usage);
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Release image
    VkImageMemoryBarrier2 release_barrier = vku::InitStructHelper();
    release_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    release_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    release_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    release_barrier.dstAccessMask = VK_ACCESS_2_NONE;
    release_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    release_barrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    release_barrier.srcQueueFamilyIndex = transfer_queue->family_index;
    release_barrier.dstQueueFamilyIndex = m_default_queue->family_index;
    release_barrier.image = image;
    // Specify exact mip/layer count
    release_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo release_dep_info = vku::InitStructHelper();
    release_dep_info.imageMemoryBarrierCount = 1;
    release_dep_info.pImageMemoryBarriers = &release_barrier;
    release_cb.Begin();
    vk::CmdPipelineBarrier2(release_cb, &release_dep_info);
    release_cb.End();

    // Acquire image
    VkImageMemoryBarrier2 acquire_barrier = vku::InitStructHelper();
    acquire_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    acquire_barrier.srcAccessMask = VK_ACCESS_2_NONE;
    acquire_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    acquire_barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    acquire_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    acquire_barrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    acquire_barrier.srcQueueFamilyIndex = transfer_queue->family_index;
    acquire_barrier.dstQueueFamilyIndex = m_default_queue->family_index;
    acquire_barrier.image = image;
    // Use VK_REMAINING shortcut to specify mip/layer count.
    // Test for regression when VK_REMAINING is not compared correctly against specific mip/layer values for ownership transfer.
    acquire_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};

    VkDependencyInfo acquire_dep_info = vku::InitStructHelper();
    acquire_dep_info.imageMemoryBarrierCount = 1;
    acquire_dep_info.pImageMemoryBarriers = &acquire_barrier;
    acquire_cb.Begin();
    vk::CmdPipelineBarrier2(acquire_cb, &acquire_dep_info);
    acquire_cb.End();

    // Submit release on the transfer queue and acquire on the main queue.
    // There should be no errors about missing release operation for acquire operation.
    vkt::Semaphore semaphore(*m_device);
    transfer_queue->Submit2(release_cb, vkt::Signal(semaphore));
    m_default_queue->Submit2(acquire_cb, vkt::Wait(semaphore));
    m_device->Wait();
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(PositiveSyncObject, GetCounterValueOfExportedSemaphore) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8212
    TEST_DESCRIPTION("Getting counter value of exported semaphore should not introduce orphaned signals");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    constexpr auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if (!SemaphoreExportImportSupported(Gpu(), handle_type)) {
        GTEST_SKIP() << "Semaphore does not support export and import through Win32 handle";
    }

    // Create exportable timeline semaphore
    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 0;
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper(&semaphore_type_create_info);
    export_info.handleTypes = handle_type;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    vkt::Semaphore semaphore(*m_device, create_info);
    HANDLE win32_handle = NULL;
    semaphore.ExportHandle(win32_handle, handle_type);

    // The problem was that GetSemaphoreCounterValue creates temporary signal to make forward progress,
    // but the code path for external semaphore failed to retire that signal. Being stuck that signal
    // conflicted with other signals.
    uint64_t counter = 0;
    vk::GetSemaphoreCounterValue(device(), semaphore, &counter);

    // Test that there are no leftovers from GetSemaphoreCounterValue, this signal should just work.
    semaphore.Signal(1);
}

TEST_F(PositiveSyncObject, GetCounterValueOfExportedSemaphore2) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8212
    TEST_DESCRIPTION("Getting counter value of exported semaphore should not introduce orphaned signals");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    constexpr auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if (!SemaphoreExportImportSupported(Gpu(), handle_type)) {
        GTEST_SKIP() << "Semaphore does not support export and import through Win32 handle";
    }

    // Create exportable timeline semaphore
    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 0;
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper(&semaphore_type_create_info);
    export_info.handleTypes = handle_type;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&export_info);
    vkt::Semaphore semaphore(*m_device, create_info);
    HANDLE win32_handle = NULL;
    semaphore.ExportHandle(win32_handle, handle_type);

    // Slight variation of the previous test to ensure that issue was not related to semaphore initial value
    semaphore.Signal(1);
    uint64_t counter = 0;
    vk::GetSemaphoreCounterValue(device(), semaphore, &counter);
    semaphore.Signal(2);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

TEST_F(PositiveSyncObject, TimelineHostWaitThenSubmitSignal) {
    TEST_DESCRIPTION("Wait on the host then submit signal");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD (WaitSemaphores)";
    }

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    auto thread = [&semaphore]() { semaphore.Wait(1, kWaitTimeout); };
    std::thread t(thread);

    // This delay increases the probability that the wait started before the signal.
    // If the waiting thread was not fast enough this becomes a common signal-then-wait setup.
    std::this_thread::sleep_for(std::chrono::milliseconds{50});

    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));
    t.join();
}

TEST_F(PositiveSyncObject, TimelineHostWaitThenSubmitLargerSignal) {
    TEST_DESCRIPTION("Wait on the host then submit signal with larger value");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD (WaitSemaphores)";
    }

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    auto thread = [&semaphore]() { semaphore.Wait(1, kWaitTimeout); };
    std::thread t(thread);

    // This delay increases the probability that the wait started before the signal.
    // If the waiting thread was not fast enough this becomes a common signal-then-wait setup.
    std::this_thread::sleep_for(std::chrono::milliseconds{50});

    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 2));
    t.join();
}

TEST_F(PositiveSyncObject, TimelineHostWaitThenHostSignal) {
    TEST_DESCRIPTION("Wait on the host then signal from the host");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD (WaitSemaphores)";
    }
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    auto thread = [&semaphore]() { semaphore.Wait(1, kWaitTimeout); };
    std::thread t(thread);

    // This delay increases the probability that the wait started before the signal.
    // If the waiting thread was not fast enough this becomes a common signal-then-wait setup.
    std::this_thread::sleep_for(std::chrono::milliseconds{50});

    semaphore.Signal(1);
    t.join();
}

TEST_F(PositiveSyncObject, TimelineHostSignalThenHostWait) {
    TEST_DESCRIPTION("Signal on the host then wait on the host");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    semaphore.Signal(1);
    semaphore.Wait(1, kWaitTimeout);
}

TEST_F(PositiveSyncObject, TimelineSubmitSignalThenHostWaitSmallerValue) {
    TEST_DESCRIPTION("Submit signal then wait smaller value on the host");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 2));
    semaphore.Wait(1, kWaitTimeout);
}

TEST_F(PositiveSyncObject, TimelineSubmitWaitThenSubmitSignalLargerValue) {
    TEST_DESCRIPTION("Submit wait then submit signal with larger value, wait on the host for wait completion");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "2 queues are needed";
    }
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    m_second_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 2));

    // This should also sync with the second queue because the second queue signals a default one.
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, TimelineSubmitSignalThenSubmitWaitSmallerValue) {
    TEST_DESCRIPTION("Submit signal then submit wait with smaller value, wait on the host for wait completion");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "2 queues are needed";
    }
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_second_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 2));
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));

    // This should also sync with the second queue because the second queue signals a default one.
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, TimelineSubmitWaitThenHostSignal) {
    TEST_DESCRIPTION("Submit wait then signal on the host, wait on the host for wait completion");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    semaphore.Signal(1);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, TimelineSubmitWaitThenHostSignalLargerValue) {
    TEST_DESCRIPTION("Submit wait then signal on the host larger value, wait on the host for wait completion");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    semaphore.Signal(2);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, PollSemaphoreCounter) {
    TEST_DESCRIPTION("Basic semaphore polling test");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD (GetSemaphoreCounterValue)";
    }

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));

    uint64_t counter = 0;
    do {
        vk::GetSemaphoreCounterValue(*m_device, semaphore, &counter);
    } while (counter != 1);
}

TEST_F(PositiveSyncObject, KhronosTimelineSemaphoreExample) {
    TEST_DESCRIPTION("https://www.khronos.org/blog/vulkan-timeline-semaphores");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD (host synchronization)";
    }
    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    int N = 1000;

    for (int i = 0; i < N; i++) {
        vkt::Semaphore timeline(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

        auto thread1 = [this, &timeline]() {
            // Can start immediately, wait on 0 is noop
            m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(timeline, 0), vkt::TimelineSignal(timeline, 5));
        };
        auto thread2 = [&timeline]() {
            // Wait for thread1
            timeline.Wait(4, kWaitTimeout);
            // Unblock thread3
            timeline.Signal(7);
        };
        auto thread3 = [this, &timeline]() {
            m_second_queue->Submit(vkt::no_cmd, vkt::TimelineWait(timeline, 7), vkt::TimelineSignal(timeline, 8));
        };

        std::thread t1(thread1);
        std::thread t2(thread2);
        std::thread t3(thread3);

        timeline.Wait(8, kWaitTimeout);

        t3.join();
        t2.join();
        t1.join();
    }
}

TEST_F(PositiveSyncObject, BarrierWithoutOwnershipTransferUseAllStages) {
    TEST_DESCRIPTION("Barrier without ownership transfer with USE_ALL_STAGES flag (no-op)");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    // The queue family are the same here, so flag is basically a no-op
    barrier.srcQueueFamilyIndex = 0;
    barrier.dstQueueFamilyIndex = 0;
    barrier.buffer = buffer;
    barrier.offset = 0;
    barrier.size = 256;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                           VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR, 0, nullptr, 1, &barrier, 0,
                           nullptr);
    m_command_buffer.End();
}

TEST_F(PositiveSyncObject, OwnershipTransferUseAllStages) {
    TEST_DESCRIPTION("Barrier with ownership transfer with USE_ALL_STAGES flag");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    // Enable feature to use stage other than ALL_COMMANDS during ownership transfer
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> transfer_only_family = m_device->TransferOnlyQueueFamily();
    if (!transfer_only_family.has_value()) {
        GTEST_SKIP() << "Transfer-only queue family is required";
    }
    vkt::CommandPool transfer_pool(*m_device, transfer_only_family.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer transfer_cb(*m_device, transfer_pool);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Acquire operation on transfer queue.
    // The src stage should be a valid transfer stage.
    VkBufferMemoryBarrier2 acquire_barrier = vku::InitStructHelper();
    acquire_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    acquire_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    acquire_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    acquire_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    acquire_barrier.srcQueueFamilyIndex = m_default_queue->family_index;
    acquire_barrier.dstQueueFamilyIndex = transfer_only_family.value();
    acquire_barrier.buffer = buffer;
    acquire_barrier.offset = 0;
    acquire_barrier.size = 256;

    VkDependencyInfo acquire_dep_info = vku::InitStructHelper();
    // Use this dependency flag to be able to use src stage other then ALL_COMMANDS
    acquire_dep_info.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;
    acquire_dep_info.bufferMemoryBarrierCount = 1;
    acquire_dep_info.pBufferMemoryBarriers = &acquire_barrier;

    transfer_cb.Begin();
    vk::CmdPipelineBarrier2(transfer_cb, &acquire_dep_info);
    transfer_cb.End();

    // Release operation on transfer queue.
    // The dst stage should be a valid transfer stage.
    VkBufferMemoryBarrier2 release_barrier = vku::InitStructHelper();
    release_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    release_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    release_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    release_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    release_barrier.srcQueueFamilyIndex = transfer_only_family.value();
    release_barrier.dstQueueFamilyIndex = m_default_queue->family_index;
    release_barrier.buffer = buffer;
    release_barrier.offset = 0;
    release_barrier.size = 256;

    VkDependencyInfo release_dep_info = vku::InitStructHelper();
    // Use this dependency flag to be able to use dst stage other then ALL_COMMANDS
    release_dep_info.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;
    release_dep_info.bufferMemoryBarrierCount = 1;
    release_dep_info.pBufferMemoryBarriers = &release_barrier;

    transfer_cb.Begin();
    vk::CmdPipelineBarrier2(transfer_cb, &release_dep_info);
    transfer_cb.End();
}

TEST_F(PositiveSyncObject, BinarySyncAfterResolvedTimelineWait) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8900
    TEST_DESCRIPTION("Test binary wait followed by the previous resolved timeline wait");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore timeline_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore binary_semaphore(*m_device);

    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(timeline_semaphore, 1));

    // This removes signal's timepoint from timeline (not pending anymore)
    timeline_semaphore.Wait(1, kWaitTimeout);

    // Wait one more time (should just check completed state).
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(timeline_semaphore, 1));

    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(binary_semaphore));

    // Waiting on binary semaphore initiates check of unresolved timeline wait dependency.
    // This check did not work properly when resolving timeline signal was already retired.
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(binary_semaphore));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, QueueWaitAfterBinarySignal) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8989
    TEST_DESCRIPTION("Wait for binary signal after queue wait");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore timeline_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE, 1);
    vkt::Semaphore binary_semaphore(*m_device);

    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(binary_semaphore));
    // this removes timepoint with signal op from timeline, but it should not be a problem to wait for this signal
    m_default_queue->Wait();

    // Values that corresponds to binary semaphore should not affect internal logic.
    // Use non-zero value for binary semaphore to increase probability of the issues in case of regression.
    const uint64_t values[2] = {
        1,  // timeline value
        42  // arbitrary value that should be ignored (binary semaphore slot)
    };
    VkTimelineSemaphoreSubmitInfo timeline_info = vku::InitStructHelper();
    timeline_info.waitSemaphoreValueCount = 2;
    timeline_info.pWaitSemaphoreValues = values;

    const VkPipelineStageFlags wait_dst_stages[2] = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
    const VkSemaphore wait_semaphores[2] = {timeline_semaphore, binary_semaphore};

    VkSubmitInfo submit = vku::InitStructHelper(&timeline_info);
    submit.waitSemaphoreCount = 2;
    submit.pWaitSemaphores = wait_semaphores;
    submit.pWaitDstStageMask = wait_dst_stages;

    // Wait on the binary signal that was followed by Wait().
    // This also waits on the timeline initial value, so effectiely no-wait.
    // The only reason timeline semaphore is used in this test, is to have a
    // slot in VkTimelineSemaphoreSubmitInfo that corresponds to binary semaphore.
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit, VK_NULL_HANDLE);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, QueueWaitAfterBinarySignal2) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8989
    TEST_DESCRIPTION("Binary signal followed by queue wait");
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_default_queue->Wait();  // this removes timepoint with signal op from timeline

    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(semaphore));
    // Test for regression that triggers assert in Semaphore::CanBinaryBeSignaled
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncObject, QueueWaitAfterBinarySignal3) {
    TEST_DESCRIPTION("Binary signal followed by queue wait");
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_default_queue->Wait();  // this removes timepoint with signal op from timeline
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(semaphore), vkt::Signal(semaphore));
    m_default_queue->Wait();
}
