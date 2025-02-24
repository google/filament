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

#include "source/fuzz/transformation_add_type_pointer.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddTypePointerTest, BasicTest) {
  // The SPIR-V was obtained from this GLSL:
  //
  // #version 450
  //
  // int x;
  // float y;
  // vec2 z;
  //
  // struct T {
  //   int a, b;
  // };
  //
  // struct S {
  //   T t;
  //   int u;
  // };
  //
  // void main() {
  //   S myS = S(T(1, 2), 3);
  //   myS.u = x;
  // }

  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
               OpName %7 "T"
               OpMemberName %7 0 "a"
               OpMemberName %7 1 "b"
               OpName %8 "S"
               OpMemberName %8 0 "t"
               OpMemberName %8 1 "u"
               OpName %10 "myS"
               OpName %17 "x"
               OpName %23 "y"
               OpName %26 "z"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeStruct %6 %6
          %8 = OpTypeStruct %7 %6
          %9 = OpTypePointer Function %8
         %11 = OpConstant %6 1
         %12 = OpConstant %6 2
         %13 = OpConstantComposite %7 %11 %12
         %14 = OpConstant %6 3
         %15 = OpConstantComposite %8 %13 %14
         %16 = OpTypePointer Private %6
         %17 = OpVariable %16 Private
         %19 = OpTypePointer Function %6
         %21 = OpTypeFloat 32
         %22 = OpTypePointer Private %21
         %23 = OpVariable %22 Private
         %24 = OpTypeVector %21 2
         %25 = OpTypePointer Private %24
         %26 = OpVariable %25 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %10 = OpVariable %9 Function
               OpStore %10 %15
         %18 = OpLoad %6 %17
         %20 = OpAccessChain %19 %10 %11
               OpStore %20 %18
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
  auto bad_type_id_does_not_exist =
      TransformationAddTypePointer(100, spv::StorageClass::Function, 101);
  auto bad_type_id_is_not_type =
      TransformationAddTypePointer(100, spv::StorageClass::Function, 23);
  auto bad_result_id_is_not_fresh =
      TransformationAddTypePointer(17, spv::StorageClass::Function, 21);

  auto good_new_private_pointer_to_t =
      TransformationAddTypePointer(101, spv::StorageClass::Private, 7);
  auto good_new_uniform_pointer_to_t =
      TransformationAddTypePointer(102, spv::StorageClass::Uniform, 7);
  auto good_another_function_pointer_to_s =
      TransformationAddTypePointer(103, spv::StorageClass::Function, 8);
  auto good_new_uniform_pointer_to_s =
      TransformationAddTypePointer(104, spv::StorageClass::Uniform, 8);
  auto good_another_private_pointer_to_float =
      TransformationAddTypePointer(105, spv::StorageClass::Private, 21);
  auto good_new_private_pointer_to_private_pointer_to_float =
      TransformationAddTypePointer(106, spv::StorageClass::Private, 105);
  auto good_new_uniform_pointer_to_vec2 =
      TransformationAddTypePointer(107, spv::StorageClass::Uniform, 24);
  auto good_new_private_pointer_to_uniform_pointer_to_vec2 =
      TransformationAddTypePointer(108, spv::StorageClass::Private, 107);

  ASSERT_FALSE(bad_type_id_does_not_exist.IsApplicable(context.get(),
                                                       transformation_context));
  ASSERT_FALSE(bad_type_id_is_not_type.IsApplicable(context.get(),
                                                    transformation_context));
  ASSERT_FALSE(bad_result_id_is_not_fresh.IsApplicable(context.get(),
                                                       transformation_context));

  {
    auto& transformation = good_new_private_pointer_to_t;
    ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(101));
    ASSERT_EQ(nullptr, context->get_type_mgr()->GetType(101));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
    ASSERT_EQ(spv::Op::OpTypePointer,
              context->get_def_use_mgr()->GetDef(101)->opcode());
    ASSERT_NE(nullptr, context->get_type_mgr()->GetType(101)->AsPointer());
  }

  for (auto& transformation :
       {good_new_uniform_pointer_to_t, good_another_function_pointer_to_s,
        good_new_uniform_pointer_to_s, good_another_private_pointer_to_float,
        good_new_private_pointer_to_private_pointer_to_float,
        good_new_uniform_pointer_to_vec2,
        good_new_private_pointer_to_uniform_pointer_to_vec2}) {
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
               OpName %7 "T"
               OpMemberName %7 0 "a"
               OpMemberName %7 1 "b"
               OpName %8 "S"
               OpMemberName %8 0 "t"
               OpMemberName %8 1 "u"
               OpName %10 "myS"
               OpName %17 "x"
               OpName %23 "y"
               OpName %26 "z"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeStruct %6 %6
          %8 = OpTypeStruct %7 %6
          %9 = OpTypePointer Function %8
         %11 = OpConstant %6 1
         %12 = OpConstant %6 2
         %13 = OpConstantComposite %7 %11 %12
         %14 = OpConstant %6 3
         %15 = OpConstantComposite %8 %13 %14
         %16 = OpTypePointer Private %6
         %17 = OpVariable %16 Private
         %19 = OpTypePointer Function %6
         %21 = OpTypeFloat 32
         %22 = OpTypePointer Private %21
         %23 = OpVariable %22 Private
         %24 = OpTypeVector %21 2
         %25 = OpTypePointer Private %24
         %26 = OpVariable %25 Private
        %101 = OpTypePointer Private %7
        %102 = OpTypePointer Uniform %7
        %103 = OpTypePointer Function %8
        %104 = OpTypePointer Uniform %8
        %105 = OpTypePointer Private %21
        %106 = OpTypePointer Private %105
        %107 = OpTypePointer Uniform %24
        %108 = OpTypePointer Private %107
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %10 = OpVariable %9 Function
               OpStore %10 %15
         %18 = OpLoad %6 %17
         %20 = OpAccessChain %19 %10 %11
               OpStore %20 %18
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
