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

#include "source/fuzz/transformation_add_type_int.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddTypeIntTest, IsApplicable) {
  std::string reference_shader = R"(
         OpCapability Shader
         OpCapability Int8
    %1 = OpExtInstImport "GLSL.std.450"
         OpMemoryModel Logical GLSL450
         OpEntryPoint Vertex %5 "main"

; Types
    %2 = OpTypeInt 8 1
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
  auto transformation = TransformationAddTypeInt(1, 32, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests missing Int16 capability.
  transformation = TransformationAddTypeInt(7, 16, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests missing Int64 capability.
  transformation = TransformationAddTypeInt(7, 64, false);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests existing signed 8-bit integer type.
  transformation = TransformationAddTypeInt(7, 8, true);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests adding unsigned 8-bit integer type.
  transformation = TransformationAddTypeInt(7, 8, false);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests adding unsigned 32-bit integer type.
  transformation = TransformationAddTypeInt(7, 32, false);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests adding signed 32-bit integer type.
  transformation = TransformationAddTypeInt(7, 32, true);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddTypeIntTest, Apply) {
  std::string reference_shader = R"(
         OpCapability Shader
         OpCapability Int8
         OpCapability Int16
         OpCapability Int64
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
  // Adds signed 8-bit integer type.
  auto transformation = TransformationAddTypeInt(6, 8, true);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  // Adds signed 16-bit integer type.
  transformation = TransformationAddTypeInt(7, 16, true);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  // Adds signed 32-bit integer type.
  transformation = TransformationAddTypeInt(8, 32, true);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  // Adds signed 64-bit integer type.
  transformation = TransformationAddTypeInt(9, 64, true);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  // Adds unsigned 8-bit integer type.
  transformation = TransformationAddTypeInt(10, 8, false);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  // Adds unsigned 16-bit integer type.
  transformation = TransformationAddTypeInt(11, 16, false);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  // Adds unsigned 32-bit integer type.
  transformation = TransformationAddTypeInt(12, 32, false);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  // Adds unsigned 64-bit integer type.
  transformation = TransformationAddTypeInt(13, 64, false);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
         OpCapability Shader
         OpCapability Int8
         OpCapability Int16
         OpCapability Int64
    %1 = OpExtInstImport "GLSL.std.450"
         OpMemoryModel Logical GLSL450
         OpEntryPoint Vertex %4 "main"

; Types
    %2 = OpTypeVoid
    %3 = OpTypeFunction %2
    %6 = OpTypeInt 8 1
    %7 = OpTypeInt 16 1
    %8 = OpTypeInt 32 1
    %9 = OpTypeInt 64 1
   %10 = OpTypeInt 8 0
   %11 = OpTypeInt 16 0
   %12 = OpTypeInt 32 0
   %13 = OpTypeInt 64 0

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
