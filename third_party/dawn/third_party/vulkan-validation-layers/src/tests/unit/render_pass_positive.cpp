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
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"

class PositiveRenderPass : public VkLayerTest {};

TEST_F(PositiveRenderPass, AttachmentUsedTwiceOK) {
    TEST_DESCRIPTION("Attachment is used simultaneously as color and input, with the same layout. This is OK.");

    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();
}

TEST_F(PositiveRenderPass, InitialLayoutUndefined) {
    TEST_DESCRIPTION(
        "Ensure that CmdBeginRenderPass with an attachment's initialLayout of VK_IMAGE_LAYOUT_UNDEFINED works when the command "
        "buffer has prior knowledge of that attachment's layout.");

    RETURN_IF_SKIP(Init());

    // A renderpass with one color attachment.
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    // A compatible framebuffer.
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &view.handle());

    // Record a single command buffer which uses this renderpass twice. The
    // bug is triggered at the beginning of the second renderpass, when the
    // command buffer already has a layout recorded for the attachment.
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32, 32);
    m_command_buffer.EndRenderPass();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32, 32);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, AttachmentLayoutWithLoadOpThenReadOnly) {
    TEST_DESCRIPTION(
        "Positive test where we create a renderpass with an attachment that uses LOAD_OP_CLEAR, the first subpass has a valid "
        "layout, and a second subpass then uses a valid *READ_ONLY* layout.");
    RETURN_IF_SKIP(Init());
    auto depth_format = FindSupportedDepthStencilFormat(Gpu());

    VkAttachmentReference attach[2] = {};
    attach[0].attachment = 0;
    attach[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attach[1].attachment = 0;
    attach[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    VkSubpassDescription subpasses[2] = {};
    // First subpass clears DS attach on load
    subpasses[0].pDepthStencilAttachment = &attach[0];
    // 2nd subpass reads in DS as input attachment
    subpasses[1].inputAttachmentCount = 1;
    subpasses[1].pInputAttachments = &attach[1];
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = depth_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;

    // Now create RenderPass and verify no errors
    vkt::RenderPass rp(*m_device, rpci);
}

TEST_F(PositiveRenderPass, BeginSubpassZeroTransitionsApplied) {
    TEST_DESCRIPTION("Ensure that CmdBeginRenderPass applies the layout transitions for the first subpass");

    RETURN_IF_SKIP(Init());

    // A renderpass with one color attachment.
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.AddSubpassDependency();
    rp.CreateRenderPass();

    // A compatible framebuffer.
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    vkt::ImageView view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &view.handle());

    // Record a single command buffer which issues a pipeline barrier w/
    // image memory barrier for the attachment. This detects the previously
    // missing tracking of the subpass layout by throwing a validation error
    // if it doesn't occur.
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32, 32);

    image.ImageMemoryBarrier(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, BeginTransitionsAttachmentUnused) {
    TEST_DESCRIPTION(
        "Ensure that layout transitions work correctly without errors, when an attachment reference is VK_ATTACHMENT_UNUSED");

    RETURN_IF_SKIP(Init());

    // A renderpass with no attachments
    VkAttachmentReference att_ref = {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &subpass, 0, nullptr};
    vkt::RenderPass rp(*m_device, rpci);

    // A compatible framebuffer.
    vkt::Framebuffer fb(*m_device, rp.handle(), 0, nullptr);

    // Record a command buffer which just begins and ends the renderpass. The
    // bug manifests in BeginRenderPass.
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.handle(), fb.handle(), 32, 32);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, BeginStencilLoadOp) {
    TEST_DESCRIPTION("Create a stencil-only attachment with a LOAD_OP set to CLEAR. stencil[Load|Store]Op used to be ignored.");
    RETURN_IF_SKIP(Init());
    VkFormat depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());
    VkImageFormatProperties formatProps;
    vk::GetPhysicalDeviceImageFormatProperties(Gpu(), depth_stencil_fmt, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0,
                                               &formatProps);
    if (formatProps.maxExtent.width < 100 || formatProps.maxExtent.height < 100) {
        GTEST_SKIP() << "Image format max extent is too small";
    }

    m_depthStencil->Init(*m_device, 100, 100, 1, depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(depth_stencil_fmt, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
    rp.AddDepthStencilAttachment(0);
    rp.CreateRenderPass();

    VkClearValue clear;
    clear.depthStencil.depth = 1.0;
    clear.depthStencil.stencil = 0;

    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &depth_image_view.handle(), 100, 100);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 100, 100, 1, &clear);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    vkt::Image destImage(*m_device, 100, 100, 1, depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::CommandBuffer cmdbuf(*m_device, m_command_pool);
    cmdbuf.Begin();

    m_depthStencil->ImageMemoryBarrier(cmdbuf, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                       VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                       VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    destImage.ImageMemoryBarrier(cmdbuf, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                 VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, 0,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkImageCopy cregion;
    cregion.srcSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    cregion.srcOffset = {0, 0, 0};
    cregion.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    cregion.dstOffset = {0, 0, 0};
    cregion.extent = {100, 100, 1};
    vk::CmdCopyImage(cmdbuf.handle(), m_depthStencil->handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destImage.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cregion);
    cmdbuf.End();
    m_default_queue->Submit(cmdbuf);
    m_default_queue->Wait();
}

TEST_F(PositiveRenderPass, BeginInlineAndSecondaryCommandBuffers) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();

    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_command_buffer.EndRenderPass();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_command_buffer.EndRenderPass();

    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, BeginDepthStencilLayoutTransitionFromUndefined) {
    TEST_DESCRIPTION(
        "Create a render pass with depth-stencil attachment where layout transition from UNDEFINED TO DS_READ_ONLY_OPTIMAL is set "
        "by render pass and verify that transition has correctly occurred at queue submit time with no validation errors.");

    RETURN_IF_SKIP(Init());
    auto depth_format = FindSupportedDepthStencilFormat(Gpu());
    VkImageFormatProperties format_props;
    vk::GetPhysicalDeviceImageFormatProperties(Gpu(), depth_format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, &format_props);
    if (format_props.maxExtent.width < 32 || format_props.maxExtent.height < 32) {
        GTEST_SKIP() << "Depth extent too small";
    }

    InitRenderTarget();

    // A renderpass with one depth/stencil attachment.
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
    rp.AddDepthStencilAttachment(0);
    rp.CreateRenderPass();

    // A compatible ds image.
    vkt::Image image(*m_device, 32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &view.handle());

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32, 32);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveRenderPass, DestroyPipeline) {
    TEST_DESCRIPTION("Draw using a pipeline whose create renderPass has been destroyed.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create a renderPass that's compatible with Draw-time renderPass
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(m_render_target_fmt, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // Destroy renderPass before pipeline is used in Draw
    //  We delay until after CmdBindPipeline to verify that invalid binding isn't
    //  created between CB & renderPass, which we used to do.
    rp.Destroy();
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveRenderPass, ImagelessFramebufferNonZeroBaseMip) {
    TEST_DESCRIPTION("Use a 1D image view for an imageless framebuffer with base mip level > 0.");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());

    constexpr uint32_t width = 512;
    constexpr uint32_t height = 1;
    VkFormat formats[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};
    VkFormat fb_attachments[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};
    constexpr uint32_t base_mip = 1;

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(formats[0]);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo fb_attachment_image_info = vku::InitStructHelper();
    fb_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    fb_attachment_image_info.width = width;
    fb_attachment_image_info.height = height;
    fb_attachment_image_info.layerCount = 1;
    fb_attachment_image_info.viewFormatCount = 2;
    fb_attachment_image_info.pViewFormats = fb_attachments;
    fb_attachment_image_info.height = 1;
    fb_attachment_image_info.width = width >> base_mip;

    VkFramebufferAttachmentsCreateInfo fb_attachments_ci = vku::InitStructHelper();
    fb_attachments_ci.attachmentImageInfoCount = 1;
    fb_attachments_ci.pAttachmentImageInfos = &fb_attachment_image_info;

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper(&fb_attachments_ci);
    fb_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_ci.width = width >> base_mip;
    fb_ci.height = height;
    fb_ci.layers = 1;
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = nullptr;
    fb_ci.renderPass = rp.Handle();
    vkt::Framebuffer fb(*m_device, fb_ci);
    ASSERT_TRUE(fb.initialized());

    auto image_ci = vkt::Image::ImageCreateInfo2D(width, 1, 2, 1, formats[0],
                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view_obj = image.CreateView(VK_IMAGE_VIEW_TYPE_1D_ARRAY, base_mip, 1, 0, 1);
    VkImageView image_view = image_view_obj.handle();

    VkRenderPassAttachmentBeginInfo rp_attachment_begin_info = vku::InitStructHelper();
    rp_attachment_begin_info.attachmentCount = 1;
    rp_attachment_begin_info.pAttachments = &image_view;
    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper(&rp_attachment_begin_info);
    rp_begin_info.renderPass = rp.Handle();
    rp_begin_info.renderArea.extent.width = width >> base_mip;
    rp_begin_info.renderArea.extent.height = height;
    rp_begin_info.framebuffer = fb.handle();

    VkCommandBufferBeginInfo cmd_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    m_command_buffer.Begin(&cmd_begin_info);
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

TEST_F(PositiveRenderPass, ValidStages) {
    TEST_DESCRIPTION("Create render pass with valid stages");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    VkSubpassDescription sci[2] = {};
    sci[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sci[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency dependency = {};
    // to be filled later by tests

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 2;
    rpci.pSubpasses = sci;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;

    const VkPipelineStageFlags kGraphicsStages =
        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    dependency.srcSubpass = 0;
    dependency.dstSubpass = 1;
    dependency.srcStageMask = kGraphicsStages;
    dependency.dstStageMask = kGraphicsStages;
    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported);

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = kGraphicsStages | VK_PIPELINE_STAGE_HOST_BIT;
    dependency.dstStageMask = kGraphicsStages;
    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported);

    dependency.srcSubpass = 0;
    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = kGraphicsStages;
    dependency.dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported);
}

TEST_F(PositiveRenderPass, SingleMipTransition) {
    TEST_DESCRIPTION("Ensure that the validation message contains the correct miplevel");

    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentDescription(VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.AddDepthStencilAttachment(1);
    rp.AddSubpassDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                            VK_ACCESS_SHADER_READ_BIT);
    rp.CreateRenderPass();

    // Create Framebuffer.
    vkt::Image colorImage(*m_device, 32, 32, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    vkt::Image depthImage(*m_device, 32, 32, 2, VK_FORMAT_D32_SFLOAT,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    vkt::ImageView color_view = colorImage.CreateView(VK_IMAGE_VIEW_TYPE_2D, /*baseMipLevel*/ 0, /*levelCount*/ 1);
    vkt::ImageView depth_view = depthImage.CreateView(VK_IMAGE_VIEW_TYPE_2D, /*baseMipLevel*/ 0, /*levelCount*/ 1, 0,
                                                      VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkImageView baseViews[] = {color_view, depth_view};

    VkImageViewCreateInfo vinfo = {};
    vinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vinfo.components.r = VK_COMPONENT_SWIZZLE_R;
    vinfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vinfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vinfo.components.a = VK_COMPONENT_SWIZZLE_A;
    vinfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};

    vinfo.image = colorImage.handle();
    vinfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    vinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkt::ImageView fullView0(*m_device, vinfo);

    vinfo.image = depthImage.handle();
    vinfo.format = VK_FORMAT_D32_SFLOAT;
    vinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    vkt::ImageView fullView1(*m_device, vinfo);

    VkImageView fullViews[] = {fullView0.handle(), fullView1.handle()};

    vkt::Framebuffer fb(*m_device, rp.Handle(), 2, baseViews);

    // Create shader modules

    char const fsSource[] = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(set=0, binding=2) uniform sampler2D depth;
        void main() {
           x = texture(depth, vec2(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Create descriptor set and friends.
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    std::vector<VkDescriptorSetLayoutBinding> binding_defs = {
        {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    const vkt::DescriptorSetLayout pipeline_dsl(*m_device, binding_defs);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&pipeline_dsl});
    OneOffDescriptorSet descriptor_set(m_device, binding_defs);

    descriptor_set.WriteDescriptorImageInfo(2, fullViews[1], sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    descriptor_set.UpdateDescriptorSets();

    VkPipelineDepthStencilStateCreateInfo ds_ci = {};
    ds_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_LESS;

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.ds_ci_ = ds_ci;
    pipe.CreateGraphicsPipeline();

    // Start pushing commands.

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32, 32);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();

    // At this point the first miplevel should be in GENERAL due to the "finalLayout" in the render pass.
    // Note that these image barriers attempt to transition *all* miplevels, even though only 1 miplevel has transitioned.

    colorImage.Layout(VK_IMAGE_LAYOUT_GENERAL);

    colorImage.ImageMemoryBarrier(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    depthImage.Layout(VK_IMAGE_LAYOUT_GENERAL);

    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, ViewMasks) {
    TEST_DESCRIPTION("Create render pass with view mask, with multiview feature enabled in Vulkan11Features.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.viewMask = 0x1;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);
}

TEST_F(PositiveRenderPass, BeginWithViewMasks) {
    TEST_DESCRIPTION("Begin render pass with view mask and a push descriptor.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    std::array subpasses = {vku::InitStruct<VkSubpassDescription2>(), vku::InitStruct<VkSubpassDescription2>()};
    subpasses[0].viewMask = 0x1;
    subpasses[1].viewMask = 0x1;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.subpassCount = subpasses.size();
    render_pass_ci.pSubpasses = subpasses.data();
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    // A compatible framebuffer.
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();
    vkt::Framebuffer fb(*m_device, render_pass.handle(), 1, &view.handle());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});
    // Create push descriptor set layout
    const vkt::DescriptorSetLayout push_ds_layout(*m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);

    // Use helper to create graphics pipeline
    CreatePipelineHelper helper(*this);
    helper.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&push_ds_layout, &ds_layout});
    helper.gp_ci_.renderPass = render_pass.handle();
    helper.CreateGraphicsPipeline();

    const uint32_t data_size = sizeof(float) * 3;
    vkt::Buffer vbo(*m_device, data_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buff_info;
    buff_info.buffer = vbo.handle();
    buff_info.offset = 0;
    buff_info.range = data_size;
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = 0;  // Should not cause a validation error

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.Handle());
    vk::CmdPushDescriptorSetKHR(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_layout_.handle(), 0, 1,
                                &descriptor_write);
    m_command_buffer.BeginRenderPass(render_pass.handle(), fb.handle(), 32, 32);
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, BeginDedicatedStencilLayout) {
    TEST_DESCRIPTION("Render pass using a dedicated stencil layout, different from depth layout");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::separateDepthStencilLayouts);
    RETURN_IF_SKIP(Init());

    // Create depth stencil image
    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());

    vkt::Image ds_image(*m_device, 32, 32, 1, ds_format,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    vkt::ImageView ds_view = ds_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    VkAttachmentDescriptionStencilLayout attachment_desc_stencil_layout = vku::InitStructHelper();
    attachment_desc_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReferenceStencilLayout attachment_ref_stencil_layout = vku::InitStructHelper();
    attachment_ref_stencil_layout.stencilLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(ds_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    rp.SetAttachmentDescriptionPNext(0, &attachment_desc_stencil_layout);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 0, &attachment_ref_stencil_layout);
    rp.AddDepthStencilAttachment(0);
    rp.CreateRenderPass();

    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &ds_view.handle(), ds_image.Width(), ds_image.Height());

    // Use helper to create graphics pipeline
    VkPipelineDepthStencilStateCreateInfo ds_state = vku::InitStructHelper();
    // One stencil op is not OP_KEEP, both write mask are not 0
    ds_state.front.failOp = VK_STENCIL_OP_ZERO;
    ds_state.front.writeMask = 0x1;
    ds_state.back.writeMask = 0x1;
    CreatePipelineHelper helper(*this);
    helper.gp_ci_.pDepthStencilState = &ds_state;
    helper.gp_ci_.renderPass = rp.Handle();
    helper.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), ds_image.Width(), ds_image.Height());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.Handle());
    // If the stencil layout was not specified separately using the separateDepthStencilLayouts feature,
    // and used in the validation code, 06887 would trigger with the following draw call
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, QueriesInMultiview) {
    TEST_DESCRIPTION("Use queries in a render pass instance with multiview enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);

    uint32_t viewMasks[] = {0x3u};
    uint32_t correlationMasks[] = {0x1u};
    VkRenderPassMultiviewCreateInfo rpmvci = vku::InitStructHelper();
    rpmvci.subpassCount = 1;
    rpmvci.pViewMasks = viewMasks;
    rpmvci.correlationMaskCount = 1;
    rpmvci.pCorrelationMasks = correlationMasks;

    rp.CreateRenderPass(&rpmvci);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 2, 3, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 3);
    VkImageView image_view_handle = view.handle();

    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &image_view_handle);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 2);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 2);

    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32, 32);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.EndRenderPass();

    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 2, buffer.handle(), 0, 4, 0);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveRenderPass, StoreOpNoneExt) {
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_NONE);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();
}

TEST_F(PositiveRenderPass, FramebufferCreateDepthStencilLayoutTransitionForDepthOnlyImageView) {
    TEST_DESCRIPTION(
        "Validate that when an imageView of a depth/stencil image is used as a depth/stencil framebuffer attachment, the "
        "aspectMask is ignored and both depth and stencil image subresources are used.");

    RETURN_IF_SKIP(Init());
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        GTEST_SKIP() << "Image format does not support sampling";
    }

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
    rp.AddDepthStencilAttachment(0);
    rp.AddSubpassDependency();
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_D32_SFLOAT_S8_UINT, 0x26 /* usage */);
    image.SetLayout(0x6, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    vkt::ImageView view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &view.handle());

    m_command_buffer.Begin();

    VkImageMemoryBarrier imb = vku::InitStructHelper();
    imb.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imb.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    imb.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imb.srcQueueFamilyIndex = 0;
    imb.dstQueueFamilyIndex = 0;
    imb.image = image.handle();
    imb.subresourceRange.aspectMask = 0x6;
    imb.subresourceRange.baseMipLevel = 0;
    imb.subresourceRange.levelCount = 0x1;
    imb.subresourceRange.baseArrayLayer = 0;
    imb.subresourceRange.layerCount = 0x1;

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imb);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveRenderPass, FramebufferWithAttachmentsTo3DImageMultipleSubpasses) {
    TEST_DESCRIPTION(
        "Test no false overlap is reported with multi attachment framebuffer (attachments are slices of a 3D image). Multiple "
        "subpasses that draw to a single slice of a 3D image");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    constexpr unsigned depth_count = 2u;

    // 3D image with 2 depths
    VkImageCreateInfo image_info = vku::InitStructHelper();
    image_info.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_info.imageType = VK_IMAGE_TYPE_3D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent = {64, 64, depth_count};
    image_info.mipLevels = 1u;
    image_info.arrayLayers = 1u;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkt::Image image_3d(*m_device, image_info, vkt::set_layout);
    image_3d.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // 2D image views to be used as color attchments for framebuffer
    VkImageViewCreateInfo view_info = vku::InitStructHelper();
    view_info.image = image_3d.handle();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = image_info.format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView image_views[depth_count];
    VkImageView views[depth_count] = {VK_NULL_HANDLE};
    for (unsigned i = 0; i < depth_count; ++i) {
        view_info.subresourceRange.baseArrayLayer = i;
        image_views[i].init(*m_device, view_info);
        views[i] = image_views[i].handle();
    }

    // Render pass with 2 subpasses
    VkAttachmentReference attach[depth_count] = {};
    VkSubpassDescription subpasses[depth_count] = {};

    for (unsigned i = 0; i < depth_count; ++i) {
        attach[i].attachment = i;
        attach[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpasses[i].pColorAttachments = &attach[i];
        subpasses[i].colorAttachmentCount = 1;
    }

    VkAttachmentDescription attach_desc[depth_count] = {};
    for (unsigned i = 0; i < depth_count; ++i) {
        attach_desc[i].format = image_info.format;
        attach_desc[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attach_desc[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attach_desc[i].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.subpassCount = depth_count;
    rp_info.pSubpasses = subpasses;
    rp_info.attachmentCount = depth_count;
    rp_info.pAttachments = attach_desc;

    vkt::RenderPass renderpass(*m_device, rp_info);
    vkt::Framebuffer framebuffer(*m_device, renderpass.handle(), depth_count, views, image_info.extent.width,
                                 image_info.extent.height);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(renderpass.handle(), framebuffer.handle(), image_info.extent.width, image_info.extent.height);
    for (unsigned i = 0; i < (depth_count - 1); ++i) {
        m_command_buffer.NextSubpass();
    }
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, ImageLayoutTransitionOf3dImageWith2dViews) {
    TEST_DESCRIPTION(
        "Test that transitioning the layout of a mip level of a 3D image using a view of one of its slice applies to the entire 3D "
        "image: all views referencing different slices of the same mip level should also see their layout transitioned");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
        GetPhysicalDeviceFeatures2(portability_subset_features);
        if (!portability_subset_features.imageView2DOn3DImage) {
            GTEST_SKIP() << "imageView2DOn3DImage not supported, skipping test";
        }
    }

    constexpr unsigned image_depth = 2u;

    // 3D image
    VkImageCreateInfo image_info = vku::InitStructHelper();
    image_info.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_info.imageType = VK_IMAGE_TYPE_3D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent = {64, 64, image_depth};
    image_info.mipLevels = 1u;
    image_info.arrayLayers = 1u;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkt::Image image_3d(*m_device, image_info, vkt::set_layout);
    image_3d.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 2D image views for each slice of the 3D image
    VkImageViewCreateInfo view_info = vku::InitStructHelper();
    view_info.image = image_3d.handle();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = image_info.format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView image_views[image_depth];
    VkImageView views[image_depth] = {VK_NULL_HANDLE};
    for (unsigned i = 0; i < image_depth; ++i) {
        view_info.subresourceRange.baseArrayLayer = i;
        image_views[i].init(*m_device, view_info);
        views[i] = image_views[i].handle();
    }

    // Render pass 1, referencing first slice
    RenderPassSingleSubpass rp_1(*this);
    rp_1.AddAttachmentDescription(image_info.format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp_1.AddAttachmentReference({0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    rp_1.AddInputAttachment(0);
    rp_1.CreateRenderPass();

    vkt::Framebuffer framebuffer_1(*m_device, rp_1.Handle(), 1, &views[0], image_info.extent.width, image_info.extent.height);

    // Render pass 2, referencing second slice
    RenderPassSingleSubpass rp_2(*this);
    // Since the previous render pass' framebuffer was using a 2D view of the first slice of the 3D image,
    // the layout transition should have applied to all the slices of the 3D image,
    // thus the 2nd image view created on the second slice of the 3D image should found its image layout to be
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    rp_2.AddAttachmentDescription(image_info.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp_2.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp_2.AddColorAttachment(0);
    rp_2.CreateRenderPass();

    vkt::Framebuffer framebuffer_2(*m_device, rp_2.Handle(), 1, &views[1], image_info.extent.width, image_info.extent.height);

    m_command_buffer.Begin();

    m_command_buffer.BeginRenderPass(rp_1.Handle(), framebuffer_1.handle(), image_info.extent.width, image_info.extent.height);
    m_command_buffer.EndRenderPass();

    m_command_buffer.BeginRenderPass(rp_2.Handle(), framebuffer_2.handle(), image_info.extent.width, image_info.extent.height);
    m_command_buffer.EndRenderPass();

    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, SubpassWithReadOnlyLayoutWithoutDependency) {
    TEST_DESCRIPTION("When both subpasses' attachments are the same and layouts are read-only, they don't need dependency.");
    RETURN_IF_SKIP(Init());

    auto depth_format = FindSupportedDepthStencilFormat(Gpu());

    // A renderpass with one color attachment.
    VkAttachmentDescription attachment = {0,
                                          depth_format,
                                          VK_SAMPLE_COUNT_1_BIT,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_STORE,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                          VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
    const int size = 2;
    std::array<VkAttachmentDescription, size> attachments = {{attachment, attachment}};

    VkAttachmentReference att_ref_depth_stencil = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};

    std::array<VkSubpassDescription, size> subpasses;
    subpasses[0] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 0, 0, nullptr, nullptr, &att_ref_depth_stencil, 0, nullptr};
    subpasses[1] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 0, 0, nullptr, nullptr, &att_ref_depth_stencil, 0, nullptr};

    VkRenderPassCreateInfo rpci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, size, attachments.data(), size, subpasses.data(), 0, nullptr};
    vkt::RenderPass rp(*m_device, rpci);

    // A compatible framebuffer.
    vkt::Image image(*m_device, 32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    std::array<VkImageView, size> views = {{view.handle(), view.handle()}};

    vkt::Framebuffer fb(*m_device, rp.handle(), size, views.data());

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.handle(), fb.handle(), 32, 32);
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, SeparateDepthStencilSubresourceLayout) {
    TEST_DESCRIPTION("Test that separate depth stencil layouts are tracked correctly.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::separateDepthStencilLayouts);
    RETURN_IF_SKIP(Init());

    VkFormat ds_format = VK_FORMAT_D24_UNORM_S8_UINT;
    VkFormatProperties props;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), ds_format, &props);
    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {
        ds_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        vk::GetPhysicalDeviceFormatProperties(Gpu(), ds_format, &props);
        ASSERT_TRUE((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0);
    }

    auto image_ci = vkt::Image::CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 64;
    image_ci.extent.height = 64;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 6;
    image_ci.format = ds_format;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image image(*m_device, image_ci);

    const auto depth_range = image.SubresourceRange(VK_IMAGE_ASPECT_DEPTH_BIT);
    const auto stencil_range = image.SubresourceRange(VK_IMAGE_ASPECT_STENCIL_BIT);
    const auto depth_stencil_range = image.SubresourceRange(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkImageViewCreateInfo view_info = vku::InitStructHelper();
    view_info.image = image.handle();
    view_info.subresourceRange = depth_stencil_range;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    view_info.format = ds_format;
    vkt::ImageView view(*m_device, view_info);

    std::vector<VkImageMemoryBarrier> barriers;

    {
        m_command_buffer.Begin();
        auto depth_barrier =
            image.ImageMemoryBarrier(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, depth_range);
        auto stencil_barrier =
            image.ImageMemoryBarrier(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, stencil_range);
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 1, &depth_barrier);
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 1, &stencil_barrier);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_command_buffer.Reset();
    }

    m_command_buffer.Begin();

    // Test that we handle initial layout in command buffer.
    barriers.push_back(image.ImageMemoryBarrier(0, 0, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depth_stencil_range));

    // Test that we can transition aspects separately and use specific layouts.
    barriers.push_back(image.ImageMemoryBarrier(0, 0, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                                VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, depth_range));

    barriers.push_back(image.ImageMemoryBarrier(0, 0, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                                                VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, stencil_range));

    // Test that transition from UNDEFINED on depth aspect does not clobber stencil layout.
    barriers.push_back(
        image.ImageMemoryBarrier(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, depth_range));

    // Test that we can transition aspects separately and use combined layouts. (Only care about the aspect in question).
    barriers.push_back(image.ImageMemoryBarrier(0, 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, depth_range));

    barriers.push_back(image.ImageMemoryBarrier(0, 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, stencil_range));

    // Test that we can transition back again with combined layout.
    barriers.push_back(image.ImageMemoryBarrier(0, 0, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depth_stencil_range));

    VkRenderPassCreateInfo2 rp2 = vku::InitStructHelper();
    VkAttachmentDescription2 desc = vku::InitStructHelper();
    VkSubpassDescription2 sub = vku::InitStructHelper();
    VkAttachmentReference2 att = vku::InitStructHelper();
    VkAttachmentDescriptionStencilLayout stencil_desc = vku::InitStructHelper();
    VkAttachmentReferenceStencilLayout stencil_att = vku::InitStructHelper();
    // Test that we can discard stencil layout.
    stencil_desc.stencilInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    stencil_desc.stencilFinalLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
    stencil_att.stencilLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

    desc.format = ds_format;
    desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    desc.samples = VK_SAMPLE_COUNT_1_BIT;
    desc.pNext = &stencil_desc;

    att.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    att.attachment = 0;
    att.pNext = &stencil_att;

    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.pDepthStencilAttachment = &att;
    rp2.subpassCount = 1;
    rp2.pSubpasses = &sub;
    rp2.attachmentCount = 1;
    rp2.pAttachments = &desc;
    vkt::RenderPass render_pass_separate(*m_device, rp2);

    desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    desc.finalLayout = desc.initialLayout;
    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.pNext = nullptr;
    att.layout = desc.initialLayout;
    att.pNext = nullptr;
    vkt::RenderPass render_pass_combined(*m_device, rp2);

    vkt::Framebuffer framebuffer_separate(*m_device, render_pass_separate.handle(), 1, &view.handle(), 1, 1);
    vkt::Framebuffer framebuffer_combined(*m_device, render_pass_combined.handle(), 1, &view.handle(), 1, 1);

    for (auto &barrier : barriers) {
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                               0, nullptr, 0, nullptr, 1, &barrier);
    }

    m_command_buffer.BeginRenderPass(render_pass_separate.handle(), framebuffer_separate.handle());
    m_command_buffer.EndRenderPass();

    m_command_buffer.BeginRenderPass(render_pass_combined.handle(), framebuffer_combined.handle());
    m_command_buffer.EndRenderPass();

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveRenderPass, InputResolve) {
    TEST_DESCRIPTION("Create render pass where input attachment == resolve attachment");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    RenderPassSingleSubpass rp(*this);
    // input attachments
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    // color attachments
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    // resolve attachment
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddInputAttachment(0);
    rp.AddColorAttachment(1);
    rp.AddResolveAttachment(2);

    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rp.GetCreateInfo(), rp2Supported);
}

TEST_F(PositiveRenderPass, TestDepthStencilRenderPassTransition) {
    TEST_DESCRIPTION(
        "Create framebuffer with a depth/stencil attachment that has only depth or stencil aspect and transition it in render "
        "pass. Then create a barrier on both aspect, for this to be valid *both* aspects must have been correctly tracked as "
        "transitioned to the new layout.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    RETURN_IF_SKIP(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    const VkFormat ds_format = FindSupportedDepthStencilFormat(m_device->Physical());
    vkt::Image depthImage(*m_device, 32, 32, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    for (size_t i = 0; i < 2; i++) {
        const vkt::ImageView depth_or_stencil_view(
            *m_device, depthImage.BasicViewCreatInfo(i == 0 ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_STENCIL_BIT));

        RenderPassSingleSubpass rp(*this);
        rp.AddAttachmentDescription(ds_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
        rp.AddDepthStencilAttachment(0);
        rp.CreateRenderPass();

        const vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &depth_or_stencil_view.handle());

        m_command_buffer.Begin();

        m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32, 32);
        m_command_buffer.EndRenderPass();

        VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
        img_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        img_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.image = depthImage.handle();
        img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        img_barrier.subresourceRange.layerCount = 1;
        img_barrier.subresourceRange.levelCount = 1;

        vk::CmdPipelineBarrier(m_command_buffer.handle(),
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &img_barrier);
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }
}

TEST_F(PositiveRenderPass, BeginRenderPassWithRenderPassStriped) {
    TEST_DESCRIPTION("Test to validate renderpass with VK_ARM_render_pass_striped.");
    AddRequiredExtensions(VK_ARM_RENDER_PASS_STRIPED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::renderPassStriped);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceRenderPassStripedPropertiesARM rp_striped_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(rp_striped_props);

    const uint32_t stripe_width = rp_striped_props.renderPassStripeGranularity.width;
    const uint32_t stripe_height = rp_striped_props.renderPassStripeGranularity.height;

    const uint32_t stripe_count = 4;
    std::vector<VkRenderPassStripeInfoARM> stripe_infos(stripe_count);
    for (uint32_t i = 0; i < stripe_count; ++i) {
        stripe_infos[i] = vku::InitStructHelper();
        stripe_infos[i].stripeArea.offset.x = stripe_width * i;
        stripe_infos[i].stripeArea.offset.y = 0;
        stripe_infos[i].stripeArea.extent.width = stripe_width;
        stripe_infos[i].stripeArea.extent.height = stripe_height;
    }

    VkRenderPassStripeBeginInfoARM rp_stripes_info = vku::InitStructHelper();
    rp_stripes_info.stripeInfoCount = stripe_count;
    rp_stripes_info.pStripeInfos = stripe_infos.data();

    m_renderPassBeginInfo.pNext = &rp_stripes_info;
    m_renderPassBeginInfo.renderArea = {{0, 0}, {stripe_width * stripe_count, stripe_height}};

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buffer(*m_device, command_pool);

    VkCommandBufferBeginInfo cmd_begin = vku::InitStructHelper();
    cmd_buffer.Begin(&cmd_begin);
    cmd_buffer.BeginRenderPass(m_renderPassBeginInfo);
    cmd_buffer.EndRenderPass();
    cmd_buffer.End();

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper();
    vkt::Semaphore semaphores[stripe_count];
    VkSemaphoreSubmitInfo semaphore_submit_infos[stripe_count];

    for (uint32_t i = 0; i < stripe_count; ++i) {
        semaphores[i].init(*m_device, semaphore_create_info);
        semaphore_submit_infos[i] = vku::InitStructHelper();
        semaphore_submit_infos[i].semaphore = semaphores[i].handle();
    }

    VkRenderPassStripeSubmitInfoARM rp_stripe_submit_info = vku::InitStructHelper();
    rp_stripe_submit_info.stripeSemaphoreInfoCount = stripe_count;
    rp_stripe_submit_info.pStripeSemaphoreInfos = semaphore_submit_infos;

    VkCommandBufferSubmitInfo cb_submit_info = vku::InitStructHelper(&rp_stripe_submit_info);
    cb_submit_info.commandBuffer = cmd_buffer.handle();

    VkSubmitInfo2 submit_info = vku::InitStructHelper();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_submit_info;
    vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_default_queue->Wait();
}

TEST_F(PositiveRenderPass, NestedCommandBuffersFeatureMaintenance7) {
    TEST_DESCRIPTION(
        "Use VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR with maintenance7, but not nextedCommandBuffers is not "
        "enabled");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_7_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance7);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo,
                           VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR);
    vk::CmdEndRenderPass(m_command_buffer.handle());
    m_command_buffer.End();
}

TEST_F(PositiveRenderPass, RenderPassSampleLocationsBeginInfo) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8388");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceSampleLocationsPropertiesEXT sample_location_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(sample_location_properties);
    if (sample_location_properties.variableSampleLocations) {
        GTEST_SKIP() << "variableSampleLocations must not be supported";
    }
    if ((sample_location_properties.sampleLocationSampleCounts & VK_SAMPLE_COUNT_1_BIT) == 0) {
        GTEST_SKIP() << "sampleLocationSampleCounts does not contain VK_SAMPLE_COUNT_1_BIT";
    }

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, rpci);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    vkt::Framebuffer framebuffer(*m_device, render_pass.handle(), 1, &image_view.handle());

    VkSampleLocationEXT sample_location = {0.5f, 0.5f};
    VkSampleLocationsInfoEXT sample_locations_info = vku::InitStructHelper();
    sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
    sample_locations_info.sampleLocationGridSize = {1u, 1u};
    sample_locations_info.sampleLocationsCount = 1u;
    sample_locations_info.pSampleLocations = &sample_location;

    VkPipelineSampleLocationsStateCreateInfoEXT sample_locations_state = vku::InitStructHelper();
    sample_locations_state.sampleLocationsEnable = VK_TRUE;
    sample_locations_state.sampleLocationsInfo = sample_locations_info;

    VkPipelineMultisampleStateCreateInfo multi_sample_state = vku::InitStructHelper(&sample_locations_state);
    multi_sample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multi_sample_state.sampleShadingEnable = VK_FALSE;
    multi_sample_state.minSampleShading = 1.0;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pMultisampleState = &multi_sample_state;
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.CreateGraphicsPipeline();

    VkClearValue color_clear_value;
    color_clear_value.color.float32[0] = 0.0f;
    color_clear_value.color.float32[1] = 0.0f;
    color_clear_value.color.float32[2] = 0.0f;
    color_clear_value.color.float32[3] = 1.0f;

    VkSampleLocationsInfoEXT sample_loc_info = vku::InitStructHelper();
    sample_loc_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
    sample_loc_info.sampleLocationGridSize = {1u, 1u};
    sample_loc_info.sampleLocationsCount = 1u;
    sample_loc_info.pSampleLocations = &sample_location;

    VkAttachmentSampleLocationsEXT attachment_sample_loc;
    attachment_sample_loc.attachmentIndex = 0u;
    attachment_sample_loc.sampleLocationsInfo = sample_loc_info;

    VkSubpassSampleLocationsEXT subpass_sample_loc;
    subpass_sample_loc.subpassIndex = 0u;
    subpass_sample_loc.sampleLocationsInfo = sample_loc_info;

    VkRenderPassSampleLocationsBeginInfoEXT sample_locations_begin_info = vku::InitStructHelper();
    sample_locations_begin_info.attachmentInitialSampleLocationsCount = 1u;
    sample_locations_begin_info.pAttachmentInitialSampleLocations = &attachment_sample_loc;
    sample_locations_begin_info.postSubpassSampleLocationsCount = 1u;
    sample_locations_begin_info.pPostSubpassSampleLocations = &subpass_sample_loc;

    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper(&sample_locations_begin_info);
    render_pass_begin_info.renderPass = render_pass.handle();
    render_pass_begin_info.framebuffer = framebuffer.handle();
    render_pass_begin_info.renderArea.extent = {32, 32};
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &color_clear_value;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass_begin_info);

    sample_location.x = 1.0f;

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}