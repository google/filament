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

#include "source/opt/struct_cfg_analysis.h"

#include <string>

#include "gmock/gmock.h"
#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using StructCFGAnalysisTest = PassTest<::testing::Test>;
using ::testing::UnorderedElementsAre;

TEST_F(StructCFGAnalysisTest, BBInSelection) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%1 = OpLabel
OpSelectionMerge %3 None
OpBranchConditional %undef_bool %2 %3
%2 = OpLabel
OpBranch %3
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The header is not in the construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // BB2 is in the construct.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 0);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 0);
  EXPECT_EQ(analysis.ContainingSwitch(2), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The merge node is not in the construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));
}

TEST_F(StructCFGAnalysisTest, BBInLoop) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%entry_lab = OpLabel
OpBranch %1
%1 = OpLabel
OpLoopMerge %3 %4 None
OpBranchConditional %undef_bool %2 %3
%2 = OpLabel
OpBranch %3
%4 = OpLabel
OpBranch %1
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The header is not in the construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // BB2 is in the construct.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 1);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 1);
  EXPECT_EQ(analysis.ContainingSwitch(2), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The merge node is not in the construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));

  // The continue block is in the construct.
  EXPECT_EQ(analysis.ContainingConstruct(4), 1);
  EXPECT_EQ(analysis.ContainingLoop(4), 1);
  EXPECT_EQ(analysis.MergeBlock(4), 3);
  EXPECT_EQ(analysis.NestingDepth(4), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(4), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(4), 1);
  EXPECT_EQ(analysis.ContainingSwitch(4), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(4), 0);
  EXPECT_TRUE(analysis.IsContinueBlock(4));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(4));
  EXPECT_TRUE(analysis.IsInContinueConstruct(4));
  EXPECT_FALSE(analysis.IsMergeBlock(4));
}

TEST_F(StructCFGAnalysisTest, SelectionInLoop) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%entry_lab = OpLabel
OpBranch %1
%1 = OpLabel
OpLoopMerge %3 %4 None
OpBranchConditional %undef_bool %2 %3
%2 = OpLabel
OpSelectionMerge %6 None
OpBranchConditional %undef_bool %5 %6
%5 = OpLabel
OpBranch %6
%6 = OpLabel
OpBranch %3
%4 = OpLabel
OpBranch %1
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The loop header is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // Selection header is in the loop only.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 1);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 1);
  EXPECT_EQ(analysis.ContainingSwitch(2), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The loop merge node is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));

  // The continue block is in the loop only.
  EXPECT_EQ(analysis.ContainingConstruct(4), 1);
  EXPECT_EQ(analysis.ContainingLoop(4), 1);
  EXPECT_EQ(analysis.MergeBlock(4), 3);
  EXPECT_EQ(analysis.NestingDepth(4), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(4), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(4), 1);
  EXPECT_EQ(analysis.ContainingSwitch(4), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(4), 0);
  EXPECT_TRUE(analysis.IsContinueBlock(4));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(4));
  EXPECT_TRUE(analysis.IsInContinueConstruct(4));
  EXPECT_FALSE(analysis.IsMergeBlock(4));

  // BB5 is in the selection and the loop.
  EXPECT_EQ(analysis.ContainingConstruct(5), 2);
  EXPECT_EQ(analysis.ContainingLoop(5), 1);
  EXPECT_EQ(analysis.MergeBlock(5), 6);
  EXPECT_EQ(analysis.NestingDepth(5), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(5), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(5), 1);
  EXPECT_EQ(analysis.ContainingSwitch(5), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(5), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(5));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(5));
  EXPECT_FALSE(analysis.IsInContinueConstruct(5));
  EXPECT_FALSE(analysis.IsMergeBlock(5));

  // The selection merge is in the loop only.
  EXPECT_EQ(analysis.ContainingConstruct(6), 1);
  EXPECT_EQ(analysis.ContainingLoop(6), 1);
  EXPECT_EQ(analysis.MergeBlock(6), 3);
  EXPECT_EQ(analysis.NestingDepth(6), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(6), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(6), 1);
  EXPECT_EQ(analysis.ContainingSwitch(6), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(6), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(6));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(6));
  EXPECT_FALSE(analysis.IsInContinueConstruct(6));
  EXPECT_TRUE(analysis.IsMergeBlock(6));
}

TEST_F(StructCFGAnalysisTest, LoopInSelection) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%entry_lab = OpLabel
OpBranch %1
%1 = OpLabel
OpSelectionMerge %3 None
OpBranchConditional %undef_bool %2 %3
%2 = OpLabel
OpLoopMerge %4 %5 None
OpBranchConditional %undef_bool %4 %6
%5 = OpLabel
OpBranch %2
%6 = OpLabel
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The selection header is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // Loop header is in the selection only.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 0);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 0);
  EXPECT_EQ(analysis.ContainingSwitch(2), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The selection merge node is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));

  // The loop merge is in the selection only.
  EXPECT_EQ(analysis.ContainingConstruct(4), 1);
  EXPECT_EQ(analysis.ContainingLoop(4), 0);
  EXPECT_EQ(analysis.MergeBlock(4), 3);
  EXPECT_EQ(analysis.NestingDepth(4), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(4), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(4), 0);
  EXPECT_EQ(analysis.ContainingSwitch(4), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(4), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(4));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(4));
  EXPECT_FALSE(analysis.IsInContinueConstruct(4));
  EXPECT_TRUE(analysis.IsMergeBlock(4));

  // The loop continue target is in the loop.
  EXPECT_EQ(analysis.ContainingConstruct(5), 2);
  EXPECT_EQ(analysis.ContainingLoop(5), 2);
  EXPECT_EQ(analysis.MergeBlock(5), 4);
  EXPECT_EQ(analysis.NestingDepth(5), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(5), 4);
  EXPECT_EQ(analysis.LoopNestingDepth(5), 1);
  EXPECT_EQ(analysis.ContainingSwitch(5), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(5), 0);
  EXPECT_TRUE(analysis.IsContinueBlock(5));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(5));
  EXPECT_TRUE(analysis.IsInContinueConstruct(5));
  EXPECT_FALSE(analysis.IsMergeBlock(5));

  // BB6 is in the loop.
  EXPECT_EQ(analysis.ContainingConstruct(6), 2);
  EXPECT_EQ(analysis.ContainingLoop(6), 2);
  EXPECT_EQ(analysis.MergeBlock(6), 4);
  EXPECT_EQ(analysis.NestingDepth(6), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(6), 4);
  EXPECT_EQ(analysis.LoopNestingDepth(6), 1);
  EXPECT_EQ(analysis.ContainingSwitch(6), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(6), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(6));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(6));
  EXPECT_FALSE(analysis.IsInContinueConstruct(6));
  EXPECT_FALSE(analysis.IsMergeBlock(6));
}

TEST_F(StructCFGAnalysisTest, SelectionInSelection) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%entry_lab = OpLabel
OpBranch %1
%1 = OpLabel
OpSelectionMerge %3 None
OpBranchConditional %undef_bool %2 %3
%2 = OpLabel
OpSelectionMerge %4 None
OpBranchConditional %undef_bool %4 %5
%5 = OpLabel
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The outer selection header is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // The inner header is in the outer selection.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 0);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 0);
  EXPECT_EQ(analysis.ContainingSwitch(2), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The outer merge node is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));

  // The inner merge is in the outer selection.
  EXPECT_EQ(analysis.ContainingConstruct(4), 1);
  EXPECT_EQ(analysis.ContainingLoop(4), 0);
  EXPECT_EQ(analysis.MergeBlock(4), 3);
  EXPECT_EQ(analysis.NestingDepth(4), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(4), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(4), 0);
  EXPECT_EQ(analysis.ContainingSwitch(4), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(4), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(4));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(4));
  EXPECT_FALSE(analysis.IsInContinueConstruct(4));
  EXPECT_TRUE(analysis.IsMergeBlock(4));

  // BB5 is in the inner selection.
  EXPECT_EQ(analysis.ContainingConstruct(5), 2);
  EXPECT_EQ(analysis.ContainingLoop(5), 0);
  EXPECT_EQ(analysis.MergeBlock(5), 4);
  EXPECT_EQ(analysis.NestingDepth(5), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(5), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(5), 0);
  EXPECT_EQ(analysis.ContainingSwitch(5), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(5), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(5));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(5));
  EXPECT_FALSE(analysis.IsInContinueConstruct(5));
  EXPECT_FALSE(analysis.IsMergeBlock(5));
}

TEST_F(StructCFGAnalysisTest, LoopInLoop) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%entry_lab = OpLabel
OpBranch %1
%1 = OpLabel
OpLoopMerge %3 %7 None
OpBranchConditional %undef_bool %2 %3
%2 = OpLabel
OpLoopMerge %4 %5 None
OpBranchConditional %undef_bool %4 %6
%5 = OpLabel
OpBranch %2
%6 = OpLabel
OpBranch %4
%4 = OpLabel
OpBranch %3
%7 = OpLabel
OpBranch %1
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The outer loop header is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // The inner loop header is in the outer loop.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 1);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 1);
  EXPECT_EQ(analysis.ContainingSwitch(2), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The outer merge node is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));

  // The inner merge is in the outer loop.
  EXPECT_EQ(analysis.ContainingConstruct(4), 1);
  EXPECT_EQ(analysis.ContainingLoop(4), 1);
  EXPECT_EQ(analysis.MergeBlock(4), 3);
  EXPECT_EQ(analysis.NestingDepth(4), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(4), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(4), 1);
  EXPECT_EQ(analysis.ContainingSwitch(4), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(4), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(4));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(4));
  EXPECT_FALSE(analysis.IsInContinueConstruct(4));
  EXPECT_TRUE(analysis.IsMergeBlock(4));

  // The inner continue target is in the inner loop.
  EXPECT_EQ(analysis.ContainingConstruct(5), 2);
  EXPECT_EQ(analysis.ContainingLoop(5), 2);
  EXPECT_EQ(analysis.MergeBlock(5), 4);
  EXPECT_EQ(analysis.NestingDepth(5), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(5), 4);
  EXPECT_EQ(analysis.LoopNestingDepth(5), 2);
  EXPECT_EQ(analysis.ContainingSwitch(5), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(5), 0);
  EXPECT_TRUE(analysis.IsContinueBlock(5));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(5));
  EXPECT_TRUE(analysis.IsInContinueConstruct(5));
  EXPECT_FALSE(analysis.IsMergeBlock(5));

  // BB6 is in the loop.
  EXPECT_EQ(analysis.ContainingConstruct(6), 2);
  EXPECT_EQ(analysis.ContainingLoop(6), 2);
  EXPECT_EQ(analysis.MergeBlock(6), 4);
  EXPECT_EQ(analysis.NestingDepth(6), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(6), 4);
  EXPECT_EQ(analysis.LoopNestingDepth(6), 2);
  EXPECT_EQ(analysis.ContainingSwitch(6), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(6), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(6));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(6));
  EXPECT_FALSE(analysis.IsInContinueConstruct(6));
  EXPECT_FALSE(analysis.IsMergeBlock(6));

  // The outer continue target is in the outer loop.
  EXPECT_EQ(analysis.ContainingConstruct(7), 1);
  EXPECT_EQ(analysis.ContainingLoop(7), 1);
  EXPECT_EQ(analysis.MergeBlock(7), 3);
  EXPECT_EQ(analysis.NestingDepth(7), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(7), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(7), 1);
  EXPECT_EQ(analysis.ContainingSwitch(7), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(7), 0);
  EXPECT_TRUE(analysis.IsContinueBlock(7));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(7));
  EXPECT_TRUE(analysis.IsInContinueConstruct(7));
  EXPECT_FALSE(analysis.IsMergeBlock(7));
}

TEST_F(StructCFGAnalysisTest, KernelTest) {
  const std::string text = R"(
OpCapability Kernel
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%1 = OpLabel
OpBranchConditional %undef_bool %2 %3
%2 = OpLabel
OpBranch %3
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // No structured control flow, so none of the basic block are in any
  // construct.
  for (uint32_t i = 1; i <= 3; i++) {
    EXPECT_EQ(analysis.ContainingConstruct(i), 0);
    EXPECT_EQ(analysis.ContainingLoop(i), 0);
    EXPECT_EQ(analysis.MergeBlock(i), 0);
    EXPECT_EQ(analysis.NestingDepth(i), 0);
    EXPECT_EQ(analysis.LoopMergeBlock(i), 0);
    EXPECT_EQ(analysis.LoopNestingDepth(i), 0);
    EXPECT_EQ(analysis.ContainingSwitch(i), 0);
    EXPECT_EQ(analysis.SwitchMergeBlock(i), 0);
    EXPECT_FALSE(analysis.IsContinueBlock(i));
    EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(i));
    EXPECT_FALSE(analysis.IsInContinueConstruct(i));
    EXPECT_FALSE(analysis.IsMergeBlock(i));
  }
}

TEST_F(StructCFGAnalysisTest, EmptyFunctionTest) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %func LinkageAttributes "x" Import
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%func = OpFunction %void None %void_fn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  // #2451: This segfaulted on empty functions.
  StructuredCFGAnalysis analysis(context.get());
}

TEST_F(StructCFGAnalysisTest, BBInSwitch) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%1 = OpLabel
OpSelectionMerge %3 None
OpSwitch %uint_undef %2 0 %3
%2 = OpLabel
OpBranch %3
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The header is not in the construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // BB2 is in the construct.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 0);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 0);
  EXPECT_EQ(analysis.ContainingSwitch(2), 1);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 3);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The merge node is not in the construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));
}

TEST_F(StructCFGAnalysisTest, LoopInSwitch) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%entry_lab = OpLabel
OpBranch %1
%1 = OpLabel
OpSelectionMerge %3 None
OpSwitch %uint_undef %2 1 %3
%2 = OpLabel
OpLoopMerge %4 %5 None
OpBranchConditional %undef_bool %4 %6
%5 = OpLabel
OpBranch %2
%6 = OpLabel
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The selection header is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // Loop header is in the selection only.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 0);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 0);
  EXPECT_EQ(analysis.ContainingSwitch(2), 1);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 3);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The selection merge node is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));

  // The loop merge is in the selection only.
  EXPECT_EQ(analysis.ContainingConstruct(4), 1);
  EXPECT_EQ(analysis.ContainingLoop(4), 0);
  EXPECT_EQ(analysis.MergeBlock(4), 3);
  EXPECT_EQ(analysis.NestingDepth(4), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(4), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(4), 0);
  EXPECT_EQ(analysis.ContainingSwitch(4), 1);
  EXPECT_EQ(analysis.SwitchMergeBlock(4), 3);
  EXPECT_FALSE(analysis.IsContinueBlock(4));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(4));
  EXPECT_FALSE(analysis.IsInContinueConstruct(4));
  EXPECT_TRUE(analysis.IsMergeBlock(4));

  // The loop continue target is in the loop.
  EXPECT_EQ(analysis.ContainingConstruct(5), 2);
  EXPECT_EQ(analysis.ContainingLoop(5), 2);
  EXPECT_EQ(analysis.MergeBlock(5), 4);
  EXPECT_EQ(analysis.NestingDepth(5), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(5), 4);
  EXPECT_EQ(analysis.LoopNestingDepth(5), 1);
  EXPECT_EQ(analysis.ContainingSwitch(5), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(5), 0);
  EXPECT_TRUE(analysis.IsContinueBlock(5));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(5));
  EXPECT_TRUE(analysis.IsInContinueConstruct(5));
  EXPECT_FALSE(analysis.IsMergeBlock(5));

  // BB6 is in the loop.
  EXPECT_EQ(analysis.ContainingConstruct(6), 2);
  EXPECT_EQ(analysis.ContainingLoop(6), 2);
  EXPECT_EQ(analysis.MergeBlock(6), 4);
  EXPECT_EQ(analysis.NestingDepth(6), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(6), 4);
  EXPECT_EQ(analysis.LoopNestingDepth(6), 1);
  EXPECT_EQ(analysis.ContainingSwitch(6), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(6), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(6));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(6));
  EXPECT_FALSE(analysis.IsInContinueConstruct(6));
  EXPECT_FALSE(analysis.IsMergeBlock(6));
}

TEST_F(StructCFGAnalysisTest, SelectionInSwitch) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%entry_lab = OpLabel
OpBranch %1
%1 = OpLabel
OpSelectionMerge %3 None
OpSwitch %uint_undef %2 10 %3
%2 = OpLabel
OpSelectionMerge %4 None
OpBranchConditional %undef_bool %4 %5
%5 = OpLabel
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The outer selection header is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // The inner header is in the outer selection.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 0);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 0);
  EXPECT_EQ(analysis.ContainingSwitch(2), 1);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 3);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The outer merge node is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));

  // The inner merge is in the outer selection.
  EXPECT_EQ(analysis.ContainingConstruct(4), 1);
  EXPECT_EQ(analysis.ContainingLoop(4), 0);
  EXPECT_EQ(analysis.MergeBlock(4), 3);
  EXPECT_EQ(analysis.NestingDepth(4), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(4), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(4), 0);
  EXPECT_EQ(analysis.ContainingSwitch(4), 1);
  EXPECT_EQ(analysis.SwitchMergeBlock(4), 3);
  EXPECT_FALSE(analysis.IsContinueBlock(4));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(4));
  EXPECT_FALSE(analysis.IsInContinueConstruct(4));
  EXPECT_TRUE(analysis.IsMergeBlock(4));

  // BB5 is in the inner selection.
  EXPECT_EQ(analysis.ContainingConstruct(5), 2);
  EXPECT_EQ(analysis.ContainingLoop(5), 0);
  EXPECT_EQ(analysis.MergeBlock(5), 4);
  EXPECT_EQ(analysis.NestingDepth(5), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(5), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(5), 0);
  EXPECT_EQ(analysis.ContainingSwitch(5), 1);
  EXPECT_EQ(analysis.SwitchMergeBlock(5), 3);
  EXPECT_FALSE(analysis.IsContinueBlock(5));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(5));
  EXPECT_FALSE(analysis.IsInContinueConstruct(5));
  EXPECT_FALSE(analysis.IsMergeBlock(5));
}

TEST_F(StructCFGAnalysisTest, SwitchInSelection) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%entry_lab = OpLabel
OpBranch %1
%1 = OpLabel
OpSelectionMerge %3 None
OpBranchConditional %undef_bool %2 %3
%2 = OpLabel
OpSelectionMerge %4 None
OpSwitch %uint_undef %4 7 %5
%5 = OpLabel
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The outer selection header is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // The inner header is in the outer selection.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 0);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 0);
  EXPECT_EQ(analysis.ContainingSwitch(2), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The outer merge node is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));

  // The inner merge is in the outer selection.
  EXPECT_EQ(analysis.ContainingConstruct(4), 1);
  EXPECT_EQ(analysis.ContainingLoop(4), 0);
  EXPECT_EQ(analysis.MergeBlock(4), 3);
  EXPECT_EQ(analysis.NestingDepth(4), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(4), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(4), 0);
  EXPECT_EQ(analysis.ContainingSwitch(4), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(4), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(4));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(4));
  EXPECT_FALSE(analysis.IsInContinueConstruct(4));
  EXPECT_TRUE(analysis.IsMergeBlock(4));

  // BB5 is in the inner selection.
  EXPECT_EQ(analysis.ContainingConstruct(5), 2);
  EXPECT_EQ(analysis.ContainingLoop(5), 0);
  EXPECT_EQ(analysis.MergeBlock(5), 4);
  EXPECT_EQ(analysis.NestingDepth(5), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(5), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(5), 0);
  EXPECT_EQ(analysis.ContainingSwitch(5), 2);
  EXPECT_EQ(analysis.SwitchMergeBlock(5), 4);
  EXPECT_FALSE(analysis.IsContinueBlock(5));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(5));
  EXPECT_FALSE(analysis.IsInContinueConstruct(5));
  EXPECT_FALSE(analysis.IsMergeBlock(5));
}

TEST_F(StructCFGAnalysisTest, SelectionInContinue) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%entry_lab = OpLabel
OpBranch %1
%1 = OpLabel
OpLoopMerge %3 %4 None
OpBranchConditional %undef_bool %2 %3
%2 = OpLabel
OpBranch %3
%4 = OpLabel
OpSelectionMerge %6 None
OpBranchConditional %undef_bool %5 %6
%5 = OpLabel
OpBranch %6
%6 = OpLabel
OpBranch %1
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The loop header is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // Selection header is in the loop only.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 1);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 1);
  EXPECT_EQ(analysis.ContainingSwitch(2), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The loop merge node is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));

  // The continue block is in the loop only.
  EXPECT_EQ(analysis.ContainingConstruct(4), 1);
  EXPECT_EQ(analysis.ContainingLoop(4), 1);
  EXPECT_EQ(analysis.MergeBlock(4), 3);
  EXPECT_EQ(analysis.NestingDepth(4), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(4), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(4), 1);
  EXPECT_EQ(analysis.ContainingSwitch(4), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(4), 0);
  EXPECT_TRUE(analysis.IsContinueBlock(4));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(4));
  EXPECT_TRUE(analysis.IsInContinueConstruct(4));
  EXPECT_FALSE(analysis.IsMergeBlock(4));

  // BB5 is in the selection and the continue for the loop.
  EXPECT_EQ(analysis.ContainingConstruct(5), 4);
  EXPECT_EQ(analysis.ContainingLoop(5), 1);
  EXPECT_EQ(analysis.MergeBlock(5), 6);
  EXPECT_EQ(analysis.NestingDepth(5), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(5), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(5), 1);
  EXPECT_EQ(analysis.ContainingSwitch(5), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(5), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(5));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(5));
  EXPECT_TRUE(analysis.IsInContinueConstruct(5));
  EXPECT_FALSE(analysis.IsMergeBlock(5));

  // BB5 is in the continue for the loop.
  EXPECT_EQ(analysis.ContainingConstruct(6), 1);
  EXPECT_EQ(analysis.ContainingLoop(6), 1);
  EXPECT_EQ(analysis.MergeBlock(6), 3);
  EXPECT_EQ(analysis.NestingDepth(6), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(6), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(6), 1);
  EXPECT_EQ(analysis.ContainingSwitch(6), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(6), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(6));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(6));
  EXPECT_TRUE(analysis.IsInContinueConstruct(6));
  EXPECT_TRUE(analysis.IsMergeBlock(6));
}

TEST_F(StructCFGAnalysisTest, LoopInContinue) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
%void = OpTypeVoid
%bool = OpTypeBool
%bool_undef = OpUndef %bool
%uint = OpTypeInt 32 0
%uint_undef = OpUndef %uint
%void_func = OpTypeFunction %void
%main = OpFunction %void None %void_func
%entry_lab = OpLabel
OpBranch %1
%1 = OpLabel
OpLoopMerge %3 %7 None
OpBranchConditional %undef_bool %2 %3
%2 = OpLabel
OpBranchConditional %undef_bool %3 %7
%7 = OpLabel
OpLoopMerge %4 %5 None
OpBranchConditional %undef_bool %4 %6
%5 = OpLabel
OpBranch %7
%6 = OpLabel
OpBranch %4
%4 = OpLabel
OpBranch %1
%3 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  // The outer loop header is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(1), 0);
  EXPECT_EQ(analysis.ContainingLoop(1), 0);
  EXPECT_EQ(analysis.MergeBlock(1), 0);
  EXPECT_EQ(analysis.NestingDepth(1), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(1), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(1), 0);
  EXPECT_EQ(analysis.ContainingSwitch(1), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(1), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(1));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(1));
  EXPECT_FALSE(analysis.IsInContinueConstruct(1));
  EXPECT_FALSE(analysis.IsMergeBlock(1));

  // BB2 is a regular block in the inner loop.
  EXPECT_EQ(analysis.ContainingConstruct(2), 1);
  EXPECT_EQ(analysis.ContainingLoop(2), 1);
  EXPECT_EQ(analysis.MergeBlock(2), 3);
  EXPECT_EQ(analysis.NestingDepth(2), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(2), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(2), 1);
  EXPECT_EQ(analysis.ContainingSwitch(2), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(2), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(2));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(2));
  EXPECT_FALSE(analysis.IsInContinueConstruct(2));
  EXPECT_FALSE(analysis.IsMergeBlock(2));

  // The outer merge node is not in either construct.
  EXPECT_EQ(analysis.ContainingConstruct(3), 0);
  EXPECT_EQ(analysis.ContainingLoop(3), 0);
  EXPECT_EQ(analysis.MergeBlock(3), 0);
  EXPECT_EQ(analysis.NestingDepth(3), 0);
  EXPECT_EQ(analysis.LoopMergeBlock(3), 0);
  EXPECT_EQ(analysis.LoopNestingDepth(3), 0);
  EXPECT_EQ(analysis.ContainingSwitch(3), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(3), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(3));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(3));
  EXPECT_FALSE(analysis.IsInContinueConstruct(3));
  EXPECT_TRUE(analysis.IsMergeBlock(3));

  // The inner merge is in the continue of the outer loop.
  EXPECT_EQ(analysis.ContainingConstruct(4), 1);
  EXPECT_EQ(analysis.ContainingLoop(4), 1);
  EXPECT_EQ(analysis.MergeBlock(4), 3);
  EXPECT_EQ(analysis.NestingDepth(4), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(4), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(4), 1);
  EXPECT_EQ(analysis.ContainingSwitch(4), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(4), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(4));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(4));
  EXPECT_TRUE(analysis.IsInContinueConstruct(4));
  EXPECT_TRUE(analysis.IsMergeBlock(4));

  // The inner continue target is in the inner loop.
  EXPECT_EQ(analysis.ContainingConstruct(5), 7);
  EXPECT_EQ(analysis.ContainingLoop(5), 7);
  EXPECT_EQ(analysis.MergeBlock(5), 4);
  EXPECT_EQ(analysis.NestingDepth(5), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(5), 4);
  EXPECT_EQ(analysis.LoopNestingDepth(5), 2);
  EXPECT_EQ(analysis.ContainingSwitch(5), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(5), 0);
  EXPECT_TRUE(analysis.IsContinueBlock(5));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(5));
  EXPECT_TRUE(analysis.IsInContinueConstruct(5));
  EXPECT_FALSE(analysis.IsMergeBlock(5));

  // BB6 is a regular block in the inner loop.
  EXPECT_EQ(analysis.ContainingConstruct(6), 7);
  EXPECT_EQ(analysis.ContainingLoop(6), 7);
  EXPECT_EQ(analysis.MergeBlock(6), 4);
  EXPECT_EQ(analysis.NestingDepth(6), 2);
  EXPECT_EQ(analysis.LoopMergeBlock(6), 4);
  EXPECT_EQ(analysis.LoopNestingDepth(6), 2);
  EXPECT_EQ(analysis.ContainingSwitch(6), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(6), 0);
  EXPECT_FALSE(analysis.IsContinueBlock(6));
  EXPECT_FALSE(analysis.IsInContainingLoopsContinueConstruct(6));
  EXPECT_TRUE(analysis.IsInContinueConstruct(6));
  EXPECT_FALSE(analysis.IsMergeBlock(6));

  // The outer continue target is in the outer loop.
  EXPECT_EQ(analysis.ContainingConstruct(7), 1);
  EXPECT_EQ(analysis.ContainingLoop(7), 1);
  EXPECT_EQ(analysis.MergeBlock(7), 3);
  EXPECT_EQ(analysis.NestingDepth(7), 1);
  EXPECT_EQ(analysis.LoopMergeBlock(7), 3);
  EXPECT_EQ(analysis.LoopNestingDepth(7), 1);
  EXPECT_EQ(analysis.ContainingSwitch(7), 0);
  EXPECT_EQ(analysis.SwitchMergeBlock(7), 0);
  EXPECT_TRUE(analysis.IsContinueBlock(7));
  EXPECT_TRUE(analysis.IsInContainingLoopsContinueConstruct(7));
  EXPECT_TRUE(analysis.IsInContinueConstruct(7));
  EXPECT_FALSE(analysis.IsMergeBlock(7));
}

TEST_F(StructCFGAnalysisTest, FuncCallInContinueDirect) {
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main"
       %void = OpTypeVoid
       %bool = OpTypeBool
          %4 = OpUndef %bool
       %uint = OpTypeInt 32 0
          %6 = OpUndef %uint
          %7 = OpTypeFunction %void
          %1 = OpFunction %void None %7
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranchConditional %12 %10 %11
         %11 = OpLabel
         %13 = OpFunctionCall %void %14
               OpBranch %9
         %10 = OpLabel
         %15 = OpFunctionCall %void %16
               OpReturn
               OpFunctionEnd
         %14 = OpFunction %void None %7
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
         %16 = OpFunction %void None %7
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  auto c = analysis.FindFuncsCalledFromContinue();
  EXPECT_THAT(c, UnorderedElementsAre(14u));
}

TEST_F(StructCFGAnalysisTest, FuncCallInContinueIndirect) {
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main"
       %void = OpTypeVoid
       %bool = OpTypeBool
          %4 = OpUndef %bool
       %uint = OpTypeInt 32 0
          %6 = OpUndef %uint
          %7 = OpTypeFunction %void
          %1 = OpFunction %void None %7
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranchConditional %12 %10 %11
         %11 = OpLabel
         %13 = OpFunctionCall %void %14
               OpBranch %9
         %10 = OpLabel
         %15 = OpFunctionCall %void %16
               OpReturn
               OpFunctionEnd
         %14 = OpFunction %void None %7
         %17 = OpLabel
         %19 = OpFunctionCall %void %16
               OpReturn
               OpFunctionEnd
         %16 = OpFunction %void None %7
         %18 = OpLabel
         %20 = OpFunctionCall %void %21
               OpReturn
               OpFunctionEnd
         %21 = OpFunction %void None %7
         %22 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  auto c = analysis.FindFuncsCalledFromContinue();
  EXPECT_THAT(c, UnorderedElementsAre(14u, 16u, 21u));
}

TEST_F(StructCFGAnalysisTest, SingleBlockLoop) {
  const std::string text = R"(
              OpCapability Shader
              OpCapability Linkage
              OpMemoryModel Logical GLSL450
      %void = OpTypeVoid
      %bool = OpTypeBool
     %undef = OpUndef %bool
   %void_fn = OpTypeFunction %void
      %main = OpFunction %void None %void_fn
         %2 = OpLabel
              OpBranch %3
         %3 = OpLabel
              OpLoopMerge %4 %3 None
              OpBranchConditional %undef %3 %4
         %4 = OpLabel
              OpReturn
              OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  StructuredCFGAnalysis analysis(context.get());

  EXPECT_TRUE(analysis.IsInContinueConstruct(3));
}
}  // namespace
}  // namespace opt
}  // namespace spvtools
