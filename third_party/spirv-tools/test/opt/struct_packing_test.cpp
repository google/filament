// Copyright (c) 2024 Epic Games, Inc.
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
#include "source/opt/struct_packing_pass.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using StructPackingTest = PassTest<::testing::Test>;

TEST_F(StructPackingTest, PackSimpleStructStd140) {
  // #version 420
  //
  // layout(std140, binding = 0) uniform Globals {
  //   layout(offset = 16) vec3 a_xyz;
  //   float a_w;
  //   layout(offset = 128) vec3 b_xyz;
  //   int b_w;
  // };
  //
  // void main() {}
  const std::string spirv = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginLowerLeft
OpSource GLSL 420
OpName %main "main"
OpName %Globals "Globals"
OpMemberName %Globals 0 "a_xyz"
OpMemberName %Globals 1 "a_w"
OpMemberName %Globals 2 "b_xyz"
OpMemberName %Globals 3 "b_w"
OpName %_ ""
; CHECK: OpMemberDecorate %Globals 0 Offset 0
OpMemberDecorate %Globals 0 Offset 16
; CHECK: OpMemberDecorate %Globals 1 Offset 12
OpMemberDecorate %Globals 1 Offset 28
; CHECK: OpMemberDecorate %Globals 2 Offset 16
OpMemberDecorate %Globals 2 Offset 128
; CHECK: OpMemberDecorate %Globals 3 Offset 28
OpMemberDecorate %Globals 3 Offset 140
OpDecorate %Globals Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v3float = OpTypeVector %float 3
%int = OpTypeInt 32 1
%Globals = OpTypeStruct %v3float %float %v3float %int
%_ptr_Uniform_Globals = OpTypePointer Uniform %Globals
%_ = OpVariable %_ptr_Uniform_Globals Uniform
%main = OpFunction %void None %3
%5 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<StructPackingPass>(
      spirv, true, "Globals", StructPackingPass::PackingRules::Std140);
}

TEST_F(StructPackingTest, PackSimpleStructWithPaddingStd140) {
  // #version 420
  //
  // layout(std140, binding = 0) uniform Globals {
  //   layout(offset = 16) vec3 a_xyz;
  //   float a_w;
  //   float b_x_padding_yzw;
  //   layout(offset = 128) vec3 c_xyz;
  //   int c_w;
  // };
  //
  // void main() {}
  const std::string spirv = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginLowerLeft
OpSource GLSL 420
OpName %main "main"
OpName %Globals "Globals"
OpMemberName %Globals 0 "a_xyz"
OpMemberName %Globals 1 "a_w"
OpMemberName %Globals 2 "b_x_padding_yzw"
OpMemberName %Globals 3 "c_xyz"
OpMemberName %Globals 4 "c_w"
OpName %_ ""
; CHECK: OpMemberDecorate %Globals 0 Offset 0
OpMemberDecorate %Globals 0 Offset 16
; CHECK: OpMemberDecorate %Globals 1 Offset 12
OpMemberDecorate %Globals 1 Offset 28
; CHECK: OpMemberDecorate %Globals 2 Offset 16
OpMemberDecorate %Globals 2 Offset 32
; CHECK: OpMemberDecorate %Globals 3 Offset 32
OpMemberDecorate %Globals 3 Offset 128
; CHECK: OpMemberDecorate %Globals 4 Offset 44
OpMemberDecorate %Globals 4 Offset 140
OpDecorate %Globals Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v3float = OpTypeVector %float 3
%int = OpTypeInt 32 1
%Globals = OpTypeStruct %v3float %float %float %v3float %int
%_ptr_Uniform_Globals = OpTypePointer Uniform %Globals
%_ = OpVariable %_ptr_Uniform_Globals Uniform
%main = OpFunction %void None %3
%5 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<StructPackingPass>(
      spirv, true, "Globals", StructPackingPass::PackingRules::Std140);
}

TEST_F(StructPackingTest, PackSimpleScalarArrayStd140) {
  // #version 420
  //
  // layout(std140, binding = 0) uniform Globals {
  //   layout(offset = 16) float a[2];
  //   layout(offset = 128) float b[2]; // Must become offset 32 with std140
  // };
  //
  // void main() {}
  const std::string spirv = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginLowerLeft
OpSource GLSL 420
OpName %main "main"
OpName %Globals "Globals"
OpMemberName %Globals 0 "a"
OpMemberName %Globals 1 "b"
OpName %_ ""
OpDecorate %_arr_float_uint_2 ArrayStride 16
OpDecorate %_arr_float_uint_2_0 ArrayStride 16
; CHECK: OpMemberDecorate %Globals 0 Offset 0
OpMemberDecorate %Globals 0 Offset 16
; CHECK: OpMemberDecorate %Globals 1 Offset 32
OpMemberDecorate %Globals 1 Offset 128
OpDecorate %Globals Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%uint = OpTypeInt 32 0
%uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_arr_float_uint_2_0 = OpTypeArray %float %uint_2
%Globals = OpTypeStruct %_arr_float_uint_2 %_arr_float_uint_2_0
%_ptr_Uniform_Globals = OpTypePointer Uniform %Globals
%_ = OpVariable %_ptr_Uniform_Globals Uniform
%main = OpFunction %void None %3
%5 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<StructPackingPass>(
      spirv, true, "Globals", StructPackingPass::PackingRules::Std140);
}

TEST_F(StructPackingTest, PackSimpleScalarArrayStd430) {
  // #version 430
  //
  // layout(std430, binding = 0) buffer Globals {
  //   layout(offset = 16) float a[2];
  //   layout(offset = 128) float b[2]; // Must become offset 8 with std430
  // };
  //
  // void main() {}
  const std::string spirv = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginLowerLeft
OpSource GLSL 430
OpName %main "main"
OpName %Globals "Globals"
OpMemberName %Globals 0 "a"
OpMemberName %Globals 1 "b"
OpName %_ ""
OpDecorate %_arr_float_uint_2 ArrayStride 4
OpDecorate %_arr_float_uint_2_0 ArrayStride 4
; CHECK: OpMemberDecorate %Globals 0 Offset 0
OpMemberDecorate %Globals 0 Offset 16
; CHECK: OpMemberDecorate %Globals 1 Offset 8
OpMemberDecorate %Globals 1 Offset 128
OpDecorate %Globals BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%uint = OpTypeInt 32 0
%uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_arr_float_uint_2_0 = OpTypeArray %float %uint_2
%Globals = OpTypeStruct %_arr_float_uint_2 %_arr_float_uint_2_0
%_ptr_Uniform_Globals = OpTypePointer Uniform %Globals
%_ = OpVariable %_ptr_Uniform_Globals Uniform
%main = OpFunction %void None %3
%5 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<StructPackingPass>(
      spirv, true, "Globals", StructPackingPass::PackingRules::Std430);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
