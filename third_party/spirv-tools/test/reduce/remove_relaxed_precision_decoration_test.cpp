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

#include "source/reduce/remove_relaxed_precision_decoration_opportunity_finder.h"

#include "source/opt/build_module.h"
#include "source/reduce/reduction_opportunity.h"
#include "source/reduce/reduction_pass.h"
#include "test/reduce/reduce_test_util.h"

namespace spvtools {
namespace reduce {
namespace {

TEST(RemoveRelaxedPrecisionDecorationTest, NothingToRemove) {
  const std::string source = R"(
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
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, source, kReduceAssembleOption);
  const auto ops = RemoveRelaxedPrecisionDecorationOpportunityFinder()
                       .GetAvailableOpportunities(context.get());
  ASSERT_EQ(0, ops.size());
}

TEST(RemoveRelaxedPrecisionDecorationTest, RemoveDecorations) {
  const std::string source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "f"
               OpName %12 "i"
               OpName %16 "v"
               OpName %19 "S"
               OpMemberName %19 0 "a"
               OpMemberName %19 1 "b"
               OpMemberName %19 2 "c"
               OpName %21 "s"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
               OpDecorate %16 RelaxedPrecision
               OpDecorate %17 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpMemberDecorate %19 0 RelaxedPrecision
               OpMemberDecorate %19 1 RelaxedPrecision
               OpMemberDecorate %19 2 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 22
         %14 = OpTypeVector %6 2
         %15 = OpTypePointer Function %14
         %19 = OpTypeStruct %10 %6 %14
         %20 = OpTypePointer Function %19
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %16 = OpVariable %15 Function
         %21 = OpVariable %20 Function
               OpStore %8 %9
               OpStore %12 %13
         %17 = OpLoad %6 %8
         %18 = OpCompositeConstruct %14 %17 %17
               OpStore %16 %18
         %22 = OpLoad %10 %12
         %23 = OpLoad %6 %8
         %24 = OpLoad %14 %16
         %25 = OpCompositeConstruct %19 %22 %23 %24
               OpStore %21 %25
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, source, kReduceAssembleOption);
  const auto ops = RemoveRelaxedPrecisionDecorationOpportunityFinder()
                       .GetAvailableOpportunities(context.get());
  ASSERT_EQ(11, ops.size());

  for (auto& op : ops) {
    ASSERT_TRUE(op->PreconditionHolds());
    op->TryToApply();
  }

  const std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "f"
               OpName %12 "i"
               OpName %16 "v"
               OpName %19 "S"
               OpMemberName %19 0 "a"
               OpMemberName %19 1 "b"
               OpMemberName %19 2 "c"
               OpName %21 "s"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 22
         %14 = OpTypeVector %6 2
         %15 = OpTypePointer Function %14
         %19 = OpTypeStruct %10 %6 %14
         %20 = OpTypePointer Function %19
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %16 = OpVariable %15 Function
         %21 = OpVariable %20 Function
               OpStore %8 %9
               OpStore %12 %13
         %17 = OpLoad %6 %8
         %18 = OpCompositeConstruct %14 %17 %17
               OpStore %16 %18
         %22 = OpLoad %10 %12
         %23 = OpLoad %6 %8
         %24 = OpLoad %14 %16
         %25 = OpCompositeConstruct %19 %22 %23 %24
               OpStore %21 %25
               OpReturn
               OpFunctionEnd
  )";

  CheckEqual(env, expected, context.get());
}

}  // namespace
}  // namespace reduce
}  // namespace spvtools
