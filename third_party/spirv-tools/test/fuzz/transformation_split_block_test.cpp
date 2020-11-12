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

#include "source/fuzz/transformation_split_block.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationSplitBlockTest, NotApplicable) {
  // The SPIR-V in this test came from the following fragment shader, with
  // local store elimination applied to get some OpPhi instructions.
  //
  // void main() {
  //   int x;
  //   int i;
  //   for (i = 0; i < 100; i++) {
  //     x += i;
  //   }
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i"
               OpName %19 "x"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %24 = OpConstant %6 1
         %28 = OpUndef %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %19 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %10 = OpLabel
         %27 = OpPhi %6 %28 %5 %22 %13
         %26 = OpPhi %6 %9 %5 %25 %13
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %18 = OpSLessThan %17 %26 %16
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
         %22 = OpIAdd %6 %27 %26
               OpStore %19 %22
               OpBranch %13
         %13 = OpLabel
         %25 = OpIAdd %6 %26 %24
               OpStore %8 %25
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // No split before OpVariable
  ASSERT_FALSE(TransformationSplitBlock(
                   MakeInstructionDescriptor(8, SpvOpVariable, 0), 100)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSplitBlock(
                   MakeInstructionDescriptor(8, SpvOpVariable, 1), 100)
                   .IsApplicable(context.get(), transformation_context));

  // No split before OpLabel
  ASSERT_FALSE(TransformationSplitBlock(
                   MakeInstructionDescriptor(14, SpvOpLabel, 0), 100)
                   .IsApplicable(context.get(), transformation_context));

  // No split if base instruction is outside a function
  ASSERT_FALSE(
      TransformationSplitBlock(MakeInstructionDescriptor(1, SpvOpLabel, 0), 100)
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSplitBlock(
                   MakeInstructionDescriptor(1, SpvOpExecutionMode, 0), 100)
                   .IsApplicable(context.get(), transformation_context));

  // No split if block is loop header
  ASSERT_FALSE(
      TransformationSplitBlock(MakeInstructionDescriptor(27, SpvOpPhi, 0), 100)
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationSplitBlock(MakeInstructionDescriptor(27, SpvOpPhi, 1), 100)
          .IsApplicable(context.get(), transformation_context));

  // No split if base instruction does not exist
  ASSERT_FALSE(
      TransformationSplitBlock(MakeInstructionDescriptor(88, SpvOpIAdd, 0), 100)
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSplitBlock(
                   MakeInstructionDescriptor(88, SpvOpIMul, 22), 100)
                   .IsApplicable(context.get(), transformation_context));

  // No split if too many instructions with the desired opcode are skipped
  ASSERT_FALSE(
      TransformationSplitBlock(
          MakeInstructionDescriptor(18, SpvOpBranchConditional, 1), 100)
          .IsApplicable(context.get(), transformation_context));

  // No split if id in use
  ASSERT_FALSE(TransformationSplitBlock(
                   MakeInstructionDescriptor(18, SpvOpSLessThan, 0), 27)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSplitBlock(
                   MakeInstructionDescriptor(18, SpvOpSLessThan, 0), 14)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationSplitBlockTest, SplitBlockSeveralTimes) {
  // The SPIR-V in this test came from the following fragment shader:
  //
  // void main() {
  //   int a;
  //   int b;
  //   a = 1;
  //   b = a;
  //   a = b;
  //   b = 2;
  //   b++;
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
               OpDecorate %14 RelaxedPrecision
               OpDecorate %15 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %13 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpStore %8 %9
         %11 = OpLoad %6 %8
               OpStore %10 %11
         %12 = OpLoad %6 %10
               OpStore %8 %12
               OpStore %10 %13
         %14 = OpLoad %6 %10
         %15 = OpIAdd %6 %14 %9
               OpStore %10 %15
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto split_1 = TransformationSplitBlock(
      MakeInstructionDescriptor(5, SpvOpStore, 0), 100);
  ASSERT_TRUE(split_1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(split_1, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_split_1 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
               OpDecorate %14 RelaxedPrecision
               OpDecorate %15 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %13 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpBranch %100
        %100 = OpLabel
               OpStore %8 %9
         %11 = OpLoad %6 %8
               OpStore %10 %11
         %12 = OpLoad %6 %10
               OpStore %8 %12
               OpStore %10 %13
         %14 = OpLoad %6 %10
         %15 = OpIAdd %6 %14 %9
               OpStore %10 %15
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_split_1, context.get()));

  auto split_2 = TransformationSplitBlock(
      MakeInstructionDescriptor(11, SpvOpStore, 0), 101);
  ASSERT_TRUE(split_2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(split_2, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_split_2 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
               OpDecorate %14 RelaxedPrecision
               OpDecorate %15 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %13 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpBranch %100
        %100 = OpLabel
               OpStore %8 %9
         %11 = OpLoad %6 %8
               OpBranch %101
        %101 = OpLabel
               OpStore %10 %11
         %12 = OpLoad %6 %10
               OpStore %8 %12
               OpStore %10 %13
         %14 = OpLoad %6 %10
         %15 = OpIAdd %6 %14 %9
               OpStore %10 %15
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_split_2, context.get()));

  auto split_3 = TransformationSplitBlock(
      MakeInstructionDescriptor(14, SpvOpLoad, 0), 102);
  ASSERT_TRUE(split_3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(split_3, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_split_3 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
               OpDecorate %14 RelaxedPrecision
               OpDecorate %15 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %13 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpBranch %100
        %100 = OpLabel
               OpStore %8 %9
         %11 = OpLoad %6 %8
               OpBranch %101
        %101 = OpLabel
               OpStore %10 %11
         %12 = OpLoad %6 %10
               OpStore %8 %12
               OpStore %10 %13
               OpBranch %102
        %102 = OpLabel
         %14 = OpLoad %6 %10
         %15 = OpIAdd %6 %14 %9
               OpStore %10 %15
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_split_3, context.get()));
}

TEST(TransformationSplitBlockTest, SplitBlockBeforeSelectBranch) {
  // The SPIR-V in this test came from the following fragment shader:
  //
  // void main() {
  //   int x, y;
  //   x = 2;
  //   if (x < y) {
  //     y = 3;
  //   } else {
  //     y = 4;
  //   }
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %11 "y"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %13 = OpTypeBool
         %17 = OpConstant %6 3
         %19 = OpConstant %6 4
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %11 = OpVariable %7 Function
               OpStore %8 %9
         %10 = OpLoad %6 %8
         %12 = OpLoad %6 %11
         %14 = OpSLessThan %13 %10 %12
               OpSelectionMerge %16 None
               OpBranchConditional %14 %15 %18
         %15 = OpLabel
               OpStore %11 %17
               OpBranch %16
         %18 = OpLabel
               OpStore %11 %19
               OpBranch %16
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Illegal to split between the merge and the conditional branch.
  ASSERT_FALSE(
      TransformationSplitBlock(
          MakeInstructionDescriptor(14, SpvOpBranchConditional, 0), 100)
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationSplitBlock(
          MakeInstructionDescriptor(12, SpvOpBranchConditional, 0), 100)
          .IsApplicable(context.get(), transformation_context));

  auto split = TransformationSplitBlock(
      MakeInstructionDescriptor(14, SpvOpSelectionMerge, 0), 100);
  ASSERT_TRUE(split.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(split, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_split = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %11 "y"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %13 = OpTypeBool
         %17 = OpConstant %6 3
         %19 = OpConstant %6 4
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %11 = OpVariable %7 Function
               OpStore %8 %9
         %10 = OpLoad %6 %8
         %12 = OpLoad %6 %11
         %14 = OpSLessThan %13 %10 %12
               OpBranch %100
        %100 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %14 %15 %18
         %15 = OpLabel
               OpStore %11 %17
               OpBranch %16
         %18 = OpLabel
               OpStore %11 %19
               OpBranch %16
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_split, context.get()));
}

TEST(TransformationSplitBlockTest, SplitBlockBeforeSwitchBranch) {
  // The SPIR-V in this test came from the following fragment shader:
  //
  // void main() {
  //   int x, y;
  //   switch (y) {
  //     case 1:
  //       x = 2;
  //     case 2:
  //       break;
  //     case 3:
  //       x = 4;
  //     default:
  //       x = 6;
  //   }
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "y"
               OpName %15 "x"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %9 RelaxedPrecision
               OpDecorate %15 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %16 = OpConstant %6 2
         %18 = OpConstant %6 4
         %19 = OpConstant %6 6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %15 = OpVariable %7 Function
          %9 = OpLoad %6 %8
               OpSelectionMerge %14 None
               OpSwitch %9 %13 1 %10 2 %11 3 %12
         %13 = OpLabel
               OpStore %15 %19
               OpBranch %14
         %10 = OpLabel
               OpStore %15 %16
               OpBranch %11
         %11 = OpLabel
               OpBranch %14
         %12 = OpLabel
               OpStore %15 %18
               OpBranch %13
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Illegal to split between the merge and the conditional branch.
  ASSERT_FALSE(TransformationSplitBlock(
                   MakeInstructionDescriptor(9, SpvOpSwitch, 0), 100)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSplitBlock(
                   MakeInstructionDescriptor(15, SpvOpSwitch, 0), 100)
                   .IsApplicable(context.get(), transformation_context));

  auto split = TransformationSplitBlock(
      MakeInstructionDescriptor(9, SpvOpSelectionMerge, 0), 100);
  ASSERT_TRUE(split.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(split, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_split = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "y"
               OpName %15 "x"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %9 RelaxedPrecision
               OpDecorate %15 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %16 = OpConstant %6 2
         %18 = OpConstant %6 4
         %19 = OpConstant %6 6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %15 = OpVariable %7 Function
          %9 = OpLoad %6 %8
               OpBranch %100
        %100 = OpLabel
               OpSelectionMerge %14 None
               OpSwitch %9 %13 1 %10 2 %11 3 %12
         %13 = OpLabel
               OpStore %15 %19
               OpBranch %14
         %10 = OpLabel
               OpStore %15 %16
               OpBranch %11
         %11 = OpLabel
               OpBranch %14
         %12 = OpLabel
               OpStore %15 %18
               OpBranch %13
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_split, context.get()));
}

TEST(TransformationSplitBlockTest, NoSplitDuringOpPhis) {
  // The SPIR-V in this test came from the following fragment shader, with
  // local store elimination applied to get some OpPhi instructions.
  //
  // void main() {
  //   int x;
  //   int i;
  //   for (i = 0; i < 100; i++) {
  //     x += i;
  //   }
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i"
               OpName %19 "x"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %24 = OpConstant %6 1
         %28 = OpUndef %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %19 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %10 = OpLabel
         %27 = OpPhi %6 %28 %5 %22 %13
         %26 = OpPhi %6 %9 %5 %25 %13
               OpBranch %50
         %50 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %18 = OpSLessThan %17 %26 %16
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
         %22 = OpIAdd %6 %27 %26
               OpStore %19 %22
               OpBranch %13
         %13 = OpLabel
         %25 = OpIAdd %6 %26 %24
               OpStore %8 %25
               OpBranch %50
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // We cannot split before OpPhi instructions, since the number of incoming
  // blocks may not appropriately match after splitting.
  ASSERT_FALSE(
      TransformationSplitBlock(MakeInstructionDescriptor(26, SpvOpPhi, 0), 100)
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationSplitBlock(MakeInstructionDescriptor(27, SpvOpPhi, 0), 100)
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationSplitBlock(MakeInstructionDescriptor(27, SpvOpPhi, 1), 100)
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationSplitBlockTest, SplitOpPhiWithSinglePredecessor) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "y"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpStore %8 %9
         %11 = OpLoad %6 %8
               OpBranch %20
         %20 = OpLabel
         %21 = OpPhi %6 %11 %5
               OpStore %10 %21
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(
      TransformationSplitBlock(MakeInstructionDescriptor(21, SpvOpPhi, 0), 100)
          .IsApplicable(context.get(), transformation_context));
  // An equivalent transformation to the above, just described with respect to a
  // different base instruction.
  auto split =
      TransformationSplitBlock(MakeInstructionDescriptor(20, SpvOpPhi, 0), 100);
  ASSERT_TRUE(split.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(split, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_split = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "y"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpStore %8 %9
         %11 = OpLoad %6 %8
               OpBranch %20
         %20 = OpLabel
               OpBranch %100
        %100 = OpLabel
         %21 = OpPhi %6 %11 %20
               OpStore %10 %21
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_split, context.get()));
}

TEST(TransformationSplitBlockTest, DeadBlockShouldSplitToTwoDeadBlocks) {
  // This checks that if a block B is marked as dead, it should split into a
  // pair of dead blocks.
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
               OpSelectionMerge %9 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Record the fact that block 8 is dead.
  transformation_context.GetFactManager()->AddFactBlockIsDead(8);

  auto split = TransformationSplitBlock(
      MakeInstructionDescriptor(8, SpvOpBranch, 0), 100);
  ASSERT_TRUE(split.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(split, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(8));
  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(100));

  std::string after_split = R"(
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
               OpSelectionMerge %9 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_split, context.get()));
}

TEST(TransformationSplitBlockTest, DoNotSplitUseOfOpSampledImage) {
  // This checks that we cannot split the definition of an OpSampledImage
  // from its use.
  std::string shader = R"(
               OpCapability Shader
               OpCapability SampledBuffer
               OpCapability ImageBuffer
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main" %40 %41
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %40 DescriptorSet 0
               OpDecorate %40 Binding 69
               OpDecorate %41 DescriptorSet 0
               OpDecorate %41 Binding 1
         %54 = OpTypeFloat 32
         %76 = OpTypeVector %54 4
         %55 = OpConstant %54 0
         %56 = OpTypeVector %54 3
         %94 = OpTypeVector %54 2
        %112 = OpConstantComposite %94 %55 %55
         %57 = OpConstantComposite %56 %55 %55 %55
         %15 = OpTypeImage %54 2D 2 0 0 1 Unknown
        %114 = OpTypePointer UniformConstant %15
         %38 = OpTypeSampler
        %125 = OpTypePointer UniformConstant %38
        %132 = OpTypeVoid
        %133 = OpTypeFunction %132
         %45 = OpTypeSampledImage %15
         %40 = OpVariable %114 UniformConstant
         %41 = OpVariable %125 UniformConstant
          %2 = OpFunction %132 None %133
        %164 = OpLabel
        %184 = OpLoad %15 %40
        %213 = OpLoad %38 %41
        %216 = OpSampledImage %45 %184 %213
        %217 = OpImageSampleImplicitLod %76 %216 %112 Bias %55
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto split = TransformationSplitBlock(
      MakeInstructionDescriptor(217, SpvOpImageSampleImplicitLod, 0), 500);
  ASSERT_FALSE(split.IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
