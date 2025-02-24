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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class NegativeGpuAVDescriptorClassTexelBuffer : public GpuAVTest {};

TEST_F(NegativeGpuAVDescriptorClassTexelBuffer, GPLTexelFetch) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    vkt::Buffer uniform_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferView(0, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform samplerBuffer u_buffer;  // texel_buffer[4]
        layout(set = 0, binding = 1) buffer StorageBuffer { vec4 data; };
        void main() {
            data = texelFetch(u_buffer, 4);
        }
    )glsl";
    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), vertshader);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Descriptor Texel Buffer texel out of bounds", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassTexelBuffer, GPLImageLoad) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = storage_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferView(0, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0, r32f) uniform imageBuffer s_buffer; // texel_buffer[4]
        layout(set = 0, binding = 1) buffer StorageBuffer { vec4 data; };
        void main() {
            data = imageLoad(s_buffer, 4);
        }
    )glsl";
    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), vertshader);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Descriptor Texel Buffer texel out of bounds", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassTexelBuffer, GPLImageStore) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = storage_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferView(0, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0, r32f) uniform imageBuffer s_buffer; // texel_buffer[4]
        layout(set = 0, binding = 1) buffer StorageBuffer { vec4 data; };
        void main() {
            imageStore(s_buffer, 4, data);
        }
    )glsl";
    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), vertshader);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Descriptor Texel Buffer texel out of bounds", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassTexelBuffer, GPLTexelFetchIndependentSets) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    vkt::Buffer uniform_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet common_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    OneOffDescriptorSet fragment_set(m_device,
                                     {{1, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    const vkt::PipelineLayout pipeline_layout_vs(*m_device, {&vertex_set.layout_, &common_set.layout_, nullptr}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout_fs(*m_device, {nullptr, &common_set.layout_, &fragment_set.layout_}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&vertex_set.layout_, &common_set.layout_, &fragment_set.layout_}, {},
                                              VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    vertex_set.WriteDescriptorBufferInfo(0, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    common_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, VK_WHOLE_SIZE);
    common_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferView(1, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    fragment_set.UpdateDescriptorSets();

    const std::array<VkDescriptorSet, 3> desc_sets = {vertex_set.set_, common_set.set_, fragment_set.set_};

    uint32_t *data = (uint32_t *)offset_buffer.Memory().Map();
    *data = 8;
    offset_buffer.Memory().Unmap();

    static const char vert_shader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) uniform ufoo { uint index[]; };        // index[1]
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; }; // data[4]
        const vec2 vertices[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2(1.0, -1.0),
            vec2(0.0, 1.0)
        );
        void main() {
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";

    static const char frag_shader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) uniform ufoo { uint index[]; };      // index[1]
        layout(set = 2, binding = 1) uniform samplerBuffer u_buffer;      // texel_buffer[4]
        layout(set = 2, binding = 2, r32f) uniform imageBuffer s_buffer;  // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            c_out = texelFetch(u_buffer, 4);
        }
    )glsl";
    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), vert_shader, frag_shader);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Descriptor Texel Buffer texel out of bounds");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassTexelBuffer, GPLImageLoadStoreIndependentSets) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = storage_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet common_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    OneOffDescriptorSet fragment_set(m_device,
                                     {{2, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    const vkt::PipelineLayout pipeline_layout_vs(*m_device, {&vertex_set.layout_, &common_set.layout_, nullptr}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout_fs(*m_device, {nullptr, &common_set.layout_, &fragment_set.layout_}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&vertex_set.layout_, &common_set.layout_, &fragment_set.layout_}, {},
                                              VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    vertex_set.WriteDescriptorBufferInfo(0, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    common_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, VK_WHOLE_SIZE);
    common_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferView(2, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    fragment_set.UpdateDescriptorSets();

    const std::array<VkDescriptorSet, 3> desc_sets = {vertex_set.set_, common_set.set_, fragment_set.set_};

    static const char vert_shader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        const vec2 vertices[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2(1.0, -1.0),
            vec2(0.0, 1.0)
        );
        void main() {
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";

    static const char frag_shader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 2, binding = 2, r32f) uniform imageBuffer s_buffer;          // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            vec4 x = imageLoad(s_buffer, 5);
            imageStore(s_buffer, 5, x);
            c_out = x;
        }
    )glsl";
    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), vert_shader, frag_shader);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    // one for Load and one for Store
    m_errorMonitor->SetDesiredError("UNASSIGNED-Descriptor Texel Buffer texel out of bounds", 2);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassTexelBuffer, TexelFetch) {
    TEST_DESCRIPTION("index into a texelFetch OOB");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0, std430) buffer foo {
            vec4 a;
        } out_buffer;

        layout(set = 0, binding = 1) uniform samplerBuffer u_buffer; // texel_buffer[4]

        void main() {
            out_buffer.a = texelFetch(u_buffer, 4);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    // 16 bytes only holds 4 indexes
    vkt::Buffer uniform_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, out_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Descriptor Texel Buffer texel out of bounds");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassTexelBuffer, TexelFetchArray) {
    TEST_DESCRIPTION("index into texelFetch OOB for an array of TexelBuffers");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0, std430) buffer foo {
            vec4 a;
        } out_buffer;

        layout(set = 0, binding = 1) uniform samplerBuffer u_buffer[2];

        void main() {
            out_buffer.a = texelFetch(u_buffer[1], 4);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    vkt::Buffer uniform_texel_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView full_buffer_view(*m_device, bvci);
    bvci.range = 16;  // only fills 4, but are accessing index[4]
    vkt::BufferView partial_buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, out_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, full_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 0);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, partial_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Descriptor Texel Buffer texel out of bounds");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassTexelBuffer, ImageLoad) {
    TEST_DESCRIPTION("index into a imageLoad OOB");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, std430) buffer foo {
            vec4 a;
        } out_buffer;

        layout(set = 0, binding = 1, r32f) uniform imageBuffer s_buffer;  // texel_buffer[4]

        void main() {
            out_buffer.a = imageLoad(s_buffer, 4);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    // 16 bytes only holds 4 indexes
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = storage_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, out_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Descriptor Texel Buffer texel out of bounds");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassTexelBuffer, ImageStore) {
    TEST_DESCRIPTION("index into a imageStore OOB");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, r32f) uniform imageBuffer s_buffer;  // texel_buffer[4]

        void main() {
            vec4 x = imageLoad(s_buffer, 4); // invalid load
            imageStore(s_buffer, 5, x); // invalid store
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    // 16 bytes only holds 4 indexes
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = storage_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferView(0, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Descriptor Texel Buffer texel out of bounds", 2);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// This will hang a GPU
// Texel Buffers might not properly under robustBufferAccess like we think, need to investigate
TEST_F(NegativeGpuAVDescriptorClassTexelBuffer, DISABLED_ConstantArrayOOBTexture) {
    TEST_DESCRIPTION("index into texelFetch OOB for an array of TexelBuffers");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0, std430) buffer foo {
            int index;
            vec4 a;
        };

        layout(set = 0, binding = 1) uniform samplerBuffer u_buffer[2];

        void main() {
            a = texelFetch(u_buffer[index], 0);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    vkt::Buffer uniform_texel_buffer(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, storage_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 0);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1);
    pipe.descriptor_set_->UpdateDescriptorSets();

    uint32_t *data = (uint32_t *)storage_buffer.Memory().Map();
    *data = 8;
    storage_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-uniformBuffers-06935");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
