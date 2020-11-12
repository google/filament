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

#include "source/fuzz/transformation_add_image_sample_unused_components.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddImageSampleUnusedComponentsTest, IsApplicable) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability LiteralSampler
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %18 "main" %17
               OpExecutionMode %18 OriginUpperLeft
               OpSource ESSL 310
               OpName %18 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeImage %4 2D 0 0 0 1 Rgba32f
          %9 = OpTypePointer Image %8
         %10 = OpTypeSampledImage %8
         %11 = OpTypeSampler
         %12 = OpConstant %4 1
         %13 = OpConstant %4 2
         %14 = OpConstant %4 3
         %15 = OpConstant %4 4
         %16 = OpConstantSampler %11 None 0 Linear
         %17 = OpVariable %9 Image
         %18 = OpFunction %2 None %3
         %19 = OpLabel
         %20 = OpLoad %8 %17
         %21 = OpSampledImage %10 %20 %16
         %22 = OpCompositeConstruct %5 %12 %13
         %23 = OpCompositeConstruct %6 %22 %14
         %24 = OpCompositeConstruct %7 %23 %15
         %25 = OpImageSampleImplicitLod %7 %21 %22
         %26 = OpImageSampleExplicitLod %7 %21 %23 Lod %12
         %27 = OpImageSampleExplicitLod %7 %21 %24 Lod %12
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
  // Tests applicable image instruction.
  auto instruction_descriptor =
      MakeInstructionDescriptor(25, SpvOpImageSampleImplicitLod, 0);
  auto transformation =
      TransformationAddImageSampleUnusedComponents(23, instruction_descriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor =
      MakeInstructionDescriptor(26, SpvOpImageSampleExplicitLod, 0);
  transformation =
      TransformationAddImageSampleUnusedComponents(24, instruction_descriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests undefined image instructions.
  instruction_descriptor =
      MakeInstructionDescriptor(27, SpvOpImageSampleImplicitLod, 0);
  transformation =
      TransformationAddImageSampleUnusedComponents(23, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor =
      MakeInstructionDescriptor(28, SpvOpImageSampleExplicitLod, 0);
  transformation =
      TransformationAddImageSampleUnusedComponents(23, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests non-image instructions.
  instruction_descriptor = MakeInstructionDescriptor(19, SpvOpLabel, 0);
  transformation =
      TransformationAddImageSampleUnusedComponents(24, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor = MakeInstructionDescriptor(20, SpvOpLoad, 0);
  transformation =
      TransformationAddImageSampleUnusedComponents(24, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests coordinate operand being a vec4.
  instruction_descriptor =
      MakeInstructionDescriptor(27, SpvOpImageSampleExplicitLod, 0);
  transformation =
      TransformationAddImageSampleUnusedComponents(22, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests undefined coordinate with unused operands.
  instruction_descriptor =
      MakeInstructionDescriptor(25, SpvOpImageSampleImplicitLod, 0);
  transformation =
      TransformationAddImageSampleUnusedComponents(27, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests coordinate with unused operands being a non-OpCompositeConstruct
  // instruction.
  instruction_descriptor =
      MakeInstructionDescriptor(25, SpvOpImageSampleImplicitLod, 0);
  transformation =
      TransformationAddImageSampleUnusedComponents(21, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests the first OpCompositeConstruct constituent not being the original
  // coordinate.
  instruction_descriptor =
      MakeInstructionDescriptor(25, SpvOpImageSampleImplicitLod, 0);
  transformation =
      TransformationAddImageSampleUnusedComponents(22, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddImageSampleUnusedComponentsTest, Apply) {
  std::string reference_shader = R"(
               OpCapability Shader
               OpCapability LiteralSampler
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %18 "main" %17
               OpExecutionMode %18 OriginUpperLeft
               OpSource ESSL 310
               OpName %18 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeImage %4 2D 0 0 0 1 Rgba32f
          %9 = OpTypePointer Image %8
         %10 = OpTypeSampledImage %8
         %11 = OpTypeSampler
         %12 = OpConstant %4 1
         %13 = OpConstant %4 2
         %14 = OpConstant %4 3
         %15 = OpConstant %4 4
         %16 = OpConstantSampler %11 None 0 Linear
         %17 = OpVariable %9 Image
         %18 = OpFunction %2 None %3
         %19 = OpLabel
         %20 = OpLoad %8 %17
         %21 = OpSampledImage %10 %20 %16
         %22 = OpCompositeConstruct %5 %12 %13
         %23 = OpCompositeConstruct %6 %22 %14
         %24 = OpCompositeConstruct %7 %23 %15
         %25 = OpImageSampleImplicitLod %7 %21 %22
         %26 = OpImageSampleExplicitLod %7 %21 %23 Lod %12
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto instruction_descriptor =
      MakeInstructionDescriptor(25, SpvOpImageSampleImplicitLod, 0);
  auto transformation =
      TransformationAddImageSampleUnusedComponents(23, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(26, SpvOpImageSampleExplicitLod, 0);
  transformation =
      TransformationAddImageSampleUnusedComponents(24, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
               OpCapability LiteralSampler
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %18 "main" %17
               OpExecutionMode %18 OriginUpperLeft
               OpSource ESSL 310
               OpName %18 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeImage %4 2D 0 0 0 1 Rgba32f
          %9 = OpTypePointer Image %8
         %10 = OpTypeSampledImage %8
         %11 = OpTypeSampler
         %12 = OpConstant %4 1
         %13 = OpConstant %4 2
         %14 = OpConstant %4 3
         %15 = OpConstant %4 4
         %16 = OpConstantSampler %11 None 0 Linear
         %17 = OpVariable %9 Image
         %18 = OpFunction %2 None %3
         %19 = OpLabel
         %20 = OpLoad %8 %17
         %21 = OpSampledImage %10 %20 %16
         %22 = OpCompositeConstruct %5 %12 %13
         %23 = OpCompositeConstruct %6 %22 %14
         %24 = OpCompositeConstruct %7 %23 %15
         %25 = OpImageSampleImplicitLod %7 %21 %23
         %26 = OpImageSampleExplicitLod %7 %21 %24 Lod %12
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
