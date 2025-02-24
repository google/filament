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

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"

class NegativeCommand : public VkLayerTest {};

TEST_F(NegativeCommand, CommandPoolConsistency) {
    TEST_DESCRIPTION("Allocate command buffers from one command pool and attempt to delete them from another.");

    m_errorMonitor->SetDesiredError("VUID-vkFreeCommandBuffers-pCommandBuffers-parent");

    RETURN_IF_SKIP(Init());

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkt::CommandPool command_pool_1(*m_device, pool_create_info);
    vkt::CommandPool command_pool_2(*m_device, pool_create_info);

    VkCommandBuffer cb;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool_1.handle();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &cb);

    vk::FreeCommandBuffers(device(), command_pool_2.handle(), 1, &cb);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, IndexBufferNotBound) {
    TEST_DESCRIPTION("Run an indexed draw call without an index buffer bound.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-None-07312");
    // Use DrawIndexed w/o an index buffer bound
    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, IndexBufferDestroyed) {
    TEST_DESCRIPTION("Run an indexed draw call without an index buffer bound.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    vkt::Buffer index_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    index_buffer.destroy();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-commandBuffer-recording");
    // Use DrawIndexed w/o an index buffer bound
    vk::CmdDrawIndexed(m_command_buffer.handle(), 3, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, IndexBufferSizeOffset) {
    TEST_DESCRIPTION("Run bind index buffer with an offset greater than the size of the index buffer.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    vkt::Buffer index_buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 512, VK_INDEX_TYPE_UINT16);

    // draw one past the end of the buffer
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-robustBufferAccess2-08798");
    vk::CmdDrawIndexed(m_command_buffer.handle(), 256, 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // draw one too many indices
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-robustBufferAccess2-08798");
    vk::CmdDrawIndexed(m_command_buffer.handle(), 257, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);

    // draw one too many indices
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-robustBufferAccess2-08798");
    vk::CmdDrawIndexed(m_command_buffer.handle(), 513, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // draw one past the end of the buffer using the offset
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-robustBufferAccess2-08798");
    vk::CmdDrawIndexed(m_command_buffer.handle(), 512, 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MissingClearAttachment) {
    TEST_DESCRIPTION("Points to a wrong colorAttachment index in a VkClearAttachment structure passed to vkCmdClearAttachments");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    VkClearAttachment color_attachment = {VK_IMAGE_ASPECT_COLOR_BIT, 2, VkClearValue{}};
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-aspectMask-07271");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    color_attachment.colorAttachment = VK_ATTACHMENT_UNUSED;
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-aspectMask-07271");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MissingClearAttachment2) {
    TEST_DESCRIPTION("Points to a wrong colorAttachment index in a VkClearAttachment structure passed to vkCmdClearAttachments");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    VkClearAttachment color_attachment = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VkClearValue{}};
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-aspectMask-07271");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearAttachment64Bit) {
    TEST_DESCRIPTION("Clear with a 64-bit format");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R64G64B64A64_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
        GTEST_SKIP() << "VK_FORMAT_R64G64B64A64_SFLOAT format not supported";
    }
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R64G64B64A64_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R64G64B64A64_SFLOAT);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &image_view.handle());

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());

    VkClearAttachment attachment;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-None-09679");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CommandBufferTwoSubmits) {
    m_errorMonitor->SetDesiredError("UNASSIGNED-DrawState-CommandBufferSingleSubmitViolation");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    m_command_buffer.End();

    // Bypass framework since it does the waits automatically
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Cause validation error by re-submitting cmd buffer that should only be
    // submitted once
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, Sync2CommandBufferTwoSubmits) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("UNASSIGNED-DrawState-CommandBufferSingleSubmitViolation");
    InitRenderTarget();

    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    m_command_buffer.End();

    // Bypass framework since it does the waits automatically
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = m_command_buffer.handle();

    VkSubmitInfo2 submit_info = vku::InitStructHelper();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_default_queue->Wait();

    // Cause validation error by re-submitting cmd buffer that should only be
    // submitted once
    vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_default_queue->Wait();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, PushConstants) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPipelineLayout pipeline_layout;
    VkPushConstantRange pc_range = {};
    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pushConstantRangeCount = 1;
    pipeline_layout_ci.pPushConstantRanges = &pc_range;

    //
    // Check for invalid push constant ranges in pipeline layouts.
    //
    struct PipelineLayoutTestCase {
        VkPushConstantRange const range;
        char const *msg;
    };

    const uint32_t too_big = m_device->Physical().limits_.maxPushConstantsSize + 0x4;
    const std::array<PipelineLayoutTestCase, 10> range_tests = {{
        {{VK_SHADER_STAGE_VERTEX_BIT, 0, 0}, "VUID-VkPushConstantRange-size-00296"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0, 1}, "VUID-VkPushConstantRange-size-00297"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 4, 1}, "VUID-VkPushConstantRange-size-00297"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 4, 0}, "VUID-VkPushConstantRange-size-00296"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 1, 4}, "VUID-VkPushConstantRange-offset-00295"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0, too_big}, "VUID-VkPushConstantRange-size-00298"},
        {{VK_SHADER_STAGE_VERTEX_BIT, too_big, too_big}, "VUID-VkPushConstantRange-offset-00294"},
        {{VK_SHADER_STAGE_VERTEX_BIT, too_big, 4}, "VUID-VkPushConstantRange-offset-00294"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0xFFFFFFF0, 0x00000020}, "VUID-VkPushConstantRange-offset-00294"},
        {{VK_SHADER_STAGE_VERTEX_BIT, 0x00000020, 0xFFFFFFF0}, "VUID-VkPushConstantRange-size-00298"},
    }};

    // Check for invalid offset and size
    for (const auto &iter : range_tests) {
        pc_range = iter.range;
        m_errorMonitor->SetDesiredError(iter.msg);
        vk::CreatePipelineLayout(device(), &pipeline_layout_ci, NULL, &pipeline_layout);
        m_errorMonitor->VerifyFound();
    }

    // Check for invalid stage flag
    pc_range.offset = 0;
    pc_range.size = 16;
    pc_range.stageFlags = 0;
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantRange-stageFlags-requiredbitmask");
    vk::CreatePipelineLayout(device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // Check for duplicate stage flags in a list of push constant ranges.
    // A shader can only have one push constant block and that block is mapped
    // to the push constant range that has that shader's stage flag set.
    // The shader's stage flag can only appear once in all the ranges, so the
    // implementation can find the one and only range to map it to.
    const uint32_t ranges_per_test = 5;
    struct DuplicateStageFlagsTestCase {
        VkPushConstantRange const ranges[ranges_per_test];
        std::vector<char const *> const msg;
    };
    // Overlapping ranges are OK, but a stage flag can appear only once.
    const std::array<DuplicateStageFlagsTestCase, 3> duplicate_stageFlags_tests = {
        {
            {{{VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4}},
             {
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
             }},
            {{{VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_GEOMETRY_BIT, 0, 4},
              {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_GEOMETRY_BIT, 0, 4}},
             {
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
             }},
            {{{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4},
              {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_VERTEX_BIT, 0, 4},
              {VK_SHADER_STAGE_GEOMETRY_BIT, 0, 4}},
             {
                 "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292",
             }},
        },
    };

    for (const auto &iter : duplicate_stageFlags_tests) {
        pipeline_layout_ci.pPushConstantRanges = iter.ranges;
        pipeline_layout_ci.pushConstantRangeCount = ranges_per_test;
        std::for_each(iter.msg.begin(), iter.msg.end(), [&](const char *vuid) { m_errorMonitor->SetDesiredError(vuid); });
        vk::CreatePipelineLayout(device(), &pipeline_layout_ci, NULL, &pipeline_layout);
        m_errorMonitor->VerifyFound();
    }

    //
    // CmdPushConstants tests
    //

    // Setup a pipeline layout with ranges: [0,32) [16,80)
    const std::vector<VkPushConstantRange> pc_range2 = {{VK_SHADER_STAGE_VERTEX_BIT, 16, 64},
                                                        {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 32}};
    const vkt::PipelineLayout pipeline_layout_obj(*m_device, {}, pc_range2);

    const uint8_t dummy_values[100] = {};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    // Check for invalid stage flag
    // Note that VU 07790 isn't reached due to parameter validation
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-stageFlags-requiredbitmask");
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_obj.handle(), 0, 0, 16, dummy_values);
    m_errorMonitor->VerifyFound();

    // Positive tests for the overlapping ranges
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16,
                         dummy_values);
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_VERTEX_BIT, 32, 48, dummy_values);
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_obj.handle(),
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 16, 16, dummy_values);

    // Wrong cmd stages for extant range
    // No range for all cmd stages -- "VUID-vkCmdPushConstants-offset-01795" VUID-vkCmdPushConstants-offset-01795
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-offset-01795");
    // Missing cmd stages for found overlapping range -- "VUID-vkCmdPushConstants-offset-01796" VUID-vkCmdPushConstants-offset-01796
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-offset-01796");
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_GEOMETRY_BIT, 0, 16,
                         dummy_values);
    m_errorMonitor->VerifyFound();

    // Wrong no extant range
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-offset-01795");
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 80, 4,
                         dummy_values);
    m_errorMonitor->VerifyFound();

    // Wrong overlapping extent
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-offset-01795");
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_obj.handle(),
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 20, dummy_values);
    m_errorMonitor->VerifyFound();

    // Wrong stage flags for valid overlapping range
    m_errorMonitor->SetDesiredError("VUID-vkCmdPushConstants-offset-01796");
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_obj.handle(), VK_SHADER_STAGE_VERTEX_BIT, 16, 16, dummy_values);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, PushConstant2PipelineLayoutCreateInfo) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance6);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const float data[16] = {};
    VkPushConstantsInfo pc_info = vku::InitStructHelper();
    pc_info.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pc_info.offset = 32;
    pc_info.size = 16;
    pc_info.pValues = data;
    pc_info.layout = VK_NULL_HANDLE;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkPushConstantsInfo-None-09495");
    vk::CmdPushConstants2KHR(m_command_buffer.handle(), &pc_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, NoBeginCommandBuffer) {
    m_errorMonitor->SetDesiredError("VUID-vkEndCommandBuffer-commandBuffer-00059");

    RETURN_IF_SKIP(Init());
    vkt::CommandBuffer commandBuffer(*m_device, m_command_pool);
    // Call EndCommandBuffer() w/o calling BeginCommandBuffer()
    vk::EndCommandBuffer(commandBuffer.handle());

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CommandBufferReset) {
    // Cause error due to Begin while recording CB
    // Then cause 2 errors for attempting to reset CB w/o having
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT set for the pool from
    // which CBs were allocated. Note that this bit is off by default.
    m_errorMonitor->SetDesiredError("VUID-vkBeginCommandBuffer-commandBuffer-00049");

    RETURN_IF_SKIP(InitFramework());
    RETURN_IF_SKIP(InitState(nullptr, nullptr, 0));

    // Calls AllocateCommandBuffers
    vkt::CommandBuffer commandBuffer(*m_device, m_command_pool);

    // Force the failure by setting the Renderpass and Framebuffer fields with (fake) data
    VkCommandBufferInheritanceInfo cmd_buf_hinfo = vku::InitStructHelper();
    VkCommandBufferBeginInfo cmd_buf_info = vku::InitStructHelper();
    cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

    // Begin CB to transition to recording state
    vk::BeginCommandBuffer(commandBuffer.handle(), &cmd_buf_info);
    // Can't re-begin. This should trigger error
    vk::BeginCommandBuffer(commandBuffer.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkResetCommandBuffer-commandBuffer-00046");
    VkCommandBufferResetFlags flags = 0;  // Don't care about flags for this test
    // Reset attempt will trigger error due to incorrect CommandPool state
    vk::ResetCommandBuffer(commandBuffer.handle(), flags);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkBeginCommandBuffer-commandBuffer-00050");
    // Transition CB to RECORDED state
    vk::EndCommandBuffer(commandBuffer.handle());
    // Now attempting to Begin will implicitly reset, which triggers error
    vk::BeginCommandBuffer(commandBuffer.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CommandBufferPrimaryFlags) {
    RETURN_IF_SKIP(Init());

    // Calls AllocateCommandBuffers
    vkt::CommandBuffer commandBuffer(*m_device, m_command_pool);

    VkCommandBufferBeginInfo cmd_buf_info = vku::InitStructHelper();
    cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    m_errorMonitor->SetDesiredError("VUID-vkBeginCommandBuffer-commandBuffer-02840");
    vk::BeginCommandBuffer(commandBuffer.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearColorAttachmentsOutsideRenderPass) {
    // Call CmdClearAttachmentss outside of an active RenderPass

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-renderpass");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Start no RenderPass
    m_command_buffer.Begin();

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}, 0, 1};
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearColorAttachmentsZeroLayercount) {
    TEST_DESCRIPTION("Call CmdClearAttachments with a pRect having a layerCount of zero.");

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-layerCount-01934");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}};
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearColorAttachmentsZeroExtent) {
    TEST_DESCRIPTION("Call CmdClearAttachments with a pRect having a rect2D extent of zero.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    clear_rect.rect.extent = {0, 1};
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-rect-02682");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    clear_rect.rect.extent = {1, 0};
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-rect-02683");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearAttachmentsAspectMasks) {
    TEST_DESCRIPTION("Check VkClearAttachment invalid aspect masks.");
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment attachment;
    attachment.clearValue.color.float32[0] = 0;
    attachment.clearValue.color.float32[1] = 0;
    attachment.clearValue.color.float32[2] = 0;
    attachment.clearValue.color.float32[3] = 0;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    attachment.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkClearAttachment-aspectMask-00020");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkClearAttachment-aspectMask-00020");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    attachment.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkClearAttachment-aspectMask-02246");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkClearAttachment-aspectMask-02246");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearAttachmentsImplicitCheck) {
    TEST_DESCRIPTION("Check VkClearAttachment implicit VUs.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment color_attachment;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    color_attachment.aspectMask = 0;
    m_errorMonitor->SetDesiredError("VUID-VkClearAttachment-aspectMask-requiredbitmask");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    color_attachment.aspectMask = 0xffffffff;
    m_errorMonitor->SetDesiredError("VUID-VkClearAttachment-aspectMask-parameter");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearAttachmentsDepth) {
    TEST_DESCRIPTION("Call CmdClearAttachments with invalid depth aspect masks.");

    RETURN_IF_SKIP(Init());
    m_depth_stencil_fmt = FindSupportedStencilOnlyFormat(Gpu());
    if (m_depth_stencil_fmt == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Couldn't find a stencil only image format";
    }

    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view.handle());

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment attachment;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);

    attachment.clearValue.depthStencil.depth = 0.0f;
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-aspectMask-07884");
    attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearAttachmentsStencil) {
    TEST_DESCRIPTION("Call CmdClearAttachments with invalid stencil aspect masks.");

    RETURN_IF_SKIP(Init());
    m_depth_stencil_fmt = FindSupportedDepthOnlyFormat(Gpu());
    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    InitRenderTarget(&depth_image_view.handle());

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment attachment;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);

    attachment.clearValue.depthStencil.depth = 0.0f;
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-aspectMask-07885");
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearAttachmentsOutsideRenderPass) {
    TEST_DESCRIPTION("Call CmdClearAttachments outside renderpass");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    VkClearAttachment attachment;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-renderpass");
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, DrawOutsideRenderPass) {
    TEST_DESCRIPTION("call vkCmdDraw without renderpass");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-renderpass");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MultiDrawDrawOutsideRenderPass) {
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiDraw);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiEXT-renderpass");
    vk::CmdDrawMultiEXT(m_command_buffer.handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ExecuteCommandsPrimaryCB) {
    TEST_DESCRIPTION("Attempt vkCmdExecuteCommands with a primary command buffer (should only be secondary)");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // An empty primary command buffer
    vkt::CommandBuffer cb(*m_device, m_command_pool);
    cb.Begin();
    cb.End();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    VkCommandBuffer handle = cb.handle();

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-00088");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &handle);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetUnexpectedError("All elements of pCommandBuffers must not be in the pending state");

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, SimultaneousUseOneShot) {
    TEST_DESCRIPTION("Submit the same command buffer twice in one submit looking for simultaneous use and one time submit errors");
    const char *simultaneous_use_message = "is already in use and is not marked for simultaneous use";
    RETURN_IF_SKIP(Init());

    VkCommandBuffer cmd_bufs[2];
    VkCommandBufferAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.commandBufferCount = 2;
    alloc_info.commandPool = m_command_pool.handle();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &alloc_info, cmd_bufs);

    VkCommandBufferBeginInfo cb_binfo = vku::InitStructHelper();
    cb_binfo.pInheritanceInfo = VK_NULL_HANDLE;
    cb_binfo.flags = 0;
    vk::BeginCommandBuffer(cmd_bufs[0], &cb_binfo);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(cmd_bufs[0], 0, 1, &viewport);
    vk::EndCommandBuffer(cmd_bufs[0]);
    VkCommandBuffer duplicates[2] = {cmd_bufs[0], cmd_bufs[0]};

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = duplicates;
    m_errorMonitor->SetDesiredError(simultaneous_use_message);
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();

    // Set one time use and now look for one time submit
    duplicates[0] = duplicates[1] = cmd_bufs[1];
    cb_binfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk::BeginCommandBuffer(cmd_bufs[1], &cb_binfo);
    vk::CmdSetViewport(cmd_bufs[1], 0, 1, &viewport);
    vk::EndCommandBuffer(cmd_bufs[1]);
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00071");
    m_errorMonitor->SetDesiredError("UNASSIGNED-DrawState-CommandBufferSingleSubmitViolation");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeCommand, DrawTimeImageViewTypeMismatchWithPipeline) {
    TEST_DESCRIPTION(
        "Test that an error is produced when an image view type does not match the dimensionality declared in the shader");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(s, vec3(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView imageView = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, imageView, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewType-07752");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, DrawTimeImageViewTypeMismatchWithPipelineFunction) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 color;

        vec4 foo(sampler3D func_sampler) {
            return texture(func_sampler, vec3(0));
        }

        void main() {
           color = foo(s);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView imageView = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, imageView, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewType-07752");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, DrawTimeImageComponentTypeMismatchWithPipeline) {
    TEST_DESCRIPTION(
        "Test that an error is produced when the component type of an imageview disagrees with the type in the shader.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform isampler2D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texelFetch(s, ivec2(0), 0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView imageView = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, imageView, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-format-07753");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ResolveImageLowSampleCount) {
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcImage-00257");

    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create two images of sample count 1 and try to Resolve between them
    VkImageCreateInfo image_create_info = vkt::Image::ImageCreateInfo2D(
        32, 1, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image srcImage(*m_device, image_create_info, vkt::set_layout);
    vkt::Image dstImage(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.srcOffset = {0, 0, 0};
    resolveRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.dstOffset = {0, 0, 0};
    resolveRegion.extent = {1, 1, 1};
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ResolveImageHighSampleCount) {
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstImage-00259");

    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create two images of sample count 4 and try to Resolve between them

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    vkt::Image srcImage(*m_device, image_create_info, vkt::set_layout);
    vkt::Image dstImage(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();
    // Need memory barrier to VK_IMAGE_LAYOUT_GENERAL for source and dest?
    // VK_IMAGE_LAYOUT_UNDEFINED = 0,
    // VK_IMAGE_LAYOUT_GENERAL = 1,
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.srcOffset = {0, 0, 0};
    resolveRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.dstOffset = {0, 0, 0};
    resolveRegion.extent = {1, 1, 1};
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ResolveImageFormatMismatch) {
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcImage-01386");

    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {32, 1, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    vkt::Image srcImage(*m_device, image_create_info, vkt::set_layout);

    // Set format to something other than source image
    image_create_info.format = VK_FORMAT_R32_SFLOAT;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image dstImage(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();
    // Need memory barrier to VK_IMAGE_LAYOUT_GENERAL for source and dest?
    // VK_IMAGE_LAYOUT_UNDEFINED = 0,
    // VK_IMAGE_LAYOUT_GENERAL = 1,
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.srcOffset = {0, 0, 0};
    resolveRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.dstOffset = {0, 0, 0};
    resolveRegion.extent = {1, 1, 1};
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ResolveImageLayoutMismatch) {
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.flags = 0;
    vkt::Image srcImage(*m_device, image_create_info, vkt::set_layout);

    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image dstImage(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();
    // source image must have valid contents before resolve
    VkClearColorValue clear_color = {{0, 0, 0, 0}};
    VkImageSubresourceRange subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.layerCount = 1;
    subresource.levelCount = 1;
    srcImage.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk::CmdClearColorImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1,
                           &subresource);
    srcImage.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dstImage.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.srcOffset = {0, 0, 0};
    resolveRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.dstOffset = {0, 0, 0};
    resolveRegion.extent = {1, 1, 1};
    // source image layout mismatch
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcImageLayout-00260");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    // dst image layout mismatch
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstImageLayout-00262");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ResolveInvalidSubresource) {
    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.flags = 0;
    vkt::Image srcImage(*m_device, image_create_info, vkt::set_layout);

    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image dstImage(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();
    // source image must have valid contents before resolve
    VkClearColorValue clear_color = {{0, 0, 0, 0}};
    VkImageSubresourceRange subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.layerCount = 1;
    subresource.levelCount = 1;
    srcImage.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk::CmdClearColorImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1,
                           &subresource);
    srcImage.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dstImage.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.srcOffset = {0, 0, 0};
    resolveRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.dstOffset = {0, 0, 0};
    resolveRegion.extent = {1, 1, 1};

    // invalid source mip level
    resolveRegion.srcSubresource.mipLevel = image_create_info.mipLevels;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcSubresource-01709");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkImageResolve2 resolveRegion2 = {VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2,
                                                NULL,
                                                resolveRegion.srcSubresource,
                                                resolveRegion.srcOffset,
                                                resolveRegion.dstSubresource,
                                                resolveRegion.dstOffset,
                                                resolveRegion.extent};
        const VkResolveImageInfo2 resolve_image_info2 = {VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2,
                                                         NULL,
                                                         srcImage.handle(),
                                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                         dstImage.handle(),
                                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                         1,
                                                         &resolveRegion2};
        m_errorMonitor->SetDesiredError("VUID-VkResolveImageInfo2-srcSubresource-01709");
        vk::CmdResolveImage2KHR(m_command_buffer.handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.srcSubresource.mipLevel = 0;
    // invalid dest mip level
    resolveRegion.dstSubresource.mipLevel = image_create_info.mipLevels;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstSubresource-01710");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkImageResolve2 resolveRegion2 = {VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2,
                                                NULL,
                                                resolveRegion.srcSubresource,
                                                resolveRegion.srcOffset,
                                                resolveRegion.dstSubresource,
                                                resolveRegion.dstOffset,
                                                resolveRegion.extent};
        const VkResolveImageInfo2 resolve_image_info2 = {VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2,
                                                         NULL,
                                                         srcImage.handle(),
                                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                         dstImage.handle(),
                                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                         1,
                                                         &resolveRegion2};
        m_errorMonitor->SetDesiredError("VUID-VkResolveImageInfo2-dstSubresource-01710");
        vk::CmdResolveImage2KHR(m_command_buffer.handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.dstSubresource.mipLevel = 0;
    // invalid source array layer range
    resolveRegion.srcSubresource.baseArrayLayer = image_create_info.arrayLayers;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcSubresource-01711");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkImageResolve2 resolveRegion2 = {VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2,
                                                NULL,
                                                resolveRegion.srcSubresource,
                                                resolveRegion.srcOffset,
                                                resolveRegion.dstSubresource,
                                                resolveRegion.dstOffset,
                                                resolveRegion.extent};
        const VkResolveImageInfo2 resolve_image_info2 = {VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2,
                                                         NULL,
                                                         srcImage.handle(),
                                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                         dstImage.handle(),
                                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                         1,
                                                         &resolveRegion2};
        m_errorMonitor->SetDesiredError("VUID-VkResolveImageInfo2-srcSubresource-01711");
        vk::CmdResolveImage2KHR(m_command_buffer.handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.srcSubresource.baseArrayLayer = 0;
    // invalid dest array layer range
    resolveRegion.dstSubresource.baseArrayLayer = image_create_info.arrayLayers;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstSubresource-01712");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkImageResolve2 resolveRegion2 = {VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2,
                                                NULL,
                                                resolveRegion.srcSubresource,
                                                resolveRegion.srcOffset,
                                                resolveRegion.dstSubresource,
                                                resolveRegion.dstOffset,
                                                resolveRegion.extent};
        const VkResolveImageInfo2 resolve_image_info2 = {VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2,
                                                         NULL,
                                                         srcImage.handle(),
                                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                         dstImage.handle(),
                                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                         1,
                                                         &resolveRegion2};
        m_errorMonitor->SetDesiredError("VUID-VkResolveImageInfo2-dstSubresource-01712");
        vk::CmdResolveImage2KHR(m_command_buffer.handle(), &resolve_image_info2);
        m_errorMonitor->VerifyFound();
    }

    resolveRegion.dstSubresource.baseArrayLayer = 0;

    m_command_buffer.End();
}

TEST_F(NegativeCommand, ResolveImageImageType) {
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 1, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;  // more than 1 to not trip other validation
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.flags = 0;

    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    vkt::Image srcImage2D(*m_device, image_create_info, vkt::set_layout);

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    vkt::Image dstImage1D(*m_device, image_create_info, vkt::set_layout);

    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.extent.height = 16;
    image_create_info.extent.depth = 16;
    image_create_info.arrayLayers = 1;
    vkt::Image dstImage3D(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.srcOffset = {0, 0, 0};
    resolveRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.dstOffset = {0, 0, 0};
    resolveRegion.extent = {1, 1, 1};

    // layerCount is not 1
    resolveRegion.srcSubresource.layerCount = 2;
    m_errorMonitor->SetDesiredError("VUID-VkImageResolve-layerCount-08803");
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcImage-04446");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage2D.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage3D.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcSubresource.layerCount = 1;

    // Set height with 1D dstImage
    resolveRegion.extent.height = 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstImage-00276");
    // Also exceed height of both images
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcOffset-00270");
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstOffset-00275");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage2D.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage1D.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.extent.height = 1;

    // Set depth with 1D dstImage and 2D srcImage
    resolveRegion.extent.depth = 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstImage-00278");
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcImage-00273");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage2D.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage1D.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.extent.depth = 1;

    m_command_buffer.End();
}

TEST_F(NegativeCommand, ResolveImageSizeExceeded) {
    TEST_DESCRIPTION("Resolve Image with subresource region greater than size of src/dst image");
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.flags = 0;

    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    vkt::Image srcImage2D(*m_device, image_create_info, vkt::set_layout);

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image dstImage2D(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageResolve resolveRegion = {};
    resolveRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.srcOffset = {0, 0, 0};
    resolveRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.dstOffset = {0, 0, 0};
    resolveRegion.extent = {32, 32, 1};

    vk::CmdResolveImage(m_command_buffer.handle(), srcImage2D.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);

    // srcImage exceeded in x-dim
    resolveRegion.srcOffset.x = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcOffset-00269");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage2D.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcOffset.x = 0;

    // dstImage exceeded in x-dim
    resolveRegion.dstOffset.x = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstOffset-00274");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage2D.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.dstOffset.x = 0;

    // both image exceeded in y-dim
    resolveRegion.srcOffset.y = 32;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcOffset-00270");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage2D.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcOffset.y = 0;

    resolveRegion.dstOffset.y = 32;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstOffset-00275");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage2D.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.dstOffset.y = 0;

    // srcImage exceeded in z-dim
    resolveRegion.srcOffset.z = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcOffset-00272");
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcImage-00273");  // because it's a 2d image
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage2D.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.srcOffset.z = 0;

    // dstImage exceeded in z-dim
    resolveRegion.dstOffset.z = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstOffset-00277");
    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstImage-00278");  // because it's a 2d image
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage2D.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage2D.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    resolveRegion.dstOffset.z = 0;

    m_command_buffer.End();
}

TEST_F(NegativeCommand, ClearImage) {
    TEST_DESCRIPTION("Call ClearColorImage w/ a depth|stencil image and ClearDepthStencilImage with a color image.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();

    // Color image
    VkClearColorValue clear_color;
    memset(clear_color.uint32, 0, sizeof(uint32_t) * 4);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image color_image_no_transfer(*m_device, image_ci);

    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image color_image(*m_device, image_ci);

    const VkImageSubresourceRange color_range = vkt::Image::SubresourceRange(image_ci, VK_IMAGE_ASPECT_COLOR_BIT);

    // Depth/Stencil image
    VkClearDepthStencilValue clear_value = {0};
    image_ci.format = VK_FORMAT_D16_UNORM;
    image_ci.extent = {64, 64, 1};
    image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image ds_image(*m_device, image_ci);

    const VkImageSubresourceRange ds_range = vkt::Image::SubresourceRange(image_ci, VK_IMAGE_ASPECT_DEPTH_BIT);

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-image-00007");

    vk::CmdClearColorImage(m_command_buffer.handle(), ds_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &color_range);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-image-00002");

    vk::CmdClearColorImage(m_command_buffer.handle(), color_image_no_transfer.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1,
                           &color_range);

    m_errorMonitor->VerifyFound();

    // Call CmdClearDepthStencilImage with color image
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-image-00014");
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-image-02826");

    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), color_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  &clear_value, 1, &ds_range);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearColor64Bit) {
    TEST_DESCRIPTION("Clear with a 64-bit format");
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R64G64B64A64_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "VK_FORMAT_R64G64B64A64_SFLOAT format not supported";
    }
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R64G64B64A64_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    m_command_buffer.Begin();
    const VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-image-09678");
    vk::CmdClearColorImage(m_command_buffer.handle(), image.handle(), image.Layout(), &clear_color, 1, &range);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CommandQueueFlags) {
    TEST_DESCRIPTION(
        "Allocate a command buffer on a queue that does not support graphics and try to issue a graphics-only command");

    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> queueFamilyIndex = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!queueFamilyIndex) {
        GTEST_SKIP() << "Non-graphics queue family not found";
    }

    // Create command pool on a non-graphics queue
    vkt::CommandPool command_pool(*m_device, queueFamilyIndex.value());

    // Setup command buffer on pool
    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    // Issue a graphics only command
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetViewport-commandBuffer-cmdpool");
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(command_buffer.handle(), 0, 1, &viewport);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MultiDraw) {
    TEST_DESCRIPTION("Test validation of multi_draw extension");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiDraw);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMultiDrawPropertiesEXT multi_draw_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multi_draw_properties);
    InitRenderTarget();

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;

    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiEXT-None-08606");
    vk::CmdDrawMultiEXT(m_command_buffer.handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-None-07312");  // missing index buffer
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-None-08606");
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // New VUIDs added with multi_draw (also see GPU-AV)
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    multi_draw_indices[2].indexCount = 511;
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer.handle(), 2, VK_INDEX_TYPE_UINT16);
    // This first should be fine
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    // Fail with index offset
    multi_draw_indices[2].firstIndex = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-08798");
    // Fail with index count
    multi_draw_indices[2].firstIndex = 0;
    multi_draw_indices[2].indexCount = 512;
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-08798");
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
    multi_draw_indices[2].indexCount = 1;

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiEXT-drawCount-09628");
    vk::CmdDrawMultiEXT(m_command_buffer.handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT) + 1);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-08798", 2);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-drawCount-09629");
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT) + 1, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiEXT-drawCount-04935");
    vk::CmdDrawMultiEXT(m_command_buffer.handle(), 3, nullptr, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-drawCount-04940");
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 3, nullptr, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();

    if (multi_draw_properties.maxMultiDrawCount < vvl::kU32Max) {
        uint32_t draw_count = multi_draw_properties.maxMultiDrawCount + 1;
        std::vector<VkMultiDrawInfoEXT> max_multi_draws(draw_count);
        std::vector<VkMultiDrawIndexedInfoEXT> max_multi_indexed_draws(draw_count);
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiEXT-drawCount-04934");
        vk::CmdDrawMultiEXT(m_command_buffer.handle(), draw_count, max_multi_draws.data(), 1, 0, sizeof(VkMultiDrawInfoEXT));
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-drawCount-04939");
        vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), draw_count, max_multi_indexed_draws.data(), 1, 0,
                                   sizeof(VkMultiDrawIndexedInfoEXT), 0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeCommand, MultiDrawMaintenance5) {
    TEST_DESCRIPTION("Test validation of multi_draw extension with VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredFeature(vkt::Feature::multiDraw);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // Use non-power-of-2 size to
    vkt::Buffer buffer(*m_device, 2048, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMultiDrawIndexedInfoEXT multi_draw_indices = {0, 514, 0};  // overflow

    // same as calling vkCmdBindIndexBuffer (size of the buffer creation)
    vk::CmdBindIndexBuffer2KHR(m_command_buffer.handle(), buffer.handle(), 2, 1024, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-08798");
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();

    multi_draw_indices.indexCount = 256;  // only uses [0 - 512]
    vk::CmdBindIndexBuffer2KHR(m_command_buffer.handle(), buffer.handle(), 2, 508, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-08798");
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MultiDrawWholeSizeMaintenance5) {
    TEST_DESCRIPTION("Test validation of multi_draw extension with VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredFeature(vkt::Feature::multiDraw);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // Use non-power-of-2 size to
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMultiDrawIndexedInfoEXT multi_draw_indices = {0, 514, 0};  // overflow

    // VK_WHOLE_SIZE also full size of the buffer
    vk::CmdBindIndexBuffer2KHR(m_command_buffer.handle(), buffer.handle(), 2, VK_WHOLE_SIZE, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-08798");
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MultiDrawMaintenance5Mixed) {
    TEST_DESCRIPTION("Test vkCmdBindIndexBuffer2KHR with vkCmdBindIndexBuffer");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredFeature(vkt::Feature::multiDraw);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // New VUIDs added with multi_draw (also see GPU-AV)
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkMultiDrawIndexedInfoEXT multi_draw_indices = {0, 511, 0};

    // valid
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    // should be overwritten with smaller size
    vk::CmdBindIndexBuffer2KHR(m_command_buffer.handle(), buffer.handle(), 0, 1000, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-08798");
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, MultiDrawFeatures) {
    TEST_DESCRIPTION("Test validation of multi draw feature enabled");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;

    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiEXT-None-04933");
    vk::CmdDrawMultiEXT(m_command_buffer.handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_errorMonitor->VerifyFound();
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMultiIndexedEXT-None-04937");
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, IndirectDraw) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndirect and vkCmdDrawIndexedIndirect");

    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    vkt::Buffer draw_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vkt::Buffer draw_buffer_correct(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect-buffer-02709");
    vk::CmdDrawIndirect(m_command_buffer.handle(), draw_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect-drawCount-00488");
    vk::CmdDrawIndirect(m_command_buffer.handle(), draw_buffer_correct.handle(), 0, 2, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect-drawCount-00540");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_buffer_correct.handle(), 0, 2, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect-offset-02710");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_buffer_correct.handle(), 2, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, MultiDrawIndirectFeature) {
    TEST_DESCRIPTION("use vkCmdDrawIndexedIndirect without MultiDrawIndirect");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (m_device->Physical().limits_.maxDrawIndirectCount < 2) {
        GTEST_SKIP() << "maxDrawIndirectCount is too low";
    }

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    vkt::Buffer draw_buffer(*m_device, sizeof(VkDrawIndirectCommand) * 3, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_buffer.handle(), 0, 0, sizeof(VkDrawIndexedIndirectCommand));
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_buffer.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect-drawCount-02718");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), draw_buffer.handle(), 0, 2, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, StrideMultiDrawIndirect) {
    TEST_DESCRIPTION("Validate Stride parameter.");
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const uint32_t buffer_size = 128;
    vkt::Buffer buffer(*m_device, buffer_size,
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    CreatePipelineHelper helper(*this);
    helper.CreateGraphicsPipeline();

    m_command_buffer.Begin();

    auto BufferMemoryBarrier = buffer.BufferMemoryBarrier(
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT, 0, VK_WHOLE_SIZE);
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1,
                           &BufferMemoryBarrier, 0, nullptr);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect-drawCount-00476");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect-drawCount-00488");
    vk::CmdDrawIndirect(m_command_buffer.handle(), buffer.handle(), 0, 100, 2);
    m_errorMonitor->VerifyFound();

    vk::CmdDrawIndirect(m_command_buffer.handle(), buffer.handle(), 0, 2, 24);

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect-drawCount-00528");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect-drawCount-00540");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), buffer.handle(), 0, 100, 2);
    m_errorMonitor->VerifyFound();

    auto draw_count = m_device->Physical().limits_.maxDrawIndirectCount;
    if (draw_count != vvl::kU32Max) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect-drawCount-02719");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect-drawCount-00476");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect-drawCount-00488");
        vk::CmdDrawIndirect(m_command_buffer.handle(), buffer.handle(), 0, draw_count + 1, 2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect-drawCount-00487");
    vk::CmdDrawIndirect(m_command_buffer.handle(), buffer.handle(), buffer_size, 1, 2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect-drawCount-00539");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), buffer.handle(), buffer_size, 1, 2);
    m_errorMonitor->VerifyFound();

    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), buffer.handle(), 0, 2, 24);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, DrawIndirectCountKHR) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndirectCountKHR");

    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(VkDrawIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer draw_buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkDeviceSize count_buffer_size = 128;
    VkBufferCreateInfo count_buffer_create_info = vku::InitStructHelper();
    count_buffer_create_info.size = count_buffer_size;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer count_buffer(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount-buffer-02708");
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    draw_buffer.AllocateAndBindMemory(*m_device);

    vkt::Buffer count_buffer_unbound(*m_device, count_buffer_create_info, vkt::no_mem);

    vkt::Buffer count_buffer_wrong;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    count_buffer_wrong.init(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount-countBuffer-02714");
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer_unbound.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount-countBuffer-02715");
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer_wrong.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount-offset-02710");
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 1, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount-countBufferOffset-02716");
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 1, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount-countBufferOffset-04129");
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), count_buffer_size, 1,
                                sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount-stride-03110");
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, DrawIndexedIndirectCountKHR) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndexedIndirectCountKHR");

    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(VkDrawIndexedIndirectCommand);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer draw_buffer(*m_device, buffer_create_info);

    VkDeviceSize count_buffer_size = 128;
    VkBufferCreateInfo count_buffer_create_info = vku::InitStructHelper();
    count_buffer_create_info.size = count_buffer_size;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    vkt::Buffer count_buffer(*m_device, count_buffer_create_info);

    VkBufferCreateInfo index_buffer_create_info = vku::InitStructHelper();
    index_buffer_create_info.size = sizeof(uint32_t);
    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vkt::Buffer index_buffer(*m_device, index_buffer_create_info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount-None-07312");
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    vkt::Buffer draw_buffer_unbound(*m_device, count_buffer_create_info, vkt::no_mem);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount-buffer-02708");
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), draw_buffer_unbound.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    vkt::Buffer count_buffer_unbound(*m_device, count_buffer_create_info, vkt::no_mem);

    vkt::Buffer count_buffer_wrong;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    count_buffer_wrong.init(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02714");
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer_unbound.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02715");
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer_wrong.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount-offset-02710");
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 1, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-02716");
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 1, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-04129");
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), count_buffer_size,
                                       1, sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount-stride-03142");
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, DrawIndirectCountFeature) {
    TEST_DESCRIPTION("Test covered valid usage for the 1.2 drawIndirectCount feature");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Make calls to valid commands but without the drawIndirectCount feature set
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount-None-04445");
    vk::CmdDrawIndirectCount(m_command_buffer.handle(), indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                             sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount-None-04445");
    vk::CmdDrawIndexedIndirectCount(m_command_buffer.handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                    sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ExclusiveScissorNV) {
    TEST_DESCRIPTION("Test VK_NV_scissor_exclusive with multiViewport disabled.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::exclusiveScissor);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (m_device->Physical().limits_.maxViewports <= 1) {
        GTEST_SKIP() << "Device doesn't support multiple viewports";
    }

    // Based on PSOViewportStateTests
    {
        VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
        VkViewport viewports[] = {viewport, viewport};
        VkRect2D scissor = {{0, 0}, {64, 64}};
        VkRect2D scissors[100] = {scissor, scissor};

        struct TestCase {
            uint32_t viewport_count;
            VkViewport *viewports;
            uint32_t scissor_count;
            VkRect2D *scissors;
            uint32_t exclusive_scissor_count;
            VkRect2D *exclusive_scissors;

            std::vector<std::string> vuids;
        };

        std::vector<TestCase> test_cases = {
            {1,
             viewports,
             1,
             scissors,
             2,
             scissors,
             {"VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02027",
              "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02029"}},
            {1,
             viewports,
             1,
             scissors,
             100,
             scissors,
             {"VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02027",
              "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02028",
              "VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02029"}},
            {1, viewports, 1, scissors, 1, nullptr, {"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04056"}},
        };

        for (const auto &test_case : test_cases) {
            VkPipelineViewportExclusiveScissorStateCreateInfoNV exc =
                vku::InitStructHelper();

            const auto break_vp = [&test_case, &exc](CreatePipelineHelper &helper) {
                helper.vp_state_ci_.viewportCount = test_case.viewport_count;
                helper.vp_state_ci_.pViewports = test_case.viewports;
                helper.vp_state_ci_.scissorCount = test_case.scissor_count;
                helper.vp_state_ci_.pScissors = test_case.scissors;
                helper.vp_state_ci_.pNext = &exc;

                exc.exclusiveScissorCount = test_case.exclusive_scissor_count;
                exc.pExclusiveScissors = test_case.exclusive_scissors;
            };
            CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
        }
    }

    // Based on SetDynScissorParamTests
    {
        const VkRect2D scissor = {{0, 0}, {16, 16}};
        const VkRect2D scissors[] = {scissor, scissor};

        m_command_buffer.Begin();

        m_errorMonitor->SetDesiredError("VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035");
        vk::CmdSetExclusiveScissorNV(m_command_buffer.handle(), 1, 1, scissors);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-arraylength");
        vk::CmdSetExclusiveScissorNV(m_command_buffer.handle(), 0, 0, nullptr);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-02036");
        vk::CmdSetExclusiveScissorNV(m_command_buffer.handle(), 0, 2, scissors);
        m_errorMonitor->VerifyFound();

        // This VU gets triggered before VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035
        m_errorMonitor->SetDesiredError("VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-arraylength");
        vk::CmdSetExclusiveScissorNV(m_command_buffer.handle(), 1, 0, scissors);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035");
        m_errorMonitor->SetDesiredError("VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-02036");
        vk::CmdSetExclusiveScissorNV(m_command_buffer.handle(), 1, 2, scissors);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdSetExclusiveScissorNV-pExclusiveScissors-parameter");
        vk::CmdSetExclusiveScissorNV(m_command_buffer.handle(), 0, 1, nullptr);
        m_errorMonitor->VerifyFound();

        struct TestCase {
            VkRect2D scissor;
            std::string vuid;
        };

        std::vector<TestCase> test_cases = {
            {{{-1, 0}, {16, 16}}, "VUID-vkCmdSetExclusiveScissorNV-x-02037"},
            {{{0, -1}, {16, 16}}, "VUID-vkCmdSetExclusiveScissorNV-x-02037"},
            {{{1, 0}, {vvl::kI32Max, 16}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02038"},
            {{{vvl::kI32Max, 0}, {1, 16}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02038"},
            {{{0, 0}, {uint32_t{vvl::kI32Max} + 1, 16}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02038"},
            {{{0, 1}, {16, vvl::kI32Max}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02039"},
            {{{0, vvl::kI32Max}, {16, 1}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02039"},
            {{{0, 0}, {16, uint32_t{vvl::kI32Max} + 1}}, "VUID-vkCmdSetExclusiveScissorNV-offset-02039"}};

        for (const auto &test_case : test_cases) {
            m_errorMonitor->SetDesiredError(test_case.vuid.c_str());
            vk::CmdSetExclusiveScissorNV(m_command_buffer.handle(), 0, 1, &test_case.scissor);
            m_errorMonitor->VerifyFound();
        }

        m_command_buffer.End();
    }
}

TEST_F(NegativeCommand, ViewportWScalingNV) {
    TEST_DESCRIPTION("Verify VK_NV_clip_space_w_scaling");

    AddRequiredExtensions(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiViewport);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const char vs_src[] = R"glsl(
        #version 450
        const vec2 positions[] = { vec2(-1.0f,  1.0f),
                                   vec2( 1.0f,  1.0f),
                                   vec2(-1.0f, -1.0f),
                                   vec2( 1.0f, -1.0f) };
        out gl_PerVertex {
            vec4 gl_Position;
        };

        void main() {
            gl_Position = vec4(positions[gl_VertexIndex % 4], 0.0f, 1.0f);
        }
    )glsl";

    const char fs_src[] = R"glsl(
        #version 450
        layout(location = 0) out vec4 outColor;

        void main() {
            outColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        }
    )glsl";

    const std::vector<VkViewport> vp = {
        {0.0f, 0.0f, 64.0f, 64.0f}, {0.0f, 0.0f, 64.0f, 64.0f}, {0.0f, 0.0f, 64.0f, 64.0f}, {0.0f, 0.0f, 64.0f, 64.0f}};
    const std::vector<VkRect2D> sc = {{{0, 0}, {32, 32}}, {{32, 0}, {32, 32}}, {{0, 32}, {32, 32}}, {{32, 32}, {32, 32}}};
    constexpr std::array scale = {VkViewportWScalingNV{-0.2f, -0.2f}, VkViewportWScalingNV{0.2f, -0.2f},
                                  VkViewportWScalingNV{-0.2f, 0.2f}, VkViewportWScalingNV{0.2f, 0.2f}};

    const uint32_t vp_count = static_cast<uint32_t>(vp.size());

    VkPipelineViewportWScalingStateCreateInfoNV vpsi = vku::InitStructHelper();
    vpsi.viewportWScalingEnable = VK_TRUE;
    vpsi.viewportCount = vp_count;
    vpsi.pViewportWScalings = scale.data();

    VkPipelineViewportStateCreateInfo vpci = vku::InitStructHelper(&vpsi);
    vpci.viewportCount = vp_count;
    vpci.pViewports = vp.data();
    vpci.scissorCount = vp_count;
    vpci.pScissors = sc.data();

    const auto set_vpci = [&vpci](CreatePipelineHelper &helper) { helper.vp_state_ci_ = vpci; };

    // Make sure no errors show up when creating the pipeline with w-scaling enabled
    CreatePipelineHelper::OneshotTest(*this, set_vpci, kErrorBit);

    // Create pipeline with w-scaling enabled but without a valid scaling array
    vpsi.pViewportWScalings = nullptr;
    CreatePipelineHelper::OneshotTest(*this, set_vpci, kErrorBit,
                                      std::vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01715"}));

    vpsi.pViewportWScalings = scale.data();

    // Create pipeline with w-scaling enabled but without matching viewport counts
    vpsi.viewportCount = 1;
    CreatePipelineHelper::OneshotTest(
        *this, set_vpci, kErrorBit,
        std::vector<std::string>({"VUID-VkPipelineViewportStateCreateInfo-viewportWScalingEnable-01726"}));

    VkShaderObj vs(this, vs_src, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    vpsi.viewportCount = vp_count;
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.vp_state_ci_ = vpci;
    pipe.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_dynamic(*this);
    pipe_dynamic.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe_dynamic.vp_state_ci_ = vpci;
    pipe_dynamic.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV);
    pipe_dynamic.CreateGraphicsPipeline();

    m_command_buffer.Begin();

    // Bind pipeline without dynamic w scaling enabled
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // Bind pipeline that has dynamic w-scaling enabled
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_dynamic.Handle());

    const auto max_vps = m_device->Physical().limits_.maxViewports;

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetViewportWScalingNV-firstViewport-01324");
    vk::CmdSetViewportWScalingNV(m_command_buffer.handle(), 1, max_vps, scale.data());
    m_errorMonitor->VerifyFound();

    vk::CmdSetViewportWScalingNV(m_command_buffer.handle(), 0, vp_count, scale.data());

    m_command_buffer.End();
}

TEST_F(NegativeCommand, FilterCubicSamplerInCmdDraw) {
    TEST_DESCRIPTION("Verify if sampler is filter cubic, image view needs to support it.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FILTER_CUBIC_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), format, &format_props);
    if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT) == 0) {
        GTEST_SKIP() << "SAMPLED_IMAGE_FILTER_CUBIC_BIT for format is not supported.";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D;

    VkPhysicalDeviceImageViewImageFormatInfoEXT imageview_format_info = vku::InitStructHelper();
    imageview_format_info.imageViewType = imageViewType;
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper(&imageview_format_info);
    image_format_info.type = image_ci.imageType;
    image_format_info.format = image_ci.format;
    image_format_info.tiling = image_ci.tiling;
    image_format_info.usage = image_ci.usage;
    image_format_info.flags = image_ci.flags;

    VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_props = vku::InitStructHelper();
    VkImageFormatProperties2 image_format_properties = vku::InitStructHelper(&filter_cubic_props);

    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_format_info, &image_format_properties);

    if (filter_cubic_props.filterCubic || filter_cubic_props.filterCubicMinmax) {
        GTEST_SKIP() << "Image and ImageView supports filter cubic ; skipped.";
    }

    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView imageView = image.CreateView();

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
    sampler_ci.minFilter = VK_FILTER_CUBIC_EXT;
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;
    vkt::Sampler sampler(*m_device, sampler_ci);
    ASSERT_TRUE(sampler.initialized());

    VkSamplerReductionModeCreateInfo reduction_mode_ci = vku::InitStructHelper();
    reduction_mode_ci.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;
    sampler_ci.pNext = &reduction_mode_ci;
    vkt::Sampler sampler_reduction(*m_device, sampler_ci);

    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    g_pipe.CreateGraphicsPipeline();

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, imageView, sampler_reduction.handle());
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-filterCubicMinmax-02695");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_command_buffer.Reset();

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, imageView, sampler.handle());
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-filterCubic-02694");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ImageFilterCubicSamplerInCmdDraw) {
    TEST_DESCRIPTION("Verify if sampler is filter cubic with the VK_IMG_filter cubic extension that it's a valid ImageViewType.");

    AddRequiredExtensions(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = format;
    image_ci.extent.width = 128;
    image_ci.extent.height = 128;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = usage;
    VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_3D;

    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView imageView = image.CreateView(imageViewType);

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
    sampler_ci.minFilter = VK_FILTER_CUBIC_EXT;
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;
    vkt::Sampler sampler(*m_device, sampler_ci);
    ASSERT_TRUE(sampler.initialized());

    static const char fs_src[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 x;
        void main(){
            x = texture(s, vec3(1));
        }
    )glsl";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    g_pipe.CreateGraphicsPipeline();

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, imageView, sampler.handle());
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-02693");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, CmdUpdateBufferSize) {
    TEST_DESCRIPTION("Update buffer with invalid dataSize");

    RETURN_IF_SKIP(Init());

    uint32_t update_data[4] = {0, 0, 0, 0};
    VkDeviceSize dataSize = sizeof(uint32_t) * 4;
    vkt::Buffer buffer(*m_device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateBuffer-dataSize-00033");
    m_command_buffer.Begin();
    vk::CmdUpdateBuffer(m_command_buffer.handle(), buffer.handle(), sizeof(uint32_t), dataSize, (void *)update_data);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CmdUpdateBufferDstOffset) {
    TEST_DESCRIPTION("Update buffer with invalid dst offset");

    RETURN_IF_SKIP(Init());

    uint32_t update_data[4] = {0, 0, 0, 0};
    VkDeviceSize dataSize = sizeof(uint32_t) * 4;
    vkt::Buffer buffer(*m_device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateBuffer-dstOffset-00032");
    m_command_buffer.Begin();
    vk::CmdUpdateBuffer(m_command_buffer.handle(), buffer.handle(), sizeof(uint32_t) * 8, dataSize, (void *)update_data);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, DescriptorSetPipelineBindPoint) {
    TEST_DESCRIPTION(
        "Attempt to bind descriptor set to a bind point not supported by command pool the command buffer was allocated from");

    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> compute_qfi = m_device->ComputeOnlyQueueFamily();
    if (!compute_qfi) {
        GTEST_SKIP() << "No compute-only queue family, skipping bindpoint and queue tests.";
    }

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::CommandPool command_pool(*m_device, compute_qfi.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pipelineBindPoint-00361");
    vk::CmdBindDescriptorSets(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, DescriptorSetPipelineBindPointMaintenance6) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance6);
    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> compute_qfi = m_device->ComputeOnlyQueueFamily();
    if (!compute_qfi) {
        GTEST_SKIP() << "No compute-only queue family, skipping bindpoint and queue tests.";
    }

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::CommandPool command_pool(*m_device, compute_qfi.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    VkBindDescriptorSetsInfo bind_ds_info = vku::InitStructHelper();
    bind_ds_info.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bind_ds_info.layout = pipeline_layout.handle();
    bind_ds_info.firstSet = 0;
    bind_ds_info.descriptorSetCount = 1;
    bind_ds_info.pDescriptorSets = &descriptor_set.set_;
    bind_ds_info.dynamicOffsetCount = 0;
    bind_ds_info.pDynamicOffsets = nullptr;

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets2-pBindDescriptorSetsInfo-09467");
    vk::CmdBindDescriptorSets2KHR(command_buffer.handle(), &bind_ds_info);
    m_errorMonitor->VerifyFound();

    bind_ds_info.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bind_ds_info.descriptorSetCount = 0;
    m_errorMonitor->SetDesiredError("VUID-VkBindDescriptorSetsInfo-descriptorSetCount-arraylength");
    vk::CmdBindDescriptorSets2KHR(command_buffer.handle(), &bind_ds_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CmdClearColorImageNullColor) {
    TEST_DESCRIPTION("Test invalid null entries for clear color");
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageSubresourceRange isr = {};
    isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    isr.baseArrayLayer = 0;
    isr.baseMipLevel = 0;
    isr.layerCount = 1;
    isr.levelCount = 1;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-pColor-04961");
    vk::CmdClearColorImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, nullptr, 1, &isr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, EndCommandBufferWithConditionalRendering) {
    TEST_DESCRIPTION("Call EndCommandBuffer when conditional rendering is active");
    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT);

    VkConditionalRenderingBeginInfoEXT conditional_rendering_begin = vku::InitStructHelper();
    conditional_rendering_begin.buffer = buffer.handle();

    VkCommandBufferBeginInfo command_buffer_begin = vku::InitStructHelper();

    vk::BeginCommandBuffer(m_command_buffer.handle(), &command_buffer_begin);
    vk::CmdBeginConditionalRenderingEXT(m_command_buffer.handle(), &conditional_rendering_begin);
    m_errorMonitor->SetDesiredError("VUID-vkEndCommandBuffer-None-01978");
    vk::EndCommandBuffer(m_command_buffer.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, DrawBlendEnabledFormatFeatures) {
    TEST_DESCRIPTION("Test pipeline blend enabled with missing image views format features");

    RETURN_IF_SKIP(Init());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    const VkFormat render_format = GetRenderTargetFormat();

    // Set format features from being found
    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(Gpu(), render_format, &formatProps);
    if ((formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0) {
        GTEST_SKIP() << "Required linear tiling features not supported";
    }
    // Gets pass pipeline creation but not the actual tiling used
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
    // will be caught at draw time that feature for optimal image is not set
    // InitRenderTarget() should be setting color attachment as VK_IMAGE_TILING_LINEAR
    formatProps.linearTilingFeatures &= ~VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), render_format, formatProps);

    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.cb_attachments_.blendEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-blendEnable-04727");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, EndConditionalRendering) {
    TEST_DESCRIPTION("Invalid calls to vkCmdEndConditionalRenderingEXT.");

    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

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

    VkSubpassDependency dep = {0,
                               1,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dep;

    vkt::RenderPass render_pass(*m_device, rpci);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();

    vkt::Framebuffer framebuffer(*m_device, render_pass.handle(), 1, &imageView.handle());

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 32;
    buffer_create_info.usage = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
    vkt::Buffer buffer(*m_device, buffer_create_info);

    VkConditionalRenderingBeginInfoEXT conditional_rendering_begin = vku::InitStructHelper();
    conditional_rendering_begin.buffer = buffer.handle();

    VkClearValue clear_value;
    clear_value.color = m_clear_color;

    VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
    rpbi.renderPass = render_pass.handle();
    rpbi.framebuffer = framebuffer.handle();
    rpbi.renderArea = {{0, 0}, {32, 32}};
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = &clear_value;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndConditionalRenderingEXT-None-01985");
    vk::CmdEndConditionalRenderingEXT(m_command_buffer.handle());
    m_errorMonitor->VerifyFound();

    vk::CmdBeginConditionalRenderingEXT(m_command_buffer.handle(), &conditional_rendering_begin);
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndConditionalRenderingEXT-None-01986");
    vk::CmdEndConditionalRenderingEXT(m_command_buffer.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.NextSubpass();
    m_command_buffer.EndRenderPass();
    vk::CmdEndConditionalRenderingEXT(m_command_buffer.handle());

    vk::CmdBeginRenderPass(m_command_buffer.handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginConditionalRenderingEXT(m_command_buffer.handle(), &conditional_rendering_begin);
    m_command_buffer.NextSubpass();
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndConditionalRenderingEXT-None-01987");
    vk::CmdEndConditionalRenderingEXT(m_command_buffer.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
}

TEST_F(NegativeCommand, ResolveUsage) {
    TEST_DESCRIPTION("Resolve image with missing usage flags.");

    RETURN_IF_SKIP(Init());
    const VkPhysicalDeviceLimits &dev_limits = m_device->Physical().limits_;
    if ((dev_limits.sampledImageColorSampleCounts & VK_SAMPLE_COUNT_2_BIT) == 0) {
        GTEST_SKIP() << "Required VkSampleCountFlagBits are not supported; skipping";
    }

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkFormat src_format = VK_FORMAT_R8_UNORM;
    VkFormat dst_format = VK_FORMAT_R8_SNORM;

    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(Gpu(), src_format, &formatProps);
    formatProps.optimalTilingFeatures &= ~VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), src_format, formatProps);
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(Gpu(), dst_format, &formatProps);
    formatProps.optimalTilingFeatures &= ~VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), dst_format, formatProps);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {32, 1, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = 0;
    vkt::Image srcImage(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = dst_format;

    // Some implementations don't support multisampling, check that image format is valid
    VkImageFormatProperties image_format_props{};
    VkResult result =
        vk::GetPhysicalDeviceImageFormatProperties(Gpu(), dst_format, image_create_info.imageType, image_create_info.tiling,
                                                   image_create_info.usage, image_create_info.flags, &image_format_props);
    bool src_image_2_tests_valid = false;
    vkt::Image srcImage2;
    if ((result == VK_SUCCESS) && (image_format_props.sampleCounts & VK_SAMPLE_COUNT_4_BIT) != 0) {
        srcImage2.init(*m_device, image_create_info, 0);
        src_image_2_tests_valid = true;
    }

    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image invalidSrcImage(*m_device, image_create_info, vkt::set_layout);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.format = src_format;
    vkt::Image invalidSrcImage2(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkt::Image dstImage(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = src_format;
    vkt::Image dstImage2(*m_device, image_create_info, vkt::set_layout);

    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image invalidDstImage(*m_device, image_create_info, vkt::set_layout);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.format = dst_format;
    vkt::Image invalidDstImage2(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.srcOffset = {0, 0, 0};
    resolveRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.dstOffset = {0, 0, 0};
    resolveRegion.extent = {1, 1, 1};

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcImage-06762");
    vk::CmdResolveImage(m_command_buffer.handle(), invalidSrcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstImage-06764");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, invalidDstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcImage-06763");
    vk::CmdResolveImage(m_command_buffer.handle(), invalidSrcImage2.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage2.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();

    if (src_image_2_tests_valid) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstImage-06765");
        vk::CmdResolveImage(m_command_buffer.handle(), srcImage2.handle(), VK_IMAGE_LAYOUT_GENERAL, invalidDstImage2.handle(),
                            VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
        m_errorMonitor->VerifyFound();
    }

    m_command_buffer.End();
}

TEST_F(NegativeCommand, DepthStencilStateForReadOnlyLayout) {
    TEST_DESCRIPTION("invalid depth stencil state for subpass that uses read only image layout.");

    RETURN_IF_SKIP(Init());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image ds_image(*m_device, 32, 32, 1, ds_format,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    ds_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView image_view = ds_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(ds_format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL});
    rp.AddDepthStencilAttachment(0);
    rp.CreateRenderPass();

    VkPipelineDepthStencilStateCreateInfo depth_state_info = vku::InitStructHelper();
    depth_state_info.depthTestEnable = VK_TRUE;
    depth_state_info.depthWriteEnable = VK_TRUE;

    VkPipelineDepthStencilStateCreateInfo stencil_state_info = vku::InitStructHelper();
    stencil_state_info.front.failOp = VK_STENCIL_OP_ZERO;
    stencil_state_info.front.writeMask = 1;
    stencil_state_info.back.writeMask = 1;

    VkPipelineDepthStencilStateCreateInfo stencil_disabled_state_info = vku::InitStructHelper();
    stencil_disabled_state_info.front.failOp = VK_STENCIL_OP_ZERO;
    stencil_disabled_state_info.front.writeMask = 1;
    stencil_disabled_state_info.back.writeMask = 0;

    CreatePipelineHelper depth_pipe(*this);
    depth_pipe.LateBindPipelineInfo();
    depth_pipe.gp_ci_.renderPass = rp.Handle();
    depth_pipe.gp_ci_.pDepthStencilState = &depth_state_info;
    depth_pipe.CreateGraphicsPipeline(false);

    CreatePipelineHelper stencil_pipe(*this);
    stencil_pipe.LateBindPipelineInfo();
    stencil_pipe.gp_ci_.renderPass = rp.Handle();
    stencil_pipe.gp_ci_.pDepthStencilState = &stencil_state_info;
    stencil_pipe.CreateGraphicsPipeline(false);

    CreatePipelineHelper stencil_disabled_pipe(*this);
    stencil_disabled_pipe.LateBindPipelineInfo();
    stencil_disabled_pipe.gp_ci_.renderPass = rp.Handle();
    stencil_disabled_pipe.gp_ci_.pDepthStencilState = &stencil_disabled_state_info;
    stencil_disabled_pipe.CreateGraphicsPipeline(false);

    CreatePipelineHelper stencil_dynamic_pipe(*this);
    stencil_dynamic_pipe.AddDynamicState(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
    stencil_dynamic_pipe.LateBindPipelineInfo();
    stencil_dynamic_pipe.gp_ci_.renderPass = rp.Handle();
    stencil_dynamic_pipe.gp_ci_.pDepthStencilState = &stencil_state_info;
    stencil_dynamic_pipe.CreateGraphicsPipeline(false);

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view.handle());

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), 32, 32);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, depth_pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06886");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, stencil_pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06887");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // valid since writeMask is set to zero
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, stencil_disabled_pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, stencil_dynamic_pipe.Handle());
    vk::CmdSetStencilWriteMask(m_command_buffer.handle(), VK_STENCIL_FACE_FRONT_AND_BACK, 1);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06887");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // valid since writeMask is set to zero
    vk::CmdSetStencilWriteMask(m_command_buffer.handle(), VK_STENCIL_FACE_FRONT_BIT, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ClearColorImageWithRange) {
    TEST_DESCRIPTION("Record clear color with an invalid VkImageSubresourceRange");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    m_command_buffer.Begin();
    const auto cb_handle = m_command_buffer.handle();

    // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-baseMipLevel-01470");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-baseMipLevel-01470");
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-pRanges-01692");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try levelCount = 0
    {
        m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceRange-levelCount-01720");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-pRanges-01692");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-baseArrayLayer-01472");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-baseArrayLayer-01472");
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-pRanges-01693");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try layerCount = 0
    {
        m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceRange-layerCount-01721");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer + layerCount > image.arrayLayers
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-pRanges-01693");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeCommand, ClearDepthStencilWithAspect) {
    TEST_DESCRIPTION("Verify ClearDepth with an invalid VkImageAspectFlags.");
    RETURN_IF_SKIP(Init());
    const auto depth_format = FindSupportedDepthStencilFormat(Gpu());

    const VkClearDepthStencilValue clear_value = {};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();

    range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    // Element of pRanges.aspect includes VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the
    // VkImageCreateInfo::usage used to create image
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-02659");
    // Element of pRanges.aspect includes VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the
    // VkImageCreateInfo::usage used to create image
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-02660");
    // ... since VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the VkImageCreateInfo::usage used to create image
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-02659");
    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), image.handle(), image.Layout(), &clear_value, 1, &range);
    m_errorMonitor->VerifyFound();

    // Using stencil aspect when format only have depth
    const VkFormat depth_only_format = FindSupportedDepthOnlyFormat(Gpu());
    if (depth_only_format != VK_FORMAT_UNDEFINED) {
        image_ci.format = depth_only_format;
        image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        vkt::Image depth_image(*m_device, image_ci, vkt::set_layout);

        m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-image-02825");
        vk::CmdClearDepthStencilImage(m_command_buffer.handle(), depth_image.handle(), depth_image.Layout(), &clear_value, 1,
                                      &range);
        m_errorMonitor->VerifyFound();
    }

    m_command_buffer.End();
}

TEST_F(NegativeCommand, ClearDepthStencilWithAspectSeparate) {
    TEST_DESCRIPTION("Verify ClearDepth with an invalid VkImageAspectFlags.");
    AddRequiredExtensions(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const auto depth_format = FindSupportedDepthStencilFormat(Gpu());

    const VkClearDepthStencilValue clear_value = {};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();

    VkImageStencilUsageCreateInfo image_stencil_create_info = vku::InitStructHelper();
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;  // not VK_IMAGE_USAGE_TRANSFER_DST_BIT

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    image_ci.pNext = &image_stencil_create_info;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    // Element of pRanges.aspect includes VK_IMAGE_ASPECT_STENCIL_BIT, and image was created with separate stencil usage,
    // VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the VkImageStencilUsageCreateInfo::stencilUsage used to create image
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-02658");
    // ... since VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the VkImageCreateInfo::usage used to create image
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-02659");
    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), image.handle(), image.Layout(), &clear_value, 1, &range);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCommand, ClearDepthStencilWithRange) {
    TEST_DESCRIPTION("Record clear depth with an invalid VkImageSubresourceRange");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const auto depth_format = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image image(*m_device, 32, 32, 1, depth_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    const VkImageAspectFlags ds_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    image.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkClearDepthStencilValue clear_value = {};

    m_command_buffer.Begin();
    const auto cb_handle = m_command_buffer.handle();

    // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474");
        const VkImageSubresourceRange range = {ds_aspect, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474");
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-01694");
        const VkImageSubresourceRange range = {ds_aspect, 1, 1, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try levelCount = 0
    {
        m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceRange-levelCount-01720");
        const VkImageSubresourceRange range = {ds_aspect, 0, 0, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-01694");
        const VkImageSubresourceRange range = {ds_aspect, 0, 2, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476");
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-01695");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 1, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try layerCount = 0
    {
        m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceRange-layerCount-01721");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 0};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer + layerCount > image.arrayLayers
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-01695");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 2};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeCommand, ClearColorImageWithinRenderPass) {
    // Call CmdClearColorImage within an active RenderPass
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-renderpass");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    VkClearColorValue clear_color;
    memset(clear_color.uint32, 0, sizeof(uint32_t) * 4);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM,
                                                  VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image dst_image(*m_device, image_ci, vkt::set_layout);

    const VkImageSubresourceRange range = vkt::Image::SubresourceRange(image_ci, VK_IMAGE_ASPECT_COLOR_BIT);

    vk::CmdClearColorImage(m_command_buffer.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &range);

    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ClearDepthStencilImage) {
    // Hit errors related to vk::CmdClearDepthStencilImage()
    // 1. Use an image that doesn't have VK_IMAGE_USAGE_TRANSFER_DST_BIT set
    // 2. Call CmdClearDepthStencilImage within an active RenderPass

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    auto depth_format = FindSupportedDepthStencilFormat(Gpu());

    VkClearDepthStencilValue clear_value = {0};
    VkImageCreateInfo image_create_info = vkt::Image::CreateInfo();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Error here is that VK_IMAGE_USAGE_TRANSFER_DST_BIT is excluded for DS image that we'll call Clear on below
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image dst_image_bad_usage(*m_device, image_create_info, vkt::set_layout);
    const VkImageSubresourceRange range = vkt::Image::SubresourceRange(image_create_info, VK_IMAGE_ASPECT_DEPTH_BIT);

    m_command_buffer.Begin();
    // need to handle since an element of pRanges includes VK_IMAGE_ASPECT_DEPTH_BIT without VkImageCreateInfo::usage having
    // VK_IMAGE_USAGE_TRANSFER_DST_BIT being set
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-02660");
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-pRanges-02659");
    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), dst_image_bad_usage.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,
                                  &range);
    m_errorMonitor->VerifyFound();

    // Fix usage for next test case
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image dst_image(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-renderpass");
    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &range);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ClearDepthRangeUnrestricted) {
    TEST_DESCRIPTION("Test clearing without VK_EXT_depth_range_unrestricted");

    // Extension doesn't have feature bit, so not enabling extension invokes restrictions
    RETURN_IF_SKIP(Init());

    // Need to set format framework uses for InitRenderTarget
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());

    int depth_attachment_index = 1;
    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view.handle());

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkClearDepthStencilValue-depth-00022");
    const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
    const VkClearDepthStencilValue bad_clear_value = {1.5f, 0};
    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), m_depthStencil->handle(), VK_IMAGE_LAYOUT_GENERAL, &bad_clear_value, 1,
                                  &range);
    m_errorMonitor->VerifyFound();

    m_renderPassClearValues[depth_attachment_index].depthStencil.depth = 1.5f;
    m_errorMonitor->SetDesiredError("VUID-VkClearDepthStencilValue-depth-00022");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    // set back to normal
    m_renderPassClearValues[depth_attachment_index].depthStencil.depth = 1.0f;
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-VkClearDepthStencilValue-depth-00022");
    VkClearAttachment clear_attachment;
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    clear_attachment.clearValue.depthStencil.depth = 1.5f;
    clear_attachment.clearValue.depthStencil.stencil = 0;
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}, 0, 1};
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &clear_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ClearColorImageImageLayout) {
    TEST_DESCRIPTION("Check ClearImage layouts with SHARED_PRESENTABLE_IMAGE extension active.");
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 4, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image dst_image(*m_device, image_ci, vkt::set_layout);
    m_command_buffer.Begin();

    VkClearColorValue color_clear_value = {};
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    // Fail by using bad layout for color clear (GENERAL, SHARED_PRESENT or TRANSFER_DST are permitted).
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-imageLayout-01394");
    vk::CmdClearColorImage(m_command_buffer.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &color_clear_value, 1,
                           &clear_range);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, CmdClearAttachmentTests) {
    TEST_DESCRIPTION("Various tests for validating usage of vkCmdClearAttachments");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkImageFormatProperties image_format_properties{};
    ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceImageFormatProperties(m_device->Physical().handle(), m_renderTargets[0]->Format(),
                                                                     VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                                     m_renderTargets[0]->Usage(), 0, &image_format_properties));
    if (image_format_properties.maxArrayLayers < 4) {
        GTEST_SKIP() << "Test needs to create image 2D array of 4 image view, but VkImageFormatProperties::maxArrayLayers is < 4. "
                        "Skipping test.";
    }

    // Create frame buffer with 2 layers, and image view with 4 layers,
    // to make sure that considered layer count is the one coming from frame buffer
    // (test would not fail if layer count used to do validation was 4)
    assert(!m_renderTargets.empty());
    const auto render_target_ci = vkt::Image::ImageCreateInfo2D(m_renderTargets[0]->Width(), m_renderTargets[0]->Height(),
                                                                m_renderTargets[0]->CreateInfo().mipLevels, 4,
                                                                m_renderTargets[0]->Format(), m_renderTargets[0]->Usage());
    vkt::Image render_target(*m_device, render_target_ci, vkt::set_layout);
    vkt::ImageView render_target_view =
        render_target.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, render_target_ci.arrayLayers);
    VkFramebufferCreateInfo fb_info = vku::InitStructHelper();
    fb_info.renderPass = RenderPass();
    fb_info.layers = 2;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &render_target_view.handle();
    fb_info.width = m_width;
    fb_info.height = m_height;
    vkt::Framebuffer framebuffer(*m_device, fb_info);
    m_renderPassBeginInfo.framebuffer = framebuffer.handle();

    // Create secondary command buffer
    VkCommandBufferAllocateInfo secondary_cmd_buffer_alloc_info = vku::InitStructHelper();
    secondary_cmd_buffer_alloc_info.commandPool = m_command_pool.handle();
    secondary_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondary_cmd_buffer_alloc_info.commandBufferCount = 1;

    vkt::CommandBuffer secondary_cmd_buffer(*m_device, secondary_cmd_buffer_alloc_info);
    VkCommandBufferInheritanceInfo secondary_cmd_buffer_inheritance_info = vku::InitStructHelper();
    secondary_cmd_buffer_inheritance_info.renderPass = m_renderPass;
    secondary_cmd_buffer_inheritance_info.framebuffer = framebuffer.handle();

    VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info = vku::InitStructHelper();
    secondary_cmd_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary_cmd_buffer_begin_info.pInheritanceInfo = &secondary_cmd_buffer_inheritance_info;

    // Create clear rect
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    auto clear_cmds = [this, &color_attachment](VkCommandBuffer cmd_buffer, VkClearRect clear_rect) {
        // extent too wide
        VkClearRect clear_rect_too_large = clear_rect;
        clear_rect_too_large.rect.extent.width = m_renderPassBeginInfo.renderArea.extent.width + 4;
        clear_rect_too_large.rect.extent.height = clear_rect_too_large.rect.extent.height / 2;
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-pRects-00016");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect_too_large);

        // baseLayer < render pass instance layer count
        clear_rect.baseArrayLayer = 1;
        clear_rect.layerCount = 1;
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer + layerCount <= render pass instance layer count
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 2;
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer >= render pass instance layer count
        clear_rect.baseArrayLayer = 2;
        clear_rect.layerCount = 1;
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-pRects-06937");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);

        // baseLayer + layerCount > render pass instance layer count
        clear_rect.baseArrayLayer = 0;
        clear_rect.layerCount = 4;
        m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-pRects-06937");
        vk::CmdClearAttachments(cmd_buffer, 1, &color_attachment, 1, &clear_rect);
    };

    // Register clear commands to secondary command buffer
    secondary_cmd_buffer.Begin(&secondary_cmd_buffer_begin_info);
    clear_cmds(secondary_cmd_buffer.handle(), clear_rect);
    secondary_cmd_buffer.End();

    m_command_buffer.Begin();

    // Execute secondary command buffer
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_cmd_buffer.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();

    // Execute same commands as previously, but in a primary command buffer
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    clear_cmds(m_command_buffer.handle(), clear_rect);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();

    m_command_buffer.End();
}

TEST_F(NegativeCommand, BindVertexIndexBufferUsage) {
    TEST_DESCRIPTION("Bad usage flags for binding the Vertex and Index buffer");
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindIndexBuffer-buffer-08784");
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    m_errorMonitor->VerifyFound();

    VkDeviceSize offsets = 0;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindVertexBuffers-pBuffers-00627");
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offsets);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCommand, BindIndexBufferHandles) {
    TEST_DESCRIPTION("call vkCmdBindIndexBuffer with bad Handles");
    RETURN_IF_SKIP(Init());
    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_command_buffer.Begin();
    VkBuffer bad_buffer = CastToHandle<VkBuffer, uintptr_t>(0xbaadbeef);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindIndexBuffer-buffer-parameter");
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), bad_buffer, 0, VK_INDEX_TYPE_UINT32);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ClearImageAspectMask) {
    TEST_DESCRIPTION("Need to use VK_IMAGE_ASPECT_COLOR_BIT.");

    RETURN_IF_SKIP(Init());

    VkClearColorValue clear_color;
    memset(clear_color.uint32, 0, sizeof(uint32_t) * 4);
    const VkImageSubresourceRange color_range = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-aspectMask-02498");
    vk::CmdClearColorImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &color_range);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, RenderPassContinueNotSupportedByCommandPool) {
    TEST_DESCRIPTION("Use render pass continue bit with unsupported command pool.");

    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> non_graphics_queue_family_index = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);

    if (!non_graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    vkt::CommandPool command_pool(*m_device, non_graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-09123");
    vk::BeginCommandBuffer(command_buffer.handle(), &begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, ClearDepthStencilImageWithInvalidAspect) {
    TEST_DESCRIPTION("Use vkCmdClearDepthStencilImage with invalid image aspect.");

    RETURN_IF_SKIP(Init());

    VkFormat format = FindSupportedDepthStencilFormat(m_device->Physical().handle());
    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkClearDepthStencilValue clear_value = {0};
    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0u;
    range.layerCount = 1u;
    range.baseArrayLayer = 0u;
    range.levelCount = 1u;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearDepthStencilImage-aspectMask-02824");
    vk::CmdClearDepthStencilImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1u, &range);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ClearColorImageWithMissingFeature) {
    TEST_DESCRIPTION("Use vkCmdClearColorImage with image format that doesnt have VK_FORMAT_FEATURE_TRANSFER_DST_BIT.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(Gpu(), format, &formatProps);
    formatProps.optimalTilingFeatures &= ~VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), format, formatProps);

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0u;
    range.layerCount = 1u;
    range.baseArrayLayer = 0u;
    range.levelCount = 1u;

    VkClearColorValue clear_value = {{0, 0, 0, 0}};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-image-01993");
    vk::CmdClearColorImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1u, &range);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCommand, ClearDsImageWithInvalidAspect) {
    TEST_DESCRIPTION("Attempt to clear color aspect of depth/stencil image.");

    RETURN_IF_SKIP(Init());

    for (uint32_t i = 0; i < 2; ++i) {
        bool missing_depth = i == 0;
        auto format = missing_depth ? FindSupportedStencilOnlyFormat(m_device->Physical().handle())
                                    : FindSupportedDepthOnlyFormat(m_device->Physical().handle());
        if (format == VK_FORMAT_UNDEFINED) {
            continue;
        }
        VkImageAspectFlags image_aspect = missing_depth ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
        VkImageAspectFlags clear_aspect = missing_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_STENCIL_BIT;

        vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        vkt::ImageView image_view = image.CreateView(image_aspect);

        RenderPassSingleSubpass rp(*this);
        rp.AddAttachmentDescription(format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
        rp.AddDepthStencilAttachment(0);
        rp.CreateRenderPass();

        vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &image_view.handle());

        VkClearValue clear_value = {};
        clear_value.depthStencil = {0};

        VkClearAttachment clear_attachment = {};
        clear_attachment.aspectMask = clear_aspect;
        clear_attachment.colorAttachment = 0u;
        clear_attachment.clearValue.depthStencil.depth = 0.0f;
        VkClearRect clear_rect;
        clear_rect.rect = {{0, 0}, {32u, 32u}};
        clear_rect.baseArrayLayer = 0u;
        clear_rect.layerCount = 1u;

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), 32, 32, 1, &clear_value);
        if (missing_depth) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-aspectMask-07884");
        } else {
            m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-aspectMask-07885");
        }
        vk::CmdClearAttachments(m_command_buffer.handle(), 1u, &clear_attachment, 1u, &clear_rect);
        m_errorMonitor->VerifyFound();
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
}

TEST_F(NegativeCommand, InvalidCommandBufferInheritanceInfo) {
    TEST_DESCRIPTION("Test VkCommandBufferInheritanceInfo using disabled featuers.");

    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceConditionalRenderingInfoEXT inheritance_conditional_rendering_info = vku::InitStructHelper();
    inheritance_conditional_rendering_info.conditionalRenderingEnable = VK_TRUE;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&inheritance_conditional_rendering_info);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.pInheritanceInfo = &inheritance_info;

    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceConditionalRenderingInfoEXT-conditionalRenderingEnable-01977");
    vk::BeginCommandBuffer(secondary_cb.handle(), &begin_info);
    m_errorMonitor->VerifyFound();

    inheritance_info.pNext = nullptr;
    inheritance_info.occlusionQueryEnable = VK_TRUE;
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceInfo-occlusionQueryEnable-00056");
    vk::BeginCommandBuffer(secondary_cb.handle(), &begin_info);
    m_errorMonitor->VerifyFound();

    inheritance_info.occlusionQueryEnable = VK_FALSE;
    inheritance_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-00058");
    vk::BeginCommandBuffer(secondary_cb.handle(), &begin_info);
    m_errorMonitor->VerifyFound();

    inheritance_info.pipelineStatistics = 0u;
    inheritance_info.queryFlags = VK_QUERY_CONTROL_PRECISE_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceInfo-queryFlags-02788");
    vk::BeginCommandBuffer(secondary_cb.handle(), &begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, PipelineStatisticsInvalidFlags) {
    TEST_DESCRIPTION("Use invalid pipelineStatistics flags.");

    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.pInheritanceInfo = &inheritance_info;

    inheritance_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_FLAG_BITS_MAX_ENUM;  // Invalid
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-02789");
    vk::BeginCommandBuffer(secondary_cb.handle(), &begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCommand, QueryControlInvalidFlags) {
    TEST_DESCRIPTION("Use invalid query control flags.");

    AddRequiredFeature(vkt::Feature::inheritedQueries);
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.pInheritanceInfo = &inheritance_info;

    inheritance_info.queryFlags = VK_QUERY_CONTROL_FLAG_BITS_MAX_ENUM;  // Invalid
    m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceInfo-queryFlags-00057");
    vk::BeginCommandBuffer(secondary_cb.handle(), &begin_info);
    m_errorMonitor->VerifyFound();
}
