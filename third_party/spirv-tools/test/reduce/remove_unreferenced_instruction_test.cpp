// Copyright (c) 2018 Google LLC
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

#include "source/reduce/remove_unreferenced_instruction_reduction_opportunity_finder.h"

#include "source/opt/build_module.h"
#include "source/reduce/reduction_opportunity.h"
#include "source/util/make_unique.h"
#include "test/reduce/reduce_test_util.h"

namespace spvtools {
namespace reduce {
namespace {

const spv_target_env kEnv = SPV_ENV_UNIVERSAL_1_3;

TEST(RemoveUnreferencedInstructionReductionPassTest, RemoveStores) {
  // A module with some unused instructions, including some unused OpStore
  // instructions.

  RemoveUnreferencedInstructionReductionOpportunityFinder finder(true);

  const std::string original = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310  ; 0
               OpName %4 "main"   ; 1
               OpName %8 "a"      ; 2
               OpName %10 "b"     ; 3
               OpName %12 "c"     ; 4
               OpName %14 "d"     ; 5
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 10
         %11 = OpConstant %6 20
         %13 = OpConstant %6 30
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %14 = OpVariable %7 Function
               OpStore %8 %9           ; 6
               OpStore %10 %11         ; 7
               OpStore %12 %13         ; 8
         %15 = OpLoad %6 %8
               OpStore %14 %15         ; 9
               OpReturn
               OpFunctionEnd

  )";

  const MessageConsumer consumer = nullptr;
  const auto context =
      BuildModule(kEnv, consumer, original, kReduceAssembleOption);

  CheckValid(kEnv, context.get());

  auto ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(10, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
    CheckValid(kEnv, context.get());
  }

  const std::string step_2 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 10       ; 0
         %11 = OpConstant %6 20       ; 1
         %13 = OpConstant %6 30       ; 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function ; 3
         %12 = OpVariable %7 Function ; 4
         %14 = OpVariable %7 Function ; 5
         %15 = OpLoad %6 %8           ; 6
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(kEnv, step_2, context.get());

  ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(7, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
    CheckValid(kEnv, context.get());
  }

  const std::string step_3 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function   ; 0
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(kEnv, step_3, context.get());

  ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(1, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
    CheckValid(kEnv, context.get());
  }

  const std::string step_4 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6  ; 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(kEnv, step_4, context.get());

  ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(1, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
    CheckValid(kEnv, context.get());
  }

  const std::string step_5 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1        ; 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(kEnv, step_5, context.get());

  ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(1, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
    CheckValid(kEnv, context.get());
  }

  const std::string step_6 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(kEnv, step_6, context.get());

  ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(0, ops.size());
}

TEST(RemoveUnreferencedInstructionReductionPassTest, Referenced) {
  // A module with some unused global variables, constants, and types. Some will
  // not be removed initially because of the OpDecorate instructions.

  RemoveUnreferencedInstructionReductionOpportunityFinder finder(true);

  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310                 ; 1
               OpName %4 "main"                  ; 2
               OpName %12 "a"                    ; 3
               OpDecorate %12 RelaxedPrecision   ; 4
               OpDecorate %13 RelaxedPrecision   ; 5
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6                 ; 6
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Private %10
         %12 = OpVariable %11 Private
         %13 = OpConstant %10 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  auto context = BuildModule(kEnv, nullptr, shader, kReduceAssembleOption);

  CheckValid(kEnv, context.get());

  auto ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(6, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
    CheckValid(kEnv, context.get());
  }

  std::string after = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool                 ; 1
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Private %10
         %12 = OpVariable %11 Private     ; 2
         %13 = OpConstant %10 1           ; 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

  CheckEqual(kEnv, after, context.get());

  ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(3, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
    CheckValid(kEnv, context.get());
  }

  std::string after_2 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Private %10   ; 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

  CheckEqual(kEnv, after_2, context.get());

  ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(1, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
    CheckValid(kEnv, context.get());
  }

  std::string after_3 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeInt 32 1          ; 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

  CheckEqual(kEnv, after_3, context.get());

  ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(1, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
    CheckValid(kEnv, context.get());
  }

  std::string after_4 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

  CheckEqual(kEnv, after_4, context.get());

  ops = finder.GetAvailableOpportunities(context.get());

  ASSERT_EQ(0, ops.size());
}

}  // namespace
}  // namespace reduce
}  // namespace spvtools
