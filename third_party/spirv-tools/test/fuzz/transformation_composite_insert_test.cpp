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

#include "source/fuzz/transformation_composite_insert.h"

#include "gtest/gtest.h"
#include "source/fuzz/data_descriptor.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationCompositeInsertTest, NotApplicableScenarios) {
  // This test handles cases where IsApplicable() returns false.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i1"
               OpName %10 "i2"
               OpName %12 "base"
               OpMemberName %12 0 "a1"
               OpMemberName %12 1 "a2"
               OpName %14 "b"
               OpName %18 "level_1"
               OpMemberName %18 0 "b1"
               OpMemberName %18 1 "b2"
               OpName %20 "l1"
               OpName %24 "level_2"
               OpMemberName %24 0 "c1"
               OpMemberName %24 1 "c2"
               OpName %26 "l2"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpTypeStruct %6 %6
         %13 = OpTypePointer Function %12
         %18 = OpTypeStruct %12 %12
         %19 = OpTypePointer Function %18
         %24 = OpTypeStruct %18 %18
         %25 = OpTypePointer Function %24
         %30 = OpTypeBool
         %31 = OpConstantTrue %30
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %26 = OpVariable %25 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
         %21 = OpLoad %12 %14
         %22 = OpLoad %12 %14
         %23 = OpCompositeConstruct %18 %21 %22
               OpStore %20 %23
         %27 = OpLoad %18 %20
         %28 = OpLoad %18 %20
         %29 = OpCompositeConstruct %24 %27 %28
               OpStore %26 %29
               OpSelectionMerge %33 None
               OpBranchConditional %31 %32 %33
         %32 = OpLabel
               OpBranch %33
         %33 = OpLabel
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
  // Bad: |fresh_id| is not fresh.
  auto transformation_bad_1 = TransformationCompositeInsert(
      MakeInstructionDescriptor(29, SpvOpStore, 0), 20, 29, 11, {1, 0, 0});
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Bad: |composite_id| does not refer to a existing instruction.
  auto transformation_bad_2 = TransformationCompositeInsert(
      MakeInstructionDescriptor(29, SpvOpStore, 0), 50, 40, 11, {1, 0, 0});
  ASSERT_FALSE(
      transformation_bad_2.IsApplicable(context.get(), transformation_context));

  // Bad: |composite_id| does not refer to a composite value.
  auto transformation_bad_3 = TransformationCompositeInsert(
      MakeInstructionDescriptor(29, SpvOpStore, 0), 50, 9, 11, {1, 0, 0});
  ASSERT_FALSE(
      transformation_bad_3.IsApplicable(context.get(), transformation_context));

  // Bad: |object_id| does not refer to a defined instruction.
  auto transformation_bad_4 = TransformationCompositeInsert(
      MakeInstructionDescriptor(29, SpvOpStore, 0), 50, 29, 40, {1, 0, 0});
  ASSERT_FALSE(
      transformation_bad_4.IsApplicable(context.get(), transformation_context));

  // Bad: |object_id| cannot refer to a pointer.
  auto transformation_bad_5 = TransformationCompositeInsert(
      MakeInstructionDescriptor(29, SpvOpStore, 0), 50, 29, 8, {1, 0, 0});
  ASSERT_FALSE(
      transformation_bad_5.IsApplicable(context.get(), transformation_context));

  // Bad: |index| is not a correct index.
  auto transformation_bad_6 = TransformationCompositeInsert(
      MakeInstructionDescriptor(29, SpvOpStore, 0), 50, 29, 11, {2, 0, 0});
  ASSERT_FALSE(
      transformation_bad_6.IsApplicable(context.get(), transformation_context));

  // Bad: Type id of the object to be inserted and the type id of the
  // component at |index| are not the same.
  auto transformation_bad_7 = TransformationCompositeInsert(
      MakeInstructionDescriptor(29, SpvOpStore, 0), 50, 29, 11, {1, 0});
  ASSERT_FALSE(
      transformation_bad_7.IsApplicable(context.get(), transformation_context));

  // Bad: |instruction_to_insert_before| does not refer to a defined
  // instruction.
  auto transformation_bad_8 = TransformationCompositeInsert(
      MakeInstructionDescriptor(29, SpvOpIMul, 0), 50, 29, 11, {1, 0, 0});
  ASSERT_FALSE(
      transformation_bad_8.IsApplicable(context.get(), transformation_context));

  // Bad: OpCompositeInsert cannot be inserted before OpBranchConditional with
  // OpSelectionMerge above it.
  auto transformation_bad_9 = TransformationCompositeInsert(
      MakeInstructionDescriptor(29, SpvOpBranchConditional, 0), 50, 29, 11,
      {1, 0, 0});
  ASSERT_FALSE(
      transformation_bad_9.IsApplicable(context.get(), transformation_context));

  // Bad: |composite_id| does not have a type_id.
  auto transformation_bad_10 = TransformationCompositeInsert(
      MakeInstructionDescriptor(29, SpvOpStore, 0), 50, 1, 11, {1, 0, 0});
  ASSERT_FALSE(transformation_bad_10.IsApplicable(context.get(),
                                                  transformation_context));
}

TEST(TransformationCompositeInsertTest, EmptyCompositeScenarios) {
  // This test handles cases where either the composite is empty or the
  // composite contains an empty composite.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i1"
               OpName %10 "i2"
               OpName %12 "base"
               OpMemberName %12 0 "a1"
               OpMemberName %12 1 "a2"
               OpName %14 "b"
          %2 = OpTypeVoid
         %60 = OpTypeStruct
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %61 = OpConstantComposite %60
         %62 = OpConstantComposite %60
         %12 = OpTypeStruct %6 %6
         %63 = OpTypeStruct %6 %60
         %13 = OpTypePointer Function %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
         %64 = OpCompositeConstruct %63 %15 %61
               OpStore %14 %17
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
  // Bad: The composite with |composite_id| cannot be empty.
  auto transformation_bad_1 = TransformationCompositeInsert(
      MakeInstructionDescriptor(64, SpvOpStore, 0), 50, 61, 62, {1});
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Good: It is possible to insert into a composite an element which is an
  // empty composite.
  auto transformation_good_1 = TransformationCompositeInsert(
      MakeInstructionDescriptor(64, SpvOpStore, 0), 50, 64, 62, {1});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i1"
               OpName %10 "i2"
               OpName %12 "base"
               OpMemberName %12 0 "a1"
               OpMemberName %12 1 "a2"
               OpName %14 "b"
          %2 = OpTypeVoid
         %60 = OpTypeStruct
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %61 = OpConstantComposite %60
         %62 = OpConstantComposite %60
         %12 = OpTypeStruct %6 %6
         %63 = OpTypeStruct %6 %60
         %13 = OpTypePointer Function %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
         %64 = OpCompositeConstruct %63 %15 %61
         %50 = OpCompositeInsert %63 %62 %64 1
               OpStore %14 %17
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationCompositeInsertTest, IrrelevantCompositeNoSynonyms) {
  // This test handles cases where either |composite| is irrelevant.
  // The transformation shouldn't create any synonyms.
  // The member composite has a different number of elements than the parent
  // composite.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i1"
               OpName %10 "i2"
               OpName %12 "base"
               OpMemberName %12 0 "a1"
               OpMemberName %12 1 "a2"
               OpName %14 "b"
               OpName %18 "level_1"
               OpMemberName %18 0 "b1"
               OpMemberName %18 1 "b2"
               OpMemberName %18 2 "b3"
               OpName %20 "l1"
               OpName %25 "level_2"
               OpMemberName %25 0 "c1"
               OpMemberName %25 1 "c2"
               OpName %27 "l2"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpTypeStruct %6 %6
         %13 = OpTypePointer Function %12
         %18 = OpTypeStruct %12 %12 %12
         %19 = OpTypePointer Function %18
         %25 = OpTypeStruct %18 %18
         %26 = OpTypePointer Function %25
         %31 = OpTypeBool
         %32 = OpConstantTrue %31
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %27 = OpVariable %26 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
         %21 = OpLoad %12 %14
         %22 = OpLoad %12 %14
         %23 = OpLoad %12 %14
         %24 = OpCompositeConstruct %18 %21 %22 %23
               OpStore %20 %24
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %20
         %30 = OpCompositeConstruct %25 %28 %29
               OpStore %27 %30
               OpSelectionMerge %34 None
               OpBranchConditional %32 %33 %34
         %33 = OpLabel
               OpBranch %34
         %34 = OpLabel
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
  // Add fact that the composite is irrelevant.
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(30);

  auto transformation_good_1 = TransformationCompositeInsert(
      MakeInstructionDescriptor(30, SpvOpStore, 0), 50, 30, 11, {1, 0, 0});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // No synonyms that involve the original object - %30 - should have been
  // added.
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {0}), MakeDataDescriptor(50, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 1}), MakeDataDescriptor(50, {1, 1})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 2}), MakeDataDescriptor(50, {1, 2})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 0, 1}), MakeDataDescriptor(50, {1, 0, 1})));
  // We *should* have a synonym between %11 and the component of %50 into which
  // it has been inserted.
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {1, 0, 0}), MakeDataDescriptor(11, {})));
}

TEST(TransformationCompositeInsertTest, IrrelevantObjectNoSynonyms) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i1"
               OpName %10 "i2"
               OpName %12 "base"
               OpMemberName %12 0 "a1"
               OpMemberName %12 1 "a2"
               OpName %14 "b"
               OpName %18 "level_1"
               OpMemberName %18 0 "b1"
               OpMemberName %18 1 "b2"
               OpMemberName %18 2 "b3"
               OpName %20 "l1"
               OpName %25 "level_2"
               OpMemberName %25 0 "c1"
               OpMemberName %25 1 "c2"
               OpName %27 "l2"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpTypeStruct %6 %6
         %13 = OpTypePointer Function %12
         %18 = OpTypeStruct %12 %12 %12
         %19 = OpTypePointer Function %18
         %25 = OpTypeStruct %18 %18
         %26 = OpTypePointer Function %25
         %31 = OpTypeBool
         %32 = OpConstantTrue %31
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %27 = OpVariable %26 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
         %21 = OpLoad %12 %14
         %22 = OpLoad %12 %14
         %23 = OpLoad %12 %14
         %24 = OpCompositeConstruct %18 %21 %22 %23
               OpStore %20 %24
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %20
         %30 = OpCompositeConstruct %25 %28 %29
               OpStore %27 %30
               OpSelectionMerge %34 None
               OpBranchConditional %32 %33 %34
         %33 = OpLabel
               OpBranch %34
         %34 = OpLabel
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
  // Add fact that the object is irrelevant.
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(11);

  auto transformation_good_1 = TransformationCompositeInsert(
      MakeInstructionDescriptor(30, SpvOpStore, 0), 50, 30, 11, {1, 0, 0});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Since %30 and %50 are not irrelevant, they should be synonymous at all
  // indices unaffected by the insertion.
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {0}), MakeDataDescriptor(50, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 1}), MakeDataDescriptor(50, {1, 1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 2}), MakeDataDescriptor(50, {1, 2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 0, 1}), MakeDataDescriptor(50, {1, 0, 1})));
  // Since %11 is irrelevant it should not be synonymous with the component into
  // which it has been inserted.
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {1, 0, 0}), MakeDataDescriptor(11, {})));
}

TEST(TransformationCompositeInsertTest, ApplicableCreatedSynonyms) {
  // This test handles cases where neither |composite| nor |object| is
  // irrelevant. The transformation should create synonyms.
  // The member composite has a different number of elements than the parent
  // composite.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i1"
               OpName %10 "i2"
               OpName %12 "base"
               OpMemberName %12 0 "a1"
               OpMemberName %12 1 "a2"
               OpName %14 "b"
               OpName %18 "level_1"
               OpMemberName %18 0 "b1"
               OpMemberName %18 1 "b2"
               OpMemberName %18 2 "b3"
               OpName %20 "l1"
               OpName %25 "level_2"
               OpMemberName %25 0 "c1"
               OpMemberName %25 1 "c2"
               OpName %27 "l2"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpTypeStruct %6 %6
         %13 = OpTypePointer Function %12
         %18 = OpTypeStruct %12 %12 %12
         %19 = OpTypePointer Function %18
         %25 = OpTypeStruct %18 %18
         %26 = OpTypePointer Function %25
         %31 = OpTypeBool
         %32 = OpConstantTrue %31
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %27 = OpVariable %26 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
         %21 = OpLoad %12 %14
         %22 = OpLoad %12 %14
         %23 = OpLoad %12 %14
         %24 = OpCompositeConstruct %18 %21 %22 %23
               OpStore %20 %24
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %20
         %30 = OpCompositeConstruct %25 %28 %29
               OpStore %27 %30
               OpSelectionMerge %34 None
               OpBranchConditional %32 %33 %34
         %33 = OpLabel
               OpBranch %34
         %34 = OpLabel
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
  auto transformation_good_1 = TransformationCompositeInsert(
      MakeInstructionDescriptor(30, SpvOpStore, 0), 50, 30, 11, {1, 0, 0});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // These synonyms should have been added.
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {0}), MakeDataDescriptor(50, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 1}), MakeDataDescriptor(50, {1, 1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 2}), MakeDataDescriptor(50, {1, 2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 0, 1}), MakeDataDescriptor(50, {1, 0, 1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {1, 0, 0}), MakeDataDescriptor(11, {})));

  // These synonyms should not have been added.
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1}), MakeDataDescriptor(50, {1})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 0}), MakeDataDescriptor(50, {1, 0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(30, {1, 0, 0}), MakeDataDescriptor(50, {1, 0, 0})));

  auto transformation_good_2 = TransformationCompositeInsert(
      MakeInstructionDescriptor(50, SpvOpStore, 0), 51, 50, 11, {0, 1, 1});
  ASSERT_TRUE(transformation_good_2.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // These synonyms should have been added.
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {1}), MakeDataDescriptor(51, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {0, 0}), MakeDataDescriptor(51, {0, 0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {0, 2}), MakeDataDescriptor(51, {0, 2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {0, 1, 0}), MakeDataDescriptor(51, {0, 1, 0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(51, {0, 1, 1}), MakeDataDescriptor(11, {})));

  // These synonyms should not have been added.
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {0}), MakeDataDescriptor(51, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {0, 1}), MakeDataDescriptor(51, {0, 1})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {0, 1, 1}), MakeDataDescriptor(51, {0, 1, 1})));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i1"
               OpName %10 "i2"
               OpName %12 "base"
               OpMemberName %12 0 "a1"
               OpMemberName %12 1 "a2"
               OpName %14 "b"
               OpName %18 "level_1"
               OpMemberName %18 0 "b1"
               OpMemberName %18 1 "b2"
               OpMemberName %18 2 "b3"
               OpName %20 "l1"
               OpName %25 "level_2"
               OpMemberName %25 0 "c1"
               OpMemberName %25 1 "c2"
               OpName %27 "l2"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpTypeStruct %6 %6
         %13 = OpTypePointer Function %12
         %18 = OpTypeStruct %12 %12 %12
         %19 = OpTypePointer Function %18
         %25 = OpTypeStruct %18 %18
         %26 = OpTypePointer Function %25
         %31 = OpTypeBool
         %32 = OpConstantTrue %31
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %20 = OpVariable %19 Function
         %27 = OpVariable %26 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
         %21 = OpLoad %12 %14
         %22 = OpLoad %12 %14
         %23 = OpLoad %12 %14
         %24 = OpCompositeConstruct %18 %21 %22 %23
               OpStore %20 %24
         %28 = OpLoad %18 %20
         %29 = OpLoad %18 %20
         %30 = OpCompositeConstruct %25 %28 %29
         %50 = OpCompositeInsert %25 %11 %30 1 0 0
         %51 = OpCompositeInsert %25 %11 %50 0 1 1
               OpStore %27 %30
               OpSelectionMerge %34 None
               OpBranchConditional %32 %33 %34
         %33 = OpLabel
               OpBranch %34
         %34 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationCompositeInsertTest, IdNotAvailableScenarios) {
  // This test handles cases where either the composite or the object is not
  // available before the |instruction_to_insert_before|.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i1"
               OpName %10 "i2"
               OpName %12 "base"
               OpMemberName %12 0 "a1"
               OpMemberName %12 1 "a2"
               OpName %14 "b1"
               OpName %18 "b2"
               OpName %22 "lvl1"
               OpMemberName %22 0 "b1"
               OpMemberName %22 1 "b2"
               OpName %24 "l1"
               OpName %28 "i3"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpTypeStruct %6 %6
         %13 = OpTypePointer Function %12
         %22 = OpTypeStruct %12 %12
         %23 = OpTypePointer Function %22
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %14 = OpVariable %13 Function
         %18 = OpVariable %13 Function
         %24 = OpVariable %23 Function
         %28 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %10 %11
         %15 = OpLoad %6 %8
         %16 = OpLoad %6 %10
         %17 = OpCompositeConstruct %12 %15 %16
               OpStore %14 %17
         %19 = OpLoad %6 %10
         %20 = OpLoad %6 %8
         %21 = OpCompositeConstruct %12 %19 %20
               OpStore %18 %21
         %25 = OpLoad %12 %14
         %26 = OpLoad %12 %18
         %27 = OpCompositeConstruct %22 %25 %26
               OpStore %24 %27
         %29 = OpLoad %6 %8
         %30 = OpLoad %6 %10
         %31 = OpIMul %6 %29 %30
               OpStore %28 %31
         %60 = OpCompositeConstruct %12 %20 %19
         %61 = OpCompositeConstruct %22 %26 %25
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
  // Bad: The object with |object_id| is not available at
  // |instruction_to_insert_before|.
  auto transformation_bad_1 = TransformationCompositeInsert(
      MakeInstructionDescriptor(31, SpvOpIMul, 0), 50, 27, 60, {1});
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Bad: The composite with |composite_id| is not available at
  // |instruction_to_insert_before|.
  auto transformation_bad_2 = TransformationCompositeInsert(
      MakeInstructionDescriptor(31, SpvOpIMul, 0), 50, 61, 21, {1});
  ASSERT_FALSE(
      transformation_bad_2.IsApplicable(context.get(), transformation_context));

  // Bad: The |instruction_to_insert_before| is the composite itself and is
  // available.
  auto transformation_bad_3 = TransformationCompositeInsert(
      MakeInstructionDescriptor(61, SpvOpCompositeConstruct, 0), 50, 61, 21,
      {1});
  ASSERT_FALSE(
      transformation_bad_3.IsApplicable(context.get(), transformation_context));

  // Bad: The |instruction_to_insert_before| is the object itself and is not
  // available.
  auto transformation_bad_4 = TransformationCompositeInsert(
      MakeInstructionDescriptor(60, SpvOpCompositeConstruct, 0), 50, 27, 60,
      {1});
  ASSERT_FALSE(
      transformation_bad_4.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationCompositeInsertTest, CompositeInsertionWithIrrelevantIds) {
  // This checks that we do *not* get data synonym facts when we do composite
  // insertion using irrelevant ids or in dead blocks.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %12 "main"
               OpExecutionMode %12 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeVector %6 2
          %8 = OpConstant %6 0
          %9 = OpConstantComposite %7 %8 %8
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
         %16 = OpConstant %6 0
         %17 = OpConstant %6 1
         %18 = OpConstantComposite %7 %8 %8
         %12 = OpFunction %2 None %3
         %13 = OpLabel
               OpSelectionMerge %15 None
               OpBranchConditional %11 %14 %15
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
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

  transformation_context.GetFactManager()->AddFactBlockIsDead(14);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(16);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(18);

  // Leads to synonyms - nothing is irrelevant.
  auto transformation1 = TransformationCompositeInsert(
      MakeInstructionDescriptor(13, SpvOpSelectionMerge, 0), 100, 9, 17, {0});
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {0}), MakeDataDescriptor(17, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {1}), MakeDataDescriptor(9, {1})));

  // Because %16 is irrelevant, we don't get a synonym with the component to
  // which it has been inserted (but we do for the other component).
  auto transformation2 = TransformationCompositeInsert(
      MakeInstructionDescriptor(13, SpvOpSelectionMerge, 0), 101, 9, 16, {0});
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(101, {0}), MakeDataDescriptor(16, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(101, {1}), MakeDataDescriptor(9, {1})));

  // Because %18 is irrelevant we only get a synonym for the component into
  // which insertion has taken place.
  auto transformation3 = TransformationCompositeInsert(
      MakeInstructionDescriptor(13, SpvOpSelectionMerge, 0), 102, 18, 17, {0});
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(102, {0}), MakeDataDescriptor(17, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(102, {1}), MakeDataDescriptor(18, {1})));

  // Does not lead to synonyms as block %14 is dead.
  auto transformation4 = TransformationCompositeInsert(
      MakeInstructionDescriptor(14, SpvOpBranch, 0), 103, 9, 17, {0});
  ASSERT_TRUE(
      transformation4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation4, context.get(),
                        &transformation_context);
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(103, {0}), MakeDataDescriptor(17, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(103, {1}), MakeDataDescriptor(9, {1})));

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
