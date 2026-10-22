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

TEST(DeadBlockFactsTest, BlockIsDead) {
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
          %2 = OpFunction %3 None %4
          %9 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %6 %11 %12
         %11 = OpLabel
               OpBranch %10
         %12 = OpLabel
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

  FactManager fact_manager(context.get());

  ASSERT_FALSE(fact_manager.BlockIsDead(9));
  ASSERT_FALSE(fact_manager.BlockIsDead(11));
  ASSERT_FALSE(fact_manager.BlockIsDead(12));

  fact_manager.AddFactBlockIsDead(12);

  ASSERT_FALSE(fact_manager.BlockIsDead(9));
  ASSERT_FALSE(fact_manager.BlockIsDead(11));
  ASSERT_TRUE(fact_manager.BlockIsDead(12));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
