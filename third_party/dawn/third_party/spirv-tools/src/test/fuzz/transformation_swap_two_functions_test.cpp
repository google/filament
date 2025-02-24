// Copyright (c) 2021 Shiyu Liu
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

#include "source/fuzz/transformation_swap_two_functions.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationSwapTwoFunctionsTest, SimpleTest) {
  //   float multiplyBy2(in float value) {
  //     return value*2.0;
  //   }

  //   float multiplyBy4(in float value) {
  //     return multiplyBy2(value)*2.0;
  //   }

  //   float multiplyBy8(in float value) {
  //     return multiplyBy2(value)*multiplyBy4(value);
  //   }

  //   layout(location=0) in float value;
  //   void main() { //4
  //     multiplyBy2(3.7); //10
  //     multiplyBy4(3.9); //13
  //     multiplyBy8(5.0); //16
  //   }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %48
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "multiplyBy2(f1;"
               OpName %9 "value"
               OpName %13 "multiplyBy4(f1;"
               OpName %12 "value"
               OpName %16 "multiplyBy8(f1;"
               OpName %15 "value"
               OpName %23 "param"
               OpName %29 "param"
               OpName %32 "param"
               OpName %39 "param"
               OpName %42 "param"
               OpName %45 "param"
               OpName %48 "value"
               OpDecorate %48 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %6 %7
         %19 = OpConstant %6 2
         %38 = OpConstant %6 3.70000005
         %41 = OpConstant %6 3.9000001
         %44 = OpConstant %6 5
         %47 = OpTypePointer Input %6
         %48 = OpVariable %47 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %39 = OpVariable %7 Function
         %42 = OpVariable %7 Function
         %45 = OpVariable %7 Function
               OpStore %39 %38
         %40 = OpFunctionCall %6 %10 %39
               OpStore %42 %41
         %43 = OpFunctionCall %6 %13 %42
               OpStore %45 %44
         %46 = OpFunctionCall %6 %16 %45
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %18 = OpLoad %6 %9
         %20 = OpFMul %6 %18 %19
               OpReturnValue %20
               OpFunctionEnd
         %13 = OpFunction %6 None %8
         %12 = OpFunctionParameter %7
         %14 = OpLabel
         %23 = OpVariable %7 Function
         %24 = OpLoad %6 %12
               OpStore %23 %24
         %25 = OpFunctionCall %6 %10 %23
         %26 = OpFMul %6 %25 %19
               OpReturnValue %26
               OpFunctionEnd
         %16 = OpFunction %6 None %8
         %15 = OpFunctionParameter %7
         %17 = OpLabel
         %29 = OpVariable %7 Function
         %32 = OpVariable %7 Function
         %30 = OpLoad %6 %15
               OpStore %29 %30
         %31 = OpFunctionCall %6 %10 %29
         %33 = OpLoad %6 %15
               OpStore %32 %33
         %34 = OpFunctionCall %6 %13 %32
         %35 = OpFMul %6 %31 %34
               OpReturnValue %35
               OpFunctionEnd
  )";
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;

  // Check context validity.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

#ifndef NDEBUG
  // Function should not swap with itself.
  ASSERT_DEATH(TransformationSwapTwoFunctions(4, 4).IsApplicable(
                   context.get(), transformation_context),
               "The two function ids cannot be the same.");
#endif

  // Function with id 29 does not exist.
  ASSERT_FALSE(TransformationSwapTwoFunctions(10, 29).IsApplicable(
      context.get(), transformation_context));

  // Function with id 30 does not exist.
  ASSERT_FALSE(TransformationSwapTwoFunctions(30, 13).IsApplicable(
      context.get(), transformation_context));

  // Both functions with id 5 and 6 do not exist.
  ASSERT_FALSE(TransformationSwapTwoFunctions(5, 6).IsApplicable(
      context.get(), transformation_context));

  // Function with result_id 10 and 13 should swap successfully.
  auto swap_test5 = TransformationSwapTwoFunctions(10, 13);
  ASSERT_TRUE(swap_test5.IsApplicable(context.get(), transformation_context));

  // Get the definitions of functions 10 and 13, as recorded by the def-use
  // manager.
  auto def_use_manager = context->get_def_use_mgr();
  auto function_10_inst = def_use_manager->GetDef(10);
  auto function_13_inst = def_use_manager->GetDef(13);

  ApplyAndCheckFreshIds(swap_test5, context.get(), &transformation_context);

  // Check that def-use information for functions 10 and 13 has been preserved
  // by the transformation.
  ASSERT_EQ(function_10_inst, def_use_manager->GetDef(10));
  ASSERT_EQ(function_13_inst, def_use_manager->GetDef(13));

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %48
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "multiplyBy2(f1;"
               OpName %9 "value"
               OpName %13 "multiplyBy4(f1;"
               OpName %12 "value"
               OpName %16 "multiplyBy8(f1;"
               OpName %15 "value"
               OpName %23 "param"
               OpName %29 "param"
               OpName %32 "param"
               OpName %39 "param"
               OpName %42 "param"
               OpName %45 "param"
               OpName %48 "value"
               OpDecorate %48 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %6 %7
         %19 = OpConstant %6 2
         %38 = OpConstant %6 3.70000005
         %41 = OpConstant %6 3.9000001
         %44 = OpConstant %6 5
         %47 = OpTypePointer Input %6
         %48 = OpVariable %47 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %39 = OpVariable %7 Function
         %42 = OpVariable %7 Function
         %45 = OpVariable %7 Function
               OpStore %39 %38
         %40 = OpFunctionCall %6 %10 %39
               OpStore %42 %41
         %43 = OpFunctionCall %6 %13 %42
               OpStore %45 %44
         %46 = OpFunctionCall %6 %16 %45
               OpReturn
               OpFunctionEnd
         %13 = OpFunction %6 None %8
         %12 = OpFunctionParameter %7
         %14 = OpLabel
         %23 = OpVariable %7 Function
         %24 = OpLoad %6 %12
               OpStore %23 %24
         %25 = OpFunctionCall %6 %10 %23
         %26 = OpFMul %6 %25 %19
               OpReturnValue %26
               OpFunctionEnd
         %10 = OpFunction %6 None %8
         %9 = OpFunctionParameter %7
         %11 = OpLabel
         %18 = OpLoad %6 %9
         %20 = OpFMul %6 %18 %19
               OpReturnValue %20
               OpFunctionEnd
         %16 = OpFunction %6 None %8
         %15 = OpFunctionParameter %7
         %17 = OpLabel
         %29 = OpVariable %7 Function
         %32 = OpVariable %7 Function
         %30 = OpLoad %6 %15
               OpStore %29 %30
         %31 = OpFunctionCall %6 %10 %29
         %33 = OpLoad %6 %15
               OpStore %32 %33
         %34 = OpFunctionCall %6 %13 %32
         %35 = OpFMul %6 %31 %34
               OpReturnValue %35
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
