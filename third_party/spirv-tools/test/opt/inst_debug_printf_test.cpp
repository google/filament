// Copyright (c) 2020-2022 Valve Corporation
// Copyright (c) 2020-2022 LunarG Inc.
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

// Debug Printf Instrumentation Tests.

#include <string>
#include <vector>

#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

static const std::string kOutputDecorations = R"(
; CHECK: OpDecorate [[output_buffer_type:%inst_printf_OutputBuffer]] Block
; CHECK: OpMemberDecorate [[output_buffer_type]] 0 Offset 0
; CHECK: OpMemberDecorate [[output_buffer_type]] 1 Offset 4
; CHECK: OpMemberDecorate [[output_buffer_type]] 2 Offset 8
; CHECK: OpDecorate [[output_buffer_var:%\w+]] DescriptorSet 7
; CHECK: OpDecorate [[output_buffer_var]] Binding 3
)";

static const std::string kOutputGlobals = R"(
; CHECK: [[output_buffer_type]] = OpTypeStruct %uint %uint %_runtimearr_uint
; CHECK: [[output_ptr_type:%\w+]] = OpTypePointer StorageBuffer [[output_buffer_type]]
; CHECK: [[output_buffer_var]] = OpVariable [[output_ptr_type]] StorageBuffer
)";

using InstDebugPrintfTest = PassTest<::testing::Test>;

TEST_F(InstDebugPrintfTest, V4Float32) {
  // SamplerState g_sDefault;
  // Texture2D g_tColor;
  //
  // struct PS_INPUT
  // {
  //   float2 vBaseTexCoord : TEXCOORD0;
  // };
  //
  // struct PS_OUTPUT
  // {
  //   float4 vDiffuse : SV_Target0;
  // };
  //
  // PS_OUTPUT MainPs(PS_INPUT i)
  // {
  //   PS_OUTPUT o;
  //
  //   o.vDiffuse.rgba = g_tColor.Sample(g_sDefault, (i.vBaseTexCoord.xy).xy);
  //   debugPrintfEXT("diffuse: %v4f", o.vDiffuse.rgba);
  //   return o;
  // }

  const std::string defs =
      R"(OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.DebugPrintf"
; CHECK-NOT: OpExtension "SPV_KHR_non_semantic_info"
; CHECK-NOT: %1 = OpExtInstImport "NonSemantic.DebugPrintf"
; CHECK: OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "MainPs" %3 %4
; CHECK: OpEntryPoint Fragment %2 "MainPs" %3 %4 %gl_FragCoord
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "Color is %vn"
)";

  // clang-format off
  const std::string decorates =
      R"(OpDecorate %6 DescriptorSet 0
OpDecorate %6 Binding 1
OpDecorate %7 DescriptorSet 0
OpDecorate %7 Binding 0
OpDecorate %3 Location 0
OpDecorate %4 Location 0
; CHECK: OpDecorate %_runtimearr_uint ArrayStride 4
)" + kOutputDecorations + R"(
; CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
)";

  const std::string globals =
      R"(%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%6 = OpVariable %_ptr_UniformConstant_13 UniformConstant
%15 = OpTypeSampler
%_ptr_UniformConstant_15 = OpTypePointer UniformConstant %15
%7 = OpVariable %_ptr_UniformConstant_15 UniformConstant
%17 = OpTypeSampledImage %13
%_ptr_Input_v2float = OpTypePointer Input %v2float
%3 = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%4 = OpVariable %_ptr_Output_v4float Output
; CHECK: %uint = OpTypeInt 32 0
; CHECK: %38 = OpTypeFunction %void %uint %uint %uint %uint %uint %uint
; CHECK: %_runtimearr_uint = OpTypeRuntimeArray %uint
)" + kOutputGlobals + R"(
; CHECK: %_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
; CHECK: %bool = OpTypeBool
; CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
; CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
; CHECK: %v4uint = OpTypeVector %uint 4
)";
  // clang-format on

  const std::string main =
      R"(%2 = OpFunction %void None %9
%20 = OpLabel
%21 = OpLoad %v2float %3
%22 = OpLoad %13 %6
%23 = OpLoad %15 %7
%24 = OpSampledImage %17 %22 %23
%25 = OpImageSampleImplicitLod %v4float %24 %21
%26 = OpExtInst %void %1 1 %5 %25
; CHECK-NOT: %26 = OpExtInst %void %1 1 %5 %25
; CHECK: %29 = OpCompositeExtract %float %25 0
; CHECK: %30 = OpBitcast %uint %29
; CHECK: %31 = OpCompositeExtract %float %25 1
; CHECK: %32 = OpBitcast %uint %31
; CHECK: %33 = OpCompositeExtract %float %25 2
; CHECK: %34 = OpBitcast %uint %33
; CHECK: %35 = OpCompositeExtract %float %25 3
; CHECK: %36 = OpBitcast %uint %35
; CHECK: %101 = OpFunctionCall %void %inst_printf_stream_write_6 %uint_36 %uint_5 %30 %32 %34 %36
; CHECK: OpBranch %102
; CHECK: %102 = OpLabel
OpStore %4 %25
OpReturn
OpFunctionEnd
)";

  const std::string output_func = R"(
; CHECK: %inst_printf_stream_write_6 = OpFunction %void None %38
; CHECK: [[param_1:%\w+]] = OpFunctionParameter %uint
; CHECK: [[param_2:%\w+]] = OpFunctionParameter %uint
; CHECK: [[param_3:%\w+]] = OpFunctionParameter %uint
; CHECK: [[param_4:%\w+]] = OpFunctionParameter %uint
; CHECK: [[param_5:%\w+]] = OpFunctionParameter %uint
; CHECK: [[param_6:%\w+]] = OpFunctionParameter %uint
; CHECK: {{%\w+}} = OpLabel
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_1
; CHECK: {{%\w+}} = OpAtomicIAdd %uint {{%\w+}} %uint_4 %uint_0 %uint_12
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_12
; CHECK: {{%\w+}} = OpArrayLength %uint %inst_printf_output_buffer 2
; CHECK: {{%\w+}} = OpULessThanEqual %bool {{%\w+}} {{%\w+}}
; CHECK: OpSelectionMerge {{%\w+}} None
; CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
; CHECK: {{%\w+}} = OpLabel
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_0
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} %uint_12
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_1
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} %uint_23
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_2
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} [[param_1]]
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_3
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} %uint_4
; CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
; CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
; CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_4
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} {{%\w+}}
; CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_5
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} {{%\w+}}
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_7
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} [[param_2]]
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_8
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} [[param_3]]
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_9
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} [[param_4]]
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_10
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} [[param_5]]
; CHECK: {{%\w+}} = OpIAdd %uint {{%\w+}} %uint_11
; CHECK: {{%\w+}} = OpAccessChain %_ptr_StorageBuffer_uint %inst_printf_output_buffer %uint_2 {{%\w+}}
; CHECK: OpStore {{%\w+}} [[param_6]]
; CHECK: OpBranch {{%\w+}}
; CHECK: {{%\w+}} = OpLabel
; CHECK: OpReturn
; CHECK: OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstDebugPrintfPass>(
      defs + decorates + globals + main + output_func, true);
}

// TODO(greg-lunarg): Add tests to verify handling of these cases:
//
//   Compute shader
//   Geometry shader
//   Tessellation control shader
//   Tessellation eval shader
//   Vertex shader

}  // namespace
}  // namespace opt
}  // namespace spvtools
