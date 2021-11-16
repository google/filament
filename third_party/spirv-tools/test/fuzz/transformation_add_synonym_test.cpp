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

#include "source/fuzz/transformation_add_synonym.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddSynonymTest, NotApplicable) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %8 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 3
         %10 = OpTypeFloat 32
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 4.5
         %14 = OpTypeVector %10 2
         %15 = OpTypePointer Function %14
         %17 = OpConstant %10 3
         %18 = OpConstant %10 4
         %19 = OpConstantComposite %14 %17 %18
         %20 = OpTypeVector %6 2
         %21 = OpTypePointer Function %20
         %23 = OpConstant %6 4
         %24 = OpConstantComposite %20 %9 %23
         %26 = OpConstantNull %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %16 = OpVariable %15 Function
         %22 = OpVariable %21 Function
               OpStore %8 %9
               OpStore %12 %13
               OpStore %16 %19
               OpStore %22 %24
         %25 = OpUndef %6
         %27 = OpLoad %6 %8
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
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(24);

  auto insert_before = MakeInstructionDescriptor(22, SpvOpReturn, 0);

#ifndef NDEBUG
  ASSERT_DEATH(
      TransformationAddSynonym(
          9, static_cast<protobufs::TransformationAddSynonym::SynonymType>(-1),
          40, insert_before)
          .IsApplicable(context.get(), transformation_context),
      "Synonym type is invalid");
#endif

  // These tests should succeed regardless of the synonym type.
  for (int i = 0;
       i < protobufs::TransformationAddSynonym::SynonymType_descriptor()
               ->value_count();
       ++i) {
    const auto* synonym_value =
        protobufs::TransformationAddSynonym::SynonymType_descriptor()->value(i);
    ASSERT_TRUE(protobufs::TransformationAddSynonym::SynonymType_IsValid(
        synonym_value->number()));
    auto synonym_type =
        static_cast<protobufs::TransformationAddSynonym::SynonymType>(
            synonym_value->number());

    // |synonym_fresh_id| is not fresh.
    ASSERT_FALSE(TransformationAddSynonym(9, synonym_type, 9, insert_before)
                     .IsApplicable(context.get(), transformation_context));

    // |result_id| is invalid.
    ASSERT_FALSE(TransformationAddSynonym(40, synonym_type, 40, insert_before)
                     .IsApplicable(context.get(), transformation_context));

    // Instruction with |result_id| has no type id.
    ASSERT_FALSE(TransformationAddSynonym(5, synonym_type, 40, insert_before)
                     .IsApplicable(context.get(), transformation_context));

    // Instruction with |result_id| is an OpUndef.
    ASSERT_FALSE(TransformationAddSynonym(25, synonym_type, 40, insert_before)
                     .IsApplicable(context.get(), transformation_context));

    // Instruction with |result_id| is an OpConstantNull.
    ASSERT_FALSE(TransformationAddSynonym(26, synonym_type, 40, insert_before)
                     .IsApplicable(context.get(), transformation_context));

    // |result_id| is irrelevant.
    ASSERT_FALSE(TransformationAddSynonym(24, synonym_type, 40, insert_before)
                     .IsApplicable(context.get(), transformation_context));

    // |insert_before| is invalid.
    ASSERT_FALSE(
        TransformationAddSynonym(9, synonym_type, 40,
                                 MakeInstructionDescriptor(25, SpvOpStore, 0))
            .IsApplicable(context.get(), transformation_context));

    // Can't insert before |insert_before|.
    ASSERT_FALSE(
        TransformationAddSynonym(9, synonym_type, 40,
                                 MakeInstructionDescriptor(5, SpvOpLabel, 0))
            .IsApplicable(context.get(), transformation_context));
    ASSERT_FALSE(TransformationAddSynonym(
                     9, synonym_type, 40,
                     MakeInstructionDescriptor(22, SpvOpVariable, 0))
                     .IsApplicable(context.get(), transformation_context));
    ASSERT_FALSE(TransformationAddSynonym(
                     9, synonym_type, 40,
                     MakeInstructionDescriptor(25, SpvOpFunctionEnd, 0))
                     .IsApplicable(context.get(), transformation_context));

    // Domination rules are not satisfied.
    ASSERT_FALSE(
        TransformationAddSynonym(27, synonym_type, 40,
                                 MakeInstructionDescriptor(27, SpvOpLoad, 0))
            .IsApplicable(context.get(), transformation_context));
    ASSERT_FALSE(
        TransformationAddSynonym(27, synonym_type, 40,
                                 MakeInstructionDescriptor(22, SpvOpStore, 1))
            .IsApplicable(context.get(), transformation_context));
  }
}

TEST(TransformationAddSynonymTest, AddZeroSubZeroMulOne) {
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
          %7 = OpConstant %6 0
          %8 = OpConstant %6 1
          %9 = OpConstant %6 34
         %10 = OpTypeInt 32 0
         %13 = OpConstant %10 34
         %14 = OpTypeFloat 32
         %15 = OpConstant %14 0
         %16 = OpConstant %14 1
         %17 = OpConstant %14 34
         %18 = OpTypeVector %14 2
         %19 = OpConstantComposite %18 %15 %15
         %20 = OpConstantComposite %18 %16 %16
         %21 = OpConstant %14 3
         %22 = OpConstant %14 4
         %23 = OpConstantComposite %18 %21 %22
         %24 = OpTypeVector %6 2
         %25 = OpConstantComposite %24 %7 %7
         %26 = OpConstantComposite %24 %8 %8
         %27 = OpConstant %6 3
         %28 = OpConstant %6 4
         %29 = OpConstantComposite %24 %27 %28
         %30 = OpTypeVector %10 2
         %33 = OpConstant %10 3
         %34 = OpConstant %10 4
         %35 = OpConstantComposite %30 %33 %34
         %36 = OpTypeBool
         %37 = OpTypeVector %36 2
         %38 = OpConstantTrue %36
         %39 = OpConstantComposite %37 %38 %38
         %40 = OpConstant %6 37
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
  auto insert_before = MakeInstructionDescriptor(5, SpvOpReturn, 0);

  uint32_t fresh_id = 50;
  for (auto synonym_type : {protobufs::TransformationAddSynonym::ADD_ZERO,
                            protobufs::TransformationAddSynonym::SUB_ZERO,
                            protobufs::TransformationAddSynonym::MUL_ONE}) {
    ASSERT_TRUE(
        TransformationAddSynonym::IsAdditionalConstantRequired(synonym_type));

    // Can't create a synonym of a scalar or a vector of a wrong (in this case -
    // boolean) type.
    ASSERT_FALSE(
        TransformationAddSynonym(38, synonym_type, fresh_id, insert_before)
            .IsApplicable(context.get(), transformation_context));
    ASSERT_FALSE(
        TransformationAddSynonym(39, synonym_type, fresh_id, insert_before)
            .IsApplicable(context.get(), transformation_context));

    // Required constant is not present in the module.
    ASSERT_FALSE(
        TransformationAddSynonym(13, synonym_type, fresh_id, insert_before)
            .IsApplicable(context.get(), transformation_context));
    ASSERT_FALSE(
        TransformationAddSynonym(35, synonym_type, fresh_id, insert_before)
            .IsApplicable(context.get(), transformation_context));

    for (auto result_id : {9, 17, 23, 29}) {
      TransformationAddSynonym transformation(result_id, synonym_type, fresh_id,
                                              insert_before);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
      ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
          MakeDataDescriptor(result_id, {}), MakeDataDescriptor(fresh_id, {})));
      ++fresh_id;
    }
  }
  {
    TransformationAddSynonym transformation(
        40, protobufs::TransformationAddSynonym::BITWISE_OR, fresh_id,
        insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(40, {}), MakeDataDescriptor(fresh_id, {})));
    ++fresh_id;
  }
  {
    TransformationAddSynonym transformation(
        40, protobufs::TransformationAddSynonym::BITWISE_XOR, fresh_id,
        insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(40, {}), MakeDataDescriptor(fresh_id, {})));
  }

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 0
          %8 = OpConstant %6 1
          %9 = OpConstant %6 34
         %10 = OpTypeInt 32 0
         %13 = OpConstant %10 34
         %14 = OpTypeFloat 32
         %15 = OpConstant %14 0
         %16 = OpConstant %14 1
         %17 = OpConstant %14 34
         %18 = OpTypeVector %14 2
         %19 = OpConstantComposite %18 %15 %15
         %20 = OpConstantComposite %18 %16 %16
         %21 = OpConstant %14 3
         %22 = OpConstant %14 4
         %23 = OpConstantComposite %18 %21 %22
         %24 = OpTypeVector %6 2
         %25 = OpConstantComposite %24 %7 %7
         %26 = OpConstantComposite %24 %8 %8
         %27 = OpConstant %6 3
         %28 = OpConstant %6 4
         %29 = OpConstantComposite %24 %27 %28
         %30 = OpTypeVector %10 2
         %33 = OpConstant %10 3
         %34 = OpConstant %10 4
         %35 = OpConstantComposite %30 %33 %34
         %36 = OpTypeBool
         %37 = OpTypeVector %36 2
         %38 = OpConstantTrue %36
         %39 = OpConstantComposite %37 %38 %38
         %40 = OpConstant %6 37
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %50 = OpIAdd %6 %9 %7
         %51 = OpFAdd %14 %17 %15
         %52 = OpFAdd %18 %23 %19
         %53 = OpIAdd %24 %29 %25
         %54 = OpISub %6 %9 %7
         %55 = OpFSub %14 %17 %15
         %56 = OpFSub %18 %23 %19
         %57 = OpISub %24 %29 %25
         %58 = OpIMul %6 %9 %8
         %59 = OpFMul %14 %17 %16
         %60 = OpFMul %18 %23 %20
         %61 = OpIMul %24 %29 %26
         %62 = OpBitwiseOr %6 %40 %7
         %63 = OpBitwiseXor %6 %40 %7
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationAddSynonymTest, LogicalAndLogicalOr) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %9 = OpConstantTrue %6
         %10 = OpTypeVector %6 2
         %11 = OpConstantComposite %10 %7 %9
         %12 = OpConstantComposite %10 %7 %7
         %13 = OpConstantComposite %10 %9 %9
         %14 = OpTypeFloat 32
         %17 = OpConstant %14 35
         %18 = OpTypeVector %14 2
         %21 = OpConstant %14 3
         %22 = OpConstant %14 4
         %23 = OpConstantComposite %18 %21 %22
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
  auto insert_before = MakeInstructionDescriptor(5, SpvOpReturn, 0);

  uint32_t fresh_id = 50;
  for (auto synonym_type : {protobufs::TransformationAddSynonym::LOGICAL_AND,
                            protobufs::TransformationAddSynonym::LOGICAL_OR}) {
    ASSERT_TRUE(
        TransformationAddSynonym::IsAdditionalConstantRequired(synonym_type));

    // Can't create a synonym of a scalar or a vector of a wrong (in this case -
    // float) type.
    ASSERT_FALSE(
        TransformationAddSynonym(17, synonym_type, fresh_id, insert_before)
            .IsApplicable(context.get(), transformation_context));
    ASSERT_FALSE(
        TransformationAddSynonym(23, synonym_type, fresh_id, insert_before)
            .IsApplicable(context.get(), transformation_context));

    for (auto result_id : {9, 11}) {
      TransformationAddSynonym transformation(result_id, synonym_type, fresh_id,
                                              insert_before);
      ASSERT_TRUE(
          transformation.IsApplicable(context.get(), transformation_context));
      ApplyAndCheckFreshIds(transformation, context.get(),
                            &transformation_context);
      ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
          MakeDataDescriptor(result_id, {}), MakeDataDescriptor(fresh_id, {})));
      ++fresh_id;
    }
  }

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %9 = OpConstantTrue %6
         %10 = OpTypeVector %6 2
         %11 = OpConstantComposite %10 %7 %9
         %12 = OpConstantComposite %10 %7 %7
         %13 = OpConstantComposite %10 %9 %9
         %14 = OpTypeFloat 32
         %17 = OpConstant %14 35
         %18 = OpTypeVector %14 2
         %21 = OpConstant %14 3
         %22 = OpConstant %14 4
         %23 = OpConstantComposite %18 %21 %22
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %50 = OpLogicalAnd %6 %9 %9
         %51 = OpLogicalAnd %10 %11 %13
         %52 = OpLogicalOr %6 %9 %7
         %53 = OpLogicalOr %10 %11 %12
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationAddSynonymTest, LogicalAndConstantIsNotPresent) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
         %10 = OpTypeVector %6 2
         %12 = OpConstantComposite %10 %7 %7
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
  auto insert_before = MakeInstructionDescriptor(5, SpvOpReturn, 0);
  const auto synonym_type = protobufs::TransformationAddSynonym::LOGICAL_AND;

  // Required constant is not present in the module.
  ASSERT_FALSE(TransformationAddSynonym(7, synonym_type, 50, insert_before)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationAddSynonym(12, synonym_type, 50, insert_before)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddSynonymTest, LogicalOrConstantIsNotPresent) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %10 = OpTypeVector %6 2
         %12 = OpConstantComposite %10 %7 %7
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
  auto insert_before = MakeInstructionDescriptor(5, SpvOpReturn, 0);
  const auto synonym_type = protobufs::TransformationAddSynonym::LOGICAL_OR;

  // Required constant is not present in the module.
  ASSERT_FALSE(TransformationAddSynonym(7, synonym_type, 50, insert_before)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationAddSynonym(12, synonym_type, 50, insert_before)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddSynonymTest, CopyObject) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %8 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 4
         %10 = OpTypeFloat 32
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 4
         %14 = OpTypeVector %10 2
         %15 = OpTypePointer Function %14
         %17 = OpConstant %10 3.4000001
         %18 = OpConstantComposite %14 %17 %17
         %19 = OpTypeBool
         %20 = OpTypeStruct %19
         %21 = OpTypePointer Function %20
         %23 = OpConstantTrue %19
         %24 = OpConstantComposite %20 %23
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %16 = OpVariable %15 Function
         %22 = OpVariable %21 Function
               OpStore %8 %9
               OpStore %12 %13
               OpStore %16 %18
               OpStore %22 %24
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
  auto insert_before = MakeInstructionDescriptor(5, SpvOpReturn, 0);
  const auto synonym_type = protobufs::TransformationAddSynonym::COPY_OBJECT;

  ASSERT_FALSE(
      TransformationAddSynonym::IsAdditionalConstantRequired(synonym_type));

  uint32_t fresh_id = 50;
  for (auto result_id : {9, 13, 17, 18, 23, 24, 22}) {
    TransformationAddSynonym transformation(result_id, synonym_type, fresh_id,
                                            insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(result_id, {}), MakeDataDescriptor(fresh_id, {})));
    ++fresh_id;
  }

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %8 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 4
         %10 = OpTypeFloat 32
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 4
         %14 = OpTypeVector %10 2
         %15 = OpTypePointer Function %14
         %17 = OpConstant %10 3.4000001
         %18 = OpConstantComposite %14 %17 %17
         %19 = OpTypeBool
         %20 = OpTypeStruct %19
         %21 = OpTypePointer Function %20
         %23 = OpConstantTrue %19
         %24 = OpConstantComposite %20 %23
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %16 = OpVariable %15 Function
         %22 = OpVariable %21 Function
               OpStore %8 %9
               OpStore %12 %13
               OpStore %16 %18
               OpStore %22 %24
         %50 = OpCopyObject %6 %9
         %51 = OpCopyObject %10 %13
         %52 = OpCopyObject %10 %17
         %53 = OpCopyObject %14 %18
         %54 = OpCopyObject %19 %23
         %55 = OpCopyObject %20 %24
         %56 = OpCopyObject %21 %22
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationAddSynonymTest, CopyBooleanConstants) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %8 = OpConstantFalse %6
          %3 = OpTypeFunction %2
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
  ASSERT_EQ(0, transformation_context.GetFactManager()
                   ->GetIdsForWhichSynonymsAreKnown()
                   .size());

  {
    TransformationAddSynonym copy_true(
        7, protobufs::TransformationAddSynonym::COPY_OBJECT, 100,
        MakeInstructionDescriptor(5, SpvOpReturn, 0));
    ASSERT_TRUE(copy_true.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(copy_true, context.get(), &transformation_context);

    std::vector<uint32_t> ids_for_which_synonyms_are_known =
        transformation_context.GetFactManager()
            ->GetIdsForWhichSynonymsAreKnown();
    ASSERT_EQ(2, ids_for_which_synonyms_are_known.size());
    ASSERT_TRUE(std::find(ids_for_which_synonyms_are_known.begin(),
                          ids_for_which_synonyms_are_known.end(),
                          7) != ids_for_which_synonyms_are_known.end());
    ASSERT_EQ(
        2, transformation_context.GetFactManager()->GetSynonymsForId(7).size());
    protobufs::DataDescriptor descriptor_100 = MakeDataDescriptor(100, {});
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(7, {}), descriptor_100));
  }

  {
    TransformationAddSynonym copy_false(
        8, protobufs::TransformationAddSynonym::COPY_OBJECT, 101,
        MakeInstructionDescriptor(100, SpvOpReturn, 0));
    ASSERT_TRUE(copy_false.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(copy_false, context.get(), &transformation_context);
    std::vector<uint32_t> ids_for_which_synonyms_are_known =
        transformation_context.GetFactManager()
            ->GetIdsForWhichSynonymsAreKnown();
    ASSERT_EQ(4, ids_for_which_synonyms_are_known.size());
    ASSERT_TRUE(std::find(ids_for_which_synonyms_are_known.begin(),
                          ids_for_which_synonyms_are_known.end(),
                          8) != ids_for_which_synonyms_are_known.end());
    ASSERT_EQ(
        2, transformation_context.GetFactManager()->GetSynonymsForId(8).size());
    protobufs::DataDescriptor descriptor_101 = MakeDataDescriptor(101, {});
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(8, {}), descriptor_101));
  }

  {
    TransformationAddSynonym copy_false_again(
        101, protobufs::TransformationAddSynonym::COPY_OBJECT, 102,
        MakeInstructionDescriptor(5, SpvOpReturn, 0));
    ASSERT_TRUE(
        copy_false_again.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(copy_false_again, context.get(),
                          &transformation_context);
    std::vector<uint32_t> ids_for_which_synonyms_are_known =
        transformation_context.GetFactManager()
            ->GetIdsForWhichSynonymsAreKnown();
    ASSERT_EQ(5, ids_for_which_synonyms_are_known.size());
    ASSERT_TRUE(std::find(ids_for_which_synonyms_are_known.begin(),
                          ids_for_which_synonyms_are_known.end(),
                          101) != ids_for_which_synonyms_are_known.end());
    ASSERT_EQ(
        3,
        transformation_context.GetFactManager()->GetSynonymsForId(101).size());
    protobufs::DataDescriptor descriptor_102 = MakeDataDescriptor(102, {});
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(101, {}), descriptor_102));
  }

  {
    TransformationAddSynonym copy_true_again(
        7, protobufs::TransformationAddSynonym::COPY_OBJECT, 103,
        MakeInstructionDescriptor(102, SpvOpReturn, 0));
    ASSERT_TRUE(
        copy_true_again.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(copy_true_again, context.get(),
                          &transformation_context);
    std::vector<uint32_t> ids_for_which_synonyms_are_known =
        transformation_context.GetFactManager()
            ->GetIdsForWhichSynonymsAreKnown();
    ASSERT_EQ(6, ids_for_which_synonyms_are_known.size());
    ASSERT_TRUE(std::find(ids_for_which_synonyms_are_known.begin(),
                          ids_for_which_synonyms_are_known.end(),
                          7) != ids_for_which_synonyms_are_known.end());
    ASSERT_EQ(
        3, transformation_context.GetFactManager()->GetSynonymsForId(7).size());
    protobufs::DataDescriptor descriptor_103 = MakeDataDescriptor(103, {});
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(7, {}), descriptor_103));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %8 = OpConstantFalse %6
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %100 = OpCopyObject %6 %7
        %101 = OpCopyObject %6 %8
        %102 = OpCopyObject %6 %101
        %103 = OpCopyObject %6 %7
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddSynonymTest, CheckIllegalCases) {
  // The following SPIR-V comes from this GLSL, pushed through spirv-opt
  // and then doctored a bit.
  //
  // #version 310 es
  //
  // precision highp float;
  //
  // struct S {
  //   int a;
  //   float b;
  // };
  //
  // layout(set = 0, binding = 2) uniform block {
  //   S s;
  //   lowp float f;
  //   int ii;
  // } ubuf;
  //
  // layout(location = 0) out vec4 color;
  //
  // void main() {
  //   float c = 0.0;
  //   lowp float d = 0.0;
  //   S localS = ubuf.s;
  //   for (int i = 0; i < ubuf.s.a; i++) {
  //     switch (ubuf.ii) {
  //       case 0:
  //         c += 0.1;
  //         d += 0.2;
  //       case 1:
  //         c += 0.1;
  //         if (c > d) {
  //           d += 0.2;
  //         } else {
  //           d += c;
  //         }
  //         break;
  //       default:
  //         i += 1;
  //         localS.b += d;
  //     }
  //   }
  //   color = vec4(c, d, localS.b, 1.0);
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %80
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %12 "S"
               OpMemberName %12 0 "a"
               OpMemberName %12 1 "b"
               OpName %15 "S"
               OpMemberName %15 0 "a"
               OpMemberName %15 1 "b"
               OpName %16 "block"
               OpMemberName %16 0 "s"
               OpMemberName %16 1 "f"
               OpMemberName %16 2 "ii"
               OpName %18 "ubuf"
               OpName %80 "color"
               OpMemberDecorate %12 0 RelaxedPrecision
               OpMemberDecorate %15 0 RelaxedPrecision
               OpMemberDecorate %15 0 Offset 0
               OpMemberDecorate %15 1 Offset 4
               OpMemberDecorate %16 0 Offset 0
               OpMemberDecorate %16 1 RelaxedPrecision
               OpMemberDecorate %16 1 Offset 16
               OpMemberDecorate %16 2 RelaxedPrecision
               OpMemberDecorate %16 2 Offset 20
               OpDecorate %16 Block
               OpDecorate %18 DescriptorSet 0
               OpDecorate %18 Binding 2
               OpDecorate %38 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 Location 0
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %9 = OpConstant %6 0
         %11 = OpTypeInt 32 1
         %12 = OpTypeStruct %11 %6
         %15 = OpTypeStruct %11 %6
         %16 = OpTypeStruct %15 %6 %11
         %17 = OpTypePointer Uniform %16
         %18 = OpVariable %17 Uniform
         %19 = OpConstant %11 0
         %20 = OpTypePointer Uniform %15
         %27 = OpConstant %11 1
         %36 = OpTypePointer Uniform %11
         %39 = OpTypeBool
         %41 = OpConstant %11 2
         %48 = OpConstant %6 0.100000001
         %51 = OpConstant %6 0.200000003
         %78 = OpTypeVector %6 4
         %79 = OpTypePointer Output %78
         %80 = OpVariable %79 Output
         %85 = OpConstant %6 1
         %95 = OpUndef %12
        %112 = OpTypePointer Uniform %6
        %113 = OpTypeInt 32 0
        %114 = OpConstant %113 1
        %179 = OpTypePointer Function %39
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %180 = OpVariable %179 Function
        %181 = OpVariable %179 Function
        %182 = OpVariable %179 Function
         %21 = OpAccessChain %20 %18 %19
        %115 = OpAccessChain %112 %21 %114
        %116 = OpLoad %6 %115
         %90 = OpCompositeInsert %12 %116 %95 1
               OpBranch %30
         %30 = OpLabel
         %99 = OpPhi %12 %90 %5 %109 %47
         %98 = OpPhi %6 %9 %5 %107 %47
         %97 = OpPhi %6 %9 %5 %105 %47
         %96 = OpPhi %11 %19 %5 %77 %47
         %37 = OpAccessChain %36 %18 %19 %19
         %38 = OpLoad %11 %37
         %40 = OpSLessThan %39 %96 %38
               OpLoopMerge %32 %47 None
               OpBranchConditional %40 %31 %32
         %31 = OpLabel
         %42 = OpAccessChain %36 %18 %41
         %43 = OpLoad %11 %42
               OpSelectionMerge %45 None
               OpSwitch %43 %46 0 %44 1 %45
         %46 = OpLabel
         %69 = OpIAdd %11 %96 %27
         %72 = OpCompositeExtract %6 %99 1
         %73 = OpFAdd %6 %72 %98
         %93 = OpCompositeInsert %12 %73 %99 1
               OpBranch %47
         %44 = OpLabel
         %50 = OpFAdd %6 %97 %48
         %53 = OpFAdd %6 %98 %51
               OpBranch %45
         %45 = OpLabel
        %101 = OpPhi %6 %98 %31 %53 %44
        %100 = OpPhi %6 %97 %31 %50 %44
         %55 = OpFAdd %6 %100 %48
         %58 = OpFOrdGreaterThan %39 %55 %101
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %63
         %59 = OpLabel
         %62 = OpFAdd %6 %101 %51
               OpBranch %60
         %63 = OpLabel
         %66 = OpFAdd %6 %101 %55
               OpBranch %60
         %60 = OpLabel
        %108 = OpPhi %6 %62 %59 %66 %63
               OpBranch %47
         %47 = OpLabel
        %109 = OpPhi %12 %93 %46 %99 %60
        %107 = OpPhi %6 %98 %46 %108 %60
        %105 = OpPhi %6 %97 %46 %55 %60
        %102 = OpPhi %11 %69 %46 %96 %60
         %77 = OpIAdd %11 %102 %27
               OpBranch %30
         %32 = OpLabel
         %84 = OpCompositeExtract %6 %99 1
         %86 = OpCompositeConstruct %78 %97 %98 %84 %85
               OpStore %80 %86
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
  // Inapplicable because %18 is decorated.
  ASSERT_FALSE(TransformationAddSynonym(
                   18, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(21, SpvOpAccessChain, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Inapplicable because %77 is decorated.
  ASSERT_FALSE(TransformationAddSynonym(
                   77, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(77, SpvOpBranch, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Inapplicable because %80 is decorated.
  ASSERT_FALSE(TransformationAddSynonym(
                   80, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(77, SpvOpIAdd, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Inapplicable because %84 is not available at the requested point
  ASSERT_FALSE(TransformationAddSynonym(
                   84, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(32, SpvOpCompositeExtract, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Fine because %84 is available at the requested point
  ASSERT_TRUE(TransformationAddSynonym(
                  84, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                  MakeInstructionDescriptor(32, SpvOpCompositeConstruct, 0))
                  .IsApplicable(context.get(), transformation_context));

  // Inapplicable because id %9 is already in use
  ASSERT_FALSE(TransformationAddSynonym(
                   84, protobufs::TransformationAddSynonym::COPY_OBJECT, 9,
                   MakeInstructionDescriptor(32, SpvOpCompositeConstruct, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Inapplicable because the requested point does not exist
  ASSERT_FALSE(TransformationAddSynonym(
                   84, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(86, SpvOpReturn, 2))
                   .IsApplicable(context.get(), transformation_context));

  // Inapplicable because %9 is not in a function
  ASSERT_FALSE(TransformationAddSynonym(
                   9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(9, SpvOpTypeInt, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Inapplicable because the insert point is right before, or inside, a chunk
  // of OpPhis
  ASSERT_FALSE(TransformationAddSynonym(
                   9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(30, SpvOpPhi, 0))
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationAddSynonym(
                   9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(99, SpvOpPhi, 1))
                   .IsApplicable(context.get(), transformation_context));

  // OK, because the insert point is just after a chunk of OpPhis.
  ASSERT_TRUE(TransformationAddSynonym(
                  9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                  MakeInstructionDescriptor(96, SpvOpAccessChain, 0))
                  .IsApplicable(context.get(), transformation_context));

  // Inapplicable because the insert point is right after an OpSelectionMerge
  ASSERT_FALSE(TransformationAddSynonym(
                   9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(58, SpvOpBranchConditional, 0))
                   .IsApplicable(context.get(), transformation_context));

  // OK, because the insert point is right before the OpSelectionMerge
  ASSERT_TRUE(TransformationAddSynonym(
                  9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                  MakeInstructionDescriptor(58, SpvOpSelectionMerge, 0))
                  .IsApplicable(context.get(), transformation_context));

  // Inapplicable because the insert point is right after an OpSelectionMerge
  ASSERT_FALSE(TransformationAddSynonym(
                   9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(43, SpvOpSwitch, 0))
                   .IsApplicable(context.get(), transformation_context));

  // OK, because the insert point is right before the OpSelectionMerge
  ASSERT_TRUE(TransformationAddSynonym(
                  9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                  MakeInstructionDescriptor(43, SpvOpSelectionMerge, 0))
                  .IsApplicable(context.get(), transformation_context));

  // Inapplicable because the insert point is right after an OpLoopMerge
  ASSERT_FALSE(TransformationAddSynonym(
                   9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(40, SpvOpBranchConditional, 0))
                   .IsApplicable(context.get(), transformation_context));

  // OK, because the insert point is right before the OpLoopMerge
  ASSERT_TRUE(TransformationAddSynonym(
                  9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                  MakeInstructionDescriptor(40, SpvOpLoopMerge, 0))
                  .IsApplicable(context.get(), transformation_context));

  // Inapplicable because id %300 does not exist
  ASSERT_FALSE(TransformationAddSynonym(
                   300, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(40, SpvOpLoopMerge, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Inapplicable because the following instruction is OpVariable
  ASSERT_FALSE(TransformationAddSynonym(
                   9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(180, SpvOpVariable, 0))
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationAddSynonym(
                   9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(181, SpvOpVariable, 0))
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationAddSynonym(
                   9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                   MakeInstructionDescriptor(182, SpvOpVariable, 0))
                   .IsApplicable(context.get(), transformation_context));

  // OK, because this is just past the group of OpVariable instructions.
  ASSERT_TRUE(TransformationAddSynonym(
                  9, protobufs::TransformationAddSynonym::COPY_OBJECT, 200,
                  MakeInstructionDescriptor(182, SpvOpAccessChain, 0))
                  .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddSynonymTest, MiscellaneousCopies) {
  // The following SPIR-V comes from this GLSL:
  //
  // #version 310 es
  //
  // precision highp float;
  //
  // float g;
  //
  // vec4 h;
  //
  // void main() {
  //   int a;
  //   int b;
  //   b = int(g);
  //   h.x = float(a);
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "b"
               OpName %11 "g"
               OpName %16 "h"
               OpName %17 "a"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypeFloat 32
         %10 = OpTypePointer Private %9
         %11 = OpVariable %10 Private
         %14 = OpTypeVector %9 4
         %15 = OpTypePointer Private %14
         %16 = OpVariable %15 Private
         %20 = OpTypeInt 32 0
         %21 = OpConstant %20 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %17 = OpVariable %7 Function
         %12 = OpLoad %9 %11
         %13 = OpConvertFToS %6 %12
               OpStore %8 %13
         %18 = OpLoad %6 %17
         %19 = OpConvertSToF %9 %18
         %22 = OpAccessChain %10 %16 %21
               OpStore %22 %19
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
  std::vector<TransformationAddSynonym> transformations = {
      TransformationAddSynonym(
          19, protobufs::TransformationAddSynonym::COPY_OBJECT, 100,
          MakeInstructionDescriptor(22, SpvOpStore, 0)),
      TransformationAddSynonym(
          22, protobufs::TransformationAddSynonym::COPY_OBJECT, 101,
          MakeInstructionDescriptor(22, SpvOpCopyObject, 0)),
      TransformationAddSynonym(
          12, protobufs::TransformationAddSynonym::COPY_OBJECT, 102,
          MakeInstructionDescriptor(22, SpvOpCopyObject, 0)),
      TransformationAddSynonym(
          11, protobufs::TransformationAddSynonym::COPY_OBJECT, 103,
          MakeInstructionDescriptor(22, SpvOpCopyObject, 0)),
      TransformationAddSynonym(
          16, protobufs::TransformationAddSynonym::COPY_OBJECT, 104,
          MakeInstructionDescriptor(22, SpvOpCopyObject, 0)),
      TransformationAddSynonym(
          8, protobufs::TransformationAddSynonym::COPY_OBJECT, 105,
          MakeInstructionDescriptor(22, SpvOpCopyObject, 0)),
      TransformationAddSynonym(
          17, protobufs::TransformationAddSynonym::COPY_OBJECT, 106,
          MakeInstructionDescriptor(22, SpvOpCopyObject, 0))};

  for (auto& transformation : transformations) {
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

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
               OpName %8 "b"
               OpName %11 "g"
               OpName %16 "h"
               OpName %17 "a"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypeFloat 32
         %10 = OpTypePointer Private %9
         %11 = OpVariable %10 Private
         %14 = OpTypeVector %9 4
         %15 = OpTypePointer Private %14
         %16 = OpVariable %15 Private
         %20 = OpTypeInt 32 0
         %21 = OpConstant %20 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %17 = OpVariable %7 Function
         %12 = OpLoad %9 %11
         %13 = OpConvertFToS %6 %12
               OpStore %8 %13
         %18 = OpLoad %6 %17
         %19 = OpConvertSToF %9 %18
         %22 = OpAccessChain %10 %16 %21
        %106 = OpCopyObject %7 %17
        %105 = OpCopyObject %7 %8
        %104 = OpCopyObject %15 %16
        %103 = OpCopyObject %10 %11
        %102 = OpCopyObject %9 %12
        %101 = OpCopyObject %10 %22
        %100 = OpCopyObject %9 %19
               OpStore %22 %19
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddSynonymTest, DoNotCopyNullPointers) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpConstantNull %7
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
  // Illegal to copy null.
  ASSERT_FALSE(TransformationAddSynonym(
                   8, protobufs::TransformationAddSynonym::COPY_OBJECT, 100,
                   MakeInstructionDescriptor(5, SpvOpReturn, 0))
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddSynonymTest, PropagateIrrelevantPointeeFact) {
  // Checks that if a pointer is known to have an irrelevant value, the same
  // holds after the pointer is copied.

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
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
          %9 = OpVariable %7 Function
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
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(8);

  TransformationAddSynonym transformation1(
      8, protobufs::TransformationAddSynonym::COPY_OBJECT, 100,
      MakeInstructionDescriptor(9, SpvOpReturn, 0));
  TransformationAddSynonym transformation2(
      9, protobufs::TransformationAddSynonym::COPY_OBJECT, 101,
      MakeInstructionDescriptor(9, SpvOpReturn, 0));
  TransformationAddSynonym transformation3(
      100, protobufs::TransformationAddSynonym::COPY_OBJECT, 102,
      MakeInstructionDescriptor(9, SpvOpReturn, 0));

  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);

  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(8));
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(100));
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(102));
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(9));
  ASSERT_FALSE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(101));
}

TEST(TransformationAddSynonymTest, DoNotCopyOpSampledImage) {
  // This checks that we do not try to copy the result id of an OpSampledImage
  // instruction.
  std::string shader = R"(
               OpCapability Shader
               OpCapability SampledBuffer
               OpCapability ImageBuffer
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main" %40 %41
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %40 DescriptorSet 0
               OpDecorate %40 Binding 69
               OpDecorate %41 DescriptorSet 0
               OpDecorate %41 Binding 1
         %54 = OpTypeFloat 32
         %76 = OpTypeVector %54 4
         %55 = OpConstant %54 0
         %56 = OpTypeVector %54 3
         %94 = OpTypeVector %54 2
        %112 = OpConstantComposite %94 %55 %55
         %57 = OpConstantComposite %56 %55 %55 %55
         %15 = OpTypeImage %54 2D 2 0 0 1 Unknown
        %114 = OpTypePointer UniformConstant %15
         %38 = OpTypeSampler
        %125 = OpTypePointer UniformConstant %38
        %132 = OpTypeVoid
        %133 = OpTypeFunction %132
         %45 = OpTypeSampledImage %15
         %40 = OpVariable %114 UniformConstant
         %41 = OpVariable %125 UniformConstant
          %2 = OpFunction %132 None %133
        %164 = OpLabel
        %184 = OpLoad %15 %40
        %213 = OpLoad %38 %41
        %216 = OpSampledImage %45 %184 %213
        %217 = OpImageSampleImplicitLod %76 %216 %112 Bias %55
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_FALSE(
      TransformationAddSynonym(
          216, protobufs::TransformationAddSynonym::COPY_OBJECT, 500,
          MakeInstructionDescriptor(217, SpvOpImageSampleImplicitLod, 0))
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddSynonymTest, DoNotCopyVoidRunctionResult) {
  // This checks that we do not try to copy the result of a void function.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
               OpName %6 "foo("
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_FALSE(TransformationAddSynonym(
                   8, protobufs::TransformationAddSynonym::COPY_OBJECT, 500,
                   MakeInstructionDescriptor(8, SpvOpReturn, 0))
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddSynonymTest, HandlesDeadBlocks) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %11 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %12 = OpVariable %11 Function
               OpSelectionMerge %10 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  transformation_context.GetFactManager()->AddFactBlockIsDead(9);

  auto insert_before = MakeInstructionDescriptor(9, SpvOpBranch, 0);

  ASSERT_FALSE(TransformationAddSynonym(
                   7, protobufs::TransformationAddSynonym::COPY_OBJECT, 100,
                   insert_before)
                   .IsApplicable(context.get(), transformation_context));

  ASSERT_FALSE(TransformationAddSynonym(
                   12, protobufs::TransformationAddSynonym::COPY_OBJECT, 100,
                   insert_before)
                   .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
