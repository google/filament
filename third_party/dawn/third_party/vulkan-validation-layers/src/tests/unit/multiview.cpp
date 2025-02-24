/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"
#include "utils/convert_utils.h"

class NegativeMultiview : public VkLayerTest {};

TEST_F(NegativeMultiview, MaxInstanceIndex) {
    TEST_DESCRIPTION("Verify if instance index in CmdDraw is greater than maxMultiviewInstanceIndex.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPhysicalDeviceMultiviewProperties multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multiview_props);
    if (multiview_props.maxMultiviewInstanceIndex == std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "maxMultiviewInstanceIndex is uint32_t max";
    }

    uint32_t viewMask = 0x1u;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = vku::InitStructHelper();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass(&renderPassMultiviewCreateInfo);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = 0u;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {32u, 32u, 1u};
    image_create_info.mipLevels = 1u;
    image_create_info.arrayLayers = 2u;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView imageView = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &imageView.handle());

    VkRenderPassBeginInfo render_pass_bi = vku::InitStructHelper();
    render_pass_bi.renderPass = rp.Handle();
    render_pass_bi.framebuffer = framebuffer.handle();
    render_pass_bi.renderArea.extent.width = 32u;
    render_pass_bi.renderArea.extent.height = 32u;
    render_pass_bi.clearValueCount = 1u;
    render_pass_bi.pClearValues = m_renderPassClearValues.data();

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass_bi);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688");
    vk::CmdDraw(m_command_buffer.handle(), 1, multiview_props.maxMultiviewInstanceIndex + 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeMultiview, ClearColorAttachments) {
    TEST_DESCRIPTION("Test cmdClearAttachments with active render pass that uses multiview");

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    uint32_t viewMask = 0x1u;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = vku::InitStructHelper();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass(&renderPassMultiviewCreateInfo);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView imageView = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &imageView.handle());

    // Start no RenderPass
    m_command_buffer.Begin();

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 0;
    color_attachment.clearValue.color.float32[1] = 0;
    color_attachment.clearValue.color.float32[2] = 0;
    color_attachment.clearValue.color.float32[3] = 0;
    color_attachment.colorAttachment = 0;

    VkClearRect clear_rect = {};
    clear_rect.rect.extent.width = 32;
    clear_rect.rect.extent.height = 32;

    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), 32, 32);
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-baseArrayLayer-00018");
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-pRects-06937");
    clear_rect.layerCount = 2;
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-baseArrayLayer-00018");
    m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-pRects-06937");
    clear_rect.baseArrayLayer = 1;
    clear_rect.layerCount = 1;
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMultiview, UnboundResourcesAfterBeginRenderPassAndNextSubpass) {
    TEST_DESCRIPTION(
        "Validate all required resources are bound if multiview is enabled after vkCmdBeginRenderPass and vkCmdNextSubpass");

    constexpr unsigned multiview_count = 2u;
    constexpr unsigned extra_subpass_count = multiview_count - 1u;

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);

    RETURN_IF_SKIP(Init());

    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachmentReference.attachment = 0;

    std::vector<VkSubpassDescription> subpasses;
    subpasses.resize(multiview_count);
    for (unsigned i = 0; i < multiview_count; ++i) {
        subpasses[i].colorAttachmentCount = 1;
        subpasses[i].pColorAttachments = &colorAttachmentReference;
    }

    uint32_t viewMasks[multiview_count] = {};
    for (unsigned i = 0; i < multiview_count; ++i) {
        viewMasks[i] = 1u << i;
    }
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = vku::InitStructHelper();
    renderPassMultiviewCreateInfo.subpassCount = multiview_count;
    renderPassMultiviewCreateInfo.pViewMasks = viewMasks;

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper(&renderPassMultiviewCreateInfo);
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachmentDescription;
    rp_info.subpassCount = subpasses.size();
    rp_info.pSubpasses = subpasses.data();

    std::vector<VkSubpassDependency> dependencies;
    dependencies.resize(extra_subpass_count);
    for (unsigned i = 0; i < dependencies.size(); ++i) {
        auto &subpass_dep = dependencies[i];
        subpass_dep.srcSubpass = i;
        subpass_dep.dstSubpass = i + 1;

        subpass_dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        subpass_dep.srcAccessMask = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpass_dep.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        subpass_dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    rp_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    rp_info.pDependencies = dependencies.data();

    vk::CreateRenderPass(m_device->handle(), &rp_info, nullptr, &m_renderPass);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = m_width;
    image_create_info.extent.height = m_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = multiview_count;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    vkt::Image image(*m_device, image_create_info);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView imageView = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    vkt::Framebuffer fb(*m_device, m_renderPass, 1, &imageView.handle(), m_width, m_height);

    VkClearValue clear{};
    clear.color = m_clear_color;
    m_renderPassClearValues.emplace_back(clear);
    m_renderPassBeginInfo.renderPass = m_renderPass;
    m_renderPassBeginInfo.framebuffer = fb.handle();
    m_renderPassBeginInfo.renderArea.extent.width = m_width;
    m_renderPassBeginInfo.renderArea.extent.height = m_height;
    m_renderPassBeginInfo.clearValueCount = m_renderPassClearValues.size();
    m_renderPassBeginInfo.pClearValues = m_renderPassClearValues.data();

    // Pipeline not bound test
    {
        // No need to create individual pipelines for each subpass since we are checking no bound pipeline
        CreatePipelineHelper pipe(*this);
        pipe.CreateGraphicsPipeline();

        m_command_buffer.Begin();
        // This bind should not be valid after we begin the renderpass
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08606");
        vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This bind should not be valid for next subpass
            vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
            m_command_buffer.NextSubpass();

            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08606");
            vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    m_command_buffer.Reset();

    // Dynamic state (checking with line width)
    {
        // Pipeline for subpass 0
        CreatePipelineHelper pipe(*this);
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        pipe.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vkt::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }

        m_command_buffer.Begin();
        // This line width set should not be valid for next subpass
        vk::CmdSetLineWidth(m_command_buffer.handle(), 1.0f);
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07833");
        vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This line width set should not be valid for next subpass
            vk::CmdSetLineWidth(m_command_buffer.handle(), 1.0f);
            m_command_buffer.NextSubpass();
            vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());

            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07833");
            vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    m_command_buffer.Reset();

    // Push constants
    {
        char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo {
           mat3 m;
        } constants;
        void main(){
            vec3 v3 = constants.m[0];
        }
    )glsl";

        VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj const fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 36};
        VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant_range;

        vkt::PipelineLayout layout(*m_device, pipeline_layout_info, std::vector<const vkt::DescriptorSetLayout *>{});

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.pipeline_layout_ci_ = pipeline_layout_info;
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vkt::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete

        const float dummy_values[16] = {};
        m_command_buffer.Begin();
        // This push constants should not be counted when render pass begins
        vk::CmdPushConstants(m_command_buffer.handle(), layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, push_constant_range.offset,
                             push_constant_range.size, dummy_values);
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maintenance4-08602");
        vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This push constants should not be counted when we change subpass
            vk::CmdPushConstants(m_command_buffer.handle(), layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, push_constant_range.offset,
                                 push_constant_range.size, dummy_values);
            m_command_buffer.NextSubpass();
            vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());

            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maintenance4-08602");
            vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    m_command_buffer.Reset();

    // Descriptor sets
    {
        vkt::Buffer buffer(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        OneOffDescriptorSet descriptor_set{m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}}};
        descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE);
        descriptor_set.UpdateDescriptorSets();

        VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = &descriptor_set.layout_.handle();

        vkt::PipelineLayout layout(*m_device, pipeline_layout_info, std::vector<vkt::DescriptorSetLayout const *>{});

        VkShaderObj const vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj const fs(this, kFragmentUniformGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.pipeline_layout_ci_ = pipeline_layout_info;
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vkt::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete

        m_command_buffer.Begin();
        // This descriptor bind should not be counted when render pass begins
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08600");
        vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This descriptor bind should not be counted when next subpass begins
            vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0,
                                      1, &descriptor_set.set_, 0, nullptr);
            m_command_buffer.NextSubpass();
            vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());

            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08600");
            vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    m_command_buffer.Reset();

    // Vertex buffer
    {
        float const vertex_data[] = {1.0f, 0.0f};
        vkt::Buffer vbo(*m_device, sizeof(vertex_data), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        VkVertexInputBindingDescription input_binding{};
        input_binding.binding = 0;
        input_binding.stride = sizeof(vertex_data);
        input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription input_attribs{};
        input_attribs.binding = 0;
        input_attribs.location = 0;
        input_attribs.format = VK_FORMAT_R32G32_SFLOAT;
        input_attribs.offset = 0;

        char const *const vsSource = R"glsl(
        #version 450
        layout(location = 0) in vec2 input0;
        void main(){
           gl_Position = vec4(input0.x, input0.y, 0.0f, 1.0f);
        }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.vi_ci_.vertexBindingDescriptionCount = 1;
        pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
        pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
        pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vkt::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete
        VkDeviceSize offset = 0;

        m_command_buffer.Begin();
        // This vertex buffer bind should not be counted when render pass begins
        vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-04007");
        vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This vertex buffer bind should not be counted when next subpass begins
            vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);
            m_command_buffer.NextSubpass();
            vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());

            m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-04007");
            vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    m_command_buffer.Reset();

    // Index buffer
    {
        float const vertex_data[] = {1.0f, 0.0f};
        vkt::Buffer vbo(*m_device, sizeof(vertex_data), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        uint32_t const index_data[] = {0};
        vkt::Buffer ibo(*m_device, sizeof(index_data), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        VkVertexInputBindingDescription input_binding{};
        input_binding.binding = 0;
        input_binding.stride = sizeof(vertex_data);
        input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription input_attribs{};
        input_attribs.binding = 0;
        input_attribs.location = 0;
        input_attribs.format = VK_FORMAT_R32G32_SFLOAT;
        input_attribs.offset = 0;

        char const *const vsSource = R"glsl(
        #version 450
        layout(location = 0) in vec2 input0;
        void main(){
           gl_Position = vec4(input0.x, input0.y, 0.0f, 1.0f);
        }
    )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.vi_ci_.vertexBindingDescriptionCount = 1;
        pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
        pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
        pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
        pipe.CreateGraphicsPipeline();

        // Pipelines for all other subpasses
        vkt::Pipeline pipelines[extra_subpass_count];
        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            auto pipe_info = pipe.gp_ci_;
            pipe_info.subpass = i + 1;
            pipelines[i].init(*m_device, pipe_info);
        }
        // Set up complete

        VkDeviceSize offset = 0;
        m_command_buffer.Begin();
        // This index buffer bind should not be counted when render pass begins
        vk::CmdBindIndexBuffer(m_command_buffer.handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT32);
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);

        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-None-07312");
        vk::CmdDrawIndexed(m_command_buffer.handle(), 0, 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        for (unsigned i = 0; i < extra_subpass_count; ++i) {
            // This index buffer bind should not be counted when next subpass begins
            vk::CmdBindIndexBuffer(m_command_buffer.handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT32);
            m_command_buffer.NextSubpass();
            vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[i].handle());
            vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &vbo.handle(), &offset);

            m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-None-07312");
            vk::CmdDrawIndexed(m_command_buffer.handle(), 0, 1, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
}

TEST_F(NegativeMultiview, BeginTransformFeedback) {
    TEST_DESCRIPTION("Test beginning transform feedback in a render pass with multiview enabled");

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    uint32_t viewMask = 0x1u;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = vku::InitStructHelper();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass(&renderPassMultiviewCreateInfo);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    auto image_view_ci = image.BasicViewCreatInfo();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    const vkt::ImageView imageView(*m_device, image_view_ci);

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 1, &imageView.handle());

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), 32, 32);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT);
    VkDeviceSize offset = 0;
    vk::CmdBindTransformFeedbackBuffersEXT(m_command_buffer.handle(), 0, 1, &buffer.handle(), &offset, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-None-04128");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedbackEXT-None-02373");
    vk::CmdBeginTransformFeedbackEXT(m_command_buffer.handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeMultiview, Features) {
    TEST_DESCRIPTION("Checks VK_KHR_multiview features.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    std::vector<const char *> device_extensions;
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        device_extensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    }

    VkPhysicalDeviceMultiviewFeatures multiview_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);

    // Set false to trigger VUs
    multiview_features.multiview = VK_FALSE;

    vkt::PhysicalDevice physical_device(Gpu());
    vkt::QueueCreateInfoArray queue_info(physical_device.queue_properties_);
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.Data();
    for (uint32_t i = 0; i < queue_info.Size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&features2);
    device_create_info.queueCreateInfoCount = queue_info.Size();
    device_create_info.pQueueCreateInfos = queue_info.Data();
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.enabledExtensionCount = device_extensions.size();
    VkDevice testDevice;

    if ((multiview_features.multiviewGeometryShader == VK_FALSE) && (multiview_features.multiviewTessellationShader == VK_FALSE)) {
        GTEST_SKIP() << "multiviewGeometryShader and multiviewTessellationShader feature not supported";
    }

    if (multiview_features.multiviewGeometryShader == VK_TRUE) {
        m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceMultiviewFeatures-multiviewGeometryShader-00580");
    }
    if (multiview_features.multiviewTessellationShader == VK_TRUE) {
        m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceMultiviewFeatures-multiviewTessellationShader-00581");
    }
    vk::CreateDevice(Gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMultiview, RenderPassCreateOverlappingCorrelationMasks) {
    TEST_DESCRIPTION("Create a subpass with overlapping correlation masks");

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    const VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    std::array viewMasks = {0x3u};
    std::array correlationMasks = {0x1u, 0x2u};
    auto rpmvci = vku::InitStruct<VkRenderPassMultiviewCreateInfo>(
        nullptr, 1u, viewMasks.data(), 0u, nullptr, static_cast<uint32_t>(correlationMasks.size()), correlationMasks.data());
    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpmvci, 0u, 0u, nullptr, 1u, &subpass, 0u, nullptr);

    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rpci, false);

    // Correlation masks must not overlap
    correlationMasks[1] = 0x3u;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported,
                         "VUID-VkRenderPassMultiviewCreateInfo-pCorrelationMasks-00841",
                         "VUID-VkRenderPassCreateInfo2-pCorrelatedViewMasks-03056");

    // Check for more specific "don't set any correlation masks when multiview is not enabled"
    if (rp2Supported) {
        viewMasks[0] = 0;
        correlationMasks[0] = 0;
        correlationMasks[1] = 0;
        auto safe_rpci2 = ConvertVkRenderPassCreateInfoToV2KHR(rpci);

        TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, *safe_rpci2.ptr(), {"VUID-VkRenderPassCreateInfo2-viewMask-03057"});
    }
}

TEST_F(NegativeMultiview, RenderPassViewMasksNotEnough) {
    TEST_DESCRIPTION("Create a subpass with the wrong number of view masks, or inconsistent setting of view masks");

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
    };
    uint32_t viewMasks[] = {0x3u, 0u};
    auto rpmvci = vku::InitStruct<VkRenderPassMultiviewCreateInfo>(nullptr, 1u, viewMasks, 0u, nullptr, 0u, nullptr);
    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpmvci, 0u, 0u, nullptr, 2u, subpasses, 0u, nullptr);

    // Not enough view masks
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pNext-01928",
                         "VUID-VkRenderPassCreateInfo2-viewMask-03058");
}

TEST_F(NegativeMultiview, RenderPassCreateSubpassMissingAttributesBitNVX) {
    TEST_DESCRIPTION("Create a subpass with the VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX flag missing");

    AddRequiredExtensions(VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    VkSubpassDescription subpasses[] = {
        {VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr,
         nullptr, 0, nullptr},
    };

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 0u, nullptr, 1u, subpasses, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-flags-00856",
                         "VUID-VkSubpassDescription2-flags-03076");
}

TEST_F(NegativeMultiview, DrawWithPipelineIncompatibleWithRenderPass) {
    TEST_DESCRIPTION(
        "Hit RenderPass incompatible cases: drawing with an active renderpass that's not compatible with the bound pipeline state "
        "object's creation renderpass since only the former uses Multiview.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiViewport);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Set up VkRenderPassCreateInfo struct used with VK_VERSION_1_0
    VkAttachmentReference color_att = {};
    color_att.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription attach = {};
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_att;

    uint32_t viewMasks[] = {0x3u};
    VkRenderPassMultiviewCreateInfo rpmvci = vku::InitStructHelper();
    rpmvci.subpassCount = 1;
    rpmvci.pViewMasks = viewMasks;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper(&rpmvci);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    // Set up VkRenderPassCreateInfo2 struct used with VK_VERSION_1_2
    VkAttachmentReference2 color_att2 = vku::InitStructHelper();
    color_att2.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription2 attach2 = vku::InitStructHelper();
    attach2.samples = VK_SAMPLE_COUNT_1_BIT;
    attach2.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach2.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach2.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass2 = vku::InitStructHelper();
    subpass2.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass2.viewMask = 0x3u;
    subpass2.colorAttachmentCount = 1;
    subpass2.pColorAttachments = &color_att2;

    VkRenderPassCreateInfo2 rpci2 = vku::InitStructHelper();
    rpci2.attachmentCount = 1;
    rpci2.pAttachments = &attach2;
    rpci2.subpassCount = 1;
    rpci2.pSubpasses = &subpass2;

    // Create render passes with VK_VERSION_1_0 struct and vkCreateRenderPass call
    // Create rp[0] with Multiview pNext, rp[1] without Multiview pNext, rp[2] with Multiview pNext but another viewMask
    std::array<vkt::RenderPass, 3> rp;
    rp[0].init(*m_device, rpci);
    rpci.pNext = nullptr;
    rp[1].init(*m_device, rpci);
    uint32_t viewMasks2[] = {0x1u};
    rpmvci.pViewMasks = viewMasks2;
    rpci.pNext = &rpmvci;
    rp[2].init(*m_device, rpci);

    // Create render passes with VK_VERSION_1_2 struct and vkCreateRenderPass2KHR call
    // Create rp2[0] with Multiview, rp2[1] without Multiview (zero viewMask), rp2[2] with Multiview but another viewMask
    std::array<vkt::RenderPass, 3> rp2;
    if (rp2Supported) {
        rp2[0].init(*m_device, rpci2);
        subpass2.viewMask = 0x0u;
        rpci2.pSubpasses = &subpass2;
        rp2[1].init(*m_device, rpci2);
        subpass2.viewMask = 0x1u;
        rpci2.pSubpasses = &subpass2;
        rp2[2].init(*m_device, rpci2);
    }

    auto ici2d = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, ici2d);
    vkt::ImageView iv = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 2);

    // Create framebuffers for rp[0] and rp2[0]
    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.renderPass = rp[0].handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &iv.handle();
    fbci.width = 128;
    fbci.height = 128;
    fbci.layers = 1;

    vkt::Framebuffer fb(*m_device, fbci);
    vkt::Framebuffer fb2;
    if (rp2Supported) {
        fbci.renderPass = rp2[0].handle();
        fb2.init(*m_device, fbci);
    }

    VkRenderPassBeginInfo rp_begin = vku::InitStructHelper();
    rp_begin.renderPass = rp[0].handle();
    rp_begin.framebuffer = fb.handle();
    rp_begin.renderArea = {{0, 0}, {128, 128}};

    // Create a graphics pipeline with rp[1]
    CreatePipelineHelper pipe_1(*this);
    pipe_1.gp_ci_.layout = pipeline_layout.handle();
    pipe_1.gp_ci_.renderPass = rp[1].handle();
    pipe_1.CreateGraphicsPipeline();

    // Create a graphics pipeline with rp[2]
    CreatePipelineHelper pipe_2(*this);
    pipe_2.gp_ci_.layout = pipeline_layout.handle();
    pipe_2.gp_ci_.renderPass = rp[2].handle();
    pipe_2.CreateGraphicsPipeline();

    CreatePipelineHelper pipe2_1(*this);
    CreatePipelineHelper pipe2_2(*this);
    if (rp2Supported) {
        pipe2_1.gp_ci_.layout = pipeline_layout.handle();
        pipe2_1.gp_ci_.renderPass = rp[1].handle();
        pipe2_1.CreateGraphicsPipeline();

        pipe2_2.gp_ci_.layout = pipeline_layout.handle();
        pipe2_2.gp_ci_.renderPass = rp[2].handle();
        pipe2_2.CreateGraphicsPipeline();
    }

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = rp[0].handle();
    cbii.subpass = 0;
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;

    // Begin rp[0] for VK_VERSION_1_0 test cases
    vk::BeginCommandBuffer(m_command_buffer.handle(), &cbbi);
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    // Bind rp[1]'s pipeline to command buffer
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_1.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (error on Multiview usage should trigger on draw)
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // Bind rp[2]'s pipeline to command buffer
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (error on non-matching viewMasks for Multiview usage should trigger on draw)
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // End rp[0]
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // Begin rp2[0] for VK_VERSION_1_2 test cases
    if (rp2Supported) {
        cbii.renderPass = rp2[0].handle();
        rp_begin.renderPass = rp2[0].handle();
        rp_begin.framebuffer = fb2.handle();
        vk::BeginCommandBuffer(m_command_buffer.handle(), &cbbi);
        vk::CmdBeginRenderPass(m_command_buffer.handle(), &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

        // Bind rp2[1]'s pipeline to command buffer
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2_1.Handle());

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-renderPass-02684");
        // Render triangle (error on Multiview usage should trigger on draw)
        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
        m_errorMonitor->VerifyFound();

        // Bind rp2[2]'s pipeline to command buffer
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2_2.Handle());

        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-renderPass-02684");
        // Render triangle (error on non-matching viewMasks for Multiview usage should trigger on draw)
        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
        m_errorMonitor->VerifyFound();

        // End rp2[0]
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }
}

TEST_F(NegativeMultiview, RenderPassViewMasksZero) {
    TEST_DESCRIPTION("Create a render pass with some view masks 0 and some not 0");

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMultiviewProperties render_pass_multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(render_pass_multiview_props);
    if (render_pass_multiview_props.maxMultiviewViewCount < 2) {
        GTEST_SKIP() << "maxMultiviewViewCount lower than required";
    }

    VkSubpassDescription subpasses[2];
    subpasses[0] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    subpasses[1] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMasks[] = {0x3u, 0x0};
    uint32_t correlationMasks[] = {0x1u, 0x3u};
    auto rpmvci = vku::InitStruct<VkRenderPassMultiviewCreateInfo>(nullptr, 2u, viewMasks, 0u, nullptr, 2u, correlationMasks);

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpmvci, 0u, 0u, nullptr, 2u, subpasses, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-pNext-02513", nullptr);
}

TEST_F(NegativeMultiview, RenderPassViewOffsets) {
    TEST_DESCRIPTION("Create a render pass with invalid multiview pViewOffsets");

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMultiviewProperties render_pass_multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(render_pass_multiview_props);
    if (render_pass_multiview_props.maxMultiviewViewCount < 2) {
        GTEST_SKIP() << "maxMultiviewViewCount lower than required";
    }

    VkSubpassDescription subpasses[2];
    subpasses[0] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    subpasses[1] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMasks[] = {0x1u, 0x2u};
    uint32_t correlationMasks[] = {0x1u, 0x2u};
    int32_t view_offset = 1;
    auto rpmvci = vku::InitStruct<VkRenderPassMultiviewCreateInfo>(nullptr, 2u, viewMasks, 1u, &view_offset, 2u, correlationMasks);

    VkSubpassDependency dependency = {};
    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpmvci, 0u, 0u, nullptr, 2u, subpasses, 1u, &dependency);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-pNext-02512", nullptr);
}

TEST_F(NegativeMultiview, RenderPassViewMasksLimit) {
    TEST_DESCRIPTION("Create a render pass with invalid view mask");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMultiviewProperties render_pass_multiview_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(render_pass_multiview_props);

    if (render_pass_multiview_props.maxMultiviewViewCount >= 32) {
        GTEST_SKIP() << "maxMultiviewViewCount too high";
    }

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMask = 1 << render_pass_multiview_props.maxMultiviewViewCount;
    uint32_t correlationMask = 0x1u;
    auto rpmvci = vku::InitStruct<VkRenderPassMultiviewCreateInfo>(nullptr, 1u, &viewMask, 0u, nullptr, 1u, &correlationMask);

    auto rpci = vku::InitStruct<VkRenderPassCreateInfo>(&rpmvci, 0u, 0u, nullptr, 1u, &subpass, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassMultiviewCreateInfo-pViewMasks-06697", nullptr);
}

TEST_F(NegativeMultiview, FeaturesDisabled) {
    TEST_DESCRIPTION("Create graphics pipeline using multiview features which are not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::multiview);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    AddRequiredFeature(vkt::Feature::geometryShader);

    RETURN_IF_SKIP(Init());

    RenderPass2SingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_B8G8R8A8_UNORM);
    rp.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.AddColorAttachment(0);
    rp.SetViewMask(0x3u);
    rp.CreateRenderPass();

    // tessellationShader
    {
        char const *tcsSource = R"glsl(
        #version 450
        layout(vertices=3) out;
        void main(){
           gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
           gl_TessLevelInner[0] = 1;
        }
        )glsl";
        char const *tesSource = R"glsl(
        #version 450
        layout(triangles, equal_spacing, cw) in;
        void main(){
           gl_Position.xyz = gl_TessCoord;
           gl_Position.w = 1.0f;
        }
        )glsl";

        VkShaderObj tcs(this, tcsSource, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

        VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                     VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

        VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

        CreatePipelineHelper pipe(*this);
        pipe.gp_ci_.renderPass = rp.Handle();
        pipe.gp_ci_.subpass = 0;
        pipe.cb_ci_.attachmentCount = 1;
        pipe.gp_ci_.pTessellationState = &tsci;
        pipe.gp_ci_.pInputAssemblyState = &iasci;
        pipe.shader_stages_.emplace_back(tcs.GetStageCreateInfo());
        pipe.shader_stages_.emplace_back(tes.GetStageCreateInfo());

        m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06047");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
    // geometryShader
    {
        static char const *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (triangle_strip) out;
        layout (max_vertices = 3) out;
        void main() {
           gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
           EmitVertex();
        }
        )glsl";

        VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.gp_ci_.renderPass = rp.Handle();
        pipe.gp_ci_.subpass = 0;
        pipe.cb_ci_.attachmentCount = 1;
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06048");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeMultiview, DynamicRenderingMaxMultiviewInstanceIndex) {
    TEST_DESCRIPTION("Draw with multiview enabled and instance index too high.");

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMultiviewProperties multiview_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(multiview_properties);

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.viewMask = 0x1;
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.CreateGraphicsPipeline();

    vkt::Image img(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView view = img.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderingInfo renderingInfo = vku::InitStructHelper();
    renderingInfo.renderArea = {{0, 0}, {100u, 100u}};
    renderingInfo.layerCount = 1u;
    renderingInfo.colorAttachmentCount = 1u;
    renderingInfo.pColorAttachments = &color_attachment;
    renderingInfo.viewMask = 0x1;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(renderingInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3u, 1u, 0u, multiview_properties.maxMultiviewInstanceIndex);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}
