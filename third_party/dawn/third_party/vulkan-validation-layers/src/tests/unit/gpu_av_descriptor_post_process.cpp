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

class NegativeGpuAVDescriptorPostProcess : public GpuAVTest {};

TEST_F(NegativeGpuAVDescriptorPostProcess, UpdateAfterBindImageViewTypeMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced when an image view type does not match the dimensionality declared in the shader");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(s, vec3(0));
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL,
                                                           nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler);
    descriptor_set.UpdateDescriptorSets();

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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewType-07752");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, PostProcesingOnly) {
    TEST_DESCRIPTION("Test only using Post Processing and turning off other shader instrumentation checks");
    const VkBool32 value_false = false;
    const VkLayerSettingEXT layer_setting = {OBJECT_LAYER_NAME, "gpuav_descriptor_checks", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                             &value_false};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                               &layer_setting};

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework(&layer_settings_create_info));
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler3D s;
        layout(location=0) out vec4 color;
        void main() {
           color = texture(s, vec3(0));
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorIndexingSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL,
                                                           nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler);
    descriptor_set.UpdateDescriptorSets();

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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewType-07752");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, ImageTypeMismatch) {
    TEST_DESCRIPTION("Detect that the index image is 3D but VkImage is only 2D");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform sampler3D s[];
        layout(set = 0, binding = 1) buffer StorageBuffer {
            uint data; // will be zero
        };
        layout(location=0) out vec4 color;
        void main() {
            color = texture(s[data], vec3(0));
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Buffer buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    *buffer_ptr = 0;
    buffer.Memory().Unmap();

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewType-07752");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, MixingProtectedResources) {
    TEST_DESCRIPTION("Have protected resources that is found in the GPU-AV post processing");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(protected_memory_properties);
    if (protected_memory_properties.protectedNoFault) {
        GTEST_SKIP() << "protectedNoFault is supported";
    }

    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    vkt::Image image_protected(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements mem_reqs_image_protected;
    vk::GetImageMemoryRequirements(device(), image_protected.handle(), &mem_reqs_image_protected);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = mem_reqs_image_protected.size;
    if (!m_device->Physical().SetMemoryType(mem_reqs_image_protected.memoryTypeBits, &alloc_info,
                                            VK_MEMORY_PROPERTY_PROTECTED_BIT)) {
        GTEST_SKIP() << "Memory type not found";
    }
    vkt::DeviceMemory memory_image_protected(*m_device, alloc_info);
    vk::BindImageMemory(device(), image_protected.handle(), memory_image_protected.handle(), 0);
    image_protected.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view_protected = image_protected.CreateView();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 1;  // access protected
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

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, sizeof(uint32_t));
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, image_view_protected, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Input {
            uint index;
        } in_buffer;

        // [0] non-protected
        // [1] protected
        layout(set = 0, binding = 1) uniform sampler2D tex[];

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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-commandBuffer-02707");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, InvalidAtomicStorageOperation) {
    TEST_DESCRIPTION(
        "If storage view use atomic operation, the view's format MUST support VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT or "
        "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT ");

    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderImageFloat32Atomics);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT;
    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;  // The format doesn't support VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT to
                                                       // cause DesiredFailure. VK_FORMAT_R32_UINT is right format.
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, image_format, usage);

    if (ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
        GTEST_SKIP() << "Cannot make VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT not supported.";
    }

    VkFormat buffer_view_format =
        VK_FORMAT_R8_UNORM;  // The format doesn't support VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT to
                             // cause DesiredFailure. VK_FORMAT_R32_UINT is right format.
    if (BufferFormatAndFeaturesSupported(Gpu(), buffer_view_format, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
        GTEST_SKIP() << "Cannot make VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT not supported.";
    }
    m_errorMonitor->SetUnexpectedError("VUID-VkBufferViewCreateInfo-format-08779");
    InitRenderTarget();

    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = buffer.handle();
    bvci.format = buffer_view_format;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, bvci);

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=3, r32f) uniform image2D si0;
        layout(set=0, binding=2, r32f) uniform image2D si1[2];
        layout(set = 0, binding = 1, r32f) uniform imageBuffer stb2;
        layout(set = 0, binding = 0, r32f) uniform imageBuffer stb3[2];
        layout(location=0) out vec4 color;
        void main() {
              color += imageAtomicExchange(si1[0], ivec2(0), 1);
              color += imageAtomicExchange(si1[1], ivec2(0), 1);
              color += imageAtomicExchange(stb3[0], 0, 1);
              color += imageAtomicExchange(stb3[1], 0, 1);
        }
    )glsl";

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.CreateGraphicsPipeline();

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(3, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL, 1);
    g_pipe.descriptor_set_->WriteDescriptorBufferView(1, buffer_view.handle());
    g_pipe.descriptor_set_->WriteDescriptorBufferView(0, buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0);
    g_pipe.descriptor_set_->WriteDescriptorBufferView(0, buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-02691");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07888");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, UnnormalizedCoordinatesInBoundsAccess) {
    TEST_DESCRIPTION("If a samper is unnormalizedCoordinates, but using OpInBoundsAccessChain");

    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    //#version 450
    // layout (set = 0, binding = 0) uniform sampler2D tex[2];
    // layout(location=0) out vec4 color;
    //
    // void main() {
    //    color = textureLodOffset(tex[1], vec2(0), 0, ivec2(0));
    //}
    // but with OpInBoundsAccessChain instead of normal generated OpAccessChain
    const char *fsSource = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpExtension "SPV_KHR_physical_storage_buffer"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint Fragment %main "main" %color
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %color "color"
               OpName %tex "tex"
               OpDecorate %color Location 0
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_11_uint_2 = OpTypeArray %11 %uint_2
%_ptr_UniformConstant__arr_11_uint_2 = OpTypePointer UniformConstant %_arr_11_uint_2
        %tex = OpVariable %_ptr_UniformConstant__arr_11_uint_2 UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %24 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %6
         %25 = OpLabel
         %26 = OpInBoundsAccessChain %_ptr_UniformConstant_11 %tex %int_1
         %27 = OpLoad %11 %26
         %28 = OpImageSampleExplicitLod %v4float %27 %21 Lod|ConstOffset %float_0 %24
               OpStore %color %28
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    g_pipe.CreateGraphicsPipeline();

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView view_pass = image.CreateView();

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08611");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, UnnormalizedCoordinatesCopyObject) {
    TEST_DESCRIPTION("If a samper is unnormalizedCoordinates, but using OpCopyObject");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    // layout (set = 0, binding = 0) uniform sampler2D tex[2];
    // void main() {
    //     vec4 x = textureLodOffset(tex[1], vec2(0), 0, ivec2(0));
    // }
    const char *fsSource = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%ptr_v4float = OpTypePointer Function %v4float
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
       %array = OpTypeArray %11 %uint_2
%ptr_uc_array = OpTypePointer UniformConstant %array
        %tex = OpVariable %ptr_uc_array UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %ptr_uc = OpTypePointer UniformConstant %11
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %24 = OpConstantComposite %v2float %float_0 %float_0
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %27 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %3
          %5 = OpLabel
          %x = OpVariable %ptr_v4float Function
   %var_copy = OpCopyObject %ptr_v4float %x
         %20 = OpAccessChain %ptr_uc %tex %int_1
    %ac_copy = OpCopyObject %ptr_uc %20
         %21 = OpLoad %11 %ac_copy
  %load_copy = OpCopyObject %11 %21
         %28 = OpImageSampleExplicitLod %v4float %load_copy %24 Lod|ConstOffset %float_0 %27
 %image_copy = OpCopyObject %v4float %28
               OpStore %var_copy %image_copy
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    g_pipe.CreateGraphicsPipeline();

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView view_pass = image.CreateView();

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_pass, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_pass, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08611");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8922
TEST_F(NegativeGpuAVDescriptorPostProcess, DISABLED_UnnormalizedCoordinatesSeparateSamplerSharedSamplerRuntime) {
    TEST_DESCRIPTION("Doesn't use COMBINED_IMAGE_SAMPLER, but multiple OpLoad share Sampler OpVariable");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // There are 2 OpLoad/OpAccessChain that point the same OpVariable
    const char fsSource[] = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        // VK_DESCRIPTOR_TYPE_SAMPLER
        layout(set = 0, binding = 0) uniform sampler s1;
        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
        layout(set = 0, binding = 1) uniform texture2D si_good[];
        layout(set = 0, binding = 2) uniform texture3D si_bad[]; // 3D image view
        layout(set = 0, binding = 3) uniform UBO { uint bad_index; };

        layout(location=0) out vec4 color;
        void main() {
            vec4 x = texture(sampler2D(si_good[bad_index], s1), vec2(0));
            vec4 y = texture(sampler3D(si_bad[bad_index], s1), vec3(0));
            color = vec4(x + y);
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.gp_ci_.layout = pipeline_layout.handle();
    g_pipe.CreateGraphicsPipeline();

    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view_3d = image_3d.CreateView(VK_IMAGE_VIEW_TYPE_3D);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);

    vkt::Buffer uniform_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *uniform_buffer_ptr = (uint32_t *)uniform_buffer.Memory().Map();
    uniform_buffer_ptr[0] = 1;  // bad_index
    uniform_buffer.Memory().Unmap();

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorImageInfo(2, image_view_3d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(2, image_view_3d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(3, uniform_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08609");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08610");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8922
TEST_F(NegativeGpuAVDescriptorPostProcess, DISABLED_UnnormalizedCoordinatesSeparateSamplerSharedSamplerSpecConstant) {
    TEST_DESCRIPTION("Doesn't use COMBINED_IMAGE_SAMPLER, but multiple OpLoad share Sampler OpVariable");

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // There are 2 OpLoad/OpAccessChain that point the same OpVariable
    const char fsSource[] = R"glsl(
        #version 450
        // VK_DESCRIPTOR_TYPE_SAMPLER
        layout(set = 0, binding = 0) uniform sampler s1;
        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
        layout(set = 0, binding = 1) uniform texture2D si_good[2];
        layout(set = 0, binding = 2) uniform texture3D si_bad[2]; // 3D image view
        layout(constant_id = 0) const uint bad_index = 1;

        layout(location=0) out vec4 color;
        void main() {
            vec4 x = texture(sampler2D(si_good[bad_index], s1), vec2(0));
            vec4 y = texture(sampler3D(si_bad[bad_index], s1), vec3(0));
            color = vec4(x + y);
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.gp_ci_.layout = pipeline_layout.handle();
    g_pipe.CreateGraphicsPipeline();

    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view_3d = image_3d.CreateView(VK_IMAGE_VIEW_TYPE_3D);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorImageInfo(2, image_view_3d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(2, image_view_3d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08609");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08610");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, YcbcrDrawFetchNonArrayPartiallyBound) {
    TEST_DESCRIPTION("Do OpImageFetch on a Ycbcr COMBINED_IMAGE_SAMPLER.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    const VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;

    auto ci = vku::InitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = format;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {256, 256, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), ci, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    vkt::Image image(*m_device, ci, vkt::set_layout);
    vkt::SamplerYcbcrConversion conversion(*m_device, format);
    auto conversion_info = conversion.ConversionInfo();
    vkt::ImageView view = image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT, &conversion_info);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.pNext = &conversion_info;
    vkt::Sampler sampler(*m_device, sampler_ci);

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 1;
    buffer.Memory().Unmap();

    OneOffDescriptorIndexingSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, &sampler.handle(),
                       VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                  });
    if (!descriptor_set.set_) {
        GTEST_SKIP() << "Can't allocate descriptor with immutable sampler";
    }
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, view, sampler);
    descriptor_set.UpdateDescriptorSets();

    const char vsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform sampler2D ycbcr;
        void main() {
            gl_Position = texelFetch(ycbcr, ivec2(0), 0);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06550");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, NonMultisampleMismatchWithPipelinePartiallyBound) {
    TEST_DESCRIPTION("Shader uses non-Multisample, but image view is Multisample.");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    auto image_create_info = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image good_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView good_image_view = good_image.CreateView();

    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image bad_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView bad_image_view = bad_image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 1;
    buffer.Memory().Unmap();

    OneOffDescriptorIndexingSet descriptor_set(m_device,
                                               {
                                                   {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                   {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                                               });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, good_image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, bad_image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        // mySampler[0] is good
        // mySampler[1] is bad
        layout(set=0, binding=0) uniform sampler2D mySampler[2];
        layout(set=0, binding=1) buffer SSBO {
            int index;
            vec4 out_value;
        };
        void main() {
           out_value = texelFetch(mySampler[index], ivec2(0), 0);
        }
    )glsl";

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

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-samples-08725");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO - Pipeline hold active_slots for all stages, but ShaderObject does it per-stage
TEST_F(NegativeGpuAVDescriptorPostProcess, DISABLED_NonMultisampleMismatchWithShaderObject) {
    TEST_DESCRIPTION("Shader uses non-Multisample, but image view is Multisample.");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    auto image_create_info = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image good_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView good_image_view = good_image.CreateView();

    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image bad_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView bad_image_view = bad_image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 1;
    buffer.Memory().Unmap();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, good_image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, bad_image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        // mySampler[0] is good
        // mySampler[1] is bad
        layout(set=0, binding=0) uniform sampler2D mySampler[2];
        layout(set=0, binding=1) buffer SSBO {
            int index;
            vec4 out_value;
        };
        void main() {
           out_value = texelFetch(mySampler[index], ivec2(0), 0);
        }
    )glsl";

    const vkt::Shader cs(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source),
                         &descriptor_set.layout_.handle());

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    VkShaderStageFlagBits shader_stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1, shader_stages, &cs.handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-samples-08725");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, AliasImageMultisample) {
    TEST_DESCRIPTION("Same binding used for Multisampling and non-Multisampling");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 0) uniform texture2D BaseTexture;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        layout (constant_id = 0) const int path = 0; // always zero, but prevents dead code elimination

        void main() {
            dummy = texture(sampler2D(BaseTexture, BaseTextureSampler), vec2(0));
            if (path == 0) {
                dummy += texelFetch(BaseTextureMS, ivec2(0), 0); // invalid
            }
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-samples-08726");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, AliasImageMultisampleDescriptorSetsPartiallyBound) {
    TEST_DESCRIPTION("Same binding used for Multisampling and non-Multisampling across two descriptor sets");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        layout(set = 0, binding = 0) uniform texture2D BaseTexture;
        layout(set = 0, binding = 1) uniform sampler BaseTextureSampler;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        void main() {
            dummy = texture(sampler2D(BaseTexture, BaseTextureSampler), vec2(0));
        }
    )glsl";

    char const *cs_source_ms = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        layout(set = 0, binding = 0) uniform texture2DMS BaseTextureMS;
        layout(set = 0, binding = 2) buffer SSBO { vec4 dummy; };
        void main() {
            dummy = texelFetch(BaseTextureMS, ivec2(0), 0);
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView ms_image_view = ms_image.CreateView();

    OneOffDescriptorIndexingSet descriptor_set0(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout0(*m_device, {&descriptor_set0.layout_});
    descriptor_set0.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set0.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set0.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set0.UpdateDescriptorSets();

    OneOffDescriptorIndexingSet descriptor_set1(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout1(*m_device, {&descriptor_set1.layout_});
    descriptor_set1.WriteDescriptorImageInfo(0, ms_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set0.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set1.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set1.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.cp_ci_.layout = pipeline_layout0.handle();
    pipe.CreateComputePipeline();

    CreateComputePipelineHelper pipe_ms(*this);
    pipe_ms.cs_ = std::make_unique<VkShaderObj>(this, cs_source_ms, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe_ms.cp_ci_.layout = pipeline_layout1.handle();
    pipe_ms.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout0.handle(), 0, 1,
                              &descriptor_set0.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    // Forgot to set descriptor set
    // need to make sure GPU-AV is patching last descriptor set even though there was a dispatch inbetween
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_ms.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-samples-08726");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, MultipleCommandBuffersSameDescriptorSet) {
    TEST_DESCRIPTION("two command buffers share same descriptor set, the second one makes invalid access");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::CommandBuffer cb_0(*m_device, m_command_pool);
    vkt::CommandBuffer cb_1(*m_device, m_command_pool);

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr,
                                                              VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                             {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                         });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    auto image_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView good_view = image.CreateView();

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView bad_view = image_3d.CreateView(VK_IMAGE_VIEW_TYPE_3D);

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) buffer SSBO {
            vec4 out_value;
        };
        layout(set=0, binding=1) uniform sampler2D sample_array;
        void main() {
           out_value = texture(sample_array, vec2(0));
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    cb_0.Begin();
    vk::CmdBindPipeline(cb_0.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(cb_0.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb_0.handle(), 1, 1, 1);
    cb_0.End();

    cb_1.Begin();
    vk::CmdBindPipeline(cb_1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(cb_1.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb_1.handle(), 1, 1, 1);
    cb_1.End();

    descriptor_set.WriteDescriptorImageInfo(1, good_view, sampler);
    descriptor_set.UpdateDescriptorSets();
    m_default_queue->Submit(cb_0);
    m_default_queue->Wait();

    descriptor_set.WriteDescriptorImageInfo(1, bad_view, sampler);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewType-07752");
    m_default_queue->Submit(cb_1);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    // Make sure both can still run as normal
    descriptor_set.WriteDescriptorImageInfo(1, good_view, sampler);
    descriptor_set.UpdateDescriptorSets();
    m_default_queue->Submit(cb_0);
    m_default_queue->Submit(cb_1);
    m_default_queue->Wait();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, AliasImageBindingRuntimeArray) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7677");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *csSource = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require
        #extension GL_EXT_nonuniform_qualifier : require

        // The 3 images in this binding are [float, uint, float]
        layout(set = 0, binding = 0) uniform texture2D float_textures[];
        layout(set = 0, binding = 0) uniform utexture2D uint_textures[];
        layout(set = 0, binding = 1) buffer output_buffer {
            uint index; // is just zero, but forces runtime array
            vec4 data;
        };

        void main() {
            const vec4 value = texelFetch(float_textures[index + 0], ivec2(0), 0); // good
            const uint mask_a = texelFetch(uint_textures[index + 1], ivec2(0), 0).x; // good
            const uint mask_b = texelFetch(uint_textures[index + 2], ivec2(0), 0).x; // bad
            data = (mask_a + mask_b) > 0 ? value : vec4(0.0);
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 3, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image float_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView float_image_view = float_image.CreateView();

    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
    vkt::Image uint_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView uint_image_view = uint_image.CreateView();

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    *buffer_ptr = 0;
    buffer.Memory().Unmap();

    descriptor_set.WriteDescriptorImageInfo(0, float_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, uint_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL, 1);
    descriptor_set.WriteDescriptorImageInfo(0, float_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL, 2);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-format-07753");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, AliasImageBindingPartiallyBound) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7677");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *csSource = R"glsl(
        #version 460
        #extension GL_EXT_samplerless_texture_functions : require

        // The 3 images in this binding are [float, uint, float]
        layout(set = 0, binding = 0) uniform texture2D float_textures[3];
        layout(set = 0, binding = 0) uniform utexture2D uint_textures[3];
        layout(set = 0, binding = 1) buffer output_buffer { vec4 data; };

        void main() {
            const vec4 value = texelFetch(float_textures[0], ivec2(0), 0); // good
            const uint mask_a = texelFetch(uint_textures[1], ivec2(0), 0).x; // good
            const uint mask_b = texelFetch(uint_textures[2], ivec2(0), 0).x; // bad
            data = (mask_a + mask_b) > 0 ? value : vec4(0.0);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 3, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image float_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView float_image_view = float_image.CreateView();

    image_ci.format = VK_FORMAT_R8G8B8A8_UINT;
    vkt::Image uint_image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView uint_image_view = uint_image.CreateView();

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    descriptor_set.WriteDescriptorImageInfo(0, float_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, uint_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL, 1);
    descriptor_set.WriteDescriptorImageInfo(0, float_image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL, 2);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-format-07753");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVDescriptorPostProcess, DescriptorIndexingSlang) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // [[vk::binding(0, 0)]]
    // Texture2D texture[];
    // [[vk::binding(1, 0)]]
    // SamplerState sampler;
    //
    // struct StorageBuffer {
    //     uint data; // Will be one
    //     float4 color;
    // };
    //
    // [[vk::binding(2, 0)]]
    // RWStructuredBuffer<StorageBuffer> storageBuffer;
    //
    // [shader("compute")]
    // void main() {
    //     uint dataIndex = storageBuffer[0].data;
    //     float4 sampledColor = texture[dataIndex].Sample(sampler, float2(0));
    //     storageBuffer[0].color = sampledColor;
    // }
    char const *cs_source = R"(
               OpCapability RuntimeDescriptorArray
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %storageBuffer %texture %sampler
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %StorageBuffer_std430 0 Offset 0
               OpMemberDecorate %StorageBuffer_std430 1 Offset 16
               OpDecorate %_runtimearr_StorageBuffer_std430 ArrayStride 32
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %storageBuffer Binding 2
               OpDecorate %storageBuffer DescriptorSet 0
               OpDecorate %_runtimearr_21 ArrayStride 8
               OpDecorate %texture Binding 0
               OpDecorate %texture DescriptorSet 0
               OpDecorate %sampler Binding 1
               OpDecorate %sampler DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%StorageBuffer_std430 = OpTypeStruct %uint %v4float
%_ptr_StorageBuffer_StorageBuffer_std430 = OpTypePointer StorageBuffer %StorageBuffer_std430
%_runtimearr_StorageBuffer_std430 = OpTypeRuntimeArray %StorageBuffer_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_StorageBuffer_std430
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
         %21 = OpTypeImage %float 2D 2 0 0 1 Unknown
%_runtimearr_21 = OpTypeRuntimeArray %21
%_ptr_UniformConstant__runtimearr_21 = OpTypePointer UniformConstant %_runtimearr_21
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %21
         %28 = OpTypeSampler
%_ptr_UniformConstant_28 = OpTypePointer UniformConstant %28
         %32 = OpTypeSampledImage %21
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %36 = OpConstantComposite %v2float %float_0 %float_0
      %int_1 = OpConstant %int 1
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
%storageBuffer = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer
    %texture = OpVariable %_ptr_UniformConstant__runtimearr_21 UniformConstant
    %sampler = OpVariable %_ptr_UniformConstant_28 UniformConstant
       %main = OpFunction %void None %3
          %4 = OpLabel
         %12 = OpAccessChain %_ptr_StorageBuffer_StorageBuffer_std430 %storageBuffer %int_0 %int_0
         %18 = OpAccessChain %_ptr_StorageBuffer_uint %12 %int_0
  %dataIndex = OpLoad %uint %18
         %25 = OpAccessChain %_ptr_UniformConstant_21 %texture %dataIndex
         %27 = OpLoad %21 %25
         %29 = OpLoad %28 %sampler
%sampledImage = OpSampledImage %32 %27 %29
    %sampled = OpImageSampleExplicitLod %v4float %sampledImage %36 None
         %38 = OpCopyObject %v4float %sampled
         %39 = OpAccessChain %_ptr_StorageBuffer_StorageBuffer_std430 %storageBuffer %int_0 %int_0
         %42 = OpAccessChain %_ptr_StorageBuffer_v4float %39 %int_1
               OpStore %42 %38
               OpReturn
               OpFunctionEnd
    )";

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    *buffer_ptr = 1;
    buffer.Memory().Unmap();

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view_3d = image_3d.CreateView(VK_IMAGE_VIEW_TYPE_3D);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
         {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
         {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, image_view_3d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-viewType-07752");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
