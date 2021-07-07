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

#include "source/fuzz/transformation_permute_phi_operands.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationPermutePhiOperandsTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %11 = OpConstant %6 1
         %14 = OpTypeBool
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %10 %11
         %12 = OpLoad %6 %8
         %13 = OpLoad %6 %10
         %15 = OpSLessThan %14 %12 %13
               OpSelectionMerge %17 None
               OpBranchConditional %15 %16 %21
         %16 = OpLabel
         %18 = OpLoad %6 %10
         %19 = OpLoad %6 %8
         %20 = OpIAdd %6 %19 %18
               OpBranch %17
         %21 = OpLabel
         %22 = OpLoad %6 %10
         %23 = OpLoad %6 %8
         %24 = OpISub %6 %23 %22
               OpBranch %17
         %17 = OpLabel
         %25 = OpPhi %6 %20 %16 %24 %21
         %30 = OpIAdd %6 %25 %25
               OpStore %8 %25
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
  // Result id is invalid.
  ASSERT_FALSE(TransformationPermutePhiOperands(26, {}).IsApplicable(
      context.get(), transformation_context));

  // Result id is not of an OpPhi instruction.
  ASSERT_FALSE(TransformationPermutePhiOperands(24, {}).IsApplicable(
      context.get(), transformation_context));

  // Result id is not of an OpPhi instruction.
  ASSERT_FALSE(TransformationPermutePhiOperands(24, {}).IsApplicable(
      context.get(), transformation_context));

  // Permutation has invalid size.
  ASSERT_FALSE(TransformationPermutePhiOperands(25, {0, 1, 2})
                   .IsApplicable(context.get(), transformation_context));

#ifndef NDEBUG
  // Permutation has duplicates.
  ASSERT_DEATH(TransformationPermutePhiOperands(25, {0, 0})
                   .IsApplicable(context.get(), transformation_context),
               "Permutation has duplicates");
#endif

  // Permutation's values are not in range.
  ASSERT_FALSE(TransformationPermutePhiOperands(25, {1, 2})
                   .IsApplicable(context.get(), transformation_context));

  TransformationPermutePhiOperands transformation(25, {1, 0});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  // Check that the def-use manager knows that the phi instruction's ids have
  // been permuted.
  std::vector<std::pair<uint32_t, uint32_t>> phi_operand_to_new_operand_index =
      {{20, 4}, {16, 5}, {24, 2}, {21, 3}};
  for (std::pair<uint32_t, uint32_t>& entry :
       phi_operand_to_new_operand_index) {
    context->get_def_use_mgr()->WhileEachUse(
        entry.first,
        [&entry](opt::Instruction* inst, uint32_t operand_index) -> bool {
          if (inst->result_id() == 25) {
            EXPECT_EQ(entry.second, operand_index);
            return false;
          }
          return true;
        });
  }
  bool found_use_in_store = false;
  bool found_use_in_add_lhs = false;
  bool found_use_in_add_rhs = false;
  context->get_def_use_mgr()->ForEachUse(
      25, [&found_use_in_store, &found_use_in_add_lhs, &found_use_in_add_rhs](
              opt::Instruction* inst, uint32_t operand_index) {
        if (inst->opcode() == SpvOpStore) {
          ASSERT_FALSE(found_use_in_store);
          found_use_in_store = true;
        } else {
          ASSERT_EQ(SpvOpIAdd, inst->opcode());
          if (operand_index == 2) {
            ASSERT_FALSE(found_use_in_add_lhs);
            found_use_in_add_lhs = true;
          } else {
            ASSERT_EQ(3, operand_index);
            ASSERT_FALSE(found_use_in_add_rhs);
            found_use_in_add_rhs = true;
          }
        }
      });
  ASSERT_TRUE(found_use_in_store);
  ASSERT_TRUE(found_use_in_add_lhs);
  ASSERT_TRUE(found_use_in_add_rhs);

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %11 = OpConstant %6 1
         %14 = OpTypeBool
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %10 %11
         %12 = OpLoad %6 %8
         %13 = OpLoad %6 %10
         %15 = OpSLessThan %14 %12 %13
               OpSelectionMerge %17 None
               OpBranchConditional %15 %16 %21
         %16 = OpLabel
         %18 = OpLoad %6 %10
         %19 = OpLoad %6 %8
         %20 = OpIAdd %6 %19 %18
               OpBranch %17
         %21 = OpLabel
         %22 = OpLoad %6 %10
         %23 = OpLoad %6 %8
         %24 = OpISub %6 %23 %22
               OpBranch %17
         %17 = OpLabel
         %25 = OpPhi %6 %24 %21 %20 %16
         %30 = OpIAdd %6 %25 %25
               OpStore %8 %25
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
