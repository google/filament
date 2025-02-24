/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

void GpuAVDescriptorIndexingTest::InitGpuVUDescriptorIndexing() {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::descriptorBindingVariableDescriptorCount);
    AddRequiredFeature(vkt::Feature::shaderSampledImageArrayNonUniformIndexing);
    AddRequiredFeature(vkt::Feature::shaderStorageBufferArrayNonUniformIndexing);

    RETURN_IF_SKIP(InitState());
}

class PositiveGpuAVDescriptorIndexing : public GpuAVDescriptorIndexingTest {};

TEST_F(PositiveGpuAVDescriptorIndexing, Basic) {
    TEST_DESCRIPTION("Basic indexing into a valid descriptor index");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    // send index to select in image array
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 1;
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
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) uniform Input {
            uint index;
        } in_buffer;

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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, BasicHLSL) {
    TEST_DESCRIPTION("Basic indexing into a valid descriptor index with HLSL");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    // send index to select in image array
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 1;
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
    // only indexing into textures[1]
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, BasicHLSLRuntimeArray) {
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
    // only indexing into textures[7]
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 7);
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, NonUniformSamplers) {
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
            vec4 a = texture(sampler2D(kTextures2D[pc.tId], kSamplers[pc.sId]), vec2(0));
            vec4 b = texture(nonuniformEXT(sampler2D(kTextures2D[pc.tId], kSamplers[pc.sId])), vec2(0));
            out_color = a + b;
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
    uint32_t sampler_id = 0;
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4, &texture_id);
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 4, 4, &sampler_id);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, UnInitImage) {
    TEST_DESCRIPTION(
        "Make sure there's not a crash if the sampler of a combined image sampler is initialized but the image isn't.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    const uint32_t buffer_size = 1024;
    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    const VkSampler samplers[2] = {sampler.handle(), sampler.handle()};

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, samplers,
             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
        });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorBufferInfo(0, buffer0, 0, sizeof(uint32_t));
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

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

        layout(set = 0, binding = 2) uniform sampler2D tex[];
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
    vk::CmdEndRenderPass(m_command_buffer.handle());
    m_command_buffer.End();

    uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 1;
    buffer0.Memory().Unmap();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, ImageMultiBinding) {
    TEST_DESCRIPTION("Make sure multiple variables using a single binding does not produce false errors.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    const uint32_t buffer_size = 1024;
    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    const VkSampler samplers[2] = {sampler.handle(), sampler.handle()};

    OneOffDescriptorIndexingSet descriptor_set(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, samplers,
             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorBufferInfo(0, buffer0, 0, sizeof(uint32_t));
    descriptor_set.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

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

    layout(set = 0, binding = 2) uniform sampler3D tex3d[];
    layout(set = 0, binding = 2) uniform sampler2D tex[];
    layout(location = 0) out vec4 uFragColor;
    layout(location = 0) in flat uint index;
    void main() {
        if ((index & 1) != 0) {
                uFragColor = texture(tex[index], vec2(0, 0));
            } else {
                uFragColor = texture(tex3d[index], vec3(0, 0, 0));
            }
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
    vk::CmdEndRenderPass(m_command_buffer.handle());
    m_command_buffer.End();

    uint32_t *buffer_ptr = (uint32_t *)buffer0.Memory().Map();
    buffer_ptr[0] = 1;
    buffer0.Memory().Unmap();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, BindingUnusedPipeline) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7737");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *fs_source_2d = R"glsl(
        #version 450
        layout(location = 0) out vec4 outColor;
        layout(set = 1, binding = 2) uniform sampler2D tex;

        void main() {
            outColor = texture(tex, gl_FragCoord.xy);
        }
    )glsl";

    char const *fs_source_3d = R"glsl(
        #version 450
        layout(location = 0) out vec4 outColor;
        layout(set = 1, binding = 2) uniform sampler3D tex;

        void main() {
            outColor = texture(tex, gl_FragCoord.xyz);
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs_2d(this, fs_source_2d, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkShaderObj fs_3d(this, fs_source_3d, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    auto image_ci = vkt::Image::ImageCreateInfo2D(16, 16, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view_3d = image_3d.CreateView(VK_IMAGE_VIEW_TYPE_3D);

    // Want to make sure using set = 0 / binding = 0 isn't covering up the issue
    OneOffDescriptorSet descriptor_set(m_device, {{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    OneOffDescriptorSet descriptor_set_unused(m_device, {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_unused.layout_, &descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(2, image_view_3d, sampler);
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe_2d(*this);
    pipe_2d.shader_stages_ = {vs.GetStageCreateInfo(), fs_2d.GetStageCreateInfo()};
    pipe_2d.gp_ci_.layout = pipeline_layout.handle();
    pipe_2d.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_3d(*this);
    pipe_3d.shader_stages_ = {vs.GetStageCreateInfo(), fs_3d.GetStageCreateInfo()};
    pipe_3d.gp_ci_.layout = pipeline_layout.handle();
    pipe_3d.CreateGraphicsPipeline();

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2d.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_3d.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, SampledImageShareBindingArray) {
    TEST_DESCRIPTION("Make sure the binding from the correct set it detected");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::imageCubeArray);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : require
        layout (set = 0, binding = 0) uniform texture2D kTextures2D[];
        layout (set = 0, binding = 1) uniform sampler kSamplers[];
        layout (set = 1, binding = 0) uniform textureCube kTexturesCube[];

        vec4 textureBindlessCube(uint textureid, uint samplerid) {
            return texture(samplerCube(kTexturesCube[textureid], kSamplers[samplerid]), vec3(0.0));
        }
        vec4 textureBindless(uint textureid, uint samplerid) {
            return texture(sampler2D(kTextures2D[textureid], kSamplers[samplerid]), vec2(0.0));
        }

        layout (location=0) out vec4 color;

        void main() {
            color = textureBindlessCube(1, 0);
            color += textureBindless(0, 0);
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_, &descriptor_set.layout_});

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image_2d(*m_device, image_ci);
    image_2d.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view_2d = image_2d.CreateView();

    image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    image_ci.arrayLayers = 6;
    vkt::Image image_cube(*m_device, image_ci);
    image_cube.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view_cube = image_cube.CreateView(VK_IMAGE_VIEW_TYPE_CUBE_ARRAY);

    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, image_view_2d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, image_view_cube, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    const VkDescriptorSet sets[2] = {descriptor_set.set_, descriptor_set.set_};
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 2, sets, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7932
TEST_F(PositiveGpuAVDescriptorIndexing, SampledImageShareBindingBDA) {
    TEST_DESCRIPTION("Make sure the binding from the correct set it detected");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::imageCubeArray);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *fs_source = R"glsl(
        #version 460
        #extension GL_EXT_buffer_reference : require
        #extension GL_EXT_nonuniform_qualifier : require
        layout (set = 0, binding = 4) uniform texture2D kTextures2D[];
        layout (set = 0, binding = 5) uniform sampler kSamplers[];
        layout (set = 1, binding = 4) uniform textureCube kTexturesCube[];

        vec4 textureBindlessCube(uint textureid, uint samplerid) {
            return texture(samplerCube(kTexturesCube[textureid], kSamplers[samplerid]), vec3(0.0));
        }
        vec4 textureBindless(uint textureid, uint samplerid) {
            return texture(sampler2D(kTextures2D[textureid], kSamplers[samplerid]), vec2(0.0));
        }

        layout(location=0) out vec4 color;

        layout(std430, buffer_reference) readonly buffer PerFrame {
            uint texture_cube_id;
            uint texture_2d_id;
            uint sampler_id;
        };

        layout(push_constant) uniform constants {
            PerFrame perFrame;
        } pc;

        void main() {
            color = textureBindlessCube(pc.perFrame.texture_cube_id, pc.perFrame.sampler_id);
            color += textureBindless(pc.perFrame.texture_2d_id, pc.perFrame.sampler_id);
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    vkt::Buffer storage_buffer(*m_device, 12, 0, vkt::device_address);
    uint32_t *storage_buffer_ptr = static_cast<uint32_t *>(storage_buffer.Memory().Map());
    storage_buffer_ptr[0] = 8;  // texture_cube_id
    storage_buffer_ptr[1] = 7;  // texture_2d_id
    storage_buffer_ptr[2] = 0;  // sampler_id
    storage_buffer.Memory().Unmap();

    OneOffDescriptorSet descriptor_set(m_device, {{4, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 16, VK_SHADER_STAGE_ALL, nullptr},
                                                  {5, VK_DESCRIPTOR_TYPE_SAMPLER, 16, VK_SHADER_STAGE_ALL, nullptr}});
    const std::vector<VkPushConstantRange> pc_ranges = {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VkDeviceAddress)}};
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_, &descriptor_set.layout_}, pc_ranges);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image_2d(*m_device, image_ci);
    image_2d.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view_2d = image_2d.CreateView();

    image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    image_ci.arrayLayers = 6;
    vkt::Image image_cube(*m_device, image_ci);
    image_cube.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view_cube = image_cube.CreateView(VK_IMAGE_VIEW_TYPE_CUBE_ARRAY);

    descriptor_set.WriteDescriptorImageInfo(5, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    descriptor_set.WriteDescriptorImageInfo(4, image_view_2d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(4, image_view_2d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 7);
    descriptor_set.WriteDescriptorImageInfo(4, image_view_cube, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 8);
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    const VkDescriptorSet sets[2] = {descriptor_set.set_, descriptor_set.set_};
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 2, sets, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    VkDeviceAddress storage_buffer_addr = storage_buffer.Address();
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout.handle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                         sizeof(storage_buffer_addr), &storage_buffer_addr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

// If on Mesa, also add MESA_SHADER_CACHE_DISABLE=1
TEST_F(PositiveGpuAVDescriptorIndexing, Stress) {
    TEST_DESCRIPTION("Do many indexing into the shader");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    // send index to select in image array
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 0;  // index
    buffer.Memory().Unmap();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, sizeof(uint32_t), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 2);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) buffer Data {
            uint index;
        } data;

        layout(set = 0, binding = 1) uniform sampler2D tex[];

        vec4 abc(uint index) {
            return texture(tex[index], vec2(1.0, 1.0));
        }

        vec4 bar(uint index) {
           vec4 result = vec4(1.0);
           result -= texture(tex[index], vec2(0.1, 5.0));
           result -= texture(tex[index], vec2(0.2, 5.0));
           result -= texture(tex[index], vec2(0.3, 5.0));
           result -= texture(tex[index], vec2(0.4, 5.0));
           result -= texture(tex[index], vec2(0.5, 5.0));
           result -= texture(tex[index], vec2(0.6, 5.0));
           result -= texture(tex[index], vec2(0.7, 5.0));
           result -= texture(tex[index], vec2(0.8, 5.0));
           result -= texture(tex[index], vec2(0.9, 5.0));
           result -= abc(index);
           return result;
        }

        vec4 foo(uint index) {
           vec4 result = vec4(0.0);
           result += texture(tex[index], vec2(0.1, 2.0));
           result += texture(tex[index], vec2(0.2, 2.0));
           result += texture(tex[index], vec2(0.3, 2.0));
           result += texture(tex[index], vec2(0.4, 2.0));
           result += texture(tex[index], vec2(0.5, 2.0));
           result += texture(tex[index], vec2(0.6, 2.0));
           result += texture(tex[index], vec2(0.7, 2.0));
           result += texture(tex[index], vec2(0.8, 2.0));
           result += texture(tex[index], vec2(0.9, 2.0));
           result += abc(index);
           return result;
        }

        void main() {
           vec4 result = vec4(0.0);
           result += texture(tex[data.index], vec2(0, 0));
           result += texture(tex[data.index], vec2(0.1, 0));
           result += texture(tex[data.index], vec2(0.2, 0));
           result += texture(tex[data.index], vec2(0.3, 0));
           result += texture(tex[data.index], vec2(0.4, 0));
           result += texture(tex[data.index], vec2(0.5, 0));
           result += texture(tex[data.index], vec2(0.6, 0));
           result += texture(tex[data.index], vec2(0.7, 0));
           result += texture(tex[data.index], vec2(0.8, 0));
           result += texture(tex[data.index], vec2(0.9, 0));
           result += texture(tex[data.index], vec2(0, 0.1));

           result += foo(data.index);
           result += bar(data.index);
           result += foo(data.index + 1);
           result += bar(data.index + 1);
           result += foo(data.index + 2);
           result += bar(data.index + 2);
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, SharedPipelineLayoutSubsetCompute) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8377");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &binding_flags;

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout dsl2(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                                  &flags_create_info);
    vkt::DescriptorSetLayout dsl1(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                                  &flags_create_info);
    VkDescriptorSetLayout set_layouts[2] = {dsl1.handle(), dsl2.handle()};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pSetLayouts = set_layouts;

    pipeline_layout_ci.setLayoutCount = 1;
    const vkt::PipelineLayout pipeline_layout_1(*m_device, pipeline_layout_ci);
    pipeline_layout_ci.setLayoutCount = 2;
    const vkt::PipelineLayout pipeline_layout_2(*m_device, pipeline_layout_ci);

    char const *source_1 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo_0 { int a; int b;};
        void main() {
            a = b;
        }
    )glsl";
    char const *source_2 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo_0 { int a; };
        layout(set = 1, binding = 0) buffer foo_1 { int b; };
        void main() {
            a = b;
        }
    )glsl";

    CreateComputePipelineHelper pipe1(*this);
    pipe1.cs_ = std::make_unique<VkShaderObj>(this, source_1, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe1.cp_ci_.layout = pipeline_layout_1.handle();
    pipe1.CreateComputePipeline();

    CreateComputePipelineHelper pipe2(*this);
    pipe2.cs_ = std::make_unique<VkShaderObj>(this, source_2, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe2.cp_ci_.layout = pipeline_layout_2.handle();
    pipe2.CreateComputePipeline();

    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2};
    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &pool_size;
    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 2;
    allocate_info.pSetLayouts = set_layouts;

    VkDescriptorSet descriptor_sets[2];
    vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkDescriptorBufferInfo buffer_info = {buffer.handle(), 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_sets[0];
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = &buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_sets[1];
    descriptor_writes[1].dstBinding = 0;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_2, 0, 2, descriptor_sets,
                              0, nullptr);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe2.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe1.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe2.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, SharedPipelineLayoutSubsetGraphics) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8377");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Create 2 pipeline layouts. Pipeline layout 2 starts the same as pipeline layout 1, with one descriptor set,
    // but one more descriptor set is added to it, for a total of 2.
    // Hence, it is valid to bind all descriptor slots from pipeline layout 2,
    // but use a pipeline create with pipeline layout 1 for rendering.
    // BUT,
    // since GPU-AV adds empty descriptor sets to pipeline layouts before adding the
    // instrumentation descriptor set, it creates an incompatibility between pipeline
    // layout 1 and 2 at the binding index 1: pipeline layout 1 has one empty descriptor set,
    // and pipeline layout 2 as an application defined descriptor set.
    // GPU-AV has to take care of this incompatibility, by picking the right pipeline layout to
    // bind its instrumentation descriptor set to, and by correctly restoring disturbed application
    // defined descriptor set bindings.

    VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &binding_flags;

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};

    vkt::DescriptorSetLayout dsl_1(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                                   &flags_create_info);

    std::array set_layouts = {dsl_1.handle(), dsl_1.handle()};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pSetLayouts = set_layouts.data();

    pipeline_layout_ci.setLayoutCount = 1;
    auto pipeline_layout_1 = std::make_unique<vkt::PipelineLayout>(*m_device, pipeline_layout_ci);
    pipeline_layout_ci.setLayoutCount = 2;
    const vkt::PipelineLayout pipeline_layout_2(*m_device, pipeline_layout_ci);

    char const *vs_source_1 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo_0 { int a; int b;};
        void main() {
            a = b;
        }
    )glsl";
    char const *vs_source_2 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo_0 { int a; };
        layout(set = 1, binding = 0) buffer foo_1 { int b; };
        void main() {
            a = b;
        }
    )glsl";
    VkShaderObj vs_1(this, vs_source_1, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj vs_2(this, vs_source_2, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe_1(*this);
    pipe_1.shader_stages_ = {vs_1.GetStageCreateInfo(), pipe_1.fs_->GetStageCreateInfo()};
    pipe_1.gp_ci_.layout = pipeline_layout_1->handle();
    pipe_1.CreateGraphicsPipeline();
    pipeline_layout_1 = nullptr;

    CreatePipelineHelper pipe_2(*this);
    pipe_2.shader_stages_ = {vs_2.GetStageCreateInfo(), pipe_2.fs_->GetStageCreateInfo()};
    pipe_2.gp_ci_.layout = pipeline_layout_2.handle();
    pipe_2.CreateGraphicsPipeline();

    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2};
    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &pool_size;
    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 2;
    allocate_info.pSetLayouts = set_layouts.data();

    std::array<VkDescriptorSet, 2> descriptor_sets{};
    vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets.data());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkDescriptorBufferInfo buffer_info = {buffer.handle(), 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_sets[0];
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = &buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_sets[1];
    descriptor_writes[1].dstBinding = 0;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_2, 0, 2,
                              descriptor_sets.data(), 0, nullptr);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_1.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, SharedPipelineLayoutSubsetGraphicsGPL) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8377");

    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    // Create 2 pipeline layouts. Pipeline layout 2 starts the same as pipeline layout 1, with one descriptor set,
    // but one more descriptor set is added to it, for a total of 2.
    // Hence, it is valid to bind all descriptor slots from pipeline layout 2,
    // but use a pipeline create with pipeline layout 1 for rendering.
    // BUT,
    // since GPU-AV adds empty descriptor sets to pipeline layouts before adding the
    // instrumentation descriptor set, it creates an incompatibility between pipeline
    // layout 1 and 2 at the binding index 1: pipeline layout 1 has one empty descriptor set,
    // and pipeline layout 2 as an application defined descriptor set.
    // GPU-AV has to take care of this incompatibility, by picking the right pipeline layout to
    // bind its instrumentation descriptor set to, and by correctly restoring disturbed application
    // defined descriptor set bindings.

    VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &binding_flags;

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};

    vkt::DescriptorSetLayout dsl_1(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                                   &flags_create_info);

    std::array set_layouts = {dsl_1.handle(), dsl_1.handle()};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pSetLayouts = set_layouts.data();

    pipeline_layout_ci.setLayoutCount = 1;
    const vkt::PipelineLayout pipeline_layout_1(*m_device, pipeline_layout_ci);
    pipeline_layout_ci.setLayoutCount = 2;
    const vkt::PipelineLayout pipeline_layout_2(*m_device, pipeline_layout_ci);

    char const *vs_source_1 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo_0 { int a; int b;};
        void main() {
            a = b;
        }
        )glsl";
    char const *vs_source_2 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo_0 { int a; };
        layout(set = 1, binding = 0) buffer foo_1 { int b; };
        void main() {
            a = b;
        }
        )glsl";

    vkt::SimpleGPL pipe_1(*this, pipeline_layout_1.handle(), vs_source_1);

    vkt::SimpleGPL pipe_2(*this, pipeline_layout_2.handle(), vs_source_2);

    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2};
    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &pool_size;
    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 2;
    allocate_info.pSetLayouts = set_layouts.data();

    std::array<VkDescriptorSet, 2> descriptor_sets{};
    vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets.data());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkDescriptorBufferInfo buffer_info = {buffer.handle(), 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_sets[0];
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = &buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_sets[1];
    descriptor_writes[1].dstBinding = 0;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_2, 0, 2,
                              descriptor_sets.data(), 0, nullptr);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_1.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, SharedPipelineLayoutSubsetGraphicsShaderObject) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8377");
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitDynamicRenderTarget();

    // Create 2 pipeline layouts. Pipeline layout 2 starts the same as pipeline layout 1, with one descriptor set,
    // but one more descriptor set is added to it, for a total of 2.
    // Hence, it is valid to bind all descriptor slots from pipeline layout 2,
    // but use a pipeline create with pipeline layout 1 for rendering.
    // BUT,
    // since GPU-AV adds empty descriptor sets to pipeline layouts before adding the
    // instrumentation descriptor set, it creates an incompatibility between pipeline
    // layout 1 and 2 at the binding index 1: pipeline layout 1 has one empty descriptor set,
    // and pipeline layout 2 as an application defined descriptor set.
    // GPU-AV has to take care of this incompatibility, by picking the right pipeline layout to
    // bind its instrumentation descriptor set to, and by correctly restoring disturbed application
    // defined descriptor set bindings.

    VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &binding_flags;

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};

    vkt::DescriptorSetLayout dsl_1(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                                   &flags_create_info);

    std::array set_layouts = {dsl_1.handle(), dsl_1.handle()};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pSetLayouts = set_layouts.data();
    pipeline_layout_ci.setLayoutCount = 2;
    const vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    char const *vs_source_1 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo_0 { int a; int b;};
        void main() {
            a = b;
        }
    )glsl";
    const std::vector<uint32_t> vs_spv_1 = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source_1);
    char const *vs_source_2 = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo_0 { int a; };
        layout(set = 1, binding = 0) buffer foo_1 { int b; };
        void main() {
            a = b;
        }
    )glsl";
    const std::vector<uint32_t> vs_spv_2 = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source_2);

    VkShaderCreateInfoEXT shader_obj_ci = vku::InitStructHelper();
    shader_obj_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_obj_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    shader_obj_ci.codeSize = vs_spv_1.size() * sizeof(uint32_t);
    shader_obj_ci.pCode = vs_spv_1.data();
    shader_obj_ci.pName = "main";
    shader_obj_ci.setLayoutCount = 1u;
    shader_obj_ci.pSetLayouts = set_layouts.data();
    vkt::Shader vs_1(*m_device, shader_obj_ci);
    shader_obj_ci.codeSize = vs_spv_2.size() * sizeof(uint32_t);
    shader_obj_ci.pCode = vs_spv_2.data();
    shader_obj_ci.setLayoutCount = 2u;
    vkt::Shader vs_2(*m_device, shader_obj_ci);

    const std::array<VkShaderStageFlagBits, 5> stages = {{VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                          VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                          VK_SHADER_STAGE_FRAGMENT_BIT}};
    const std::array<VkShaderEXT, 5> shaders_1 = {{vs_1.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE}};
    const std::array<VkShaderEXT, 5> shaders_2 = {{vs_2.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE}};

    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2};
    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &pool_size;
    vkt::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 2;
    allocate_info.pSetLayouts = set_layouts.data();

    std::array<VkDescriptorSet, 2> descriptor_sets{};
    vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets.data());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkDescriptorBufferInfo buffer_info = {buffer.handle(), 0, VK_WHOLE_SIZE};

    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_sets[0];
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[0].pBufferInfo = &buffer_info;
    descriptor_writes[1] = vku::InitStructHelper();
    descriptor_writes[1].dstSet = descriptor_sets[1];
    descriptor_writes[1].dstBinding = 0;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_writes[1].pBufferInfo = &buffer_info;
    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 2,
                              descriptor_sets.data(), 0, nullptr);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), size32(stages), stages.data(), shaders_2.data());
    SetDefaultDynamicStatesAll(m_command_buffer.handle());

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), size32(stages), stages.data(), shaders_1.data());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), size32(stages), stages.data(), shaders_2.data());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRendering();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, SharedPipelineLayoutSubsetGraphicsShaderObjectPushConstants) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8377");
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitDynamicRenderTarget();
    // Create 2 pipeline layouts. Pipeline layout 2 starts the same as pipeline layout 1, with one push constant range,
    // but one more push constant range is added to it, for a total of 2.
    // The descriptor set layout of both pipeline layout are empty, thus compatible
    // GPU-AV should work as expected.

    std::array<VkPushConstantRange, 2> push_constant_ranges;
    push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_ranges[0].offset = 0;
    push_constant_ranges[0].size = 2 * sizeof(uint32_t);
    push_constant_ranges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_ranges[1].offset = push_constant_ranges[0].size;
    push_constant_ranges[1].size = sizeof(uint32_t);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pushConstantRangeCount = 2;
    pipeline_layout_ci.pPushConstantRanges = push_constant_ranges.data();

    const vkt::PipelineLayout pipeline_layout_2(*m_device, pipeline_layout_ci);

    char const *vs_source_1 = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo_0 { uint a; uint b; };
        void main() {}
    )glsl";
    char const *vs_source_2 = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo_1 { uint a; uint b; };
        void main() {}
    )glsl";
    char const *fs_source_2 = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo_1 { uint c; };
        void main() {}
    )glsl";

    const std::vector<uint32_t> vs_spv_1 = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source_1);
    const std::vector<uint32_t> vs_spv_2 = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source_2);
    const std::vector<uint32_t> fs_spv_2 = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source_2);

    VkShaderCreateInfoEXT shader_obj_ci = vku::InitStructHelper();
    shader_obj_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_obj_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    shader_obj_ci.codeSize = vs_spv_1.size() * sizeof(uint32_t);
    shader_obj_ci.pCode = vs_spv_1.data();
    shader_obj_ci.pName = "main";
    shader_obj_ci.pushConstantRangeCount = 1;
    shader_obj_ci.pPushConstantRanges = &push_constant_ranges[0];
    vkt::Shader vs_1(*m_device, shader_obj_ci);

    shader_obj_ci.codeSize = vs_spv_2.size() * sizeof(uint32_t);
    shader_obj_ci.pCode = vs_spv_2.data();
    shader_obj_ci.pushConstantRangeCount = 2;
    vkt::Shader vs_2(*m_device, shader_obj_ci);

    shader_obj_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_obj_ci.codeSize = fs_spv_2.size() * sizeof(uint32_t);
    shader_obj_ci.pCode = fs_spv_2.data();
    vkt::Shader fs_2(*m_device, shader_obj_ci);

    const std::array<VkShaderStageFlagBits, 5> stages = {{VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                          VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                          VK_SHADER_STAGE_FRAGMENT_BIT}};
    const std::array<VkShaderEXT, 5> shaders_1 = {{vs_1.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE}};
    const std::array<VkShaderEXT, 5> shaders_2 = {{vs_2.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fs_2.handle()}};

    std::array<uint32_t, 3> push_constants_data = {{1, 2, 3}};

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_2.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                         static_cast<uint32_t>(2 * sizeof(uint32_t)), &push_constants_data[0]);
    vk::CmdPushConstants(m_command_buffer.handle(), pipeline_layout_2.handle(), VK_SHADER_STAGE_FRAGMENT_BIT,
                         static_cast<uint32_t>(2 * sizeof(uint32_t)), static_cast<uint32_t>(1 * sizeof(uint32_t)),
                         &push_constants_data[2]);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), size32(stages), stages.data(), shaders_2.data());
    SetDefaultDynamicStatesAll(m_command_buffer.handle());

    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), size32(stages), stages.data(), shaders_1.data());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindShadersEXT(m_command_buffer.handle(), size32(stages), stages.data(), shaders_2.data());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRendering();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, MultipleBoundDescriptorsSameSet) {
    TEST_DESCRIPTION("Bind various valid descriptor sets and do dispatch on each of them");
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, MultipleBoundDescriptorsDifferentSet) {
    TEST_DESCRIPTION("Bind various valid descriptor sets and do dispatch on each of them");
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
        layout(set = 1, binding = 1) uniform Input { uint index; };
        layout(set = 1, binding = 2) buffer StorageBuffer { uint data; } storage_buffers[];
        void main() {
            storage_buffers[index].data = 0;
        }
    )glsl";

    OneOffDescriptorSet descriptor_set_2(m_device, {
                                                       {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                       {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                   });
    const vkt::PipelineLayout pipeline_layout_2(*m_device, {&descriptor_set_2.layout_, &descriptor_set_2.layout_});

    CreateComputePipelineHelper pipe_2(*this);
    pipe_2.cs_ = std::make_unique<VkShaderObj>(this, cs_source_2, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe_2.cp_ci_.layout = pipeline_layout_2.handle();
    pipe_2.CreateComputePipeline();
    descriptor_set_2.WriteDescriptorBufferInfo(1, input_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set_2.WriteDescriptorBufferInfo(2, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set_2.WriteDescriptorBufferInfo(2, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set_2.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_1.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_1.pipeline_layout_, 0, 1,
                              &pipe_1.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_2.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_2, 1, 1,
                              &descriptor_set_2.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, MultipleBoundDescriptorsUpdateAfterBind) {
    TEST_DESCRIPTION("Bind various valid descriptor sets and do dispatch on each of them with UpdateAfterBind");
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
    descriptor_set_2.WriteDescriptorBufferInfo(2, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set_2.UpdateDescriptorSets();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, MultipleUnusedBoundDescriptorsUpdateAfterBind) {
    TEST_DESCRIPTION("Bind various valid descriptor sets and do dispatch on each of them with UpdateAfterBind");
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

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform Input { uint index; };
        layout(set = 0, binding = 1) uniform sampler2D tex[];
        void main() {
           vec4 result = texture(tex[index], vec2(0, 0));
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr,
                                                              VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                             {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                         });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout.handle();
    pipe.CreateComputePipeline();

    // same, but never will be updated
    OneOffDescriptorIndexingSet descriptor_set_unused(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr,
             VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
        });
    const vkt::PipelineLayout pipeline_layout_bad_1(*m_device, {&descriptor_set_unused.layout_});
    // use vaild set, but put at set == 1 so doesn't match shader
    const vkt::PipelineLayout pipeline_layout_bad_2(*m_device, {&descriptor_set_unused.layout_, &descriptor_set.layout_});
    const VkDescriptorSet bad_sets[2] = {descriptor_set_unused.set_, descriptor_set.set_};

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_bad_1, 0, 1, &bad_sets[1],
                              0, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_bad_2, 0, 2, bad_sets, 0,
                              nullptr);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_bad_2, 0, 1, &bad_sets[0],
                              0, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_bad_1, 0, 1, &bad_sets[1],
                              0, nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_bad_2, 0, 2, bad_sets, 0,
                              nullptr);

    // bound valid and dispatch
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_bad_1, 0, 1, &bad_sets[1],
                              0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_bad_2, 0, 2, bad_sets, 0,
                              nullptr);
    m_command_buffer.End();

    vkt::Image image(*m_device, 16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, MultipleSetSomeUninitialized) {
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

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_bad.set_, 0,
                              nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, MultipleSetSomeUninitializedUpdateAfterBind) {
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

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_bad.set_, 0,
                              nullptr);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    ds_good.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    ds_good.WriteDescriptorBufferInfo(1, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    ds_good.WriteDescriptorBufferInfo(1, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    ds_good.UpdateDescriptorSets();

    ds_bad.WriteDescriptorBufferInfo(0, input_buffer, 0, VK_WHOLE_SIZE);
    ds_bad.UpdateDescriptorSets();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, ReSubmitCommandBuffer) {
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

    // don't submit but, get in a bad state
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_bad.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    // good again on re-record
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds_good.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, VariableDescriptorCountAllocateAfterPipeline) {
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
    in_buffer_ptr[0] = 4;
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, SpecConstantNullDescriptor) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    char const *cs_source = R"glsl(
        #version 450
        // will update to zero which is a null descriptor
        layout(constant_id = 0) const uint index = 1;

        layout(set = 0, binding = 0) buffer foo {
            uint a;
        } descriptors[2];

        void main() {
            descriptors[index].a = 0;
        }
    )glsl";

    const uint32_t value = 0;
    VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo spec_info = {1, &entry, sizeof(uint32_t), &value};

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                                             &spec_info);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
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

TEST_F(PositiveGpuAVDescriptorIndexing, SpecConstantNullDescriptorBindless) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    char const *cs_source = R"glsl(
        #version 450
        // will update to zero which is a null descriptor
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
    descriptor_set.WriteDescriptorBufferInfo(0, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
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

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVDescriptorIndexing, TexelFetch) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *cs_source = R"glsl(
        #version 460
        #extension GL_EXT_nonuniform_qualifier : enable
        layout (set = 0, binding = 0) uniform samplerBuffer u_buffer;
        layout (set = 0, binding = 1) uniform sampler2D tex[];
        layout (set = 0, binding = 2) buffer SSBO { vec4 color; };

        vec4 foo(uint i) {
            return texelFetch(tex[i], ivec2(0), 0);
        }

        void main() {
            color = texelFetch(u_buffer, 4) + foo(1);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr,
                       VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                      {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr, 0},
                      {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                  });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    vkt::Buffer uniform_texel_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferView(0, uniform_buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
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

TEST_F(PositiveGpuAVDescriptorIndexing, AtomicImage) {
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

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                              VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                         });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL, 1);
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

TEST_F(PositiveGpuAVDescriptorIndexing, AtomicBuffer) {
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    char const *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer ssbo { uint x; } atomic_buffers[];
        void main() {
            atomicAdd(atomic_buffers[1].x, 1);
        }
    )glsl";

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                              VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                         });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer storage_buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    descriptor_set.WriteDescriptorBufferInfo(0, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.WriteDescriptorBufferInfo(0, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
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

TEST_F(PositiveGpuAVDescriptorIndexing, StorageImage) {
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

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                              VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                         });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL, 1);
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

TEST_F(PositiveGpuAVDescriptorIndexing, DestroyedPipelineLayout) {
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint x[]; };
        void main() {
            x[1234] = 0;
        }
    )glsl";
    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    CreatePipelineHelper pipe1(*this);
    pipe1.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe1.gp_ci_.layout = pipeline_layout.handle();
    pipe1.CreateGraphicsPipeline();

    // Destroy pipeline layout after creating pipeline
    CreatePipelineHelper pipe2(*this);
    {
        const vkt::PipelineLayout doomed_pipeline_layout(*m_device);
        pipe2.gp_ci_.layout = doomed_pipeline_layout.handle();
        pipe2.CreateGraphicsPipeline();
    }

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    // We will create a fake pipeline layout underneath here
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveGpuAVDescriptorIndexing, BindingPartiallyBound) {
    TEST_DESCRIPTION("Ensure that no validation errors for invalid descriptors if binding is PARTIALLY_BOUND");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkDescriptorBindingFlags ds_binding_flags[2] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    ds_binding_flags[0] = 0;
    // No Error
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    // Uncomment for Error
    // ds_binding_flags[1] = 0;

    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    // Prepare descriptors
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uint32_t *data = (uint32_t *)buffer.Memory().Map();
    data[0] = 0;
    buffer.Memory().Unmap();

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    // Only update binding 0
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform foo_0 { int val; } doit;
        layout(set = 0, binding = 1) uniform foo_1 { int val; } readit;
        void main() {
            if (doit.val == 0)
                gl_Position = vec4(0.0);
            else
                gl_Position = vec4(readit.val);
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexed(m_command_buffer.handle(), 1, 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}
