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

#include "source/fuzz/transformation_add_constant_scalar.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddConstantScalarTest, IsApplicable) {
  std::string reference_shader = R"(
               OpCapability Shader
               OpCapability Int64
               OpCapability Float64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %17 "main"

; Types

  ; 32-bit types
          %2 = OpTypeInt 32 0
          %3 = OpTypeInt 32 1
          %4 = OpTypeFloat 32

  ; 64-bit types
          %5 = OpTypeInt 64 0
          %6 = OpTypeInt 64 1
          %7 = OpTypeFloat 64

          %8 = OpTypePointer Private %2
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9

; Constants

  ; 32-bit constants
         %11 = OpConstant %2 1
         %12 = OpConstant %3 2
         %13 = OpConstant %4 3

  ; 64-bit constants
         %14 = OpConstant %5 1
         %15 = OpConstant %6 2
         %16 = OpConstant %7 3

; main function
         %17 = OpFunction %9 None %10
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Tests |fresh_id| being non-fresh.
  auto transformation = TransformationAddConstantScalar(18, 2, {0}, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests undefined |type_id|.
  transformation = TransformationAddConstantScalar(19, 20, {0}, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests |type_id| not representing a type instruction.
  transformation = TransformationAddConstantScalar(19, 11, {0}, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests |type_id| representing an OpTypePointer instruction.
  transformation = TransformationAddConstantScalar(19, 8, {0}, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests |type_id| representing an OpTypeVoid instruction.
  transformation = TransformationAddConstantScalar(19, 9, {0}, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests |words| having no words.
  transformation = TransformationAddConstantScalar(19, 2, {}, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests |words| having 2 words for a 32-bit type.
  transformation = TransformationAddConstantScalar(19, 2, {0, 1}, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests |words| having 3 words for a 64-bit type.
  transformation = TransformationAddConstantScalar(19, 5, {0, 1, 2}, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests |words| having 2 words for a 32-bit float type.
  transformation = TransformationAddConstantScalar(19, 4, {0, 1}, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddConstantScalarTest, Apply) {
  std::string reference_shader = R"(
               OpCapability Shader
               OpCapability Int64
               OpCapability Float64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %17 "main"

; Types

  ; 32-bit types
          %2 = OpTypeInt 32 0
          %3 = OpTypeInt 32 1
          %4 = OpTypeFloat 32

  ; 64-bit types
          %5 = OpTypeInt 64 0
          %6 = OpTypeInt 64 1
          %7 = OpTypeFloat 64

          %8 = OpTypePointer Private %2
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9

; Constants

  ; 32-bit constants
         %11 = OpConstant %2 1
         %12 = OpConstant %3 2
         %13 = OpConstant %4 3

  ; 64-bit constants
         %14 = OpConstant %5 1
         %15 = OpConstant %6 2
         %16 = OpConstant %7 3

; main function
         %17 = OpFunction %9 None %10
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Adds 32-bit unsigned integer (1 logical operand with 1 word).
  auto transformation = TransformationAddConstantScalar(19, 2, {4}, false);
  ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(19));
  ASSERT_EQ(nullptr, context->get_constant_mgr()->FindDeclaredConstant(19));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_EQ(spv::Op::OpConstant,
            context->get_def_use_mgr()->GetDef(19)->opcode());
  ASSERT_EQ(4, context->get_constant_mgr()->FindDeclaredConstant(19)->GetU32());
  auto* constant_instruction = context->get_def_use_mgr()->GetDef(19);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds 32-bit signed integer (1 logical operand with 1 word).
  transformation = TransformationAddConstantScalar(20, 3, {5}, false);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(20);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds 32-bit float (1 logical operand with 1 word).
  transformation = TransformationAddConstantScalar(
      21, 4, {0b01000000110000000000000000000000}, false);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(21);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds 64-bit unsigned integer (1 logical operand with 2 words).
  transformation = TransformationAddConstantScalar(22, 5, {7, 0}, false);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(22);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds 64-bit signed integer (1 logical operand with 2 words).
  transformation = TransformationAddConstantScalar(23, 6, {8, 0}, false);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(23);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds 64-bit float (1 logical operand with 2 words).
  transformation = TransformationAddConstantScalar(
      24, 7, {0, 0b01000000001000100000000000000000}, false);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(24);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds irrelevant 32-bit unsigned integer (1 logical operand with 1 word).
  transformation = TransformationAddConstantScalar(25, 2, {10}, true);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(25);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds irrelevant 32-bit signed integer (1 logical operand with 1 word).
  transformation = TransformationAddConstantScalar(26, 3, {11}, true);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(26);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds irrelevant 32-bit float (1 logical operand with 1 word).
  transformation = TransformationAddConstantScalar(
      27, 4, {0b01000001010000000000000000000000}, true);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(27);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds irrelevant 64-bit unsigned integer (1 logical operand with 2 words).
  transformation = TransformationAddConstantScalar(28, 5, {13, 0}, true);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(28);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds irrelevant 64-bit signed integer (1 logical operand with 2 words).
  transformation = TransformationAddConstantScalar(29, 6, {14, 0}, true);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(29);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds irrelevant 64-bit float (1 logical operand with 2 words).
  transformation = TransformationAddConstantScalar(
      30, 7, {0, 0b01000000001011100000000000000000}, true);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  constant_instruction = context->get_def_use_mgr()->GetDef(30);
  EXPECT_EQ(constant_instruction->NumInOperands(), 1);
  EXPECT_EQ(constant_instruction->NumInOperandWords(), 2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  for (uint32_t result_id = 19; result_id <= 24; ++result_id) {
    ASSERT_FALSE(
        transformation_context.GetFactManager()->IdIsIrrelevant(result_id));
  }

  for (uint32_t result_id = 25; result_id <= 30; ++result_id) {
    ASSERT_TRUE(
        transformation_context.GetFactManager()->IdIsIrrelevant(result_id));
  }

  std::string variant_shader = R"(
               OpCapability Shader
               OpCapability Int64
               OpCapability Float64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %17 "main"

; Types

  ; 32-bit types
          %2 = OpTypeInt 32 0
          %3 = OpTypeInt 32 1
          %4 = OpTypeFloat 32

  ; 64-bit types
          %5 = OpTypeInt 64 0
          %6 = OpTypeInt 64 1
          %7 = OpTypeFloat 64

          %8 = OpTypePointer Private %2
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9

; Constants

  ; 32-bit constants
         %11 = OpConstant %2 1
         %12 = OpConstant %3 2
         %13 = OpConstant %4 3

  ; 64-bit constants
         %14 = OpConstant %5 1
         %15 = OpConstant %6 2
         %16 = OpConstant %7 3

  ; added constants
         %19 = OpConstant %2 4
         %20 = OpConstant %3 5
         %21 = OpConstant %4 6
         %22 = OpConstant %5 7
         %23 = OpConstant %6 8
         %24 = OpConstant %7 9
         %25 = OpConstant %2 10
         %26 = OpConstant %3 11
         %27 = OpConstant %4 12
         %28 = OpConstant %5 13
         %29 = OpConstant %6 14
         %30 = OpConstant %7 15

; main function
         %17 = OpFunction %9 None %10
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
