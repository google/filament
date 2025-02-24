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

#include <unordered_set>

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using ElimDeadOutputStoresTest = PassTest<::testing::Test>;

TEST_F(ElimDeadOutputStoresTest, VertMultipleLocations) {
  // #version 450
  //
  // layout(location = 2) out Vertex
  // {
  //         vec4 color0;
  //         vec4 color1;
  //         vec4 color2[3];
  // } oVert;
  //
  // void main()
  // {
  //     oVert.color0 = vec4(0.0,0.0,0.0,0.0);
  //     oVert.color1 = vec4(0.1,0.0,0.0,0.0);
  //     oVert.color2[0] = vec4(0.2,0.0,0.0,0.0);
  //     oVert.color2[1] = vec4(0.3,0.0,0.0,0.0);
  //     oVert.color2[2] = vec4(0.4,0.0,0.0,0.0);
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %oVert
               OpSource GLSL 450
               OpName %main "main"
               OpName %Vertex "Vertex"
               OpMemberName %Vertex 0 "color0"
               OpMemberName %Vertex 1 "color1"
               OpMemberName %Vertex 2 "color2"
               OpName %oVert "oVert"
               OpDecorate %Vertex Block
               OpDecorate %oVert Location 2
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
     %Vertex = OpTypeStruct %v4float %v4float %_arr_v4float_uint_3
%_ptr_Output_Vertex = OpTypePointer Output %Vertex
      %oVert = OpVariable %_ptr_Output_Vertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
%float_0_100000001 = OpConstant %float 0.100000001
         %22 = OpConstantComposite %v4float %float_0_100000001 %float_0 %float_0 %float_0
      %int_2 = OpConstant %int 2
%float_0_200000003 = OpConstant %float 0.200000003
         %26 = OpConstantComposite %v4float %float_0_200000003 %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
         %29 = OpConstantComposite %v4float %float_0_300000012 %float_0 %float_0 %float_0
%float_0_400000006 = OpConstant %float 0.400000006
         %32 = OpConstantComposite %v4float %float_0_400000006 %float_0 %float_0 %float_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4float %oVert %int_0
               OpStore %19 %17
;CHECK:            OpStore %19 %17
         %23 = OpAccessChain %_ptr_Output_v4float %oVert %int_1
               OpStore %23 %22
;CHECK-NOT:        OpStore %23 %22
         %27 = OpAccessChain %_ptr_Output_v4float %oVert %int_2 %int_0
               OpStore %27 %26
;CHECK-NOT:        OpStore %27 %26
         %30 = OpAccessChain %_ptr_Output_v4float %oVert %int_2 %int_1
               OpStore %30 %29
;CHECK:            OpStore %30 %29
         %33 = OpAccessChain %_ptr_Output_v4float %oVert %int_2 %int_2
               OpStore %33 %32
;CHECK-NOT:        OpStore %33 %32
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::unordered_set<uint32_t> live_inputs;
  std::unordered_set<uint32_t> live_builtins;
  live_inputs.insert(2);
  live_inputs.insert(5);
  SinglePassRunAndMatch<EliminateDeadOutputStoresPass>(text, true, &live_inputs,
                                                       &live_builtins);
}

TEST_F(ElimDeadOutputStoresTest, VertMatrix) {
  // #version 450
  //
  // layout(location = 2) out Vertex
  // {
  //         vec4 color0;
  //         vec4 color1;
  //         mat4 color2;
  //         mat4 color3;
  //         mat4 color4;
  // } oVert;
  //
  // void main()
  // {
  //     oVert.color0 = vec4(0.0,0.0,0.0,0.0);
  //     oVert.color1 = vec4(0.1,0.0,0.0,0.0);
  //     oVert.color2[2] = vec4(0.2,0.0,0.0,0.0);
  //     oVert.color3[1] = vec4(0.3,0.0,0.0,0.0);
  //     oVert.color4[0] = vec4(0.4,0.0,0.0,0.0);
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %oVert
               OpSource GLSL 450
               OpName %main "main"
               OpName %Vertex "Vertex"
               OpMemberName %Vertex 0 "color0"
               OpMemberName %Vertex 1 "color1"
               OpMemberName %Vertex 2 "color2"
               OpMemberName %Vertex 3 "color3"
               OpMemberName %Vertex 4 "color4"
               OpName %oVert "oVert"
               OpDecorate %Vertex Block
               OpDecorate %oVert Location 2
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
     %Vertex = OpTypeStruct %v4float %v4float %mat4v4float %mat4v4float %mat4v4float
%_ptr_Output_Vertex = OpTypePointer Output %Vertex
      %oVert = OpVariable %_ptr_Output_Vertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
         %15 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
%float_0_100000001 = OpConstant %float 0.100000001
         %20 = OpConstantComposite %v4float %float_0_100000001 %float_0 %float_0 %float_0
      %int_2 = OpConstant %int 2
%float_0_200000003 = OpConstant %float 0.200000003
         %24 = OpConstantComposite %v4float %float_0_200000003 %float_0 %float_0 %float_0
      %int_3 = OpConstant %int 3
%float_0_300000012 = OpConstant %float 0.300000012
         %28 = OpConstantComposite %v4float %float_0_300000012 %float_0 %float_0 %float_0
      %int_4 = OpConstant %int 4
%float_0_400000006 = OpConstant %float 0.400000006
         %32 = OpConstantComposite %v4float %float_0_400000006 %float_0 %float_0 %float_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpAccessChain %_ptr_Output_v4float %oVert %int_0
               OpStore %17 %15
; CHECK:           OpStore %17 %15
         %21 = OpAccessChain %_ptr_Output_v4float %oVert %int_1
               OpStore %21 %20
; CHECK-NOT:       OpStore %21 %20
         %25 = OpAccessChain %_ptr_Output_v4float %oVert %int_2 %int_2
               OpStore %25 %24
; CHECK-NOT:       OpStore %25 %24
         %29 = OpAccessChain %_ptr_Output_v4float %oVert %int_3 %int_1
               OpStore %29 %28
; CHECK:           OpStore %29 %28
         %33 = OpAccessChain %_ptr_Output_v4float %oVert %int_4 %int_0
               OpStore %33 %32
; CHECK-NOT:       OpStore %33 %32
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::unordered_set<uint32_t> live_inputs;
  std::unordered_set<uint32_t> live_builtins;
  live_inputs.insert(2);
  live_inputs.insert(8);
  live_inputs.insert(9);
  live_inputs.insert(10);
  live_inputs.insert(11);
  SinglePassRunAndMatch<EliminateDeadOutputStoresPass>(text, true, &live_inputs,
                                                       &live_builtins);
}

TEST_F(ElimDeadOutputStoresTest, VertMemberLocs) {
  // #version 450
  //
  // out Vertex
  // {
  //     layout (location = 1) vec4 Cd;
  //     layout (location = 0) vec2 uv;
  // } oVert;
  //
  // layout (location  = 0) in vec3 P;
  //
  // void main()
  // {
  //     oVert.uv = vec2(0.1, 0.7);
  //     oVert.Cd = vec4(1, 0.5, 0, 1);
  //     gl_Position = vec4(P, 1);
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %oVert %_ %P
               OpSource GLSL 450
               OpName %main "main"
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
               OpName %P "P"
               OpMemberDecorate %Vertex 0 Location 1
               OpMemberDecorate %Vertex 1 Location 0
               OpDecorate %Vertex Block
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %P Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
     %Vertex = OpTypeStruct %v4float %v2float
%_ptr_Output_Vertex = OpTypePointer Output %Vertex
      %oVert = OpVariable %_ptr_Output_Vertex Output
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%float_0_100000001 = OpConstant %float 0.100000001
%float_0_699999988 = OpConstant %float 0.699999988
         %16 = OpConstantComposite %v2float %float_0_100000001 %float_0_699999988
%_ptr_Output_v2float = OpTypePointer Output %v2float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
  %float_0_5 = OpConstant %float 0.5
    %float_0 = OpConstant %float 0
         %23 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
          %P = OpVariable %_ptr_Input_v3float Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_Output_v2float %oVert %int_1
               OpStore %18 %16
; CHECK-NOT:       OpStore %18 %16
         %25 = OpAccessChain %_ptr_Output_v4float %oVert %int_0
               OpStore %25 %23
; CHECK:           OpStore %25 %23
         %35 = OpLoad %v3float %P
         %36 = OpCompositeExtract %float %35 0
         %37 = OpCompositeExtract %float %35 1
         %38 = OpCompositeExtract %float %35 2
         %39 = OpCompositeConstruct %v4float %36 %37 %38 %float_1
         %40 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %40 %39
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::unordered_set<uint32_t> live_inputs;
  std::unordered_set<uint32_t> live_builtins;
  live_inputs.insert(1);
  SinglePassRunAndMatch<EliminateDeadOutputStoresPass>(text, true, &live_inputs,
                                                       &live_builtins);
}

TEST_F(ElimDeadOutputStoresTest, ArrayedOutput) {
  // Tests elimination of arrayed output as seen in Tesc shaders.
  //
  // #version 450
  //
  // layout (vertices = 4) out;
  //
  // layout (location = 0) in vec3 N[];
  // layout (location = 1) in vec3 P[];
  //
  // layout (location = 5) out Vertex
  // {
  //                 vec4 c;
  //                 vec3 n;
  //                 vec3 f[10];
  // } oVert[];
  //
  // void main()
  // {
  //                 oVert[gl_InvocationID].c = vec4(1, 0, 0, 1);
  //                 oVert[gl_InvocationID].n = N[gl_InvocationID];
  //                 oVert[gl_InvocationID].f[3] = vec3(0, 1, 0);
  //                 vec4 worldSpacePos = vec4(P[gl_InvocationID], 1);
  //                 gl_out[gl_InvocationID].gl_Position = worldSpacePos;
  // }
  const std::string text = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %oVert %gl_InvocationID %N %P %gl_out
               OpExecutionMode %main OutputVertices 4
               OpSource GLSL 450
               OpName %main "main"
               OpName %Vertex "Vertex"
               OpMemberName %Vertex 0 "c"
               OpMemberName %Vertex 1 "n"
               OpMemberName %Vertex 2 "f"
               OpName %oVert "oVert"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %N "N"
               OpName %P "P"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpDecorate %Vertex Block
               OpDecorate %oVert Location 5
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorate %N Location 0
               OpDecorate %P Location 1
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
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
%_arr_v3float_uint_10 = OpTypeArray %v3float %uint_10
     %Vertex = OpTypeStruct %v4float %v3float %_arr_v3float_uint_10
     %uint_4 = OpConstant %uint 4
%_arr_Vertex_uint_4 = OpTypeArray %Vertex %uint_4
%_ptr_Output__arr_Vertex_uint_4 = OpTypePointer Output %_arr_Vertex_uint_4
      %oVert = OpVariable %_ptr_Output__arr_Vertex_uint_4 Output
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
         %24 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
    %uint_32 = OpConstant %uint 32
%_arr_v3float_uint_32 = OpTypeArray %v3float %uint_32
%_ptr_Input__arr_v3float_uint_32 = OpTypePointer Input %_arr_v3float_uint_32
          %N = OpVariable %_ptr_Input__arr_v3float_uint_32 Input
%_ptr_Input_v3float = OpTypePointer Input %v3float
%_ptr_Output_v3float = OpTypePointer Output %v3float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
         %42 = OpConstantComposite %v3float %float_0 %float_1 %float_0
          %P = OpVariable %_ptr_Input__arr_v3float_uint_32 Input
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_PerVertex_uint_4 = OpTypeArray %gl_PerVertex %uint_4
%_ptr_Output__arr_gl_PerVertex_uint_4 = OpTypePointer Output %_arr_gl_PerVertex_uint_4
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_uint_4 Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpLoad %int %gl_InvocationID
         %26 = OpAccessChain %_ptr_Output_v4float %oVert %20 %int_0
               OpStore %26 %24
; CHECK:           OpStore %26 %24
         %35 = OpAccessChain %_ptr_Input_v3float %N %20
         %36 = OpLoad %v3float %35
         %38 = OpAccessChain %_ptr_Output_v3float %oVert %20 %int_1
               OpStore %38 %36
; CHECK-NOT:       OpStore %38 %36
         %43 = OpAccessChain %_ptr_Output_v3float %oVert %20 %int_2 %int_3
               OpStore %43 %42
; CHECK:           OpStore %43 %42
         %48 = OpAccessChain %_ptr_Input_v3float %P %20
         %49 = OpLoad %v3float %48
         %50 = OpCompositeExtract %float %49 0
         %51 = OpCompositeExtract %float %49 1
         %52 = OpCompositeExtract %float %49 2
         %53 = OpCompositeConstruct %v4float %50 %51 %52 %float_1
         %62 = OpAccessChain %_ptr_Output_v4float %gl_out %20 %int_0
               OpStore %62 %53
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::unordered_set<uint32_t> live_inputs;
  std::unordered_set<uint32_t> live_builtins;
  live_inputs.insert(5);
  live_inputs.insert(10);
  SinglePassRunAndMatch<EliminateDeadOutputStoresPass>(text, true, &live_inputs,
                                                       &live_builtins);
}

TEST_F(ElimDeadOutputStoresTest, ArrayedOutputMemberLocs) {
  // Tests elimination of member location with arrayed output as seen in
  // Tesc shaders.
  //
  // #version 450
  //
  // layout (vertices = 4) out;
  //
  // layout (location = 0) in vec3 N[];
  // layout (location = 1) in vec3 P[];
  //
  // out Vertex
  // {
  //                 layout (location = 1) vec4 c;
  //                 layout (location = 3) vec3 n;
  //                 layout (location = 5) vec3 f[10];
  // } oVert[];
  //
  // void main()
  // {
  //                 oVert[gl_InvocationID].c = vec4(1, 0, 0, 1);
  //                 oVert[gl_InvocationID].n = N[gl_InvocationID];
  //                 oVert[gl_InvocationID].f[3] = vec3(0, 1, 0);
  //                 vec4 worldSpacePos = vec4(P[gl_InvocationID], 1);
  //                 gl_out[gl_InvocationID].gl_Position = worldSpacePos;
  // }
  const std::string text = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %oVert %gl_InvocationID %N %P %gl_out
               OpExecutionMode %main OutputVertices 4
               OpSource GLSL 450
               OpName %main "main"
               OpName %Vertex "Vertex"
               OpMemberName %Vertex 0 "c"
               OpMemberName %Vertex 1 "n"
               OpMemberName %Vertex 2 "f"
               OpName %oVert "oVert"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %N "N"
               OpName %P "P"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpMemberDecorate %Vertex 0 Location 1
               OpMemberDecorate %Vertex 1 Location 3
               OpMemberDecorate %Vertex 2 Location 5
               OpDecorate %Vertex Block
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorate %N Location 0
               OpDecorate %P Location 1
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
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
%_arr_v3float_uint_10 = OpTypeArray %v3float %uint_10
     %Vertex = OpTypeStruct %v4float %v3float %_arr_v3float_uint_10
     %uint_4 = OpConstant %uint 4
%_arr_Vertex_uint_4 = OpTypeArray %Vertex %uint_4
%_ptr_Output__arr_Vertex_uint_4 = OpTypePointer Output %_arr_Vertex_uint_4
      %oVert = OpVariable %_ptr_Output__arr_Vertex_uint_4 Output
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
         %24 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
    %uint_32 = OpConstant %uint 32
%_arr_v3float_uint_32 = OpTypeArray %v3float %uint_32
%_ptr_Input__arr_v3float_uint_32 = OpTypePointer Input %_arr_v3float_uint_32
          %N = OpVariable %_ptr_Input__arr_v3float_uint_32 Input
%_ptr_Input_v3float = OpTypePointer Input %v3float
%_ptr_Output_v3float = OpTypePointer Output %v3float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
         %42 = OpConstantComposite %v3float %float_0 %float_1 %float_0
          %P = OpVariable %_ptr_Input__arr_v3float_uint_32 Input
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_PerVertex_uint_4 = OpTypeArray %gl_PerVertex %uint_4
%_ptr_Output__arr_gl_PerVertex_uint_4 = OpTypePointer Output %_arr_gl_PerVertex_uint_4
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_uint_4 Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpLoad %int %gl_InvocationID
         %26 = OpAccessChain %_ptr_Output_v4float %oVert %20 %int_0
               OpStore %26 %24
;CHECK:            OpStore %26 %24
         %35 = OpAccessChain %_ptr_Input_v3float %N %20
         %36 = OpLoad %v3float %35
         %38 = OpAccessChain %_ptr_Output_v3float %oVert %20 %int_1
               OpStore %38 %36
;CHECK-NOT:        OpStore %38 %36
         %43 = OpAccessChain %_ptr_Output_v3float %oVert %20 %int_2 %int_3
               OpStore %43 %42
;CHECK:            OpStore %43 %42
         %48 = OpAccessChain %_ptr_Input_v3float %P %20
         %49 = OpLoad %v3float %48
         %50 = OpCompositeExtract %float %49 0
         %51 = OpCompositeExtract %float %49 1
         %52 = OpCompositeExtract %float %49 2
         %53 = OpCompositeConstruct %v4float %50 %51 %52 %float_1
         %62 = OpAccessChain %_ptr_Output_v4float %gl_out %20 %int_0
               OpStore %62 %53
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::unordered_set<uint32_t> live_inputs;
  std::unordered_set<uint32_t> live_builtins;
  live_inputs.insert(1);
  live_inputs.insert(8);
  SinglePassRunAndMatch<EliminateDeadOutputStoresPass>(text, true, &live_inputs,
                                                       &live_builtins);
}

TEST_F(ElimDeadOutputStoresTest, ScalarBuiltins) {
  // Tests elimination of scalar builtins as seen in vert shaders.
  //
  // #version 460
  //
  // layout (location = 0) in vec3 P;
  //
  // void main()
  // {
  //         gl_Position = vec4(P, 1.0);
  //         gl_PointSize = 1.0;
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %P
               OpSource GLSL 460
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""
               OpName %P "P"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %P Location 0
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
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
          %P = OpVariable %_ptr_Input_v3float Input
    %float_1 = OpConstant %float 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpLoad %v3float %P
         %21 = OpCompositeExtract %float %19 0
         %22 = OpCompositeExtract %float %19 1
         %23 = OpCompositeExtract %float %19 2
         %24 = OpCompositeConstruct %v4float %21 %22 %23 %float_1
         %26 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %26 %24
;CHECK:                   OpStore %26 %24
         %29 = OpAccessChain %_ptr_Output_float %_ %int_1
               OpStore %29 %float_1
;CHECK-NOT:               OpStore %29 %float_1
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::unordered_set<uint32_t> live_inputs;
  std::unordered_set<uint32_t> live_builtins;
  // Omit spv::BuiltIn::PointSize
  live_builtins.insert((uint32_t)spv::BuiltIn::ClipDistance);
  live_builtins.insert((uint32_t)spv::BuiltIn::CullDistance);
  SinglePassRunAndMatch<EliminateDeadOutputStoresPass>(text, true, &live_inputs,
                                                       &live_builtins);
}

TEST_F(ElimDeadOutputStoresTest, ArrayedBuiltins) {
  // Tests elimination of arrayed builtins as seen in geom, tesc, and tese
  // shaders.
  //
  // #version 460
  //
  // layout(triangle_strip, max_vertices = 3) out;
  // layout(triangles) in;
  //
  // void main()
  // {
  //         for (int i = 0; i < 3; i++)
  //         {
  //                 gl_Position = gl_in[i].gl_Position;
  //                 gl_PointSize = gl_in[i].gl_PointSize;
  //
  //                 EmitVertex();
  //         }
  //
  //         EndPrimitive();
  // }
  const std::string text = R"(
               OpCapability Geometry
               OpCapability GeometryPointSize
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
               OpMemberName %gl_PerVertex_0 1 "gl_PointSize"
               OpMemberName %gl_PerVertex_0 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex_0 3 "gl_CullDistance"
               OpName %gl_in "gl_in"
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
%gl_PerVertex_0 = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
     %uint_3 = OpConstant %uint 3
%_arr_gl_PerVertex_0_uint_3 = OpTypeArray %gl_PerVertex_0 %uint_3
%_ptr_Input__arr_gl_PerVertex_0_uint_3 = OpTypePointer Input %_arr_gl_PerVertex_0_uint_3
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_0_uint_3 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_float = OpTypePointer Output %float
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
;CHECK:                   OpStore %37 %35
         %39 = OpLoad %int %i
         %41 = OpAccessChain %_ptr_Input_float %gl_in %39 %int_1
         %42 = OpLoad %float %41
         %44 = OpAccessChain %_ptr_Output_float %_ %int_1
               OpStore %44 %42
;CHECK-NOT:               OpStore %44 %42
               OpEmitVertex
               OpBranch %13
         %13 = OpLabel
         %45 = OpLoad %int %i
         %46 = OpIAdd %int %45 %int_1
               OpStore %i %46
               OpBranch %10
         %12 = OpLabel
               OpEndPrimitive
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::unordered_set<uint32_t> live_inputs;
  std::unordered_set<uint32_t> live_builtins;
  // Omit spv::BuiltIn::PointSize
  live_builtins.insert((uint32_t)spv::BuiltIn::ClipDistance);
  live_builtins.insert((uint32_t)spv::BuiltIn::CullDistance);
  SinglePassRunAndMatch<EliminateDeadOutputStoresPass>(text, true, &live_inputs,
                                                       &live_builtins);
}

TEST_F(ElimDeadOutputStoresTest, ArrayedOutputPatchLocs) {
  // Tests elimination of location with arrayed patch output as seen in
  // Tesc shaders.
  //
  // #version 450 core
  //
  // layout(vertices = 4) out;
  //
  // layout(location=0) patch out float patchOut0[2];
  // layout(location=2) patch out float patchOut1[2];
  //
  // void main()
  // {
  //     patchOut0[1] = 0.0;  // Dead loc 1
  //     patchOut1[1] = 1.0;  // Live loc 3
  // }
  const std::string text = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %patchOut0 %patchOut1
               OpExecutionMode %main OutputVertices 4
               OpSource GLSL 450
               OpName %main "main"
               OpName %patchOut0 "patchOut0"
               OpName %patchOut1 "patchOut1"
               OpDecorate %patchOut0 Patch
               OpDecorate %patchOut0 Location 0
               OpDecorate %patchOut1 Patch
               OpDecorate %patchOut1 Location 2
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
  %patchOut0 = OpVariable %_ptr_Output__arr_float_uint_2 Output
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
    %float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
  %patchOut1 = OpVariable %_ptr_Output__arr_float_uint_2 Output
    %float_1 = OpConstant %float 1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpAccessChain %_ptr_Output_float %patchOut0 %int_1
               OpStore %16 %float_0
;CHECK-NOT:               OpStore %16 %float_0
         %19 = OpAccessChain %_ptr_Output_float %patchOut1 %int_1
               OpStore %19 %float_1
;CHECK:                   OpStore %19 %float_1
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::unordered_set<uint32_t> live_inputs;
  std::unordered_set<uint32_t> live_builtins;
  live_inputs.insert(3);
  SinglePassRunAndMatch<EliminateDeadOutputStoresPass>(text, true, &live_inputs,
                                                       &live_builtins);
}

TEST_F(ElimDeadOutputStoresTest, VertMultipleLocationsF16) {
  // #version 450
  //
  // layout(location = 2) out Vertex
  // {
  //         f16vec4 color0;
  //         f16vec4 color1;
  //         f16vec4 color2[3];
  // } oVert;
  //
  // void main()
  // {
  //     oVert.color0 = f16vec4(0.0,0.0,0.0,0.0);
  //     oVert.color1 = f16vec4(0.1,0.0,0.0,0.0);
  //     oVert.color2[0] = f16vec4(0.2,0.0,0.0,0.0);
  //     oVert.color2[1] = f16vec4(0.3,0.0,0.0,0.0);
  //     oVert.color2[2] = f16vec4(0.4,0.0,0.0,0.0);
  // }
  const std::string text = R"(
               OpCapability Shader
               OpCapability Float16
               OpCapability StorageInputOutput16
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %oVert
               OpSource GLSL 450
               OpName %main "main"
               OpName %Vertex "Vertex"
               OpMemberName %Vertex 0 "color0"
               OpMemberName %Vertex 1 "color1"
               OpMemberName %Vertex 2 "color2"
               OpName %oVert "oVert"
               OpDecorate %Vertex Block
               OpDecorate %oVert Location 2
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %half = OpTypeFloat 32
    %v4half = OpTypeVector %half 4
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
%_arr_v4half_uint_3 = OpTypeArray %v4half %uint_3
     %Vertex = OpTypeStruct %v4half %v4half %_arr_v4half_uint_3
%_ptr_Output_Vertex = OpTypePointer Output %Vertex
      %oVert = OpVariable %_ptr_Output_Vertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %half_0 = OpConstant %half 0
         %17 = OpConstantComposite %v4half %half_0 %half_0 %half_0 %half_0
%_ptr_Output_v4half = OpTypePointer Output %v4half
      %int_1 = OpConstant %int 1
%half_0_100000001 = OpConstant %half 0.100000001
         %22 = OpConstantComposite %v4half %half_0_100000001 %half_0 %half_0 %half_0
      %int_2 = OpConstant %int 2
%half_0_200000003 = OpConstant %half 0.200000003
         %26 = OpConstantComposite %v4half %half_0_200000003 %half_0 %half_0 %half_0
%half_0_300000012 = OpConstant %half 0.300000012
         %29 = OpConstantComposite %v4half %half_0_300000012 %half_0 %half_0 %half_0
%half_0_400000006 = OpConstant %half 0.400000006
         %32 = OpConstantComposite %v4half %half_0_400000006 %half_0 %half_0 %half_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_v4half %oVert %int_0
               OpStore %19 %17
;CHECK:            OpStore %19 %17
         %23 = OpAccessChain %_ptr_Output_v4half %oVert %int_1
               OpStore %23 %22
;CHECK-NOT:        OpStore %23 %22
         %27 = OpAccessChain %_ptr_Output_v4half %oVert %int_2 %int_0
               OpStore %27 %26
;CHECK-NOT:        OpStore %27 %26
         %30 = OpAccessChain %_ptr_Output_v4half %oVert %int_2 %int_1
               OpStore %30 %29
;CHECK:            OpStore %30 %29
         %33 = OpAccessChain %_ptr_Output_v4half %oVert %int_2 %int_2
               OpStore %33 %32
;CHECK-NOT:        OpStore %33 %32
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::unordered_set<uint32_t> live_inputs;
  std::unordered_set<uint32_t> live_builtins;
  live_inputs.insert(2);
  live_inputs.insert(5);
  SinglePassRunAndMatch<EliminateDeadOutputStoresPass>(text, true, &live_inputs,
                                                       &live_builtins);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
