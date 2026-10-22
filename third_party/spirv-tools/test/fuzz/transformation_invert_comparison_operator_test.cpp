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

#include "source/fuzz/transformation_invert_comparison_operator.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationInvertComparisonOperatorTest, BasicTest) {
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
          %7 = OpTypeInt 32 0
          %8 = OpTypeBool
          %9 = OpConstant %6 3
         %10 = OpConstant %6 4
         %11 = OpConstant %7 3
         %12 = OpConstant %7 4
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %13 = OpSLessThan %8 %9 %10
         %14 = OpSLessThanEqual %8 %9 %10
         %15 = OpSGreaterThan %8 %9 %10
         %16 = OpSGreaterThanEqual %8 %9 %10
         %17 = OpULessThan %8 %11 %12
         %18 = OpULessThanEqual %8 %11 %12
         %19 = OpUGreaterThan %8 %11 %12
         %20 = OpUGreaterThanEqual %8 %11 %12
         %21 = OpIEqual %8 %9 %10
         %22 = OpINotEqual %8 %9 %10
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
  // Operator id is not valid.
  ASSERT_FALSE(TransformationInvertComparisonOperator(23, 23).IsApplicable(
      context.get(), transformation_context));

  // Operator instruction is not supported.
  ASSERT_FALSE(TransformationInvertComparisonOperator(5, 23).IsApplicable(
      context.get(), transformation_context));

  // Fresh id is not fresh.
  ASSERT_FALSE(TransformationInvertComparisonOperator(13, 22).IsApplicable(
      context.get(), transformation_context));

  for (uint32_t fresh_id = 23, operator_id = 13; operator_id <= 22;
       ++fresh_id, ++operator_id) {
    TransformationInvertComparisonOperator transformation(operator_id,
                                                          fresh_id);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }

  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpTypeBool
          %9 = OpConstant %6 3
         %10 = OpConstant %6 4
         %11 = OpConstant %7 3
         %12 = OpConstant %7 4
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %23 = OpSGreaterThanEqual %8 %9 %10
         %13 = OpLogicalNot %8 %23
         %24 = OpSGreaterThan %8 %9 %10
         %14 = OpLogicalNot %8 %24
         %25 = OpSLessThanEqual %8 %9 %10
         %15 = OpLogicalNot %8 %25
         %26 = OpSLessThan %8 %9 %10
         %16 = OpLogicalNot %8 %26
         %27 = OpUGreaterThanEqual %8 %11 %12
         %17 = OpLogicalNot %8 %27
         %28 = OpUGreaterThan %8 %11 %12
         %18 = OpLogicalNot %8 %28
         %29 = OpULessThanEqual %8 %11 %12
         %19 = OpLogicalNot %8 %29
         %30 = OpULessThan %8 %11 %12
         %20 = OpLogicalNot %8 %30
         %31 = OpINotEqual %8 %9 %10
         %21 = OpLogicalNot %8 %31
         %32 = OpIEqual %8 %9 %10
         %22 = OpLogicalNot %8 %32
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, expected, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools