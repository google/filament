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

// Relax float ops tests

#include <string>
#include <vector>

#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using RelaxFloatOpsTest = PassTest<::testing::Test>;

TEST_F(RelaxFloatOpsTest, RelaxFloatOpsBasic) {
  // All float result instructions in functions should be relaxed
  // clang-format off
  //
  // SamplerState       g_sSamp : register(s0);
  // uniform Texture1D <float4> g_tTex1df4 : register(t0);
  //
  // struct PS_INPUT
  // {
  //   float Tex0 : TEXCOORD0;
  //   float Tex1 : TEXCOORD1;
  // };
  //
  // struct PS_OUTPUT
  // {
  //   float4 Color : SV_Target0;
  // };
  //
  // PS_OUTPUT main(PS_INPUT i)
  // {
  //   PS_OUTPUT psout;
  //   float4 txval10 = g_tTex1df4.Sample(g_sSamp, i.Tex0);
  //   float4 txval11 = g_tTex1df4.Sample(g_sSamp, i.Tex1);
  //   float4 t = txval10 + txval11;
  //   float4 t2 = t / 2.0;
  //   psout.Color = t2;
  //   return psout;
  // }
  // clang-format on

  const std::string defs0 =
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
OpName %i_Tex0 "i.Tex0"
OpName %i_Tex1 "i.Tex1"
OpName %_entryPointOutput_Color "@entryPointOutput.Color"
OpDecorate %g_tTex1df4 DescriptorSet 0
OpDecorate %g_tTex1df4 Binding 0
OpDecorate %g_sSamp DescriptorSet 0
OpDecorate %g_sSamp Binding 0
OpDecorate %i_Tex0 Location 0
OpDecorate %i_Tex1 Location 1
OpDecorate %_entryPointOutput_Color Location 0
)";

  const std::string defs1 =
      R"(%void = OpTypeVoid
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
%_ptr_Input_float = OpTypePointer Input %float
%i_Tex0 = OpVariable %_ptr_Input_float Input
%i_Tex1 = OpVariable %_ptr_Input_float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
%float_0_5 = OpConstant %float 0.5
%116 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
)";

  const std::string relax_decos =
      R"(OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
)";

  const std::string func_orig =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%60 = OpLoad %float %i_Tex0
%63 = OpLoad %float %i_Tex1
%77 = OpLoad %17 %g_tTex1df4
%78 = OpLoad %21 %g_sSamp
%79 = OpSampledImage %25 %77 %78
%82 = OpImageSampleImplicitLod %v4float %79 %60
%83 = OpLoad %17 %g_tTex1df4
%84 = OpLoad %21 %g_sSamp
%85 = OpSampledImage %25 %83 %84
%88 = OpImageSampleImplicitLod %v4float %85 %63
%91 = OpFAdd %v4float %82 %88
%94 = OpFMul %v4float %91 %116
OpStore %_entryPointOutput_Color %94
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<RelaxFloatOpsPass>(
      defs0 + defs1 + func_orig, defs0 + relax_decos + defs1 + func_orig, true,
      true);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
