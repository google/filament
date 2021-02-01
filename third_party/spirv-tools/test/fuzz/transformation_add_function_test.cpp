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

#include "source/fuzz/transformation_add_function.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_message.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

protobufs::AccessChainClampingInfo MakeAccessClampingInfo(
    uint32_t access_chain_id,
    const std::vector<std::pair<uint32_t, uint32_t>>& compare_and_select_ids) {
  protobufs::AccessChainClampingInfo result;
  result.set_access_chain_id(access_chain_id);
  for (auto& compare_and_select_id : compare_and_select_ids) {
    auto pair = result.add_compare_and_select_ids();
    pair->set_first(compare_and_select_id.first);
    pair->set_second(compare_and_select_id.second);
  }
  return result;
}

std::vector<protobufs::Instruction> GetInstructionsForFunction(
    spv_target_env env, const MessageConsumer& consumer,
    const std::string& donor, uint32_t function_id) {
  std::vector<protobufs::Instruction> result;
  const auto donor_context =
      BuildModule(env, consumer, donor, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  assert(fuzzerutil::IsValidAndWellFormed(
             donor_context.get(), validator_options, kConsoleMessageConsumer) &&
         "The given donor must be valid.");
  for (auto& function : *donor_context->module()) {
    if (function.result_id() == function_id) {
      function.ForEachInst([&result](opt::Instruction* inst) {
        opt::Instruction::OperandList input_operands;
        for (uint32_t i = 0; i < inst->NumInOperands(); i++) {
          input_operands.push_back(inst->GetInOperand(i));
        }
        result.push_back(MakeInstructionMessage(inst->opcode(), inst->type_id(),
                                                inst->result_id(),
                                                input_operands));
      });
      break;
    }
  }
  assert(!result.empty() && "The required function should have been found.");
  return result;
}

// Returns true if and only if every pointer parameter and variable associated
// with |function_id| in |context| is known by |transformation_context| to be
// irrelevant, with the exception of |loop_limiter_id|, which must not be
// irrelevant.  (It can be 0 if no loop limiter is expected, and 0 should not be
// deemed irrelevant).
bool AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
    opt::IRContext* context,
    const TransformationContext& transformation_context, uint32_t function_id,
    uint32_t loop_limiter_id) {
  // Look at all the functions until the function of interest is found.
  for (auto& function : *context->module()) {
    if (function.result_id() != function_id) {
      continue;
    }
    // Check that the parameters are all irrelevant.
    bool found_non_irrelevant_parameter = false;
    function.ForEachParam([context, &transformation_context,
                           &found_non_irrelevant_parameter](
                              opt::Instruction* inst) {
      if (context->get_def_use_mgr()->GetDef(inst->type_id())->opcode() ==
              SpvOpTypePointer &&
          !transformation_context.GetFactManager()->PointeeValueIsIrrelevant(
              inst->result_id())) {
        found_non_irrelevant_parameter = true;
      }
    });
    if (found_non_irrelevant_parameter) {
      // A non-irrelevant parameter was found.
      return false;
    }
    // Look through the instructions in the function's first block.
    for (auto& inst : *function.begin()) {
      if (inst.opcode() != SpvOpVariable) {
        // We have found a non-variable instruction; this means we have gotten
        // past all variables, so we are done.
        return true;
      }
      // The variable should be irrelevant if and only if it is not the loop
      // limiter.
      if ((inst.result_id() == loop_limiter_id) ==
          transformation_context.GetFactManager()->PointeeValueIsIrrelevant(
              inst.result_id())) {
        return false;
      }
    }
    assert(false &&
           "We should have processed all variables and returned by "
           "this point.");
  }
  assert(false && "We should have found the function of interest.");
  return true;
}

TEST(TransformationAddFunctionTest, BasicTest) {
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
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %8 %7 %9
         %18 = OpConstant %8 0
         %20 = OpConstant %6 0
         %28 = OpTypeBool
         %37 = OpConstant %6 1
         %42 = OpTypePointer Private %8
         %43 = OpVariable %42 Private
         %47 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationAddFunction transformation1(std::vector<protobufs::Instruction>(
      {MakeInstructionMessage(
           SpvOpFunction, 8, 13,
           {{SPV_OPERAND_TYPE_FUNCTION_CONTROL, {SpvFunctionControlMaskNone}},
            {SPV_OPERAND_TYPE_ID, {10}}}),
       MakeInstructionMessage(SpvOpFunctionParameter, 7, 11, {}),
       MakeInstructionMessage(SpvOpFunctionParameter, 9, 12, {}),
       MakeInstructionMessage(SpvOpLabel, 0, 14, {}),
       MakeInstructionMessage(
           SpvOpVariable, 9, 17,
           {{SPV_OPERAND_TYPE_STORAGE_CLASS, {SpvStorageClassFunction}}}),
       MakeInstructionMessage(
           SpvOpVariable, 7, 19,
           {{SPV_OPERAND_TYPE_STORAGE_CLASS, {SpvStorageClassFunction}}}),
       MakeInstructionMessage(
           SpvOpStore, 0, 0,
           {{SPV_OPERAND_TYPE_ID, {17}}, {SPV_OPERAND_TYPE_ID, {18}}}),
       MakeInstructionMessage(
           SpvOpStore, 0, 0,
           {{SPV_OPERAND_TYPE_ID, {19}}, {SPV_OPERAND_TYPE_ID, {20}}}),
       MakeInstructionMessage(SpvOpBranch, 0, 0, {{SPV_OPERAND_TYPE_ID, {21}}}),
       MakeInstructionMessage(SpvOpLabel, 0, 21, {}),
       MakeInstructionMessage(
           SpvOpLoopMerge, 0, 0,
           {{SPV_OPERAND_TYPE_ID, {23}},
            {SPV_OPERAND_TYPE_ID, {24}},
            {SPV_OPERAND_TYPE_LOOP_CONTROL, {SpvLoopControlMaskNone}}}),
       MakeInstructionMessage(SpvOpBranch, 0, 0, {{SPV_OPERAND_TYPE_ID, {25}}}),
       MakeInstructionMessage(SpvOpLabel, 0, 25, {}),
       MakeInstructionMessage(SpvOpLoad, 6, 26, {{SPV_OPERAND_TYPE_ID, {19}}}),
       MakeInstructionMessage(SpvOpLoad, 6, 27, {{SPV_OPERAND_TYPE_ID, {11}}}),
       MakeInstructionMessage(
           SpvOpSLessThan, 28, 29,
           {{SPV_OPERAND_TYPE_ID, {26}}, {SPV_OPERAND_TYPE_ID, {27}}}),
       MakeInstructionMessage(SpvOpBranchConditional, 0, 0,
                              {{SPV_OPERAND_TYPE_ID, {29}},
                               {SPV_OPERAND_TYPE_ID, {22}},
                               {SPV_OPERAND_TYPE_ID, {23}}}),
       MakeInstructionMessage(SpvOpLabel, 0, 22, {}),
       MakeInstructionMessage(SpvOpLoad, 8, 30, {{SPV_OPERAND_TYPE_ID, {12}}}),
       MakeInstructionMessage(SpvOpLoad, 6, 31, {{SPV_OPERAND_TYPE_ID, {19}}}),
       MakeInstructionMessage(SpvOpConvertSToF, 8, 32,
                              {{SPV_OPERAND_TYPE_ID, {31}}}),
       MakeInstructionMessage(
           SpvOpFMul, 8, 33,
           {{SPV_OPERAND_TYPE_ID, {30}}, {SPV_OPERAND_TYPE_ID, {32}}}),
       MakeInstructionMessage(SpvOpLoad, 8, 34, {{SPV_OPERAND_TYPE_ID, {17}}}),
       MakeInstructionMessage(
           SpvOpFAdd, 8, 35,
           {{SPV_OPERAND_TYPE_ID, {34}}, {SPV_OPERAND_TYPE_ID, {33}}}),
       MakeInstructionMessage(
           SpvOpStore, 0, 0,
           {{SPV_OPERAND_TYPE_ID, {17}}, {SPV_OPERAND_TYPE_ID, {35}}}),
       MakeInstructionMessage(SpvOpBranch, 0, 0, {{SPV_OPERAND_TYPE_ID, {24}}}),
       MakeInstructionMessage(SpvOpLabel, 0, 24, {}),
       MakeInstructionMessage(SpvOpLoad, 6, 36, {{SPV_OPERAND_TYPE_ID, {19}}}),
       MakeInstructionMessage(
           SpvOpIAdd, 6, 38,
           {{SPV_OPERAND_TYPE_ID, {36}}, {SPV_OPERAND_TYPE_ID, {37}}}),
       MakeInstructionMessage(
           SpvOpStore, 0, 0,
           {{SPV_OPERAND_TYPE_ID, {19}}, {SPV_OPERAND_TYPE_ID, {38}}}),
       MakeInstructionMessage(SpvOpBranch, 0, 0, {{SPV_OPERAND_TYPE_ID, {21}}}),
       MakeInstructionMessage(SpvOpLabel, 0, 23, {}),
       MakeInstructionMessage(SpvOpLoad, 8, 39, {{SPV_OPERAND_TYPE_ID, {17}}}),
       MakeInstructionMessage(SpvOpReturnValue, 0, 0,
                              {{SPV_OPERAND_TYPE_ID, {39}}}),
       MakeInstructionMessage(SpvOpFunctionEnd, 0, 0, {})}));

  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation1 = R"(
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
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %8 %7 %9
         %18 = OpConstant %8 0
         %20 = OpConstant %6 0
         %28 = OpTypeBool
         %37 = OpConstant %6 1
         %42 = OpTypePointer Private %8
         %43 = OpVariable %42 Private
         %47 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %13 = OpFunction %8 None %10
         %11 = OpFunctionParameter %7
         %12 = OpFunctionParameter %9
         %14 = OpLabel
         %17 = OpVariable %9 Function
         %19 = OpVariable %7 Function
               OpStore %17 %18
               OpStore %19 %20
               OpBranch %21
         %21 = OpLabel
               OpLoopMerge %23 %24 None
               OpBranch %25
         %25 = OpLabel
         %26 = OpLoad %6 %19
         %27 = OpLoad %6 %11
         %29 = OpSLessThan %28 %26 %27
               OpBranchConditional %29 %22 %23
         %22 = OpLabel
         %30 = OpLoad %8 %12
         %31 = OpLoad %6 %19
         %32 = OpConvertSToF %8 %31
         %33 = OpFMul %8 %30 %32
         %34 = OpLoad %8 %17
         %35 = OpFAdd %8 %34 %33
               OpStore %17 %35
               OpBranch %24
         %24 = OpLabel
         %36 = OpLoad %6 %19
         %38 = OpIAdd %6 %36 %37
               OpStore %19 %38
               OpBranch %21
         %23 = OpLabel
         %39 = OpLoad %8 %17
               OpReturnValue %39
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation1, context.get()));
  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(14));
  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(21));
  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(22));
  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(23));
  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(24));
  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(25));

  TransformationAddFunction transformation2(std::vector<protobufs::Instruction>(
      {MakeInstructionMessage(
           SpvOpFunction, 2, 15,
           {{SPV_OPERAND_TYPE_FUNCTION_CONTROL, {SpvFunctionControlMaskNone}},
            {SPV_OPERAND_TYPE_ID, {3}}}),
       MakeInstructionMessage(SpvOpLabel, 0, 16, {}),
       MakeInstructionMessage(
           SpvOpVariable, 7, 44,
           {{SPV_OPERAND_TYPE_STORAGE_CLASS, {SpvStorageClassFunction}}}),
       MakeInstructionMessage(
           SpvOpVariable, 9, 45,
           {{SPV_OPERAND_TYPE_STORAGE_CLASS, {SpvStorageClassFunction}}}),
       MakeInstructionMessage(
           SpvOpVariable, 7, 48,
           {{SPV_OPERAND_TYPE_STORAGE_CLASS, {SpvStorageClassFunction}}}),
       MakeInstructionMessage(
           SpvOpVariable, 9, 49,
           {{SPV_OPERAND_TYPE_STORAGE_CLASS, {SpvStorageClassFunction}}}),
       MakeInstructionMessage(
           SpvOpStore, 0, 0,
           {{SPV_OPERAND_TYPE_ID, {44}}, {SPV_OPERAND_TYPE_ID, {20}}}),
       MakeInstructionMessage(
           SpvOpStore, 0, 0,
           {{SPV_OPERAND_TYPE_ID, {45}}, {SPV_OPERAND_TYPE_ID, {18}}}),
       MakeInstructionMessage(SpvOpFunctionCall, 8, 46,
                              {{SPV_OPERAND_TYPE_ID, {13}},
                               {SPV_OPERAND_TYPE_ID, {44}},
                               {SPV_OPERAND_TYPE_ID, {45}}}),
       MakeInstructionMessage(
           SpvOpStore, 0, 0,
           {{SPV_OPERAND_TYPE_ID, {48}}, {SPV_OPERAND_TYPE_ID, {37}}}),
       MakeInstructionMessage(
           SpvOpStore, 0, 0,
           {{SPV_OPERAND_TYPE_ID, {49}}, {SPV_OPERAND_TYPE_ID, {47}}}),
       MakeInstructionMessage(SpvOpFunctionCall, 8, 50,
                              {{SPV_OPERAND_TYPE_ID, {13}},
                               {SPV_OPERAND_TYPE_ID, {48}},
                               {SPV_OPERAND_TYPE_ID, {49}}}),
       MakeInstructionMessage(
           SpvOpFAdd, 8, 51,
           {{SPV_OPERAND_TYPE_ID, {46}}, {SPV_OPERAND_TYPE_ID, {50}}}),
       MakeInstructionMessage(
           SpvOpStore, 0, 0,
           {{SPV_OPERAND_TYPE_ID, {43}}, {SPV_OPERAND_TYPE_ID, {51}}}),
       MakeInstructionMessage(SpvOpReturn, 0, 0, {}),
       MakeInstructionMessage(SpvOpFunctionEnd, 0, 0, {})}));

  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation2 = R"(
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
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %8 %7 %9
         %18 = OpConstant %8 0
         %20 = OpConstant %6 0
         %28 = OpTypeBool
         %37 = OpConstant %6 1
         %42 = OpTypePointer Private %8
         %43 = OpVariable %42 Private
         %47 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %13 = OpFunction %8 None %10
         %11 = OpFunctionParameter %7
         %12 = OpFunctionParameter %9
         %14 = OpLabel
         %17 = OpVariable %9 Function
         %19 = OpVariable %7 Function
               OpStore %17 %18
               OpStore %19 %20
               OpBranch %21
         %21 = OpLabel
               OpLoopMerge %23 %24 None
               OpBranch %25
         %25 = OpLabel
         %26 = OpLoad %6 %19
         %27 = OpLoad %6 %11
         %29 = OpSLessThan %28 %26 %27
               OpBranchConditional %29 %22 %23
         %22 = OpLabel
         %30 = OpLoad %8 %12
         %31 = OpLoad %6 %19
         %32 = OpConvertSToF %8 %31
         %33 = OpFMul %8 %30 %32
         %34 = OpLoad %8 %17
         %35 = OpFAdd %8 %34 %33
               OpStore %17 %35
               OpBranch %24
         %24 = OpLabel
         %36 = OpLoad %6 %19
         %38 = OpIAdd %6 %36 %37
               OpStore %19 %38
               OpBranch %21
         %23 = OpLabel
         %39 = OpLoad %8 %17
               OpReturnValue %39
               OpFunctionEnd
         %15 = OpFunction %2 None %3
         %16 = OpLabel
         %44 = OpVariable %7 Function
         %45 = OpVariable %9 Function
         %48 = OpVariable %7 Function
         %49 = OpVariable %9 Function
               OpStore %44 %20
               OpStore %45 %18
         %46 = OpFunctionCall %8 %13 %44 %45
               OpStore %48 %37
               OpStore %49 %47
         %50 = OpFunctionCall %8 %13 %48 %49
         %51 = OpFAdd %8 %46 %50
               OpStore %43 %51
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation2, context.get()));
  ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(16));
}

TEST(TransformationAddFunctionTest, InapplicableTransformations) {
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
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %8 %7 %9
         %18 = OpConstant %8 0
         %20 = OpConstant %6 0
         %28 = OpTypeBool
         %37 = OpConstant %6 1
         %42 = OpTypePointer Private %8
         %43 = OpVariable %42 Private
         %47 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %13 = OpFunction %8 None %10
         %11 = OpFunctionParameter %7
         %12 = OpFunctionParameter %9
         %14 = OpLabel
         %17 = OpVariable %9 Function
         %19 = OpVariable %7 Function
               OpStore %17 %18
               OpStore %19 %20
               OpBranch %21
         %21 = OpLabel
               OpLoopMerge %23 %24 None
               OpBranch %25
         %25 = OpLabel
         %26 = OpLoad %6 %19
         %27 = OpLoad %6 %11
         %29 = OpSLessThan %28 %26 %27
               OpBranchConditional %29 %22 %23
         %22 = OpLabel
         %30 = OpLoad %8 %12
         %31 = OpLoad %6 %19
         %32 = OpConvertSToF %8 %31
         %33 = OpFMul %8 %30 %32
         %34 = OpLoad %8 %17
         %35 = OpFAdd %8 %34 %33
               OpStore %17 %35
               OpBranch %24
         %24 = OpLabel
         %36 = OpLoad %6 %19
         %38 = OpIAdd %6 %36 %37
               OpStore %19 %38
               OpBranch %21
         %23 = OpLabel
         %39 = OpLoad %8 %17
               OpReturnValue %39
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // No instructions
  ASSERT_FALSE(
      TransformationAddFunction(std::vector<protobufs::Instruction>({}))
          .IsApplicable(context.get(), transformation_context));

  // No function begin
  ASSERT_FALSE(
      TransformationAddFunction(
          std::vector<protobufs::Instruction>(
              {MakeInstructionMessage(SpvOpFunctionParameter, 7, 11, {}),
               MakeInstructionMessage(SpvOpFunctionParameter, 9, 12, {}),
               MakeInstructionMessage(SpvOpLabel, 0, 14, {})}))
          .IsApplicable(context.get(), transformation_context));

  // No OpLabel
  ASSERT_FALSE(
      TransformationAddFunction(
          std::vector<protobufs::Instruction>(
              {MakeInstructionMessage(SpvOpFunction, 8, 13,
                                      {{SPV_OPERAND_TYPE_FUNCTION_CONTROL,
                                        {SpvFunctionControlMaskNone}},
                                       {SPV_OPERAND_TYPE_ID, {10}}}),
               MakeInstructionMessage(SpvOpReturnValue, 0, 0,
                                      {{SPV_OPERAND_TYPE_ID, {39}}}),
               MakeInstructionMessage(SpvOpFunctionEnd, 0, 0, {})}))
          .IsApplicable(context.get(), transformation_context));

  // Abrupt end of instructions
  ASSERT_FALSE(TransformationAddFunction(
                   std::vector<protobufs::Instruction>({MakeInstructionMessage(
                       SpvOpFunction, 8, 13,
                       {{SPV_OPERAND_TYPE_FUNCTION_CONTROL,
                         {SpvFunctionControlMaskNone}},
                        {SPV_OPERAND_TYPE_ID, {10}}})}))
                   .IsApplicable(context.get(), transformation_context));

  // No function end
  ASSERT_FALSE(
      TransformationAddFunction(
          std::vector<protobufs::Instruction>(
              {MakeInstructionMessage(SpvOpFunction, 8, 13,
                                      {{SPV_OPERAND_TYPE_FUNCTION_CONTROL,
                                        {SpvFunctionControlMaskNone}},
                                       {SPV_OPERAND_TYPE_ID, {10}}}),
               MakeInstructionMessage(SpvOpLabel, 0, 14, {}),
               MakeInstructionMessage(SpvOpReturnValue, 0, 0,
                                      {{SPV_OPERAND_TYPE_ID, {39}}})}))
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddFunctionTest, LoopLimiters) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
          %8 = OpConstant %6 0
          %9 = OpConstant %6 1
         %10 = OpConstant %6 5
         %11 = OpTypeBool
         %12 = OpConstantTrue %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;

  std::vector<protobufs::Instruction> instructions;
  instructions.push_back(MakeInstructionMessage(
      SpvOpFunction, 2, 30,
      {{SPV_OPERAND_TYPE_FUNCTION_CONTROL, {SpvFunctionControlMaskNone}},
       {SPV_OPERAND_TYPE_TYPE_ID, {3}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 31, {}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpBranch, 0, 0, {{SPV_OPERAND_TYPE_ID, {20}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 20, {}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpLoopMerge, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {21}},
       {SPV_OPERAND_TYPE_ID, {22}},
       {SPV_OPERAND_TYPE_LOOP_CONTROL, {SpvLoopControlMaskNone}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpBranchConditional, 0, 0,
                                                {{SPV_OPERAND_TYPE_ID, {12}},
                                                 {SPV_OPERAND_TYPE_ID, {23}},
                                                 {SPV_OPERAND_TYPE_ID, {21}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 23, {}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpLoopMerge, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {25}},
       {SPV_OPERAND_TYPE_ID, {26}},
       {SPV_OPERAND_TYPE_LOOP_CONTROL, {SpvLoopControlMaskNone}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpBranch, 0, 0, {{SPV_OPERAND_TYPE_ID, {28}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 28, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpBranchConditional, 0, 0,
                                                {{SPV_OPERAND_TYPE_ID, {12}},
                                                 {SPV_OPERAND_TYPE_ID, {26}},
                                                 {SPV_OPERAND_TYPE_ID, {25}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 26, {}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpBranch, 0, 0, {{SPV_OPERAND_TYPE_ID, {23}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 25, {}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpLoopMerge, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {24}},
       {SPV_OPERAND_TYPE_ID, {27}},
       {SPV_OPERAND_TYPE_LOOP_CONTROL, {SpvLoopControlMaskNone}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpBranchConditional, 0, 0,
                                                {{SPV_OPERAND_TYPE_ID, {12}},
                                                 {SPV_OPERAND_TYPE_ID, {24}},
                                                 {SPV_OPERAND_TYPE_ID, {27}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 27, {}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpBranch, 0, 0, {{SPV_OPERAND_TYPE_ID, {25}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 24, {}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpBranch, 0, 0, {{SPV_OPERAND_TYPE_ID, {22}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 22, {}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpBranch, 0, 0, {{SPV_OPERAND_TYPE_ID, {20}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 21, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpReturn, 0, 0, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpFunctionEnd, 0, 0, {}));

  spvtools::ValidatorOptions validator_options;

  const auto context1 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  const auto context2 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context1(
      MakeUnique<FactManager>(context1.get()), validator_options);
  TransformationContext transformation_context2(
      MakeUnique<FactManager>(context2.get()), validator_options);

  TransformationAddFunction add_dead_function(instructions);
  ASSERT_TRUE(
      add_dead_function.IsApplicable(context1.get(), transformation_context1));
  ApplyAndCheckFreshIds(add_dead_function, context1.get(),
                        &transformation_context1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));
  // The added function should not be deemed livesafe.
  ASSERT_FALSE(
      transformation_context1.GetFactManager()->FunctionIsLivesafe(30));
  // All variables/parameters in the function should be deemed irrelevant.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context1.get(), transformation_context1, 30, 0));

  std::string added_as_dead_code = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
          %8 = OpConstant %6 0
          %9 = OpConstant %6 1
         %10 = OpConstant %6 5
         %11 = OpTypeBool
         %12 = OpConstantTrue %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %30 = OpFunction %2 None %3
         %31 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %21 %22 None
               OpBranchConditional %12 %23 %21
         %23 = OpLabel
               OpLoopMerge %25 %26 None
               OpBranch %28
         %28 = OpLabel
               OpBranchConditional %12 %26 %25
         %26 = OpLabel
               OpBranch %23
         %25 = OpLabel
               OpLoopMerge %24 %27 None
               OpBranchConditional %12 %24 %27
         %27 = OpLabel
               OpBranch %25
         %24 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpBranch %20
         %21 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, added_as_dead_code, context1.get()));

  protobufs::LoopLimiterInfo loop_limiter1;
  loop_limiter1.set_loop_header_id(20);
  loop_limiter1.set_load_id(101);
  loop_limiter1.set_increment_id(102);
  loop_limiter1.set_compare_id(103);
  loop_limiter1.set_logical_op_id(104);

  protobufs::LoopLimiterInfo loop_limiter2;
  loop_limiter2.set_loop_header_id(23);
  loop_limiter2.set_load_id(105);
  loop_limiter2.set_increment_id(106);
  loop_limiter2.set_compare_id(107);
  loop_limiter2.set_logical_op_id(108);

  protobufs::LoopLimiterInfo loop_limiter3;
  loop_limiter3.set_loop_header_id(25);
  loop_limiter3.set_load_id(109);
  loop_limiter3.set_increment_id(110);
  loop_limiter3.set_compare_id(111);
  loop_limiter3.set_logical_op_id(112);

  std::vector<protobufs::LoopLimiterInfo> loop_limiters = {
      loop_limiter1, loop_limiter2, loop_limiter3};

  TransformationAddFunction add_livesafe_function(instructions, 100, 10,
                                                  loop_limiters, 0, {});
  ASSERT_TRUE(add_livesafe_function.IsApplicable(context2.get(),
                                                 transformation_context2));
  ApplyAndCheckFreshIds(add_livesafe_function, context2.get(),
                        &transformation_context2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context2.get(), validator_options, kConsoleMessageConsumer));
  // The added function should indeed be deemed livesafe.
  ASSERT_TRUE(transformation_context2.GetFactManager()->FunctionIsLivesafe(30));
  // All variables/parameters in the function should be deemed irrelevant,
  // except the loop limiter.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context2.get(), transformation_context2, 30, 100));
  std::string added_as_livesafe_code = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
          %8 = OpConstant %6 0
          %9 = OpConstant %6 1
         %10 = OpConstant %6 5
         %11 = OpTypeBool
         %12 = OpConstantTrue %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %30 = OpFunction %2 None %3
         %31 = OpLabel
        %100 = OpVariable %7 Function %8
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %21 %22 None
               OpBranchConditional %12 %23 %21
         %23 = OpLabel
               OpLoopMerge %25 %26 None
               OpBranch %28
         %28 = OpLabel
               OpBranchConditional %12 %26 %25
         %26 = OpLabel
        %105 = OpLoad %6 %100
        %106 = OpIAdd %6 %105 %9
               OpStore %100 %106
        %107 = OpUGreaterThanEqual %11 %105 %10
               OpBranchConditional %107 %25 %23
         %25 = OpLabel
               OpLoopMerge %24 %27 None
               OpBranchConditional %12 %24 %27
         %27 = OpLabel
        %109 = OpLoad %6 %100
        %110 = OpIAdd %6 %109 %9
               OpStore %100 %110
        %111 = OpUGreaterThanEqual %11 %109 %10
               OpBranchConditional %111 %24 %25
         %24 = OpLabel
               OpBranch %22
         %22 = OpLabel
        %101 = OpLoad %6 %100
        %102 = OpIAdd %6 %101 %9
               OpStore %100 %102
        %103 = OpUGreaterThanEqual %11 %101 %10
               OpBranchConditional %103 %21 %20
         %21 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, added_as_livesafe_code, context2.get()));
}

TEST(TransformationAddFunctionTest, KillAndUnreachableInVoidFunction) {
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
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 2
         %14 = OpTypeBool
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;

  std::vector<protobufs::Instruction> instructions;

  instructions.push_back(MakeInstructionMessage(
      SpvOpFunction, 2, 10,
      {{SPV_OPERAND_TYPE_FUNCTION_CONTROL, {SpvFunctionControlMaskNone}},
       {SPV_OPERAND_TYPE_TYPE_ID, {8}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpFunctionParameter, 7, 9, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 11, {}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 12, {{SPV_OPERAND_TYPE_ID, {9}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpIEqual, 14, 15,
      {{SPV_OPERAND_TYPE_ID, {12}}, {SPV_OPERAND_TYPE_ID, {13}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpSelectionMerge, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {17}},
       {SPV_OPERAND_TYPE_SELECTION_CONTROL, {SpvSelectionControlMaskNone}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpBranchConditional, 0, 0,
                                                {{SPV_OPERAND_TYPE_ID, {15}},
                                                 {SPV_OPERAND_TYPE_ID, {16}},
                                                 {SPV_OPERAND_TYPE_ID, {17}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 16, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpUnreachable, 0, 0, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 17, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpKill, 0, 0, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpFunctionEnd, 0, 0, {}));

  spvtools::ValidatorOptions validator_options;

  const auto context1 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  const auto context2 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context1(
      MakeUnique<FactManager>(context1.get()), validator_options);
  TransformationContext transformation_context2(
      MakeUnique<FactManager>(context2.get()), validator_options);

  TransformationAddFunction add_dead_function(instructions);
  ASSERT_TRUE(
      add_dead_function.IsApplicable(context1.get(), transformation_context1));
  ApplyAndCheckFreshIds(add_dead_function, context1.get(),
                        &transformation_context1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));
  // The added function should not be deemed livesafe.
  ASSERT_FALSE(
      transformation_context1.GetFactManager()->FunctionIsLivesafe(10));
  // All variables/parameters in the function should be deemed irrelevant.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context1.get(), transformation_context1, 10, 0));

  std::string added_as_dead_code = R"(
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
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 2
         %14 = OpTypeBool
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpLoad %6 %9
         %15 = OpIEqual %14 %12 %13
               OpSelectionMerge %17 None
               OpBranchConditional %15 %16 %17
         %16 = OpLabel
               OpUnreachable
         %17 = OpLabel
               OpKill
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, added_as_dead_code, context1.get()));

  TransformationAddFunction add_livesafe_function(instructions, 0, 0, {}, 0,
                                                  {});
  ASSERT_TRUE(add_livesafe_function.IsApplicable(context2.get(),
                                                 transformation_context2));
  ApplyAndCheckFreshIds(add_livesafe_function, context2.get(),
                        &transformation_context2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context2.get(), validator_options, kConsoleMessageConsumer));
  // The added function should indeed be deemed livesafe.
  ASSERT_TRUE(transformation_context2.GetFactManager()->FunctionIsLivesafe(10));
  // All variables/parameters in the function should be deemed irrelevant.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context2.get(), transformation_context2, 10, 0));
  std::string added_as_livesafe_code = R"(
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
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 2
         %14 = OpTypeBool
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpLoad %6 %9
         %15 = OpIEqual %14 %12 %13
               OpSelectionMerge %17 None
               OpBranchConditional %15 %16 %17
         %16 = OpLabel
               OpReturn
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, added_as_livesafe_code, context2.get()));
}

TEST(TransformationAddFunctionTest, KillAndUnreachableInNonVoidFunction) {
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
          %8 = OpTypeFunction %2 %7
         %50 = OpTypeFunction %6 %7
         %13 = OpConstant %6 2
         %14 = OpTypeBool
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;

  std::vector<protobufs::Instruction> instructions;

  instructions.push_back(MakeInstructionMessage(
      SpvOpFunction, 6, 10,
      {{SPV_OPERAND_TYPE_FUNCTION_CONTROL, {SpvFunctionControlMaskNone}},
       {SPV_OPERAND_TYPE_TYPE_ID, {50}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpFunctionParameter, 7, 9, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 11, {}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 12, {{SPV_OPERAND_TYPE_ID, {9}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpIEqual, 14, 15,
      {{SPV_OPERAND_TYPE_ID, {12}}, {SPV_OPERAND_TYPE_ID, {13}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpSelectionMerge, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {17}},
       {SPV_OPERAND_TYPE_SELECTION_CONTROL, {SpvSelectionControlMaskNone}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpBranchConditional, 0, 0,
                                                {{SPV_OPERAND_TYPE_ID, {15}},
                                                 {SPV_OPERAND_TYPE_ID, {16}},
                                                 {SPV_OPERAND_TYPE_ID, {17}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 16, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpUnreachable, 0, 0, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 17, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpKill, 0, 0, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpFunctionEnd, 0, 0, {}));

  spvtools::ValidatorOptions validator_options;

  const auto context1 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  const auto context2 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context1(
      MakeUnique<FactManager>(context1.get()), validator_options);
  TransformationContext transformation_context2(
      MakeUnique<FactManager>(context2.get()), validator_options);

  TransformationAddFunction add_dead_function(instructions);
  ASSERT_TRUE(
      add_dead_function.IsApplicable(context1.get(), transformation_context1));
  ApplyAndCheckFreshIds(add_dead_function, context1.get(),
                        &transformation_context1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));
  // The added function should not be deemed livesafe.
  ASSERT_FALSE(
      transformation_context1.GetFactManager()->FunctionIsLivesafe(10));
  // All variables/parameters in the function should be deemed irrelevant.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context1.get(), transformation_context1, 10, 0));

  std::string added_as_dead_code = R"(
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
          %8 = OpTypeFunction %2 %7
         %50 = OpTypeFunction %6 %7
         %13 = OpConstant %6 2
         %14 = OpTypeBool
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %50
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpLoad %6 %9
         %15 = OpIEqual %14 %12 %13
               OpSelectionMerge %17 None
               OpBranchConditional %15 %16 %17
         %16 = OpLabel
               OpUnreachable
         %17 = OpLabel
               OpKill
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, added_as_dead_code, context1.get()));

  TransformationAddFunction add_livesafe_function(instructions, 0, 0, {}, 13,
                                                  {});
  ASSERT_TRUE(add_livesafe_function.IsApplicable(context2.get(),
                                                 transformation_context2));
  ApplyAndCheckFreshIds(add_livesafe_function, context2.get(),
                        &transformation_context2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context2.get(), validator_options, kConsoleMessageConsumer));
  // The added function should indeed be deemed livesafe.
  ASSERT_TRUE(transformation_context2.GetFactManager()->FunctionIsLivesafe(10));
  // All variables/parameters in the function should be deemed irrelevant.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context2.get(), transformation_context2, 10, 0));
  std::string added_as_livesafe_code = R"(
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
          %8 = OpTypeFunction %2 %7
         %50 = OpTypeFunction %6 %7
         %13 = OpConstant %6 2
         %14 = OpTypeBool
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %50
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpLoad %6 %9
         %15 = OpIEqual %14 %12 %13
               OpSelectionMerge %17 None
               OpBranchConditional %15 %16 %17
         %16 = OpLabel
               OpReturnValue %13
         %17 = OpLabel
               OpReturnValue %13
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, added_as_livesafe_code, context2.get()));
}

TEST(TransformationAddFunctionTest, ClampedAccessChains) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
        %100 = OpTypeBool
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %15 = OpTypeInt 32 0
        %102 = OpTypePointer Function %15
          %8 = OpTypeFunction %2 %7 %102 %7
         %16 = OpConstant %15 5
         %17 = OpTypeArray %6 %16
         %18 = OpTypeArray %17 %16
         %19 = OpTypePointer Private %18
         %20 = OpVariable %19 Private
         %21 = OpConstant %6 0
         %23 = OpTypePointer Private %6
         %26 = OpTypePointer Function %17
         %29 = OpTypePointer Private %17
         %33 = OpConstant %6 4
        %200 = OpConstant %15 4
         %35 = OpConstant %15 10
         %36 = OpTypeArray %6 %35
         %37 = OpTypePointer Private %36
         %38 = OpVariable %37 Private
         %54 = OpTypeFloat 32
         %55 = OpTypeVector %54 4
         %56 = OpTypePointer Private %55
         %57 = OpVariable %56 Private
         %59 = OpTypeVector %54 3
         %60 = OpTypeMatrix %59 2
         %61 = OpTypePointer Private %60
         %62 = OpVariable %61 Private
         %64 = OpTypePointer Private %54
         %69 = OpConstant %54 2
         %71 = OpConstant %6 1
         %72 = OpConstant %6 2
        %201 = OpConstant %15 2
         %73 = OpConstant %6 3
        %202 = OpConstant %15 3
        %203 = OpConstant %6 1
        %204 = OpConstant %6 9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;

  std::vector<protobufs::Instruction> instructions;

  instructions.push_back(MakeInstructionMessage(
      SpvOpFunction, 2, 12,
      {{SPV_OPERAND_TYPE_FUNCTION_CONTROL, {SpvFunctionControlMaskNone}},
       {SPV_OPERAND_TYPE_TYPE_ID, {8}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpFunctionParameter, 7, 9, {}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpFunctionParameter, 102, 10, {}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpFunctionParameter, 7, 11, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 13, {}));

  instructions.push_back(MakeInstructionMessage(
      SpvOpVariable, 7, 14,
      {{SPV_OPERAND_TYPE_STORAGE_CLASS, {SpvStorageClassFunction}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpVariable, 26, 27,
      {{SPV_OPERAND_TYPE_STORAGE_CLASS, {SpvStorageClassFunction}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 22, {{SPV_OPERAND_TYPE_ID, {11}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpAccessChain, 23, 24,
                                                {{SPV_OPERAND_TYPE_ID, {20}},
                                                 {SPV_OPERAND_TYPE_ID, {21}},
                                                 {SPV_OPERAND_TYPE_ID, {22}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 25, {{SPV_OPERAND_TYPE_ID, {24}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpStore, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {14}}, {SPV_OPERAND_TYPE_ID, {25}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 15, 28, {{SPV_OPERAND_TYPE_ID, {10}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpAccessChain, 29, 30,
      {{SPV_OPERAND_TYPE_ID, {20}}, {SPV_OPERAND_TYPE_ID, {28}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 17, 31, {{SPV_OPERAND_TYPE_ID, {30}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpStore, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {27}}, {SPV_OPERAND_TYPE_ID, {31}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 32, {{SPV_OPERAND_TYPE_ID, {9}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpInBoundsAccessChain, 7, 34,
      {{SPV_OPERAND_TYPE_ID, {27}}, {SPV_OPERAND_TYPE_ID, {32}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpStore, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {34}}, {SPV_OPERAND_TYPE_ID, {33}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 39, {{SPV_OPERAND_TYPE_ID, {9}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpAccessChain, 23, 40,
      {{SPV_OPERAND_TYPE_ID, {38}}, {SPV_OPERAND_TYPE_ID, {33}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 41, {{SPV_OPERAND_TYPE_ID, {40}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpInBoundsAccessChain, 23, 42,
      {{SPV_OPERAND_TYPE_ID, {38}}, {SPV_OPERAND_TYPE_ID, {39}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpStore, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {42}}, {SPV_OPERAND_TYPE_ID, {41}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 15, 43, {{SPV_OPERAND_TYPE_ID, {10}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 44, {{SPV_OPERAND_TYPE_ID, {11}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 45, {{SPV_OPERAND_TYPE_ID, {9}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 15, 46, {{SPV_OPERAND_TYPE_ID, {10}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpIAdd, 6, 47,
      {{SPV_OPERAND_TYPE_ID, {45}}, {SPV_OPERAND_TYPE_ID, {46}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpAccessChain, 23, 48,
      {{SPV_OPERAND_TYPE_ID, {38}}, {SPV_OPERAND_TYPE_ID, {47}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 49, {{SPV_OPERAND_TYPE_ID, {48}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpInBoundsAccessChain, 23,
                                                50,
                                                {{SPV_OPERAND_TYPE_ID, {20}},
                                                 {SPV_OPERAND_TYPE_ID, {43}},
                                                 {SPV_OPERAND_TYPE_ID, {44}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 51, {{SPV_OPERAND_TYPE_ID, {50}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpIAdd, 6, 52,
      {{SPV_OPERAND_TYPE_ID, {51}}, {SPV_OPERAND_TYPE_ID, {49}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpAccessChain, 23, 53,
                                                {{SPV_OPERAND_TYPE_ID, {20}},
                                                 {SPV_OPERAND_TYPE_ID, {43}},
                                                 {SPV_OPERAND_TYPE_ID, {44}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpStore, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {53}}, {SPV_OPERAND_TYPE_ID, {52}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 15, 58, {{SPV_OPERAND_TYPE_ID, {10}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 63, {{SPV_OPERAND_TYPE_ID, {11}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpAccessChain, 64, 65,
                                                {{SPV_OPERAND_TYPE_ID, {62}},
                                                 {SPV_OPERAND_TYPE_ID, {21}},
                                                 {SPV_OPERAND_TYPE_ID, {63}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpAccessChain, 64, 101,
                                                {{SPV_OPERAND_TYPE_ID, {62}},
                                                 {SPV_OPERAND_TYPE_ID, {45}},
                                                 {SPV_OPERAND_TYPE_ID, {46}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 54, 66, {{SPV_OPERAND_TYPE_ID, {65}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpAccessChain, 64, 67,
      {{SPV_OPERAND_TYPE_ID, {57}}, {SPV_OPERAND_TYPE_ID, {58}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpStore, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {67}}, {SPV_OPERAND_TYPE_ID, {66}}}));
  instructions.push_back(
      MakeInstructionMessage(SpvOpLoad, 6, 68, {{SPV_OPERAND_TYPE_ID, {9}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpInBoundsAccessChain, 64, 70,
      {{SPV_OPERAND_TYPE_ID, {57}}, {SPV_OPERAND_TYPE_ID, {68}}}));
  instructions.push_back(MakeInstructionMessage(
      SpvOpStore, 0, 0,
      {{SPV_OPERAND_TYPE_ID, {70}}, {SPV_OPERAND_TYPE_ID, {69}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpReturn, 0, 0, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpFunctionEnd, 0, 0, {}));

  spvtools::ValidatorOptions validator_options;

  const auto context1 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  const auto context2 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context1(
      MakeUnique<FactManager>(context1.get()), validator_options);
  TransformationContext transformation_context2(
      MakeUnique<FactManager>(context2.get()), validator_options);

  TransformationAddFunction add_dead_function(instructions);
  ASSERT_TRUE(
      add_dead_function.IsApplicable(context1.get(), transformation_context1));
  ApplyAndCheckFreshIds(add_dead_function, context1.get(),
                        &transformation_context1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));
  // The function should not be deemed livesafe
  ASSERT_FALSE(
      transformation_context1.GetFactManager()->FunctionIsLivesafe(12));
  // All variables/parameters in the function should be deemed irrelevant.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context1.get(), transformation_context1, 12, 0));

  std::string added_as_dead_code = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
        %100 = OpTypeBool
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %15 = OpTypeInt 32 0
        %102 = OpTypePointer Function %15
          %8 = OpTypeFunction %2 %7 %102 %7
         %16 = OpConstant %15 5
         %17 = OpTypeArray %6 %16
         %18 = OpTypeArray %17 %16
         %19 = OpTypePointer Private %18
         %20 = OpVariable %19 Private
         %21 = OpConstant %6 0
         %23 = OpTypePointer Private %6
         %26 = OpTypePointer Function %17
         %29 = OpTypePointer Private %17
         %33 = OpConstant %6 4
        %200 = OpConstant %15 4
         %35 = OpConstant %15 10
         %36 = OpTypeArray %6 %35
         %37 = OpTypePointer Private %36
         %38 = OpVariable %37 Private
         %54 = OpTypeFloat 32
         %55 = OpTypeVector %54 4
         %56 = OpTypePointer Private %55
         %57 = OpVariable %56 Private
         %59 = OpTypeVector %54 3
         %60 = OpTypeMatrix %59 2
         %61 = OpTypePointer Private %60
         %62 = OpVariable %61 Private
         %64 = OpTypePointer Private %54
         %69 = OpConstant %54 2
         %71 = OpConstant %6 1
         %72 = OpConstant %6 2
        %201 = OpConstant %15 2
         %73 = OpConstant %6 3
        %202 = OpConstant %15 3
        %203 = OpConstant %6 1
        %204 = OpConstant %6 9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %10 = OpFunctionParameter %102
         %11 = OpFunctionParameter %7
         %13 = OpLabel
         %14 = OpVariable %7 Function
         %27 = OpVariable %26 Function
         %22 = OpLoad %6 %11
         %24 = OpAccessChain %23 %20 %21 %22
         %25 = OpLoad %6 %24
               OpStore %14 %25
         %28 = OpLoad %15 %10
         %30 = OpAccessChain %29 %20 %28
         %31 = OpLoad %17 %30
               OpStore %27 %31
         %32 = OpLoad %6 %9
         %34 = OpInBoundsAccessChain %7 %27 %32
               OpStore %34 %33
         %39 = OpLoad %6 %9
         %40 = OpAccessChain %23 %38 %33
         %41 = OpLoad %6 %40
         %42 = OpInBoundsAccessChain %23 %38 %39
               OpStore %42 %41
         %43 = OpLoad %15 %10
         %44 = OpLoad %6 %11
         %45 = OpLoad %6 %9
         %46 = OpLoad %15 %10
         %47 = OpIAdd %6 %45 %46
         %48 = OpAccessChain %23 %38 %47
         %49 = OpLoad %6 %48
         %50 = OpInBoundsAccessChain %23 %20 %43 %44
         %51 = OpLoad %6 %50
         %52 = OpIAdd %6 %51 %49
         %53 = OpAccessChain %23 %20 %43 %44
               OpStore %53 %52
         %58 = OpLoad %15 %10
         %63 = OpLoad %6 %11
         %65 = OpAccessChain %64 %62 %21 %63
        %101 = OpAccessChain %64 %62 %45 %46
         %66 = OpLoad %54 %65
         %67 = OpAccessChain %64 %57 %58
               OpStore %67 %66
         %68 = OpLoad %6 %9
         %70 = OpInBoundsAccessChain %64 %57 %68
               OpStore %70 %69
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, added_as_dead_code, context1.get()));

  std::vector<protobufs::AccessChainClampingInfo> access_chain_clamping_info;
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(24, {{1001, 2001}, {1002, 2002}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(30, {{1003, 2003}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(34, {{1004, 2004}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(40, {{1005, 2005}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(42, {{1006, 2006}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(48, {{1007, 2007}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(50, {{1008, 2008}, {1009, 2009}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(53, {{1010, 2010}, {1011, 2011}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(65, {{1012, 2012}, {1013, 2013}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(101, {{1014, 2014}, {1015, 2015}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(67, {{1016, 2016}}));
  access_chain_clamping_info.push_back(
      MakeAccessClampingInfo(70, {{1017, 2017}}));

  TransformationAddFunction add_livesafe_function(instructions, 0, 0, {}, 13,
                                                  access_chain_clamping_info);
  ASSERT_TRUE(add_livesafe_function.IsApplicable(context2.get(),
                                                 transformation_context2));
  ApplyAndCheckFreshIds(add_livesafe_function, context2.get(),
                        &transformation_context2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context2.get(), validator_options, kConsoleMessageConsumer));
  // The function should be deemed livesafe
  ASSERT_TRUE(transformation_context2.GetFactManager()->FunctionIsLivesafe(12));
  // All variables/parameters in the function should be deemed irrelevant.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context2.get(), transformation_context2, 12, 0));
  std::string added_as_livesafe_code = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
        %100 = OpTypeBool
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %15 = OpTypeInt 32 0
        %102 = OpTypePointer Function %15
          %8 = OpTypeFunction %2 %7 %102 %7
         %16 = OpConstant %15 5
         %17 = OpTypeArray %6 %16
         %18 = OpTypeArray %17 %16
         %19 = OpTypePointer Private %18
         %20 = OpVariable %19 Private
         %21 = OpConstant %6 0
         %23 = OpTypePointer Private %6
         %26 = OpTypePointer Function %17
         %29 = OpTypePointer Private %17
         %33 = OpConstant %6 4
        %200 = OpConstant %15 4
         %35 = OpConstant %15 10
         %36 = OpTypeArray %6 %35
         %37 = OpTypePointer Private %36
         %38 = OpVariable %37 Private
         %54 = OpTypeFloat 32
         %55 = OpTypeVector %54 4
         %56 = OpTypePointer Private %55
         %57 = OpVariable %56 Private
         %59 = OpTypeVector %54 3
         %60 = OpTypeMatrix %59 2
         %61 = OpTypePointer Private %60
         %62 = OpVariable %61 Private
         %64 = OpTypePointer Private %54
         %69 = OpConstant %54 2
         %71 = OpConstant %6 1
         %72 = OpConstant %6 2
        %201 = OpConstant %15 2
         %73 = OpConstant %6 3
        %202 = OpConstant %15 3
        %203 = OpConstant %6 1
        %204 = OpConstant %6 9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %10 = OpFunctionParameter %102
         %11 = OpFunctionParameter %7
         %13 = OpLabel
         %14 = OpVariable %7 Function
         %27 = OpVariable %26 Function
         %22 = OpLoad %6 %11
       %1002 = OpULessThanEqual %100 %22 %33
       %2002 = OpSelect %6 %1002 %22 %33
         %24 = OpAccessChain %23 %20 %21 %2002
         %25 = OpLoad %6 %24
               OpStore %14 %25
         %28 = OpLoad %15 %10
       %1003 = OpULessThanEqual %100 %28 %200
       %2003 = OpSelect %15 %1003 %28 %200
         %30 = OpAccessChain %29 %20 %2003
         %31 = OpLoad %17 %30
               OpStore %27 %31
         %32 = OpLoad %6 %9
       %1004 = OpULessThanEqual %100 %32 %33
       %2004 = OpSelect %6 %1004 %32 %33
         %34 = OpInBoundsAccessChain %7 %27 %2004
               OpStore %34 %33
         %39 = OpLoad %6 %9
         %40 = OpAccessChain %23 %38 %33
         %41 = OpLoad %6 %40
       %1006 = OpULessThanEqual %100 %39 %204
       %2006 = OpSelect %6 %1006 %39 %204
         %42 = OpInBoundsAccessChain %23 %38 %2006
               OpStore %42 %41
         %43 = OpLoad %15 %10
         %44 = OpLoad %6 %11
         %45 = OpLoad %6 %9
         %46 = OpLoad %15 %10
         %47 = OpIAdd %6 %45 %46
       %1007 = OpULessThanEqual %100 %47 %204
       %2007 = OpSelect %6 %1007 %47 %204
         %48 = OpAccessChain %23 %38 %2007
         %49 = OpLoad %6 %48
       %1008 = OpULessThanEqual %100 %43 %200
       %2008 = OpSelect %15 %1008 %43 %200
       %1009 = OpULessThanEqual %100 %44 %33
       %2009 = OpSelect %6 %1009 %44 %33
         %50 = OpInBoundsAccessChain %23 %20 %2008 %2009
         %51 = OpLoad %6 %50
         %52 = OpIAdd %6 %51 %49
       %1010 = OpULessThanEqual %100 %43 %200
       %2010 = OpSelect %15 %1010 %43 %200
       %1011 = OpULessThanEqual %100 %44 %33
       %2011 = OpSelect %6 %1011 %44 %33
         %53 = OpAccessChain %23 %20 %2010 %2011
               OpStore %53 %52
         %58 = OpLoad %15 %10
         %63 = OpLoad %6 %11
       %1013 = OpULessThanEqual %100 %63 %72
       %2013 = OpSelect %6 %1013 %63 %72
         %65 = OpAccessChain %64 %62 %21 %2013
       %1014 = OpULessThanEqual %100 %45 %71
       %2014 = OpSelect %6 %1014 %45 %71
       %1015 = OpULessThanEqual %100 %46 %201
       %2015 = OpSelect %15 %1015 %46 %201
        %101 = OpAccessChain %64 %62 %2014 %2015
         %66 = OpLoad %54 %65
       %1016 = OpULessThanEqual %100 %58 %202
       %2016 = OpSelect %15 %1016 %58 %202
         %67 = OpAccessChain %64 %57 %2016
               OpStore %67 %66
         %68 = OpLoad %6 %9
       %1017 = OpULessThanEqual %100 %68 %73
       %2017 = OpSelect %6 %1017 %68 %73
         %70 = OpInBoundsAccessChain %64 %57 %2017
               OpStore %70 %69
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, added_as_livesafe_code, context2.get()));
}

TEST(TransformationAddFunctionTest, LivesafeCanCallLivesafe) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;

  std::vector<protobufs::Instruction> instructions;

  instructions.push_back(MakeInstructionMessage(
      SpvOpFunction, 2, 8,
      {{SPV_OPERAND_TYPE_FUNCTION_CONTROL, {SpvFunctionControlMaskNone}},
       {SPV_OPERAND_TYPE_TYPE_ID, {3}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 9, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpFunctionCall, 2, 11,
                                                {{SPV_OPERAND_TYPE_ID, {6}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpReturn, 0, 0, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpFunctionEnd, 0, 0, {}));

  spvtools::ValidatorOptions validator_options;

  const auto context1 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  const auto context2 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context1(
      MakeUnique<FactManager>(context1.get()), validator_options);
  TransformationContext transformation_context2(
      MakeUnique<FactManager>(context2.get()), validator_options);

  // Mark function 6 as livesafe.
  transformation_context2.GetFactManager()->AddFactFunctionIsLivesafe(6);

  TransformationAddFunction add_dead_function(instructions);
  ASSERT_TRUE(
      add_dead_function.IsApplicable(context1.get(), transformation_context1));
  ApplyAndCheckFreshIds(add_dead_function, context1.get(),
                        &transformation_context1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));
  // The function should not be deemed livesafe
  ASSERT_FALSE(transformation_context1.GetFactManager()->FunctionIsLivesafe(8));
  // All variables/parameters in the function should be deemed irrelevant.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context1.get(), transformation_context1, 8, 0));

  std::string added_as_live_or_dead_code = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
          %8 = OpFunction %2 None %3
          %9 = OpLabel
         %11 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, added_as_live_or_dead_code, context1.get()));

  TransformationAddFunction add_livesafe_function(instructions, 0, 0, {}, 0,
                                                  {});
  ASSERT_TRUE(add_livesafe_function.IsApplicable(context2.get(),
                                                 transformation_context2));
  ApplyAndCheckFreshIds(add_livesafe_function, context2.get(),
                        &transformation_context2);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context2.get(), validator_options, kConsoleMessageConsumer));
  // The function should be deemed livesafe
  ASSERT_TRUE(transformation_context2.GetFactManager()->FunctionIsLivesafe(8));
  // All variables/parameters in the function should be deemed irrelevant.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context2.get(), transformation_context2, 8, 0));
  ASSERT_TRUE(IsEqual(env, added_as_live_or_dead_code, context2.get()));
}

TEST(TransformationAddFunctionTest, LivesafeOnlyCallsLivesafe) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpKill
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;

  std::vector<protobufs::Instruction> instructions;

  instructions.push_back(MakeInstructionMessage(
      SpvOpFunction, 2, 8,
      {{SPV_OPERAND_TYPE_FUNCTION_CONTROL, {SpvFunctionControlMaskNone}},
       {SPV_OPERAND_TYPE_TYPE_ID, {3}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpLabel, 0, 9, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpFunctionCall, 2, 11,
                                                {{SPV_OPERAND_TYPE_ID, {6}}}));
  instructions.push_back(MakeInstructionMessage(SpvOpReturn, 0, 0, {}));
  instructions.push_back(MakeInstructionMessage(SpvOpFunctionEnd, 0, 0, {}));

  spvtools::ValidatorOptions validator_options;

  const auto context1 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  const auto context2 = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context1(
      MakeUnique<FactManager>(context1.get()), validator_options);
  TransformationContext transformation_context2(
      MakeUnique<FactManager>(context2.get()), validator_options);

  TransformationAddFunction add_dead_function(instructions);
  ASSERT_TRUE(
      add_dead_function.IsApplicable(context1.get(), transformation_context1));
  ApplyAndCheckFreshIds(add_dead_function, context1.get(),
                        &transformation_context1);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      context1.get(), validator_options, kConsoleMessageConsumer));
  // The function should not be deemed livesafe
  ASSERT_FALSE(transformation_context1.GetFactManager()->FunctionIsLivesafe(8));
  // All variables/parameters in the function should be deemed irrelevant.
  ASSERT_TRUE(AllVariablesAndParametersExceptLoopLimiterAreIrrelevant(
      context1.get(), transformation_context1, 8, 0));

  std::string added_as_dead_code = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpKill
               OpFunctionEnd
          %8 = OpFunction %2 None %3
          %9 = OpLabel
         %11 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, added_as_dead_code, context1.get()));

  TransformationAddFunction add_livesafe_function(instructions, 0, 0, {}, 0,
                                                  {});
  ASSERT_FALSE(add_livesafe_function.IsApplicable(context2.get(),
                                                  transformation_context2));
}

TEST(TransformationAddFunctionTest,
     LoopLimitersBackEdgeBlockEndsWithConditional1) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpStore %10 %11
               OpBranch %12
         %12 = OpLabel
               OpLoopMerge %14 %15 None
               OpBranch %15
         %15 = OpLabel
         %17 = OpLoad %8 %10
         %20 = OpSLessThan %19 %17 %18
         %21 = OpLoad %8 %10
         %23 = OpIAdd %8 %21 %22
               OpStore %10 %23
               OpBranchConditional %20 %12 %14
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Make a sequence of instruction messages corresponding to function %6 in
  // |donor|.
  std::vector<protobufs::Instruction> instructions =
      GetInstructionsForFunction(env, consumer, donor, 6);

  protobufs::LoopLimiterInfo loop_limiter_info;
  loop_limiter_info.set_loop_header_id(12);
  loop_limiter_info.set_load_id(102);
  loop_limiter_info.set_increment_id(103);
  loop_limiter_info.set_compare_id(104);
  loop_limiter_info.set_logical_op_id(105);
  TransformationAddFunction add_livesafe_function(instructions, 100, 32,
                                                  {loop_limiter_info}, 0, {});
  ASSERT_TRUE(add_livesafe_function.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(add_livesafe_function, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
        %100 = OpVariable %29 Function %30
         %10 = OpVariable %9 Function
               OpStore %10 %11
               OpBranch %12
         %12 = OpLabel
               OpLoopMerge %14 %15 None
               OpBranch %15
         %15 = OpLabel
         %17 = OpLoad %8 %10
         %20 = OpSLessThan %19 %17 %18
         %21 = OpLoad %8 %10
         %23 = OpIAdd %8 %21 %22
               OpStore %10 %23
        %102 = OpLoad %28 %100
        %103 = OpIAdd %28 %102 %31
               OpStore %100 %103
        %104 = OpULessThan %19 %102 %32
        %105 = OpLogicalAnd %19 %20 %104
               OpBranchConditional %105 %12 %14
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected, context.get()));
}

TEST(TransformationAddFunctionTest,
     LoopLimitersBackEdgeBlockEndsWithConditional2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpStore %10 %11
               OpBranch %12
         %12 = OpLabel
               OpLoopMerge %14 %15 None
               OpBranch %15
         %15 = OpLabel
         %17 = OpLoad %8 %10
         %20 = OpSLessThan %19 %17 %18
         %21 = OpLoad %8 %10
         %23 = OpIAdd %8 %21 %22
               OpStore %10 %23
         %50 = OpLogicalNot %19 %20
               OpBranchConditional %50 %14 %12
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Make a sequence of instruction messages corresponding to function %6 in
  // |donor|.
  std::vector<protobufs::Instruction> instructions =
      GetInstructionsForFunction(env, consumer, donor, 6);

  protobufs::LoopLimiterInfo loop_limiter_info;
  loop_limiter_info.set_loop_header_id(12);
  loop_limiter_info.set_load_id(102);
  loop_limiter_info.set_increment_id(103);
  loop_limiter_info.set_compare_id(104);
  loop_limiter_info.set_logical_op_id(105);
  TransformationAddFunction add_livesafe_function(instructions, 100, 32,
                                                  {loop_limiter_info}, 0, {});
  ASSERT_TRUE(add_livesafe_function.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(add_livesafe_function, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
        %100 = OpVariable %29 Function %30
         %10 = OpVariable %9 Function
               OpStore %10 %11
               OpBranch %12
         %12 = OpLabel
               OpLoopMerge %14 %15 None
               OpBranch %15
         %15 = OpLabel
         %17 = OpLoad %8 %10
         %20 = OpSLessThan %19 %17 %18
         %21 = OpLoad %8 %10
         %23 = OpIAdd %8 %21 %22
               OpStore %10 %23
         %50 = OpLogicalNot %19 %20
        %102 = OpLoad %28 %100
        %103 = OpIAdd %28 %102 %31
               OpStore %100 %103
        %104 = OpUGreaterThanEqual %19 %102 %32
        %105 = OpLogicalOr %19 %50 %104
               OpBranchConditional %105 %14 %12
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected, context.get()));
}

TEST(TransformationAddFunctionTest, LoopLimitersHeaderIsBackEdgeBlock) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpStore %10 %11
               OpBranch %12
         %12 = OpLabel
         %17 = OpLoad %8 %10
         %20 = OpSLessThan %19 %17 %18
         %21 = OpLoad %8 %10
         %23 = OpIAdd %8 %21 %22
               OpStore %10 %23
         %50 = OpLogicalNot %19 %20
               OpLoopMerge %14 %12 None
               OpBranchConditional %50 %14 %12
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Make a sequence of instruction messages corresponding to function %6 in
  // |donor|.
  std::vector<protobufs::Instruction> instructions =
      GetInstructionsForFunction(env, consumer, donor, 6);

  protobufs::LoopLimiterInfo loop_limiter_info;
  loop_limiter_info.set_loop_header_id(12);
  loop_limiter_info.set_load_id(102);
  loop_limiter_info.set_increment_id(103);
  loop_limiter_info.set_compare_id(104);
  loop_limiter_info.set_logical_op_id(105);
  TransformationAddFunction add_livesafe_function(instructions, 100, 32,
                                                  {loop_limiter_info}, 0, {});
  ASSERT_TRUE(add_livesafe_function.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(add_livesafe_function, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
        %100 = OpVariable %29 Function %30
         %10 = OpVariable %9 Function
               OpStore %10 %11
               OpBranch %12
         %12 = OpLabel
         %17 = OpLoad %8 %10
         %20 = OpSLessThan %19 %17 %18
         %21 = OpLoad %8 %10
         %23 = OpIAdd %8 %21 %22
               OpStore %10 %23
         %50 = OpLogicalNot %19 %20
        %102 = OpLoad %28 %100
        %103 = OpIAdd %28 %102 %31
               OpStore %100 %103
        %104 = OpUGreaterThanEqual %19 %102 %32
        %105 = OpLogicalOr %19 %50 %104
               OpLoopMerge %14 %12 None
               OpBranchConditional %105 %14 %12
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected, context.get()));
}

TEST(TransformationAddFunctionTest, InfiniteLoop) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
         %22 = OpConstant %8 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpStore %10 %11
               OpBranch %12
         %12 = OpLabel
               OpLoopMerge %14 %12 None
               OpBranch %12
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Make a sequence of instruction messages corresponding to function %6 in
  // |donor|.
  std::vector<protobufs::Instruction> instructions =
      GetInstructionsForFunction(env, consumer, donor, 6);

  protobufs::LoopLimiterInfo loop_limiter_info;
  loop_limiter_info.set_loop_header_id(12);
  loop_limiter_info.set_load_id(102);
  loop_limiter_info.set_increment_id(103);
  loop_limiter_info.set_compare_id(104);
  loop_limiter_info.set_logical_op_id(105);
  TransformationAddFunction add_livesafe_function(instructions, 100, 32,
                                                  {loop_limiter_info}, 0, {});

  // To make sure the loop's merge block is reachable, it must be dominated by
  // the loop header.
  ASSERT_FALSE(add_livesafe_function.IsApplicable(context.get(),
                                                  transformation_context));
}

TEST(TransformationAddFunctionTest, UnreachableContinueConstruct) {
  // This captures the case where the loop's continue construct is statically
  // unreachable.  In this case the loop cannot iterate and so we do not add
  // a loop limiter.  (The reason we do not just add one anyway is that
  // detecting which block would be the back-edge block is difficult in the
  // absence of reliable dominance information.)
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %23 = OpConstant %8 1
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %23 = OpConstant %8 1
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpStore %10 %11
               OpBranch %12
         %12 = OpLabel
               OpLoopMerge %14 %15 None
               OpBranch %16
         %16 = OpLabel
         %17 = OpLoad %8 %10
         %20 = OpSLessThan %19 %17 %18
               OpBranchConditional %20 %13 %14
         %13 = OpLabel
               OpBranch %14
         %15 = OpLabel
         %22 = OpLoad %8 %10
         %24 = OpIAdd %8 %22 %23
               OpStore %10 %24
               OpBranch %12
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Make a sequence of instruction messages corresponding to function %6 in
  // |donor|.
  std::vector<protobufs::Instruction> instructions =
      GetInstructionsForFunction(env, consumer, donor, 6);

  protobufs::LoopLimiterInfo loop_limiter_info;
  loop_limiter_info.set_loop_header_id(12);
  loop_limiter_info.set_load_id(102);
  loop_limiter_info.set_increment_id(103);
  loop_limiter_info.set_compare_id(104);
  loop_limiter_info.set_logical_op_id(105);
  TransformationAddFunction add_livesafe_function(instructions, 100, 32,
                                                  {loop_limiter_info}, 0, {});
  ASSERT_TRUE(add_livesafe_function.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(add_livesafe_function, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %18 = OpConstant %8 10
         %19 = OpTypeBool
         %23 = OpConstant %8 1
         %26 = OpConstantTrue %19
         %27 = OpConstantFalse %19
         %28 = OpTypeInt 32 0
         %29 = OpTypePointer Function %28
         %30 = OpConstant %28 0
         %31 = OpConstant %28 1
         %32 = OpConstant %28 5
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
        %100 = OpVariable %29 Function %30
         %10 = OpVariable %9 Function
               OpStore %10 %11
               OpBranch %12
         %12 = OpLabel
               OpLoopMerge %14 %15 None
               OpBranch %16
         %16 = OpLabel
         %17 = OpLoad %8 %10
         %20 = OpSLessThan %19 %17 %18
               OpBranchConditional %20 %13 %14
         %13 = OpLabel
               OpBranch %14
         %15 = OpLabel
         %22 = OpLoad %8 %10
         %24 = OpIAdd %8 %22 %23
               OpStore %10 %24
               OpBranch %12
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected, context.get()));
}

TEST(TransformationAddFunctionTest, LoopLimitersAndOpPhi1) {
  // This captures the scenario where breaking a loop due to a loop limiter
  // requires patching up OpPhi instructions occurring at the loop merge block.

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
         %50 = OpTypeInt 32 0
         %51 = OpConstant %50 0
         %52 = OpConstant %50 1
         %53 = OpTypePointer Function %50
          %7 = OpTypeFunction %6
         %10 = OpTypePointer Function %6
         %12 = OpConstant %6 0
         %19 = OpConstant %6 100
         %20 = OpTypeBool
         %23 = OpConstant %6 20
         %28 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFunction %6
         %10 = OpTypePointer Function %6
         %12 = OpConstant %6 0
         %19 = OpConstant %6 100
         %20 = OpTypeBool
         %23 = OpConstant %6 20
         %28 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %36 = OpFunctionCall %6 %8
               OpReturn
               OpFunctionEnd
          %8 = OpFunction %6 None %7
          %9 = OpLabel
         %11 = OpVariable %10 Function
               OpStore %11 %12
               OpBranch %13
         %13 = OpLabel
         %37 = OpPhi %6 %12 %9 %32 %16
               OpLoopMerge %15 %16 None
               OpBranch %17
         %17 = OpLabel
         %21 = OpSLessThan %20 %37 %19
               OpBranchConditional %21 %14 %15
         %14 = OpLabel
         %24 = OpSGreaterThan %20 %37 %23
               OpSelectionMerge %26 None
               OpBranchConditional %24 %25 %26
         %25 = OpLabel
         %29 = OpIAdd %6 %37 %28
               OpStore %11 %29
               OpBranch %15
         %26 = OpLabel
               OpBranch %16
         %16 = OpLabel
         %32 = OpIAdd %6 %37 %28
               OpStore %11 %32
               OpBranch %13
         %15 = OpLabel
         %38 = OpPhi %6 %37 %17 %29 %25
               OpReturnValue %38
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Make a sequence of instruction messages corresponding to function %8 in
  // |donor|.
  std::vector<protobufs::Instruction> instructions =
      GetInstructionsForFunction(env, consumer, donor, 8);

  protobufs::LoopLimiterInfo loop_limiter_info;
  loop_limiter_info.set_loop_header_id(13);
  loop_limiter_info.set_load_id(102);
  loop_limiter_info.set_increment_id(103);
  loop_limiter_info.set_compare_id(104);
  loop_limiter_info.set_logical_op_id(105);

  TransformationAddFunction no_op_phi_data(instructions, 100, 28,
                                           {loop_limiter_info}, 0, {});
  // The loop limiter info is not good enough; it does not include ids to patch
  // up the OpPhi at the loop merge.
  ASSERT_FALSE(
      no_op_phi_data.IsApplicable(context.get(), transformation_context));

  // Add a phi id for the new edge from the loop back edge block to the loop
  // merge.
  loop_limiter_info.add_phi_id(28);
  TransformationAddFunction with_op_phi_data(instructions, 100, 28,
                                             {loop_limiter_info}, 0, {});
  ASSERT_TRUE(
      with_op_phi_data.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(with_op_phi_data, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %50 = OpTypeInt 32 0
         %51 = OpConstant %50 0
         %52 = OpConstant %50 1
         %53 = OpTypePointer Function %50
          %7 = OpTypeFunction %6
         %10 = OpTypePointer Function %6
         %12 = OpConstant %6 0
         %19 = OpConstant %6 100
         %20 = OpTypeBool
         %23 = OpConstant %6 20
         %28 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %8 = OpFunction %6 None %7
          %9 = OpLabel
        %100 = OpVariable %53 Function %51
         %11 = OpVariable %10 Function
               OpStore %11 %12
               OpBranch %13
         %13 = OpLabel
         %37 = OpPhi %6 %12 %9 %32 %16
               OpLoopMerge %15 %16 None
               OpBranch %17
         %17 = OpLabel
         %21 = OpSLessThan %20 %37 %19
               OpBranchConditional %21 %14 %15
         %14 = OpLabel
         %24 = OpSGreaterThan %20 %37 %23
               OpSelectionMerge %26 None
               OpBranchConditional %24 %25 %26
         %25 = OpLabel
         %29 = OpIAdd %6 %37 %28
               OpStore %11 %29
               OpBranch %15
         %26 = OpLabel
               OpBranch %16
         %16 = OpLabel
         %32 = OpIAdd %6 %37 %28
               OpStore %11 %32
        %102 = OpLoad %50 %100
        %103 = OpIAdd %50 %102 %52
               OpStore %100 %103
        %104 = OpUGreaterThanEqual %20 %102 %28
               OpBranchConditional %104 %15 %13
         %15 = OpLabel
         %38 = OpPhi %6 %37 %17 %29 %25 %28 %16
               OpReturnValue %38
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected, context.get()));
}

TEST(TransformationAddFunctionTest, LoopLimitersAndOpPhi2) {
  // This captures the scenario where the loop merge block already has an OpPhi
  // with the loop back edge block as a predecessor.

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
         %50 = OpTypeInt 32 0
         %51 = OpConstant %50 0
         %52 = OpConstant %50 1
         %53 = OpTypePointer Function %50
          %7 = OpTypeFunction %6
         %10 = OpTypePointer Function %6
         %12 = OpConstant %6 0
         %19 = OpConstant %6 100
         %20 = OpTypeBool
         %60 = OpConstantTrue %20
         %23 = OpConstant %6 20
         %28 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %50 = OpTypeInt 32 0
         %51 = OpConstant %50 0
         %52 = OpConstant %50 1
         %53 = OpTypePointer Function %50
          %7 = OpTypeFunction %6
         %10 = OpTypePointer Function %6
         %12 = OpConstant %6 0
         %19 = OpConstant %6 100
         %20 = OpTypeBool
         %60 = OpConstantTrue %20
         %23 = OpConstant %6 20
         %28 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %8 = OpFunction %6 None %7
          %9 = OpLabel
         %11 = OpVariable %10 Function
               OpStore %11 %12
               OpBranch %13
         %13 = OpLabel
         %37 = OpPhi %6 %12 %9 %32 %16
               OpLoopMerge %15 %16 None
               OpBranch %17
         %17 = OpLabel
         %21 = OpSLessThan %20 %37 %19
               OpBranchConditional %21 %14 %15
         %14 = OpLabel
         %24 = OpSGreaterThan %20 %37 %23
               OpSelectionMerge %26 None
               OpBranchConditional %24 %25 %26
         %25 = OpLabel
         %29 = OpIAdd %6 %37 %28
               OpStore %11 %29
               OpBranch %15
         %26 = OpLabel
               OpBranch %16
         %16 = OpLabel
         %32 = OpIAdd %6 %37 %28
               OpStore %11 %32
               OpBranchConditional %60 %15 %13
         %15 = OpLabel
         %38 = OpPhi %6 %37 %17 %29 %25 %23 %16
               OpReturnValue %38
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Make a sequence of instruction messages corresponding to function %8 in
  // |donor|.
  std::vector<protobufs::Instruction> instructions =
      GetInstructionsForFunction(env, consumer, donor, 8);

  protobufs::LoopLimiterInfo loop_limiter_info;
  loop_limiter_info.set_loop_header_id(13);
  loop_limiter_info.set_load_id(102);
  loop_limiter_info.set_increment_id(103);
  loop_limiter_info.set_compare_id(104);
  loop_limiter_info.set_logical_op_id(105);

  TransformationAddFunction transformation(instructions, 100, 28,
                                           {loop_limiter_info}, 0, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %50 = OpTypeInt 32 0
         %51 = OpConstant %50 0
         %52 = OpConstant %50 1
         %53 = OpTypePointer Function %50
          %7 = OpTypeFunction %6
         %10 = OpTypePointer Function %6
         %12 = OpConstant %6 0
         %19 = OpConstant %6 100
         %20 = OpTypeBool
         %60 = OpConstantTrue %20
         %23 = OpConstant %6 20
         %28 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %8 = OpFunction %6 None %7
          %9 = OpLabel
        %100 = OpVariable %53 Function %51
         %11 = OpVariable %10 Function
               OpStore %11 %12
               OpBranch %13
         %13 = OpLabel
         %37 = OpPhi %6 %12 %9 %32 %16
               OpLoopMerge %15 %16 None
               OpBranch %17
         %17 = OpLabel
         %21 = OpSLessThan %20 %37 %19
               OpBranchConditional %21 %14 %15
         %14 = OpLabel
         %24 = OpSGreaterThan %20 %37 %23
               OpSelectionMerge %26 None
               OpBranchConditional %24 %25 %26
         %25 = OpLabel
         %29 = OpIAdd %6 %37 %28
               OpStore %11 %29
               OpBranch %15
         %26 = OpLabel
               OpBranch %16
         %16 = OpLabel
         %32 = OpIAdd %6 %37 %28
               OpStore %11 %32
        %102 = OpLoad %50 %100
        %103 = OpIAdd %50 %102 %52
               OpStore %100 %103
        %104 = OpUGreaterThanEqual %20 %102 %28
        %105 = OpLogicalOr %20 %60 %104
               OpBranchConditional %105 %15 %13
         %15 = OpLabel
         %38 = OpPhi %6 %37 %17 %29 %25 %23 %16
               OpReturnValue %38
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected, context.get()));
}

TEST(TransformationAddFunctionTest, StaticallyOutOfBoundsArrayAccess) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypeInt 32 0
         %10 = OpConstant %9 3
         %11 = OpTypeArray %8 %10
         %12 = OpTypePointer Private %11
         %13 = OpVariable %12 Private
         %14 = OpConstant %8 3
         %20 = OpConstant %8 2
         %15 = OpConstant %8 1
         %21 = OpTypeBool
         %16 = OpTypePointer Private %8
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypeInt 32 0
         %10 = OpConstant %9 3
         %11 = OpTypeArray %8 %10
         %12 = OpTypePointer Private %11
         %13 = OpVariable %12 Private
         %14 = OpConstant %8 3
         %15 = OpConstant %8 1
         %16 = OpTypePointer Private %8
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %17 = OpAccessChain %16 %13 %14
               OpStore %17 %15
               OpReturn
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Make a sequence of instruction messages corresponding to function %6 in
  // |donor|.
  std::vector<protobufs::Instruction> instructions =
      GetInstructionsForFunction(env, consumer, donor, 6);

  TransformationAddFunction add_livesafe_function(
      instructions, 0, 0, {}, 0, {MakeAccessClampingInfo(17, {{100, 101}})});
  ASSERT_TRUE(add_livesafe_function.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(add_livesafe_function, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypeInt 32 0
         %10 = OpConstant %9 3
         %11 = OpTypeArray %8 %10
         %12 = OpTypePointer Private %11
         %13 = OpVariable %12 Private
         %14 = OpConstant %8 3
         %20 = OpConstant %8 2
         %15 = OpConstant %8 1
         %21 = OpTypeBool
         %16 = OpTypePointer Private %8
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
        %100 = OpULessThanEqual %21 %14 %20
        %101 = OpSelect %8 %100 %14 %20
         %17 = OpAccessChain %16 %13 %101
               OpStore %17 %15
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
