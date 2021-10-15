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

#include "source/fuzz/transformation_flatten_conditional_branch.h"

#include "gtest/gtest.h"
#include "source/fuzz/counter_overflow_id_source.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

protobufs::SideEffectWrapperInfo MakeSideEffectWrapperInfo(
    const protobufs::InstructionDescriptor& instruction,
    uint32_t merge_block_id, uint32_t execute_block_id,
    uint32_t actual_result_id, uint32_t alternative_block_id,
    uint32_t placeholder_result_id, uint32_t value_to_copy_id) {
  protobufs::SideEffectWrapperInfo result;
  *result.mutable_instruction() = instruction;
  result.set_merge_block_id(merge_block_id);
  result.set_execute_block_id(execute_block_id);
  result.set_actual_result_id(actual_result_id);
  result.set_alternative_block_id(alternative_block_id);
  result.set_placeholder_result_id(placeholder_result_id);
  result.set_value_to_copy_id(value_to_copy_id);
  return result;
}

protobufs::SideEffectWrapperInfo MakeSideEffectWrapperInfo(
    const protobufs::InstructionDescriptor& instruction,
    uint32_t merge_block_id, uint32_t execute_block_id) {
  return MakeSideEffectWrapperInfo(instruction, merge_block_id,
                                   execute_block_id, 0, 0, 0, 0);
}

TEST(TransformationFlattenConditionalBranchTest, Inapplicable) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main" %3
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %4 = OpTypeVoid
          %5 = OpTypeFunction %4
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 0
          %9 = OpTypeBool
         %10 = OpConstantTrue %9
         %11 = OpTypePointer Function %6
         %12 = OpTypePointer Workgroup %6
          %3 = OpVariable %12 Workgroup
         %13 = OpConstant %6 2
          %2 = OpFunction %4 None %5
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
               OpSelectionMerge %16 None
               OpSwitch %13 %17 2 %18
         %17 = OpLabel
               OpBranch %16
         %18 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpLoopMerge %19 %16 None
               OpBranchConditional %10 %16 %19
         %19 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %10 %21 %20
         %21 = OpLabel
               OpReturn
         %20 = OpLabel
               OpSelectionMerge %22 None
               OpBranchConditional %10 %23 %22
         %23 = OpLabel
               OpSelectionMerge %24 None
               OpBranchConditional %10 %25 %24
         %25 = OpLabel
               OpBranch %24
         %24 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpSelectionMerge %26 None
               OpBranchConditional %10 %26 %27
         %27 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpLoopMerge %29 %28 None
               OpBranchConditional %10 %28 %29
         %29 = OpLabel
               OpBranch %26
         %26 = OpLabel
               OpSelectionMerge %30 None
               OpBranchConditional %10 %30 %31
         %31 = OpLabel
               OpBranch %32
         %32 = OpLabel
         %33 = OpAtomicLoad %6 %3 %8 %8
               OpBranch %30
         %30 = OpLabel
               OpSelectionMerge %34 None
               OpBranchConditional %10 %35 %34
         %35 = OpLabel
               OpMemoryBarrier %8 %8
               OpBranch %34
         %34 = OpLabel
               OpLoopMerge %40 %39 None
               OpBranchConditional %10 %36 %40
         %36 = OpLabel
               OpSelectionMerge %38 None
               OpBranchConditional %10 %37 %38
         %37 = OpLabel
               OpBranch %40
         %38 = OpLabel
               OpBranch %39
         %39 = OpLabel
               OpBranch %34
         %40 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Block %15 does not end with OpBranchConditional.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(15, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));

  // Block %17 is not a selection header.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(17, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));

  // Block %16 is a loop header, not a selection header.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(16, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));

  // Block %19 and the corresponding merge block do not describe a single-entry,
  // single-exit region, because there is a return instruction in %21.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(19, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));

  // Block %20 is the header of a construct containing an inner selection
  // construct.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(20, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));

  // Block %22 is the header of a construct containing an inner loop.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(22, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));

  // Block %30 is the header of a construct containing a barrier instruction.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(30, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));

  // %33 is not a block.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(33, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));

  // Block %36 and the corresponding merge block do not describe a single-entry,
  // single-exit region, because block %37 breaks out of the outer loop.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(36, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationFlattenConditionalBranchTest, Simple) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeBool
          %4 = OpConstantTrue %3
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %2 = OpFunction %5 None %6
          %7 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %4 %9 %10
         %10 = OpLabel
         %26 = OpPhi %3 %4 %7
               OpBranch %8
          %9 = OpLabel
         %27 = OpPhi %3 %4 %7
         %11 = OpCopyObject %3 %4
               OpBranch %8
          %8 = OpLabel
         %12 = OpPhi %3 %11 %9 %4 %10
         %23 = OpPhi %3 %4 %9 %4 %10
               OpBranch %13
         %13 = OpLabel
         %14 = OpCopyObject %3 %4
               OpSelectionMerge %15 None
               OpBranchConditional %4 %16 %17
         %16 = OpLabel
         %28 = OpPhi %3 %4 %13
               OpBranch %18
         %18 = OpLabel
               OpBranch %19
         %17 = OpLabel
         %29 = OpPhi %3 %4 %13
         %20 = OpCopyObject %3 %4
               OpBranch %19
         %19 = OpLabel
         %21 = OpPhi %3 %4 %18 %20 %17
               OpBranch %15
         %15 = OpLabel
               OpSelectionMerge %22 None
               OpBranchConditional %4 %22 %22
         %22 = OpLabel
         %30 = OpPhi %3 %4 %15
               OpSelectionMerge %25 None
               OpBranchConditional %4 %24 %24
         %24 = OpLabel
               OpBranch %25
         %25 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto transformation1 =
      TransformationFlattenConditionalBranch(7, true, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);

  auto transformation2 =
      TransformationFlattenConditionalBranch(13, false, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);

  auto transformation3 =
      TransformationFlattenConditionalBranch(15, true, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);

  auto transformation4 =
      TransformationFlattenConditionalBranch(22, false, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation4, context.get(),
                        &transformation_context);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
          %3 = OpTypeBool
          %4 = OpConstantTrue %3
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %2 = OpFunction %5 None %6
          %7 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %27 = OpPhi %3 %4 %7
         %11 = OpCopyObject %3 %4
               OpBranch %10
         %10 = OpLabel
         %26 = OpPhi %3 %4 %9
               OpBranch %8
          %8 = OpLabel
         %12 = OpSelect %3 %4 %11 %4
         %23 = OpSelect %3 %4 %4 %4
               OpBranch %13
         %13 = OpLabel
         %14 = OpCopyObject %3 %4
               OpBranch %17
         %17 = OpLabel
         %29 = OpPhi %3 %4 %13
         %20 = OpCopyObject %3 %4
               OpBranch %16
         %16 = OpLabel
         %28 = OpPhi %3 %4 %17
               OpBranch %18
         %18 = OpLabel
               OpBranch %19
         %19 = OpLabel
         %21 = OpSelect %3 %4 %4 %20
               OpBranch %15
         %15 = OpLabel
               OpBranch %22
         %22 = OpLabel
         %30 = OpPhi %3 %4 %15
               OpBranch %24
         %24 = OpLabel
               OpBranch %25
         %25 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest, LoadStoreFunctionCall) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
         %11 = OpTypeInt 32 1
         %12 = OpTypeVector %11 4
         %13 = OpTypeFunction %11
         %70 = OpConstant %11 0
         %14 = OpConstant %11 1
         %15 = OpTypeFloat 32
         %16 = OpTypeVector %15 2
         %17 = OpConstant %15 1
         %18 = OpConstantComposite %16 %17 %17
         %19 = OpTypeBool
         %20 = OpConstantTrue %19
         %21 = OpTypePointer Function %11
         %22 = OpTypeSampler
         %23 = OpTypeImage %9 2D 2 0 0 1 Unknown
         %24 = OpTypeSampledImage %23
         %25 = OpTypePointer Function %23
         %26 = OpTypePointer Function %22
         %27 = OpTypeInt 32 0
         %28 = OpConstant %27 2
         %29 = OpTypeArray %11 %28
         %30 = OpTypePointer Function %29
          %2 = OpFunction %9 None %10
         %31 = OpLabel
          %4 = OpVariable %21 Function
          %5 = OpVariable %30 Function
         %32 = OpVariable %25 Function
         %33 = OpVariable %26 Function
         %34 = OpLoad %23 %32
         %35 = OpLoad %22 %33
               OpSelectionMerge %36 None
               OpBranchConditional %20 %37 %36
         %37 = OpLabel
          %6 = OpLoad %11 %4
          %7 = OpIAdd %11 %6 %14
               OpStore %4 %7
               OpBranch %36
         %36 = OpLabel
         %42 = OpPhi %11 %14 %37 %14 %31
               OpSelectionMerge %43 None
               OpBranchConditional %20 %44 %45
         %44 = OpLabel
          %8 = OpFunctionCall %11 %3
               OpStore %4 %8
               OpBranch %46
         %45 = OpLabel
         %47 = OpAccessChain %21 %5 %14
               OpStore %47 %14
               OpBranch %46
         %46 = OpLabel
               OpStore %4 %14
               OpBranch %43
         %43 = OpLabel
               OpStore %4 %14
               OpSelectionMerge %48 None
               OpBranchConditional %20 %49 %48
         %49 = OpLabel
               OpBranch %48
         %48 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %20 %51 %50
         %51 = OpLabel
         %52 = OpSampledImage %24 %34 %35
         %53 = OpLoad %11 %4
         %54 = OpImageSampleImplicitLod %12 %52 %18
               OpBranch %50
         %50 = OpLabel
               OpReturn
               OpFunctionEnd
          %3 = OpFunction %11 None %13
         %55 = OpLabel
               OpReturnValue %14
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
#ifndef NDEBUG
  // The following checks lead to assertion failures, since some entries
  // requiring fresh ids are not present in the map, and the transformation
  // context does not have a source overflow ids.

  ASSERT_DEATH(TransformationFlattenConditionalBranch(31, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context),
               "Bad attempt to query whether overflow ids are available.");

  ASSERT_DEATH(TransformationFlattenConditionalBranch(
                   31, true, 0, 0, 0,
                   {{MakeSideEffectWrapperInfo(
                       MakeInstructionDescriptor(6, SpvOpLoad, 0), 100, 101,
                       102, 103, 104, 14)}})
                   .IsApplicable(context.get(), transformation_context),
               "Bad attempt to query whether overflow ids are available.");
#endif

  // The map maps from an instruction to a list with not enough fresh ids.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(
                   31, true, 0, 0, 0,
                   {{MakeSideEffectWrapperInfo(
                       MakeInstructionDescriptor(6, SpvOpLoad, 0), 100, 101,
                       102, 103, 0, 0)}})
                   .IsApplicable(context.get(), transformation_context));

  // Not all fresh ids given are distinct.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(
                   31, true, 0, 0, 0,
                   {{MakeSideEffectWrapperInfo(
                       MakeInstructionDescriptor(6, SpvOpLoad, 0), 100, 100,
                       102, 103, 104, 0)}})
                   .IsApplicable(context.get(), transformation_context));

  // %48 heads a construct containing an OpSampledImage instruction.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(
                   48, true, 0, 0, 0,
                   {{MakeSideEffectWrapperInfo(
                       MakeInstructionDescriptor(53, SpvOpLoad, 0), 100, 101,
                       102, 103, 104, 0)}})
                   .IsApplicable(context.get(), transformation_context));

  // %0 is not a valid id.
  ASSERT_FALSE(
      TransformationFlattenConditionalBranch(
          31, true, 0, 0, 0,
          {MakeSideEffectWrapperInfo(MakeInstructionDescriptor(6, SpvOpLoad, 0),
                                     104, 100, 101, 102, 103, 0),
           MakeSideEffectWrapperInfo(
               MakeInstructionDescriptor(6, SpvOpStore, 0), 106, 105)})
          .IsApplicable(context.get(), transformation_context));

  // %17 is a float constant, while %6 has int type.
  ASSERT_FALSE(
      TransformationFlattenConditionalBranch(
          31, true, 0, 0, 0,
          {MakeSideEffectWrapperInfo(MakeInstructionDescriptor(6, SpvOpLoad, 0),
                                     104, 100, 101, 102, 103, 17),
           MakeSideEffectWrapperInfo(
               MakeInstructionDescriptor(6, SpvOpStore, 0), 106, 105)})
          .IsApplicable(context.get(), transformation_context));

  auto transformation1 = TransformationFlattenConditionalBranch(
      31, true, 0, 0, 0,
      {MakeSideEffectWrapperInfo(MakeInstructionDescriptor(6, SpvOpLoad, 0),
                                 104, 100, 101, 102, 103, 70),
       MakeSideEffectWrapperInfo(MakeInstructionDescriptor(6, SpvOpStore, 0),
                                 106, 105)});
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);

  // Check that the placeholder id was marked as irrelevant.
  ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(103));

  // Make a new transformation context with a source of overflow ids.
  auto overflow_ids_unique_ptr = MakeUnique<CounterOverflowIdSource>(1000);
  auto overflow_ids_ptr = overflow_ids_unique_ptr.get();
  TransformationContext new_transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options,
      std::move(overflow_ids_unique_ptr));

  auto transformation2 = TransformationFlattenConditionalBranch(
      36, false, 0, 0, 0,
      {MakeSideEffectWrapperInfo(MakeInstructionDescriptor(8, SpvOpStore, 0),
                                 114, 113)});
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), new_transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &new_transformation_context,
                        overflow_ids_ptr->GetIssuedOverflowIds());

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
         %11 = OpTypeInt 32 1
         %12 = OpTypeVector %11 4
         %13 = OpTypeFunction %11
         %70 = OpConstant %11 0
         %14 = OpConstant %11 1
         %15 = OpTypeFloat 32
         %16 = OpTypeVector %15 2
         %17 = OpConstant %15 1
         %18 = OpConstantComposite %16 %17 %17
         %19 = OpTypeBool
         %20 = OpConstantTrue %19
         %21 = OpTypePointer Function %11
         %22 = OpTypeSampler
         %23 = OpTypeImage %9 2D 2 0 0 1 Unknown
         %24 = OpTypeSampledImage %23
         %25 = OpTypePointer Function %23
         %26 = OpTypePointer Function %22
         %27 = OpTypeInt 32 0
         %28 = OpConstant %27 2
         %29 = OpTypeArray %11 %28
         %30 = OpTypePointer Function %29
          %2 = OpFunction %9 None %10
         %31 = OpLabel
          %4 = OpVariable %21 Function
          %5 = OpVariable %30 Function
         %32 = OpVariable %25 Function
         %33 = OpVariable %26 Function
         %34 = OpLoad %23 %32
         %35 = OpLoad %22 %33
               OpBranch %37
         %37 = OpLabel
               OpSelectionMerge %104 None
               OpBranchConditional %20 %100 %102
        %100 = OpLabel
        %101 = OpLoad %11 %4
               OpBranch %104
        %102 = OpLabel
        %103 = OpCopyObject %11 %70
               OpBranch %104
        %104 = OpLabel
          %6 = OpPhi %11 %101 %100 %103 %102
          %7 = OpIAdd %11 %6 %14
               OpSelectionMerge %106 None
               OpBranchConditional %20 %105 %106
        %105 = OpLabel
               OpStore %4 %7
               OpBranch %106
        %106 = OpLabel
               OpBranch %36
         %36 = OpLabel
         %42 = OpSelect %11 %20 %14 %14
               OpBranch %45
         %45 = OpLabel
         %47 = OpAccessChain %21 %5 %14
               OpSelectionMerge %1005 None
               OpBranchConditional %20 %1005 %1006
       %1006 = OpLabel
               OpStore %47 %14
               OpBranch %1005
       %1005 = OpLabel
               OpBranch %44
         %44 = OpLabel
               OpSelectionMerge %1000 None
               OpBranchConditional %20 %1001 %1003
       %1001 = OpLabel
       %1002 = OpFunctionCall %11 %3
               OpBranch %1000
       %1003 = OpLabel
       %1004 = OpCopyObject %11 %70
               OpBranch %1000
       %1000 = OpLabel
          %8 = OpPhi %11 %1002 %1001 %1004 %1003
               OpSelectionMerge %114 None
               OpBranchConditional %20 %113 %114
        %113 = OpLabel
               OpStore %4 %8
               OpBranch %114
        %114 = OpLabel
               OpBranch %46
         %46 = OpLabel
               OpStore %4 %14
               OpBranch %43
         %43 = OpLabel
               OpStore %4 %14
               OpSelectionMerge %48 None
               OpBranchConditional %20 %49 %48
         %49 = OpLabel
               OpBranch %48
         %48 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %20 %51 %50
         %51 = OpLabel
         %52 = OpSampledImage %24 %34 %35
         %53 = OpLoad %11 %4
         %54 = OpImageSampleImplicitLod %12 %52 %18
               OpBranch %50
         %50 = OpLabel
               OpReturn
               OpFunctionEnd
          %3 = OpFunction %11 None %13
         %55 = OpLabel
               OpReturnValue %14
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}  // namespace

TEST(TransformationFlattenConditionalBranchTest, EdgeCases) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
          %6 = OpTypeFunction %3
          %2 = OpFunction %3 None %6
          %7 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %5 %9 %8
          %9 = OpLabel
         %10 = OpFunctionCall %3 %11
               OpBranch %8
          %8 = OpLabel
               OpSelectionMerge %12 None
               OpBranchConditional %5 %13 %12
         %13 = OpLabel
         %14 = OpFunctionCall %3 %11
               OpBranch %12
         %12 = OpLabel
               OpReturn
         %16 = OpLabel
               OpSelectionMerge %17 None
               OpBranchConditional %5 %18 %17
         %18 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %3 None %6
         %19 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpSelectionMerge %25 None
               OpBranchConditional %5 %21 %22
         %21 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpSelectionMerge %24 None
               OpBranchConditional %5 %24 %23
         %23 = OpLabel
               OpBranch %24
         %24 = OpLabel
               OpBranch %25
         %25 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
#ifndef NDEBUG
  // The selection construct headed by %7 requires fresh ids because it contains
  // a function call. This causes an assertion failure because transformation
  // context does not have a source of overflow ids.
  ASSERT_DEATH(TransformationFlattenConditionalBranch(7, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context),
               "Bad attempt to query whether overflow ids are available.");
#endif

  auto transformation1 = TransformationFlattenConditionalBranch(
      7, true, 0, 0, 0,
      {{MakeSideEffectWrapperInfo(
          MakeInstructionDescriptor(10, SpvOpFunctionCall, 0), 100, 101)}});
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);

  // The selection construct headed by %8 cannot be flattened because it
  // contains a function call returning void, whose result id is used.
  ASSERT_FALSE(
      TransformationFlattenConditionalBranch(
          7, true, 0, 0, 0,
          {{MakeSideEffectWrapperInfo(
              MakeInstructionDescriptor(14, SpvOpFunctionCall, 0), 102, 103)}})
          .IsApplicable(context.get(), transformation_context));

  // Block %16 is unreachable.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(16, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));

  auto transformation2 =
      TransformationFlattenConditionalBranch(20, false, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
          %6 = OpTypeFunction %3
          %2 = OpFunction %3 None %6
          %7 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpSelectionMerge %100 None
               OpBranchConditional %5 %101 %100
        %101 = OpLabel
         %10 = OpFunctionCall %3 %11
               OpBranch %100
        %100 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpSelectionMerge %12 None
               OpBranchConditional %5 %13 %12
         %13 = OpLabel
         %14 = OpFunctionCall %3 %11
               OpBranch %12
         %12 = OpLabel
               OpReturn
         %16 = OpLabel
               OpSelectionMerge %17 None
               OpBranchConditional %5 %18 %17
         %18 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %3 None %6
         %19 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpBranch %21
         %21 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpSelectionMerge %24 None
               OpBranchConditional %5 %24 %23
         %23 = OpLabel
               OpBranch %24
         %24 = OpLabel
               OpBranch %25
         %25 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest, PhiToSelect1) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
         %10 = OpConstantFalse %4
          %6 = OpTypeFunction %3
          %2 = OpFunction %3 None %6
          %7 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %5 %9 %8
          %9 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %4 %5 %9 %10 %7
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  auto transformation =
      TransformationFlattenConditionalBranch(7, true, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
         %10 = OpConstantFalse %4
          %6 = OpTypeFunction %3
          %2 = OpFunction %3 None %6
          %7 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpSelect %4 %5 %5 %10
               OpReturn
               OpFunctionEnd
)";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest, PhiToSelect2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
         %10 = OpConstantFalse %4
          %6 = OpTypeFunction %3
          %2 = OpFunction %3 None %6
          %7 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %5 %9 %8
          %9 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %4 %10 %7 %5 %9
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  auto transformation =
      TransformationFlattenConditionalBranch(7, true, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
         %10 = OpConstantFalse %4
          %6 = OpTypeFunction %3
          %2 = OpFunction %3 None %6
          %7 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpSelect %4 %5 %5 %10
               OpReturn
               OpFunctionEnd
)";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest, PhiToSelect3) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
         %10 = OpConstantFalse %4
          %6 = OpTypeFunction %3
          %2 = OpFunction %3 None %6
          %7 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %5 %9 %12
          %9 = OpLabel
               OpBranch %8
         %12 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %4 %10 %12 %5 %9
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  auto transformation =
      TransformationFlattenConditionalBranch(7, true, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
         %10 = OpConstantFalse %4
          %6 = OpTypeFunction %3
          %2 = OpFunction %3 None %6
          %7 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranch %12
         %12 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpSelect %4 %5 %5 %10
               OpReturn
               OpFunctionEnd
)";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest, PhiToSelect4) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
         %10 = OpConstantFalse %4
          %6 = OpTypeFunction %3
          %2 = OpFunction %3 None %6
          %7 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %5 %9 %12
          %9 = OpLabel
               OpBranch %8
         %12 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %4 %5 %9 %10 %12
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  auto transformation =
      TransformationFlattenConditionalBranch(7, true, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
         %10 = OpConstantFalse %4
          %6 = OpTypeFunction %3
          %2 = OpFunction %3 None %6
          %7 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranch %12
         %12 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpSelect %4 %5 %5 %10
               OpReturn
               OpFunctionEnd
)";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest, PhiToSelect5) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
         %10 = OpConstantFalse %4
          %6 = OpTypeFunction %3
        %100 = OpTypePointer Function %4
          %2 = OpFunction %3 None %6
          %7 = OpLabel
        %101 = OpVariable %100 Function
        %102 = OpVariable %100 Function
               OpSelectionMerge %470 None
               OpBranchConditional %5 %454 %462
        %454 = OpLabel
        %522 = OpLoad %4 %101
               OpBranch %470
        %462 = OpLabel
        %466 = OpLoad %4 %102
               OpBranch %470
        %470 = OpLabel
        %534 = OpPhi %4 %522 %454 %466 %462
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  auto transformation = TransformationFlattenConditionalBranch(
      7, true, 0, 0, 0,
      {MakeSideEffectWrapperInfo(MakeInstructionDescriptor(522, SpvOpLoad, 0),
                                 200, 201, 202, 203, 204, 5),
       MakeSideEffectWrapperInfo(MakeInstructionDescriptor(466, SpvOpLoad, 0),
                                 300, 301, 302, 303, 304, 5)});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeBool
          %5 = OpConstantTrue %4
         %10 = OpConstantFalse %4
          %6 = OpTypeFunction %3
        %100 = OpTypePointer Function %4
          %2 = OpFunction %3 None %6
          %7 = OpLabel
        %101 = OpVariable %100 Function
        %102 = OpVariable %100 Function
               OpBranch %454
        %454 = OpLabel
               OpSelectionMerge %200 None
               OpBranchConditional %5 %201 %203
        %201 = OpLabel
        %202 = OpLoad %4 %101
               OpBranch %200
        %203 = OpLabel
        %204 = OpCopyObject %4 %5
               OpBranch %200
        %200 = OpLabel
        %522 = OpPhi %4 %202 %201 %204 %203
               OpBranch %462
        %462 = OpLabel
               OpSelectionMerge %300 None
               OpBranchConditional %5 %303 %301
        %301 = OpLabel
        %302 = OpLoad %4 %102
               OpBranch %300
        %303 = OpLabel
        %304 = OpCopyObject %4 %5
               OpBranch %300
        %300 = OpLabel
        %466 = OpPhi %4 %302 %301 %304 %303
               OpBranch %470
        %470 = OpLabel
        %534 = OpSelect %4 %5 %522 %466
               OpReturn
               OpFunctionEnd
)";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest,
     LoadFromBufferBlockDecoratedStruct) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpMemberDecorate %11 0 Offset 0
               OpDecorate %11 BufferBlock
               OpDecorate %13 DescriptorSet 0
               OpDecorate %13 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
         %11 = OpTypeStruct %10
         %12 = OpTypePointer Uniform %11
         %13 = OpVariable %12 Uniform
         %21 = OpUndef %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
         %20 = OpLoad %11 %13
               OpBranch %9
          %9 = OpLabel
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

  auto transformation = TransformationFlattenConditionalBranch(
      5, true, 0, 0, 0,
      {MakeSideEffectWrapperInfo(MakeInstructionDescriptor(20, SpvOpLoad, 0),
                                 100, 101, 102, 103, 104, 21)});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
}

TEST(TransformationFlattenConditionalBranchTest, InapplicableSampledImageLoad) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %12 %96
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpDecorate %12 BuiltIn FragCoord
               OpDecorate %91 DescriptorSet 0
               OpDecorate %91 Binding 0
               OpDecorate %96 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
         %10 = OpTypeVector %6 4
         %11 = OpTypePointer Input %10
         %12 = OpVariable %11 Input
         %21 = OpConstant %6 2
         %24 = OpTypeInt 32 1
         %33 = OpTypeBool
         %35 = OpConstantTrue %33
         %88 = OpTypeImage %6 2D 0 0 0 1 Unknown
         %89 = OpTypeSampledImage %88
         %90 = OpTypePointer UniformConstant %89
         %91 = OpVariable %90 UniformConstant
         %95 = OpTypePointer Output %10
         %96 = OpVariable %95 Output
        %200 = OpUndef %89
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpSelectionMerge %38 None
               OpBranchConditional %35 %32 %37
         %32 = OpLabel
         %40 = OpLoad %89 %91
               OpBranch %38
         %37 = OpLabel
               OpBranch %38
         %38 = OpLabel
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

  ASSERT_FALSE(TransformationFlattenConditionalBranch(
                   28, true, 0, 0, 0,
                   {MakeSideEffectWrapperInfo(
                       MakeInstructionDescriptor(40, SpvOpLoad, 0), 100, 101,
                       102, 103, 104, 200)})
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationFlattenConditionalBranchTest,
     InapplicablePhiToSelectVector) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
         %11 = OpTypeVector %10 3
         %12 = OpUndef %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %20
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %21 = OpPhi %11 %12 %8 %12 %9
               OpReturn
               OpFunctionEnd
  )";

  for (auto env :
       {SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1, SPV_ENV_UNIVERSAL_1_2,
        SPV_ENV_UNIVERSAL_1_3, SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1}) {
    const auto consumer = nullptr;
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));

    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);

    auto transformation =
        TransformationFlattenConditionalBranch(5, true, 0, 0, 0, {});
    ASSERT_FALSE(
        transformation.IsApplicable(context.get(), transformation_context));
  }
}

TEST(TransformationFlattenConditionalBranchTest,
     InapplicablePhiToSelectVector2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
         %30 = OpTypeVector %6 3
         %31 = OpTypeVector %6 2
          %7 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
         %11 = OpTypeVector %10 3
         %40 = OpTypeFloat 32
         %41 = OpTypeVector %40 4
         %12 = OpUndef %11
         %60 = OpUndef %41
	 %61 = OpConstantComposite %31 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %20
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %21 = OpPhi %11 %12 %8 %12 %9
         %22 = OpPhi %11 %12 %8 %12 %9
         %23 = OpPhi %41 %60 %8 %60 %9
         %24 = OpPhi %31 %61 %8 %61 %9
         %25 = OpPhi %41 %60 %8 %60 %9
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

  auto transformation =
      TransformationFlattenConditionalBranch(5, true, 101, 102, 103, {});

  // bvec4 is not present in the module.
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
}

TEST(TransformationFlattenConditionalBranchTest,
     InapplicablePhiToSelectMatrix) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %10 = OpTypeFloat 32
         %30 = OpTypeVector %10 3
         %11 = OpTypeMatrix %30 3
         %12 = OpUndef %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %20
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %21 = OpPhi %11 %12 %8 %12 %9
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

  auto transformation =
      TransformationFlattenConditionalBranch(5, true, 0, 0, 0, {});
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationFlattenConditionalBranchTest, ApplicablePhiToSelectVector) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
         %11 = OpTypeVector %10 3
         %12 = OpUndef %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %20
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %21 = OpPhi %11 %12 %8 %12 %9
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  auto transformation =
      TransformationFlattenConditionalBranch(5, true, 0, 0, 0, {});
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
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
         %11 = OpTypeVector %10 3
         %12 = OpUndef %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %21 = OpSelect %11 %7 %12 %12
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest, ApplicablePhiToSelectVector2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
         %30 = OpTypeVector %6 3
         %31 = OpTypeVector %6 2
         %32 = OpTypeVector %6 4
          %7 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
         %11 = OpTypeVector %10 3
         %40 = OpTypeFloat 32
         %41 = OpTypeVector %40 4
         %12 = OpUndef %11
         %60 = OpUndef %41
	 %61 = OpConstantComposite %31 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %20
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %21 = OpPhi %11 %12 %8 %12 %9
         %22 = OpPhi %11 %12 %8 %12 %9
         %23 = OpPhi %41 %60 %8 %60 %9
         %24 = OpPhi %31 %61 %8 %61 %9
         %25 = OpPhi %41 %60 %8 %60 %9
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

  // No id for the 2D vector case is provided.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(5, true, 0, 102, 103, {})
                   .IsApplicable(context.get(), transformation_context));

  // No id for the 3D vector case is provided.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(5, true, 101, 0, 103, {})
                   .IsApplicable(context.get(), transformation_context));

  // No id for the 4D vector case is provided.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(5, true, 101, 102, 0, {})
                   .IsApplicable(context.get(), transformation_context));

  // %10 is not fresh
  ASSERT_FALSE(TransformationFlattenConditionalBranch(5, true, 10, 102, 103, {})
                   .IsApplicable(context.get(), transformation_context));

  // %10 is not fresh
  ASSERT_FALSE(TransformationFlattenConditionalBranch(5, true, 101, 10, 103, {})
                   .IsApplicable(context.get(), transformation_context));

  // %10 is not fresh
  ASSERT_FALSE(TransformationFlattenConditionalBranch(5, true, 101, 102, 10, {})
                   .IsApplicable(context.get(), transformation_context));

  // Duplicate "fresh" ids used for boolean vector constructors
  ASSERT_FALSE(
      TransformationFlattenConditionalBranch(5, true, 101, 102, 102, {})
          .IsApplicable(context.get(), transformation_context));

  auto transformation =
      TransformationFlattenConditionalBranch(5, true, 101, 102, 103, {});

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
          %6 = OpTypeBool
         %30 = OpTypeVector %6 3
         %31 = OpTypeVector %6 2
         %32 = OpTypeVector %6 4
          %7 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
         %11 = OpTypeVector %10 3
         %40 = OpTypeFloat 32
         %41 = OpTypeVector %40 4
         %12 = OpUndef %11
         %60 = OpUndef %41
	 %61 = OpConstantComposite %31 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %103 = OpCompositeConstruct %32 %7 %7 %7 %7
        %102 = OpCompositeConstruct %30 %7 %7 %7
        %101 = OpCompositeConstruct %31 %7 %7
         %21 = OpSelect %11 %102 %12 %12
         %22 = OpSelect %11 %102 %12 %12
         %23 = OpSelect %41 %103 %60 %60
         %24 = OpSelect %31 %101 %61 %61
         %25 = OpSelect %41 %103 %60 %60
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest, ApplicablePhiToSelectVector3) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
         %30 = OpTypeVector %6 3
         %31 = OpTypeVector %6 2
         %32 = OpTypeVector %6 4
          %7 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
         %11 = OpTypeVector %10 3
         %40 = OpTypeFloat 32
         %41 = OpTypeVector %40 4
         %12 = OpUndef %11
         %60 = OpUndef %41
	 %61 = OpConstantComposite %31 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %20
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %21 = OpPhi %11 %12 %8 %12 %9
         %22 = OpPhi %11 %12 %8 %12 %9
         %23 = OpPhi %41 %60 %8 %60 %9
         %24 = OpPhi %31 %61 %8 %61 %9
         %25 = OpPhi %41 %60 %8 %60 %9
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  auto transformation =
      TransformationFlattenConditionalBranch(5, true, 101, 0, 103, {});

  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Check that the in operands of any OpSelect instructions all have the
  // appropriate operand type.
  context->module()->ForEachInst([](opt::Instruction* inst) {
    if (inst->opcode() == SpvOpSelect) {
      ASSERT_EQ(SPV_OPERAND_TYPE_ID, inst->GetInOperand(0).type);
      ASSERT_EQ(SPV_OPERAND_TYPE_ID, inst->GetInOperand(1).type);
      ASSERT_EQ(SPV_OPERAND_TYPE_ID, inst->GetInOperand(2).type);
    }
  });

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
         %30 = OpTypeVector %6 3
         %31 = OpTypeVector %6 2
         %32 = OpTypeVector %6 4
          %7 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
         %11 = OpTypeVector %10 3
         %40 = OpTypeFloat 32
         %41 = OpTypeVector %40 4
         %12 = OpUndef %11
         %60 = OpUndef %41
	 %61 = OpConstantComposite %31 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %103 = OpCompositeConstruct %32 %7 %7 %7 %7
        %101 = OpCompositeConstruct %31 %7 %7
         %21 = OpSelect %11 %7 %12 %12
         %22 = OpSelect %11 %7 %12 %12
         %23 = OpSelect %41 %103 %60 %60
         %24 = OpSelect %31 %101 %61 %61
         %25 = OpSelect %41 %103 %60 %60
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest, ApplicablePhiToSelectMatrix) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %10 = OpTypeFloat 32
         %30 = OpTypeVector %10 3
         %11 = OpTypeMatrix %30 3
         %12 = OpUndef %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %20
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %21 = OpPhi %11 %12 %8 %12 %9
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  auto transformation =
      TransformationFlattenConditionalBranch(5, true, 0, 0, 0, {});
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
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %10 = OpTypeFloat 32
         %30 = OpTypeVector %10 3
         %11 = OpTypeMatrix %30 3
         %12 = OpUndef %11
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %21 = OpSelect %11 %7 %12 %12
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest,
     InapplicableConditionIsIrrelevant) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
         %10 = OpTypeInt 32 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
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

  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(7);

  // Inapplicable because the branch condition, %7, is irrelevant.
  ASSERT_FALSE(TransformationFlattenConditionalBranch(5, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationFlattenConditionalBranchTest,
     OpPhiWhenTrueBranchIsConvergenceBlock) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %7 %9 %8
          %8 = OpLabel
         %10 = OpCopyObject %6 %7
               OpBranch %9
          %9 = OpLabel
         %11 = OpPhi %6 %10 %8 %7 %5
         %12 = OpPhi %6 %7 %5 %10 %8
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

  TransformationFlattenConditionalBranch transformation(5, true, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %10 = OpCopyObject %6 %7
               OpBranch %9
          %9 = OpLabel
         %11 = OpSelect %6 %7 %7 %10
         %12 = OpSelect %6 %7 %7 %10
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, expected, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest,
     OpPhiWhenFalseBranchIsConvergenceBlock) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
         %10 = OpCopyObject %6 %7
               OpBranch %9
          %9 = OpLabel
         %11 = OpPhi %6 %10 %8 %7 %5
         %12 = OpPhi %6 %7 %5 %10 %8
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

  TransformationFlattenConditionalBranch transformation(5, true, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantTrue %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %10 = OpCopyObject %6 %7
               OpBranch %9
          %9 = OpLabel
         %11 = OpSelect %6 %7 %10 %7
         %12 = OpSelect %6 %7 %10 %7
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, expected, context.get()));
}

TEST(TransformationFlattenConditionalBranchTest, ContainsDeadBlocksTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %7 %8 %9
          %8 = OpLabel
         %10 = OpCopyObject %6 %7
               OpBranch %9
          %9 = OpLabel
         %11 = OpPhi %6 %10 %8 %7 %5
         %12 = OpPhi %6 %7 %5 %10 %8
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

  TransformationFlattenConditionalBranch transformation(5, true, 0, 0, 0, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  transformation_context.GetFactManager()->AddFactBlockIsDead(8);

  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationFlattenConditionalBranchTest, ContainsContinueBlockTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %4 = OpFunction %2 None %3
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpLoopMerge %15 %14 None
               OpBranchConditional %7 %5 %15
          %5 = OpLabel
               OpSelectionMerge %11 None
               OpBranchConditional %7 %9 %10
          %9 = OpLabel
               OpBranch %11
         %10 = OpLabel
               OpBranch %14
         %11 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpBranch %13
         %15 = OpLabel
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

  ASSERT_FALSE(TransformationFlattenConditionalBranch(5, true, 0, 0, 0, {})
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationFlattenConditionalBranchTest, ContainsSynonymCreation) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpConstantFalse %6
          %8 = OpTypeInt 32 0
          %9 = OpTypePointer Function %8
         %10 = OpConstant %8 42
         %80 = OpConstant %8 0
          %4 = OpFunction %2 None %3
         %11 = OpLabel
         %20 = OpVariable %9 Function
               OpBranch %12
         %12 = OpLabel
               OpSelectionMerge %31 None
               OpBranchConditional %7 %30 %31
         %30 = OpLabel
               OpStore %20 %10
         %21 = OpLoad %8 %20
               OpBranch %31
         %31 = OpLabel
               OpBranch %14
         %14 = OpLabel
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

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(10, {}), MakeDataDescriptor(21, {}));
  ASSERT_FALSE(TransformationFlattenConditionalBranch(
                   12, true, 0, 0, 0,
                   {MakeSideEffectWrapperInfo(
                        MakeInstructionDescriptor(30, SpvOpStore, 0), 100, 101),
                    MakeSideEffectWrapperInfo(
                        MakeInstructionDescriptor(21, SpvOpLoad, 0), 102, 103,
                        104, 105, 106, 80)})
                   .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
