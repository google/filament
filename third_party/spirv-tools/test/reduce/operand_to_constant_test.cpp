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

#include "source/reduce/operand_to_const_reduction_opportunity_finder.h"

#include "source/opt/build_module.h"
#include "source/reduce/reduction_opportunity.h"
#include "test/reduce/reduce_test_util.h"

namespace spvtools {
namespace reduce {
namespace {

TEST(OperandToConstantReductionPassTest, BasicCheck) {
  std::string prologue = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %37
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "buf1"
               OpMemberName %9 0 "f"
               OpName %11 ""
               OpName %24 "buf2"
               OpMemberName %24 0 "i"
               OpName %26 ""
               OpName %37 "_GLF_color"
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 1
               OpMemberDecorate %24 0 Offset 0
               OpDecorate %24 Block
               OpDecorate %26 DescriptorSet 0
               OpDecorate %26 Binding 2
               OpDecorate %37 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %9 = OpTypeStruct %6
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %12 = OpTypeInt 32 1
         %13 = OpConstant %12 0
         %14 = OpTypePointer Uniform %6
         %20 = OpConstant %6 2
         %24 = OpTypeStruct %12
         %25 = OpTypePointer Uniform %24
         %26 = OpVariable %25 Uniform
         %27 = OpTypePointer Uniform %12
         %33 = OpConstant %12 3
         %35 = OpTypeVector %6 4
         %36 = OpTypePointer Output %35
         %37 = OpVariable %36 Output
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %15 = OpAccessChain %14 %11 %13
         %16 = OpLoad %6 %15
         %19 = OpFAdd %6 %16 %16
         %21 = OpFAdd %6 %19 %20
         %28 = OpAccessChain %27 %26 %13
         %29 = OpLoad %12 %28
  )";

  std::string epilogue = R"(
         %45 = OpConvertSToF %6 %34
         %46 = OpCompositeConstruct %35 %16 %21 %43 %45
               OpStore %37 %46
               OpReturn
               OpFunctionEnd
  )";

  std::string original = prologue + R"(
         %32 = OpIAdd %12 %29 %29
         %34 = OpIAdd %12 %32 %33
         %43 = OpConvertSToF %6 %29
  )" + epilogue;

  std::string expected = prologue + R"(
         %32 = OpIAdd %12 %13 %13 ; %29 -> %13 x 2
         %34 = OpIAdd %12 %13 %33 ; %32 -> %13
         %43 = OpConvertSToF %6 %13 ; %29 -> %13
  )" + epilogue;

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, original, kReduceAssembleOption);
  const auto ops =
      OperandToConstReductionOpportunityFinder().GetAvailableOpportunities(
          context.get(), 0);
  ASSERT_EQ(17, ops.size());
  ASSERT_TRUE(ops[0]->PreconditionHolds());
  ops[0]->TryToApply();
  ASSERT_TRUE(ops[1]->PreconditionHolds());
  ops[1]->TryToApply();
  ASSERT_TRUE(ops[2]->PreconditionHolds());
  ops[2]->TryToApply();
  ASSERT_TRUE(ops[3]->PreconditionHolds());
  ops[3]->TryToApply();

  CheckEqual(env, expected, context.get());
}

TEST(OperandToConstantReductionPassTest, WithCalledFunction) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %10 %12
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypeFunction %7
          %9 = OpTypePointer Output %7
         %10 = OpVariable %9 Output
         %11 = OpTypePointer Input %7
         %12 = OpVariable %11 Input
         %13 = OpConstant %6 0
         %14 = OpConstantComposite %7 %13 %13 %13 %13
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %15 = OpFunctionCall %7 %16
               OpReturn
               OpFunctionEnd
         %16 = OpFunction %7 None %8
         %17 = OpLabel
               OpReturnValue %14
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, shader, kReduceAssembleOption);
  const auto ops =
      OperandToConstReductionOpportunityFinder().GetAvailableOpportunities(
          context.get(), 0);
  ASSERT_EQ(0, ops.size());
}

TEST(OperandToConstantReductionPassTest, TargetSpecificFunction) {
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
          %8 = OpTypeFunction %6 %7
         %17 = OpConstant %6 1
         %20 = OpConstant %6 2
         %23 = OpConstant %6 0
         %24 = OpTypeBool
         %35 = OpConstant %6 3
         %53 = OpConstant %6 10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %65 = OpVariable %7 Function
         %68 = OpVariable %7 Function
         %73 = OpVariable %7 Function
               OpStore %65 %35
         %66 = OpLoad %6 %65
         %67 = OpIAdd %6 %66 %17
               OpStore %65 %67
         %69 = OpLoad %6 %65
               OpStore %68 %69
         %70 = OpFunctionCall %6 %13 %68
         %71 = OpLoad %6 %65
         %72 = OpIAdd %6 %71 %70
               OpStore %65 %72
         %74 = OpLoad %6 %65
               OpStore %73 %74
         %75 = OpFunctionCall %6 %10 %73
         %76 = OpLoad %6 %65
         %77 = OpIAdd %6 %76 %75
               OpStore %65 %77
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %15 = OpVariable %7 Function
         %16 = OpLoad %6 %9
         %18 = OpIAdd %6 %16 %17
               OpStore %15 %18
         %19 = OpLoad %6 %15
         %21 = OpIAdd %6 %19 %20
               OpStore %15 %21
         %22 = OpLoad %6 %15
         %25 = OpSGreaterThan %24 %22 %23
               OpSelectionMerge %27 None
               OpBranchConditional %25 %26 %27
         %26 = OpLabel
         %28 = OpLoad %6 %9
               OpReturnValue %28
         %27 = OpLabel
         %30 = OpLoad %6 %9
         %31 = OpIAdd %6 %30 %17
               OpReturnValue %31
               OpFunctionEnd
         %13 = OpFunction %6 None %8
         %12 = OpFunctionParameter %7
         %14 = OpLabel
         %41 = OpVariable %7 Function
         %46 = OpVariable %7 Function
         %55 = OpVariable %7 Function
         %34 = OpLoad %6 %12
         %36 = OpIEqual %24 %34 %35
               OpSelectionMerge %38 None
               OpBranchConditional %36 %37 %38
         %37 = OpLabel
         %39 = OpLoad %6 %12
         %40 = OpIMul %6 %20 %39
               OpStore %41 %40
         %42 = OpFunctionCall %6 %10 %41
               OpReturnValue %42
         %38 = OpLabel
         %44 = OpLoad %6 %12
         %45 = OpIAdd %6 %44 %17
               OpStore %12 %45
               OpStore %46 %23
               OpBranch %47
         %47 = OpLabel
               OpLoopMerge %49 %50 None
               OpBranch %51
         %51 = OpLabel
         %52 = OpLoad %6 %46
         %54 = OpSLessThan %24 %52 %53
               OpBranchConditional %54 %48 %49
         %48 = OpLabel
         %56 = OpLoad %6 %12
               OpStore %55 %56
         %57 = OpFunctionCall %6 %10 %55
         %58 = OpLoad %6 %12
         %59 = OpIAdd %6 %58 %57
               OpStore %12 %59
               OpBranch %50
         %50 = OpLabel
         %60 = OpLoad %6 %46
         %61 = OpIAdd %6 %60 %17
               OpStore %46 %61
               OpBranch %47
         %49 = OpLabel
         %62 = OpLoad %6 %12
               OpReturnValue %62
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, shader, kReduceAssembleOption);

  // Targeting all functions, there are quite a few opportunities.  To avoid
  // making the test too sensitive, we check that there are more than a number
  // somewhat lower than the real number.
  const auto all_ops =
      OperandToConstReductionOpportunityFinder().GetAvailableOpportunities(
          context.get(), 0);
  ASSERT_TRUE(all_ops.size() > 100);

  // Targeting individual functions, there are fewer opportunities.  Again, we
  // avoid checking against an exact number so that the test is not too
  // sensitive.
  const auto ops_for_function_4 =
      OperandToConstReductionOpportunityFinder().GetAvailableOpportunities(
          context.get(), 4);
  const auto ops_for_function_10 =
      OperandToConstReductionOpportunityFinder().GetAvailableOpportunities(
          context.get(), 10);
  const auto ops_for_function_13 =
      OperandToConstReductionOpportunityFinder().GetAvailableOpportunities(
          context.get(), 13);
  ASSERT_TRUE(ops_for_function_4.size() < 60);
  ASSERT_TRUE(ops_for_function_10.size() < 50);
  ASSERT_TRUE(ops_for_function_13.size() < 80);

  // The total number of opportunities should be the sum of the per-function
  // opportunities.
  ASSERT_EQ(all_ops.size(), ops_for_function_4.size() +
                                ops_for_function_10.size() +
                                ops_for_function_13.size());
}

}  // namespace
}  // namespace reduce
}  // namespace spvtools
