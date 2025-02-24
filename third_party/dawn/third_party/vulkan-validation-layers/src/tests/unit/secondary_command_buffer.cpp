/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/render_pass_helper.h"

class NegativeSecondaryCommandBuffer : public VkLayerTest {};

TEST_F(NegativeSecondaryCommandBuffer, AsPrimary) {
    TEST_DESCRIPTION("Create a secondary command buffer and pass it to QueueSubmit.");
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pCommandBuffers-00075");

    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.End();

    m_default_queue->Submit(secondary);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSecondaryCommandBuffer, Barrier) {
    TEST_DESCRIPTION("Add an invalid image barrier in a secondary command buffer");
    RETURN_IF_SKIP(Init());

    // A renderpass with a single subpass that declared a self-dependency
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.AddSubpassDependency(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT);
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView imageView = image.CreateView();
    // Second image that img_barrier will incorrectly use
    vkt::Image image2(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image2.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &imageView.handle());

    m_command_buffer.Begin();

    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.Handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &rpbi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(*m_device, pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cbii = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                                           nullptr,
                                           rp.Handle(),
                                           0,
                                           VK_NULL_HANDLE,  // Set to NULL FB handle intentionally to flesh out any errors
                                           VK_FALSE,
                                           0,
                                           0};
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                     VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                                     &cbii};
    vk::BeginCommandBuffer(secondary.handle(), &cbbi);
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.image = image2.handle();  // Image mis-matches with FB image
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(secondary.handle(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    secondary.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-image-04073");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSecondaryCommandBuffer, Sync2AsPrimary) {
    TEST_DESCRIPTION("Create a secondary command buffer and pass it to QueueSubmit2KHR.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferSubmitInfo-commandBuffer-03890");

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.End();

    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = secondary.handle();

    VkSubmitInfo2 submit_info = vku::InitStructHelper();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSecondaryCommandBuffer, RerecordedExplicitReset) {
#if defined(VVL_ENABLE_TSAN)
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif

    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-recording");

    // A pool we can reset in.
    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(*m_device, pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary.Begin();
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());

    // rerecording of secondary
    secondary.Reset();  // explicit reset here.
    secondary.Begin();
    secondary.End();

    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSecondaryCommandBuffer, RerecordedNoReset) {
#if defined(VVL_ENABLE_TSAN)
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif

    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-recording");

    // A pool we can reset in.
    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(*m_device, pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary.Begin();
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());

    // rerecording of secondary
    secondary.Begin();  // implicit reset in begin
    secondary.End();

    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSecondaryCommandBuffer, CascadedInvalidation) {
    RETURN_IF_SKIP(Init());

    VkEventCreateInfo eci = vku::InitStructHelper();
    eci.flags = 0;
    VkEvent event;
    vk::CreateEvent(device(), &eci, nullptr, &event);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    vk::CmdSetEvent(secondary.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_command_buffer.End();

    // destroying the event should invalidate both primary and secondary CB
    vk::DestroyEvent(device(), event, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSecondaryCommandBuffer, ExecuteCommandsTo) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands to a Secondary command buffer");

    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer main_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb.Begin();
    secondary_cb.End();

    main_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-09375");
    vk::CmdExecuteCommands(main_cb.handle(), 1, &secondary_cb.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSecondaryCommandBuffer, SimultaneousUseTwoExecutes) {
    RETURN_IF_SKIP(Init());

    const char *simultaneous_use_message = "VUID-vkCmdExecuteCommands-pCommandBuffers-00092";

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper();
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, &inh};

    secondary.Begin(&cbbi);
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->SetDesiredError(simultaneous_use_message);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSecondaryCommandBuffer, SimultaneousUseSingleExecute) {
    RETURN_IF_SKIP(Init());

    // variation on previous test executing the same CB twice in the same
    // CmdExecuteCommands call

    const char *simultaneous_use_message = "VUID-vkCmdExecuteCommands-pCommandBuffers-00093";

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper();
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, &inh};

    secondary.Begin(&cbbi);
    secondary.End();

    m_command_buffer.Begin();
    VkCommandBuffer cbs[] = {secondary.handle(), secondary.handle()};
    m_errorMonitor->SetDesiredError(simultaneous_use_message);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 2, cbs);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSecondaryCommandBuffer, ExecuteDiffertQueueFlags) {
    TEST_DESCRIPTION("Allocate a command buffer from two different queues and try to use a secondary command buffer");

    RETURN_IF_SKIP(Init());

    if (m_device->Physical().queue_properties_.size() < 2) {
        GTEST_SKIP() << "Need 2 different queues for testing skipping.";
    }

    // First two queue families
    uint32_t queue_index_a = 0;
    uint32_t queue_index_b = 1;

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.flags = 0;

    pool_create_info.queueFamilyIndex = queue_index_a;
    vkt::CommandPool command_pool_a(*m_device, pool_create_info);
    ASSERT_TRUE(command_pool_a.initialized());

    pool_create_info.queueFamilyIndex = queue_index_b;
    vkt::CommandPool command_pool_b(*m_device, pool_create_info);
    ASSERT_TRUE(command_pool_b.initialized());

    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.commandPool = command_pool_a.handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkt::CommandBuffer command_buffer_primary(*m_device, command_buffer_allocate_info);
    ASSERT_TRUE(command_buffer_primary.initialized());

    command_buffer_allocate_info.commandPool = command_pool_b.handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    vkt::CommandBuffer command_buffer_secondary(*m_device, command_buffer_allocate_info);

    VkCommandBufferInheritanceInfo cmdbuff_ii = vku::InitStructHelper();
    cmdbuff_ii.renderPass = m_renderPass;
    cmdbuff_ii.subpass = 0;

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.pInheritanceInfo = &cmdbuff_ii;

    // secondary
    command_buffer_secondary.Begin(&begin_info);
    command_buffer_secondary.End();

    // Try using different pool's command buffer as secondary
    command_buffer_primary.Begin(&begin_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-00094");
    vk::CmdExecuteCommands(command_buffer_primary.handle(), 1, &command_buffer_secondary.handle());
    m_errorMonitor->VerifyFound();
    command_buffer_primary.End();
}

TEST_F(NegativeSecondaryCommandBuffer, ExecuteUnrecorded) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands with a CB in the initial state");
    RETURN_IF_SKIP(Init());
    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    // never record secondary

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-00089");
    m_command_buffer.Begin();
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSecondaryCommandBuffer, ExecuteWithLayoutMismatch) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands with a CB with incorrect initial layout.");

    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info = vkt::Image::ImageCreateInfo2D(
        32, 1, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);

    VkImageSubresource image_sub = vkt::Image::Subresource(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
    VkImageSubresourceRange image_sub_range = vkt::Image::SubresourceRange(image_sub);

    VkImageMemoryBarrier image_barrier =
        image.ImageMemoryBarrier(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, image_sub_range);

    auto pipeline = [&image_barrier](const vkt::CommandBuffer &cb, VkImageLayout old_layout, VkImageLayout new_layout) {
        image_barrier.oldLayout = old_layout;
        image_barrier.newLayout = new_layout;
        vk::CmdPipelineBarrier(cb.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                               0, nullptr, 1, &image_barrier);
    };

    // Validate that mismatched use of image layout in secondary command buffer is caught at record time
    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    pipeline(secondary, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    secondary.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-vkCmdExecuteCommands-commandBuffer-00001");
    m_command_buffer.Begin();
    pipeline(m_command_buffer, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_command_buffer.Reset();
    secondary.Reset();

    // Validate that UNDEFINED doesn't false positive on us
    secondary.Begin();
    pipeline(secondary, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    secondary.End();
    m_command_buffer.Begin();
    pipeline(m_command_buffer, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_command_buffer.End();
}

TEST_F(NegativeSecondaryCommandBuffer, RenderPassScope) {
    TEST_DESCRIPTION(
        "Test secondary buffers executed in wrong render pass scope wrt VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::CommandBuffer sec_cmdbuff_inside_rp(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer sec_cmdbuff_outside_rp(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        m_renderPass,
        0,  // subpass
        Framebuffer(),
    };
    const VkCommandBufferBeginInfo cmdbuff_bi_tmpl = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                                      nullptr,  // pNext
                                                      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};

    VkCommandBufferBeginInfo cmdbuff_inside_rp_bi = cmdbuff_bi_tmpl;
    cmdbuff_inside_rp_bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    sec_cmdbuff_inside_rp.Begin(&cmdbuff_inside_rp_bi);
    sec_cmdbuff_inside_rp.End();

    VkCommandBufferBeginInfo cmdbuff_outside_rp_bi = cmdbuff_bi_tmpl;
    cmdbuff_outside_rp_bi.flags &= ~VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    sec_cmdbuff_outside_rp.Begin(&cmdbuff_outside_rp_bi);
    sec_cmdbuff_outside_rp.End();

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-00100");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &sec_cmdbuff_inside_rp.handle());
    m_errorMonitor->VerifyFound();

    VkRenderPassBeginInfo rp_bi = vku::InitStruct<VkRenderPassBeginInfo>(
        nullptr, m_renderPass, Framebuffer(), VkRect2D{{0, 0}, {32u, 32u}}, static_cast<uint32_t>(m_renderPassClearValues.size()),
        m_renderPassClearValues.data());
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &rp_bi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-00096");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &sec_cmdbuff_outside_rp.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSecondaryCommandBuffer, ClearColorAttachmentsRenderArea) {
    TEST_DESCRIPTION(
        "Create a secondary command buffer with CmdClearAttachments call that has a rect outside of renderPass renderArea");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_command_pool.handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    vkt::CommandBuffer secondary_command_buffer(*m_device, command_buffer_allocate_info);
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = vku::InitStructHelper();
    command_buffer_inheritance_info.renderPass = m_renderPass;
    command_buffer_inheritance_info.framebuffer = Framebuffer();

    VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
    command_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    secondary_command_buffer.Begin(&command_buffer_begin_info);

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    // x extent of 257 exceeds render area of 256
    VkClearRect clear_rect = {{{0, 0}, {257, 32}}, 0, 1};
    vk::CmdClearAttachments(secondary_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    secondary_command_buffer.End();
    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-pRects-00016");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_command_buffer.handle());
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeSecondaryCommandBuffer, RenderPassContentsFirstSubpass) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with CmdBeginRenderPass that hasn't set "
        "VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        m_renderPass,
        0,  // subpass
        Framebuffer(),
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.Begin(&cmdbuff__bi);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPass, Framebuffer(), 32, 32, m_renderPassClearValues.size(),
                                     m_renderPassClearValues.data());

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-contents-09680");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeSecondaryCommandBuffer, RenderPassContentsNotFirstSubpass) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands inside a render pass begun with vkCmdNextSubpass that hasn't set "
        "VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS");
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    std::array subpasses = {vku::InitStruct<VkSubpassDescription2>(), vku::InitStruct<VkSubpassDescription2>()};

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = subpasses.size();
    render_pass_ci.pSubpasses = subpasses.data();
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    vkt::RenderPass rp(*m_device, render_pass_ci);

    // A compatible framebuffer.
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp.handle(), 1, &view.handle());

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        rp.handle(),
        1,  // subpass
        fb.handle(),
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.Begin(&cmdbuff__bi);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.handle(), fb.handle(), 32, 32, m_renderPassClearValues.size(),
                                     m_renderPassClearValues.data());
    m_command_buffer.NextSubpass();
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-None-09681");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeSecondaryCommandBuffer, ExecuteCommandsSubpassIndices) {
    TEST_DESCRIPTION("Test invalid subpass when calling CmdExecuteCommands");
    RETURN_IF_SKIP(Init());

    // A renderpass with two subpasses, both writing the same attachment.
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkSubpassDependency dependencies = {
        0,                                     // srcSubpass
        1,                                     // dstSubpass
        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,    // srcStageMask
        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,    // dstStageMask
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,  // srcAccessMask
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,  // dstAccessMask
        0,                                     // dependencyFlags
    };

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependencies;
    vkt::RenderPass render_pass(*m_device, rpci);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();
    vkt::Framebuffer framebuffer(*m_device, render_pass.handle(), 1, &imageView.handle());

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        render_pass.handle(),
        1,  // subpass
        framebuffer.handle(),
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.Begin(&cmdbuff__bi);
    secondary.End();

    const auto rp_bi = vku::InitStruct<VkRenderPassBeginInfo>(nullptr, render_pass.handle(), framebuffer.handle(),
                                                              VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_bi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-06019");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSecondaryCommandBuffer, IncompatibleRenderPassesInExecuteCommands) {
    TEST_DESCRIPTION("Test invalid subpass when calling CmdExecuteCommands");
    RETURN_IF_SKIP(Init());

    // A renderpass with two subpasses, both writing the same attachment.
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 0, nullptr};
    vkt::RenderPass render_pass_1(*m_device, rpci);
    rpci.subpassCount = 2;
    vkt::RenderPass render_pass_2(*m_device, rpci);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();
    vkt::Framebuffer framebuffer(*m_device, render_pass_1.handle(), 1, &imageView.handle());

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        nullptr,  // pNext
        render_pass_2.handle(),
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.Begin(&cmdbuff__bi);
    secondary.End();

    const auto rp_bi = vku::InitStruct<VkRenderPassBeginInfo>(nullptr, render_pass_1.handle(), framebuffer.handle(),
                                                              VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_bi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pBeginInfo-06020");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeSecondaryCommandBuffer, CommandBufferInheritanceInfo) {
    TEST_DESCRIPTION("Test invalid command buffer begin inheritance info.");
    AddRequiredFeature(vkt::Feature::inheritedQueries);
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-vkBeginCommandBuffer-commandBuffer-00051");
    vk::BeginCommandBuffer(secondary.handle(), &begin_info);
    m_errorMonitor->VerifyFound();

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper();
    inheritance_info.queryFlags = VK_QUERY_CONTROL_PRECISE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    m_errorMonitor->SetDesiredError("VUID-vkBeginCommandBuffer-commandBuffer-00052");
    vk::BeginCommandBuffer(secondary.handle(), &begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSecondaryCommandBuffer, NestedCommandBufferRendering) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nestedCommandBuffer);

    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::CommandBuffer secondary1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceRenderingInfo cbiri = vku::InitStructHelper();
    cbiri.colorAttachmentCount = 1;
    cbiri.pColorAttachmentFormats = &m_render_target_fmt;
    cbiri.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper(&cbiri);
    cbii.occlusionQueryEnable = VK_FALSE;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;

    secondary1.Begin(&cbbi);
    secondary1.End();

    secondary2.Begin(&cbbi);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-nestedCommandBufferRendering-09377");
    vk::CmdExecuteCommands(secondary2.handle(), 1u, &secondary1.handle());
    m_errorMonitor->VerifyFound();
    secondary2.End();
}

TEST_F(NegativeSecondaryCommandBuffer, NestedCommandBufferSimultaneousUse) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nestedCommandBuffer);
    AddRequiredFeature(vkt::Feature::nestedCommandBufferRendering);

    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::CommandBuffer secondary1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceRenderingInfo cbiri = vku::InitStructHelper();
    cbiri.colorAttachmentCount = 1;
    cbiri.pColorAttachmentFormats = &m_render_target_fmt;
    cbiri.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper(&cbiri);
    cbii.occlusionQueryEnable = VK_FALSE;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    cbbi.pInheritanceInfo = &cbii;

    secondary1.Begin(&cbbi);
    secondary1.End();

    secondary2.Begin(&cbbi);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-nestedCommandBufferSimultaneousUse-09378");
    vk::CmdExecuteCommands(secondary2.handle(), 1u, &secondary1.handle());
    m_errorMonitor->VerifyFound();
    secondary2.End();
}

TEST_F(NegativeSecondaryCommandBuffer, MaxCommandBufferNestingLevel) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nestedCommandBuffer);
    AddRequiredFeature(vkt::Feature::nestedCommandBufferRendering);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceNestedCommandBufferPropertiesEXT nested_cb_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(nested_cb_props);
    if (nested_cb_props.maxCommandBufferNestingLevel != 2) {
        GTEST_SKIP() << "needs maxCommandBufferNestingLevel to be 2";
    }

    vkt::CommandBuffer secondary1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary3(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary4(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceRenderingInfo cbiri = vku::InitStructHelper();
    cbiri.colorAttachmentCount = 1;
    cbiri.pColorAttachmentFormats = &m_render_target_fmt;
    cbiri.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper(&cbiri);
    cbii.occlusionQueryEnable = VK_FALSE;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;

    secondary1.Begin(&cbbi);
    secondary1.End();

    secondary2.Begin(&cbbi);
    vk::CmdExecuteCommands(secondary2.handle(), 1u, &secondary1.handle());
    secondary2.End();

    secondary3.Begin(&cbbi);
    vk::CmdExecuteCommands(secondary3.handle(), 1u, &secondary2.handle());
    vk::CmdExecuteCommands(secondary3.handle(), 1u, &secondary1.handle());
    secondary3.End();

    secondary4.Begin(&cbbi);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-nestedCommandBuffer-09376");
    vk::CmdExecuteCommands(secondary4.handle(), 1u, &secondary3.handle());
    m_errorMonitor->VerifyFound();
    secondary4.End();
}

TEST_F(NegativeSecondaryCommandBuffer, MissingInheritedQueriesFeature) {
    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);
    RETURN_IF_SKIP(Init());

    VkQueryPoolCreateInfo qpci = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_PIPELINE_STATISTICS, 1);
    qpci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
    vkt::QueryPool query_pool(*m_device, qpci);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper();
    inheritance_info.pipelineStatistics = qpci.pipelineStatistics;

    VkCommandBufferBeginInfo seconary_begin_info = vku::InitStructHelper();
    seconary_begin_info.pInheritanceInfo = &inheritance_info;
    secondary.Begin(&seconary_begin_info);
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0u, 1u);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0u, 0u);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-00101");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &secondary.handle());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0u);
    m_command_buffer.End();
}

TEST_F(NegativeSecondaryCommandBuffer, MissingSimultaniousUseBit) {
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    secondary.End();

    vkt::CommandBuffer primary(*m_device, m_command_pool);
    primary.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vk::CmdExecuteCommands(primary.handle(), 1u, &secondary.handle());
    primary.End();

    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &secondary.handle());
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00073");
    m_default_queue->Submit(primary);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}
