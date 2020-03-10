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
  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, recipient_context.get()));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, donor_context.get()));

  FactManager fact_manager;

  auto prng = MakeUnique<PseudoRandomGenerator>(0);
  FuzzerContext fuzzer_context(prng.get(), 100);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(), &fact_manager,
                                      &fuzzer_context, &transformation_sequence,
                                      {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(IsValid(env, recipient_context.get()));
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
  const auto recipient_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, recipient_context.get()));

  const auto donor_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, donor_context.get()));

  FactManager fact_manager;

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0).get(), 100);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(), &fact_manager,
                                      &fuzzer_context, &transformation_sequence,
                                      {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  ASSERT_TRUE(IsValid(env, recipient_context.get()));

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
        %103 = OpVariable %102 Private
        %104 = OpConstant %12 0
        %105 = OpTypePointer Private %6
        %106 = OpTypePointer Function %12
        %107 = OpTypeStruct %12
        %108 = OpTypePointer Private %107
        %109 = OpVariable %108 Private
        %110 = OpTypePointer Private %12
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
        %111 = OpFunction %2 None %3
        %112 = OpLabel
        %113 = OpVariable %100 Function
        %114 = OpVariable %106 Function
        %115 = OpAccessChain %105 %103 %104
        %116 = OpLoad %6 %115
               OpStore %113 %116
        %117 = OpAccessChain %110 %109 %104
        %118 = OpLoad %12 %117
               OpStore %114 %118
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
  const auto recipient_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, recipient_context.get()));

  const auto donor_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, donor_context.get()));

  FactManager fact_manager;

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0).get(), 100);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(), &fact_manager,
                                      &fuzzer_context, &transformation_sequence,
                                      {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  ASSERT_TRUE(IsValid(env, recipient_context.get()));

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
        %101 = OpVariable %100 Private
        %102 = OpTypePointer Private %7
        %103 = OpVariable %102 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %12 = OpLoad %7 %11
               OpStore %9 %12
               OpReturn
               OpFunctionEnd
        %104 = OpFunction %2 None %3
        %105 = OpLabel
        %106 = OpLoad %7 %103
               OpStore %101 %106
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
  const auto recipient_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, recipient_context.get()));

  const auto donor_context = BuildModule(
      env, consumer, recipient_and_donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, donor_context.get()));

  FactManager fact_manager;

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0).get(), 100);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(), &fact_manager,
                                      &fuzzer_context, &transformation_sequence,
                                      {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(IsValid(env, recipient_context.get()));
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
  const auto recipient_context =
      BuildModule(env, consumer, recipient_shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, recipient_context.get()));

  const auto donor_context =
      BuildModule(env, consumer, donor_shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, donor_context.get()));

  FactManager fact_manager;

  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0).get(), 100);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassDonateModules fuzzer_pass(recipient_context.get(), &fact_manager,
                                      &fuzzer_context, &transformation_sequence,
                                      {});

  fuzzer_pass.DonateSingleModule(donor_context.get(), false);

  // We just check that the result is valid.  Checking to what it should be
  // exactly equal to would be very fragile.
  ASSERT_TRUE(IsValid(env, recipient_context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
