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

#include "source/fuzz/fact_manager/fact_manager.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(IrrelevantValueFactsTest, IdIsIrrelevant) {
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
         %12 = OpConstant %6 0
         %13 = OpConstant %6 1
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

  FactManager fact_manager(context.get());

  ASSERT_FALSE(fact_manager.IdIsIrrelevant(12));
  ASSERT_FALSE(fact_manager.IdIsIrrelevant(13));

  fact_manager.AddFactIdIsIrrelevant(12);

  ASSERT_TRUE(fact_manager.IdIsIrrelevant(12));
  ASSERT_FALSE(fact_manager.IdIsIrrelevant(13));
}

TEST(IrrelevantValueFactsTest, GetIrrelevantIds) {
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
         %12 = OpConstant %6 0
         %13 = OpConstant %6 1
         %14 = OpConstant %6 2
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

  FactManager fact_manager(context.get());

  ASSERT_EQ(fact_manager.GetIrrelevantIds(), std::unordered_set<uint32_t>({}));

  fact_manager.AddFactIdIsIrrelevant(12);

  ASSERT_EQ(fact_manager.GetIrrelevantIds(),
            std::unordered_set<uint32_t>({12}));

  fact_manager.AddFactIdIsIrrelevant(13);

  ASSERT_EQ(fact_manager.GetIrrelevantIds(),
            std::unordered_set<uint32_t>({12, 13}));
}

TEST(IrrelevantValueFactsTest, IdsFromDeadBlocksAreIrrelevant) {
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
          %8 = OpTypePointer Function %7
          %9 = OpConstant %7 1
          %2 = OpFunction %3 None %4
         %10 = OpLabel
         %11 = OpVariable %8 Function
               OpSelectionMerge %12 None
               OpBranchConditional %6 %13 %14
         %13 = OpLabel
               OpBranch %12
         %14 = OpLabel
         %15 = OpCopyObject %8 %11
         %16 = OpCopyObject %7 %9
         %17 = OpFunctionCall %3 %18
               OpBranch %12
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
         %18 = OpFunction %3 None %4
         %19 = OpLabel
         %20 = OpVariable %8 Function
         %21 = OpCopyObject %7 %9
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  FactManager fact_manager(context.get());

  ASSERT_FALSE(fact_manager.BlockIsDead(14));
  ASSERT_FALSE(fact_manager.BlockIsDead(19));

  // Initially no id is irrelevant.
  ASSERT_FALSE(fact_manager.IdIsIrrelevant(16));
  ASSERT_FALSE(fact_manager.IdIsIrrelevant(17));
  ASSERT_EQ(fact_manager.GetIrrelevantIds(), std::unordered_set<uint32_t>({}));

  fact_manager.AddFactBlockIsDead(14);

  // %16 and %17 should now be considered irrelevant.
  ASSERT_TRUE(fact_manager.IdIsIrrelevant(16));
  ASSERT_TRUE(fact_manager.IdIsIrrelevant(17));
  ASSERT_EQ(fact_manager.GetIrrelevantIds(),
            std::unordered_set<uint32_t>({16, 17}));

  // Similarly for %21.
  ASSERT_FALSE(fact_manager.IdIsIrrelevant(21));

  fact_manager.AddFactBlockIsDead(19);

  ASSERT_TRUE(fact_manager.IdIsIrrelevant(21));
  ASSERT_EQ(fact_manager.GetIrrelevantIds(),
            std::unordered_set<uint32_t>({16, 17, 21}));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
