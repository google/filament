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

#include "source/fuzz/transformation_add_dead_break.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddDeadBreakTest, BreaksOutOfSimpleIf) {
  // For a simple if-then-else, checks that some dead break scenarios are
  // possible, and sanity-checks that some illegal scenarios are indeed not
  // allowed.

  // The SPIR-V for this test is adapted from the following GLSL, by separating
  // some assignments into their own basic blocks, and adding constants for true
  // and false:
  //
  // void main() {
  //   int x;
  //   int y;
  //   x = 1;
  //   if (x < y) {
  //     x = 2;
  //     x = 3;
  //   } else {
  //     y = 2;
  //     y = 3;
  //   }
  //   x = y;
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %11 "y"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %13 = OpTypeBool
         %17 = OpConstant %6 2
         %18 = OpConstant %6 3
         %25 = OpConstantTrue %13
         %26 = OpConstantFalse %13
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %11 = OpVariable %7 Function
               OpStore %8 %9
         %10 = OpLoad %6 %8
         %12 = OpLoad %6 %11
         %14 = OpSLessThan %13 %10 %12
               OpSelectionMerge %16 None
               OpBranchConditional %14 %15 %19
         %15 = OpLabel
               OpStore %8 %17
               OpBranch %21
         %21 = OpLabel
               OpStore %8 %18
               OpBranch %22
         %22 = OpLabel
               OpBranch %16
         %19 = OpLabel
               OpStore %11 %17
               OpBranch %23
         %23 = OpLabel
               OpStore %11 %18
               OpBranch %24
         %24 = OpLabel
               OpBranch %16
         %16 = OpLabel
         %20 = OpLoad %6 %11
               OpStore %8 %20
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));
  FactManager fact_manager;

  const uint32_t merge_block = 16;

  // These are all possibilities.
  ASSERT_TRUE(TransformationAddDeadBreak(15, merge_block, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(15, merge_block, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(21, merge_block, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(21, merge_block, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(22, merge_block, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(22, merge_block, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(19, merge_block, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(19, merge_block, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(23, merge_block, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(23, merge_block, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(24, merge_block, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(24, merge_block, false, {})
                  .IsApplicable(context.get(), fact_manager));

  // Inapplicable: 100 is not a block id.
  ASSERT_FALSE(TransformationAddDeadBreak(100, merge_block, true, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(15, 100, true, {})
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable: 24 is not a merge block.
  ASSERT_FALSE(TransformationAddDeadBreak(15, 24, true, {})
                   .IsApplicable(context.get(), fact_manager));

  // These are the transformations we will apply.
  auto transformation1 = TransformationAddDeadBreak(15, merge_block, true, {});
  auto transformation2 = TransformationAddDeadBreak(21, merge_block, false, {});
  auto transformation3 = TransformationAddDeadBreak(22, merge_block, true, {});
  auto transformation4 = TransformationAddDeadBreak(19, merge_block, false, {});
  auto transformation5 = TransformationAddDeadBreak(23, merge_block, true, {});
  auto transformation6 = TransformationAddDeadBreak(24, merge_block, false, {});

  ASSERT_TRUE(transformation1.IsApplicable(context.get(), fact_manager));
  transformation1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation2.IsApplicable(context.get(), fact_manager));
  transformation2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation3.IsApplicable(context.get(), fact_manager));
  transformation3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation4.IsApplicable(context.get(), fact_manager));
  transformation4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation5.IsApplicable(context.get(), fact_manager));
  transformation5.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation6.IsApplicable(context.get(), fact_manager));
  transformation6.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %11 "y"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %13 = OpTypeBool
         %17 = OpConstant %6 2
         %18 = OpConstant %6 3
         %25 = OpConstantTrue %13
         %26 = OpConstantFalse %13
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %11 = OpVariable %7 Function
               OpStore %8 %9
         %10 = OpLoad %6 %8
         %12 = OpLoad %6 %11
         %14 = OpSLessThan %13 %10 %12
               OpSelectionMerge %16 None
               OpBranchConditional %14 %15 %19
         %15 = OpLabel
               OpStore %8 %17
               OpBranchConditional %25 %21 %16
         %21 = OpLabel
               OpStore %8 %18
               OpBranchConditional %26 %16 %22
         %22 = OpLabel
               OpBranchConditional %25 %16 %16
         %19 = OpLabel
               OpStore %11 %17
               OpBranchConditional %26 %16 %23
         %23 = OpLabel
               OpStore %11 %18
               OpBranchConditional %25 %24 %16
         %24 = OpLabel
               OpBranchConditional %26 %16 %16
         %16 = OpLabel
         %20 = OpLoad %6 %11
               OpStore %8 %20
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadBreakTest, BreakOutOfNestedIfs) {
  // Checks some allowed and disallowed scenarios for nests of ifs.

  // The SPIR-V for this test is adapted from the following GLSL:
  //
  // void main() {
  //   int x;
  //   int y;
  //   x = 1;
  //   if (x < y) {
  //     x = 2;
  //     x = 3;
  //     if (x == y) {
  //       y = 3;
  //     }
  //   } else {
  //     y = 2;
  //     y = 3;
  //   }
  //   if (x == y) {
  //     x = 2;
  //   }
  //   x = y;
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %11 "y"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %13 = OpTypeBool
         %17 = OpConstant %6 2
         %18 = OpConstant %6 3
         %31 = OpConstantTrue %13
         %32 = OpConstantFalse %13
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %11 = OpVariable %7 Function
               OpStore %8 %9
         %10 = OpLoad %6 %8
         %12 = OpLoad %6 %11
         %14 = OpSLessThan %13 %10 %12
               OpSelectionMerge %16 None
               OpBranchConditional %14 %15 %24
         %15 = OpLabel
               OpStore %8 %17
               OpBranch %33
         %33 = OpLabel
               OpStore %8 %18
         %19 = OpLoad %6 %8
               OpBranch %34
         %34 = OpLabel
         %20 = OpLoad %6 %11
         %21 = OpIEqual %13 %19 %20
               OpSelectionMerge %23 None
               OpBranchConditional %21 %22 %23
         %22 = OpLabel
               OpStore %11 %18
               OpBranch %35
         %35 = OpLabel
               OpBranch %23
         %23 = OpLabel
               OpBranch %16
         %24 = OpLabel
               OpStore %11 %17
               OpBranch %36
         %36 = OpLabel
               OpStore %11 %18
               OpBranch %16
         %16 = OpLabel
         %25 = OpLoad %6 %8
               OpBranch %37
         %37 = OpLabel
         %26 = OpLoad %6 %11
         %27 = OpIEqual %13 %25 %26
               OpSelectionMerge %29 None
               OpBranchConditional %27 %28 %29
         %28 = OpLabel
               OpStore %8 %17
               OpBranch %38
         %38 = OpLabel
               OpBranch %29
         %29 = OpLabel
         %30 = OpLoad %6 %11
               OpStore %8 %30
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // The header and merge blocks
  const uint32_t header_inner = 34;
  const uint32_t merge_inner = 23;
  const uint32_t header_outer = 5;
  const uint32_t merge_outer = 16;
  const uint32_t header_after = 37;
  const uint32_t merge_after = 29;

  // The non-merge-nor-header blocks in each construct
  const uint32_t inner_block_1 = 22;
  const uint32_t inner_block_2 = 35;
  const uint32_t outer_block_1 = 15;
  const uint32_t outer_block_2 = 33;
  const uint32_t outer_block_3 = 24;
  const uint32_t outer_block_4 = 36;
  const uint32_t after_block_1 = 28;
  const uint32_t after_block_2 = 38;

  // Fine to break from a construct to its merge
  ASSERT_TRUE(TransformationAddDeadBreak(inner_block_1, merge_inner, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(inner_block_2, merge_inner, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(outer_block_1, merge_outer, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(outer_block_2, merge_outer, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(outer_block_3, merge_outer, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(outer_block_4, merge_outer, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(after_block_1, merge_after, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(after_block_2, merge_after, false, {})
                  .IsApplicable(context.get(), fact_manager));

  // Not OK to break to the wrong merge (whether enclosing or not)
  ASSERT_FALSE(TransformationAddDeadBreak(inner_block_1, merge_outer, true, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(inner_block_2, merge_after, false, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(outer_block_1, merge_inner, true, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(outer_block_2, merge_after, false, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(after_block_1, merge_inner, true, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(after_block_2, merge_outer, false, {})
                   .IsApplicable(context.get(), fact_manager));

  // Not OK to break from header (as it does not branch unconditionally)
  ASSERT_FALSE(TransformationAddDeadBreak(header_inner, merge_inner, true, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(header_outer, merge_outer, false, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(header_after, merge_after, true, {})
                   .IsApplicable(context.get(), fact_manager));

  // Not OK to break to non-merge
  ASSERT_FALSE(
      TransformationAddDeadBreak(inner_block_1, inner_block_2, true, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      TransformationAddDeadBreak(outer_block_2, after_block_1, false, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(outer_block_1, header_after, true, {})
                   .IsApplicable(context.get(), fact_manager));

  auto transformation1 =
      TransformationAddDeadBreak(inner_block_1, merge_inner, true, {});
  auto transformation2 =
      TransformationAddDeadBreak(inner_block_2, merge_inner, false, {});
  auto transformation3 =
      TransformationAddDeadBreak(outer_block_1, merge_outer, true, {});
  auto transformation4 =
      TransformationAddDeadBreak(outer_block_2, merge_outer, false, {});
  auto transformation5 =
      TransformationAddDeadBreak(outer_block_3, merge_outer, true, {});
  auto transformation6 =
      TransformationAddDeadBreak(outer_block_4, merge_outer, false, {});
  auto transformation7 =
      TransformationAddDeadBreak(after_block_1, merge_after, true, {});
  auto transformation8 =
      TransformationAddDeadBreak(after_block_2, merge_after, false, {});

  ASSERT_TRUE(transformation1.IsApplicable(context.get(), fact_manager));
  transformation1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation2.IsApplicable(context.get(), fact_manager));
  transformation2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation3.IsApplicable(context.get(), fact_manager));
  transformation3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation4.IsApplicable(context.get(), fact_manager));
  transformation4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation5.IsApplicable(context.get(), fact_manager));
  transformation5.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation6.IsApplicable(context.get(), fact_manager));
  transformation6.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation7.IsApplicable(context.get(), fact_manager));
  transformation7.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation8.IsApplicable(context.get(), fact_manager));
  transformation8.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %11 "y"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %13 = OpTypeBool
         %17 = OpConstant %6 2
         %18 = OpConstant %6 3
         %31 = OpConstantTrue %13
         %32 = OpConstantFalse %13
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %11 = OpVariable %7 Function
               OpStore %8 %9
         %10 = OpLoad %6 %8
         %12 = OpLoad %6 %11
         %14 = OpSLessThan %13 %10 %12
               OpSelectionMerge %16 None
               OpBranchConditional %14 %15 %24
         %15 = OpLabel
               OpStore %8 %17
               OpBranchConditional %31 %33 %16
         %33 = OpLabel
               OpStore %8 %18
         %19 = OpLoad %6 %8
               OpBranchConditional %32 %16 %34
         %34 = OpLabel
         %20 = OpLoad %6 %11
         %21 = OpIEqual %13 %19 %20
               OpSelectionMerge %23 None
               OpBranchConditional %21 %22 %23
         %22 = OpLabel
               OpStore %11 %18
               OpBranchConditional %31 %35 %23
         %35 = OpLabel
               OpBranchConditional %32 %23 %23
         %23 = OpLabel
               OpBranch %16
         %24 = OpLabel
               OpStore %11 %17
               OpBranchConditional %31 %36 %16
         %36 = OpLabel
               OpStore %11 %18
               OpBranchConditional %32 %16 %16
         %16 = OpLabel
         %25 = OpLoad %6 %8
               OpBranch %37
         %37 = OpLabel
         %26 = OpLoad %6 %11
         %27 = OpIEqual %13 %25 %26
               OpSelectionMerge %29 None
               OpBranchConditional %27 %28 %29
         %28 = OpLabel
               OpStore %8 %17
               OpBranchConditional %31 %38 %29
         %38 = OpLabel
               OpBranchConditional %32 %29 %29
         %29 = OpLabel
         %30 = OpLoad %6 %11
               OpStore %8 %30
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadBreakTest, BreakOutOfNestedSwitches) {
  // Checks some allowed and disallowed scenarios for nests of switches.

  // The SPIR-V for this test is adapted from the following GLSL:
  //
  // void main() {
  //   int x;
  //   int y;
  //   x = 1;
  //   if (x < y) {
  //     switch (x) {
  //       case 0:
  //       case 1:
  //         if (x == y) {
  //         }
  //         x = 2;
  //         break;
  //       case 3:
  //         if (y == 4) {
  //           y = 2;
  //           x = 3;
  //         }
  //       case 10:
  //         break;
  //       default:
  //         switch (y) {
  //           case 1:
  //             break;
  //           case 2:
  //             x = 4;
  //             y = 2;
  //           default:
  //             x = 3;
  //             break;
  //         }
  //     }
  //   } else {
  //     switch (y) {
  //       case 1:
  //         x = 4;
  //       case 2:
  //         y = 3;
  //       default:
  //         x = y;
  //         break;
  //     }
  //   }
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %11 "y"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %13 = OpTypeBool
         %29 = OpConstant %6 2
         %32 = OpConstant %6 4
         %36 = OpConstant %6 3
         %60 = OpConstantTrue %13
         %61 = OpConstantFalse %13
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %11 = OpVariable %7 Function
               OpStore %8 %9
         %10 = OpLoad %6 %8
         %12 = OpLoad %6 %11
         %14 = OpSLessThan %13 %10 %12
               OpSelectionMerge %16 None
               OpBranchConditional %14 %15 %47
         %15 = OpLabel
         %17 = OpLoad %6 %8
               OpSelectionMerge %22 None
               OpSwitch %17 %21 0 %18 1 %18 3 %19 10 %20
         %21 = OpLabel
         %38 = OpLoad %6 %11
               OpSelectionMerge %42 None
               OpSwitch %38 %41 1 %39 2 %40
         %41 = OpLabel
               OpStore %8 %36
               OpBranch %42
         %39 = OpLabel
               OpBranch %42
         %40 = OpLabel
               OpStore %8 %32
               OpStore %11 %29
               OpBranch %41
         %42 = OpLabel
               OpBranch %22
         %18 = OpLabel
         %23 = OpLoad %6 %8
               OpBranch %63
         %63 = OpLabel
         %24 = OpLoad %6 %11
         %25 = OpIEqual %13 %23 %24
               OpSelectionMerge %27 None
               OpBranchConditional %25 %26 %27
         %26 = OpLabel
               OpBranch %27
         %27 = OpLabel
               OpStore %8 %29
               OpBranch %22
         %19 = OpLabel
         %31 = OpLoad %6 %11
         %33 = OpIEqual %13 %31 %32
               OpSelectionMerge %35 None
               OpBranchConditional %33 %34 %35
         %34 = OpLabel
               OpStore %11 %29
               OpBranch %62
         %62 = OpLabel
               OpStore %8 %36
               OpBranch %35
         %35 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpBranch %16
         %47 = OpLabel
         %48 = OpLoad %6 %11
               OpSelectionMerge %52 None
               OpSwitch %48 %51 1 %49 2 %50
         %51 = OpLabel
         %53 = OpLoad %6 %11
               OpStore %8 %53
               OpBranch %52
         %49 = OpLabel
               OpStore %8 %32
               OpBranch %50
         %50 = OpLabel
               OpStore %11 %36
               OpBranch %51
         %52 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // The header and merge blocks
  const uint32_t header_outer_if = 5;
  const uint32_t merge_outer_if = 16;
  const uint32_t header_then_outer_switch = 15;
  const uint32_t merge_then_outer_switch = 22;
  const uint32_t header_then_inner_switch = 21;
  const uint32_t merge_then_inner_switch = 42;
  const uint32_t header_else_switch = 47;
  const uint32_t merge_else_switch = 52;
  const uint32_t header_inner_if_1 = 19;
  const uint32_t merge_inner_if_1 = 35;
  const uint32_t header_inner_if_2 = 63;
  const uint32_t merge_inner_if_2 = 27;

  // The non-merge-nor-header blocks in each construct
  const uint32_t then_outer_switch_block_1 = 18;
  const uint32_t then_inner_switch_block_1 = 39;
  const uint32_t then_inner_switch_block_2 = 40;
  const uint32_t then_inner_switch_block_3 = 41;
  const uint32_t else_switch_block_1 = 49;
  const uint32_t else_switch_block_2 = 50;
  const uint32_t else_switch_block_3 = 51;
  const uint32_t inner_if_1_block_1 = 34;
  const uint32_t inner_if_1_block_2 = 62;
  const uint32_t inner_if_2_block_1 = 26;

  // Fine to branch straight to direct merge block for a construct
  ASSERT_TRUE(TransformationAddDeadBreak(then_outer_switch_block_1,
                                         merge_then_outer_switch, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(then_inner_switch_block_1,
                                         merge_then_inner_switch, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(then_inner_switch_block_2,
                                         merge_then_inner_switch, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(then_inner_switch_block_3,
                                         merge_then_inner_switch, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(else_switch_block_1, merge_else_switch,
                                         false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(else_switch_block_2, merge_else_switch,
                                         true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(else_switch_block_3, merge_else_switch,
                                         false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(
      TransformationAddDeadBreak(inner_if_1_block_1, merge_inner_if_1, true, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(inner_if_1_block_2, merge_inner_if_1,
                                         false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(
      TransformationAddDeadBreak(inner_if_2_block_1, merge_inner_if_2, true, {})
          .IsApplicable(context.get(), fact_manager));

  // Not OK to break out of a switch from a selection construct inside the
  // switch.
  ASSERT_FALSE(TransformationAddDeadBreak(inner_if_1_block_1,
                                          merge_then_outer_switch, true, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(inner_if_1_block_2,
                                          merge_then_outer_switch, false, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(inner_if_2_block_1,
                                          merge_then_outer_switch, true, {})
                   .IsApplicable(context.get(), fact_manager));

  // Some miscellaneous inapplicable cases.
  ASSERT_FALSE(
      TransformationAddDeadBreak(header_outer_if, merge_outer_if, true, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(header_inner_if_1, inner_if_1_block_2,
                                          false, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(header_then_inner_switch,
                                          header_then_outer_switch, false, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(header_else_switch,
                                          then_inner_switch_block_3, false, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(header_inner_if_2, header_inner_if_2,
                                          false, {})
                   .IsApplicable(context.get(), fact_manager));

  auto transformation1 = TransformationAddDeadBreak(
      then_outer_switch_block_1, merge_then_outer_switch, true, {});
  auto transformation2 = TransformationAddDeadBreak(
      then_inner_switch_block_1, merge_then_inner_switch, false, {});
  auto transformation3 = TransformationAddDeadBreak(
      then_inner_switch_block_2, merge_then_inner_switch, true, {});
  auto transformation4 = TransformationAddDeadBreak(
      then_inner_switch_block_3, merge_then_inner_switch, true, {});
  auto transformation5 = TransformationAddDeadBreak(
      else_switch_block_1, merge_else_switch, false, {});
  auto transformation6 = TransformationAddDeadBreak(
      else_switch_block_2, merge_else_switch, true, {});
  auto transformation7 = TransformationAddDeadBreak(
      else_switch_block_3, merge_else_switch, false, {});
  auto transformation8 = TransformationAddDeadBreak(inner_if_1_block_1,
                                                    merge_inner_if_1, true, {});
  auto transformation9 = TransformationAddDeadBreak(
      inner_if_1_block_2, merge_inner_if_1, false, {});
  auto transformation10 = TransformationAddDeadBreak(
      inner_if_2_block_1, merge_inner_if_2, true, {});

  ASSERT_TRUE(transformation1.IsApplicable(context.get(), fact_manager));
  transformation1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation2.IsApplicable(context.get(), fact_manager));
  transformation2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation3.IsApplicable(context.get(), fact_manager));
  transformation3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation4.IsApplicable(context.get(), fact_manager));
  transformation4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation5.IsApplicable(context.get(), fact_manager));
  transformation5.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation6.IsApplicable(context.get(), fact_manager));
  transformation6.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation7.IsApplicable(context.get(), fact_manager));
  transformation7.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation8.IsApplicable(context.get(), fact_manager));
  transformation8.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation9.IsApplicable(context.get(), fact_manager));
  transformation9.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation10.IsApplicable(context.get(), fact_manager));
  transformation10.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %11 "y"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %13 = OpTypeBool
         %29 = OpConstant %6 2
         %32 = OpConstant %6 4
         %36 = OpConstant %6 3
         %60 = OpConstantTrue %13
         %61 = OpConstantFalse %13
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %11 = OpVariable %7 Function
               OpStore %8 %9
         %10 = OpLoad %6 %8
         %12 = OpLoad %6 %11
         %14 = OpSLessThan %13 %10 %12
               OpSelectionMerge %16 None
               OpBranchConditional %14 %15 %47
         %15 = OpLabel
         %17 = OpLoad %6 %8
               OpSelectionMerge %22 None
               OpSwitch %17 %21 0 %18 1 %18 3 %19 10 %20
         %21 = OpLabel
         %38 = OpLoad %6 %11
               OpSelectionMerge %42 None
               OpSwitch %38 %41 1 %39 2 %40
         %41 = OpLabel
               OpStore %8 %36
               OpBranchConditional %60 %42 %42
         %39 = OpLabel
               OpBranchConditional %61 %42 %42
         %40 = OpLabel
               OpStore %8 %32
               OpStore %11 %29
               OpBranchConditional %60 %41 %42
         %42 = OpLabel
               OpBranch %22
         %18 = OpLabel
         %23 = OpLoad %6 %8
               OpBranchConditional %60 %63 %22
         %63 = OpLabel
         %24 = OpLoad %6 %11
         %25 = OpIEqual %13 %23 %24
               OpSelectionMerge %27 None
               OpBranchConditional %25 %26 %27
         %26 = OpLabel
               OpBranchConditional %60 %27 %27
         %27 = OpLabel
               OpStore %8 %29
               OpBranch %22
         %19 = OpLabel
         %31 = OpLoad %6 %11
         %33 = OpIEqual %13 %31 %32
               OpSelectionMerge %35 None
               OpBranchConditional %33 %34 %35
         %34 = OpLabel
               OpStore %11 %29
               OpBranchConditional %60 %62 %35
         %62 = OpLabel
               OpStore %8 %36
               OpBranchConditional %61 %35 %35
         %35 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpBranch %16
         %47 = OpLabel
         %48 = OpLoad %6 %11
               OpSelectionMerge %52 None
               OpSwitch %48 %51 1 %49 2 %50
         %51 = OpLabel
         %53 = OpLoad %6 %11
               OpStore %8 %53
               OpBranchConditional %61 %52 %52
         %49 = OpLabel
               OpStore %8 %32
               OpBranchConditional %61 %52 %50
         %50 = OpLabel
               OpStore %11 %36
               OpBranchConditional %60 %51 %52
         %52 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadBreakTest, BreakOutOfLoopNest) {
  // Checks some allowed and disallowed scenarios for a nest of loops, including
  // breaking from an if or switch right out of a loop.

  // The SPIR-V for this test is adapted from the following GLSL:
  //
  // void main() {
  //   int x, y;
  //   do {
  //     x++;
  //     for (int j = 0; j < 100; j++) {
  //       y++;
  //       if (x == y) {
  //         x++;
  //         if (x == 2) {
  //           y++;
  //         }
  //         switch (x) {
  //           case 0:
  //             x = 2;
  //           default:
  //             break;
  //         }
  //       }
  //     }
  //   } while (x > y);
  //
  //   for (int i = 0; i < 100; i++) {
  //     x++;
  //   }
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %12 "x"
               OpName %16 "j"
               OpName %27 "y"
               OpName %55 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Function %10
         %14 = OpConstant %10 1
         %17 = OpConstant %10 0
         %24 = OpConstant %10 100
         %25 = OpTypeBool
         %38 = OpConstant %10 2
         %67 = OpConstantTrue %25
         %68 = OpConstantFalse %25
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %12 = OpVariable %11 Function
         %16 = OpVariable %11 Function
         %27 = OpVariable %11 Function
         %55 = OpVariable %11 Function
               OpBranch %6
          %6 = OpLabel
               OpLoopMerge %8 %9 None
               OpBranch %7
          %7 = OpLabel
         %13 = OpLoad %10 %12
         %15 = OpIAdd %10 %13 %14
               OpStore %12 %15
               OpStore %16 %17
               OpBranch %18
         %18 = OpLabel
               OpLoopMerge %20 %21 None
               OpBranch %22
         %22 = OpLabel
         %23 = OpLoad %10 %16
         %26 = OpSLessThan %25 %23 %24
               OpBranchConditional %26 %19 %20
         %19 = OpLabel
         %28 = OpLoad %10 %27
         %29 = OpIAdd %10 %28 %14
               OpStore %27 %29
         %30 = OpLoad %10 %12
         %31 = OpLoad %10 %27
         %32 = OpIEqual %25 %30 %31
               OpSelectionMerge %34 None
               OpBranchConditional %32 %33 %34
         %33 = OpLabel
         %35 = OpLoad %10 %12
         %36 = OpIAdd %10 %35 %14
               OpStore %12 %36
         %37 = OpLoad %10 %12
         %39 = OpIEqual %25 %37 %38
               OpSelectionMerge %41 None
               OpBranchConditional %39 %40 %41
         %40 = OpLabel
         %42 = OpLoad %10 %27
         %43 = OpIAdd %10 %42 %14
               OpStore %27 %43
               OpBranch %41
         %41 = OpLabel
         %44 = OpLoad %10 %12
               OpSelectionMerge %47 None
               OpSwitch %44 %46 0 %45
         %46 = OpLabel
               OpBranch %47
         %45 = OpLabel
               OpStore %12 %38
               OpBranch %46
         %47 = OpLabel
               OpBranch %34
         %34 = OpLabel
               OpBranch %21
         %21 = OpLabel
         %50 = OpLoad %10 %16
         %51 = OpIAdd %10 %50 %14
               OpStore %16 %51
               OpBranch %18
         %20 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %52 = OpLoad %10 %12
         %53 = OpLoad %10 %27
         %54 = OpSGreaterThan %25 %52 %53
               OpBranchConditional %54 %6 %8
          %8 = OpLabel
               OpStore %55 %17
               OpBranch %56
         %56 = OpLabel
               OpLoopMerge %58 %59 None
               OpBranch %60
         %60 = OpLabel
         %61 = OpLoad %10 %55
         %62 = OpSLessThan %25 %61 %24
               OpBranchConditional %62 %57 %58
         %57 = OpLabel
         %63 = OpLoad %10 %12
         %64 = OpIAdd %10 %63 %14
               OpStore %12 %64
               OpBranch %59
         %59 = OpLabel
         %65 = OpLoad %10 %55
         %66 = OpIAdd %10 %65 %14
               OpStore %55 %66
               OpBranch %56
         %58 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // The header and merge blocks
  const uint32_t header_do_while = 6;
  const uint32_t merge_do_while = 8;
  const uint32_t header_for_j = 18;
  const uint32_t merge_for_j = 20;
  const uint32_t header_for_i = 56;
  const uint32_t merge_for_i = 58;
  const uint32_t header_switch = 41;
  const uint32_t merge_switch = 47;
  const uint32_t header_if_x_eq_y = 19;
  const uint32_t merge_if_x_eq_y = 34;
  const uint32_t header_if_x_eq_2 = 33;
  const uint32_t merge_if_x_eq_2 = 41;

  // Loop continue targets
  const uint32_t continue_do_while = 9;
  const uint32_t continue_for_j = 21;
  const uint32_t continue_for_i = 59;

  // Some blocks in these constructs
  const uint32_t block_in_inner_if = 40;
  const uint32_t block_switch_case = 46;
  const uint32_t block_switch_default = 45;
  const uint32_t block_in_for_i_loop = 57;

  // Fine to break from any loop header to its merge
  ASSERT_TRUE(
      TransformationAddDeadBreak(header_do_while, merge_do_while, true, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(header_for_i, merge_for_i, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadBreak(header_for_j, merge_for_j, true, {})
                  .IsApplicable(context.get(), fact_manager));

  // Fine to break from any of the blocks in constructs in the "for j" loop to
  // that loop's merge
  ASSERT_TRUE(
      TransformationAddDeadBreak(block_in_inner_if, merge_for_j, false, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(
      TransformationAddDeadBreak(block_switch_case, merge_for_j, true, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(
      TransformationAddDeadBreak(block_switch_default, merge_for_j, false, {})
          .IsApplicable(context.get(), fact_manager));

  // Fine to break from the body of the "for i" loop to that loop's merge
  ASSERT_TRUE(
      TransformationAddDeadBreak(block_in_for_i_loop, merge_for_i, true, {})
          .IsApplicable(context.get(), fact_manager));

  // Not OK to break from multiple loops
  ASSERT_FALSE(
      TransformationAddDeadBreak(block_in_inner_if, merge_do_while, false, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      TransformationAddDeadBreak(block_switch_case, merge_do_while, true, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(block_switch_default, merge_do_while,
                                          false, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      TransformationAddDeadBreak(header_for_j, merge_do_while, true, {})
          .IsApplicable(context.get(), fact_manager));

  // Not OK to break loop from its continue construct
  ASSERT_FALSE(
      TransformationAddDeadBreak(continue_do_while, merge_do_while, true, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      TransformationAddDeadBreak(continue_for_j, merge_for_j, false, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(continue_for_i, merge_for_i, true, {})
                   .IsApplicable(context.get(), fact_manager));

  // Not OK to break out of multiple non-loop constructs if not breaking to a
  // loop merge
  ASSERT_FALSE(
      TransformationAddDeadBreak(block_in_inner_if, merge_if_x_eq_y, false, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      TransformationAddDeadBreak(block_switch_case, merge_if_x_eq_y, true, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(block_switch_default, merge_if_x_eq_y,
                                          false, {})
                   .IsApplicable(context.get(), fact_manager));

  // Some miscellaneous inapplicable transformations
  ASSERT_FALSE(
      TransformationAddDeadBreak(header_if_x_eq_2, header_if_x_eq_y, false, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      TransformationAddDeadBreak(merge_if_x_eq_2, merge_switch, false, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      TransformationAddDeadBreak(header_switch, header_switch, false, {})
          .IsApplicable(context.get(), fact_manager));

  auto transformation1 =
      TransformationAddDeadBreak(header_do_while, merge_do_while, true, {});
  auto transformation2 =
      TransformationAddDeadBreak(header_for_i, merge_for_i, false, {});
  auto transformation3 =
      TransformationAddDeadBreak(header_for_j, merge_for_j, true, {});
  auto transformation4 =
      TransformationAddDeadBreak(block_in_inner_if, merge_for_j, false, {});
  auto transformation5 =
      TransformationAddDeadBreak(block_switch_case, merge_for_j, true, {});
  auto transformation6 =
      TransformationAddDeadBreak(block_switch_default, merge_for_j, false, {});
  auto transformation7 =
      TransformationAddDeadBreak(block_in_for_i_loop, merge_for_i, true, {});

  ASSERT_TRUE(transformation1.IsApplicable(context.get(), fact_manager));
  transformation1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation2.IsApplicable(context.get(), fact_manager));
  transformation2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation3.IsApplicable(context.get(), fact_manager));
  transformation3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation4.IsApplicable(context.get(), fact_manager));
  transformation4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation5.IsApplicable(context.get(), fact_manager));
  transformation5.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation6.IsApplicable(context.get(), fact_manager));
  transformation6.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation7.IsApplicable(context.get(), fact_manager));
  transformation7.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %12 "x"
               OpName %16 "j"
               OpName %27 "y"
               OpName %55 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Function %10
         %14 = OpConstant %10 1
         %17 = OpConstant %10 0
         %24 = OpConstant %10 100
         %25 = OpTypeBool
         %38 = OpConstant %10 2
         %67 = OpConstantTrue %25
         %68 = OpConstantFalse %25
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %12 = OpVariable %11 Function
         %16 = OpVariable %11 Function
         %27 = OpVariable %11 Function
         %55 = OpVariable %11 Function
               OpBranch %6
          %6 = OpLabel
               OpLoopMerge %8 %9 None
               OpBranchConditional %67 %7 %8
          %7 = OpLabel
         %13 = OpLoad %10 %12
         %15 = OpIAdd %10 %13 %14
               OpStore %12 %15
               OpStore %16 %17
               OpBranch %18
         %18 = OpLabel
               OpLoopMerge %20 %21 None
               OpBranchConditional %67 %22 %20
         %22 = OpLabel
         %23 = OpLoad %10 %16
         %26 = OpSLessThan %25 %23 %24
               OpBranchConditional %26 %19 %20
         %19 = OpLabel
         %28 = OpLoad %10 %27
         %29 = OpIAdd %10 %28 %14
               OpStore %27 %29
         %30 = OpLoad %10 %12
         %31 = OpLoad %10 %27
         %32 = OpIEqual %25 %30 %31
               OpSelectionMerge %34 None
               OpBranchConditional %32 %33 %34
         %33 = OpLabel
         %35 = OpLoad %10 %12
         %36 = OpIAdd %10 %35 %14
               OpStore %12 %36
         %37 = OpLoad %10 %12
         %39 = OpIEqual %25 %37 %38
               OpSelectionMerge %41 None
               OpBranchConditional %39 %40 %41
         %40 = OpLabel
         %42 = OpLoad %10 %27
         %43 = OpIAdd %10 %42 %14
               OpStore %27 %43
               OpBranchConditional %68 %20 %41
         %41 = OpLabel
         %44 = OpLoad %10 %12
               OpSelectionMerge %47 None
               OpSwitch %44 %46 0 %45
         %46 = OpLabel
               OpBranchConditional %67 %47 %20
         %45 = OpLabel
               OpStore %12 %38
               OpBranchConditional %68 %20 %46
         %47 = OpLabel
               OpBranch %34
         %34 = OpLabel
               OpBranch %21
         %21 = OpLabel
         %50 = OpLoad %10 %16
         %51 = OpIAdd %10 %50 %14
               OpStore %16 %51
               OpBranch %18
         %20 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %52 = OpLoad %10 %12
         %53 = OpLoad %10 %27
         %54 = OpSGreaterThan %25 %52 %53
               OpBranchConditional %54 %6 %8
          %8 = OpLabel
               OpStore %55 %17
               OpBranch %56
         %56 = OpLabel
               OpLoopMerge %58 %59 None
               OpBranchConditional %68 %58 %60
         %60 = OpLabel
         %61 = OpLoad %10 %55
         %62 = OpSLessThan %25 %61 %24
               OpBranchConditional %62 %57 %58
         %57 = OpLabel
         %63 = OpLoad %10 %12
         %64 = OpIAdd %10 %63 %14
               OpStore %12 %64
               OpBranchConditional %67 %59 %58
         %59 = OpLabel
         %65 = OpLoad %10 %55
         %66 = OpIAdd %10 %65 %14
               OpStore %55 %66
               OpBranch %56
         %58 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadBreakTest, NoBreakFromContinueConstruct) {
  // Checks that it is illegal to break straight from a continue construct.

  // The SPIR-V for this test is adapted from the following GLSL:
  //
  // void main() {
  //   for (int i = 0; i < 100; i++) {
  //   }
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %15 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %22 = OpConstantTrue %17
         %20 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %15 = OpLoad %6 %8
         %18 = OpSLessThan %17 %15 %16
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %19 = OpLoad %6 %8
         %21 = OpIAdd %6 %19 %20
               OpBranch %23
         %23 = OpLabel
               OpStore %8 %21
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // Not OK to break loop from its continue construct
  ASSERT_FALSE(TransformationAddDeadBreak(13, 12, true, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(23, 12, true, {})
                   .IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadBreakTest, SelectionInContinueConstruct) {
  // Considers some scenarios where there is a selection construct in a loop's
  // continue construct.

  // The SPIR-V for this test is adapted from the following GLSL:
  //
  // void main() {
  //   for (int i = 0; i < 100; i = (i < 50 ? i + 2 : i + 1)) {
  //   }
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %99 = OpConstantTrue %17
         %20 = OpConstant %6 50
         %26 = OpConstant %6 2
         %30 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %22 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %15 = OpLoad %6 %8
         %18 = OpSLessThan %17 %15 %16
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %19 = OpLoad %6 %8
         %21 = OpSLessThan %17 %19 %20
               OpSelectionMerge %24 None
               OpBranchConditional %21 %23 %28
         %23 = OpLabel
         %25 = OpLoad %6 %8
               OpBranch %100
        %100 = OpLabel
         %27 = OpIAdd %6 %25 %26
               OpStore %22 %27
               OpBranch %24
         %28 = OpLabel
         %29 = OpLoad %6 %8
               OpBranch %101
        %101 = OpLabel
         %31 = OpIAdd %6 %29 %30
               OpStore %22 %31
               OpBranch %24
         %24 = OpLabel
         %32 = OpLoad %6 %22
               OpStore %8 %32
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  const uint32_t loop_merge = 12;
  const uint32_t selection_merge = 24;
  const uint32_t in_selection_1 = 23;
  const uint32_t in_selection_2 = 100;
  const uint32_t in_selection_3 = 28;
  const uint32_t in_selection_4 = 101;

  // Not OK to jump from the selection to the loop merge, as this would break
  // from the loop's continue construct.
  ASSERT_FALSE(TransformationAddDeadBreak(in_selection_1, loop_merge, true, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(in_selection_2, loop_merge, true, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(in_selection_3, loop_merge, true, {})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationAddDeadBreak(in_selection_4, loop_merge, true, {})
                   .IsApplicable(context.get(), fact_manager));

  // But fine to jump from the selection to its merge.

  auto transformation1 =
      TransformationAddDeadBreak(in_selection_1, selection_merge, true, {});
  auto transformation2 =
      TransformationAddDeadBreak(in_selection_2, selection_merge, true, {});
  auto transformation3 =
      TransformationAddDeadBreak(in_selection_3, selection_merge, true, {});
  auto transformation4 =
      TransformationAddDeadBreak(in_selection_4, selection_merge, true, {});

  ASSERT_TRUE(transformation1.IsApplicable(context.get(), fact_manager));
  transformation1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation2.IsApplicable(context.get(), fact_manager));
  transformation2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation3.IsApplicable(context.get(), fact_manager));
  transformation3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation4.IsApplicable(context.get(), fact_manager));
  transformation4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %99 = OpConstantTrue %17
         %20 = OpConstant %6 50
         %26 = OpConstant %6 2
         %30 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %22 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %15 = OpLoad %6 %8
         %18 = OpSLessThan %17 %15 %16
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %19 = OpLoad %6 %8
         %21 = OpSLessThan %17 %19 %20
               OpSelectionMerge %24 None
               OpBranchConditional %21 %23 %28
         %23 = OpLabel
         %25 = OpLoad %6 %8
               OpBranchConditional %99 %100 %24
        %100 = OpLabel
         %27 = OpIAdd %6 %25 %26
               OpStore %22 %27
               OpBranchConditional %99 %24 %24
         %28 = OpLabel
         %29 = OpLoad %6 %8
               OpBranchConditional %99 %101 %24
        %101 = OpLabel
         %31 = OpIAdd %6 %29 %30
               OpStore %22 %31
               OpBranchConditional %99 %24 %24
         %24 = OpLabel
         %32 = OpLoad %6 %22
               OpStore %8 %32
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadBreakTest, LoopInContinueConstruct) {
  // Considers some scenarios where there is a loop in a loop's continue
  // construct.

  // The SPIR-V for this test is adapted from the following GLSL, with inlining
  // applied so that the loop from foo is in the main loop's continue construct:
  //
  // int foo() {
  //   int result = 0;
  //   for (int j = 0; j < 10; j++) {
  //     result++;
  //   }
  //   return result;
  // }
  //
  // void main() {
  //   for (int i = 0; i < 100; i += foo()) {
  //   }
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %31 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFunction %6
         %10 = OpTypePointer Function %6
         %12 = OpConstant %6 0
         %20 = OpConstant %6 10
         %21 = OpTypeBool
        %100 = OpConstantTrue %21
         %24 = OpConstant %6 1
         %38 = OpConstant %6 100
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %43 = OpVariable %10 Function
         %44 = OpVariable %10 Function
         %45 = OpVariable %10 Function
         %31 = OpVariable %10 Function
               OpStore %31 %12
               OpBranch %32
         %32 = OpLabel
               OpLoopMerge %34 %35 None
               OpBranch %36
         %36 = OpLabel
         %37 = OpLoad %6 %31
         %39 = OpSLessThan %21 %37 %38
               OpBranchConditional %39 %33 %34
         %33 = OpLabel
               OpBranch %35
         %35 = OpLabel
               OpStore %43 %12
               OpStore %44 %12
               OpBranch %46
         %46 = OpLabel
               OpLoopMerge %47 %48 None
               OpBranch %49
         %49 = OpLabel
         %50 = OpLoad %6 %44
         %51 = OpSLessThan %21 %50 %20
               OpBranchConditional %51 %52 %47
         %52 = OpLabel
         %53 = OpLoad %6 %43
               OpBranch %101
        %101 = OpLabel
         %54 = OpIAdd %6 %53 %24
               OpStore %43 %54
               OpBranch %48
         %48 = OpLabel
         %55 = OpLoad %6 %44
         %56 = OpIAdd %6 %55 %24
               OpStore %44 %56
               OpBranch %46
         %47 = OpLabel
         %57 = OpLoad %6 %43
               OpStore %45 %57
         %40 = OpLoad %6 %45
         %41 = OpLoad %6 %31
         %42 = OpIAdd %6 %41 %40
               OpStore %31 %42
               OpBranch %32
         %34 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  const uint32_t outer_loop_merge = 34;
  const uint32_t outer_loop_block = 33;
  const uint32_t inner_loop_merge = 47;
  const uint32_t inner_loop_block = 52;

  // Some inapplicable cases
  ASSERT_FALSE(
      TransformationAddDeadBreak(inner_loop_block, outer_loop_merge, true, {})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      TransformationAddDeadBreak(outer_loop_block, inner_loop_merge, true, {})
          .IsApplicable(context.get(), fact_manager));

  auto transformation1 =
      TransformationAddDeadBreak(inner_loop_block, inner_loop_merge, true, {});
  auto transformation2 =
      TransformationAddDeadBreak(outer_loop_block, outer_loop_merge, true, {});

  ASSERT_TRUE(transformation1.IsApplicable(context.get(), fact_manager));
  transformation1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation2.IsApplicable(context.get(), fact_manager));
  transformation2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %31 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFunction %6
         %10 = OpTypePointer Function %6
         %12 = OpConstant %6 0
         %20 = OpConstant %6 10
         %21 = OpTypeBool
        %100 = OpConstantTrue %21
         %24 = OpConstant %6 1
         %38 = OpConstant %6 100
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %43 = OpVariable %10 Function
         %44 = OpVariable %10 Function
         %45 = OpVariable %10 Function
         %31 = OpVariable %10 Function
               OpStore %31 %12
               OpBranch %32
         %32 = OpLabel
               OpLoopMerge %34 %35 None
               OpBranch %36
         %36 = OpLabel
         %37 = OpLoad %6 %31
         %39 = OpSLessThan %21 %37 %38
               OpBranchConditional %39 %33 %34
         %33 = OpLabel
               OpBranchConditional %100 %35 %34
         %35 = OpLabel
               OpStore %43 %12
               OpStore %44 %12
               OpBranch %46
         %46 = OpLabel
               OpLoopMerge %47 %48 None
               OpBranch %49
         %49 = OpLabel
         %50 = OpLoad %6 %44
         %51 = OpSLessThan %21 %50 %20
               OpBranchConditional %51 %52 %47
         %52 = OpLabel
         %53 = OpLoad %6 %43
               OpBranchConditional %100 %101 %47
        %101 = OpLabel
         %54 = OpIAdd %6 %53 %24
               OpStore %43 %54
               OpBranch %48
         %48 = OpLabel
         %55 = OpLoad %6 %44
         %56 = OpIAdd %6 %55 %24
               OpStore %44 %56
               OpBranch %46
         %47 = OpLabel
         %57 = OpLoad %6 %43
               OpStore %45 %57
         %40 = OpLoad %6 %45
         %41 = OpLoad %6 %31
         %42 = OpIAdd %6 %41 %40
               OpStore %31 %42
               OpBranch %32
         %34 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadBreakTest, PhiInstructions) {
  // Checks that the transformation works in the presence of phi instructions.

  // The SPIR-V for this test is adapted from the following GLSL, with a bit of
  // extra and artificial work to get some interesting uses of OpPhi:
  //
  // void main() {
  //   int x; int y;
  //   float f;
  //   x = 2;
  //   f = 3.0;
  //   if (x > y) {
  //     x = 3;
  //     f = 4.0;
  //   } else {
  //     x = x + 2;
  //     f = f + 10.0;
  //   }
  //   while (x < y) {
  //     x = x + 1;
  //     f = f + 1.0;
  //   }
  //   y = x;
  //   f = f + 3.0;
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %12 "f"
               OpName %15 "y"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %10 = OpTypeFloat 32
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 3
         %17 = OpTypeBool
         %80 = OpConstantTrue %17
         %21 = OpConstant %6 3
         %22 = OpConstant %10 4
         %27 = OpConstant %10 10
         %38 = OpConstant %6 1
         %41 = OpConstant %10 1
         %46 = OpUndef %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %15 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %12 %13
         %18 = OpSGreaterThan %17 %9 %46
               OpSelectionMerge %20 None
               OpBranchConditional %18 %19 %23
         %19 = OpLabel
               OpStore %8 %21
               OpStore %12 %22
               OpBranch %20
         %23 = OpLabel
         %25 = OpIAdd %6 %9 %9
               OpStore %8 %25
               OpBranch %70
         %70 = OpLabel
         %28 = OpFAdd %10 %13 %27
               OpStore %12 %28
               OpBranch %20
         %20 = OpLabel
         %52 = OpPhi %10 %22 %19 %28 %70
         %48 = OpPhi %6 %21 %19 %25 %70
               OpBranch %29
         %29 = OpLabel
         %51 = OpPhi %10 %52 %20 %42 %32
         %47 = OpPhi %6 %48 %20 %39 %32
               OpLoopMerge %31 %32 None
               OpBranch %33
         %33 = OpLabel
         %36 = OpSLessThan %17 %47 %46
               OpBranchConditional %36 %30 %31
         %30 = OpLabel
         %39 = OpIAdd %6 %47 %38
               OpStore %8 %39
               OpBranch %75
         %75 = OpLabel
         %42 = OpFAdd %10 %51 %41
               OpStore %12 %42
               OpBranch %32
         %32 = OpLabel
               OpBranch %29
         %31 = OpLabel
         %71 = OpPhi %6 %47 %33
         %72 = OpPhi %10 %51 %33
               OpStore %15 %71
         %45 = OpFAdd %10 %72 %13
               OpStore %12 %45
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // Some inapplicable transformations
  // Not applicable because there is already an edge 19->20, so the OpPhis at 20
  // do not need to be updated
  ASSERT_FALSE(TransformationAddDeadBreak(19, 20, true, {13, 21})
                   .IsApplicable(context.get(), fact_manager));
  // Not applicable because two OpPhis (not zero) need to be updated at 20
  ASSERT_FALSE(TransformationAddDeadBreak(23, 20, true, {})
                   .IsApplicable(context.get(), fact_manager));
  // Not applicable because two OpPhis (not just one) need to be updated at 20
  ASSERT_FALSE(TransformationAddDeadBreak(23, 20, true, {13})
                   .IsApplicable(context.get(), fact_manager));
  // Not applicable because the given ids do not have types that match the
  // OpPhis at 20, in order
  ASSERT_FALSE(TransformationAddDeadBreak(23, 20, true, {21, 13})
                   .IsApplicable(context.get(), fact_manager));
  // Not applicable because id 23 is a label
  ASSERT_FALSE(TransformationAddDeadBreak(23, 20, true, {21, 23})
                   .IsApplicable(context.get(), fact_manager));
  // Not applicable because 101 is not an id
  ASSERT_FALSE(TransformationAddDeadBreak(23, 20, true, {21, 101})
                   .IsApplicable(context.get(), fact_manager));
  // Not applicable because ids 51 and 47 are not available at the end of block
  // 23
  ASSERT_FALSE(TransformationAddDeadBreak(23, 20, true, {51, 47})
                   .IsApplicable(context.get(), fact_manager));

  // Not applicable because OpConstantFalse is not present in the module
  ASSERT_FALSE(TransformationAddDeadBreak(19, 20, false, {})
                   .IsApplicable(context.get(), fact_manager));

  auto transformation1 = TransformationAddDeadBreak(19, 20, true, {});
  auto transformation2 = TransformationAddDeadBreak(23, 20, true, {13, 21});
  auto transformation3 = TransformationAddDeadBreak(70, 20, true, {});
  auto transformation4 = TransformationAddDeadBreak(30, 31, true, {21, 13});
  auto transformation5 = TransformationAddDeadBreak(75, 31, true, {47, 51});

  ASSERT_TRUE(transformation1.IsApplicable(context.get(), fact_manager));
  transformation1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation2.IsApplicable(context.get(), fact_manager));
  transformation2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation3.IsApplicable(context.get(), fact_manager));
  transformation3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation4.IsApplicable(context.get(), fact_manager));
  transformation4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation5.IsApplicable(context.get(), fact_manager));
  transformation5.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %12 "f"
               OpName %15 "y"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %10 = OpTypeFloat 32
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 3
         %17 = OpTypeBool
         %80 = OpConstantTrue %17
         %21 = OpConstant %6 3
         %22 = OpConstant %10 4
         %27 = OpConstant %10 10
         %38 = OpConstant %6 1
         %41 = OpConstant %10 1
         %46 = OpUndef %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %15 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %12 %13
         %18 = OpSGreaterThan %17 %9 %46
               OpSelectionMerge %20 None
               OpBranchConditional %18 %19 %23
         %19 = OpLabel
               OpStore %8 %21
               OpStore %12 %22
               OpBranchConditional %80 %20 %20
         %23 = OpLabel
         %25 = OpIAdd %6 %9 %9
               OpStore %8 %25
               OpBranchConditional %80 %70 %20
         %70 = OpLabel
         %28 = OpFAdd %10 %13 %27
               OpStore %12 %28
               OpBranchConditional %80 %20 %20
         %20 = OpLabel
         %52 = OpPhi %10 %22 %19 %28 %70 %13 %23
         %48 = OpPhi %6 %21 %19 %25 %70 %21 %23
               OpBranch %29
         %29 = OpLabel
         %51 = OpPhi %10 %52 %20 %42 %32
         %47 = OpPhi %6 %48 %20 %39 %32
               OpLoopMerge %31 %32 None
               OpBranch %33
         %33 = OpLabel
         %36 = OpSLessThan %17 %47 %46
               OpBranchConditional %36 %30 %31
         %30 = OpLabel
         %39 = OpIAdd %6 %47 %38
               OpStore %8 %39
               OpBranchConditional %80 %75 %31
         %75 = OpLabel
         %42 = OpFAdd %10 %51 %41
               OpStore %12 %42
               OpBranchConditional %80 %32 %31
         %32 = OpLabel
               OpBranch %29
         %31 = OpLabel
         %71 = OpPhi %6 %47 %33 %21 %30 %47 %75
         %72 = OpPhi %10 %51 %33 %13 %30 %51 %75
               OpStore %15 %71
         %45 = OpFAdd %10 %72 %13
               OpStore %12 %45
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadBreakTest, RespectDominanceRules1) {
  // Right after the loop, an OpCopyObject defined by the loop is used.  Adding
  // a dead break would prevent that use from being dominated by its definition,
  // so is not allowed.

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
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %102 None
               OpBranch %103
        %103 = OpLabel
        %200 = OpCopyObject %10 %11
               OpBranch %104
        %104 = OpLabel
               OpBranch %102
        %102 = OpLabel
               OpBranchConditional %11 %100 %101
        %101 = OpLabel
        %201 = OpCopyObject %10 %200
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto bad_transformation = TransformationAddDeadBreak(100, 101, false, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadBreakTest, RespectDominanceRules2) {
  // This example captures the following idiom:
  //
  //   if {
  // L1:
  //   }
  //   definition;
  // L2:
  //   use;
  //
  // Adding a dead jump from L1 to L2 would lead to 'definition' no longer
  // dominating 'use', and so is not allowed.

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
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpSelectionMerge %101 None
               OpBranchConditional %11 %102 %103
        %102 = OpLabel
               OpBranch %103
        %103 = OpLabel
        %200 = OpCopyObject %10 %11
               OpBranch %101
        %101 = OpLabel
        %201 = OpCopyObject %10 %200
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto bad_transformation = TransformationAddDeadBreak(102, 101, false, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadBreakTest, RespectDominanceRules3) {
  // Right after the loop, an OpCopyObject defined by the loop is used in an
  // OpPhi. Adding a dead break is OK in this case, due to the use being in an
  // OpPhi.

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
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %102 None
               OpBranch %103
        %103 = OpLabel
        %200 = OpCopyObject %10 %11
               OpBranch %104
        %104 = OpLabel
               OpBranch %102
        %102 = OpLabel
               OpBranchConditional %11 %100 %101
        %101 = OpLabel
        %201 = OpPhi %10 %200 %102
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto good_transformation = TransformationAddDeadBreak(100, 101, false, {11});
  ASSERT_TRUE(good_transformation.IsApplicable(context.get(), fact_manager));

  good_transformation.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %102 None
               OpBranchConditional %11 %101 %103
        %103 = OpLabel
        %200 = OpCopyObject %10 %11
               OpBranch %104
        %104 = OpLabel
               OpBranch %102
        %102 = OpLabel
               OpBranchConditional %11 %100 %101
        %101 = OpLabel
        %201 = OpPhi %10 %200 %102 %11 %100
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadBreakTest, RespectDominanceRules4) {
  // This example captures the following idiom:
  //
  //   if {
  // L1:
  //   }
  //   definition;
  // L2:
  //   use in OpPhi;
  //
  // Adding a dead jump from L1 to L2 is OK, due to 'use' being in an OpPhi.

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
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpSelectionMerge %101 None
               OpBranchConditional %11 %102 %103
        %102 = OpLabel
               OpBranch %103
        %103 = OpLabel
        %200 = OpCopyObject %10 %11
               OpBranch %101
        %101 = OpLabel
        %201 = OpPhi %10 %200 %103
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto good_transformation = TransformationAddDeadBreak(102, 101, false, {11});
  ASSERT_TRUE(good_transformation.IsApplicable(context.get(), fact_manager));

  good_transformation.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpSelectionMerge %101 None
               OpBranchConditional %11 %102 %103
        %102 = OpLabel
               OpBranchConditional %11 %101 %103
        %103 = OpLabel
        %200 = OpCopyObject %10 %11
               OpBranch %101
        %101 = OpLabel
        %201 = OpPhi %10 %200 %103 %11 %102
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadBreakTest, RespectDominanceRules5) {
  // After, but not right after, the loop, an OpCopyObject defined by the loop
  // is used in an OpPhi. Adding a dead break is not OK in this case.

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
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %102 None
               OpBranch %103
        %103 = OpLabel
        %200 = OpCopyObject %10 %11
               OpBranch %104
        %104 = OpLabel
               OpBranch %102
        %102 = OpLabel
               OpBranchConditional %11 %100 %101
        %101 = OpLabel
               OpBranch %105
        %105 = OpLabel
        %201 = OpPhi %10 %200 %101
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto bad_transformation = TransformationAddDeadBreak(100, 101, false, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadBreakTest, RespectDominanceRules6) {
  // This example captures the following idiom:
  //
  //   if {
  // L1:
  //   }
  //   definition;
  // L2:
  //   goto L3;
  // L3:
  //   use in OpPhi;
  //
  // Adding a dead jump from L1 to L2 not OK, due to the use in an OpPhi being
  // in L3.

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
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpSelectionMerge %101 None
               OpBranchConditional %11 %102 %103
        %102 = OpLabel
               OpBranch %103
        %103 = OpLabel
        %200 = OpCopyObject %10 %11
               OpBranch %101
        %101 = OpLabel
               OpBranch %150
        %150 = OpLabel
        %201 = OpPhi %10 %200 %101
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto bad_transformation = TransformationAddDeadBreak(102, 101, false, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadBreakTest, RespectDominanceRules7) {
  // This example - a variation on an earlier test - captures the following
  // idiom:
  //
  //   loop {
  // L1:
  //   }
  //   definition;
  // L2:
  //   use;
  //
  // Adding a dead jump from L1 to L2 would lead to 'definition' no longer
  // dominating 'use', and so is not allowed.
  //
  // This version of the test captures the case where L1 appears after the
  // loop merge (which SPIR-V dominance rules allow).

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
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %104 None
               OpBranchConditional %11 %102 %103
        %103 = OpLabel
        %200 = OpCopyObject %10 %11
               OpBranch %101
        %101 = OpLabel
        %201 = OpCopyObject %10 %200
               OpReturn
        %102 = OpLabel
               OpBranch %103
        %104 = OpLabel
               OpBranch %100
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto bad_transformation = TransformationAddDeadBreak(102, 101, false, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadBreakTest, RespectDominanceRules8) {
  // A variation of RespectDominanceRules8 where the defining block appears
  // in the loop, but after the definition of interest.

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
         %10 = OpTypeBool
         %11 = OpConstantFalse %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %104 None
               OpBranchConditional %11 %102 %103
        %103 = OpLabel
        %200 = OpCopyObject %10 %11
               OpBranch %101
        %102 = OpLabel
               OpBranch %103
        %101 = OpLabel
        %201 = OpCopyObject %10 %200
               OpReturn
        %104 = OpLabel
               OpBranch %100
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto bad_transformation = TransformationAddDeadBreak(102, 101, false, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadBreakTest,
     BreakWouldDisobeyDominanceBlockOrderingRules) {
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
          %9 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %16 %15 None
               OpBranch %11
         %11 = OpLabel
               OpSelectionMerge %14 None
               OpBranchConditional %9 %12 %13
         %14 = OpLabel
               OpBranch %15
         %12 = OpLabel
               OpBranch %16
         %13 = OpLabel
               OpBranch %16
         %15 = OpLabel
               OpBranch %10
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // Bad because 14 comes before 12 in the module, and 14 has no predecessors.
  // This means that an edge from 12 to 14 will lead to 12 dominating 14, which
  // is illegal if 12 appears after 14.
  auto bad_transformation = TransformationAddDeadBreak(12, 14, true, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
