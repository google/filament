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

#include "source/fuzz/transformation_wrap_early_terminator_in_function.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationWrapEarlyTerminatorInFunctionTest, IsApplicable) {
  std::string shader = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_terminate_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 0
         %90 = OpTypeBool
         %91 = OpConstantFalse %90

         %20 = OpTypeFunction %2 %6
         %21 = OpTypeFunction %6

          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %11 None
               OpSwitch %7 %11 0 %8 1 %9 2 %10
          %8 = OpLabel
               OpKill
          %9 = OpLabel
               OpUnreachable
         %10 = OpLabel
               OpTerminateInvocation
         %11 = OpLabel
               OpReturn
               OpFunctionEnd

         %30 = OpFunction %2 None %3
         %31 = OpLabel
               OpKill
               OpFunctionEnd

         %50 = OpFunction %2 None %3
         %51 = OpLabel
               OpTerminateInvocation
               OpFunctionEnd

         %60 = OpFunction %6 None %21
         %61 = OpLabel
               OpBranch %62
         %62 = OpLabel
               OpKill
               OpFunctionEnd

         %70 = OpFunction %6 None %21
         %71 = OpLabel
               OpUnreachable
               OpFunctionEnd

         %80 = OpFunction %2 None %20
         %81 = OpFunctionParameter %6
         %82 = OpLabel
               OpTerminateInvocation
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

  // Bad: id is not fresh
  ASSERT_FALSE(TransformationWrapEarlyTerminatorInFunction(
                   61, MakeInstructionDescriptor(8, spv::Op::OpKill, 0), 0)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: early terminator instruction descriptor does not exist
  ASSERT_FALSE(TransformationWrapEarlyTerminatorInFunction(
                   100, MakeInstructionDescriptor(82, spv::Op::OpKill, 0), 0)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: early terminator instruction does not identify an early terminator
  ASSERT_FALSE(
      TransformationWrapEarlyTerminatorInFunction(
          100, MakeInstructionDescriptor(5, spv::Op::OpSelectionMerge, 0), 0)
          .IsApplicable(context.get(), transformation_context));

  // Bad: no wrapper function is available
  ASSERT_FALSE(
      TransformationWrapEarlyTerminatorInFunction(
          100, MakeInstructionDescriptor(9, spv::Op::OpUnreachable, 0), 0)
          .IsApplicable(context.get(), transformation_context));

  // Bad: returned value does not exist
  ASSERT_FALSE(TransformationWrapEarlyTerminatorInFunction(
                   100, MakeInstructionDescriptor(62, spv::Op::OpKill, 0), 1000)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: returned value does not have a type
  ASSERT_FALSE(TransformationWrapEarlyTerminatorInFunction(
                   100, MakeInstructionDescriptor(62, spv::Op::OpKill, 0), 61)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: returned value type does not match
  ASSERT_FALSE(TransformationWrapEarlyTerminatorInFunction(
                   100, MakeInstructionDescriptor(62, spv::Op::OpKill, 0), 91)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: returned value is not available
  ASSERT_FALSE(TransformationWrapEarlyTerminatorInFunction(
                   100, MakeInstructionDescriptor(62, spv::Op::OpKill, 0), 81)
                   .IsApplicable(context.get(), transformation_context));

  // Bad: the OpKill being targeted is in the only available wrapper; we cannot
  // have the wrapper call itself.
  ASSERT_FALSE(TransformationWrapEarlyTerminatorInFunction(
                   100, MakeInstructionDescriptor(31, spv::Op::OpKill, 0), 0)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationWrapEarlyTerminatorInFunctionTest, Apply) {
  std::string shader = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_terminate_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 0

         %20 = OpTypeFunction %2 %6
         %21 = OpTypeFunction %6

          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %11 None
               OpSwitch %7 %11 0 %8 1 %9 2 %10
          %8 = OpLabel
               OpKill
          %9 = OpLabel
               OpUnreachable
         %10 = OpLabel
               OpTerminateInvocation
         %11 = OpLabel
               OpReturn
               OpFunctionEnd

         %30 = OpFunction %2 None %3
         %31 = OpLabel
               OpKill
               OpFunctionEnd

         %40 = OpFunction %2 None %3
         %41 = OpLabel
               OpUnreachable
               OpFunctionEnd

         %50 = OpFunction %2 None %3
         %51 = OpLabel
               OpTerminateInvocation
               OpFunctionEnd

         %60 = OpFunction %2 None %3
         %61 = OpLabel
               OpBranch %62
         %62 = OpLabel
               OpKill
               OpFunctionEnd

         %70 = OpFunction %6 None %21
         %71 = OpLabel
               OpUnreachable
               OpFunctionEnd

         %80 = OpFunction %2 None %20
         %81 = OpFunctionParameter %6
         %82 = OpLabel
               OpTerminateInvocation
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

  for (auto& transformation :
       {TransformationWrapEarlyTerminatorInFunction(
            100, MakeInstructionDescriptor(8, spv::Op::OpKill, 0), 0),
        TransformationWrapEarlyTerminatorInFunction(
            101, MakeInstructionDescriptor(9, spv::Op::OpUnreachable, 0), 0),
        TransformationWrapEarlyTerminatorInFunction(
            102,
            MakeInstructionDescriptor(10, spv::Op::OpTerminateInvocation, 0),
            0),
        TransformationWrapEarlyTerminatorInFunction(
            103, MakeInstructionDescriptor(62, spv::Op::OpKill, 0), 0),
        TransformationWrapEarlyTerminatorInFunction(
            104, MakeInstructionDescriptor(71, spv::Op::OpUnreachable, 0), 7),
        TransformationWrapEarlyTerminatorInFunction(
            105,
            MakeInstructionDescriptor(82, spv::Op::OpTerminateInvocation, 0),
            0)}) {
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_terminate_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 0

         %20 = OpTypeFunction %2 %6
         %21 = OpTypeFunction %6

          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %11 None
               OpSwitch %7 %11 0 %8 1 %9 2 %10
          %8 = OpLabel
        %100 = OpFunctionCall %2 %30
               OpReturn
          %9 = OpLabel
        %101 = OpFunctionCall %2 %40
               OpReturn
         %10 = OpLabel
        %102 = OpFunctionCall %2 %50
               OpReturn
         %11 = OpLabel
               OpReturn
               OpFunctionEnd

         %30 = OpFunction %2 None %3
         %31 = OpLabel
               OpKill
               OpFunctionEnd

         %40 = OpFunction %2 None %3
         %41 = OpLabel
               OpUnreachable
               OpFunctionEnd

         %50 = OpFunction %2 None %3
         %51 = OpLabel
               OpTerminateInvocation
               OpFunctionEnd

         %60 = OpFunction %2 None %3
         %61 = OpLabel
               OpBranch %62
         %62 = OpLabel
        %103 = OpFunctionCall %2 %30
               OpReturn
               OpFunctionEnd

         %70 = OpFunction %6 None %21
         %71 = OpLabel
        %104 = OpFunctionCall %2 %40
               OpReturnValue %7
               OpFunctionEnd

         %80 = OpFunction %2 None %20
         %81 = OpFunctionParameter %6
         %82 = OpLabel
        %105 = OpFunctionCall %2 %50
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
