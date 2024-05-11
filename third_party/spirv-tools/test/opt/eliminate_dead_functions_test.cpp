// Copyright (c) 2017 Google Inc.
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

#include "gmock/gmock.h"
#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using ::testing::HasSubstr;
using EliminateDeadFunctionsBasicTest = PassTest<::testing::Test>;

TEST_F(EliminateDeadFunctionsBasicTest, BasicDeleteDeadFunction) {
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
  SinglePassRunAndCheck<EliminateDeadFunctionsPass>(
      JoinAllInsts(Concat(common_code, dead_function)),
      JoinAllInsts(common_code), /* skip_nop = */ true);
}

TEST_F(EliminateDeadFunctionsBasicTest, BasicKeepLiveFunction) {
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
  auto result = SinglePassRunAndDisassemble<EliminateDeadFunctionsPass>(
      assembly, /* skip_nop = */ true, /* do_validation = */ false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
  EXPECT_EQ(assembly, std::get<0>(result));
}

TEST_F(EliminateDeadFunctionsBasicTest, BasicKeepExportFunctions) {
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
  auto result = SinglePassRunAndDisassemble<EliminateDeadFunctionsPass>(
      assembly, /* skip_nop = */ true, /* do_validation = */ false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
  EXPECT_EQ(assembly, std::get<0>(result));
}

TEST_F(EliminateDeadFunctionsBasicTest, BasicRemoveDecorationsAndNames) {
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
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%main = OpFunction %void None %10
%14 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<EliminateDeadFunctionsPass>(text, expected_output,
                                                    /* skip_nop = */ true);
}

TEST_F(EliminateDeadFunctionsBasicTest, DebugRemoveFunctionFromDebugFunction) {
  // We want to remove id of OpFunction from DebugFunction.
  const std::string text = R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "float4 foo() {
  return 1;
}
float4 main(float4 color : COLOR) : SV_TARGET {
  return foo() + color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "foo"
; CHECK: [[foo:%\d+]] = OpString "foo"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%v4float = OpTypeVector %float 4
%14 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%18 = OpTypeFunction %void
%19 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
; CHECK: [[info_none:%\d+]] = OpExtInst %void %1 DebugInfoNone
%20 = OpExtInst %void %1 DebugSource %5
%21 = OpExtInst %void %1 DebugCompilationUnit 1 4 %20 HLSL
%22 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%23 = OpExtInst %void %1 DebugTypeVector %22 4
%24 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %23 %23
%25 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %23
%26 = OpExtInst %void %1 DebugFunction %7 %24 %20 4 1 %21 %7 FlagIsProtected|FlagIsPrivate 4 %2
%27 = OpExtInst %void %1 DebugFunction %8 %25 %20 1 1 %21 %8 FlagIsProtected|FlagIsPrivate 1 %28
; CHECK: {{%\d+}} = OpExtInst %void %1 DebugFunction [[foo]] {{%\d+}} {{%\d+}} 1 1 {{%\d+}} {{%\d+}} FlagIsProtected|FlagIsPrivate 1 [[info_none]]
%29 = OpExtInst %void %1 DebugLexicalBlock %20 1 14 %27
%40 = OpExtInst %void %1 DebugInlinedAt 4 %26
%2 = OpFunction %void None %18
%30 = OpLabel
%39 = OpVariable %_ptr_Function_v4float Function
%41 = OpExtInst %void %1 DebugScope %27 %40
OpStore %39 %14
%32 = OpLoad %v4float %39
%42 = OpExtInst %void %1 DebugScope %26
%33 = OpLoad %v4float %3
%34 = OpFAdd %v4float %32 %33
OpStore %4 %34
%43 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%28 = OpFunction %v4float None %19
%36 = OpLabel
OpReturnValue %14
OpFunctionEnd
)";

  SinglePassRunAndMatch<EliminateDeadFunctionsPass>(text, false);
}

TEST_F(EliminateDeadFunctionsBasicTest,
       DebugRemoveFunctionUsingExistingDebugInfoNone) {
  // We want to remove id of OpFunction from DebugFunction.
  const std::string text = R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main" %3 %4
OpExecutionMode %2 OriginUpperLeft
%5 = OpString "ps.hlsl"
OpSource HLSL 600 %5 "float4 foo() {
  return 1;
}
float4 main(float4 color : COLOR) : SV_TARGET {
  return foo() + color;
}
"
%6 = OpString "float"
%7 = OpString "main"
%8 = OpString "foo"
; CHECK: [[foo:%\d+]] = OpString "foo"
OpDecorate %3 Location 0
OpDecorate %4 Location 0
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%v4float = OpTypeVector %float 4
%14 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%18 = OpTypeFunction %void
%19 = OpTypeFunction %v4float
%3 = OpVariable %_ptr_Input_v4float Input
%4 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
; CHECK: [[info_none:%\d+]] = OpExtInst %void %1 DebugInfoNone
%20 = OpExtInst %void %1 DebugSource %5
%21 = OpExtInst %void %1 DebugCompilationUnit 1 4 %20 HLSL
%22 = OpExtInst %void %1 DebugTypeBasic %6 %uint_32 Float
%23 = OpExtInst %void %1 DebugTypeVector %22 4
%24 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %23 %23
%25 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %23
%26 = OpExtInst %void %1 DebugFunction %7 %24 %20 4 1 %21 %7 FlagIsProtected|FlagIsPrivate 4 %2
%27 = OpExtInst %void %1 DebugFunction %8 %25 %20 1 1 %21 %8 FlagIsProtected|FlagIsPrivate 1 %28
; CHECK: {{%\d+}} = OpExtInst %void %1 DebugFunction [[foo]] {{%\d+}} {{%\d+}} 1 1 {{%\d+}} {{%\d+}} FlagIsProtected|FlagIsPrivate 1 [[info_none]]
%29 = OpExtInst %void %1 DebugLexicalBlock %20 1 14 %27
%35 = OpExtInst %void %1 DebugInfoNone
%40 = OpExtInst %void %1 DebugInlinedAt 4 %26
%2 = OpFunction %void None %18
%30 = OpLabel
%39 = OpVariable %_ptr_Function_v4float Function
%41 = OpExtInst %void %1 DebugScope %27 %40
OpStore %39 %14
%32 = OpLoad %v4float %39
%42 = OpExtInst %void %1 DebugScope %26
%33 = OpLoad %v4float %3
%34 = OpFAdd %v4float %32 %33
OpStore %4 %34
%43 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
%28 = OpFunction %v4float None %19
%36 = OpLabel
OpReturnValue %14
OpFunctionEnd
)";

  SinglePassRunAndMatch<EliminateDeadFunctionsPass>(text, false);
}

TEST_F(EliminateDeadFunctionsBasicTest, NonSemanticInfoPersists) {
  const std::string text = R"(
; CHECK: [[import:%\w+]] = OpExtInstImport
; CHECK: [[void:%\w+]] = OpTypeVoid
; CHECK-NOT: OpExtInst [[void]] [[import]] 1
; CHECK: OpExtInst [[void]] [[import]] 2
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.Test"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%foo = OpFunction %void None %void_fn
%foo_entry = OpLabel
%non_semantic1 = OpExtInst %void %ext 1
OpReturn
OpFunctionEnd
%non_semantic2 = OpExtInst %void %ext 2
)";

  SinglePassRunAndMatch<EliminateDeadFunctionsPass>(text, true);
}

TEST_F(EliminateDeadFunctionsBasicTest, NonSemanticInfoRemoveDependent) {
  const std::string text = R"(
; CHECK: [[import:%\w+]] = OpExtInstImport
; CHECK: [[void:%\w+]] = OpTypeVoid
; CHECK-NOT: OpExtInst [[void]] [[import]] 1
; CHECK-NOT: OpExtInst [[void]] [[import]] 2
; CHECK: OpExtInst [[void]] [[import]] 3
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.Test"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%foo = OpFunction %void None %void_fn
%foo_entry = OpLabel
%non_semantic1 = OpExtInst %void %ext 1
OpReturn
OpFunctionEnd
%non_semantic2 = OpExtInst %void %ext 2 %foo
%non_semantic3 = OpExtInst %void %ext 3 
)";

  SinglePassRunAndMatch<EliminateDeadFunctionsPass>(text, true);
}

TEST_F(EliminateDeadFunctionsBasicTest, NonSemanticInfoRemoveDependentTree) {
  const std::string text = R"(
; CHECK: [[import:%\w+]] = OpExtInstImport
; CHECK: [[void:%\w+]] = OpTypeVoid
; CHECK-NOT: OpExtInst [[void]] [[import]] 1
; CHECK-NOT: OpExtInst [[void]] [[import]] 2
; CHECK: OpExtInst [[void]] [[import]] 3
; CHECK-NOT: OpExtInst [[void]] [[import]] 4
; CHECK-NOT: OpExtInst [[void]] [[import]] 5
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.Test"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%foo = OpFunction %void None %void_fn
%foo_entry = OpLabel
%non_semantic1 = OpExtInst %void %ext 1
OpReturn
OpFunctionEnd
%non_semantic2 = OpExtInst %void %ext 2 %foo
%non_semantic3 = OpExtInst %void %ext 3 
%non_semantic4 = OpExtInst %void %ext 4 %non_semantic2
%non_semantic5 = OpExtInst %void %ext 5 %non_semantic4
)";

  SinglePassRunAndMatch<EliminateDeadFunctionsPass>(text, true);
}

TEST_F(EliminateDeadFunctionsBasicTest, NonSemanticInfoRemoveDebugPrintf) {
  const std::string text = R"(
; CHECK-NOT: %foo_ = OpFunction %void None % 3
; CHECK-NOT: % 7 = OpLabel
; CHECK-NOT: %c = OpVariable %_ptr_Function_v4float Function
; CHECK-NOT: % 22 = OpAccessChain %_ptr_UniformConstant_13 %samplers %int_0
; CHECK-NOT: % 23 = OpLoad % 13 % 22
; CHECK-NOT: % 27 = OpImageSampleExplicitLod %v4float % 23 % 26 Lod %float_0
; CHECK-NOT: OpStore %c % 27
; CHECK-NOT: % 31 = OpAccessChain %_ptr_Function_float %c %uint_0
; CHECK-NOT: % 32 = OpLoad %float %31
; CHECK-NOT: % 34 = OpExtInst %void %33 1 % 28 % 32
OpCapability RayTracingKHR
OpExtension "SPV_KHR_non_semantic_info"
OpExtension "SPV_KHR_ray_tracing"
%1 = OpExtInstImport "GLSL.std.450"
%33 = OpExtInstImport "NonSemantic.DebugPrintf"
OpMemoryModel Logical GLSL450
OpEntryPoint ClosestHitNV %main "main" %samplers
%28 = OpString "%f"
OpSource GLSL 460
OpSourceExtension "GL_EXT_debug_printf"
OpName %main "main"
OpName %foo_ "foo("
OpName %c "c"
OpName %samplers "samplers"
OpDecorate %samplers DescriptorSet 0
OpDecorate %samplers Binding 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%12 = OpTypeImage %float 3D 0 0 0 1 Unknown
%13 = OpTypeSampledImage %12
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%_arr_13_uint_1 = OpTypeArray %13 %uint_1
%_ptr_UniformConstant__arr_13_uint_1 = OpTypePointer UniformConstant %_arr_13_uint_1
%samplers = OpVariable %_ptr_UniformConstant__arr_13_uint_1 UniformConstant
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%v3float = OpTypeVector %float 3
%float_0 = OpConstant %float 0
%26 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
%main = OpFunction %void None %3
%5 = OpLabel
%36 = OpVariable %_ptr_Function_v4float Function
%38 = OpAccessChain %_ptr_UniformConstant_13 %samplers %int_0
%39 = OpLoad %13 %38
%40 = OpImageSampleExplicitLod %v4float %39 %26 Lod %float_0
OpStore %36 %40
%41 = OpAccessChain %_ptr_Function_float %36 %uint_0
%42 = OpLoad %float %41
%43 = OpExtInst %void %33 1 %28 %42
OpReturn
OpFunctionEnd
%foo_ = OpFunction %void None %3
%7 = OpLabel
%c = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_UniformConstant_13 %samplers %int_0
%23 = OpLoad %13 %22
%27 = OpImageSampleExplicitLod %v4float %23 %26 Lod %float_0
OpStore %c %27
%31 = OpAccessChain %_ptr_Function_float %c %uint_0
%32 = OpLoad %float %31
%34 = OpExtInst %void %33 1 %28 %32
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_VULKAN_1_2);
  SinglePassRunAndMatch<EliminateDeadFunctionsPass>(text, true);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
