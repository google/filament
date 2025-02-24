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
#include "../framework/render_pass_helper.h"

void DynamicStateTest::InitBasicExtendedDynamicState() {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
}

class PositiveDynamicState : public DynamicStateTest {};

TEST_F(PositiveDynamicState, DiscardRectanglesVersion) {
    TEST_DESCRIPTION("check version of VK_EXT_discard_rectangles");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    if (!InstanceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_EXT_discard_rectangles version 2";
    }
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetDiscardRectangleEnableEXT(m_command_buffer.handle(), VK_TRUE);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, ViewportWithCountNoMultiViewport) {
    TEST_DESCRIPTION("DynamicViewportWithCount/ScissorWithCount without multiViewport feature not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    pipe.vp_state_ci_.viewportCount = 0;
    pipe.vp_state_ci_.scissorCount = 0;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, CmdSetVertexInputEXT) {
    TEST_DESCRIPTION("Test CmdSetVertexInputEXT");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Fill with bad data as should be ignored with dynamic state
    VkVertexInputBindingDescription input_binding = {5, 7, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attrib = {5, 7, VK_FORMAT_UNDEFINED, 9};
    VkPipelineVertexInputStateCreateInfo vi_ci = vku::InitStructHelper();
    vi_ci.pVertexBindingDescriptions = &input_binding;
    vi_ci.vertexBindingDescriptionCount = 1;
    vi_ci.pVertexAttributeDescriptions = &input_attrib;
    vi_ci.vertexAttributeDescriptionCount = 1;

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.gp_ci_.pVertexInputState = &vi_ci;  // ignored
    pipe.CreateGraphicsPipeline();

    VkVertexInputBindingDescription2EXT binding = vku::InitStructHelper();
    binding.binding = 0;
    binding.stride = sizeof(float);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.divisor = 1;
    VkVertexInputAttributeDescription2EXT attribute = vku::InitStructHelper();
    attribute.location = 0;
    attribute.binding = 0;
    attribute.format = VK_FORMAT_R32_SFLOAT;
    attribute.offset = 0;

    vkt::Buffer vtx_buf(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vtx_buf.handle(), &offset);
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, &binding, 1, &attribute);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, CmdSetVertexInputEXTStride) {
    TEST_DESCRIPTION("Test CmdSetVertexInputEXT");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    pipe.gp_ci_.pVertexInputState = nullptr;
    pipe.CreateGraphicsPipeline();

    VkVertexInputBindingDescription2EXT binding = vku::InitStructHelper();
    binding.binding = 0;
    binding.stride = sizeof(float);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.divisor = 1;
    VkVertexInputAttributeDescription2EXT attribute = vku::InitStructHelper();
    attribute.location = 0;
    attribute.binding = 0;
    attribute.format = VK_FORMAT_R32_SFLOAT;
    attribute.offset = 0;

    vkt::Buffer vtx_buf(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vtx_buf.handle(), &offset);
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 1, &binding, 1, &attribute);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, ExtendedDynamicStateBindVertexBuffersMaintenance5) {
    TEST_DESCRIPTION("VK_KHR_maintenance5 lets you use VK_WHOLE_SIZE with VK_EXT_extended_dynamic_state");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();
    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize size = VK_WHOLE_SIZE;
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers2EXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, &size, nullptr);
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DiscardRectanglesWithDynamicState) {
    TEST_DESCRIPTION("Don't check discard rectangles if dynamic state is not set");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // pass in struct, but don't set the dynamic state in the pipeline
    VkPipelineDiscardRectangleStateCreateInfoEXT discard_rect_ci = vku::InitStructHelper();
    discard_rect_ci.discardRectangleMode = VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT;
    discard_rect_ci.discardRectangleCount = 4;
    std::vector<VkRect2D> discard_rectangles(4);
    discard_rect_ci.pDiscardRectangles = discard_rectangles.data();

    CreatePipelineHelper pipe(*this, &discard_rect_ci);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DynamicColorWriteNoColorAttachments) {
    TEST_DESCRIPTION("Create a graphics pipeline with no color attachments, but use dynamic color write enable.");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::colorWriteEnable);
    RETURN_IF_SKIP(Init());

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());
    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view.handle());

    CreatePipelineHelper pipe(*this);

    // Create a render pass without any color attachments
    VkAttachmentReference attach = {};
    attach.attachment = 0;
    attach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpasses = {};
    subpasses.pDepthStencilAttachment = &attach;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = m_depth_stencil_fmt;
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
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpasses;
    vkt::RenderPass rp(*m_device, rpci);
    vkt::Framebuffer fb(*m_device, rp.handle(), 1, &depth_image_view.handle(), m_width, m_height);

    // Enable dynamic color write enable
    pipe.gp_ci_.renderPass = rp.handle();
    // pColorBlendState is not required since there are no color attachments
    pipe.gp_ci_.pColorBlendState = nullptr;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT);
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.ds_ci_.depthTestEnable = VK_TRUE;
    pipe.ds_ci_.stencilTestEnable = VK_TRUE;
    ASSERT_EQ(VK_SUCCESS, pipe.CreateGraphicsPipeline());

    m_command_buffer.Begin();
    m_renderPassBeginInfo.renderPass = rp.handle();
    m_renderPassBeginInfo.framebuffer = fb.handle();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkBool32 color_write_enable = VK_TRUE;
    vk::CmdSetColorWriteEnableEXT(m_command_buffer.handle(), 1, &color_write_enable);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DepthTestEnableOverridesPipelineDepthWriteEnable) {
    RETURN_IF_SKIP(InitBasicExtendedDynamicState());

    vkt::Image color_image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    auto color_view = color_image.CreateView();

    VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image ds_image(*m_device, m_width, m_height, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    auto ds_view = ds_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentDescription(ds_format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.AddDepthStencilAttachment(1);
    rp.CreateRenderPass();
    VkImageView views[2] = {color_view.handle(), ds_view.handle()};
    vkt::Framebuffer fb(*m_device, rp.Handle(), 2, views);

    VkPipelineDepthStencilStateCreateInfo ds_state = vku::InitStructHelper();
    ds_state.depthWriteEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.ds_ci_ = ds_state;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());

    vk::CmdSetDepthTestEnableEXT(m_command_buffer.handle(), VK_FALSE);

    vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DepthTestEnableOverridesDynamicDepthWriteEnable) {
    TEST_DESCRIPTION("setting vkCmdSetDepthTestEnable to false cancels what ever is written to vkCmdSetDepthWriteEnable.");
    RETURN_IF_SKIP(InitBasicExtendedDynamicState());

    vkt::Image color_image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    auto color_view = color_image.CreateView();

    VkFormat ds_format = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image ds_image(*m_device, m_width, m_height, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    auto ds_view = ds_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentDescription(ds_format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.AddDepthStencilAttachment(1);
    rp.CreateRenderPass();
    VkImageView views[2] = {color_view.handle(), ds_view.handle()};
    vkt::Framebuffer fb(*m_device, rp.Handle(), 2, views);

    VkPipelineDepthStencilStateCreateInfo ds_state = vku::InitStructHelper();
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.ds_ci_ = ds_state;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());

    vk::CmdSetDepthTestEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetDepthWriteEnableEXT(m_command_buffer.handle(), VK_TRUE);

    vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, SetBeforePipeline) {
    TEST_DESCRIPTION("Pipeline set state, but prior to last bound pipeline that had it");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe_line(*this);
    pipe_line.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
    pipe_line.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_blend(*this);
    pipe_blend.AddDynamicState(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    pipe_blend.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdSetLineWidth(m_command_buffer.handle(), 1.0f);
    float blends[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    vk::CmdSetBlendConstants(m_command_buffer.handle(), blends);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_line.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_blend.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, AttachmentFeedbackLoopEnable) {
    TEST_DESCRIPTION("Use vkCmdSetAttachmentFeedbackLoopEnableEXT correctly");
    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFeedbackLoopDynamicState);
    AddRequiredFeature(vkt::Feature::attachmentFeedbackLoopLayout);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdSetAttachmentFeedbackLoopEnableEXT(m_command_buffer.handle(), VK_IMAGE_ASPECT_COLOR_BIT);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();

    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, SetDepthBias2EXTDepthBiasClampEnabled) {
    TEST_DESCRIPTION("Call vkCmdSetDepthBias2EXT with VkPhysicalDeviceFeatures::depthBiasClamp feature enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);
    AddRequiredFeature(vkt::Feature::depthBiasControl);
    AddRequiredFeature(vkt::Feature::depthBiasClamp);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create a pipeline with a dynamically set depth bias
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
    VkPipelineRasterizationStateCreateInfo raster_state = vku::InitStructHelper();
    raster_state.depthBiasEnable = VK_TRUE;
    raster_state.lineWidth = 1.0f;
    pipe.rs_state_ci_ = raster_state;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();

    vk::CmdSetDepthBiasEnableEXT(m_command_buffer.handle(), VK_TRUE);

    VkDepthBiasInfoEXT depth_bias_info = vku::InitStructHelper();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 1.0f;
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    vk::CmdSetDepthBias2EXT(m_command_buffer.handle(), &depth_bias_info);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0,
                0);  // Without correct state tracking, VUID-vkCmdDraw-None-07834 would be thrown here
    m_command_buffer.EndRenderPass();

    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, SetDepthBias2EXTDepthBiasClampDisabled) {
    TEST_DESCRIPTION("Call vkCmdSetDepthBias2EXT with VkPhysicalDeviceFeatures::depthBiasClamp feature disabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create a pipeline with a dynamically set depth bias
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
    VkPipelineRasterizationStateCreateInfo raster_state = vku::InitStructHelper();
    raster_state.depthBiasEnable = VK_TRUE;
    raster_state.lineWidth = 1.0f;
    pipe.rs_state_ci_ = raster_state;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();

    vk::CmdSetDepthBiasEnableEXT(m_command_buffer.handle(), VK_TRUE);

    VkDepthBiasInfoEXT depth_bias_info = vku::InitStructHelper();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 0.0f;  // depthBiasClamp feature is disabled, so depth_bias_info.depthBiasClamp must be 0
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    vk::CmdSetDepthBias2EXT(m_command_buffer.handle(), &depth_bias_info);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0,
                0);  // Without correct state tracking, VUID-vkCmdDraw-None-07834 would be thrown here
    m_command_buffer.EndRenderPass();

    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, SetDepthBias2EXTDepthBiasWithDepthBiasRepresentationInfo) {
    TEST_DESCRIPTION(
        "Call vkCmdSetDepthBias2EXT with VkDepthBiasRepresentationInfoEXT and VkPhysicalDeviceFeatures::depthBiasClamp and "
        "VkPhysicalDeviceDepthBiasControlFeaturesEXT "
        "features enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);
    AddRequiredFeature(vkt::Feature::leastRepresentableValueForceUnormRepresentation);
    AddRequiredFeature(vkt::Feature::floatRepresentation);
    AddRequiredFeature(vkt::Feature::depthBiasExact);
    AddRequiredFeature(vkt::Feature::depthBiasClamp);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create a pipeline with a dynamically set depth bias
    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
    VkPipelineRasterizationStateCreateInfo raster_state = vku::InitStructHelper();
    raster_state.depthBiasEnable = VK_TRUE;
    raster_state.lineWidth = 1.0f;
    pipe.rs_state_ci_ = raster_state;
    pipe.CreateGraphicsPipeline();
    m_command_buffer.Begin();

    vk::CmdSetDepthBiasEnableEXT(m_command_buffer.handle(), VK_TRUE);

    VkDepthBiasInfoEXT depth_bias_info = vku::InitStructHelper();
    depth_bias_info.depthBiasConstantFactor = 1.0f;
    depth_bias_info.depthBiasClamp = 1.0f;
    depth_bias_info.depthBiasSlopeFactor = 1.0f;
    vk::CmdSetDepthBias2EXT(m_command_buffer.handle(), &depth_bias_info);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // Without correct state tracking, VUID-vkCmdDraw-None-07834 would be thrown here and in the follow-up calls
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    VkDepthBiasRepresentationInfoEXT depth_bias_representation = vku::InitStructHelper();
    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORCE_UNORM_EXT;
    depth_bias_representation.depthBiasExact = VK_TRUE;
    depth_bias_info.pNext = &depth_bias_representation;
    vk::CmdSetDepthBias2EXT(m_command_buffer.handle(), &depth_bias_info);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT;
    vk::CmdSetDepthBias2EXT(m_command_buffer.handle(), &depth_bias_info);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();

    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, AlphaToCoverageSetFalse) {
    TEST_DESCRIPTION("Dynamically set alphaToCoverageEnabled to false so its not checked.");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3AlphaToCoverageEnable);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location = 0) out float x;
        void main(){
            x = 1.0;
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;  // should be ignored

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    pipe.ms_ci_ = ms_state_ci;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetAlphaToCoverageEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, AlphaToCoverageSetTrue) {
    TEST_DESCRIPTION("Dynamically set alphaToCoverageEnabled to true, but have component set.");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3AlphaToCoverageEnable);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetAlphaToCoverageEnableEXT(m_command_buffer.handle(), VK_TRUE);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, MultisampleStateIgnored) {
    TEST_DESCRIPTION("Ignore null pMultisampleState, with alphaToOne disabled");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3RasterizationSamples);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3SampleMask);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3AlphaToCoverageEnable);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3AlphaToOneEnable);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, MultisampleStateIgnoredAlphaToOne) {
    TEST_DESCRIPTION("Ignore null pMultisampleState with alphaToOne enabled");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3RasterizationSamples);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3SampleMask);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3AlphaToCoverageEnable);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3AlphaToOneEnable);
    AddRequiredFeature(vkt::Feature::alphaToOne);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT);
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, InputAssemblyStateIgnored) {
    TEST_DESCRIPTION("Ignore null pInputAssemblyState");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT dynamic_state_3_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dynamic_state_3_props);
    if (!dynamic_state_3_props.dynamicPrimitiveTopologyUnrestricted) {
        GTEST_SKIP() << "dynamicPrimitiveTopologyUnrestricted is VK_FALSE";
    }

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    pipe.gp_ci_.pInputAssemblyState = nullptr;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, ViewportStateIgnored) {
    TEST_DESCRIPTION("Ignore null pViewportState");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_FALSE;
    pipe.gp_ci_.pViewportState = nullptr;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, ColorBlendStateIgnored) {
    TEST_DESCRIPTION("Ignore null pColorBlendState");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2LogicOp);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3LogicOpEnable);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEnable);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEquation);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorWriteMask);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_BLEND_CONSTANTS);

    VkPipelineColorBlendAttachmentState att_state = {};
    att_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
    att_state.blendEnable = VK_TRUE;
    pipe.cb_attachments_ = att_state;
    pipe.gp_ci_.pColorBlendState = nullptr;

    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicState, DepthBoundsTestEnableState) {
    TEST_DESCRIPTION("Dynamically set depthBoundsTestEnable and not call vkCmdSetDepthBounds before the draw");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    RETURN_IF_SKIP(Init());

    // Need to set format framework uses for InitRenderTarget
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());

    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view.handle());

    CreatePipelineHelper pipe(*this);
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.ds_ci_.depthTestEnable = VK_TRUE;  // ignored
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetDepthBoundsTestEnableEXT(m_command_buffer.handle(), VK_FALSE);
    // don't need vkCmdSetDepthBounds since test is disabled now
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, ViewportInheritance) {
    TEST_DESCRIPTION("Dynamically set viewport multiple times");

    AddRequiredExtensions(VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiViewport);
    AddRequiredFeature(vkt::Feature::inheritedViewportScissor2D);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.vp_state_ci_.viewportCount = 2u;
    pipe.vp_state_ci_.scissorCount = 2u;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    pipe.CreateGraphicsPipeline();

    vkt::CommandBuffer cmd_buffer(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer set_state(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkViewport viewports[2] = {{0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f}};
    const VkRect2D scissors[2] = {{{0, 0}, {100u, 100u}}, {{0, 0}, {100u, 100u}}};

    auto viewport_scissor_inheritance = vku::InitStruct<VkCommandBufferInheritanceViewportScissorInfoNV>();
    viewport_scissor_inheritance.viewportScissor2D = VK_TRUE;
    viewport_scissor_inheritance.viewportDepthCount = 2u;
    viewport_scissor_inheritance.pViewportDepths = viewports;

    auto hinfo = vku::InitStruct<VkCommandBufferInheritanceInfo>(&viewport_scissor_inheritance);
    hinfo.renderPass = m_renderPass;
    hinfo.subpass = 0;
    hinfo.framebuffer = Framebuffer();

    auto info = vku::InitStruct<VkCommandBufferBeginInfo>();
    info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    info.pInheritanceInfo = &hinfo;

    cmd_buffer.Begin(&info);
    vk::CmdBindPipeline(cmd_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(cmd_buffer.handle(), 3, 1, 0, 0);
    cmd_buffer.End();

    set_state.Begin();
    vk::CmdSetViewport(set_state.handle(), 1u, 1u, &viewports[1]);
    set_state.End();

    m_command_buffer.Begin();
    vk::CmdSetViewport(m_command_buffer.handle(), 0u, 2u, viewports);
    vk::CmdSetViewport(m_command_buffer.handle(), 0u, 1u, viewports);
    vk::CmdSetScissor(m_command_buffer.handle(), 0u, 2u, scissors);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &cmd_buffer.handle());

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveDynamicState, AttachmentFeedbackLoopEnableAspectMask) {
    TEST_DESCRIPTION("Valid aspect masks for vkCmdSetAttachmentFeedbackLoopEnableEXT");
    AddRequiredExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::attachmentFeedbackLoopDynamicState);
    AddRequiredFeature(vkt::Feature::attachmentFeedbackLoopLayout);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdSetAttachmentFeedbackLoopEnableEXT(m_command_buffer.handle(), VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, RasterizationSamplesDynamicRendering) {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3RasterizationSamples);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = color_format;
    image_ci.extent = {32u, 32u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    vkt::Image resolve_image(*m_device, 32u, 32u, 1, color_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView resolve_image_view = resolve_image.CreateView();

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo colorAttachment = vku::InitStructHelper();
    colorAttachment.imageView = image_view.handle();
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    colorAttachment.resolveImageView = resolve_image_view.handle();
    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.clearValue.color = m_clear_color;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32u, 32u}};
    rendering_info.layerCount = 1u;
    rendering_info.colorAttachmentCount = 1u;
    rendering_info.pColorAttachments = &colorAttachment;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    vk::CmdSetRasterizationSamplesEXT(m_command_buffer.handle(), VK_SAMPLE_COUNT_4_BIT);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 4u, 1u, 0u, 0u);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, RasterizationSamples) {
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3RasterizationSamples);
    RETURN_IF_SKIP(Init());

    VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = color_format;
    image_ci.extent = {32u, 32u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    vkt::Image resolve_image(*m_device, 32u, 32u, 1, color_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView resolve_image_view = resolve_image.CreateView();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(color_format, VK_SAMPLE_COUNT_4_BIT);
    rp.AddAttachmentDescription(color_format, VK_SAMPLE_COUNT_1_BIT);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.AddResolveAttachment(1);
    rp.CreateRenderPass();

    VkImageView attachments[2] = {image_view, resolve_image_view};
    const vkt::Framebuffer fb(*m_device, rp.Handle(), 2, attachments);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), 32u, 32u);
    vk::CmdSetRasterizationSamplesEXT(m_command_buffer.handle(), VK_SAMPLE_COUNT_4_BIT);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 4u, 1u, 0u, 0u);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, PrimitiveTopology) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8028");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    pipe.ia_ci_.primitiveRestartEnable = VK_FALSE;
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetPrimitiveTopologyEXT(m_command_buffer.handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    vk::CmdSetPrimitiveRestartEnableEXT(m_command_buffer.handle(), VK_TRUE);
    vk::CmdDraw(m_command_buffer.handle(), 4u, 1u, 0u, 0u);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, VertexInputMultipleBindings) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8458");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::vertexInputDynamicState);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    struct PerVertex {
        int a;
        float b;
    };

    struct PerInstance {
        uint32_t c;
        float d;
    };

    VkVertexInputBindingDescription2EXT bindings[2];
    bindings[0] = vku::InitStructHelper();
    bindings[0].binding = 0u;
    bindings[0].stride = sizeof(PerVertex);
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[0].divisor = 1u;
    bindings[1] = vku::InitStructHelper();
    bindings[1].binding = 1u;
    bindings[1].stride = 0u;
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    bindings[1].divisor = 1u;

    VkVertexInputAttributeDescription2EXT attributes[4];
    attributes[0] = vku::InitStructHelper();
    attributes[0].location = 0u;
    attributes[0].binding = 0u;
    attributes[0].format = VK_FORMAT_R8_SINT;
    attributes[0].offset = offsetof(PerVertex, a);

    attributes[1] = vku::InitStructHelper();
    attributes[1].location = 1u;
    attributes[1].binding = 0u;
    attributes[1].format = VK_FORMAT_R32_SFLOAT;
    attributes[1].offset = offsetof(PerVertex, b);

    attributes[2] = vku::InitStructHelper();
    attributes[2].location = 2u;
    attributes[2].binding = 1u;
    attributes[2].format = VK_FORMAT_R8_UINT;
    attributes[2].offset = offsetof(PerInstance, c);

    attributes[3] = vku::InitStructHelper();
    attributes[3].location = 3u;
    attributes[3].binding = 1u;
    attributes[3].format = VK_FORMAT_R32_SFLOAT;
    attributes[3].offset = offsetof(PerInstance, d);

    char const *vsSource = R"glsl(
        #version 450
        layout(location = 0) in int a;
        layout(location = 1) in float b;
        layout(location = 2) in uint c;
        layout(location = 3) in float d;

        void main(){
            gl_Position = vec4(float(a), b, float(c), d);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    vkt::Buffer vertex_buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vkt::Buffer instance_buffer(*m_device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkBuffer buffers[2] = {vertex_buffer.handle(), instance_buffer.handle()};
    VkDeviceSize offsets[2] = {0u, 0u};

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 2, buffers, offsets);
    vk::CmdSetVertexInputEXT(m_command_buffer.handle(), 2, bindings, 4, attributes);
    vk::CmdDraw(m_command_buffer.handle(), 3u, 3u, 0u, 0u);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, ColorBlendEquationMultipleAttachments) {
    TEST_DESCRIPTION("Only update some of the dynamic color blend equations");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEquation);
    RETURN_IF_SKIP(Init());
    InitRenderTarget(2);

    VkPipelineColorBlendAttachmentState color_blend[2] = {};
    color_blend[0] = DefaultColorBlendAttachmentState();
    color_blend[1] = DefaultColorBlendAttachmentState();

    CreatePipelineHelper pipe(*this);
    pipe.cb_ci_.attachmentCount = 2;
    pipe.cb_ci_.pAttachments = color_blend;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    const VkColorBlendEquationEXT equation = {VK_BLEND_FACTOR_SRC_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_OP_ADD,
                                              VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD};
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 0, 1, &equation);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 1, 1, &equation);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, MaxFragmentDualSrcAttachmentsDynamicBlendEnable) {
    TEST_DESCRIPTION("Test maxFragmentDualSrcAttachments when blend is not enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dualSrcBlend);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEnable);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEquation);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorWriteMask);
    RETURN_IF_SKIP(Init());

    const uint32_t count = m_device->Physical().limits_.maxFragmentDualSrcAttachments + 1;
    if (count != 2) {
        GTEST_SKIP() << "Test is designed for a maxFragmentDualSrcAttachments of 1";
    }
    InitRenderTarget(count);

    const char *fs_src = R"glsl(
        #version 460
        layout(location = 0) out vec4 c0;
        layout(location = 1) out vec4 c1;
        void main() {
            c0 = vec4(0.0f);
            c1 = vec4(0.0f);
        }
    )glsl";
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineColorBlendAttachmentState cb_attachments = DefaultColorBlendAttachmentState();

    CreatePipelineHelper pipe(*this);
    pipe.cb_ci_.pAttachments = &cb_attachments;
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    VkColorBlendEquationEXT dual_color_blend_equation = {
        VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_SRC_ALPHA,  VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD};
    VkColorBlendEquationEXT normal_color_blend_equation = {
        VK_BLEND_FACTOR_ONE,       VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD};
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 0, 1, &dual_color_blend_equation);
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 1, 1, &normal_color_blend_equation);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // The dual color blend is disabled
    VkBool32 color_blend_enabled[2] = {VK_FALSE, VK_TRUE};
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0, 2, color_blend_enabled);

    VkColorComponentFlags color_component_flags[2] = {VK_COLOR_COMPONENT_R_BIT, VK_COLOR_COMPONENT_R_BIT};
    vk::CmdSetColorWriteMaskEXT(m_command_buffer.handle(), 0, 2, color_component_flags);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DynamicColorBlendEnable) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8444");
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEnable);
    RETURN_IF_SKIP(Init());
    InitDynamicRenderTarget();

    const VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    const VkFormat depth_stencil_format = FindSupportedDepthStencilFormat(Gpu());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = color_format;
    image_ci.extent = {32u, 32u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);
    vkt::Image image3(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view1 = image1.CreateView();
    vkt::ImageView image_view2 = image2.CreateView();
    vkt::ImageView image_view3 = image3.CreateView();

    vkt::Image depth_image(*m_device, 32, 32, 1, depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    vkt::ImageView depth_image_view = depth_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfo color_attachments[3];
    color_attachments[0] = vku::InitStructHelper();
    color_attachments[0].imageView = image_view1.handle();
    color_attachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachments[1] = color_attachments[0];
    color_attachments[1].imageView = image_view2.handle();
    color_attachments[2] = color_attachments[0];
    color_attachments[2].imageView = image_view3.handle();

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = depth_image_view.handle();

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32u, 32u}};
    rendering_info.layerCount = 1u;
    rendering_info.colorAttachmentCount = 3u;
    rendering_info.pColorAttachments = color_attachments;

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.depthAttachmentFormat = depth_stencil_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_stencil_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBeginRenderingKHR(m_command_buffer.handle(), &rendering_info);
    VkBool32 color_blend_enables[] = {VK_TRUE, VK_TRUE, VK_TRUE};
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0u, 3u, color_blend_enables);
    vk::CmdEndRenderingKHR(m_command_buffer.handle());

    rendering_info.colorAttachmentCount = 1u;
    rendering_info.pDepthAttachment = &depth_attachment;

    vk::CmdBeginRenderingKHR(m_command_buffer.handle(), &rendering_info);
    VkBool32 color_blend_disabled = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0u, 1u, &color_blend_disabled);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3u, 1u, 0u, 0u);
    vk::CmdEndRenderingKHR(m_command_buffer.handle());

    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DynamicAdvancedBlendMaxAttachments) {
    TEST_DESCRIPTION("Attempt to use the maximum attachments in subpass when advanced blend is enabled");
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEnable);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendAdvanced);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_advanced_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(blend_advanced_props);
    const uint32_t attachment_count = blend_advanced_props.advancedBlendMaxColorAttachments;

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    std::vector<std::unique_ptr<vkt::Image>> images(attachment_count);
    std::vector<vkt::ImageView> image_views(attachment_count);
    std::vector<VkRenderingAttachmentInfo> rendering_attachment_info(attachment_count);
    for (uint32_t i = 0; i < attachment_count; ++i) {
        images[i] = std::make_unique<vkt::Image>(*m_device, image_ci);
        image_views[i] = images[i]->CreateView();
        rendering_attachment_info[i] = vku::InitStructHelper();
        rendering_attachment_info[i].imageView = image_views[i];
        rendering_attachment_info[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        rendering_attachment_info[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        rendering_attachment_info[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        rendering_attachment_info[i].clearValue.color = m_clear_color;
    }

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT);
    pipe.cb_ci_.attachmentCount = 0;
    pipe.CreateGraphicsPipeline();

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32, 32}};
    rendering_info.layerCount = 1u;
    rendering_info.colorAttachmentCount = attachment_count;
    rendering_info.pColorAttachments = rendering_attachment_info.data();

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    for (uint32_t i = 0; i < attachment_count; ++i) {
        VkBool32 color_blend_enable = i == 0;
        vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), i, 1u, &color_blend_enable);
        VkColorBlendAdvancedEXT color_blend_advanced;
        color_blend_advanced.advancedBlendOp = VK_BLEND_OP_ADD;
        color_blend_advanced.srcPremultiplied = VK_FALSE;
        color_blend_advanced.dstPremultiplied = VK_FALSE;
        color_blend_advanced.blendOverlap = VK_BLEND_OVERLAP_UNCORRELATED_EXT;
        color_blend_advanced.clampResults = VK_FALSE;
        vk::CmdSetColorBlendAdvancedEXT(m_command_buffer.handle(), i, 1u, &color_blend_advanced);
    }

    vk::CmdDraw(m_command_buffer.handle(), 4u, 1u, 0u, 0u);

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DynamicSampleLocationsRasterizationSamplesMismatch) {
    TEST_DESCRIPTION("Dynamically set sample locations and rasterizationSamples that dont match");

    AddRequiredExtensions(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3RasterizationSamples);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3SampleLocationsEnable);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdSetSampleLocationsEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdSetRasterizationSamplesEXT(m_command_buffer.handle(), VK_SAMPLE_COUNT_1_BIT);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3u, 1u, 0u, 0u);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DepthClampControl) {
    AddRequiredExtensions(VK_EXT_DEPTH_CLAMP_CONTROL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::depthClampControl);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkDepthClampRangeEXT clamp_range = {0.0f, 1.0f};
    vk::CmdSetDepthClampRangeEXT(m_command_buffer.handle(), VK_DEPTH_CLAMP_MODE_USER_DEFINED_RANGE_EXT, &clamp_range);
    vk::CmdDraw(m_command_buffer.handle(), 3u, 1u, 0u, 0u);
    vk::CmdSetDepthClampRangeEXT(m_command_buffer.handle(), VK_DEPTH_CLAMP_MODE_VIEWPORT_RANGE_EXT, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3u, 1u, 0u, 0u);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, MultisampledRenderToSingleSampled) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6921");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3RasterizationSamples);
    AddRequiredFeature(vkt::Feature::multisampledRenderToSingleSampled);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;  // is ignored since dynamic

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    pipe.ms_ci_ = ms_state_ci;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdSetRasterizationSamplesEXT(m_command_buffer.handle(), VK_SAMPLE_COUNT_4_BIT);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DiscardRectanglesModeNotSetWithZeroCount) {
    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    if (!DeviceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_EXT_discard_rectangles version 2";
    }
    InitRenderTarget();

    VkRect2D discard_rectangle = {{0, 0}, {16, 16}};
    VkPipelineDiscardRectangleStateCreateInfoEXT discard_rect_ci = vku::InitStructHelper();
    discard_rect_ci.discardRectangleCount = 0;  // implicitly set discardRectangleEnable to FALSE
    discard_rect_ci.pDiscardRectangles = &discard_rectangle;

    CreatePipelineHelper pipe(*this, &discard_rect_ci);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DiscardRectanglesModeNotSetWithCommand) {
    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    if (!DeviceExtensionSupported(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, 2)) {
        GTEST_SKIP() << "need VK_EXT_discard_rectangles version 2";
    }
    InitRenderTarget();

    VkRect2D discard_rectangle = {{0, 0}, {16, 16}};
    VkPipelineDiscardRectangleStateCreateInfoEXT discard_rect_ci = vku::InitStructHelper();
    discard_rect_ci.discardRectangleCount = 1;
    discard_rect_ci.pDiscardRectangles = &discard_rectangle;

    CreatePipelineHelper pipe(*this, &discard_rect_ci);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetDiscardRectangleEnableEXT(m_command_buffer.handle(), false);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DiscardRectanglesDisabled) {
    TEST_DESCRIPTION("Draw without discard rectangles if discard rectangle enable is false");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetDiscardRectangleEnableEXT(m_command_buffer.handle(), VK_FALSE);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DrawNotSetRasterizationStream) {
    TEST_DESCRIPTION("Set VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT but without a geometry shader in the pipeline");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::geometryStreams);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3RasterizationStream);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // never called vkCmdSetRasterizationStreamEXT
    vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicState, DrawNotSetTessellationDomainOrigin) {
    TEST_DESCRIPTION("Set VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT but without a tese shader in the pipeline");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3TessellationDomainOrigin);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPipelineTessellationDomainOriginStateCreateInfo tess_domain_ci = vku::InitStructHelper();
    tess_domain_ci.domainOrigin = VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT;
    VkPipelineTessellationStateCreateInfo tess_ci = vku::InitStructHelper(&tess_domain_ci);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT);
    pipe.gp_ci_.pTessellationState = &tess_ci;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // vkCmdSetTessellationDomainOriginEXT not set
    vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}
