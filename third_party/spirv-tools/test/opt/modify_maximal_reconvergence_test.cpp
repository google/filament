// Copyright (c) 2024 Google LLC
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

#include "assembly_builder.h"
#include "pass_fixture.h"
#include "pass_utils.h"

namespace {

using namespace spvtools;

using ModifyMaximalReconvergenceTest = opt::PassTest<::testing::Test>;

TEST_F(ModifyMaximalReconvergenceTest, AddNoEntryPoint) {
  const std::string text = R"(
; CHECK-NOT: OpExtension
OpCapability Kernel
OpCapability Linkage
OpMemoryModel Logical OpenCL
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, true);
}

TEST_F(ModifyMaximalReconvergenceTest, AddSingleEntryPoint) {
  const std::string text = R"(
; CHECK: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK: OpExecutionMode %main MaximallyReconvergesKHR

OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpName %main "main"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, true);
}

TEST_F(ModifyMaximalReconvergenceTest, AddExtensionExists) {
  const std::string text = R"(
; CHECK: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK-NOT: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK: OpExecutionMode %main MaximallyReconvergesKHR

OpCapability Shader
OpExtension "SPV_KHR_maximal_reconvergence"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpName %main "main"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, true);
}

TEST_F(ModifyMaximalReconvergenceTest, AddExecutionModeExists) {
  const std::string text = R"(
; CHECK: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK-NOT: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK: OpExecutionMode %main LocalSize 1 1 1
; CHECK-NEXT: OpExecutionMode %main MaximallyReconvergesKHR
; CHECK-NOT: OpExecutionMode %main MaximallyReconvergesKHR

OpCapability Shader
OpExtension "SPV_KHR_maximal_reconvergence"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionMode %main MaximallyReconvergesKHR
OpName %main "main"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, true);
}

TEST_F(ModifyMaximalReconvergenceTest, AddTwoEntryPoints) {
  const std::string text = R"(
; CHECK: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK: OpExecutionMode %comp MaximallyReconvergesKHR
; CHECK: OpExecutionMode %frag MaximallyReconvergesKHR

OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %comp "main"
OpEntryPoint Fragment %frag "main"
OpExecutionMode %comp LocalSize 1 1 1
OpExecutionMode %frag OriginUpperLeft
OpName %comp "comp"
OpName %frag "frag"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%comp = OpFunction %void None %void_fn
%entry1 = OpLabel
OpReturn
OpFunctionEnd
%frag = OpFunction %void None %void_fn
%entry2 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, true);
}

TEST_F(ModifyMaximalReconvergenceTest, AddTwoEntryPointsOneFunc) {
  const std::string text = R"(
; CHECK: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK: OpExecutionMode %comp MaximallyReconvergesKHR
; CHECK-NOT: OpExecutionMode %comp MaximallyReconvergesKHR

OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %comp "main1"
OpEntryPoint GLCompute %comp "main2"
OpExecutionMode %comp LocalSize 1 1 1
OpName %comp "comp"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%comp = OpFunction %void None %void_fn
%entry1 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, true);
}

TEST_F(ModifyMaximalReconvergenceTest, AddTwoEntryPointsOneExecutionMode) {
  const std::string text = R"(
; CHECK: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK: OpExecutionMode %comp MaximallyReconvergesKHR
; CHECK-NOT: OpExecutionMode %comp MaximallyReconvergesKHR
; CHECK: OpExecutionMode %frag MaximallyReconvergesKHR
; CHECK-NOT: OpExecutionMode %comp MaximallyReconvergesKHR

OpCapability Shader
OpExtension "SPV_KHR_maximal_reconvergence"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %comp "main"
OpEntryPoint Fragment %frag "main"
OpExecutionMode %comp LocalSize 1 1 1
OpExecutionMode %frag OriginUpperLeft
OpExecutionMode %comp MaximallyReconvergesKHR
OpName %comp "comp"
OpName %frag "frag"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%comp = OpFunction %void None %void_fn
%entry1 = OpLabel
OpReturn
OpFunctionEnd
%frag = OpFunction %void None %void_fn
%entry2 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, true);
}

TEST_F(ModifyMaximalReconvergenceTest, RemoveNoEntryPoint) {
  const std::string text = R"(OpCapability Kernel
OpCapability Linkage
OpMemoryModel Logical OpenCL
)";

  SinglePassRunAndCheck<opt::ModifyMaximalReconvergence>(text, text, false,
                                                         true, false);
}

TEST_F(ModifyMaximalReconvergenceTest, RemoveOnlyExtension) {
  const std::string text = R"(
; CHECK-NOT: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK: OpExecutionMode %main LocalSize 1 1 1

OpCapability Shader
OpExtension "SPV_KHR_maximal_reconvergence"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpName %main "main"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, false);
}

TEST_F(ModifyMaximalReconvergenceTest, RemoveSingleEntryPoint) {
  const std::string text = R"(
; CHECK-NOT: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK: OpExecutionMode %main LocalSize 1 1 1
; CHECK-NOT: OpExecutionMode %main MaximallyReconvergesKHR

OpCapability Shader
OpExtension "SPV_KHR_maximal_reconvergence"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionMode %main MaximallyReconvergesKHR
OpName %main "main"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, false);
}

TEST_F(ModifyMaximalReconvergenceTest, RemoveTwoEntryPointsOneExecutionMode) {
  const std::string text = R"(
; CHECK-NOT: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK: OpExecutionMode %comp LocalSize 1 1 1
; CHECK-NEXT: OpExecutionMode %frag OriginUpperLeft
; CHECK-NOT: OpExecutionMode %comp MaximallyReconvergesKHR

OpCapability Shader
OpExtension "SPV_KHR_maximal_reconvergence"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %comp "main"
OpEntryPoint Fragment %frag "main"
OpExecutionMode %comp LocalSize 1 1 1
OpExecutionMode %comp MaximallyReconvergesKHR
OpExecutionMode %frag OriginUpperLeft
OpName %comp "comp"
OpName %frag "frag"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%comp = OpFunction %void None %void_fn
%entry1 = OpLabel
OpReturn
OpFunctionEnd
%frag = OpFunction %void None %void_fn
%entry2 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, false);
}

TEST_F(ModifyMaximalReconvergenceTest, RemoveTwoEntryPoints) {
  const std::string text = R"(
; CHECK-NOT: OpExtension "SPV_KHR_maximal_reconvergence"
; CHECK: OpExecutionMode %comp LocalSize 1 1 1
; CHECK-NEXT: OpExecutionMode %frag OriginUpperLeft
; CHECK-NOT: OpExecutionMode {{%\w}} MaximallyReconvergesKHR

OpCapability Shader
OpExtension "SPV_KHR_maximal_reconvergence"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %comp "main"
OpEntryPoint Fragment %frag "main"
OpExecutionMode %comp LocalSize 1 1 1
OpExecutionMode %comp MaximallyReconvergesKHR
OpExecutionMode %frag OriginUpperLeft
OpExecutionMode %frag MaximallyReconvergesKHR
OpName %comp "comp"
OpName %frag "frag"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%comp = OpFunction %void None %void_fn
%entry1 = OpLabel
OpReturn
OpFunctionEnd
%frag = OpFunction %void None %void_fn
%entry2 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::ModifyMaximalReconvergence>(text, true, false);
}

}  // namespace
