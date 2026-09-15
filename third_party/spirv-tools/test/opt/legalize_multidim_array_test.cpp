// Copyright (c) 2026 Google LLC
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

#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using LegalizeMultidimArrayTest = PassTest<::testing::Test>;

TEST_F(LegalizeMultidimArrayTest, Flatten2DResourceArray) {
  // HLSL:
  // Texture2D g_Textures[2][3];
  // SamplerState g_Sampler;
  // float4 main(float2 uv : TEXCOORD) : SV_Target {
  //   return g_Textures[0][1].Sample(g_Sampler, uv);
  // }
  const std::string text = R"(
; CHECK: %uint_6 = OpConstant %uint 6
; CHECK: %_arr_type_2d_image_uint_6 = OpTypeArray %type_2d_image %uint_6
; CHECK: %_ptr_UniformConstant__arr_type_2d_image_uint_6 = OpTypePointer UniformConstant %_arr_type_2d_image_uint_6
; CHECK: %g_Textures = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_6 UniformConstant
; CHECK: [[mul:%\w+]] = OpIMul %uint %int_0 %uint_3
; CHECK: [[idx:%\w+]] = OpIAdd %uint [[mul]] %int_1
; CHECK: [[ptr:%\w+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures [[idx]]
; CHECK: OpLoad %type_2d_image [[ptr]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_TEXCOORD %out_var_SV_Target
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %g_Textures "g_Textures"
               OpName %type_sampler "type.sampler"
               OpName %g_Sampler "g_Sampler"
               OpName %main "main"
               OpName %src_main "src.main"
               OpDecorate %in_var_TEXCOORD Location 0
               OpDecorate %out_var_SV_Target Location 0
               OpDecorate %g_Textures DescriptorSet 0
               OpDecorate %g_Textures Binding 0
               OpDecorate %g_Sampler DescriptorSet 0
               OpDecorate %g_Sampler Binding 1
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_3 = OpTypeArray %type_2d_image %uint_3
%_arr__arr_type_2d_image_uint_3_uint_2 = OpTypeArray %_arr_type_2d_image_uint_3 %uint_2
%_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 = OpTypePointer UniformConstant %_arr__arr_type_2d_image_uint_3_uint_2
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %24 = OpTypeFunction %void
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %31 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_UniformConstant__arr_type_2d_image_uint_3 = OpTypePointer UniformConstant %_arr_type_2d_image_uint_3
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampled_image = OpTypeSampledImage %type_2d_image
 %g_Textures = OpVariable %_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 UniformConstant
  %g_Sampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %24
         %25 = OpLabel
%param_var_uv = OpVariable %_ptr_Function_v2float Function
         %28 = OpLoad %v2float %in_var_TEXCOORD
               OpStore %param_var_uv %28
         %29 = OpFunctionCall %v4float %src_main %param_var_uv
               OpStore %out_var_SV_Target %29
               OpReturn
               OpFunctionEnd
   %src_main = OpFunction %v4float None %31
         %uv = OpFunctionParameter %_ptr_Function_v2float
   %bb_entry = OpLabel
         %37 = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures %int_0 %int_1
         %38 = OpLoad %type_2d_image %37
         %39 = OpLoad %type_sampler %g_Sampler
         %40 = OpLoad %v2float %uv
         %42 = OpSampledImage %type_sampled_image %38 %39
         %43 = OpImageSampleImplicitLod %v4float %42 %40 None
               OpReturnValue %43
               OpFunctionEnd
  )";

  const std::string expected = R"(
)";

  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text,
                                                   /*do_validation=*/true);
}

// Test that the pass fails when the access chain is split into multiple access
// chains. We expect CombineAccessChains to be run before this pass to avoid
// this.
TEST_F(LegalizeMultidimArrayTest, IndirectUseViaPartialAccessChain) {
  // HLSL source approximation:
  // Texture2D g_Textures[2][3];
  // ...
  // Texture2D row[3] = g_Textures[0];
  // return row[1].Sample(...);
  //
  // In SPIR-V, this often looks like:
  // %ptr_row = OpAccessChain %_ptr_UniformConstant_arr_type_2d_image_uint_3
  // %g_Textures %int_0 %ptr_tex = OpAccessChain
  // %_ptr_UniformConstant_type_2d_image %ptr_row %int_1 OpLoad %type_2d_image
  // %ptr_tex

  const std::string text = R"(
  ; CHECK: Unable to legalize multidimensional array
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %out_var_SV_Target
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %g_Textures "g_Textures"
               OpName %main "main"
               OpDecorate %out_var_SV_Target Location 0
               OpDecorate %g_Textures DescriptorSet 0
               OpDecorate %g_Textures Binding 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_3 = OpTypeArray %type_2d_image %uint_3
%_arr__arr_type_2d_image_uint_3_uint_2 = OpTypeArray %_arr_type_2d_image_uint_3 %uint_2
%_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 = OpTypePointer UniformConstant %_arr__arr_type_2d_image_uint_3_uint_2
%_ptr_UniformConstant__arr_type_2d_image_uint_3 = OpTypePointer UniformConstant %_arr_type_2d_image_uint_3
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %24 = OpTypeFunction %void
 %g_Textures = OpVariable %_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 UniformConstant
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %24
         %25 = OpLabel
         %37 = OpAccessChain %_ptr_UniformConstant__arr_type_2d_image_uint_3 %g_Textures %int_0
         %38 = OpAccessChain %_ptr_UniformConstant_type_2d_image %37 %int_1
         %39 = OpLoad %type_2d_image %38
               OpReturn
               OpFunctionEnd
  )";

  SinglePassRunAndFail<LegalizeMultidimArrayPass>(text);
}

TEST_F(LegalizeMultidimArrayTest, Flatten3DResourceArray) {
  // Texture2D g_Textures[2][3][4];
  // Access: g_Textures[0][1][2]
  const std::string text = R"(
; CHECK: %uint_24 = OpConstant %uint 24
; CHECK: %_arr_type_2d_image_uint_24 = OpTypeArray %type_2d_image %uint_24
; CHECK: %_ptr_UniformConstant__arr_type_2d_image_uint_24 = OpTypePointer UniformConstant %_arr_type_2d_image_uint_24
; CHECK: %g_Textures = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_24 UniformConstant
; CHECK: [[mul1:%\w+]] = OpIMul %uint %int_0 %uint_12
; CHECK: [[mul2:%\w+]] = OpIMul %uint %int_1 %uint_4
; CHECK: [[add1:%\w+]] = OpIAdd %uint [[mul1]] [[mul2]]
; CHECK: [[final_idx:%\w+]] = OpIAdd %uint [[add1]] %int_2
; CHECK: [[ptr:%\w+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures [[final_idx]]
; CHECK: OpLoad %type_2d_image [[ptr]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %type_2d_image "type.2d.image"
               OpName %g_Textures "g_Textures"
               OpDecorate %g_Textures DescriptorSet 0
               OpDecorate %g_Textures Binding 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %uint_4 = OpConstant %uint 4
     %uint_12 = OpConstant %uint 12
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_4 = OpTypeArray %type_2d_image %uint_4
%_arr_arr_type_2d_image_uint_4_uint_3 = OpTypeArray %_arr_type_2d_image_uint_4 %uint_3
%_arr_arr_arr_type_2d_image_uint_4_uint_3_uint_2 = OpTypeArray %_arr_arr_type_2d_image_uint_4_uint_3 %uint_2
%_ptr_UniformConstant_arr_3d = OpTypePointer UniformConstant %_arr_arr_arr_type_2d_image_uint_4_uint_3_uint_2
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
       %void = OpTypeVoid
       %main_func = OpTypeFunction %void
 %g_Textures = OpVariable %_ptr_UniformConstant_arr_3d UniformConstant
       %main = OpFunction %void None %main_func
         %label = OpLabel
         %ptr = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures %int_0 %int_1 %int_2
         %val = OpLoad %type_2d_image %ptr
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text, true);
}

TEST_F(LegalizeMultidimArrayTest, FlattenSamplerArray) {
  // SamplerState g_Samplers[2][2];
  const std::string text = R"(
; CHECK: %uint_4 = OpConstant %uint 4
; CHECK: %_arr_type_sampler_uint_4 = OpTypeArray %type_sampler %uint_4
; CHECK: %_ptr_UniformConstant__arr_type_sampler_uint_4 = OpTypePointer UniformConstant %_arr_type_sampler_uint_4
; CHECK: %g_Samplers = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_4 UniformConstant
; CHECK: [[mul:%\w+]] = OpIMul %uint %int_0 %uint_2
; CHECK: [[idx:%\w+]] = OpIAdd %uint [[mul]] %int_1
; CHECK: [[ptr:%\w+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %g_Samplers [[idx]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %type_sampler "type.sampler"
               OpName %g_Samplers "g_Samplers"
               OpDecorate %g_Samplers DescriptorSet 0
               OpDecorate %g_Samplers Binding 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%type_sampler = OpTypeSampler
%_arr_type_sampler_uint_2 = OpTypeArray %type_sampler %uint_2
%_arr_arr_type_sampler_uint_2_uint_2 = OpTypeArray %_arr_type_sampler_uint_2 %uint_2
%_ptr_UniformConstant_arr_2d = OpTypePointer UniformConstant %_arr_arr_type_sampler_uint_2_uint_2
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
       %void = OpTypeVoid
       %main_func = OpTypeFunction %void
 %g_Samplers = OpVariable %_ptr_UniformConstant_arr_2d UniformConstant
       %main = OpFunction %void None %main_func
         %label = OpLabel
         %ptr = OpAccessChain %_ptr_UniformConstant_type_sampler %g_Samplers %int_0 %int_1
         %val = OpLoad %type_sampler %ptr
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text, true);
}

TEST_F(LegalizeMultidimArrayTest, FlattenStorageBufferArray) {
  // struct S { float f; };
  // S buffers[2][3];
  // buffers[0][1].f
  const std::string text = R"(
; CHECK: %uint_6 = OpConstant %uint 6
; CHECK: %_arr_S_uint_6 = OpTypeArray %S %uint_6
; CHECK: %_ptr_StorageBuffer__arr_S_uint_6 = OpTypePointer StorageBuffer %_arr_S_uint_6
; CHECK: %g_Buffers = OpVariable %_ptr_StorageBuffer__arr_S_uint_6 StorageBuffer
; CHECK: [[mul:%\w+]] = OpIMul %uint %int_0 %uint_3
; CHECK: [[idx:%\w+]] = OpIAdd %uint [[mul]] %int_1
; CHECK: [[ptr:%\w+]] = OpAccessChain %_ptr_StorageBuffer_float %g_Buffers [[idx]] %int_0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %S "S"
               OpName %g_Buffers "g_Buffers"
               OpDecorate %g_Buffers DescriptorSet 0
               OpDecorate %g_Buffers Binding 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %S Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
          %S = OpTypeStruct %float
%_arr_S_uint_3 = OpTypeArray %S %uint_3
%_arr__arr_S_uint_3_uint_2 = OpTypeArray %_arr_S_uint_3 %uint_2
%_ptr_StorageBuffer_arr_2d = OpTypePointer StorageBuffer %_arr__arr_S_uint_3_uint_2
%_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
       %void = OpTypeVoid
       %main_func = OpTypeFunction %void
  %g_Buffers = OpVariable %_ptr_StorageBuffer_arr_2d StorageBuffer
       %main = OpFunction %void None %main_func
         %label = OpLabel
         %ptr = OpAccessChain %_ptr_StorageBuffer_float %g_Buffers %int_0 %int_1 %int_0
         %val = OpLoad %float %ptr
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text, true);
}

TEST_F(LegalizeMultidimArrayTest, FlattenUniformArray) {
  // Uniform buffer array: MyBlock buffers[2][3];
  // Access: buffers[0][1].member
  const std::string text = R"(
; CHECK: %uint_6 = OpConstant %uint 6
; CHECK: %_arr_MyBlock_uint_6 = OpTypeArray %MyBlock %uint_6
; CHECK: %_ptr_Uniform__arr_MyBlock_uint_6 = OpTypePointer Uniform %_arr_MyBlock_uint_6
; CHECK: %g_Uniforms = OpVariable %_ptr_Uniform__arr_MyBlock_uint_6 Uniform
; CHECK: [[mul:%\w+]] = OpIMul %uint %int_0 %uint_3
; CHECK: [[idx:%\w+]] = OpIAdd %uint [[mul]] %int_1
; CHECK: [[ptr:%\w+]] = OpAccessChain %_ptr_Uniform_float %g_Uniforms [[idx]] %int_0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %MyBlock "MyBlock"
               OpName %g_Uniforms "g_Uniforms"
               OpDecorate %g_Uniforms DescriptorSet 0
               OpDecorate %g_Uniforms Binding 0
               OpMemberDecorate %MyBlock 0 Offset 0
               OpDecorate %MyBlock Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %MyBlock = OpTypeStruct %float
%_arr_MyBlock_uint_3 = OpTypeArray %MyBlock %uint_3
%_arr__arr_MyBlock_uint_3_uint_2 = OpTypeArray %_arr_MyBlock_uint_3 %uint_2
%_ptr_Uniform__arr__arr_MyBlock_uint_3_uint_2 = OpTypePointer Uniform %_arr__arr_MyBlock_uint_3_uint_2
%_ptr_Uniform_float = OpTypePointer Uniform %float
       %void = OpTypeVoid
       %main_func = OpTypeFunction %void
 %g_Uniforms = OpVariable %_ptr_Uniform__arr__arr_MyBlock_uint_3_uint_2 Uniform
       %main = OpFunction %void None %main_func
         %label = OpLabel
         %ptr = OpAccessChain %_ptr_Uniform_float %g_Uniforms %int_0 %int_1 %int_0
         %val = OpLoad %float %ptr
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text, true);
}

TEST_F(LegalizeMultidimArrayTest, AccessChainThroughCopyObject) {
  // Texture2D g_Textures[2][3];
  // %copy = OpCopyObject %ptr_type %g_Textures
  // %ptr = OpAccessChain %... %copy %int_0 %int_1
  const std::string text = R"(
; CHECK: %uint_6 = OpConstant %uint 6
; CHECK: %_arr_type_2d_image_uint_6 = OpTypeArray %type_2d_image %uint_6
; CHECK: %_ptr_UniformConstant__arr_type_2d_image_uint_6 = OpTypePointer UniformConstant %_arr_type_2d_image_uint_6
; CHECK: %g_Textures = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_6 UniformConstant
; CHECK: [[copy:%\w+]] = OpCopyObject %_ptr_UniformConstant__arr_type_2d_image_uint_6 %g_Textures
; CHECK: [[mul:%\w+]] = OpIMul %uint %int_0 %uint_3
; CHECK: [[idx:%\w+]] = OpIAdd %uint [[mul]] %int_1
; CHECK: [[ptr:%\w+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image [[copy]] [[idx]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %type_2d_image "type.2d.image"
               OpName %g_Textures "g_Textures"
               OpDecorate %g_Textures DescriptorSet 0
               OpDecorate %g_Textures Binding 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_3 = OpTypeArray %type_2d_image %uint_3
%_arr__arr_type_2d_image_uint_3_uint_2 = OpTypeArray %_arr_type_2d_image_uint_3 %uint_2
%_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 = OpTypePointer UniformConstant %_arr__arr_type_2d_image_uint_3_uint_2
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
       %void = OpTypeVoid
       %main_func = OpTypeFunction %void
 %g_Textures = OpVariable %_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 UniformConstant
       %main = OpFunction %void None %main_func
         %label = OpLabel
         %copy = OpCopyObject %_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 %g_Textures
         %ptr = OpAccessChain %_ptr_UniformConstant_type_2d_image %copy %int_0 %int_1
         %val = OpLoad %type_2d_image %ptr
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text, true);
}

TEST_F(LegalizeMultidimArrayTest, DynamicIndices) {
  // Access with non-constant indices.
  // g_Textures[var_i][var_j]
  const std::string text = R"(
; CHECK: [[idx1:%\w+]] = OpLoad %int %idx_var_1
; CHECK: [[idx2:%\w+]] = OpLoad %int %idx_var_2
; CHECK: [[mul:%\w+]] = OpIMul %uint [[idx1]] %uint_3
; CHECK: [[add:%\w+]] = OpIAdd %uint [[mul]] [[idx2]]
; CHECK: OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures [[add]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %type_2d_image "type.2d.image"
               OpName %g_Textures "g_Textures"
               OpName %idx_var_1 "idx_var_1"
               OpName %idx_var_2 "idx_var_2"
               OpDecorate %g_Textures DescriptorSet 0
               OpDecorate %g_Textures Binding 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_3 = OpTypeArray %type_2d_image %uint_3
%_arr__arr_type_2d_image_uint_3_uint_2 = OpTypeArray %_arr_type_2d_image_uint_3 %uint_2
%_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 = OpTypePointer UniformConstant %_arr__arr_type_2d_image_uint_3_uint_2
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%_ptr_Function_int = OpTypePointer Function %int
       %void = OpTypeVoid
       %main_func = OpTypeFunction %void
 %g_Textures = OpVariable %_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 UniformConstant
       %main = OpFunction %void None %main_func
         %label = OpLabel
    %idx_var_1 = OpVariable %_ptr_Function_int Function
    %idx_var_2 = OpVariable %_ptr_Function_int Function
         %i = OpLoad %int %idx_var_1
         %j = OpLoad %int %idx_var_2
         %ptr = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures %i %j
         %val = OpLoad %type_2d_image %ptr
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text, true);
}

TEST_F(LegalizeMultidimArrayTest, IgnoreFunctionScopeArray) {
  // Function scope array [2][3] should NOT be legalized.
  const std::string text = R"(
; CHECK: %_arr__arr_float_uint_3_uint_2 = OpTypeArray %_arr_float_uint_3 %uint_2
; CHECK: %_ptr_Function__arr__arr_float_uint_3_uint_2 = OpTypePointer Function %_arr__arr_float_uint_3_uint_2
; CHECK: %var = OpVariable %_ptr_Function__arr__arr_float_uint_3_uint_2 Function
; CHECK: OpAccessChain %_ptr_Function_float %var %int_0 %int_1
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %var "var"
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
%_arr_float_uint_3 = OpTypeArray %float %uint_3
%_arr__arr_float_uint_3_uint_2 = OpTypeArray %_arr_float_uint_3 %uint_2
%_ptr_Function__arr__arr_float_uint_3_uint_2 = OpTypePointer Function %_arr__arr_float_uint_3_uint_2
%_ptr_Function_float = OpTypePointer Function %float
       %void = OpTypeVoid
       %main_func = OpTypeFunction %void
       %main = OpFunction %void None %main_func
         %label = OpLabel
         %var = OpVariable %_ptr_Function__arr__arr_float_uint_3_uint_2 Function
         %ptr = OpAccessChain %_ptr_Function_float %var %int_0 %int_1
         %val = OpLoad %float %ptr
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text, true);
}

TEST_F(LegalizeMultidimArrayTest, IgnoreWorkgroupScopeArray) {
  // Workgroup scope array [2][3] should NOT be legalized.
  const std::string text = R"(
; CHECK: %_arr__arr_float_uint_3_uint_2 = OpTypeArray %_arr_float_uint_3 %uint_2
; CHECK: %_ptr_Workgroup__arr__arr_float_uint_3_uint_2 = OpTypePointer Workgroup %_arr__arr_float_uint_3_uint_2
; CHECK: %var = OpVariable %_ptr_Workgroup__arr__arr_float_uint_3_uint_2 Workgroup
; CHECK: OpAccessChain %_ptr_Workgroup_float %var %int_0 %int_1
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %var "var"
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
%_arr_float_uint_3 = OpTypeArray %float %uint_3
%_arr__arr_float_uint_3_uint_2 = OpTypeArray %_arr_float_uint_3 %uint_2
%_ptr_Workgroup__arr__arr_float_uint_3_uint_2 = OpTypePointer Workgroup %_arr__arr_float_uint_3_uint_2
%_ptr_Workgroup_float = OpTypePointer Workgroup %float
       %void = OpTypeVoid
       %main_func = OpTypeFunction %void
 %var = OpVariable %_ptr_Workgroup__arr__arr_float_uint_3_uint_2 Workgroup
       %main = OpFunction %void None %main_func
         %label = OpLabel
         %ptr = OpAccessChain %_ptr_Workgroup_float %var %int_0 %int_1
         %val = OpLoad %float %ptr
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text, true);
}

TEST_F(LegalizeMultidimArrayTest, MultipleAccessChains) {
  // Access g_Textures[0][1] and g_Textures[1][2] in the same function.
  const std::string text = R"(
; CHECK: [[mul1:%\w+]] = OpIMul %uint %int_0 %uint_3
; CHECK: [[idx1:%\w+]] = OpIAdd %uint [[mul1]] %int_1
; CHECK: [[ptr1:%\w+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures [[idx1]]
; CHECK: OpLoad %type_2d_image [[ptr1]]
; CHECK: [[mul2:%\w+]] = OpIMul %uint %int_1 %uint_3
; CHECK: [[idx2:%\w+]] = OpIAdd %uint [[mul2]] %int_2
; CHECK: [[ptr2:%\w+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures [[idx2]]
; CHECK: OpLoad %type_2d_image [[ptr2]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %type_2d_image "type.2d.image"
               OpName %g_Textures "g_Textures"
               OpDecorate %g_Textures DescriptorSet 0
               OpDecorate %g_Textures Binding 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_3 = OpTypeArray %type_2d_image %uint_3
%_arr__arr_type_2d_image_uint_3_uint_2 = OpTypeArray %_arr_type_2d_image_uint_3 %uint_2
%_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 = OpTypePointer UniformConstant %_arr__arr_type_2d_image_uint_3_uint_2
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
       %void = OpTypeVoid
       %main_func = OpTypeFunction %void
 %g_Textures = OpVariable %_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 UniformConstant
       %main = OpFunction %void None %main_func
         %label = OpLabel
         %ptr1 = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures %int_0 %int_1
         %val1 = OpLoad %type_2d_image %ptr1
         %ptr2 = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures %int_1 %int_2
         %val2 = OpLoad %type_2d_image %ptr2
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text, true);
}

TEST_F(LegalizeMultidimArrayTest, MultipleResources) {
  // Two different resource arrays:
  // Texture2D g_Textures[2][3];
  // SamplerState g_Samplers[2][2];
  const std::string text = R"(
; CHECK: %g_Textures = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_6 UniformConstant
; CHECK: %g_Samplers = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_4 UniformConstant
; CHECK: [[mul1:%\w+]] = OpIMul %uint %int_0 %uint_3
; CHECK: [[idx1:%\w+]] = OpIAdd %uint [[mul1]] %int_1
; CHECK: [[ptr1:%\w+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures [[idx1]]
; CHECK: OpLoad %type_2d_image [[ptr1]]
; CHECK: [[mul2:%\w+]] = OpIMul %uint %int_1 %uint_2
; CHECK: [[idx2:%\w+]] = OpIAdd %uint [[mul2]] %int_1
; CHECK: [[ptr2:%\w+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %g_Samplers [[idx2]]
; CHECK: OpLoad %type_sampler [[ptr2]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %type_2d_image "type.2d.image"
               OpName %g_Textures "g_Textures"
               OpName %type_sampler "type.sampler"
               OpName %g_Samplers "g_Samplers"
               OpDecorate %g_Textures DescriptorSet 0
               OpDecorate %g_Textures Binding 0
               OpDecorate %g_Samplers DescriptorSet 0
               OpDecorate %g_Samplers Binding 1
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_3 = OpTypeArray %type_2d_image %uint_3
%_arr__arr_type_2d_image_uint_3_uint_2 = OpTypeArray %_arr_type_2d_image_uint_3 %uint_2
%_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 = OpTypePointer UniformConstant %_arr__arr_type_2d_image_uint_3_uint_2
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_arr_type_sampler_uint_2 = OpTypeArray %type_sampler %uint_2
%_arr_arr_type_sampler_uint_2_uint_2 = OpTypeArray %_arr_type_sampler_uint_2 %uint_2
%_ptr_UniformConstant_arr_2d_sampler = OpTypePointer UniformConstant %_arr_arr_type_sampler_uint_2_uint_2
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
       %void = OpTypeVoid
       %main_func = OpTypeFunction %void
 %g_Textures = OpVariable %_ptr_UniformConstant__arr__arr_type_2d_image_uint_3_uint_2 UniformConstant
 %g_Samplers = OpVariable %_ptr_UniformConstant_arr_2d_sampler UniformConstant
       %main = OpFunction %void None %main_func
         %label = OpLabel
         %ptr1 = OpAccessChain %_ptr_UniformConstant_type_2d_image %g_Textures %int_0 %int_1
         %val1 = OpLoad %type_2d_image %ptr1
         %ptr2 = OpAccessChain %_ptr_UniformConstant_type_sampler %g_Samplers %int_1 %int_1
         %val2 = OpLoad %type_sampler %ptr2
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LegalizeMultidimArrayPass>(text, true);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools