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

#include "source/fuzz/transformation_add_dead_block.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddDeadBlockTest, BasicTest) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %6 "main"
               OpExecutionMode %6 OriginUpperLeft

; Types
          %2 = OpTypeBool
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3

; Constants
          %5 = OpConstantTrue %2

; main function
          %6 = OpFunction %3 None %4
          %7 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %5 %8 %9
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpBranch %13
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Id 4 is already in use
  auto transformation = TransformationAddDeadBlock(4, 11, true);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Id 5 is not a block
  transformation = TransformationAddDeadBlock(14, 5, true);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests existing block not dominating its successor block.
  transformation = TransformationAddDeadBlock(14, 8, true);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  transformation = TransformationAddDeadBlock(14, 9, true);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests existing block being an unreachable block.
  transformation = TransformationAddDeadBlock(14, 12, true);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests applicable case.
  transformation = TransformationAddDeadBlock(14, 11, true);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(14));

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %6 "main"
               OpExecutionMode %6 OriginUpperLeft

; Types
          %2 = OpTypeBool
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3

; Constants
          %5 = OpConstantTrue %2

; main function
          %6 = OpFunction %3 None %4
          %7 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %5 %8 %9
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %5 %13 %14
         %14 = OpLabel
               OpBranch %13
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

TEST(TransformationAddDeadBlockTest, ApplicableWithFalseCondition) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %6 "main"
               OpExecutionMode %6 OriginUpperLeft

; Types
          %2 = OpTypeBool
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3

; Constants
          %5 = OpConstantFalse %2

; main function
          %6 = OpFunction %3 None %4
          %7 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %5 %8 %9
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpBranch %13
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto transformation = TransformationAddDeadBlock(14, 11, false);

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %6 "main"
               OpExecutionMode %6 OriginUpperLeft

; Types
          %2 = OpTypeBool
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3

; Constants
          %5 = OpConstantFalse %2

; main function
          %6 = OpFunction %3 None %4
          %7 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %5 %8 %9
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %5 %14 %13
         %14 = OpLabel
               OpBranch %13
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

TEST(TransformationAddDeadBlockTest, TargetBlockMustNotBeSelectionMerge) {
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
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_FALSE(TransformationAddDeadBlock(100, 9, true)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddDeadBlockTest, TargetBlockMustNotBeLoopMergeOrContinue) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %6 "main"
               OpExecutionMode %6 OriginUpperLeft

; Types
          %2 = OpTypeBool
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3

; Constants
          %5 = OpConstantTrue %2

; main function
          %6 = OpFunction %3 None %4
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpLoopMerge %12 %11 None
               OpBranch %13
         %13 = OpLabel
               OpSelectionMerge %14 None
               OpBranchConditional %5 %9 %10
          %9 = OpLabel
               OpBranch %11
         %10 = OpLabel
               OpBranch %12
         %14 = OpLabel
               OpUnreachable
         %11 = OpLabel
               OpBranch %8
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Bad because 9's successor is the loop continue target.
  ASSERT_FALSE(TransformationAddDeadBlock(100, 9, true)
                   .IsApplicable(context.get(), transformation_context));
  // Bad because 10's successor is the loop merge.
  ASSERT_FALSE(TransformationAddDeadBlock(100, 10, true)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddDeadBlockTest, SourceBlockMustNotBeLoopHead) {
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
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpLoopMerge %11 %12 None
               OpBranch %9
          %9 = OpLabel
               OpBranchConditional %7 %11 %12
         %12 = OpLabel
               OpBranch %8
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Bad because 8 is a loop head.
  ASSERT_FALSE(TransformationAddDeadBlock(100, 8, true)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddDeadBlockTest, OpPhiInTarget) {
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
          %7 = OpConstantTrue %6
          %9 = OpTypeInt 32 0
         %10 = OpConstant %9 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %12 = OpPhi %6 %7 %5
         %13 = OpPhi %9 %10 %5
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationAddDeadBlock transformation(100, 5, true);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(100));

  std::string after_transformation = R"(
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
          %7 = OpConstantTrue %6
          %9 = OpTypeInt 32 0
         %10 = OpConstant %9 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %7 %8 %100
        %100 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %12 = OpPhi %6 %7 %5 %7 %100
         %13 = OpPhi %9 %10 %5 %10 %100
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadBlockTest, BackEdge) {
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
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpLoopMerge %10 %9 None
               OpBranchConditional %7 %9 %10
          %9 = OpLabel
               OpBranch %8
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // 9 is a back edge block, so it would not be OK to add a dead block here,
  // as then both 9 and the dead block would branch to the loop header, 8.
  ASSERT_FALSE(TransformationAddDeadBlock(100, 9, true)
                   .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
