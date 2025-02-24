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

#include "source/fuzz/transformation_replace_opselect_with_conditional_branch.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationReplaceOpSelectWithConditionalBranchTest, Inapplicable) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpConstant %5 1
          %7 = OpConstant %5 2
          %8 = OpTypeVector %5 4
          %9 = OpConstantNull %8
         %10 = OpConstantComposite %8 %6 %6 %7 %7
         %11 = OpTypeBool
         %12 = OpTypeVector %11 4
         %13 = OpConstantTrue %11
         %14 = OpConstantFalse %11
         %15 = OpConstantComposite %12 %13 %14 %14 %13
          %2 = OpFunction %3 None %4
         %16 = OpLabel
         %17 = OpCopyObject %5 %6
         %18 = OpCopyObject %5 %7
               OpBranch %19
         %19 = OpLabel
         %20 = OpCopyObject %5 %17
         %21 = OpSelect %5 %13 %17 %18
               OpBranch %22
         %22 = OpLabel
         %23 = OpSelect %8 %15 %9 %10
               OpBranch %24
         %24 = OpLabel
               OpSelectionMerge %25 None
               OpBranchConditional %13 %26 %27
         %26 = OpLabel
         %28 = OpSelect %5 %13 %17 %18
               OpBranch %27
         %27 = OpLabel
         %29 = OpSelect %5 %13 %17 %18
               OpBranch %25
         %25 = OpLabel
         %30 = OpSelect %5 %13 %17 %18
               OpBranch %31
         %31 = OpLabel
               OpLoopMerge %32 %33 None
               OpBranch %33
         %33 = OpLabel
         %34 = OpSelect %5 %13 %17 %18
               OpBranchConditional %13 %31 %32
         %32 = OpLabel
         %35 = OpSelect %5 %13 %17 %18
               OpBranch %36
         %36 = OpLabel
         %37 = OpSelect %5 %13 %17 %18
               OpReturn
               OpFunctionEnd
)";
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // %20 is not an OpSelect instruction.
  ASSERT_FALSE(TransformationReplaceOpSelectWithConditionalBranch(20, 100, 101)
                   .IsApplicable(context.get(), transformation_context));

  // %21 is not the first instruction in its block.
  ASSERT_FALSE(TransformationReplaceOpSelectWithConditionalBranch(21, 100, 101)
                   .IsApplicable(context.get(), transformation_context));

  // The condition for %23 is not a scalar, but a vector of booleans.
  ASSERT_FALSE(TransformationReplaceOpSelectWithConditionalBranch(23, 100, 101)
                   .IsApplicable(context.get(), transformation_context));

  // The predecessor (%24) of the block containing %28 is the header of a
  // selection construct and does not branch unconditionally.
  ASSERT_FALSE(TransformationReplaceOpSelectWithConditionalBranch(24, 100, 101)
                   .IsApplicable(context.get(), transformation_context));

  // The block containing %29 has two predecessors (%24 and %26).
  ASSERT_FALSE(TransformationReplaceOpSelectWithConditionalBranch(29, 100, 101)
                   .IsApplicable(context.get(), transformation_context));

  // The block containing %30 is the merge block for a selection construct.
  ASSERT_FALSE(TransformationReplaceOpSelectWithConditionalBranch(30, 100, 101)
                   .IsApplicable(context.get(), transformation_context));

  // The predecessor (%31) of the block containing %34 is a loop header.
  ASSERT_FALSE(TransformationReplaceOpSelectWithConditionalBranch(31, 100, 101)
                   .IsApplicable(context.get(), transformation_context));

  // The block containing %35 is the merge block for a loop construct.
  ASSERT_FALSE(TransformationReplaceOpSelectWithConditionalBranch(35, 100, 101)
                   .IsApplicable(context.get(), transformation_context));

#ifndef NDEBUG
  // |true_block_id| and |false_block_id| are both 0.
  ASSERT_DEATH(
      TransformationReplaceOpSelectWithConditionalBranch(37, 0, 0).IsApplicable(
          context.get(), transformation_context),
      "At least one of the ids must be non-zero.");
#endif

  // The fresh ids are not distinct.
  ASSERT_FALSE(TransformationReplaceOpSelectWithConditionalBranch(37, 100, 100)
                   .IsApplicable(context.get(), transformation_context));

  // One of the ids is not fresh.
  ASSERT_FALSE(TransformationReplaceOpSelectWithConditionalBranch(37, 100, 10)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceOpSelectWithConditionalBranchTest, Simple) {
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
          %6 = OpConstant %5 1
          %7 = OpConstant %5 2
          %8 = OpTypeVector %5 4
          %9 = OpConstantNull %8
         %10 = OpConstantComposite %8 %6 %6 %7 %7
         %11 = OpTypeBool
         %12 = OpTypeVector %11 4
         %13 = OpConstantTrue %11
         %14 = OpConstantFalse %11
         %15 = OpConstantComposite %12 %13 %14 %14 %13
          %2 = OpFunction %3 None %4
         %16 = OpLabel
         %17 = OpCopyObject %5 %6
         %18 = OpCopyObject %5 %7
               OpBranch %19
         %19 = OpLabel
         %20 = OpSelect %5 %13 %17 %18
               OpSelectionMerge %21 None
               OpBranchConditional %13 %22 %21
         %22 = OpLabel
               OpBranch %23
         %23 = OpLabel
         %24 = OpSelect %8 %13 %9 %10
               OpBranch %21
         %21 = OpLabel
               OpBranch %25
         %25 = OpLabel
         %26 = OpSelect %5 %13 %17 %18
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto transformation =
      TransformationReplaceOpSelectWithConditionalBranch(20, 100, 101);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  auto transformation2 =
      TransformationReplaceOpSelectWithConditionalBranch(24, 0, 102);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);

  auto transformation3 =
      TransformationReplaceOpSelectWithConditionalBranch(26, 103, 0);
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
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
          %6 = OpConstant %5 1
          %7 = OpConstant %5 2
          %8 = OpTypeVector %5 4
          %9 = OpConstantNull %8
         %10 = OpConstantComposite %8 %6 %6 %7 %7
         %11 = OpTypeBool
         %12 = OpTypeVector %11 4
         %13 = OpConstantTrue %11
         %14 = OpConstantFalse %11
         %15 = OpConstantComposite %12 %13 %14 %14 %13
          %2 = OpFunction %3 None %4
         %16 = OpLabel
         %17 = OpCopyObject %5 %6
         %18 = OpCopyObject %5 %7
               OpSelectionMerge %19 None
               OpBranchConditional %13 %100 %101
        %100 = OpLabel
               OpBranch %19
        %101 = OpLabel
               OpBranch %19
         %19 = OpLabel
         %20 = OpPhi %5 %17 %100 %18 %101
               OpSelectionMerge %21 None
               OpBranchConditional %13 %22 %21
         %22 = OpLabel
               OpSelectionMerge %23 None
               OpBranchConditional %13 %23 %102
        %102 = OpLabel
               OpBranch %23
         %23 = OpLabel
         %24 = OpPhi %8 %9 %22 %10 %102
               OpBranch %21
         %21 = OpLabel
               OpSelectionMerge %25 None
               OpBranchConditional %13 %103 %25
        %103 = OpLabel
               OpBranch %25
         %25 = OpLabel
         %26 = OpPhi %5 %17 %103 %18 %21
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
