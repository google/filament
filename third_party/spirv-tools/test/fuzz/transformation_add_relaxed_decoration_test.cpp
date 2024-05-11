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

#include "source/fuzz/transformation_add_relaxed_decoration.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {
TEST(TransformationAddRelaxedDecorationTest, BasicScenarios) {
  // This is a simple transformation and this test handles the main cases.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpName %14 "c"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 4
         %11 = OpConstant %6 6
         %12 = OpTypeBool
         %13 = OpTypePointer Function %12
         %15 = OpConstantTrue %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %14 %15
               OpSelectionMerge %19 None
               OpBranchConditional %15 %19 %100
        %100 = OpLabel
         %25 = OpISub %6 %9 %11
         %28 = OpLogicalNot %12 %15
               OpBranch %19
         %19 = OpLabel
         %27 = OpISub %6 %9 %11
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
  transformation_context.GetFactManager()->AddFactBlockIsDead(100);

  // Invalid: 200 is not an id.
  ASSERT_FALSE(TransformationAddRelaxedDecoration(200).IsApplicable(
      context.get(), transformation_context));
  // Invalid: 1 is not in a block.
  ASSERT_FALSE(TransformationAddRelaxedDecoration(1).IsApplicable(
      context.get(), transformation_context));
  // Invalid: 27 is not in a dead block.
  ASSERT_FALSE(TransformationAddRelaxedDecoration(27).IsApplicable(
      context.get(), transformation_context));
  // Invalid: 28 is in a dead block, but returns bool (not numeric).
  ASSERT_FALSE(TransformationAddRelaxedDecoration(28).IsApplicable(
      context.get(), transformation_context));
  // It is valid to add RelaxedPrecision to 25 (and it's fine to
  // have a duplicate).
  for (uint32_t result_id : {25u, 25u}) {
    TransformationAddRelaxedDecoration transformation(result_id);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpName %14 "c"
               OpDecorate %25 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 4
         %11 = OpConstant %6 6
         %12 = OpTypeBool
         %13 = OpTypePointer Function %12
         %15 = OpConstantTrue %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %14 %15
               OpSelectionMerge %19 None
               OpBranchConditional %15 %19 %100
        %100 = OpLabel
         %25 = OpISub %6 %9 %11
         %28 = OpLogicalNot %12 %15
               OpBranch %19
         %19 = OpLabel
         %27 = OpISub %6 %9 %11
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
