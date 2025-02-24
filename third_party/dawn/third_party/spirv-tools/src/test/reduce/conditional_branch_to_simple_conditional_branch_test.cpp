// Copyright (c) 2019 Google LLC
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

#include "source/reduce/conditional_branch_to_simple_conditional_branch_opportunity_finder.h"

#include "source/opt/build_module.h"
#include "source/reduce/reduction_opportunity.h"
#include "source/reduce/reduction_pass.h"
#include "test/reduce/reduce_test_util.h"

namespace spvtools {
namespace reduce {
namespace {

const spv_target_env kEnv = SPV_ENV_UNIVERSAL_1_3;

TEST(ConditionalBranchToSimpleConditionalBranchTest, Diamond) {
  // A test with the following structure.
  //
  // selection header
  // OpBranchConditional
  //  |         |
  //  b         b
  //  |         |
  //  selection merge
  //
  // There should be two opportunities for redirecting the OpBranchConditional
  // targets: redirecting the true to false, and vice-versa. E.g. false to true:
  //
  // selection header
  // OpBranchConditional
  //  ||
  //  b         b
  //  |         |
  //  selection merge

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypePointer Function %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %8 %12 %13
         %12 = OpLabel
               OpBranch %11
         %13 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpReturn
               OpFunctionEnd

    )";

  auto context = BuildModule(kEnv, nullptr, shader, kReduceAssembleOption);

  CheckValid(kEnv, context.get());

  auto ops = ConditionalBranchToSimpleConditionalBranchOpportunityFinder()
                 .GetAvailableOpportunities(context.get(), 0);

  ASSERT_EQ(2, ops.size());

  ASSERT_TRUE(ops[0]->PreconditionHolds());
  ASSERT_TRUE(ops[1]->PreconditionHolds());
  ops[0]->TryToApply();
  // The other opportunity should now be disabled.
  ASSERT_FALSE(ops[1]->PreconditionHolds());

  CheckValid(kEnv, context.get());

  {
    std::string after = R"(
                 OpCapability Shader
            %1 = OpExtInstImport "GLSL.std.450"
                 OpMemoryModel Logical GLSL450
                 OpEntryPoint Fragment %2 "main"
                 OpExecutionMode %2 OriginUpperLeft
                 OpSource ESSL 310
                 OpName %2 "main"
            %3 = OpTypeVoid
            %4 = OpTypeFunction %3
            %5 = OpTypeInt 32 1
            %6 = OpTypePointer Function %5
            %7 = OpTypeBool
            %8 = OpConstantTrue %7
            %2 = OpFunction %3 None %4
            %9 = OpLabel
                 OpBranch %10
           %10 = OpLabel
                 OpSelectionMerge %11 None
                 OpBranchConditional %8 %12 %12
           %12 = OpLabel
                 OpBranch %11
           %13 = OpLabel
                 OpBranch %11
           %11 = OpLabel
                 OpReturn
                 OpFunctionEnd
      )";
    CheckEqual(kEnv, after, context.get());
  }

  ops = ConditionalBranchToSimpleConditionalBranchOpportunityFinder()
            .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(0, ops.size());

  // Start again, and apply the other op.
  context = BuildModule(kEnv, nullptr, shader, kReduceAssembleOption);

  CheckValid(kEnv, context.get());

  ops = ConditionalBranchToSimpleConditionalBranchOpportunityFinder()
            .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(2, ops.size());

  ASSERT_TRUE(ops[0]->PreconditionHolds());
  ASSERT_TRUE(ops[1]->PreconditionHolds());
  ops[1]->TryToApply();
  // The other opportunity should now be disabled.
  ASSERT_FALSE(ops[0]->PreconditionHolds());

  CheckValid(kEnv, context.get());

  {
    std::string after2 = R"(
                 OpCapability Shader
            %1 = OpExtInstImport "GLSL.std.450"
                 OpMemoryModel Logical GLSL450
                 OpEntryPoint Fragment %2 "main"
                 OpExecutionMode %2 OriginUpperLeft
                 OpSource ESSL 310
                 OpName %2 "main"
            %3 = OpTypeVoid
            %4 = OpTypeFunction %3
            %5 = OpTypeInt 32 1
            %6 = OpTypePointer Function %5
            %7 = OpTypeBool
            %8 = OpConstantTrue %7
            %2 = OpFunction %3 None %4
            %9 = OpLabel
                 OpBranch %10
           %10 = OpLabel
                 OpSelectionMerge %11 None
                 OpBranchConditional %8 %13 %13
           %12 = OpLabel
                 OpBranch %11
           %13 = OpLabel
                 OpBranch %11
           %11 = OpLabel
                 OpReturn
                 OpFunctionEnd
      )";
    CheckEqual(kEnv, after2, context.get());
  }
}

TEST(ConditionalBranchToSimpleConditionalBranchTest, AlreadySimplified) {
  // A test with the following structure.
  //
  // selection header
  // OpBranchConditional
  //  ||
  //  b         b
  //  |         |
  //  selection merge
  //
  // There should be no opportunities for redirecting the OpBranchConditional
  // as it is already simplified.
  //

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypePointer Function %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %8 %12 %12
         %12 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpReturn
               OpFunctionEnd

    )";

  auto context = BuildModule(kEnv, nullptr, shader, kReduceAssembleOption);

  CheckValid(kEnv, context.get());

  auto ops = ConditionalBranchToSimpleConditionalBranchOpportunityFinder()
                 .GetAvailableOpportunities(context.get(), 0);

  ASSERT_EQ(0, ops.size());
}

TEST(ConditionalBranchToSimpleConditionalBranchTest, DontRemoveBackEdge) {
  // A test with the following structure. The loop has a continue construct that
  // ends with OpBranchConditional. The OpBranchConditional can be simplified,
  // but only to point to the loop header, otherwise we have removed the
  // back-edge. Thus, there should be one opportunity instead of two.
  //
  // loop header
  //   |
  //   loop continue target and back-edge block
  //   OpBranchConditional
  //        |         |
  // loop merge       (to loop header^)

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypePointer Function %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %11 %12 None
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %8 %11 %10
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

  const auto context =
      BuildModule(kEnv, nullptr, shader, kReduceAssembleOption);

  CheckValid(kEnv, context.get());

  auto ops = ConditionalBranchToSimpleConditionalBranchOpportunityFinder()
                 .GetAvailableOpportunities(context.get(), 0);

  ASSERT_EQ(1, ops.size());

  ASSERT_TRUE(ops[0]->PreconditionHolds());
  ops[0]->TryToApply();
  CheckValid(kEnv, context.get());

  std::string after = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypePointer Function %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %11 %12 None
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %8 %10 %10
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
    )";
  CheckEqual(kEnv, after, context.get());

  ops = ConditionalBranchToSimpleConditionalBranchOpportunityFinder()
            .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(0, ops.size());
}

TEST(ConditionalBranchToSimpleConditionalBranchTest,
     DontRemoveBackEdgeCombinedHeaderContinue) {
  // A test with the following structure.
  //
  // loop header and continue target and back-edge block
  //   OpBranchConditional
  //        |         |
  // loop merge       (to loop header^)
  //
  // The OpBranchConditional-to-header edge must not be removed, so there should
  // only be one opportunity. It should change both targets to be to the loop
  // header.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypePointer Function %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %11 %10 None
               OpBranchConditional %8 %11 %10
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

  const auto context =
      BuildModule(kEnv, nullptr, shader, kReduceAssembleOption);

  CheckValid(kEnv, context.get());

  auto ops = ConditionalBranchToSimpleConditionalBranchOpportunityFinder()
                 .GetAvailableOpportunities(context.get(), 0);

  ASSERT_EQ(1, ops.size());

  ASSERT_TRUE(ops[0]->PreconditionHolds());
  ops[0]->TryToApply();
  CheckValid(kEnv, context.get());

  std::string after = R"(
                              OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypePointer Function %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %11 %10 None
               OpBranchConditional %8 %10 %10
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
    )";
  CheckEqual(kEnv, after, context.get());

  ops = ConditionalBranchToSimpleConditionalBranchOpportunityFinder()
            .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(0, ops.size());
}

TEST(ConditionalBranchToSimpleConditionalBranchTest, BackEdgeUnreachable) {
  // A test with the following structure. I.e. a loop with an unreachable
  // continue construct that ends with OpBranchConditional.
  //
  // loop header
  //   |
  //   | loop continue target (unreachable)
  //   |      |
  //   | back-edge block (unreachable)
  //   | OpBranchConditional
  //   |     |         |
  // loop merge       (to loop header^)
  //
  // The branch to the loop header must not be removed, even though the continue
  // construct is unreachable. So there should only be one opportunity to make
  // the true and false targets of the OpBranchConditional to point to the loop
  // header.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypePointer Function %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %11 %12 None
               OpBranch %11
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpBranchConditional %8 %11 %10
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

  const auto context =
      BuildModule(kEnv, nullptr, shader, kReduceAssembleOption);

  CheckValid(kEnv, context.get());

  auto ops = ConditionalBranchToSimpleConditionalBranchOpportunityFinder()
                 .GetAvailableOpportunities(context.get(), 0);

  ASSERT_EQ(1, ops.size());

  ASSERT_TRUE(ops[0]->PreconditionHolds());
  ops[0]->TryToApply();
  CheckValid(kEnv, context.get());

  std::string after = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypePointer Function %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %11 %12 None
               OpBranch %11
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpBranchConditional %8 %10 %10
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
    )";
  CheckEqual(kEnv, after, context.get());

  ops = ConditionalBranchToSimpleConditionalBranchOpportunityFinder()
            .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(0, ops.size());
}

}  // namespace
}  // namespace reduce
}  // namespace spvtools
