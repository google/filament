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

#include "source/fuzz/transformation_swap_conditional_branch_operands.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationSwapConditionalBranchOperandsTest, BasicTest) {
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
               OpBranchConditional %15 %16 %21 10 20
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
  // Invalid instruction descriptor.
  ASSERT_FALSE(TransformationSwapConditionalBranchOperands(
                   MakeInstructionDescriptor(26, spv::Op::OpPhi, 0), 26)
                   .IsApplicable(context.get(), transformation_context));

  // Descriptor for a wrong instruction.
  ASSERT_FALSE(TransformationSwapConditionalBranchOperands(
                   MakeInstructionDescriptor(25, spv::Op::OpPhi, 0), 26)
                   .IsApplicable(context.get(), transformation_context));

  // Fresh id is not fresh.
  ASSERT_FALSE(
      TransformationSwapConditionalBranchOperands(
          MakeInstructionDescriptor(15, spv::Op::OpBranchConditional, 0), 25)
          .IsApplicable(context.get(), transformation_context));

  TransformationSwapConditionalBranchOperands transformation(
      MakeInstructionDescriptor(15, spv::Op::OpBranchConditional, 0), 26);
  ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(26));
  ASSERT_EQ(nullptr, context->get_instr_block(26));
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_EQ(spv::Op::OpLogicalNot,
            context->get_def_use_mgr()->GetDef(26)->opcode());
  ASSERT_EQ(5, context->get_instr_block(26)->id());
  ASSERT_EQ(1, context->get_def_use_mgr()->NumUses(26));

  // Check that the def-use manager knows that the conditional branch operands
  // have been swapped.
  std::vector<std::pair<uint32_t, uint32_t>> phi_operand_to_new_operand_index =
      {{16, 2}, {21, 1}};
  for (std::pair<uint32_t, uint32_t>& entry :
       phi_operand_to_new_operand_index) {
    context->get_def_use_mgr()->WhileEachUse(
        entry.first,
        [&entry](opt::Instruction* inst, uint32_t operand_index) -> bool {
          if (inst->opcode() == spv::Op::OpBranchConditional) {
            EXPECT_EQ(entry.second, operand_index);
            return false;
          }
          return true;
        });
  }

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
         %26 = OpLogicalNot %14 %15
               OpSelectionMerge %17 None
               OpBranchConditional %26 %21 %16 20 10
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
               OpStore %8 %25
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
