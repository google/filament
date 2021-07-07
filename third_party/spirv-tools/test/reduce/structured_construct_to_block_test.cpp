// Copyright (c) 2021 Alastair F. Donaldson
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

#include "source/reduce/structured_construct_to_block_reduction_opportunity_finder.h"

#include "source/opt/build_module.h"
#include "source/reduce/reduction_opportunity.h"
#include "test/reduce/reduce_test_util.h"

namespace spvtools {
namespace reduce {
namespace {

TEST(StructuredConstructToBlockReductionPassTest, SimpleTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %10 = OpTypeBool
         %11 = OpConstantTrue %10
         %19 = OpConstant %6 3
         %29 = OpConstant %6 1
         %31 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpSelectionMerge %13 None
               OpBranchConditional %11 %12 %13
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpLoopMerge %16 %17 None
               OpBranch %15
         %15 = OpLabel
         %18 = OpLoad %6 %8
         %20 = OpSGreaterThan %10 %18 %19
               OpSelectionMerge %22 None
               OpBranchConditional %20 %21 %22
         %21 = OpLabel
               OpBranch %16
         %22 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpBranch %14
         %16 = OpLabel
         %24 = OpLoad %6 %8
               OpSelectionMerge %28 None
               OpSwitch %24 %27 1 %25 2 %26
         %27 = OpLabel
               OpStore %8 %19
               OpBranch %28
         %25 = OpLabel
               OpStore %8 %29
               OpBranch %28
         %26 = OpLabel
               OpStore %8 %31
               OpBranch %28
         %28 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto context = BuildModule(env, nullptr, shader, kReduceAssembleOption);
  const auto ops = StructuredConstructToBlockReductionOpportunityFinder()
                       .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(3, ops.size());

  ASSERT_TRUE(ops[0]->PreconditionHolds());
  ops[0]->TryToApply();
  CheckValid(env, context.get());

  ASSERT_TRUE(ops[1]->PreconditionHolds());
  ops[1]->TryToApply();
  CheckValid(env, context.get());

  ASSERT_TRUE(ops[2]->PreconditionHolds());
  ops[2]->TryToApply();
  CheckValid(env, context.get());

  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %10 = OpTypeBool
         %11 = OpConstantTrue %10
         %19 = OpConstant %6 3
         %29 = OpConstant %6 1
         %31 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %13
         %13 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpBranch %16
         %16 = OpLabel
         %24 = OpLoad %6 %8
               OpBranch %28
         %28 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  CheckEqual(env, expected, context.get());
}

TEST(StructuredConstructToBlockReductionPassTest, CannotBeRemovedDueToUses) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %100 "temp"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %10 = OpTypeBool
         %11 = OpConstantTrue %10
         %19 = OpConstant %6 3
         %29 = OpConstant %6 1
         %31 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpSelectionMerge %13 None
               OpBranchConditional %11 %12 %13
         %12 = OpLabel
        %100 = OpCopyObject %10 %11
               OpBranch %13
         %13 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpLoopMerge %16 %17 None
               OpBranch %15
         %15 = OpLabel
         %18 = OpLoad %6 %8
         %20 = OpSGreaterThan %10 %18 %19
               OpSelectionMerge %22 None
               OpBranchConditional %20 %21 %22
         %21 = OpLabel
               OpBranch %16
         %22 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpBranch %14
         %16 = OpLabel
        %101 = OpCopyObject %6 %18
         %24 = OpLoad %6 %8
               OpSelectionMerge %28 None
               OpSwitch %24 %27 1 %25 2 %26
         %27 = OpLabel
               OpStore %8 %19
        %102 = OpCopyObject %10 %11
               OpBranch %28
         %25 = OpLabel
               OpStore %8 %29
               OpBranch %28
         %26 = OpLabel
               OpStore %8 %31
               OpBranch %28
         %28 = OpLabel
        %103 = OpPhi %10 %102 %27 %11 %25 %11 %26
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto context = BuildModule(env, nullptr, shader, kReduceAssembleOption);
  const auto ops = StructuredConstructToBlockReductionOpportunityFinder()
                       .GetAvailableOpportunities(context.get(), 0);
  ASSERT_TRUE(ops.empty());
}

TEST(StructuredConstructToBlockReductionPassTest,
     CannotBeRemovedDueToOpPhiAtMerge) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeBool
         %11 = OpConstantTrue %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %11 %12 %13
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
        %101 = OpPhi %10 %11 %5 %11 %12
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto context = BuildModule(env, nullptr, shader, kReduceAssembleOption);
  const auto ops = StructuredConstructToBlockReductionOpportunityFinder()
                       .GetAvailableOpportunities(context.get(), 0);
  ASSERT_TRUE(ops.empty());
}

}  // namespace
}  // namespace reduce
}  // namespace spvtools
