// Copyright (c) 2020 Stefano Milizia
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

#include "source/fuzz/transformation_record_synonymous_constants.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

// Apply the TransformationRecordSynonymousConstants defined by the given
// constant1_id and constant2_id and check that the fact that the two
// constants are synonym is recorded.
void ApplyTransformationAndCheckFactManager(
    uint32_t constant1_id, uint32_t constant2_id, opt::IRContext* ir_context,
    TransformationContext* transformation_context) {
  ApplyAndCheckFreshIds(
      TransformationRecordSynonymousConstants(constant1_id, constant2_id),
      ir_context, transformation_context);

  ASSERT_TRUE(transformation_context->GetFactManager()->IsSynonymous(
      MakeDataDescriptor(constant1_id, {}),
      MakeDataDescriptor(constant2_id, {})));
}

TEST(TransformationRecordSynonymousConstantsTest, IntConstants) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %17
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpName %12 "c"
               OpName %17 "color"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
               OpDecorate %17 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
         %19 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %18 = OpConstant %6 0
         %11 = OpConstantNull %6
         %13 = OpConstant %6 1
         %20 = OpConstant %19 1
         %21 = OpConstant %19 -1
         %22 = OpConstant %6 1
         %14 = OpTypeFloat 32
         %15 = OpTypeVector %14 4
         %16 = OpTypePointer Output %15
         %17 = OpVariable %16 Output
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %13
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %3 is not a constant declaration
  ASSERT_FALSE(TransformationRecordSynonymousConstants(3, 9).IsApplicable(
      context.get(), transformation_context));
  ASSERT_FALSE(TransformationRecordSynonymousConstants(9, 3).IsApplicable(
      context.get(), transformation_context));

  // The two constants must be different
  ASSERT_FALSE(TransformationRecordSynonymousConstants(9, 9).IsApplicable(
      context.get(), transformation_context));

  // %9 and %13 are not equivalent
  ASSERT_FALSE(TransformationRecordSynonymousConstants(9, 13).IsApplicable(
      context.get(), transformation_context));

  // Swapping the ids gives the same result
  ASSERT_FALSE(TransformationRecordSynonymousConstants(13, 9).IsApplicable(
      context.get(), transformation_context));

  // %11 and %13 are not equivalent
  ASSERT_FALSE(TransformationRecordSynonymousConstants(11, 13).IsApplicable(
      context.get(), transformation_context));

  // Swapping the ids gives the same result
  ASSERT_FALSE(TransformationRecordSynonymousConstants(13, 11).IsApplicable(
      context.get(), transformation_context));

  // %20 and %21 have different values
  ASSERT_FALSE(TransformationRecordSynonymousConstants(20, 21).IsApplicable(
      context.get(), transformation_context));

  // %13 and %22 are equal and thus equivalent (having the same value and type)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(13, 22).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(13, 22, context.get(),
                                         &transformation_context);

  // %13 and %20 are equal even if %13 is signed and %20 is unsigned
  ASSERT_TRUE(TransformationRecordSynonymousConstants(13, 20).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(13, 20, context.get(),
                                         &transformation_context);

  // %9 and %11 are equivalent (OpConstant with value 0 and OpConstantNull)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(9, 11).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(9, 11, context.get(),
                                         &transformation_context);

  // Swapping the ids gives the same result
  ASSERT_TRUE(TransformationRecordSynonymousConstants(11, 9).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(11, 9, context.get(),
                                         &transformation_context);
}

TEST(TransformationRecordSynonymousConstantsTest, BoolConstants) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %19
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b"
               OpName %19 "color"
               OpDecorate %19 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypePointer Function %6
          %9 = OpConstantFalse %6
         %20 = OpConstantNull %6
         %11 = OpConstantTrue %6
         %21 = OpConstantFalse %6
         %22 = OpConstantTrue %6
         %16 = OpTypeFloat 32
         %17 = OpTypeVector %16 4
         %18 = OpTypePointer Output %17
         %19 = OpVariable %18 Output
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
         %10 = OpLoad %6 %8
         %12 = OpLogicalEqual %6 %10 %11
               OpSelectionMerge %14 None
               OpBranchConditional %12 %13 %14
         %13 = OpLabel
               OpReturn
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %9 and %11 are not equivalent
  ASSERT_FALSE(TransformationRecordSynonymousConstants(9, 11).IsApplicable(
      context.get(), transformation_context));

  // %20 and %11 are not equivalent
  ASSERT_FALSE(TransformationRecordSynonymousConstants(20, 11).IsApplicable(
      context.get(), transformation_context));

  // %9 and %21 are equivalent (both OpConstantFalse)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(9, 21).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(9, 21, context.get(),
                                         &transformation_context);

  // %11 and %22 are equivalent (both OpConstantTrue)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(11, 22).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(11, 22, context.get(),
                                         &transformation_context);

  // %9 and %20 are equivalent (OpConstantFalse and boolean OpConstantNull)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(9, 20).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(9, 20, context.get(),
                                         &transformation_context);
}

TEST(TransformationRecordSynonymousConstantsTest, FloatConstants) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %22
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpName %12 "c"
               OpName %22 "color"
               OpDecorate %22 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %11 = OpConstantNull %6
         %13 = OpConstant %6 2
         %26 = OpConstant %6 2
         %16 = OpTypeBool
         %20 = OpTypeVector %6 4
         %21 = OpTypePointer Output %20
         %22 = OpVariable %21 Output
         %23 = OpConstantComposite %20 %9 %11 %9 %11
         %25 = OpConstantComposite %20 %11 %9 %9 %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %10 %11
               OpStore %12 %13
         %14 = OpLoad %6 %8
         %15 = OpLoad %6 %10
         %17 = OpFOrdEqual %16 %14 %15
               OpSelectionMerge %19 None
               OpBranchConditional %17 %18 %24
         %18 = OpLabel
               OpStore %22 %23
               OpBranch %19
         %24 = OpLabel
               OpStore %22 %25
               OpBranch %19
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %9 and %13 are not equivalent
  ASSERT_FALSE(TransformationRecordSynonymousConstants(9, 13).IsApplicable(
      context.get(), transformation_context));

  // %11 and %13 are not equivalent
  ASSERT_FALSE(TransformationRecordSynonymousConstants(11, 13).IsApplicable(
      context.get(), transformation_context));

  // %13 and %23 are not equivalent
  ASSERT_FALSE(TransformationRecordSynonymousConstants(13, 23).IsApplicable(
      context.get(), transformation_context));

  // %13 and %26 are identical float constants
  ASSERT_TRUE(TransformationRecordSynonymousConstants(13, 26).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(13, 26, context.get(),
                                         &transformation_context);

  // %9 and %11 are equivalent ()
  ASSERT_TRUE(TransformationRecordSynonymousConstants(9, 11).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(9, 11, context.get(),
                                         &transformation_context);
}

TEST(TransformationRecordSynonymousConstantsTest,
     VectorAndMatrixCompositeConstants) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %24
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %12 "d"
               OpName %16 "e"
               OpName %24 "color"
               OpDecorate %12 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %24 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %28 = OpConstant %6 0
         %30 = OpConstant %6 1
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 0
         %14 = OpTypeBool
         %15 = OpTypePointer Function %14
         %17 = OpConstantFalse %14
         %22 = OpTypeVector %6 4
         %37 = OpTypeVector %6 3
         %32 = OpTypeMatrix %22 2
         %39 = OpTypeMatrix %22 3
         %23 = OpTypePointer Output %22
         %24 = OpVariable %23 Output
         %25 = OpConstantComposite %22 %9 %9 %9 %9
         %27 = OpConstantNull %22
         %29 = OpConstantComposite %22 %9 %28 %28 %9
         %31 = OpConstantComposite %22 %30 %9 %9 %9
         %38 = OpConstantComposite %37 %9 %9 %9
         %33 = OpConstantComposite %32 %25 %29
         %34 = OpConstantComposite %32 %27 %25
         %35 = OpConstantNull %32
         %36 = OpConstantComposite %32 %31 %25
         %40 = OpConstantComposite %39 %25 %25 %25
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %16 = OpVariable %15 Function
               OpStore %8 %9
               OpStore %12 %13
               OpStore %16 %17
         %18 = OpLoad %10 %12
         %19 = OpIEqual %14 %18 %13
               OpSelectionMerge %21 None
               OpBranchConditional %19 %20 %26
         %20 = OpLabel
               OpStore %24 %25
               OpBranch %21
         %26 = OpLabel
               OpStore %24 %25
               OpBranch %21
         %21 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %25 and %27 are equivalent (25 is zero-like, 27 is null)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(25, 27).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(25, 27, context.get(),
                                         &transformation_context);

  // %25 and %29 are equivalent (same type and value)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(25, 29).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(25, 29, context.get(),
                                         &transformation_context);

  // %27 and %29 are equivalent (27 is null, 29 is zero-like)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(27, 29).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(27, 29, context.get(),
                                         &transformation_context);

  // %25 and %31 are not equivalent (they have different values)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(25, 31).IsApplicable(
      context.get(), transformation_context));

  // %27 and %31 are not equivalent (27 is null, 31 is not zero-like)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(27, 31).IsApplicable(
      context.get(), transformation_context));

  // %25 and %38 are not equivalent (they have different sizes)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(25, 38).IsApplicable(
      context.get(), transformation_context));

  // %35 and %36 are not equivalent (35 is null, 36 has non-zero components)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(35, 36).IsApplicable(
      context.get(), transformation_context));

  // %33 and %36 are not equivalent (not all components are equivalent)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(33, 36).IsApplicable(
      context.get(), transformation_context));

  // %33 and %40 are not equivalent (they have different sizes)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(33, 40).IsApplicable(
      context.get(), transformation_context));

  // %33 and %34 are equivalent (same type, equivalent components)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(33, 34).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(33, 34, context.get(),
                                         &transformation_context);

  // %33 and %35 are equivalent (33 has zero-valued components, 35 is null)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(33, 35).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(33, 35, context.get(),
                                         &transformation_context);
}

TEST(TransformationRecordSynonymousConstantsTest, StructCompositeConstants) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %24
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %12 "d"
               OpName %16 "e"
               OpName %24 "color"
               OpDecorate %12 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %24 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %28 = OpConstant %6 0
         %30 = OpConstant %6 1
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 0
         %33 = OpConstantNull %10
         %14 = OpTypeBool
         %15 = OpTypePointer Function %14
         %17 = OpConstantFalse %14
         %34 = OpConstantNull %14
         %22 = OpTypeVector %6 4
         %32 = OpTypeStruct %22 %10 %14 %6
         %38 = OpTypeStruct %6 %6 %6 %6
         %23 = OpTypePointer Output %22
         %24 = OpVariable %23 Output
         %25 = OpConstantComposite %22 %9 %9 %9 %9
         %27 = OpConstantNull %22
         %29 = OpConstantComposite %22 %9 %28 %28 %9
         %31 = OpConstantComposite %22 %30 %9 %9 %9
         %35 = OpConstantComposite %32 %25 %13 %17 %9
         %36 = OpConstantComposite %32 %27 %33 %34 %28
         %37 = OpConstantComposite %32 %31 %13 %17 %9
         %39 = OpConstantComposite %38 %9 %9 %9 %9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %16 = OpVariable %15 Function
               OpStore %8 %9
               OpStore %12 %13
               OpStore %16 %17
         %18 = OpLoad %10 %12
         %19 = OpIEqual %14 %18 %13
               OpSelectionMerge %21 None
               OpBranchConditional %19 %20 %26
         %20 = OpLabel
               OpStore %24 %25
               OpBranch %21
         %26 = OpLabel
               OpStore %24 %25
               OpBranch %21
         %21 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %29 and %35 are not equivalent (they have different types)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(29, 35).IsApplicable(
      context.get(), transformation_context));

  // %35 and %37 are not equivalent (their first components are not equivalent)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(35, 37).IsApplicable(
      context.get(), transformation_context));

  // %35 and %36 are equivalent (all their components are equivalent)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(35, 36).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(35, 36, context.get(),
                                         &transformation_context);

  // %25 and %39 are not equivalent (they have different types)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(25, 39).IsApplicable(
      context.get(), transformation_context));
}

TEST(TransformationRecordSynonymousConstantsTest, ArrayCompositeConstants) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %24
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %12 "d"
               OpName %16 "e"
               OpName %24 "color"
               OpDecorate %12 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %24 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %38 = OpConstant %6 1
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 0
         %27 = OpConstant %10 4
         %39 = OpConstant %10 2
         %14 = OpTypeBool
         %15 = OpTypePointer Function %14
         %17 = OpConstantFalse %14
         %22 = OpTypeVector %6 4
         %28 = OpTypeArray %6 %27
         %29 = OpTypeArray %28 %27
         %40 = OpTypeArray %6 %39
         %23 = OpTypePointer Output %22
         %24 = OpVariable %23 Output
         %25 = OpConstantComposite %22 %9 %9 %9 %9
         %31 = OpConstantComposite %28 %9 %9 %9 %9
         %41 = OpConstantComposite %40 %9 %9
         %32 = OpConstantComposite %28 %38 %9 %9 %9
         %33 = OpConstantNull %28
         %34 = OpConstantComposite %29 %31 %33 %31 %33
         %35 = OpConstantComposite %29 %33 %31 %33 %31
         %36 = OpConstantNull %29
         %37 = OpConstantComposite %29 %32 %33 %31 %33
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %16 = OpVariable %15 Function
               OpStore %8 %9
               OpStore %12 %13
               OpStore %16 %17
         %18 = OpLoad %10 %12
         %19 = OpIEqual %14 %18 %13
               OpSelectionMerge %21 None
               OpBranchConditional %19 %20 %26
         %20 = OpLabel
               OpStore %24 %25
               OpBranch %21
         %26 = OpLabel
               OpStore %24 %25
               OpBranch %21
         %21 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %25 and %31 are not equivalent (they have different types)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(25, 31).IsApplicable(
      context.get(), transformation_context));

  // %25 and %41 are not equivalent (they have different sizes)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(25, 41).IsApplicable(
      context.get(), transformation_context));

  // %31 and %32 are not equivalent (their components are not pairwise
  // equivalent)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(31, 32).IsApplicable(
      context.get(), transformation_context));

  // %31 and %33 are equivalent (%31 has zero-valued components, 32 is null)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(31, 33).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(31, 33, context.get(),
                                         &transformation_context);

  // %34 and %35 are equivalent (same type, equivalent components)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(34, 35).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(34, 35, context.get(),
                                         &transformation_context);

  // %35 and %36 are equivalent (%36 is null, %35 has zero-valued components)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(35, 36).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(35, 36, context.get(),
                                         &transformation_context);

  // %34 and %37 are not equivalent (they have non-equivalent components)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(34, 37).IsApplicable(
      context.get(), transformation_context));

  // %36 and %37 are not equivalent (36 is null, 37 does not have all-zero
  // components)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(36, 37).IsApplicable(
      context.get(), transformation_context));
}

TEST(TransformationRecordSynonymousConstantsTest, IntVectors) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main" %3
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %3 Location 0
          %4 = OpTypeVoid
          %5 = OpTypeFunction %4
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpTypeVector %6 4
          %9 = OpTypeVector %7 4
         %10 = OpTypePointer Function %8
         %11 = OpTypePointer Function %8
         %12 = OpConstant %6 0
         %13 = OpConstant %7 0
         %14 = OpConstant %6 1
         %25 = OpConstant %7 1
         %15 = OpConstantComposite %8 %12 %12 %12 %12
         %16 = OpConstantComposite %9 %13 %13 %13 %13
         %17 = OpConstantComposite %8 %14 %12 %12 %14
         %18 = OpConstantComposite %9 %25 %13 %13 %25
         %19 = OpConstantNull %8
         %20 = OpConstantNull %9
         %21 = OpTypeFloat 32
         %22 = OpTypeVector %21 4
         %23 = OpTypePointer Output %22
          %3 = OpVariable %23 Output
          %2 = OpFunction %4 None %5
         %24 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %15 and %17 are not equivalent (having non-equivalent components)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(15, 17).IsApplicable(
      context.get(), transformation_context));

  // %17 and %19 are not equivalent (%19 is null, %17 is non-zero)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(17, 19).IsApplicable(
      context.get(), transformation_context));

  // %17 and %20 are not equivalent (%19 is null, %20 is non-zero)
  ASSERT_FALSE(TransformationRecordSynonymousConstants(17, 20).IsApplicable(
      context.get(), transformation_context));

  // %15 and %16 are equivalent (having pairwise equivalent components)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(15, 16).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(15, 16, context.get(),
                                         &transformation_context);

  // %17 and %18 are equivalent (having pairwise equivalent components)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(17, 18).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(17, 18, context.get(),
                                         &transformation_context);

  // %19 and %20 are equivalent (both null vectors with compatible types)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(19, 20).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(19, 20, context.get(),
                                         &transformation_context);

  // %15 and %19 are equivalent (they have compatible types, %15 is zero-like
  // and %19 is null)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(15, 19).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(15, 19, context.get(),
                                         &transformation_context);

  // %15 and %20 are equivalent (they have compatible types, %15 is zero-like
  // and %20 is null)
  ASSERT_TRUE(TransformationRecordSynonymousConstants(15, 20).IsApplicable(
      context.get(), transformation_context));

  ApplyTransformationAndCheckFactManager(15, 20, context.get(),
                                         &transformation_context);
}

TEST(TransformationRecordSynonymousConstantsTest, FirstIrrelevantConstant) {
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
          %7 = OpConstant %6 23
          %8 = OpConstant %6 23
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
  ASSERT_TRUE(TransformationRecordSynonymousConstants(7, 8).IsApplicable(
      context.get(), transformation_context));

  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(7);
  ASSERT_FALSE(TransformationRecordSynonymousConstants(7, 8).IsApplicable(
      context.get(), transformation_context));
}

TEST(TransformationRecordSynonymousConstantsTest, SecondIrrelevantConstant) {
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
          %7 = OpConstant %6 23
          %8 = OpConstant %6 23
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
  ASSERT_TRUE(TransformationRecordSynonymousConstants(7, 8).IsApplicable(
      context.get(), transformation_context));

  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(8);
  ASSERT_FALSE(TransformationRecordSynonymousConstants(7, 8).IsApplicable(
      context.get(), transformation_context));
}

TEST(TransformationRecordSynonymousConstantsTest, InvalidIds) {
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
          %7 = OpConstant %6 23
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
  ASSERT_FALSE(TransformationRecordSynonymousConstants(7, 8).IsApplicable(
      context.get(), transformation_context));

  ASSERT_FALSE(TransformationRecordSynonymousConstants(8, 7).IsApplicable(
      context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
