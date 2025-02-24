/* Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class PositiveGpuAVSpirv : public GpuAVTest {};

TEST_F(PositiveGpuAVSpirv, LoopPhi) {
    TEST_DESCRIPTION("Loop that has the Phi parent pointed to itself");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer buffer_uniform(*m_device, 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer buffer_storage(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    uint32_t *data = (uint32_t *)buffer_uniform.Memory().Map();
    data[0] = 4;  // Scene.lightCount
    buffer_uniform.Memory().Unmap();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_uniform.handle(), 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer_storage.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    // compiled with
    //  dxc -spirv -T ps_6_0 -E psmain -fspv-target-env=vulkan1.1
    //
    //  struct SceneData {
    //      uint lightCount;
    //  };
    //  struct Light {
    //      float3 position;
    //  };
    //  ConstantBuffer<SceneData> Scene  : register(b0, space0);
    //  StructuredBuffer<Light>   Lights : register(t1, space0);
    //
    //  float4 main(float4 Position : SV_POSITION, float2 TexCoord : TEXCOORD) : SV_TARGET {
    //      float3 color = (float3)0;
    //      for (uint i = 0; i < Scene.lightCount; ++i) {
    //          color += normalize(Lights[i].position);
    //      }
    //      return float4(color, 1.0);
    //  }
    //
    // Produces this nasty loop pattern where it seems at first glance the Phi 2nd Parent is actually after it
    //
    // %1 = OpLabel
    // OpBranch %2
    //
    // %2 = OpLabel
    // %phi = OpPhi %int %A %1 %B %3
    // OpLoopMerge %4 %3 None
    // OpBranchConditional %bool %3 %4
    //
    // %3 = OpLabel
    // %B = OpIAdd %int %_ %_
    // OpBranch %2
    //
    // %4 = OpLabel
    const char *fs_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %psmain "main" %out_var_SV_TARGET
               OpExecutionMode %psmain OriginUpperLeft
               OpDecorate %out_var_SV_TARGET Location 0
               OpDecorate %Scene DescriptorSet 0
               OpDecorate %Scene Binding 0
               OpDecorate %Lights DescriptorSet 0
               OpDecorate %Lights Binding 1
               OpMemberDecorate %type_ConstantBuffer_SceneData 0 Offset 0
               OpDecorate %type_ConstantBuffer_SceneData Block
               OpMemberDecorate %Light 0 Offset 0
               OpDecorate %_runtimearr_Light ArrayStride 16
               OpMemberDecorate %type_StructuredBuffer_Light 0 Offset 0
               OpMemberDecorate %type_StructuredBuffer_Light 0 NonWritable
               OpDecorate %type_StructuredBuffer_Light BufferBlock
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v3float = OpTypeVector %float 3
         %13 = OpConstantComposite %v3float %float_0 %float_0 %float_0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_1 = OpConstant %uint 1
    %float_1 = OpConstant %float 1
%type_ConstantBuffer_SceneData = OpTypeStruct %uint
%_ptr_Uniform_type_ConstantBuffer_SceneData = OpTypePointer Uniform %type_ConstantBuffer_SceneData
      %Light = OpTypeStruct %v3float
%_runtimearr_Light = OpTypeRuntimeArray %Light
%type_StructuredBuffer_Light = OpTypeStruct %_runtimearr_Light
%_ptr_Uniform_type_StructuredBuffer_Light = OpTypePointer Uniform %type_StructuredBuffer_Light
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
       %bool = OpTypeBool
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
      %Scene = OpVariable %_ptr_Uniform_type_ConstantBuffer_SceneData Uniform
     %Lights = OpVariable %_ptr_Uniform_type_StructuredBuffer_Light Uniform
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
     %psmain = OpFunction %void None %25
         %29 = OpLabel
               OpBranch %30
         %30 = OpLabel
         %31 = OpPhi %v3float %13 %29 %32 %33
         %34 = OpPhi %uint %uint_0 %29 %35 %33
         %36 = OpAccessChain %_ptr_Uniform_uint %Scene %int_0
         %37 = OpLoad %uint %36
         %38 = OpULessThan %bool %34 %37
               OpLoopMerge %39 %33 None
               OpBranchConditional %38 %33 %39
         %33 = OpLabel
         %40 = OpAccessChain %_ptr_Uniform_v3float %Lights %int_0 %34 %int_0
         %41 = OpLoad %v3float %40
         %42 = OpExtInst %v3float %1 Normalize %41
         %32 = OpFAdd %v3float %31 %42
         %35 = OpIAdd %uint %34 %uint_1
               OpBranch %30
         %39 = OpLabel
         %43 = OpCompositeExtract %float %31 0
         %44 = OpCompositeExtract %float %31 1
         %45 = OpCompositeExtract %float %31 2
         %46 = OpCompositeConstruct %v4float %43 %44 %45 %float_1
               OpStore %out_var_SV_TARGET %46
               OpReturn
               OpFunctionEnd
        )";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_command_buffer.handle());
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveGpuAVSpirv, LoopHeaderPhi) {
    TEST_DESCRIPTION("Require injection in the Loop Header block that contains a Phi");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // The folling HLSL:
    //
    // RWStructuredBuffer<int> data : register(u0);
    // [numthreads(1, 1, 1)]
    // void main() {
    //     int i = 0;
    //     for (i = 0; i < data[i]; i++) {
    //         data[0] += i;
    //     }
    // }
    const char *cs_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %data
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %data DescriptorSet 0
               OpDecorate %data Binding 0
               OpDecorate %runtimearr ArrayStride 4
               OpMemberDecorate %type_RWStructuredBuffer 0 Offset 0
               OpDecorate %type_RWStructuredBuffer Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %int_1 = OpConstant %int 1
%runtimearr = OpTypeRuntimeArray %int
%type_RWStructuredBuffer = OpTypeStruct %runtimearr
%ptr_RWStructuredBuffer = OpTypePointer StorageBuffer %type_RWStructuredBuffer
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
%ptr_StorageBuffer = OpTypePointer StorageBuffer %int
       %bool = OpTypeBool
       %data = OpVariable %ptr_RWStructuredBuffer StorageBuffer
       %main = OpFunction %void None %12
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
         %17 = OpPhi %int %int_0 %15 %18 %19
         %20 = OpBitcast %uint %17
         %21 = OpAccessChain %ptr_StorageBuffer %data %int_0 %20
         %22 = OpLoad %int %21
         %23 = OpSLessThan %bool %17 %22
               OpLoopMerge %24 %19 None
               OpBranchConditional %23 %19 %24
         %19 = OpLabel
         %25 = OpAccessChain %ptr_StorageBuffer %data %int_0 %uint_0
         %26 = OpLoad %int %25
         %27 = OpIAdd %int %26 %17
               OpStore %25 %27
         %18 = OpIAdd %int %17 %int_1
               OpBranch %16
         %24 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *data = (uint32_t *)buffer.Memory().Map();
    data[0] = 1;  // data[0]
    data[1] = 2;  // data[1]
    data[2] = 3;  // data[2]
    data[3] = 0;  // data[3]
    buffer.Memory().Unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    data = (uint32_t *)buffer.Memory().Map();
    ASSERT_EQ(4u, data[0]);
    buffer.Memory().Unmap();
}

TEST_F(PositiveGpuAVSpirv, VulkanMemoryModelDeviceScope) {
    TEST_DESCRIPTION("Test adding VulkanMemoryModelDeviceScope support");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModelDeviceScope);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    uint32_t *data = (uint32_t *)buffer.Memory().Map();
    data[0] = 1;
    buffer.Memory().Unmap();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    // Simple shader with added MemoryModel capability
    //
    // layout(set=0, binding=0) buffer InOut {
    //   uint x;
    //   uint bar[8];
    // } foo;
    // void main() {
    //     foo.bar[0] = foo.bar[foo.x];
    // }
    const char *cs_source = R"(
               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpCapability PhysicalStorageBufferAddresses
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 Vulkan
               OpEntryPoint GLCompute %main "main" %foo
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %_arr_uint_uint_8 ArrayStride 4
               OpMemberDecorate %InOut 0 Offset 0
               OpMemberDecorate %InOut 1 Offset 4
               OpDecorate %InOut Block
               OpDecorate %foo DescriptorSet 0
               OpDecorate %foo Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_uint_uint_8 = OpTypeArray %uint %uint_8
      %InOut = OpTypeStruct %uint %_arr_uint_uint_8
%_ptr_StorageBuffer_InOut = OpTypePointer StorageBuffer %InOut
        %foo = OpVariable %_ptr_StorageBuffer_InOut StorageBuffer
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpAccessChain %_ptr_StorageBuffer_uint %foo %int_0
         %17 = OpLoad %uint %16
         %18 = OpAccessChain %_ptr_StorageBuffer_uint %foo %int_1 %17
         %19 = OpLoad %uint %18
         %20 = OpAccessChain %_ptr_StorageBuffer_uint %foo %int_1 %int_0
               OpStore %20 %19
               OpReturn
               OpFunctionEnd
        )";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
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

TEST_F(PositiveGpuAVSpirv, FindMultipleStores) {
    TEST_DESCRIPTION("Catches bug when various OpStore are in top of a function");

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char shader_source[] = R"glsl(
      #version 450
      layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;  // data[4]

      int foo() {
            return (gl_WorkGroupSize.x > 1) ? 1 : 0;
      }

      void main() {
            int index = foo(); // first OpStore is not instrumented
            Data.data[index] = 0xdeadca71;
            Data.data[index + 1] = 0xdeadca71;
      }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    vkt::Buffer write_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, write_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
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
