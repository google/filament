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

using LocalSingleBlockLoadStoreElimTest = PassTest<::testing::Test>;

TEST_F(LocalSingleBlockLoadStoreElimTest, SimpleStoreLoadElim) {
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     gl_FragColor = v;
  // }

  const std::string predefs_before =
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
%15 = OpLoad %v4float %v
OpStore %gl_FragColor %15
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
OpStore %gl_FragColor %14
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(
      predefs_before + before, predefs_before + after, true, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, SimpleLoadLoadElim) {
  // #version 140
  //
  // in vec4 BaseColor;
  // in float fi;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     if (fi < 0)
  //         v = vec4(0.0);
  //     gl_FragData[0] = v;
  //     gl_FragData[1] = v;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %fi %gl_FragData
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %fi "fi"
OpName %gl_FragData "gl_FragData"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%fi = OpVariable %_ptr_Input_float Input
%float_0 = OpConstant %float 0
%bool = OpTypeBool
%16 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_arr_v4float_uint_32 = OpTypeArray %v4float %uint_32
%_ptr_Output__arr_v4float_uint_32 = OpTypePointer Output %_arr_v4float_uint_32
%gl_FragData = OpVariable %_ptr_Output__arr_v4float_uint_32 Output
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%int_1 = OpConstant %int 1
)";

  const std::string before =
      R"(%main = OpFunction %void None %8
%25 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%26 = OpLoad %v4float %BaseColor
OpStore %v %26
%27 = OpLoad %float %fi
%28 = OpFOrdLessThan %bool %27 %float_0
OpSelectionMerge %29 None
OpBranchConditional %28 %30 %29
%30 = OpLabel
OpStore %v %16
OpBranch %29
%29 = OpLabel
%31 = OpLoad %v4float %v
%32 = OpAccessChain %_ptr_Output_v4float %gl_FragData %int_0
OpStore %32 %31
%33 = OpLoad %v4float %v
%34 = OpAccessChain %_ptr_Output_v4float %gl_FragData %int_1
OpStore %34 %33
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %8
%25 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%26 = OpLoad %v4float %BaseColor
OpStore %v %26
%27 = OpLoad %float %fi
%28 = OpFOrdLessThan %bool %27 %float_0
OpSelectionMerge %29 None
OpBranchConditional %28 %30 %29
%30 = OpLabel
OpStore %v %16
OpBranch %29
%29 = OpLabel
%31 = OpLoad %v4float %v
%32 = OpAccessChain %_ptr_Output_v4float %gl_FragData %int_0
OpStore %32 %31
%34 = OpAccessChain %_ptr_Output_v4float %gl_FragData %int_1
OpStore %34 %31
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(
      predefs + before, predefs + after, true, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, StoreStoreElim) {
  //
  // Note first store to v is eliminated
  //
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor;
  // layout(location = 0) out vec4 OutColor;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     v = v * 0.5;
  //     OutColor = v;
  // }

  const std::string predefs_before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %OutColor "OutColor"
OpDecorate %BaseColor Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%float_0_5 = OpConstant %float 0.5
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %7
%14 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%15 = OpLoad %v4float %BaseColor
OpStore %v %15
%16 = OpLoad %v4float %v
%17 = OpVectorTimesScalar %v4float %16 %float_0_5
OpStore %v %17
%18 = OpLoad %v4float %v
OpStore %OutColor %18
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %7
%14 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%15 = OpLoad %v4float %BaseColor
%17 = OpVectorTimesScalar %v4float %15 %float_0_5
OpStore %v %17
OpStore %OutColor %17
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(
      predefs_before + before, predefs_before + after, true, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest,
       NoStoreElimIfInterveningAccessChainLoad) {
  //
  // Note the first Store to %v is not eliminated due to the following access
  // chain reference.
  //
  // #version 450
  //
  // layout(location = 0) in vec4 BaseColor0;
  // layout(location = 1) in vec4 BaseColor1;
  // layout(location = 2) flat in int Idx;
  // layout(location = 0) out vec4 OutColor;
  //
  // void main()
  // {
  //     vec4 v = BaseColor0;
  //     float f = v[Idx];
  //     v = BaseColor1 + vec4(0.1);
  //     OutColor = v/f;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor0 %Idx %BaseColor1 %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %v "v"
OpName %BaseColor0 "BaseColor0"
OpName %f "f"
OpName %Idx "Idx"
OpName %BaseColor1 "BaseColor1"
OpName %OutColor "OutColor"
OpDecorate %BaseColor0 Location 0
OpDecorate %Idx Flat
OpDecorate %Idx Location 2
OpDecorate %BaseColor1 Location 1
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor0 = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%Idx = OpVariable %_ptr_Input_int Input
%BaseColor1 = OpVariable %_ptr_Input_v4float Input
%float_0_100000001 = OpConstant %float 0.100000001
%19 = OpConstantComposite %v4float %float_0_100000001 %float_0_100000001 %float_0_100000001 %float_0_100000001
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %10
%21 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%f = OpVariable %_ptr_Function_float Function
%22 = OpLoad %v4float %BaseColor0
OpStore %v %22
%23 = OpLoad %int %Idx
%24 = OpAccessChain %_ptr_Function_float %v %23
%25 = OpLoad %float %24
OpStore %f %25
%26 = OpLoad %v4float %BaseColor1
%27 = OpFAdd %v4float %26 %19
OpStore %v %27
%28 = OpLoad %v4float %v
%29 = OpLoad %float %f
%30 = OpCompositeConstruct %v4float %29 %29 %29 %29
%31 = OpFDiv %v4float %28 %30
OpStore %OutColor %31
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %10
%21 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%f = OpVariable %_ptr_Function_float Function
%22 = OpLoad %v4float %BaseColor0
OpStore %v %22
%23 = OpLoad %int %Idx
%24 = OpAccessChain %_ptr_Function_float %v %23
%25 = OpLoad %float %24
OpStore %f %25
%26 = OpLoad %v4float %BaseColor1
%27 = OpFAdd %v4float %26 %19
OpStore %v %27
%30 = OpCompositeConstruct %v4float %25 %25 %25 %25
%31 = OpFDiv %v4float %27 %30
OpStore %OutColor %31
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(
      predefs + before, predefs + after, true, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, NoElimIfInterveningAccessChainStore) {
  // #version 140
  //
  // in vec4 BaseColor;
  // flat in int Idx;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     v[Idx] = 0;
  //     gl_FragColor = v;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %Idx %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %Idx "Idx"
OpName %gl_FragColor "gl_FragColor"
OpDecorate %Idx Flat
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%Idx = OpVariable %_ptr_Input_int Input
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %8
%18 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%19 = OpLoad %v4float %BaseColor
OpStore %v %19
%20 = OpLoad %int %Idx
%21 = OpAccessChain %_ptr_Function_float %v %20
OpStore %21 %float_0
%22 = OpLoad %v4float %v
OpStore %gl_FragColor %22
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(assembly, assembly,
                                                           false, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, NoElimIfInterveningFunctionCall) {
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // void foo() {
  // }
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     foo();
  //     gl_FragColor = v;
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %foo_ "foo("
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %8
%14 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%15 = OpLoad %v4float %BaseColor
OpStore %v %15
%16 = OpFunctionCall %void %foo_
%17 = OpLoad %v4float %v
OpStore %gl_FragColor %17
OpReturn
OpFunctionEnd
%foo_ = OpFunction %void None %8
%18 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(assembly, assembly,
                                                           false, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, ElimIfCopyObjectInFunction) {
  // Note: SPIR-V hand edited to insert CopyObject
  //
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // void main()
  // {
  //   vec4 v1 = BaseColor;
  //   gl_FragData[0] = v1;
  //   vec4 v2 = BaseColor * 0.5;
  //   gl_FragData[1] = v2;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragData
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v1 "v1"
OpName %BaseColor "BaseColor"
OpName %gl_FragData "gl_FragData"
OpName %v2 "v2"
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_arr_v4float_uint_32 = OpTypeArray %v4float %uint_32
%_ptr_Output__arr_v4float_uint_32 = OpTypePointer Output %_arr_v4float_uint_32
%gl_FragData = OpVariable %_ptr_Output__arr_v4float_uint_32 Output
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%float_0_5 = OpConstant %float 0.5
%int_1 = OpConstant %int 1
)";

  const std::string before =
      R"(%main = OpFunction %void None %8
%22 = OpLabel
%v1 = OpVariable %_ptr_Function_v4float Function
%v2 = OpVariable %_ptr_Function_v4float Function
%23 = OpLoad %v4float %BaseColor
OpStore %v1 %23
%24 = OpLoad %v4float %v1
%25 = OpAccessChain %_ptr_Output_v4float %gl_FragData %int_0
OpStore %25 %24
%26 = OpLoad %v4float %BaseColor
%27 = OpVectorTimesScalar %v4float %26 %float_0_5
%28 = OpCopyObject %_ptr_Function_v4float %v2
OpStore %28 %27
%29 = OpLoad %v4float %28
%30 = OpAccessChain %_ptr_Output_v4float %gl_FragData %int_1
OpStore %30 %29
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %8
%22 = OpLabel
%v1 = OpVariable %_ptr_Function_v4float Function
%v2 = OpVariable %_ptr_Function_v4float Function
%23 = OpLoad %v4float %BaseColor
OpStore %v1 %23
%25 = OpAccessChain %_ptr_Output_v4float %gl_FragData %int_0
OpStore %25 %23
%26 = OpLoad %v4float %BaseColor
%27 = OpVectorTimesScalar %v4float %26 %float_0_5
%28 = OpCopyObject %_ptr_Function_v4float %v2
OpStore %28 %27
%30 = OpAccessChain %_ptr_Output_v4float %gl_FragData %int_1
OpStore %30 %27
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(
      predefs + before, predefs + after, true, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, ElimOpaque) {
  // SPIR-V not representable in GLSL; not generatable from HLSL
  // at the moment

  const std::string predefs =
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
      R"(%main = OpFunction %void None %12
%28 = OpLabel
%s0 = OpVariable %_ptr_Function_S_t Function
%param = OpVariable %_ptr_Function_S_t Function
%29 = OpLoad %v2float %texCoords
%30 = OpLoad %S_t %s0
%31 = OpCompositeInsert %S_t %29 %30 0
OpStore %s0 %31
%32 = OpLoad %18 %sampler15
%33 = OpLoad %S_t %s0
%34 = OpCompositeInsert %S_t %32 %33 2
OpStore %s0 %34
%35 = OpLoad %S_t %s0
OpStore %param %35
%36 = OpLoad %S_t %param
%37 = OpCompositeExtract %18 %36 2
%38 = OpLoad %S_t %param
%39 = OpCompositeExtract %v2float %38 0
%40 = OpImageSampleImplicitLod %v4float %37 %39
OpStore %outColor %40
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %12
%28 = OpLabel
%s0 = OpVariable %_ptr_Function_S_t Function
%param = OpVariable %_ptr_Function_S_t Function
%29 = OpLoad %v2float %texCoords
%30 = OpLoad %S_t %s0
%31 = OpCompositeInsert %S_t %29 %30 0
%32 = OpLoad %18 %sampler15
%34 = OpCompositeInsert %S_t %32 %31 2
OpStore %s0 %34
OpStore %param %34
%37 = OpCompositeExtract %18 %34 2
%39 = OpCompositeExtract %v2float %34 0
%40 = OpImageSampleImplicitLod %v4float %37 %39
OpStore %outColor %40
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(
      predefs + before, predefs + after, true, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, PositiveAndNegativeCallTree) {
  // Note that the call tree function bar is optimized, but foo is not
  //
  // #version 140
  //
  // in vec4 BaseColor;
  //
  // vec4 foo(vec4 v1)
  // {
  //     vec4 t = v1;
  //     return t;
  // }
  //
  // vec4 bar(vec4 v1)
  // {
  //     vec4 t = v1;
  //     return t;
  // }
  //
  // void main()
  // {
  //     gl_FragColor = bar(BaseColor);
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %gl_FragColor %BaseColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %foo_vf4_ "foo(vf4;"
OpName %v1 "v1"
OpName %bar_vf4_ "bar(vf4;"
OpName %v1_0 "v1"
OpName %t "t"
OpName %t_0 "t"
OpName %gl_FragColor "gl_FragColor"
OpName %BaseColor "BaseColor"
OpName %param "param"
%void = OpTypeVoid
%13 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%17 = OpTypeFunction %v4float %_ptr_Function_v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %13
%20 = OpLabel
%param = OpVariable %_ptr_Function_v4float Function
%21 = OpLoad %v4float %BaseColor
OpStore %param %21
%22 = OpFunctionCall %v4float %bar_vf4_ %param
OpStore %gl_FragColor %22
OpReturn
OpFunctionEnd
)";

  const std::string before =
      R"(%foo_vf4_ = OpFunction %v4float None %17
%v1 = OpFunctionParameter %_ptr_Function_v4float
%23 = OpLabel
%t = OpVariable %_ptr_Function_v4float Function
%24 = OpLoad %v4float %v1
OpStore %t %24
%25 = OpLoad %v4float %t
OpReturnValue %25
OpFunctionEnd
%bar_vf4_ = OpFunction %v4float None %17
%v1_0 = OpFunctionParameter %_ptr_Function_v4float
%26 = OpLabel
%t_0 = OpVariable %_ptr_Function_v4float Function
%27 = OpLoad %v4float %v1_0
OpStore %t_0 %27
%28 = OpLoad %v4float %t_0
OpReturnValue %28
OpFunctionEnd
)";

  const std::string after =
      R"(%foo_vf4_ = OpFunction %v4float None %17
%v1 = OpFunctionParameter %_ptr_Function_v4float
%23 = OpLabel
%t = OpVariable %_ptr_Function_v4float Function
%24 = OpLoad %v4float %v1
OpStore %t %24
%25 = OpLoad %v4float %t
OpReturnValue %25
OpFunctionEnd
%bar_vf4_ = OpFunction %v4float None %17
%v1_0 = OpFunctionParameter %_ptr_Function_v4float
%26 = OpLabel
%t_0 = OpVariable %_ptr_Function_v4float Function
%27 = OpLoad %v4float %v1_0
OpStore %t_0 %27
OpReturnValue %27
OpFunctionEnd
)";

  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(
      predefs + before, predefs + after, true, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, PointerVariable) {
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
  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(before, after, true,
                                                           true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, RedundantStore) {
  // Test that checks if a pointer variable is removed.
  const std::string predefs_before =
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
OpBranch %16
%16 = OpLabel
%15 = OpLoad %v4float %v
OpStore %v %15
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
OpBranch %16
%16 = OpLabel
%15 = OpLoad %v4float %v
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(
      predefs_before + before, predefs_before + after, true, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, RedundantStore2) {
  // Test that checks if a pointer variable is removed.
  const std::string predefs_before =
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
OpBranch %16
%16 = OpLabel
%15 = OpLoad %v4float %v
OpStore %v %15
%17 = OpLoad %v4float %v
OpStore %v %17
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
OpBranch %16
%16 = OpLabel
%15 = OpLoad %v4float %v
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(
      predefs_before + before, predefs_before + after, true, true);
}

// Test that that an unused OpAccessChain between two store does does not
// hinders the removal of the first store.  We need to check this because
// local-access-chain-convert does always remove the OpAccessChain instructions
// that become dead.

TEST_F(LocalSingleBlockLoadStoreElimTest,
       StoreElimIfInterveningUnusedAccessChain) {
  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor0 %Idx %BaseColor1 %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %v "v"
OpName %BaseColor0 "BaseColor0"
OpName %Idx "Idx"
OpName %BaseColor1 "BaseColor1"
OpName %OutColor "OutColor"
OpDecorate %BaseColor0 Location 0
OpDecorate %Idx Flat
OpDecorate %Idx Location 2
OpDecorate %BaseColor1 Location 1
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor0 = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%Idx = OpVariable %_ptr_Input_int Input
%BaseColor1 = OpVariable %_ptr_Input_v4float Input
%float_0_100000001 = OpConstant %float 0.100000001
%19 = OpConstantComposite %v4float %float_0_100000001 %float_0_100000001 %float_0_100000001 %float_0_100000001
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %10
%21 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%22 = OpLoad %v4float %BaseColor0
OpStore %v %22
%23 = OpLoad %int %Idx
%24 = OpAccessChain %_ptr_Function_float %v %23
%26 = OpLoad %v4float %BaseColor1
%27 = OpFAdd %v4float %26 %19
OpStore %v %27
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %10
%21 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%22 = OpLoad %v4float %BaseColor0
%23 = OpLoad %int %Idx
%24 = OpAccessChain %_ptr_Function_float %v %23
%26 = OpLoad %v4float %BaseColor1
%27 = OpFAdd %v4float %26 %19
OpStore %v %27
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<LocalSingleBlockLoadStoreElimPass>(
      predefs + before, predefs + after, true, true);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, VariablePointerTest) {
  // Check that the load of the first variable is still used and that the load
  // of the third variable is propagated.  The first load has to remain because
  // of the store to the variable pointer.
  const std::string text = R"(
; CHECK: [[v1:%\w+]] = OpVariable
; CHECK: [[v2:%\w+]] = OpVariable
; CHECK: [[v3:%\w+]] = OpVariable
; CHECK: [[phi:%\w+]] = OpPhi
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
               OpSelectionMerge %18 None
               OpBranchConditional %true %19 %20
         %19 = OpLabel
               OpBranch %18
         %20 = OpLabel
               OpBranch %18
         %18 = OpLabel
         %21 = OpPhi %_ptr_Function_int %15 %19 %16 %20
               OpStore %15 %int_1
               OpStore %21 %int_0
         %22 = OpLoad %int %15
               OpStore %17 %int_0
         %23 = OpLoad %int %17
         %24 = OpIAdd %int %22 %23
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<LocalSingleBlockLoadStoreElimPass>(text, false);
}

TEST_F(LocalSingleBlockLoadStoreElimTest, DebugDeclareTest) {
  // If OpenCL.DebugInfo.100 enabled, check that store/load is still
  // optimized, but stores are not deleted for store/store.
  //
  // struct PS_INPUT {
  //   float4 c0 : COLOR0;
  //   float4 c1 : COLOR1;
  // };
  //
  // struct PS_OUTPUT {
  //   float4 vColor : SV_Target0;
  // };
  //
  // PS_OUTPUT MainPs(PS_INPUT i) {
  //   PS_OUTPUT ps_output;
  //   float4 c;
  //   c = i.c0;
  //   c += i.c1;
  //   c /= 2.0;
  //   ps_output.vColor = c;
  //   return ps_output;
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %in_var_COLOR0 %in_var_COLOR1 %out_var_SV_Target0
               OpExecutionMode %MainPs OriginUpperLeft
          %6 = OpString "foo3.frag"
          %7 = OpString "PS_OUTPUT"
          %8 = OpString "float"
          %9 = OpString "vColor"
         %10 = OpString "PS_INPUT"
         %11 = OpString "c1"
         %12 = OpString "c0"
         %13 = OpString "src.MainPs"
         %14 = OpString "c"
         %15 = OpString "ps_output"
         %16 = OpString "i"
               OpName %in_var_COLOR0 "in.var.COLOR0"
               OpName %in_var_COLOR1 "in.var.COLOR1"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPs "MainPs"
               OpName %PS_INPUT "PS_INPUT"
               OpMemberName %PS_INPUT 0 "c0"
               OpMemberName %PS_INPUT 1 "c1"
               OpName %param_var_i "param.var.i"
               OpName %PS_OUTPUT "PS_OUTPUT"
               OpMemberName %PS_OUTPUT 0 "vColor"
               OpName %src_MainPs "src.MainPs"
               OpName %i "i"
               OpName %bb_entry "bb.entry"
               OpName %ps_output "ps_output"
               OpName %c "c"
               OpDecorate %in_var_COLOR0 Location 0
               OpDecorate %in_var_COLOR1 Location 1
               OpDecorate %out_var_SV_Target0 Location 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %float = OpTypeFloat 32
    %float_2 = OpConstant %float 2
    %v4float = OpTypeVector %float 4
         %31 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
   %uint_128 = OpConstant %uint 128
     %uint_0 = OpConstant %uint 0
   %uint_256 = OpConstant %uint 256
         %40 = OpTypeFunction %void
   %PS_INPUT = OpTypeStruct %v4float %v4float
%_ptr_Function_PS_INPUT = OpTypePointer Function %PS_INPUT
  %PS_OUTPUT = OpTypeStruct %v4float
         %42 = OpTypeFunction %PS_OUTPUT %_ptr_Function_PS_INPUT
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%_ptr_Function_v4float = OpTypePointer Function %v4float
%in_var_COLOR0 = OpVariable %_ptr_Input_v4float Input
%in_var_COLOR1 = OpVariable %_ptr_Input_v4float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %45 = OpExtInst %void %1 DebugSource %6
         %46 = OpExtInst %void %1 DebugCompilationUnit 1 4 %45 HLSL
         %47 = OpExtInst %void %1 DebugTypeComposite %7 Structure %45 8 1 %46 %7 %uint_128 FlagIsProtected|FlagIsPrivate %48
         %49 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 Float
         %50 = OpExtInst %void %1 DebugTypeVector %49 4
         %48 = OpExtInst %void %1 DebugTypeMember %9 %50 %45 10 5 %47 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %51 = OpExtInst %void %1 DebugTypeComposite %10 Structure %45 2 1 %46 %10 %uint_256 FlagIsProtected|FlagIsPrivate %52 %53
         %53 = OpExtInst %void %1 DebugTypeMember %11 %50 %45 5 5 %51 %uint_128 %uint_128 FlagIsProtected|FlagIsPrivate
         %52 = OpExtInst %void %1 DebugTypeMember %12 %50 %45 4 5 %51 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %54 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %47 %51
         %55 = OpExtInst %void %1 DebugFunction %13 %54 %45 13 1 %46 %13 FlagIsProtected|FlagIsPrivate 14 %src_MainPs
         %56 = OpExtInst %void %1 DebugLexicalBlock %45 14 1 %55
         %57 = OpExtInst %void %1 DebugLocalVariable %14 %50 %45 16 12 %56 FlagIsLocal
         %58 = OpExtInst %void %1 DebugLocalVariable %15 %47 %45 15 15 %56 FlagIsLocal
         %59 = OpExtInst %void %1 DebugExpression
         %60 = OpExtInst %void %1 DebugLocalVariable %16 %51 %45 13 29 %55 FlagIsLocal 1
         %61 = OpExtInst %void %1 DebugLocalVariable %14 %50 %45 16 12 %55 FlagIsLocal 1
     %MainPs = OpFunction %void None %40
         %62 = OpLabel
%param_var_i = OpVariable %_ptr_Function_PS_INPUT Function
         %63 = OpLoad %v4float %in_var_COLOR0
         %64 = OpLoad %v4float %in_var_COLOR1
         %65 = OpCompositeConstruct %PS_INPUT %63 %64
               OpStore %param_var_i %65
         %66 = OpFunctionCall %PS_OUTPUT %src_MainPs %param_var_i
         %67 = OpCompositeExtract %v4float %66 0
               OpStore %out_var_SV_Target0 %67
               OpReturn
               OpFunctionEnd
               OpLine %6 13 1
 %src_MainPs = OpFunction %PS_OUTPUT None %42
         %83 = OpExtInst %void %1 DebugScope %55
               OpLine %6 13 29
          %i = OpFunctionParameter %_ptr_Function_PS_INPUT
         %69 = OpExtInst %void %1 DebugDeclare %60 %i %59
         %84 = OpExtInst %void %1 DebugNoScope
   %bb_entry = OpLabel
         %85 = OpExtInst %void %1 DebugScope %56
  %ps_output = OpVariable %_ptr_Function_PS_OUTPUT Function
          %c = OpVariable %_ptr_Function_v4float Function
         %71 = OpExtInst %void %1 DebugDeclare %61 %c %59
               OpLine %6 18 9
         %72 = OpAccessChain %_ptr_Function_v4float %i %int_0
               OpLine %6 18 13
         %73 = OpLoad %v4float %72
               OpLine %6 18 5
               OpStore %c %73
;CHECK:        OpStore %c %73
               OpLine %6 19 10
         %74 = OpAccessChain %_ptr_Function_v4float %i %int_1
               OpLine %6 19 14
         %75 = OpLoad %v4float %74
               OpLine %6 19 5
         %76 = OpLoad %v4float %c
;CHECK-NOT:       OpLine %6 19 5
;CHECK-NOT: %76 = OpLoad %v4float %c
               OpLine %6 19 7
         %77 = OpFAdd %v4float %76 %75
;CHECK-NOT: %77 = OpFAdd %v4float %76 %75
;CHECK:     %77 = OpFAdd %v4float %73 %75
               OpLine %6 19 5
               OpStore %c %77
;CHECK:        OpStore %c %77
               OpLine %6 20 5
         %78 = OpLoad %v4float %c
;CHECK-NOT:       OpLine %6 20 5
;CHECK-NOT: %78 = OpLoad %v4float %c
               OpLine %6 20 7
         %79 = OpFDiv %v4float %78 %31
;CHECK-NOT %79 = OpFDiv %v4float %78 %31
;CHECK:    %79 = OpFDiv %v4float %77 %31
               OpLine %6 20 5
               OpStore %c %79
;CHECK:        OpStore %c %79
               OpLine %6 22 26
         %80 = OpLoad %v4float %c
;CHECK-NOT:       OpLine %6 22 26
;CHECK-NOT: %80 = OpLoad %v4float %c
               OpLine %6 22 5
         %81 = OpAccessChain %_ptr_Function_v4float %ps_output %int_0
               OpStore %81 %80
;CHECK-NOT:    OpStore %81 %80
;CHECK:        OpStore %81 %79
               OpLine %6 23 12
         %82 = OpLoad %PS_OUTPUT %ps_output
               OpLine %6 23 5
               OpReturnValue %82
         %86 = OpExtInst %void %1 DebugNoScope
               OpFunctionEnd
  )";
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<LocalSingleBlockLoadStoreElimPass>(text, false);
}
TEST_F(LocalSingleBlockLoadStoreElimTest, DebugValueTest) {
  // If OpenCL.DebugInfo.100 enabled, check that store/load is still
  // optimized, but stores are not deleted for store/store.
  // Same source as DebugDeclareTest; DebugDeclare replaced with
  // equivalent DebugValue
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %in_var_COLOR0 %in_var_COLOR1 %out_var_SV_Target0
               OpExecutionMode %MainPs OriginUpperLeft
          %6 = OpString "foo3.frag"
          %7 = OpString "PS_OUTPUT"
          %8 = OpString "float"
          %9 = OpString "vColor"
         %10 = OpString "PS_INPUT"
         %11 = OpString "c1"
         %12 = OpString "c0"
         %13 = OpString "src.MainPs"
         %14 = OpString "c"
         %15 = OpString "ps_output"
         %16 = OpString "i"
               OpName %in_var_COLOR0 "in.var.COLOR0"
               OpName %in_var_COLOR1 "in.var.COLOR1"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPs "MainPs"
               OpName %PS_INPUT "PS_INPUT"
               OpMemberName %PS_INPUT 0 "c0"
               OpMemberName %PS_INPUT 1 "c1"
               OpName %param_var_i "param.var.i"
               OpName %PS_OUTPUT "PS_OUTPUT"
               OpMemberName %PS_OUTPUT 0 "vColor"
               OpName %src_MainPs "src.MainPs"
               OpName %i "i"
               OpName %bb_entry "bb.entry"
               OpName %ps_output "ps_output"
               OpName %c "c"
               OpDecorate %in_var_COLOR0 Location 0
               OpDecorate %in_var_COLOR1 Location 1
               OpDecorate %out_var_SV_Target0 Location 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %float = OpTypeFloat 32
    %float_2 = OpConstant %float 2
    %v4float = OpTypeVector %float 4
         %31 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
   %uint_128 = OpConstant %uint 128
     %uint_0 = OpConstant %uint 0
   %uint_256 = OpConstant %uint 256
         %40 = OpTypeFunction %void
   %PS_INPUT = OpTypeStruct %v4float %v4float
%_ptr_Function_PS_INPUT = OpTypePointer Function %PS_INPUT
  %PS_OUTPUT = OpTypeStruct %v4float
         %42 = OpTypeFunction %PS_OUTPUT %_ptr_Function_PS_INPUT
%_ptr_Function_PS_OUTPUT = OpTypePointer Function %PS_OUTPUT
%_ptr_Function_v4float = OpTypePointer Function %v4float
%in_var_COLOR0 = OpVariable %_ptr_Input_v4float Input
%in_var_COLOR1 = OpVariable %_ptr_Input_v4float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %45 = OpExtInst %void %1 DebugSource %6
         %46 = OpExtInst %void %1 DebugCompilationUnit 1 4 %45 HLSL
         %47 = OpExtInst %void %1 DebugTypeComposite %7 Structure %45 8 1 %46 %7 %uint_128 FlagIsProtected|FlagIsPrivate %48
         %49 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 Float
         %50 = OpExtInst %void %1 DebugTypeVector %49 4
         %48 = OpExtInst %void %1 DebugTypeMember %9 %50 %45 10 5 %47 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %51 = OpExtInst %void %1 DebugTypeComposite %10 Structure %45 2 1 %46 %10 %uint_256 FlagIsProtected|FlagIsPrivate %52 %53
         %53 = OpExtInst %void %1 DebugTypeMember %11 %50 %45 5 5 %51 %uint_128 %uint_128 FlagIsProtected|FlagIsPrivate
         %52 = OpExtInst %void %1 DebugTypeMember %12 %50 %45 4 5 %51 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %54 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %47 %51
         %55 = OpExtInst %void %1 DebugFunction %13 %54 %45 13 1 %46 %13 FlagIsProtected|FlagIsPrivate 14 %src_MainPs
         %56 = OpExtInst %void %1 DebugLexicalBlock %45 14 1 %55
         %57 = OpExtInst %void %1 DebugLocalVariable %14 %50 %45 16 12 %56 FlagIsLocal
         %58 = OpExtInst %void %1 DebugLocalVariable %15 %47 %45 15 15 %56 FlagIsLocal
         %59 = OpExtInst %void %1 DebugExpression %60 = OpExtInst %void %1 DebugOperation Deref %61 = OpExtInst
%void %1 DebugExpression %60 %62 = OpExtInst %void %1 DebugLocalVariable %16 %51
%45 13 29 %55 FlagIsLocal 1 %63 = OpExtInst %void %1 DebugLocalVariable %14 %50
%45 16 12 %55 FlagIsLocal 1 %MainPs = OpFunction %void None %40 %64 = OpLabel
%param_var_i = OpVariable %_ptr_Function_PS_INPUT Function
         %65 = OpLoad %v4float %in_var_COLOR0
         %66 = OpLoad %v4float %in_var_COLOR1
         %67 = OpCompositeConstruct %PS_INPUT %65 %66
               OpStore %param_var_i %67
         %68 = OpFunctionCall %PS_OUTPUT %src_MainPs %param_var_i
         %69 = OpCompositeExtract %v4float %68 0
               OpStore %out_var_SV_Target0 %69
               OpReturn
               OpFunctionEnd
               OpLine %6 13 1
 %src_MainPs = OpFunction %PS_OUTPUT None %42
         %70 = OpExtInst %void %1 DebugScope %55
               OpLine %6 13 29
          %i = OpFunctionParameter %_ptr_Function_PS_INPUT
         %71 = OpExtInst %void %1 DebugDeclare %62 %i %59
         %72 = OpExtInst %void %1 DebugNoScope
   %bb_entry = OpLabel
         %73 = OpExtInst %void %1 DebugScope %56
  %ps_output = OpVariable %_ptr_Function_PS_OUTPUT Function
          %c = OpVariable %_ptr_Function_v4float Function
         %74 = OpExtInst %void %1 DebugValue %63 %c %61
               OpLine %6 18 9
         %75 = OpAccessChain %_ptr_Function_v4float %i %int_0
               OpLine %6 18 13
         %76 = OpLoad %v4float %75
               OpLine %6 18 5
               OpStore %c %76
;CHECK:        OpStore %c %76
               OpLine %6 19 10
         %77 = OpAccessChain %_ptr_Function_v4float %i %int_1
               OpLine %6 19 14
         %78 = OpLoad %v4float %77
               OpLine %6 19 5
         %79 = OpLoad %v4float %c
;CHECK-NOT:       OpLine %6 19 5
;CHECK-NOT: %79 = OpLoad %v4float %c
               OpLine %6 19 7
         %80 = OpFAdd %v4float %79 %78
;CHECK-NOT: %80 = OpFAdd %v4float %79 %78
;CHECK:     %80 = OpFAdd %v4float %76 %78
               OpLine %6 19 5
               OpStore %c %80
;CHECK:        OpStore %c %80
               OpLine %6 20 5
         %81 = OpLoad %v4float %c
;CHECK-NOT:       OpLine %6 20 5
;CHECK-NOT: %81 = OpLoad %v4float %c
               OpLine %6 20 7
         %82 = OpFDiv %v4float %81 %31
;CHECK-NOT: %82 = OpFDiv %v4float %81 %31
;CHECK:     %82 = OpFDiv %v4float %80 %31
               OpLine %6 20 5
               OpStore %c %82
;CHECK:        OpStore %c %82
               OpLine %6 22 26
         %83 = OpLoad %v4float %c
;CHECK-NOT:       OpLine %6 22 26
;CHECK-NOT: %83 = OpLoad %v4float %c
               OpLine %6 22 5
         %84 = OpAccessChain %_ptr_Function_v4float %ps_output %int_0
               OpStore %84 %83
;CHECK-NOT:    OpStore %84 %83
;CHECK:        OpStore %84 %82
               OpLine %6 23 12
         %85 = OpLoad %PS_OUTPUT %ps_output
               OpLine %6 23 5
               OpReturnValue %85
         %86 = OpExtInst %void %1 DebugNoScope
               OpFunctionEnd
  )";
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<LocalSingleBlockLoadStoreElimPass>(text, false);
}

// TODO(greg-lunarg): Add tests to verify handling of these cases:
//
//    Other target variable types
//    InBounds Access Chains
//    Check for correctness in the presence of function calls
//    Others?

}  // namespace
}  // namespace opt
}  // namespace spvtools
