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

#include "source/fuzz/transformation_replace_load_store_with_copy_memory.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationReplaceLoadStoreWithCopyMemoryTest, BasicScenarios) {
  // This is a simple transformation and this test handles the main cases.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %26 %28 %31 %33
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpName %12 "c"
               OpName %14 "d"
               OpName %18 "e"
               OpName %20 "f"
               OpName %26 "i1"
               OpName %28 "i2"
               OpName %31 "g1"
               OpName %33 "g2"
               OpDecorate %26 Location 0
               OpDecorate %28 Location 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %11 = OpConstant %6 3
         %13 = OpConstant %6 4
         %15 = OpConstant %6 5
         %16 = OpTypeFloat 32
         %17 = OpTypePointer Function %16
         %19 = OpConstant %16 2
         %21 = OpConstant %16 3
         %25 = OpTypePointer Output %6
         %26 = OpVariable %25 Output
         %27 = OpConstant %6 1
         %28 = OpVariable %25 Output
         %30 = OpTypePointer Private %6
         %31 = OpVariable %30 Private
         %32 = OpConstant %6 0
         %33 = OpVariable %30 Private
         %35 = OpTypeBool
         %36 = OpConstantTrue %35
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %14 = OpVariable %7 Function
         %18 = OpVariable %17 Function
         %20 = OpVariable %17 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %13
               OpStore %14 %15
               OpStore %18 %19
               OpStore %20 %21
         %22 = OpLoad %6 %8
               OpCopyMemory %10 %8
               OpStore %10 %22
         %23 = OpLoad %6 %12
               OpStore %14 %23
         %24 = OpLoad %16 %18
               OpStore %20 %24
               OpStore %26 %27
               OpStore %28 %27
         %29 = OpLoad %6 %26
               OpMemoryBarrier %32 %32
               OpStore %28 %29
               OpStore %31 %32
               OpStore %33 %32
         %34 = OpLoad %6 %33
               OpSelectionMerge %38 None
               OpBranchConditional %36 %37 %38
         %37 = OpLabel
               OpStore %31 %34
               OpBranch %38
         %38 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto bad_instruction_descriptor_1 =
      MakeInstructionDescriptor(11, spv::Op::OpConstant, 0);

  auto load_instruction_descriptor_1 =
      MakeInstructionDescriptor(22, spv::Op::OpLoad, 0);
  auto load_instruction_descriptor_2 =
      MakeInstructionDescriptor(23, spv::Op::OpLoad, 0);
  auto load_instruction_descriptor_3 =
      MakeInstructionDescriptor(24, spv::Op::OpLoad, 0);
  auto load_instruction_descriptor_other_block =
      MakeInstructionDescriptor(34, spv::Op::OpLoad, 0);
  auto load_instruction_descriptor_unsafe =
      MakeInstructionDescriptor(29, spv::Op::OpLoad, 0);

  auto store_instruction_descriptor_1 =
      MakeInstructionDescriptor(22, spv::Op::OpStore, 0);
  auto store_instruction_descriptor_2 =
      MakeInstructionDescriptor(23, spv::Op::OpStore, 0);
  auto store_instruction_descriptor_3 =
      MakeInstructionDescriptor(24, spv::Op::OpStore, 0);
  auto store_instruction_descriptor_other_block =
      MakeInstructionDescriptor(37, spv::Op::OpStore, 0);
  auto store_instruction_descriptor_unsafe =
      MakeInstructionDescriptor(29, spv::Op::OpStore, 0);

  // Bad: |load_instruction_descriptor| is incorrect.
  auto transformation_bad_1 = TransformationReplaceLoadStoreWithCopyMemory(
      bad_instruction_descriptor_1, store_instruction_descriptor_1);
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Bad: |store_instruction_descriptor| is incorrect.
  auto transformation_bad_2 = TransformationReplaceLoadStoreWithCopyMemory(
      load_instruction_descriptor_1, bad_instruction_descriptor_1);
  ASSERT_FALSE(
      transformation_bad_2.IsApplicable(context.get(), transformation_context));

  // Bad: Intermediate values of the OpLoad and the OpStore don't match.
  auto transformation_bad_3 = TransformationReplaceLoadStoreWithCopyMemory(
      load_instruction_descriptor_1, store_instruction_descriptor_2);
  ASSERT_FALSE(
      transformation_bad_3.IsApplicable(context.get(), transformation_context));

  // Bad: There is an interfering OpCopyMemory instruction between the OpLoad
  // and the OpStore.
  auto transformation_bad_4 = TransformationReplaceLoadStoreWithCopyMemory(
      load_instruction_descriptor_1, store_instruction_descriptor_1);
  ASSERT_FALSE(
      transformation_bad_4.IsApplicable(context.get(), transformation_context));

  // Bad: There is an interfering OpMemoryBarrier instruction between the OpLoad
  // and the OpStore.
  auto transformation_bad_5 = TransformationReplaceLoadStoreWithCopyMemory(
      load_instruction_descriptor_unsafe, store_instruction_descriptor_unsafe);
  ASSERT_FALSE(
      transformation_bad_5.IsApplicable(context.get(), transformation_context));

  // Bad: OpLoad and OpStore instructions are in different blocks.
  auto transformation_bad_6 = TransformationReplaceLoadStoreWithCopyMemory(
      load_instruction_descriptor_other_block,
      store_instruction_descriptor_other_block);
  ASSERT_FALSE(
      transformation_bad_6.IsApplicable(context.get(), transformation_context));

  auto transformation_good_1 = TransformationReplaceLoadStoreWithCopyMemory(
      load_instruction_descriptor_2, store_instruction_descriptor_2);
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation_good_2 = TransformationReplaceLoadStoreWithCopyMemory(
      load_instruction_descriptor_3, store_instruction_descriptor_3);
  ASSERT_TRUE(transformation_good_2.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %26 %28 %31 %33
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpName %12 "c"
               OpName %14 "d"
               OpName %18 "e"
               OpName %20 "f"
               OpName %26 "i1"
               OpName %28 "i2"
               OpName %31 "g1"
               OpName %33 "g2"
               OpDecorate %26 Location 0
               OpDecorate %28 Location 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %11 = OpConstant %6 3
         %13 = OpConstant %6 4
         %15 = OpConstant %6 5
         %16 = OpTypeFloat 32
         %17 = OpTypePointer Function %16
         %19 = OpConstant %16 2
         %21 = OpConstant %16 3
         %25 = OpTypePointer Output %6
         %26 = OpVariable %25 Output
         %27 = OpConstant %6 1
         %28 = OpVariable %25 Output
         %30 = OpTypePointer Private %6
         %31 = OpVariable %30 Private
         %32 = OpConstant %6 0
         %33 = OpVariable %30 Private
         %35 = OpTypeBool
         %36 = OpConstantTrue %35
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %14 = OpVariable %7 Function
         %18 = OpVariable %17 Function
         %20 = OpVariable %17 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %13
               OpStore %14 %15
               OpStore %18 %19
               OpStore %20 %21
         %22 = OpLoad %6 %8
               OpCopyMemory %10 %8
               OpStore %10 %22
         %23 = OpLoad %6 %12
               OpCopyMemory %14 %12
         %24 = OpLoad %16 %18
               OpCopyMemory %20 %18
               OpStore %26 %27
               OpStore %28 %27
         %29 = OpLoad %6 %26
               OpMemoryBarrier %32 %32
               OpStore %28 %29
               OpStore %31 %32
               OpStore %33 %32
         %34 = OpLoad %6 %33
               OpSelectionMerge %38 None
               OpBranchConditional %36 %37 %38
         %37 = OpLabel
               OpStore %31 %34
               OpBranch %38
         %38 = OpLabel
               OpReturn
               OpFunctionEnd
    )";
  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
