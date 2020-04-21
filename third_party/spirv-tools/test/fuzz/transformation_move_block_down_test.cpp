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

#include "source/fuzz/transformation_move_block_down.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationMoveBlockDownTest, NoMovePossible1) {
  // Block 11 cannot be moved down as it dominates block 12.
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
          %9 = OpConstant %6 1
         %10 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpBranch %11
         %11 = OpLabel
               OpStore %8 %9
               OpBranch %12
         %12 = OpLabel
               OpStore %8 %10
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  FactManager fact_manager;

  auto transformation = TransformationMoveBlockDown(11);
  ASSERT_FALSE(transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationMoveBlockDownTest, NoMovePossible2) {
  // Block 5 cannot be moved down as it is the entry block.
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
          %9 = OpConstant %6 1
         %10 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %8 %10
               OpReturn
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  FactManager fact_manager;

  auto transformation = TransformationMoveBlockDown(5);
  ASSERT_FALSE(transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationMoveBlockDownTest, NoMovePossible3) {
  // Block 100 does not exist, so cannot be moved down.
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
          %9 = OpConstant %6 1
         %10 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpBranch %11
         %11 = OpLabel
               OpStore %8 %9
               OpBranch %12
         %12 = OpLabel
               OpStore %8 %10
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  FactManager fact_manager;

  auto transformation = TransformationMoveBlockDown(100);
  ASSERT_FALSE(transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationMoveBlockDownTest, NoMovePossible4) {
  // Block 12 is the last block in its function, so cannot be moved down.
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
          %9 = OpConstant %6 1
         %10 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpBranch %11
         %11 = OpLabel
               OpStore %8 %9
               OpBranch %12
         %12 = OpLabel
               OpStore %8 %10
               OpReturn
               OpFunctionEnd
         %13 = OpFunction %2 None %3
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  FactManager fact_manager;

  auto transformation = TransformationMoveBlockDown(12);
  ASSERT_FALSE(transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationMoveBlockDownTest, ManyMovesPossible) {
  // The SPIR-V arising from this shader has lots of opportunities for moving
  // blocks around.
  //
  // void main() {
  //   int x;
  //   int y;
  //   if (x < y) {
  //     x = 1;
  //     if (y == x) {
  //       x = 3;
  //     } else {
  //       x = 4;
  //     }
  //   } else {
  //     if (y < x) {
  //       x = 5;
  //     } else {
  //       x = 6;
  //     }
  //   }
  // }

  std::string before_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "y"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %9 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
               OpDecorate %17 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %12 = OpTypeBool
         %16 = OpConstant %6 1
         %22 = OpConstant %6 3
         %24 = OpConstant %6 4
         %31 = OpConstant %6 5
         %33 = OpConstant %6 6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
          %9 = OpLoad %6 %8
         %11 = OpLoad %6 %10
         %13 = OpSLessThan %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %25
         %14 = OpLabel
               OpStore %8 %16
         %17 = OpLoad %6 %10
         %18 = OpLoad %6 %8
         %19 = OpIEqual %12 %17 %18
               OpSelectionMerge %21 None
               OpBranchConditional %19 %20 %23
         %20 = OpLabel
               OpStore %8 %22
               OpBranch %21
         %23 = OpLabel
               OpStore %8 %24
               OpBranch %21
         %21 = OpLabel
               OpBranch %15
         %25 = OpLabel
         %26 = OpLoad %6 %10
         %27 = OpLoad %6 %8
         %28 = OpSLessThan %12 %26 %27
               OpSelectionMerge %30 None
               OpBranchConditional %28 %29 %32
         %29 = OpLabel
               OpStore %8 %31
               OpBranch %30
         %32 = OpLabel
               OpStore %8 %33
               OpBranch %30
         %30 = OpLabel
               OpBranch %15
         %15 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, before_transformation, kFuzzAssembleOption);

  FactManager fact_manager;

  // The block ids are: 5 14 20 23 21 25 29 32 30 15
  // We make a transformation to move each of them down, plus a transformation
  // to move a non-block, 27, down.
  auto move_down_5 = TransformationMoveBlockDown(5);
  auto move_down_14 = TransformationMoveBlockDown(14);
  auto move_down_20 = TransformationMoveBlockDown(20);
  auto move_down_23 = TransformationMoveBlockDown(23);
  auto move_down_21 = TransformationMoveBlockDown(21);
  auto move_down_25 = TransformationMoveBlockDown(25);
  auto move_down_29 = TransformationMoveBlockDown(29);
  auto move_down_32 = TransformationMoveBlockDown(32);
  auto move_down_30 = TransformationMoveBlockDown(30);
  auto move_down_15 = TransformationMoveBlockDown(15);
  auto move_down_27 = TransformationMoveBlockDown(27);

  // Dominance is as follows:
  //  5 dominates everything else
  // 14 dominates 20, 23, 21
  // 20 dominates nothing
  // 23 dominates nothing
  // 21 dominates nothing
  // 25 dominates 29, 32, 30
  // 29 dominates nothing
  // 32 dominates nothing
  // 30 dominates nothing
  // 15 dominates nothing

  // Current ordering: 5 14 20 23 21 25 29 32 30 15
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_20.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_15.IsApplicable(context.get(), fact_manager));

  // Let's bubble 20 all the way down.

  move_down_20.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Current ordering: 5 14 23 20 21 25 29 32 30 15
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_20.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_15.IsApplicable(context.get(), fact_manager));

  move_down_20.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Current ordering: 5 14 23 21 20 25 29 32 30 15
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_20.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_15.IsApplicable(context.get(), fact_manager));

  move_down_20.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Current ordering: 5 14 23 21 25 20 29 32 30 15
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_20.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_15.IsApplicable(context.get(), fact_manager));

  move_down_20.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Current ordering: 5 14 23 21 25 29 20 32 30 15
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_20.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_15.IsApplicable(context.get(), fact_manager));

  move_down_20.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Current ordering: 5 14 23 21 25 29 32 20 30 15
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_20.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_15.IsApplicable(context.get(), fact_manager));

  move_down_20.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Current ordering: 5 14 23 21 25 29 32 30 20 15
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_20.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_15.IsApplicable(context.get(), fact_manager));

  move_down_20.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_bubbling_20_down = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "y"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %9 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
               OpDecorate %17 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %12 = OpTypeBool
         %16 = OpConstant %6 1
         %22 = OpConstant %6 3
         %24 = OpConstant %6 4
         %31 = OpConstant %6 5
         %33 = OpConstant %6 6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
          %9 = OpLoad %6 %8
         %11 = OpLoad %6 %10
         %13 = OpSLessThan %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %25
         %14 = OpLabel
               OpStore %8 %16
         %17 = OpLoad %6 %10
         %18 = OpLoad %6 %8
         %19 = OpIEqual %12 %17 %18
               OpSelectionMerge %21 None
               OpBranchConditional %19 %20 %23
         %23 = OpLabel
               OpStore %8 %24
               OpBranch %21
         %21 = OpLabel
               OpBranch %15
         %25 = OpLabel
         %26 = OpLoad %6 %10
         %27 = OpLoad %6 %8
         %28 = OpSLessThan %12 %26 %27
               OpSelectionMerge %30 None
               OpBranchConditional %28 %29 %32
         %29 = OpLabel
               OpStore %8 %31
               OpBranch %30
         %32 = OpLabel
               OpStore %8 %33
               OpBranch %30
         %30 = OpLabel
               OpBranch %15
         %15 = OpLabel
               OpReturn
         %20 = OpLabel
               OpStore %8 %22
               OpBranch %21
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_bubbling_20_down, context.get()));

  // Current ordering: 5 14 23 21 25 29 32 30 15 20
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_15.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_20.IsApplicable(context.get(), fact_manager));

  move_down_23.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Current ordering: 5 14 21 23 25 29 32 30 15 20
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_15.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_20.IsApplicable(context.get(), fact_manager));

  move_down_23.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Current ordering: 5 14 21 25 23 29 32 30 15 20
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_15.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_20.IsApplicable(context.get(), fact_manager));

  move_down_21.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Current ordering: 5 14 25 21 23 29 32 30 15 20
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_15.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_20.IsApplicable(context.get(), fact_manager));

  move_down_14.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_more_shuffling = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "y"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %9 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
               OpDecorate %17 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %12 = OpTypeBool
         %16 = OpConstant %6 1
         %22 = OpConstant %6 3
         %24 = OpConstant %6 4
         %31 = OpConstant %6 5
         %33 = OpConstant %6 6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
          %9 = OpLoad %6 %8
         %11 = OpLoad %6 %10
         %13 = OpSLessThan %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %25
         %25 = OpLabel
         %26 = OpLoad %6 %10
         %27 = OpLoad %6 %8
         %28 = OpSLessThan %12 %26 %27
               OpSelectionMerge %30 None
               OpBranchConditional %28 %29 %32
         %14 = OpLabel
               OpStore %8 %16
         %17 = OpLoad %6 %10
         %18 = OpLoad %6 %8
         %19 = OpIEqual %12 %17 %18
               OpSelectionMerge %21 None
               OpBranchConditional %19 %20 %23
         %21 = OpLabel
               OpBranch %15
         %23 = OpLabel
               OpStore %8 %24
               OpBranch %21
         %29 = OpLabel
               OpStore %8 %31
               OpBranch %30
         %32 = OpLabel
               OpStore %8 %33
               OpBranch %30
         %30 = OpLabel
               OpBranch %15
         %15 = OpLabel
               OpReturn
         %20 = OpLabel
               OpStore %8 %22
               OpBranch %21
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_more_shuffling, context.get()));

  // Final ordering: 5 25 14 21 23 29 32 30 15 20
  ASSERT_FALSE(move_down_5.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_25.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_14.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_21.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_23.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_29.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_32.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_30.IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(move_down_15.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(move_down_20.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationMoveBlockDownTest, DoNotMoveUnreachable) {
  // Block 6 is unreachable, so cannot be moved down.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeInt 32 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
          %6 = OpLabel
          %7 = OpUndef %10
               OpBranch %8
          %8 = OpLabel
          %9 = OpCopyObject %10 %7
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto transformation = TransformationMoveBlockDown(6);
  ASSERT_FALSE(transformation.IsApplicable(context.get(), fact_manager));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
