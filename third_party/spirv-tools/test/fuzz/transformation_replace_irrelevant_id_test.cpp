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

#include "source/fuzz/transformation_replace_irrelevant_id.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/id_use_descriptor.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {
const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
               OpName %3 "a"
               OpName %4 "b"
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %9 = OpTypeInt 32 1
         %10 = OpTypePointer Function %9
         %11 = OpConstant %9 2
         %12 = OpTypeStruct %9
         %13 = OpTypeInt 32 0
         %14 = OpConstant %13 3
         %15 = OpTypeArray %12 %14
         %16 = OpTypePointer Function %15
         %17 = OpConstant %9 0
          %2 = OpFunction %5 None %6
         %18 = OpLabel
          %3 = OpVariable %10 Function
          %4 = OpVariable %10 Function
         %19 = OpVariable %16 Function
               OpStore %3 %11
         %20 = OpLoad %9 %3
         %21 = OpAccessChain %10 %19 %20 %17
         %22 = OpLoad %9 %21
               OpStore %4 %22
         %23 = OpLoad %9 %4
         %24 = OpIAdd %9 %20 %23
         %25 = OpISub %9 %23 %20
               OpReturn
               OpFunctionEnd
)";

void SetUpIrrelevantIdFacts(FactManager* fact_manager) {
  fact_manager->AddFactIdIsIrrelevant(17);
  fact_manager->AddFactIdIsIrrelevant(23);
  fact_manager->AddFactIdIsIrrelevant(24);
  fact_manager->AddFactIdIsIrrelevant(25);
}

TEST(TransformationReplaceIrrelevantIdTest, Inapplicable) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  SetUpIrrelevantIdFacts(transformation_context.GetFactManager());

  auto instruction_21_descriptor =
      MakeInstructionDescriptor(21, SpvOpAccessChain, 0);
  auto instruction_24_descriptor = MakeInstructionDescriptor(24, SpvOpIAdd, 0);

  // %20 has not been declared as irrelevant.
  ASSERT_FALSE(TransformationReplaceIrrelevantId(
                   MakeIdUseDescriptor(20, instruction_24_descriptor, 0), 23)
                   .IsApplicable(context.get(), transformation_context));

  // %22 is not used in %24.
  ASSERT_FALSE(TransformationReplaceIrrelevantId(
                   MakeIdUseDescriptor(22, instruction_24_descriptor, 1), 20)
                   .IsApplicable(context.get(), transformation_context));

  // Replacement id %50 does not exist.
  ASSERT_FALSE(TransformationReplaceIrrelevantId(
                   MakeIdUseDescriptor(23, instruction_24_descriptor, 1), 50)
                   .IsApplicable(context.get(), transformation_context));

  // %25 is not available to use at %24.
  ASSERT_FALSE(TransformationReplaceIrrelevantId(
                   MakeIdUseDescriptor(23, instruction_24_descriptor, 1), 25)
                   .IsApplicable(context.get(), transformation_context));

  // %24 is not available to use at %24.
  ASSERT_FALSE(TransformationReplaceIrrelevantId(
                   MakeIdUseDescriptor(23, instruction_24_descriptor, 1), 24)
                   .IsApplicable(context.get(), transformation_context));

  // %8 has not the same type as %23.
  ASSERT_FALSE(TransformationReplaceIrrelevantId(
                   MakeIdUseDescriptor(23, instruction_24_descriptor, 1), 8)
                   .IsApplicable(context.get(), transformation_context));

  // %17 is an index to a struct in an access chain, so it can't be replaced.
  ASSERT_FALSE(TransformationReplaceIrrelevantId(
                   MakeIdUseDescriptor(17, instruction_21_descriptor, 2), 20)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceIrrelevantIdTest, Apply) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  SetUpIrrelevantIdFacts(transformation_context.GetFactManager());

  auto instruction_24_descriptor = MakeInstructionDescriptor(24, SpvOpIAdd, 0);

  // Replace the use of %23 in %24 with %22.
  auto transformation = TransformationReplaceIrrelevantId(
      MakeIdUseDescriptor(23, instruction_24_descriptor, 1), 22);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

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
               OpName %3 "a"
               OpName %4 "b"
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %9 = OpTypeInt 32 1
         %10 = OpTypePointer Function %9
         %11 = OpConstant %9 2
         %12 = OpTypeStruct %9
         %13 = OpTypeInt 32 0
         %14 = OpConstant %13 3
         %15 = OpTypeArray %12 %14
         %16 = OpTypePointer Function %15
         %17 = OpConstant %9 0
          %2 = OpFunction %5 None %6
         %18 = OpLabel
          %3 = OpVariable %10 Function
          %4 = OpVariable %10 Function
         %19 = OpVariable %16 Function
               OpStore %3 %11
         %20 = OpLoad %9 %3
         %21 = OpAccessChain %10 %19 %20 %17
         %22 = OpLoad %9 %21
               OpStore %4 %22
         %23 = OpLoad %9 %4
         %24 = OpIAdd %9 %20 %22
         %25 = OpISub %9 %23 %20
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIrrelevantIdTest,
     DoNotReplaceVariableInitializerWithNonConstant) {
  // Checks that it is not possible to replace the initializer of a variable
  // with a non-constant id (such as a function parameter).
  const std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %6
         %13 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %6
         %11 = OpLabel
         %12 = OpVariable %7 Function %13
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(13);

  // We cannot replace the use of %13 in the initializer of %12 with %9 because
  // %9 is not a constant.
  ASSERT_FALSE(TransformationReplaceIrrelevantId(
                   MakeIdUseDescriptor(
                       13, MakeInstructionDescriptor(12, SpvOpVariable, 0), 1),
                   9)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceIrrelevantIdTest,
     DoNotReplaceIrrelevantIdWithOpFunction) {
  // Checks that an OpFunction result id is not allowed to be used to replace an
  // irrelevant id.
  const std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFunction %6
         %13 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpCopyObject %6 %13
         %21 = OpCopyObject %6 %20
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %7
         %11 = OpLabel
               OpReturnValue %13
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(20);

  // We cannot replace the use of %20 in by %21 with %10 because %10 is an
  // OpFunction instruction.
  ASSERT_FALSE(
      TransformationReplaceIrrelevantId(
          MakeIdUseDescriptor(
              20, MakeInstructionDescriptor(21, SpvOpCopyObject, 0), 0),
          10)
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceIrrelevantIdTest, OpAccessChainIrrelevantIndex) {
  // Checks that we can't replace irrelevant index operands in OpAccessChain.
  const std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeVector %6 2
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 0
         %11 = OpConstant %6 2
         %13 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %12 = OpAccessChain %13 %9 %10
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(10);

  // We cannot replace the use of %10 in %12 with %11 because %10 is an
  // irrelevant id.
  ASSERT_FALSE(
      TransformationReplaceIrrelevantId(
          MakeIdUseDescriptor(
              10, MakeInstructionDescriptor(12, SpvOpAccessChain, 0), 1),
          11)
          .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
