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

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using ElimDeadIOComponentsTest = PassTest<::testing::Test>;

TEST_F(ElimDeadIOComponentsTest, ElimOneConstantIndex) {
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
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, false);
}

TEST_F(ElimDeadIOComponentsTest, ElimOneConstantIndexInBounds) {
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
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, false);
}

TEST_F(ElimDeadIOComponentsTest, ElimTwoConstantIndices) {
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
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, false);
}

TEST_F(ElimDeadIOComponentsTest, NoElimMaxConstantIndex) {
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
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, false);
}

TEST_F(ElimDeadIOComponentsTest, NoElimNonConstantIndex) {
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
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, false);
}

TEST_F(ElimDeadIOComponentsTest, NoElimNonIndexedAccessChain) {
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
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, false);
}

TEST_F(ElimDeadIOComponentsTest, ElimStructMember) {
  // Should eliminate uv
  //
  // #version 450
  //
  // in Vertex {
  //   vec4 Cd;
  //   vec2 uv;
  // } iVert;
  //
  // out vec4 fragColor;
  //
  // void main()
  // {
  //   vec4 color = vec4(iVert.Cd);
  //   fragColor = color;
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %iVert %fragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %Vertex "Vertex"
               OpMemberName %Vertex 0 "Cd"
               OpMemberName %Vertex 1 "uv"
               OpName %iVert "iVert"
               OpName %fragColor "fragColor"
               OpDecorate %Vertex Block
               OpDecorate %iVert Location 0
               OpDecorate %fragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
     %Vertex = OpTypeStruct %v4float %v2float
; CHECK: %Vertex = OpTypeStruct %v4float %v2float
; CHECK: %Vertex_0 = OpTypeStruct %v4float
%_ptr_Input_Vertex = OpTypePointer Input %Vertex
; CHECK: [[pty:%\w+]] = OpTypePointer Input %Vertex_0
      %iVert = OpVariable %_ptr_Input_Vertex Input
; CHECK: %iVert = OpVariable [[pty]] Input
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %fragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpAccessChain %_ptr_Input_v4float %iVert %int_0
         %18 = OpLoad %v4float %17
               OpStore %fragColor %18
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, false);
}

TEST_F(ElimDeadIOComponentsTest, ElimOutputStructMember) {
  // Should eliminate uv from Vertex and all but gl_Position from gl_PerVertex
  //
  // #version 450
  //
  // out Vertex {
  //   vec4 Cd;
  //   vec2 uv;
  // } oVert;
  //
  // in vec3 P;
  //
  // void main()
  // {
  //   vec4 worldSpacePos = vec4(P, 1);
  //   oVert.Cd = vec4(1, 0.5, 0, 1);
  //   gl_Position = worldSpacePos;
  // }

  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %P %oVert %_
               OpSource GLSL 450
               OpName %main "main"
               OpName %P "P"
               OpName %Vertex "Vertex"
               OpMemberName %Vertex 0 "Cd"
               OpMemberName %Vertex 1 "uv"
               OpName %oVert "oVert"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""
               OpDecorate %P Location 0
               OpDecorate %Vertex Block
               OpDecorate %oVert Location 0
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
          %P = OpVariable %_ptr_Input_v3float Input
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
     %Vertex = OpTypeStruct %v4float %v2float
%_ptr_Output_Vertex = OpTypePointer Output %Vertex
      %oVert = OpVariable %_ptr_Output_Vertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
  %float_0_5 = OpConstant %float 0.5
    %float_0 = OpConstant %float 0
         %27 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
; CHECK: %Vertex = OpTypeStruct %v4float %v2float
; CHECK: %gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
; CHECK: %_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
; CHECK: [[sty:%\w+]] = OpTypeStruct %v4float
; CHECK: [[pty:%\w+]] = OpTypePointer Output [[sty]]
; CHECK: %oVert = OpVariable [[pty]] Output
; CHECK: [[sty2:%\w+]] = OpTypeStruct %v4float
; CHECK: [[pty2:%\w+]] = OpTypePointer Output [[sty2]]
; CHECK: %_ = OpVariable [[pty2]] Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %v3float %P
         %15 = OpCompositeExtract %float %13 0
         %16 = OpCompositeExtract %float %13 1
         %17 = OpCompositeExtract %float %13 2
         %18 = OpCompositeConstruct %v4float %15 %16 %17 %float_1
         %29 = OpAccessChain %_ptr_Output_v4float %oVert %int_0
               OpStore %29 %27
         %37 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %37 %18
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Output, false);
}

TEST_F(ElimDeadIOComponentsTest, ElimOutputArrayMembers) {
  // Should reduce to uv[2]
  //
  // #version 450
  //
  // layout(location = 0) out vec2 uv[8];
  //
  // void main()
  // {
  //     uv[1] = vec2(1, 0.5);
  // }

  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %uv
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %uv "uv"
               OpDecorate %uv Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_v2float_uint_8 = OpTypeArray %v2float %uint_8
%_ptr_Output__arr_v2float_uint_8 = OpTypePointer Output %_arr_v2float_uint_8
         %uv = OpVariable %_ptr_Output__arr_v2float_uint_8 Output
;CHECK-NOT:         %uv = OpVariable %_ptr_Output__arr_v2float_uint_8 Output
;CHECK:             %uv = OpVariable %_ptr_Output__arr_v2float_uint_2 Output
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
  %float_0_5 = OpConstant %float 0.5
         %17 = OpConstantComposite %v2float %float_1 %float_0_5
%_ptr_Output_v2float = OpTypePointer Output %v2float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v2float %uv %int_1
               OpStore %19 %17
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Output, false);
}

TEST_F(ElimDeadIOComponentsTest, VertexOnly) {
  // Should NOT eliminate uv
  //
  // #version 450
  //
  // in Vertex {
  //   vec4 Cd;
  //   vec2 uv;
  // } iVert;
  //
  // out vec4 fragColor;
  //
  // void main()
  // {
  //   vec4 color = vec4(iVert.Cd);
  //   fragColor = color;
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %iVert %fragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %Vertex "Vertex"
               OpMemberName %Vertex 0 "Cd"
               OpMemberName %Vertex 1 "uv"
               OpName %iVert "iVert"
               OpName %fragColor "fragColor"
               OpDecorate %Vertex Block
               OpDecorate %iVert Location 0
               OpDecorate %fragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
     %Vertex = OpTypeStruct %v4float %v2float
; CHECK: %Vertex = OpTypeStruct %v4float %v2float
%_ptr_Input_Vertex = OpTypePointer Input %Vertex
; CHECK: %_ptr_Input_Vertex = OpTypePointer Input %Vertex
      %iVert = OpVariable %_ptr_Input_Vertex Input
; CHECK: %iVert = OpVariable %_ptr_Input_Vertex Input
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %fragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpAccessChain %_ptr_Input_v4float %iVert %int_0
         %18 = OpLoad %v4float %17
               OpStore %fragColor %18
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, true);
}

TEST_F(ElimDeadIOComponentsTest, TescInput) {
  // Eliminate PointSize, ClipDistance, CullDistance from gl_in[]
  //
  // #version 450
  //
  // layout (vertices = 4) out;
  //
  // void main()
  // {
  //     vec4 pos = gl_in[gl_InvocationID].gl_Position;
  //     gl_out[gl_InvocationID].gl_Position = pos;
  // }
  const std::string text = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_in %gl_InvocationID %gl_out
               OpExecutionMode %main OutputVertices 4
               OpSource GLSL 450
               OpName %main "main"
               OpName %pos "pos"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_in "gl_in"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %gl_PerVertex_0 "gl_PerVertex"
               OpMemberName %gl_PerVertex_0 0 "gl_Position"
               OpName %gl_out "gl_out"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn Position
               OpDecorate %gl_PerVertex_0 Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
    %uint_32 = OpConstant %uint 32
%_arr_gl_PerVertex_uint_32 = OpTypeArray %gl_PerVertex %uint_32
%_ptr_Input__arr_gl_PerVertex_uint_32 = OpTypePointer Input %_arr_gl_PerVertex_uint_32
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_uint_32 Input
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
      %int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_PerVertex_0 = OpTypeStruct %v4float
     %uint_4 = OpConstant %uint 4
%_arr_gl_PerVertex_0_uint_4 = OpTypeArray %gl_PerVertex_0 %uint_4
%_ptr_Output__arr_gl_PerVertex_0_uint_4 = OpTypePointer Output %_arr_gl_PerVertex_0_uint_4
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_0_uint_4 Output
%_ptr_Output_v4float = OpTypePointer Output %v4float
; CHECK: %gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
; CHECK: [[sty:%\w+]] = OpTypeStruct %v4float
; CHECK: [[asty:%\w+]] = OpTypeArray [[sty]] %uint_32
; CHECK: [[pasty:%\w+]] = OpTypePointer Input [[asty]]
; CHECK: %gl_in = OpVariable [[pasty]] Input
       %main = OpFunction %void None %3
          %5 = OpLabel
        %pos = OpVariable %_ptr_Function_v4float Function
         %21 = OpLoad %int %gl_InvocationID
         %24 = OpAccessChain %_ptr_Input_v4float %gl_in %21 %int_0
         %25 = OpLoad %v4float %24
               OpStore %pos %25
         %31 = OpLoad %int %gl_InvocationID
         %32 = OpLoad %v4float %pos
         %34 = OpAccessChain %_ptr_Output_v4float %gl_out %31 %int_0
               OpStore %34 %32
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, false);
}

TEST_F(ElimDeadIOComponentsTest, TescOutput) {
  // Eliminate PointSize, ClipDistance, CullDistance from gl_out[]
  //
  // #version 450
  //
  // layout (vertices = 4) out;
  //
  // void main()
  // {
  //     vec4 pos = gl_in[gl_InvocationID].gl_Position;
  //     gl_out[gl_InvocationID].gl_Position = pos;
  // }
  const std::string text = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_in %gl_InvocationID %gl_out
               OpExecutionMode %main OutputVertices 4
               OpSource GLSL 450
               OpName %main "main"
               OpName %pos "pos"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpName %gl_in "gl_in"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %gl_PerVertex_0 "gl_PerVertex"
               OpMemberName %gl_PerVertex_0 0 "gl_Position"
               OpMemberName %gl_PerVertex_0 1 "gl_PointSize"
               OpMemberName %gl_PerVertex_0 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex_0 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex_0 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex_0 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex_0 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex_0 Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float 
    %uint_32 = OpConstant %uint 32
%_arr_gl_PerVertex_uint_32 = OpTypeArray %gl_PerVertex %uint_32
%_ptr_Input__arr_gl_PerVertex_uint_32 = OpTypePointer Input %_arr_gl_PerVertex_uint_32
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_uint_32 Input
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
      %int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_PerVertex_0 = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
     %uint_4 = OpConstant %uint 4
%_arr_gl_PerVertex_0_uint_4 = OpTypeArray %gl_PerVertex_0 %uint_4
%_ptr_Output__arr_gl_PerVertex_0_uint_4 = OpTypePointer Output %_arr_gl_PerVertex_0_uint_4
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_0_uint_4 Output
%_ptr_Output_v4float = OpTypePointer Output %v4float
; CHECK: [[sty:%\w+]] = OpTypeStruct %v4float
; CHECK: [[asty:%\w+]] = OpTypeArray [[sty]] %uint_4
; CHECK: [[pasty:%\w+]] = OpTypePointer Output [[asty]]
; CHECK: %gl_out = OpVariable [[pasty]] Output
       %main = OpFunction %void None %3
          %5 = OpLabel
        %pos = OpVariable %_ptr_Function_v4float Function
         %21 = OpLoad %int %gl_InvocationID
         %24 = OpAccessChain %_ptr_Input_v4float %gl_in %21 %int_0
         %25 = OpLoad %v4float %24
               OpStore %pos %25
         %31 = OpLoad %int %gl_InvocationID
         %32 = OpLoad %v4float %pos
         %34 = OpAccessChain %_ptr_Output_v4float %gl_out %31 %int_0
               OpStore %34 %32
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Output, false);
}

TEST_F(ElimDeadIOComponentsTest, TeseInput) {
  // Eliminate PointSize, ClipDistance, CullDistance from gl_in[]
  //
  // #version 450
  //
  // layout(triangles, ccw) in;
  // layout(fractional_odd_spacing) in;
  // layout(point_mode) in;
  //
  // void main()
  // {
  //     vec4 p = gl_in[1].gl_Position;
  //     gl_Position = p;
  // }
  const std::string text = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %gl_in %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main SpacingFractionalOdd
               OpExecutionMode %main VertexOrderCcw
               OpExecutionMode %main PointMode
               OpSource GLSL 450
               OpName %main "main"
               OpName %p "p"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_in "gl_in"
               OpName %gl_PerVertex_0 "gl_PerVertex"
               OpMemberName %gl_PerVertex_0 0 "gl_Position"
               OpName %_ ""
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn Position
               OpDecorate %gl_PerVertex_0 Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
    %uint_32 = OpConstant %uint 32
%_arr_gl_PerVertex_uint_32 = OpTypeArray %gl_PerVertex %uint_32
%_ptr_Input__arr_gl_PerVertex_uint_32 = OpTypePointer Input %_arr_gl_PerVertex_uint_32
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_uint_32 Input
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_PerVertex_0 = OpTypeStruct %v4float
%_ptr_Output_gl_PerVertex_0 = OpTypePointer Output %gl_PerVertex_0
          %_ = OpVariable %_ptr_Output_gl_PerVertex_0 Output
%_ptr_Output_v4float = OpTypePointer Output %v4float
; CHECK: %gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
; CHECK: [[sty:%\w+]] = OpTypeStruct %v4float
; CHECK: [[asty:%\w+]] = OpTypeArray [[sty]] %uint_32
; CHECK: [[pasty:%\w+]] = OpTypePointer Input [[asty]]
; CHECK: %gl_in = OpVariable [[pasty]] Input
       %main = OpFunction %void None %3
          %5 = OpLabel
          %p = OpVariable %_ptr_Function_v4float Function
         %22 = OpAccessChain %_ptr_Input_v4float %gl_in %int_1 %int_0
         %23 = OpLoad %v4float %22
               OpStore %p %23
         %27 = OpLoad %v4float %p
         %29 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %29 %27
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, false);
}

TEST_F(ElimDeadIOComponentsTest, TeseOutput) {
  // Eliminate PointSize, ClipDistance, CullDistance from gl_out
  //
  // #version 450
  //
  // layout(triangles, ccw) in;
  // layout(fractional_odd_spacing) in;
  // layout(point_mode) in;
  //
  // void main()
  // {
  //     vec4 p = gl_in[1].gl_Position;
  //     gl_Position = p;
  // }
  const std::string text = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %gl_in %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main SpacingFractionalOdd
               OpExecutionMode %main VertexOrderCcw
               OpExecutionMode %main PointMode
               OpSource GLSL 450
               OpName %main "main"
               OpName %p "p"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpName %gl_in "gl_in"
               OpName %gl_PerVertex_0 "gl_PerVertex"
               OpMemberName %gl_PerVertex_0 0 "gl_Position"
               OpMemberName %gl_PerVertex_0 1 "gl_PointSize"
               OpMemberName %gl_PerVertex_0 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex_0 3 "gl_CullDistance"
               OpName %_ ""
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex_0 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex_0 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex_0 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex_0 Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float
    %uint_32 = OpConstant %uint 32
%_arr_gl_PerVertex_uint_32 = OpTypeArray %gl_PerVertex %uint_32
%_ptr_Input__arr_gl_PerVertex_uint_32 = OpTypePointer Input %_arr_gl_PerVertex_uint_32
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_uint_32 Input
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_PerVertex_0 = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex_0 = OpTypePointer Output %gl_PerVertex_0
          %_ = OpVariable %_ptr_Output_gl_PerVertex_0 Output
%_ptr_Output_v4float = OpTypePointer Output %v4float
; CHECK: %_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
; CHECK: %_ = OpVariable %_ptr_Output_gl_PerVertex Output
       %main = OpFunction %void None %3
          %5 = OpLabel
          %p = OpVariable %_ptr_Function_v4float Function
         %22 = OpAccessChain %_ptr_Input_v4float %gl_in %int_1 %int_0
         %23 = OpLoad %v4float %22
               OpStore %p %23
         %27 = OpLoad %v4float %p
         %29 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %29 %27
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Output, false);
}

TEST_F(ElimDeadIOComponentsTest, GeomInput) {
  // Eliminate PointSize, ClipDistance, CullDistance from gl_in[]
  //
  // #version 450
  //
  // layout(triangle_strip, max_vertices = 3) out;
  // layout(triangles) in;
  //
  // void main()
  // {
  //         for (int i = 0; i < 3; i++)
  //         {
  //                 gl_Position = gl_in[i].gl_Position;
  //                 EmitVertex();
  //         }
  //         EndPrimitive();
  // }
  const std::string text = R"(
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %_ %gl_in
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 3
               OpSource GLSL 460
               OpName %main "main"
               OpName %i "i"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpName %_ ""
               OpName %gl_PerVertex_0 "gl_PerVertex"
               OpMemberName %gl_PerVertex_0 0 "gl_Position"
               OpMemberName %gl_PerVertex_0 1 "gl_PointSize"
               OpMemberName %gl_PerVertex_0 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex_0 3 "gl_CullDistance"
               OpName %gl_in "gl_in"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex_0 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex_0 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex_0 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex_0 Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
       %bool = OpTypeBool
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
%gl_PerVertex_0 = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
     %uint_3 = OpConstant %uint 3
%_arr_gl_PerVertex_0_uint_3 = OpTypeArray %gl_PerVertex_0 %uint_3
%_ptr_Input__arr_gl_PerVertex_0_uint_3 = OpTypePointer Input %_arr_gl_PerVertex_0_uint_3
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_0_uint_3 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
; CHECK: [[asty:%\w+]] = OpTypeArray %gl_PerVertex %uint_3
; CHECK: [[pasty:%\w+]] = OpTypePointer Input [[asty]]
; CHECK: %gl_in = OpVariable [[pasty]] Input
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
         %18 = OpSLessThan %bool %15 %int_3
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
         %32 = OpLoad %int %i
         %34 = OpAccessChain %_ptr_Input_v4float %gl_in %32 %int_0
         %35 = OpLoad %v4float %34
         %37 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %37 %35
               OpEmitVertex
               OpBranch %13
         %13 = OpLabel
         %38 = OpLoad %int %i
         %40 = OpIAdd %int %38 %int_1
               OpStore %i %40
               OpBranch %10
         %12 = OpLabel
               OpEndPrimitive
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Input, false);
}

TEST_F(ElimDeadIOComponentsTest, GeomOutput) {
  // Eliminate PointSize, ClipDistance, CullDistance from gl_out
  //
  // #version 450
  //
  // layout(triangle_strip, max_vertices = 3) out;
  // layout(triangles) in;
  //
  // void main()
  // {
  //         for (int i = 0; i < 3; i++)
  //         {
  //                 gl_Position = gl_in[i].gl_Position;
  //                 EmitVertex();
  //         }
  //         EndPrimitive();
  // }
  const std::string text = R"(
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %_ %gl_in
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 3
               OpSource GLSL 460
               OpName %main "main"
               OpName %i "i"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""
               OpName %gl_PerVertex_0 "gl_PerVertex"
               OpMemberName %gl_PerVertex_0 0 "gl_Position"
               OpName %gl_in "gl_in"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn Position
               OpDecorate %gl_PerVertex_0 Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
       %bool = OpTypeBool
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
%gl_PerVertex_0 = OpTypeStruct %v4float
     %uint_3 = OpConstant %uint 3
%_arr_gl_PerVertex_0_uint_3 = OpTypeArray %gl_PerVertex_0 %uint_3
%_ptr_Input__arr_gl_PerVertex_0_uint_3 = OpTypePointer Input %_arr_gl_PerVertex_0_uint_3
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_0_uint_3 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
; CHECK: %_ptr_Output_gl_PerVertex_0 = OpTypePointer Output %gl_PerVertex_0
; CHECK: %_ = OpVariable %_ptr_Output_gl_PerVertex_0 Output
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
         %18 = OpSLessThan %bool %15 %int_3
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
         %32 = OpLoad %int %i
         %34 = OpAccessChain %_ptr_Input_v4float %gl_in %32 %int_0
         %35 = OpLoad %v4float %34
         %37 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %37 %35
               OpEmitVertex
               OpBranch %13
         %13 = OpLabel
         %38 = OpLoad %int %i
         %40 = OpIAdd %int %38 %int_1
               OpStore %i %40
               OpBranch %10
         %12 = OpLabel
               OpEndPrimitive
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<EliminateDeadIOComponentsPass>(
      text, true, spv::StorageClass::Output, false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
