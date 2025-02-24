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
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/queue_submit_context.h"
#include <utils/vk_layer_utils.h>

class NegativeSyncVal : public VkSyncValTest {};

TEST_F(NegativeSyncVal, BufferCopy) {
    TEST_DESCRIPTION("Hazards caused by buffer copy commands");
    RETURN_IF_SKIP(InitSyncVal());

    const VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, usage);
    vkt::Buffer buffer_b(*m_device, 256, usage);
    vkt::Buffer buffer_c(*m_device, 256, usage);

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_command_buffer.Copy(buffer_c, buffer_a);
    m_errorMonitor->VerifyFound();

    // Sync with buffer_a from the first copy (a->b). The second copy caused error and has no effect
    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier.buffer = buffer_a;
    buffer_barrier.size = 256;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                           &buffer_barrier, 0, nullptr);

    // Write to buffer_a is protected by the above barrier
    m_command_buffer.Copy(buffer_c, buffer_a);

    // Cause WAW by writing buffer_a again
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_command_buffer.Copy(buffer_c, buffer_a);
    m_errorMonitor->VerifyFound();

    // buffer_b was not protected by the above barrier
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_command_buffer.Copy(buffer_c, buffer_b);
    m_errorMonitor->VerifyFound();

    // Use global barrier to protect buffer_b write
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0,
                           nullptr, 0, nullptr);
    m_command_buffer.Copy(buffer_c, buffer_b);

    // Protect buffer_c READ but not buffer_b WRITE
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0,
                           nullptr, 0, nullptr);
    m_command_buffer.Copy(buffer_b, buffer_c);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, BufferCopyWrongBarrier) {
    TEST_DESCRIPTION("Buffer barrier does not specify proper dst stage/access");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.buffer = buffer_b;
    barrier.size = 256;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, BufferCopyWrongBarrier2) {
    TEST_DESCRIPTION("Buffer barrier does not specify proper dst stage/access");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_c(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // It's enough to defined only execution dependency for WAR hazard.
    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;  // wrong stage to cause hazard
    barrier.buffer = buffer_a;
    barrier.size = 256;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_command_buffer.Copy(buffer_c, buffer_a);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, BufferCopySecondary) {
    TEST_DESCRIPTION("Record buffer copy commands in secondary command buffers");
    RETURN_IF_SKIP(InitSyncVal());

    const VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, usage);
    vkt::Buffer buffer_b(*m_device, 256, usage);
    vkt::Buffer buffer_c(*m_device, 256, usage);

    vkt::CommandBuffer secondary_cb1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb1.Begin();
    secondary_cb1.Copy(buffer_c, buffer_a);
    secondary_cb1.End();

    vkt::CommandBuffer secondary_cb2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb2.Begin();
    secondary_cb2.Copy(buffer_a, buffer_b);
    secondary_cb2.End();

    vkt::CommandBuffer secondary_cb3(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb3.Begin();
    secondary_cb3.Copy(buffer_b, buffer_c);
    secondary_cb3.End();

    // Secondary CB hazard with active command buffer
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_c, buffer_a);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary_cb1.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Two secondary CBs hazard with each other
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    VkCommandBuffer two_cbs[2] = {secondary_cb1, secondary_cb2};
    vk::CmdExecuteCommands(m_command_buffer, 2, two_cbs);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    VkCommandBuffer two_cbs2[2] = {secondary_cb1, secondary_cb3};
    vk::CmdExecuteCommands(m_command_buffer, 2, two_cbs2);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, BufferCopyHazardsSync2) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_c(*m_device, 256, transfer_usage, mem_prop);

    VkBufferCopy region = {0, 0, 256};
    VkBufferCopy front2front = {0, 0, 128};
    VkBufferCopy front2back = {0, 128, 128};
    VkBufferCopy back2back = {128, 128, 128};

    auto cb = m_command_buffer.handle();
    m_command_buffer.Begin();

    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    {
        VkBufferMemoryBarrier2 buffer_barrier = vku::InitStructHelper();
        buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        buffer_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        buffer_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        buffer_barrier.buffer = buffer_a.handle();
        buffer_barrier.offset = 0;
        buffer_barrier.size = 256;
        VkDependencyInfo dep_info = vku::InitStructHelper();
        dep_info.bufferMemoryBarrierCount = 1;
        dep_info.pBufferMemoryBarriers = &buffer_barrier;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);
    }

    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2front);
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &back2back);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2back);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_b.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    {
        VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
        mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        mem_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        mem_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        VkDependencyInfo dep_info = vku::InitStructHelper();
        dep_info.memoryBarrierCount = 1;
        dep_info.pMemoryBarriers = &mem_barrier;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);

        vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_c.handle(), buffer_b.handle(), 1, &region);

        m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
        mem_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;  // Protect C but not B
        mem_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);
        vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_b.handle(), buffer_c.handle(), 1, &region);
        m_errorMonitor->VerifyFound();

        m_command_buffer.End();
    }
}

struct ClearAttachmentHazardHelper {
    static constexpr VkImageUsageFlags kTransferUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    const uint32_t width = 256;
    const uint32_t height = 128;
    const VkFormat rt_format = VK_FORMAT_B8G8R8A8_UNORM;
    const VkImageUsageFlags transfer_usage = kTransferUsage;
    const VkImageUsageFlags rt_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | kTransferUsage;
    const VkImageUsageFlags ds_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | kTransferUsage;
    VkLayerTest& test;
    vkt::Device& device;
    vkt::Queue& queue;
    vkt::CommandBuffer& command_buffer;
    const VkFormat ds_format;
    vkt::Image image;
    vkt::Image image_ds;
    vkt::Image rt;
    vkt::Image ds;
    vkt::ImageView rt_view;
    vkt::ImageView ds_view;

    ClearAttachmentHazardHelper(VkLayerTest& test_, vkt::Device& device_, vkt::Queue& queue_, vkt::CommandBuffer& cb_)
        : test(test_),
          device(device_),
          queue(queue_),
          command_buffer(cb_),
          ds_format(FindSupportedDepthStencilFormat(test_.Gpu())),
          image(),
          image_ds(),
          rt(),
          ds() {
        auto image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, rt_format, transfer_usage);

        image.init(device, image_ci, 0);
        image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

        image_ci.format = ds_format;
        image_ds.init(device, image_ci, 0);
        image_ds.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

        image_ci.format = rt_format;
        image_ci.usage = rt_usage;
        rt.init(device, image_ci, 0);
        rt.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        rt_view = rt.CreateView();

        image_ci.format = ds_format;
        image_ci.usage = ds_usage;
        ds.init(device, image_ci, 0);
        ds.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        ds_view = ds.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    }

    template <typename BeginRenderFn, typename EndRenderFn>
    void Test(BeginRenderFn& begin_render, EndRenderFn& end_render);
};
template <typename BeginRenderFn, typename EndRenderFn>
void ClearAttachmentHazardHelper::Test(BeginRenderFn& begin_render, EndRenderFn& end_render) {
    struct AspectInfo {
        VkImageAspectFlagBits aspect;
        VkImage src_image;
        VkImage dst_image;
    };
    const AspectInfo aspect_infos[] = {{VK_IMAGE_ASPECT_COLOR_BIT, image, rt},
                                       {VK_IMAGE_ASPECT_DEPTH_BIT, image_ds, ds},
                                       {VK_IMAGE_ASPECT_STENCIL_BIT, image_ds, ds}};

    // WAW hazard: copy to render target then clear it. Test each aspect (color/depth/stencil).
    for (const auto& info : aspect_infos) {
        const VkClearAttachment clear_attachment = {VkImageAspectFlags(info.aspect)};

        VkClearRect clear_rect = {};
        clear_rect.rect.offset = {0, 0};
        clear_rect.rect.extent = {width / 2, height / 2};
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 1;

        VkImageCopy copy_region = {};
        copy_region.srcSubresource = {VkImageAspectFlags(info.aspect), 0, 0, 1};
        copy_region.dstSubresource = {VkImageAspectFlags(info.aspect), 0, 0, 1};
        copy_region.extent = {width, height, 1};

        command_buffer.Begin();
        // Write 1
        vk::CmdCopyImage(command_buffer, info.src_image, VK_IMAGE_LAYOUT_GENERAL, info.dst_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                         &copy_region);
        begin_render(command_buffer);

        test.SetDesiredFailureMsg(kErrorBit, "SYNC-HAZARD-WRITE-AFTER-WRITE");
        // Write 2
        vk::CmdClearAttachments(command_buffer, 1, &clear_attachment, 1, &clear_rect);
        test.VerifyFound();

        end_render(command_buffer);
        command_buffer.End();
        queue.Submit(command_buffer);
        queue.Wait();
        test.DefaultQueue()->Wait();
    }

    // RAW hazard: clear render target then copy from it.
    // This tests that vkCmdClearAttachments correctly updates access state, so vkCmdCopyImage can detect hazard.
    {
        const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_STENCIL_BIT};

        VkClearRect clear_rect = {};
        clear_rect.rect.offset = {0, 0};
        clear_rect.rect.extent = {width, height};
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 1;

        VkImageCopy copy_region = {};
        copy_region.srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
        copy_region.dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
        copy_region.extent = {width, height, 1};

        command_buffer.Begin();
        begin_render(command_buffer);
        // Write
        vk::CmdClearAttachments(command_buffer, 1, &clear_attachment, 1, &clear_rect);
        end_render(command_buffer);

        test.SetDesiredFailureMsg(kErrorBit, "SYNC-HAZARD-READ-AFTER-WRITE");
        // Read
        vk::CmdCopyImage(command_buffer, ds, VK_IMAGE_LAYOUT_GENERAL, image_ds, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
        test.VerifyFound();

        command_buffer.End();
        queue.Submit(command_buffer);
        queue.Wait();
        test.DefaultQueue()->Wait();
    }

    // RAW hazard: two regions with a single pixel overlap, otherwise the same as the previous scenario.
    {
        const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_COLOR_BIT};

        VkClearRect clear_rect = {};
        clear_rect.rect.offset = {0, 0};
        clear_rect.rect.extent = {32, 32};
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 1;

        VkImageCopy copy_region = {};
        copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copy_region.srcOffset = {31, 31, 0};
        copy_region.dstOffset = {31, 31, 0};
        copy_region.extent = {64, 64, 1};

        command_buffer.Begin();
        begin_render(command_buffer);
        // Write
        vk::CmdClearAttachments(command_buffer, 1, &clear_attachment, 1, &clear_rect);
        end_render(command_buffer);

        test.SetDesiredFailureMsg(kErrorBit, "SYNC-HAZARD-READ-AFTER-WRITE");
        // Read
        vk::CmdCopyImage(command_buffer, rt, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
        test.VerifyFound();

        command_buffer.End();
        queue.Submit(command_buffer);
        queue.Wait();
        test.DefaultQueue()->Wait();
    }

    // Nudge regions by one pixel compared to the previous test, now they touch but do not overlap. There should be no errors.
    // Copy to the first region, clear the second region.
    {
        const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_DEPTH_BIT};

        VkClearRect clear_rect = {};
        clear_rect.rect.offset = {0, 0};
        clear_rect.rect.extent = {32, 32};
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 1;

        VkImageCopy copy_region = {};
        copy_region.srcSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT), 0, 0, 1};
        copy_region.dstSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT), 0, 0, 1};
        copy_region.srcOffset = {32, 32, 0};
        copy_region.dstOffset = {32, 32, 0};
        copy_region.extent = {64, 64, 1};

        command_buffer.Begin();
        // Write 1
        vk::CmdCopyImage(command_buffer, image_ds, VK_IMAGE_LAYOUT_GENERAL, ds, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
        begin_render(command_buffer);
        // Write 2
        vk::CmdClearAttachments(command_buffer, 1, &clear_attachment, 1, &clear_rect);
        end_render(command_buffer);
        command_buffer.End();
        queue.Submit(command_buffer);
        queue.Wait();
        test.DefaultQueue()->Wait();
    }
}

TEST_F(NegativeSyncVal, CmdClearAttachmentsHazards) {
    TEST_DESCRIPTION("Test for hazards when attachment is cleared inside render pass.");

    // VK_EXT_load_store_op_none is needed to disable render pass load/store accesses, so clearing
    // attachment inside a render pass can create hazards with the copy operations outside render pass.
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    ClearAttachmentHazardHelper helper(*this, *m_device, *m_default_queue, m_command_buffer);
    auto attachment_without_load_store = [](VkFormat format) {
        VkAttachmentDescription attachment = {};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        return attachment;
    };
    const VkAttachmentDescription attachments[] = {attachment_without_load_store(helper.rt_format),
                                                   attachment_without_load_store(helper.ds_format)};

    const VkImageView views[] = {helper.rt_view, helper.ds_view};
    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    const VkAttachmentReference depth_ref = {1, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pDepthStencilAttachment = &depth_ref;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = size32(attachments);
    rpci.pAttachments = attachments;
    vkt::RenderPass render_pass(*m_device, rpci);
    vkt::Framebuffer framebuffer(*m_device, render_pass, size32(views), views, helper.width, helper.height);

    VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
    rpbi.framebuffer = framebuffer;
    rpbi.renderPass = render_pass;
    rpbi.renderArea.extent.width = helper.width;
    rpbi.renderArea.extent.height = helper.height;

    auto begin_rendering = [&rpbi](vkt::CommandBuffer& cb) { vk::CmdBeginRenderPass(cb, &rpbi, VK_SUBPASS_CONTENTS_INLINE); };
    auto end_rendering = [](vkt::CommandBuffer& cb) { vk::CmdEndRenderPass(cb); };
    helper.Test(begin_rendering, end_rendering);
}

TEST_F(NegativeSyncVal, CmdClearAttachmentsDynamicHazards) {
    TEST_DESCRIPTION("Test for hazards when attachment is cleared inside a dynamic render pass.");
    SetTargetApiVersion(VK_API_VERSION_1_3);

    // VK_EXT_load_store_op_none is needed to disable render pass load/store accesses, so clearing
    // attachment inside a render pass can create hazards with the copy operations outside render pass.
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    ClearAttachmentHazardHelper helper(*this, *m_device, *m_default_queue, m_command_buffer);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = helper.rt_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
    color_attachment.clearValue.color = m_clear_color;

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageView = helper.ds_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
    depth_attachment.clearValue.color = m_clear_color;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {helper.width, helper.height};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;
    rendering_info.pDepthAttachment = &depth_attachment;

    auto begin_rendering = [&rendering_info](vkt::CommandBuffer& cb) { vk::CmdBeginRendering(cb, &rendering_info); };
    auto end_rendering = [](vkt::CommandBuffer& cb) { vk::CmdEndRendering(cb); };
    helper.Test(begin_rendering, end_rendering);
}

TEST_F(NegativeSyncVal, CopyOptimalImageHazards) {
#if defined(VVL_ENABLE_TSAN)
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, format, usage);
    vkt::Image image_a(*m_device, image_ci);
    vkt::Image image_b(*m_device, image_ci);
    image_ci.flags |= VK_IMAGE_CREATE_ALIAS_BIT;
    vkt::Image image_c(*m_device, image_ci);

    vkt::Image image_c_alias(*m_device, image_ci, vkt::no_mem);
    image_c_alias.BindMemory(image_c.Memory(), 0);

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 2};
    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{64, 64, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};    // <-- image type is 2D

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};
    VkImageCopy region_0_to_0 = {layers_0, zero_offset, layers_0, zero_offset, full_extent};
    VkImageCopy region_0_to_1 = {layers_0, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_1_to_1 = {layers_1, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_0_front = {layers_0, zero_offset, layers_0, zero_offset, half_extent};
    VkImageCopy region_0_back = {layers_0, half_offset, layers_0, half_offset, half_extent};

    m_command_buffer.Begin();

    image_c.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_command_buffer.handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_a.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_to_0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_1_to_1);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_to_1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_back);

    // Safe all transfer accesses
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);

    // Write to both versions of an alias
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_alias.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_0_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_alias.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_0_back);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_back);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();

    // Test secondary command buffers
    // Create secondary buffers to use
    vkt::CommandBuffer secondary_cb1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer scb1 = secondary_cb1.handle();
    secondary_cb1.Begin();
    vk::CmdCopyImage(scb1, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    secondary_cb1.End();

    auto record_primary = [&]() {
        m_command_buffer.Reset();
        m_command_buffer.Begin();
        vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
        vk::CmdExecuteCommands(cb, 1, &scb1);
        m_command_buffer.End();
    };

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    record_primary();
    m_errorMonitor->VerifyFound();

    // With a barrier...
    secondary_cb1.Reset();
    secondary_cb1.Begin();
    vk::CmdPipelineBarrier(scb1, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(scb1, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    secondary_cb1.End();
    record_primary();

    auto image_transition_barrier = image_barrier;
    image_transition_barrier.image = image_a.handle();
    image_transition_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_transition_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    image_transition_barrier.srcAccessMask = 0;
    image_transition_barrier.dstAccessMask = 0;

    secondary_cb1.Reset();
    secondary_cb1.Begin();
    // Use the wrong stage, get an error
    vk::CmdPipelineBarrier(scb1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_transition_barrier);
    secondary_cb1.End();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    record_primary();
    m_errorMonitor->VerifyFound();

    // CmdResolveImage hazard testing
    VkImageFormatProperties formProps = {{0, 0, 0}, 0, 0, 0, 0};
    vk::GetPhysicalDeviceImageFormatProperties(m_device->Physical().handle(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &formProps);

    if (!(formProps.sampleCounts & VK_SAMPLE_COUNT_2_BIT)) {
        printf("CmdResolveImage Test requires unsupported VK_SAMPLE_COUNT_2_BIT feature. Skipped.\n");
    } else {
        image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
        vkt::Image image_s2_a(*m_device, image_ci);
        vkt::Image image_s2_b(*m_device, image_ci);

        VkImageResolve r_full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};

        m_command_buffer.Reset();
        m_command_buffer.Begin();
        image_s2_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
        image_s2_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
        vk::CmdResolveImage(cb, image_s2_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                            &r_full_region);
        m_command_buffer.End();

        m_command_buffer.Reset();
        m_command_buffer.Begin();
        vk::CmdCopyImage(cb, image_s2_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_s2_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                         &full_region);
        vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

        m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
        m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
        vk::CmdResolveImage(cb, image_s2_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                            &r_full_region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
        vk::CmdResolveImage(cb, image_s2_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                            &r_full_region);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeSyncVal, CopyOptimalImageHazardsSync2) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, format, usage);
    vkt::Image image_a(*m_device, image_ci);
    vkt::Image image_b(*m_device, image_ci);
    vkt::Image image_c(*m_device, image_ci);

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 2};
    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{64, 64, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};    // <-- image type is 2D

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};
    VkImageCopy region_0_to_0 = {layers_0, zero_offset, layers_0, zero_offset, full_extent};
    VkImageCopy region_0_to_1 = {layers_0, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_1_to_1 = {layers_1, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_0_front = {layers_0, zero_offset, layers_0, zero_offset, half_extent};
    VkImageCopy region_0_back = {layers_0, half_offset, layers_0, half_offset, half_extent};

    m_command_buffer.Begin();

    image_c.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_command_buffer.handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    {
        VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
        image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        image_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        image_barrier.image = image_a.handle();
        image_barrier.subresourceRange = full_subresource_range;
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkDependencyInfo dep_info = vku::InitStructHelper();
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &image_barrier;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);
    }

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_to_0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_1_to_1);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_to_1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    {
        VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
        mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        VkDependencyInfo dep_info = vku::InitStructHelper();
        dep_info.memoryBarrierCount = 1;
        dep_info.pMemoryBarriers = &mem_barrier;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);
        vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

        // Use barrier to protect last reader, but not last writer...
        m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
        mem_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;  // Protects C but not B
        mem_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        vk::CmdPipelineBarrier2KHR(cb, &dep_info);
        vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
        m_errorMonitor->VerifyFound();
    }

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_back);

    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, CopyOptimalMultiPlanarHazards) {
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());

    RETURN_IF_SKIP(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    const auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, format, usage);
    // Verify format
    bool supported = ImageFormatIsSupported(instance(), Gpu(), image_ci,
                                            VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
    if (!supported) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    vkt::Image image_a(*m_device, image_ci);
    vkt::Image image_b(*m_device, image_ci);
    vkt::Image image_c(*m_device, image_ci);

    VkImageSubresourceLayers layer_all_plane0{VK_IMAGE_ASPECT_PLANE_0_BIT, 0, 0, 2};
    VkImageSubresourceLayers layer0_plane0{VK_IMAGE_ASPECT_PLANE_0_BIT, 0, 0, 1};
    VkImageSubresourceLayers layer0_plane1{VK_IMAGE_ASPECT_PLANE_1_BIT, 0, 0, 1};
    VkImageSubresourceLayers layer1_plane1{VK_IMAGE_ASPECT_PLANE_1_BIT, 0, 1, 1};
    VkImageSubresourceRange full_subresource_range{
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT, 0, 1, 0, 2};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D one_four_offset{32, 32, 0};
    VkExtent3D full_extent{128, 128, 1};    // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};      // <-- image type is 2D
    VkExtent3D one_four_extent{32, 32, 1};  // <-- image type is 2D

    VkImageCopy region_all_plane0_to_all_plane0 = {layer_all_plane0, zero_offset, layer_all_plane0, zero_offset, full_extent};
    VkImageCopy region_layer0_plane0_to_layer0_plane0 = {layer0_plane0, zero_offset, layer0_plane0, zero_offset, full_extent};
    VkImageCopy region_layer0_plane0_to_layer0_plane1 = {layer0_plane0, zero_offset, layer0_plane1, zero_offset, half_extent};
    VkImageCopy region_layer1_plane1_to_layer1_plane1_front = {layer1_plane1, zero_offset, layer1_plane1, zero_offset,
                                                               one_four_extent};
    VkImageCopy region_layer1_plane1_to_layer1_plane1_back = {layer1_plane1, one_four_offset, layer1_plane1, one_four_offset,
                                                              one_four_extent};

    m_command_buffer.Begin();

    image_c.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_command_buffer.handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_a.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer0_plane0_to_layer0_plane0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer0_plane0_to_layer0_plane1);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer0_plane0_to_layer0_plane1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer1_plane1_to_layer1_plane1_front);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer1_plane1_to_layer1_plane1_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer1_plane1_to_layer1_plane1_back);

    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, CopyLinearImageHazards) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_LINEAR);
    vkt::Image image_a(*m_device, image_ci);
    vkt::Image image_b(*m_device, image_ci);
    vkt::Image image_c(*m_device, image_ci);

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{64, 64, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};    // <-- image type is 2D

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};
    VkImageCopy region_front = {layers_all, zero_offset, layers_all, zero_offset, half_extent};
    VkImageCopy region_back = {layers_all, half_offset, layers_all, half_offset, half_extent};

    m_command_buffer.Begin();

    image_c.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_command_buffer.handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_b.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_front);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_back);
}

TEST_F(NegativeSyncVal, CopyLinearMultiPlanarHazards) {
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());

    RETURN_IF_SKIP(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    const auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_LINEAR);
    // Verify format
    bool supported = ImageFormatIsSupported(instance(), Gpu(), image_ci,
                                            VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
    if (!supported) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    vkt::Image image_a(*m_device, image_ci);
    vkt::Image image_b(*m_device, image_ci);
    vkt::Image image_c(*m_device, image_ci);

    VkImageSubresourceLayers layer_all_plane0{VK_IMAGE_ASPECT_PLANE_0_BIT, 0, 0, 1};
    VkImageSubresourceLayers layer_all_plane1{VK_IMAGE_ASPECT_PLANE_1_BIT, 0, 0, 1};
    VkImageSubresourceRange full_subresource_range{
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT, 0, 1, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D one_four_offset{32, 32, 0};
    VkExtent3D full_extent{128, 128, 1};    // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};      // <-- image type is 2D
    VkExtent3D one_four_extent{32, 32, 1};  // <-- image type is 2D

    VkImageCopy region_plane0_to_plane0 = {layer_all_plane0, zero_offset, layer_all_plane0, zero_offset, full_extent};
    VkImageCopy region_plane0_to_plane1 = {layer_all_plane0, zero_offset, layer_all_plane1, zero_offset, half_extent};
    VkImageCopy region_plane1_to_plane1_front = {layer_all_plane1, zero_offset, layer_all_plane1, zero_offset, one_four_extent};
    VkImageCopy region_plane1_to_plane1_back = {layer_all_plane1, one_four_offset, layer_all_plane1, one_four_offset,
                                                one_four_extent};

    m_command_buffer.Begin();

    image_c.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_command_buffer.handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_a.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane1);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane1_to_plane1_front);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane1_to_plane1_front);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane1_to_plane1_back);

    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, CopyBufferImageHazards) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 2048, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 2048, transfer_usage, mem_prop);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 2, format, usage);
    vkt::Image image_a(*m_device, image_ci);
    vkt::Image image_b(*m_device, image_ci);

    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{16, 16, 0};
    VkExtent3D half_extent{16, 16, 1};  // <-- image type is 2D

    VkBufferImageCopy region_buffer_front_image_0_front = {0, 16, 16, layers_0, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_front_image_1_front = {0, 16, 16, layers_1, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_front_image_1_back = {0, 16, 16, layers_1, half_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_0_front = {1024, 16, 16, layers_0, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_0_back = {1024, 16, 16, layers_0, half_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_1_front = {1024, 16, 16, layers_1, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_1_back = {1024, 16, 16, layers_1, half_offset, half_extent};

    m_command_buffer.Begin();
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_command_buffer.handle();
    vk::CmdCopyBufferToImage(cb, buffer_a.handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_0_front);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_a.handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_back_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_front_image_1_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_front_image_1_back);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1, &region_buffer_back_image_0_back);

    VkBufferMemoryBarrier buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier.buffer = buffer_a.handle();
    buffer_barrier.offset = 1024;
    buffer_barrier.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_back_image_1_front);

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1, &region_buffer_back_image_1_back);

    vk::CmdCopyImageToBuffer(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_b.handle(), 1,
                             &region_buffer_front_image_0_front);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImageToBuffer(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_b.handle(), 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_back_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_1_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_1_back);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_buffer_back_image_0_back);

    buffer_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buffer_barrier.buffer = buffer_b.handle();
    buffer_barrier.offset = 1024;
    buffer_barrier.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_back_image_1_front);

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_buffer_back_image_1_back);

    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, BlitImageHazards) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 2, format, usage);
    vkt::Image image_a(*m_device, image_ci);
    vkt::Image image_b(*m_device, image_ci);

    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_0_offset{16, 16, 0};
    VkOffset3D half_1_offset{16, 16, 1};
    VkOffset3D full_offset{32, 32, 1};
    VkImageBlit region_0_front_1_front = {layers_0, {zero_offset, half_1_offset}, layers_1, {zero_offset, half_1_offset}};
    VkImageBlit region_1_front_0_front = {layers_1, {zero_offset, half_1_offset}, layers_0, {zero_offset, half_1_offset}};
    VkImageBlit region_1_back_0_back = {layers_1, {half_0_offset, full_offset}, layers_0, {half_0_offset, full_offset}};

    m_command_buffer.Begin();
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_command_buffer.handle();

    vk::CmdBlitImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_0_front_1_front, VK_FILTER_NEAREST);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdBlitImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_0_front_1_front, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdBlitImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_1_front_0_front, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    vk::CmdBlitImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_1_back_0_back, VK_FILTER_NEAREST);

    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, RenderPassBeginTransitionHazard) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Image rt_image_0(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    auto rt_image_view_0 = rt_image_0.CreateView();

    vkt::Image rt_image_1(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    auto rt_image_view_1 = rt_image_1.CreateView();

    const VkSubpassDependency external_subpass_dependency = {VK_SUBPASS_EXTERNAL,
                                                             0,
                                                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                             VK_DEPENDENCY_BY_REGION_BIT};

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddColorAttachment(1);
    rp.AddSubpassDependency(external_subpass_dependency);
    rp.CreateRenderPass();
    VkImageView views[2] = {rt_image_view_0.handle(), rt_image_view_1.handle()};
    vkt::Framebuffer fb(*m_device, rp.Handle(), 2, views);

    // Other buffers with which to interact
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const auto image_ci = vkt::Image::ImageCreateInfo2D(m_width, m_height, 1, 1, format, usage);
    vkt::Image image_a(*m_device, image_ci);
    vkt::Image image_b(*m_device, image_ci);

    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{m_width, m_height, 1};  // <-- image type is 2D
    VkImageSubresourceLayers layer_color{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageCopy region_to_copy = {layer_color, zero_offset, layer_color, zero_offset, full_extent};

    auto cb = m_command_buffer.handle();

    m_command_buffer.Begin();
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    rt_image_0.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    rt_image_1.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    rt_image_0.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, rt_image_0.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_to_copy);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());  // This fails so the driver call is skip and no end is valid
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageSubresourceRange rt_full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.image = rt_image_0.handle();
    image_barrier.subresourceRange = rt_full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0,
                           nullptr, 1, &image_barrier);
    vk::CmdCopyImage(cb, rt_image_1.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_to_copy);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());  // This fails so the driver call is skip and no end is valid
    m_errorMonitor->VerifyFound();

    // A global execution barrier that the implict external dependency can chain with should work...
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 0,
                           nullptr);

    // With the barrier above, the layout transition has a chained execution sync operation, and the default
    // implict VkSubpassDependency safes the load op clear vs. the layout transition...
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());
    m_command_buffer.EndRenderPass();
}

TEST_F(NegativeSyncVal, AttachmentLoadHazard) {
    TEST_DESCRIPTION("Copying to attachment creates hazard with attachment load operation");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Image dst_image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM,
                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    auto dst_image_view = dst_image.CreateView();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &dst_image_view.handle());

    // Transition layout manually, so the render pass will start with LOAD_OP operation instead of layout transition.
    dst_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image src_image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    src_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy region = {};
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.extent = {m_width, m_height, 1};

    m_command_buffer.Begin();
    // Initiate copy write
    vk::CmdCopyImage(m_command_buffer, src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region);

    // Execution barrier to ensure that copy and loadOp operations do not overlap.
    // This does not synchronize memory accesses though and WAW hazard remains.
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                           nullptr, 0, nullptr, 0, nullptr);

    // Attachment load operation collides with copy
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, AttachmentStoreHazard) {
    TEST_DESCRIPTION("Copying to attachment creates hazard with attachment store operation");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    // GENERAL layout is needed to perform a copy. Also,
    // the absence of layout transition after the render pass is needed because the test hazards
    // attachment store operation with subsequent copy (and the transition would happen in between).
    vkt::Image dst_image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM,
                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    auto dst_image_view = dst_image.CreateView();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &dst_image_view.handle());

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy region = {};
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.extent = {m_width, m_height, 1};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());

    // Initiate attachment store
    m_command_buffer.EndRenderPass();

    // Execution barrier to ensure that storeOp and copy operations do not overlap.
    // This does not synchronize memory accesses though and WAW hazard remains.
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                           nullptr, 0, nullptr, 0, nullptr);

    // Collide with attachment store by copying to the same attachment
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, DynamicRenderingAttachmentLoadHazard) {
    TEST_DESCRIPTION("Copying to attachment creates hazard with attachment load operation");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    InitRenderTarget();
    m_renderTargets[0]->SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy region = {};
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.extent = {m_width, m_height, 1};

    vkt::ImageView render_target_view = m_renderTargets[0]->CreateView();
    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = render_target_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
    color_attachment.clearValue.color = m_clear_color;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {m_width, m_height};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    // Initiate copy write
    vk::CmdCopyImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, *m_renderTargets[0], VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Execution barrier to ensure that copy and loadOp operations do not overlap.
    // This does not synchronize memory accesses though and WAW hazard remains.
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                           nullptr, 0, nullptr, 0, nullptr);

    // Attachment load operation collides with copy
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, DynamicRenderingAttachmentStoreHazard) {
    TEST_DESCRIPTION("Copying to attachment creates hazard with attachment store operation");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    InitRenderTarget();
    m_renderTargets[0]->SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy region = {};
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.extent = {m_width, m_height, 1};

    vkt::ImageView render_target_view = m_renderTargets[0]->CreateView();
    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = render_target_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue.color = m_clear_color;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {m_width, m_height};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    // Initiate attachment store
    vk::CmdEndRendering(m_command_buffer);

    // Execution barrier to ensure that storeOp and copy operations do not overlap.
    // This does not synchronize memory accesses though and WAW hazard remains.
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                           nullptr, 0, nullptr, 0, nullptr);

    // Collide with attachment store by copying to the same attachment
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, *m_renderTargets[0], VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, CmdDispatchDrawHazards) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkImageUsageFlags image_usage_combine = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const auto image_c_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_combine);
    vkt::Image image_c_a(*m_device, image_c_ci, vkt::set_layout);
    vkt::Image image_c_b(*m_device, image_c_ci, vkt::set_layout);

    vkt::ImageView imageview_c = image_c_a.CreateView();
    VkImageUsageFlags image_usage_storage =
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    const auto image_s_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_storage);
    vkt::Image image_s_a(*m_device, image_s_ci, vkt::set_layout);
    vkt::Image image_s_b(*m_device, image_s_ci, vkt::set_layout);
    image_s_a.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_s_b.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView imageview_s = image_s_a.CreateView();

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler_s(*m_device, sampler_ci);
    vkt::Sampler sampler_c(*m_device, sampler_ci);

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, vkt::Buffer::CreateInfo(2048, buffer_usage), mem_prop);
    vkt::Buffer buffer_b(*m_device, vkt::Buffer::CreateInfo(2048, buffer_usage), mem_prop);

    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = buffer_a.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.offset = 0;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView bufferview(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a.handle(), 0, 2048);
    descriptor_set.WriteDescriptorImageInfo(1, imageview_c, sampler_c.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorImageInfo(2, imageview_s, sampler_s.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorBufferView(3, bufferview.handle());
    descriptor_set.UpdateDescriptorSets();

    // Dispatch
    const char *csSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform foo { float x; } ub0;
        layout(set=0, binding=1) uniform sampler2D cis1;
        layout(set=0, binding=2, rgba8) uniform readonly image2D si2;
        layout(set=0, binding=3, r32f) uniform readonly imageBuffer stb3;
        void main(){
            vec4 vColor4;
            vColor4.x = ub0.x;
            vColor4 = texture(cis1, vec2(0));
            vColor4 = imageLoad(si2, ivec2(0));
            vColor4 = imageLoad(stb3, 0);
        }
    )glsl";

    vkt::Event event(*m_device);
    VkEvent event_handle = event.handle();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();

    VkBufferCopy buffer_region = {0, 0, 2048};
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_b.handle(), buffer_a.handle(), 1, &buffer_region);

    VkImageSubresourceLayers layer{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{16, 16, 1};
    VkImageCopy image_region = {layer, zero_offset, layer, zero_offset, full_extent};
    vk::CmdCopyImage(m_command_buffer.handle(), image_c_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);
    vk::CmdCopyImage(m_command_buffer.handle(), image_s_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_s_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
    m_command_buffer.Reset();
    m_command_buffer.Begin();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_b.handle(), buffer_a.handle(), 1, &buffer_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyImage(m_command_buffer.handle(), image_c_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
    m_command_buffer.Reset();

    // DispatchIndirect
    buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_dispatchIndirect(*m_device, sizeof(VkDispatchIndirectCommand), buffer_usage, mem_prop);
    vkt::Buffer buffer_dispatchIndirect2(*m_device, sizeof(VkDispatchIndirectCommand), buffer_usage, mem_prop);
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatchIndirect(m_command_buffer.handle(), buffer_dispatchIndirect.handle(), 0);
    m_command_buffer.End();

    m_command_buffer.Reset();
    m_command_buffer.Begin();

    buffer_region = {0, 0, sizeof(VkDispatchIndirectCommand)};
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_dispatchIndirect2.handle(), buffer_dispatchIndirect.handle(), 1,
                      &buffer_region);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDispatchIndirect(m_command_buffer.handle(), buffer_dispatchIndirect.handle(), 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Draw
    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkVertexInputAttributeDescription VertexInputAttributeDescription = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vbo_data)};
    VkVertexInputBindingDescription VertexInputBindingDescription = {0, sizeof(vbo_data), VK_VERTEX_INPUT_RATE_VERTEX};
    buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer vbo(*m_device, vkt::Buffer::CreateInfo(sizeof(vbo_data), buffer_usage), mem_prop);
    vkt::Buffer vbo2(*m_device, vkt::Buffer::CreateInfo(sizeof(vbo_data), buffer_usage), mem_prop);

    VkShaderObj fs(this, csSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.vi_ci_.pVertexBindingDescriptions = &VertexInputBindingDescription;
    g_pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    g_pipe.vi_ci_.pVertexAttributeDescriptions = &VertexInputAttributeDescription;
    g_pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    m_command_buffer.Reset();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_command_buffer.Reset();
    m_command_buffer.Begin();

    buffer_region = {0, 0, sizeof(vbo_data)};
    vk::CmdCopyBuffer(m_command_buffer.handle(), vbo2.handle(), vbo.handle(), 1, &buffer_region);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Repeat the draw test with a WaitEvent to protect it.
    m_command_buffer.Reset();
    m_command_buffer.Begin();

    vk::CmdCopyBuffer(m_command_buffer.handle(), vbo2.handle(), vbo.handle(), 1, &buffer_region);

    VkBufferMemoryBarrier vbo_barrier = vku::InitStructHelper();
    vbo_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vbo_barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    vbo_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vbo_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vbo_barrier.buffer = vbo.handle();
    vbo_barrier.offset = buffer_region.dstOffset;
    vbo_barrier.size = buffer_region.size;

    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, nullptr, 1,
                                &vbo_barrier, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // DrawIndexed
    const float ibo_data[3] = {0.f, 0.f, 0.f};
    buffer_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer ibo(*m_device, vkt::Buffer::CreateInfo(sizeof(ibo_data), buffer_usage), mem_prop);
    vkt::Buffer ibo2(*m_device, vkt::Buffer::CreateInfo(sizeof(ibo_data), buffer_usage), mem_prop);

    m_command_buffer.Reset();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_command_buffer.Reset();
    m_command_buffer.Begin();

    buffer_region = {0, 0, sizeof(ibo_data)};
    vk::CmdCopyBuffer(m_command_buffer.handle(), ibo2.handle(), ibo.handle(), 1, &buffer_region);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // DrawIndirect
    buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_drawIndirect(*m_device, sizeof(VkDrawIndexedIndirectCommand), buffer_usage, mem_prop);
    vkt::Buffer buffer_drawIndirect2(*m_device, sizeof(VkDrawIndirectCommand), buffer_usage, mem_prop);

    m_command_buffer.Reset();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawIndirect(m_command_buffer.handle(), buffer_drawIndirect.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_command_buffer.Reset();
    m_command_buffer.Begin();

    buffer_region = {0, 0, sizeof(VkDrawIndirectCommand)};
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_drawIndirect2.handle(), buffer_drawIndirect.handle(), 1, &buffer_region);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDrawIndirect(m_command_buffer.handle(), buffer_drawIndirect.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // DrawIndexedIndirect
    buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_drawIndexedIndirect(*m_device, sizeof(VkDrawIndexedIndirectCommand), buffer_usage, mem_prop);
    vkt::Buffer buffer_drawIndexedIndirect2(*m_device, sizeof(VkDrawIndexedIndirectCommand), buffer_usage, mem_prop);

    m_command_buffer.Reset();
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), buffer_drawIndirect.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_command_buffer.Reset();
    m_command_buffer.Begin();

    buffer_region = {0, 0, sizeof(VkDrawIndexedIndirectCommand)};
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_drawIndexedIndirect2.handle(), buffer_drawIndexedIndirect.handle(), 1,
                      &buffer_region);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), buffer_drawIndexedIndirect.handle(), 0, 1,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, CmdDispatchDrawHazardsDrawIndirectCount) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkImageUsageFlags image_usage_combine = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const auto image_c_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_combine);
    vkt::Image image_c_a(*m_device, image_c_ci, vkt::set_layout);
    vkt::ImageView imageview_c = image_c_a.CreateView();

    VkImageUsageFlags image_usage_storage =
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    const auto image_s_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_storage);
    vkt::Image image_s_a(*m_device, image_s_ci, vkt::set_layout);
    image_s_a.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView imageview_s = image_s_a.CreateView();

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler_s(*m_device, sampler_ci);
    vkt::Sampler sampler_c(*m_device, sampler_ci);

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, vkt::Buffer::CreateInfo(2048, buffer_usage), mem_prop);

    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = buffer_a.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.offset = 0;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView bufferview(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a.handle(), 0, 2048);
    descriptor_set.WriteDescriptorImageInfo(1, imageview_c, sampler_c.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorImageInfo(2, imageview_s, sampler_s.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorBufferView(3, bufferview.handle());
    descriptor_set.UpdateDescriptorSets();

    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkVertexInputAttributeDescription VertexInputAttributeDescription = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vbo_data)};
    VkVertexInputBindingDescription VertexInputBindingDescription = {0, sizeof(vbo_data), VK_VERTEX_INPUT_RATE_VERTEX};
    buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer vbo(*m_device, vkt::Buffer::CreateInfo(sizeof(vbo_data), buffer_usage), mem_prop);
    vkt::Buffer vbo2(*m_device, vkt::Buffer::CreateInfo(sizeof(vbo_data), buffer_usage), mem_prop);

    const char* fs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform foo { float x; } ub0;
        layout(set=0, binding=1) uniform sampler2D cis1;
        layout(set=0, binding=2, rgba8) uniform readonly image2D si2;
        layout(set=0, binding=3, r32f) uniform readonly imageBuffer stb3;
        void main(){
            vec4 vColor4;
            vColor4.x = ub0.x;
            vColor4 = texture(cis1, vec2(0));
            vColor4 = imageLoad(si2, ivec2(0));
            vColor4 = imageLoad(stb3, 0);
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.vi_ci_.pVertexBindingDescriptions = &VertexInputBindingDescription;
    g_pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    g_pipe.vi_ci_.pVertexAttributeDescriptions = &VertexInputAttributeDescription;
    g_pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_count(*m_device, sizeof(uint32_t), buffer_usage, mem_prop);
    vkt::Buffer buffer_count2(*m_device, sizeof(uint32_t), buffer_usage, mem_prop);

    VkDeviceSize offset = 0;
    // DrawIndirectCount
    {
        buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        vkt::Buffer buffer_drawIndirect(*m_device, sizeof(VkDrawIndexedIndirectCommand), buffer_usage, mem_prop);

        m_command_buffer.Reset();
        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);

        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0,
                                  1, &descriptor_set.set_, 0, nullptr);
        vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), buffer_drawIndirect.handle(), 0, buffer_count.handle(), 0, 1,
                                    sizeof(VkDrawIndirectCommand));
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();

        m_command_buffer.Reset();
        m_command_buffer.Begin();

        VkBufferCopy buffer_region = {0, 0, sizeof(uint32_t)};
        vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_count2.handle(), buffer_count.handle(), 1, &buffer_region);

        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0,
                                  1, &descriptor_set.set_, 0, nullptr);

        m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
        vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), buffer_drawIndirect.handle(), 0, buffer_count.handle(), 0, 1,
                                    sizeof(VkDrawIndirectCommand));
        m_errorMonitor->VerifyFound();

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    // DrawIndexedIndirectCount
    {
        buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        vkt::Buffer buffer_drawIndexedIndirect(*m_device, sizeof(VkDrawIndexedIndirectCommand), buffer_usage, mem_prop);

        const float ibo_data[3] = {0.f, 0.f, 0.f};
        buffer_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        vkt::Buffer ibo(*m_device, vkt::Buffer::CreateInfo(sizeof(ibo_data), buffer_usage), mem_prop);
        vkt::Buffer ibo2(*m_device, vkt::Buffer::CreateInfo(sizeof(ibo_data), buffer_usage), mem_prop);

        m_command_buffer.Reset();
        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
        vk::CmdBindIndexBuffer(m_command_buffer.handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);

        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0,
                                  1, &descriptor_set.set_, 0, nullptr);
        vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), buffer_drawIndexedIndirect.handle(), 0, buffer_count.handle(),
                                           0, 1, sizeof(VkDrawIndexedIndirectCommand));
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();

        m_command_buffer.Reset();
        m_command_buffer.Begin();

        VkBufferCopy buffer_region = {0, 0, sizeof(uint32_t)};
        vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_count2.handle(), buffer_count.handle(), 1, &buffer_region);

        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
        vk::CmdBindIndexBuffer(m_command_buffer.handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0,
                                  1, &descriptor_set.set_, 0, nullptr);

        m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
        vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), buffer_drawIndexedIndirect.handle(), 0, buffer_count.handle(),
                                           0, 1, sizeof(VkDrawIndexedIndirectCommand));
        m_errorMonitor->VerifyFound();

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
}

TEST_F(NegativeSyncVal, CmdClear) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    // CmdClearColorImage
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    vkt::Image image_a(*m_device, image_ci, vkt::set_layout);
    vkt::Image image_b(*m_device, image_ci, vkt::set_layout);

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};

    m_command_buffer.Begin();

    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_command_buffer.handle();
    VkClearColorValue ccv = {};
    vk::CmdClearColorImage(m_command_buffer.handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);
    m_command_buffer.End();

    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdClearColorImage(m_command_buffer.handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdClearColorImage(m_command_buffer.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();

    // CmdClearDepthStencilImage
    format = FindSupportedDepthStencilFormat(Gpu());
    image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    vkt::Image image_ds_a(*m_device, image_ci);
    vkt::Image image_ds_b(*m_device, image_ci);

    const VkImageAspectFlags ds_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    image_ds_a.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_GENERAL);
    image_ds_b.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_GENERAL);

    m_command_buffer.Begin();
    const VkClearDepthStencilValue clear_value = {};
    VkImageSubresourceRange ds_range = {ds_aspect, 0, 1, 0, 1};

    vk::CmdClearDepthStencilImage(cb, image_ds_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &ds_range);
    m_command_buffer.End();

    VkImageSubresourceLayers ds_layers_all{ds_aspect, 0, 0, 1};
    VkImageCopy ds_full_region = {ds_layers_all, zero_offset, ds_layers_all, zero_offset, full_extent};

    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdCopyImage(cb, image_ds_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_ds_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &ds_full_region);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), image_ds_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,
                                  &ds_range);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), image_ds_b.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,
                                  &ds_range);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, CmdQuery) {
    // CmdCopyQueryPoolResults
    all_queue_count_ = true;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    if ((m_device->Physical().queue_properties_.empty()) || (m_device->Physical().queue_properties_[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 256, transfer_usage, mem_prop);

    VkBufferCopy region = {0, 0, 256};

    auto cb = m_command_buffer.handle();
    m_command_buffer.Begin();
    vk::CmdResetQueryPool(cb, query_pool.handle(), 0, 1);
    vk::CmdWriteTimestamp(cb, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);
    vk::CmdCopyQueryPoolResults(cb, query_pool.handle(), 0, 1, buffer_a.handle(), 0, 0, VK_QUERY_RESULT_WAIT_BIT);
    m_command_buffer.End();

    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    vk::CmdResetQueryPool(cb, query_pool.handle(), 0, 1);
    vk::CmdWriteTimestamp(cb, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyQueryPoolResults(cb, query_pool.handle(), 0, 1, buffer_a.handle(), 0, 256, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyQueryPoolResults(cb, query_pool.handle(), 0, 1, buffer_b.handle(), 0, 256, VK_QUERY_RESULT_WAIT_BIT);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();

    // TODO:Track VkQueryPool
    // TODO:CmdWriteTimestamp
}

TEST_F(NegativeSyncVal, CmdDrawDepthStencil) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    auto format_ds = FindSupportedDepthStencilFormat(Gpu());

    // Vulkan doesn't support copying between different depth stencil formats, so the formats have to change.
    auto format_dp = format_ds;
    auto format_st = format_ds;

    vkt::Image image_ds(*m_device, 16, 16, 1, format_ds,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image_dp(*m_device, 16, 16, 1, format_dp,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image_st(*m_device, 16, 16, 1, format_st,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView image_view_ds = image_ds.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    vkt::ImageView image_view_dp = image_dp.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    vkt::ImageView image_view_st = image_st.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pDepthStencilAttachment = &attach;

    VkAttachmentDescription attach_desc = {};
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    attach_desc.format = format_ds;
    vkt::RenderPass rp_ds(*m_device, rpci);
    attach_desc.format = format_dp;
    vkt::RenderPass rp_dp(*m_device, rpci);
    attach_desc.format = format_st;
    vkt::RenderPass rp_st(*m_device, rpci);

    vkt::Framebuffer fb_ds(*m_device, rp_ds.handle(), 1, &image_view_ds.handle(), 16, 16);
    vkt::Framebuffer fb_dp(*m_device, rp_dp.handle(), 1, &image_view_dp.handle(), 16, 16);
    vkt::Framebuffer fb_st(*m_device, rp_st.handle(), 1, &image_view_st.handle(), 16, 16);

    VkStencilOpState stencil = {};
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.compareOp = VK_COMPARE_OP_NEVER;

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthWriteEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
    ds_ci.stencilTestEnable = VK_TRUE;
    ds_ci.front = stencil;
    ds_ci.back = stencil;

    CreatePipelineHelper g_pipe_ds(*this), g_pipe_dp(*this), g_pipe_st(*this);
    g_pipe_ds.gp_ci_.renderPass = rp_ds.handle();
    g_pipe_ds.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe_ds.CreateGraphicsPipeline();
    g_pipe_dp.gp_ci_.renderPass = rp_dp.handle();
    ds_ci.stencilTestEnable = VK_FALSE;
    g_pipe_dp.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe_dp.CreateGraphicsPipeline();
    g_pipe_st.gp_ci_.renderPass = rp_st.handle();
    ds_ci.depthTestEnable = VK_FALSE;
    ds_ci.stencilTestEnable = VK_TRUE;
    g_pipe_st.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe_st.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_renderPassBeginInfo.renderArea = {{0, 0}, {16, 16}};
    m_renderPassBeginInfo.pClearValues = nullptr;
    m_renderPassBeginInfo.clearValueCount = 0;

    m_renderPassBeginInfo.renderPass = rp_ds.handle();
    m_renderPassBeginInfo.framebuffer = fb_ds.handle();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_ds.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();

    m_renderPassBeginInfo.renderPass = rp_dp.handle();
    m_renderPassBeginInfo.framebuffer = fb_dp.handle();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_dp.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();

    m_renderPassBeginInfo.renderPass = rp_st.handle();
    m_renderPassBeginInfo.framebuffer = fb_st.handle();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_st.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();

    m_command_buffer.End();

    m_command_buffer.Reset();
    m_command_buffer.Begin();

    image_ds.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_dp.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_st.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy copyRegion;
    copyRegion.srcSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent = {16, 16, 1};

    vk::CmdCopyImage(m_command_buffer.handle(), image_ds.handle(), VK_IMAGE_LAYOUT_GENERAL, image_dp.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);

    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    vk::CmdCopyImage(m_command_buffer.handle(), image_ds.handle(), VK_IMAGE_LAYOUT_GENERAL, image_st.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_renderPassBeginInfo.renderPass = rp_ds.handle();
    m_renderPassBeginInfo.framebuffer = fb_ds.handle();
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    m_renderPassBeginInfo.renderPass = rp_dp.handle();
    m_renderPassBeginInfo.framebuffer = fb_dp.handle();
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    m_renderPassBeginInfo.renderPass = rp_st.handle();
    m_renderPassBeginInfo.framebuffer = fb_st.handle();
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, RenderPassLoadHazardVsInitialLayout) {
    AddOptionalExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    const bool load_store_op_none = IsExtensionsEnabled(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);

    VkImageUsageFlags usage_color = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageUsageFlags usage_input = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, format, usage_color);
    vkt::Image image_color(*m_device, image_ci);
    image_ci.usage = usage_input;
    vkt::Image image_input(*m_device, image_ci);
    vkt::ImageView image_color_view = image_color.CreateView();
    vkt::ImageView image_input_view = image_input.CreateView();
    VkImageView attachments[] = {image_color_view, image_input_view};

    VkAttachmentDescription attachmentDescriptions[] = {
        // Result attachment
        {(VkAttachmentDescriptionFlags)0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_UNDEFINED,  // Here causes DesiredError that SYNC-HAZARD-NONE in BeginRenderPass.
                                     // It should be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // Input attachment
        {(VkAttachmentDescriptionFlags)0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};

    const VkAttachmentReference resultAttachmentRef = {0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const VkAttachmentReference inputAttachmentRef = {1u, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    const VkSubpassDescription subpassDescription = {(VkSubpassDescriptionFlags)0,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     1u,
                                                     &inputAttachmentRef,
                                                     1u,
                                                     &resultAttachmentRef,
                                                     0,
                                                     0,
                                                     0u,
                                                     0};

    const VkSubpassDependency subpassDependency = {VK_SUBPASS_EXTERNAL,
                                                   0,
                                                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                   VK_ACCESS_TRANSFER_WRITE_BIT,
                                                   VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT,
                                                   VK_DEPENDENCY_BY_REGION_BIT};

    const VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                                   0,
                                                   (VkRenderPassCreateFlags)0,
                                                   2u,
                                                   attachmentDescriptions,
                                                   1u,
                                                   &subpassDescription,
                                                   1u,
                                                   &subpassDependency};
    vkt::RenderPass rp(*m_device, renderPassInfo);
    vkt::Framebuffer fb(*m_device, rp.handle(), 2, attachments);

    image_input.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.Begin();

    m_renderPassBeginInfo.renderArea = {{0, 0}, {32, 32}};
    m_renderPassBeginInfo.renderPass = rp.handle();
    m_renderPassBeginInfo.framebuffer = fb.handle();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    // Even though we have no accesses prior, the layout transition *is* an access, so load can be validated vs. layout transition
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    vkt::RenderPass rp_no_load_store;
    if (load_store_op_none) {
        attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
        attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_NONE;
        attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
        attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_NONE;
        rp_no_load_store.init(*m_device, renderPassInfo);
        m_renderPassBeginInfo.renderPass = rp_no_load_store.handle();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        m_command_buffer.EndRenderPass();
    } else {
        printf("VK_EXT_load_store_op_none not supported, skipping sub-test\n");
    }
}

TEST_F(NegativeSyncVal, RenderPassWithWrongDepthStencilInitialLayout) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());
    VkImageUsageFlags usage_color = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageUsageFlags usage_ds = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, color_format, usage_color);
    vkt::Image image_color(*m_device, image_ci, vkt::set_layout);
    vkt::Image image_color2(*m_device, image_ci, vkt::set_layout);
    vkt::Image image_ds(*m_device, 32, 32, 1, ds_format, usage_ds);
    image_ds.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    const VkAttachmentDescription colorAttachmentDescription = {(VkAttachmentDescriptionFlags)0,
                                                                color_format,
                                                                VK_SAMPLE_COUNT_1_BIT,
                                                                VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                VK_ATTACHMENT_STORE_OP_STORE,
                                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const VkAttachmentDescription depthStencilAttachmentDescription = {
        (VkAttachmentDescriptionFlags)0, ds_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
        VK_IMAGE_LAYOUT_UNDEFINED,  // Here causes DesiredError that SYNC-HAZARD-WRITE_AFTER_WRITE in BeginRenderPass.
                                    // It should be VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    std::vector<VkAttachmentDescription> attachmentDescriptions;
    attachmentDescriptions.push_back(colorAttachmentDescription);
    attachmentDescriptions.push_back(depthStencilAttachmentDescription);

    const VkAttachmentReference colorAttachmentRef = {0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const VkAttachmentReference depthStencilAttachmentRef = {1u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    const VkSubpassDescription subpassDescription = {(VkSubpassDescriptionFlags)0,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     0u,
                                                     0,
                                                     1u,
                                                     &colorAttachmentRef,
                                                     0,
                                                     &depthStencilAttachmentRef,
                                                     0u,
                                                     0};

    const VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                                   0,
                                                   (VkRenderPassCreateFlags)0,
                                                   (uint32_t)attachmentDescriptions.size(),
                                                   &attachmentDescriptions[0],
                                                   1u,
                                                   &subpassDescription,
                                                   0u,
                                                   0};
    vkt::RenderPass rp(*m_device, renderPassInfo);

    vkt::ImageView color_view = image_color.CreateView();
    vkt::ImageView depth_view = image_ds.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    VkImageView fb_attachments[] = {color_view, depth_view};

    vkt::Framebuffer fb(*m_device, rp.handle(), 2u, fb_attachments);

    vkt::ImageView color2_view = image_color2.CreateView();
    fb_attachments[0] = color2_view;
    vkt::Framebuffer fb1(*m_device, rp.handle(), 2u, fb_attachments);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.gp_ci_.renderPass = rp.handle();

    VkStencilOpState stencil = {};
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.compareOp = VK_COMPARE_OP_NEVER;

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthWriteEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
    ds_ci.stencilTestEnable = VK_TRUE;
    ds_ci.front = stencil;
    ds_ci.back = stencil;

    g_pipe.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    VkClearValue clear = {};
    std::array<VkClearValue, 2> clear_values = { {clear, clear} };
    m_renderPassBeginInfo.pClearValues = clear_values.data();
    m_renderPassBeginInfo.clearValueCount = clear_values.size();
    m_renderPassBeginInfo.renderArea = {{0, 0}, {32, 32}};
    m_renderPassBeginInfo.renderPass = rp.handle();

    m_renderPassBeginInfo.framebuffer = fb.handle();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();

    m_renderPassBeginInfo.framebuffer = fb1.handle();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
}

struct CreateRenderPassHelper {
    class SubpassDescriptionStore {
      public:
        using AttachRefVec = std::vector<VkAttachmentReference>;
        using PreserveVec = std::vector<uint32_t>;

        SubpassDescriptionStore() = default;
        SubpassDescriptionStore(const AttachRefVec& input, const AttachRefVec& color) : input_store(input), color_store(color) {}
        void SetResolve(const AttachRefVec& resolve) { resolve_store = resolve; }
        void SetDepthStencil(const AttachRefVec& ds) { ds_store = ds; }
        void SetPreserve(const PreserveVec& preserve) { preserve_store = preserve; }

        VkSubpassDescription operator*() const {
            VkSubpassDescription desc = {0u,
                                         VK_PIPELINE_BIND_POINT_GRAPHICS,
                                         static_cast<uint32_t>(input_store.size()),
                                         vvl::DataOrNull(input_store),
                                         static_cast<uint32_t>(color_store.size()),
                                         vvl::DataOrNull(color_store),
                                         vvl::DataOrNull(resolve_store),
                                         vvl::DataOrNull(ds_store),
                                         static_cast<uint32_t>(preserve_store.size()),
                                         vvl::DataOrNull(preserve_store)};
            return desc;
        }

      private:
        AttachRefVec input_store;
        AttachRefVec color_store;
        AttachRefVec resolve_store;
        AttachRefVec ds_store;
        PreserveVec preserve_store;
    };

    VkImageUsageFlags usage_color =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageUsageFlags usage_input =
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkClearColorValue ccv = {};

    vkt::Device* dev;
    const static uint32_t kDefaultImageSize = 64;
    uint32_t width = kDefaultImageSize;
    uint32_t height = kDefaultImageSize;
    std::shared_ptr<vkt::Image> image_color;
    std::shared_ptr<vkt::Image> image_input;
    vkt::ImageView view_input;
    vkt::ImageView view_color;

    VkAttachmentReference color_ref;
    VkAttachmentReference input_ref;
    std::vector<VkImageView> attachments;
    VkAttachmentDescription fb_attach_desc;
    VkAttachmentDescription input_attach_desc;
    std::vector<VkAttachmentDescription> attachment_descs;
    std::vector<VkAttachmentReference> input_attachments;
    std::vector<VkAttachmentReference> color_attachments;
    std::vector<VkSubpassDependency> subpass_dep;
    std::vector<VkSubpassDescription> subpasses;
    std::vector<SubpassDescriptionStore> subpass_description_store;
    VkRenderPassCreateInfo render_pass_create_info;
    std::shared_ptr<vkt::RenderPass> render_pass;
    std::shared_ptr<vkt::Framebuffer> framebuffer;
    VkRenderPassBeginInfo render_pass_begin;
    std::vector<VkClearValue> clear_colors;

    CreateRenderPassHelper(vkt::Device* dev_)
        : dev(dev_),
          color_ref(DefaultColorRef()),
          input_ref(DefaultInputRef()),
          fb_attach_desc(DefaultFbAttachDesc()),
          input_attach_desc(DefaultInputAttachDesc()) {}

    void InitImageAndView() {
        auto image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, format, usage_input);
        image_input = std::make_shared<vkt::Image>(*dev, image_ci, 0);
        image_ci.usage = usage_color;
        image_color = std::make_shared<vkt::Image>(*dev, image_ci, 0);

        view_input = image_input->CreateView();
        view_color = image_color->CreateView();
        attachments = {view_color, view_input};
    }

    static VkAttachmentReference DefaultColorRef() {
        return {
            0u,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
    }

    static VkAttachmentReference DefaultInputRef() {
        return {
            1u,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    };

    static VkAttachmentReference UnusedColorAttachmentRef() {
        return {
            VK_ATTACHMENT_UNUSED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    };

    VkAttachmentDescription DefaultFbAttachDesc() {
        return VkAttachmentDescription{
            0u,
            format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
    }
    VkAttachmentDescription DefaultInputAttachDesc() const {
        return VkAttachmentDescription{
            0u,
            format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_LOAD,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
        };
    }

    void InitAllAttachmentsToLayoutGeneral() {
        fb_attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        fb_attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        color_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
        input_attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        input_attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        input_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    }

    void SetAttachmentLayout(vkt::Image* image, const VkAttachmentDescription& attach_desc) {
        if (image && image->initialized() && (attach_desc.initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)) {
            image->SetLayout(attach_desc.initialLayout);
        }
    }

    void SetColorLayout() { SetAttachmentLayout(image_color.get(), fb_attach_desc); }
    void SetInputLayout() { SetAttachmentLayout(image_input.get(), input_attach_desc); }

    void InitAttachmentLayouts() {
        SetColorLayout();
        SetInputLayout();
    }

    void InitAttachmentArrays() {
        // Add attachments
        if (attachment_descs.empty()) {
            attachment_descs = {fb_attach_desc, input_attach_desc};
        }
        if (color_attachments.empty()) {
            color_attachments = {color_ref};
        }
        if (input_attachments.empty()) {
            input_attachments = {input_ref};
        }
    }

    void AddSubpassDescription(const std::vector<VkAttachmentReference>& input, const std::vector<VkAttachmentReference>& color) {
        subpass_description_store.emplace_back(input, color);
    }

    // Capture the current input and color attachements, which can then be modified
    void AddInputColorSubpassDescription() { subpass_description_store.emplace_back(input_attachments, color_attachments); }

    // Create a subpass description with all the attachments preserved
    void AddPreserveInputColorSubpassDescription() {
        std::vector<uint32_t> preserve;
        preserve.reserve(input_attachments.size() + color_attachments.size());
        for (const auto& att : input_attachments) {
            preserve.push_back(att.attachment);
        }
        for (const auto& att : color_attachments) {
            preserve.push_back(att.attachment);
        }
        subpass_description_store.emplace_back();
        subpass_description_store.back().SetPreserve(preserve);
    }

    // This is the default for a single subpass renderpass, don't call if you want to change that
    void InitSubpassDescription() {
        if (subpass_description_store.empty()) {
            // The default subpass has input and color attachments
            AddInputColorSubpassDescription();
        }
    }

    void InitSubpasses() {
        if (subpasses.empty()) {
            subpasses.reserve(subpass_description_store.size());
            for (const auto& desc_store : subpass_description_store) {
                subpasses.emplace_back(*desc_store);
            }
        }
    }

    void InitRenderPassInfo() {
        render_pass_create_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0u,
                                   static_cast<uint32_t>(attachment_descs.size()),
                                   attachment_descs.data(),
                                   static_cast<uint32_t>(subpasses.size()),
                                   subpasses.data(),
                                   static_cast<uint32_t>(subpass_dep.size()),
                                   subpass_dep.data()};
    }

    void InitRenderPass() {
        InitAttachmentArrays();
        InitSubpassDescription();
        InitSubpasses();
        InitRenderPassInfo();
        render_pass = std::make_shared<vkt::RenderPass>();
        render_pass->init(*dev, render_pass_create_info);
    }

    void InitFramebuffer() {
        framebuffer = std::make_shared<vkt::Framebuffer>();
        VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                        0,
                                        0u,
                                        render_pass->handle(),
                                        static_cast<uint32_t>(attachments.size()),
                                        attachments.data(),
                                        width,
                                        height,
                                        1u};
        framebuffer->init(*dev, fbci);
    }

    void InitBeginInfo() {
        render_pass_begin = vku::InitStructHelper();
        render_pass_begin.renderArea = {{0, 0}, {width, height}};
        render_pass_begin.renderPass = render_pass->handle();
        render_pass_begin.framebuffer = framebuffer->handle();

        // Simplistic ensure enough clear colors, if not provided
        // TODO: Should eventually be smart enough to fill in color/depth as appropos
        VkClearValue fill_in;
        fill_in.color = ccv;
        for (size_t i = clear_colors.size(); i < attachments.size(); ++i) {
            clear_colors.push_back(fill_in);
        }
        render_pass_begin.clearValueCount = static_cast<uint32_t>(clear_colors.size());
        render_pass_begin.pClearValues = clear_colors.data();
    }

    void InitPipelineHelper(CreatePipelineHelper& g_pipe) {
        g_pipe.ResetShaderInfo(kVertexMinimalGlsl, kFragmentSubpassLoadGlsl);
        g_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        g_pipe.gp_ci_.renderPass = render_pass->handle();
        ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());
    }

    void Init() {
        InitImageAndView();
        InitRenderPass();
        InitFramebuffer();
        InitBeginInfo();
    }
};

struct SyncTestPipeline {
    VkLayerTest& test;
    VkRenderPass rp;
    CreatePipelineHelper g_pipe;
    VkShaderObj vs;
    VkShaderObj fs;
    VkSamplerCreateInfo sampler_info;
    vkt::Sampler sampler;
    VkImageView view_input = VK_NULL_HANDLE;
    SyncTestPipeline(VkLayerTest& test_, VkRenderPass rp_)
        : test(test_),
          rp(rp_),
          g_pipe(test),
          vs(&test, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT),
          fs(&test, kFragmentSubpassLoadGlsl, VK_SHADER_STAGE_FRAGMENT_BIT),
          sampler_info(SafeSaneSamplerCreateInfo()),
          sampler() {
        sampler.init(*test.DeviceObj(), sampler_info);
        g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        g_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        g_pipe.gp_ci_.renderPass = rp;
    }
    void Init() {
        ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());
        g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_input, sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        g_pipe.descriptor_set_->UpdateDescriptorSets();
    }
};

TEST_F(NegativeSyncVal, LayoutTransition) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    CreateRenderPassHelper rp_helper(m_device);
    rp_helper.Init();
    const VkImage image_input_handle = rp_helper.image_input->handle();
    const VkRenderPass rp = rp_helper.render_pass->handle();

    SyncTestPipeline st_pipe(*this, rp);
    st_pipe.view_input = rp_helper.view_input;
    st_pipe.Init();
    const auto& g_pipe = st_pipe.g_pipe;

    m_command_buffer.Begin();
    auto cb = m_command_buffer.handle();
    VkClearColorValue ccv = {};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    const VkImageMemoryBarrier preClearBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, 0, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,   0, 0, image_input_handle,           full_subresource_range,
    };
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                           &preClearBarrier);

    vk::CmdClearColorImage(m_command_buffer.handle(), image_input_handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ccv, 1,
                           &full_subresource_range);

    const VkImageMemoryBarrier postClearBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0,
        0,
        image_input_handle,
        full_subresource_range,
    };
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u, 0u, nullptr,
                           0u, nullptr, 1u, &postClearBarrier);

    m_command_buffer.BeginRenderPass(rp_helper.render_pass_begin);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    // Positive test for ordering rules between load and input attachment usage
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    // Positive test for store ordering vs. input attachment and dependency *to* external for layout transition
    m_command_buffer.EndRenderPass();

    // Catch a conflict with the input attachment final layout transition
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdClearColorImage(m_command_buffer.handle(), image_input_handle, VK_IMAGE_LAYOUT_GENERAL, &ccv, 1,
                           &full_subresource_range);
    m_errorMonitor->VerifyFound();

    // There should be no hazard for ILT after ILT
    m_command_buffer.End();
    vk::ResetCommandPool(device(), m_command_pool.handle(), 0);
    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                           &preClearBarrier);
    const VkImageMemoryBarrier wawBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        0,
        VK_ACCESS_SHADER_READ_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0,
        0,
        image_input_handle,
        full_subresource_range,
    };
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u, nullptr, 0u,
                           nullptr, 1u, &wawBarrier);
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, SubpassMultiDep) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    CreateRenderPassHelper rp_helper_positive(m_device);

    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageSubresourceLayers mip_0_layer_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D image_zero{0, 0, 0};
    VkExtent3D image_size{rp_helper_positive.width, rp_helper_positive.height, 1};

    VkImageCopy full_region{mip_0_layer_0, image_zero, mip_0_layer_0, image_zero, image_size};

    rp_helper_positive.InitImageAndView();
    rp_helper_positive.InitAllAttachmentsToLayoutGeneral();

    // Copy the comon state to the other renderpass helper
    CreateRenderPassHelper rp_helper_negative(m_device);

    auto& subpass_dep_positive = rp_helper_positive.subpass_dep;

    subpass_dep_positive.push_back({VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_DEPENDENCY_BY_REGION_BIT});
    subpass_dep_positive.push_back({VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT});
    subpass_dep_positive.push_back({0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_ACCESS_TRANSFER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT});
    subpass_dep_positive.push_back({0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_ACCESS_TRANSFER_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT});

    rp_helper_positive.InitRenderPass();
    rp_helper_positive.InitFramebuffer();
    rp_helper_positive.InitBeginInfo();

    auto& subpass_dep_negative = rp_helper_negative.subpass_dep;
    subpass_dep_negative.push_back({VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_DEPENDENCY_BY_REGION_BIT});
    // Show that the two barriers do *not* chain by breaking the positive barrier into two bits.
    subpass_dep_negative.push_back({VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
                                    VK_DEPENDENCY_BY_REGION_BIT});
    subpass_dep_negative.push_back({VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                                    VK_DEPENDENCY_BY_REGION_BIT});

    rp_helper_negative.InitAllAttachmentsToLayoutGeneral();

    // Negative and postive RP's are compatible.
    rp_helper_negative.attachments = rp_helper_positive.attachments;
    rp_helper_negative.InitRenderPass();
    rp_helper_negative.InitFramebuffer();
    rp_helper_negative.InitBeginInfo();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    CreatePipelineHelper g_pipe(*this);
    rp_helper_positive.InitPipelineHelper(g_pipe);

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, rp_helper_positive.view_input, VK_NULL_HANDLE,
                                                     VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    auto cb = m_command_buffer.handle();
    VkClearColorValue ccv = {};

    const VkImageMemoryBarrier xferDestBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                                  nullptr,
                                                  VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
                                                  VK_ACCESS_TRANSFER_WRITE_BIT,
                                                  VK_IMAGE_LAYOUT_GENERAL,
                                                  VK_IMAGE_LAYOUT_GENERAL,
                                                  VK_QUEUE_FAMILY_IGNORED,
                                                  VK_QUEUE_FAMILY_IGNORED,
                                                  VK_NULL_HANDLE,
                                                  full_subresource_range};
    const VkImageMemoryBarrier xferDestToSrcBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        VK_NULL_HANDLE,
        full_subresource_range,
    };

    const VkImage image_color = rp_helper_positive.image_color->handle();
    const VkImage image_input = rp_helper_positive.image_input->handle();

    VkImageMemoryBarrier preClearBarrier = xferDestBarrier;
    preClearBarrier.image = image_color;

    VkImageMemoryBarrier preCopyBarriers[2] = {xferDestToSrcBarrier, xferDestBarrier};
    preCopyBarriers[0].image = image_color;
    preCopyBarriers[1].image = image_input;
    // Positive test for ordering rules between load and input attachment usage

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                           &preClearBarrier);

    vk::CmdClearColorImage(m_command_buffer.handle(), image_color, VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 2u,
                           preCopyBarriers);

    vk::CmdCopyImage(m_command_buffer.handle(), image_color, VK_IMAGE_LAYOUT_GENERAL, image_input, VK_IMAGE_LAYOUT_GENERAL, 1u,
                     &full_region);

    // No post copy image barrier, we are testing the subpass dependencies

    // Postive renderpass multidependency test
    m_command_buffer.BeginRenderPass(rp_helper_positive.render_pass_begin);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    // Positive test for store ordering vs. input attachment and dependency *to* external for layout transition
    m_command_buffer.EndRenderPass();

    vk::CmdCopyImage(m_command_buffer.handle(), image_color, VK_IMAGE_LAYOUT_GENERAL, image_input, VK_IMAGE_LAYOUT_GENERAL, 1u,
                     &full_region);

    // Postive renderpass multidependency test, will fail IFF the dependencies are acting indepently.
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    m_command_buffer.BeginRenderPass(rp_helper_negative.render_pass_begin);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, RenderPassAsyncHazard) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    // overall set up:
    // subpass 0:
    //   write image 0
    // subpass 1:
    //   read image 0
    //   write image 1
    // subpass 2:
    //   read image 0
    //   write image 2
    // subpass 3:
    //   read image 0
    //   write image 3
    //
    // subpasses 1 & 2 can run in parallel but both should depend on 0
    // subpass 3 must run after 1 & 2 because otherwise the store operation will
    // race with the reads in the other subpasses.

    constexpr VkFormat kFormat = VK_FORMAT_R8G8B8A8_UNORM;
    constexpr uint32_t kWidth = 32, kHeight = 32;
    constexpr uint32_t kNumImages = 4;

    VkImageCreateInfo src_img_info = vku::InitStructHelper();
    src_img_info.flags = 0;
    src_img_info.imageType = VK_IMAGE_TYPE_2D;
    src_img_info.format = kFormat;
    src_img_info.extent = {kWidth, kHeight, 1};
    src_img_info.mipLevels = 1;
    src_img_info.arrayLayers = 1;
    src_img_info.samples = VK_SAMPLE_COUNT_1_BIT;
    src_img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    src_img_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    VkImageCreateInfo dst_img_info = vku::InitStructHelper();
    dst_img_info.flags = 0;
    dst_img_info.imageType = VK_IMAGE_TYPE_2D;
    dst_img_info.format = kFormat;
    dst_img_info.extent = {kWidth, kHeight, 1};
    dst_img_info.mipLevels = 1;
    dst_img_info.arrayLayers = 1;
    dst_img_info.samples = VK_SAMPLE_COUNT_1_BIT;
    dst_img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    dst_img_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    std::vector<std::unique_ptr<vkt::Image>> images;
    images.emplace_back(std::make_unique<vkt::Image>(*m_device, src_img_info));
    for (uint32_t i = 1; i < kNumImages; i++) {
        images.emplace_back(std::make_unique<vkt::Image>(*m_device, dst_img_info));
    }

    vkt::ImageView attachment_wrappers[kNumImages];
    std::array<VkImageView, kNumImages> attachments{};
    std::array<VkAttachmentDescription, kNumImages> attachment_descriptions{};
    std::array<VkAttachmentReference, kNumImages> color_refs{};
    std::array<VkImageMemoryBarrier, kNumImages> img_barriers{};

    for (uint32_t i = 0; i < attachments.size(); i++) {
        attachment_wrappers[i] = images[i]->CreateView();
        attachments[i] = attachment_wrappers[i].handle();
        attachment_descriptions[i] = {};
        attachment_descriptions[i].flags = 0;
        attachment_descriptions[i].format = kFormat;
        attachment_descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_descriptions[i].finalLayout =
            (i == 0) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        color_refs[i] = {i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        img_barriers[i] = vku::InitStructHelper();
        img_barriers[i].srcAccessMask = 0;
        img_barriers[i].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        img_barriers[i].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        img_barriers[i].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_barriers[i].image = images[i]->handle();
        img_barriers[i].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
    }

    const VkAttachmentReference input_ref{0u, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    std::array<std::array<uint32_t, 2>, kNumImages - 1> preserve_subpass{{{2, 3}, {1, 3}, {1, 2}}};

    std::array<VkSubpassDescription, kNumImages> subpasses{};

    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].inputAttachmentCount = 0;
    subpasses[0].pInputAttachments = nullptr;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &color_refs[0];

    for (uint32_t i = 1; i < subpasses.size(); i++) {
        subpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[i].inputAttachmentCount = 1;
        subpasses[i].pInputAttachments = &input_ref;
        subpasses[i].colorAttachmentCount = 1;
        subpasses[i].pColorAttachments = &color_refs[i];
        subpasses[i].preserveAttachmentCount = preserve_subpass[i - 1].size();
        subpasses[i].pPreserveAttachments = preserve_subpass[i - 1].data();
    }

    VkRenderPassCreateInfo renderpass_info = vku::InitStructHelper();
    renderpass_info.flags = 0;
    renderpass_info.attachmentCount = attachment_descriptions.size();
    renderpass_info.pAttachments = attachment_descriptions.data();
    renderpass_info.subpassCount = subpasses.size();
    renderpass_info.pSubpasses = subpasses.data();
    renderpass_info.dependencyCount = 0;
    renderpass_info.pDependencies = nullptr;

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_info);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentSubpassLoadGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkClearValue clear = {};
    clear.color = m_clear_color;
    std::array<VkClearValue, 4> clear_values = {{clear, clear, clear, clear}};

    // run the renderpass with no dependencies
    {
        vkt::RenderPass rp(*m_device, renderpass_info);
        vkt::Framebuffer fb(*m_device, rp.handle(), attachments.size(), attachments.data(), kWidth, kHeight);

        CreatePipelineHelper g_pipe_0(*this);
        g_pipe_0.gp_ci_.renderPass = rp.handle();
        ASSERT_EQ(VK_SUCCESS, g_pipe_0.CreateGraphicsPipeline());

        CreatePipelineHelper g_pipe_12(*this);
        g_pipe_12.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        g_pipe_12.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        g_pipe_12.gp_ci_.renderPass = rp.handle();
        g_pipe_12.gp_ci_.subpass = 1;
        g_pipe_12.LateBindPipelineInfo();

        std::vector<vkt::Pipeline> g_pipes(kNumImages - 1);
        for (size_t i = 0; i < g_pipes.size(); i++) {
            g_pipe_12.gp_ci_.subpass = i + 1;
            g_pipes[i].init(*m_device, g_pipe_12.gp_ci_);
        }

        g_pipe_12.descriptor_set_->WriteDescriptorImageInfo(0, attachments[0], sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        g_pipe_12.descriptor_set_->UpdateDescriptorSets();

        m_command_buffer.Begin();

        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, img_barriers.size(),
                               img_barriers.data());

        m_renderPassBeginInfo.renderArea = {{0, 0}, {16, 16}};
        m_renderPassBeginInfo.pClearValues = clear_values.data();
        m_renderPassBeginInfo.clearValueCount = clear_values.size();

        m_renderPassBeginInfo.renderArea = {{0, 0}, {kWidth, kHeight}};
        m_renderPassBeginInfo.renderPass = rp.handle();
        m_renderPassBeginInfo.framebuffer = fb.handle();

        // Test is intentionally running without dependencies.
        vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.pipeline_layout_.handle(), 0,
                                  1, &g_pipe_0.descriptor_set_->set_, 0, NULL);

        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

        for (uint32_t i = 1; i < subpasses.size(); i++) {
            // we're racing the writes from subpass 0 with our layout transitions... (from initial)
            m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
            m_command_buffer.NextSubpass();
            m_errorMonitor->VerifyFound();
        }

        // Suppress core validation that render pass ends before reaching final subpass.
        // NextSubpass does not update current subpass (Record is skipped) due to syncval error.
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndRenderPass-None-00910");

        // m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
        // No sync error here, as all of the NextSubpass calls *failed*
        m_command_buffer.EndRenderPass();
        // m_errorMonitor->VerifyFound();

        vk::ResetCommandPool(device(), m_command_pool.handle(), 0);
    }

    // add dependencies from subpass 0 to the others, which are necessary but not sufficient
    std::vector<VkSubpassDependency> subpass_dependencies;
    for (uint32_t i = 1; i < subpasses.size(); i++) {
        VkSubpassDependency dep{0,
                                i,
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
                                0};
        subpass_dependencies.push_back(dep);
    }
    renderpass_info.dependencyCount = subpass_dependencies.size();
    renderpass_info.pDependencies = subpass_dependencies.data();

    {
        vkt::RenderPass rp(*m_device, renderpass_info);
        vkt::Framebuffer fb(*m_device, rp.handle(), attachments.size(), attachments.data(), kWidth, kHeight);

        CreatePipelineHelper g_pipe_0(*this);
        g_pipe_0.gp_ci_.renderPass = rp.handle();
        ASSERT_EQ(VK_SUCCESS, g_pipe_0.CreateGraphicsPipeline());

        CreatePipelineHelper g_pipe_12(*this);
        g_pipe_12.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        g_pipe_12.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        g_pipe_12.gp_ci_.renderPass = rp.handle();
        g_pipe_12.gp_ci_.subpass = 1;
        g_pipe_12.LateBindPipelineInfo();

        std::vector<vkt::Pipeline> g_pipes(kNumImages - 1);
        for (size_t i = 0; i < g_pipes.size(); i++) {
            g_pipe_12.gp_ci_.subpass = i + 1;
            g_pipes[i].init(*m_device, g_pipe_12.gp_ci_);
        }

        g_pipe_12.descriptor_set_->WriteDescriptorImageInfo(0, attachments[0], sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        g_pipe_12.descriptor_set_->UpdateDescriptorSets();

        m_command_buffer.Begin();

        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, img_barriers.size(),
                               img_barriers.data());

        m_renderPassBeginInfo.renderArea = {{0, 0}, {16, 16}};
        m_renderPassBeginInfo.pClearValues = clear_values.data();
        m_renderPassBeginInfo.clearValueCount = clear_values.size();

        m_renderPassBeginInfo.renderArea = {{0, 0}, {kWidth, kHeight}};
        m_renderPassBeginInfo.renderPass = rp.handle();
        m_renderPassBeginInfo.framebuffer = fb.handle();

        vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.pipeline_layout_.handle(), 0,
                                  1, &g_pipe_0.descriptor_set_->set_, 0, NULL);

        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

        for (uint32_t i = 1; i < subpasses.size(); i++) {
            if (i > 1) {
                // We've fixed the dependency with 0, but 2 and 3 still fight with 1
                m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
            }
            m_command_buffer.NextSubpass();
            if (i > 1) {
                m_errorMonitor->VerifyFound();

                // Suppress core validation that current subpass index should match pipeline's subpass.
                // NextSubpass does not update current subpass (Record is skipped) due to syncval error.
                m_errorMonitor->SetUnexpectedError("VUID-vkCmdDraw-subpass-02685");
            }

            vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipes[i - 1].handle());
            vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      g_pipe_12.pipeline_layout_.handle(), 0, 1, &g_pipe_12.descriptor_set_->set_, 0, NULL);

            vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
        }

        // Suppress core validation that render pass ends before reaching final subpass.
        // NextSubpass does not update current subpass (Record is skipped) due to syncval error.
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndRenderPass-None-00910");

        // There is no race, because the NextSubpass calls failed above
        m_command_buffer.EndRenderPass();

        vk::ResetCommandPool(device(), m_command_pool.handle(), 0);
    }

    // try again with correct dependencies to make subpasses:
    //   2 depend on 1 (avoid ILT hazard)
    subpass_dependencies.emplace_back(
        VkSubpassDependency{1, 2, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, 0});
    //   3 depend on 2 (avoid store hazard)
    subpass_dependencies.emplace_back(
        VkSubpassDependency{2, 3, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0});

    renderpass_info.dependencyCount = subpass_dependencies.size();
    renderpass_info.pDependencies = subpass_dependencies.data();
    {
        vkt::RenderPass rp(*m_device, renderpass_info);
        vkt::Framebuffer fb(*m_device, rp.handle(), attachments.size(), attachments.data(), kWidth, kHeight);

        CreatePipelineHelper g_pipe_0(*this);
        g_pipe_0.gp_ci_.renderPass = rp.handle();
        ASSERT_EQ(VK_SUCCESS, g_pipe_0.CreateGraphicsPipeline());

        CreatePipelineHelper g_pipe_12(*this);
        g_pipe_12.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        g_pipe_12.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        g_pipe_12.gp_ci_.renderPass = rp.handle();
        g_pipe_12.gp_ci_.subpass = 1;
        g_pipe_12.LateBindPipelineInfo();

        std::vector<vkt::Pipeline> g_pipes(kNumImages - 1);
        for (size_t i = 0; i < g_pipes.size(); i++) {
            g_pipe_12.gp_ci_.subpass = i + 1;
            g_pipes[i].init(*m_device, g_pipe_12.gp_ci_);
        }

        g_pipe_12.descriptor_set_->WriteDescriptorImageInfo(0, attachments[0], sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        g_pipe_12.descriptor_set_->UpdateDescriptorSets();

        m_command_buffer.Begin();
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, img_barriers.size(),
                               img_barriers.data());

        m_renderPassBeginInfo.renderArea = {{0, 0}, {16, 16}};
        m_renderPassBeginInfo.pClearValues = clear_values.data();
        m_renderPassBeginInfo.clearValueCount = clear_values.size();

        m_renderPassBeginInfo.renderArea = {{0, 0}, {kWidth, kHeight}};
        m_renderPassBeginInfo.renderPass = rp.handle();
        m_renderPassBeginInfo.framebuffer = fb.handle();

        vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_0.pipeline_layout_.handle(), 0,
                                  1, &g_pipe_0.descriptor_set_->set_, 0, NULL);

        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

        for (uint32_t i = 1; i < subpasses.size(); i++) {
            m_command_buffer.NextSubpass();
            vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipes[i - 1].handle());
            vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      g_pipe_12.pipeline_layout_.handle(), 0, 1, &g_pipe_12.descriptor_set_->set_, 0, NULL);

            vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
        }

        m_command_buffer.EndRenderPass();

        m_command_buffer.End();
    }
}

TEST_F(NegativeSyncVal, EventsBufferCopy) {
    TEST_DESCRIPTION("Check Set/Wait protection for a variety of use cases using buffer copies");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_c(*m_device, 256, transfer_usage, mem_prop);

    VkBufferCopy region = {0, 0, 256};
    VkBufferCopy front2front = {0, 0, 128};
    VkBufferCopy front2back = {0, 128, 128};
    VkBufferCopy back2back = {128, 128, 128};

    vkt::Event event(*m_device);
    VkEvent event_handle = event.handle();

    auto cb = m_command_buffer.handle();
    m_command_buffer.Begin();

    // Copy after set for WAR (note we are writing to the back half of c but only reading from the front
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_c.handle(), 1, &back2back);
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2front);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2back);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // WAR prevented
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    // Just protect against WAR, only need a sync barrier.
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &region);

    // Wait shouldn't prevent this WAW though, as it's only a synchronization barrier
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_b.handle(), 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Prevent WAR and WAW
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    VkMemoryBarrier mem_barrier_waw = vku::InitStructHelper();
    mem_barrier_waw.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier_waw.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 1,
                                &mem_barrier_waw, 0, nullptr, 0, nullptr);
    // The WAW should be safe (on a memory barrier)
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_b.handle(), 1, &region);
    // The WAR should also be safe (on a sync barrier)
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &region);
    m_command_buffer.End();

    // Barrier range check for WAW
    VkBufferMemoryBarrier buffer_barrier_front_waw = vku::InitStructHelper();
    buffer_barrier_front_waw.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier_front_waw.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier_front_waw.buffer = buffer_b.handle();
    buffer_barrier_front_waw.offset = front2front.dstOffset;
    buffer_barrier_front_waw.size = front2front.size;

    // Front safe, back WAW
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 1,
                                &buffer_barrier_front_waw, 0, nullptr);
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &front2front);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &back2back);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, EventsCopyImageHazards) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, format, usage);
    vkt::Image image_a(*m_device, image_ci, vkt::set_layout);
    vkt::Image image_b(*m_device, image_ci, vkt::set_layout);
    vkt::Image image_c(*m_device, image_ci, vkt::set_layout);

    vkt::Event event(*m_device);
    VkEvent event_handle = event.handle();

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 2};
    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkImageSubresourceRange layers_0_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{64, 64, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};    // <-- image type is 2D

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};
    VkImageCopy region_0_to_0 = {layers_0, zero_offset, layers_0, zero_offset, full_extent};
    VkImageCopy region_1_to_1 = {layers_1, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_0_q0toq0 = {layers_0, zero_offset, layers_0, zero_offset, half_extent};
    VkImageCopy region_0_q0toq3 = {layers_0, zero_offset, layers_0, half_offset, half_extent};
    VkImageCopy region_0_q3toq3 = {layers_0, half_offset, layers_0, half_offset, half_extent};

    auto cb = m_command_buffer.handle();
    auto copy_general = [cb](const vkt::Image& from, const vkt::Image& to, const VkImageCopy& region) {
        vk::CmdCopyImage(cb, from.handle(), VK_IMAGE_LAYOUT_GENERAL, to.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    };

    auto set_layouts = [this, &image_a, &image_b, &image_c]() {
        image_c.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
        image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
        image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    };

    // Scope check.  One access in, one access not
    m_command_buffer.Begin();
    set_layouts();
    copy_general(image_a, image_b, full_region);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    copy_general(image_a, image_c, region_0_q3toq3);
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    copy_general(image_c, image_a, region_0_q0toq0);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    copy_general(image_c, image_a, region_0_q0toq3);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // WAR prevented
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    set_layouts();
    copy_general(image_a, image_b, full_region);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    // Just protect against WAR, only need a sync barrier.
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    copy_general(image_c, image_a, full_region);

    // Wait shouldn't prevent this WAW though, as it's only a synchronization barrier
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    copy_general(image_c, image_b, full_region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // Prevent WAR and WAW
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    set_layouts();
    copy_general(image_a, image_b, full_region);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    VkMemoryBarrier mem_barrier_waw = vku::InitStructHelper();
    mem_barrier_waw.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier_waw.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 1,
                                &mem_barrier_waw, 0, nullptr, 0, nullptr);
    // The WAW should be safe (on a memory barrier)
    copy_general(image_c, image_b, full_region);
    // The WAR should also be safe (on a sync barrier)
    copy_general(image_c, image_a, full_region);
    m_command_buffer.End();

    // Barrier range check for WAW
    VkImageMemoryBarrier image_barrier_region0_waw = vku::InitStructHelper();
    image_barrier_region0_waw.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier_region0_waw.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier_region0_waw.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier_region0_waw.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier_region0_waw.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier_region0_waw.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier_region0_waw.image = image_b.handle();
    image_barrier_region0_waw.subresourceRange = layers_0_subresource_range;

    // Region 0 safe, back WAW
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    set_layouts();
    copy_general(image_a, image_b, full_region);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 1, &image_barrier_region0_waw);
    copy_general(image_a, image_b, region_0_to_0);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    copy_general(image_a, image_b, region_1_to_1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, EventsCommandHazards) {
    TEST_DESCRIPTION("Check Set/Reset/Wait command hazard checking");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Event event(*m_device);
    const VkEvent event_handle = event.handle();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);

    m_errorMonitor->SetDesiredError("VUID-vkCmdResetEvent-event-03834");
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, nullptr,
                                0, nullptr, 0, nullptr);
    m_errorMonitor->SetDesiredError("SYNC-vkCmdResetEvent-missingbarrier-wait");
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    m_command_buffer.Begin();
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->SetDesiredError("SYNC-vkCmdSetEvent-missingbarrier-reset");
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0U, 0,
                           nullptr, 0, nullptr, 0, nullptr);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_command_buffer.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0,
                                nullptr, 0, nullptr);
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0U, 0,
                           nullptr, 0, nullptr, 0, nullptr);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);

    // Need a barrier between set and a reset
    m_errorMonitor->SetDesiredError("SYNC-vkCmdResetEvent-missingbarrier-set");
    m_command_buffer.ResetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->SetDesiredError("SYNC-vkCmdSetEvent-missingbarrier-set");
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();

    // Secondary command buffer events tests
    const auto cb = m_command_buffer.handle();
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, transfer_usage, mem_prop);
    vkt::Buffer buffer_b(*m_device, 256, transfer_usage, mem_prop);

    VkBufferCopy front2front = {0, 0, 128};

    // Barrier range check for WAW
    VkBufferMemoryBarrier buffer_barrier_front_waw = vku::InitStructHelper();
    buffer_barrier_front_waw.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier_front_waw.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier_front_waw.buffer = buffer_b.handle();
    buffer_barrier_front_waw.offset = front2front.dstOffset;
    buffer_barrier_front_waw.size = front2front.size;

    vkt::CommandBuffer secondary_cb1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer scb1 = secondary_cb1.handle();
    secondary_cb1.Begin();
    secondary_cb1.WaitEvents(1, &event_handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 1,
                             &buffer_barrier_front_waw, 0, nullptr);
    vk::CmdCopyBuffer(scb1, buffer_a.handle(), buffer_b.handle(), 1, &front2front);
    secondary_cb1.End();

    // One secondary cb hazarding with primary
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &front2front);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdExecuteCommands(cb, 1, &scb1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // One secondary cb sharing event with primary
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &front2front);
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vk::CmdExecuteCommands(cb, 1, &scb1);
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, DestroyedUnusedDescriptors) {
    TEST_DESCRIPTION("Verify unused descriptors are ignored and don't crash syncval if they've been destroyed.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::descriptorBindingUpdateUnusedWhilePending);

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    InitRenderTarget();

    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    constexpr size_t kNumDescriptors = 6;

    std::array<VkDescriptorBindingFlags, kNumDescriptors> ds_binding_flags;
    for (auto &elem : ds_binding_flags) {
        elem = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    }

    layout_createinfo_binding_flags.bindingCount = ds_binding_flags.size();
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags.data();

    // Prepare descriptors
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {4, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    const uint32_t zeroit_value = 0;
    vkt::Buffer doit_buffer(*m_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &zeroit_value, sizeof(uint32_t));

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 32;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    auto buffer = std::make_unique<vkt::Buffer>();
    buffer->init(*m_device, buffer_create_info);

    VkDescriptorBufferInfo buffer_info[2] = {};
    buffer_info[0].buffer = doit_buffer.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);
    buffer_info[1].buffer = buffer->handle();
    buffer_info[1].offset = 0;
    buffer_info[1].range = sizeof(uint32_t);

    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    vkt::Buffer texel_buffer(*m_device, buffer_create_info);

    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.offset = 0;
    bvci.range = VK_WHOLE_SIZE;

    auto texel_bufferview = std::make_unique<vkt::BufferView>();
    texel_bufferview->init(*m_device, bvci);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image sampled_image(*m_device, image_ci, vkt::set_layout);
    auto sampled_view = std::make_unique<vkt::ImageView>();
    auto imageview_ci = sampled_image.BasicViewCreatInfo();
    sampled_view->init(*m_device, imageview_ci);

    image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image combined_image(*m_device, image_ci, vkt::set_layout);
    imageview_ci = combined_image.BasicViewCreatInfo();
    auto combined_view = std::make_unique<vkt::ImageView>();
    combined_view->init(*m_device, imageview_ci);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorImageInfo image_info[3] = {};
    image_info[0].sampler = sampler.handle();
    image_info[0].imageView = VK_NULL_HANDLE;
    image_info[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info[1].sampler = VK_NULL_HANDLE;
    image_info[1].imageView = sampled_view->handle();
    image_info[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info[2].sampler = sampler.handle();
    image_info[2].imageView = combined_view->handle();
    image_info[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Update all descriptors
    std::array<VkWriteDescriptorSet, kNumDescriptors> descriptor_writes;
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = &buffer_info[0];

    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info[1];

    descriptor_writes[2] = vku::InitStructHelper();
    descriptor_writes[2].dstSet = descriptor_set.set_;
    descriptor_writes[2].dstBinding = 2;
    descriptor_writes[2].descriptorCount = 1;
    descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    descriptor_writes[2].pTexelBufferView = &texel_bufferview->handle();

    descriptor_writes[3] = vku::InitStructHelper();
    descriptor_writes[3].dstSet = descriptor_set.set_;
    descriptor_writes[3].dstBinding = 3;
    descriptor_writes[3].descriptorCount = 1;
    descriptor_writes[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptor_writes[3].pImageInfo = &image_info[0];

    descriptor_writes[4] = vku::InitStructHelper();
    descriptor_writes[4].dstSet = descriptor_set.set_;
    descriptor_writes[4].dstBinding = 4;
    descriptor_writes[4].descriptorCount = 1;
    descriptor_writes[4].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_writes[4].pImageInfo = &image_info[1];

    descriptor_writes[5] = vku::InitStructHelper();
    descriptor_writes[5].dstSet = descriptor_set.set_;
    descriptor_writes[5].dstBinding = 5;
    descriptor_writes[5].descriptorCount = 1;
    descriptor_writes[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[5].pImageInfo = &image_info[2];

    vk::UpdateDescriptorSets(device(), descriptor_writes.size(), descriptor_writes.data(), 0, NULL);

    // only descriptor 0 is used, the rest are going to get destroyed
    char const* shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform foo_0 { int val; } doit;
        layout(set = 0, binding = 1) uniform foo_1 { int val; } readit;
        layout(set = 0, binding = 2) uniform samplerBuffer texels;
        layout(set = 0, binding = 3) uniform sampler samp;
        layout(set = 0, binding = 4) uniform texture2D img;
        layout(set = 0, binding = 5) uniform sampler2D sampled_image;

        void main() {
            vec4 x;
            vec4 y;
            vec4 z;
            if (doit.val == 0) {
                gl_Position = vec4(0.0);
                x = vec4(0.0);
                y = vec4(0.0);
                z = vec4(0.0);
            } else {
                gl_Position = vec4(readit.val);
                x = texelFetch(texels, 5);
                y = texture(sampler2D(img, samp), vec2(0));
                z = texture(sampled_image, vec2(0));
        }
        }
    )glsl";

    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    // destroy resources for the unused descriptors
    buffer.reset();
    texel_bufferview.reset();
    sampled_view.reset();
    combined_view.reset();

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexed(m_command_buffer.handle(), 1, 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, TestInvalidExternalSubpassDependency) {
    TEST_DESCRIPTION("Test write after write hazard with invalid external subpass dependency");

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkSubpassDependency subpass_dependency = {};
    subpass_dependency.srcSubpass = 0;
    subpass_dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = 0;
    subpass_dependency.dependencyFlags = 0;

    VkAttachmentReference attach_ref1 = {};
    attach_ref1.attachment = 0;
    attach_ref1.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference attach_ref2 = {};
    attach_ref2.attachment = 0;
    attach_ref2.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkSubpassDescription subpass_descriptions[2] = {};
    subpass_descriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_descriptions[0].pDepthStencilAttachment = &attach_ref1;
    subpass_descriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_descriptions[1].pDepthStencilAttachment = &attach_ref2;

    VkAttachmentDescription attachment_description = {};
    attachment_description.format = VK_FORMAT_D32_SFLOAT;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = subpass_descriptions;
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment_description;
    rp_ci.dependencyCount = 1;
    rp_ci.pDependencies = &subpass_dependency;

    vkt::RenderPass render_pass(*m_device, rp_ci);

    VkClearValue clear_value = {};
    clear_value.color = {{0, 0, 0, 0}};

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_D32_SFLOAT;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view1 = image1.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    VkImageView framebuffer_attachments[1] = {image_view1.handle()};

    vkt::Framebuffer framebuffer(*m_device, render_pass.handle(), 1, framebuffer_attachments);

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.depthTestEnable = VK_FALSE;
    ds_ci.depthWriteEnable = VK_FALSE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.gp_ci_.pDepthStencilState = &ds_ci;
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass.handle(), framebuffer.handle(), 32, 32, 1, &clear_value);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncVal, CopyToCompressedImage) {
    TEST_DESCRIPTION("Copy from uncompressed to compressed image with and without overlap.");

    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    const bool copy_commands_2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    VkFormatProperties format_properties;
    VkFormat mp_format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), mp_format, &format_properties);
    if ((format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0) {
        GTEST_SKIP()
            << "Device does not support VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT for VK_FORMAT_BC1_RGBA_UNORM_BLOCK, skipping test.\n";
    }

    vkt::Image src_image(*m_device, 1, 1, 1, VK_FORMAT_R32G32_UINT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image dst_image(*m_device, 12, 4, 1, VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkImageCopy copy_regions[2] = {};
    copy_regions[0].srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_regions[0].srcOffset = {0, 0, 0};
    copy_regions[0].dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_regions[0].dstOffset = {0, 0, 0};
    copy_regions[0].extent = {1, 1, 1};
    copy_regions[1].srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_regions[1].srcOffset = {0, 0, 0};
    copy_regions[1].dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_regions[1].dstOffset = {4, 0, 0};
    copy_regions[1].extent = {1, 1, 1};

    m_command_buffer.Begin();

    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_regions[0]);
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_regions[1]);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    copy_regions[1].dstOffset = {4, 0, 0};
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_regions[1]);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();

    if (copy_commands_2) {
        m_command_buffer.Reset();

        VkImageCopy2 copy_regions2[2];
        copy_regions2[0] = vku::InitStructHelper();
        copy_regions2[0].srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copy_regions2[0].srcOffset = {0, 0, 0};
        copy_regions2[0].dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copy_regions2[0].dstOffset = {0, 0, 0};
        copy_regions2[0].extent = {1, 1, 1};
        copy_regions2[1] = vku::InitStructHelper();
        copy_regions2[1].srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copy_regions2[1].srcOffset = {0, 0, 0};
        copy_regions2[1].dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copy_regions2[1].dstOffset = {4, 0, 0};
        copy_regions2[1].extent = {1, 1, 1};

        VkCopyImageInfo2 copy_image_info = vku::InitStructHelper();
        copy_image_info.srcImage = src_image.handle();
        copy_image_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        copy_image_info.dstImage = dst_image.handle();
        copy_image_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        copy_image_info.regionCount = 2;
        copy_image_info.pRegions = copy_regions2;

        m_command_buffer.Begin();

        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info);
        m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
        copy_image_info.regionCount = 1;
        copy_image_info.pRegions = &copy_regions2[1];
        copy_regions[1].dstOffset = {7, 0, 0};
        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info);
        m_errorMonitor->VerifyFound();

        m_command_buffer.End();
    }
}

TEST_F(NegativeSyncVal, StageAccessExpansion) {
    SetTargetApiVersion(VK_API_VERSION_1_2);

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkImageUsageFlags image_usage_combine = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const auto image_c_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_combine);
    vkt::Image image_c_a(*m_device, image_c_ci, vkt::set_layout);
    vkt::Image image_c_b(*m_device, image_c_ci, vkt::set_layout);

    vkt::ImageView imageview_c = image_c_a.CreateView();
    VkImageUsageFlags image_usage_storage =
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    const auto image_s_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_storage);
    vkt::Image image_s_a(*m_device, image_s_ci, vkt::set_layout);
    vkt::Image image_s_b(*m_device, image_s_ci, vkt::set_layout);
    image_s_a.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_s_b.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView imageview_s = image_s_a.CreateView();

    vkt::Sampler sampler_s, sampler_c;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_s.init(*m_device, sampler_ci);
    sampler_c.init(*m_device, sampler_ci);

    vkt::Buffer buffer_a, buffer_b;
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_a.init(*m_device, buffer_a.CreateInfo(2048, buffer_usage), mem_prop);
    buffer_b.init(*m_device, buffer_b.CreateInfo(2048, buffer_usage), mem_prop);

    vkt::BufferView bufferview;
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = buffer_a.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.offset = 0;
    bvci.range = VK_WHOLE_SIZE;

    bufferview.init(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a.handle(), 0, 2048);
    descriptor_set.WriteDescriptorImageInfo(1, imageview_c, sampler_c.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorImageInfo(2, imageview_s, sampler_s.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorBufferView(3, bufferview.handle());
    descriptor_set.UpdateDescriptorSets();

    // Dispatch
    std::string csSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform foo { float x; } ub0;
        layout(set=0, binding=1) uniform sampler2D cis1;
        layout(set=0, binding=2, rgba8) uniform readonly image2D si2;
        layout(set=0, binding=3, r32f) uniform readonly imageBuffer stb3;
        void main(){
            vec4 vColor4;
            vColor4.x = ub0.x;
            vColor4 = texture(cis1, vec2(0));
            vColor4 = imageLoad(si2, ivec2(0));
            vColor4 = imageLoad(stb3, 0);
        }
    )glsl";

    // Draw
    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkVertexInputAttributeDescription VertexInputAttributeDescription = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vbo_data)};
    VkVertexInputBindingDescription VertexInputBindingDescription = {0, sizeof(vbo_data), VK_VERTEX_INPUT_RATE_VERTEX};
    vkt::Buffer vbo, vbo2;
    buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vbo.init(*m_device, vbo.CreateInfo(sizeof(vbo_data), buffer_usage), mem_prop);
    vbo2.init(*m_device, vbo2.CreateInfo(sizeof(vbo_data), buffer_usage), mem_prop);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, csSource.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.vi_ci_.pVertexBindingDescriptions = &VertexInputBindingDescription;
    g_pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    g_pipe.vi_ci_.pVertexAttributeDescriptions = &VertexInputAttributeDescription;
    g_pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    g_pipe.CreateGraphicsPipeline();

    m_command_buffer.Reset();
    m_command_buffer.Begin();
    VkImageSubresourceLayers layer{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{16, 16, 1};
    VkImageCopy image_region = {layer, zero_offset, layer, zero_offset, full_extent};
    vk::CmdCopyImage(m_command_buffer.handle(), image_c_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);
    vk::CmdCopyImage(m_command_buffer.handle(), image_s_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_s_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);

    VkMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    // wrong: dst stage should be VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1,
                           &barrier, 0, nullptr, 0, nullptr);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    // one error for each image copied above
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Try again with the correct dst stage on the barrier
    m_command_buffer.Reset();
    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer.handle(), image_c_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);
    vk::CmdCopyImage(m_command_buffer.handle(), image_s_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_s_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1,
                           &barrier, 0, nullptr, 0, nullptr);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, QSBufferCopyHazards) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device, m_device->QueuesWithGraphicsCapability()[0]);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires a valid queue object.";
    }

    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);
    test.RecordCopy(test.cbb, test.buffer_c, test.buffer_a);

    VkSubmitInfo submit1 = vku::InitStructHelper();
    submit1.commandBufferCount = 2;
    VkCommandBuffer two_cbs[2] = {test.h_cba, test.h_cbb};
    submit1.pCommandBuffers = two_cbs;

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::QueueSubmit(test.q0, 1, &submit1, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    test.DeviceWait();

    VkSubmitInfo submit2[2] = {vku::InitStruct<VkSubmitInfo>(), vku::InitStruct<VkSubmitInfo>()};
    submit2[0].commandBufferCount = 1;
    submit2[0].pCommandBuffers = &test.h_cba;
    submit2[1].commandBufferCount = 1;
    submit2[1].pCommandBuffers = &test.h_cbb;
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    vk::QueueSubmit(test.q0, 2, submit2, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // With the skip settings, the above QueueSubmit's didn't record, so we can treat the global queue contexts as empty
    test.Submit0(test.cba);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit0(test.cbb);
    m_errorMonitor->VerifyFound();

    test.DeviceWait();

    // A little grey box testing to ensure the trim code is referenced
    test.BeginA();
    test.Copy(test.buffer_a, test.buffer_c, test.first_half);
    test.Copy(test.buffer_a, test.buffer_c, test.second_half);
    test.End();
    test.Submit0(test.cba);
    test.BeginB();
    test.TransferBarrierWAR(test.buffer_a);
    test.Copy(test.buffer_b, test.buffer_a);
    test.TransferBarrierRAW(test.buffer_c);
    test.TransferBarrierWAR(test.buffer_b);
    test.Copy(test.buffer_c, test.buffer_b);
    test.End();
    test.Submit0(test.cbb);

    test.DeviceWait();
}

TEST_F(NegativeSyncVal, QSSubmit2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device, m_device->QueuesWithGraphicsCapability()[0]);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires a valid queue object.";
    }

    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);
    test.RecordCopy(test.cbb, test.buffer_c, test.buffer_a);

    // Test that the signal mask is controlling the first scope
    test.SubmitX(test.q0, test.cba, VK_NULL_HANDLE, 0, test.semaphore.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit0(test.cbb);
    m_errorMonitor->VerifyFound();

    // Since the last submit skipped, we need a wait that will success
    test.BeginC();
    test.End();
    test.Submit0Wait(test.cbc, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    test.DeviceWait();

    // This time with the correct first sync scope.
    test.SubmitX(test.q0, test.cba, VK_NULL_HANDLE, 0, test.semaphore.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.Submit0Wait(test.cbb, VK_PIPELINE_STAGE_TRANSFER_BIT);

    test.DeviceWait();
}

TEST_F(NegativeSyncVal, QSBufferCopyVsIdle) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device, m_device->QueuesWithGraphicsCapability()[0]);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires a valid queue object.";
    }

    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);
    test.RecordCopy(test.cbb, test.buffer_c, test.buffer_a);

    // Submit A
    test.Submit0(test.cba);

    // Submit B which hazards vs. A
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit0(test.cbb);
    m_errorMonitor->VerifyFound();

    // With the skip settings, the above QueueSubmit's didn't record, so we can treat the previous submit as not
    // having happened. So we'll try again with a device wait idle
    // Submit B again, but after idling, which should remove the hazard
    test.DeviceWait();
    test.Submit0(test.cbb);

    // Submit the same command again for another hazard
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    test.Submit0(test.cbb);
    m_errorMonitor->VerifyFound();

    // With the skip settings, the above QueueSubmit's didn't record, so we can treat the previous submit as not
    // having happened. So we'll try again with a queue wait idle
    // Submit B again, but after idling, which should remove the hazard
    test.QueueWait0();
    test.Submit0(test.cbb);

    m_device->Wait();
}

TEST_F(NegativeSyncVal, QSBufferCopyVsFence) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device, m_device->QueuesWithGraphicsCapability()[0]);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires a valid queue object.";
    }

    vkt::Fence fence(*m_device);
    VkResult wait_result;
    vkt::CommandBuffer cbd;
    test.InitFromPool(cbd);

    // Set up four CB with copy commands
    // We'll wait for the first, but not the second
    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);
    test.RecordCopy(test.cbb, test.buffer_a, test.buffer_c);
    test.RecordCopy(test.cbc, test.buffer_a, test.buffer_b);

    // This is the one that should error
    test.RecordCopy(cbd, test.buffer_a, test.buffer_c);

    // Two copies *better* finish in a second...
    const uint64_t kFourSeconds = 1U << 30;
    // Copy A to B
    test.Submit0(test.cba, VK_NULL_HANDLE, 0U, VK_NULL_HANDLE, fence.handle());
    // Copy A to C
    test.Submit0(test.cbb);
    // Wait for A to B
    wait_result = fence.Wait(kFourSeconds);

    if (wait_result != VK_SUCCESS) {
        ADD_FAILURE() << "Fence wait failed. Aborting test.";
        m_device->Wait();
    }

    // A and B should be good to go...
    test.Submit0(test.cbc);

    // But C shouldn't
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    test.Submit0(cbd);
    m_errorMonitor->VerifyFound();

    test.DeviceWait();
}

TEST_F(NegativeSyncVal, QSBufferCopyQSORules) {
    all_queue_count_ = true;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires at least 2 TRANSFER capable queues in the same queue_family";
    }

    // Need an extra buffer and CB
    vkt::Buffer buffer_d(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkt::CommandBuffer cbd;
    test.InitFromPool(cbd);
    // This gives a noop command buffer w.r.t. buffers a, b, and c.
    test.RecordCopy(cbd, buffer_d, buffer_d, test.first_to_second);

    // Command Buffer A reads froms buffer A and writes to buffer B
    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);

    // Command Buffer B reads froms buffer C and writes to buffer A, but has a barrier to protect the write to A when
    // executed on the same queue, given that commands in "queue submission order" are within the first scope of the barrier.
    test.BeginB();

    // Use the barrier to clean up the WAR, which will work for command buffers ealier in queue submission order, or with
    // correct semaphore operations between queues.
    test.TransferBarrierWAR(test.buffer_a);
    test.CopyCToA();
    test.End();

    // Command Buffer C does the same copy as B but without the barrier.
    test.RecordCopy(test.cbc, test.buffer_c, test.buffer_a);

    // Submit A and B on the same queue, to assure us the barrier *would* be sufficient given QSO
    // This is included in a "Sucess" section, just to verify CBA and CBB are set up correctly.
    test.Submit0(test.cba);
    test.Submit0(test.cbb);
    m_device->Wait();  // DeviceWaitIdle, clearing the field for the next subcase

    // Submit A and B on the different queues. Since no semaphore is used between the queues, CB B hazards asynchronously with,
    // CB A with A being read and written on independent queues.
    test.Submit0(test.cba);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-READ");
    test.Submit1(test.cbb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();  // DeviceWaitIdle, clearing the field for the next subcase

    // Test full async detection
    test.Submit0(test.cba);
    test.Submit0(cbd);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-READ");
    test.Submit1(test.cbb);
    m_errorMonitor->VerifyFound();

    // Set up the semaphore for the next two cases

    m_device->Wait();

    // Submit A and B on the different queues, with an ineffectual semaphore.  The wait mask is empty, thus nothing in CB B is in
    // the second excution scope of the waited signal.
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit0Signal(test.cba);
    test.Submit1Wait(test.cbb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);  // wait mask is BOTTOM, s.t. this is a wait-for-nothing.
    m_errorMonitor->VerifyFound();

    // The since second submit failed, it was skipped. So we can try again, without having to WaitDeviceIdle
    // Include transfers in the second execution scope of the waited signal, s.t. the PipelineBarrier in CB B can chain with it.
    test.Submit1Wait(test.cbb, VK_PIPELINE_STAGE_TRANSFER_BIT);  //

    m_device->Wait();

    // Draw A and then C to verify the second access scope of the signal
    test.Submit0Signal(test.cba);
    test.Submit1Wait(test.cbc, VK_PIPELINE_STAGE_TRANSFER_BIT);

    m_device->Wait();

    //  ... and again on the same queue
    test.Submit0Signal(test.cba);
    test.Submit0Wait(test.cbc, VK_PIPELINE_STAGE_TRANSFER_BIT);

    m_device->Wait();
}

TEST_F(NegativeSyncVal, QSBufferEvents) {
    all_queue_count_ = true;
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires at least 2 TRANSFER capable queues in the same queue_family";
    }

    // Command Buffer A reads froms buffer A and writes to buffer B
    test.BeginA();
    test.CopyAToB();
    test.SetEvent(VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.End();

    // Command Buffer B reads froms buffer C and writes to buffer A, but has a wait to protect the write to A when
    // executed on the same queue, given that commands in "queue submission order" are within the first scope of the barrier.
    test.BeginB();

    // Use the barrier to clean up the WAR, which will work for command buffers ealier in queue submission order, or with
    // correct semaphore operations between queues.
    test.WaitEventBufferTransfer(test.buffer_a, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.CopyCToA();
    test.End();

    // Command Buffer C merges the operations from A and B, to ensure the set/wait is correct.
    //    reads froms buffer A and writes to buffer B
    //    reads froms buffer C and writes to buffer A, but has a barrier to protect the write to A when
    test.BeginC();
    test.CopyAToB();
    test.SetEvent(VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.WaitEventBufferTransfer(test.buffer_a, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.CopyCToA();
    test.End();

    // Ensure this would work on one queue (sanity check)
    vkt::CommandBuffer reset(*test.dev, test.pool);
    test.Begin(reset);
    test.ResetEvent(VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.End();

    // Reset the event s.t. I reuse it
    test.Submit0(reset);
    m_device->Wait();

    test.Submit0(test.cba);
    test.Submit0(test.cbb);

    // Ensure that the wait doesn't apply to async queues
    test.Submit0(reset);
    m_device->Wait();

    test.Submit0(test.cba);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-READ");
    // Suppress submit time validation about inter-queue event usage.
    // TODO: update test to avoid inter-queue event usage.
    m_errorMonitor->SetUnexpectedError("UNASSIGNED-SubmitValidation-WaitEvents-WrongQueue");
    test.Submit1(test.cbb);
    m_errorMonitor->VerifyFound();

    // Ensure that the wait doesn't apply to access on other synchronized queues
    m_device->Wait();

    test.Submit0Signal(test.cba);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    // Suppress submit time validation about inter-queue event usage.
    // TODO: update test to avoid inter-queue event usage.
    m_errorMonitor->SetUnexpectedError("UNASSIGNED-SubmitValidation-WaitEvents-WrongQueue");
    test.Submit1Wait(test.cbb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_errorMonitor->VerifyFound();

    // Need to have a successful signal wait to get the semaphore in a usuable state.
    test.BeginC();
    test.End();
    test.Submit1Wait(test.cbc, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_device->Wait();

    // Next ensure that accesses from other queues aren't included in the first scope
    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);

    test.BeginB();
    test.SetEvent(VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.WaitEventBufferTransfer(test.buffer_a, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    test.CopyCToA();
    test.End();

    // Sanity check that same queue works
    test.Submit0(reset);
    m_device->Wait();
    test.Submit0(test.cba);
    test.Submit0(test.cbb);

    // Reset the signal
    test.Submit0(reset);
    m_device->Wait();

    test.Submit0Signal(test.cba);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit1Wait(test.cbb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeSyncVal, QSOBarrierHazard) {
    all_queue_count_ = true;
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires at least 2 TRANSFER capable queues in the same queue_family.";
    }

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);

    vkt::Image image_a(*m_device, image_ci, vkt::set_layout);
    image_a.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image image_b(*m_device, image_ci, vkt::set_layout);
    image_b.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageSubresourceLayers all_layers{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkImageCopy full_region = {all_layers, zero_offset, all_layers, zero_offset, full_extent};

    test.BeginA();
    test.CopyGeneral(image_a, image_b, full_region);
    test.End();

    test.BeginB();
    image_a.ImageMemoryBarrier(*test.current_cb, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_NONE, VK_ACCESS_NONE,
                               VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    test.End();

    // We're going to do the copy first, then use the skip on fail, to test three different ways...
    test.Submit0Signal(test.cba);

    // First asynchronously fail -- the pipeline barrier in B shouldn't work on queue 1
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-READ ");
    test.Submit1(test.cbb);
    m_errorMonitor->VerifyFound();

    // Next synchronously fail -- the pipeline barrier in B shouldn't work on queue 1
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    test.Submit1Wait(test.cbb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    m_errorMonitor->VerifyFound();

    // Then prove qso works (note that with the failure, the semaphore hasn't been waited, nor the layout changed)
    test.Submit0Wait(test.cbb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    m_device->Wait();
}

TEST_F(NegativeSyncVal, QSRenderPass) {
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    CreateRenderPassHelper rp_helper(m_device);
    rp_helper.InitAllAttachmentsToLayoutGeneral();

    rp_helper.InitImageAndView();
    rp_helper.InitAttachmentLayouts();  // Quiet any CoreChecks ImageLayout complaints
    m_device->Wait();                   // and quiesce the system

    // The  dependency protects the input attachment but not the color attachment
    VkSubpassDependency protect_input_subpass_0 = {VK_SUBPASS_EXTERNAL,
                                                   0,
                                                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                   VK_ACCESS_TRANSFER_WRITE_BIT,
                                                   VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                                                   0U};
    rp_helper.subpass_dep.push_back(protect_input_subpass_0);

    rp_helper.InitRenderPass();
    rp_helper.InitFramebuffer();
    rp_helper.InitBeginInfo();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    auto do_clear = [](vkt::CommandBuffer& cb_obj, CreateRenderPassHelper& rp_helper) {
        VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vk::CmdClearColorImage(cb_obj.handle(), rp_helper.image_input->handle(), VK_IMAGE_LAYOUT_GENERAL, &rp_helper.ccv, 1,
                               &full_subresource_range);
        vk::CmdClearColorImage(cb_obj.handle(), rp_helper.image_color->handle(), VK_IMAGE_LAYOUT_GENERAL, &rp_helper.ccv, 1,
                               &full_subresource_range);
    };

    // Single renderpass barrier  (sanity check)
    cb0.Begin();
    do_clear(cb0, rp_helper);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb0.BeginRenderPass(rp_helper.render_pass_begin);
    m_errorMonitor->VerifyFound();
    // No "end render pass" as the begin fails

    cb0.End();
    cb0.Reset();

    // Inter CB detection (dual cb), load is safe, clear errors at submit time
    cb0.Begin();
    do_clear(cb0, rp_helper);
    cb0.End();

    cb1.Begin();
    cb1.BeginRenderPass(rp_helper.render_pass_begin);
    cb1.EndRenderPass();
    cb1.End();

    VkSubmitInfo submit2 = vku::InitStructHelper();
    VkCommandBuffer two_cbs[2] = {cb0.handle(), cb1.handle()};
    submit2.commandBufferCount = 2;
    submit2.pCommandBuffers = two_cbs;
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit2, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_device->Wait();  // quiesce the system for the next subtest

    CreateRenderPassHelper rp_helper2(m_device);
    rp_helper2.InitAllAttachmentsToLayoutGeneral();

    rp_helper2.InitImageAndView();
    rp_helper2.InitAttachmentLayouts();  // Quiet any CoreChecks ImageLayout complaints
    m_device->Wait();                    // and quiesce the system

    // The  dependency protects the input attachment but not the color attachment
    VkSubpassDependency protect_input_subpass_1 = protect_input_subpass_0;
    protect_input_subpass_1.dstSubpass = 1;
    rp_helper2.subpass_dep.push_back(protect_input_subpass_1);

    // Two subpasses to ensure that the "next subpass" error checks work
    rp_helper2.InitAttachmentArrays();
    rp_helper2.AddPreserveInputColorSubpassDescription();
    rp_helper2.AddInputColorSubpassDescription();

    rp_helper2.InitRenderPass();
    rp_helper2.InitFramebuffer();
    rp_helper2.InitBeginInfo();

    // Single CB sanity check
    cb0.Reset();
    cb0.Begin();
    do_clear(cb0, rp_helper2);
    cb0.BeginRenderPass(rp_helper2.render_pass_begin);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb0.NextSubpass();
    m_errorMonitor->VerifyFound();

    cb0.Reset();
    cb0.Begin();
    do_clear(cb0, rp_helper2);
    cb0.End();

    cb1.Reset();
    cb1.Begin();
    cb1.BeginRenderPass(rp_helper2.render_pass_begin);
    cb1.NextSubpass();
    cb1.EndRenderPass();
    cb1.End();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit2, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_device->Wait();  // and quiesce the system
}

TEST_F(NegativeSyncVal, AvailabilityWithoutVisibilityForBuffer) {
    TEST_DESCRIPTION("Buffer barrier makes writes available but not visible. The second write generates WAW harard.");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 1024;
    const vkt::Buffer staging_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    const vkt::Buffer buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy region = {};
    region.size = size;

    m_command_buffer.Begin();
    // Perform a copy
    vk::CmdCopyBuffer(m_command_buffer, staging_buffer, buffer, 1, &region);

    // Make writes available
    VkBufferMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.buffer = buffer;
    barrier.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                           &barrier, 0, nullptr);

    // Perform one more copy. Should generate WAW due to missing visibility operation.
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(m_command_buffer, staging_buffer, buffer, 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, ImageCopyHazardsLayoutTransition) {
    TEST_DESCRIPTION("Copy to image and then start image layout transition without waiting for copy to end.");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    vkt::Buffer buffer(*m_device, 64 * 64 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, usage);
    image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {64, 64, 1};

    VkImageMemoryBarrier transition = vku::InitStructHelper();
    transition.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transition.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    transition.image = image;
    transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    // Create only execution dependency but do not specify any accesses, so copy writes still
    // hazards with image layout transition writes
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0,
                           nullptr, 1, &transition);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7010#issuecomment-1846751346
TEST_F(NegativeSyncVal, TestMessageReportingWithManyBarriers) {
    TEST_DESCRIPTION("Hazardous pipeline barrier contains many barrier structures (> 255). Test that reporting can handle this.");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    // Setup copy from buffer to image that creates hazardous situation with the earlier layout transition.
    vkt::Buffer buffer(*m_device, 64 * 64 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, usage);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {64, 64, 1};

    VkImageMemoryBarrier transition = vku::InitStructHelper();
    transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    transition.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transition.image = image;
    transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    // Create a lot of buffer barriers (> 255) so error reporting will have a lot of objects to report.
    constexpr uint32_t buffer_count = 300;
    std::vector<vkt::Buffer> buffers;
    std::vector<VkBufferMemoryBarrier> buffer_barriers(buffer_count);
    for (uint32_t i = 0; i < buffer_count; i++) {
        buffers.emplace_back(*m_device, 16, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        buffer_barriers[i] = vku::InitStructHelper();
        buffer_barriers[i].buffer = buffers[i];
        buffer_barriers[i].size = VK_WHOLE_SIZE;
    }

    m_command_buffer.Begin();

    // Create only execution dependency but do not specify any accesses,
    // so subsequent copy writes will hazard with the layout transition writes
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                           buffer_count, buffer_barriers.data(), 1, &transition);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

// The original issue was that writeonly buffer accesss can be detected as READ:
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7093
TEST_F(NegativeSyncVal, WriteOnlyBufferWriteHazard) {
    TEST_DESCRIPTION("Test that writeonly buffer access is reported as WRITE access");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buf_a(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    vkt::Buffer buf_b(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorBufferInfo(0, buf_a, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, buf_b, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) readonly buffer buf_a { uint values_a[]; };
        layout(set=0, binding=1) writeonly buffer buf_b { uint values_b[]; };
        void main(){
            values_b[0] = values_a[0];
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    VkBufferCopy region{};
    region.size = 128;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_,
                              0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    // Test that we get WAW and not WAR
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBuffer(m_command_buffer, buf_a, buf_b, 1, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, WriteOnlyImageWriteHazard) {
    TEST_DESCRIPTION("Test that writeonly image access is reported as WRITE access");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_source(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                                 });
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();

    const char* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, rgba8) writeonly uniform image2D image;
        void main(){
            imageStore(image, ivec2(0), vec4(0.5f));
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {32, 32, 1};

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_,
                              0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    // Test that we get WAW and not WAR
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdCopyBufferToImage(m_command_buffer, copy_source, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, UseShaderReadAccessForUniformBuffer) {
    TEST_DESCRIPTION("SHADER_READ_BIT barrier cannot protect UNIFORM_READ_BIT accesses");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 1024;
    const vkt::Buffer staging_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    const vkt::Buffer uniform_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkBufferCopy region = {};
    region.size = size;

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform UBO { float pong; } constants;
        void main(){
            float ping = constants.pong;
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    // Transfer data.
    vk::CmdCopyBuffer(m_command_buffer, staging_buffer, uniform_buffer, 1, &region);

    // Attempt to barrier transfer writes from subsequent dispatch reads.
    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    // VK_ACCESS_2_SHADER_READ_BIT cannot protect uniform buffer shader accesses.
    // Expect RAW on the next dispatch.
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.memoryBarrierCount = 1;
    dep_info.pMemoryBarriers = &barrier;
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);

    // Initiate dispatch that reads tranferred data.
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_,
                              0, nullptr);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, FillBufferMissingBarrier) {
    TEST_DESCRIPTION("Missing synchronization with vkCmdFillBuffer TRANSFER_WRITE access");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 1024;
    vkt::Buffer src_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy region{};
    region.size = size;

    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, src_buffer, 0, size, 42);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, dst_buffer, 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, FillBufferWrongBarrier) {
    TEST_DESCRIPTION("Insufficient synchronization with vkCmdFillBuffer TRANSFER_WRITE access");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 1024;
    vkt::Buffer src_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy region{};
    region.size = size;

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;    // should be COPY
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;  // should be READ
    barrier.buffer = src_buffer;
    barrier.size = size;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, src_buffer, 0, size, 42);
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, dst_buffer, 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, UpdateBufferMissingBarrier) {
    TEST_DESCRIPTION("Missing synchronization with vkCmdUpdateBuffer TRANSFER_WRITE access");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 64;
    vkt::Buffer src_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    std::array<uint8_t, size> data = {};

    VkBufferCopy region{};
    region.size = size;

    m_command_buffer.Begin();
    vk::CmdUpdateBuffer(m_command_buffer, src_buffer, 0, static_cast<VkDeviceSize>(data.size()), data.data());
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, dst_buffer, 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, UpdateBufferWrongBarrier) {
    TEST_DESCRIPTION("Insufficient synchronization with vkCmdUpdateBuffer TRANSFER_WRITE access");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 64;
    vkt::Buffer src_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    std::array<uint8_t, size> data = {};

    VkBufferCopy region{};
    region.size = size;

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;  // should be CLEAR
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    barrier.buffer = src_buffer;
    barrier.size = size;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    vk::CmdUpdateBuffer(m_command_buffer, src_buffer, 0, static_cast<VkDeviceSize>(data.size()), data.data());
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, dst_buffer, 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, QSWriteRacingWrite) {
    TEST_DESCRIPTION("Write to the same image from different queues");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Queue* transfer_queue = m_device->TransferOnlyQueue();
    if (!transfer_queue) {
        GTEST_SKIP() << "Transfer-only queue is not present";
    }

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer(*m_device, 64 * 64, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {64, 64, 1};

    // Submit from Graphics queue: perform image layout transition (WRITE access).
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    image_barrier.srcAccessMask = VK_ACCESS_2_NONE;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    image_barrier.dstAccessMask = VK_ACCESS_2_NONE;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &image_barrier;
    vk::CmdPipelineBarrier2(cb0, &dep_info);
    cb0.End();

    m_default_queue->Submit2(cb0);

    // Submit from Transfer queue: write image data (racing WRITE access)
    vkt::CommandPool transfer_pool(*m_device, transfer_queue->family_index);
    vkt::CommandBuffer cb1(*m_device, transfer_pool);
    cb1.Begin();
    vk::CmdCopyBufferToImage(cb1, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    cb1.End();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
    transfer_queue->Submit2(cb1);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeSyncVal, QSWriteRacingWrite2) {
    TEST_DESCRIPTION("Transfer queue synchronizes with graphics queue and after that both queues initiate image write");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Queue* transfer_queue = m_device->TransferOnlyQueue();
    if (!transfer_queue) {
        GTEST_SKIP() << "Transfer-only queue is not present";
    }

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer(*m_device, 64 * 64, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {64, 64, 1};

    vkt::Semaphore semaphore(*m_device);

    // Submit on Graphics queue: empty batch just so Transfer queue can synchronize with.
    m_default_queue->Submit2(vkt::no_cmd, vkt::Signal(semaphore));

    // Submit on Graphics queue: image layout transition (WRITE access).
    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    image_barrier.srcAccessMask = VK_ACCESS_2_NONE;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    image_barrier.dstAccessMask = VK_ACCESS_2_NONE;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &image_barrier;
    vk::CmdPipelineBarrier2(cb1, &dep_info);
    cb1.End();

    m_default_queue->Submit2(cb1);

    // Submit on Transfer queue: write image data (racing WRITE access)
    vkt::CommandPool transfer_pool(*m_device, transfer_queue->family_index);
    vkt::CommandBuffer cb2(*m_device, transfer_pool);
    cb2.Begin();
    vk::CmdCopyBufferToImage(cb2, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    cb2.End();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
    transfer_queue->Submit2(cb2, vkt::Wait(semaphore));
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

// Gfx      :   Submit 0: [Read A, signal sem]  Submit 1: [Read A]
// Compute  :   Submit 1: [Unrelated access, signal sem2]
// Transfer :   Submit 3: [wait sem+sem2, Write A]
//
// Write in Submit 3 races with read in Sumbit 1.
// Compute submit is needed to generate tags in such a ways that reproduces regression
// with incorrect async tag tracking. This generates compute tags that have larger values
// than the tags from the second gfx queue submission.
TEST_F(NegativeSyncVal, QSWriteRacingRead) {
    TEST_DESCRIPTION("Write-racing-read scenario that involves three queues");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Queue* gfx_queue = m_default_queue;
    vkt::CommandPool gfx_pool(*m_device, gfx_queue->family_index);

    vkt::Queue* compute_queue = m_device->ComputeOnlyQueue();
    if (!compute_queue) {
        GTEST_SKIP() << "Compute-only queue is not present";
    }
    vkt::CommandPool compute_pool(*m_device, compute_queue->family_index);

    vkt::Queue* transfer_queue = m_device->TransferOnlyQueue();
    if (!transfer_queue) {
        GTEST_SKIP() << "Transfer-only queue is not present";
    }
    vkt::CommandPool transfer_pool(*m_device, transfer_queue->family_index);

    constexpr VkDeviceSize size = 1024;
    vkt::Buffer buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer gfx_src_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer gfx_dst_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer gfx_dst_buffer2(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer compute_src_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer compute_dst_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy region{};
    region.size = size;

    vkt::CommandBuffer gfx_cb(*m_device, gfx_pool);
    vkt::CommandBuffer gfx_cb2(*m_device, gfx_pool);
    vkt::CommandBuffer compute_cb(*m_device, compute_pool);
    vkt::CommandBuffer transfer_cb(*m_device, transfer_pool);
    vkt::Semaphore semaphore(*m_device);
    vkt::Semaphore semaphore2(*m_device);

    // Submit 0 (gfx queue): buffer read
    gfx_cb.Begin();
    vk::CmdCopyBuffer(gfx_cb, buffer, gfx_dst_buffer, 1, &region);
    gfx_cb.End();
    gfx_queue->Submit2(gfx_cb, vkt::Signal(semaphore));

    // Submit 1 (gfx queue): another read from the same buffer
    gfx_cb2.Begin();
    vk::CmdCopyBuffer(gfx_cb2, buffer, gfx_dst_buffer2, 1, &region);
    gfx_cb2.End();
    gfx_queue->Submit2(gfx_cb2);

    // Submit 2 (compute queue): compute buffer copy (does not interract with other buffers)
    compute_cb.Begin();
    vk::CmdCopyBuffer(compute_cb, compute_src_buffer, compute_dst_buffer, 1, &region);
    compute_cb.End();
    compute_queue->Submit2(compute_cb, vkt::Signal(semaphore2));

    // Submit 3 (transfer queue): wait for gfx/compute semaphores
    {
        transfer_cb.Begin();
        vk::CmdCopyBuffer(transfer_cb, gfx_src_buffer, buffer, 1, &region);
        transfer_cb.End();

        VkSemaphoreSubmitInfo wait_infos[2];
        wait_infos[0] = vku::InitStructHelper();
        wait_infos[0].semaphore = semaphore;
        wait_infos[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        wait_infos[1] = vku::InitStructHelper();
        wait_infos[1].semaphore = semaphore2;
        wait_infos[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        VkCommandBufferSubmitInfo cbuf_info = vku::InitStructHelper();
        cbuf_info.commandBuffer = transfer_cb;
        VkSubmitInfo2 submit = vku::InitStructHelper();
        submit.waitSemaphoreInfoCount = 2;
        submit.pWaitSemaphoreInfos = wait_infos;
        submit.commandBufferInfoCount = 1;
        submit.pCommandBufferInfos = &cbuf_info;
        m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-READ");
        vk::QueueSubmit2(*transfer_queue, 1, &submit, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    gfx_queue->Wait();
    compute_queue->Wait();
    transfer_queue->Wait();
}

TEST_F(NegativeSyncVal, RenderPassStoreOpNone) {
    TEST_DESCRIPTION("Missing synchronization with draw command when render pass uses storeOp=NONE");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(Gpu());
    const VkImageLayout input_attachment_layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(depth_format, input_attachment_layout, input_attachment_layout, VK_ATTACHMENT_LOAD_OP_LOAD,
                                VK_ATTACHMENT_STORE_OP_NONE);
    rp.AddAttachmentReference({0, input_attachment_layout});
    rp.AddInputAttachment(0);
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 32, 32, 1, depth_format, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    image.SetLayout(input_attachment_layout);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &image_view.handle());

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    // Form an execution dependency with loadOp (EARLY_FRAGMENT_TESTS) but not with draw command (FRAGMENT_SHADER)
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
    layout_transition.srcAccessMask = VK_ACCESS_2_NONE;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.dstAccessMask = VK_ACCESS_2_NONE;
    layout_transition.oldLayout = input_attachment_layout;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    // Fragment shader READs input attachment.
    VkShaderObj fs(this, kFragmentSubpassLoadGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    OneOffDescriptorSet descriptor_set(m_device, {binding});
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                            input_attachment_layout);
    descriptor_set.UpdateDescriptorSets();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();

    // SYNC-HAZARD-WRITE-AFTER-READ hazard: transition should synchronize with draw command
    m_errorMonitor->SetDesiredError("SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ");
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

// TODO: this very simple case should cause WRITE-AFTER-WRITE hazard but it passes successfully.
// This happens because of SupressedBoundDescriptorWAW(). Remove SupressedBoundDescriptorWAW()
// after SDK release. SupressedBoundDescriptorWAW works only for command buffer validation,
// that's why the next test (WriteSameLocationFromTwoSubmits) that uses Submit does not have this problem.
//
// Prevention of tricky/complex false-positives should be done by having descriptor validation feature
// turned off as default (current plan), as opposite to solutions where syncval behaves unreliable even
// in simple scenarios. Simple should work. Complex should be manageable.
TEST_F(NegativeSyncVal, WriteSameLocationFromTwoDispatches) {
    TEST_DESCRIPTION("Not synchronized write to the same location causes WAW hazard");
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT}});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer ssbo { uint data[]; };
        void main(){
            data[0] = 42;
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    // TODO: enable error monitor when we fix WAW detection.
    // m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    // m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, WriteSameLocationFromTwoSubmits) {
    TEST_DESCRIPTION("Not synchronized write to the same location causes WAW hazard");
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT}});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer ssbo { uint data[]; };
        void main(){
            data[0] = 42;
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncVal, ResourceHandleIndexStability) {
    TEST_DESCRIPTION("Test that stale handle indices (inconsistent state after core validation error) are handled correctly");
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_c(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_d(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);  // buffer_a: handle index 0, buffer_b: handle index 1
    m_command_buffer.Copy(buffer_c, buffer);    // buffer_c: handle index 2, buffer: handle index 3
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    // Intentional violation of core validation. Begin command buffer while previous one is still pending.
    // The command buffer's Reset call clears HandleRecord array. Due to missing synchronization, the
    // access context still contains references to already remove handles. This core validation error
    // leads to inconsistent command buffer state, but syncval should handle this gracefully and do not
    // crash/assert (although no guarantees about validity of syncval reporting from this moment).
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkBeginCommandBuffer-commandBuffer-00049");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkQueueSubmit-pCommandBuffers-00071");

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer, buffer_d);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    // Hazard prior access refers to "buffer" (handle 3) but handles array is cleared
    // (does not happen under normal circumstances). This should not cause out of bounds array access.
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeSyncVal, ExpandedMetaStage) {
    TEST_DESCRIPTION("Check barrier is printed in compact form");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;  // Read access mask does not protect subsequent copy writes
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.memoryBarrierCount = 1;
    dep_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dep_info);
    m_errorMonitor->SetDesiredErrorRegex("SYNC-HAZARD-WRITE-AFTER-WRITE",
                                         "VK_ACCESS_2_SHADER_READ_BIT accesses at VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT");
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, ExpandedMetaStage2) {
    TEST_DESCRIPTION("Check barrier is printed in compact form");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;  // Read access mask does not protect subsequent copy writes
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.memoryBarrierCount = 1;
    dep_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dep_info);
    m_errorMonitor->SetDesiredErrorRegex("SYNC-HAZARD-WRITE-AFTER-WRITE",
                                         "VK_ACCESS_2_MEMORY_READ_BIT accesses at VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT");
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, ExpandedMetaStage3) {
    TEST_DESCRIPTION("Check barrier is printed in compact form");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;  // Shader write access mask does not protect subsequent copy writes
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.memoryBarrierCount = 1;
    dep_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dep_info);
    m_errorMonitor->SetDesiredErrorRegex("SYNC-HAZARD-WRITE-AFTER-WRITE",
                                         "VK_ACCESS_2_SHADER_WRITE_BIT accesses at VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT");
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncVal, AmdBufferMarker) {
    TEST_DESCRIPTION("Hazard with AMD buffer marker accesses");
    AddRequiredExtensions(VK_AMD_BUFFER_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_b, buffer_a);
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdWriteBufferMarkerAMD(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, buffer_a, 0, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}
