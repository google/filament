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

#include "source/fuzz/comparator_deep_blocks_first.h"

#include "gtest/gtest.h"
#include "source/fuzz/fact_manager/fact_manager.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/pseudo_random_generator.h"
#include "source/fuzz/transformation_context.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

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
         %10 = OpConstant %7 10
         %11 = OpConstant %7 2
          %2 = OpFunction %3 None %4
         %12 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %6 %14 %15
         %14 = OpLabel
               OpBranch %13
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpLoopMerge %17 %18 None
               OpBranch %19
         %19 = OpLabel
               OpBranchConditional %6 %20 %17
         %20 = OpLabel
               OpSelectionMerge %21 None
               OpBranchConditional %6 %22 %23
         %22 = OpLabel
               OpBranch %21
         %23 = OpLabel
               OpBranch %21
         %21 = OpLabel
               OpBranch %18
         %18 = OpLabel
               OpBranch %16
         %17 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
)";

TEST(ComparatorDeepBlocksFirstTest, Compare) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto is_deeper = ComparatorDeepBlocksFirst(context.get());

  // The block ids and the corresponding depths are:
  // 12, 13          -> depth 0
  // 14, 15, 16, 17  -> depth 1
  // 18, 19, 20, 21  -> depth 2
  // 22, 23          -> depth 3

  // Perform some comparisons and check that they return true iff the first
  // block is deeper than the second.
  ASSERT_FALSE(is_deeper(12, 12));
  ASSERT_FALSE(is_deeper(12, 13));
  ASSERT_FALSE(is_deeper(12, 14));
  ASSERT_FALSE(is_deeper(12, 18));
  ASSERT_FALSE(is_deeper(12, 22));
  ASSERT_TRUE(is_deeper(14, 12));
  ASSERT_FALSE(is_deeper(14, 15));
  ASSERT_FALSE(is_deeper(15, 14));
  ASSERT_FALSE(is_deeper(14, 18));
  ASSERT_TRUE(is_deeper(18, 12));
  ASSERT_TRUE(is_deeper(18, 16));
}

TEST(ComparatorDeepBlocksFirstTest, Sort) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Check that, sorting using the comparator, the blocks are ordered from more
  // deeply nested to less deeply nested.
  // 17 has depth 1, 20 has depth 2, 13 has depth 0.
  std::vector<opt::BasicBlock*> blocks = {context->get_instr_block(17),
                                          context->get_instr_block(20),
                                          context->get_instr_block(13)};

  std::sort(blocks.begin(), blocks.end(),
            ComparatorDeepBlocksFirst(context.get()));

  // Check that the blocks are in the correct order.
  ASSERT_EQ(blocks[0]->id(), 20);
  ASSERT_EQ(blocks[1]->id(), 17);
  ASSERT_EQ(blocks[2]->id(), 13);
}
}  // namespace
}  // namespace fuzz
}  // namespace spvtools
