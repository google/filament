/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021-2022 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "utils/convert_utils.h"

class NegativeDynamicRenderingLocalRead : public DynamicRenderingTest {};

TEST_F(NegativeDynamicRenderingLocalRead, AttachmentLayout) {
    TEST_DESCRIPTION("Feature is disabled, but attachment descriptor and/or reference uses VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ");

    // Add extention, but keep feature disabled
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference refs[] = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &refs[0], nullptr, nullptr, 0, nullptr},
    };

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 1u, attach, 1u, subpasses, 0u, nullptr);

    refs[0].layout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ;

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, true, "VUID-VkAttachmentReference-dynamicRenderingLocalRead-09546",
                         "VUID-VkAttachmentReference2-dynamicRenderingLocalRead-09546");

    refs[0].layout = VK_IMAGE_LAYOUT_GENERAL;
    attach->initialLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, true, "VUID-VkAttachmentDescription-dynamicRenderingLocalRead-09544",
                         "VUID-VkAttachmentDescription2-dynamicRenderingLocalRead-09544");

    attach->initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach->finalLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, true,
                         "VUID-VkAttachmentDescription-dynamicRenderingLocalRead-09545",
                         "VUID-VkAttachmentDescription2-dynamicRenderingLocalRead-09545");
}

TEST_F(NegativeDynamicRenderingLocalRead, CmdDrawColorLocation) {
    TEST_DESCRIPTION("Validate that mapping is not applied in CmdDraw call if rendering is not started by vkCmdBeginRendering");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    VkFormat color_formats[] = {VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED};
    uint32_t locations[] = {1, 0, 2};

    VkRenderingAttachmentLocationInfo pipeline_location_info = vku::InitStructHelper();
    pipeline_location_info.colorAttachmentCount = 2;
    pipeline_location_info.pColorAttachmentLocations = locations;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper(&pipeline_location_info);
    pipeline_rendering_info.colorAttachmentCount = 2;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments(2);

    VkPipelineColorBlendStateCreateInfo cbi = vku::InitStructHelper();
    cbi.attachmentCount = 2;
    cbi.pAttachments = color_blend_attachments.data();

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pColorBlendState = &cbi;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment[2] = {vku::InitStructHelper(), vku::InitStructHelper()};
    color_attachment[0].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment[1].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 2;
    rendering_info.pColorAttachments = &color_attachment[0];

    m_command_buffer.BeginRendering(rendering_info);

    VkRenderingAttachmentLocationInfo location_info = vku::InitStructHelper();
    location_info.colorAttachmentCount = 2;
    location_info.pColorAttachmentLocations = &locations[1];
    vk::CmdSetRenderingAttachmentLocationsKHR(m_command_buffer.handle(), &location_info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09548");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, CmdDrawColorIndex) {
    TEST_DESCRIPTION("Validate that mapping is not applied in CmdDraw call if rendering is not started by vkCmdBeginRendering");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    VkFormat color_formats[] = {VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED};
    uint32_t locations[] = {0, 1};
    uint32_t inputs[] = {0, 1, 0};

    VkRenderingInputAttachmentIndexInfo inputs_info = vku::InitStructHelper();
    inputs_info.colorAttachmentCount = 2;
    inputs_info.pColorAttachmentInputIndices = inputs;

    VkRenderingAttachmentLocationInfo locations_info = vku::InitStructHelper(&inputs_info);
    locations_info.colorAttachmentCount = 2;
    locations_info.pColorAttachmentLocations = locations;

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper(&locations_info);
    pipeline_rendering_info.colorAttachmentCount = 2;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments(2);
    VkPipelineColorBlendStateCreateInfo cbi = vku::InitStructHelper();
    cbi.attachmentCount = 2u;
    cbi.pAttachments = color_blend_attachments.data();

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pColorBlendState = &cbi;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment[2] = {vku::InitStructHelper(), vku::InitStructHelper()};
    color_attachment[0].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment[1].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 2;
    rendering_info.pColorAttachments = &color_attachment[0];

    m_command_buffer.BeginRendering(rendering_info);

    VkRenderingAttachmentLocationInfo location_info = vku::InitStructHelper();
    location_info.colorAttachmentCount = 2;
    location_info.pColorAttachmentLocations = locations;

    VkRenderingInputAttachmentIndexInfo input_info = vku::InitStructHelper();
    input_info.colorAttachmentCount = 2;
    input_info.pColorAttachmentInputIndices = &inputs[1];

    vk::CmdSetRenderingAttachmentLocationsKHR(m_command_buffer.handle(), &location_info);
    vk::CmdSetRenderingInputAttachmentIndicesKHR(m_command_buffer.handle(), &input_info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09549");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, CmdClearAttachments) {
    TEST_DESCRIPTION("Clear unmapped color attachment");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {m_width, m_height}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.BeginRendering(rendering_info);

    uint32_t location = VK_ATTACHMENT_UNUSED;
    VkRenderingAttachmentLocationInfo location_info = vku::InitStructHelper();
    location_info.colorAttachmentCount = 1;
    location_info.pColorAttachmentLocations = &location;

    vk::CmdSetRenderingAttachmentLocationsKHR(m_command_buffer.handle(), &location_info);

    VkClearAttachment clear_attachment;
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_attachment.clearValue.color.float32[0] = 1.0;
    clear_attachment.clearValue.color.float32[1] = 1.0;
    clear_attachment.clearValue.color.float32[2] = 1.0;
    clear_attachment.clearValue.color.float32[3] = 1.0;
    clear_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-colorAttachment-09503");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &clear_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, ImageBarrier) {
    TEST_DESCRIPTION("Test setting image memory barrier without dynamic rendering local read features enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderTileImageColorReadAccess);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();
    m_command_buffer.Begin();
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.srcAccessMask = VK_ACCESS_NONE;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ;
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    auto img_barrier2 = ConvertVkImageMemoryBarrierToV2(img_barrier, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = img_barrier2.ptr();

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-dynamicRenderingLocalRead-09552");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdPipelineBarrier2-None-09554");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-dynamicRenderingLocalRead-09552");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdPipelineBarrier-None-09554");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    img_barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier2.dstAccessMask = VK_ACCESS_NONE;
    img_barrier2.oldLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ;
    img_barrier2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-dynamicRenderingLocalRead-09551");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdPipelineBarrier2-None-09554");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.dstAccessMask = VK_ACCESS_NONE;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ;
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-dynamicRenderingLocalRead-09551");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdPipelineBarrier-None-09554");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeDynamicRenderingLocalRead, ImageBarrierOwnership) {
    TEST_DESCRIPTION("Test setting image memory barrier transferring ownership without dynamic rendering local read features enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderTileImageColorReadAccess);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();

    m_command_buffer.Begin();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.srcAccessMask = VK_ACCESS_NONE;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ;
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    auto img_barrier2 = ConvertVkImageMemoryBarrierToV2(img_barrier, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = img_barrier2.ptr();

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-09550");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageMemoryBarrier2-dynamicRenderingLocalRead-09552");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdPipelineBarrier2-None-09554");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-09550");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageMemoryBarrier-dynamicRenderingLocalRead-09552");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdPipelineBarrier-None-09554");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeDynamicRenderingLocalRead, ImageBarrierNoBufferOrImage) {
    TEST_DESCRIPTION("Test setting image memory barrier within BeginRenderning without dynamic rendering local read features enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::shaderTileImageColorReadAccess);
    RETURN_IF_SKIP(InitBasicDynamicRendering());
    InitDynamicRenderTarget();

    m_command_buffer.Begin();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.srcAccessMask = VK_ACCESS_NONE;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    auto img_barrier2 = ConvertVkImageMemoryBarrierToV2(img_barrier, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = img_barrier2.ptr();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-None-09554");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-09554");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeDynamicRenderingLocalRead, ImageBarrierFramebufferStagesOnly) {
    TEST_DESCRIPTION("Test barriers within render pass started by vkCmdBeginRendering specify only framebuffer stages.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());
    InitDynamicRenderTarget();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
    barrier2.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
    barrier2.srcAccessMask = VK_ACCESS_2_NONE;
    barrier2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier2;

    // testing vkCmdPipelineBarrier2 srcStageMask
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-srcStageMask-09556");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    // testing vkCmdPipelineBarrier2 dstStageMask
    std::swap(barrier2.srcStageMask, barrier2.dstStageMask);
    std::swap(barrier2.srcAccessMask, barrier2.dstAccessMask);

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-srcStageMask-09556");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    // testing vkCmdPipelineBarrier srcStageMask
    VkMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_2_NONE;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-09556");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &barrier, 0, nullptr, 0,
                           nullptr);
    m_errorMonitor->VerifyFound();

    // testing vkCmdPipelineBarrier dstStageMask
    std::swap(barrier.srcAccessMask, barrier.dstAccessMask);

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-09556");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &barrier, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeDynamicRenderingLocalRead, ImageBarrierRequireFeature) {
    TEST_DESCRIPTION(
        "Test setting image memory barrier within BeginRendering without dynamic rendering local read feature enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDynamicRendering());
    InitDynamicRenderTarget();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    VkMemoryBarrier2 barrier2 = vku::InitStructHelper();
    barrier2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    barrier2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-None-09553");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    VkMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-09553");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &barrier, 0, nullptr, 0,
                           nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeDynamicRenderingLocalRead, ImageBarrierInProperLayout) {
    TEST_DESCRIPTION(
        "Barrier within a render pass instance started with vkCmdBeginRendering, then the image must be in the "
        "VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ or VK_IMAGE_LAYOUT_GENERAL layout");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());
    InitRenderTarget();

    m_command_buffer.Begin();

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.srcAccessMask = VK_ACCESS_NONE;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_command_buffer.BeginRendering(begin_rendering_info);

    auto img_barrier2 = ConvertVkImageMemoryBarrierToV2(img_barrier, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = img_barrier2.ptr();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-image-09555");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-image-09555");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(NegativeDynamicRenderingLocalRead, BeginWithinRenderPass) {
    TEST_DESCRIPTION("Test setting initialLayout and finalLayout in attachment descriptor and reference against image usage.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    for (uint32_t i = 0; i < 3; i++) {
        std::vector<VkAttachmentReference> color_references;
        std::vector<VkAttachmentDescription> attachment_descriptions;
        std::shared_ptr<vkt::Framebuffer> framebuffer;
        std::vector<std::unique_ptr<vkt::Image>> renderTargets;
        std::vector<vkt::ImageView> render_target_views;   // color attachments but not depth
        std::vector<VkImageView> framebuffer_attachments;  // all attachments, can be consumed directly by the API

        VkAttachmentDescription att = {};
        att.format = m_render_target_fmt;
        att.samples = VK_SAMPLE_COUNT_1_BIT;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        att.initialLayout = i == 0 ? VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        att.finalLayout = i == 1 ? VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference ref = {};
        ref.layout = i == 2 ? VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        ref.attachment = 0;

        m_renderPassClearValues.clear();
        VkClearValue clear = {};
        clear.color = m_clear_color;

        attachment_descriptions.push_back(att);

        color_references.push_back(ref);

        m_renderPassClearValues.push_back(clear);

        std::unique_ptr<vkt::Image> img(new vkt::Image());

        VkFormatProperties props;

        vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), m_render_target_fmt, &props);

        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            img->Init(*m_device, m_width, m_height, 1, m_render_target_fmt,
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        } else {
            FAIL() << "Neither Linear nor Optimal allowed for render target";
        }

        render_target_views.push_back(img->CreateView());
        framebuffer_attachments.push_back(render_target_views.back().handle());
        renderTargets.push_back(std::move(img));

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.flags = 0;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = NULL;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = color_references.data();
        subpass.pResolveAttachments = NULL;

        VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
        rp_info.attachmentCount = attachment_descriptions.size();
        rp_info.pAttachments = attachment_descriptions.data();
        rp_info.subpassCount = 1;
        rp_info.pSubpasses = &subpass;
        rp_info.dependencyCount = 0;
        rp_info.pDependencies = nullptr;

        vk::CreateRenderPass(device(), &rp_info, NULL, &m_renderPass);

        framebuffer = std::shared_ptr<vkt::Framebuffer>(new vkt::Framebuffer(
            *DeviceObj(), m_renderPass, framebuffer_attachments.size(), framebuffer_attachments.data(), m_width, m_height));

        m_renderPassBeginInfo.renderPass = m_renderPass;
        m_renderPassBeginInfo.framebuffer = framebuffer->handle();
        m_renderPassBeginInfo.renderArea.extent.width = m_width;
        m_renderPassBeginInfo.renderArea.extent.height = m_height;
        m_renderPassBeginInfo.clearValueCount = m_renderPassClearValues.size();
        m_renderPassBeginInfo.pClearValues = m_renderPassClearValues.data();

        m_command_buffer.Begin();

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRenderPass-initialLayout-09537");
        vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_errorMonitor->VerifyFound();

        if (rp2Supported) {
            auto subpassBeginInfo = vku::InitStruct<VkSubpassBeginInfo>(nullptr, VK_SUBPASS_CONTENTS_INLINE);

            m_errorMonitor->SetDesiredError("VUID-vkCmdBeginRenderPass2-initialLayout-09538");
            vk::CmdBeginRenderPass2KHR(m_command_buffer.handle(), &m_renderPassBeginInfo, &subpassBeginInfo);
            m_errorMonitor->VerifyFound();
        }
        m_command_buffer.End();
        vk::DestroyRenderPass(*DeviceObj(), m_renderPass, nullptr);

        m_renderPass = VK_NULL_HANDLE;
    }
}

TEST_F(NegativeDynamicRenderingLocalRead, RemappingAtCreatePipeline) {
    TEST_DESCRIPTION("Color attachment count in Inputs Info must match to Rendering Create Info");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    VkRenderingAttachmentLocationInfo location_info = vku::InitStructHelper();
    location_info.colorAttachmentCount = 0;
    location_info.pColorAttachmentLocations = nullptr;

    VkRenderingInputAttachmentIndexInfo input_info = vku::InitStructHelper(&location_info);
    input_info.colorAttachmentCount = 0;
    input_info.pColorAttachmentInputIndices = nullptr;

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper(&input_info);
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-renderPass-09531");
    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-renderPass-09532");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, InputAttachmentIndexColorAttachmentCount) {
    TEST_DESCRIPTION("colorAttachmentCount in Inputs Info must be less than maxColorAttachments");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    std::vector<uint32_t> input_attachment_indices(m_device->Physical().limits_.maxColorAttachments + 1);
    for (size_t i = 0; i < input_attachment_indices.size(); i++) {
        input_attachment_indices[i] = static_cast<uint32_t>(i);
    }

    VkRenderingInputAttachmentIndexInfo input_info = vku::InitStructHelper();
    input_info.colorAttachmentCount = static_cast<uint32_t>(input_attachment_indices.size());
    input_info.pColorAttachmentInputIndices = input_attachment_indices.data();

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper(&input_info);
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkGraphicsPipelineCreateInfo-renderPass-09531");
    m_errorMonitor->SetDesiredError("VUID-VkRenderingInputAttachmentIndexInfo-colorAttachmentCount-09525");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, ColorAttachmentCountInPipelineRenderingCreateInfo) {
    TEST_DESCRIPTION("colorAttachmentCount must be less than or equal to maxColorAttachments in VkPipelineRenderingCreateInfo");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    std::vector<VkFormat> color_attachments(m_device->Physical().limits_.maxColorAttachments + 1, VK_FORMAT_R8G8B8A8_UNORM);

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = color_attachments.size();
    pipeline_rendering_info.pColorAttachmentFormats = color_attachments.data();

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredError("VUID-VkPipelineRenderingCreateInfo-colorAttachmentCount-09533");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, CmdSetAttachmentIndicesColorAttachmentCount) {
    TEST_DESCRIPTION("colorAttachmentCount must be equal to the value used to begin the current render pass instance");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment[2] = {vku::InitStructHelper(), vku::InitStructHelper()};
    color_attachment[0].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment[1].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 2;
    rendering_info.pColorAttachments = &color_attachment[0];

    m_command_buffer.BeginRendering(rendering_info);

    uint32_t locations[] = {0};
    VkRenderingInputAttachmentIndexInfo input_info = vku::InitStructHelper();
    input_info.colorAttachmentCount = 1;
    input_info.pColorAttachmentInputIndices = locations;

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetRenderingInputAttachmentIndices-pInputAttachmentIndexInfo-09517");
    vk::CmdSetRenderingInputAttachmentIndicesKHR(m_command_buffer.handle(), &input_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, CmdSetAttachmentIndices) {
    TEST_DESCRIPTION("Test CmdSetRenderingInputAttachmentIndicesKHR is called for render pass initiated by vkCmdBeginRendering");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    VkRenderingInputAttachmentIndexInfo input_info = vku::InitStructHelper();
    input_info.colorAttachmentCount = 0;
    input_info.pColorAttachmentInputIndices = nullptr;

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetRenderingInputAttachmentIndices-commandBuffer-09518");
    vk::CmdSetRenderingInputAttachmentIndicesKHR(m_command_buffer.handle(), &input_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, InputAttachmentIndexSetToUnused) {
    TEST_DESCRIPTION("If the feature is not enabled all pColorAttachmentInputIndices must be set to VK_ATTACHMENT_UNUSED");
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.BeginRendering(rendering_info);

    uint32_t locations[] = {0};
    uint32_t unused = VK_ATTACHMENT_UNUSED;
    VkRenderingInputAttachmentIndexInfo input_info[3] = {
        {VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR, nullptr, 1, &locations[0], nullptr, nullptr},
        {VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR, nullptr, 1, &unused, &locations[0], nullptr},
        {VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR, nullptr, 1, &unused, nullptr, &locations[0]}};
    const char *vuids[] = {"VUID-VkRenderingInputAttachmentIndexInfo-dynamicRenderingLocalRead-09519",
                           "VUID-VkRenderingInputAttachmentIndexInfo-dynamicRenderingLocalRead-09520",
                           "VUID-VkRenderingInputAttachmentIndexInfo-dynamicRenderingLocalRead-09521"};

    for (uint32_t i = 0; i < 3; i++) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdSetRenderingInputAttachmentIndices-dynamicRenderingLocalRead-09516");
        m_errorMonitor->SetDesiredError(vuids[i]);
        vk::CmdSetRenderingInputAttachmentIndicesKHR(m_command_buffer.handle(), &input_info[i]);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicRenderingLocalRead, InputAttachmentIndexUnique) {
    TEST_DESCRIPTION("Color, depth and stencil attachment indices are set to unique values");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachments[2] = {vku::InitStructHelper(), vku::InitStructHelper()};

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 2;
    rendering_info.pColorAttachments = &color_attachments[0];

    m_command_buffer.BeginRendering(rendering_info);

    uint32_t locations_bad[] = {0, 0};
    uint32_t locations_good[] = {0, 1};
    VkRenderingInputAttachmentIndexInfo input_info[3] = {
        {VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR, nullptr, 2, &locations_bad[0], nullptr, nullptr},
        {VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR, nullptr, 2, &locations_good[0], &locations_bad[0], nullptr},
        {VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR, nullptr, 2, &locations_good[0], nullptr, &locations_bad[0]}};
    const char *vuids[] = {"VUID-VkRenderingInputAttachmentIndexInfo-pColorAttachmentInputIndices-09522",
                           "VUID-VkRenderingInputAttachmentIndexInfo-pColorAttachmentInputIndices-09523",
                           "VUID-VkRenderingInputAttachmentIndexInfo-pColorAttachmentInputIndices-09524"};

    for (uint32_t i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredError(vuids[i]);
        vk::CmdSetRenderingInputAttachmentIndicesKHR(m_command_buffer.handle(), &input_info[i]);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDynamicRenderingLocalRead, CmdSetAttachmentLocationsColorAttachmentCount) {
    TEST_DESCRIPTION("colorAttachmentCount must be equal to the value used to begin the current render pass instance");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment[2] = {vku::InitStructHelper(), vku::InitStructHelper()};
    color_attachment[0].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment[1].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 2;
    rendering_info.pColorAttachments = &color_attachment[0];

    m_command_buffer.BeginRendering(rendering_info);

    uint32_t locations[] = {0};
    VkRenderingAttachmentLocationInfo location_info = vku::InitStructHelper();
    location_info.colorAttachmentCount = 1;
    location_info.pColorAttachmentLocations = locations;

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetRenderingAttachmentLocations-pLocationInfo-09510");
    vk::CmdSetRenderingAttachmentLocationsKHR(m_command_buffer.handle(), &location_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, NewFunctionsReportErrorExtensionDisabled) {
    TEST_DESCRIPTION("Check that new functions cannot be called if extension is disabled");
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;

    m_command_buffer.BeginRendering(rendering_info);

    VkRenderingAttachmentLocationInfo location_info = vku::InitStructHelper();
    location_info.colorAttachmentCount = 0;
    location_info.pColorAttachmentLocations = nullptr;

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetRenderingAttachmentLocations-dynamicRenderingLocalRead-09509");
    vk::CmdSetRenderingAttachmentLocationsKHR(m_command_buffer.handle(), &location_info);
    m_errorMonitor->VerifyFound();

    VkRenderingInputAttachmentIndexInfo input_info = vku::InitStructHelper();
    input_info.colorAttachmentCount = 0;
    input_info.pColorAttachmentInputIndices = nullptr;

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetRenderingInputAttachmentIndices-dynamicRenderingLocalRead-09516");
    vk::CmdSetRenderingInputAttachmentIndicesKHR(m_command_buffer.handle(), &input_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, CmdSetRenderingAttachmentLocations) {
    TEST_DESCRIPTION("Test CmdSetRenderingAttachmentLocationsKHR is called for render pass initiated by vkCmdBeginRendering");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    VkRenderingAttachmentLocationInfo location_info = vku::InitStructHelper();
    location_info.colorAttachmentCount = 0;
    location_info.pColorAttachmentLocations = nullptr;

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetRenderingAttachmentLocations-commandBuffer-09511");
    vk::CmdSetRenderingAttachmentLocationsKHR(m_command_buffer.handle(), &location_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, AttachmentLocationsValidity) {
    TEST_DESCRIPTION(
        "If the feature is not enabled all pColorAttachmentLocations must be set to its index within the array and unique");
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachments[2] = {vku::InitStructHelper(), vku::InitStructHelper()};
    color_attachments[0].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachments[1].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 2;
    rendering_info.pColorAttachments = &color_attachments[0];

    m_command_buffer.BeginRendering(rendering_info);

    uint32_t color_attachment_locations[2] = {1, 1};
    VkRenderingAttachmentLocationInfo location_info = vku::InitStructHelper();
    location_info.colorAttachmentCount = 2;
    location_info.pColorAttachmentLocations = color_attachment_locations;

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdSetRenderingAttachmentLocations-dynamicRenderingLocalRead-09509");
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentLocationInfo-pColorAttachmentLocations-09513");
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentLocationInfo-dynamicRenderingLocalRead-09512");
    vk::CmdSetRenderingAttachmentLocationsKHR(m_command_buffer.handle(), &location_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, AttachmentLocationsMax) {
    TEST_DESCRIPTION(
        "colorAttachmentCount must be less than or equal to maxColorAttachments. pColorAttachmentLocations[i] must be less than.");
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1;

    m_command_buffer.BeginRendering(rendering_info);

    std::vector<uint32_t> color_attachment_locations(m_device->Physical().limits_.maxColorAttachments + 1);
    for (size_t i = 0; i < color_attachment_locations.size(); i++) {
        color_attachment_locations[i] = static_cast<uint32_t>(i);
    }

    VkRenderingAttachmentLocationInfo location_info = vku::InitStructHelper();
    location_info.colorAttachmentCount = static_cast<uint32_t>(color_attachment_locations.size());
    location_info.pColorAttachmentLocations = color_attachment_locations.data();

    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentLocationInfo-colorAttachmentCount-09514");
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentLocationInfo-pColorAttachmentLocations-09515");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdSetRenderingAttachmentLocations-pLocationInfo-09510");
    vk::CmdSetRenderingAttachmentLocationsKHR(m_command_buffer.handle(), &location_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, DependencyViewLocalInsideRendering) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());
    InitDynamicRenderTarget();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    VkMemoryBarrier2 memory_barrier_2 = vku::InitStructHelper();
    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_VIEW_LOCAL_BIT;
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &memory_barrier_2;
    dependency_info.bufferMemoryBarrierCount = 0;
    dependency_info.imageMemoryBarrierCount = 0;

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-dependencyFlags-07891");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, DependencyViewLocalOutsideRendering) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    m_command_buffer.Begin();

    VkMemoryBarrier2 memory_barrier_2 = vku::InitStructHelper();
    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_VIEW_LOCAL_BIT;
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &memory_barrier_2;
    dependency_info.bufferMemoryBarrierCount = 0;
    dependency_info.imageMemoryBarrierCount = 0;

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-dependencyFlags-01186");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, FramebufferSpaceStagesDst) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());
    InitDynamicRenderTarget();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    VkMemoryBarrier2 memory_barrier_2 = vku::InitStructHelper();
    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_2_NONE;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &memory_barrier_2;
    dependency_info.bufferMemoryBarrierCount = 0;
    dependency_info.imageMemoryBarrierCount = 0;

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-srcStageMask-09556");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDynamicRenderingLocalRead, RenderingAttachmentLocationInfoMismatch) {
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());
    InitDynamicRenderTarget();

    const auto render_target_ci = vkt::Image::ImageCreateInfo2D(m_renderTargets[0]->Width(), m_renderTargets[0]->Height(),
                                                                m_renderTargets[0]->CreateInfo().mipLevels, 1,
                                                                m_renderTargets[0]->Format(), m_renderTargets[0]->Usage());

    vkt::Image render_target(*m_device, render_target_ci, vkt::set_layout);
    vkt::ImageView render_target_view = render_target.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, render_target_ci.arrayLayers);

    VkCommandBufferAllocateInfo secondary_cmd_buffer_alloc_info = vku::InitStructHelper();
    secondary_cmd_buffer_alloc_info.commandPool = m_command_pool.handle();
    secondary_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondary_cmd_buffer_alloc_info.commandBufferCount = 1;

    vkt::CommandBuffer secondary_cmd_buffer(*m_device, secondary_cmd_buffer_alloc_info);

    uint32_t color_attachment_locations[1] = {0};
    uint32_t invalid_attachment_locations[1] = {1};

    VkRenderingAttachmentLocationInfo rendering_attachment_location_info = vku::InitStructHelper{};
    rendering_attachment_location_info.colorAttachmentCount = 1;
    rendering_attachment_location_info.pColorAttachmentLocations = color_attachment_locations;

    VkRenderingAttachmentLocationInfo invalid_rendering_attachment_location_info = vku::InitStructHelper{};
    invalid_rendering_attachment_location_info.colorAttachmentCount = 0;
    invalid_rendering_attachment_location_info.pColorAttachmentLocations = nullptr;

    VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info =
        vku::InitStructHelper(&invalid_rendering_attachment_location_info);
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &render_target_ci.format;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper(&inheritance_rendering_info);

    VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info = vku::InitStructHelper{};
    secondary_cmd_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary_cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    secondary_cmd_buffer.Begin(&secondary_cmd_buffer_begin_info);
    secondary_cmd_buffer.End();

    m_command_buffer.Begin();

    VkRenderingAttachmentInfo color_attachment_info = vku::InitStructHelper{};
    color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_info.imageView = render_target_view.handle();

    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper{};
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 2;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment_info;

    // Force invalid colorAttachmentCount
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-09504");
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdSetRenderingAttachmentLocationsKHR(m_command_buffer.handle(), &rendering_attachment_location_info);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_cmd_buffer.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    // Force colorAttachmentLocation mismatch.
    invalid_rendering_attachment_location_info.colorAttachmentCount = 1;
    invalid_rendering_attachment_location_info.pColorAttachmentLocations = invalid_attachment_locations;

    secondary_cmd_buffer.Reset();
    secondary_cmd_buffer.Begin(&secondary_cmd_buffer_begin_info);
    secondary_cmd_buffer.End();

    m_command_buffer.Reset();
    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-09504");
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdSetRenderingAttachmentLocationsKHR(m_command_buffer.handle(), &rendering_attachment_location_info);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_cmd_buffer.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();

    m_command_buffer.End();
}

TEST_F(NegativeDynamicRenderingLocalRead, RenderingInputAttachmentIndexInfoMismatch) {
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());
    InitDynamicRenderTarget();

    const auto render_target_ci = vkt::Image::ImageCreateInfo2D(m_renderTargets[0]->Width(), m_renderTargets[0]->Height(),
                                                                m_renderTargets[0]->CreateInfo().mipLevels, 1,
                                                                m_renderTargets[0]->Format(), m_renderTargets[0]->Usage());

    vkt::Image render_target(*m_device, render_target_ci, vkt::set_layout);
    vkt::ImageView render_target_view = render_target.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, render_target_ci.arrayLayers);

    VkCommandBufferAllocateInfo secondary_cmd_buffer_alloc_info = vku::InitStructHelper();
    secondary_cmd_buffer_alloc_info.commandPool = m_command_pool.handle();
    secondary_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondary_cmd_buffer_alloc_info.commandBufferCount = 1;

    vkt::CommandBuffer secondary_cmd_buffer(*m_device, secondary_cmd_buffer_alloc_info);

    uint32_t color_attachment_indices[1] = {1};
    uint32_t invalid_color_attachment_indices[1] = {0};

    const uint32_t depth_attachment_index = VK_ATTACHMENT_UNUSED;
    const uint32_t stencil_attachment_index = VK_ATTACHMENT_UNUSED;
    const uint32_t invalid_depth_attachment_index = 0;
    const uint32_t invalid_stencil_attachment_index = 0;

    VkRenderingInputAttachmentIndexInfo rendering_input_attachment_index_info = vku::InitStructHelper{};
    rendering_input_attachment_index_info.colorAttachmentCount = 1;
    rendering_input_attachment_index_info.pColorAttachmentInputIndices = color_attachment_indices;
    rendering_input_attachment_index_info.pDepthInputAttachmentIndex = &depth_attachment_index;
    rendering_input_attachment_index_info.pStencilInputAttachmentIndex = &stencil_attachment_index;

    VkRenderingInputAttachmentIndexInfo invalid_rendering_input_attachment_index_info = vku::InitStructHelper{};
    invalid_rendering_input_attachment_index_info.colorAttachmentCount = 1;
    invalid_rendering_input_attachment_index_info.pColorAttachmentInputIndices = color_attachment_indices;
    invalid_rendering_input_attachment_index_info.pDepthInputAttachmentIndex = &depth_attachment_index;
    invalid_rendering_input_attachment_index_info.pStencilInputAttachmentIndex = &stencil_attachment_index;

    VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info =
        vku::InitStructHelper(&invalid_rendering_input_attachment_index_info);
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &render_target_ci.format;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper(&inheritance_rendering_info);

    VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info = vku::InitStructHelper{};
    secondary_cmd_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary_cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    VkRenderingAttachmentInfo color_attachment_info = vku::InitStructHelper{};
    color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_info.imageView = render_target_view.handle();

    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper{};
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 2;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment_info;

    const auto begin_record_and_verify_cmd_buffers = [&](const VkCommandBufferBeginInfo &commandBufferBeginInfo) {
        secondary_cmd_buffer.Begin(&commandBufferBeginInfo);
        secondary_cmd_buffer.End();

        m_command_buffer.Begin();
        m_command_buffer.BeginRendering(begin_rendering_info);

        vk::CmdSetRenderingInputAttachmentIndicesKHR(m_command_buffer.handle(), &rendering_input_attachment_index_info);

        m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-09505");
        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_cmd_buffer.handle());
        m_errorMonitor->VerifyFound();

        m_command_buffer.EndRendering();
        m_command_buffer.End();
    };

    // Force invalid colorAttachmentCount
    invalid_rendering_input_attachment_index_info.colorAttachmentCount = 0;

    begin_record_and_verify_cmd_buffers(secondary_cmd_buffer_begin_info);

    // Force colorAttachmentIndices mismatch.
    invalid_rendering_input_attachment_index_info.colorAttachmentCount = 1;
    invalid_rendering_input_attachment_index_info.pColorAttachmentInputIndices = invalid_color_attachment_indices;

    begin_record_and_verify_cmd_buffers(secondary_cmd_buffer_begin_info);

    // Force pDepthInputAttachmentIndex mismatch
    invalid_rendering_input_attachment_index_info.pColorAttachmentInputIndices = color_attachment_indices;
    invalid_rendering_input_attachment_index_info.pDepthInputAttachmentIndex = &invalid_depth_attachment_index;

    begin_record_and_verify_cmd_buffers(secondary_cmd_buffer_begin_info);

    // Force pStencilInputAttachmentIndex mismatch
    invalid_rendering_input_attachment_index_info.pDepthInputAttachmentIndex = &depth_attachment_index;
    invalid_rendering_input_attachment_index_info.pStencilInputAttachmentIndex = &invalid_stencil_attachment_index;

    begin_record_and_verify_cmd_buffers(secondary_cmd_buffer_begin_info);
}
