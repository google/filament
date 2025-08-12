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

#include "source/fuzz/transformation_outline_function.h"

#include "gtest/gtest.h"
#include "source/fuzz/counter_overflow_id_source.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationOutlineFunctionTest, TrivialOutline) {
  // This tests outlining of a single, empty basic block.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
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
  TransformationOutlineFunction transformation(5, 5, /* not relevant */ 200,
                                               100, 101, 102, 103,
                                               /* not relevant */ 201, {}, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %103 = OpFunctionCall %2 %101
               OpReturn
               OpFunctionEnd
        %101 = OpFunction %2 None %3
        %102 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineIfRegionStartsWithOpVariable) {
  // This checks that we do not outline the first block of a function if it
  // contains OpVariable.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %7 = OpTypeBool
          %8 = OpTypePointer Function %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %6 = OpVariable %8 Function
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
  TransformationOutlineFunction transformation(5, 5, /* not relevant */ 200,
                                               100, 101, 102, 103,
                                               /* not relevant */ 201, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest, OutlineInterestingControlFlowNoState) {
  // This tests outlining of some non-trivial control flow, but such that the
  // basic blocks in the control flow do not actually do anything.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeBool
         %21 = OpConstantTrue %20
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %21 %8 %9
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %12 %11 None
               OpBranch %10
         %10 = OpLabel
               OpBranchConditional %21 %11 %12
         %11 = OpLabel
               OpBranch %9
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
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
  TransformationOutlineFunction transformation(6, 13, /* not relevant */
                                               200, 100, 101, 102, 103,
                                               /* not relevant */ 201, {}, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeBool
         %21 = OpConstantTrue %20
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
        %103 = OpFunctionCall %2 %101
               OpReturn
               OpFunctionEnd
        %101 = OpFunction %2 None %3
        %102 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %21 %8 %9
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %12 %11 None
               OpBranch %10
         %10 = OpLabel
               OpBranchConditional %21 %11 %12
         %11 = OpLabel
               OpBranch %9
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, OutlineCodeThatGeneratesUnusedIds) {
  // This tests outlining of a single basic block that does some computation,
  // but that does not use nor generate ids required outside of the outlined
  // region.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeInt 32 1
         %21 = OpConstant %20 5
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
          %7 = OpCopyObject %20 %21
          %8 = OpCopyObject %20 %21
          %9 = OpIAdd %20 %7 %8
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
  TransformationOutlineFunction transformation(6, 6, /* not relevant */ 200,
                                               100, 101, 102, 103,
                                               /* not relevant */ 201, {}, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeInt 32 1
         %21 = OpConstant %20 5
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
        %103 = OpFunctionCall %2 %101
               OpReturn
               OpFunctionEnd
        %101 = OpFunction %2 None %3
        %102 = OpLabel
          %7 = OpCopyObject %20 %21
          %8 = OpCopyObject %20 %21
          %9 = OpIAdd %20 %7 %8
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, OutlineCodeThatGeneratesSingleUsedId) {
  // This tests outlining of a block that generates an id that is used in a
  // later block.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeInt 32 1
         %21 = OpConstant %20 5
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
          %7 = OpCopyObject %20 %21
          %8 = OpCopyObject %20 %21
          %9 = OpIAdd %20 %7 %8
               OpBranch %10
         %10 = OpLabel
         %11 = OpCopyObject %20 %9
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
  TransformationOutlineFunction transformation(6, 6, 99, 100, 101, 102, 103,
                                               105, {}, {{9, 104}});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeInt 32 1
         %21 = OpConstant %20 5
          %3 = OpTypeFunction %2
         %99 = OpTypeStruct %20
        %100 = OpTypeFunction %99
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
        %103 = OpFunctionCall %99 %101
          %9 = OpCompositeExtract %20 %103 0
               OpBranch %10
         %10 = OpLabel
         %11 = OpCopyObject %20 %9
               OpReturn
               OpFunctionEnd
        %101 = OpFunction %99 None %100
        %102 = OpLabel
          %7 = OpCopyObject %20 %21
          %8 = OpCopyObject %20 %21
        %104 = OpIAdd %20 %7 %8
        %105 = OpCompositeConstruct %99 %104
               OpReturnValue %105
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, OutlineDiamondThatGeneratesSeveralIds) {
  // This tests outlining of several blocks that generate a number of ids that
  // are used in later blocks.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeInt 32 1
         %21 = OpConstant %20 5
         %22 = OpTypeBool
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
          %7 = OpCopyObject %20 %21
          %8 = OpCopyObject %20 %21
          %9 = OpSLessThan %22 %7 %8
               OpSelectionMerge %12 None
               OpBranchConditional %9 %10 %11
         %10 = OpLabel
         %13 = OpIAdd %20 %7 %8
               OpBranch %12
         %11 = OpLabel
         %14 = OpIAdd %20 %7 %7
               OpBranch %12
         %12 = OpLabel
         %15 = OpPhi %20 %13 %10 %14 %11
               OpBranch %80
         %80 = OpLabel
               OpBranch %16
         %16 = OpLabel
         %17 = OpCopyObject %20 %15
         %18 = OpCopyObject %22 %9
         %19 = OpIAdd %20 %7 %8
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
  TransformationOutlineFunction transformation(
      6, 80, 100, 101, 102, 103, 104, 105, {},
      {{15, 106}, {9, 107}, {7, 108}, {8, 109}});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeInt 32 1
         %21 = OpConstant %20 5
         %22 = OpTypeBool
          %3 = OpTypeFunction %2
        %100 = OpTypeStruct %20 %20 %22 %20
        %101 = OpTypeFunction %100
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
        %104 = OpFunctionCall %100 %102
          %7 = OpCompositeExtract %20 %104 0
          %8 = OpCompositeExtract %20 %104 1
          %9 = OpCompositeExtract %22 %104 2
         %15 = OpCompositeExtract %20 %104 3
               OpBranch %16
         %16 = OpLabel
         %17 = OpCopyObject %20 %15
         %18 = OpCopyObject %22 %9
         %19 = OpIAdd %20 %7 %8
               OpReturn
               OpFunctionEnd
        %102 = OpFunction %100 None %101
        %103 = OpLabel
        %108 = OpCopyObject %20 %21
        %109 = OpCopyObject %20 %21
        %107 = OpSLessThan %22 %108 %109
               OpSelectionMerge %12 None
               OpBranchConditional %107 %10 %11
         %10 = OpLabel
         %13 = OpIAdd %20 %108 %109
               OpBranch %12
         %11 = OpLabel
         %14 = OpIAdd %20 %108 %108
               OpBranch %12
         %12 = OpLabel
        %106 = OpPhi %20 %13 %10 %14 %11
               OpBranch %80
         %80 = OpLabel
        %105 = OpCompositeConstruct %100 %108 %109 %107 %106
               OpReturnValue %105
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, OutlineCodeThatUsesASingleId) {
  // This tests outlining of a block that uses an id defined earlier.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeInt 32 1
         %21 = OpConstant %20 5
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %7 = OpCopyObject %20 %21
               OpBranch %6
          %6 = OpLabel
          %8 = OpCopyObject %20 %7
               OpBranch %10
         %10 = OpLabel
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
  TransformationOutlineFunction transformation(6, 6, 100, 101, 102, 103, 104,
                                               105, {{7, 106}}, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeInt 32 1
         %21 = OpConstant %20 5
          %3 = OpTypeFunction %2
        %101 = OpTypeFunction %2 %20
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %7 = OpCopyObject %20 %21
               OpBranch %6
          %6 = OpLabel
        %104 = OpFunctionCall %2 %102 %7
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
        %102 = OpFunction %2 None %101
        %106 = OpFunctionParameter %20
        %103 = OpLabel
          %8 = OpCopyObject %20 %106
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, OutlineCodeThatUsesAVariable) {
  // This tests outlining of a block that uses a variable.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeInt 32 1
         %21 = OpConstant %20 5
          %3 = OpTypeFunction %2
         %12 = OpTypePointer Function %20
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %13 = OpVariable %12 Function
               OpBranch %6
          %6 = OpLabel
          %8 = OpLoad %20 %13
               OpBranch %10
         %10 = OpLabel
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
  TransformationOutlineFunction transformation(6, 6, 100, 101, 102, 103, 104,
                                               105, {{13, 106}}, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeInt 32 1
         %21 = OpConstant %20 5
          %3 = OpTypeFunction %2
         %12 = OpTypePointer Function %20
        %101 = OpTypeFunction %2 %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %13 = OpVariable %12 Function
               OpBranch %6
          %6 = OpLabel
        %104 = OpFunctionCall %2 %102 %13
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
        %102 = OpFunction %2 None %101
        %106 = OpFunctionParameter %12
        %103 = OpLabel
          %8 = OpLoad %20 %106
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, OutlineCodeThatUsesAParameter) {
  // This tests outlining of a block that uses a function parameter.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "foo(i1;"
               OpName %9 "x"
               OpName %18 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %6 %7
         %13 = OpConstant %6 1
         %17 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %18 = OpVariable %7 Function
               OpStore %18 %17
         %19 = OpFunctionCall %6 %10 %18
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpLoad %6 %9
         %14 = OpIAdd %6 %12 %13
               OpReturnValue %14
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
  TransformationOutlineFunction transformation(11, 11, 100, 101, 102, 103, 104,
                                               105, {{9, 106}}, {{14, 107}});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "foo(i1;"
               OpName %9 "x"
               OpName %18 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %6 %7
         %13 = OpConstant %6 1
         %17 = OpConstant %6 3
        %100 = OpTypeStruct %6
        %101 = OpTypeFunction %100 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %18 = OpVariable %7 Function
               OpStore %18 %17
         %19 = OpFunctionCall %6 %10 %18
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
        %104 = OpFunctionCall %100 %102 %9
         %14 = OpCompositeExtract %6 %104 0
               OpReturnValue %14
               OpFunctionEnd
        %102 = OpFunction %100 None %101
        %106 = OpFunctionParameter %7
        %103 = OpLabel
         %12 = OpLoad %6 %106
        %107 = OpIAdd %6 %12 %13
        %105 = OpCompositeConstruct %100 %107
               OpReturnValue %105
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineIfLoopMergeIsOutsideRegion) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %9 = OpTypeBool
         %10 = OpConstantTrue %9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpLoopMerge %7 %8 None
               OpBranch %8
          %8 = OpLabel
               OpBranchConditional %10 %6 %7
          %7 = OpLabel
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
  TransformationOutlineFunction transformation(6, 8, 100, 101, 102, 103, 104,
                                               105, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest, DoNotOutlineIfRegionInvolvesReturn) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeBool
         %21 = OpConstantTrue %20
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %21 %8 %9
          %8 = OpLabel
               OpReturn
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpBranch %12
         %12 = OpLabel
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
  TransformationOutlineFunction transformation(6, 11, /* not relevant */ 200,
                                               100, 101, 102, 103,
                                               /* not relevant */ 201, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest, DoNotOutlineIfRegionInvolvesKill) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeBool
         %21 = OpConstantTrue %20
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %21 %8 %9
          %8 = OpLabel
               OpKill
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpBranch %12
         %12 = OpLabel
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
  TransformationOutlineFunction transformation(6, 11, /* not relevant */ 200,
                                               100, 101, 102, 103,
                                               /* not relevant */ 201, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineIfRegionInvolvesUnreachable) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
         %20 = OpTypeBool
         %21 = OpConstantTrue %20
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %21 %8 %9
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpUnreachable
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpBranch %12
         %12 = OpLabel
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
  TransformationOutlineFunction transformation(6, 11, /* not relevant */ 200,
                                               100, 101, 102, 103,
                                               /* not relevant */ 201, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineIfSelectionMergeIsOutsideRegion) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %9 = OpTypeBool
         %10 = OpConstantTrue %9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpSelectionMerge %7 None
               OpBranchConditional %10 %8 %7
          %8 = OpLabel
               OpBranch %7
          %7 = OpLabel
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
  TransformationOutlineFunction transformation(6, 8, 100, 101, 102, 103, 104,
                                               105, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest, DoNotOutlineIfLoopHeadIsOutsideRegion) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %9 = OpTypeBool
         %10 = OpConstantTrue %9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpLoopMerge %8 %11 None
               OpBranch %7
          %7 = OpLabel
               OpBranchConditional %10 %11 %8
         %11 = OpLabel
               OpBranch %6
          %8 = OpLabel
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
  TransformationOutlineFunction transformation(7, 8, 100, 101, 102, 103, 104,
                                               105, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineIfLoopContinueIsOutsideRegion) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %9 = OpTypeBool
         %10 = OpConstantTrue %9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpLoopMerge %7 %8 None
               OpBranch %7
          %8 = OpLabel
               OpBranch %6
          %7 = OpLabel
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
  TransformationOutlineFunction transformation(6, 7, 100, 101, 102, 103, 104,
                                               105, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineWithLoopCarriedPhiDependence) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %9 = OpTypeBool
         %10 = OpConstantTrue %9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
         %12 = OpPhi %9 %10 %5 %13 %8
               OpLoopMerge %7 %8 None
               OpBranch %8
          %8 = OpLabel
         %13 = OpCopyObject %9 %10
               OpBranchConditional %10 %6 %7
          %7 = OpLabel
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
  TransformationOutlineFunction transformation(6, 7, 100, 101, 102, 103, 104,
                                               105, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineSelectionHeaderNotInRegion) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %7 %8 %8
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
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
  TransformationOutlineFunction transformation(8, 11, 100, 101, 102, 103, 104,
                                               105, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest, OutlineRegionEndingWithReturnVoid) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
         %20 = OpTypeInt 32 0
         %21 = OpConstant %20 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %22 = OpCopyObject %20 %21
               OpBranch %54
         %54 = OpLabel
               OpBranch %57
         %57 = OpLabel
         %23 = OpCopyObject %20 %22
               OpBranch %58
         %58 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 54,
      /*exit_block*/ 58,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {{22, 206}},
      /*output_id_to_fresh_id*/ {});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
         %20 = OpTypeInt 32 0
         %21 = OpConstant %20 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
        %201 = OpTypeFunction %2 %20
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %22 = OpCopyObject %20 %21
               OpBranch %54
         %54 = OpLabel
        %204 = OpFunctionCall %2 %202 %22
               OpReturn
               OpFunctionEnd
        %202 = OpFunction %2 None %201
        %206 = OpFunctionParameter %20
        %203 = OpLabel
               OpBranch %57
         %57 = OpLabel
         %23 = OpCopyObject %20 %206
               OpBranch %58
         %58 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, OutlineRegionEndingWithReturnValue) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
         %20 = OpTypeInt 32 0
         %21 = OpConstant %20 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %30 = OpTypeFunction %20
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %6 = OpFunctionCall %20 %100
               OpReturn
               OpFunctionEnd
        %100 = OpFunction %20 None %30
          %8 = OpLabel
         %31 = OpCopyObject %20 %21
               OpBranch %9
          %9 = OpLabel
         %32 = OpCopyObject %20 %31
               OpBranch %10
         %10 = OpLabel
               OpReturnValue %32
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 9,
      /*exit_block*/ 10,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {{31, 206}},
      /*output_id_to_fresh_id*/ {{32, 207}});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
         %20 = OpTypeInt 32 0
         %21 = OpConstant %20 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %30 = OpTypeFunction %20
        %200 = OpTypeStruct %20
        %201 = OpTypeFunction %200 %20
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %6 = OpFunctionCall %20 %100
               OpReturn
               OpFunctionEnd
        %100 = OpFunction %20 None %30
          %8 = OpLabel
         %31 = OpCopyObject %20 %21
               OpBranch %9
          %9 = OpLabel
        %204 = OpFunctionCall %200 %202 %31
         %32 = OpCompositeExtract %20 %204 0
               OpReturnValue %32
               OpFunctionEnd
        %202 = OpFunction %200 None %201
        %206 = OpFunctionParameter %20
        %203 = OpLabel
        %207 = OpCopyObject %20 %206
               OpBranch %10
         %10 = OpLabel
        %205 = OpCompositeConstruct %200 %207
               OpReturnValue %205
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest,
     OutlineRegionEndingWithConditionalBranch) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
         %20 = OpTypeBool
         %21 = OpConstantTrue %20
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %54
         %54 = OpLabel
          %6 = OpCopyObject %20 %21
               OpSelectionMerge %8 None
               OpBranchConditional %6 %7 %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 54,
      /*exit_block*/ 54,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {{}},
      /*output_id_to_fresh_id*/ {{6, 206}});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
         %20 = OpTypeBool
         %21 = OpConstantTrue %20
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
        %200 = OpTypeStruct %20
        %201 = OpTypeFunction %200
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %54
         %54 = OpLabel
        %204 = OpFunctionCall %200 %202
          %6 = OpCompositeExtract %20 %204 0
               OpSelectionMerge %8 None
               OpBranchConditional %6 %7 %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
        %202 = OpFunction %200 None %201
        %203 = OpLabel
        %206 = OpCopyObject %20 %21
        %205 = OpCompositeConstruct %200 %206
               OpReturnValue %205
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest,
     OutlineRegionEndingWithConditionalBranch2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
         %20 = OpTypeBool
         %21 = OpConstantTrue %20
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %6 = OpCopyObject %20 %21
               OpBranch %54
         %54 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %6 %7 %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 54,
      /*exit_block*/ 54,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {},
      /*output_id_to_fresh_id*/ {});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
         %20 = OpTypeBool
         %21 = OpConstantTrue %20
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %6 = OpCopyObject %20 %21
               OpBranch %54
         %54 = OpLabel
        %204 = OpFunctionCall %2 %202
               OpSelectionMerge %8 None
               OpBranchConditional %6 %7 %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
        %202 = OpFunction %2 None %3
        %203 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, DoNotOutlineRegionThatStartsWithOpPhi) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %21
         %21 = OpLabel
         %22 = OpPhi %6 %7 %5
         %23 = OpCopyObject %6 %22
               OpBranch %24
         %24 = OpLabel
         %25 = OpCopyObject %6 %23
         %26 = OpCopyObject %6 %22
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 21,
      /*exit_block*/ 21,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 204,
      /*new_caller_result_id*/ 205,
      /*new_callee_result_id*/ 206,
      /*input_id_to_fresh_id*/ {{22, 207}},
      /*output_id_to_fresh_id*/ {{23, 208}});

  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineRegionThatStartsWithLoopHeader) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %21
         %21 = OpLabel
               OpLoopMerge %22 %23 None
               OpBranch %24
         %24 = OpLabel
               OpBranchConditional %7 %22 %23
         %23 = OpLabel
               OpBranch %21
         %22 = OpLabel
               OpBranch %25
         %25 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 21,
      /*exit_block*/ 24,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 204,
      /*new_caller_result_id*/ 205,
      /*new_callee_result_id*/ 206,
      /*input_id_to_fresh_id*/ {},
      /*output_id_to_fresh_id*/ {});

  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineRegionThatEndsWithLoopMerge) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %21
         %21 = OpLabel
               OpLoopMerge %22 %23 None
               OpBranch %24
         %24 = OpLabel
               OpBranchConditional %7 %22 %23
         %23 = OpLabel
               OpBranch %21
         %22 = OpLabel
               OpBranch %25
         %25 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 5,
      /*exit_block*/ 22,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 204,
      /*new_caller_result_id*/ 205,
      /*new_callee_result_id*/ 206,
      /*input_id_to_fresh_id*/ {},
      /*output_id_to_fresh_id*/ {});

  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest, DoNotOutlineRegionThatUsesAccessChain) {
  // An access chain result is a pointer, but it cannot be passed as a function
  // parameter, as it is not a memory object.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Function %7
          %9 = OpTypePointer Function %6
         %18 = OpTypeInt 32 0
         %19 = OpConstant %18 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %10 = OpVariable %8 Function
               OpBranch %11
         %11 = OpLabel
         %12 = OpAccessChain %9 %10 %19
               OpBranch %13
         %13 = OpLabel
         %14 = OpLoad %6 %12
               OpBranch %15
         %15 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 13,
      /*exit_block*/ 15,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 204,
      /*new_caller_result_id*/ 205,
      /*new_callee_result_id*/ 206,
      /*input_id_to_fresh_id*/ {{12, 207}},
      /*output_id_to_fresh_id*/ {});

  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineRegionThatUsesCopiedObject) {
  // Copying a variable leads to a pointer, but one that cannot be passed as a
  // function parameter, as it is not a memory object.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Function %7
          %9 = OpTypePointer Function %6
         %18 = OpTypeInt 32 0
         %19 = OpConstant %18 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %10 = OpVariable %8 Function
               OpBranch %11
         %11 = OpLabel
         %20 = OpCopyObject %8 %10
               OpBranch %13
         %13 = OpLabel
         %12 = OpAccessChain %9 %20 %19
         %14 = OpLoad %6 %12
               OpBranch %15
         %15 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 13,
      /*exit_block*/ 15,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 204,
      /*new_caller_result_id*/ 205,
      /*new_callee_result_id*/ 206,
      /*input_id_to_fresh_id*/ {{20, 207}},
      /*output_id_to_fresh_id*/ {});

  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest,
     DoOutlineRegionThatUsesPointerParameter) {
  // The region being outlined reads from a function parameter of pointer type.
  // This is OK: the function parameter can itself be passed on as a function
  // parameter.
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
         %13 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %15 = OpVariable %7 Function
         %16 = OpVariable %7 Function
         %17 = OpLoad %6 %15
               OpStore %16 %17
         %18 = OpFunctionCall %2 %10 %16
         %19 = OpLoad %6 %16
               OpStore %15 %19
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpLoad %6 %9
         %14 = OpIAdd %6 %12 %13
               OpBranch %20
         %20 = OpLabel
               OpStore %9 %14
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 11,
      /*exit_block*/ 11,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 204,
      /*new_caller_result_id*/ 205,
      /*new_callee_result_id*/ 206,
      /*input_id_to_fresh_id*/ {{9, 207}},
      /*output_id_to_fresh_id*/ {{14, 208}});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
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
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 2
        %200 = OpTypeStruct %6
        %201 = OpTypeFunction %200 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %15 = OpVariable %7 Function
         %16 = OpVariable %7 Function
         %17 = OpLoad %6 %15
               OpStore %16 %17
         %18 = OpFunctionCall %2 %10 %16
         %19 = OpLoad %6 %16
               OpStore %15 %19
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
        %205 = OpFunctionCall %200 %202 %9
         %14 = OpCompositeExtract %6 %205 0
               OpBranch %20
         %20 = OpLabel
               OpStore %9 %14
               OpReturn
               OpFunctionEnd
        %202 = OpFunction %200 None %201
        %207 = OpFunctionParameter %7
        %204 = OpLabel
         %12 = OpLoad %6 %207
        %208 = OpIAdd %6 %12 %13
        %206 = OpCompositeConstruct %200 %208
               OpReturnValue %206
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, OutlineLivesafe) {
  // In the following, %30 is a livesafe function, with irrelevant parameter
  // %200 and irrelevant local variable %201.  Variable %100 is a loop limiter,
  // which is not irrelevant.  The test checks that the outlined function is
  // livesafe, and that the parameters corresponding to %200 and %201 have the
  // irrelevant fact associated with them.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
        %199 = OpTypeFunction %2 %7
          %8 = OpConstant %6 0
          %9 = OpConstant %6 1
         %10 = OpConstant %6 5
         %11 = OpTypeBool
         %12 = OpConstantTrue %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %30 = OpFunction %2 None %199
        %200 = OpFunctionParameter %7
         %31 = OpLabel
        %100 = OpVariable %7 Function %8
        %201 = OpVariable %7 Function %8
               OpBranch %198
        %198 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %101 = OpLoad %6 %100
        %102 = OpIAdd %6 %101 %9
        %202 = OpLoad %6 %200
               OpStore %201 %202
               OpStore %100 %102
        %103 = OpUGreaterThanEqual %11 %101 %10
               OpLoopMerge %21 %22 None
               OpBranchConditional %103 %21 %104
        %104 = OpLabel
               OpBranchConditional %12 %23 %21
         %23 = OpLabel
        %105 = OpLoad %6 %100
        %106 = OpIAdd %6 %105 %9
               OpStore %100 %106
        %107 = OpUGreaterThanEqual %11 %105 %10
               OpLoopMerge %25 %26 None
               OpBranchConditional %107 %25 %108
        %108 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpBranchConditional %12 %26 %25
         %26 = OpLabel
               OpBranch %23
         %25 = OpLabel
        %109 = OpLoad %6 %100
        %110 = OpIAdd %6 %109 %9
               OpStore %100 %110
        %111 = OpUGreaterThanEqual %11 %109 %10
               OpLoopMerge %24 %27 None
               OpBranchConditional %111 %24 %112
        %112 = OpLabel
               OpBranchConditional %12 %24 %27
         %27 = OpLabel
               OpBranch %25
         %24 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpBranch %20
         %21 = OpLabel
               OpBranch %197
        %197 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  transformation_context.GetFactManager()->AddFactFunctionIsLivesafe(30);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      200);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      201);

  TransformationOutlineFunction transformation(
      /*entry_block*/ 198,
      /*exit_block*/ 197,
      /*new_function_struct_return_type_id*/ 400,
      /*new_function_type_id*/ 401,
      /*new_function_id*/ 402,
      /*new_function_region_entry_block*/ 404,
      /*new_caller_result_id*/ 405,
      /*new_callee_result_id*/ 406,
      /*input_id_to_fresh_id*/ {{100, 407}, {200, 408}, {201, 409}},
      /*output_id_to_fresh_id*/ {});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // The original function should still be livesafe.
  ASSERT_TRUE(transformation_context.GetFactManager()->FunctionIsLivesafe(30));
  // The outlined function should be livesafe.
  ASSERT_TRUE(transformation_context.GetFactManager()->FunctionIsLivesafe(402));
  // The variable and parameter that were originally irrelevant should still be.
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(200));
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(201));
  // The loop limiter should still be non-irrelevant.
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(100));
  // The parameters for the original irrelevant variables should be irrelevant.
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(408));
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(409));
  // The parameter for the loop limiter should not be irrelevant.
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(407));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
        %199 = OpTypeFunction %2 %7
          %8 = OpConstant %6 0
          %9 = OpConstant %6 1
         %10 = OpConstant %6 5
         %11 = OpTypeBool
         %12 = OpConstantTrue %11
        %401 = OpTypeFunction %2 %7 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %30 = OpFunction %2 None %199
        %200 = OpFunctionParameter %7
         %31 = OpLabel
        %100 = OpVariable %7 Function %8
        %201 = OpVariable %7 Function %8
               OpBranch %198
        %198 = OpLabel
        %405 = OpFunctionCall %2 %402 %200 %100 %201
               OpReturn
               OpFunctionEnd
        %402 = OpFunction %2 None %401
        %408 = OpFunctionParameter %7
        %407 = OpFunctionParameter %7
        %409 = OpFunctionParameter %7
        %404 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %101 = OpLoad %6 %407
        %102 = OpIAdd %6 %101 %9
        %202 = OpLoad %6 %408
               OpStore %409 %202
               OpStore %407 %102
        %103 = OpUGreaterThanEqual %11 %101 %10
               OpLoopMerge %21 %22 None
               OpBranchConditional %103 %21 %104
        %104 = OpLabel
               OpBranchConditional %12 %23 %21
         %23 = OpLabel
        %105 = OpLoad %6 %407
        %106 = OpIAdd %6 %105 %9
               OpStore %407 %106
        %107 = OpUGreaterThanEqual %11 %105 %10
               OpLoopMerge %25 %26 None
               OpBranchConditional %107 %25 %108
        %108 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpBranchConditional %12 %26 %25
         %26 = OpLabel
               OpBranch %23
         %25 = OpLabel
        %109 = OpLoad %6 %407
        %110 = OpIAdd %6 %109 %9
               OpStore %407 %110
        %111 = OpUGreaterThanEqual %11 %109 %10
               OpLoopMerge %24 %27 None
               OpBranchConditional %111 %24 %112
        %112 = OpLabel
               OpBranchConditional %12 %24 %27
         %27 = OpLabel
               OpBranch %25
         %24 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpBranch %20
         %21 = OpLabel
               OpBranch %197
        %197 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, OutlineWithDeadBlocks1) {
  // This checks that if all blocks in the region being outlined were dead, all
  // blocks in the outlined function will be dead.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "foo(i1;"
               OpName %9 "x"
               OpName %12 "y"
               OpName %21 "i"
               OpName %46 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 2
         %14 = OpTypeBool
         %15 = OpConstantFalse %14
         %22 = OpConstant %6 0
         %29 = OpConstant %6 10
         %41 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %46 = OpVariable %7 Function
               OpStore %46 %13
         %47 = OpFunctionCall %2 %10 %46
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
         %21 = OpVariable %7 Function
               OpStore %12 %13
               OpSelectionMerge %17 None
               OpBranchConditional %15 %16 %17
         %16 = OpLabel
         %18 = OpLoad %6 %9
               OpStore %12 %18
         %19 = OpLoad %6 %9
         %20 = OpIAdd %6 %19 %13
               OpStore %9 %20
               OpStore %21 %22
               OpBranch %23
         %23 = OpLabel
               OpLoopMerge %25 %26 None
               OpBranch %27
         %27 = OpLabel
         %28 = OpLoad %6 %21
         %30 = OpSLessThan %14 %28 %29
               OpBranchConditional %30 %24 %25
         %24 = OpLabel
         %31 = OpLoad %6 %9
         %32 = OpLoad %6 %21
         %33 = OpSGreaterThan %14 %31 %32
               OpSelectionMerge %35 None
               OpBranchConditional %33 %34 %35
         %34 = OpLabel
               OpBranch %26
         %35 = OpLabel
         %37 = OpLoad %6 %9
         %38 = OpLoad %6 %12
         %39 = OpIAdd %6 %38 %37
               OpStore %12 %39
               OpBranch %26
         %26 = OpLabel
         %40 = OpLoad %6 %21
         %42 = OpIAdd %6 %40 %41
               OpStore %21 %42
               OpBranch %23
         %25 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %17
         %17 = OpLabel
         %43 = OpLoad %6 %9
         %44 = OpLoad %6 %12
         %45 = OpIAdd %6 %44 %43
               OpStore %12 %45
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  for (uint32_t block_id : {16u, 23u, 24u, 26u, 27u, 34u, 35u, 50u}) {
    transformation_context.GetFactManager()->AddFactBlockIsDead(block_id);
  }

  TransformationOutlineFunction transformation(
      /*entry_block*/ 16,
      /*exit_block*/ 50,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {{9, 206}, {12, 207}, {21, 208}},
      /*output_id_to_fresh_id*/ {});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  // All the original blocks, plus the new function entry block, should be dead.
  for (uint32_t block_id : {16u, 23u, 24u, 26u, 27u, 34u, 35u, 50u, 203u}) {
    ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(block_id));
  }
}

TEST(TransformationOutlineFunctionTest, OutlineWithDeadBlocks2) {
  // This checks that if some, but not all, blocks in the outlined region are
  // dead, those (but not others) will be dead in the outlined function.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %8
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypePointer Private %6
          %8 = OpVariable %7 Private
          %9 = OpConstantFalse %6
         %10 = OpTypePointer Function %6
         %12 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
               OpBranch %30
         %30 = OpLabel
               OpStore %8 %9
               OpBranch %31
         %31 = OpLabel
               OpStore %11 %12
               OpSelectionMerge %36 None
               OpBranchConditional %9 %32 %33
         %32 = OpLabel
               OpBranch %34
         %33 = OpLabel
               OpBranch %36
         %34 = OpLabel
               OpBranch %35
         %35 = OpLabel
               OpBranch %36
         %36 = OpLabel
               OpBranch %37
         %37 = OpLabel
         %13 = OpLoad %6 %8
               OpStore %11 %13
         %14 = OpLoad %6 %11
               OpStore %8 %14
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  for (uint32_t block_id : {32u, 34u, 35u}) {
    transformation_context.GetFactManager()->AddFactBlockIsDead(block_id);
  }

  TransformationOutlineFunction transformation(
      /*entry_block*/ 30,
      /*exit_block*/ 37,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {{11, 206}},
      /*output_id_to_fresh_id*/ {});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  // The blocks that were originally dead, but not others, should be dead.
  for (uint32_t block_id : {32u, 34u, 35u}) {
    ASSERT_TRUE(transformation_context.GetFactManager()->BlockIsDead(block_id));
  }
  for (uint32_t block_id : {5u, 30u, 31u, 33u, 36u, 37u, 203u}) {
    ASSERT_FALSE(
        transformation_context.GetFactManager()->BlockIsDead(block_id));
  }
}

TEST(TransformationOutlineFunctionTest,
     OutlineWithIrrelevantVariablesAndParameters) {
  // This checks that if the outlined region uses a mixture of irrelevant and
  // non-irrelevant variables and parameters, these properties are preserved
  // during outlining.
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
          %8 = OpTypeFunction %2 %7 %7
         %13 = OpConstant %6 2
         %15 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %10 = OpFunctionParameter %7
         %12 = OpLabel
         %14 = OpVariable %7 Function
         %20 = OpVariable %7 Function
               OpBranch %50
         %50 = OpLabel
               OpStore %9 %13
               OpStore %14 %15
         %16 = OpLoad %6 %14
               OpStore %10 %16
         %17 = OpLoad %6 %9
         %18 = OpLoad %6 %10
         %19 = OpIAdd %6 %17 %18
               OpStore %14 %19
         %21 = OpLoad %6 %9
               OpStore %20 %21
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(9);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      14);

  TransformationOutlineFunction transformation(
      /*entry_block*/ 50,
      /*exit_block*/ 50,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {{9, 206}, {10, 207}, {14, 208}, {20, 209}},
      /*output_id_to_fresh_id*/ {});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  // The variables that were originally irrelevant, plus input parameters
  // corresponding to them, should be irrelevant.  The rest should not be.
  for (uint32_t variable_id : {9u, 14u, 206u, 208u}) {
    ASSERT_TRUE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(
            variable_id));
  }
  for (uint32_t variable_id : {10u, 20u, 207u, 209u}) {
    ASSERT_FALSE(
        transformation_context.GetFactManager()->BlockIsDead(variable_id));
  }
}

TEST(TransformationOutlineFunctionTest,
     DoNotOutlineCodeThatProducesUsedPointer) {
  // This checks that we cannot outline a region of code if it produces a
  // pointer result id that gets used outside the region.  This avoids creating
  // a struct with a pointer member.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %6 "main"
               OpExecutionMode %6 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %21 = OpTypeBool
        %100 = OpTypeInt 32 0
         %99 = OpConstant %100 0
        %101 = OpTypeVector %100 2
        %102 = OpTypePointer Function %100
        %103 = OpTypePointer Function %101
          %6 = OpFunction %2 None %3
          %7 = OpLabel
        %104 = OpVariable %103 Function
               OpBranch %80
         %80 = OpLabel
        %105 = OpAccessChain %102 %104 %99
               OpBranch %106
        %106 = OpLabel
               OpStore %105 %99
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 80,
      /*exit_block*/ 80,
      /*new_function_struct_return_type_id*/ 300,
      /*new_function_type_id*/ 301,
      /*new_function_id*/ 302,
      /*new_function_region_entry_block*/ 304,
      /*new_caller_result_id*/ 305,
      /*new_callee_result_id*/ 306,
      /*input_id_to_fresh_id*/ {{104, 307}},
      /*output_id_to_fresh_id*/ {{105, 308}});

  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest, ExitBlockHeadsLoop) {
  // This checks that it is not possible outline a region that ends in a loop
  // head.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %15 = OpTypeInt 32 1
         %35 = OpTypeBool
         %39 = OpConstant %15 1
         %40 = OpConstantTrue %35
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpBranch %23
         %23 = OpLabel
         %24 = OpPhi %15 %39 %22 %39 %25
               OpLoopMerge %26 %25 None
               OpBranchConditional %40 %25 %26
         %25 = OpLabel
               OpBranch %23
         %26 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 22,
      /*exit_block*/ 23,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {},
      /*output_id_to_fresh_id*/ {});

  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
}

TEST(TransformationOutlineFunctionTest, Miscellaneous1) {
  // This tests outlining of some non-trivial code, and also tests the way
  // overflow ids are used by the transformation.

  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %85
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %28 "buf"
               OpMemberName %28 0 "u1"
               OpMemberName %28 1 "u2"
               OpName %30 ""
               OpName %85 "color"
               OpMemberDecorate %28 0 Offset 0
               OpMemberDecorate %28 1 Offset 4
               OpDecorate %28 Block
               OpDecorate %30 DescriptorSet 0
               OpDecorate %30 Binding 0
               OpDecorate %85 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
         %10 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpConstant %6 3
         %13 = OpConstant %6 4
         %14 = OpConstantComposite %7 %10 %11 %12 %13
         %15 = OpTypeInt 32 1
         %18 = OpConstant %15 0
         %28 = OpTypeStruct %6 %6
         %29 = OpTypePointer Uniform %28
         %30 = OpVariable %29 Uniform
         %31 = OpTypePointer Uniform %6
         %35 = OpTypeBool
         %39 = OpConstant %15 1
         %84 = OpTypePointer Output %7
         %85 = OpVariable %84 Output
        %114 = OpConstant %15 8
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %22
         %22 = OpLabel
        %103 = OpPhi %15 %18 %5 %106 %43
        %102 = OpPhi %7 %14 %5 %107 %43
        %101 = OpPhi %15 %18 %5 %40 %43
         %32 = OpAccessChain %31 %30 %18
         %33 = OpLoad %6 %32
         %34 = OpConvertFToS %15 %33
         %36 = OpSLessThan %35 %101 %34
               OpLoopMerge %24 %43 None
               OpBranchConditional %36 %23 %24
         %23 = OpLabel
         %40 = OpIAdd %15 %101 %39
               OpBranch %150
        %150 = OpLabel
               OpBranch %41
         %41 = OpLabel
        %107 = OpPhi %7 %102 %150 %111 %65
        %106 = OpPhi %15 %103 %150 %110 %65
        %104 = OpPhi %15 %40 %150 %81 %65
         %47 = OpAccessChain %31 %30 %39
         %48 = OpLoad %6 %47
         %49 = OpConvertFToS %15 %48
         %50 = OpSLessThan %35 %104 %49
               OpLoopMerge %1000 %65 None
               OpBranchConditional %50 %42 %1000
         %42 = OpLabel
         %60 = OpIAdd %15 %106 %114
         %63 = OpSGreaterThan %35 %104 %60
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %71 = OpCompositeExtract %6 %107 0
         %72 = OpFAdd %6 %71 %11
         %97 = OpCompositeInsert %7 %72 %107 0
         %76 = OpCompositeExtract %6 %107 3
         %77 = OpConvertFToS %15 %76
         %79 = OpIAdd %15 %60 %77
               OpBranch %65
         %65 = OpLabel
        %111 = OpPhi %7 %107 %42 %97 %64
        %110 = OpPhi %15 %60 %42 %79 %64
         %81 = OpIAdd %15 %104 %39
               OpBranch %41
       %1000 = OpLabel
               OpBranch %1001
       %1001 = OpLabel
               OpBranch %43
         %43 = OpLabel
               OpBranch %22
         %24 = OpLabel
         %87 = OpCompositeExtract %6 %102 0
         %91 = OpConvertSToF %6 %103
         %92 = OpCompositeConstruct %7 %87 %11 %91 %10
               OpStore %85 %92
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  TransformationOutlineFunction transformation(
      /*entry_block*/ 150,
      /*exit_block*/ 1001,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {{102, 300}, {103, 301}, {40, 302}},
      /*output_id_to_fresh_id*/ {{106, 400}, {107, 401}});

  TransformationOutlineFunction transformation_with_missing_input_id(
      /*entry_block*/ 150,
      /*exit_block*/ 1001,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {{102, 300}, {40, 302}},
      /*output_id_to_fresh_id*/ {{106, 400}, {107, 401}});

  TransformationOutlineFunction transformation_with_missing_output_id(
      /*entry_block*/ 150,
      /*exit_block*/ 1001,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {{102, 300}, {103, 301}, {40, 302}},
      /*output_id_to_fresh_id*/ {{106, 400}});

  TransformationOutlineFunction
      transformation_with_missing_input_and_output_ids(
          /*entry_block*/ 150,
          /*exit_block*/ 1001,
          /*new_function_struct_return_type_id*/ 200,
          /*new_function_type_id*/ 201,
          /*new_function_id*/ 202,
          /*new_function_region_entry_block*/ 203,
          /*new_caller_result_id*/ 204,
          /*new_callee_result_id*/ 205,
          /*input_id_to_fresh_id*/ {{102, 300}, {40, 302}},
          /*output_id_to_fresh_id*/ {{106, 400}});

  {
    const auto context =
        BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));

    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);

#ifndef NDEBUG
    // We expect the following applicability checks to lead to assertion
    // failures since the transformations are missing input or output ids, and
    // the transformation context does not have a source of overflow ids.
    ASSERT_DEATH(transformation_with_missing_input_id.IsApplicable(
                     context.get(), transformation_context),
                 "Bad attempt to query whether overflow ids are available.");
    ASSERT_DEATH(transformation_with_missing_output_id.IsApplicable(
                     context.get(), transformation_context),
                 "Bad attempt to query whether overflow ids are available.");
    ASSERT_DEATH(transformation_with_missing_input_and_output_ids.IsApplicable(
                     context.get(), transformation_context),
                 "Bad attempt to query whether overflow ids are available.");
#endif

    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));

    std::string variant_shader = R"(
                 OpCapability Shader
            %1 = OpExtInstImport "GLSL.std.450"
                 OpMemoryModel Logical GLSL450
                 OpEntryPoint Fragment %4 "main" %85
                 OpExecutionMode %4 OriginUpperLeft
                 OpSource ESSL 310
                 OpName %4 "main"
                 OpName %28 "buf"
                 OpMemberName %28 0 "u1"
                 OpMemberName %28 1 "u2"
                 OpName %30 ""
                 OpName %85 "color"
                 OpMemberDecorate %28 0 Offset 0
                 OpMemberDecorate %28 1 Offset 4
                 OpDecorate %28 Block
                 OpDecorate %30 DescriptorSet 0
                 OpDecorate %30 Binding 0
                 OpDecorate %85 Location 0
            %2 = OpTypeVoid
            %3 = OpTypeFunction %2
            %6 = OpTypeFloat 32
            %7 = OpTypeVector %6 4
           %10 = OpConstant %6 1
           %11 = OpConstant %6 2
           %12 = OpConstant %6 3
           %13 = OpConstant %6 4
           %14 = OpConstantComposite %7 %10 %11 %12 %13
           %15 = OpTypeInt 32 1
           %18 = OpConstant %15 0
           %28 = OpTypeStruct %6 %6
           %29 = OpTypePointer Uniform %28
           %30 = OpVariable %29 Uniform
           %31 = OpTypePointer Uniform %6
           %35 = OpTypeBool
           %39 = OpConstant %15 1
           %84 = OpTypePointer Output %7
           %85 = OpVariable %84 Output
          %114 = OpConstant %15 8
          %200 = OpTypeStruct %7 %15
          %201 = OpTypeFunction %200 %15 %7 %15
            %4 = OpFunction %2 None %3
            %5 = OpLabel
                 OpBranch %22
           %22 = OpLabel
          %103 = OpPhi %15 %18 %5 %106 %43
          %102 = OpPhi %7 %14 %5 %107 %43
          %101 = OpPhi %15 %18 %5 %40 %43
           %32 = OpAccessChain %31 %30 %18
           %33 = OpLoad %6 %32
           %34 = OpConvertFToS %15 %33
           %36 = OpSLessThan %35 %101 %34
                 OpLoopMerge %24 %43 None
                 OpBranchConditional %36 %23 %24
           %23 = OpLabel
           %40 = OpIAdd %15 %101 %39
                 OpBranch %150
          %150 = OpLabel
          %204 = OpFunctionCall %200 %202 %103 %102 %40
          %107 = OpCompositeExtract %7 %204 0
          %106 = OpCompositeExtract %15 %204 1
                 OpBranch %43
           %43 = OpLabel
                 OpBranch %22
           %24 = OpLabel
           %87 = OpCompositeExtract %6 %102 0
           %91 = OpConvertSToF %6 %103
           %92 = OpCompositeConstruct %7 %87 %11 %91 %10
                 OpStore %85 %92
                 OpReturn
                 OpFunctionEnd
          %202 = OpFunction %200 None %201
          %301 = OpFunctionParameter %15
          %300 = OpFunctionParameter %7
          %302 = OpFunctionParameter %15
          %203 = OpLabel
                 OpBranch %41
           %41 = OpLabel
          %401 = OpPhi %7 %300 %203 %111 %65
          %400 = OpPhi %15 %301 %203 %110 %65
          %104 = OpPhi %15 %302 %203 %81 %65
           %47 = OpAccessChain %31 %30 %39
           %48 = OpLoad %6 %47
           %49 = OpConvertFToS %15 %48
           %50 = OpSLessThan %35 %104 %49
                 OpLoopMerge %1000 %65 None
                 OpBranchConditional %50 %42 %1000
           %42 = OpLabel
           %60 = OpIAdd %15 %400 %114
           %63 = OpSGreaterThan %35 %104 %60
                 OpBranchConditional %63 %64 %65
           %64 = OpLabel
           %71 = OpCompositeExtract %6 %401 0
           %72 = OpFAdd %6 %71 %11
           %97 = OpCompositeInsert %7 %72 %401 0
           %76 = OpCompositeExtract %6 %401 3
           %77 = OpConvertFToS %15 %76
           %79 = OpIAdd %15 %60 %77
                 OpBranch %65
           %65 = OpLabel
          %111 = OpPhi %7 %401 %42 %97 %64
          %110 = OpPhi %15 %60 %42 %79 %64
           %81 = OpIAdd %15 %104 %39
                 OpBranch %41
         %1000 = OpLabel
                 OpBranch %1001
         %1001 = OpLabel
          %205 = OpCompositeConstruct %200 %401 %400
                 OpReturnValue %205
                 OpFunctionEnd
    )";
    ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
  }

  {
    const auto context =
        BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    auto overflow_ids_unique_ptr = MakeUnique<CounterOverflowIdSource>(2000);
    auto overflow_ids_ptr = overflow_ids_unique_ptr.get();
    TransformationContext new_transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options,
        std::move(overflow_ids_unique_ptr));
    ASSERT_TRUE(transformation_with_missing_input_id.IsApplicable(
        context.get(), new_transformation_context));
    ASSERT_TRUE(transformation_with_missing_output_id.IsApplicable(
        context.get(), new_transformation_context));
    ASSERT_TRUE(transformation_with_missing_input_and_output_ids.IsApplicable(
        context.get(), new_transformation_context));
    ApplyAndCheckFreshIds(transformation_with_missing_input_and_output_ids,
                          context.get(), &new_transformation_context,
                          overflow_ids_ptr->GetIssuedOverflowIds());
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));

    std::string variant_shader = R"(
                 OpCapability Shader
            %1 = OpExtInstImport "GLSL.std.450"
                 OpMemoryModel Logical GLSL450
                 OpEntryPoint Fragment %4 "main" %85
                 OpExecutionMode %4 OriginUpperLeft
                 OpSource ESSL 310
                 OpName %4 "main"
                 OpName %28 "buf"
                 OpMemberName %28 0 "u1"
                 OpMemberName %28 1 "u2"
                 OpName %30 ""
                 OpName %85 "color"
                 OpMemberDecorate %28 0 Offset 0
                 OpMemberDecorate %28 1 Offset 4
                 OpDecorate %28 Block
                 OpDecorate %30 DescriptorSet 0
                 OpDecorate %30 Binding 0
                 OpDecorate %85 Location 0
            %2 = OpTypeVoid
            %3 = OpTypeFunction %2
            %6 = OpTypeFloat 32
            %7 = OpTypeVector %6 4
           %10 = OpConstant %6 1
           %11 = OpConstant %6 2
           %12 = OpConstant %6 3
           %13 = OpConstant %6 4
           %14 = OpConstantComposite %7 %10 %11 %12 %13
           %15 = OpTypeInt 32 1
           %18 = OpConstant %15 0
           %28 = OpTypeStruct %6 %6
           %29 = OpTypePointer Uniform %28
           %30 = OpVariable %29 Uniform
           %31 = OpTypePointer Uniform %6
           %35 = OpTypeBool
           %39 = OpConstant %15 1
           %84 = OpTypePointer Output %7
           %85 = OpVariable %84 Output
          %114 = OpConstant %15 8
          %200 = OpTypeStruct %7 %15
          %201 = OpTypeFunction %200 %15 %7 %15
            %4 = OpFunction %2 None %3
            %5 = OpLabel
                 OpBranch %22
           %22 = OpLabel
          %103 = OpPhi %15 %18 %5 %106 %43
          %102 = OpPhi %7 %14 %5 %107 %43
          %101 = OpPhi %15 %18 %5 %40 %43
           %32 = OpAccessChain %31 %30 %18
           %33 = OpLoad %6 %32
           %34 = OpConvertFToS %15 %33
           %36 = OpSLessThan %35 %101 %34
                 OpLoopMerge %24 %43 None
                 OpBranchConditional %36 %23 %24
           %23 = OpLabel
           %40 = OpIAdd %15 %101 %39
                 OpBranch %150
          %150 = OpLabel
          %204 = OpFunctionCall %200 %202 %103 %102 %40
          %107 = OpCompositeExtract %7 %204 0
          %106 = OpCompositeExtract %15 %204 1
                 OpBranch %43
           %43 = OpLabel
                 OpBranch %22
           %24 = OpLabel
           %87 = OpCompositeExtract %6 %102 0
           %91 = OpConvertSToF %6 %103
           %92 = OpCompositeConstruct %7 %87 %11 %91 %10
                 OpStore %85 %92
                 OpReturn
                 OpFunctionEnd
          %202 = OpFunction %200 None %201
         %2000 = OpFunctionParameter %15
          %300 = OpFunctionParameter %7
          %302 = OpFunctionParameter %15
          %203 = OpLabel
                 OpBranch %41
           %41 = OpLabel
         %2001 = OpPhi %7 %300 %203 %111 %65
          %400 = OpPhi %15 %2000 %203 %110 %65
          %104 = OpPhi %15 %302 %203 %81 %65
           %47 = OpAccessChain %31 %30 %39
           %48 = OpLoad %6 %47
           %49 = OpConvertFToS %15 %48
           %50 = OpSLessThan %35 %104 %49
                 OpLoopMerge %1000 %65 None
                 OpBranchConditional %50 %42 %1000
           %42 = OpLabel
           %60 = OpIAdd %15 %400 %114
           %63 = OpSGreaterThan %35 %104 %60
                 OpBranchConditional %63 %64 %65
           %64 = OpLabel
           %71 = OpCompositeExtract %6 %2001 0
           %72 = OpFAdd %6 %71 %11
           %97 = OpCompositeInsert %7 %72 %2001 0
           %76 = OpCompositeExtract %6 %2001 3
           %77 = OpConvertFToS %15 %76
           %79 = OpIAdd %15 %60 %77
                 OpBranch %65
           %65 = OpLabel
          %111 = OpPhi %7 %2001 %42 %97 %64
          %110 = OpPhi %15 %60 %42 %79 %64
           %81 = OpIAdd %15 %104 %39
                 OpBranch %41
         %1000 = OpLabel
                 OpBranch %1001
         %1001 = OpLabel
          %205 = OpCompositeConstruct %200 %2001 %400
                 OpReturnValue %205
                 OpFunctionEnd
    )";
    ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
  }
}

TEST(TransformationOutlineFunctionTest, Miscellaneous2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %21 = OpTypeBool
        %167 = OpConstantTrue %21
        %168 = OpConstantFalse %21
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %34
         %34 = OpLabel
               OpLoopMerge %36 %37 None
               OpBranchConditional %168 %37 %38
         %38 = OpLabel
               OpBranchConditional %168 %37 %36
         %37 = OpLabel
               OpBranch %34
         %36 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 38,
      /*exit_block*/ 36,
      /*new_function_struct_return_type_id*/ 200,
      /*new_function_type_id*/ 201,
      /*new_function_id*/ 202,
      /*new_function_region_entry_block*/ 203,
      /*new_caller_result_id*/ 204,
      /*new_callee_result_id*/ 205,
      /*input_id_to_fresh_id*/ {},
      /*output_id_to_fresh_id*/ {});

  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationOutlineFunctionTest, Miscellaneous3) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %6 "main"
               OpExecutionMode %6 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %21 = OpTypeBool
        %167 = OpConstantTrue %21
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpBranch %80
         %80 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpLoopMerge %16 %17 None
               OpBranch %18
         %18 = OpLabel
               OpBranchConditional %167 %15 %16
         %15 = OpLabel
               OpBranch %17
         %16 = OpLabel
               OpBranch %81
         %81 = OpLabel
               OpReturn
         %17 = OpLabel
               OpBranch %14
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 80,
      /*exit_block*/ 81,
      /*new_function_struct_return_type_id*/ 300,
      /*new_function_type_id*/ 301,
      /*new_function_id*/ 302,
      /*new_function_region_entry_block*/ 304,
      /*new_caller_result_id*/ 305,
      /*new_callee_result_id*/ 306,
      /*input_id_to_fresh_id*/ {},
      /*output_id_to_fresh_id*/ {});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %6 "main"
               OpExecutionMode %6 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %21 = OpTypeBool
        %167 = OpConstantTrue %21
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpBranch %80
         %80 = OpLabel
        %305 = OpFunctionCall %2 %302
               OpReturn
               OpFunctionEnd
        %302 = OpFunction %2 None %3
        %304 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpLoopMerge %16 %17 None
               OpBranch %18
         %18 = OpLabel
               OpBranchConditional %167 %15 %16
         %15 = OpLabel
               OpBranch %17
         %16 = OpLabel
               OpBranch %81
         %81 = OpLabel
               OpReturn
         %17 = OpLabel
               OpBranch %14
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, Miscellaneous4) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %6 "main"
               OpExecutionMode %6 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %21 = OpTypeBool
        %100 = OpTypeInt 32 0
        %101 = OpTypePointer Function %100
        %102 = OpTypePointer Function %100
        %103 = OpTypeFunction %2 %101
          %6 = OpFunction %2 None %3
          %7 = OpLabel
        %104 = OpVariable %102 Function
               OpBranch %80
         %80 = OpLabel
        %105 = OpLoad %100 %104
               OpBranch %106
        %106 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  TransformationOutlineFunction transformation(
      /*entry_block*/ 80,
      /*exit_block*/ 106,
      /*new_function_struct_return_type_id*/ 300,
      /*new_function_type_id*/ 301,
      /*new_function_id*/ 302,
      /*new_function_region_entry_block*/ 304,
      /*new_caller_result_id*/ 305,
      /*new_callee_result_id*/ 306,
      /*input_id_to_fresh_id*/ {{104, 307}},
      /*output_id_to_fresh_id*/ {});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %6 "main"
               OpExecutionMode %6 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %21 = OpTypeBool
        %100 = OpTypeInt 32 0
        %101 = OpTypePointer Function %100
        %102 = OpTypePointer Function %100
        %103 = OpTypeFunction %2 %101
        %301 = OpTypeFunction %2 %102
          %6 = OpFunction %2 None %3
          %7 = OpLabel
        %104 = OpVariable %102 Function
               OpBranch %80
         %80 = OpLabel
        %305 = OpFunctionCall %2 %302 %104
               OpReturn
               OpFunctionEnd
        %302 = OpFunction %2 None %301
        %307 = OpFunctionParameter %102
        %304 = OpLabel
        %105 = OpLoad %100 %307
               OpBranch %106
        %106 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationOutlineFunctionTest, NoOutlineWithUnreachableBlocks) {
  // This checks that outlining will not be performed if a node in the region
  // has an unreachable predecessor.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %7 = OpLabel
               OpBranch %5
          %5 = OpLabel
               OpReturn
          %6 = OpLabel
               OpBranch %5
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
  TransformationOutlineFunction transformation(5, 5, /* not relevant */ 200,
                                               100, 101, 102, 103,
                                               /* not relevant */ 201, {}, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
