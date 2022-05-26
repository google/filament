// Copyright (c) 2020 André Perez Maselco
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

#include "source/fuzz/transformation_adjust_branch_weights.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAdjustBranchWeightsTest, IsApplicableTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %51 %27
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %25 "buf"
               OpMemberName %25 0 "value"
               OpName %27 ""
               OpName %51 "color"
               OpMemberDecorate %25 0 Offset 0
               OpDecorate %25 Block
               OpDecorate %27 DescriptorSet 0
               OpDecorate %27 Binding 0
               OpDecorate %51 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
        %150 = OpTypeVector %6 2
         %10 = OpConstant %6 0.300000012
         %11 = OpConstant %6 0.400000006
         %12 = OpConstant %6 0.5
         %13 = OpConstant %6 1
         %14 = OpConstantComposite %7 %10 %11 %12 %13
         %15 = OpTypeInt 32 1
         %18 = OpConstant %15 0
         %25 = OpTypeStruct %6
         %26 = OpTypePointer Uniform %25
         %27 = OpVariable %26 Uniform
         %28 = OpTypePointer Uniform %6
         %32 = OpTypeBool
        %103 = OpConstantTrue %32
         %34 = OpConstant %6 0.100000001
         %48 = OpConstant %15 1
         %50 = OpTypePointer Output %7
         %51 = OpVariable %50 Output
        %100 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %101 = OpVariable %100 Function
        %102 = OpVariable %100 Function
               OpBranch %19
         %19 = OpLabel
         %60 = OpPhi %7 %14 %5 %58 %20
         %59 = OpPhi %15 %18 %5 %49 %20
         %29 = OpAccessChain %28 %27 %18
         %30 = OpLoad %6 %29
         %31 = OpConvertFToS %15 %30
         %33 = OpSLessThan %32 %59 %31
               OpLoopMerge %21 %20 None
               OpBranchConditional %33 %20 %21 1 2
         %20 = OpLabel
         %39 = OpCompositeExtract %6 %60 0
         %40 = OpFAdd %6 %39 %34
         %55 = OpCompositeInsert %7 %40 %60 0
         %44 = OpCompositeExtract %6 %60 1
         %45 = OpFSub %6 %44 %34
         %58 = OpCompositeInsert %7 %45 %55 1
         %49 = OpIAdd %15 %59 %48
               OpBranch %19
         %21 = OpLabel
               OpStore %51 %60
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
               OpBranch %105
        %105 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Tests OpBranchConditional instruction with weights.
  auto instruction_descriptor =
      MakeInstructionDescriptor(33, SpvOpBranchConditional, 0);
  auto transformation =
      TransformationAdjustBranchWeights(instruction_descriptor, {0, 1});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests the two branch weights equal to 0.
  instruction_descriptor =
      MakeInstructionDescriptor(33, SpvOpBranchConditional, 0);
  transformation =
      TransformationAdjustBranchWeights(instruction_descriptor, {0, 0});
#ifndef NDEBUG
  ASSERT_DEATH(
      transformation.IsApplicable(context.get(), transformation_context),
      "At least one weight must be non-zero");
#endif

  // Tests 32-bit unsigned integer overflow.
  instruction_descriptor =
      MakeInstructionDescriptor(33, SpvOpBranchConditional, 0);
  transformation = TransformationAdjustBranchWeights(instruction_descriptor,
                                                     {UINT32_MAX, 0});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor =
      MakeInstructionDescriptor(33, SpvOpBranchConditional, 0);
  transformation = TransformationAdjustBranchWeights(instruction_descriptor,
                                                     {1, UINT32_MAX});
#ifndef NDEBUG
  ASSERT_DEATH(
      transformation.IsApplicable(context.get(), transformation_context),
      "The sum of the two weights must not be greater than UINT32_MAX");
#endif

  // Tests OpBranchConditional instruction with no weights.
  instruction_descriptor =
      MakeInstructionDescriptor(21, SpvOpBranchConditional, 0);
  transformation =
      TransformationAdjustBranchWeights(instruction_descriptor, {0, 1});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests non-OpBranchConditional instructions.
  instruction_descriptor = MakeInstructionDescriptor(2, SpvOpTypeVoid, 0);
  transformation =
      TransformationAdjustBranchWeights(instruction_descriptor, {5, 6});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor = MakeInstructionDescriptor(20, SpvOpLabel, 0);
  transformation =
      TransformationAdjustBranchWeights(instruction_descriptor, {1, 2});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor = MakeInstructionDescriptor(49, SpvOpIAdd, 0);
  transformation =
      TransformationAdjustBranchWeights(instruction_descriptor, {1, 2});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAdjustBranchWeightsTest, ApplyTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %51 %27
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %25 "buf"
               OpMemberName %25 0 "value"
               OpName %27 ""
               OpName %51 "color"
               OpMemberDecorate %25 0 Offset 0
               OpDecorate %25 Block
               OpDecorate %27 DescriptorSet 0
               OpDecorate %27 Binding 0
               OpDecorate %51 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
        %150 = OpTypeVector %6 2
         %10 = OpConstant %6 0.300000012
         %11 = OpConstant %6 0.400000006
         %12 = OpConstant %6 0.5
         %13 = OpConstant %6 1
         %14 = OpConstantComposite %7 %10 %11 %12 %13
         %15 = OpTypeInt 32 1
         %18 = OpConstant %15 0
         %25 = OpTypeStruct %6
         %26 = OpTypePointer Uniform %25
         %27 = OpVariable %26 Uniform
         %28 = OpTypePointer Uniform %6
         %32 = OpTypeBool
        %103 = OpConstantTrue %32
         %34 = OpConstant %6 0.100000001
         %48 = OpConstant %15 1
         %50 = OpTypePointer Output %7
         %51 = OpVariable %50 Output
        %100 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %101 = OpVariable %100 Function
        %102 = OpVariable %100 Function
               OpBranch %19
         %19 = OpLabel
         %60 = OpPhi %7 %14 %5 %58 %20
         %59 = OpPhi %15 %18 %5 %49 %20
         %29 = OpAccessChain %28 %27 %18
         %30 = OpLoad %6 %29
         %31 = OpConvertFToS %15 %30
         %33 = OpSLessThan %32 %59 %31
               OpLoopMerge %21 %20 None
               OpBranchConditional %33 %20 %21 1 2
         %20 = OpLabel
         %39 = OpCompositeExtract %6 %60 0
         %40 = OpFAdd %6 %39 %34
         %55 = OpCompositeInsert %7 %40 %60 0
         %44 = OpCompositeExtract %6 %60 1
         %45 = OpFSub %6 %44 %34
         %58 = OpCompositeInsert %7 %45 %55 1
         %49 = OpIAdd %15 %59 %48
               OpBranch %19
         %21 = OpLabel
               OpStore %51 %60
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
               OpBranch %105
        %105 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto instruction_descriptor =
      MakeInstructionDescriptor(33, SpvOpBranchConditional, 0);
  auto transformation =
      TransformationAdjustBranchWeights(instruction_descriptor, {5, 6});
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(21, SpvOpBranchConditional, 0);
  transformation =
      TransformationAdjustBranchWeights(instruction_descriptor, {7, 8});
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %51 %27
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %25 "buf"
               OpMemberName %25 0 "value"
               OpName %27 ""
               OpName %51 "color"
               OpMemberDecorate %25 0 Offset 0
               OpDecorate %25 Block
               OpDecorate %27 DescriptorSet 0
               OpDecorate %27 Binding 0
               OpDecorate %51 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
        %150 = OpTypeVector %6 2
         %10 = OpConstant %6 0.300000012
         %11 = OpConstant %6 0.400000006
         %12 = OpConstant %6 0.5
         %13 = OpConstant %6 1
         %14 = OpConstantComposite %7 %10 %11 %12 %13
         %15 = OpTypeInt 32 1
         %18 = OpConstant %15 0
         %25 = OpTypeStruct %6
         %26 = OpTypePointer Uniform %25
         %27 = OpVariable %26 Uniform
         %28 = OpTypePointer Uniform %6
         %32 = OpTypeBool
        %103 = OpConstantTrue %32
         %34 = OpConstant %6 0.100000001
         %48 = OpConstant %15 1
         %50 = OpTypePointer Output %7
         %51 = OpVariable %50 Output
        %100 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %101 = OpVariable %100 Function
        %102 = OpVariable %100 Function
               OpBranch %19
         %19 = OpLabel
         %60 = OpPhi %7 %14 %5 %58 %20
         %59 = OpPhi %15 %18 %5 %49 %20
         %29 = OpAccessChain %28 %27 %18
         %30 = OpLoad %6 %29
         %31 = OpConvertFToS %15 %30
         %33 = OpSLessThan %32 %59 %31
               OpLoopMerge %21 %20 None
               OpBranchConditional %33 %20 %21 5 6
         %20 = OpLabel
         %39 = OpCompositeExtract %6 %60 0
         %40 = OpFAdd %6 %39 %34
         %55 = OpCompositeInsert %7 %40 %60 0
         %44 = OpCompositeExtract %6 %60 1
         %45 = OpFSub %6 %44 %34
         %58 = OpCompositeInsert %7 %45 %55 1
         %49 = OpIAdd %15 %59 %48
               OpBranch %19
         %21 = OpLabel
               OpStore %51 %60
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105 7 8
        %104 = OpLabel
               OpBranch %105
        %105 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
