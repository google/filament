// Copyright (c) 2019 Google LLC
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

#include <string>

#include "gmock/gmock.h"
#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using DescriptorScalarReplacementTest = PassTest<::testing::Test>;

TEST_F(DescriptorScalarReplacementTest, ExpandTexture) {
  const std::string text = R"(
; CHECK: OpDecorate [[var1:%\w+]] DescriptorSet 0
; CHECK: OpDecorate [[var1]] Binding 0
; CHECK: OpDecorate [[var2:%\w+]] DescriptorSet 0
; CHECK: OpDecorate [[var2]] Binding 1
; CHECK: OpDecorate [[var3:%\w+]] DescriptorSet 0
; CHECK: OpDecorate [[var3]] Binding 2
; CHECK: OpDecorate [[var4:%\w+]] DescriptorSet 0
; CHECK: OpDecorate [[var4]] Binding 3
; CHECK: OpDecorate [[var5:%\w+]] DescriptorSet 0
; CHECK: OpDecorate [[var5]] Binding 4
; CHECK: [[image_type:%\w+]] = OpTypeImage
; CHECK: [[ptr_type:%\w+]] = OpTypePointer UniformConstant [[image_type]]
; CHECK: [[var1]] = OpVariable [[ptr_type]] UniformConstant
; CHECK: [[var2]] = OpVariable [[ptr_type]] UniformConstant
; CHECK: [[var3]] = OpVariable [[ptr_type]] UniformConstant
; CHECK: [[var4]] = OpVariable [[ptr_type]] UniformConstant
; CHECK: [[var5]] = OpVariable [[ptr_type]] UniformConstant
; CHECK: OpLoad [[image_type]] [[var1]]
; CHECK: OpLoad [[image_type]] [[var2]]
; CHECK: OpLoad [[image_type]] [[var3]]
; CHECK: OpLoad [[image_type]] [[var4]]
; CHECK: OpLoad [[image_type]] [[var5]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpDecorate %MyTextures DescriptorSet 0
               OpDecorate %MyTextures Binding 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
       %uint = OpTypeInt 32 0
     %uint_5 = OpConstant %uint 5
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_5 = OpTypeArray %type_2d_image %uint_5
%_ptr_UniformConstant__arr_type_2d_image_uint_5 = OpTypePointer UniformConstant %_arr_type_2d_image_uint_5
    %v2float = OpTypeVector %float 2
       %void = OpTypeVoid
         %26 = OpTypeFunction %void
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
 %MyTextures = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_5 UniformConstant
       %main = OpFunction %void None %26
         %28 = OpLabel
         %29 = OpUndef %v2float
         %30 = OpAccessChain %_ptr_UniformConstant_type_2d_image %MyTextures %int_0
         %31 = OpLoad %type_2d_image %30
         %35 = OpAccessChain %_ptr_UniformConstant_type_2d_image %MyTextures %int_1
         %36 = OpLoad %type_2d_image %35
         %40 = OpAccessChain %_ptr_UniformConstant_type_2d_image %MyTextures %int_2
         %41 = OpLoad %type_2d_image %40
         %45 = OpAccessChain %_ptr_UniformConstant_type_2d_image %MyTextures %int_3
         %46 = OpLoad %type_2d_image %45
         %50 = OpAccessChain %_ptr_UniformConstant_type_2d_image %MyTextures %int_4
         %51 = OpLoad %type_2d_image %50
               OpReturn
               OpFunctionEnd

  )";

  SinglePassRunAndMatch<DescriptorScalarReplacement>(text, true);
}

TEST_F(DescriptorScalarReplacementTest, ExpandSampler) {
  const std::string text = R"(
; CHECK: OpDecorate [[var1:%\w+]] DescriptorSet 0
; CHECK: OpDecorate [[var1]] Binding 1
; CHECK: OpDecorate [[var2:%\w+]] DescriptorSet 0
; CHECK: OpDecorate [[var2]] Binding 2
; CHECK: OpDecorate [[var3:%\w+]] DescriptorSet 0
; CHECK: OpDecorate [[var3]] Binding 3
; CHECK: [[sampler_type:%\w+]] = OpTypeSampler
; CHECK: [[ptr_type:%\w+]] = OpTypePointer UniformConstant [[sampler_type]]
; CHECK: [[var1]] = OpVariable [[ptr_type]] UniformConstant
; CHECK: [[var2]] = OpVariable [[ptr_type]] UniformConstant
; CHECK: [[var3]] = OpVariable [[ptr_type]] UniformConstant
; CHECK: OpLoad [[sampler_type]] [[var1]]
; CHECK: OpLoad [[sampler_type]] [[var2]]
; CHECK: OpLoad [[sampler_type]] [[var3]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpDecorate %MySampler DescriptorSet 0
               OpDecorate %MySampler Binding 1
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
%type_sampler = OpTypeSampler
%_arr_type_sampler_uint_3 = OpTypeArray %type_sampler %uint_3
%_ptr_UniformConstant__arr_type_sampler_uint_3 = OpTypePointer UniformConstant %_arr_type_sampler_uint_3
       %void = OpTypeVoid
         %26 = OpTypeFunction %void
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
  %MySampler = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant
       %main = OpFunction %void None %26
         %28 = OpLabel
         %31 = OpAccessChain %_ptr_UniformConstant_type_sampler %MySampler %int_0
         %32 = OpLoad %type_sampler %31
         %35 = OpAccessChain %_ptr_UniformConstant_type_sampler %MySampler %int_1
         %36 = OpLoad %type_sampler %35
         %40 = OpAccessChain %_ptr_UniformConstant_type_sampler %MySampler %int_2
         %41 = OpLoad %type_sampler %40
               OpReturn
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<DescriptorScalarReplacement>(text, true);
}

TEST_F(DescriptorScalarReplacementTest, ExpandSSBO) {
  // Tests the expansion of an SSBO.  Also check that an access chain with more
  // than 1 index is correctly handled.
  const std::string text = R"(
; CHECK: OpDecorate [[var1:%\w+]] DescriptorSet 0
; CHECK: OpDecorate [[var1]] Binding 0
; CHECK: OpDecorate [[var2:%\w+]] DescriptorSet 0
; CHECK: OpDecorate [[var2]] Binding 1
; CHECK: OpTypeStruct
; CHECK: [[struct_type:%\w+]] = OpTypeStruct
; CHECK: [[ptr_type:%\w+]] = OpTypePointer Uniform [[struct_type]]
; CHECK: [[var1]] = OpVariable [[ptr_type]] Uniform
; CHECK: [[var2]] = OpVariable [[ptr_type]] Uniform
; CHECK: [[ac1:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[var1]] %uint_0 %uint_0 %uint_0
; CHECK: OpLoad %v4float [[ac1]]
; CHECK: [[ac2:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[var2]] %uint_0 %uint_0 %uint_0
; CHECK: OpLoad %v4float [[ac2]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpDecorate %buffers DescriptorSet 0
               OpDecorate %buffers Binding 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 16
               OpMemberDecorate %type_StructuredBuffer_S 0 Offset 0
               OpMemberDecorate %type_StructuredBuffer_S 0 NonWritable
               OpDecorate %type_StructuredBuffer_S BufferBlock
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %S = OpTypeStruct %v4float
%_runtimearr_S = OpTypeRuntimeArray %S
%type_StructuredBuffer_S = OpTypeStruct %_runtimearr_S
%_arr_type_StructuredBuffer_S_uint_2 = OpTypeArray %type_StructuredBuffer_S %uint_2
%_ptr_Uniform__arr_type_StructuredBuffer_S_uint_2 = OpTypePointer Uniform %_arr_type_StructuredBuffer_S_uint_2
%_ptr_Uniform_type_StructuredBuffer_S = OpTypePointer Uniform %type_StructuredBuffer_S
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %buffers = OpVariable %_ptr_Uniform__arr_type_StructuredBuffer_S_uint_2 Uniform
       %main = OpFunction %void None %19
         %21 = OpLabel
         %22 = OpAccessChain %_ptr_Uniform_v4float %buffers %uint_0 %uint_0 %uint_0 %uint_0
         %23 = OpLoad %v4float %22
         %24 = OpAccessChain %_ptr_Uniform_type_StructuredBuffer_S %buffers %uint_1
         %25 = OpAccessChain %_ptr_Uniform_v4float %24 %uint_0 %uint_0 %uint_0
         %26 = OpLoad %v4float %25
               OpReturn
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<DescriptorScalarReplacement>(text, true);
}

TEST_F(DescriptorScalarReplacementTest, NameNewVariables) {
  // Checks that if the original variable has a name, then the new variables
  // will have a name derived from that name.
  const std::string text = R"(
; CHECK: OpName [[var1:%\w+]] "SSBO[0]"
; CHECK: OpName [[var2:%\w+]] "SSBO[1]"
; CHECK: OpDecorate [[var1]] DescriptorSet 0
; CHECK: OpDecorate [[var1]] Binding 0
; CHECK: OpDecorate [[var2]] DescriptorSet 0
; CHECK: OpDecorate [[var2]] Binding 1
; CHECK: OpTypeStruct
; CHECK: [[struct_type:%\w+]] = OpTypeStruct
; CHECK: [[ptr_type:%\w+]] = OpTypePointer Uniform [[struct_type]]
; CHECK: [[var1]] = OpVariable [[ptr_type]] Uniform
; CHECK: [[var2]] = OpVariable [[ptr_type]] Uniform
; CHECK: [[ac1:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[var1]] %uint_0 %uint_0 %uint_0
; CHECK: OpLoad %v4float [[ac1]]
; CHECK: [[ac2:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[var2]] %uint_0 %uint_0 %uint_0
; CHECK: OpLoad %v4float [[ac2]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %buffers "SSBO"
               OpDecorate %buffers DescriptorSet 0
               OpDecorate %buffers Binding 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 16
               OpMemberDecorate %type_StructuredBuffer_S 0 Offset 0
               OpMemberDecorate %type_StructuredBuffer_S 0 NonWritable
               OpDecorate %type_StructuredBuffer_S BufferBlock
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %S = OpTypeStruct %v4float
%_runtimearr_S = OpTypeRuntimeArray %S
%type_StructuredBuffer_S = OpTypeStruct %_runtimearr_S
%_arr_type_StructuredBuffer_S_uint_2 = OpTypeArray %type_StructuredBuffer_S %uint_2
%_ptr_Uniform__arr_type_StructuredBuffer_S_uint_2 = OpTypePointer Uniform %_arr_type_StructuredBuffer_S_uint_2
%_ptr_Uniform_type_StructuredBuffer_S = OpTypePointer Uniform %type_StructuredBuffer_S
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %buffers = OpVariable %_ptr_Uniform__arr_type_StructuredBuffer_S_uint_2 Uniform
       %main = OpFunction %void None %19
         %21 = OpLabel
         %22 = OpAccessChain %_ptr_Uniform_v4float %buffers %uint_0 %uint_0 %uint_0 %uint_0
         %23 = OpLoad %v4float %22
         %24 = OpAccessChain %_ptr_Uniform_type_StructuredBuffer_S %buffers %uint_1
         %25 = OpAccessChain %_ptr_Uniform_v4float %24 %uint_0 %uint_0 %uint_0
         %26 = OpLoad %v4float %25
               OpReturn
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<DescriptorScalarReplacement>(text, true);
}
}  // namespace
}  // namespace opt
}  // namespace spvtools
