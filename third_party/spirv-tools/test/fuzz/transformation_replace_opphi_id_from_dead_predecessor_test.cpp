// Copyright (c) 2020 Google LLC
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

#include "source/fuzz/transformation_replace_opphi_id_from_dead_predecessor.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

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
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpConstantFalse %5
          %8 = OpTypeInt 32 1
          %9 = OpConstant %8 2
         %10 = OpConstant %8 3
         %11 = OpConstant %8 4
         %12 = OpConstant %8 5
         %13 = OpConstant %8 6
          %2 = OpFunction %3 None %4
         %14 = OpLabel
               OpSelectionMerge %15 None
               OpBranchConditional %6 %16 %17
         %16 = OpLabel
         %18 = OpCopyObject %8 %9
               OpSelectionMerge %19 None
               OpBranchConditional %7 %20 %21
         %20 = OpLabel
         %22 = OpCopyObject %8 %10
         %23 = OpCopyObject %8 %12
               OpBranch %19
         %21 = OpLabel
         %24 = OpCopyObject %8 %9
               OpBranch %19
         %19 = OpLabel
         %25 = OpPhi %8 %22 %20 %24 %21
               OpBranch %15
         %17 = OpLabel
         %26 = OpCopyObject %8 %12
         %27 = OpCopyObject %8 %13
               OpBranch %28
         %28 = OpLabel
         %29 = OpPhi %8 %27 %17
               OpBranch %15
         %15 = OpLabel
         %30 = OpPhi %8 %25 %19 %26 %28
               OpReturn
               OpFunctionEnd
)";

TEST(TransformationReplaceOpPhiIdFromDeadPredecessorTest, Inapplicable) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Record the fact that blocks 20, 17, 28 are dead.
  transformation_context.GetFactManager()->AddFactBlockIsDead(20);
  transformation_context.GetFactManager()->AddFactBlockIsDead(17);
  transformation_context.GetFactManager()->AddFactBlockIsDead(28);

  // %26 is not an OpPhi instruction.
  ASSERT_FALSE(TransformationReplaceOpPhiIdFromDeadPredecessor(26, 14, 10)
                   .IsApplicable(context.get(), transformation_context));

  // %25 is not a block label.
  ASSERT_FALSE(TransformationReplaceOpPhiIdFromDeadPredecessor(30, 25, 10)
                   .IsApplicable(context.get(), transformation_context));

  // %14 is not a predecessor of %28 (which contains %29).
  ASSERT_FALSE(TransformationReplaceOpPhiIdFromDeadPredecessor(29, 14, 10)
                   .IsApplicable(context.get(), transformation_context));

  // %19 is not a dead block.
  ASSERT_FALSE(TransformationReplaceOpPhiIdFromDeadPredecessor(30, 19, 10)
                   .IsApplicable(context.get(), transformation_context));

  // %7 does not have the same type id as %25.
  ASSERT_FALSE(
      TransformationReplaceOpPhiIdFromDeadPredecessor(25, 20, 7).IsApplicable(
          context.get(), transformation_context));

  // %29 is not available at the end of %20.
  ASSERT_FALSE(TransformationReplaceOpPhiIdFromDeadPredecessor(25, 20, 29)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceOpPhiIdFromDeadPredecessorTest, Apply) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Record the fact that blocks 20, 17, 28 are dead.
  transformation_context.GetFactManager()->AddFactBlockIsDead(20);
  transformation_context.GetFactManager()->AddFactBlockIsDead(17);
  transformation_context.GetFactManager()->AddFactBlockIsDead(28);

  auto transformation1 =
      TransformationReplaceOpPhiIdFromDeadPredecessor(25, 20, 18);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);

  auto transformation2 =
      TransformationReplaceOpPhiIdFromDeadPredecessor(30, 28, 29);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);

  auto transformation3 =
      TransformationReplaceOpPhiIdFromDeadPredecessor(29, 17, 10);
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpConstantFalse %5
          %8 = OpTypeInt 32 1
          %9 = OpConstant %8 2
         %10 = OpConstant %8 3
         %11 = OpConstant %8 4
         %12 = OpConstant %8 5
         %13 = OpConstant %8 6
          %2 = OpFunction %3 None %4
         %14 = OpLabel
               OpSelectionMerge %15 None
               OpBranchConditional %6 %16 %17
         %16 = OpLabel
         %18 = OpCopyObject %8 %9
               OpSelectionMerge %19 None
               OpBranchConditional %7 %20 %21
         %20 = OpLabel
         %22 = OpCopyObject %8 %10
         %23 = OpCopyObject %8 %12
               OpBranch %19
         %21 = OpLabel
         %24 = OpCopyObject %8 %9
               OpBranch %19
         %19 = OpLabel
         %25 = OpPhi %8 %18 %20 %24 %21
               OpBranch %15
         %17 = OpLabel
         %26 = OpCopyObject %8 %12
         %27 = OpCopyObject %8 %13
               OpBranch %28
         %28 = OpLabel
         %29 = OpPhi %8 %10 %17
               OpBranch %15
         %15 = OpLabel
         %30 = OpPhi %8 %25 %19 %29 %28
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
