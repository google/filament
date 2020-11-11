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

#include "source/fuzz/transformation_merge_function_returns.h"

#include "gtest/gtest.h"
#include "source/fuzz/counter_overflow_id_source.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

protobufs::ReturnMergingInfo MakeReturnMergingInfo(
    uint32_t merge_block_id, uint32_t is_returning_id,
    uint32_t maybe_return_val_id,
    const std::map<uint32_t, uint32_t>& opphi_to_suitable_id) {
  protobufs::ReturnMergingInfo result;
  result.set_merge_block_id(merge_block_id);
  result.set_is_returning_id(is_returning_id);
  result.set_maybe_return_val_id(maybe_return_val_id);
  *result.mutable_opphi_to_suitable_id() =
      fuzzerutil::MapToRepeatedUInt32Pair(opphi_to_suitable_id);
  return result;
}

TEST(TransformationMergeFunctionReturnsTest, SimpleInapplicable) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypeFunction %5
          %7 = OpTypeFloat 32
          %8 = OpTypeFunction %7
          %9 = OpTypeBool
         %10 = OpConstantTrue %9
         %11 = OpConstantFalse %9
         %12 = OpConstant %5 0
         %13 = OpConstant %5 1
          %2 = OpFunction %3 None %4
         %14 = OpLabel
         %15 = OpFunctionCall %3 %16
         %17 = OpFunctionCall %3 %18
         %19 = OpFunctionCall %3 %20
         %21 = OpFunctionCall %7 %22
               OpReturn
               OpFunctionEnd
         %16 = OpFunction %3 None %4
         %23 = OpLabel
               OpSelectionMerge %24 None
               OpBranchConditional %10 %25 %26
         %25 = OpLabel
               OpReturn
         %26 = OpLabel
               OpReturn
         %24 = OpLabel
               OpUnreachable
               OpFunctionEnd
         %18 = OpFunction %3 None %4
         %27 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpLoopMerge %29 %30 None
               OpBranch %31
         %31 = OpLabel
               OpBranchConditional %10 %32 %29
         %32 = OpLabel
               OpReturn
         %30 = OpLabel
               OpBranch %28
         %29 = OpLabel
               OpReturn
               OpFunctionEnd
         %20 = OpFunction %3 None %4
         %33 = OpLabel
               OpBranch %34
         %34 = OpLabel
               OpLoopMerge %35 %36 None
               OpBranch %37
         %37 = OpLabel
               OpBranchConditional %10 %38 %35
         %38 = OpLabel
               OpReturn
         %36 = OpLabel
               OpBranch %34
         %35 = OpLabel
         %39 = OpFunctionCall %3 %18
               OpReturn
               OpFunctionEnd
         %22 = OpFunction %7 None %8
         %40 = OpLabel
               OpBranch %51
         %51 = OpLabel
               OpLoopMerge %41 %53 None
               OpBranchConditional %10 %42 %41
         %42 = OpLabel
         %43 = OpConvertSToF %7 %12
               OpReturnValue %43
         %41 = OpLabel
         %44 = OpConvertSToF %7 %13
               OpReturnValue %44
         %53 = OpLabel
               OpBranch %51
               OpFunctionEnd
         %45 = OpFunction %5 None %6
         %46 = OpLabel
               OpBranch %52
         %52 = OpLabel
         %47 = OpConvertSToF %7 %13
               OpLoopMerge %48 %54 None
               OpBranchConditional %10 %49 %48
         %49 = OpLabel
               OpReturnValue %12
         %48 = OpLabel
         %50 = OpCopyObject %5 %12
               OpReturnValue %13
         %54 = OpLabel
               OpBranch %52
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

  // Function %1 does not exist.
  ASSERT_FALSE(TransformationMergeFunctionReturns(1, 100, 101, 0, 0, {{}})
                   .IsApplicable(context.get(), transformation_context));

  // The entry block (%22) of function %15 does not branch unconditionally to
  // the following block.
  ASSERT_FALSE(TransformationMergeFunctionReturns(16, 100, 101, 0, 0, {{}})
                   .IsApplicable(context.get(), transformation_context));

  // Block %28 is the merge block of a loop containing a return instruction, but
  // it contains an OpReturn instruction (so, it contains instructions that are
  // not OpLabel, OpPhi or OpBranch).
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          18, 100, 101, 0, 0, {{MakeReturnMergingInfo(29, 102, 0, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // Block %34 is the merge block of a loop containing a return instruction, but
  // it contains an OpFunctionCall instruction (so, it contains instructions
  // that are not OpLabel, OpPhi or OpBranch).
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          20, 100, 101, 0, 0, {{MakeReturnMergingInfo(35, 102, 0, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // Id %1000 cannot be found in the module and there is no id of the correct
  // type (float) available at the end of the entry block of function %21.
  ASSERT_FALSE(TransformationMergeFunctionReturns(22, 100, 101, 102, 1000, {{}})
                   .IsApplicable(context.get(), transformation_context));

  // Id %47 is of type float, while function %45 has return type int.
  ASSERT_FALSE(TransformationMergeFunctionReturns(45, 100, 101, 102, 47, {{}})
                   .IsApplicable(context.get(), transformation_context));

  // Id %50 is not available at the end of the entry block of function %45.
  ASSERT_FALSE(TransformationMergeFunctionReturns(45, 100, 101, 102, 50, {{}})
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationMergeFunctionReturnsTest, MissingBooleans) {
  {
    // OpConstantTrue is missing.
    std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
               OpName %3 "A("
               OpDecorate %3 RelaxedPrecision
               OpDecorate %4 RelaxedPrecision
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %7 = OpTypeInt 32 1
          %8 = OpTypeFunction %7
          %9 = OpTypeBool
         %10 = OpConstantFalse %9
         %11 = OpConstant %7 1
         %12 = OpConstant %7 2
          %2 = OpFunction %5 None %6
         %13 = OpLabel
          %4 = OpFunctionCall %7 %3
               OpReturn
               OpFunctionEnd
          %3 = OpFunction %7 None %8
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %10 %17 %16
         %17 = OpLabel
               OpReturnValue %11
         %16 = OpLabel
               OpReturnValue %12
               OpFunctionEnd
)";

    const auto env = SPV_ENV_UNIVERSAL_1_5;
    const auto consumer = nullptr;
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);

    ASSERT_FALSE(TransformationMergeFunctionReturns(3, 100, 101, 0, 0, {{}})
                     .IsApplicable(context.get(), transformation_context));
  }
  {
    // OpConstantFalse is missing.
    std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
               OpName %2 "main"
               OpName %3 "A("
               OpDecorate %3 RelaxedPrecision
               OpDecorate %4 RelaxedPrecision
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %7 = OpTypeInt 32 1
          %8 = OpTypeFunction %7
          %9 = OpTypeBool
         %10 = OpConstantTrue %9
         %11 = OpConstant %7 1
         %12 = OpConstant %7 2
          %2 = OpFunction %5 None %6
         %13 = OpLabel
          %4 = OpFunctionCall %7 %3
               OpReturn
               OpFunctionEnd
          %3 = OpFunction %7 None %8
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %10 %17 %16
         %17 = OpLabel
               OpReturnValue %11
         %16 = OpLabel
               OpReturnValue %12
               OpFunctionEnd
)";

    const auto env = SPV_ENV_UNIVERSAL_1_5;
    const auto consumer = nullptr;
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);

    ASSERT_FALSE(TransformationMergeFunctionReturns(3, 100, 101, 0, 0, {{}})
                     .IsApplicable(context.get(), transformation_context));
  }
}

TEST(TransformationMergeFunctionReturnsTest, InvalidIds) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypeFunction %5
         %42 = OpTypeFloat 32
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %9 = OpConstantFalse %7
         %10 = OpConstant %5 0
         %11 = OpConstant %5 1
          %2 = OpFunction %3 None %4
         %12 = OpLabel
         %13 = OpFunctionCall %5 %14
         %15 = OpFunctionCall %3 %16
               OpReturn
               OpFunctionEnd
         %17 = OpFunction %3 None %4
         %18 = OpLabel
               OpBranch %19
         %19 = OpLabel
               OpLoopMerge %20 %21 None
               OpBranch %22
         %22 = OpLabel
               OpBranchConditional %8 %23 %20
         %23 = OpLabel
               OpReturn
         %21 = OpLabel
               OpBranch %19
         %20 = OpLabel
               OpBranch %24
         %24 = OpLabel
               OpReturn
               OpFunctionEnd
         %14 = OpFunction %5 None %6
         %25 = OpLabel
               OpBranch %26
         %26 = OpLabel
               OpLoopMerge %27 %28 None
               OpBranch %29
         %29 = OpLabel
               OpBranchConditional %8 %30 %27
         %30 = OpLabel
               OpReturnValue %10
         %28 = OpLabel
               OpBranch %26
         %27 = OpLabel
               OpBranch %33
         %33 = OpLabel
               OpReturnValue %11
               OpFunctionEnd
         %16 = OpFunction %3 None %4
         %34 = OpLabel
               OpBranch %35
         %35 = OpLabel
               OpLoopMerge %36 %37 None
               OpBranch %38
         %38 = OpLabel
         %43 = OpConvertSToF %42 %10
               OpBranchConditional %8 %39 %36
         %39 = OpLabel
               OpReturn
         %37 = OpLabel
         %44 = OpConvertSToF %42 %10
               OpBranch %35
         %36 = OpLabel
         %31 = OpPhi %42 %43 %38
         %32 = OpPhi %5 %11 %38
               OpBranch %40
         %40 = OpLabel
         %41 = OpFunctionCall %3 %17
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

  // Fresh id %100 is used twice.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          17, 100, 100, 0, 0, {{MakeReturnMergingInfo(20, 101, 0, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // Fresh id %100 is used twice.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          17, 100, 101, 0, 0, {{MakeReturnMergingInfo(20, 100, 0, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // %0 cannot be a fresh id for the new merge block.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          17, 100, 0, 0, 0, {{MakeReturnMergingInfo(20, 101, 0, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // %0 cannot be a fresh id for the new header block.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          17, 0, 100, 0, 0, {{MakeReturnMergingInfo(20, 101, 0, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // %0 cannot be a fresh id for the new |is_returning| instruction in an
  // existing merge block.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          17, 100, 101, 0, 0, {{MakeReturnMergingInfo(20, 0, 0, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // %0 cannot be a fresh id for the new |return_val| instruction in the new
  // return block.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          14, 100, 101, 0, 10, {{MakeReturnMergingInfo(27, 102, 103, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // %0 cannot be a fresh id for the new |maybe_return_val| instruction in an
  // existing merge block, inside a non-void function.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          14, 100, 101, 102, 10, {{MakeReturnMergingInfo(27, 103, 0, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // Fresh id %102 is repeated.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          14, 100, 101, 102, 10, {{MakeReturnMergingInfo(27, 102, 104, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // Id %11 (type int) does not have the correct type (float) for OpPhi
  // instruction %31.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          16, 100, 101, 0, 0,
          {{MakeReturnMergingInfo(36, 103, 104, {{{31, 11}, {32, 11}}})}})
          .IsApplicable(context.get(), transformation_context));

  // Id %11 (type int) does not have the correct type (float) for OpPhi
  // instruction %31.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          16, 100, 101, 0, 0,
          {{MakeReturnMergingInfo(36, 102, 0, {{{31, 11}, {32, 11}}})}})
          .IsApplicable(context.get(), transformation_context));

  // Id %43 is not available at the end of the entry block.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          16, 100, 101, 0, 0,
          {{MakeReturnMergingInfo(36, 102, 0, {{{31, 44}, {32, 11}}})}})
          .IsApplicable(context.get(), transformation_context));

  // There is not a mapping for id %31 (float OpPhi instruction in a loop merge
  // block) and no suitable id is available at the end of the entry block.
  ASSERT_FALSE(TransformationMergeFunctionReturns(
                   16, 100, 101, 0, 0,
                   {{MakeReturnMergingInfo(36, 102, 0, {{{32, 11}}})}})
                   .IsApplicable(context.get(), transformation_context));

  // Id %1000 cannot be found in the module and no suitable id for OpPhi %31 is
  // available at the end of the entry block.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          16, 100, 101, 0, 0,
          {{MakeReturnMergingInfo(36, 102, 0, {{{31, 1000}, {32, 11}}})}})
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationMergeFunctionReturnsTest, Simple) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypeFunction %5
          %7 = OpTypeFloat 32
          %8 = OpTypeFunction %7 %7
          %9 = OpTypeFunction %7
         %10 = OpTypeBool
         %11 = OpConstantTrue %10
         %40 = OpConstantFalse %10
         %12 = OpConstant %5 1
          %2 = OpFunction %3 None %4
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
         %14 = OpFunction %3 None %4
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpSelectionMerge %17 None
               OpBranchConditional %11 %18 %17
         %18 = OpLabel
               OpReturn
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
         %19 = OpFunction %5 None %6
         %20 = OpLabel
               OpBranch %21
         %21 = OpLabel
               OpSelectionMerge %22 None
               OpBranchConditional %11 %23 %24
         %23 = OpLabel
               OpReturnValue %12
         %24 = OpLabel
         %25 = OpIAdd %5 %12 %12
               OpReturnValue %25
         %22 = OpLabel
               OpUnreachable
               OpFunctionEnd
         %26 = OpFunction %7 None %8
         %27 = OpFunctionParameter %7
         %28 = OpLabel
               OpBranch %29
         %29 = OpLabel
               OpSelectionMerge %30 None
               OpBranchConditional %11 %31 %30
         %31 = OpLabel
         %32 = OpFAdd %7 %27 %27
               OpReturnValue %32
         %30 = OpLabel
               OpReturnValue %27
               OpFunctionEnd
         %33 = OpFunction %7 None %9
         %34 = OpLabel
         %35 = OpConvertSToF %7 %12
               OpBranch %36
         %36 = OpLabel
               OpSelectionMerge %37 None
               OpBranchConditional %11 %38 %37
         %38 = OpLabel
         %39 = OpFAdd %7 %35 %35
               OpReturnValue %39
         %37 = OpLabel
               OpReturnValue %35
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

  // The 0s are allowed because the function's return type is void.
  auto transformation1 =
      TransformationMergeFunctionReturns(14, 100, 101, 0, 0, {{}});
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %12 is available at the end of the entry block of %19 (it is a global
  // variable).
  ASSERT_TRUE(TransformationMergeFunctionReturns(19, 110, 111, 112, 12, {{}})
                  .IsApplicable(context.get(), transformation_context));

  // %1000 cannot be found in the module, but there is a suitable id available
  // at the end of the entry block (%12).
  auto transformation2 =
      TransformationMergeFunctionReturns(19, 110, 111, 112, 1000, {{}});
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %27 is available at the end of the entry block of %26 (it is a function
  // parameter).
  ASSERT_TRUE(TransformationMergeFunctionReturns(26, 120, 121, 122, 27, {{}})
                  .IsApplicable(context.get(), transformation_context));

  // %1000 cannot be found in the module, but there is a suitable id available
  // at the end of the entry block (%27).
  auto transformation3 =
      TransformationMergeFunctionReturns(26, 120, 121, 122, 1000, {{}});
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // %35 is available at the end of the entry block of %33 (it is in the entry
  // block).
  ASSERT_TRUE(TransformationMergeFunctionReturns(26, 130, 131, 132, 27, {{}})
                  .IsApplicable(context.get(), transformation_context));

  // %1000 cannot be found in the module, but there is a suitable id available
  // at the end of the entry block (%35).
  auto transformation4 =
      TransformationMergeFunctionReturns(33, 130, 131, 132, 1000, {{}});
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
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypeFunction %5
          %7 = OpTypeFloat 32
          %8 = OpTypeFunction %7 %7
          %9 = OpTypeFunction %7
         %10 = OpTypeBool
         %11 = OpConstantTrue %10
         %40 = OpConstantFalse %10
         %12 = OpConstant %5 1
          %2 = OpFunction %3 None %4
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
         %14 = OpFunction %3 None %4
         %15 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %100 None
               OpBranchConditional %11 %16 %100
         %16 = OpLabel
               OpSelectionMerge %17 None
               OpBranchConditional %11 %18 %17
         %18 = OpLabel
               OpBranch %101
         %17 = OpLabel
               OpBranch %101
        %101 = OpLabel
               OpReturn
               OpFunctionEnd
         %19 = OpFunction %5 None %6
         %20 = OpLabel
               OpBranch %110
        %110 = OpLabel
               OpLoopMerge %111 %110 None
               OpBranchConditional %11 %21 %110
         %21 = OpLabel
               OpSelectionMerge %22 None
               OpBranchConditional %11 %23 %24
         %23 = OpLabel
               OpBranch %111
         %24 = OpLabel
         %25 = OpIAdd %5 %12 %12
               OpBranch %111
         %22 = OpLabel
               OpUnreachable
        %111 = OpLabel
        %112 = OpPhi %5 %12 %23 %25 %24
               OpReturnValue %112
               OpFunctionEnd
         %26 = OpFunction %7 None %8
         %27 = OpFunctionParameter %7
         %28 = OpLabel
               OpBranch %120
        %120 = OpLabel
               OpLoopMerge %121 %120 None
               OpBranchConditional %11 %29 %120
         %29 = OpLabel
               OpSelectionMerge %30 None
               OpBranchConditional %11 %31 %30
         %31 = OpLabel
         %32 = OpFAdd %7 %27 %27
               OpBranch %121
         %30 = OpLabel
               OpBranch %121
        %121 = OpLabel
        %122 = OpPhi %7 %27 %30 %32 %31
               OpReturnValue %122
               OpFunctionEnd
         %33 = OpFunction %7 None %9
         %34 = OpLabel
         %35 = OpConvertSToF %7 %12
               OpBranch %130
        %130 = OpLabel
               OpLoopMerge %131 %130 None
               OpBranchConditional %11 %36 %130
         %36 = OpLabel
               OpSelectionMerge %37 None
               OpBranchConditional %11 %38 %37
         %38 = OpLabel
         %39 = OpFAdd %7 %35 %35
               OpBranch %131
         %37 = OpLabel
               OpBranch %131
        %131 = OpLabel
        %132 = OpPhi %7 %35 %37 %39 %38
               OpReturnValue %132
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationMergeFunctionReturnsTest, NestedLoops) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypeFunction %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %9 = OpConstantFalse %7
         %10 = OpConstant %5 2
         %11 = OpConstant %5 1
         %12 = OpConstant %5 3
          %2 = OpFunction %3 None %4
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
         %14 = OpFunction %5 None %6
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpLoopMerge %17 %16 None
               OpBranchConditional %8 %18 %16
         %18 = OpLabel
               OpLoopMerge %19 %20 None
               OpBranchConditional %8 %19 %21
         %19 = OpLabel
               OpBranch %17
         %21 = OpLabel
               OpReturnValue %12
         %17 = OpLabel
               OpBranch %22
         %20 = OpLabel
               OpBranch %18
         %22 = OpLabel
               OpLoopMerge %23 %24 None
               OpBranch %25
         %25 = OpLabel
               OpBranchConditional %8 %26 %23
         %26 = OpLabel
               OpSelectionMerge %27 None
               OpBranchConditional %9 %28 %27
         %28 = OpLabel
               OpBranch %29
         %29 = OpLabel
               OpLoopMerge %30 %29 None
               OpBranchConditional %8 %30 %29
         %30 = OpLabel
               OpLoopMerge %31 %32 None
               OpBranch %33
         %33 = OpLabel
               OpBranchConditional %9 %34 %31
         %34 = OpLabel
               OpReturnValue %10
         %32 = OpLabel
               OpBranch %30
         %31 = OpLabel
         %35 = OpPhi %5 %11 %33
         %36 = OpPhi %5 %10 %33
               OpBranch %37
         %37 = OpLabel
               OpReturnValue %35
         %27 = OpLabel
               OpBranch %24
         %24 = OpLabel
               OpBranch %22
         %23 = OpLabel
               OpBranch %38
         %38 = OpLabel
               OpReturnValue %12
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

  auto transformation = TransformationMergeFunctionReturns(
      14, 100, 101, 102, 11,
      {{MakeReturnMergingInfo(19, 103, 104, {{}}),
        MakeReturnMergingInfo(17, 105, 106, {{}}),
        MakeReturnMergingInfo(31, 107, 108, {{{35, 10}, {36, 12}}}),
        MakeReturnMergingInfo(23, 109, 110, {})}});
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
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypeFunction %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %9 = OpConstantFalse %7
         %10 = OpConstant %5 2
         %11 = OpConstant %5 1
         %12 = OpConstant %5 3
          %2 = OpFunction %3 None %4
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
         %14 = OpFunction %5 None %6
         %15 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %100 None
               OpBranchConditional %8 %16 %100
         %16 = OpLabel
               OpLoopMerge %17 %16 None
               OpBranchConditional %8 %18 %16
         %18 = OpLabel
               OpLoopMerge %19 %20 None
               OpBranchConditional %8 %19 %21
         %19 = OpLabel
        %103 = OpPhi %7 %8 %21 %9 %18
        %104 = OpPhi %5 %12 %21 %11 %18
               OpBranch %17
         %21 = OpLabel
               OpBranch %19
         %17 = OpLabel
        %105 = OpPhi %7 %103 %19
        %106 = OpPhi %5 %104 %19
               OpBranchConditional %105 %101 %22
         %20 = OpLabel
               OpBranch %18
         %22 = OpLabel
               OpLoopMerge %23 %24 None
               OpBranch %25
         %25 = OpLabel
               OpBranchConditional %8 %26 %23
         %26 = OpLabel
               OpSelectionMerge %27 None
               OpBranchConditional %9 %28 %27
         %28 = OpLabel
               OpBranch %29
         %29 = OpLabel
               OpLoopMerge %30 %29 None
               OpBranchConditional %8 %30 %29
         %30 = OpLabel
               OpLoopMerge %31 %32 None
               OpBranch %33
         %33 = OpLabel
               OpBranchConditional %9 %34 %31
         %34 = OpLabel
               OpBranch %31
         %32 = OpLabel
               OpBranch %30
         %31 = OpLabel
        %107 = OpPhi %7 %8 %34 %9 %33
        %108 = OpPhi %5 %10 %34 %11 %33
         %35 = OpPhi %5 %11 %33 %10 %34
         %36 = OpPhi %5 %10 %33 %12 %34
               OpBranchConditional %107 %23 %37
         %37 = OpLabel
               OpBranch %23
         %27 = OpLabel
               OpBranch %24
         %24 = OpLabel
               OpBranch %22
         %23 = OpLabel
        %109 = OpPhi %7 %107 %31 %8 %37 %9 %25
        %110 = OpPhi %5 %108 %31 %35 %37 %11 %25
               OpBranchConditional %109 %101 %38
         %38 = OpLabel
               OpBranch %101
        %101 = OpLabel
        %102 = OpPhi %5 %106 %17 %110 %23 %12 %38
               OpReturnValue %102
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationMergeFunctionReturnsTest, OverflowIds) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypeFunction %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %9 = OpConstantFalse %7
         %10 = OpConstant %5 1
          %2 = OpFunction %3 None %4
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %5 None %6
         %13 = OpLabel
               OpBranch %14
         %14 = OpLabel
         %15 = OpIAdd %5 %10 %10
               OpLoopMerge %16 %17 None
               OpBranch %18
         %18 = OpLabel
               OpBranchConditional %8 %19 %16
         %19 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %9 %21 %20
         %21 = OpLabel
               OpReturnValue %10
         %20 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpBranchConditional %8 %14 %16
         %16 = OpLabel
         %22 = OpPhi %5 %15 %17 %10 %18
               OpBranch %23
         %23 = OpLabel
               OpReturnValue %22
               OpFunctionEnd
         %24 = OpFunction %3 None %4
         %25 = OpLabel
               OpBranch %26
         %26 = OpLabel
               OpLoopMerge %27 %28 None
               OpBranch %29
         %29 = OpLabel
               OpBranchConditional %8 %30 %27
         %30 = OpLabel
               OpSelectionMerge %31 None
               OpBranchConditional %9 %32 %31
         %32 = OpLabel
               OpReturn
         %31 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpBranch %26
         %27 = OpLabel
         %33 = OpPhi %5 %10 %29
               OpBranch %34
         %34 = OpLabel
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

  auto overflow_ids_unique_ptr = MakeUnique<CounterOverflowIdSource>(1000);
  auto overflow_ids_ptr = overflow_ids_unique_ptr.get();
  TransformationContext transformation_context_with_overflow_ids(
      MakeUnique<FactManager>(context.get()), validator_options,
      std::move(overflow_ids_unique_ptr));

  // No mapping from merge block %16 to fresh ids is given, so overflow ids are
  // needed.
  auto transformation1 =
      TransformationMergeFunctionReturns(12, 100, 101, 102, 10, {{}});

#ifndef NDEBUG
  ASSERT_DEATH(
      transformation1.IsApplicable(context.get(), transformation_context),
      "Bad attempt to query whether overflow ids are available.");
#endif

  ASSERT_TRUE(transformation1.IsApplicable(
      context.get(), transformation_context_with_overflow_ids));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context_with_overflow_ids,
                        overflow_ids_ptr->GetIssuedOverflowIds());
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // No mapping from merge block %27 to fresh ids is given, so overflow ids are
  // needed.
  auto transformation2 =
      TransformationMergeFunctionReturns(24, 110, 111, 0, 0, {{}});

#ifndef NDEBUG
  ASSERT_DEATH(
      transformation2.IsApplicable(context.get(), transformation_context),
      "Bad attempt to query whether overflow ids are available.");
#endif

  ASSERT_TRUE(transformation2.IsApplicable(
      context.get(), transformation_context_with_overflow_ids));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context_with_overflow_ids, {1002});
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformations = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 1
          %6 = OpTypeFunction %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %9 = OpConstantFalse %7
         %10 = OpConstant %5 1
          %2 = OpFunction %3 None %4
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %5 None %6
         %13 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %100 None
               OpBranchConditional %8 %14 %100
         %14 = OpLabel
         %15 = OpIAdd %5 %10 %10
               OpLoopMerge %16 %17 None
               OpBranch %18
         %18 = OpLabel
               OpBranchConditional %8 %19 %16
         %19 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %9 %21 %20
         %21 = OpLabel
               OpBranch %16
         %20 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpBranchConditional %8 %14 %16
         %16 = OpLabel
       %1000 = OpPhi %7 %8 %21 %9 %17 %9 %18
       %1001 = OpPhi %5 %10 %21 %10 %17 %10 %18
         %22 = OpPhi %5 %15 %17 %10 %18 %10 %21
               OpBranchConditional %1000 %101 %23
         %23 = OpLabel
               OpBranch %101
        %101 = OpLabel
        %102 = OpPhi %5 %1001 %16 %22 %23
               OpReturnValue %102
               OpFunctionEnd
         %24 = OpFunction %3 None %4
         %25 = OpLabel
               OpBranch %110
        %110 = OpLabel
               OpLoopMerge %111 %110 None
               OpBranchConditional %8 %26 %110
         %26 = OpLabel
               OpLoopMerge %27 %28 None
               OpBranch %29
         %29 = OpLabel
               OpBranchConditional %8 %30 %27
         %30 = OpLabel
               OpSelectionMerge %31 None
               OpBranchConditional %9 %32 %31
         %32 = OpLabel
               OpBranch %27
         %31 = OpLabel
               OpBranch %28
         %28 = OpLabel
               OpBranch %26
         %27 = OpLabel
       %1002 = OpPhi %7 %8 %32 %9 %29
         %33 = OpPhi %5 %10 %29 %10 %32
               OpBranchConditional %1002 %111 %34
         %34 = OpLabel
               OpBranch %111
        %111 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformations, context.get()));
}

TEST(TransformationMergeFunctionReturnsTest, MissingIdsForOpPhi) {
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
          %7 = OpConstantFalse %5
          %8 = OpTypeInt 32 1
          %9 = OpTypeFunction %3 %8
         %10 = OpTypeFloat 32
          %2 = OpFunction %3 None %4
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %3 None %9
         %13 = OpFunctionParameter %8
         %14 = OpLabel
         %15 = OpConvertSToF %10 %13
               OpBranch %16
         %16 = OpLabel
               OpLoopMerge %17 %18 None
               OpBranch %19
         %19 = OpLabel
               OpBranchConditional %6 %20 %17
         %20 = OpLabel
               OpSelectionMerge %21 None
               OpBranchConditional %7 %22 %21
         %22 = OpLabel
               OpReturn
         %21 = OpLabel
               OpBranch %18
         %18 = OpLabel
               OpBranch %16
         %17 = OpLabel
         %23 = OpPhi %8 %13 %19
         %24 = OpPhi %10 %15 %19
         %25 = OpPhi %5 %6 %19
               OpBranch %26
         %26 = OpLabel
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

  // This test checks whether the transformation is able to find suitable ids
  // to use in existing OpPhi instructions if they are not provided in the
  // corresponding mapping.

  auto transformation = TransformationMergeFunctionReturns(
      12, 101, 102, 0, 0,
      {{MakeReturnMergingInfo(17, 103, 0, {{{25, 7}, {35, 8}}})}});
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
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpConstantFalse %5
          %8 = OpTypeInt 32 1
          %9 = OpTypeFunction %3 %8
         %10 = OpTypeFloat 32
          %2 = OpFunction %3 None %4
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %3 None %9
         %13 = OpFunctionParameter %8
         %14 = OpLabel
         %15 = OpConvertSToF %10 %13
               OpBranch %101
        %101 = OpLabel
               OpLoopMerge %102 %101 None
               OpBranchConditional %6 %16 %101
         %16 = OpLabel
               OpLoopMerge %17 %18 None
               OpBranch %19
         %19 = OpLabel
               OpBranchConditional %6 %20 %17
         %20 = OpLabel
               OpSelectionMerge %21 None
               OpBranchConditional %7 %22 %21
         %22 = OpLabel
               OpBranch %17
         %21 = OpLabel
               OpBranch %18
         %18 = OpLabel
               OpBranch %16
         %17 = OpLabel
        %103 = OpPhi %5 %6 %22 %7 %19
         %23 = OpPhi %8 %13 %19 %13 %22
         %24 = OpPhi %10 %15 %19 %15 %22
         %25 = OpPhi %5 %6 %19 %7 %22
               OpBranchConditional %103 %102 %26
         %26 = OpLabel
               OpBranch %102
        %102 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationMergeFunctionReturnsTest, RespectDominanceRules1) {
  // An id defined in a loop is used in the corresponding merge block. After the
  // transformation, the id will not dominate the merge block anymore. This is
  // only OK if the use is inside an OpPhi instruction. (Note that there is also
  // another condition for this transformation that forbids non-OpPhi
  // instructions in relevant merge blocks, but that case is also considered
  // here for completeness).

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
          %7 = OpConstantFalse %5
          %2 = OpFunction %3 None %4
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %7 %13 %14
         %14 = OpLabel
               OpReturn
         %13 = OpLabel
         %15 = OpCopyObject %5 %7
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %7 %9 %10
         %10 = OpLabel
         %16 = OpCopyObject %5 %15
               OpBranch %17
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
         %18 = OpFunction %3 None %4
         %19 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %21 %22 None
               OpBranch %23
         %23 = OpLabel
               OpSelectionMerge %24 None
               OpBranchConditional %7 %24 %25
         %25 = OpLabel
               OpReturn
         %24 = OpLabel
         %26 = OpCopyObject %5 %7
               OpBranch %22
         %22 = OpLabel
               OpBranchConditional %7 %20 %21
         %21 = OpLabel
         %27 = OpPhi %5 %26 %22
               OpBranch %28
         %28 = OpLabel
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

  // In function %2, the definition of id %15 will not dominate its use in
  // instruction %16 (inside merge block %10) after a new branch from return
  // block %14 is added.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          2, 100, 101, 0, 0, {{MakeReturnMergingInfo(10, 102, 103, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // In function %18, The definition of id %26 will still dominate its use in
  // instruction %27 (inside merge block %21), because %27 is an OpPhi
  // instruction.
  auto transformation = TransformationMergeFunctionReturns(
      18, 100, 101, 0, 0, {{MakeReturnMergingInfo(21, 102, 103, {{}})}});
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
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpConstantFalse %5
          %2 = OpFunction %3 None %4
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %7 %13 %14
         %14 = OpLabel
               OpReturn
         %13 = OpLabel
         %15 = OpCopyObject %5 %7
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %7 %9 %10
         %10 = OpLabel
         %16 = OpCopyObject %5 %15
               OpBranch %17
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
         %18 = OpFunction %3 None %4
         %19 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %100 None
               OpBranchConditional %6 %20 %100
         %20 = OpLabel
               OpLoopMerge %21 %22 None
               OpBranch %23
         %23 = OpLabel
               OpSelectionMerge %24 None
               OpBranchConditional %7 %24 %25
         %25 = OpLabel
               OpBranch %21
         %24 = OpLabel
         %26 = OpCopyObject %5 %7
               OpBranch %22
         %22 = OpLabel
               OpBranchConditional %7 %20 %21
         %21 = OpLabel
        %102 = OpPhi %5 %6 %25 %7 %22
         %27 = OpPhi %5 %26 %22 %6 %25
               OpBranchConditional %102 %101 %28
         %28 = OpLabel
               OpBranch %101
        %101 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationMergeFunctionReturnsTest, RespectDominanceRules2) {
  // An id defined in a loop is used after the corresponding merge block. After
  // the transformation, the id will not dominate its use anymore, regardless of
  // the kind of instruction in which it is used.

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
          %7 = OpConstantFalse %5
          %2 = OpFunction %3 None %4
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %7 %13 %14
         %14 = OpLabel
               OpReturn
         %13 = OpLabel
         %15 = OpCopyObject %5 %7
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %7 %9 %10
         %10 = OpLabel
               OpBranch %16
         %16 = OpLabel
         %17 = OpCopyObject %5 %15
               OpReturn
               OpFunctionEnd
         %18 = OpFunction %3 None %4
         %19 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %21 %22 None
               OpBranch %23
         %23 = OpLabel
               OpSelectionMerge %24 None
               OpBranchConditional %7 %24 %25
         %25 = OpLabel
               OpReturn
         %24 = OpLabel
         %26 = OpCopyObject %5 %7
               OpBranch %22
         %22 = OpLabel
               OpBranchConditional %7 %20 %21
         %21 = OpLabel
               OpBranch %27
         %27 = OpLabel
         %28 = OpPhi %5 %26 %21
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

  // In function %2, the definition of id %15 will not dominate its use in
  // instruction %17 (inside block %16) after a new branch from return
  // block %14 to merge block %10 is added.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          2, 100, 101, 0, 0, {{MakeReturnMergingInfo(10, 102, 103, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  // In function %18, the definition of id %26 will not dominate its use in
  // instruction %28 (inside block %27) after a new branch from return
  // block %25 to merge block %21 is added.
  ASSERT_FALSE(TransformationMergeFunctionReturns(
                   2, 100, 101, 0, 0,
                   {{MakeReturnMergingInfo(10, 102, 0, {{}}),
                     MakeReturnMergingInfo(21, 103, 0, {{}})}})
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationMergeFunctionReturnsTest, RespectDominanceRules3) {
  // An id defined in a loop is used inside the loop.
  // Changes to the predecessors of the merge block do not affect the validity
  // of the uses of such id.

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
          %7 = OpConstantFalse %5
          %2 = OpFunction %3 None %4
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %7 %13 %14
         %14 = OpLabel
               OpReturn
         %13 = OpLabel
         %15 = OpCopyObject %5 %7
               OpSelectionMerge %16 None
               OpBranchConditional %7 %16 %17
         %17 = OpLabel
         %18 = OpPhi %5 %15 %13
         %19 = OpCopyObject %5 %15
               OpBranch %16
         %16 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %7 %9 %10
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
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

  // In function %2, the definition of id %15 will still dominate its use in
  // instructions %18 and %19 after the transformation is applied, because the
  // fact that the id definition dominates the uses does not depend on it
  // dominating the merge block.
  auto transformation = TransformationMergeFunctionReturns(
      2, 100, 101, 0, 0, {{MakeReturnMergingInfo(10, 102, 103, {{}})}});
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
          %4 = OpTypeFunction %3
          %5 = OpTypeBool
          %6 = OpConstantTrue %5
          %7 = OpConstantFalse %5
          %2 = OpFunction %3 None %4
          %8 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %100 None
               OpBranchConditional %6 %9 %100
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %7 %13 %14
         %14 = OpLabel
               OpBranch %10
         %13 = OpLabel
         %15 = OpCopyObject %5 %7
               OpSelectionMerge %16 None
               OpBranchConditional %7 %16 %17
         %17 = OpLabel
         %18 = OpPhi %5 %15 %13
         %19 = OpCopyObject %5 %15
               OpBranch %16
         %16 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %7 %9 %10
         %10 = OpLabel
        %102 = OpPhi %5 %6 %14 %7 %11
               OpBranchConditional %102 %101 %20
         %20 = OpLabel
               OpBranch %101
        %101 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationMergeFunctionReturnsTest, RespectDominanceRules4) {
  // An id defined in a loop, which contain 2 return statements, is used after
  // the loop. We can only apply the transformation if the id dominates all of
  // the return blocks.

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
          %7 = OpConstantFalse %5
          %2 = OpFunction %3 None %4
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
         %13 = OpCopyObject %5 %7
               OpSelectionMerge %14 None
               OpBranchConditional %7 %14 %15
         %15 = OpLabel
               OpReturn
         %14 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %7 %16 %17
         %17 = OpLabel
               OpReturn
         %16 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %7 %9 %10
         %10 = OpLabel
               OpBranch %18
         %18 = OpLabel
         %19 = OpCopyObject %5 %13
               OpReturn
               OpFunctionEnd
         %20 = OpFunction %3 None %4
         %21 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpLoopMerge %23 %24 None
               OpBranch %25
         %25 = OpLabel
               OpSelectionMerge %26 None
               OpBranchConditional %7 %26 %27
         %27 = OpLabel
               OpReturn
         %26 = OpLabel
         %28 = OpCopyObject %5 %7
               OpSelectionMerge %29 None
               OpBranchConditional %7 %29 %30
         %30 = OpLabel
               OpReturn
         %29 = OpLabel
               OpBranch %24
         %24 = OpLabel
               OpBranchConditional %7 %22 %23
         %23 = OpLabel
               OpBranch %31
         %31 = OpLabel
         %32 = OpCopyObject %5 %28
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

  // In function %2, the definition of id %13 will still dominate its use in
  // instruction %19 after the transformation is applied, because %13 dominates
  // all of the return blocks.
  auto transformation = TransformationMergeFunctionReturns(
      2, 100, 101, 0, 0, {{MakeReturnMergingInfo(10, 102, 103, {{}})}});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // In function %20, the definition of id %28 will not dominate its use in
  // instruction %32 after the transformation is applied, because %28 dominates
  // only one of the return blocks.
  ASSERT_FALSE(
      TransformationMergeFunctionReturns(
          20, 100, 101, 0, 0, {{MakeReturnMergingInfo(23, 102, 103, {{}})}})
          .IsApplicable(context.get(), transformation_context));

  std::string after_transformation = R"(
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
          %7 = OpConstantFalse %5
          %2 = OpFunction %3 None %4
          %8 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %100 None
               OpBranchConditional %6 %9 %100
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
         %13 = OpCopyObject %5 %7
               OpSelectionMerge %14 None
               OpBranchConditional %7 %14 %15
         %15 = OpLabel
               OpBranch %10
         %14 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %7 %16 %17
         %17 = OpLabel
               OpBranch %10
         %16 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %7 %9 %10
         %10 = OpLabel
        %102 = OpPhi %5 %6 %15 %6 %17 %7 %11
               OpBranchConditional %102 %101 %18
         %18 = OpLabel
         %19 = OpCopyObject %5 %13
               OpBranch %101
        %101 = OpLabel
               OpReturn
               OpFunctionEnd
         %20 = OpFunction %3 None %4
         %21 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpLoopMerge %23 %24 None
               OpBranch %25
         %25 = OpLabel
               OpSelectionMerge %26 None
               OpBranchConditional %7 %26 %27
         %27 = OpLabel
               OpReturn
         %26 = OpLabel
         %28 = OpCopyObject %5 %7
               OpSelectionMerge %29 None
               OpBranchConditional %7 %29 %30
         %30 = OpLabel
               OpReturn
         %29 = OpLabel
               OpBranch %24
         %24 = OpLabel
               OpBranchConditional %7 %22 %23
         %23 = OpLabel
               OpBranch %31
         %31 = OpLabel
         %32 = OpCopyObject %5 %28
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationMergeFunctionReturnsTest, OpPhiAfterFirstBlock) {
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
          %7 = OpConstantFalse %5
          %2 = OpFunction %3 None %4
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %10 = OpPhi %5 %6 %8
               OpSelectionMerge %11 None
               OpBranchConditional %6 %12 %11
         %12 = OpLabel
               OpReturn
         %11 = OpLabel
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
      TransformationMergeFunctionReturns(2, 100, 101, 0, 0, {});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Ensure that all input operands of OpBranchConditional instructions have
  // the right operand type.
  context->module()->ForEachInst([](opt::Instruction* inst) {
    if (inst->opcode() == SpvOpBranchConditional) {
      ASSERT_EQ(inst->GetInOperand(0).type, SPV_OPERAND_TYPE_ID);
      ASSERT_EQ(inst->GetInOperand(1).type, SPV_OPERAND_TYPE_ID);
      ASSERT_EQ(inst->GetInOperand(2).type, SPV_OPERAND_TYPE_ID);
    }
  });

  std::string after_transformation = R"(
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
          %7 = OpConstantFalse %5
          %2 = OpFunction %3 None %4
          %8 = OpLabel
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %101 %100 None
               OpBranchConditional %6 %9 %100
          %9 = OpLabel
         %10 = OpPhi %5 %6 %100
               OpSelectionMerge %11 None
               OpBranchConditional %6 %12 %11
         %12 = OpLabel
               OpBranch %101
         %11 = OpLabel
               OpBranch %101
        %101 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools