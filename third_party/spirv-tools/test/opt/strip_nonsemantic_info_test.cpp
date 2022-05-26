// Copyright (c) 2018 Google LLC
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
#include "spirv-tools/optimizer.hpp"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using StripNonSemanticInfoTest = PassTest<::testing::Test>;

// This test acts as an end-to-end code example on how to strip
// reflection info from a SPIR-V module.  Use this code pattern
// when you have compiled HLSL code with Glslang or DXC using
// option -fhlsl_functionality1 to insert reflection information,
// but then want to filter out the extra instructions before sending
// it to a driver that does not implement VK_GOOGLE_hlsl_functionality1.
TEST_F(StripNonSemanticInfoTest, StripReflectEnd2EndExample) {
  // This is a non-sensical example, but exercises the instructions.
  std::string before = R"(OpCapability Shader
OpCapability Linkage
OpExtension "SPV_GOOGLE_decorate_string"
OpExtension "SPV_GOOGLE_hlsl_functionality1"
OpMemoryModel Logical Simple
OpDecorateStringGOOGLE %float HlslSemanticGOOGLE "foobar"
OpDecorateStringGOOGLE %void HlslSemanticGOOGLE "my goodness"
%void = OpTypeVoid
%float = OpTypeFloat 32
)";
  SpirvTools tools(SPV_ENV_UNIVERSAL_1_1);
  std::vector<uint32_t> binary_in;
  tools.Assemble(before, &binary_in);

  // Instantiate the optimizer, and run the strip-nonsemantic-info
  // pass over the |binary_in| module, and place the modified module
  // into |binary_out|.
  spvtools::Optimizer optimizer(SPV_ENV_UNIVERSAL_1_1);
  optimizer.RegisterPass(spvtools::CreateStripNonSemanticInfoPass());
  std::vector<uint32_t> binary_out;
  optimizer.Run(binary_in.data(), binary_in.size(), &binary_out);

  // Check results
  std::string disassembly;
  tools.Disassemble(binary_out.data(), binary_out.size(), &disassembly);
  std::string after = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical Simple
%void = OpTypeVoid
%float = OpTypeFloat 32
)";
  EXPECT_THAT(disassembly, testing::Eq(after));
}

// This test is functionally the same as the end-to-end test above,
// but uses the test SinglePassRunAndCheck test fixture instead.
TEST_F(StripNonSemanticInfoTest, StripHlslSemantic) {
  // This is a non-sensical example, but exercises the instructions.
  std::string before = R"(OpCapability Shader
OpCapability Linkage
OpExtension "SPV_GOOGLE_decorate_string"
OpExtension "SPV_GOOGLE_hlsl_functionality1"
OpMemoryModel Logical Simple
OpDecorateStringGOOGLE %float HlslSemanticGOOGLE "foobar"
OpDecorateStringGOOGLE %void HlslSemanticGOOGLE "my goodness"
%void = OpTypeVoid
%float = OpTypeFloat 32
)";
  std::string after = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical Simple
%void = OpTypeVoid
%float = OpTypeFloat 32
)";

  SinglePassRunAndCheck<StripNonSemanticInfoPass>(before, after, false);
}

TEST_F(StripNonSemanticInfoTest, StripHlslCounterBuffer) {
  std::string before = R"(OpCapability Shader
OpCapability Linkage
OpExtension "SPV_GOOGLE_hlsl_functionality1"
OpMemoryModel Logical Simple
OpDecorateId %void HlslCounterBufferGOOGLE %float
%void = OpTypeVoid
%float = OpTypeFloat 32
)";
  std::string after = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical Simple
%void = OpTypeVoid
%float = OpTypeFloat 32
)";

  SinglePassRunAndCheck<StripNonSemanticInfoPass>(before, after, false);
}

TEST_F(StripNonSemanticInfoTest, StripHlslSemanticOnMember) {
  // This is a non-sensical example, but exercises the instructions.
  std::string before = R"(OpCapability Shader
OpCapability Linkage
OpExtension "SPV_GOOGLE_decorate_string"
OpExtension "SPV_GOOGLE_hlsl_functionality1"
OpMemoryModel Logical Simple
OpMemberDecorateStringGOOGLE %struct 0 HlslSemanticGOOGLE "foobar"
%float = OpTypeFloat 32
%_struct_3 = OpTypeStruct %float
)";
  std::string after = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical Simple
%float = OpTypeFloat 32
%_struct_3 = OpTypeStruct %float
)";

  SinglePassRunAndCheck<StripNonSemanticInfoPass>(before, after, false);
}

TEST_F(StripNonSemanticInfoTest, StripNonSemanticImport) {
  std::string text = R"(
; CHECK-NOT: OpExtension "SPV_KHR_non_semantic_info"
; CHECK-NOT: OpExtInstImport
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.Test"
OpMemoryModel Logical GLSL450
)";

  SinglePassRunAndMatch<StripNonSemanticInfoPass>(text, true);
}

TEST_F(StripNonSemanticInfoTest, StripNonSemanticGlobal) {
  std::string text = R"(
; CHECK-NOT: OpExtInst
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.Test"
OpMemoryModel Logical GLSL450
%void = OpTypeVoid
%1 = OpExtInst %void %ext 1
)";

  SinglePassRunAndMatch<StripNonSemanticInfoPass>(text, true);
}

TEST_F(StripNonSemanticInfoTest, StripNonSemanticInFunction) {
  std::string text = R"(
; CHECK-NOT: OpExtInst
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.Test"
OpMemoryModel Logical GLSL450
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
%1 = OpExtInst %void %ext 1 %foo
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<StripNonSemanticInfoPass>(text, true);
}

TEST_F(StripNonSemanticInfoTest, StripNonSemanticAfterFunction) {
  std::string text = R"(
; CHECK-NOT: OpExtInst
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.Test"
OpMemoryModel Logical GLSL450
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%1 = OpExtInst %void %ext 1 %foo
)";

  SinglePassRunAndMatch<StripNonSemanticInfoPass>(text, true);
}

TEST_F(StripNonSemanticInfoTest, StripNonSemanticBetweenFunctions) {
  std::string text = R"(
; CHECK-NOT: OpExtInst
OpCapability Shader
OpCapability Linkage
OpExtension "SPV_KHR_non_semantic_info"
%ext = OpExtInstImport "NonSemantic.Test"
OpMemoryModel Logical GLSL450
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%foo = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%1 = OpExtInst %void %ext 1 %foo
%bar = OpFunction %void None %void_fn
%bar_entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<StripNonSemanticInfoPass>(text, true);
}

// Make sure that strip reflect does not remove the debug info (OpString and
// OpLine).
TEST_F(StripNonSemanticInfoTest, DontStripDebug) {
  std::string text = R"(OpCapability Shader
OpMemoryModel Logical Simple
OpEntryPoint Fragment %1 "main"
OpExecutionMode %1 OriginUpperLeft
%2 = OpString "file"
%void = OpTypeVoid
%4 = OpTypeFunction %void
%1 = OpFunction %void None %4
%5 = OpLabel
OpLine %2 1 1
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<StripNonSemanticInfoPass>(text, text, false);
}

TEST_F(StripNonSemanticInfoTest, RemovedNonSemanticDebugInfo) {
  const std::string text = R"(
;CHECK-NOT: OpExtension "SPV_KHR_non_semantic_info
;CHECK-NOT: OpExtInstImport "NonSemantic.Shader.DebugInfo.100
;CHECK-NOT: OpExtInst %void {{%\w+}} DebugSource
;CHECK-NOT: OpExtInst %void {{%\w+}} DebugLine
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %PSMain "PSMain" %in_var_COLOR %out_var_SV_TARGET
               OpExecutionMode %PSMain OriginUpperLeft
          %5 = OpString "t.hlsl"
          %6 = OpString "float"
          %7 = OpString "color"
          %8 = OpString "PSInput"
          %9 = OpString "PSMain"
         %10 = OpString ""
         %11 = OpString "input"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %out_var_SV_TARGET "out.var.SV_TARGET"
               OpName %PSMain "PSMain"
               OpDecorate %in_var_COLOR Location 0
               OpDecorate %out_var_SV_TARGET Location 0
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
     %uint_1 = OpConstant %uint 1
     %uint_9 = OpConstant %uint 9
         %21 = OpTypeFunction %void
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
         %13 = OpExtInst %void %1 DebugSource %5
     %PSMain = OpFunction %void None %21
         %22 = OpLabel
         %23 = OpLoad %v4float %in_var_COLOR
               OpStore %out_var_SV_TARGET %23
         %24 = OpExtInst %void %1 DebugLine %13 %uint_9 %uint_9 %uint_1 %uint_1
               OpReturn
               OpFunctionEnd
)";
  SinglePassRunAndMatch<StripNonSemanticInfoPass>(text, true);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
