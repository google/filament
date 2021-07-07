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

#include "source/fuzz/transformation_add_global_undef.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddGlobalUndefTest, BasicTest) {
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
  ASSERT_FALSE(TransformationAddGlobalUndef(4, 11).IsApplicable(
      context.get(), transformation_context));
  // %1 is not a type
  ASSERT_FALSE(TransformationAddGlobalUndef(100, 1).IsApplicable(
      context.get(), transformation_context));

  // %3 is a function type
  ASSERT_FALSE(TransformationAddGlobalUndef(100, 3).IsApplicable(
      context.get(), transformation_context));

  {
    // %100 = OpUndef %6
    TransformationAddGlobalUndef transformation(100, 6);
    ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(100));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_EQ(SpvOpUndef, context->get_def_use_mgr()->GetDef(100)->opcode());
  }

  TransformationAddGlobalUndef transformations[] = {

      // %101 = OpUndef %7
      TransformationAddGlobalUndef(101, 7),

      // %102 = OpUndef %8
      TransformationAddGlobalUndef(102, 8),

      // %103 = OpUndef %9
      TransformationAddGlobalUndef(103, 9),

      // %104 = OpUndef %10
      TransformationAddGlobalUndef(104, 10),

      // %105 = OpUndef %11
      TransformationAddGlobalUndef(105, 11)};

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
        %100 = OpUndef %6
        %101 = OpUndef %7
        %102 = OpUndef %8
        %103 = OpUndef %9
        %104 = OpUndef %10
        %105 = OpUndef %11
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
