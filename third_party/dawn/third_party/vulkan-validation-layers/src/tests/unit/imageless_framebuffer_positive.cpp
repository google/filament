/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/render_pass_helper.h"

class PositiveImagelessFramebuffer : public VkLayerTest {};

TEST_F(PositiveImagelessFramebuffer, BasicUsage) {
    TEST_DESCRIPTION("Create an imageless framebuffer");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo fb_attachment_image_info = vku::InitStructHelper();
    fb_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    fb_attachment_image_info.width = attachment_width;
    fb_attachment_image_info.height = attachment_height;
    fb_attachment_image_info.layerCount = 1;
    fb_attachment_image_info.viewFormatCount = 1;
    fb_attachment_image_info.pViewFormats = &format;
    VkFramebufferAttachmentsCreateInfo fb_attachment_ci = vku::InitStructHelper();
    fb_attachment_ci.attachmentImageInfoCount = 1;
    fb_attachment_ci.pAttachmentImageInfos = &fb_attachment_image_info;
    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper(&fb_attachment_ci);
    fb_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_ci.width = attachment_width;
    fb_ci.height = attachment_height;
    fb_ci.layers = 1;
    fb_ci.renderPass = rp.Handle();
    fb_ci.attachmentCount = 1;

    fb_ci.pAttachments  = nullptr;
    vkt::Framebuffer framebuffer_null(*m_device, fb_ci);

    vkt::ImageView rt_view = m_renderTargets[0]->CreateView();
    VkImageView image_views[2] = {rt_view, CastToHandle<VkImageView, uintptr_t>(0xbaadbeef)};
    fb_ci.pAttachments = image_views;
    vkt::Framebuffer framebuffer_bad_image_view(*m_device, fb_ci);
}

TEST_F(PositiveImagelessFramebuffer, Image3D) {
    TEST_DESCRIPTION("Create imageless framebuffer with image view from 3D image.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());

    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_portability_subset enabled - requires imageView2DOn3DImage to be VK_TRUE.\n";
    }

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = format;
    image_ci.extent = {32, 32, 4};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView imageView = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 4);

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info = vku::InitStructHelper();
    framebuffer_attachment_image_info.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info.width = 32;
    framebuffer_attachment_image_info.height = 32;
    framebuffer_attachment_image_info.layerCount = 4;
    framebuffer_attachment_image_info.viewFormatCount = 1;
    framebuffer_attachment_image_info.pViewFormats = &format;

    VkFramebufferAttachmentsCreateInfo framebuffer_attachments = vku::InitStructHelper();
    framebuffer_attachments.attachmentImageInfoCount = 1;
    framebuffer_attachments.pAttachmentImageInfos = &framebuffer_attachment_image_info;

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper(&framebuffer_attachments);
    framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebuffer_ci.renderPass = rp.Handle();
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments = &imageView.handle();
    framebuffer_ci.width = 32;
    framebuffer_ci.height = 32;
    framebuffer_ci.layers = 1;

    vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);

    VkClearValue clear_value = {};
    clear_value.color = {{0u, 0u, 0u, 0u}};

    VkRenderPassAttachmentBeginInfo render_pass_attachment_bi = vku::InitStructHelper();
    render_pass_attachment_bi.attachmentCount = 1;
    render_pass_attachment_bi.pAttachments = &imageView.handle();

    VkRenderPassBeginInfo render_pass_bi = vku::InitStructHelper(&render_pass_attachment_bi);
    render_pass_bi.renderPass = rp.Handle();
    render_pass_bi.framebuffer = framebuffer.handle();
    render_pass_bi.renderArea.extent = {1, 1};
    render_pass_bi.clearValueCount = 1;
    render_pass_bi.pClearValues = &clear_value;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass_bi);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveImagelessFramebuffer, SecondaryCmdBuffer) {
    TEST_DESCRIPTION("Use an imageless framebuffer in a secondary command buffer");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachment_width = 512;
    uint32_t attachment_height = 512;
    VkFormat format = FindSupportedDepthOnlyFormat(Gpu());

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddDepthStencilAttachment(0);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo fb_attachment_image_info = vku::InitStructHelper();
    fb_attachment_image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    fb_attachment_image_info.width = attachment_width;
    fb_attachment_image_info.height = attachment_height;
    fb_attachment_image_info.layerCount = 1;
    fb_attachment_image_info.viewFormatCount = 1;
    fb_attachment_image_info.pViewFormats = &format;

    VkFramebufferAttachmentsCreateInfo fb_attachment_ci = vku::InitStructHelper();
    fb_attachment_ci.attachmentImageInfoCount = 1;
    fb_attachment_ci.pAttachmentImageInfos = &fb_attachment_image_info;

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper(&fb_attachment_ci);
    fb_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_ci.width = attachment_width;
    fb_ci.height = attachment_height;
    fb_ci.layers = 1;
    fb_ci.renderPass = rp.Handle();
    fb_ci.attachmentCount = 1;

    fb_ci.pAttachments = nullptr;
    vkt::Framebuffer framebuffer_null(*m_device, fb_ci);

    vkt::ImageView rt_view = m_renderTargets[0]->CreateView();
    VkImageView image_views[2] = {rt_view, CastToHandle<VkImageView, uintptr_t>(0xbaadbeef)};
    fb_ci.pAttachments = image_views;
    vkt::Framebuffer framebuffer_bad_image_view(*m_device, fb_ci);

    VkCommandBufferInheritanceInfo inheritanceInfo = vku::InitStructHelper();
    inheritanceInfo.renderPass = rp.Handle();
    inheritanceInfo.framebuffer = framebuffer_null.handle();

    VkCommandBufferBeginInfo beginInfo = vku::InitStructHelper();
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    VkClearRect clearRect;
    clearRect.rect = {{0, 0}, {32u, 32u}};
    clearRect.baseArrayLayer = 0u;
    clearRect.layerCount = 1u;

    VkClearAttachment clearAttachment;
    clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearAttachment.clearValue.color.float32[0] = 0.0f;
    clearAttachment.clearValue.color.float32[1] = 0.0f;
    clearAttachment.clearValue.color.float32[2] = 0.0f;
    clearAttachment.clearValue.color.float32[3] = 1.0f;
    clearAttachment.colorAttachment = 0;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin(&beginInfo);
    vk::CmdClearAttachments(secondary.handle(), 1u, &clearAttachment, 1u, &clearRect);
    secondary.End();
}

TEST_F(PositiveImagelessFramebuffer, FragmentShadingRateDimensionsMultiview) {
    TEST_DESCRIPTION(
        "Specify a fragment shading rate attachment for imageless FB with different view masks and attachment layer "
        "configurations");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    // Render Pass with view mask (0x4)
    RenderPass2SingleSubpass rp_view_mask(*this);
    rp_view_mask.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp_view_mask.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp_view_mask.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp_view_mask.SetViewMask(0x4);
    rp_view_mask.CreateRenderPass();

    // Render Pass without view mask
    RenderPass2SingleSubpass rp_no_view_mask(*this);
    rp_no_view_mask.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp_no_view_mask.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp_no_view_mask.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp_no_view_mask.SetViewMask(0);
    rp_no_view_mask.CreateRenderPass();

    // Framebuffer attachment with layerCount == 2
    VkFormat viewFormat = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info_layer = vku::InitStructHelper();
    fbai_info_layer.usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    fbai_info_layer.width = 1;
    fbai_info_layer.height = 1;
    fbai_info_layer.layerCount = 2;
    fbai_info_layer.viewFormatCount = 1;
    fbai_info_layer.pViewFormats = &viewFormat;

    VkFramebufferAttachmentsCreateInfo fba_info_layer = vku::InitStructHelper();
    fba_info_layer.attachmentImageInfoCount = 1;
    fba_info_layer.pAttachmentImageInfos = &fbai_info_layer;

    // Framebuffer attachment with layerCount == 1
    VkFramebufferAttachmentImageInfo fbai_info_no_layer = vku::InitStructHelper();
    fbai_info_no_layer.usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    fbai_info_no_layer.width = 1;
    fbai_info_no_layer.height = 1;
    fbai_info_no_layer.layerCount = 1;
    fbai_info_no_layer.viewFormatCount = 1;
    fbai_info_no_layer.pViewFormats = &viewFormat;

    VkFramebufferAttachmentsCreateInfo fba_info_no_layer = vku::InitStructHelper();
    fba_info_no_layer.attachmentImageInfoCount = 1;
    fba_info_no_layer.pAttachmentImageInfos = &fbai_info_no_layer;

    {
        // Framebuffer with view mask > 0 and attachment layer count == 1
        VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info_no_layer);
        fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
        fb_info.renderPass = rp_view_mask.Handle();
        fb_info.attachmentCount = 1;
        fb_info.pAttachments = NULL;
        fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
        fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
        fb_info.layers = 1;

        vkt::Framebuffer fb(*m_device, fb_info);
    }

    {
        // Framebuffer with view mask == 0 and attachment layer count > 1
        VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info_layer);
        fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
        fb_info.renderPass = rp_no_view_mask.Handle();
        fb_info.attachmentCount = 1;
        fb_info.pAttachments = NULL;
        fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
        fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
        fb_info.layers = 1;

        vkt::Framebuffer fb(*m_device, fb_info);
    }

    {
        // Framebuffer with view mask == 0 and attachment layer count == 1
        VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info_no_layer);
        fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
        fb_info.renderPass = rp_no_view_mask.Handle();
        fb_info.attachmentCount = 1;
        fb_info.pAttachments = NULL;
        fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
        fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
        fb_info.layers = 1;

        vkt::Framebuffer fb(*m_device, fb_info);
    }
}
