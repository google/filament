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

#include "source/fuzz/transformation_add_constant_scalar.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddConstantScalarTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %12 "y"
               OpName %16 "z"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %10 = OpTypeInt 32 0
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 2
         %14 = OpTypeFloat 32
         %15 = OpTypePointer Function %14
         %17 = OpConstant %14 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %16 = OpVariable %15 Function
               OpStore %8 %9
               OpStore %12 %13
               OpStore %16 %17
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  const float float_values[2] = {3.0, 30.0};
  uint32_t uint_for_float[2];
  memcpy(uint_for_float, float_values, sizeof(float_values));

  auto add_signed_int_1 = TransformationAddConstantScalar(100, 6, {1});
  auto add_signed_int_10 = TransformationAddConstantScalar(101, 6, {10});
  auto add_unsigned_int_2 = TransformationAddConstantScalar(102, 10, {2});
  auto add_unsigned_int_20 = TransformationAddConstantScalar(103, 10, {20});
  auto add_float_3 =
      TransformationAddConstantScalar(104, 14, {uint_for_float[0]});
  auto add_float_30 =
      TransformationAddConstantScalar(105, 14, {uint_for_float[1]});
  auto bad_add_float_30_id_already_used =
      TransformationAddConstantScalar(104, 14, {uint_for_float[1]});
  auto bad_id_already_used = TransformationAddConstantScalar(1, 6, {1});
  auto bad_no_data = TransformationAddConstantScalar(100, 6, {});
  auto bad_too_much_data = TransformationAddConstantScalar(100, 6, {1, 2});
  auto bad_type_id_does_not_exist =
      TransformationAddConstantScalar(108, 2020, {uint_for_float[0]});
  auto bad_type_id_is_not_a_type = TransformationAddConstantScalar(109, 9, {0});
  auto bad_type_id_is_void = TransformationAddConstantScalar(110, 2, {0});
  auto bad_type_id_is_pointer = TransformationAddConstantScalar(111, 11, {0});

  // Id is already in use.
  ASSERT_FALSE(bad_id_already_used.IsApplicable(context.get(), fact_manager));

  // At least one word of data must be provided.
  ASSERT_FALSE(bad_no_data.IsApplicable(context.get(), fact_manager));

  // Cannot give two data words for a 32-bit type.
  ASSERT_FALSE(bad_too_much_data.IsApplicable(context.get(), fact_manager));

  // Type id does not exist
  ASSERT_FALSE(
      bad_type_id_does_not_exist.IsApplicable(context.get(), fact_manager));

  // Type id is not a type
  ASSERT_FALSE(
      bad_type_id_is_not_a_type.IsApplicable(context.get(), fact_manager));

  // Type id is void
  ASSERT_FALSE(bad_type_id_is_void.IsApplicable(context.get(), fact_manager));

  // Type id is pointer
  ASSERT_FALSE(
      bad_type_id_is_pointer.IsApplicable(context.get(), fact_manager));

  ASSERT_TRUE(add_signed_int_1.IsApplicable(context.get(), fact_manager));
  add_signed_int_1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(add_signed_int_10.IsApplicable(context.get(), fact_manager));
  add_signed_int_10.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(add_unsigned_int_2.IsApplicable(context.get(), fact_manager));
  add_unsigned_int_2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(add_unsigned_int_20.IsApplicable(context.get(), fact_manager));
  add_unsigned_int_20.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(add_float_3.IsApplicable(context.get(), fact_manager));
  add_float_3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(add_float_30.IsApplicable(context.get(), fact_manager));
  add_float_30.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_FALSE(bad_add_float_30_id_already_used.IsApplicable(context.get(),
                                                             fact_manager));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %12 "y"
               OpName %16 "z"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %10 = OpTypeInt 32 0
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 2
         %14 = OpTypeFloat 32
         %15 = OpTypePointer Function %14
         %17 = OpConstant %14 3
        %100 = OpConstant %6 1
        %101 = OpConstant %6 10
        %102 = OpConstant %10 2
        %103 = OpConstant %10 20
        %104 = OpConstant %14 3
        %105 = OpConstant %14 30
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %16 = OpVariable %15 Function
               OpStore %8 %9
               OpStore %12 %13
               OpStore %16 %17
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
