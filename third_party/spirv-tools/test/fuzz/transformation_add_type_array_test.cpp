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

#include "source/fuzz/transformation_add_type_array.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddTypeArrayTest, BasicTest) {
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
         %12 = OpConstant %7 3
         %13 = OpConstant %7 0
         %14 = OpConstant %7 -1
         %15 = OpTypeInt 32 0
         %16 = OpConstant %15 5
         %17 = OpConstant %15 0
         %18 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // Id already in use
  ASSERT_FALSE(TransformationAddTypeArray(4, 10, 16).IsApplicable(
      context.get(), fact_manager));
  // %1 is not a type
  ASSERT_FALSE(TransformationAddTypeArray(100, 1, 16)
                   .IsApplicable(context.get(), fact_manager));

  // %3 is a function type
  ASSERT_FALSE(TransformationAddTypeArray(100, 3, 16)
                   .IsApplicable(context.get(), fact_manager));

  // %2 is not a constant
  ASSERT_FALSE(TransformationAddTypeArray(100, 11, 2)
                   .IsApplicable(context.get(), fact_manager));

  // %18 is not an integer
  ASSERT_FALSE(TransformationAddTypeArray(100, 11, 18)
                   .IsApplicable(context.get(), fact_manager));

  // %13 is signed 0
  ASSERT_FALSE(TransformationAddTypeArray(100, 11, 13)
                   .IsApplicable(context.get(), fact_manager));

  // %14 is negative
  ASSERT_FALSE(TransformationAddTypeArray(100, 11, 14)
                   .IsApplicable(context.get(), fact_manager));

  // %17 is unsigned 0
  ASSERT_FALSE(TransformationAddTypeArray(100, 11, 17)
                   .IsApplicable(context.get(), fact_manager));

  TransformationAddTypeArray transformations[] = {
      // %100 = OpTypeArray %10 %16
      TransformationAddTypeArray(100, 10, 16),

      // %101 = OpTypeArray %7 %12
      TransformationAddTypeArray(101, 7, 12)};

  for (auto& transformation : transformations) {
    ASSERT_TRUE(transformation.IsApplicable(context.get(), fact_manager));
    transformation.Apply(context.get(), &fact_manager);
  }
  ASSERT_TRUE(IsValid(env, context.get()));

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
         %12 = OpConstant %7 3
         %13 = OpConstant %7 0
         %14 = OpConstant %7 -1
         %15 = OpTypeInt 32 0
         %16 = OpConstant %15 5
         %17 = OpConstant %15 0
         %18 = OpConstant %6 1
        %100 = OpTypeArray %10 %16
        %101 = OpTypeArray %7 %12
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
