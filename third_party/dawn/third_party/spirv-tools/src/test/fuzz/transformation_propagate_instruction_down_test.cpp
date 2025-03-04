// Copyright (c) 2020 Vasyl Teliman
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

#include "source/fuzz/transformation_propagate_instruction_down.h"

#include "gtest/gtest.h"
#include "source/fuzz/counter_overflow_id_source.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationPropagateInstructionDownTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %9 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          ; Has no instruction to propagate
          %5 = OpLabel
         %10 = OpVariable %9 Function
          %8 = OpCopyObject %6 %7
               OpStore %10 %8
               OpBranch %11

        ; Unreachable block
        %100 = OpLabel
        %101 = OpCopyObject %6 %7
               OpBranch %11

         ; Selection header
         ;
         ; One of acceptable successors has an OpPhi that uses propagated
         ; instruction's id
         %11 = OpLabel
         %19 = OpCopyObject %6 %7
               OpSelectionMerge %18 None
               OpBranchConditional %13 %14 %18

         ; %16 has no acceptable successors
         %14 = OpLabel
         %20 = OpPhi %6 %19 %11
         %15 = OpCopyObject %6 %7 ; dependency
               OpBranch %16
         %16 = OpLabel
         %17 = OpCopyObject %6 %15
               OpBranch %18

         ; Can be applied
         %18 = OpLabel
         %21 = OpCopyObject %6 %7
               OpSelectionMerge %24 None
               OpBranchConditional %13 %22 %23
         %22 = OpLabel
         %29 = OpPhi %6 %7 %18
               OpStore %10 %21
               OpBranch %24
         %23 = OpLabel
               OpStore %10 %21
               OpBranch %24
         %24 = OpLabel
               OpStore %10 %21
               OpBranch %32

         ; Can't replace all uses of the propagated instruction: %30 is
         ; propagated into %27.
         %32 = OpLabel
               OpLoopMerge %28 %27 None
               OpBranchConditional %13 %26 %28
         %26 = OpLabel
         %25 = OpCopyObject %6 %7
         %30 = OpCopyObject %6 %25
               OpBranchConditional %13 %27 %28
         %27 = OpLabel
               OpBranch %32
         %28 = OpLabel
         %31 = OpPhi %6 %30 %26 %7 %32 ; Can't replace this use
               OpReturn

               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  // Invalid block id.
  ASSERT_FALSE(TransformationPropagateInstructionDown(200, 200, {{}})
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationPropagateInstructionDown(101, 200, {{}})
                   .IsApplicable(context.get(), transformation_context));

  // The block is unreachable.
  ASSERT_FALSE(TransformationPropagateInstructionDown(100, 200, {{}})
                   .IsApplicable(context.get(), transformation_context));

  // The block has no instruction to propagate.
  ASSERT_FALSE(TransformationPropagateInstructionDown(5, 200, {{{11, 201}}})
                   .IsApplicable(context.get(), transformation_context));

  // The block has no acceptable successors.
  ASSERT_FALSE(TransformationPropagateInstructionDown(16, 200, {{{18, 201}}})
                   .IsApplicable(context.get(), transformation_context));

  // One of acceptable successors has an OpPhi that uses propagated
  // instruction's id.
  ASSERT_FALSE(
      TransformationPropagateInstructionDown(11, 200, {{{14, 201}, {18, 202}}})
          .IsApplicable(context.get(), transformation_context));

#ifndef NDEBUG
  // Not all fresh ids are provided.
  ASSERT_DEATH(
      TransformationPropagateInstructionDown(18, 200, {{{22, 201}, {202, 203}}})
          .IsApplicable(context.get(), transformation_context),
      "Bad attempt to query whether overflow ids are available.");
#endif

  // Not all fresh ids are fresh.
  ASSERT_FALSE(TransformationPropagateInstructionDown(
                   18, 18, {{{22, 201}, {23, 202}, {202, 203}}})
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationPropagateInstructionDown(
                   18, 200, {{{22, 22}, {23, 202}, {202, 203}}})
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationPropagateInstructionDown(
                   18, 18, {{{22, 22}, {23, 202}, {202, 203}}})
                   .IsApplicable(context.get(), transformation_context));

  // Not all fresh ids are unique.
  ASSERT_FALSE(TransformationPropagateInstructionDown(
                   18, 200, {{{22, 200}, {23, 202}, {202, 200}}})
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationPropagateInstructionDown(
                   18, 200, {{{22, 201}, {23, 202}, {202, 200}}})
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationPropagateInstructionDown(
                   18, 200, {{{22, 201}, {23, 201}, {202, 203}}})
                   .IsApplicable(context.get(), transformation_context));

  // Can't replace all uses of the propagated instruction: %30 is propagated
  // into %27.
  ASSERT_FALSE(TransformationPropagateInstructionDown(26, 200, {{{27, 201}}})
                   .IsApplicable(context.get(), transformation_context));

  {
    TransformationPropagateInstructionDown transformation(
        18, 200, {{{22, 201}, {23, 202}, {202, 203}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));

    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(201, {}), MakeDataDescriptor(202, {})));
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(201, {}), MakeDataDescriptor(200, {})));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %9 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          ; Has no instruction to propagate
          %5 = OpLabel
         %10 = OpVariable %9 Function
          %8 = OpCopyObject %6 %7
               OpStore %10 %8
               OpBranch %11

        ; Unreachable block
        %100 = OpLabel
        %101 = OpCopyObject %6 %7
               OpBranch %11

         ; Selection header
         ;
         ; One of acceptable successors has an OpPhi that uses propagated
         ; instruction's id
         %11 = OpLabel
         %19 = OpCopyObject %6 %7
               OpSelectionMerge %18 None
               OpBranchConditional %13 %14 %18

         ; %16 has no acceptable successors
         %14 = OpLabel
         %20 = OpPhi %6 %19 %11
         %15 = OpCopyObject %6 %7 ; dependency
               OpBranch %16
         %16 = OpLabel
         %17 = OpCopyObject %6 %15
               OpBranch %18

         ; Can be applied
         %18 = OpLabel
               OpSelectionMerge %24 None
               OpBranchConditional %13 %22 %23
         %22 = OpLabel
         %29 = OpPhi %6 %7 %18
        %201 = OpCopyObject %6 %7
               OpStore %10 %201
               OpBranch %24
         %23 = OpLabel
        %202 = OpCopyObject %6 %7
               OpStore %10 %202
               OpBranch %24
         %24 = OpLabel
        %200 = OpPhi %6 %201 %22 %202 %23
               OpStore %10 %200
               OpBranch %32

         ; Can't replace all uses of the propagated instruction: %30 is
         ; propagated into %27.
         %32 = OpLabel
               OpLoopMerge %28 %27 None
               OpBranchConditional %13 %26 %28
         %26 = OpLabel
         %25 = OpCopyObject %6 %7
         %30 = OpCopyObject %6 %25
               OpBranchConditional %13 %27 %28
         %27 = OpLabel
               OpBranch %32
         %28 = OpLabel
         %31 = OpPhi %6 %30 %26 %7 %32 ; Can't replace this use
               OpReturn

               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionDownTest, CantCreateOpPhiTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %4 = OpFunction %2 None %3

          ; %5 doesn't belong to any construct
          %5 = OpLabel
         %15 = OpCopyObject %6 %7
               OpBranch %16

         ; The merge block (%19) is unreachable
         %16 = OpLabel
         %17 = OpCopyObject %6 %7
               OpSelectionMerge %19 None
               OpBranchConditional %13 %18 %18

         ; %21 doesn't dominate the merge block - %20
         %18 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %13 %20 %21
         %21 = OpLabel
         %22 = OpCopyObject %6 %7
               OpBranch %20

         ; The merge block (%24) is an acceptable successor of the propagated
         ; instruction's block
         %20 = OpLabel
         %23 = OpCopyObject %6 %7
               OpSelectionMerge %24 None
               OpBranchConditional %13 %24 %30
         %30 = OpLabel
               OpBranch %24

         ; One of the predecessors of the merge block is not dominated by any
         ; successor of the propagated instruction's block
         %24 = OpLabel
         %26 = OpCopyObject %6 %7
               OpLoopMerge %29 %25 None
               OpBranch %25
         %25 = OpLabel
               OpBranchConditional %13 %24 %29
         %28 = OpLabel ; unreachable predecessor of %29
               OpBranch %29
         %29 = OpLabel
               OpReturn

         %19 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  TransformationPropagateInstructionDown transformations[] = {
      // %5 doesn't belong to any construct.
      {5, 200, {{{16, 201}}}},

      // The merge block (%19) is unreachable.
      {16, 200, {{{18, 202}}}},

      // %21 doesn't dominate the merge block - %20.
      {21, 200, {{{20, 203}}}},

      // The merge block (%24) is an acceptable successor of the propagated
      // instruction's block.
      {20, 200, {{{24, 204}, {30, 205}}}},

      // One of the predecessors of the merge block is not dominated by any
      // successor of the propagated instruction's block.
      {24, 200, {{{25, 206}}}},
  };

  for (const auto& transformation : transformations) {
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  // No transformation has introduced an OpPhi instruction.
  ASSERT_FALSE(context->get_def_use_mgr()->GetDef(200));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %4 = OpFunction %2 None %3

          ; %5 doesn't belong to any construct
          %5 = OpLabel
               OpBranch %16

         ; The merge block (%19) is unreachable
         %16 = OpLabel
        %201 = OpCopyObject %6 %7
               OpSelectionMerge %19 None
               OpBranchConditional %13 %18 %18

         ; %21 doesn't dominate the merge block - %20
         %18 = OpLabel
        %202 = OpCopyObject %6 %7
               OpSelectionMerge %20 None
               OpBranchConditional %13 %20 %21
         %21 = OpLabel
               OpBranch %20

         ; The merge block (%24) is an acceptable successor of the propagated
         ; instruction's block
         %20 = OpLabel
        %203 = OpCopyObject %6 %7
               OpSelectionMerge %24 None
               OpBranchConditional %13 %24 %30
         %30 = OpLabel
        %205 = OpCopyObject %6 %7
               OpBranch %24

         ; One of the predecessors of the merge block is not dominated by any
         ; successor of the propagated instruction's block
         %24 = OpLabel
        %204 = OpCopyObject %6 %7
               OpLoopMerge %29 %25 None
               OpBranch %25
         %25 = OpLabel
        %206 = OpCopyObject %6 %7
               OpBranchConditional %13 %24 %29
         %28 = OpLabel ; unreachable predecessor of %29
               OpBranch %29
         %29 = OpLabel
               OpReturn

         %19 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionDownTest, VariablePointersCapability) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
         %10 = OpTypePointer Workgroup %6
         %11 = OpVariable %10 Workgroup
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %18 = OpCopyObject %10 %11
         %14 = OpCopyObject %10 %11
               OpSelectionMerge %17 None
               OpBranchConditional %13 %15 %16
         %15 = OpLabel
               OpBranch %17
         %16 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpStore %18 %7
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  {
    // Can propagate a pointer only if we don't have to create an OpPhi.
    TransformationPropagateInstructionDown transformation(
        5, 200, {{{15, 201}, {16, 202}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_FALSE(context->get_def_use_mgr()->GetDef(200));
  }
  {
    // Can't propagate a pointer if there is no VariablePointersStorageBuffer
    // capability and we need to create an OpPhi.
    TransformationPropagateInstructionDown transformation(
        5, 200, {{{15, 203}, {16, 204}}});
    ASSERT_FALSE(context->get_feature_mgr()->HasCapability(
        spv::Capability::VariablePointersStorageBuffer));
    ASSERT_FALSE(
        transformation.IsApplicable(context.get(), transformation_context));

    context->AddCapability(spv::Capability::VariablePointers);
    ASSERT_TRUE(context->get_feature_mgr()->HasCapability(
        spv::Capability::VariablePointersStorageBuffer));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  std::string after_transformation = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
         %10 = OpTypePointer Workgroup %6
         %11 = OpVariable %10 Workgroup
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %17 None
               OpBranchConditional %13 %15 %16
         %15 = OpLabel
        %203 = OpCopyObject %10 %11
        %201 = OpCopyObject %10 %11
               OpBranch %17
         %16 = OpLabel
        %204 = OpCopyObject %10 %11
        %202 = OpCopyObject %10 %11
               OpBranch %17
         %17 = OpLabel
        %200 = OpPhi %10 %203 %15 %204 %16
               OpStore %200 %7
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionDownTest, UseOverflowIdsTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
         %10 = OpTypePointer Private %6
         %11 = OpVariable %10 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpCopyObject %6 %7
               OpSelectionMerge %23 None
               OpBranchConditional %13 %21 %22
         %21 = OpLabel
               OpStore %11 %20
               OpBranch %23
         %22 = OpLabel
               OpStore %11 %20
               OpBranch %23
         %23 = OpLabel
               OpStore %11 %20
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options,
      MakeUnique<CounterOverflowIdSource>(300));

  TransformationPropagateInstructionDown transformation(5, 200, {{{21, 201}}});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context,
                        {300});
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
         %10 = OpTypePointer Private %6
         %11 = OpVariable %10 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %23 None
               OpBranchConditional %13 %21 %22
         %21 = OpLabel
        %201 = OpCopyObject %6 %7
               OpStore %11 %201
               OpBranch %23
         %22 = OpLabel
        %300 = OpCopyObject %6 %7
               OpStore %11 %300
               OpBranch %23
         %23 = OpLabel
        %200 = OpPhi %6 %201 %21 %300 %22
               OpStore %11 %200
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionDownTest, TestCreatedFacts) {
  std::string shader = R"(
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
         %10 = OpTypePointer Private %6
         %11 = OpVariable %10 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpCopyObject %6 %7
         %24 = OpCopyObject %6 %7 ; Irrelevant id
         %25 = OpCopyObject %10 %11 ; Pointee is irrelevant
               OpSelectionMerge %23 None
               OpBranchConditional %13 %21 %22
         %21 = OpLabel
               OpStore %25 %20
               OpBranch %23
         %22 = OpLabel ; Dead block
               OpStore %25 %20
               OpBranch %23
         %23 = OpLabel
               OpStore %25 %20
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  transformation_context.GetFactManager()->AddFactBlockIsDead(22);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(24);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      25);

  {
    // Propagate pointer with PointeeIsIrrelevant fact.
    TransformationPropagateInstructionDown transformation(
        5, 200, {{{21, 201}, {22, 202}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));

    ASSERT_FALSE(transformation_context.GetFactManager()->IdIsIrrelevant(201));
    ASSERT_FALSE(transformation_context.GetFactManager()->IdIsIrrelevant(202));
    ASSERT_FALSE(transformation_context.GetFactManager()->IdIsIrrelevant(200));

    ASSERT_TRUE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(201));
    ASSERT_TRUE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(202));
    ASSERT_TRUE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(200));

    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(201, {}), MakeDataDescriptor(202, {})));
    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(201, {}), MakeDataDescriptor(200, {})));
  }
  {
    // Propagate an irrelevant id.
    TransformationPropagateInstructionDown transformation(
        5, 203, {{{21, 204}, {22, 205}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));

    ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(203));
    ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(204));
    ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(205));

    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(203));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(204));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(205));

    ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(204, {}), MakeDataDescriptor(205, {})));
    ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(204, {}), MakeDataDescriptor(203, {})));
  }
  {
    // Propagate a regular id.
    TransformationPropagateInstructionDown transformation(
        5, 206, {{{21, 207}, {22, 208}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));

    ASSERT_FALSE(transformation_context.GetFactManager()->IdIsIrrelevant(206));
    ASSERT_FALSE(transformation_context.GetFactManager()->IdIsIrrelevant(207));
    ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(208));

    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(206));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(207));
    ASSERT_FALSE(
        transformation_context.GetFactManager()->PointeeValueIsIrrelevant(208));

    ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(206, {}), MakeDataDescriptor(207, {})));
    ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
        MakeDataDescriptor(206, {}), MakeDataDescriptor(208, {})));
  }

  std::string after_transformation = R"(
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
         %10 = OpTypePointer Private %6
         %11 = OpVariable %10 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %23 None
               OpBranchConditional %13 %21 %22
         %21 = OpLabel
        %207 = OpCopyObject %6 %7
        %204 = OpCopyObject %6 %7 ; Irrelevant id
        %201 = OpCopyObject %10 %11 ; Pointee is irrelevant
               OpStore %201 %207
               OpBranch %23
         %22 = OpLabel ; Dead block
        %208 = OpCopyObject %6 %7
        %205 = OpCopyObject %6 %7 ; Irrelevant id
        %202 = OpCopyObject %10 %11 ; Pointee is irrelevant
               OpStore %202 %208
               OpBranch %23
         %23 = OpLabel
        %206 = OpPhi %6 %207 %21 %208 %22
        %203 = OpPhi %6 %204 %21 %205 %22 ; Irrelevant id
        %200 = OpPhi %10 %201 %21 %202 %22 ; Pointee is irrelevant
               OpStore %200 %206
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionDownTest, TestLoops1) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %20

         %20 = OpLabel
               OpLoopMerge %26 %25 None
               OpBranch %21

         %21 = OpLabel
         %22 = OpCopyObject %6 %7
         %31 = OpCopyObject %6 %7
               OpSelectionMerge %35 None
               OpBranchConditional %13 %23 %24

         %23 = OpLabel
         %27 = OpCopyObject %6 %22
         %32 = OpCopyObject %6 %31
               OpBranch %26
         %24 = OpLabel
         %28 = OpCopyObject %6 %22
         %33 = OpCopyObject %6 %31
               OpBranchConditional %13 %26 %25

         %35 = OpLabel
               OpBranch %25

         %25 = OpLabel
         %29 = OpCopyObject %6 %22
         %34 = OpCopyObject %6 %31
               OpBranch %20
         %26 = OpLabel
         %30 = OpCopyObject %6 %22
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  {
    TransformationPropagateInstructionDown transformation(
        21, 200, {{{23, 201}, {24, 202}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  // Can't replace usage of %22 in %26.
  ASSERT_FALSE(
      TransformationPropagateInstructionDown(21, 200, {{{23, 201}, {24, 202}}})
          .IsApplicable(context.get(), transformation_context));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %20

         %20 = OpLabel
               OpLoopMerge %26 %25 None
               OpBranch %21

         %21 = OpLabel
         %22 = OpCopyObject %6 %7
               OpSelectionMerge %35 None
               OpBranchConditional %13 %23 %24
         %23 = OpLabel
        %201 = OpCopyObject %6 %7
         %27 = OpCopyObject %6 %22
         %32 = OpCopyObject %6 %201
               OpBranch %26
         %24 = OpLabel
        %202 = OpCopyObject %6 %7
         %28 = OpCopyObject %6 %22
         %33 = OpCopyObject %6 %202
               OpBranchConditional %13 %26 %25

         %35 = OpLabel
               OpBranch %25

         %25 = OpLabel
         %29 = OpCopyObject %6 %22
         %34 = OpCopyObject %6 %202
               OpBranch %20
         %26 = OpLabel
         %30 = OpCopyObject %6 %22
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionDownTest, TestLoops2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %20

         %20 = OpLabel
         %23 = OpPhi %6 %7 %5 %24 %21
               OpLoopMerge %22 %21 None
               OpBranch %21

         %21 = OpLabel
         %24 = OpCopyObject %6 %23
         %25 = OpCopyObject %6 %7
               OpBranchConditional %13 %22 %20

         %22 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  {
    // Can propagate %25 from %21 into %20.
    TransformationPropagateInstructionDown transformation(
        21, 200, {{{20, 201}, {22, 202}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
  {
    // Can propagate %201 from %20 into %21.
    TransformationPropagateInstructionDown transformation(20, 200,
                                                          {{{21, 203}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  // Can't propagate %24 from %21 into %20.
  ASSERT_FALSE(
      TransformationPropagateInstructionDown(21, 200, {{{20, 204}, {22, 205}}})
          .IsApplicable(context.get(), transformation_context));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %20

         %20 = OpLabel
         %23 = OpPhi %6 %7 %5 %24 %21
               OpLoopMerge %22 %21 None
               OpBranch %21

         %21 = OpLabel
        %203 = OpCopyObject %6 %7
         %24 = OpCopyObject %6 %23
               OpBranchConditional %13 %22 %20

         %22 = OpLabel
        %200 = OpPhi %6 %203 %21
        %202 = OpCopyObject %6 %7
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationPropagateInstructionDownTest, TestLoops3) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %20

         %20 = OpLabel
         %27 = OpPhi %6 %7 %5 %26 %20
         %25 = OpCopyObject %6 %7
         %26 = OpCopyObject %6 %7
               OpLoopMerge %22 %20 None
               OpBranchConditional %13 %20 %22

         %22 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  {
    // Propagate %25 into %20 and %22. Not that we are skipping %26 since not
    // all of its users are in different blocks (%27).h
    TransformationPropagateInstructionDown transformation(
        20, 200, {{{20, 201}, {22, 202}}});
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 1
         %12 = OpTypeBool
         %13 = OpConstantTrue %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %20

         %20 = OpLabel
         %27 = OpPhi %6 %7 %5 %26 %20
        %201 = OpCopyObject %6 %7
         %26 = OpCopyObject %6 %7
               OpLoopMerge %22 %20 None
               OpBranchConditional %13 %20 %22

         %22 = OpLabel
        %202 = OpCopyObject %6 %7
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
