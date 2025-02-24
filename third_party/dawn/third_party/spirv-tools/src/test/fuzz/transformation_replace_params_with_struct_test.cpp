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

#include "source/fuzz/transformation_replace_params_with_struct.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationReplaceParamsWithStructTest, BasicTest) {
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
          %6 = OpTypeInt 32 1
          %3 = OpTypeFunction %2
          %7 = OpTypePointer Private %6
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Private %8
         %10 = OpTypeVector %8 2
         %11 = OpTypePointer Private %10
         %12 = OpTypeBool
         %51 = OpTypeFunction %2 %12
         %64 = OpTypeStruct %6
         %63 = OpTypeFunction %2 %64
         %65 = OpTypeFunction %2 %6
         %75 = OpTypeStruct %8
         %76 = OpTypeFunction %2 %75
         %77 = OpTypeFunction %2 %8
         %40 = OpTypePointer Function %12
         %13 = OpTypeStruct %6 %8
         %45 = OpTypeStruct %6 %10 %13
         %46 = OpTypeStruct %12
         %47 = OpTypeStruct %8 %45 %46
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

         ; create a new function type
         %50 = OpFunction %2 None %51
         %52 = OpFunctionParameter %12
         %53 = OpLabel
               OpReturn
               OpFunctionEnd
         %54 = OpFunction %2 None %51
         %55 = OpFunctionParameter %12
         %56 = OpLabel
               OpReturn
               OpFunctionEnd

         ; reuse an existing function type
         %57 = OpFunction %2 None %63
         %58 = OpFunctionParameter %64
         %59 = OpLabel
               OpReturn
               OpFunctionEnd
         %60 = OpFunction %2 None %65
         %61 = OpFunctionParameter %6
         %62 = OpLabel
               OpReturn
               OpFunctionEnd
         %66 = OpFunction %2 None %65
         %67 = OpFunctionParameter %6
         %68 = OpLabel
               OpReturn
               OpFunctionEnd

         ; don't adjust the type of the function if it creates a duplicate
         %69 = OpFunction %2 None %76
         %70 = OpFunctionParameter %75
         %71 = OpLabel
               OpReturn
               OpFunctionEnd
         %72 = OpFunction %2 None %77
         %73 = OpFunctionParameter %8
         %74 = OpLabel
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
  // |parameter_id| is empty.
  ASSERT_FALSE(
      TransformationReplaceParamsWithStruct({}, 90, 91, {{33, 92}, {90, 93}})
          .IsApplicable(context.get(), transformation_context));

  // |parameter_id| has duplicates.
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 16, 17}, 90, 91,
                                                     {{33, 92}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));

  // |parameter_id| has invalid values.
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({21, 16, 17}, 90, 91,
                                                     {{33, 92}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 90, 17}, 90, 91,
                                                     {{33, 92}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));

  // Parameter's belong to different functions.
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 17, 52}, 90, 91,
                                                     {{33, 92}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));

  // Parameter has unsupported type.
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 17, 42, 43}, 90, 91,
                                                     {{33, 92}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));

  // OpTypeStruct does not exist in the module.
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 43}, 90, 91,
                                                     {{33, 92}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));

  // |caller_id_to_fresh_composite_id| misses values.
  ASSERT_FALSE(
      TransformationReplaceParamsWithStruct({16, 17}, 90, 91, {{90, 93}})
          .IsApplicable(context.get(), transformation_context));

  // All fresh ids must be unique.
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 17}, 90, 90,
                                                     {{33, 92}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 17}, 90, 91,
                                                     {{33, 90}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 17}, 90, 91,
                                                     {{33, 92}, {90, 92}})
                   .IsApplicable(context.get(), transformation_context));

  // All 'fresh' ids must be fresh.
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 17}, 90, 91,
                                                     {{33, 33}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 17}, 33, 91,
                                                     {{33, 92}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationReplaceParamsWithStruct({16, 17}, 90, 33,
                                                     {{33, 92}, {90, 93}})
                   .IsApplicable(context.get(), transformation_context));

  {
    TransformationReplaceParamsWithStruct transformation({16, 18, 19}, 90, 91,
                                                         {{33, 92}, {90, 93}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParamsWithStruct transformation({43}, 93, 94,
                                                         {{33, 95}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParamsWithStruct transformation({17, 91, 94}, 96, 97,
                                                         {{33, 98}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParamsWithStruct transformation({55}, 99, 100, {{}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParamsWithStruct transformation({61}, 101, 102, {{}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  {
    TransformationReplaceParamsWithStruct transformation({73}, 103, 104, {{}});
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
          %6 = OpTypeInt 32 1
          %3 = OpTypeFunction %2
          %7 = OpTypePointer Private %6
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Private %8
         %10 = OpTypeVector %8 2
         %11 = OpTypePointer Private %10
         %12 = OpTypeBool
         %51 = OpTypeFunction %2 %12
         %64 = OpTypeStruct %6
         %63 = OpTypeFunction %2 %64
         %65 = OpTypeFunction %2 %6
         %75 = OpTypeStruct %8
         %76 = OpTypeFunction %2 %75
         %40 = OpTypePointer Function %12
         %13 = OpTypeStruct %6 %8
         %45 = OpTypeStruct %6 %10 %13
         %46 = OpTypeStruct %12
         %47 = OpTypeStruct %8 %45 %46
         %14 = OpTypePointer Private %13
         %22 = OpConstant %6 0
         %23 = OpConstant %8 0
         %26 = OpConstantComposite %10 %23 %23
         %27 = OpConstantTrue %12
         %28 = OpConstantComposite %13 %22 %23
         %15 = OpTypeFunction %2 %40 %47
         %99 = OpTypeFunction %2 %46
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %41 = OpVariable %40 Function %27
         %92 = OpCompositeConstruct %45 %22 %26 %28
         %95 = OpCompositeConstruct %46 %27
         %98 = OpCompositeConstruct %47 %23 %92 %95
         %33 = OpFunctionCall %2 %20 %41 %98
               OpReturn
               OpFunctionEnd
         %20 = OpFunction %2 None %15
         %42 = OpFunctionParameter %40
         %97 = OpFunctionParameter %47
         %21 = OpLabel
         %94 = OpCompositeExtract %46 %97 2
         %91 = OpCompositeExtract %45 %97 1
         %17 = OpCompositeExtract %8 %97 0
         %43 = OpCompositeExtract %12 %94 0
         %19 = OpCompositeExtract %13 %91 2
         %18 = OpCompositeExtract %10 %91 1
         %16 = OpCompositeExtract %6 %91 0
               OpReturn
               OpFunctionEnd
         %50 = OpFunction %2 None %51
         %52 = OpFunctionParameter %12
         %53 = OpLabel
               OpReturn
               OpFunctionEnd
         %54 = OpFunction %2 None %99
        %100 = OpFunctionParameter %46
         %56 = OpLabel
         %55 = OpCompositeExtract %12 %100 0
               OpReturn
               OpFunctionEnd
         %57 = OpFunction %2 None %63
         %58 = OpFunctionParameter %64
         %59 = OpLabel
               OpReturn
               OpFunctionEnd
         %60 = OpFunction %2 None %63
        %102 = OpFunctionParameter %64
         %62 = OpLabel
         %61 = OpCompositeExtract %6 %102 0
               OpReturn
               OpFunctionEnd
         %66 = OpFunction %2 None %65
         %67 = OpFunctionParameter %6
         %68 = OpLabel
               OpReturn
               OpFunctionEnd
         %69 = OpFunction %2 None %76
         %70 = OpFunctionParameter %75
         %71 = OpLabel
               OpReturn
               OpFunctionEnd
         %72 = OpFunction %2 None %76
        %104 = OpFunctionParameter %75
         %74 = OpLabel
         %73 = OpCompositeExtract %8 %104 0
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationReplaceParamsWithStructTest, ParametersRemainValid) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %6 = OpTypeInt 32 1
          %3 = OpTypeFunction %2
          %8 = OpTypeFloat 32
         %10 = OpTypeVector %8 2
         %12 = OpTypeBool
         %40 = OpTypePointer Function %12
         %13 = OpTypeStruct %6 %8
         %45 = OpTypeStruct %6 %8 %13
         %47 = OpTypeStruct %45 %12 %10
         %15 = OpTypeFunction %2 %6 %8 %10 %13 %40 %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
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
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  {
    // Try to replace parameters in "increasing" order of their declaration.
    TransformationReplaceParamsWithStruct transformation({16, 17, 19}, 70, 71,
                                                         {{}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %6 = OpTypeInt 32 1
          %3 = OpTypeFunction %2
          %8 = OpTypeFloat 32
         %10 = OpTypeVector %8 2
         %12 = OpTypeBool
         %40 = OpTypePointer Function %12
         %13 = OpTypeStruct %6 %8
         %45 = OpTypeStruct %6 %8 %13
         %47 = OpTypeStruct %45 %12 %10
         %15 = OpTypeFunction %2 %10 %40 %12 %45
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %20 = OpFunction %2 None %15
         %18 = OpFunctionParameter %10
         %42 = OpFunctionParameter %40
         %43 = OpFunctionParameter %12
         %71 = OpFunctionParameter %45
         %21 = OpLabel
         %19 = OpCompositeExtract %13 %71 2
         %17 = OpCompositeExtract %8 %71 1
         %16 = OpCompositeExtract %6 %71 0
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));

  {
    // Try to replace parameters in "decreasing" order of their declaration.
    TransformationReplaceParamsWithStruct transformation({71, 43, 18}, 72, 73,
                                                         {{}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %6 = OpTypeInt 32 1
          %3 = OpTypeFunction %2
          %8 = OpTypeFloat 32
         %10 = OpTypeVector %8 2
         %12 = OpTypeBool
         %40 = OpTypePointer Function %12
         %13 = OpTypeStruct %6 %8
         %45 = OpTypeStruct %6 %8 %13
         %47 = OpTypeStruct %45 %12 %10
         %15 = OpTypeFunction %2 %40 %47
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %20 = OpFunction %2 None %15
         %42 = OpFunctionParameter %40
         %73 = OpFunctionParameter %47
         %21 = OpLabel
         %18 = OpCompositeExtract %10 %73 2
         %43 = OpCompositeExtract %12 %73 1
         %71 = OpCompositeExtract %45 %73 0
         %19 = OpCompositeExtract %13 %71 2
         %17 = OpCompositeExtract %8 %71 1
         %16 = OpCompositeExtract %6 %71 0
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceParamsWithStructTest, IsomorphicStructs) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %16 "main"
               OpExecutionMode %16 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %6 = OpTypeInt 32 1
          %7 = OpTypeStruct %6
          %8 = OpTypeStruct %6
          %9 = OpTypeStruct %8
         %10 = OpTypeFunction %2 %7
         %15 = OpTypeFunction %2
         %16 = OpFunction %2 None %15
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %2 None %10
         %12 = OpFunctionParameter %7
         %13 = OpLabel
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

  ASSERT_FALSE(TransformationReplaceParamsWithStruct({12}, 100, 101, {{}})
                   .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
