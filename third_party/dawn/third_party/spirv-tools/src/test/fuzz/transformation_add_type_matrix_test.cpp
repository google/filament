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

#include "source/fuzz/transformation_add_type_matrix.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddTypeMatrixTest, BasicTest) {
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
  ASSERT_FALSE(TransformationAddTypeMatrix(4, 9, 2).IsApplicable(
      context.get(), transformation_context));
  // %1 is not a type
  ASSERT_FALSE(TransformationAddTypeMatrix(100, 1, 2).IsApplicable(
      context.get(), transformation_context));

  // %11 is not a floating-point vector
  ASSERT_FALSE(TransformationAddTypeMatrix(100, 11, 2)
                   .IsApplicable(context.get(), transformation_context));

  {
    // %100 = OpTypeMatrix %8 2
    TransformationAddTypeMatrix transformation(100, 8, 2);
    ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(100));
    ASSERT_EQ(nullptr, context->get_type_mgr()->GetType(100));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_EQ(spv::Op::OpTypeMatrix,
              context->get_def_use_mgr()->GetDef(100)->opcode());
    ASSERT_NE(nullptr, context->get_type_mgr()->GetType(100)->AsMatrix());
  }

  TransformationAddTypeMatrix transformations[] = {
      // %101 = OpTypeMatrix %8 3
      TransformationAddTypeMatrix(101, 8, 3),

      // %102 = OpTypeMatrix %8 4
      TransformationAddTypeMatrix(102, 8, 4),

      // %103 = OpTypeMatrix %9 2
      TransformationAddTypeMatrix(103, 9, 2),

      // %104 = OpTypeMatrix %9 3
      TransformationAddTypeMatrix(104, 9, 3),

      // %105 = OpTypeMatrix %9 4
      TransformationAddTypeMatrix(105, 9, 4),

      // %106 = OpTypeMatrix %10 2
      TransformationAddTypeMatrix(106, 10, 2),

      // %107 = OpTypeMatrix %10 3
      TransformationAddTypeMatrix(107, 10, 3),

      // %108 = OpTypeMatrix %10 4
      TransformationAddTypeMatrix(108, 10, 4)};

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
        %100 = OpTypeMatrix %8 2
        %101 = OpTypeMatrix %8 3
        %102 = OpTypeMatrix %8 4
        %103 = OpTypeMatrix %9 2
        %104 = OpTypeMatrix %9 3
        %105 = OpTypeMatrix %9 4
        %106 = OpTypeMatrix %10 2
        %107 = OpTypeMatrix %10 3
        %108 = OpTypeMatrix %10 4
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
