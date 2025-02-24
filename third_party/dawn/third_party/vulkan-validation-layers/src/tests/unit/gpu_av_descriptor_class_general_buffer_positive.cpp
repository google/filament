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

class PositiveGpuAVDescriptorClassGeneralBuffer : public GpuAVDescriptorClassGeneralBuffer {};

void GpuAVDescriptorClassGeneralBuffer::ComputeStorageBufferTest(const char *shader, bool is_glsl, VkDeviceSize buffer_size,
                                                                 const char *expected_error) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2,
                                             is_glsl ? SPV_SOURCE_GLSL : SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    if (expected_error) m_errorMonitor->SetDesiredError(expected_error);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    if (expected_error) m_errorMonitor->VerifyFound();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, Basic) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);

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

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(2, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferView(3, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const char vs_source[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        layout(set = 0, binding = 2) buffer NullBuffer { uint data[]; } Null;     // VK_NULL_HANDLE
        layout(set = 0, binding = 3, r32f) uniform imageBuffer s_buffer;          // texel_buffer[4]
        void main() {
            vec4 x;
            if (u_index.index[0] == 1) {
                Data.data[0] = Null.data[40];
            }
            else if (u_index.index[0] == 2) {
                imageStore(s_buffer, 0, x);
            }
            else if (u_index.index[0] == 3) {
                x = imageLoad(s_buffer, 0);
            }
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
    *data = 1;
    offset_buffer.Memory().Unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    data = (uint32_t *)offset_buffer.Memory().Map();
    *data = 2;
    offset_buffer.Memory().Unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    data = (uint32_t *)offset_buffer.Memory().Map();
    *data = 3;
    offset_buffer.Memory().Unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, GPL) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    vkt::Buffer uniform_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, kHostVisibleMemProps);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);
    bvci.buffer = storage_texel_buffer.handle();
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {3, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {4, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(2, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferView(3, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.WriteDescriptorBufferView(4, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const char vs_source[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        layout(set = 0, binding = 2) buffer NullBuffer { uint data[]; } Null;     // VK_NULL_HANDLE
        layout(set = 0, binding = 3) uniform samplerBuffer u_buffer;              // texel_buffer[4]
        layout(set = 0, binding = 4, r32f) uniform imageBuffer s_buffer;          // texel_buffer[4]
        void main() {
            vec4 x;
            if (u_index.index[0] == 1) {
                Data.data[0] = Null.data[40];
            }
            else if (u_index.index[0] == 1) {
                imageStore(s_buffer, 0, x);
            }
            else if (u_index.index[0] == 2) {
                x = imageLoad(s_buffer, 0);
            }
        }
    )glsl";
    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), vs_source);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    uint32_t *data = (uint32_t *)offset_buffer.Memory().Map();
    *data = 1;
    offset_buffer.Memory().Unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    data = (uint32_t *)offset_buffer.Memory().Map();
    *data = 2;
    offset_buffer.Memory().Unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    data = (uint32_t *)offset_buffer.Memory().Map();
    *data = 3;
    offset_buffer.Memory().Unmap();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, GPLNonInlined) {
    TEST_DESCRIPTION("Make sure GPL works when shader modules are not inlined at pipeline creation time with valid GPU-AV code");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    uint32_t *offset_buffer_ptr = (uint32_t *)offset_buffer.Memory().Map();
    *offset_buffer_ptr = 8;
    offset_buffer.Memory().Unmap();

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform Uniform { uint offset_buffer[]; };
        layout(set = 0, binding = 1) buffer StorageBuffer { uint write_buffer[]; };
        void main() {
            uint index = offset_buffer[0];
            write_buffer[index] = 0xdeadca71;
        }
    )glsl";
    // Create VkShaderModule to pass in
    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    // For GPU-AV tests this shrinks things so only a single fragment is executed
    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};

    CreatePipelineHelper pre_raster_lib(*this);
    {
        pre_raster_lib.InitPreRasterLibInfo(&vs.GetStageCreateInfo());
        pre_raster_lib.vp_state_ci_.pViewports = &viewport;
        pre_raster_lib.vp_state_ci_.pScissors = &scissor;
        pre_raster_lib.gp_ci_.layout = pipeline_layout.handle();
        pre_raster_lib.CreateGraphicsPipeline();
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        frag_shader_lib.InitFragmentLibInfo(&fs.GetStageCreateInfo());
        frag_shader_lib.gp_ci_.layout = pipeline_layout.handle();
        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.Handle(),
        pre_raster_lib.Handle(),
        frag_shader_lib.Handle(),
        frag_out_lib.Handle(),
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pipeline_layout.handle();
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, exe_pipe.handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, GPLFragmentIndependentSets) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkDeviceSize buffer_size = 4;
    vkt::Buffer vs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer fs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet fragment_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    // Independent sets
    const vkt::PipelineLayout pipeline_layout_vs(*m_device, {&vertex_set.layout_, nullptr}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto vs_layout = pipeline_layout_vs.handle();
    const vkt::PipelineLayout pipeline_layout_fs(*m_device, {nullptr, &fragment_set.layout_}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto fs_layout = pipeline_layout_fs.handle();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&vertex_set.layout_, &fragment_set.layout_}, {},
                                              VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto layout = pipeline_layout.handle();

    vertex_set.WriteDescriptorBufferInfo(0, vs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferInfo(0, fs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    fragment_set.UpdateDescriptorSets();

    {
        vvl::span<uint32_t> vert_data(static_cast<uint32_t *>(vs_buffer.Memory().Map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : vert_data) {
            v = 0x01030507;
        }
        vs_buffer.Memory().Unmap();
    }
    {
        vvl::span<uint32_t> frag_data(static_cast<uint32_t *>(fs_buffer.Memory().Map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : frag_data) {
            v = 0x02040608;
        }
        fs_buffer.Memory().Unmap();
    }

    const std::array<VkDescriptorSet, 2> desc_sets = {vertex_set.set_, fragment_set.set_};

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer Input { uint u_buffer[]; } v_in; // texel_buffer[4]
        const vec2 vertices[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2(1.0, -1.0),
            vec2(0.0, 1.0)
        );
        void main() {
            if (gl_VertexIndex == 0) {
                const uint t = v_in.u_buffer[0];
            }
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vertshader);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};
    CreatePipelineHelper pre_raster_lib(*this);
    pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
    pre_raster_lib.vp_state_ci_.pViewports = &viewport;
    pre_raster_lib.vp_state_ci_.pScissors = &scissor;
    pre_raster_lib.gp_ci_.layout = vs_layout;
    pre_raster_lib.CreateGraphicsPipeline(false);

    static const char frag_shader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) buffer Input { uint u_buffer[]; } f_in; // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            c_out = vec4(1.0);
            const uint t = f_in.u_buffer[0];
        }
    )glsl";
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper frag_shader_lib(*this);
    frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
    frag_shader_lib.gp_ci_.layout = fs_layout;
    frag_shader_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.Handle(),
        pre_raster_lib.Handle(),
        frag_shader_lib.Handle(),
        frag_out_lib.Handle(),
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pre_raster_lib.gp_ci_.layout;
    vkt::Pipeline pipe(*m_device, exe_pipe_ci);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, VertexFragmentMultiEntrypoint) {
    TEST_DESCRIPTION("Same as negative test, but buffer are large enough");

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer uniform_buffer(*m_device, 256, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer storage_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, storage_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    // layout(set = 0, binding = 0) uniform ufoo { uint index[]; };
    // layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; };
    // layout(location = 0) out vec4 c_out;
    // void vert_main() {
    //     data[0] = index[4];
    //     gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
    // }
    // void frag_main() {
    //     data[4] = index[0];
    // }
    const char *shader_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %frag_main "frag_main" %c_out
               OpEntryPoint Vertex %vert_main "vert_main" %2 %gl_VertexIndex
               OpExecutionMode %frag_main OriginUpperLeft
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 1
               OpDecorate %_arr_uint_uint_5 ArrayStride 16
               OpMemberDecorate %_struct_10 0 Offset 0
               OpDecorate %_struct_10 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
               OpDecorate %c_out Location 0
               OpMemberDecorate %_struct_12 0 BuiltIn Position
               OpMemberDecorate %_struct_12 1 BuiltIn PointSize
               OpMemberDecorate %_struct_12 2 BuiltIn ClipDistance
               OpMemberDecorate %_struct_12 3 BuiltIn CullDistance
               OpDecorate %_struct_12 Block
               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_runtimearr_uint = OpTypeRuntimeArray %uint
  %_struct_7 = OpTypeStruct %_runtimearr_uint
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
          %8 = OpVariable %_ptr_Uniform__struct_7 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_5 = OpConstant %uint 5
%_arr_uint_uint_5 = OpTypeArray %uint %uint_5
 %_struct_10 = OpTypeStruct %_arr_uint_uint_5
%_ptr_Uniform__struct_10 = OpTypePointer Uniform %_struct_10
         %11 = OpVariable %_ptr_Uniform__struct_10 Uniform
      %int_4 = OpConstant %int 4
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
 %_struct_12 = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output__struct_12 = OpTypePointer Output %_struct_12
          %2 = OpVariable %_ptr_Output__struct_12 Output
    %v2float = OpTypeVector %float 2
     %uint_3 = OpConstant %uint 3
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
   %float_n1 = OpConstant %float -1
         %32 = OpConstantComposite %v2float %float_n1 %float_n1
    %float_1 = OpConstant %float 1
         %34 = OpConstantComposite %v2float %float_1 %float_n1
    %float_0 = OpConstant %float 0
         %36 = OpConstantComposite %v2float %float_0 %float_1
         %37 = OpConstantComposite %_arr_v2float_uint_3 %32 %34 %36
%_ptr_Input_int = OpTypePointer Input %int
%gl_VertexIndex = OpVariable %_ptr_Input_int Input
      %int_3 = OpConstant %int 3
%_ptr_Function__arr_v2float_uint_3 = OpTypePointer Function %_arr_v2float_uint_3
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
          %c_out = OpVariable %_ptr_Output_v4float Output
          %vert_main = OpFunction %void None %14
         %43 = OpLabel
         %44 = OpVariable %_ptr_Function__arr_v2float_uint_3 Function
         %45 = OpAccessChain %_ptr_Uniform_uint %11 %int_0 %int_4
         %46 = OpLoad %uint %45
         %47 = OpAccessChain %_ptr_Uniform_uint %8 %int_0 %int_0
               OpStore %47 %46
         %48 = OpLoad %int %gl_VertexIndex
         %49 = OpSMod %int %48 %int_3
               OpStore %44 %37
         %50 = OpAccessChain %_ptr_Function_v2float %44 %49
         %51 = OpLoad %v2float %50
         %52 = OpCompositeExtract %float %51 0
         %53 = OpCompositeExtract %float %51 1
         %54 = OpCompositeConstruct %v4float %52 %53 %float_0 %float_1
         %55 = OpAccessChain %_ptr_Output_v4float %2 %int_0
               OpStore %55 %54
               OpReturn
               OpFunctionEnd
          %frag_main = OpFunction %void None %14
         %56 = OpLabel
         %57 = OpAccessChain %_ptr_Uniform_uint %11 %int_0 %int_0
         %58 = OpLoad %uint %57
         %59 = OpAccessChain %_ptr_Uniform_uint %8 %int_0 %int_4
               OpStore %59 %58
               OpReturn
               OpFunctionEnd
        )";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr, "vert_main");
    VkShaderObj fs(this, shader_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr, "frag_main");

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, PartialBoundDescriptorSSBO) {
    TEST_DESCRIPTION("Only bound part of a SSBO, but only use that part so it is still valid");

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            vec4 a; // offset 0
            vec4 b; // offset 16
            vec4 c; // offset 32 - not bound, can't use
            vec4 d; // offset 48 - not bound, can't use
        };

        void main() {
            a = b;
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, PartialBoundDescriptorSSBOUpdateAfterBind) {
    TEST_DESCRIPTION("Only bound part of a SSBO (with update after bind), but only use that part so it is still valid");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            vec4 a; // offset 0
            vec4 b; // offset 16
            vec4 c; // offset 32 - not bound, can't use
            vec4 d; // offset 48 - not bound, can't use
        };
        void main() {
            a = b;
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr,
                                                           VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, PartialBoundDescriptorBuffer) {
    TEST_DESCRIPTION("Make large enough buffer, but only bound part of it to the SSBO that is used");

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            vec4 a; // offset 0
            vec4 b; // offset 16
            vec4 c; // offset 32 - not bound, can't use
            vec4 d; // offset 48 - not bound, can't use
        };

        void main() {
            a = b;
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    // only half of the buffer that is needed is
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 32, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, PartialBoundDescriptorCopy) {
    TEST_DESCRIPTION("Copy the partial bound buffer the descriptor that is used");

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            vec4 a; // offset 0
            vec4 b; // offset 16
            vec4 c; // offset 32 - not bound, can't use
            vec4 d; // offset 48 - not bound, can't use
        };

        void main() {
            a = b;
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set_src(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set_src.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set_src.UpdateDescriptorSets();

    OneOffDescriptorSet descriptor_set_dst(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_dst.layout_});

    VkCopyDescriptorSet descriptor_copy = vku::InitStructHelper();
    descriptor_copy.srcSet = descriptor_set_src.set_;
    descriptor_copy.srcBinding = 0;
    descriptor_copy.dstSet = descriptor_set_dst.set_;
    descriptor_copy.dstBinding = 0;
    descriptor_copy.dstArrayElement = 0;
    descriptor_copy.descriptorCount = 1;
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &descriptor_copy);

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_dst.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, RobustBuffer) {
    TEST_DESCRIPTION("OOB errors should not occur with robustness turned on");
    AddRequiredFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char vs_source[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        void main() {
            Data.data[u_index.index[0]] = 0xdeadca71;
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, VectorArray) {
    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            uvec4 a[8]; // stride 16
        };
        void main() {
            a[3].y = 44; // write at byte[52:56]
        }
    )glsl";

    ComputeStorageBufferTest(cs_source, true, 64);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, ArrayCopyGLSL) {
    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            uvec4 a;
            uint b[4];
            uint c;
        };

        void main() {
            uint d[4] = {4, 5, 6, 7};
            b = d;
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 32);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, ArrayCopySlang) {
    TEST_DESCRIPTION("Note that in slang and array copy is really a struct copy");
    // struct Bar {
    //     uint4 a;
    //     uint b[4];
    //     uint c;
    // };
    //
    // [[vk::binding(0, 0)]]
    // RWStructuredBuffer<Bar> foo;
    //
    // [shader("compute")]
    // void main() {
    //     uint d[4] = {4, 5, 6, 7};
    //     foo[0].b = d;
    // }
    char const *cs_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %foo
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_uint_int_4 ArrayStride 4
               OpMemberDecorate %_Array_std430_uint4 0 Offset 0
               OpMemberDecorate %Bar_std430 0 Offset 0
               OpMemberDecorate %Bar_std430 1 Offset 16
               OpMemberDecorate %Bar_std430 2 Offset 32
               OpDecorate %_runtimearr_Bar_std430 ArrayStride 48
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %foo Binding 0
               OpDecorate %foo DescriptorSet 0
               OpDecorate %_arr_uint_int_4_0 ArrayStride 4
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
      %int_4 = OpConstant %int 4
%_arr_uint_int_4 = OpTypeArray %uint %int_4
%_Array_std430_uint4 = OpTypeStruct %_arr_uint_int_4
 %Bar_std430 = OpTypeStruct %v4uint %_Array_std430_uint4 %uint
%_ptr_StorageBuffer_Bar_std430 = OpTypePointer StorageBuffer %Bar_std430
%_runtimearr_Bar_std430 = OpTypeRuntimeArray %Bar_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_Bar_std430
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
      %int_1 = OpConstant %int 1
%_ptr_StorageBuffer__Array_std430_uint4 = OpTypePointer StorageBuffer %_Array_std430_uint4
%_arr_uint_int_4_0 = OpTypeArray %uint %int_4
         %25 = OpTypeFunction %_Array_std430_uint4 %_arr_uint_int_4_0
     %uint_4 = OpConstant %uint 4
     %uint_5 = OpConstant %uint 5
     %uint_6 = OpConstant %uint 6
     %uint_7 = OpConstant %uint 7
         %35 = OpConstantComposite %_arr_uint_int_4_0 %uint_4 %uint_5 %uint_6 %uint_7
        %foo = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
       %main = OpFunction %void None %3
          %4 = OpLabel
         %14 = OpAccessChain %_ptr_StorageBuffer_Bar_std430 %foo %int_0 %int_0
         %21 = OpAccessChain %_ptr_StorageBuffer__Array_std430_uint4 %14 %int_1
         %22 = OpFunctionCall %_Array_std430_uint4 %packStorage %35
               OpStore %21 %22
               OpReturn
               OpFunctionEnd
%packStorage = OpFunction %_Array_std430_uint4 None %25
         %26 = OpFunctionParameter %_arr_uint_int_4_0
         %27 = OpLabel
         %28 = OpCompositeExtract %uint %26 0
         %29 = OpCompositeExtract %uint %26 1
         %30 = OpCompositeExtract %uint %26 2
         %31 = OpCompositeExtract %uint %26 3
         %32 = OpCompositeConstruct %_arr_uint_int_4 %28 %29 %30 %31
         %33 = OpCompositeConstruct %_Array_std430_uint4 %32
               OpReturnValue %33
               OpFunctionEnd
    )";
    ComputeStorageBufferTest(cs_source, false, 32);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, ArrayCopyTwoBindingsGLSL) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, std430) buffer foo1 {
            uvec4 a;
            uint b[4];
            uint c;
        };

        layout(set = 0, binding = 1, std430) buffer foo2 {
            uint d;
            uint e[4];
            uvec2 f;
        };

        void main() {
            b = e;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, 64, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(1, in_buffer.handle(), 64, 64, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, ArrayCopyTwoBindingsSlang) {
    TEST_DESCRIPTION("Note that in slang and array copy is really a struct copy");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // struct Bar1 {
    //     uint4 a;
    //     uint b[4];
    //     uint c;
    // };
    //
    // struct Bar2 {
    //     uint4 d;
    //     uint e[4];
    //     uint f;
    // };
    //
    // [[vk::binding(0, 0)]]
    // RWStructuredBuffer<Bar1> foo1;
    //
    // [[vk::binding(1, 0)]]
    // RWStructuredBuffer<Bar2> foo2;
    //
    // [shader("compute")]
    // void main() {
    //     foo1[0].b = foo2[0].e;
    // }
    char const *cs_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %foo1 %foo2
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_uint_int_4 ArrayStride 4
               OpMemberDecorate %_Array_std430_uint4 0 Offset 0
               OpMemberDecorate %Bar1_std430 0 Offset 0
               OpMemberDecorate %Bar1_std430 1 Offset 16
               OpMemberDecorate %Bar1_std430 2 Offset 32
               OpDecorate %_runtimearr_Bar1_std430 ArrayStride 48
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %foo1 Binding 0
               OpDecorate %foo1 DescriptorSet 0
               OpMemberDecorate %Bar2_std430 0 Offset 0
               OpMemberDecorate %Bar2_std430 1 Offset 16
               OpMemberDecorate %Bar2_std430 2 Offset 32
               OpDecorate %_runtimearr_Bar2_std430 ArrayStride 48
               OpDecorate %RWStructuredBuffer_0 Block
               OpMemberDecorate %RWStructuredBuffer_0 0 Offset 0
               OpDecorate %foo2 Binding 1
               OpDecorate %foo2 DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_4 = OpConstant %int 4
      %int_0 = OpConstant %int 0
     %v4uint = OpTypeVector %uint 4
%_arr_uint_int_4 = OpTypeArray %uint %int_4
%_Array_std430_uint4 = OpTypeStruct %_arr_uint_int_4
%Bar1_std430 = OpTypeStruct %v4uint %_Array_std430_uint4 %uint
%_ptr_StorageBuffer_Bar1_std430 = OpTypePointer StorageBuffer %Bar1_std430
%_runtimearr_Bar1_std430 = OpTypeRuntimeArray %Bar1_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_Bar1_std430
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
      %int_1 = OpConstant %int 1
%_ptr_StorageBuffer__Array_std430_uint4 = OpTypePointer StorageBuffer %_Array_std430_uint4
%Bar2_std430 = OpTypeStruct %v4uint %_Array_std430_uint4 %uint
%_ptr_StorageBuffer_Bar2_std430 = OpTypePointer StorageBuffer %Bar2_std430
%_runtimearr_Bar2_std430 = OpTypeRuntimeArray %Bar2_std430
%RWStructuredBuffer_0 = OpTypeStruct %_runtimearr_Bar2_std430
%_ptr_StorageBuffer_RWStructuredBuffer_0 = OpTypePointer StorageBuffer %RWStructuredBuffer_0
       %foo1 = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
       %foo2 = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer_0 StorageBuffer
       %main = OpFunction %void None %3
          %4 = OpLabel
         %17 = OpAccessChain %_ptr_StorageBuffer_Bar1_std430 %foo1 %int_0 %int_0
         %24 = OpAccessChain %_ptr_StorageBuffer__Array_std430_uint4 %17 %int_1
         %27 = OpAccessChain %_ptr_StorageBuffer_Bar2_std430 %foo2 %int_0 %int_0
         %32 = OpAccessChain %_ptr_StorageBuffer__Array_std430_uint4 %27 %int_1
         %33 = OpLoad %_Array_std430_uint4 %32
         %64 = OpCompositeExtract %_arr_uint_int_4 %33 0
         %65 = OpCompositeExtract %uint %64 0
         %66 = OpCompositeExtract %uint %64 1
         %67 = OpCompositeExtract %uint %64 2
         %68 = OpCompositeExtract %uint %64 3
        %110 = OpCompositeConstruct %_arr_uint_int_4 %65 %66 %67 %68
         %83 = OpCompositeConstruct %_Array_std430_uint4 %110
               OpStore %24 %83
               OpReturn
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, 64, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(1, in_buffer.handle(), 64, 64, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, StructCopyGLSL) {
    char const *cs_source = R"glsl(
        #version 450

        struct Bar {
            uint x;
            uint y;
            uint z[2];
        };

        layout(set = 0, binding = 0, std430) buffer foo {
            uvec4 a;
            Bar b; // size 16 at offset 16
            uint c;
        };

        void main() {
            Bar new_bar;
            b = new_bar;
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 32);
}
TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, StructCopyGLSL2) {
    char const *cs_source = R"glsl(
        #version 450

        struct Bar {
            vec4 x;
        };

        layout(set = 0, binding = 0, std430) buffer foo {
            uvec4 a;
            Bar b; // size 16 at offset 16
            uint c;
        };

        void main() {
            Bar new_bar;
            b = new_bar;
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 32);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, StructCopyGLSL3) {
    char const *cs_source = R"glsl(
        #version 450

        struct Bar2 {
            vec4 x;
        };

        struct Bar {
            uint x;
            Bar2 y;
        };

        layout(set = 0, binding = 0, std430) buffer foo {
            uvec4 a;
            uvec4 padding;
            Bar b; // size 32 at offset 32
            uint c;
        };

        void main() {
            Bar new_bar;
            b = new_bar;
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 64);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, StructCopySlang) {
    // struct Bar {
    //   uint x;
    //   uint y;
    //   uint z[2];
    // };
    //
    // struct FooBuffer {
    //   float4 a;
    //   Bar b;
    //   uint c;
    // };
    //
    // [[vk::binding(0, 0)]]
    // RWStructuredBuffer<FooBuffer> foo;
    //
    // [shader("compute")]
    // void main() {
    //   foo[1].b = foo[0].b;
    // }
    char const *cs_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %foo
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_uint_int_2 ArrayStride 4
               OpMemberDecorate %_Array_std430_uint2 0 Offset 0
               OpMemberDecorate %Bar_std430 0 Offset 0
               OpMemberDecorate %Bar_std430 1 Offset 4
               OpMemberDecorate %Bar_std430 2 Offset 8
               OpMemberDecorate %FooBuffer_std430 0 Offset 0
               OpMemberDecorate %FooBuffer_std430 1 Offset 16
               OpMemberDecorate %FooBuffer_std430 2 Offset 32
               OpDecorate %_runtimearr_FooBuffer_std430 ArrayStride 48
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %foo Binding 0
               OpDecorate %foo DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
      %int_2 = OpConstant %int 2
%_arr_uint_int_2 = OpTypeArray %uint %int_2
%_Array_std430_uint2 = OpTypeStruct %_arr_uint_int_2
 %Bar_std430 = OpTypeStruct %uint %uint %_Array_std430_uint2
%FooBuffer_std430 = OpTypeStruct %v4float %Bar_std430 %uint
%_ptr_StorageBuffer_FooBuffer_std430 = OpTypePointer StorageBuffer %FooBuffer_std430
%_runtimearr_FooBuffer_std430 = OpTypeRuntimeArray %FooBuffer_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_FooBuffer_std430
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%_ptr_StorageBuffer_Bar_std430 = OpTypePointer StorageBuffer %Bar_std430
        %foo = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
       %main = OpFunction %void None %3
          %4 = OpLabel
         %17 = OpAccessChain %_ptr_StorageBuffer_FooBuffer_std430 %foo %int_0 %int_1
         %23 = OpAccessChain %_ptr_StorageBuffer_Bar_std430 %17 %int_1
         %24 = OpAccessChain %_ptr_StorageBuffer_FooBuffer_std430 %foo %int_0 %int_0
         %25 = OpAccessChain %_ptr_StorageBuffer_Bar_std430 %24 %int_1
         %26 = OpLoad %Bar_std430 %25
         %80 = OpCompositeExtract %uint %26 0
         %81 = OpCompositeExtract %uint %26 1
         %82 = OpCompositeExtract %_Array_std430_uint2 %26 2
         %87 = OpCompositeExtract %_arr_uint_int_2 %82 0
         %88 = OpCompositeExtract %uint %87 0
         %89 = OpCompositeExtract %uint %87 1
        %163 = OpCompositeConstruct %_arr_uint_int_2 %88 %89
        %149 = OpCompositeConstruct %_Array_std430_uint2 %163
        %121 = OpCompositeConstruct %Bar_std430 %80 %81 %149
               OpStore %23 %121
               OpReturn
               OpFunctionEnd
    )";
    ComputeStorageBufferTest(cs_source, false, 80);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, ChainOfAccessChains) {
    TEST_DESCRIPTION("Slang can sometimes generate a single OpAccessChain like GLSL/HLSL");

    // struct Bar {
    //     uint a;
    //     uint d[4];
    // };
    //
    // [[vk::binding(0, 0)]]
    // RWStructuredBuffer<Bar> foo; // 20 byte stride
    //
    // [shader("compute")]
    // void main() {
    //     foo[1].d[3] = 44;
    // }
    char const *cs_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %foo
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_uint_int_4 ArrayStride 4
               OpMemberDecorate %_Array_std430_uint4 0 Offset 0
               OpMemberDecorate %Bar_std430 0 Offset 0
               OpMemberDecorate %Bar_std430 1 Offset 4
               OpDecorate %_runtimearr_Bar_std430 ArrayStride 20
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %foo Binding 0
               OpDecorate %foo DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
      %int_4 = OpConstant %int 4
%_arr_uint_int_4 = OpTypeArray %uint %int_4
%_Array_std430_uint4 = OpTypeStruct %_arr_uint_int_4
 %Bar_std430 = OpTypeStruct %uint %_Array_std430_uint4
%_ptr_StorageBuffer_Bar_std430 = OpTypePointer StorageBuffer %Bar_std430
%_runtimearr_Bar_std430 = OpTypeRuntimeArray %Bar_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_Bar_std430
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%_ptr_StorageBuffer__Array_std430_uint4 = OpTypePointer StorageBuffer %_Array_std430_uint4
%_ptr_StorageBuffer__arr_uint_int_4 = OpTypePointer StorageBuffer %_arr_uint_int_4
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
      %int_3 = OpConstant %int 3
    %uint_44 = OpConstant %uint 44
        %foo = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
       %main = OpFunction %void None %3
          %4 = OpLabel
         %14 = OpAccessChain %_ptr_StorageBuffer_Bar_std430 %foo %int_0 %int_1
         %20 = OpAccessChain %_ptr_StorageBuffer__Array_std430_uint4 %14 %int_1
         %22 = OpAccessChain %_ptr_StorageBuffer__arr_uint_int_4 %20 %int_0
         %24 = OpAccessChain %_ptr_StorageBuffer_uint %22 %int_3
               OpStore %24 %uint_44
               OpReturn
               OpFunctionEnd
    )";
    ComputeStorageBufferTest(cs_source, false, 48);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, Atomics) {
    char const *cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, std430) buffer foo {
            uvec4 a;
            uvec4 b;
            uvec4 c; // offset at 32
        };

        void main() {
            uint x = atomicLoad(c.x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
            atomicStore(c.y, 0u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
            atomicExchange(c.z, x);
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 64);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, AtomicsDescriptorIndex) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, std430) buffer SSBO {
            uvec4 a;
            uvec4 b;
            uvec4 c; // offset at 32
        } ssbo[2];

        void main() {
            uint x = atomicLoad(ssbo[1].c.x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
            atomicStore(ssbo[0].c.y, 0u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
            atomicStore(ssbo[1].c.y, 0u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
            atomicExchange(ssbo[1].c.z, x);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, DescriptorIndexSlang) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // struct Bar {
    //   uint4 a;
    //   uint b[4];
    //   uint c;
    //   uint d;
    // };
    //
    // [[vk::binding(0, 0)]]
    // RWStructuredBuffer<Bar> foo[2]; // stride of 48 bytes
    //
    // [shader("compute")]
    // void main() {
    //   foo[1][1].d = 0;
    // }
    char const *cs_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %foo
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_uint_int_4 ArrayStride 4
               OpMemberDecorate %_Array_std430_uint4 0 Offset 0
               OpMemberDecorate %Bar_std430 0 Offset 0
               OpMemberDecorate %Bar_std430 1 Offset 16
               OpMemberDecorate %Bar_std430 2 Offset 32
               OpMemberDecorate %Bar_std430 3 Offset 36
               OpDecorate %_runtimearr_Bar_std430 ArrayStride 48
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %foo Binding 0
               OpDecorate %foo DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
        %int = OpTypeInt 32 1
      %int_4 = OpConstant %int 4
%_arr_uint_int_4 = OpTypeArray %uint %int_4
%_Array_std430_uint4 = OpTypeStruct %_arr_uint_int_4
 %Bar_std430 = OpTypeStruct %v4uint %_Array_std430_uint4 %uint %uint
%_runtimearr_Bar_std430 = OpTypeRuntimeArray %Bar_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_Bar_std430
      %int_2 = OpConstant %int 2
%_arr_RWStructuredBuffer_int_2 = OpTypeArray %RWStructuredBuffer %int_2
%_ptr_StorageBuffer__arr_RWStructuredBuffer_int_2 = OpTypePointer StorageBuffer %_arr_RWStructuredBuffer_int_2
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_Bar_std430 = OpTypePointer StorageBuffer %Bar_std430
      %int_3 = OpConstant %int 3
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
     %uint_0 = OpConstant %uint 0
        %foo = OpVariable %_ptr_StorageBuffer__arr_RWStructuredBuffer_int_2 StorageBuffer
       %main = OpFunction %void None %3
          %4 = OpLabel
         %19 = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer %foo %int_1
         %23 = OpAccessChain %_ptr_StorageBuffer_Bar_std430 %19 %int_0 %int_1
         %26 = OpAccessChain %_ptr_StorageBuffer_uint %23 %int_3
               OpStore %26 %uint_0
               OpReturn
               OpFunctionEnd
    )";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 96, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBuffer, OpArrayLength) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/vulkan/vulkan/-/issues/4145");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable

        layout(set = 0, binding = 0) buffer SSBO_0 {
            uint a;
        };

        layout(set = 1, binding = 0) buffer SSBO_1 {
            vec4 b;
            uint c[];
        };

        void main() {
            // length() here is NOT an access
            a = c.length();
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    OneOffDescriptorSet descriptor_set0(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    OneOffDescriptorSet descriptor_set1(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set0.layout_, &descriptor_set1.layout_});
    descriptor_set0.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set0.UpdateDescriptorSets();
    descriptor_set1.WriteDescriptorBufferInfo(0, buffer, 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set1.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 1, 1,
                              &descriptor_set1.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}
