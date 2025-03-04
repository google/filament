// Copyright (c) 2021 Mostafa Ashraf
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

#include "source/fuzz/transformation_swap_function_variables.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationSwapFunctionVariables, NotApplicable) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Function %8
         %10 = OpTypeVector %8 2
         %11 = OpTypePointer Function %10
         %12 = OpTypeVector %8 3
         %13 = OpTypeMatrix %12 3
         %14 = OpTypePointer Function %13
         %15 = OpTypeFunction %2 %7 %9 %11 %14 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %24 = OpVariable %7 Function
         %25 = OpVariable %9 Function
         %26 = OpVariable %11 Function
         %27 = OpVariable %14 Function
         %28 = OpVariable %7 Function
         %29 = OpVariable %7 Function
         %30 = OpVariable %7 Function
         %32 = OpVariable %9 Function
         %34 = OpVariable %11 Function
         %36 = OpVariable %14 Function
         %38 = OpVariable %7 Function
         %40 = OpVariable %7 Function
         %31 = OpLoad %6 %24
               OpStore %30 %31
         %33 = OpLoad %8 %25
               OpStore %32 %33
         %35 = OpLoad %10 %26
               OpStore %34 %35
         %37 = OpLoad %13 %27
               OpStore %36 %37
         %39 = OpLoad %6 %28
               OpStore %38 %39
         %41 = OpLoad %6 %29
               OpStore %40 %41
         %42 = OpFunctionCall %2 %22 %30 %32 %34 %36 %38 %40
               OpReturn
               OpFunctionEnd
         %22 = OpFunction %2 None %15
         %16 = OpFunctionParameter %7
         %17 = OpFunctionParameter %9
         %18 = OpFunctionParameter %11
         %19 = OpFunctionParameter %14
         %20 = OpFunctionParameter %7
         %21 = OpFunctionParameter %7
         %23 = OpLabel
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

#ifndef NDEBUG
  // Can't swap variable with itself.
  ASSERT_DEATH(TransformationSwapFunctionVariables(7, 7).IsApplicable(
                   context.get(), transformation_context),
               "Two results ids are equal");
#endif

  // Invalid because 200 is not the id of an instruction.
  ASSERT_FALSE(TransformationSwapFunctionVariables(1, 200).IsApplicable(
      context.get(), transformation_context));
  // Invalid because 5 is not the id of an instruction.
  ASSERT_FALSE(TransformationSwapFunctionVariables(5, 24).IsApplicable(
      context.get(), transformation_context));
  // Can't swap two instructions from two different blocks.
  ASSERT_FALSE(TransformationSwapFunctionVariables(16, 26).IsApplicable(
      context.get(), transformation_context));
}

TEST(TransformationSwapFunctionVariables, IsApplicable) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Function %8
         %10 = OpTypeVector %8 2
         %11 = OpTypePointer Function %10
         %12 = OpTypeVector %8 3
         %13 = OpTypeMatrix %12 3
         %14 = OpTypePointer Function %13
         %15 = OpTypeFunction %2 %7 %9 %11 %14 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %24 = OpVariable %7 Function
         %25 = OpVariable %9 Function
         %26 = OpVariable %11 Function
         %27 = OpVariable %14 Function
         %28 = OpVariable %7 Function
         %29 = OpVariable %7 Function
         %30 = OpVariable %7 Function
         %32 = OpVariable %9 Function
         %34 = OpVariable %11 Function
         %36 = OpVariable %14 Function
         %38 = OpVariable %7 Function
         %40 = OpVariable %7 Function
         %31 = OpLoad %6 %24
               OpStore %30 %31
         %33 = OpLoad %8 %25
               OpStore %32 %33
         %35 = OpLoad %10 %26
               OpStore %34 %35
         %37 = OpLoad %13 %27
               OpStore %36 %37
         %39 = OpLoad %6 %28
               OpStore %38 %39
         %41 = OpLoad %6 %29
               OpStore %40 %41
         %42 = OpFunctionCall %2 %22 %30 %32 %34 %36 %38 %40
               OpReturn
               OpFunctionEnd
         %22 = OpFunction %2 None %15
         %16 = OpFunctionParameter %7
         %17 = OpFunctionParameter %9
         %18 = OpFunctionParameter %11
         %19 = OpFunctionParameter %14
         %20 = OpFunctionParameter %7
         %21 = OpFunctionParameter %7
         %23 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  // Get Unique pointer of IRContext.
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);

  // Successful transformations
  {
    auto first_instruction = context->get_def_use_mgr()->GetDef(24);
    auto second_instruction = context->get_def_use_mgr()->GetDef(28);
    // Swap two OpVariable instructions in the same function.
    TransformationSwapFunctionVariables transformation(24, 28);

    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));

    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);

    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));

    ASSERT_EQ(first_instruction, context->get_def_use_mgr()->GetDef(24));
    ASSERT_EQ(second_instruction, context->get_def_use_mgr()->GetDef(28));
  }
  {
    auto first_instruction = context->get_def_use_mgr()->GetDef(38);
    auto second_instruction = context->get_def_use_mgr()->GetDef(40);
    // Swap two OpVariable instructions in the same function.
    TransformationSwapFunctionVariables transformation(38, 40);

    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));

    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);

    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));

    ASSERT_EQ(first_instruction, context->get_def_use_mgr()->GetDef(38));
    ASSERT_EQ(second_instruction, context->get_def_use_mgr()->GetDef(40));
  }
  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFloat 32
          %9 = OpTypePointer Function %8
         %10 = OpTypeVector %8 2
         %11 = OpTypePointer Function %10
         %12 = OpTypeVector %8 3
         %13 = OpTypeMatrix %12 3
         %14 = OpTypePointer Function %13
         %15 = OpTypeFunction %2 %7 %9 %11 %14 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %28 = OpVariable %7 Function
         %25 = OpVariable %9 Function
         %26 = OpVariable %11 Function
         %27 = OpVariable %14 Function
         %24 = OpVariable %7 Function
         %29 = OpVariable %7 Function
         %30 = OpVariable %7 Function
         %32 = OpVariable %9 Function
         %34 = OpVariable %11 Function
         %36 = OpVariable %14 Function
         %40 = OpVariable %7 Function
         %38 = OpVariable %7 Function
         %31 = OpLoad %6 %24
               OpStore %30 %31
         %33 = OpLoad %8 %25
               OpStore %32 %33
         %35 = OpLoad %10 %26
               OpStore %34 %35
         %37 = OpLoad %13 %27
               OpStore %36 %37
         %39 = OpLoad %6 %28
               OpStore %38 %39
         %41 = OpLoad %6 %29
               OpStore %40 %41
         %42 = OpFunctionCall %2 %22 %30 %32 %34 %36 %38 %40
               OpReturn
               OpFunctionEnd
         %22 = OpFunction %2 None %15
         %16 = OpFunctionParameter %7
         %17 = OpFunctionParameter %9
         %18 = OpFunctionParameter %11
         %19 = OpFunctionParameter %14
         %20 = OpFunctionParameter %7
         %21 = OpFunctionParameter %7
         %23 = OpLabel
               OpReturn
               OpFunctionEnd
)";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
