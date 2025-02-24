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

class NegativeAndroidExternalResolve : public AndroidExternalResolveTest {};

TEST_F(NegativeAndroidExternalResolve, SubpassDescriptionSample) {
    TEST_DESCRIPTION("invalid samples for VkSubpassDescription");
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device);

    // index 0 = color | index 1 = resolve
    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_UNDEFINED, VK_SAMPLE_COUNT_2_BIT);  // 2_bit is invalid
    rp.AddAttachmentDescription(VK_FORMAT_UNDEFINED);
    rp.SetAttachmentDescriptionPNext(0, &external_format);
    rp.SetAttachmentDescriptionPNext(1, &external_format);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_PLANE_0_BIT);
    rp.AddAttachmentReference(1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);

    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-externalFormatResolve-09345");
    rp.CreateRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, AttachmentDescriptionZeroExternalFormat) {
    TEST_DESCRIPTION("invalid samples for VkSubpassDescription");
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
    ahb.GetExternalFormat(*m_device, &format_resolve_prop);
    external_format.externalFormat = 0;  // bad

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

    m_errorMonitor->SetDesiredError("VUID-VkAttachmentDescription2-format-09334");
    rp.CreateRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, SubpassDescriptionViewMask) {
    TEST_DESCRIPTION("invalid ViewMask for VkSubpassDescription");
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device);

    // index 0 = color | index 1 = resolve
    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_UNDEFINED);
    rp.AddAttachmentDescription(VK_FORMAT_UNDEFINED);
    rp.SetAttachmentDescriptionPNext(0, &external_format);
    rp.SetAttachmentDescriptionPNext(1, &external_format);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    rp.AddAttachmentReference(1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);
    rp.SetViewMask(1);  // bad

    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-externalFormatResolve-09346");
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-multiview-06558");
    rp.CreateRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, SubpassDescriptionColorAttachmentCount) {
    TEST_DESCRIPTION("invalid colorAttachmentCount for VkSubpassDescription");
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device);

    // index 0 = color | index 1 = resolve
    VkAttachmentDescription2 attachment_desc[2];
    attachment_desc[0] = vku::InitStructHelper(&external_format);
    attachment_desc[0].format = VK_FORMAT_UNDEFINED;
    attachment_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    attachment_desc[1] = attachment_desc[0];

    VkAttachmentReference2 color_attachment_ref = vku::InitStructHelper();
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment_ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment_ref.attachment = 0;

    VkAttachmentReference2 resolve_attachment_ref = vku::InitStructHelper();
    resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_attachment_ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolve_attachment_ref.attachment = 1;

    VkAttachmentReference2 color_attachments[2] = {color_attachment_ref, color_attachment_ref};
    VkAttachmentReference2 resolve_attachments[2] = {resolve_attachment_ref, resolve_attachment_ref};
    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 2;
    subpass.pColorAttachments = color_attachments;
    subpass.pResolveAttachments = resolve_attachments;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.attachmentCount = 2;
    render_pass_ci.pAttachments = attachment_desc;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-externalFormatResolve-09344");
    vkt::RenderPass render_pass(*m_device, render_pass_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, SubpassDescriptionMultiPlaneInput) {
    TEST_DESCRIPTION("invalid use of multiplanar for input attachments");
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device);

    // index 0 = color | index 1 = resolve
    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_UNDEFINED);
    rp.AddAttachmentDescription(VK_FORMAT_UNDEFINED);
    rp.SetAttachmentDescriptionPNext(0, &external_format);
    rp.SetAttachmentDescriptionPNext(1, &external_format);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_PLANE_0_BIT);
    rp.AddAttachmentReference(1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);
    rp.AddInputAttachment(0);

    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-externalFormatResolve-09348");
    rp.CreateRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, SubpassDescriptionNullColorProperty) {
    TEST_DESCRIPTION("Setting UNUSED depending on nullColorAttachmentWithExternalFormatResolve");
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device);

    // index 0 = color | index 1 = resolve
    VkAttachmentDescription2 attachment_desc[2];
    attachment_desc[0] = vku::InitStructHelper(&external_format);
    attachment_desc[0].format = VK_FORMAT_UNDEFINED;
    attachment_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    attachment_desc[1] = attachment_desc[0];

    VkAttachmentReference2 color_attachment_ref = vku::InitStructHelper();
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment_ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkAttachmentReference2 resolve_attachment_ref = vku::InitStructHelper();
    resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_attachment_ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolve_attachment_ref.attachment = 1;

    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pResolveAttachments = &resolve_attachment_ref;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.attachmentCount = 2;
    render_pass_ci.pAttachments = attachment_desc;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    if (nullColorAttachmentWithExternalFormatResolve) {
        color_attachment_ref.attachment = 0;
        m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-nullColorAttachmentWithExternalFormatResolve-09337");
    } else {
        color_attachment_ref.attachment = VK_ATTACHMENT_UNUSED;
        m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription2-nullColorAttachmentWithExternalFormatResolve-09336");
    }

    vkt::RenderPass render_pass(*m_device, render_pass_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, Framebuffer) {
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
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
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

    external_format.externalFormat++;  // create wrong format
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkExternalFormatANDROID-externalFormat-01894");
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

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pAttachments-09350");
    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 2, attachments);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, ImagelessFramebuffer) {
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

    external_format.externalFormat++;  // create wrong format
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkExternalFormatANDROID-externalFormat-01894");
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

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info[2];
    framebuffer_attachment_image_info[0] = vku::InitStructHelper();
    framebuffer_attachment_image_info[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info[0].width = 32;
    framebuffer_attachment_image_info[0].height = 32;
    framebuffer_attachment_image_info[0].layerCount = 1;
    framebuffer_attachment_image_info[0].viewFormatCount = 1;
    framebuffer_attachment_image_info[0].pViewFormats = &format_resolve_prop.colorAttachmentFormat;

    framebuffer_attachment_image_info[1] = framebuffer_attachment_image_info[0];
    framebuffer_attachment_image_info[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
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
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-framebuffer-09354");
    m_command_buffer.BeginRenderPass(render_pass_bi);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeAndroidExternalResolve, ImagelessFramebufferFormat) {
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
    const VkFormat color_format = VK_FORMAT_R8G8B8A8_UINT;
    if (format_resolve_prop.colorAttachmentFormat == color_format) {
        GTEST_SKIP() << "assuming colorAttachmentFormat is not VK_FORMAT_R8G8B8A8_UINT";
    }

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device, &format_resolve_prop);

    // index 0 = color | index 1 = resolve
    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(color_format);
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
    image_ci.format = color_format;
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

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info[2];
    framebuffer_attachment_image_info[0] = vku::InitStructHelper();
    framebuffer_attachment_image_info[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info[0].width = 32;
    framebuffer_attachment_image_info[0].height = 32;
    framebuffer_attachment_image_info[0].layerCount = 1;
    framebuffer_attachment_image_info[0].viewFormatCount = 1;
    framebuffer_attachment_image_info[0].pViewFormats = &color_format;

    framebuffer_attachment_image_info[1] = framebuffer_attachment_image_info[0];
    framebuffer_attachment_image_info[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
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
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-framebuffer-09353");
    m_command_buffer.BeginRenderPass(render_pass_bi);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeAndroidExternalResolve, DynamicRendering) {
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

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view.handle();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    color_attachment.resolveImageView = resolve_view.handle();
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingAttachmentInfo ds_attachment = vku::InitStructHelper();
    ds_attachment.imageView = VK_NULL_HANDLE;
    ds_attachment.resolveMode = VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {32, 32}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.pDepthAttachment = &ds_attachment;
    begin_rendering_info.pStencilAttachment = &ds_attachment;

    m_command_buffer.Begin();

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkRenderingAttachmentInfo-imageView-06129");
    // One for depth and stencil attachment
    m_errorMonitor->SetDesiredError("VUID-VkRenderingInfo-pDepthAttachment-09318");
    m_errorMonitor->SetDesiredError("VUID-VkRenderingInfo-pStencilAttachment-09319");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.colorAttachmentCount = 2;
    VkRenderingAttachmentInfo attachments[2]{color_attachment, ds_attachment};
    begin_rendering_info.pColorAttachments = attachments;
    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkRenderingInfo-colorAttachmentCount-09320");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, DynamicRenderingResolveModeNonNullColor) {
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

    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image bad_resolve_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView bad_resolve_view = bad_resolve_image.CreateView();

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

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_view.handle();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {32, 32}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();

    color_attachment.resolveImageView = resolve_view.handle();
    color_attachment.imageView = VK_NULL_HANDLE;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkRenderingAttachmentInfo-imageView-06129");
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentInfo-resolveMode-09329");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageView = color_view.handle();
    color_attachment.resolveImageView = bad_resolve_view.handle();
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkRenderingAttachmentInfo-imageView-06129");
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentInfo-resolveMode-09326");
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentInfo-resolveMode-09327");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, PipelineRasterizationSamples) {
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

    CreatePipelineHelper pipe(*this, &external_format);
    pipe.ms_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    pipe.gp_ci_.renderPass = rp.Handle();
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853");
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09313");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, PipelineRasterizationSamplesDynamicRendering) {
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

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper(&external_format);
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.ms_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09304");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, MissingImageUsage) {
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
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT; // missing VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
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
    m_errorMonitor->SetDesiredError("VUID-VkRenderingInfo-colorAttachmentCount-09476");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeAndroidExternalResolve, ClearAttachment) {
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-aspectMask-09298");
    VkClearAttachment clear_depth_attachment;
    clear_depth_attachment.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    clear_depth_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}, 0, 1};
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &clear_depth_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeAndroidExternalResolve, DrawDynamicRasterizationSamples) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3RasterizationSamples);
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

    VkFormat color_formats = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper(&external_format);
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkRenderingAttachmentInfo-imageView-06865");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkRenderingAttachmentInfo-imageView-06129");
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdSetRasterizationSamplesEXT(m_command_buffer.handle(), VK_SAMPLE_COUNT_2_BIT);
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-None-09363");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09365");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-rasterizationSamples-07474");
    vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeAndroidExternalResolve, PipelineBarrier) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
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
    rp.AddAttachmentDescription(format_resolve_prop.colorAttachmentFormat);
    rp.SetAttachmentDescriptionPNext(0, &external_format);
    rp.SetAttachmentDescriptionPNext(1, &external_format);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_PLANE_0_BIT);
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

    image_ci.format = format_resolve_prop.colorAttachmentFormat;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image resolve_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView resolve_view = resolve_image.CreateView();

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = resolve_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format_resolve_prop.colorAttachmentFormat;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageView attachments[2];
    attachments[0] = color_view.handle();
    attachments[1] = resolve_view.handle();

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 2, attachments);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();

    VkImageMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = resolve_image.handle();
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;
    // Valid because outside renderpass
    vk::CmdPipelineBarrier2KHR(m_command_buffer, &dependency_info);

    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), 32, 32);

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-image-09374");
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier2KHR(m_command_buffer, &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeAndroidExternalResolve, PipelineBarrierUnused) {
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-image-09373");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeAndroidExternalResolve, RenderPassAndFramebuffer) {
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve());

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkAndroidHardwareBufferFormatResolvePropertiesANDROID format_resolve_prop = vku::InitStructHelper();
    if (format_resolve_prop.colorAttachmentFormat == VK_FORMAT_R8G8B8A8_UINT) {
        GTEST_SKIP() << "assuming colorAttachmentFormat is not VK_FORMAT_R8G8B8A8_UINT";
    }

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device, &format_resolve_prop);

    // index 0 = color | index 1 = resolve
    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UINT);
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
    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
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

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-nullColorAttachmentWithExternalFormatResolve-09349");
    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 2, attachments);
    m_errorMonitor->VerifyFound();
}

#endif