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

#include "source/fuzz/transformation_add_constant_composite.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddConstantCompositeTest, BasicTest) {
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
          %7 = OpTypeVector %6 2
          %8 = OpTypeMatrix %7 3
         %11 = OpConstant %6 0
         %12 = OpConstant %6 1
         %14 = OpConstant %6 2
         %15 = OpConstant %6 3
         %17 = OpConstant %6 4
         %18 = OpConstant %6 5
         %21 = OpTypeInt 32 1
         %22 = OpTypeInt 32 0
         %23 = OpConstant %22 3
         %24 = OpTypeArray %21 %23
         %25 = OpTypeBool
         %26 = OpTypeStruct %24 %25
         %29 = OpConstant %21 1
         %30 = OpConstant %21 2
         %31 = OpConstant %21 3
         %33 = OpConstantFalse %25
         %35 = OpTypeVector %6 3
         %38 = OpConstant %6 6
         %39 = OpConstant %6 7
         %40 = OpConstant %6 8
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

  // Too few ids
  ASSERT_FALSE(TransformationAddConstantComposite(103, 8, {100, 101})
                   .IsApplicable(context.get(), fact_manager));
  // Too many ids
  ASSERT_FALSE(TransformationAddConstantComposite(101, 7, {14, 15, 14})
                   .IsApplicable(context.get(), fact_manager));
  // Id already in use
  ASSERT_FALSE(TransformationAddConstantComposite(40, 7, {11, 12})
                   .IsApplicable(context.get(), fact_manager));
  // %39 is not a type
  ASSERT_FALSE(TransformationAddConstantComposite(100, 39, {11, 12})
                   .IsApplicable(context.get(), fact_manager));

  TransformationAddConstantComposite transformations[] = {
      // %100 = OpConstantComposite %7 %11 %12
      TransformationAddConstantComposite(100, 7, {11, 12}),

      // %101 = OpConstantComposite %7 %14 %15
      TransformationAddConstantComposite(101, 7, {14, 15}),

      // %102 = OpConstantComposite %7 %17 %18
      TransformationAddConstantComposite(102, 7, {17, 18}),

      // %103 = OpConstantComposite %8 %100 %101 %102
      TransformationAddConstantComposite(103, 8, {100, 101, 102}),

      // %104 = OpConstantComposite %24 %29 %30 %31
      TransformationAddConstantComposite(104, 24, {29, 30, 31}),

      // %105 = OpConstantComposite %26 %104 %33
      TransformationAddConstantComposite(105, 26, {104, 33}),

      // %106 = OpConstantComposite %35 %38 %39 %40
      TransformationAddConstantComposite(106, 35, {38, 39, 40})};

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
          %7 = OpTypeVector %6 2
          %8 = OpTypeMatrix %7 3
         %11 = OpConstant %6 0
         %12 = OpConstant %6 1
         %14 = OpConstant %6 2
         %15 = OpConstant %6 3
         %17 = OpConstant %6 4
         %18 = OpConstant %6 5
         %21 = OpTypeInt 32 1
         %22 = OpTypeInt 32 0
         %23 = OpConstant %22 3
         %24 = OpTypeArray %21 %23
         %25 = OpTypeBool
         %26 = OpTypeStruct %24 %25
         %29 = OpConstant %21 1
         %30 = OpConstant %21 2
         %31 = OpConstant %21 3
         %33 = OpConstantFalse %25
         %35 = OpTypeVector %6 3
         %38 = OpConstant %6 6
         %39 = OpConstant %6 7
         %40 = OpConstant %6 8
        %100 = OpConstantComposite %7 %11 %12
        %101 = OpConstantComposite %7 %14 %15
        %102 = OpConstantComposite %7 %17 %18
        %103 = OpConstantComposite %8 %100 %101 %102
        %104 = OpConstantComposite %24 %29 %30 %31
        %105 = OpConstantComposite %26 %104 %33
        %106 = OpConstantComposite %35 %38 %39 %40
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
