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

#include "source/fuzz/transformation_add_constant_null.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddConstantNullTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %6 2
          %9 = OpTypeVector %6 3
         %10 = OpTypeVector %6 4
         %11 = OpTypeVector %7 2
         %20 = OpTypeSampler
         %21 = OpTypeImage %6 2D 0 0 0 0 Rgba32f
         %22 = OpTypeSampledImage %21
          %4 = OpFunction %2 None %3
          %5 = OpLabel
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
  // Id already in use
  ASSERT_FALSE(TransformationAddConstantNull(4, 11).IsApplicable(
      context.get(), transformation_context));
  // %1 is not a type
  ASSERT_FALSE(TransformationAddConstantNull(100, 1).IsApplicable(
      context.get(), transformation_context));

  // %3 is a function type
  ASSERT_FALSE(TransformationAddConstantNull(100, 3).IsApplicable(
      context.get(), transformation_context));

  // %20 is a sampler type
  ASSERT_FALSE(TransformationAddConstantNull(100, 20).IsApplicable(
      context.get(), transformation_context));

  // %21 is an image type
  ASSERT_FALSE(TransformationAddConstantNull(100, 21).IsApplicable(
      context.get(), transformation_context));

  // %22 is a sampled image type
  ASSERT_FALSE(TransformationAddConstantNull(100, 22).IsApplicable(
      context.get(), transformation_context));

  {
    // %100 = OpConstantNull %6
    TransformationAddConstantNull transformation(100, 6);
    ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(100));
    ASSERT_EQ(nullptr, context->get_constant_mgr()->FindDeclaredConstant(100));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_EQ(spv::Op::OpConstantNull,
              context->get_def_use_mgr()->GetDef(100)->opcode());
    ASSERT_EQ(
        0.0F,
        context->get_constant_mgr()->FindDeclaredConstant(100)->GetFloat());
  }

  TransformationAddConstantNull transformations[] = {

      // %101 = OpConstantNull %7
      TransformationAddConstantNull(101, 7),

      // %102 = OpConstantNull %8
      TransformationAddConstantNull(102, 8),

      // %103 = OpConstantNull %9
      TransformationAddConstantNull(103, 9),

      // %104 = OpConstantNull %10
      TransformationAddConstantNull(104, 10),

      // %105 = OpConstantNull %11
      TransformationAddConstantNull(105, 11)};

  for (auto& transformation : transformations) {
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %6 2
          %9 = OpTypeVector %6 3
         %10 = OpTypeVector %6 4
         %11 = OpTypeVector %7 2
         %20 = OpTypeSampler
         %21 = OpTypeImage %6 2D 0 0 0 0 Rgba32f
         %22 = OpTypeSampledImage %21
        %100 = OpConstantNull %6
        %101 = OpConstantNull %7
        %102 = OpConstantNull %8
        %103 = OpConstantNull %9
        %104 = OpConstantNull %10
        %105 = OpConstantNull %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
