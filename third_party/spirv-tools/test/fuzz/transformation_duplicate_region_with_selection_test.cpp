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

#include "source/fuzz/transformation_duplicate_region_with_selection.h"

#include "gtest/gtest.h"
#include "source/fuzz/counter_overflow_id_source.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationDuplicateRegionWithSelectionTest, BasicUseTest) {
  // This test handles a case where the ids from the original region are used in
  // subsequent block.

  std::string shader = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "b"
               OpName %18 "c"
               OpName %20 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %14 = OpConstant %6 2
         %16 = OpTypeBool
         %17 = OpTypePointer Function %16
         %19 = OpConstantTrue %16
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %18 = OpVariable %17 Function
         %20 = OpVariable %7 Function
               OpStore %18 %19
               OpStore %20 %14
         %21 = OpFunctionCall %2 %10 %20
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
               OpBranch %800
        %800 = OpLabel
         %13 = OpLoad %6 %9
         %15 = OpIAdd %6 %13 %14
               OpStore %12 %15
               OpBranch %900
         %900 = OpLabel
         %901 = OpIAdd %6 %15 %13
         %902 = OpISub %6 %13 %15
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
  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(
          500, 19, 501, 800, 800, {{800, 100}}, {{13, 201}, {15, 202}},
          {{13, 301}, {15, 302}});

  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  std::string expected_shader = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "b"
               OpName %18 "c"
               OpName %20 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %14 = OpConstant %6 2
         %16 = OpTypeBool
         %17 = OpTypePointer Function %16
         %19 = OpConstantTrue %16
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %18 = OpVariable %17 Function
         %20 = OpVariable %7 Function
               OpStore %18 %19
               OpStore %20 %14
         %21 = OpFunctionCall %2 %10 %20
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
               OpBranch %500
        %500 = OpLabel
               OpSelectionMerge %501 None
               OpBranchConditional %19 %800 %100
        %800 = OpLabel
         %13 = OpLoad %6 %9
         %15 = OpIAdd %6 %13 %14
               OpStore %12 %15
               OpBranch %501
        %100 = OpLabel
        %201 = OpLoad %6 %9
        %202 = OpIAdd %6 %201 %14
               OpStore %12 %202
               OpBranch %501
        %501 = OpLabel
        %301 = OpPhi %6 %13 %800 %201 %100
        %302 = OpPhi %6 %15 %800 %202 %100
               OpBranch %900
        %900 = OpLabel
        %901 = OpIAdd %6 %302 %301
        %902 = OpISub %6 %301 %302
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest, BasicExitBlockTest) {
  // This test handles a case where the exit block of the region is the exit
  // block of the containing function.

  std::string shader = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "b"
               OpName %18 "c"
               OpName %20 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %14 = OpConstant %6 2
         %16 = OpTypeBool
         %17 = OpTypePointer Function %16
         %19 = OpConstantTrue %16
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %18 = OpVariable %17 Function
         %20 = OpVariable %7 Function
               OpStore %18 %19
               OpStore %20 %14
         %21 = OpFunctionCall %2 %10 %20
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
               OpBranch %800
        %800 = OpLabel
         %13 = OpLoad %6 %9
         %15 = OpIAdd %6 %13 %14
               OpStore %12 %15
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
  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(
          500, 19, 501, 800, 800, {{800, 100}}, {{13, 201}, {15, 202}},
          {{13, 301}, {15, 302}});

  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
   OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "b"
               OpName %18 "c"
               OpName %20 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %14 = OpConstant %6 2
         %16 = OpTypeBool
         %17 = OpTypePointer Function %16
         %19 = OpConstantTrue %16
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %18 = OpVariable %17 Function
         %20 = OpVariable %7 Function
               OpStore %18 %19
               OpStore %20 %14
         %21 = OpFunctionCall %2 %10 %20
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
               OpBranch %500
        %500 = OpLabel
               OpSelectionMerge %501 None
               OpBranchConditional %19 %800 %100
        %800 = OpLabel
         %13 = OpLoad %6 %9
         %15 = OpIAdd %6 %13 %14
               OpStore %12 %15
               OpBranch %501
        %100 = OpLabel
        %201 = OpLoad %6 %9
        %202 = OpIAdd %6 %201 %14
               OpStore %12 %202
               OpBranch %501
        %501 = OpLabel
        %301 = OpPhi %6 %13 %800 %201 %100
        %302 = OpPhi %6 %15 %800 %202 %100
               OpReturn
               OpFunctionEnd

  )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest, NotApplicableCFGTest) {
  // This test handles few cases where the transformation is not applicable
  // because of the control flow graph or layout of the blocks.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %18 "b"
               OpName %25 "c"
               OpName %27 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 2
         %14 = OpTypeBool
         %24 = OpTypePointer Function %14
         %26 = OpConstantTrue %14
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %25 = OpVariable %24 Function
         %27 = OpVariable %7 Function
               OpStore %25 %26
               OpStore %27 %13
         %28 = OpFunctionCall %2 %10 %27
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %18 = OpVariable %7 Function
         %12 = OpLoad %6 %9
         %15 = OpSLessThan %14 %12 %13
               OpSelectionMerge %17 None
               OpBranchConditional %15 %16 %21
         %16 = OpLabel
         %19 = OpLoad %6 %9
         %20 = OpIAdd %6 %19 %13
               OpStore %18 %20
               OpBranch %17
         %21 = OpLabel
         %22 = OpLoad %6 %9
         %23 = OpISub %6 %22 %13
               OpStore %18 %23
               OpBranch %17
         %17 = OpLabel
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
  // Bad: |entry_block_id| refers to the entry block of the function (this
  // transformation currently avoids such cases).
  TransformationDuplicateRegionWithSelection transformation_bad_1 =
      TransformationDuplicateRegionWithSelection(
          500, 26, 501, 11, 11, {{11, 100}}, {{18, 201}, {12, 202}, {15, 203}},
          {{18, 301}, {12, 302}, {15, 303}});
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Bad: The block with id 16 does not dominate the block with id 21.
  TransformationDuplicateRegionWithSelection transformation_bad_2 =
      TransformationDuplicateRegionWithSelection(
          500, 26, 501, 16, 21, {{16, 100}, {21, 101}},
          {{19, 201}, {20, 202}, {22, 203}, {23, 204}},
          {{19, 301}, {20, 302}, {22, 303}, {23, 304}});
  ASSERT_FALSE(
      transformation_bad_2.IsApplicable(context.get(), transformation_context));

  // Bad: The block with id 21 does not post-dominate the block with id 11.
  TransformationDuplicateRegionWithSelection transformation_bad_3 =
      TransformationDuplicateRegionWithSelection(
          500, 26, 501, 11, 21, {{11, 100}, {21, 101}},
          {{18, 201}, {12, 202}, {15, 203}, {22, 204}, {23, 205}},
          {{18, 301}, {12, 302}, {15, 303}, {22, 304}, {23, 305}});
  ASSERT_FALSE(
      transformation_bad_3.IsApplicable(context.get(), transformation_context));

  // Bad: The block with id 5 is contained in a different function than the
  // block with id 11.
  TransformationDuplicateRegionWithSelection transformation_bad_4 =
      TransformationDuplicateRegionWithSelection(
          500, 26, 501, 5, 11, {{5, 100}, {11, 101}},
          {{25, 201}, {27, 202}, {28, 203}, {18, 204}, {12, 205}, {15, 206}},
          {{25, 301}, {27, 302}, {28, 303}, {18, 304}, {12, 305}, {15, 306}});
  ASSERT_FALSE(
      transformation_bad_4.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationDuplicateRegionWithSelectionTest, NotApplicableIdTest) {
  // This test handles a case where the supplied ids are either not fresh, not
  // distinct, not valid in their context or do not refer to the existing
  // instructions.

  std::string shader = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "b"
               OpName %18 "c"
               OpName %20 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %14 = OpConstant %6 2
         %16 = OpTypeBool
         %17 = OpTypePointer Function %16
         %19 = OpConstantTrue %16
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %18 = OpVariable %17 Function
         %20 = OpVariable %7 Function
               OpStore %18 %19
               OpStore %20 %14
         %21 = OpFunctionCall %2 %10 %20
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
               OpBranch %800
        %800 = OpLabel
         %13 = OpLoad %6 %9
         %15 = OpIAdd %6 %13 %14
               OpStore %12 %15
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
  // Bad: A value in the |original_label_to_duplicate_label| is not a fresh id.
  TransformationDuplicateRegionWithSelection transformation_bad_1 =
      TransformationDuplicateRegionWithSelection(
          500, 19, 501, 800, 800, {{800, 21}}, {{13, 201}, {15, 202}},
          {{13, 301}, {15, 302}});

  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Bad: Values in the |original_id_to_duplicate_id| are not distinct.
  TransformationDuplicateRegionWithSelection transformation_bad_2 =
      TransformationDuplicateRegionWithSelection(
          500, 19, 501, 800, 800, {{800, 100}}, {{13, 201}, {15, 201}},
          {{13, 301}, {15, 302}});
  ASSERT_FALSE(
      transformation_bad_2.IsApplicable(context.get(), transformation_context));

  // Bad: Values in the |original_id_to_phi_id| are not fresh and are not
  // distinct with previous values.
  TransformationDuplicateRegionWithSelection transformation_bad_3 =
      TransformationDuplicateRegionWithSelection(
          500, 19, 501, 800, 800, {{800, 100}}, {{13, 201}, {15, 202}},
          {{13, 18}, {15, 202}});
  ASSERT_FALSE(
      transformation_bad_3.IsApplicable(context.get(), transformation_context));

  // Bad: |entry_block_id| does not refer to an existing instruction.
  TransformationDuplicateRegionWithSelection transformation_bad_4 =
      TransformationDuplicateRegionWithSelection(
          500, 19, 501, 802, 800, {{800, 100}}, {{13, 201}, {15, 202}},
          {{13, 301}, {15, 302}});
  ASSERT_FALSE(
      transformation_bad_4.IsApplicable(context.get(), transformation_context));

  // Bad: |exit_block_id| does not refer to a block.
  TransformationDuplicateRegionWithSelection transformation_bad_5 =
      TransformationDuplicateRegionWithSelection(
          500, 19, 501, 800, 9, {{800, 100}}, {{13, 201}, {15, 202}},
          {{13, 301}, {15, 302}});
  ASSERT_FALSE(
      transformation_bad_5.IsApplicable(context.get(), transformation_context));

  // Bad: |new_entry_fresh_id| is not fresh.
  TransformationDuplicateRegionWithSelection transformation_bad_6 =
      TransformationDuplicateRegionWithSelection(
          20, 19, 501, 800, 800, {{800, 100}}, {{13, 201}, {15, 202}},
          {{13, 301}, {15, 302}});
  ASSERT_FALSE(
      transformation_bad_6.IsApplicable(context.get(), transformation_context));

  // Bad: |merge_label_fresh_id| is not fresh.
  TransformationDuplicateRegionWithSelection transformation_bad_7 =
      TransformationDuplicateRegionWithSelection(
          500, 19, 20, 800, 800, {{800, 100}}, {{13, 201}, {15, 202}},
          {{13, 301}, {15, 302}});
  ASSERT_FALSE(
      transformation_bad_7.IsApplicable(context.get(), transformation_context));

#ifndef NDEBUG
  // Bad: Instruction with id 15 is from the original region and is available
  // at the end of the region but it is not present in the
  // |original_id_to_phi_id|.
  TransformationDuplicateRegionWithSelection transformation_bad_8 =
      TransformationDuplicateRegionWithSelection(
          500, 19, 501, 800, 800, {{800, 100}}, {{13, 201}, {15, 202}},
          {{13, 301}});
  ASSERT_DEATH(
      transformation_bad_8.IsApplicable(context.get(), transformation_context),
      "Bad attempt to query whether overflow ids are available.");

  // Bad: Instruction with id 15 is from the original region but it is
  // not present in the |original_id_to_duplicate_id|.
  TransformationDuplicateRegionWithSelection transformation_bad_9 =
      TransformationDuplicateRegionWithSelection(500, 19, 501, 800, 800,
                                                 {{800, 100}}, {{13, 201}},
                                                 {{13, 301}, {15, 302}});
  ASSERT_DEATH(
      transformation_bad_9.IsApplicable(context.get(), transformation_context),
      "Bad attempt to query whether overflow ids are available.");
#endif

  // Bad: |condition_id| does not refer to the valid instruction.
  TransformationDuplicateRegionWithSelection transformation_bad_10 =
      TransformationDuplicateRegionWithSelection(
          500, 200, 501, 800, 800, {{800, 100}}, {{13, 201}, {15, 202}},
          {{13, 301}, {15, 302}});

  ASSERT_FALSE(transformation_bad_10.IsApplicable(context.get(),
                                                  transformation_context));

  // Bad: |condition_id| does not refer to the instruction of type OpTypeBool
  TransformationDuplicateRegionWithSelection transformation_bad_11 =
      TransformationDuplicateRegionWithSelection(
          500, 14, 501, 800, 800, {{800, 100}}, {{13, 201}, {15, 202}},
          {{13, 301}, {15, 302}});

  ASSERT_FALSE(transformation_bad_11.IsApplicable(context.get(),
                                                  transformation_context));
}

TEST(TransformationDuplicateRegionWithSelectionTest, NotApplicableCFGTest2) {
  // This test handles few cases where the transformation is not applicable
  // because of the control flow graph or the layout of the blocks.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun("
               OpName %10 "s"
               OpName %12 "i"
               OpName %29 "b"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %19 = OpConstant %8 10
         %20 = OpTypeBool
         %26 = OpConstant %8 1
         %28 = OpTypePointer Function %20
         %30 = OpConstantTrue %20
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %29 = OpVariable %28 Function
               OpStore %29 %30
         %31 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
         %12 = OpVariable %9 Function
               OpStore %10 %11
               OpStore %12 %11
               OpBranch %13
         %13 = OpLabel
               OpLoopMerge %15 %16 None
               OpBranch %17
         %17 = OpLabel
         %18 = OpLoad %8 %12
         %21 = OpSLessThan %20 %18 %19
               OpBranchConditional %21 %14 %15
         %14 = OpLabel
         %22 = OpLoad %8 %10
         %23 = OpLoad %8 %12
         %24 = OpIAdd %8 %22 %23
               OpStore %10 %24
               OpBranch %16
         %16 = OpLabel
               OpBranch %50
         %50 = OpLabel
         %25 = OpLoad %8 %12
         %27 = OpIAdd %8 %25 %26
               OpStore %12 %27
               OpBranch %13
         %15 = OpLabel
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
  // Bad: The exit block cannot be a header of a loop, because the region won't
  // be a single-entry, single-exit region.
  TransformationDuplicateRegionWithSelection transformation_bad_1 =
      TransformationDuplicateRegionWithSelection(500, 30, 501, 13, 13,
                                                 {{13, 100}}, {{}}, {{}});
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Bad: The block with id 13, the loop header, is in the region. The block
  // with id 15, the loop merge block, is not in the region.
  TransformationDuplicateRegionWithSelection transformation_bad_2 =
      TransformationDuplicateRegionWithSelection(
          500, 30, 501, 13, 17, {{13, 100}, {17, 101}}, {{18, 201}, {21, 202}},
          {{18, 301}, {21, 302}});
  ASSERT_FALSE(
      transformation_bad_2.IsApplicable(context.get(), transformation_context));

  // Bad: The block with id 13, the loop header, is not in the region. The block
  // with id 16, the loop continue target, is in the region.
  TransformationDuplicateRegionWithSelection transformation_bad_3 =
      TransformationDuplicateRegionWithSelection(
          500, 30, 501, 16, 50, {{16, 100}, {50, 101}}, {{25, 201}, {27, 202}},
          {{25, 301}, {27, 302}});
  ASSERT_FALSE(
      transformation_bad_3.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationDuplicateRegionWithSelectionTest, NotApplicableCFGTest3) {
  // This test handles a case where for the block which is not the exit block,
  // not all successors are in the region.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun("
               OpName %14 "a"
               OpName %19 "b"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeBool
          %9 = OpConstantTrue %8
         %12 = OpTypeInt 32 1
         %13 = OpTypePointer Function %12
         %15 = OpConstant %12 2
         %17 = OpConstant %12 3
         %18 = OpTypePointer Function %8
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %19 = OpVariable %18 Function
               OpStore %19 %9
         %20 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %14 = OpVariable %13 Function
               OpSelectionMerge %11 None
               OpBranchConditional %9 %10 %16
         %10 = OpLabel
               OpStore %14 %15
               OpBranch %11
         %16 = OpLabel
               OpStore %14 %17
               OpBranch %11
         %11 = OpLabel
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
  // Bad: The block with id 7, which is not an exit block, has two successors:
  // the block with id 10 and the block with id 16. The block with id 16 is not
  // in the region.
  TransformationDuplicateRegionWithSelection transformation_bad_1 =
      TransformationDuplicateRegionWithSelection(
          500, 30, 501, 7, 10, {{13, 100}}, {{14, 201}}, {{14, 301}});
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationDuplicateRegionWithSelectionTest, MultipleBlocksLoopTest) {
  // This test handles a case where the region consists of multiple blocks
  // (they form a loop). The transformation is applicable and the region is
  // duplicated.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun("
               OpName %10 "s"
               OpName %12 "i"
               OpName %29 "b"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %19 = OpConstant %8 10
         %20 = OpTypeBool
         %26 = OpConstant %8 1
         %28 = OpTypePointer Function %20
         %30 = OpConstantTrue %20
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %29 = OpVariable %28 Function
               OpStore %29 %30
         %31 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
         %12 = OpVariable %9 Function
               OpStore %10 %11
               OpStore %12 %11
               OpBranch %50
         %50 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpLoopMerge %15 %16 None
               OpBranch %17
         %17 = OpLabel
         %18 = OpLoad %8 %12
         %21 = OpSLessThan %20 %18 %19
               OpBranchConditional %21 %14 %15
         %14 = OpLabel
         %22 = OpLoad %8 %10
         %23 = OpLoad %8 %12
         %24 = OpIAdd %8 %22 %23
               OpStore %10 %24
               OpBranch %16
         %16 = OpLabel
         %25 = OpLoad %8 %12
         %27 = OpIAdd %8 %25 %26
               OpStore %12 %27
               OpBranch %13
         %15 = OpLabel
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
  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(
          500, 30, 501, 50, 15,
          {{50, 100}, {13, 101}, {14, 102}, {15, 103}, {16, 104}, {17, 105}},
          {{22, 201},
           {23, 202},
           {24, 203},
           {25, 204},
           {27, 205},
           {18, 206},
           {21, 207}},
          {{22, 301},
           {23, 302},
           {24, 303},
           {25, 304},
           {27, 305},
           {18, 306},
           {21, 307}});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
                 OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun("
               OpName %10 "s"
               OpName %12 "i"
               OpName %29 "b"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %19 = OpConstant %8 10
         %20 = OpTypeBool
         %26 = OpConstant %8 1
         %28 = OpTypePointer Function %20
         %30 = OpConstantTrue %20
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %29 = OpVariable %28 Function
               OpStore %29 %30
         %31 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
         %12 = OpVariable %9 Function
               OpStore %10 %11
               OpStore %12 %11
               OpBranch %500
        %500 = OpLabel
               OpSelectionMerge %501 None
               OpBranchConditional %30 %50 %100
         %50 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpLoopMerge %15 %16 None
               OpBranch %17
         %17 = OpLabel
         %18 = OpLoad %8 %12
         %21 = OpSLessThan %20 %18 %19
               OpBranchConditional %21 %14 %15
         %14 = OpLabel
         %22 = OpLoad %8 %10
         %23 = OpLoad %8 %12
         %24 = OpIAdd %8 %22 %23
               OpStore %10 %24
               OpBranch %16
         %16 = OpLabel
         %25 = OpLoad %8 %12
         %27 = OpIAdd %8 %25 %26
               OpStore %12 %27
               OpBranch %13
         %15 = OpLabel
               OpBranch %501
        %100 = OpLabel
               OpBranch %101
        %101 = OpLabel
               OpLoopMerge %103 %104 None
               OpBranch %105
        %105 = OpLabel
        %206 = OpLoad %8 %12
        %207 = OpSLessThan %20 %206 %19
               OpBranchConditional %207 %102 %103
        %102 = OpLabel
        %201 = OpLoad %8 %10
        %202 = OpLoad %8 %12
        %203 = OpIAdd %8 %201 %202
               OpStore %10 %203
               OpBranch %104
        %104 = OpLabel
        %204 = OpLoad %8 %12
        %205 = OpIAdd %8 %204 %26
               OpStore %12 %205
               OpBranch %101
        %103 = OpLabel
               OpBranch %501
        %501 = OpLabel
        %306 = OpPhi %8 %18 %15 %206 %103
        %307 = OpPhi %20 %21 %15 %207 %103
               OpReturn
               OpFunctionEnd
    )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     ResolvingOpPhiExitBlockTest) {
  // This test handles a case where the region under the transformation is
  // referenced in OpPhi instructions. Since the new merge block becomes the
  // exit of the region, these OpPhi instructions need to be updated.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "s"
               OpName %26 "b"
               OpName %29 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 0
         %15 = OpConstant %6 2
         %16 = OpTypeBool
         %25 = OpTypePointer Function %16
         %27 = OpConstantTrue %16
         %28 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %26 = OpVariable %25 Function
         %29 = OpVariable %7 Function
               OpStore %26 %27
               OpStore %29 %28
         %30 = OpFunctionCall %2 %10 %29
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
               OpStore %12 %13
         %14 = OpLoad %6 %9
         %17 = OpSLessThan %16 %14 %15
               OpSelectionMerge %19 None
               OpBranchConditional %17 %18 %22
         %18 = OpLabel
         %20 = OpLoad %6 %9
         %21 = OpIAdd %6 %20 %15
               OpStore %12 %21
               OpBranch %19
         %22 = OpLabel
         %23 = OpLoad %6 %9
         %24 = OpIMul %6 %23 %15
               OpStore %12 %24
               OpBranch %19
         %19 = OpLabel
         %40 = OpPhi %6 %21 %18 %24 %22
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

  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(
          500, 27, 501, 22, 22, {{22, 100}}, {{23, 201}, {24, 202}},
          {{23, 301}, {24, 302}});

  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "s"
               OpName %26 "b"
               OpName %29 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 0
         %15 = OpConstant %6 2
         %16 = OpTypeBool
         %25 = OpTypePointer Function %16
         %27 = OpConstantTrue %16
         %28 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %26 = OpVariable %25 Function
         %29 = OpVariable %7 Function
               OpStore %26 %27
               OpStore %29 %28
         %30 = OpFunctionCall %2 %10 %29
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
               OpStore %12 %13
         %14 = OpLoad %6 %9
         %17 = OpSLessThan %16 %14 %15
               OpSelectionMerge %19 None
               OpBranchConditional %17 %18 %500
         %18 = OpLabel
         %20 = OpLoad %6 %9
         %21 = OpIAdd %6 %20 %15
               OpStore %12 %21
               OpBranch %19
        %500 = OpLabel
               OpSelectionMerge %501 None
               OpBranchConditional %27 %22 %100
         %22 = OpLabel
         %23 = OpLoad %6 %9
         %24 = OpIMul %6 %23 %15
               OpStore %12 %24
               OpBranch %501
        %100 = OpLabel
        %201 = OpLoad %6 %9
        %202 = OpIMul %6 %201 %15
               OpStore %12 %202
               OpBranch %501
        %501 = OpLabel
        %301 = OpPhi %6 %23 %22 %201 %100
        %302 = OpPhi %6 %24 %22 %202 %100
               OpBranch %19
         %19 = OpLabel
         %40 = OpPhi %6 %21 %18 %302 %501
               OpReturn
               OpFunctionEnd
    )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest, NotApplicableEarlyReturn) {
  // This test handles a case where one of the blocks has successor outside of
  // the region, which has an early return from the function, so that the
  // transformation is not applicable.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "s"
               OpName %27 "b"
               OpName %30 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 0
         %15 = OpConstant %6 2
         %16 = OpTypeBool
         %26 = OpTypePointer Function %16
         %28 = OpConstantTrue %16
         %29 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %27 = OpVariable %26 Function
         %30 = OpVariable %7 Function
               OpStore %27 %28
               OpStore %30 %29
         %31 = OpFunctionCall %2 %10 %30
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
               OpBranch %50
         %50 = OpLabel
               OpStore %12 %13
         %14 = OpLoad %6 %9
         %17 = OpSLessThan %16 %14 %15
               OpSelectionMerge %19 None
               OpBranchConditional %17 %18 %22
         %18 = OpLabel
         %20 = OpLoad %6 %9
         %21 = OpIAdd %6 %20 %15
               OpStore %12 %21
               OpBranch %19
         %22 = OpLabel
               OpReturn
         %19 = OpLabel
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
  // Bad: The block with id 50, which is the entry block, has two successors:
  // the block with id 18 and the block with id 22. The block 22 has an early
  // return from the function, so that the entry block is not post-dominated by
  // the exit block.
  TransformationDuplicateRegionWithSelection transformation_bad_1 =
      TransformationDuplicateRegionWithSelection(
          500, 28, 501, 50, 19, {{50, 100}, {18, 101}, {22, 102}, {19, 103}},
          {{14, 202}, {17, 203}, {20, 204}, {21, 205}},
          {{14, 302}, {17, 303}, {20, 304}, {21, 305}});
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     ResolvingOpPhiEntryBlockOnePredecessor) {
  // This test handles a case where the entry block has an OpPhi instruction
  // referring to its predecessor. After transformation, this instruction needs
  // to be updated.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "s"
               OpName %14 "t"
               OpName %20 "b"
               OpName %23 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 0
         %15 = OpConstant %6 2
         %18 = OpTypeBool
         %19 = OpTypePointer Function %18
         %21 = OpConstantTrue %18
         %22 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %19 Function
         %23 = OpVariable %7 Function
               OpStore %20 %21
               OpStore %23 %22
         %24 = OpFunctionCall %2 %10 %23
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
         %14 = OpVariable %7 Function
               OpStore %12 %13
         %16 = OpLoad %6 %12
         %17 = OpIMul %6 %15 %16
               OpStore %14 %17
               OpBranch %50
         %50 = OpLabel
         %51 = OpPhi %6 %17 %11
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

  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(
          500, 21, 501, 50, 50, {{50, 100}}, {{51, 201}}, {{51, 301}});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "s"
               OpName %14 "t"
               OpName %20 "b"
               OpName %23 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 0
         %15 = OpConstant %6 2
         %18 = OpTypeBool
         %19 = OpTypePointer Function %18
         %21 = OpConstantTrue %18
         %22 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %19 Function
         %23 = OpVariable %7 Function
               OpStore %20 %21
               OpStore %23 %22
         %24 = OpFunctionCall %2 %10 %23
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
         %14 = OpVariable %7 Function
               OpStore %12 %13
         %16 = OpLoad %6 %12
         %17 = OpIMul %6 %15 %16
               OpStore %14 %17
               OpBranch %500
        %500 = OpLabel
               OpSelectionMerge %501 None
               OpBranchConditional %21 %50 %100
         %50 = OpLabel
         %51 = OpPhi %6 %17 %500
               OpBranch %501
        %100 = OpLabel
        %201 = OpPhi %6 %17 %500
               OpBranch %501
        %501 = OpLabel
        %301 = OpPhi %6 %51 %50 %201 %100
               OpReturn
               OpFunctionEnd
    )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     NotApplicableNoVariablePointerCapability) {
  // This test handles a case where the transformation would create an OpPhi
  // instruction with pointer operands, however there is no cab
  // CapabilityVariablePointers. Hence, the transformation is not applicable.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "s"
               OpName %14 "t"
               OpName %20 "b"
               OpName %23 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 0
         %15 = OpConstant %6 2
         %18 = OpTypeBool
         %19 = OpTypePointer Function %18
         %21 = OpConstantTrue %18
         %22 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %19 Function
         %23 = OpVariable %7 Function
               OpStore %20 %21
               OpStore %23 %22
         %24 = OpFunctionCall %2 %10 %23
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
         %14 = OpVariable %7 Function
               OpStore %12 %13
         %16 = OpLoad %6 %12
         %17 = OpIMul %6 %15 %16
               OpStore %14 %17
               OpBranch %50
         %50 = OpLabel
         %51 = OpCopyObject %7 %12
               OpBranch %52
         %52 = OpLabel
         %53 = OpCopyObject %7 %51
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
  // Bad: There is no required capability CapabilityVariablePointers
  TransformationDuplicateRegionWithSelection transformation_bad_1 =
      TransformationDuplicateRegionWithSelection(
          500, 21, 501, 50, 50, {{50, 100}}, {{51, 201}}, {{51, 301}});
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     ExitBlockTerminatorOpUnreachable) {
  // This test handles a case where the exit block ends with OpUnreachable.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun("
               OpName %10 "s"
               OpName %17 "b"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %13 = OpConstant %8 2
         %15 = OpTypeBool
         %16 = OpTypePointer Function %15
         %18 = OpConstantTrue %15
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %17 = OpVariable %16 Function
               OpStore %17 %18
         %19 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpBranch %50
         %50 = OpLabel
               OpStore %10 %11
         %12 = OpLoad %8 %10
         %14 = OpIAdd %8 %12 %13
               OpStore %10 %14
               OpUnreachable
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

  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(
          500, 18, 501, 50, 50, {{50, 100}}, {{12, 201}, {14, 202}},
          {{12, 301}, {14, 302}});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun("
               OpName %10 "s"
               OpName %17 "b"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %13 = OpConstant %8 2
         %15 = OpTypeBool
         %16 = OpTypePointer Function %15
         %18 = OpConstantTrue %15
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %17 = OpVariable %16 Function
               OpStore %17 %18
         %19 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpBranch %500
        %500 = OpLabel
               OpSelectionMerge %501 None
               OpBranchConditional %18 %50 %100
         %50 = OpLabel
               OpStore %10 %11
         %12 = OpLoad %8 %10
         %14 = OpIAdd %8 %12 %13
               OpStore %10 %14
               OpBranch %501
        %100 = OpLabel
               OpStore %10 %11
        %201 = OpLoad %8 %10
        %202 = OpIAdd %8 %201 %13
               OpStore %10 %202
               OpBranch %501
        %501 = OpLabel
        %301 = OpPhi %8 %12 %50 %201 %100
        %302 = OpPhi %8 %14 %50 %202 %100
               OpUnreachable
               OpFunctionEnd
        )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     ExitBlockTerminatorOpKill) {
  // This test handles a case where the exit block ends with OpKill.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun("
               OpName %10 "s"
               OpName %17 "b"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %13 = OpConstant %8 2
         %15 = OpTypeBool
         %16 = OpTypePointer Function %15
         %18 = OpConstantTrue %15
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %17 = OpVariable %16 Function
               OpStore %17 %18
         %19 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpBranch %50
         %50 = OpLabel
               OpStore %10 %11
         %12 = OpLoad %8 %10
         %14 = OpIAdd %8 %12 %13
               OpStore %10 %14
               OpKill
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

  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(
          500, 18, 501, 50, 50, {{50, 100}}, {{12, 201}, {14, 202}},
          {{12, 301}, {14, 302}});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun("
               OpName %10 "s"
               OpName %17 "b"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %13 = OpConstant %8 2
         %15 = OpTypeBool
         %16 = OpTypePointer Function %15
         %18 = OpConstantTrue %15
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %17 = OpVariable %16 Function
               OpStore %17 %18
         %19 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpBranch %500
        %500 = OpLabel
               OpSelectionMerge %501 None
               OpBranchConditional %18 %50 %100
         %50 = OpLabel
               OpStore %10 %11
         %12 = OpLoad %8 %10
         %14 = OpIAdd %8 %12 %13
               OpStore %10 %14
               OpBranch %501
        %100 = OpLabel
               OpStore %10 %11
        %201 = OpLoad %8 %10
        %202 = OpIAdd %8 %201 %13
               OpStore %10 %202
               OpBranch %501
        %501 = OpLabel
        %301 = OpPhi %8 %12 %50 %201 %100
        %302 = OpPhi %8 %14 %50 %202 %100
               OpKill
               OpFunctionEnd
        )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     ContinueExitBlockNotApplicable) {
  // This test handles a case where the exit block is the continue target and
  // the transformation is not applicable.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "s"
               OpName %10 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %17 = OpConstant %6 10
         %18 = OpTypeBool
         %24 = OpConstant %6 5
         %30 = OpConstant %6 1
         %50 = OpConstantTrue %18
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %10 %9
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %13 %14 None
               OpBranch %15
         %15 = OpLabel
         %16 = OpLoad %6 %10
         %19 = OpSLessThan %18 %16 %17
               OpBranchConditional %19 %12 %13
         %12 = OpLabel
         %20 = OpLoad %6 %10
         %21 = OpLoad %6 %8
         %22 = OpIAdd %6 %21 %20
               OpStore %8 %22
         %23 = OpLoad %6 %10
         %25 = OpIEqual %18 %23 %24
               OpSelectionMerge %27 None
               OpBranchConditional %25 %26 %27
         %26 = OpLabel
               OpBranch %13
         %27 = OpLabel
               OpBranch %14
         %14 = OpLabel
         %29 = OpLoad %6 %10
         %31 = OpIAdd %6 %29 %30
               OpStore %10 %31
               OpBranch %11
         %13 = OpLabel
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
  TransformationDuplicateRegionWithSelection transformation_bad =
      TransformationDuplicateRegionWithSelection(
          500, 50, 501, 27, 14, {{27, 101}, {14, 102}}, {{29, 201}, {31, 202}},
          {{29, 301}, {31, 302}});

  ASSERT_FALSE(
      transformation_bad.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     MultiplePredecessorsNotApplicableTest) {
  // This test handles a case where the entry block has multiple predecessors
  // and the transformation is not applicable.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun1(i1;"
               OpName %9 "a"
               OpName %18 "b"
               OpName %24 "b"
               OpName %27 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 2
         %14 = OpTypeBool
         %23 = OpTypePointer Function %14
         %25 = OpConstantTrue %14
         %26 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %24 = OpVariable %23 Function
         %27 = OpVariable %7 Function
               OpStore %24 %25
               OpStore %27 %26
         %28 = OpFunctionCall %2 %10 %27
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %18 = OpVariable %7 Function
         %12 = OpLoad %6 %9
         %15 = OpSLessThan %14 %12 %13
               OpSelectionMerge %17 None
               OpBranchConditional %15 %16 %20
         %16 = OpLabel
         %19 = OpLoad %6 %9
               OpStore %18 %19
               OpBranch %60
         %20 = OpLabel
         %21 = OpLoad %6 %9
         %22 = OpIAdd %6 %21 %13
               OpStore %18 %22
               OpBranch %60
         %60 = OpLabel
               OpBranch %17
         %17 = OpLabel
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

  TransformationDuplicateRegionWithSelection transformation_bad =
      TransformationDuplicateRegionWithSelection(500, 25, 501, 60, 60,
                                                 {{60, 101}}, {{}}, {{}});

  ASSERT_FALSE(
      transformation_bad.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationDuplicateRegionWithSelectionTest, OverflowIds) {
  // This test checks that the transformation correctly uses overflow ids, when
  // they are both needed and provided.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun("
               OpName %10 "s"
               OpName %17 "b"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %13 = OpConstant %8 2
         %15 = OpTypeBool
         %16 = OpTypePointer Function %15
         %18 = OpConstantTrue %15
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %17 = OpVariable %16 Function
               OpStore %17 %18
         %19 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpBranch %50
         %50 = OpLabel
               OpStore %10 %11
         %12 = OpLoad %8 %10
         %14 = OpIAdd %8 %12 %13
               OpStore %10 %14
               OpKill
               OpFunctionEnd
         )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  auto overflow_ids_unique_ptr = MakeUnique<CounterOverflowIdSource>(1000);
  auto overflow_ids_ptr = overflow_ids_unique_ptr.get();
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options,
      std::move(overflow_ids_unique_ptr));

  // The mappings do not provide sufficient ids, thus overflow ids are required.
  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(500, 18, 501, 50, 50, {},
                                                 {{12, 201}}, {{14, 302}});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context,
                        overflow_ids_ptr->GetIssuedOverflowIds());
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun("
               OpName %10 "s"
               OpName %17 "b"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 0
         %13 = OpConstant %8 2
         %15 = OpTypeBool
         %16 = OpTypePointer Function %15
         %18 = OpConstantTrue %15
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %17 = OpVariable %16 Function
               OpStore %17 %18
         %19 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
               OpBranch %500
        %500 = OpLabel
               OpSelectionMerge %501 None
               OpBranchConditional %18 %50 %1000
         %50 = OpLabel
               OpStore %10 %11
         %12 = OpLoad %8 %10
         %14 = OpIAdd %8 %12 %13
               OpStore %10 %14
               OpBranch %501
       %1000 = OpLabel
               OpStore %10 %11
        %201 = OpLoad %8 %10
       %1002 = OpIAdd %8 %201 %13
               OpStore %10 %1002
               OpBranch %501
        %501 = OpLabel
       %1001 = OpPhi %8 %12 %50 %201 %1000
        %302 = OpPhi %8 %14 %50 %1002 %1000
               OpKill
               OpFunctionEnd
        )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     RegionExitIsOpBranchConditional) {
  // Checks the case where the exit block of a region ends with
  // OpBranchConditional (but is not a header).
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
               OpName %8 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %50 = OpConstantTrue %17
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
         %19 = OpLoad %6 %8
         %21 = OpIAdd %6 %19 %20
               OpStore %8 %21
               OpBranchConditional %50 %13 %12
         %13 = OpLabel
         %22 = OpLoad %6 %8
         %23 = OpIAdd %6 %22 %20
               OpStore %8 %23
               OpBranch %10
         %12 = OpLabel
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

  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(
          600, 50, 601, 11, 11, {{11, 602}}, {{19, 603}, {21, 604}},
          {{19, 605}, {21, 606}});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
               OpName %8 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %50 = OpConstantTrue %17
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
               OpBranchConditional %18 %600 %12
        %600 = OpLabel
               OpSelectionMerge %601 None
               OpBranchConditional %50 %11 %602
         %11 = OpLabel
         %19 = OpLoad %6 %8
         %21 = OpIAdd %6 %19 %20
               OpStore %8 %21
               OpBranch %601
        %602 = OpLabel
        %603 = OpLoad %6 %8
        %604 = OpIAdd %6 %603 %20
               OpStore %8 %604
               OpBranch %601
        %601 = OpLabel
        %605 = OpPhi %6 %19 %11 %603 %602
        %606 = OpPhi %6 %21 %11 %604 %602
               OpBranchConditional %50 %13 %12
         %13 = OpLabel
         %22 = OpLoad %6 %8
         %23 = OpIAdd %6 %22 %20
               OpStore %8 %23
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     RegionExitIsOpBranchConditionalUsingBooleanDefinedInBlock) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
               OpName %8 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %50 = OpConstantTrue %17
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
         %19 = OpLoad %6 %8
         %21 = OpIAdd %6 %19 %20
         %70 = OpCopyObject %17 %50
               OpStore %8 %21
               OpBranchConditional %70 %13 %12
         %13 = OpLabel
         %22 = OpLoad %6 %8
         %23 = OpIAdd %6 %22 %20
               OpStore %8 %23
               OpBranch %10
         %12 = OpLabel
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

  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(
          600, 50, 601, 11, 11, {{11, 602}}, {{19, 603}, {21, 604}, {70, 608}},
          {{19, 605}, {21, 606}, {70, 607}});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
               OpName %8 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %50 = OpConstantTrue %17
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
               OpBranchConditional %18 %600 %12
        %600 = OpLabel
               OpSelectionMerge %601 None
               OpBranchConditional %50 %11 %602
         %11 = OpLabel
         %19 = OpLoad %6 %8
         %21 = OpIAdd %6 %19 %20
         %70 = OpCopyObject %17 %50
               OpStore %8 %21
               OpBranch %601
        %602 = OpLabel
        %603 = OpLoad %6 %8
        %604 = OpIAdd %6 %603 %20
        %608 = OpCopyObject %17 %50
               OpStore %8 %604
               OpBranch %601
        %601 = OpLabel
        %605 = OpPhi %6 %19 %11 %603 %602
        %606 = OpPhi %6 %21 %11 %604 %602
        %607 = OpPhi %17 %70 %11 %608 %602
               OpBranchConditional %607 %13 %12
         %13 = OpLabel
         %22 = OpLoad %6 %8
         %23 = OpIAdd %6 %22 %20
               OpStore %8 %23
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     RegionExitUsesOpReturnValueWithIdDefinedInRegion) {
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
          %7 = OpTypeFunction %6
         %10 = OpConstant %6 2
         %30 = OpTypeBool
         %31 = OpConstantTrue %30
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %13 = OpFunctionCall %6 %8
               OpReturn
               OpFunctionEnd
          %8 = OpFunction %6 None %7
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %21 = OpCopyObject %6 %10
               OpReturnValue %21
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

  TransformationDuplicateRegionWithSelection transformation_good_1 =
      TransformationDuplicateRegionWithSelection(
          600, 31, 601, 20, 20, {{20, 602}}, {{21, 603}}, {{21, 605}});
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFunction %6
         %10 = OpConstant %6 2
         %30 = OpTypeBool
         %31 = OpConstantTrue %30
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %13 = OpFunctionCall %6 %8
               OpReturn
               OpFunctionEnd
          %8 = OpFunction %6 None %7
          %9 = OpLabel
               OpBranch %600
        %600 = OpLabel
               OpSelectionMerge %601 None
               OpBranchConditional %31 %20 %602
         %20 = OpLabel
         %21 = OpCopyObject %6 %10
               OpBranch %601
        %602 = OpLabel
        %603 = OpCopyObject %6 %10
               OpBranch %601
        %601 = OpLabel
        %605 = OpPhi %6 %21 %20 %603 %602
               OpReturnValue %605
               OpFunctionEnd
        )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     InapplicableDueToOpTypeSampledImage) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %10
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpDecorate %10 RelaxedPrecision
               OpDecorate %10 DescriptorSet 0
               OpDecorate %10 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeImage %6 2D 0 0 0 1 Unknown
          %8 = OpTypeSampledImage %7
          %9 = OpTypePointer UniformConstant %8
         %10 = OpVariable %9 UniformConstant
         %12 = OpTypeVector %6 2
         %13 = OpConstant %6 0
         %14 = OpConstantComposite %12 %13 %13
         %15 = OpTypeVector %6 4
         %30 = OpTypeBool
         %31 = OpConstantTrue %30
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %11 = OpLoad %8 %10
               OpBranch %21
         %21 = OpLabel
         %16 = OpImageSampleImplicitLod %15 %11 %14
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  ASSERT_FALSE(TransformationDuplicateRegionWithSelection(
                   600, 31, 601, 20, 20, {{20, 602}}, {{11, 603}}, {{11, 605}})
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     DoNotProduceOpPhiWithVoidType) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeBool
         %11 = OpConstantTrue %10
          %4 = OpFunction %2 None %3
         %12 = OpLabel
               OpBranch %5
          %5 = OpLabel
          %8 = OpFunctionCall %2 %6
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  TransformationDuplicateRegionWithSelection transformation(
      100, 11, 101, 5, 5, {{5, 102}}, {{8, 103}}, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %10 = OpTypeBool
         %11 = OpConstantTrue %10
          %4 = OpFunction %2 None %3
         %12 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpSelectionMerge %101 None
               OpBranchConditional %11 %5 %102
          %5 = OpLabel
          %8 = OpFunctionCall %2 %6
               OpBranch %101
        %102 = OpLabel
        %103 = OpFunctionCall %2 %6
               OpBranch %101
        %101 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationDuplicateRegionWithSelectionTest,
     DoNotProduceOpPhiWithDisallowedType) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpDecorate %13 DescriptorSet 0
               OpDecorate %13 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
          %8 = OpTypePointer Function %7
         %10 = OpTypeImage %6 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
         %12 = OpTypePointer UniformConstant %11
         %13 = OpVariable %12 UniformConstant
         %15 = OpConstant %6 1
         %16 = OpConstantComposite %7 %15 %15
         %17 = OpTypeVector %6 4
         %19 = OpTypeInt 32 0
         %20 = OpConstant %19 0
         %22 = OpTypePointer Function %6
         %90 = OpTypeBool
         %91 = OpConstantTrue %90
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
               OpBranch %81
         %81 = OpLabel
         %14 = OpLoad %11 %13
         %18 = OpImageSampleImplicitLod %17 %14 %16
         %21 = OpCompositeExtract %6 %18 0
         %23 = OpAccessChain %22 %9 %20
               OpStore %23 %21
               OpBranch %80
         %80 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  TransformationDuplicateRegionWithSelection transformation(
      100, 91, 101, 81, 81, {{81, 102}},
      {{14, 103}, {18, 104}, {21, 105}, {23, 106}}, {{18, 107}, {21, 108}});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpDecorate %13 DescriptorSet 0
               OpDecorate %13 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
          %8 = OpTypePointer Function %7
         %10 = OpTypeImage %6 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
         %12 = OpTypePointer UniformConstant %11
         %13 = OpVariable %12 UniformConstant
         %15 = OpConstant %6 1
         %16 = OpConstantComposite %7 %15 %15
         %17 = OpTypeVector %6 4
         %19 = OpTypeInt 32 0
         %20 = OpConstant %19 0
         %22 = OpTypePointer Function %6
         %90 = OpTypeBool
         %91 = OpConstantTrue %90
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
               OpBranch %100
        %100 = OpLabel
               OpSelectionMerge %101 None
               OpBranchConditional %91 %81 %102
         %81 = OpLabel
         %14 = OpLoad %11 %13
         %18 = OpImageSampleImplicitLod %17 %14 %16
         %21 = OpCompositeExtract %6 %18 0
         %23 = OpAccessChain %22 %9 %20
               OpStore %23 %21
               OpBranch %101
        %102 = OpLabel
        %103 = OpLoad %11 %13
        %104 = OpImageSampleImplicitLod %17 %103 %16
        %105 = OpCompositeExtract %6 %104 0
        %106 = OpAccessChain %22 %9 %20
               OpStore %106 %105
               OpBranch %101
        %101 = OpLabel
        %107 = OpPhi %17 %18 %81 %104 %102
        %108 = OpPhi %6 %21 %81 %105 %102
               OpBranch %80
         %80 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
