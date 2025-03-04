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

#include "source/fuzz/transformation_make_vector_operation_dynamic.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationMakeVectorOperationDynamicTest, IsApplicable) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %22 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 0
          %5 = OpTypeFloat 32
          %6 = OpTypeVector %5 2
          %7 = OpTypeVector %5 3
          %8 = OpTypeVector %5 4
          %9 = OpTypeMatrix %6 2

; Constant scalars
         %10 = OpConstant %4 0
         %11 = OpConstant %4 1
         %12 = OpConstant %4 2
         %13 = OpConstant %5 0
         %14 = OpConstant %5 1
         %15 = OpConstant %5 2
         %16 = OpConstant %5 3

; Constant composites
         %17 = OpConstantComposite %6 %13 %14
         %18 = OpConstantComposite %6 %15 %16
         %19 = OpConstantComposite %7 %13 %14 %15
         %20 = OpConstantComposite %8 %13 %14 %15 %16
         %21 = OpConstantComposite %9 %17 %18

; main function
         %22 = OpFunction %2 None %3
         %23 = OpLabel
         %24 = OpCompositeExtract %5 %17 0
         %25 = OpCompositeExtract %5 %17 1
         %26 = OpCompositeExtract %5 %18 0
         %27 = OpCompositeExtract %5 %18 1
         %28 = OpCompositeExtract %5 %19 0
         %29 = OpCompositeExtract %5 %19 1
         %30 = OpCompositeExtract %5 %19 2
         %31 = OpCompositeExtract %5 %20 0
         %32 = OpCompositeExtract %5 %20 1
         %33 = OpCompositeExtract %5 %20 2
         %34 = OpCompositeExtract %5 %20 3
         %35 = OpCompositeExtract %6 %21 0
         %36 = OpCompositeExtract %6 %21 1
         %37 = OpCompositeInsert %6 %15 %17 0
         %38 = OpCompositeInsert %6 %16 %17 1
         %39 = OpCompositeInsert %6 %13 %18 0
         %40 = OpCompositeInsert %6 %14 %18 1
         %41 = OpCompositeInsert %7 %13 %19 0
         %42 = OpCompositeInsert %7 %14 %19 1
         %43 = OpCompositeInsert %7 %15 %19 2
         %44 = OpCompositeInsert %8 %13 %20 0
         %45 = OpCompositeInsert %8 %14 %20 1
         %46 = OpCompositeInsert %8 %15 %20 2
         %47 = OpCompositeInsert %8 %16 %20 3
         %48 = OpCompositeInsert %9 %17 %21 0
         %49 = OpCompositeInsert %9 %18 %21 1
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
  // Tests undefined instruction.
  auto transformation = TransformationMakeVectorOperationDynamic(50, 10);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests non-composite instruction.
  transformation = TransformationMakeVectorOperationDynamic(23, 11);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests composite being a matrix.
  transformation = TransformationMakeVectorOperationDynamic(48, 12);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests literal not defined as constant.
  transformation = TransformationMakeVectorOperationDynamic(34, 51);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests applicable instructions.
  transformation = TransformationMakeVectorOperationDynamic(24, 10);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  transformation = TransformationMakeVectorOperationDynamic(25, 11);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  transformation = TransformationMakeVectorOperationDynamic(26, 10);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  transformation = TransformationMakeVectorOperationDynamic(37, 10);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  transformation = TransformationMakeVectorOperationDynamic(38, 11);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  transformation = TransformationMakeVectorOperationDynamic(39, 10);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationMakeVectorOperationDynamicTest, Apply) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %20 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 0
          %5 = OpTypeFloat 32
          %6 = OpTypeVector %5 2
          %7 = OpTypeVector %5 3
          %8 = OpTypeVector %5 4

; Constant scalars
          %9 = OpConstant %4 0
         %10 = OpConstant %4 1
         %11 = OpConstant %4 2
         %12 = OpConstant %4 3
         %13 = OpConstant %5 0
         %14 = OpConstant %5 1
         %15 = OpConstant %5 2
         %16 = OpConstant %5 3

; Constant vectors
         %17 = OpConstantComposite %6 %13 %14
         %18 = OpConstantComposite %7 %13 %14 %15
         %19 = OpConstantComposite %8 %13 %14 %15 %16

; main function
         %20 = OpFunction %2 None %3
         %21 = OpLabel
         %22 = OpCompositeExtract %5 %17 0
         %23 = OpCompositeExtract %5 %17 1
         %24 = OpCompositeExtract %5 %18 0
         %25 = OpCompositeExtract %5 %18 1
         %26 = OpCompositeExtract %5 %18 2
         %27 = OpCompositeExtract %5 %19 0
         %28 = OpCompositeExtract %5 %19 1
         %29 = OpCompositeExtract %5 %19 2
         %30 = OpCompositeExtract %5 %19 3
         %31 = OpCompositeInsert %6 %13 %17 0
         %32 = OpCompositeInsert %6 %14 %17 1
         %33 = OpCompositeInsert %7 %13 %18 0
         %34 = OpCompositeInsert %7 %14 %18 1
         %35 = OpCompositeInsert %7 %15 %18 2
         %36 = OpCompositeInsert %8 %13 %19 0
         %37 = OpCompositeInsert %8 %14 %19 1
         %38 = OpCompositeInsert %8 %15 %19 2
         %39 = OpCompositeInsert %8 %16 %19 3
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
  auto transformation = TransformationMakeVectorOperationDynamic(22, 9);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(23, 10);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(24, 9);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(25, 10);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(26, 11);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(27, 9);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(28, 10);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(29, 11);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(30, 12);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(31, 9);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(32, 10);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(33, 9);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(34, 10);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(35, 11);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(36, 9);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(37, 10);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(38, 11);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  transformation = TransformationMakeVectorOperationDynamic(39, 12);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %20 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeInt 32 0
          %5 = OpTypeFloat 32
          %6 = OpTypeVector %5 2
          %7 = OpTypeVector %5 3
          %8 = OpTypeVector %5 4

; Constant scalars
          %9 = OpConstant %4 0
         %10 = OpConstant %4 1
         %11 = OpConstant %4 2
         %12 = OpConstant %4 3
         %13 = OpConstant %5 0
         %14 = OpConstant %5 1
         %15 = OpConstant %5 2
         %16 = OpConstant %5 3

; Constant vectors
         %17 = OpConstantComposite %6 %13 %14
         %18 = OpConstantComposite %7 %13 %14 %15
         %19 = OpConstantComposite %8 %13 %14 %15 %16

; main function
         %20 = OpFunction %2 None %3
         %21 = OpLabel
         %22 = OpVectorExtractDynamic %5 %17 %9
         %23 = OpVectorExtractDynamic %5 %17 %10
         %24 = OpVectorExtractDynamic %5 %18 %9
         %25 = OpVectorExtractDynamic %5 %18 %10
         %26 = OpVectorExtractDynamic %5 %18 %11
         %27 = OpVectorExtractDynamic %5 %19 %9
         %28 = OpVectorExtractDynamic %5 %19 %10
         %29 = OpVectorExtractDynamic %5 %19 %11
         %30 = OpVectorExtractDynamic %5 %19 %12
         %31 = OpVectorInsertDynamic %6 %17 %13 %9
         %32 = OpVectorInsertDynamic %6 %17 %14 %10
         %33 = OpVectorInsertDynamic %7 %18 %13 %9
         %34 = OpVectorInsertDynamic %7 %18 %14 %10
         %35 = OpVectorInsertDynamic %7 %18 %15 %11
         %36 = OpVectorInsertDynamic %8 %19 %13 %9
         %37 = OpVectorInsertDynamic %8 %19 %14 %10
         %38 = OpVectorInsertDynamic %8 %19 %15 %11
         %39 = OpVectorInsertDynamic %8 %19 %16 %12
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
