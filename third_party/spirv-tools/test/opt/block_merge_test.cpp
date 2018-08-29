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

#include <string>

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using BlockMergeTest = PassTest<::testing::Test>;

TEST_F(BlockMergeTest, Simple) {
  // Note: SPIR-V hand edited to insert block boundary
  // between two statements in main.
  //
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  void main()
  //  {
  //      vec4 v = BaseColor;
  //      gl_FragColor = v;
  //  }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
OpBranch %15
%15 = OpLabel
%16 = OpLoad %v4float %v
OpStore %gl_FragColor %16
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
%16 = OpLoad %v4float %v
OpStore %gl_FragColor %16
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<BlockMergePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(BlockMergeTest, EmptyBlock) {
  // Note: SPIR-V hand edited to insert empty block
  // after two statements in main.
  //
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  void main()
  //  {
  //      vec4 v = BaseColor;
  //      gl_FragColor = v;
  //  }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
OpBranch %15
%15 = OpLabel
%16 = OpLoad %v4float %v
OpStore %gl_FragColor %16
OpBranch %17
%17 = OpLabel
OpBranch %18
%18 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
%16 = OpLoad %v4float %v
OpStore %gl_FragColor %16
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<BlockMergePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(BlockMergeTest, NestedInControlFlow) {
  // Note: SPIR-V hand edited to insert block boundary
  // between OpFMul and OpStore in then-part.
  //
  // #version 140
  // in vec4 BaseColor;
  //
  // layout(std140) uniform U_t
  // {
  //     bool g_B ;
  // } ;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     if (g_B)
  //       vec4 v = v * 0.25;
  //     gl_FragColor = v;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_B"
OpName %_ ""
OpName %v_0 "v"
OpName %gl_FragColor "gl_FragColor"
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t Block
OpDecorate %_ DescriptorSet 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%U_t = OpTypeStruct %uint
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%bool = OpTypeBool
%uint_0 = OpConstant %uint 0
%float_0_25 = OpConstant %float 0.25
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %10
%24 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%v_0 = OpVariable %_ptr_Function_v4float Function
%25 = OpLoad %v4float %BaseColor
OpStore %v %25
%26 = OpAccessChain %_ptr_Uniform_uint %_ %int_0
%27 = OpLoad %uint %26
%28 = OpINotEqual %bool %27 %uint_0
OpSelectionMerge %29 None
OpBranchConditional %28 %30 %29
%30 = OpLabel
%31 = OpLoad %v4float %v
%32 = OpVectorTimesScalar %v4float %31 %float_0_25
OpBranch %33
%33 = OpLabel
OpStore %v_0 %32
OpBranch %29
%29 = OpLabel
%34 = OpLoad %v4float %v
OpStore %gl_FragColor %34
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %10
%24 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%v_0 = OpVariable %_ptr_Function_v4float Function
%25 = OpLoad %v4float %BaseColor
OpStore %v %25
%26 = OpAccessChain %_ptr_Uniform_uint %_ %int_0
%27 = OpLoad %uint %26
%28 = OpINotEqual %bool %27 %uint_0
OpSelectionMerge %29 None
OpBranchConditional %28 %30 %29
%30 = OpLabel
%31 = OpLoad %v4float %v
%32 = OpVectorTimesScalar %v4float %31 %float_0_25
OpStore %v_0 %32
OpBranch %29
%29 = OpLabel
%34 = OpLoad %v4float %v
OpStore %gl_FragColor %34
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<BlockMergePass>(predefs + before, predefs + after, true,
                                        true);
}

#ifdef SPIRV_EFFCEE
TEST_F(BlockMergeTest, PhiInSuccessorOfMergedBlock) {
  const std::string text = R"(
; CHECK: OpSelectionMerge [[merge:%\w+]] None
; CHECK-NEXT: OpBranchConditional {{%\w+}} [[then:%\w+]] [[else:%\w+]]
; CHECK: [[then]] = OpLabel
; CHECK-NEXT: OpBranch [[merge]]
; CHECK: [[else]] = OpLabel
; CHECK-NEXT: OpBranch [[merge]]
; CHECK: [[merge]] = OpLabel
; CHECK-NEXT: OpPhi {{%\w+}} %true [[then]] %false [[else]]
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpSelectionMerge %merge None
OpBranchConditional %true %then %else
%then = OpLabel
OpBranch %then_next
%then_next = OpLabel
OpBranch %merge
%else = OpLabel
OpBranch %merge
%merge = OpLabel
%phi = OpPhi %bool %true %then_next %false %else
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, UpdateMergeInstruction) {
  const std::string text = R"(
; CHECK: OpSelectionMerge [[merge:%\w+]] None
; CHECK-NEXT: OpBranchConditional {{%\w+}} [[then:%\w+]] [[else:%\w+]]
; CHECK: [[then]] = OpLabel
; CHECK-NEXT: OpBranch [[merge]]
; CHECK: [[else]] = OpLabel
; CHECK-NEXT: OpBranch [[merge]]
; CHECK: [[merge]] = OpLabel
; CHECK-NEXT: OpReturn
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpSelectionMerge %real_merge None
OpBranchConditional %true %then %else
%then = OpLabel
OpBranch %merge
%else = OpLabel
OpBranch %merge
%merge = OpLabel
OpBranch %real_merge
%real_merge = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, TwoMergeBlocksCannotBeMerged) {
  const std::string text = R"(
; CHECK: OpSelectionMerge [[outer_merge:%\w+]] None
; CHECK: OpSelectionMerge [[inner_merge:%\w+]] None
; CHECK: [[inner_merge]] = OpLabel
; CHECK-NEXT: OpBranch [[outer_merge]]
; CHECK: [[outer_merge]] = OpLabel
; CHECK-NEXT: OpReturn
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpSelectionMerge %outer_merge None
OpBranchConditional %true %then %else
%then = OpLabel
OpBranch %inner_header
%else = OpLabel
OpBranch %inner_header
%inner_header = OpLabel
OpSelectionMerge %inner_merge None
OpBranchConditional %true %inner_then %inner_else
%inner_then = OpLabel
OpBranch %inner_merge
%inner_else = OpLabel
OpBranch %inner_merge
%inner_merge = OpLabel
OpBranch %outer_merge
%outer_merge = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, MergeContinue) {
  const std::string text = R"(
; CHECK: OpBranch [[header:%\w+]]
; CHECK: [[header]] = OpLabel
; CHECK-NEXT: OpLogicalAnd
; CHECK-NEXT: OpLoopMerge {{%\w+}} [[header]] None
; CHECK-NEXT: OpBranch [[header]]
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpBranch %header
%header = OpLabel
OpLoopMerge %merge %continue None
OpBranch %continue
%continue = OpLabel
%op = OpLogicalAnd %bool %true %false
OpBranch %header
%merge = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, TwoHeadersCannotBeMerged) {
  const std::string text = R"(
; CHECK: OpBranch [[loop_header:%\w+]]
; CHECK: [[loop_header]] = OpLabel
; CHECK-NEXT: OpLoopMerge
; CHECK-NEXT: OpBranch [[if_header:%\w+]]
; CHECK: [[if_header]] = OpLabel
; CHECK-NEXT: OpSelectionMerge
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpBranch %header
%header = OpLabel
OpLoopMerge %merge %continue None
OpBranch %inner_header
%inner_header = OpLabel
OpSelectionMerge %continue None
OpBranchConditional %true %then %continue
%then = OpLabel
OpBranch %continue
%continue = OpLabel
OpBranchConditional %false %merge %header
%merge = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, RemoveStructuredDeclaration) {
  // Note: SPIR-V hand edited remove dead branch and add block
  // before continue block
  //
  // #version 140
  // in vec4 BaseColor;
  //
  // void main()
  // {
  //     while (true) {
  //         break;
  //     }
  //     gl_FragColor = BaseColor;
  // }

  const std::string assembly =
      R"(
; CHECK: OpLabel
; CHECK: [[header:%\w+]] = OpLabel
; CHECK-NOT: OpLoopMerge
; CHECK: OpReturn
; CHECK: [[continue:%\w+]] = OpLabel
; CHECK-NEXT: OpBranch [[header]]
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %gl_FragColor %BaseColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %gl_FragColor "gl_FragColor"
OpName %BaseColor "BaseColor"
%void = OpTypeVoid
%6 = OpTypeFunction %void
%bool = OpTypeBool
%true = OpConstantTrue %bool
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %6
%13 = OpLabel
OpBranch %14
%14 = OpLabel
OpLoopMerge %15 %16 None
OpBranch %17
%17 = OpLabel
OpBranch %15
%18 = OpLabel
OpBranch %16
%16 = OpLabel
OpBranch %14
%15 = OpLabel
%19 = OpLoad %v4float %BaseColor
OpStore %gl_FragColor %19
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(assembly, true);
}

TEST_F(BlockMergeTest, DontMergeKill) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpKill
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
%void = OpTypeVoid
%bool = OpTypeBool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%1 = OpLabel
OpBranch %2
%2 = OpLabel
OpLoopMerge %3 %4 None
OpBranch %5
%5 = OpLabel
OpKill
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, DontMergeUnreachable) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpUnreachable
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
%void = OpTypeVoid
%bool = OpTypeBool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%1 = OpLabel
OpBranch %2
%2 = OpLabel
OpLoopMerge %3 %4 None
OpBranch %5
%5 = OpLabel
OpUnreachable
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, DontMergeReturn) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpReturn
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
%void = OpTypeVoid
%bool = OpTypeBool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%1 = OpLabel
OpBranch %2
%2 = OpLabel
OpLoopMerge %3 %4 None
OpBranch %5
%5 = OpLabel
OpReturn
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, DontMergeSwitch) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpSwitch
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
%void = OpTypeVoid
%bool = OpTypeBool
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%1 = OpLabel
OpBranch %2
%2 = OpLabel
OpLoopMerge %3 %4 None
OpBranch %5
%5 = OpLabel
OpSwitch %int_0 %6
%6 = OpLabel
OpReturn
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, DontMergeReturnValue) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpReturn
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
%void = OpTypeVoid
%bool = OpTypeBool
%functy = OpTypeFunction %void
%otherfuncty = OpTypeFunction %bool
%true = OpConstantTrue %bool
%func = OpFunction %void None %functy
%1 = OpLabel
%2 = OpFunctionCall %bool %3
OpReturn
OpFunctionEnd
%3 = OpFunction %bool None %otherfuncty
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %6 %7 None
OpBranch %8
%8 = OpLabel
OpReturnValue %true
%7 = OpLabel
OpBranch %5
%6 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}
#endif  // SPIRV_EFFCEE

// TODO(greg-lunarg): Add tests to verify handling of these cases:
//
//    More complex control flow
//    Others?

}  // namespace
}  // namespace opt
}  // namespace spvtools
