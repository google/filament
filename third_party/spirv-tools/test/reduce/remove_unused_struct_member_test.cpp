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

#include "source/reduce/remove_unused_struct_member_reduction_opportunity_finder.h"

#include "source/opt/build_module.h"
#include "source/reduce/reduction_opportunity.h"
#include "test/reduce/reduce_test_util.h"

namespace spvtools {
namespace reduce {
namespace {

TEST(RemoveUnusedStructMemberTest, RemoveOneMember) {
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
          %7 = OpTypeStruct %6 %6
          %8 = OpTypePointer Function %7
         %50 = OpConstant %6 0
         %10 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpConstantComposite %7 %10 %11
         %13 = OpConstant %6 4
         %14 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
               OpStore %9 %12
         %15 = OpAccessChain %14 %9 %10
         %22 = OpInBoundsAccessChain %14 %9 %10
         %20 = OpLoad %7 %9
         %21 = OpCompositeExtract %6 %20 1
         %23 = OpCompositeInsert %7 %10 %20 1
               OpStore %15 %13
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, shader, kReduceAssembleOption);

  auto ops = RemoveUnusedStructMemberReductionOpportunityFinder()
                 .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(1, ops.size());
  ASSERT_TRUE(ops[0]->PreconditionHolds());
  ops[0]->TryToApply();

  CheckValid(env, context.get());

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
          %7 = OpTypeStruct %6
          %8 = OpTypePointer Function %7
         %50 = OpConstant %6 0
         %10 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpConstantComposite %7 %11
         %13 = OpConstant %6 4
         %14 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
               OpStore %9 %12
         %15 = OpAccessChain %14 %9 %50
         %22 = OpInBoundsAccessChain %14 %9 %50
         %20 = OpLoad %7 %9
         %21 = OpCompositeExtract %6 %20 0
         %23 = OpCompositeInsert %7 %10 %20 0
               OpStore %15 %13
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(env, expected, context.get());
}

TEST(RemoveUnusedStructMemberTest, RemoveUniformBufferMember) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpMemberDecorate %10 0 Offset 0
               OpMemberDecorate %10 1 Offset 4
               OpDecorate %10 Block
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpTypeInt 32 1
         %10 = OpTypeStruct %9 %6
         %11 = OpTypePointer Uniform %10
         %12 = OpVariable %11 Uniform
         %13 = OpConstant %9 1
         %20 = OpConstant %9 0
         %14 = OpTypePointer Uniform %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %15 = OpAccessChain %14 %12 %13
         %16 = OpLoad %6 %15
               OpStore %8 %16
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, shader, kReduceAssembleOption);

  auto ops = RemoveUnusedStructMemberReductionOpportunityFinder()
                 .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(1, ops.size());
  ASSERT_TRUE(ops[0]->PreconditionHolds());
  ops[0]->TryToApply();

  CheckValid(env, context.get());

  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpMemberDecorate %10 0 Offset 4
               OpDecorate %10 Block
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpTypeInt 32 1
         %10 = OpTypeStruct %6
         %11 = OpTypePointer Uniform %10
         %12 = OpVariable %11 Uniform
         %13 = OpConstant %9 1
         %20 = OpConstant %9 0
         %14 = OpTypePointer Uniform %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %15 = OpAccessChain %14 %12 %20
         %16 = OpLoad %6 %15
               OpStore %8 %16
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(env, expected, context.get());
}

TEST(RemoveUnusedStructMemberTest, DoNotRemoveNamedMemberRemoveOneMember) {
  // This illustrates that naming a member is enough to prevent its removal.
  // Removal of names is done by a different pass.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpMemberName %7 0 "someName"
               OpMemberName %7 1 "someOtherName"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeStruct %6 %6
          %8 = OpTypePointer Function %7
         %50 = OpConstant %6 0
         %10 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpConstantComposite %7 %10 %11
         %13 = OpConstant %6 4
         %14 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
               OpStore %9 %12
         %15 = OpAccessChain %14 %9 %10
         %22 = OpInBoundsAccessChain %14 %9 %10
         %20 = OpLoad %7 %9
         %21 = OpCompositeExtract %6 %20 1
         %23 = OpCompositeInsert %7 %10 %20 1
               OpStore %15 %13
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, shader, kReduceAssembleOption);

  auto ops = RemoveUnusedStructMemberReductionOpportunityFinder()
                 .GetAvailableOpportunities(context.get(), 0);
  ASSERT_EQ(0, ops.size());
}

}  // namespace
}  // namespace reduce
}  // namespace spvtools
