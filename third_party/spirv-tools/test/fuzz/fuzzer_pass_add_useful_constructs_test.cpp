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

#include "source/fuzz/fuzzer_pass_add_useful_constructs.h"
#include "source/fuzz/pseudo_random_generator.h"
#include "source/fuzz/uniform_buffer_element_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

bool AddFactHelper(
    FactManager* fact_manager, opt::IRContext* context, uint32_t word,
    const protobufs::UniformBufferElementDescriptor& descriptor) {
  protobufs::FactConstantUniform constant_uniform_fact;
  constant_uniform_fact.add_constant_word(word);
  *constant_uniform_fact.mutable_uniform_buffer_element_descriptor() =
      descriptor;
  protobufs::Fact fact;
  *fact.mutable_constant_uniform_fact() = constant_uniform_fact;
  return fact_manager->AddFact(fact, context);
}

TEST(FuzzerPassAddUsefulConstructsTest, CheckBasicStuffIsAdded) {
  // The SPIR-V came from the following empty GLSL shader:
  //
  // #version 450
  //
  // void main()
  // {
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
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
  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0).get(), 100);
  protobufs::TransformationSequence transformation_sequence;

  FuzzerPassAddUsefulConstructs pass(context.get(), &fact_manager,
                                     &fuzzer_context, &transformation_sequence);
  pass.Apply();
  ASSERT_TRUE(IsValid(env, context.get()));

  std::string after = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
        %100 = OpTypeBool
        %101 = OpTypeInt 32 1
        %102 = OpTypeInt 32 0
        %103 = OpTypeFloat 32
        %104 = OpConstantTrue %100
        %105 = OpConstantFalse %100
        %106 = OpConstant %101 0
        %107 = OpConstant %101 1
        %108 = OpConstant %102 0
        %109 = OpConstant %102 1
        %110 = OpConstant %103 0
        %111 = OpConstant %103 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after, context.get()));
}

TEST(FuzzerPassAddUsefulConstructsTest,
     CheckTypesIndicesAndConstantsAddedForUniformFacts) {
  // The SPIR-V came from the following GLSL shader:
  //
  // #version 450
  //
  // struct S {
  //   int x;
  //   float y;
  //   int z;
  //   int w;
  // };
  //
  // uniform buf {
  //   S s;
  //   uint w[10];
  // };
  //
  // void main() {
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
               OpName %8 "S"
               OpMemberName %8 0 "x"
               OpMemberName %8 1 "y"
               OpMemberName %8 2 "z"
               OpMemberName %8 3 "w"
               OpName %12 "buf"
               OpMemberName %12 0 "s"
               OpMemberName %12 1 "w"
               OpName %14 ""
               OpMemberDecorate %8 0 Offset 0
               OpMemberDecorate %8 1 Offset 4
               OpMemberDecorate %8 2 Offset 8
               OpMemberDecorate %8 3 Offset 12
               OpDecorate %11 ArrayStride 16
               OpMemberDecorate %12 0 Offset 0
               OpMemberDecorate %12 1 Offset 16
               OpDecorate %12 Block
               OpDecorate %14 DescriptorSet 0
               OpDecorate %14 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeStruct %6 %7 %6 %6
          %9 = OpTypeInt 32 0
         %10 = OpConstant %9 10
         %11 = OpTypeArray %9 %10
         %12 = OpTypeStruct %8 %11
         %13 = OpTypePointer Uniform %12
         %14 = OpVariable %13 Uniform
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
  FuzzerContext fuzzer_context(MakeUnique<PseudoRandomGenerator>(0).get(), 100);
  protobufs::TransformationSequence transformation_sequence;

  // Add some uniform facts.

  // buf.s.x == 200
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), 200,
                            MakeUniformBufferElementDescriptor(0, 0, {0, 0})));

  // buf.s.y == 0.5
  const float float_value = 0.5;
  uint32_t float_value_as_uint;
  memcpy(&float_value_as_uint, &float_value, sizeof(float_value));
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), float_value_as_uint,
                            MakeUniformBufferElementDescriptor(0, 0, {0, 1})));

  // buf.s.z == 300
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), 300,
                            MakeUniformBufferElementDescriptor(0, 0, {0, 2})));

  // buf.s.w == 400
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), 400,
                            MakeUniformBufferElementDescriptor(0, 0, {0, 3})));

  // buf.w[6] = 22
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), 22,
                            MakeUniformBufferElementDescriptor(0, 0, {1, 6})));

  // buf.w[8] = 23
  ASSERT_TRUE(AddFactHelper(&fact_manager, context.get(), 23,
                            MakeUniformBufferElementDescriptor(0, 0, {1, 8})));

  // Assert some things about the module that are not true prior to adding the
  // pass

  {
    // No uniform int pointer
    opt::analysis::Integer temp_type_signed_int(32, true);
    opt::analysis::Integer* registered_type_signed_int =
        context->get_type_mgr()
            ->GetRegisteredType(&temp_type_signed_int)
            ->AsInteger();
    opt::analysis::Pointer type_pointer_uniform_signed_int(
        registered_type_signed_int, SpvStorageClassUniform);
    ASSERT_EQ(0,
              context->get_type_mgr()->GetId(&type_pointer_uniform_signed_int));

    // No uniform uint pointer
    opt::analysis::Integer temp_type_unsigned_int(32, false);
    opt::analysis::Integer* registered_type_unsigned_int =
        context->get_type_mgr()
            ->GetRegisteredType(&temp_type_unsigned_int)
            ->AsInteger();
    opt::analysis::Pointer type_pointer_uniform_unsigned_int(
        registered_type_unsigned_int, SpvStorageClassUniform);
    ASSERT_EQ(
        0, context->get_type_mgr()->GetId(&type_pointer_uniform_unsigned_int));

    // No uniform float pointer
    opt::analysis::Float temp_type_float(32);
    opt::analysis::Float* registered_type_float =
        context->get_type_mgr()->GetRegisteredType(&temp_type_float)->AsFloat();
    opt::analysis::Pointer type_pointer_uniform_float(registered_type_float,
                                                      SpvStorageClassUniform);
    ASSERT_EQ(0, context->get_type_mgr()->GetId(&type_pointer_uniform_float));

    // No int constants 200, 300 nor 400
    opt::analysis::IntConstant int_constant_200(registered_type_signed_int,
                                                {200});
    opt::analysis::IntConstant int_constant_300(registered_type_signed_int,
                                                {300});
    opt::analysis::IntConstant int_constant_400(registered_type_signed_int,
                                                {400});
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_200));
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_300));
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_400));

    // No float constant 0.5
    opt::analysis::FloatConstant float_constant_zero_point_five(
        registered_type_float, {float_value_as_uint});
    ASSERT_EQ(nullptr, context->get_constant_mgr()->FindConstant(
                           &float_constant_zero_point_five));

    // No uint constant 22
    opt::analysis::IntConstant uint_constant_22(registered_type_unsigned_int,
                                                {22});
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&uint_constant_22));

    // No uint constant 23
    opt::analysis::IntConstant uint_constant_23(registered_type_unsigned_int,
                                                {23});
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&uint_constant_23));

    // No int constants 0, 1, 2, 3, 6, 8
    opt::analysis::IntConstant int_constant_0(registered_type_signed_int, {0});
    opt::analysis::IntConstant int_constant_1(registered_type_signed_int, {1});
    opt::analysis::IntConstant int_constant_2(registered_type_signed_int, {2});
    opt::analysis::IntConstant int_constant_3(registered_type_signed_int, {3});
    opt::analysis::IntConstant int_constant_6(registered_type_signed_int, {6});
    opt::analysis::IntConstant int_constant_8(registered_type_signed_int, {8});
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_0));
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_1));
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_2));
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_3));
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_6));
    ASSERT_EQ(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_8));
  }

  FuzzerPassAddUsefulConstructs pass(context.get(), &fact_manager,
                                     &fuzzer_context, &transformation_sequence);
  pass.Apply();
  ASSERT_TRUE(IsValid(env, context.get()));

  // Now assert some things about the module that should be true following the
  // pass.

  // We reconstruct all necessary types and constants to guard against the type
  // and constant managers for the module having been invalidated.

  {
    // Uniform int pointer now present
    opt::analysis::Integer temp_type_signed_int(32, true);
    opt::analysis::Integer* registered_type_signed_int =
        context->get_type_mgr()
            ->GetRegisteredType(&temp_type_signed_int)
            ->AsInteger();
    opt::analysis::Pointer type_pointer_uniform_signed_int(
        registered_type_signed_int, SpvStorageClassUniform);
    ASSERT_NE(0,
              context->get_type_mgr()->GetId(&type_pointer_uniform_signed_int));

    // Uniform uint pointer now present
    opt::analysis::Integer temp_type_unsigned_int(32, false);
    opt::analysis::Integer* registered_type_unsigned_int =
        context->get_type_mgr()
            ->GetRegisteredType(&temp_type_unsigned_int)
            ->AsInteger();
    opt::analysis::Pointer type_pointer_uniform_unsigned_int(
        registered_type_unsigned_int, SpvStorageClassUniform);
    ASSERT_NE(
        0, context->get_type_mgr()->GetId(&type_pointer_uniform_unsigned_int));

    // Uniform float pointer now present
    opt::analysis::Float temp_type_float(32);
    opt::analysis::Float* registered_type_float =
        context->get_type_mgr()->GetRegisteredType(&temp_type_float)->AsFloat();
    opt::analysis::Pointer type_pointer_uniform_float(registered_type_float,
                                                      SpvStorageClassUniform);
    ASSERT_NE(0, context->get_type_mgr()->GetId(&type_pointer_uniform_float));

    // int constants 200, 300, 400 now present
    opt::analysis::IntConstant int_constant_200(registered_type_signed_int,
                                                {200});
    opt::analysis::IntConstant int_constant_300(registered_type_signed_int,
                                                {300});
    opt::analysis::IntConstant int_constant_400(registered_type_signed_int,
                                                {400});
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_200));
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_300));
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_400));

    // float constant 0.5 now present
    opt::analysis::FloatConstant float_constant_zero_point_five(
        registered_type_float, {float_value_as_uint});
    ASSERT_NE(nullptr, context->get_constant_mgr()->FindConstant(
                           &float_constant_zero_point_five));

    // uint constant 22 now present
    opt::analysis::IntConstant uint_constant_22(registered_type_unsigned_int,
                                                {22});
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&uint_constant_22));

    // uint constant 23 now present
    opt::analysis::IntConstant uint_constant_23(registered_type_unsigned_int,
                                                {23});
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&uint_constant_23));

    // int constants 0, 1, 2, 3, 6, 8 now present
    opt::analysis::IntConstant int_constant_0(registered_type_signed_int, {0});
    opt::analysis::IntConstant int_constant_1(registered_type_signed_int, {1});
    opt::analysis::IntConstant int_constant_2(registered_type_signed_int, {2});
    opt::analysis::IntConstant int_constant_3(registered_type_signed_int, {3});
    opt::analysis::IntConstant int_constant_6(registered_type_signed_int, {6});
    opt::analysis::IntConstant int_constant_8(registered_type_signed_int, {8});
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_0));
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_1));
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_2));
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_3));
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_6));
    ASSERT_NE(nullptr,
              context->get_constant_mgr()->FindConstant(&int_constant_8));
  }
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
