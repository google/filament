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

ParentTest::~ParentTest() {
    if (m_second_device) {
        delete m_second_device;
        m_second_device = nullptr;
    }
}

class PositiveParent : public ParentTest {};

TEST_F(PositiveParent, ImagelessFramebuffer) {
    TEST_DESCRIPTION("pAttachments is ignored even for common parent");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();  // Renderpass created on first device
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info = vku::InitStructHelper();
    framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info.width = 256;
    framebuffer_attachment_image_info.height = 256;
    framebuffer_attachment_image_info.layerCount = 1;
    framebuffer_attachment_image_info.viewFormatCount = 1;
    framebuffer_attachment_image_info.pViewFormats = &format;

    VkFramebufferAttachmentsCreateInfo framebuffer_attachments = vku::InitStructHelper();
    framebuffer_attachments.attachmentImageInfoCount = 1;
    framebuffer_attachments.pAttachmentImageInfos = &framebuffer_attachment_image_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&framebuffer_attachments);
    fb_info.renderPass = m_renderPass;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &image_view.handle();
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.width = m_width;
    fb_info.height = m_height;
    fb_info.layers = 1;

    vkt::Framebuffer fb(*m_device, fb_info);
}
