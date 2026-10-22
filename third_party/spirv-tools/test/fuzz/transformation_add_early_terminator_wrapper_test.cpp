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

#include "source/fuzz/transformation_add_early_terminator_wrapper.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddEarlyTerminatorWrapperTest, NoVoidType) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability Linkage
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpSource ESSL 320
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  ASSERT_FALSE(
      TransformationAddEarlyTerminatorWrapper(100, 101, spv::Op::OpKill)
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddEarlyTerminatorWrapperTest, NoVoidFunctionType) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability Linkage
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpSource ESSL 320
          %2 = OpTypeVoid
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  ASSERT_FALSE(
      TransformationAddEarlyTerminatorWrapper(100, 101, spv::Op::OpKill)
          .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddEarlyTerminatorWrapperTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_terminate_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
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

  ASSERT_FALSE(TransformationAddEarlyTerminatorWrapper(2, 101, spv::Op::OpKill)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(TransformationAddEarlyTerminatorWrapper(100, 4, spv::Op::OpKill)
                   .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationAddEarlyTerminatorWrapper(100, 100, spv::Op::OpKill)
          .IsApplicable(context.get(), transformation_context));

#ifndef NDEBUG
  ASSERT_DEATH(
      TransformationAddEarlyTerminatorWrapper(100, 101, spv::Op::OpReturn)
          .IsApplicable(context.get(), transformation_context),
      "Invalid opcode.");
#endif

  auto transformation1 =
      TransformationAddEarlyTerminatorWrapper(100, 101, spv::Op::OpKill);
  auto transformation2 =
      TransformationAddEarlyTerminatorWrapper(102, 103, spv::Op::OpUnreachable);
  auto transformation3 = TransformationAddEarlyTerminatorWrapper(
      104, 105, spv::Op::OpTerminateInvocation);

  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);
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
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        %100 = OpFunction %2 None %3
        %101 = OpLabel
               OpKill
               OpFunctionEnd
        %102 = OpFunction %2 None %3
        %103 = OpLabel
               OpUnreachable
               OpFunctionEnd
        %104 = OpFunction %2 None %3
        %105 = OpLabel
               OpTerminateInvocation
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
