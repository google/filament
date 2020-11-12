// Copyright (c) 2020 Vasyl Teliman
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

#include "source/fuzz/transformation_wrap_region_in_selection.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationWrapRegionInSelectionTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %7 = OpTypeBool
          %8 = OpConstantTrue %7

          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6

          %6 = OpLabel
               OpSelectionMerge %12 None
               OpBranchConditional %8 %11 %12
         %11 = OpLabel
               OpReturn

         %12 = OpLabel
               OpSelectionMerge %15 None
               OpBranchConditional %8 %13 %14
         %13 = OpLabel
               OpBranch %15
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
               OpBranch %16

         %16 = OpLabel
               OpReturn
               OpFunctionEnd

          %9 = OpFunction %2 None %3
         %10 = OpLabel
               OpBranch %20

         %20 = OpLabel
               OpLoopMerge %23 %22 None
               OpBranch %21
         %21 = OpLabel
               OpBranchConditional %8 %24 %23
         %24 = OpLabel
               OpBranch %22

         ; continue target
         %22 = OpLabel
               OpLoopMerge %25 %28 None
               OpBranchConditional %8 %27 %25
         %27 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpBranch %22
         %25 = OpLabel
               OpBranch %20

         ; merge block
         %23 = OpLabel
               OpBranch %26

         %26 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  // Boolean constant does not exist.
  ASSERT_FALSE(TransformationWrapRegionInSelection(5, 6, false)
                   .IsApplicable(context.get(), transformation_context));

  // Irrelevant constant does not exist.
  ASSERT_FALSE(TransformationWrapRegionInSelection(5, 6, true)
                   .IsApplicable(context.get(), transformation_context));

  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(8);

  // Block ids are invalid.
  ASSERT_FALSE(TransformationWrapRegionInSelection(100, 6, true)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationWrapRegionInSelection(5, 100, true)
                   .IsApplicable(context.get(), transformation_context));

  // Blocks are from different functions.
  ASSERT_FALSE(TransformationWrapRegionInSelection(5, 10, true)
                   .IsApplicable(context.get(), transformation_context));

  // Header block candidate does not dominate merge block candidate.
  ASSERT_FALSE(TransformationWrapRegionInSelection(13, 16, true)
                   .IsApplicable(context.get(), transformation_context));

  // Header block candidate does not *strictly* dominate merge block candidate.
  ASSERT_FALSE(TransformationWrapRegionInSelection(5, 5, true)
                   .IsApplicable(context.get(), transformation_context));

  // Merge block candidate does not postdominate header block candidate.
  ASSERT_FALSE(TransformationWrapRegionInSelection(5, 16, true)
                   .IsApplicable(context.get(), transformation_context));

  // Header block candidate is already a header block of some other construct.
  ASSERT_FALSE(TransformationWrapRegionInSelection(12, 16, true)
                   .IsApplicable(context.get(), transformation_context));

  // Header block's terminator is not an OpBranch.
  ASSERT_FALSE(TransformationWrapRegionInSelection(21, 24, true)
                   .IsApplicable(context.get(), transformation_context));

  // Merge block candidate is already a merge block of some other construct.
  ASSERT_FALSE(TransformationWrapRegionInSelection(5, 15, true)
                   .IsApplicable(context.get(), transformation_context));

  // Header block candidate and merge block candidate are in different
  // constructs.
  ASSERT_FALSE(TransformationWrapRegionInSelection(10, 21, true)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationWrapRegionInSelection(24, 25, true)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationWrapRegionInSelection(24, 22, true)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationWrapRegionInSelection(24, 27, true)
                   .IsApplicable(context.get(), transformation_context));

  {
    // Header block candidate can be a merge block of some existing construct.
    TransformationWrapRegionInSelection transformation(15, 16, true);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
  {
    // Merge block candidate can be a header block of some existing construct.
    TransformationWrapRegionInSelection transformation(5, 6, true);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
  {
    // Wrap a loop construct.
    TransformationWrapRegionInSelection transformation(10, 26, true);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %7 = OpTypeBool
          %8 = OpConstantTrue %7

          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %6 None
               OpBranchConditional %8 %6 %6

          %6 = OpLabel
               OpSelectionMerge %12 None
               OpBranchConditional %8 %11 %12
         %11 = OpLabel
               OpReturn

         %12 = OpLabel
               OpSelectionMerge %15 None
               OpBranchConditional %8 %13 %14
         %13 = OpLabel
               OpBranch %15
         %14 = OpLabel
               OpBranch %15

         %15 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %8 %16 %16
         %16 = OpLabel
               OpReturn
               OpFunctionEnd

          %9 = OpFunction %2 None %3
         %10 = OpLabel
               OpSelectionMerge %26 None
               OpBranchConditional %8 %20 %20

         %20 = OpLabel
               OpLoopMerge %23 %22 None
               OpBranch %21
         %21 = OpLabel
               OpBranchConditional %8 %24 %23
         %24 = OpLabel
               OpBranch %22

         ; continue target
         %22 = OpLabel
               OpLoopMerge %25 %28 None
               OpBranchConditional %8 %27 %25
         %27 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpBranch %22
         %25 = OpLabel
               OpBranch %20

         ; merge block
         %23 = OpLabel
               OpBranch %26

         %26 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
