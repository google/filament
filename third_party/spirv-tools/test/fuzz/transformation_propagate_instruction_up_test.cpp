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

#include "source/fuzz/transformation_propagate_instruction_up.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationPropagateInstructionUpTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3.5
         %11 = OpConstant %6 3.4000001
         %12 = OpTypeBool
         %17 = OpConstant %6 4
         %20 = OpConstant %6 45
         %27 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %26 = OpVariable %27 Function
         %13 = OpFOrdEqual %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %19

         %14 = OpLabel
         %18 = OpFMod %6 %9 %17
               OpBranch %15

         %19 = OpLabel
         %22 = OpFAdd %6 %11 %20
               OpBranch %15

         %15 = OpLabel
         %21 = OpPhi %6 %18 %14 %22 %19
         %23 = OpFMul %6 %21 %21
         %24 = OpFDiv %6 %21 %23
               OpBranch %25

         %25 = OpLabel
         %28 = OpPhi %6 %20 %15
               OpStore %26 %28
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
  // |block_id| is invalid.
  ASSERT_FALSE(TransformationPropagateInstructionUp(40, {{}}).IsApplicable(
      context.get(), transformation_context));
  ASSERT_FALSE(TransformationPropagateInstructionUp(26, {{}}).IsApplicable(
      context.get(), transformation_context));

  // |block_id| has no predecessors.
  ASSERT_FALSE(TransformationPropagateInstructionUp(5, {{}}).IsApplicable(
      context.get(), transformation_context));

  // |block_id| has no valid instructions to propagate.
  ASSERT_FALSE(TransformationPropagateInstructionUp(25, {{{15, 40}}})
                   .IsApplicable(context.get(), transformation_context));

  // Not all predecessors have fresh ids.
  ASSERT_FALSE(TransformationPropagateInstructionUp(15, {{{19, 40}, {40, 41}}})
                   .IsApplicable(context.get(), transformation_context));

  // Not all ids are fresh.
  ASSERT_FALSE(
      TransformationPropagateInstructionUp(15, {{{19, 40}, {14, 14}, {40, 42}}})
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationPropagateInstructionUp(15, {{{19, 19}, {14, 40}, {40, 42}}})
          .IsApplicable(context.get(), transformation_context));

  // Fresh ids have duplicates.
  ASSERT_FALSE(
      TransformationPropagateInstructionUp(15, {{{19, 40}, {14, 40}, {19, 41}}})
          .IsApplicable(context.get(), transformation_context));

  // Valid transformations.
  {
    TransformationPropagateInstructionUp transformation(14, {{{5, 40}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
  {
    TransformationPropagateInstructionUp transformation(19, {{{5, 41}}});
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
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3.5
         %11 = OpConstant %6 3.4000001
         %12 = OpTypeBool
         %17 = OpConstant %6 4
         %20 = OpConstant %6 45
         %27 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %26 = OpVariable %27 Function
         %13 = OpFOrdEqual %12 %9 %11
         %40 = OpFMod %6 %9 %17 ; propagated from %14
         %41 = OpFAdd %6 %11 %20 ; propagated from %19
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %19

         %14 = OpLabel
         %18 = OpPhi %6 %40 %5 ; propagated into %5
               OpBranch %15

         %19 = OpLabel
         %22 = OpPhi %6 %41 %5 ; propagated into %5
               OpBranch %15

         %15 = OpLabel
         %21 = OpPhi %6 %18 %14 %22 %19
         %23 = OpFMul %6 %21 %21
         %24 = OpFDiv %6 %21 %23
               OpBranch %25

         %25 = OpLabel
         %28 = OpPhi %6 %20 %15
               OpStore %26 %28
               OpReturn

               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));

  {
    TransformationPropagateInstructionUp transformation(15,
                                                        {{{14, 43}, {19, 44}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3.5
         %11 = OpConstant %6 3.4000001
         %12 = OpTypeBool
         %17 = OpConstant %6 4
         %20 = OpConstant %6 45
         %27 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %26 = OpVariable %27 Function
         %13 = OpFOrdEqual %12 %9 %11
         %40 = OpFMod %6 %9 %17
         %41 = OpFAdd %6 %11 %20
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %19

         %14 = OpLabel
         %18 = OpPhi %6 %40 %5
         %43 = OpFMul %6 %18 %18 ; propagated from %15
               OpBranch %15

         %19 = OpLabel
         %22 = OpPhi %6 %41 %5
         %44 = OpFMul %6 %22 %22 ; propagated from %15
               OpBranch %15

         %15 = OpLabel
         %23 = OpPhi %6 %43 %14 %44 %19 ; propagated into %14 and %19
         %21 = OpPhi %6 %18 %14 %22 %19
         %24 = OpFDiv %6 %21 %23
               OpBranch %25

         %25 = OpLabel
         %28 = OpPhi %6 %20 %15
               OpStore %26 %28
               OpReturn

               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));

  {
    TransformationPropagateInstructionUp transformation(15,
                                                        {{{14, 45}, {19, 46}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3.5
         %11 = OpConstant %6 3.4000001
         %12 = OpTypeBool
         %17 = OpConstant %6 4
         %20 = OpConstant %6 45
         %27 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %26 = OpVariable %27 Function
         %13 = OpFOrdEqual %12 %9 %11
         %40 = OpFMod %6 %9 %17
         %41 = OpFAdd %6 %11 %20
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %19

         %14 = OpLabel
         %18 = OpPhi %6 %40 %5
         %43 = OpFMul %6 %18 %18
         %45 = OpFDiv %6 %18 %43 ; propagated from %15
               OpBranch %15

         %19 = OpLabel
         %22 = OpPhi %6 %41 %5
         %44 = OpFMul %6 %22 %22
         %46 = OpFDiv %6 %22 %44 ; propagated from %15
               OpBranch %15

         %15 = OpLabel
         %24 = OpPhi %6 %45 %14 %46 %19 ; propagated into %14 and %19
         %23 = OpPhi %6 %43 %14 %44 %19
         %21 = OpPhi %6 %18 %14 %22 %19
               OpBranch %25

         %25 = OpLabel
         %28 = OpPhi %6 %20 %15
               OpStore %26 %28
               OpReturn

               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionUpTest, BlockDominatesPredecessor1) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3.5
         %11 = OpConstant %6 3.4000001
         %12 = OpTypeBool
         %17 = OpConstant %6 4
         %20 = OpConstant %6 45
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %13 = OpFOrdEqual %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %19

         %14 = OpLabel
         %18 = OpFMod %6 %9 %17
               OpBranch %15

         %19 = OpLabel
         %22 = OpFAdd %6 %11 %20
               OpBranch %15

         %15 = OpLabel ; dominates %26
         %21 = OpPhi %6 %18 %14 %22 %19 %28 %26
         %23 = OpFMul %6 %21 %21
         %24 = OpFDiv %6 %21 %23
               OpLoopMerge %27 %26 None
               OpBranch %26

         %26 = OpLabel
         %28 = OpFAdd %6 %24 %23
               OpBranch %15

         %27 = OpLabel
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
  TransformationPropagateInstructionUp transformation(
      15, {{{14, 40}, {19, 41}, {26, 42}}});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3.5
         %11 = OpConstant %6 3.4000001
         %12 = OpTypeBool
         %17 = OpConstant %6 4
         %20 = OpConstant %6 45
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %13 = OpFOrdEqual %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %19

         %14 = OpLabel
         %18 = OpFMod %6 %9 %17
         %40 = OpFMul %6 %18 %18 ; propagated from %15
               OpBranch %15

         %19 = OpLabel
         %22 = OpFAdd %6 %11 %20
         %41 = OpFMul %6 %22 %22 ; propagated from %15
               OpBranch %15

         %15 = OpLabel
         %23 = OpPhi %6 %40 %14 %41 %19 %42 %26 ; propagated into %14, %19, %26
         %21 = OpPhi %6 %18 %14 %22 %19 %28 %26
         %24 = OpFDiv %6 %21 %23
               OpLoopMerge %27 %26 None
               OpBranch %26

         %26 = OpLabel
         %28 = OpFAdd %6 %24 %23
         %42 = OpFMul %6 %28 %28 ; propagated from %15
               OpBranch %15

         %27 = OpLabel
               OpReturn

               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionUpTest, BlockDominatesPredecessor2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3.5
         %11 = OpConstant %6 3.4000001
         %12 = OpTypeBool
         %17 = OpConstant %6 4
         %20 = OpConstant %6 45
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %13 = OpFOrdEqual %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %19

         %14 = OpLabel
         %18 = OpFMod %6 %9 %17
               OpBranch %15

         %19 = OpLabel
         %22 = OpFAdd %6 %11 %20
               OpBranch %15

         %15 = OpLabel ; doesn't dominate %26
         %21 = OpPhi %6 %18 %14 %22 %19 %20 %26
         %23 = OpFMul %6 %21 %21
         %24 = OpFDiv %6 %21 %23
               OpLoopMerge %27 %26 None
               OpBranch %27

         %26 = OpLabel
               OpBranch %15

         %27 = OpLabel
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
  TransformationPropagateInstructionUp transformation(
      15, {{{14, 40}, {19, 41}, {26, 42}}});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3.5
         %11 = OpConstant %6 3.4000001
         %12 = OpTypeBool
         %17 = OpConstant %6 4
         %20 = OpConstant %6 45
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %13 = OpFOrdEqual %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %19

         %14 = OpLabel
         %18 = OpFMod %6 %9 %17
         %40 = OpFMul %6 %18 %18 ; propagated from %15
               OpBranch %15

         %19 = OpLabel
         %22 = OpFAdd %6 %11 %20
         %41 = OpFMul %6 %22 %22 ; propagated from %15
               OpBranch %15

         %15 = OpLabel
         %23 = OpPhi %6 %40 %14 %41 %19 %42 %26 ; propagated into %14, %19, %26
         %21 = OpPhi %6 %18 %14 %22 %19 %20 %26
         %24 = OpFDiv %6 %21 %23
               OpLoopMerge %27 %26 None
               OpBranch %27

         %26 = OpLabel
         %42 = OpFMul %6 %20 %20 ; propagated from %15
               OpBranch %15

         %27 = OpLabel
               OpReturn

               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionUpTest, BlockDominatesPredecessor3) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3.5
         %11 = OpConstant %6 3.4000001
         %12 = OpTypeBool
         %17 = OpConstant %6 4
         %20 = OpConstant %6 45
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %13 = OpFOrdEqual %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %19

         %14 = OpLabel
         %18 = OpFMod %6 %9 %17
               OpBranch %15

         %19 = OpLabel
         %22 = OpFAdd %6 %11 %20
               OpBranch %15

         %15 = OpLabel ; branches to itself
         %21 = OpPhi %6 %18 %14 %22 %19 %24 %15
         %23 = OpFMul %6 %21 %21
         %24 = OpFDiv %6 %21 %23
               OpLoopMerge %27 %15 None
               OpBranch %15

         %27 = OpLabel
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
  TransformationPropagateInstructionUp transformation(
      15, {{{14, 40}, {19, 41}, {15, 42}}});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3.5
         %11 = OpConstant %6 3.4000001
         %12 = OpTypeBool
         %17 = OpConstant %6 4
         %20 = OpConstant %6 45
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %13 = OpFOrdEqual %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %19

         %14 = OpLabel
         %18 = OpFMod %6 %9 %17
         %40 = OpFMul %6 %18 %18 ; propagated from %15
               OpBranch %15

         %19 = OpLabel
         %22 = OpFAdd %6 %11 %20
         %41 = OpFMul %6 %22 %22 ; propagated from %15
               OpBranch %15

         %15 = OpLabel
         %23 = OpPhi %6 %40 %14 %41 %19 %42 %15 ; propagated into %14, %19, %15
         %21 = OpPhi %6 %18 %14 %22 %19 %24 %15
         %24 = OpFDiv %6 %21 %23
         %42 = OpFMul %6 %24 %24 ; propagated from %15
               OpLoopMerge %27 %15 None
               OpBranch %15

         %27 = OpLabel
               OpReturn

               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionUpTest,
     HandlesVariablePointersCapability) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
         %11 = OpConstant %6 23
          %7 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpBranch %9

          %9 = OpLabel
         %10 = OpCopyObject %7 %8
               OpStore %10 %11
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
  // Required capabilities haven't yet been specified.
  TransformationPropagateInstructionUp transformation(9, {{{5, 40}}});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  context->AddCapability(SpvCapabilityVariablePointers);

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
         %11 = OpConstant %6 23
          %7 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          %5 = OpLabel
          %8 = OpVariable %7 Function
         %40 = OpCopyObject %7 %8 ; propagated from %9
               OpBranch %9

          %9 = OpLabel
         %10 = OpPhi %7 %40 %5 ; propagated into %5
               OpStore %10 %11
               OpReturn

               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionUpTest,
     HandlesVariablePointersStorageBufferCapability) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
         %11 = OpConstant %6 23
          %7 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpBranch %9

          %9 = OpLabel
         %10 = OpCopyObject %7 %8
               OpStore %10 %11
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
  // Required capabilities haven't yet been specified
  TransformationPropagateInstructionUp transformation(9, {{{5, 40}}});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  context->AddCapability(SpvCapabilityVariablePointersStorageBuffer);

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
               OpCapability VariablePointersStorageBuffer
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
         %11 = OpConstant %6 23
          %7 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          %5 = OpLabel
          %8 = OpVariable %7 Function
         %40 = OpCopyObject %7 %8 ; propagated from %9
               OpBranch %9

          %9 = OpLabel
         %10 = OpPhi %7 %40 %5 ; propagated into %5
               OpStore %10 %11
               OpReturn

               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionUpTest, MultipleIdenticalPredecessors) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
         %11 = OpConstant %6 23
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %4 = OpFunction %2 None %3

          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %13 %9 %9

          %9 = OpLabel
         %14 = OpPhi %6 %11 %5
         %10 = OpCopyObject %6 %14
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
  TransformationPropagateInstructionUp transformation(9, {{{5, 40}}});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
         %11 = OpConstant %6 23
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %40 = OpCopyObject %6 %11
               OpSelectionMerge %9 None
               OpBranchConditional %13 %9 %9

          %9 = OpLabel
         %10 = OpPhi %6 %40 %5
         %14 = OpPhi %6 %11 %5
               OpReturn

               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionUpTest,
     InapplicableDueToOpTypeSampledImage) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %10
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpDecorate %10 RelaxedPrecision
               OpDecorate %10 DescriptorSet 0
               OpDecorate %10 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeImage %6 2D 0 0 0 1 Unknown
          %8 = OpTypeSampledImage %7
          %9 = OpTypePointer UniformConstant %8
         %10 = OpVariable %9 UniformConstant
         %12 = OpTypeVector %6 2
         %13 = OpConstant %6 0
         %14 = OpConstantComposite %12 %13 %13
         %15 = OpTypeVector %6 4
         %30 = OpTypeBool
         %31 = OpConstantTrue %30
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpLoad %8 %10
               OpSelectionMerge %20 None
               OpBranchConditional %31 %40 %41
         %40 = OpLabel
               OpBranch %20
         %41 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %50 = OpCopyObject %8 %11
         %16 = OpImageSampleImplicitLod %15 %50 %14
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

  ASSERT_FALSE(
      TransformationPropagateInstructionUp(20, {{{40, 100}, {41, 101}}})
          .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
