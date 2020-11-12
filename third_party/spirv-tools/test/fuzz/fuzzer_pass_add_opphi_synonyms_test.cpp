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

#include "source/fuzz/fuzzer_pass_add_opphi_synonyms.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/pseudo_random_generator.h"
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
  // Synonyms {9, 11, 15, 16, 21, 22}
  fact_manager->MaybeAddFact(MakeSynonymFact(11, 9));
  fact_manager->MaybeAddFact(MakeSynonymFact(15, 9));
  fact_manager->MaybeAddFact(MakeSynonymFact(16, 9));
  fact_manager->MaybeAddFact(MakeSynonymFact(21, 9));
  fact_manager->MaybeAddFact(MakeSynonymFact(22, 9));

  // Synonyms {10, 23}
  fact_manager->MaybeAddFact(MakeSynonymFact(10, 23));

  // Synonyms {14, 27}
  fact_manager->MaybeAddFact(MakeSynonymFact(14, 27));

  // Synonyms {24, 26, 30}
  fact_manager->MaybeAddFact(MakeSynonymFact(26, 24));
  fact_manager->MaybeAddFact(MakeSynonymFact(30, 24));
}

// Returns true if the given lists have the same elements, regardless of their
// order.
template <typename T>
bool ListsHaveTheSameElements(const std::vector<T>& list1,
                              const std::vector<T>& list2) {
  auto sorted1 = list1;
  std::sort(sorted1.begin(), sorted1.end());

  auto sorted2 = list2;
  std::sort(sorted2.begin(), sorted2.end());

  return sorted1 == sorted2;
}

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
         %31 = OpTypeFunction %7
          %8 = OpTypeInt 32 0
          %9 = OpConstant %7 1
         %10 = OpConstant %7 2
         %11 = OpConstant %8 1
         %12 = OpTypePointer Function %7
          %2 = OpFunction %3 None %4
         %13 = OpLabel
         %14 = OpVariable %12 Function
         %15 = OpCopyObject %7 %9
         %16 = OpCopyObject %8 %11
               OpBranch %17
         %17 = OpLabel
               OpSelectionMerge %18 None
               OpBranchConditional %6 %19 %20
         %19 = OpLabel
         %21 = OpCopyObject %7 %15
         %22 = OpCopyObject %8 %16
         %23 = OpCopyObject %7 %10
         %24 = OpIAdd %7 %9 %10
               OpBranch %18
         %20 = OpLabel
               OpBranch %18
         %18 = OpLabel
         %26 = OpIAdd %7 %15 %10
         %27 = OpCopyObject %12 %14
               OpSelectionMerge %28 None
               OpBranchConditional %6 %29 %28
         %29 = OpLabel
         %30 = OpCopyObject %7 %26
               OpBranch %28
         %28 = OpLabel
               OpReturn
               OpFunctionEnd
         %32 = OpFunction %7 None %31
         %33 = OpLabel
               OpReturnValue %9
               OpFunctionEnd
)";

TEST(FuzzerPassAddOpPhiSynonymsTest, HelperFunctions) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  PseudoRandomGenerator prng(0);
  FuzzerContext fuzzer_context(&prng, 100);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassAddOpPhiSynonyms fuzzer_pass(context.get(), &transformation_context,
                                         &fuzzer_context,
                                         &transformation_sequence);

  SetUpIdSynonyms(transformation_context.GetFactManager());

  std::vector<std::set<uint32_t>> expected_equivalence_classes = {
      {9, 15, 21}, {11, 16, 22}, {10, 23}, {6}, {24, 26, 30}};

  ASSERT_TRUE(ListsHaveTheSameElements<std::set<uint32_t>>(
      fuzzer_pass.GetIdEquivalenceClasses(), expected_equivalence_classes));

  // The set {24, 26, 30} is not suitable for 18 (none if the ids is available
  // for predecessor 20).
  ASSERT_FALSE(
      fuzzer_pass.EquivalenceClassIsSuitableForBlock({24, 26, 30}, 18, 1));

  // The set {6} is not suitable for 18 if we require at least 2 distinct
  // available ids.
  ASSERT_FALSE(fuzzer_pass.EquivalenceClassIsSuitableForBlock({6}, 18, 2));

  // Only id 26 from the set {24, 26, 30} is available to use for the
  // transformation at block 29, so the set is not suitable if we want at least
  // 2 available ids.
  ASSERT_FALSE(
      fuzzer_pass.EquivalenceClassIsSuitableForBlock({24, 26, 30}, 29, 2));

  ASSERT_TRUE(
      fuzzer_pass.EquivalenceClassIsSuitableForBlock({24, 26, 30}, 29, 1));

  // %21 is not available at the end of block 20.
  ASSERT_TRUE(ListsHaveTheSameElements<uint32_t>(
      fuzzer_pass.GetSuitableIds({9, 15, 21}, 20), {9, 15}));

  // %24 and %30 are not available at the end of block 18.
  ASSERT_TRUE(ListsHaveTheSameElements<uint32_t>(
      fuzzer_pass.GetSuitableIds({24, 26, 30}, 18), {26}));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
