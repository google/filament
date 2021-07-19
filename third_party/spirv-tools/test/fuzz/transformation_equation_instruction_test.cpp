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

#include "source/fuzz/transformation_equation_instruction.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationEquationInstructionTest, SignedNegate) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 24
         %40 = OpTypeBool
         %41 = OpConstantTrue %40
         %20 = OpUndef %6
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %30 = OpCopyObject %6 %7
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
  protobufs::InstructionDescriptor return_instruction =
      MakeInstructionDescriptor(13, SpvOpReturn, 0);

  // Bad: id already in use.
  ASSERT_FALSE(TransformationEquationInstruction(7, SpvOpSNegate, {7},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: identified instruction does not exist.
  ASSERT_FALSE(
      TransformationEquationInstruction(
          14, SpvOpSNegate, {7}, MakeInstructionDescriptor(13, SpvOpLoad, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: id 100 does not exist
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpSNegate, {100},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: id 20 is an OpUndef
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpSNegate, {20},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: id 30 is not available right before its definition
  ASSERT_FALSE(TransformationEquationInstruction(
                   14, SpvOpSNegate, {30},
                   MakeInstructionDescriptor(30, SpvOpCopyObject, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: too many arguments to OpSNegate.
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpSNegate, {7, 7},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: 40 is a type id.
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpSNegate, {40},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: wrong type of argument to OpSNegate.
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpSNegate, {41},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  auto transformation1 = TransformationEquationInstruction(
      14, SpvOpSNegate, {7}, return_instruction);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(14));
  ASSERT_EQ(nullptr, context->get_instr_block(14));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_EQ(SpvOpSNegate, context->get_def_use_mgr()->GetDef(14)->opcode());
  ASSERT_EQ(13, context->get_instr_block(14)->id());
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation2 = TransformationEquationInstruction(
      15, SpvOpSNegate, {14}, return_instruction);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(15, {}), MakeDataDescriptor(7, {})));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 24
         %40 = OpTypeBool
         %41 = OpConstantTrue %40
         %20 = OpUndef %6
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %30 = OpCopyObject %6 %7
         %14 = OpSNegate %6 %7
         %15 = OpSNegate %6 %14
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationEquationInstructionTest, LogicalNot) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %20 = OpTypeInt 32 0
         %21 = OpConstant %20 5
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  protobufs::InstructionDescriptor return_instruction =
      MakeInstructionDescriptor(13, SpvOpReturn, 0);

  // Bad: too few arguments to OpLogicalNot.
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpLogicalNot, {},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: 6 is a type id.
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpLogicalNot, {6},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: wrong type of argument to OpLogicalNot.
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpLogicalNot, {21},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  auto transformation1 = TransformationEquationInstruction(
      14, SpvOpLogicalNot, {7}, return_instruction);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation2 = TransformationEquationInstruction(
      15, SpvOpLogicalNot, {14}, return_instruction);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(15, {}), MakeDataDescriptor(7, {})));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %20 = OpTypeInt 32 0
         %21 = OpConstant %20 5
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %14 = OpLogicalNot %6 %7
         %15 = OpLogicalNot %6 %14
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationEquationInstructionTest, AddSubNegate1) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %30 = OpTypeVector %6 3
         %15 = OpConstant %6 24
         %16 = OpConstant %6 37
         %31 = OpConstantComposite %30 %15 %16 %15
         %33 = OpTypeBool
         %32 = OpConstantTrue %33
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  protobufs::InstructionDescriptor return_instruction =
      MakeInstructionDescriptor(13, SpvOpReturn, 0);

  // Bad: too many arguments to OpIAdd.
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpIAdd, {15, 16, 16},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));
  // Bad: boolean argument to OpIAdd.
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpIAdd, {15, 32},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));
  // Bad: type as argument to OpIAdd.
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpIAdd, {33, 16},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));
  // Bad: arguments of mismatched widths
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpIAdd, {15, 31},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));
  // Bad: arguments of mismatched widths
  ASSERT_FALSE(TransformationEquationInstruction(14, SpvOpIAdd, {31, 15},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  auto transformation1 = TransformationEquationInstruction(
      14, SpvOpIAdd, {15, 16}, return_instruction);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation2 = TransformationEquationInstruction(
      19, SpvOpISub, {14, 16}, return_instruction);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(15, {}), MakeDataDescriptor(19, {})));

  auto transformation3 = TransformationEquationInstruction(
      20, SpvOpISub, {14, 15}, return_instruction);
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(20, {}), MakeDataDescriptor(16, {})));

  auto transformation4 = TransformationEquationInstruction(
      22, SpvOpISub, {16, 14}, return_instruction);
  ASSERT_TRUE(
      transformation4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation4, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation5 = TransformationEquationInstruction(
      24, SpvOpSNegate, {22}, return_instruction);
  ASSERT_TRUE(
      transformation5.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation5, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(24, {}), MakeDataDescriptor(15, {})));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %30 = OpTypeVector %6 3
         %15 = OpConstant %6 24
         %16 = OpConstant %6 37
         %31 = OpConstantComposite %30 %15 %16 %15
         %33 = OpTypeBool
         %32 = OpConstantTrue %33
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %14 = OpIAdd %6 %15 %16
         %19 = OpISub %6 %14 %16 ; ==> synonymous(%19, %15)
         %20 = OpISub %6 %14 %15 ; ==> synonymous(%20, %16)
         %22 = OpISub %6 %16 %14
         %24 = OpSNegate %6 %22 ; ==> synonymous(%24, %15)
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationEquationInstructionTest, AddSubNegate2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %15 = OpConstant %6 24
         %16 = OpConstant %6 37
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  protobufs::InstructionDescriptor return_instruction =
      MakeInstructionDescriptor(13, SpvOpReturn, 0);

  auto transformation1 = TransformationEquationInstruction(
      14, SpvOpISub, {15, 16}, return_instruction);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation2 = TransformationEquationInstruction(
      17, SpvOpIAdd, {14, 16}, return_instruction);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(17, {}), MakeDataDescriptor(15, {})));

  auto transformation3 = TransformationEquationInstruction(
      18, SpvOpIAdd, {16, 14}, return_instruction);
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(17, {}), MakeDataDescriptor(18, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(18, {}), MakeDataDescriptor(15, {})));

  auto transformation4 = TransformationEquationInstruction(
      19, SpvOpISub, {14, 15}, return_instruction);
  ASSERT_TRUE(
      transformation4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation4, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation5 = TransformationEquationInstruction(
      20, SpvOpSNegate, {19}, return_instruction);
  ASSERT_TRUE(
      transformation5.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation5, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(20, {}), MakeDataDescriptor(16, {})));

  auto transformation6 = TransformationEquationInstruction(
      21, SpvOpISub, {14, 19}, return_instruction);
  ASSERT_TRUE(
      transformation6.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation6, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(21, {}), MakeDataDescriptor(15, {})));

  auto transformation7 = TransformationEquationInstruction(
      22, SpvOpISub, {14, 18}, return_instruction);
  ASSERT_TRUE(
      transformation7.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation7, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation8 = TransformationEquationInstruction(
      23, SpvOpSNegate, {22}, return_instruction);
  ASSERT_TRUE(
      transformation8.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation8, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(23, {}), MakeDataDescriptor(16, {})));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %15 = OpConstant %6 24
         %16 = OpConstant %6 37
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %14 = OpISub %6 %15 %16
         %17 = OpIAdd %6 %14 %16 ; ==> synonymous(%17, %15)
         %18 = OpIAdd %6 %16 %14 ; ==> synonymous(%17, %18, %15)
         %19 = OpISub %6 %14 %15
         %20 = OpSNegate %6 %19 ; ==> synonymous(%20, %16)
         %21 = OpISub %6 %14 %19 ; ==> synonymous(%21, %15)
         %22 = OpISub %6 %14 %18
         %23 = OpSNegate %6 %22 ; ==> synonymous(%23, %16)
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationEquationInstructionTest, Bitcast) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %6 2
         %10 = OpTypeVector %7 2
         %11 = OpTypeVector %8 2
         %21 = OpTypeBool
         %22 = OpTypeVector %21 2
         %15 = OpConstant %6 24
         %16 = OpConstant %7 24
         %17 = OpConstant %8 24
         %18 = OpConstantComposite %9 %15 %15
         %19 = OpConstantComposite %10 %16 %16
         %20 = OpConstantComposite %11 %17 %17
         %23 = OpConstantTrue %21
         %24 = OpConstantComposite %22 %23 %23
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  auto insert_before = MakeInstructionDescriptor(13, SpvOpReturn, 0);

  // Too many operands.
  ASSERT_FALSE(TransformationEquationInstruction(50, SpvOpBitcast, {15, 16},
                                                 insert_before)
                   .IsApplicable(context.get(), transformation_context));

  // Too few operands.
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {}, insert_before)
          .IsApplicable(context.get(), transformation_context));

  // Operand's id is invalid.
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {50}, insert_before)
          .IsApplicable(context.get(), transformation_context));

  // Operand's type is invalid
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {13}, insert_before)
          .IsApplicable(context.get(), transformation_context));

  // Operand must be a scalar or a vector of numerical type.
#ifndef NDEBUG
  ASSERT_DEATH(
      TransformationEquationInstruction(50, SpvOpBitcast, {23}, insert_before)
          .IsApplicable(context.get(), transformation_context),
      "Operand is not a scalar or a vector of numerical type");
  ASSERT_DEATH(
      TransformationEquationInstruction(50, SpvOpBitcast, {24}, insert_before)
          .IsApplicable(context.get(), transformation_context),
      "Only vectors of numerical components are supported");
#else
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {23}, insert_before)
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {24}, insert_before)
          .IsApplicable(context.get(), transformation_context));
#endif

  for (uint32_t operand_id = 15, fresh_id = 50; operand_id <= 20;
       ++operand_id, ++fresh_id) {
    TransformationEquationInstruction transformation(
        fresh_id, SpvOpBitcast, {operand_id}, insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %6 2
         %10 = OpTypeVector %7 2
         %11 = OpTypeVector %8 2
         %21 = OpTypeBool
         %22 = OpTypeVector %21 2
         %15 = OpConstant %6 24
         %16 = OpConstant %7 24
         %17 = OpConstant %8 24
         %18 = OpConstantComposite %9 %15 %15
         %19 = OpConstantComposite %10 %16 %16
         %20 = OpConstantComposite %11 %17 %17
         %23 = OpConstantTrue %21
         %24 = OpConstantComposite %22 %23 %23
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %50 = OpBitcast %8 %15
         %51 = OpBitcast %8 %16
         %52 = OpBitcast %6 %17
         %53 = OpBitcast %11 %18
         %54 = OpBitcast %11 %19
         %55 = OpBitcast %9 %20
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationEquationInstructionTest,
     BitcastResultTypeFloatDoesNotExist) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %9 = OpTypeVector %6 2
         %10 = OpTypeVector %7 2
         %15 = OpConstant %6 24
         %16 = OpConstant %7 24
         %18 = OpConstantComposite %9 %15 %15
         %19 = OpConstantComposite %10 %16 %16
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  auto insert_before = MakeInstructionDescriptor(13, SpvOpReturn, 0);

  // Scalar floating-point type does not exist.
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {15}, insert_before)
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {16}, insert_before)
          .IsApplicable(context.get(), transformation_context));

  // Vector of floating-point components does not exist.
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {18}, insert_before)
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {19}, insert_before)
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationEquationInstructionTest, BitcastResultTypeIntDoesNotExist1) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeFloat 32
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  auto insert_before = MakeInstructionDescriptor(13, SpvOpReturn, 0);

  // Scalar integral type does not exist.
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {17}, insert_before)
          .IsApplicable(context.get(), transformation_context));

  // Vector of integral components does not exist.
  ASSERT_FALSE(
      TransformationEquationInstruction(50, SpvOpBitcast, {20}, insert_before)
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationEquationInstructionTest, BitcastResultTypeIntDoesNotExist2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %4 2
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  auto insert_before = MakeInstructionDescriptor(13, SpvOpReturn, 0);

  {
    TransformationEquationInstruction transformation(50, SpvOpBitcast, {17},
                                                     insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(51, SpvOpBitcast, {20},
                                                     insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %4 2
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %50 = OpBitcast %4 %17
         %51 = OpBitcast %9 %20
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationEquationInstructionTest, BitcastResultTypeIntDoesNotExist3) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 1
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %4 2
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  auto insert_before = MakeInstructionDescriptor(13, SpvOpReturn, 0);

  {
    TransformationEquationInstruction transformation(50, SpvOpBitcast, {17},
                                                     insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(51, SpvOpBitcast, {20},
                                                     insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 1
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %4 2
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %50 = OpBitcast %4 %17
         %51 = OpBitcast %9 %20
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationEquationInstructionTest, BitcastResultTypeIntDoesNotExist4) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 1
          %8 = OpTypeFloat 32
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  auto insert_before = MakeInstructionDescriptor(13, SpvOpReturn, 0);

  {
    TransformationEquationInstruction transformation(50, SpvOpBitcast, {17},
                                                     insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  ASSERT_FALSE(
      TransformationEquationInstruction(51, SpvOpBitcast, {20}, insert_before)
          .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 1
          %8 = OpTypeFloat 32
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %50 = OpBitcast %4 %17
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationEquationInstructionTest, BitcastResultTypeIntDoesNotExist5) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  auto insert_before = MakeInstructionDescriptor(13, SpvOpReturn, 0);

  {
    TransformationEquationInstruction transformation(50, SpvOpBitcast, {17},
                                                     insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  ASSERT_FALSE(
      TransformationEquationInstruction(51, SpvOpBitcast, {20}, insert_before)
          .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %50 = OpBitcast %4 %17
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationEquationInstructionTest, BitcastResultTypeIntDoesNotExist6) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 1
          %5 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %5 2
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  auto insert_before = MakeInstructionDescriptor(13, SpvOpReturn, 0);

  {
    TransformationEquationInstruction transformation(50, SpvOpBitcast, {17},
                                                     insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(51, SpvOpBitcast, {20},
                                                     insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 1
          %5 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %5 2
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %50 = OpBitcast %4 %17
         %51 = OpBitcast %9 %20
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationEquationInstructionTest, BitcastResultTypeIntDoesNotExist7) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 1
          %5 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %4 2
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  auto insert_before = MakeInstructionDescriptor(13, SpvOpReturn, 0);

  {
    TransformationEquationInstruction transformation(50, SpvOpBitcast, {17},
                                                     insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(51, SpvOpBitcast, {20},
                                                     insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 1
          %5 = OpTypeInt 32 0
          %8 = OpTypeFloat 32
          %9 = OpTypeVector %4 2
         %11 = OpTypeVector %8 2
         %17 = OpConstant %8 24
         %20 = OpConstantComposite %11 %17 %17
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %50 = OpBitcast %4 %17
         %51 = OpBitcast %9 %20
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationEquationInstructionTest, Miscellaneous1) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
        %113 = OpConstant %6 24
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  protobufs::InstructionDescriptor return_instruction =
      MakeInstructionDescriptor(13, SpvOpReturn, 0);

  auto transformation1 = TransformationEquationInstruction(
      522, SpvOpISub, {113, 113}, return_instruction);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation2 = TransformationEquationInstruction(
      570, SpvOpIAdd, {522, 113}, return_instruction);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
        %113 = OpConstant %6 24
         %12 = OpFunction %2 None %3
         %13 = OpLabel
        %522 = OpISub %6 %113 %113
        %570 = OpIAdd %6 %522 %113
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(570, {}), MakeDataDescriptor(113, {})));

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationEquationInstructionTest, Miscellaneous2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
        %113 = OpConstant %6 24
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  protobufs::InstructionDescriptor return_instruction =
      MakeInstructionDescriptor(13, SpvOpReturn, 0);

  auto transformation1 = TransformationEquationInstruction(
      522, SpvOpISub, {113, 113}, return_instruction);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation2 = TransformationEquationInstruction(
      570, SpvOpIAdd, {522, 113}, return_instruction);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
        %113 = OpConstant %6 24
         %12 = OpFunction %2 None %3
         %13 = OpLabel
        %522 = OpISub %6 %113 %113
        %570 = OpIAdd %6 %522 %113
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(570, {}), MakeDataDescriptor(113, {})));

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationEquationInstructionTest, ConversionInstructions) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %4 = OpTypeInt 32 0
          %5 = OpTypeFloat 32
          %7 = OpTypeVector %6 3
          %8 = OpTypeVector %4 3
          %9 = OpTypeVector %5 3
         %10 = OpConstant %6 12
         %20 = OpConstant %6 12
         %11 = OpConstant %4 12
         %21 = OpConstant %4 12
         %14 = OpConstant %5 12
         %15 = OpConstantComposite %7 %10 %10 %10
         %18 = OpConstantComposite %7 %10 %10 %10
         %16 = OpConstantComposite %8 %11 %11 %11
         %19 = OpConstantComposite %8 %11 %11 %11
         %17 = OpConstantComposite %9 %14 %14 %14
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  protobufs::InstructionDescriptor return_instruction =
      MakeInstructionDescriptor(13, SpvOpReturn, 0);

  // Too few instruction operands.
  ASSERT_FALSE(TransformationEquationInstruction(50, SpvOpConvertSToF, {},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // Too many instruction operands.
  ASSERT_FALSE(TransformationEquationInstruction(50, SpvOpConvertSToF, {15, 16},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // Operand has no type id.
  ASSERT_FALSE(TransformationEquationInstruction(50, SpvOpConvertSToF, {7},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // OpConvertSToF and OpConvertUToF require an operand to have scalar or vector
  // of integral components type.
  ASSERT_FALSE(TransformationEquationInstruction(50, SpvOpConvertSToF, {17},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationEquationInstruction(50, SpvOpConvertSToF, {14},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationEquationInstruction(50, SpvOpConvertUToF, {17},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationEquationInstruction(50, SpvOpConvertUToF, {14},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  {
    TransformationEquationInstruction transformation(50, SpvOpConvertSToF, {15},
                                                     return_instruction);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(51, SpvOpConvertSToF, {10},
                                                     return_instruction);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(52, SpvOpConvertUToF, {16},
                                                     return_instruction);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(53, SpvOpConvertUToF, {11},
                                                     return_instruction);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(58, SpvOpConvertSToF, {18},
                                                     return_instruction);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(59, SpvOpConvertUToF, {19},
                                                     return_instruction);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(60, SpvOpConvertSToF, {20},
                                                     return_instruction);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationEquationInstruction transformation(61, SpvOpConvertUToF, {21},
                                                     return_instruction);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %4 = OpTypeInt 32 0
          %5 = OpTypeFloat 32
          %7 = OpTypeVector %6 3
          %8 = OpTypeVector %4 3
          %9 = OpTypeVector %5 3
         %10 = OpConstant %6 12
         %20 = OpConstant %6 12
         %11 = OpConstant %4 12
         %21 = OpConstant %4 12
         %14 = OpConstant %5 12
         %15 = OpConstantComposite %7 %10 %10 %10
         %18 = OpConstantComposite %7 %10 %10 %10
         %16 = OpConstantComposite %8 %11 %11 %11
         %19 = OpConstantComposite %8 %11 %11 %11
         %17 = OpConstantComposite %9 %14 %14 %14
         %12 = OpFunction %2 None %3
         %13 = OpLabel
         %50 = OpConvertSToF %9 %15
         %51 = OpConvertSToF %5 %10
         %52 = OpConvertUToF %9 %16
         %53 = OpConvertUToF %5 %11
         %58 = OpConvertSToF %9 %18
         %59 = OpConvertUToF %9 %19
         %60 = OpConvertSToF %5 %20
         %61 = OpConvertUToF %5 %21
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationEquationInstructionTest, FloatResultTypeDoesNotExist) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %6 3
          %9 = OpTypeVector %7 3
         %10 = OpConstant %6 24
         %11 = OpConstant %7 25
         %14 = OpConstantComposite %8 %10 %10 %10
         %15 = OpConstantComposite %9 %11 %11 %11
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  protobufs::InstructionDescriptor return_instruction =
      MakeInstructionDescriptor(13, SpvOpReturn, 0);

  // Scalar float type doesn't exist.
  ASSERT_FALSE(TransformationEquationInstruction(16, SpvOpConvertUToF, {10},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationEquationInstruction(16, SpvOpConvertSToF, {11},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));

  // Vector float type doesn't exist.
  ASSERT_FALSE(TransformationEquationInstruction(16, SpvOpConvertUToF, {14},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationEquationInstruction(16, SpvOpConvertSToF, {15},
                                                 return_instruction)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationEquationInstructionTest, HandlesIrrelevantIds) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %30 = OpTypeVector %6 3
         %15 = OpConstant %6 24
         %16 = OpConstant %6 37
         %31 = OpConstantComposite %30 %15 %16 %15
         %33 = OpTypeBool
         %32 = OpConstantTrue %33
         %12 = OpFunction %2 None %3
         %13 = OpLabel
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
  auto return_instruction = MakeInstructionDescriptor(13, SpvOpReturn, 0);

  // Applicable.
  TransformationEquationInstruction transformation(14, SpvOpIAdd, {15, 16},
                                                   return_instruction);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Handles irrelevant ids.
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(16);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(15);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationEquationInstructionTest, HandlesDeadBlock) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %30 = OpTypeVector %6 3
         %15 = OpConstant %6 24
         %16 = OpConstant %6 37
         %31 = OpConstantComposite %30 %15 %16 %15
         %33 = OpTypeBool
         %32 = OpConstantTrue %33
         %12 = OpFunction %2 None %3
         %13 = OpLabel
               OpSelectionMerge %40 None
               OpBranchConditional %32 %40 %41
         %41 = OpLabel
               OpBranch %40
         %40 = OpLabel
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

  transformation_context.GetFactManager()->AddFactBlockIsDead(41);

  TransformationEquationInstruction transformation1(
      14, SpvOpIAdd, {15, 16},
      MakeInstructionDescriptor(13, SpvOpSelectionMerge, 0));
  // No synonym is created since block is dead.
  TransformationEquationInstruction transformation2(
      100, SpvOpISub, {14, 16}, MakeInstructionDescriptor(41, SpvOpBranch, 0));
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {}), MakeDataDescriptor(15, {})));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
