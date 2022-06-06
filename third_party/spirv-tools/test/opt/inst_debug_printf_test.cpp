// Copyright (c) 2020 Valve Corporation
// Copyright (c) 2020 LunarG Inc.
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

  const std::string decorates =
      R"(OpDecorate %6 DescriptorSet 0
OpDecorate %6 Binding 1
OpDecorate %7 DescriptorSet 0
OpDecorate %7 Binding 0
OpDecorate %3 Location 0
OpDecorate %4 Location 0
; CHECK: OpDecorate %_runtimearr_uint ArrayStride 4
; CHECK: OpDecorate %_struct_47 Block
; CHECK: OpMemberDecorate %_struct_47 0 Offset 0
; CHECK: OpMemberDecorate %_struct_47 1 Offset 4
; CHECK: OpDecorate %49 DescriptorSet 7
; CHECK: OpDecorate %49 Binding 3
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
; CHECK: %_struct_47 = OpTypeStruct %uint %_runtimearr_uint
; CHECK: %_ptr_StorageBuffer__struct_47 = OpTypePointer StorageBuffer %_struct_47
; CHECK: %49 = OpVariable %_ptr_StorageBuffer__struct_47 StorageBuffer
; CHECK: %_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
; CHECK: %bool = OpTypeBool
; CHECK: %_ptr_Input_v4float = OpTypePointer Input %v4float
; CHECK: %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
; CHECK: %v4uint = OpTypeVector %uint 4
)";

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
; CHECK: %101 = OpFunctionCall %void %37 %uint_36 %uint_5 %30 %32 %34 %36
; CHECK: OpBranch %102
; CHECK: %102 = OpLabel
OpStore %4 %25
OpReturn
OpFunctionEnd
)";

  const std::string output_func =
      R"(; CHECK: %37 = OpFunction %void None %38
; CHECK: %39 = OpFunctionParameter %uint
; CHECK: %40 = OpFunctionParameter %uint
; CHECK: %41 = OpFunctionParameter %uint
; CHECK: %42 = OpFunctionParameter %uint
; CHECK: %43 = OpFunctionParameter %uint
; CHECK: %44 = OpFunctionParameter %uint
; CHECK: %45 = OpLabel
; CHECK: %52 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_0
; CHECK: %55 = OpAtomicIAdd %uint %52 %uint_4 %uint_0 %uint_12
; CHECK: %56 = OpIAdd %uint %55 %uint_12
; CHECK: %57 = OpArrayLength %uint %49 1
; CHECK: %59 = OpULessThanEqual %bool %56 %57
; CHECK: OpSelectionMerge %60 None
; CHECK: OpBranchConditional %59 %61 %60
; CHECK: %61 = OpLabel
; CHECK: %62 = OpIAdd %uint %55 %uint_0
; CHECK: %64 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %62
; CHECK: OpStore %64 %uint_12
; CHECK: %66 = OpIAdd %uint %55 %uint_1
; CHECK: %67 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %66
; CHECK: OpStore %67 %uint_23
; CHECK: %69 = OpIAdd %uint %55 %uint_2
; CHECK: %70 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %69
; CHECK: OpStore %70 %39
; CHECK: %72 = OpIAdd %uint %55 %uint_3
; CHECK: %73 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %72
; CHECK: OpStore %73 %uint_4
; CHECK: %76 = OpLoad %v4float %gl_FragCoord
; CHECK: %78 = OpBitcast %v4uint %76
; CHECK: %79 = OpCompositeExtract %uint %78 0
; CHECK: %80 = OpIAdd %uint %55 %uint_4
; CHECK: %81 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %80
; CHECK: OpStore %81 %79
; CHECK: %82 = OpCompositeExtract %uint %78 1
; CHECK: %83 = OpIAdd %uint %55 %uint_5
; CHECK: %84 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %83
; CHECK: OpStore %84 %82
; CHECK: %86 = OpIAdd %uint %55 %uint_7
; CHECK: %87 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %86
; CHECK: OpStore %87 %40
; CHECK: %89 = OpIAdd %uint %55 %uint_8
; CHECK: %90 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %89
; CHECK: OpStore %90 %41
; CHECK: %92 = OpIAdd %uint %55 %uint_9
; CHECK: %93 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %92
; CHECK: OpStore %93 %42
; CHECK: %95 = OpIAdd %uint %55 %uint_10
; CHECK: %96 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %95
; CHECK: OpStore %96 %43
; CHECK: %98 = OpIAdd %uint %55 %uint_11
; CHECK: %99 = OpAccessChain %_ptr_StorageBuffer_uint %49 %uint_1 %98
; CHECK: OpStore %99 %44
; CHECK: OpBranch %60
; CHECK: %60 = OpLabel
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
