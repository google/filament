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

using LocalAccessChainConvertTest = PassTest<::testing::Test>;

TEST_F(LocalAccessChainConvertTest, StructOfVecsOfFloatConverted) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  struct S_t {
  //      vec4 v0;
  //      vec4 v1;
  //  };
  //
  //  void main()
  //  {
  //      S_t s0;
  //      s0.v1 = BaseColor;
  //      gl_FragColor = s0.v1;
  //  }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %S_t "S_t"
OpMemberName %S_t 0 "v0"
OpMemberName %S_t 1 "v1"
OpName %s0 "s0"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%S_t = OpTypeStruct %v4float %v4float
%_ptr_Function_S_t = OpTypePointer Function %S_t
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(
; CHECK: [[st_id:%\w+]] = OpLoad %v4float %BaseColor
; CHECK: [[ld1:%\w+]] = OpLoad %S_t %s0
; CHECK: [[ex1:%\w+]] = OpCompositeInsert %S_t [[st_id]] [[ld1]] 1
; CHECK: OpStore %s0 [[ex1]]
; CHECK: [[ld2:%\w+]] = OpLoad %S_t %s0
; CHECK: [[ex2:%\w+]] = OpCompositeExtract %v4float [[ld2]] 1
; CHECK: OpStore %gl_FragColor [[ex2]]
%main = OpFunction %void None %8
%17 = OpLabel
%s0 = OpVariable %_ptr_Function_S_t Function
%18 = OpLoad %v4float %BaseColor
%19 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
OpStore %19 %18
%20 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
%21 = OpLoad %v4float %20
OpStore %gl_FragColor %21
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(predefs_before + before,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, DebugScopeAndLineInfoForNewInstructions) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  struct S_t {
  //      vec4 v0;
  //      vec4 v1;
  //  };
  //
  //  void main()
  //  {
  //      S_t s0;
  //      s0.v1 = BaseColor;
  //      gl_FragColor = s0.v1;
  //  }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %S_t "S_t"
OpMemberName %S_t 0 "v0"
OpMemberName %S_t 1 "v1"
OpName %s0 "s0"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%5 = OpString "ps.hlsl"
%6 = OpString "float"
%var_name = OpString "s0"
%main_name = OpString "main"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%S_t = OpTypeStruct %v4float %v4float
%_ptr_Function_S_t = OpTypePointer Function %S_t
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_32 = OpConstant %int 32
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%20 = OpExtInst %void %ext DebugSource %5
%21 = OpExtInst %void %ext DebugCompilationUnit 1 4 %20 HLSL
%22 = OpExtInst %void %ext DebugTypeBasic %6 %int_32 Float
%23 = OpExtInst %void %ext DebugTypeVector %22 4
%24 = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %23
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %24 %20 4 1 %21 %main_name FlagIsProtected|FlagIsPrivate 4 %main
%25 = OpExtInst %void %ext DebugLocalVariable %var_name %23 %20 0 0 %dbg_main FlagIsLocal
)";

  const std::string before =
      R"(
; CHECK: [[st_id:%\w+]] = OpLoad %v4float %BaseColor
; CHECK: OpLine {{%\w+}} 1 0
; CHECK: [[ld1:%\w+]] = OpLoad %S_t %s0
; CHECK: [[ex1:%\w+]] = OpCompositeInsert %S_t [[st_id]] [[ld1]] 1
; CHECK: OpStore %s0 [[ex1]]
; CHECK: OpLine {{%\w+}} 3 0
; CHECK: [[ld2:%\w+]] = OpLoad %S_t %s0
; CHECK: [[ex2:%\w+]] = OpCompositeExtract %v4float [[ld2]] 1
; CHECK: OpLine {{%\w+}} 4 0
; CHECK: OpStore %gl_FragColor [[ex2]]
%main = OpFunction %void None %8
%17 = OpLabel
%26 = OpExtInst %void %ext DebugScope %dbg_main
%s0 = OpVariable %_ptr_Function_S_t Function
%18 = OpLoad %v4float %BaseColor
OpLine %5 0 0
%19 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
OpLine %5 1 0
OpStore %19 %18
OpLine %5 2 0
%27 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
OpLine %5 3 0
%28 = OpLoad %v4float %27
OpLine %5 4 0
OpStore %gl_FragColor %28
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(predefs_before + before,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, TestTargetsReferencedByDebugValue) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  struct S_t {
  //      vec4 v0;
  //      vec4 v1;
  //  };
  //
  //  void main()
  //  {
  //      S_t s0;
  //      s0.v1 = BaseColor;
  //      gl_FragColor = s0.v1;
  //  }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %S_t "S_t"
OpMemberName %S_t 0 "v0"
OpMemberName %S_t 1 "v1"
OpName %s0 "s0"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%5 = OpString "ps.hlsl"
%6 = OpString "float"
%var_name = OpString "s0"
%main_name = OpString "main"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%S_t = OpTypeStruct %v4float %v4float
%_ptr_Function_S_t = OpTypePointer Function %S_t
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_32 = OpConstant %int 32
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%deref = OpExtInst %void %ext DebugOperation Deref
%deref_expr = OpExtInst %void %ext DebugExpression %deref
%null_expr = OpExtInst %void %ext DebugExpression
%20 = OpExtInst %void %ext DebugSource %5
%21 = OpExtInst %void %ext DebugCompilationUnit 1 4 %20 HLSL
%22 = OpExtInst %void %ext DebugTypeBasic %6 %int_32 Float
%23 = OpExtInst %void %ext DebugTypeVector %22 4
%24 = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %23
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %24 %20 4 1 %21 %main_name FlagIsProtected|FlagIsPrivate 4 %main
%25 = OpExtInst %void %ext DebugLocalVariable %var_name %23 %20 0 0 %dbg_main FlagIsLocal
)";

  const std::string before =
      R"(
; CHECK: [[st_id:%\w+]] = OpLoad %v4float %BaseColor
; CHECK: OpLine {{%\w+}} 0 0
; CHECK: [[s0_1_ptr:%\w+]] = OpAccessChain %_ptr_Function_v4float %s0 %int_1
; CHECK: DebugValue [[dbg_s0:%\w+]] [[s0_1_ptr]]
; CHECK: OpLine {{%\w+}} 1 0
; CHECK: [[s0:%\w+]] = OpLoad %S_t %s0
; CHECK: [[comp:%\w+]] = OpCompositeInsert %S_t [[st_id]] [[s0]] 1
; CHECK: OpStore %s0 [[comp]]
; CHECK: OpLine {{%\w+}} 2 0
; CHECK: [[s0_2_ptr:%\w+]] = OpAccessChain %_ptr_Function_v4float %s0 %int_1
; CHECK: OpLine {{%\w+}} 3 0
; CHECK: [[s0:%\w+]] = OpLoad %S_t %s0
; CHECK: [[s0_2_val:%\w+]] = OpCompositeExtract %v4float [[s0]] 1
; CHECK: DebugValue [[dbg_s0]] [[s0_2_val]]
; CHECK: OpLine {{%\w+}} 4 0
; CHECK: OpStore %gl_FragColor [[s0_2_val]]
%main = OpFunction %void None %8
%17 = OpLabel
%26 = OpExtInst %void %ext DebugScope %dbg_main
%s0 = OpVariable %_ptr_Function_S_t Function
%18 = OpLoad %v4float %BaseColor
OpLine %5 0 0
%19 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
%29 = OpExtInst %void %ext DebugValue %25 %19 %deref_expr %int_1
OpLine %5 1 0
OpStore %19 %18
OpLine %5 2 0
%27 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
OpLine %5 3 0
%28 = OpLoad %v4float %27
%30 = OpExtInst %void %ext DebugValue %25 %28 %null_expr %int_1
OpLine %5 4 0
OpStore %gl_FragColor %28
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(predefs_before + before,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, InBoundsAccessChainsConverted) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  struct S_t {
  //      vec4 v0;
  //      vec4 v1;
  //  };
  //
  //  void main()
  //  {
  //      S_t s0;
  //      s0.v1 = BaseColor;
  //      gl_FragColor = s0.v1;
  //  }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %S_t "S_t"
OpMemberName %S_t 0 "v0"
OpMemberName %S_t 1 "v1"
OpName %s0 "s0"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%S_t = OpTypeStruct %v4float %v4float
%_ptr_Function_S_t = OpTypePointer Function %S_t
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(
; CHECK: [[st_id:%\w+]] = OpLoad %v4float %BaseColor
; CHECK: [[ld1:%\w+]] = OpLoad %S_t %s0
; CHECK: [[ex1:%\w+]] = OpCompositeInsert %S_t [[st_id]] [[ld1]] 1
; CHECK: OpStore %s0 [[ex1]]
; CHECK: [[ld2:%\w+]] = OpLoad %S_t %s0
; CHECK: [[ex2:%\w+]] = OpCompositeExtract %v4float [[ld2]] 1
; CHECK: OpStore %gl_FragColor [[ex2]]
%main = OpFunction %void None %8
%17 = OpLabel
%s0 = OpVariable %_ptr_Function_S_t Function
%18 = OpLoad %v4float %BaseColor
%19 = OpInBoundsAccessChain %_ptr_Function_v4float %s0 %int_1
OpStore %19 %18
%20 = OpInBoundsAccessChain %_ptr_Function_v4float %s0 %int_1
%21 = OpLoad %v4float %20
OpStore %gl_FragColor %21
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(predefs_before + before,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, TwoUsesofSingleChainConverted) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  struct S_t {
  //      vec4 v0;
  //      vec4 v1;
  //  };
  //
  //  void main()
  //  {
  //      S_t s0;
  //      s0.v1 = BaseColor;
  //      gl_FragColor = s0.v1;
  //  }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %S_t "S_t"
OpMemberName %S_t 0 "v0"
OpMemberName %S_t 1 "v1"
OpName %s0 "s0"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%S_t = OpTypeStruct %v4float %v4float
%_ptr_Function_S_t = OpTypePointer Function %S_t
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(
; CHECK: [[st_id:%\w+]] = OpLoad %v4float %BaseColor
; CHECK: [[ld1:%\w+]] = OpLoad %S_t %s0
; CHECK: [[ex1:%\w+]] = OpCompositeInsert %S_t [[st_id]] [[ld1]] 1
; CHECK: OpStore %s0 [[ex1]]
; CHECK: [[ld2:%\w+]] = OpLoad %S_t %s0
; CHECK: [[ex2:%\w+]] = OpCompositeExtract %v4float [[ld2]] 1
; CHECK: OpStore %gl_FragColor [[ex2]]
%main = OpFunction %void None %8
%17 = OpLabel
%s0 = OpVariable %_ptr_Function_S_t Function
%18 = OpLoad %v4float %BaseColor
%19 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
OpStore %19 %18
%20 = OpLoad %v4float %19
OpStore %gl_FragColor %20
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(predefs_before + before,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, OpaqueConverted) {
  // SPIR-V not representable in GLSL; not generatable from HLSL
  // at the moment

  const std::string predefs =
      R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %outColor %texCoords
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %S_t "S_t"
OpMemberName %S_t 0 "v0"
OpMemberName %S_t 1 "v1"
OpMemberName %S_t 2 "smp"
OpName %foo_struct_S_t_vf2_vf21_ "foo(struct-S_t-vf2-vf21;"
OpName %s "s"
OpName %outColor "outColor"
OpName %sampler15 "sampler15"
OpName %s0 "s0"
OpName %texCoords "texCoords"
OpName %param "param"
OpDecorate %sampler15 DescriptorSet 0
%void = OpTypeVoid
%12 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%outColor = OpVariable %_ptr_Output_v4float Output
%17 = OpTypeImage %float 2D 0 0 0 1 Unknown
%18 = OpTypeSampledImage %17
%S_t = OpTypeStruct %v2float %v2float %18
%_ptr_Function_S_t = OpTypePointer Function %S_t
%20 = OpTypeFunction %void %_ptr_Function_S_t
%_ptr_UniformConstant_18 = OpTypePointer UniformConstant %18
%_ptr_Function_18 = OpTypePointer Function %18
%sampler15 = OpVariable %_ptr_UniformConstant_18 UniformConstant
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Input_v2float = OpTypePointer Input %v2float
%texCoords = OpVariable %_ptr_Input_v2float Input
)";

  const std::string before =
      R"(
; CHECK: [[l1:%\w+]] = OpLoad %S_t %param
; CHECK: [[e1:%\w+]] = OpCompositeExtract {{%\w+}} [[l1]] 2
; CHECK: [[l2:%\w+]] = OpLoad %S_t %param
; CHECK: [[e2:%\w+]] = OpCompositeExtract {{%\w+}} [[l2]] 0
; CHECK: OpImageSampleImplicitLod {{%\w+}} [[e1]] [[e2]]
%main = OpFunction %void None %12
%28 = OpLabel
%s0 = OpVariable %_ptr_Function_S_t Function
%param = OpVariable %_ptr_Function_S_t Function
%29 = OpLoad %v2float %texCoords
%30 = OpAccessChain %_ptr_Function_v2float %s0 %int_0
OpStore %30 %29
%31 = OpLoad %18 %sampler15
%32 = OpAccessChain %_ptr_Function_18 %s0 %int_2
OpStore %32 %31
%33 = OpLoad %S_t %s0
OpStore %param %33
%34 = OpAccessChain %_ptr_Function_18 %param %int_2
%35 = OpLoad %18 %34
%36 = OpAccessChain %_ptr_Function_v2float %param %int_0
%37 = OpLoad %v2float %36
%38 = OpImageSampleImplicitLod %v4float %35 %37
OpStore %outColor %38
OpReturn
OpFunctionEnd
)";

  const std::string remain =
      R"(%foo_struct_S_t_vf2_vf21_ = OpFunction %void None %20
%s = OpFunctionParameter %_ptr_Function_S_t
%39 = OpLabel
%40 = OpAccessChain %_ptr_Function_18 %s %int_2
%41 = OpLoad %18 %40
%42 = OpAccessChain %_ptr_Function_v2float %s %int_0
%43 = OpLoad %v2float %42
%44 = OpImageSampleImplicitLod %v4float %41 %43
OpStore %outColor %44
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(predefs + before + remain,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, NestedStructsConverted) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  struct S1_t {
  //      vec4 v1;
  //  };
  //
  //  struct S2_t {
  //      vec4 v2;
  //      S1_t s1;
  //  };
  //
  //  void main()
  //  {
  //      S2_t s2;
  //      s2.s1.v1 = BaseColor;
  //      gl_FragColor = s2.s1.v1;
  //  }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %S1_t "S1_t"
OpMemberName %S1_t 0 "v1"
OpName %S2_t "S2_t"
OpMemberName %S2_t 0 "v2"
OpMemberName %S2_t 1 "s1"
OpName %s2 "s2"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%S1_t = OpTypeStruct %v4float
%S2_t = OpTypeStruct %v4float %S1_t
%_ptr_Function_S2_t = OpTypePointer Function %S2_t
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(
; CHECK: [[st_id:%\w+]] = OpLoad %v4float %BaseColor
; CHECK: [[ld1:%\w+]] = OpLoad %S2_t %s2
; CHECK: [[ex1:%\w+]] = OpCompositeInsert %S2_t [[st_id]] [[ld1]] 1 0
; CHECK: OpStore %s2 [[ex1]]
; CHECK: [[ld2:%\w+]] = OpLoad %S2_t %s2
; CHECK: [[ex2:%\w+]] = OpCompositeExtract %v4float [[ld2]] 1 0
; CHECK: OpStore %gl_FragColor [[ex2]]
%main = OpFunction %void None %9
%19 = OpLabel
%s2 = OpVariable %_ptr_Function_S2_t Function
%20 = OpLoad %v4float %BaseColor
%21 = OpAccessChain %_ptr_Function_v4float %s2 %int_1 %int_0
OpStore %21 %20
%22 = OpAccessChain %_ptr_Function_v4float %s2 %int_1 %int_0
%23 = OpLoad %v4float %22
OpStore %gl_FragColor %23
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(predefs_before + before,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, SomeAccessChainsHaveNoUse) {
  // Based on HLSL source code:
  // struct S {
  //   float f;
  // };

  // float main(float input : A) : B {
  //   S local = { input };
  //   return local.f;
  // }

  const std::string predefs = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %in_var_A %out_var_B
OpName %main "main"
OpName %in_var_A "in.var.A"
OpName %out_var_B "out.var.B"
OpName %S "S"
OpName %local "local"
%int = OpTypeInt 32 1
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_float = OpTypePointer Output %float
%S = OpTypeStruct %float
%_ptr_Function_S = OpTypePointer Function %S
%int_0 = OpConstant %int 0
%in_var_A = OpVariable %_ptr_Input_float Input
%out_var_B = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %8
%15 = OpLabel
%local = OpVariable %_ptr_Function_S Function
%16 = OpLoad %float %in_var_A
%17 = OpCompositeConstruct %S %16
OpStore %local %17
)";

  const std::string before =
      R"(
; CHECK: [[ld:%\w+]] = OpLoad %S %local
; CHECK: [[ex:%\w+]] = OpCompositeExtract %float [[ld]] 0
; CHECK: OpStore %out_var_B [[ex]]
%18 = OpAccessChain %_ptr_Function_float %local %int_0
%19 = OpAccessChain %_ptr_Function_float %local %int_0
%20 = OpLoad %float %18
OpStore %out_var_B %20
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(predefs + before, true);
}

TEST_F(LocalAccessChainConvertTest,
       StructOfVecsOfFloatConvertedWithDecorationOnLoad) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  struct S_t {
  //      vec4 v0;
  //      vec4 v1;
  //  };
  //
  //  void main()
  //  {
  //      S_t s0;
  //      s0.v1 = BaseColor;
  //      gl_FragColor = s0.v1;
  //  }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %S_t "S_t"
OpMemberName %S_t 0 "v0"
OpMemberName %S_t 1 "v1"
OpName %s0 "s0"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
OpDecorate %21 RelaxedPrecision
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%S_t = OpTypeStruct %v4float %v4float
%_ptr_Function_S_t = OpTypePointer Function %S_t
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(
; CHECK: OpDecorate
; CHECK: OpDecorate [[ld2:%\w+]] RelaxedPrecision
; CHECK-NOT: OpDecorate
; CHECK: [[st_id:%\w+]] = OpLoad %v4float %BaseColor
; CHECK: [[ld1:%\w+]] = OpLoad %S_t %s0
; CHECK: [[ins:%\w+]] = OpCompositeInsert %S_t [[st_id]] [[ld1]] 1
; CHECK: OpStore %s0 [[ins]]
; CHECK: [[ld2]] = OpLoad %S_t %s0
; CHECK: [[ex2:%\w+]] = OpCompositeExtract %v4float [[ld2]] 1
; CHECK: OpStore %gl_FragColor [[ex2]]
%main = OpFunction %void None %8
%17 = OpLabel
%s0 = OpVariable %_ptr_Function_S_t Function
%18 = OpLoad %v4float %BaseColor
%19 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
OpStore %19 %18
%20 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
%21 = OpLoad %v4float %20
OpStore %gl_FragColor %21
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(predefs_before + before,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest,
       StructOfVecsOfFloatConvertedWithDecorationOnStore) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  struct S_t {
  //      vec4 v0;
  //      vec4 v1;
  //  };
  //
  //  void main()
  //  {
  //      S_t s0;
  //      s0.v1 = BaseColor;
  //      gl_FragColor = s0.v1;
  //  }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %S_t "S_t"
OpMemberName %S_t 0 "v0"
OpMemberName %S_t 1 "v1"
OpName %s0 "s0"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
OpDecorate %s0 RelaxedPrecision
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%S_t = OpTypeStruct %v4float %v4float
%_ptr_Function_S_t = OpTypePointer Function %S_t
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(
; CHECK: OpDecorate
; CHECK: OpDecorate [[ld1:%\w+]] RelaxedPrecision
; CHECK: OpDecorate [[ins:%\w+]] RelaxedPrecision
; CHECK-NOT: OpDecorate
; CHECK: [[st_id:%\w+]] = OpLoad %v4float %BaseColor
; CHECK: [[ld1]] = OpLoad %S_t %s0
; CHECK: [[ins]] = OpCompositeInsert %S_t [[st_id]] [[ld1]] 1
; CHECK: OpStore %s0 [[ins]]
; CHECK: [[ld2:%\w+]] = OpLoad %S_t %s0
; CHECK: [[ex2:%\w+]] = OpCompositeExtract %v4float [[ld2]] 1
; CHECK: OpStore %gl_FragColor [[ex2]]
%main = OpFunction %void None %8
%17 = OpLabel
%s0 = OpVariable %_ptr_Function_S_t Function
%18 = OpLoad %v4float %BaseColor
%19 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
OpStore %19 %18
%20 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
%21 = OpLoad %v4float %20
OpStore %gl_FragColor %21
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(predefs_before + before,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, DynamicallyIndexedVarNotConverted) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //  flat in int Idx;
  //  in float Bi;
  //
  //  struct S_t {
  //      vec4 v0;
  //      vec4 v1;
  //  };
  //
  //  void main()
  //  {
  //      S_t s0;
  //      s0.v1 = BaseColor;
  //      s0.v1[Idx] = Bi;
  //      gl_FragColor = s0.v1;
  //  }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %Idx %Bi %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %S_t "S_t"
OpMemberName %S_t 0 "v0"
OpMemberName %S_t 1 "v1"
OpName %s0 "s0"
OpName %BaseColor "BaseColor"
OpName %Idx "Idx"
OpName %Bi "Bi"
OpName %gl_FragColor "gl_FragColor"
OpDecorate %Idx Flat
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%S_t = OpTypeStruct %v4float %v4float
%_ptr_Function_S_t = OpTypePointer Function %S_t
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_int = OpTypePointer Input %int
%Idx = OpVariable %_ptr_Input_int Input
%_ptr_Input_float = OpTypePointer Input %float
%Bi = OpVariable %_ptr_Input_float Input
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %10
%22 = OpLabel
%s0 = OpVariable %_ptr_Function_S_t Function
%23 = OpLoad %v4float %BaseColor
%24 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
OpStore %24 %23
%25 = OpLoad %int %Idx
%26 = OpLoad %float %Bi
%27 = OpAccessChain %_ptr_Function_float %s0 %int_1 %25
OpStore %27 %26
%28 = OpAccessChain %_ptr_Function_v4float %s0 %int_1
%29 = OpLoad %v4float %28
OpStore %gl_FragColor %29
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalAccessChainConvertPass>(assembly, assembly, false,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, VariablePointersStorageBuffer) {
  // A case with a storage buffer variable pointer.  We should still convert
  // the access chain on the function scope symbol.
  const std::string test =
      R"(
; CHECK: OpFunction
; CHECK: [[var:%\w+]] = OpVariable {{%\w+}} Function
; CHECK: [[ld:%\w+]] = OpLoad {{%\w+}} [[var]]
; CHECK: OpCompositeExtract {{%\w+}} [[ld]] 0 0
               OpCapability Shader
               OpCapability VariablePointersStorageBuffer
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "main"
               OpExecutionMode %2 LocalSize 1 1 1
               OpSource GLSL 450
               OpMemberDecorate %_struct_3 0 Offset 0
               OpDecorate %_struct_3 Block
               OpDecorate %4 DescriptorSet 0
               OpDecorate %4 Binding 0
               OpDecorate %_ptr_StorageBuffer_int ArrayStride 4
               OpDecorate %_arr_int_int_128 ArrayStride 4
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
        %int = OpTypeInt 32 1
    %int_128 = OpConstant %int 128
%_arr_int_int_128 = OpTypeArray %int %int_128
  %_struct_3 = OpTypeStruct %_arr_int_int_128
%_ptr_StorageBuffer__struct_3 = OpTypePointer StorageBuffer %_struct_3
%_ptr_Function__struct_3 = OpTypePointer Function %_struct_3
          %4 = OpVariable %_ptr_StorageBuffer__struct_3 StorageBuffer
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
%_ptr_Function_int = OpTypePointer Function %int
          %2 = OpFunction %void None %8
         %18 = OpLabel
         %19 = OpVariable %_ptr_Function__struct_3 Function
         %20 = OpAccessChain %_ptr_StorageBuffer_int %4 %int_0 %int_0
               OpBranch %21
         %21 = OpLabel
         %22 = OpPhi %_ptr_StorageBuffer_int %20 %18 %23 %24
               OpLoopMerge %25 %24 None
               OpBranchConditional %true %26 %25
         %26 = OpLabel
               OpStore %22 %int_0
               OpBranch %24
         %24 = OpLabel
         %23 = OpPtrAccessChain %_ptr_StorageBuffer_int %22 %int_1
               OpBranch %21
         %25 = OpLabel
         %27 = OpAccessChain %_ptr_Function_int %19 %int_0 %int_0
         %28 = OpLoad %int %27
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(test, true);
}

TEST_F(LocalAccessChainConvertTest, VariablePointers) {
  // A case with variable pointer capability.  We should not convert
  // the access chain on the function scope symbol because the variable pointer
  // could the analysis to miss references to function scope symbols.
  const std::string test =
      R"(OpCapability Shader
OpCapability VariablePointers
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %2 "main"
OpExecutionMode %2 LocalSize 1 1 1
OpSource GLSL 450
OpMemberDecorate %_struct_3 0 Offset 0
OpDecorate %_struct_3 Block
OpDecorate %4 DescriptorSet 0
OpDecorate %4 Binding 0
OpDecorate %_ptr_StorageBuffer_int ArrayStride 4
OpDecorate %_arr_int_int_128 ArrayStride 4
%void = OpTypeVoid
%8 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_128 = OpConstant %int 128
%_arr_int_int_128 = OpTypeArray %int %int_128
%_struct_3 = OpTypeStruct %_arr_int_int_128
%_ptr_StorageBuffer__struct_3 = OpTypePointer StorageBuffer %_struct_3
%_ptr_Function__struct_3 = OpTypePointer Function %_struct_3
%4 = OpVariable %_ptr_StorageBuffer__struct_3 StorageBuffer
%bool = OpTypeBool
%true = OpConstantTrue %bool
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
%_ptr_Function_int = OpTypePointer Function %int
%2 = OpFunction %void None %8
%18 = OpLabel
%19 = OpVariable %_ptr_Function__struct_3 Function
%20 = OpAccessChain %_ptr_StorageBuffer_int %4 %int_0 %int_0
OpBranch %21
%21 = OpLabel
%22 = OpPhi %_ptr_StorageBuffer_int %20 %18 %23 %24
OpLoopMerge %25 %24 None
OpBranchConditional %true %26 %25
%26 = OpLabel
OpStore %22 %int_0
OpBranch %24
%24 = OpLabel
%23 = OpPtrAccessChain %_ptr_StorageBuffer_int %22 %int_1
OpBranch %21
%25 = OpLabel
%27 = OpAccessChain %_ptr_Function_int %19 %int_0 %int_0
%28 = OpLoad %int %27
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalAccessChainConvertPass>(test, test, false, true);
}

TEST_F(LocalAccessChainConvertTest, IdOverflowReplacingLoad) {
  const std::string text =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "PSMain"
               OpExecutionMode %4 OriginUpperLeft
               OpDecorate %10 Location 47360
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
  %_struct_8 = OpTypeStruct %v4float
%_ptr_Function__struct_8 = OpTypePointer Function %_struct_8
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
          %4 = OpFunction %void None %3
          %5 = OpLabel
         %10 = OpVariable %_ptr_Function__struct_8 Function
    %4194301 = OpAccessChain %_ptr_Function_v4float %10 %int_0
    %4194302 = OpLoad %v4float %4194301
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<LocalAccessChainConvertPass>(text, true);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

TEST_F(LocalAccessChainConvertTest, IdOverflowReplacingStore1) {
  const std::string text =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "PSMain"
               OpExecutionMode %4 OriginUpperLeft
               OpDecorate %10 Location 47360
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
  %_struct_7 = OpTypeStruct %v4float
%_ptr_Function__struct_7 = OpTypePointer Function %_struct_7
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %13 = OpConstantNull %v4float
          %4 = OpFunction %void None %3
          %5 = OpLabel
         %10 = OpVariable %_ptr_Function__struct_7 Function
    %4194302 = OpAccessChain %_ptr_Function_v4float %10 %int_0
               OpStore %4194302 %13
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<LocalAccessChainConvertPass>(text, true);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

TEST_F(LocalAccessChainConvertTest, IdOverflowReplacingStore2) {
  const std::string text =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "PSMain"
               OpExecutionMode %4 OriginUpperLeft
               OpDecorate %10 Location 47360
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
  %_struct_7 = OpTypeStruct %v4float
%_ptr_Function__struct_7 = OpTypePointer Function %_struct_7
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %13 = OpConstantNull %v4float
          %4 = OpFunction %void None %3
          %5 = OpLabel
         %10 = OpVariable %_ptr_Function__struct_7 Function
    %4194301 = OpAccessChain %_ptr_Function_v4float %10 %int_0
               OpStore %4194301 %13
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<LocalAccessChainConvertPass>(text, true);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

TEST_F(LocalAccessChainConvertTest, AccessChainWithNoIndex) {
  const std::string before =
      R"(
; CHECK: OpFunction
; CHECK: [[var:%\w+]] = OpVariable
; CHECK: OpStore [[var]] %true
; CHECK: OpLoad %bool [[var]]
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
%_ptr_Function_bool = OpTypePointer Function %bool
          %2 = OpFunction %void None %4
          %8 = OpLabel
          %9 = OpVariable %_ptr_Function_bool Function
         %10 = OpAccessChain %_ptr_Function_bool %9
               OpStore %10 %true
         %11 = OpLoad %bool %10
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(before, true);
}
TEST_F(LocalAccessChainConvertTest, AccessChainWithLongIndex) {
  // The access chain take a value that is larger than 32-bit.  The index cannot
  // be encoded in an OpCompositeExtract, so nothing should be done.
  const std::string before =
      R"(OpCapability Shader
OpCapability Int64
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main_0004f4d4_85b2f584"
OpExecutionMode %2 OriginUpperLeft
%ulong = OpTypeInt 64 0
%ulong_8589934592 = OpConstant %ulong 8589934592
%ulong_8589934591 = OpConstant %ulong 8589934591
%_arr_ulong_ulong_8589934592 = OpTypeArray %ulong %ulong_8589934592
%_ptr_Function__arr_ulong_ulong_8589934592 = OpTypePointer Function %_arr_ulong_ulong_8589934592
%_ptr_Function_ulong = OpTypePointer Function %ulong
%void = OpTypeVoid
%10 = OpTypeFunction %void
%2 = OpFunction %void None %10
%11 = OpLabel
%12 = OpVariable %_ptr_Function__arr_ulong_ulong_8589934592 Function
%13 = OpAccessChain %_ptr_Function_ulong %12 %ulong_8589934591
%14 = OpLoad %ulong %13
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalAccessChainConvertPass>(before, before, false,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, AccessChainWith32BitIndexInLong) {
  // The access chain has a value that is 32-bits, but it is stored in a 64-bit
  // variable.  This access change can be converted to an extract.
  const std::string before =
      R"(
; CHECK: OpFunction
; CHECK: [[var:%\w+]] = OpVariable
; CHECK: [[ld:%\w+]] = OpLoad {{%\w+}} [[var]]
; CHECK: OpCompositeExtract %ulong [[ld]] 3
               OpCapability Shader
               OpCapability Int64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main_0004f4d4_85b2f584"
               OpExecutionMode %2 OriginUpperLeft
      %ulong = OpTypeInt 64 0
%ulong_8589934592 = OpConstant %ulong 8589934592
%ulong_3 = OpConstant %ulong 3
%_arr_ulong_ulong_8589934592 = OpTypeArray %ulong %ulong_8589934592
%_ptr_Function__arr_ulong_ulong_8589934592 = OpTypePointer Function %_arr_ulong_ulong_8589934592
%_ptr_Function_ulong = OpTypePointer Function %ulong
       %void = OpTypeVoid
         %10 = OpTypeFunction %void
          %2 = OpFunction %void None %10
         %11 = OpLabel
         %12 = OpVariable %_ptr_Function__arr_ulong_ulong_8589934592 Function
         %13 = OpAccessChain %_ptr_Function_ulong %12 %ulong_3
         %14 = OpLoad %ulong %13
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(before, true);
}

TEST_F(LocalAccessChainConvertTest, AccessChainWithVarIndex) {
  // The access chain has a value that is not constant, so there should not be
  // any changes.
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main_0004f4d4_85b2f584"
OpExecutionMode %2 OriginUpperLeft
%uint = OpTypeInt 32 0
%uint_5 = OpConstant %uint 5
%_arr_uint_uint_5 = OpTypeArray %uint %uint_5
%_ptr_Function__arr_uint_uint_5 = OpTypePointer Function %_arr_uint_uint_5
%_ptr_Function_uint = OpTypePointer Function %uint
%8 = OpUndef %uint
%void = OpTypeVoid
%10 = OpTypeFunction %void
%2 = OpFunction %void None %10
%11 = OpLabel
%12 = OpVariable %_ptr_Function__arr_uint_uint_5 Function
%13 = OpAccessChain %_ptr_Function_uint %12 %8
%14 = OpLoad %uint %13
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalAccessChainConvertPass>(before, before, false,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, OutOfBoundsAccess) {
  // The access chain indexes element 12 in an array of size 10.  Nothing should
  // be done.
  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3
OpExecutionMode %2 OriginUpperLeft
%void = OpTypeVoid
%5 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_10 = OpConstant %int 10
%_arr_int_int_10 = OpTypeArray %int %int_10
%_ptr_Function_int = OpTypePointer Function %int
%int_12 = OpConstant %int 12
%_ptr_Output_int = OpTypePointer Output %int
%3 = OpVariable %_ptr_Output_int Output
%_ptr_Function__arr_int_int_10 = OpTypePointer Function %_arr_int_int_10
%2 = OpFunction %void None %5
%13 = OpLabel
%14 = OpVariable %_ptr_Function__arr_int_int_10 Function
%15 = OpAccessChain %_ptr_Function_int %14 %int_12
%16 = OpLoad %int %15
OpStore %3 %16
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalAccessChainConvertPass>(assembly, assembly, false,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, OutOfBoundsAccessAtBoundary) {
  // The access chain indexes element 10 in an array of size 10.  Nothing should
  // be done.
  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3
OpExecutionMode %2 OriginUpperLeft
%void = OpTypeVoid
%5 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_10 = OpConstant %int 10
%_arr_int_int_10 = OpTypeArray %int %int_10
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Output_int = OpTypePointer Output %int
%3 = OpVariable %_ptr_Output_int Output
%_ptr_Function__arr_int_int_10 = OpTypePointer Function %_arr_int_int_10
%2 = OpFunction %void None %5
%12 = OpLabel
%13 = OpVariable %_ptr_Function__arr_int_int_10 Function
%14 = OpAccessChain %_ptr_Function_int %13 %int_10
%15 = OpLoad %int %14
OpStore %3 %15
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalAccessChainConvertPass>(assembly, assembly, false,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, NegativeIndex) {
  // The access chain has a negative index and should not be converted because
  // the extract instruction cannot hold a negative number.
  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main"
OpExecutionMode %2 OriginUpperLeft
%void = OpTypeVoid
%4 = OpTypeFunction %void
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%uint_3808428041 = OpConstant %uint 3808428041
%_arr_int_uint_3808428041 = OpTypeArray %int %uint_3808428041
%_ptr_Function__arr_int_uint_3808428041 = OpTypePointer Function %_arr_int_uint_3808428041
%_ptr_Function_int = OpTypePointer Function %int
%int_n1272971256 = OpConstant %int -1272971256
%2 = OpFunction %void None %4
%12 = OpLabel
%13 = OpVariable %_ptr_Function__arr_int_uint_3808428041 Function
%14 = OpAccessChain %_ptr_Function_int %13 %int_n1272971256
%15 = OpLoad %int %14
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalAccessChainConvertPass>(assembly, assembly, false,
                                                     true);
}

TEST_F(LocalAccessChainConvertTest, VkMemoryModelTest) {
  const std::string text =
      R"(
; CHECK: OpCapability Shader
; CHECK: OpCapability VulkanMemoryModel
; CHECK: OpExtension "SPV_KHR_vulkan_memory_model"
               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpExtension "SPV_KHR_vulkan_memory_model"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %main "main"
               OpName %a "a"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[a:%\w+]] = OpVariable
; Make sure the access chains were removed.
; CHECK: [[ld:%\w+]] = OpLoad {{%\w+}} [[a]]
; CHECK: [[ex:%\w+]] = OpCompositeExtract {{%\w+}} [[ld]] 0
; CHECK: [[ld2:%\w+]] = OpLoad {{%\w+}} [[a]]
; CHECK: [[v:%\w+]] = OpCompositeInsert {{%\w+}} [[ex]] [[ld2]] 0
; CHECK: OpStore [[a]] [[v]]
       %main = OpFunction %void None %3
          %5 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
         %13 = OpAccessChain %_ptr_Function_float %a %uint_0
         %14 = OpLoad %float %13
         %17 = OpAccessChain %_ptr_Function_float %a %uint_0
               OpStore %17 %14
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<LocalAccessChainConvertPass>(text, false);
}

// TODO(greg-lunarg): Add tests to verify handling of these cases:
//
//    Assorted vector and matrix types
//    Assorted struct array types
//    Assorted scalar types
//    Assorted non-target types
//    OpInBoundsAccessChain
//    Others?

}  // namespace
}  // namespace opt
}  // namespace spvtools
