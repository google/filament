// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG Inc.
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

#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using InstBindlessTest = PassTest<::testing::Test>;

TEST_F(InstBindlessTest, NoInstrumentConstIndexInbounds) {
  // Texture2D g_tColor[128];
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
  //   ps_output.vColor = g_tColor[ 37 ].Sample(g_sAniso, i.vTextureCoords.xy);
  //   return ps_output;
  // }

  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 3
OpDecorate %g_tColor Binding 0
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_37 = OpConstant %int 37
%15 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%_arr_15_uint_128 = OpTypeArray %15 %uint_128
%_ptr_UniformConstant__arr_15_uint_128 = OpTypePointer UniformConstant %_arr_15_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_15_uint_128 UniformConstant
%_ptr_UniformConstant_15 = OpTypePointer UniformConstant %15
%21 = OpTypeSampler
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %21
%g_sAniso = OpVariable %_ptr_UniformConstant_21 UniformConstant
%23 = OpTypeSampledImage %15
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
%MainPs = OpFunction %void None %8
%26 = OpLabel
%27 = OpLoad %v2float %i_vTextureCoords
%28 = OpAccessChain %_ptr_UniformConstant_15 %g_tColor %int_37
%29 = OpLoad %15 %28
%30 = OpLoad %21 %g_sAniso
%31 = OpSampledImage %23 %29 %30
%32 = OpImageSampleImplicitLod %v4float %31 %27
OpStore %_entryPointOutput_vColor %32
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      before, before, true, true, 7u, 23u, false, false, false, false, false);
}

TEST_F(InstBindlessTest, NoInstrumentNonBindless) {
  // This test verifies that the pass will correctly not instrument vanilla
  // texture sample.
  //
  // Texture2D g_tColor;
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
  //       g_tColor.Sample(g_sAniso, i.vTextureCoords.xy);
  //   return ps_output;
  // }

  const std::string whole_file =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 0
OpDecorate %g_tColor Binding 0
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %g_sAniso Binding 0
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
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
%MainPs = OpFunction %void None %8
%19 = OpLabel
%20 = OpLoad %v2float %i_vTextureCoords
%21 = OpLoad %12 %g_tColor
%22 = OpLoad %14 %g_sAniso
%23 = OpSampledImage %16 %21 %22
%24 = OpImageSampleImplicitLod %v4float %23 %20
OpStore %_entryPointOutput_vColor %24
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(whole_file, whole_file, true,
                                               true, 7u, 23u, false, false,
                                               false, false, false);
}

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

  const std::string entry_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
)";

  const std::string entry_after =
      R"(OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
)";

  const std::string names_annots =
      R"(OpName %MainPs "MainPs"
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
OpDecorate %_entryPointOutput_vColor Location 0
)";

  const std::string new_annots =
      R"(OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_55 Block
OpMemberDecorate %_struct_55 0 Offset 0
OpMemberDecorate %_struct_55 1 Offset 4
OpDecorate %57 DescriptorSet 7
OpDecorate %57 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
)";

  const std::string consts_types_vars =
      R"(%void = OpTypeVoid
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
)";

  const std::string new_consts_types_vars =
      R"(%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%48 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_55 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_55 = OpTypePointer StorageBuffer %_struct_55
%57 = OpVariable %_ptr_StorageBuffer__struct_55 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_1 = OpConstant %uint 1
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_56 = OpConstant %uint 56
%103 = OpConstantNull %v4float
)";

  const std::string func_pt1 =
      R"(%MainPs = OpFunction %void None %10
%29 = OpLabel
%30 = OpLoad %v2float %i_vTextureCoords
%31 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%32 = OpLoad %uint %31
%33 = OpAccessChain %_ptr_UniformConstant_16 %g_tColor %32
%34 = OpLoad %16 %33
%35 = OpLoad %24 %g_sAniso
%36 = OpSampledImage %26 %34 %35
)";

  const std::string func_pt2_before =
      R"(%37 = OpImageSampleImplicitLod %v4float %36 %30
OpStore %_entryPointOutput_vColor %37
OpReturn
OpFunctionEnd
)";

  const std::string func_pt2_after =
      R"(%40 = OpULessThan %bool %32 %uint_128
OpSelectionMerge %41 None
OpBranchConditional %40 %42 %43
%42 = OpLabel
%44 = OpLoad %16 %33
%45 = OpSampledImage %26 %44 %35
%46 = OpImageSampleImplicitLod %v4float %45 %30
OpBranch %41
%43 = OpLabel
%102 = OpFunctionCall %void %47 %uint_56 %uint_0 %32 %uint_128
OpBranch %41
%41 = OpLabel
%104 = OpPhi %v4float %46 %42 %103 %43
OpStore %_entryPointOutput_vColor %104
OpReturn
OpFunctionEnd
)";

  const std::string output_func =
      R"(%47 = OpFunction %void None %48
%49 = OpFunctionParameter %uint
%50 = OpFunctionParameter %uint
%51 = OpFunctionParameter %uint
%52 = OpFunctionParameter %uint
%53 = OpLabel
%59 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_0
%62 = OpAtomicIAdd %uint %59 %uint_4 %uint_0 %uint_10
%63 = OpIAdd %uint %62 %uint_10
%64 = OpArrayLength %uint %57 1
%65 = OpULessThanEqual %bool %63 %64
OpSelectionMerge %66 None
OpBranchConditional %65 %67 %66
%67 = OpLabel
%68 = OpIAdd %uint %62 %uint_0
%70 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %68
OpStore %70 %uint_10
%72 = OpIAdd %uint %62 %uint_1
%73 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %72
OpStore %73 %uint_23
%75 = OpIAdd %uint %62 %uint_2
%76 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %75
OpStore %76 %49
%78 = OpIAdd %uint %62 %uint_3
%79 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %78
OpStore %79 %uint_4
%82 = OpLoad %v4float %gl_FragCoord
%84 = OpBitcast %v4uint %82
%85 = OpCompositeExtract %uint %84 0
%86 = OpIAdd %uint %62 %uint_4
%87 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %86
OpStore %87 %85
%88 = OpCompositeExtract %uint %84 1
%90 = OpIAdd %uint %62 %uint_5
%91 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %90
OpStore %91 %88
%93 = OpIAdd %uint %62 %uint_7
%94 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %93
OpStore %94 %50
%96 = OpIAdd %uint %62 %uint_8
%97 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %96
OpStore %97 %51
%99 = OpIAdd %uint %62 %uint_9
%100 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %99
OpStore %100 %52
OpBranch %66
%66 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass, uint32_t, uint32_t, bool, bool>(
      entry_before + names_annots + consts_types_vars + func_pt1 +
          func_pt2_before,
      entry_after + names_annots + new_annots + consts_types_vars +
          new_consts_types_vars + func_pt1 + func_pt2_after + output_func,
      true, true, 7u, 23u, false, false, false, false, false);
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

  const std::string defs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
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
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpMemberDecorate %PerViewConstantBuffer_t 1 Offset 4
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
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
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpMemberDecorate %PerViewConstantBuffer_t 1 Offset 4
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_63 Block
OpMemberDecorate %_struct_63 0 Offset 0
OpMemberDecorate %_struct_63 1 Offset 4
OpDecorate %65 DescriptorSet 7
OpDecorate %65 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
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
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%56 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_63 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_63 = OpTypePointer StorageBuffer %_struct_63
%65 = OpVariable %_ptr_StorageBuffer__struct_63 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_1 = OpConstant %uint 1
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_58 = OpConstant %uint 58
%111 = OpConstantNull %v4float
%uint_64 = OpConstant %uint 64
)";

  const std::string func_before =
      R"(%MainPs = OpFunction %void None %10
%30 = OpLabel
%31 = OpLoad %v2float %i_vTextureCoords
%32 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%33 = OpLoad %uint %32
%34 = OpAccessChain %_ptr_UniformConstant_17 %g_tColor %33
%35 = OpLoad %17 %34
%36 = OpLoad %25 %g_sAniso
%37 = OpSampledImage %27 %35 %36
%38 = OpImageSampleImplicitLod %v4float %37 %31
%39 = OpAccessChain %_ptr_PushConstant_uint %_ %int_1
%40 = OpLoad %uint %39
%41 = OpAccessChain %_ptr_UniformConstant_17 %g_tColor %40
%42 = OpLoad %17 %41
%43 = OpSampledImage %27 %42 %36
%44 = OpImageSampleImplicitLod %v4float %43 %31
%45 = OpFAdd %v4float %38 %44
OpStore %_entryPointOutput_vColor %45
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%MainPs = OpFunction %void None %10
%30 = OpLabel
%31 = OpLoad %v2float %i_vTextureCoords
%32 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%33 = OpLoad %uint %32
%34 = OpAccessChain %_ptr_UniformConstant_17 %g_tColor %33
%35 = OpLoad %17 %34
%36 = OpLoad %25 %g_sAniso
%37 = OpSampledImage %27 %35 %36
%48 = OpULessThan %bool %33 %uint_128
OpSelectionMerge %49 None
OpBranchConditional %48 %50 %51
%50 = OpLabel
%52 = OpLoad %17 %34
%53 = OpSampledImage %27 %52 %36
%54 = OpImageSampleImplicitLod %v4float %53 %31
OpBranch %49
%51 = OpLabel
%110 = OpFunctionCall %void %55 %uint_58 %uint_0 %33 %uint_128
OpBranch %49
%49 = OpLabel
%112 = OpPhi %v4float %54 %50 %111 %51
%39 = OpAccessChain %_ptr_PushConstant_uint %_ %int_1
%40 = OpLoad %uint %39
%41 = OpAccessChain %_ptr_UniformConstant_17 %g_tColor %40
%42 = OpLoad %17 %41
%43 = OpSampledImage %27 %42 %36
%113 = OpULessThan %bool %40 %uint_128
OpSelectionMerge %114 None
OpBranchConditional %113 %115 %116
%115 = OpLabel
%117 = OpLoad %17 %41
%118 = OpSampledImage %27 %117 %36
%119 = OpImageSampleImplicitLod %v4float %118 %31
OpBranch %114
%116 = OpLabel
%121 = OpFunctionCall %void %55 %uint_64 %uint_0 %40 %uint_128
OpBranch %114
%114 = OpLabel
%122 = OpPhi %v4float %119 %115 %111 %116
%45 = OpFAdd %v4float %112 %122
OpStore %_entryPointOutput_vColor %45
OpReturn
OpFunctionEnd
)";

  const std::string output_func =
      R"(%55 = OpFunction %void None %56
%57 = OpFunctionParameter %uint
%58 = OpFunctionParameter %uint
%59 = OpFunctionParameter %uint
%60 = OpFunctionParameter %uint
%61 = OpLabel
%67 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_0
%70 = OpAtomicIAdd %uint %67 %uint_4 %uint_0 %uint_10
%71 = OpIAdd %uint %70 %uint_10
%72 = OpArrayLength %uint %65 1
%73 = OpULessThanEqual %bool %71 %72
OpSelectionMerge %74 None
OpBranchConditional %73 %75 %74
%75 = OpLabel
%76 = OpIAdd %uint %70 %uint_0
%78 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %76
OpStore %78 %uint_10
%80 = OpIAdd %uint %70 %uint_1
%81 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %80
OpStore %81 %uint_23
%83 = OpIAdd %uint %70 %uint_2
%84 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %83
OpStore %84 %57
%86 = OpIAdd %uint %70 %uint_3
%87 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %86
OpStore %87 %uint_4
%90 = OpLoad %v4float %gl_FragCoord
%92 = OpBitcast %v4uint %90
%93 = OpCompositeExtract %uint %92 0
%94 = OpIAdd %uint %70 %uint_4
%95 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %94
OpStore %95 %93
%96 = OpCompositeExtract %uint %92 1
%98 = OpIAdd %uint %70 %uint_5
%99 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %98
OpStore %99 %96
%101 = OpIAdd %uint %70 %uint_7
%102 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %101
OpStore %102 %58
%104 = OpIAdd %uint %70 %uint_8
%105 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %104
OpStore %105 %59
%107 = OpIAdd %uint %70 %uint_9
%108 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %107
OpStore %108 %60
OpBranch %74
%74 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + output_func, true,
      true, 7u, 23u, false, false, false, false, false);
}

TEST_F(InstBindlessTest, InstrumentOpImage) {
  // This test verifies that the pass will correctly instrument shader
  // using OpImage. This test was created by editing the SPIR-V
  // from the Simple test.

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability StorageImageReadWithoutFormat
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
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
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability StorageImageReadWithoutFormat
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
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
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_51 Block
OpMemberDecorate %_struct_51 0 Offset 0
OpMemberDecorate %_struct_51 1 Offset 4
OpDecorate %53 DescriptorSet 7
OpDecorate %53 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%int_0 = OpConstant %int 0
%15 = OpTypeImage %float 2D 0 0 0 0 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%18 = OpTypeSampledImage %15
%_arr_18_uint_128 = OpTypeArray %18 %uint_128
%_ptr_UniformConstant__arr_18_uint_128 = OpTypePointer UniformConstant %_arr_18_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_18_uint_128 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_18 = OpTypePointer UniformConstant %18
%_ptr_Input_v2int = OpTypePointer Input %v2int
%i_vTextureCoords = OpVariable %_ptr_Input_v2int Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%44 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_51 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_51 = OpTypePointer StorageBuffer %_struct_51
%53 = OpVariable %_ptr_StorageBuffer__struct_51 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_1 = OpConstant %uint 1
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_51 = OpConstant %uint 51
%99 = OpConstantNull %v4float
)";

  const std::string func_before =
      R"(%MainPs = OpFunction %void None %3
%5 = OpLabel
%53 = OpLoad %v2int %i_vTextureCoords
%63 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%64 = OpLoad %uint %63
%65 = OpAccessChain %_ptr_UniformConstant_39 %g_tColor %64
%66 = OpLoad %39 %65
%75 = OpImage %20 %66
%71 = OpImageRead %v4float %75 %53
OpStore %_entryPointOutput_vColor %71
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%MainPs = OpFunction %void None %9
%26 = OpLabel
%27 = OpLoad %v2int %i_vTextureCoords
%28 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%29 = OpLoad %uint %28
%30 = OpAccessChain %_ptr_UniformConstant_18 %g_tColor %29
%31 = OpLoad %18 %30
%32 = OpImage %15 %31
%36 = OpULessThan %bool %29 %uint_128
OpSelectionMerge %37 None
OpBranchConditional %36 %38 %39
%38 = OpLabel
%40 = OpLoad %18 %30
%41 = OpImage %15 %40
%42 = OpImageRead %v4float %41 %27
OpBranch %37
%39 = OpLabel
%98 = OpFunctionCall %void %43 %uint_51 %uint_0 %29 %uint_128
OpBranch %37
%37 = OpLabel
%100 = OpPhi %v4float %42 %38 %99 %39
OpStore %_entryPointOutput_vColor %100
OpReturn
OpFunctionEnd
)";

  const std::string output_func =
      R"(%43 = OpFunction %void None %44
%45 = OpFunctionParameter %uint
%46 = OpFunctionParameter %uint
%47 = OpFunctionParameter %uint
%48 = OpFunctionParameter %uint
%49 = OpLabel
%55 = OpAccessChain %_ptr_StorageBuffer_uint %53 %uint_0
%58 = OpAtomicIAdd %uint %55 %uint_4 %uint_0 %uint_10
%59 = OpIAdd %uint %58 %uint_10
%60 = OpArrayLength %uint %53 1
%61 = OpULessThanEqual %bool %59 %60
OpSelectionMerge %62 None
OpBranchConditional %61 %63 %62
%63 = OpLabel
%64 = OpIAdd %uint %58 %uint_0
%66 = OpAccessChain %_ptr_StorageBuffer_uint %53 %uint_1 %64
OpStore %66 %uint_10
%68 = OpIAdd %uint %58 %uint_1
%69 = OpAccessChain %_ptr_StorageBuffer_uint %53 %uint_1 %68
OpStore %69 %uint_23
%71 = OpIAdd %uint %58 %uint_2
%72 = OpAccessChain %_ptr_StorageBuffer_uint %53 %uint_1 %71
OpStore %72 %45
%74 = OpIAdd %uint %58 %uint_3
%75 = OpAccessChain %_ptr_StorageBuffer_uint %53 %uint_1 %74
OpStore %75 %uint_4
%78 = OpLoad %v4float %gl_FragCoord
%80 = OpBitcast %v4uint %78
%81 = OpCompositeExtract %uint %80 0
%82 = OpIAdd %uint %58 %uint_4
%83 = OpAccessChain %_ptr_StorageBuffer_uint %53 %uint_1 %82
OpStore %83 %81
%84 = OpCompositeExtract %uint %80 1
%86 = OpIAdd %uint %58 %uint_5
%87 = OpAccessChain %_ptr_StorageBuffer_uint %53 %uint_1 %86
OpStore %87 %84
%89 = OpIAdd %uint %58 %uint_7
%90 = OpAccessChain %_ptr_StorageBuffer_uint %53 %uint_1 %89
OpStore %90 %46
%92 = OpIAdd %uint %58 %uint_8
%93 = OpAccessChain %_ptr_StorageBuffer_uint %53 %uint_1 %92
OpStore %93 %47
%95 = OpIAdd %uint %58 %uint_9
%96 = OpAccessChain %_ptr_StorageBuffer_uint %53 %uint_1 %95
OpStore %96 %48
OpBranch %62
%62 = OpLabel
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + output_func, true,
      true, 7u, 23u, false, false, false, false, false);
}

TEST_F(InstBindlessTest, InstrumentSampledImage) {
  // This test verifies that the pass will correctly instrument shader
  // using sampled image. This test was created by editing the SPIR-V
  // from the Simple test.

  const std::string defs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
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
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
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
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_49 Block
OpMemberDecorate %_struct_49 0 Offset 0
OpMemberDecorate %_struct_49 1 Offset 4
OpDecorate %51 DescriptorSet 7
OpDecorate %51 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%15 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%18 = OpTypeSampledImage %15
%_arr_18_uint_128 = OpTypeArray %18 %uint_128
%_ptr_UniformConstant__arr_18_uint_128 = OpTypePointer UniformConstant %_arr_18_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_18_uint_128 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_18 = OpTypePointer UniformConstant %18
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%42 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_49 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_49 = OpTypePointer StorageBuffer %_struct_49
%51 = OpVariable %_ptr_StorageBuffer__struct_49 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_1 = OpConstant %uint 1
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_49 = OpConstant %uint 49
%97 = OpConstantNull %v4float
)";

  const std::string func_before =
      R"(%MainPs = OpFunction %void None %3
%5 = OpLabel
%53 = OpLoad %v2float %i_vTextureCoords
%63 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%64 = OpLoad %uint %63
%65 = OpAccessChain %_ptr_UniformConstant_39 %g_tColor %64
%66 = OpLoad %39 %65
%71 = OpImageSampleImplicitLod %v4float %66 %53
OpStore %_entryPointOutput_vColor %71
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%MainPs = OpFunction %void None %9
%26 = OpLabel
%27 = OpLoad %v2float %i_vTextureCoords
%28 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%29 = OpLoad %uint %28
%30 = OpAccessChain %_ptr_UniformConstant_18 %g_tColor %29
%31 = OpLoad %18 %30
%35 = OpULessThan %bool %29 %uint_128
OpSelectionMerge %36 None
OpBranchConditional %35 %37 %38
%37 = OpLabel
%39 = OpLoad %18 %30
%40 = OpImageSampleImplicitLod %v4float %39 %27
OpBranch %36
%38 = OpLabel
%96 = OpFunctionCall %void %41 %uint_49 %uint_0 %29 %uint_128
OpBranch %36
%36 = OpLabel
%98 = OpPhi %v4float %40 %37 %97 %38
OpStore %_entryPointOutput_vColor %98
OpReturn
OpFunctionEnd
)";

  const std::string output_func =
      R"(%41 = OpFunction %void None %42
%43 = OpFunctionParameter %uint
%44 = OpFunctionParameter %uint
%45 = OpFunctionParameter %uint
%46 = OpFunctionParameter %uint
%47 = OpLabel
%53 = OpAccessChain %_ptr_StorageBuffer_uint %51 %uint_0
%56 = OpAtomicIAdd %uint %53 %uint_4 %uint_0 %uint_10
%57 = OpIAdd %uint %56 %uint_10
%58 = OpArrayLength %uint %51 1
%59 = OpULessThanEqual %bool %57 %58
OpSelectionMerge %60 None
OpBranchConditional %59 %61 %60
%61 = OpLabel
%62 = OpIAdd %uint %56 %uint_0
%64 = OpAccessChain %_ptr_StorageBuffer_uint %51 %uint_1 %62
OpStore %64 %uint_10
%66 = OpIAdd %uint %56 %uint_1
%67 = OpAccessChain %_ptr_StorageBuffer_uint %51 %uint_1 %66
OpStore %67 %uint_23
%69 = OpIAdd %uint %56 %uint_2
%70 = OpAccessChain %_ptr_StorageBuffer_uint %51 %uint_1 %69
OpStore %70 %43
%72 = OpIAdd %uint %56 %uint_3
%73 = OpAccessChain %_ptr_StorageBuffer_uint %51 %uint_1 %72
OpStore %73 %uint_4
%76 = OpLoad %v4float %gl_FragCoord
%78 = OpBitcast %v4uint %76
%79 = OpCompositeExtract %uint %78 0
%80 = OpIAdd %uint %56 %uint_4
%81 = OpAccessChain %_ptr_StorageBuffer_uint %51 %uint_1 %80
OpStore %81 %79
%82 = OpCompositeExtract %uint %78 1
%84 = OpIAdd %uint %56 %uint_5
%85 = OpAccessChain %_ptr_StorageBuffer_uint %51 %uint_1 %84
OpStore %85 %82
%87 = OpIAdd %uint %56 %uint_7
%88 = OpAccessChain %_ptr_StorageBuffer_uint %51 %uint_1 %87
OpStore %88 %44
%90 = OpIAdd %uint %56 %uint_8
%91 = OpAccessChain %_ptr_StorageBuffer_uint %51 %uint_1 %90
OpStore %91 %45
%93 = OpIAdd %uint %56 %uint_9
%94 = OpAccessChain %_ptr_StorageBuffer_uint %51 %uint_1 %93
OpStore %94 %46
OpBranch %60
%60 = OpLabel
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + output_func, true,
      true, 7u, 23u, false, false, false, false, false);
}

TEST_F(InstBindlessTest, InstrumentImageWrite) {
  // This test verifies that the pass will correctly instrument shader
  // doing bindless image write. This test was created by editing the SPIR-V
  // from the Simple test.

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability StorageImageWriteWithoutFormat
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
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
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability StorageImageWriteWithoutFormat
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
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
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_48 Block
OpMemberDecorate %_struct_48 0 Offset 0
OpMemberDecorate %_struct_48 1 Offset 4
OpDecorate %50 DescriptorSet 7
OpDecorate %50 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%int_0 = OpConstant %int 0
%16 = OpTypeImage %float 2D 0 0 0 0 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%19 = OpConstantNull %v4float
%_arr_16_uint_128 = OpTypeArray %16 %uint_128
%_ptr_UniformConstant__arr_16_uint_128 = OpTypePointer UniformConstant %_arr_16_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_16_uint_128 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
%_ptr_Input_v2int = OpTypePointer Input %v2int
%i_vTextureCoords = OpVariable %_ptr_Input_v2int Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%41 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_48 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_48 = OpTypePointer StorageBuffer %_struct_48
%50 = OpVariable %_ptr_StorageBuffer__struct_48 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_1 = OpConstant %uint 1
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_51 = OpConstant %uint 51
)";

  const std::string func_before =
      R"(%MainPs = OpFunction %void None %3
%5 = OpLabel
%53 = OpLoad %v2int %i_vTextureCoords
%63 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%64 = OpLoad %uint %63
%65 = OpAccessChain %_ptr_UniformConstant_20 %g_tColor %64
%66 = OpLoad %20 %65
OpImageWrite %66 %53 %80
OpStore %_entryPointOutput_vColor %80
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%MainPs = OpFunction %void None %9
%27 = OpLabel
%28 = OpLoad %v2int %i_vTextureCoords
%29 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%30 = OpLoad %uint %29
%31 = OpAccessChain %_ptr_UniformConstant_16 %g_tColor %30
%32 = OpLoad %16 %31
%35 = OpULessThan %bool %30 %uint_128
OpSelectionMerge %36 None
OpBranchConditional %35 %37 %38
%37 = OpLabel
%39 = OpLoad %16 %31
OpImageWrite %39 %28 %19
OpBranch %36
%38 = OpLabel
%95 = OpFunctionCall %void %40 %uint_51 %uint_0 %30 %uint_128
OpBranch %36
%36 = OpLabel
OpStore %_entryPointOutput_vColor %19
OpReturn
OpFunctionEnd
)";

  const std::string output_func =
      R"(%40 = OpFunction %void None %41
%42 = OpFunctionParameter %uint
%43 = OpFunctionParameter %uint
%44 = OpFunctionParameter %uint
%45 = OpFunctionParameter %uint
%46 = OpLabel
%52 = OpAccessChain %_ptr_StorageBuffer_uint %50 %uint_0
%55 = OpAtomicIAdd %uint %52 %uint_4 %uint_0 %uint_10
%56 = OpIAdd %uint %55 %uint_10
%57 = OpArrayLength %uint %50 1
%58 = OpULessThanEqual %bool %56 %57
OpSelectionMerge %59 None
OpBranchConditional %58 %60 %59
%60 = OpLabel
%61 = OpIAdd %uint %55 %uint_0
%63 = OpAccessChain %_ptr_StorageBuffer_uint %50 %uint_1 %61
OpStore %63 %uint_10
%65 = OpIAdd %uint %55 %uint_1
%66 = OpAccessChain %_ptr_StorageBuffer_uint %50 %uint_1 %65
OpStore %66 %uint_23
%68 = OpIAdd %uint %55 %uint_2
%69 = OpAccessChain %_ptr_StorageBuffer_uint %50 %uint_1 %68
OpStore %69 %42
%71 = OpIAdd %uint %55 %uint_3
%72 = OpAccessChain %_ptr_StorageBuffer_uint %50 %uint_1 %71
OpStore %72 %uint_4
%75 = OpLoad %v4float %gl_FragCoord
%77 = OpBitcast %v4uint %75
%78 = OpCompositeExtract %uint %77 0
%79 = OpIAdd %uint %55 %uint_4
%80 = OpAccessChain %_ptr_StorageBuffer_uint %50 %uint_1 %79
OpStore %80 %78
%81 = OpCompositeExtract %uint %77 1
%83 = OpIAdd %uint %55 %uint_5
%84 = OpAccessChain %_ptr_StorageBuffer_uint %50 %uint_1 %83
OpStore %84 %81
%86 = OpIAdd %uint %55 %uint_7
%87 = OpAccessChain %_ptr_StorageBuffer_uint %50 %uint_1 %86
OpStore %87 %43
%89 = OpIAdd %uint %55 %uint_8
%90 = OpAccessChain %_ptr_StorageBuffer_uint %50 %uint_1 %89
OpStore %90 %44
%92 = OpIAdd %uint %55 %uint_9
%93 = OpAccessChain %_ptr_StorageBuffer_uint %50 %uint_1 %92
OpStore %93 %45
OpBranch %59
%59 = OpLabel
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + output_func, true,
      true, 7u, 23u, false, false, false, false, false);
}

TEST_F(InstBindlessTest, InstrumentVertexSimple) {
  // This test verifies that the pass will correctly instrument shader
  // doing bindless image write. This test was created by editing the SPIR-V
  // from the Simple test.

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability Sampled1D
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
OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
OpDecorate %gl_PerVertex Block
OpDecorate %texSampler1D DescriptorSet 0
OpDecorate %texSampler1D Binding 3
OpMemberDecorate %foo 0 Offset 0
OpDecorate %foo Block
OpDecorate %__0 DescriptorSet 0
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability Sampled1D
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %_ %coords2D %gl_VertexIndex %gl_InstanceIndex
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
OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
OpDecorate %gl_PerVertex Block
OpDecorate %texSampler1D DescriptorSet 0
OpDecorate %texSampler1D Binding 3
OpMemberDecorate %foo 0 Offset 0
OpDecorate %foo Block
OpDecorate %__0 DescriptorSet 0
OpDecorate %__0 Binding 5
OpDecorate %coords2D Location 0
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_61 Block
OpMemberDecorate %_struct_61 0 Offset 0
OpMemberDecorate %_struct_61 1 Offset 4
OpDecorate %63 DescriptorSet 7
OpDecorate %63 Binding 0
OpDecorate %gl_VertexIndex BuiltIn VertexIndex
OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
%void = OpTypeVoid
%12 = OpTypeFunction %void
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
%24 = OpTypeImage %float 1D 0 0 0 1 Unknown
%25 = OpTypeSampledImage %24
%uint_128 = OpConstant %uint 128
%_arr_25_uint_128 = OpTypeArray %25 %uint_128
%_ptr_UniformConstant__arr_25_uint_128 = OpTypePointer UniformConstant %_arr_25_uint_128
%texSampler1D = OpVariable %_ptr_UniformConstant__arr_25_uint_128 UniformConstant
%foo = OpTypeStruct %int
%_ptr_Uniform_foo = OpTypePointer Uniform %foo
%__0 = OpVariable %_ptr_Uniform_foo Uniform
%_ptr_Uniform_int = OpTypePointer Uniform %int
%_ptr_UniformConstant_25 = OpTypePointer UniformConstant %25
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%coords2D = OpVariable %_ptr_Input_v2float Input
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%54 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_61 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_61 = OpTypePointer StorageBuffer %_struct_61
%63 = OpVariable %_ptr_StorageBuffer__struct_61 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%_ptr_Input_uint = OpTypePointer Input %uint
%gl_VertexIndex = OpVariable %_ptr_Input_uint Input
%gl_InstanceIndex = OpVariable %_ptr_Input_uint Input
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_74 = OpConstant %uint 74
%106 = OpConstantNull %v4float
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
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
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %12
%35 = OpLabel
%lod = OpVariable %_ptr_Function_float Function
%coords1D = OpVariable %_ptr_Function_float Function
OpStore %lod %float_3
OpStore %coords1D %float_1_78900003
%36 = OpAccessChain %_ptr_Uniform_int %__0 %int_0
%37 = OpLoad %int %36
%38 = OpAccessChain %_ptr_UniformConstant_25 %texSampler1D %37
%39 = OpLoad %25 %38
%40 = OpLoad %float %coords1D
%41 = OpLoad %float %lod
%46 = OpULessThan %bool %37 %uint_128
OpSelectionMerge %47 None
OpBranchConditional %46 %48 %49
%48 = OpLabel
%50 = OpLoad %25 %38
%51 = OpImageSampleExplicitLod %v4float %50 %40 Lod %41
OpBranch %47
%49 = OpLabel
%52 = OpBitcast %uint %37
%105 = OpFunctionCall %void %53 %uint_74 %uint_0 %52 %uint_128
OpBranch %47
%47 = OpLabel
%107 = OpPhi %v4float %51 %48 %106 %49
%43 = OpAccessChain %_ptr_Output_v4float %_ %int_0
OpStore %43 %107
OpReturn
OpFunctionEnd
)";

  const std::string output_func =
      R"(%53 = OpFunction %void None %54
%55 = OpFunctionParameter %uint
%56 = OpFunctionParameter %uint
%57 = OpFunctionParameter %uint
%58 = OpFunctionParameter %uint
%59 = OpLabel
%65 = OpAccessChain %_ptr_StorageBuffer_uint %63 %uint_0
%68 = OpAtomicIAdd %uint %65 %uint_4 %uint_0 %uint_10
%69 = OpIAdd %uint %68 %uint_10
%70 = OpArrayLength %uint %63 1
%71 = OpULessThanEqual %bool %69 %70
OpSelectionMerge %72 None
OpBranchConditional %71 %73 %72
%73 = OpLabel
%74 = OpIAdd %uint %68 %uint_0
%75 = OpAccessChain %_ptr_StorageBuffer_uint %63 %uint_1 %74
OpStore %75 %uint_10
%77 = OpIAdd %uint %68 %uint_1
%78 = OpAccessChain %_ptr_StorageBuffer_uint %63 %uint_1 %77
OpStore %78 %uint_23
%80 = OpIAdd %uint %68 %uint_2
%81 = OpAccessChain %_ptr_StorageBuffer_uint %63 %uint_1 %80
OpStore %81 %55
%83 = OpIAdd %uint %68 %uint_3
%84 = OpAccessChain %_ptr_StorageBuffer_uint %63 %uint_1 %83
OpStore %84 %uint_0
%87 = OpLoad %uint %gl_VertexIndex
%88 = OpIAdd %uint %68 %uint_4
%89 = OpAccessChain %_ptr_StorageBuffer_uint %63 %uint_1 %88
OpStore %89 %87
%91 = OpLoad %uint %gl_InstanceIndex
%93 = OpIAdd %uint %68 %uint_5
%94 = OpAccessChain %_ptr_StorageBuffer_uint %63 %uint_1 %93
OpStore %94 %91
%96 = OpIAdd %uint %68 %uint_7
%97 = OpAccessChain %_ptr_StorageBuffer_uint %63 %uint_1 %96
OpStore %97 %56
%99 = OpIAdd %uint %68 %uint_8
%100 = OpAccessChain %_ptr_StorageBuffer_uint %63 %uint_1 %99
OpStore %100 %57
%102 = OpIAdd %uint %68 %uint_9
%103 = OpAccessChain %_ptr_StorageBuffer_uint %63 %uint_1 %102
OpStore %103 %58
OpBranch %72
%72 = OpLabel
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + output_func, true,
      true, 7u, 23u, false, false, false, false, false);
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
  // layout(std140, set = 0, binding = 0) uniform ufoo { uint index; } uniform_index_buffer;
  //
  // layout(set = 0, binding = 1) buffer bfoo { vec4 val; } adds[11];
  //
  // layout(triangles, equal_spacing, cw) in;
  //
  // void main() {
  //   gl_Position = adds[uniform_index_buffer.index].val;
  // }
  //
  // clang-format on

  const std::string defs_before =
      R"(OpCapability Tessellation
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main" %_
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
OpDecorate %adds DescriptorSet 0
OpDecorate %adds Binding 1
OpMemberDecorate %ufoo 0 Offset 0
OpDecorate %ufoo Block
OpDecorate %uniform_index_buffer DescriptorSet 0
OpDecorate %uniform_index_buffer Binding 0
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
)";

  const std::string defs_after =
      R"(OpCapability Tessellation
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main" %_ %gl_PrimitiveID %gl_TessCoord
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
OpDecorate %adds DescriptorSet 0
OpDecorate %adds Binding 1
OpMemberDecorate %ufoo 0 Offset 0
OpDecorate %ufoo Block
OpDecorate %uniform_index_buffer DescriptorSet 0
OpDecorate %uniform_index_buffer Binding 0
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_47 Block
OpMemberDecorate %_struct_47 0 Offset 0
OpMemberDecorate %_struct_47 1 Offset 4
OpDecorate %49 DescriptorSet 7
OpDecorate %49 Binding 0
OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
OpDecorate %gl_TessCoord BuiltIn TessCoord
%void = OpTypeVoid
%10 = OpTypeFunction %void
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
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%40 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_47 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_47 = OpTypePointer StorageBuffer %_struct_47
%49 = OpVariable %_ptr_StorageBuffer__struct_47 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%_ptr_Input_uint = OpTypePointer Input %uint
%gl_PrimitiveID = OpVariable %_ptr_Input_uint Input
%v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
%gl_TessCoord = OpVariable %_ptr_Input_v3float Input
%v3uint = OpTypeVector %uint 3
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_63 = OpConstant %uint 63
%101 = OpConstantNull %v4float
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_uint %uniform_index_buffer %int_0
%26 = OpLoad %uint %25
%28 = OpAccessChain %_ptr_StorageBuffer_v4float %adds %26 %int_0
%29 = OpLoad %v4float %28
%31 = OpAccessChain %_ptr_Output_v4float %_ %int_0
OpStore %31 %29
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %10
%26 = OpLabel
%27 = OpAccessChain %_ptr_Uniform_uint %uniform_index_buffer %int_0
%28 = OpLoad %uint %27
%29 = OpAccessChain %_ptr_StorageBuffer_v4float %adds %28 %int_0
%34 = OpULessThan %bool %28 %uint_11
OpSelectionMerge %35 None
OpBranchConditional %34 %36 %37
%36 = OpLabel
%38 = OpLoad %v4float %29
OpBranch %35
%37 = OpLabel
%100 = OpFunctionCall %void %39 %uint_63 %uint_0 %28 %uint_11
OpBranch %35
%35 = OpLabel
%102 = OpPhi %v4float %38 %36 %101 %37
%31 = OpAccessChain %_ptr_Output_v4float %_ %int_0
OpStore %31 %102
OpReturn
OpFunctionEnd
)";

  const std::string output_func =
      R"(%39 = OpFunction %void None %40
%41 = OpFunctionParameter %uint
%42 = OpFunctionParameter %uint
%43 = OpFunctionParameter %uint
%44 = OpFunctionParameter %uint
%45 = OpLabel
%51 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_0
%54 = OpAtomicIAdd %uint %51 %uint_4 %uint_0 %uint_10
%55 = OpIAdd %uint %54 %uint_10
%56 = OpArrayLength %uint %49 1
%57 = OpULessThanEqual %bool %55 %56
OpSelectionMerge %58 None
OpBranchConditional %57 %59 %58
%59 = OpLabel
%60 = OpIAdd %uint %54 %uint_0
%61 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %60
OpStore %61 %uint_10
%63 = OpIAdd %uint %54 %uint_1
%64 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %63
OpStore %64 %uint_23
%66 = OpIAdd %uint %54 %uint_2
%67 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %66
OpStore %67 %41
%69 = OpIAdd %uint %54 %uint_3
%70 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %69
OpStore %70 %uint_2
%73 = OpLoad %uint %gl_PrimitiveID
%74 = OpIAdd %uint %54 %uint_4
%75 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %74
OpStore %75 %73
%79 = OpLoad %v3float %gl_TessCoord
%81 = OpBitcast %v3uint %79
%82 = OpCompositeExtract %uint %81 0
%83 = OpCompositeExtract %uint %81 1
%85 = OpIAdd %uint %54 %uint_5
%86 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %85
OpStore %86 %82
%88 = OpIAdd %uint %54 %uint_6
%89 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %88
OpStore %89 %83
%91 = OpIAdd %uint %54 %uint_7
%92 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %91
OpStore %92 %42
%94 = OpIAdd %uint %54 %uint_8
%95 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %94
OpStore %95 %43
%97 = OpIAdd %uint %54 %uint_9
%98 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %97
OpStore %98 %44
OpBranch %58
%58 = OpLabel
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + output_func, true,
      true, 7u, 23u, false, false, false, false, false);
}

TEST_F(InstBindlessTest, MultipleDebugFunctions) {
  // Same source as Simple, but compiled -g and not optimized, especially not
  // inlined. The OpSource has had the source extracted for the sake of brevity.

  const std::string defs_before =
      R"(OpCapability Shader
%2 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
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
OpDecorate %g_tColor DescriptorSet 0
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %g_sAniso Binding 1
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
%5 = OpString "foo5.frag"
OpSource HLSL 500 %5
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
OpDecorate %g_tColor DescriptorSet 0
OpDecorate %g_tColor Binding 0
OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
OpDecorate %PerViewConstantBuffer_t Block
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %g_sAniso Binding 1
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_77 Block
OpMemberDecorate %_struct_77 0 Offset 0
OpMemberDecorate %_struct_77 1 Offset 4
OpDecorate %79 DescriptorSet 7
OpDecorate %79 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%18 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%PS_INPUT = OpTypeStruct %v2float
%_ptr_Function_PS_INPUT = OpTypePointer Function %PS_INPUT
%v4float = OpTypeVector %float 4
%PS_OUTPUT = OpTypeStruct %v4float
%23 = OpTypeFunction %PS_OUTPUT %_ptr_Function_PS_INPUT
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%27 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_128 = OpConstant %uint 128
%_arr_27_uint_128 = OpTypeArray %27 %uint_128
%_ptr_UniformConstant__arr_27_uint_128 = OpTypePointer UniformConstant %_arr_27_uint_128
%g_tColor = OpVariable %_ptr_UniformConstant__arr_27_uint_128 UniformConstant
%PerViewConstantBuffer_t = OpTypeStruct %uint
%_ptr_PushConstant_PerViewConstantBuffer_t = OpTypePointer PushConstant %PerViewConstantBuffer_t
%_ = OpVariable %_ptr_PushConstant_PerViewConstantBuffer_t PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_UniformConstant_27 = OpTypePointer UniformConstant %27
%35 = OpTypeSampler
%_ptr_UniformConstant_35 = OpTypePointer UniformConstant %35
%g_sAniso = OpVariable %_ptr_UniformConstant_35 UniformConstant
%37 = OpTypeSampledImage %27
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v2float = OpTypePointer Input %v2float
%i_vTextureCoords = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_vColor = OpVariable %_ptr_Output_v4float Output
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%70 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_77 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_77 = OpTypePointer StorageBuffer %_struct_77
%79 = OpVariable %_ptr_StorageBuffer__struct_77 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_1 = OpConstant %uint 1
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_109 = OpConstant %uint 109
%125 = OpConstantNull %v4float
)";

  const std::string func1_before =
      R"(%MainPs = OpFunction %void None %4
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

  const std::string func1_after =
      R"(%MainPs = OpFunction %void None %18
%42 = OpLabel
%i_0 = OpVariable %_ptr_Function_PS_INPUT Function
%param = OpVariable %_ptr_Function_PS_INPUT Function
OpLine %5 21 0
%43 = OpLoad %v2float %i_vTextureCoords
%44 = OpAccessChain %_ptr_Function_v2float %i_0 %int_0
OpStore %44 %43
%45 = OpLoad %PS_INPUT %i_0
OpStore %param %45
%46 = OpFunctionCall %PS_OUTPUT %_MainPs_struct_PS_INPUT_vf21_ %param
%47 = OpCompositeExtract %v4float %46 0
OpStore %_entryPointOutput_vColor %47
OpReturn
OpFunctionEnd
)";

  const std::string func2_before =
      R"(%_MainPs_struct_PS_INPUT_vf21_ = OpFunction %PS_OUTPUT None %13
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
%47 = OpAccessChain %_ptr_Function_v4float %ps_output %int_0
OpStore %47 %45
OpLine %1 25 0
%48 = OpLoad %PS_OUTPUT %ps_output
OpReturnValue %48
OpFunctionEnd
)";

  const std::string func2_after =
      R"(%_MainPs_struct_PS_INPUT_vf21_ = OpFunction %PS_OUTPUT None %23
%i = OpFunctionParameter %_ptr_Function_PS_INPUT
%48 = OpLabel
%ps_output = OpVariable %_ptr_Function_PS_OUTPUT Function
OpLine %5 24 0
%49 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%50 = OpLoad %uint %49
%51 = OpAccessChain %_ptr_UniformConstant_27 %g_tColor %50
%52 = OpLoad %27 %51
%53 = OpLoad %35 %g_sAniso
%54 = OpSampledImage %37 %52 %53
%55 = OpAccessChain %_ptr_Function_v2float %i %int_0
%56 = OpLoad %v2float %55
OpNoLine
%62 = OpULessThan %bool %50 %uint_128
OpSelectionMerge %63 None
OpBranchConditional %62 %64 %65
%64 = OpLabel
%66 = OpLoad %27 %51
%67 = OpSampledImage %37 %66 %53
OpLine %5 24 0
%68 = OpImageSampleImplicitLod %v4float %67 %56
OpNoLine
OpBranch %63
%65 = OpLabel
%124 = OpFunctionCall %void %69 %uint_109 %uint_0 %50 %uint_128
OpBranch %63
%63 = OpLabel
%126 = OpPhi %v4float %68 %64 %125 %65
OpLine %5 24 0
%58 = OpAccessChain %_ptr_Function_v4float %ps_output %int_0
OpStore %58 %126
OpLine %5 25 0
%59 = OpLoad %PS_OUTPUT %ps_output
OpReturnValue %59
OpFunctionEnd
)";

  const std::string output_func =
      R"(%69 = OpFunction %void None %70
%71 = OpFunctionParameter %uint
%72 = OpFunctionParameter %uint
%73 = OpFunctionParameter %uint
%74 = OpFunctionParameter %uint
%75 = OpLabel
%81 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_0
%84 = OpAtomicIAdd %uint %81 %uint_4 %uint_0 %uint_10
%85 = OpIAdd %uint %84 %uint_10
%86 = OpArrayLength %uint %79 1
%87 = OpULessThanEqual %bool %85 %86
OpSelectionMerge %88 None
OpBranchConditional %87 %89 %88
%89 = OpLabel
%90 = OpIAdd %uint %84 %uint_0
%92 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %90
OpStore %92 %uint_10
%94 = OpIAdd %uint %84 %uint_1
%95 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %94
OpStore %95 %uint_23
%97 = OpIAdd %uint %84 %uint_2
%98 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %97
OpStore %98 %71
%100 = OpIAdd %uint %84 %uint_3
%101 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %100
OpStore %101 %uint_4
%104 = OpLoad %v4float %gl_FragCoord
%106 = OpBitcast %v4uint %104
%107 = OpCompositeExtract %uint %106 0
%108 = OpIAdd %uint %84 %uint_4
%109 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %108
OpStore %109 %107
%110 = OpCompositeExtract %uint %106 1
%112 = OpIAdd %uint %84 %uint_5
%113 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %112
OpStore %113 %110
%115 = OpIAdd %uint %84 %uint_7
%116 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %115
OpStore %116 %72
%118 = OpIAdd %uint %84 %uint_8
%119 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %118
OpStore %119 %73
%121 = OpIAdd %uint %84 %uint_9
%122 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %121
OpStore %122 %74
OpBranch %88
%88 = OpLabel
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func1_before + func2_before,
      defs_after + func1_after + func2_after + output_func, true, true, 7u, 23u,
      false, false, false, false, false);
}

TEST_F(InstBindlessTest, RuntimeArray) {
  // This test verifies that the pass will correctly instrument shader
  // with runtime descriptor array. This test was created by editing the
  // SPIR-V from the Simple test.

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability RuntimeDescriptorArray
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
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
OpDecorate %g_sAniso Binding 0
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability RuntimeDescriptorArray
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
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
OpDecorate %g_sAniso Binding 0
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_46 Block
OpMemberDecorate %_struct_46 0 Offset 0
OpDecorate %48 DescriptorSet 7
OpDecorate %48 Binding 1
OpDecorate %_struct_71 Block
OpMemberDecorate %_struct_71 0 Offset 0
OpMemberDecorate %_struct_71 1 Offset 4
OpDecorate %73 DescriptorSet 7
OpDecorate %73 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%16 = OpTypeImage %float 2D 0 0 0 1 Unknown
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%_runtimearr_16 = OpTypeRuntimeArray %16
%_ptr_UniformConstant__runtimearr_16 = OpTypePointer UniformConstant %_runtimearr_16
%g_tColor = OpVariable %_ptr_UniformConstant__runtimearr_16 UniformConstant
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
%uint_0 = OpConstant %uint 0
%uint_2 = OpConstant %uint 2
%41 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_46 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_46 = OpTypePointer StorageBuffer %_struct_46
%48 = OpVariable %_ptr_StorageBuffer__struct_46 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%65 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_71 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_71 = OpTypePointer StorageBuffer %_struct_71
%73 = OpVariable %_ptr_StorageBuffer__struct_71 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_3 = OpConstant %uint 3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_59 = OpConstant %uint 59
%116 = OpConstantNull %v4float
%119 = OpTypeFunction %uint %uint %uint %uint %uint
)";

  const std::string func_before =
      R"(%MainPs = OpFunction %void None %3
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
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%MainPs = OpFunction %void None %10
%29 = OpLabel
%30 = OpLoad %v2float %i_vTextureCoords
%31 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
%32 = OpLoad %uint %31
%33 = OpAccessChain %_ptr_UniformConstant_16 %g_tColor %32
%34 = OpLoad %16 %33
%35 = OpLoad %24 %g_sAniso
%36 = OpSampledImage %26 %34 %35
%55 = OpFunctionCall %uint %40 %uint_2 %uint_2
%57 = OpULessThan %bool %32 %55
OpSelectionMerge %58 None
OpBranchConditional %57 %59 %60
%59 = OpLabel
%61 = OpLoad %16 %33
%62 = OpSampledImage %26 %61 %35
%136 = OpFunctionCall %uint %118 %uint_0 %uint_1 %uint_2 %32
%137 = OpULessThan %bool %uint_0 %136
OpSelectionMerge %138 None
OpBranchConditional %137 %139 %140
%139 = OpLabel
%141 = OpLoad %16 %33
%142 = OpSampledImage %26 %141 %35
%143 = OpImageSampleImplicitLod %v4float %142 %30
OpBranch %138
%140 = OpLabel
%144 = OpFunctionCall %void %64 %uint_59 %uint_1 %32 %uint_0
OpBranch %138
%138 = OpLabel
%145 = OpPhi %v4float %143 %139 %116 %140
OpBranch %58
%60 = OpLabel
%115 = OpFunctionCall %void %64 %uint_59 %uint_0 %32 %55
OpBranch %58
%58 = OpLabel
%117 = OpPhi %v4float %145 %138 %116 %60
OpStore %_entryPointOutput_vColor %117
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%40 = OpFunction %uint None %41
%42 = OpFunctionParameter %uint
%43 = OpFunctionParameter %uint
%44 = OpLabel
%50 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_0 %42
%51 = OpLoad %uint %50
%52 = OpIAdd %uint %51 %43
%53 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_0 %52
%54 = OpLoad %uint %53
OpReturnValue %54
OpFunctionEnd
%64 = OpFunction %void None %65
%66 = OpFunctionParameter %uint
%67 = OpFunctionParameter %uint
%68 = OpFunctionParameter %uint
%69 = OpFunctionParameter %uint
%70 = OpLabel
%74 = OpAccessChain %_ptr_StorageBuffer_uint %73 %uint_0
%77 = OpAtomicIAdd %uint %74 %uint_4 %uint_0 %uint_10
%78 = OpIAdd %uint %77 %uint_10
%79 = OpArrayLength %uint %73 1
%80 = OpULessThanEqual %bool %78 %79
OpSelectionMerge %81 None
OpBranchConditional %80 %82 %81
%82 = OpLabel
%83 = OpIAdd %uint %77 %uint_0
%84 = OpAccessChain %_ptr_StorageBuffer_uint %73 %uint_1 %83
OpStore %84 %uint_10
%86 = OpIAdd %uint %77 %uint_1
%87 = OpAccessChain %_ptr_StorageBuffer_uint %73 %uint_1 %86
OpStore %87 %uint_23
%88 = OpIAdd %uint %77 %uint_2
%89 = OpAccessChain %_ptr_StorageBuffer_uint %73 %uint_1 %88
OpStore %89 %66
%91 = OpIAdd %uint %77 %uint_3
%92 = OpAccessChain %_ptr_StorageBuffer_uint %73 %uint_1 %91
OpStore %92 %uint_4
%95 = OpLoad %v4float %gl_FragCoord
%97 = OpBitcast %v4uint %95
%98 = OpCompositeExtract %uint %97 0
%99 = OpIAdd %uint %77 %uint_4
%100 = OpAccessChain %_ptr_StorageBuffer_uint %73 %uint_1 %99
OpStore %100 %98
%101 = OpCompositeExtract %uint %97 1
%103 = OpIAdd %uint %77 %uint_5
%104 = OpAccessChain %_ptr_StorageBuffer_uint %73 %uint_1 %103
OpStore %104 %101
%106 = OpIAdd %uint %77 %uint_7
%107 = OpAccessChain %_ptr_StorageBuffer_uint %73 %uint_1 %106
OpStore %107 %67
%109 = OpIAdd %uint %77 %uint_8
%110 = OpAccessChain %_ptr_StorageBuffer_uint %73 %uint_1 %109
OpStore %110 %68
%112 = OpIAdd %uint %77 %uint_9
%113 = OpAccessChain %_ptr_StorageBuffer_uint %73 %uint_1 %112
OpStore %113 %69
OpBranch %81
%81 = OpLabel
OpReturn
OpFunctionEnd
%118 = OpFunction %uint None %119
%120 = OpFunctionParameter %uint
%121 = OpFunctionParameter %uint
%122 = OpFunctionParameter %uint
%123 = OpFunctionParameter %uint
%124 = OpLabel
%125 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_0 %120
%126 = OpLoad %uint %125
%127 = OpIAdd %uint %126 %121
%128 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_0 %127
%129 = OpLoad %uint %128
%130 = OpIAdd %uint %129 %122
%131 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_0 %130
%132 = OpLoad %uint %131
%133 = OpIAdd %uint %132 %123
%134 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_0 %133
%135 = OpLoad %uint %134
OpReturnValue %135
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest, InstrumentInitCheckOnScalarDescriptor) {
  // This test verifies that the pass will correctly instrument vanilla
  // texture sample on a scalar descriptor with an initialization check if the
  // input_init_enable argument is set to true. This can happen when the
  // descriptor indexing extension is enabled in the API but the SPIR-V
  // does not have the extension enabled because it does not contain a
  // runtime array. This is the same shader as NoInstrumentNonBindless.

  const std::string defs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 0
OpDecorate %g_tColor Binding 0
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %g_sAniso Binding 0
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %i_vTextureCoords %_entryPointOutput_vColor %gl_FragCoord
OpExecutionMode %MainPs OriginUpperLeft
OpSource HLSL 500
OpName %MainPs "MainPs"
OpName %g_tColor "g_tColor"
OpName %g_sAniso "g_sAniso"
OpName %i_vTextureCoords "i.vTextureCoords"
OpName %_entryPointOutput_vColor "@entryPointOutput.vColor"
OpDecorate %g_tColor DescriptorSet 0
OpDecorate %g_tColor Binding 0
OpDecorate %g_sAniso DescriptorSet 0
OpDecorate %g_sAniso Binding 0
OpDecorate %i_vTextureCoords Location 0
OpDecorate %_entryPointOutput_vColor Location 0
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_35 Block
OpMemberDecorate %_struct_35 0 Offset 0
OpDecorate %37 DescriptorSet 7
OpDecorate %37 Binding 1
OpDecorate %_struct_67 Block
OpMemberDecorate %_struct_67 0 Offset 0
OpMemberDecorate %_struct_67 1 Offset 4
OpDecorate %69 DescriptorSet 7
OpDecorate %69 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
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
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%28 = OpTypeFunction %uint %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_35 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_35 = OpTypePointer StorageBuffer %_struct_35
%37 = OpVariable %_ptr_StorageBuffer__struct_35 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%uint_1 = OpConstant %uint 1
%61 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_67 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_67 = OpTypePointer StorageBuffer %_struct_67
%69 = OpVariable %_ptr_StorageBuffer__struct_67 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_39 = OpConstant %uint 39
%113 = OpConstantNull %v4float
)";

  const std::string func_before =
      R"(%MainPs = OpFunction %void None %8
%19 = OpLabel
%20 = OpLoad %v2float %i_vTextureCoords
%21 = OpLoad %12 %g_tColor
%22 = OpLoad %14 %g_sAniso
%23 = OpSampledImage %16 %21 %22
%24 = OpImageSampleImplicitLod %v4float %23 %20
OpStore %_entryPointOutput_vColor %24
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%MainPs = OpFunction %void None %8
%19 = OpLabel
%20 = OpLoad %v2float %i_vTextureCoords
%21 = OpLoad %12 %g_tColor
%22 = OpLoad %14 %g_sAniso
%23 = OpSampledImage %16 %21 %22
%50 = OpFunctionCall %uint %27 %uint_0 %uint_0 %uint_0 %uint_0
%52 = OpULessThan %bool %uint_0 %50
OpSelectionMerge %54 None
OpBranchConditional %52 %55 %56
%55 = OpLabel
%57 = OpLoad %12 %g_tColor
%58 = OpSampledImage %16 %57 %22
%59 = OpImageSampleImplicitLod %v4float %58 %20
OpBranch %54
%56 = OpLabel
%112 = OpFunctionCall %void %60 %uint_39 %uint_1 %uint_0 %uint_0
OpBranch %54
%54 = OpLabel
%114 = OpPhi %v4float %59 %55 %113 %56
OpStore %_entryPointOutput_vColor %114
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%27 = OpFunction %uint None %28
%29 = OpFunctionParameter %uint
%30 = OpFunctionParameter %uint
%31 = OpFunctionParameter %uint
%32 = OpFunctionParameter %uint
%33 = OpLabel
%39 = OpAccessChain %_ptr_StorageBuffer_uint %37 %uint_0 %29
%40 = OpLoad %uint %39
%41 = OpIAdd %uint %40 %30
%42 = OpAccessChain %_ptr_StorageBuffer_uint %37 %uint_0 %41
%43 = OpLoad %uint %42
%44 = OpIAdd %uint %43 %31
%45 = OpAccessChain %_ptr_StorageBuffer_uint %37 %uint_0 %44
%46 = OpLoad %uint %45
%47 = OpIAdd %uint %46 %32
%48 = OpAccessChain %_ptr_StorageBuffer_uint %37 %uint_0 %47
%49 = OpLoad %uint %48
OpReturnValue %49
OpFunctionEnd
%60 = OpFunction %void None %61
%62 = OpFunctionParameter %uint
%63 = OpFunctionParameter %uint
%64 = OpFunctionParameter %uint
%65 = OpFunctionParameter %uint
%66 = OpLabel
%70 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_0
%73 = OpAtomicIAdd %uint %70 %uint_4 %uint_0 %uint_10
%74 = OpIAdd %uint %73 %uint_10
%75 = OpArrayLength %uint %69 1
%76 = OpULessThanEqual %bool %74 %75
OpSelectionMerge %77 None
OpBranchConditional %76 %78 %77
%78 = OpLabel
%79 = OpIAdd %uint %73 %uint_0
%80 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_1 %79
OpStore %80 %uint_10
%82 = OpIAdd %uint %73 %uint_1
%83 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_1 %82
OpStore %83 %uint_23
%85 = OpIAdd %uint %73 %uint_2
%86 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_1 %85
OpStore %86 %62
%88 = OpIAdd %uint %73 %uint_3
%89 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_1 %88
OpStore %89 %uint_4
%92 = OpLoad %v4float %gl_FragCoord
%94 = OpBitcast %v4uint %92
%95 = OpCompositeExtract %uint %94 0
%96 = OpIAdd %uint %73 %uint_4
%97 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_1 %96
OpStore %97 %95
%98 = OpCompositeExtract %uint %94 1
%100 = OpIAdd %uint %73 %uint_5
%101 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_1 %100
OpStore %101 %98
%103 = OpIAdd %uint %73 %uint_7
%104 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_1 %103
OpStore %104 %63
%106 = OpIAdd %uint %73 %uint_8
%107 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_1 %106
OpStore %107 %64
%109 = OpIAdd %uint %73 %uint_9
%110 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_1 %109
OpStore %110 %65
OpBranch %77
%77 = OpLabel
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest, SPV14AddToEntryPoint) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment {{%\w+}} "foo" {{%\w+}} {{%\w+}} {{%\w+}} [[v1:%\w+]] [[v2:%\w+]]
; CHECK: OpDecorate [[v1]] DescriptorSet 7
; CHECK: OpDecorate [[v2]] DescriptorSet 7
; CHECK: [[v1]] = OpVariable {{%\w+}} StorageBuffer
; CHECK: [[v2]] = OpVariable {{%\w+}} StorageBuffer
OpCapability Shader
OpExtension "SPV_EXT_descriptor_indexing"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %foo "foo" %gid %image_var %sampler_var
OpExecutionMode %foo OriginUpperLeft
OpDecorate %image_var DescriptorSet 0
OpDecorate %image_var Binding 0
OpDecorate %sampler_var DescriptorSet 0
OpDecorate %sampler_var Binding 1
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
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, true, true,
                                               false, false, false);
}

TEST_F(InstBindlessTest, SPV14AddToEntryPoints) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment {{%\w+}} "foo" {{%\w+}} {{%\w+}} {{%\w+}} [[v1:%\w+]] [[v2:%\w+]]
; CHECK: OpEntryPoint Fragment {{%\w+}} "bar" {{%\w+}} {{%\w+}} {{%\w+}} [[v1:%\w+]] [[v2:%\w+]]
; CHECK: OpDecorate [[v1]] DescriptorSet 7
; CHECK: OpDecorate [[v2]] DescriptorSet 7
; CHECK: [[v1]] = OpVariable {{%\w+}} StorageBuffer
; CHECK: [[v2]] = OpVariable {{%\w+}} StorageBuffer
OpCapability Shader
OpExtension "SPV_EXT_descriptor_indexing"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %foo "foo" %gid %image_var %sampler_var
OpEntryPoint Fragment %foo "bar" %gid %image_var %sampler_var
OpExecutionMode %foo OriginUpperLeft
OpDecorate %image_var DescriptorSet 0
OpDecorate %image_var Binding 0
OpDecorate %sampler_var DescriptorSet 0
OpDecorate %sampler_var Binding 1
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
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, true, true,
                                               false, false, false);
}

TEST_F(InstBindlessTest, InstBoundsAndInitLoadUnsizedUBOArray) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location=0) in nonuniformEXT flat int nu_ii;
  // layout(location=0) out float b;
  //
  // layout(binding=3)  uniform uname { float a; }  uniformBuffer[];
  //
  // void main()
  // {
  //     b = uniformBuffer[nu_ii].a;
  // }

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability UniformBufferArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii
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
OpDecorate %uniformBuffer DescriptorSet 0
OpDecorate %uniformBuffer Binding 3
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %16 NonUniform
OpDecorate %20 NonUniform
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability UniformBufferArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii %gl_FragCoord
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
OpDecorate %uniformBuffer DescriptorSet 0
OpDecorate %uniformBuffer Binding 3
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %7 NonUniform
OpDecorate %102 NonUniform
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_31 Block
OpMemberDecorate %_struct_31 0 Offset 0
OpDecorate %33 DescriptorSet 7
OpDecorate %33 Binding 1
OpDecorate %130 NonUniform
OpDecorate %_struct_55 Block
OpMemberDecorate %_struct_55 0 Offset 0
OpMemberDecorate %_struct_55 1 Offset 4
OpDecorate %57 DescriptorSet 7
OpDecorate %57 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
OpDecorate %127 NonUniform
%void = OpTypeVoid
%10 = OpTypeFunction %void
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
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_3 = OpConstant %uint 3
%26 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_31 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_31 = OpTypePointer StorageBuffer %_struct_31
%33 = OpVariable %_ptr_StorageBuffer__struct_31 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%49 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_55 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_55 = OpTypePointer StorageBuffer %_struct_55
%57 = OpVariable %_ptr_StorageBuffer__struct_55 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_45 = OpConstant %uint 45
%101 = OpConstantNull %float
%105 = OpTypeFunction %uint %uint %uint %uint %uint
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%16 = OpLoad %int %nu_ii
%19 = OpAccessChain %_ptr_Uniform_float %uniformBuffer %16 %int_0
%20 = OpLoad %float %19
OpStore %b %20
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %10
%19 = OpLabel
%7 = OpLoad %int %nu_ii
%20 = OpAccessChain %_ptr_Uniform_float %uniformBuffer %7 %int_0
%40 = OpFunctionCall %uint %25 %uint_1 %uint_3
%42 = OpULessThan %bool %7 %40
OpSelectionMerge %43 None
OpBranchConditional %42 %44 %45
%44 = OpLabel
%103 = OpBitcast %uint %7
%122 = OpFunctionCall %uint %104 %uint_0 %uint_0 %uint_3 %103
%123 = OpULessThan %bool %uint_0 %122
OpSelectionMerge %124 None
OpBranchConditional %123 %125 %126
%125 = OpLabel
%127 = OpLoad %float %20
OpBranch %124
%126 = OpLabel
%128 = OpBitcast %uint %7
%129 = OpFunctionCall %void %48 %uint_45 %uint_1 %128 %uint_0
OpBranch %124
%124 = OpLabel
%130 = OpPhi %float %127 %125 %101 %126
OpBranch %43
%45 = OpLabel
%47 = OpBitcast %uint %7
%100 = OpFunctionCall %void %48 %uint_45 %uint_0 %47 %40
OpBranch %43
%43 = OpLabel
%102 = OpPhi %float %130 %124 %101 %45
OpStore %b %102
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%25 = OpFunction %uint None %26
%27 = OpFunctionParameter %uint
%28 = OpFunctionParameter %uint
%29 = OpLabel
%35 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %27
%36 = OpLoad %uint %35
%37 = OpIAdd %uint %36 %28
%38 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %37
%39 = OpLoad %uint %38
OpReturnValue %39
OpFunctionEnd
%48 = OpFunction %void None %49
%50 = OpFunctionParameter %uint
%51 = OpFunctionParameter %uint
%52 = OpFunctionParameter %uint
%53 = OpFunctionParameter %uint
%54 = OpLabel
%58 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_0
%61 = OpAtomicIAdd %uint %58 %uint_4 %uint_0 %uint_10
%62 = OpIAdd %uint %61 %uint_10
%63 = OpArrayLength %uint %57 1
%64 = OpULessThanEqual %bool %62 %63
OpSelectionMerge %65 None
OpBranchConditional %64 %66 %65
%66 = OpLabel
%67 = OpIAdd %uint %61 %uint_0
%68 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %67
OpStore %68 %uint_10
%70 = OpIAdd %uint %61 %uint_1
%71 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %70
OpStore %71 %uint_23
%73 = OpIAdd %uint %61 %uint_2
%74 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %73
OpStore %74 %50
%75 = OpIAdd %uint %61 %uint_3
%76 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %75
OpStore %76 %uint_4
%80 = OpLoad %v4float %gl_FragCoord
%82 = OpBitcast %v4uint %80
%83 = OpCompositeExtract %uint %82 0
%84 = OpIAdd %uint %61 %uint_4
%85 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %84
OpStore %85 %83
%86 = OpCompositeExtract %uint %82 1
%88 = OpIAdd %uint %61 %uint_5
%89 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %88
OpStore %89 %86
%91 = OpIAdd %uint %61 %uint_7
%92 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %91
OpStore %92 %51
%94 = OpIAdd %uint %61 %uint_8
%95 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %94
OpStore %95 %52
%97 = OpIAdd %uint %61 %uint_9
%98 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %97
OpStore %98 %53
OpBranch %65
%65 = OpLabel
OpReturn
OpFunctionEnd
%104 = OpFunction %uint None %105
%106 = OpFunctionParameter %uint
%107 = OpFunctionParameter %uint
%108 = OpFunctionParameter %uint
%109 = OpFunctionParameter %uint
%110 = OpLabel
%111 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %106
%112 = OpLoad %uint %111
%113 = OpIAdd %uint %112 %107
%114 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %113
%115 = OpLoad %uint %114
%116 = OpIAdd %uint %115 %108
%117 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %116
%118 = OpLoad %uint %117
%119 = OpIAdd %uint %118 %109
%120 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %119
%121 = OpLoad %uint %120
OpReturnValue %121
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest, InstBoundsAndInitLoadUnsizedSSBOArrayDeprecated) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location=0) in nonuniformEXT flat int nu_ii;
  // layout(location=0) out float b;
  //
  // layout(binding=3)  buffer bname { float b; }  storageBuffer[];
  //
  // void main()
  // {
  //     b = storageBuffer[nu_ii].b;
  // }

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability StorageBufferArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability StorageBufferArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii %gl_FragCoord
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
OpDecorate %7 NonUniform
OpDecorate %102 NonUniform
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_31 Block
OpMemberDecorate %_struct_31 0 Offset 0
OpDecorate %33 DescriptorSet 7
OpDecorate %33 Binding 1
OpDecorate %130 NonUniform
OpDecorate %_struct_55 Block
OpMemberDecorate %_struct_55 0 Offset 0
OpMemberDecorate %_struct_55 1 Offset 4
OpDecorate %57 DescriptorSet 7
OpDecorate %57 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
OpDecorate %127 NonUniform
%void = OpTypeVoid
%10 = OpTypeFunction %void
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
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_3 = OpConstant %uint 3
%26 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_31 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_31 = OpTypePointer StorageBuffer %_struct_31
%33 = OpVariable %_ptr_StorageBuffer__struct_31 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%49 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_55 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_55 = OpTypePointer StorageBuffer %_struct_55
%57 = OpVariable %_ptr_StorageBuffer__struct_55 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_45 = OpConstant %uint 45
%101 = OpConstantNull %float
%105 = OpTypeFunction %uint %uint %uint %uint %uint
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%16 = OpLoad %int %nu_ii
%19 = OpAccessChain %_ptr_StorageBuffer_float %storageBuffer %16 %int_0
%20 = OpLoad %float %19
OpStore %b %20
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %10
%19 = OpLabel
%7 = OpLoad %int %nu_ii
%20 = OpAccessChain %_ptr_StorageBuffer_float %storageBuffer %7 %int_0
%40 = OpFunctionCall %uint %25 %uint_1 %uint_3
%42 = OpULessThan %bool %7 %40
OpSelectionMerge %43 None
OpBranchConditional %42 %44 %45
%44 = OpLabel
%103 = OpBitcast %uint %7
%122 = OpFunctionCall %uint %104 %uint_0 %uint_0 %uint_3 %103
%123 = OpULessThan %bool %uint_0 %122
OpSelectionMerge %124 None
OpBranchConditional %123 %125 %126
%125 = OpLabel
%127 = OpLoad %float %20
OpBranch %124
%126 = OpLabel
%128 = OpBitcast %uint %7
%129 = OpFunctionCall %void %48 %uint_45 %uint_1 %128 %uint_0
OpBranch %124
%124 = OpLabel
%130 = OpPhi %float %127 %125 %101 %126
OpBranch %43
%45 = OpLabel
%47 = OpBitcast %uint %7
%100 = OpFunctionCall %void %48 %uint_45 %uint_0 %47 %40
OpBranch %43
%43 = OpLabel
%102 = OpPhi %float %130 %124 %101 %45
OpStore %b %102
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%25 = OpFunction %uint None %26
%27 = OpFunctionParameter %uint
%28 = OpFunctionParameter %uint
%29 = OpLabel
%35 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %27
%36 = OpLoad %uint %35
%37 = OpIAdd %uint %36 %28
%38 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %37
%39 = OpLoad %uint %38
OpReturnValue %39
OpFunctionEnd
%48 = OpFunction %void None %49
%50 = OpFunctionParameter %uint
%51 = OpFunctionParameter %uint
%52 = OpFunctionParameter %uint
%53 = OpFunctionParameter %uint
%54 = OpLabel
%58 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_0
%61 = OpAtomicIAdd %uint %58 %uint_4 %uint_0 %uint_10
%62 = OpIAdd %uint %61 %uint_10
%63 = OpArrayLength %uint %57 1
%64 = OpULessThanEqual %bool %62 %63
OpSelectionMerge %65 None
OpBranchConditional %64 %66 %65
%66 = OpLabel
%67 = OpIAdd %uint %61 %uint_0
%68 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %67
OpStore %68 %uint_10
%70 = OpIAdd %uint %61 %uint_1
%71 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %70
OpStore %71 %uint_23
%73 = OpIAdd %uint %61 %uint_2
%74 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %73
OpStore %74 %50
%75 = OpIAdd %uint %61 %uint_3
%76 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %75
OpStore %76 %uint_4
%80 = OpLoad %v4float %gl_FragCoord
%82 = OpBitcast %v4uint %80
%83 = OpCompositeExtract %uint %82 0
%84 = OpIAdd %uint %61 %uint_4
%85 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %84
OpStore %85 %83
%86 = OpCompositeExtract %uint %82 1
%88 = OpIAdd %uint %61 %uint_5
%89 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %88
OpStore %89 %86
%91 = OpIAdd %uint %61 %uint_7
%92 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %91
OpStore %92 %51
%94 = OpIAdd %uint %61 %uint_8
%95 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %94
OpStore %95 %52
%97 = OpIAdd %uint %61 %uint_9
%98 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %97
OpStore %98 %53
OpBranch %65
%65 = OpLabel
OpReturn
OpFunctionEnd
%104 = OpFunction %uint None %105
%106 = OpFunctionParameter %uint
%107 = OpFunctionParameter %uint
%108 = OpFunctionParameter %uint
%109 = OpFunctionParameter %uint
%110 = OpLabel
%111 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %106
%112 = OpLoad %uint %111
%113 = OpIAdd %uint %112 %107
%114 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %113
%115 = OpLoad %uint %114
%116 = OpIAdd %uint %115 %108
%117 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %116
%118 = OpLoad %uint %117
%119 = OpIAdd %uint %118 %109
%120 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %119
%121 = OpLoad %uint %120
OpReturnValue %121
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest, InstBoundsAndInitLoadUnsizedSSBOArray) {
  // Same as Deprecated but declaring as StorageBuffer Block

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability StorageBufferArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability StorageBufferArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii %gl_FragCoord
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
OpDecorate %7 NonUniform
OpDecorate %102 NonUniform
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_31 Block
OpMemberDecorate %_struct_31 0 Offset 0
OpDecorate %33 DescriptorSet 7
OpDecorate %33 Binding 1
OpDecorate %130 NonUniform
OpDecorate %_struct_55 Block
OpMemberDecorate %_struct_55 0 Offset 0
OpMemberDecorate %_struct_55 1 Offset 4
OpDecorate %57 DescriptorSet 7
OpDecorate %57 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
OpDecorate %127 NonUniform
%void = OpTypeVoid
%10 = OpTypeFunction %void
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
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_3 = OpConstant %uint 3
%26 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_31 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_31 = OpTypePointer StorageBuffer %_struct_31
%33 = OpVariable %_ptr_StorageBuffer__struct_31 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%49 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_55 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_55 = OpTypePointer StorageBuffer %_struct_55
%57 = OpVariable %_ptr_StorageBuffer__struct_55 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_45 = OpConstant %uint 45
%101 = OpConstantNull %float
%105 = OpTypeFunction %uint %uint %uint %uint %uint
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%16 = OpLoad %int %nu_ii
%19 = OpAccessChain %_ptr_StorageBuffer_float %storageBuffer %16 %int_0
%20 = OpLoad %float %19
OpStore %b %20
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %10
%19 = OpLabel
%7 = OpLoad %int %nu_ii
%20 = OpAccessChain %_ptr_StorageBuffer_float %storageBuffer %7 %int_0
%40 = OpFunctionCall %uint %25 %uint_1 %uint_3
%42 = OpULessThan %bool %7 %40
OpSelectionMerge %43 None
OpBranchConditional %42 %44 %45
%44 = OpLabel
%103 = OpBitcast %uint %7
%122 = OpFunctionCall %uint %104 %uint_0 %uint_0 %uint_3 %103
%123 = OpULessThan %bool %uint_0 %122
OpSelectionMerge %124 None
OpBranchConditional %123 %125 %126
%125 = OpLabel
%127 = OpLoad %float %20
OpBranch %124
%126 = OpLabel
%128 = OpBitcast %uint %7
%129 = OpFunctionCall %void %48 %uint_45 %uint_1 %128 %uint_0
OpBranch %124
%124 = OpLabel
%130 = OpPhi %float %127 %125 %101 %126
OpBranch %43
%45 = OpLabel
%47 = OpBitcast %uint %7
%100 = OpFunctionCall %void %48 %uint_45 %uint_0 %47 %40
OpBranch %43
%43 = OpLabel
%102 = OpPhi %float %130 %124 %101 %45
OpStore %b %102
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%25 = OpFunction %uint None %26
%27 = OpFunctionParameter %uint
%28 = OpFunctionParameter %uint
%29 = OpLabel
%35 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %27
%36 = OpLoad %uint %35
%37 = OpIAdd %uint %36 %28
%38 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %37
%39 = OpLoad %uint %38
OpReturnValue %39
OpFunctionEnd
%48 = OpFunction %void None %49
%50 = OpFunctionParameter %uint
%51 = OpFunctionParameter %uint
%52 = OpFunctionParameter %uint
%53 = OpFunctionParameter %uint
%54 = OpLabel
%58 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_0
%61 = OpAtomicIAdd %uint %58 %uint_4 %uint_0 %uint_10
%62 = OpIAdd %uint %61 %uint_10
%63 = OpArrayLength %uint %57 1
%64 = OpULessThanEqual %bool %62 %63
OpSelectionMerge %65 None
OpBranchConditional %64 %66 %65
%66 = OpLabel
%67 = OpIAdd %uint %61 %uint_0
%68 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %67
OpStore %68 %uint_10
%70 = OpIAdd %uint %61 %uint_1
%71 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %70
OpStore %71 %uint_23
%73 = OpIAdd %uint %61 %uint_2
%74 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %73
OpStore %74 %50
%75 = OpIAdd %uint %61 %uint_3
%76 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %75
OpStore %76 %uint_4
%80 = OpLoad %v4float %gl_FragCoord
%82 = OpBitcast %v4uint %80
%83 = OpCompositeExtract %uint %82 0
%84 = OpIAdd %uint %61 %uint_4
%85 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %84
OpStore %85 %83
%86 = OpCompositeExtract %uint %82 1
%88 = OpIAdd %uint %61 %uint_5
%89 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %88
OpStore %89 %86
%91 = OpIAdd %uint %61 %uint_7
%92 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %91
OpStore %92 %51
%94 = OpIAdd %uint %61 %uint_8
%95 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %94
OpStore %95 %52
%97 = OpIAdd %uint %61 %uint_9
%98 = OpAccessChain %_ptr_StorageBuffer_uint %57 %uint_1 %97
OpStore %98 %53
OpBranch %65
%65 = OpLabel
OpReturn
OpFunctionEnd
%104 = OpFunction %uint None %105
%106 = OpFunctionParameter %uint
%107 = OpFunctionParameter %uint
%108 = OpFunctionParameter %uint
%109 = OpFunctionParameter %uint
%110 = OpLabel
%111 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %106
%112 = OpLoad %uint %111
%113 = OpIAdd %uint %112 %107
%114 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %113
%115 = OpLoad %uint %114
%116 = OpIAdd %uint %115 %108
%117 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %116
%118 = OpLoad %uint %117
%119 = OpIAdd %uint %118 %109
%120 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %119
%121 = OpLoad %uint %120
OpReturnValue %121
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest, InstInitLoadUBOScalar) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location=0) out float b;
  // layout(binding=3)  uniform uname { float a; }  uniformBuffer;
  //
  // void main()
  // {
  //     b = uniformBuffer.a;
  // }

  const std::string defs_before =
      R"(OpCapability Shader
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b
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
OpDecorate %uniformBuffer DescriptorSet 0
OpDecorate %uniformBuffer Binding 3
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %gl_FragCoord
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
OpDecorate %uniformBuffer DescriptorSet 0
OpDecorate %uniformBuffer Binding 3
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_28 Block
OpMemberDecorate %_struct_28 0 Offset 0
OpDecorate %30 DescriptorSet 7
OpDecorate %30 Binding 1
OpDecorate %_struct_58 Block
OpMemberDecorate %_struct_58 0 Offset 0
OpMemberDecorate %_struct_58 1 Offset 4
OpDecorate %60 DescriptorSet 7
OpDecorate %60 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
%b = OpVariable %_ptr_Output_float Output
%uname = OpTypeStruct %float
%_ptr_Uniform_uname = OpTypePointer Uniform %uname
%uniformBuffer = OpVariable %_ptr_Uniform_uname Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_3 = OpConstant %uint 3
%21 = OpTypeFunction %uint %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_28 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_28 = OpTypePointer StorageBuffer %_struct_28
%30 = OpVariable %_ptr_StorageBuffer__struct_28 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%uint_1 = OpConstant %uint 1
%52 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_58 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_58 = OpTypePointer StorageBuffer %_struct_58
%60 = OpVariable %_ptr_StorageBuffer__struct_58 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_32 = OpConstant %uint 32
%104 = OpConstantNull %float
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%15 = OpAccessChain %_ptr_Uniform_float %uniformBuffer %int_0
%16 = OpLoad %float %15
OpStore %b %16
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %7
%14 = OpLabel
%15 = OpAccessChain %_ptr_Uniform_float %uniformBuffer %int_0
%43 = OpFunctionCall %uint %20 %uint_0 %uint_0 %uint_3 %uint_0
%45 = OpULessThan %bool %uint_0 %43
OpSelectionMerge %47 None
OpBranchConditional %45 %48 %49
%48 = OpLabel
%50 = OpLoad %float %15
OpBranch %47
%49 = OpLabel
%103 = OpFunctionCall %void %51 %uint_32 %uint_1 %uint_0 %uint_0
OpBranch %47
%47 = OpLabel
%105 = OpPhi %float %50 %48 %104 %49
OpStore %b %105
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%20 = OpFunction %uint None %21
%22 = OpFunctionParameter %uint
%23 = OpFunctionParameter %uint
%24 = OpFunctionParameter %uint
%25 = OpFunctionParameter %uint
%26 = OpLabel
%32 = OpAccessChain %_ptr_StorageBuffer_uint %30 %uint_0 %22
%33 = OpLoad %uint %32
%34 = OpIAdd %uint %33 %23
%35 = OpAccessChain %_ptr_StorageBuffer_uint %30 %uint_0 %34
%36 = OpLoad %uint %35
%37 = OpIAdd %uint %36 %24
%38 = OpAccessChain %_ptr_StorageBuffer_uint %30 %uint_0 %37
%39 = OpLoad %uint %38
%40 = OpIAdd %uint %39 %25
%41 = OpAccessChain %_ptr_StorageBuffer_uint %30 %uint_0 %40
%42 = OpLoad %uint %41
OpReturnValue %42
OpFunctionEnd
%51 = OpFunction %void None %52
%53 = OpFunctionParameter %uint
%54 = OpFunctionParameter %uint
%55 = OpFunctionParameter %uint
%56 = OpFunctionParameter %uint
%57 = OpLabel
%61 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_0
%64 = OpAtomicIAdd %uint %61 %uint_4 %uint_0 %uint_10
%65 = OpIAdd %uint %64 %uint_10
%66 = OpArrayLength %uint %60 1
%67 = OpULessThanEqual %bool %65 %66
OpSelectionMerge %68 None
OpBranchConditional %67 %69 %68
%69 = OpLabel
%70 = OpIAdd %uint %64 %uint_0
%71 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %70
OpStore %71 %uint_10
%73 = OpIAdd %uint %64 %uint_1
%74 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %73
OpStore %74 %uint_23
%76 = OpIAdd %uint %64 %uint_2
%77 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %76
OpStore %77 %53
%78 = OpIAdd %uint %64 %uint_3
%79 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %78
OpStore %79 %uint_4
%83 = OpLoad %v4float %gl_FragCoord
%85 = OpBitcast %v4uint %83
%86 = OpCompositeExtract %uint %85 0
%87 = OpIAdd %uint %64 %uint_4
%88 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %87
OpStore %88 %86
%89 = OpCompositeExtract %uint %85 1
%91 = OpIAdd %uint %64 %uint_5
%92 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %91
OpStore %92 %89
%94 = OpIAdd %uint %64 %uint_7
%95 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %94
OpStore %95 %54
%97 = OpIAdd %uint %64 %uint_8
%98 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %97
OpStore %98 %55
%100 = OpIAdd %uint %64 %uint_9
%101 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %100
OpStore %101 %56
OpBranch %68
%68 = OpLabel
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest, InstBoundsInitStoreUnsizedSSBOArray) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location=0) in nonuniformEXT flat int nu_ii;
  // layout(location=1) in float b;
  //
  // layout(binding=4)  buffer bname { float b; }  storageBuffer[];
  //
  // void main()
  // {
  //     storageBuffer[nu_ii].b = b;
  // }

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability StorageBufferArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %nu_ii %b
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
OpDecorate %storageBuffer DescriptorSet 0
OpDecorate %storageBuffer Binding 4
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %14 NonUniform
OpDecorate %b Location 1
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability RuntimeDescriptorArray
OpCapability StorageBufferArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %nu_ii %b %gl_FragCoord
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
OpDecorate %storageBuffer DescriptorSet 0
OpDecorate %storageBuffer Binding 4
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %7 NonUniform
OpDecorate %b Location 1
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_31 Block
OpMemberDecorate %_struct_31 0 Offset 0
OpDecorate %33 DescriptorSet 7
OpDecorate %33 Binding 1
OpDecorate %_struct_54 Block
OpMemberDecorate %_struct_54 0 Offset 0
OpMemberDecorate %_struct_54 1 Offset 4
OpDecorate %56 DescriptorSet 7
OpDecorate %56 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
%void = OpTypeVoid
%9 = OpTypeFunction %void
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
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_4 = OpConstant %uint 4
%26 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_31 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_31 = OpTypePointer StorageBuffer %_struct_31
%33 = OpVariable %_ptr_StorageBuffer__struct_31 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%48 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_54 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_54 = OpTypePointer StorageBuffer %_struct_54
%56 = OpVariable %_ptr_StorageBuffer__struct_54 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_45 = OpConstant %uint 45
%102 = OpTypeFunction %uint %uint %uint %uint %uint
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%14 = OpLoad %int %nu_ii
%18 = OpLoad %float %b
%20 = OpAccessChain %_ptr_Uniform_float %storageBuffer %14 %int_0
OpStore %20 %18
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %9
%18 = OpLabel
%7 = OpLoad %int %nu_ii
%19 = OpLoad %float %b
%20 = OpAccessChain %_ptr_Uniform_float %storageBuffer %7 %int_0
%40 = OpFunctionCall %uint %25 %uint_1 %uint_4
%42 = OpULessThan %bool %7 %40
OpSelectionMerge %43 None
OpBranchConditional %42 %44 %45
%44 = OpLabel
%100 = OpBitcast %uint %7
%119 = OpFunctionCall %uint %101 %uint_0 %uint_0 %uint_4 %100
%120 = OpULessThan %bool %uint_0 %119
OpSelectionMerge %121 None
OpBranchConditional %120 %122 %123
%122 = OpLabel
OpStore %20 %19
OpBranch %121
%123 = OpLabel
%124 = OpBitcast %uint %7
%125 = OpFunctionCall %void %47 %uint_45 %uint_1 %124 %uint_0
OpBranch %121
%121 = OpLabel
OpBranch %43
%45 = OpLabel
%46 = OpBitcast %uint %7
%99 = OpFunctionCall %void %47 %uint_45 %uint_0 %46 %40
OpBranch %43
%43 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%25 = OpFunction %uint None %26
%27 = OpFunctionParameter %uint
%28 = OpFunctionParameter %uint
%29 = OpLabel
%35 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %27
%36 = OpLoad %uint %35
%37 = OpIAdd %uint %36 %28
%38 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %37
%39 = OpLoad %uint %38
OpReturnValue %39
OpFunctionEnd
%47 = OpFunction %void None %48
%49 = OpFunctionParameter %uint
%50 = OpFunctionParameter %uint
%51 = OpFunctionParameter %uint
%52 = OpFunctionParameter %uint
%53 = OpLabel
%57 = OpAccessChain %_ptr_StorageBuffer_uint %56 %uint_0
%59 = OpAtomicIAdd %uint %57 %uint_4 %uint_0 %uint_10
%60 = OpIAdd %uint %59 %uint_10
%61 = OpArrayLength %uint %56 1
%62 = OpULessThanEqual %bool %60 %61
OpSelectionMerge %63 None
OpBranchConditional %62 %64 %63
%64 = OpLabel
%65 = OpIAdd %uint %59 %uint_0
%66 = OpAccessChain %_ptr_StorageBuffer_uint %56 %uint_1 %65
OpStore %66 %uint_10
%68 = OpIAdd %uint %59 %uint_1
%69 = OpAccessChain %_ptr_StorageBuffer_uint %56 %uint_1 %68
OpStore %69 %uint_23
%71 = OpIAdd %uint %59 %uint_2
%72 = OpAccessChain %_ptr_StorageBuffer_uint %56 %uint_1 %71
OpStore %72 %49
%74 = OpIAdd %uint %59 %uint_3
%75 = OpAccessChain %_ptr_StorageBuffer_uint %56 %uint_1 %74
OpStore %75 %uint_4
%79 = OpLoad %v4float %gl_FragCoord
%81 = OpBitcast %v4uint %79
%82 = OpCompositeExtract %uint %81 0
%83 = OpIAdd %uint %59 %uint_4
%84 = OpAccessChain %_ptr_StorageBuffer_uint %56 %uint_1 %83
OpStore %84 %82
%85 = OpCompositeExtract %uint %81 1
%87 = OpIAdd %uint %59 %uint_5
%88 = OpAccessChain %_ptr_StorageBuffer_uint %56 %uint_1 %87
OpStore %88 %85
%90 = OpIAdd %uint %59 %uint_7
%91 = OpAccessChain %_ptr_StorageBuffer_uint %56 %uint_1 %90
OpStore %91 %50
%93 = OpIAdd %uint %59 %uint_8
%94 = OpAccessChain %_ptr_StorageBuffer_uint %56 %uint_1 %93
OpStore %94 %51
%96 = OpIAdd %uint %59 %uint_9
%97 = OpAccessChain %_ptr_StorageBuffer_uint %56 %uint_1 %96
OpStore %97 %52
OpBranch %63
%63 = OpLabel
OpReturn
OpFunctionEnd
%101 = OpFunction %uint None %102
%103 = OpFunctionParameter %uint
%104 = OpFunctionParameter %uint
%105 = OpFunctionParameter %uint
%106 = OpFunctionParameter %uint
%107 = OpLabel
%108 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %103
%109 = OpLoad %uint %108
%110 = OpIAdd %uint %109 %104
%111 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %110
%112 = OpLoad %uint %111
%113 = OpIAdd %uint %112 %105
%114 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %113
%115 = OpLoad %uint %114
%116 = OpIAdd %uint %115 %106
%117 = OpAccessChain %_ptr_StorageBuffer_uint %33 %uint_0 %116
%118 = OpLoad %uint %117
OpReturnValue %118
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest, InstBoundsInitLoadSizedUBOArray) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout(location=0) in nonuniformEXT flat int nu_ii;
  // layout(location=0) out float b;
  //
  // layout(binding=3)  uniform uname { float a; }  uniformBuffer[128];
  //
  // void main()
  // {
  //     b = uniformBuffer[nu_ii].a;
  // }

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability UniformBufferArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii
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
OpDecorate %uniformBuffer DescriptorSet 0
OpDecorate %uniformBuffer Binding 3
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %18 NonUniform
OpDecorate %22 NonUniform
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability UniformBufferArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %b %nu_ii %gl_FragCoord
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
OpDecorate %uniformBuffer DescriptorSet 0
OpDecorate %uniformBuffer Binding 3
OpDecorate %nu_ii Flat
OpDecorate %nu_ii Location 0
OpDecorate %nu_ii NonUniform
OpDecorate %7 NonUniform
OpDecorate %89 NonUniform
OpDecorate %120 NonUniform
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_39 Block
OpMemberDecorate %_struct_39 0 Offset 0
OpMemberDecorate %_struct_39 1 Offset 4
OpDecorate %41 DescriptorSet 7
OpDecorate %41 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
OpDecorate %_struct_98 Block
OpMemberDecorate %_struct_98 0 Offset 0
OpDecorate %100 DescriptorSet 7
OpDecorate %100 Binding 1
OpDecorate %117 NonUniform
%void = OpTypeVoid
%10 = OpTypeFunction %void
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
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%32 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_39 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_39 = OpTypePointer StorageBuffer %_struct_39
%41 = OpVariable %_ptr_StorageBuffer__struct_39 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_1 = OpConstant %uint 1
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_46 = OpConstant %uint 46
%88 = OpConstantNull %float
%92 = OpTypeFunction %uint %uint %uint %uint %uint
%_struct_98 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_98 = OpTypePointer StorageBuffer %_struct_98
%100 = OpVariable %_ptr_StorageBuffer__struct_98 StorageBuffer
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%18 = OpLoad %int %nu_ii
%21 = OpAccessChain %_ptr_Uniform_float %uniformBuffer %18 %int_0
%22 = OpLoad %float %21
OpStore %b %22
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %10
%21 = OpLabel
%7 = OpLoad %int %nu_ii
%22 = OpAccessChain %_ptr_Uniform_float %uniformBuffer %7 %int_0
%25 = OpULessThan %bool %7 %uint_128
OpSelectionMerge %26 None
OpBranchConditional %25 %27 %28
%27 = OpLabel
%90 = OpBitcast %uint %7
%112 = OpFunctionCall %uint %91 %uint_0 %uint_0 %uint_3 %90
%113 = OpULessThan %bool %uint_0 %112
OpSelectionMerge %114 None
OpBranchConditional %113 %115 %116
%115 = OpLabel
%117 = OpLoad %float %22
OpBranch %114
%116 = OpLabel
%118 = OpBitcast %uint %7
%119 = OpFunctionCall %void %31 %uint_46 %uint_1 %118 %uint_0
OpBranch %114
%114 = OpLabel
%120 = OpPhi %float %117 %115 %88 %116
OpBranch %26
%28 = OpLabel
%30 = OpBitcast %uint %7
%87 = OpFunctionCall %void %31 %uint_46 %uint_0 %30 %uint_128
OpBranch %26
%26 = OpLabel
%89 = OpPhi %float %120 %114 %88 %28
OpStore %b %89
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%31 = OpFunction %void None %32
%33 = OpFunctionParameter %uint
%34 = OpFunctionParameter %uint
%35 = OpFunctionParameter %uint
%36 = OpFunctionParameter %uint
%37 = OpLabel
%43 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0
%46 = OpAtomicIAdd %uint %43 %uint_4 %uint_0 %uint_10
%47 = OpIAdd %uint %46 %uint_10
%48 = OpArrayLength %uint %41 1
%49 = OpULessThanEqual %bool %47 %48
OpSelectionMerge %50 None
OpBranchConditional %49 %51 %50
%51 = OpLabel
%52 = OpIAdd %uint %46 %uint_0
%54 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_1 %52
OpStore %54 %uint_10
%56 = OpIAdd %uint %46 %uint_1
%57 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_1 %56
OpStore %57 %uint_23
%59 = OpIAdd %uint %46 %uint_2
%60 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_1 %59
OpStore %60 %33
%62 = OpIAdd %uint %46 %uint_3
%63 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_1 %62
OpStore %63 %uint_4
%67 = OpLoad %v4float %gl_FragCoord
%69 = OpBitcast %v4uint %67
%70 = OpCompositeExtract %uint %69 0
%71 = OpIAdd %uint %46 %uint_4
%72 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_1 %71
OpStore %72 %70
%73 = OpCompositeExtract %uint %69 1
%75 = OpIAdd %uint %46 %uint_5
%76 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_1 %75
OpStore %76 %73
%78 = OpIAdd %uint %46 %uint_7
%79 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_1 %78
OpStore %79 %34
%81 = OpIAdd %uint %46 %uint_8
%82 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_1 %81
OpStore %82 %35
%84 = OpIAdd %uint %46 %uint_9
%85 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_1 %84
OpStore %85 %36
OpBranch %50
%50 = OpLabel
OpReturn
OpFunctionEnd
%91 = OpFunction %uint None %92
%93 = OpFunctionParameter %uint
%94 = OpFunctionParameter %uint
%95 = OpFunctionParameter %uint
%96 = OpFunctionParameter %uint
%97 = OpLabel
%101 = OpAccessChain %_ptr_StorageBuffer_uint %100 %uint_0 %93
%102 = OpLoad %uint %101
%103 = OpIAdd %uint %102 %94
%104 = OpAccessChain %_ptr_StorageBuffer_uint %100 %uint_0 %103
%105 = OpLoad %uint %104
%106 = OpIAdd %uint %105 %95
%107 = OpAccessChain %_ptr_StorageBuffer_uint %100 %uint_0 %106
%108 = OpLoad %uint %107
%109 = OpIAdd %uint %108 %96
%110 = OpAccessChain %_ptr_StorageBuffer_uint %100 %uint_0 %109
%111 = OpLoad %uint %110
OpReturnValue %111
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest,
       InstBoundsComputeShaderInitLoadVariableSizedSampledImagesArray) {
  // #version 450
  // #extension GL_EXT_nonuniform_qualifier : enable
  //
  // layout (local_size_x = 1, local_size_y = 1) in;
  //
  // layout(set = 0, binding = 0, std140) buffer Input {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 0, binding = 1, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability RuntimeDescriptorArray
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability RuntimeDescriptorArray
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_39 Block
OpMemberDecorate %_struct_39 0 Offset 0
OpDecorate %41 DescriptorSet 7
OpDecorate %41 Binding 1
OpDecorate %_struct_63 Block
OpMemberDecorate %_struct_63 0 Offset 0
OpMemberDecorate %_struct_63 1 Offset 4
OpDecorate %65 DescriptorSet 7
OpDecorate %65 Binding 0
OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
%void = OpTypeVoid
%7 = OpTypeFunction %void
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
%20 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%uint_1 = OpConstant %uint 1
%34 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_39 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_39 = OpTypePointer StorageBuffer %_struct_39
%41 = OpVariable %_ptr_StorageBuffer__struct_39 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%57 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_63 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_63 = OpTypePointer StorageBuffer %_struct_63
%65 = OpVariable %_ptr_StorageBuffer__struct_63 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_5 = OpConstant %uint 5
%uint_3 = OpConstant %uint 3
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_50 = OpConstant %uint 50
%112 = OpConstantNull %v4float
%115 = OpTypeFunction %uint %uint %uint %uint %uint
%uint_47 = OpConstant %uint 47
%140 = OpConstantNull %uint
%uint_53 = OpConstant %uint 53
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%19 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%20 = OpLoad %uint %19
%22 = OpAccessChain %_ptr_UniformConstant_13 %images %20
%23 = OpLoad %13 %22
%27 = OpImageRead %v4float %23 %25
%29 = OpCompositeExtract %float %27 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
OpStore %31 %29
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %7
%24 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%132 = OpFunctionCall %uint %114 %uint_0 %uint_0 %uint_0 %uint_0
%133 = OpULessThan %bool %uint_0 %132
OpSelectionMerge %134 None
OpBranchConditional %133 %135 %136
%135 = OpLabel
%137 = OpLoad %uint %25
OpBranch %134
%136 = OpLabel
%139 = OpFunctionCall %void %56 %uint_47 %uint_1 %uint_0 %uint_0
OpBranch %134
%134 = OpLabel
%141 = OpPhi %uint %137 %135 %140 %136
%27 = OpAccessChain %_ptr_UniformConstant_13 %images %141
%28 = OpLoad %13 %27
%48 = OpFunctionCall %uint %33 %uint_1 %uint_1
%50 = OpULessThan %bool %141 %48
OpSelectionMerge %51 None
OpBranchConditional %50 %52 %53
%52 = OpLabel
%54 = OpLoad %13 %27
%142 = OpFunctionCall %uint %114 %uint_0 %uint_0 %uint_1 %141
%143 = OpULessThan %bool %uint_0 %142
OpSelectionMerge %144 None
OpBranchConditional %143 %145 %146
%145 = OpLabel
%147 = OpLoad %13 %27
%148 = OpImageRead %v4float %147 %20
OpBranch %144
%146 = OpLabel
%149 = OpFunctionCall %void %56 %uint_50 %uint_1 %141 %uint_0
OpBranch %144
%144 = OpLabel
%150 = OpPhi %v4float %148 %145 %112 %146
OpBranch %51
%53 = OpLabel
%111 = OpFunctionCall %void %56 %uint_50 %uint_0 %141 %48
OpBranch %51
%51 = OpLabel
%113 = OpPhi %v4float %150 %144 %112 %53
%30 = OpCompositeExtract %float %113 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
%151 = OpFunctionCall %uint %114 %uint_0 %uint_0 %uint_0 %uint_0
%152 = OpULessThan %bool %uint_0 %151
OpSelectionMerge %153 None
OpBranchConditional %152 %154 %155
%154 = OpLabel
OpStore %31 %30
OpBranch %153
%155 = OpLabel
%157 = OpFunctionCall %void %56 %uint_53 %uint_1 %uint_0 %uint_0
OpBranch %153
%153 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%33 = OpFunction %uint None %34
%35 = OpFunctionParameter %uint
%36 = OpFunctionParameter %uint
%37 = OpLabel
%43 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %35
%44 = OpLoad %uint %43
%45 = OpIAdd %uint %44 %36
%46 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %45
%47 = OpLoad %uint %46
OpReturnValue %47
OpFunctionEnd
%56 = OpFunction %void None %57
%58 = OpFunctionParameter %uint
%59 = OpFunctionParameter %uint
%60 = OpFunctionParameter %uint
%61 = OpFunctionParameter %uint
%62 = OpLabel
%66 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_0
%69 = OpAtomicIAdd %uint %66 %uint_4 %uint_0 %uint_10
%70 = OpIAdd %uint %69 %uint_10
%71 = OpArrayLength %uint %65 1
%72 = OpULessThanEqual %bool %70 %71
OpSelectionMerge %73 None
OpBranchConditional %72 %74 %73
%74 = OpLabel
%75 = OpIAdd %uint %69 %uint_0
%76 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %75
OpStore %76 %uint_10
%78 = OpIAdd %uint %69 %uint_1
%79 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %78
OpStore %79 %uint_23
%81 = OpIAdd %uint %69 %uint_2
%82 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %81
OpStore %82 %58
%85 = OpIAdd %uint %69 %uint_3
%86 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %85
OpStore %86 %uint_5
%90 = OpLoad %v3uint %gl_GlobalInvocationID
%91 = OpCompositeExtract %uint %90 0
%92 = OpCompositeExtract %uint %90 1
%93 = OpCompositeExtract %uint %90 2
%94 = OpIAdd %uint %69 %uint_4
%95 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %94
OpStore %95 %91
%96 = OpIAdd %uint %69 %uint_5
%97 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %96
OpStore %97 %92
%99 = OpIAdd %uint %69 %uint_6
%100 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %99
OpStore %100 %93
%102 = OpIAdd %uint %69 %uint_7
%103 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %102
OpStore %103 %59
%105 = OpIAdd %uint %69 %uint_8
%106 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %105
OpStore %106 %60
%108 = OpIAdd %uint %69 %uint_9
%109 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %108
OpStore %109 %61
OpBranch %73
%73 = OpLabel
OpReturn
OpFunctionEnd
%114 = OpFunction %uint None %115
%116 = OpFunctionParameter %uint
%117 = OpFunctionParameter %uint
%118 = OpFunctionParameter %uint
%119 = OpFunctionParameter %uint
%120 = OpLabel
%121 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %116
%122 = OpLoad %uint %121
%123 = OpIAdd %uint %122 %117
%124 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %123
%125 = OpLoad %uint %124
%126 = OpIAdd %uint %125 %118
%127 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %126
%128 = OpLoad %uint %127
%129 = OpIAdd %uint %128 %119
%130 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %129
%131 = OpLoad %uint %130
OpReturnValue %131
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest,
       InstBoundsRayGenerationInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 0, binding = 0, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 0, binding = 1, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  const std::string defs_before =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint RayGenerationNV %main "main"
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
%void = OpTypeVoid
)";

  const std::string defs_after =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint RayGenerationNV %main "main" %89
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_39 Block
OpMemberDecorate %_struct_39 0 Offset 0
OpDecorate %41 DescriptorSet 7
OpDecorate %41 Binding 1
OpDecorate %_struct_63 Block
OpMemberDecorate %_struct_63 0 Offset 0
OpMemberDecorate %_struct_63 1 Offset 4
OpDecorate %65 DescriptorSet 7
OpDecorate %65 Binding 0
OpDecorate %89 BuiltIn LaunchIdNV
%void = OpTypeVoid
)";

  const std::string func_before =
      R"(%3 = OpTypeFunction %void
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
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%7 = OpTypeFunction %void
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
%20 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%uint_1 = OpConstant %uint 1
%34 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_39 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_39 = OpTypePointer StorageBuffer %_struct_39
%41 = OpVariable %_ptr_StorageBuffer__struct_39 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%57 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_63 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_63 = OpTypePointer StorageBuffer %_struct_63
%65 = OpVariable %_ptr_StorageBuffer__struct_63 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_5313 = OpConstant %uint 5313
%uint_3 = OpConstant %uint 3
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%89 = OpVariable %_ptr_Input_v3uint Input
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_51 = OpConstant %uint 51
%113 = OpConstantNull %v4float
%116 = OpTypeFunction %uint %uint %uint %uint %uint
%uint_48 = OpConstant %uint 48
%141 = OpConstantNull %uint
%uint_54 = OpConstant %uint 54
%main = OpFunction %void None %7
%24 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%133 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%134 = OpULessThan %bool %uint_0 %133
OpSelectionMerge %135 None
OpBranchConditional %134 %136 %137
%136 = OpLabel
%138 = OpLoad %uint %25
OpBranch %135
%137 = OpLabel
%140 = OpFunctionCall %void %56 %uint_48 %uint_1 %uint_0 %uint_0
OpBranch %135
%135 = OpLabel
%142 = OpPhi %uint %138 %136 %141 %137
%27 = OpAccessChain %_ptr_UniformConstant_13 %images %142
%28 = OpLoad %13 %27
%48 = OpFunctionCall %uint %33 %uint_1 %uint_1
%50 = OpULessThan %bool %142 %48
OpSelectionMerge %51 None
OpBranchConditional %50 %52 %53
%52 = OpLabel
%54 = OpLoad %13 %27
%143 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_1 %142
%144 = OpULessThan %bool %uint_0 %143
OpSelectionMerge %145 None
OpBranchConditional %144 %146 %147
%146 = OpLabel
%148 = OpLoad %13 %27
%149 = OpImageRead %v4float %148 %20
OpBranch %145
%147 = OpLabel
%150 = OpFunctionCall %void %56 %uint_51 %uint_1 %142 %uint_0
OpBranch %145
%145 = OpLabel
%151 = OpPhi %v4float %149 %146 %113 %147
OpBranch %51
%53 = OpLabel
%112 = OpFunctionCall %void %56 %uint_51 %uint_0 %142 %48
OpBranch %51
%51 = OpLabel
%114 = OpPhi %v4float %151 %145 %113 %53
%30 = OpCompositeExtract %float %114 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
%152 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%153 = OpULessThan %bool %uint_0 %152
OpSelectionMerge %154 None
OpBranchConditional %153 %155 %156
%155 = OpLabel
OpStore %31 %30
OpBranch %154
%156 = OpLabel
%158 = OpFunctionCall %void %56 %uint_54 %uint_1 %uint_0 %uint_0
OpBranch %154
%154 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%33 = OpFunction %uint None %34
%35 = OpFunctionParameter %uint
%36 = OpFunctionParameter %uint
%37 = OpLabel
%43 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %35
%44 = OpLoad %uint %43
%45 = OpIAdd %uint %44 %36
%46 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %45
%47 = OpLoad %uint %46
OpReturnValue %47
OpFunctionEnd
%56 = OpFunction %void None %57
%58 = OpFunctionParameter %uint
%59 = OpFunctionParameter %uint
%60 = OpFunctionParameter %uint
%61 = OpFunctionParameter %uint
%62 = OpLabel
%66 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_0
%69 = OpAtomicIAdd %uint %66 %uint_4 %uint_0 %uint_10
%70 = OpIAdd %uint %69 %uint_10
%71 = OpArrayLength %uint %65 1
%72 = OpULessThanEqual %bool %70 %71
OpSelectionMerge %73 None
OpBranchConditional %72 %74 %73
%74 = OpLabel
%75 = OpIAdd %uint %69 %uint_0
%76 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %75
OpStore %76 %uint_10
%78 = OpIAdd %uint %69 %uint_1
%79 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %78
OpStore %79 %uint_23
%81 = OpIAdd %uint %69 %uint_2
%82 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %81
OpStore %82 %58
%85 = OpIAdd %uint %69 %uint_3
%86 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %85
OpStore %86 %uint_5313
%90 = OpLoad %v3uint %89
%91 = OpCompositeExtract %uint %90 0
%92 = OpCompositeExtract %uint %90 1
%93 = OpCompositeExtract %uint %90 2
%94 = OpIAdd %uint %69 %uint_4
%95 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %94
OpStore %95 %91
%97 = OpIAdd %uint %69 %uint_5
%98 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %97
OpStore %98 %92
%100 = OpIAdd %uint %69 %uint_6
%101 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %100
OpStore %101 %93
%103 = OpIAdd %uint %69 %uint_7
%104 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %103
OpStore %104 %59
%106 = OpIAdd %uint %69 %uint_8
%107 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %106
OpStore %107 %60
%109 = OpIAdd %uint %69 %uint_9
%110 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %109
OpStore %110 %61
OpBranch %73
%73 = OpLabel
OpReturn
OpFunctionEnd
%115 = OpFunction %uint None %116
%117 = OpFunctionParameter %uint
%118 = OpFunctionParameter %uint
%119 = OpFunctionParameter %uint
%120 = OpFunctionParameter %uint
%121 = OpLabel
%122 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %117
%123 = OpLoad %uint %122
%124 = OpIAdd %uint %123 %118
%125 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %124
%126 = OpLoad %uint %125
%127 = OpIAdd %uint %126 %119
%128 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %127
%129 = OpLoad %uint %128
%130 = OpIAdd %uint %129 %120
%131 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %130
%132 = OpLoad %uint %131
OpReturnValue %132
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest,
       InstBoundsIntersectionInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 0, binding = 0, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 0, binding = 1, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  const std::string defs_before =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint IntersectionNV %main "main"
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
%void = OpTypeVoid
)";

  const std::string defs_after =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint IntersectionNV %main "main" %89
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_39 Block
OpMemberDecorate %_struct_39 0 Offset 0
OpDecorate %41 DescriptorSet 7
OpDecorate %41 Binding 1
OpDecorate %_struct_63 Block
OpMemberDecorate %_struct_63 0 Offset 0
OpMemberDecorate %_struct_63 1 Offset 4
OpDecorate %65 DescriptorSet 7
OpDecorate %65 Binding 0
OpDecorate %89 BuiltIn LaunchIdNV
%void = OpTypeVoid
)";

  const std::string func_before =
      R"(%3 = OpTypeFunction %void
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
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%7 = OpTypeFunction %void
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
%20 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%uint_1 = OpConstant %uint 1
%34 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_39 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_39 = OpTypePointer StorageBuffer %_struct_39
%41 = OpVariable %_ptr_StorageBuffer__struct_39 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%57 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_63 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_63 = OpTypePointer StorageBuffer %_struct_63
%65 = OpVariable %_ptr_StorageBuffer__struct_63 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_5314 = OpConstant %uint 5314
%uint_3 = OpConstant %uint 3
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%89 = OpVariable %_ptr_Input_v3uint Input
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_51 = OpConstant %uint 51
%113 = OpConstantNull %v4float
%116 = OpTypeFunction %uint %uint %uint %uint %uint
%uint_48 = OpConstant %uint 48
%141 = OpConstantNull %uint
%uint_54 = OpConstant %uint 54
%main = OpFunction %void None %7
%24 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%133 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%134 = OpULessThan %bool %uint_0 %133
OpSelectionMerge %135 None
OpBranchConditional %134 %136 %137
%136 = OpLabel
%138 = OpLoad %uint %25
OpBranch %135
%137 = OpLabel
%140 = OpFunctionCall %void %56 %uint_48 %uint_1 %uint_0 %uint_0
OpBranch %135
%135 = OpLabel
%142 = OpPhi %uint %138 %136 %141 %137
%27 = OpAccessChain %_ptr_UniformConstant_13 %images %142
%28 = OpLoad %13 %27
%48 = OpFunctionCall %uint %33 %uint_1 %uint_1
%50 = OpULessThan %bool %142 %48
OpSelectionMerge %51 None
OpBranchConditional %50 %52 %53
%52 = OpLabel
%54 = OpLoad %13 %27
%143 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_1 %142
%144 = OpULessThan %bool %uint_0 %143
OpSelectionMerge %145 None
OpBranchConditional %144 %146 %147
%146 = OpLabel
%148 = OpLoad %13 %27
%149 = OpImageRead %v4float %148 %20
OpBranch %145
%147 = OpLabel
%150 = OpFunctionCall %void %56 %uint_51 %uint_1 %142 %uint_0
OpBranch %145
%145 = OpLabel
%151 = OpPhi %v4float %149 %146 %113 %147
OpBranch %51
%53 = OpLabel
%112 = OpFunctionCall %void %56 %uint_51 %uint_0 %142 %48
OpBranch %51
%51 = OpLabel
%114 = OpPhi %v4float %151 %145 %113 %53
%30 = OpCompositeExtract %float %114 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
%152 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%153 = OpULessThan %bool %uint_0 %152
OpSelectionMerge %154 None
OpBranchConditional %153 %155 %156
%155 = OpLabel
OpStore %31 %30
OpBranch %154
%156 = OpLabel
%158 = OpFunctionCall %void %56 %uint_54 %uint_1 %uint_0 %uint_0
OpBranch %154
%154 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%33 = OpFunction %uint None %34
%35 = OpFunctionParameter %uint
%36 = OpFunctionParameter %uint
%37 = OpLabel
%43 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %35
%44 = OpLoad %uint %43
%45 = OpIAdd %uint %44 %36
%46 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %45
%47 = OpLoad %uint %46
OpReturnValue %47
OpFunctionEnd
%56 = OpFunction %void None %57
%58 = OpFunctionParameter %uint
%59 = OpFunctionParameter %uint
%60 = OpFunctionParameter %uint
%61 = OpFunctionParameter %uint
%62 = OpLabel
%66 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_0
%69 = OpAtomicIAdd %uint %66 %uint_4 %uint_0 %uint_10
%70 = OpIAdd %uint %69 %uint_10
%71 = OpArrayLength %uint %65 1
%72 = OpULessThanEqual %bool %70 %71
OpSelectionMerge %73 None
OpBranchConditional %72 %74 %73
%74 = OpLabel
%75 = OpIAdd %uint %69 %uint_0
%76 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %75
OpStore %76 %uint_10
%78 = OpIAdd %uint %69 %uint_1
%79 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %78
OpStore %79 %uint_23
%81 = OpIAdd %uint %69 %uint_2
%82 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %81
OpStore %82 %58
%85 = OpIAdd %uint %69 %uint_3
%86 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %85
OpStore %86 %uint_5314
%90 = OpLoad %v3uint %89
%91 = OpCompositeExtract %uint %90 0
%92 = OpCompositeExtract %uint %90 1
%93 = OpCompositeExtract %uint %90 2
%94 = OpIAdd %uint %69 %uint_4
%95 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %94
OpStore %95 %91
%97 = OpIAdd %uint %69 %uint_5
%98 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %97
OpStore %98 %92
%100 = OpIAdd %uint %69 %uint_6
%101 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %100
OpStore %101 %93
%103 = OpIAdd %uint %69 %uint_7
%104 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %103
OpStore %104 %59
%106 = OpIAdd %uint %69 %uint_8
%107 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %106
OpStore %107 %60
%109 = OpIAdd %uint %69 %uint_9
%110 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %109
OpStore %110 %61
OpBranch %73
%73 = OpLabel
OpReturn
OpFunctionEnd
%115 = OpFunction %uint None %116
%117 = OpFunctionParameter %uint
%118 = OpFunctionParameter %uint
%119 = OpFunctionParameter %uint
%120 = OpFunctionParameter %uint
%121 = OpLabel
%122 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %117
%123 = OpLoad %uint %122
%124 = OpIAdd %uint %123 %118
%125 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %124
%126 = OpLoad %uint %125
%127 = OpIAdd %uint %126 %119
%128 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %127
%129 = OpLoad %uint %128
%130 = OpIAdd %uint %129 %120
%131 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %130
%132 = OpLoad %uint %131
OpReturnValue %132
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest,
       InstBoundsAnyHitInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 0, binding = 0, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 0, binding = 1, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  const std::string defs_before =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint AnyHitNV %main "main"
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
%void = OpTypeVoid
)";

  const std::string defs_after =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint AnyHitNV %main "main" %89
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_39 Block
OpMemberDecorate %_struct_39 0 Offset 0
OpDecorate %41 DescriptorSet 7
OpDecorate %41 Binding 1
OpDecorate %_struct_63 Block
OpMemberDecorate %_struct_63 0 Offset 0
OpMemberDecorate %_struct_63 1 Offset 4
OpDecorate %65 DescriptorSet 7
OpDecorate %65 Binding 0
OpDecorate %89 BuiltIn LaunchIdNV
%void = OpTypeVoid
)";

  const std::string func_before =
      R"(%3 = OpTypeFunction %void
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
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%7 = OpTypeFunction %void
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
%20 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%uint_1 = OpConstant %uint 1
%34 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_39 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_39 = OpTypePointer StorageBuffer %_struct_39
%41 = OpVariable %_ptr_StorageBuffer__struct_39 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%57 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_63 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_63 = OpTypePointer StorageBuffer %_struct_63
%65 = OpVariable %_ptr_StorageBuffer__struct_63 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_5315 = OpConstant %uint 5315
%uint_3 = OpConstant %uint 3
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%89 = OpVariable %_ptr_Input_v3uint Input
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_51 = OpConstant %uint 51
%113 = OpConstantNull %v4float
%116 = OpTypeFunction %uint %uint %uint %uint %uint
%uint_48 = OpConstant %uint 48
%141 = OpConstantNull %uint
%uint_54 = OpConstant %uint 54
%main = OpFunction %void None %7
%24 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%133 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%134 = OpULessThan %bool %uint_0 %133
OpSelectionMerge %135 None
OpBranchConditional %134 %136 %137
%136 = OpLabel
%138 = OpLoad %uint %25
OpBranch %135
%137 = OpLabel
%140 = OpFunctionCall %void %56 %uint_48 %uint_1 %uint_0 %uint_0
OpBranch %135
%135 = OpLabel
%142 = OpPhi %uint %138 %136 %141 %137
%27 = OpAccessChain %_ptr_UniformConstant_13 %images %142
%28 = OpLoad %13 %27
%48 = OpFunctionCall %uint %33 %uint_1 %uint_1
%50 = OpULessThan %bool %142 %48
OpSelectionMerge %51 None
OpBranchConditional %50 %52 %53
%52 = OpLabel
%54 = OpLoad %13 %27
%143 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_1 %142
%144 = OpULessThan %bool %uint_0 %143
OpSelectionMerge %145 None
OpBranchConditional %144 %146 %147
%146 = OpLabel
%148 = OpLoad %13 %27
%149 = OpImageRead %v4float %148 %20
OpBranch %145
%147 = OpLabel
%150 = OpFunctionCall %void %56 %uint_51 %uint_1 %142 %uint_0
OpBranch %145
%145 = OpLabel
%151 = OpPhi %v4float %149 %146 %113 %147
OpBranch %51
%53 = OpLabel
%112 = OpFunctionCall %void %56 %uint_51 %uint_0 %142 %48
OpBranch %51
%51 = OpLabel
%114 = OpPhi %v4float %151 %145 %113 %53
%30 = OpCompositeExtract %float %114 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
%152 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%153 = OpULessThan %bool %uint_0 %152
OpSelectionMerge %154 None
OpBranchConditional %153 %155 %156
%155 = OpLabel
OpStore %31 %30
OpBranch %154
%156 = OpLabel
%158 = OpFunctionCall %void %56 %uint_54 %uint_1 %uint_0 %uint_0
OpBranch %154
%154 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%33 = OpFunction %uint None %34
%35 = OpFunctionParameter %uint
%36 = OpFunctionParameter %uint
%37 = OpLabel
%43 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %35
%44 = OpLoad %uint %43
%45 = OpIAdd %uint %44 %36
%46 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %45
%47 = OpLoad %uint %46
OpReturnValue %47
OpFunctionEnd
%56 = OpFunction %void None %57
%58 = OpFunctionParameter %uint
%59 = OpFunctionParameter %uint
%60 = OpFunctionParameter %uint
%61 = OpFunctionParameter %uint
%62 = OpLabel
%66 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_0
%69 = OpAtomicIAdd %uint %66 %uint_4 %uint_0 %uint_10
%70 = OpIAdd %uint %69 %uint_10
%71 = OpArrayLength %uint %65 1
%72 = OpULessThanEqual %bool %70 %71
OpSelectionMerge %73 None
OpBranchConditional %72 %74 %73
%74 = OpLabel
%75 = OpIAdd %uint %69 %uint_0
%76 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %75
OpStore %76 %uint_10
%78 = OpIAdd %uint %69 %uint_1
%79 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %78
OpStore %79 %uint_23
%81 = OpIAdd %uint %69 %uint_2
%82 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %81
OpStore %82 %58
%85 = OpIAdd %uint %69 %uint_3
%86 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %85
OpStore %86 %uint_5315
%90 = OpLoad %v3uint %89
%91 = OpCompositeExtract %uint %90 0
%92 = OpCompositeExtract %uint %90 1
%93 = OpCompositeExtract %uint %90 2
%94 = OpIAdd %uint %69 %uint_4
%95 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %94
OpStore %95 %91
%97 = OpIAdd %uint %69 %uint_5
%98 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %97
OpStore %98 %92
%100 = OpIAdd %uint %69 %uint_6
%101 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %100
OpStore %101 %93
%103 = OpIAdd %uint %69 %uint_7
%104 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %103
OpStore %104 %59
%106 = OpIAdd %uint %69 %uint_8
%107 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %106
OpStore %107 %60
%109 = OpIAdd %uint %69 %uint_9
%110 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %109
OpStore %110 %61
OpBranch %73
%73 = OpLabel
OpReturn
OpFunctionEnd
%115 = OpFunction %uint None %116
%117 = OpFunctionParameter %uint
%118 = OpFunctionParameter %uint
%119 = OpFunctionParameter %uint
%120 = OpFunctionParameter %uint
%121 = OpLabel
%122 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %117
%123 = OpLoad %uint %122
%124 = OpIAdd %uint %123 %118
%125 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %124
%126 = OpLoad %uint %125
%127 = OpIAdd %uint %126 %119
%128 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %127
%129 = OpLoad %uint %128
%130 = OpIAdd %uint %129 %120
%131 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %130
%132 = OpLoad %uint %131
OpReturnValue %132
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest,
       InstBoundsClosestHitInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 0, binding = 0, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 0, binding = 1, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  const std::string defs_before =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint ClosestHitNV %main "main"
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
%void = OpTypeVoid
)";

  const std::string defs_after =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint ClosestHitNV %main "main" %89
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_39 Block
OpMemberDecorate %_struct_39 0 Offset 0
OpDecorate %41 DescriptorSet 7
OpDecorate %41 Binding 1
OpDecorate %_struct_63 Block
OpMemberDecorate %_struct_63 0 Offset 0
OpMemberDecorate %_struct_63 1 Offset 4
OpDecorate %65 DescriptorSet 7
OpDecorate %65 Binding 0
OpDecorate %89 BuiltIn LaunchIdNV
%void = OpTypeVoid
)";

  const std::string func_before =
      R"(%3 = OpTypeFunction %void
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
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%7 = OpTypeFunction %void
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
%20 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%uint_1 = OpConstant %uint 1
%34 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_39 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_39 = OpTypePointer StorageBuffer %_struct_39
%41 = OpVariable %_ptr_StorageBuffer__struct_39 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%57 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_63 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_63 = OpTypePointer StorageBuffer %_struct_63
%65 = OpVariable %_ptr_StorageBuffer__struct_63 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_5316 = OpConstant %uint 5316
%uint_3 = OpConstant %uint 3
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%89 = OpVariable %_ptr_Input_v3uint Input
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_51 = OpConstant %uint 51
%113 = OpConstantNull %v4float
%116 = OpTypeFunction %uint %uint %uint %uint %uint
%uint_48 = OpConstant %uint 48
%141 = OpConstantNull %uint
%uint_54 = OpConstant %uint 54
%main = OpFunction %void None %7
%24 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%133 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%134 = OpULessThan %bool %uint_0 %133
OpSelectionMerge %135 None
OpBranchConditional %134 %136 %137
%136 = OpLabel
%138 = OpLoad %uint %25
OpBranch %135
%137 = OpLabel
%140 = OpFunctionCall %void %56 %uint_48 %uint_1 %uint_0 %uint_0
OpBranch %135
%135 = OpLabel
%142 = OpPhi %uint %138 %136 %141 %137
%27 = OpAccessChain %_ptr_UniformConstant_13 %images %142
%28 = OpLoad %13 %27
%48 = OpFunctionCall %uint %33 %uint_1 %uint_1
%50 = OpULessThan %bool %142 %48
OpSelectionMerge %51 None
OpBranchConditional %50 %52 %53
%52 = OpLabel
%54 = OpLoad %13 %27
%143 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_1 %142
%144 = OpULessThan %bool %uint_0 %143
OpSelectionMerge %145 None
OpBranchConditional %144 %146 %147
%146 = OpLabel
%148 = OpLoad %13 %27
%149 = OpImageRead %v4float %148 %20
OpBranch %145
%147 = OpLabel
%150 = OpFunctionCall %void %56 %uint_51 %uint_1 %142 %uint_0
OpBranch %145
%145 = OpLabel
%151 = OpPhi %v4float %149 %146 %113 %147
OpBranch %51
%53 = OpLabel
%112 = OpFunctionCall %void %56 %uint_51 %uint_0 %142 %48
OpBranch %51
%51 = OpLabel
%114 = OpPhi %v4float %151 %145 %113 %53
%30 = OpCompositeExtract %float %114 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
%152 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%153 = OpULessThan %bool %uint_0 %152
OpSelectionMerge %154 None
OpBranchConditional %153 %155 %156
%155 = OpLabel
OpStore %31 %30
OpBranch %154
%156 = OpLabel
%158 = OpFunctionCall %void %56 %uint_54 %uint_1 %uint_0 %uint_0
OpBranch %154
%154 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%33 = OpFunction %uint None %34
%35 = OpFunctionParameter %uint
%36 = OpFunctionParameter %uint
%37 = OpLabel
%43 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %35
%44 = OpLoad %uint %43
%45 = OpIAdd %uint %44 %36
%46 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %45
%47 = OpLoad %uint %46
OpReturnValue %47
OpFunctionEnd
%56 = OpFunction %void None %57
%58 = OpFunctionParameter %uint
%59 = OpFunctionParameter %uint
%60 = OpFunctionParameter %uint
%61 = OpFunctionParameter %uint
%62 = OpLabel
%66 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_0
%69 = OpAtomicIAdd %uint %66 %uint_4 %uint_0 %uint_10
%70 = OpIAdd %uint %69 %uint_10
%71 = OpArrayLength %uint %65 1
%72 = OpULessThanEqual %bool %70 %71
OpSelectionMerge %73 None
OpBranchConditional %72 %74 %73
%74 = OpLabel
%75 = OpIAdd %uint %69 %uint_0
%76 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %75
OpStore %76 %uint_10
%78 = OpIAdd %uint %69 %uint_1
%79 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %78
OpStore %79 %uint_23
%81 = OpIAdd %uint %69 %uint_2
%82 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %81
OpStore %82 %58
%85 = OpIAdd %uint %69 %uint_3
%86 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %85
OpStore %86 %uint_5316
%90 = OpLoad %v3uint %89
%91 = OpCompositeExtract %uint %90 0
%92 = OpCompositeExtract %uint %90 1
%93 = OpCompositeExtract %uint %90 2
%94 = OpIAdd %uint %69 %uint_4
%95 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %94
OpStore %95 %91
%97 = OpIAdd %uint %69 %uint_5
%98 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %97
OpStore %98 %92
%100 = OpIAdd %uint %69 %uint_6
%101 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %100
OpStore %101 %93
%103 = OpIAdd %uint %69 %uint_7
%104 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %103
OpStore %104 %59
%106 = OpIAdd %uint %69 %uint_8
%107 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %106
OpStore %107 %60
%109 = OpIAdd %uint %69 %uint_9
%110 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %109
OpStore %110 %61
OpBranch %73
%73 = OpLabel
OpReturn
OpFunctionEnd
%115 = OpFunction %uint None %116
%117 = OpFunctionParameter %uint
%118 = OpFunctionParameter %uint
%119 = OpFunctionParameter %uint
%120 = OpFunctionParameter %uint
%121 = OpLabel
%122 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %117
%123 = OpLoad %uint %122
%124 = OpIAdd %uint %123 %118
%125 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %124
%126 = OpLoad %uint %125
%127 = OpIAdd %uint %126 %119
%128 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %127
%129 = OpLoad %uint %128
%130 = OpIAdd %uint %129 %120
%131 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %130
%132 = OpLoad %uint %131
OpReturnValue %132
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest,
       InstBoundsMissInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 0, binding = 0, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 0, binding = 1, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  const std::string defs_before =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint MissNV %main "main"
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
%void = OpTypeVoid
)";

  const std::string defs_after =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint MissNV %main "main" %89
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_39 Block
OpMemberDecorate %_struct_39 0 Offset 0
OpDecorate %41 DescriptorSet 7
OpDecorate %41 Binding 1
OpDecorate %_struct_63 Block
OpMemberDecorate %_struct_63 0 Offset 0
OpMemberDecorate %_struct_63 1 Offset 4
OpDecorate %65 DescriptorSet 7
OpDecorate %65 Binding 0
OpDecorate %89 BuiltIn LaunchIdNV
%void = OpTypeVoid
)";

  const std::string func_before =
      R"(%3 = OpTypeFunction %void
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
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%7 = OpTypeFunction %void
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
%20 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%uint_1 = OpConstant %uint 1
%34 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_39 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_39 = OpTypePointer StorageBuffer %_struct_39
%41 = OpVariable %_ptr_StorageBuffer__struct_39 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%57 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_63 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_63 = OpTypePointer StorageBuffer %_struct_63
%65 = OpVariable %_ptr_StorageBuffer__struct_63 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_5317 = OpConstant %uint 5317
%uint_3 = OpConstant %uint 3
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%89 = OpVariable %_ptr_Input_v3uint Input
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_51 = OpConstant %uint 51
%113 = OpConstantNull %v4float
%116 = OpTypeFunction %uint %uint %uint %uint %uint
%uint_48 = OpConstant %uint 48
%141 = OpConstantNull %uint
%uint_54 = OpConstant %uint 54
%main = OpFunction %void None %7
%24 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%133 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%134 = OpULessThan %bool %uint_0 %133
OpSelectionMerge %135 None
OpBranchConditional %134 %136 %137
%136 = OpLabel
%138 = OpLoad %uint %25
OpBranch %135
%137 = OpLabel
%140 = OpFunctionCall %void %56 %uint_48 %uint_1 %uint_0 %uint_0
OpBranch %135
%135 = OpLabel
%142 = OpPhi %uint %138 %136 %141 %137
%27 = OpAccessChain %_ptr_UniformConstant_13 %images %142
%28 = OpLoad %13 %27
%48 = OpFunctionCall %uint %33 %uint_1 %uint_1
%50 = OpULessThan %bool %142 %48
OpSelectionMerge %51 None
OpBranchConditional %50 %52 %53
%52 = OpLabel
%54 = OpLoad %13 %27
%143 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_1 %142
%144 = OpULessThan %bool %uint_0 %143
OpSelectionMerge %145 None
OpBranchConditional %144 %146 %147
%146 = OpLabel
%148 = OpLoad %13 %27
%149 = OpImageRead %v4float %148 %20
OpBranch %145
%147 = OpLabel
%150 = OpFunctionCall %void %56 %uint_51 %uint_1 %142 %uint_0
OpBranch %145
%145 = OpLabel
%151 = OpPhi %v4float %149 %146 %113 %147
OpBranch %51
%53 = OpLabel
%112 = OpFunctionCall %void %56 %uint_51 %uint_0 %142 %48
OpBranch %51
%51 = OpLabel
%114 = OpPhi %v4float %151 %145 %113 %53
%30 = OpCompositeExtract %float %114 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
%152 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%153 = OpULessThan %bool %uint_0 %152
OpSelectionMerge %154 None
OpBranchConditional %153 %155 %156
%155 = OpLabel
OpStore %31 %30
OpBranch %154
%156 = OpLabel
%158 = OpFunctionCall %void %56 %uint_54 %uint_1 %uint_0 %uint_0
OpBranch %154
%154 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%33 = OpFunction %uint None %34
%35 = OpFunctionParameter %uint
%36 = OpFunctionParameter %uint
%37 = OpLabel
%43 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %35
%44 = OpLoad %uint %43
%45 = OpIAdd %uint %44 %36
%46 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %45
%47 = OpLoad %uint %46
OpReturnValue %47
OpFunctionEnd
%56 = OpFunction %void None %57
%58 = OpFunctionParameter %uint
%59 = OpFunctionParameter %uint
%60 = OpFunctionParameter %uint
%61 = OpFunctionParameter %uint
%62 = OpLabel
%66 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_0
%69 = OpAtomicIAdd %uint %66 %uint_4 %uint_0 %uint_10
%70 = OpIAdd %uint %69 %uint_10
%71 = OpArrayLength %uint %65 1
%72 = OpULessThanEqual %bool %70 %71
OpSelectionMerge %73 None
OpBranchConditional %72 %74 %73
%74 = OpLabel
%75 = OpIAdd %uint %69 %uint_0
%76 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %75
OpStore %76 %uint_10
%78 = OpIAdd %uint %69 %uint_1
%79 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %78
OpStore %79 %uint_23
%81 = OpIAdd %uint %69 %uint_2
%82 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %81
OpStore %82 %58
%85 = OpIAdd %uint %69 %uint_3
%86 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %85
OpStore %86 %uint_5317
%90 = OpLoad %v3uint %89
%91 = OpCompositeExtract %uint %90 0
%92 = OpCompositeExtract %uint %90 1
%93 = OpCompositeExtract %uint %90 2
%94 = OpIAdd %uint %69 %uint_4
%95 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %94
OpStore %95 %91
%97 = OpIAdd %uint %69 %uint_5
%98 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %97
OpStore %98 %92
%100 = OpIAdd %uint %69 %uint_6
%101 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %100
OpStore %101 %93
%103 = OpIAdd %uint %69 %uint_7
%104 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %103
OpStore %104 %59
%106 = OpIAdd %uint %69 %uint_8
%107 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %106
OpStore %107 %60
%109 = OpIAdd %uint %69 %uint_9
%110 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %109
OpStore %110 %61
OpBranch %73
%73 = OpLabel
OpReturn
OpFunctionEnd
%115 = OpFunction %uint None %116
%117 = OpFunctionParameter %uint
%118 = OpFunctionParameter %uint
%119 = OpFunctionParameter %uint
%120 = OpFunctionParameter %uint
%121 = OpLabel
%122 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %117
%123 = OpLoad %uint %122
%124 = OpIAdd %uint %123 %118
%125 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %124
%126 = OpLoad %uint %125
%127 = OpIAdd %uint %126 %119
%128 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %127
%129 = OpLoad %uint %128
%130 = OpIAdd %uint %129 %120
%131 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %130
%132 = OpLoad %uint %131
OpReturnValue %132
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
}

TEST_F(InstBindlessTest,
       InstBoundsCallableInitLoadVariableSizedSampledImagesArray) {
  // #version 460
  // #extension GL_EXT_nonuniform_qualifier : require
  // #extension GL_NV_ray_tracing : require
  //
  // layout(set = 0, binding = 0, std140) buffer StorageBuffer {
  //   uint index;
  //   float red;
  // } sbo;
  //
  // layout(set = 0, binding = 1, rgba32f) readonly uniform image2D images[];
  //
  // void main()
  // {
  //    sbo.red = imageLoad(images[sbo.index], ivec2(0, 0)).r;
  // }

  const std::string defs_before =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint CallableNV %main "main"
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
%void = OpTypeVoid
)";

  const std::string defs_after =
      R"(OpCapability RuntimeDescriptorArray
OpCapability RayTracingNV
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_NV_ray_tracing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint CallableNV %main "main" %89
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
OpDecorate %sbo DescriptorSet 0
OpDecorate %sbo Binding 0
OpDecorate %images DescriptorSet 0
OpDecorate %images Binding 1
OpDecorate %images NonWritable
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_39 Block
OpMemberDecorate %_struct_39 0 Offset 0
OpDecorate %41 DescriptorSet 7
OpDecorate %41 Binding 1
OpDecorate %_struct_63 Block
OpMemberDecorate %_struct_63 0 Offset 0
OpMemberDecorate %_struct_63 1 Offset 4
OpDecorate %65 DescriptorSet 7
OpDecorate %65 Binding 0
OpDecorate %89 BuiltIn LaunchIdNV
%void = OpTypeVoid
)";

  const std::string func_before =
      R"(%3 = OpTypeFunction %void
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
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%7 = OpTypeFunction %void
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
%20 = OpConstantComposite %v2int %int_0 %int_0
%v4float = OpTypeVector %float 4
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%uint_1 = OpConstant %uint 1
%34 = OpTypeFunction %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_39 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_39 = OpTypePointer StorageBuffer %_struct_39
%41 = OpVariable %_ptr_StorageBuffer__struct_39 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%bool = OpTypeBool
%57 = OpTypeFunction %void %uint %uint %uint %uint
%_struct_63 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_63 = OpTypePointer StorageBuffer %_struct_63
%65 = OpVariable %_ptr_StorageBuffer__struct_63 StorageBuffer
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_5318 = OpConstant %uint 5318
%uint_3 = OpConstant %uint 3
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%89 = OpVariable %_ptr_Input_v3uint Input
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_51 = OpConstant %uint 51
%113 = OpConstantNull %v4float
%116 = OpTypeFunction %uint %uint %uint %uint %uint
%uint_48 = OpConstant %uint 48
%141 = OpConstantNull %uint
%uint_54 = OpConstant %uint 54
%main = OpFunction %void None %7
%24 = OpLabel
%25 = OpAccessChain %_ptr_Uniform_uint %sbo %int_0
%133 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%134 = OpULessThan %bool %uint_0 %133
OpSelectionMerge %135 None
OpBranchConditional %134 %136 %137
%136 = OpLabel
%138 = OpLoad %uint %25
OpBranch %135
%137 = OpLabel
%140 = OpFunctionCall %void %56 %uint_48 %uint_1 %uint_0 %uint_0
OpBranch %135
%135 = OpLabel
%142 = OpPhi %uint %138 %136 %141 %137
%27 = OpAccessChain %_ptr_UniformConstant_13 %images %142
%28 = OpLoad %13 %27
%48 = OpFunctionCall %uint %33 %uint_1 %uint_1
%50 = OpULessThan %bool %142 %48
OpSelectionMerge %51 None
OpBranchConditional %50 %52 %53
%52 = OpLabel
%54 = OpLoad %13 %27
%143 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_1 %142
%144 = OpULessThan %bool %uint_0 %143
OpSelectionMerge %145 None
OpBranchConditional %144 %146 %147
%146 = OpLabel
%148 = OpLoad %13 %27
%149 = OpImageRead %v4float %148 %20
OpBranch %145
%147 = OpLabel
%150 = OpFunctionCall %void %56 %uint_51 %uint_1 %142 %uint_0
OpBranch %145
%145 = OpLabel
%151 = OpPhi %v4float %149 %146 %113 %147
OpBranch %51
%53 = OpLabel
%112 = OpFunctionCall %void %56 %uint_51 %uint_0 %142 %48
OpBranch %51
%51 = OpLabel
%114 = OpPhi %v4float %151 %145 %113 %53
%30 = OpCompositeExtract %float %114 0
%31 = OpAccessChain %_ptr_Uniform_float %sbo %int_1
%152 = OpFunctionCall %uint %115 %uint_0 %uint_0 %uint_0 %uint_0
%153 = OpULessThan %bool %uint_0 %152
OpSelectionMerge %154 None
OpBranchConditional %153 %155 %156
%155 = OpLabel
OpStore %31 %30
OpBranch %154
%156 = OpLabel
%158 = OpFunctionCall %void %56 %uint_54 %uint_1 %uint_0 %uint_0
OpBranch %154
%154 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%33 = OpFunction %uint None %34
%35 = OpFunctionParameter %uint
%36 = OpFunctionParameter %uint
%37 = OpLabel
%43 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %35
%44 = OpLoad %uint %43
%45 = OpIAdd %uint %44 %36
%46 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %45
%47 = OpLoad %uint %46
OpReturnValue %47
OpFunctionEnd
%56 = OpFunction %void None %57
%58 = OpFunctionParameter %uint
%59 = OpFunctionParameter %uint
%60 = OpFunctionParameter %uint
%61 = OpFunctionParameter %uint
%62 = OpLabel
%66 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_0
%69 = OpAtomicIAdd %uint %66 %uint_4 %uint_0 %uint_10
%70 = OpIAdd %uint %69 %uint_10
%71 = OpArrayLength %uint %65 1
%72 = OpULessThanEqual %bool %70 %71
OpSelectionMerge %73 None
OpBranchConditional %72 %74 %73
%74 = OpLabel
%75 = OpIAdd %uint %69 %uint_0
%76 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %75
OpStore %76 %uint_10
%78 = OpIAdd %uint %69 %uint_1
%79 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %78
OpStore %79 %uint_23
%81 = OpIAdd %uint %69 %uint_2
%82 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %81
OpStore %82 %58
%85 = OpIAdd %uint %69 %uint_3
%86 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %85
OpStore %86 %uint_5318
%90 = OpLoad %v3uint %89
%91 = OpCompositeExtract %uint %90 0
%92 = OpCompositeExtract %uint %90 1
%93 = OpCompositeExtract %uint %90 2
%94 = OpIAdd %uint %69 %uint_4
%95 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %94
OpStore %95 %91
%97 = OpIAdd %uint %69 %uint_5
%98 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %97
OpStore %98 %92
%100 = OpIAdd %uint %69 %uint_6
%101 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %100
OpStore %101 %93
%103 = OpIAdd %uint %69 %uint_7
%104 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %103
OpStore %104 %59
%106 = OpIAdd %uint %69 %uint_8
%107 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %106
OpStore %107 %60
%109 = OpIAdd %uint %69 %uint_9
%110 = OpAccessChain %_ptr_StorageBuffer_uint %65 %uint_1 %109
OpStore %110 %61
OpBranch %73
%73 = OpLabel
OpReturn
OpFunctionEnd
%115 = OpFunction %uint None %116
%117 = OpFunctionParameter %uint
%118 = OpFunctionParameter %uint
%119 = OpFunctionParameter %uint
%120 = OpFunctionParameter %uint
%121 = OpLabel
%122 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %117
%123 = OpLoad %uint %122
%124 = OpIAdd %uint %123 %118
%125 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %124
%126 = OpLoad %uint %125
%127 = OpIAdd %uint %126 %119
%128 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %127
%129 = OpLoad %uint %128
%130 = OpIAdd %uint %129 %120
%131 = OpAccessChain %_ptr_StorageBuffer_uint %41 %uint_0 %130
%132 = OpLoad %uint %131
OpReturnValue %132
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
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
  // layout(set = 0, binding = 0) uniform Uniforms {
  //   vec2 var0;
  // } uniforms;
  //
  // layout(set = 0, binding = 1) uniform sampler uniformSampler;
  // layout(set = 0, binding = 2) uniform texture2D uniformTex;
  // layout(set = 0, binding = 3) uniform texture2D uniformTexArr[8];
  //
  // void main() {
  //   int index = 0;
  //   float x = texture(sampler2D(uniformTexArr[nonuniformEXT(index)], uniformSampler), inTexcoord.xy).x;
  //   float y = texture(sampler2D(uniformTex, uniformSampler), inTexcoord.xy * uniforms.var0.xy).x;
  //   outColor = vec4(x, y, 0.0, 0.0);
  // }
  //
  // clang-format on

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability ShaderNonUniformEXT
OpCapability SampledImageArrayNonUniformIndexingEXT
OpExtension "SPV_EXT_descriptor_indexing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %inTexcoord %outColor
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
OpDecorate %uniformTexArr DescriptorSet 0
OpDecorate %uniformTexArr Binding 3
OpDecorate %19 NonUniformEXT
OpDecorate %22 NonUniformEXT
OpDecorate %uniformSampler DescriptorSet 0
OpDecorate %uniformSampler Binding 1
OpDecorate %inTexcoord Location 0
OpDecorate %uniformTex DescriptorSet 0
OpDecorate %uniformTex Binding 2
OpMemberDecorate %Uniforms 0 Offset 0
OpDecorate %Uniforms Block
OpDecorate %uniforms DescriptorSet 0
OpDecorate %uniforms Binding 0
OpDecorate %outColor Location 0
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability ShaderNonUniform
OpCapability SampledImageArrayNonUniformIndexing
OpExtension "SPV_EXT_descriptor_indexing"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %inTexcoord %outColor %gl_FragCoord
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
OpDecorate %uniformTexArr DescriptorSet 0
OpDecorate %uniformTexArr Binding 3
OpDecorate %19 NonUniform
OpDecorate %22 NonUniform
OpDecorate %uniformSampler DescriptorSet 0
OpDecorate %uniformSampler Binding 1
OpDecorate %inTexcoord Location 0
OpDecorate %uniformTex DescriptorSet 0
OpDecorate %uniformTex Binding 2
OpMemberDecorate %Uniforms 0 Offset 0
OpDecorate %Uniforms Block
OpDecorate %uniforms DescriptorSet 0
OpDecorate %uniforms Binding 0
OpDecorate %outColor Location 0
OpDecorate %63 NonUniform
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_75 Block
OpMemberDecorate %_struct_75 0 Offset 0
OpMemberDecorate %_struct_75 1 Offset 4
OpDecorate %77 DescriptorSet 7
OpDecorate %77 Binding 0
OpDecorate %gl_FragCoord BuiltIn FragCoord
OpDecorate %_struct_132 Block
OpMemberDecorate %_struct_132 0 Offset 0
OpDecorate %134 DescriptorSet 7
OpDecorate %134 Binding 1
OpDecorate %151 NonUniform
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
%bool = OpTypeBool
%68 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_75 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_75 = OpTypePointer StorageBuffer %_struct_75
%77 = OpVariable %_ptr_StorageBuffer__struct_75 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_1 = OpConstant %uint 1
%uint_23 = OpConstant %uint 23
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%v4uint = OpTypeVector %uint 4
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%uint_9 = OpConstant %uint 9
%uint_79 = OpConstant %uint 79
%122 = OpConstantNull %v4float
%126 = OpTypeFunction %uint %uint %uint %uint %uint
%_struct_132 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_132 = OpTypePointer StorageBuffer %_struct_132
%134 = OpVariable %_ptr_StorageBuffer__struct_132 StorageBuffer
%uint_87 = OpConstant %uint 87
%165 = OpConstantNull %v2float
%uint_89 = OpConstant %uint 89
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
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
OpStore %x %36
%39 = OpLoad %13 %uniformTex
%40 = OpLoad %23 %uniformSampler
%41 = OpSampledImage %27 %39 %40
%42 = OpLoad %v2float %inTexcoord
%47 = OpAccessChain %_ptr_Uniform_v2float %uniforms %int_0
%48 = OpLoad %v2float %47
%49 = OpFMul %v2float %42 %48
%50 = OpImageSampleImplicitLod %v4float %41 %49
%51 = OpCompositeExtract %float %50 0
OpStore %y %51
%54 = OpLoad %float %x
%55 = OpLoad %float %y
%57 = OpCompositeConstruct %v4float %54 %55 %float_0 %float_0
OpStore %outColor %57
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %3
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
%59 = OpULessThan %bool %19 %uint_8
OpSelectionMerge %60 None
OpBranchConditional %59 %61 %62
%61 = OpLabel
%63 = OpLoad %13 %21
%64 = OpSampledImage %27 %63 %26
%124 = OpBitcast %uint %19
%146 = OpFunctionCall %uint %125 %uint_0 %uint_0 %uint_3 %124
%147 = OpULessThan %bool %uint_0 %146
OpSelectionMerge %148 None
OpBranchConditional %147 %149 %150
%149 = OpLabel
%151 = OpLoad %13 %21
%152 = OpSampledImage %27 %151 %26
%153 = OpImageSampleImplicitLod %v4float %152 %32
OpBranch %148
%150 = OpLabel
%154 = OpBitcast %uint %19
%155 = OpFunctionCall %void %67 %uint_79 %uint_1 %154 %uint_0
OpBranch %148
%148 = OpLabel
%156 = OpPhi %v4float %153 %149 %122 %150
OpBranch %60
%62 = OpLabel
%66 = OpBitcast %uint %19
%121 = OpFunctionCall %void %67 %uint_79 %uint_0 %66 %uint_8
OpBranch %60
%60 = OpLabel
%123 = OpPhi %v4float %156 %148 %122 %62
%36 = OpCompositeExtract %float %123 0
OpStore %x %36
%39 = OpLoad %13 %uniformTex
%40 = OpLoad %23 %uniformSampler
%41 = OpSampledImage %27 %39 %40
%42 = OpLoad %v2float %inTexcoord
%47 = OpAccessChain %_ptr_Uniform_v2float %uniforms %int_0
%157 = OpFunctionCall %uint %125 %uint_0 %uint_0 %uint_0 %uint_0
%158 = OpULessThan %bool %uint_0 %157
OpSelectionMerge %159 None
OpBranchConditional %158 %160 %161
%160 = OpLabel
%162 = OpLoad %v2float %47
OpBranch %159
%161 = OpLabel
%164 = OpFunctionCall %void %67 %uint_87 %uint_1 %uint_0 %uint_0
OpBranch %159
%159 = OpLabel
%166 = OpPhi %v2float %162 %160 %165 %161
%49 = OpFMul %v2float %42 %166
%167 = OpSampledImage %27 %39 %40
%168 = OpFunctionCall %uint %125 %uint_0 %uint_0 %uint_2 %uint_0
%169 = OpULessThan %bool %uint_0 %168
OpSelectionMerge %170 None
OpBranchConditional %169 %171 %172
%171 = OpLabel
%173 = OpLoad %13 %uniformTex
%174 = OpSampledImage %27 %173 %40
%175 = OpImageSampleImplicitLod %v4float %174 %49
OpBranch %170
%172 = OpLabel
%177 = OpFunctionCall %void %67 %uint_89 %uint_1 %uint_0 %uint_0
OpBranch %170
%170 = OpLabel
%178 = OpPhi %v4float %175 %171 %122 %172
%51 = OpCompositeExtract %float %178 0
OpStore %y %51
%54 = OpLoad %float %x
%55 = OpLoad %float %y
%57 = OpCompositeConstruct %v4float %54 %55 %float_0 %float_0
OpStore %outColor %57
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%67 = OpFunction %void None %68
%69 = OpFunctionParameter %uint
%70 = OpFunctionParameter %uint
%71 = OpFunctionParameter %uint
%72 = OpFunctionParameter %uint
%73 = OpLabel
%79 = OpAccessChain %_ptr_StorageBuffer_uint %77 %uint_0
%82 = OpAtomicIAdd %uint %79 %uint_4 %uint_0 %uint_10
%83 = OpIAdd %uint %82 %uint_10
%84 = OpArrayLength %uint %77 1
%85 = OpULessThanEqual %bool %83 %84
OpSelectionMerge %86 None
OpBranchConditional %85 %87 %86
%87 = OpLabel
%88 = OpIAdd %uint %82 %uint_0
%90 = OpAccessChain %_ptr_StorageBuffer_uint %77 %uint_1 %88
OpStore %90 %uint_10
%92 = OpIAdd %uint %82 %uint_1
%93 = OpAccessChain %_ptr_StorageBuffer_uint %77 %uint_1 %92
OpStore %93 %uint_23
%95 = OpIAdd %uint %82 %uint_2
%96 = OpAccessChain %_ptr_StorageBuffer_uint %77 %uint_1 %95
OpStore %96 %69
%98 = OpIAdd %uint %82 %uint_3
%99 = OpAccessChain %_ptr_StorageBuffer_uint %77 %uint_1 %98
OpStore %99 %uint_4
%102 = OpLoad %v4float %gl_FragCoord
%104 = OpBitcast %v4uint %102
%105 = OpCompositeExtract %uint %104 0
%106 = OpIAdd %uint %82 %uint_4
%107 = OpAccessChain %_ptr_StorageBuffer_uint %77 %uint_1 %106
OpStore %107 %105
%108 = OpCompositeExtract %uint %104 1
%110 = OpIAdd %uint %82 %uint_5
%111 = OpAccessChain %_ptr_StorageBuffer_uint %77 %uint_1 %110
OpStore %111 %108
%113 = OpIAdd %uint %82 %uint_7
%114 = OpAccessChain %_ptr_StorageBuffer_uint %77 %uint_1 %113
OpStore %114 %70
%115 = OpIAdd %uint %82 %uint_8
%116 = OpAccessChain %_ptr_StorageBuffer_uint %77 %uint_1 %115
OpStore %116 %71
%118 = OpIAdd %uint %82 %uint_9
%119 = OpAccessChain %_ptr_StorageBuffer_uint %77 %uint_1 %118
OpStore %119 %72
OpBranch %86
%86 = OpLabel
OpReturn
OpFunctionEnd
%125 = OpFunction %uint None %126
%127 = OpFunctionParameter %uint
%128 = OpFunctionParameter %uint
%129 = OpFunctionParameter %uint
%130 = OpFunctionParameter %uint
%131 = OpLabel
%135 = OpAccessChain %_ptr_StorageBuffer_uint %134 %uint_0 %127
%136 = OpLoad %uint %135
%137 = OpIAdd %uint %136 %128
%138 = OpAccessChain %_ptr_StorageBuffer_uint %134 %uint_0 %137
%139 = OpLoad %uint %138
%140 = OpIAdd %uint %139 %129
%141 = OpAccessChain %_ptr_StorageBuffer_uint %134 %uint_0 %140
%142 = OpLoad %uint %141
%143 = OpIAdd %uint %142 %130
%144 = OpAccessChain %_ptr_StorageBuffer_uint %134 %uint_0 %143
%145 = OpLoad %uint %144
OpReturnValue %145
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBindlessCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u, true, true, false, false, false);
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

  const std::string text = R"(
               OpCapability Shader
;CHECK:        OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor
;CHECK:        OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor %130 %157 %gl_FragCoord
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
 ;CHECK:       OpDecorate %_runtimearr_uint ArrayStride 4
 ;CHECK:       OpDecorate %_struct_128 Block
 ;CHECK:       OpMemberDecorate %_struct_128 0 Offset 0
 ;CHECK:       OpDecorate %130 DescriptorSet 7
 ;CHECK:       OpDecorate %130 Binding 1
 ;CHECK:       OpDecorate %_struct_155 Block
 ;CHECK:       OpMemberDecorate %_struct_155 0 Offset 0
 ;CHECK:       OpMemberDecorate %_struct_155 1 Offset 4
 ;CHECK:       OpDecorate %157 DescriptorSet 7
 ;CHECK:       OpDecorate %157 Binding 0
 ;CHECK:       OpDecorate %gl_FragCoord BuiltIn FragCoord
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
 ;CHECK:      %uint_7 = OpConstant %uint 7
 ;CHECK:      %uint_1 = OpConstant %uint 1
 ;CHECK:         %122 = OpTypeFunction %uint %uint %uint %uint
 ;CHECK: %_runtimearr_uint = OpTypeRuntimeArray %uint
 ;CHECK: %_struct_128 = OpTypeStruct %_runtimearr_uint
 ;CHECK: %_ptr_StorageBuffer__struct_128 = OpTypePointer StorageBuffer %_struct_128
 ;CHECK:         %130 = OpVariable %_ptr_StorageBuffer__struct_128 StorageBuffer
 ;CHECK: %_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
 ;CHECK:     %uint_4 = OpConstant %uint 4
 ;CHECK:         %148 = OpTypeFunction %void %uint %uint %uint %uint %uint
 ;CHECK: %_struct_155 = OpTypeStruct %uint %_runtimearr_uint
 ;CHECK: %_ptr_StorageBuffer__struct_155 = OpTypePointer StorageBuffer %_struct_155
 ;CHECK:        %157 = OpVariable %_ptr_StorageBuffer__struct_155 StorageBuffer
 ;CHECK:    %uint_11 = OpConstant %uint 11
 ;CHECK:    %uint_23 = OpConstant %uint 23
 ;CHECK:     %uint_2 = OpConstant %uint 2
 ;CHECK:      %uint_3 = OpConstant %uint 3
 ;CHECK:%_ptr_Input_v4float = OpTypePointer Input %v4float
 ;CHECK:%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
 ;CHECK:     %v4uint = OpTypeVector %uint 4
 ;CHECK:     %uint_5 = OpConstant %uint 5
 ;CHECK:     %uint_8 = OpConstant %uint 8
 ;CHECK:     %uint_9 = OpConstant %uint 9
 ;CHECK:    %uint_10 = OpConstant %uint 10
 ;CHECK:    %uint_71 = OpConstant %uint 71
 ;CHECK:        %202 = OpConstantNull %v2float
 ;CHECK:    %uint_75 = OpConstant %uint 75
     %MainPs = OpFunction %void None %3
          %5 = OpLabel
 ;CHECK: %140 = OpFunctionCall %uint %121 %uint_1 %uint_1 %uint_0
 ;CHECK:        OpBranch %117
 ;CHECK: %117 = OpLabel
 ;CHECK:        OpBranch %116
 ;CHECK: %116 = OpLabel
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
 ;CHECK:        %119 = OpIAdd %uint %uint_0 %uint_7
 ;CHECK:        %141 = OpULessThan %bool %119 %140
 ;CHECK:               OpSelectionMerge %143 None
 ;CHECK:               OpBranchConditional %141 %144 %145
 ;CHECK:        %144 = OpLabel
 ;CHECK:        %146 = OpLoad %v2float %86
 ;CHECK:               OpBranch %143
 ;CHECK:        %145 = OpLabel
 ;CHECK:        %201 = OpFunctionCall %void %147 %uint_71 %uint_4 %uint_0 %119 %140
 ;CHECK:               OpBranch %143
 ;CHECK:        %143 = OpLabel
 ;CHECK:        %203 = OpPhi %v2float %146 %144 %202 %145
               OpBranch %91
         %88 = OpLabel
         %89 = OpAccessChain %_ptr_Uniform_v2float %__0 %int_1
         %90 = OpLoad %v2float %89
 ;CHECK-NOT:     %90 = OpLoad %v2float %89
 ;CHECK:        %204 = OpIAdd %uint %uint_8 %uint_7
 ;CHECK:        %205 = OpULessThan %bool %204 %140
 ;CHECK:               OpSelectionMerge %206 None
 ;CHECK:               OpBranchConditional %205 %207 %208
 ;CHECK:        %207 = OpLabel
 ;CHECK:        %209 = OpLoad %v2float %89
 ;CHECK:               OpBranch %206
 ;CHECK:        %208 = OpLabel
 ;CHECK:        %211 = OpFunctionCall %void %147 %uint_75 %uint_4 %uint_0 %204 %140
 ;CHECK:               OpBranch %206
 ;CHECK:        %206 = OpLabel
 ;CHECK:        %212 = OpPhi %v2float %209 %207 %202 %208
               OpBranch %91
         %91 = OpLabel
        %115 = OpPhi %v2float %87 %85 %90 %88
 ;CHECK-NOT:       %115 = OpPhi %v2float %87 %85 %90 %88
 ;CHECK:           %115 = OpPhi %v2float %203 %143 %212 %206
         %95 = OpFAdd %v2float %69 %115
         %96 = OpLoad %49 %g_tColor
         %97 = OpLoad %53 %g_sAniso
         %98 = OpSampledImage %57 %96 %97
        %100 = OpImageSampleImplicitLod %v4float %98 %95
               OpStore %_entryPointOutput_vColor %100
               OpReturn
               OpFunctionEnd
 ;CHECK:        %121 = OpFunction %uint None %122
 ;CHECK:        %123 = OpFunctionParameter %uint
 ;CHECK:        %124 = OpFunctionParameter %uint
 ;CHECK:        %125 = OpFunctionParameter %uint
 ;CHECK:        %126 = OpLabel
 ;CHECK:        %132 = OpAccessChain %_ptr_StorageBuffer_uint %130 %uint_0 %123
 ;CHECK:        %133 = OpLoad %uint %132
 ;CHECK:        %134 = OpIAdd %uint %133 %124
 ;CHECK:        %135 = OpAccessChain %_ptr_StorageBuffer_uint %130 %uint_0 %134
 ;CHECK:        %136 = OpLoad %uint %135
 ;CHECK:        %137 = OpIAdd %uint %136 %125
 ;CHECK:        %138 = OpAccessChain %_ptr_StorageBuffer_uint %130 %uint_0 %137
 ;CHECK:        %139 = OpLoad %uint %138
 ;CHECK:               OpReturnValue %139
 ;CHECK:               OpFunctionEnd
 ;CHECK:        %147 = OpFunction %void None %148
 ;CHECK:        %149 = OpFunctionParameter %uint
 ;CHECK:        %150 = OpFunctionParameter %uint
 ;CHECK:        %151 = OpFunctionParameter %uint
 ;CHECK:        %152 = OpFunctionParameter %uint
 ;CHECK:        %153 = OpFunctionParameter %uint
 ;CHECK:        %154 = OpLabel
 ;CHECK:        %158 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_0
 ;CHECK:        %160 = OpAtomicIAdd %uint %158 %uint_4 %uint_0 %uint_11
 ;CHECK:        %161 = OpIAdd %uint %160 %uint_11
 ;CHECK:        %162 = OpArrayLength %uint %157 1
 ;CHECK:        %163 = OpULessThanEqual %bool %161 %162
 ;CHECK:               OpSelectionMerge %164 None
 ;CHECK:               OpBranchConditional %163 %165 %164
 ;CHECK:        %165 = OpLabel
 ;CHECK:        %166 = OpIAdd %uint %160 %uint_0
 ;CHECK:        %167 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_1 %166
 ;CHECK:               OpStore %167 %uint_11
 ;CHECK:        %169 = OpIAdd %uint %160 %uint_1
 ;CHECK:        %170 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_1 %169
 ;CHECK:               OpStore %170 %uint_23
 ;CHECK:        %172 = OpIAdd %uint %160 %uint_2
 ;CHECK:        %173 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_1 %172
 ;CHECK:               OpStore %173 %149
 ;CHECK:        %175 = OpIAdd %uint %160 %uint_3
 ;CHECK:        %176 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_1 %175
 ;CHECK:               OpStore %176 %uint_4
 ;CHECK:        %179 = OpLoad %v4float %gl_FragCoord
 ;CHECK:        %181 = OpBitcast %v4uint %179
 ;CHECK:        %182 = OpCompositeExtract %uint %181 0
 ;CHECK:        %183 = OpIAdd %uint %160 %uint_4
 ;CHECK:        %184 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_1 %183
 ;CHECK:               OpStore %184 %182
 ;CHECK:        %185 = OpCompositeExtract %uint %181 1
 ;CHECK:        %187 = OpIAdd %uint %160 %uint_5
 ;CHECK:        %188 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_1 %187
 ;CHECK:               OpStore %188 %185
 ;CHECK:        %189 = OpIAdd %uint %160 %uint_7
 ;CHECK:        %190 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_1 %189
 ;CHECK:               OpStore %190 %150
 ;CHECK:        %192 = OpIAdd %uint %160 %uint_8
 ;CHECK:        %193 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_1 %192
 ;CHECK:               OpStore %193 %151
 ;CHECK:        %195 = OpIAdd %uint %160 %uint_9
 ;CHECK:        %196 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_1 %195
 ;CHECK:               OpStore %196 %152
 ;CHECK:        %198 = OpIAdd %uint %160 %uint_10
 ;CHECK:        %199 = OpAccessChain %_ptr_StorageBuffer_uint %157 %uint_1 %198
 ;CHECK:               OpStore %199 %153
 ;CHECK:               OpBranch %164
 ;CHECK:        %164 = OpLabel
 ;CHECK:               OpReturn
 ;CHECK:               OpFunctionEnd
 )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, false, true);
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

  const std::string text = R"(
               OpCapability Shader
;CHECK:               OpExtension "SPV_KHR_storage_buffer_storage_class"
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
;CHECK:               OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:               OpDecorate %_struct_111 Block
;CHECK:               OpMemberDecorate %_struct_111 0 Offset 0
;CHECK:               OpDecorate %113 DescriptorSet 7
;CHECK:               OpDecorate %113 Binding 1
;CHECK:               OpDecorate %_struct_139 Block
;CHECK:               OpMemberDecorate %_struct_139 0 Offset 0
;CHECK:               OpMemberDecorate %_struct_139 1 Offset 4
;CHECK:               OpDecorate %141 DescriptorSet 7
;CHECK:               OpDecorate %141 Binding 0
;CHECK:               OpDecorate %gl_FragCoord BuiltIn FragCoord
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
;CHECK:     %uint_0 = OpConstant %uint 0
;CHECK:    %uint_80 = OpConstant %uint 80
;CHECK:    %uint_64 = OpConstant %uint 64
;CHECK:     %uint_7 = OpConstant %uint 7
;CHECK:     %uint_2 = OpConstant %uint 2
;CHECK:     %uint_1 = OpConstant %uint 1
;CHECK:        %105 = OpTypeFunction %uint %uint %uint %uint
;CHECK:%_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK:%_struct_111 = OpTypeStruct %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_111 = OpTypePointer StorageBuffer %_struct_111
;CHECK:        %113 = OpVariable %_ptr_StorageBuffer__struct_111 StorageBuffer
;CHECK:%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:       %bool = OpTypeBool
;CHECK:     %uint_4 = OpConstant %uint 4
;CHECK:        %132 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK:%_struct_139 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_139 = OpTypePointer StorageBuffer %_struct_139
;CHECK:        %141 = OpVariable %_ptr_StorageBuffer__struct_139 StorageBuffer
;CHECK:    %uint_11 = OpConstant %uint 11
;CHECK:    %uint_23 = OpConstant %uint 23
;CHECK:     %uint_3 = OpConstant %uint 3
;CHECK:%_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK:%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK:     %v4uint = OpTypeVector %uint 4
;CHECK:     %uint_5 = OpConstant %uint 5
;CHECK:     %uint_8 = OpConstant %uint 8
;CHECK:     %uint_9 = OpConstant %uint 9
;CHECK:    %uint_10 = OpConstant %uint 10
;CHECK:    %uint_78 = OpConstant %uint 78
;CHECK:        %185 = OpConstantNull %v2float
     %MainPs = OpFunction %void None %3
          %5 = OpLabel
;CHECK:        %123 = OpFunctionCall %uint %104 %uint_1 %uint_2 %uint_0
;CHECK:               OpBranch %93
;CHECK:         %93 = OpLabel
;CHECK:               OpBranch %92
;CHECK:         %92 = OpLabel
         %66 = OpLoad %v2float %i_vTextureCoords
         %79 = OpAccessChain %_ptr_PushConstant_uint %__0 %int_0
         %80 = OpLoad %uint %79
         %81 = OpAccessChain %_ptr_Uniform_v2float %_ %int_0 %80 %int_2
         %82 = OpLoad %v2float %81
;CHECK-NOT:     %82 = OpLoad %v2float %81
;CHECK:         %96 = OpIMul %uint %uint_80 %80
;CHECK:         %97 = OpIAdd %uint %uint_0 %96
;CHECK:         %99 = OpIAdd %uint %97 %uint_64
;CHECK:        %101 = OpIAdd %uint %99 %uint_7
;CHECK:        %125 = OpULessThan %bool %101 %123
;CHECK:               OpSelectionMerge %127 None
;CHECK:               OpBranchConditional %125 %128 %129
;CHECK:        %128 = OpLabel
;CHECK:        %130 = OpLoad %v2float %81
;CHECK:               OpBranch %127
;CHECK:        %129 = OpLabel
;CHECK:        %184 = OpFunctionCall %void %131 %uint_78 %uint_4 %uint_0 %101 %123
;CHECK:               OpBranch %127
;CHECK:        %127 = OpLabel
;CHECK:        %186 = OpPhi %v2float %130 %128 %185 %129
         %86 = OpFAdd %v2float %66 %82
;CHECK-NOT:         %86 = OpFAdd %v2float %66 %82
;CHECK:             %86 = OpFAdd %v2float %66 %186
         %87 = OpLoad %46 %g_tColor
         %88 = OpLoad %50 %g_sAniso
         %89 = OpSampledImage %54 %87 %88
         %91 = OpImageSampleImplicitLod %v4float %89 %86
               OpStore %_entryPointOutput_vColor %91
               OpReturn
               OpFunctionEnd
;CHECK:        %104 = OpFunction %uint None %105
;CHECK:        %106 = OpFunctionParameter %uint
;CHECK:        %107 = OpFunctionParameter %uint
;CHECK:        %108 = OpFunctionParameter %uint
;CHECK:        %109 = OpLabel
;CHECK:        %115 = OpAccessChain %_ptr_StorageBuffer_uint %113 %uint_0 %106
;CHECK:        %116 = OpLoad %uint %115
;CHECK:        %117 = OpIAdd %uint %116 %107
;CHECK:        %118 = OpAccessChain %_ptr_StorageBuffer_uint %113 %uint_0 %117
;CHECK:        %119 = OpLoad %uint %118
;CHECK:        %120 = OpIAdd %uint %119 %108
;CHECK:        %121 = OpAccessChain %_ptr_StorageBuffer_uint %113 %uint_0 %120
;CHECK:        %122 = OpLoad %uint %121
;CHECK:               OpReturnValue %122
;CHECK:               OpFunctionEnd
;CHECK:        %131 = OpFunction %void None %132
;CHECK:        %133 = OpFunctionParameter %uint
;CHECK:        %134 = OpFunctionParameter %uint
;CHECK:        %135 = OpFunctionParameter %uint
;CHECK:        %136 = OpFunctionParameter %uint
;CHECK:        %137 = OpFunctionParameter %uint
;CHECK:        %138 = OpLabel
;CHECK:        %142 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_0
;CHECK:        %144 = OpAtomicIAdd %uint %142 %uint_4 %uint_0 %uint_11
;CHECK:        %145 = OpIAdd %uint %144 %uint_11
;CHECK:        %146 = OpArrayLength %uint %141 1
;CHECK:        %147 = OpULessThanEqual %bool %145 %146
;CHECK:               OpSelectionMerge %148 None
;CHECK:               OpBranchConditional %147 %149 %148
;CHECK:        %149 = OpLabel
;CHECK:        %150 = OpIAdd %uint %144 %uint_0
;CHECK:        %151 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_1 %150
;CHECK:               OpStore %151 %uint_11
;CHECK:        %153 = OpIAdd %uint %144 %uint_1
;CHECK:        %154 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_1 %153
;CHECK:               OpStore %154 %uint_23
;CHECK:        %155 = OpIAdd %uint %144 %uint_2
;CHECK:        %156 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_1 %155
;CHECK:               OpStore %156 %133
;CHECK:        %158 = OpIAdd %uint %144 %uint_3
;CHECK:        %159 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_1 %158
;CHECK:               OpStore %159 %uint_4
;CHECK:        %162 = OpLoad %v4float %gl_FragCoord
;CHECK:        %164 = OpBitcast %v4uint %162
;CHECK:        %165 = OpCompositeExtract %uint %164 0
;CHECK:        %166 = OpIAdd %uint %144 %uint_4
;CHECK:        %167 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_1 %166
;CHECK:               OpStore %167 %165
;CHECK:        %168 = OpCompositeExtract %uint %164 1
;CHECK:        %170 = OpIAdd %uint %144 %uint_5
;CHECK:        %171 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_1 %170
;CHECK:               OpStore %171 %168
;CHECK:        %172 = OpIAdd %uint %144 %uint_7
;CHECK:        %173 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_1 %172
;CHECK:               OpStore %173 %134
;CHECK:        %175 = OpIAdd %uint %144 %uint_8
;CHECK:        %176 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_1 %175
;CHECK:               OpStore %176 %135
;CHECK:        %178 = OpIAdd %uint %144 %uint_9
;CHECK:        %179 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_1 %178
;CHECK:               OpStore %179 %136
;CHECK:        %181 = OpIAdd %uint %144 %uint_10
;CHECK:        %182 = OpAccessChain %_ptr_StorageBuffer_uint %141 %uint_1 %181
;CHECK:               OpStore %182 %137
;CHECK:               OpBranch %148
;CHECK:        %148 = OpLabel
;CHECK:               OpReturn
;CHECK:               OpFunctionEnd
 )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, false, true);
}

TEST_F(InstBindlessTest, UniformArrayRefWithDescInit) {
  // The buffer-oob and desc-init checks should use the same debug
  // output buffer write function.
  //
  // Same source as UniformArrayRefNoDescInit

  const std::string text = R"(
               OpCapability Shader
;CHECK:               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor
;CHECK:               OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor %113 %144 %gl_FragCoord
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
;CHECK:               OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:               OpDecorate %_struct_111 Block
;CHECK:               OpMemberDecorate %_struct_111 0 Offset 0
;CHECK:               OpDecorate %113 DescriptorSet 7
;CHECK:               OpDecorate %113 Binding 1
;CHECK:               OpDecorate %_struct_142 Block
;CHECK:               OpMemberDecorate %_struct_142 0 Offset 0
;CHECK:               OpMemberDecorate %_struct_142 1 Offset 4
;CHECK:               OpDecorate %144 DescriptorSet 7
;CHECK:               OpDecorate %144 Binding 0
;CHECK:               OpDecorate %gl_FragCoord BuiltIn FragCoord
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
;CHECK:     %uint_0 = OpConstant %uint 0
;CHECK:    %uint_80 = OpConstant %uint 80
;CHECK:    %uint_64 = OpConstant %uint 64
;CHECK:     %uint_7 = OpConstant %uint 7
;CHECK:     %uint_2 = OpConstant %uint 2
;CHECK:        %104 = OpTypeFunction %uint %uint %uint %uint %uint
;CHECK:%_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK:%_struct_111 = OpTypeStruct %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_111 = OpTypePointer StorageBuffer %_struct_111
;CHECK:        %113 = OpVariable %_ptr_StorageBuffer__struct_111 StorageBuffer
;CHECK:%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:       %bool = OpTypeBool
;CHECK:     %uint_4 = OpConstant %uint 4
;CHECK:        %135 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK:%_struct_142 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_142 = OpTypePointer StorageBuffer %_struct_142
;CHECK:        %144 = OpVariable %_ptr_StorageBuffer__struct_142 StorageBuffer
;CHECK:    %uint_11 = OpConstant %uint 11
;CHECK:     %uint_1 = OpConstant %uint 1
;CHECK:    %uint_23 = OpConstant %uint 23
;CHECK:     %uint_3 = OpConstant %uint 3
;CHECK:%_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK:%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK:     %v4uint = OpTypeVector %uint 4
;CHECK:     %uint_5 = OpConstant %uint 5
;CHECK:     %uint_8 = OpConstant %uint 8
;CHECK:     %uint_9 = OpConstant %uint 9
;CHECK:    %uint_10 = OpConstant %uint 10
;CHECK:    %uint_78 = OpConstant %uint 78
;CHECK:        %189 = OpConstantNull %v2float
;CHECK:    %uint_83 = OpConstant %uint 83
;CHECK:        %201 = OpConstantNull %v4float
     %MainPs = OpFunction %void None %3
          %5 = OpLabel
;CHECK:        %126 = OpFunctionCall %uint %103 %uint_0 %uint_0 %uint_2 %uint_0
;CHECK:        %191 = OpFunctionCall %uint %103 %uint_0 %uint_0 %uint_0 %uint_0
;CHECK:               OpBranch %93
;CHECK:         %93 = OpLabel
;CHECK:               OpBranch %92
;CHECK:         %92 = OpLabel
         %66 = OpLoad %v2float %i_vTextureCoords
         %79 = OpAccessChain %_ptr_PushConstant_uint %__0 %int_0
         %80 = OpLoad %uint %79
         %81 = OpAccessChain %_ptr_Uniform_v2float %_ %int_0 %80 %int_2
         %82 = OpLoad %v2float %81
         %86 = OpFAdd %v2float %66 %82
;CHECK-NOT: %82 = OpLoad %v2float %81
;CHECK-NOT: %86 = OpFAdd %v2float %66 %82
;CHECK:         %96 = OpIMul %uint %uint_80 %80
;CHECK:         %97 = OpIAdd %uint %uint_0 %96
;CHECK:         %99 = OpIAdd %uint %97 %uint_64
;CHECK:        %101 = OpIAdd %uint %99 %uint_7
;CHECK:        %128 = OpULessThan %bool %101 %126
;CHECK:               OpSelectionMerge %130 None
;CHECK:               OpBranchConditional %128 %131 %132
;CHECK:        %131 = OpLabel
;CHECK:        %133 = OpLoad %v2float %81
;CHECK:               OpBranch %130
;CHECK:        %132 = OpLabel
;CHECK:        %188 = OpFunctionCall %void %134 %uint_78 %uint_4 %uint_0 %101 %126
;CHECK:               OpBranch %130
;CHECK:        %130 = OpLabel
;CHECK:        %190 = OpPhi %v2float %133 %131 %189 %132
;CHECK:         %86 = OpFAdd %v2float %66 %190
         %87 = OpLoad %46 %g_tColor
         %88 = OpLoad %50 %g_sAniso
         %89 = OpSampledImage %54 %87 %88
         %91 = OpImageSampleImplicitLod %v4float %89 %86
               OpStore %_entryPointOutput_vColor %91
;CHECK-NOT: %91 = OpImageSampleImplicitLod %v4float %89 %86
;CHECK-NOT:       OpStore %_entryPointOutput_vColor %91
;CHECK:        %192 = OpULessThan %bool %uint_0 %191
;CHECK:               OpSelectionMerge %193 None
;CHECK:               OpBranchConditional %192 %194 %195
;CHECK:        %194 = OpLabel
;CHECK:        %196 = OpLoad %46 %g_tColor
;CHECK:        %197 = OpSampledImage %54 %196 %88
;CHECK:        %198 = OpImageSampleImplicitLod %v4float %197 %86
;CHECK:               OpBranch %193
;CHECK:        %195 = OpLabel
;CHECK:        %200 = OpFunctionCall %void %134 %uint_83 %uint_1 %uint_0 %uint_0 %uint_0
;CHECK:               OpBranch %193
;CHECK:        %193 = OpLabel
;CHECK:        %202 = OpPhi %v4float %198 %194 %201 %195
;CHECK:               OpStore %_entryPointOutput_vColor %202
               OpReturn
               OpFunctionEnd
;CHECK:        %103 = OpFunction %uint None %104
;CHECK:        %105 = OpFunctionParameter %uint
;CHECK:        %106 = OpFunctionParameter %uint
;CHECK:        %107 = OpFunctionParameter %uint
;CHECK:        %108 = OpFunctionParameter %uint
;CHECK:        %109 = OpLabel
;CHECK:        %115 = OpAccessChain %_ptr_StorageBuffer_uint %113 %uint_0 %105
;CHECK:        %116 = OpLoad %uint %115
;CHECK:        %117 = OpIAdd %uint %116 %106
;CHECK:        %118 = OpAccessChain %_ptr_StorageBuffer_uint %113 %uint_0 %117
;CHECK:        %119 = OpLoad %uint %118
;CHECK:        %120 = OpIAdd %uint %119 %107
;CHECK:        %121 = OpAccessChain %_ptr_StorageBuffer_uint %113 %uint_0 %120
;CHECK:        %122 = OpLoad %uint %121
;CHECK:        %123 = OpIAdd %uint %122 %108
;CHECK:        %124 = OpAccessChain %_ptr_StorageBuffer_uint %113 %uint_0 %123
;CHECK:        %125 = OpLoad %uint %124
;CHECK:               OpReturnValue %125
;CHECK:               OpFunctionEnd
;CHECK:        %134 = OpFunction %void None %135
;CHECK:        %136 = OpFunctionParameter %uint
;CHECK:        %137 = OpFunctionParameter %uint
;CHECK:        %138 = OpFunctionParameter %uint
;CHECK:        %139 = OpFunctionParameter %uint
;CHECK:        %140 = OpFunctionParameter %uint
;CHECK:        %141 = OpLabel
;CHECK:        %145 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_0
;CHECK:        %147 = OpAtomicIAdd %uint %145 %uint_4 %uint_0 %uint_11
;CHECK:        %148 = OpIAdd %uint %147 %uint_11
;CHECK:        %149 = OpArrayLength %uint %144 1
;CHECK:        %150 = OpULessThanEqual %bool %148 %149
;CHECK:               OpSelectionMerge %151 None
;CHECK:               OpBranchConditional %150 %152 %151
;CHECK:        %152 = OpLabel
;CHECK:        %153 = OpIAdd %uint %147 %uint_0
;CHECK:        %155 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_1 %153
;CHECK:               OpStore %155 %uint_11
;CHECK:        %157 = OpIAdd %uint %147 %uint_1
;CHECK:        %158 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_1 %157
;CHECK:               OpStore %158 %uint_23
;CHECK:        %159 = OpIAdd %uint %147 %uint_2
;CHECK:        %160 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_1 %159
;CHECK:               OpStore %160 %136
;CHECK:        %162 = OpIAdd %uint %147 %uint_3
;CHECK:        %163 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_1 %162
;CHECK:               OpStore %163 %uint_4
;CHECK:        %166 = OpLoad %v4float %gl_FragCoord
;CHECK:        %168 = OpBitcast %v4uint %166
;CHECK:        %169 = OpCompositeExtract %uint %168 0
;CHECK:        %170 = OpIAdd %uint %147 %uint_4
;CHECK:        %171 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_1 %170
;CHECK:               OpStore %171 %169
;CHECK:        %172 = OpCompositeExtract %uint %168 1
;CHECK:        %174 = OpIAdd %uint %147 %uint_5
;CHECK:        %175 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_1 %174
;CHECK:               OpStore %175 %172
;CHECK:        %176 = OpIAdd %uint %147 %uint_7
;CHECK:        %177 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_1 %176
;CHECK:               OpStore %177 %137
;CHECK:        %179 = OpIAdd %uint %147 %uint_8
;CHECK:        %180 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_1 %179
;CHECK:               OpStore %180 %138
;CHECK:        %182 = OpIAdd %uint %147 %uint_9
;CHECK:        %183 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_1 %182
;CHECK:               OpStore %183 %139
;CHECK:        %185 = OpIAdd %uint %147 %uint_10
;CHECK:        %186 = OpAccessChain %_ptr_StorageBuffer_uint %144 %uint_1 %185
;CHECK:               OpStore %186 %140
;CHECK:               OpBranch %151
;CHECK:        %151 = OpLabel
;CHECK:               OpReturn
;CHECK:               OpFunctionEnd
 )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, true, true,
                                               true, false, true);
}

TEST_F(InstBindlessTest, Descriptor16BitIdxRef) {
  // Check that descriptor indexed with 16bit index is inbounds and
  // initialized
  //
  // Use Simple source with min16uint g_nDataIdx

  const std::string text = R"(
               OpCapability Shader
               OpCapability Int16
               OpCapability StoragePushConstant16
;CHECK:               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %g_tColor %_ %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor
;CHECK:               OpEntryPoint Fragment %MainPs "MainPs" %g_tColor %_ %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor %60 %gl_FragCoord %119
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
               OpDecorate %g_tColor DescriptorSet 0
               OpDecorate %g_tColor Binding 0
               OpMemberDecorate %PerViewConstantBuffer_t 0 Offset 0
               OpDecorate %PerViewConstantBuffer_t Block
               OpDecorate %g_sAniso DescriptorSet 0
               OpDecorate %g_sAniso Binding 0
               OpDecorate %i_vTextureCoords Location 0
               OpDecorate %_entryPointOutput_vColor Location 0
;CHECK:               OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:               OpDecorate %_struct_58 Block
;CHECK:               OpMemberDecorate %_struct_58 0 Offset 0
;CHECK:               OpMemberDecorate %_struct_58 1 Offset 4
;CHECK:               OpDecorate %60 DescriptorSet 7
;CHECK:               OpDecorate %60 Binding 0
;CHECK:               OpDecorate %gl_FragCoord BuiltIn FragCoord
;CHECK:               OpDecorate %_struct_117 Block
;CHECK:               OpMemberDecorate %_struct_117 0 Offset 0
;CHECK:               OpDecorate %119 DescriptorSet 7
;CHECK:               OpDecorate %119 Binding 1
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
;CHECK:     %uint_0 = OpConstant %uint 0
;CHECK:       %bool = OpTypeBool
;CHECK:         %51 = OpTypeFunction %void %uint %uint %uint %uint
;CHECK:%_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK: %_struct_58 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_58 = OpTypePointer StorageBuffer %_struct_58
;CHECK:         %60 = OpVariable %_ptr_StorageBuffer__struct_58 StorageBuffer
;CHECK:%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:    %uint_10 = OpConstant %uint 10
;CHECK:     %uint_4 = OpConstant %uint 4
;CHECK:     %uint_1 = OpConstant %uint 1
;CHECK:    %uint_23 = OpConstant %uint 23
;CHECK:     %uint_2 = OpConstant %uint 2
;CHECK:     %uint_3 = OpConstant %uint 3
;CHECK:%_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK:%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK:     %v4uint = OpTypeVector %uint 4
;CHECK:     %uint_5 = OpConstant %uint 5
;CHECK:     %uint_7 = OpConstant %uint 7
;CHECK:     %uint_8 = OpConstant %uint 8
;CHECK:     %uint_9 = OpConstant %uint 9
;CHECK:    %uint_60 = OpConstant %uint 60
;CHECK:        %106 = OpConstantNull %v4float
;CHECK:        %111 = OpTypeFunction %uint %uint %uint %uint %uint
;CHECK:%_struct_117 = OpTypeStruct %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_117 = OpTypePointer StorageBuffer %_struct_117
;CHECK:        %119 = OpVariable %_ptr_StorageBuffer__struct_117 StorageBuffer
     %MainPs = OpFunction %void None %10
         %30 = OpLabel
;CHECK:               OpBranch %108
;CHECK:        %108 = OpLabel
;CHECK:               OpBranch %39
;CHECK:         %39 = OpLabel
         %31 = OpLoad %v2float %i_vTextureCoords
         %32 = OpAccessChain %_ptr_PushConstant_ushort %_ %int_0
         %33 = OpLoad %ushort %32
         %34 = OpAccessChain %_ptr_UniformConstant_16 %g_tColor %33
         %35 = OpLoad %16 %34
         %36 = OpLoad %25 %g_sAniso
         %37 = OpSampledImage %27 %35 %36
         %38 = OpImageSampleImplicitLod %v4float %37 %31
               OpStore %_entryPointOutput_vColor %38
;CHECK-NOT:         %38 = OpImageSampleImplicitLod %v4float %37 %31
;CHECK-NOT:               OpStore %_entryPointOutput_vColor %38
;CHECK:         %41 = OpUConvert %uint %33
;CHECK:         %43 = OpULessThan %bool %41 %uint_128
;CHECK:               OpSelectionMerge %44 None
;CHECK:               OpBranchConditional %43 %45 %46
;CHECK:         %45 = OpLabel
;CHECK:         %47 = OpLoad %16 %34
;CHECK:         %48 = OpSampledImage %27 %47 %36
;CHECK:        %109 = OpUConvert %uint %33
;CHECK:        %131 = OpFunctionCall %uint %110 %uint_0 %uint_0 %uint_0 %109
;CHECK:        %132 = OpULessThan %bool %uint_0 %131
;CHECK:               OpSelectionMerge %133 None
;CHECK:               OpBranchConditional %132 %134 %135
;CHECK:        %134 = OpLabel
;CHECK:        %136 = OpLoad %16 %34
;CHECK:        %137 = OpSampledImage %27 %136 %36
;CHECK:        %138 = OpImageSampleImplicitLod %v4float %137 %31
;CHECK:               OpBranch %133
;CHECK:        %135 = OpLabel
;CHECK:        %139 = OpUConvert %uint %33
;CHECK:        %140 = OpFunctionCall %void %50 %uint_60 %uint_1 %139 %uint_0
;CHECK:               OpBranch %133
;CHECK:        %133 = OpLabel
;CHECK:        %141 = OpPhi %v4float %138 %134 %106 %135
;CHECK:               OpBranch %44
;CHECK:         %46 = OpLabel
;CHECK:        %105 = OpFunctionCall %void %50 %uint_60 %uint_0 %41 %uint_128
;CHECK:               OpBranch %44
;CHECK:         %44 = OpLabel
;CHECK:        %107 = OpPhi %v4float %141 %133 %106 %46
;CHECK:               OpStore %_entryPointOutput_vColor %107
               OpReturn
               OpFunctionEnd
;CHECK:         %50 = OpFunction %void None %51
;CHECK:         %52 = OpFunctionParameter %uint
;CHECK:         %53 = OpFunctionParameter %uint
;CHECK:         %54 = OpFunctionParameter %uint
;CHECK:         %55 = OpFunctionParameter %uint
;CHECK:         %56 = OpLabel
;CHECK:         %62 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_0
;CHECK:         %65 = OpAtomicIAdd %uint %62 %uint_4 %uint_0 %uint_10
;CHECK:         %66 = OpIAdd %uint %65 %uint_10
;CHECK:         %67 = OpArrayLength %uint %60 1
;CHECK:         %68 = OpULessThanEqual %bool %66 %67
;CHECK:               OpSelectionMerge %69 None
;CHECK:               OpBranchConditional %68 %70 %69
;CHECK:         %70 = OpLabel
;CHECK:         %71 = OpIAdd %uint %65 %uint_0
;CHECK:         %73 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %71
;CHECK:               OpStore %73 %uint_10
;CHECK:         %75 = OpIAdd %uint %65 %uint_1
;CHECK:         %76 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %75
;CHECK:               OpStore %76 %uint_23
;CHECK:         %78 = OpIAdd %uint %65 %uint_2
;CHECK:         %79 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %78
;CHECK:               OpStore %79 %52
;CHECK:         %81 = OpIAdd %uint %65 %uint_3
;CHECK:         %82 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %81
;CHECK:               OpStore %82 %uint_4
;CHECK:         %85 = OpLoad %v4float %gl_FragCoord
;CHECK:         %87 = OpBitcast %v4uint %85
;CHECK:         %88 = OpCompositeExtract %uint %87 0
;CHECK:         %89 = OpIAdd %uint %65 %uint_4
;CHECK:         %90 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %89
;CHECK:               OpStore %90 %88
;CHECK:         %91 = OpCompositeExtract %uint %87 1
;CHECK:         %93 = OpIAdd %uint %65 %uint_5
;CHECK:         %94 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %93
;CHECK:               OpStore %94 %91
;CHECK:         %96 = OpIAdd %uint %65 %uint_7
;CHECK:         %97 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %96
;CHECK:               OpStore %97 %53
;CHECK:         %99 = OpIAdd %uint %65 %uint_8
;CHECK:        %100 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %99
;CHECK:               OpStore %100 %54
;CHECK:        %102 = OpIAdd %uint %65 %uint_9
;CHECK:        %103 = OpAccessChain %_ptr_StorageBuffer_uint %60 %uint_1 %102
;CHECK:               OpStore %103 %55
;CHECK:               OpBranch %69
;CHECK:         %69 = OpLabel
;CHECK:               OpReturn
;CHECK:               OpFunctionEnd
;CHECK:        %110 = OpFunction %uint None %111
;CHECK:        %112 = OpFunctionParameter %uint
;CHECK:        %113 = OpFunctionParameter %uint
;CHECK:        %114 = OpFunctionParameter %uint
;CHECK:        %115 = OpFunctionParameter %uint
;CHECK:        %116 = OpLabel
;CHECK:        %120 = OpAccessChain %_ptr_StorageBuffer_uint %119 %uint_0 %112
;CHECK:        %121 = OpLoad %uint %120
;CHECK:        %122 = OpIAdd %uint %121 %113
;CHECK:        %123 = OpAccessChain %_ptr_StorageBuffer_uint %119 %uint_0 %122
;CHECK:        %124 = OpLoad %uint %123
;CHECK:        %125 = OpIAdd %uint %124 %114
;CHECK:        %126 = OpAccessChain %_ptr_StorageBuffer_uint %119 %uint_0 %125
;CHECK:        %127 = OpLoad %uint %126
;CHECK:        %128 = OpIAdd %uint %127 %115
;CHECK:        %129 = OpAccessChain %_ptr_StorageBuffer_uint %119 %uint_0 %128
;CHECK:        %130 = OpLoad %uint %129
;CHECK:               OpReturnValue %130
;CHECK:               OpFunctionEnd
 )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, true, true,
                                               false, false, true);
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

  const std::string text = R"(
               OpCapability Shader
               OpCapability Int16
               OpCapability StoragePushConstant16
;CHECK:               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor
;CHECK:               OpEntryPoint Fragment %MainPs "MainPs" %_ %__0 %g_tColor %g_sAniso %i_vTextureCoords %_entryPointOutput_vColor %69 %97 %gl_FragCoord
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
;CHECK:               OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:               OpDecorate %_struct_67 Block
;CHECK:               OpMemberDecorate %_struct_67 0 Offset 0
;CHECK:               OpDecorate %69 DescriptorSet 7
;CHECK:               OpDecorate %69 Binding 1
;CHECK:               OpDecorate %_struct_95 Block
;CHECK:               OpMemberDecorate %_struct_95 0 Offset 0
;CHECK:               OpMemberDecorate %_struct_95 1 Offset 4
;CHECK:               OpDecorate %97 DescriptorSet 7
;CHECK:               OpDecorate %97 Binding 0
;CHECK:               OpDecorate %gl_FragCoord BuiltIn FragCoord
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
;CHECK:     %uint_0 = OpConstant %uint 0
;CHECK:    %uint_80 = OpConstant %uint 80
;CHECK:    %uint_64 = OpConstant %uint 64
;CHECK:     %uint_7 = OpConstant %uint 7
;CHECK:     %uint_1 = OpConstant %uint 1
;CHECK:         %61 = OpTypeFunction %uint %uint %uint %uint
;CHECK:%_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK: %_struct_67 = OpTypeStruct %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_67 = OpTypePointer StorageBuffer %_struct_67
;CHECK:         %69 = OpVariable %_ptr_StorageBuffer__struct_67 StorageBuffer
;CHECK:%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:       %bool = OpTypeBool
;CHECK:     %uint_4 = OpConstant %uint 4
;CHECK:         %88 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK: %_struct_95 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_95 = OpTypePointer StorageBuffer %_struct_95
;CHECK:         %97 = OpVariable %_ptr_StorageBuffer__struct_95 StorageBuffer
;CHECK:    %uint_11 = OpConstant %uint 11
;CHECK:    %uint_23 = OpConstant %uint 23
;CHECK:     %uint_2 = OpConstant %uint 2
;CHECK:     %uint_3 = OpConstant %uint 3
;CHECK:%_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK:%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK:     %v4uint = OpTypeVector %uint 4
;CHECK:     %uint_5 = OpConstant %uint 5
;CHECK:     %uint_8 = OpConstant %uint 8
;CHECK:     %uint_9 = OpConstant %uint 9
;CHECK:    %uint_10 = OpConstant %uint 10
;CHECK:    %uint_81 = OpConstant %uint 81
;CHECK:        %142 = OpConstantNull %v2float
     %MainPs = OpFunction %void None %14
         %37 = OpLabel
;CHECK:         %79 = OpFunctionCall %uint %60 %uint_1 %uint_0 %uint_0
;CHECK:               OpBranch %49
;CHECK:         %49 = OpLabel
;CHECK:               OpBranch %48
;CHECK:         %48 = OpLabel
         %38 = OpLoad %v2float %i_vTextureCoords
         %39 = OpAccessChain %_ptr_PushConstant_ushort %__0 %int_0
         %40 = OpLoad %ushort %39
         %41 = OpAccessChain %_ptr_Uniform_v2float %_ %int_0 %40 %int_2
         %42 = OpLoad %v2float %41
         %43 = OpFAdd %v2float %38 %42
;CHECK-NOT:     %42 = OpLoad %v2float %41
;CHECK-NOT:     %43 = OpFAdd %v2float %38 %42
;CHECK:         %52 = OpUConvert %uint %40
;CHECK:         %53 = OpIMul %uint %uint_80 %52
;CHECK:         %54 = OpIAdd %uint %uint_0 %53
;CHECK:         %56 = OpIAdd %uint %54 %uint_64
;CHECK:         %58 = OpIAdd %uint %56 %uint_7
;CHECK:         %81 = OpULessThan %bool %58 %79
;CHECK:               OpSelectionMerge %83 None
;CHECK:               OpBranchConditional %81 %84 %85
;CHECK:         %84 = OpLabel
;CHECK:         %86 = OpLoad %v2float %41
;CHECK:               OpBranch %83
;CHECK:         %85 = OpLabel
;CHECK:        %141 = OpFunctionCall %void %87 %uint_81 %uint_4 %uint_0 %58 %79
;CHECK:               OpBranch %83
;CHECK:         %83 = OpLabel
;CHECK:        %143 = OpPhi %v2float %86 %84 %142 %85
;CHECK:         %43 = OpFAdd %v2float %38 %143
         %44 = OpLoad %30 %g_tColor
         %45 = OpLoad %32 %g_sAniso
         %46 = OpSampledImage %34 %44 %45
         %47 = OpImageSampleImplicitLod %v4float %46 %43
               OpStore %_entryPointOutput_vColor %47
               OpReturn
               OpFunctionEnd
;CHECK:         %60 = OpFunction %uint None %61
;CHECK:         %62 = OpFunctionParameter %uint
;CHECK:         %63 = OpFunctionParameter %uint
;CHECK:         %64 = OpFunctionParameter %uint
;CHECK:         %65 = OpLabel
;CHECK:         %71 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_0 %62
;CHECK:         %72 = OpLoad %uint %71
;CHECK:         %73 = OpIAdd %uint %72 %63
;CHECK:         %74 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_0 %73
;CHECK:         %75 = OpLoad %uint %74
;CHECK:         %76 = OpIAdd %uint %75 %64
;CHECK:         %77 = OpAccessChain %_ptr_StorageBuffer_uint %69 %uint_0 %76
;CHECK:         %78 = OpLoad %uint %77
;CHECK:               OpReturnValue %78
;CHECK:               OpFunctionEnd
;CHECK:         %87 = OpFunction %void None %88
;CHECK:         %89 = OpFunctionParameter %uint
;CHECK:         %90 = OpFunctionParameter %uint
;CHECK:         %91 = OpFunctionParameter %uint
;CHECK:         %92 = OpFunctionParameter %uint
;CHECK:         %93 = OpFunctionParameter %uint
;CHECK:         %94 = OpLabel
;CHECK:         %98 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_0
;CHECK:        %100 = OpAtomicIAdd %uint %98 %uint_4 %uint_0 %uint_11
;CHECK:        %101 = OpIAdd %uint %100 %uint_11
;CHECK:        %102 = OpArrayLength %uint %97 1
;CHECK:        %103 = OpULessThanEqual %bool %101 %102
;CHECK:               OpSelectionMerge %104 None
;CHECK:               OpBranchConditional %103 %105 %104
;CHECK:        %105 = OpLabel
;CHECK:        %106 = OpIAdd %uint %100 %uint_0
;CHECK:        %107 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_1 %106
;CHECK:               OpStore %107 %uint_11
;CHECK:        %109 = OpIAdd %uint %100 %uint_1
;CHECK:        %110 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_1 %109
;CHECK:               OpStore %110 %uint_23
;CHECK:        %112 = OpIAdd %uint %100 %uint_2
;CHECK:        %113 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_1 %112
;CHECK:               OpStore %113 %89
;CHECK:        %115 = OpIAdd %uint %100 %uint_3
;CHECK:        %116 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_1 %115
;CHECK:               OpStore %116 %uint_4
;CHECK:        %119 = OpLoad %v4float %gl_FragCoord
;CHECK:        %121 = OpBitcast %v4uint %119
;CHECK:        %122 = OpCompositeExtract %uint %121 0
;CHECK:        %123 = OpIAdd %uint %100 %uint_4
;CHECK:        %124 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_1 %123
;CHECK:               OpStore %124 %122
;CHECK:        %125 = OpCompositeExtract %uint %121 1
;CHECK:        %127 = OpIAdd %uint %100 %uint_5
;CHECK:        %128 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_1 %127
;CHECK:               OpStore %128 %125
;CHECK:        %129 = OpIAdd %uint %100 %uint_7
;CHECK:        %130 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_1 %129
;CHECK:               OpStore %130 %90
;CHECK:        %132 = OpIAdd %uint %100 %uint_8
;CHECK:        %133 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_1 %132
;CHECK:               OpStore %133 %91
;CHECK:        %135 = OpIAdd %uint %100 %uint_9
;CHECK:        %136 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_1 %135
;CHECK:               OpStore %136 %92
;CHECK:        %138 = OpIAdd %uint %100 %uint_10
;CHECK:        %139 = OpAccessChain %_ptr_StorageBuffer_uint %97 %uint_1 %138
;CHECK:               OpStore %139 %93
;CHECK:               OpBranch %104
;CHECK:        %104 = OpLabel
;CHECK:               OpReturn
;CHECK:               OpFunctionEnd
 )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, false, true);
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

  const std::string text = R"(
               OpCapability Shader
;CHECK:               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position
;CHECK:               OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position %45 %72 %gl_VertexIndex %gl_InstanceIndex
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
;CHECK-NOT:           OpDecorate %21 RelaxedPrecision
;CHECK:               OpDecorate %116 RelaxedPrecision
               OpDecorate %a_position Location 0
;CHECK:               OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:               OpDecorate %_struct_43 Block
;CHECK:               OpMemberDecorate %_struct_43 0 Offset 0
;CHECK:               OpDecorate %45 DescriptorSet 7
;CHECK:               OpDecorate %45 Binding 1
;CHECK:               OpDecorate %61 RelaxedPrecision
;CHECK:               OpDecorate %_struct_70 Block
;CHECK:               OpMemberDecorate %_struct_70 0 Offset 0
;CHECK:               OpMemberDecorate %_struct_70 1 Offset 4
;CHECK:               OpDecorate %72 DescriptorSet 7
;CHECK:               OpDecorate %72 Binding 0
;CHECK:               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
;CHECK:               OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
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
;CHECK;     %uint_0 = OpConstant %uint 0
;CHECK;     %uint_16 = OpConstant %uint 16
;CHECK;     %uint_4 = OpConstant %uint 4
;CHECK;     %uint_3 = OpConstant %uint 3
;CHECK;         %37 = OpTypeFunction %uint %uint %uint %uint
;CHECK;%_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK; %_struct_43 = OpTypeStruct %_runtimearr_uint
;CHECK;%_ptr_StorageBuffer__struct_43 = OpTypePointer StorageBuffer %_struct_43
;CHECK;         %45 = OpVariable %_ptr_StorageBuffer__struct_43 StorageBuffer
;CHECK;%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK;       %bool = OpTypeBool
;CHECK;         %63 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK; %_struct_70 = OpTypeStruct %uint %_runtimearr_uint
;CHECK;%_ptr_StorageBuffer__struct_70 = OpTypePointer StorageBuffer %_struct_70
;CHECK;         %72 = OpVariable %_ptr_StorageBuffer__struct_70 StorageBuffer
;CHECK;    %uint_11 = OpConstant %uint 11
;CHECK;    %uint_23 = OpConstant %uint 23
;CHECK;     %uint_2 = OpConstant %uint 2
;CHECK;%_ptr_Input_uint = OpTypePointer Input %uint
;CHECK;%gl_VertexIndex = OpVariable %_ptr_Input_uint Input
;CHECK;%gl_InstanceIndex = OpVariable %_ptr_Input_uint Input
;CHECK;     %uint_5 = OpConstant %uint 5
;CHECK;     %uint_7 = OpConstant %uint 7
;CHECK;     %uint_8 = OpConstant %uint 8
;CHECK;     %uint_9 = OpConstant %uint 9
;CHECK;    %uint_10 = OpConstant %uint 10
;CHECK;    %uint_45 = OpConstant %uint 45
;CHECK;        %115 = OpConstantNull %float
       %main = OpFunction %void None %3
          %5 = OpLabel
;CHECK:         %55 = OpFunctionCall %uint %36 %uint_1 %uint_0 %uint_0
;CHECK:               OpBranch %26
;CHECK:         %26 = OpLabel
;CHECK:               OpBranch %25
;CHECK:         %25 = OpLabel
         %20 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %int_2 %uint_1
         %21 = OpLoad %float %20
;CHECK-NOT:     %21 = OpLoad %float %20
;CHECK:         %30 = OpIMul %uint %uint_4 %int_2
;CHECK:         %31 = OpIAdd %uint %uint_0 %30
;CHECK:         %32 = OpIMul %uint %uint_16 %uint_1
;CHECK:         %33 = OpIAdd %uint %31 %32
;CHECK:         %35 = OpIAdd %uint %33 %uint_3
;CHECK:         %57 = OpULessThan %bool %35 %55
;CHECK:               OpSelectionMerge %58 None
;CHECK:               OpBranchConditional %57 %59 %60
;CHECK:         %59 = OpLabel
;CHECK:         %61 = OpLoad %float %20
;CHECK:               OpBranch %58
;CHECK:         %60 = OpLabel
;CHECK:        %114 = OpFunctionCall %void %62 %uint_45 %uint_4 %uint_0 %35 %55
;CHECK:               OpBranch %58
;CHECK:         %58 = OpLabel
;CHECK:        %116 = OpPhi %float %61 %59 %115 %60
               OpStore %v_vtxResult %21
;CHECK-NOT:           OpStore %v_vtxResult %21
;CHECK:               OpStore %v_vtxResult %116
               OpReturn
               OpFunctionEnd
;CHECK:         %36 = OpFunction %uint None %37
;CHECK:         %38 = OpFunctionParameter %uint
;CHECK:         %39 = OpFunctionParameter %uint
;CHECK:         %40 = OpFunctionParameter %uint
;CHECK:         %41 = OpLabel
;CHECK:         %47 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_0 %38
;CHECK:         %48 = OpLoad %uint %47
;CHECK:         %49 = OpIAdd %uint %48 %39
;CHECK:         %50 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_0 %49
;CHECK:         %51 = OpLoad %uint %50
;CHECK:         %52 = OpIAdd %uint %51 %40
;CHECK:         %53 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_0 %52
;CHECK:         %54 = OpLoad %uint %53
;CHECK:               OpReturnValue %54
;CHECK:               OpFunctionEnd
;CHECK:         %62 = OpFunction %void None %63
;CHECK:         %64 = OpFunctionParameter %uint
;CHECK:         %65 = OpFunctionParameter %uint
;CHECK:         %66 = OpFunctionParameter %uint
;CHECK:         %67 = OpFunctionParameter %uint
;CHECK:         %68 = OpFunctionParameter %uint
;CHECK:         %69 = OpLabel
;CHECK:         %73 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_0
;CHECK:         %75 = OpAtomicIAdd %uint %73 %uint_4 %uint_0 %uint_11
;CHECK:         %76 = OpIAdd %uint %75 %uint_11
;CHECK:         %77 = OpArrayLength %uint %72 1
;CHECK:         %78 = OpULessThanEqual %bool %76 %77
;CHECK:               OpSelectionMerge %79 None
;CHECK:               OpBranchConditional %78 %80 %79
;CHECK:         %80 = OpLabel
;CHECK:         %81 = OpIAdd %uint %75 %uint_0
;CHECK:         %82 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %81
;CHECK:               OpStore %82 %uint_11
;CHECK:         %84 = OpIAdd %uint %75 %uint_1
;CHECK:         %85 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %84
;CHECK:               OpStore %85 %uint_23
;CHECK:         %87 = OpIAdd %uint %75 %uint_2
;CHECK:         %88 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %87
;CHECK:               OpStore %88 %64
;CHECK:         %89 = OpIAdd %uint %75 %uint_3
;CHECK:         %90 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %89
;CHECK:               OpStore %90 %uint_0
;CHECK:         %93 = OpLoad %uint %gl_VertexIndex
;CHECK:         %94 = OpIAdd %uint %75 %uint_4
;CHECK:         %95 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %94
;CHECK:               OpStore %95 %93
;CHECK:         %97 = OpLoad %uint %gl_InstanceIndex
;CHECK:         %99 = OpIAdd %uint %75 %uint_5
;CHECK:        %100 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %99
;CHECK:               OpStore %100 %97
;CHECK:        %102 = OpIAdd %uint %75 %uint_7
;CHECK:        %103 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %102
;CHECK:               OpStore %103 %65
;CHECK:        %105 = OpIAdd %uint %75 %uint_8
;CHECK:        %106 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %105
;CHECK:               OpStore %106 %66
;CHECK:        %108 = OpIAdd %uint %75 %uint_9
;CHECK:        %109 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %108
;CHECK:               OpStore %109 %67
;CHECK:        %111 = OpIAdd %uint %75 %uint_10
;CHECK:        %112 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %111
;CHECK:               OpStore %112 %68
;CHECK:               OpBranch %79
;CHECK:         %79 = OpLabel
;CHECK:               OpReturn
;CHECK:               OpFunctionEnd
 )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, false, true);
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

  const std::string text = R"(
               OpCapability Shader
;CHECK:               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position
;CHECK:               OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position %45 %72 %gl_VertexIndex %gl_InstanceIndex
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
;CHECK-NOT:           OpDecorate %21 RelaxedPrecision
;CHECK:               OpDecorate %115 RelaxedPrecision
               OpDecorate %a_position Location 0
;CHECK:               OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:               OpDecorate %_struct_43 Block
;CHECK:               OpMemberDecorate %_struct_43 0 Offset 0
;CHECK:               OpDecorate %45 DescriptorSet 7
;CHECK:               OpDecorate %45 Binding 1
;CHECK:               OpDecorate %61 RelaxedPrecision
;CHECK:               OpDecorate %_struct_70 Block
;CHECK:               OpMemberDecorate %_struct_70 0 Offset 0
;CHECK:               OpMemberDecorate %_struct_70 1 Offset 4
;CHECK:               OpDecorate %72 DescriptorSet 7
;CHECK:               OpDecorate %72 Binding 0
;CHECK:               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
;CHECK:               OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
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
;CHECK:     %uint_0 = OpConstant %uint 0
;CHECK:     %uint_8 = OpConstant %uint 8
;CHECK:     %uint_4 = OpConstant %uint 4
;CHECK:     %uint_3 = OpConstant %uint 3
;CHECK:         %37 = OpTypeFunction %uint %uint %uint %uint
;CHECK:%_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK: %_struct_43 = OpTypeStruct %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_43 = OpTypePointer StorageBuffer %_struct_43
;CHECK:         %45 = OpVariable %_ptr_StorageBuffer__struct_43 StorageBuffer
;CHECK:%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:       %bool = OpTypeBool
;CHECK:         %63 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK: %_struct_70 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_70 = OpTypePointer StorageBuffer %_struct_70
;CHECK:         %72 = OpVariable %_ptr_StorageBuffer__struct_70 StorageBuffer
;CHECK:    %uint_11 = OpConstant %uint 11
;CHECK:    %uint_23 = OpConstant %uint 23
;CHECK:     %uint_2 = OpConstant %uint 2
;CHECK:%_ptr_Input_uint = OpTypePointer Input %uint
;CHECK:%gl_VertexIndex = OpVariable %_ptr_Input_uint Input
;CHECK:%gl_InstanceIndex = OpVariable %_ptr_Input_uint Input
;CHECK:     %uint_5 = OpConstant %uint 5
;CHECK:     %uint_7 = OpConstant %uint 7
;CHECK:     %uint_9 = OpConstant %uint 9
;CHECK:    %uint_10 = OpConstant %uint 10
;CHECK:    %uint_45 = OpConstant %uint 45
;CHECK:        %114 = OpConstantNull %float
%main = OpFunction %void None %3
          %5 = OpLabel
;CHECK:         %55 = OpFunctionCall %uint %36 %uint_1 %uint_0 %uint_0
;CHECK:               OpBranch %26
;CHECK:         %26 = OpLabel
;CHECK:               OpBranch %25
;CHECK:         %25 = OpLabel
         %20 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %int_2 %uint_1
         %21 = OpLoad %float %20
;CHECK-NOT:     %21 = OpLoad %float %20
;CHECK:         %29 = OpIMul %uint %uint_8 %int_2
;CHECK:         %30 = OpIAdd %uint %uint_0 %29
;CHECK:         %32 = OpIMul %uint %uint_4 %uint_1
;CHECK:         %33 = OpIAdd %uint %30 %32
;CHECK:         %35 = OpIAdd %uint %33 %uint_3
;CHECK:         %57 = OpULessThan %bool %35 %55
;CHECK:               OpSelectionMerge %58 None
;CHECK:               OpBranchConditional %57 %59 %60
;CHECK:         %59 = OpLabel
;CHECK:         %61 = OpLoad %float %20
;CHECK:               OpBranch %58
;CHECK:         %60 = OpLabel
;CHECK:        %113 = OpFunctionCall %void %62 %uint_45 %uint_4 %uint_0 %35 %55
;CHECK:               OpBranch %58
;CHECK:         %58 = OpLabel
;CHECK:        %115 = OpPhi %float %61 %59 %114 %60
               OpStore %v_vtxResult %21
;CHECK-NOT:           OpStore %v_vtxResult %21
;CHECK:               OpStore %v_vtxResult %115
               OpReturn
               OpFunctionEnd
;CHECK:         %36 = OpFunction %uint None %37
;CHECK:         %38 = OpFunctionParameter %uint
;CHECK:         %39 = OpFunctionParameter %uint
;CHECK:         %40 = OpFunctionParameter %uint
;CHECK:         %41 = OpLabel
;CHECK:         %47 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_0 %38
;CHECK:         %48 = OpLoad %uint %47
;CHECK:         %49 = OpIAdd %uint %48 %39
;CHECK:         %50 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_0 %49
;CHECK:         %51 = OpLoad %uint %50
;CHECK:         %52 = OpIAdd %uint %51 %40
;CHECK:         %53 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_0 %52
;CHECK:         %54 = OpLoad %uint %53
;CHECK:               OpReturnValue %54
;CHECK:               OpFunctionEnd
;CHECK:         %62 = OpFunction %void None %63
;CHECK:         %64 = OpFunctionParameter %uint
;CHECK:         %65 = OpFunctionParameter %uint
;CHECK:         %66 = OpFunctionParameter %uint
;CHECK:         %67 = OpFunctionParameter %uint
;CHECK:         %68 = OpFunctionParameter %uint
;CHECK:         %69 = OpLabel
;CHECK:         %73 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_0
;CHECK:         %75 = OpAtomicIAdd %uint %73 %uint_4 %uint_0 %uint_11
;CHECK:         %76 = OpIAdd %uint %75 %uint_11
;CHECK:         %77 = OpArrayLength %uint %72 1
;CHECK:         %78 = OpULessThanEqual %bool %76 %77
;CHECK:               OpSelectionMerge %79 None
;CHECK:               OpBranchConditional %78 %80 %79
;CHECK:         %80 = OpLabel
;CHECK:         %81 = OpIAdd %uint %75 %uint_0
;CHECK:         %82 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %81
;CHECK:               OpStore %82 %uint_11
;CHECK:         %84 = OpIAdd %uint %75 %uint_1
;CHECK:         %85 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %84
;CHECK:               OpStore %85 %uint_23
;CHECK:         %87 = OpIAdd %uint %75 %uint_2
;CHECK:         %88 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %87
;CHECK:               OpStore %88 %64
;CHECK:         %89 = OpIAdd %uint %75 %uint_3
;CHECK:         %90 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %89
;CHECK:               OpStore %90 %uint_0
;CHECK:         %93 = OpLoad %uint %gl_VertexIndex
;CHECK:         %94 = OpIAdd %uint %75 %uint_4
;CHECK:         %95 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %94
;CHECK:               OpStore %95 %93
;CHECK:         %97 = OpLoad %uint %gl_InstanceIndex
;CHECK:         %99 = OpIAdd %uint %75 %uint_5
;CHECK:        %100 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %99
;CHECK:               OpStore %100 %97
;CHECK:        %102 = OpIAdd %uint %75 %uint_7
;CHECK:        %103 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %102
;CHECK:               OpStore %103 %65
;CHECK:        %104 = OpIAdd %uint %75 %uint_8
;CHECK:        %105 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %104
;CHECK:               OpStore %105 %66
;CHECK:        %107 = OpIAdd %uint %75 %uint_9
;CHECK:        %108 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %107
;CHECK:               OpStore %108 %67
;CHECK:        %110 = OpIAdd %uint %75 %uint_10
;CHECK:        %111 = OpAccessChain %_ptr_StorageBuffer_uint %72 %uint_1 %110
;CHECK:               OpStore %111 %68
;CHECK:               OpBranch %79
;CHECK:         %79 = OpLabel
;CHECK:               OpReturn
;CHECK:               OpFunctionEnd
 )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, false, true);
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
  // layout(set = 0, binding = 0, std430, row_major) uniform Block
  // {
  //    lowp mat2 var[3][4];
  // };
  //
  // void main (void)
  // {
  //    v_vtxResult = var[2][3][1];
  // }

  const std::string text = R"(
               OpCapability Shader
;CHECK:               OpExtension "SPV_KHR_storage_buffer_storage_class"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position
;CHECK:               OpEntryPoint Vertex %main "main" %v_vtxResult %_ %a_position %54 %81 %gl_VertexIndex %gl_InstanceIndex
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
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %26 RelaxedPrecision
;CHECK-NOT:               OpDecorate %26 RelaxedPrecision
;CHECK:               OpDecorate %125 RelaxedPrecision
               OpDecorate %a_position Location 0
;CHECK:               OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:               OpDecorate %_struct_52 Block
;CHECK:               OpMemberDecorate %_struct_52 0 Offset 0
;CHECK:               OpDecorate %54 DescriptorSet 7
;CHECK:               OpDecorate %54 Binding 1
;CHECK:               OpDecorate %70 RelaxedPrecision
;CHECK:               OpDecorate %_struct_79 Block
;CHECK:               OpMemberDecorate %_struct_79 0 Offset 0
;CHECK:               OpMemberDecorate %_struct_79 1 Offset 4
;CHECK:               OpDecorate %81 DescriptorSet 7
;CHECK:               OpDecorate %81 Binding 0
;CHECK:               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
;CHECK:               OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
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
;CHECK:     %uint_0 = OpConstant %uint 0
;CHECK:   %uint_128 = OpConstant %uint 128
;CHECK:    %uint_32 = OpConstant %uint 32
;CHECK:    %uint_16 = OpConstant %uint 16
;CHECK:    %uint_19 = OpConstant %uint 19
;CHECK:     %uint_1 = OpConstant %uint 1
;CHECK:         %46 = OpTypeFunction %uint %uint %uint %uint
;CHECK:%_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK: %_struct_52 = OpTypeStruct %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_52 = OpTypePointer StorageBuffer %_struct_52
;CHECK:         %54 = OpVariable %_ptr_StorageBuffer__struct_52 StorageBuffer
;CHECK:%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:       %bool = OpTypeBool
;CHECK:         %72 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK: %_struct_79 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:%_ptr_StorageBuffer__struct_79 = OpTypePointer StorageBuffer %_struct_79
;CHECK:         %81 = OpVariable %_ptr_StorageBuffer__struct_79 StorageBuffer
;CHECK:    %uint_11 = OpConstant %uint 11
;CHECK:    %uint_23 = OpConstant %uint 23
;CHECK:     %uint_2 = OpConstant %uint 2
;CHECK:%_ptr_Input_uint = OpTypePointer Input %uint
;CHECK:%gl_VertexIndex = OpVariable %_ptr_Input_uint Input
;CHECK:%gl_InstanceIndex = OpVariable %_ptr_Input_uint Input
;CHECK:     %uint_5 = OpConstant %uint 5
;CHECK:     %uint_7 = OpConstant %uint 7
;CHECK:     %uint_8 = OpConstant %uint 8
;CHECK:     %uint_9 = OpConstant %uint 9
;CHECK:    %uint_10 = OpConstant %uint 10
;CHECK:    %uint_51 = OpConstant %uint 51
;CHECK:        %124 = OpConstantNull %v2float
       %main = OpFunction %void None %3
          %5 = OpLabel
;CHECK:         %64 = OpFunctionCall %uint %45 %uint_1 %uint_0 %uint_0
;CHECK:               OpBranch %31
;CHECK:         %31 = OpLabel
;CHECK:               OpBranch %30
;CHECK:         %30 = OpLabel
         %25 = OpAccessChain %_ptr_Uniform_v2float %_ %int_0 %int_2 %int_3 %int_1
         %26 = OpLoad %v2float %25
               OpStore %v_vtxResult %26
;CHECK-NOT:         %26 = OpLoad %v2float %25
;CHECK-NOT:               OpStore %v_vtxResult %26
;CHECK:         %34 = OpIMul %uint %uint_128 %int_2
;CHECK:         %35 = OpIAdd %uint %uint_0 %34
;CHECK:         %37 = OpIMul %uint %uint_32 %int_3
;CHECK:         %38 = OpIAdd %uint %35 %37
;CHECK:         %40 = OpIMul %uint %uint_4 %int_1
;CHECK:         %41 = OpIAdd %uint %38 %40
;CHECK:         %43 = OpIAdd %uint %41 %uint_19
;CHECK:         %66 = OpULessThan %bool %43 %64
;CHECK:               OpSelectionMerge %67 None
;CHECK:               OpBranchConditional %66 %68 %69
;CHECK:         %68 = OpLabel
;CHECK:         %70 = OpLoad %v2float %25
;CHECK:               OpBranch %67
;CHECK:         %69 = OpLabel
;CHECK:        %123 = OpFunctionCall %void %71 %uint_51 %uint_4 %uint_0 %43 %64
;CHECK:               OpBranch %67
;CHECK:         %67 = OpLabel
;CHECK:        %125 = OpPhi %v2float %70 %68 %124 %69
;CHECK:               OpStore %v_vtxResult %125
               OpReturn
               OpFunctionEnd
;CHECK:         %45 = OpFunction %uint None %46
;CHECK:         %47 = OpFunctionParameter %uint
;CHECK:         %48 = OpFunctionParameter %uint
;CHECK:         %49 = OpFunctionParameter %uint
;CHECK:         %50 = OpLabel
;CHECK:         %56 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_0 %47
;CHECK:         %57 = OpLoad %uint %56
;CHECK:         %58 = OpIAdd %uint %57 %48
;CHECK:         %59 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_0 %58
;CHECK:         %60 = OpLoad %uint %59
;CHECK:         %61 = OpIAdd %uint %60 %49
;CHECK:         %62 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_0 %61
;CHECK:         %63 = OpLoad %uint %62
;CHECK:               OpReturnValue %63
;CHECK:               OpFunctionEnd
;CHECK:         %71 = OpFunction %void None %72
;CHECK:         %73 = OpFunctionParameter %uint
;CHECK:         %74 = OpFunctionParameter %uint
;CHECK:         %75 = OpFunctionParameter %uint
;CHECK:         %76 = OpFunctionParameter %uint
;CHECK:         %77 = OpFunctionParameter %uint
;CHECK:         %78 = OpLabel
;CHECK:         %82 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_0
;CHECK:         %84 = OpAtomicIAdd %uint %82 %uint_4 %uint_0 %uint_11
;CHECK:         %85 = OpIAdd %uint %84 %uint_11
;CHECK:         %86 = OpArrayLength %uint %81 1
;CHECK:         %87 = OpULessThanEqual %bool %85 %86
;CHECK:               OpSelectionMerge %88 None
;CHECK:               OpBranchConditional %87 %89 %88
;CHECK:         %89 = OpLabel
;CHECK:         %90 = OpIAdd %uint %84 %uint_0
;CHECK:         %91 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_1 %90
;CHECK:               OpStore %91 %uint_11
;CHECK:         %93 = OpIAdd %uint %84 %uint_1
;CHECK:         %94 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_1 %93
;CHECK:               OpStore %94 %uint_23
;CHECK:         %96 = OpIAdd %uint %84 %uint_2
;CHECK:         %97 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_1 %96
;CHECK:               OpStore %97 %73
;CHECK:         %98 = OpIAdd %uint %84 %uint_3
;CHECK:         %99 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_1 %98
;CHECK:               OpStore %99 %uint_0
;CHECK:        %102 = OpLoad %uint %gl_VertexIndex
;CHECK:        %103 = OpIAdd %uint %84 %uint_4
;CHECK:        %104 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_1 %103
;CHECK:               OpStore %104 %102
;CHECK:        %106 = OpLoad %uint %gl_InstanceIndex
;CHECK:        %108 = OpIAdd %uint %84 %uint_5
;CHECK:        %109 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_1 %108
;CHECK:               OpStore %109 %106
;CHECK:        %111 = OpIAdd %uint %84 %uint_7
;CHECK:        %112 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_1 %111
;CHECK:               OpStore %112 %74
;CHECK:        %114 = OpIAdd %uint %84 %uint_8
;CHECK:        %115 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_1 %114
;CHECK:               OpStore %115 %75
;CHECK:        %117 = OpIAdd %uint %84 %uint_9
;CHECK:        %118 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_1 %117
;CHECK:               OpStore %118 %76
;CHECK:        %120 = OpIAdd %uint %84 %uint_10
;CHECK:        %121 = OpAccessChain %_ptr_StorageBuffer_uint %81 %uint_1 %120
;CHECK:               OpStore %121 %77
;CHECK:               OpBranch %88
;CHECK:         %88 = OpLabel
;CHECK:               OpReturn
;CHECK:               OpFunctionEnd
 )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, false, true);
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

  const std::string text = R"(
                          OpCapability Shader
                          OpCapability ImageBuffer
;CHECK:                   OpCapability ImageQuery
;CHECK:                   OpExtension "SPV_KHR_storage_buffer_storage_class"
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
;CHECK:                   OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:                   OpDecorate %_struct_43 Block
;CHECK:                   OpMemberDecorate %_struct_43 0 Offset 0
;CHECK:                   OpMemberDecorate %_struct_43 1 Offset 4
;CHECK:                   OpDecorate %45 DescriptorSet 7
;CHECK:                   OpDecorate %45 Binding 0
;CHECK:                   OpDecorate %gl_FragCoord BuiltIn FragCoord
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
;CHECK:           %uint = OpTypeInt 32 0
;CHECK:         %uint_0 = OpConstant %uint 0
;CHECK:           %bool = OpTypeBool
;CHECK:         %uint_7 = OpConstant %uint 7
;CHECK:             %35 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK:    %_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK:     %_struct_43 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:    %_ptr_StorageBuffer__struct_43 = OpTypePointer StorageBuffer %_struct_43
;CHECK:             %45 = OpVariable %_ptr_StorageBuffer__struct_43 StorageBuffer
;CHECK:    %_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:        %uint_11 = OpConstant %uint 11
;CHECK:         %uint_4 = OpConstant %uint 4
;CHECK:         %uint_1 = OpConstant %uint 1
;CHECK:        %uint_23 = OpConstant %uint 23
;CHECK:         %uint_2 = OpConstant %uint 2
;CHECK:         %uint_3 = OpConstant %uint 3
;CHECK:    %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK:    %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK:         %v4uint = OpTypeVector %uint 4
;CHECK:         %uint_5 = OpConstant %uint 5
;CHECK:         %uint_8 = OpConstant %uint 8
;CHECK:         %uint_9 = OpConstant %uint 9
;CHECK:        %uint_10 = OpConstant %uint 10
;CHECK:        %uint_33 = OpConstant %uint 33
;CHECK:             %93 = OpConstantNull %v4float
                  %main = OpFunction %void None %3
                     %5 = OpLabel
;CHECK:                   OpBranch %21
;CHECK:             %21 = OpLabel
;CHECK:                   OpBranch %20
;CHECK:             %20 = OpLabel
;CHECK:                   OpBranch %19
;CHECK:             %19 = OpLabel
                    %13 = OpLoad %10 %s
                    %17 = OpLoad %int %ii
                    %18 = OpImageRead %v4float %13 %17
                          OpStore %x %18
;CHECK-NOT:         %18 = OpImageRead %v4float %13 %17
;CHECK-NOT:               OpStore %x %18
;CHECK:             %23 = OpBitcast %uint %17
;CHECK:             %25 = OpImageQuerySize %uint %13
;CHECK:             %27 = OpULessThan %bool %23 %25
;CHECK:                   OpSelectionMerge %29 None
;CHECK:                   OpBranchConditional %27 %30 %31
;CHECK:             %30 = OpLabel
;CHECK:             %32 = OpLoad %10 %s
;CHECK:             %33 = OpImageRead %v4float %32 %17
;CHECK:                   OpBranch %29
;CHECK:             %31 = OpLabel
;CHECK:             %92 = OpFunctionCall %void %34 %uint_33 %uint_7 %uint_0 %23 %25
;CHECK:                   OpBranch %29
;CHECK:             %29 = OpLabel
;CHECK:             %94 = OpPhi %v4float %33 %30 %93 %31
;CHECK:                   OpStore %x %94
                          OpReturn
                          OpFunctionEnd
;CHECK:             %34 = OpFunction %void None %35
;CHECK:             %36 = OpFunctionParameter %uint
;CHECK:             %37 = OpFunctionParameter %uint
;CHECK:             %38 = OpFunctionParameter %uint
;CHECK:             %39 = OpFunctionParameter %uint
;CHECK:             %40 = OpFunctionParameter %uint
;CHECK:             %41 = OpLabel
;CHECK:             %47 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_0
;CHECK:             %50 = OpAtomicIAdd %uint %47 %uint_4 %uint_0 %uint_11
;CHECK:             %51 = OpIAdd %uint %50 %uint_11
;CHECK:             %52 = OpArrayLength %uint %45 1
;CHECK:             %53 = OpULessThanEqual %bool %51 %52
;CHECK:                   OpSelectionMerge %54 None
;CHECK:                   OpBranchConditional %53 %55 %54
;CHECK:             %55 = OpLabel
;CHECK:             %56 = OpIAdd %uint %50 %uint_0
;CHECK:             %58 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %56
;CHECK:                   OpStore %58 %uint_11
;CHECK:             %60 = OpIAdd %uint %50 %uint_1
;CHECK:             %61 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %60
;CHECK:                   OpStore %61 %uint_23
;CHECK:             %63 = OpIAdd %uint %50 %uint_2
;CHECK:             %64 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %63
;CHECK:                   OpStore %64 %36
;CHECK:             %66 = OpIAdd %uint %50 %uint_3
;CHECK:             %67 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %66
;CHECK:                   OpStore %67 %uint_4
;CHECK:             %70 = OpLoad %v4float %gl_FragCoord
;CHECK:             %72 = OpBitcast %v4uint %70
;CHECK:             %73 = OpCompositeExtract %uint %72 0
;CHECK:             %74 = OpIAdd %uint %50 %uint_4
;CHECK:             %75 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %74
;CHECK:                   OpStore %75 %73
;CHECK:             %76 = OpCompositeExtract %uint %72 1
;CHECK:             %78 = OpIAdd %uint %50 %uint_5
;CHECK:             %79 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %78
;CHECK:                   OpStore %79 %76
;CHECK:             %80 = OpIAdd %uint %50 %uint_7
;CHECK:             %81 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %80
;CHECK:                   OpStore %81 %37
;CHECK:             %83 = OpIAdd %uint %50 %uint_8
;CHECK:             %84 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %83
;CHECK:                   OpStore %84 %38
;CHECK:             %86 = OpIAdd %uint %50 %uint_9
;CHECK:             %87 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %86
;CHECK:                   OpStore %87 %39
;CHECK:             %89 = OpIAdd %uint %50 %uint_10
;CHECK:             %90 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %89
;CHECK:                   OpStore %90 %40
;CHECK:                   OpBranch %54
;CHECK:             %54 = OpLabel
;CHECK:                   OpReturn
;CHECK:                   OpFunctionEnd
  )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, true, true);
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

  const std::string text = R"(
                          OpCapability Shader
                          OpCapability ImageBuffer
;CHECK:                   OpCapability ImageQuery
;CHECK:                   OpExtension "SPV_KHR_storage_buffer_storage_class"
                     %1 = OpExtInstImport "GLSL.std.450"
                          OpMemoryModel Logical GLSL450
                          OpEntryPoint Fragment %main "main" %s %ii %x
;CHECK:                   OpEntryPoint Fragment %main "main" %s %ii %x %44 %gl_FragCoord
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
;CHECK:                   OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:                   OpDecorate %_struct_42 Block
;CHECK:                   OpMemberDecorate %_struct_42 0 Offset 0
;CHECK:                   OpMemberDecorate %_struct_42 1 Offset 4
;CHECK:                   OpDecorate %44 DescriptorSet 7
;CHECK:                   OpDecorate %44 Binding 0
;CHECK:                   OpDecorate %gl_FragCoord BuiltIn FragCoord
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
;CHECK:           %uint = OpTypeInt 32 0
;CHECK:         %uint_0 = OpConstant %uint 0
;CHECK:           %bool = OpTypeBool
;CHECK:         %uint_7 = OpConstant %uint 7
;CHECK:             %34 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK:    %_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK:     %_struct_42 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:    %_ptr_StorageBuffer__struct_42 = OpTypePointer StorageBuffer %_struct_42
;CHECK:             %44 = OpVariable %_ptr_StorageBuffer__struct_42 StorageBuffer
;CHECK:    %_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:        %uint_11 = OpConstant %uint 11
;CHECK:         %uint_4 = OpConstant %uint 4
;CHECK:         %uint_1 = OpConstant %uint 1
;CHECK:        %uint_23 = OpConstant %uint 23
;CHECK:         %uint_2 = OpConstant %uint 2
;CHECK:         %uint_3 = OpConstant %uint 3
;CHECK:    %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK:    %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK:         %v4uint = OpTypeVector %uint 4
;CHECK:         %uint_5 = OpConstant %uint 5
;CHECK:         %uint_8 = OpConstant %uint 8
;CHECK:         %uint_9 = OpConstant %uint 9
;CHECK:        %uint_10 = OpConstant %uint 10
;CHECK:        %uint_34 = OpConstant %uint 34
                  %main = OpFunction %void None %3
                     %5 = OpLabel
;CHECK:                   OpBranch %21
;CHECK:             %21 = OpLabel
;CHECK:                   OpBranch %20
;CHECK:             %20 = OpLabel
;CHECK:                   OpBranch %19
;CHECK:             %19 = OpLabel
                    %10 = OpLoad %7 %s
                    %14 = OpLoad %int %ii
                    %18 = OpLoad %v4float %x
                          OpImageWrite %10 %14 %18
;CHECK-NOT:               OpImageWrite %10 %14 %18
;CHECK:             %23 = OpBitcast %uint %14
;CHECK:             %25 = OpImageQuerySize %uint %10
;CHECK:             %27 = OpULessThan %bool %23 %25
;CHECK:                   OpSelectionMerge %29 None
;CHECK:                   OpBranchConditional %27 %30 %31
;CHECK:             %30 = OpLabel
;CHECK:             %32 = OpLoad %7 %s
;CHECK:                   OpImageWrite %32 %14 %18
;CHECK:                   OpBranch %29
;CHECK:             %31 = OpLabel
;CHECK:             %91 = OpFunctionCall %void %33 %uint_34 %uint_7 %uint_0 %23 %25
;CHECK:                   OpBranch %29
;CHECK:             %29 = OpLabel
                          OpReturn
                          OpFunctionEnd
;CHECK:             %33 = OpFunction %void None %34
;CHECK:             %35 = OpFunctionParameter %uint
;CHECK:             %36 = OpFunctionParameter %uint
;CHECK:             %37 = OpFunctionParameter %uint
;CHECK:             %38 = OpFunctionParameter %uint
;CHECK:             %39 = OpFunctionParameter %uint
;CHECK:             %40 = OpLabel
;CHECK:             %46 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_0
;CHECK:             %49 = OpAtomicIAdd %uint %46 %uint_4 %uint_0 %uint_11
;CHECK:             %50 = OpIAdd %uint %49 %uint_11
;CHECK:             %51 = OpArrayLength %uint %44 1
;CHECK:             %52 = OpULessThanEqual %bool %50 %51
;CHECK:                   OpSelectionMerge %53 None
;CHECK:                   OpBranchConditional %52 %54 %53
;CHECK:             %54 = OpLabel
;CHECK:             %55 = OpIAdd %uint %49 %uint_0
;CHECK:             %57 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_1 %55
;CHECK:                   OpStore %57 %uint_11
;CHECK:             %59 = OpIAdd %uint %49 %uint_1
;CHECK:             %60 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_1 %59
;CHECK:                   OpStore %60 %uint_23
;CHECK:             %62 = OpIAdd %uint %49 %uint_2
;CHECK:             %63 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_1 %62
;CHECK:                   OpStore %63 %35
;CHECK:             %65 = OpIAdd %uint %49 %uint_3
;CHECK:             %66 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_1 %65
;CHECK:                   OpStore %66 %uint_4
;CHECK:             %69 = OpLoad %v4float %gl_FragCoord
;CHECK:             %71 = OpBitcast %v4uint %69
;CHECK:             %72 = OpCompositeExtract %uint %71 0
;CHECK:             %73 = OpIAdd %uint %49 %uint_4
;CHECK:             %74 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_1 %73
;CHECK:                   OpStore %74 %72
;CHECK:             %75 = OpCompositeExtract %uint %71 1
;CHECK:             %77 = OpIAdd %uint %49 %uint_5
;CHECK:             %78 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_1 %77
;CHECK:                   OpStore %78 %75
;CHECK:             %79 = OpIAdd %uint %49 %uint_7
;CHECK:             %80 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_1 %79
;CHECK:                   OpStore %80 %36
;CHECK:             %82 = OpIAdd %uint %49 %uint_8
;CHECK:             %83 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_1 %82
;CHECK:                   OpStore %83 %37
;CHECK:             %85 = OpIAdd %uint %49 %uint_9
;CHECK:             %86 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_1 %85
;CHECK:                   OpStore %86 %38
;CHECK:             %88 = OpIAdd %uint %49 %uint_10
;CHECK:             %89 = OpAccessChain %_ptr_StorageBuffer_uint %44 %uint_1 %88
;CHECK:                   OpStore %89 %39
;CHECK:                   OpBranch %53
;CHECK:             %53 = OpLabel
;CHECK:                   OpReturn
;CHECK:                   OpFunctionEnd
  )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, true, true);
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

  const std::string text = R"(
                          OpCapability Shader
                          OpCapability SampledBuffer
;CHECK:                   OpCapability ImageQuery
;CHECK:                   OpExtension "SPV_KHR_storage_buffer_storage_class"
                     %1 = OpExtInstImport "GLSL.std.450"
                          OpMemoryModel Logical GLSL450
                          OpEntryPoint Fragment %main "main" %x %s %ii
;CHECK:                   OpEntryPoint Fragment %main "main" %x %s %ii %45 %gl_FragCoord
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
;CHECK:                   OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:                   OpDecorate %_struct_43 Block
;CHECK:                   OpMemberDecorate %_struct_43 0 Offset 0
;CHECK:                   OpMemberDecorate %_struct_43 1 Offset 4
;CHECK:                   OpDecorate %45 DescriptorSet 7
;CHECK:                   OpDecorate %45 Binding 0
;CHECK:                   OpDecorate %gl_FragCoord BuiltIn FragCoord
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
;CHECK:           %uint = OpTypeInt 32 0
;CHECK:         %uint_0 = OpConstant %uint 0
;CHECK:           %bool = OpTypeBool
;CHECK:         %uint_6 = OpConstant %uint 6
;CHECK:             %35 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK:    %_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK:     %_struct_43 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:    %_ptr_StorageBuffer__struct_43 = OpTypePointer StorageBuffer %_struct_43
;CHECK:             %45 = OpVariable %_ptr_StorageBuffer__struct_43 StorageBuffer
;CHECK:    %_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:        %uint_11 = OpConstant %uint 11
;CHECK:         %uint_4 = OpConstant %uint 4
;CHECK:         %uint_1 = OpConstant %uint 1
;CHECK:        %uint_23 = OpConstant %uint 23
;CHECK:         %uint_2 = OpConstant %uint 2
;CHECK:         %uint_3 = OpConstant %uint 3
;CHECK:    %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK:    %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK:         %v4uint = OpTypeVector %uint 4
;CHECK:         %uint_5 = OpConstant %uint 5
;CHECK:         %uint_7 = OpConstant %uint 7
;CHECK:         %uint_8 = OpConstant %uint 8
;CHECK:         %uint_9 = OpConstant %uint 9
;CHECK:        %uint_10 = OpConstant %uint 10
;CHECK:        %uint_32 = OpConstant %uint 32
;CHECK:             %94 = OpConstantNull %v4float
                  %main = OpFunction %void None %3
                     %5 = OpLabel
;CHECK:                   OpBranch %21
;CHECK:             %21 = OpLabel
;CHECK:                   OpBranch %20
;CHECK:             %20 = OpLabel
;CHECK:                   OpBranch %19
;CHECK:             %19 = OpLabel
                    %13 = OpLoad %10 %s
                    %17 = OpLoad %int %ii
                    %18 = OpImageFetch %v4float %13 %17
                          OpStore %x %18
;CHECK-NOT:         %18 = OpImageFetch %v4float %13 %17
;CHECK-NOT:               OpStore %x %18
;CHECK:             %23 = OpBitcast %uint %17
;CHECK:             %25 = OpImageQuerySize %uint %13
;CHECK:             %27 = OpULessThan %bool %23 %25
;CHECK:                   OpSelectionMerge %29 None
;CHECK:                   OpBranchConditional %27 %30 %31
;CHECK:             %30 = OpLabel
;CHECK:             %32 = OpLoad %10 %s
;CHECK:             %33 = OpImageFetch %v4float %32 %17
;CHECK:                   OpBranch %29
;CHECK:             %31 = OpLabel
;CHECK:             %93 = OpFunctionCall %void %34 %uint_32 %uint_6 %uint_0 %23 %25
;CHECK:                   OpBranch %29
;CHECK:             %29 = OpLabel
;CHECK:             %95 = OpPhi %v4float %33 %30 %94 %31
;CHECK:                   OpStore %x %95
                          OpReturn
                          OpFunctionEnd
;CHECK:             %34 = OpFunction %void None %35
;CHECK:             %36 = OpFunctionParameter %uint
;CHECK:             %37 = OpFunctionParameter %uint
;CHECK:             %38 = OpFunctionParameter %uint
;CHECK:             %39 = OpFunctionParameter %uint
;CHECK:             %40 = OpFunctionParameter %uint
;CHECK:             %41 = OpLabel
;CHECK:             %47 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_0
;CHECK:             %50 = OpAtomicIAdd %uint %47 %uint_4 %uint_0 %uint_11
;CHECK:             %51 = OpIAdd %uint %50 %uint_11
;CHECK:             %52 = OpArrayLength %uint %45 1
;CHECK:             %53 = OpULessThanEqual %bool %51 %52
;CHECK:                   OpSelectionMerge %54 None
;CHECK:                   OpBranchConditional %53 %55 %54
;CHECK:             %55 = OpLabel
;CHECK:             %56 = OpIAdd %uint %50 %uint_0
;CHECK:             %58 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %56
;CHECK:                   OpStore %58 %uint_11
;CHECK:             %60 = OpIAdd %uint %50 %uint_1
;CHECK:             %61 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %60
;CHECK:                   OpStore %61 %uint_23
;CHECK:             %63 = OpIAdd %uint %50 %uint_2
;CHECK:             %64 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %63
;CHECK:                   OpStore %64 %36
;CHECK:             %66 = OpIAdd %uint %50 %uint_3
;CHECK:             %67 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %66
;CHECK:                   OpStore %67 %uint_4
;CHECK:             %70 = OpLoad %v4float %gl_FragCoord
;CHECK:             %72 = OpBitcast %v4uint %70
;CHECK:             %73 = OpCompositeExtract %uint %72 0
;CHECK:             %74 = OpIAdd %uint %50 %uint_4
;CHECK:             %75 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %74
;CHECK:                   OpStore %75 %73
;CHECK:             %76 = OpCompositeExtract %uint %72 1
;CHECK:             %78 = OpIAdd %uint %50 %uint_5
;CHECK:             %79 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %78
;CHECK:                   OpStore %79 %76
;CHECK:             %81 = OpIAdd %uint %50 %uint_7
;CHECK:             %82 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %81
;CHECK:                   OpStore %82 %37
;CHECK:             %84 = OpIAdd %uint %50 %uint_8
;CHECK:             %85 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %84
;CHECK:                   OpStore %85 %38
;CHECK:             %87 = OpIAdd %uint %50 %uint_9
;CHECK:             %88 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %87
;CHECK:                   OpStore %88 %39
;CHECK:             %90 = OpIAdd %uint %50 %uint_10
;CHECK:             %91 = OpAccessChain %_ptr_StorageBuffer_uint %45 %uint_1 %90
;CHECK:                   OpStore %91 %40
;CHECK:                   OpBranch %54
;CHECK:             %54 = OpLabel
;CHECK:                   OpReturn
;CHECK:                   OpFunctionEnd
  )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, true, true);
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

  const std::string text = R"(
                          OpCapability Shader
                          OpCapability SampledBuffer
;CHECK:                   OpCapability ImageQuery
;CHECK:                   OpExtension "SPV_KHR_storage_buffer_storage_class"
                     %1 = OpExtInstImport "GLSL.std.450"
                          OpMemoryModel Logical GLSL450
                          OpEntryPoint Fragment %main "main" %x %s %ii
;CHECK:                   OpEntryPoint Fragment %main "main" %x %s %ii %48 %gl_FragCoord
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
;CHECK:                   OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:                   OpDecorate %_struct_46 Block
;CHECK:                   OpMemberDecorate %_struct_46 0 Offset 0
;CHECK:                   OpMemberDecorate %_struct_46 1 Offset 4
;CHECK:                   OpDecorate %48 DescriptorSet 7
;CHECK:                   OpDecorate %48 Binding 0
;CHECK:                   OpDecorate %gl_FragCoord BuiltIn FragCoord
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
;CHECK:           %uint = OpTypeInt 32 0
;CHECK:         %uint_0 = OpConstant %uint 0
;CHECK:           %bool = OpTypeBool
;CHECK:         %uint_6 = OpConstant %uint 6
;CHECK:             %38 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK:    %_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK:     %_struct_46 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:    %_ptr_StorageBuffer__struct_46 = OpTypePointer StorageBuffer %_struct_46
;CHECK:             %48 = OpVariable %_ptr_StorageBuffer__struct_46 StorageBuffer
;CHECK:    %_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:        %uint_11 = OpConstant %uint 11
;CHECK:         %uint_4 = OpConstant %uint 4
;CHECK:         %uint_1 = OpConstant %uint 1
;CHECK:        %uint_23 = OpConstant %uint 23
;CHECK:         %uint_2 = OpConstant %uint 2
;CHECK:         %uint_3 = OpConstant %uint 3
;CHECK:    %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK:    %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK:         %v4uint = OpTypeVector %uint 4
;CHECK:         %uint_5 = OpConstant %uint 5
;CHECK:         %uint_7 = OpConstant %uint 7
;CHECK:         %uint_8 = OpConstant %uint 8
;CHECK:         %uint_9 = OpConstant %uint 9
;CHECK:        %uint_10 = OpConstant %uint 10
;CHECK:        %uint_34 = OpConstant %uint 34
;CHECK:             %97 = OpConstantNull %v4float
                  %main = OpFunction %void None %3
                     %5 = OpLabel
;CHECK:                   OpBranch %23
;CHECK:             %23 = OpLabel
;CHECK:                   OpBranch %22
;CHECK:             %22 = OpLabel
;CHECK:                   OpBranch %21
;CHECK:             %21 = OpLabel
                    %14 = OpLoad %11 %s
                    %18 = OpLoad %int %ii
                    %19 = OpImage %10 %14
                    %20 = OpImageFetch %v4float %19 %18
                          OpStore %x %20
;CHECK-NOT:         %20 = OpImageFetch %v4float %19 %18
;CHECK-NOT:               OpStore %x %20
;CHECK:             %25 = OpBitcast %uint %18
;CHECK:             %27 = OpImageQuerySize %uint %19
;CHECK:             %29 = OpULessThan %bool %25 %27
;CHECK:                   OpSelectionMerge %31 None
;CHECK:                   OpBranchConditional %29 %32 %33
;CHECK:             %32 = OpLabel
;CHECK:             %34 = OpLoad %11 %s
;CHECK:             %35 = OpImage %10 %34
;CHECK:             %36 = OpImageFetch %v4float %35 %18
;CHECK:                   OpBranch %31
;CHECK:             %33 = OpLabel
;CHECK:             %96 = OpFunctionCall %void %37 %uint_34 %uint_6 %uint_0 %25 %27
;CHECK:                   OpBranch %31
;CHECK:             %31 = OpLabel
;CHECK:             %98 = OpPhi %v4float %36 %32 %97 %33
;CHECK:                   OpStore %x %98
                          OpReturn
                          OpFunctionEnd
;CHECK:             %37 = OpFunction %void None %38
;CHECK:             %39 = OpFunctionParameter %uint
;CHECK:             %40 = OpFunctionParameter %uint
;CHECK:             %41 = OpFunctionParameter %uint
;CHECK:             %42 = OpFunctionParameter %uint
;CHECK:             %43 = OpFunctionParameter %uint
;CHECK:             %44 = OpLabel
;CHECK:             %50 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_0
;CHECK:             %53 = OpAtomicIAdd %uint %50 %uint_4 %uint_0 %uint_11
;CHECK:             %54 = OpIAdd %uint %53 %uint_11
;CHECK:             %55 = OpArrayLength %uint %48 1
;CHECK:             %56 = OpULessThanEqual %bool %54 %55
;CHECK:                   OpSelectionMerge %57 None
;CHECK:                   OpBranchConditional %56 %58 %57
;CHECK:             %58 = OpLabel
;CHECK:             %59 = OpIAdd %uint %53 %uint_0
;CHECK:             %61 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_1 %59
;CHECK:                   OpStore %61 %uint_11
;CHECK:             %63 = OpIAdd %uint %53 %uint_1
;CHECK:             %64 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_1 %63
;CHECK:                   OpStore %64 %uint_23
;CHECK:             %66 = OpIAdd %uint %53 %uint_2
;CHECK:             %67 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_1 %66
;CHECK:                   OpStore %67 %39
;CHECK:             %69 = OpIAdd %uint %53 %uint_3
;CHECK:             %70 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_1 %69
;CHECK:                   OpStore %70 %uint_4
;CHECK:             %73 = OpLoad %v4float %gl_FragCoord
;CHECK:             %75 = OpBitcast %v4uint %73
;CHECK:             %76 = OpCompositeExtract %uint %75 0
;CHECK:             %77 = OpIAdd %uint %53 %uint_4
;CHECK:             %78 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_1 %77
;CHECK:                   OpStore %78 %76
;CHECK:             %79 = OpCompositeExtract %uint %75 1
;CHECK:             %81 = OpIAdd %uint %53 %uint_5
;CHECK:             %82 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_1 %81
;CHECK:                   OpStore %82 %79
;CHECK:             %84 = OpIAdd %uint %53 %uint_7
;CHECK:             %85 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_1 %84
;CHECK:                   OpStore %85 %40
;CHECK:             %87 = OpIAdd %uint %53 %uint_8
;CHECK:             %88 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_1 %87
;CHECK:                   OpStore %88 %41
;CHECK:             %90 = OpIAdd %uint %53 %uint_9
;CHECK:             %91 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_1 %90
;CHECK:                   OpStore %91 %42
;CHECK:             %93 = OpIAdd %uint %53 %uint_10
;CHECK:             %94 = OpAccessChain %_ptr_StorageBuffer_uint %48 %uint_1 %93
;CHECK:                   OpStore %94 %43
;CHECK:                   OpBranch %57
;CHECK:             %57 = OpLabel
;CHECK:                   OpReturn
;CHECK:                   OpFunctionEnd
  )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, true, true);
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

  const std::string text = R"(
                          OpCapability Shader
                          OpCapability SampledBuffer
;CHECK:                   OpCapability ImageQuery
;CHECK:                   OpExtension "SPV_KHR_storage_buffer_storage_class"
                     %1 = OpExtInstImport "GLSL.std.450"
                          OpMemoryModel Logical GLSL450
                          OpEntryPoint Fragment %main "main" %x %tBuf %s %ii
;CHECK:                   OpEntryPoint Fragment %main "main" %x %tBuf %s %ii %54 %gl_FragCoord
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
;CHECK:                   OpDecorate %_runtimearr_uint ArrayStride 4
;CHECK:                   OpDecorate %_struct_52 Block
;CHECK:                   OpMemberDecorate %_struct_52 0 Offset 0
;CHECK:                   OpMemberDecorate %_struct_52 1 Offset 4
;CHECK:                   OpDecorate %54 DescriptorSet 7
;CHECK:                   OpDecorate %54 Binding 0
;CHECK:                   OpDecorate %gl_FragCoord BuiltIn FragCoord
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
;CHECK:           %uint = OpTypeInt 32 0
;CHECK:         %uint_0 = OpConstant %uint 0
;CHECK:           %bool = OpTypeBool
;CHECK:         %uint_6 = OpConstant %uint 6
;CHECK:             %44 = OpTypeFunction %void %uint %uint %uint %uint %uint
;CHECK:    %_runtimearr_uint = OpTypeRuntimeArray %uint
;CHECK:     %_struct_52 = OpTypeStruct %uint %_runtimearr_uint
;CHECK:    %_ptr_StorageBuffer__struct_52 = OpTypePointer StorageBuffer %_struct_52
;CHECK:             %54 = OpVariable %_ptr_StorageBuffer__struct_52 StorageBuffer
;CHECK:    %_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
;CHECK:        %uint_11 = OpConstant %uint 11
;CHECK:         %uint_4 = OpConstant %uint 4
;CHECK:         %uint_1 = OpConstant %uint 1
;CHECK:        %uint_23 = OpConstant %uint 23
;CHECK:         %uint_2 = OpConstant %uint 2
;CHECK:         %uint_3 = OpConstant %uint 3
;CHECK:    %_ptr_Input_v4float = OpTypePointer Input %v4float
;CHECK:    %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
;CHECK:         %v4uint = OpTypeVector %uint 4
;CHECK:         %uint_5 = OpConstant %uint 5
;CHECK:         %uint_7 = OpConstant %uint 7
;CHECK:         %uint_8 = OpConstant %uint 8
;CHECK:         %uint_9 = OpConstant %uint 9
;CHECK:        %uint_10 = OpConstant %uint 10
;CHECK:        %uint_42 = OpConstant %uint 42
;CHECK:            %103 = OpConstantNull %v4float
                  %main = OpFunction %void None %3
                     %5 = OpLabel
;CHECK:                   OpBranch %28
;CHECK:             %28 = OpLabel
;CHECK:                   OpBranch %27
;CHECK:             %27 = OpLabel
;CHECK:                   OpBranch %26
;CHECK:             %26 = OpLabel
                    %13 = OpLoad %10 %tBuf
                    %17 = OpLoad %14 %s
                    %19 = OpSampledImage %18 %13 %17
                    %23 = OpLoad %int %ii
                    %24 = OpImage %10 %19
                    %25 = OpImageFetch %v4float %24 %23
                          OpStore %x %25
;CHECK-NOT:         %25 = OpImageFetch %v4float %24 %23
;CHECK-NOT:               OpStore %x %25
;CHECK:             %30 = OpBitcast %uint %23
;CHECK:             %32 = OpImageQuerySize %uint %24
;CHECK:             %34 = OpULessThan %bool %30 %32
;CHECK:                   OpSelectionMerge %36 None
;CHECK:                   OpBranchConditional %34 %37 %38
;CHECK:             %37 = OpLabel
;CHECK:             %39 = OpLoad %10 %tBuf
;CHECK:             %40 = OpSampledImage %18 %39 %17
;CHECK:             %41 = OpImage %10 %40
;CHECK:             %42 = OpImageFetch %v4float %41 %23
;CHECK:                   OpBranch %36
;CHECK:             %38 = OpLabel
;CHECK:            %102 = OpFunctionCall %void %43 %uint_42 %uint_6 %uint_0 %30 %32
;CHECK:                   OpBranch %36
;CHECK:             %36 = OpLabel
;CHECK:            %104 = OpPhi %v4float %42 %37 %103 %38
;CHECK:                   OpStore %x %104
                          OpReturn
                          OpFunctionEnd
;CHECK:             %43 = OpFunction %void None %44
;CHECK:             %45 = OpFunctionParameter %uint
;CHECK:             %46 = OpFunctionParameter %uint
;CHECK:             %47 = OpFunctionParameter %uint
;CHECK:             %48 = OpFunctionParameter %uint
;CHECK:             %49 = OpFunctionParameter %uint
;CHECK:             %50 = OpLabel
;CHECK:             %56 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_0
;CHECK:             %59 = OpAtomicIAdd %uint %56 %uint_4 %uint_0 %uint_11
;CHECK:             %60 = OpIAdd %uint %59 %uint_11
;CHECK:             %61 = OpArrayLength %uint %54 1
;CHECK:             %62 = OpULessThanEqual %bool %60 %61
;CHECK:                   OpSelectionMerge %63 None
;CHECK:                   OpBranchConditional %62 %64 %63
;CHECK:             %64 = OpLabel
;CHECK:             %65 = OpIAdd %uint %59 %uint_0
;CHECK:             %67 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_1 %65
;CHECK:                   OpStore %67 %uint_11
;CHECK:             %69 = OpIAdd %uint %59 %uint_1
;CHECK:             %70 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_1 %69
;CHECK:                   OpStore %70 %uint_23
;CHECK:             %72 = OpIAdd %uint %59 %uint_2
;CHECK:             %73 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_1 %72
;CHECK:                   OpStore %73 %45
;CHECK:             %75 = OpIAdd %uint %59 %uint_3
;CHECK:             %76 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_1 %75
;CHECK:                   OpStore %76 %uint_4
;CHECK:             %79 = OpLoad %v4float %gl_FragCoord
;CHECK:             %81 = OpBitcast %v4uint %79
;CHECK:             %82 = OpCompositeExtract %uint %81 0
;CHECK:             %83 = OpIAdd %uint %59 %uint_4
;CHECK:             %84 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_1 %83
;CHECK:                   OpStore %84 %82
;CHECK:             %85 = OpCompositeExtract %uint %81 1
;CHECK:             %87 = OpIAdd %uint %59 %uint_5
;CHECK:             %88 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_1 %87
;CHECK:                   OpStore %88 %85
;CHECK:             %90 = OpIAdd %uint %59 %uint_7
;CHECK:             %91 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_1 %90
;CHECK:                   OpStore %91 %46
;CHECK:             %93 = OpIAdd %uint %59 %uint_8
;CHECK:             %94 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_1 %93
;CHECK:                   OpStore %94 %47
;CHECK:             %96 = OpIAdd %uint %59 %uint_9
;CHECK:             %97 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_1 %96
;CHECK:                   OpStore %97 %48
;CHECK:             %99 = OpIAdd %uint %59 %uint_10
;CHECK:             %100 = OpAccessChain %_ptr_StorageBuffer_uint %54 %uint_1 %99
;CHECK:                   OpStore %100 %49
;CHECK:                   OpBranch %63
;CHECK:             %63 = OpLabel
;CHECK:                   OpReturn
;CHECK:                   OpFunctionEnd
  )";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBindlessCheckPass>(text, true, 7u, 23u, false,
                                               false, true, true, true);
}

// TODO(greg-lunarg): Add tests to verify handling of these cases:
//
//   Compute shader
//   Geometry shader
//   Tesselation control shader
//   Tesselation eval shader
//   OpImage
//   SampledImage variable

}  // namespace
}  // namespace opt
}  // namespace spvtools
