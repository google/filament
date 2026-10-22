// Copyright (c) 2021 ZHOU He
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using RemoveUnusedInterfaceVariablesTest = PassTest<::testing::Test>;

static const std::string expected = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_Z5func1v "_Z5func1v" %out_var_SV_TARGET
OpEntryPoint Fragment %_Z5func2v "_Z5func2v" %out_var_SV_TARGET_0
OpExecutionMode %_Z5func1v OriginUpperLeft
OpExecutionMode %_Z5func2v OriginUpperLeft
OpSource HLSL 630
OpName %type_cba "type.cba"
OpMemberName %type_cba 0 "color"
OpName %cba "cba"
OpName %out_var_SV_TARGET "out.var.SV_TARGET"
OpName %out_var_SV_TARGET_0 "out.var.SV_TARGET"
OpName %_Z5func1v "_Z5func1v"
OpName %_Z5func2v "_Z5func2v"
OpDecorate %out_var_SV_TARGET Location 0
OpDecorate %out_var_SV_TARGET_0 Location 0
OpDecorate %cba DescriptorSet 0
OpDecorate %cba Binding 0
OpMemberDecorate %type_cba 0 Offset 0
OpDecorate %type_cba Block
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%type_cba = OpTypeStruct %v4float
%_ptr_Uniform_type_cba = OpTypePointer Uniform %type_cba
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%cba = OpVariable %_ptr_Uniform_type_cba Uniform
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%out_var_SV_TARGET_0 = OpVariable %_ptr_Output_v4float Output
%_Z5func1v = OpFunction %void None %14
%16 = OpLabel
%17 = OpAccessChain %_ptr_Uniform_v4float %cba %int_0
%18 = OpLoad %v4float %17
OpStore %out_var_SV_TARGET %18
OpReturn
OpFunctionEnd
%_Z5func2v = OpFunction %void None %14
%19 = OpLabel
%20 = OpAccessChain %_ptr_Uniform_v4float %cba %int_0
%21 = OpLoad %v4float %20
OpStore %out_var_SV_TARGET_0 %21
OpReturn
OpFunctionEnd
)";

TEST_F(RemoveUnusedInterfaceVariablesTest, RemoveUnusedVariable) {
  const std::string text = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_Z5func1v "_Z5func1v" %out_var_SV_TARGET %out_var_SV_TARGET_0
OpEntryPoint Fragment %_Z5func2v "_Z5func2v" %out_var_SV_TARGET %out_var_SV_TARGET_0
OpExecutionMode %_Z5func1v OriginUpperLeft
OpExecutionMode %_Z5func2v OriginUpperLeft
OpSource HLSL 630
OpName %type_cba "type.cba"
OpMemberName %type_cba 0 "color"
OpName %cba "cba"
OpName %out_var_SV_TARGET "out.var.SV_TARGET"
OpName %out_var_SV_TARGET_0 "out.var.SV_TARGET"
OpName %_Z5func1v "_Z5func1v"
OpName %_Z5func2v "_Z5func2v"
OpDecorate %out_var_SV_TARGET Location 0
OpDecorate %out_var_SV_TARGET_0 Location 0
OpDecorate %cba DescriptorSet 0
OpDecorate %cba Binding 0
OpMemberDecorate %type_cba 0 Offset 0
OpDecorate %type_cba Block
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%type_cba = OpTypeStruct %v4float
%_ptr_Uniform_type_cba = OpTypePointer Uniform %type_cba
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%cba = OpVariable %_ptr_Uniform_type_cba Uniform
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%out_var_SV_TARGET_0 = OpVariable %_ptr_Output_v4float Output
%_Z5func1v = OpFunction %void None %14
%16 = OpLabel
%17 = OpAccessChain %_ptr_Uniform_v4float %cba %int_0
%18 = OpLoad %v4float %17
OpStore %out_var_SV_TARGET %18
OpReturn
OpFunctionEnd
%_Z5func2v = OpFunction %void None %14
%19 = OpLabel
%20 = OpAccessChain %_ptr_Uniform_v4float %cba %int_0
%21 = OpLoad %v4float %20
OpStore %out_var_SV_TARGET_0 %21
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<RemoveUnusedInterfaceVariablesPass>(text, expected,
                                                            true, true);
}

TEST_F(RemoveUnusedInterfaceVariablesTest, FixMissingVariable) {
  const std::string text = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_Z5func1v "_Z5func1v"
OpEntryPoint Fragment %_Z5func2v "_Z5func2v"
OpExecutionMode %_Z5func1v OriginUpperLeft
OpExecutionMode %_Z5func2v OriginUpperLeft
OpSource HLSL 630
OpName %type_cba "type.cba"
OpMemberName %type_cba 0 "color"
OpName %cba "cba"
OpName %out_var_SV_TARGET "out.var.SV_TARGET"
OpName %out_var_SV_TARGET_0 "out.var.SV_TARGET"
OpName %_Z5func1v "_Z5func1v"
OpName %_Z5func2v "_Z5func2v"
OpDecorate %out_var_SV_TARGET Location 0
OpDecorate %out_var_SV_TARGET_0 Location 0
OpDecorate %cba DescriptorSet 0
OpDecorate %cba Binding 0
OpMemberDecorate %type_cba 0 Offset 0
OpDecorate %type_cba Block
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%type_cba = OpTypeStruct %v4float
%_ptr_Uniform_type_cba = OpTypePointer Uniform %type_cba
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%cba = OpVariable %_ptr_Uniform_type_cba Uniform
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%out_var_SV_TARGET_0 = OpVariable %_ptr_Output_v4float Output
%_Z5func1v = OpFunction %void None %14
%16 = OpLabel
%17 = OpAccessChain %_ptr_Uniform_v4float %cba %int_0
%18 = OpLoad %v4float %17
OpStore %out_var_SV_TARGET %18
OpReturn
OpFunctionEnd
%_Z5func2v = OpFunction %void None %14
%19 = OpLabel
%20 = OpAccessChain %_ptr_Uniform_v4float %cba %int_0
%21 = OpLoad %v4float %20
OpStore %out_var_SV_TARGET_0 %21
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<RemoveUnusedInterfaceVariablesPass>(text, expected,
                                                            true, true);
}

TEST_F(RemoveUnusedInterfaceVariablesTest,
       DontEliminateUntypedVariableInterfaceVulkan12) {
  const std::string spirv = R"(
                               OpCapability Shader
                               OpCapability SampledBuffer
                               OpCapability ImageBuffer
                               OpCapability Int64
                               OpCapability DescriptorHeapEXT
                               OpCapability UntypedPointersKHR
                               OpExtension "SPV_EXT_descriptor_heap"
                               OpExtension "SPV_KHR_untyped_pointers"
                               OpMemoryModel Logical GLSL450
                               OpEntryPoint Fragment %1 "main" %2
; CHECK:                       OpEntryPoint Fragment %1 "main" %2
                               OpExecutionMode %1 OriginUpperLeft
                               OpDecorate %2 BuiltIn ResourceHeapEXT
                               OpDecorateId %_runtimearr_type_buffer_ext ArrayStrideIdEXT %buffer_size
                       %uint = OpTypeInt 32 0
                      %float = OpTypeFloat 32
                    %v4float = OpTypeVector %float 4
                     %uint_0 = OpConstant %uint 0
                     %uint_1 = OpConstant %uint 1
       %type_untyped_pointer = OpTypeUntypedPointerKHR UniformConstant
                       %void = OpTypeVoid
                         %10 = OpTypeFunction %void
          %type_buffer_image = OpTypeImage %float Buffer 2 0 0 1 Rgba32f
        %type_buffer_image_0 = OpTypeImage %float Buffer 2 0 0 2 Rgba32f
            %type_buffer_ext = OpTypeBufferEXT StorageBuffer
                %buffer_size = OpConstantSizeOfEXT %uint %type_buffer_ext
%_runtimearr_type_buffer_ext = OpTypeRuntimeArray %type_buffer_ext
                          %2 = OpUntypedVariableKHR %type_untyped_pointer UniformConstant
                          %1 = OpFunction %void None %10
                          %3 = OpLabel
                         %20 = OpUntypedAccessChainKHR %type_untyped_pointer %_runtimearr_type_buffer_ext %2 %uint_0
                         %24 = OpUntypedAccessChainKHR %type_untyped_pointer %_runtimearr_type_buffer_ext %2 %uint_1
                         %26 = OpLoad %type_buffer_image %20
                         %28 = OpImageFetch %v4float %26 %uint_0 None
                         %29 = OpLoad %type_buffer_image_0 %24
                               OpImageWrite %29 %uint_0 %28 None
                               OpReturn
                               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SinglePassRunAndMatch<RemoveUnusedInterfaceVariablesPass>(spirv, true);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
