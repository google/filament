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

#include "source/fuzz/transformation_replace_boolean_constant_with_constant_binary.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/id_use_descriptor.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationReplaceBooleanConstantWithConstantBinaryTest,
     BasicReplacements) {
  // The test came from the following pseudo-GLSL, where int64 and uint64 denote
  // 64-bit integer types (they were replaced with int and uint during
  // translation to SPIR-V, and the generated SPIR-V has been doctored to
  // accommodate them).
  //
  // #version 450
  //
  // void main() {
  //   double d1, d2;
  //   d1 = 1.0;
  //   d2 = 2.0;
  //   float f1, f2;
  //   f1 = 4.0;
  //   f2 = 8.0;
  //   int i1, i2;
  //   i1 = 100;
  //   i2 = 200;
  //
  //   uint u1, u2;
  //   u1 = 300u;
  //   u2 = 400u;
  //
  //   int64 i64_1, i64_2;
  //   i64_1 = 500;
  //   i64_2 = 600;
  //
  //   uint64 u64_1, u64_2;
  //   u64_1 = 700u;
  //   u64_2 = 800u;
  //
  //   bool b, c, d, e;
  //   b = true;
  //   c = false;
  //   d = true || c;
  //   c = c && false;
  // }
  std::string shader = R"(
               OpCapability Shader
               OpCapability Float64
               OpCapability Int64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
               OpName %8 "d1"
               OpName %10 "d2"
               OpName %14 "f1"
               OpName %16 "f2"
               OpName %20 "i1"
               OpName %22 "i2"
               OpName %26 "u1"
               OpName %28 "u2"
               OpName %30 "i64_1"
               OpName %32 "i64_2"
               OpName %34 "u64_1"
               OpName %36 "u64_2"
               OpName %40 "b"
               OpName %42 "c"
               OpName %44 "d"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 64
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpTypeFloat 32
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 4
         %17 = OpConstant %12 8
         %18 = OpTypeInt 32 1
         %60 = OpTypeInt 64 1
         %61 = OpTypePointer Function %60
         %19 = OpTypePointer Function %18
         %21 = OpConstant %18 -100
         %23 = OpConstant %18 200
         %24 = OpTypeInt 32 0
         %62 = OpTypeInt 64 0
         %63 = OpTypePointer Function %62
         %25 = OpTypePointer Function %24
         %27 = OpConstant %24 300
         %29 = OpConstant %24 400
         %31 = OpConstant %60 -600
         %33 = OpConstant %60 -500
         %35 = OpConstant %62 700
         %37 = OpConstant %62 800
         %38 = OpTypeBool
         %39 = OpTypePointer Function %38
         %41 = OpConstantTrue %38
         %43 = OpConstantFalse %38
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %16 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %22 = OpVariable %19 Function
         %26 = OpVariable %25 Function
         %28 = OpVariable %25 Function
         %30 = OpVariable %61 Function
         %32 = OpVariable %61 Function
         %34 = OpVariable %63 Function
         %36 = OpVariable %63 Function
         %40 = OpVariable %39 Function
         %42 = OpVariable %39 Function
         %44 = OpVariable %39 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %14 %15
               OpStore %16 %17
               OpStore %20 %21
               OpStore %22 %23
               OpStore %26 %27
               OpStore %28 %29
               OpStore %30 %31
               OpStore %32 %33
               OpStore %34 %35
               OpStore %36 %37
               OpStore %40 %41
               OpStore %42 %43
         %45 = OpLoad %38 %42
         %46 = OpLogicalOr %38 %41 %45
               OpStore %44 %46
         %47 = OpLoad %38 %42
         %48 = OpLogicalAnd %38 %47 %43
               OpStore %42 %48
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
  std::vector<protobufs::IdUseDescriptor> uses_of_true = {
      MakeIdUseDescriptor(41, MakeInstructionDescriptor(44, SpvOpStore, 12), 1),
      MakeIdUseDescriptor(41, MakeInstructionDescriptor(46, SpvOpLogicalOr, 0),
                          0)};

  std::vector<protobufs::IdUseDescriptor> uses_of_false = {
      MakeIdUseDescriptor(43, MakeInstructionDescriptor(44, SpvOpStore, 13), 1),
      MakeIdUseDescriptor(43, MakeInstructionDescriptor(48, SpvOpLogicalAnd, 0),
                          1)};

  const uint32_t fresh_id = 100;

  std::vector<SpvOp> fp_gt_opcodes = {
      SpvOpFOrdGreaterThan, SpvOpFOrdGreaterThanEqual, SpvOpFUnordGreaterThan,
      SpvOpFUnordGreaterThanEqual};

  std::vector<SpvOp> fp_lt_opcodes = {SpvOpFOrdLessThan, SpvOpFOrdLessThanEqual,
                                      SpvOpFUnordLessThan,
                                      SpvOpFUnordLessThanEqual};

  std::vector<SpvOp> int_gt_opcodes = {SpvOpSGreaterThan,
                                       SpvOpSGreaterThanEqual};

  std::vector<SpvOp> int_lt_opcodes = {SpvOpSLessThan, SpvOpSLessThanEqual};

  std::vector<SpvOp> uint_gt_opcodes = {SpvOpUGreaterThan,
                                        SpvOpUGreaterThanEqual};

  std::vector<SpvOp> uint_lt_opcodes = {SpvOpULessThan, SpvOpULessThanEqual};

#define CHECK_OPERATOR(USE_DESCRIPTOR, LHS_ID, RHS_ID, OPCODE, FRESH_ID) \
  ASSERT_TRUE(TransformationReplaceBooleanConstantWithConstantBinary(    \
                  USE_DESCRIPTOR, LHS_ID, RHS_ID, OPCODE, FRESH_ID)      \
                  .IsApplicable(context.get(), transformation_context)); \
  ASSERT_FALSE(TransformationReplaceBooleanConstantWithConstantBinary(   \
                   USE_DESCRIPTOR, RHS_ID, LHS_ID, OPCODE, FRESH_ID)     \
                   .IsApplicable(context.get(), transformation_context));

#define CHECK_TRANSFORMATION_APPLICABILITY(GT_OPCODES, LT_OPCODES, SMALL_ID, \
                                           LARGE_ID)                         \
  for (auto gt_opcode : GT_OPCODES) {                                        \
    for (auto& true_use : uses_of_true) {                                    \
      CHECK_OPERATOR(true_use, LARGE_ID, SMALL_ID, gt_opcode, fresh_id);     \
    }                                                                        \
    for (auto& false_use : uses_of_false) {                                  \
      CHECK_OPERATOR(false_use, SMALL_ID, LARGE_ID, gt_opcode, fresh_id);    \
    }                                                                        \
  }                                                                          \
  for (auto lt_opcode : LT_OPCODES) {                                        \
    for (auto& true_use : uses_of_true) {                                    \
      CHECK_OPERATOR(true_use, SMALL_ID, LARGE_ID, lt_opcode, fresh_id);     \
    }                                                                        \
    for (auto& false_use : uses_of_false) {                                  \
      CHECK_OPERATOR(false_use, LARGE_ID, SMALL_ID, lt_opcode, fresh_id);    \
    }                                                                        \
  }

  // Float
  { CHECK_TRANSFORMATION_APPLICABILITY(fp_gt_opcodes, fp_lt_opcodes, 15, 17); }

  // Double
  { CHECK_TRANSFORMATION_APPLICABILITY(fp_gt_opcodes, fp_lt_opcodes, 9, 11); }

  // Int32
  {
    CHECK_TRANSFORMATION_APPLICABILITY(int_gt_opcodes, int_lt_opcodes, 21, 23);
  }

  // Int64
  {
    CHECK_TRANSFORMATION_APPLICABILITY(int_gt_opcodes, int_lt_opcodes, 31, 33);
  }

  // Uint32
  {
    CHECK_TRANSFORMATION_APPLICABILITY(uint_gt_opcodes, uint_lt_opcodes, 27,
                                       29);
  }

  // Uint64
  {
    CHECK_TRANSFORMATION_APPLICABILITY(uint_gt_opcodes, uint_lt_opcodes, 35,
                                       37);
  }

  // Target id is not fresh
  ASSERT_FALSE(TransformationReplaceBooleanConstantWithConstantBinary(
                   uses_of_true[0], 15, 17, SpvOpFOrdLessThan, 15)
                   .IsApplicable(context.get(), transformation_context));

  // LHS id does not exist
  ASSERT_FALSE(TransformationReplaceBooleanConstantWithConstantBinary(
                   uses_of_true[0], 300, 17, SpvOpFOrdLessThan, 200)
                   .IsApplicable(context.get(), transformation_context));

  // RHS id does not exist
  ASSERT_FALSE(TransformationReplaceBooleanConstantWithConstantBinary(
                   uses_of_true[0], 15, 300, SpvOpFOrdLessThan, 200)
                   .IsApplicable(context.get(), transformation_context));

  // LHS and RHS ids do not match type
  ASSERT_FALSE(TransformationReplaceBooleanConstantWithConstantBinary(
                   uses_of_true[0], 11, 17, SpvOpFOrdLessThan, 200)
                   .IsApplicable(context.get(), transformation_context));

  // Opcode not appropriate
  ASSERT_FALSE(TransformationReplaceBooleanConstantWithConstantBinary(
                   uses_of_true[0], 15, 17, SpvOpFDiv, 200)
                   .IsApplicable(context.get(), transformation_context));

  auto replace_true_with_double_comparison =
      TransformationReplaceBooleanConstantWithConstantBinary(
          uses_of_true[0], 11, 9, SpvOpFUnordGreaterThan, 100);
  auto replace_true_with_uint32_comparison =
      TransformationReplaceBooleanConstantWithConstantBinary(
          uses_of_true[1], 27, 29, SpvOpULessThanEqual, 101);
  auto replace_false_with_float_comparison =
      TransformationReplaceBooleanConstantWithConstantBinary(
          uses_of_false[0], 17, 15, SpvOpFOrdLessThan, 102);
  auto replace_false_with_sint64_comparison =
      TransformationReplaceBooleanConstantWithConstantBinary(
          uses_of_false[1], 33, 31, SpvOpSLessThan, 103);

  ASSERT_TRUE(replace_true_with_double_comparison.IsApplicable(
      context.get(), transformation_context));
  ApplyAndCheckFreshIds(replace_true_with_double_comparison, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(replace_true_with_uint32_comparison.IsApplicable(
      context.get(), transformation_context));
  ApplyAndCheckFreshIds(replace_true_with_uint32_comparison, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(replace_false_with_float_comparison.IsApplicable(
      context.get(), transformation_context));
  ApplyAndCheckFreshIds(replace_false_with_float_comparison, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(replace_false_with_sint64_comparison.IsApplicable(
      context.get(), transformation_context));
  ApplyAndCheckFreshIds(replace_false_with_sint64_comparison, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after = R"(
               OpCapability Shader
               OpCapability Float64
               OpCapability Int64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
               OpName %8 "d1"
               OpName %10 "d2"
               OpName %14 "f1"
               OpName %16 "f2"
               OpName %20 "i1"
               OpName %22 "i2"
               OpName %26 "u1"
               OpName %28 "u2"
               OpName %30 "i64_1"
               OpName %32 "i64_2"
               OpName %34 "u64_1"
               OpName %36 "u64_2"
               OpName %40 "b"
               OpName %42 "c"
               OpName %44 "d"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 64
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpTypeFloat 32
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 4
         %17 = OpConstant %12 8
         %18 = OpTypeInt 32 1
         %60 = OpTypeInt 64 1
         %61 = OpTypePointer Function %60
         %19 = OpTypePointer Function %18
         %21 = OpConstant %18 -100
         %23 = OpConstant %18 200
         %24 = OpTypeInt 32 0
         %62 = OpTypeInt 64 0
         %63 = OpTypePointer Function %62
         %25 = OpTypePointer Function %24
         %27 = OpConstant %24 300
         %29 = OpConstant %24 400
         %31 = OpConstant %60 -600
         %33 = OpConstant %60 -500
         %35 = OpConstant %62 700
         %37 = OpConstant %62 800
         %38 = OpTypeBool
         %39 = OpTypePointer Function %38
         %41 = OpConstantTrue %38
         %43 = OpConstantFalse %38
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %16 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %22 = OpVariable %19 Function
         %26 = OpVariable %25 Function
         %28 = OpVariable %25 Function
         %30 = OpVariable %61 Function
         %32 = OpVariable %61 Function
         %34 = OpVariable %63 Function
         %36 = OpVariable %63 Function
         %40 = OpVariable %39 Function
         %42 = OpVariable %39 Function
         %44 = OpVariable %39 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %14 %15
               OpStore %16 %17
               OpStore %20 %21
               OpStore %22 %23
               OpStore %26 %27
               OpStore %28 %29
               OpStore %30 %31
               OpStore %32 %33
               OpStore %34 %35
               OpStore %36 %37
        %100 = OpFUnordGreaterThan %38 %11 %9
               OpStore %40 %100
        %102 = OpFOrdLessThan %38 %17 %15
               OpStore %42 %102
         %45 = OpLoad %38 %42
        %101 = OpULessThanEqual %38 %27 %29
         %46 = OpLogicalOr %38 %101 %45
               OpStore %44 %46
         %47 = OpLoad %38 %42
        %103 = OpSLessThan %38 %33 %31
         %48 = OpLogicalAnd %38 %47 %103
               OpStore %42 %48
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after, context.get()));

  if (std::numeric_limits<double>::has_quiet_NaN) {
    double quiet_nan_double = std::numeric_limits<double>::quiet_NaN();
    uint32_t words[2];
    memcpy(words, &quiet_nan_double, sizeof(double));
    opt::Instruction::OperandList operands = {
        {SPV_OPERAND_TYPE_LITERAL_INTEGER, {words[0]}},
        {SPV_OPERAND_TYPE_LITERAL_INTEGER, {words[1]}}};
    context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
        context.get(), SpvOpConstant, 6, 200, operands));
    fuzzerutil::UpdateModuleIdBound(context.get(), 200);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    // The transformation is not applicable because %200 is NaN.
    ASSERT_FALSE(TransformationReplaceBooleanConstantWithConstantBinary(
                     uses_of_true[0], 11, 200, SpvOpFOrdLessThan, 300)
                     .IsApplicable(context.get(), transformation_context));
  }
  if (std::numeric_limits<double>::has_infinity) {
    double positive_infinity_double = std::numeric_limits<double>::infinity();
    uint32_t words[2];
    memcpy(words, &positive_infinity_double, sizeof(double));
    opt::Instruction::OperandList operands = {
        {SPV_OPERAND_TYPE_LITERAL_INTEGER, {words[0]}},
        {SPV_OPERAND_TYPE_LITERAL_INTEGER, {words[1]}}};
    context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
        context.get(), SpvOpConstant, 6, 201, operands));
    fuzzerutil::UpdateModuleIdBound(context.get(), 201);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    // Even though the double constant %11 is less than the infinity %201, the
    // transformation is restricted to only apply to finite values.
    ASSERT_FALSE(TransformationReplaceBooleanConstantWithConstantBinary(
                     uses_of_true[0], 11, 201, SpvOpFOrdLessThan, 300)
                     .IsApplicable(context.get(), transformation_context));
  }
  if (std::numeric_limits<float>::has_infinity) {
    float positive_infinity_float = std::numeric_limits<float>::infinity();
    float negative_infinity_float = -1 * positive_infinity_float;
    uint32_t words_positive_infinity[1];
    uint32_t words_negative_infinity[1];
    memcpy(words_positive_infinity, &positive_infinity_float, sizeof(float));
    memcpy(words_negative_infinity, &negative_infinity_float, sizeof(float));
    opt::Instruction::OperandList operands_positive_infinity = {
        {SPV_OPERAND_TYPE_LITERAL_INTEGER, {words_positive_infinity[0]}}};
    context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
        context.get(), SpvOpConstant, 12, 202, operands_positive_infinity));
    fuzzerutil::UpdateModuleIdBound(context.get(), 202);
    opt::Instruction::OperandList operands = {
        {SPV_OPERAND_TYPE_LITERAL_INTEGER, {words_negative_infinity[0]}}};
    context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
        context.get(), SpvOpConstant, 12, 203, operands));
    fuzzerutil::UpdateModuleIdBound(context.get(), 203);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    // Even though the negative infinity at %203 is less than the positive
    // infinity %202, the transformation is restricted to only apply to finite
    // values.
    ASSERT_FALSE(TransformationReplaceBooleanConstantWithConstantBinary(
                     uses_of_true[0], 203, 202, SpvOpFOrdLessThan, 300)
                     .IsApplicable(context.get(), transformation_context));
  }
}

TEST(TransformationReplaceBooleanConstantWithConstantBinaryTest,
     MergeInstructions) {
  // The test came from the following GLSL:
  //
  // void main() {
  //   int x = 1;
  //   int y = 2;
  //   if (true) {
  //     x = 2;
  //   }
  //   while(false) {
  //     y = 2;
  //   }
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "y"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
         %21 = OpConstantFalse %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %10 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %15
         %14 = OpLabel
               OpStore %8 %11
               OpBranch %15
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpLoopMerge %18 %19 None
               OpBranchConditional %21 %17 %18
         %17 = OpLabel
               OpStore %10 %11
               OpBranch %19
         %19 = OpLabel
               OpBranch %16
         %18 = OpLabel
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
  auto use_of_true_in_if = MakeIdUseDescriptor(
      13, MakeInstructionDescriptor(10, SpvOpBranchConditional, 0), 0);
  auto use_of_false_in_while = MakeIdUseDescriptor(
      21, MakeInstructionDescriptor(16, SpvOpBranchConditional, 0), 0);

  auto replacement_1 = TransformationReplaceBooleanConstantWithConstantBinary(
      use_of_true_in_if, 9, 11, SpvOpSLessThan, 100);
  auto replacement_2 = TransformationReplaceBooleanConstantWithConstantBinary(
      use_of_false_in_while, 9, 11, SpvOpSGreaterThanEqual, 101);

  ASSERT_TRUE(
      replacement_1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement_1, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  ASSERT_TRUE(
      replacement_2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(replacement_2, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "y"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
         %21 = OpConstantFalse %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %10 %11
        %100 = OpSLessThan %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %100 %14 %15
         %14 = OpLabel
               OpStore %8 %11
               OpBranch %15
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
        %101 = OpSGreaterThanEqual %12 %9 %11
               OpLoopMerge %18 %19 None
               OpBranchConditional %101 %17 %18
         %17 = OpLabel
               OpStore %10 %11
               OpBranch %19
         %19 = OpLabel
               OpBranch %16
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after, context.get()));
}

TEST(TransformationReplaceBooleanConstantWithConstantBinaryTest, OpPhi) {
  // Hand-written SPIR-V to check applicability of the transformation on an
  // OpPhi argument.

  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %10 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 0
          %5 = OpTypeBool

; Constants
          %6 = OpConstant %4 0
          %7 = OpConstant %4 1
          %8 = OpConstantTrue %5
          %9 = OpConstantFalse %5

; main function
         %10 = OpFunction %2 None %3
         %11 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %8 %12 %13
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %14 = OpPhi %5 %8 %11 %9 %12
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
  auto instruction_descriptor = MakeInstructionDescriptor(14, SpvOpPhi, 0);
  auto id_use_descriptor = MakeIdUseDescriptor(8, instruction_descriptor, 0);
  auto transformation = TransformationReplaceBooleanConstantWithConstantBinary(
      id_use_descriptor, 6, 7, SpvOpULessThan, 15);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %10 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 0
          %5 = OpTypeBool

; Constants
          %6 = OpConstant %4 0
          %7 = OpConstant %4 1
          %8 = OpConstantTrue %5
          %9 = OpConstantFalse %5

; main function
         %10 = OpFunction %2 None %3
         %11 = OpLabel
         %15 = OpULessThan %5 %6 %7
               OpSelectionMerge %13 None
               OpBranchConditional %8 %12 %13
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %14 = OpPhi %5 %15 %11 %9 %12
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

TEST(TransformationReplaceBooleanConstantWithConstantBinaryTest,
     DoNotReplaceVariableInitializer) {
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
          %7 = OpTypePointer Function %6
          %9 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
         %13 = OpConstant %10 0
         %15 = OpConstant %10 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %50 = OpVariable %7 Function %9
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
  ASSERT_FALSE(TransformationReplaceBooleanConstantWithConstantBinary(
                   MakeIdUseDescriptor(
                       9, MakeInstructionDescriptor(50, SpvOpVariable, 0), 1),
                   13, 15, SpvOpSLessThan, 100)
                   .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
