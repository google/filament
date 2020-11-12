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

#include "source/fuzz/transformation_add_opphi_synonym.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

protobufs::Fact MakeSynonymFact(uint32_t first, uint32_t second) {
  protobufs::FactDataSynonym data_synonym_fact;
  *data_synonym_fact.mutable_data1() = MakeDataDescriptor(first, {});
  *data_synonym_fact.mutable_data2() = MakeDataDescriptor(second, {});
  protobufs::Fact result;
  *result.mutable_data_synonym_fact() = data_synonym_fact;
  return result;
}

// Adds synonym facts to the fact manager.
void SetUpIdSynonyms(FactManager* fact_manager) {
  fact_manager->MaybeAddFact(MakeSynonymFact(11, 9));
  fact_manager->MaybeAddFact(MakeSynonymFact(13, 9));
  fact_manager->MaybeAddFact(MakeSynonymFact(14, 9));
  fact_manager->MaybeAddFact(MakeSynonymFact(19, 9));
  fact_manager->MaybeAddFact(MakeSynonymFact(20, 9));
  fact_manager->MaybeAddFact(MakeSynonymFact(10, 21));
}

TEST(TransformationAddOpPhiSynonymTest, Inapplicable) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpTypeInt 32 0
         %22 = OpTypePointer Function %7
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %8 1
          %2 = OpFunction %3 None %4
         %12 = OpLabel
         %23 = OpVariable %22 Function
         %13 = OpCopyObject %7 %9
         %14 = OpCopyObject %8 %11
               OpBranch %15
         %15 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %6 %17 %18
         %17 = OpLabel
         %19 = OpCopyObject %7 %13
         %20 = OpCopyObject %8 %14
         %21 = OpCopyObject %7 %10
               OpBranch %16
         %18 = OpLabel
         %24 = OpCopyObject %22 %23
               OpBranch %16
         %16 = OpLabel
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
  SetUpIdSynonyms(transformation_context.GetFactManager());
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(23, 24));

  // %13 is not a block label.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(13, {{}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // Block %12 does not have a predecessor.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(12, {{}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // Not all predecessors of %16 (%17 and %18) are considered in the map.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(16, {{{17, 19}}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // %30 does not exist in the module.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(16, {{{30, 19}}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // %20 is not a block label.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(16, {{{20, 19}}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // %15 is not the id of one of the predecessors of the block.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(16, {{{15, 19}}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // %30 does not exist in the module.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(16, {{{17, 30}, {18, 13}}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // %19 and %10 are not synonymous.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(16, {{{17, 19}, {18, 10}}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // %19 and %14 do not have the same type.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(16, {{{17, 19}, {18, 14}}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // %19 is not available at the end of %18.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(16, {{{17, 9}, {18, 19}}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // %21 is not a fresh id.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(16, {{{17, 9}, {18, 9}}}, 21)
                   .IsApplicable(context.get(), transformation_context));

  // %23 and %24 have pointer id.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(16, {{{17, 23}, {18, 24}}}, 100)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddOpPhiSynonymTest, Apply) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpTypeInt 32 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %8 1
          %2 = OpFunction %3 None %4
         %12 = OpLabel
         %13 = OpCopyObject %7 %9
         %14 = OpCopyObject %8 %11
               OpBranch %15
         %15 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %6 %17 %18
         %17 = OpLabel
         %19 = OpCopyObject %7 %13
         %20 = OpCopyObject %8 %14
         %21 = OpCopyObject %7 %10
               OpBranch %16
         %18 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpLoopMerge %23 %24 None
               OpBranchConditional %6 %25 %23
         %25 = OpLabel
               OpSelectionMerge %26 None
               OpBranchConditional %6 %27 %26
         %27 = OpLabel
         %28 = OpCopyObject %7 %13
               OpBranch %23
         %26 = OpLabel
               OpSelectionMerge %29 None
               OpBranchConditional %6 %29 %24
         %29 = OpLabel
         %30 = OpCopyObject %7 %13
               OpBranch %23
         %24 = OpLabel
               OpBranch %22
         %23 = OpLabel
               OpSelectionMerge %31 None
               OpBranchConditional %6 %31 %31
         %31 = OpLabel
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
  SetUpIdSynonyms(transformation_context.GetFactManager());

  // Add some further synonym facts.
  transformation_context.GetFactManager()->MaybeAddFact(MakeSynonymFact(28, 9));
  transformation_context.GetFactManager()->MaybeAddFact(MakeSynonymFact(30, 9));

  auto transformation1 = TransformationAddOpPhiSynonym(17, {{{15, 13}}}, 100);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {}), MakeDataDescriptor(9, {})));

  auto transformation2 =
      TransformationAddOpPhiSynonym(16, {{{17, 19}, {18, 13}}}, 101);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(101, {}), MakeDataDescriptor(9, {})));

  auto transformation3 =
      TransformationAddOpPhiSynonym(23, {{{22, 13}, {27, 28}, {29, 30}}}, 102);
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(102, {}), MakeDataDescriptor(9, {})));

  auto transformation4 = TransformationAddOpPhiSynonym(31, {{{23, 13}}}, 103);
  ASSERT_TRUE(
      transformation4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation4, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(103, {}), MakeDataDescriptor(9, {})));

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpTypeInt 32 1
          %8 = OpTypeInt 32 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %8 1
          %2 = OpFunction %3 None %4
         %12 = OpLabel
         %13 = OpCopyObject %7 %9
         %14 = OpCopyObject %8 %11
               OpBranch %15
         %15 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %6 %17 %18
         %17 = OpLabel
        %100 = OpPhi %7 %13 %15
         %19 = OpCopyObject %7 %13
         %20 = OpCopyObject %8 %14
         %21 = OpCopyObject %7 %10
               OpBranch %16
         %18 = OpLabel
               OpBranch %16
         %16 = OpLabel
        %101 = OpPhi %7 %19 %17 %13 %18
               OpBranch %22
         %22 = OpLabel
               OpLoopMerge %23 %24 None
               OpBranchConditional %6 %25 %23
         %25 = OpLabel
               OpSelectionMerge %26 None
               OpBranchConditional %6 %27 %26
         %27 = OpLabel
         %28 = OpCopyObject %7 %13
               OpBranch %23
         %26 = OpLabel
               OpSelectionMerge %29 None
               OpBranchConditional %6 %29 %24
         %29 = OpLabel
         %30 = OpCopyObject %7 %13
               OpBranch %23
         %24 = OpLabel
               OpBranch %22
         %23 = OpLabel
        %102 = OpPhi %7 %13 %22 %28 %27 %30 %29
               OpSelectionMerge %31 None
               OpBranchConditional %6 %31 %31
         %31 = OpLabel
        %103 = OpPhi %7 %13 %23
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationAddOpPhiSynonymTest, VariablePointers) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main" %3
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %4 = OpTypeVoid
          %5 = OpTypeFunction %4
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %10 = OpTypePointer Workgroup %8
          %3 = OpVariable %10 Workgroup
          %2 = OpFunction %4 None %5
         %11 = OpLabel
         %12 = OpVariable %9 Function
               OpSelectionMerge %13 None
               OpBranchConditional %7 %14 %13
         %14 = OpLabel
         %15 = OpCopyObject %10 %3
         %16 = OpCopyObject %9 %12
               OpBranch %13
         %13 = OpLabel
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
  // Declare synonyms
  transformation_context.GetFactManager()->MaybeAddFact(MakeSynonymFact(3, 15));
  transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(12, 16));

  // Remove the VariablePointers capability.
  context.get()->get_feature_mgr()->RemoveCapability(
      SpvCapabilityVariablePointers);

  // The VariablePointers capability is required to add an OpPhi instruction of
  // pointer type.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(13, {{{11, 3}, {14, 15}}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  // Add the VariablePointers capability back.
  context.get()->get_feature_mgr()->AddCapability(
      SpvCapabilityVariablePointers);

  // If the ids have pointer type, the storage class must be Workgroup or
  // StorageBuffer, but it is Function in this case.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(13, {{{11, 12}, {14, 16}}}, 100)
                   .IsApplicable(context.get(), transformation_context));

  auto transformation =
      TransformationAddOpPhiSynonym(13, {{{11, 3}, {14, 15}}}, 100);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string after_transformation = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main" %3
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %4 = OpTypeVoid
          %5 = OpTypeFunction %4
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %10 = OpTypePointer Workgroup %8
          %3 = OpVariable %10 Workgroup
          %2 = OpFunction %4 None %5
         %11 = OpLabel
         %12 = OpVariable %9 Function
               OpSelectionMerge %13 None
               OpBranchConditional %7 %14 %13
         %14 = OpLabel
         %15 = OpCopyObject %10 %3
         %16 = OpCopyObject %9 %12
               OpBranch %13
         %13 = OpLabel
        %100 = OpPhi %10 %3 %11 %15 %14
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddOpPhiSynonymTest, DeadBlock) {
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
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
         %15 = OpConstant %6 0
         %50 = OpConstant %6 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpSelectionMerge %13 None
               OpBranchConditional %11 %12 %13
         %12 = OpLabel
         %14 = OpLoad %6 %8
         %16 = OpIEqual %10 %14 %15
               OpSelectionMerge %18 None
               OpBranchConditional %16 %17 %40
         %17 = OpLabel
               OpBranch %18
         %40 = OpLabel
               OpBranch %18
         %18 = OpLabel
               OpBranch %13
         %13 = OpLabel
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
  // Dead blocks
  transformation_context.GetFactManager()->AddFactBlockIsDead(12);
  transformation_context.GetFactManager()->AddFactBlockIsDead(17);
  transformation_context.GetFactManager()->AddFactBlockIsDead(18);

  // Declare synonym
  ASSERT_TRUE(transformation_context.GetFactManager()->MaybeAddFact(
      MakeSynonymFact(15, 50)));

  // Bad because the block 18 is dead.
  ASSERT_FALSE(TransformationAddOpPhiSynonym(18, {{{17, 15}, {40, 50}}}, 100)
                   .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
