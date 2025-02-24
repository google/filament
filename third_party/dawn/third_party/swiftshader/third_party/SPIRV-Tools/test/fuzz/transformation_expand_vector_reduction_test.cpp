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

#include "source/fuzz/transformation_expand_vector_reduction.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationExpandVectorReductionTest, IsApplicable) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %9 "main"

          ; Types
          %2 = OpTypeBool
          %3 = OpTypeVector %2 2
          %4 = OpTypeVoid
          %5 = OpTypeFunction %4

          ; Constants
          %6 = OpConstantTrue %2
          %7 = OpConstantFalse %2
          %8 = OpConstantComposite %3 %6 %7

          ; main function
          %9 = OpFunction %4 None %5
         %10 = OpLabel
         %11 = OpAny %2 %8
         %12 = OpAll %2 %8
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Tests undefined instruction.
  auto transformation = TransformationExpandVectorReduction(13, {14, 15, 16});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests non OpAny or OpAll instruction.
  transformation = TransformationExpandVectorReduction(10, {13, 14, 15});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests the number of fresh ids being different than the necessary.
  transformation = TransformationExpandVectorReduction(11, {13, 14});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  transformation = TransformationExpandVectorReduction(12, {13, 14, 15, 16});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests non-fresh ids.
  transformation = TransformationExpandVectorReduction(11, {12, 13, 14});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests duplicated fresh ids.
  transformation = TransformationExpandVectorReduction(11, {13, 13, 14});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests applicable transformations.
  transformation = TransformationExpandVectorReduction(11, {13, 14, 15});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  transformation = TransformationExpandVectorReduction(12, {13, 14, 15});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationExpandVectorReductionTest, Apply) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %13 "main"

          ; Types
          %2 = OpTypeBool
          %3 = OpTypeVector %2 2
          %4 = OpTypeVector %2 3
          %5 = OpTypeVector %2 4
          %6 = OpTypeVoid
          %7 = OpTypeFunction %6

          ; Constants
          %8 = OpConstantTrue %2
          %9 = OpConstantFalse %2
         %10 = OpConstantComposite %3 %8 %9
         %11 = OpConstantComposite %4 %8 %9 %8
         %12 = OpConstantComposite %5 %8 %9 %8 %9

         ; main function
         %13 = OpFunction %6 None %7
         %14 = OpLabel

         ; OpAny for 2-dimensional vector
         %15 = OpAny %2 %10

         ; OpAny for 3-dimensional vector
         %16 = OpAny %2 %11

         ; OpAny for 4-dimensional vector
         %17 = OpAny %2 %12

         ; OpAll for 2-dimensional vector
         %18 = OpAll %2 %10

         ; OpAll for 3-dimensional vector
         %19 = OpAll %2 %11

         ; OpAll for 4-dimensional vector
         %20 = OpAll %2 %12
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Adds OpAny synonym for 2-dimensional vector.
  auto transformation = TransformationExpandVectorReduction(15, {21, 22, 23});
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(23, {}), MakeDataDescriptor(15, {})));

  // Adds OpAny synonym for 3-dimensional vector.
  transformation =
      TransformationExpandVectorReduction(16, {24, 25, 26, 27, 28});
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(28, {}), MakeDataDescriptor(16, {})));

  // Adds OpAny synonym for 4-dimensional vector.
  transformation =
      TransformationExpandVectorReduction(17, {29, 30, 31, 32, 33, 34, 35});
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(35, {}), MakeDataDescriptor(17, {})));

  // Adds OpAll synonym for 2-dimensional vector.
  transformation = TransformationExpandVectorReduction(18, {36, 37, 38});
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(38, {}), MakeDataDescriptor(18, {})));

  // Adds OpAll synonym for 3-dimensional vector.
  transformation =
      TransformationExpandVectorReduction(19, {39, 40, 41, 42, 43});
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(43, {}), MakeDataDescriptor(19, {})));

  // Adds OpAll synonym for 4-dimensional vector.
  transformation =
      TransformationExpandVectorReduction(20, {44, 45, 46, 47, 48, 49, 50});
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {}), MakeDataDescriptor(20, {})));

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %13 "main"

          ; Types
          %2 = OpTypeBool
          %3 = OpTypeVector %2 2
          %4 = OpTypeVector %2 3
          %5 = OpTypeVector %2 4
          %6 = OpTypeVoid
          %7 = OpTypeFunction %6

          ; Constants
          %8 = OpConstantTrue %2
          %9 = OpConstantFalse %2
         %10 = OpConstantComposite %3 %8 %9
         %11 = OpConstantComposite %4 %8 %9 %8
         %12 = OpConstantComposite %5 %8 %9 %8 %9

         ; main function
         %13 = OpFunction %6 None %7
         %14 = OpLabel

         ; Add OpAny synonym for 2-dimensional vector
         %21 = OpCompositeExtract %2 %10 0
         %22 = OpCompositeExtract %2 %10 1
         %23 = OpLogicalOr %2 %21 %22
         %15 = OpAny %2 %10

         ; Add OpAny synonym for 3-dimensional vector
         %24 = OpCompositeExtract %2 %11 0
         %25 = OpCompositeExtract %2 %11 1
         %26 = OpCompositeExtract %2 %11 2
         %27 = OpLogicalOr %2 %24 %25
         %28 = OpLogicalOr %2 %26 %27
         %16 = OpAny %2 %11

         ; Add OpAny synonym for 4-dimensional vector
         %29 = OpCompositeExtract %2 %12 0
         %30 = OpCompositeExtract %2 %12 1
         %31 = OpCompositeExtract %2 %12 2
         %32 = OpCompositeExtract %2 %12 3
         %33 = OpLogicalOr %2 %29 %30
         %34 = OpLogicalOr %2 %31 %33
         %35 = OpLogicalOr %2 %32 %34
         %17 = OpAny %2 %12

         ; Add OpAll synonym for 2-dimensional vector
         %36 = OpCompositeExtract %2 %10 0
         %37 = OpCompositeExtract %2 %10 1
         %38 = OpLogicalAnd %2 %36 %37
         %18 = OpAll %2 %10

         ; Add OpAll synonym for 3-dimensional vector
         %39 = OpCompositeExtract %2 %11 0
         %40 = OpCompositeExtract %2 %11 1
         %41 = OpCompositeExtract %2 %11 2
         %42 = OpLogicalAnd %2 %39 %40
         %43 = OpLogicalAnd %2 %41 %42
         %19 = OpAll %2 %11

         ; Add OpAll synonym for 4-dimensional vector
         %44 = OpCompositeExtract %2 %12 0
         %45 = OpCompositeExtract %2 %12 1
         %46 = OpCompositeExtract %2 %12 2
         %47 = OpCompositeExtract %2 %12 3
         %48 = OpLogicalAnd %2 %44 %45
         %49 = OpLogicalAnd %2 %46 %48
         %50 = OpLogicalAnd %2 %47 %49
         %20 = OpAll %2 %12
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
