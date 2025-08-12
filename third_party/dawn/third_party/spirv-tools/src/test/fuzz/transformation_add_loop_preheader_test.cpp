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

#include "source/fuzz/transformation_add_loop_preheader.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddLoopPreheaderTest, SimpleTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %12 %11 None
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %7 %10 %12
         %12 = OpLabel
               OpLoopMerge %14 %13 None
               OpBranch %13
         %13 = OpLabel
               OpBranchConditional %7 %14 %12
         %15 = OpLabel
               OpLoopMerge %17 %16 None
               OpBranch %16
         %16 = OpLabel
               OpBranchConditional %7 %15 %17
         %17 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %9 is not a loop header
  ASSERT_FALSE(TransformationAddLoopPreheader(9, 15, {}).IsApplicable(
      context.get(), transformation_context));

  // The id %12 is not fresh
  ASSERT_FALSE(TransformationAddLoopPreheader(10, 12, {})
                   .IsApplicable(context.get(), transformation_context));

  // Loop header %15 is not reachable (the only predecessor is the back-edge
  // block)
  ASSERT_FALSE(TransformationAddLoopPreheader(15, 100, {})
                   .IsApplicable(context.get(), transformation_context));

  auto transformation1 = TransformationAddLoopPreheader(10, 20, {});
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);

  auto transformation2 = TransformationAddLoopPreheader(12, 21, {});
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %20
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %21 %11 None
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %7 %10 %21
         %21 = OpLabel
               OpBranch %12
         %12 = OpLabel
               OpLoopMerge %14 %13 None
               OpBranch %13
         %13 = OpLabel
               OpBranchConditional %7 %14 %12
         %15 = OpLabel
               OpLoopMerge %17 %16 None
               OpBranch %16
         %16 = OpLabel
               OpBranchConditional %7 %15 %17
         %17 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationAddLoopPreheaderTest, OpPhi) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpCopyObject %6 %7
               OpBranch %8
          %8 = OpLabel
         %31 = OpPhi %6 %20 %5 %21 %9
               OpLoopMerge %10 %9 None
               OpBranch %9
          %9 = OpLabel
         %21 = OpCopyObject %6 %7
               OpBranchConditional %7 %8 %10
         %10 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %7 %11 %12
         %11 = OpLabel
         %22 = OpCopyObject %6 %7
               OpBranch %13
         %12 = OpLabel
         %23 = OpCopyObject %6 %7
               OpBranch %13
         %13 = OpLabel
         %32 = OpPhi %6 %22 %11 %23 %12 %24 %14
         %33 = OpPhi %6 %7 %11 %7 %12 %24 %14
               OpLoopMerge %15 %14 None
               OpBranch %14
         %14 = OpLabel
         %24 = OpCopyObject %6 %7
               OpBranchConditional %7 %13 %15
         %15 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation1 = TransformationAddLoopPreheader(8, 40, {});
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);

  // Not enough ids for the OpPhi instructions are given
  ASSERT_FALSE(TransformationAddLoopPreheader(13, 41, {})
                   .IsApplicable(context.get(), transformation_context));

  // Not enough ids for the OpPhi instructions are given
  ASSERT_FALSE(TransformationAddLoopPreheader(13, 41, {42})
                   .IsApplicable(context.get(), transformation_context));

  // One of the ids is not fresh
  ASSERT_FALSE(TransformationAddLoopPreheader(13, 41, {31, 42})
                   .IsApplicable(context.get(), transformation_context));

  // One of the ids is repeated
  ASSERT_FALSE(TransformationAddLoopPreheader(13, 41, {41, 42})
                   .IsApplicable(context.get(), transformation_context));

  auto transformation2 = TransformationAddLoopPreheader(13, 41, {42, 43});
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpCopyObject %6 %7
               OpBranch %40
         %40 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %31 = OpPhi %6 %20 %40 %21 %9
               OpLoopMerge %10 %9 None
               OpBranch %9
          %9 = OpLabel
         %21 = OpCopyObject %6 %7
               OpBranchConditional %7 %8 %10
         %10 = OpLabel
               OpSelectionMerge %41 None
               OpBranchConditional %7 %11 %12
         %11 = OpLabel
         %22 = OpCopyObject %6 %7
               OpBranch %41
         %12 = OpLabel
         %23 = OpCopyObject %6 %7
               OpBranch %41
         %41 = OpLabel
         %42 = OpPhi %6 %22 %11 %23 %12
         %43 = OpPhi %6 %7 %11 %7 %12
               OpBranch %13
         %13 = OpLabel
         %32 = OpPhi %6 %42 %41 %24 %14
         %33 = OpPhi %6 %43 %41 %24 %14
               OpLoopMerge %15 %14 None
               OpBranch %14
         %14 = OpLabel
         %24 = OpCopyObject %6 %7
               OpBranchConditional %7 %13 %15
         %15 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
