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

#include "source/fuzz/transformation_set_selection_control.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationSetSelectionControlTest, VariousScenarios) {
  // This is a simple transformation; this test captures the important things
  // to check for.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 10
         %17 = OpTypeBool
         %20 = OpConstant %6 3
         %25 = OpConstant %6 1
         %28 = OpConstant %6 2
         %38 = OpConstant %6 4
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %15 = OpLoad %6 %8
         %18 = OpSLessThan %17 %15 %16
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
         %19 = OpLoad %6 %8
         %21 = OpSGreaterThan %17 %19 %20
               OpSelectionMerge %23 Flatten
               OpBranchConditional %21 %22 %23
         %22 = OpLabel
         %24 = OpLoad %6 %8
         %26 = OpIAdd %6 %24 %25
               OpStore %8 %26
               OpBranch %23
         %23 = OpLabel
         %27 = OpLoad %6 %8
         %29 = OpSLessThan %17 %27 %28
               OpSelectionMerge %31 DontFlatten
               OpBranchConditional %29 %30 %31
         %30 = OpLabel
         %32 = OpLoad %6 %8
         %33 = OpISub %6 %32 %25
               OpStore %8 %33
               OpBranch %31
         %31 = OpLabel
         %34 = OpLoad %6 %8
               OpSelectionMerge %37 None
               OpSwitch %34 %36 0 %35
         %36 = OpLabel
               OpBranch %37
         %35 = OpLabel
         %39 = OpLoad %6 %8
         %40 = OpIAdd %6 %39 %38
               OpStore %8 %40
               OpBranch %36
         %37 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %43 = OpLoad %6 %8
         %44 = OpIAdd %6 %43 %25
               OpStore %8 %44
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  FactManager fact_manager;

  // %44 is not a block
  ASSERT_FALSE(
      TransformationSetSelectionControl(44, SpvSelectionControlFlattenMask)
          .IsApplicable(context.get(), fact_manager));
  // %13 does not end with OpSelectionMerge
  ASSERT_FALSE(
      TransformationSetSelectionControl(13, SpvSelectionControlMaskNone)
          .IsApplicable(context.get(), fact_manager));
  // %10 ends in OpLoopMerge, not OpSelectionMerge
  ASSERT_FALSE(
      TransformationSetSelectionControl(10, SpvSelectionControlMaskNone)
          .IsApplicable(context.get(), fact_manager));

  TransformationSetSelectionControl transformation1(
      11, SpvSelectionControlDontFlattenMask);
  ASSERT_TRUE(transformation1.IsApplicable(context.get(), fact_manager));
  transformation1.Apply(context.get(), &fact_manager);

  TransformationSetSelectionControl transformation2(
      23, SpvSelectionControlFlattenMask);
  ASSERT_TRUE(transformation2.IsApplicable(context.get(), fact_manager));
  transformation2.Apply(context.get(), &fact_manager);

  TransformationSetSelectionControl transformation3(
      31, SpvSelectionControlMaskNone);
  ASSERT_TRUE(transformation3.IsApplicable(context.get(), fact_manager));
  transformation3.Apply(context.get(), &fact_manager);

  TransformationSetSelectionControl transformation4(
      31, SpvSelectionControlFlattenMask);
  ASSERT_TRUE(transformation4.IsApplicable(context.get(), fact_manager));
  transformation4.Apply(context.get(), &fact_manager);

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 10
         %17 = OpTypeBool
         %20 = OpConstant %6 3
         %25 = OpConstant %6 1
         %28 = OpConstant %6 2
         %38 = OpConstant %6 4
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %15 = OpLoad %6 %8
         %18 = OpSLessThan %17 %15 %16
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
         %19 = OpLoad %6 %8
         %21 = OpSGreaterThan %17 %19 %20
               OpSelectionMerge %23 DontFlatten
               OpBranchConditional %21 %22 %23
         %22 = OpLabel
         %24 = OpLoad %6 %8
         %26 = OpIAdd %6 %24 %25
               OpStore %8 %26
               OpBranch %23
         %23 = OpLabel
         %27 = OpLoad %6 %8
         %29 = OpSLessThan %17 %27 %28
               OpSelectionMerge %31 Flatten
               OpBranchConditional %29 %30 %31
         %30 = OpLabel
         %32 = OpLoad %6 %8
         %33 = OpISub %6 %32 %25
               OpStore %8 %33
               OpBranch %31
         %31 = OpLabel
         %34 = OpLoad %6 %8
               OpSelectionMerge %37 Flatten
               OpSwitch %34 %36 0 %35
         %36 = OpLabel
               OpBranch %37
         %35 = OpLabel
         %39 = OpLoad %6 %8
         %40 = OpIAdd %6 %39 %38
               OpStore %8 %40
               OpBranch %36
         %37 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %43 = OpLoad %6 %8
         %44 = OpIAdd %6 %43 %25
               OpStore %8 %44
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
