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

#include "source/fuzz/transformation_set_loop_control.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationSetLoopControlTest, VariousScenarios) {
  // This test features loops with various different controls, and goes through
  // a number of acceptable and unacceptable transformations to those controls.

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
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %20 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %22 = OpVariable %7 Function
         %32 = OpVariable %7 Function
         %42 = OpVariable %7 Function
         %52 = OpVariable %7 Function
         %62 = OpVariable %7 Function
         %72 = OpVariable %7 Function
         %82 = OpVariable %7 Function
         %92 = OpVariable %7 Function
        %102 = OpVariable %7 Function
        %112 = OpVariable %7 Function
        %122 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %10 = OpLabel
        %132 = OpPhi %6 %9 %5 %21 %13
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %18 = OpSLessThan %17 %132 %16
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %21 = OpIAdd %6 %132 %20
               OpStore %8 %21
               OpBranch %10
         %12 = OpLabel
               OpStore %22 %9
               OpBranch %23
         %23 = OpLabel
        %133 = OpPhi %6 %9 %12 %31 %26
               OpLoopMerge %25 %26 Unroll
               OpBranch %27
         %27 = OpLabel
         %29 = OpSLessThan %17 %133 %16
               OpBranchConditional %29 %24 %25
         %24 = OpLabel
               OpBranch %26
         %26 = OpLabel
         %31 = OpIAdd %6 %133 %20
               OpStore %22 %31
               OpBranch %23
         %25 = OpLabel
               OpStore %32 %9
               OpBranch %33
         %33 = OpLabel
        %134 = OpPhi %6 %9 %25 %41 %36
               OpLoopMerge %35 %36 DontUnroll
               OpBranch %37
         %37 = OpLabel
         %39 = OpSLessThan %17 %134 %16
               OpBranchConditional %39 %34 %35
         %34 = OpLabel
               OpBranch %36
         %36 = OpLabel
         %41 = OpIAdd %6 %134 %20
               OpStore %32 %41
               OpBranch %33
         %35 = OpLabel
               OpStore %42 %9
               OpBranch %43
         %43 = OpLabel
        %135 = OpPhi %6 %9 %35 %51 %46
               OpLoopMerge %45 %46 DependencyInfinite
               OpBranch %47
         %47 = OpLabel
         %49 = OpSLessThan %17 %135 %16
               OpBranchConditional %49 %44 %45
         %44 = OpLabel
               OpBranch %46
         %46 = OpLabel
         %51 = OpIAdd %6 %135 %20
               OpStore %42 %51
               OpBranch %43
         %45 = OpLabel
               OpStore %52 %9
               OpBranch %53
         %53 = OpLabel
        %136 = OpPhi %6 %9 %45 %61 %56
               OpLoopMerge %55 %56 DependencyLength 3
               OpBranch %57
         %57 = OpLabel
         %59 = OpSLessThan %17 %136 %16
               OpBranchConditional %59 %54 %55
         %54 = OpLabel
               OpBranch %56
         %56 = OpLabel
         %61 = OpIAdd %6 %136 %20
               OpStore %52 %61
               OpBranch %53
         %55 = OpLabel
               OpStore %62 %9
               OpBranch %63
         %63 = OpLabel
        %137 = OpPhi %6 %9 %55 %71 %66
               OpLoopMerge %65 %66 MinIterations 10
               OpBranch %67
         %67 = OpLabel
         %69 = OpSLessThan %17 %137 %16
               OpBranchConditional %69 %64 %65
         %64 = OpLabel
               OpBranch %66
         %66 = OpLabel
         %71 = OpIAdd %6 %137 %20
               OpStore %62 %71
               OpBranch %63
         %65 = OpLabel
               OpStore %72 %9
               OpBranch %73
         %73 = OpLabel
        %138 = OpPhi %6 %9 %65 %81 %76
               OpLoopMerge %75 %76 MaxIterations 50
               OpBranch %77
         %77 = OpLabel
         %79 = OpSLessThan %17 %138 %16
               OpBranchConditional %79 %74 %75
         %74 = OpLabel
               OpBranch %76
         %76 = OpLabel
         %81 = OpIAdd %6 %138 %20
               OpStore %72 %81
               OpBranch %73
         %75 = OpLabel
               OpStore %82 %9
               OpBranch %83
         %83 = OpLabel
        %139 = OpPhi %6 %9 %75 %91 %86
               OpLoopMerge %85 %86 IterationMultiple 4
               OpBranch %87
         %87 = OpLabel
         %89 = OpSLessThan %17 %139 %16
               OpBranchConditional %89 %84 %85
         %84 = OpLabel
               OpBranch %86
         %86 = OpLabel
         %91 = OpIAdd %6 %139 %20
               OpStore %82 %91
               OpBranch %83
         %85 = OpLabel
               OpStore %92 %9
               OpBranch %93
         %93 = OpLabel
        %140 = OpPhi %6 %9 %85 %101 %96
               OpLoopMerge %95 %96 PeelCount 2
               OpBranch %97
         %97 = OpLabel
         %99 = OpSLessThan %17 %140 %16
               OpBranchConditional %99 %94 %95
         %94 = OpLabel
               OpBranch %96
         %96 = OpLabel
        %101 = OpIAdd %6 %140 %20
               OpStore %92 %101
               OpBranch %93
         %95 = OpLabel
               OpStore %102 %9
               OpBranch %103
        %103 = OpLabel
        %141 = OpPhi %6 %9 %95 %111 %106
               OpLoopMerge %105 %106 PartialCount 3
               OpBranch %107
        %107 = OpLabel
        %109 = OpSLessThan %17 %141 %16
               OpBranchConditional %109 %104 %105
        %104 = OpLabel
               OpBranch %106
        %106 = OpLabel
        %111 = OpIAdd %6 %141 %20
               OpStore %102 %111
               OpBranch %103
        %105 = OpLabel
               OpStore %112 %9
               OpBranch %113
        %113 = OpLabel
        %142 = OpPhi %6 %9 %105 %121 %116
               OpLoopMerge %115 %116 Unroll|PeelCount|PartialCount 3 4
               OpBranch %117
        %117 = OpLabel
        %119 = OpSLessThan %17 %142 %16
               OpBranchConditional %119 %114 %115
        %114 = OpLabel
               OpBranch %116
        %116 = OpLabel
        %121 = OpIAdd %6 %142 %20
               OpStore %112 %121
               OpBranch %113
        %115 = OpLabel
               OpStore %122 %9
               OpBranch %123
        %123 = OpLabel
        %143 = OpPhi %6 %9 %115 %131 %126
               OpLoopMerge %125 %126 DependencyLength|MinIterations|MaxIterations|IterationMultiple|PeelCount|PartialCount 2 5 90 4 7 14
               OpBranch %127
        %127 = OpLabel
        %129 = OpSLessThan %17 %143 %16
               OpBranchConditional %129 %124 %125
        %124 = OpLabel
               OpBranch %126
        %126 = OpLabel
        %131 = OpIAdd %6 %143 %20
               OpStore %122 %131
               OpBranch %123
        %125 = OpLabel
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
  // These are the loop headers together with the selection controls of their
  // merge instructions:
  //  %10 None
  //  %23 Unroll
  //  %33 DontUnroll
  //  %43 DependencyInfinite
  //  %53 DependencyLength 3
  //  %63 MinIterations 10
  //  %73 MaxIterations 50
  //  %83 IterationMultiple 4
  //  %93 PeelCount 2
  // %103 PartialCount 3
  // %113 Unroll|PeelCount|PartialCount 3 4
  // %123
  // DependencyLength|MinIterations|MaxIterations|IterationMultiple|PeelCount|PartialCount
  // 2 5 90 4 7 14

  ASSERT_TRUE(TransformationSetLoopControl(
                  10, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  10, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  10, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   10, (uint32_t)spv::LoopControlMask::DependencyInfinite, 0, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   10, (uint32_t)spv::LoopControlMask::DependencyLength, 0, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   10, (uint32_t)spv::LoopControlMask::MinIterations, 0, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   10, (uint32_t)spv::LoopControlMask::MaxIterations, 0, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   10, (uint32_t)spv::LoopControlMask::IterationMultiple, 0, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  10, (uint32_t)spv::LoopControlMask::PeelCount, 3, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   10, (uint32_t)spv::LoopControlMask::PeelCount, 3, 3)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  10, (uint32_t)spv::LoopControlMask::PartialCount, 0, 3)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   10, (uint32_t)spv::LoopControlMask::PartialCount, 3, 3)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  10,
                  (uint32_t)spv::LoopControlMask::PeelCount |
                      (uint32_t)spv::LoopControlMask::PartialCount,
                  3, 3)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  10,
                  (uint32_t)spv::LoopControlMask::Unroll |
                      (uint32_t)spv::LoopControlMask::PeelCount |
                      (uint32_t)spv::LoopControlMask::PartialCount,
                  3, 3)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   10,
                   (uint32_t)spv::LoopControlMask::DontUnroll |
                       (uint32_t)spv::LoopControlMask::PeelCount |
                       (uint32_t)spv::LoopControlMask::PartialCount,
                   3, 3)
                   .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  23, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  23, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  23, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  23,
                  (uint32_t)spv::LoopControlMask::PeelCount |
                      (uint32_t)spv::LoopControlMask::PartialCount,
                  3, 3)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   23, (uint32_t)spv::LoopControlMask::MaxIterations, 2, 3)
                   .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  33, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  33, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  33, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   33, (uint32_t)spv::LoopControlMask::MinIterations, 0, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  33,
                  (uint32_t)spv::LoopControlMask::Unroll |
                      (uint32_t)spv::LoopControlMask::PeelCount,
                  5, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   33,
                   (uint32_t)spv::LoopControlMask::DontUnroll |
                       (uint32_t)spv::LoopControlMask::PartialCount,
                   0, 10)
                   .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  43, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  43, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  43, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  43,
                  (uint32_t)spv::LoopControlMask::MaskNone |
                      (uint32_t)spv::LoopControlMask::DependencyInfinite,
                  0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  43,
                  (uint32_t)spv::LoopControlMask::Unroll |
                      (uint32_t)spv::LoopControlMask::DependencyInfinite,
                  0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  43,
                  (uint32_t)spv::LoopControlMask::DontUnroll |
                      (uint32_t)spv::LoopControlMask::DependencyInfinite,
                  0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   43,
                   (uint32_t)spv::LoopControlMask::DependencyInfinite |
                       (uint32_t)spv::LoopControlMask::DependencyLength,
                   0, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  43,
                  (uint32_t)spv::LoopControlMask::Unroll |
                      (uint32_t)spv::LoopControlMask::PeelCount,
                  5, 0)
                  .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  53, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  53, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  53, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   53, (uint32_t)spv::LoopControlMask::MaxIterations, 0, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  53,
                  (uint32_t)spv::LoopControlMask::MaskNone |
                      (uint32_t)spv::LoopControlMask::DependencyLength,
                  0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   53,
                   (uint32_t)spv::LoopControlMask::Unroll |
                       (uint32_t)spv::LoopControlMask::DependencyInfinite,
                   0, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  53,
                  (uint32_t)spv::LoopControlMask::DontUnroll |
                      (uint32_t)spv::LoopControlMask::DependencyLength,
                  0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   53,
                   (uint32_t)spv::LoopControlMask::DependencyInfinite |
                       (uint32_t)spv::LoopControlMask::DependencyLength,
                   0, 0)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  53,
                  (uint32_t)spv::LoopControlMask::Unroll |
                      (uint32_t)spv::LoopControlMask::DependencyLength |
                      (uint32_t)spv::LoopControlMask::PeelCount |
                      (uint32_t)spv::LoopControlMask::PartialCount,
                  5, 3)
                  .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  63, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  63, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  63, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  63,
                  (uint32_t)spv::LoopControlMask::Unroll |
                      (uint32_t)spv::LoopControlMask::MinIterations |
                      (uint32_t)spv::LoopControlMask::PeelCount |
                      (uint32_t)spv::LoopControlMask::PartialCount,
                  5, 3)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  63,
                  (uint32_t)spv::LoopControlMask::Unroll |
                      (uint32_t)spv::LoopControlMask::MinIterations |
                      (uint32_t)spv::LoopControlMask::PeelCount,
                  23, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   63,
                   (uint32_t)spv::LoopControlMask::Unroll |
                       (uint32_t)spv::LoopControlMask::MinIterations |
                       (uint32_t)spv::LoopControlMask::PeelCount,
                   2, 23)
                   .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  73, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  73, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  73, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   73,
                   (uint32_t)spv::LoopControlMask::Unroll |
                       (uint32_t)spv::LoopControlMask::MinIterations |
                       (uint32_t)spv::LoopControlMask::PeelCount |
                       (uint32_t)spv::LoopControlMask::PartialCount,
                   5, 3)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  73,
                  (uint32_t)spv::LoopControlMask::Unroll |
                      (uint32_t)spv::LoopControlMask::MaxIterations |
                      (uint32_t)spv::LoopControlMask::PeelCount,
                  23, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   73,
                   (uint32_t)spv::LoopControlMask::Unroll |
                       (uint32_t)spv::LoopControlMask::MaxIterations |
                       (uint32_t)spv::LoopControlMask::PeelCount,
                   2, 23)
                   .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  83, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  83, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  83, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   83,
                   (uint32_t)spv::LoopControlMask::Unroll |
                       (uint32_t)spv::LoopControlMask::MinIterations |
                       (uint32_t)spv::LoopControlMask::PeelCount |
                       (uint32_t)spv::LoopControlMask::PartialCount,
                   5, 3)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  83,
                  (uint32_t)spv::LoopControlMask::Unroll |
                      (uint32_t)spv::LoopControlMask::IterationMultiple |
                      (uint32_t)spv::LoopControlMask::PeelCount,
                  23, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   83,
                   (uint32_t)spv::LoopControlMask::Unroll |
                       (uint32_t)spv::LoopControlMask::IterationMultiple |
                       (uint32_t)spv::LoopControlMask::PeelCount,
                   2, 23)
                   .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  93, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  93, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  93, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  93, (uint32_t)spv::LoopControlMask::PeelCount, 8, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   93, (uint32_t)spv::LoopControlMask::PeelCount, 8, 8)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  93, (uint32_t)spv::LoopControlMask::PartialCount, 0, 8)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  93,
                  (uint32_t)spv::LoopControlMask::PeelCount |
                      (uint32_t)spv::LoopControlMask::PartialCount,
                  16, 8)
                  .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  103, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  103, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  103, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  103, (uint32_t)spv::LoopControlMask::PartialCount, 0, 60)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   103,
                   (uint32_t)spv::LoopControlMask::DontUnroll |
                       (uint32_t)spv::LoopControlMask::PartialCount,
                   0, 60)
                   .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  113, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  113, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  113, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  113, (uint32_t)spv::LoopControlMask::PeelCount, 12, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   113,
                   (uint32_t)spv::LoopControlMask::IterationMultiple |
                       (uint32_t)spv::LoopControlMask::PeelCount,
                   12, 0)
                   .IsApplicable(context.get(), transformation_context));

  ASSERT_TRUE(TransformationSetLoopControl(
                  123, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  123, (uint32_t)spv::LoopControlMask::Unroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  123, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  123,
                  (uint32_t)spv::LoopControlMask::MinIterations |
                      (uint32_t)spv::LoopControlMask::MaxIterations |
                      (uint32_t)spv::LoopControlMask::IterationMultiple |
                      (uint32_t)spv::LoopControlMask::PeelCount |
                      (uint32_t)spv::LoopControlMask::PartialCount,
                  7, 8)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_TRUE(TransformationSetLoopControl(
                  123,
                  (uint32_t)spv::LoopControlMask::Unroll |
                      (uint32_t)spv::LoopControlMask::MinIterations |
                      (uint32_t)spv::LoopControlMask::MaxIterations |
                      (uint32_t)spv::LoopControlMask::PartialCount,
                  0, 9)
                  .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   123,
                   (uint32_t)spv::LoopControlMask::Unroll |
                       (uint32_t)spv::LoopControlMask::MinIterations |
                       (uint32_t)spv::LoopControlMask::MaxIterations |
                       (uint32_t)spv::LoopControlMask::PartialCount,
                   7, 9)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationSetLoopControl(
                   123,
                   (uint32_t)spv::LoopControlMask::DontUnroll |
                       (uint32_t)spv::LoopControlMask::MinIterations |
                       (uint32_t)spv::LoopControlMask::MaxIterations |
                       (uint32_t)spv::LoopControlMask::PartialCount,
                   7, 9)
                   .IsApplicable(context.get(), transformation_context));

  ApplyAndCheckFreshIds(TransformationSetLoopControl(
                            10,
                            (uint32_t)spv::LoopControlMask::Unroll |
                                (uint32_t)spv::LoopControlMask::PeelCount |
                                (uint32_t)spv::LoopControlMask::PartialCount,
                            3, 3),
                        context.get(), &transformation_context);
  ApplyAndCheckFreshIds(
      TransformationSetLoopControl(
          23, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0),
      context.get(), &transformation_context);
  ApplyAndCheckFreshIds(TransformationSetLoopControl(
                            33, (uint32_t)spv::LoopControlMask::Unroll, 0, 0),
                        context.get(), &transformation_context);
  ApplyAndCheckFreshIds(
      TransformationSetLoopControl(
          43,
          (uint32_t)spv::LoopControlMask::DontUnroll |
              (uint32_t)spv::LoopControlMask::DependencyInfinite,
          0, 0),
      context.get(), &transformation_context);
  ApplyAndCheckFreshIds(TransformationSetLoopControl(
                            53, (uint32_t)spv::LoopControlMask::MaskNone, 0, 0),
                        context.get(), &transformation_context);
  ApplyAndCheckFreshIds(TransformationSetLoopControl(
                            63,
                            (uint32_t)spv::LoopControlMask::Unroll |
                                (uint32_t)spv::LoopControlMask::MinIterations |
                                (uint32_t)spv::LoopControlMask::PeelCount,
                            23, 0),
                        context.get(), &transformation_context);
  ApplyAndCheckFreshIds(TransformationSetLoopControl(
                            73,
                            (uint32_t)spv::LoopControlMask::Unroll |
                                (uint32_t)spv::LoopControlMask::MaxIterations |
                                (uint32_t)spv::LoopControlMask::PeelCount,
                            23, 0),
                        context.get(), &transformation_context);
  ApplyAndCheckFreshIds(
      TransformationSetLoopControl(
          83, (uint32_t)spv::LoopControlMask::DontUnroll, 0, 0),
      context.get(), &transformation_context);
  ApplyAndCheckFreshIds(TransformationSetLoopControl(
                            93,
                            (uint32_t)spv::LoopControlMask::PeelCount |
                                (uint32_t)spv::LoopControlMask::PartialCount,
                            16, 8),
                        context.get(), &transformation_context);
  ApplyAndCheckFreshIds(
      TransformationSetLoopControl(
          103, (uint32_t)spv::LoopControlMask::PartialCount, 0, 60),
      context.get(), &transformation_context);
  ApplyAndCheckFreshIds(
      TransformationSetLoopControl(
          113, (uint32_t)spv::LoopControlMask::PeelCount, 12, 0),
      context.get(), &transformation_context);
  ApplyAndCheckFreshIds(TransformationSetLoopControl(
                            123,
                            (uint32_t)spv::LoopControlMask::Unroll |
                                (uint32_t)spv::LoopControlMask::MinIterations |
                                (uint32_t)spv::LoopControlMask::MaxIterations |
                                (uint32_t)spv::LoopControlMask::PartialCount,
                            0, 9),
                        context.get(), &transformation_context);

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
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpConstant %6 100
         %17 = OpTypeBool
         %20 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %22 = OpVariable %7 Function
         %32 = OpVariable %7 Function
         %42 = OpVariable %7 Function
         %52 = OpVariable %7 Function
         %62 = OpVariable %7 Function
         %72 = OpVariable %7 Function
         %82 = OpVariable %7 Function
         %92 = OpVariable %7 Function
        %102 = OpVariable %7 Function
        %112 = OpVariable %7 Function
        %122 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %10 = OpLabel
        %132 = OpPhi %6 %9 %5 %21 %13
               OpLoopMerge %12 %13 Unroll|PeelCount|PartialCount 3 3
               OpBranch %14
         %14 = OpLabel
         %18 = OpSLessThan %17 %132 %16
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %21 = OpIAdd %6 %132 %20
               OpStore %8 %21
               OpBranch %10
         %12 = OpLabel
               OpStore %22 %9
               OpBranch %23
         %23 = OpLabel
        %133 = OpPhi %6 %9 %12 %31 %26
               OpLoopMerge %25 %26 DontUnroll
               OpBranch %27
         %27 = OpLabel
         %29 = OpSLessThan %17 %133 %16
               OpBranchConditional %29 %24 %25
         %24 = OpLabel
               OpBranch %26
         %26 = OpLabel
         %31 = OpIAdd %6 %133 %20
               OpStore %22 %31
               OpBranch %23
         %25 = OpLabel
               OpStore %32 %9
               OpBranch %33
         %33 = OpLabel
        %134 = OpPhi %6 %9 %25 %41 %36
               OpLoopMerge %35 %36 Unroll
               OpBranch %37
         %37 = OpLabel
         %39 = OpSLessThan %17 %134 %16
               OpBranchConditional %39 %34 %35
         %34 = OpLabel
               OpBranch %36
         %36 = OpLabel
         %41 = OpIAdd %6 %134 %20
               OpStore %32 %41
               OpBranch %33
         %35 = OpLabel
               OpStore %42 %9
               OpBranch %43
         %43 = OpLabel
        %135 = OpPhi %6 %9 %35 %51 %46
               OpLoopMerge %45 %46 DontUnroll|DependencyInfinite
               OpBranch %47
         %47 = OpLabel
         %49 = OpSLessThan %17 %135 %16
               OpBranchConditional %49 %44 %45
         %44 = OpLabel
               OpBranch %46
         %46 = OpLabel
         %51 = OpIAdd %6 %135 %20
               OpStore %42 %51
               OpBranch %43
         %45 = OpLabel
               OpStore %52 %9
               OpBranch %53
         %53 = OpLabel
        %136 = OpPhi %6 %9 %45 %61 %56
               OpLoopMerge %55 %56 None
               OpBranch %57
         %57 = OpLabel
         %59 = OpSLessThan %17 %136 %16
               OpBranchConditional %59 %54 %55
         %54 = OpLabel
               OpBranch %56
         %56 = OpLabel
         %61 = OpIAdd %6 %136 %20
               OpStore %52 %61
               OpBranch %53
         %55 = OpLabel
               OpStore %62 %9
               OpBranch %63
         %63 = OpLabel
        %137 = OpPhi %6 %9 %55 %71 %66
               OpLoopMerge %65 %66 Unroll|MinIterations|PeelCount 10 23
               OpBranch %67
         %67 = OpLabel
         %69 = OpSLessThan %17 %137 %16
               OpBranchConditional %69 %64 %65
         %64 = OpLabel
               OpBranch %66
         %66 = OpLabel
         %71 = OpIAdd %6 %137 %20
               OpStore %62 %71
               OpBranch %63
         %65 = OpLabel
               OpStore %72 %9
               OpBranch %73
         %73 = OpLabel
        %138 = OpPhi %6 %9 %65 %81 %76
               OpLoopMerge %75 %76 Unroll|MaxIterations|PeelCount 50 23
               OpBranch %77
         %77 = OpLabel
         %79 = OpSLessThan %17 %138 %16
               OpBranchConditional %79 %74 %75
         %74 = OpLabel
               OpBranch %76
         %76 = OpLabel
         %81 = OpIAdd %6 %138 %20
               OpStore %72 %81
               OpBranch %73
         %75 = OpLabel
               OpStore %82 %9
               OpBranch %83
         %83 = OpLabel
        %139 = OpPhi %6 %9 %75 %91 %86
               OpLoopMerge %85 %86 DontUnroll
               OpBranch %87
         %87 = OpLabel
         %89 = OpSLessThan %17 %139 %16
               OpBranchConditional %89 %84 %85
         %84 = OpLabel
               OpBranch %86
         %86 = OpLabel
         %91 = OpIAdd %6 %139 %20
               OpStore %82 %91
               OpBranch %83
         %85 = OpLabel
               OpStore %92 %9
               OpBranch %93
         %93 = OpLabel
        %140 = OpPhi %6 %9 %85 %101 %96
               OpLoopMerge %95 %96 PeelCount|PartialCount 16 8
               OpBranch %97
         %97 = OpLabel
         %99 = OpSLessThan %17 %140 %16
               OpBranchConditional %99 %94 %95
         %94 = OpLabel
               OpBranch %96
         %96 = OpLabel
        %101 = OpIAdd %6 %140 %20
               OpStore %92 %101
               OpBranch %93
         %95 = OpLabel
               OpStore %102 %9
               OpBranch %103
        %103 = OpLabel
        %141 = OpPhi %6 %9 %95 %111 %106
               OpLoopMerge %105 %106 PartialCount 60
               OpBranch %107
        %107 = OpLabel
        %109 = OpSLessThan %17 %141 %16
               OpBranchConditional %109 %104 %105
        %104 = OpLabel
               OpBranch %106
        %106 = OpLabel
        %111 = OpIAdd %6 %141 %20
               OpStore %102 %111
               OpBranch %103
        %105 = OpLabel
               OpStore %112 %9
               OpBranch %113
        %113 = OpLabel
        %142 = OpPhi %6 %9 %105 %121 %116
               OpLoopMerge %115 %116 PeelCount 12
               OpBranch %117
        %117 = OpLabel
        %119 = OpSLessThan %17 %142 %16
               OpBranchConditional %119 %114 %115
        %114 = OpLabel
               OpBranch %116
        %116 = OpLabel
        %121 = OpIAdd %6 %142 %20
               OpStore %112 %121
               OpBranch %113
        %115 = OpLabel
               OpStore %122 %9
               OpBranch %123
        %123 = OpLabel
        %143 = OpPhi %6 %9 %115 %131 %126
               OpLoopMerge %125 %126 Unroll|MinIterations|MaxIterations|PartialCount 5 90 9
               OpBranch %127
        %127 = OpLabel
        %129 = OpSLessThan %17 %143 %16
               OpBranchConditional %129 %124 %125
        %124 = OpLabel
               OpBranch %126
        %126 = OpLabel
        %131 = OpIAdd %6 %143 %20
               OpStore %122 %131
               OpBranch %123
        %125 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationSetLoopControlTest, CheckSPIRVVersionsRespected) {
  // This test checks that we do not allow introducing PeelCount and
  // PartialCount loop controls if the SPIR-V version being used does not
  // support them.

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
         %16 = OpConstant %6 10
         %17 = OpTypeBool
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
               OpStore %8 %21
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  for (auto env :
       {SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1, SPV_ENV_UNIVERSAL_1_2,
        SPV_ENV_UNIVERSAL_1_3, SPV_ENV_UNIVERSAL_1_4, SPV_ENV_UNIVERSAL_1_5,
        SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1, SPV_ENV_VULKAN_1_1_SPIRV_1_4,
        SPV_ENV_VULKAN_1_2}) {
    const auto consumer = nullptr;
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);
    TransformationSetLoopControl peel_count(
        10, (uint32_t)spv::LoopControlMask::PeelCount, 4, 0);
    TransformationSetLoopControl partial_count(
        10, (uint32_t)spv::LoopControlMask::PartialCount, 0, 4);

    switch (env) {
      case SPV_ENV_UNIVERSAL_1_0:
      case SPV_ENV_UNIVERSAL_1_1:
      case SPV_ENV_UNIVERSAL_1_2:
      case SPV_ENV_UNIVERSAL_1_3:
      case SPV_ENV_VULKAN_1_0:
      case SPV_ENV_VULKAN_1_1:
        // PeelCount and PartialCount were introduced in SPIRV 1.4, so are not
        // valid in the context of older versions.
        ASSERT_FALSE(
            peel_count.IsApplicable(context.get(), transformation_context));
        ASSERT_FALSE(
            partial_count.IsApplicable(context.get(), transformation_context));
        break;
      case SPV_ENV_UNIVERSAL_1_4:
      case SPV_ENV_UNIVERSAL_1_5:
      case SPV_ENV_VULKAN_1_1_SPIRV_1_4:
      case SPV_ENV_VULKAN_1_2:
        ASSERT_TRUE(
            peel_count.IsApplicable(context.get(), transformation_context));
        ASSERT_TRUE(
            partial_count.IsApplicable(context.get(), transformation_context));
        break;
      default:
        assert(false && "Unhandled environment");
        break;
    }
  }
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
