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

#include "source/fuzz/fuzzer_pass_outline_functions.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/pseudo_random_generator.h"
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
               OpName %3 "a"
               OpName %4 "b"
               OpDecorate %3 RelaxedPrecision
               OpDecorate %4 RelaxedPrecision
               OpDecorate %5 RelaxedPrecision
               OpDecorate %6 RelaxedPrecision
               OpDecorate %7 RelaxedPrecision
               OpDecorate %8 RelaxedPrecision
               OpDecorate %9 RelaxedPrecision
         %10 = OpTypeVoid
         %11 = OpTypeFunction %10
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %14 = OpConstant %12 8
         %15 = OpConstant %12 23
         %16 = OpTypeBool
         %17 = OpConstantTrue %16
         %18 = OpConstant %12 0
         %19 = OpConstant %12 1
          %2 = OpFunction %10 None %11
         %20 = OpLabel
          %3 = OpVariable %13 Function
          %4 = OpVariable %13 Function
               OpStore %3 %14
               OpStore %4 %15
               OpBranch %21
         %21 = OpLabel
               OpLoopMerge %22 %23 None
               OpBranch %24
         %24 = OpLabel
         %25 = OpPhi %12 %19 %21 %18 %26
               OpLoopMerge %27 %26 None
               OpBranch %28
         %28 = OpLabel
          %5 = OpLoad %12 %3
         %29 = OpSGreaterThan %16 %5 %18
               OpBranchConditional %29 %30 %27
         %30 = OpLabel
          %6 = OpLoad %12 %4
          %7 = OpISub %12 %6 %19
               OpStore %4 %7
               OpBranch %26
         %26 = OpLabel
          %8 = OpLoad %12 %3
          %9 = OpISub %12 %8 %19
               OpStore %3 %9
               OpBranch %24
         %27 = OpLabel
               OpBranch %23
         %23 = OpLabel
               OpBranch %21
         %22 = OpLabel
               OpBranch %31
         %31 = OpLabel
               OpLoopMerge %32 %31 None
               OpBranchConditional %17 %31 %32
         %32 = OpLabel
               OpSelectionMerge %33 None
               OpBranchConditional %17 %34 %35
         %34 = OpLabel
               OpBranch %33
         %35 = OpLabel
               OpBranch %33
         %33 = OpLabel
         %42 = OpPhi %12 %19 %33 %18 %34 %18 %35
               OpLoopMerge %36 %33 None
               OpBranchConditional %17 %36 %33
         %36 = OpLabel
         %43 = OpPhi %12 %18 %33 %18 %41
               OpReturn
         %37 = OpLabel
               OpLoopMerge %38 %39 None
               OpBranch %40
         %40 = OpLabel
               OpBranchConditional %17 %41 %38
         %41 = OpLabel
               OpBranchConditional %17 %36 %39
         %39 = OpLabel
               OpBranch %37
         %38 = OpLabel
               OpReturn
               OpFunctionEnd
)";

TEST(FuzzerPassOutlineFunctionsTest, EntryIsAlreadySuitable) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassOutlineFunctions fuzzer_pass(context.get(), &transformation_context,
                                         &fuzzer_context,
                                         &transformation_sequence, false);

  // Block 28
  auto suitable_entry_block =
      fuzzer_pass.MaybeGetEntryBlockSuitableForOutlining(
          context->get_instr_block(28));

  ASSERT_TRUE(suitable_entry_block);
  ASSERT_TRUE(suitable_entry_block->GetLabel()->result_id() == 28);

  // Block 32
  suitable_entry_block = fuzzer_pass.MaybeGetEntryBlockSuitableForOutlining(
      context->get_instr_block(32));

  ASSERT_TRUE(suitable_entry_block);
  ASSERT_TRUE(suitable_entry_block->GetLabel()->result_id() == 32);

  // Block 41
  suitable_entry_block = fuzzer_pass.MaybeGetEntryBlockSuitableForOutlining(
      context->get_instr_block(41));

  ASSERT_TRUE(suitable_entry_block);
  ASSERT_TRUE(suitable_entry_block->GetLabel()->result_id() == 41);

  // The module should not have been changed.
  ASSERT_TRUE(IsEqual(env, shader, context.get()));
}

TEST(FuzzerPassOutlineFunctionsTest, EntryHasOpVariable) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassOutlineFunctions fuzzer_pass(context.get(), &transformation_context,
                                         &fuzzer_context,
                                         &transformation_sequence, false);

  // Block 20
  auto suitable_entry_block =
      fuzzer_pass.MaybeGetEntryBlockSuitableForOutlining(
          context->get_instr_block(20));

  // The block should have been split, the new entry block being the block
  // generated by the splitting.
  ASSERT_TRUE(suitable_entry_block);
  ASSERT_TRUE(suitable_entry_block->GetLabel()->result_id() == 100);

  std::string after_adjustment = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
               OpName %3 "a"
               OpName %4 "b"
               OpDecorate %3 RelaxedPrecision
               OpDecorate %4 RelaxedPrecision
               OpDecorate %5 RelaxedPrecision
               OpDecorate %6 RelaxedPrecision
               OpDecorate %7 RelaxedPrecision
               OpDecorate %8 RelaxedPrecision
               OpDecorate %9 RelaxedPrecision
         %10 = OpTypeVoid
         %11 = OpTypeFunction %10
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %14 = OpConstant %12 8
         %15 = OpConstant %12 23
         %16 = OpTypeBool
         %17 = OpConstantTrue %16
         %18 = OpConstant %12 0
         %19 = OpConstant %12 1
          %2 = OpFunction %10 None %11
         %20 = OpLabel
          %3 = OpVariable %13 Function
          %4 = OpVariable %13 Function
               OpBranch %100
        %100 = OpLabel
               OpStore %3 %14
               OpStore %4 %15
               OpBranch %21
         %21 = OpLabel
               OpLoopMerge %22 %23 None
               OpBranch %24
         %24 = OpLabel
         %25 = OpPhi %12 %19 %21 %18 %26
               OpLoopMerge %27 %26 None
               OpBranch %28
         %28 = OpLabel
          %5 = OpLoad %12 %3
         %29 = OpSGreaterThan %16 %5 %18
               OpBranchConditional %29 %30 %27
         %30 = OpLabel
          %6 = OpLoad %12 %4
          %7 = OpISub %12 %6 %19
               OpStore %4 %7
               OpBranch %26
         %26 = OpLabel
          %8 = OpLoad %12 %3
          %9 = OpISub %12 %8 %19
               OpStore %3 %9
               OpBranch %24
         %27 = OpLabel
               OpBranch %23
         %23 = OpLabel
               OpBranch %21
         %22 = OpLabel
               OpBranch %31
         %31 = OpLabel
               OpLoopMerge %32 %31 None
               OpBranchConditional %17 %31 %32
         %32 = OpLabel
               OpSelectionMerge %33 None
               OpBranchConditional %17 %34 %35
         %34 = OpLabel
               OpBranch %33
         %35 = OpLabel
               OpBranch %33
         %33 = OpLabel
         %42 = OpPhi %12 %19 %33 %18 %34 %18 %35
               OpLoopMerge %36 %33 None
               OpBranchConditional %17 %36 %33
         %36 = OpLabel
         %43 = OpPhi %12 %18 %33 %18 %41
               OpReturn
         %37 = OpLabel
               OpLoopMerge %38 %39 None
               OpBranch %40
         %40 = OpLabel
               OpBranchConditional %17 %41 %38
         %41 = OpLabel
               OpBranchConditional %17 %36 %39
         %39 = OpLabel
               OpBranch %37
         %38 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_adjustment, context.get()));
}

TEST(FuzzerPassOutlineFunctionsTest, EntryBlockIsHeader) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassOutlineFunctions fuzzer_pass(context.get(), &transformation_context,
                                         &fuzzer_context,
                                         &transformation_sequence, false);

  // Block 21
  auto suitable_entry_block =
      fuzzer_pass.MaybeGetEntryBlockSuitableForOutlining(
          context->get_instr_block(21));

  // A suitable entry block should have been found by finding the preheader
  // (%20) and then splitting it.
  ASSERT_TRUE(suitable_entry_block);
  ASSERT_TRUE(suitable_entry_block->GetLabel()->result_id() == 100);

  // Block 24
  suitable_entry_block = fuzzer_pass.MaybeGetEntryBlockSuitableForOutlining(
      context->get_instr_block(24));

  // A preheader should have been created, because the current one is a loop
  // header.
  ASSERT_TRUE(suitable_entry_block);
  ASSERT_TRUE(suitable_entry_block->GetLabel()->result_id() == 101);

  // Block 31
  suitable_entry_block = fuzzer_pass.MaybeGetEntryBlockSuitableForOutlining(
      context->get_instr_block(31));

  // An existing suitable entry block should have been found by finding the
  // preheader (%22), which is already suitable.
  ASSERT_TRUE(suitable_entry_block);
  ASSERT_TRUE(suitable_entry_block->GetLabel()->result_id() == 22);

  // Block 33
  suitable_entry_block = fuzzer_pass.MaybeGetEntryBlockSuitableForOutlining(
      context->get_instr_block(33));

  // An existing suitable entry block should have been found by creating a new
  // preheader (there is not one already), and then splitting it (as it contains
  // OpPhi).
  ASSERT_TRUE(suitable_entry_block);
  ASSERT_TRUE(suitable_entry_block->GetLabel()->result_id() == 104);

  // Block 37
  suitable_entry_block = fuzzer_pass.MaybeGetEntryBlockSuitableForOutlining(
      context->get_instr_block(37));

  // No suitable entry block can be found for block 37, since it is a loop
  // header with only one predecessor (the back-edge block).
  ASSERT_FALSE(suitable_entry_block);

  std::string after_adjustments = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
               OpName %3 "a"
               OpName %4 "b"
               OpDecorate %3 RelaxedPrecision
               OpDecorate %4 RelaxedPrecision
               OpDecorate %5 RelaxedPrecision
               OpDecorate %6 RelaxedPrecision
               OpDecorate %7 RelaxedPrecision
               OpDecorate %8 RelaxedPrecision
               OpDecorate %9 RelaxedPrecision
         %10 = OpTypeVoid
         %11 = OpTypeFunction %10
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %14 = OpConstant %12 8
         %15 = OpConstant %12 23
         %16 = OpTypeBool
         %17 = OpConstantTrue %16
         %18 = OpConstant %12 0
         %19 = OpConstant %12 1
          %2 = OpFunction %10 None %11
         %20 = OpLabel
          %3 = OpVariable %13 Function
          %4 = OpVariable %13 Function
               OpBranch %100
        %100 = OpLabel
               OpStore %3 %14
               OpStore %4 %15
               OpBranch %21
         %21 = OpLabel
               OpLoopMerge %22 %23 None
               OpBranch %101
        %101 = OpLabel
               OpBranch %24
         %24 = OpLabel
         %25 = OpPhi %12 %19 %101 %18 %26
               OpLoopMerge %27 %26 None
               OpBranch %28
         %28 = OpLabel
          %5 = OpLoad %12 %3
         %29 = OpSGreaterThan %16 %5 %18
               OpBranchConditional %29 %30 %27
         %30 = OpLabel
          %6 = OpLoad %12 %4
          %7 = OpISub %12 %6 %19
               OpStore %4 %7
               OpBranch %26
         %26 = OpLabel
          %8 = OpLoad %12 %3
          %9 = OpISub %12 %8 %19
               OpStore %3 %9
               OpBranch %24
         %27 = OpLabel
               OpBranch %23
         %23 = OpLabel
               OpBranch %21
         %22 = OpLabel
               OpBranch %31
         %31 = OpLabel
               OpLoopMerge %32 %31 None
               OpBranchConditional %17 %31 %32
         %32 = OpLabel
               OpSelectionMerge %102 None
               OpBranchConditional %17 %34 %35
         %34 = OpLabel
               OpBranch %102
         %35 = OpLabel
               OpBranch %102
        %102 = OpLabel
        %103 = OpPhi %12 %18 %34 %18 %35
               OpBranch %104
        %104 = OpLabel
               OpBranch %33
         %33 = OpLabel
         %42 = OpPhi %12 %103 %104 %19 %33
               OpLoopMerge %36 %33 None
               OpBranchConditional %17 %36 %33
         %36 = OpLabel
         %43 = OpPhi %12 %18 %33 %18 %41
               OpReturn
         %37 = OpLabel
               OpLoopMerge %38 %39 None
               OpBranch %40
         %40 = OpLabel
               OpBranchConditional %17 %41 %38
         %41 = OpLabel
               OpBranchConditional %17 %36 %39
         %39 = OpLabel
               OpBranch %37
         %38 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_adjustments, context.get()));
}

TEST(FuzzerPassOutlineFunctionsTest, ExitBlock) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassOutlineFunctions fuzzer_pass(context.get(), &transformation_context,
                                         &fuzzer_context,
                                         &transformation_sequence, false);

  // Block 39 is not a merge block, so it is already suitable.
  auto suitable_exit_block = fuzzer_pass.MaybeGetExitBlockSuitableForOutlining(
      context->get_instr_block(39));
  ASSERT_TRUE(suitable_exit_block);
  ASSERT_TRUE(suitable_exit_block->GetLabel()->result_id() == 39);

  // The following are merge blocks and, thus, they will need to be split.

  // Block 22
  suitable_exit_block = fuzzer_pass.MaybeGetExitBlockSuitableForOutlining(
      context->get_instr_block(22));
  ASSERT_TRUE(suitable_exit_block);
  ASSERT_TRUE(suitable_exit_block->GetLabel()->result_id() == 100);

  // Block 27
  suitable_exit_block = fuzzer_pass.MaybeGetExitBlockSuitableForOutlining(
      context->get_instr_block(27));
  ASSERT_TRUE(suitable_exit_block);
  ASSERT_TRUE(suitable_exit_block->GetLabel()->result_id() == 101);

  // Block 36
  suitable_exit_block = fuzzer_pass.MaybeGetExitBlockSuitableForOutlining(
      context->get_instr_block(36));
  ASSERT_TRUE(suitable_exit_block);
  ASSERT_TRUE(suitable_exit_block->GetLabel()->result_id() == 102);

  std::string after_adjustments = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
               OpName %3 "a"
               OpName %4 "b"
               OpDecorate %3 RelaxedPrecision
               OpDecorate %4 RelaxedPrecision
               OpDecorate %5 RelaxedPrecision
               OpDecorate %6 RelaxedPrecision
               OpDecorate %7 RelaxedPrecision
               OpDecorate %8 RelaxedPrecision
               OpDecorate %9 RelaxedPrecision
         %10 = OpTypeVoid
         %11 = OpTypeFunction %10
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %14 = OpConstant %12 8
         %15 = OpConstant %12 23
         %16 = OpTypeBool
         %17 = OpConstantTrue %16
         %18 = OpConstant %12 0
         %19 = OpConstant %12 1
          %2 = OpFunction %10 None %11
         %20 = OpLabel
          %3 = OpVariable %13 Function
          %4 = OpVariable %13 Function
               OpStore %3 %14
               OpStore %4 %15
               OpBranch %21
         %21 = OpLabel
               OpLoopMerge %22 %23 None
               OpBranch %24
         %24 = OpLabel
         %25 = OpPhi %12 %19 %21 %18 %26
               OpLoopMerge %27 %26 None
               OpBranch %28
         %28 = OpLabel
          %5 = OpLoad %12 %3
         %29 = OpSGreaterThan %16 %5 %18
               OpBranchConditional %29 %30 %27
         %30 = OpLabel
          %6 = OpLoad %12 %4
          %7 = OpISub %12 %6 %19
               OpStore %4 %7
               OpBranch %26
         %26 = OpLabel
          %8 = OpLoad %12 %3
          %9 = OpISub %12 %8 %19
               OpStore %3 %9
               OpBranch %24
         %27 = OpLabel
               OpBranch %101
        %101 = OpLabel
               OpBranch %23
         %23 = OpLabel
               OpBranch %21
         %22 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpBranch %31
         %31 = OpLabel
               OpLoopMerge %32 %31 None
               OpBranchConditional %17 %31 %32
         %32 = OpLabel
               OpSelectionMerge %33 None
               OpBranchConditional %17 %34 %35
         %34 = OpLabel
               OpBranch %33
         %35 = OpLabel
               OpBranch %33
         %33 = OpLabel
         %42 = OpPhi %12 %19 %33 %18 %34 %18 %35
               OpLoopMerge %36 %33 None
               OpBranchConditional %17 %36 %33
         %36 = OpLabel
         %43 = OpPhi %12 %18 %33 %18 %41
               OpBranch %102
        %102 = OpLabel
               OpReturn
         %37 = OpLabel
               OpLoopMerge %38 %39 None
               OpBranch %40
         %40 = OpLabel
               OpBranchConditional %17 %41 %38
         %41 = OpLabel
               OpBranchConditional %17 %36 %39
         %39 = OpLabel
               OpBranch %37
         %38 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_adjustments, context.get()));
}
}  // namespace
}  // namespace fuzz
}  // namespace spvtools
