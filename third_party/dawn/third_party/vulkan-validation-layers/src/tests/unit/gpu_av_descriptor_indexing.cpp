/*
 * Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 * Copyright (c) 2020-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

#include "../layers/gpuav/shaders/gpuav_shaders_constants.h"

class NegativeGpuAVDescriptorIndexing : public GpuAVDescriptorIndexingTest {};

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayOOBBuffer) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors.");

    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info[7] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    for (int i = 1; i < 7; i++) {
        buffer_info[i].buffer = buffer1.handle();
        buffer_info[i].offset = 0;
        buffer_info[i].range = 4 * sizeof(float);
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info[1];
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
        )glsl";
    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 1) buffer foo { vec4 val; } colors[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = colors[index].val;
        }
        )glsl";

    {
        VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_.clear();
        pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
        pipe.shader_stages_.push_back(fs.GetStageCreateInfo());
        pipe.gp_ci_.layout = pipeline_layout.handle();
        pipe.CreateGraphicsPipeline();

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
        uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
        buffer_ptr[0] = 25;
        buffer0.Memory().Unmap();

        SCOPED_TRACE("Out of Bounds");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10068", gpuav::glsl::kMaxErrorsPerCmd);
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();

        buffer_ptr = (uint32_t *)buffer0.Memory().Map();
        buffer_ptr[0] = 5;
        buffer0.Memory().Unmap();

        SCOPED_TRACE("uninitialized");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114", gpuav::glsl::kMaxErrorsPerCmd);
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayOOBVertex) {
    TEST_DESCRIPTION(
        "Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors - vertex shader.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                      {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, VK_SHADER_STAGE_ALL, nullptr,
                       VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                  });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[6] = {};
    for (int i = 0; i < 6; i++) {
        image_info[i] = {sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    char const *vs_source = R"glsl(
        #version 450

        layout(std140, set = 0, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(set = 0, binding = 1) uniform sampler2D tex[6];
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           gl_Position += 1e-30 * texture(tex[uniform_index_buffer.tex_index[0]], vec2(0, 0));
        }
        )glsl";
    char const *fs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 1) uniform sampler2D tex[6];
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = texture(tex[0], vec2(0, 0));
        }
        )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_.clear();
    pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
    pipe.shader_stages_.push_back(fs.GetStageCreateInfo());
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 25;
    buffer0.Memory().Unmap();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10068", 2 * 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayOOBFragment) {
    TEST_DESCRIPTION(
        "Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors - Fragment shader.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                      {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, VK_SHADER_STAGE_ALL, nullptr,
                       VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                  });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[6] = {};
    for (int i = 0; i < 6; i++) {
        image_info[i] = {sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
        )glsl";
    char const *fs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 1) uniform sampler2D tex[6];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
        )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_.clear();
    pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
    pipe.shader_stages_.push_back(fs.GetStageCreateInfo());
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 25;
    buffer0.Memory().Unmap();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10068", gpuav::glsl::kMaxErrorsPerCmd);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayOOBRuntime) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors.");

    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    VkDescriptorBindingFlags ds_binding_flags[2] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &layout_createinfo_binding_flags,
                                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    uint32_t desc_counts = 6;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 6
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &desc_counts;

    OneOffDescriptorSet descriptor_set_variable(m_device,
                                                {
                                                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, VK_SHADER_STAGE_ALL, nullptr},
                                                },
                                                0, &layout_createinfo_binding_flags, 0, &variable_count);
    const vkt::PipelineLayout pipeline_layout_variable(*m_device, {&descriptor_set_variable.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[6] = {};
    for (int i = 0; i < 6; i++) {
        image_info[i] = {sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_set.set_;  // descriptor_set;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);
    descriptor_writes[0].dstSet = descriptor_set_variable.set_;
    descriptor_writes[1].dstSet = descriptor_set_variable.set_;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
        )glsl";
    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 1) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
        )glsl";

    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_.clear();
    pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
    pipe.shader_stages_.push_back(fs.GetStageCreateInfo());
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 25;
    buffer0.Memory().Unmap();

    SCOPED_TRACE("Out of Bounds");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10068", gpuav::glsl::kMaxErrorsPerCmd);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 5;
    buffer0.Memory().Unmap();

    SCOPED_TRACE("Uninitialized");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114", gpuav::glsl::kMaxErrorsPerCmd);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayOOBVariableDescriptorCountAllocate) {
    TEST_DESCRIPTION(
        "detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors with "
        "VkDescriptorSetVariableDescriptorCountAllocateInfo.");

    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    VkDescriptorBindingFlags ds_binding_flags[2] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    uint32_t desc_counts = 2;  // We'll reserve 4 spaces in the layout, but the descriptor will only use 2
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &desc_counts;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0, &variable_count);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform foo { uint tex_index; };
        layout(set = 0, binding = 1) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = texture(tex[tex_index], vec2(0, 0));
        }
        )glsl";

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 2;
    buffer.Memory().Unmap();

    // VUID-vkCmdDraw-None-10068
    m_errorMonitor->SetDesiredError(
        "Descriptor index 2 is uninitialized. VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT was used and the original "
        "descriptorCount (4) could have been reduced during AllocateDescriptorSets",
        gpuav::glsl::kMaxErrorsPerCmd);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayOOBVariableDescriptorCountAllocateUninitialized) {
    TEST_DESCRIPTION(
        "detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors with "
        "VkDescriptorSetVariableDescriptorCountAllocateInfo.");

    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    VkDescriptorBindingFlags ds_binding_flags[2] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    uint32_t desc_counts = 2;  // We'll reserve 4 spaces in the layout, but the descriptor will only use 2
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &desc_counts;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0, &variable_count);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.UpdateDescriptorSets();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform foo { uint tex_index; };
        layout(set = 0, binding = 1) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = texture(tex[tex_index], vec2(0, 0));
        }
        )glsl";

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 1;
    buffer.Memory().Unmap();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114", gpuav::glsl::kMaxErrorsPerCmd);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayOOBTess) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    if (!m_device->Physical().Features().tessellationShader) {
        GTEST_SKIP() << "Tessellation not supported";
    }

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info[7] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    for (int i = 1; i < 7; i++) {
        buffer_info[i].buffer = buffer1.handle();
        buffer_info[i].offset = 0;
        buffer_info[i].range = 4 * sizeof(float);
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info[1];
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    const char *tesSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(std140, set = 0, binding = 0) uniform ufoo { uint index; } uniform_index_buffer;
        layout(set = 0, binding = 1) buffer bfoo { vec4 val; } adds[];
        layout(triangles, equal_spacing, cw) in;
        void main() {
            gl_Position = adds[uniform_index_buffer.index].val;
        }
    )glsl";

    VkShaderObj tcs(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, tesSource, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci = vku::InitStructHelper();
    iasci.primitiveRestartEnable = VK_FALSE;
    iasci.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    VkPipelineTessellationDomainOriginStateCreateInfo domain_origin_state = vku::InitStructHelper();
    domain_origin_state.domainOrigin = VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT;
    VkPipelineTessellationStateCreateInfo tsci = vku::InitStructHelper(&domain_origin_state);
    tsci.patchControlPoints = 3;

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                           pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.pTessellationState = &tsci;
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 25;
    buffer0.Memory().Unmap();

    SCOPED_TRACE("Out of Bounds");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10068", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 5;
    buffer0.Memory().Unmap();

    SCOPED_TRACE("Uninitialized");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayOOBGeom) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    if (!m_device->Physical().Features().geometryShader) {
        GTEST_SKIP() << "Geometry shaders not supported";
    }

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info[7] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    for (int i = 1; i < 7; i++) {
        buffer_info[i].buffer = buffer1.handle();
        buffer_info[i].offset = 0;
        buffer_info[i].range = 4 * sizeof(float);
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info[1];
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    const char vs_source[] = R"glsl(
        #version 450
        layout(location=0) out foo {vec4 val;} gs_out[3];
        void main() {
           gs_out[0].val = vec4(0);
           gl_Position = vec4(1);
        }
    )glsl";
    char const *gs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(triangles) in;
        layout(triangle_strip, max_vertices=3) out;
        layout(location=0) in VertexData { vec4 x; } gs_in[];
        layout(std140, set = 0, binding = 0) uniform ufoo { uint index; } uniform_index_buffer;
        layout(set = 0, binding = 1) buffer bfoo { vec4 val; } adds[];
        void main() {
           gl_Position = gs_in[0].x + adds[uniform_index_buffer.index].val.x;
           EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gs_source, VK_SHADER_STAGE_GEOMETRY_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 25;
    buffer0.Memory().Unmap();

    SCOPED_TRACE("Out of Bounds");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10068");
    // On Windows Arm, it re-runs the geometry shader 3 times on same primitive
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-None-10068");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-None-10068");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 5;
    buffer0.Memory().Unmap();

    SCOPED_TRACE("Uninitialized");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    // On Windows Arm, it re-runs the geometry shader 3 times on same primitive
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-None-08114");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayOOBCompute) {
    TEST_DESCRIPTION(
        "GPU validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors.");

    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info[7] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    for (int i = 1; i < 7; i++) {
        buffer_info[i].buffer = buffer1.handle();
        buffer_info[i].offset = 0;
        buffer_info[i].range = 4 * sizeof(float);
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info[1];
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    char const *csSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform ufoo { uint index; } u_index;
        layout(set = 0, binding = 1) buffer StorageBuffer {
            uint data;
        } Data[];
        void main() {
            Data[(u_index.index - 1)].data = Data[u_index.index].data;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    {
        SCOPED_TRACE("Uninitialized");
        uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
        buffer_ptr[0] = 5;
        buffer0.Memory().Unmap();
        // Invalid read
        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }

    {
        SCOPED_TRACE("Out of Bounds");
        uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
        buffer_ptr[0] = 25;
        buffer0.Memory().Unmap();
        // Invalid read and invalid write
        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-10068", 2);
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayEarlyDelete) {
    TEST_DESCRIPTION("GPU validation: Verify detection descriptors where resources have been deleted while in use.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    VkDescriptorBindingFlags ds_binding_flags[2] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};
    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    const uint32_t kDescCount = 2;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 2
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &kDescCount;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0, &variable_count);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buffer_info[kDescCount] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[kDescCount] = {};
    for (int i = 0; i < kDescCount; i++) {
        image_info[i] = {sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 2;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
        )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 1) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
        )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 1;
    buffer0.Memory().Unmap();

    // NOTE: object in use checking is entirely disabled for bindless descriptor sets so
    // destroying before submit still needs to be caught by GPU-AV. Once GPU-AV no
    // longer does QueueWaitIdle() in each submit call, we should also be able to detect
    // resource destruction while a submission is blocked on a semaphore as well.
    image.destroy();

    // UNASSIGNED-Descriptor destroyed
    m_errorMonitor->SetDesiredError("(set = 0, binding = 1) Descriptor index 1 references a resource that was destroyed.",
                                    gpuav::glsl::kMaxErrorsPerCmd);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ArrayEarlySamplerDelete) {
    TEST_DESCRIPTION("GPU validation: Verify detection descriptors where resources have been deleted while in use.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    VkDescriptorBindingFlags ds_binding_flags[2] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};
    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    const uint32_t kDescCount = 2;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 2
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &kDescCount;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0, &variable_count);

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkDescriptorBufferInfo buffer_info[kDescCount] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[kDescCount] = {};
    for (int i = 0; i < kDescCount; i++) {
        image_info[i] = {sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = descriptor_set.set_;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorCount = 2;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
         )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 1) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
         )glsl";

    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 1;
    buffer0.Memory().Unmap();

    // NOTE: object in use checking is entirely disabled for bindless descriptor sets so
    // destroying before submit still needs to be caught by GPU-AV. Once GPU-AV no
    // longer does QueueWaitIdle() in each submit call, we should also be able to detect
    // resource destruction while a submission is blocked on a semaphore as well.
    sampler.destroy();

    // UNASSIGNED-Descriptor destroyed
    m_errorMonitor->SetDesiredError("(set = 0, binding = 1) Descriptor index 1 references a resource that was destroyed.",
                                    gpuav::glsl::kMaxErrorsPerCmd);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, UpdateAfterBind) {
    TEST_DESCRIPTION("Exercise errors for updating a descriptor set after it is bound.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkDescriptorBindingFlags flags[3] = {0, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                                         VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 3;
    flags_create_info.pBindingFlags = &flags[0];

    // Descriptor set has two bindings - only the second is update_after_bind
    VkDescriptorSetLayoutBinding binding[3] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    ds_layout_ci.bindingCount = 3;
    ds_layout_ci.pBindings = &binding[0];
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    VkDescriptorPoolSize pool_sizes[3] = {
        {binding[0].descriptorType, binding[0].descriptorCount},
        {binding[1].descriptorType, binding[1].descriptorCount},
        {binding[2].descriptorType, binding[2].descriptorCount},
    };
    VkDescriptorPoolCreateInfo dspci = vku::InitStructHelper();
    dspci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    dspci.poolSizeCount = 3;
    dspci.pPoolSizes = &pool_sizes[0];
    dspci.maxSets = 1;
    vkt::DescriptorPool pool(*m_device, dspci);

    VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper();
    ds_alloc_info.descriptorPool = pool.handle();
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout.handle();

    VkDescriptorSet ds = VK_NULL_HANDLE;
    vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);

    vkt::Buffer dynamic_uniform_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkDescriptorBufferInfo buffInfo[2] = {};
    buffInfo[0].buffer = dynamic_uniform_buffer.handle();
    buffInfo[0].offset = 0;
    buffInfo[0].range = 1024;

    VkWriteDescriptorSet descriptor_write[2] = {};
    descriptor_write[0] = vku::InitStructHelper();
    descriptor_write[0].dstSet = ds;
    descriptor_write[0].dstBinding = 0;
    descriptor_write[0].descriptorCount = 1;
    descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write[0].pBufferInfo = buffInfo;
    descriptor_write[1] = descriptor_write[0];
    descriptor_write[1].dstBinding = 1;
    descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    // Create a dummy pipeline, since VL inspects which bindings are actually used at draw time
    char const *fs_source = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        layout(set=0, binding=0) uniform foo0 { float x0; } bar0;
        layout(set=0, binding=1) buffer  foo1 { float x1; } bar1;
        layout(set=0, binding=2) buffer  foo2 { float x2; } bar2;
        void main(){
           color = vec4(bar0.x0 + bar1.x1 + bar2.x2);
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    // Make both bindings valid before binding to the command buffer
    vk::UpdateDescriptorSets(device(), 2, &descriptor_write[0], 0, nullptr);

    m_command_buffer.Begin();

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &ds, 0, nullptr);

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();

    // Valid to update binding 1 after being bound
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write[1], 0, nullptr);

    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114", gpuav::glsl::kMaxErrorsPerCmd);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, VariableDescriptorCountAllocateAfterPipeline) {
    TEST_DESCRIPTION(
        "use VARIABLE_DESCRIPTOR_COUNT_BIT and allocate after creating the pipeline (so the pipeline layout has the full count "
        "still).");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::descriptorBindingVariableDescriptorCount);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    VkDescriptorBindingFlags flags[3] = {
        0, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 3;
    flags_create_info.pBindingFlags = flags;

    VkDescriptorSetLayoutBinding binding[3] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    ds_layout_ci.bindingCount = 3;
    ds_layout_ci.pBindings = &binding[0];
    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) uniform UBO { uint in_buffer; };
        layout(set=0, binding=3) buffer  SSBO_B { float x; } B[4];
        layout(set=0, binding=5) buffer  SSBO_C { float x; } C[];
        void main(){
           C[in_buffer].x = B[2].x;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    VkDescriptorPoolSize pool_sizes[3] = {
        {binding[0].descriptorType, binding[0].descriptorCount},
        {binding[1].descriptorType, binding[1].descriptorCount},
        {binding[2].descriptorType, binding[2].descriptorCount},
    };
    VkDescriptorPoolCreateInfo dspci = vku::InitStructHelper();
    dspci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    dspci.poolSizeCount = 3;
    dspci.pPoolSizes = &pool_sizes[0];
    dspci.maxSets = 1;
    vkt::DescriptorPool pool(*m_device, dspci);

    uint32_t desc_counts = 6;  // We'll reserve 8 spaces in the layout, but the descriptor will only use 6
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count = vku::InitStructHelper();
    variable_count.descriptorSetCount = 1;
    variable_count.pDescriptorCounts = &desc_counts;

    VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper(&variable_count);
    ds_alloc_info.descriptorPool = pool.handle();
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout.handle();

    VkDescriptorSet ds = VK_NULL_HANDLE;
    vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *in_buffer_ptr = (uint32_t *)in_buffer.Memory().Map();
    in_buffer_ptr[0] = 7;  // point to index that no longer exsist because the variable count shrunk it
    in_buffer.Memory().Unmap();

    vkt::Buffer storage_buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    VkDescriptorBufferInfo desc_buffer_infos[2] = {{in_buffer.handle(), 0, VK_WHOLE_SIZE},
                                                   {storage_buffer.handle(), 0, VK_WHOLE_SIZE}};

    VkWriteDescriptorSet descriptor_write[3] = {};
    descriptor_write[0] = vku::InitStructHelper();
    descriptor_write[0].dstSet = ds;
    descriptor_write[0].dstBinding = 0;
    descriptor_write[0].descriptorCount = 1;
    descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write[0].pBufferInfo = &desc_buffer_infos[0];
    descriptor_write[1] = descriptor_write[0];
    descriptor_write[1].dstBinding = 3;
    descriptor_write[1].dstArrayElement = 2;
    descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write[1].pBufferInfo = &desc_buffer_infos[1];
    descriptor_write[2] = descriptor_write[1];
    descriptor_write[2].dstBinding = 5;
    descriptor_write[2].dstArrayElement = 4;
    vk::UpdateDescriptorSets(device(), 3, descriptor_write, 0, nullptr);

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    // VUID-vkCmdDispatch-None-10068
    m_errorMonitor->SetDesiredError(
        "Descriptor index 7 is uninitialized. VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT was used and the original "
        "descriptorCount (8) could have been reduced during AllocateDescriptorSets");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, BasicHLSL) {
    TEST_DESCRIPTION("Basic indexing into a valid descriptor index with HLSL");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 5;  // go past textures[4]
    buffer.Memory().Unmap();

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.UpdateDescriptorSets();

    // struct Data {
    //     uint index;
    //     float4 output;
    // };
    // RWStructuredBuffer<Data> data : register(u0);
    //
    // SamplerState ss : register(s1);
    // Texture2D textures[4] : register(t2);
    //
    // [numthreads(1, 1, 1)]
    // void main(uint3 tid : SV_DispatchThreadID) {
    //     data[0].output = textures[data[0].index].SampleLevel(ss, float2(0,0), 0);
    // }
    char const *cs_source = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %data %ss %textures
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %data DescriptorSet 0
               OpDecorate %data Binding 0
               OpDecorate %ss DescriptorSet 0
               OpDecorate %ss Binding 1
               OpDecorate %textures DescriptorSet 0
               OpDecorate %textures Binding 2
               OpMemberDecorate %Data 0 Offset 0
               OpMemberDecorate %Data 1 Offset 16
               OpDecorate %_runtimearr_Data ArrayStride 32
               OpMemberDecorate %type_RWStructuredBuffer_Data 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_Data Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
      %int_1 = OpConstant %int 1
    %v4float = OpTypeVector %float 4
       %Data = OpTypeStruct %uint %v4float
%_runtimearr_Data = OpTypeRuntimeArray %Data
%type_RWStructuredBuffer_Data = OpTypeStruct %_runtimearr_Data
%_ptr_StorageBuffer_type_RWStructuredBuffer_Data = OpTypePointer StorageBuffer %type_RWStructuredBuffer_Data
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
     %uint_4 = OpConstant %uint 4
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_4 = OpTypeArray %type_2d_image %uint_4
%_ptr_UniformConstant__arr_type_2d_image_uint_4 = OpTypePointer UniformConstant %_arr_type_2d_image_uint_4
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
       %data = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_Data StorageBuffer
         %ss = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
   %textures = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_4 UniformConstant
       %main = OpFunction %void None %27
         %31 = OpLabel
         %32 = OpAccessChain %_ptr_StorageBuffer_uint %data %int_0 %uint_0 %int_0
         %33 = OpLoad %uint %32
         %34 = OpAccessChain %_ptr_UniformConstant_type_2d_image %textures %33
         %35 = OpLoad %type_2d_image %34
         %36 = OpLoad %type_sampler %ss
         %37 = OpSampledImage %type_sampled_image %35 %36
         %38 = OpImageSampleExplicitLod %v4float %37 %18 Lod %float_0
         %39 = OpAccessChain %_ptr_StorageBuffer_v4float %data %int_0 %uint_0 %int_1
               OpStore %39 %38
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-10068");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, BasicHLSLRuntimeArray) {
    TEST_DESCRIPTION("Basic indexing into a valid descriptor index with HLSL via runtime array");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    // send index to select in image array
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 7;
    buffer.Memory().Unmap();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    // indexing into textures[7], but don't fill it
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 2);
    descriptor_set.UpdateDescriptorSets();

    // struct Data {
    //     uint index;
    //     float4 output;
    // };
    // RWStructuredBuffer<Data> data : register(u0);
    //
    // SamplerState ss : register(s1);
    // Texture2D textures[] : register(t2);
    //
    // [numthreads(1, 1, 1)]
    // void main(uint3 tid : SV_DispatchThreadID) {
    //     data[0].output = textures[data[0].index].SampleLevel(ss, float2(0,0), 0);
    // }
    char const *cs_source = R"asm(
               OpCapability Shader
               OpCapability RuntimeDescriptorArray
               OpExtension "SPV_EXT_descriptor_indexing"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %data %ss %textures
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %data DescriptorSet 0
               OpDecorate %data Binding 0
               OpDecorate %ss DescriptorSet 0
               OpDecorate %ss Binding 1
               OpDecorate %textures DescriptorSet 0
               OpDecorate %textures Binding 2
               OpMemberDecorate %Data 0 Offset 0
               OpMemberDecorate %Data 1 Offset 16
               OpDecorate %_runtimearr_Data ArrayStride 32
               OpMemberDecorate %type_RWStructuredBuffer_Data 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_Data Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
      %int_1 = OpConstant %int 1
    %v4float = OpTypeVector %float 4
       %Data = OpTypeStruct %uint %v4float
%_runtimearr_Data = OpTypeRuntimeArray %Data
%type_RWStructuredBuffer_Data = OpTypeStruct %_runtimearr_Data
%_ptr_StorageBuffer_type_RWStructuredBuffer_Data = OpTypePointer StorageBuffer %type_RWStructuredBuffer_Data
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_runtimearr_type_2d_image = OpTypeRuntimeArray %type_2d_image
%_ptr_UniformConstant__runtimearr_type_2d_image = OpTypePointer UniformConstant %_runtimearr_type_2d_image
       %void = OpTypeVoid
         %26 = OpTypeFunction %void
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
       %data = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_Data StorageBuffer
         %ss = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
   %textures = OpVariable %_ptr_UniformConstant__runtimearr_type_2d_image UniformConstant
       %main = OpFunction %void None %26
         %30 = OpLabel
         %31 = OpAccessChain %_ptr_StorageBuffer_uint %data %int_0 %uint_0 %int_0
         %32 = OpLoad %uint %31
         %33 = OpAccessChain %_ptr_UniformConstant_type_2d_image %textures %32
         %34 = OpLoad %type_2d_image %33
         %35 = OpLoad %type_sampler %ss
         %36 = OpSampledImage %type_sampled_image %34 %35
         %37 = OpImageSampleExplicitLod %v4float %36 %18 Lod %float_0
         %38 = OpAccessChain %_ptr_StorageBuffer_v4float %data %int_0 %uint_0 %int_1
               OpStore %38 %37
               OpReturn
               OpFunctionEnd
    )asm";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, PushConstant) {
    TEST_DESCRIPTION("Basic indexing from a push constant");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    std::vector<VkPushConstantRange> push_constant_ranges = {{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)}};
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_}, push_constant_ranges);

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(push_constant) uniform Input {
            uint index;
        } in_buffer;

        layout(set = 0, binding = 0) uniform sampler2D tex[];

        void main() {
           vec4 result = texture(tex[in_buffer.index], vec2(0, 0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();

    uint32_t index = 8;
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t),
                         &index);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-10068");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleIndexes) {
    TEST_DESCRIPTION("Mis-index multiple times");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    // send index to select in image array
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 3;
    buffer_ptr[1] = 0;  // valid
    buffer_ptr[2] = 5;
    buffer.Memory().Unmap();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Input {
            uint index_0;
            uint index_1;
            uint index_2;
        } in_buffer;

        layout(set = 0, binding = 1) uniform sampler2D tex[];

        void main() {
           vec2 uv = vec2(0, 0);
           vec4 result = texture(tex[in_buffer.index_0], uv);
           result += texture(tex[in_buffer.index_1], uv); // valid
           result += texture(tex[in_buffer.index_2], uv);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("(set = 0, binding = 1) Index of 3 used to index descriptor array of length 2");
    m_errorMonitor->SetDesiredError("(set = 0, binding = 1) Index of 5 used to index descriptor array of length 2");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleOOBInMultipleCmdBuffers) {
    TEST_DESCRIPTION(
        "Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors in multiple command "
        "buffers. At time of writing, this text is a combination of above tests ArrayOOBFragment and ArrayOOBCompute.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // 1st Command Buffer
    // ---

    vkt::CommandBuffer cb_1(*m_device, m_command_pool);

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set_cb_1(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                      {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, VK_SHADER_STAGE_ALL, nullptr,
                       VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                  });
    const vkt::PipelineLayout pipeline_layout_cb_1(*m_device, {&descriptor_set_cb_1.layout_});

    vkt::Image image_cb_1(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_cb_1.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view_cb_1 = image_cb_1.CreateView();
    vkt::Sampler sampler_cb_1(*m_device, SafeSaneSamplerCreateInfo());

    {
        VkDescriptorBufferInfo buffer_info[1] = {};
        buffer_info[0].buffer = buffer0.handle();
        buffer_info[0].offset = 0;
        buffer_info[0].range = sizeof(uint32_t);

        VkDescriptorImageInfo image_info[6] = {};
        for (int i = 0; i < 6; i++) {
            image_info[i] = {sampler_cb_1.handle(), image_view_cb_1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        }

        VkWriteDescriptorSet descriptor_writes[2] = {};
        descriptor_writes[0] = vku::InitStructHelper();
        descriptor_writes[0].dstSet = descriptor_set_cb_1.set_;  // descriptor_set;
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].pBufferInfo = buffer_info;
        descriptor_writes[1] = vku::InitStructHelper();
        descriptor_writes[1].dstSet = descriptor_set_cb_1.set_;  // descriptor_set;
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].pImageInfo = image_info;
        vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);
    }

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
        )glsl";
    char const *fs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 1) uniform sampler2D tex[6];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
        )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe_cb_1(*this);
    pipe_cb_1.shader_stages_.clear();
    pipe_cb_1.shader_stages_.push_back(vs.GetStageCreateInfo());
    pipe_cb_1.shader_stages_.push_back(fs.GetStageCreateInfo());
    pipe_cb_1.gp_ci_.layout = pipeline_layout_cb_1.handle();
    pipe_cb_1.CreateGraphicsPipeline();

    cb_1.Begin();
    cb_1.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(cb_1.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_cb_1.Handle());
    vk::CmdBindDescriptorSets(cb_1.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_cb_1, 0, 1, &descriptor_set_cb_1.set_,
                              0, nullptr);
    vk::CmdDraw(cb_1.handle(), 3, 1, 0, 0);
    cb_1.EndRenderPass();
    cb_1.End();
    {
        uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
        buffer_ptr[0] = 25;
        buffer0.Memory().Unmap();
    }

    // 2nd Command Buffer
    // ---

    vkt::CommandBuffer cb_2(*m_device, m_command_pool);

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0_cb_2(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1_cb_2(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set_cb_2(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout_cb_2(*m_device, {&descriptor_set_cb_2.layout_});
    {
        VkDescriptorBufferInfo buffer_info_cb_2[7] = {};

        buffer_info_cb_2[0].buffer = buffer0_cb_2.handle();
        buffer_info_cb_2[0].offset = 0;
        buffer_info_cb_2[0].range = sizeof(uint32_t);

        for (int i = 1; i < 7; i++) {
            buffer_info_cb_2[i].buffer = buffer1_cb_2.handle();
            buffer_info_cb_2[i].offset = 0;
            buffer_info_cb_2[i].range = 4 * sizeof(float);
        }

        VkWriteDescriptorSet descriptor_writes_cb_2[2] = {};

        descriptor_writes_cb_2[0] = vku::InitStructHelper();
        descriptor_writes_cb_2[0].dstSet = descriptor_set_cb_2.set_;
        descriptor_writes_cb_2[0].dstBinding = 0;
        descriptor_writes_cb_2[0].descriptorCount = 1;
        descriptor_writes_cb_2[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes_cb_2[0].pBufferInfo = &buffer_info_cb_2[0];

        descriptor_writes_cb_2[1] = vku::InitStructHelper();
        descriptor_writes_cb_2[1].dstSet = descriptor_set_cb_2.set_;
        descriptor_writes_cb_2[1].dstBinding = 1;
        descriptor_writes_cb_2[1].descriptorCount = 5;  // Intentionally don't write index 5
        descriptor_writes_cb_2[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor_writes_cb_2[1].pBufferInfo = &buffer_info_cb_2[1];
        vk::UpdateDescriptorSets(device(), 2, descriptor_writes_cb_2, 0, nullptr);
    }

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform ufoo { uint index; } u_index;
        layout(set = 0, binding = 1) buffer StorageBuffer {
            uint data;
        } Data[];
        void main() {
            Data[(u_index.index - 1)].data = Data[u_index.index].data;
        }
    )glsl";

    CreateComputePipelineHelper pipe_cb_2(*this);
    pipe_cb_2.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_cb_2.cp_ci_.layout = pipeline_layout_cb_2.handle();
    pipe_cb_2.CreateComputePipeline();

    cb_2.Begin();
    vk::CmdBindPipeline(cb_2.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_cb_2.Handle());
    vk::CmdBindDescriptorSets(cb_2.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_cb_2, 0, 1, &descriptor_set_cb_2.set_,
                              0, nullptr);
    vk::CmdDispatch(cb_2.handle(), 1, 1, 1);
    cb_2.End();
    {
        uint32_t *buffer_ptr = (uint32_t *)buffer0_cb_2.Memory().Map();
        buffer_ptr[0] = 25;
        buffer0_cb_2.Memory().Unmap();
    }

    m_errorMonitor->SetDesiredError("vkCmdDraw(): (set = 0, binding = 1) Index of 25 used to index descriptor array of length 6",
                                    gpuav::glsl::kMaxErrorsPerCmd);
    m_default_queue->Submit(cb_1);

    m_errorMonitor->SetDesiredError(
        "vkCmdDispatch(): (set = 0, binding = 1) Index of 25 used to index descriptor array of length 6");
    m_errorMonitor->SetDesiredError(
        "vkCmdDispatch(): (set = 0, binding = 1) Index of 24 used to index descriptor array of length 6");
    m_default_queue->Submit(cb_2);

    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleOOBTypesInOneCmdBuffer) {
    TEST_DESCRIPTION(
        "Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors coming from both "
        "graphics and compute pipelines in one command buffer. At time of writing, this text is a combination of above tests "
        "ArrayOOBFragment and ArrayOOBCompute.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set_cb_1(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                      {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, VK_SHADER_STAGE_ALL, nullptr,
                       VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                  });
    const vkt::PipelineLayout pipeline_layout_cb_1(*m_device, {&descriptor_set_cb_1.layout_});

    vkt::Image image_cb_1(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_cb_1.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view_cb_1 = image_cb_1.CreateView();
    vkt::Sampler sampler_cb_1(*m_device, SafeSaneSamplerCreateInfo());

    {
        VkDescriptorBufferInfo buffer_info[1] = {};
        buffer_info[0].buffer = buffer0.handle();
        buffer_info[0].offset = 0;
        buffer_info[0].range = sizeof(uint32_t);

        VkDescriptorImageInfo image_info[6] = {};
        for (int i = 0; i < 6; i++) {
            image_info[i] = {sampler_cb_1.handle(), image_view_cb_1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        }

        VkWriteDescriptorSet descriptor_writes[2] = {};
        descriptor_writes[0] = vku::InitStructHelper();
        descriptor_writes[0].dstSet = descriptor_set_cb_1.set_;  // descriptor_set;
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].pBufferInfo = buffer_info;
        descriptor_writes[1] = vku::InitStructHelper();
        descriptor_writes[1].dstSet = descriptor_set_cb_1.set_;  // descriptor_set;
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].descriptorCount = 5;  // Intentionally don't write index 5
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].pImageInfo = image_info;
        vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);
    }

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
        )glsl";
    char const *fs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 1) uniform sampler2D tex[6];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
        )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe_cb_1(*this);
    pipe_cb_1.shader_stages_.clear();
    pipe_cb_1.shader_stages_.push_back(vs.GetStageCreateInfo());
    pipe_cb_1.shader_stages_.push_back(fs.GetStageCreateInfo());
    pipe_cb_1.gp_ci_.layout = pipeline_layout_cb_1.handle();
    pipe_cb_1.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_cb_1.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_cb_1, 0, 1,
                              &descriptor_set_cb_1.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    {
        uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
        buffer_ptr[0] = 25;
        buffer0.Memory().Unmap();
    }

    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0_cb_2(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1_cb_2(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set_cb_2(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout_cb_2(*m_device, {&descriptor_set_cb_2.layout_});
    {
        VkDescriptorBufferInfo buffer_info_cb_2[7] = {};

        buffer_info_cb_2[0].buffer = buffer0_cb_2.handle();
        buffer_info_cb_2[0].offset = 0;
        buffer_info_cb_2[0].range = sizeof(uint32_t);

        for (int i = 1; i < 7; i++) {
            buffer_info_cb_2[i].buffer = buffer1_cb_2.handle();
            buffer_info_cb_2[i].offset = 0;
            buffer_info_cb_2[i].range = 4 * sizeof(float);
        }

        VkWriteDescriptorSet descriptor_writes_cb_2[2] = {};

        descriptor_writes_cb_2[0] = vku::InitStructHelper();
        descriptor_writes_cb_2[0].dstSet = descriptor_set_cb_2.set_;
        descriptor_writes_cb_2[0].dstBinding = 0;
        descriptor_writes_cb_2[0].descriptorCount = 1;
        descriptor_writes_cb_2[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes_cb_2[0].pBufferInfo = &buffer_info_cb_2[0];

        descriptor_writes_cb_2[1] = vku::InitStructHelper();
        descriptor_writes_cb_2[1].dstSet = descriptor_set_cb_2.set_;
        descriptor_writes_cb_2[1].dstBinding = 1;
        descriptor_writes_cb_2[1].descriptorCount = 5;  // Intentionally don't write index 5
        descriptor_writes_cb_2[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor_writes_cb_2[1].pBufferInfo = &buffer_info_cb_2[1];
        vk::UpdateDescriptorSets(device(), 2, descriptor_writes_cb_2, 0, nullptr);
    }

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform ufoo { uint index; } u_index;
        layout(set = 0, binding = 1) buffer StorageBuffer {
            uint data;
        } Data[];
        void main() {
            Data[(u_index.index - 1)].data = Data[u_index.index].data;
        }
    )glsl";

    CreateComputePipelineHelper pipe_cb_2(*this);
    pipe_cb_2.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_cb_2.cp_ci_.layout = pipeline_layout_cb_2.handle();
    pipe_cb_2.CreateComputePipeline();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_cb_2.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_cb_2, 0, 1,
                              &descriptor_set_cb_2.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    {
        uint32_t *buffer_ptr = (uint32_t *)buffer0_cb_2.Memory().Map();
        buffer_ptr[0] = 25;
        buffer0_cb_2.Memory().Unmap();
    }

    m_errorMonitor->SetDesiredError("vkCmdDraw(): (set = 0, binding = 1) Index of 25 used to index descriptor array of length 6",
                                    gpuav::glsl::kMaxErrorsPerCmd);

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "vkCmdDispatch(): (set = 0, binding = 1) Index of 25 used to index descriptor array of length 6");
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "vkCmdDispatch(): (set = 0, binding = 1) Index of 24 used to index descriptor array of length 6");
    m_default_queue->Submit(m_command_buffer);

    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, BindingOOB) {
    TEST_DESCRIPTION("Use a binding that is OOB and won't be in the internal layout representation");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    // send index to select in image array
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 1;
    buffer.Memory().Unmap();

    OneOffDescriptorSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                      {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                      {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},  // not used
                  });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, sizeof(uint32_t));
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Input {
            uint index;
        } in_buffer;

        layout(set = 0, binding = 2) uniform sampler2D tex[];

        void main() {
           vec4 result = texture(tex[in_buffer.index], vec2(0, 0));
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
    // VUID-vkCmdDispatch-None-08114
    m_errorMonitor->SetDesiredError("(set = 0, binding = 2) Descriptor index 1 is uninitialized");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleBoundDescriptorsSameSetFirst) {
    TEST_DESCRIPTION("Bind various descriptor sets and do dispatch on each of them, the first one is invalid");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer input_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *input_buffer_ptr = (uint32_t *)input_buffer.Memory().Map();
    input_buffer_ptr[0] = 1;
    input_buffer.Memory().Unmap();

    char const *cs_source_1 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform Input { uint index; };
        layout(set = 0, binding = 1) uniform sampler2D tex[];
        void main() {
           vec4 result = texture(tex[index], vec2(0, 0));
        }
    )glsl";

    CreateComputePipelineHelper pipe_1(*this);
    pipe_1.cs_ = std::make_unique<VkShaderObj>(this, cs_source_1, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_1.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr}};
    pipe_1.CreateComputePipeline();

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    // Don't find index [1] so this is invalid
    pipe_1.descriptor_set_->WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    pipe_1.descriptor_set_->WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    pipe_1.descriptor_set_->UpdateDescriptorSets();

    char const *cs_source_2 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 1) uniform Input { uint index; };
        layout(set = 0, binding = 2) buffer StorageBuffer { uint data; } storage_buffers[];
        void main() {
            storage_buffers[index].data = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe_2(*this);
    pipe_2.cs_ = std::make_unique<VkShaderObj>(this, cs_source_2, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_2.dsl_bindings_ = {{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr}};
    pipe_2.CreateComputePipeline();
    pipe_2.descriptor_set_->WriteDescriptorBufferInfo(1, input_buffer, 0, VK_WHOLE_SIZE);
    pipe_2.descriptor_set_->WriteDescriptorBufferInfo(2, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    pipe_2.descriptor_set_->WriteDescriptorBufferInfo(2, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipe_2.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_1.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_1.pipeline_layout_, 0, 1,
                              &pipe_1.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_2.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_2.pipeline_layout_, 0, 1,
                              &pipe_2.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleBoundDescriptorsSameSetLast) {
    TEST_DESCRIPTION("Bind various descriptor sets and do dispatch on each of them, the last one is invalid");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer input_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *input_buffer_ptr = (uint32_t *)input_buffer.Memory().Map();
    input_buffer_ptr[0] = 1;
    input_buffer.Memory().Unmap();

    char const *cs_source_1 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform Input { uint index; };
        layout(set = 0, binding = 1) uniform sampler2D tex[];
        void main() {
           vec4 result = texture(tex[index], vec2(0, 0));
        }
    )glsl";

    CreateComputePipelineHelper pipe_1(*this);
    pipe_1.cs_ = std::make_unique<VkShaderObj>(this, cs_source_1, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_1.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr}};
    pipe_1.CreateComputePipeline();

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    pipe_1.descriptor_set_->WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    pipe_1.descriptor_set_->WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    pipe_1.descriptor_set_->WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    pipe_1.descriptor_set_->UpdateDescriptorSets();

    char const *cs_source_2 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 1) uniform Input { uint index; };
        layout(set = 0, binding = 2) buffer StorageBuffer { uint data; } storage_buffers[];
        void main() {
            storage_buffers[index].data = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe_2(*this);
    pipe_2.cs_ = std::make_unique<VkShaderObj>(this, cs_source_2, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_2.dsl_bindings_ = {{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr}};
    pipe_2.CreateComputePipeline();
    // Don't find index [1] so this is invalid
    pipe_2.descriptor_set_->WriteDescriptorBufferInfo(1, input_buffer, 0, VK_WHOLE_SIZE);
    pipe_2.descriptor_set_->WriteDescriptorBufferInfo(2, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    pipe_2.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_1.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_1.pipeline_layout_, 0, 1,
                              &pipe_1.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_2.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_2.pipeline_layout_, 0, 1,
                              &pipe_2.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleBoundDescriptorsUpdateAfterBindFirst) {
    TEST_DESCRIPTION("Bind various descriptor sets and do dispatch on each of them with UpdateAfterBind, the first one is invalid");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingUniformBufferUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();
    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer input_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *input_buffer_ptr = (uint32_t *)input_buffer.Memory().Map();
    input_buffer_ptr[0] = 1;  // will be valid for both shaders
    input_buffer.Memory().Unmap();

    char const *cs_source_1 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform Input { uint index; };
        layout(set = 0, binding = 1) uniform sampler2D tex[];
        void main() {
           vec4 result = texture(tex[index], vec2(0, 0));
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set_1(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr,
             VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout_1(*m_device, {&descriptor_set_1.layout_});

    CreateComputePipelineHelper pipe_1(*this);
    pipe_1.cs_ = std::make_unique<VkShaderObj>(this, cs_source_1, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_1.cp_ci_.layout = pipeline_layout_1.handle();
    pipe_1.CreateComputePipeline();

    char const *cs_source_2 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 1) uniform Input { uint index; };
        layout(set = 0, binding = 2) buffer StorageBuffer { uint data; } storage_buffers[];
        void main() {
            storage_buffers[index].data = 0;
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set_2(
        m_device,
        {
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout_2(*m_device, {&descriptor_set_2.layout_});

    CreateComputePipelineHelper pipe_2(*this);
    pipe_2.cs_ = std::make_unique<VkShaderObj>(this, cs_source_2, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_2.cp_ci_.layout = pipeline_layout_2.handle();
    pipe_2.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_1.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_1, 0, 1,
                              &descriptor_set_1.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_2.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_2, 0, 1,
                              &descriptor_set_2.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set_1.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set_1.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set_1.UpdateDescriptorSets();

    descriptor_set_2.WriteDescriptorBufferInfo(1, input_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set_2.WriteDescriptorBufferInfo(2, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set_2.WriteDescriptorBufferInfo(2, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set_2.UpdateDescriptorSets();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleBoundDescriptorsUpdateAfterBindLast) {
    TEST_DESCRIPTION("Bind various descriptor sets and do dispatch on each of them with UpdateAfterBind, the last one is invalid");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingUniformBufferUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer input_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *input_buffer_ptr = (uint32_t *)input_buffer.Memory().Map();
    input_buffer_ptr[0] = 1;  // will be valid for both shaders
    input_buffer.Memory().Unmap();

    char const *cs_source_1 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform Input { uint index; };
        layout(set = 0, binding = 1) uniform sampler2D tex[];
        void main() {
           vec4 result = texture(tex[index], vec2(0, 0));
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set_1(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr,
             VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout_1(*m_device, {&descriptor_set_1.layout_});

    CreateComputePipelineHelper pipe_1(*this);
    pipe_1.cs_ = std::make_unique<VkShaderObj>(this, cs_source_1, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_1.cp_ci_.layout = pipeline_layout_1.handle();
    pipe_1.CreateComputePipeline();

    char const *cs_source_2 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 1) uniform Input { uint index; };
        layout(set = 0, binding = 2) buffer StorageBuffer { uint data; } storage_buffers[];
        void main() {
            storage_buffers[index].data = 0;
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set_2(
        m_device,
        {
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout_2(*m_device, {&descriptor_set_2.layout_});

    CreateComputePipelineHelper pipe_2(*this);
    pipe_2.cs_ = std::make_unique<VkShaderObj>(this, cs_source_2, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_2.cp_ci_.layout = pipeline_layout_2.handle();
    pipe_2.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_1.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_1, 0, 1,
                              &descriptor_set_1.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_2.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_2, 0, 1,
                              &descriptor_set_2.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set_1.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set_1.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set_1.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set_1.UpdateDescriptorSets();

    descriptor_set_2.WriteDescriptorBufferInfo(1, input_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set_2.WriteDescriptorBufferInfo(2, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set_2.UpdateDescriptorSets();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleSetSomeUninitialized) {
    TEST_DESCRIPTION("Layout are the same, but some set are uninitialized");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer input_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *input_buffer_ptr = (uint32_t *)input_buffer.Memory().Map();
    input_buffer_ptr[0] = 1;  // storage_buffers[1]
    input_buffer.Memory().Unmap();

    OneOffDescriptorSet ds_good(m_device, {
                                              {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                              {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                          });
    OneOffDescriptorSet ds_bad(m_device, {
                                             {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                             {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                         });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_good.layout_});

    ds_good.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    ds_good.WriteDescriptorBufferInfo(1, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    ds_good.WriteDescriptorBufferInfo(1, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    ds_good.UpdateDescriptorSets();

    ds_bad.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    ds_bad.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Index { uint index; };
        layout(set = 0, binding = 1) buffer Input {
            uint data;
        } storage_buffers[];

        void main() {
           storage_buffers[index].data = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_bad.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    // bad
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_bad.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    // bad
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_bad.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114", 2);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleSetSomeUninitializedUpdateAfterBind) {
    TEST_DESCRIPTION("Layout are the same, but some set are uninitialized");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingUniformBufferUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer input_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *input_buffer_ptr = (uint32_t *)input_buffer.Memory().Map();
    input_buffer_ptr[0] = 1;  // storage_buffers[1]
    input_buffer.Memory().Unmap();

    OneOffDescriptorIndexingSet ds_good(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
        });
    OneOffDescriptorIndexingSet ds_bad(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_good.layout_});

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Index { uint index; };
        layout(set = 0, binding = 1) buffer Input {
            uint data;
        } storage_buffers[];

        void main() {
           storage_buffers[index].data = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_bad.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    // bad
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_bad.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    // bad
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_bad.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    ds_good.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    ds_good.WriteDescriptorBufferInfo(1, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    ds_good.WriteDescriptorBufferInfo(1, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    ds_good.UpdateDescriptorSets();

    ds_bad.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    ds_bad.UpdateDescriptorSets();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114", 2);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ReSubmitCommandBuffer) {
    TEST_DESCRIPTION("Make sure we are resetting the command buffer tracking correctly");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer input_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *input_buffer_ptr = (uint32_t *)input_buffer.Memory().Map();
    input_buffer_ptr[0] = 1;  // storage_buffers[1]
    input_buffer.Memory().Unmap();

    OneOffDescriptorSet ds_good(m_device, {
                                              {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                              {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                          });
    OneOffDescriptorSet ds_bad(m_device, {
                                             {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                             {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                         });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds_good.layout_});

    ds_good.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    ds_good.WriteDescriptorBufferInfo(1, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    ds_good.WriteDescriptorBufferInfo(1, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    ds_good.UpdateDescriptorSets();

    ds_bad.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    ds_bad.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Index { uint index; };
        layout(set = 0, binding = 1) buffer Input {
            uint data;
        } storage_buffers[];

        void main() {
           storage_buffers[index].data = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // don't submit but was valid
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    // bad now on re-record
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_bad.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, SpecConstantUpdateAfterBind) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    char const *cs_source = R"glsl(
        #version 450
        layout(constant_id = 0) const uint index = 0;

        layout(set = 0, binding = 0) buffer foo {
            uint a;
        } descriptors[2];

        void main() {
            descriptors[index].a = 0;
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                           VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create 2 pipelines with 2 values, make sure the first isn't cached or anything strange preventing the second one from
    // producing an error
    const uint32_t value_good = 1;
    const uint32_t value_bad = 5;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo spec_info_good = {1, &entry, sizeof(uint32_t), &value_good};
    VkSpecializationInfo spec_info_bad = {1, &entry, sizeof(uint32_t), &value_bad};

    CreateComputePipelineHelper pipe_good(*this);
    pipe_good.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                                                  &spec_info_good);
    pipe_good.cp_ci_.layout = pipeline_layout.handle();
    pipe_good.CreateComputePipeline();

    CreateComputePipelineHelper pipe_bad(*this);
    pipe_bad.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                                                 &spec_info_bad);
    pipe_bad.cp_ci_.layout = pipeline_layout.handle();
    pipe_bad.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_good.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_bad.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-10068");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, SpecConstantPartiallyBound) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    char const *cs_source = R"glsl(
        #version 450
        layout(constant_id = 0) const uint index = 0;

        layout(set = 0, binding = 0) buffer foo {
            uint a;
        } descriptors[2];

        void main() {
            descriptors[index].a = 0;
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                           VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create 2 pipelines with 2 values, make sure the first isn't cached or anything strange preventing the second one from
    // producing an error
    const uint32_t value_good = 1;
    const uint32_t value_bad = 0;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo spec_info_good = {1, &entry, sizeof(uint32_t), &value_good};
    VkSpecializationInfo spec_info_bad = {1, &entry, sizeof(uint32_t), &value_bad};

    CreateComputePipelineHelper pipe_good(*this);
    pipe_good.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                                                  &spec_info_good);
    pipe_good.cp_ci_.layout = pipeline_layout.handle();
    pipe_good.CreateComputePipeline();

    CreateComputePipelineHelper pipe_bad(*this);
    pipe_bad.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                                                 &spec_info_bad);
    pipe_bad.cp_ci_.layout = pipeline_layout.handle();
    pipe_bad.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    // Don't update index zero
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_good.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_bad.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, SpecConstant) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    char const *cs_source = R"glsl(
        #version 450
        layout(constant_id = 0) const uint index = 1;

        layout(set = 0, binding = 0) buffer foo {
            uint a;
        } descriptors[2];

        void main() {
            descriptors[index].a = 0;
        }
    )glsl";

    const uint32_t value = 5;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo spec_info = {1, &entry, sizeof(uint32_t), &value};

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                                             &spec_info);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-10068");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, SpecConstantNullDescriptorBindless) {
    TEST_DESCRIPTION("Use null descriptor, but forget to set to VK_NULL_HANDLE so it is uninitialized");
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    char const *cs_source = R"glsl(
        #version 450
        // will update to zero which is uninitialized
        layout(constant_id = 0) const uint index = 1;

        layout(set = 0, binding = 0) buffer foo {
            uint a;
        } descriptors[2];

        void main() {
            descriptors[index].a = 0;
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                           VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    descriptor_set.WriteDescriptorBufferInfo(0, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set.UpdateDescriptorSets();

    const uint32_t value = 0;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo spec_info = {1, &entry, sizeof(uint32_t), &value};

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                                             &spec_info);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    // VUID-vkCmdDispatch-None-08114
    m_errorMonitor->SetDesiredError(
        "Descriptor index 0 is uninitialized. nullDescriptor feature is on, but vkUpdateDescriptorSets was not called with "
        "VK_NULL_HANDLE for this descriptor");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, PartiallyBoundNoArray) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    char const *cs_source = R"glsl(
        #version 450
        layout(constant_id = 0) const uint index = 0;

        // No array, but descriptor will be uninitialized
        layout(set = 0, binding = 0) buffer foo {
            uint a;
        } descriptors;

        void main() {
            descriptors.a = 0;
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr,
                                                           VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    // VUID-vkCmdDispatch-None-08114
    m_errorMonitor->SetDesiredError("There is no array, but descriptor is viewed as having an array of length 1");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, TexelFetch) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        layout (set = 0, binding = 0) uniform sampler2D tex[];
        layout (location=0) out vec4 color;

        vec4 foo(uint i) {
            return texelFetch(tex[i], ivec2(0), 0);
        }

        void main() {
            color = foo(5);
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_, &descriptor_set.layout_});

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 2);
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10068", gpuav::glsl::kMaxErrorsPerCmd);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, ConstantArrayOOBBuffer) {
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set.UpdateDescriptorSets();

    const char vs_source[] = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform ufoo { uint index; };
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data; } Data[2];
        void main() {
            Data[index].data = 0xdeadca71;
        }
    )glsl";

    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    uint32_t *data = (uint32_t *)offset_buffer.Memory().Map();
    *data = 8;
    offset_buffer.Memory().Unmap();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10068", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8922
TEST_F(NegativeGpuAVDescriptorIndexing, DISABLED_NonUniformSamplers) {
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const char fs_source[] = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : require

        layout(push_constant, std430) uniform PushConstants {
            uint tId;
            uint sId;
        } pc;

        layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
        layout(set = 0, binding = 1) uniform sampler kSamplers[];
        layout(location = 0) out vec4 out_color;

        void main() {
            out_color = texture(nonuniformEXT(sampler2D(kTextures2D[pc.tId], kSamplers[pc.sId])), vec2(0));
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16};
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_}, {pc_range});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    uint32_t texture_id = 0;
    uint32_t sampler_id = 1;
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4, &texture_id);
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 4, 4, &sampler_id);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-10068", gpuav::glsl::kMaxErrorsPerCmd);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, TexelFetchNested) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        layout (set = 0, binding = 0) uniform sampler2D tex[];
        layout (set = 0, binding = 1) buffer SSBO { vec4 color; };

        vec4 bar(uint i) {
            return texelFetch(tex[i], ivec2(0), 0);
        }

        vec4 foo(uint i) {
            if (i > 3) {
                return bar(i);
            }
        }

        void main() {
            color = foo(5);
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 2);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-10068");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, TexelFetchTexelBuffer) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *cs_source = R"glsl(
        #version 460
        layout (set = 0, binding = 0) uniform samplerBuffer u_buffer;
        layout (set = 0, binding = 1) buffer SSBO { vec4 color; };

        void main() {
            color = texelFetch(u_buffer, 4);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device,
                                               {
                                                   {0, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr,
                                                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                   {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                                               });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, AtomicImagePartiallyBound) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, R32ui) uniform uimage2D atomic_image;

        void main() {
            uint y = imageAtomicLoad(atomic_image, ivec2(0), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
            imageAtomicStore(atomic_image, ivec2(0), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
            imageAtomicExchange(atomic_image, ivec2(0), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr,
                                                              VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                         });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    // One for the store, load, and exchange
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, AtomicImageRuntimeArray) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT);
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
        GTEST_SKIP() << "VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT is not supported.";
    }
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        #extension GL_KHR_memory_scope_semantics : enable

        layout(set = 0, binding = 0, R32ui) uniform uimage2D atomic_image_array[];

        void main() {
            uint y = imageAtomicLoad(atomic_image_array[1], ivec2(0), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
            imageAtomicStore(atomic_image_array[1], ivec2(0), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
            imageAtomicExchange(atomic_image_array[1], ivec2(0), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                           VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL, 0);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    // One for the store, load, and exchange
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, AtomicBufferPartiallyBound) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer ssbo { uint z; };
        void main() {
            atomicAdd(z, 1);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr,
                                                           VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, AtomicBufferRuntimeArray) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer ssbo { uint x; } atomic_buffers[];
        void main() {
            atomicAdd(atomic_buffers[1].x, 1);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                           VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    descriptor_set.WriteDescriptorBufferInfo(0, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, StorageImagePartiallyBound) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *cs_source = R"glsl(
        #version 450
        // VK_FORMAT_R32_UINT
        layout(set = 0, binding = 0, r32ui) uniform uimage2D storageImage;

        void main() {
            uvec4 texel = imageLoad(storageImage, ivec2(0, 0));
            imageStore(storageImage, ivec2(1, 1), texel * 2);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr,
                                                           VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, StorageImageRuntimeArray) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *cs_source = R"glsl(
        #version 450
        // VK_FORMAT_R32_UINT
        layout(set = 0, binding = 0, r32ui) uniform uimage2D storageImageArray[];

        void main() {
            uvec4 texel = imageLoad(storageImageArray[1], ivec2(0, 0));
            imageStore(storageImageArray[1], ivec2(1, 1), texel * 2);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                           VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleCommandBuffersSameDescriptorSet) {
    TEST_DESCRIPTION("two command buffers share same descriptor set, the second one makes invalid access");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    vkt::CommandBuffer cb_0(*m_device, m_command_pool);
    vkt::CommandBuffer cb_1(*m_device, m_command_pool);

    OneOffDescriptorIndexingSet descriptor_set(m_device,
                                               {
                                                   {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                   {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                                               });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source0 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) uniform sampler2D sample_array[]; // only [0] is set
        layout(set=0, binding=1) buffer SSBO {
            uint index;
            vec4 out_value;
        };
        void main() {
           out_value = texture(sample_array[index], vec2(0));
        }
    )glsl";
    CreateComputePipelineHelper pipe0(*this);
    pipe0.cs_ = std::make_unique<VkShaderObj>(this, cs_source0, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe0.cp_ci_.layout = pipeline_layout.handle();
    pipe0.CreateComputePipeline();

    // Create different shaders so pipelines are different
    char const *cs_source1 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) uniform sampler2D sample_array[2];
        layout(set=0, binding=1) buffer SSBO {
            uint index;
            float some_padding;
            vec4 out_value;
        };
        void main() {
           out_value = texture(sample_array[index], vec2(0));
        }
    )glsl";
    CreateComputePipelineHelper pipe1(*this);
    pipe1.cs_ = std::make_unique<VkShaderObj>(this, cs_source1, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe1.cp_ci_.layout = pipeline_layout.handle();
    pipe1.CreateComputePipeline();

    cb_0.Begin();
    vk::CmdBindPipeline(cb_0.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe0.Handle());
    vk::CmdBindDescriptorSets(cb_0.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb_0.handle(), 1, 1, 1);
    cb_0.End();

    cb_1.Begin();
    vk::CmdBindPipeline(cb_1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe1.Handle());
    vk::CmdBindDescriptorSets(cb_1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb_1.handle(), 1, 1, 1);
    cb_1.End();

    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 0;

    m_default_queue->Submit(cb_0);
    m_default_queue->Wait();

    buffer_ptr[0] = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(cb_1);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Invalid access should not carry back over
    buffer_ptr[0] = 0;
    m_default_queue->Submit(cb_0);
    m_default_queue->Wait();

    buffer.Memory().Unmap();
}

TEST_F(NegativeGpuAVDescriptorIndexing, CommandBufferRerecordSameDescriptorSet) {
    TEST_DESCRIPTION("rerecord the command buffer with an invalid pipeline the second time");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    OneOffDescriptorIndexingSet descriptor_set(m_device,
                                               {
                                                   {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                   {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                                               });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 0;
    buffer_ptr[1] = 1;
    buffer.Memory().Unmap();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source0 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) uniform sampler2D sample_array[]; // only [0] is set
        layout(set=0, binding=1) buffer SSBO {
            uint index_0; // has value 0
            uint index_1; // has value 1
            vec4 out_value;
        };
        void main() {
           out_value = texture(sample_array[index_0], vec2(0));
        }
    )glsl";
    CreateComputePipelineHelper pipe_good(*this);
    pipe_good.cs_ = std::make_unique<VkShaderObj>(this, cs_source0, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_good.cp_ci_.layout = pipeline_layout.handle();
    pipe_good.CreateComputePipeline();

    // Create different shaders so pipelines are different
    char const *cs_source1 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) uniform sampler2D sample_array[2];
        layout(set=0, binding=1) buffer SSBO {
            uint index_0; // has value 0
            uint index_1; // has value 1
            float some_padding;
            vec4 out_value;
        };
        void main() {
           out_value = texture(sample_array[index_1], vec2(0));
        }
    )glsl";
    CreateComputePipelineHelper pipe_bad(*this);
    pipe_bad.cs_ = std::make_unique<VkShaderObj>(this, cs_source1, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_bad.cp_ci_.layout = pipeline_layout.handle();
    pipe_bad.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_bad.Handle());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_good.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_bad.Handle());
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_good.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // record with bad pipeline
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_bad.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08114");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Re-record to make sure VU doesn't presist
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_good.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(NegativeGpuAVDescriptorIndexing, MultipleAccessChains) {
    TEST_DESCRIPTION("Slang will produce a chain of OpAccessChains");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    // struct Bar {
    //     uint x;
    // };
    //
    // [[vk::binding(0, 0)]]
    // RWStructuredBuffer<Bar> foo[4];
    //
    // [shader("compute")]
    // void main() {
    //     uint index = foo[0][0].x;
    //     foo[index][7].x = 8;
    // }
    char const *cs_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %foo
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %Bar_std430 0 Offset 0
               OpDecorate %_runtimearr_Bar_std430 ArrayStride 4
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %foo Binding 0
               OpDecorate %foo DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
 %Bar_std430 = OpTypeStruct %uint
%_runtimearr_Bar_std430 = OpTypeRuntimeArray %Bar_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_Bar_std430
        %int = OpTypeInt 32 1
      %int_4 = OpConstant %int 4
%_arr_RWStructuredBuffer_int_4 = OpTypeArray %RWStructuredBuffer %int_4
%_ptr_StorageBuffer__arr_RWStructuredBuffer_int_4 = OpTypePointer StorageBuffer %_arr_RWStructuredBuffer_int_4
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_Bar_std430 = OpTypePointer StorageBuffer %Bar_std430
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
      %int_7 = OpConstant %int 7
     %uint_8 = OpConstant %uint 8
        %foo = OpVariable %_ptr_StorageBuffer__arr_RWStructuredBuffer_int_4 StorageBuffer
       %main = OpFunction %void None %3
          %4 = OpLabel
         %15 = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer %foo %int_0
         %18 = OpAccessChain %_ptr_StorageBuffer_Bar_std430 %15 %int_0 %int_0
         %20 = OpAccessChain %_ptr_StorageBuffer_uint %18 %int_0
      %index = OpLoad %uint %20
         %22 = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer %foo %index
         %24 = OpAccessChain %_ptr_StorageBuffer_Bar_std430 %22 %int_0 %int_7
         %25 = OpAccessChain %_ptr_StorageBuffer_uint %24 %int_0
               OpStore %25 %uint_8
               OpReturn
               OpFunctionEnd
    )";

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3);
    descriptor_set.UpdateDescriptorSets();

    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 9;
    buffer.Memory().Unmap();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-10068");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
