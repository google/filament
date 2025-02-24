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
#include "../layers/gpuav/shaders/gpuav_shaders_constants.h"

class NegativeGpuAVDescriptorClassGeneralBuffer : public GpuAVDescriptorClassGeneralBuffer {
  public:
    void ShaderBufferSizeTest(VkDeviceSize buffer_size, VkDeviceSize binding_offset, VkDeviceSize binding_range,
                              VkDescriptorType descriptor_type, const char *fragment_shader,
                              std::vector<const char *> expected_errors, bool shader_objects = false);
};

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, RobustBuffer) {
    TEST_DESCRIPTION("Check buffer oob validation when per pipeline robustness is enabled");

    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    AddRequiredFeature(vkt::Feature::pipelineRobustness);
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    vkt::Buffer uniform_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer storage_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, storage_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char *vertshader = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform foo { uint index[]; } u_index;
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;

        void main() {
            vec4 x;
            if (u_index.index[0] == 0)
                x[0] = u_index.index[1]; // Uniform read OOB
            else
                Data.data[8] = 0xdeadca71; // Storage write OOB
        }
    )glsl";

    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineRobustnessCreateInfo pipeline_robustness_ci = vku::InitStructHelper();
    pipeline_robustness_ci.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS;
    pipeline_robustness_ci.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS;

    CreatePipelineHelper robust_pipe(*this, &pipeline_robustness_ci);
    robust_pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    robust_pipe.gp_ci_.layout = pipeline_layout.handle();
    robust_pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, robust_pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    uint32_t *data = (uint32_t *)uniform_buffer.Memory().Map();
    *data = 0;
    uniform_buffer.Memory().Unmap();
    // normally VUID-vkCmdDraw-uniformBuffers-06935
    m_errorMonitor->SetDesiredWarning("size is 4 bytes, 4 bytes were bound, and the highest out of bounds access was at [19] bytes",
                                      3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
    data = (uint32_t *)uniform_buffer.Memory().Map();
    *data = 1;
    uniform_buffer.Memory().Unmap();
    // normally VUID-vkCmdDraw-storageBuffers-06936
    m_errorMonitor->SetDesiredWarning(
        "size is 16 bytes, 16 bytes were bound, and the highest out of bounds access was at [35] bytes", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, Basic) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);

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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

void NegativeGpuAVDescriptorClassGeneralBuffer::ShaderBufferSizeTest(VkDeviceSize buffer_size, VkDeviceSize binding_offset,
                                                                     VkDeviceSize binding_range, VkDescriptorType descriptor_type,
                                                                     const char *fragment_shader,
                                                                     std::vector<const char *> expected_errors,
                                                                     bool shader_objects) {
    if (shader_objects) {
        AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    }
    RETURN_IF_SKIP(InitGpuAvFramework());

    if (shader_objects) {
        AddRequiredFeature(vkt::Feature::dynamicRendering);
        AddRequiredFeature(vkt::Feature::shaderObject);
    }
    RETURN_IF_SKIP(InitState());
    if (shader_objects) {
        InitDynamicRenderTarget();
    } else {
        InitRenderTarget();
    }
    for (const char *error : expected_errors) {
        m_errorMonitor->SetDesiredError(error);
    }

    OneOffDescriptorSet ds(m_device, {{0, descriptor_type, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds.layout_});

    vkt::Buffer buffer(*m_device, buffer_size,
                       (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                                                                              : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    ds.WriteDescriptorBufferInfo(0, buffer, binding_offset, binding_range, descriptor_type);
    ds.UpdateDescriptorSets();

    vkt::Shader *vso = nullptr;
    vkt::Shader *fso = nullptr;
    if (shader_objects) {
        vso = new vkt::Shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT,
                              GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexDrawPassthroughGlsl), &ds.layout_.handle());
        fso = new vkt::Shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader),
                              &ds.layout_.handle());
    }

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();

    VkFormat color_formats = VK_FORMAT_UNDEFINED;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    const auto depth_format = FindSupportedDepthOnlyFormat(Gpu());

    if (shader_objects) {
        pipeline_rendering_info.colorAttachmentCount = 1;
        pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
        pipeline_rendering_info.depthAttachmentFormat = depth_format;
        pipe.gp_ci_.pNext = &pipeline_rendering_info;
    }

    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    if (shader_objects) {
        m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    } else {
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    }

    if (shader_objects) {
        const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                VK_SHADER_STAGE_FRAGMENT_BIT};
        const VkShaderEXT shaders[] = {vso->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fso->handle()};
        vk::CmdBindShadersEXT(m_command_buffer.handle(), 5u, stages, shaders);
        SetDefaultDynamicStatesAll(m_command_buffer.handle());
    } else {
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    }
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds.set_,
                              0, nullptr);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    if (shader_objects) {
        m_command_buffer.EndRendering();
    } else {
        m_command_buffer.EndRenderPass();
    }
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    vk::DeviceWaitIdle(*m_device);
    m_errorMonitor->VerifyFound();
    DestroyRenderTarget();
    if (shader_objects) {
        delete vso;
        delete fso;
    }
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, UniformBufferTooSmall) {
    TEST_DESCRIPTION("Test that an error is produced when trying to access uniform buffer outside the bound region.");
    char const *fsSource = R"glsl(
        #version 450

        layout(location=0) out vec4 x;
        layout(set=0, binding=0) uniform readonly foo { int x; int y; } bar;
        void main() {
           x = vec4(bar.x, bar.y, 0, 1);
        }
        )glsl";
    std::vector<const char *> expected_errors(gpuav::glsl::kMaxErrorsPerCmd, "VUID-vkCmdDraw-uniformBuffers-06935");
    ShaderBufferSizeTest(4,  // buffer size
                         0,  // binding offset
                         4,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource, expected_errors);
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, UniformBufferTooSmall2) {
    TEST_DESCRIPTION("Buffer is correct size, but only updating half of it.");
    char const *fsSource = R"glsl(
        #version 450

        layout(location=0) out vec4 x;
        layout(set=0, binding=0) uniform readonly foo {
            int x; // Bound
            int y; // Not bound, illegal to use
        } bar;
        void main() {
           x = vec4(bar.x, bar.y, 0, 1);
        }
        )glsl";
    std::vector<const char *> expected_errors(gpuav::glsl::kMaxErrorsPerCmd, "VUID-vkCmdDraw-uniformBuffers-06935");
    ShaderBufferSizeTest(8,  // buffer size
                         0,  // binding offset
                         4,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource, expected_errors);
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, StorageBufferTooSmall) {
    TEST_DESCRIPTION("Test that an error is produced when trying to access storage buffer outside the bound region.");

    char const *fsSource = R"glsl(
        #version 450

        layout(location=0) out vec4 x;
        layout(set=0, binding=0) buffer readonly foo { int x; int y; } bar;
        void main(){
           x = vec4(bar.x, bar.y, 0, 1);
        }
        )glsl";

    std::vector<const char *> expected_errors(gpuav::glsl::kMaxErrorsPerCmd, "VUID-vkCmdDraw-storageBuffers-06936");
    ShaderBufferSizeTest(4,  // buffer size
                         0,  // binding offset
                         4,  // binding range
                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, fsSource, expected_errors);
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, UniformBufferTooSmallArray) {
    TEST_DESCRIPTION(
        "Test that an error is produced when trying to access uniform buffer outside the bound region. Uses array in block "
        "definition.");

    char const *fsSource = R"glsl(
        #version 450

        layout(location=0) out vec4 x;
        layout(set=0, binding=0) uniform readonly foo { int x[17]; } bar;
        void main(){
           int y = 0;
           for (int i = 0; i < 17; i++)
               y += bar.x[i];
           x = vec4(y, 0, 0, 1);
        }
        )glsl";

    std::vector<const char *> expected_errors(gpuav::glsl::kMaxErrorsPerCmd, "VUID-vkCmdDraw-uniformBuffers-06935");
    ShaderBufferSizeTest(64,  // buffer size
                         0,   // binding offset
                         64,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource, expected_errors);
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, UniformBufferTooSmallNestedStruct) {
    TEST_DESCRIPTION(
        "Test that an error is produced when trying to access uniform buffer outside the bound region. Uses nested struct in block "
        "definition.");

    char const *fsSource = R"glsl(
        #version 450

        struct S {
            int x;
            int y;
        };
        layout(location=0) out vec4 x;
        layout(set=0, binding=0) uniform readonly foo { int a; S b; } bar;
        void main(){
           x = vec4(bar.a, bar.b.x, bar.b.y, 1);
        }
        )glsl";

    std::vector<const char *> expected_errors(gpuav::glsl::kMaxErrorsPerCmd, "VUID-vkCmdDraw-uniformBuffers-06935");
    ShaderBufferSizeTest(8,  // buffer size
                         0,  // binding offset
                         8,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource, expected_errors);
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, ObjectUniformBufferTooSmall) {
    TEST_DESCRIPTION("Test that an error is produced when trying to access uniform buffer outside the bound region.");
    char const *fsSource = R"glsl(
        #version 450

        layout(location=0) out vec4 x;
        layout(set=0, binding=0) uniform readonly foo { int x; int y; } bar;
        void main(){
           x = vec4(bar.x, bar.y, 0, 1);
        }
        )glsl";

    std::vector<const char *> expecetd_errors(gpuav::glsl::kMaxErrorsPerCmd, "VUID-vkCmdDraw-None-08612");
    ShaderBufferSizeTest(4,  // buffer size
                         0,  // binding offset
                         4,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource, expecetd_errors, true);
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, GPLWrite) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    uint32_t *data = (uint32_t *)offset_buffer.Memory().Map();
    *data = 8;
    offset_buffer.Memory().Unmap();

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform Foo { uint index[]; };
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; };
        void main() {
            uint index = index[0];
            data[index] = 0xdeadca71;
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, GPLRead) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform Foo { uint index[]; };
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; };
        void main() {
            // Uniform buffer stride rounded up to the alignment of a vec4 (16 bytes)
            // so u_index.index[4] accesses bytes 64, 65, 66, and 67
            data[0] = index[4];
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-uniformBuffers-06935", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, GPLReadWriteIndependentSets) {
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
            uint x = index[0];
            data[x] = 0xdeadca71;
            data[0] = index[4];
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";

    static const char frag_shader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) uniform ufoo { uint index[]; };      // index[1]
        layout(set = 2, binding = 1) uniform samplerBuffer u_buffer;      // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            c_out = vec4(0.0);
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);  // write
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-uniformBuffers-06935", 3);  // read
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, GPLNonInlined) {
    TEST_DESCRIPTION("Make sure GPL works when shader modules are not inlined at pipeline creation time");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, StorageBuffer) {
    TEST_DESCRIPTION("Make sure OOB is still checked when result is from a BufferDeviceAddress");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        #extension GL_ARB_gpu_shader_int64 : enable

        struct Test {
            float a;
        };

        layout(buffer_reference, std430, buffer_reference_align = 16) buffer TestBuffer {
            Test test;
        };

        Test GetTest(uint64_t ptr) {
            return TestBuffer(ptr).test;
        }

        // 12 bytes large
        layout(set = 0, binding = 0) buffer foo {
            TestBuffer data;
            float x;
        } in_buffer;

        void main() {
            in_buffer.x = GetTest(uint64_t(in_buffer.data)).a;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    // too small
    vkt::Buffer in_buffer(*m_device, 8, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    VkDeviceAddress block_ptr = block_buffer.Address();

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.Memory().Map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));
    in_buffer.Memory().Unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, Vector) {
    TEST_DESCRIPTION("index into a vector OOB");

    char const *cs_source = R"glsl(
        #version 450

        // 28 bytes large
        layout(set = 0, binding = 0, std430) buffer foo {
            int a;
            vec3 b; // offset 16
        } in_buffer;

        void main() {
            in_buffer.b.y = 0.0;
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 20, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, Matrix) {
    TEST_DESCRIPTION("index into a matrix OOB");

    char const *cs_source = R"glsl(
        #version 450

        // 32 bytes large
        layout(set = 0, binding = 0, std430) buffer foo {
            int a;
            mat3x2 b; // offset 8
        } in_buffer;

        void main() {
            in_buffer.b[2][1] = 0.0;
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 30, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, Geometry) {
    TEST_DESCRIPTION("Basic Geometry shader test");
    AddRequiredFeature(vkt::Feature::geometryShader);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *gsSource = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices=3) out;
        layout(set = 0, binding = 0, std430) buffer foo {
            vec3 x;
        } in_buffer;

        void main() {
            in_buffer.x.y = 0.0;
            gl_Position = vec4(1);
            EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    // too small
    vkt::Buffer in_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936");
    // On Windows Arm, it re-runs the geometry shader 3 times on same primitive
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-storageBuffers-06936");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO - Not being triggered, shader is instrumented, but doesn't seem tessellation shader is executed
TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, DISABLED_TessellationControl) {
    TEST_DESCRIPTION("Basic TessellationControl shader test");
    AddRequiredFeature(vkt::Feature::tessellationShader);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    static const char shader_source[] = R"glsl(
        #version 460
        layout(vertices=3) out;
        layout(set = 0, binding = 0, std430) buffer foo {
            vec3 x;
        } in_buffer;

        void main() {
            in_buffer.x.y = 0.0;
            gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
            gl_TessLevelInner[0] = 1;
        }
    )glsl";

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj tcs(this, shader_source, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, kTessellationEvalMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineTessellationStateCreateInfo tess_ci = vku::InitStructHelper();
    tess_ci.patchControlPoints = 4u;

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    pipe.tess_ci_ = tess_ci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                           pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    // too small
    vkt::Buffer in_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO - Not being triggered, shader is instrumented, but doesn't seem tessellation shader is executed
TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, DISABLED_TessellationEvaluation) {
    TEST_DESCRIPTION("Basic TessellationEvaluation shader test");
    AddRequiredFeature(vkt::Feature::tessellationShader);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    static const char shader_source[] = R"glsl(
        #version 460
        layout(triangles, equal_spacing, cw) in;
        layout(set = 0, binding = 0, std430) buffer foo {
            vec3 x;
        } in_buffer;
        void main() {
            in_buffer.x.y = 0.0;
            gl_Position = vec4(1);
        }
    )glsl";

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj tcs(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, shader_source, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineTessellationStateCreateInfo tess_ci = vku::InitStructHelper();
    tess_ci.patchControlPoints = 4u;

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    pipe.tess_ci_ = tess_ci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                           pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    // too small
    vkt::Buffer in_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, VertexFragmentMultiEntrypoint) {
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer uniform_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer storage_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-uniformBuffers-06935", 3);  // vertex
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-storageBuffers-06936", 3);  // fragment

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, PartialBoundDescriptorCopy) {
    TEST_DESCRIPTION("Copy the partial bound buffer the descriptor that is used");

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            vec4 a;
            vec4 b[4];
        };

        void main() {
            a = b[3];
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, DeviceGeneratedCommandsCompute) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if ((dgc_props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_COMPUTE_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_COMPUTE_BIT is not supported.";
    }

    VkIndirectCommandsLayoutTokenEXT token;
    token = vku::InitStructHelper();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT;
    token.offset = 0;

    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    command_layout_ci.pipelineLayout = VK_NULL_HANDLE;
    command_layout_ci.tokenCount = 1;
    command_layout_ci.pTokens = &token;
    vkt::IndirectCommandsLayout command_layout(*m_device, command_layout_ci);

    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer ssbo {
            uint x[];
        };
        void main() {
            x[48] = 0;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.CreateComputePipeline();

    vkt::Buffer ssbo_buffer(*m_device, 8, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, ssbo_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    VkGeneratedCommandsPipelineInfoEXT pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = pipe.Handle();

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, kHostVisibleMemProps, &allocate_flag_info);

    VkDeviceSize pre_process_size = 0;
    {
        VkGeneratedCommandsMemoryRequirementsInfoEXT dgc_mem_reqs = vku::InitStructHelper(&pipeline_info);
        dgc_mem_reqs.indirectCommandsLayout = command_layout.handle();
        dgc_mem_reqs.indirectExecutionSet = VK_NULL_HANDLE;
        dgc_mem_reqs.maxSequenceCount = 1;
        VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper();
        vk::GetGeneratedCommandsMemoryRequirementsEXT(device(), &dgc_mem_reqs, &mem_reqs2);
        pre_process_size = mem_reqs2.memoryRequirements.size;
    }

    VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_ci.size = pre_process_size;
    vkt::Buffer pre_process_buffer(*m_device, buffer_ci, 0, &allocate_flag_info);

    VkDispatchIndirectCommand *block_buffer_ptr = (VkDispatchIndirectCommand *)block_buffer.Memory().Map();
    block_buffer_ptr->x = 1;
    block_buffer_ptr->y = 1;
    block_buffer_ptr->z = 1;
    block_buffer.Memory().Unmap();

    VkGeneratedCommandsInfoEXT generated_commands_info = vku::InitStructHelper(&pipeline_info);
    generated_commands_info.shaderStages = VK_SHADER_STAGE_COMPUTE_BIT;
    generated_commands_info.indirectExecutionSet = VK_NULL_HANDLE;
    generated_commands_info.indirectCommandsLayout = command_layout.handle();
    generated_commands_info.indirectAddressSize = sizeof(VkDispatchIndirectCommand);
    generated_commands_info.indirectAddress = block_buffer.Address();
    generated_commands_info.preprocessAddress = pre_process_buffer.Address();
    generated_commands_info.preprocessSize = pre_process_size;
    generated_commands_info.sequenceCountAddress = 0;
    generated_commands_info.maxSequenceCount = 1;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdExecuteGeneratedCommandsEXT(m_command_buffer.handle(), false, &generated_commands_info);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteGeneratedCommandsEXT-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, SpecConstant) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(constant_id = 0) const uint index = 2;

        layout(set = 0, binding = 0) buffer foo {
            uint a;
            uint b[4];
        };

        void main() {
            a = b[index];
        }
    )glsl";

    const uint32_t value = 25;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo spec_info = {1, &entry, sizeof(uint32_t), &value};

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                                             &spec_info);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 20, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, PartialBoundDescriptorSSBO) {
    TEST_DESCRIPTION("Only bound part of a SSBO (with update after bind), and use the part that is not valid");
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
            a = c;
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, VectorArray) {
    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            uvec4 a[8]; // stride 16
        };
        void main() {
            a[3].y = 44; // write at byte[52:56]
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 48, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, ArrayCopyGLSL) {
    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, std430) buffer foo {
            uvec4 a;
            uint padding;
            uint b[4]; // b[3] is OOB
            uint c; // offset 36
        };

        void main() {
            uint d[4] = {4, 5, 6, 7};
            b = d;
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 32, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, ArrayCopySlang) {
    TEST_DESCRIPTION("Note that in slang and array copy is really a struct copy");
    // struct Bar {
    //   uint4 a;
    //   uint pad;
    //   uint b[4]; // b[3] is OOB
    //   uint c; // offset 36
    // };
    //
    // [[vk::binding(0, 0)]]
    // RWStructuredBuffer<Bar> foo;
    //
    // [shader("compute")]
    // void main() {
    //   uint d[4] = {4, 5, 6, 7};
    //   foo[0].b = d;
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
               OpMemberDecorate %Bar_std430 2 Offset 20
               OpMemberDecorate %Bar_std430 3 Offset 36
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
 %Bar_std430 = OpTypeStruct %v4uint %uint %_Array_std430_uint4 %uint
%_ptr_StorageBuffer_Bar_std430 = OpTypePointer StorageBuffer %Bar_std430
%_runtimearr_Bar_std430 = OpTypeRuntimeArray %Bar_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_Bar_std430
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
      %int_2 = OpConstant %int 2
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
         %21 = OpAccessChain %_ptr_StorageBuffer__Array_std430_uint4 %14 %int_2
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
    ComputeStorageBufferTest(cs_source, false, 32, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, ArrayCopyTwoBindingsGLSL) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, std430) buffer foo1 {
            uvec4 a;
            uint padding;
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
    // not enough bound for either
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, 32, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(1, in_buffer.handle(), 64, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936", 2);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, ArrayCopyTwoBindingsSlang) {
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
    pipe.descriptor_set_->WriteDescriptorBufferInfo(1, in_buffer.handle(), 64, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, StructCopyGLSL) {
    char const *cs_source = R"glsl(
        #version 450

        struct Bar {
            uint x;
            uint y;
            uint z[2];
        };

        layout(set = 0, binding = 0, std430) buffer foo {
            uvec4 a;
            uint padding;
            Bar b; // size 16 at offset 20
            uint c;
        };

        void main() {
            Bar new_bar;
            b = new_bar;
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 32, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, StructCopyGLSL2) {
    char const *cs_source = R"glsl(
        #version 450

        struct Bar {
            vec4 x;
        };

        layout(set = 0, binding = 0, std430) buffer foo {
            uvec4 a;
            uint padding;
            Bar b; // size 16 at offset 20
            uint c;
        };

        void main() {
            Bar new_bar;
            b = new_bar;
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 32, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, StructCopyGLSL3) {
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
            Bar b; // size 32 at offset 16
            uint c;
        };

        void main() {
            Bar new_bar;
            b = new_bar;
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 32, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, StructCopySlang) {
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
    ComputeStorageBufferTest(cs_source, false, 72, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, ChainOfAccessChains) {
    TEST_DESCRIPTION("Slang can sometimes generate a single OpAccessChain like GLSL/HLSL");

    // struct Bar {
    //     uint a;
    //     uint d[4]; // this really ends up being a 1 element struct with the array in it
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
    ComputeStorageBufferTest(cs_source, false, 32, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, AtomicStore) {
    char const *cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, std430) buffer foo {
            uvec4 a;
            uint b;
        };

        void main() {
            atomicStore(b, 0u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 16, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, AtomicLoad) {
    char const *cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, std430) buffer foo {
            uvec4 a;
            uvec4 b;
        };

        void main() {
            a.x = atomicLoad(b.x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 16, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, AtomicExchange) {
    char const *cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, std430) buffer foo {
            uvec4 a;
            uint b;
        };

        void main() {
            atomicExchange(b, a.x);
        }
    )glsl";
    ComputeStorageBufferTest(cs_source, true, 16, "VUID-vkCmdDispatch-storageBuffers-06936");
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, AtomicsDescriptorIndex) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, std430) buffer SSBO {
            uvec4 a;
            uint b;
        } ssbo[2];

        void main() {
            atomicStore(ssbo[0].b, 0u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed); // valid
            atomicStore(ssbo[1].b, 0u, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed); // invalid
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, 64, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 64, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorClassGeneralBuffer, DescriptorIndexSlang) {
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
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, 80, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-storageBuffers-06936");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
