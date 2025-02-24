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

class NegativeGpuAVSpirv : public GpuAVTest {};

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7462
TEST_F(NegativeGpuAVSpirv, DISABLED_LoopHeaderPhi) {
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
    //
    // Produces a required OOB check in the Loop Header block
    // The issue is the OpPhi inside the loop header, it needs to be
    // in the OpLoopMerge block for the back edge (%19) to be valid.
    // This means doing a if/else like normal breaks this.
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
    // Will get to data[4] in loop and can crash
    data[0] = 32;  // data[0]
    data[1] = 32;  // data[1]
    data[2] = 32;  // data[2]
    data[3] = 32;  // data[3]
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
}
