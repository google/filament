// Copyright (c) 2020 Vasyl Teliman
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

#include "source/fuzz/transformation_replace_parameter_with_global.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationReplaceParameterWithGlobalTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpMemberDecorate %13 0 RelaxedPrecision
               OpDecorate %16 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Private %6
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Private %8
         %10 = OpTypeVector %8 2
         %11 = OpTypePointer Private %10
         %12 = OpTypeBool
         %71 = OpTypeFunction %2 %6
         %83 = OpTypeFunction %2 %6 %12
         %93 = OpTypeFunction %2 %10
         %94 = OpTypeFunction %2 %8 %10
         %40 = OpTypePointer Function %12
         %13 = OpTypeStruct %6 %8
         %14 = OpTypePointer Private %13
         %15 = OpTypeFunction %2 %6 %8 %10 %13 %40 %12
         %22 = OpConstant %6 0
         %23 = OpConstant %8 0
         %26 = OpConstantComposite %10 %23 %23
         %27 = OpConstantTrue %12
         %28 = OpConstantComposite %13 %22 %23
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %41 = OpVariable %40 Function %27
         %33 = OpFunctionCall %2 %20 %22 %23 %26 %28 %41 %27
               OpReturn
               OpFunctionEnd

         ; adjust type of the function in-place
         %20 = OpFunction %2 None %15
         %16 = OpFunctionParameter %6
         %17 = OpFunctionParameter %8
         %18 = OpFunctionParameter %10
         %19 = OpFunctionParameter %13
         %42 = OpFunctionParameter %40
         %43 = OpFunctionParameter %12
         %21 = OpLabel
               OpReturn
               OpFunctionEnd

         ; reuse an existing function type
         %70 = OpFunction %2 None %71
         %72 = OpFunctionParameter %6
         %73 = OpLabel
               OpReturn
               OpFunctionEnd
         %74 = OpFunction %2 None %71
         %75 = OpFunctionParameter %6
         %76 = OpLabel
               OpReturn
               OpFunctionEnd

         ; create a new function type
         %77 = OpFunction %2 None %83
         %78 = OpFunctionParameter %6
         %84 = OpFunctionParameter %12
         %79 = OpLabel
               OpReturn
               OpFunctionEnd
         %80 = OpFunction %2 None %83
         %81 = OpFunctionParameter %6
         %85 = OpFunctionParameter %12
         %82 = OpLabel
               OpReturn
               OpFunctionEnd

         ; don't adjust the type of the function if it creates a duplicate
         %86 = OpFunction %2 None %93
         %87 = OpFunctionParameter %10
         %89 = OpLabel
               OpReturn
               OpFunctionEnd
         %90 = OpFunction %2 None %94
         %91 = OpFunctionParameter %8
         %95 = OpFunctionParameter %10
         %92 = OpLabel
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
  // Parameter id is invalid.
  ASSERT_FALSE(TransformationReplaceParameterWithGlobal(50, 50, 51)
                   .IsApplicable(context.get(), transformation_context));

  // Parameter id is not a result id of an OpFunctionParameter instruction.
  ASSERT_FALSE(TransformationReplaceParameterWithGlobal(50, 21, 51)
                   .IsApplicable(context.get(), transformation_context));

  // Parameter has unsupported type.
  ASSERT_FALSE(TransformationReplaceParameterWithGlobal(50, 42, 51)
                   .IsApplicable(context.get(), transformation_context));

  // Initializer for a global variable doesn't exist in the module.
  ASSERT_FALSE(TransformationReplaceParameterWithGlobal(50, 43, 51)
                   .IsApplicable(context.get(), transformation_context));

  // Pointer type for a global variable doesn't exist in the module.
  ASSERT_FALSE(TransformationReplaceParameterWithGlobal(50, 43, 51)
                   .IsApplicable(context.get(), transformation_context));

  // Function type id is not fresh.
  ASSERT_FALSE(TransformationReplaceParameterWithGlobal(16, 16, 51)
                   .IsApplicable(context.get(), transformation_context));

  // Global variable id is not fresh.
  ASSERT_FALSE(TransformationReplaceParameterWithGlobal(50, 16, 16)
                   .IsApplicable(context.get(), transformation_context));

  // Function type fresh id and global variable fresh id are equal.
  ASSERT_FALSE(TransformationReplaceParameterWithGlobal(50, 16, 50)
                   .IsApplicable(context.get(), transformation_context));

  {
    TransformationReplaceParameterWithGlobal transformation(50, 16, 51);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParameterWithGlobal transformation(52, 17, 53);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParameterWithGlobal transformation(54, 18, 55);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParameterWithGlobal transformation(56, 19, 57);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParameterWithGlobal transformation(58, 75, 59);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParameterWithGlobal transformation(60, 81, 61);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParameterWithGlobal transformation(62, 91, 63);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpMemberDecorate %13 0 RelaxedPrecision
               OpDecorate %16 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Private %6
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Private %8
         %10 = OpTypeVector %8 2
         %11 = OpTypePointer Private %10
         %12 = OpTypeBool
         %71 = OpTypeFunction %2 %6
         %83 = OpTypeFunction %2 %6 %12
         %93 = OpTypeFunction %2 %10
         %40 = OpTypePointer Function %12
         %13 = OpTypeStruct %6 %8
         %14 = OpTypePointer Private %13
         %22 = OpConstant %6 0
         %23 = OpConstant %8 0
         %26 = OpConstantComposite %10 %23 %23
         %27 = OpConstantTrue %12
         %28 = OpConstantComposite %13 %22 %23
         %51 = OpVariable %7 Private %22
         %53 = OpVariable %9 Private %23
         %55 = OpVariable %11 Private %26
         %57 = OpVariable %14 Private %28
         %15 = OpTypeFunction %2 %40 %12
         %59 = OpVariable %7 Private %22
         %61 = OpVariable %7 Private %22
         %60 = OpTypeFunction %2 %12
         %63 = OpVariable %9 Private %23
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %41 = OpVariable %40 Function %27
               OpStore %51 %22
               OpStore %53 %23
               OpStore %55 %26
               OpStore %57 %28
         %33 = OpFunctionCall %2 %20 %41 %27
               OpReturn
               OpFunctionEnd
         %20 = OpFunction %2 None %15
         %42 = OpFunctionParameter %40
         %43 = OpFunctionParameter %12
         %21 = OpLabel
         %19 = OpLoad %13 %57
         %18 = OpLoad %10 %55
         %17 = OpLoad %8 %53
         %16 = OpLoad %6 %51
               OpReturn
               OpFunctionEnd
         %70 = OpFunction %2 None %71
         %72 = OpFunctionParameter %6
         %73 = OpLabel
               OpReturn
               OpFunctionEnd
         %74 = OpFunction %2 None %3
         %76 = OpLabel
         %75 = OpLoad %6 %59
               OpReturn
               OpFunctionEnd
         %77 = OpFunction %2 None %83
         %78 = OpFunctionParameter %6
         %84 = OpFunctionParameter %12
         %79 = OpLabel
               OpReturn
               OpFunctionEnd
         %80 = OpFunction %2 None %60
         %85 = OpFunctionParameter %12
         %82 = OpLabel
         %81 = OpLoad %6 %61
               OpReturn
               OpFunctionEnd
         %86 = OpFunction %2 None %93
         %87 = OpFunctionParameter %10
         %89 = OpLabel
               OpReturn
               OpFunctionEnd
         %90 = OpFunction %2 None %93
         %95 = OpFunctionParameter %10
         %92 = OpLabel
         %91 = OpLoad %8 %63
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationReplaceParameterWithGlobalTest,
     HandlesIrrelevantParameters) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %9 = OpTypeInt 32 1
          %3 = OpTypeFunction %2
          %7 = OpTypeFunction %2 %9 %9
         %12 = OpTypePointer Private %9
         %13 = OpConstant %9 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %7
         %10 = OpFunctionParameter %9
         %11 = OpFunctionParameter %9
          %8 = OpLabel
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
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(10);

  {
    TransformationReplaceParameterWithGlobal transformation(20, 10, 21);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(21));
  }
  {
    TransformationReplaceParameterWithGlobal transformation(22, 11, 23);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(23));
  }
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
