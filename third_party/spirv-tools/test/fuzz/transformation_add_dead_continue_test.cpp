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

#include "source/fuzz/transformation_add_dead_continue.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddDeadContinueTest, SimpleExample) {
  // For a simple loop, checks that some dead continue scenarios are possible,
  // sanity-checks that some illegal scenarios are indeed not allowed, and then
  // applies a transformation.

  // The SPIR-V for this test is adapted from the following GLSL, by separating
  // some assignments into their own basic blocks, and adding constants for true
  // and false:
  //
  // void main() {
  //   int x = 0;
  //   for (int i = 0; i < 10; i++) {
  //     x = x + i;
  //     x = x + i;
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
               OpName %10 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %17 = OpConstant %6 10
         %18 = OpTypeBool
         %41 = OpConstantTrue %18
         %42 = OpConstantFalse %18
         %27 = OpConstant %6 1
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
         %20 = OpLoad %6 %8
         %21 = OpLoad %6 %10
         %22 = OpIAdd %6 %20 %21
               OpStore %8 %22
               OpBranch %40
         %40 = OpLabel
         %23 = OpLoad %6 %8
         %24 = OpLoad %6 %10
         %25 = OpIAdd %6 %23 %24
               OpStore %8 %25
               OpBranch %14
         %14 = OpLabel
         %26 = OpLoad %6 %10
         %28 = OpIAdd %6 %26 %27
               OpStore %10 %28
               OpBranch %11
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));
  FactManager fact_manager;

  // These are all possibilities.
  ASSERT_TRUE(TransformationAddDeadContinue(11, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadContinue(11, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadContinue(12, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadContinue(12, false, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadContinue(40, true, {})
                  .IsApplicable(context.get(), fact_manager));
  ASSERT_TRUE(TransformationAddDeadContinue(40, false, {})
                  .IsApplicable(context.get(), fact_manager));

  // Inapplicable: 100 is not a block id.
  ASSERT_FALSE(TransformationAddDeadContinue(100, true, {})
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable: 10 is not in a loop.
  ASSERT_FALSE(TransformationAddDeadContinue(10, true, {})
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable: 15 does not branch unconditionally to a single successor.
  ASSERT_FALSE(TransformationAddDeadContinue(15, true, {})
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable: 13 is not in a loop and has no successor.
  ASSERT_FALSE(TransformationAddDeadContinue(13, true, {})
                   .IsApplicable(context.get(), fact_manager));

  // Inapplicable: 14 is the loop continue target, so it's not OK to jump to
  // the loop continue from there.
  ASSERT_FALSE(TransformationAddDeadContinue(14, false, {})
                   .IsApplicable(context.get(), fact_manager));

  // These are the transformations we will apply.
  auto transformation1 = TransformationAddDeadContinue(11, true, {});
  auto transformation2 = TransformationAddDeadContinue(12, false, {});
  auto transformation3 = TransformationAddDeadContinue(40, true, {});

  ASSERT_TRUE(transformation1.IsApplicable(context.get(), fact_manager));
  transformation1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation2.IsApplicable(context.get(), fact_manager));
  transformation2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(transformation3.IsApplicable(context.get(), fact_manager));
  transformation3.Apply(context.get(), &fact_manager);
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
               OpName %10 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %17 = OpConstant %6 10
         %18 = OpTypeBool
         %41 = OpConstantTrue %18
         %42 = OpConstantFalse %18
         %27 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %10 %9
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %13 %14 None
               OpBranchConditional %41 %15 %14
         %15 = OpLabel
         %16 = OpLoad %6 %10
         %19 = OpSLessThan %18 %16 %17
               OpBranchConditional %19 %12 %13
         %12 = OpLabel
         %20 = OpLoad %6 %8
         %21 = OpLoad %6 %10
         %22 = OpIAdd %6 %20 %21
               OpStore %8 %22
               OpBranchConditional %42 %14 %40
         %40 = OpLabel
         %23 = OpLoad %6 %8
         %24 = OpLoad %6 %10
         %25 = OpIAdd %6 %23 %24
               OpStore %8 %25
               OpBranchConditional %41 %14 %14
         %14 = OpLabel
         %26 = OpLoad %6 %10
         %28 = OpIAdd %6 %26 %27
               OpStore %10 %28
               OpBranch %11
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadContinueTest, LoopNest) {
  // Checks some allowed and disallowed scenarios for a nest of loops, including
  // continuing a loop from an if or switch.

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

  std::vector<uint32_t> good = {6, 7, 18, 20, 34, 40, 45, 46, 47, 56, 57};
  std::vector<uint32_t> bad = {5, 8, 9, 19, 21, 22, 33, 41, 58, 59, 60};

  for (uint32_t from_block : bad) {
    ASSERT_FALSE(TransformationAddDeadContinue(from_block, true, {})
                     .IsApplicable(context.get(), fact_manager));
  }
  for (uint32_t from_block : good) {
    const TransformationAddDeadContinue transformation(from_block, true, {});
    ASSERT_TRUE(transformation.IsApplicable(context.get(), fact_manager));
    transformation.Apply(context.get(), &fact_manager);
    ASSERT_FALSE(transformation.IsApplicable(context.get(), fact_manager));
  }

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
               OpBranchConditional %67 %7 %9
          %7 = OpLabel
         %13 = OpLoad %10 %12
         %15 = OpIAdd %10 %13 %14
               OpStore %12 %15
               OpStore %16 %17
               OpBranchConditional %67 %18 %9
         %18 = OpLabel
               OpLoopMerge %20 %21 None
               OpBranchConditional %67 %22 %21
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
               OpBranchConditional %67 %41 %21
         %41 = OpLabel
         %44 = OpLoad %10 %12
               OpSelectionMerge %47 None
               OpSwitch %44 %46 0 %45
         %46 = OpLabel
               OpBranchConditional %67 %47 %21
         %45 = OpLabel
               OpStore %12 %38
               OpBranchConditional %67 %46 %21
         %47 = OpLabel
               OpBranchConditional %67 %34 %21
         %34 = OpLabel
               OpBranchConditional %67 %21 %21
         %21 = OpLabel
         %50 = OpLoad %10 %16
         %51 = OpIAdd %10 %50 %14
               OpStore %16 %51
               OpBranch %18
         %20 = OpLabel
               OpBranchConditional %67 %9 %9
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
               OpBranchConditional %67 %60 %59
         %60 = OpLabel
         %61 = OpLoad %10 %55
         %62 = OpSLessThan %25 %61 %24
               OpBranchConditional %62 %57 %58
         %57 = OpLabel
         %63 = OpLoad %10 %12
         %64 = OpIAdd %10 %63 %14
               OpStore %12 %64
               OpBranchConditional %67 %59 %59
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

TEST(TransformationAddDeadConditionalTest, LoopInContinueConstruct) {
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
        %100 = OpConstantFalse %21
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

  std::vector<uint32_t> good = {32, 33, 46, 52, 101};
  std::vector<uint32_t> bad = {5, 34, 36, 35, 47, 49, 48};

  for (uint32_t from_block : bad) {
    ASSERT_FALSE(TransformationAddDeadContinue(from_block, false, {})
                     .IsApplicable(context.get(), fact_manager));
  }
  for (uint32_t from_block : good) {
    const TransformationAddDeadContinue transformation(from_block, false, {});
    ASSERT_TRUE(transformation.IsApplicable(context.get(), fact_manager));
    transformation.Apply(context.get(), &fact_manager);
    ASSERT_FALSE(transformation.IsApplicable(context.get(), fact_manager));
  }

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
        %100 = OpConstantFalse %21
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
               OpBranchConditional %100 %35 %36
         %36 = OpLabel
         %37 = OpLoad %6 %31
         %39 = OpSLessThan %21 %37 %38
               OpBranchConditional %39 %33 %34
         %33 = OpLabel
               OpBranchConditional %100 %35 %35
         %35 = OpLabel
               OpStore %43 %12
               OpStore %44 %12
               OpBranch %46
         %46 = OpLabel
               OpLoopMerge %47 %48 None
               OpBranchConditional %100 %48 %49
         %49 = OpLabel
         %50 = OpLoad %6 %44
         %51 = OpSLessThan %21 %50 %20
               OpBranchConditional %51 %52 %47
         %52 = OpLabel
         %53 = OpLoad %6 %43
               OpBranchConditional %100 %48 %101
        %101 = OpLabel
         %54 = OpIAdd %6 %53 %24
               OpStore %43 %54
               OpBranchConditional %100 %48 %48
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

TEST(TransformationAddDeadContinueTest, PhiInstructions) {
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
         %51 = OpPhi %10 %52 %20 %100 %32
         %47 = OpPhi %6 %48 %20 %101 %32
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
        %100 = OpPhi %10 %42 %75
        %101 = OpPhi %6 %39 %75
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

  std::vector<uint32_t> bad = {5, 19, 20, 23, 31, 32, 33, 70};

  std::vector<uint32_t> good = {29, 30, 75};

  for (uint32_t from_block : bad) {
    ASSERT_FALSE(TransformationAddDeadContinue(from_block, true, {})
                     .IsApplicable(context.get(), fact_manager));
  }
  auto transformation1 = TransformationAddDeadContinue(29, true, {13, 21});
  ASSERT_TRUE(transformation1.IsApplicable(context.get(), fact_manager));
  transformation1.Apply(context.get(), &fact_manager);

  auto transformation2 = TransformationAddDeadContinue(30, true, {22, 46});
  ASSERT_TRUE(transformation2.IsApplicable(context.get(), fact_manager));
  transformation2.Apply(context.get(), &fact_manager);

  // 75 already has the continue block as a successor, so we should not provide
  // phi ids.
  auto transformationBad = TransformationAddDeadContinue(75, true, {27, 46});
  ASSERT_FALSE(transformationBad.IsApplicable(context.get(), fact_manager));

  auto transformation3 = TransformationAddDeadContinue(75, true, {});
  ASSERT_TRUE(transformation3.IsApplicable(context.get(), fact_manager));
  transformation3.Apply(context.get(), &fact_manager);

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
         %51 = OpPhi %10 %52 %20 %100 %32
         %47 = OpPhi %6 %48 %20 %101 %32
               OpLoopMerge %31 %32 None
               OpBranchConditional %80 %33 %32
         %33 = OpLabel
         %36 = OpSLessThan %17 %47 %46
               OpBranchConditional %36 %30 %31
         %30 = OpLabel
         %39 = OpIAdd %6 %47 %38
               OpStore %8 %39
               OpBranchConditional %80 %75 %32
         %75 = OpLabel
         %42 = OpFAdd %10 %51 %41
               OpStore %12 %42
               OpBranchConditional %80 %32 %32
         %32 = OpLabel
        %100 = OpPhi %10 %42 %75 %13 %29 %22 %30
        %101 = OpPhi %6 %39 %75 %21 %29 %46 %30
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

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddDeadContinueTest, RespectDominanceRules1) {
  // Checks that a dead continue cannot be added if it would prevent a block
  // later in the loop from dominating the loop's continue construct, in the
  // case where said block defines and id that is used in the loop's continue
  // construct.

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
               OpBranch %6
          %6 = OpLabel
               OpLoopMerge %8 %9 None
               OpBranch %7
          %7 = OpLabel
         %21 = OpCopyObject %10 %11
               OpBranch %9
          %9 = OpLabel
         %20 = OpPhi %10 %21 %7
               OpBranchConditional %11 %6 %8
          %8 = OpLabel
               OpBranch %12
         %12 = OpLabel
               OpLoopMerge %14 %15 None
               OpBranch %13
         %13 = OpLabel
               OpBranch %22
         %22 = OpLabel
         %23 = OpCopyObject %10 %11
               OpBranch %25
         %25 = OpLabel
               OpBranch %15
         %15 = OpLabel
         %26 = OpCopyObject %10 %23
               OpBranchConditional %11 %12 %14
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // This transformation is not applicable because the dead continue from the
  // loop body prevents the definition of %23 later in the loop body from
  // dominating its use in the loop's continue target.
  auto bad_transformation = TransformationAddDeadContinue(13, false, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));

  auto good_transformation_1 = TransformationAddDeadContinue(7, false, {});
  ASSERT_TRUE(good_transformation_1.IsApplicable(context.get(), fact_manager));
  good_transformation_1.Apply(context.get(), &fact_manager);

  auto good_transformation_2 = TransformationAddDeadContinue(22, false, {});
  ASSERT_TRUE(good_transformation_2.IsApplicable(context.get(), fact_manager));
  good_transformation_2.Apply(context.get(), &fact_manager);

  // This transformation is OK, because the definition of %21 in the loop body
  // is only used in an OpPhi in the loop's continue target.
  auto good_transformation_3 = TransformationAddDeadContinue(6, false, {11});
  ASSERT_TRUE(good_transformation_3.IsApplicable(context.get(), fact_manager));
  good_transformation_3.Apply(context.get(), &fact_manager);

  std::string after_transformations = R"(
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
               OpBranch %6
          %6 = OpLabel
               OpLoopMerge %8 %9 None
               OpBranchConditional %11 %9 %7
          %7 = OpLabel
         %21 = OpCopyObject %10 %11
               OpBranchConditional %11 %9 %9
          %9 = OpLabel
         %20 = OpPhi %10 %21 %7 %11 %6
               OpBranchConditional %11 %6 %8
          %8 = OpLabel
               OpBranch %12
         %12 = OpLabel
               OpLoopMerge %14 %15 None
               OpBranch %13
         %13 = OpLabel
               OpBranch %22
         %22 = OpLabel
         %23 = OpCopyObject %10 %11
               OpBranchConditional %11 %15 %25
         %25 = OpLabel
               OpBranch %15
         %15 = OpLabel
         %26 = OpCopyObject %10 %23
               OpBranchConditional %11 %12 %14
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationAddDeadContinueTest, RespectDominanceRules2) {
  // Checks that a dead continue cannot be added if it would lead to a use after
  // the loop failing to be dominated by its definition.

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

  // This transformation would shortcut the part of the loop body that defines
  // an id used after the loop.
  auto bad_transformation = TransformationAddDeadContinue(100, false, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadContinueTest, RespectDominanceRules3) {
  // Checks that a dead continue cannot be added if it would lead to a dominance
  // problem with an id used in an OpPhi after the loop.

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

  // This transformation would shortcut the part of the loop body that defines
  // an id used after the loop.
  auto bad_transformation = TransformationAddDeadContinue(100, false, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadContinueTest, Miscellaneous1) {
  // A miscellaneous test that exposed a bug in spirv-fuzz.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %586 %623
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpMemberDecorate %34 0 Offset 0
               OpDecorate %34 Block
               OpDecorate %36 DescriptorSet 0
               OpDecorate %36 Binding 0
               OpDecorate %586 BuiltIn FragCoord
               OpMemberDecorate %591 0 Offset 0
               OpDecorate %591 Block
               OpDecorate %593 DescriptorSet 0
               OpDecorate %593 Binding 1
               OpDecorate %623 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 2
         %17 = OpTypeBool
         %27 = OpTypeFloat 32
         %28 = OpTypeVector %27 2
         %29 = OpTypeMatrix %28 2
         %30 = OpTypePointer Private %29
         %31 = OpVariable %30 Private
         %34 = OpTypeStruct %27
         %35 = OpTypePointer Uniform %34
         %36 = OpVariable %35 Uniform
         %37 = OpTypePointer Uniform %27
         %40 = OpTypePointer Private %27
         %43 = OpConstant %6 1
         %62 = OpConstant %6 3
         %64 = OpTypeVector %27 3
         %65 = OpTypeMatrix %64 2
         %66 = OpTypePointer Private %65
         %67 = OpVariable %66 Private
         %92 = OpConstant %6 4
         %94 = OpTypeVector %27 4
         %95 = OpTypeMatrix %94 2
         %96 = OpTypePointer Private %95
         %97 = OpVariable %96 Private
        %123 = OpTypeMatrix %28 3
        %124 = OpTypePointer Private %123
        %125 = OpVariable %124 Private
        %151 = OpTypeMatrix %64 3
        %152 = OpTypePointer Private %151
        %153 = OpVariable %152 Private
        %179 = OpTypeMatrix %94 3
        %180 = OpTypePointer Private %179
        %181 = OpVariable %180 Private
        %207 = OpTypeMatrix %28 4
        %208 = OpTypePointer Private %207
        %209 = OpVariable %208 Private
        %235 = OpTypeMatrix %64 4
        %236 = OpTypePointer Private %235
        %237 = OpVariable %236 Private
        %263 = OpTypeMatrix %94 4
        %264 = OpTypePointer Private %263
        %265 = OpVariable %264 Private
        %275 = OpTypeInt 32 0
        %276 = OpConstant %275 9
        %277 = OpTypeArray %27 %276
        %278 = OpTypePointer Function %277
        %280 = OpConstant %27 0
        %281 = OpTypePointer Function %27
        %311 = OpConstant %27 16
        %448 = OpConstant %6 5
        %482 = OpConstant %6 6
        %516 = OpConstant %6 7
        %550 = OpConstant %6 8
        %585 = OpTypePointer Input %94
        %586 = OpVariable %585 Input
        %587 = OpConstant %275 0
        %588 = OpTypePointer Input %27
        %591 = OpTypeStruct %28
        %592 = OpTypePointer Uniform %591
        %593 = OpVariable %592 Uniform
        %596 = OpConstant %27 3
        %601 = OpConstant %275 1
        %617 = OpConstant %6 9
        %622 = OpTypePointer Output %94
        %623 = OpVariable %622 Output
        %628 = OpConstant %27 1
        %634 = OpConstantComposite %94 %280 %280 %280 %628
        %635 = OpUndef %6
        %636 = OpUndef %17
        %637 = OpUndef %27
        %638 = OpUndef %64
        %639 = OpUndef %94
        %640 = OpConstantTrue %17
        %736 = OpConstantFalse %17
        %642 = OpVariable %37 Uniform
        %643 = OpVariable %40 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %164
        %164 = OpLabel
               OpLoopMerge %166 %167 None
               OpBranch %165
        %165 = OpLabel
               OpBranch %172
        %172 = OpLabel
               OpSelectionMerge %174 None
               OpBranchConditional %640 %174 %174
        %174 = OpLabel
        %785 = OpCopyObject %6 %43
               OpBranch %167
        %167 = OpLabel
        %190 = OpIAdd %6 %9 %785
               OpBranchConditional %640 %164 %166
        %166 = OpLabel
               OpBranch %196
        %196 = OpLabel
               OpBranch %194
        %194 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // This transformation would shortcut the part of the loop body that defines
  // an id used in the continue target.
  auto bad_transformation = TransformationAddDeadContinue(165, false, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadContinueTest, Miscellaneous2) {
  // A miscellaneous test that exposed a bug in spirv-fuzz.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %51 = OpTypeBool
        %395 = OpConstantTrue %51
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %389
        %389 = OpLabel
               OpLoopMerge %388 %391 None
               OpBranch %339
        %339 = OpLabel
               OpSelectionMerge %396 None
               OpBranchConditional %395 %388 %396
        %396 = OpLabel
               OpBranch %1552
       %1552 = OpLabel
               OpLoopMerge %1553 %1554 None
               OpBranch %1556
       %1556 = OpLabel
               OpLoopMerge %1557 %1570 None
               OpBranchConditional %395 %1562 %1557
       %1562 = OpLabel
               OpBranchConditional %395 %1571 %1570
       %1571 = OpLabel
               OpBranch %1557
       %1570 = OpLabel
               OpBranch %1556
       %1557 = OpLabel
               OpSelectionMerge %1586 None
               OpBranchConditional %395 %1553 %1586
       %1586 = OpLabel
               OpBranch %1553
       %1554 = OpLabel
               OpBranch %1552
       %1553 = OpLabel
               OpBranch %388
        %391 = OpLabel
               OpBranch %389
        %388 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // This transformation would introduce a branch from a continue target to
  // itself.
  auto bad_transformation = TransformationAddDeadContinue(1554, true, {});
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadContinueTest, Miscellaneous3) {
  // A miscellaneous test that exposed a bug in spirv-fuzz.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %85 = OpTypeBool
        %434 = OpConstantFalse %85
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %234
        %234 = OpLabel
               OpLoopMerge %235 %236 None
               OpBranch %259
        %259 = OpLabel
               OpLoopMerge %260 %274 None
               OpBranchConditional %434 %265 %260
        %265 = OpLabel
               OpBranch %275
        %275 = OpLabel
               OpBranch %260
        %274 = OpLabel
               OpBranch %259
        %260 = OpLabel
               OpSelectionMerge %298 None
               OpBranchConditional %434 %299 %300
        %300 = OpLabel
               OpBranch %235
        %298 = OpLabel
               OpUnreachable
        %236 = OpLabel
               OpBranch %234
        %299 = OpLabel
               OpBranch %235
        %235 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto bad_transformation = TransformationAddDeadContinue(299, false, {});

  // The continue edge would connect %299 to the previously-unreachable %236,
  // making %299 dominate %236, and breaking the rule that block ordering must
  // respect dominance.
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadContinueTest, Miscellaneous4) {
  // A miscellaneous test that exposed a bug in spirv-fuzz.

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
        %100 = OpConstantFalse %17
         %21 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %13 = OpLabel
         %20 = OpLoad %6 %8
         %22 = OpIAdd %6 %20 %21
               OpStore %8 %22
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %15 = OpLoad %6 %8
         %18 = OpSLessThan %17 %15 %16
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
               OpBranch %12
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto bad_transformation = TransformationAddDeadContinue(10, false, {});

  // The continue edge would connect %10 to the previously-unreachable %13,
  // making %10 dominate %13, and breaking the rule that block ordering must
  // respect dominance.
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadContinueTest, Miscellaneous5) {
  // A miscellaneous test that exposed a bug in spirv-fuzz.

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
          %7 = OpTypePointer Function %6
          %9 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %98
         %98 = OpLabel
               OpLoopMerge %100 %101 None
               OpBranch %99
         %99 = OpLabel
               OpSelectionMerge %111 None
               OpBranchConditional %9 %110 %111
        %110 = OpLabel
               OpBranch %100
        %111 = OpLabel
        %200 = OpCopyObject %6 %9
               OpBranch %101
        %101 = OpLabel
        %201 = OpCopyObject %6 %200
               OpBranchConditional %9 %98 %100
        %100 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto bad_transformation = TransformationAddDeadContinue(110, true, {});

  // The continue edge would lead to the use of %200 in block %101 no longer
  // being dominated by its definition in block %111.
  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationAddDeadContinueTest, Miscellaneous6) {
  // A miscellaneous test that exposed a bug in spirv-fuzz.

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
               OpLoopMerge %13 %12 None
               OpBranch %11
         %11 = OpLabel
         %20 = OpCopyObject %6 %9
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %9 %10 %13
         %13 = OpLabel
         %21 = OpCopyObject %6 %20
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  auto bad_transformation = TransformationAddDeadContinue(10, true, {});

  ASSERT_FALSE(bad_transformation.IsApplicable(context.get(), fact_manager));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
