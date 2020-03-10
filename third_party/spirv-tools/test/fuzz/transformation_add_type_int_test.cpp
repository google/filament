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
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddTypeIntTest, BasicTest) {
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
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // Not applicable because id 1 is already in use.
  ASSERT_FALSE(TransformationAddTypeInt(1, 32, false)
                   .IsApplicable(context.get(), fact_manager));

  auto add_type_signed_int_32 = TransformationAddTypeInt(100, 32, true);
  auto add_type_unsigned_int_32 = TransformationAddTypeInt(101, 32, false);
  auto add_type_signed_int_32_again = TransformationAddTypeInt(102, 32, true);
  auto add_type_unsigned_int_32_again =
      TransformationAddTypeInt(103, 32, false);

  ASSERT_TRUE(add_type_signed_int_32.IsApplicable(context.get(), fact_manager));
  add_type_signed_int_32.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(
      add_type_unsigned_int_32.IsApplicable(context.get(), fact_manager));
  add_type_unsigned_int_32.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Not applicable as we already have these types now.
  ASSERT_FALSE(
      add_type_signed_int_32_again.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      add_type_unsigned_int_32_again.IsApplicable(context.get(), fact_manager));

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
        %100 = OpTypeInt 32 1
        %101 = OpTypeInt 32 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
