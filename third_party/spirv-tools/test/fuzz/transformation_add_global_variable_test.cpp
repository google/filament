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

#include "source/fuzz/transformation_add_global_variable.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddGlobalVariableTest, BasicTest) {
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
         %40 = OpConstant %6 0
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %6 2
         %41 = OpConstantComposite %8 %40 %40
          %9 = OpTypePointer Function %6
         %10 = OpTypePointer Private %6
         %20 = OpTypePointer Uniform %6
         %11 = OpTypePointer Function %7
         %12 = OpTypePointer Private %7
         %13 = OpTypePointer Private %8
         %14 = OpVariable %10 Private
         %15 = OpVariable %20 Uniform
         %16 = OpConstant %7 1
         %17 = OpTypePointer Private %10
         %18 = OpTypeBool
         %19 = OpTypePointer Private %18
         %21 = OpConstantTrue %18
         %22 = OpConstantFalse %18
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Id already in use
  ASSERT_FALSE(
      TransformationAddGlobalVariable(4, 10, SpvStorageClassPrivate, 0, true)
          .IsApplicable(context.get(), transformation_context));
  // %1 is not a type
  ASSERT_FALSE(
      TransformationAddGlobalVariable(100, 1, SpvStorageClassPrivate, 0, false)
          .IsApplicable(context.get(), transformation_context));

  // %7 is not a pointer type
  ASSERT_FALSE(
      TransformationAddGlobalVariable(100, 7, SpvStorageClassPrivate, 0, true)
          .IsApplicable(context.get(), transformation_context));

  // %9 does not have Private storage class
  ASSERT_FALSE(
      TransformationAddGlobalVariable(100, 9, SpvStorageClassPrivate, 0, false)
          .IsApplicable(context.get(), transformation_context));

  // %15 does not have Private storage class
  ASSERT_FALSE(
      TransformationAddGlobalVariable(100, 15, SpvStorageClassPrivate, 0, true)
          .IsApplicable(context.get(), transformation_context));

  // %10 is a pointer to float, while %16 is an int constant
  ASSERT_FALSE(TransformationAddGlobalVariable(100, 10, SpvStorageClassPrivate,
                                               16, false)
                   .IsApplicable(context.get(), transformation_context));

  // %10 is a Private pointer to float, while %15 is a variable with type
  // Uniform float pointer
  ASSERT_FALSE(
      TransformationAddGlobalVariable(100, 10, SpvStorageClassPrivate, 15, true)
          .IsApplicable(context.get(), transformation_context));

  // %12 is a Private pointer to int, while %10 is a variable with type
  // Private float pointer
  ASSERT_FALSE(TransformationAddGlobalVariable(100, 12, SpvStorageClassPrivate,
                                               10, false)
                   .IsApplicable(context.get(), transformation_context));

  // %10 is pointer-to-float, and %14 has type pointer-to-float; that's not OK
  // since the initializer's type should be the *pointee* type.
  ASSERT_FALSE(
      TransformationAddGlobalVariable(104, 10, SpvStorageClassPrivate, 14, true)
          .IsApplicable(context.get(), transformation_context));

  // This would work in principle, but logical addressing does not allow
  // a pointer to a pointer.
  ASSERT_FALSE(TransformationAddGlobalVariable(104, 17, SpvStorageClassPrivate,
                                               14, false)
                   .IsApplicable(context.get(), transformation_context));

  TransformationAddGlobalVariable transformations[] = {
      // %100 = OpVariable %12 Private
      TransformationAddGlobalVariable(100, 12, SpvStorageClassPrivate, 16,
                                      true),

      // %101 = OpVariable %10 Private
      TransformationAddGlobalVariable(101, 10, SpvStorageClassPrivate, 40,
                                      false),

      // %102 = OpVariable %13 Private
      TransformationAddGlobalVariable(102, 13, SpvStorageClassPrivate, 41,
                                      true),

      // %103 = OpVariable %12 Private %16
      TransformationAddGlobalVariable(103, 12, SpvStorageClassPrivate, 16,
                                      false),

      // %104 = OpVariable %19 Private %21
      TransformationAddGlobalVariable(104, 19, SpvStorageClassPrivate, 21,
                                      true),

      // %105 = OpVariable %19 Private %22
      TransformationAddGlobalVariable(105, 19, SpvStorageClassPrivate, 22,
                                      false)};

  for (auto& transformation : transformations) {
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(100));
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(102));
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(104));
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(101));
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(103));
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(105));

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
         %40 = OpConstant %6 0
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %6 2
         %41 = OpConstantComposite %8 %40 %40
          %9 = OpTypePointer Function %6
         %10 = OpTypePointer Private %6
         %20 = OpTypePointer Uniform %6
         %11 = OpTypePointer Function %7
         %12 = OpTypePointer Private %7
         %13 = OpTypePointer Private %8
         %14 = OpVariable %10 Private
         %15 = OpVariable %20 Uniform
         %16 = OpConstant %7 1
         %17 = OpTypePointer Private %10
         %18 = OpTypeBool
         %19 = OpTypePointer Private %18
         %21 = OpConstantTrue %18
         %22 = OpConstantFalse %18
        %100 = OpVariable %12 Private %16
        %101 = OpVariable %10 Private %40
        %102 = OpVariable %13 Private %41
        %103 = OpVariable %12 Private %16
        %104 = OpVariable %19 Private %21
        %105 = OpVariable %19 Private %22
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddGlobalVariableTest, TestEntryPointInterfaceEnlargement) {
  // This checks that when global variables are added to a SPIR-V 1.4+ module,
  // they are also added to entry points of that module.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "m1"
               OpEntryPoint Vertex %5 "m2"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %6 2
          %9 = OpTypePointer Function %6
         %10 = OpTypePointer Private %6
         %20 = OpTypePointer Uniform %6
         %11 = OpTypePointer Function %7
         %12 = OpTypePointer Private %7
         %13 = OpTypePointer Private %8
         %14 = OpVariable %10 Private
         %15 = OpVariable %20 Uniform
         %16 = OpConstant %7 1
         %17 = OpTypePointer Private %10
         %18 = OpTypeBool
         %19 = OpTypePointer Private %18
         %21 = OpConstantTrue %18
          %4 = OpFunction %2 None %3
         %30 = OpLabel
               OpReturn
               OpFunctionEnd
          %5 = OpFunction %2 None %3
         %31 = OpLabel
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
  TransformationAddGlobalVariable transformations[] = {
      // %100 = OpVariable %12 Private
      TransformationAddGlobalVariable(100, 12, SpvStorageClassPrivate, 16,
                                      true),

      // %101 = OpVariable %12 Private %16
      TransformationAddGlobalVariable(101, 12, SpvStorageClassPrivate, 16,
                                      false),

      // %102 = OpVariable %19 Private %21
      TransformationAddGlobalVariable(102, 19, SpvStorageClassPrivate, 21,
                                      true)};

  for (auto& transformation : transformations) {
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(100));
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(102));
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(101));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "m1" %100 %101 %102
               OpEntryPoint Vertex %5 "m2" %100 %101 %102
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %6 2
          %9 = OpTypePointer Function %6
         %10 = OpTypePointer Private %6
         %20 = OpTypePointer Uniform %6
         %11 = OpTypePointer Function %7
         %12 = OpTypePointer Private %7
         %13 = OpTypePointer Private %8
         %14 = OpVariable %10 Private
         %15 = OpVariable %20 Uniform
         %16 = OpConstant %7 1
         %17 = OpTypePointer Private %10
         %18 = OpTypeBool
         %19 = OpTypePointer Private %18
         %21 = OpConstantTrue %18
        %100 = OpVariable %12 Private %16
        %101 = OpVariable %12 Private %16
        %102 = OpVariable %19 Private %21
          %4 = OpFunction %2 None %3
         %30 = OpLabel
               OpReturn
               OpFunctionEnd
          %5 = OpFunction %2 None %3
         %31 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddGlobalVariableTest, TestAddingWorkgroupGlobals) {
  // This checks that workgroup globals can be added to a compute shader.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Workgroup %6
         %50 = OpConstant %6 2
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
#ifndef NDEBUG
  ASSERT_DEATH(
      TransformationAddGlobalVariable(8, 7, SpvStorageClassWorkgroup, 50, true)
          .IsApplicable(context.get(), transformation_context),
      "By construction this transformation should not have an.*initializer "
      "when Workgroup storage class is used");
#endif

  TransformationAddGlobalVariable transformations[] = {
      // %8 = OpVariable %7 Workgroup
      TransformationAddGlobalVariable(8, 7, SpvStorageClassWorkgroup, 0, true),

      // %10 = OpVariable %7 Workgroup
      TransformationAddGlobalVariable(10, 7, SpvStorageClassWorkgroup, 0,
                                      false)};

  for (auto& transformation : transformations) {
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(8));
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(10));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main" %8 %10
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Workgroup %6
         %50 = OpConstant %6 2
          %8 = OpVariable %7 Workgroup
         %10 = OpVariable %7 Workgroup
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
