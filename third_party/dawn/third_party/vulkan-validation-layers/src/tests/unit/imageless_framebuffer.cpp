/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/render_pass_helper.h"

class NegativeImagelessFramebuffer : public VkLayerTest {};

TEST_F(NegativeImagelessFramebuffer, RenderPassBeginImageViewMismatch) {
    TEST_DESCRIPTION(
        "Begin a renderPass where the image views specified do not match the parameters used to create the framebuffer and render "
        "pass.");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());

    bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormats[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};
    VkFormat framebufferAttachmentFormats[3] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachmentFormats[0]);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo framebufferAttachmentImageInfo = vku::InitStructHelper();
    framebufferAttachmentImageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 2;
    framebufferAttachmentImageInfo.pViewFormats = framebufferAttachmentFormats;
    VkFramebufferAttachmentsCreateInfo framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = nullptr;
    framebufferCreateInfo.renderPass = rp.Handle();

    VkImageFormatListCreateInfo imageFormatListCreateInfo = vku::InitStructHelper();
    imageFormatListCreateInfo.viewFormatCount = 2;
    imageFormatListCreateInfo.pViewFormats = attachmentFormats;
    VkImageCreateInfo imageCreateInfo = vku::InitStructHelper(&imageFormatListCreateInfo);
    imageCreateInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageCreateInfo.extent.width = attachmentWidth;
    imageCreateInfo.extent.height = attachmentHeight;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.mipLevels = 10;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.format = attachmentFormats[0];
    vkt::Image image(*m_device, imageCreateInfo, vkt::set_layout);

    // Only use the subset without the TRANSFER bit
    VkImageViewUsageCreateInfo image_view_usage_create_info = vku::InitStructHelper();
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageViewCreateInfo imageViewCreateInfo = vku::InitStructHelper(&image_view_usage_create_info);
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = attachmentFormats[0];
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Has subset of usage flags
    vkt::ImageView imageViewSubset(*m_device, imageViewCreateInfo);

    imageViewCreateInfo.pNext = nullptr;
    vkt::ImageView imageView(*m_device, imageViewCreateInfo);

    VkImageView image_views[2] = {imageView.handle(), imageView.handle()};
    VkRenderPassAttachmentBeginInfo renderPassAttachmentBeginInfo = vku::InitStructHelper();
    renderPassAttachmentBeginInfo.attachmentCount = 1;
    renderPassAttachmentBeginInfo.pAttachments = image_views;
    VkRenderPassBeginInfo renderPassBeginInfo = vku::InitStructHelper(&renderPassAttachmentBeginInfo);
    renderPassBeginInfo.renderPass = rp.Handle();
    renderPassBeginInfo.renderArea.extent.width = attachmentWidth;
    renderPassBeginInfo.renderArea.extent.height = attachmentHeight;

    VkCommandBufferBeginInfo cmd_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    // Positive test first
    {
        framebufferCreateInfo.pAttachments = nullptr;
        framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        m_command_buffer.Begin(&cmd_begin_info);
        m_command_buffer.BeginRenderPass(renderPassBeginInfo);
        m_command_buffer.Reset();
    }

    // Imageless framebuffer creation bit not present
    {
        framebufferCreateInfo.pAttachments = &imageView.handle();
        framebufferCreateInfo.flags = 0;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03207", "VUID-VkRenderPassBeginInfo-framebuffer-03207");
    }
    {
        framebufferCreateInfo.pAttachments = nullptr;
        framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassAttachmentBeginInfo.attachmentCount = 2;
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03208", "VUID-VkRenderPassBeginInfo-framebuffer-03208");
        renderPassAttachmentBeginInfo.attachmentCount = 1;
    }

    // Mismatched number of attachments
    {
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassAttachmentBeginInfo.attachmentCount = 2;
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03208", "VUID-VkRenderPassBeginInfo-framebuffer-03208");
        renderPassAttachmentBeginInfo.attachmentCount = 1;
    }

    // Mismatched flags
    {
        framebufferAttachmentImageInfo.flags = 0;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03209", "VUID-VkRenderPassBeginInfo-framebuffer-03209");
        framebufferAttachmentImageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }

    // Mismatched usage
    {
        framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-04627", "VUID-VkRenderPassBeginInfo-framebuffer-04627");
        framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Mismatched usage because VkImageViewUsageCreateInfo restricted to TRANSFER
    {
        renderPassAttachmentBeginInfo.pAttachments = &imageViewSubset.handle();
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-04627", "VUID-VkRenderPassBeginInfo-framebuffer-04627");
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
    }

    // Mismatched width
    {
        framebufferAttachmentImageInfo.width += 1;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03211", "VUID-VkRenderPassBeginInfo-framebuffer-03211");
        framebufferAttachmentImageInfo.width -= 1;
    }

    // Mismatched height
    {
        framebufferAttachmentImageInfo.height += 1;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03212", "VUID-VkRenderPassBeginInfo-framebuffer-03212");
        framebufferAttachmentImageInfo.height -= 1;
    }

    // Mismatched layer count
    {
        framebufferAttachmentImageInfo.layerCount += 1;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03213", "VUID-VkRenderPassBeginInfo-framebuffer-03213");
        framebufferAttachmentImageInfo.layerCount -= 1;
    }

    // Mismatched view format count
    {
        framebufferAttachmentImageInfo.viewFormatCount = 3;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03214", "VUID-VkRenderPassBeginInfo-framebuffer-03214");
        framebufferAttachmentImageInfo.viewFormatCount = 2;
    }

    // Mismatched format lists
    {
        framebufferAttachmentFormats[1] = VK_FORMAT_B8G8R8A8_SRGB;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03215", "VUID-VkRenderPassBeginInfo-framebuffer-03215");
        framebufferAttachmentFormats[1] = VK_FORMAT_B8G8R8A8_UNORM;
    }

    // Mismatched formats
    {
        imageViewCreateInfo.format = attachmentFormats[1];
        vkt::ImageView imageView2(*m_device, imageViewCreateInfo);
        renderPassAttachmentBeginInfo.pAttachments = &imageView2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-03216", "VUID-VkRenderPassBeginInfo-framebuffer-03216");
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
        imageViewCreateInfo.format = attachmentFormats[0];
    }

    // Mismatched sample counts
    {
        imageCreateInfo.samples = VK_SAMPLE_COUNT_4_BIT;
        imageCreateInfo.mipLevels = 1;
        vkt::Image imageObject2(*m_device, imageCreateInfo, vkt::set_layout);
        imageViewCreateInfo.image = imageObject2.handle();
        vkt::ImageView imageView2(*m_device, imageViewCreateInfo);
        renderPassAttachmentBeginInfo.pAttachments = &imageView2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassBeginInfo-framebuffer-09047", "VUID-VkRenderPassBeginInfo-framebuffer-09047");
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
        imageViewCreateInfo.image = image.handle();
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.mipLevels = 10;
    }

    // Mismatched level counts
    {
        imageViewCreateInfo.subresourceRange.levelCount = 2;
        vkt::ImageView imageView2(*m_device, imageViewCreateInfo);
        renderPassAttachmentBeginInfo.pAttachments = &imageView2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03218",
                            "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03218");
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
        imageViewCreateInfo.subresourceRange.levelCount = 1;
    }

    // Non-identity component swizzle
    {
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_A;
        vkt::ImageView imageView2(*m_device, imageViewCreateInfo);
        renderPassAttachmentBeginInfo.pAttachments = &imageView2.handle();
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                            "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03219",
                            "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03219");
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    }

    {
        imageViewCreateInfo.subresourceRange.baseMipLevel = 1;
        vkt::ImageView imageView2(*m_device, imageViewCreateInfo);
        renderPassAttachmentBeginInfo.pAttachments = &imageView2.handle();
        framebufferAttachmentImageInfo.height = framebufferAttachmentImageInfo.height / 2;
        framebufferAttachmentImageInfo.width = framebufferAttachmentImageInfo.width / 2;
        framebufferCreateInfo.height = framebufferCreateInfo.height / 2;
        framebufferCreateInfo.width = framebufferCreateInfo.width / 2;
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        renderPassBeginInfo.framebuffer = framebuffer.handle();
        renderPassBeginInfo.renderArea.extent.height = renderPassBeginInfo.renderArea.extent.height / 2;
        renderPassBeginInfo.renderArea.extent.width = renderPassBeginInfo.renderArea.extent.width / 2;
        m_command_buffer.Begin(&cmd_begin_info);
        m_command_buffer.BeginRenderPass(renderPassBeginInfo);
        m_command_buffer.Reset();
        renderPassAttachmentBeginInfo.pAttachments = &imageView.handle();
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        framebufferAttachmentImageInfo.height = framebufferAttachmentImageInfo.height * 2;
        framebufferAttachmentImageInfo.width = framebufferAttachmentImageInfo.width * 2;
    }
}

TEST_F(NegativeImagelessFramebuffer, FeatureEnable) {
    TEST_DESCRIPTION("Use imageless framebuffer functionality without enabling the feature");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachmentFormat);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo framebufferAttachmentImageInfo = vku::InitStructHelper();
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfo framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = rp.Handle();
    framebufferCreateInfo.attachmentCount = 1;

    // Imageless framebuffer creation bit not present
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03189");
    vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, BasicUsage) {
    TEST_DESCRIPTION("Create an imageless framebuffer in various invalid ways");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachmentFormat);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo framebufferAttachmentImageInfo = vku::InitStructHelper();
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfo framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = rp.Handle();
    framebufferCreateInfo.attachmentCount = 1;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Attachments info not present
    framebufferCreateInfo.pNext = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03190");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;

    // Mismatched attachment counts
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 2;
    VkFramebufferAttachmentImageInfo framebufferAttachmentImageInfos[2] = {framebufferAttachmentImageInfo,
                                                                           framebufferAttachmentImageInfo};
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03191");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;

    // Mismatched format list
    attachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03205");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Mismatched format list
    attachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03205");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    attachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Mismatched layer count, multiview disabled
    framebufferCreateInfo.layers = 2;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-04546");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferCreateInfo.layers = 1;

    // Mismatched width
    framebufferCreateInfo.width += 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04541");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferCreateInfo.width -= 1;

    // Mismatched height
    framebufferCreateInfo.height += 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04542");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferCreateInfo.height -= 1;
}

TEST_F(NegativeImagelessFramebuffer, AttachmentImageUsageMismatch) {
    TEST_DESCRIPTION("Create an imageless framebuffer with mismatched attachment image usage");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat colorAndInputAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depthStencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(colorAndInputAttachmentFormat, VK_SAMPLE_COUNT_4_BIT);  // Color attachment
    rp.AddAttachmentDescription(colorAndInputAttachmentFormat);                         // Color resolve attachment
    rp.AddAttachmentDescription(depthStencilAttachmentFormat, VK_SAMPLE_COUNT_4_BIT);   // Depth stencil attachment
    rp.AddAttachmentDescription(colorAndInputAttachmentFormat);                         // Input attachment
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({2, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({3, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);
    rp.AddDepthStencilAttachment(2);
    rp.AddInputAttachment(3);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo framebufferAttachmentImageInfos[4] = {};
    // Color attachment
    framebufferAttachmentImageInfos[0] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 1;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &colorAndInputAttachmentFormat;
    // Color resolve attachment
    framebufferAttachmentImageInfos[1] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 1;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &colorAndInputAttachmentFormat;
    // Depth stencil attachment
    framebufferAttachmentImageInfos[2] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[2].width = attachmentWidth;
    framebufferAttachmentImageInfos[2].height = attachmentHeight;
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[2].layerCount = 1;
    framebufferAttachmentImageInfos[2].viewFormatCount = 1;
    framebufferAttachmentImageInfos[2].pViewFormats = &depthStencilAttachmentFormat;
    // Input attachment
    framebufferAttachmentImageInfos[3] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[3].width = attachmentWidth;
    framebufferAttachmentImageInfos[3].height = attachmentHeight;
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[3].layerCount = 1;
    framebufferAttachmentImageInfos[3].viewFormatCount = 1;
    framebufferAttachmentImageInfos[3].pViewFormats = &colorAndInputAttachmentFormat;
    VkFramebufferAttachmentsCreateInfo framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 4;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = rp.Handle();
    framebufferCreateInfo.attachmentCount = 4;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched usage
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03201");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Color resolve attachment, mismatched usage
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03201");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Depth stencil attachment, mismatched usage
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03202");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // Color attachment, mismatched usage
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03204");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
}

TEST_F(NegativeImagelessFramebuffer, AttachmentMultiviewImageLayerCountMismatch) {
    TEST_DESCRIPTION("Create an imageless framebuffer against a multiview-enabled render pass with mismatched layer counts");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat colorAndInputAttachmentFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depthStencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    uint32_t viewMask = 0x3u;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = vku::InitStructHelper();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(colorAndInputAttachmentFormat, VK_SAMPLE_COUNT_4_BIT);  // Color attachment
    rp.AddAttachmentDescription(colorAndInputAttachmentFormat);                         // Color resolve attachment
    rp.AddAttachmentDescription(depthStencilAttachmentFormat, VK_SAMPLE_COUNT_4_BIT);   // Depth stencil attachment
    rp.AddAttachmentDescription(colorAndInputAttachmentFormat);                         // Input attachment
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({2, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({3, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);
    rp.AddDepthStencilAttachment(2);
    rp.AddInputAttachment(3);
    rp.CreateRenderPass(&renderPassMultiviewCreateInfo);

    VkFramebufferAttachmentImageInfo framebufferAttachmentImageInfos[4] = {};
    // Color attachment
    framebufferAttachmentImageInfos[0] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 2;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &colorAndInputAttachmentFormat;
    // Color resolve attachment
    framebufferAttachmentImageInfos[1] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 2;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &colorAndInputAttachmentFormat;
    // Depth stencil attachment
    framebufferAttachmentImageInfos[2] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[2].width = attachmentWidth;
    framebufferAttachmentImageInfos[2].height = attachmentHeight;
    framebufferAttachmentImageInfos[2].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[2].layerCount = 2;
    framebufferAttachmentImageInfos[2].viewFormatCount = 1;
    framebufferAttachmentImageInfos[2].pViewFormats = &depthStencilAttachmentFormat;
    // Input attachment
    framebufferAttachmentImageInfos[3] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[3].width = attachmentWidth;
    framebufferAttachmentImageInfos[3].height = attachmentHeight;
    framebufferAttachmentImageInfos[3].usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[3].layerCount = 2;
    framebufferAttachmentImageInfos[3].viewFormatCount = 1;
    framebufferAttachmentImageInfos[3].pViewFormats = &colorAndInputAttachmentFormat;
    VkFramebufferAttachmentsCreateInfo framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 4;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = rp.Handle();
    framebufferCreateInfo.attachmentCount = 4;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched layer count
    framebufferAttachmentImageInfos[0].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[0].layerCount = 2;

    // Color resolve attachment, mismatched layer count
    framebufferAttachmentImageInfos[1].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[1].layerCount = 2;

    // Depth stencil attachment, mismatched layer count
    framebufferAttachmentImageInfos[2].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[2].layerCount = 2;

    // Input attachment, mismatched layer count
    framebufferAttachmentImageInfos[3].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, DepthStencilResolveAttachment) {
    TEST_DESCRIPTION(
        "Create an imageless framebuffer against a render pass using depth stencil resolve, with mismatched information");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = FindSupportedDepthStencilFormat(Gpu());

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachmentFormat, VK_SAMPLE_COUNT_4_BIT);  // Depth/stencil
    rp.AddAttachmentDescription(attachmentFormat, VK_SAMPLE_COUNT_1_BIT);  // Depth/stencil resolve
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddAttachmentReference(1, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddDepthStencilAttachment(0);
    rp.AddDepthStencilResolveAttachment(1, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
    rp.SetViewMask(0x3u);
    rp.CreateRenderPass();

    VkFramebufferAttachmentImageInfo framebufferAttachmentImageInfos[2] = {};
    // Depth/stencil attachment
    framebufferAttachmentImageInfos[0] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[0].width = attachmentWidth;
    framebufferAttachmentImageInfos[0].height = attachmentHeight;
    framebufferAttachmentImageInfos[0].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[0].layerCount = 2;
    framebufferAttachmentImageInfos[0].viewFormatCount = 1;
    framebufferAttachmentImageInfos[0].pViewFormats = &attachmentFormat;
    // Depth/stencil resolve attachment
    framebufferAttachmentImageInfos[1] = vku::InitStructHelper();
    framebufferAttachmentImageInfos[1].width = attachmentWidth;
    framebufferAttachmentImageInfos[1].height = attachmentHeight;
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    framebufferAttachmentImageInfos[1].layerCount = 2;
    framebufferAttachmentImageInfos[1].viewFormatCount = 1;
    framebufferAttachmentImageInfos[1].pViewFormats = &attachmentFormat;
    VkFramebufferAttachmentsCreateInfo framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 2;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachmentImageInfos;
    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&framebufferAttachmentsCreateInfo);
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = rp.Handle();
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = nullptr;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Color attachment, mismatched layer count
    framebufferAttachmentImageInfos[0].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[0].layerCount = 2;

    // Depth resolve attachment, mismatched image usage
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-03203");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
    framebufferAttachmentImageInfos[1].usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // Depth resolve attachment, mismatched layer count
    framebufferAttachmentImageInfos[1].layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-renderPass-03198");
    vk::CreateFramebuffer(device(), &framebufferCreateInfo, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, FragmentShadingRateUsage) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.CreateRenderPass();

    VkFormat viewFormat = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info = vku::InitStructHelper();
    fbai_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    fbai_info.width = 1;
    fbai_info.height = 1;
    fbai_info.layerCount = 1;
    fbai_info.viewFormatCount = 1;
    fbai_info.pViewFormats = &viewFormat;

    VkFramebufferAttachmentsCreateInfo fba_info = vku::InitStructHelper();
    fba_info.attachmentImageInfoCount = 1;
    fba_info.pAttachmentImageInfos = &fbai_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = rp.Handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = NULL;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04549");
    vkt::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, FragmentShadingRateDimensions) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.CreateRenderPass();

    VkFormat viewFormat = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info = vku::InitStructHelper();
    fbai_info.usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    fbai_info.width = 1;
    fbai_info.height = 1;
    fbai_info.layerCount = 1;
    fbai_info.viewFormatCount = 1;
    fbai_info.pViewFormats = &viewFormat;

    VkFramebufferAttachmentsCreateInfo fba_info = vku::InitStructHelper();
    fba_info.attachmentImageInfoCount = 1;
    fba_info.pAttachmentImageInfos = &fbai_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = rp.Handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = NULL;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width * 2;
    {
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04543");
        vkt::Framebuffer fb(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;

    {
        fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height * 2;
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04544");
        vkt::Framebuffer fb(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;

    {
        fbai_info.layerCount = 2;
        fb_info.layers = 3;
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04545");
        vkt::Framebuffer fb(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImagelessFramebuffer, FragmentShadingRateDimensionsMultiview) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage with imageless FB");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.SetViewMask(0x4);
    rp.CreateRenderPass();

    VkFormat viewFormat = VK_FORMAT_R8_UINT;
    VkFramebufferAttachmentImageInfo fbai_info = vku::InitStructHelper();
    fbai_info.usage = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    fbai_info.width = 1;
    fbai_info.height = 1;
    fbai_info.layerCount = 2;
    fbai_info.viewFormatCount = 1;
    fbai_info.pViewFormats = &viewFormat;

    VkFramebufferAttachmentsCreateInfo fba_info = vku::InitStructHelper();
    fba_info.attachmentImageInfoCount = 1;
    fba_info.pAttachmentImageInfos = &fbai_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&fba_info);
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.renderPass = rp.Handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = NULL;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04587");
    vkt::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, RenderPassBeginImageView3D) {
    TEST_DESCRIPTION("Misuse of VK_IMAGE_VIEW_TYPE_3D.");

    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());

    bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormats[1] = {VK_FORMAT_R8G8B8A8_UNORM};
    VkFormat framebufferAttachmentFormats[1] = {VK_FORMAT_R8G8B8A8_UNORM};

    // Create a renderPass with a single attachment
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachmentFormats[0], VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    // Create Attachments
    VkImageCreateInfo imageCreateInfo = vku::InitStructHelper();
    imageCreateInfo.flags = 0;
    imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageCreateInfo.extent.width = attachmentWidth;
    imageCreateInfo.extent.height = attachmentHeight;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.format = attachmentFormats[0];
    vkt::Image image3D(*m_device, imageCreateInfo, vkt::set_layout);
    vkt::ImageView imageView3D = image3D.CreateView(VK_IMAGE_VIEW_TYPE_3D);

    VkFramebufferAttachmentImageInfo framebufferAttachmentImageInfo = vku::InitStructHelper();
    framebufferAttachmentImageInfo.flags = 0;
    framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebufferAttachmentImageInfo.width = attachmentWidth;
    framebufferAttachmentImageInfo.height = attachmentHeight;
    framebufferAttachmentImageInfo.layerCount = 1;
    framebufferAttachmentImageInfo.viewFormatCount = 1;
    framebufferAttachmentImageInfo.pViewFormats = framebufferAttachmentFormats;
    VkFramebufferAttachmentsCreateInfo framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;

    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper();
    framebufferCreateInfo.width = attachmentWidth;
    framebufferCreateInfo.height = attachmentHeight;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.renderPass = rp.Handle();

    // Try to use 3D Image View without imageless flag
    {
        framebufferCreateInfo.pNext = nullptr;
        framebufferCreateInfo.flags = 0;
        framebufferCreateInfo.pAttachments = &imageView3D.handle();
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-flags-04113");
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        m_errorMonitor->VerifyFound();
    }

    framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
    framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebufferCreateInfo.pAttachments = nullptr;
    vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
    ASSERT_TRUE(framebuffer.initialized());

    VkRenderPassAttachmentBeginInfo renderPassAttachmentBeginInfo = vku::InitStructHelper();
    renderPassAttachmentBeginInfo.attachmentCount = 1;
    renderPassAttachmentBeginInfo.pAttachments = &imageView3D.handle();
    VkRenderPassBeginInfo renderPassBeginInfo = vku::InitStructHelper(&renderPassAttachmentBeginInfo);
    renderPassBeginInfo.renderPass = rp.Handle();
    renderPassBeginInfo.renderArea.extent.width = attachmentWidth;
    renderPassBeginInfo.renderArea.extent.height = attachmentHeight;
    renderPassBeginInfo.framebuffer = framebuffer.handle();

    // Try to use 3D Image View with imageless flag
    TestRenderPassBegin(m_errorMonitor, device(), m_command_buffer.handle(), &renderPassBeginInfo, rp2Supported,
                        "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-04114",
                        "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-04114");
}

TEST_F(NegativeImagelessFramebuffer, AttachmentImagePNext) {
    TEST_DESCRIPTION("Begin render pass with missing framebuffer attachment");
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Image image(*m_device, 256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView imageView = image.CreateView();

    // random invalid struct for a framebuffer pNext change
    VkCommandPoolCreateInfo invalid_struct = vku::InitStructHelper();

    VkFormat attachment_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkFramebufferAttachmentImageInfo fb_fdm = vku::InitStructHelper(&invalid_struct);
    fb_fdm.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    fb_fdm.width = 64;
    fb_fdm.height = 64;
    fb_fdm.layerCount = 1;
    fb_fdm.viewFormatCount = 1;
    fb_fdm.pViewFormats = &attachment_format;

    VkFramebufferAttachmentsCreateInfo fb_aci_fdm = vku::InitStructHelper();
    fb_aci_fdm.attachmentImageInfoCount = 1;
    fb_aci_fdm.pAttachmentImageInfos = &fb_fdm;

    VkFramebufferCreateInfo framebufferCreateInfo = vku::InitStructHelper(&fb_aci_fdm);
    framebufferCreateInfo.width = 64;
    framebufferCreateInfo.height = 64;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.renderPass = m_renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &imageView.handle();

    // VkFramebufferCreateInfo -pNext-> VkFramebufferAttachmentsCreateInfo
    //                                             |-> VkFramebufferAttachmentImageInfo -pNext-> INVALID
    {
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferAttachmentImageInfo-pNext-pNext");
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        m_errorMonitor->VerifyFound();
    }

    // VkFramebufferCreateInfo -pNext-> VkFramebufferAttachmentsCreateInfo -pNext-> INVALID
    {
        fb_fdm.pNext = nullptr;
        fb_aci_fdm.pNext = &invalid_struct;
        // Has parent struct name in VUID since child stucture don't have a pNext VU
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pNext-pNext");
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        m_errorMonitor->VerifyFound();
    }

    // VkFramebufferCreateInfo -pNext-> INVALID
    {
        fb_aci_fdm.pNext = nullptr;
        framebufferCreateInfo.pNext = &invalid_struct;
        m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-pNext-pNext");
        vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeImagelessFramebuffer, AttachmentImageFormat) {
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Image image(*m_device, 256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::ImageView imageView = image.CreateView();

    VkFormat attachment_format = VK_FORMAT_UNDEFINED;
    VkFramebufferAttachmentImageInfo fb_fdm = vku::InitStructHelper();
    fb_fdm.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    fb_fdm.width = 64;
    fb_fdm.height = 64;
    fb_fdm.layerCount = 1;
    fb_fdm.viewFormatCount = 1;
    fb_fdm.pViewFormats = &attachment_format;

    VkFramebufferAttachmentsCreateInfo fb_aci_fdm = vku::InitStructHelper();
    fb_aci_fdm.attachmentImageInfoCount = 1;
    fb_aci_fdm.pAttachmentImageInfos = &fb_fdm;

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper(&fb_aci_fdm);
    fb_ci.width = 64;
    fb_ci.height = 64;
    fb_ci.layers = 1;
    fb_ci.renderPass = m_renderPass;
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = &imageView.handle();

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferAttachmentImageInfo-viewFormatCount-09536");
    vkt::Framebuffer framebuffer(*m_device, fb_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImagelessFramebuffer, MissingInheritanceRenderingInfo) {
    TEST_DESCRIPTION("Begin cmd buffer with imageless framebuffer and missing VkCommandBufferInheritanceRenderingInfo structure");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
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

    fb_ci.pAttachments = nullptr;
    vkt::Framebuffer framebuffer_null(*m_device, fb_ci);

    vkt::ImageView rt_view = m_renderTargets[0]->CreateView();
    VkImageView image_views[2] = {rt_view, CastToHandle<VkImageView, uintptr_t>(0xbaadbeef)};
    fb_ci.pAttachments = image_views;
    vkt::Framebuffer framebuffer_bad_image_view(*m_device, fb_ci);

    VkCommandBufferInheritanceInfo inheritanceInfo = vku::InitStructHelper();
    inheritanceInfo.framebuffer = framebuffer_null.handle();

    VkCommandBufferBeginInfo beginInfo = vku::InitStructHelper();
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-06002");
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-09240");
    vk::BeginCommandBuffer(secondary.handle(), &beginInfo);
    m_errorMonitor->VerifyFound();
}
