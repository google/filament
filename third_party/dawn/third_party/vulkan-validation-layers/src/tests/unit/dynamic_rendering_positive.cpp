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

void DynamicRenderingTest::InitBasicDynamicRendering() {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());
}

class PositiveDynamicRendering : public DynamicRenderingTest {};

TEST_F(PositiveDynamicRendering, BasicUsage) {
    TEST_DESCRIPTION("Most simple way to use dynamic rendering");
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {64, 64}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, Draw) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering.");
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, DrawMultiBind) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering and multiple CmdBindPipeline calls.");
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    const auto depth_format = FindSupportedDepthOnlyFormat(Gpu());

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = depth_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.CreateGraphicsPipeline();

    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    CreatePipelineHelper pipe2(*this, &pipeline_rendering_info);
    pipe2.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();

    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, BeginQuery) {
    TEST_DESCRIPTION("Test calling vkCmdBeginQuery with a dynamic render pass.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    InitRenderTarget();

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 2);

    m_command_buffer.Begin();

    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.EndRendering();

    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, PipeWithDiscard) {
    TEST_DESCRIPTION("Create dynamic rendering pipeline with rasterizer discard.");
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthWriteEnable = VK_TRUE;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;  // D16_UNORM has guaranteed support

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.ds_ci_ = ds_ci;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicRendering, UseStencilAttachmentWithIntegerFormatAndDepthStencilResolve) {
    TEST_DESCRIPTION("Use stencil attachment with integer format and depth stencil resolve extension");
    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering());
    InitRenderTarget();

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        GTEST_SKIP() << "VK_FORMAT_S8_UINT format not supported";
    }

    VkPhysicalDeviceDepthStencilResolveProperties depth_stencil_resolve_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(depth_stencil_resolve_properties);
    if ((depth_stencil_resolve_properties.supportedStencilResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) == 0) {
        GTEST_SKIP() << "VK_RESOLVE_MODE_AVERAGE_BIT not supported for VK_FORMAT_S8_UINT";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_S8_UINT;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_STENCIL_BIT);

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image resolve_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView resolve_image_view = resolve_image.CreateView(VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfo stencil_attachment = vku::InitStructHelper();
    stencil_attachment.imageView = image_view;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    stencil_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.pStencilAttachment = &stencil_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, FragmentDensityMapSubsampledBit) {
    TEST_DESCRIPTION("Test creating an image with subsampled bit.");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDynamicRendering());
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT not supported";
    }

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    image_ci.flags = 0;
    image_ci.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;

    vkt::Image fdm_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView fdm_image_view = fdm_image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.imageView = image_view;

    VkRenderingFragmentDensityMapAttachmentInfoEXT fragment_density_map = vku::InitStructHelper();
    fragment_density_map.imageView = fdm_image_view;
    fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper(&fragment_density_map);
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
    begin_rendering_info.layerCount = 1;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
}

TEST_F(PositiveDynamicRendering, SuspendResumeDraw) {
    TEST_DESCRIPTION("Resume and suspend at vkCmdBeginRendering time");
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.ds_ci_ = vku::InitStructHelper();
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    vkt::CommandBuffer cb2(*m_device, m_command_pool);

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    begin_rendering_info.flags = VK_RENDERING_RESUMING_BIT | VK_RENDERING_SUSPENDING_BIT;
    cb1.Begin();
    cb1.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(cb1.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(cb1.handle(), 3, 1, 0, 0);
    cb1.EndRendering();
    cb1.End();

    begin_rendering_info.flags = VK_RENDERING_RESUMING_BIT;
    cb2.Begin();
    cb2.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(cb2.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(cb2.handle(), 3, 1, 0, 0);
    cb2.EndRendering();
    cb2.End();

    std::array cbs = {&m_command_buffer, &cb1, &cb2};
    m_default_queue->Submit(cbs);
    m_default_queue->Wait();
}

TEST_F(PositiveDynamicRendering, CreateGraphicsPipeline) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_KHR_dynamic_rendering enabled");
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    char const* fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(x);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo rendering_info = vku::InitStructHelper();
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &color_format;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.CreateRenderPass();

    CreatePipelineHelper pipe(*this, &rendering_info);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicRendering, CreateGraphicsPipelineNoInfo) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_KHR_dynamic_rendering enabled but no rendering info struct.");
    RETURN_IF_SKIP(InitBasicDynamicRendering());
    char const* fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(x);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const vkt::DescriptorSetLayout dsl(*m_device, {dslb});
    const vkt::PipelineLayout pl(*m_device, {&dsl});

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddInputAttachment(0);
    rp.CreateRenderPass();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicRendering, CommandDrawWithShaderTileImageRead) {
    TEST_DESCRIPTION("vkCmdDraw* with shader tile image read extension using dynamic Rendering Tests.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::sampleRateShading);
    AddRequiredFeature(vkt::Feature::shaderTileImageDepthReadAccess);
    AddRequiredFeature(vkt::Feature::shaderTileImageStencilReadAccess);
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageDepthStencilReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineDepthStencilStateCreateInfo ds_state = vku::InitStructHelper();
    ds_state.depthWriteEnable = VK_TRUE;

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    VkPipelineMultisampleStateCreateInfo ms_ci = vku::InitStructHelper();
    ms_ci.sampleShadingEnable = VK_TRUE;
    ms_ci.minSampleShading = 1.0;
    ms_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs->GetStageCreateInfo()};
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pMultisampleState = &ms_ci;
    pipe.gp_ci_.pDepthStencilState = &ds_state;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
    pipe.CreateGraphicsPipeline();

    vkt::Image depth_image(*m_device, 32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    vkt::ImageView depth_image_view = depth_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    vkt::Image color_image(*m_device, 32, 32, 1, color_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView color_image_view = color_image.CreateView();

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = depth_image_view.handle();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = color_image_view.handle();

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &depth_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetDepthWriteEnable(m_command_buffer.handle(), false);
    vk::CmdSetStencilWriteMask(m_command_buffer.handle(), VK_STENCIL_FACE_FRONT_BIT, 0);
    vk::CmdSetStencilWriteMask(m_command_buffer.handle(), VK_STENCIL_FACE_BACK_BIT, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, DualSourceBlending) {
    TEST_DESCRIPTION("Test drawing with dynamic rendering and dual source blending.");
    AddRequiredFeature(vkt::Feature::dualSrcBlend);
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkPipelineColorBlendAttachmentState cb_attachments = {};
    cb_attachments.blendEnable = VK_TRUE;
    cb_attachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;  // bad!
    cb_attachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    cb_attachments.colorBlendOp = VK_BLEND_OP_ADD;
    cb_attachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cb_attachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cb_attachments.alphaBlendOp = VK_BLEND_OP_ADD;

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.cb_attachments_ = cb_attachments;
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, ExecuteCommandsWithNullImageView) {
    TEST_DESCRIPTION(
        "Test CmdExecuteCommands with an inherited image format of VK_FORMAT_UNDEFINED inside a render pass begun with "
        "CmdBeginRendering where the same image is specified as null");
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = VK_NULL_HANDLE;

    constexpr std::array bad_color_formats = {VK_FORMAT_UNDEFINED};

    VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info = vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = bad_color_formats.size();
    inheritance_rendering_info.pColorAttachmentFormats = bad_color_formats.data();
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = vku::InitStructHelper(&inheritance_rendering_info);

    VkCommandBufferBeginInfo cmdbuff_bi = vku::InitStructHelper();
    cmdbuff_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmdbuff_bi.pInheritanceInfo = &cmdbuff_ii;

    secondary.Begin(&cmdbuff_bi);
    secondary.End();

    m_command_buffer.Begin();

    m_command_buffer.BeginRendering(begin_rendering_info);

    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());

    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, SuspendPrimaryResumeInSecondary) {
    TEST_DESCRIPTION("Suspend in primary and resume in secondary");
    RETURN_IF_SKIP(InitBasicDynamicRendering());
    VkFormat color_formats = {VK_FORMAT_UNDEFINED};
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    // Primary suspends render
    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();

    // Secondary resumes render
    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    begin_rendering_info.flags = VK_RENDERING_RESUMING_BIT;
    secondary.Begin();
    secondary.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(secondary.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(secondary.handle(), 3, 1, 0, 0);
    secondary.EndRendering();
    secondary.End();

    // Execute secondary
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveDynamicRendering, SuspendSecondaryResumeInPrimary) {
    TEST_DESCRIPTION("Suspend in secondary and resume in primary");
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkFormat color_formats = {VK_FORMAT_UNDEFINED};
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = 0;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    // First primary with secondary that suspends render
    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    begin_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT;
    secondary.Begin();
    secondary.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(secondary.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(secondary.handle(), 3, 1, 0, 0);
    secondary.EndRendering();
    secondary.End();

    // Execute secondary
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());

    m_command_buffer.End();

    // Second Primary resumes render
    vkt::CommandBuffer cb(*m_device, m_command_pool);
    begin_rendering_info.flags = VK_RENDERING_RESUMING_BIT;
    cb.Begin();
    cb.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(cb.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(cb.handle(), 3, 1, 0, 0);
    cb.EndRendering();
    cb.End();

    std::array cbs = {&m_command_buffer, &cb};
    m_default_queue->Submit(cbs);
    m_default_queue->Wait();
}

TEST_F(PositiveDynamicRendering, WithShaderTileImageAndBarrier) {
    TEST_DESCRIPTION("Test setting memory barrier with shader tile image features are enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::shaderTileImageColorReadAccess);
    AddRequiredFeature(vkt::Feature::shaderTileImageDepthReadAccess);
    AddRequiredFeature(vkt::Feature::shaderTileImageStencilReadAccess);
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    InitRenderTarget();

    m_command_buffer.Begin();

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;

    VkMemoryBarrier2 memory_barrier_2 = vku::InitStructHelper();
    memory_barrier_2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier_2.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    memory_barrier_2.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &memory_barrier_2;
    dependency_info.bufferMemoryBarrierCount = 0;
    dependency_info.pBufferMemoryBarriers = VK_NULL_HANDLE;
    dependency_info.imageMemoryBarrierCount = 0;
    dependency_info.pImageMemoryBarriers = VK_NULL_HANDLE;

    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_command_buffer.EndRendering();

    VkMemoryBarrier memory_barrier = vku::InitStructHelper();
    memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &memory_barrier, 0,
                           nullptr, 0, nullptr);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, IgnoreUnusedColorAttachment) {
    TEST_DESCRIPTION("Case from https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6518");
    RETURN_IF_SKIP(InitBasicDynamicRendering());
    VkFormat color_formats[] = {VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED};
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 2;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.pColorBlendState = nullptr;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicRendering, MatchingAttachmentFormats) {
    TEST_DESCRIPTION(
        "Draw with Dynamic Rendering with attachment specified as VK_NULL_HANDLE in VkRenderingInfo, and with corresponding "
        "format in VkPipelineRenderingCreateInfo set to VK_FORMAT_UNDEFINED");
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();

    VkFormat color_formats[] = {VK_FORMAT_UNDEFINED};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    CreatePipelineHelper pipeline_color(*this, &pipeline_rendering_info);
    pipeline_color.CreateGraphicsPipeline();

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

    CreatePipelineHelper pipeline_depth(*this, &pipeline_rendering_info);
    pipeline_depth.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipeline_depth.cb_ci_.attachmentCount = 0;
    pipeline_depth.CreateGraphicsPipeline();

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    CreatePipelineHelper pipeline_stencil(*this, &pipeline_rendering_info);
    pipeline_stencil.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipeline_stencil.cb_ci_.attachmentCount = 0;
    pipeline_stencil.CreateGraphicsPipeline();

    vkt::Image colorImage(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    const VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image depthStencilImage(*m_device, 32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = VK_NULL_HANDLE;

    VkRenderingAttachmentInfo depth_stencil_attachment = vku::InitStructHelper();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = VK_NULL_HANDLE;

    m_command_buffer.Begin();

    {
        // Mismatching color formats
        VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_command_buffer.BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_color.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
        m_command_buffer.EndRendering();
    }

    {
        // Mismatching depth format
        VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_command_buffer.BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_depth.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
        m_command_buffer.EndRendering();
    }

    {
        // Mismatching stencil format
        VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_command_buffer.BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_stencil.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
        m_command_buffer.EndRendering();
    }

    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, MatchingAttachmentFormats2) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with dynamicRenderingUnusedAttachments enabled and matching formats");
    AddRequiredExtensions(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRenderingUnusedAttachments);
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    CreatePipelineHelper pipeline_color(*this, &pipeline_rendering_info);
    pipeline_color.CreateGraphicsPipeline();

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(Gpu());

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = depthStencilFormat;

    CreatePipelineHelper pipeline_depth(*this, &pipeline_rendering_info);
    pipeline_depth.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipeline_depth.cb_ci_.attachmentCount = 0;
    pipeline_depth.CreateGraphicsPipeline();

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = depthStencilFormat;

    CreatePipelineHelper pipeline_stencil(*this, &pipeline_rendering_info);
    pipeline_stencil.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipeline_stencil.cb_ci_.attachmentCount = 0;
    pipeline_stencil.CreateGraphicsPipeline();

    vkt::Image colorImage(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView colorImageView = colorImage.CreateView();

    vkt::Image depthStencilImage(*m_device, 32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = VK_NULL_HANDLE;

    VkRenderingAttachmentInfo depth_stencil_attachment = vku::InitStructHelper();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = VK_NULL_HANDLE;

    m_command_buffer.Begin();

    {
        // Null color image view
        VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_command_buffer.BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_color.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
        m_command_buffer.EndRendering();

        // Matching color formats
        color_attachment.imageView = colorImageView;
        m_command_buffer.BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_color.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
        m_command_buffer.EndRendering();
        color_attachment.imageView = VK_NULL_HANDLE;
    }

    {
        // Null depth image view
        VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_command_buffer.BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_depth.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
        m_command_buffer.EndRendering();

        // Matching depth format
        const vkt::ImageView depth_view(*m_device, depthStencilImage.BasicViewCreatInfo(VK_IMAGE_ASPECT_DEPTH_BIT));
        depth_stencil_attachment.imageView = depth_view.handle();
        m_command_buffer.BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_depth.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
        m_command_buffer.EndRendering();
        depth_stencil_attachment.imageView = VK_NULL_HANDLE;
    }

    {
        // Null stencil image view
        VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
        begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
        m_command_buffer.BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_stencil.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
        m_command_buffer.EndRendering();

        // Matching stencil format
        const vkt::ImageView stencil_view(*m_device, depthStencilImage.BasicViewCreatInfo(VK_IMAGE_ASPECT_STENCIL_BIT));
        depth_stencil_attachment.imageView = stencil_view.handle();
        m_command_buffer.BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_stencil.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
        m_command_buffer.EndRendering();
        depth_stencil_attachment.imageView = VK_NULL_HANDLE;
    }

    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, ExecuteCommandsFlags) {
    TEST_DESCRIPTION("Test CmdExecuteCommands inside a render pass begun with CmdBeginRendering that has same flags");
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    constexpr VkFormat color_formats = {VK_FORMAT_UNDEFINED};  // undefined because no image view will be used

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info = vku::InitStructHelper();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    inheritance_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cb_inheritance_info = vku::InitStructHelper(&inheritance_rendering_info);
    cb_inheritance_info.renderPass = VK_NULL_HANDLE;
    cb_inheritance_info.subpass = 0;
    cb_inheritance_info.framebuffer = VK_NULL_HANDLE;

    VkCommandBufferBeginInfo cb_begin_info = vku::InitStructHelper();
    cb_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cb_begin_info.pInheritanceInfo = &cb_inheritance_info;
    secondary.Begin(&cb_begin_info);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, ColorAttachmentMismatch) {
    TEST_DESCRIPTION("colorAttachmentCount and attachmentCount don't match but it is dynamically ignored");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2LogicOp);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3LogicOpEnable);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEnable);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEquation);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorWriteMask);
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 0;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicRendering, PipelineUnusedAttachments) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching color attachment counts and depth/stencil formats");
    AddRequiredExtensions(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRenderingUnusedAttachments);
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM};
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 2;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments(2);

    VkPipelineColorBlendStateCreateInfo cbi = vku::InitStructHelper();
    cbi.attachmentCount = 2u;
    cbi.pAttachments = color_blend_attachments.data();

    CreatePipelineHelper pipeline(*this, &pipeline_rendering_info);
    pipeline.gp_ci_.pColorBlendState = &cbi;
    pipeline.CreateGraphicsPipeline();

    vkt::Image colorImage(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView colorImageView = colorImage.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};
    begin_rendering_info.colorAttachmentCount = 1u;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 1, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, DynamicColorBlendEnable) {
    TEST_DESCRIPTION("Do a draw with VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEnable);
    RETURN_IF_SKIP(Init());

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkBool32 color_blend_enable = VK_TRUE;
    vk::CmdSetColorBlendEnableEXT(m_command_buffer.handle(), 0, 1, &color_blend_enable);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, UnusedAttachmentsMismatchedFormats) {
    TEST_DESCRIPTION(
        "Draw with dynamic rendering unused attachments with mismatching color attachment counts and depth/stencil formats");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::dynamicRenderingUnusedAttachments);
    RETURN_IF_SKIP(Init());

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;

    CreatePipelineHelper pipeline_depth(*this, &pipeline_rendering_info);
    pipeline_depth.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipeline_depth.cb_ci_.attachmentCount = 0;
    pipeline_depth.CreateGraphicsPipeline();

    VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info = vku::InitStructHelper();
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    const VkCommandBufferInheritanceInfo cmdbuff_ii = vku::InitStructHelper(&inheritance_rendering_info);

    VkCommandBufferBeginInfo cmdbuff_bi = vku::InitStructHelper();
    cmdbuff_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmdbuff_bi.pInheritanceInfo = &cmdbuff_ii;

    secondary.Begin(&cmdbuff_bi);
    vk::CmdBindPipeline(secondary.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_depth.Handle());
    vk::CmdDraw(secondary.handle(), 1, 1, 0, 0);
    secondary.End();
}

TEST_F(PositiveDynamicRendering, BeginRenderingWithRenderPassStriped) {
    TEST_DESCRIPTION("Test to validate begin rendering with VK_ARM_render_pass_striped. ");
    AddRequiredExtensions(VK_ARM_RENDER_PASS_STRIPED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::renderPassStriped);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkPhysicalDeviceRenderPassStripedPropertiesARM rp_striped_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(rp_striped_props);
    const uint32_t stripe_width = rp_striped_props.renderPassStripeGranularity.width * 2;
    const uint32_t stripe_height = rp_striped_props.renderPassStripeGranularity.height;

    const uint32_t stripe_count = 8;
    const uint32_t width = stripe_width * stripe_count;
    const uint32_t height = stripe_height;
    std::vector<VkRenderPassStripeInfoARM> stripe_infos(stripe_count);
    for (uint32_t i = 0; i < stripe_count; ++i) {
        stripe_infos[i] = vku::InitStructHelper();
        stripe_infos[i].stripeArea.offset.x = stripe_width * i;
        stripe_infos[i].stripeArea.offset.y = 0;
        stripe_infos[i].stripeArea.extent.width = stripe_width;
        stripe_infos[i].stripeArea.extent.height = stripe_height;
    }

    VkRenderPassStripeBeginInfoARM rp_striped_info = vku::InitStructHelper();
    rp_striped_info.stripeInfoCount = stripe_count;
    rp_striped_info.pStripeInfos = stripe_infos.data();

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper(&rp_striped_info);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {width, height}};

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buffer(*m_device, command_pool);

    VkCommandBufferBeginInfo cmd_begin = vku::InitStructHelper();
    cmd_buffer.Begin(&cmd_begin);
    cmd_buffer.BeginRendering(begin_rendering_info);
    cmd_buffer.EndRendering();
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

TEST_F(PositiveDynamicRendering, LegacyDithering) {
    AddRequiredExtensions(VK_EXT_LEGACY_DITHERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::legacyDithering);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    VkPipelineCreateFlags2CreateInfo create_flags_2 = vku::InitStructHelper();
    create_flags_2.flags = VK_PIPELINE_CREATE_2_ENABLE_LEGACY_DITHERING_BIT_EXT;

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper(&create_flags_2);
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_ENABLE_LEGACY_DITHERING_BIT_EXT;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveDynamicRendering, AttachmentCountDynamicState) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7881");
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEnable);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEquation);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorWriteMask);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendAdvanced);
    RETURN_IF_SKIP(InitBasicDynamicRendering());
    InitRenderTarget();

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = vku::InitStructHelper();
    color_blend_state_create_info.attachmentCount = 0;
    color_blend_state_create_info.pAttachments = nullptr;

    VkFormat color_format = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.cb_ci_ = color_blend_state_create_info;
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveDynamicRendering, VertexOnlyDepth) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8015");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::sampleRateShading);
    AddRequiredFeature(vkt::Feature::extendedDynamicState2);
    RETURN_IF_SKIP(InitBasicDynamicRendering());

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(Gpu());
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = depth_format;

    VkPipelineMultisampleStateCreateInfo ms_ci = vku::InitStructHelper();
    ms_ci.sampleShadingEnable = VK_TRUE;
    ms_ci.minSampleShading = 1.0;
    ms_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds_state = vku::InitStructHelper();
    ds_state.depthWriteEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.VertexShaderOnly();
    pipe.AddDynamicState(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.gp_ci_.pMultisampleState = &ms_ci;
    pipe.gp_ci_.pDepthStencilState = &ds_state;
    pipe.CreateGraphicsPipeline();
}
