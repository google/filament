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

#include "source/fuzz/transformation_add_type_float.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddTypeFloatTest, IsApplicable) {
  std::string reference_shader = R"(
         OpCapability Shader
         OpCapability Float16
    %1 = OpExtInstImport "GLSL.std.450"
         OpMemoryModel Logical GLSL450
         OpEntryPoint Vertex %5 "main"

; Types
    %2 = OpTypeFloat 16
    %3 = OpTypeVoid
    %4 = OpTypeFunction %3

; main function
    %5 = OpFunction %3 None %4
    %6 = OpLabel
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
  // Tests non-fresh id.
  auto transformation = TransformationAddTypeFloat(1, 32);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests missing Float64 capability.
  transformation = TransformationAddTypeFloat(7, 64);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests existing 16-bit float type.
  transformation = TransformationAddTypeFloat(7, 16);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests adding 32-bit float type.
  transformation = TransformationAddTypeFloat(7, 32);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddTypeFloatTest, Apply) {
  std::string reference_shader = R"(
         OpCapability Shader
         OpCapability Float16
         OpCapability Float64
    %1 = OpExtInstImport "GLSL.std.450"
         OpMemoryModel Logical GLSL450
         OpEntryPoint Vertex %4 "main"

; Types
    %2 = OpTypeVoid
    %3 = OpTypeFunction %2

; main function
    %4 = OpFunction %2 None %3
    %5 = OpLabel
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
  // Adds 16-bit float type.
  auto transformation = TransformationAddTypeFloat(6, 16);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  // Adds 32-bit float type.
  transformation = TransformationAddTypeFloat(7, 32);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  // Adds 64-bit float type.
  transformation = TransformationAddTypeFloat(8, 64);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
         OpCapability Shader
         OpCapability Float16
         OpCapability Float64
    %1 = OpExtInstImport "GLSL.std.450"
         OpMemoryModel Logical GLSL450
         OpEntryPoint Vertex %4 "main"

; Types
    %2 = OpTypeVoid
    %3 = OpTypeFunction %2
    %6 = OpTypeFloat 16
    %7 = OpTypeFloat 32
    %8 = OpTypeFloat 64

; main function
    %4 = OpFunction %2 None %3
    %5 = OpLabel
         OpReturn
         OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
