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

#include "source/fuzz/transformation_add_type_array.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddTypeArrayTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %6 2
          %9 = OpTypeVector %6 3
         %10 = OpTypeVector %6 4
         %11 = OpTypeVector %7 2
         %12 = OpConstant %7 3
         %13 = OpConstant %7 0
         %14 = OpConstant %7 -1
         %15 = OpTypeInt 32 0
         %16 = OpConstant %15 5
         %17 = OpConstant %15 0
         %18 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Id already in use
  ASSERT_FALSE(TransformationAddTypeArray(4, 10, 16).IsApplicable(
      context.get(), transformation_context));
  // %1 is not a type
  ASSERT_FALSE(TransformationAddTypeArray(100, 1, 16)
                   .IsApplicable(context.get(), transformation_context));

  // %3 is a function type
  ASSERT_FALSE(TransformationAddTypeArray(100, 3, 16)
                   .IsApplicable(context.get(), transformation_context));

  // %2 is not a constant
  ASSERT_FALSE(TransformationAddTypeArray(100, 11, 2)
                   .IsApplicable(context.get(), transformation_context));

  // %18 is not an integer
  ASSERT_FALSE(TransformationAddTypeArray(100, 11, 18)
                   .IsApplicable(context.get(), transformation_context));

  // %13 is signed 0
  ASSERT_FALSE(TransformationAddTypeArray(100, 11, 13)
                   .IsApplicable(context.get(), transformation_context));

  // %14 is negative
  ASSERT_FALSE(TransformationAddTypeArray(100, 11, 14)
                   .IsApplicable(context.get(), transformation_context));

  // %17 is unsigned 0
  ASSERT_FALSE(TransformationAddTypeArray(100, 11, 17)
                   .IsApplicable(context.get(), transformation_context));

  {
    // %100 = OpTypeArray %10 %16
    TransformationAddTypeArray transformation(100, 10, 16);
    ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(100));
    ASSERT_EQ(nullptr, context->get_type_mgr()->GetType(100));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_EQ(spv::Op::OpTypeArray,
              context->get_def_use_mgr()->GetDef(100)->opcode());
    ASSERT_NE(nullptr, context->get_type_mgr()->GetType(100)->AsArray());
  }

  {
    // %101 = OpTypeArray %7 %12
    TransformationAddTypeArray transformation(101, 7, 12);
    ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(101));
    ASSERT_EQ(nullptr, context->get_type_mgr()->GetType(101));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_EQ(spv::Op::OpTypeArray,
              context->get_def_use_mgr()->GetDef(100)->opcode());
    ASSERT_NE(nullptr, context->get_type_mgr()->GetType(100)->AsArray());
  }

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeInt 32 1
          %8 = OpTypeVector %6 2
          %9 = OpTypeVector %6 3
         %10 = OpTypeVector %6 4
         %11 = OpTypeVector %7 2
         %12 = OpConstant %7 3
         %13 = OpConstant %7 0
         %14 = OpConstant %7 -1
         %15 = OpTypeInt 32 0
         %16 = OpConstant %15 5
         %17 = OpConstant %15 0
         %18 = OpConstant %6 1
        %100 = OpTypeArray %10 %16
        %101 = OpTypeArray %7 %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
