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

#include "source/fuzz/transformation_add_no_contraction_decoration.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddNoContractionDecorationTest, BasicScenarios) {
  // This is a simple transformation and this test handles the main cases.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "y"
               OpName %14 "i"
               OpDecorate %32 NoContraction
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %11 = OpConstant %6 2
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 0
         %22 = OpConstant %12 10
         %23 = OpTypeBool
         %31 = OpConstant %6 3.5999999
         %38 = OpConstant %12 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %14 %15
               OpBranch %16
         %16 = OpLabel
               OpLoopMerge %18 %19 None
               OpBranch %20
         %20 = OpLabel
         %21 = OpLoad %12 %14
         %24 = OpSLessThan %23 %21 %22
               OpBranchConditional %24 %17 %18
         %17 = OpLabel
         %25 = OpLoad %6 %10
         %26 = OpLoad %6 %10
         %27 = OpFMul %6 %25 %26
         %28 = OpLoad %6 %8
         %29 = OpFAdd %6 %28 %27
               OpStore %8 %29
         %30 = OpLoad %6 %10
         %32 = OpFDiv %6 %30 %31
               OpStore %10 %32
         %33 = OpLoad %12 %14
         %34 = OpConvertSToF %6 %33
         %35 = OpLoad %6 %8
         %36 = OpFAdd %6 %35 %34
               OpStore %8 %36
               OpBranch %19
         %19 = OpLabel
         %37 = OpLoad %12 %14
         %39 = OpIAdd %12 %37 %38
               OpStore %14 %39
               OpBranch %16
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));
  FactManager fact_manager;

  // Invalid: 200 is not an id
  ASSERT_FALSE(TransformationAddNoContractionDecoration(200).IsApplicable(
      context.get(), fact_manager));
  // Invalid: 17 is a block id
  ASSERT_FALSE(TransformationAddNoContractionDecoration(17).IsApplicable(
      context.get(), fact_manager));
  // Invalid: 24 is not arithmetic
  ASSERT_FALSE(TransformationAddNoContractionDecoration(24).IsApplicable(
      context.get(), fact_manager));

  // It is valid to add NoContraction to each of these ids (and it's fine to
  // have duplicates of the decoration, in the case of 32).
  for (uint32_t result_id : {32u, 32u, 27u, 29u, 39u}) {
    TransformationAddNoContractionDecoration transformation(result_id);
    ASSERT_TRUE(transformation.IsApplicable(context.get(), fact_manager));
    transformation.Apply(context.get(), &fact_manager);
    ASSERT_TRUE(IsValid(env, context.get()));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "y"
               OpName %14 "i"
               OpDecorate %32 NoContraction
               OpDecorate %32 NoContraction
               OpDecorate %32 NoContraction
               OpDecorate %27 NoContraction
               OpDecorate %29 NoContraction
               OpDecorate %39 NoContraction
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %11 = OpConstant %6 2
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 0
         %22 = OpConstant %12 10
         %23 = OpTypeBool
         %31 = OpConstant %6 3.5999999
         %38 = OpConstant %12 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %14 %15
               OpBranch %16
         %16 = OpLabel
               OpLoopMerge %18 %19 None
               OpBranch %20
         %20 = OpLabel
         %21 = OpLoad %12 %14
         %24 = OpSLessThan %23 %21 %22
               OpBranchConditional %24 %17 %18
         %17 = OpLabel
         %25 = OpLoad %6 %10
         %26 = OpLoad %6 %10
         %27 = OpFMul %6 %25 %26
         %28 = OpLoad %6 %8
         %29 = OpFAdd %6 %28 %27
               OpStore %8 %29
         %30 = OpLoad %6 %10
         %32 = OpFDiv %6 %30 %31
               OpStore %10 %32
         %33 = OpLoad %12 %14
         %34 = OpConvertSToF %6 %33
         %35 = OpLoad %6 %8
         %36 = OpFAdd %6 %35 %34
               OpStore %8 %36
               OpBranch %19
         %19 = OpLabel
         %37 = OpLoad %12 %14
         %39 = OpIAdd %12 %37 %38
               OpStore %14 %39
               OpBranch %16
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
