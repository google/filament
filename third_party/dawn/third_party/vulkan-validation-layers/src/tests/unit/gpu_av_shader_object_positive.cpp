/*
 * Copyright (c) 2023-2024 Nintendo
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"
#include "../framework/shader_object_helper.h"
#include "../framework/shader_templates.h"
#include "../framework/pipeline_helper.h"

class PositiveGpuAVShaderObject : public GpuAVTest {
  public:
    void InitBasicShaderObject() {
        SetTargetApiVersion(VK_API_VERSION_1_2);
        AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
        AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        AddRequiredFeature(vkt::Feature::dynamicRendering);
        AddRequiredFeature(vkt::Feature::shaderObject);
    }
};

TEST_F(PositiveGpuAVShaderObject, SelectInstrumentedShaders) {
    TEST_DESCRIPTION("GPU validation: Validate selection of which shaders get instrumented for GPU-AV");
    InitBasicShaderObject();

    AddRequiredFeature(vkt::Feature::robustBufferAccess);
    const VkBool32 value = true;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                       &value};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                               &setting};
    RETURN_IF_SKIP(InitGpuAvFramework(&layer_settings_create_info));

    // Robust buffer access will be on by default
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    InitState(nullptr, nullptr, pool_flags);
    InitDynamicRenderTarget();

    OneOffDescriptorSet vert_descriptor_set(m_device,
                                            {
                                                {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                            });
    vkt::PipelineLayout pipeline_layout(*m_device, {&vert_descriptor_set.layout_});

    static const char vert_src[] = R"glsl(
        #version 460
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
            Data.data[4] = 0xdeadca71;
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vert_src);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    VkDescriptorSetLayout descriptor_set_layouts[] = {vert_descriptor_set.layout_.handle()};

    VkShaderCreateInfoEXT vert_create_info = ShaderCreateInfo(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, 1, descriptor_set_layouts);
    VkShaderCreateInfoEXT frag_create_info = ShaderCreateInfo(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 1, descriptor_set_layouts);

    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    vert_create_info.pNext = &features;
    frag_create_info.pNext = &features;

    const vkt::Shader vertShader(*m_device, vert_create_info);
    const vkt::Shader fragShader(*m_device, frag_create_info);

    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vert_descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vert_descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vertShader, fragShader);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &vert_descriptor_set.set_, 0u, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    // Should get a warning since shader was instrumented
    m_errorMonitor->SetDesiredWarning("VUID-vkCmdDraw-None-08613", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    vert_create_info.pNext = nullptr;
    const vkt::Shader vertShader2(*m_device, vert_create_info);
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vertShader2, fragShader);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0u, 1u,
                              &vert_descriptor_set.set_, 0u, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 4, 1, 0, 0);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    // Should not get a warning since shader was not instrumented
    m_errorMonitor->ExpectSuccess(kWarningBit | kErrorBit);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVShaderObject, RestoreUserPushConstants) {
    TEST_DESCRIPTION("Test that user supplied push constants are correctly restored. One graphics pipeline, indirect draw.");
    InitBasicShaderObject();
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    vkt::Buffer indirect_draw_parameters_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                kHostVisibleMemProps);
    auto &indirect_draw_parameters = *static_cast<VkDrawIndirectCommand *>(indirect_draw_parameters_buffer.Memory().Map());
    indirect_draw_parameters.vertexCount = 3;
    indirect_draw_parameters.instanceCount = 1;
    indirect_draw_parameters.firstVertex = 0;
    indirect_draw_parameters.firstInstance = 0;

    indirect_draw_parameters_buffer.Memory().Unmap();

    constexpr int32_t int_count = 16;
    vkt::Buffer storage_buffer(*m_device, int_count * sizeof(int32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    // Use different push constant ranges for vertex and fragment shader.
    // The underlying storage buffer is the same.
    // Vertex shader will fill the first 8 integers, fragment shader the other 8
    struct PushConstants {
        // Vertex shader
        VkDeviceAddress storage_buffer_ptr_1;
        int32_t integers_1[int_count / 2];
        // Fragment shader
        VkDeviceAddress storage_buffer_ptr_2;
        int32_t integers_2[int_count / 2];
    } push_constants;

    push_constants.storage_buffer_ptr_1 = storage_buffer.Address();
    push_constants.storage_buffer_ptr_2 = storage_buffer.Address() + sizeof(int32_t) * (int_count / 2);
    for (int32_t i = 0; i < int_count / 2; ++i) {
        push_constants.integers_1[i] = i;
        push_constants.integers_2[i] = (int_count / 2) + i;
    }

    constexpr uint32_t shader_pcr_byte_size = uint32_t(sizeof(VkDeviceAddress)) + uint32_t(sizeof(int32_t)) * (int_count / 2);
    std::array<VkPushConstantRange, 2> push_constant_ranges = {{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, shader_pcr_byte_size},
        {VK_SHADER_STAGE_FRAGMENT_BIT, shader_pcr_byte_size, shader_pcr_byte_size},
    }};
    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.pushConstantRangeCount = size32(push_constant_ranges);
    plci.pPushConstantRanges = push_constant_ranges.data();
    vkt::PipelineLayout pipeline_layout(*m_device, plci);

    char const *vs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            vec2 vertices[3];

            void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
              gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);

              for (int i = 0; i < 8; ++i) {
                  pc.ptr.out_array[i] = pc.in_array[i];
              }
            }
        )glsl";

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source);

    char const *fs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                layout(offset = 40) MyPtrType ptr;
                int in_array[8];
            } pc;

            layout(location = 0) out vec4 uFragColor;

            void main() {
                for (int i = 0; i < 8; ++i) {
                      pc.ptr.out_array[i] = pc.in_array[i];
                }
            }
        )glsl";
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source);

    VkShaderCreateInfoEXT vs_ci = ShaderCreateInfo(vs_spv, VK_SHADER_STAGE_VERTEX_BIT, 0, nullptr,
                                                   static_cast<uint32_t>(push_constant_ranges.size()), push_constant_ranges.data());
    VkShaderCreateInfoEXT fs_ci = ShaderCreateInfo(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 0, nullptr,
                                                   static_cast<uint32_t>(push_constant_ranges.size()), push_constant_ranges.data());

    const vkt::Shader vs(*m_device, vs_ci);
    const vkt::Shader fs(*m_device, fs_ci);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindVertFragShader(vs, fs);

    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, shader_pcr_byte_size,
                         &push_constants);
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, shader_pcr_byte_size,
                         shader_pcr_byte_size, &push_constants.storage_buffer_ptr_2);
    // Make sure pushing the same push constants twice does not break internal management
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, shader_pcr_byte_size,
                         &push_constants);
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, shader_pcr_byte_size,
                         shader_pcr_byte_size, &push_constants.storage_buffer_ptr_2);
    // Vertex shader will write 8 values to storage buffer, fragment shader another 8
    vk::CmdDrawIndirect(m_command_buffer.handle(), indirect_draw_parameters_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRendering();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    auto storage_buffer_ptr = static_cast<int32_t *>(storage_buffer.Memory().Map());
    for (int32_t i = 0; i < int_count; ++i) {
        ASSERT_EQ(storage_buffer_ptr[i], i);
    }
    storage_buffer.Memory().Unmap();
}

TEST_F(PositiveGpuAVShaderObject, RestoreUserPushConstants2) {
    TEST_DESCRIPTION(
        "Test that user supplied push constants are correctly restored. One graphics pipeline, one compute pipeline, indirect draw "
        "and dispatch.");
    InitBasicShaderObject();
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    constexpr int32_t int_count = 8;

    struct PushConstants {
        VkDeviceAddress storage_buffer;
        int32_t integers[int_count];
    };

    // Graphics pipeline
    // ---

    char const *vs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            vec2 vertices[3];

            void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
              gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);

              for (int i = 0; i < 4; ++i) {
                  pc.ptr.out_array[i] = pc.in_array[i];
              }
            }
        )glsl";

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source);

    char const *fs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            layout(location = 0) out vec4 uFragColor;

            void main() {
                for (int i = 4; i < 8; ++i) {
                      pc.ptr.out_array[i] = pc.in_array[i];
                }
            }
        )glsl";

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source);

    VkPushConstantRange graphics_push_constant_ranges = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                         sizeof(PushConstants)};
    VkPipelineLayoutCreateInfo graphics_plci = vku::InitStructHelper();
    graphics_plci.pushConstantRangeCount = 1;
    graphics_plci.pPushConstantRanges = &graphics_push_constant_ranges;
    vkt::PipelineLayout graphics_pipeline_layout(*m_device, graphics_plci);

    vkt::Buffer graphics_storage_buffer(*m_device, int_count * sizeof(int32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                        vkt::device_address);

    PushConstants graphics_push_constants;
    graphics_push_constants.storage_buffer = graphics_storage_buffer.Address();
    for (int32_t i = 0; i < int_count; ++i) {
        graphics_push_constants.integers[i] = i;
    }

    vkt::Buffer indirect_draw_parameters_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                kHostVisibleMemProps);
    auto &indirect_draw_parameters = *static_cast<VkDrawIndirectCommand *>(indirect_draw_parameters_buffer.Memory().Map());
    indirect_draw_parameters.vertexCount = 3;
    indirect_draw_parameters.instanceCount = 1;
    indirect_draw_parameters.firstVertex = 0;
    indirect_draw_parameters.firstInstance = 0;
    indirect_draw_parameters_buffer.Memory().Unmap();

    VkShaderCreateInfoEXT vs_ci =
        ShaderCreateInfo(vs_spv, VK_SHADER_STAGE_VERTEX_BIT, 0, nullptr, 1, &graphics_push_constant_ranges);
    VkShaderCreateInfoEXT fs_ci =
        ShaderCreateInfo(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 0, nullptr, 1, &graphics_push_constant_ranges);

    const vkt::Shader vs(*m_device, vs_ci);
    const vkt::Shader fs(*m_device, fs_ci);

    // Compute pipeline
    // ---

    char const *cs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

            void main() {
              for (int i = 0; i < 8; ++i) {
                  pc.ptr.out_array[i] = pc.in_array[i];
              }
            }
        )glsl";

    const auto cs_spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source);

    VkPushConstantRange compute_push_constant_ranges = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants)};
    VkPipelineLayoutCreateInfo compute_plci = vku::InitStructHelper();
    compute_plci.pushConstantRangeCount = 1;
    compute_plci.pPushConstantRanges = &compute_push_constant_ranges;
    vkt::PipelineLayout compute_pipeline_layout(*m_device, compute_plci);

    vkt::Buffer compute_storage_buffer(*m_device, int_count * sizeof(int32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                       vkt::device_address);

    PushConstants compute_push_constants;
    compute_push_constants.storage_buffer = compute_storage_buffer.Address();
    for (int32_t i = 0; i < int_count; ++i) {
        compute_push_constants.integers[i] = int_count + i;
    }

    VkShaderCreateInfoEXT cs_ci =
        ShaderCreateInfo(cs_spv, VK_SHADER_STAGE_COMPUTE_BIT, 0, nullptr, 1, &compute_push_constant_ranges);

    const vkt::Shader cs(*m_device, cs_ci);

    vkt::Buffer indirect_dispatch_parameters_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                    kHostVisibleMemProps);
    auto &indirect_dispatch_parameters =
        *static_cast<VkDispatchIndirectCommand *>(indirect_dispatch_parameters_buffer.Memory().Map());
    indirect_dispatch_parameters.x = 1;
    indirect_dispatch_parameters.y = 1;
    indirect_dispatch_parameters.z = 1;
    indirect_dispatch_parameters_buffer.Memory().Unmap();

    // Submit commands
    // ---

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();

    m_command_buffer.BindVertFragShader(vs, fs);
    m_command_buffer.BindCompShader(cs);

    vk::CmdPushConstants(m_command_buffer.handle(), graphics_pipeline_layout.handle(),
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(graphics_push_constants),
                         &graphics_push_constants);

    // Vertex shader will write 4 values to graphics storage buffer, fragment shader another 4
    vk::CmdDrawIndirect(m_command_buffer.handle(), indirect_draw_parameters_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRendering();

    vk::CmdPushConstants(m_command_buffer.handle(), compute_pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0,
                         sizeof(compute_push_constants), &compute_push_constants);
    // Compute shaders will write 8 values to compute storage buffer
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_dispatch_parameters_buffer.handle(), 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    auto compute_storage_buffer_ptr = static_cast<int32_t *>(compute_storage_buffer.Memory().Map());
    for (int32_t i = 0; i < int_count; ++i) {
        ASSERT_EQ(compute_storage_buffer_ptr[i], int_count + i);
    }
    compute_storage_buffer.Memory().Unmap();

    auto graphics_storage_buffer_ptr = static_cast<int32_t *>(graphics_storage_buffer.Memory().Map());
    for (int32_t i = 0; i < int_count; ++i) {
        ASSERT_EQ(graphics_storage_buffer_ptr[i], i);
    }
    graphics_storage_buffer.Memory().Unmap();
}

TEST_F(PositiveGpuAVShaderObject, DispatchShaderObjectAndPipeline) {
    TEST_DESCRIPTION("GPU validation: Validate selection of which shaders get instrumented for GPU-AV");
    InitBasicShaderObject();

    AddRequiredFeature(vkt::Feature::robustBufferAccess);
    const VkBool32 value = true;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                       &value};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                               &setting};
    RETURN_IF_SKIP(InitGpuAvFramework(&layer_settings_create_info));
    InitState();

    static const char comp_src[] = R"glsl(
        #version 450
        layout(local_size_x=16, local_size_x=1, local_size_x=1) in;

        void main() {
        }
    )glsl";

    const auto comp_spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src);
    VkShaderCreateInfoEXT comp_create_info = ShaderCreateInfo(comp_spv, VK_SHADER_STAGE_COMPUTE_BIT);

    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    comp_create_info.pNext = &features;

    const vkt::Shader compShader(*m_device, comp_create_info);

    CreateComputePipelineHelper compute_pipe(*this);
    compute_pipe.cs_ = std::make_unique<VkShaderObj>(this, comp_src, VK_SHADER_STAGE_COMPUTE_BIT);
    compute_pipe.CreateComputePipeline();

    vkt::Buffer indirect_dispatch_parameters_buffer(*m_device, sizeof(VkDispatchIndirectCommand),
                                                    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    auto &indirect_dispatch_parameters =
        *static_cast<VkDispatchIndirectCommand *>(indirect_dispatch_parameters_buffer.Memory().Map());
    indirect_dispatch_parameters.x = 1u;
    indirect_dispatch_parameters.y = 1u;
    indirect_dispatch_parameters.z = 1u;
    indirect_dispatch_parameters_buffer.Memory().Unmap();

    m_command_buffer.Begin();
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindCompShader(compShader);
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_dispatch_parameters_buffer.handle(), 0u);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe.Handle());
    vk::CmdDispatchIndirect(m_command_buffer.handle(), indirect_dispatch_parameters_buffer.handle(), 0u);

    m_command_buffer.End();
}
