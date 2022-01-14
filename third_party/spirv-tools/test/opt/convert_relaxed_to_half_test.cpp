// Copyright (c) 2019 Valve Corporation
// Copyright (c) 2019 LunarG Inc.
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

// Convert Relaxed to Half tests

#include <string>
#include <vector>

#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using ConvertToHalfTest = PassTest<::testing::Test>;

TEST_F(ConvertToHalfTest, ConvertToHalfBasic) {
  // The resulting SPIR-V was processed with --relax-float-ops.
  //
  // clang-format off
  //
  // SamplerState       g_sSamp : register(s0);
  // uniform Texture1D <float4> g_tTex1df4 : register(t0);
  //
  // struct PS_INPUT
  // {
  //   float Tex0 : TEXCOORD0;
  // };
  //
  // struct PS_OUTPUT
  // {
  //   float4 Color : SV_Target0;
  // };
  //
  // cbuffer cbuff{
  //   float c;
  // }
  //
  // PS_OUTPUT main(PS_INPUT i)
  // {
  //   PS_OUTPUT psout;
  //   psout.Color = g_tTex1df4.Sample(g_sSamp, i.Tex0) * c;
  //   return psout;
  // }
  //
  // clang-format on

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability Sampled1D
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %i_Tex0 %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %g_tTex1df4 "g_tTex1df4"
OpName %g_sSamp "g_sSamp"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "c"
OpName %_ ""
OpName %i_Tex0 "i.Tex0"
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpMemberDecorate %cbuff 0 Offset 0
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 1
OpDecorate %i_Tex0 Location 0
OpDecorate %_entryPointOutput_Color Location 0
OpDecorate %48 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%19 = OpTypeImage %float 1D 0 0 0 1 Unknown
%_ptr_UniformConstant_19 = OpTypePointer UniformConstant %19
%g_tTex1df4 = OpVariable %_ptr_UniformConstant_19 UniformConstant
%23 = OpTypeSampler
%_ptr_UniformConstant_23 = OpTypePointer UniformConstant %23
%g_sSamp = OpVariable %_ptr_UniformConstant_23 UniformConstant
%27 = OpTypeSampledImage %19
%cbuff = OpTypeStruct %float
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Input_float = OpTypePointer Input %float
%i_Tex0 = OpVariable %_ptr_Input_float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability Sampled1D
OpCapability Float16
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %i_Tex0 %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %g_tTex1df4 "g_tTex1df4"
OpName %g_sSamp "g_sSamp"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "c"
OpName %_ ""
OpName %i_Tex0 "i.Tex0"
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpMemberDecorate %cbuff 0 Offset 0
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 1
OpDecorate %i_Tex0 Location 0
OpDecorate %_entryPointOutput_Color Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%19 = OpTypeImage %float 1D 0 0 0 1 Unknown
%_ptr_UniformConstant_19 = OpTypePointer UniformConstant %19
%g_tTex1df4 = OpVariable %_ptr_UniformConstant_19 UniformConstant
%23 = OpTypeSampler
%_ptr_UniformConstant_23 = OpTypePointer UniformConstant %23
%g_sSamp = OpVariable %_ptr_UniformConstant_23 UniformConstant
%27 = OpTypeSampledImage %19
%cbuff = OpTypeStruct %float
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Input_float = OpTypePointer Input %float
%i_Tex0 = OpVariable %_ptr_Input_float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
%half = OpTypeFloat 16
%v4half = OpTypeVector %half 4
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%48 = OpLoad %float %i_Tex0
%58 = OpLoad %19 %g_tTex1df4
%59 = OpLoad %23 %g_sSamp
%60 = OpSampledImage %27 %58 %59
%63 = OpImageSampleImplicitLod %v4float %60 %48
%64 = OpAccessChain %_ptr_Uniform_float %_ %int_0
%65 = OpLoad %float %64
%66 = OpVectorTimesScalar %v4float %63 %65
OpStore %_entryPointOutput_Color %66
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%48 = OpLoad %float %i_Tex0
%58 = OpLoad %19 %g_tTex1df4
%59 = OpLoad %23 %g_sSamp
%60 = OpSampledImage %27 %58 %59
%63 = OpImageSampleImplicitLod %v4float %60 %48
%64 = OpAccessChain %_ptr_Uniform_float %_ %int_0
%65 = OpLoad %float %64
%69 = OpFConvert %v4half %63
%70 = OpFConvert %half %65
%66 = OpVectorTimesScalar %v4half %69 %70
%71 = OpFConvert %v4float %66
OpStore %_entryPointOutput_Color %71
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<ConvertToHalfPass>(defs_before + func_before,
                                           defs_after + func_after, true, true);
}

TEST_F(ConvertToHalfTest, ConvertToHalfForLinkage) {
  const std::string before =
      R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpSource HLSL 630
OpName %type_cbuff "type.cbuff"
OpMemberName %type_cbuff 0 "c"
OpName %cbuff "cbuff"
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %bb_entry "bb.entry"
OpName %v "v"
OpDecorate %main LinkageAttributes "main" Export
OpDecorate %cbuff DescriptorSet 0
OpDecorate %cbuff Binding 0
OpMemberDecorate %type_cbuff 0 Offset 0
OpDecorate %type_cbuff Block
OpDecorate %18 RelaxedPrecision
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%type_cbuff = OpTypeStruct %float
%_ptr_Uniform_type_cbuff = OpTypePointer Uniform %type_cbuff
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%9 = OpTypeFunction %v4float %_ptr_Function_v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%cbuff = OpVariable %_ptr_Uniform_type_cbuff Uniform
%main = OpFunction %v4float None %9
%BaseColor = OpFunctionParameter %_ptr_Function_v4float
%bb_entry = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
%16 = OpAccessChain %_ptr_Uniform_float %cbuff %int_0
%17 = OpLoad %float %16
%18 = OpVectorTimesScalar %v4float %14 %17
OpStore %v %18
%19 = OpLoad %v4float %v
OpReturnValue %19
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
OpCapability Linkage
OpCapability Float16
OpMemoryModel Logical GLSL450
OpSource HLSL 630
OpName %type_cbuff "type.cbuff"
OpMemberName %type_cbuff 0 "c"
OpName %cbuff "cbuff"
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %bb_entry "bb.entry"
OpName %v "v"
OpDecorate %main LinkageAttributes "main" Export
OpDecorate %cbuff DescriptorSet 0
OpDecorate %cbuff Binding 0
OpMemberDecorate %type_cbuff 0 Offset 0
OpDecorate %type_cbuff Block
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%type_cbuff = OpTypeStruct %float
%_ptr_Uniform_type_cbuff = OpTypePointer Uniform %type_cbuff
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%14 = OpTypeFunction %v4float %_ptr_Function_v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%cbuff = OpVariable %_ptr_Uniform_type_cbuff Uniform
%half = OpTypeFloat 16
%v4half = OpTypeVector %half 4
%main = OpFunction %v4float None %14
%BaseColor = OpFunctionParameter %_ptr_Function_v4float
%bb_entry = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%16 = OpLoad %v4float %BaseColor
%17 = OpAccessChain %_ptr_Uniform_float %cbuff %int_0
%18 = OpLoad %float %17
%22 = OpFConvert %v4half %16
%23 = OpFConvert %half %18
%7 = OpVectorTimesScalar %v4half %22 %23
%24 = OpFConvert %v4float %7
OpStore %v %24
%19 = OpLoad %v4float %v
OpReturnValue %19
OpFunctionEnd
)";

  SinglePassRunAndCheck<ConvertToHalfPass>(before, after, true, true);
}
TEST_F(ConvertToHalfTest, ConvertToHalfWithDrefSample) {
  // The resulting SPIR-V was processed with --relax-float-ops.
  //
  // clang-format off
  //
  // SamplerComparisonState       g_sSamp : register(s0);
  // uniform Texture1D <float4> g_tTex1df4 : register(t0);
  //
  // cbuffer cbuff{
  //   float c1;
  // float c2;
  // };
  //
  // struct PS_INPUT
  // {
  //   float Tex0 : TEXCOORD0;
  //   float Tex1 : TEXCOORD1;
  // };
  //
  // struct PS_OUTPUT
  // {
  //   float Color : SV_Target0;
  // };
  //
  // PS_OUTPUT main(PS_INPUT i)
  // {
  //   PS_OUTPUT psout;
  //   float txval10 = g_tTex1df4.SampleCmp(g_sSamp, i.Tex0 * 0.1, c1 + 0.1);
  //   float txval11 = g_tTex1df4.SampleCmp(g_sSamp, i.Tex1 * 0.2, c2 + 0.2);
  //   float t = txval10 + txval11;
  //   float t2 = t / 2.0;
  //   psout.Color = t2;
  //   return psout;
  // }
  //
  // clang-format on

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability Sampled1D
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %i_Tex0 %i_Tex1 %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %g_tTex1df4 "g_tTex1df4"
OpName %g_sSamp "g_sSamp"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "c1"
OpMemberName %cbuff 1 "c2"
OpName %_ ""
OpName %i_Tex0 "i.Tex0"
OpName %i_Tex1 "i.Tex1"
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpMemberDecorate %cbuff 0 Offset 0
OpMemberDecorate %cbuff 1 Offset 4
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 1
OpDecorate %i_Tex0 Location 0
OpDecorate %i_Tex1 Location 1
OpDecorate %_entryPointOutput_Color Location 0
OpDecorate %100 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%16 = OpTypeImage %float 1D 1 0 0 1 Unknown
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
%g_tTex1df4 = OpVariable %_ptr_UniformConstant_16 UniformConstant
%20 = OpTypeSampler
%_ptr_UniformConstant_20 = OpTypePointer UniformConstant %20
%g_sSamp = OpVariable %_ptr_UniformConstant_20 UniformConstant
%24 = OpTypeSampledImage %16
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_100000001 = OpConstant %float 0.100000001
%cbuff = OpTypeStruct %float %float
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%v2float = OpTypeVector %float 2
%int_1 = OpConstant %int 1
%float_0_200000003 = OpConstant %float 0.200000003
%_ptr_Input_float = OpTypePointer Input %float
%i_Tex0 = OpVariable %_ptr_Input_float Input
%i_Tex1 = OpVariable %_ptr_Input_float Input
%_ptr_Output_float = OpTypePointer Output %float
%_entryPointOutput_Color = OpVariable %_ptr_Output_float Output
%float_0_5 = OpConstant %float 0.5
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability Sampled1D
OpCapability Float16
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %i_Tex0 %i_Tex1 %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %g_tTex1df4 "g_tTex1df4"
OpName %g_sSamp "g_sSamp"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "c1"
OpMemberName %cbuff 1 "c2"
OpName %_ ""
OpName %i_Tex0 "i.Tex0"
OpName %i_Tex1 "i.Tex1"
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpMemberDecorate %cbuff 0 Offset 0
OpMemberDecorate %cbuff 1 Offset 4
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 1
OpDecorate %i_Tex0 Location 0
OpDecorate %i_Tex1 Location 1
OpDecorate %_entryPointOutput_Color Location 0
%void = OpTypeVoid
%25 = OpTypeFunction %void
%float = OpTypeFloat 32
%27 = OpTypeImage %float 1D 1 0 0 1 Unknown
%_ptr_UniformConstant_27 = OpTypePointer UniformConstant %27
%g_tTex1df4 = OpVariable %_ptr_UniformConstant_27 UniformConstant
%29 = OpTypeSampler
%_ptr_UniformConstant_29 = OpTypePointer UniformConstant %29
%g_sSamp = OpVariable %_ptr_UniformConstant_29 UniformConstant
%31 = OpTypeSampledImage %27
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_100000001 = OpConstant %float 0.100000001
%cbuff = OpTypeStruct %float %float
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%v2float = OpTypeVector %float 2
%int_1 = OpConstant %int 1
%float_0_200000003 = OpConstant %float 0.200000003
%_ptr_Input_float = OpTypePointer Input %float
%i_Tex0 = OpVariable %_ptr_Input_float Input
%i_Tex1 = OpVariable %_ptr_Input_float Input
%_ptr_Output_float = OpTypePointer Output %float
%_entryPointOutput_Color = OpVariable %_ptr_Output_float Output
%float_0_5 = OpConstant %float 0.5
%half = OpTypeFloat 16
%v2half = OpTypeVector %half 2
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%76 = OpLoad %float %i_Tex0
%79 = OpLoad %float %i_Tex1
%93 = OpLoad %16 %g_tTex1df4
%94 = OpLoad %20 %g_sSamp
%95 = OpSampledImage %24 %93 %94
%98 = OpFMul %float %76 %float_0_100000001
%99 = OpAccessChain %_ptr_Uniform_float %_ %int_0
%100 = OpLoad %float %99
%101 = OpFAdd %float %100 %float_0_100000001
%102 = OpCompositeConstruct %v2float %98 %101
%104 = OpImageSampleDrefImplicitLod %float %95 %102 %101
%105 = OpLoad %16 %g_tTex1df4
%106 = OpLoad %20 %g_sSamp
%107 = OpSampledImage %24 %105 %106
%110 = OpFMul %float %79 %float_0_200000003
%111 = OpAccessChain %_ptr_Uniform_float %_ %int_1
%112 = OpLoad %float %111
%113 = OpFAdd %float %112 %float_0_200000003
%114 = OpCompositeConstruct %v2float %110 %113
%116 = OpImageSampleDrefImplicitLod %float %107 %114 %113
%119 = OpFAdd %float %104 %116
%121 = OpFMul %float %119 %float_0_5
OpStore %_entryPointOutput_Color %121
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %25
%43 = OpLabel
%11 = OpLoad %float %i_Tex0
%12 = OpLoad %float %i_Tex1
%44 = OpLoad %27 %g_tTex1df4
%45 = OpLoad %29 %g_sSamp
%46 = OpSampledImage %31 %44 %45
%53 = OpFConvert %half %11
%54 = OpFConvert %half %float_0_100000001
%13 = OpFMul %half %53 %54
%47 = OpAccessChain %_ptr_Uniform_float %_ %int_0
%10 = OpLoad %float %47
%55 = OpFConvert %half %10
%56 = OpFConvert %half %float_0_100000001
%14 = OpFAdd %half %55 %56
%16 = OpCompositeConstruct %v2half %13 %14
%58 = OpFConvert %float %14
%18 = OpImageSampleDrefImplicitLod %float %46 %16 %58
%48 = OpLoad %27 %g_tTex1df4
%49 = OpLoad %29 %g_sSamp
%50 = OpSampledImage %31 %48 %49
%59 = OpFConvert %half %12
%60 = OpFConvert %half %float_0_200000003
%15 = OpFMul %half %59 %60
%51 = OpAccessChain %_ptr_Uniform_float %_ %int_1
%17 = OpLoad %float %51
%61 = OpFConvert %half %17
%62 = OpFConvert %half %float_0_200000003
%19 = OpFAdd %half %61 %62
%20 = OpCompositeConstruct %v2half %15 %19
%63 = OpFConvert %float %19
%21 = OpImageSampleDrefImplicitLod %float %50 %20 %63
%64 = OpFConvert %half %18
%65 = OpFConvert %half %21
%22 = OpFAdd %half %64 %65
%66 = OpFConvert %half %float_0_5
%23 = OpFMul %half %22 %66
%67 = OpFConvert %float %23
OpStore %_entryPointOutput_Color %67
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<ConvertToHalfPass>(defs_before + func_before,
                                           defs_after + func_after, true, true);
}

TEST_F(ConvertToHalfTest, ConvertToHalfWithVectorMatrixMult) {
  // The resulting SPIR-V was processed with --relax-float-ops.
  //
  // clang-format off
  //
  // SamplerState       g_sSamp : register(s0);
  // uniform Texture1D <float4> g_tTex1df4 : register(t0);
  //
  // struct PS_OUTPUT
  // {
  //   float4 Color : SV_Target0;
  // };
  //
  // cbuffer cbuff{
  //   float4x4 M;
  // }
  //
  // PS_OUTPUT main()
  // {
  //  PS_OUTPUT psout;
  //  float4 txval10 = g_tTex1df4.Sample(g_sSamp, 0.1);
  //  float4 t = mul(txval10, M);
  //  psout.Color = t;
  //  return psout;
  //}
  //
  // clang-format on

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability Sampled1D
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %g_tTex1df4 "g_tTex1df4"
OpName %g_sSamp "g_sSamp"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "M"
OpName %_ ""
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpMemberDecorate %cbuff 0 RowMajor
OpMemberDecorate %cbuff 0 Offset 0
OpMemberDecorate %cbuff 0 MatrixStride 16
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 1
OpDecorate %_entryPointOutput_Color Location 0
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%14 = OpTypeImage %float 1D 0 0 0 1 Unknown
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
%g_tTex1df4 = OpVariable %_ptr_UniformConstant_14 UniformConstant
%18 = OpTypeSampler
%_ptr_UniformConstant_18 = OpTypePointer UniformConstant %18
%g_sSamp = OpVariable %_ptr_UniformConstant_18 UniformConstant
%22 = OpTypeSampledImage %14
%float_0_100000001 = OpConstant %float 0.100000001
%mat4v4float = OpTypeMatrix %v4float 4
%cbuff = OpTypeStruct %mat4v4float
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability Sampled1D
OpCapability Float16
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %g_tTex1df4 "g_tTex1df4"
OpName %g_sSamp "g_sSamp"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "M"
OpName %_ ""
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpMemberDecorate %cbuff 0 RowMajor
OpMemberDecorate %cbuff 0 Offset 0
OpMemberDecorate %cbuff 0 MatrixStride 16
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 1
OpDecorate %_entryPointOutput_Color Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%14 = OpTypeImage %float 1D 0 0 0 1 Unknown
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
%g_tTex1df4 = OpVariable %_ptr_UniformConstant_14 UniformConstant
%18 = OpTypeSampler
%_ptr_UniformConstant_18 = OpTypePointer UniformConstant %18
%g_sSamp = OpVariable %_ptr_UniformConstant_18 UniformConstant
%22 = OpTypeSampledImage %14
%float_0_100000001 = OpConstant %float 0.100000001
%mat4v4float = OpTypeMatrix %v4float 4
%cbuff = OpTypeStruct %mat4v4float
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
%half = OpTypeFloat 16
%v4half = OpTypeVector %half 4
%mat4v4half = OpTypeMatrix %v4half 4
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%53 = OpLoad %14 %g_tTex1df4
%54 = OpLoad %18 %g_sSamp
%55 = OpSampledImage %22 %53 %54
%56 = OpImageSampleImplicitLod %v4float %55 %float_0_100000001
%57 = OpAccessChain %_ptr_Uniform_mat4v4float %_ %int_0
%58 = OpLoad %mat4v4float %57
%60 = OpMatrixTimesVector %v4float %58 %56
OpStore %_entryPointOutput_Color %60
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%53 = OpLoad %14 %g_tTex1df4
%54 = OpLoad %18 %g_sSamp
%55 = OpSampledImage %22 %53 %54
%56 = OpImageSampleImplicitLod %v4float %55 %float_0_100000001
%57 = OpAccessChain %_ptr_Uniform_mat4v4float %_ %int_0
%58 = OpLoad %mat4v4float %57
%67 = OpCompositeExtract %v4float %58 0
%68 = OpFConvert %v4half %67
%69 = OpCompositeExtract %v4float %58 1
%70 = OpFConvert %v4half %69
%71 = OpCompositeExtract %v4float %58 2
%72 = OpFConvert %v4half %71
%73 = OpCompositeExtract %v4float %58 3
%74 = OpFConvert %v4half %73
%75 = OpCompositeConstruct %mat4v4half %68 %70 %72 %74
%64 = OpCopyObject %mat4v4float %58
%65 = OpFConvert %v4half %56
%60 = OpMatrixTimesVector %v4half %75 %65
%66 = OpFConvert %v4float %60
OpStore %_entryPointOutput_Color %66
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<ConvertToHalfPass>(defs_before + func_before,
                                           defs_after + func_after, true, true);
}

TEST_F(ConvertToHalfTest, ConvertToHalfWithPhi) {
  // The resulting SPIR-V was processed with --relax-float-ops.
  //
  // clang-format off
  //
  // SamplerState       g_sSamp : register(s0);
  // uniform Texture1D <float4> g_tTex1df4 : register(t0);
  //
  // struct PS_OUTPUT
  // {
  //   float4 Color : SV_Target0;
  // };
  //
  // cbuffer cbuff{
  //   bool b;
  //   float4x4 M;
  // }
  //
  // PS_OUTPUT main()
  // {
  //   PS_OUTPUT psout;
  //   float4 t;
  //
  //   if (b)
  //     t = g_tTex1df4.Sample(g_sSamp, 0.1);
  //   else
  //     t = float4(0.0, 0.0, 0.0, 0.0);
  //
  //   float4 t2 = t * 2.0;
  //   psout.Color = t2;
  //   return psout;
  // }
  //
  // clang-format on

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability Sampled1D
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "b"
OpMemberName %cbuff 1 "M"
OpName %_ ""
OpName %g_tTex1df4 "g_tTex1df4"
OpName %g_sSamp "g_sSamp"
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpMemberDecorate %cbuff 0 Offset 0
OpMemberDecorate %cbuff 1 RowMajor
OpMemberDecorate %cbuff 1 Offset 16
OpMemberDecorate %cbuff 1 MatrixStride 16
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 1
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpDecorate %_entryPointOutput_Color Location 0
OpDecorate %72 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%mat4v4float = OpTypeMatrix %v4float 4
%cbuff = OpTypeStruct %uint %mat4v4float
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%bool = OpTypeBool
%uint_0 = OpConstant %uint 0
%29 = OpTypeImage %float 1D 0 0 0 1 Unknown
%_ptr_UniformConstant_29 = OpTypePointer UniformConstant %29
%g_tTex1df4 = OpVariable %_ptr_UniformConstant_29 UniformConstant
%33 = OpTypeSampler
%_ptr_UniformConstant_33 = OpTypePointer UniformConstant %33
%g_sSamp = OpVariable %_ptr_UniformConstant_33 UniformConstant
%37 = OpTypeSampledImage %29
%float_0_100000001 = OpConstant %float 0.100000001
%float_0 = OpConstant %float 0
%43 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_2 = OpConstant %float 2
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability Sampled1D
OpCapability Float16
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "b"
OpMemberName %cbuff 1 "M"
OpName %_ ""
OpName %g_tTex1df4 "g_tTex1df4"
OpName %g_sSamp "g_sSamp"
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpMemberDecorate %cbuff 0 Offset 0
OpMemberDecorate %cbuff 1 RowMajor
OpMemberDecorate %cbuff 1 Offset 16
OpMemberDecorate %cbuff 1 MatrixStride 16
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 1
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpDecorate %_entryPointOutput_Color Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%mat4v4float = OpTypeMatrix %v4float 4
%cbuff = OpTypeStruct %uint %mat4v4float
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%bool = OpTypeBool
%uint_0 = OpConstant %uint 0
%29 = OpTypeImage %float 1D 0 0 0 1 Unknown
%_ptr_UniformConstant_29 = OpTypePointer UniformConstant %29
%g_tTex1df4 = OpVariable %_ptr_UniformConstant_29 UniformConstant
%33 = OpTypeSampler
%_ptr_UniformConstant_33 = OpTypePointer UniformConstant %33
%g_sSamp = OpVariable %_ptr_UniformConstant_33 UniformConstant
%37 = OpTypeSampledImage %29
%float_0_100000001 = OpConstant %float 0.100000001
%float_0 = OpConstant %float 0
%43 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_2 = OpConstant %float 2
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
%half = OpTypeFloat 16
%v4half = OpTypeVector %half 4
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%63 = OpAccessChain %_ptr_Uniform_uint %_ %int_0
%64 = OpLoad %uint %63
%65 = OpINotEqual %bool %64 %uint_0
OpSelectionMerge %66 None
OpBranchConditional %65 %67 %68
%67 = OpLabel
%69 = OpLoad %29 %g_tTex1df4
%70 = OpLoad %33 %g_sSamp
%71 = OpSampledImage %37 %69 %70
%72 = OpImageSampleImplicitLod %v4float %71 %float_0_100000001
OpBranch %66
%68 = OpLabel
OpBranch %66
%66 = OpLabel
%85 = OpPhi %v4float %72 %67 %43 %68
%74 = OpVectorTimesScalar %v4float %85 %float_2
OpStore %_entryPointOutput_Color %74
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%63 = OpAccessChain %_ptr_Uniform_uint %_ %int_0
%64 = OpLoad %uint %63
%65 = OpINotEqual %bool %64 %uint_0
OpSelectionMerge %66 None
OpBranchConditional %65 %67 %68
%67 = OpLabel
%69 = OpLoad %29 %g_tTex1df4
%70 = OpLoad %33 %g_sSamp
%71 = OpSampledImage %37 %69 %70
%72 = OpImageSampleImplicitLod %v4float %71 %float_0_100000001
%88 = OpFConvert %v4half %72
OpBranch %66
%68 = OpLabel
%89 = OpFConvert %v4half %43
OpBranch %66
%66 = OpLabel
%85 = OpPhi %v4half %88 %67 %89 %68
%90 = OpFConvert %half %float_2
%74 = OpVectorTimesScalar %v4half %85 %90
%91 = OpFConvert %v4float %74
OpStore %_entryPointOutput_Color %91
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<ConvertToHalfPass>(defs_before + func_before,
                                           defs_after + func_after, true, true);
}

TEST_F(ConvertToHalfTest, ConvertToHalfWithLoopAndFConvert) {
  // The resulting SPIR-V was processed with --relax-float-ops.
  //
  // The loop causes an FConvert to be generated at the bottom of the loop
  // for the Phi. The FConvert is later processed and turned into a (dead)
  // copy.
  //
  // clang-format off
  //
  // struct PS_OUTPUT
  // {
  //   float4 Color : SV_Target0;
  // };
  //
  // cbuffer cbuff{
  //   float4 a[10];
  // }
  //
  // PS_OUTPUT main()
  // {
  //   PS_OUTPUT psout;
  //   float4 t = 0.0;;
  //
  //   for (int i = 0; i<10; ++i)
  //     t = t + a[i];
  //
  //   float4 t2 = t / 10.0;
  //   psout.Color = t2;
  //   return psout;
  // }
  //
  // clang-format on

  const std::string defs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "a"
OpName %_ ""
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %_arr_v4float_uint_10 ArrayStride 16
OpMemberDecorate %cbuff 0 Offset 0
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
OpDecorate %_entryPointOutput_Color Location 0
OpDecorate %96 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%float_0 = OpConstant %float 0
%15 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_v4float_uint_10 = OpTypeArray %v4float %uint_10
%cbuff = OpTypeStruct %_arr_v4float_uint_10
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
%float_0_100000001 = OpConstant %float 0.100000001
%94 = OpConstantComposite %v4float %float_0_100000001 %float_0_100000001 %float_0_100000001 %float_0_100000001
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability Float16
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "a"
OpName %_ ""
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %_arr_v4float_uint_10 ArrayStride 16
OpMemberDecorate %cbuff 0 Offset 0
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
OpDecorate %_entryPointOutput_Color Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%float_0 = OpConstant %float 0
%15 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_v4float_uint_10 = OpTypeArray %v4float %uint_10
%cbuff = OpTypeStruct %_arr_v4float_uint_10
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
%float_0_100000001 = OpConstant %float 0.100000001
%94 = OpConstantComposite %v4float %float_0_100000001 %float_0_100000001 %float_0_100000001 %float_0_100000001
%half = OpTypeFloat 16
%v4half = OpTypeVector %half 4
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
OpBranch %65
%65 = OpLabel
%96 = OpPhi %v4float %15 %5 %76 %71
%95 = OpPhi %int %int_0 %5 %78 %71
%70 = OpSLessThan %bool %95 %int_10
OpLoopMerge %66 %71 None
OpBranchConditional %70 %71 %66
%71 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %_ %int_0 %95
%75 = OpLoad %v4float %74
%76 = OpFAdd %v4float %96 %75
%78 = OpIAdd %int %95 %int_1
OpBranch %65
%66 = OpLabel
%81 = OpFMul %v4float %96 %94
OpStore %_entryPointOutput_Color %81
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%99 = OpFConvert %v4half %15
OpBranch %65
%65 = OpLabel
%96 = OpPhi %v4half %99 %5 %100 %71
%95 = OpPhi %int %int_0 %5 %78 %71
%70 = OpSLessThan %bool %95 %int_10
OpLoopMerge %66 %71 None
OpBranchConditional %70 %71 %66
%71 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %_ %int_0 %95
%75 = OpLoad %v4float %74
%103 = OpFConvert %v4half %75
%76 = OpFAdd %v4half %96 %103
%78 = OpIAdd %int %95 %int_1
%100 = OpCopyObject %v4half %76
OpBranch %65
%66 = OpLabel
%101 = OpFConvert %v4half %94
%81 = OpFMul %v4half %96 %101
%102 = OpFConvert %v4float %81
OpStore %_entryPointOutput_Color %102
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<ConvertToHalfPass>(defs_before + func_before,
                                           defs_after + func_after, true, true);
}

TEST_F(ConvertToHalfTest, ConvertToHalfWithExtracts) {
  // The resulting SPIR-V was processed with --relax-float-ops.
  //
  // The extra converts in the func_after can be DCE'd.
  //
  // clang-format off
  //
  // SamplerState       g_sSamp : register(s0);
  // uniform Texture1D <float4> g_tTex1df4 : register(t0);
  //
  // struct PS_INPUT
  // {
  //   float Tex0 : TEXCOORD0;
  // };
  //
  // struct PS_OUTPUT
  // {
  //   float4 Color : SV_Target0;
  // };
  //
  // cbuffer cbuff{
  //   float c;
  // }
  //
  // PS_OUTPUT main(PS_INPUT i)
  // {
  //   PS_OUTPUT psout;
  //   float4 tx = g_tTex1df4.Sample(g_sSamp, i.Tex0);
  //   float4 t = float4(tx.y, tx.z, tx.x, tx.w) * c;
  //   psout.Color = t;
  //   return psout;
  // }
  //
  // clang-format on

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability Sampled1D
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %i_Tex0 %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %g_tTex1df4 "g_tTex1df4"
OpName %g_sSamp "g_sSamp"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "c"
OpName %_ ""
OpName %i_Tex0 "i.Tex0"
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpMemberDecorate %cbuff 0 Offset 0
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 1
OpDecorate %i_Tex0 Location 0
OpDecorate %_entryPointOutput_Color Location 0
OpDecorate %65 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%17 = OpTypeImage %float 1D 0 0 0 1 Unknown
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
%g_tTex1df4 = OpVariable %_ptr_UniformConstant_17 UniformConstant
%21 = OpTypeSampler
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %21
%g_sSamp = OpVariable %_ptr_UniformConstant_21 UniformConstant
%25 = OpTypeSampledImage %17
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%cbuff = OpTypeStruct %float
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Input_float = OpTypePointer Input %float
%i_Tex0 = OpVariable %_ptr_Input_float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability Sampled1D
OpCapability Float16
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %i_Tex0 %_entryPointOutput_Color
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %g_tTex1df4 "g_tTex1df4"
OpName %g_sSamp "g_sSamp"
OpName %cbuff "cbuff"
OpMemberName %cbuff 0 "c"
OpName %_ ""
OpName %i_Tex0 "i.Tex0"
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpMemberDecorate %cbuff 0 Offset 0
OpDecorate %cbuff Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 1
OpDecorate %i_Tex0 Location 0
OpDecorate %_entryPointOutput_Color Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%17 = OpTypeImage %float 1D 0 0 0 1 Unknown
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
%g_tTex1df4 = OpVariable %_ptr_UniformConstant_17 UniformConstant
%21 = OpTypeSampler
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %21
%g_sSamp = OpVariable %_ptr_UniformConstant_21 UniformConstant
%25 = OpTypeSampledImage %17
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%cbuff = OpTypeStruct %float
%_ptr_Uniform_cbuff = OpTypePointer Uniform %cbuff
%_ = OpVariable %_ptr_Uniform_cbuff Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Input_float = OpTypePointer Input %float
%i_Tex0 = OpVariable %_ptr_Input_float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
%half = OpTypeFloat 16
%v4half = OpTypeVector %half 4
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%65 = OpLoad %float %i_Tex0
%77 = OpLoad %17 %g_tTex1df4
%78 = OpLoad %21 %g_sSamp
%79 = OpSampledImage %25 %77 %78
%82 = OpImageSampleImplicitLod %v4float %79 %65
%84 = OpCompositeExtract %float %82 1
%86 = OpCompositeExtract %float %82 2
%88 = OpCompositeExtract %float %82 0
%90 = OpCompositeExtract %float %82 3
%91 = OpCompositeConstruct %v4float %84 %86 %88 %90
%92 = OpAccessChain %_ptr_Uniform_float %_ %int_0
%93 = OpLoad %float %92
%94 = OpVectorTimesScalar %v4float %91 %93
OpStore %_entryPointOutput_Color %94
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%65 = OpLoad %float %i_Tex0
%77 = OpLoad %17 %g_tTex1df4
%78 = OpLoad %21 %g_sSamp
%79 = OpSampledImage %25 %77 %78
%82 = OpImageSampleImplicitLod %v4float %79 %65
%97 = OpFConvert %v4half %82
%84 = OpCompositeExtract %half %97 1
%98 = OpFConvert %v4half %82
%86 = OpCompositeExtract %half %98 2
%99 = OpFConvert %v4half %82
%88 = OpCompositeExtract %half %99 0
%100 = OpFConvert %v4half %82
%90 = OpCompositeExtract %half %100 3
%91 = OpCompositeConstruct %v4half %84 %86 %88 %90
%92 = OpAccessChain %_ptr_Uniform_float %_ %int_0
%93 = OpLoad %float %92
%101 = OpFConvert %half %93
%94 = OpVectorTimesScalar %v4half %91 %101
%102 = OpFConvert %v4float %94
OpStore %_entryPointOutput_Color %102
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<ConvertToHalfPass>(defs_before + func_before,
                                           defs_after + func_after, true, true);
}

TEST_F(ConvertToHalfTest, ConvertToHalfWithClosure) {
  // Include as many contiguous composite instructions as possible into
  // half-precision computations
  //
  // Compiled with glslang -V -Os
  //
  // clang-format off
  //
  // #version 410 core
  //
  //   precision mediump float;
  //
  // layout(location = 1) in vec3 foo;
  // layout(location = 2) in mat2 bar;
  // layout(location = 1) out vec3 res;
  //
  // vec3 func(vec3 tap, mat2 M) {
  //   return vec3(M * tap.xy, 1.0);
  // }
  //
  // void main() {
  //   res = func(foo, bar);
  // }
  //
  // clang-format on

  const std::string defs =
      R"(OpCapability Shader
; CHECK: OpCapability Float16
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %res %foo %bar
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 410
OpName %main "main"
OpName %res "res"
OpName %foo "foo"
OpName %bar "bar"
OpDecorate %res RelaxedPrecision
; CHECK-NOT: OpDecorate %res RelaxedPrecision
OpDecorate %res Location 1
OpDecorate %foo RelaxedPrecision
; CHECK-NOT: OpDecorate %foo RelaxedPrecision
OpDecorate %foo Location 1
OpDecorate %bar RelaxedPrecision
; CHECK-NOT: OpDecorate %bar RelaxedPrecision
OpDecorate %bar Location 2
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
; CHECK-NOT: OpDecorate %34 RelaxedPrecision
; CHECK-NOT: OpDecorate %36 RelaxedPrecision
; CHECK-NOT: OpDecorate %41 RelaxedPrecision
; CHECK-NOT: OpDecorate %42 RelaxedPrecision
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v3float = OpTypeVector %float 3
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%float_1 = OpConstant %float 1
%_ptr_Output_v3float = OpTypePointer Output %v3float
%res = OpVariable %_ptr_Output_v3float Output
%_ptr_Input_v3float = OpTypePointer Input %v3float
%foo = OpVariable %_ptr_Input_v3float Input
%_ptr_Input_mat2v2float = OpTypePointer Input %mat2v2float
%bar = OpVariable %_ptr_Input_mat2v2float Input
)";

  const std::string func =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%34 = OpLoad %v3float %foo
%36 = OpLoad %mat2v2float %bar
; CHECK: %48 = OpFConvert %v3half %34
; CHECK: %49 = OpFConvert %v3half %34
%41 = OpVectorShuffle %v2float %34 %34 0 1
; CHECK-NOT: %41 = OpVectorShuffle %v2float %34 %34 0 1
; CHECK: %41 = OpVectorShuffle %v2half %48 %49 0 1
%42 = OpMatrixTimesVector %v2float %36 %41
; CHECK-NOT: %42 = OpMatrixTimesVector %v2float %36 %41
; CHECK: %55 = OpCompositeExtract %v2float %36 0
; CHECK: %56 = OpFConvert %v2half %55
; CHECK: %57 = OpCompositeExtract %v2float %36 1
; CHECK: %58 = OpFConvert %v2half %57
; CHECK: %59 = OpCompositeConstruct %mat2v2half %56 %58
; CHECK: %52 = OpCopyObject %mat2v2float %36
; CHECK: %42 = OpMatrixTimesVector %v2half %59 %41
%43 = OpCompositeExtract %float %42 0
%44 = OpCompositeExtract %float %42 1
; CHECK-NOT: %43 = OpCompositeExtract %float %42 0
; CHECK-NOT: %44 = OpCompositeExtract %float %42 1
; CHECK: %43 = OpCompositeExtract %half %42 0
; CHECK: %44 = OpCompositeExtract %half %42 1
%45 = OpCompositeConstruct %v3float %43 %44 %float_1
; CHECK-NOT: %45 = OpCompositeConstruct %v3float %43 %44 %float_1
; CHECK: %53 = OpFConvert %float %43
; CHECK: %54 = OpFConvert %float %44
; CHECK: %45 = OpCompositeConstruct %v3float %53 %54 %float_1
OpStore %res %45
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<ConvertToHalfPass>(defs + func, true);
}

TEST_F(ConvertToHalfTest, RemoveRelaxDec) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/4117

  // This test is a case where the relax precision decorations need to be
  // removed, but the body of the function does not change because there are not
  // arithmetic operations.  So, there is not need for the Float16 capability.
  const std::string test =
      R"(
; CHECK-NOT: OpCapability Float16
; GLSL seems to generate this decoration on the load of a texture, which seems odd to me.
; This pass does not currently remove it, and I'm not sure what we should do with it, so I will leave it.
; CHECK: OpDecorate [[tex:%\w+]] RelaxedPrecision
; CHECK-NOT: OpDecorate {{%\w+}} RelaxedPrecision
; CHECK: OpLabel
; CHECK: [[tex]] = OpLoad {{%\w+}} %sTexture
; CHECK: [[coord:%\w+]] = OpLoad %v2float
; CHECK: [[retval:%\w+]] = OpImageSampleImplicitLod %v4float {{%\w+}} [[coord]]
; CHECK: OpStore %outFragColor [[retval]]
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %outFragColor %v_texcoord
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %outFragColor "outFragColor"
               OpName %sTexture "sTexture"
               OpName %v_texcoord "v_texcoord"
               OpDecorate %outFragColor RelaxedPrecision
               OpDecorate %outFragColor Location 0
               OpDecorate %sTexture RelaxedPrecision
               OpDecorate %sTexture DescriptorSet 0
               OpDecorate %sTexture Binding 0
               OpDecorate %14 RelaxedPrecision
               OpDecorate %v_texcoord RelaxedPrecision
               OpDecorate %v_texcoord Location 0
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%outFragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
   %sTexture = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
 %v_texcoord = OpVariable %_ptr_Input_v2float Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %11 %sTexture
         %18 = OpLoad %v2float %v_texcoord
         %19 = OpImageSampleImplicitLod %v4float %14 %18
               OpStore %outFragColor %19
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto result = SinglePassRunAndMatch<ConvertToHalfPass>(test, true);
  EXPECT_EQ(Pass::Status::SuccessWithChange, std::get<1>(result));
}

TEST_F(ConvertToHalfTest, HandleNonRelaxedPhi) {
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/4452

  // This test is a case with a non-relaxed phi with a relaxed operand.
  // A convert must be inserted at the end of the block associated with
  // the operand.
  const std::string test =
      R"(
; CHECK: [[fcvt:%\w+]] = OpFConvert %v3float {{%\w+}}
; CHECK-NEXT: OpSelectionMerge {{%\w+}} None
; CHECK: {{%\w+}} = OpPhi %v3float [[fcvt]] {{%\w+}} {{%\w+}} {{%\w+}}
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %output_color
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %MaterialParams "MaterialParams"
               OpMemberName %MaterialParams 0 "foo"
               OpName %materialParams "materialParams"
               OpName %output_color "output_color"
               OpMemberDecorate %MaterialParams 0 Offset 0
               OpDecorate %MaterialParams Block
               OpDecorate %materialParams DescriptorSet 0
               OpDecorate %materialParams Binding 5
               OpDecorate %output_color Location 0
               OpDecorate %57 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%MaterialParams = OpTypeStruct %float
%_ptr_Uniform_MaterialParams = OpTypePointer Uniform %MaterialParams
%materialParams = OpVariable %_ptr_Uniform_MaterialParams Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
    %float_0 = OpConstant %float 0
       %bool = OpTypeBool
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%output_color = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Output_float = OpTypePointer Output %float
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
  %float_0_5 = OpConstant %float 0.5
         %61 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
       %main = OpFunction %void None %3
          %5 = OpLabel
         %55 = OpAccessChain %_ptr_Uniform_float %materialParams %int_0
         %56 = OpLoad %float %55
         %57 = OpCompositeConstruct %v3float %56 %56 %56
         %31 = OpFOrdGreaterThan %bool %56 %float_0
               OpSelectionMerge %33 None
               OpBranchConditional %31 %32 %33
         %32 = OpLabel
         %37 = OpFMul %v3float %57 %61
               OpBranch %33
         %33 = OpLabel
         %58 = OpPhi %v3float %57 %5 %37 %32
         %45 = OpAccessChain %_ptr_Output_float %output_color %uint_0
         %46 = OpCompositeExtract %float %58 0
               OpStore %45 %46
         %48 = OpAccessChain %_ptr_Output_float %output_color %uint_1
         %49 = OpCompositeExtract %float %58 1
               OpStore %48 %49
         %51 = OpAccessChain %_ptr_Output_float %output_color %uint_2
         %52 = OpCompositeExtract %float %58 2
               OpStore %51 %52
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto result = SinglePassRunAndMatch<ConvertToHalfPass>(test, true);
  EXPECT_EQ(Pass::Status::SuccessWithChange, std::get<1>(result));
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
