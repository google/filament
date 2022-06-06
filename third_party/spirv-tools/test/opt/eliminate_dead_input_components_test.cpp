// Copyright (c) 2022 The Khronos Group Inc.
// Copyright (c) 2022 LunarG Inc.
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

#include <vector>

#include "gmock/gmock.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using ElimDeadInputComponentsTest = PassTest<::testing::Test>;

TEST_F(ElimDeadInputComponentsTest, ElimOneConstantIndex) {
  // Should reduce to uv[2]
  //
  // #version 450
  //
  // layout(location = 0) in vec4 uv[8];
  //
  // out gl_PerVertex {
  //     vec4 gl_Position;
  // };
  //
  // void main()
  // {
  //     gl_Position = uv[1];
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %uv
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpName %_ ""
               OpName %uv "uv"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpDecorate %gl_PerVertex Block
               OpDecorate %uv Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%gl_PerVertex = OpTypeStruct %v4float
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%_ptr_Input__arr_v4float_uint_8 = OpTypePointer Input %_arr_v4float_uint_8
           %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
      %int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
;CHECK-NOT: %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
;CHECK:     %uv = OpVariable %_ptr_Input__arr_v4float_uint_2 Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpAccessChain %_ptr_Input_v4float %uv %int_1
         %21 = OpLoad %v4float %20
         %23 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %23 %21
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadInputComponentsPass>(text, true);
}

TEST_F(ElimDeadInputComponentsTest, ElimOneConstantIndexInBounds) {
  // Same as ElimOneConstantIndex but with OpInBoundsAccessChain
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %uv
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpName %_ ""
               OpName %uv "uv"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpDecorate %gl_PerVertex Block
               OpDecorate %uv Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%gl_PerVertex = OpTypeStruct %v4float
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%_ptr_Input__arr_v4float_uint_8 = OpTypePointer Input %_arr_v4float_uint_8
           %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
      %int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
;CHECK-NOT: %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
;CHECK:     %uv = OpVariable %_ptr_Input__arr_v4float_uint_2 Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpInBoundsAccessChain %_ptr_Input_v4float %uv %int_1
         %21 = OpLoad %v4float %20
         %23 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %23 %21
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadInputComponentsPass>(text, true);
}

TEST_F(ElimDeadInputComponentsTest, ElimTwoConstantIndices) {
  // Should reduce to uv[4]
  //
  // #version 450
  //
  // layout(location = 0) in vec4 uv[8];
  //
  // out gl_PerVertex {
  //     vec4 gl_Position;
  // };
  //
  // void main()
  // {
  //     gl_Position = uv[1] + uv[3];
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %uv
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpName %_ ""
               OpName %uv "uv"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpDecorate %gl_PerVertex Block
               OpDecorate %uv Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%gl_PerVertex = OpTypeStruct %v4float
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%_ptr_Input__arr_v4float_uint_8 = OpTypePointer Input %_arr_v4float_uint_8
         %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
      %int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
      %int_3 = OpConstant %int 3
%_ptr_Output_v4float = OpTypePointer Output %v4float
;CHECK-NOT: %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
;CHECK:     %uv = OpVariable %_ptr_Input__arr_v4float_uint_4 Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpAccessChain %_ptr_Input_v4float %uv %int_1
         %21 = OpLoad %v4float %20
         %23 = OpAccessChain %_ptr_Input_v4float %uv %int_3
         %24 = OpLoad %v4float %23
         %25 = OpFAdd %v4float %21 %24
         %27 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %27 %25
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadInputComponentsPass>(text, true);
}

TEST_F(ElimDeadInputComponentsTest, NoElimMaxConstantIndex) {
  // Should not reduce uv[8] because of max index of 7
  //
  // #version 450
  //
  // layout(location = 0) in vec4 uv[8];
  //
  // out gl_PerVertex {
  //     vec4 gl_Position;
  // };
  //
  // void main()
  // {
  //     gl_Position = uv[1] + uv[7];
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %uv
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpName %_ ""
               OpName %uv "uv"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpDecorate %gl_PerVertex Block
               OpDecorate %uv Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%gl_PerVertex = OpTypeStruct %v4float
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%_ptr_Input__arr_v4float_uint_8 = OpTypePointer Input %_arr_v4float_uint_8
         %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
      %int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
      %int_7 = OpConstant %int 7
%_ptr_Output_v4float = OpTypePointer Output %v4float
;CHECK:     %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpAccessChain %_ptr_Input_v4float %uv %int_1
         %21 = OpLoad %v4float %20
         %23 = OpAccessChain %_ptr_Input_v4float %uv %int_7
         %24 = OpLoad %v4float %23
         %25 = OpFAdd %v4float %21 %24
         %27 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %27 %25
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadInputComponentsPass>(text, true);
}

TEST_F(ElimDeadInputComponentsTest, NoElimNonConstantIndex) {
  // Should not reduce uv[8] because of non-constant index of ui
  //
  // #version 450
  //
  // layout(location = 0) in vec4 uv[8];
  //
  // out gl_PerVertex {
  //     vec4 gl_Position;
  // };
  //
  // uniform ubname {
  //     int ui;
  // } ubinst;
  //
  // void main()
  // {
  //     gl_Position = uv[1] + uv[ubinst.ui];
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %uv %ubinst
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpName %_ ""
               OpName %uv "uv"
               OpName %ubname "ubname"
               OpMemberName %ubname 0 "ui"
               OpName %ubinst "ubinst"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpDecorate %gl_PerVertex Block
               OpDecorate %uv Location 0
               OpMemberDecorate %ubname 0 Offset 0
               OpDecorate %ubname Block
               OpDecorate %ubinst DescriptorSet 0
               OpDecorate %ubinst Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%gl_PerVertex = OpTypeStruct %v4float
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%_ptr_Input__arr_v4float_uint_8 = OpTypePointer Input %_arr_v4float_uint_8
         %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
      %int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
     %ubname = OpTypeStruct %int
%_ptr_Uniform_ubname = OpTypePointer Uniform %ubname
     %ubinst = OpVariable %_ptr_Uniform_ubname Uniform
%_ptr_Uniform_int = OpTypePointer Uniform %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
;CHECK:  %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpAccessChain %_ptr_Input_v4float %uv %int_1
         %21 = OpLoad %v4float %20
         %26 = OpAccessChain %_ptr_Uniform_int %ubinst %int_0
         %27 = OpLoad %int %26
         %28 = OpAccessChain %_ptr_Input_v4float %uv %27
         %29 = OpLoad %v4float %28
         %30 = OpFAdd %v4float %21 %29
         %32 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %32 %30
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadInputComponentsPass>(text, true);
}

TEST_F(ElimDeadInputComponentsTest, NoElimNonIndexedAccessChain) {
  // Should not change due to non-indexed access chain
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %uv
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpName %_ ""
               OpName %uv "uv"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpDecorate %gl_PerVertex Block
               OpDecorate %uv Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%gl_PerVertex = OpTypeStruct %v4float
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%_ptr_Input__arr_v4float_uint_8 = OpTypePointer Input %_arr_v4float_uint_8
           %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
      %int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
;CHECK:  %uv = OpVariable %_ptr_Input__arr_v4float_uint_8 Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpAccessChain %_ptr_Input__arr_v4float_uint_8 %uv
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadInputComponentsPass>(text, true);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
