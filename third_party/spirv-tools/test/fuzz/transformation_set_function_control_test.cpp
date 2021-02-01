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

#include "source/fuzz/transformation_set_function_control.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationSetFunctionControlTest, VariousScenarios) {
  // This is a simple transformation; this test captures the important things
  // to check for.

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %54
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %11 "foo(i1;i1;"
               OpName %9 "a"
               OpName %10 "b"
               OpName %13 "bar("
               OpName %17 "baz(i1;"
               OpName %16 "x"
               OpName %21 "boo(i1;i1;"
               OpName %19 "a"
               OpName %20 "b"
               OpName %29 "g"
               OpName %42 "param"
               OpName %44 "param"
               OpName %45 "param"
               OpName %48 "param"
               OpName %49 "param"
               OpName %54 "color"
               OpDecorate %54 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %6 %7 %7
         %15 = OpTypeFunction %6 %7
         %28 = OpTypePointer Private %6
         %29 = OpVariable %28 Private
         %30 = OpConstant %6 2
         %31 = OpConstant %6 5
         %51 = OpTypeFloat 32
         %52 = OpTypeVector %51 4
         %53 = OpTypePointer Output %52
         %54 = OpVariable %53 Output
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %42 = OpVariable %7 Function
         %44 = OpVariable %7 Function
         %45 = OpVariable %7 Function
         %48 = OpVariable %7 Function
         %49 = OpVariable %7 Function
         %41 = OpFunctionCall %2 %13
               OpStore %42 %30
         %43 = OpFunctionCall %6 %17 %42
               OpStore %44 %31
         %46 = OpLoad %6 %29
               OpStore %45 %46
         %47 = OpFunctionCall %6 %21 %44 %45
               OpStore %48 %43
               OpStore %49 %47
         %50 = OpFunctionCall %6 %11 %48 %49
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %6 Const %8
          %9 = OpFunctionParameter %7
         %10 = OpFunctionParameter %7
         %12 = OpLabel
         %23 = OpLoad %6 %9
         %24 = OpLoad %6 %10
         %25 = OpIAdd %6 %23 %24
               OpReturnValue %25
               OpFunctionEnd
         %13 = OpFunction %2 Inline %3
         %14 = OpLabel
               OpStore %29 %30
               OpReturn
               OpFunctionEnd
         %17 = OpFunction %6 Pure|DontInline %15
         %16 = OpFunctionParameter %7
         %18 = OpLabel
         %32 = OpLoad %6 %16
         %33 = OpIAdd %6 %31 %32
               OpReturnValue %33
               OpFunctionEnd
         %21 = OpFunction %6 DontInline %8
         %19 = OpFunctionParameter %7
         %20 = OpFunctionParameter %7
         %22 = OpLabel
         %36 = OpLoad %6 %19
         %37 = OpLoad %6 %20
         %38 = OpIMul %6 %36 %37
               OpReturnValue %38
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // %36 is not a function
  ASSERT_FALSE(TransformationSetFunctionControl(36, SpvFunctionControlMaskNone)
                   .IsApplicable(context.get(), transformation_context));
  // Cannot add the Pure function control to %4 as it did not already have it
  ASSERT_FALSE(TransformationSetFunctionControl(4, SpvFunctionControlPureMask)
                   .IsApplicable(context.get(), transformation_context));
  // Cannot add the Const function control to %21 as it did not already
  // have it
  ASSERT_FALSE(TransformationSetFunctionControl(21, SpvFunctionControlConstMask)
                   .IsApplicable(context.get(), transformation_context));

  // Set to None, removing Const
  TransformationSetFunctionControl transformation1(11,
                                                   SpvFunctionControlMaskNone);
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);

  // Set to Inline; silly to do it on an entry point, but it is allowed
  TransformationSetFunctionControl transformation2(
      4, SpvFunctionControlInlineMask);
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);

  // Set to Pure, removing DontInline
  TransformationSetFunctionControl transformation3(17,
                                                   SpvFunctionControlPureMask);
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);

  // Change from Inline to DontInline
  TransformationSetFunctionControl transformation4(
      13, SpvFunctionControlDontInlineMask);
  ASSERT_TRUE(
      transformation4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation4, context.get(),
                        &transformation_context);

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %54
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %11 "foo(i1;i1;"
               OpName %9 "a"
               OpName %10 "b"
               OpName %13 "bar("
               OpName %17 "baz(i1;"
               OpName %16 "x"
               OpName %21 "boo(i1;i1;"
               OpName %19 "a"
               OpName %20 "b"
               OpName %29 "g"
               OpName %42 "param"
               OpName %44 "param"
               OpName %45 "param"
               OpName %48 "param"
               OpName %49 "param"
               OpName %54 "color"
               OpDecorate %54 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %6 %7 %7
         %15 = OpTypeFunction %6 %7
         %28 = OpTypePointer Private %6
         %29 = OpVariable %28 Private
         %30 = OpConstant %6 2
         %31 = OpConstant %6 5
         %51 = OpTypeFloat 32
         %52 = OpTypeVector %51 4
         %53 = OpTypePointer Output %52
         %54 = OpVariable %53 Output
          %4 = OpFunction %2 Inline %3
          %5 = OpLabel
         %42 = OpVariable %7 Function
         %44 = OpVariable %7 Function
         %45 = OpVariable %7 Function
         %48 = OpVariable %7 Function
         %49 = OpVariable %7 Function
         %41 = OpFunctionCall %2 %13
               OpStore %42 %30
         %43 = OpFunctionCall %6 %17 %42
               OpStore %44 %31
         %46 = OpLoad %6 %29
               OpStore %45 %46
         %47 = OpFunctionCall %6 %21 %44 %45
               OpStore %48 %43
               OpStore %49 %47
         %50 = OpFunctionCall %6 %11 %48 %49
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %10 = OpFunctionParameter %7
         %12 = OpLabel
         %23 = OpLoad %6 %9
         %24 = OpLoad %6 %10
         %25 = OpIAdd %6 %23 %24
               OpReturnValue %25
               OpFunctionEnd
         %13 = OpFunction %2 DontInline %3
         %14 = OpLabel
               OpStore %29 %30
               OpReturn
               OpFunctionEnd
         %17 = OpFunction %6 Pure %15
         %16 = OpFunctionParameter %7
         %18 = OpLabel
         %32 = OpLoad %6 %16
         %33 = OpIAdd %6 %31 %32
               OpReturnValue %33
               OpFunctionEnd
         %21 = OpFunction %6 DontInline %8
         %19 = OpFunctionParameter %7
         %20 = OpFunctionParameter %7
         %22 = OpLabel
         %36 = OpLoad %6 %19
         %37 = OpLoad %6 %20
         %38 = OpIMul %6 %36 %37
               OpReturnValue %38
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
