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

#include "source/fuzz/transformation_add_type_struct.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddTypeStructTest, BasicTest) {
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
  ASSERT_FALSE(TransformationAddTypeStruct(4, {}).IsApplicable(
      context.get(), transformation_context));
  // %1 is not a type
  ASSERT_FALSE(TransformationAddTypeStruct(100, {1}).IsApplicable(
      context.get(), transformation_context));

  // %3 is a function type
  ASSERT_FALSE(TransformationAddTypeStruct(100, {3}).IsApplicable(
      context.get(), transformation_context));

  {
    // %100 = OpTypeStruct %6 %7 %8 %9 %10 %11
    TransformationAddTypeStruct transformation(100, {6, 7, 8, 9, 10, 11});
    ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(100));
    ASSERT_EQ(nullptr, context->get_type_mgr()->GetType(100));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_EQ(SpvOpTypeStruct,
              context->get_def_use_mgr()->GetDef(100)->opcode());
    ASSERT_NE(nullptr, context->get_type_mgr()->GetType(100)->AsStruct());
  }

  TransformationAddTypeStruct transformations[] = {
      // %101 = OpTypeStruct
      TransformationAddTypeStruct(101, {}),

      // %102 = OpTypeStruct %6
      TransformationAddTypeStruct(102, {6}),

      // %103 = OpTypeStruct %6 %6
      TransformationAddTypeStruct(103, {6, 6})};

  for (auto& transformation : transformations) {
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
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
        %100 = OpTypeStruct %6 %7 %8 %9 %10 %11
        %101 = OpTypeStruct
        %102 = OpTypeStruct %6
        %103 = OpTypeStruct %6 %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddTypeStructTest, HandlesBuiltInMembers) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %2 "main"
               OpMemberDecorate %4 0 BuiltIn Position
               OpMemberDecorate %4 1 BuiltIn PointSize
               OpMemberDecorate %4 2 BuiltIn ClipDistance
          %6 = OpTypeFloat 32
          %5 = OpTypeVector %6 4
          %9 = OpTypeInt 32 1
          %8 = OpConstant %9 1
          %7 = OpTypeArray %6 %8
          %4 = OpTypeStruct %5 %6 %7
         %27 = OpTypeVoid
         %28 = OpTypeFunction %27
          %2 = OpFunction %27 None %28
         %29 = OpLabel
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
  // From the spec for the BuiltIn decoration:
  // - When applied to a structure-type member, that structure type cannot
  //   be contained as a member of another structure type.
  //
  // OpTypeStruct with id %4 has BuiltIn members.
  ASSERT_FALSE(TransformationAddTypeStruct(50, {6, 5, 4, 6, 7})
                   .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
