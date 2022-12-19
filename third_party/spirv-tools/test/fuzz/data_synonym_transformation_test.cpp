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

#include "source/fuzz/data_descriptor.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/id_use_descriptor.h"
#include "source/fuzz/instruction_descriptor.h"
#include "source/fuzz/transformation_composite_extract.h"
#include "source/fuzz/transformation_replace_id_with_synonym.h"
#include "source/fuzz/transformation_vector_shuffle.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

// This file captures tests that check correctness of the collective use of a
// number of transformations that relate to data synonyms.

protobufs::Fact MakeSynonymFact(uint32_t first_id,
                                const std::vector<uint32_t>& first_indices,
                                uint32_t second_id,
                                const std::vector<uint32_t>& second_indices) {
  protobufs::FactDataSynonym data_synonym_fact;
  *data_synonym_fact.mutable_data1() =
      MakeDataDescriptor(first_id, first_indices);
  *data_synonym_fact.mutable_data2() =
      MakeDataDescriptor(second_id, second_indices);
  protobufs::Fact result;
  *result.mutable_data_synonym_fact() = data_synonym_fact;
  return result;
}

TEST(DataSynonymTransformationTest, ArrayCompositeSynonyms) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %11 "A"
               OpName %20 "B"
               OpName %31 "g"
               OpName %35 "h"
               OpDecorate %11 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 3
          %9 = OpTypeArray %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 0
         %13 = OpConstant %6 3
         %14 = OpTypePointer Function %6
         %16 = OpTypeFloat 32
         %17 = OpConstant %7 4
         %18 = OpTypeArray %16 %17
         %19 = OpTypePointer Function %18
         %24 = OpTypePointer Function %16
         %28 = OpConstant %16 42
         %30 = OpConstant %6 2
         %34 = OpConstant %6 1
         %38 = OpConstant %6 42
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %20 = OpVariable %19 Function
         %31 = OpVariable %24 Function
         %35 = OpVariable %14 Function
         %15 = OpAccessChain %14 %11 %12
         %21 = OpAccessChain %14 %11 %12
         %22 = OpLoad %6 %21
        %100 = OpCompositeConstruct %9 %12 %13 %22
               OpStore %15 %13
         %23 = OpConvertSToF %16 %22
         %25 = OpAccessChain %24 %20 %12
               OpStore %25 %23
         %26 = OpAccessChain %14 %11 %12
         %27 = OpLoad %6 %26
         %29 = OpAccessChain %24 %20 %27
               OpStore %29 %28
         %32 = OpLoad %16 %31
        %101 = OpCompositeConstruct %18 %28 %23 %32 %23
         %50 = OpCopyObject %16 %23
         %51 = OpCopyObject %16 %23
         %33 = OpAccessChain %24 %20 %30
               OpStore %33 %28
               OpStore %33 %32
         %36 = OpLoad %6 %35
         %37 = OpAccessChain %14 %11 %34
               OpStore %37 %36
         %39 = OpAccessChain %14 %11 %12
         %40 = OpLoad %6 %39
         %41 = OpIAdd %6 %38 %40
         %42 = OpAccessChain %14 %11 %30
               OpStore %42 %41
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

  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(12, {}, 100, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(13, {}, 100, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(22, {}, 100, {2}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(28, {}, 101, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(23, {}, 101, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(32, {}, 101, {2}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(23, {}, 101, {3}));

  // Replace %12 with %100[0] in '%25 = OpAccessChain %24 %20 %12'
  auto instruction_descriptor_1 =
      MakeInstructionDescriptor(25, spv::Op::OpAccessChain, 0);
  auto good_extract_1 =
      TransformationCompositeExtract(instruction_descriptor_1, 102, 100, {0});
  // Bad: id already in use
  auto bad_extract_1 = TransformationCompositeExtract(
      MakeInstructionDescriptor(25, spv::Op::OpAccessChain, 0), 25, 100, {0});
  ASSERT_TRUE(
      good_extract_1.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      bad_extract_1.IsApplicable(context.get(), transformation_context));
  good_extract_1.Apply(context.get(), &transformation_context);
  auto replacement_1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(12, instruction_descriptor_1, 1), 102);
  ASSERT_TRUE(
      replacement_1.IsApplicable(context.get(), transformation_context));
  replacement_1.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %13 with %100[1] in 'OpStore %15 %13'
  auto instruction_descriptor_2 =
      MakeInstructionDescriptor(100, spv::Op::OpStore, 0);
  auto good_extract_2 =
      TransformationCompositeExtract(instruction_descriptor_2, 103, 100, {1});
  // No bad example provided here.
  ASSERT_TRUE(
      good_extract_2.IsApplicable(context.get(), transformation_context));
  good_extract_2.Apply(context.get(), &transformation_context);
  auto replacement_2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(13, instruction_descriptor_2, 1), 103);
  ASSERT_TRUE(
      replacement_2.IsApplicable(context.get(), transformation_context));
  replacement_2.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %22 with %100[2] in '%23 = OpConvertSToF %16 %22'
  auto instruction_descriptor_3 =
      MakeInstructionDescriptor(23, spv::Op::OpConvertSToF, 0);
  auto good_extract_3 =
      TransformationCompositeExtract(instruction_descriptor_3, 104, 100, {2});
  ASSERT_TRUE(
      good_extract_3.IsApplicable(context.get(), transformation_context));
  good_extract_3.Apply(context.get(), &transformation_context);
  auto replacement_3 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(22, instruction_descriptor_3, 0), 104);
  // Bad: wrong input operand index
  auto bad_replacement_3 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(22, instruction_descriptor_3, 1), 104);
  ASSERT_TRUE(
      replacement_3.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      bad_replacement_3.IsApplicable(context.get(), transformation_context));
  replacement_3.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %28 with %101[0] in 'OpStore %33 %28'
  auto instruction_descriptor_4 =
      MakeInstructionDescriptor(33, spv::Op::OpStore, 0);
  auto good_extract_4 =
      TransformationCompositeExtract(instruction_descriptor_4, 105, 101, {0});
  // Bad: instruction descriptor does not identify an appropriate instruction
  auto bad_extract_4 = TransformationCompositeExtract(
      MakeInstructionDescriptor(33, spv::Op::OpCopyObject, 0), 105, 101, {0});
  ASSERT_TRUE(
      good_extract_4.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      bad_extract_4.IsApplicable(context.get(), transformation_context));
  good_extract_4.Apply(context.get(), &transformation_context);
  auto replacement_4 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(28, instruction_descriptor_4, 1), 105);
  ASSERT_TRUE(
      replacement_4.IsApplicable(context.get(), transformation_context));
  replacement_4.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %23 with %101[1] in '%50 = OpCopyObject %16 %23'
  auto instruction_descriptor_5 =
      MakeInstructionDescriptor(50, spv::Op::OpCopyObject, 0);
  auto good_extract_5 =
      TransformationCompositeExtract(instruction_descriptor_5, 106, 101, {1});
  ASSERT_TRUE(
      good_extract_5.IsApplicable(context.get(), transformation_context));
  good_extract_5.Apply(context.get(), &transformation_context);
  auto replacement_5 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(23, instruction_descriptor_5, 0), 106);
  // Bad: wrong synonym fact being used
  auto bad_replacement_5 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(23, instruction_descriptor_5, 0), 105);
  ASSERT_TRUE(
      replacement_5.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      bad_replacement_5.IsApplicable(context.get(), transformation_context));
  replacement_5.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %32 with %101[2] in 'OpStore %33 %32'
  auto instruction_descriptor_6 =
      MakeInstructionDescriptor(33, spv::Op::OpStore, 1);
  auto good_extract_6 =
      TransformationCompositeExtract(instruction_descriptor_6, 107, 101, {2});
  // Bad: id 1001 does not exist
  auto bad_extract_6 =
      TransformationCompositeExtract(instruction_descriptor_6, 107, 1001, {2});
  ASSERT_TRUE(
      good_extract_6.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      bad_extract_6.IsApplicable(context.get(), transformation_context));
  good_extract_6.Apply(context.get(), &transformation_context);
  auto replacement_6 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(32, instruction_descriptor_6, 1), 107);
  ASSERT_TRUE(
      replacement_6.IsApplicable(context.get(), transformation_context));
  replacement_6.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %23 with %101[3] in '%51 = OpCopyObject %16 %23'
  auto instruction_descriptor_7 =
      MakeInstructionDescriptor(51, spv::Op::OpCopyObject, 0);
  auto good_extract_7 =
      TransformationCompositeExtract(instruction_descriptor_7, 108, 101, {3});
  ASSERT_TRUE(
      good_extract_7.IsApplicable(context.get(), transformation_context));
  good_extract_7.Apply(context.get(), &transformation_context);
  auto replacement_7 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(23, instruction_descriptor_7, 0), 108);
  // Bad: use id 0 is invalid
  auto bad_replacement_7 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(0, instruction_descriptor_7, 0), 108);
  ASSERT_TRUE(
      replacement_7.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      bad_replacement_7.IsApplicable(context.get(), transformation_context));
  replacement_7.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %11 "A"
               OpName %20 "B"
               OpName %31 "g"
               OpName %35 "h"
               OpDecorate %11 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 3
          %9 = OpTypeArray %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 0
         %13 = OpConstant %6 3
         %14 = OpTypePointer Function %6
         %16 = OpTypeFloat 32
         %17 = OpConstant %7 4
         %18 = OpTypeArray %16 %17
         %19 = OpTypePointer Function %18
         %24 = OpTypePointer Function %16
         %28 = OpConstant %16 42
         %30 = OpConstant %6 2
         %34 = OpConstant %6 1
         %38 = OpConstant %6 42
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %20 = OpVariable %19 Function
         %31 = OpVariable %24 Function
         %35 = OpVariable %14 Function
         %15 = OpAccessChain %14 %11 %12
         %21 = OpAccessChain %14 %11 %12
         %22 = OpLoad %6 %21
        %100 = OpCompositeConstruct %9 %12 %13 %22
        %103 = OpCompositeExtract %6 %100 1
               OpStore %15 %103
        %104 = OpCompositeExtract %6 %100 2
         %23 = OpConvertSToF %16 %104
        %102 = OpCompositeExtract %6 %100 0
         %25 = OpAccessChain %24 %20 %102
               OpStore %25 %23
         %26 = OpAccessChain %14 %11 %12
         %27 = OpLoad %6 %26
         %29 = OpAccessChain %24 %20 %27
               OpStore %29 %28
         %32 = OpLoad %16 %31
        %101 = OpCompositeConstruct %18 %28 %23 %32 %23
        %106 = OpCompositeExtract %16 %101 1
         %50 = OpCopyObject %16 %106
        %108 = OpCompositeExtract %16 %101 3
         %51 = OpCopyObject %16 %108
         %33 = OpAccessChain %24 %20 %30
        %105 = OpCompositeExtract %16 %101 0
               OpStore %33 %105
        %107 = OpCompositeExtract %16 %101 2
               OpStore %33 %107
         %36 = OpLoad %6 %35
         %37 = OpAccessChain %14 %11 %34
               OpStore %37 %36
         %39 = OpAccessChain %14 %11 %12
         %40 = OpLoad %6 %39
         %41 = OpIAdd %6 %38 %40
         %42 = OpAccessChain %14 %11 %30
               OpStore %42 %41
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(DataSynonymTransformationTest, MatrixCompositeSynonyms) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "m"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
         %50 = OpUndef %7
          %8 = OpTypeMatrix %7 3
          %9 = OpTypePointer Function %8
         %11 = OpTypeInt 32 1
         %12 = OpConstant %11 0
         %13 = OpConstant %6 1
         %14 = OpConstantComposite %7 %13 %13 %13 %13
         %15 = OpTypePointer Function %7
         %17 = OpConstant %11 1
         %18 = OpConstant %6 2
         %19 = OpConstantComposite %7 %18 %18 %18 %18
         %21 = OpConstant %11 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %10 = OpVariable %9 Function
         %16 = OpAccessChain %15 %10 %12
               OpStore %16 %14
         %20 = OpAccessChain %15 %10 %17
               OpStore %20 %19
         %22 = OpAccessChain %15 %10 %12
         %23 = OpLoad %7 %22
         %24 = OpAccessChain %15 %10 %17
         %25 = OpLoad %7 %24
        %100 = OpCompositeConstruct %8 %23 %25 %50
         %26 = OpFAdd %7 %23 %25
         %27 = OpAccessChain %15 %10 %21
               OpStore %27 %26
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

  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(23, {}, 100, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(25, {}, 100, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(50, {}, 100, {2}));

  // Replace %23 with %100[0] in '%26 = OpFAdd %7 %23 %25'
  auto instruction_descriptor_1 =
      MakeInstructionDescriptor(26, spv::Op::OpFAdd, 0);
  auto extract_1 =
      TransformationCompositeExtract(instruction_descriptor_1, 101, 100, {0});
  ASSERT_TRUE(extract_1.IsApplicable(context.get(), transformation_context));
  extract_1.Apply(context.get(), &transformation_context);
  auto replacement_1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(23, instruction_descriptor_1, 0), 101);
  ASSERT_TRUE(
      replacement_1.IsApplicable(context.get(), transformation_context));
  replacement_1.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %25 with %100[1] in '%26 = OpFAdd %7 %23 %25'
  auto instruction_descriptor_2 =
      MakeInstructionDescriptor(26, spv::Op::OpFAdd, 0);
  auto extract_2 =
      TransformationCompositeExtract(instruction_descriptor_2, 102, 100, {1});
  ASSERT_TRUE(extract_2.IsApplicable(context.get(), transformation_context));
  extract_2.Apply(context.get(), &transformation_context);
  auto replacement_2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(25, instruction_descriptor_2, 1), 102);
  ASSERT_TRUE(
      replacement_2.IsApplicable(context.get(), transformation_context));
  replacement_2.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "m"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
         %50 = OpUndef %7
          %8 = OpTypeMatrix %7 3
          %9 = OpTypePointer Function %8
         %11 = OpTypeInt 32 1
         %12 = OpConstant %11 0
         %13 = OpConstant %6 1
         %14 = OpConstantComposite %7 %13 %13 %13 %13
         %15 = OpTypePointer Function %7
         %17 = OpConstant %11 1
         %18 = OpConstant %6 2
         %19 = OpConstantComposite %7 %18 %18 %18 %18
         %21 = OpConstant %11 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %10 = OpVariable %9 Function
         %16 = OpAccessChain %15 %10 %12
               OpStore %16 %14
         %20 = OpAccessChain %15 %10 %17
               OpStore %20 %19
         %22 = OpAccessChain %15 %10 %12
         %23 = OpLoad %7 %22
         %24 = OpAccessChain %15 %10 %17
         %25 = OpLoad %7 %24
        %100 = OpCompositeConstruct %8 %23 %25 %50
        %101 = OpCompositeExtract %7 %100 0
        %102 = OpCompositeExtract %7 %100 1
         %26 = OpFAdd %7 %101 %102
         %27 = OpAccessChain %15 %10 %21
               OpStore %27 %26
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(DataSynonymTransformationTest, StructCompositeSynonyms) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "Inner"
               OpMemberName %9 0 "a"
               OpMemberName %9 1 "b"
               OpName %11 "i1"
               OpName %17 "i2"
               OpName %31 "Point"
               OpMemberName %31 0 "x"
               OpMemberName %31 1 "y"
               OpMemberName %31 2 "z"
               OpName %32 "Outer"
               OpMemberName %32 0 "c"
               OpMemberName %32 1 "d"
               OpName %34 "o1"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeVector %7 2
          %9 = OpTypeStruct %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 1
         %13 = OpConstant %7 2
         %14 = OpConstant %7 3
         %15 = OpConstantComposite %8 %13 %14
         %16 = OpConstantComposite %9 %12 %15
         %18 = OpConstant %6 0
         %19 = OpTypePointer Function %6
         %24 = OpTypePointer Function %8
         %27 = OpConstant %7 4
         %31 = OpTypeStruct %7 %7 %7
         %32 = OpTypeStruct %9 %31
         %33 = OpTypePointer Function %32
         %36 = OpConstant %7 10
         %37 = OpTypeInt 32 0
         %38 = OpConstant %37 0
         %39 = OpTypePointer Function %7
         %42 = OpConstant %37 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %17 = OpVariable %10 Function
         %34 = OpVariable %33 Function
        %101 = OpCompositeConstruct %31 %27 %36 %27
               OpStore %11 %16
         %20 = OpAccessChain %19 %11 %18
         %21 = OpLoad %6 %20
         %22 = OpIAdd %6 %21 %12
        %102 = OpCompositeConstruct %9 %22 %15
         %23 = OpAccessChain %19 %17 %18
               OpStore %23 %22
         %25 = OpAccessChain %24 %17 %12
         %26 = OpLoad %8 %25
         %28 = OpCompositeConstruct %8 %27 %27
         %29 = OpFAdd %8 %26 %28
         %30 = OpAccessChain %24 %17 %12
               OpStore %30 %29
         %35 = OpLoad %9 %11
         %40 = OpAccessChain %39 %11 %12 %38
         %41 = OpLoad %7 %40
         %43 = OpAccessChain %39 %11 %12 %42
         %44 = OpLoad %7 %43
         %45 = OpCompositeConstruct %31 %36 %41 %44
        %100 = OpCompositeConstruct %32 %16 %45
         %46 = OpCompositeConstruct %32 %35 %45
               OpStore %34 %46
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

  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(16, {}, 100, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(45, {}, 100, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(27, {}, 101, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(36, {}, 101, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(27, {}, 101, {2}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(22, {}, 102, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(15, {}, 102, {1}));

  // Replace %45 with %100[1] in '%46 = OpCompositeConstruct %32 %35 %45'
  auto instruction_descriptor_1 =
      MakeInstructionDescriptor(46, spv::Op::OpCompositeConstruct, 0);
  auto extract_1 =
      TransformationCompositeExtract(instruction_descriptor_1, 201, 100, {1});
  ASSERT_TRUE(extract_1.IsApplicable(context.get(), transformation_context));
  extract_1.Apply(context.get(), &transformation_context);
  auto replacement_1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(45, instruction_descriptor_1, 1), 201);
  ASSERT_TRUE(
      replacement_1.IsApplicable(context.get(), transformation_context));
  replacement_1.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace second occurrence of %27 with %101[0] in '%28 =
  // OpCompositeConstruct %8 %27 %27'
  auto instruction_descriptor_2 =
      MakeInstructionDescriptor(28, spv::Op::OpCompositeConstruct, 0);
  auto extract_2 =
      TransformationCompositeExtract(instruction_descriptor_2, 202, 101, {0});
  ASSERT_TRUE(extract_2.IsApplicable(context.get(), transformation_context));
  extract_2.Apply(context.get(), &transformation_context);
  auto replacement_2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(27, instruction_descriptor_2, 1), 202);
  ASSERT_TRUE(
      replacement_2.IsApplicable(context.get(), transformation_context));
  replacement_2.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %36 with %101[1] in '%45 = OpCompositeConstruct %31 %36 %41 %44'
  auto instruction_descriptor_3 =
      MakeInstructionDescriptor(45, spv::Op::OpCompositeConstruct, 0);
  auto extract_3 =
      TransformationCompositeExtract(instruction_descriptor_3, 203, 101, {1});
  ASSERT_TRUE(extract_3.IsApplicable(context.get(), transformation_context));
  extract_3.Apply(context.get(), &transformation_context);
  auto replacement_3 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(36, instruction_descriptor_3, 0), 203);
  ASSERT_TRUE(
      replacement_3.IsApplicable(context.get(), transformation_context));
  replacement_3.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace first occurrence of %27 with %101[2] in '%28 = OpCompositeConstruct
  // %8 %27 %27'
  auto instruction_descriptor_4 =
      MakeInstructionDescriptor(28, spv::Op::OpCompositeConstruct, 0);
  auto extract_4 =
      TransformationCompositeExtract(instruction_descriptor_4, 204, 101, {2});
  ASSERT_TRUE(extract_4.IsApplicable(context.get(), transformation_context));
  extract_4.Apply(context.get(), &transformation_context);
  auto replacement_4 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(27, instruction_descriptor_4, 0), 204);
  ASSERT_TRUE(
      replacement_4.IsApplicable(context.get(), transformation_context));
  replacement_4.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %22 with %102[0] in 'OpStore %23 %22'
  auto instruction_descriptor_5 =
      MakeInstructionDescriptor(23, spv::Op::OpStore, 0);
  auto extract_5 =
      TransformationCompositeExtract(instruction_descriptor_5, 205, 102, {0});
  ASSERT_TRUE(extract_5.IsApplicable(context.get(), transformation_context));
  extract_5.Apply(context.get(), &transformation_context);
  auto replacement_5 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(22, instruction_descriptor_5, 1), 205);
  ASSERT_TRUE(
      replacement_5.IsApplicable(context.get(), transformation_context));
  replacement_5.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "Inner"
               OpMemberName %9 0 "a"
               OpMemberName %9 1 "b"
               OpName %11 "i1"
               OpName %17 "i2"
               OpName %31 "Point"
               OpMemberName %31 0 "x"
               OpMemberName %31 1 "y"
               OpMemberName %31 2 "z"
               OpName %32 "Outer"
               OpMemberName %32 0 "c"
               OpMemberName %32 1 "d"
               OpName %34 "o1"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeVector %7 2
          %9 = OpTypeStruct %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 1
         %13 = OpConstant %7 2
         %14 = OpConstant %7 3
         %15 = OpConstantComposite %8 %13 %14
         %16 = OpConstantComposite %9 %12 %15
         %18 = OpConstant %6 0
         %19 = OpTypePointer Function %6
         %24 = OpTypePointer Function %8
         %27 = OpConstant %7 4
         %31 = OpTypeStruct %7 %7 %7
         %32 = OpTypeStruct %9 %31
         %33 = OpTypePointer Function %32
         %36 = OpConstant %7 10
         %37 = OpTypeInt 32 0
         %38 = OpConstant %37 0
         %39 = OpTypePointer Function %7
         %42 = OpConstant %37 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %17 = OpVariable %10 Function
         %34 = OpVariable %33 Function
        %101 = OpCompositeConstruct %31 %27 %36 %27
               OpStore %11 %16
         %20 = OpAccessChain %19 %11 %18
         %21 = OpLoad %6 %20
         %22 = OpIAdd %6 %21 %12
        %102 = OpCompositeConstruct %9 %22 %15
         %23 = OpAccessChain %19 %17 %18
        %205 = OpCompositeExtract %6 %102 0
               OpStore %23 %205
         %25 = OpAccessChain %24 %17 %12
         %26 = OpLoad %8 %25
        %202 = OpCompositeExtract %7 %101 0
        %204 = OpCompositeExtract %7 %101 2
         %28 = OpCompositeConstruct %8 %204 %202
         %29 = OpFAdd %8 %26 %28
         %30 = OpAccessChain %24 %17 %12
               OpStore %30 %29
         %35 = OpLoad %9 %11
         %40 = OpAccessChain %39 %11 %12 %38
         %41 = OpLoad %7 %40
         %43 = OpAccessChain %39 %11 %12 %42
         %44 = OpLoad %7 %43
        %203 = OpCompositeExtract %7 %101 1
         %45 = OpCompositeConstruct %31 %203 %41 %44
        %100 = OpCompositeConstruct %32 %16 %45
        %201 = OpCompositeExtract %31 %100 1
         %46 = OpCompositeConstruct %32 %35 %201
               OpStore %34 %46
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(DataSynonymTransformationTest, VectorCompositeSynonyms) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "f"
               OpName %12 "v2"
               OpName %18 "v3"
               OpName %23 "v4"
               OpName %32 "b"
               OpName %36 "bv2"
               OpName %41 "bv3"
               OpName %50 "bv4"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 42
         %10 = OpTypeVector %6 2
         %11 = OpTypePointer Function %10
         %16 = OpTypeVector %6 3
         %17 = OpTypePointer Function %16
         %21 = OpTypeVector %6 4
         %22 = OpTypePointer Function %21
         %30 = OpTypeBool
         %31 = OpTypePointer Function %30
         %33 = OpConstantFalse %30
         %34 = OpTypeVector %30 2
         %35 = OpTypePointer Function %34
         %37 = OpConstantTrue %30
         %38 = OpConstantComposite %34 %37 %37
         %39 = OpTypeVector %30 3
         %40 = OpTypePointer Function %39
         %48 = OpTypeVector %30 4
         %49 = OpTypePointer Function %48
         %51 = OpTypeInt 32 0
         %52 = OpConstant %51 2
         %55 = OpConstant %6 0
         %57 = OpConstant %51 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %18 = OpVariable %17 Function
         %23 = OpVariable %22 Function
         %32 = OpVariable %31 Function
         %36 = OpVariable %35 Function
         %41 = OpVariable %40 Function
         %50 = OpVariable %49 Function
               OpStore %8 %9
         %13 = OpLoad %6 %8
         %14 = OpLoad %6 %8
         %15 = OpCompositeConstruct %10 %13 %14
               OpStore %12 %15
         %19 = OpLoad %10 %12
         %20 = OpVectorShuffle %16 %19 %19 0 0 1
               OpStore %18 %20
         %24 = OpLoad %16 %18
         %25 = OpLoad %6 %8
         %26 = OpCompositeExtract %6 %24 0
         %27 = OpCompositeExtract %6 %24 1
         %28 = OpCompositeExtract %6 %24 2
         %29 = OpCompositeConstruct %21 %26 %27 %28 %25
               OpStore %23 %29
               OpStore %32 %33
               OpStore %36 %38
         %42 = OpLoad %30 %32
         %43 = OpLoad %34 %36
         %44 = OpVectorShuffle %34 %43 %43 0 0
         %45 = OpCompositeExtract %30 %44 0
         %46 = OpCompositeExtract %30 %44 1
         %47 = OpCompositeConstruct %39 %42 %45 %46
               OpStore %41 %47
         %53 = OpAccessChain %7 %23 %52
         %54 = OpLoad %6 %53

        %100 = OpCompositeConstruct %21 %20 %54
        %101 = OpCompositeConstruct %21 %15 %19
        %102 = OpCompositeConstruct %16 %27 %15
        %103 = OpCompositeConstruct %48 %33 %47
        %104 = OpCompositeConstruct %34 %42 %45
        %105 = OpCompositeConstruct %39 %38 %46

         %86 = OpCopyObject %30 %33
         %56 = OpFOrdNotEqual %30 %54 %55
         %80 = OpCopyObject %16 %20
         %58 = OpAccessChain %7 %18 %57
         %59 = OpLoad %6 %58
         %60 = OpFOrdNotEqual %30 %59 %55
         %61 = OpLoad %34 %36
         %62 = OpLogicalAnd %30 %45 %46
         %63 = OpLogicalOr %30 %45 %46
         %64 = OpCompositeConstruct %48 %56 %60 %62 %63
               OpStore %12 %15
         %81 = OpVectorShuffle %16 %19 %19 0 0 1
         %82 = OpCompositeConstruct %21 %26 %27 %28 %25
         %83 = OpCopyObject %10 %15
         %84 = OpCopyObject %39 %47
               OpStore %50 %64
         %85 = OpCopyObject %30 %42
               OpStore %36 %38
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

  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(20, {0}, 100, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(20, {1}, 100, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(20, {2}, 100, {2}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(54, {}, 100, {3}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(15, {0}, 101, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(15, {1}, 101, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(19, {0}, 101, {2}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(19, {1}, 101, {3}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(27, {}, 102, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(15, {0}, 102, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(15, {1}, 102, {2}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(33, {}, 103, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(47, {0}, 103, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(47, {1}, 103, {2}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(47, {2}, 103, {3}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(42, {}, 104, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(45, {}, 104, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(38, {0}, 105, {0}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(38, {1}, 105, {1}));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(46, {}, 105, {2}));

  // Replace %20 with %100[0:2] in '%80 = OpCopyObject %16 %20'
  auto instruction_descriptor_1 =
      MakeInstructionDescriptor(80, spv::Op::OpCopyObject, 0);
  auto shuffle_1 = TransformationVectorShuffle(instruction_descriptor_1, 200,
                                               100, 100, {0, 1, 2});
  ASSERT_TRUE(shuffle_1.IsApplicable(context.get(), transformation_context));
  shuffle_1.Apply(context.get(), &transformation_context);
  transformation_context.GetFactManager()->ComputeClosureOfFacts(100);

  auto replacement_1 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(20, instruction_descriptor_1, 0), 200);
  ASSERT_TRUE(
      replacement_1.IsApplicable(context.get(), transformation_context));
  replacement_1.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %54 with %100[3] in '%56 = OpFOrdNotEqual %30 %54 %55'
  auto instruction_descriptor_2 =
      MakeInstructionDescriptor(56, spv::Op::OpFOrdNotEqual, 0);
  auto extract_2 =
      TransformationCompositeExtract(instruction_descriptor_2, 201, 100, {3});

  ASSERT_TRUE(extract_2.IsApplicable(context.get(), transformation_context));
  extract_2.Apply(context.get(), &transformation_context);
  auto replacement_2 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(54, instruction_descriptor_2, 0), 201);
  ASSERT_TRUE(
      replacement_2.IsApplicable(context.get(), transformation_context));
  replacement_2.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %15 with %101[0:1] in 'OpStore %12 %15'
  auto instruction_descriptor_3 =
      MakeInstructionDescriptor(64, spv::Op::OpStore, 0);
  auto shuffle_3 = TransformationVectorShuffle(instruction_descriptor_3, 202,
                                               101, 101, {0, 1});
  ASSERT_TRUE(shuffle_3.IsApplicable(context.get(), transformation_context));
  shuffle_3.Apply(context.get(), &transformation_context);
  transformation_context.GetFactManager()->ComputeClosureOfFacts(100);

  auto replacement_3 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(15, instruction_descriptor_3, 1), 202);
  ASSERT_TRUE(
      replacement_3.IsApplicable(context.get(), transformation_context));
  replacement_3.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %19 with %101[2:3] in '%81 = OpVectorShuffle %16 %19 %19 0 0 1'
  auto instruction_descriptor_4 =
      MakeInstructionDescriptor(81, spv::Op::OpVectorShuffle, 0);
  auto shuffle_4 = TransformationVectorShuffle(instruction_descriptor_4, 203,
                                               101, 101, {2, 3});
  ASSERT_TRUE(shuffle_4.IsApplicable(context.get(), transformation_context));
  shuffle_4.Apply(context.get(), &transformation_context);
  transformation_context.GetFactManager()->ComputeClosureOfFacts(100);

  auto replacement_4 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(19, instruction_descriptor_4, 0), 203);
  ASSERT_TRUE(
      replacement_4.IsApplicable(context.get(), transformation_context));
  replacement_4.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %27 with %102[0] in '%82 = OpCompositeConstruct %21 %26 %27 %28
  // %25'
  auto instruction_descriptor_5 =
      MakeInstructionDescriptor(82, spv::Op::OpCompositeConstruct, 0);
  auto extract_5 =
      TransformationCompositeExtract(instruction_descriptor_5, 204, 102, {0});

  ASSERT_TRUE(extract_5.IsApplicable(context.get(), transformation_context));
  extract_5.Apply(context.get(), &transformation_context);
  auto replacement_5 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(27, instruction_descriptor_5, 1), 204);
  ASSERT_TRUE(
      replacement_5.IsApplicable(context.get(), transformation_context));
  replacement_5.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %15 with %102[1:2] in '%83 = OpCopyObject %10 %15'
  auto instruction_descriptor_6 =
      MakeInstructionDescriptor(83, spv::Op::OpCopyObject, 0);
  auto shuffle_6 = TransformationVectorShuffle(instruction_descriptor_6, 205,
                                               102, 102, {1, 2});
  ASSERT_TRUE(shuffle_6.IsApplicable(context.get(), transformation_context));
  shuffle_6.Apply(context.get(), &transformation_context);
  transformation_context.GetFactManager()->ComputeClosureOfFacts(100);

  auto replacement_6 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(15, instruction_descriptor_6, 0), 205);
  ASSERT_TRUE(
      replacement_6.IsApplicable(context.get(), transformation_context));
  replacement_6.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %33 with %103[0] in '%86 = OpCopyObject %30 %33'
  auto instruction_descriptor_7 =
      MakeInstructionDescriptor(86, spv::Op::OpCopyObject, 0);
  auto extract_7 =
      TransformationCompositeExtract(instruction_descriptor_7, 206, 103, {0});
  ASSERT_TRUE(extract_7.IsApplicable(context.get(), transformation_context));
  extract_7.Apply(context.get(), &transformation_context);
  auto replacement_7 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(33, instruction_descriptor_7, 0), 206);
  ASSERT_TRUE(
      replacement_7.IsApplicable(context.get(), transformation_context));
  replacement_7.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %47 with %103[1:3] in '%84 = OpCopyObject %39 %47'
  auto instruction_descriptor_8 =
      MakeInstructionDescriptor(84, spv::Op::OpCopyObject, 0);
  auto shuffle_8 = TransformationVectorShuffle(instruction_descriptor_8, 207,
                                               103, 103, {1, 2, 3});
  ASSERT_TRUE(shuffle_8.IsApplicable(context.get(), transformation_context));
  shuffle_8.Apply(context.get(), &transformation_context);
  transformation_context.GetFactManager()->ComputeClosureOfFacts(100);

  auto replacement_8 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(47, instruction_descriptor_8, 0), 207);
  ASSERT_TRUE(
      replacement_8.IsApplicable(context.get(), transformation_context));
  replacement_8.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %42 with %104[0] in '%85 = OpCopyObject %30 %42'
  auto instruction_descriptor_9 =
      MakeInstructionDescriptor(85, spv::Op::OpCopyObject, 0);
  auto extract_9 =
      TransformationCompositeExtract(instruction_descriptor_9, 208, 104, {0});
  ASSERT_TRUE(extract_9.IsApplicable(context.get(), transformation_context));
  extract_9.Apply(context.get(), &transformation_context);
  auto replacement_9 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(42, instruction_descriptor_9, 0), 208);
  ASSERT_TRUE(
      replacement_9.IsApplicable(context.get(), transformation_context));
  replacement_9.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %45 with %104[1] in '%63 = OpLogicalOr %30 %45 %46'
  auto instruction_descriptor_10 =
      MakeInstructionDescriptor(63, spv::Op::OpLogicalOr, 0);
  auto extract_10 =
      TransformationCompositeExtract(instruction_descriptor_10, 209, 104, {1});
  ASSERT_TRUE(extract_10.IsApplicable(context.get(), transformation_context));
  extract_10.Apply(context.get(), &transformation_context);
  auto replacement_10 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(45, instruction_descriptor_10, 0), 209);
  ASSERT_TRUE(
      replacement_10.IsApplicable(context.get(), transformation_context));
  replacement_10.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %38 with %105[0:1] in 'OpStore %36 %38'
  auto instruction_descriptor_11 =
      MakeInstructionDescriptor(85, spv::Op::OpStore, 0);
  auto shuffle_11 = TransformationVectorShuffle(instruction_descriptor_11, 210,
                                                105, 105, {0, 1});
  ASSERT_TRUE(shuffle_11.IsApplicable(context.get(), transformation_context));
  shuffle_11.Apply(context.get(), &transformation_context);
  transformation_context.GetFactManager()->ComputeClosureOfFacts(100);

  auto replacement_11 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(38, instruction_descriptor_11, 1), 210);
  ASSERT_TRUE(
      replacement_11.IsApplicable(context.get(), transformation_context));
  replacement_11.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Replace %46 with %105[2] in '%62 = OpLogicalAnd %30 %45 %46'
  auto instruction_descriptor_12 =
      MakeInstructionDescriptor(62, spv::Op::OpLogicalAnd, 0);
  auto extract_12 =
      TransformationCompositeExtract(instruction_descriptor_12, 211, 105, {2});
  ASSERT_TRUE(extract_12.IsApplicable(context.get(), transformation_context));
  extract_12.Apply(context.get(), &transformation_context);
  auto replacement_12 = TransformationReplaceIdWithSynonym(
      MakeIdUseDescriptor(46, instruction_descriptor_12, 1), 211);
  ASSERT_TRUE(
      replacement_12.IsApplicable(context.get(), transformation_context));
  replacement_12.Apply(context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "f"
               OpName %12 "v2"
               OpName %18 "v3"
               OpName %23 "v4"
               OpName %32 "b"
               OpName %36 "bv2"
               OpName %41 "bv3"
               OpName %50 "bv4"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 42
         %10 = OpTypeVector %6 2
         %11 = OpTypePointer Function %10
         %16 = OpTypeVector %6 3
         %17 = OpTypePointer Function %16
         %21 = OpTypeVector %6 4
         %22 = OpTypePointer Function %21
         %30 = OpTypeBool
         %31 = OpTypePointer Function %30
         %33 = OpConstantFalse %30
         %34 = OpTypeVector %30 2
         %35 = OpTypePointer Function %34
         %37 = OpConstantTrue %30
         %38 = OpConstantComposite %34 %37 %37
         %39 = OpTypeVector %30 3
         %40 = OpTypePointer Function %39
         %48 = OpTypeVector %30 4
         %49 = OpTypePointer Function %48
         %51 = OpTypeInt 32 0
         %52 = OpConstant %51 2
         %55 = OpConstant %6 0
         %57 = OpConstant %51 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %18 = OpVariable %17 Function
         %23 = OpVariable %22 Function
         %32 = OpVariable %31 Function
         %36 = OpVariable %35 Function
         %41 = OpVariable %40 Function
         %50 = OpVariable %49 Function
               OpStore %8 %9
         %13 = OpLoad %6 %8
         %14 = OpLoad %6 %8
         %15 = OpCompositeConstruct %10 %13 %14
               OpStore %12 %15
         %19 = OpLoad %10 %12
         %20 = OpVectorShuffle %16 %19 %19 0 0 1
               OpStore %18 %20
         %24 = OpLoad %16 %18
         %25 = OpLoad %6 %8
         %26 = OpCompositeExtract %6 %24 0
         %27 = OpCompositeExtract %6 %24 1
         %28 = OpCompositeExtract %6 %24 2
         %29 = OpCompositeConstruct %21 %26 %27 %28 %25
               OpStore %23 %29
               OpStore %32 %33
               OpStore %36 %38
         %42 = OpLoad %30 %32
         %43 = OpLoad %34 %36
         %44 = OpVectorShuffle %34 %43 %43 0 0
         %45 = OpCompositeExtract %30 %44 0
         %46 = OpCompositeExtract %30 %44 1
         %47 = OpCompositeConstruct %39 %42 %45 %46
               OpStore %41 %47
         %53 = OpAccessChain %7 %23 %52
         %54 = OpLoad %6 %53

        %100 = OpCompositeConstruct %21 %20 %54
        %101 = OpCompositeConstruct %21 %15 %19
        %102 = OpCompositeConstruct %16 %27 %15
        %103 = OpCompositeConstruct %48 %33 %47
        %104 = OpCompositeConstruct %34 %42 %45
        %105 = OpCompositeConstruct %39 %38 %46

        %206 = OpCompositeExtract %30 %103 0
         %86 = OpCopyObject %30 %206
        %201 = OpCompositeExtract %6 %100 3
         %56 = OpFOrdNotEqual %30 %201 %55
        %200 = OpVectorShuffle %16 %100 %100 0 1 2
         %80 = OpCopyObject %16 %200
         %58 = OpAccessChain %7 %18 %57
         %59 = OpLoad %6 %58
         %60 = OpFOrdNotEqual %30 %59 %55
         %61 = OpLoad %34 %36
        %211 = OpCompositeExtract %30 %105 2
         %62 = OpLogicalAnd %30 %45 %211
        %209 = OpCompositeExtract %30 %104 1
         %63 = OpLogicalOr %30 %209 %46
         %64 = OpCompositeConstruct %48 %56 %60 %62 %63
        %202 = OpVectorShuffle %10 %101 %101 0 1
               OpStore %12 %202
        %203 = OpVectorShuffle %10 %101 %101 2 3
         %81 = OpVectorShuffle %16 %203 %19 0 0 1
        %204 = OpCompositeExtract %6 %102 0
         %82 = OpCompositeConstruct %21 %26 %204 %28 %25
        %205 = OpVectorShuffle %10 %102 %102 1 2
         %83 = OpCopyObject %10 %205
        %207 = OpVectorShuffle %39 %103 %103 1 2 3
         %84 = OpCopyObject %39 %207
               OpStore %50 %64
        %208 = OpCompositeExtract %30 %104 0
         %85 = OpCopyObject %30 %208
        %210 = OpVectorShuffle %34 %105 %105 0 1
               OpStore %36 %210
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
