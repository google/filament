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
#include <vector>

#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using AggressiveDCETest = PassTest<::testing::Test>;

TEST_F(AggressiveDCETest, EliminateExtendedInst) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //  in vec4 Dead;
  //
  //  void main()
  //  {
  //      vec4 v = BaseColor;
  //      vec4 dv = sqrt(Dead);
  //      gl_FragColor = v;
  //  }
  const std::string spirv = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
; CHECK: OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpEntryPoint Fragment %main "main" %BaseColor %Dead %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
; CHECK-NOT: OpName %dv "dv"
OpName %dv "dv"
; CHECK-NOT: OpName %Dead "Dead"
OpName %Dead "Dead"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
; CHECK-NOT: %Dead = OpVariable
%Dead = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %9
%15 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
; CHECK-NOT: %dv = OpVariable
%dv = OpVariable %_ptr_Function_v4float Function
%16 = OpLoad %v4float %BaseColor
OpStore %v %16
; CHECK-NOT: OpLoad %v4float %Dead
%17 = OpLoad %v4float %Dead
; CHECK-NOT: OpExtInst %v4float %1 Sqrt
%18 = OpExtInst %v4float %1 Sqrt %17
; CHECK-NOT: OpStore %dv
OpStore %dv %18
%19 = OpLoad %v4float %v
OpStore %gl_FragColor %19
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, NoEliminateFrexp) {
  // Note: SPIR-V hand-edited to utilize Frexp
  //
  // #version 450
  //
  // in vec4 BaseColor;
  // in vec4 Dead;
  // out vec4 Color;
  // out ivec4 iv2;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     vec4 dv = frexp(Dead, iv2);
  //     Color = v;
  // }

  const std::string predefs1 =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %Dead %iv2 %Color
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
)";

  const std::string names_before =
      R"(OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %dv "dv"
OpName %Dead "Dead"
OpName %iv2 "iv2"
OpName %ResType "ResType"
OpName %Color "Color"
)";

  const std::string names_after =
      R"(OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %Dead "Dead"
OpName %iv2 "iv2"
OpName %Color "Color"
)";

  const std::string predefs2_before =
      R"(%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%Dead = OpVariable %_ptr_Input_v4float Input
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Output_v4int = OpTypePointer Output %v4int
%iv2 = OpVariable %_ptr_Output_v4int Output
%ResType = OpTypeStruct %v4float %v4int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%Color = OpVariable %_ptr_Output_v4float Output
)";

  const std::string predefs2_after =
      R"(%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%Dead = OpVariable %_ptr_Input_v4float Input
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Output_v4int = OpTypePointer Output %v4int
%iv2 = OpVariable %_ptr_Output_v4int Output
%_ptr_Output_v4float = OpTypePointer Output %v4float
%Color = OpVariable %_ptr_Output_v4float Output
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %11
%20 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%dv = OpVariable %_ptr_Function_v4float Function
%21 = OpLoad %v4float %BaseColor
OpStore %v %21
%22 = OpLoad %v4float %Dead
%23 = OpExtInst %v4float %1 Frexp %22 %iv2
OpStore %dv %23
%24 = OpLoad %v4float %v
OpStore %Color %24
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %11
%20 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%21 = OpLoad %v4float %BaseColor
OpStore %v %21
%22 = OpLoad %v4float %Dead
%23 = OpExtInst %v4float %1 Frexp %22 %iv2
%24 = OpLoad %v4float %v
OpStore %Color %24
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(
      predefs1 + names_before + predefs2_before + func_before,
      predefs1 + names_after + predefs2_after + func_after, true, true);
}

TEST_F(AggressiveDCETest, EliminateDecorate) {
  // Note: The SPIR-V was hand-edited to add the OpDecorate
  //
  // #version 140
  //
  // in vec4 BaseColor;
  // in vec4 Dead;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     vec4 dv = Dead * 0.5;
  //     gl_FragColor = v;
  // }

  const std::string spirv =
      R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %Dead %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %dv "dv"
OpName %Dead "Dead"
OpName %gl_FragColor "gl_FragColor"
; CHECK-NOT: OpDecorate
OpDecorate %8 RelaxedPrecision
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%Dead = OpVariable %_ptr_Input_v4float Input
%float_0_5 = OpConstant %float 0.5
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %10
%17 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%dv = OpVariable %_ptr_Function_v4float Function
%18 = OpLoad %v4float %BaseColor
OpStore %v %18
%19 = OpLoad %v4float %Dead
; CHECK-NOT: OpVectorTimesScalar
%8 = OpVectorTimesScalar %v4float %19 %float_0_5
OpStore %dv %8
%20 = OpLoad %v4float %v
OpStore %gl_FragColor %20
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, Simple) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //  in vec4 Dead;
  //
  //  void main()
  //  {
  //      vec4 v = BaseColor;
  //      vec4 dv = Dead;
  //      gl_FragColor = v;
  //  }

  const std::string spirv =
      R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
; CHECK: OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpEntryPoint Fragment %main "main" %BaseColor %Dead %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
; CHECK-NOT: OpName %dv "dv"
OpName %dv "dv"
; CHECK-NOT: OpName %Dead "Dead"
OpName %Dead "Dead"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
; CHECK-NOT: %Dead = OpVariable
%Dead = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %9
%15 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
; CHECK-NOT: %dv = OpVariable
%dv = OpVariable %_ptr_Function_v4float Function
%16 = OpLoad %v4float %BaseColor
OpStore %v %16
; CHECK-NOT: OpLoad %v4float %Dead
%17 = OpLoad %v4float %Dead
; CHECK-NOT: OpStore %dv
OpStore %dv %17
%18 = OpLoad %v4float %v
OpStore %gl_FragColor %18
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, OptAllowListExtension) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //  in vec4 Dead;
  //
  //  void main()
  //  {
  //      vec4 v = BaseColor;
  //      vec4 dv = Dead;
  //      gl_FragColor = v;
  //  }

  const std::string spirv =
      R"(OpCapability Shader
OpExtension "SPV_AMD_gpu_shader_int16"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
; CHECK: OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpEntryPoint Fragment %main "main" %BaseColor %Dead %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %dv "dv"
OpName %Dead "Dead"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%Dead = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %9
%15 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%dv = OpVariable %_ptr_Function_v4float Function
%16 = OpLoad %v4float %BaseColor
OpStore %v %16
%17 = OpLoad %v4float %Dead
OpStore %dv %17
%18 = OpLoad %v4float %v
OpStore %gl_FragColor %18
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, NoOptDenyListExtension) {
  //  #version 140
  //
  //  in vec4 BaseColor;
  //  in vec4 Dead;
  //
  //  void main()
  //  {
  //      vec4 v = BaseColor;
  //      vec4 dv = Dead;
  //      gl_FragColor = v;
  //  }

  const std::string assembly =
      R"(OpCapability Shader
OpExtension "SPV_KHR_variable_pointers"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %Dead %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %dv "dv"
OpName %Dead "Dead"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%Dead = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %9
%15 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%dv = OpVariable %_ptr_Function_v4float Function
%16 = OpLoad %v4float %BaseColor
OpStore %v %16
%17 = OpLoad %v4float %Dead
OpStore %dv %17
%18 = OpLoad %v4float %v
OpStore %gl_FragColor %18
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, ElimWithCall) {
  // This demonstrates that "dead" function calls are not eliminated.
  // Also demonstrates that DCE will happen in presence of function call.
  // #version 140
  // in vec4 i1;
  // in vec4 i2;
  //
  // void nothing(vec4 v)
  // {
  // }
  //
  // void main()
  // {
  //     vec4 v1 = i1;
  //     vec4 v2 = i2;
  //     nothing(v1);
  //     gl_FragColor = vec4(0.0);
  // }

  const std::string text =
      R"( OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %i1 %i2 %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %nothing_vf4_ "nothing(vf4;"
OpName %v "v"
OpName %v1 "v1"
OpName %i1 "i1"
OpName %v2 "v2"
OpName %i2 "i2"
OpName %param "param"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%12 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%16 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%i1 = OpVariable %_ptr_Input_v4float Input
%i2 = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%main = OpFunction %void None %12
%21 = OpLabel
%v1 = OpVariable %_ptr_Function_v4float Function
%v2 = OpVariable %_ptr_Function_v4float Function
%param = OpVariable %_ptr_Function_v4float Function
%22 = OpLoad %v4float %i1
OpStore %v1 %22
; CHECK-NOT: OpLoad %v4float %i2
%23 = OpLoad %v4float %i2
; CHECK-NOT: OpStore %v2
OpStore %v2 %23
%24 = OpLoad %v4float %v1
OpStore %param %24
; CHECK: OpFunctionCall %void %nothing_vf4_
%25 = OpFunctionCall %void %nothing_vf4_ %param
OpStore %gl_FragColor %20
OpReturn
OpFunctionEnd
; CHECK: %nothing_vf4_ = OpFunction
%nothing_vf4_ = OpFunction %void None %16
%v = OpFunctionParameter %_ptr_Function_v4float
%26 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, NoParamElim) {
  // This demonstrates that unused parameters are not eliminated, but
  // dead uses of them are.
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // vec4 foo(vec4 v1, vec4 v2)
  // {
  //     vec4 t = -v1;
  //     return v2;
  // }
  //
  // void main()
  // {
  //     vec4 dead;
  //     gl_FragColor = foo(dead, BaseColor);
  // }

  const std::string defs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %gl_FragColor %BaseColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %foo_vf4_vf4_ "foo(vf4;vf4;"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %t "t"
OpName %gl_FragColor "gl_FragColor"
OpName %dead "dead"
OpName %BaseColor "BaseColor"
OpName %param "param"
OpName %param_0 "param"
%void = OpTypeVoid
%13 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%17 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %13
%20 = OpLabel
%dead = OpVariable %_ptr_Function_v4float Function
%param = OpVariable %_ptr_Function_v4float Function
%param_0 = OpVariable %_ptr_Function_v4float Function
%21 = OpLoad %v4float %dead
OpStore %param %21
%22 = OpLoad %v4float %BaseColor
OpStore %param_0 %22
%23 = OpFunctionCall %v4float %foo_vf4_vf4_ %param %param_0
OpStore %gl_FragColor %23
OpReturn
OpFunctionEnd
)";

  const std::string defs_after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %gl_FragColor %BaseColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %foo_vf4_vf4_ "foo(vf4;vf4;"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %gl_FragColor "gl_FragColor"
OpName %dead "dead"
OpName %BaseColor "BaseColor"
OpName %param "param"
OpName %param_0 "param"
%void = OpTypeVoid
%13 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%17 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %13
%20 = OpLabel
%dead = OpVariable %_ptr_Function_v4float Function
%param = OpVariable %_ptr_Function_v4float Function
%param_0 = OpVariable %_ptr_Function_v4float Function
%21 = OpLoad %v4float %dead
OpStore %param %21
%22 = OpLoad %v4float %BaseColor
OpStore %param_0 %22
%23 = OpFunctionCall %v4float %foo_vf4_vf4_ %param %param_0
OpStore %gl_FragColor %23
OpReturn
OpFunctionEnd
)";

  const std::string func_before =
      R"(%foo_vf4_vf4_ = OpFunction %v4float None %17
%v1 = OpFunctionParameter %_ptr_Function_v4float
%v2 = OpFunctionParameter %_ptr_Function_v4float
%24 = OpLabel
%t = OpVariable %_ptr_Function_v4float Function
%25 = OpLoad %v4float %v1
%26 = OpFNegate %v4float %25
OpStore %t %26
%27 = OpLoad %v4float %v2
OpReturnValue %27
OpFunctionEnd
)";

  const std::string func_after =
      R"(%foo_vf4_vf4_ = OpFunction %v4float None %17
%v1 = OpFunctionParameter %_ptr_Function_v4float
%v2 = OpFunctionParameter %_ptr_Function_v4float
%24 = OpLabel
%27 = OpLoad %v4float %v2
OpReturnValue %27
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(defs_before + func_before,
                                           defs_after + func_after, true, true);
}

TEST_F(AggressiveDCETest, ElimOpaque) {
  // SPIR-V not representable from GLSL; not generatable from HLSL
  // for the moment.

  const std::string defs_before =
      R"(OpCapability Shader
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
OpName %outColor "outColor"
OpName %sampler15 "sampler15"
OpName %s0 "s0"
OpName %texCoords "texCoords"
OpDecorate %sampler15 DescriptorSet 0
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%outColor = OpVariable %_ptr_Output_v4float Output
%14 = OpTypeImage %float 2D 0 0 0 1 Unknown
%15 = OpTypeSampledImage %14
%S_t = OpTypeStruct %v2float %v2float %15
%_ptr_Function_S_t = OpTypePointer Function %S_t
%17 = OpTypeFunction %void %_ptr_Function_S_t
%_ptr_UniformConstant_15 = OpTypePointer UniformConstant %15
%_ptr_Function_15 = OpTypePointer Function %15
%sampler15 = OpVariable %_ptr_UniformConstant_15 UniformConstant
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Input_v2float = OpTypePointer Input %v2float
%texCoords = OpVariable %_ptr_Input_v2float Input
)";

  const std::string defs_after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %outColor %texCoords
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %outColor "outColor"
OpName %sampler15 "sampler15"
OpName %texCoords "texCoords"
OpDecorate %sampler15 DescriptorSet 0
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%outColor = OpVariable %_ptr_Output_v4float Output
%14 = OpTypeImage %float 2D 0 0 0 1 Unknown
%15 = OpTypeSampledImage %14
%_ptr_UniformConstant_15 = OpTypePointer UniformConstant %15
%sampler15 = OpVariable %_ptr_UniformConstant_15 UniformConstant
%_ptr_Input_v2float = OpTypePointer Input %v2float
%texCoords = OpVariable %_ptr_Input_v2float Input
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %9
%25 = OpLabel
%s0 = OpVariable %_ptr_Function_S_t Function
%26 = OpLoad %v2float %texCoords
%27 = OpLoad %S_t %s0
%28 = OpCompositeInsert %S_t %26 %27 0
%29 = OpLoad %15 %sampler15
%30 = OpCompositeInsert %S_t %29 %28 2
OpStore %s0 %30
%31 = OpImageSampleImplicitLod %v4float %29 %26
OpStore %outColor %31
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %9
%25 = OpLabel
%26 = OpLoad %v2float %texCoords
%29 = OpLoad %15 %sampler15
%31 = OpImageSampleImplicitLod %v4float %29 %26
OpStore %outColor %31
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(defs_before + func_before,
                                           defs_after + func_after, true, true);
}

TEST_F(AggressiveDCETest, NoParamStoreElim) {
  // Should not eliminate stores to params
  //
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // void foo(in vec4 v1, out vec4 v2)
  // {
  //     v2 = -v1;
  // }
  //
  // void main()
  // {
  //     foo(BaseColor, OutColor);
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %foo_vf4_vf4_ "foo(vf4;vf4;"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %BaseColor "BaseColor"
OpName %OutColor "OutColor"
OpName %param "param"
OpName %param_0 "param"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%15 = OpTypeFunction %void %_ptr_Function_v4float %_ptr_Function_v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %11
%18 = OpLabel
%param = OpVariable %_ptr_Function_v4float Function
%param_0 = OpVariable %_ptr_Function_v4float Function
%19 = OpLoad %v4float %BaseColor
OpStore %param %19
%20 = OpFunctionCall %void %foo_vf4_vf4_ %param %param_0
%21 = OpLoad %v4float %param_0
OpStore %OutColor %21
OpReturn
OpFunctionEnd
%foo_vf4_vf4_ = OpFunction %void None %15
%v1 = OpFunctionParameter %_ptr_Function_v4float
%v2 = OpFunctionParameter %_ptr_Function_v4float
%22 = OpLabel
%23 = OpLoad %v4float %v1
%24 = OpFNegate %v4float %23
OpStore %v2 %24
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, PrivateStoreElimInEntryNoCalls) {
  // Eliminate stores to private in entry point with no calls
  // Note: Not legal GLSL
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 1) in vec4 Dead;
  // layout(location = 0) out vec4 OutColor;
  //
  // private vec4 dv;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     dv = Dead;
  //     OutColor = v;
  // }

  const std::string spirv =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %Dead %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
; CHECK-NOT: OpName %dv "dv"
OpName %dv "dv"
OpName %Dead "Dead"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %Dead Location 1
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
; CHECK-NOT: OpTypePointer Private
%_ptr_Private_v4float = OpTypePointer Private %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%Dead = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
; CHECK-NOT: %dv = OpVariable
%dv = OpVariable %_ptr_Private_v4float Private
%OutColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %9
%16 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%17 = OpLoad %v4float %BaseColor
OpStore %v %17
%18 = OpLoad %v4float %Dead
; CHECK-NOT: OpStore %dv
OpStore %dv %18
%19 = OpLoad %v4float %v
%20 = OpFNegate %v4float %19
OpStore %OutColor %20
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, NoPrivateStoreElimIfLoad) {
  // Should not eliminate stores to private when there is a load
  // Note: Not legal GLSL
  //
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // private vec4 pv;
  //
  // void main()
  // {
  //     pv = BaseColor;
  //     OutColor = pv;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %pv "pv"
OpName %BaseColor "BaseColor"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Private_v4float = OpTypePointer Private %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%pv = OpVariable %_ptr_Private_v4float Private
%main = OpFunction %void None %7
%13 = OpLabel
%14 = OpLoad %v4float %BaseColor
OpStore %pv %14
%15 = OpLoad %v4float %pv
%16 = OpFNegate %v4float %15
OpStore %OutColor %16
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, NoPrivateStoreElimWithCall) {
  // Should not eliminate stores to private when function contains call
  // Note: Not legal GLSL
  //
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // private vec4 v1;
  //
  // void foo()
  // {
  //     OutColor = -v1;
  // }
  //
  // void main()
  // {
  //     v1 = BaseColor;
  //     foo();
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %OutColor %BaseColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %foo_ "foo("
OpName %OutColor "OutColor"
OpName %v1 "v1"
OpName %BaseColor "BaseColor"
OpDecorate %OutColor Location 0
OpDecorate %BaseColor Location 0
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Private_v4float = OpTypePointer Private %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%v1 = OpVariable %_ptr_Private_v4float Private
%BaseColor = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %8
%14 = OpLabel
%15 = OpLoad %v4float %BaseColor
OpStore %v1 %15
%16 = OpFunctionCall %void %foo_
OpReturn
OpFunctionEnd
%foo_ = OpFunction %void None %8
%17 = OpLabel
%18 = OpLoad %v4float %v1
%19 = OpFNegate %v4float %18
OpStore %OutColor %19
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, NoPrivateStoreElimInNonEntry) {
  // Should not eliminate stores to private when function is not entry point
  // Note: Not legal GLSL
  //
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // private vec4 v1;
  //
  // void foo()
  // {
  //     v1 = BaseColor;
  // }
  //
  // void main()
  // {
  //     foo();
  //     OutColor = -v1;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %foo_ "foo("
OpName %v1 "v1"
OpName %BaseColor "BaseColor"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Private_v4float = OpTypePointer Private %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v1 = OpVariable %_ptr_Private_v4float Private
%OutColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %8
%14 = OpLabel
%15 = OpFunctionCall %void %foo_
%16 = OpLoad %v4float %v1
%17 = OpFNegate %v4float %16
OpStore %OutColor %17
OpReturn
OpFunctionEnd
%foo_ = OpFunction %void None %8
%18 = OpLabel
%19 = OpLoad %v4float %BaseColor
OpStore %v1 %19
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, WorkgroupStoreElimInEntryNoCalls) {
  // Eliminate stores to private in entry point with no calls
  // Note: Not legal GLSL
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 1) in vec4 Dead;
  // layout(location = 0) out vec4 OutColor;
  //
  // workgroup vec4 dv;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     dv = Dead;
  //     OutColor = v;
  // }

  const std::string spirv =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %Dead %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
; CHECK-NOT: OpName %dv "dv"
OpName %dv "dv"
OpName %Dead "Dead"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %Dead Location 1
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%9 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
; CHECK-NOT: OpTypePointer Workgroup
%_ptr_Workgroup_v4float = OpTypePointer Workgroup %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%Dead = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
; CHECK-NOT: %dv = OpVariable
%dv = OpVariable %_ptr_Workgroup_v4float Workgroup
%OutColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %9
%16 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%17 = OpLoad %v4float %BaseColor
OpStore %v %17
%18 = OpLoad %v4float %Dead
; CHECK-NOT: OpStore %dv
OpStore %dv %18
%19 = OpLoad %v4float %v
%20 = OpFNegate %v4float %19
OpStore %OutColor %20
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, EliminateDeadIfThenElse) {
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // void main()
  // {
  //     float d;
  //     if (BaseColor.x == 0)
  //       d = BaseColor.y;
  //     else
  //       d = BaseColor.z;
  //     OutColor = vec4(1.0,1.0,1.0,1.0);
  // }

  const std::string spirv =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %d "d"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%float_1 = OpConstant %float 1
%21 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
; CHECK: = OpFunction %void
; CHECK-NEXT: %22 = OpLabel
; CHECK-NEXT: OpBranch %26
; CHECK-NEXT: %26 = OpLabel
%main = OpFunction %void None %7
%22 = OpLabel
%d = OpVariable %_ptr_Function_float Function
%23 = OpAccessChain %_ptr_Input_float %BaseColor %uint_0
%24 = OpLoad %float %23
%25 = OpFOrdEqual %bool %24 %float_0
OpSelectionMerge %26 None
OpBranchConditional %25 %27 %28
%27 = OpLabel
%29 = OpAccessChain %_ptr_Input_float %BaseColor %uint_1
%30 = OpLoad %float %29
OpStore %d %30
OpBranch %26
%28 = OpLabel
%31 = OpAccessChain %_ptr_Input_float %BaseColor %uint_2
%32 = OpLoad %float %31
OpStore %d %32
OpBranch %26
%26 = OpLabel
OpStore %OutColor %21
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, EliminateDeadIfThen) {
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // void main()
  // {
  //     float d;
  //     if (BaseColor.x == 0)
  //       d = BaseColor.y;
  //     OutColor = vec4(1.0,1.0,1.0,1.0);
  // }

  const std::string spirv =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %d "d"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
%uint_1 = OpConstant %uint 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%float_1 = OpConstant %float 1
%20 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
; CHECK: = OpFunction
; CHECK-NEXT: %21 = OpLabel
; CHECK-NEXT: OpBranch [[target:%\w+]]
; CHECK-NEXT: [[target]] = OpLabel
%main = OpFunction %void None %7
%21 = OpLabel
%d = OpVariable %_ptr_Function_float Function
%22 = OpAccessChain %_ptr_Input_float %BaseColor %uint_0
%23 = OpLoad %float %22
%24 = OpFOrdEqual %bool %23 %float_0
OpSelectionMerge %25 None
OpBranchConditional %24 %26 %25
%26 = OpLabel
%27 = OpAccessChain %_ptr_Input_float %BaseColor %uint_1
%28 = OpLoad %float %27
OpStore %d %28
OpBranch %25
%25 = OpLabel
OpStore %OutColor %20
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, EliminateDeadSwitch) {
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 1) in flat int x;
  // layout(location = 0) out vec4 OutColor;
  //
  // void main()
  // {
  //     float d;
  //     switch (x) {
  //       case 0:
  //         d = BaseColor.y;
  //     }
  //     OutColor = vec4(1.0,1.0,1.0,1.0);
  // }
  const std::string spirv =
      R"(OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %x %BaseColor %OutColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %x "x"
               OpName %d "d"
               OpName %BaseColor "BaseColor"
               OpName %OutColor "OutColor"
               OpDecorate %x Flat
               OpDecorate %x Location 1
               OpDecorate %BaseColor Location 0
               OpDecorate %OutColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
          %x = OpVariable %_ptr_Input_int Input
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
  %BaseColor = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
   %OutColor = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
         %27 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
; CHECK: = OpFunction
; CHECK-NEXT: = OpLabel
; CHECK-NEXT: OpBranch [[target:%\w+]]
; CHECK-NEXT: [[target]] = OpLabel
       %main = OpFunction %void None %3
          %5 = OpLabel
          %d = OpVariable %_ptr_Function_float Function
          %9 = OpLoad %int %x
               OpSelectionMerge %11 None
               OpSwitch %9 %11 0 %10
         %10 = OpLabel
         %21 = OpAccessChain %_ptr_Input_float %BaseColor %uint_1
         %22 = OpLoad %float %21
               OpStore %d %22
               OpBranch %11
         %11 = OpLabel
               OpStore %OutColor %27
               OpReturn
               OpFunctionEnd)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, EliminateDeadIfThenElseNested) {
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // void main()
  // {
  //     float d;
  //     if (BaseColor.x == 0)
  //       if (BaseColor.y == 0)
  //         d = 0.0;
  //       else
  //         d = 0.25;
  //     else
  //       if (BaseColor.y == 0)
  //         d = 0.5;
  //       else
  //         d = 0.75;
  //     OutColor = vec4(1.0,1.0,1.0,1.0);
  // }

  const std::string spirv =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %d "d"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%uint_1 = OpConstant %uint 1
%_ptr_Function_float = OpTypePointer Function %float
%float_0_25 = OpConstant %float 0.25
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%float_1 = OpConstant %float 1
%23 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1

; CHECK: = OpFunction
; CHECK-NEXT: = OpLabel
; CHECK-NEXT: OpBranch [[target:%\w+]]
; CHECK-NEXT: [[target]] = OpLabel
; CHECK-NOT: OpLabel

%main = OpFunction %void None %7
%24 = OpLabel
%d = OpVariable %_ptr_Function_float Function
%25 = OpAccessChain %_ptr_Input_float %BaseColor %uint_0
%26 = OpLoad %float %25
%27 = OpFOrdEqual %bool %26 %float_0
OpSelectionMerge %28 None
OpBranchConditional %27 %29 %30
%29 = OpLabel
%31 = OpAccessChain %_ptr_Input_float %BaseColor %uint_1
%32 = OpLoad %float %31
%33 = OpFOrdEqual %bool %32 %float_0
OpSelectionMerge %34 None
OpBranchConditional %33 %35 %36
%35 = OpLabel
OpStore %d %float_0
OpBranch %34
%36 = OpLabel
OpStore %d %float_0_25
OpBranch %34
%34 = OpLabel
OpBranch %28
%30 = OpLabel
%37 = OpAccessChain %_ptr_Input_float %BaseColor %uint_1
%38 = OpLoad %float %37
%39 = OpFOrdEqual %bool %38 %float_0
OpSelectionMerge %40 None
OpBranchConditional %39 %41 %42
%41 = OpLabel
OpStore %d %float_0_5
OpBranch %40
%42 = OpLabel
OpStore %d %float_0_75
OpBranch %40
%40 = OpLabel
OpBranch %28
%28 = OpLabel
OpStore %OutColor %23
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, NoEliminateLiveIfThenElse) {
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // void main()
  // {
  //     float t;
  //     if (BaseColor.x == 0)
  //       t = BaseColor.y;
  //     else
  //       t = BaseColor.z;
  //     OutColor = vec4(t);
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %t "t"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %7
%20 = OpLabel
%t = OpVariable %_ptr_Function_float Function
%21 = OpAccessChain %_ptr_Input_float %BaseColor %uint_0
%22 = OpLoad %float %21
%23 = OpFOrdEqual %bool %22 %float_0
OpSelectionMerge %24 None
OpBranchConditional %23 %25 %26
%25 = OpLabel
%27 = OpAccessChain %_ptr_Input_float %BaseColor %uint_1
%28 = OpLoad %float %27
OpStore %t %28
OpBranch %24
%26 = OpLabel
%29 = OpAccessChain %_ptr_Input_float %BaseColor %uint_2
%30 = OpLoad %float %29
OpStore %t %30
OpBranch %24
%24 = OpLabel
%31 = OpLoad %float %t
%32 = OpCompositeConstruct %v4float %31 %31 %31 %31
OpStore %OutColor %32
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, NoEliminateLiveIfThenElseNested) {
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // void main()
  // {
  //     float t;
  //     if (BaseColor.x == 0)
  //       if (BaseColor.y == 0)
  //         t = 0.0;
  //       else
  //         t = 0.25;
  //     else
  //       if (BaseColor.y == 0)
  //         t = 0.5;
  //       else
  //         t = 0.75;
  //     OutColor = vec4(t);
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %t "t"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%uint_1 = OpConstant %uint 1
%_ptr_Function_float = OpTypePointer Function %float
%float_0_25 = OpConstant %float 0.25
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %7
%22 = OpLabel
%t = OpVariable %_ptr_Function_float Function
%23 = OpAccessChain %_ptr_Input_float %BaseColor %uint_0
%24 = OpLoad %float %23
%25 = OpFOrdEqual %bool %24 %float_0
OpSelectionMerge %26 None
OpBranchConditional %25 %27 %28
%27 = OpLabel
%29 = OpAccessChain %_ptr_Input_float %BaseColor %uint_1
%30 = OpLoad %float %29
%31 = OpFOrdEqual %bool %30 %float_0
OpSelectionMerge %32 None
OpBranchConditional %31 %33 %34
%33 = OpLabel
OpStore %t %float_0
OpBranch %32
%34 = OpLabel
OpStore %t %float_0_25
OpBranch %32
%32 = OpLabel
OpBranch %26
%28 = OpLabel
%35 = OpAccessChain %_ptr_Input_float %BaseColor %uint_1
%36 = OpLoad %float %35
%37 = OpFOrdEqual %bool %36 %float_0
OpSelectionMerge %38 None
OpBranchConditional %37 %39 %40
%39 = OpLabel
OpStore %t %float_0_5
OpBranch %38
%40 = OpLabel
OpStore %t %float_0_75
OpBranch %38
%38 = OpLabel
OpBranch %26
%26 = OpLabel
%41 = OpLoad %float %t
%42 = OpCompositeConstruct %v4float %41 %41 %41 %41
OpStore %OutColor %42
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, NoEliminateIfWithPhi) {
  // Note: Assembly hand-optimized from GLSL
  //
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // void main()
  // {
  //     float t;
  //     if (BaseColor.x == 0)
  //       t = 0.0;
  //     else
  //       t = 1.0;
  //     OutColor = vec4(t);
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%6 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%float_1 = OpConstant %float 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %6
%17 = OpLabel
%18 = OpAccessChain %_ptr_Input_float %BaseColor %uint_0
%19 = OpLoad %float %18
%20 = OpFOrdEqual %bool %19 %float_0
OpSelectionMerge %21 None
OpBranchConditional %20 %22 %23
%22 = OpLabel
OpBranch %21
%23 = OpLabel
OpBranch %21
%21 = OpLabel
%24 = OpPhi %float %float_0 %22 %float_1 %23
%25 = OpCompositeConstruct %v4float %24 %24 %24 %24
OpStore %OutColor %25
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, NoEliminateIfBreak) {
  // Note: Assembly optimized from GLSL
  //
  // #version 450
  //
  // layout(location=0) in vec4 InColor;
  // layout(location=0) out vec4 OutColor;
  //
  // void main()
  // {
  //     float f = 0.0;
  //     for (;;) {
  //         f += 2.0;
  //         if (f > 20.0)
  //             break;
  //     }
  //
  //     OutColor = InColor / f;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %OutColor %InColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %f "f"
OpName %OutColor "OutColor"
OpName %InColor "InColor"
OpDecorate %OutColor Location 0
OpDecorate %InColor Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%float_2 = OpConstant %float 2
%float_20 = OpConstant %float 20
%bool = OpTypeBool
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%InColor = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %7
%17 = OpLabel
%f = OpVariable %_ptr_Function_float Function
OpStore %f %float_0
OpBranch %18
%18 = OpLabel
OpLoopMerge %19 %20 None
OpBranch %21
%21 = OpLabel
%22 = OpLoad %float %f
%23 = OpFAdd %float %22 %float_2
OpStore %f %23
%24 = OpLoad %float %f
%25 = OpFOrdGreaterThan %bool %24 %float_20
OpSelectionMerge %26 None
OpBranchConditional %25 %27 %26
%27 = OpLabel
OpBranch %19
%26 = OpLabel
OpBranch %20
%20 = OpLabel
OpBranch %18
%19 = OpLabel
%28 = OpLoad %v4float %InColor
%29 = OpLoad %float %f
%30 = OpCompositeConstruct %v4float %29 %29 %29 %29
%31 = OpFDiv %v4float %28 %30
OpStore %OutColor %31
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, NoEliminateIfBreak2) {
  // Do not eliminate break as conditional branch with merge instruction
  // Note: SPIR-V edited to add merge instruction before break.
  //
  // #version 430
  //
  // layout(std430) buffer U_t
  // {
  //     float g_F[10];
  // };
  //
  // layout(location = 0)out float o;
  //
  // void main(void)
  // {
  //     float s = 0.0;
  //     for (int i=0; i<10; i++)
  //         s += g_F[i];
  //     o = s;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %s "s"
OpName %i "i"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%U_t = OpTypeStruct %_arr_float_uint_10
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %10
%25 = OpLabel
%s = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %s %float_0
OpStore %i %int_0
OpBranch %26
%26 = OpLabel
OpLoopMerge %27 %28 None
OpBranch %29
%29 = OpLabel
%30 = OpLoad %int %i
%31 = OpSLessThan %bool %30 %int_10
OpSelectionMerge %32 None
OpBranchConditional %31 %32 %27
%32 = OpLabel
%33 = OpLoad %int %i
%34 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %33
%35 = OpLoad %float %34
%36 = OpLoad %float %s
%37 = OpFAdd %float %36 %35
OpStore %s %37
OpBranch %28
%28 = OpLabel
%38 = OpLoad %int %i
%39 = OpIAdd %int %38 %int_1
OpStore %i %39
OpBranch %26
%27 = OpLabel
%40 = OpLoad %float %s
OpStore %o %40
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, EliminateEntireUselessLoop) {
  // #version 140
  // in vec4 BaseColor;
  //
  // layout(std140) uniform U_t
  // {
  //     int g_I ;
  // } ;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     float df = 0.0;
  //     int i = 0;
  //     while (i < g_I) {
  //       df = df * 0.5;
  //       i = i + 1;
  //     }
  //     gl_FragColor = v;
  // }

  const std::string predefs1 =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
)";

  const std::string names_before =
      R"(OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %df "df"
OpName %i "i"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_I"
OpName %_ ""
OpName %gl_FragColor "gl_FragColor"
)";

  const std::string names_after =
      R"(OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
)";

  const std::string predefs2_before =
      R"(OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t Block
OpDecorate %_ DescriptorSet 0
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%U_t = OpTypeStruct %int
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_int = OpTypePointer Uniform %int
%bool = OpTypeBool
%float_0_5 = OpConstant %float 0.5
%int_1 = OpConstant %int 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string predefs2_after =
      R"(%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %11
%27 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%df = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%28 = OpLoad %v4float %BaseColor
OpStore %v %28
OpStore %df %float_0
OpStore %i %int_0
OpBranch %29
%29 = OpLabel
OpLoopMerge %30 %31 None
OpBranch %32
%32 = OpLabel
%33 = OpLoad %int %i
%34 = OpAccessChain %_ptr_Uniform_int %_ %int_0
%35 = OpLoad %int %34
%36 = OpSLessThan %bool %33 %35
OpBranchConditional %36 %37 %30
%37 = OpLabel
%38 = OpLoad %float %df
%39 = OpFMul %float %38 %float_0_5
OpStore %df %39
%40 = OpLoad %int %i
%41 = OpIAdd %int %40 %int_1
OpStore %i %41
OpBranch %31
%31 = OpLabel
OpBranch %29
%30 = OpLabel
%42 = OpLoad %v4float %v
OpStore %gl_FragColor %42
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %11
%27 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%28 = OpLoad %v4float %BaseColor
OpStore %v %28
OpBranch %29
%29 = OpLabel
OpBranch %30
%30 = OpLabel
%42 = OpLoad %v4float %v
OpStore %gl_FragColor %42
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(
      predefs1 + names_before + predefs2_before + func_before,
      predefs1 + names_after + predefs2_after + func_after, true, true);
}

TEST_F(AggressiveDCETest, NoEliminateBusyLoop) {
  // Note: SPIR-V edited to replace AtomicAdd(i,0) with AtomicLoad(i)
  //
  // #version 450
  //
  // layout(std430) buffer I_t
  // {
  // 	int g_I;
  // 	int g_I2;
  // };
  //
  // layout(location = 0) out int o;
  //
  // void main(void)
  // {
  // 	while (atomicAdd(g_I, 0) == 0) {}
  // 	o = g_I2;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %I_t "I_t"
OpMemberName %I_t 0 "g_I"
OpMemberName %I_t 1 "g_I2"
OpName %_ ""
OpName %o "o"
OpMemberDecorate %I_t 0 Offset 0
OpMemberDecorate %I_t 1 Offset 4
OpDecorate %I_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%int = OpTypeInt 32 1
%I_t = OpTypeStruct %int %int
%_ptr_Uniform_I_t = OpTypePointer Uniform %I_t
%_ = OpVariable %_ptr_Uniform_I_t Uniform
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_ptr_Uniform_int = OpTypePointer Uniform %int
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%_ptr_Output_int = OpTypePointer Output %int
%o = OpVariable %_ptr_Output_int Output
%main = OpFunction %void None %7
%18 = OpLabel
OpBranch %19
%19 = OpLabel
OpLoopMerge %20 %21 None
OpBranch %22
%22 = OpLabel
%23 = OpAccessChain %_ptr_Uniform_int %_ %int_0
%24 = OpAtomicLoad %int %23 %uint_1 %uint_0
%25 = OpIEqual %bool %24 %int_0
OpBranchConditional %25 %26 %20
%26 = OpLabel
OpBranch %21
%21 = OpLabel
OpBranch %19
%20 = OpLabel
%27 = OpAccessChain %_ptr_Uniform_int %_ %int_1
%28 = OpLoad %int %27
OpStore %o %28
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, NoEliminateLiveLoop) {
  // Note: SPIR-V optimized
  //
  // #version 430
  //
  // layout(std430) buffer U_t
  // {
  //     float g_F[10];
  // };
  //
  // layout(location = 0)out float o;
  //
  // void main(void)
  // {
  //     float s = 0.0;
  //     for (int i=0; i<10; i++)
  //         s += g_F[i];
  //     o = s;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%U_t = OpTypeStruct %_arr_float_uint_10
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %8
%21 = OpLabel
OpBranch %22
%22 = OpLabel
%23 = OpPhi %float %float_0 %21 %24 %25
%26 = OpPhi %int %int_0 %21 %27 %25
OpLoopMerge %28 %25 None
OpBranch %29
%29 = OpLabel
%30 = OpSLessThan %bool %26 %int_10
OpBranchConditional %30 %31 %28
%31 = OpLabel
%32 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %26
%33 = OpLoad %float %32
%24 = OpFAdd %float %23 %33
OpBranch %25
%25 = OpLabel
%27 = OpIAdd %int %26 %int_1
OpBranch %22
%28 = OpLabel
OpStore %o %23
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, EliminateEntireFunctionBody) {
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // void main()
  // {
  //     float d;
  //     if (BaseColor.x == 0)
  //       d = BaseColor.y;
  //     else
  //       d = BaseColor.z;
  // }

  const std::string spirv =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %d "d"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output

; CHECK: = OpFunction
; CHECK-NEXT: = OpLabel
; CHECK-NEXT: OpBranch [[target:%\w+]]
; CHECK-NEXT: [[target]] = OpLabel
; CHECK-NEXT: OpReturn
; CHECK-NEXT: OpFunctionEnd

%main = OpFunction %void None %7
%20 = OpLabel
%d = OpVariable %_ptr_Function_float Function
%21 = OpAccessChain %_ptr_Input_float %BaseColor %uint_0
%22 = OpLoad %float %21
%23 = OpFOrdEqual %bool %22 %float_0
OpSelectionMerge %24 None
OpBranchConditional %23 %25 %26
%25 = OpLabel
%27 = OpAccessChain %_ptr_Input_float %BaseColor %uint_1
%28 = OpLoad %float %27
OpStore %d %28
OpBranch %24
%26 = OpLabel
%29 = OpAccessChain %_ptr_Input_float %BaseColor %uint_2
%30 = OpLoad %float %29
OpStore %d %30
OpBranch %24
%24 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, EliminateUselessInnerLoop) {
  // #version 430
  //
  // layout(std430) buffer U_t
  // {
  //     float g_F[10];
  // };
  //
  // layout(location = 0)out float o;
  //
  // void main(void)
  // {
  //     float s = 0.0;
  //     for (int i=0; i<10; i++) {
  //         for (int j=0; j<10; j++) {
  //         }
  //         s += g_F[i];
  //     }
  //     o = s;
  // }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %s "s"
OpName %i "i"
OpName %j "j"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%int_1 = OpConstant %int 1
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%U_t = OpTypeStruct %_arr_float_uint_10
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
)";

  const std::string predefs_after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %s "s"
OpName %i "i"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%int_1 = OpConstant %int 1
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%U_t = OpTypeStruct %_arr_float_uint_10
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %11
%26 = OpLabel
%s = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%j = OpVariable %_ptr_Function_int Function
OpStore %s %float_0
OpStore %i %int_0
OpBranch %27
%27 = OpLabel
OpLoopMerge %28 %29 None
OpBranch %30
%30 = OpLabel
%31 = OpLoad %int %i
%32 = OpSLessThan %bool %31 %int_10
OpBranchConditional %32 %33 %28
%33 = OpLabel
OpStore %j %int_0
OpBranch %34
%34 = OpLabel
OpLoopMerge %35 %36 None
OpBranch %37
%37 = OpLabel
%38 = OpLoad %int %j
%39 = OpSLessThan %bool %38 %int_10
OpBranchConditional %39 %40 %35
%40 = OpLabel
OpBranch %36
%36 = OpLabel
%41 = OpLoad %int %j
%42 = OpIAdd %int %41 %int_1
OpStore %j %42
OpBranch %34
%35 = OpLabel
%43 = OpLoad %int %i
%44 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %43
%45 = OpLoad %float %44
%46 = OpLoad %float %s
%47 = OpFAdd %float %46 %45
OpStore %s %47
OpBranch %29
%29 = OpLabel
%48 = OpLoad %int %i
%49 = OpIAdd %int %48 %int_1
OpStore %i %49
OpBranch %27
%28 = OpLabel
%50 = OpLoad %float %s
OpStore %o %50
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %11
%26 = OpLabel
%s = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %s %float_0
OpStore %i %int_0
OpBranch %27
%27 = OpLabel
OpLoopMerge %28 %29 None
OpBranch %30
%30 = OpLabel
%31 = OpLoad %int %i
%32 = OpSLessThan %bool %31 %int_10
OpBranchConditional %32 %33 %28
%33 = OpLabel
OpBranch %34
%34 = OpLabel
OpBranch %35
%35 = OpLabel
%43 = OpLoad %int %i
%44 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %43
%45 = OpLoad %float %44
%46 = OpLoad %float %s
%47 = OpFAdd %float %46 %45
OpStore %s %47
OpBranch %29
%29 = OpLabel
%48 = OpLoad %int %i
%49 = OpIAdd %int %48 %int_1
OpStore %i %49
OpBranch %27
%28 = OpLabel
%50 = OpLoad %float %s
OpStore %o %50
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(
      predefs_before + func_before, predefs_after + func_after, true, true);
}

TEST_F(AggressiveDCETest, EliminateUselessNestedLoopWithIf) {
  // #version 430
  //
  // layout(std430) buffer U_t
  // {
  //     float g_F[10][10];
  // };
  //
  // layout(location = 0)out float o;
  //
  // void main(void)
  // {
  //     float s = 0.0;
  //     for (int i=0; i<10; i++) {
  //         for (int j=0; j<10; j++) {
  //             float t = g_F[i][j];
  //             if (t > 0.0)
  //                 s += t;
  //         }
  //     }
  //     o = 0.0;
  // }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %s "s"
OpName %i "i"
OpName %j "j"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpDecorate %_arr__arr_float_uint_10_uint_10 ArrayStride 40
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%12 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%_arr__arr_float_uint_10_uint_10 = OpTypeArray %_arr_float_uint_10 %uint_10
%U_t = OpTypeStruct %_arr__arr_float_uint_10_uint_10
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
)";

  const std::string predefs_after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %o "o"
OpDecorate %o Location 0
%void = OpTypeVoid
%12 = OpTypeFunction %void
%float = OpTypeFloat 32
%float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %12
%27 = OpLabel
%s = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%j = OpVariable %_ptr_Function_int Function
OpStore %s %float_0
OpStore %i %int_0
OpBranch %28
%28 = OpLabel
OpLoopMerge %29 %30 None
OpBranch %31
%31 = OpLabel
%32 = OpLoad %int %i
%33 = OpSLessThan %bool %32 %int_10
OpBranchConditional %33 %34 %29
%34 = OpLabel
OpStore %j %int_0
OpBranch %35
%35 = OpLabel
OpLoopMerge %36 %37 None
OpBranch %38
%38 = OpLabel
%39 = OpLoad %int %j
%40 = OpSLessThan %bool %39 %int_10
OpBranchConditional %40 %41 %36
%41 = OpLabel
%42 = OpLoad %int %i
%43 = OpLoad %int %j
%44 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %42 %43
%45 = OpLoad %float %44
%46 = OpFOrdGreaterThan %bool %45 %float_0
OpSelectionMerge %47 None
OpBranchConditional %46 %48 %47
%48 = OpLabel
%49 = OpLoad %float %s
%50 = OpFAdd %float %49 %45
OpStore %s %50
OpBranch %47
%47 = OpLabel
OpBranch %37
%37 = OpLabel
%51 = OpLoad %int %j
%52 = OpIAdd %int %51 %int_1
OpStore %j %52
OpBranch %35
%36 = OpLabel
OpBranch %30
%30 = OpLabel
%53 = OpLoad %int %i
%54 = OpIAdd %int %53 %int_1
OpStore %i %54
OpBranch %28
%29 = OpLabel
OpStore %o %float_0
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %12
%27 = OpLabel
OpBranch %28
%28 = OpLabel
OpBranch %29
%29 = OpLabel
OpStore %o %float_0
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(
      predefs_before + func_before, predefs_after + func_after, true, true);
}

TEST_F(AggressiveDCETest, EliminateEmptyIfBeforeContinue) {
  // #version 430
  //
  // layout(location = 0)out float o;
  //
  // void main(void)
  // {
  //     float s = 0.0;
  //     for (int i=0; i<10; i++) {
  //         s += 1.0;
  //         if (i > s) {}
  //     }
  //     o = s;
  // }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %3
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
OpSourceExtension "GL_GOOGLE_include_directive"
OpName %main "main"
OpDecorate %3 Location 0
%void = OpTypeVoid
%5 = OpTypeFunction %void
%float = OpTypeFloat 32
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%3 = OpVariable %_ptr_Output_float Output
)";

  const std::string predefs_after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %3
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
OpSourceExtension "GL_GOOGLE_include_directive"
OpName %main "main"
OpDecorate %3 Location 0
%void = OpTypeVoid
%5 = OpTypeFunction %void
%float = OpTypeFloat 32
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%3 = OpVariable %_ptr_Output_float Output
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %5
%16 = OpLabel
OpBranch %17
%17 = OpLabel
%18 = OpPhi %float %float_0 %16 %19 %20
%21 = OpPhi %int %int_0 %16 %22 %20
OpLoopMerge %23 %20 None
OpBranch %24
%24 = OpLabel
%25 = OpSLessThan %bool %21 %int_10
OpBranchConditional %25 %26 %23
%26 = OpLabel
%19 = OpFAdd %float %18 %float_1
%27 = OpConvertFToS %int %19
%28 = OpSGreaterThan %bool %21 %27
OpSelectionMerge %20 None
OpBranchConditional %28 %29 %20
%29 = OpLabel
OpBranch %20
%20 = OpLabel
%22 = OpIAdd %int %21 %int_1
OpBranch %17
%23 = OpLabel
OpStore %3 %18
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %5
%16 = OpLabel
OpBranch %17
%17 = OpLabel
%18 = OpPhi %float %float_0 %16 %19 %20
%21 = OpPhi %int %int_0 %16 %22 %20
OpLoopMerge %23 %20 None
OpBranch %24
%24 = OpLabel
%25 = OpSLessThan %bool %21 %int_10
OpBranchConditional %25 %26 %23
%26 = OpLabel
%19 = OpFAdd %float %18 %float_1
OpBranch %20
%20 = OpLabel
%22 = OpIAdd %int %21 %int_1
OpBranch %17
%23 = OpLabel
OpStore %3 %18
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(
      predefs_before + func_before, predefs_after + func_after, true, true);
}

TEST_F(AggressiveDCETest, NoEliminateLiveNestedLoopWithIf) {
  // Note: SPIR-V optimized
  //
  // #version 430
  //
  // layout(std430) buffer U_t
  // {
  //     float g_F[10][10];
  // };
  //
  // layout(location = 0)out float o;
  //
  // void main(void)
  // {
  //     float s = 0.0;
  //     for (int i=0; i<10; i++) {
  //         for (int j=0; j<10; j++) {
  //             float t = g_F[i][j];
  //             if (t > 0.0)
  //                 s += t;
  //         }
  //     }
  //     o = s;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %s "s"
OpName %i "i"
OpName %j "j"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpDecorate %_arr__arr_float_uint_10_uint_10 ArrayStride 40
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%12 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%_arr__arr_float_uint_10_uint_10 = OpTypeArray %_arr_float_uint_10 %uint_10
%U_t = OpTypeStruct %_arr__arr_float_uint_10_uint_10
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %12
%27 = OpLabel
%s = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%j = OpVariable %_ptr_Function_int Function
OpStore %s %float_0
OpStore %i %int_0
OpBranch %28
%28 = OpLabel
OpLoopMerge %29 %30 None
OpBranch %31
%31 = OpLabel
%32 = OpLoad %int %i
%33 = OpSLessThan %bool %32 %int_10
OpBranchConditional %33 %34 %29
%34 = OpLabel
OpStore %j %int_0
OpBranch %35
%35 = OpLabel
OpLoopMerge %36 %37 None
OpBranch %38
%38 = OpLabel
%39 = OpLoad %int %j
%40 = OpSLessThan %bool %39 %int_10
OpBranchConditional %40 %41 %36
%41 = OpLabel
%42 = OpLoad %int %i
%43 = OpLoad %int %j
%44 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %42 %43
%45 = OpLoad %float %44
%46 = OpFOrdGreaterThan %bool %45 %float_0
OpSelectionMerge %47 None
OpBranchConditional %46 %48 %47
%48 = OpLabel
%49 = OpLoad %float %s
%50 = OpFAdd %float %49 %45
OpStore %s %50
OpBranch %47
%47 = OpLabel
OpBranch %37
%37 = OpLabel
%51 = OpLoad %int %j
%52 = OpIAdd %int %51 %int_1
OpStore %j %52
OpBranch %35
%36 = OpLabel
OpBranch %30
%30 = OpLabel
%53 = OpLoad %int %i
%54 = OpIAdd %int %53 %int_1
OpStore %i %54
OpBranch %28
%29 = OpLabel
%55 = OpLoad %float %s
OpStore %o %55
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, NoEliminateIfContinue) {
  // Do not eliminate continue embedded in if construct
  //
  // #version 430
  //
  // layout(std430) buffer U_t
  // {
  //     float g_F[10];
  // };
  //
  // layout(location = 0)out float o;
  //
  // void main(void)
  // {
  //     float s = 0.0;
  //     for (int i=0; i<10; i++) {
  //         if (i % 2 == 0) continue;
  //         s += g_F[i];
  //     }
  //     o = s;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %s "s"
OpName %i "i"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%int_2 = OpConstant %int 2
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%U_t = OpTypeStruct %_arr_float_uint_10
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %10
%26 = OpLabel
%s = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %s %float_0
OpStore %i %int_0
OpBranch %27
%27 = OpLabel
OpLoopMerge %28 %29 None
OpBranch %30
%30 = OpLabel
%31 = OpLoad %int %i
%32 = OpSLessThan %bool %31 %int_10
OpBranchConditional %32 %33 %28
%33 = OpLabel
%34 = OpLoad %int %i
%35 = OpSMod %int %34 %int_2
%36 = OpIEqual %bool %35 %int_0
OpSelectionMerge %37 None
OpBranchConditional %36 %38 %37
%38 = OpLabel
OpBranch %29
%37 = OpLabel
%39 = OpLoad %int %i
%40 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %39
%41 = OpLoad %float %40
%42 = OpLoad %float %s
%43 = OpFAdd %float %42 %41
OpStore %s %43
OpBranch %29
%29 = OpLabel
%44 = OpLoad %int %i
%45 = OpIAdd %int %44 %int_1
OpStore %i %45
OpBranch %27
%28 = OpLabel
%46 = OpLoad %float %s
OpStore %o %46
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, NoEliminateIfContinue2) {
  // Do not eliminate continue not embedded in if construct
  //
  // #version 430
  //
  // layout(std430) buffer U_t
  // {
  //     float g_F[10];
  // };
  //
  // layout(location = 0)out float o;
  //
  // void main(void)
  // {
  //     float s = 0.0;
  //     for (int i=0; i<10; i++) {
  //         if (i % 2 == 0) continue;
  //         s += g_F[i];
  //     }
  //     o = s;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %s "s"
OpName %i "i"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%int_2 = OpConstant %int 2
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%U_t = OpTypeStruct %_arr_float_uint_10
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %10
%26 = OpLabel
%s = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %s %float_0
OpStore %i %int_0
OpBranch %27
%27 = OpLabel
OpLoopMerge %28 %29 None
OpBranch %30
%30 = OpLabel
%31 = OpLoad %int %i
%32 = OpSLessThan %bool %31 %int_10
OpBranchConditional %32 %33 %28
%33 = OpLabel
%34 = OpLoad %int %i
%35 = OpSMod %int %34 %int_2
%36 = OpIEqual %bool %35 %int_0
OpBranchConditional %36 %29 %37
%37 = OpLabel
%38 = OpLoad %int %i
%39 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %38
%40 = OpLoad %float %39
%41 = OpLoad %float %s
%42 = OpFAdd %float %41 %40
OpStore %s %42
OpBranch %29
%29 = OpLabel
%43 = OpLoad %int %i
%44 = OpIAdd %int %43 %int_1
OpStore %i %44
OpBranch %27
%28 = OpLabel
%45 = OpLoad %float %s
OpStore %o %45
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

TEST_F(AggressiveDCETest, NoEliminateIfContinue3) {
  // Do not eliminate continue as conditional branch with merge instruction
  // Note: SPIR-V edited to add merge instruction before continue.
  //
  // #version 430
  //
  // layout(std430) buffer U_t
  // {
  //     float g_F[10];
  // };
  //
  // layout(location = 0)out float o;
  //
  // void main(void)
  // {
  //     float s = 0.0;
  //     for (int i=0; i<10; i++) {
  //         if (i % 2 == 0) continue;
  //         s += g_F[i];
  //     }
  //     o = s;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %s "s"
OpName %i "i"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%int_2 = OpConstant %int 2
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%U_t = OpTypeStruct %_arr_float_uint_10
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %10
%26 = OpLabel
%s = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
OpStore %s %float_0
OpStore %i %int_0
OpBranch %27
%27 = OpLabel
OpLoopMerge %28 %29 None
OpBranch %30
%30 = OpLabel
%31 = OpLoad %int %i
%32 = OpSLessThan %bool %31 %int_10
OpBranchConditional %32 %33 %28
%33 = OpLabel
%34 = OpLoad %int %i
%35 = OpSMod %int %34 %int_2
%36 = OpIEqual %bool %35 %int_0
OpSelectionMerge %37 None
OpBranchConditional %36 %29 %37
%37 = OpLabel
%38 = OpLoad %int %i
%39 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %38
%40 = OpLoad %float %39
%41 = OpLoad %float %s
%42 = OpFAdd %float %41 %40
OpStore %s %42
OpBranch %29
%29 = OpLabel
%43 = OpLoad %int %i
%44 = OpIAdd %int %43 %int_1
OpStore %i %44
OpBranch %27
%28 = OpLabel
%45 = OpLoad %float %s
OpStore %o %45
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(assembly, assembly, true, true);
}

// This is not valid input and ADCE does not support variable pointers and only
// supports shaders.
TEST_F(AggressiveDCETest, PointerVariable) {
  // ADCE is able to handle code that contains a load whose base address
  // comes from a load and not an OpVariable.  I want to see an instruction
  // removed to be sure that ADCE is not exiting early.

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
OpDecorate %8 DescriptorSet 0
OpDecorate %8 Binding 1
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
%8 = OpVariable %_ptr_Uniform__struct_6 Uniform
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
%_ptr_Function__ptr_Uniform__struct_5 = OpTypePointer Function %_ptr_Uniform__struct_5
%int_0 = OpConstant %int 0
%uint_0 = OpConstant %uint 0
%2 = OpVariable %_ptr_Output_v4float Output
%7 = OpVariable %_ptr_Uniform__struct_5 Uniform
%1 = OpFunction %void None %10
%23 = OpLabel
%24 = OpVariable %_ptr_Function__ptr_Uniform__struct_5 Function
OpStore %24 %7
%25 = OpLoad %_ptr_Uniform__struct_5 %24
%26 = OpAccessChain %_ptr_Uniform_v4float %25 %int_0 %uint_0 %int_0
%27 = OpLoad %v4float %26
OpStore %2 %27
OpReturn
OpFunctionEnd
)";

  // The input is not valid and ADCE only supports shaders, but not variable
  // pointers. Workaround this by enabling relaxed logical pointers in the
  // validator.
  ValidatorOptions()->relax_logical_pointer = true;
  SinglePassRunAndCheck<AggressiveDCEPass>(before, after, true, true);
}

// %dead is unused.  Make sure we remove it along with its name.
TEST_F(AggressiveDCETest, RemoveUnreferenced) {
  const std::string before =
      R"(OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 150
OpName %main "main"
OpName %dead "dead"
%void = OpTypeVoid
%5 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%dead = OpVariable %_ptr_Private_float Private
%main = OpFunction %void None %5
%8 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 150
OpName %main "main"
%void = OpTypeVoid
%5 = OpTypeFunction %void
%main = OpFunction %void None %5
%8 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(before, after, true, true);
}

// Delete %dead because it is unreferenced.  Then %initializer becomes
// unreferenced, so remove it as well.
TEST_F(AggressiveDCETest, RemoveUnreferencedWithInit1) {
  const std::string before =
      R"(OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 150
OpName %main "main"
OpName %dead "dead"
OpName %initializer "initializer"
%void = OpTypeVoid
%6 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%initializer = OpVariable %_ptr_Private_float Private
%dead = OpVariable %_ptr_Private_float Private %initializer
%main = OpFunction %void None %6
%9 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 150
OpName %main "main"
%void = OpTypeVoid
%6 = OpTypeFunction %void
%main = OpFunction %void None %6
%9 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(before, after, true, true);
}

// Keep %live because it is used, and its initializer.
TEST_F(AggressiveDCETest, KeepReferenced) {
  const std::string before =
      R"(OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %output
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 150
OpName %main "main"
OpName %live "live"
OpName %initializer "initializer"
OpName %output "output"
%void = OpTypeVoid
%6 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%initializer = OpConstant %float 0
%live = OpVariable %_ptr_Private_float Private %initializer
%_ptr_Output_float = OpTypePointer Output %float
%output = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %6
%9 = OpLabel
%10 = OpLoad %float %live
OpStore %output %10
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(before, before, true, true);
}

// This test that the decoration associated with a variable are removed when the
// variable is removed.
TEST_F(AggressiveDCETest, RemoveVariableAndDecorations) {
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpSource GLSL 450
OpName %main "main"
OpName %B "B"
OpMemberName %B 0 "a"
OpName %Bdat "Bdat"
OpMemberDecorate %B 0 Offset 0
OpDecorate %B BufferBlock
OpDecorate %Bdat DescriptorSet 0
OpDecorate %Bdat Binding 0
%void = OpTypeVoid
%6 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%B = OpTypeStruct %uint
%_ptr_Uniform_B = OpTypePointer Uniform %B
%Bdat = OpVariable %_ptr_Uniform_B Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%uint_1 = OpConstant %uint 1
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%main = OpFunction %void None %6
%13 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpSource GLSL 450
OpName %main "main"
%void = OpTypeVoid
%6 = OpTypeFunction %void
%main = OpFunction %void None %6
%13 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(before, after, true, true);
}

TEST_F(AggressiveDCETest, DeadNestedSwitch) {
  const std::string text = R"(
; CHECK: OpLabel
; CHECK: OpBranch [[block:%\w+]]
; CHECK-NOT: OpSwitch
; CHECK-NEXT: [[block]] = OpLabel
; CHECK: OpBranch [[block:%\w+]]
; CHECK-NOT: OpSwitch
; CHECK-NEXT: [[block]] = OpLabel
; CHECK-NEXT: OpStore
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func" %x
OpExecutionMode %func OriginUpperLeft
OpName %func "func"
%void = OpTypeVoid
%1 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_ptr_Output = OpTypePointer Output %uint
%uint_ptr_Input = OpTypePointer Input %uint
%x = OpVariable %uint_ptr_Output Output
%a = OpVariable %uint_ptr_Input Input
%func = OpFunction %void None %1
%entry = OpLabel
OpBranch %header
%header = OpLabel
%ld = OpLoad %uint %a
OpLoopMerge %merge %continue None
OpBranch %postheader
%postheader = OpLabel
; This switch doesn't require an OpSelectionMerge and is nested in the dead loop.
OpSwitch %ld %merge 0 %extra 1 %continue
%extra = OpLabel
OpBranch %continue
%continue = OpLabel
OpBranch %header
%merge = OpLabel
OpStore %x %uint_0
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, LiveNestedSwitch) {
  const std::string text = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func" %3 %10
OpExecutionMode %func OriginUpperLeft
OpName %func "func"
%void = OpTypeVoid
%1 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%_ptr_Output_uint = OpTypePointer Output %uint
%_ptr_Input_uint = OpTypePointer Input %uint
%3 = OpVariable %_ptr_Output_uint Output
%10 = OpVariable %_ptr_Input_uint Input
%func = OpFunction %void None %1
%11 = OpLabel
OpBranch %12
%12 = OpLabel
%13 = OpLoad %uint %10
OpLoopMerge %14 %15 None
OpBranch %16
%16 = OpLabel
OpSelectionMerge %18 None
OpSwitch %13 %18 0 %17 1 %19
%17 = OpLabel
OpStore %3 %uint_1
OpBranch %19
%19 = OpLabel
OpBranch %15
%15 = OpLabel
OpBranch %12
%18 = OpLabel
OpBranch %14
%14 = OpLabel
OpStore %3 %uint_0
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(text, text, false, true);
}

TEST_F(AggressiveDCETest, BasicDeleteDeadFunction) {
  // The function Dead should be removed because it is never called.
  const std::vector<const char*> common_code = {
      // clang-format off
               "OpCapability Shader",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\"",
               "OpName %main \"main\"",
               "OpName %Live \"Live\"",
       "%void = OpTypeVoid",
          "%7 = OpTypeFunction %void",
       "%main = OpFunction %void None %7",
         "%15 = OpLabel",
         "%16 = OpFunctionCall %void %Live",
         "%17 = OpFunctionCall %void %Live",
               "OpReturn",
               "OpFunctionEnd",
  "%Live = OpFunction %void None %7",
         "%20 = OpLabel",
               "OpReturn",
               "OpFunctionEnd"
      // clang-format on
  };

  const std::vector<const char*> dead_function = {
      // clang-format off
      "%Dead = OpFunction %void None %7",
         "%19 = OpLabel",
               "OpReturn",
               "OpFunctionEnd",
      // clang-format on
  };

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(
      JoinAllInsts(Concat(common_code, dead_function)),
      JoinAllInsts(common_code), /* skip_nop = */ true);
}

TEST_F(AggressiveDCETest, BasicKeepLiveFunction) {
  // Everything is reachable from an entry point, so no functions should be
  // deleted.
  const std::vector<const char*> text = {
      // clang-format off
               "OpCapability Shader",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\"",
               "OpName %main \"main\"",
               "OpName %Live1 \"Live1\"",
               "OpName %Live2 \"Live2\"",
       "%void = OpTypeVoid",
          "%7 = OpTypeFunction %void",
       "%main = OpFunction %void None %7",
         "%15 = OpLabel",
         "%16 = OpFunctionCall %void %Live2",
         "%17 = OpFunctionCall %void %Live1",
               "OpReturn",
               "OpFunctionEnd",
      "%Live1 = OpFunction %void None %7",
         "%19 = OpLabel",
               "OpReturn",
               "OpFunctionEnd",
      "%Live2 = OpFunction %void None %7",
         "%20 = OpLabel",
               "OpReturn",
               "OpFunctionEnd"
      // clang-format on
  };

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  std::string assembly = JoinAllInsts(text);
  auto result = SinglePassRunAndDisassemble<AggressiveDCEPass>(
      assembly, /* skip_nop = */ true, /* do_validation = */ false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
  EXPECT_EQ(assembly, std::get<0>(result));
}

TEST_F(AggressiveDCETest, BasicRemoveDecorationsAndNames) {
  // We want to remove the names and decorations associated with results that
  // are removed.  This test will check for that.
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpName %main "main"
               OpName %Dead "Dead"
               OpName %x "x"
               OpName %y "y"
               OpName %z "z"
               OpDecorate %x RelaxedPrecision
               OpDecorate %y RelaxedPrecision
               OpDecorate %z RelaxedPrecision
               OpDecorate %6 RelaxedPrecision
               OpDecorate %7 RelaxedPrecision
               OpDecorate %8 RelaxedPrecision
       %void = OpTypeVoid
         %10 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
       %main = OpFunction %void None %10
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
       %Dead = OpFunction %void None %10
         %15 = OpLabel
          %x = OpVariable %_ptr_Function_float Function
          %y = OpVariable %_ptr_Function_float Function
          %z = OpVariable %_ptr_Function_float Function
               OpStore %x %float_1
               OpStore %y %float_1
          %6 = OpLoad %float %x
          %7 = OpLoad %float %y
          %8 = OpFAdd %float %6 %7
               OpStore %z %8
               OpReturn
               OpFunctionEnd)";

  const std::string expected_output = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpName %main "main"
%void = OpTypeVoid
%10 = OpTypeFunction %void
%main = OpFunction %void None %10
%14 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(text, expected_output,
                                           /* skip_nop = */ true);
}

TEST_F(AggressiveDCETest, BasicAllDeadConstants) {
  const std::string text = R"(
  ; CHECK-NOT: OpConstant
               OpCapability Shader
               OpCapability Float64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpName %main "main"
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
        %int = OpTypeInt 32 1
          %9 = OpConstant %int 1
       %uint = OpTypeInt 32 0
         %11 = OpConstant %uint 2
      %float = OpTypeFloat 32
         %13 = OpConstant %float 3.1415
     %double = OpTypeFloat 64
         %15 = OpConstant %double 3.14159265358979
       %main = OpFunction %void None %4
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, BasicNoneDeadConstants) {
  const std::vector<const char*> text = {
      // clang-format off
                "OpCapability Shader",
                "OpCapability Float64",
           "%1 = OpExtInstImport \"GLSL.std.450\"",
                "OpMemoryModel Logical GLSL450",
                "OpEntryPoint Vertex %main \"main\" %btv %bfv %iv %uv %fv %dv",
                "OpName %main \"main\"",
                "OpName %btv \"btv\"",
                "OpName %bfv \"bfv\"",
                "OpName %iv \"iv\"",
                "OpName %uv \"uv\"",
                "OpName %fv \"fv\"",
                "OpName %dv \"dv\"",
        "%void = OpTypeVoid",
          "%10 = OpTypeFunction %void",
        "%bool = OpTypeBool",
 "%_ptr_Output_bool = OpTypePointer Output %bool",
        "%true = OpConstantTrue %bool",
       "%false = OpConstantFalse %bool",
         "%int = OpTypeInt 32 1",
 "%_ptr_Output_int = OpTypePointer Output %int",
       "%int_1 = OpConstant %int 1",
        "%uint = OpTypeInt 32 0",
 "%_ptr_Output_uint = OpTypePointer Output %uint",
      "%uint_2 = OpConstant %uint 2",
       "%float = OpTypeFloat 32",
 "%_ptr_Output_float = OpTypePointer Output %float",
  "%float_3_1415 = OpConstant %float 3.1415",
      "%double = OpTypeFloat 64",
 "%_ptr_Output_double = OpTypePointer Output %double",
 "%double_3_14159265358979 = OpConstant %double 3.14159265358979",
         "%btv = OpVariable %_ptr_Output_bool Output",
         "%bfv = OpVariable %_ptr_Output_bool Output",
          "%iv = OpVariable %_ptr_Output_int Output",
          "%uv = OpVariable %_ptr_Output_uint Output",
          "%fv = OpVariable %_ptr_Output_float Output",
          "%dv = OpVariable %_ptr_Output_double Output",
        "%main = OpFunction %void None %10",
          "%27 = OpLabel",
                "OpStore %btv %true",
                "OpStore %bfv %false",
                "OpStore %iv %int_1",
                "OpStore %uv %uint_2",
                "OpStore %fv %float_3_1415",
                "OpStore %dv %double_3_14159265358979",
                "OpReturn",
                "OpFunctionEnd",
      // clang-format on
  };
  // All constants are used, so none of them should be eliminated.
  SinglePassRunAndCheck<AggressiveDCEPass>(
      JoinAllInsts(text), JoinAllInsts(text), /* skip_nop = */ true);
}

struct AggressiveEliminateDeadConstantTestCase {
  // Type declarations and constants that should be kept.
  std::vector<std::string> used_consts;
  // Instructions that refer to constants, this is added to create uses for
  // some constants so they won't be treated as dead constants.
  std::vector<std::string> main_insts;
  // Dead constants that should be removed.
  std::vector<std::string> dead_consts;
  // Expectations
  std::vector<std::string> checks;
};

// All types that are potentially required in
// AggressiveEliminateDeadConstantTest.
const std::vector<std::string> CommonTypes = {
    // clang-format off
    // scalar types
    "%bool = OpTypeBool",
    "%uint = OpTypeInt 32 0",
    "%int = OpTypeInt 32 1",
    "%float = OpTypeFloat 32",
    "%double = OpTypeFloat 64",
    // vector types
    "%v2bool = OpTypeVector %bool 2",
    "%v2uint = OpTypeVector %uint 2",
    "%v2int = OpTypeVector %int 2",
    "%v3int = OpTypeVector %int 3",
    "%v4int = OpTypeVector %int 4",
    "%v2float = OpTypeVector %float 2",
    "%v3float = OpTypeVector %float 3",
    "%v2double = OpTypeVector %double 2",
    // variable pointer types
    "%_pf_bool = OpTypePointer Output %bool",
    "%_pf_uint = OpTypePointer Output %uint",
    "%_pf_int = OpTypePointer Output %int",
    "%_pf_float = OpTypePointer Output %float",
    "%_pf_double = OpTypePointer Output %double",
    "%_pf_v2int = OpTypePointer Output %v2int",
    "%_pf_v3int = OpTypePointer Output %v3int",
    "%_pf_v2float = OpTypePointer Output %v2float",
    "%_pf_v3float = OpTypePointer Output %v3float",
    "%_pf_v2double = OpTypePointer Output %v2double",
    // struct types
    "%inner_struct = OpTypeStruct %bool %int %float %double",
    "%outer_struct = OpTypeStruct %inner_struct %int %double",
    "%flat_struct = OpTypeStruct %bool %int %float %double",
    // clang-format on
};

using AggressiveEliminateDeadConstantTest =
    PassTest<::testing::TestWithParam<AggressiveEliminateDeadConstantTestCase>>;

TEST_P(AggressiveEliminateDeadConstantTest, Custom) {
  auto& tc = GetParam();
  AssemblyBuilder builder;
  builder.AppendTypesConstantsGlobals(CommonTypes)
      .AppendTypesConstantsGlobals(tc.used_consts)
      .AppendInMain(tc.main_insts);
  const std::string expected = builder.GetCode();
  builder.AppendTypesConstantsGlobals(tc.dead_consts);
  builder.PrependPreamble(tc.checks);
  const std::string assembly_with_dead_const = builder.GetCode();

  // Do not enable validation. As the input code is invalid from the base
  // tests (ported from other passes).
  SinglePassRunAndMatch<AggressiveDCEPass>(assembly_with_dead_const, false);
}

INSTANTIATE_TEST_SUITE_P(
    ScalarTypeConstants, AggressiveEliminateDeadConstantTest,
    ::testing::ValuesIn(std::vector<AggressiveEliminateDeadConstantTestCase>({
        // clang-format off
        // Scalar type constants, one dead constant and one used constant.
        {
            /* .used_consts = */
            {
              "%used_const_int = OpConstant %int 1",
            },
            /* .main_insts = */
            {
              "%int_var = OpVariable %_pf_int Output",
              "OpStore %int_var %used_const_int",
            },
            /* .dead_consts = */
            {
              "%dead_const_int = OpConstant %int 1",
            },
            /* .checks = */
            {
              "; CHECK: [[const:%\\w+]] = OpConstant %int 1",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[const]]",
            },
        },
        {
            /* .used_consts = */
            {
              "%used_const_uint = OpConstant %uint 1",
            },
            /* .main_insts = */
            {
              "%uint_var = OpVariable %_pf_uint Output",
              "OpStore %uint_var %used_const_uint",
            },
            /* .dead_consts = */
            {
              "%dead_const_uint = OpConstant %uint 1",
            },
            /* .checks = */
            {
              "; CHECK: [[const:%\\w+]] = OpConstant %uint 1",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[const]]",
            },
        },
        {
            /* .used_consts = */
            {
              "%used_const_float = OpConstant %float 3.1415",
            },
            /* .main_insts = */
            {
              "%float_var = OpVariable %_pf_float Output",
              "OpStore %float_var %used_const_float",
            },
            /* .dead_consts = */
            {
              "%dead_const_float = OpConstant %float 3.1415",
            },
            /* .checks = */
            {
              "; CHECK: [[const:%\\w+]] = OpConstant %float 3.1415",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[const]]",
            },
        },
        {
            /* .used_consts = */
            {
              "%used_const_double = OpConstant %double 3.14",
            },
            /* .main_insts = */
            {
              "%double_var = OpVariable %_pf_double Output",
              "OpStore %double_var %used_const_double",
            },
            /* .dead_consts = */
            {
              "%dead_const_double = OpConstant %double 3.14",
            },
            /* .checks = */
            {
              "; CHECK: [[const:%\\w+]] = OpConstant %double 3.14",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[const]]",
            },
        },
        // clang-format on
    })));

INSTANTIATE_TEST_SUITE_P(
    VectorTypeConstants, AggressiveEliminateDeadConstantTest,
    ::testing::ValuesIn(std::vector<AggressiveEliminateDeadConstantTestCase>({
        // clang-format off
        // Tests eliminating dead constant type ivec2. One dead constant vector
        // and one used constant vector, each built from its own group of
        // scalar constants.
        {
            /* .used_consts = */
            {
              "%used_int_x = OpConstant %int 1",
              "%used_int_y = OpConstant %int 2",
              "%used_v2int = OpConstantComposite %v2int %used_int_x %used_int_y",
            },
            /* .main_insts = */
            {
              "%v2int_var = OpVariable %_pf_v2int Output",
              "OpStore %v2int_var %used_v2int",
            },
            /* .dead_consts = */
            {
              "%dead_int_x = OpConstant %int 1",
              "%dead_int_y = OpConstant %int 2",
              "%dead_v2int = OpConstantComposite %v2int %dead_int_x %dead_int_y",
            },
            /* .checks = */
            {
              "; CHECK: [[constx:%\\w+]] = OpConstant %int 1",
              "; CHECK: [[consty:%\\w+]] = OpConstant %int 2",
              "; CHECK: [[const:%\\w+]] = OpConstantComposite %v2int [[constx]] [[consty]]",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[const]]",
            },
        },
        // Tests eliminating dead constant ivec3. One dead constant vector and
        // one used constant vector. But both built from a same group of
        // scalar constants.
        {
            /* .used_consts = */
            {
              "%used_int_x = OpConstant %int 1",
              "%used_int_y = OpConstant %int 2",
              "%used_int_z = OpConstant %int 3",
              "%used_v3int = OpConstantComposite %v3int %used_int_x %used_int_y %used_int_z",
            },
            /* .main_insts = */
            {
              "%v3int_var = OpVariable %_pf_v3int Output",
              "OpStore %v3int_var %used_v3int",
            },
            /* .dead_consts = */
            {
              "%dead_v3int = OpConstantComposite %v3int %used_int_x %used_int_y %used_int_z",
            },
            /* .checks = */
            {
              "; CHECK: [[constx:%\\w+]] = OpConstant %int 1",
              "; CHECK: [[consty:%\\w+]] = OpConstant %int 2",
              "; CHECK: [[constz:%\\w+]] = OpConstant %int 3",
              "; CHECK: [[const:%\\w+]] = OpConstantComposite %v3int [[constx]] [[consty]] [[constz]]",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[const]]",
            },
        },
        // Tests eliminating dead constant vec2. One dead constant vector and
        // one used constant vector. Each built from its own group of scalar
        // constants.
        {
            /* .used_consts = */
            {
              "%used_float_x = OpConstant %float 3.1415",
              "%used_float_y = OpConstant %float 4.13",
              "%used_v2float = OpConstantComposite %v2float %used_float_x %used_float_y",
            },
            /* .main_insts = */
            {
              "%v2float_var = OpVariable %_pf_v2float Output",
              "OpStore %v2float_var %used_v2float",
            },
            /* .dead_consts = */
            {
              "%dead_float_x = OpConstant %float 3.1415",
              "%dead_float_y = OpConstant %float 4.13",
              "%dead_v2float = OpConstantComposite %v2float %dead_float_x %dead_float_y",
            },
            /* .checks = */
            {
              "; CHECK: [[constx:%\\w+]] = OpConstant %float 3.1415",
              "; CHECK: [[consty:%\\w+]] = OpConstant %float 4.13",
              "; CHECK: [[const:%\\w+]] = OpConstantComposite %v2float [[constx]] [[consty]]",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[const]]",
            },
        },
        // Tests eliminating dead constant vec3. One dead constant vector and
        // one used constant vector. Both built from a same group of scalar
        // constants.
        {
            /* .used_consts = */
            {
              "%used_float_x = OpConstant %float 3.1415",
              "%used_float_y = OpConstant %float 4.25",
              "%used_float_z = OpConstant %float 4.75",
              "%used_v3float = OpConstantComposite %v3float %used_float_x %used_float_y %used_float_z",
            },
            /* .main_insts = */
            {
              "%v3float_var = OpVariable %_pf_v3float Output",
              "OpStore %v3float_var %used_v3float",
            },
            /* .dead_consts = */
            {
              "%dead_v3float = OpConstantComposite %v3float %used_float_x %used_float_y %used_float_z",
            },
            /* .checks = */
            {
              "; CHECK: [[constx:%\\w+]] = OpConstant %float 3.1415",
              "; CHECK: [[consty:%\\w+]] = OpConstant %float 4.25",
              "; CHECK: [[constz:%\\w+]] = OpConstant %float 4.75",
              "; CHECK: [[const:%\\w+]] = OpConstantComposite %v3float [[constx]] [[consty]]",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[const]]",
            },
        },
        // clang-format on
    })));

INSTANTIATE_TEST_SUITE_P(
    StructTypeConstants, AggressiveEliminateDeadConstantTest,
    ::testing::ValuesIn(std::vector<AggressiveEliminateDeadConstantTestCase>({
        // clang-format off
        // A plain struct type dead constants. All of its components are dead
        // constants too.
        {
            /* .used_consts = */ {},
            /* .main_insts = */ {},
            /* .dead_consts = */
            {
              "%dead_bool = OpConstantTrue %bool",
              "%dead_int = OpConstant %int 1",
              "%dead_float = OpConstant %float 2.5",
              "%dead_double = OpConstant %double 3.14159265358979",
              "%dead_struct = OpConstantComposite %flat_struct %dead_bool %dead_int %dead_float %dead_double",
            },
            /* .checks = */
            {
              "; CHECK-NOT: OpConstant",
            },
        },
        // A plain struct type dead constants. Some of its components are dead
        // constants while others are not.
        {
            /* .used_consts = */
            {
                "%used_int = OpConstant %int 1",
                "%used_double = OpConstant %double 3.14159265358979",
            },
            /* .main_insts = */
            {
                "%int_var = OpVariable %_pf_int Output",
                "OpStore %int_var %used_int",
                "%double_var = OpVariable %_pf_double Output",
                "OpStore %double_var %used_double",
            },
            /* .dead_consts = */
            {
                "%dead_bool = OpConstantTrue %bool",
                "%dead_float = OpConstant %float 2.5",
                "%dead_struct = OpConstantComposite %flat_struct %dead_bool %used_int %dead_float %used_double",
            },
            /* .checks = */
            {
              "; CHECK: [[int:%\\w+]] = OpConstant %int 1",
              "; CHECK: [[double:%\\w+]] = OpConstant %double 3.14159265358979",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[int]]",
              "; CHECK: OpStore {{%\\w+}} [[double]]",
            },
        },
        // A nesting struct type dead constants. All components of both outer
        // and inner structs are dead and should be removed after dead constant
        // elimination.
        {
            /* .used_consts = */ {},
            /* .main_insts = */ {},
            /* .dead_consts = */
            {
              "%dead_bool = OpConstantTrue %bool",
              "%dead_int = OpConstant %int 1",
              "%dead_float = OpConstant %float 2.5",
              "%dead_double = OpConstant %double 3.1415926535",
              "%dead_inner_struct = OpConstantComposite %inner_struct %dead_bool %dead_int %dead_float %dead_double",
              "%dead_int2 = OpConstant %int 2",
              "%dead_double2 = OpConstant %double 1.428571428514",
              "%dead_outer_struct = OpConstantComposite %outer_struct %dead_inner_struct %dead_int2 %dead_double2",
            },
            /* .checks = */
            {
              "; CHECK-NOT: OpConstant",
            },
        },
        // A nesting struct type dead constants. Some of its components are
        // dead constants while others are not.
        {
            /* .used_consts = */
            {
              "%used_int = OpConstant %int 1",
              "%used_double = OpConstant %double 3.14159265358979",
            },
            /* .main_insts = */
            {
              "%int_var = OpVariable %_pf_int Output",
              "OpStore %int_var %used_int",
              "%double_var = OpVariable %_pf_double Output",
              "OpStore %double_var %used_double",
            },
            /* .dead_consts = */
            {
              "%dead_bool = OpConstantTrue %bool",
              "%dead_float = OpConstant %float 2.5",
              "%dead_inner_struct = OpConstantComposite %inner_struct %dead_bool %used_int %dead_float %used_double",
              "%dead_int = OpConstant %int 2",
              "%dead_outer_struct = OpConstantComposite %outer_struct %dead_inner_struct %dead_int %used_double",
            },
            /* .checks = */
            {
              "; CHECK: [[int:%\\w+]] = OpConstant %int 1",
              "; CHECK: [[double:%\\w+]] = OpConstant %double 3.14159265358979",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[int]]",
              "; CHECK: OpStore {{%\\w+}} [[double]]",
            },
        },
        // A nesting struct case. The inner struct is used while the outer struct is not
        {
          /* .used_const = */
          {
            "%used_bool = OpConstantTrue %bool",
            "%used_int = OpConstant %int 1",
            "%used_float = OpConstant %float 1.23",
            "%used_double = OpConstant %double 1.2345678901234",
            "%used_inner_struct = OpConstantComposite %inner_struct %used_bool %used_int %used_float %used_double",
          },
          /* .main_insts = */
          {
            "%bool_var = OpVariable %_pf_bool Output",
            "%bool_from_inner_struct = OpCompositeExtract %bool %used_inner_struct 0",
            "OpStore %bool_var %bool_from_inner_struct",
          },
          /* .dead_consts = */
          {
            "%dead_int = OpConstant %int 2",
            "%dead_outer_struct = OpConstantComposite %outer_struct %used_inner_struct %dead_int %used_double"
          },
          /* .checks = */
          {
            "; CHECK: [[bool:%\\w+]] = OpConstantTrue",
            "; CHECK: [[int:%\\w+]] = OpConstant %int 1",
            "; CHECK: [[float:%\\w+]] = OpConstant %float 1.23",
            "; CHECK: [[double:%\\w+]] = OpConstant %double 1.2345678901234",
            "; CHECK: [[struct:%\\w+]] = OpConstantComposite %inner_struct [[bool]] [[int]] [[float]] [[double]]",
            "; CHECK-NOT: OpConstant",
            "; CHECK: OpCompositeExtract %bool [[struct]]",
          }
        },
        // A nesting struct case. The outer struct is used, so the inner struct should not
        // be removed even though it is not used anywhere.
        {
          /* .used_const = */
          {
            "%used_bool = OpConstantTrue %bool",
            "%used_int = OpConstant %int 1",
            "%used_float = OpConstant %float 1.23",
            "%used_double = OpConstant %double 1.2345678901234",
            "%used_inner_struct = OpConstantComposite %inner_struct %used_bool %used_int %used_float %used_double",
            "%used_outer_struct = OpConstantComposite %outer_struct %used_inner_struct %used_int %used_double"
          },
          /* .main_insts = */
          {
            "%int_var = OpVariable %_pf_int Output",
            "%int_from_outer_struct = OpCompositeExtract %int %used_outer_struct 1",
            "OpStore %int_var %int_from_outer_struct",
          },
          /* .dead_consts = */ {},
          /* .checks = */
          {
            "; CHECK: [[bool:%\\w+]] = OpConstantTrue %bool",
            "; CHECK: [[int:%\\w+]] = OpConstant %int 1",
            "; CHECK: [[float:%\\w+]] = OpConstant %float 1.23",
            "; CHECK: [[double:%\\w+]] = OpConstant %double 1.2345678901234",
            "; CHECK: [[inner_struct:%\\w+]] = OpConstantComposite %inner_struct %used_bool %used_int %used_float %used_double",
            "; CHECK: [[outer_struct:%\\w+]] = OpConstantComposite %outer_struct %used_inner_struct %used_int %used_double",
            "; CHECK: OpCompositeExtract %int [[outer_struct]]",
          },
        },
        // clang-format on
    })));

INSTANTIATE_TEST_SUITE_P(
    ScalarTypeSpecConstants, AggressiveEliminateDeadConstantTest,
    ::testing::ValuesIn(std::vector<AggressiveEliminateDeadConstantTestCase>({
        // clang-format off
        // All scalar type spec constants.
        {
            /* .used_consts = */
            {
              "%used_bool = OpSpecConstantTrue %bool",
              "%used_uint = OpSpecConstant %uint 2",
              "%used_int = OpSpecConstant %int 2",
              "%used_float = OpSpecConstant %float 2.5",
              "%used_double = OpSpecConstant %double 1.428571428514",
            },
            /* .main_insts = */
            {
              "%bool_var = OpVariable %_pf_bool Output",
              "%uint_var = OpVariable %_pf_uint Output",
              "%int_var = OpVariable %_pf_int Output",
              "%float_var = OpVariable %_pf_float Output",
              "%double_var = OpVariable %_pf_double Output",
              "OpStore %bool_var %used_bool",
              "OpStore %uint_var %used_uint",
              "OpStore %int_var %used_int",
              "OpStore %float_var %used_float",
              "OpStore %double_var %used_double",
            },
            /* .dead_consts = */
            {
              "%dead_bool = OpSpecConstantTrue %bool",
              "%dead_uint = OpSpecConstant %uint 2",
              "%dead_int = OpSpecConstant %int 2",
              "%dead_float = OpSpecConstant %float 2.5",
              "%dead_double = OpSpecConstant %double 1.428571428514",
            },
            /* .checks = */
            {
              "; CHECK: [[bool:%\\w+]] = OpSpecConstantTrue %bool",
              "; CHECK: [[uint:%\\w+]] = OpSpecConstant %uint 2",
              "; CHECK: [[int:%\\w+]] = OpSpecConstant %int 2",
              "; CHECK: [[float:%\\w+]] = OpSpecConstant %float 2.5",
              "; CHECK: [[double:%\\w+]] = OpSpecConstant %double 1.428571428514",
              "; CHECK-NOT: OpSpecConstant",
              "; CHECK: OpStore {{%\\w+}} [[bool]]",
              "; CHECK: OpStore {{%\\w+}} [[uint]]",
              "; CHECK: OpStore {{%\\w+}} [[int]]",
              "; CHECK: OpStore {{%\\w+}} [[float]]",
              "; CHECK: OpStore {{%\\w+}} [[double]]",
            },
        },
        // clang-format on
    })));

INSTANTIATE_TEST_SUITE_P(
    VectorTypeSpecConstants, AggressiveEliminateDeadConstantTest,
    ::testing::ValuesIn(std::vector<AggressiveEliminateDeadConstantTestCase>({
        // clang-format off
        // Bool vector type spec constants. One vector has all component dead,
        // another vector has one dead boolean and one used boolean.
        {
            /* .used_consts = */
            {
              "%used_bool = OpSpecConstantTrue %bool",
            },
            /* .main_insts = */
            {
              "%bool_var = OpVariable %_pf_bool Output",
              "OpStore %bool_var %used_bool",
            },
            /* .dead_consts = */
            {
              "%dead_bool = OpSpecConstantFalse %bool",
              "%dead_bool_vec1 = OpSpecConstantComposite %v2bool %dead_bool %dead_bool",
              "%dead_bool_vec2 = OpSpecConstantComposite %v2bool %dead_bool %used_bool",
            },
            /* .checks = */
            {
              "; CHECK: [[bool:%\\w+]] = OpSpecConstantTrue %bool",
              "; CHECK-NOT: OpSpecConstant",
              "; CHECK: OpStore {{%\\w+}} [[bool]]",
            },
        },

        // Uint vector type spec constants. One vector has all component dead,
        // another vector has one dead unsigned integer and one used unsigned
        // integer.
        {
            /* .used_consts = */
            {
              "%used_uint = OpSpecConstant %uint 3",
            },
            /* .main_insts = */
            {
              "%uint_var = OpVariable %_pf_uint Output",
              "OpStore %uint_var %used_uint",
            },
            /* .dead_consts = */
            {
              "%dead_uint = OpSpecConstant %uint 1",
              "%dead_uint_vec1 = OpSpecConstantComposite %v2uint %dead_uint %dead_uint",
              "%dead_uint_vec2 = OpSpecConstantComposite %v2uint %dead_uint %used_uint",
            },
            /* .checks = */
            {
              "; CHECK: [[uint:%\\w+]] = OpSpecConstant %uint 3",
              "; CHECK-NOT: OpSpecConstant",
              "; CHECK: OpStore {{%\\w+}} [[uint]]",
            },
        },

        // Int vector type spec constants. One vector has all component dead,
        // another vector has one dead integer and one used integer.
        {
            /* .used_consts = */
            {
              "%used_int = OpSpecConstant %int 3",
            },
            /* .main_insts = */
            {
              "%int_var = OpVariable %_pf_int Output",
              "OpStore %int_var %used_int",
            },
            /* .dead_consts = */
            {
              "%dead_int = OpSpecConstant %int 1",
              "%dead_int_vec1 = OpSpecConstantComposite %v2int %dead_int %dead_int",
              "%dead_int_vec2 = OpSpecConstantComposite %v2int %dead_int %used_int",
            },
            /* .checks = */
            {
              "; CHECK: [[int:%\\w+]] = OpSpecConstant %int 3",
              "; CHECK-NOT: OpSpecConstant",
              "; CHECK: OpStore {{%\\w+}} [[int]]",
            },
        },

        // Int vector type spec constants built with both spec constants and
        // front-end constants.
        {
            /* .used_consts = */
            {
              "%used_spec_int = OpSpecConstant %int 3",
              "%used_front_end_int = OpConstant %int 3",
            },
            /* .main_insts = */
            {
              "%int_var1 = OpVariable %_pf_int Output",
              "OpStore %int_var1 %used_spec_int",
              "%int_var2 = OpVariable %_pf_int Output",
              "OpStore %int_var2 %used_front_end_int",
            },
            /* .dead_consts = */
            {
              "%dead_spec_int = OpSpecConstant %int 1",
              "%dead_front_end_int = OpConstant %int 1",
              // Dead front-end and dead spec constants
              "%dead_int_vec1 = OpSpecConstantComposite %v2int %dead_spec_int %dead_front_end_int",
              // Used front-end and dead spec constants
              "%dead_int_vec2 = OpSpecConstantComposite %v2int %dead_spec_int %used_front_end_int",
              // Dead front-end and used spec constants
              "%dead_int_vec3 = OpSpecConstantComposite %v2int %dead_front_end_int %used_spec_int",
            },
            /* .checks = */
            {
              "; CHECK: [[int1:%\\w+]] = OpSpecConstant %int 3",
              "; CHECK: [[int2:%\\w+]] = OpConstant %int 3",
              "; CHECK-NOT: OpSpecConstant",
              "; CHECK-NOT: OpConstant",
              "; CHECK: OpStore {{%\\w+}} [[int1]]",
              "; CHECK: OpStore {{%\\w+}} [[int2]]",
            },
        },
        // clang-format on
    })));

INSTANTIATE_TEST_SUITE_P(
    SpecConstantOp, AggressiveEliminateDeadConstantTest,
    ::testing::ValuesIn(std::vector<AggressiveEliminateDeadConstantTestCase>({
        // clang-format off
        // Cast operations: uint <-> int <-> bool
        {
            /* .used_consts = */ {},
            /* .main_insts = */ {},
            /* .dead_consts = */
            {
              // Assistant constants, only used in dead spec constant
              // operations.
              "%signed_zero = OpConstant %int 0",
              "%signed_zero_vec = OpConstantComposite %v2int %signed_zero %signed_zero",
              "%unsigned_zero = OpConstant %uint 0",
              "%unsigned_zero_vec = OpConstantComposite %v2uint %unsigned_zero %unsigned_zero",
              "%signed_one = OpConstant %int 1",
              "%signed_one_vec = OpConstantComposite %v2int %signed_one %signed_one",
              "%unsigned_one = OpConstant %uint 1",
              "%unsigned_one_vec = OpConstantComposite %v2uint %unsigned_one %unsigned_one",

              // Spec constants that support casting to each other.
              "%dead_bool = OpSpecConstantTrue %bool",
              "%dead_uint = OpSpecConstant %uint 1",
              "%dead_int = OpSpecConstant %int 2",
              "%dead_bool_vec = OpSpecConstantComposite %v2bool %dead_bool %dead_bool",
              "%dead_uint_vec = OpSpecConstantComposite %v2uint %dead_uint %dead_uint",
              "%dead_int_vec = OpSpecConstantComposite %v2int %dead_int %dead_int",

              // Scalar cast to boolean spec constant.
              "%int_to_bool = OpSpecConstantOp %bool INotEqual %dead_int %signed_zero",
              "%uint_to_bool = OpSpecConstantOp %bool INotEqual %dead_uint %unsigned_zero",

              // Vector cast to boolean spec constant.
              "%int_to_bool_vec = OpSpecConstantOp %v2bool INotEqual %dead_int_vec %signed_zero_vec",
              "%uint_to_bool_vec = OpSpecConstantOp %v2bool INotEqual %dead_uint_vec %unsigned_zero_vec",

              // Scalar cast to int spec constant.
              "%bool_to_int = OpSpecConstantOp %int Select %dead_bool %signed_one %signed_zero",
              "%uint_to_int = OpSpecConstantOp %uint IAdd %dead_uint %unsigned_zero",

              // Vector cast to int spec constant.
              "%bool_to_int_vec = OpSpecConstantOp %v2int Select %dead_bool_vec %signed_one_vec %signed_zero_vec",
              "%uint_to_int_vec = OpSpecConstantOp %v2uint IAdd %dead_uint_vec %unsigned_zero_vec",

              // Scalar cast to uint spec constant.
              "%bool_to_uint = OpSpecConstantOp %uint Select %dead_bool %unsigned_one %unsigned_zero",
              "%int_to_uint_vec = OpSpecConstantOp %uint IAdd %dead_int %signed_zero",

              // Vector cast to uint spec constant.
              "%bool_to_uint_vec = OpSpecConstantOp %v2uint Select %dead_bool_vec %unsigned_one_vec %unsigned_zero_vec",
              "%int_to_uint = OpSpecConstantOp %v2uint IAdd %dead_int_vec %signed_zero_vec",
            },
            /* .checks = */
            {
              "; CHECK-NOT: OpConstant",
              "; CHECK-NOT: OpSpecConstant",
            },
        },

        // Add, sub, mul, div, rem.
        {
            /* .used_consts = */ {},
            /* .main_insts = */ {},
            /* .dead_consts = */
            {
              "%dead_spec_int_a = OpSpecConstant %int 1",
              "%dead_spec_int_a_vec = OpSpecConstantComposite %v2int %dead_spec_int_a %dead_spec_int_a",

              "%dead_spec_int_b = OpSpecConstant %int 2",
              "%dead_spec_int_b_vec = OpSpecConstantComposite %v2int %dead_spec_int_b %dead_spec_int_b",

              "%dead_const_int_c = OpConstant %int 3",
              "%dead_const_int_c_vec = OpConstantComposite %v2int %dead_const_int_c %dead_const_int_c",

              // Add
              "%add_a_b = OpSpecConstantOp %int IAdd %dead_spec_int_a %dead_spec_int_b",
              "%add_a_b_vec = OpSpecConstantOp %v2int IAdd %dead_spec_int_a_vec %dead_spec_int_b_vec",

              // Sub
              "%sub_a_b = OpSpecConstantOp %int ISub %dead_spec_int_a %dead_spec_int_b",
              "%sub_a_b_vec = OpSpecConstantOp %v2int ISub %dead_spec_int_a_vec %dead_spec_int_b_vec",

              // Mul
              "%mul_a_b = OpSpecConstantOp %int IMul %dead_spec_int_a %dead_spec_int_b",
              "%mul_a_b_vec = OpSpecConstantOp %v2int IMul %dead_spec_int_a_vec %dead_spec_int_b_vec",

              // Div
              "%div_a_b = OpSpecConstantOp %int SDiv %dead_spec_int_a %dead_spec_int_b",
              "%div_a_b_vec = OpSpecConstantOp %v2int SDiv %dead_spec_int_a_vec %dead_spec_int_b_vec",

              // Bitwise Xor
              "%xor_a_b = OpSpecConstantOp %int BitwiseXor %dead_spec_int_a %dead_spec_int_b",
              "%xor_a_b_vec = OpSpecConstantOp %v2int BitwiseXor %dead_spec_int_a_vec %dead_spec_int_b_vec",

              // Scalar Comparison
              "%less_a_b = OpSpecConstantOp %bool SLessThan %dead_spec_int_a %dead_spec_int_b",
            },
            /* .checks = */
            {
              "; CHECK-NOT: OpConstant",
              "; CHECK-NOT: OpSpecConstant",
            },
        },

        // Vectors without used swizzles should be removed.
        {
            /* .used_consts = */
            {
              "%used_int = OpConstant %int 3",
            },
            /* .main_insts = */
            {
              "%int_var = OpVariable %_pf_int Output",
              "OpStore %int_var %used_int",
            },
            /* .dead_consts = */
            {
              "%dead_int = OpConstant %int 3",

              "%dead_spec_int_a = OpSpecConstant %int 1",
              "%vec_a = OpSpecConstantComposite %v4int %dead_spec_int_a %dead_spec_int_a %dead_int %dead_int",

              "%dead_spec_int_b = OpSpecConstant %int 2",
              "%vec_b = OpSpecConstantComposite %v4int %dead_spec_int_b %dead_spec_int_b %used_int %used_int",

              // Extract scalar
              "%a_x = OpSpecConstantOp %int CompositeExtract %vec_a 0",
              "%b_x = OpSpecConstantOp %int CompositeExtract %vec_b 0",

              // Extract vector
              "%a_xy = OpSpecConstantOp %v2int VectorShuffle %vec_a %vec_a 0 1",
              "%b_xy = OpSpecConstantOp %v2int VectorShuffle %vec_b %vec_b 0 1",
            },
            /* .checks = */
            {
              "; CHECK: [[int:%\\w+]] = OpConstant %int 3",
              "; CHECK-NOT: OpConstant",
              "; CHECK-NOT: OpSpecConstant",
              "; CHECK: OpStore {{%\\w+}} [[int]]",
            },
        },
        // Vectors with used swizzles should not be removed.
        {
            /* .used_consts = */
            {
              "%used_int = OpConstant %int 3",
              "%used_spec_int_a = OpSpecConstant %int 1",
              "%used_spec_int_b = OpSpecConstant %int 2",
              // Create vectors
              "%vec_a = OpSpecConstantComposite %v4int %used_spec_int_a %used_spec_int_a %used_int %used_int",
              "%vec_b = OpSpecConstantComposite %v4int %used_spec_int_b %used_spec_int_b %used_int %used_int",
              // Extract vector
              "%a_xy = OpSpecConstantOp %v2int VectorShuffle %vec_a %vec_a 0 1",
              "%b_xy = OpSpecConstantOp %v2int VectorShuffle %vec_b %vec_b 0 1",
            },
            /* .main_insts = */
            {
              "%v2int_var_a = OpVariable %_pf_v2int Output",
              "%v2int_var_b = OpVariable %_pf_v2int Output",
              "OpStore %v2int_var_a %a_xy",
              "OpStore %v2int_var_b %b_xy",
            },
            /* .dead_consts = */ {},
            /* .checks = */
            {
              "; CHECK: [[int:%\\w+]] = OpConstant %int 3",
              "; CHECK: [[a:%\\w+]] = OpSpecConstant %int 1",
              "; CHECK: [[b:%\\w+]] = OpSpecConstant %int 2",
              "; CHECK: [[veca:%\\w+]] = OpSpecConstantComposite %v4int [[a]] [[a]] [[int]] [[int]]",
              "; CHECK: [[vecb:%\\w+]] = OpSpecConstantComposite %v4int [[b]] [[b]] [[int]] [[int]]",
              "; CHECK: [[exa:%\\w+]] = OpSpecConstantOp %v2int VectorShuffle [[veca]] [[veca]] 0 1",
              "; CHECK: [[exb:%\\w+]] = OpSpecConstantOp %v2int VectorShuffle [[vecb]] [[vecb]] 0 1",
              "; CHECK-NOT: OpConstant",
              "; CHECK-NOT: OpSpecConstant",
              "; CHECK: OpStore {{%\\w+}} [[exa]]",
              "; CHECK: OpStore {{%\\w+}} [[exb]]",
            },
        },
        // clang-format on
    })));

INSTANTIATE_TEST_SUITE_P(
    LongDefUseChain, AggressiveEliminateDeadConstantTest,
    ::testing::ValuesIn(std::vector<AggressiveEliminateDeadConstantTestCase>({
        // clang-format off
        // Long Def-Use chain with binary operations.
        {
            /* .used_consts = */
            {
              "%array_size = OpConstant %int 4",
              "%type_arr_int_4 = OpTypeArray %int %array_size",
              "%used_int_0 = OpConstant %int 100",
              "%used_int_1 = OpConstant %int 1",
              "%used_int_2 = OpSpecConstantOp %int IAdd %used_int_0 %used_int_1",
              "%used_int_3 = OpSpecConstantOp %int ISub %used_int_0 %used_int_2",
              "%used_int_4 = OpSpecConstantOp %int IAdd %used_int_0 %used_int_3",
              "%used_int_5 = OpSpecConstantOp %int ISub %used_int_0 %used_int_4",
              "%used_int_6 = OpSpecConstantOp %int IAdd %used_int_0 %used_int_5",
              "%used_int_7 = OpSpecConstantOp %int ISub %used_int_0 %used_int_6",
              "%used_int_8 = OpSpecConstantOp %int IAdd %used_int_0 %used_int_7",
              "%used_int_9 = OpSpecConstantOp %int ISub %used_int_0 %used_int_8",
              "%used_int_10 = OpSpecConstantOp %int IAdd %used_int_0 %used_int_9",
              "%used_int_11 = OpSpecConstantOp %int ISub %used_int_0 %used_int_10",
              "%used_int_12 = OpSpecConstantOp %int IAdd %used_int_0 %used_int_11",
              "%used_int_13 = OpSpecConstantOp %int ISub %used_int_0 %used_int_12",
              "%used_int_14 = OpSpecConstantOp %int IAdd %used_int_0 %used_int_13",
              "%used_int_15 = OpSpecConstantOp %int ISub %used_int_0 %used_int_14",
              "%used_int_16 = OpSpecConstantOp %int ISub %used_int_0 %used_int_15",
              "%used_int_17 = OpSpecConstantOp %int IAdd %used_int_0 %used_int_16",
              "%used_int_18 = OpSpecConstantOp %int ISub %used_int_0 %used_int_17",
              "%used_int_19 = OpSpecConstantOp %int IAdd %used_int_0 %used_int_18",
              "%used_int_20 = OpSpecConstantOp %int ISub %used_int_0 %used_int_19",
              "%used_vec_a = OpSpecConstantComposite %v2int %used_int_18 %used_int_19",
              "%used_vec_b = OpSpecConstantOp %v2int IMul %used_vec_a %used_vec_a",
              "%used_int_21 = OpSpecConstantOp %int CompositeExtract %used_vec_b 0",
              "%used_array = OpConstantComposite %type_arr_int_4 %used_int_20 %used_int_20 %used_int_21 %used_int_21",
            },
            /* .main_insts = */
            {
              "%int_var = OpVariable %_pf_int Output",
              "%used_array_2 = OpCompositeExtract %int %used_array 2",
              "OpStore %int_var %used_array_2",
            },
            /* .dead_consts = */
            {
              "%dead_int_1 = OpConstant %int 2",
              "%dead_int_2 = OpSpecConstantOp %int IAdd %used_int_0 %dead_int_1",
              "%dead_int_3 = OpSpecConstantOp %int ISub %used_int_0 %dead_int_2",
              "%dead_int_4 = OpSpecConstantOp %int IAdd %used_int_0 %dead_int_3",
              "%dead_int_5 = OpSpecConstantOp %int ISub %used_int_0 %dead_int_4",
              "%dead_int_6 = OpSpecConstantOp %int IAdd %used_int_0 %dead_int_5",
              "%dead_int_7 = OpSpecConstantOp %int ISub %used_int_0 %dead_int_6",
              "%dead_int_8 = OpSpecConstantOp %int IAdd %used_int_0 %dead_int_7",
              "%dead_int_9 = OpSpecConstantOp %int ISub %used_int_0 %dead_int_8",
              "%dead_int_10 = OpSpecConstantOp %int IAdd %used_int_0 %dead_int_9",
              "%dead_int_11 = OpSpecConstantOp %int ISub %used_int_0 %dead_int_10",
              "%dead_int_12 = OpSpecConstantOp %int IAdd %used_int_0 %dead_int_11",
              "%dead_int_13 = OpSpecConstantOp %int ISub %used_int_0 %dead_int_12",
              "%dead_int_14 = OpSpecConstantOp %int IAdd %used_int_0 %dead_int_13",
              "%dead_int_15 = OpSpecConstantOp %int ISub %used_int_0 %dead_int_14",
              "%dead_int_16 = OpSpecConstantOp %int ISub %used_int_0 %dead_int_15",
              "%dead_int_17 = OpSpecConstantOp %int IAdd %used_int_0 %dead_int_16",
              "%dead_int_18 = OpSpecConstantOp %int ISub %used_int_0 %dead_int_17",
              "%dead_int_19 = OpSpecConstantOp %int IAdd %used_int_0 %dead_int_18",
              "%dead_int_20 = OpSpecConstantOp %int ISub %used_int_0 %dead_int_19",
              "%dead_vec_a = OpSpecConstantComposite %v2int %dead_int_18 %dead_int_19",
              "%dead_vec_b = OpSpecConstantOp %v2int IMul %dead_vec_a %dead_vec_a",
              "%dead_int_21 = OpSpecConstantOp %int CompositeExtract %dead_vec_b 0",
              "%dead_array = OpConstantComposite %type_arr_int_4 %dead_int_20 %used_int_20 %dead_int_19 %used_int_19",
            },
            /* .checks = */
            {
              "; CHECK: OpConstant %int 4",
              "; CHECK: [[array:%\\w+]] = OpConstantComposite %type_arr_int_4 %used_int_20 %used_int_20 %used_int_21 %used_int_21",
              "; CHECK-NOT: OpConstant",
              "; CHECK-NOT: OpSpecConstant",
              "; CHECK: OpStore {{%\\w+}} [[array]]",
            },
        },
        // Long Def-Use chain with swizzle
        // clang-format on
    })));

TEST_F(AggressiveDCETest, DeadDecorationGroup) {
  // The decoration group should be eliminated because the target of group
  // decorate is dead.
  const std::string text = R"(
; CHECK-NOT: OpDecorat
; CHECK-NOT: OpGroupDecorate
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %1 Restrict
OpDecorate %1 Aliased
%1 = OpDecorationGroup
OpGroupDecorate %1 %var
%void = OpTypeVoid
%func = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_ptr = OpTypePointer Function %uint
%main = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %uint_ptr Function
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, DeadDecorationGroupAndValidDecorationMgr) {
  // The decoration group should be eliminated because the target of group
  // decorate is dead.
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %1 Restrict
OpDecorate %1 Aliased
%1 = OpDecorationGroup
OpGroupDecorate %1 %var
%void = OpTypeVoid
%func = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_ptr = OpTypePointer Function %uint
%main = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %uint_ptr Function
OpReturn
OpFunctionEnd
  )";

  auto pass = MakeUnique<AggressiveDCEPass>();
  auto consumer = [](spv_message_level_t, const char*, const spv_position_t&,
                     const char* message) {
    std::cerr << message << std::endl;
  };
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_1, consumer, text);

  // Build the decoration manager before the pass.
  context->get_decoration_mgr();

  const auto status = pass->Run(context.get());
  EXPECT_EQ(status, Pass::Status::SuccessWithChange);
}

TEST_F(AggressiveDCETest, ParitallyDeadDecorationGroup) {
  const std::string text = R"(
; CHECK: OpDecorate [[grp:%\w+]] Restrict
; CHECK: [[grp]] = OpDecorationGroup
; CHECK: OpGroupDecorate [[grp]] [[output:%\w+]]
; CHECK: [[output]] = OpVariable {{%\w+}} Output
; CHECK-NOT: OpVariable {{%\w+}} Function
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %output
OpExecutionMode %main OriginUpperLeft
OpDecorate %1 Restrict
%1 = OpDecorationGroup
OpGroupDecorate %1 %var %output
%void = OpTypeVoid
%func = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_ptr_Function = OpTypePointer Function %uint
%uint_ptr_Output = OpTypePointer Output %uint
%uint_0 = OpConstant %uint 0
%output = OpVariable %uint_ptr_Output Output
%main = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %uint_ptr_Function Function
OpStore %output %uint_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, ParitallyDeadDecorationGroupDifferentGroupDecorate) {
  const std::string text = R"(
; CHECK: OpDecorate [[grp:%\w+]] Restrict
; CHECK: [[grp]] = OpDecorationGroup
; CHECK: OpGroupDecorate [[grp]] [[output:%\w+]]
; CHECK-NOT: OpGroupDecorate
; CHECK: [[output]] = OpVariable {{%\w+}} Output
; CHECK-NOT: OpVariable {{%\w+}} Function
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %output
OpExecutionMode %main OriginUpperLeft
OpDecorate %1 Restrict
%1 = OpDecorationGroup
OpGroupDecorate %1 %output
OpGroupDecorate %1 %var
%void = OpTypeVoid
%func = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_ptr_Function = OpTypePointer Function %uint
%uint_ptr_Output = OpTypePointer Output %uint
%uint_0 = OpConstant %uint 0
%output = OpVariable %uint_ptr_Output Output
%main = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %uint_ptr_Function Function
OpStore %output %uint_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, DeadGroupMemberDecorate) {
  const std::string text = R"(
; CHECK-NOT: OpDec
; CHECK-NOT: OpGroup
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %1 Offset 0
OpDecorate %1 Uniform
%1 = OpDecorationGroup
OpGroupMemberDecorate %1 %var 0
%void = OpTypeVoid
%func = OpTypeFunction %void
%uint = OpTypeInt 32 0
%struct = OpTypeStruct %uint %uint
%struct_ptr = OpTypePointer Function %struct
%main = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct_ptr Function
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, PartiallyDeadGroupMemberDecorate) {
  const std::string text = R"(
; CHECK: OpDecorate [[grp:%\w+]] Offset 0
; CHECK: OpDecorate [[grp]] RelaxedPrecision
; CHECK: [[grp]] = OpDecorationGroup
; CHECK: OpGroupMemberDecorate [[grp]] [[output:%\w+]] 1
; CHECK: [[output]] = OpTypeStruct
; CHECK-NOT: OpTypeStruct
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %output
OpExecutionMode %main OriginUpperLeft
OpDecorate %1 Offset 0
OpDecorate %1 RelaxedPrecision
%1 = OpDecorationGroup
OpGroupMemberDecorate %1 %var_struct 0 %output_struct 1
%void = OpTypeVoid
%func = OpTypeFunction %void
%uint = OpTypeInt 32 0
%var_struct = OpTypeStruct %uint %uint
%output_struct = OpTypeStruct %uint %uint
%struct_ptr_Function = OpTypePointer Function %var_struct
%struct_ptr_Output = OpTypePointer Output %output_struct
%uint_ptr_Output = OpTypePointer Output %uint
%output = OpVariable %struct_ptr_Output Output
%uint_0 = OpConstant %uint 0
%main = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct_ptr_Function Function
%3 = OpAccessChain %uint_ptr_Output %output %uint_0
OpStore %3 %uint_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest,
       PartiallyDeadGroupMemberDecorateDifferentGroupDecorate) {
  const std::string text = R"(
; CHECK: OpDecorate [[grp:%\w+]] Offset 0
; CHECK: OpDecorate [[grp]] RelaxedPrecision
; CHECK: [[grp]] = OpDecorationGroup
; CHECK: OpGroupMemberDecorate [[grp]] [[output:%\w+]] 1
; CHECK-NOT: OpGroupMemberDecorate
; CHECK: [[output]] = OpTypeStruct
; CHECK-NOT: OpTypeStruct
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %output
OpExecutionMode %main OriginUpperLeft
OpDecorate %1 Offset 0
OpDecorate %1 RelaxedPrecision
%1 = OpDecorationGroup
OpGroupMemberDecorate %1 %var_struct 0
OpGroupMemberDecorate %1 %output_struct 1
%void = OpTypeVoid
%func = OpTypeFunction %void
%uint = OpTypeInt 32 0
%var_struct = OpTypeStruct %uint %uint
%output_struct = OpTypeStruct %uint %uint
%struct_ptr_Function = OpTypePointer Function %var_struct
%struct_ptr_Output = OpTypePointer Output %output_struct
%uint_ptr_Output = OpTypePointer Output %uint
%output = OpVariable %struct_ptr_Output Output
%uint_0 = OpConstant %uint 0
%main = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct_ptr_Function Function
%3 = OpAccessChain %uint_ptr_Output %output %uint_0
OpStore %3 %uint_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

// Test for #1404
TEST_F(AggressiveDCETest, DontRemoveWorkgroupSize) {
  const std::string text = R"(
; CHECK: OpDecorate [[wgs:%\w+]] BuiltIn WorkgroupSize
; CHECK: [[wgs]] = OpSpecConstantComposite
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %func "func"
OpExecutionMode %func LocalSize 1 1 1
OpDecorate %1 BuiltIn WorkgroupSize
%void = OpTypeVoid
%int = OpTypeInt 32 0
%functy = OpTypeFunction %void
%v3int = OpTypeVector %int 3
%2 = OpSpecConstant %int 1
%1 = OpSpecConstantComposite %v3int %2 %2 %2
%func = OpFunction %void None %functy
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

// Test for #1214
TEST_F(AggressiveDCETest, LoopHeaderIsAlsoAnotherLoopMerge) {
  const std::string text = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "func" %2
OpExecutionMode %1 OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%uint = OpTypeInt 32 0
%_ptr_Output_uint = OpTypePointer Output %uint
%2 = OpVariable %_ptr_Output_uint Output
%uint_0 = OpConstant %uint 0
%9 = OpTypeFunction %void
%1 = OpFunction %void None %9
%10 = OpLabel
OpBranch %11
%11 = OpLabel
OpLoopMerge %12 %13 None
OpBranchConditional %true %14 %13
%14 = OpLabel
OpStore %2 %uint_0
OpLoopMerge %15 %16 None
OpBranchConditional %true %15 %16
%16 = OpLabel
OpBranch %14
%15 = OpLabel
OpBranchConditional %true %12 %13
%13 = OpLabel
OpBranch %11
%12 = OpLabel
%17 = OpPhi %uint %uint_0 %15 %uint_0 %18
OpStore %2 %17
OpLoopMerge %19 %18 None
OpBranchConditional %true %19 %18
%18 = OpLabel
OpBranch %12
%19 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(text, text, true, true);
}

TEST_F(AggressiveDCETest, BreaksDontVisitPhis) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func" %var
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 0
%int_ptr_Output = OpTypePointer Output %int
%var = OpVariable %int_ptr_Output Output
%int0 = OpConstant %int 0
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpBranch %outer_header
%outer_header = OpLabel
OpLoopMerge %outer_merge %outer_continue None
OpBranchConditional %true %inner_header %outer_continue
%inner_header = OpLabel
%phi = OpPhi %int %int0 %outer_header %int0 %inner_continue
OpStore %var %phi
OpLoopMerge %inner_merge %inner_continue None
OpBranchConditional %true %inner_merge %inner_continue
%inner_continue = OpLabel
OpBranch %inner_header
%inner_merge = OpLabel
OpBranch %outer_continue
%outer_continue = OpLabel
%p = OpPhi %int %int0 %outer_header %int0 %inner_merge
OpStore %var %p
OpBranch %outer_header
%outer_merge = OpLabel
OpReturn
OpFunctionEnd
)";

  EXPECT_EQ(Pass::Status::SuccessWithoutChange,
            std::get<1>(SinglePassRunAndDisassemble<AggressiveDCEPass>(
                text, false, true)));
}

// Test for #1212
TEST_F(AggressiveDCETest, ConstStoreInnerLoop) {
  const std::string text = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %1 "main" %2
%void = OpTypeVoid
%4 = OpTypeFunction %void
%float = OpTypeFloat 32
%bool = OpTypeBool
%true = OpConstantTrue %bool
%_ptr_Output_float = OpTypePointer Output %float
%2 = OpVariable %_ptr_Output_float Output
%float_3 = OpConstant %float 3
%1 = OpFunction %void None %4
%13 = OpLabel
OpBranch %14
%14 = OpLabel
OpLoopMerge %15 %16 None
OpBranchConditional %true %17 %15
%17 = OpLabel
OpStore %2 %float_3
OpLoopMerge %18 %17 None
OpBranchConditional %true %18 %17
%18 = OpLabel
OpBranch %15
%16 = OpLabel
OpBranch %14
%15 = OpLabel
OpBranch %20
%20 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(text, text, true, true);
}

// Test for #1212
TEST_F(AggressiveDCETest, InnerLoopCopy) {
  const std::string text = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %1 "main" %2 %3
%void = OpTypeVoid
%5 = OpTypeFunction %void
%float = OpTypeFloat 32
%bool = OpTypeBool
%true = OpConstantTrue %bool
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Input_float = OpTypePointer Input %float
%2 = OpVariable %_ptr_Output_float Output
%3 = OpVariable %_ptr_Input_float Input
%1 = OpFunction %void None %5
%14 = OpLabel
OpBranch %15
%15 = OpLabel
OpLoopMerge %16 %17 None
OpBranchConditional %true %18 %16
%18 = OpLabel
%19 = OpLoad %float %3
OpStore %2 %19
OpLoopMerge %20 %18 None
OpBranchConditional %true %20 %18
%20 = OpLabel
OpBranch %16
%17 = OpLabel
OpBranch %15
%16 = OpLabel
OpBranch %22
%22 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(text, text, true, true);
}

TEST_F(AggressiveDCETest, AtomicAdd) {
  const std::string text = R"(OpCapability SampledBuffer
OpCapability StorageImageExtendedFormats
OpCapability ImageBuffer
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %2 "min"
OpExecutionMode %2 LocalSize 64 1 1
OpSource HLSL 600
OpDecorate %4 DescriptorSet 4
OpDecorate %4 Binding 70
%uint = OpTypeInt 32 0
%6 = OpTypeImage %uint Buffer 0 0 0 2 R32ui
%_ptr_UniformConstant_6 = OpTypePointer UniformConstant %6
%_ptr_Private_6 = OpTypePointer Private %6
%void = OpTypeVoid
%10 = OpTypeFunction %void
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%_ptr_Image_uint = OpTypePointer Image %uint
%4 = OpVariable %_ptr_UniformConstant_6 UniformConstant
%16 = OpVariable %_ptr_Private_6 Private
%2 = OpFunction %void None %10
%17 = OpLabel
%18 = OpLoad %6 %4
OpStore %16 %18
%19 = OpImageTexelPointer %_ptr_Image_uint %16 %uint_0 %uint_0
%20 = OpAtomicIAdd %uint %19 %uint_1 %uint_0 %uint_1
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(text, text, true, true);
}

TEST_F(AggressiveDCETest, SafelyRemoveDecorateString) {
  const std::string preamble = R"(OpCapability Shader
OpExtension "SPV_GOOGLE_hlsl_functionality1"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %1 "main"
OpExecutionMode %1 OriginUpperLeft
)";

  const std::string body_before =
      R"(OpDecorateStringGOOGLE %2 HlslSemanticGOOGLE "FOOBAR"
%void = OpTypeVoid
%4 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%2 = OpVariable %_ptr_StorageBuffer_uint StorageBuffer
%1 = OpFunction %void None %4
%7 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string body_after = R"(%void = OpTypeVoid
%4 = OpTypeFunction %void
%1 = OpFunction %void None %4
%7 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(preamble + body_before,
                                           preamble + body_after, true, true);
}

TEST_F(AggressiveDCETest, CopyMemoryToGlobal) {
  // |local| is loaded in an OpCopyMemory instruction.  So the store must be
  // kept alive.
  const std::string test =
      R"(OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %global
OpExecutionMode %main Triangles
OpExecutionMode %main Invocations 1
OpExecutionMode %main OutputTriangleStrip
OpExecutionMode %main OutputVertices 5
OpSource GLSL 440
OpName %main "main"
OpName %local "local"
OpName %global "global"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%12 = OpConstantNull %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%global = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %7
%19 = OpLabel
%local = OpVariable %_ptr_Function_v4float Function
OpStore %local %12
OpCopyMemory %global %local
OpEndPrimitive
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(test, test, true, true);
}

TEST_F(AggressiveDCETest, CopyMemoryToLocal) {
  // Make sure the store to |local2| using OpCopyMemory is kept and keeps
  // |local1| alive.
  const std::string test =
      R"(OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %global
OpExecutionMode %main Triangles
OpExecutionMode %main Invocations 1
OpExecutionMode %main OutputTriangleStrip
OpExecutionMode %main OutputVertices 5
OpSource GLSL 440
OpName %main "main"
OpName %local1 "local1"
OpName %local2 "local2"
OpName %global "global"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%12 = OpConstantNull %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%global = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %7
%19 = OpLabel
%local1 = OpVariable %_ptr_Function_v4float Function
%local2 = OpVariable %_ptr_Function_v4float Function
OpStore %local1 %12
OpCopyMemory %local2 %local1
OpCopyMemory %global %local2
OpEndPrimitive
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(test, test, true, true);
}

TEST_F(AggressiveDCETest, RemoveCopyMemoryToLocal) {
  // Test that we remove function scope variables that are stored to using
  // OpCopyMemory, but are never loaded.  We can remove both |local1| and
  // |local2|.
  const std::string test =
      R"(OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %global
OpExecutionMode %main Triangles
OpExecutionMode %main Invocations 1
OpExecutionMode %main OutputTriangleStrip
OpExecutionMode %main OutputVertices 5
OpSource GLSL 440
OpName %main "main"
OpName %local1 "local1"
OpName %local2 "local2"
OpName %global "global"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%12 = OpConstantNull %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%global = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %7
%19 = OpLabel
%local1 = OpVariable %_ptr_Function_v4float Function
%local2 = OpVariable %_ptr_Function_v4float Function
OpStore %local1 %12
OpCopyMemory %local2 %local1
OpEndPrimitive
OpReturn
OpFunctionEnd
)";

  const std::string result =
      R"(OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %global
OpExecutionMode %main Triangles
OpExecutionMode %main Invocations 1
OpExecutionMode %main OutputTriangleStrip
OpExecutionMode %main OutputVertices 5
OpSource GLSL 440
OpName %main "main"
OpName %global "global"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%global = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %7
%19 = OpLabel
OpEndPrimitive
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(test, result, true, true);
}

TEST_F(AggressiveDCETest, RemoveCopyMemoryToLocal2) {
  // We are able to remove "local2" because it is not loaded, but have to keep
  // the stores to "local1".
  const std::string test =
      R"(OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %global
OpExecutionMode %main Triangles
OpExecutionMode %main Invocations 1
OpExecutionMode %main OutputTriangleStrip
OpExecutionMode %main OutputVertices 5
OpSource GLSL 440
OpName %main "main"
OpName %local1 "local1"
OpName %local2 "local2"
OpName %global "global"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%12 = OpConstantNull %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%global = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %7
%19 = OpLabel
%local1 = OpVariable %_ptr_Function_v4float Function
%local2 = OpVariable %_ptr_Function_v4float Function
OpStore %local1 %12
OpCopyMemory %local2 %local1
OpCopyMemory %global %local1
OpEndPrimitive
OpReturn
OpFunctionEnd
)";

  const std::string result =
      R"(OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %global
OpExecutionMode %main Triangles
OpExecutionMode %main Invocations 1
OpExecutionMode %main OutputTriangleStrip
OpExecutionMode %main OutputVertices 5
OpSource GLSL 440
OpName %main "main"
OpName %local1 "local1"
OpName %global "global"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%12 = OpConstantNull %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%global = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %7
%19 = OpLabel
%local1 = OpVariable %_ptr_Function_v4float Function
OpStore %local1 %12
OpCopyMemory %global %local1
OpEndPrimitive
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(test, result, true, true);
}

TEST_F(AggressiveDCETest, StructuredIfWithConditionalExit) {
  // We are able to remove "local2" because it is not loaded, but have to keep
  // the stores to "local1".
  const std::string test =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
OpSourceExtension "GL_GOOGLE_include_directive"
OpName %main "main"
OpName %a "a"
%void = OpTypeVoid
%5 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Uniform_int = OpTypePointer Uniform %int
%int_0 = OpConstant %int 0
%bool = OpTypeBool
%int_100 = OpConstant %int 100
%int_1 = OpConstant %int 1
%a = OpVariable %_ptr_Uniform_int Uniform
%main = OpFunction %void None %5
%12 = OpLabel
%13 = OpLoad %int %a
%14 = OpSGreaterThan %bool %13 %int_0
OpSelectionMerge %15 None
OpBranchConditional %14 %16 %15
%16 = OpLabel
%17 = OpLoad %int %a
%18 = OpSLessThan %bool %17 %int_100
OpBranchConditional %18 %19 %15
%19 = OpLabel
OpStore %a %int_1
OpBranch %15
%15 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(test, test, true, true);
}

TEST_F(AggressiveDCETest, CountingLoopNotEliminated) {
  // #version 310 es
  //
  // precision highp float;
  // precision highp int;
  //
  // layout(location = 0) out vec4 _GLF_color;
  //
  // void main()
  // {
  //   float data[1];
  //   for (int c = 0; c < 1; c++) {
  //     if (true) {
  //       do {
  //         for (int i = 0; i < 1; i++) {
  //           data[i] = 1.0;
  //         }
  //       } while (false);
  //     }
  //   }
  //   _GLF_color = vec4(data[0], 0.0, 0.0, 1.0);
  // }
  const std::string test =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %_GLF_color
OpExecutionMode %main OriginUpperLeft
OpSource ESSL 310
OpName %main "main"
OpName %c "c"
OpName %i "i"
OpName %data "data"
OpName %_GLF_color "_GLF_color"
OpDecorate %_GLF_color Location 0
%void = OpTypeVoid
%8 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%bool = OpTypeBool
%float = OpTypeFloat 32
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%_ptr_Function__arr_float_uint_1 = OpTypePointer Function %_arr_float_uint_1
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_GLF_color = OpVariable %_ptr_Output_v4float Output
%float_0 = OpConstant %float 0
%main = OpFunction %void None %8
%26 = OpLabel
%c = OpVariable %_ptr_Function_int Function
%i = OpVariable %_ptr_Function_int Function
%data = OpVariable %_ptr_Function__arr_float_uint_1 Function
OpStore %c %int_0
OpBranch %27
%27 = OpLabel
OpLoopMerge %28 %29 None
OpBranch %30
%30 = OpLabel
%31 = OpLoad %int %c
%32 = OpSLessThan %bool %31 %int_1
OpBranchConditional %32 %33 %28
%33 = OpLabel
OpBranch %34
%34 = OpLabel
OpBranch %35
%35 = OpLabel
OpLoopMerge %36 %37 None
OpBranch %38
%38 = OpLabel
OpStore %i %int_0
OpBranch %39
%39 = OpLabel
OpLoopMerge %40 %41 None
OpBranch %42
%42 = OpLabel
%43 = OpLoad %int %i
%44 = OpSLessThan %bool %43 %int_1
OpBranchConditional %44 %46 %40
%46 = OpLabel
%47 = OpLoad %int %i
%48 = OpAccessChain %_ptr_Function_float %data %47
OpStore %48 %float_1
OpBranch %41
%41 = OpLabel
%49 = OpLoad %int %i
%50 = OpIAdd %int %49 %int_1
OpStore %i %50
OpBranch %39
%40 = OpLabel
OpBranch %37
%37 = OpLabel
OpBranchConditional %false %35 %36
%36 = OpLabel
OpBranch %45
%45 = OpLabel
OpBranch %29
%29 = OpLabel
%51 = OpLoad %int %c
%52 = OpIAdd %int %51 %int_1
OpStore %c %52
OpBranch %27
%28 = OpLabel
%53 = OpAccessChain %_ptr_Function_float %data %int_0
%54 = OpLoad %float %53
%55 = OpCompositeConstruct %v4float %54 %float_0 %float_0 %float_1
OpStore %_GLF_color %55
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(test, test, true, true);
}

TEST_F(AggressiveDCETest, EliminateLoopWithUnreachable) {
  // #version 430
  //
  // layout(std430) buffer U_t
  // {
  //   float g_F[10];
  //   float g_S;
  // };
  //
  // layout(location = 0)out float o;
  //
  // void main(void)
  // {
  //   // Useless loop
  //   for (int i = 0; i<10; i++) {
  //     if (g_F[i] == 0.0)
  //       break;
  //     else
  //       break;
  //     // Unreachable merge block created here.
  //     // Need to edit SPIR-V to change to OpUnreachable
  //   }
  //   o = g_S;
  // }

  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %i "i"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpMemberName %U_t 1 "g_S"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpMemberDecorate %U_t 0 Offset 0
OpMemberDecorate %U_t 1 Offset 40
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%9 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%bool = OpTypeBool
%float = OpTypeFloat 32
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%U_t = OpTypeStruct %_arr_float_uint_10 %float
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%float_0 = OpConstant %float 0
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %9
%23 = OpLabel
%i = OpVariable %_ptr_Function_int Function
OpStore %i %int_0
OpBranch %24
%24 = OpLabel
OpLoopMerge %25 %26 None
OpBranch %27
%27 = OpLabel
%28 = OpLoad %int %i
%29 = OpSLessThan %bool %28 %int_10
OpBranchConditional %29 %30 %25
%30 = OpLabel
%31 = OpLoad %int %i
%32 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %31
%33 = OpLoad %float %32
%34 = OpFOrdEqual %bool %33 %float_0
OpSelectionMerge %35 None
OpBranchConditional %34 %36 %37
%36 = OpLabel
OpBranch %25
%37 = OpLabel
OpBranch %25
%35 = OpLabel
OpUnreachable
%26 = OpLabel
%38 = OpLoad %int %i
%39 = OpIAdd %int %38 %int_1
OpStore %i %39
OpBranch %24
%25 = OpLabel
%40 = OpAccessChain %_ptr_Uniform_float %_ %int_1
%41 = OpLoad %float %40
OpStore %o %41
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %o
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_F"
OpMemberName %U_t 1 "g_S"
OpName %_ ""
OpName %o "o"
OpDecorate %_arr_float_uint_10 ArrayStride 4
OpMemberDecorate %U_t 0 Offset 0
OpMemberDecorate %U_t 1 Offset 40
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpDecorate %o Location 0
%void = OpTypeVoid
%9 = OpTypeFunction %void
%int = OpTypeInt 32 1
%float = OpTypeFloat 32
%uint = OpTypeInt 32 0
%uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%U_t = OpTypeStruct %_arr_float_uint_10 %float
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%o = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %9
%23 = OpLabel
OpBranch %24
%24 = OpLabel
OpBranch %25
%25 = OpLabel
%40 = OpAccessChain %_ptr_Uniform_float %_ %int_1
%41 = OpLoad %float %40
OpStore %o %41
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(before, after, true, true);
}

TEST_F(AggressiveDCETest, DeadHlslCounterBufferGOOGLE) {
  // We are able to remove "local2" because it is not loaded, but have to keep
  // the stores to "local1".
  const std::string test =
      R"(
; CHECK-NOT: OpDecorateId
; CHECK: [[var:%\w+]] = OpVariable
; CHECK-NOT: OpVariable
; CHECK: [[ac:%\w+]] = OpAccessChain {{%\w+}} [[var]]
; CHECK: OpStore [[ac]]
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 32 1 1
               OpSource HLSL 600
               OpDecorate %_runtimearr_v2float ArrayStride 8
               OpMemberDecorate %_struct_3 0 Offset 0
               OpDecorate %_struct_3 BufferBlock
               OpMemberDecorate %_struct_4 0 Offset 0
               OpDecorate %_struct_4 BufferBlock
               OpDecorateId %5 HlslCounterBufferGOOGLE %6
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
               OpDecorate %6 DescriptorSet 0
               OpDecorate %6 Binding 1
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_runtimearr_v2float = OpTypeRuntimeArray %v2float
  %_struct_3 = OpTypeStruct %_runtimearr_v2float
%_ptr_Uniform__struct_3 = OpTypePointer Uniform %_struct_3
        %int = OpTypeInt 32 1
  %_struct_4 = OpTypeStruct %int
%_ptr_Uniform__struct_4 = OpTypePointer Uniform %_struct_4
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
         %19 = OpConstantNull %v2float
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
          %5 = OpVariable %_ptr_Uniform__struct_3 Uniform
          %6 = OpVariable %_ptr_Uniform__struct_4 Uniform
          %1 = OpFunction %void None %13
         %22 = OpLabel
         %23 = OpAccessChain %_ptr_Uniform_v2float %5 %int_0 %int_0
               OpStore %23 %19
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(test, true);
}

TEST_F(AggressiveDCETest, Dead) {
  // We are able to remove "local2" because it is not loaded, but have to keep
  // the stores to "local1".
  const std::string test =
      R"(
; CHECK: OpCapability
; CHECK-NOT: OpMemberDecorateStringGOOGLE
; CHECK: OpFunctionEnd
           OpCapability Shader
           OpExtension "SPV_GOOGLE_hlsl_functionality1"
      %1 = OpExtInstImport "GLSL.std.450"
           OpMemoryModel Logical GLSL450
           OpEntryPoint Vertex %VSMain "VSMain"
           OpSource HLSL 500
           OpName %VSMain "VSMain"
           OpName %PSInput "PSInput"
           OpMemberName %PSInput 0 "Pos"
           OpMemberName %PSInput 1 "uv"
           OpMemberDecorateStringGOOGLE %PSInput 0 HlslSemanticGOOGLE "SV_POSITION"
           OpMemberDecorateStringGOOGLE %PSInput 1 HlslSemanticGOOGLE "TEX_COORD"
   %void = OpTypeVoid
      %5 = OpTypeFunction %void
  %float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%PSInput = OpTypeStruct %v4float %v2float
 %VSMain = OpFunction %void None %5
      %9 = OpLabel
           OpReturn
           OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(test, true);
}

TEST_F(AggressiveDCETest, DeadInfiniteLoop) {
  const std::string test = R"(
; CHECK: OpSwitch {{%\w+}} {{%\w+}} {{\w+}} {{%\w+}} {{\w+}} [[block:%\w+]]
; CHECK: [[block]] = OpLabel
; CHECK-NEXT: OpBranch [[block:%\w+]]
; CHECK: [[block]] = OpLabel
; CHECK-NEXT: OpBranch [[block:%\w+]]
; CHECK: [[block]] = OpLabel
; CHECK-NEXT: OpReturn
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
          %6 = OpTypeVoid
          %7 = OpTypeFunction %6
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %8 3
         %10 = OpTypeFunction %9
         %11 = OpConstant %8 1
         %12 = OpConstantComposite %9 %11 %11 %11
         %13 = OpTypeInt 32 1
         %32 = OpUndef %13
          %2 = OpFunction %6 None %7
         %33 = OpLabel
               OpBranch %34
         %34 = OpLabel
               OpLoopMerge %35 %36 None
               OpBranch %37
         %37 = OpLabel
         %38 = OpFunctionCall %9 %39
               OpSelectionMerge %40 None
               OpSwitch %32 %40 14 %41 58 %42
         %42 = OpLabel
               OpBranch %43
         %43 = OpLabel
               OpLoopMerge %44 %45 None
               OpBranch %45
         %45 = OpLabel
               OpBranch %43
         %44 = OpLabel
               OpUnreachable
         %41 = OpLabel
               OpBranch %36
         %40 = OpLabel
               OpBranch %36
         %36 = OpLabel
               OpBranch %34
         %35 = OpLabel
               OpReturn
               OpFunctionEnd
         %39 = OpFunction %9 None %10
         %46 = OpLabel
               OpReturnValue %12
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(test, true);
}

TEST_F(AggressiveDCETest, DeadInfiniteLoopReturnValue) {
  const std::string test = R"(
; CHECK: [[vec3:%\w+]] = OpTypeVector
; CHECK: [[undef:%\w+]] = OpUndef [[vec3]]
; CHECK: OpSwitch {{%\w+}} {{%\w+}} {{\w+}} {{%\w+}} {{\w+}} [[block:%\w+]]
; CHECK: [[block]] = OpLabel
; CHECK-NEXT: OpBranch [[block:%\w+]]
; CHECK: [[block]] = OpLabel
; CHECK-NEXT: OpBranch [[block:%\w+]]
; CHECK: [[block]] = OpLabel
; CHECK-NEXT: OpReturnValue [[undef]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
          %6 = OpTypeVoid
          %7 = OpTypeFunction %6
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %8 3
         %10 = OpTypeFunction %9
         %11 = OpConstant %8 1
         %12 = OpConstantComposite %9 %11 %11 %11
         %13 = OpTypeInt 32 1
         %32 = OpUndef %13
          %2 = OpFunction %6 None %7
      %entry = OpLabel
       %call = OpFunctionCall %9 %func
               OpReturn
               OpFunctionEnd
       %func = OpFunction %9 None %10
         %33 = OpLabel
               OpBranch %34
         %34 = OpLabel
               OpLoopMerge %35 %36 None
               OpBranch %37
         %37 = OpLabel
         %38 = OpFunctionCall %9 %39
               OpSelectionMerge %40 None
               OpSwitch %32 %40 14 %41 58 %42
         %42 = OpLabel
               OpBranch %43
         %43 = OpLabel
               OpLoopMerge %44 %45 None
               OpBranch %45
         %45 = OpLabel
               OpBranch %43
         %44 = OpLabel
               OpUnreachable
         %41 = OpLabel
               OpBranch %36
         %40 = OpLabel
               OpBranch %36
         %36 = OpLabel
               OpBranch %34
         %35 = OpLabel
               OpReturnValue %12
               OpFunctionEnd
         %39 = OpFunction %9 None %10
         %46 = OpLabel
               OpReturnValue %12
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(test, true);
}

TEST_F(AggressiveDCETest, TestVariablePointer) {
  const std::string before =
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
%4 = OpVariable %_ptr_StorageBuffer__struct_3 StorageBuffer
%bool = OpTypeBool
%true = OpConstantTrue %bool
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
%2 = OpFunction %void None %8
%16 = OpLabel
%17 = OpAccessChain %_ptr_StorageBuffer_int %4 %int_0 %int_0
OpBranch %18
%18 = OpLabel
%19 = OpPhi %_ptr_StorageBuffer_int %17 %16 %20 %21
OpLoopMerge %22 %21 None
OpBranchConditional %true %23 %22
%23 = OpLabel
OpStore %19 %int_0
OpBranch %21
%21 = OpLabel
%20 = OpPtrAccessChain %_ptr_StorageBuffer_int %19 %int_1
OpBranch %18
%22 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(before, before, true, true);
}

TEST_F(AggressiveDCETest, DeadInputInterfaceV13) {
  const std::string spirv = R"(
; CHECK: OpEntryPoint GLCompute %main "main"
; CHECK-NOT: OpVariable
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %dead
OpExecutionMode %main LocalSize 1 1 1
OpName %main "main"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%ptr_input_int = OpTypePointer Input %int
%dead = OpVariable %ptr_input_int Input
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_3);
  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, DeadInputInterfaceV14) {
  const std::string spirv = R"(
; CHECK: OpEntryPoint GLCompute %main "main"
; CHECK-NOT: OpVariable
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %dead
OpExecutionMode %main LocalSize 1 1 1
OpName %main "main"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%ptr_input_int = OpTypePointer Input %int
%dead = OpVariable %ptr_input_int Input
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, DeadInterfaceV14) {
  const std::string spirv = R"(
; CHECK-NOT: OpEntryPoint GLCompute %main "main" %
; CHECK: OpEntryPoint GLCompute %main "main"
; CHECK-NOT: OpVariable
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %dead
OpExecutionMode %main LocalSize 1 1 1
OpName %main "main"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%ptr_private_int = OpTypePointer Private %int
%dead = OpVariable %ptr_private_int Private
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, DeadInterfacesV14) {
  const std::string spirv = R"(
; CHECK: OpEntryPoint GLCompute %main "main" %live1 %live2
; CHECK-NOT: %dead
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %live1 %dead1 %dead2 %live2
OpExecutionMode %main LocalSize 1 1 1
OpName %main "main"
OpName %live1 "live1"
OpName %live2 "live2"
OpName %dead1 "dead1"
OpName %dead2 "dead2"
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int0 = OpConstant %int 0
%ptr_ssbo_int = OpTypePointer StorageBuffer %int
%live1 = OpVariable %ptr_ssbo_int StorageBuffer
%live2 = OpVariable %ptr_ssbo_int StorageBuffer
%dead1 = OpVariable %ptr_ssbo_int StorageBuffer
%dead2 = OpVariable %ptr_ssbo_int StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpStore %live1 %int0
OpStore %live2 %int0
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, PreserveBindings) {
  const std::string spirv = R"(
; CHECK: OpDecorate %unusedSampler DescriptorSet 0
; CHECK: OpDecorate %unusedSampler Binding 0
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %unusedSampler "unusedSampler"
OpDecorate %unusedSampler DescriptorSet 0
OpDecorate %unusedSampler Binding 0
%void = OpTypeVoid
%5 = OpTypeFunction %void
%float = OpTypeFloat 32
%7 = OpTypeImage %float 2D 0 0 0 1 Unknown
%8 = OpTypeSampledImage %7
%_ptr_UniformConstant_8 = OpTypePointer UniformConstant %8
%unusedSampler = OpVariable %_ptr_UniformConstant_8 UniformConstant
%main = OpFunction %void None %5
%10 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);

  OptimizerOptions()->preserve_bindings_ = true;

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, PreserveSpecConstants) {
  const std::string spirv = R"(
; CHECK: OpName %specConstant "specConstant"
; CHECK: %specConstant = OpSpecConstant %int 0
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %specConstant "specConstant"
OpDecorate %specConstant SpecId 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%int = OpTypeInt 32 1
%specConstant = OpSpecConstant %int 0
%main = OpFunction %void None %3
%5 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);

  OptimizerOptions()->preserve_spec_constants_ = true;

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, LiveDecorateId) {
  const std::string spirv = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main" %2
OpExecutionMode %1 LocalSize 8 1 1
OpDecorate %2 DescriptorSet 0
OpDecorate %2 Binding 0
OpDecorateId %3 UniformId %uint_2
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint_2 = OpConstant %uint 2
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%2 = OpVariable %_ptr_StorageBuffer_uint StorageBuffer
%8 = OpTypeFunction %void
%1 = OpFunction %void None %8
%9 = OpLabel
%3 = OpLoad %uint %2
OpStore %2 %3
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  OptimizerOptions()->preserve_spec_constants_ = true;
  SinglePassRunAndCheck<AggressiveDCEPass>(spirv, spirv, true);
}

TEST_F(AggressiveDCETest, LiveDecorateIdOnGroup) {
  const std::string spirv = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main" %2
OpExecutionMode %1 LocalSize 8 1 1
OpDecorate %2 DescriptorSet 0
OpDecorate %2 Binding 0
OpDecorateId %3 UniformId %uint_2
%3 = OpDecorationGroup
OpGroupDecorate %3 %5
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint_2 = OpConstant %uint 2
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%2 = OpVariable %_ptr_StorageBuffer_uint StorageBuffer
%9 = OpTypeFunction %void
%1 = OpFunction %void None %9
%10 = OpLabel
%5 = OpLoad %uint %2
OpStore %2 %5
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  OptimizerOptions()->preserve_spec_constants_ = true;
  SinglePassRunAndCheck<AggressiveDCEPass>(spirv, spirv, true);
}

TEST_F(AggressiveDCETest, NoEliminateForwardPointer) {
  // clang-format off
  //
  //  #version 450
  //  #extension GL_EXT_buffer_reference : enable
  //
  //    // forward reference
  //    layout(buffer_reference) buffer blockType;
  //
  //  layout(buffer_reference, std430, buffer_reference_align = 16) buffer blockType {
  //    int x;
  //    blockType next;
  //  };
  //
  //  layout(std430) buffer rootBlock {
  //    blockType root;
  //  } r;
  //
  //  void main()
  //  {
  //    blockType b = r.root;
  //    b = b.next;
  //    b.x = 531;
  //  }
  //
  // clang-format on

  const std::string predefs1 =
      R"(OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_EXT_physical_storage_buffer"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource GLSL 450
OpSourceExtension "GL_EXT_buffer_reference"
)";

  const std::string names_before =
      R"(OpName %main "main"
OpName %blockType "blockType"
OpMemberName %blockType 0 "x"
OpMemberName %blockType 1 "next"
OpName %b "b"
OpName %rootBlock "rootBlock"
OpMemberName %rootBlock 0 "root"
OpName %r "r"
OpMemberDecorate %blockType 0 Offset 0
OpMemberDecorate %blockType 1 Offset 8
OpDecorate %blockType Block
OpDecorate %b AliasedPointer
OpMemberDecorate %rootBlock 0 Offset 0
OpDecorate %rootBlock Block
OpDecorate %r DescriptorSet 0
OpDecorate %r Binding 0
)";

  const std::string names_after =
      R"(OpName %main "main"
OpName %blockType "blockType"
OpMemberName %blockType 0 "x"
OpMemberName %blockType 1 "next"
OpName %rootBlock "rootBlock"
OpMemberName %rootBlock 0 "root"
OpName %r "r"
OpMemberDecorate %blockType 0 Offset 0
OpMemberDecorate %blockType 1 Offset 8
OpDecorate %blockType Block
OpMemberDecorate %rootBlock 0 Offset 0
OpDecorate %rootBlock Block
OpDecorate %r DescriptorSet 0
OpDecorate %r Binding 0
)";

  const std::string predefs2_before =
      R"(%void = OpTypeVoid
%3 = OpTypeFunction %void
OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_blockType PhysicalStorageBuffer
%int = OpTypeInt 32 1
%blockType = OpTypeStruct %int %_ptr_PhysicalStorageBuffer_blockType
%_ptr_PhysicalStorageBuffer_blockType = OpTypePointer PhysicalStorageBuffer %blockType
%_ptr_Function__ptr_PhysicalStorageBuffer_blockType = OpTypePointer Function %_ptr_PhysicalStorageBuffer_blockType
%rootBlock = OpTypeStruct %_ptr_PhysicalStorageBuffer_blockType
%_ptr_StorageBuffer_rootBlock = OpTypePointer StorageBuffer %rootBlock
%r = OpVariable %_ptr_StorageBuffer_rootBlock StorageBuffer
%int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_blockType = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_blockType
%int_1 = OpConstant %int 1
%_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_blockType = OpTypePointer PhysicalStorageBuffer %_ptr_PhysicalStorageBuffer_blockType
%int_531 = OpConstant %int 531
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
)";

  const std::string predefs2_after =
      R"(%void = OpTypeVoid
%8 = OpTypeFunction %void
OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_blockType PhysicalStorageBuffer
%int = OpTypeInt 32 1
%blockType = OpTypeStruct %int %_ptr_PhysicalStorageBuffer_blockType
%_ptr_PhysicalStorageBuffer_blockType = OpTypePointer PhysicalStorageBuffer %blockType
%rootBlock = OpTypeStruct %_ptr_PhysicalStorageBuffer_blockType
%_ptr_StorageBuffer_rootBlock = OpTypePointer StorageBuffer %rootBlock
%r = OpVariable %_ptr_StorageBuffer_rootBlock StorageBuffer
%int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_blockType = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_blockType
%int_1 = OpConstant %int 1
%_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_blockType = OpTypePointer PhysicalStorageBuffer %_ptr_PhysicalStorageBuffer_blockType
%int_531 = OpConstant %int 531
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%b = OpVariable %_ptr_Function__ptr_PhysicalStorageBuffer_blockType Function
%16 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_blockType %r %int_0
%17 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %16
%21 = OpAccessChain %_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_blockType %17 %int_1
%22 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %21 Aligned 8
OpStore %b %22
%26 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %22 %int_0
OpStore %26 %int_531 Aligned 16
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %8
%19 = OpLabel
%20 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_blockType %r %int_0
%21 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %20
%22 = OpAccessChain %_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_blockType %21 %int_1
%23 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %22 Aligned 8
%24 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %23 %int_0
OpStore %24 %int_531 Aligned 16
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<AggressiveDCEPass>(
      predefs1 + names_before + predefs2_before + func_before,
      predefs1 + names_after + predefs2_after + func_after, true, true);
}

TEST_F(AggressiveDCETest, MultipleFunctionProcessIndependently) {
  const std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %entryHistogram "entryHistogram" %gl_GlobalInvocationID %gl_LocalInvocationIndex
               OpEntryPoint GLCompute %entryAverage "entryAverage" %gl_GlobalInvocationID %gl_LocalInvocationIndex
               OpExecutionMode %entryHistogram LocalSize 16 16 1
               OpExecutionMode %entryAverage LocalSize 256 1 1
               OpSource HLSL 640
               OpName %type_RWStructuredBuffer_uint "type.RWStructuredBuffer.uint"
               OpName %uHistogram "uHistogram"
               OpName %type_ACSBuffer_counter "type.ACSBuffer.counter"
               OpMemberName %type_ACSBuffer_counter 0 "counter"
               OpName %counter_var_uHistogram "counter.var.uHistogram"
               OpName %sharedHistogram "sharedHistogram"
               OpName %entryHistogram "entryHistogram"
               OpName %param_var_id "param.var.id"
               OpName %param_var_idx "param.var.idx"
               OpName %entryAverage "entryAverage"
               OpName %param_var_id_0 "param.var.id"
               OpName %param_var_idx_0 "param.var.idx"
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %gl_LocalInvocationIndex BuiltIn LocalInvocationIndex
               OpDecorate %uHistogram DescriptorSet 0
               OpDecorate %uHistogram Binding 0
               OpDecorate %counter_var_uHistogram DescriptorSet 0
               OpDecorate %counter_var_uHistogram Binding 1
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpMemberDecorate %type_RWStructuredBuffer_uint 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_uint BufferBlock
               OpMemberDecorate %type_ACSBuffer_counter 0 Offset 0
               OpDecorate %type_ACSBuffer_counter BufferBlock
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_4 = OpConstant %uint 4
     %uint_8 = OpConstant %uint 8
    %uint_16 = OpConstant %uint 16
    %uint_32 = OpConstant %uint 32
    %uint_64 = OpConstant %uint 64
   %uint_128 = OpConstant %uint 128
   %uint_256 = OpConstant %uint 256
   %uint_512 = OpConstant %uint 512
   %uint_254 = OpConstant %uint 254
   %uint_255 = OpConstant %uint 255
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_runtimearr_uint = OpTypeRuntimeArray %uint
%type_RWStructuredBuffer_uint = OpTypeStruct %_runtimearr_uint
%_ptr_Uniform_type_RWStructuredBuffer_uint = OpTypePointer Uniform %type_RWStructuredBuffer_uint
%type_ACSBuffer_counter = OpTypeStruct %int
%_ptr_Uniform_type_ACSBuffer_counter = OpTypePointer Uniform %type_ACSBuffer_counter
%_arr_uint_uint_256 = OpTypeArray %uint %uint_256
%_ptr_Workgroup__arr_uint_uint_256 = OpTypePointer Workgroup %_arr_uint_uint_256
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%_ptr_Input_uint = OpTypePointer Input %uint
       %void = OpTypeVoid
         %49 = OpTypeFunction %void
%_ptr_Function_v3uint = OpTypePointer Function %v3uint
%_ptr_Function_uint = OpTypePointer Function %uint
         %52 = OpTypeFunction %void %_ptr_Function_v3uint %_ptr_Function_uint
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
   %uint_264 = OpConstant %uint 264
       %bool = OpTypeBool
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
 %uHistogram = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_uint Uniform
%counter_var_uHistogram = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
%sharedHistogram = OpVariable %_ptr_Workgroup__arr_uint_uint_256 Workgroup
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_LocalInvocationIndex = OpVariable %_ptr_Input_uint Input
%entryHistogram = OpFunction %void None %49
         %57 = OpLabel
%param_var_id = OpVariable %_ptr_Function_v3uint Function
%param_var_idx = OpVariable %_ptr_Function_uint Function
         %58 = OpLoad %v3uint %gl_GlobalInvocationID
         %59 = OpLoad %uint %gl_LocalInvocationIndex
         %79 = OpAccessChain %_ptr_Workgroup_uint %sharedHistogram %int_0
         %80 = OpAtomicIAdd %uint %79 %uint_1 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
%entryAverage = OpFunction %void None %49
         %63 = OpLabel
%param_var_id_0 = OpVariable %_ptr_Function_v3uint Function
%param_var_idx_0 = OpVariable %_ptr_Function_uint Function
         %64 = OpLoad %v3uint %gl_GlobalInvocationID
         %65 = OpLoad %uint %gl_LocalInvocationIndex
               OpStore %param_var_idx_0 %65
         %83 = OpAccessChain %_ptr_Workgroup_uint %sharedHistogram %65
               OpStore %83 %uint_0

; CHECK:      [[ieq:%\w+]] = OpIEqual
; CHECK-NEXT: OpSelectionMerge [[merge:%\w+]]
; CHECK-NEXT: OpBranchConditional [[ieq]] [[not_elim:%\w+]] [[merge]]
; CHECK-NEXT: [[not_elim]] = OpLabel
; CHECK:      [[merge]] = OpLabel

               OpControlBarrier %uint_2 %uint_2 %uint_264
         %85 = OpIEqual %bool %65 %uint_0
               OpSelectionMerge %89 None
               OpBranchConditional %85 %86 %89
         %86 = OpLabel
         %88 = OpAccessChain %_ptr_Workgroup_uint %sharedHistogram %65
               OpStore %88 %uint_1
               OpBranch %89
         %89 = OpLabel
               OpControlBarrier %uint_2 %uint_2 %uint_264
         %91 = OpAccessChain %_ptr_Workgroup_uint %sharedHistogram %65
         %92 = OpLoad %uint %91
         %94 = OpAccessChain %_ptr_Uniform_uint %uHistogram %int_0 %65
               OpStore %94 %92
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_3);

  SinglePassRunAndMatch<AggressiveDCEPass>(spirv, true);
}

TEST_F(AggressiveDCETest, DebugInfoKeepInFunctionElimStoreVar) {
  // Verify that dead local variable tc and store eliminated but all
  // in-function debuginfo kept.
  //
  // The SPIR-V has been inlined and local single store eliminated
  //
  // Texture2D g_tColor;
  // SamplerState g_sAniso;
  //
  // struct PS_INPUT {
  //   float2 vTextureCoords : TEXCOORD2;
  // };
  //
  // struct PS_OUTPUT {
  //   float4 vColor : SV_Target0;
  // };
  //
  // PS_OUTPUT MainPs(PS_INPUT i) {
  //   PS_OUTPUT ps_output;
  //   float2 tc = i.vTextureCoords.xy;
  //   ps_output.vColor = g_tColor.Sample(g_sAniso, tc);
  //   return ps_output;
  // }

  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %g_tColor %g_sAniso %in_var_TEXCOORD2 %out_var_SV_Target0
               OpExecutionMode %MainPs OriginUpperLeft
          %7 = OpString "foo.frag"
          %8 = OpString "PS_OUTPUT"
          %9 = OpString "float"
         %10 = OpString "vColor"
         %11 = OpString "PS_INPUT"
         %12 = OpString "vTextureCoords"
         %13 = OpString "@type.2d.image"
         %14 = OpString "type.2d.image"
         %15 = OpString "Texture2D.TemplateParam"
         %16 = OpString "src.MainPs"
         %17 = OpString "tc"
         %18 = OpString "ps_output"
         %19 = OpString "i"
         %20 = OpString "@type.sampler"
         %21 = OpString "type.sampler"
         %22 = OpString "g_sAniso"
         %23 = OpString "g_tColor"
               OpName %type_2d_image "type.2d.image"
               OpName %g_tColor "g_tColor"
               OpName %type_sampler "type.sampler"
               OpName %g_sAniso "g_sAniso"
               OpName %in_var_TEXCOORD2 "in.var.TEXCOORD2"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPs "MainPs"
               OpName %PS_INPUT "PS_INPUT"
               OpMemberName %PS_INPUT 0 "vTextureCoords"
               OpName %param_var_i "param.var.i"
               OpName %PS_OUTPUT "PS_OUTPUT"
               OpMemberName %PS_OUTPUT 0 "vColor"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_TEXCOORD2 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %g_tColor DescriptorSet 0
               OpDecorate %g_tColor Binding 0
               OpDecorate %g_sAniso DescriptorSet 0
               OpDecorate %g_sAniso Binding 1
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
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
         %45 = OpTypeFunction %void
   %PS_INPUT = OpTypeStruct %v2float
%_ptr_Function_PS_INPUT = OpTypePointer Function %PS_INPUT
  %PS_OUTPUT = OpTypeStruct %v4float
         %47 = OpTypeFunction %PS_OUTPUT %_ptr_Function_PS_INPUT
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%_ptr_Function_v2float = OpTypePointer Function %v2float
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_Function_v4float = OpTypePointer Function %v4float
   %g_tColor = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
   %g_sAniso = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD2 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %51 = OpExtInst %void %1 DebugInfoNone
         %52 = OpExtInst %void %1 DebugExpression
         %53 = OpExtInst %void %1 DebugOperation Deref
         %54 = OpExtInst %void %1 DebugExpression %53
         %55 = OpExtInst %void %1 DebugSource %7
         %56 = OpExtInst %void %1 DebugCompilationUnit 1 4 %55 HLSL
         %57 = OpExtInst %void %1 DebugTypeComposite %8 Structure %55 10 1 %56 %8 %uint_128 FlagIsProtected|FlagIsPrivate %58
         %59 = OpExtInst %void %1 DebugTypeBasic %9 %uint_32 Float
         %60 = OpExtInst %void %1 DebugTypeVector %59 4
         %58 = OpExtInst %void %1 DebugTypeMember %10 %60 %55 12 5 %57 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %61 = OpExtInst %void %1 DebugTypeComposite %11 Structure %55 5 1 %56 %11 %uint_64 FlagIsProtected|FlagIsPrivate %62
         %63 = OpExtInst %void %1 DebugTypeVector %59 2
         %62 = OpExtInst %void %1 DebugTypeMember %12 %63 %55 7 5 %61 %uint_0 %uint_64 FlagIsProtected|FlagIsPrivate
         %64 = OpExtInst %void %1 DebugTypeComposite %13 Class %55 0 0 %56 %14 %51 FlagIsProtected|FlagIsPrivate
         %65 = OpExtInst %void %1 DebugTypeTemplateParameter %15 %59 %51 %55 0 0
         %66 = OpExtInst %void %1 DebugTypeTemplate %64 %65
         %67 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %57 %61
         %68 = OpExtInst %void %1 DebugFunction %16 %67 %55 15 1 %56 %16 FlagIsProtected|FlagIsPrivate 16 %51
         %69 = OpExtInst %void %1 DebugLexicalBlock %55 16 1 %68
         %70 = OpExtInst %void %1 DebugLocalVariable %17 %63 %55 19 12 %69 FlagIsLocal
         %71 = OpExtInst %void %1 DebugLocalVariable %18 %57 %55 17 15 %69 FlagIsLocal
         %72 = OpExtInst %void %1 DebugLocalVariable %19 %61 %55 15 29 %68 FlagIsLocal 1
         %73 = OpExtInst %void %1 DebugTypeComposite %20 Structure %55 0 0 %56 %21 %51 FlagIsProtected|FlagIsPrivate
         %74 = OpExtInst %void %1 DebugGlobalVariable %22 %73 %55 3 14 %56 %22 %g_sAniso FlagIsDefinition
         %75 = OpExtInst %void %1 DebugGlobalVariable %23 %64 %55 1 11 %56 %23 %g_tColor FlagIsDefinition
     %MainPs = OpFunction %void None %45
         %76 = OpLabel
        %107 = OpExtInst %void %1 DebugScope %69
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugScope %69
         %78 = OpVariable %_ptr_Function_PS_OUTPUT Function
         %79 = OpVariable %_ptr_Function_v2float Function
        %108 = OpExtInst %void %1 DebugNoScope
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugNoScope
         %81 = OpVariable %_ptr_Function_PS_OUTPUT Function
%param_var_i = OpVariable %_ptr_Function_PS_INPUT Function
         %82 = OpLoad %v2float %in_var_TEXCOORD2
         %83 = OpCompositeConstruct %PS_INPUT %82
               OpStore %param_var_i %83
        %109 = OpExtInst %void %1 DebugScope %68
         %85 = OpExtInst %void %1 DebugDeclare %72 %param_var_i %52
        %110 = OpExtInst %void %1 DebugScope %69
         %87 = OpExtInst %void %1 DebugDeclare %71 %78 %52
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugScope %68
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugDeclare %72 %param_var_i %52
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugScope %69
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugDeclare %71 %78 %52
               OpLine %7 19 17
         %88 = OpAccessChain %_ptr_Function_v2float %param_var_i %int_0
         %89 = OpLoad %v2float %88
               OpLine %7 19 12
               OpStore %79 %89
;CHECK-NOT:    OpStore %79 %89
               OpLine %7 19 12
        %106 = OpExtInst %void %1 DebugValue %70 %89 %52
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugValue %70 %89 %52
               OpLine %7 20 26
         %91 = OpLoad %type_2d_image %g_tColor
               OpLine %7 20 46
         %92 = OpLoad %type_sampler %g_sAniso
               OpLine %7 20 26
         %94 = OpSampledImage %type_sampled_image %91 %92
         %95 = OpImageSampleImplicitLod %v4float %94 %89 None
               OpLine %7 20 5
         %96 = OpAccessChain %_ptr_Function_v4float %78 %int_0
               OpStore %96 %95
               OpLine %7 21 12
         %97 = OpLoad %PS_OUTPUT %78
               OpLine %7 21 5
               OpStore %81 %97
        %111 = OpExtInst %void %1 DebugNoScope
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugNoScope
        %100 = OpCompositeExtract %v4float %97 0
               OpStore %out_var_SV_Target0 %100
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, ShaderDebugInfoKeepInFunctionElimStoreVar) {
  // Verify that dead local variable tc and store eliminated but all
  // in-function NonSemantic Shader debuginfo kept.

  const std::string text = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %g_tColor %g_sAniso %in_var_TEXCOORD2 %out_var_SV_Target0
               OpExecutionMode %MainPs OriginUpperLeft
          %7 = OpString "foo.frag"
          %8 = OpString "PS_OUTPUT"
          %9 = OpString "float"
         %10 = OpString "vColor"
         %11 = OpString "PS_INPUT"
         %12 = OpString "vTextureCoords"
         %13 = OpString "@type.2d.image"
         %14 = OpString "type.2d.image"
         %15 = OpString "Texture2D.TemplateParam"
         %16 = OpString "src.MainPs"
         %17 = OpString "tc"
         %18 = OpString "ps_output"
         %19 = OpString "i"
         %20 = OpString "@type.sampler"
         %21 = OpString "type.sampler"
         %22 = OpString "g_sAniso"
         %23 = OpString "g_tColor"
               OpName %type_2d_image "type.2d.image"
               OpName %g_tColor "g_tColor"
               OpName %type_sampler "type.sampler"
               OpName %g_sAniso "g_sAniso"
               OpName %in_var_TEXCOORD2 "in.var.TEXCOORD2"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPs "MainPs"
               OpName %PS_INPUT "PS_INPUT"
               OpMemberName %PS_INPUT 0 "vTextureCoords"
               OpName %param_var_i "param.var.i"
               OpName %PS_OUTPUT "PS_OUTPUT"
               OpMemberName %PS_OUTPUT 0 "vColor"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_TEXCOORD2 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %g_tColor DescriptorSet 0
               OpDecorate %g_tColor Binding 0
               OpDecorate %g_sAniso DescriptorSet 0
               OpDecorate %g_sAniso Binding 1
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
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
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %uint_4 = OpConstant %uint 4
     %uint_5 = OpConstant %uint 5
     %uint_7 = OpConstant %uint 7
     %uint_8 = OpConstant %uint 8
    %uint_10 = OpConstant %uint 10
    %uint_11 = OpConstant %uint 11
    %uint_12 = OpConstant %uint 12
    %uint_14 = OpConstant %uint 14
    %uint_15 = OpConstant %uint 15
    %uint_16 = OpConstant %uint 16
    %uint_17 = OpConstant %uint 17
    %uint_19 = OpConstant %uint 19
    %uint_20 = OpConstant %uint 20
    %uint_21 = OpConstant %uint 21
    %uint_25 = OpConstant %uint 25
    %uint_29 = OpConstant %uint 29
    %uint_30 = OpConstant %uint 30
    %uint_35 = OpConstant %uint 35
    %uint_41 = OpConstant %uint 41
    %uint_48 = OpConstant %uint 48
    %uint_53 = OpConstant %uint 53
    %uint_64 = OpConstant %uint 64
         %45 = OpTypeFunction %void
   %PS_INPUT = OpTypeStruct %v2float
%_ptr_Function_PS_INPUT = OpTypePointer Function %PS_INPUT
  %PS_OUTPUT = OpTypeStruct %v4float
         %47 = OpTypeFunction %PS_OUTPUT %_ptr_Function_PS_INPUT
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%_ptr_Function_v2float = OpTypePointer Function %v2float
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_Function_v4float = OpTypePointer Function %v4float
   %g_tColor = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
   %g_sAniso = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD2 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %51 = OpExtInst %void %1 DebugInfoNone
         %52 = OpExtInst %void %1 DebugExpression
         %53 = OpExtInst %void %1 DebugOperation %uint_0
         %54 = OpExtInst %void %1 DebugExpression %53
         %55 = OpExtInst %void %1 DebugSource %7
         %56 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %55 %uint_5
         %59 = OpExtInst %void %1 DebugTypeBasic %9 %uint_32 %uint_3 %uint_0
         %60 = OpExtInst %void %1 DebugTypeVector %59 %uint_4
         %58 = OpExtInst %void %1 DebugTypeMember %10 %60 %55 %uint_12 %uint_5 %uint_0 %uint_128 %uint_3
         %57 = OpExtInst %void %1 DebugTypeComposite %8 %uint_1 %55 %uint_10 %uint_1 %56 %8 %uint_128 %uint_3 %58
         %63 = OpExtInst %void %1 DebugTypeVector %59 %uint_2 
         %62 = OpExtInst %void %1 DebugTypeMember %12 %63 %55 %uint_7 %uint_5 %uint_0 %uint_64 %uint_3
         %61 = OpExtInst %void %1 DebugTypeComposite %11 %uint_1 %55 %uint_5 %uint_1 %56 %11 %uint_64 %uint_3 %62
         %64 = OpExtInst %void %1 DebugTypeComposite %13 %uint_0 %55 %uint_0 %uint_0 %56 %14 %51 %uint_3
         %67 = OpExtInst %void %1 DebugTypeFunction %uint_3 %57 %61
         %68 = OpExtInst %void %1 DebugFunction %16 %67 %55 %uint_15 %uint_1 %56 %16 %uint_3 %uint_16
         %69 = OpExtInst %void %1 DebugLexicalBlock %55 %uint_16 %uint_1 %68
         %70 = OpExtInst %void %1 DebugLocalVariable %17 %63 %55 %uint_19 %uint_12 %69 %uint_4
         %71 = OpExtInst %void %1 DebugLocalVariable %18 %57 %55 %uint_17 %uint_15 %69 %uint_4
         %72 = OpExtInst %void %1 DebugLocalVariable %19 %61 %55 %uint_15 %uint_29 %68 %uint_4 %uint_1
         %73 = OpExtInst %void %1 DebugTypeComposite %20 %uint_1 %55 %uint_0 %uint_0 %56 %21 %51 %uint_3
         %74 = OpExtInst %void %1 DebugGlobalVariable %22 %73 %55 %uint_3 %uint_14 %56 %22 %g_sAniso %uint_8
         %75 = OpExtInst %void %1 DebugGlobalVariable %23 %64 %55 %uint_1 %uint_11 %56 %23 %g_tColor %uint_8
     %MainPs = OpFunction %void None %45
         %76 = OpLabel
         %78 = OpVariable %_ptr_Function_PS_OUTPUT Function
         %79 = OpVariable %_ptr_Function_v2float Function
         %81 = OpVariable %_ptr_Function_PS_OUTPUT Function
%param_var_i = OpVariable %_ptr_Function_PS_INPUT Function
         %82 = OpLoad %v2float %in_var_TEXCOORD2
         %83 = OpCompositeConstruct %PS_INPUT %82
               OpStore %param_var_i %83
        %112 = OpExtInst %void %1 DebugFunctionDefinition %68 %MainPs
        %109 = OpExtInst %void %1 DebugScope %68
         %85 = OpExtInst %void %1 DebugDeclare %72 %param_var_i %52
        %110 = OpExtInst %void %1 DebugScope %69
         %87 = OpExtInst %void %1 DebugDeclare %71 %78 %52
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugFunctionDefinition %68 %MainPs
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugScope %68
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugDeclare %72 %param_var_i %52
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugScope %69
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugDeclare %71 %78 %52
        %300 = OpExtInst %void %1 DebugLine %55 %uint_19 %uint_19 %uint_17 %uint_30
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugLine %55 %uint_19 %uint_19 %uint_17 %uint_30
         %88 = OpAccessChain %_ptr_Function_v2float %param_var_i %int_0
         %89 = OpLoad %v2float %88
        %301 = OpExtInst %void %1 DebugLine %55 %uint_19 %uint_19 %uint_12 %uint_35
               OpStore %79 %89
;CHECK-NOT:    OpStore %79 %89
        %302 = OpExtInst %void %1 DebugLine %55 %uint_19 %uint_19 %uint_12 %uint_35
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugLine %55 %uint_19 %uint_19 %uint_12 %uint_35
        %106 = OpExtInst %void %1 DebugValue %70 %89 %52
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugValue %70 %89 %52
        %303 = OpExtInst %void %1 DebugLine %55 %uint_20 %uint_20 %uint_25 %uint_32
         %91 = OpLoad %type_2d_image %g_tColor
        %304 = OpExtInst %void %1 DebugLine %55 %uint_20 %uint_20 %uint_41 %uint_48
         %92 = OpLoad %type_sampler %g_sAniso
        %305 = OpExtInst %void %1 DebugLine %55 %uint_20 %uint_20 %uint_25 %uint_53
         %94 = OpSampledImage %type_sampled_image %91 %92
         %95 = OpImageSampleImplicitLod %v4float %94 %89 None
        %306 = OpExtInst %void %1 DebugLine %55 %uint_20 %uint_20 %uint_5 %uint_53
         %96 = OpAccessChain %_ptr_Function_v4float %78 %int_0
               OpStore %96 %95
        %307 = OpExtInst %void %1 DebugLine %55 %uint_21 %uint_21 %uint_12 %uint_20
         %97 = OpLoad %PS_OUTPUT %78
        %308 = OpExtInst %void %1 DebugLine %55 %uint_21 %uint_21 %uint_5 %uint_20
               OpStore %81 %97
        %309 = OpExtInst %void %1 DebugNoLine
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugNoLine
        %111 = OpExtInst %void %1 DebugNoScope
;CHECK: {{%\w+}} = OpExtInst %void %1 DebugNoScope
        %100 = OpCompositeExtract %v4float %97 0
               OpStore %out_var_SV_Target0 %100
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, ShaderDebugInfoGlobalDCE) {
  // Verify that DebugGlobalVariable for eliminated private variable has
  // variable operand replaced with DebugInfoNone.

  const std::string text = R"(OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %MainPs "MainPs" %out_var_SV_Target0 %a
OpExecutionMode %MainPs OriginUpperLeft
%5 = OpString "source2.hlsl"
%24 = OpString "float"
%29 = OpString "vColor"
%33 = OpString "PS_OUTPUT"
%37 = OpString "MainPs"
%38 = OpString ""
%42 = OpString "ps_output"
%46 = OpString "a"
OpName %a "a"
OpName %out_var_SV_Target0 "out.var.SV_Target0"
OpName %MainPs "MainPs"
OpName %PS_OUTPUT "PS_OUTPUT"
OpMemberName %PS_OUTPUT 0 "vColor"
OpDecorate %out_var_SV_Target0 Location 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%8 = OpConstantNull %v4float
%float_0 = OpConstant %float 0
%10 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_ptr_Private_v4float = OpTypePointer Private %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%uint_1 = OpConstant %uint 1
%uint_4 = OpConstant %uint 4
%uint_5 = OpConstant %uint 5
%uint_3 = OpConstant %uint 3
%uint_0 = OpConstant %uint 0
%uint_128 = OpConstant %uint 128
%uint_12 = OpConstant %uint 12
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_10 = OpConstant %uint 10
%uint_15 = OpConstant %uint 15
%48 = OpTypeFunction %void
%PS_OUTPUT = OpTypeStruct %v4float
%54 = OpTypeFunction %PS_OUTPUT
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%_ptr_Function_v4float = OpTypePointer Function %v4float
%a = OpVariable %_ptr_Private_v4float Private
;CHECK-NOT: %a = OpVariable %_ptr_Private_v4float Private
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
;CHECK: [[dbg_none:%\w+]] = OpExtInst %void %1 DebugInfoNone
%18 = OpExtInst %void %1 DebugExpression
%19 = OpExtInst %void %1 DebugSource %5
%20 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %19 %uint_5
%25 = OpExtInst %void %1 DebugTypeBasic %24 %uint_32 %uint_3 %uint_0
%28 = OpExtInst %void %1 DebugTypeVector %25 %uint_4
%31 = OpExtInst %void %1 DebugTypeMember %29 %28 %19 %uint_5 %uint_12 %uint_0 %uint_128 %uint_3
%34 = OpExtInst %void %1 DebugTypeComposite %33 %uint_1 %19 %uint_3 %uint_8 %20 %33 %uint_128 %uint_3 %31
%36 = OpExtInst %void %1 DebugTypeFunction %uint_3 %34
%39 = OpExtInst %void %1 DebugFunction %37 %36 %19 %uint_8 %uint_1 %20 %38 %uint_3 %uint_9
%41 = OpExtInst %void %1 DebugLexicalBlock %19 %uint_9 %uint_1 %39
%43 = OpExtInst %void %1 DebugLocalVariable %42 %34 %19 %uint_10 %uint_15 %41 %uint_4
%47 = OpExtInst %void %1 DebugGlobalVariable %46 %28 %19 %uint_1 %uint_15 %20 %46 %a %uint_8
;CHECK: %47 = OpExtInst %void %1 DebugGlobalVariable %46 %28 %19 %uint_1 %uint_15 %20 %46 [[dbg_none]] %uint_8
%MainPs = OpFunction %void None %48
%49 = OpLabel
%65 = OpVariable %_ptr_Function_PS_OUTPUT Function
%66 = OpVariable %_ptr_Function_PS_OUTPUT Function
OpStore %a %8
%72 = OpExtInst %void %1 DebugScope %41
%69 = OpExtInst %void %1 DebugDeclare %43 %65 %18
OpLine %5 11 5
%70 = OpAccessChain %_ptr_Function_v4float %65 %int_0
OpStore %70 %10
OpLine %5 12 12
%71 = OpLoad %PS_OUTPUT %65
OpLine %5 12 5
OpStore %66 %71
%73 = OpExtInst %void %1 DebugNoLine
%74 = OpExtInst %void %1 DebugNoScope
%51 = OpLoad %PS_OUTPUT %66
%53 = OpCompositeExtract %v4float %51 0
OpStore %out_var_SV_Target0 %53
OpLine %5 13 1
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, DebugInfoDeclareKeepsStore) {
  // Verify that local variable tc and its store are kept by DebugDeclare.
  //
  // Same shader source as DebugInfoInFunctionKeepStoreVarElim. The SPIR-V
  // has just been inlined.

  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %g_tColor %g_sAniso %in_var_TEXCOORD2 %out_var_SV_Target0
               OpExecutionMode %MainPs OriginUpperLeft
         %20 = OpString "foo.frag"
         %24 = OpString "PS_OUTPUT"
         %28 = OpString "float"
         %31 = OpString "vColor"
         %33 = OpString "PS_INPUT"
         %38 = OpString "vTextureCoords"
         %40 = OpString "@type.2d.image"
         %41 = OpString "type.2d.image"
         %43 = OpString "Texture2D.TemplateParam"
         %47 = OpString "src.MainPs"
         %51 = OpString "tc"
         %53 = OpString "ps_output"
         %56 = OpString "i"
         %58 = OpString "@type.sampler"
         %59 = OpString "type.sampler"
         %61 = OpString "g_sAniso"
         %63 = OpString "g_tColor"
               OpName %type_2d_image "type.2d.image"
               OpName %g_tColor "g_tColor"
               OpName %type_sampler "type.sampler"
               OpName %g_sAniso "g_sAniso"
               OpName %in_var_TEXCOORD2 "in.var.TEXCOORD2"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPs "MainPs"
               OpName %PS_INPUT "PS_INPUT"
               OpMemberName %PS_INPUT 0 "vTextureCoords"
               OpName %param_var_i "param.var.i"
               OpName %PS_OUTPUT "PS_OUTPUT"
               OpMemberName %PS_OUTPUT 0 "vColor"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_TEXCOORD2 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %g_tColor DescriptorSet 0
               OpDecorate %g_tColor Binding 0
               OpDecorate %g_sAniso DescriptorSet 0
               OpDecorate %g_sAniso Binding 1
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
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
         %65 = OpTypeFunction %void
   %PS_INPUT = OpTypeStruct %v2float
%_ptr_Function_PS_INPUT = OpTypePointer Function %PS_INPUT
  %PS_OUTPUT = OpTypeStruct %v4float
         %75 = OpTypeFunction %PS_OUTPUT %_ptr_Function_PS_INPUT
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%_ptr_Function_v2float = OpTypePointer Function %v2float
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_Function_v4float = OpTypePointer Function %v4float
   %g_tColor = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
   %g_sAniso = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD2 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %39 = OpExtInst %void %1 DebugInfoNone
         %55 = OpExtInst %void %1 DebugExpression
         %22 = OpExtInst %void %1 DebugSource %20
         %23 = OpExtInst %void %1 DebugCompilationUnit 1 4 %22 HLSL
         %26 = OpExtInst %void %1 DebugTypeComposite %24 Structure %22 10 1 %23 %24 %uint_128 FlagIsProtected|FlagIsPrivate %27
         %29 = OpExtInst %void %1 DebugTypeBasic %28 %uint_32 Float
         %30 = OpExtInst %void %1 DebugTypeVector %29 4
         %27 = OpExtInst %void %1 DebugTypeMember %31 %30 %22 12 5 %26 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %35 = OpExtInst %void %1 DebugTypeComposite %33 Structure %22 5 1 %23 %33 %uint_64 FlagIsProtected|FlagIsPrivate %36
         %37 = OpExtInst %void %1 DebugTypeVector %29 2
         %36 = OpExtInst %void %1 DebugTypeMember %38 %37 %22 7 5 %35 %uint_0 %uint_64 FlagIsProtected|FlagIsPrivate
         %42 = OpExtInst %void %1 DebugTypeComposite %40 Class %22 0 0 %23 %41 %39 FlagIsProtected|FlagIsPrivate
         %44 = OpExtInst %void %1 DebugTypeTemplateParameter %43 %29 %39 %22 0 0
         %45 = OpExtInst %void %1 DebugTypeTemplate %42 %44
         %46 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %26 %35
         %48 = OpExtInst %void %1 DebugFunction %47 %46 %22 15 1 %23 %47 FlagIsProtected|FlagIsPrivate 16 %39
         %50 = OpExtInst %void %1 DebugLexicalBlock %22 16 1 %48
         %52 = OpExtInst %void %1 DebugLocalVariable %51 %37 %22 19 12 %50 FlagIsLocal
         %54 = OpExtInst %void %1 DebugLocalVariable %53 %26 %22 17 15 %50 FlagIsLocal
         %57 = OpExtInst %void %1 DebugLocalVariable %56 %35 %22 15 29 %48 FlagIsLocal 1
         %60 = OpExtInst %void %1 DebugTypeComposite %58 Structure %22 0 0 %23 %59 %39 FlagIsProtected|FlagIsPrivate
         %62 = OpExtInst %void %1 DebugGlobalVariable %61 %60 %22 3 14 %23 %61 %g_sAniso FlagIsDefinition
         %64 = OpExtInst %void %1 DebugGlobalVariable %63 %42 %22 1 11 %23 %63 %g_tColor FlagIsDefinition
     %MainPs = OpFunction %void None %65
         %66 = OpLabel
        %114 = OpExtInst %void %1 DebugScope %50
         %98 = OpVariable %_ptr_Function_PS_OUTPUT Function
         %99 = OpVariable %_ptr_Function_v2float Function
        %115 = OpExtInst %void %1 DebugNoScope
        %100 = OpVariable %_ptr_Function_PS_OUTPUT Function
%param_var_i = OpVariable %_ptr_Function_PS_INPUT Function
         %70 = OpLoad %v2float %in_var_TEXCOORD2
         %71 = OpCompositeConstruct %PS_INPUT %70
               OpStore %param_var_i %71
        %116 = OpExtInst %void %1 DebugScope %48
        %102 = OpExtInst %void %1 DebugDeclare %57 %param_var_i %55
        %117 = OpExtInst %void %1 DebugScope %50
        %103 = OpExtInst %void %1 DebugDeclare %54 %98 %55
               OpLine %20 19 17
        %104 = OpAccessChain %_ptr_Function_v2float %param_var_i %int_0
        %105 = OpLoad %v2float %104
               OpLine %20 19 12
               OpStore %99 %105
;CHECK:        OpStore %99 %105
        %106 = OpExtInst %void %1 DebugDeclare %52 %99 %55
               OpLine %20 20 26
        %107 = OpLoad %type_2d_image %g_tColor
               OpLine %20 20 46
        %108 = OpLoad %type_sampler %g_sAniso
               OpLine %20 20 26
        %110 = OpSampledImage %type_sampled_image %107 %108
        %111 = OpImageSampleImplicitLod %v4float %110 %105 None
               OpLine %20 20 5
        %112 = OpAccessChain %_ptr_Function_v4float %98 %int_0
               OpStore %112 %111
               OpLine %20 21 12
        %113 = OpLoad %PS_OUTPUT %98
               OpLine %20 21 5
               OpStore %100 %113
        %118 = OpExtInst %void %1 DebugNoScope
         %73 = OpLoad %PS_OUTPUT %100
         %74 = OpCompositeExtract %v4float %73 0
               OpStore %out_var_SV_Target0 %74
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, DebugInfoValueDerefKeepsStore) {
  // Verify that local variable tc and its store are kept by DebugValue with
  // Deref.
  //
  // Same shader source as DebugInfoInFunctionKeepStoreVarElim. The SPIR-V
  // has just been inlined and edited to replace the DebugDeclare with the
  // DebugValue/Deref.

  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %g_tColor %g_sAniso %in_var_TEXCOORD2 %out_var_SV_Target0
               OpExecutionMode %MainPs OriginUpperLeft
          %7 = OpString "foo.frag"
          %8 = OpString "PS_OUTPUT"
          %9 = OpString "float"
         %10 = OpString "vColor"
         %11 = OpString "PS_INPUT"
         %12 = OpString "vTextureCoords"
         %13 = OpString "@type.2d.image"
         %14 = OpString "type.2d.image"
         %15 = OpString "Texture2D.TemplateParam"
         %16 = OpString "src.MainPs"
         %17 = OpString "tc"
         %18 = OpString "ps_output"
         %19 = OpString "i"
         %20 = OpString "@type.sampler"
         %21 = OpString "type.sampler"
         %22 = OpString "g_sAniso"
         %23 = OpString "g_tColor"
               OpName %type_2d_image "type.2d.image"
               OpName %g_tColor "g_tColor"
               OpName %type_sampler "type.sampler"
               OpName %g_sAniso "g_sAniso"
               OpName %in_var_TEXCOORD2 "in.var.TEXCOORD2"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPs "MainPs"
               OpName %PS_INPUT "PS_INPUT"
               OpMemberName %PS_INPUT 0 "vTextureCoords"
               OpName %param_var_i "param.var.i"
               OpName %PS_OUTPUT "PS_OUTPUT"
               OpMemberName %PS_OUTPUT 0 "vColor"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_TEXCOORD2 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %g_tColor DescriptorSet 0
               OpDecorate %g_tColor Binding 0
               OpDecorate %g_sAniso DescriptorSet 0
               OpDecorate %g_sAniso Binding 1
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
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
         %45 = OpTypeFunction %void
   %PS_INPUT = OpTypeStruct %v2float
%_ptr_Function_PS_INPUT = OpTypePointer Function %PS_INPUT
  %PS_OUTPUT = OpTypeStruct %v4float
         %47 = OpTypeFunction %PS_OUTPUT %_ptr_Function_PS_INPUT
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%_ptr_Function_v2float = OpTypePointer Function %v2float
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_Function_v4float = OpTypePointer Function %v4float
   %g_tColor = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
   %g_sAniso = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD2 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %51 = OpExtInst %void %1 DebugInfoNone
         %52 = OpExtInst %void %1 DebugExpression
         %53 = OpExtInst %void %1 DebugOperation Deref
         %54 = OpExtInst %void %1 DebugExpression %53
         %55 = OpExtInst %void %1 DebugSource %7
         %56 = OpExtInst %void %1 DebugCompilationUnit 1 4 %55 HLSL
         %57 = OpExtInst %void %1 DebugTypeComposite %8 Structure %55 10 1 %56 %8 %uint_128 FlagIsProtected|FlagIsPrivate %58
         %59 = OpExtInst %void %1 DebugTypeBasic %9 %uint_32 Float
         %60 = OpExtInst %void %1 DebugTypeVector %59 4
         %58 = OpExtInst %void %1 DebugTypeMember %10 %60 %55 12 5 %57 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %61 = OpExtInst %void %1 DebugTypeComposite %11 Structure %55 5 1 %56 %11 %uint_64 FlagIsProtected|FlagIsPrivate %62
         %63 = OpExtInst %void %1 DebugTypeVector %59 2
         %62 = OpExtInst %void %1 DebugTypeMember %12 %63 %55 7 5 %61 %uint_0 %uint_64 FlagIsProtected|FlagIsPrivate
         %64 = OpExtInst %void %1 DebugTypeComposite %13 Class %55 0 0 %56 %14 %51 FlagIsProtected|FlagIsPrivate
         %65 = OpExtInst %void %1 DebugTypeTemplateParameter %15 %59 %51 %55 0 0
         %66 = OpExtInst %void %1 DebugTypeTemplate %64 %65
         %67 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %57 %61
         %68 = OpExtInst %void %1 DebugFunction %16 %67 %55 15 1 %56 %16 FlagIsProtected|FlagIsPrivate 16 %51
         %69 = OpExtInst %void %1 DebugLexicalBlock %55 16 1 %68
         %70 = OpExtInst %void %1 DebugLocalVariable %17 %63 %55 19 12 %69 FlagIsLocal
         %71 = OpExtInst %void %1 DebugLocalVariable %18 %57 %55 17 15 %69 FlagIsLocal
         %72 = OpExtInst %void %1 DebugLocalVariable %19 %61 %55 15 29 %68 FlagIsLocal 1
         %73 = OpExtInst %void %1 DebugTypeComposite %20 Structure %55 0 0 %56 %21 %51 FlagIsProtected|FlagIsPrivate
         %74 = OpExtInst %void %1 DebugGlobalVariable %22 %73 %55 3 14 %56 %22 %g_sAniso FlagIsDefinition
         %75 = OpExtInst %void %1 DebugGlobalVariable %23 %64 %55 1 11 %56 %23 %g_tColor FlagIsDefinition
     %MainPs = OpFunction %void None %45
         %76 = OpLabel
        %101 = OpExtInst %void %1 DebugScope %69
         %78 = OpVariable %_ptr_Function_PS_OUTPUT Function
         %79 = OpVariable %_ptr_Function_v2float Function
        %102 = OpExtInst %void %1 DebugNoScope
         %81 = OpVariable %_ptr_Function_PS_OUTPUT Function
%param_var_i = OpVariable %_ptr_Function_PS_INPUT Function
         %82 = OpLoad %v2float %in_var_TEXCOORD2
         %83 = OpCompositeConstruct %PS_INPUT %82
               OpStore %param_var_i %83
        %103 = OpExtInst %void %1 DebugScope %68
         %85 = OpExtInst %void %1 DebugDeclare %72 %param_var_i %52
        %104 = OpExtInst %void %1 DebugScope %69
         %87 = OpExtInst %void %1 DebugDeclare %71 %78 %52
               OpLine %7 19 17
         %88 = OpAccessChain %_ptr_Function_v2float %param_var_i %int_0
         %89 = OpLoad %v2float %88
               OpLine %7 19 12
               OpStore %79 %89
;CHECK:        OpStore %79 %89
         %90 = OpExtInst %void %1 DebugValue %70 %79 %54
               OpLine %7 20 26
         %91 = OpLoad %type_2d_image %g_tColor
               OpLine %7 20 46
         %92 = OpLoad %type_sampler %g_sAniso
               OpLine %7 20 26
         %94 = OpSampledImage %type_sampled_image %91 %92
         %95 = OpImageSampleImplicitLod %v4float %94 %89 None
               OpLine %7 20 5
         %96 = OpAccessChain %_ptr_Function_v4float %78 %int_0
               OpStore %96 %95
               OpLine %7 21 12
         %97 = OpLoad %PS_OUTPUT %78
               OpLine %7 21 5
               OpStore %81 %97
        %105 = OpExtInst %void %1 DebugNoScope
         %99 = OpLoad %PS_OUTPUT %81
        %100 = OpCompositeExtract %v4float %99 0
               OpStore %out_var_SV_Target0 %100
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, DebugInfoElimUnusedTextureKeepGlobalVariable) {
  // Verify that unused texture g_tColor2 is eliminated but its
  // DebugGlobalVariable is retained but with DebugInfoNone for its Variable.
  //
  // Same shader source as DebugInfoInFunctionKeepStoreVarElim but with unused
  // g_tColor2 added.

  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %g_tColor %g_tColor2 %g_sAniso %in_var_TEXCOORD2 %out_var_SV_Target0
               OpExecutionMode %MainPs OriginUpperLeft
         %21 = OpString "foo6.frag"
         %25 = OpString "PS_OUTPUT"
         %29 = OpString "float"
         %32 = OpString "vColor"
         %34 = OpString "PS_INPUT"
         %39 = OpString "vTextureCoords"
         %41 = OpString "@type.2d.image"
         %42 = OpString "type.2d.image"
         %44 = OpString "Texture2D.TemplateParam"
         %48 = OpString "src.MainPs"
         %52 = OpString "tc"
         %54 = OpString "ps_output"
         %57 = OpString "i"
         %59 = OpString "@type.sampler"
         %60 = OpString "type.sampler"
         %62 = OpString "g_sAniso"
         %64 = OpString "g_tColor2"
         %66 = OpString "g_tColor"
               OpName %type_2d_image "type.2d.image"
               OpName %g_tColor "g_tColor"
               OpName %g_tColor2 "g_tColor2"
               OpName %type_sampler "type.sampler"
               OpName %g_sAniso "g_sAniso"
               OpName %in_var_TEXCOORD2 "in.var.TEXCOORD2"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPs "MainPs"
               OpName %PS_INPUT "PS_INPUT"
               OpMemberName %PS_INPUT 0 "vTextureCoords"
               OpName %param_var_i "param.var.i"
               OpName %PS_OUTPUT "PS_OUTPUT"
               OpMemberName %PS_OUTPUT 0 "vColor"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_TEXCOORD2 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %g_tColor DescriptorSet 0
               OpDecorate %g_tColor Binding 0
               OpDecorate %g_tColor2 DescriptorSet 0
               OpDecorate %g_tColor2 Binding 1
               OpDecorate %g_sAniso DescriptorSet 0
               OpDecorate %g_sAniso Binding 2
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
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
         %68 = OpTypeFunction %void
   %PS_INPUT = OpTypeStruct %v2float
%_ptr_Function_PS_INPUT = OpTypePointer Function %PS_INPUT
  %PS_OUTPUT = OpTypeStruct %v4float
         %78 = OpTypeFunction %PS_OUTPUT %_ptr_Function_PS_INPUT
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%_ptr_Function_v2float = OpTypePointer Function %v2float
%type_sampled_image = OpTypeSampledImage %type_2d_image
%_ptr_Function_v4float = OpTypePointer Function %v4float
   %g_tColor = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
  %g_tColor2 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
;CHECK-NOT: %g_tColor2 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
   %g_sAniso = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD2 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %40 = OpExtInst %void %1 DebugInfoNone
         %56 = OpExtInst %void %1 DebugExpression
         %23 = OpExtInst %void %1 DebugSource %21
         %24 = OpExtInst %void %1 DebugCompilationUnit 1 4 %23 HLSL
         %27 = OpExtInst %void %1 DebugTypeComposite %25 Structure %23 11 1 %24 %25 %uint_128 FlagIsProtected|FlagIsPrivate %28
         %30 = OpExtInst %void %1 DebugTypeBasic %29 %uint_32 Float
         %31 = OpExtInst %void %1 DebugTypeVector %30 4
         %28 = OpExtInst %void %1 DebugTypeMember %32 %31 %23 13 5 %27 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %36 = OpExtInst %void %1 DebugTypeComposite %34 Structure %23 6 1 %24 %34 %uint_64 FlagIsProtected|FlagIsPrivate %37
         %38 = OpExtInst %void %1 DebugTypeVector %30 2
         %37 = OpExtInst %void %1 DebugTypeMember %39 %38 %23 8 5 %36 %uint_0 %uint_64 FlagIsProtected|FlagIsPrivate
         %43 = OpExtInst %void %1 DebugTypeComposite %41 Class %23 0 0 %24 %42 %40 FlagIsProtected|FlagIsPrivate
         %45 = OpExtInst %void %1 DebugTypeTemplateParameter %44 %30 %40 %23 0 0
         %46 = OpExtInst %void %1 DebugTypeTemplate %43 %45
         %47 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %27 %36
         %49 = OpExtInst %void %1 DebugFunction %48 %47 %23 16 1 %24 %48 FlagIsProtected|FlagIsPrivate 17 %40
         %51 = OpExtInst %void %1 DebugLexicalBlock %23 17 1 %49
         %53 = OpExtInst %void %1 DebugLocalVariable %52 %38 %23 20 12 %51 FlagIsLocal
         %55 = OpExtInst %void %1 DebugLocalVariable %54 %27 %23 18 15 %51 FlagIsLocal
         %58 = OpExtInst %void %1 DebugLocalVariable %57 %36 %23 16 29 %49 FlagIsLocal 1
         %61 = OpExtInst %void %1 DebugTypeComposite %59 Structure %23 0 0 %24 %60 %40 FlagIsProtected|FlagIsPrivate
         %63 = OpExtInst %void %1 DebugGlobalVariable %62 %61 %23 4 14 %24 %62 %g_sAniso FlagIsDefinition
         %65 = OpExtInst %void %1 DebugGlobalVariable %64 %43 %23 2 11 %24 %64 %g_tColor2 FlagIsDefinition
;CHECK-NOT: %65 = OpExtInst %void %1 DebugGlobalVariable %64 %43 %23 2 11 %24 %64 %g_tColor2 FlagIsDefinition
;CHECK:     %65 = OpExtInst %void %1 DebugGlobalVariable %64 %43 %23 2 11 %24 %64 %40 FlagIsDefinition
         %67 = OpExtInst %void %1 DebugGlobalVariable %66 %43 %23 1 11 %24 %66 %g_tColor FlagIsDefinition
     %MainPs = OpFunction %void None %68
         %69 = OpLabel
        %117 = OpExtInst %void %1 DebugScope %51
        %101 = OpVariable %_ptr_Function_PS_OUTPUT Function
        %102 = OpVariable %_ptr_Function_v2float Function
        %118 = OpExtInst %void %1 DebugNoScope
        %103 = OpVariable %_ptr_Function_PS_OUTPUT Function
%param_var_i = OpVariable %_ptr_Function_PS_INPUT Function
         %73 = OpLoad %v2float %in_var_TEXCOORD2
         %74 = OpCompositeConstruct %PS_INPUT %73
               OpStore %param_var_i %74
        %119 = OpExtInst %void %1 DebugScope %49
        %105 = OpExtInst %void %1 DebugDeclare %58 %param_var_i %56
        %120 = OpExtInst %void %1 DebugScope %51
        %106 = OpExtInst %void %1 DebugDeclare %55 %101 %56
               OpLine %21 20 17
        %107 = OpAccessChain %_ptr_Function_v2float %param_var_i %int_0
        %108 = OpLoad %v2float %107
               OpLine %21 20 12
               OpStore %102 %108
        %109 = OpExtInst %void %1 DebugDeclare %53 %102 %56
               OpLine %21 21 26
        %110 = OpLoad %type_2d_image %g_tColor
               OpLine %21 21 46
        %111 = OpLoad %type_sampler %g_sAniso
               OpLine %21 21 57
        %112 = OpLoad %v2float %102
               OpLine %21 21 26
        %113 = OpSampledImage %type_sampled_image %110 %111
        %114 = OpImageSampleImplicitLod %v4float %113 %112 None
               OpLine %21 21 5
        %115 = OpAccessChain %_ptr_Function_v4float %101 %int_0
               OpStore %115 %114
               OpLine %21 22 12
        %116 = OpLoad %PS_OUTPUT %101
               OpLine %21 22 5
               OpStore %103 %116
        %121 = OpExtInst %void %1 DebugNoScope
         %76 = OpLoad %PS_OUTPUT %103
         %77 = OpCompositeExtract %v4float %76 0
               OpStore %out_var_SV_Target0 %77
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, KeepDebugScopeParent) {
  // Verify that local variable tc and its store are kept by DebugDeclare.
  //
  // Same shader source as DebugInfoInFunctionKeepStoreVarElim. The SPIR-V
  // has just been inlined.

  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %out_var_SV_TARGET0
               OpExecutionMode %main OriginUpperLeft
         %11 = OpString "float"
         %16 = OpString "t.hlsl"
         %19 = OpString "src.main"
               OpName %out_var_SV_TARGET0 "out.var.SV_TARGET0"
               OpName %main "main"
               OpDecorate %out_var_SV_TARGET0 Location 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
          %7 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
         %26 = OpTypeFunction %v4float
%out_var_SV_TARGET0 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %33 = OpExtInst %void %1 DebugInfoNone
         %13 = OpExtInst %void %1 DebugTypeBasic %11 %uint_32 Float
         %14 = OpExtInst %void %1 DebugTypeVector %13 4
         %15 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %14
         %17 = OpExtInst %void %1 DebugSource %16
         %18 = OpExtInst %void %1 DebugCompilationUnit 1 4 %17 HLSL
         %20 = OpExtInst %void %1 DebugFunction %19 %15 %17 1 1 %18 %19 FlagIsProtected|FlagIsPrivate 2 %33
         %22 = OpExtInst %void %1 DebugLexicalBlock %17 2 1 %20
       %main = OpFunction %void None %23
         %24 = OpLabel
         %31 = OpVariable %_ptr_Function_v4float Function
; CHECK: [[block:%\w+]] = OpExtInst %void %1 DebugLexicalBlock
; CHECK: DebugScope [[block]]
         %34 = OpExtInst %void %1 DebugScope %22
               OpLine %16 3 5
               OpStore %31 %7
               OpStore %out_var_SV_TARGET0 %7
         %35 = OpExtInst %void %1 DebugNoScope
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, KeepExportFunctions) {
  // All functions are reachable.  In particular, ExportedFunc and Constant are
  // reachable because ExportedFunc is exported.  Nothing should be removed.
  const std::vector<const char*> text = {
      // clang-format off
               "OpCapability Shader",
               "OpCapability Linkage",
               "OpMemoryModel Logical GLSL450",
               "OpEntryPoint Fragment %main \"main\"",
               "OpName %main \"main\"",
               "OpName %ExportedFunc \"ExportedFunc\"",
               "OpName %Live \"Live\"",
               "OpDecorate %ExportedFunc LinkageAttributes \"ExportedFunc\" Export",
       "%void = OpTypeVoid",
          "%7 = OpTypeFunction %void",
       "%main = OpFunction %void None %7",
         "%15 = OpLabel",
               "OpReturn",
               "OpFunctionEnd",
"%ExportedFunc = OpFunction %void None %7",
         "%19 = OpLabel",
         "%16 = OpFunctionCall %void %Live",
               "OpReturn",
               "OpFunctionEnd",
  "%Live = OpFunction %void None %7",
         "%20 = OpLabel",
               "OpReturn",
               "OpFunctionEnd"
      // clang-format on
  };

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  std::string assembly = JoinAllInsts(text);
  auto result = SinglePassRunAndDisassemble<AggressiveDCEPass>(
      assembly, /* skip_nop = */ true, /* do_validation = */ false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
  EXPECT_EQ(assembly, std::get<0>(result));
}

TEST_F(AggressiveDCETest, KeepPrivateVarInExportFunctions) {
  // The loads and stores from the private variable should not be removed
  // because the functions are exported and could be called.
  const std::string text = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpSource HLSL 630
OpName %privateVar "privateVar"
OpName %ReadPrivate "ReadPrivate"
OpName %WritePrivate "WritePrivate"
OpName %value "value"
OpDecorate %ReadPrivate LinkageAttributes "ReadPrivate" Export
OpDecorate %WritePrivate LinkageAttributes "WritePrivate" Export
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%6 = OpTypeFunction %int
%void = OpTypeVoid
%_ptr_Function_int = OpTypePointer Function %int
%10 = OpTypeFunction %void %_ptr_Function_int
%privateVar = OpVariable %_ptr_Private_int Private
%ReadPrivate = OpFunction %int None %6
%12 = OpLabel
%8 = OpLoad %int %privateVar
OpReturnValue %8
OpFunctionEnd
%WritePrivate = OpFunction %void None %10
%value = OpFunctionParameter %_ptr_Function_int
%13 = OpLabel
%14 = OpLoad %int %value
OpStore %privateVar %14
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto result = SinglePassRunAndDisassemble<AggressiveDCEPass>(
      text, /* skip_nop = */ true, /* do_validation = */ false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
  EXPECT_EQ(text, std::get<0>(result));
}

TEST_F(AggressiveDCETest, KeepLableNames) {
  const std::string text = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpSource HLSL 630
OpName %WritePrivate "WritePrivate"
OpName %entry "entry"
OpName %target "target"
OpDecorate %WritePrivate LinkageAttributes "WritePrivate" Export
%void = OpTypeVoid
%3 = OpTypeFunction %void
%WritePrivate = OpFunction %void None %3
%entry = OpLabel
OpBranch %target
%target = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto result = SinglePassRunAndDisassemble<AggressiveDCEPass>(
      text, /* skip_nop = */ true, /* do_validation = */ false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
  EXPECT_EQ(text, std::get<0>(result));
}

TEST_F(AggressiveDCETest, PreserveInterface) {
  // Set preserve_interface to true. Verify that unused uniform
  // constant in entry point interface is not eliminated.
  const std::string text = R"(OpCapability RayTracingKHR
OpExtension "SPV_KHR_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint RayGenerationNV %2 "main" %3 %4
OpDecorate %3 Location 0
OpDecorate %4 DescriptorSet 2
OpDecorate %4 Binding 0
%void = OpTypeVoid
%6 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%float = OpTypeFloat 32
%_ptr_CallableDataNV_float = OpTypePointer CallableDataNV %float
%3 = OpVariable %_ptr_CallableDataNV_float CallableDataNV
%13 = OpTypeAccelerationStructureKHR
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%4 = OpVariable %_ptr_UniformConstant_13 UniformConstant
%2 = OpFunction %void None %6
%15 = OpLabel
OpExecuteCallableKHR %uint_0 %3
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto result = SinglePassRunAndDisassemble<AggressiveDCEPass>(
      text, /* skip_nop = */ true, /* do_validation = */ false,
      /* preserve_interface */ true);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
  EXPECT_EQ(text, std::get<0>(result));
}

TEST_F(AggressiveDCETest, EmptyContinueWithConditionalBranch) {
  const std::string text = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main"
OpExecutionMode %2 OriginUpperLeft
%void = OpTypeVoid
%4 = OpTypeFunction %void
%bool = OpTypeBool
%false = OpConstantFalse %bool
%2 = OpFunction %void None %4
%9 = OpLabel
OpBranch %10
%10 = OpLabel
OpLoopMerge %11 %12 None
OpBranch %13
%13 = OpLabel
OpKill
%12 = OpLabel
OpBranchConditional %false %10 %10
%11 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<AggressiveDCEPass>(text, text, false);
}

TEST_F(AggressiveDCETest, FunctionBecomesUnreachableAfterDCE) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 320
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
          %7 = OpTypeFunction %int
     %int_n1 = OpConstant %int -1
          %2 = OpFunction %void None %4
          %9 = OpLabel
               OpKill
         %10 = OpLabel
         %11 = OpFunctionCall %int %12
               OpReturn
               OpFunctionEnd
; CHECK: {{%\w+}} = OpFunction %int DontInline|Pure
         %12 = OpFunction %int DontInline|Pure %7
; CHECK-NEXT: {{%\w+}} = OpLabel
         %13 = OpLabel
         %14 = OpVariable %_ptr_Function_int Function
; CHECK-NEXT: OpBranch [[header:%\w+]]
               OpBranch %15
; CHECK-NEXT: [[header]] = OpLabel
; CHECK-NEXT: OpBranch [[merge:%\w+]]
         %15 = OpLabel
               OpLoopMerge %16 %17 None
               OpBranch %18
         %18 = OpLabel
         %19 = OpLoad %int %14
               OpBranch %17
         %17 = OpLabel
               OpBranch %15
; CHECK-NEXT: [[merge]] = OpLabel
         %16 = OpLabel
; CHECK-NEXT: OpReturnValue %int_n1
               OpReturnValue %int_n1
; CHECK-NEXT: OpFunctionEnd
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, KeepTopLevelDebugInfo) {
  // Don't eliminate DebugCompilationUnit, DebugSourceContinued, and
  // DebugEntryPoint
  const std::string text = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %out_var_SV_Target0
               OpExecutionMode %MainPs OriginUpperLeft
          %4 = OpString "foo2.frag"
          %5 = OpString "
struct PS_OUTPUT
{
    float4 vColor : SV_Target0 ;
} ;

"
          %6 = OpString "
PS_OUTPUT MainPs ( )
{
    PS_OUTPUT ps_output ;
    ps_output . vColor = float4( 1.0, 0.0, 0.0, 0.0 );
    return ps_output ;
}

"
          %7 = OpString "float"
          %8 = OpString "vColor"
          %9 = OpString "PS_OUTPUT"
         %10 = OpString "MainPs"
         %11 = OpString ""
         %12 = OpString "ps_output"
         %13 = OpString "97a939fb"
         %14 = OpString " foo2.frag -E MainPs -T ps_6_1 -spirv -fspv-target-env=vulkan1.2 -fspv-debug=vulkan-with-source -fcgl -Fo foo2.frag.nopt.spv -Qembed_debug"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPs "MainPs"
               OpDecorate %out_var_SV_Target0 Location 0
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
         %23 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_5 = OpConstant %uint 5
     %uint_3 = OpConstant %uint 3
     %uint_0 = OpConstant %uint 0
   %uint_128 = OpConstant %uint 128
    %uint_12 = OpConstant %uint 12
     %uint_2 = OpConstant %uint 2
     %uint_8 = OpConstant %uint 8
     %uint_7 = OpConstant %uint 7
     %uint_9 = OpConstant %uint 9
    %uint_15 = OpConstant %uint 15
         %42 = OpTypeFunction %void
    %uint_10 = OpConstant %uint 10
    %uint_53 = OpConstant %uint 53
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %49 = OpExtInst %void %1 DebugExpression
         %50 = OpExtInst %void %1 DebugSource %4 %5
         %51 = OpExtInst %void %1 DebugSourceContinued %6
; CHECK:     %51 = OpExtInst %void %1 DebugSourceContinued %6
         %52 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %50 %uint_5
; CHECK:     %52 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %50 %uint_5
         %53 = OpExtInst %void %1 DebugTypeBasic %7 %uint_32 %uint_3 %uint_0
         %54 = OpExtInst %void %1 DebugTypeVector %53 %uint_4
         %55 = OpExtInst %void %1 DebugTypeMember %8 %54 %50 %uint_4 %uint_12 %uint_0 %uint_128 %uint_3
         %56 = OpExtInst %void %1 DebugTypeComposite %9 %uint_1 %50 %uint_2 %uint_8 %52 %9 %uint_128 %uint_3 %55
         %57 = OpExtInst %void %1 DebugTypeFunction %uint_3 %56
         %58 = OpExtInst %void %1 DebugFunction %10 %57 %50 %uint_7 %uint_1 %52 %11 %uint_3 %uint_8
         %59 = OpExtInst %void %1 DebugLexicalBlock %50 %uint_8 %uint_1 %58
         %60 = OpExtInst %void %1 DebugLocalVariable %12 %56 %50 %uint_9 %uint_15 %59 %uint_4
         %61 = OpExtInst %void %1 DebugEntryPoint %58 %52 %13 %14
; CHECK:     %61 = OpExtInst %void %1 DebugEntryPoint %58 %52 %13 %14
     %MainPs = OpFunction %void None %42
         %62 = OpLabel
         %63 = OpExtInst %void %1 DebugFunctionDefinition %58 %MainPs
        %112 = OpExtInst %void %1 DebugScope %59
        %111 = OpExtInst %void %1 DebugLine %50 %uint_10 %uint_10 %uint_5 %uint_53
        %110 = OpExtInst %void %1 DebugValue %60 %23 %49 %int_0
        %113 = OpExtInst %void %1 DebugNoLine
        %114 = OpExtInst %void %1 DebugNoScope
               OpStore %out_var_SV_Target0 %23
         %66 = OpExtInst %void %1 DebugLine %50 %uint_12 %uint_12 %uint_1 %uint_1
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(text, true);
}

TEST_F(AggressiveDCETest, RemoveOutputTrue) {
  // Remove dead n_out output variable from module
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %c_out %c_in %n_out
;CHECK: OpEntryPoint Vertex %main "main" %c_out %c_in
               OpSource GLSL 450
               OpName %main "main"
               OpName %c_out "c_out"
               OpName %c_in "c_in"
               OpName %n_out "n_out"
               OpDecorate %c_out Location 0
               OpDecorate %c_in Location 0
               OpDecorate %n_out Location 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %c_out = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
       %c_in = OpVariable %_ptr_Input_v4float Input
    %v3float = OpTypeVector %float 3
%_ptr_Output_v3float = OpTypePointer Output %v3float
      %n_out = OpVariable %_ptr_Output_v3float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %12 = OpLoad %v4float %c_in
               OpStore %c_out %12
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(text, true, false, true);
}

TEST_F(AggressiveDCETest, RemoveOutputFalse) {
  // Remove dead n_out output variable from module
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %c_out %c_in %n_out
;CHECK: OpEntryPoint Vertex %main "main" %c_out %c_in %n_out
               OpSource GLSL 450
               OpName %main "main"
               OpName %c_out "c_out"
               OpName %c_in "c_in"
               OpName %n_out "n_out"
               OpDecorate %c_out Location 0
               OpDecorate %c_in Location 0
               OpDecorate %n_out Location 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %c_out = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
       %c_in = OpVariable %_ptr_Input_v4float Input
    %v3float = OpTypeVector %float 3
%_ptr_Output_v3float = OpTypePointer Output %v3float
      %n_out = OpVariable %_ptr_Output_v3float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %12 = OpLoad %v4float %c_in
               OpStore %c_out %12
               OpReturn
               OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<AggressiveDCEPass>(text, true, false, false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
