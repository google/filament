// Copyright (c) 2023 Google Inc.
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

#include "spirv-tools/optimizer.hpp"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using InterlockInvocationPlacementTest = PassTest<::testing::Test>;

TEST_F(InterlockInvocationPlacementTest, CheckUnchangedIfNotFragment) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %main = OpFunction %void None %1
          %2 = OpLabel
               OpBeginInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
	             OpEndInvocationInterlockEXT
	             OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(
      Pass::Status::SuccessWithoutChange,
      std::get<1>(SinglePassRunAndDisassemble<InvocationInterlockPlacementPass>(
          kTest, /* skip_nop= */ false, /* do_validation= */ false)));
}

TEST_F(InterlockInvocationPlacementTest, CheckUnchangedWithoutCapability) {
  const std::string kTest = R"(
               OpCapability Shader
               OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %main = OpFunction %void None %1
          %2 = OpLabel
               OpBeginInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
	             OpEndInvocationInterlockEXT
	             OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(
      Pass::Status::SuccessWithoutChange,
      std::get<1>(SinglePassRunAndDisassemble<InvocationInterlockPlacementPass>(
          kTest, /* skip_nop= */ false, /* do_validation= */ false)));
}

TEST_F(InterlockInvocationPlacementTest, CheckSingleBasicBlock) {
  // We're using OpNoLine as a generic standin for any other instruction, to
  // test that begin and end aren't moved.
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %main = OpFunction %void None %1
; CHECK: OpLabel
          %2 = OpLabel
; CHECK-NEXT: OpNoLine
               OpNoLine
; CHECK-NEXT: OpBeginInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
	             OpEndInvocationInterlockEXT
	             OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpNoLine
               OpNoLine
; CHECK-NEXT: OpEndInvocationInterlockEXT
               OpEndInvocationInterlockEXT
; CHECK-NEXT: OpNoLine
               OpNoLine
; CHECK-NEXT: OpReturn
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(InterlockInvocationPlacementTest, CheckFunctionCallExtractionBegin) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
        %foo = OpFunction %void None %1
; CHECK: OpLabel
; CHECK-NOT: OpBeginInvocationInterlockEXT
          %2 = OpLabel
               OpBeginInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
               OpReturn
; CHECK: OpFunctionEnd
               OpFunctionEnd
       %main = OpFunction %void None %1
; CHECK: OpLabel
          %3 = OpLabel
; CHECK-NEXT: OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpFunctionCall
          %4 = OpFunctionCall %void %foo
; CHECK-NEXT: OpReturn
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(InterlockInvocationPlacementTest, CheckFunctionCallExtractionEnd) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
        %foo = OpFunction %void None %1
; CHECK: OpLabel
; CHECK-NOT: OpEndInvocationInterlockEXT
          %2 = OpLabel
               OpEndInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
; CHECK: OpFunctionEnd
               OpFunctionEnd
       %main = OpFunction %void None %1
; CHECK: OpLabel
          %3 = OpLabel
; CHECK-NEXT: OpFunctionCall
          %4 = OpFunctionCall %void %foo
; CHECK-NEXT: OpEndInvocationInterlockEXT
; CHECK-NEXT: OpReturn
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(InterlockInvocationPlacementTest,
       CheckFunctionCallExtractionRepeatedCall) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
        %foo = OpFunction %void None %1
; CHECK: OpLabel
; CHECK-NOT: OpBeginInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
          %2 = OpLabel
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
; CHECK: OpFunctionEnd
               OpFunctionEnd
       %main = OpFunction %void None %1
; CHECK: OpLabel
          %3 = OpLabel
; CHECK-NEXT: OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpFunctionCall
          %4 = OpFunctionCall %void %foo
; CHECK-NEXT: OpFunctionCall
          %5 = OpFunctionCall %void %foo
; CHECK-NEXT: OpEndInvocationInterlockEXT
; CHECK-NEXT: OpReturn
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(InterlockInvocationPlacementTest,
       CheckFunctionCallExtractionNestedCall) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
        %foo = OpFunction %void None %1
; CHECK: OpLabel
; CHECK-NOT: OpBeginInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
          %2 = OpLabel
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
; CHECK: OpFunctionEnd
               OpFunctionEnd
        %bar = OpFunction %void None %1
; CHECK: OpLabel
; CHECK-NOT: OpBeginInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
          %3 = OpLabel
          %4 = OpFunctionCall %void %foo
               OpReturn
; CHECK: OpFunctionEnd
               OpFunctionEnd
       %main = OpFunction %void None %1
; CHECK: OpLabel
          %5 = OpLabel
; CHECK-NEXT: OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpFunctionCall
          %6 = OpFunctionCall %void %bar
; CHECK-NEXT: OpEndInvocationInterlockEXT
; CHECK-NEXT: OpReturn
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(InterlockInvocationPlacementTest, CheckLoopExtraction) {
  // Tests that any begin or end instructions in a loop are moved outside of the
  // loop.
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
           %1 = OpTypeFunction %void
       %main = OpFunction %void None %1

          %2 = OpLabel
; CHECK: OpBeginInvocationInterlockEXT
; CHECK-NOT: OpBeginInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
               OpBranch %3

          %3 = OpLabel
               OpLoopMerge %3 %4 None
; CHECK: OpBranchConditional
; CHECK-NOT: OpBeginInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
               OpBranchConditional %true %4 %5

          %4 = OpLabel
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
; CHECK: OpBranch
               OpBranch %3

; CHECK-NEXT: OpLabel
          %5 = OpLabel
; CHECK-NEXT: OpEndInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(InterlockInvocationPlacementTest, CheckAddBeginToElse) {
  // Test that if there is a begin in a single branch of a conditional, begin
  // will be added to the other branch.
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
	             OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
           %1 = OpTypeFunction %void
       %main = OpFunction %void None %1

          %2 = OpLabel
; CHECK-NOT: OpBeginInvocationInterlockEXT
               OpSelectionMerge %5 None
; CHECK: OpBranchConditional
               OpBranchConditional %true %3 %4

; CHECK-NEXT: OpLabel
          %3 = OpLabel
; CHECK-NEXT: OpBeginInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
; CHECK-NEXT: OpBranch
               OpBranch %5

          %4 = OpLabel
; CHECK: OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpBranch
               OpBranch %5

; CHECK-NEXT: OpLabel
          %5 = OpLabel
               OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpEndInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(InterlockInvocationPlacementTest, CheckAddEndToElse) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
	             OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
           %1 = OpTypeFunction %void
       %main = OpFunction %void None %1

          %2 = OpLabel
; CHECK: OpBeginInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpSelectionMerge %5 None
; CHECK: OpBranchConditional
               OpBranchConditional %true %3 %4

; CHECK-NEXT: OpLabel
          %3 = OpLabel
               OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpEndInvocationInterlockEXT
               OpEndInvocationInterlockEXT
; CHECK-NEXT: OpBranch
               OpBranch %5

          %4 = OpLabel
; CHECK: OpEndInvocationInterlockEXT
; CHECK-NEXT: OpBranch
               OpBranch %5

; CHECK-NEXT: OpLabel
          %5 = OpLabel
; CHECK-NOT: OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(InterlockInvocationPlacementTest, CheckSplitIfWithoutElseBegin) {
  // Test that if there is a begin in the then branch of a conditional, and no
  // else branch, an else branch with a begin will created.
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
	             OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
           %1 = OpTypeFunction %void
       %main = OpFunction %void None %1

          %2 = OpLabel
; CHECK-NOT: OpBeginInvocationInterlockEXT
               OpSelectionMerge %5 None
; CHECK: OpBranchConditional
               OpBranchConditional %true %3 %5

; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpBranch

; CHECK-NEXT: OpLabel
          %3 = OpLabel
; CHECK-NEXT: OpBeginInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpBranch %5

; CHECK: OpLabel
          %5 = OpLabel
; CHECK-NOT: OpBeginInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpEndInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(InterlockInvocationPlacementTest, CheckSplitIfWithoutElseEnd) {
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
	             OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
           %1 = OpTypeFunction %void
       %main = OpFunction %void None %1

          %2 = OpLabel

; CHECK: OpBeginInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
               OpEndInvocationInterlockEXT
; CHECK-NEXT: OpSelectionMerge [[merge:%\d+]]
               OpSelectionMerge %5 None
; CHECK-NEXT: OpBranchConditional %true [[then:%\d+]] [[else:%\d+]]
               OpBranchConditional %true %3 %5

; CHECK-NEXT: [[else]] = OpLabel
; CHECK-NEXT: OpEndInvocationInterlockEXT
; CHECK-NEXT: OpBranch [[merge]]

; CHECK-NEXT: [[then]] = OpLabel
          %3 = OpLabel
; CHECK-NEXT: OpEndInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
; CHECK-NEXT: OpBranch [[merge]]
               OpBranch %5

; CHECK-NEXT: [[merge]] = OpLabel
          %5 = OpLabel
; CHECK-NEXT: OpReturn
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(InterlockInvocationPlacementTest, CheckSplitSwitch) {
  // Test that if there is a begin or end in a single branch of a switch, begin
  // or end will be added to all the other branches.
  const std::string kTest = R"(
               OpCapability Shader
               OpCapability FragmentShaderSampleInterlockEXT
	             OpExtension "SPV_EXT_fragment_shader_interlock"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SampleInterlockOrderedEXT
               OpName %main "main"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
           %1 = OpTypeFunction %void
       %main = OpFunction %void None %1

; CHECK: OpLabel
          %2 = OpLabel
; CHECK-NEXT: OpSelectionMerge [[merge:%\d+]]
               OpSelectionMerge %8 None
; CHECK-NEXT: OpSwitch %uint_1 [[default:%\d+]] 0 [[case_0:%\d+]] 1 [[case_1:%\d+]] 2 [[case_2:%\d+]]
               OpSwitch %uint_1 %8 0 %4 1 %5 2 %8

; CHECK-NEXT: [[case_2]] = OpLabel
; CHECK-NEXT: OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpBranch [[merge]]

; CHECK-NEXT: [[default]] = OpLabel
; CHECK-NEXT: OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpBranch [[merge]]

; CHECK-NEXT: [[case_0]] = OpLabel
          %4 = OpLabel
; CHECK-NEXT: OpBeginInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
; CHECK-NEXT: OpNoLine
               OpNoLine
; CHECK-NEXT: OpBranch [[merge]]
               OpBranch %8

; CHECK-NEXT: [[case_1]] = OpLabel
          %5 = OpLabel
; CHECK-NEXT: OpBeginInvocationInterlockEXT
; CHECK-NOT: OpEndInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
               OpEndInvocationInterlockEXT
; CHECK-NEXT: OpNoLine
               OpNoLine
; CHECK-NEXT: OpNoLine
               OpNoLine
; CHECK-NEXT: OpBranch [[merge]]
               OpBranch %8

; CHECK-NEXT: [[merge]] = OpLabel
          %8 = OpLabel
; CHECK-NOT: OpBeginInvocationInterlockEXT
               OpBeginInvocationInterlockEXT
; CHECK-NEXT: OpEndInvocationInterlockEXT
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd
  )";
  SetTargetEnv(SPV_ENV_VULKAN_1_3);
  const auto result = SinglePassRunAndMatch<InvocationInterlockPlacementPass>(
      kTest, /* skip_nop= */ false);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
