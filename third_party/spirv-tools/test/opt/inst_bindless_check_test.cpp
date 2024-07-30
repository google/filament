// Copyright (c) 2017-2022 Valve Corporation
// Copyright (c) 2017-2022 LunarG Inc.
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

// Bindless Check Instrumentation Tests.

#include <string>
#include <vector>

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using InstBindlessTest = PassTest<::testing::Test>;

static const std::string kFuncName = "inst_bindless_check_desc";

static const std::string kImportDeco = R"(
;CHECK: OpDecorate %)" + kFuncName + R"( LinkageAttributes ")" +
                                       kFuncName + R"(" Import
)";

static const std::string kImportStub = R"(
;CHECK: %)" + kFuncName + R"( = OpFunction %bool None {{%\w+}}
;CHECK: OpFunctionEnd
)";

TEST_F(InstBindlessTest, Simple) {
  // Texture2D g_tColor[128];
  //
  // layout(push_constant) cbuffer PerViewConstantBuffer_t
  // {
  //   uint g_nDataIdx;
  // };
  //
  // SamplerState g_sAniso;
  //
  // struct PS_INPUT
  // {
  //   float2 vTextureCoords : TEXCOORD2;
  // };
  //
  // struct PS_OUTPUT
  // {
  //   float4 vColor : SV_Target0;
  // };
  //
  // PS_OUTPUT MainPs(PS_INPUT i)
  // {
  //   PS_OUTPUT ps_output;
  //   ps_output.vColor =
  //       g_tColor[ g_nDataIdx ].Sample(g_sAniso, i.vTextureCoords.xy);
  //   return ps_output;
  // }

  const std::string entry = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
)";

  // clang-format off
  const std::string names_annots = R"(
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %PerViewConstantBuffer_t "PerViewConstantBuffer_t"
OpMemberName %PerViewConstantBuffer_t 0 "g_nDataIdx"
OpName %_ ""
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 3
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0)"
+ kImportDeco +
R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
)";

  const std::string consts_types_vars = R"(
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%16 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%_arr_16_uint_128 = OpTypeArray %16 %uint_128
%_ptr_UniformConstant__arr_16_uint_128 = OpTypePointer UniformConstant %_arr_16_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_16_uint_128 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
%24 = OpTypeSampler
%_ptr_UniformConstant_24 = OpTypePointer UniformConstant %24
%g_sAniso = OpVariable %_ptr_UniformConstant_24 UniformConstant
%26 = OpTypeSampledImage %16
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%MainPs = OpFunction %void None %10
%29 = OpLabel
%30 = OpLoad %v2float %i_vTextureCoords
%31 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%32 = OpLoad %uint %31
%33 = OpAccessChain %_ptr_UniformConstant_16 %g_tColor %32
%34 = OpLoad %16 %33
%35 = OpLoad %24 %g_sAniso
%36 = OpSampledImage %26 %34 %35
%37 = OpImageSampleImplicitLod %v4float %36 %30
OpStore %_entryPointOutput_vColor %37
;CHECK-NOT: %37 = OpImageSampleImplicitLod %v4float %36 %30
;CHECK-NOT: OpStore %_entryPointOutput_vColor %37
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_57 {{%\w+}} %uint_3 %uint_0 %32 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %16 %33
;CHECK: {{%\w+}} = OpSampledImage %26 {{%\w+}} %35
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} %30
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %_entryPointOutput_vColor [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(
      entry + names_annots + consts_types_vars + kImportStub + main_func, true,
      23u);
}

TEST_F(InstBindlessTest, InstrumentMultipleInstructions) {
  // Texture2D g_tColor[128];
  //
  // layout(push_constant) cbuffer PerViewConstantBuffer_t
  // {
  //   uint g_nDataIdx;
  //   uint g_nDataIdx2;
  // };
  //
  // SamplerState g_sAniso;
  //
  // struct PS_INPUT
  // {
  //   float2 vTextureCoords : TEXCOORD2;
  // };
  //
  // struct PS_OUTPUT
  // {
  //   float4 vColor : SV_Target0;
  // };
  //
  // PS_OUTPUT MainPs(PS_INPUT i)
  // {
  //   PS_OUTPUT ps_output;
  //
  //   float t  = g_tColor[g_nDataIdx ].Sample(g_sAniso, i.vTextureCoords.xy);
  //   float t2 = g_tColor[g_nDataIdx2].Sample(g_sAniso, i.vTextureCoords.xy);
  //   ps_output.vColor = t + t2;
  //   return ps_output;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %PerViewConstantBuffer_t "PerViewConstantBuffer_t"
OpMemberName %PerViewConstantBuffer_t 0 "g_nDataIdx"
OpName %_ ""
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 3
OpDecorate %g_tColor Binding 4
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpMemberDecorate %PerViewConstantBuffer_t 1 Offset 4
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %g_sAniso DescriptorSet 3
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%17 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%_arr_17_uint_128 = OpTypeArray %17 %uint_128
%_ptr_UniformConstant__arr_17_uint_128 = OpTypePointer UniformConstant %_arr_17_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_17_uint_128 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
%25 = OpTypeSampler
%_ptr_UniformConstant_25 = OpTypePointer UniformConstant %25
%g_sAniso = OpVariable %_ptr_UniformConstant_25 UniformConstant
%27 = OpTypeSampledImage %17
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %v4uint = OpTypeVector %uint 4
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%MainPs = OpFunction %void None %10
%30 = OpLabel
%31 = OpLoad %v2float %i_vTextureCoords
%32 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%33 = OpLoad %uint %32
%34 = OpAccessChain %_ptr_UniformConstant_17 %g_tColor %33
%35 = OpLoad %17 %34
%36 = OpLoad %25 %g_sAniso
%37 = OpSampledImage %27 %35 %36
%38 = OpImageSampleImplicitLod %v4float %37 %31
;CHECK-NOT: %38 = OpImageSampleImplicitLod %v4float %37 %31
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_60 {{%\w+}} %uint_3 %uint_4 %33 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %17 %34
;CHECK: {{%\w+}} = OpSampledImage %27 {{%\w+}} %36
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} %31
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
%39 = OpAccessChain %_ptr_PushConstant_uint %_ %int_1
%40 = OpLoad %uint %39
%41 = OpAccessChain %_ptr_UniformConstant_17 %g_tColor %40
%42 = OpLoad %17 %41
%43 = OpSampledImage %27 %42 %36
%44 = OpImageSampleImplicitLod %v4float %43 %31
%45 = OpFAdd %v4float %38 %44
;CHECK-NOT: %44 = OpImageSampleImplicitLod %v4float %43 %31
;CHECK-NOT: %45 = OpFAdd %v4float %38 %44
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_66 {{%\w+}} %uint_3 %uint_4 %40 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %17 %41
;CHECK: {{%\w+}} = OpSampledImage %27 {{%\w+}} %36
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} %31
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: %45 = OpFAdd %v4float {{%\w+}} {{%\w+}}
OpStore %_entryPointOutput_vColor %45
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstrumentOpImage) {
  // This test verifies that the pass will correctly instrument shader
  // using OpImage. This test was created by editing the SPIR-V
  // from the Simple test.

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
OpCapability StorageImageReadWithoutFormat
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %PerViewConstantBuffer_t "PerViewConstantBuffer_t"
OpMemberName %PerViewConstantBuffer_t 0 "g_nDataIdx"
OpName %_ ""
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 3
OpDecorate %g_tColor Binding 9
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%int_0 = OpConstant %int 0
%20 = OpTypeImage %float 2D 0 0 0 0 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%39 = OpTypeSampledImage %20
%_arr_39_uint_128 = OpTypeArray %39 %uint_128
%_ptr_UniformConstant__arr_39_uint_128 = OpTypePointer UniformConstant %_arr_39_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_39_uint_128 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_39 = OpTypePointer UniformConstant %39
%_ptr_Input_v2int = OpTypePointer Input %v2int
%i_vTextureCoords = OpVariable %_ptr_Input_v2int Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%MainPs = OpFunction %void None %3
%5 = OpLabel
%53 = OpLoad %v2int %i_vTextureCoords
%63 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%64 = OpLoad %uint %63
%65 = OpAccessChain %_ptr_UniformConstant_39 %g_tColor %64
%66 = OpLoad %39 %65
%75 = OpImage %20 %66
%71 = OpImageRead %v4float %75 %53
OpStore %_entryPointOutput_vColor %71
;CHECK-NOT: %71 = OpImageRead %v4float %75 %53
;CHECK-NOT: OpStore %_entryPointOutput_vColor %71
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_52 {{%\w+}} %uint_3 %uint_9 %64 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %39 %65
;CHECK: {{%\w+}} = OpImage %20 {{%\w+}}
;CHECK: {{%\w+}} = OpImageRead %v4float {{%\w+}} %53
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %_entryPointOutput_vColor {{%\w+}}
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstrumentSampledImage) {
  // This test verifies that the pass will correctly instrument shader
  // using sampled image. This test was created by editing the SPIR-V
  // from the Simple test.

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %PerViewConstantBuffer_t "PerViewConstantBuffer_t"
OpMemberName %PerViewConstantBuffer_t 0 "g_nDataIdx"
OpName %_ ""
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 4
OpDecorate %g_tColor Binding 11
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%20 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%39 = OpTypeSampledImage %20
%_arr_39_uint_128 = OpTypeArray %39 %uint_128
%_ptr_UniformConstant__arr_39_uint_128 = OpTypePointer UniformConstant %_arr_39_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_39_uint_128 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_39 = OpTypePointer UniformConstant %39
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%MainPs = OpFunction %void None %3
%5 = OpLabel
%53 = OpLoad %v2float %i_vTextureCoords
%63 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%64 = OpLoad %uint %63
%65 = OpAccessChain %_ptr_UniformConstant_39 %g_tColor %64
%66 = OpLoad %39 %65
%71 = OpImageSampleImplicitLod %v4float %66 %53
OpStore %_entryPointOutput_vColor %71
;CHECK-NOT: %71 = OpImageSampleImplicitLod %v4float %66 %53
;CHECK-NOT: OpStore %_entryPointOutput_vColor %71
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_50 {{%\w+}} %uint_4 %uint_11 %64 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %39 %65
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} %53
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %_entryPointOutput_vColor {{%\w+}}
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstrumentImageWrite) {
  // This test verifies that the pass will correctly instrument shader
  // doing bindless image write. This test was created by editing the SPIR-V
  // from the Simple test.

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
OpCapability StorageImageWriteWithoutFormat
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %PerViewConstantBuffer_t "PerViewConstantBuffer_t"
OpMemberName %PerViewConstantBuffer_t 0 "g_nDataIdx"
OpName %_ ""
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 30
OpDecorate %g_tColor Binding 2
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%int_0 = OpConstant %int 0
%20 = OpTypeImage %float 2D 0 0 0 0 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%80 = OpConstantNull %v4float
%_arr_20_uint_128 = OpTypeArray %20 %uint_128
%_ptr_UniformConstant__arr_20_uint_128 = OpTypePointer UniformConstant %_arr_20_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_20_uint_128 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_20 = OpTypePointer UniformConstant %20
%_ptr_Input_v2int = OpTypePointer Input %v2int
%i_vTextureCoords = OpVariable %_ptr_Input_v2int Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %v4uint = OpTypeVector %uint 4
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
)";

  const std::string main_func = R"(
%MainPs = OpFunction %void None %3
%5 = OpLabel
%53 = OpLoad %v2int %i_vTextureCoords
%63 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%64 = OpLoad %uint %63
%65 = OpAccessChain %_ptr_UniformConstant_20 %g_tColor %64
%66 = OpLoad %20 %65
OpImageWrite %66 %53 %80
OpStore %_entryPointOutput_vColor %80
;CHECK-NOT: OpImageWrite %66 %53 %80
;CHECK-NOT: OpStore %_entryPointOutput_vColor %80
;CHECK: %32 = OpLoad %16 %31
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_52 {{%\w+}} %uint_30 %uint_2 %30 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %16 %31
;CHECK: OpImageWrite {{%\w+}} %28 %19
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore %_entryPointOutput_vColor %19
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstrumentVertexSimple) {
  // This test verifies that the pass will correctly instrument shader
  // doing bindless image write. This test was created by editing the SPIR-V
  // from the Simple test.

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
OpCapability Sampled1D
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %_ %coords2D
OpSource GLSL 450
OpName %main "main"
OpName %lod "lod"
OpName %coords1D "coords1D"
OpName %gl_PerVertex "gl_PerVertex"
OpMemberName %gl_PerVertex 0 "gl_Position"
OpMemberName %gl_PerVertex 1 "gl_PointSize"
OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
OpMemberName %gl_PerVertex 3 "gl_CullDistance"
OpName %_ ""
OpName %texSampler1D "texSampler1D"
OpName %foo "foo"
OpMemberName %foo 0 "g_idx"
OpName %__0 ""
OpName %coords2D "coords2D"
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_VertexIndex BuiltIn VertexIndex
;CHECK: OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
OpDecorate %gl_PerVertex Block
OpDecorate %texSampler1D DescriptorSet 2
OpDecorate %texSampler1D Binding 13
OpMemberDecorate %foo 0 Offset 0
OpDecorate %foo Block
OpDecorate %__0 DescriptorSet 7
OpDecorate %__0 Binding 5
OpDecorate %coords2D Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_3 = OpConstant %float 3
%float_1_78900003 = OpConstant %float 1.78900003
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
%_ = OpVariable %_ptr_Output_gl_PerVertex Output
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%21 = OpTypeImage %float 1D 0 0 0 1 Unknown
%22 = OpTypeSampledImage %21
%uint_128 = OpConstant %uint 128
%_arr_22_uint_128 = OpTypeArray %22 %uint_128
%_ptr_UniformConstant__arr_22_uint_128 = OpTypePointer UniformConstant %_arr_22_uint_128
%texSampler1D = OpVariable %_ptr_UniformConstant__arr_22_uint_128 UniformConstant
%foo = OpTypeStruct %int
%_ptr_Uniform_foo = OpTypePointer Uniform %foo
%__0 = OpVariable %_ptr_Uniform_foo Uniform
%_ptr_Uniform_int = OpTypePointer Uniform %int
%_ptr_UniformConstant_22 = OpTypePointer UniformConstant %22
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%coords2D = OpVariable %_ptr_Input_v2float Input
;CHECK: %_ptr_Input_uint = OpTypePointer Input %uint
;CHECK: %gl_VertexIndex = OpVariable %_ptr_Input_uint Input
;CHECK: %gl_InstanceIndex = OpVariable %_ptr_Input_uint Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%lod = OpVariable %_ptr_Function_float Function
%coords1D = OpVariable %_ptr_Function_float Function
OpStore %lod %float_3
OpStore %coords1D %float_1_78900003
%31 = OpAccessChain %_ptr_Uniform_int %__0 %int_0
%32 = OpLoad %int %31
%34 = OpAccessChain %_ptr_UniformConstant_22 %texSampler1D %32
%35 = OpLoad %22 %34
%36 = OpLoad %float %coords1D
%37 = OpLoad %float %lod
%38 = OpImageSampleExplicitLod %v4float %35 %36 Lod %37
%40 = OpAccessChain %_ptr_Output_v4float %_ %int_0
OpStore %40 %38
;CHECK-NOT: %38 = OpImageSampleExplicitLod %v4float %35 %36 Lod %37
;CHECK-NOT: %40 = OpAccessChain %_ptr_Output_v4float %_ %int_0
;CHECK-NOT: OpStore %40 %38
;CHECK: {{%\w+}} = OpLoad %uint %gl_VertexIndex
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_70 {{%\w+}} %uint_7 %uint_5 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %int {{%\w+}}
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %int {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpAccessChain %_ptr_UniformConstant_25 %texSampler1D {{%\w+}}
;CHECK: {{%\w+}} = OpLoad {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLoad %float %coords1D
;CHECK: {{%\w+}} = OpLoad %float %lod
;CHECK: {{%\w+}} = OpLoad %uint %gl_VertexIndex
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpBitcast %uint {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_75 {{%\w+}} %uint_2 %uint_13 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %25 %38
;CHECK: {{%\w+}} = OpImageSampleExplicitLod %v4float {{%\w+}} %40 Lod %41
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: %43 = OpAccessChain %_ptr_Output_v4float %_ %int_0
;CHECK: OpStore %43 {{%\w+}}
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstrumentTeseSimple) {
  // This test verifies that the pass will correctly instrument tessellation
  // evaluation shader doing bindless buffer load.
  //
  // clang-format off
  //
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(std140, set = 9, binding = 1) uniform ufoo { uint index; } uniform_index_buffer;
  //
  // layout(set = 9, binding = 2) buffer bfoo { vec4 val; } adds[11];
  //
  // layout(triangles, equal_spacing, cw) in;
  //
  // void main() {
  //   gl_Position = adds[uniform_index_buffer.index].val;
  // }
  //

  const std::string defs = R"(
OpCapability Tessellation
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main" %_
;CHECK: OpEntryPoint TessellationEvaluation %main "main" %_ %gl_PrimitiveID %gl_TessCoord
OpExecutionMode %main Triangles
OpExecutionMode %main SpacingEqual
OpExecutionMode %main VertexOrderCw
OpSource GLSL 450
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpName %main "main"
OpName %gl_PerVertex "gl_PerVertex"
OpMemberName %gl_PerVertex 0 "gl_Position"
OpMemberName %gl_PerVertex 1 "gl_PointSize"
OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
OpMemberName %gl_PerVertex 3 "gl_CullDistance"
OpName %_ ""
OpName %bfoo "bfoo"
OpMemberName %bfoo 0 "val"
OpName %adds "adds"
OpName %ufoo "ufoo"
OpMemberName %ufoo 0 "index"
OpName %uniform_index_buffer "uniform_index_buffer"
OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
OpDecorate %gl_PerVertex Block
OpMemberDecorate %bfoo 0 Offset 0
OpDecorate %bfoo Block
OpDecorate %adds DescriptorSet 9
OpDecorate %adds Binding 1
OpMemberDecorate %ufoo 0 Offset 0
OpDecorate %ufoo Block
OpDecorate %uniform_index_buffer DescriptorSet 9
OpDecorate %uniform_index_buffer Binding 2
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
;CHECK: OpDecorate %gl_TessCoord BuiltIn TessCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
%_ = OpVariable %_ptr_Output_gl_PerVertex Output
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%bfoo = OpTypeStruct %v4float
%uint_11 = OpConstant %uint 11
%_arr_bfoo_uint_11 = OpTypeArray %bfoo %uint_11
%_ptr_StorageBuffer__arr_bfoo_uint_11 = OpTypePointer StorageBuffer %_arr_bfoo_uint_11
%adds = OpVariable %_ptr_StorageBuffer__arr_bfoo_uint_11 StorageBuffer
%ufoo = OpTypeStruct %uint
%_ptr_Uniform_ufoo = OpTypePointer Uniform %ufoo
%uniform_index_buffer = OpVariable %_ptr_Uniform_ufoo Uniform
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
;CHECK: %_ptr_Input_uint = OpTypePointer Input %uint
;CHECK: %gl_PrimitiveID = OpVariable %_ptr_Input_uint Input
;CHECK: %v3float = OpTypeVector %float 3
;CHECK: %_ptr_Input_v3float = OpTypePointer Input %v3float
;CHECK: %gl_TessCoord = OpVariable %_ptr_Input_v3float Input
;CHECK: %v3uint = OpTypeVector %uint 3
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func =
      R"(
%main = OpFunction %void None %3
%5 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_uint %uniform_index_buffer %int_0
%26 = OpLoad %uint %25
%28 = OpAccessChain %_ptr_StorageBuffer_v4float %adds %26 %int_0
%29 = OpLoad %v4float %28
;CHECK-NOT: %29 = OpLoad %v4float %28
;CHECK: {{%\w+}} = OpLoad %uint %gl_PrimitiveID
;CHECK: {{%\w+}} = OpLoad %v3float %gl_TessCoord
;CHECK: {{%\w+}} = OpBitcast %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_2 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_62 {{%\w+}} %uint_9 %uint_2 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %uint %27
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %uint {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
%31 = OpAccessChain %_ptr_Output_v4float %_ %int_0
OpStore %31 %29
;CHECK-NOT: OpStore %31 %29
;CHECK: {{%\w+}} = OpLoad %uint %gl_PrimitiveID
;CHECK: {{%\w+}} = OpLoad %v3float %gl_TessCoord
;CHECK: {{%\w+}} = OpBitcast %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_2 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_64 {{%\w+}} %uint_9 %uint_1 {{%\w+}} {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %v4float %29
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: %31 = OpAccessChain %_ptr_Output_v4float %_ %int_0
;CHECK: OpStore %31 [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstrumentTesc) {
  // This test verifies that the pass will correctly instrument tessellation
  // control shader
  //
  // clang-format off
  //
  // #version 450
  // layout(vertices = 3) out;
  // layout(set = 0, binding = 0) uniform texture1D _77;
  // layout(set = 0, binding = 1) uniform sampler _78;

  // layout(location = 1) flat in int _3[];
  // layout(location = 0) out vec4 _5[3];

  // void main()
  // {
  //     float param;
  //     if (_3[gl_InvocationID] == 0)
  //     {
  //         param = 0.0234375;
  //     }
  //     else
  //     {
  //         param = 1.0156199932098388671875;
  //     }
  //     _5[gl_InvocationID] = textureLod(sampler1D(_77, _78), param, 0.0);
  //     vec4 _203;
  //     if (gl_InvocationID == 0)
  //     {
  //         _203 = gl_in[0].gl_Position;
  //     }
  //     else
  //     {
  //         _203 = gl_in[2].gl_Position;
  //     }
  //     gl_out[gl_InvocationID].gl_Position = _203;
  //     gl_TessLevelInner[0] = 2.7999999523162841796875;
  //     gl_TessLevelInner[1] = 2.7999999523162841796875;
  //     gl_TessLevelOuter[0] = 2.7999999523162841796875;
  //     gl_TessLevelOuter[1] = 2.7999999523162841796875;
  //     gl_TessLevelOuter[2] = 2.7999999523162841796875;
  //     gl_TessLevelOuter[3] = 2.7999999523162841796875;
  // }
  //
  // clang-format on
  //
  //

  // clang-format off
  const std::string defs = R"(
OpCapability Tessellation
OpCapability Sampled1D
;CHECK: OpCapability Linkage
;CHECK: OpExtension "SPV_KHR_storage_buffer_storage_class"
;CHECK: OpExtension "SPV_KHR_physical_storage_buffer"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
;CHECK: OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint TessellationControl %main "main" %_3 %gl_InvocationID %_5 %gl_in %gl_out %gl_TessLevelInner %gl_TessLevelOuter
;CHECK: OpEntryPoint TessellationControl %main "main" %_3 %gl_InvocationID %_5 %gl_in %gl_out %gl_TessLevelInner %gl_TessLevelOuter %gl_PrimitiveID
OpExecutionMode %main OutputVertices 3
OpSource GLSL 450
OpName %main "main"
OpName %_3 "_3"
OpName %gl_InvocationID "gl_InvocationID"
OpName %param "param"
OpName %_5 "_5"
OpName %_77 "_77"
OpName %_78 "_78"
OpName %_203 "_203"
OpName %gl_PerVertex "gl_PerVertex"
OpMemberName %gl_PerVertex 0 "gl_Position"
OpMemberName %gl_PerVertex 1 "gl_PointSize"
OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
OpMemberName %gl_PerVertex 3 "gl_CullDistance"
OpName %gl_in "gl_in"
OpName %gl_PerVertex_0 "gl_PerVertex"
OpMemberName %gl_PerVertex_0 0 "gl_Position"
OpMemberName %gl_PerVertex_0 1 "gl_PointSize"
OpMemberName %gl_PerVertex_0 2 "gl_ClipDistance"
OpMemberName %gl_PerVertex_0 3 "gl_CullDistance"
OpName %gl_out "gl_out"
OpName %gl_TessLevelInner "gl_TessLevelInner"
OpName %gl_TessLevelOuter "gl_TessLevelOuter"
OpDecorate %_3 Flat
OpDecorate %_3 Location 1
OpDecorate %gl_InvocationID BuiltIn InvocationId
OpDecorate %_5 Location 0
OpDecorate %_77 DescriptorSet 0
OpDecorate %_77 Binding 0
OpDecorate %_78 DescriptorSet 0
OpDecorate %_78 Binding 1
OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
OpDecorate %gl_PerVertex Block
OpMemberDecorate %gl_PerVertex_0 0 BuiltIn Position
OpMemberDecorate %gl_PerVertex_0 1 BuiltIn PointSize
OpMemberDecorate %gl_PerVertex_0 2 BuiltIn ClipDistance
OpMemberDecorate %gl_PerVertex_0 3 BuiltIn CullDistance
OpDecorate %gl_PerVertex_0 Block
OpDecorate %gl_TessLevelInner Patch
OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
OpDecorate %gl_TessLevelOuter Patch
OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
%void = OpTypeVoid
%3 = OpTypeFunction %void
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_arr_int_uint_32 = OpTypeArray %int %uint_32
%_ptr_Input__arr_int_uint_32 = OpTypePointer Input %_arr_int_uint_32
%_3 = OpVariable %_ptr_Input__arr_int_uint_32 Input
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
%int_0 = OpConstant %int 0
%bool = OpTypeBool
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0_0234375 = OpConstant %float 0.0234375
%float_1_01561999 = OpConstant %float 1.01561999
%v4float = OpTypeVector %float 4
%uint_3 = OpConstant %uint 3
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_ptr_Output__arr_v4float_uint_3 = OpTypePointer Output %_arr_v4float_uint_3
%_5 = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%34 = OpTypeImage %float 1D 0 0 0 1 Unknown
%_ptr_UniformConstant_34 = OpTypePointer UniformConstant %34
%_77 = OpVariable %_ptr_UniformConstant_34 UniformConstant
%38 = OpTypeSampler
%_ptr_UniformConstant_38 = OpTypePointer UniformConstant %38
%_78 = OpVariable %_ptr_UniformConstant_38 UniformConstant
%42 = OpTypeSampledImage %34
%float_0 = OpConstant %float 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_PerVertex_uint_32 = OpTypeArray %gl_PerVertex %uint_32
%_ptr_Input__arr_gl_PerVertex_uint_32 = OpTypePointer Input %_arr_gl_PerVertex_uint_32
%gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_uint_32 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%int_2 = OpConstant %int 2
%gl_PerVertex_0 = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_PerVertex_0_uint_3 = OpTypeArray %gl_PerVertex_0 %uint_3
%_ptr_Output__arr_gl_PerVertex_0_uint_3 = OpTypePointer Output %_arr_gl_PerVertex_0_uint_3
%gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_0_uint_3 Output
%uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
%float_2_79999995 = OpConstant %float 2.79999995
%_ptr_Output_float = OpTypePointer Output %float
%int_1 = OpConstant %int 1
%uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
%int_3 = OpConstant %int 3
)";

  const std::string main_func =
      R"(
%main = OpFunction %void None %3
%5 = OpLabel
%param = OpVariable %_ptr_Function_float Function
%_203 = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %int %gl_InvocationID
%15 = OpAccessChain %_ptr_Input_int %_3 %14
%16 = OpLoad %int %15
%19 = OpIEqual %bool %16 %int_0
OpSelectionMerge %21 None
OpBranchConditional %19 %20 %26
%20 = OpLabel
;CHECK-NOT: %15 = OpAccessChain %_ptr_Input_int %_3 %14
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %int %gl_InvocationID
;CHECK: {{%\w+}} = OpAccessChain %_ptr_Input_int %_3 {{%\w+}}
;CHECK: {{%\w+}} = OpLoad %int {{%\w+}}
;CHECK: {{%\w+}} = OpIEqual %bool {{%\w+}} %int_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpStore %param %float_0_0234375
OpBranch %21
%26 = OpLabel
OpStore %param %float_1_01561999
OpBranch %21
%21 = OpLabel
%33 = OpLoad %int %gl_InvocationID
%37 = OpLoad %34 %_77
%41 = OpLoad %38 %_78
%43 = OpSampledImage %42 %37 %41
%44 = OpLoad %float %param
;CHECK: {{%\w+}} = OpLoad %int %gl_InvocationID
;CHECK: {{%\w+}} = OpBitcast %uint {{%\w+}}
;CHECK: {{%\w+}} = OpLoad %uint %gl_PrimitiveID
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_1 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %inst_bindless_check_desc %uint_23 %uint_129 {{%\w+}} %uint_0 %uint_0 %uint_0 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
%46 = OpImageSampleExplicitLod %v4float %43 %44 Lod %float_0
%48 = OpAccessChain %_ptr_Output_v4float %_5 %33
OpStore %48 %46
;CHECK-NOT: %48 = OpAccessChain %_ptr_Output_v4float %_5 %33
;CHECK-NOT: OpStore %48 %46
;CHECK: [[phi_result:%\w+]] = OpPhi %v4float {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: [[access_chain:%\w+]] = OpAccessChain %_ptr_Output_v4float %_5 {{%\w+}}
;CHECK: OpStore [[access_chain]] [[phi_result]]
%49 = OpLoad %int %gl_InvocationID
%50 = OpIEqual %bool %49 %int_0
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %64
%51 = OpLabel
%62 = OpAccessChain %_ptr_Input_v4float %gl_in %int_0 %int_0
%63 = OpLoad %v4float %62
OpStore %_203 %63
OpBranch %52
%64 = OpLabel
%66 = OpAccessChain %_ptr_Input_v4float %gl_in %int_2 %int_0
%67 = OpLoad %v4float %66
OpStore %_203 %67
OpBranch %52
%52 = OpLabel
%72 = OpLoad %int %gl_InvocationID
%73 = OpLoad %v4float %_203
%74 = OpAccessChain %_ptr_Output_v4float %gl_out %72 %int_0
OpStore %74 %73
%81 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
OpStore %81 %float_2_79999995
%83 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_1
OpStore %83 %float_2_79999995
%88 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
OpStore %88 %float_2_79999995
%89 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
OpStore %89 %float_2_79999995
%90 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
OpStore %90 %float_2_79999995
%92 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_3
OpStore %92 %float_2_79999995
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, MultipleDebugFunctions) {
  // Same source as Simple, but compiled -g and not optimized, especially not
  // inlined. The OpSource has had the source extracted for the sake of brevity.

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
%2 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
%1 = OpString "foo5.frag"
OpSource HLSL 500 %1
OpName %MainPs "MainPs"
OpName %PS_INPUT "PS_INPUT"
OpMemberName %PS_INPUT 0 "vTextureCoords"
OpName %PS_OUTPUT "PS_OUTPUT"
OpMemberName %PS_OUTPUT 0 "vColor"
OpName %_MainPs_struct_PS_INPUT_vf21_ "@MainPs(struct-PS_INPUT-vf21;"
OpName %i "i"
OpName %ps_output "ps_output"
OpName %g_tColor "g_tColor"
OpName %PerViewConstantBuffer_t "PerViewConstantBuffer_t"
OpMemberName %PerViewConstantBuffer_t 0 "g_nDataIdx"
OpName %_ ""
OpName %g_sAniso "g_sAniso"
OpName %i_0 "i"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpName %param "param"
OpDecorate %g_tColor DescriptorSet 1
OpDecorate %g_tColor Binding 2
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %g_sAniso DescriptorSet 1
OpDecorate %g_sAniso Binding 3
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%4 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%PS_INPUT = OpTypeStruct %v2float
%_ptr_Function_PS_INPUT = OpTypePointer Function %PS_INPUT
%v4float = OpTypeVector %float 4
%PS_OUTPUT = OpTypeStruct %v4float
%13 = OpTypeFunction %PS_OUTPUT %_ptr_Function_PS_INPUT
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%21 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%_arr_21_uint_128 = OpTypeArray %21 %uint_128
%_ptr_UniformConstant__arr_21_uint_128 = OpTypePointer UniformConstant %_arr_21_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_21_uint_128 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %21
%36 = OpTypeSampler
%_ptr_UniformConstant_36 = OpTypePointer UniformConstant %36
%g_sAniso = OpVariable %_ptr_UniformConstant_36 UniformConstant
%40 = OpTypeSampledImage %21
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string func1 = R"(
%MainPs = OpFunction %void None %4
%6 = OpLabel
%i_0 = OpVariable %_ptr_Function_PS_INPUT Function
%param = OpVariable %_ptr_Function_PS_INPUT Function
OpLine %1 21 0
%54 = OpLoad %v2float %i_vTextureCoords
%55 = OpAccessChain %_ptr_Function_v2float %i_0 %int_0
OpStore %55 %54
%59 = OpLoad %PS_INPUT %i_0
OpStore %param %59
%60 = OpFunctionCall %PS_OUTPUT %_MainPs_struct_PS_INPUT_vf21_ %param
%61 = OpCompositeExtract %v4float %60 0
OpStore %_entryPointOutput_vColor %61
OpReturn
OpFunctionEnd
)";

  const std::string func2 = R"(
%_MainPs_struct_PS_INPUT_vf21_ = OpFunction %PS_OUTPUT None %13
%i = OpFunctionParameter %_ptr_Function_PS_INPUT
%16 = OpLabel
%ps_output = OpVariable %_ptr_Function_PS_OUTPUT Function
OpLine %1 24 0
%31 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%32 = OpLoad %uint %31
%34 = OpAccessChain %_ptr_UniformConstant_21 %g_tColor %32
%35 = OpLoad %21 %34
%39 = OpLoad %36 %g_sAniso
%41 = OpSampledImage %40 %35 %39
%43 = OpAccessChain %_ptr_Function_v2float %i %int_0
%44 = OpLoad %v2float %43
%45 = OpImageSampleImplicitLod %v4float %41 %44
;CHECK-NOT: %45 = OpImageSampleImplicitLod %v4float %41 %44
;CHECK: {{%\w+}} = OpLoad %v2float {{%\w+}}
;CHECK: OpNoLine
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_128 {{%\w+}} %uint_1 %uint_2 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %27 {{%\w+}}
;CHECK: {{%\w+}} = OpSampledImage %37 {{%\w+}} {{%\w+}}
;CHECK: OpLine %5 24 0
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} {{%\w+}}
;CHECK: OpNoLine
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
%47 = OpAccessChain %_ptr_Function_v4float %ps_output %int_0
OpStore %47 %45
;CHECK-NOT: OpStore %47 %45
;CHECK: [[store_loc:%\w+]] = OpAccessChain %_ptr_Function_v4float %ps_output %int_0
;CHECK: OpStore [[store_loc]] [[phi_result]]
OpLine %1 25 0
%48 = OpLoad %PS_OUTPUT %ps_output
OpReturnValue %48
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(
      defs + kImportStub + func1 + func2, true, 23u);
}

TEST_F(InstBindlessTest, RuntimeArray) {
  // This test verifies that the pass will correctly instrument shader
  // with runtime descriptor array. This test was created by editing the
  // SPIR-V from the Simple test.

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
OpCapability RuntimeDescriptorArray
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %PerViewConstantBuffer_t "PerViewConstantBuffer_t"
OpMemberName %PerViewConstantBuffer_t 0 "g_nDataIdx"
OpName %_ ""
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 1
OpDecorate %g_tColor Binding 2
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %g_sAniso DescriptorSet 1
OpDecorate %g_sAniso Binding 3
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%20 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%_rarr_20 = OpTypeRuntimeArray %20
%_ptr_UniformConstant__arr_20 = OpTypePointer UniformConstant %_rarr_20
%g_tColor = OpVariable %_ptr_UniformConstant__arr_20 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_20 = OpTypePointer UniformConstant %20
%35 = OpTypeSampler
%_ptr_UniformConstant_35 = OpTypePointer UniformConstant %35
%g_sAniso = OpVariable %_ptr_UniformConstant_35 UniformConstant
%39 = OpTypeSampledImage %20
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%MainPs = OpFunction %void None %3
%5 = OpLabel
%53 = OpLoad %v2float %i_vTextureCoords
%63 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%64 = OpLoad %uint %63
%65 = OpAccessChain %_ptr_UniformConstant_20 %g_tColor %64
%66 = OpLoad %20 %65
%67 = OpLoad %35 %g_sAniso
%68 = OpSampledImage %39 %66 %67
%71 = OpImageSampleImplicitLod %v4float %68 %53
OpStore %_entryPointOutput_vColor %71
;CHECK-NOT: %71 = OpImageSampleImplicitLod %v4float %68 %53
;CHECK-NOT: OpStore %_entryPointOutput_vColor %71
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: [[check_result:%\w+]] = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_60 {{%\w+}} %uint_1 %uint_2 %32 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %16 %33
;CHECK: {{%\w+}} = OpSampledImage %26 {{%\w+}} %35
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} %30
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result_1:%\w+]] = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %_entryPointOutput_vColor [[phi_result_1]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstrumentInitCheckOnScalarDescriptor) {
  // This test verifies that the pass will correctly instrument vanilla
  // texture sample on a scalar descriptor with an initialization check if the
  // input_init_enable argument is set to true. This can happen when the
  // descriptor indexing extension is enabled in the API but the SPIR-V
  // does not have the extension enabled because it does not contain a
  // runtime array. This is the same shader as NoInstrumentNonBindless.

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 1
OpDecorate %g_tColor Binding 2
OpDecorate %g_sAniso DescriptorSet 1
OpDecorate %g_sAniso Binding 2
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%12 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%g_tColor = OpVariable %_ptr_UniformConstant_12 UniformConstant
%14 = OpTypeSampler
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
%g_sAniso = OpVariable %_ptr_UniformConstant_14 UniformConstant
%16 = OpTypeSampledImage %12
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%MainPs = OpFunction %void None %8
%19 = OpLabel
%20 = OpLoad %v2float %i_vTextureCoords
%21 = OpLoad %12 %g_tColor
%22 = OpLoad %14 %g_sAniso
%23 = OpSampledImage %16 %21 %22
%24 = OpImageSampleImplicitLod %v4float %23 %20
OpStore %_entryPointOutput_vColor %24
;CHECK-NOT: %24 = OpImageSampleImplicitLod %v4float %23 %20
;CHECK-NOT: OpStore %_entryPointOutput_vColor %24
;CHECK: [[check_result:%\w+]] = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_40 {{%\w+}} %uint_1 %uint_2 %uint_0 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional [[check_result]] {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %12 %g_tColor
;CHECK: {{%\w+}} = OpSampledImage %16 {{%\w+}} %22
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %_entryPointOutput_vColor [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, SPV14AddToEntryPoint) {
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_EXT_descriptor_indexing"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %foo "foo" %gid %image_var %sampler_var
;CHECK: OpEntryPoint Fragment {{%\w+}} "foo" {{%\w+}} {{%\w+}} {{%\w+}} %gl_FragCoord
OpExecutionMode %foo OriginUpperLeft
OpDecorate %image_var DescriptorSet 4
OpDecorate %image_var Binding 1
OpDecorate %sampler_var DescriptorSet 4
OpDecorate %sampler_var Binding 2
OpDecorate %gid DescriptorSet 0
OpDecorate %gid Binding 2
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%v3int = OpTypeVector %int 3
%float = OpTypeFloat 32
%v3float = OpTypeVector %float 3
%v4float = OpTypeVector %float 4
%struct = OpTypeStruct %v3int
%ptr_ssbo_struct = OpTypePointer StorageBuffer %struct
%ptr_ssbo_v3int = OpTypePointer StorageBuffer %v3int
%gid = OpVariable %ptr_ssbo_struct StorageBuffer
%image = OpTypeImage %float 3D 0 0 0 1 Unknown
%ptr_uc_image = OpTypePointer UniformConstant %image
%sampler = OpTypeSampler
%ptr_uc_sampler = OpTypePointer UniformConstant %sampler
%image_var = OpVariable %ptr_uc_image UniformConstant
%sampler_var = OpVariable %ptr_uc_sampler UniformConstant
%sampled = OpTypeSampledImage %image
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
%ld_image = OpLoad %image %image_var
%ld_sampler = OpLoad %sampler %sampler_var
%gep = OpAccessChain %ptr_ssbo_v3int %gid %int_0
%ld_gid = OpLoad %v3int %gep
%convert = OpConvertUToF %v3float %ld_gid
%sampled_image = OpSampledImage %sampled %ld_image %ld_sampler
%sample = OpImageSampleImplicitLod %v4float %sampled_image %convert
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_1_SPIRV_1_4);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, SPV14AddToEntryPoints) {
  const std::string text = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %foo "foo" %gid %image_var %sampler_var
;CHECK: OpEntryPoint Fragment {{%\w+}} "foo" {{%\w+}} {{%\w+}} {{%\w+}} %gl_FragCoord
OpEntryPoint Fragment %foo "bar" %gid %image_var %sampler_var
;CHECK: OpEntryPoint Fragment {{%\w+}} "bar" {{%\w+}} {{%\w+}} {{%\w+}} %gl_FragCoord
OpExecutionMode %foo OriginUpperLeft
OpDecorate %image_var DescriptorSet 3
OpDecorate %image_var Binding 2
OpDecorate %sampler_var DescriptorSet 3
OpDecorate %sampler_var Binding 3
OpDecorate %gid DescriptorSet 3
OpDecorate %gid Binding 4
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%v3int = OpTypeVector %int 3
%float = OpTypeFloat 32
%v3float = OpTypeVector %float 3
%v4float = OpTypeVector %float 4
%struct = OpTypeStruct %v3int
%ptr_ssbo_struct = OpTypePointer StorageBuffer %struct
%ptr_ssbo_v3int = OpTypePointer StorageBuffer %v3int
%gid = OpVariable %ptr_ssbo_struct StorageBuffer
%image = OpTypeImage %float 3D 0 0 0 1 Unknown
%ptr_uc_image = OpTypePointer UniformConstant %image
%sampler = OpTypeSampler
%ptr_uc_sampler = OpTypePointer UniformConstant %sampler
%image_var = OpVariable %ptr_uc_image UniformConstant
%sampler_var = OpVariable %ptr_uc_sampler UniformConstant
%sampled = OpTypeSampledImage %image
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
%ld_image = OpLoad %image %image_var
%ld_sampler = OpLoad %sampler %sampler_var
%gep = OpAccessChain %ptr_ssbo_v3int %gid %int_0
%ld_gid = OpLoad %v3int %gep
%convert = OpConvertUToF %v3float %ld_gid
%sampled_image = OpSampledImage %sampled %ld_image %ld_sampler
%sample = OpImageSampleImplicitLod %v4float %sampled_image %convert
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_1_SPIRV_1_4);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, InstBoundsAndInitLoadUnsizedUBOArray) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location=0) in nonuniformEXT flat int nu_ii;
  // layout(location=0) out float b;
  //
  // layout(set = 6, binding=3)  uniform uname { float a; }  uniformBuffer[];
  //
  // void main()
  // {
  //     b = uniformBuffer[nu_ii].a;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability UniformBufferArrayNonUniformIndexing
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii
;CHECK: OpEntryPoint Fragment %main "main" %b %nu_ii %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpName %main "main"
OpName %b "b"
OpName %uname "uname"
OpMemberName %uname 0 "a"
OpName %uniformBuffer "uniformBuffer"
OpName %nu_ii "nu_ii"
OpDecorate %b Location 0
OpMemberDecorate %uname 0 Offset 0
OpDecorate %uname Block
OpDecorate %uniformBuffer DescriptorSet 6
OpDecorate %uniformBuffer Binding 3
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %16 NonUniform
OpDecorate %20 NonUniform
;CHECK: OpDecorate {{%\w+}} NonUniform
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
;CHECK: OpDecorate {{%\w+}} NonUniform
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
%b = OpVariable %_ptr_Output_float Output
%uname = OpTypeStruct %float
%_runtimearr_uname = OpTypeRuntimeArray %uname
%_ptr_Uniform__runtimearr_uname = OpTypePointer Uniform %_runtimearr_uname
%uniformBuffer = OpVariable %_ptr_Uniform__runtimearr_uname Uniform
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%nu_ii = OpVariable %_ptr_Input_int Input
%int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: %v4uint = OpTypeVector %uint 4
;CHECK: %v4float = OpTypeVector %float 4
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_float:%\w+]] = OpConstantNull %float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%16 = OpLoad %int %nu_ii
%19 = OpAccessChain %_ptr_Uniform_float %uniformBuffer %16 %int_0
%20 = OpLoad %float %19
OpStore %b %20
;CHECK-NOT: %20 = OpLoad %float %19
;CHECK-NOT: OpStore %b %20
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpBitcast %uint %7
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_46 {{%\w+}} %uint_6 %uint_3 {{%\w+}} {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %float %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %float {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: OpStore %b [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstBoundsAndInitLoadUnsizedSSBOArrayDeprecated) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location=0) in nonuniformEXT flat int nu_ii;
  // layout(location=0) out float b;
  //
  // layout(set = 7, binding=3)  buffer bname { float b; }  storageBuffer[];
  //
  // void main()
  // {
  //     b = storageBuffer[nu_ii].b;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability StorageBufferArrayNonUniformIndexing
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii
;CHECK: OpEntryPoint Fragment %main "main" %b %nu_ii %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpName %main "main"
OpName %b "b"
OpName %bname "bname"
OpMemberName %bname 0 "a"
OpName %storageBuffer "storageBuffer"
OpName %nu_ii "nu_ii"
OpDecorate %b Location 0
OpMemberDecorate %bname 0 Offset 0
OpDecorate %bname Block
OpDecorate %storageBuffer DescriptorSet 7
OpDecorate %storageBuffer Binding 3
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %16 NonUniform
OpDecorate %20 NonUniform
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
%b = OpVariable %_ptr_Output_float Output
%bname = OpTypeStruct %float
%_runtimearr_bname = OpTypeRuntimeArray %bname
%_ptr_StorageBuffer__runtimearr_bname = OpTypePointer StorageBuffer %_runtimearr_bname
%storageBuffer = OpVariable %_ptr_StorageBuffer__runtimearr_bname StorageBuffer
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%nu_ii = OpVariable %_ptr_Input_int Input
%int_0 = OpConstant %int 0
%_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
;CHECK: %uint = OpTypeInt 32 0
;CHECK: %v4uint = OpTypeVector %uint 4
;CHECK: %v4float = OpTypeVector %float 4
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_float:%\w+]] = OpConstantNull %float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%16 = OpLoad %int %nu_ii
%19 = OpAccessChain %_ptr_StorageBuffer_float %storageBuffer %16 %int_0
%20 = OpLoad %float %19
OpStore %b %20
;CHECK-NOT: %20 = OpLoad %float %19
;CHECK-NOT: OpStore %b %20
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpBitcast %uint %7
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_46 {{%\w+}} %uint_7 %uint_3 {{%\w+}} {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %float %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %float {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: OpStore %b [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstBoundsAndInitLoadUnsizedSSBOArray) {
  // Same as Deprecated but declaring as StorageBuffer Block

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability StorageBufferArrayNonUniformIndexing
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii
;CHECK: OpEntryPoint Fragment %main "main" %b %nu_ii %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpName %main "main"
OpName %b "b"
OpName %bname "bname"
OpMemberName %bname 0 "a"
OpName %storageBuffer "storageBuffer"
OpName %nu_ii "nu_ii"
OpDecorate %b Location 0
OpMemberDecorate %bname 0 Offset 0
OpDecorate %bname Block
OpDecorate %storageBuffer DescriptorSet 0
OpDecorate %storageBuffer Binding 3
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %16 NonUniform
OpDecorate %20 NonUniform
;CHECK: OpDecorate {{%\w+}} NonUniform
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
;CHECK: OpDecorate {{%\w+}} NonUniform
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
%b = OpVariable %_ptr_Output_float Output
%bname = OpTypeStruct %float
%_runtimearr_bname = OpTypeRuntimeArray %bname
%_ptr_StorageBuffer__runtimearr_bname = OpTypePointer StorageBuffer %_runtimearr_bname
%storageBuffer = OpVariable %_ptr_StorageBuffer__runtimearr_bname StorageBuffer
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%nu_ii = OpVariable %_ptr_Input_int Input
%int_0 = OpConstant %int 0
%_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
;CHECK: %v4uint = OpTypeVector %uint 4
;CHECK: %v4float = OpTypeVector %float 4
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_float:%\w+]] = OpConstantNull %float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%16 = OpLoad %int %nu_ii
%19 = OpAccessChain %_ptr_StorageBuffer_float %storageBuffer %16 %int_0
%20 = OpLoad %float %19
OpStore %b %20
;CHECK-NOT: %20 = OpLoad %float %19
;CHECK-NOT: OpStore %b %20
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpBitcast %uint %7
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_46 {{%\w+}} %uint_0 %uint_3 {{%\w+}} {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %float %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %float {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: OpStore %b {{%\w+}}
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstInitLoadUBOScalar) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location=0) out float b;
  // layout(set=7, binding=3)  uniform uname { float a; }  uniformBuffer;
  //
  // void main()
  // {
  //     b = uniformBuffer.a;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b
;CHECK: OpEntryPoint Fragment %main "main" %b %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpName %main "main"
OpName %b "b"
OpName %uname "uname"
OpMemberName %uname 0 "a"
OpName %uniformBuffer "uniformBuffer"
OpDecorate %b Location 0
OpMemberDecorate %uname 0 Offset 0
OpDecorate %uname Block
OpDecorate %uniformBuffer DescriptorSet 7
OpDecorate %uniformBuffer Binding 3
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
%b = OpVariable %_ptr_Output_float Output
%uname = OpTypeStruct %float
%_ptr_Uniform_uname = OpTypePointer Uniform %uname
%uniformBuffer = OpVariable %_ptr_Uniform_uname Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: %int = OpTypeInt 32 1
;CHECK: %_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: %uint = OpTypeInt 32 0
;CHECK: %v4uint = OpTypeVector %uint 4
;CHECK: %v4float = OpTypeVector %float 4
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_float:%\w+]] = OpConstantNull %float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%15 = OpAccessChain %_ptr_Uniform_float %uniformBuffer %int_0
%16 = OpLoad %float %15
OpStore %b %16
;CHECK-NOT: %16 = OpLoad %float %15
;CHECK-NOT: OpStore %b %16
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: [[check_result:%\w+]] = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_33 {{%\w+}} %uint_7 %uint_3 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional [[check_result]] {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %float %15
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %float {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: OpStore %b [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstBoundsInitStoreUnsizedSSBOArray) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location=0) in nonuniformEXT flat int nu_ii;
  // layout(location=1) in float b;
  //
  // layout(set=5, binding=4)  buffer bname { float b; }  storageBuffer[];
  //
  // void main()
  // {
  //     storageBuffer[nu_ii].b = b;
  // }

  // clang-format off
  const std::string defs = R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability StorageBufferArrayNonUniformIndexing
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %nu_ii %b
;CHECK: OpEntryPoint Fragment %main "main" %nu_ii %b %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpName %main "main"
OpName %bname "bname"
OpMemberName %bname 0 "b"
OpName %storageBuffer "storageBuffer"
OpName %nu_ii "nu_ii"
OpName %b "b"
OpMemberDecorate %bname 0 Offset 0
OpDecorate %bname BufferBlock
OpDecorate %storageBuffer DescriptorSet 5
OpDecorate %storageBuffer Binding 4
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %14 NonUniform
OpDecorate %b Location 1
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%bname = OpTypeStruct %float
%_runtimearr_bname = OpTypeRuntimeArray %bname
%_ptr_Uniform__runtimearr_bname = OpTypePointer Uniform %_runtimearr_bname
%storageBuffer = OpVariable %_ptr_Uniform__runtimearr_bname Uniform
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%nu_ii = OpVariable %_ptr_Input_int Input
%int_0 = OpConstant %int 0
%_ptr_Input_float = OpTypePointer Input %float
%b = OpVariable %_ptr_Input_float Input
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: %v4uint = OpTypeVector %uint 4
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%14 = OpLoad %int %nu_ii
%18 = OpLoad %float %b
%20 = OpAccessChain %_ptr_Uniform_float %storageBuffer %14 %int_0
OpStore %20 %18
;CHECK-NOT: OpStore %20 %18
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpBitcast %uint %7
;CHECK: [[check_result:%\w+]] = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_46 {{%\w+}} %uint_5 %uint_4 {{%\w+}} {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional [[check_result]] {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore %20 %19
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstBoundsInitLoadSizedUBOArray) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location=0) in nonuniformEXT flat int nu_ii;
  // layout(location=0) out float b;
  //
  // layout(set=1, binding=3)  uniform uname { float a; }  uniformBuffer[128];
  //
  // void main()
  // {
  //     b = uniformBuffer[nu_ii].a;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
OpCapability ShaderNonUniform
OpCapability UniformBufferArrayNonUniformIndexing
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii
;CHECK: OpEntryPoint Fragment %main "main" %b %nu_ii %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpName %main "main"
OpName %b "b"
OpName %uname "uname"
OpMemberName %uname 0 "a"
OpName %uniformBuffer "uniformBuffer"
OpName %nu_ii "nu_ii"
OpDecorate %b Location 0
OpMemberDecorate %uname 0 Offset 0
OpDecorate %uname Block
OpDecorate %uniformBuffer DescriptorSet 1
OpDecorate %uniformBuffer Binding 3
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %18 NonUniform
OpDecorate %22 NonUniform
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
;CHECK: OpDecorate [[load_result:%\w+]] NonUniform
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
%b = OpVariable %_ptr_Output_float Output
%uname = OpTypeStruct %float
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%_arr_uname_uint_128 = OpTypeArray %uname %uint_128
%_ptr_Uniform__arr_uname_uint_128 = OpTypePointer Uniform %_arr_uname_uint_128
%uniformBuffer = OpVariable %_ptr_Uniform__arr_uname_uint_128 Uniform
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%nu_ii = OpVariable %_ptr_Input_int Input
%int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_float:%\w+]] = OpConstantNull %float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%18 = OpLoad %int %nu_ii
%21 = OpAccessChain %_ptr_Uniform_float %uniformBuffer %18 %int_0
%22 = OpLoad %float %21
OpStore %b %22
;CHECK-NOT: %22 = OpLoad %float %21
;CHECK-NOT: OpStore %b %22
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpBitcast %uint %7
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_47 {{%\w+}} %uint_1 %uint_3 {{%\w+}} {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %float %22
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %float {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: OpStore %b {{%\w+}}
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest,
       InstBoundsComputeShaderInitLoadVariableSizedSampledImagesArray) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout (local_size_x = 1, local_size_y = 1) in;
  //
  // layout(set = 2, binding = 0, std140) buffer Input {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 2, binding = 1, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability Shader
OpCapability RuntimeDescriptorArray
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
;CHECK: OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
OpExecutionMode %main LocalSize 1 1 1
OpSource GLSL 450
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpName %main "main"
OpName %Input "Input"
OpMemberName %Input 0 "index"
OpMemberName %Input 1 "red"
OpName %sbo "sbo"
OpName %images "images"
OpMemberDecorate %Input 0 Offset 0
OpMemberDecorate %Input 1 Offset 4
OpDecorate %Input BufferBlock
OpDecorate %sbo DescriptorSet 2
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 2
OpDecorate %images Binding 1
OpDecorate %images NonWritable
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%Input = OpTypeStruct %uint %float
%_ptr_Uniform_Input = OpTypePointer Uniform %Input
%sbo = OpVariable %_ptr_Uniform_Input Uniform
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%13 = OpTypeImage %float 2D 0 0 0 2 Rgba32f
%_runtimearr_13 = OpTypeRuntimeArray %13
%_ptr_UniformConstant__runtimearr_13 = OpTypePointer UniformConstant %_runtimearr_13
%images = OpVariable %_ptr_UniformConstant__runtimearr_13 UniformConstant
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%v2int = OpTypeVector %int 2
%25 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: %v3uint = OpTypeVector %uint 3
;CHECK: %_ptr_Input_v3uint = OpTypePointer Input %v3uint
;CHECK: %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
;CHECK: [[null_uint:%\w+]] = OpConstantNull %uint
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%19 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%20 = OpLoad %uint %19
%22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
%23 = OpLoad %13 %22
%27 = OpImageRead %v4float %23 %25
%29 = OpCompositeExtract %float %27 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
OpStore %31 %29
;CHECK-NOT: OpStore %31 %29
;CHECK: {{%\w+}} = OpLoad %v3uint %gl_GlobalInvocationID
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_48 {{%\w+}} %uint_2 %uint_0 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %uint %25
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %uint {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpAccessChain %_ptr_UniformConstant_13 %images {{%\w+}}
;CHECK: {{%\w+}} = OpLoad %13 {{%\w+}}
;CHECK: {{%\w+}} = OpLoad %v3uint %gl_GlobalInvocationID
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_51 {{%\w+}} %uint_2 %uint_1 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %13 {{%\w+}}
;CHECK: {{%\w+}} = OpImageRead %v4float {{%\w+}} %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %float {{%\w+}} 0
;CHECK: {{%\w+}} = OpAccessChain %_ptr_Uniform_float %sbo %int_1
;CHECK: {{%\w+}} = OpLoad %v3uint %gl_GlobalInvocationID
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_54 {{%\w+}} %uint_2 %uint_0 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore %31 {{%\w+}}
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest,
       InstBoundsRayGenerationInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 3, binding = 1, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 3, binding = 5, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint RayGenerationNV %main "main"
;CHECK: OpEntryPoint RayGenerationNV %main "main" [[launch_id:%\w+]]
OpSource GLSL 460
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpSourceExtension "GL_NV_ray_tracing"
OpName %main "main"
OpName %StorageBuffer "StorageBuffer"
OpMemberName %StorageBuffer 0 "index"
OpMemberName %StorageBuffer 1 "red"
OpName %sbo "sbo"
OpName %images "images"
OpMemberDecorate %StorageBuffer 0 Offset 0
OpMemberDecorate %StorageBuffer 1 Offset 4
OpDecorate %StorageBuffer BufferBlock
OpDecorate %sbo DescriptorSet 3
OpDecorate %sbo Binding 1
OpDecorate %images DescriptorSet 3
OpDecorate %images Binding 5
OpDecorate %images NonWritable
)" + kImportDeco + R"(
;CHECK: OpDecorate [[launch_id]] BuiltIn LaunchIdNV
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%StorageBuffer = OpTypeStruct %uint %float
%_ptr_Uniform_StorageBuffer = OpTypePointer Uniform %StorageBuffer
%sbo = OpVariable %_ptr_Uniform_StorageBuffer Uniform
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%13 = OpTypeImage %float 2D 0 0 0 2 Rgba32f
%_runtimearr_13 = OpTypeRuntimeArray %13
%_ptr_UniformConstant__runtimearr_13 = OpTypePointer UniformConstant %_runtimearr_13
%images = OpVariable %_ptr_UniformConstant__runtimearr_13 UniformConstant
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%v2int = OpTypeVector %int 2
%25 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: [[null_uint:%\w+]] = OpConstantNull %uint
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%19 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%20 = OpLoad %uint %19
%22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
%23 = OpLoad %13 %22
%27 = OpImageRead %v4float %23 %25
%29 = OpCompositeExtract %float %27 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
OpStore %31 %29
;CHECK-NOT: OpStore %31 %29
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5313 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_49 {{%\w+}} %uint_3 %uint_1 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %uint %25
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %uint {{%\w+}} {{%\w+}} [[null_uint]] {{%\w+}}
;CHECK: {{%\w+}} = OpAccessChain %_ptr_UniformConstant_13 %images {{%\w+}}
;CHECK: {{%\w+}} = OpLoad %13 {{%\w+}}
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5313 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_52 {{%\w+}} %uint_3 %uint_5 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %13 {{%\w+}}
;CHECK: {{%\w+}} = OpImageRead %v4float {{%\w+}} %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %float {{%\w+}} 0
;CHECK: {{%\w+}} = OpAccessChain %_ptr_Uniform_float %sbo %int_1
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5313 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_55 {{%\w+}} %uint_3 %uint_1 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore {{%\w+}} {{%\w+}}
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest,
       InstBoundsIntersectionInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 5, binding = 1, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 5, binding = 3, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint IntersectionNV %main "main"
;CHECK: OpEntryPoint IntersectionNV %main "main" [[launch_id:%\w+]]
OpSource GLSL 460
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpSourceExtension "GL_NV_ray_tracing"
OpName %main "main"
OpName %StorageBuffer "StorageBuffer"
OpMemberName %StorageBuffer 0 "index"
OpMemberName %StorageBuffer 1 "red"
OpName %sbo "sbo"
OpName %images "images"
OpMemberDecorate %StorageBuffer 0 Offset 0
OpMemberDecorate %StorageBuffer 1 Offset 4
OpDecorate %StorageBuffer BufferBlock
OpDecorate %sbo DescriptorSet 5
OpDecorate %sbo Binding 1
OpDecorate %images DescriptorSet 5
OpDecorate %images Binding 3
OpDecorate %images NonWritable
)" + kImportDeco + R"(
;CHECK: OpDecorate [[launch_id]] BuiltIn LaunchIdNV
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%StorageBuffer = OpTypeStruct %uint %float
%_ptr_Uniform_StorageBuffer = OpTypePointer Uniform %StorageBuffer
%sbo = OpVariable %_ptr_Uniform_StorageBuffer Uniform
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%13 = OpTypeImage %float 2D 0 0 0 2 Rgba32f
%_runtimearr_13 = OpTypeRuntimeArray %13
%_ptr_UniformConstant__runtimearr_13 = OpTypePointer UniformConstant %_runtimearr_13
%images = OpVariable %_ptr_UniformConstant__runtimearr_13 UniformConstant
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%v2int = OpTypeVector %int 2
%25 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: [[launch_id]] = OpVariable %_ptr_Input_v3uint Input
;CHECK: [[null_uint:%\w+]] = OpConstantNull %uint
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%19 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%20 = OpLoad %uint %19
%22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
%23 = OpLoad %13 %22
%27 = OpImageRead %v4float %23 %25
%29 = OpCompositeExtract %float %27 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
OpStore %31 %29
;CHECK-NOT: OpStore %31 %29
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5314 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_49 {{%\w+}} %uint_5 %uint_1 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %uint %25
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %uint {{%\w+}} {{%\w+}} [[null_uint]] {{%\w+}}
;CHECK: {{%\w+}} = OpAccessChain %_ptr_UniformConstant_13 %images {{%\w+}}
;CHECK: {{%\w+}} = OpLoad %13 {{%\w+}}
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5314 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_52 {{%\w+}} %uint_5 %uint_3 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %13 {{%\w+}}
;CHECK: {{%\w+}} = OpImageRead %v4float {{%\w+}} %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %float {{%\w+}} 0
;CHECK: {{%\w+}} = OpAccessChain %_ptr_Uniform_float %sbo %int_1
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5314 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_55 {{%\w+}} %uint_5 %uint_1 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore %31 {{%\w+}}
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest,
       InstBoundsAnyHitInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 2, binding = 1, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 2, binding = 3, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint AnyHitNV %main "main"
;CHECK: OpEntryPoint AnyHitNV %main "main" [[launch_id:%\w+]]
OpSource GLSL 460
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpSourceExtension "GL_NV_ray_tracing"
OpName %main "main"
OpName %StorageBuffer "StorageBuffer"
OpMemberName %StorageBuffer 0 "index"
OpMemberName %StorageBuffer 1 "red"
OpName %sbo "sbo"
OpName %images "images"
OpMemberDecorate %StorageBuffer 0 Offset 0
OpMemberDecorate %StorageBuffer 1 Offset 4
OpDecorate %StorageBuffer BufferBlock
OpDecorate %sbo DescriptorSet 2
OpDecorate %sbo Binding 1
OpDecorate %images DescriptorSet 2
OpDecorate %images Binding 3
OpDecorate %images NonWritable
)" + kImportDeco + R"(
;CHECK: OpDecorate [[launch_id]] BuiltIn LaunchIdNV
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%StorageBuffer = OpTypeStruct %uint %float
%_ptr_Uniform_StorageBuffer = OpTypePointer Uniform %StorageBuffer
%sbo = OpVariable %_ptr_Uniform_StorageBuffer Uniform
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%13 = OpTypeImage %float 2D 0 0 0 2 Rgba32f
%_runtimearr_13 = OpTypeRuntimeArray %13
%_ptr_UniformConstant__runtimearr_13 = OpTypePointer UniformConstant %_runtimearr_13
%images = OpVariable %_ptr_UniformConstant__runtimearr_13 UniformConstant
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%v2int = OpTypeVector %int 2
%25 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: [[launch_id]] = OpVariable %_ptr_Input_v3uint Input
;CHECK: [[null_uint:%\w+]] = OpConstantNull %uint
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%19 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%20 = OpLoad %uint %19
%22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
%23 = OpLoad %13 %22
%27 = OpImageRead %v4float %23 %25
%29 = OpCompositeExtract %float %27 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
OpStore %31 %29
;CHECK-NOT: %20 = OpLoad %uint %19
;CHECK-NOT: %22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
;CHECK-NOT: %23 = OpLoad %13 %22
;CHECK-NOT: %27 = OpImageRead %v4float %23 %25
;CHECK-NOT: %29 = OpCompositeExtract %float %27 0
;CHECK-NOT: %31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
;CHECK-NOT: OpStore %31 %29
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5315 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_49 {{%\w+}} %uint_2 %uint_1 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %uint %25
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %uint {{%\w+}} {{%\w+}} [[null_uint]] {{%\w+}}
;CHECK: %27 = OpAccessChain %_ptr_UniformConstant_13 %images [[phi_result]]
;CHECK: %28 = OpLoad %13 %27
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5315 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_52 {{%\w+}} %uint_2 %uint_3 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %13 %27
;CHECK: {{%\w+}} = OpImageRead %v4float {{%\w+}} %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: %30 = OpCompositeExtract %float {{%\w+}} 0
;CHECK: %31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5315 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_55 {{%\w+}} %uint_2 %uint_1 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore %31 %30
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest,
       InstBoundsClosestHitInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 1, binding = 2, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 1, binding = 3, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint ClosestHitNV %main "main"
;CHECK: OpEntryPoint ClosestHitNV %main "main" [[launch_id:%\w+]]
OpSource GLSL 460
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpSourceExtension "GL_NV_ray_tracing"
OpName %main "main"
OpName %StorageBuffer "StorageBuffer"
OpMemberName %StorageBuffer 0 "index"
OpMemberName %StorageBuffer 1 "red"
OpName %sbo "sbo"
OpName %images "images"
OpMemberDecorate %StorageBuffer 0 Offset 0
OpMemberDecorate %StorageBuffer 1 Offset 4
OpDecorate %StorageBuffer BufferBlock
OpDecorate %sbo DescriptorSet 1
OpDecorate %sbo Binding 2
OpDecorate %images DescriptorSet 1
OpDecorate %images Binding 3
OpDecorate %images NonWritable
)" + kImportDeco + R"(
;CHECK: OpDecorate [[launch_id]] BuiltIn LaunchIdNV
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%StorageBuffer = OpTypeStruct %uint %float
%_ptr_Uniform_StorageBuffer = OpTypePointer Uniform %StorageBuffer
%sbo = OpVariable %_ptr_Uniform_StorageBuffer Uniform
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%13 = OpTypeImage %float 2D 0 0 0 2 Rgba32f
%_runtimearr_13 = OpTypeRuntimeArray %13
%_ptr_UniformConstant__runtimearr_13 = OpTypePointer UniformConstant %_runtimearr_13
%images = OpVariable %_ptr_UniformConstant__runtimearr_13 UniformConstant
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%v2int = OpTypeVector %int 2
%25 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: [[launch_id]] = OpVariable %_ptr_Input_v3uint Input
;CHECK: [[null_uint:%\w+]] = OpConstantNull %uint
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%19 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%20 = OpLoad %uint %19
%22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
%23 = OpLoad %13 %22
%27 = OpImageRead %v4float %23 %25
%29 = OpCompositeExtract %float %27 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
OpStore %31 %29
;CHECK-NOT: %20 = OpLoad %uint %19
;CHECK-NOT: %22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
;CHECK-NOT: %23 = OpLoad %13 %22
;CHECK-NOT: %27 = OpImageRead %v4float %23 %25
;CHECK-NOT: %29 = OpCompositeExtract %float %27 0
;CHECK-NOT: %31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
;CHECK-NOT: OpStore %31 %29
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5316 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_49 {{%\w+}} %uint_1 %uint_2 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %uint %25
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %uint {{%\w+}} {{%\w+}} [[null_uint]] {{%\w+}}
;CHECK: %27 = OpAccessChain %_ptr_UniformConstant_13 %images [[phi_result]]
;CHECK: %28 = OpLoad %13 %27
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5316 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_52 {{%\w+}} %uint_1 %uint_3 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %13 %27
;CHECK: {{%\w+}} = OpImageRead %v4float {{%\w+}} %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: %30 = OpCompositeExtract %float {{%\w+}} 0
;CHECK: %31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5316 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_55 {{%\w+}} %uint_1 %uint_2 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore %31 %30
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest,
       InstBoundsMissInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 1, binding = 2, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 1, binding = 3, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint MissNV %main "main"
;CHECK: OpEntryPoint MissNV %main "main" [[launch_id:%\w+]]
OpSource GLSL 460
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpSourceExtension "GL_NV_ray_tracing"
OpName %main "main"
OpName %StorageBuffer "StorageBuffer"
OpMemberName %StorageBuffer 0 "index"
OpMemberName %StorageBuffer 1 "red"
OpName %sbo "sbo"
OpName %images "images"
OpMemberDecorate %StorageBuffer 0 Offset 0
OpMemberDecorate %StorageBuffer 1 Offset 4
OpDecorate %StorageBuffer BufferBlock
OpDecorate %sbo DescriptorSet 1
OpDecorate %sbo Binding 2
OpDecorate %images DescriptorSet 1
OpDecorate %images Binding 3
OpDecorate %images NonWritable
)" + kImportDeco + R"(
;CHECK: OpDecorate [[launch_id]] BuiltIn LaunchIdNV
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%StorageBuffer = OpTypeStruct %uint %float
%_ptr_Uniform_StorageBuffer = OpTypePointer Uniform %StorageBuffer
%sbo = OpVariable %_ptr_Uniform_StorageBuffer Uniform
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%13 = OpTypeImage %float 2D 0 0 0 2 Rgba32f
%_runtimearr_13 = OpTypeRuntimeArray %13
%_ptr_UniformConstant__runtimearr_13 = OpTypePointer UniformConstant %_runtimearr_13
%images = OpVariable %_ptr_UniformConstant__runtimearr_13 UniformConstant
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%v2int = OpTypeVector %int 2
%25 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: [[launch_id]] = OpVariable %_ptr_Input_v3uint Input
;CHECK: [[null_uint:%\w+]] = OpConstantNull %uint
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%19 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%20 = OpLoad %uint %19
%22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
%23 = OpLoad %13 %22
%27 = OpImageRead %v4float %23 %25
%29 = OpCompositeExtract %float %27 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
OpStore %31 %29
;CHECK-NOT: %20 = OpLoad %uint %19
;CHECK-NOT: %22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
;CHECK-NOT: %27 = OpImageRead %v4float %23 %25
;CHECK-NOT: %29 = OpCompositeExtract %float %27 0
;CHECK-NOT OpStore %31 %29
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5317 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_49 {{%\w+}} %uint_1 %uint_2 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %uint %25
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %uint {{%\w+}} {{%\w+}} [[null_uint]] {{%\w+}}
;CHECK: %27 = OpAccessChain %_ptr_UniformConstant_13 %images [[phi_result]]
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5317 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_52 {{%\w+}} %uint_1 %uint_3 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %13 %27
;CHECK: {{%\w+}} = OpImageRead %v4float {{%\w+}} %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: %30 = OpCompositeExtract %float {{%\w+}} 0
;CHECK: %31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5317 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_55 {{%\w+}} %uint_1 %uint_2 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore %31 %30
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest,
       InstBoundsCallableInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 1, binding = 2, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 1, binding = 3, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  // clang-format off
  const std::string defs = R"(
OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint CallableNV %main "main"
;CHECK: OpEntryPoint CallableNV %main "main" [[launch_id:%\w+]]
OpSource GLSL 460
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpSourceExtension "GL_NV_ray_tracing"
OpName %main "main"
OpName %StorageBuffer "StorageBuffer"
OpMemberName %StorageBuffer 0 "index"
OpMemberName %StorageBuffer 1 "red"
OpName %sbo "sbo"
OpName %images "images"
OpMemberDecorate %StorageBuffer 0 Offset 0
OpMemberDecorate %StorageBuffer 1 Offset 4
OpDecorate %StorageBuffer BufferBlock
OpDecorate %sbo DescriptorSet 1
OpDecorate %sbo Binding 2
OpDecorate %images DescriptorSet 1
OpDecorate %images Binding 3
OpDecorate %images NonWritable
)" + kImportDeco + R"(
;CHECK: OpDecorate [[launch_id]] BuiltIn LaunchIdNV
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%StorageBuffer = OpTypeStruct %uint %float
%_ptr_Uniform_StorageBuffer = OpTypePointer Uniform %StorageBuffer
%sbo = OpVariable %_ptr_Uniform_StorageBuffer Uniform
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%13 = OpTypeImage %float 2D 0 0 0 2 Rgba32f
%_runtimearr_13 = OpTypeRuntimeArray %13
%_ptr_UniformConstant__runtimearr_13 = OpTypePointer UniformConstant %_runtimearr_13
%images = OpVariable %_ptr_UniformConstant__runtimearr_13 UniformConstant
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%v2int = OpTypeVector %int 2
%25 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
;CHECK: [[null_uint:%\w+]] = OpConstantNull %uint
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%19 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%20 = OpLoad %uint %19
%22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
%23 = OpLoad %13 %22
%27 = OpImageRead %v4float %23 %25
%29 = OpCompositeExtract %float %27 0
;CHECK-NOT: %20 = OpLoad %uint %19
;CHECK-NOT: %22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5318 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_49 {{%\w+}} %uint_1 %uint_2 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %uint %25
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %uint {{%\w+}} {{%\w+}} [[null_uint]] {{%\w+}}
;CHECK: %27 = OpAccessChain %_ptr_UniformConstant_13 %images {{%\w+}}
;CHECK-NOT: %23 = OpLoad %13 %22
;CHECK-NOT: %27 = OpImageRead %v4float %23 %25
;CHECK-NOT: %29 = OpCompositeExtract %float %27 0
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5318 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_52 {{%\w+}} %uint_1 %uint_3 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %13 %27
;CHECK: {{%\w+}} = OpImageRead %v4float {{%\w+}} %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: %30 = OpCompositeExtract %float {{%\w+}} 0
;CHECK: %31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
OpStore %31 %29
;CHECK-NOT: %31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
;CHECK-NOT: OpStore %31 %29
;CHECK: {{%\w+}} = OpLoad %v3uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5318 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_55 {{%\w+}} %uint_1 %uint_2 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore %31 %30
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpReturn
OpFunctionEnd
)";
  // clang-format on

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, InstBoundsInitSameBlockOpReplication) {
  // Test that same block ops like OpSampledImage are replicated properly
  // where needed.
  //
  // clang-format off
  //
  // #version 450 core
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location = 0) in vec2 inTexcoord;
  // layout(location = 0) out vec4 outColor;
  //
  // layout(set = 1, binding = 0) uniform Uniforms {
  //   vec2 var0;
  // } uniforms;
  //
  // layout(set = 1, binding = 1) uniform sampler uniformSampler;
  // layout(set = 1, binding = 2) uniform texture2D uniformTex;
  // layout(set = 1, binding = 3) uniform texture2D uniformTexArr[8];
  //
  // void main() {
  //   int index = 0;
  //   float x = texture(sampler2D(uniformTexArr[nonuniformEXT(index)], uniformSampler), inTexcoord.xy).x;
  //   float y = texture(sampler2D(uniformTex, uniformSampler), inTexcoord.xy * uniforms.var0.xy).x;
  //   outColor = vec4(x, y, 0.0, 0.0);
  // }
  //

  const std::string defs = R"(
OpCapability Shader
OpCapability ShaderNonUniformEXT
OpCapability SampledImageArrayNonUniformIndexingEXT
;CHECK: OpCapability Linkage
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %inTexcoord %outColor
;CHECK: OpEntryPoint Fragment %main "main" %inTexcoord %outColor %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_EXT_nonuniform_qualifier"
OpName %main "main"
OpName %index "index"
OpName %x "x"
OpName %uniformTexArr "uniformTexArr"
OpName %uniformSampler "uniformSampler"
OpName %inTexcoord "inTexcoord"
OpName %y "y"
OpName %uniformTex "uniformTex"
OpName %Uniforms "Uniforms"
OpMemberName %Uniforms 0 "var0"
OpName %uniforms "uniforms"
OpName %outColor "outColor"
OpDecorate %uniformTexArr DescriptorSet 1
OpDecorate %uniformTexArr Binding 3
OpDecorate %19 NonUniformEXT
OpDecorate %22 NonUniformEXT
OpDecorate %uniformSampler DescriptorSet 1
OpDecorate %uniformSampler Binding 1
OpDecorate %inTexcoord Location 0
OpDecorate %uniformTex DescriptorSet 1
OpDecorate %uniformTex Binding 2
OpMemberDecorate %Uniforms 0 Offset 0
OpDecorate %Uniforms Block
OpDecorate %uniforms DescriptorSet 1
OpDecorate %uniforms Binding 0
OpDecorate %outColor Location 0
;CHECK: OpDecorate {{%\w+}} NonUniform
;CHECK: OpDecorate {{%\w+}} NonUniform
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
;CHECK: OpDecorate [[desc_state_result:%\w+]] NonUniform
%void = OpTypeVoid
%3 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_13_uint_8 = OpTypeArray %13 %uint_8
%_ptr_UniformConstant__arr_13_uint_8 = OpTypePointer UniformConstant %_arr_13_uint_8
%uniformTexArr = OpVariable %_ptr_UniformConstant__arr_13_uint_8 UniformConstant
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%23 = OpTypeSampler
%_ptr_UniformConstant_23 = OpTypePointer UniformConstant %23
%uniformSampler = OpVariable %_ptr_UniformConstant_23 UniformConstant
%27 = OpTypeSampledImage %13
%v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%inTexcoord = OpVariable %_ptr_Input_v2float Input
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%uniformTex = OpVariable %_ptr_UniformConstant_13 UniformConstant
%Uniforms = OpTypeStruct %v2float
%_ptr_Uniform_Uniforms = OpTypePointer Uniform %Uniforms
%uniforms = OpVariable %_ptr_Uniform_Uniforms Uniform
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%outColor = OpVariable %_ptr_Output_v4float Output
%float_0 = OpConstant %float 0
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
;CHECK: [[null_v2float:%\w+]] = OpConstantNull %v2float
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%index = OpVariable %_ptr_Function_int Function
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
OpStore %index %int_0
%19 = OpLoad %int %index
%21 = OpAccessChain %_ptr_UniformConstant_13 %uniformTexArr %19
%22 = OpLoad %13 %21
%26 = OpLoad %23 %uniformSampler
%28 = OpSampledImage %27 %22 %26
%32 = OpLoad %v2float %inTexcoord
%34 = OpImageSampleImplicitLod %v4float %28 %32
%36 = OpCompositeExtract %float %34 0
;CHECK-NOT: %34 = OpImageSampleImplicitLod %v4float %28 %32
;CHECK-NOT: %36 = OpCompositeExtract %float %34 0
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpBitcast %uint %19
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_80 {{%\w+}} %uint_1 %uint_3 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %13 %21
;CHECK: {{%\w+}} = OpSampledImage %27 {{%\w+}} %26
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} %32
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
OpStore %x %36
%39 = OpLoad %13 %uniformTex
%40 = OpLoad %23 %uniformSampler
%41 = OpSampledImage %27 %39 %40
%42 = OpLoad %v2float %inTexcoord
%47 = OpAccessChain %_ptr_Uniform_v2float %uniforms %int_0
%48 = OpLoad %v2float %47
%49 = OpFMul %v2float %42 %48
;CHECK-NOT: %48 = OpLoad %v2float %47
;CHECK-NOT: %49 = OpFMul %v2float %42 %48
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_88 {{%\w+}} %uint_1 %uint_0 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %v2float %47
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %v2float {{%\w+}} {{%\w+}} [[null_v2float]] {{%\w+}}
;CHECK: %49 = OpFMul %v2float %42 [[phi_result]]
%50 = OpImageSampleImplicitLod %v4float %41 %49
%51 = OpCompositeExtract %float %50 0
OpStore %y %51
;CHECK-NOT: %50 = OpImageSampleImplicitLod %v4float %41 %49
;CHECK-NOT: %51 = OpCompositeExtract %float %50 0
;CHECK: {{%\w+}} = OpSampledImage %27 %39 %40
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_90 {{%\w+}} %uint_1 %uint_2 %uint_0 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %13 %uniformTex
;CHECK: {{%\w+}} = OpSampledImage %27 {{%\w+}} %40
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} %49
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: %51 = OpCompositeExtract %float {{%\w+}} 0
OpStore %y %51
%54 = OpLoad %float %x
%55 = OpLoad %float %y
%57 = OpCompositeConstruct %v4float %54 %55 %float_0 %float_0
OpStore %outColor %57
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(defs + kImportStub + main_func,
                                               true, 23u);
}

TEST_F(InstBindlessTest, MultipleUniformNonAggregateRefsNoDescInit) {
  // Check that uniform refs do not go out-of-bounds. All checks use same input
  // buffer read function call result at top of function for uniform buffer
  // length. Because descriptor indexing is not being checked, we can avoid one
  // buffer load.
  //
  // Texture2D g_tColor;
  // SamplerState g_sAniso;
  //
  // layout(push_constant) cbuffer PerViewPushConst_t { bool g_B; };
  //
  // cbuffer PerViewConstantBuffer_t {
  //   float2 g_TexOff0;
  //   float2 g_TexOff1;
  // };
  //
  // struct PS_INPUT {
  //   float2 vTextureCoords : TEXCOORD2;
  // };
  //
  // struct PS_OUTPUT {
  //   float4 vColor : SV_Target0;
  // };
  //
  // PS_OUTPUT MainPs(PS_INPUT i) {
  //   PS_OUTPUT ps_output;
  //   float2 off;
  //   float2 vtc;
  //   if (g_B)
  //     off = g_TexOff0;
  //   else
  //     off = g_TexOff1;
  //   vtc = i.vTextureCoords.xy + off;
  //   ps_output.vColor = g_tColor.Sample(g_sAniso, vtc);
  //   return ps_output;
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %PerViewPushConst_t "PerViewPushConst_t"
OpMemberName %PerViewPushConst_t 0 "g_B"
OpName %_ ""
OpName %PerViewConstantBuffer_t "PerViewConstantBuffer_t"
OpMemberName %PerViewConstantBuffer_t 0 "g_TexOff0"
OpMemberName %PerViewConstantBuffer_t 1 "g_TexOff1"
OpName %__0 ""
OpName %g_tColor "g_tColor"
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpMemberDecorate %PerViewPushConst_t 0 Offset 0
OpDecorate %PerViewPushConst_t Block
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpMemberDecorate %PerViewConstantBuffer_t 1 Offset 8
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %__0 DescriptorSet 0
OpDecorate %__0 Binding 1
OpDecorate %g_tColor DescriptorSet 0
OpDecorate %g_tColor Binding 0
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %g_sAniso Binding 2
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%PerViewPushConst_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewPushConst_t = OpTypePointer PushConstant %PerViewPushConst_t
%_ = OpVariable %_ptr_PushConstant_PerViewPushConst_t PushConstant
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%bool = OpTypeBool
%uint_0 = OpConstant %uint 0
%PerViewConstantBuffer_t = OpTypeStruct %v2float %v2float
%_ptr_Uniform_PerViewConstantBuffer_t = OpTypePointer Uniform %PerViewConstantBuffer_t
%__0 = OpVariable %_ptr_Uniform_PerViewConstantBuffer_t Uniform
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int_1 = OpConstant %int 1
%49 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_49 = OpTypePointer UniformConstant %49
%g_tColor = OpVariable %_ptr_UniformConstant_49 UniformConstant
%53 = OpTypeSampler
%_ptr_UniformConstant_53 = OpTypePointer UniformConstant %53
%g_sAniso = OpVariable %_ptr_UniformConstant_53 UniformConstant
%57 = OpTypeSampledImage %49
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %v4uint = OpTypeVector %uint 4
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v2float:%\w+]] = OpConstantNull %v2float
 )" + kImportStub + R"(
%MainPs = OpFunction %void None %3
%5 = OpLabel
%69 = OpLoad %v2float %i_vTextureCoords
%82 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%83 = OpLoad %uint %82
%84 = OpINotEqual %bool %83 %uint_0
OpSelectionMerge %91 None
OpBranchConditional %84 %85 %88
%85 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v2float %__0 %int_0
%87 = OpLoad %v2float %86
;CHECK-NOT:     %87 = OpLoad %v2float %86
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpIAdd %uint %uint_0 %uint_7
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: [[desc_state_result:%\w+]] = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_72 {{%\w+}} %uint_0 %uint_1 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional [[desc_state_result]] {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %v2float %86
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
 ;CHECK: {{%\w+}} = OpPhi %v2float {{%\w+}} {{%\w+}} [[null_v2float]] {{%\w+}}
OpBranch %91
%88 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v2float %__0 %int_1
%90 = OpLoad %v2float %89
;CHECK-NOT:     %90 = OpLoad %v2float %89
;CHECK: {{%\w+}} = OpIAdd %uint %uint_8 %uint_7
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_76 {{%\w+}} %uint_0 %uint_1 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %v2float %89
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v2float {{%\w+}} {{%\w+}} [[null_v2float]] {{%\w+}}
OpBranch %91
%91 = OpLabel
%115 = OpPhi %v2float %87 %85 %90 %88
;CHECK-NOT:       %115 = OpPhi %v2float %87 %85 %90 %88
;CHECK: %115 = OpPhi %v2float {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
%95 = OpFAdd %v2float %69 %115
%96 = OpLoad %49 %g_tColor
%97 = OpLoad %53 %g_sAniso
%98 = OpSampledImage %57 %96 %97
%100 = OpImageSampleImplicitLod %v4float %98 %95
OpStore %_entryPointOutput_vColor %100
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, UniformArrayRefNoDescInit) {
  // Check that uniform array ref does not go out-of-bounds.
  //
  // Texture2D g_tColor;
  // SamplerState g_sAniso;
  //
  // layout(push_constant) cbuffer PerViewPushConst_t { uint g_c; };
  //
  // struct PerBatchEnvMapConstantBuffer_t {
  //   float4x3 g_matEnvMapWorldToLocal;
  //   float4 g_vEnvironmentMapBoxMins;
  //   float2 g_TexOff;
  // };
  //
  // cbuffer _BindlessFastEnvMapCB_PS_t {
  //   PerBatchEnvMapConstantBuffer_t g_envMapConstants[128];
  // };
  //
  // struct PS_INPUT {
  //   float2 vTextureCoords : TEXCOORD2;
  // };
  //
  // struct PS_OUTPUT {
  //   float4 vColor : SV_Target0;
  // };
  //
  // PS_OUTPUT MainPs(PS_INPUT i) {
  //   PS_OUTPUT ps_output;
  //   float2 off;
  //   float2 vtc;
  //   off = g_envMapConstants[g_c].g_TexOff;
  //   vtc = i.vTextureCoords.xy + off;
  //   ps_output.vColor = g_tColor.Sample(g_sAniso, vtc);
  //   return ps_output;
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %PerBatchEnvMapConstantBuffer_t "PerBatchEnvMapConstantBuffer_t"
OpMemberName %PerBatchEnvMapConstantBuffer_t 0 "g_matEnvMapWorldToLocal"
OpMemberName %PerBatchEnvMapConstantBuffer_t 1 "g_vEnvironmentMapBoxMins"
OpMemberName %PerBatchEnvMapConstantBuffer_t 2 "g_TexOff"
OpName %_BindlessFastEnvMapCB_PS_t "_BindlessFastEnvMapCB_PS_t"
OpMemberName %_BindlessFastEnvMapCB_PS_t 0 "g_envMapConstants"
OpName %_ ""
OpName %PerViewPushConst_t "PerViewPushConst_t"
OpMemberName %PerViewPushConst_t 0 "g_c"
OpName %__0 ""
OpName %g_tColor "g_tColor"
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 0 RowMajor
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 0 Offset 0
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 0 MatrixStride 16
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 1 Offset 48
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 2 Offset 64
OpDecorate %_arr_PerBatchEnvMapConstantBuffer_t_uint_128 ArrayStride 80
OpMemberDecorate %_BindlessFastEnvMapCB_PS_t 0 Offset 0
OpDecorate %_BindlessFastEnvMapCB_PS_t Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 2
OpMemberDecorate %PerViewPushConst_t 0 Offset 0
OpDecorate %PerViewPushConst_t Block
OpDecorate %g_tColor DescriptorSet 0
OpDecorate %g_tColor Binding 0
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %g_sAniso Binding 1
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%PerBatchEnvMapConstantBuffer_t = OpTypeStruct %mat4v3float %v4float %v2float
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%_arr_PerBatchEnvMapConstantBuffer_t_uint_128 = OpTypeArray %PerBatchEnvMapConstantBuffer_t %uint_128
%_BindlessFastEnvMapCB_PS_t = OpTypeStruct %_arr_PerBatchEnvMapConstantBuffer_t_uint_128
%_ptr_Uniform__BindlessFastEnvMapCB_PS_t = OpTypePointer Uniform %_BindlessFastEnvMapCB_PS_t
%_ = OpVariable %_ptr_Uniform__BindlessFastEnvMapCB_PS_t Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%PerViewPushConst_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewPushConst_t = OpTypePointer PushConstant %PerViewPushConst_t
%__0 = OpVariable %_ptr_PushConstant_PerViewPushConst_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%int_2 = OpConstant %int 2
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%46 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_46 = OpTypePointer UniformConstant %46
%g_tColor = OpVariable %_ptr_UniformConstant_46 UniformConstant
%50 = OpTypeSampler
%_ptr_UniformConstant_50 = OpTypePointer UniformConstant %50
%g_sAniso = OpVariable %_ptr_UniformConstant_50 UniformConstant
%54 = OpTypeSampledImage %46
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %v4uint = OpTypeVector %uint 4
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v2float:%\w+]] = OpConstantNull %v2float
)" + kImportStub + R"(
%MainPs = OpFunction %void None %3
%5 = OpLabel
%66 = OpLoad %v2float %i_vTextureCoords
%79 = OpAccessChain %_ptr_PushConstant_uint %__0 %int_0
%80 = OpLoad %uint %79
%81 = OpAccessChain %_ptr_Uniform_v2float %_ %int_0 %80 %int_2
%82 = OpLoad %v2float %81
;CHECK-NOT: %82 = OpLoad %v2float %81
;CHECK: {{%\w+}} = OpIMul %uint %uint_80 %80
;CHECK: {{%\w+}} = OpIAdd %uint %uint_0 {{%\w+}}
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_64
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_7
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_79 {{%\w+}} %uint_0 %uint_2 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %v2float %81
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v2float {{%\w+}} {{%\w+}} [[null_v2float]] {{%\w+}}
%86 = OpFAdd %v2float %66 %82
;CHECK-NOT: %86 = OpFAdd %v2float %66 %82
;CHECK: %86 = OpFAdd %v2float %66 {{%\w+}}
%87 = OpLoad %46 %g_tColor
%88 = OpLoad %50 %g_sAniso
%89 = OpSampledImage %54 %87 %88
%91 = OpImageSampleImplicitLod %v4float %89 %86
OpStore %_entryPointOutput_vColor %91
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, UniformArrayRefWithDescInit) {
  // The buffer-oob and desc-init checks should use the same debug
  // output buffer write function.
  //
  // Same source as UniformArrayRefNoDescInit

  // clang-format off
  const std::string text = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %PerBatchEnvMapConstantBuffer_t "PerBatchEnvMapConstantBuffer_t"
OpMemberName %PerBatchEnvMapConstantBuffer_t 0 "g_matEnvMapWorldToLocal"
OpMemberName %PerBatchEnvMapConstantBuffer_t 1 "g_vEnvironmentMapBoxMins"
OpMemberName %PerBatchEnvMapConstantBuffer_t 2 "g_TexOff"
OpName %_BindlessFastEnvMapCB_PS_t "_BindlessFastEnvMapCB_PS_t"
OpMemberName %_BindlessFastEnvMapCB_PS_t 0 "g_envMapConstants"
OpName %_ ""
OpName %PerViewPushConst_t "PerViewPushConst_t"
OpMemberName %PerViewPushConst_t 0 "g_c"
OpName %__0 ""
OpName %g_tColor "g_tColor"
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 0 RowMajor
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 0 Offset 0
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 0 MatrixStride 16
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 1 Offset 48
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 2 Offset 64
OpDecorate %_arr_PerBatchEnvMapConstantBuffer_t_uint_128 ArrayStride 80
OpMemberDecorate %_BindlessFastEnvMapCB_PS_t 0 Offset 0
OpDecorate %_BindlessFastEnvMapCB_PS_t Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 2
OpMemberDecorate %PerViewPushConst_t 0 Offset 0
OpDecorate %PerViewPushConst_t Block
OpDecorate %g_tColor DescriptorSet 0
OpDecorate %g_tColor Binding 0
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %g_sAniso Binding 1
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%PerBatchEnvMapConstantBuffer_t = OpTypeStruct %mat4v3float %v4float %v2float
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%_arr_PerBatchEnvMapConstantBuffer_t_uint_128 = OpTypeArray %PerBatchEnvMapConstantBuffer_t %uint_128
%_BindlessFastEnvMapCB_PS_t = OpTypeStruct %_arr_PerBatchEnvMapConstantBuffer_t_uint_128
%_ptr_Uniform__BindlessFastEnvMapCB_PS_t = OpTypePointer Uniform %_BindlessFastEnvMapCB_PS_t
%_ = OpVariable %_ptr_Uniform__BindlessFastEnvMapCB_PS_t Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%PerViewPushConst_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewPushConst_t = OpTypePointer PushConstant %PerViewPushConst_t
%__0 = OpVariable %_ptr_PushConstant_PerViewPushConst_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%int_2 = OpConstant %int 2
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%46 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_46 = OpTypePointer UniformConstant %46
%g_tColor = OpVariable %_ptr_UniformConstant_46 UniformConstant
%50 = OpTypeSampler
%_ptr_UniformConstant_50 = OpTypePointer UniformConstant %50
%g_sAniso = OpVariable %_ptr_UniformConstant_50 UniformConstant
%54 = OpTypeSampledImage %46
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %v4uint = OpTypeVector %uint 4
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v2float:%\w+]] = OpConstantNull %v2float
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)" + kImportStub + R"(
%MainPs = OpFunction %void None %3
%5 = OpLabel
%66 = OpLoad %v2float %i_vTextureCoords
%79 = OpAccessChain %_ptr_PushConstant_uint %__0 %int_0
%80 = OpLoad %uint %79
%81 = OpAccessChain %_ptr_Uniform_v2float %_ %int_0 %80 %int_2
%82 = OpLoad %v2float %81
%86 = OpFAdd %v2float %66 %82
;CHECK-NOT: %82 = OpLoad %v2float %81
;CHECK-NOT: %86 = OpFAdd %v2float %66 %82
;CHECK: {{%\w+}} = OpIMul %uint %uint_80 %80
;CHECK: {{%\w+}} = OpIAdd %uint %uint_0 {{%\w+}}
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_64
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_7
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_79 {{%\w+}} %uint_0 %uint_2 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %v2float %81
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v2float {{%\w+}} {{%\w+}} [[null_v2float]] {{%\w+}}
;CHECK: %86 = OpFAdd %v2float %66 {{%\w+}}
%87 = OpLoad %46 %g_tColor
%88 = OpLoad %50 %g_sAniso
%89 = OpSampledImage %54 %87 %88
%91 = OpImageSampleImplicitLod %v4float %89 %86
OpStore %_entryPointOutput_vColor %91
;CHECK-NOT: %91 = OpImageSampleImplicitLod %v4float %89 %86
;CHECK-NOT:       OpStore %_entryPointOutput_vColor %91
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_84 {{%\w+}} %uint_0 %uint_0 %uint_0 %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %46 %g_tColor
;CHECK: {{%\w+}} = OpSampledImage %54 {{%\w+}} %88
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} %86
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %_entryPointOutput_vColor {{%\w+}}
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, Descriptor16BitIdxRef) {
  // Check that descriptor indexed with 16bit index is inbounds and
  // initialized
  //
  // Use Simple source with min16uint g_nDataIdx

  // clang-format off
  const std::string text = R"(
OpCapability Shader
OpCapability Int16
OpCapability StoragePushConstant16
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %g_tColor %_ %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %g_tColor %_ %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %PerViewConstantBuffer_t "PerViewConstantBuffer_t"
OpMemberName %PerViewConstantBuffer_t 0 "g_nDataIdx"
OpName %_ ""
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 1
OpDecorate %g_tColor Binding 2
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %g_sAniso DescriptorSet 1
OpDecorate %g_sAniso Binding 2
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%16 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%_arr_16_uint_128 = OpTypeArray %16 %uint_128
%_ptr_UniformConstant__arr_16_uint_128 = OpTypePointer UniformConstant %_arr_16_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_16_uint_128 UniformConstant
%ushort = OpTypeInt 16 0
%PerViewConstantBuffer_t = OpTypeStruct %ushort
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_ushort = OpTypePointer PushConstant %ushort
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
%25 = OpTypeSampler
%_ptr_UniformConstant_25 = OpTypePointer UniformConstant %25
%g_sAniso = OpVariable %_ptr_UniformConstant_25 UniformConstant
%27 = OpTypeSampledImage %16
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)" + kImportStub + R"(
%MainPs = OpFunction %void None %10
%30 = OpLabel
;CHECK:  OpBranch %39
;CHECK:  %39 = OpLabel
%31 = OpLoad %v2float %i_vTextureCoords
%32 = OpAccessChain %_ptr_PushConstant_ushort %_ %int_0
%33 = OpLoad %ushort %32
%34 = OpAccessChain %_ptr_UniformConstant_16 %g_tColor %33
%35 = OpLoad %16 %34
%36 = OpLoad %25 %g_sAniso
%37 = OpSampledImage %27 %35 %36
%38 = OpImageSampleImplicitLod %v4float %37 %31
OpStore %_entryPointOutput_vColor %38
;CHECK-NOT: %38 = OpImageSampleImplicitLod %v4float %37 %31
;CHECK-NOT: OpStore %_entryPointOutput_vColor %38
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpUConvert %uint %33
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_61 {{%\w+}} %uint_1 %uint_2 {{%\w+}} %uint_0
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %16 %34
;CHECK: {{%\w+}} = OpSampledImage %27 {{%\w+}} %36
;CHECK: {{%\w+}} = OpImageSampleImplicitLod %v4float {{%\w+}} %31
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %_entryPointOutput_vColor [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, UniformArray16bitIdxRef) {
  // Check that uniform array ref with 16bit index does not go out-of-bounds.
  //
  // Texture2D g_tColor;
  // SamplerState g_sAniso;
  //
  // layout(push_constant) cbuffer PerViewPushConst_t { min16uint g_c; };
  //
  // struct PerBatchEnvMapConstantBuffer_t {
  //   float4x3 g_matEnvMapWorldToLocal;
  //   float4 g_vEnvironmentMapBoxMins;
  //   float2 g_TexOff;
  // };
  //
  // cbuffer _BindlessFastEnvMapCB_PS_t {
  //   PerBatchEnvMapConstantBuffer_t g_envMapConstants[128];
  // };
  //
  // struct PS_INPUT {
  //   float2 vTextureCoords : TEXCOORD2;
  // };
  //
  // struct PS_OUTPUT {
  //   float4 vColor : SV_Target0;
  // };
  //
  // PS_OUTPUT MainPs(PS_INPUT i) {
  //   PS_OUTPUT ps_output;
  //   float2 off;
  //   float2 vtc;
  //   off = g_envMapConstants[g_c].g_TexOff;
  //   vtc = i.vTextureCoords.xy + off;
  //   ps_output.vColor = g_tColor.Sample(g_sAniso, vtc);
  //   return ps_output;
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
OpCapability Int16
OpCapability StoragePushConstant16
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor
;CHECK: OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %PerBatchEnvMapConstantBuffer_t "PerBatchEnvMapConstantBuffer_t"
OpMemberName %PerBatchEnvMapConstantBuffer_t 0 "g_matEnvMapWorldToLocal"
OpMemberName %PerBatchEnvMapConstantBuffer_t 1 "g_vEnvironmentMapBoxMins"
OpMemberName %PerBatchEnvMapConstantBuffer_t 2 "g_TexOff"
OpName %_BindlessFastEnvMapCB_PS_t "_BindlessFastEnvMapCB_PS_t"
OpMemberName %_BindlessFastEnvMapCB_PS_t 0 "g_envMapConstants"
OpName %_ ""
OpName %PerViewPushConst_t "PerViewPushConst_t"
OpMemberName %PerViewPushConst_t 0 "g_c"
OpName %__0 ""
OpName %g_tColor "g_tColor"
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 0 RowMajor
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 0 Offset 0
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 0 MatrixStride 16
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 1 Offset 48
OpMemberDecorate %PerBatchEnvMapConstantBuffer_t 2 Offset 64
OpDecorate %_arr_PerBatchEnvMapConstantBuffer_t_uint_128 ArrayStride 80
OpMemberDecorate %_BindlessFastEnvMapCB_PS_t 0 Offset 0
OpDecorate %_BindlessFastEnvMapCB_PS_t Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
OpMemberDecorate %PerViewPushConst_t 0 Offset 0
OpDecorate %PerViewPushConst_t Block
OpDecorate %g_tColor DescriptorSet 0
OpDecorate %g_tColor Binding 0
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %g_sAniso Binding 0
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%14 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%PerBatchEnvMapConstantBuffer_t = OpTypeStruct %mat4v3float %v4float %v2float
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%_arr_PerBatchEnvMapConstantBuffer_t_uint_128 = OpTypeArray %PerBatchEnvMapConstantBuffer_t %uint_128
%_BindlessFastEnvMapCB_PS_t = OpTypeStruct %_arr_PerBatchEnvMapConstantBuffer_t_uint_128
%_ptr_Uniform__BindlessFastEnvMapCB_PS_t = OpTypePointer Uniform %_BindlessFastEnvMapCB_PS_t
%_ = OpVariable %_ptr_Uniform__BindlessFastEnvMapCB_PS_t Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%ushort = OpTypeInt 16 0
%PerViewPushConst_t = OpTypeStruct %ushort
%_ptr_PushConstant_PerViewPushConst_t = OpTypePointer PushConstant %PerViewPushConst_t
%__0 = OpVariable %_ptr_PushConstant_PerViewPushConst_t PushConstant
%_ptr_PushConstant_ushort = OpTypePointer PushConstant %ushort
%int_2 = OpConstant %int 2
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%30 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_30 = OpTypePointer UniformConstant %30
%g_tColor = OpVariable %_ptr_UniformConstant_30 UniformConstant
%32 = OpTypeSampler
%_ptr_UniformConstant_32 = OpTypePointer UniformConstant %32
%g_sAniso = OpVariable %_ptr_UniformConstant_32 UniformConstant
%34 = OpTypeSampledImage %30
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v2float:%\w+]] = OpConstantNull %v2float
)" + kImportStub + R"(
%MainPs = OpFunction %void None %14
%37 = OpLabel
%38 = OpLoad %v2float %i_vTextureCoords
%39 = OpAccessChain %_ptr_PushConstant_ushort %__0 %int_0
%40 = OpLoad %ushort %39
%41 = OpAccessChain %_ptr_Uniform_v2float %_ %int_0 %40 %int_2
%42 = OpLoad %v2float %41
%43 = OpFAdd %v2float %38 %42
;CHECK-NOT: %42 = OpLoad %v2float %41
;CHECK-NOT: %43 = OpFAdd %v2float %38 %42
;CHECK: {{%\w+}} = OpUConvert %uint %40
;CHECK: {{%\w+}} = OpIMul %uint %uint_80 {{%\w+}}
;CHECK: {{%\w+}} = OpIAdd %uint %uint_0 {{%\w+}}
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_64
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_7
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_82 {{%\w+}} %uint_0 %uint_0 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %v2float %41
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %v2float {{%\w+}} {{%\w+}} [[null_v2float]] {{%\w+}}
;CHECK: %43 = OpFAdd %v2float %38 {{%\w+}}
%44 = OpLoad %30 %g_tColor
%45 = OpLoad %32 %g_sAniso
%46 = OpSampledImage %34 %44 %45
%47 = OpImageSampleImplicitLod %v4float %46 %43
OpStore %_entryPointOutput_vColor %47
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, UniformMatrixRefRowMajor) {
  // The buffer-oob row major matrix check
  //
  // #version 450
  // #extension GL_EXT_scalar_block_layout : enable
  //
  // layout(location = 0) in highp vec4 a_position;
  // layout(location = 0) out mediump float v_vtxResult;
  //
  // layout(set = 0, binding = 0, std430, row_major) uniform Block
  // {
  //    lowp mat4x2 var;
  // };
  //
  // void main (void)
  // {
  //    v_vtxResult = var[2][1];
  // }

  // clang-format off
  std::string text = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position
;CHECK: OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position %gl_VertexIndex %gl_InstanceIndex
OpSource GLSL 450
OpSourceExtension "GL_EXT_scalar_block_layout"
OpName %main "main"
OpName %v_vtxResult "v_vtxResult"
OpName %Block "Block"
OpMemberName %Block 0 "var"
OpName %_ ""
OpName %a_position "a_position"
OpDecorate %v_vtxResult RelaxedPrecision
OpDecorate %v_vtxResult Location 0
OpMemberDecorate %Block 0 RowMajor
OpMemberDecorate %Block 0 RelaxedPrecision
OpMemberDecorate %Block 0 Offset 0
OpMemberDecorate %Block 0 MatrixStride 16
OpDecorate %Block Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
OpDecorate %21 RelaxedPrecision
;CHECK-NOT: OpDecorate %21 RelaxedPrecision
;CHECK: OpDecorate %v_vtxResult RelaxedPrecision
;CHECK: OpDecorate [[phi_result:%\w+]] RelaxedPrecision
OpDecorate %a_position Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_VertexIndex BuiltIn VertexIndex
;CHECK: OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
;CHECK: OpDecorate [[load_result:%\w+]] RelaxedPrecision
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
%v_vtxResult = OpVariable %_ptr_Output_float Output
%v2float = OpTypeVector %float 2
%mat4v2float = OpTypeMatrix %v2float 4
%Block = OpTypeStruct %mat4v2float
%_ptr_Uniform_Block = OpTypePointer Uniform %Block
%_ = OpVariable %_ptr_Uniform_Block Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%_ptr_Uniform_float = OpTypePointer Uniform %float
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%a_position = OpVariable %_ptr_Input_v4float Input
;CHECK: %_ptr_Input_uint = OpTypePointer Input %uint
;CHECK: %gl_VertexIndex = OpVariable %_ptr_Input_uint Input
;CHECK: %gl_InstanceIndex = OpVariable %_ptr_Input_uint Input
;CHECK: [[null_float:%\w+]] = OpConstantNull %float
)" + kImportStub + R"(
%main = OpFunction %void None %3
%5 = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
%20 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %int_2 %uint_1
%21 = OpLoad %float %20
;CHECK-NOT: %21 = OpLoad %float %20
;CHECK: {{%\w+}} = OpIMul %uint %uint_4 %int_2
;CHECK: {{%\w+}} = OpIAdd %uint %uint_0 {{%\w+}}
;CHECK: {{%\w+}} = OpIMul %uint %uint_16 %uint_1
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_3
;CHECK: {{%\w+}} = OpLoad %uint %gl_VertexIndex
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: [[desc_state:%\w+]] = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_46 {{%\w+}} %uint_0 %uint_0 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional [[desc_state]] {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[load_result]] = OpLoad %float %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result]] = OpPhi %float [[load_result]] {{%\w+}} [[null_float]] {{%\w+}}
OpStore %v_vtxResult %21
;CHECK-NOT: OpStore %v_vtxResult %21$
;CHECK: OpStore %v_vtxResult [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, UniformMatrixRefColumnMajor) {
  // The buffer-oob column major matrix check
  //
  // #version 450
  // #extension GL_EXT_scalar_block_layout : enable
  //
  // layout(location = 0) in highp vec4 a_position;
  // layout(location = 0) out mediump float v_vtxResult;
  //
  // layout(set = 0, binding = 0, std430, column_major) uniform Block
  // {
  //    lowp mat4x2 var;
  // };
  //
  // void main (void)
  // {
  //    v_vtxResult = var[2][1];
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position
;CHECK: OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position %gl_VertexIndex %gl_InstanceIndex
OpSource GLSL 450
OpSourceExtension "GL_EXT_scalar_block_layout"
OpName %main "main"
OpName %v_vtxResult "v_vtxResult"
OpName %Block "Block"
OpMemberName %Block 0 "var"
OpName %_ ""
OpName %a_position "a_position"
OpDecorate %v_vtxResult RelaxedPrecision
OpDecorate %v_vtxResult Location 0
OpMemberDecorate %Block 0 ColMajor
OpMemberDecorate %Block 0 RelaxedPrecision
OpMemberDecorate %Block 0 Offset 0
OpMemberDecorate %Block 0 MatrixStride 8
OpDecorate %Block Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
OpDecorate %21 RelaxedPrecision
;CHECK-NOT: OpDecorate %21 RelaxedPrecision
;CHECK: OpDecorate %v_vtxResult RelaxedPrecision
;CHECK: OpDecorate [[phi_result:%\w+]] RelaxedPrecision
OpDecorate %a_position Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_VertexIndex BuiltIn VertexIndex
;CHECK: OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
;CHECK: OpDecorate [[load_result:%\w+]] RelaxedPrecision
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
%v_vtxResult = OpVariable %_ptr_Output_float Output
%v2float = OpTypeVector %float 2
%mat4v2float = OpTypeMatrix %v2float 4
%Block = OpTypeStruct %mat4v2float
%_ptr_Uniform_Block = OpTypePointer Uniform %Block
%_ = OpVariable %_ptr_Uniform_Block Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%_ptr_Uniform_float = OpTypePointer Uniform %float
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%a_position = OpVariable %_ptr_Input_v4float Input
;CHECK: %_ptr_Input_uint = OpTypePointer Input %uint
;CHECK: %gl_VertexIndex = OpVariable %_ptr_Input_uint Input
;CHECK: %gl_InstanceIndex = OpVariable %_ptr_Input_uint Input
;CHECK: [[null_float:%\w+]] = OpConstantNull %float
)" + kImportStub + R"(
%main = OpFunction %void None %3
%5 = OpLabel
%20 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %int_2 %uint_1
%21 = OpLoad %float %20
;CHECK-NOT: %21 = OpLoad %float %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpIMul %uint %uint_8 %int_2
;CHECK: {{%\w+}} = OpIAdd %uint %uint_0 {{%\w+}}
;CHECK: {{%\w+}} = OpIMul %uint %uint_4 %uint_1
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_3
;CHECK: {{%\w+}} = OpLoad %uint %gl_VertexIndex
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: [[desc_state:%\w+]] = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_46 {{%\w+}} %uint_0 %uint_0 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional [[desc_state]] {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK:[[load_result]] = OpLoad %float %20
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result]] = OpPhi %float [[load_result]] {{%\w+}} [[null_float]] {{%\w+}}
OpStore %v_vtxResult %21
;CHECK-NOT: OpStore %v_vtxResult %21$
;CHECK: OpStore %v_vtxResult [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  ValidatorOptions()->uniform_buffer_standard_layout = true;
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, UniformMatrixVecRefRowMajor) {
  // The buffer-oob row major matrix vector ref check
  //
  // #version 450
  // #extension GL_EXT_scalar_block_layout : enable
  //
  // layout(location = 0) in highp vec4 a_position;
  // layout(location = 0) out highp vec2 v_vtxResult;
  //
  // layout(set = 3, binding = 7, std430, row_major) uniform Block
  // {
  //    lowp mat2 var[3][4];
  // };
  //
  // void main (void)
  // {
  //    v_vtxResult = var[2][3][1];
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position
;CHECK: OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position %gl_VertexIndex %gl_InstanceIndex
OpSource GLSL 450
OpSourceExtension "GL_EXT_scalar_block_layout"
OpName %main "main"
OpName %v_vtxResult "v_vtxResult"
OpName %Block "Block"
OpMemberName %Block 0 "var"
OpName %_ ""
OpName %a_position "a_position"
OpDecorate %v_vtxResult Location 0
OpDecorate %_arr_mat2v2float_uint_4 ArrayStride 32
OpDecorate %_arr__arr_mat2v2float_uint_4_uint_3 ArrayStride 128
OpMemberDecorate %Block 0 RowMajor
OpMemberDecorate %Block 0 RelaxedPrecision
OpMemberDecorate %Block 0 Offset 0
OpMemberDecorate %Block 0 MatrixStride 16
OpDecorate %Block Block
OpDecorate %_ DescriptorSet 3
OpDecorate %_ Binding 7
OpDecorate %26 RelaxedPrecision
;CHECK-NOT: OpDecorate %26 RelaxedPrecision
;CHECK: OpDecorate [[phi_result:%\w+]] RelaxedPrecision
OpDecorate %a_position Location 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_VertexIndex BuiltIn VertexIndex
;CHECK: OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
;CHECK: OpDecorate [[load_result:%\w+]] RelaxedPrecision
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%_ptr_Output_v2float = OpTypePointer Output %v2float
%v_vtxResult = OpVariable %_ptr_Output_v2float Output
%mat2v2float = OpTypeMatrix %v2float 2
%uint = OpTypeInt 32 0
%uint_4 = OpConstant %uint 4
%_arr_mat2v2float_uint_4 = OpTypeArray %mat2v2float %uint_4
%uint_3 = OpConstant %uint 3
%_arr__arr_mat2v2float_uint_4_uint_3 = OpTypeArray %_arr_mat2v2float_uint_4 %uint_3
%Block = OpTypeStruct %_arr__arr_mat2v2float_uint_4_uint_3
%_ptr_Uniform_Block = OpTypePointer Uniform %Block
%_ = OpVariable %_ptr_Uniform_Block Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_1 = OpConstant %int 1
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%a_position = OpVariable %_ptr_Input_v4float Input
;CHECK: %_ptr_Input_uint = OpTypePointer Input %uint
;CHECK: %gl_VertexIndex = OpVariable %_ptr_Input_uint Input
;CHECK: %gl_InstanceIndex = OpVariable %_ptr_Input_uint Input
;CHECK: [[null_v2float:%\w+]] = OpConstantNull %v2float
)" + kImportStub + R"(
%main = OpFunction %void None %3
%5 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_v2float %_ %int_0 %int_2 %int_3 %int_1
;CHECK: {{%\w+}} = OpIMul %uint %uint_128 %int_2
;CHECK: {{%\w+}} = OpIAdd %uint %uint_0 {{%\w+}}
;CHECK: {{%\w+}} = OpIMul %uint %uint_32 %int_3
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpIMul %uint %uint_4 %int_1
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_19
;CHECK: {{%\w+}} = OpLoad %uint %gl_VertexIndex
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: [[desc_state:%\w+]] = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_52 {{%\w+}} %uint_3 %uint_7 %uint_0 {{%\w+}}
%26 = OpLoad %v2float %25
OpStore %v_vtxResult %26
;CHECK-NOT: %26 = OpLoad %v2float %25
;CHECK-NOT: OpStore %v_vtxResult %26
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional [[desc_state]] {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[load_result]] = OpLoad %v2float %25
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result]] = OpPhi %v2float [[load_result]] {{%\w+}} [[null_v2float]] {{%\w+}}
;CHECK: OpStore %v_vtxResult [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, ImageBufferOOBRead) {
  // Texel buffer (imagebuffer) oob check for ImageRead
  //
  // #version 450
  // layout(set=3, binding=7, r32f) uniform readonly imageBuffer s;
  // layout(location=11) out vec4 x;
  // layout(location=13) in flat int ii;
  //
  // void main(){
  //    x = imageLoad(s, ii);
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
OpCapability ImageBuffer
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %x %s %ii
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %x "x"
OpName %s "s"
OpName %ii "ii"
OpDecorate %x Location 11
OpDecorate %s DescriptorSet 3
OpDecorate %s Binding 7
OpDecorate %s NonWritable
OpDecorate %ii Flat
OpDecorate %ii Location 13
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%x = OpVariable %_ptr_Output_v4float Output
%10 = OpTypeImage %float Buffer 0 0 0 2 R32f
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
%s = OpVariable %_ptr_UniformConstant_10 UniformConstant
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%ii = OpVariable %_ptr_Input_int Input
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
%main = OpFunction %void None %3
%5 = OpLabel
;CHECK: OpBranch %19
;CHECK: %19 = OpLabel
%13 = OpLoad %10 %s
%17 = OpLoad %int %ii
%18 = OpImageRead %v4float %13 %17
OpStore %x %18
;CHECK-NOT: %18 = OpImageRead %v4float %13 %17
;CHECK-NOT: OpStore %x %18
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_34 {{%\w+}} %uint_3 %uint_7 %uint_0 %22
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %10 %s
;CHECK: {{%\w+}} = OpImageRead %v4float {{%\w+}} %17
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %x [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, ImageBufferOOBWrite) {
  // Texel buffer (imagebuffer) oob check for ImageWrite
  //
  // #version 450
  // layout(set=3, binding=7, r32f) uniform readonly imageBuffer s;
  // layout(location=11) out vec4 x;
  // layout(location=13) in flat int ii;
  //
  // void main(){
  //    imageStore(s, ii, x);
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
OpCapability ImageBuffer
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %s %ii %x
;CHECK: OpEntryPoint Fragment %main "main" %s %ii %x %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %s "s"
OpName %ii "ii"
OpName %x "x"
OpDecorate %s DescriptorSet 3
OpDecorate %s Binding 7
OpDecorate %s NonReadable
OpDecorate %ii Flat
OpDecorate %ii Location 13
OpDecorate %x Location 11
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%7 = OpTypeImage %float Buffer 0 0 0 2 R32f
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
%s = OpVariable %_ptr_UniformConstant_7 UniformConstant
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%ii = OpVariable %_ptr_Input_int Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%x = OpVariable %_ptr_Output_v4float Output
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
)" + kImportStub + R"(
%main = OpFunction %void None %3
%5 = OpLabel
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: %19 = OpLabel
%10 = OpLoad %7 %s
%14 = OpLoad %int %ii
%18 = OpLoad %v4float %x
OpImageWrite %10 %14 %18
;CHECK-NOT: OpImageWrite %10 %14 %18
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_35 {{%\w+}} %uint_3 %uint_7 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %7 %s
;CHECK: OpImageWrite {{%\w+}} %14 %18
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, TextureBufferOOBFetch) {
  // Texel buffer (texturebuffer) oob check for ImageFetch
  //
  // #version 450
  // layout(set=3, binding=7) uniform textureBuffer s;
  // layout(location=11) out vec4 x;
  // layout(location=13) in flat int ii;
  //
  // void main(){
  //    x = texelFetch(s, ii);
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
OpCapability SampledBuffer
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %x %s %ii
;CHECK: OpEntryPoint Fragment %main "main" %x %s %ii %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %x "x"
OpName %s "s"
OpName %ii "ii"
OpDecorate %x Location 11
OpDecorate %s DescriptorSet 3
OpDecorate %s Binding 7
OpDecorate %ii Flat
OpDecorate %ii Location 13
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%x = OpVariable %_ptr_Output_v4float Output
%10 = OpTypeImage %float Buffer 0 0 0 1 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
%s = OpVariable %_ptr_UniformConstant_10 UniformConstant
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%ii = OpVariable %_ptr_Input_int Input
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
%main = OpFunction %void None %3
%5 = OpLabel
;CHECK: OpBranch %19
;CHECK: %19 = OpLabel
%13 = OpLoad %10 %s
%17 = OpLoad %int %ii
%18 = OpImageFetch %v4float %13 %17
OpStore %x %18
;CHECK-NOT: %18 = OpImageFetch %v4float %13 %17
;CHECK-NOT: OpStore %x %18
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_33 {{%\w+}} %uint_3 %uint_7 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %10 %s
;CHECK: {{%\w+}} = OpImageFetch %v4float {{%\w+}} %17
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %x [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, SamplerBufferOOBFetch) {
  // Texel buffer (samplerbuffer) oob check for ImageFetch
  //
  // #version 450
  // layout(set=3, binding=7) uniform samplerBuffer s;
  // layout(location=11) out vec4 x;
  // layout(location=13) in flat int ii;
  //
  // void main(){
  //    x = texelFetch(s, ii);
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
OpCapability SampledBuffer
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %x %s %ii
;CHECK: OpEntryPoint Fragment %main "main" %x %s %ii %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %x "x"
OpName %s "s"
OpName %ii "ii"
OpDecorate %x Location 11
OpDecorate %s DescriptorSet 3
OpDecorate %s Binding 7
OpDecorate %ii Flat
OpDecorate %ii Location 13
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%x = OpVariable %_ptr_Output_v4float Output
%10 = OpTypeImage %float Buffer 0 0 0 1 Unknown
%11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
%s = OpVariable %_ptr_UniformConstant_11 UniformConstant
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%ii = OpVariable %_ptr_Input_int Input
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
)" + kImportStub + R"(
%main = OpFunction %void None %3
%5 = OpLabel
;CHECK: OpBranch %21
;CHECK: %21 = OpLabel
%14 = OpLoad %11 %s
%18 = OpLoad %int %ii
%19 = OpImage %10 %14
%20 = OpImageFetch %v4float %19 %18
OpStore %x %20
;CHECK-NOT: %20 = OpImageFetch %v4float %19 %18
;CHECK-NOT: OpStore %x %20
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_35 {{%\w+}} %uint_3 %uint_7 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %11 %s
;CHECK: {{%\w+}} = OpImage %10 {{%\w+}}
;CHECK: {{%\w+}} = OpImageFetch %v4float {{%\w+}} {{%\w+}}
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %x [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, SamplerBufferConstructorOOBFetch) {
  // Texel buffer (samplerbuffer constructor) oob check for ImageFetch
  //
  // #version 450
  // layout(set=3, binding=7) uniform textureBuffer tBuf;
  // layout(set=3, binding=8) uniform sampler s;
  // layout(location=11) out vec4 x;
  // layout(location=13) in flat int ii;
  //
  // void main(){
  //    x = texelFetch(samplerBuffer(tBuf, s), ii);
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
OpCapability SampledBuffer
;CHECK: OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %x %tBuf %s %ii
;CHECK: OpEntryPoint Fragment %main "main" %x %tBuf %s %ii %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %x "x"
OpName %tBuf "tBuf"
OpName %s "s"
OpName %ii "ii"
OpDecorate %x Location 11
OpDecorate %tBuf DescriptorSet 3
OpDecorate %tBuf Binding 7
OpDecorate %s DescriptorSet 3
OpDecorate %s Binding 8
OpDecorate %ii Flat
OpDecorate %ii Location 13
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%x = OpVariable %_ptr_Output_v4float Output
%10 = OpTypeImage %float Buffer 0 0 0 1 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
%tBuf = OpVariable %_ptr_UniformConstant_10 UniformConstant
%14 = OpTypeSampler
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
%s = OpVariable %_ptr_UniformConstant_14 UniformConstant
%18 = OpTypeSampledImage %10
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%ii = OpVariable %_ptr_Input_int Input
;CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK: [[null_v4float:%\w+]] = OpConstantNull %v4float
%main = OpFunction %void None %3
%5 = OpLabel
%13 = OpLoad %10 %tBuf
%17 = OpLoad %14 %s
%19 = OpSampledImage %18 %13 %17
%23 = OpLoad %int %ii
%24 = OpImage %10 %19
%25 = OpImageFetch %v4float %24 %23
OpStore %x %25
;CHECK-NOT: %25 = OpImageFetch %v4float %24 %23
;CHECK-NOT: OpStore %x %25
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_43 {{%\w+}} %uint_3 %uint_7 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %10 %tBuf
;CHECK: {{%\w+}} = OpSampledImage %18 {{%\w+}} %17
;CHECK: {{%\w+}} = OpImage %10 {{%\w+}}
;CHECK: {{%\w+}} = OpImageFetch %v4float {{%\w+}} %23
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %v4float {{%\w+}} {{%\w+}} [[null_v4float]] {{%\w+}}
;CHECK: OpStore %x [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, DeviceBufferAddressOOB) {
  // #version 450
  // #extension GL_EXT_buffer_reference : enable
  //  layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
  // layout(set = 0, binding = 0) uniform ufoo {
  //     bufStruct data;
  //     int nWrites;
  // } u_info;
  // layout(buffer_reference, std140) buffer bufStruct {
  //     int a[4];
  // };
  // void main() {
  //     for (int i=0; i < u_info.nWrites; ++i) {
  //         u_info.data.a[i] = 0xdeadca71;
  //     }
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
;CHECK: OpCapability Linkage
;CHECK: OpCapability Int64
OpExtension "SPV_KHR_physical_storage_buffer"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Vertex %main "main" %u_info
;CHECK: OpEntryPoint Vertex %main "main" %u_info %gl_VertexIndex %gl_InstanceIndex
OpSource GLSL 450
OpSourceExtension "GL_EXT_buffer_reference"
OpName %main "main"
OpName %i "i"
OpName %ufoo "ufoo"
OpMemberName %ufoo 0 "data"
OpMemberName %ufoo 1 "nWrites"
OpName %bufStruct "bufStruct"
OpMemberName %bufStruct 0 "a"
OpName %u_info "u_info"
OpMemberDecorate %ufoo 0 Offset 0
OpMemberDecorate %ufoo 1 Offset 8
OpDecorate %ufoo Block
OpDecorate %_arr_int_uint_4 ArrayStride 16
OpMemberDecorate %bufStruct 0 Offset 0
OpDecorate %bufStruct Block
OpDecorate %u_info DescriptorSet 0
OpDecorate %u_info Binding 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_bufStruct PhysicalStorageBuffer
%ufoo = OpTypeStruct %_ptr_PhysicalStorageBuffer_bufStruct %int
%uint = OpTypeInt 32 0
%uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
%bufStruct = OpTypeStruct %_arr_int_uint_4
%_ptr_PhysicalStorageBuffer_bufStruct = OpTypePointer PhysicalStorageBuffer %bufStruct
%_ptr_Uniform_ufoo = OpTypePointer Uniform %ufoo
%u_info = OpVariable %_ptr_Uniform_ufoo Uniform
%int_1 = OpConstant %int 1
%_ptr_Uniform_int = OpTypePointer Uniform %int
%bool = OpTypeBool
%_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct = OpTypePointer Uniform %_ptr_PhysicalStorageBuffer_bufStruct
%int_n559035791 = OpConstant %int -559035791
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
)" + kImportStub + R"(
%main = OpFunction %void None %3
%5 = OpLabel
%i = OpVariable %_ptr_Function_int Function
OpStore %i %int_0
OpBranch %10
%10 = OpLabel
OpLoopMerge %12 %13 None
OpBranch %14
%14 = OpLabel
%15 = OpLoad %int %i
%26 = OpAccessChain %_ptr_Uniform_int %u_info %int_1
;CHECK: {{%\w+}} = OpIAdd %uint %uint_8 %uint_3
;CHECK: {{%\w+}} = OpLoad %uint %gl_VertexIndex
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_56 {{%\w+}} %uint_0 %uint_0 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[load_result:%\w+]] = OpLoad %int %26
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %int [[load_result]] {{%\w+}} {{%\w+}} {{%\w+}}
%27 = OpLoad %int %26
%29 = OpSLessThan %bool %15 %27
;CHECK-NOT: %27 = OpLoad %int %26
;CHECK-NOT: %29 = OpSLessThan %bool %15 %27
;CHECK: %29 = OpSLessThan %bool %15 [[phi_result]]
OpBranchConditional %29 %11 %12
%11 = OpLabel
%31 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %u_info %int_0
%32 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %31
;CHECK-NOT: %32 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %31
;CHECK: {{%\w+}} = OpIAdd %uint %uint_0 %uint_7
;CHECK: {{%\w+}} = OpLoad %uint %gl_VertexIndex
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_61 {{%\w+}} %uint_0 %uint_0 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[load_result_2:%\w+]] = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %31
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_bufStruct {{%\w+}}
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result_2:%\w+]] = OpPhi %_ptr_PhysicalStorageBuffer_bufStruct [[load_result_2]] {{%\w+}} {{%\w+}} {{%\w+}}
%33 = OpLoad %int %i
%36 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %32 %int_0 %33
;CHECK-NOT: %36 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %32 %int_0 %33
;CHECK: %36 = OpAccessChain %_ptr_PhysicalStorageBuffer_int [[phi_result_2]] %int_0 %33
OpStore %36 %int_n559035791 Aligned 16
OpBranch %13
%13 = OpLabel
%37 = OpLoad %int %i
%38 = OpIAdd %int %37 %int_1
OpStore %i %38
OpBranch %10
%12 = OpLabel
OpReturn
OpFunctionEnd)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

TEST_F(InstBindlessTest, VertexIndexOOB) {
  // #version 450
  // layout(std140, binding = 0) uniform foo { uint tex_index[1]; }
  // uniform_index_buffer; layout(location = 0) out flat uint index; vec2
  // vertices[3]; void main() {
  //     vertices[0] = vec2(-1.0, -1.0);
  //     vertices[1] = vec2( 1.0, -1.0);
  //     vertices[2] = vec2( 0.0,  1.0);
  //     gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
  //     index = uniform_index_buffer.tex_index[0];
  // }
  // clang-format off
  const std::string text = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %vertices %_ %gl_VertexIndex %index %uniform_index_buffer
OpSource GLSL 450
OpName %main "main"
OpName %vertices "vertices"
OpName %gl_PerVertex "gl_PerVertex"
OpMemberName %gl_PerVertex 0 "gl_Position"
OpMemberName %gl_PerVertex 1 "gl_PointSize"
OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
OpMemberName %gl_PerVertex 3 "gl_CullDistance"
OpName %_ ""
OpName %gl_VertexIndex "gl_VertexIndex"
OpName %index "index"
OpName %foo "foo"
OpMemberName %foo 0 "tex_index"
OpName %uniform_index_buffer "uniform_index_buffer"
OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
OpDecorate %gl_PerVertex Block
OpDecorate %gl_VertexIndex BuiltIn VertexIndex
OpDecorate %index Flat
OpDecorate %index Location 0
OpDecorate %_arr_uint_uint_1 ArrayStride 16
OpMemberDecorate %foo 0 Offset 0
OpDecorate %foo Block
OpDecorate %uniform_index_buffer DescriptorSet 0
OpDecorate %uniform_index_buffer Binding 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%uint = OpTypeInt 32 0
%uint_3 = OpConstant %uint 3
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
%_ptr_Private__arr_v2float_uint_3 = OpTypePointer Private %_arr_v2float_uint_3
%vertices = OpVariable %_ptr_Private__arr_v2float_uint_3 Private
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_n1 = OpConstant %float -1
%16 = OpConstantComposite %v2float %float_n1 %float_n1
%_ptr_Private_v2float = OpTypePointer Private %v2float
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%21 = OpConstantComposite %v2float %float_1 %float_n1
%int_2 = OpConstant %int 2
%float_0 = OpConstant %float 0
%25 = OpConstantComposite %v2float %float_0 %float_1
%v4float = OpTypeVector %float 4
%uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
%_ = OpVariable %_ptr_Output_gl_PerVertex Output
%_ptr_Input_int = OpTypePointer Input %int
%gl_VertexIndex = OpVariable %_ptr_Input_int Input
%int_3 = OpConstant %int 3
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Output_uint = OpTypePointer Output %uint
%index = OpVariable %_ptr_Output_uint Output
%_arr_uint_uint_1 = OpTypeArray %uint %uint_1
%foo = OpTypeStruct %_arr_uint_uint_1
%_ptr_Uniform_foo = OpTypePointer Uniform %foo
%uniform_index_buffer = OpVariable %_ptr_Uniform_foo Uniform
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
)" + kImportStub + R"(
%main = OpFunction %void None %3
%5 = OpLabel
%18 = OpAccessChain %_ptr_Private_v2float %vertices %int_0
OpStore %18 %16
%22 = OpAccessChain %_ptr_Private_v2float %vertices %int_1
OpStore %22 %21
%26 = OpAccessChain %_ptr_Private_v2float %vertices %int_2
OpStore %26 %25
%35 = OpLoad %int %gl_VertexIndex
%37 = OpSMod %int %35 %int_3
%38 = OpAccessChain %_ptr_Private_v2float %vertices %37
%39 = OpLoad %v2float %38
%40 = OpCompositeExtract %float %39 0
%41 = OpCompositeExtract %float %39 1
%42 = OpCompositeConstruct %v4float %40 %41 %float_0 %float_1
%44 = OpAccessChain %_ptr_Output_v4float %_ %int_0
OpStore %44 %42
%52 = OpAccessChain %_ptr_Uniform_uint %uniform_index_buffer %int_0 %int_0
%53 = OpLoad %uint %52
;CHECK-NOT: %53 = OpLoad %uint %52
;CHECK: {{%\w+}} = OpIMul %uint %uint_16 %int_0
;CHECK: {{%\w+}} = OpIAdd %uint %uint_0 {{%\w+}}
;CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_3
;CHECK: {{%\w+}} = OpLoad %int %gl_VertexIndex
;CHECK: {{%\w+}} = OpBitcast %uint {{%\w+}}
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_87 {{%\w+}} %uint_0 %uint_0 %uint_0 {{%\w+}}
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %uint %52
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %uint {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: OpStore %index [[phi_result]]
OpStore %index %53
;CHECK-NOT: OpStore %index %53
OpReturn
;CHECK: OpReturn
OpFunctionEnd)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 23u);
}

// TODO(greg-lunarg): Add tests to verify handling of these cases:
//
//   Compute shader
//   Geometry shader
//   Tessellation control shader
//   Tessellation eval shader
//   OpImage
//   SampledImage variable

}  // namespace
}  // namespace opt
}  // namespace spvtools
