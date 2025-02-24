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

#include "source/fuzz/transformation_replace_add_sub_mul_with_carrying_extended.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationReplaceAddSubMulWithCarryingExtendedTest,
     NotApplicableBasicChecks) {
  // First conditions in IsApplicable() are checked.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i1"
               OpName %10 "i2"
               OpName %12 "i3"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 2
         %11 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
               OpStore %8 %9
               OpStore %10 %11
         %13 = OpLoad %6 %10
         %14 = OpLoad %6 %8
         %15 = OpSDiv %6 %13 %14
               OpStore %12 %15
               OpReturn
               OpFunctionEnd
    )";
  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Bad: |struct_fresh_id| must be fresh.
  auto transformation_bad_1 =
      TransformationReplaceAddSubMulWithCarryingExtended(14, 15);
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Bad: The transformation cannot be applied to an instruction OpSDiv.
  auto transformation_bad_2 =
      TransformationReplaceAddSubMulWithCarryingExtended(20, 15);
  ASSERT_FALSE(
      transformation_bad_2.IsApplicable(context.get(), transformation_context));

  // Bad: The transformation cannot be applied to an nonexistent instruction.
  auto transformation_bad_3 =
      TransformationReplaceAddSubMulWithCarryingExtended(20, 21);
  ASSERT_FALSE(
      transformation_bad_3.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceAddSubMulWithCarryingExtendedTest,
     NotApplicableDifferingSignedTypes) {
  // Operand types and result types do not match. Not applicable to an operation
  // on vectors with signed integers and operation on signed integers.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "i1"
               OpName %10 "i2"
               OpName %16 "v1"
               OpName %20 "v2"
               OpName %25 "v3"
               OpName %31 "u1"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %14 = OpTypeVector %6 3
         %15 = OpTypePointer Function %14
         %17 = OpConstant %6 0
         %18 = OpConstant %6 2
         %19 = OpConstantComposite %14 %17 %9 %18
         %21 = OpConstant %6 3
         %22 = OpConstant %6 4
         %23 = OpConstant %6 5
         %24 = OpConstantComposite %14 %21 %22 %23
         %29 = OpTypeInt 32 0
         %30 = OpTypePointer Function %29
         %32 = OpConstant %29 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %16 = OpVariable %15 Function
         %20 = OpVariable %15 Function
         %25 = OpVariable %15 Function
         %31 = OpVariable %30 Function
               OpStore %8 %9
         %11 = OpLoad %6 %8
         %12 = OpLoad %6 %8
         %13 = OpISub %6 %11 %12
               OpStore %10 %13
               OpStore %16 %19
               OpStore %20 %24
         %26 = OpLoad %14 %16
         %27 = OpLoad %14 %20
         %28 = OpIAdd %14 %26 %27
               OpStore %25 %28
               OpStore %31 %32
         %40 = OpIMul %6 %32 %18
         %41 = OpIAdd %6 %32 %32
               OpReturn
               OpFunctionEnd
    )";
  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Bad: The transformation cannot be applied to an instruction OpIMul that has
  // different signedness of the types of operands.
  auto transformation_bad_1 =
      TransformationReplaceAddSubMulWithCarryingExtended(50, 40);
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  // Bad: The transformation cannot be applied to an instruction OpIAdd that has
  // different signedness of the result type than the signedness of the types of
  // the operands.
  auto transformation_bad_2 =
      TransformationReplaceAddSubMulWithCarryingExtended(50, 41);
  ASSERT_FALSE(
      transformation_bad_2.IsApplicable(context.get(), transformation_context));

  // Bad: The transformation cannot be applied to the instruction OpIAdd of two
  // vectors that have signed components.
  auto transformation_bad_3 =
      TransformationReplaceAddSubMulWithCarryingExtended(50, 28);
  ASSERT_FALSE(
      transformation_bad_3.IsApplicable(context.get(), transformation_context));

  // Bad: The transformation cannot be applied to the instruction OpISub of two
  // signed integers
  auto transformation_bad_4 =
      TransformationReplaceAddSubMulWithCarryingExtended(50, 13);
  ASSERT_FALSE(
      transformation_bad_4.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceAddSubMulWithCarryingExtendedTest,
     NotApplicableMissingStructTypes) {
  // In all cases the required struct types are missing.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "u1"
               OpName %10 "u2"
               OpName %12 "u3"
               OpName %24 "i1"
               OpName %26 "i2"
               OpName %28 "i3"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %22 = OpTypeInt 32 1
         %23 = OpTypePointer Function %22
         %25 = OpConstant %22 1
         %27 = OpConstant %22 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %24 = OpVariable %23 Function
         %26 = OpVariable %23 Function
         %28 = OpVariable %23 Function
               OpStore %8 %9
               OpStore %10 %11
         %13 = OpLoad %6 %8
         %14 = OpLoad %6 %10
         %15 = OpIAdd %6 %13 %14
               OpStore %12 %15
         %16 = OpLoad %6 %8
         %17 = OpLoad %6 %10
         %18 = OpISub %6 %16 %17
               OpStore %12 %18
         %19 = OpLoad %6 %8
         %20 = OpLoad %6 %10
         %21 = OpIMul %6 %19 %20
               OpStore %12 %21
               OpStore %24 %25
               OpStore %26 %27
         %29 = OpLoad %22 %24
         %30 = OpLoad %22 %26
         %31 = OpIMul %22 %29 %30
               OpStore %28 %31
               OpReturn
               OpFunctionEnd
    )";
  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto transformation_bad_1 =
      TransformationReplaceAddSubMulWithCarryingExtended(50, 15);
  ASSERT_FALSE(
      transformation_bad_1.IsApplicable(context.get(), transformation_context));

  auto transformation_bad_2 =
      TransformationReplaceAddSubMulWithCarryingExtended(50, 18);
  ASSERT_FALSE(
      transformation_bad_2.IsApplicable(context.get(), transformation_context));

  // Bad: The transformation cannot be applied to the instruction OpIAdd of two
  // vectors that have signed components.
  auto transformation_bad_3 =
      TransformationReplaceAddSubMulWithCarryingExtended(50, 21);
  ASSERT_FALSE(
      transformation_bad_3.IsApplicable(context.get(), transformation_context));

  // Bad: The transformation cannot be applied to the instruction OpISub of two
  // signed integers
  auto transformation_bad_4 =
      TransformationReplaceAddSubMulWithCarryingExtended(50, 31);
  ASSERT_FALSE(
      transformation_bad_4.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceAddSubMulWithCarryingExtendedTest,
     ApplicableScenarios) {
  // In this test all of the transformations can be applied. The required struct
  // types are provided.
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "u1"
               OpName %10 "u2"
               OpName %12 "u3"
               OpName %24 "i1"
               OpName %26 "i2"
               OpName %28 "i3"
               OpName %34 "uv1"
               OpName %36 "uv2"
               OpName %39 "uv3"
               OpName %51 "v1"
               OpName %53 "v2"
               OpName %56 "v3"
               OpName %60 "pair_uint"
               OpMemberName %60 0 "u_1"
               OpMemberName %60 1 "u_2"
               OpName %62 "p_uint"
               OpName %63 "pair_uvec2"
               OpMemberName %63 0 "uv_1"
               OpMemberName %63 1 "uv_2"
               OpName %65 "p_uvec2"
               OpName %66 "pair_ivec2"
               OpMemberName %66 0 "v_1"
               OpMemberName %66 1 "v_2"
               OpName %68 "p_ivec2"
               OpName %69 "pair_int"
               OpMemberName %69 0 "i_1"
               OpMemberName %69 1 "i_2"
               OpName %71 "p_int"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %22 = OpTypeInt 32 1
         %23 = OpTypePointer Function %22
         %25 = OpConstant %22 1
         %27 = OpConstant %22 2
         %32 = OpTypeVector %6 2
         %33 = OpTypePointer Function %32
         %35 = OpConstantComposite %32 %9 %11
         %37 = OpConstant %6 3
         %38 = OpConstantComposite %32 %11 %37
         %49 = OpTypeVector %22 2
         %50 = OpTypePointer Function %49
         %52 = OpConstantComposite %49 %25 %27
         %54 = OpConstant %22 3
         %55 = OpConstantComposite %49 %27 %54
         %60 = OpTypeStruct %6 %6
         %61 = OpTypePointer Private %60
         %62 = OpVariable %61 Private
         %63 = OpTypeStruct %32 %32
         %64 = OpTypePointer Private %63
         %65 = OpVariable %64 Private
         %66 = OpTypeStruct %49 %49
         %67 = OpTypePointer Private %66
         %68 = OpVariable %67 Private
         %69 = OpTypeStruct %22 %22
         %70 = OpTypePointer Private %69
         %71 = OpVariable %70 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %24 = OpVariable %23 Function
         %26 = OpVariable %23 Function
         %28 = OpVariable %23 Function
         %34 = OpVariable %33 Function
         %36 = OpVariable %33 Function
         %39 = OpVariable %33 Function
         %51 = OpVariable %50 Function
         %53 = OpVariable %50 Function
         %56 = OpVariable %50 Function
               OpStore %8 %9
               OpStore %10 %11
         %13 = OpLoad %6 %8
         %14 = OpLoad %6 %10
         %15 = OpIAdd %6 %13 %14
               OpStore %12 %15
         %16 = OpLoad %6 %8
         %17 = OpLoad %6 %10
         %18 = OpISub %6 %16 %17
               OpStore %12 %18
         %19 = OpLoad %6 %8
         %20 = OpLoad %6 %10
         %21 = OpIMul %6 %19 %20
               OpStore %12 %21
               OpStore %24 %25
               OpStore %26 %27
         %29 = OpLoad %22 %24
         %30 = OpLoad %22 %26
         %31 = OpIMul %22 %29 %30
               OpStore %28 %31
               OpStore %34 %35
               OpStore %36 %38
         %40 = OpLoad %32 %34
         %41 = OpLoad %32 %36
         %42 = OpIAdd %32 %40 %41
               OpStore %39 %42
         %43 = OpLoad %32 %34
         %44 = OpLoad %32 %36
         %45 = OpISub %32 %43 %44
               OpStore %39 %45
         %46 = OpLoad %32 %34
         %47 = OpLoad %32 %36
         %48 = OpIMul %32 %46 %47
               OpStore %39 %48
               OpStore %51 %52
               OpStore %53 %55
         %57 = OpLoad %49 %51
         %58 = OpLoad %49 %53
         %59 = OpIMul %49 %57 %58
               OpStore %56 %59
               OpReturn
               OpFunctionEnd
    )";
  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto transformation_good_1 =
      TransformationReplaceAddSubMulWithCarryingExtended(80, 15);
  ASSERT_TRUE(transformation_good_1.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_1, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation_good_2 =
      TransformationReplaceAddSubMulWithCarryingExtended(81, 18);
  ASSERT_TRUE(transformation_good_2.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation_good_3 =
      TransformationReplaceAddSubMulWithCarryingExtended(82, 21);
  ASSERT_TRUE(transformation_good_3.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation_good_4 =
      TransformationReplaceAddSubMulWithCarryingExtended(83, 31);
  ASSERT_TRUE(transformation_good_4.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_4, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation_good_5 =
      TransformationReplaceAddSubMulWithCarryingExtended(84, 42);
  ASSERT_TRUE(transformation_good_5.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_5, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation_good_6 =
      TransformationReplaceAddSubMulWithCarryingExtended(85, 45);
  ASSERT_TRUE(transformation_good_6.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_6, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation_good_7 =
      TransformationReplaceAddSubMulWithCarryingExtended(86, 48);
  ASSERT_TRUE(transformation_good_7.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_7, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  auto transformation_good_8 =
      TransformationReplaceAddSubMulWithCarryingExtended(87, 59);
  ASSERT_TRUE(transformation_good_8.IsApplicable(context.get(),
                                                 transformation_context));
  ApplyAndCheckFreshIds(transformation_good_8, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "u1"
               OpName %10 "u2"
               OpName %12 "u3"
               OpName %24 "i1"
               OpName %26 "i2"
               OpName %28 "i3"
               OpName %34 "uv1"
               OpName %36 "uv2"
               OpName %39 "uv3"
               OpName %51 "v1"
               OpName %53 "v2"
               OpName %56 "v3"
               OpName %60 "pair_uint"
               OpMemberName %60 0 "u_1"
               OpMemberName %60 1 "u_2"
               OpName %62 "p_uint"
               OpName %63 "pair_uvec2"
               OpMemberName %63 0 "uv_1"
               OpMemberName %63 1 "uv_2"
               OpName %65 "p_uvec2"
               OpName %66 "pair_ivec2"
               OpMemberName %66 0 "v_1"
               OpMemberName %66 1 "v_2"
               OpName %68 "p_ivec2"
               OpName %69 "pair_int"
               OpMemberName %69 0 "i_1"
               OpMemberName %69 1 "i_2"
               OpName %71 "p_int"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
         %11 = OpConstant %6 2
         %22 = OpTypeInt 32 1
         %23 = OpTypePointer Function %22
         %25 = OpConstant %22 1
         %27 = OpConstant %22 2
         %32 = OpTypeVector %6 2
         %33 = OpTypePointer Function %32
         %35 = OpConstantComposite %32 %9 %11
         %37 = OpConstant %6 3
         %38 = OpConstantComposite %32 %11 %37
         %49 = OpTypeVector %22 2
         %50 = OpTypePointer Function %49
         %52 = OpConstantComposite %49 %25 %27
         %54 = OpConstant %22 3
         %55 = OpConstantComposite %49 %27 %54
         %60 = OpTypeStruct %6 %6
         %61 = OpTypePointer Private %60
         %62 = OpVariable %61 Private
         %63 = OpTypeStruct %32 %32
         %64 = OpTypePointer Private %63
         %65 = OpVariable %64 Private
         %66 = OpTypeStruct %49 %49
         %67 = OpTypePointer Private %66
         %68 = OpVariable %67 Private
         %69 = OpTypeStruct %22 %22
         %70 = OpTypePointer Private %69
         %71 = OpVariable %70 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %12 = OpVariable %7 Function
         %24 = OpVariable %23 Function
         %26 = OpVariable %23 Function
         %28 = OpVariable %23 Function
         %34 = OpVariable %33 Function
         %36 = OpVariable %33 Function
         %39 = OpVariable %33 Function
         %51 = OpVariable %50 Function
         %53 = OpVariable %50 Function
         %56 = OpVariable %50 Function
               OpStore %8 %9
               OpStore %10 %11
         %13 = OpLoad %6 %8
         %14 = OpLoad %6 %10
         %80 = OpIAddCarry %60 %13 %14
         %15 = OpCompositeExtract %6 %80 0
               OpStore %12 %15
         %16 = OpLoad %6 %8
         %17 = OpLoad %6 %10
         %81 = OpISubBorrow %60 %16 %17
         %18 = OpCompositeExtract %6 %81 0
               OpStore %12 %18
         %19 = OpLoad %6 %8
         %20 = OpLoad %6 %10
         %82 = OpUMulExtended %60 %19 %20
         %21 = OpCompositeExtract %6 %82 0
               OpStore %12 %21
               OpStore %24 %25
               OpStore %26 %27
         %29 = OpLoad %22 %24
         %30 = OpLoad %22 %26
         %83 = OpSMulExtended %69 %29 %30
         %31 = OpCompositeExtract %22 %83 0
               OpStore %28 %31
               OpStore %34 %35
               OpStore %36 %38
         %40 = OpLoad %32 %34
         %41 = OpLoad %32 %36
         %84 = OpIAddCarry %63 %40 %41
         %42 = OpCompositeExtract %32 %84 0
               OpStore %39 %42
         %43 = OpLoad %32 %34
         %44 = OpLoad %32 %36
         %85 = OpISubBorrow %63 %43 %44
         %45 = OpCompositeExtract %32 %85 0
               OpStore %39 %45
         %46 = OpLoad %32 %34
         %47 = OpLoad %32 %36
         %86 = OpUMulExtended %63 %46 %47
         %48 = OpCompositeExtract %32 %86 0
               OpStore %39 %48
               OpStore %51 %52
               OpStore %53 %55
         %57 = OpLoad %49 %51
         %58 = OpLoad %49 %53
         %87 = OpSMulExtended %66 %57 %58
         %59 = OpCompositeExtract %49 %87 0
               OpStore %56 %59
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
