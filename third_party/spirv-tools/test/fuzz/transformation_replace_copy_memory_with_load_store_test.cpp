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

#include "source/fuzz/transformation_replace_copy_memory_with_load_store.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationReplaceCopyMemoryWithLoadStoreTest, BasicScenarios) {
  // This is a simple transformation and this test handles the main cases.

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
               OpName %14 "c"
               OpName %16 "d"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %11 = OpConstant %6 3
         %12 = OpTypeFloat 32
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 2
         %17 = OpConstant %12 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %16 = OpVariable %13 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %14 %15
               OpStore %16 %17
               OpCopyMemory %8 %10
               OpCopyMemory %16 %14
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

  auto instruction_descriptor_invalid_1 =
      MakeInstructionDescriptor(5, SpvOpStore, 0);
  auto instruction_descriptor_valid_1 =
      MakeInstructionDescriptor(5, SpvOpCopyMemory, 0);
  auto instruction_descriptor_valid_2 =
      MakeInstructionDescriptor(5, SpvOpCopyMemory, 0);

  // Invalid: |source_id| is not a fresh id.
  auto transformation_invalid_1 = TransformationReplaceCopyMemoryWithLoadStore(
      15, instruction_descriptor_valid_1);
  ASSERT_FALSE(transformation_invalid_1.IsApplicable(context.get(),
                                                     transformation_context));

  // Invalid: |instruction_descriptor_invalid| refers to an instruction OpStore.
  auto transformation_invalid_2 = TransformationReplaceCopyMemoryWithLoadStore(
      20, instruction_descriptor_invalid_1);
  ASSERT_FALSE(transformation_invalid_2.IsApplicable(context.get(),
                                                     transformation_context));

  auto transformation_valid_1 = TransformationReplaceCopyMemoryWithLoadStore(
      20, instruction_descriptor_valid_1);
  ASSERT_TRUE(transformation_valid_1.IsApplicable(context.get(),
                                                  transformation_context));
  ApplyAndCheckFreshIds(transformation_valid_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation_valid_2 = TransformationReplaceCopyMemoryWithLoadStore(
      21, instruction_descriptor_valid_2);
  ASSERT_TRUE(transformation_valid_2.IsApplicable(context.get(),
                                                  transformation_context));
  ApplyAndCheckFreshIds(transformation_valid_2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpName %14 "c"
               OpName %16 "d"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %11 = OpConstant %6 3
         %12 = OpTypeFloat 32
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 2
         %17 = OpConstant %12 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %16 = OpVariable %13 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %14 %15
               OpStore %16 %17
         %20 = OpLoad %6 %10
               OpStore %8 %20
         %21 = OpLoad %12 %14
               OpStore %16 %21
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
