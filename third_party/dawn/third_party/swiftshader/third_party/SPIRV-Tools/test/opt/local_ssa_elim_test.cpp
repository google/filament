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

#include <memory>
#include <string>

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using LocalSSAElimTest = PassTest<::testing::Test>;

TEST_F(LocalSSAElimTest, ForLoop) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     for (int i=0; i<4; i++) {
  //       f = f + BC[i];
  //     }
  //     fo = f;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %8
%22 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
OpBranch %23
%23 = OpLabel
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%27 = OpLoad %int %i
%28 = OpSLessThan %bool %27 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%30 = OpLoad %float %f
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
%34 = OpFAdd %float %30 %33
OpStore %f %34
OpBranch %25
%25 = OpLabel
%35 = OpLoad %int %i
%36 = OpIAdd %int %35 %int_1
OpStore %i %36
OpBranch %23
%24 = OpLabel
%37 = OpLoad %float %f
OpStore %fo %37
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %8
%22 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
OpBranch %23
%23 = OpLabel
%39 = OpPhi %float %float_0 %22 %34 %25
%38 = OpPhi %int %int_0 %22 %36 %25
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%28 = OpSLessThan %bool %38 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%32 = OpAccessChain %_ptr_Input_float %BC %38
%33 = OpLoad %float %32
%34 = OpFAdd %float %39 %33
OpStore %f %34
OpBranch %25
%25 = OpLabel
%36 = OpIAdd %int %38 %int_1
OpStore %i %36
OpBranch %23
%24 = OpLabel
OpStore %fo %39
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(LocalSSAElimTest, NestedForLoop) {
  // #version 450
  //
  // layout (location=0) in mat4 BC;
  // layout (location=0) out float fo;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     for (int i=0; i<4; i++)
  //       for (int j=0; j<4; j++)
  //         f = f + BC[i][j];
  //     fo = f;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %j "j"
OpName %BC "BC"
OpName %fo "fo"
OpDecorate %BC Location 0
OpDecorate %fo Location 0
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Input_mat4v4float = OpTypePointer Input %mat4v4float
%BC = OpVariable %_ptr_Input_mat4v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
)";

  const std::string before =
      R"(
; CHECK: = OpFunction
; CHECK-NEXT: [[entry:%\w+]] = OpLabel
; CHECK: [[outer_header:%\w+]] = OpLabel
; CHECK-NEXT: [[outer_f:%\w+]] = OpPhi %float %float_0 [[entry]] [[inner_f:%\w+]] [[outer_be:%\w+]]
; CHECK-NEXT: [[i:%\w+]] = OpPhi %int %int_0 [[entry]] [[i_next:%\w+]] [[outer_be]]
; CHECK-NEXT: OpSLessThan {{%\w+}} [[i]]
; CHECK: [[inner_pre_header:%\w+]] = OpLabel
; CHECK: [[inner_header:%\w+]] = OpLabel
; CHECK-NEXT: [[inner_f]] = OpPhi %float [[outer_f]] [[inner_pre_header]] [[f_next:%\w+]] [[inner_be:%\w+]]
; CHECK-NEXT: [[j:%\w+]] = OpPhi %int %int_0 [[inner_pre_header]] [[j_next:%\w+]] [[inner_be]]
; CHECK: [[inner_be]] = OpLabel
; CHECK: [[f_next]] = OpFAdd %float [[inner_f]]
; CHECK: [[j_next]] = OpIAdd %int [[j]] %int_1
; CHECK: [[outer_be]] = OpLabel
; CHECK: [[i_next]] = OpIAdd
; CHECK: OpStore %fo [[outer_f]]
%main = OpFunction %void None %9
%24 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%j = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
OpBranch %25
%25 = OpLabel
%26 = OpLoad %int %i
%27 = OpSLessThan %bool %26 %int_4
OpLoopMerge %28 %29 None
OpBranchConditional %27 %30 %28
%30 = OpLabel
OpStore %j %int_0
OpBranch %31
%31 = OpLabel
%32 = OpLoad %int %j
%33 = OpSLessThan %bool %32 %int_4
OpLoopMerge %50 %34 None
OpBranchConditional %33 %34 %50
%34 = OpLabel
%35 = OpLoad %float %f
%36 = OpLoad %int %i
%37 = OpLoad %int %j
%38 = OpAccessChain %_ptr_Input_float %BC %36 %37
%39 = OpLoad %float %38
%40 = OpFAdd %float %35 %39
OpStore %f %40
%41 = OpLoad %int %j
%42 = OpIAdd %int %41 %int_1
OpStore %j %42
OpBranch %31
%50 = OpLabel
OpBranch %29
%29 = OpLabel
%43 = OpLoad %int %i
%44 = OpIAdd %int %43 %int_1
OpStore %i %44
OpBranch %25
%28 = OpLabel
%45 = OpLoad %float %f
OpStore %fo %45
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(predefs + before, true);
}

TEST_F(LocalSSAElimTest, ForLoopWithContinue) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     for (int i=0; i<4; i++) {
  //       float t = BC[i];
  //       if (t < 0.0)
  //         continue;
  //       f = f + t;
  //     }
  //     fo = f;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
)";

  const std::string names =
      R"(OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %t "t"
OpName %BC "BC"
OpName %fo "fo"
)";

  const std::string predefs2 =
      R"(%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %9
%23 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_float Function
OpStore %f %float_0
OpStore %i %int_0
OpBranch %24
%24 = OpLabel
OpLoopMerge %25 %26 None
OpBranch %27
%27 = OpLabel
%28 = OpLoad %int %i
%29 = OpSLessThan %bool %28 %int_4
OpBranchConditional %29 %30 %25
%30 = OpLabel
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
OpStore %t %33
%34 = OpLoad %float %t
%35 = OpFOrdLessThan %bool %34 %float_0
OpSelectionMerge %36 None
OpBranchConditional %35 %37 %36
%37 = OpLabel
OpBranch %26
%36 = OpLabel
%38 = OpLoad %float %f
%39 = OpLoad %float %t
%40 = OpFAdd %float %38 %39
OpStore %f %40
OpBranch %26
%26 = OpLabel
%41 = OpLoad %int %i
%42 = OpIAdd %int %41 %int_1
OpStore %i %42
OpBranch %24
%25 = OpLabel
%43 = OpLoad %float %f
OpStore %fo %43
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %9
%23 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_float Function
OpStore %f %float_0
OpStore %i %int_0
OpBranch %24
%24 = OpLabel
%45 = OpPhi %float %float_0 %23 %47 %26
%44 = OpPhi %int %int_0 %23 %42 %26
OpLoopMerge %25 %26 None
OpBranch %27
%27 = OpLabel
%29 = OpSLessThan %bool %44 %int_4
OpBranchConditional %29 %30 %25
%30 = OpLabel
%32 = OpAccessChain %_ptr_Input_float %BC %44
%33 = OpLoad %float %32
OpStore %t %33
%35 = OpFOrdLessThan %bool %33 %float_0
OpSelectionMerge %36 None
OpBranchConditional %35 %37 %36
%37 = OpLabel
OpBranch %26
%36 = OpLabel
%40 = OpFAdd %float %45 %33
OpStore %f %40
OpBranch %26
%26 = OpLabel
%47 = OpPhi %float %45 %37 %40 %36
%42 = OpIAdd %int %44 %int_1
OpStore %i %42
OpBranch %24
%25 = OpLabel
OpStore %fo %45
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(predefs + names + predefs2 + before,
                                        predefs + names + predefs2 + after,
                                        true, true);
}

TEST_F(LocalSSAElimTest, ForLoopWithBreak) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     for (int i=0; i<4; i++) {
  //       float t = f + BC[i];
  //       if (t > 1.0)
  //         break;
  //       f = t;
  //     }
  //     fo = f;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %t "t"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %9
%24 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_float Function
OpStore %f %float_0
OpStore %i %int_0
OpBranch %25
%25 = OpLabel
OpLoopMerge %26 %27 None
OpBranch %28
%28 = OpLabel
%29 = OpLoad %int %i
%30 = OpSLessThan %bool %29 %int_4
OpBranchConditional %30 %31 %26
%31 = OpLabel
%32 = OpLoad %float %f
%33 = OpLoad %int %i
%34 = OpAccessChain %_ptr_Input_float %BC %33
%35 = OpLoad %float %34
%36 = OpFAdd %float %32 %35
OpStore %t %36
%37 = OpLoad %float %t
%38 = OpFOrdGreaterThan %bool %37 %float_1
OpSelectionMerge %39 None
OpBranchConditional %38 %40 %39
%40 = OpLabel
OpBranch %26
%39 = OpLabel
%41 = OpLoad %float %t
OpStore %f %41
OpBranch %27
%27 = OpLabel
%42 = OpLoad %int %i
%43 = OpIAdd %int %42 %int_1
OpStore %i %43
OpBranch %25
%26 = OpLabel
%44 = OpLoad %float %f
OpStore %fo %44
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %9
%24 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_float Function
OpStore %f %float_0
OpStore %i %int_0
OpBranch %25
%25 = OpLabel
%46 = OpPhi %float %float_0 %24 %36 %27
%45 = OpPhi %int %int_0 %24 %43 %27
OpLoopMerge %26 %27 None
OpBranch %28
%28 = OpLabel
%30 = OpSLessThan %bool %45 %int_4
OpBranchConditional %30 %31 %26
%31 = OpLabel
%34 = OpAccessChain %_ptr_Input_float %BC %45
%35 = OpLoad %float %34
%36 = OpFAdd %float %46 %35
OpStore %t %36
%38 = OpFOrdGreaterThan %bool %36 %float_1
OpSelectionMerge %39 None
OpBranchConditional %38 %40 %39
%40 = OpLabel
OpBranch %26
%39 = OpLabel
OpStore %f %36
OpBranch %27
%27 = OpLabel
%43 = OpIAdd %int %45 %int_1
OpStore %i %43
OpBranch %25
%26 = OpLabel
OpStore %fo %46
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(LocalSSAElimTest, SwapProblem) {
  // #version 140
  //
  // in float fe;
  // out float fo;
  //
  // void main()
  // {
  //     float f1 = 0.0;
  //     float f2 = 1.0;
  //     int ie = int(fe);
  //     for (int i=0; i<ie; i++) {
  //       float t = f1;
  //       f1 = f2;
  //       f2 = t;
  //     }
  //     fo = f1;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %fe %fo
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %ie "ie"
OpName %fe "fe"
OpName %i "i"
OpName %t "t"
OpName %fo "fo"
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Input_float = OpTypePointer Input %float
%fe = OpVariable %_ptr_Input_float Input
%int_0 = OpConstant %int 0
%bool = OpTypeBool
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %11
%23 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%ie = OpVariable %_ptr_Function_int Function
%i = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_float Function
OpStore %f1 %float_0
OpStore %f2 %float_1
%24 = OpLoad %float %fe
%25 = OpConvertFToS %int %24
OpStore %ie %25
OpStore %i %int_0
OpBranch %26
%26 = OpLabel
OpLoopMerge %27 %28 None
OpBranch %29
%29 = OpLabel
%30 = OpLoad %int %i
%31 = OpLoad %int %ie
%32 = OpSLessThan %bool %30 %31
OpBranchConditional %32 %33 %27
%33 = OpLabel
%34 = OpLoad %float %f1
OpStore %t %34
%35 = OpLoad %float %f2
OpStore %f1 %35
%36 = OpLoad %float %t
OpStore %f2 %36
OpBranch %28
%28 = OpLabel
%37 = OpLoad %int %i
%38 = OpIAdd %int %37 %int_1
OpStore %i %38
OpBranch %26
%27 = OpLabel
%39 = OpLoad %float %f1
OpStore %fo %39
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %11
%23 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%ie = OpVariable %_ptr_Function_int Function
%i = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_float Function
OpStore %f1 %float_0
OpStore %f2 %float_1
%24 = OpLoad %float %fe
%25 = OpConvertFToS %int %24
OpStore %ie %25
OpStore %i %int_0
OpBranch %26
%26 = OpLabel
%43 = OpPhi %float %float_1 %23 %42 %28
%42 = OpPhi %float %float_0 %23 %43 %28
%40 = OpPhi %int %int_0 %23 %38 %28
OpLoopMerge %27 %28 None
OpBranch %29
%29 = OpLabel
%32 = OpSLessThan %bool %40 %25
OpBranchConditional %32 %33 %27
%33 = OpLabel
OpStore %t %42
OpStore %f1 %43
OpStore %f2 %42
OpBranch %28
%28 = OpLabel
%38 = OpIAdd %int %40 %int_1
OpStore %i %38
OpBranch %26
%27 = OpLabel
OpStore %fo %42
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(LocalSSAElimTest, LostCopyProblem) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     float t;
  //     for (int i=0; i<4; i++) {
  //       t = f;
  //       f = f + BC[i];
  //       if (f > 1.0)
  //         break;
  //     }
  //     fo = t;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %t "t"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %9
%24 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_float Function
OpStore %f %float_0
OpStore %i %int_0
OpBranch %25
%25 = OpLabel
OpLoopMerge %26 %27 None
OpBranch %28
%28 = OpLabel
%29 = OpLoad %int %i
%30 = OpSLessThan %bool %29 %int_4
OpBranchConditional %30 %31 %26
%31 = OpLabel
%32 = OpLoad %float %f
OpStore %t %32
%33 = OpLoad %float %f
%34 = OpLoad %int %i
%35 = OpAccessChain %_ptr_Input_float %BC %34
%36 = OpLoad %float %35
%37 = OpFAdd %float %33 %36
OpStore %f %37
%38 = OpLoad %float %f
%39 = OpFOrdGreaterThan %bool %38 %float_1
OpSelectionMerge %40 None
OpBranchConditional %39 %41 %40
%41 = OpLabel
OpBranch %26
%40 = OpLabel
OpBranch %27
%27 = OpLabel
%42 = OpLoad %int %i
%43 = OpIAdd %int %42 %int_1
OpStore %i %43
OpBranch %25
%26 = OpLabel
%44 = OpLoad %float %t
OpStore %fo %44
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%49 = OpUndef %float
%main = OpFunction %void None %9
%24 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_float Function
OpStore %f %float_0
OpStore %i %int_0
OpBranch %25
%25 = OpLabel
%46 = OpPhi %float %float_0 %24 %37 %27
%45 = OpPhi %int %int_0 %24 %43 %27
%48 = OpPhi %float %49 %24 %46 %27
OpLoopMerge %26 %27 None
OpBranch %28
%28 = OpLabel
%30 = OpSLessThan %bool %45 %int_4
OpBranchConditional %30 %31 %26
%31 = OpLabel
OpStore %t %46
%35 = OpAccessChain %_ptr_Input_float %BC %45
%36 = OpLoad %float %35
%37 = OpFAdd %float %46 %36
OpStore %f %37
%39 = OpFOrdGreaterThan %bool %37 %float_1
OpSelectionMerge %40 None
OpBranchConditional %39 %41 %40
%41 = OpLabel
OpBranch %26
%40 = OpLabel
OpBranch %27
%27 = OpLabel
%43 = OpIAdd %int %45 %int_1
OpStore %i %43
OpBranch %25
%26 = OpLabel
%47 = OpPhi %float %48 %28 %46 %41
OpStore %fo %47
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(LocalSSAElimTest, IfThenElse) {
  // #version 140
  //
  // in vec4 BaseColor;
  // in float f;
  //
  // void main()
  // {
  //     vec4 v;
  //     if (f >= 0)
  //       v = BaseColor * 0.5;
  //     else
  //       v = BaseColor + vec4(1.0,1.0,1.0,1.0);
  //     gl_FragColor = v;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %f %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %f "f"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
%f = OpVariable %_ptr_Input_float Input
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%float_0_5 = OpConstant %float 0.5
%float_1 = OpConstant %float 1
%18 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %8
%20 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%21 = OpLoad %float %f
%22 = OpFOrdGreaterThanEqual %bool %21 %float_0
OpSelectionMerge %23 None
OpBranchConditional %22 %24 %25
%24 = OpLabel
%26 = OpLoad %v4float %BaseColor
%27 = OpVectorTimesScalar %v4float %26 %float_0_5
OpStore %v %27
OpBranch %23
%25 = OpLabel
%28 = OpLoad %v4float %BaseColor
%29 = OpFAdd %v4float %28 %18
OpStore %v %29
OpBranch %23
%23 = OpLabel
%30 = OpLoad %v4float %v
OpStore %gl_FragColor %30
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %8
%20 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%21 = OpLoad %float %f
%22 = OpFOrdGreaterThanEqual %bool %21 %float_0
OpSelectionMerge %23 None
OpBranchConditional %22 %24 %25
%24 = OpLabel
%26 = OpLoad %v4float %BaseColor
%27 = OpVectorTimesScalar %v4float %26 %float_0_5
OpStore %v %27
OpBranch %23
%25 = OpLabel
%28 = OpLoad %v4float %BaseColor
%29 = OpFAdd %v4float %28 %18
OpStore %v %29
OpBranch %23
%23 = OpLabel
%31 = OpPhi %v4float %27 %24 %29 %25
OpStore %gl_FragColor %31
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(LocalSSAElimTest, IfThen) {
  // #version 140
  //
  // in vec4 BaseColor;
  // in float f;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     if (f <= 0)
  //       v = v * 0.5;
  //     gl_FragColor = v;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %f %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %f "f"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%f = OpVariable %_ptr_Input_float Input
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%float_0_5 = OpConstant %float 0.5
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %8
%18 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%19 = OpLoad %v4float %BaseColor
OpStore %v %19
%20 = OpLoad %float %f
%21 = OpFOrdLessThanEqual %bool %20 %float_0
OpSelectionMerge %22 None
OpBranchConditional %21 %23 %22
%23 = OpLabel
%24 = OpLoad %v4float %v
%25 = OpVectorTimesScalar %v4float %24 %float_0_5
OpStore %v %25
OpBranch %22
%22 = OpLabel
%26 = OpLoad %v4float %v
OpStore %gl_FragColor %26
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %8
%18 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%19 = OpLoad %v4float %BaseColor
OpStore %v %19
%20 = OpLoad %float %f
%21 = OpFOrdLessThanEqual %bool %20 %float_0
OpSelectionMerge %22 None
OpBranchConditional %21 %23 %22
%23 = OpLabel
%25 = OpVectorTimesScalar %v4float %19 %float_0_5
OpStore %v %25
OpBranch %22
%22 = OpLabel
%27 = OpPhi %v4float %19 %18 %25 %23
OpStore %gl_FragColor %27
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(LocalSSAElimTest, Switch) {
  // #version 140
  //
  // in vec4 BaseColor;
  // in float f;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     int i = int(f);
  //     switch (i) {
  //       case 0:
  //         v = v * 0.25;
  //         break;
  //       case 1:
  //         v = v * 0.625;
  //         break;
  //       case 2:
  //         v = v * 0.75;
  //         break;
  //       default:
  //         break;
  //     }
  //     gl_FragColor = v;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %f %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %i "i"
OpName %f "f"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Input_float = OpTypePointer Input %float
%f = OpVariable %_ptr_Input_float Input
%float_0_25 = OpConstant %float 0.25
%float_0_625 = OpConstant %float 0.625
%float_0_75 = OpConstant %float 0.75
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %9
%21 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%i = OpVariable %_ptr_Function_int Function
%22 = OpLoad %v4float %BaseColor
OpStore %v %22
%23 = OpLoad %float %f
%24 = OpConvertFToS %int %23
OpStore %i %24
%25 = OpLoad %int %i
OpSelectionMerge %26 None
OpSwitch %25 %27 0 %28 1 %29 2 %30
%27 = OpLabel
OpBranch %26
%28 = OpLabel
%31 = OpLoad %v4float %v
%32 = OpVectorTimesScalar %v4float %31 %float_0_25
OpStore %v %32
OpBranch %26
%29 = OpLabel
%33 = OpLoad %v4float %v
%34 = OpVectorTimesScalar %v4float %33 %float_0_625
OpStore %v %34
OpBranch %26
%30 = OpLabel
%35 = OpLoad %v4float %v
%36 = OpVectorTimesScalar %v4float %35 %float_0_75
OpStore %v %36
OpBranch %26
%26 = OpLabel
%37 = OpLoad %v4float %v
OpStore %gl_FragColor %37
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %9
%21 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%i = OpVariable %_ptr_Function_int Function
%22 = OpLoad %v4float %BaseColor
OpStore %v %22
%23 = OpLoad %float %f
%24 = OpConvertFToS %int %23
OpStore %i %24
OpSelectionMerge %26 None
OpSwitch %24 %27 0 %28 1 %29 2 %30
%27 = OpLabel
OpBranch %26
%28 = OpLabel
%32 = OpVectorTimesScalar %v4float %22 %float_0_25
OpStore %v %32
OpBranch %26
%29 = OpLabel
%34 = OpVectorTimesScalar %v4float %22 %float_0_625
OpStore %v %34
OpBranch %26
%30 = OpLabel
%36 = OpVectorTimesScalar %v4float %22 %float_0_75
OpStore %v %36
OpBranch %26
%26 = OpLabel
%38 = OpPhi %v4float %22 %27 %32 %28 %34 %29 %36 %30
OpStore %gl_FragColor %38
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(LocalSSAElimTest, SwitchWithFallThrough) {
  // #version 140
  //
  // in vec4 BaseColor;
  // in float f;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     int i = int(f);
  //     switch (i) {
  //       case 0:
  //         v = v * 0.25;
  //         break;
  //       case 1:
  //         v = v + 0.25;
  //       case 2:
  //         v = v * 0.75;
  //         break;
  //       default:
  //         break;
  //     }
  //     gl_FragColor = v;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %f %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %i "i"
OpName %f "f"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Input_float = OpTypePointer Input %float
%f = OpVariable %_ptr_Input_float Input
%float_0_25 = OpConstant %float 0.25
%float_0_75 = OpConstant %float 0.75
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %9
%20 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%i = OpVariable %_ptr_Function_int Function
%21 = OpLoad %v4float %BaseColor
OpStore %v %21
%22 = OpLoad %float %f
%23 = OpConvertFToS %int %22
OpStore %i %23
%24 = OpLoad %int %i
OpSelectionMerge %25 None
OpSwitch %24 %26 0 %27 1 %28 2 %29
%26 = OpLabel
OpBranch %25
%27 = OpLabel
%30 = OpLoad %v4float %v
%31 = OpVectorTimesScalar %v4float %30 %float_0_25
OpStore %v %31
OpBranch %25
%28 = OpLabel
%32 = OpLoad %v4float %v
%33 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%34 = OpFAdd %v4float %32 %33
OpStore %v %34
OpBranch %29
%29 = OpLabel
%35 = OpLoad %v4float %v
%36 = OpVectorTimesScalar %v4float %35 %float_0_75
OpStore %v %36
OpBranch %25
%25 = OpLabel
%37 = OpLoad %v4float %v
OpStore %gl_FragColor %37
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %9
%20 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%i = OpVariable %_ptr_Function_int Function
%21 = OpLoad %v4float %BaseColor
OpStore %v %21
%22 = OpLoad %float %f
%23 = OpConvertFToS %int %22
OpStore %i %23
OpSelectionMerge %25 None
OpSwitch %23 %26 0 %27 1 %28 2 %29
%26 = OpLabel
OpBranch %25
%27 = OpLabel
%31 = OpVectorTimesScalar %v4float %21 %float_0_25
OpStore %v %31
OpBranch %25
%28 = OpLabel
%33 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%34 = OpFAdd %v4float %21 %33
OpStore %v %34
OpBranch %29
%29 = OpLabel
%38 = OpPhi %v4float %21 %20 %34 %28
%36 = OpVectorTimesScalar %v4float %38 %float_0_75
OpStore %v %36
OpBranch %25
%25 = OpLabel
%39 = OpPhi %v4float %21 %26 %31 %27 %36 %29
OpStore %gl_FragColor %39
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(LocalSSAElimTest, DontPatchPhiInLoopHeaderThatIsNotAVar) {
  // From https://github.com/KhronosGroup/SPIRV-Tools/issues/826
  // Don't try patching the (%16 %7) value/predecessor pair in the OpPhi.
  // That OpPhi is unrelated to this optimization: we did not set that up
  // in the SSA initialization for the loop header block.
  // The pass should be a no-op on this module.

  const std::string before = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main"
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%1 = OpFunction %void None %3
%6 = OpLabel
OpBranch %7
%7 = OpLabel
%8 = OpPhi %float %float_1 %6 %9 %7
%9 = OpFAdd %float %8 %float_1
OpLoopMerge %10 %7 None
OpBranch %7
%10 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(before, before, true, true);
}

TEST_F(LocalSSAElimTest, OptInitializedVariableLikeStore) {
  // Note: SPIR-V edited to change store to v into variable initialization
  //
  // #version 450
  //
  // layout (location=0) in vec4 iColor;
  // layout (location=1) in float fi;
  // layout (location=0) out vec4 oColor;
  //
  // void main()
  // {
  //     vec4 v = vec4(0.0);
  //     if (fi < 0.0)
  //       v.x = iColor.x;
  //     oColor = v;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %fi %iColor %oColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %v "v"
OpName %fi "fi"
OpName %iColor "iColor"
OpName %oColor "oColor"
OpDecorate %fi Location 1
OpDecorate %iColor Location 0
OpDecorate %oColor Location 0
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%13 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_float = OpTypePointer Input %float
%fi = OpVariable %_ptr_Input_float Input
%bool = OpTypeBool
%_ptr_Input_v4float = OpTypePointer Input %v4float
%iColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%oColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %8
%21 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function %13
%22 = OpLoad %float %fi
%23 = OpFOrdLessThan %bool %22 %float_0
OpSelectionMerge %24 None
OpBranchConditional %23 %25 %24
%25 = OpLabel
%26 = OpAccessChain %_ptr_Input_float %iColor %uint_0
%27 = OpLoad %float %26
%28 = OpLoad %v4float %v
%29 = OpCompositeInsert %v4float %27 %28 0
OpStore %v %29
OpBranch %24
%24 = OpLabel
%30 = OpLoad %v4float %v
OpStore %oColor %30
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %8
%21 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function %13
%22 = OpLoad %float %fi
%23 = OpFOrdLessThan %bool %22 %float_0
OpSelectionMerge %24 None
OpBranchConditional %23 %25 %24
%25 = OpLabel
%26 = OpAccessChain %_ptr_Input_float %iColor %uint_0
%27 = OpLoad %float %26
%29 = OpCompositeInsert %v4float %27 %13 0
OpStore %v %29
OpBranch %24
%24 = OpLabel
%31 = OpPhi %v4float %13 %21 %29 %25
OpStore %oColor %31
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(predefs + func_before,
                                        predefs + func_after, true, true);
}

TEST_F(LocalSSAElimTest, PointerVariable) {
  // Test that checks if a pointer variable is removed.

  const std::string before =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "main" %2
OpExecutionMode %1 OriginUpperLeft
OpMemberDecorate %_struct_3 0 Offset 0
OpDecorate %_runtimearr__struct_3 ArrayStride 16
OpMemberDecorate %_struct_5 0 Offset 0
OpDecorate %_struct_5 BufferBlock
OpMemberDecorate %_struct_6 0 Offset 0
OpDecorate %_struct_6 BufferBlock
OpDecorate %2 Location 0
OpDecorate %7 DescriptorSet 0
OpDecorate %7 Binding 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_struct_3 = OpTypeStruct %v4float
%_runtimearr__struct_3 = OpTypeRuntimeArray %_struct_3
%_struct_5 = OpTypeStruct %_runtimearr__struct_3
%_ptr_Uniform__struct_5 = OpTypePointer Uniform %_struct_5
%_struct_6 = OpTypeStruct %int
%_ptr_Uniform__struct_6 = OpTypePointer Uniform %_struct_6
%_ptr_Function__ptr_Uniform__struct_5 = OpTypePointer Function %_ptr_Uniform__struct_5
%_ptr_Function__ptr_Uniform__struct_6 = OpTypePointer Function %_ptr_Uniform__struct_6
%int_0 = OpConstant %int 0
%uint_0 = OpConstant %uint 0
%2 = OpVariable %_ptr_Output_v4float Output
%7 = OpVariable %_ptr_Uniform__struct_5 Uniform
%1 = OpFunction %void None %10
%23 = OpLabel
%24 = OpVariable %_ptr_Function__ptr_Uniform__struct_5 Function
OpStore %24 %7
%26 = OpLoad %_ptr_Uniform__struct_5 %24
%27 = OpAccessChain %_ptr_Uniform_v4float %26 %int_0 %uint_0 %int_0
%28 = OpLoad %v4float %27
%29 = OpCopyObject %v4float %28
OpStore %2 %28
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "main" %2
OpExecutionMode %1 OriginUpperLeft
OpMemberDecorate %_struct_3 0 Offset 0
OpDecorate %_runtimearr__struct_3 ArrayStride 16
OpMemberDecorate %_struct_5 0 Offset 0
OpDecorate %_struct_5 BufferBlock
OpMemberDecorate %_struct_6 0 Offset 0
OpDecorate %_struct_6 BufferBlock
OpDecorate %2 Location 0
OpDecorate %7 DescriptorSet 0
OpDecorate %7 Binding 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_struct_3 = OpTypeStruct %v4float
%_runtimearr__struct_3 = OpTypeRuntimeArray %_struct_3
%_struct_5 = OpTypeStruct %_runtimearr__struct_3
%_ptr_Uniform__struct_5 = OpTypePointer Uniform %_struct_5
%_struct_6 = OpTypeStruct %int
%_ptr_Uniform__struct_6 = OpTypePointer Uniform %_struct_6
%_ptr_Function__ptr_Uniform__struct_5 = OpTypePointer Function %_ptr_Uniform__struct_5
%_ptr_Function__ptr_Uniform__struct_6 = OpTypePointer Function %_ptr_Uniform__struct_6
%int_0 = OpConstant %int 0
%uint_0 = OpConstant %uint 0
%2 = OpVariable %_ptr_Output_v4float Output
%7 = OpVariable %_ptr_Uniform__struct_5 Uniform
%1 = OpFunction %void None %10
%23 = OpLabel
%24 = OpVariable %_ptr_Function__ptr_Uniform__struct_5 Function
OpStore %24 %7
%27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0 %uint_0 %int_0
%28 = OpLoad %v4float %27
%29 = OpCopyObject %v4float %28
OpStore %2 %28
OpReturn
OpFunctionEnd
)";

  // Relax logical pointers to allow pointer allocations.
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ValidatorOptions()->relax_logical_pointer = true;
  SinglePassRunAndCheck<SSARewritePass>(before, after, true, true);
}

TEST_F(LocalSSAElimTest, VerifyInstToBlockMap) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     for (int i=0; i<4; i++) {
  //       f = f + BC[i];
  //     }
  //     fo = f;
  // }

  const std::string text = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %8
%22 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
OpBranch %23
%23 = OpLabel
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%27 = OpLoad %int %i
%28 = OpSLessThan %bool %27 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%30 = OpLoad %float %f
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
%34 = OpFAdd %float %30 %33
OpStore %f %34
OpBranch %25
%25 = OpLabel
%35 = OpLoad %int %i
%36 = OpIAdd %int %35 %int_1
OpStore %i %36
OpBranch %23
%24 = OpLabel
%37 = OpLoad %float %f
OpStore %fo %37
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  EXPECT_NE(nullptr, context);

  // Force the instruction to block mapping to get built.
  context->get_instr_block(27u);

  auto pass = MakeUnique<SSARewritePass>();
  pass->SetMessageConsumer(nullptr);
  const auto status = pass->Run(context.get());
  EXPECT_TRUE(status == Pass::Status::SuccessWithChange);
}

TEST_F(LocalSSAElimTest, CompositeExtractProblem) {
  const std::string spv_asm = R"(
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %2 "main" %16 %17 %18 %20 %22 %26 %27 %30 %31
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
 %_struct_11 = OpTypeStruct %v4float %v4float %v4float %v3float %v3float %v2float %v2float
%_arr__struct_11_uint_3 = OpTypeArray %_struct_11 %uint_3
%_ptr_Function__arr__struct_11_uint_3 = OpTypePointer Function %_arr__struct_11_uint_3
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_ptr_Input__arr_v4float_uint_3 = OpTypePointer Input %_arr_v4float_uint_3
         %16 = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
         %17 = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
         %18 = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
%_ptr_Input_uint = OpTypePointer Input %uint
         %20 = OpVariable %_ptr_Input_uint Input
%_ptr_Output__arr_v4float_uint_3 = OpTypePointer Output %_arr_v4float_uint_3
         %22 = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v3float_uint_3 = OpTypeArray %v3float %uint_3
%_ptr_Input__arr_v3float_uint_3 = OpTypePointer Input %_arr_v3float_uint_3
         %26 = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
         %27 = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
%_ptr_Input__arr_v2float_uint_3 = OpTypePointer Input %_arr_v2float_uint_3
         %30 = OpVariable %_ptr_Input__arr_v2float_uint_3 Input
         %31 = OpVariable %_ptr_Input__arr_v2float_uint_3 Input
%_ptr_Function__struct_11 = OpTypePointer Function %_struct_11
          %2 = OpFunction %void None %4
         %33 = OpLabel
         %66 = OpVariable %_ptr_Function__arr__struct_11_uint_3 Function
         %34 = OpLoad %_arr_v4float_uint_3 %16
         %35 = OpLoad %_arr_v4float_uint_3 %17
         %36 = OpLoad %_arr_v4float_uint_3 %18
         %37 = OpLoad %_arr_v3float_uint_3 %26
         %38 = OpLoad %_arr_v3float_uint_3 %27
         %39 = OpLoad %_arr_v2float_uint_3 %30
         %40 = OpLoad %_arr_v2float_uint_3 %31
         %41 = OpCompositeExtract %v4float %34 0
         %42 = OpCompositeExtract %v4float %35 0
         %43 = OpCompositeExtract %v4float %36 0
         %44 = OpCompositeExtract %v3float %37 0
         %45 = OpCompositeExtract %v3float %38 0
         %46 = OpCompositeExtract %v2float %39 0
         %47 = OpCompositeExtract %v2float %40 0
         %48 = OpCompositeConstruct %_struct_11 %41 %42 %43 %44 %45 %46 %47
         %49 = OpCompositeExtract %v4float %34 1
         %50 = OpCompositeExtract %v4float %35 1
         %51 = OpCompositeExtract %v4float %36 1
         %52 = OpCompositeExtract %v3float %37 1
         %53 = OpCompositeExtract %v3float %38 1
         %54 = OpCompositeExtract %v2float %39 1
         %55 = OpCompositeExtract %v2float %40 1
         %56 = OpCompositeConstruct %_struct_11 %49 %50 %51 %52 %53 %54 %55
         %57 = OpCompositeExtract %v4float %34 2
         %58 = OpCompositeExtract %v4float %35 2
         %59 = OpCompositeExtract %v4float %36 2
         %60 = OpCompositeExtract %v3float %37 2
         %61 = OpCompositeExtract %v3float %38 2
         %62 = OpCompositeExtract %v2float %39 2
         %63 = OpCompositeExtract %v2float %40 2
         %64 = OpCompositeConstruct %_struct_11 %57 %58 %59 %60 %61 %62 %63
         %65 = OpCompositeConstruct %_arr__struct_11_uint_3 %48 %56 %64
         %67 = OpLoad %uint %20

; CHECK OpStore {{%\d+}} [[store_source:%\d+]]
               OpStore %66 %65
         %68 = OpAccessChain %_ptr_Function__struct_11 %66 %67

; This load was being removed, because %_ptr_Function__struct_11 was being
; wrongfully considered an SSA target.
; CHECK OpLoad %_struct_11 %68
         %69 = OpLoad %_struct_11 %68

; Similarly, %69 cannot be replaced with %65.
; CHECK-NOT: OpCompositeExtract %v4float [[store_source]] 0
         %70 = OpCompositeExtract %v4float %69 0

         %71 = OpAccessChain %_ptr_Output_v4float %22 %67
               OpStore %71 %70
               OpReturn
               OpFunctionEnd)";

  SinglePassRunAndMatch<SSARewritePass>(spv_asm, true);
}

// Test that the RelaxedPrecision decoration on the variable to added to the
// result of the OpPhi instruction.
TEST_F(LocalSSAElimTest, DecoratedVariable) {
  const std::string spv_asm = R"(
; CHECK: OpDecorate [[var:%\w+]] RelaxedPrecision
; CHECK: OpDecorate [[phi_id:%\w+]] RelaxedPrecision
; CHECK: [[phi_id]] = OpPhi
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main"
               OpDecorate %v RelaxedPrecision
       %void = OpTypeVoid
     %func_t = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %int  = OpTypeInt 32 0
      %int_p = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
          %2 = OpFunction %void None %func_t
         %33 = OpLabel
         %v  = OpVariable %int_p Function
               OpSelectionMerge %merge None
               OpBranchConditional %true %l1 %l2
         %l1 = OpLabel
               OpStore %v %int_1
               OpBranch %merge
         %l2 = OpLabel
               OpStore %v %int_0
               OpBranch %merge
      %merge = OpLabel
         %ld = OpLoad %int %v
               OpReturn
               OpFunctionEnd)";

  SinglePassRunAndMatch<SSARewritePass>(spv_asm, true);
}

// Test that the RelaxedPrecision decoration on the variable to added to the
// result of the OpPhi instruction.
TEST_F(LocalSSAElimTest, MultipleEdges) {
  const std::string spv_asm = R"(
  ; CHECK: OpSelectionMerge
  ; CHECK: [[header_bb:%\w+]] = OpLabel
  ; CHECK-NOT: OpLabel
  ; CHECK: OpSwitch {{%\w+}} {{%\w+}} 76 [[bb1:%\w+]] 17 [[bb2:%\w+]]
  ; CHECK-SAME: 4 [[bb2]]
  ; CHECK: [[bb2]] = OpLabel
  ; CHECK-NEXT: OpPhi [[type:%\w+]] [[val:%\w+]] [[header_bb]] %int_0 [[bb1]]
          OpCapability Shader
     %1 = OpExtInstImport "GLSL.std.450"
          OpMemoryModel Logical GLSL450
          OpEntryPoint Fragment %4 "main"
          OpExecutionMode %4 OriginUpperLeft
          OpSource ESSL 310
  %void = OpTypeVoid
     %3 = OpTypeFunction %void
   %int = OpTypeInt 32 1
  %_ptr_Function_int = OpTypePointer Function %int
  %int_0 = OpConstant %int 0
  %bool = OpTypeBool
  %true = OpConstantTrue %bool
  %false = OpConstantFalse %bool
  %int_1 = OpConstant %int 1
     %4 = OpFunction %void None %3
     %5 = OpLabel
     %8 = OpVariable %_ptr_Function_int Function
          OpBranch %10
    %10 = OpLabel
          OpLoopMerge %12 %13 None
          OpBranch %14
    %14 = OpLabel
          OpBranchConditional %true %11 %12
    %11 = OpLabel
          OpSelectionMerge %19 None
          OpBranchConditional %false %18 %19
    %18 = OpLabel
          OpSelectionMerge %22 None
          OpSwitch %int_0 %22 76 %20 17 %21 4 %21
    %20 = OpLabel
    %23 = OpLoad %int %8
          OpStore %8 %int_0
          OpBranch %21
    %21 = OpLabel
          OpBranch %22
    %22 = OpLabel
          OpBranch %19
    %19 = OpLabel
          OpBranch %13
    %13 = OpLabel
          OpBranch %10
    %12 = OpLabel
          OpReturn
          OpFunctionEnd
  )";

  SinglePassRunAndMatch<SSARewritePass>(spv_asm, true);
}

TEST_F(LocalSSAElimTest, VariablePointerTest1) {
  // Check that the load of the first variable is still used and that the load
  // of the third variable is propagated.  The first load has to remain because
  // of the store to the variable pointer.
  const std::string text = R"(
; CHECK: [[v1:%\w+]] = OpVariable
; CHECK: [[v2:%\w+]] = OpVariable
; CHECK: [[v3:%\w+]] = OpVariable
; CHECK: [[ld1:%\w+]] = OpLoad %int [[v1]]
; CHECK: OpIAdd %int [[ld1]] %int_0
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main"
               OpExecutionMode %2 LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %_struct_3 0 Offset 0
               OpMemberDecorate %_struct_3 1 Offset 4
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %bool = OpTypeBool
  %_struct_3 = OpTypeStruct %int %int
%_ptr_Function__struct_3 = OpTypePointer Function %_struct_3
%_ptr_Function_int = OpTypePointer Function %int
       %true = OpConstantTrue %bool
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
         %13 = OpConstantNull %_struct_3
          %2 = OpFunction %void None %5
         %14 = OpLabel
         %15 = OpVariable %_ptr_Function_int Function
         %16 = OpVariable %_ptr_Function_int Function
         %17 = OpVariable %_ptr_Function_int Function
               OpStore %15 %int_1
               OpStore %17 %int_0
               OpSelectionMerge %18 None
               OpBranchConditional %true %19 %20
         %19 = OpLabel
               OpBranch %18
         %20 = OpLabel
               OpBranch %18
         %18 = OpLabel
         %21 = OpPhi %_ptr_Function_int %15 %19 %16 %20
               OpStore %21 %int_0
         %22 = OpLoad %int %15
         %23 = OpLoad %int %17
         %24 = OpIAdd %int %22 %23
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<SSARewritePass>(text, false);
}

TEST_F(LocalSSAElimTest, VariablePointerTest2) {
  // Check that the load of the first variable is still used and that the load
  // of the third variable is propagated.  The first load has to remain because
  // of the store to the variable pointer.
  const std::string text = R"(
; CHECK: [[v1:%\w+]] = OpVariable
; CHECK: [[v2:%\w+]] = OpVariable
; CHECK: [[v3:%\w+]] = OpVariable
; CHECK: [[ld1:%\w+]] = OpLoad %int [[v1]]
; CHECK: OpIAdd %int [[ld1]] %int_0
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main"
               OpExecutionMode %2 LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %_struct_3 0 Offset 0
               OpMemberDecorate %_struct_3 1 Offset 4
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %bool = OpTypeBool
  %_struct_3 = OpTypeStruct %int %int
%_ptr_Function__struct_3 = OpTypePointer Function %_struct_3
%_ptr_Function_int = OpTypePointer Function %int
       %true = OpConstantTrue %bool
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
         %13 = OpConstantNull %_struct_3
          %2 = OpFunction %void None %5
         %14 = OpLabel
         %15 = OpVariable %_ptr_Function_int Function
         %16 = OpVariable %_ptr_Function_int Function
         %17 = OpVariable %_ptr_Function_int Function
               OpStore %15 %int_1
               OpStore %17 %int_0
               OpSelectionMerge %18 None
               OpBranchConditional %true %19 %20
         %19 = OpLabel
               OpBranch %18
         %20 = OpLabel
               OpBranch %18
         %18 = OpLabel
         %21 = OpPhi %_ptr_Function_int %15 %19 %16 %20
               OpStore %21 %int_0
         %22 = OpLoad %int %15
         %23 = OpLoad %int %17
         %24 = OpIAdd %int %22 %23
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<SSARewritePass>(text, false);
}

TEST_F(LocalSSAElimTest, ChainedTrivialPhis) {
  // Check that the copy object get the undef value implicitly assigned in the
  // entry block.
  const std::string text = R"(
; CHECK: [[undef:%\w+]] = OpUndef %v4float
; CHECK: OpCopyObject %v4float [[undef]]
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main"
               OpExecutionMode %2 LocalSize 1 18 6
               OpSource ESSL 310
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %bool = OpTypeBool
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
          %2 = OpFunction %void None %4
          %9 = OpLabel
         %10 = OpVariable %_ptr_Function_v4float Function
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %15 = OpUndef %bool
               OpBranchConditional %15 %16 %12
         %16 = OpLabel
         %17 = OpUndef %bool
               OpSelectionMerge %18 None
               OpBranchConditional %17 %19 %18
         %19 = OpLabel
         %20 = OpUndef %bool
               OpLoopMerge %21 %22 None
               OpBranchConditional %20 %23 %21
         %23 = OpLabel
         %24 = OpLoad %v4float %10
         %25 = OpCopyObject %v4float %24
         %26 = OpUndef %bool
               OpBranch %22
         %22 = OpLabel
               OpBranch %19
         %21 = OpLabel
               OpBranch %12
         %18 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpBranch %11
         %12 = OpLabel
         %27 = OpLoad %v4float %10
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<SSARewritePass>(text, false);
}

TEST_F(LocalSSAElimTest, Overflowtest1) {
  // Check that the copy object get the undef value implicitly assigned in the
  // entry block.
  const std::string text = R"(
OpCapability Geometry
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "P2Mai" %12 %17
OpExecutionMode %4 OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeVector %6 4
%11 = OpTypePointer Input %7
%16 = OpTypePointer Output %7
%23 = OpTypePointer Function %7
%12 = OpVariable %11 Input
%17 = OpVariable %16 Output
%4 = OpFunction %2 None %3
%2177 = OpLabel
%4194302 = OpVariable %23 Function
%4194301 = OpLoad %7 %4194302
OpStore %17 %4194301
OpReturn
OpFunctionEnd
  )";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<SSARewritePass>(text, true);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

TEST_F(LocalSSAElimTest, OpConstantNull) {
  const std::string text = R"(
OpCapability Addresses
OpCapability Kernel
OpCapability Int64
OpMemoryModel Physical64 OpenCL
OpEntryPoint Kernel %4 "A"
OpSource OpenCL_C 200000
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%11 = OpTypePointer CrossWorkgroup %6
%16 = OpConstantNull %11
%20 = OpConstant %6 269484031
%4 = OpFunction %2 None %3
%17 = OpLabel
%18 = OpLoad %6 %16 Aligned 536870912
%19 = OpBitwiseXor %6 %18 %20
OpStore %16 %19 Aligned 536870912
OpReturn
OpFunctionEnd
  )";

  SinglePassRunToBinary<SSARewritePass>(text, false);
}

TEST_F(LocalSSAElimTest, DebugForLoop) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     for (int i=0; i<4; i++) {
  //       f = f + BC[i];
  //     }
  //     fo = f;
  // }

  const std::string text = R"(
; CHECK: [[f_name:%\w+]] = OpString "f"
; CHECK: [[i_name:%\w+]] = OpString "i"
; CHECK: [[dbg_f:%\w+]] = OpExtInst %void [[ext:%\d+]] DebugLocalVariable [[f_name]]
; CHECK: [[dbg_i:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[i_name]]

; CHECK:      OpStore %f %float_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] %float_0
; CHECK-NEXT: OpStore %i %int_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] %int_0

; CHECK-NOT:  DebugDeclare

; CHECK:      [[loop_head:%\w+]] = OpLabel
; CHECK:      [[phi0:%\w+]] = OpPhi %float %float_0
; CHECK:      [[phi1:%\w+]] = OpPhi %int %int_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[phi0]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] [[phi1]]
; CHECK:      OpLoopMerge [[loop_merge:%\w+]] [[loop_cont:%\w+]] None
; CHECK-NEXT: OpBranch [[loop_body:%\w+]]

; CHECK-NEXT: [[loop_body]] = OpLabel
; CHECK:      OpBranchConditional {{%\w+}} [[bb:%\w+]] [[loop_merge]]

; CHECK:      [[bb]] = OpLabel
; CHECK:      OpStore %f [[f_val:%\w+]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[f_val]]
; CHECK-NEXT: OpBranch [[loop_cont]]

; CHECK:      [[loop_cont]] = OpLabel
; CHECK:      OpStore %i [[i_val:%\w+]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] [[i_val]]
; CHECK-NEXT: OpBranch [[loop_head]]

; CHECK:      [[loop_merge]] = OpLabel

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
OpSource GLSL 140
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
%i_name = OpString "i"
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_tf 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_v4f %src 0 0 %dbg_main FlagIsLocal
%dbg_i = OpExtInst %void %ext DebugLocalVariable %i_name %dbg_v4f %src 0 0 %dbg_main FlagIsLocal
%main = OpFunction %void None %8
%22 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
%decl0 = OpExtInst %void %ext DebugDeclare %dbg_f %f %null_expr
%decl1 = OpExtInst %void %ext DebugDeclare %dbg_i %i %null_expr
OpBranch %23
%23 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%27 = OpLoad %int %i
%28 = OpSLessThan %bool %27 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%30 = OpLoad %float %f
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
%34 = OpFAdd %float %30 %33
OpStore %f %34
OpBranch %25
%25 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%35 = OpLoad %int %i
%36 = OpIAdd %int %35 %int_1
OpStore %i %36
OpBranch %23
%24 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%37 = OpLoad %float %f
OpStore %fo %37
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, ShaderDebugForLoop) {
  const std::string text = R"(
; CHECK: [[f_name:%\w+]] = OpString "f"
; CHECK: [[i_name:%\w+]] = OpString "i"
; CHECK: [[dbg_f:%\w+]] = OpExtInst %void [[ext:%\d+]] DebugLocalVariable [[f_name]]
; CHECK: [[dbg_i:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[i_name]]

; CHECK:      OpStore %f %float_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] %float_0
; CHECK-NEXT: OpStore %i %int_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] %int_0

; CHECK-NOT:  DebugDeclare

; CHECK:      [[loop_head:%\w+]] = OpLabel
; CHECK:      [[phi0:%\w+]] = OpPhi %float %float_0
; CHECK:      [[phi1:%\w+]] = OpPhi %int %int_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[phi0]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] [[phi1]]
; CHECK:      OpLoopMerge [[loop_merge:%\w+]] [[loop_cont:%\w+]] None
; CHECK-NEXT: OpBranch [[loop_body:%\w+]]

; CHECK-NEXT: [[loop_body]] = OpLabel
; CHECK:      OpBranchConditional {{%\w+}} [[bb:%\w+]] [[loop_merge]]

; CHECK:      [[bb]] = OpLabel
; CHECK:      OpStore %f [[f_val:%\w+]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[f_val]]
; CHECK-NEXT: OpBranch [[loop_cont]]

; CHECK:      [[loop_cont]] = OpLabel
; CHECK:      OpStore %i [[i_val:%\w+]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] [[i_val]]
; CHECK-NEXT: OpBranch [[loop_head]]

; CHECK:      [[loop_merge]] = OpLabel

OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
OpSource GLSL 140
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
%i_name = OpString "i"
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_3 = OpConstant %uint 3
%uint_4 = OpConstant %uint 4
%uint_5 = OpConstant %uint 5
%uint_10 = OpConstant %uint 10
%uint_32 = OpConstant %uint 32
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit %uint_1 %uint_4 %src %uint_5
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 %uint_3 %uint_0
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_tf %uint_4
%main_ty = OpExtInst %void %ext DebugTypeFunction %uint_3 %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src %uint_0 %uint_0 %cu %main_name %uint_3 %uint_10
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_v4f %src %uint_0 %uint_0 %dbg_main %uint_4
%dbg_i = OpExtInst %void %ext DebugLocalVariable %i_name %dbg_v4f %src %uint_0 %uint_0 %dbg_main %uint_4
%main = OpFunction %void None %8
%22 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
%decl0 = OpExtInst %void %ext DebugDeclare %dbg_f %f %null_expr
%decl1 = OpExtInst %void %ext DebugDeclare %dbg_i %i %null_expr
OpBranch %23
%23 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%27 = OpLoad %int %i
%28 = OpSLessThan %bool %27 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%30 = OpLoad %float %f
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
%34 = OpFAdd %float %30 %33
OpStore %f %34
OpBranch %25
%25 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%35 = OpLoad %int %i
%36 = OpIAdd %int %35 %int_1
OpStore %i %36
OpBranch %23
%24 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%37 = OpLoad %float %f
OpStore %fo %37
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, AddDebugValueForFunctionParameterWithPhi) {
  // Test the distribution of DebugValue for a parameter of an inlined function
  // and the visibility of Phi instruction. The ssa-rewrite pass must add
  // DebugValue for the value assignment of function argument even when it is an
  // inlined function. It has to check the visibility Phi through all its value
  // operands. See the DebugValue for "int i" of "foo()" in the following code.
  //
  // struct VS_OUTPUT {
  //   float4 pos : SV_POSITION;
  //   float4 color : COLOR;
  // };
  //
  // float4 foo(int i, float4 pos) {
  //   while (i < pos.x) {
  //     pos = pos.x + i;
  //     ++i;
  //   }
  //   return pos;
  // }
  //
  // VS_OUTPUT main(float4 pos : POSITION,
  //                float4 color : COLOR) {
  //   VS_OUTPUT vout;
  //   vout.pos = foo(4, pos);
  //   vout.color = color;
  //   return vout;
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %in_var_POSITION %in_var_COLOR %gl_Position %out_var_COLOR
          %7 = OpString "vertex.hlsl"
          %8 = OpString "float"
          %9 = OpString "VS_OUTPUT"
         %10 = OpString "color"
         %11 = OpString "pos"
         %12 = OpString "int"
         %13 = OpString "foo"
         %14 = OpString ""
         %15 = OpString "i"
         %16 = OpString "main"
         %17 = OpString "vout"
               OpName %in_var_POSITION "in.var.POSITION"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %out_var_COLOR "out.var.COLOR"
               OpName %main "main"
               OpName %param_var_pos "param.var.pos"
               OpName %param_var_color "param.var.color"
               OpName %VS_OUTPUT "VS_OUTPUT"
               OpMemberName %VS_OUTPUT 0 "pos"
               OpMemberName %VS_OUTPUT 1 "color"
               OpDecorate %gl_Position BuiltIn Position
               OpDecorate %in_var_POSITION Location 0
               OpDecorate %in_var_COLOR Location 1
               OpDecorate %out_var_COLOR Location 0
        %int = OpTypeInt 32 1
      %int_4 = OpConstant %int 4
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
   %uint_256 = OpConstant %uint 256
   %uint_128 = OpConstant %uint 128
     %uint_0 = OpConstant %uint 0
         %50 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
  %VS_OUTPUT = OpTypeStruct %v4float %v4float
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
       %bool = OpTypeBool
%in_var_POSITION = OpVariable %_ptr_Input_v4float Input
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%gl_Position = OpVariable %_ptr_Output_v4float Output
%out_var_COLOR = OpVariable %_ptr_Output_v4float Output
        %156 = OpExtInst %void %1 DebugInfoNone
         %77 = OpExtInst %void %1 DebugExpression
         %58 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 Float
         %59 = OpExtInst %void %1 DebugTypeVector %58 4
         %60 = OpExtInst %void %1 DebugSource %7
         %61 = OpExtInst %void %1 DebugCompilationUnit 1 4 %60 HLSL
         %62 = OpExtInst %void %1 DebugTypeComposite %9 Structure %60 1 8 %61 %9 %uint_256 FlagIsProtected|FlagIsPrivate %63 %64
         %64 = OpExtInst %void %1 DebugTypeMember %10 %59 %60 3 10 %62 %uint_128 %uint_128 FlagIsProtected|FlagIsPrivate
         %63 = OpExtInst %void %1 DebugTypeMember %11 %59 %60 2 10 %62 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %65 = OpExtInst %void %1 DebugTypeBasic %12 %uint_32 Signed
         %66 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %59 %65 %59
         %67 = OpExtInst %void %1 DebugFunction %13 %66 %60 6 1 %61 %14 FlagIsProtected|FlagIsPrivate 6 %156
         %68 = OpExtInst %void %1 DebugLexicalBlock %60 6 31 %67
         %69 = OpExtInst %void %1 DebugLexicalBlock %60 7 21 %68
         %70 = OpExtInst %void %1 DebugLocalVariable %11 %59 %60 6 26 %67 FlagIsLocal 2

; CHECK: [[color_name:%\w+]] = OpString "color"
; CHECK: [[pos_name:%\w+]] = OpString "pos"
; CHECK: [[i_name:%\w+]] = OpString "i"
; CHECK: [[null_expr:%\w+]] = OpExtInst %void [[ext:%\w+]] DebugExpression
; CHECK: [[dbg_i:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[i_name]] {{%\w+}} {{%\w+}} 6 16 {{%\w+}} FlagIsLocal 1
; CHECK: [[dbg_color:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[color_name]] {{%\w+}} {{%\w+}} 15 23
; CHECK: [[dbg_pos:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[pos_name]] {{%\w+}} {{%\w+}} 14 23
         %71 = OpExtInst %void %1 DebugLocalVariable %15 %65 %60 6 16 %67 FlagIsLocal 1
         %72 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %62 %59 %59
         %73 = OpExtInst %void %1 DebugFunction %16 %72 %60 14 1 %61 %14 FlagIsProtected|FlagIsPrivate 15 %156
         %74 = OpExtInst %void %1 DebugLexicalBlock %60 15 38 %73
         %75 = OpExtInst %void %1 DebugLocalVariable %17 %62 %60 16 13 %74 FlagIsLocal
         %76 = OpExtInst %void %1 DebugLocalVariable %10 %59 %60 15 23 %73 FlagIsLocal 2
         %78 = OpExtInst %void %1 DebugLocalVariable %11 %59 %60 14 23 %73 FlagIsLocal 1
        %155 = OpExtInst %void %1 DebugInlinedAt 17 %74
       %main = OpFunction %void None %50
         %79 = OpLabel
        %168 = OpExtInst %void %1 DebugScope %74

; CHECK: [[i:%\w+]] = OpVariable %_ptr_Function_int Function
        %120 = OpVariable %_ptr_Function_int Function
        %121 = OpVariable %_ptr_Function_v4float Function
        %169 = OpExtInst %void %1 DebugNoScope
%param_var_pos = OpVariable %_ptr_Function_v4float Function
%param_var_color = OpVariable %_ptr_Function_v4float Function
               OpLine %7 100 105
         %80 = OpLoad %v4float %in_var_POSITION
               OpStore %param_var_pos %80
               OpNoLine
               OpLine %7 200 205
         %81 = OpLoad %v4float %in_var_COLOR
               OpStore %param_var_color %81
               OpNoLine
        %170 = OpExtInst %void %1 DebugScope %73

; CHECK: OpLine {{%\w+}} 100 105
; CHECK: DebugValue [[dbg_pos]]
        %124 = OpExtInst %void %1 DebugDeclare %78 %param_var_pos %77
; CHECK: OpLine {{%\w+}} 200 205
; CHECK: DebugValue [[dbg_color]]
        %125 = OpExtInst %void %1 DebugDeclare %76 %param_var_color %77

        %171 = OpExtInst %void %1 DebugScope %74
               OpLine %7 17 18

; CHECK: OpStore {{%\w+}} %int_4
; CHECK: DebugValue [[dbg_i]] %int_4 [[null_expr]]
               OpStore %120 %int_4
               OpStore %121 %80
        %172 = OpExtInst %void %1 DebugScope %67 %155
        %135 = OpExtInst %void %1 DebugDeclare %71 %120 %77
        %136 = OpExtInst %void %1 DebugDeclare %70 %121 %77
        %173 = OpExtInst %void %1 DebugScope %68 %155
               OpLine %7 7 3
               OpBranch %137
        %174 = OpExtInst %void %1 DebugNoScope
        %137 = OpLabel

; CHECK: [[phi:%\w+]] = OpPhi %int %int_4
; CHECK: DebugValue [[dbg_i]] [[phi]] [[null_expr]]
        %175 = OpExtInst %void %1 DebugScope %68 %155
               OpLine %7 7 10
        %138 = OpLoad %int %120
        %139 = OpConvertSToF %float %138
               OpLine %7 7 14
        %140 = OpAccessChain %_ptr_Function_float %121 %int_0
        %141 = OpLoad %float %140
               OpLine %7 7 12
        %142 = OpFOrdLessThan %bool %139 %141
               OpLine %7 7 3
        %176 = OpExtInst %void %1 DebugNoScope
               OpLoopMerge %153 %152 None
               OpBranchConditional %142 %143 %153
        %177 = OpExtInst %void %1 DebugNoScope
        %143 = OpLabel
        %178 = OpExtInst %void %1 DebugScope %69 %155
               OpLine %7 8 11
        %144 = OpAccessChain %_ptr_Function_float %121 %int_0
        %145 = OpLoad %float %144
               OpLine %7 8 19
        %146 = OpLoad %int %120
        %147 = OpConvertSToF %float %146
               OpLine %7 8 17
        %148 = OpFAdd %float %145 %147
               OpLine %7 8 11
        %149 = OpCompositeConstruct %v4float %148 %148 %148 %148
               OpLine %7 8 5
               OpStore %121 %149
               OpLine %7 9 5
        %151 = OpIAdd %int %146 %int_1
               OpLine %7 9 7

; CHECK: OpStore [[i]] [[value:%\w+]]
; CHECK: DebugValue [[dbg_i]] [[value]] [[null_expr]]
               OpStore %120 %151
        %179 = OpExtInst %void %1 DebugScope %68 %155
               OpLine %7 10 3
               OpBranch %152
        %180 = OpExtInst %void %1 DebugNoScope
        %152 = OpLabel
        %181 = OpExtInst %void %1 DebugScope %68 %155
               OpBranch %137
        %182 = OpExtInst %void %1 DebugNoScope
        %153 = OpLabel
        %183 = OpExtInst %void %1 DebugScope %68 %155
               OpLine %7 11 10
        %154 = OpLoad %v4float %121
        %184 = OpExtInst %void %1 DebugScope %74
        %167 = OpExtInst %void %1 DebugValue %75 %154 %77 %int_0
        %166 = OpExtInst %void %1 DebugValue %75 %81 %77 %int_1
               OpLine %7 19 10
        %165 = OpCompositeConstruct %VS_OUTPUT %154 %81
        %185 = OpExtInst %void %1 DebugNoScope
         %83 = OpCompositeExtract %v4float %165 0
               OpStore %gl_Position %83
         %84 = OpCompositeExtract %v4float %165 1
               OpStore %out_var_COLOR %84
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, DebugValueWithIndexesInForLoop) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // struct T {
  //   float a;
  //   float f;
  // };
  //
  // struct value {
  //   int x;
  //   int y;
  //   T z;
  // };
  //
  // void main()
  // {
  //     value v;
  //     v.z.f = 0.0;
  //     for (int i=0; i<4; i++) {
  //       v.z.f = v.z.f + BC[i];
  //     }
  //     fo = v.z.f;
  // }

  const std::string text = R"(
; CHECK: [[f_name:%\w+]] = OpString "f"
; CHECK: [[dbg_f:%\w+]] = OpExtInst %void [[ext:%\d+]] DebugLocalVariable [[f_name]]

; CHECK:      OpStore %f %float_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] %float_0 [[null_expr:%\d+]] %int_2 %int_1

; CHECK-NOT:  DebugDeclare

; CHECK:      [[loop_head:%\w+]] = OpLabel
; CHECK:      [[phi0:%\w+]] = OpPhi %float %float_0
; CHECK:      OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[phi0]] [[null_expr]] %int_2 %int_1
; CHECK:      OpLoopMerge [[loop_merge:%\w+]] [[loop_cont:%\w+]] None
; CHECK-NEXT: OpBranch [[loop_body:%\w+]]

; CHECK-NEXT: [[loop_body]] = OpLabel
; CHECK:      OpBranchConditional {{%\w+}} [[bb:%\w+]] [[loop_merge]]

; CHECK:      [[bb]] = OpLabel
; CHECK:      OpStore %f [[f_val:%\w+]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[f_val]] [[null_expr]] %int_2 %int_1
; CHECK-NEXT: OpBranch [[loop_cont]]

; CHECK: [[loop_cont]] = OpLabel
; CHECK: OpBranch [[loop_head]]

; CHECK: [[loop_merge]] = OpLabel

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
OpSource GLSL 140
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
%i_name = OpString "i"
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
%deref = OpExtInst %void %ext DebugOperation Deref
%null_expr = OpExtInst %void %ext DebugExpression
%deref_expr = OpExtInst %void %ext DebugExpression %deref
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_tf 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_v4f %src 0 0 %dbg_main FlagIsLocal
%dbg_i = OpExtInst %void %ext DebugLocalVariable %i_name %dbg_v4f %src 0 0 %dbg_main FlagIsLocal
%main = OpFunction %void None %8
%22 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
%decl0 = OpExtInst %void %ext DebugValue %dbg_f %f %deref_expr %int_2 %int_1
%decl1 = OpExtInst %void %ext DebugDeclare %dbg_i %i %null_expr
OpBranch %23
%23 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%27 = OpLoad %int %i
%28 = OpSLessThan %bool %27 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%30 = OpLoad %float %f
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
%34 = OpFAdd %float %30 %33
OpStore %f %34
OpBranch %25
%25 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%35 = OpLoad %int %i
%36 = OpIAdd %int %35 %int_1
OpStore %i %36
OpBranch %23
%24 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%37 = OpLoad %float %f
OpStore %fo %37
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, PartiallyKillDebugDeclare) {
  // For a reference variable e.g., int i in the following example,
  // we do not propagate DebugValue for a store or phi instruction
  // out of the variable's scope. In that case, we should not remove
  // DebugDeclare for the variable that we did not add its DebugValue.
  //
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // int j;
  // void main()
  // {
  //     float f = 0.0;
  //     for (j=0; j<4; j++) {
  //       int& i = j;
  //       f = f + BC[i];
  //     }
  //     fo = f;
  // }

  const std::string text = R"(
; CHECK: [[f_name:%\w+]] = OpString "f"
; CHECK: [[i_name:%\w+]] = OpString "i"
; CHECK: [[fn:%\w+]] = OpExtInst %void [[ext:%\d+]] DebugFunction
; CHECK: [[bb:%\w+]] = OpExtInst %void [[ext]] DebugLexicalBlock
; CHECK: [[dbg_f:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[f_name]] {{%\w+}} {{%\w+}} 0 0 [[fn]]
; CHECK: [[dbg_i:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[i_name]] {{%\w+}} {{%\w+}} 0 0 [[bb]]

; CHECK:      OpStore %f %float_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] %float_0
; CHECK-NOT:  DebugDeclare [[dbg_f]]
; CHECK:      OpExtInst %void [[ext]] DebugDeclare [[dbg_i]] %j

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
OpSource GLSL 140
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
%i_name = OpString "i"
%j_name = OpString "j"
OpName %main "main"
OpName %f "f"
OpName %j "j"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Private_int = OpTypePointer Private %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
%j = OpVariable %_ptr_Private_int Private
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_tf 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%bb = OpExtInst %void %ext DebugLexicalBlock %src 0 0 %dbg_main
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_v4f %src 0 0 %dbg_main FlagIsLocal
%dbg_i = OpExtInst %void %ext DebugLocalVariable %i_name %dbg_v4f %src 0 0 %bb FlagIsLocal
%dbg_j = OpExtInst %void %ext DebugGlobalVariable %j_name %dbg_v4f %src 0 0 %dbg_main %j_name %j FlagIsPrivate
%main = OpFunction %void None %8
%22 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%f = OpVariable %_ptr_Function_float Function
OpStore %f %float_0
OpStore %j %int_0
%decl0 = OpExtInst %void %ext DebugDeclare %dbg_f %f %null_expr
OpBranch %23
%23 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%27 = OpLoad %int %j
%28 = OpSLessThan %bool %27 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %bb
%decl1 = OpExtInst %void %ext DebugDeclare %dbg_i %j %null_expr
%30 = OpLoad %float %f
%31 = OpLoad %int %j
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
%34 = OpFAdd %float %30 %33
OpStore %f %34
OpBranch %25
%25 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%35 = OpLoad %int %j
%36 = OpIAdd %int %35 %int_1
OpStore %j %36
OpBranch %23
%24 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%37 = OpLoad %float %f
OpStore %fo %37
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, DebugValueForReferenceVariable) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     float& x = f;
  //     for (int i=0; i<4; i++) {
  //       x = x + BC[i];
  //     }
  //     fo = f;
  // }

  const std::string text = R"(
; CHECK: [[f_name:%\w+]] = OpString "f"
; CHECK: [[i_name:%\w+]] = OpString "i"
; CHECK: [[x_name:%\w+]] = OpString "x"
; CHECK: [[dbg_f:%\w+]] = OpExtInst %void [[ext:%\d+]] DebugLocalVariable [[f_name]]
; CHECK: [[dbg_i:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[i_name]]
; CHECK: [[dbg_x:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[x_name]]

; CHECK:      OpStore %f %float_0
; CHECK-DAG:  OpExtInst %void [[ext]] DebugValue [[dbg_f]] %float_0
; CHECK-DAG:  OpExtInst %void [[ext]] DebugValue [[dbg_x]] %float_0
; CHECK:      OpStore %i %int_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] %int_0

; CHECK-NOT:  DebugDeclare

; CHECK:      [[loop_head:%\w+]] = OpLabel
; CHECK:      [[phi0:%\w+]] = OpPhi %float %float_0
; CHECK:      [[phi1:%\w+]] = OpPhi %int %int_0
; CHECK-DAG:  OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[phi0]]
; CHECK-DAG:  OpExtInst %void [[ext]] DebugValue [[dbg_x]] [[phi0]]
; CHECK:      OpExtInst %void [[ext]] DebugValue [[dbg_i]] [[phi1]]
; CHECK:      OpLoopMerge [[loop_merge:%\w+]] [[loop_cont:%\w+]] None
; CHECK-NEXT: OpBranch [[loop_body:%\w+]]

; CHECK:      [[loop_body]] = OpLabel
; CHECK:      OpBranchConditional {{%\w+}} [[bb:%\w+]] [[loop_merge]]

; CHECK:      [[bb]] = OpLabel
; CHECK:      OpStore %f [[f_val:%\w+]]
; CHECK-DAG:  OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[f_val]]
; CHECK-DAG:  OpExtInst %void [[ext]] DebugValue [[dbg_x]] [[f_val]]
; CHECK:      OpBranch [[loop_cont]]

; CHECK:      [[loop_cont]] = OpLabel
; CHECK:      OpStore %i [[i_val:%\w+]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] [[i_val]]
; CHECK-NEXT: OpBranch [[loop_head]]

; CHECK:      [[loop_merge]] = OpLabel

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
OpSource GLSL 140
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
%i_name = OpString "i"
%x_name = OpString "x"
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_tf 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_v4f %src 0 0 %dbg_main FlagIsLocal
%dbg_i = OpExtInst %void %ext DebugLocalVariable %i_name %dbg_v4f %src 1 0 %dbg_main FlagIsLocal
%dbg_x = OpExtInst %void %ext DebugLocalVariable %x_name %dbg_v4f %src 2 0 %dbg_main FlagIsLocal
%main = OpFunction %void None %8
%22 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
%decl0 = OpExtInst %void %ext DebugDeclare %dbg_f %f %null_expr
%decl1 = OpExtInst %void %ext DebugDeclare %dbg_i %i %null_expr
%decl2 = OpExtInst %void %ext DebugDeclare %dbg_x %f %null_expr
OpBranch %23
%23 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%27 = OpLoad %int %i
%28 = OpSLessThan %bool %27 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%30 = OpLoad %float %f
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
%34 = OpFAdd %float %30 %33
OpStore %f %34
OpBranch %25
%25 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%35 = OpLoad %int %i
%36 = OpIAdd %int %35 %int_1
OpStore %i %36
OpBranch %23
%24 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%37 = OpLoad %float %f
OpStore %fo %37
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, DebugValueForReferenceVariableInBB) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     for (int i=0; i<4; i++) {
  //       float& x = f;
  //       x = x + BC[i];
  //       {
  //         x = x + BC[i];
  //       }
  //     }
  //     fo = f;
  // }

  const std::string text = R"(
; CHECK: [[f_name:%\w+]] = OpString "f"
; CHECK: [[i_name:%\w+]] = OpString "i"
; CHECK: [[x_name:%\w+]] = OpString "x"
; CHECK: [[dbg_main:%\w+]] = OpExtInst %void [[ext:%\d+]] DebugFunction
; CHECK: [[dbg_bb:%\w+]] = OpExtInst %void [[ext]] DebugLexicalBlock
; CHECK: [[dbg_bb_child:%\w+]] = OpExtInst %void [[ext]] DebugLexicalBlock
; CHECK: [[dbg_f:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[f_name]]
; CHECK: [[dbg_i:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[i_name]]
; CHECK: [[dbg_x:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[x_name]]

; CHECK:      OpExtInst %void [[ext]] DebugScope [[dbg_main]]
; CHECK:      OpStore %f %float_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_x]] %float_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] %float_0
; CHECK-NEXT: OpStore %i %int_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] %int_0

; CHECK-NOT:  DebugDeclare

; CHECK:      [[loop_head:%\w+]] = OpLabel
; CHECK:      OpExtInst %void [[ext]] DebugScope [[dbg_main]]
; CHECK:      [[phi0:%\w+]] = OpPhi %float %float_0
; CHECK:      [[phi1:%\w+]] = OpPhi %int %int_0
; CHECK-DAG: OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[phi0]]
; CHECK-DAG: OpExtInst %void [[ext]] DebugValue [[dbg_x]] [[phi0]]
; CHECK-DAG: OpExtInst %void [[ext]] DebugValue [[dbg_i]] [[phi1]]
; CHECK:      OpLoopMerge [[loop_merge:%\w+]] [[loop_cont:%\w+]] None
; CHECK-NEXT: OpBranch [[loop_body:%\w+]]

; CHECK-NEXT: [[loop_body]] = OpLabel
; CHECK:      OpBranchConditional {{%\w+}} [[bb:%\w+]] [[loop_merge]]

; CHECK:      [[bb]] = OpLabel
; CHECK:      OpExtInst %void [[ext]] DebugScope [[dbg_bb]]
; CHECK:      OpStore %f [[f_val:%\w+]]
; CHECK-DAG:  OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[f_val]]
; CHECK-DAG:  OpExtInst %void [[ext]] DebugValue [[dbg_x]] [[f_val]]
; CHECK:      OpBranch [[bb_child:%\w+]]

; CHECK:      [[bb_child]] = OpLabel
; CHECK:      OpExtInst %void [[ext]] DebugScope [[dbg_bb_child]]
; CHECK:      OpStore %f [[new_f_val:%\w+]]
; CHECK-DAG:  OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[new_f_val]]
; CHECK-DAG:  OpExtInst %void [[ext]] DebugValue [[dbg_x]] [[new_f_val]]
; CHECK:      OpBranch [[loop_cont]]

; CHECK:      [[loop_cont]] = OpLabel
; CHECK:      OpStore %i [[i_val:%\w+]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] [[i_val]]
; CHECK-NEXT: OpBranch [[loop_head]]

; CHECK:      [[loop_merge]] = OpLabel

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
OpSource GLSL 140
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
%i_name = OpString "i"
%x_name = OpString "x"
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_tf 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%bb = OpExtInst %void %ext DebugLexicalBlock %src 0 0 %dbg_main
%bb_child = OpExtInst %void %ext DebugLexicalBlock %src 1 0 %bb
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_v4f %src 0 0 %dbg_main FlagIsLocal
%dbg_i = OpExtInst %void %ext DebugLocalVariable %i_name %dbg_v4f %src 1 0 %dbg_main FlagIsLocal
%dbg_x = OpExtInst %void %ext DebugLocalVariable %x_name %dbg_v4f %src 2 0 %bb FlagIsLocal
%main = OpFunction %void None %8
%22 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
%decl0 = OpExtInst %void %ext DebugDeclare %dbg_f %f %null_expr
%decl1 = OpExtInst %void %ext DebugDeclare %dbg_i %i %null_expr
OpBranch %23
%23 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%27 = OpLoad %int %i
%28 = OpSLessThan %bool %27 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%scope = OpExtInst %void %ext DebugScope %bb
%decl2 = OpExtInst %void %ext DebugDeclare %dbg_x %f %null_expr
%30 = OpLoad %float %f
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
%34 = OpFAdd %float %30 %33
OpStore %f %34
OpBranch %38
%38 = OpLabel
%child_scope = OpExtInst %void %ext DebugScope %bb_child
%39 = OpLoad %float %f
%40 = OpFAdd %float %39 %33
OpStore %f %40
OpBranch %25
%25 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%35 = OpLoad %int %i
%36 = OpIAdd %int %35 %int_1
OpStore %i %36
OpBranch %23
%24 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%37 = OpLoad %float %f
OpStore %fo %37
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, DebugForLoopUseDebugValueInsteadOfDebugDeclare) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // struct S {
  //     float f;
  //     int i;
  // };
  //
  // void main()
  // {
  //     S foo = {0.0, 0};
  //     for (; foo.i<4; foo.i++) {
  //       foo.f = foo.f + BC[foo.i];
  //     }
  //     fo = foo.f;
  // }

  const std::string text = R"(
; CHECK: [[f_name:%\w+]] = OpString "f"
; CHECK: [[empty_expr:%\w+]] = OpExtInst %void [[ext:%\d+]] DebugExpression
; CHECK: [[deref_op:%\w+]] = OpExtInst %void [[ext]] DebugOperation Deref
; CHECK: [[deref:%\w+]] = OpExtInst %void [[ext]] DebugExpression [[deref_op]]
; CHECK: [[dbg_foo:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[f_name]]

; CHECK:      OpStore %f %float_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_foo]] %float_0 [[empty_expr]] %uint_0
; CHECK-NEXT: OpStore %i %int_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_foo]] %int_0 [[empty_expr]] %uint_1

; CHECK:      [[loop_head:%\w+]] = OpLabel
; CHECK:      [[phi0:%\w+]] = OpPhi %float %float_0
; CHECK:      [[phi1:%\w+]] = OpPhi %int %int_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_foo]] [[phi0]] [[empty_expr]] %uint_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_foo]] [[phi1]] [[empty_expr]] %uint_1
; CHECK:      OpLoopMerge [[loop_merge:%\w+]] [[loop_cont:%\w+]] None
; CHECK-NEXT: OpBranch [[loop_body:%\w+]]

; CHECK:      [[loop_body]] = OpLabel
; CHECK:      OpBranchConditional {{%\w+}} [[bb:%\w+]] [[loop_merge]]

; CHECK:      [[bb]] = OpLabel
; CHECK:      OpStore %f [[f_val:%\w+]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_foo]] [[f_val]] [[empty_expr]] %uint_0
; CHECK-NEXT: OpBranch [[loop_cont]]

; CHECK:      [[loop_cont]] = OpLabel
; CHECK:      OpStore %i [[i_val:%\w+]]
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_foo]] [[i_val]] [[empty_expr]] %uint_1
; CHECK-NEXT: OpBranch [[loop_head]]

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
OpSource GLSL 140
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
%deref_op = OpExtInst %void %ext DebugOperation Deref
%deref = OpExtInst %void %ext DebugExpression %deref_op
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_tf 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_foo = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_v4f %src 0 0 %dbg_main FlagIsLocal
%main = OpFunction %void None %8
%22 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
%decl0 = OpExtInst %void %ext DebugValue %dbg_foo %f %deref %uint_0
%decl1 = OpExtInst %void %ext DebugValue %dbg_foo %i %deref %uint_1
OpBranch %23
%23 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%27 = OpLoad %int %i
%28 = OpSLessThan %bool %27 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%30 = OpLoad %float %f
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
%34 = OpFAdd %float %30 %33
OpStore %f %34
OpBranch %25
%25 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%35 = OpLoad %int %i
%36 = OpIAdd %int %35 %int_1
OpStore %i %36
OpBranch %23
%24 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%37 = OpLoad %float %f
OpStore %fo %37
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, DebugValueNotUsedForDebugDeclare) {
  // #version 140
  //
  // in vec4 BC;
  // out float fo;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     for (int i=0; i<4; i++) {
  //       f = f + BC[i];
  //     }
  //     fo = f;
  // }

  const std::string text = R"(
; CHECK: [[f_name:%\w+]] = OpString "f"
; CHECK: [[i_name:%\w+]] = OpString "i"
; CHECK: [[dbg_f:%\w+]] = OpExtInst %void [[ext:%\d+]] DebugLocalVariable [[f_name]]
; CHECK: [[dbg_i:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable [[i_name]]

; CHECK:      OpStore %f %float_0
; CHECK-NEXT: OpStore %i %int_0
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_f]] %f
; CHECK-NEXT: OpExtInst %void [[ext]] DebugValue [[dbg_i]] %i

; CHECK-NOT:  DebugValue
; CHECK-NOT:  DebugDeclare

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
OpSource GLSL 140
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
%i_name = OpString "i"
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%dbg_v4f = OpExtInst %void %ext DebugTypeVector %dbg_tf 4
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_v4f %dbg_v4f
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_v4f %src 0 0 %dbg_main FlagIsLocal
%dbg_i = OpExtInst %void %ext DebugLocalVariable %i_name %dbg_v4f %src 1 0 %dbg_main FlagIsLocal
%main = OpFunction %void None %8
%22 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %f %float_0
OpStore %i %int_0
%decl0 = OpExtInst %void %ext DebugValue %dbg_f %f %null_expr
%decl1 = OpExtInst %void %ext DebugValue %dbg_i %i %null_expr
OpBranch %23
%23 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpLoopMerge %24 %25 None
OpBranch %26
%26 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%27 = OpLoad %int %i
%28 = OpSLessThan %bool %27 %int_4
OpBranchConditional %28 %29 %24
%29 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%30 = OpLoad %float %f
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
%34 = OpFAdd %float %30 %33
OpStore %f %34
OpBranch %25
%25 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%35 = OpLoad %int %i
%36 = OpIAdd %int %35 %int_1
OpStore %i %36
OpBranch %23
%24 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%37 = OpLoad %float %f
OpStore %fo %37
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, DebugNestedForLoop) {
  const std::string text = R"(
; CHECK: = OpFunction
; CHECK-NEXT: [[entry:%\w+]] = OpLabel
; CHECK: OpStore %f %float_0
; CHECK-NEXT: = OpExtInst %void [[ext:%\w+]] DebugValue [[dbg_f:%\w+]] %float_0

; CHECK: [[outer_header:%\w+]] = OpLabel
; CHECK: [[outer_f:%\w+]] = OpPhi %float %float_0 [[entry]] [[inner_f:%\w+]] [[outer_be:%\w+]]
; CHECK: = OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[outer_f]]

; CHECK: [[inner_pre_header:%\w+]] = OpLabel
; CHECK: [[inner_header:%\w+]] = OpLabel
; CHECK: [[inner_f]] = OpPhi %float [[outer_f]] [[inner_pre_header]] [[f_next:%\w+]] [[inner_be:%\w+]]
; CHECK: = OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[inner_f]]

; CHECK: [[inner_be]] = OpLabel
; CHECK: [[f_next]] = OpFAdd %float [[inner_f]]
; CHECK-NEXT: OpStore %f [[f_next]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[f_next]]

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
OpSource GLSL 450
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %j "j"
OpName %BC "BC"
OpName %fo "fo"
OpDecorate %BC Location 0
OpDecorate %fo Location 0
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Input_mat4v4float = OpTypePointer Input %mat4v4float
%BC = OpVariable %_ptr_Input_mat4v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output

; Debug information
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %void
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_tf %src 0 0 %dbg_main FlagIsLocal

%main = OpFunction %void None %9
%24 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%j = OpVariable %_ptr_Function_int Function

; DebugDeclare
OpStore %f %float_0
%decl = OpExtInst %void %ext DebugDeclare %dbg_f %f %null_expr

OpStore %i %int_0
OpBranch %25
%25 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%26 = OpLoad %int %i
%27 = OpSLessThan %bool %26 %int_4
OpLoopMerge %28 %29 None
OpBranchConditional %27 %30 %28
%30 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
OpStore %j %int_0
OpBranch %31
%31 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%32 = OpLoad %int %j
%33 = OpSLessThan %bool %32 %int_4
OpLoopMerge %50 %34 None
OpBranchConditional %33 %34 %50
%34 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%35 = OpLoad %float %f
%36 = OpLoad %int %i
%37 = OpLoad %int %j
%38 = OpAccessChain %_ptr_Input_float %BC %36 %37
%39 = OpLoad %float %38
%40 = OpFAdd %float %35 %39
OpStore %f %40
%41 = OpLoad %int %j
%42 = OpIAdd %int %41 %int_1
OpStore %j %42
OpBranch %31
%50 = OpLabel
%s6 = OpExtInst %void %ext DebugScope %dbg_main
OpBranch %29
%29 = OpLabel
%s7 = OpExtInst %void %ext DebugScope %dbg_main
%43 = OpLoad %int %i
%44 = OpIAdd %int %43 %int_1
OpStore %i %44
OpBranch %25
%28 = OpLabel
%s8 = OpExtInst %void %ext DebugScope %dbg_main
%45 = OpLoad %float %f
OpStore %fo %45
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, DebugForLoopWithContinue) {
  const std::string text = R"(
; CHECK: = OpFunction
; CHECK-NEXT: [[entry:%\w+]] = OpLabel
; CHECK: OpStore %f %float_0
; CHECK-NEXT: = OpExtInst %void [[ext:%\w+]] DebugValue [[dbg_f:%\w+]] %float_0

; CHECK: [[outer_header:%\w+]] = OpLabel
; CHECK: [[outer_f:%\w+]] = OpPhi %float %float_0 [[entry]] [[inner_f:%\w+]] [[cont:%\w+]]
; CHECK: = OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[outer_f]]

; CHECK: [[f_next:%\w+]] = OpFAdd %float [[outer_f]]
; CHECK-NEXT: OpStore %f [[f_next]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[f_next]]

; CHECK: [[cont]] = OpLabel
; CHECK: [[inner_f]] = OpPhi %float [[outer_f]] {{%\d+}} [[f_next]] {{%\d+}}
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[inner_f]]

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BC %fo
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
OpSource GLSL 140
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %t "t"
OpName %BC "BC"
OpName %fo "fo"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BC = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output

; Debug information
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %void
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_tf %src 0 0 %dbg_main FlagIsLocal

%main = OpFunction %void None %9
%23 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_float Function

; DebugDeclare
OpStore %f %float_0
%decl = OpExtInst %void %ext DebugDeclare %dbg_f %f %null_expr

OpStore %i %int_0
OpBranch %24
%24 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpLoopMerge %25 %26 None
OpBranch %27
%27 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%28 = OpLoad %int %i
%29 = OpSLessThan %bool %28 %int_4
OpBranchConditional %29 %30 %25
%30 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Input_float %BC %31
%33 = OpLoad %float %32
OpStore %t %33
%34 = OpLoad %float %t
%35 = OpFOrdLessThan %bool %34 %float_0
OpSelectionMerge %36 None
OpBranchConditional %35 %37 %36
%37 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
OpBranch %26
%36 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%38 = OpLoad %float %f
%39 = OpLoad %float %t
%40 = OpFAdd %float %38 %39
OpStore %f %40
OpBranch %26
%26 = OpLabel
%s6 = OpExtInst %void %ext DebugScope %dbg_main
%41 = OpLoad %int %i
%42 = OpIAdd %int %41 %int_1
OpStore %i %42
OpBranch %24
%25 = OpLabel
%s7 = OpExtInst %void %ext DebugScope %dbg_main
%43 = OpLoad %float %f
OpStore %fo %43
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, DebugIfElse) {
  const std::string text = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %f %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
%float_name = OpString "float"
%main_name = OpString "main"
%v_name = OpString "v"
OpSource GLSL 140
OpName %main "main"
OpName %f "f"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
%f = OpVariable %_ptr_Input_float Input
%float_0 = OpConstant %float 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%float_0_5 = OpConstant %float 0.5
%float_1 = OpConstant %float 1
%18 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output

; Debug information
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %void
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_v = OpExtInst %void %ext DebugLocalVariable %v_name %dbg_tf %src 0 0 %dbg_main FlagIsLocal

%main = OpFunction %void None %8
%20 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main

; DebugDeclare
%v = OpVariable %_ptr_Function_v4float Function
%decl = OpExtInst %void %ext DebugDeclare %dbg_v %v %null_expr

%21 = OpLoad %float %f
%22 = OpFOrdGreaterThanEqual %bool %21 %float_0
OpSelectionMerge %23 None
OpBranchConditional %22 %24 %25

; CHECK: OpBranchConditional
; CHECK-NEXT: [[br0:%\w+]] = OpLabel
; CHECK: OpStore %v [[v0:%\w+]]
; CHECK-NEXT: = OpExtInst %void [[ext:%\w+]] DebugValue [[dbg_v:%\w+]] [[v0]]
%24 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
%26 = OpLoad %v4float %BaseColor
%27 = OpVectorTimesScalar %v4float %26 %float_0_5
OpStore %v %27
OpBranch %23

; CHECK: [[br1:%\w+]] = OpLabel
; CHECK: OpStore %v [[v1:%\w+]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_v]] [[v1]]
%25 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%28 = OpLoad %v4float %BaseColor
%29 = OpFAdd %v4float %28 %18
OpStore %v %29
OpBranch %23

; CHECK: [[phi:%\w+]] = OpPhi %v4float [[v0]] [[br0]] [[v1]] [[br1]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_v]] [[phi]]
%23 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%30 = OpLoad %v4float %v
OpStore %gl_FragColor %30
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, DebugSwitch) {
  const std::string text = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %f %gl_FragColor
OpExecutionMode %main OriginUpperLeft
%file_name = OpString "test"
%float_name = OpString "float"
%main_name = OpString "main"
%v_name = OpString "v"
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %i "i"
OpName %f "f"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Input_float = OpTypePointer Input %float
%f = OpVariable %_ptr_Input_float Input
%float_0_25 = OpConstant %float 0.25
%float_0_75 = OpConstant %float 0.75
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output

; Debug information
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %void
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_v = OpExtInst %void %ext DebugLocalVariable %v_name %dbg_tf %src 0 0 %dbg_main FlagIsLocal

%main = OpFunction %void None %9
%20 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%v = OpVariable %_ptr_Function_v4float Function
%i = OpVariable %_ptr_Function_int Function
%21 = OpLoad %v4float %BaseColor

; DebugDeclare
OpStore %v %21
%decl = OpExtInst %void %ext DebugDeclare %dbg_v %v %null_expr

; CHECK: %main = OpFunction %void None
; CHECK-NEXT: [[entry:%\w+]] = OpLabel
; CHECK: OpStore %v [[v0:%\w+]]
; CHECK-NEXT: = OpExtInst %void [[ext:%\w+]] DebugValue [[dbg_v:%\w+]] [[v0]]
; CHECK: OpSwitch {{%\w+}} [[case0:%\w+]] 0 [[case1:%\w+]] 1 [[case2:%\w+]] 2 [[case3:%\w+]]
; CHECK: OpStore %v [[v1:%\w+]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_v]] [[v1]]
; CHECK: OpStore %v [[v2:%\w+]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_v]] [[v2]]
; CHECK: [[phi0:%\w+]] = OpPhi %v4float [[v0]] [[entry]] [[v2]] [[case2]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_v]] [[phi0]]
; CHECK: OpStore %v [[v3:%\w+]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_v]] [[v3]]
; CHECK: [[phi1:%\w+]] = OpPhi %v4float [[v0]] [[case0]] [[v1]] [[case1]] [[v3]] [[case3]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_v]] [[phi1]]

%22 = OpLoad %float %f
%23 = OpConvertFToS %int %22
OpStore %i %23
%24 = OpLoad %int %i
OpSelectionMerge %25 None
OpSwitch %24 %26 0 %27 1 %28 2 %29
%26 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpBranch %25
%27 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%30 = OpLoad %v4float %v
%31 = OpVectorTimesScalar %v4float %30 %float_0_25
OpStore %v %31
OpBranch %25
%28 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%32 = OpLoad %v4float %v
%33 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%34 = OpFAdd %v4float %32 %33
OpStore %v %34
OpBranch %29
%29 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%35 = OpLoad %v4float %v
%36 = OpVectorTimesScalar %v4float %35 %float_0_75
OpStore %v %36
OpBranch %25
%25 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%37 = OpLoad %v4float %v
OpStore %gl_FragColor %37
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, DebugSwapProblem) {
  // #version 140
  //
  // in float fe;
  // out float fo;
  //
  // void main()
  // {
  //     float f1 = 0.0;
  //     float f2 = 1.0;
  //     int ie = int(fe);
  //     for (int i=0; i<ie; i++) {
  //       float t = f1;
  //       f1 = f2;
  //       f2 = t;
  //     }
  //     fo = f1;
  // }
  //
  // Because of the swap in the for loop, it generates the following phi
  // instructions:
  //
  // [[phi_f2]] = OpPhi %float %float_1 [[entry]] [[phi_f1]] ..
  // [[phi_f1]] = OpPhi %float %float_0 [[entry]] [[phi_f2]] ..
  //
  // Since they are used as operands by each other, we want to clearly check
  // what DebugValue we have to add for them.

  const std::string text = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %fe %fo
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
%file_name = OpString "test"
%float_name = OpString "float"
%main_name = OpString "main"
%t_name = OpString "t"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %ie "ie"
OpName %fe "fe"
OpName %i "i"
OpName %t "t"
OpName %fo "fo"
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Input_float = OpTypePointer Input %float
%fe = OpVariable %_ptr_Input_float Input
%int_0 = OpConstant %int 0
%bool = OpTypeBool
%int_1 = OpConstant %int 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output

; Debug information
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %void
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
%dbg_f1 = OpExtInst %void %ext DebugLocalVariable %t_name %dbg_tf %src 0 0 %dbg_main FlagIsLocal
%dbg_f2 = OpExtInst %void %ext DebugLocalVariable %t_name %dbg_tf %src 0 0 %dbg_main FlagIsLocal
%dbg_i = OpExtInst %void %ext DebugLocalVariable %t_name %dbg_tf %src 0 0 %dbg_main FlagIsLocal

%main = OpFunction %void None %11
%23 = OpLabel
%s0 = OpExtInst %void %ext DebugScope %dbg_main
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%ie = OpVariable %_ptr_Function_int Function
%i = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_float Function
OpStore %f1 %float_0
OpStore %f2 %float_1
%24 = OpLoad %float %fe
%25 = OpConvertFToS %int %24
OpStore %ie %25
OpStore %i %int_0

; DebugDeclare
%decl0 = OpExtInst %void %ext DebugDeclare %dbg_f1 %f1 %null_expr
%decl1 = OpExtInst %void %ext DebugDeclare %dbg_f2 %f2 %null_expr
%decl2 = OpExtInst %void %ext DebugDeclare %dbg_i  %i  %null_expr

; CHECK: %main = OpFunction %void None
; CHECK-NEXT: [[entry:%\w+]] = OpLabel

; CHECK: OpStore %f1 %float_0
; CHECK-NEXT: = OpExtInst %void [[ext:%\w+]] DebugValue [[dbg_f1:%\w+]] %float_0
; CHECK: OpStore %f2 %float_1
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_f2:%\w+]] %float_1
; CHECK: OpStore %i %int_0
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_i:%\w+]] %int_0

; CHECK: [[phi_f2:%\w+]] = OpPhi %float %float_1 [[entry]] [[phi_f1:%\w+]]
; CHECK: [[phi_f1]] = OpPhi %float %float_0 [[entry]] [[phi_f2]]
; CHECK: [[phi_i:%\w+]] = OpPhi %int %int_0 [[entry]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_f2]] [[phi_f2]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_f1]] [[phi_f1]]
; CHECK-NEXT: = OpExtInst %void [[ext]] DebugValue [[dbg_i]] [[phi_i]]

OpBranch %26
%26 = OpLabel
%s1 = OpExtInst %void %ext DebugScope %dbg_main
OpLoopMerge %27 %28 None
OpBranch %29
%29 = OpLabel
%s2 = OpExtInst %void %ext DebugScope %dbg_main
%30 = OpLoad %int %i
%31 = OpLoad %int %ie
%32 = OpSLessThan %bool %30 %31
OpBranchConditional %32 %33 %27
%33 = OpLabel
%s3 = OpExtInst %void %ext DebugScope %dbg_main
%34 = OpLoad %float %f1
OpStore %t %34
%35 = OpLoad %float %f2
OpStore %f1 %35
%36 = OpLoad %float %t
OpStore %f2 %36
OpBranch %28
%28 = OpLabel
%s4 = OpExtInst %void %ext DebugScope %dbg_main
%37 = OpLoad %int %i
%38 = OpIAdd %int %37 %int_1
OpStore %i %38
OpBranch %26
%27 = OpLabel
%s5 = OpExtInst %void %ext DebugScope %dbg_main
%39 = OpLoad %float %f1
OpStore %fo %39
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, RemoveDebugDeclareWithoutLoads) {
  // Check that the DebugDeclare for c is removed even though its loads
  // had been removed previously by single block store/load optimization.
  // In the presence of DebugDeclare, single-block can and does remove loads,
  // but cannot change the stores into DebugValues and remove the DebugDeclare
  // because it is only a per block optimization, not a function optimization.
  // So SSA-rewrite must perform this role.
  //
  // Texture2D g_tColor;
  // SamplerState g_sAniso;
  //
  // struct PS_INPUT
  // {
  //   float2 vTextureCoords2 : TEXCOORD2;
  //   float2 vTextureCoords3 : TEXCOORD3;
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
  //   float4 c;
  //   c = g_tColor.Sample(g_sAniso, i.vTextureCoords2.xy);
  //   c += g_tColor.Sample(g_sAniso, i.vTextureCoords3.xy);
  //   ps_output.vColor = c;
  //   return ps_output;
  // }

  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %g_tColor %g_sAniso %in_var_TEXCOORD2 %in_var_TEXCOORD3 %out_var_SV_Target0
               OpExecutionMode %MainPs OriginUpperLeft
         %22 = OpString "foo.frag"
         %26 = OpString "PS_OUTPUT"
         %30 = OpString "float"
         %33 = OpString "vColor"
         %35 = OpString "PS_INPUT"
         %40 = OpString "vTextureCoords3"
         %42 = OpString "vTextureCoords2"
         %44 = OpString "@type.2d.image"
         %45 = OpString "type.2d.image"
         %47 = OpString "Texture2D.TemplateParam"
         %51 = OpString "src.MainPs"
         %55 = OpString "c"
         %57 = OpString "ps_output"
         %60 = OpString "i"
         %62 = OpString "@type.sampler"
         %63 = OpString "type.sampler"
         %65 = OpString "g_sAniso"
         %67 = OpString "g_tColor"
               OpName %type_2d_image "type.2d.image"
               OpName %g_tColor "g_tColor"
               OpName %type_sampler "type.sampler"
               OpName %g_sAniso "g_sAniso"
               OpName %in_var_TEXCOORD2 "in.var.TEXCOORD2"
               OpName %in_var_TEXCOORD3 "in.var.TEXCOORD3"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPs "MainPs"
               OpName %PS_INPUT "PS_INPUT"
               OpMemberName %PS_INPUT 0 "vTextureCoords2"
               OpMemberName %PS_INPUT 1 "vTextureCoords3"
               OpName %param_var_i "param.var.i"
               OpName %PS_OUTPUT "PS_OUTPUT"
               OpMemberName %PS_OUTPUT 0 "vColor"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_TEXCOORD2 Location 0
               OpDecorate %in_var_TEXCOORD3 Location 1
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %g_tColor DescriptorSet 0
               OpDecorate %g_tColor Binding 0
               OpDecorate %g_sAniso DescriptorSet 0
               OpDecorate %g_sAniso Binding 1
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
   %uint_128 = OpConstant %uint 128
     %uint_0 = OpConstant %uint 0
    %uint_64 = OpConstant %uint 64
         %69 = OpTypeFunction %void
   %PS_INPUT = OpTypeStruct %v2float %v2float
%_ptr_Function_PS_INPUT = OpTypePointer Function %PS_INPUT
  %PS_OUTPUT = OpTypeStruct %v4float
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%type_sampled_image = OpTypeSampledImage %type_2d_image
   %g_tColor = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
   %g_sAniso = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD2 = OpVariable %_ptr_Input_v2float Input
%in_var_TEXCOORD3 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %43 = OpExtInst %void %1 DebugInfoNone
         %59 = OpExtInst %void %1 DebugExpression
         %24 = OpExtInst %void %1 DebugSource %22
         %25 = OpExtInst %void %1 DebugCompilationUnit 1 4 %24 HLSL
         %28 = OpExtInst %void %1 DebugTypeComposite %26 Structure %24 11 1 %25 %26 %uint_128 FlagIsProtected|FlagIsPrivate %29
         %31 = OpExtInst %void %1 DebugTypeBasic %30 %uint_32 Float
         %32 = OpExtInst %void %1 DebugTypeVector %31 4
         %29 = OpExtInst %void %1 DebugTypeMember %33 %32 %24 13 5 %28 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %36 = OpExtInst %void %1 DebugTypeComposite %35 Structure %24 5 1 %25 %35 %uint_128 FlagIsProtected|FlagIsPrivate %37 %38
         %39 = OpExtInst %void %1 DebugTypeVector %31 2
         %38 = OpExtInst %void %1 DebugTypeMember %40 %39 %24 8 5 %36 %uint_64 %uint_64 FlagIsProtected|FlagIsPrivate
         %37 = OpExtInst %void %1 DebugTypeMember %42 %39 %24 7 5 %36 %uint_0 %uint_64 FlagIsProtected|FlagIsPrivate
         %46 = OpExtInst %void %1 DebugTypeComposite %44 Class %24 0 0 %25 %45 %43 FlagIsProtected|FlagIsPrivate
         %50 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %28 %36
         %52 = OpExtInst %void %1 DebugFunction %51 %50 %24 16 1 %25 %51 FlagIsProtected|FlagIsPrivate 17 %43
         %54 = OpExtInst %void %1 DebugLexicalBlock %24 17 1 %52
         %56 = OpExtInst %void %1 DebugLocalVariable %55 %32 %24 20 12 %54 FlagIsLocal
         %58 = OpExtInst %void %1 DebugLocalVariable %57 %28 %24 18 15 %54 FlagIsLocal
         %61 = OpExtInst %void %1 DebugLocalVariable %60 %36 %24 16 29 %52 FlagIsLocal 1
         %64 = OpExtInst %void %1 DebugTypeComposite %62 Structure %24 0 0 %25 %63 %43 FlagIsProtected|FlagIsPrivate
         %66 = OpExtInst %void %1 DebugGlobalVariable %65 %64 %24 3 14 %25 %65 %g_sAniso FlagIsDefinition
         %68 = OpExtInst %void %1 DebugGlobalVariable %67 %46 %24 1 11 %25 %67 %g_tColor FlagIsDefinition
     %MainPs = OpFunction %void None %69
         %70 = OpLabel
        %135 = OpExtInst %void %1 DebugScope %54
        %111 = OpVariable %_ptr_Function_PS_OUTPUT Function
        %112 = OpVariable %_ptr_Function_v4float Function
        %136 = OpExtInst %void %1 DebugNoScope
%param_var_i = OpVariable %_ptr_Function_PS_INPUT Function
         %74 = OpLoad %v2float %in_var_TEXCOORD2
         %75 = OpLoad %v2float %in_var_TEXCOORD3
         %76 = OpCompositeConstruct %PS_INPUT %74 %75
               OpStore %param_var_i %76
        %137 = OpExtInst %void %1 DebugScope %52
        %115 = OpExtInst %void %1 DebugDeclare %61 %param_var_i %59
        %138 = OpExtInst %void %1 DebugScope %54
        %116 = OpExtInst %void %1 DebugDeclare %58 %111 %59
        %117 = OpExtInst %void %1 DebugDeclare %56 %112 %59
;CHECK-NOT: %117 = OpExtInst %void %1 DebugDeclare %56 %112 %59
               OpLine %22 21 9
        %118 = OpLoad %type_2d_image %g_tColor
               OpLine %22 21 29
        %119 = OpLoad %type_sampler %g_sAniso
               OpLine %22 21 40
        %120 = OpAccessChain %_ptr_Function_v2float %param_var_i %int_0
        %121 = OpLoad %v2float %120
               OpLine %22 21 9
        %122 = OpSampledImage %type_sampled_image %118 %119
        %123 = OpImageSampleImplicitLod %v4float %122 %121 None
               OpLine %22 21 5
               OpStore %112 %123
;CHECK: %140 = OpExtInst %void %1 DebugValue %56 %123 %59
               OpLine %22 22 10
        %124 = OpLoad %type_2d_image %g_tColor
               OpLine %22 22 30
        %125 = OpLoad %type_sampler %g_sAniso
               OpLine %22 22 41
        %126 = OpAccessChain %_ptr_Function_v2float %param_var_i %int_1
        %127 = OpLoad %v2float %126
               OpLine %22 22 10
        %128 = OpSampledImage %type_sampled_image %124 %125
        %129 = OpImageSampleImplicitLod %v4float %128 %127 None
               OpLine %22 22 7
        %131 = OpFAdd %v4float %123 %129
               OpLine %22 22 5
               OpStore %112 %131
;CHECK: %141 = OpExtInst %void %1 DebugValue %56 %131 %59
               OpLine %22 23 5
        %133 = OpAccessChain %_ptr_Function_v4float %111 %int_0
               OpStore %133 %131
               OpLine %22 24 12
        %134 = OpLoad %PS_OUTPUT %111
        %139 = OpExtInst %void %1 DebugNoScope
         %79 = OpCompositeExtract %v4float %134 0
               OpStore %out_var_SV_Target0 %79
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

// Check support for pointer variables. When pointer variables are used, the
// computation of reaching definitions may need to follow pointer chains.
// See https://github.com/KhronosGroup/SPIRV-Tools/issues/3873 for details.
TEST_F(LocalSSAElimTest, PointerVariables) {
  const std::string text = R"(
               OpCapability Shader
               OpCapability VariablePointers
               OpExtension "SPV_KHR_variable_pointers"
               OpMemoryModel Logical Simple
               OpEntryPoint Fragment %1 "main" %2 %3
               OpExecutionMode %1 OriginUpperLeft
      %float = OpTypeFloat 32
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Function__ptr_Input_float = OpTypePointer Function %_ptr_Input_float
          %2 = OpVariable %_ptr_Input_float Input
          %3 = OpVariable %_ptr_Output_float Output
          %1 = OpFunction %void None %6
         %10 = OpLabel
         %11 = OpVariable %_ptr_Function__ptr_Input_float Function
               OpStore %11 %2

; CHECK-NOT: %12 = OpLoad %_ptr_Input_float %11
         %12 = OpLoad %_ptr_Input_float %11

; CHECK: %13 = OpLoad %float %2
         %13 = OpLoad %float %12

               OpStore %3 %13
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

TEST_F(LocalSSAElimTest, FunctionDeclaration) {
  // Make sure the pass works with a function declaration that is called.
  const std::string text = R"(OpCapability Addresses
OpCapability Linkage
OpCapability Kernel
OpCapability Int8
%1 = OpExtInstImport "OpenCL.std"
OpMemoryModel Physical64 OpenCL
OpEntryPoint Kernel %2 "_Z23julia__1166_kernel_77094Bool"
OpExecutionMode %2 ContractionOff
OpSource Unknown 0
OpDecorate %3 LinkageAttributes "julia_error_7712" Import
%void = OpTypeVoid
%5 = OpTypeFunction %void
%3 = OpFunction %void None %5
OpFunctionEnd
%2 = OpFunction %void None %5
%6 = OpLabel
%7 = OpFunctionCall %void %3
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<SSARewritePass>(text, text, false);
}

TEST_F(LocalSSAElimTest, MissingDebugValue) {
  // Make sure DebugValue for final fragcolor assignment is generated.

  const std::string text =
      R"(
               OpCapability Shader
               OpCapability ImageQuery
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "GLSL.std.450"
          %2 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_TEXCOORD0 %out_var_SV_TARGET %textureposition %samplerposition %textureNormal %samplerNormal %textureAlbedo %samplerAlbedo %textureShadowMap %samplerShadowMap %ubo
               OpExecutionMode %main OriginUpperLeft
         %15 = OpString "d2.frag"
         %55 = OpString "float"
         %63 = OpString "// Copyright 2020 Google LLC

Texture2D textureposition : register(t1);
SamplerState samplerposition : register(s1);
Texture2D textureNormal : register(t2);
SamplerState samplerNormal : register(s2);
Texture2D textureAlbedo : register(t3);
SamplerState samplerAlbedo : register(s3);
// Depth from the light's point of view
//layout (binding = 5) uniform sampler2DShadow samplerShadowMap;
Texture2DArray textureShadowMap : register(t5);
SamplerState samplerShadowMap : register(s5);

#define LIGHT_COUNT 3
#define SHADOW_FACTOR 0.25
#define AMBIENT_LIGHT 0.1
#define USE_PCF

struct Light
{
	float4 position;
	float4 target;
	float4 color;
	float4x4 viewMatrix;
};

struct UBO
{
	float4 viewPos;
	Light lights[LIGHT_COUNT];
	int useShadows;
	int displayDebugTarget;
};

cbuffer ubo : register(b4) { UBO ubo; }

float textureProj(float4 P, float layer, float2 offset)
{
	float shadow = 1.0;
	float4 shadowCoord = P / P.w;
	shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;

	if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
	{
		float dist = textureShadowMap.Sample(samplerShadowMap, float3(shadowCoord.xy + offset, layer)).r;
		if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
		{
			shadow = SHADOW_FACTOR;
		}
	}
	return shadow;
}

float filterPCF(float4 sc, float layer)
{
	int2 texDim; int elements; int levels;
	textureShadowMap.GetDimensions(0, texDim.x, texDim.y, elements, levels);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;

	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, layer, float2(dx*x, dy*y));
			count++;
		}

	}
	return shadowFactor / count;
}

float3 shadow(float3 fragcolor, float3 fragPos) {
	for (int i = 0; i < LIGHT_COUNT; ++i)
	{
		float4 shadowClip = mul(ubo.lights[i].viewMatrix, float4(fragPos.xyz, 1.0));

		float shadowFactor;
		#ifdef USE_PCF
			shadowFactor= filterPCF(shadowClip, i);
		#else
			shadowFactor = textureProj(shadowClip, i, float2(0.0, 0.0));
		#endif

		fragcolor *= shadowFactor;
	}
	return fragcolor;
}

float4 main([[vk::location(0)]] float2 inUV : TEXCOORD0) : SV_TARGET
{
	// Get G-Buffer values
	float3 fragPos = textureposition.Sample(samplerposition, inUV).rgb;
	float3 normal = textureNormal.Sample(samplerNormal, inUV).rgb;
	float4 albedo = textureAlbedo.Sample(samplerAlbedo, inUV);

	// Ambient part
	float3 fragcolor  = albedo.rgb * AMBIENT_LIGHT;

	float3 N = normalize(normal);

	for(int i = 0; i < LIGHT_COUNT; ++i)
	{
		// Vector to light
		float3 L = ubo.lights[i].position.xyz - fragPos;
		// Distance from light to fragment position
		float dist = length(L);
		L = normalize(L);

		// Viewer to fragment
		float3 V = ubo.viewPos.xyz - fragPos;
		V = normalize(V);

		float lightCosInnerAngle = cos(radians(15.0));
		float lightCosOuterAngle = cos(radians(25.0));
		float lightRange = 100.0;

		// Direction vector from source to target
		float3 dir = normalize(ubo.lights[i].position.xyz - ubo.lights[i].target.xyz);

		// Dual cone spot light with smooth transition between inner and outer angle
		float cosDir = dot(L, dir);
		float spotEffect = smoothstep(lightCosOuterAngle, lightCosInnerAngle, cosDir);
		float heightAttenuation = smoothstep(lightRange, 0.0f, dist);

		// Diffuse lighting
		float NdotL = max(0.0, dot(N, L));
		float3 diff = NdotL.xxx;

		// Specular lighting
		float3 R = reflect(-L, N);
		float NdotR = max(0.0, dot(R, V));
		float3 spec = (pow(NdotR, 16.0) * albedo.a * 2.5).xxx;

		fragcolor += float3((diff + spec) * spotEffect * heightAttenuation) * ubo.lights[i].color.rgb * albedo.rgb;
	}

	// Shadow calculations in a separate pass
	if (ubo.useShadows > 0)
	{
		fragcolor = shadow(fragcolor, fragPos);
	}

	return float4(fragcolor, 1);
}
"
         %68 = OpString "textureProj"
         %69 = OpString ""
         %78 = OpString "dist"
         %82 = OpString "shadowCoord"
         %85 = OpString "shadow"
         %89 = OpString "offset"
         %92 = OpString "layer"
         %95 = OpString "P"
         %99 = OpString "filterPCF"
        %108 = OpString "int"
        %110 = OpString "y"
        %114 = OpString "x"
        %118 = OpString "range"
        %122 = OpString "count"
        %125 = OpString "shadowFactor"
        %128 = OpString "dy"
        %131 = OpString "dx"
        %134 = OpString "scale"
        %137 = OpString "levels"
        %141 = OpString "elements"
        %145 = OpString "texDim"
        %150 = OpString "sc"
        %162 = OpString "shadowClip"
        %166 = OpString "i"
        %169 = OpString "fragPos"
        %171 = OpString "fragcolor"
        %175 = OpString "main"
        %184 = OpString "spec"
        %187 = OpString "NdotR"
        %190 = OpString "R"
        %193 = OpString "diff"
        %196 = OpString "NdotL"
        %199 = OpString "heightAttenuation"
        %202 = OpString "spotEffect"
        %205 = OpString "cosDir"
        %208 = OpString "dir"
        %211 = OpString "lightRange"
        %214 = OpString "lightCosOuterAngle"
        %217 = OpString "lightCosInnerAngle"
        %220 = OpString "V"
        %225 = OpString "L"
        %230 = OpString "N"
        %235 = OpString "albedo"
        %238 = OpString "normal"
        %244 = OpString "inUV"
        %246 = OpString "viewPos"
        %249 = OpString "position"
        %252 = OpString "target"
        %254 = OpString "color"
        %259 = OpString "viewMatrix"
        %263 = OpString "Light"
        %267 = OpString "lights"
        %271 = OpString "useShadows"
        %275 = OpString "displayDebugTarget"
        %278 = OpString "UBO"
        %282 = OpString "ubo"
        %285 = OpString "type.ubo"
        %289 = OpString "@type.sampler"
        %290 = OpString "type.sampler"
        %292 = OpString "samplerShadowMap"
        %295 = OpString "@type.2d.image.array"
        %296 = OpString "type.2d.image.array"
        %298 = OpString "TemplateParam"
        %301 = OpString "textureShadowMap"
        %304 = OpString "samplerAlbedo"
        %306 = OpString "@type.2d.image"
        %307 = OpString "type.2d.image"
        %311 = OpString "textureAlbedo"
        %313 = OpString "samplerNormal"
        %315 = OpString "textureNormal"
        %317 = OpString "samplerposition"
        %319 = OpString "textureposition"
               OpName %type_2d_image "type.2d.image"
               OpName %textureposition "textureposition"
               OpName %type_sampler "type.sampler"
               OpName %samplerposition "samplerposition"
               OpName %textureNormal "textureNormal"
               OpName %samplerNormal "samplerNormal"
               OpName %textureAlbedo "textureAlbedo"
               OpName %samplerAlbedo "samplerAlbedo"
               OpName %type_2d_image_array "type.2d.image.array"
               OpName %textureShadowMap "textureShadowMap"
               OpName %samplerShadowMap "samplerShadowMap"
               OpName %type_ubo "type.ubo"
               OpMemberName %type_ubo 0 "ubo"
               OpName %UBO "UBO"
               OpMemberName %UBO 0 "viewPos"
               OpMemberName %UBO 1 "lights"
               OpMemberName %UBO 2 "useShadows"
               OpMemberName %UBO 3 "displayDebugTarget"
               OpName %Light "Light"
               OpMemberName %Light 0 "position"
               OpMemberName %Light 1 "target"
               OpMemberName %Light 2 "color"
               OpMemberName %Light 3 "viewMatrix"
               OpName %ubo "ubo"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_TARGET "out.var.SV_TARGET"
               OpName %main "main"
               OpName %param_var_inUV "param.var.inUV"
               OpName %type_sampled_image "type.sampled.image"
               OpName %type_sampled_image_0 "type.sampled.image"
               OpDecorate %in_var_TEXCOORD0 Location 0
               OpDecorate %out_var_SV_TARGET Location 0
               OpDecorate %textureposition DescriptorSet 0
               OpDecorate %textureposition Binding 1
               OpDecorate %samplerposition DescriptorSet 0
               OpDecorate %samplerposition Binding 1
               OpDecorate %textureNormal DescriptorSet 0
               OpDecorate %textureNormal Binding 2
               OpDecorate %samplerNormal DescriptorSet 0
               OpDecorate %samplerNormal Binding 2
               OpDecorate %textureAlbedo DescriptorSet 0
               OpDecorate %textureAlbedo Binding 3
               OpDecorate %samplerAlbedo DescriptorSet 0
               OpDecorate %samplerAlbedo Binding 3
               OpDecorate %textureShadowMap DescriptorSet 0
               OpDecorate %textureShadowMap Binding 5
               OpDecorate %samplerShadowMap DescriptorSet 0
               OpDecorate %samplerShadowMap Binding 5
               OpDecorate %ubo DescriptorSet 0
               OpDecorate %ubo Binding 4
               OpMemberDecorate %Light 0 Offset 0
               OpMemberDecorate %Light 1 Offset 16
               OpMemberDecorate %Light 2 Offset 32
               OpMemberDecorate %Light 3 Offset 48
               OpMemberDecorate %Light 3 MatrixStride 16
               OpMemberDecorate %Light 3 RowMajor
               OpDecorate %_arr_Light_uint_3 ArrayStride 112
               OpMemberDecorate %UBO 0 Offset 0
               OpMemberDecorate %UBO 1 Offset 16
               OpMemberDecorate %UBO 2 Offset 352
               OpMemberDecorate %UBO 3 Offset 356
               OpMemberDecorate %type_ubo 0 Offset 0
               OpDecorate %type_ubo Block
)"
      R"(   %float = OpTypeFloat 32
%float_0_100000001 = OpConstant %float 0.100000001
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
      %int_1 = OpConstant %int 1
   %float_15 = OpConstant %float 15
   %float_25 = OpConstant %float 25
  %float_100 = OpConstant %float 100
    %float_0 = OpConstant %float 0
   %float_16 = OpConstant %float 16
  %float_2_5 = OpConstant %float 2.5
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
  %float_1_5 = OpConstant %float 1.5
  %float_0_5 = OpConstant %float 0.5
    %v2float = OpTypeVector %float 2
         %35 = OpConstantComposite %v2float %float_0_5 %float_0_5
   %float_n1 = OpConstant %float -1
 %float_0_25 = OpConstant %float 0.25
    %uint_32 = OpConstant %uint 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%type_2d_image_array = OpTypeImage %float 2D 2 1 0 1 Unknown
%_ptr_UniformConstant_type_2d_image_array = OpTypePointer UniformConstant %type_2d_image_array
    %v4float = OpTypeVector %float 4
     %uint_3 = OpConstant %uint 3
%mat4v4float = OpTypeMatrix %v4float 4
      %Light = OpTypeStruct %v4float %v4float %v4float %mat4v4float
%_arr_Light_uint_3 = OpTypeArray %Light %uint_3
        %UBO = OpTypeStruct %v4float %_arr_Light_uint_3 %int %int
   %type_ubo = OpTypeStruct %UBO
%_ptr_Uniform_type_ubo = OpTypePointer Uniform %type_ubo
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
     %uint_5 = OpConstant %uint 5
    %uint_37 = OpConstant %uint 37
    %uint_38 = OpConstant %uint 38
    %uint_44 = OpConstant %uint 44
    %uint_47 = OpConstant %uint 47
    %uint_45 = OpConstant %uint 45
     %uint_9 = OpConstant %uint 9
    %uint_40 = OpConstant %uint 40
    %uint_39 = OpConstant %uint 39
     %uint_8 = OpConstant %uint 8
    %uint_49 = OpConstant %uint 49
    %uint_35 = OpConstant %uint 35
    %uint_26 = OpConstant %uint 26
    %uint_54 = OpConstant %uint 54
    %uint_55 = OpConstant %uint 55
    %uint_67 = OpConstant %uint 67
    %uint_69 = OpConstant %uint 69
    %uint_68 = OpConstant %uint 68
    %uint_12 = OpConstant %uint 12
    %uint_66 = OpConstant %uint 66
    %uint_11 = OpConstant %uint 11
    %uint_64 = OpConstant %uint 64
     %uint_6 = OpConstant %uint 6
    %uint_63 = OpConstant %uint 63
    %uint_62 = OpConstant %uint 62
    %uint_60 = OpConstant %uint 60
    %uint_59 = OpConstant %uint 59
    %uint_58 = OpConstant %uint 58
    %uint_56 = OpConstant %uint 56
    %uint_33 = OpConstant %uint 33
    %uint_19 = OpConstant %uint 19
     %uint_7 = OpConstant %uint 7
    %uint_34 = OpConstant %uint 34
    %uint_24 = OpConstant %uint 24
    %uint_78 = OpConstant %uint 78
    %uint_80 = OpConstant %uint 80
    %uint_83 = OpConstant %uint 83
    %uint_81 = OpConstant %uint 81
    %uint_10 = OpConstant %uint 10
    %uint_79 = OpConstant %uint 79
    %uint_22 = OpConstant %uint 22
    %uint_95 = OpConstant %uint 95
    %uint_96 = OpConstant %uint 96
   %uint_145 = OpConstant %uint 145
   %uint_108 = OpConstant %uint 108
   %uint_138 = OpConstant %uint 138
   %uint_137 = OpConstant %uint 137
   %uint_136 = OpConstant %uint 136
   %uint_133 = OpConstant %uint 133
   %uint_132 = OpConstant %uint 132
   %uint_129 = OpConstant %uint 129
   %uint_128 = OpConstant %uint 128
   %uint_127 = OpConstant %uint 127
   %uint_124 = OpConstant %uint 124
   %uint_121 = OpConstant %uint 121
   %uint_120 = OpConstant %uint 120
   %uint_119 = OpConstant %uint 119
   %uint_116 = OpConstant %uint 116
   %uint_112 = OpConstant %uint 112
   %uint_110 = OpConstant %uint 110
   %uint_107 = OpConstant %uint 107
   %uint_105 = OpConstant %uint 105
   %uint_103 = OpConstant %uint 103
   %uint_100 = OpConstant %uint 100
    %uint_99 = OpConstant %uint 99
    %uint_98 = OpConstant %uint 98
    %uint_29 = OpConstant %uint 29
    %uint_21 = OpConstant %uint 21
   %uint_256 = OpConstant %uint 256
    %uint_23 = OpConstant %uint 23
   %uint_384 = OpConstant %uint 384
   %uint_512 = OpConstant %uint 512
   %uint_896 = OpConstant %uint 896
  %uint_2688 = OpConstant %uint 2688
    %uint_30 = OpConstant %uint 30
  %uint_2816 = OpConstant %uint 2816
    %uint_31 = OpConstant %uint 31
  %uint_2848 = OpConstant %uint 2848
  %uint_2880 = OpConstant %uint 2880
    %uint_27 = OpConstant %uint 27
  %uint_2944 = OpConstant %uint 2944
    %uint_14 = OpConstant %uint 14
    %uint_16 = OpConstant %uint 16
        %321 = OpTypeFunction %void
%_ptr_Function_v2float = OpTypePointer Function %v2float
   %uint_150 = OpConstant %uint 150
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
    %uint_42 = OpConstant %uint 42
%type_sampled_image = OpTypeSampledImage %type_2d_image
    %uint_65 = OpConstant %uint 65
    %uint_18 = OpConstant %uint 18
    %uint_13 = OpConstant %uint 13
    %uint_15 = OpConstant %uint 15
    %uint_17 = OpConstant %uint 17
       %bool = OpTypeBool
    %uint_25 = OpConstant %uint 25
%_ptr_Uniform_UBO = OpTypePointer Uniform %UBO
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %uint_28 = OpConstant %uint 28
    %uint_43 = OpConstant %uint 43
   %uint_113 = OpConstant %uint 113
   %uint_117 = OpConstant %uint 117
    %uint_46 = OpConstant %uint 46
    %uint_76 = OpConstant %uint 76
    %uint_53 = OpConstant %uint 53
    %uint_73 = OpConstant %uint 73
    %uint_48 = OpConstant %uint 48
   %uint_140 = OpConstant %uint 140
    %uint_52 = OpConstant %uint 52
    %uint_93 = OpConstant %uint 93
    %uint_87 = OpConstant %uint 87
   %uint_106 = OpConstant %uint 106
    %uint_36 = OpConstant %uint 36
   %uint_144 = OpConstant %uint 144
%_ptr_Uniform_int = OpTypePointer Uniform %int
   %uint_146 = OpConstant %uint 146
   %uint_147 = OpConstant %uint 147
   %uint_149 = OpConstant %uint 149
    %uint_41 = OpConstant %uint 41
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
    %uint_77 = OpConstant %uint 77
    %uint_85 = OpConstant %uint 85
    %uint_90 = OpConstant %uint 90
    %uint_92 = OpConstant %uint 92
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
    %uint_57 = OpConstant %uint 57
     %v3uint = OpTypeVector %uint 3
    %uint_72 = OpConstant %uint 72
    %uint_70 = OpConstant %uint 70
    %uint_50 = OpConstant %uint 50
    %uint_61 = OpConstant %uint 61
    %uint_71 = OpConstant %uint 71
    %uint_75 = OpConstant %uint 75
    %uint_82 = OpConstant %uint 82
%type_sampled_image_0 = OpTypeSampledImage %type_2d_image_array
    %uint_51 = OpConstant %uint 51
%textureposition = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%samplerposition = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%textureNormal = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%samplerNormal = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%textureAlbedo = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%samplerAlbedo = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%textureShadowMap = OpVariable %_ptr_UniformConstant_type_2d_image_array UniformConstant
%samplerShadowMap = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
        %ubo = OpVariable %_ptr_Uniform_type_ubo Uniform
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
  %uint_1792 = OpConstant %uint 1792
  %uint_1869 = OpConstant %uint 1869
  %uint_2060 = OpConstant %uint 2060
        %288 = OpExtInst %void %2 DebugInfoNone
        %243 = OpExtInst %void %2 DebugExpression
         %57 = OpExtInst %void %2 DebugTypeBasic %55 %uint_32 %uint_3 %uint_0
         %58 = OpExtInst %void %2 DebugTypeVector %57 %uint_4
         %60 = OpExtInst %void %2 DebugTypeVector %57 %uint_2
         %62 = OpExtInst %void %2 DebugTypeFunction %uint_3 %57 %58 %57 %60
         %64 = OpExtInst %void %2 DebugSource %15 %63
         %65 = OpExtInst %void %2 DebugCompilationUnit %uint_1 %uint_4 %64 %uint_5
         %70 = OpExtInst %void %2 DebugFunction %68 %62 %64 %uint_37 %uint_1 %65 %69 %uint_3 %uint_38
         %73 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_38 %uint_1 %70
         %74 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_44 %uint_2 %73
         %76 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_47 %uint_3 %74
         %79 = OpExtInst %void %2 DebugLocalVariable %78 %57 %64 %uint_45 %uint_9 %74 %uint_4
         %83 = OpExtInst %void %2 DebugLocalVariable %82 %58 %64 %uint_40 %uint_9 %73 %uint_4
         %86 = OpExtInst %void %2 DebugLocalVariable %85 %57 %64 %uint_39 %uint_8 %73 %uint_4
         %90 = OpExtInst %void %2 DebugLocalVariable %89 %60 %64 %uint_37 %uint_49 %70 %uint_4 %uint_3
         %93 = OpExtInst %void %2 DebugLocalVariable %92 %57 %64 %uint_37 %uint_35 %70 %uint_4 %uint_2
         %96 = OpExtInst %void %2 DebugLocalVariable %95 %58 %64 %uint_37 %uint_26 %70 %uint_4 %uint_1
         %98 = OpExtInst %void %2 DebugTypeFunction %uint_3 %57 %58 %57
        %100 = OpExtInst %void %2 DebugFunction %99 %98 %64 %uint_54 %uint_1 %65 %69 %uint_3 %uint_55
        %103 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_55 %uint_1 %100
        %104 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_67 %uint_2 %103
        %106 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_69 %uint_3 %104
        %109 = OpExtInst %void %2 DebugTypeBasic %108 %uint_32 %uint_4 %uint_0
        %111 = OpExtInst %void %2 DebugLocalVariable %110 %109 %64 %uint_68 %uint_12 %104 %uint_4
        %115 = OpExtInst %void %2 DebugLocalVariable %114 %109 %64 %uint_66 %uint_11 %103 %uint_4
        %119 = OpExtInst %void %2 DebugLocalVariable %118 %109 %64 %uint_64 %uint_6 %103 %uint_4
        %123 = OpExtInst %void %2 DebugLocalVariable %122 %109 %64 %uint_63 %uint_6 %103 %uint_4
        %126 = OpExtInst %void %2 DebugLocalVariable %125 %57 %64 %uint_62 %uint_8 %103 %uint_4
        %129 = OpExtInst %void %2 DebugLocalVariable %128 %57 %64 %uint_60 %uint_8 %103 %uint_4
        %132 = OpExtInst %void %2 DebugLocalVariable %131 %57 %64 %uint_59 %uint_8 %103 %uint_4
        %135 = OpExtInst %void %2 DebugLocalVariable %134 %57 %64 %uint_58 %uint_8 %103 %uint_4
        %138 = OpExtInst %void %2 DebugLocalVariable %137 %109 %64 %uint_56 %uint_33 %103 %uint_4
        %142 = OpExtInst %void %2 DebugLocalVariable %141 %109 %64 %uint_56 %uint_19 %103 %uint_4
        %144 = OpExtInst %void %2 DebugTypeVector %109 %uint_2
        %146 = OpExtInst %void %2 DebugLocalVariable %145 %144 %64 %uint_56 %uint_7 %103 %uint_4
        %148 = OpExtInst %void %2 DebugLocalVariable %92 %57 %64 %uint_54 %uint_34 %100 %uint_4 %uint_2
        %151 = OpExtInst %void %2 DebugLocalVariable %150 %58 %64 %uint_54 %uint_24 %100 %uint_4 %uint_1
        %153 = OpExtInst %void %2 DebugTypeVector %57 %uint_3
        %154 = OpExtInst %void %2 DebugTypeFunction %uint_3 %153 %153 %153
        %155 = OpExtInst %void %2 DebugFunction %85 %154 %64 %uint_78 %uint_1 %65 %69 %uint_3 %uint_78
        %157 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_78 %uint_49 %155
        %158 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_80 %uint_2 %157
        %160 = OpExtInst %void %2 DebugLocalVariable %125 %57 %64 %uint_83 %uint_9 %158 %uint_4
        %163 = OpExtInst %void %2 DebugLocalVariable %162 %58 %64 %uint_81 %uint_10 %158 %uint_4
        %167 = OpExtInst %void %2 DebugLocalVariable %166 %109 %64 %uint_79 %uint_11 %157 %uint_4
        %170 = OpExtInst %void %2 DebugLocalVariable %169 %153 %64 %uint_78 %uint_40 %155 %uint_4 %uint_2
        %172 = OpExtInst %void %2 DebugLocalVariable %171 %153 %64 %uint_78 %uint_22 %155 %uint_4 %uint_1
        %174 = OpExtInst %void %2 DebugTypeFunction %uint_3 %58 %60
        %176 = OpExtInst %void %2 DebugFunction %175 %174 %64 %uint_95 %uint_1 %65 %69 %uint_3 %uint_96
        %179 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_96 %uint_1 %176
        %180 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_145 %uint_2 %179
        %182 = OpExtInst %void %2 DebugLexicalBlock %64 %uint_108 %uint_2 %179
        %185 = OpExtInst %void %2 DebugLocalVariable %184 %153 %64 %uint_138 %uint_10 %182 %uint_4
        %188 = OpExtInst %void %2 DebugLocalVariable %187 %57 %64 %uint_137 %uint_9 %182 %uint_4
        %191 = OpExtInst %void %2 DebugLocalVariable %190 %153 %64 %uint_136 %uint_10 %182 %uint_4
        %194 = OpExtInst %void %2 DebugLocalVariable %193 %153 %64 %uint_133 %uint_10 %182 %uint_4
        %197 = OpExtInst %void %2 DebugLocalVariable %196 %57 %64 %uint_132 %uint_9 %182 %uint_4
        %200 = OpExtInst %void %2 DebugLocalVariable %199 %57 %64 %uint_129 %uint_9 %182 %uint_4
        %203 = OpExtInst %void %2 DebugLocalVariable %202 %57 %64 %uint_128 %uint_9 %182 %uint_4
        %206 = OpExtInst %void %2 DebugLocalVariable %205 %57 %64 %uint_127 %uint_9 %182 %uint_4
        %209 = OpExtInst %void %2 DebugLocalVariable %208 %153 %64 %uint_124 %uint_10 %182 %uint_4
        %212 = OpExtInst %void %2 DebugLocalVariable %211 %57 %64 %uint_121 %uint_9 %182 %uint_4
        %215 = OpExtInst %void %2 DebugLocalVariable %214 %57 %64 %uint_120 %uint_9 %182 %uint_4
        %218 = OpExtInst %void %2 DebugLocalVariable %217 %57 %64 %uint_119 %uint_9 %182 %uint_4
        %221 = OpExtInst %void %2 DebugLocalVariable %220 %153 %64 %uint_116 %uint_10 %182 %uint_4
        %223 = OpExtInst %void %2 DebugLocalVariable %78 %57 %64 %uint_112 %uint_9 %182 %uint_4
        %226 = OpExtInst %void %2 DebugLocalVariable %225 %153 %64 %uint_110 %uint_10 %182 %uint_4
        %228 = OpExtInst %void %2 DebugLocalVariable %166 %109 %64 %uint_107 %uint_10 %179 %uint_4
        %231 = OpExtInst %void %2 DebugLocalVariable %230 %153 %64 %uint_105 %uint_9 %179 %uint_4
        %233 = OpExtInst %void %2 DebugLocalVariable %171 %153 %64 %uint_103 %uint_9 %179 %uint_4
        %236 = OpExtInst %void %2 DebugLocalVariable %235 %58 %64 %uint_100 %uint_9 %179 %uint_4
        %239 = OpExtInst %void %2 DebugLocalVariable %238 %153 %64 %uint_99 %uint_9 %179 %uint_4
        %241 = OpExtInst %void %2 DebugLocalVariable %169 %153 %64 %uint_98 %uint_9 %179 %uint_4
        %245 = OpExtInst %void %2 DebugLocalVariable %244 %60 %64 %uint_95 %uint_40 %176 %uint_4 %uint_1
)"
      R"(     %247 = OpExtInst %void %2 DebugTypeMember %246 %58 %64 %uint_29 %uint_9 %uint_0 %uint_128 %uint_3
        %250 = OpExtInst %void %2 DebugTypeMember %249 %58 %64 %uint_21 %uint_9 %uint_0 %uint_128 %uint_3
        %253 = OpExtInst %void %2 DebugTypeMember %252 %58 %64 %uint_22 %uint_9 %uint_128 %uint_128 %uint_3
        %256 = OpExtInst %void %2 DebugTypeMember %254 %58 %64 %uint_23 %uint_9 %uint_256 %uint_128 %uint_3
        %258 = OpExtInst %void %2 DebugTypeArray %57 %uint_4 %uint_4
        %262 = OpExtInst %void %2 DebugTypeMember %259 %258 %64 %uint_24 %uint_11 %uint_384 %uint_512 %uint_3
        %265 = OpExtInst %void %2 DebugTypeComposite %263 %uint_1 %64 %uint_19 %uint_8 %65 %263 %uint_896 %uint_3 %250 %253 %256 %262
        %266 = OpExtInst %void %2 DebugTypeArray %265 %uint_3
        %269 = OpExtInst %void %2 DebugTypeMember %267 %266 %64 %uint_30 %uint_8 %uint_128 %uint_2688 %uint_3
        %273 = OpExtInst %void %2 DebugTypeMember %271 %109 %64 %uint_31 %uint_6 %uint_2816 %uint_32 %uint_3
        %277 = OpExtInst %void %2 DebugTypeMember %275 %109 %64 %uint_32 %uint_6 %uint_2848 %uint_32 %uint_3
        %280 = OpExtInst %void %2 DebugTypeComposite %278 %uint_1 %64 %uint_27 %uint_8 %65 %278 %uint_2880 %uint_3 %247 %269 %273 %277
        %284 = OpExtInst %void %2 DebugTypeMember %282 %280 %64 %uint_35 %uint_34 %uint_0 %uint_2944 %uint_3
        %286 = OpExtInst %void %2 DebugTypeComposite %285 %uint_1 %64 %uint_35 %uint_9 %65 %285 %uint_2944 %uint_3 %284
        %287 = OpExtInst %void %2 DebugGlobalVariable %282 %286 %64 %uint_35 %uint_9 %65 %282 %ubo %uint_8
        %291 = OpExtInst %void %2 DebugTypeComposite %289 %uint_1 %64 %uint_0 %uint_0 %65 %290 %288 %uint_3
        %293 = OpExtInst %void %2 DebugGlobalVariable %292 %291 %64 %uint_12 %uint_14 %65 %292 %samplerShadowMap %uint_8
        %297 = OpExtInst %void %2 DebugTypeComposite %295 %uint_0 %64 %uint_0 %uint_0 %65 %296 %288 %uint_3
        %299 = OpExtInst %void %2 DebugTypeTemplateParameter %298 %58 %288 %64 %uint_0 %uint_0
        %300 = OpExtInst %void %2 DebugTypeTemplate %297 %299
        %302 = OpExtInst %void %2 DebugGlobalVariable %301 %300 %64 %uint_11 %uint_16 %65 %301 %textureShadowMap %uint_8
        %305 = OpExtInst %void %2 DebugGlobalVariable %304 %291 %64 %uint_8 %uint_14 %65 %304 %samplerAlbedo %uint_8
        %308 = OpExtInst %void %2 DebugTypeComposite %306 %uint_0 %64 %uint_0 %uint_0 %65 %307 %288 %uint_3
        %309 = OpExtInst %void %2 DebugTypeTemplateParameter %298 %58 %288 %64 %uint_0 %uint_0
        %310 = OpExtInst %void %2 DebugTypeTemplate %308 %309
        %312 = OpExtInst %void %2 DebugGlobalVariable %311 %310 %64 %uint_7 %uint_11 %65 %311 %textureAlbedo %uint_8
        %314 = OpExtInst %void %2 DebugGlobalVariable %313 %291 %64 %uint_6 %uint_14 %65 %313 %samplerNormal %uint_8
        %316 = OpExtInst %void %2 DebugGlobalVariable %315 %310 %64 %uint_5 %uint_11 %65 %315 %textureNormal %uint_8
        %318 = OpExtInst %void %2 DebugGlobalVariable %317 %291 %64 %uint_4 %uint_14 %65 %317 %samplerposition %uint_8
        %320 = OpExtInst %void %2 DebugGlobalVariable %319 %310 %64 %uint_3 %uint_11 %65 %319 %textureposition %uint_8
       %1803 = OpExtInst %void %2 DebugInlinedAt %uint_1792 %180
       %1885 = OpExtInst %void %2 DebugInlinedAt %uint_1869 %158 %1803
       %2085 = OpExtInst %void %2 DebugInlinedAt %uint_2060 %106 %1885
)"
      R"(    %main = OpFunction %void None %321
        %322 = OpLabel
       %2083 = OpVariable %_ptr_Function_float Function
       %2086 = OpVariable %_ptr_Function_v4float Function
       %1883 = OpVariable %_ptr_Function_v2int Function
       %1891 = OpVariable %_ptr_Function_float Function
       %1892 = OpVariable %_ptr_Function_int Function
       %1894 = OpVariable %_ptr_Function_int Function
       %1895 = OpVariable %_ptr_Function_int Function
       %1896 = OpVariable %_ptr_Function_v4float Function
       %1801 = OpVariable %_ptr_Function_int Function
       %1447 = OpVariable %_ptr_Function_v4float Function
       %1448 = OpVariable %_ptr_Function_v3float Function
       %1450 = OpVariable %_ptr_Function_int Function
       %1451 = OpVariable %_ptr_Function_v3float Function
       %1453 = OpVariable %_ptr_Function_v3float Function
       %1466 = OpVariable %_ptr_Function_v3float Function
%param_var_inUV = OpVariable %_ptr_Function_v2float Function
        %325 = OpExtInst %void %2 DebugFunctionDefinition %176 %main
        %326 = OpLoad %v2float %in_var_TEXCOORD0
               OpStore %param_var_inUV %326
       %2290 = OpExtInst %void %2 DebugScope %176
       %1620 = OpExtInst %void %2 DebugLine %64 %uint_95 %uint_95 %uint_33 %uint_40
       %1470 = OpExtInst %void %2 DebugDeclare %245 %param_var_inUV %243
       %2291 = OpExtInst %void %2 DebugScope %179
       %1621 = OpExtInst %void %2 DebugLine %64 %uint_98 %uint_98 %uint_19 %uint_19
       %1471 = OpLoad %type_2d_image %textureposition
       %1622 = OpExtInst %void %2 DebugLine %64 %uint_98 %uint_98 %uint_42 %uint_42
       %1472 = OpLoad %type_sampler %samplerposition
       %1624 = OpExtInst %void %2 DebugLine %64 %uint_98 %uint_98 %uint_19 %uint_63
       %1474 = OpSampledImage %type_sampled_image %1471 %1472
       %1475 = OpImageSampleImplicitLod %v4float %1474 %326 None
       %1626 = OpExtInst %void %2 DebugLine %64 %uint_98 %uint_98 %uint_19 %uint_65
       %1476 = OpVectorShuffle %v3float %1475 %1475 0 1 2
       %2241 = OpExtInst %void %2 DebugLine %64 %uint_98 %uint_98 %uint_2 %uint_65
       %2240 = OpExtInst %void %2 DebugValue %241 %1476 %243
       %1629 = OpExtInst %void %2 DebugLine %64 %uint_99 %uint_99 %uint_18 %uint_18
       %1478 = OpLoad %type_2d_image %textureNormal
       %1630 = OpExtInst %void %2 DebugLine %64 %uint_99 %uint_99 %uint_39 %uint_39
       %1479 = OpLoad %type_sampler %samplerNormal
       %1632 = OpExtInst %void %2 DebugLine %64 %uint_99 %uint_99 %uint_18 %uint_58
       %1481 = OpSampledImage %type_sampled_image %1478 %1479
       %1482 = OpImageSampleImplicitLod %v4float %1481 %326 None
       %1634 = OpExtInst %void %2 DebugLine %64 %uint_99 %uint_99 %uint_18 %uint_60
       %1483 = OpVectorShuffle %v3float %1482 %1482 0 1 2
       %2244 = OpExtInst %void %2 DebugLine %64 %uint_99 %uint_99 %uint_2 %uint_60
       %2243 = OpExtInst %void %2 DebugValue %239 %1483 %243
       %1637 = OpExtInst %void %2 DebugLine %64 %uint_100 %uint_100 %uint_18 %uint_18
       %1485 = OpLoad %type_2d_image %textureAlbedo
       %1638 = OpExtInst %void %2 DebugLine %64 %uint_100 %uint_100 %uint_39 %uint_39
       %1486 = OpLoad %type_sampler %samplerAlbedo
       %1640 = OpExtInst %void %2 DebugLine %64 %uint_100 %uint_100 %uint_18 %uint_58
       %1488 = OpSampledImage %type_sampled_image %1485 %1486
       %1489 = OpImageSampleImplicitLod %v4float %1488 %326 None
       %1642 = OpExtInst %void %2 DebugLine %64 %uint_100 %uint_100 %uint_2 %uint_58
               OpStore %1447 %1489
       %1490 = OpExtInst %void %2 DebugDeclare %236 %1447 %243
       %1645 = OpExtInst %void %2 DebugLine %64 %uint_103 %uint_103 %uint_22 %uint_29
       %1492 = OpVectorShuffle %v3float %1489 %1489 0 1 2
       %1646 = OpExtInst %void %2 DebugLine %64 %uint_103 %uint_103 %uint_22 %uint_35
       %1493 = OpVectorTimesScalar %v3float %1492 %float_0_100000001
       %1647 = OpExtInst %void %2 DebugLine %64 %uint_103 %uint_103 %uint_2 %uint_35
               OpStore %1448 %1493
       %1494 = OpExtInst %void %2 DebugDeclare %233 %1448 %243
       %1650 = OpExtInst %void %2 DebugLine %64 %uint_105 %uint_105 %uint_13 %uint_29
       %1496 = OpExtInst %v3float %1 Normalize %1483
       %2247 = OpExtInst %void %2 DebugLine %64 %uint_105 %uint_105 %uint_2 %uint_29
       %2246 = OpExtInst %void %2 DebugValue %231 %1496 %243
       %1653 = OpExtInst %void %2 DebugLine %64 %uint_107 %uint_107 %uint_6 %uint_14
               OpStore %1450 %int_0
       %1498 = OpExtInst %void %2 DebugDeclare %228 %1450 %243
       %1655 = OpExtInst %void %2 DebugLine %64 %uint_107 %uint_107 %uint_6 %uint_15
               OpBranch %1499
       %1499 = OpLabel
       %2292 = OpExtInst %void %2 DebugScope %179
       %1656 = OpExtInst %void %2 DebugLine %64 %uint_107 %uint_107 %uint_17 %uint_17
       %1500 = OpLoad %int %1450
       %1657 = OpExtInst %void %2 DebugLine %64 %uint_107 %uint_107 %uint_17 %uint_21
       %1501 = OpSLessThan %bool %1500 %int_3
       %2293 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %1605 %1602 None
               OpBranchConditional %1501 %1502 %1605
       %1502 = OpLabel
       %2294 = OpExtInst %void %2 DebugScope %182
       %1660 = OpExtInst %void %2 DebugLine %64 %uint_110 %uint_110 %uint_25 %uint_25
       %1503 = OpLoad %int %1450
       %1661 = OpExtInst %void %2 DebugLine %64 %uint_110 %uint_110 %uint_14 %uint_37
       %1504 = OpAccessChain %_ptr_Uniform_UBO %ubo %int_0
       %1505 = OpAccessChain %_ptr_Uniform_v4float %1504 %int_1 %1503 %int_0
       %1663 = OpExtInst %void %2 DebugLine %64 %uint_110 %uint_110 %uint_14 %uint_28
       %1506 = OpLoad %v4float %1505
       %1664 = OpExtInst %void %2 DebugLine %64 %uint_110 %uint_110 %uint_14 %uint_37
       %1507 = OpVectorShuffle %v3float %1506 %1506 0 1 2
       %1666 = OpExtInst %void %2 DebugLine %64 %uint_110 %uint_110 %uint_14 %uint_43
       %1509 = OpFSub %v3float %1507 %1476
       %1667 = OpExtInst %void %2 DebugLine %64 %uint_110 %uint_110 %uint_3 %uint_43
               OpStore %1451 %1509
       %1510 = OpExtInst %void %2 DebugDeclare %226 %1451 %243
       %1670 = OpExtInst %void %2 DebugLine %64 %uint_112 %uint_112 %uint_16 %uint_24
       %1512 = OpExtInst %float %1 Length %1509
       %2250 = OpExtInst %void %2 DebugLine %64 %uint_112 %uint_112 %uint_3 %uint_24
       %2249 = OpExtInst %void %2 DebugValue %223 %1512 %243
       %1674 = OpExtInst %void %2 DebugLine %64 %uint_113 %uint_113 %uint_7 %uint_18
       %1515 = OpExtInst %v3float %1 Normalize %1509
       %1675 = OpExtInst %void %2 DebugLine %64 %uint_113 %uint_113 %uint_3 %uint_18
               OpStore %1451 %1515
       %1676 = OpExtInst %void %2 DebugLine %64 %uint_116 %uint_116 %uint_14 %uint_26
       %1516 = OpAccessChain %_ptr_Uniform_UBO %ubo %int_0
       %1517 = OpAccessChain %_ptr_Uniform_v4float %1516 %int_0
       %1678 = OpExtInst %void %2 DebugLine %64 %uint_116 %uint_116 %uint_14 %uint_18
       %1518 = OpLoad %v4float %1517
       %1679 = OpExtInst %void %2 DebugLine %64 %uint_116 %uint_116 %uint_14 %uint_26
       %1519 = OpVectorShuffle %v3float %1518 %1518 0 1 2
       %1681 = OpExtInst %void %2 DebugLine %64 %uint_116 %uint_116 %uint_14 %uint_32
       %1521 = OpFSub %v3float %1519 %1476
       %1682 = OpExtInst %void %2 DebugLine %64 %uint_116 %uint_116 %uint_3 %uint_32
               OpStore %1453 %1521
       %1522 = OpExtInst %void %2 DebugDeclare %221 %1453 %243
       %1685 = OpExtInst %void %2 DebugLine %64 %uint_117 %uint_117 %uint_7 %uint_18
       %1524 = OpExtInst %v3float %1 Normalize %1521
       %1686 = OpExtInst %void %2 DebugLine %64 %uint_117 %uint_117 %uint_3 %uint_18
               OpStore %1453 %1524
       %1687 = OpExtInst %void %2 DebugLine %64 %uint_119 %uint_119 %uint_34 %uint_46
       %1525 = OpExtInst %float %1 Radians %float_15
       %1688 = OpExtInst %void %2 DebugLine %64 %uint_119 %uint_119 %uint_30 %uint_47
       %1526 = OpExtInst %float %1 Cos %1525
       %2253 = OpExtInst %void %2 DebugLine %64 %uint_119 %uint_119 %uint_3 %uint_47
       %2252 = OpExtInst %void %2 DebugValue %218 %1526 %243
       %1691 = OpExtInst %void %2 DebugLine %64 %uint_120 %uint_120 %uint_34 %uint_46
       %1528 = OpExtInst %float %1 Radians %float_25
       %1692 = OpExtInst %void %2 DebugLine %64 %uint_120 %uint_120 %uint_30 %uint_47
       %1529 = OpExtInst %float %1 Cos %1528
       %2256 = OpExtInst %void %2 DebugLine %64 %uint_120 %uint_120 %uint_3 %uint_47
       %2255 = OpExtInst %void %2 DebugValue %215 %1529 %243
       %2259 = OpExtInst %void %2 DebugLine %64 %uint_121 %uint_121 %uint_3 %uint_22
       %2258 = OpExtInst %void %2 DebugValue %212 %float_100 %243
       %1698 = OpExtInst %void %2 DebugLine %64 %uint_124 %uint_124 %uint_26 %uint_49
       %1533 = OpAccessChain %_ptr_Uniform_UBO %ubo %int_0
       %1534 = OpAccessChain %_ptr_Uniform_v4float %1533 %int_1 %1503 %int_0
       %1700 = OpExtInst %void %2 DebugLine %64 %uint_124 %uint_124 %uint_26 %uint_40
       %1535 = OpLoad %v4float %1534
       %1701 = OpExtInst %void %2 DebugLine %64 %uint_124 %uint_124 %uint_26 %uint_49
       %1536 = OpVectorShuffle %v3float %1535 %1535 0 1 2
       %1703 = OpExtInst %void %2 DebugLine %64 %uint_124 %uint_124 %uint_55 %uint_76
       %1538 = OpAccessChain %_ptr_Uniform_UBO %ubo %int_0
       %1539 = OpAccessChain %_ptr_Uniform_v4float %1538 %int_1 %1503 %int_1
       %1705 = OpExtInst %void %2 DebugLine %64 %uint_124 %uint_124 %uint_55 %uint_69
       %1540 = OpLoad %v4float %1539
       %1706 = OpExtInst %void %2 DebugLine %64 %uint_124 %uint_124 %uint_55 %uint_76
       %1541 = OpVectorShuffle %v3float %1540 %1540 0 1 2
       %1707 = OpExtInst %void %2 DebugLine %64 %uint_124 %uint_124 %uint_26 %uint_76
       %1542 = OpFSub %v3float %1536 %1541
       %1708 = OpExtInst %void %2 DebugLine %64 %uint_124 %uint_124 %uint_16 %uint_79
       %1543 = OpExtInst %v3float %1 Normalize %1542
       %2262 = OpExtInst %void %2 DebugLine %64 %uint_124 %uint_124 %uint_3 %uint_79
       %2261 = OpExtInst %void %2 DebugValue %209 %1543 %243
       %1713 = OpExtInst %void %2 DebugLine %64 %uint_127 %uint_127 %uint_18 %uint_28
       %1547 = OpDot %float %1515 %1543
       %2265 = OpExtInst %void %2 DebugLine %64 %uint_127 %uint_127 %uint_3 %uint_28
       %2264 = OpExtInst %void %2 DebugValue %206 %1547 %243
       %1719 = OpExtInst %void %2 DebugLine %64 %uint_128 %uint_128 %uint_22 %uint_79
       %1552 = OpExtInst %float %1 SmoothStep %1529 %1526 %1547
       %2268 = OpExtInst %void %2 DebugLine %64 %uint_128 %uint_128 %uint_3 %uint_79
       %2267 = OpExtInst %void %2 DebugValue %203 %1552 %243
       %1724 = OpExtInst %void %2 DebugLine %64 %uint_129 %uint_129 %uint_29 %uint_62
       %1556 = OpExtInst %float %1 SmoothStep %float_100 %float_0 %1512
       %2271 = OpExtInst %void %2 DebugLine %64 %uint_129 %uint_129 %uint_3 %uint_62
       %2270 = OpExtInst %void %2 DebugValue %200 %1556 %243
       %1729 = OpExtInst %void %2 DebugLine %64 %uint_132 %uint_132 %uint_26 %uint_34
       %1560 = OpDot %float %1496 %1515
       %1730 = OpExtInst %void %2 DebugLine %64 %uint_132 %uint_132 %uint_17 %uint_35
       %1561 = OpExtInst %float %1 FMax %float_0 %1560
       %2274 = OpExtInst %void %2 DebugLine %64 %uint_132 %uint_132 %uint_3 %uint_35
       %2273 = OpExtInst %void %2 DebugValue %197 %1561 %243
       %1734 = OpExtInst %void %2 DebugLine %64 %uint_133 %uint_133 %uint_17 %uint_23
       %1564 = OpCompositeConstruct %v3float %1561 %1561 %1561
       %2277 = OpExtInst %void %2 DebugLine %64 %uint_133 %uint_133 %uint_3 %uint_23
       %2276 = OpExtInst %void %2 DebugValue %194 %1564 %243
       %1738 = OpExtInst %void %2 DebugLine %64 %uint_136 %uint_136 %uint_22 %uint_23
       %1567 = OpFNegate %v3float %1515
       %1740 = OpExtInst %void %2 DebugLine %64 %uint_136 %uint_136 %uint_14 %uint_27
       %1569 = OpExtInst %v3float %1 Reflect %1567 %1496
       %2280 = OpExtInst %void %2 DebugLine %64 %uint_136 %uint_136 %uint_3 %uint_27
       %2279 = OpExtInst %void %2 DebugValue %191 %1569 %243
       %1745 = OpExtInst %void %2 DebugLine %64 %uint_137 %uint_137 %uint_26 %uint_34
       %1573 = OpDot %float %1569 %1524
       %1746 = OpExtInst %void %2 DebugLine %64 %uint_137 %uint_137 %uint_17 %uint_35
       %1574 = OpExtInst %float %1 FMax %float_0 %1573
       %2283 = OpExtInst %void %2 DebugLine %64 %uint_137 %uint_137 %uint_3 %uint_35
       %2282 = OpExtInst %void %2 DebugValue %188 %1574 %243
       %1750 = OpExtInst %void %2 DebugLine %64 %uint_138 %uint_138 %uint_18 %uint_33
       %1577 = OpExtInst %float %1 Pow %1574 %float_16
       %1751 = OpExtInst %void %2 DebugLine %64 %uint_138 %uint_138 %uint_37 %uint_44
       %1578 = OpAccessChain %_ptr_Function_float %1447 %int_3
       %1579 = OpLoad %float %1578
       %1753 = OpExtInst %void %2 DebugLine %64 %uint_138 %uint_138 %uint_18 %uint_44
       %1580 = OpFMul %float %1577 %1579
       %1754 = OpExtInst %void %2 DebugLine %64 %uint_138 %uint_138 %uint_18 %uint_48
       %1581 = OpFMul %float %1580 %float_2_5
       %1755 = OpExtInst %void %2 DebugLine %64 %uint_138 %uint_138 %uint_17 %uint_53
       %1582 = OpCompositeConstruct %v3float %1581 %1581 %1581
       %2286 = OpExtInst %void %2 DebugLine %64 %uint_138 %uint_138 %uint_3 %uint_53
       %2285 = OpExtInst %void %2 DebugValue %185 %1582 %243
       %1760 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_24 %uint_31
       %1586 = OpFAdd %v3float %1564 %1582
       %1762 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_23 %uint_39
       %1588 = OpVectorTimesScalar %v3float %1586 %1552
       %1764 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_23 %uint_52
       %1590 = OpVectorTimesScalar %v3float %1588 %1556
       %1766 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_73 %uint_93
       %1592 = OpAccessChain %_ptr_Uniform_UBO %ubo %int_0
       %1593 = OpAccessChain %_ptr_Uniform_v4float %1592 %int_1 %1503 %int_2
       %1768 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_73 %uint_87
       %1594 = OpLoad %v4float %1593
       %1769 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_73 %uint_93
       %1595 = OpVectorShuffle %v3float %1594 %1594 0 1 2
       %1770 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_16 %uint_93
       %1596 = OpFMul %v3float %1590 %1595
       %1772 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_99 %uint_106
       %1598 = OpVectorShuffle %v3float %1489 %1489 0 1 2
       %1773 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_16 %uint_106
       %1599 = OpFMul %v3float %1596 %1598
       %1774 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_3 %uint_3
       %1600 = OpLoad %v3float %1448
       %1775 = OpExtInst %void %2 DebugLine %64 %uint_140 %uint_140 %uint_3 %uint_106
       %1601 = OpFAdd %v3float %1600 %1599
               OpStore %1448 %1601
       %2295 = OpExtInst %void %2 DebugScope %179
       %1777 = OpExtInst %void %2 DebugLine %64 %uint_107 %uint_107 %uint_34 %uint_36
               OpBranch %1602
       %1602 = OpLabel
       %2296 = OpExtInst %void %2 DebugScope %179
       %1778 = OpExtInst %void %2 DebugLine %64 %uint_107 %uint_107 %uint_34 %uint_36
       %1603 = OpLoad %int %1450
       %1604 = OpIAdd %int %1603 %int_1
               OpStore %1450 %1604
               OpBranch %1499
       %1605 = OpLabel
       %2297 = OpExtInst %void %2 DebugScope %179
       %1782 = OpExtInst %void %2 DebugLine %64 %uint_144 %uint_144 %uint_6 %uint_10
       %1606 = OpAccessChain %_ptr_Uniform_UBO %ubo %int_0
       %1607 = OpAccessChain %_ptr_Uniform_int %1606 %int_2
       %1608 = OpLoad %int %1607
       %1785 = OpExtInst %void %2 DebugLine %64 %uint_144 %uint_144 %uint_6 %uint_23
       %1609 = OpSGreaterThan %bool %1608 %int_0
       %2298 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %1614 None
               OpBranchConditional %1609 %1610 %1614
)"
      R"(    %1610 = OpLabel
       %2299 = OpExtInst %void %2 DebugScope %180
       %1788 = OpExtInst %void %2 DebugLine %64 %uint_146 %uint_146 %uint_22 %uint_22
       %1611 = OpLoad %v3float %1448
               OpStore %1466 %1611
       %2300 = OpExtInst %void %2 DebugScope %155 %1803
       %1842 = OpExtInst %void %2 DebugLine %64 %uint_78 %uint_78 %uint_15 %uint_22
       %1810 = OpExtInst %void %2 DebugDeclare %172 %1466 %243
       %2301 = OpExtInst %void %2 DebugScope %180
       %2289 = OpExtInst %void %2 DebugLine %64 %uint_146 %uint_146 %uint_33 %uint_33
       %2288 = OpExtInst %void %2 DebugValue %170 %1476 %243
       %2302 = OpExtInst %void %2 DebugScope %157 %1803
       %1844 = OpExtInst %void %2 DebugLine %64 %uint_79 %uint_79 %uint_7 %uint_15
               OpStore %1801 %int_0
       %1813 = OpExtInst %void %2 DebugDeclare %167 %1801 %243
       %1846 = OpExtInst %void %2 DebugLine %64 %uint_79 %uint_79 %uint_7 %uint_16
               OpBranch %1814
       %1814 = OpLabel
       %2303 = OpExtInst %void %2 DebugScope %157 %1803
       %1847 = OpExtInst %void %2 DebugLine %64 %uint_79 %uint_79 %uint_18 %uint_18
       %1815 = OpLoad %int %1801
       %1848 = OpExtInst %void %2 DebugLine %64 %uint_79 %uint_79 %uint_18 %uint_22
       %1816 = OpSLessThan %bool %1815 %int_3
       %2304 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %1840 %1837 None
               OpBranchConditional %1816 %1817 %1840
       %1817 = OpLabel
       %2305 = OpExtInst %void %2 DebugScope %158 %1803
       %1851 = OpExtInst %void %2 DebugLine %64 %uint_81 %uint_81 %uint_38 %uint_38
       %1818 = OpLoad %int %1801
       %1852 = OpExtInst %void %2 DebugLine %64 %uint_81 %uint_81 %uint_27 %uint_41
       %1819 = OpAccessChain %_ptr_Uniform_UBO %ubo %int_0
       %1820 = OpAccessChain %_ptr_Uniform_mat4v4float %1819 %int_1 %1818 %int_3
       %1821 = OpLoad %mat4v4float %1820
       %1856 = OpExtInst %void %2 DebugLine %64 %uint_81 %uint_81 %uint_60 %uint_68
       %1823 = OpCompositeExtract %float %1476 0
       %1824 = OpCompositeExtract %float %1476 1
       %1825 = OpCompositeExtract %float %1476 2
       %1859 = OpExtInst %void %2 DebugLine %64 %uint_81 %uint_81 %uint_53 %uint_76
       %1826 = OpCompositeConstruct %v4float %1823 %1824 %1825 %float_1
       %1860 = OpExtInst %void %2 DebugLine %64 %uint_81 %uint_81 %uint_23 %uint_77
       %1827 = OpVectorTimesMatrix %v4float %1826 %1821
       %2229 = OpExtInst %void %2 DebugLine %64 %uint_81 %uint_81 %uint_3 %uint_77
       %2228 = OpExtInst %void %2 DebugValue %163 %1827 %243
       %1867 = OpExtInst %void %2 DebugLine %64 %uint_85 %uint_85 %uint_40 %uint_40
       %1832 = OpConvertSToF %float %1818
       %2235 = OpExtInst %void %2 DebugLine %64 %uint_85 %uint_85 %uint_28 %uint_28
       %2234 = OpExtInst %void %2 DebugValue %151 %1827 %243
       %2238 = OpExtInst %void %2 DebugLine %64 %uint_85 %uint_85 %uint_40 %uint_40
       %2237 = OpExtInst %void %2 DebugValue %148 %1832 %243
       %2306 = OpExtInst %void %2 DebugScope %103 %1885
       %1983 = OpExtInst %void %2 DebugLine %64 %uint_56 %uint_56 %uint_2 %uint_7
       %1904 = OpExtInst %void %2 DebugDeclare %146 %1883 %243
       %1986 = OpExtInst %void %2 DebugLine %64 %uint_57 %uint_57 %uint_2 %uint_2
       %1907 = OpLoad %type_2d_image_array %textureShadowMap
)"
      R"(    %1987 = OpExtInst %void %2 DebugLine %64 %uint_57 %uint_57 %uint_2 %uint_72
       %1908 = OpImageQuerySizeLod %v3uint %1907 %uint_0
       %1909 = OpCompositeExtract %uint %1908 0
       %1910 = OpBitcast %int %1909
       %1911 = OpAccessChain %_ptr_Function_int %1883 %int_0
               OpStore %1911 %1910
       %1912 = OpCompositeExtract %uint %1908 1
       %1913 = OpBitcast %int %1912
       %1914 = OpAccessChain %_ptr_Function_int %1883 %int_1
               OpStore %1914 %1913
       %1915 = OpCompositeExtract %uint %1908 2
       %1916 = OpBitcast %int %1915
       %2204 = OpExtInst %void %2 DebugValue %142 %1916 %243
       %1999 = OpExtInst %void %2 DebugLine %64 %uint_57 %uint_57 %uint_19 %uint_19
       %1917 = OpImageQueryLevels %uint %1907
       %2000 = OpExtInst %void %2 DebugLine %64 %uint_57 %uint_57 %uint_2 %uint_72
       %1918 = OpBitcast %int %1917
       %2207 = OpExtInst %void %2 DebugValue %138 %1918 %243
       %2211 = OpExtInst %void %2 DebugLine %64 %uint_58 %uint_58 %uint_2 %uint_16
       %2210 = OpExtInst %void %2 DebugValue %135 %float_1_5 %243
       %2005 = OpExtInst %void %2 DebugLine %64 %uint_59 %uint_59 %uint_13 %uint_21
       %1921 = OpFMul %float %float_1_5 %float_1
       %2006 = OpExtInst %void %2 DebugLine %64 %uint_59 %uint_59 %uint_33 %uint_40
       %1922 = OpAccessChain %_ptr_Function_int %1883 %int_0
       %1923 = OpLoad %int %1922
       %1924 = OpConvertSToF %float %1923
       %2009 = OpExtInst %void %2 DebugLine %64 %uint_59 %uint_59 %uint_13 %uint_41
       %1925 = OpFDiv %float %1921 %1924
       %2214 = OpExtInst %void %2 DebugLine %64 %uint_59 %uint_59 %uint_2 %uint_41
       %2213 = OpExtInst %void %2 DebugValue %132 %1925 %243
       %2013 = OpExtInst %void %2 DebugLine %64 %uint_60 %uint_60 %uint_13 %uint_21
       %1928 = OpFMul %float %float_1_5 %float_1
       %2014 = OpExtInst %void %2 DebugLine %64 %uint_60 %uint_60 %uint_33 %uint_40
       %1929 = OpAccessChain %_ptr_Function_int %1883 %int_1
       %1930 = OpLoad %int %1929
       %1931 = OpConvertSToF %float %1930
       %2017 = OpExtInst %void %2 DebugLine %64 %uint_60 %uint_60 %uint_13 %uint_41
       %1932 = OpFDiv %float %1928 %1931
       %2217 = OpExtInst %void %2 DebugLine %64 %uint_60 %uint_60 %uint_2 %uint_41
       %2216 = OpExtInst %void %2 DebugValue %129 %1932 %243
       %2020 = OpExtInst %void %2 DebugLine %64 %uint_62 %uint_62 %uint_2 %uint_23
               OpStore %1891 %float_0
       %1934 = OpExtInst %void %2 DebugDeclare %126 %1891 %243
       %2022 = OpExtInst %void %2 DebugLine %64 %uint_63 %uint_63 %uint_2 %uint_14
               OpStore %1892 %int_0
       %1935 = OpExtInst %void %2 DebugDeclare %123 %1892 %243
       %2220 = OpExtInst %void %2 DebugLine %64 %uint_64 %uint_64 %uint_2 %uint_14
       %2219 = OpExtInst %void %2 DebugValue %119 %int_1 %243
       %2027 = OpExtInst %void %2 DebugLine %64 %uint_66 %uint_66 %uint_15 %uint_16
       %1938 = OpSNegate %int %int_1
       %2028 = OpExtInst %void %2 DebugLine %64 %uint_66 %uint_66 %uint_7 %uint_16
               OpStore %1894 %1938
       %1939 = OpExtInst %void %2 DebugDeclare %115 %1894 %243
       %2030 = OpExtInst %void %2 DebugLine %64 %uint_66 %uint_66 %uint_7 %uint_21
               OpBranch %1940
       %1940 = OpLabel
       %2307 = OpExtInst %void %2 DebugScope %103 %1885
       %2031 = OpExtInst %void %2 DebugLine %64 %uint_66 %uint_66 %uint_23 %uint_23
       %1941 = OpLoad %int %1894
       %2033 = OpExtInst %void %2 DebugLine %64 %uint_66 %uint_66 %uint_23 %uint_28
       %1943 = OpSLessThanEqual %bool %1941 %int_1
       %2308 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %1976 %1973 None
               OpBranchConditional %1943 %1944 %1976
       %1944 = OpLabel
       %2309 = OpExtInst %void %2 DebugScope %104 %1885
       %2037 = OpExtInst %void %2 DebugLine %64 %uint_68 %uint_68 %uint_16 %uint_17
       %1946 = OpSNegate %int %int_1
       %2038 = OpExtInst %void %2 DebugLine %64 %uint_68 %uint_68 %uint_8 %uint_17
               OpStore %1895 %1946
       %1947 = OpExtInst %void %2 DebugDeclare %111 %1895 %243
       %2040 = OpExtInst %void %2 DebugLine %64 %uint_68 %uint_68 %uint_8 %uint_22
               OpBranch %1948
       %1948 = OpLabel
       %2310 = OpExtInst %void %2 DebugScope %104 %1885
       %2041 = OpExtInst %void %2 DebugLine %64 %uint_68 %uint_68 %uint_24 %uint_24
       %1949 = OpLoad %int %1895
       %2043 = OpExtInst %void %2 DebugLine %64 %uint_68 %uint_68 %uint_24 %uint_29
       %1951 = OpSLessThanEqual %bool %1949 %int_1
       %2311 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %1972 %1969 None
               OpBranchConditional %1951 %1952 %1972
       %1952 = OpLabel
       %2312 = OpExtInst %void %2 DebugScope %106 %1885
       %2047 = OpExtInst %void %2 DebugLine %64 %uint_70 %uint_70 %uint_32 %uint_32
               OpStore %1896 %1827
       %2051 = OpExtInst %void %2 DebugLine %64 %uint_70 %uint_70 %uint_53 %uint_53
       %1956 = OpLoad %int %1894
       %1957 = OpConvertSToF %float %1956
       %2053 = OpExtInst %void %2 DebugLine %64 %uint_70 %uint_70 %uint_50 %uint_53
       %1958 = OpFMul %float %1925 %1957
       %2055 = OpExtInst %void %2 DebugLine %64 %uint_70 %uint_70 %uint_59 %uint_59
       %1960 = OpLoad %int %1895
       %1961 = OpConvertSToF %float %1960
       %2057 = OpExtInst %void %2 DebugLine %64 %uint_70 %uint_70 %uint_56 %uint_59
       %1962 = OpFMul %float %1932 %1961
       %2058 = OpExtInst %void %2 DebugLine %64 %uint_70 %uint_70 %uint_43 %uint_60
       %1963 = OpCompositeConstruct %v2float %1958 %1962
       %2313 = OpExtInst %void %2 DebugScope %70 %2085
       %2141 = OpExtInst %void %2 DebugLine %64 %uint_37 %uint_37 %uint_19 %uint_26
       %2090 = OpExtInst %void %2 DebugDeclare %96 %1896 %243
       %2314 = OpExtInst %void %2 DebugScope %106 %1885
       %2223 = OpExtInst %void %2 DebugLine %64 %uint_70 %uint_70 %uint_36 %uint_36
       %2222 = OpExtInst %void %2 DebugValue %93 %1832 %243
       %2226 = OpExtInst %void %2 DebugLine %64 %uint_70 %uint_70 %uint_43 %uint_60
       %2225 = OpExtInst %void %2 DebugValue %90 %1963 %243
       %2315 = OpExtInst %void %2 DebugScope %73 %2085
       %2144 = OpExtInst %void %2 DebugLine %64 %uint_39 %uint_39 %uint_2 %uint_17
               OpStore %2083 %float_1
       %2094 = OpExtInst %void %2 DebugDeclare %86 %2083 %243
       %2147 = OpExtInst %void %2 DebugLine %64 %uint_40 %uint_40 %uint_27 %uint_29
       %2096 = OpAccessChain %_ptr_Function_float %1896 %int_3
       %2097 = OpLoad %float %2096
       %2098 = OpCompositeConstruct %v4float %2097 %2097 %2097 %2097
       %2150 = OpExtInst %void %2 DebugLine %64 %uint_40 %uint_40 %uint_23 %uint_29
       %2099 = OpFDiv %v4float %1827 %2098
       %2151 = OpExtInst %void %2 DebugLine %64 %uint_40 %uint_40 %uint_2 %uint_29
               OpStore %2086 %2099
       %2100 = OpExtInst %void %2 DebugDeclare %83 %2086 %243
       %2154 = OpExtInst %void %2 DebugLine %64 %uint_41 %uint_41 %uint_19 %uint_31
       %2102 = OpVectorShuffle %v2float %2099 %2099 0 1
       %2155 = OpExtInst %void %2 DebugLine %64 %uint_41 %uint_41 %uint_19 %uint_36
       %2103 = OpVectorTimesScalar %v2float %2102 %float_0_5
       %2156 = OpExtInst %void %2 DebugLine %64 %uint_41 %uint_41 %uint_19 %uint_42
       %2104 = OpFAdd %v2float %2103 %35
       %2158 = OpExtInst %void %2 DebugLine %64 %uint_41 %uint_41 %uint_2 %uint_42
       %2106 = OpVectorShuffle %v4float %2099 %2104 4 5 2 3
               OpStore %2086 %2106
       %2160 = OpExtInst %void %2 DebugLine %64 %uint_43 %uint_43 %uint_6 %uint_18
       %2107 = OpAccessChain %_ptr_Function_float %2086 %int_2
       %2108 = OpLoad %float %2107
       %2162 = OpExtInst %void %2 DebugLine %64 %uint_43 %uint_43 %uint_6 %uint_23
       %2109 = OpFOrdGreaterThan %bool %2108 %float_n1
       %2163 = OpExtInst %void %2 DebugLine %64 %uint_43 %uint_43 %uint_30 %uint_42
       %2110 = OpAccessChain %_ptr_Function_float %2086 %int_2
       %2111 = OpLoad %float %2110
       %2165 = OpExtInst %void %2 DebugLine %64 %uint_43 %uint_43 %uint_30 %uint_46
       %2112 = OpFOrdLessThan %bool %2111 %float_1
       %2166 = OpExtInst %void %2 DebugLine %64 %uint_43 %uint_43 %uint_6 %uint_46
       %2113 = OpLogicalAnd %bool %2109 %2112
       %2316 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %2139 None
               OpBranchConditional %2113 %2114 %2139
       %2114 = OpLabel
       %2317 = OpExtInst %void %2 DebugScope %74 %2085
       %2169 = OpExtInst %void %2 DebugLine %64 %uint_45 %uint_45 %uint_16 %uint_16
       %2115 = OpLoad %type_2d_image_array %textureShadowMap
       %2170 = OpExtInst %void %2 DebugLine %64 %uint_45 %uint_45 %uint_40 %uint_40
       %2116 = OpLoad %type_sampler %samplerShadowMap
       %2171 = OpExtInst %void %2 DebugLine %64 %uint_45 %uint_45 %uint_65 %uint_65
       %2117 = OpLoad %v4float %2086
       %2172 = OpExtInst %void %2 DebugLine %64 %uint_45 %uint_45 %uint_65 %uint_77
       %2118 = OpVectorShuffle %v2float %2117 %2117 0 1
       %2174 = OpExtInst %void %2 DebugLine %64 %uint_45 %uint_45 %uint_65 %uint_82
       %2120 = OpFAdd %v2float %2118 %1963
       %2122 = OpCompositeExtract %float %2120 0
       %2123 = OpCompositeExtract %float %2120 1
       %2178 = OpExtInst %void %2 DebugLine %64 %uint_45 %uint_45 %uint_58 %uint_95
       %2124 = OpCompositeConstruct %v3float %2122 %2123 %1832
       %2179 = OpExtInst %void %2 DebugLine %64 %uint_45 %uint_45 %uint_16 %uint_96
       %2125 = OpSampledImage %type_sampled_image_0 %2115 %2116
       %2126 = OpImageSampleImplicitLod %v4float %2125 %2124 None
       %2181 = OpExtInst %void %2 DebugLine %64 %uint_45 %uint_45 %uint_16 %uint_98
       %2127 = OpCompositeExtract %float %2126 0
       %2202 = OpExtInst %void %2 DebugLine %64 %uint_45 %uint_45 %uint_3 %uint_98
       %2201 = OpExtInst %void %2 DebugValue %79 %2127 %243
       %2184 = OpExtInst %void %2 DebugLine %64 %uint_46 %uint_46 %uint_7 %uint_19
       %2129 = OpAccessChain %_ptr_Function_float %2086 %int_3
       %2130 = OpLoad %float %2129
       %2186 = OpExtInst %void %2 DebugLine %64 %uint_46 %uint_46 %uint_7 %uint_23
       %2131 = OpFOrdGreaterThan %bool %2130 %float_0
       %2188 = OpExtInst %void %2 DebugLine %64 %uint_46 %uint_46 %uint_37 %uint_49
       %2133 = OpAccessChain %_ptr_Function_float %2086 %int_2
       %2134 = OpLoad %float %2133
       %2190 = OpExtInst %void %2 DebugLine %64 %uint_46 %uint_46 %uint_30 %uint_49
       %2135 = OpFOrdLessThan %bool %2127 %2134
       %2191 = OpExtInst %void %2 DebugLine %64 %uint_46 %uint_46 %uint_7 %uint_49
       %2136 = OpLogicalAnd %bool %2131 %2135
       %2318 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %2138 None
               OpBranchConditional %2136 %2137 %2138
       %2137 = OpLabel
       %2319 = OpExtInst %void %2 DebugScope %76 %2085
       %2194 = OpExtInst %void %2 DebugLine %64 %uint_48 %uint_48 %uint_4 %uint_13
               OpStore %2083 %float_0_25
       %2320 = OpExtInst %void %2 DebugScope %74 %2085
       %2195 = OpExtInst %void %2 DebugLine %64 %uint_49 %uint_49 %uint_3 %uint_3
               OpBranch %2138
       %2138 = OpLabel
       %2321 = OpExtInst %void %2 DebugScope %73 %2085
       %2196 = OpExtInst %void %2 DebugLine %64 %uint_50 %uint_50 %uint_2 %uint_2
               OpBranch %2139
       %2139 = OpLabel
       %2322 = OpExtInst %void %2 DebugScope %73 %2085
       %2197 = OpExtInst %void %2 DebugLine %64 %uint_51 %uint_51 %uint_9 %uint_9
       %2140 = OpLoad %float %2083
       %2323 = OpExtInst %void %2 DebugScope %106 %1885
       %2061 = OpExtInst %void %2 DebugLine %64 %uint_70 %uint_70 %uint_4 %uint_4
       %1965 = OpLoad %float %1891
       %2062 = OpExtInst %void %2 DebugLine %64 %uint_70 %uint_70 %uint_4 %uint_61
       %1966 = OpFAdd %float %1965 %2140
               OpStore %1891 %1966
       %2064 = OpExtInst %void %2 DebugLine %64 %uint_71 %uint_71 %uint_4 %uint_9
       %1967 = OpLoad %int %1892
       %1968 = OpIAdd %int %1967 %int_1
               OpStore %1892 %1968
       %2324 = OpExtInst %void %2 DebugScope %104 %1885
       %2067 = OpExtInst %void %2 DebugLine %64 %uint_68 %uint_68 %uint_36 %uint_37
               OpBranch %1969
       %1969 = OpLabel
       %2325 = OpExtInst %void %2 DebugScope %104 %1885
       %2068 = OpExtInst %void %2 DebugLine %64 %uint_68 %uint_68 %uint_36 %uint_37
       %1970 = OpLoad %int %1895
       %1971 = OpIAdd %int %1970 %int_1
               OpStore %1895 %1971
               OpBranch %1948
       %1972 = OpLabel
       %2326 = OpExtInst %void %2 DebugScope %103 %1885
       %2072 = OpExtInst %void %2 DebugLine %64 %uint_66 %uint_66 %uint_35 %uint_36
               OpBranch %1973
       %1973 = OpLabel
       %2327 = OpExtInst %void %2 DebugScope %103 %1885
       %2073 = OpExtInst %void %2 DebugLine %64 %uint_66 %uint_66 %uint_35 %uint_36
       %1974 = OpLoad %int %1894
       %1975 = OpIAdd %int %1974 %int_1
               OpStore %1894 %1975
               OpBranch %1940
       %1976 = OpLabel
       %2328 = OpExtInst %void %2 DebugScope %103 %1885
       %2077 = OpExtInst %void %2 DebugLine %64 %uint_75 %uint_75 %uint_9 %uint_9
       %1977 = OpLoad %float %1891
       %2078 = OpExtInst %void %2 DebugLine %64 %uint_75 %uint_75 %uint_24 %uint_24
       %1978 = OpLoad %int %1892
       %1979 = OpConvertSToF %float %1978
       %2080 = OpExtInst %void %2 DebugLine %64 %uint_75 %uint_75 %uint_9 %uint_24
       %1980 = OpFDiv %float %1977 %1979
       %2329 = OpExtInst %void %2 DebugScope %158 %1803
       %2232 = OpExtInst %void %2 DebugLine %64 %uint_85 %uint_85 %uint_4 %uint_41
       %2231 = OpExtInst %void %2 DebugValue %160 %1980 %243
       %1872 = OpExtInst %void %2 DebugLine %64 %uint_90 %uint_90 %uint_3 %uint_3
       %1835 = OpLoad %v3float %1466
       %1873 = OpExtInst %void %2 DebugLine %64 %uint_90 %uint_90 %uint_3 %uint_16
       %1836 = OpVectorTimesScalar %v3float %1835 %1980
               OpStore %1466 %1836
       %2330 = OpExtInst %void %2 DebugScope %157 %1803
       %1875 = OpExtInst %void %2 DebugLine %64 %uint_79 %uint_79 %uint_35 %uint_37
               OpBranch %1837
       %1837 = OpLabel
       %2331 = OpExtInst %void %2 DebugScope %157 %1803
       %1876 = OpExtInst %void %2 DebugLine %64 %uint_79 %uint_79 %uint_35 %uint_37
       %1838 = OpLoad %int %1801
       %1839 = OpIAdd %int %1838 %int_1
               OpStore %1801 %1839
               OpBranch %1814
       %1840 = OpLabel
       %2332 = OpExtInst %void %2 DebugScope %157 %1803
       %1880 = OpExtInst %void %2 DebugLine %64 %uint_92 %uint_92 %uint_9 %uint_9
       %1841 = OpLoad %v3float %1466
       %2333 = OpExtInst %void %2 DebugScope %180
       %1793 = OpExtInst %void %2 DebugLine %64 %uint_146 %uint_146 %uint_3 %uint_40
               OpStore %1448 %1841
       %2334 = OpExtInst %void %2 DebugScope %179
       %1794 = OpExtInst %void %2 DebugLine %64 %uint_147 %uint_147 %uint_2 %uint_2
               OpBranch %1614
       %1614 = OpLabel
;CHECK:      %1614 = OpLabel
;CHECK-NEXT: [[phi:%\w+]] = OpPhi 
;CHECK-NEXT: {{%\w+}} = OpExtInst %void {{%\w+}} DebugValue %233
       %2335 = OpExtInst %void %2 DebugScope %179
       %1795 = OpExtInst %void %2 DebugLine %64 %uint_149 %uint_149 %uint_16 %uint_16
       %1615 = OpLoad %v3float %1448
       %1616 = OpCompositeExtract %float %1615 0
       %1617 = OpCompositeExtract %float %1615 1
       %1618 = OpCompositeExtract %float %1615 2
       %1799 = OpExtInst %void %2 DebugLine %64 %uint_149 %uint_149 %uint_9 %uint_28
       %1619 = OpCompositeConstruct %v4float %1616 %1617 %1618 %float_1
       %2336 = OpExtInst %void %2 DebugNoLine
       %2337 = OpExtInst %void %2 DebugNoScope
               OpStore %out_var_SV_TARGET %1619
        %329 = OpExtInst %void %2 DebugLine %64 %uint_150 %uint_150 %uint_1 %uint_1
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<SSARewritePass>(text, true);
}

// TODO(greg-lunarg): Add tests to verify handling of these cases:
//
//    No optimization in the presence of
//      access chains
//      function calls
//      OpCopyMemory?
//      unsupported extensions
//    Others?

}  // namespace
}  // namespace opt
}  // namespace spvtools
