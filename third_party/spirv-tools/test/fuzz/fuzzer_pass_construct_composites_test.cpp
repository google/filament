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

#include "source/fuzz/fuzzer_pass_construct_composites.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/pseudo_random_generator.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(FuzzerPassConstructCompositesTest, IsomorphicStructs) {
  // This test declares various isomorphic structs, and a struct that is made up
  // of these isomorphic structs.  The pass to construct composites is then
  // applied several times to check that no issues arise related to using a
  // value of one struct type when a value of an isomorphic struct type is
  // required.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpConstant %6 0
          %8 = OpTypeStruct %6 %6 %6
          %9 = OpTypeStruct %6 %6 %6
         %10 = OpTypeStruct %6 %6 %6
         %11 = OpTypeStruct %6 %6 %6
         %12 = OpTypeStruct %6 %6 %6
         %13 = OpTypeStruct %8 %9 %10 %11 %12
         %14 = OpConstantComposite %8 %7 %7 %7
         %15 = OpConstantComposite %9 %7 %7 %7
         %16 = OpConstantComposite %10 %7 %7 %7
         %17 = OpConstantComposite %11 %7 %7 %7
         %18 = OpConstantComposite %12 %7 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);

  for (uint32_t i = 0; i < 10; i++) {
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);
    protobufs::TransformationSequence transformation_sequence;

    FuzzerPassConstructComposites fuzzer_pass(
        context.get(), &transformation_context, &fuzzer_context,
        &transformation_sequence);

    fuzzer_pass.Apply();

    // We just check that the result is valid.
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
}

TEST(FuzzerPassConstructCompositesTest, IsomorphicArrays) {
  // This test declares various isomorphic arrays, and a struct that is made up
  // of these isomorphic arrays.  The pass to construct composites is then
  // applied several times to check that no issues arise related to using a
  // value of one array type when a value of an isomorphic array type is
  // required.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
         %50 = OpTypeInt 32 0
         %51 = OpConstant %50 3
          %7 = OpConstant %6 0
          %8 = OpTypeArray %6 %51
          %9 = OpTypeArray %6 %51
         %10 = OpTypeArray %6 %51
         %11 = OpTypeArray %6 %51
         %12 = OpTypeArray %6 %51
         %13 = OpTypeStruct %8 %9 %10 %11 %12
         %14 = OpConstantComposite %8 %7 %7 %7
         %15 = OpConstantComposite %9 %7 %7 %7
         %16 = OpConstantComposite %10 %7 %7 %7
         %17 = OpConstantComposite %11 %7 %7 %7
         %18 = OpConstantComposite %12 %7 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpNop
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);

  for (uint32_t i = 0; i < 10; i++) {
    const auto context =
        BuildModule(env, consumer, shader, kFuzzAssembleOption);
    spvtools::ValidatorOptions validator_options;
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    TransformationContext transformation_context(
        MakeUnique<FactManager>(context.get()), validator_options);
    protobufs::TransformationSequence transformation_sequence;

    FuzzerPassConstructComposites fuzzer_pass(
        context.get(), &transformation_context, &fuzzer_context,
        &transformation_sequence);

    fuzzer_pass.Apply();

    // We just check that the result is valid.
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
