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

#include "source/fuzz/fuzzer_pass_donate_modules.h"

#include <algorithm>

#include "gtest/gtest.h"
#include "source/fuzz/pseudo_random_generator.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(FuzzerPassDonateModulesTest, BasicDonation) {
  std::string recipient_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "m"
               OpName %16 "v"
               OpDecorate %16 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 3
          %8 = OpTypeMatrix %7 2
          %9 = OpTypePointer Private %8
         %10 = OpVariable %9 Private
         %11 = OpTypeInt 32 1
         %12 = OpConstant %11 0
         %13 = OpTypeInt 32 0
         %14 = OpTypeVector %13 4
         %15 = OpTypePointer Private %14
         %16 = OpVariable %15 Private
         %17 = OpConstant %13 2
         %18 = OpTypePointer Private %13
         %22 = OpConstant %13 0
         %23 = OpTypePointer Private %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %19 = OpAccessChain %18 %16 %17
         %20 = OpLoad %13 %19
         %21 = OpConvertUToF %6 %20
         %24 = OpAccessChain %23 %10 %12 %22
               OpStore %24 %21
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %12 "bar(mf24;"
               OpName %11 "m"
               OpName %20 "foo(vu4;"
               OpName %19 "v"
               OpName %23 "x"
               OpName %26 "param"
               OpName %29 "result"
               OpName %31 "i"
               OpName %81 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypeMatrix %7 2
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %9
         %14 = OpTypeInt 32 0
         %15 = OpTypeVector %14 4
         %16 = OpTypePointer Function %15
         %17 = OpTypeInt 32 1
         %18 = OpTypeFunction %17 %16
         %22 = OpTypePointer Function %17
         %24 = OpConstant %14 2
         %25 = OpConstantComposite %15 %24 %24 %24 %24
         %28 = OpTypePointer Function %6
         %30 = OpConstant %6 0
         %32 = OpConstant %17 0
         %39 = OpConstant %17 10
         %40 = OpTypeBool
         %43 = OpConstant %17 3
         %50 = OpConstant %17 1
         %55 = OpConstant %14 0
         %56 = OpTypePointer Function %14
         %59 = OpConstant %14 1
         %65 = OpConstant %17 2
         %68 = OpConstant %6 1
         %69 = OpConstant %6 2
         %70 = OpConstant %6 3
         %71 = OpConstant %6 4
         %72 = OpConstant %14 3
         %76 = OpConstant %6 6
         %77 = OpConstant %6 7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %23 = OpVariable %22 Function
         %26 = OpVariable %16 Function
               OpStore %26 %25
         %27 = OpFunctionCall %17 %20 %26
               OpStore %23 %27
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %29 = OpVariable %28 Function
         %31 = OpVariable %22 Function
               OpStore %29 %30
               OpStore %31 %32
               OpBranch %33
         %33 = OpLabel
               OpLoopMerge %35 %36 None
               OpBranch %37
         %37 = OpLabel
         %38 = OpLoad %17 %31
         %41 = OpSLessThan %40 %38 %39
               OpBranchConditional %41 %34 %35
         %34 = OpLabel
         %42 = OpLoad %17 %31
         %44 = OpExtInst %17 %1 SClamp %42 %32 %43
         %45 = OpAccessChain %28 %11 %32 %44
         %46 = OpLoad %6 %45
         %47 = OpLoad %6 %29
         %48 = OpFAdd %6 %47 %46
               OpStore %29 %48
               OpBranch %36
         %36 = OpLabel
         %49 = OpLoad %17 %31
         %51 = OpIAdd %17 %49 %50
               OpStore %31 %51
               OpBranch %33
         %35 = OpLabel
         %52 = OpLoad %6 %29
               OpReturnValue %52
               OpFunctionEnd
         %20 = OpFunction %17 None %18
         %19 = OpFunctionParameter %16
         %21 = OpLabel
         %81 = OpVariable %9 Function
         %57 = OpAccessChain %56 %19 %55
         %58 = OpLoad %14 %57
         %60 = OpAccessChain %56 %19 %59
         %61 = OpLoad %14 %60
         %62 = OpUGreaterThan %40 %58 %61
               OpSelectionMerge %64 None
               OpBranchConditional %62 %63 %67
         %63 = OpLabel
               OpReturnValue %65
         %67 = OpLabel
         %73 = OpAccessChain %56 %19 %72
         %74 = OpLoad %14 %73
         %75 = OpConvertUToF %6 %74
         %78 = OpCompositeConstruct %7 %30 %68 %69 %70
         %79 = OpCompositeConstruct %7 %71 %75 %76 %77
         %80 = OpCompositeConstruct %8 %78 %79
               OpStore %81 %80
         %82 = OpFunctionCall %6 %12 %81
         %83 = OpConvertFToS %17 %82
               OpReturnValue %83
         %64 = OpLabel
         %85 = OpUndef %17
               OpReturnValue %85
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonationWithUniforms) {
  // This test checks that when donating a shader that contains uniforms,
  // uniform variables and associated pointer types are demoted from having
  // Uniform storage class to Private storage class.
  std::string recipient_and_donor_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
               OpMemberDecorate %19 0 Offset 0
               OpDecorate %19 Block
               OpDecorate %21 DescriptorSet 0
               OpDecorate %21 Binding 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpTypeStruct %6
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %12 = OpTypeInt 32 1
         %13 = OpConstant %12 0
         %14 = OpTypePointer Uniform %6
         %17 = OpTypePointer Function %12
         %19 = OpTypeStruct %12
         %20 = OpTypePointer Uniform %19
         %21 = OpVariable %20 Uniform
         %22 = OpTypePointer Uniform %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %18 = OpVariable %17 Function
         %15 = OpAccessChain %14 %11 %13
         %16 = OpLoad %6 %15
               OpStore %8 %16
         %23 = OpAccessChain %22 %21 %13
         %24 = OpLoad %12 %23
               OpStore %18 %24
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
               OpMemberDecorate %19 0 Offset 0
               OpDecorate %19 Block
               OpDecorate %21 DescriptorSet 0
               OpDecorate %21 Binding 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpTypeStruct %6
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %12 = OpTypeInt 32 1
         %13 = OpConstant %12 0
         %14 = OpTypePointer Uniform %6
         %17 = OpTypePointer Function %12
         %19 = OpTypeStruct %12
         %20 = OpTypePointer Uniform %19
         %21 = OpVariable %20 Uniform
         %22 = OpTypePointer Uniform %12
        %100 = OpTypePointer Function %6
        %101 = OpTypeStruct %6
        %102 = OpTypePointer Private %101
        %104 = OpConstant %6 0
        %105 = OpConstantComposite %101 %104
        %103 = OpVariable %102 Private %105
        %106 = OpConstant %12 0
        %107 = OpTypePointer Private %6
        %108 = OpTypePointer Function %12
        %109 = OpTypeStruct %12
        %110 = OpTypePointer Private %109
        %112 = OpConstantComposite %109 %13
        %111 = OpVariable %110 Private %112
        %113 = OpTypePointer Private %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %18 = OpVariable %17 Function
         %15 = OpAccessChain %14 %11 %13
         %16 = OpLoad %6 %15
               OpStore %8 %16
         %23 = OpAccessChain %22 %21 %13
         %24 = OpLoad %12 %23
               OpStore %18 %24
               OpReturn
               OpFunctionEnd
        %114 = OpFunction %2 None %3
        %115 = OpLabel
        %116 = OpVariable %100 Function %104
        %117 = OpVariable %108 Function %13
        %118 = OpAccessChain %107 %103 %106
        %119 = OpLoad %6 %118
               OpStore %116 %119
        %120 = OpAccessChain %113 %111 %106
        %121 = OpLoad %12 %120
               OpStore %117 %121
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, recipient_context.get()));
}

TEST(FuzzerPassDonateModulesTest, DonationWithInputAndOutputVariables) {
  // This test checks that when donating a shader that contains input and output
  // variables, such variables and associated pointer types are demoted to have
  // the Private storage class.
  std::string recipient_and_donor_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %9 %11
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %9 Location 0
               OpDecorate %11 Location 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Output %7
          %9 = OpVariable %8 Output
         %10 = OpTypePointer Input %7
         %11 = OpVariable %10 Input
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %12 = OpLoad %7 %11
               OpStore %9 %12
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %9 %11
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %9 Location 0
               OpDecorate %11 Location 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Output %7
          %9 = OpVariable %8 Output
         %10 = OpTypePointer Input %7
         %11 = OpVariable %10 Input
        %100 = OpTypePointer Private %7
        %102 = OpConstant %6 0
        %103 = OpConstantComposite %7 %102 %102 %102 %102
        %101 = OpVariable %100 Private %103
        %104 = OpTypePointer Private %7
        %105 = OpVariable %104 Private %103
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %12 = OpLoad %7 %11
               OpStore %9 %12
               OpReturn
               OpFunctionEnd
        %106 = OpFunction %2 None %3
        %107 = OpLabel
        %108 = OpLoad %7 %105
               OpStore %101 %108
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, recipient_context.get()));
}

TEST(FuzzerPassDonateModulesTest, DonateFunctionTypeWithDifferentPointers) {
  std::string recipient_and_donor_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %7 Function
         %10 = OpFunctionCall %2 %11 %9
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %2 None %8
         %12 = OpFunctionParameter %7
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonateOpConstantNull) {
  std::string recipient_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Private %6
          %8 = OpConstantNull %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonateCodeThatUsesImages) {
  std::string recipient_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
               OpName %4 "main"
               OpName %10 "mySampler"
               OpName %21 "myTexture"
               OpName %33 "v"
               OpDecorate %10 RelaxedPrecision
               OpDecorate %10 DescriptorSet 0
               OpDecorate %10 Binding 0
               OpDecorate %11 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %21 DescriptorSet 0
               OpDecorate %21 Binding 1
               OpDecorate %22 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeImage %6 2D 0 0 0 1 Unknown
          %8 = OpTypeSampledImage %7
          %9 = OpTypePointer UniformConstant %8
         %10 = OpVariable %9 UniformConstant
         %12 = OpTypeInt 32 1
         %13 = OpConstant %12 2
         %15 = OpTypeVector %12 2
         %17 = OpTypeInt 32 0
         %18 = OpConstant %17 0
         %20 = OpTypePointer UniformConstant %7
         %21 = OpVariable %20 UniformConstant
         %23 = OpConstant %12 1
         %25 = OpConstant %17 1
         %27 = OpTypeBool
         %31 = OpTypeVector %6 4
         %32 = OpTypePointer Function %31
         %35 = OpConstantComposite %15 %23 %23
         %36 = OpConstant %12 3
         %37 = OpConstant %12 4
         %38 = OpConstantComposite %15 %36 %37
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %33 = OpVariable %32 Function
         %11 = OpLoad %8 %10
         %14 = OpImage %7 %11
         %16 = OpImageQuerySizeLod %15 %14 %13
         %19 = OpCompositeExtract %12 %16 0
         %22 = OpLoad %7 %21
         %24 = OpImageQuerySizeLod %15 %22 %23
         %26 = OpCompositeExtract %12 %24 1
         %28 = OpSGreaterThan %27 %19 %26
               OpSelectionMerge %30 None
               OpBranchConditional %28 %29 %41
         %29 = OpLabel
         %34 = OpLoad %8 %10
         %39 = OpImage %7 %34
         %40 = OpImageFetch %31 %39 %35 Lod|ConstOffset %13 %38
               OpStore %33 %40
               OpBranch %30
         %41 = OpLabel
         %42 = OpLoad %7 %21
         %43 = OpImageFetch %31 %42 %35 Lod|ConstOffset %13 %38
               OpStore %33 %43
               OpBranch %30
         %30 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonateCodeThatUsesSampler) {
  std::string recipient_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpDecorate %16 DescriptorSet 0
               OpDecorate %16 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 64
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %23 = OpTypeFloat 32
          %6 = OpTypeImage %23 2D 2 0 0 1 Unknown
         %47 = OpTypePointer UniformConstant %6
         %12 = OpVariable %47 UniformConstant
         %15 = OpTypeSampler
         %55 = OpTypePointer UniformConstant %15
         %17 = OpTypeSampledImage %6
         %16 = OpVariable %55 UniformConstant
         %37 = OpTypeVector %23 4
        %109 = OpConstant %23 0
         %66 = OpConstantComposite %37 %109 %109 %109 %109
         %56 = OpTypeBool
         %54 = OpConstantTrue %56
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %50
         %50 = OpLabel
         %51 = OpPhi %37 %66 %5 %111 %53
               OpLoopMerge %52 %53 None
               OpBranchConditional %54 %53 %52
         %53 = OpLabel
        %106 = OpLoad %6 %12
        %107 = OpLoad %15 %16
        %110 = OpSampledImage %17 %106 %107
        %111 = OpImageSampleImplicitLod %37 %110 %66 Bias %109
               OpBranch %50
         %52 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonateCodeThatUsesImageStructField) {
  std::string recipient_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
               OpName %4 "main"
               OpName %10 "mySampler"
               OpName %21 "myTexture"
               OpName %33 "v"
               OpDecorate %10 RelaxedPrecision
               OpDecorate %10 DescriptorSet 0
               OpDecorate %10 Binding 0
               OpDecorate %11 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %21 DescriptorSet 0
               OpDecorate %21 Binding 1
               OpDecorate %22 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeImage %6 2D 0 0 0 1 Unknown
          %8 = OpTypeSampledImage %7
          %9 = OpTypePointer UniformConstant %8
         %10 = OpVariable %9 UniformConstant
         %12 = OpTypeInt 32 1
         %13 = OpConstant %12 2
         %15 = OpTypeVector %12 2
         %17 = OpTypeInt 32 0
         %18 = OpConstant %17 0
         %20 = OpTypePointer UniformConstant %7
         %21 = OpVariable %20 UniformConstant
         %23 = OpConstant %12 1
         %25 = OpConstant %17 1
         %27 = OpTypeBool
         %31 = OpTypeVector %6 4
         %32 = OpTypePointer Function %31
         %35 = OpConstantComposite %15 %23 %23
         %36 = OpConstant %12 3
         %37 = OpConstant %12 4
         %38 = OpConstantComposite %15 %36 %37
        %201 = OpTypeStruct %7 %7
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %33 = OpVariable %32 Function
         %11 = OpLoad %8 %10
         %14 = OpImage %7 %11
         %22 = OpLoad %7 %21
        %200 = OpCompositeConstruct %201 %14 %22
        %202 = OpCompositeExtract %7 %200 0
        %203 = OpCompositeExtract %7 %200 1
         %24 = OpImageQuerySizeLod %15 %203 %23
         %16 = OpImageQuerySizeLod %15 %202 %13
         %26 = OpCompositeExtract %12 %24 1
         %19 = OpCompositeExtract %12 %16 0
         %28 = OpSGreaterThan %27 %19 %26
               OpSelectionMerge %30 None
               OpBranchConditional %28 %29 %41
         %29 = OpLabel
         %34 = OpLoad %8 %10
         %39 = OpImage %7 %34
         %40 = OpImageFetch %31 %39 %35 Lod|ConstOffset %13 %38
               OpStore %33 %40
               OpBranch %30
         %41 = OpLabel
         %42 = OpLoad %7 %21
         %43 = OpImageFetch %31 %42 %35 Lod|ConstOffset %13 %38
               OpStore %33 %43
               OpBranch %30
         %30 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonateCodeThatUsesImageFunctionParameter) {
  std::string recipient_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
               OpName %4 "main"
               OpName %10 "mySampler"
               OpName %21 "myTexture"
               OpName %33 "v"
               OpDecorate %10 RelaxedPrecision
               OpDecorate %10 DescriptorSet 0
               OpDecorate %10 Binding 0
               OpDecorate %11 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %21 DescriptorSet 0
               OpDecorate %21 Binding 1
               OpDecorate %22 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeImage %6 2D 0 0 0 1 Unknown
          %8 = OpTypeSampledImage %7
          %9 = OpTypePointer UniformConstant %8
         %10 = OpVariable %9 UniformConstant
         %12 = OpTypeInt 32 1
         %13 = OpConstant %12 2
         %15 = OpTypeVector %12 2
         %17 = OpTypeInt 32 0
         %18 = OpConstant %17 0
         %20 = OpTypePointer UniformConstant %7
         %21 = OpVariable %20 UniformConstant
         %23 = OpConstant %12 1
         %25 = OpConstant %17 1
         %27 = OpTypeBool
         %31 = OpTypeVector %6 4
         %32 = OpTypePointer Function %31
         %35 = OpConstantComposite %15 %23 %23
         %36 = OpConstant %12 3
         %37 = OpConstant %12 4
         %38 = OpConstantComposite %15 %36 %37
        %201 = OpTypeFunction %15 %7 %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %33 = OpVariable %32 Function
         %11 = OpLoad %8 %10
         %14 = OpImage %7 %11
         %16 = OpFunctionCall %15 %200 %14 %13
         %19 = OpCompositeExtract %12 %16 0
         %22 = OpLoad %7 %21
         %24 = OpImageQuerySizeLod %15 %22 %23
         %26 = OpCompositeExtract %12 %24 1
         %28 = OpSGreaterThan %27 %19 %26
               OpSelectionMerge %30 None
               OpBranchConditional %28 %29 %41
         %29 = OpLabel
         %34 = OpLoad %8 %10
         %39 = OpImage %7 %34
         %40 = OpImageFetch %31 %39 %35 Lod|ConstOffset %13 %38
               OpStore %33 %40
               OpBranch %30
         %41 = OpLabel
         %42 = OpLoad %7 %21
         %43 = OpImageFetch %31 %42 %35 Lod|ConstOffset %13 %38
               OpStore %33 %43
               OpBranch %30
         %30 = OpLabel
               OpReturn
               OpFunctionEnd
        %200 = OpFunction %15 None %201
        %202 = OpFunctionParameter %7
        %203 = OpFunctionParameter %12
        %204 = OpLabel
        %205 = OpImageQuerySizeLod %15 %202 %203
               OpReturnValue %205
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonateShaderWithImageStorageClass) {
  std::string recipient_shader = R"(
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
               OpCapability SampledBuffer
               OpCapability ImageBuffer
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "MainPSPacked"
               OpExecutionMode %2 OriginUpperLeft
               OpDecorate %18 DescriptorSet 0
               OpDecorate %18 Binding 128
         %49 = OpTypeInt 32 0
         %50 = OpTypeFloat 32
         %58 = OpConstant %50 1
         %66 = OpConstant %49 0
         %87 = OpTypeVector %50 2
         %88 = OpConstantComposite %87 %58 %58
         %17 = OpTypeImage %49 2D 2 0 0 2 R32ui
        %118 = OpTypePointer UniformConstant %17
        %123 = OpTypeVector %49 2
        %132 = OpTypeVoid
        %133 = OpTypeFunction %132
        %142 = OpTypePointer Image %49
         %18 = OpVariable %118 UniformConstant
          %2 = OpFunction %132 None %133
        %153 = OpLabel
        %495 = OpConvertFToU %123 %88
        %501 = OpImageTexelPointer %142 %18 %495 %66
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), true);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonateComputeShaderWithRuntimeArray) {
  std::string recipient_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 310
               OpDecorate %9 ArrayStride 4
               OpMemberDecorate %10 0 Offset 0
               OpDecorate %10 BufferBlock
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypeRuntimeArray %6
         %10 = OpTypeStruct %9
         %11 = OpTypePointer Uniform %10
         %12 = OpVariable %11 Uniform
         %13 = OpTypeInt 32 0
         %16 = OpConstant %6 0
         %18 = OpConstant %6 1
         %20 = OpTypePointer Uniform %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %14 = OpArrayLength %13 %12 0
         %15 = OpBitcast %6 %14
               OpStore %8 %15
         %17 = OpLoad %6 %8
         %19 = OpISub %6 %17 %18
         %21 = OpAccessChain %20 %12 %16 %19
               OpStore %21 %16
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonateComputeShaderWithRuntimeArrayLivesafe) {
  std::string recipient_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 310
               OpDecorate %16 ArrayStride 4
               OpMemberDecorate %17 0 Offset 0
               OpDecorate %17 BufferBlock
               OpDecorate %19 DescriptorSet 0
               OpDecorate %19 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %16 = OpTypeRuntimeArray %6
         %17 = OpTypeStruct %16
         %18 = OpTypePointer Uniform %17
         %19 = OpVariable %18 Uniform
         %20 = OpTypeInt 32 0
         %23 = OpTypeBool
         %26 = OpConstant %6 32
         %27 = OpTypePointer Uniform %6
         %30 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpStore %8 %9
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %14
         %14 = OpLabel
         %15 = OpLoad %6 %8
         %21 = OpArrayLength %20 %19 0
         %22 = OpBitcast %6 %21
         %24 = OpSLessThan %23 %15 %22
               OpBranchConditional %24 %11 %12
         %11 = OpLabel
         %25 = OpLoad %6 %8
         %28 = OpAccessChain %27 %19 %9 %25
               OpStore %28 %26
               OpBranch %13
         %13 = OpLabel
         %29 = OpLoad %6 %8
         %31 = OpIAdd %6 %29 %30
               OpStore %8 %31
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), true);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonateComputeShaderWithWorkgroupVariables) {
  std::string recipient_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Workgroup %6
          %8 = OpVariable %7 Workgroup
          %9 = OpConstant %6 2
         %10 = OpVariable %7 Workgroup
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpStore %8 %9
         %11 = OpLoad %6 %8
               OpStore %10 %11
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), true);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, DonateComputeShaderWithAtomics) {
  std::string recipient_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 310
               OpMemberDecorate %9 0 Offset 0
               OpDecorate %9 BufferBlock
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
          %9 = OpTypeStruct %6
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %12 = OpTypeInt 32 1
         %13 = OpConstant %12 0
         %14 = OpTypePointer Uniform %6
         %16 = OpConstant %6 1
         %17 = OpConstant %6 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %15 = OpAccessChain %14 %11 %13
         %18 = OpAtomicIAdd %6 %15 %16 %17 %16
               OpStore %8 %18
         %19 = OpAccessChain %14 %11 %13
         %20 = OpLoad %6 %8
         %21 = OpAtomicUMin %6 %19 %16 %17 %20
               OpStore %8 %21
         %22 = OpAccessChain %14 %11 %13
         %23 = OpLoad %6 %8
         %24 = OpAtomicUMax %6 %22 %16 %17 %23
               OpStore %8 %24
         %25 = OpAccessChain %14 %11 %13
         %26 = OpLoad %6 %8
         %27 = OpAtomicAnd %6 %25 %16 %17 %26
               OpStore %8 %27
         %28 = OpAccessChain %14 %11 %13
         %29 = OpLoad %6 %8
         %30 = OpAtomicOr %6 %28 %16 %17 %29
               OpStore %8 %30
         %31 = OpAccessChain %14 %11 %13
         %32 = OpLoad %6 %8
         %33 = OpAtomicXor %6 %31 %16 %17 %32
               OpStore %8 %33
         %34 = OpAccessChain %14 %11 %13
         %35 = OpLoad %6 %8
         %36 = OpAtomicExchange %6 %34 %16 %17 %35
               OpStore %8 %36
         %37 = OpAccessChain %14 %11 %13
         %38 = OpLoad %6 %8
         %39 = OpAtomicCompareExchange %6 %37 %16 %17 %17 %16 %38
               OpStore %8 %39
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), true);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, Miscellaneous1) {
  std::string recipient_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string donor_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "foo("
               OpName %10 "x"
               OpName %12 "i"
               OpName %33 "i"
               OpName %42 "j"
               OpDecorate %10 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 2
         %13 = OpConstant %8 0
         %20 = OpConstant %8 100
         %21 = OpTypeBool
         %29 = OpConstant %8 1
         %40 = OpConstant %8 10
         %43 = OpConstant %8 20
         %61 = OpConstant %8 4
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %33 = OpVariable %9 Function
         %42 = OpVariable %9 Function
         %32 = OpFunctionCall %2 %6
               OpStore %33 %13
               OpBranch %34
         %34 = OpLabel
               OpLoopMerge %36 %37 None
               OpBranch %38
         %38 = OpLabel
         %39 = OpLoad %8 %33
         %41 = OpSLessThan %21 %39 %40
               OpBranchConditional %41 %35 %36
         %35 = OpLabel
               OpStore %42 %43
               OpBranch %44
         %44 = OpLabel
               OpLoopMerge %46 %47 None
               OpBranch %48
         %48 = OpLabel
         %49 = OpLoad %8 %42
         %50 = OpSGreaterThan %21 %49 %13
               OpBranchConditional %50 %45 %46
         %45 = OpLabel
         %51 = OpFunctionCall %2 %6
         %52 = OpLoad %8 %42
         %53 = OpISub %8 %52 %29
               OpStore %42 %53
               OpBranch %47
         %47 = OpLabel
               OpBranch %44
         %46 = OpLabel
               OpBranch %54
         %54 = OpLabel
               OpLoopMerge %56 %57 None
               OpBranch %55
         %55 = OpLabel
         %58 = OpLoad %8 %33
         %59 = OpIAdd %8 %58 %29
               OpStore %33 %59
               OpBranch %57
         %57 = OpLabel
         %60 = OpLoad %8 %33
         %62 = OpSLessThan %21 %60 %61
               OpBranchConditional %62 %54 %56
         %56 = OpLabel
               OpBranch %37
         %37 = OpLabel
         %63 = OpLoad %8 %33
         %64 = OpIAdd %8 %63 %29
               OpStore %33 %64
               OpBranch %34
         %36 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
         %12 = OpVariable %9 Function
               OpStore %10 %11
               OpStore %12 %13
               OpBranch %14
         %14 = OpLabel
               OpLoopMerge %16 %17 None
               OpBranch %18
         %18 = OpLabel
         %19 = OpLoad %8 %12
         %22 = OpSLessThan %21 %19 %20
               OpBranchConditional %22 %15 %16
         %15 = OpLabel
         %23 = OpLoad %8 %12
         %24 = OpLoad %8 %10
         %25 = OpIAdd %8 %24 %23
               OpStore %10 %25
         %26 = OpLoad %8 %10
         %27 = OpIMul %8 %26 %11
               OpStore %10 %27
               OpBranch %17
         %17 = OpLabel
         %28 = OpLoad %8 %12
         %30 = OpIAdd %8 %28 %29
               OpStore %12 %30
               OpBranch %14
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, OpSpecConstantInstructions) {
  std::string donor_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypeInt 32 1
          %8 = OpTypeStruct %6 %6 %7
          %9 = OpSpecConstantTrue %6
         %10 = OpSpecConstantFalse %6
         %11 = OpSpecConstant %7 2
         %12 = OpSpecConstantComposite %8 %9 %10 %11
         %13 = OpSpecConstantOp %6 LogicalEqual %9 %10
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  std::string recipient_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // Check that the module is valid first.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  std::string expected_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
        %100 = OpTypeBool
        %101 = OpTypeInt 32 1
        %102 = OpTypeStruct %100 %100 %101
        %103 = OpConstantTrue %100
        %104 = OpConstantFalse %100
        %105 = OpConstant %101 2
        %106 = OpConstantComposite %102 %103 %104 %105
        %107 = OpSpecConstantOp %100 LogicalEqual %103 %104
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        %108 = OpFunction %2 None %3
        %109 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  // Now check that the transformation has produced the expected result.
  ASSERT_TRUE(IsEqual(env, expected_shader, recipient_context.get()));
}

TEST(FuzzerPassDonateModulesTest, DonationSupportsOpTypeRuntimeArray) {
  std::string donor_shader = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %29 "kernel_1"
               OpEntryPoint GLCompute %37 "kernel_2"
               OpSource OpenCL_C 120
               OpDecorate %2 ArrayStride 4
               OpMemberDecorate %3 0 Offset 0
               OpDecorate %3 Block
               OpMemberDecorate %5 0 Offset 0
               OpMemberDecorate %6 0 Offset 0
               OpDecorate %6 Block
               OpDecorate %21 BuiltIn WorkgroupSize
               OpDecorate %23 DescriptorSet 0
               OpDecorate %23 Binding 0
               OpDecorate %25 SpecId 3
               OpDecorate %18 SpecId 0
               OpDecorate %19 SpecId 1
               OpDecorate %20 SpecId 2
          %1 = OpTypeInt 32 0
          %2 = OpTypeRuntimeArray %1
          %3 = OpTypeStruct %2
          %4 = OpTypePointer StorageBuffer %3
          %5 = OpTypeStruct %1
          %6 = OpTypeStruct %5
          %7 = OpTypePointer PushConstant %6
          %8 = OpTypeFloat 32
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
         %11 = OpTypePointer Workgroup %1
         %12 = OpTypePointer PushConstant %5
         %13 = OpTypePointer StorageBuffer %1
         %14 = OpTypeFunction %1 %1
         %15 = OpTypeVector %1 3
         %16 = OpTypePointer Private %15
         %17 = OpConstant %1 0
         %18 = OpSpecConstant %1 1
         %19 = OpSpecConstant %1 1
         %20 = OpSpecConstant %1 1
         %21 = OpSpecConstantComposite %15 %18 %19 %20
         %25 = OpSpecConstant %1 1
         %26 = OpTypeArray %1 %25
         %27 = OpTypePointer Workgroup %26
         %22 = OpVariable %16 Private %21
         %23 = OpVariable %4 StorageBuffer
         %24 = OpVariable %7 PushConstant
         %28 = OpVariable %27 Workgroup
         %29 = OpFunction %9 None %10
         %30 = OpLabel
         %31 = OpAccessChain %11 %28 %17
         %32 = OpAccessChain %12 %24 %17
         %33 = OpLoad %5 %32
         %34 = OpCompositeExtract %1 %33 0
         %35 = OpFunctionCall %1 %45 %34
         %36 = OpAccessChain %13 %23 %17 %34
               OpStore %36 %35
               OpReturn
               OpFunctionEnd
         %37 = OpFunction %9 None %10
         %38 = OpLabel
         %39 = OpAccessChain %11 %28 %17
         %40 = OpAccessChain %12 %24 %17
         %41 = OpLoad %5 %40
         %42 = OpCompositeExtract %1 %41 0
         %43 = OpFunctionCall %1 %45 %42
         %44 = OpAccessChain %13 %23 %17 %42
               OpStore %44 %43
               OpReturn
               OpFunctionEnd
         %45 = OpFunction %1 Pure %14
         %46 = OpFunctionParameter %1
         %47 = OpLabel
         %48 = OpAccessChain %11 %28 %46
         %49 = OpLoad %1 %48
               OpReturnValue %49
               OpFunctionEnd
  )";

  std::string recipient_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_0;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

TEST(FuzzerPassDonateModulesTest, HandlesCapabilities) {
  std::string donor_shader = R"(
               OpCapability VariablePointersStorageBuffer
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
         %11 = OpConstant %6 23
          %7 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3

          %5 = OpLabel
          %8 = OpVariable %7 Function
               OpBranch %9

          %9 = OpLabel
         %10 = OpPhi %7 %8 %5
               OpStore %10 %11
               OpReturn

               OpFunctionEnd
  )";

  std::string recipient_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  ASSERT_TRUE(donor_context->get_feature_mgr()->HasCapability(
      SpvCapabilityVariablePointersStorageBuffer));
  ASSERT_FALSE(recipient_context->get_feature_mgr()->HasCapability(
      SpvCapabilityVariablePointersStorageBuffer));

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // Check that recipient module hasn't changed.
  ASSERT_TRUE(IsEqual(env, recipient_shader, recipient_context.get()));

  // Add the missing capability.
  //
  // We are adding VariablePointers to test the case when donor and recipient
  // have different OpCapability instructions but the same capabilities. In our
  // example, VariablePointers implicitly declares
  // VariablePointersStorageBuffer. Thus, two modules must be compatible.
  recipient_context->AddCapability(SpvCapabilityVariablePointers);

  ASSERT_TRUE(donor_context->get_feature_mgr()->HasCapability(
      SpvCapabilityVariablePointersStorageBuffer));
  ASSERT_TRUE(recipient_context->get_feature_mgr()->HasCapability(
      SpvCapabilityVariablePointersStorageBuffer));

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // Check that donation was successful.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
        %100 = OpTypeFloat 32
        %101 = OpConstant %100 23
        %102 = OpTypePointer Function %100
        %105 = OpConstant %100 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        %103 = OpFunction %2 None %3
        %104 = OpLabel
        %106 = OpVariable %102 Function %105
               OpBranch %107
        %107 = OpLabel
        %108 = OpPhi %102 %106 %104
               OpStore %108 %101
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, recipient_context.get()));
}

TEST(FuzzerPassDonateModulesTest, HandlesOpPhisInMergeBlock) {
  std::string donor_shader = R"(
               ; OpPhis don't support pointers without this capability
               ; and we need pointers to test some of the functionality
               OpCapability VariablePointers
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
         %14 = OpTypeBool
         %15 = OpConstantTrue %14
         %42 = OpTypePointer Function %14

          ; back-edge block is unreachable in the CFG
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpLoopMerge %8 %7 None
               OpBranch %8
          %7 = OpLabel
               OpBranch %6
          %8 = OpLabel
               OpReturn
               OpFunctionEnd

          ; back-edge block already has an edge to the merge block
          %9 = OpFunction %2 None %3
         %10 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %13 %12 None
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %15 %11 %13
         %13 = OpLabel
               OpReturn
               OpFunctionEnd

         ; merge block has no OpPhis
         %16 = OpFunction %2 None %3
         %17 = OpLabel
               OpBranch %18
         %18 = OpLabel
               OpLoopMerge %20 %19 None
               OpBranchConditional %15 %19 %20
         %19 = OpLabel
               OpBranch %18
         %20 = OpLabel
               OpReturn
               OpFunctionEnd

         ; merge block has OpPhis and some of their operands are available at
         ; the back-edge block
         %21 = OpFunction %2 None %3
         %22 = OpLabel
               OpBranch %23
         %23 = OpLabel
         %24 = OpCopyObject %14 %15
               OpLoopMerge %28 %27 None
               OpBranchConditional %15 %25 %28
         %25 = OpLabel
         %26 = OpCopyObject %14 %15
               OpBranchConditional %15 %28 %27
         %27 = OpLabel
               OpBranch %23
         %28 = OpLabel
         %29 = OpPhi %14 %24 %23 %26 %25
               OpReturn
               OpFunctionEnd

         ; none of the OpPhis' operands dominate the back-edge block but some of
         ; them have basic type
         %30 = OpFunction %2 None %3
         %31 = OpLabel
               OpBranch %32
         %32 = OpLabel
               OpLoopMerge %40 %39 None
               OpBranch %33
         %33 = OpLabel
               OpSelectionMerge %38 None
               OpBranchConditional %15 %34 %36
         %34 = OpLabel
         %35 = OpCopyObject %14 %15
               OpBranchConditional %35 %38 %40
         %36 = OpLabel
         %37 = OpCopyObject %14 %15
               OpBranchConditional %37 %38 %40
         %38 = OpLabel
               OpBranch %39
         %39 = OpLabel
               OpBranch %32
         %40 = OpLabel
         %41 = OpPhi %14 %35 %34 %37 %36
               OpReturn
               OpFunctionEnd

         ; none of the OpPhis' operands dominate the back-edge block and none of
         ; them have basic type
         %43 = OpFunction %2 None %3
         %44 = OpLabel
         %45 = OpVariable %42 Function
               OpBranch %46
         %46 = OpLabel
               OpLoopMerge %54 %53 None
               OpBranch %47
         %47 = OpLabel
               OpSelectionMerge %52 None
               OpBranchConditional %15 %48 %50
         %48 = OpLabel
         %49 = OpCopyObject %42 %45
               OpBranchConditional %15 %52 %54
         %50 = OpLabel
         %51 = OpCopyObject %42 %45
               OpBranchConditional %15 %52 %54
         %52 = OpLabel
               OpBranch %53
         %53 = OpLabel
               OpBranch %46
         %54 = OpLabel
         %55 = OpPhi %42 %49 %48 %51 %50
               OpReturn
               OpFunctionEnd
  )";

  std::string recipient_shader = R"(
               ; OpPhis don't support pointers without this capability
               ; and we need pointers to test some of the functionality
               OpCapability VariablePointers
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  spvtools::ValidatorOptions validator_options;

  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      donor_context.get(), validator_options, kConsoleMessageConsumer));

  TransformationContext transformation_context(
      MakeUnique<FactManager>(recipient_context.get()), validator_options);

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0), 100,
                               false);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(),
                                      &transformation_context, &fuzzer_context,
                                      &transformation_sequence, false, {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), true);

  // We just check that the result is valid. Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
      recipient_context.get(), validator_options, kConsoleMessageConsumer));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
