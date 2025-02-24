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

#include "source/reduce/remove_unused_instruction_reduction_opportunity_finder.h"

#include "source/opt/build_module.h"
#include "source/reduce/reduction_opportunity.h"
#include "source/util/make_unique.h"
#include "test/reduce/reduce_test_util.h"

namespace spvtools {
namespace reduce {
namespace {

const spv_target_env kEnv = SPV_ENV_UNIVERSAL_1_3;

TEST(RemoveUnusedInstructionReductionPassTest, RemoveStores) {
  // A module with some unused instructions, including some unused OpStore
  // instructions.

  RemoveUnusedInstructionReductionOpportunityFinder finder(true);

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

  auto ops = finder.GetAvailableOpportunities(context.get(), 0);

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

  ops = finder.GetAvailableOpportunities(context.get(), 0);

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

  ops = finder.GetAvailableOpportunities(context.get(), 0);

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

  ops = finder.GetAvailableOpportunities(context.get(), 0);

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

  ops = finder.GetAvailableOpportunities(context.get(), 0);

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

  ops = finder.GetAvailableOpportunities(context.get(), 0);

  ASSERT_EQ(0, ops.size());
}

TEST(RemoveUnusedInstructionReductionPassTest, Referenced) {
  // A module with some unused global variables, constants, and types. Some will
  // not be removed initially because of the OpDecorate instructions.

  RemoveUnusedInstructionReductionOpportunityFinder finder(true);

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

  auto ops = finder.GetAvailableOpportunities(context.get(), 0);

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

  ops = finder.GetAvailableOpportunities(context.get(), 0);

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

  ops = finder.GetAvailableOpportunities(context.get(), 0);

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

  ops = finder.GetAvailableOpportunities(context.get(), 0);

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

  ops = finder.GetAvailableOpportunities(context.get(), 0);

  ASSERT_EQ(0, ops.size());
}

TEST(RemoveUnusedResourceVariableTest, RemoveUnusedResourceVariables) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 1
               OpMemberDecorate %16 0 Offset 0
               OpMemberDecorate %16 1 Offset 4
               OpDecorate %16 Block
               OpDecorate %18 DescriptorSet 0
               OpDecorate %18 Binding 0
               OpMemberDecorate %19 0 Offset 0
               OpDecorate %19 BufferBlock
               OpDecorate %21 DescriptorSet 1
               OpDecorate %21 Binding 0
               OpMemberDecorate %22 0 Offset 0
               OpDecorate %22 Block
               OpDecorate %29 DescriptorSet 1
               OpDecorate %29 Binding 1
               OpDecorate %32 DescriptorSet 1
               OpDecorate %32 Binding 2
               OpDecorate %32 NonReadable
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %9 = OpTypeStruct %6
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %13 = OpTypePointer Uniform %6
         %16 = OpTypeStruct %6 %6
         %17 = OpTypePointer Uniform %16
         %18 = OpVariable %17 Uniform
         %19 = OpTypeStruct %6
         %20 = OpTypePointer Uniform %19
         %21 = OpVariable %20 Uniform
         %22 = OpTypeStruct %6
         %23 = OpTypePointer PushConstant %22
         %24 = OpVariable %23 PushConstant
         %25 = OpTypeFloat 32
         %26 = OpTypeImage %25 2D 0 0 0 1 Unknown
         %27 = OpTypeSampledImage %26
         %28 = OpTypePointer UniformConstant %27
         %29 = OpVariable %28 UniformConstant
         %30 = OpTypeImage %25 2D 0 0 0 2 Unknown
         %31 = OpTypePointer UniformConstant %30
         %32 = OpVariable %31 UniformConstant
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, shader, kReduceAssembleOption);

  auto ops = RemoveUnusedInstructionReductionOpportunityFinder(true)
                 .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(7, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
  }

  std::string expected_1 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 Block
               OpMemberDecorate %16 0 Offset 0
               OpMemberDecorate %16 1 Offset 4
               OpDecorate %16 Block
               OpMemberDecorate %19 0 Offset 0
               OpDecorate %19 BufferBlock
               OpMemberDecorate %22 0 Offset 0
               OpDecorate %22 Block
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %9 = OpTypeStruct %6
         %10 = OpTypePointer Uniform %9
         %16 = OpTypeStruct %6 %6
         %17 = OpTypePointer Uniform %16
         %19 = OpTypeStruct %6
         %20 = OpTypePointer Uniform %19
         %22 = OpTypeStruct %6
         %23 = OpTypePointer PushConstant %22
         %25 = OpTypeFloat 32
         %26 = OpTypeImage %25 2D 0 0 0 1 Unknown
         %27 = OpTypeSampledImage %26
         %28 = OpTypePointer UniformConstant %27
         %30 = OpTypeImage %25 2D 0 0 0 2 Unknown
         %31 = OpTypePointer UniformConstant %30
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(env, expected_1, context.get());

  ops = RemoveUnusedInstructionReductionOpportunityFinder(true)
            .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(6, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
  }

  std::string expected_2 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 Block
               OpMemberDecorate %16 0 Offset 0
               OpMemberDecorate %16 1 Offset 4
               OpDecorate %16 Block
               OpMemberDecorate %19 0 Offset 0
               OpDecorate %19 BufferBlock
               OpMemberDecorate %22 0 Offset 0
               OpDecorate %22 Block
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %9 = OpTypeStruct %6
         %16 = OpTypeStruct %6 %6
         %19 = OpTypeStruct %6
         %22 = OpTypeStruct %6
         %25 = OpTypeFloat 32
         %26 = OpTypeImage %25 2D 0 0 0 1 Unknown
         %27 = OpTypeSampledImage %26
         %30 = OpTypeImage %25 2D 0 0 0 2 Unknown
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(env, expected_2, context.get());

  ops = RemoveUnusedInstructionReductionOpportunityFinder(true)
            .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(6, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
  }

  std::string expected_3 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %25 = OpTypeFloat 32
         %26 = OpTypeImage %25 2D 0 0 0 1 Unknown
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(env, expected_3, context.get());
}

}  // namespace
}  // namespace reduce
}  // namespace spvtools
