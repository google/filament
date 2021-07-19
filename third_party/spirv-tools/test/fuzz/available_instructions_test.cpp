// Copyright (c) 2021 Alastair F. Donaldson
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

#include "source/fuzz/available_instructions.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(AvailableInstructionsTest, BasicTest) {
  std::string shader = R"(
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
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %7 %9
         %15 = OpTypeVector %8 2
         %16 = OpTypePointer Private %15
         %17 = OpVariable %16 Private
         %18 = OpConstant %8 1
         %19 = OpConstant %8 2
         %20 = OpConstantComposite %15 %18 %19
         %21 = OpTypeVector %8 4
         %22 = OpTypePointer Private %21
         %23 = OpVariable %22 Private
         %24 = OpConstant %8 10
         %25 = OpConstant %8 20
         %26 = OpConstant %8 30
         %27 = OpConstant %8 40
         %28 = OpConstantComposite %21 %24 %25 %26 %27
         %31 = OpTypeInt 32 0
         %32 = OpConstant %31 0
         %33 = OpTypePointer Private %8
         %41 = OpTypeBool
         %46 = OpConstant %6 1
         %54 = OpConstant %6 10
         %57 = OpConstant %31 3
         %61 = OpConstant %6 0
         %66 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %55 = OpVariable %7 Function
         %56 = OpVariable %9 Function
         %65 = OpVariable %7 Function
         %68 = OpVariable %7 Function
               OpStore %17 %20
               OpStore %23 %28
               OpStore %55 %54
         %58 = OpAccessChain %33 %23 %57
         %59 = OpLoad %8 %58
               OpStore %56 %59
         %60 = OpFunctionCall %6 %13 %55 %56
        %100 = OpCopyObject %21 %28
         %62 = OpSGreaterThan %41 %60 %61
               OpSelectionMerge %64 None
               OpBranchConditional %62 %63 %67
         %63 = OpLabel
               OpStore %65 %66
        %101 = OpCopyObject %21 %28
               OpBranch %64
         %67 = OpLabel
               OpStore %68 %61
               OpBranch %69
         %69 = OpLabel
               OpLoopMerge %71 %72 None
               OpBranch %73
         %73 = OpLabel
         %74 = OpLoad %6 %68
         %75 = OpSLessThan %41 %74 %54
               OpBranchConditional %75 %70 %71
         %70 = OpLabel
         %76 = OpLoad %6 %65
         %77 = OpIAdd %6 %76 %46
               OpStore %65 %77
               OpBranch %72
         %72 = OpLabel
         %78 = OpLoad %6 %68
         %79 = OpIAdd %6 %78 %46
               OpStore %68 %79
               OpBranch %69
         %71 = OpLabel
        %102 = OpCopyObject %21 %28
               OpBranch %64
         %64 = OpLabel
               OpReturn
               OpFunctionEnd
         %13 = OpFunction %6 None %10
         %11 = OpFunctionParameter %7
         %12 = OpFunctionParameter %9
         %14 = OpLabel
         %29 = OpVariable %7 Function
         %30 = OpLoad %6 %11
         %34 = OpAccessChain %33 %17 %32
         %35 = OpLoad %8 %34
         %36 = OpConvertFToS %6 %35
         %37 = OpIAdd %6 %30 %36
               OpStore %29 %37
         %38 = OpLoad %6 %11
         %39 = OpLoad %8 %12
         %40 = OpConvertFToS %6 %39
         %42 = OpSLessThan %41 %38 %40
        %103 = OpCopyObject %21 %28
               OpSelectionMerge %44 None
               OpBranchConditional %42 %43 %48
         %43 = OpLabel
         %45 = OpLoad %6 %29
         %47 = OpIAdd %6 %45 %46
               OpStore %29 %47
               OpBranch %44
         %48 = OpLabel
         %49 = OpLoad %6 %29
         %50 = OpISub %6 %49 %46
               OpStore %29 %50
               OpBranch %44
         %44 = OpLabel
         %51 = OpLoad %6 %29
               OpReturnValue %51
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  opt::Instruction* i1 = context->get_def_use_mgr()->GetDef(55);
  opt::Instruction* i2 = context->get_def_use_mgr()->GetDef(101);
  opt::Instruction* i3 = &*context->cfg()->block(67)->begin();
  opt::Instruction* i4 = context->get_def_use_mgr()->GetDef(74);
  opt::Instruction* i5 = context->get_def_use_mgr()->GetDef(102);
  opt::Instruction* i6 = context->get_def_use_mgr()->GetDef(30);
  opt::Instruction* i7 = context->get_def_use_mgr()->GetDef(47);
  opt::Instruction* i8 = context->get_def_use_mgr()->GetDef(50);
  opt::Instruction* i9 = context->get_def_use_mgr()->GetDef(51);

  {
    AvailableInstructions no_instructions(
        context.get(),
        [](opt::IRContext*, opt::Instruction*) -> bool { return false; });
    for (auto i : {i1, i2, i3, i4, i5, i6, i7, i8, i9}) {
      auto available = no_instructions.GetAvailableBeforeInstruction(i);
      ASSERT_EQ(0, available.size());
      ASSERT_TRUE(available.empty());
    }
  }
  {
    AvailableInstructions all_instructions(
        context.get(),
        [](opt::IRContext*, opt::Instruction*) -> bool { return true; });
    {
      auto available = all_instructions.GetAvailableBeforeInstruction(i1);
      ASSERT_FALSE(available.empty());
      ASSERT_EQ(30, available.size());
      ASSERT_EQ(SpvOpTypeVoid, available[0]->opcode());
      ASSERT_EQ(SpvOpVariable, available[15]->opcode());
    }
    {
      auto available = all_instructions.GetAvailableBeforeInstruction(i2);
      ASSERT_FALSE(available.empty());
      ASSERT_EQ(46, available.size());
      ASSERT_EQ(SpvOpTypeVoid, available[0]->opcode());
      ASSERT_EQ(SpvOpTypePointer, available[3]->opcode());
      ASSERT_EQ(SpvOpVariable, available[15]->opcode());
      ASSERT_EQ(SpvOpFunctionCall, available[40]->opcode());
      ASSERT_EQ(SpvOpStore, available[45]->opcode());
    }
    {
      auto available = all_instructions.GetAvailableBeforeInstruction(i3);
      ASSERT_FALSE(available.empty());
      ASSERT_EQ(45, available.size());
      ASSERT_EQ(SpvOpTypeVoid, available[0]->opcode());
      ASSERT_EQ(SpvOpTypePointer, available[3]->opcode());
      ASSERT_EQ(SpvOpVariable, available[15]->opcode());
      ASSERT_EQ(SpvOpFunctionCall, available[40]->opcode());
      ASSERT_EQ(SpvOpBranchConditional, available[44]->opcode());
    }
    {
      auto available = all_instructions.GetAvailableBeforeInstruction(i6);
      ASSERT_FALSE(available.empty());
      ASSERT_EQ(33, available.size());
      ASSERT_EQ(SpvOpTypeVoid, available[0]->opcode());
      ASSERT_EQ(SpvOpTypeFloat, available[4]->opcode());
      ASSERT_EQ(SpvOpTypePointer, available[8]->opcode());
      ASSERT_EQ(SpvOpConstantComposite, available[12]->opcode());
      ASSERT_EQ(SpvOpConstant, available[16]->opcode());
      ASSERT_EQ(SpvOpFunctionParameter, available[30]->opcode());
      ASSERT_EQ(SpvOpFunctionParameter, available[31]->opcode());
      ASSERT_EQ(SpvOpVariable, available[32]->opcode());
    }
  }
  {
    AvailableInstructions vector_instructions(
        context.get(),
        [](opt::IRContext* ir_context, opt::Instruction* inst) -> bool {
          return inst->type_id() != 0 && ir_context->get_type_mgr()
                                                 ->GetType(inst->type_id())
                                                 ->AsVector() != nullptr;
        });
    {
      auto available = vector_instructions.GetAvailableBeforeInstruction(i4);
      ASSERT_FALSE(available.empty());
      ASSERT_EQ(3, available.size());
      ASSERT_EQ(SpvOpConstantComposite, available[0]->opcode());
      ASSERT_EQ(SpvOpConstantComposite, available[1]->opcode());
      ASSERT_EQ(SpvOpCopyObject, available[2]->opcode());
    }
    {
      auto available = vector_instructions.GetAvailableBeforeInstruction(i5);
      ASSERT_FALSE(available.empty());
      ASSERT_EQ(3, available.size());
      ASSERT_EQ(SpvOpConstantComposite, available[0]->opcode());
      ASSERT_EQ(SpvOpConstantComposite, available[1]->opcode());
      ASSERT_EQ(SpvOpCopyObject, available[2]->opcode());
    }
    {
      auto available = vector_instructions.GetAvailableBeforeInstruction(i6);
      ASSERT_FALSE(available.empty());
      ASSERT_EQ(2, available.size());
      ASSERT_EQ(SpvOpConstantComposite, available[0]->opcode());
      ASSERT_EQ(SpvOpConstantComposite, available[1]->opcode());
    }
  }
  {
    AvailableInstructions integer_add_instructions(
        context.get(), [](opt::IRContext*, opt::Instruction* inst) -> bool {
          return inst->opcode() == SpvOpIAdd;
        });
    {
      auto available =
          integer_add_instructions.GetAvailableBeforeInstruction(i7);
      ASSERT_FALSE(available.empty());
      ASSERT_EQ(1, available.size());
      ASSERT_EQ(SpvOpIAdd, available[0]->opcode());
    }
    {
      auto available =
          integer_add_instructions.GetAvailableBeforeInstruction(i8);
      ASSERT_FALSE(available.empty());
      ASSERT_EQ(1, available.size());
      ASSERT_EQ(SpvOpIAdd, available[0]->opcode());
    }
    {
      auto available =
          integer_add_instructions.GetAvailableBeforeInstruction(i9);
      ASSERT_FALSE(available.empty());
      ASSERT_EQ(1, available.size());
      ASSERT_EQ(SpvOpIAdd, available[0]->opcode());
    }
  }
}

TEST(AvailableInstructionsTest, UnreachableBlock) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
               OpName %8 "x"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
         %12 = OpLoad %6 %8
               OpReturn
         %10 = OpLabel
         %11 = OpLoad %6 %8
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  AvailableInstructions all_instructions(
      context.get(),
      [](opt::IRContext*, opt::Instruction*) -> bool { return true; });
  ASSERT_EQ(7, all_instructions
                   .GetAvailableBeforeInstruction(
                       context->get_def_use_mgr()->GetDef(12))
                   .size());

#ifndef NDEBUG
  ASSERT_DEATH(all_instructions.GetAvailableBeforeInstruction(
                   context->get_def_use_mgr()->GetDef(11)),
               "Availability can only be queried for reachable instructions.");
#endif
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
