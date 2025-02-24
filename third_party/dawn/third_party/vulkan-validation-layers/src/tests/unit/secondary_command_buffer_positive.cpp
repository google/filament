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
#include "../framework/render_pass_helper.h"

class PositiveSecondaryCommandBuffer : public VkLayerTest {};

TEST_F(PositiveSecondaryCommandBuffer, Barrier) {
    TEST_DESCRIPTION("Add a pipeline barrier in a secondary command buffer");
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
    image.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkt::ImageView imageView = image.CreateView();

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
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    vk::CmdPipelineBarrier(secondary.handle(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr, 0, nullptr);

    image.ImageMemoryBarrier(secondary, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    secondary.End();

    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveSecondaryCommandBuffer, ClearAttachmentsCalled) {
    TEST_DESCRIPTION(
        "This test is to verify that when vkCmdClearAttachments is called by a secondary commandbuffer, the validation layers do "
        "not throw an error if the primary commandbuffer begins a renderpass before executing the secondary commandbuffer.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    info.pInheritanceInfo = &hinfo;
    hinfo.renderPass = RenderPass();
    hinfo.subpass = 0;
    hinfo.framebuffer = Framebuffer();
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    secondary.Begin(&info);
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0.0;
    color_attachment.clearValue.color.float32[1] = 0.0;
    color_attachment.clearValue.color.float32[2] = 0.0;
    color_attachment.clearValue.color.float32[3] = 0.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    vk::CmdClearAttachments(secondary.handle(), 1, &color_attachment, 1, &clear_rect);
    secondary.End();
    // Modify clear rect here to verify that it doesn't cause validation error
    clear_rect = {{{0, 0}, {99999999, 99999999}}, 0, 0};

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSecondaryCommandBuffer, ClearAttachmentsCalledWithoutFb) {
    TEST_DESCRIPTION(
        "Verify calling vkCmdClearAttachments inside secondary commandbuffer without linking framebuffer pointer to"
        "inheritance struct");

    RETURN_IF_SKIP(Init());

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());
    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view.handle());

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    hinfo.renderPass = RenderPass();

    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    info.pInheritanceInfo = &hinfo;

    secondary.Begin(&info);
    VkClearAttachment color_attachment = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VkClearValue{}};

    VkClearAttachment ds_attachment = {};
    ds_attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    ds_attachment.clearValue.depthStencil.depth = 0.2f;
    ds_attachment.clearValue.depthStencil.stencil = 1;

    const std::array clear_attachments = {color_attachment, ds_attachment};
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    vk::CmdClearAttachments(secondary.handle(), size32(clear_attachments), clear_attachments.data(), 1, &clear_rect);
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSecondaryCommandBuffer, CommandPoolDeleteWithReferences) {
    TEST_DESCRIPTION("Ensure the validation layers bookkeeping tracks the implicit command buffer frees.");
    RETURN_IF_SKIP(Init());

    VkCommandPoolCreateInfo cmd_pool_info = vku::InitStructHelper();
    cmd_pool_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_info.flags = 0;

    VkCommandPool secondary_cmd_pool;
    VkResult res = vk::CreateCommandPool(m_device->handle(), &cmd_pool_info, NULL, &secondary_cmd_pool);
    ASSERT_EQ(VK_SUCCESS, res);

    VkCommandBufferAllocateInfo cmdalloc = vkt::CommandBuffer::CreateInfo(secondary_cmd_pool);
    cmdalloc.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

    VkCommandBuffer secondary_cmds;
    res = vk::AllocateCommandBuffers(m_device->handle(), &cmdalloc, &secondary_cmds);

    VkCommandBufferInheritanceInfo cmd_buf_inheritance_info = vku::InitStructHelper();
    cmd_buf_inheritance_info.renderPass = VK_NULL_HANDLE;
    cmd_buf_inheritance_info.subpass = 0;
    cmd_buf_inheritance_info.framebuffer = VK_NULL_HANDLE;
    cmd_buf_inheritance_info.occlusionQueryEnable = VK_FALSE;
    cmd_buf_inheritance_info.queryFlags = 0;
    cmd_buf_inheritance_info.pipelineStatistics = 0;

    VkCommandBufferBeginInfo secondary_begin = vku::InitStructHelper();
    secondary_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    secondary_begin.pInheritanceInfo = &cmd_buf_inheritance_info;

    res = vk::BeginCommandBuffer(secondary_cmds, &secondary_begin);
    ASSERT_EQ(VK_SUCCESS, res);
    vk::EndCommandBuffer(secondary_cmds);

    m_command_buffer.Begin();
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_cmds);
    m_command_buffer.End();

    // DestroyCommandPool *implicitly* frees the command buffers allocated from it
    vk::DestroyCommandPool(m_device->handle(), secondary_cmd_pool, NULL);
    // If bookkeeping has been lax, validating the reset will attempt to touch deleted data
    res = vk::ResetCommandPool(m_device->handle(), m_command_pool.handle(), 0);
    ASSERT_EQ(VK_SUCCESS, res);
}

TEST_F(PositiveSecondaryCommandBuffer, ClearColorAttachments) {
    TEST_DESCRIPTION("Create a secondary command buffer and record a CmdClearAttachments call into it");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_command_pool.handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_command_buffer;
    ASSERT_EQ(VK_SUCCESS, vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &secondary_command_buffer));
    VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = vku::InitStructHelper();
    command_buffer_inheritance_info.renderPass = m_renderPass;
    command_buffer_inheritance_info.framebuffer = Framebuffer();

    command_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    vk::BeginCommandBuffer(secondary_command_buffer, &command_buffer_begin_info);
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}, 0, 1};
    vk::CmdClearAttachments(secondary_command_buffer, 1, &color_attachment, 1, &clear_rect);
    vk::EndCommandBuffer(secondary_command_buffer);
    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_command_buffer);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveSecondaryCommandBuffer, ImageLayoutTransitions) {
    TEST_DESCRIPTION("Perform an image layout transition in a secondary command buffer followed by a transition in the primary.");
    RETURN_IF_SKIP(Init());
    auto depth_format = FindSupportedDepthStencilFormat(Gpu());
    InitRenderTarget();
    // Allocate a secondary and primary cmd buffer
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_command_pool.handle();
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_command_buffer;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &secondary_command_buffer);
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer primary_command_buffer;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &primary_command_buffer);
    VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = vku::InitStructHelper();
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;

    vk::BeginCommandBuffer(secondary_command_buffer, &command_buffer_begin_info);
    vkt::Image image(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(secondary_command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &img_barrier);
    vk::EndCommandBuffer(secondary_command_buffer);

    // Now update primary cmd buffer to execute secondary and transitions image
    command_buffer_begin_info.pInheritanceInfo = nullptr;
    vk::BeginCommandBuffer(primary_command_buffer, &command_buffer_begin_info);
    vk::CmdExecuteCommands(primary_command_buffer, 1, &secondary_command_buffer);
    VkImageMemoryBarrier img_barrier2 = vku::InitStructHelper();
    img_barrier2.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier2.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    img_barrier2.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    img_barrier2.image = image.handle();
    img_barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    img_barrier2.subresourceRange.baseArrayLayer = 0;
    img_barrier2.subresourceRange.baseMipLevel = 0;
    img_barrier2.subresourceRange.layerCount = 1;
    img_barrier2.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(primary_command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &img_barrier2);
    vk::EndCommandBuffer(primary_command_buffer);
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &primary_command_buffer;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_device->Wait();
    vk::FreeCommandBuffers(device(), m_command_pool.handle(), 1, &secondary_command_buffer);
    vk::FreeCommandBuffers(device(), m_command_pool.handle(), 1, &primary_command_buffer);
}

TEST_F(PositiveSecondaryCommandBuffer, EventStageMask) {
    TEST_DESCRIPTION("Check secondary command buffers transfer event data when executed by primary ones");
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer commandBuffer(*m_device, m_command_pool);
    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    vkt::Event event(*m_device);

    secondary.Begin();
    vk::CmdSetEvent(secondary.handle(), event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    secondary.End();

    commandBuffer.Begin();
    vk::CmdExecuteCommands(commandBuffer.handle(), 1, &secondary.handle());
    vk::CmdWaitEvents(commandBuffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer.End();

    m_default_queue->Submit(commandBuffer);
    m_default_queue->Wait();
}

TEST_F(PositiveSecondaryCommandBuffer, EventsIn) {
    TEST_DESCRIPTION("Test setting and waiting for an event in a secondary command buffer");
    RETURN_IF_SKIP(Init());

    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_portability_subset enabled, skipping.\n";
    }

    vkt::Event ev(*m_device);
    VkEvent ev_handle = ev.handle();
    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer scb = secondary_cb.handle();
    secondary_cb.Begin();
    vk::CmdSetEvent(scb, ev_handle, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    vk::CmdWaitEvents(scb, 1, &ev_handle, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0,
                      nullptr, 0, nullptr);
    secondary_cb.End();
    m_command_buffer.Begin();
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &scb);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveSecondaryCommandBuffer, Nested) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nestedCommandBuffer);
    AddRequiredFeature(vkt::Feature::nestedCommandBufferRendering);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::inheritedQueries);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    vkt::CommandBuffer secondary1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceRenderingInfo cbiri = vku::InitStructHelper();
    cbiri.colorAttachmentCount = 1;
    cbiri.pColorAttachmentFormats = &m_render_target_fmt;
    cbiri.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper(&cbiri);
    cbii.occlusionQueryEnable = VK_TRUE;
    cbii.queryFlags = 0;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;

    secondary1.Begin(&cbbi);
    secondary1.End();

    secondary2.Begin(&cbbi);
    vk::CmdBeginQuery(secondary2.handle(), query_pool, 0, 0);
    vk::CmdExecuteCommands(secondary2.handle(), 1u, &secondary1.handle());
    vk::CmdEndQuery(secondary2.handle(), query_pool, 0);
    secondary2.End();
}

TEST_F(PositiveSecondaryCommandBuffer, NestedPrimary) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7090");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nestedCommandBuffer);
    AddRequiredFeature(vkt::Feature::nestedCommandBufferRendering);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::inheritedQueries);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceNestedCommandBufferPropertiesEXT nested_cb_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(nested_cb_props);
    if (nested_cb_props.maxCommandBufferNestingLevel != 1) {
        GTEST_SKIP() << "needs maxCommandBufferNestingLevel to be 1";
    }

    vkt::CommandBuffer secondary1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceRenderingInfo cbiri = vku::InitStructHelper();
    cbiri.colorAttachmentCount = 1;
    cbiri.pColorAttachmentFormats = &m_render_target_fmt;
    cbiri.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper(&cbiri);
    cbii.occlusionQueryEnable = VK_TRUE;
    cbii.queryFlags = 0;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;

    secondary1.Begin(&cbbi);
    secondary1.End();

    secondary2.Begin(&cbbi);
    vk::CmdExecuteCommands(secondary2.handle(), 1u, &secondary1.handle());
    secondary2.End();

    // The primary command buffer doesn't count toward nesting
    m_command_buffer.Begin(&cbbi);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &secondary2.handle());
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
