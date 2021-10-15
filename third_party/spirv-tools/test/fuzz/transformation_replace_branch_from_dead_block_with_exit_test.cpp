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

#include "source/fuzz/transformation_replace_branch_from_dead_block_with_exit.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationReplaceBranchFromDeadBlockWithExitTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 1
         %17 = OpConstant %12 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %14 = OpVariable %13 Function
               OpSelectionMerge %9 None
               OpBranchConditional %7 %8 %21
          %8 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %7 %10 %16
         %10 = OpLabel
               OpStore %14 %15
               OpBranch %20
         %20 = OpLabel
               OpBranch %11
         %16 = OpLabel
               OpStore %14 %17
               OpBranch %11
         %11 = OpLabel
               OpBranch %9
         %21 = OpLabel
               OpBranch %9
          %9 = OpLabel
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

  transformation_context.GetFactManager()->AddFactBlockIsDead(8);
  transformation_context.GetFactManager()->AddFactBlockIsDead(10);
  transformation_context.GetFactManager()->AddFactBlockIsDead(11);
  transformation_context.GetFactManager()->AddFactBlockIsDead(16);
  transformation_context.GetFactManager()->AddFactBlockIsDead(20);

  // Bad: 4 is not a block
  ASSERT_FALSE(TransformationReplaceBranchFromDeadBlockWithExit(4, SpvOpKill, 0)
                   .IsApplicable(context.get(), transformation_context));
  // Bad: 200 does not exist
  ASSERT_FALSE(
      TransformationReplaceBranchFromDeadBlockWithExit(200, SpvOpKill, 0)
          .IsApplicable(context.get(), transformation_context));
  // Bad: 21 is not a dead block
  ASSERT_FALSE(
      TransformationReplaceBranchFromDeadBlockWithExit(21, SpvOpKill, 0)
          .IsApplicable(context.get(), transformation_context));
  // Bad: terminator of 8 is not OpBranch
  ASSERT_FALSE(TransformationReplaceBranchFromDeadBlockWithExit(8, SpvOpKill, 0)
                   .IsApplicable(context.get(), transformation_context));
  // Bad: 10's successor only has 10 as a predecessor
  ASSERT_FALSE(
      TransformationReplaceBranchFromDeadBlockWithExit(10, SpvOpKill, 0)
          .IsApplicable(context.get(), transformation_context));

#ifndef NDEBUG
  ASSERT_DEATH(
      TransformationReplaceBranchFromDeadBlockWithExit(20, SpvOpSwitch, 0)
          .IsApplicable(context.get(), transformation_context),
      "Invalid early exit opcode.");
#endif

  auto transformation1 =
      TransformationReplaceBranchFromDeadBlockWithExit(20, SpvOpKill, 0);
  auto transformation2 =
      TransformationReplaceBranchFromDeadBlockWithExit(16, SpvOpKill, 0);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));

  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  // Applying transformation 1 should disable transformation 2
  ASSERT_FALSE(
      transformation2.IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 1
         %17 = OpConstant %12 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %14 = OpVariable %13 Function
               OpSelectionMerge %9 None
               OpBranchConditional %7 %8 %21
          %8 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %7 %10 %16
         %10 = OpLabel
               OpStore %14 %15
               OpBranch %20
         %20 = OpLabel
               OpKill
         %16 = OpLabel
               OpStore %14 %17
               OpBranch %11
         %11 = OpLabel
               OpBranch %9
         %21 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceBranchFromDeadBlockWithExitTest,
     VertexShaderWithLoopInContinueConstruct) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %4 "main"
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFunction %6
         %12 = OpTypePointer Function %6
         %14 = OpConstant %6 0
         %22 = OpConstant %6 10
         %23 = OpTypeBool
         %26 = OpConstant %6 1
         %40 = OpConstant %6 100
         %48 = OpConstantFalse %23
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
         %51 = OpFunctionCall %6 %10
               OpBranch %50
         %50 = OpLabel
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %7
         %11 = OpLabel
         %13 = OpVariable %12 Function
         %15 = OpVariable %12 Function
         %33 = OpVariable %12 Function
               OpStore %33 %14
               OpBranch %34
         %34 = OpLabel
               OpLoopMerge %36 %37 None
               OpBranch %38
         %38 = OpLabel
         %39 = OpLoad %6 %33
         %41 = OpSLessThan %23 %39 %40
               OpBranchConditional %41 %35 %36
         %35 = OpLabel
               OpSelectionMerge %202 None
               OpBranchConditional %48 %200 %201
        %200 = OpLabel
               OpBranch %202
        %201 = OpLabel
        %400 = OpCopyObject %6 %14
               OpBranch %202
        %202 = OpLabel
               OpBranch %37
         %37 = OpLabel
               OpStore %13 %14
               OpStore %15 %14
               OpBranch %16
         %16 = OpLabel
               OpLoopMerge %18 %19 None
               OpBranch %20
         %20 = OpLabel
               OpSelectionMerge %102 None
               OpBranchConditional %48 %100 %101
        %100 = OpLabel
               OpBranch %102
        %101 = OpLabel
               OpBranch %102
        %102 = OpLabel
         %21 = OpLoad %6 %15
         %24 = OpSLessThan %23 %21 %22
               OpBranchConditional %24 %17 %18
         %17 = OpLabel
               OpSelectionMerge %302 None
               OpBranchConditional %48 %300 %301
        %300 = OpLabel
               OpBranch %302
        %301 = OpLabel
               OpBranch %302
        %302 = OpLabel
         %25 = OpLoad %6 %13
         %27 = OpIAdd %6 %25 %26
               OpStore %13 %27
               OpBranch %19
         %19 = OpLabel
         %28 = OpLoad %6 %15
         %29 = OpIAdd %6 %28 %26
               OpStore %15 %29
               OpBranch %16
         %18 = OpLabel
         %30 = OpLoad %6 %13
         %42 = OpCopyObject %6 %30
         %43 = OpLoad %6 %33
         %44 = OpIAdd %6 %43 %42
               OpStore %33 %44
               OpBranch %34
         %36 = OpLabel
         %45 = OpLoad %6 %33
               OpReturnValue %45
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

  for (auto block : {16, 17,  18,  19,  20,  34,  35,  36,  37,  38,
                     49, 100, 101, 102, 200, 201, 202, 300, 301, 302}) {
    transformation_context.GetFactManager()->AddFactBlockIsDead(block);
  }

  // Bad: OpKill not allowed in vertex shader
  ASSERT_FALSE(
      TransformationReplaceBranchFromDeadBlockWithExit(201, SpvOpKill, 0)
          .IsApplicable(context.get(), transformation_context));

  // Bad: OpReturn is not allowed in function that expects a returned value.
  ASSERT_FALSE(
      TransformationReplaceBranchFromDeadBlockWithExit(200, SpvOpReturn, 0)
          .IsApplicable(context.get(), transformation_context));

  // Bad: Return value id does not exist
  ASSERT_FALSE(TransformationReplaceBranchFromDeadBlockWithExit(
                   201, SpvOpReturnValue, 1000)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: Return value id does not have a type
  ASSERT_FALSE(
      TransformationReplaceBranchFromDeadBlockWithExit(200, SpvOpReturnValue, 6)
          .IsApplicable(context.get(), transformation_context));

  // Bad: Return value id does not have the right type
  ASSERT_FALSE(TransformationReplaceBranchFromDeadBlockWithExit(
                   201, SpvOpReturnValue, 48)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: Return value id is not available
  ASSERT_FALSE(TransformationReplaceBranchFromDeadBlockWithExit(
                   200, SpvOpReturnValue, 400)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: Early exit now allowed in continue construct
  ASSERT_FALSE(
      TransformationReplaceBranchFromDeadBlockWithExit(101, SpvOpUnreachable, 0)
          .IsApplicable(context.get(), transformation_context));

  // Bad: Early exit now allowed in continue construct (again)
  ASSERT_FALSE(TransformationReplaceBranchFromDeadBlockWithExit(
                   300, SpvOpReturnValue, 14)
                   .IsApplicable(context.get(), transformation_context));

  auto transformation1 = TransformationReplaceBranchFromDeadBlockWithExit(
      200, SpvOpUnreachable, 0);
  auto transformation2 = TransformationReplaceBranchFromDeadBlockWithExit(
      201, SpvOpReturnValue, 400);

  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));

  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_FALSE(
      transformation1.IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  opt::Instruction* return_value_inst =
      context->get_instr_block(201)->terminator();
  ASSERT_EQ(SpvOpReturnValue, return_value_inst->opcode());
  ASSERT_EQ(SPV_OPERAND_TYPE_ID, return_value_inst->GetInOperand(0).type);
  ASSERT_EQ(400, return_value_inst->GetSingleWordInOperand(0));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %4 "main"
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFunction %6
         %12 = OpTypePointer Function %6
         %14 = OpConstant %6 0
         %22 = OpConstant %6 10
         %23 = OpTypeBool
         %26 = OpConstant %6 1
         %40 = OpConstant %6 100
         %48 = OpConstantFalse %23
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
         %51 = OpFunctionCall %6 %10
               OpBranch %50
         %50 = OpLabel
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %7
         %11 = OpLabel
         %13 = OpVariable %12 Function
         %15 = OpVariable %12 Function
         %33 = OpVariable %12 Function
               OpStore %33 %14
               OpBranch %34
         %34 = OpLabel
               OpLoopMerge %36 %37 None
               OpBranch %38
         %38 = OpLabel
         %39 = OpLoad %6 %33
         %41 = OpSLessThan %23 %39 %40
               OpBranchConditional %41 %35 %36
         %35 = OpLabel
               OpSelectionMerge %202 None
               OpBranchConditional %48 %200 %201
        %200 = OpLabel
               OpBranch %202
        %201 = OpLabel
        %400 = OpCopyObject %6 %14
               OpReturnValue %400
        %202 = OpLabel
               OpBranch %37
         %37 = OpLabel
               OpStore %13 %14
               OpStore %15 %14
               OpBranch %16
         %16 = OpLabel
               OpLoopMerge %18 %19 None
               OpBranch %20
         %20 = OpLabel
               OpSelectionMerge %102 None
               OpBranchConditional %48 %100 %101
        %100 = OpLabel
               OpBranch %102
        %101 = OpLabel
               OpBranch %102
        %102 = OpLabel
         %21 = OpLoad %6 %15
         %24 = OpSLessThan %23 %21 %22
               OpBranchConditional %24 %17 %18
         %17 = OpLabel
               OpSelectionMerge %302 None
               OpBranchConditional %48 %300 %301
        %300 = OpLabel
               OpBranch %302
        %301 = OpLabel
               OpBranch %302
        %302 = OpLabel
         %25 = OpLoad %6 %13
         %27 = OpIAdd %6 %25 %26
               OpStore %13 %27
               OpBranch %19
         %19 = OpLabel
         %28 = OpLoad %6 %15
         %29 = OpIAdd %6 %28 %26
               OpStore %15 %29
               OpBranch %16
         %18 = OpLabel
         %30 = OpLoad %6 %13
         %42 = OpCopyObject %6 %30
         %43 = OpLoad %6 %33
         %44 = OpIAdd %6 %43 %42
               OpStore %33 %44
               OpBranch %34
         %36 = OpLabel
         %45 = OpLoad %6 %33
               OpReturnValue %45
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceBranchFromDeadBlockWithExitTest, OpPhi) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 1
         %17 = OpConstant %12 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %14 = OpVariable %13 Function
               OpSelectionMerge %9 None
               OpBranchConditional %7 %8 %21
          %8 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %7 %10 %16
         %10 = OpLabel
               OpStore %14 %15
               OpBranch %20
         %20 = OpLabel
         %48 = OpCopyObject %12 %15
               OpBranch %11
         %16 = OpLabel
               OpStore %14 %17
         %49 = OpCopyObject %12 %17
               OpBranch %11
         %11 = OpLabel
         %50 = OpPhi %12 %48 %20 %49 %16
               OpBranch %9
         %21 = OpLabel
               OpBranch %9
          %9 = OpLabel
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

  transformation_context.GetFactManager()->AddFactBlockIsDead(8);
  transformation_context.GetFactManager()->AddFactBlockIsDead(10);
  transformation_context.GetFactManager()->AddFactBlockIsDead(11);
  transformation_context.GetFactManager()->AddFactBlockIsDead(16);
  transformation_context.GetFactManager()->AddFactBlockIsDead(20);

  auto transformation1 =
      TransformationReplaceBranchFromDeadBlockWithExit(20, SpvOpKill, 0);
  auto transformation2 =
      TransformationReplaceBranchFromDeadBlockWithExit(16, SpvOpKill, 0);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));

  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  // Applying transformation 1 should disable transformation 2
  ASSERT_FALSE(
      transformation2.IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 1
         %17 = OpConstant %12 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %14 = OpVariable %13 Function
               OpSelectionMerge %9 None
               OpBranchConditional %7 %8 %21
          %8 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %7 %10 %16
         %10 = OpLabel
               OpStore %14 %15
               OpBranch %20
         %20 = OpLabel
         %48 = OpCopyObject %12 %15
               OpKill
         %16 = OpLabel
               OpStore %14 %17
         %49 = OpCopyObject %12 %17
               OpBranch %11
         %11 = OpLabel
         %50 = OpPhi %12 %49 %16
               OpBranch %9
         %21 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceBranchFromDeadBlockWithExitTest,
     DominatorAfterDeadBlockSuccessor) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %7 %9 %10
          %9 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %12 = OpCopyObject %6 %7
               OpBranch %8
         %10 = OpLabel
               OpBranch %13
          %8 = OpLabel
               OpReturn
         %13 = OpLabel
               OpBranch %8
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

  transformation_context.GetFactManager()->AddFactBlockIsDead(9);
  transformation_context.GetFactManager()->AddFactBlockIsDead(11);

  ASSERT_FALSE(
      TransformationReplaceBranchFromDeadBlockWithExit(11, SpvOpUnreachable, 0)
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceBranchFromDeadBlockWithExitTest,
     UnreachableSuccessor) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %7 %9 %10
          %9 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %12 = OpCopyObject %6 %7
               OpBranch %8
         %10 = OpLabel
               OpReturn
          %8 = OpLabel
               OpReturn
         %13 = OpLabel
               OpBranch %8
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

  transformation_context.GetFactManager()->AddFactBlockIsDead(9);
  transformation_context.GetFactManager()->AddFactBlockIsDead(11);

  TransformationReplaceBranchFromDeadBlockWithExit transformation(
      11, SpvOpUnreachable, 0);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  transformation.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %7 %9 %10
          %9 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %12 = OpCopyObject %6 %7
               OpUnreachable
         %10 = OpLabel
               OpReturn
          %8 = OpLabel
               OpReturn
         %13 = OpLabel
               OpBranch %8
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceBranchFromDeadBlockWithExitTest,
     DeadBlockAfterItsSuccessor) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %7 %9 %10
          %9 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %12 = OpCopyObject %6 %7
               OpBranch %8
         %10 = OpLabel
               OpBranch %13
          %8 = OpLabel
               OpReturn
         %13 = OpLabel
               OpBranch %8
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

  transformation_context.GetFactManager()->AddFactBlockIsDead(10);
  transformation_context.GetFactManager()->AddFactBlockIsDead(13);

  TransformationReplaceBranchFromDeadBlockWithExit transformation(
      13, SpvOpUnreachable, 0);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  transformation.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %7 %9 %10
          %9 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %12 = OpCopyObject %6 %7
               OpBranch %8
         %10 = OpLabel
               OpBranch %13
          %8 = OpLabel
               OpReturn
         %13 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceBranchFromDeadBlockWithExitTest,
     BranchToOuterMergeBlock) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %15 = OpTypeInt 32 0
         %14 = OpUndef %15
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %8 None
               OpSwitch %14 %9 1 %8
          %9 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %7 %11 %10
          %8 = OpLabel
               OpReturn
         %11 = OpLabel
               OpBranch %8
         %10 = OpLabel
               OpBranch %8
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

  transformation_context.GetFactManager()->AddFactBlockIsDead(10);

  TransformationReplaceBranchFromDeadBlockWithExit transformation(
      10, SpvOpUnreachable, 0);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  transformation.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %15 = OpTypeInt 32 0
         %14 = OpUndef %15
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %8 None
               OpSwitch %14 %9 1 %8
          %9 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %7 %11 %10
          %8 = OpLabel
               OpReturn
         %11 = OpLabel
               OpBranch %8
         %10 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
