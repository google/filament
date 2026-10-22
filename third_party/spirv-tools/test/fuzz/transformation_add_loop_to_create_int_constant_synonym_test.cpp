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

#include "source/fuzz/transformation_add_loop_to_create_int_constant_synonym.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddLoopToCreateIntConstantSynonymTest,
     ConstantsNotSuitable) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability Int64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
         %36 = OpTypeBool
          %5 = OpTypeInt 32 1
          %6 = OpConstant %5 -1
          %7 = OpConstant %5 0
          %8 = OpConstant %5 1
          %9 = OpConstant %5 2
         %10 = OpConstant %5 5
         %11 = OpConstant %5 10
         %12 = OpConstant %5 20
         %13 = OpConstant %5 33
         %14 = OpTypeVector %5 2
         %15 = OpConstantComposite %14 %10 %11
         %16 = OpConstantComposite %14 %12 %12
         %17 = OpTypeVector %5 3
         %18 = OpConstantComposite %17 %11 %7 %11
         %19 = OpTypeInt 64 1
         %20 = OpConstant %19 0
         %21 = OpConstant %19 10
         %22 = OpTypeVector %19 2
         %23 = OpConstantComposite %22 %21 %20
         %24 = OpTypeFloat 32
         %25 = OpConstant %24 0
         %26 = OpConstant %24 5
         %27 = OpConstant %24 10
         %28 = OpConstant %24 20
         %29 = OpTypeVector %24 3
         %30 = OpConstantComposite %29 %26 %27 %26
         %31 = OpConstantComposite %29 %28 %28 %28
         %32 = OpConstantComposite %29 %27 %25 %27
          %2 = OpFunction %3 None %4
         %33 = OpLabel
         %34 = OpCopyObject %5 %11
               OpBranch %35
         %35 = OpLabel
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
  // Reminder: the first four parameters of the constructor are the constants
  // with values for C, I, S, N respectively.

  // %70 does not correspond to an id in the module.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   70, 12, 10, 9, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // %35 is not a constant.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   35, 12, 10, 9, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // %27, %28 and %26 are not integer constants, but scalar floats.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   27, 28, 26, 9, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // %32, %31 and %30 are not integer constants, but vector floats.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   32, 31, 30, 9, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // %18=(10, 0, 10) has 3 components, while %16=(20, 20) and %15=(5, 10)
  // have 2.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   18, 16, 15, 9, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // %21 has bit width 64, while the width of %12 and %10 is 32.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   21, 12, 10, 9, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // %13 has component width 64, while the component width of %16 and %15 is 32.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   13, 16, 15, 9, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // %21 (N) is a 64-bit integer, not 32-bit.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   7, 7, 7, 21, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // %7 (N) has value 0, so N <= 0.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   7, 7, 7, 7, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // %6 (N) has value -1, so N <= 1.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   7, 7, 7, 6, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // %13 (N) has value 33, so N > 32.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   7, 7, 7, 6, 13, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // C(%11)=10, I(%12)=20, S(%10)=5, N(%8)=1, so C=I-S*N does not hold, as
  // 20-5*1=15.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   11, 12, 10, 8, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // C(%15)=(5, 10), I(%16)=(20, 20), S(%15)=(5, 10), N(%8)=1, so C=I-S*N does
  // not hold, as (20, 20)-1*(5, 10) = (15, 10).
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   15, 16, 15, 8, 35, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddLoopToCreateIntConstantSynonymTest,
     MissingConstantsOrBoolType) {
  {
    // The shader is missing the boolean type.
    std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
         %20 = OpConstant %5 0
          %6 = OpConstant %5 1
          %7 = OpConstant %5 2
          %8 = OpConstant %5 5
          %9 = OpConstant %5 10
         %10 = OpConstant %5 20
          %2 = OpFunction %3 None %4
         %11 = OpLabel
               OpBranch %12
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
)";

    const auto env = SPV_ENV_UNIVERSAL_1_5;
    const auto consumer = nullptr;
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);
    ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                     9, 10, 8, 7, 12, 100, 101, 102, 103, 104, 105, 106, 107)
                     .IsApplicable(context.get(), transformation_context));
  }
  {
    // The shader is missing a 32-bit integer 0 constant.
    std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
         %20 = OpTypeBool
          %5 = OpTypeInt 32 1
          %6 = OpConstant %5 1
          %7 = OpConstant %5 2
          %8 = OpConstant %5 5
          %9 = OpConstant %5 10
         %10 = OpConstant %5 20
          %2 = OpFunction %3 None %4
         %11 = OpLabel
               OpBranch %12
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
)";

    const auto env = SPV_ENV_UNIVERSAL_1_5;
    const auto consumer = nullptr;
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);
    ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                     9, 10, 8, 7, 12, 100, 101, 102, 103, 104, 105, 106, 107)
                     .IsApplicable(context.get(), transformation_context));
  }
  {
    // The shader is missing a 32-bit integer 1 constant.
    std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
         %20 = OpTypeBool
          %5 = OpTypeInt 32 1
          %6 = OpConstant %5 0
          %7 = OpConstant %5 2
          %8 = OpConstant %5 5
          %9 = OpConstant %5 10
         %10 = OpConstant %5 20
          %2 = OpFunction %3 None %4
         %11 = OpLabel
               OpBranch %12
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
)";

    const auto env = SPV_ENV_UNIVERSAL_1_5;
    const auto consumer = nullptr;
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);
    ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                     9, 10, 8, 7, 12, 100, 101, 102, 103, 104, 105, 106, 107)
                     .IsApplicable(context.get(), transformation_context));
  }
}

TEST(TransformationAddLoopToCreateIntConstantSynonymTest, Simple) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpConstant %7 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %7 5
         %12 = OpConstant %7 10
         %13 = OpConstant %7 20
          %2 = OpFunction %3 None %4
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
         %22 = OpPhi %7 %12 %14
               OpSelectionMerge %16 None
               OpBranchConditional %6 %17 %18
         %17 = OpLabel
         %23 = OpPhi %7 %13 %15
               OpBranch %18
         %18 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpBranch %19
         %19 = OpLabel
               OpLoopMerge %20 %19 None
               OpBranchConditional %6 %20 %19
         %20 = OpLabel
               OpBranch %21
         %21 = OpLabel
               OpBranch %24
         %24 = OpLabel
               OpLoopMerge %27 %25 None
               OpBranch %25
         %25 = OpLabel
               OpBranch %26
         %26 = OpLabel
               OpBranchConditional %6 %24 %27
         %27 = OpLabel
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
  // Block %14 has no predecessors.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 14, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // Block %18 has more than one predecessor.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 18, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // Block %16 is a merge block.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 16, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // Block %25 is a continue block.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 25, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // Block %19 has more than one predecessor.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 19, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // Block %20 is a merge block.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 20, 100, 101, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // Id %20 is supposed to be fresh, but it is not.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 15, 100, 20, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // Id %100 is used twice.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 15, 100, 100, 102, 103, 104, 105, 106, 107)
                   .IsApplicable(context.get(), transformation_context));

  // Id %100 is used twice.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 15, 100, 101, 102, 103, 104, 105, 106, 100)
                   .IsApplicable(context.get(), transformation_context));

  // Only the last id (for the additional block) is optional, so the other ones
  // cannot be 0.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 15, 0, 101, 102, 103, 104, 105, 106, 100)
                   .IsApplicable(context.get(), transformation_context));

  // This transformation will create a synonym of constant %12 from a 1-block
  // loop.
  auto transformation1 = TransformationAddLoopToCreateIntConstantSynonym(
      12, 13, 11, 10, 15, 100, 101, 102, 103, 104, 105, 106, 0);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(12, {}), MakeDataDescriptor(100, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // This transformation will create a synonym of constant %12 from a 2-block
  // loop.
  auto transformation2 = TransformationAddLoopToCreateIntConstantSynonym(
      12, 13, 11, 10, 17, 107, 108, 109, 110, 111, 112, 113, 114);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(12, {}), MakeDataDescriptor(107, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // This transformation will create a synonym of constant %12 from a 2-block
  // loop.
  auto transformation3 = TransformationAddLoopToCreateIntConstantSynonym(
      12, 13, 11, 10, 26, 115, 116, 117, 118, 119, 120, 121, 0);
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(12, {}), MakeDataDescriptor(115, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpConstant %7 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %7 5
         %12 = OpConstant %7 10
         %13 = OpConstant %7 20
          %2 = OpFunction %3 None %4
         %14 = OpLabel
               OpBranch %101
        %101 = OpLabel
        %102 = OpPhi %7 %8 %14 %105 %101
        %103 = OpPhi %7 %13 %14 %104 %101
        %104 = OpISub %7 %103 %11
        %105 = OpIAdd %7 %102 %9
        %106 = OpSLessThan %5 %105 %10
               OpLoopMerge %15 %101 None
               OpBranchConditional %106 %101 %15
         %15 = OpLabel
        %100 = OpPhi %7 %104 %101
         %22 = OpPhi %7 %12 %101
               OpSelectionMerge %16 None
               OpBranchConditional %6 %108 %18
        %108 = OpLabel
        %109 = OpPhi %7 %8 %15 %112 %114
        %110 = OpPhi %7 %13 %15 %111 %114
               OpLoopMerge %17 %114 None
               OpBranch %114
        %114 = OpLabel
        %111 = OpISub %7 %110 %11
        %112 = OpIAdd %7 %109 %9
        %113 = OpSLessThan %5 %112 %10
               OpBranchConditional %113 %108 %17
         %17 = OpLabel
        %107 = OpPhi %7 %111 %114
         %23 = OpPhi %7 %13 %114
               OpBranch %18
         %18 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpBranch %19
         %19 = OpLabel
               OpLoopMerge %20 %19 None
               OpBranchConditional %6 %20 %19
         %20 = OpLabel
               OpBranch %21
         %21 = OpLabel
               OpBranch %24
         %24 = OpLabel
               OpLoopMerge %27 %25 None
               OpBranch %25
         %25 = OpLabel
               OpBranch %116
        %116 = OpLabel
        %117 = OpPhi %7 %8 %25 %120 %116
        %118 = OpPhi %7 %13 %25 %119 %116
        %119 = OpISub %7 %118 %11
        %120 = OpIAdd %7 %117 %9
        %121 = OpSLessThan %5 %120 %10
               OpLoopMerge %26 %116 None
               OpBranchConditional %121 %116 %26
         %26 = OpLabel
        %115 = OpPhi %7 %119 %116
               OpBranchConditional %6 %24 %27
         %27 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationAddLoopToCreateIntConstantSynonymTest,
     DifferentSignednessAndVectors) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpConstant %7 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %7 5
         %12 = OpConstant %7 10
         %13 = OpConstant %7 20
         %14 = OpTypeInt 32 0
         %15 = OpConstant %14 0
         %16 = OpConstant %14 5
         %17 = OpConstant %14 10
         %18 = OpConstant %14 20
         %19 = OpTypeVector %7 2
         %20 = OpTypeVector %14 2
         %21 = OpConstantComposite %19 %12 %8
         %22 = OpConstantComposite %20 %17 %15
         %23 = OpConstantComposite %19 %13 %12
         %24 = OpConstantComposite %19 %11 %11
          %2 = OpFunction %3 None %4
         %25 = OpLabel
               OpBranch %26
         %26 = OpLabel
               OpBranch %27
         %27 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpBranch %29
         %29 = OpLabel
               OpBranch %30
         %30 = OpLabel
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
  // These tests check that the transformation is applicable and is applied
  // correctly with integers, scalar and vectors, of different signedness.

  // %12 is a signed integer, %18 and %16 are unsigned integers.
  auto transformation1 = TransformationAddLoopToCreateIntConstantSynonym(
      12, 18, 16, 10, 26, 100, 101, 102, 103, 104, 105, 106, 0);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(12, {}), MakeDataDescriptor(100, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %12 and %11 are signed integers, %18 is an unsigned integer.
  auto transformation2 = TransformationAddLoopToCreateIntConstantSynonym(
      12, 18, 11, 10, 27, 108, 109, 110, 111, 112, 113, 114, 0);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(12, {}), MakeDataDescriptor(108, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %17, %18 and %16 are all signed integers.
  auto transformation3 = TransformationAddLoopToCreateIntConstantSynonym(
      17, 18, 16, 10, 28, 115, 116, 117, 118, 119, 120, 121, 0);
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(17, {}), MakeDataDescriptor(115, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %22 is an unsigned integer vector, %23 and %24 are signed integer vectors.
  auto transformation4 = TransformationAddLoopToCreateIntConstantSynonym(
      22, 23, 24, 10, 29, 122, 123, 124, 125, 126, 127, 128, 0);
  ASSERT_TRUE(
      transformation4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation4, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(22, {}), MakeDataDescriptor(122, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %21, %23 and %24 are all signed integer vectors.
  auto transformation5 = TransformationAddLoopToCreateIntConstantSynonym(
      21, 23, 24, 10, 30, 129, 130, 131, 132, 133, 134, 135, 0);
  ASSERT_TRUE(
      transformation5.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation5, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(21, {}), MakeDataDescriptor(129, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpConstant %7 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %7 5
         %12 = OpConstant %7 10
         %13 = OpConstant %7 20
         %14 = OpTypeInt 32 0
         %15 = OpConstant %14 0
         %16 = OpConstant %14 5
         %17 = OpConstant %14 10
         %18 = OpConstant %14 20
         %19 = OpTypeVector %7 2
         %20 = OpTypeVector %14 2
         %21 = OpConstantComposite %19 %12 %8
         %22 = OpConstantComposite %20 %17 %15
         %23 = OpConstantComposite %19 %13 %12
         %24 = OpConstantComposite %19 %11 %11
          %2 = OpFunction %3 None %4
         %25 = OpLabel
               OpBranch %101
        %101 = OpLabel
        %102 = OpPhi %7 %8 %25 %105 %101
        %103 = OpPhi %14 %18 %25 %104 %101
        %104 = OpISub %14 %103 %16
        %105 = OpIAdd %7 %102 %9
        %106 = OpSLessThan %5 %105 %10
               OpLoopMerge %26 %101 None
               OpBranchConditional %106 %101 %26
         %26 = OpLabel
        %100 = OpPhi %14 %104 %101
               OpBranch %109
        %109 = OpLabel
        %110 = OpPhi %7 %8 %26 %113 %109
        %111 = OpPhi %14 %18 %26 %112 %109
        %112 = OpISub %14 %111 %11
        %113 = OpIAdd %7 %110 %9
        %114 = OpSLessThan %5 %113 %10
               OpLoopMerge %27 %109 None
               OpBranchConditional %114 %109 %27
         %27 = OpLabel
        %108 = OpPhi %14 %112 %109
               OpBranch %116
        %116 = OpLabel
        %117 = OpPhi %7 %8 %27 %120 %116
        %118 = OpPhi %14 %18 %27 %119 %116
        %119 = OpISub %14 %118 %16
        %120 = OpIAdd %7 %117 %9
        %121 = OpSLessThan %5 %120 %10
               OpLoopMerge %28 %116 None
               OpBranchConditional %121 %116 %28
         %28 = OpLabel
        %115 = OpPhi %14 %119 %116
               OpBranch %123
        %123 = OpLabel
        %124 = OpPhi %7 %8 %28 %127 %123
        %125 = OpPhi %19 %23 %28 %126 %123
        %126 = OpISub %19 %125 %24
        %127 = OpIAdd %7 %124 %9
        %128 = OpSLessThan %5 %127 %10
               OpLoopMerge %29 %123 None
               OpBranchConditional %128 %123 %29
         %29 = OpLabel
        %122 = OpPhi %19 %126 %123
               OpBranch %130
        %130 = OpLabel
        %131 = OpPhi %7 %8 %29 %134 %130
        %132 = OpPhi %19 %23 %29 %133 %130
        %133 = OpISub %19 %132 %24
        %134 = OpIAdd %7 %131 %9
        %135 = OpSLessThan %5 %134 %10
               OpLoopMerge %30 %130 None
               OpBranchConditional %135 %130 %30
         %30 = OpLabel
        %129 = OpPhi %19 %133 %130
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationAddLoopToCreateIntConstantSynonymTest, 64BitConstants) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability Int64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpConstant %7 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpTypeInt 64 1
         %12 = OpConstant %11 5
         %13 = OpConstant %11 10
         %14 = OpConstant %11 20
         %15 = OpTypeVector %11 2
         %16 = OpConstantComposite %15 %13 %13
         %17 = OpConstantComposite %15 %14 %14
         %18 = OpConstantComposite %15 %12 %12
          %2 = OpFunction %3 None %4
         %19 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpBranch %21
         %21 = OpLabel
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
  // These tests check that the transformation can be applied, and is applied
  // correctly, to 64-bit integer (scalar and vector) constants.

  // 64-bit scalar integers.
  auto transformation1 = TransformationAddLoopToCreateIntConstantSynonym(
      13, 14, 12, 10, 20, 100, 101, 102, 103, 104, 105, 106, 0);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(13, {}), MakeDataDescriptor(100, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // 64-bit vector integers.
  auto transformation2 = TransformationAddLoopToCreateIntConstantSynonym(
      16, 17, 18, 10, 21, 107, 108, 109, 110, 111, 112, 113, 0);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(16, {}), MakeDataDescriptor(107, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
               OpCapability Int64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpConstant %7 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpTypeInt 64 1
         %12 = OpConstant %11 5
         %13 = OpConstant %11 10
         %14 = OpConstant %11 20
         %15 = OpTypeVector %11 2
         %16 = OpConstantComposite %15 %13 %13
         %17 = OpConstantComposite %15 %14 %14
         %18 = OpConstantComposite %15 %12 %12
          %2 = OpFunction %3 None %4
         %19 = OpLabel
               OpBranch %101
        %101 = OpLabel
        %102 = OpPhi %7 %8 %19 %105 %101
        %103 = OpPhi %11 %14 %19 %104 %101
        %104 = OpISub %11 %103 %12
        %105 = OpIAdd %7 %102 %9
        %106 = OpSLessThan %5 %105 %10
               OpLoopMerge %20 %101 None
               OpBranchConditional %106 %101 %20
         %20 = OpLabel
        %100 = OpPhi %11 %104 %101
               OpBranch %108
        %108 = OpLabel
        %109 = OpPhi %7 %8 %20 %112 %108
        %110 = OpPhi %15 %17 %20 %111 %108
        %111 = OpISub %15 %110 %18
        %112 = OpIAdd %7 %109 %9
        %113 = OpSLessThan %5 %112 %10
               OpLoopMerge %21 %108 None
               OpBranchConditional %113 %108 %21
         %21 = OpLabel
        %107 = OpPhi %15 %111 %108
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationAddLoopToCreateIntConstantSynonymTest, Underflow) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpConstant %7 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %7 20
         %12 = OpConstant %7 -4
         %13 = OpTypeInt 32 0
         %14 = OpConstant %13 214748365
         %15 = OpConstant %13 4294967256
          %2 = OpFunction %3 None %4
         %16 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpBranch %18
         %18 = OpLabel
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
  // These tests check that underflows are taken into consideration when
  // deciding if  transformation is applicable.

  // Subtracting 2147483648 20 times from 32-bit integer 0 underflows 2 times
  // and the result is equivalent to -4.
  auto transformation1 = TransformationAddLoopToCreateIntConstantSynonym(
      12, 8, 14, 11, 17, 100, 101, 102, 103, 104, 105, 106, 0);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(12, {}), MakeDataDescriptor(100, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Subtracting 20 twice from 0 underflows and gives the unsigned integer
  // 4294967256.
  auto transformation2 = TransformationAddLoopToCreateIntConstantSynonym(
      15, 8, 11, 10, 18, 107, 108, 109, 110, 111, 112, 113, 0);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(15, {}), MakeDataDescriptor(107, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpConstant %7 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %7 20
         %12 = OpConstant %7 -4
         %13 = OpTypeInt 32 0
         %14 = OpConstant %13 214748365
         %15 = OpConstant %13 4294967256
          %2 = OpFunction %3 None %4
         %16 = OpLabel
               OpBranch %101
        %101 = OpLabel
        %102 = OpPhi %7 %8 %16 %105 %101
        %103 = OpPhi %7 %8 %16 %104 %101
        %104 = OpISub %7 %103 %14
        %105 = OpIAdd %7 %102 %9
        %106 = OpSLessThan %5 %105 %11
               OpLoopMerge %17 %101 None
               OpBranchConditional %106 %101 %17
         %17 = OpLabel
        %100 = OpPhi %7 %104 %101
               OpBranch %108
        %108 = OpLabel
        %109 = OpPhi %7 %8 %17 %112 %108
        %110 = OpPhi %7 %8 %17 %111 %108
        %111 = OpISub %7 %110 %11
        %112 = OpIAdd %7 %109 %9
        %113 = OpSLessThan %5 %112 %10
               OpLoopMerge %18 %108 None
               OpBranchConditional %113 %108 %18
         %18 = OpLabel
        %107 = OpPhi %7 %111 %108
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationAddLoopToCreateIntConstantSynonymTest,
     InapplicableDueToDeadBlockOrIrrelevantId) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpConstant %7 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %7 5
         %12 = OpConstant %7 10
         %13 = OpConstant %7 20
       %1010 = OpConstant %7 2
       %1011 = OpConstant %7 5
       %1012 = OpConstant %7 10
       %1013 = OpConstant %7 20
          %2 = OpFunction %3 None %4
         %14 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %6 %16 %15
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpBranch %17
         %17 = OpLabel
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
  transformation_context.GetFactManager()->AddFactBlockIsDead(15);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(1010);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(1011);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(1012);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(1013);
  // Bad because the block before which the loop would be inserted is dead.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 10, 15, 100, 101, 102, 103, 104, 105, 106, 0)
                   .IsApplicable(context.get(), transformation_context));
  // OK
  ASSERT_TRUE(TransformationAddLoopToCreateIntConstantSynonym(
                  12, 13, 11, 10, 17, 100, 101, 102, 103, 104, 105, 106, 0)
                  .IsApplicable(context.get(), transformation_context));
  // Bad because in each case one of the constants involved is irrelevant.
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   1012, 13, 11, 10, 17, 100, 101, 102, 103, 104, 105, 106, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 1013, 11, 10, 17, 100, 101, 102, 103, 104, 105, 106, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 1011, 10, 17, 100, 101, 102, 103, 104, 105, 106, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationAddLoopToCreateIntConstantSynonym(
                   12, 13, 11, 1010, 17, 100, 101, 102, 103, 104, 105, 106, 0)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddLoopToCreateIntConstantSynonymTest, InserBeforeOpSwitch) {
  // Checks that it is acceptable for a loop to be added before a target of an
  // OpSwitch instruction.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 0
         %20 = OpConstant %6 1
         %21 = OpConstant %6 2
         %22 = OpTypeBool
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %10 None
               OpSwitch %7 %9 0 %8
          %9 = OpLabel
               OpBranch %10
          %8 = OpLabel
               OpBranch %10
         %10 = OpLabel
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

  auto transformation1 = TransformationAddLoopToCreateIntConstantSynonym(
      20, 21, 20, 20, 9, 100, 101, 102, 103, 104, 105, 106, 0);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(20, {}), MakeDataDescriptor(100, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation2 = TransformationAddLoopToCreateIntConstantSynonym(
      20, 21, 20, 20, 8, 200, 201, 202, 203, 204, 205, 206, 0);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(20, {}), MakeDataDescriptor(200, {})));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 0
         %20 = OpConstant %6 1
         %21 = OpConstant %6 2
         %22 = OpTypeBool
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %10 None
               OpSwitch %7 %101 0 %201
        %101 = OpLabel
        %102 = OpPhi %6 %7 %5 %105 %101
        %103 = OpPhi %6 %21 %5 %104 %101
        %104 = OpISub %6 %103 %20
        %105 = OpIAdd %6 %102 %20
        %106 = OpSLessThan %22 %105 %20
               OpLoopMerge %9 %101 None
               OpBranchConditional %106 %101 %9
          %9 = OpLabel
        %100 = OpPhi %6 %104 %101
               OpBranch %10
        %201 = OpLabel
        %202 = OpPhi %6 %7 %5 %205 %201
        %203 = OpPhi %6 %21 %5 %204 %201
        %204 = OpISub %6 %203 %20
        %205 = OpIAdd %6 %202 %20
        %206 = OpSLessThan %22 %205 %20
               OpLoopMerge %8 %201 None
               OpBranchConditional %206 %201 %8
          %8 = OpLabel
        %200 = OpPhi %6 %204 %201
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
