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

class PositiveFragmentShadingRate : public VkLayerTest {};

TEST_F(PositiveFragmentShadingRate, StageInVariousAPIs) {
    TEST_DESCRIPTION("Specify shading rate pipeline stage with attachmentFragmentShadingRate feature enabled");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    const vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);
    const vkt::Event event(*m_device);
    const vkt::Event event2(*m_device);

    m_command_buffer.Begin();
    // Different API calls to cover three category of VUIDs: 07316, 07318, 07314
    vk::CmdResetEvent2KHR(m_command_buffer, event, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    vk::CmdSetEvent(m_command_buffer, event2, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    vk::CmdWriteTimestamp(m_command_buffer, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, query_pool, 0);
    m_command_buffer.End();
}

TEST_F(PositiveFragmentShadingRate, StageWithPipelineBarrier) {
    TEST_DESCRIPTION("Test pipeline barrier with VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR stage");
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);

    RETURN_IF_SKIP(Init());

    VkImageFormatProperties format_props = {};
    VkResult result = vk::GetPhysicalDeviceImageFormatProperties(
        m_device->Physical().handle(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, 0, &format_props);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "Image options not supported";
    }

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageMemoryBarrier imageMemoryBarrier = vku::InitStructHelper();
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.subresourceRange.levelCount = 1;

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, 0u, 0u, nullptr, 0u, nullptr, 1u,
                           &imageMemoryBarrier);
    m_command_buffer.End();
}

TEST_F(PositiveFragmentShadingRate, Attachments) {
    TEST_DESCRIPTION("Create framebuffer with a fragment shading rate attachment that has layout count 1.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::attachmentFragmentShadingRate);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(fsr_properties);

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddFragmentShadingRateAttachment(0, fsr_properties.minFragmentShadingRateAttachmentTexelSize);
    rp.SetViewMask(0x2);
    rp.CreateRenderPass();
    vkt::Image image(*m_device, 1, 1, 1, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    vkt::ImageView imageView = image.CreateView();

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &imageView.handle(),
                                 fsr_properties.minFragmentShadingRateAttachmentTexelSize.width,
                                 fsr_properties.minFragmentShadingRateAttachmentTexelSize.height);
    ASSERT_TRUE(framebuffer.initialized());
}
