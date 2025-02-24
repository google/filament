/*
 * Copyright (c) 2023-2024 The Khronos Group Inc.
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
#include "../framework/android_hardware_buffer.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)

void AndroidExternalResolveTest::InitBasicAndroidExternalResolve() {
    SetTargetApiVersion(VK_API_VERSION_1_2); // for RenderPass2
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_FORMAT_RESOLVE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::externalFormatResolve);
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceExternalFormatResolvePropertiesANDROID external_format_resolve_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(external_format_resolve_props);
    nullColorAttachmentWithExternalFormatResolve = external_format_resolve_props.nullColorAttachmentWithExternalFormatResolve;
}

class PositiveAndroidExternalResolve : public AndroidExternalResolveTest {};

TEST_F(PositiveAndroidExternalResolve, NoResolve) {
    TEST_DESCRIPTION("Make sure enabling the feature doesn't break normal usage of API.");
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveAndroidExternalResolve, RenderPassAndFramebuffer) {
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkAndroidHardwareBufferFormatResolvePropertiesANDROID format_resolve_prop = vku::InitStructHelper();

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device, &format_resolve_prop);

    // index 0 = color | index 1 = resolve
    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(format_resolve_prop.colorAttachmentFormat);
    rp.AddAttachmentDescription(VK_FORMAT_UNDEFINED);
    rp.SetAttachmentDescriptionPNext(0, &external_format);
    rp.SetAttachmentDescriptionPNext(1, &external_format);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_PLANE_0_BIT);
    rp.AddAttachmentReference(1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);
    rp.CreateRenderPass();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_resolve_prop.colorAttachmentFormat;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    vkt::Image color_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView color_view = color_image.CreateView();

    image_ci.pNext = &external_format;
    image_ci.format = VK_FORMAT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image resolve_image(*m_device, image_ci, vkt::set_layout);

    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper(&external_format);
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    sycci.components = {VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    vkt::SamplerYcbcrConversion ycbcr_conv(*m_device, sycci);

    VkSamplerYcbcrConversionInfo syci = vku::InitStructHelper();
    syci.conversion = ycbcr_conv.handle();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&syci);
    ivci.image = resolve_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    const vkt::ImageView resolve_view(*m_device, ivci);

    VkImageView attachments[2];
    attachments[0] = color_view.handle();
    attachments[1] = resolve_view.handle();

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 2, attachments);

    CreatePipelineHelper pipe(*this, &external_format);
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveAndroidExternalResolve, ImagelessFramebuffer) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkAndroidHardwareBufferFormatResolvePropertiesANDROID format_resolve_prop = vku::InitStructHelper();

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device, &format_resolve_prop);

    // index 0 = color | index 1 = resolve
    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(format_resolve_prop.colorAttachmentFormat);
    rp.AddAttachmentDescription(VK_FORMAT_UNDEFINED);
    rp.SetAttachmentDescriptionPNext(0, &external_format);
    rp.SetAttachmentDescriptionPNext(1, &external_format);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_PLANE_0_BIT);
    rp.AddAttachmentReference(1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);
    rp.CreateRenderPass();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_resolve_prop.colorAttachmentFormat;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image color_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView color_view = color_image.CreateView();

    image_ci.pNext = &external_format;
    image_ci.format = VK_FORMAT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image resolve_image(*m_device, image_ci, vkt::set_layout);

    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper(&external_format);
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    sycci.components = {VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    vkt::SamplerYcbcrConversion ycbcr_conv(*m_device, sycci);

    VkSamplerYcbcrConversionInfo syci = vku::InitStructHelper();
    syci.conversion = ycbcr_conv.handle();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&syci);
    ivci.image = resolve_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    const vkt::ImageView resolve_view(*m_device, ivci);

    VkImageView attachments[2];
    attachments[0] = color_view.handle();
    attachments[1] = resolve_view.handle();

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info[2];
    framebuffer_attachment_image_info[0] = vku::InitStructHelper();
    framebuffer_attachment_image_info[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info[0].width = 32;
    framebuffer_attachment_image_info[0].height = 32;
    framebuffer_attachment_image_info[0].layerCount = 1;
    framebuffer_attachment_image_info[0].viewFormatCount = 1;
    framebuffer_attachment_image_info[0].pViewFormats = &format_resolve_prop.colorAttachmentFormat;

    framebuffer_attachment_image_info[1] = framebuffer_attachment_image_info[0];
    framebuffer_attachment_image_info[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info[1].pViewFormats = &ivci.format;

    VkFramebufferAttachmentsCreateInfo framebuffer_attachments = vku::InitStructHelper();
    framebuffer_attachments.attachmentImageInfoCount = 2;
    framebuffer_attachments.pAttachmentImageInfos = framebuffer_attachment_image_info;

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper(&framebuffer_attachments);
    fb_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_ci.width = 32;
    fb_ci.height = 32;
    fb_ci.layers = 1;
    fb_ci.attachmentCount = 2;
    fb_ci.renderPass = rp.Handle();
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkFramebufferCreateInfo-flags-03201");
    vkt::Framebuffer framebuffer(*m_device, fb_ci);

    VkClearValue clear_value = {};
    clear_value.color = {{0u, 0u, 0u, 0u}};

    VkRenderPassAttachmentBeginInfo render_pass_attachment_bi = vku::InitStructHelper();
    render_pass_attachment_bi.attachmentCount = 2;
    render_pass_attachment_bi.pAttachments = attachments;

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

TEST_F(PositiveAndroidExternalResolve, DynamicRendering) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkAndroidHardwareBufferFormatResolvePropertiesANDROID format_resolve_prop = vku::InitStructHelper();

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device, &format_resolve_prop);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_resolve_prop.colorAttachmentFormat;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image color_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView color_view = color_image.CreateView();

    image_ci.pNext = &external_format;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.format = VK_FORMAT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image resolve_image(*m_device, image_ci, vkt::set_layout);

    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper(&external_format);
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    sycci.components = {VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    vkt::SamplerYcbcrConversion ycbcr_conv(*m_device, sycci);

    VkSamplerYcbcrConversionInfo syci = vku::InitStructHelper();
    syci.conversion = ycbcr_conv.handle();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&syci);
    ivci.image = resolve_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    const vkt::ImageView resolve_view(*m_device, ivci);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view.handle();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID;
    color_attachment.resolveImageView = resolve_view.handle();
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {32, 32}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkRenderingAttachmentInfo-imageView-06865");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkRenderingAttachmentInfo-imageView-06129");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveAndroidExternalResolve, PipelineBarrier) {
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());

    // needed to set VK_ATTACHMENT_UNUSED
    if (!nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve not enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkAndroidHardwareBufferFormatResolvePropertiesANDROID format_resolve_prop = vku::InitStructHelper();

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device, &format_resolve_prop);

    // index 0 = color | index 1 = resolve
    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(format_resolve_prop.colorAttachmentFormat);
    rp.AddAttachmentDescription(VK_FORMAT_UNDEFINED);
    rp.SetAttachmentDescriptionPNext(0, &external_format);
    rp.SetAttachmentDescriptionPNext(1, &external_format);
    rp.AddAttachmentReference(VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_PLANE_0_BIT);
    rp.AddAttachmentReference(1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);
    rp.AddSubpassDependency();
    rp.CreateRenderPass();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_resolve_prop.colorAttachmentFormat;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image color_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView color_view = color_image.CreateView();

    image_ci.pNext = &external_format;
    image_ci.format = VK_FORMAT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image resolve_image(*m_device, image_ci, vkt::set_layout);

    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper(&external_format);
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    sycci.components = {VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    vkt::SamplerYcbcrConversion ycbcr_conv(*m_device, sycci);

    VkSamplerYcbcrConversionInfo syci = vku::InitStructHelper();
    syci.conversion = ycbcr_conv.handle();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&syci);
    ivci.image = resolve_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    const vkt::ImageView resolve_view(*m_device, ivci);

    VkImageView attachments[2];
    attachments[0] = color_view.handle();
    attachments[1] = resolve_view.handle();

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 2, attachments);

    CreatePipelineHelper pipe(*this, &external_format);
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle());

    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = resolve_image.handle();
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
