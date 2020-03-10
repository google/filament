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

#include "source/fuzz/transformation_add_type_function.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddTypeFunctionTest, BasicTest) {
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
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %12 = OpTypeFloat 32
         %13 = OpTypeStruct %6 %12
         %14 = OpTypePointer Function %13
        %200 = OpTypePointer Function %13
         %15 = OpTypeVector %12 3
         %16 = OpTypePointer Function %15
         %17 = OpTypeVector %12 2
         %18 = OpTypeFunction %17 %14 %16
         %23 = OpConstant %12 1
         %24 = OpConstantComposite %17 %23 %23
         %27 = OpConstant %6 3
         %30 = OpConstant %6 1
         %31 = OpConstant %12 2
         %32 = OpConstantComposite %13 %30 %31
         %33 = OpConstantComposite %15 %23 %23 %23
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
  ASSERT_FALSE(TransformationAddTypeFunction(4, 12, {12, 16, 14})
                   .IsApplicable(context.get(), fact_manager));
  // %1 is not a type
  ASSERT_FALSE(TransformationAddTypeFunction(100, 1, {12, 16, 14})
                   .IsApplicable(context.get(), fact_manager));

  // %18 is a function type
  ASSERT_FALSE(TransformationAddTypeFunction(100, 12, {18})
                   .IsApplicable(context.get(), fact_manager));

  // A function of this signature already exists
  ASSERT_FALSE(TransformationAddTypeFunction(100, 17, {14, 16})
                   .IsApplicable(context.get(), fact_manager));

  TransformationAddTypeFunction transformations[] = {
      // %100 = OpTypeFunction %12 %12 %16 %14
      TransformationAddTypeFunction(100, 12, {12, 16, 14}),

      // %101 = OpTypeFunction %12
      TransformationAddTypeFunction(101, 12, {}),

      // %102 = OpTypeFunction %17 %200 %16
      TransformationAddTypeFunction(102, 17, {200, 16})};

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
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %12 = OpTypeFloat 32
         %13 = OpTypeStruct %6 %12
         %14 = OpTypePointer Function %13
        %200 = OpTypePointer Function %13
         %15 = OpTypeVector %12 3
         %16 = OpTypePointer Function %15
         %17 = OpTypeVector %12 2
         %18 = OpTypeFunction %17 %14 %16
         %23 = OpConstant %12 1
         %24 = OpConstantComposite %17 %23 %23
         %27 = OpConstant %6 3
         %30 = OpConstant %6 1
         %31 = OpConstant %12 2
         %32 = OpConstantComposite %13 %30 %31
         %33 = OpConstantComposite %15 %23 %23 %23
        %100 = OpTypeFunction %12 %12 %16 %14
        %101 = OpTypeFunction %12
        %102 = OpTypeFunction %17 %200 %16
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
