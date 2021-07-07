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

#include "source/fuzz/transformation_add_parameter.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddParameterTest, NonPointerBasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %7 = OpTypeBool
         %11 = OpTypeInt 32 1
         %16 = OpTypeFloat 32
         %51 = OpConstant %11 2
         %52 = OpTypeArray %16 %51
         %53 = OpConstant %16 7
         %54 = OpConstantComposite %52 %53 %53
          %3 = OpTypeFunction %2
          %6 = OpTypeFunction %7 %7
          %8 = OpConstant %11 23
         %12 = OpConstantTrue %7
         %15 = OpTypeFunction %2 %16
         %24 = OpTypeFunction %2 %16 %7
         %31 = OpTypeStruct %7 %11
         %32 = OpConstant %16 23
         %33 = OpConstantComposite %31 %12 %8
         %41 = OpTypeStruct %11 %16
         %42 = OpConstantComposite %41 %8 %32
         %43 = OpTypeFunction %2 %41
         %44 = OpTypeFunction %2 %41 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %13 = OpFunctionCall %7 %9 %12
               OpReturn
               OpFunctionEnd

          ; adjust type of the function in-place
          %9 = OpFunction %7 None %6
         %14 = OpFunctionParameter %7
         %10 = OpLabel
               OpReturnValue %12
               OpFunctionEnd

         ; reuse an existing function type
         %17 = OpFunction %2 None %15
         %18 = OpFunctionParameter %16
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
         %20 = OpFunction %2 None %15
         %21 = OpFunctionParameter %16
         %22 = OpLabel
               OpReturn
               OpFunctionEnd
         %25 = OpFunction %2 None %24
         %26 = OpFunctionParameter %16
         %27 = OpFunctionParameter %7
         %28 = OpLabel
               OpReturn
               OpFunctionEnd

         ; create a new function type
         %29 = OpFunction %2 None %3
         %30 = OpLabel
               OpReturn
               OpFunctionEnd

         ; don't adjust the type of the function if it creates a duplicate
         %34 = OpFunction %2 None %43
         %35 = OpFunctionParameter %41
         %36 = OpLabel
               OpReturn
               OpFunctionEnd
         %37 = OpFunction %2 None %44
         %38 = OpFunctionParameter %41
         %39 = OpFunctionParameter %7
         %40 = OpLabel
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
  // Can't modify entry point function.
  ASSERT_FALSE(TransformationAddParameter(4, 60, 7, {{}}, 61)
                   .IsApplicable(context.get(), transformation_context));

  // There is no function with result id 60.
  ASSERT_FALSE(TransformationAddParameter(60, 60, 11, {{}}, 61)
                   .IsApplicable(context.get(), transformation_context));

  // Parameter id is not fresh.
  ASSERT_FALSE(TransformationAddParameter(9, 14, 11, {{{13, 8}}}, 61)
                   .IsApplicable(context.get(), transformation_context));

  // Function type id is not fresh.
  ASSERT_FALSE(TransformationAddParameter(9, 60, 11, {{{13, 8}}}, 14)
                   .IsApplicable(context.get(), transformation_context));

  // Function type id and parameter type id are equal.
  ASSERT_FALSE(TransformationAddParameter(9, 60, 11, {{{13, 8}}}, 60)
                   .IsApplicable(context.get(), transformation_context));

  // Parameter's initializer doesn't exist.
  ASSERT_FALSE(TransformationAddParameter(9, 60, 11, {{{13, 60}}}, 61)
                   .IsApplicable(context.get(), transformation_context));

  // Correct transformations.
  {
    TransformationAddParameter correct(9, 60, 11, {{{13, 8}}}, 61);
    ASSERT_TRUE(correct.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(correct, context.get(), &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(60));
  }
  {
    TransformationAddParameter correct(9, 68, 52, {{{13, 54}}}, 69);
    ASSERT_TRUE(correct.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(correct, context.get(), &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(68));
  }
  {
    TransformationAddParameter correct(17, 62, 7, {{}}, 63);
    ASSERT_TRUE(correct.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(correct, context.get(), &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(62));
  }
  {
    TransformationAddParameter correct(29, 64, 31, {{}}, 65);
    ASSERT_TRUE(correct.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(correct, context.get(), &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(64));
  }
  {
    TransformationAddParameter correct(34, 66, 7, {{}}, 67);
    ASSERT_TRUE(correct.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(correct, context.get(), &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(66));
  }

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %7 = OpTypeBool
         %11 = OpTypeInt 32 1
         %16 = OpTypeFloat 32
         %51 = OpConstant %11 2
         %52 = OpTypeArray %16 %51
         %53 = OpConstant %16 7
         %54 = OpConstantComposite %52 %53 %53
          %3 = OpTypeFunction %2
          %8 = OpConstant %11 23
         %12 = OpConstantTrue %7
         %15 = OpTypeFunction %2 %16
         %24 = OpTypeFunction %2 %16 %7
         %31 = OpTypeStruct %7 %11
         %32 = OpConstant %16 23
         %33 = OpConstantComposite %31 %12 %8
         %41 = OpTypeStruct %11 %16
         %42 = OpConstantComposite %41 %8 %32
         %44 = OpTypeFunction %2 %41 %7
          %6 = OpTypeFunction %7 %7 %11 %52
         %65 = OpTypeFunction %2 %31
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %13 = OpFunctionCall %7 %9 %12 %8 %54
               OpReturn
               OpFunctionEnd

          ; adjust type of the function in-place
          %9 = OpFunction %7 None %6
         %14 = OpFunctionParameter %7
         %60 = OpFunctionParameter %11
         %68 = OpFunctionParameter %52
         %10 = OpLabel
               OpReturnValue %12
               OpFunctionEnd

         ; reuse an existing function type
         %17 = OpFunction %2 None %24
         %18 = OpFunctionParameter %16
         %62 = OpFunctionParameter %7
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
         %20 = OpFunction %2 None %15
         %21 = OpFunctionParameter %16
         %22 = OpLabel
               OpReturn
               OpFunctionEnd
         %25 = OpFunction %2 None %24
         %26 = OpFunctionParameter %16
         %27 = OpFunctionParameter %7
         %28 = OpLabel
               OpReturn
               OpFunctionEnd

         ; create a new function type
         %29 = OpFunction %2 None %65
         %64 = OpFunctionParameter %31
         %30 = OpLabel
               OpReturn
               OpFunctionEnd

         ; don't adjust the type of the function if it creates a duplicate
         %34 = OpFunction %2 None %44
         %35 = OpFunctionParameter %41
         %66 = OpFunctionParameter %7
         %36 = OpLabel
               OpReturn
               OpFunctionEnd
         %37 = OpFunction %2 None %44
         %38 = OpFunctionParameter %41
         %39 = OpFunctionParameter %7
         %40 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationAddParameterTest, NonPointerNotApplicableTest) {
  // This types handles case of adding a new parameter of a non-pointer type
  // where the transformation is not applicable.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun1("
               OpName %12 "fun2(i1;"
               OpName %11 "a"
               OpName %14 "fun3("
               OpName %24 "f1"
               OpName %27 "f2"
               OpName %30 "i1"
               OpName %31 "i2"
               OpName %32 "param"
               OpName %35 "i3"
               OpName %36 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %8 %9
         %18 = OpConstant %8 2
         %22 = OpTypeFloat 32
         %23 = OpTypePointer Private %22
         %24 = OpVariable %23 Private
         %25 = OpConstant %22 1
         %26 = OpTypePointer Function %22
         %28 = OpConstant %22 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %27 = OpVariable %26 Function
         %30 = OpVariable %9 Function
         %31 = OpVariable %9 Function
         %32 = OpVariable %9 Function
         %35 = OpVariable %9 Function
         %36 = OpVariable %9 Function
               OpStore %24 %25
               OpStore %27 %28
         %29 = OpFunctionCall %2 %6
               OpStore %30 %18
         %33 = OpLoad %8 %30
               OpStore %32 %33
         %34 = OpFunctionCall %8 %12 %32
               OpStore %31 %34
         %37 = OpLoad %8 %31
               OpStore %36 %37
         %38 = OpFunctionCall %8 %12 %36
               OpStore %35 %38
         ; %39 = OpFunctionCall %2 %14
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %8 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %17 = OpLoad %8 %11
         %19 = OpIAdd %8 %17 %18
               OpReturnValue %19
               OpFunctionEnd
         %14 = OpFunction %2 None %3
         %15 = OpLabel
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
  // Bad: Id 19 is not available in the caller that has id 34.
  TransformationAddParameter transformation_bad_1(12, 50, 8,
                                                  {{{34, 19}, {38, 19}}}, 51);

  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Bad: Id 8 does not have a type.
  TransformationAddParameter transformation_bad_2(12, 50, 8,
                                                  {{{34, 8}, {38, 8}}}, 51);

  ASSERT_FALSE(
      transformation_bad_2.IsApplicable(context.get(), transformation_context));

  // Bad: Types of id 25 and id 18 are different.
  TransformationAddParameter transformation_bad_3(12, 50, 22,
                                                  {{{34, 25}, {38, 18}}}, 51);
  ASSERT_FALSE(
      transformation_bad_3.IsApplicable(context.get(), transformation_context));

  // Function with id 14 does not have any callers.
  // Bad: Id 18 is not a vaild type.
  TransformationAddParameter transformation_bad_4(14, 50, 18, {{}}, 51);
  ASSERT_FALSE(
      transformation_bad_4.IsApplicable(context.get(), transformation_context));

  // Function with id 14 does not have any callers.
  // Bad:  Id 3 refers to OpTypeVoid, which is not supported.
  TransformationAddParameter transformation_bad_6(14, 50, 3, {{}}, 51);
  ASSERT_FALSE(
      transformation_bad_6.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddParameterTest, PointerFunctionTest) {
  // This types handles case of adding a new parameter of a pointer type with
  // storage class Function.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun1("
               OpName %12 "fun2(i1;"
               OpName %11 "a"
               OpName %14 "fun3("
               OpName %17 "s"
               OpName %24 "s"
               OpName %28 "f1"
               OpName %31 "f2"
               OpName %34 "i1"
               OpName %35 "i2"
               OpName %36 "param"
               OpName %39 "i3"
               OpName %40 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %8 %9
         %20 = OpConstant %8 2
         %25 = OpConstant %8 0
         %26 = OpTypeFloat 32
         %27 = OpTypePointer Private %26
         %28 = OpVariable %27 Private
         %60 = OpTypePointer Output %26
         %61 = OpVariable %60 Output
         %29 = OpConstant %26 1
         %30 = OpTypePointer Function %26
         %32 = OpConstant %26 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %31 = OpVariable %30 Function
         %34 = OpVariable %9 Function
         %35 = OpVariable %9 Function
         %36 = OpVariable %9 Function
         %39 = OpVariable %9 Function
         %40 = OpVariable %9 Function
               OpStore %28 %29
               OpStore %31 %32
         %33 = OpFunctionCall %2 %6
               OpStore %34 %20
         %37 = OpLoad %8 %34
               OpStore %36 %37
         %38 = OpFunctionCall %8 %12 %36
               OpStore %35 %38
         %41 = OpLoad %8 %35
               OpStore %40 %41
         %42 = OpFunctionCall %8 %12 %40
               OpStore %39 %42
         %43 = OpFunctionCall %2 %14
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %8 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %17 = OpVariable %9 Function
         %18 = OpLoad %8 %11
               OpStore %17 %18
         %19 = OpLoad %8 %17
         %21 = OpIAdd %8 %19 %20
               OpReturnValue %21
               OpFunctionEnd
         %14 = OpFunction %2 None %3
         %15 = OpLabel
         %24 = OpVariable %9 Function
               OpStore %24 %25
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
  // Bad: Pointer of id 61 has storage class Output, which is not supported.
  TransformationAddParameter transformation_bad_1(12, 50, 60,
                                                  {{{38, 61}, {42, 61}}}, 51);

  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Good: Local variable of id 31 is defined in the caller (main).
  TransformationAddParameter transformation_good_1(12, 50, 30,
                                                   {{{38, 31}, {42, 31}}}, 51);
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Good: Local variable of id 34 is defined in the caller (main).
  TransformationAddParameter transformation_good_2(14, 52, 9, {{{43, 34}}}, 53);
  ASSERT_TRUE(transformation_good_2.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Good: Local variable of id 39 is defined in the caller (main).
  TransformationAddParameter transformation_good_3(6, 54, 9, {{{33, 39}}}, 55);
  ASSERT_TRUE(transformation_good_3.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Good: This adds another pointer parameter to the function of id 6.
  TransformationAddParameter transformation_good_4(6, 56, 30, {{{33, 31}}}, 57);
  ASSERT_TRUE(transformation_good_4.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_4, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun1("
               OpName %12 "fun2(i1;"
               OpName %11 "a"
               OpName %14 "fun3("
               OpName %17 "s"
               OpName %24 "s"
               OpName %28 "f1"
               OpName %31 "f2"
               OpName %34 "i1"
               OpName %35 "i2"
               OpName %36 "param"
               OpName %39 "i3"
               OpName %40 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %20 = OpConstant %8 2
         %25 = OpConstant %8 0
         %26 = OpTypeFloat 32
         %27 = OpTypePointer Private %26
         %28 = OpVariable %27 Private
         %60 = OpTypePointer Output %26
         %61 = OpVariable %60 Output
         %29 = OpConstant %26 1
         %30 = OpTypePointer Function %26
         %32 = OpConstant %26 2
         %10 = OpTypeFunction %8 %9 %30
         %53 = OpTypeFunction %2 %9
         %57 = OpTypeFunction %2 %9 %30
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %31 = OpVariable %30 Function
         %34 = OpVariable %9 Function
         %35 = OpVariable %9 Function
         %36 = OpVariable %9 Function
         %39 = OpVariable %9 Function
         %40 = OpVariable %9 Function
               OpStore %28 %29
               OpStore %31 %32
         %33 = OpFunctionCall %2 %6 %39 %31
               OpStore %34 %20
         %37 = OpLoad %8 %34
               OpStore %36 %37
         %38 = OpFunctionCall %8 %12 %36 %31
               OpStore %35 %38
         %41 = OpLoad %8 %35
               OpStore %40 %41
         %42 = OpFunctionCall %8 %12 %40 %31
               OpStore %39 %42
         %43 = OpFunctionCall %2 %14 %34
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %57
         %54 = OpFunctionParameter %9
         %56 = OpFunctionParameter %30
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %8 None %10
         %11 = OpFunctionParameter %9
         %50 = OpFunctionParameter %30
         %13 = OpLabel
         %17 = OpVariable %9 Function
         %18 = OpLoad %8 %11
               OpStore %17 %18
         %19 = OpLoad %8 %17
         %21 = OpIAdd %8 %19 %20
               OpReturnValue %21
               OpFunctionEnd
         %14 = OpFunction %2 None %53
         %52 = OpFunctionParameter %9
         %15 = OpLabel
         %24 = OpVariable %9 Function
               OpStore %24 %25
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationAddParameterTest, PointerPrivateWorkgroupTest) {
  // This types handles case of adding a new parameter of a pointer type with
  // storage class Private or Workgroup.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun1("
               OpName %12 "fun2(i1;"
               OpName %11 "a"
               OpName %14 "fun3("
               OpName %17 "s"
               OpName %24 "s"
               OpName %28 "f1"
               OpName %31 "f2"
               OpName %34 "i1"
               OpName %35 "i2"
               OpName %36 "param"
               OpName %39 "i3"
               OpName %40 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %8 %9
         %20 = OpConstant %8 2
         %25 = OpConstant %8 0
         %26 = OpTypeFloat 32
         %27 = OpTypePointer Private %26
         %28 = OpVariable %27 Private
         %60 = OpTypePointer Workgroup %26
         %61 = OpVariable %60 Workgroup
         %29 = OpConstant %26 1
         %30 = OpTypePointer Function %26
         %32 = OpConstant %26 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %31 = OpVariable %30 Function
         %34 = OpVariable %9 Function
         %35 = OpVariable %9 Function
         %36 = OpVariable %9 Function
         %39 = OpVariable %9 Function
         %40 = OpVariable %9 Function
               OpStore %28 %29
               OpStore %31 %32
         %33 = OpFunctionCall %2 %6
               OpStore %34 %20
         %37 = OpLoad %8 %34
               OpStore %36 %37
         %38 = OpFunctionCall %8 %12 %36
               OpStore %35 %38
         %41 = OpLoad %8 %35
               OpStore %40 %41
         %42 = OpFunctionCall %8 %12 %40
               OpStore %39 %42
         %43 = OpFunctionCall %2 %14
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %8 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %17 = OpVariable %9 Function
         %18 = OpLoad %8 %11
               OpStore %17 %18
         %19 = OpLoad %8 %17
         %21 = OpIAdd %8 %19 %20
               OpReturnValue %21
               OpFunctionEnd
         %14 = OpFunction %2 None %3
         %15 = OpLabel
         %24 = OpVariable %9 Function
               OpStore %24 %25
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
  // Good: Global variable of id 28 (storage class Private) is defined in the
  // caller (main).
  TransformationAddParameter transformation_good_1(12, 70, 27,
                                                   {{{38, 28}, {42, 28}}}, 71);
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Good: Global variable of id 61 is (storage class Workgroup) is defined in
  // the caller (main).
  TransformationAddParameter transformation_good_2(12, 72, 27,
                                                   {{{38, 28}, {42, 28}}}, 73);
  ASSERT_TRUE(transformation_good_2.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_2, context.get(),
                        &transformation_context);

  // Good: Global variable of id 28 (storage class Private) is defined in the
  // caller (main).
  TransformationAddParameter transformation_good_3(6, 74, 27, {{{33, 28}}}, 75);
  ASSERT_TRUE(transformation_good_3.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Good: Global variable of id 61 is (storage class Workgroup) is defined in
  // the caller (main).
  TransformationAddParameter transformation_good_4(6, 76, 60, {{{33, 61}}}, 77);
  ASSERT_TRUE(transformation_good_4.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_4, context.get(),
                        &transformation_context);

  // Good: Global variable of id 28 (storage class Private) is defined in the
  // caller (main).
  TransformationAddParameter transformation_good_5(14, 78, 27, {{{43, 28}}},
                                                   79);
  ASSERT_TRUE(transformation_good_5.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_5, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Good: Global variable of id 61 is (storage class Workgroup) is defined in
  // the caller (main).
  TransformationAddParameter transformation_good_6(14, 80, 60, {{{43, 61}}},
                                                   81);
  ASSERT_TRUE(transformation_good_6.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_6, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "fun1("
               OpName %12 "fun2(i1;"
               OpName %11 "a"
               OpName %14 "fun3("
               OpName %17 "s"
               OpName %24 "s"
               OpName %28 "f1"
               OpName %31 "f2"
               OpName %34 "i1"
               OpName %35 "i2"
               OpName %36 "param"
               OpName %39 "i3"
               OpName %40 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %20 = OpConstant %8 2
         %25 = OpConstant %8 0
         %26 = OpTypeFloat 32
         %27 = OpTypePointer Private %26
         %28 = OpVariable %27 Private
         %60 = OpTypePointer Workgroup %26
         %61 = OpVariable %60 Workgroup
         %29 = OpConstant %26 1
         %30 = OpTypePointer Function %26
         %32 = OpConstant %26 2
         %10 = OpTypeFunction %8 %9 %27 %27
         %75 = OpTypeFunction %2 %27 %60
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %31 = OpVariable %30 Function
         %34 = OpVariable %9 Function
         %35 = OpVariable %9 Function
         %36 = OpVariable %9 Function
         %39 = OpVariable %9 Function
         %40 = OpVariable %9 Function
               OpStore %28 %29
               OpStore %31 %32
         %33 = OpFunctionCall %2 %6 %28 %61
               OpStore %34 %20
         %37 = OpLoad %8 %34
               OpStore %36 %37
         %38 = OpFunctionCall %8 %12 %36 %28 %28
               OpStore %35 %38
         %41 = OpLoad %8 %35
               OpStore %40 %41
         %42 = OpFunctionCall %8 %12 %40 %28 %28
               OpStore %39 %42
         %43 = OpFunctionCall %2 %14 %28 %61
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %75
         %74 = OpFunctionParameter %27
         %76 = OpFunctionParameter %60
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %8 None %10
         %11 = OpFunctionParameter %9
         %70 = OpFunctionParameter %27
         %72 = OpFunctionParameter %27
         %13 = OpLabel
         %17 = OpVariable %9 Function
         %18 = OpLoad %8 %11
               OpStore %17 %18
         %19 = OpLoad %8 %17
         %21 = OpIAdd %8 %19 %20
               OpReturnValue %21
               OpFunctionEnd
         %14 = OpFunction %2 None %75
         %78 = OpFunctionParameter %27
         %80 = OpFunctionParameter %60
         %15 = OpLabel
         %24 = OpVariable %9 Function
               OpStore %24 %25
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationAddParameterTest, PointerMoreEntriesInMapTest) {
  // This types handles case where call_parameter_id has an entry for at least
  // every caller (there are more entries than it is necessary).
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "s"
               OpName %19 "i1"
               OpName %21 "i2"
               OpName %22 "i3"
               OpName %24 "i4"
               OpName %25 "param"
               OpName %28 "i5"
               OpName %29 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %6 %7
         %15 = OpConstant %6 2
         %20 = OpConstant %6 1
         %23 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %19 = OpVariable %7 Function
         %21 = OpVariable %7 Function
         %22 = OpVariable %7 Function
         %24 = OpVariable %7 Function
         %25 = OpVariable %7 Function
         %28 = OpVariable %7 Function
         %29 = OpVariable %7 Function
               OpStore %19 %20
               OpStore %21 %15
               OpStore %22 %23
         %26 = OpLoad %6 %19
               OpStore %25 %26
         %27 = OpFunctionCall %6 %10 %25
               OpStore %24 %27
         %30 = OpLoad %6 %21
               OpStore %29 %30
         %31 = OpFunctionCall %6 %10 %29
               OpStore %28 %31
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
         %13 = OpLoad %6 %9
               OpStore %12 %13
         %14 = OpLoad %6 %12
         %16 = OpIAdd %6 %14 %15
               OpReturnValue %16
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
  // Good: Local variable of id 21 is defined in every caller (id 27 and id 31).
  TransformationAddParameter transformation_good_1(
      10, 70, 7, {{{27, 21}, {31, 21}, {30, 21}}}, 71);
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Good: Local variable of id 28 is defined in every caller (id 27 and id 31).
  TransformationAddParameter transformation_good_2(
      10, 72, 7, {{{27, 28}, {31, 28}, {14, 21}, {16, 14}}}, 73);
  ASSERT_TRUE(transformation_good_2.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string expected_shader = R"(
              OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "s"
               OpName %19 "i1"
               OpName %21 "i2"
               OpName %22 "i3"
               OpName %24 "i4"
               OpName %25 "param"
               OpName %28 "i5"
               OpName %29 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %15 = OpConstant %6 2
         %20 = OpConstant %6 1
         %23 = OpConstant %6 3
          %8 = OpTypeFunction %6 %7 %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %19 = OpVariable %7 Function
         %21 = OpVariable %7 Function
         %22 = OpVariable %7 Function
         %24 = OpVariable %7 Function
         %25 = OpVariable %7 Function
         %28 = OpVariable %7 Function
         %29 = OpVariable %7 Function
               OpStore %19 %20
               OpStore %21 %15
               OpStore %22 %23
         %26 = OpLoad %6 %19
               OpStore %25 %26
         %27 = OpFunctionCall %6 %10 %25 %21 %28
               OpStore %24 %27
         %30 = OpLoad %6 %21
               OpStore %29 %30
         %31 = OpFunctionCall %6 %10 %29 %21 %28
               OpStore %28 %31
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %70 = OpFunctionParameter %7
         %72 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
         %13 = OpLoad %6 %9
               OpStore %12 %13
         %14 = OpLoad %6 %12
         %16 = OpIAdd %6 %14 %15
               OpReturnValue %16
               OpFunctionEnd
    )";
  ASSERT_TRUE(IsEqual(env, expected_shader, context.get()));
}

TEST(TransformationAddParameterTest, PointeeValueIsIrrelevantTest) {
  // This test checks if the transformation has correctly applied the
  // PointeeValueIsIrrelevant fact for new pointer parameters.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "fun(i1;"
               OpName %9 "a"
               OpName %12 "s"
               OpName %20 "b"
               OpName %22 "i1"
               OpName %24 "i2"
               OpName %25 "i3"
               OpName %26 "param"
               OpName %29 "i4"
               OpName %30 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %50 = OpTypePointer Workgroup %6
         %51 = OpVariable %50 Workgroup
          %8 = OpTypeFunction %6 %7
         %15 = OpConstant %6 2
         %19 = OpTypePointer Private %6
         %20 = OpVariable %19 Private
         %21 = OpConstant %6 0
         %23 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %22 = OpVariable %7 Function
         %24 = OpVariable %7 Function
         %25 = OpVariable %7 Function
         %26 = OpVariable %7 Function
         %29 = OpVariable %7 Function
         %30 = OpVariable %7 Function
               OpStore %20 %21
               OpStore %22 %23
               OpStore %24 %15
         %27 = OpLoad %6 %22
               OpStore %26 %27
         %28 = OpFunctionCall %6 %10 %26
               OpStore %25 %28
         %31 = OpLoad %6 %24
               OpStore %30 %31
         %32 = OpFunctionCall %6 %10 %30
               OpStore %29 %32
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %12 = OpVariable %7 Function
         %13 = OpLoad %6 %9
               OpStore %12 %13
         %14 = OpLoad %6 %12
         %16 = OpIAdd %6 %14 %15
               OpReturnValue %16
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
  TransformationAddParameter transformation_good_1(10, 70, 7,
                                                   {{{28, 22}, {32, 22}}}, 71);
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Check if the fact PointeeValueIsIrrelevant is set for the new parameter
  // (storage class Function).
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(70));

  TransformationAddParameter transformation_good_2(10, 72, 19,
                                                   {{{28, 20}, {32, 20}}}, 73);
  ASSERT_TRUE(transformation_good_2.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Check if the fact PointeeValueIsIrrelevant is set for the new parameter
  // (storage class Private).
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(72));

  TransformationAddParameter transformation_good_3(10, 74, 50,
                                                   {{{28, 51}, {32, 51}}}, 75);
  ASSERT_TRUE(transformation_good_3.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Check if the fact PointeeValueIsIrrelevant is set for the new parameter
  // (storage class Workgroup).
  ASSERT_TRUE(
      transformation_context.GetFactManager()->PointeeValueIsIrrelevant(74));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
