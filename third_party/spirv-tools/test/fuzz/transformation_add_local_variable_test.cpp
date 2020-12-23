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

#include "source/fuzz/transformation_add_local_variable.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddLocalVariableTest, BasicTest) {
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
          %7 = OpTypeStruct %6 %6
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpConstantComposite %7 %10 %11
         %13 = OpTypeFloat 32
         %14 = OpTypeInt 32 0
         %15 = OpConstant %14 3
         %16 = OpTypeArray %13 %15
         %17 = OpTypeBool
         %18 = OpTypeStruct %16 %7 %17
         %19 = OpTypePointer Function %18
         %21 = OpConstant %13 1
         %22 = OpConstant %13 2
         %23 = OpConstant %13 4
         %24 = OpConstantComposite %16 %21 %22 %23
         %25 = OpConstant %6 5
         %26 = OpConstant %6 6
         %27 = OpConstantComposite %7 %25 %26
         %28 = OpConstantFalse %17
         %29 = OpConstantComposite %18 %24 %27 %28
         %30 = OpTypeVector %13 2
         %31 = OpTypePointer Function %30
         %33 = OpConstantComposite %30 %21 %21
         %34 = OpTypeVector %17 3
         %35 = OpTypePointer Function %34
         %37 = OpConstantTrue %17
         %38 = OpConstantComposite %34 %37 %28 %28
         %39 = OpTypeVector %13 4
         %40 = OpTypeMatrix %39 3
         %41 = OpTypePointer Function %40
         %43 = OpConstantComposite %39 %21 %22 %23 %21
         %44 = OpConstantComposite %39 %22 %23 %21 %22
         %45 = OpConstantComposite %39 %23 %21 %22 %23
         %46 = OpConstantComposite %40 %43 %44 %45
         %50 = OpTypePointer Function %14
         %51 = OpConstantNull %14
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
  // A few cases of inapplicable transformations:
  // Id 4 is already in use
  ASSERT_FALSE(TransformationAddLocalVariable(4, 50, 4, 51, true)
                   .IsApplicable(context.get(), transformation_context));
  // Type mismatch between initializer and pointer
  ASSERT_FALSE(TransformationAddLocalVariable(105, 46, 4, 51, true)
                   .IsApplicable(context.get(), transformation_context));
  // Id 5 is not a function
  ASSERT_FALSE(TransformationAddLocalVariable(105, 50, 5, 51, true)
                   .IsApplicable(context.get(), transformation_context));

  // %105 = OpVariable %50 Function %51
  {
    TransformationAddLocalVariable transformation(105, 50, 4, 51, true);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  // %104 = OpVariable %41 Function %46
  {
    TransformationAddLocalVariable transformation(104, 41, 4, 46, false);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  // %103 = OpVariable %35 Function %38
  {
    TransformationAddLocalVariable transformation(103, 35, 4, 38, true);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  // %102 = OpVariable %31 Function %33
  {
    TransformationAddLocalVariable transformation(102, 31, 4, 33, false);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  // %101 = OpVariable %19 Function %29
  {
    TransformationAddLocalVariable transformation(101, 19, 4, 29, true);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  // %100 = OpVariable %8 Function %12
  {
    TransformationAddLocalVariable transformation(100, 8, 4, 12, false);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(100));
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(101));
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(102));
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(103));
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(104));
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(105));

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
          %7 = OpTypeStruct %6 %6
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpConstantComposite %7 %10 %11
         %13 = OpTypeFloat 32
         %14 = OpTypeInt 32 0
         %15 = OpConstant %14 3
         %16 = OpTypeArray %13 %15
         %17 = OpTypeBool
         %18 = OpTypeStruct %16 %7 %17
         %19 = OpTypePointer Function %18
         %21 = OpConstant %13 1
         %22 = OpConstant %13 2
         %23 = OpConstant %13 4
         %24 = OpConstantComposite %16 %21 %22 %23
         %25 = OpConstant %6 5
         %26 = OpConstant %6 6
         %27 = OpConstantComposite %7 %25 %26
         %28 = OpConstantFalse %17
         %29 = OpConstantComposite %18 %24 %27 %28
         %30 = OpTypeVector %13 2
         %31 = OpTypePointer Function %30
         %33 = OpConstantComposite %30 %21 %21
         %34 = OpTypeVector %17 3
         %35 = OpTypePointer Function %34
         %37 = OpConstantTrue %17
         %38 = OpConstantComposite %34 %37 %28 %28
         %39 = OpTypeVector %13 4
         %40 = OpTypeMatrix %39 3
         %41 = OpTypePointer Function %40
         %43 = OpConstantComposite %39 %21 %22 %23 %21
         %44 = OpConstantComposite %39 %22 %23 %21 %22
         %45 = OpConstantComposite %39 %23 %21 %22 %23
         %46 = OpConstantComposite %40 %43 %44 %45
         %50 = OpTypePointer Function %14
         %51 = OpConstantNull %14
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %100 = OpVariable %8 Function %12
        %101 = OpVariable %19 Function %29
        %102 = OpVariable %31 Function %33
        %103 = OpVariable %35 Function %38
        %104 = OpVariable %41 Function %46
        %105 = OpVariable %50 Function %51
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
