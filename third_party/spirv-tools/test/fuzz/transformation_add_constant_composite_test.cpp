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

#include "source/fuzz/transformation_add_constant_composite.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationAddConstantCompositeTest, BasicTest) {
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
          %7 = OpTypeVector %6 2
          %8 = OpTypeMatrix %7 3
         %11 = OpConstant %6 0
         %12 = OpConstant %6 1
         %14 = OpConstant %6 2
         %15 = OpConstant %6 3
         %17 = OpConstant %6 4
         %18 = OpConstant %6 5
         %21 = OpTypeInt 32 1
         %22 = OpTypeInt 32 0
         %23 = OpConstant %22 3
         %24 = OpTypeArray %21 %23
         %25 = OpTypeBool
         %26 = OpTypeStruct %24 %25
         %29 = OpConstant %21 1
         %30 = OpConstant %21 2
         %31 = OpConstant %21 3
         %33 = OpConstantFalse %25
         %35 = OpTypeVector %6 3
         %38 = OpConstant %6 6
         %39 = OpConstant %6 7
         %40 = OpConstant %6 8
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
  // Too few ids
  ASSERT_FALSE(TransformationAddConstantComposite(103, 8, {100, 101}, false)
                   .IsApplicable(context.get(), transformation_context));
  // Too many ids
  ASSERT_FALSE(TransformationAddConstantComposite(101, 7, {14, 15, 14}, false)
                   .IsApplicable(context.get(), transformation_context));
  // Id already in use
  ASSERT_FALSE(TransformationAddConstantComposite(40, 7, {11, 12}, false)
                   .IsApplicable(context.get(), transformation_context));
  // %39 is not a type
  ASSERT_FALSE(TransformationAddConstantComposite(100, 39, {11, 12}, false)
                   .IsApplicable(context.get(), transformation_context));

  {
    // %100 = OpConstantComposite %7 %11 %12
    TransformationAddConstantComposite transformation(100, 7, {11, 12}, false);
    ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(100));
    ASSERT_EQ(nullptr, context->get_constant_mgr()->FindDeclaredConstant(100));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_EQ(spv::Op::OpConstantComposite,
              context->get_def_use_mgr()->GetDef(100)->opcode());
    ASSERT_EQ(0.0F, context->get_constant_mgr()
                        ->FindDeclaredConstant(100)
                        ->AsVectorConstant()
                        ->GetComponents()[0]
                        ->GetFloat());
    ASSERT_EQ(1.0F, context->get_constant_mgr()
                        ->FindDeclaredConstant(100)
                        ->AsVectorConstant()
                        ->GetComponents()[1]
                        ->GetFloat());
  }

  TransformationAddConstantComposite transformations[] = {
      // %101 = OpConstantComposite %7 %14 %15
      TransformationAddConstantComposite(101, 7, {14, 15}, false),

      // %102 = OpConstantComposite %7 %17 %18
      TransformationAddConstantComposite(102, 7, {17, 18}, false),

      // %103 = OpConstantComposite %8 %100 %101 %102
      TransformationAddConstantComposite(103, 8, {100, 101, 102}, false),

      // %104 = OpConstantComposite %24 %29 %30 %31
      TransformationAddConstantComposite(104, 24, {29, 30, 31}, false),

      // %105 = OpConstantComposite %26 %104 %33
      TransformationAddConstantComposite(105, 26, {104, 33}, false),

      // %106 = OpConstantComposite %35 %38 %39 %40
      TransformationAddConstantComposite(106, 35, {38, 39, 40}, false),

      // Same constants but with an irrelevant fact applied.

      // %107 = OpConstantComposite %7 %11 %12
      TransformationAddConstantComposite(107, 7, {11, 12}, true),

      // %108 = OpConstantComposite %7 %14 %15
      TransformationAddConstantComposite(108, 7, {14, 15}, true),

      // %109 = OpConstantComposite %7 %17 %18
      TransformationAddConstantComposite(109, 7, {17, 18}, true),

      // %110 = OpConstantComposite %8 %100 %101 %102
      TransformationAddConstantComposite(110, 8, {100, 101, 102}, true),

      // %111 = OpConstantComposite %24 %29 %30 %31
      TransformationAddConstantComposite(111, 24, {29, 30, 31}, true),

      // %112 = OpConstantComposite %26 %104 %33
      TransformationAddConstantComposite(112, 26, {104, 33}, true),

      // %113 = OpConstantComposite %35 %38 %39 %40
      TransformationAddConstantComposite(113, 35, {38, 39, 40}, true)};

  for (auto& transformation : transformations) {
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
  }
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  for (uint32_t id = 100; id <= 106; ++id) {
    ASSERT_FALSE(transformation_context.GetFactManager()->IdIsIrrelevant(id));
  }

  for (uint32_t id = 107; id <= 113; ++id) {
    ASSERT_TRUE(transformation_context.GetFactManager()->IdIsIrrelevant(id));
  }

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
          %7 = OpTypeVector %6 2
          %8 = OpTypeMatrix %7 3
         %11 = OpConstant %6 0
         %12 = OpConstant %6 1
         %14 = OpConstant %6 2
         %15 = OpConstant %6 3
         %17 = OpConstant %6 4
         %18 = OpConstant %6 5
         %21 = OpTypeInt 32 1
         %22 = OpTypeInt 32 0
         %23 = OpConstant %22 3
         %24 = OpTypeArray %21 %23
         %25 = OpTypeBool
         %26 = OpTypeStruct %24 %25
         %29 = OpConstant %21 1
         %30 = OpConstant %21 2
         %31 = OpConstant %21 3
         %33 = OpConstantFalse %25
         %35 = OpTypeVector %6 3
         %38 = OpConstant %6 6
         %39 = OpConstant %6 7
         %40 = OpConstant %6 8
        %100 = OpConstantComposite %7 %11 %12
        %101 = OpConstantComposite %7 %14 %15
        %102 = OpConstantComposite %7 %17 %18
        %103 = OpConstantComposite %8 %100 %101 %102
        %104 = OpConstantComposite %24 %29 %30 %31
        %105 = OpConstantComposite %26 %104 %33
        %106 = OpConstantComposite %35 %38 %39 %40
        %107 = OpConstantComposite %7 %11 %12
        %108 = OpConstantComposite %7 %14 %15
        %109 = OpConstantComposite %7 %17 %18
        %110 = OpConstantComposite %8 %100 %101 %102
        %111 = OpConstantComposite %24 %29 %30 %31
        %112 = OpConstantComposite %26 %104 %33
        %113 = OpConstantComposite %35 %38 %39 %40
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationAddConstantCompositeTest, DisallowBufferBlockDecoration) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 320
               OpName %4 "main"
               OpName %7 "buf"
               OpMemberName %7 0 "a"
               OpMemberName %7 1 "b"
               OpName %9 ""
               OpMemberDecorate %7 0 Offset 0
               OpMemberDecorate %7 1 Offset 4
               OpDecorate %7 BufferBlock
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %10 = OpConstant %6 42
          %7 = OpTypeStruct %6 %6
          %8 = OpTypePointer Uniform %7
          %9 = OpVariable %8 Uniform
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_0;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_FALSE(TransformationAddConstantComposite(100, 7, {10, 10}, false)
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationAddConstantCompositeTest, DisallowBlockDecoration) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main" %9
               OpExecutionMode %4 LocalSize 1 1 1
               OpSource ESSL 320
               OpName %4 "main"
               OpName %7 "buf"
               OpMemberName %7 0 "a"
               OpMemberName %7 1 "b"
               OpName %9 ""
               OpMemberDecorate %7 0 Offset 0
               OpMemberDecorate %7 1 Offset 4
               OpDecorate %7 Block
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %10 = OpConstant %6 42
          %7 = OpTypeStruct %6 %6
          %8 = OpTypePointer StorageBuffer %7
          %9 = OpVariable %8 StorageBuffer
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  ASSERT_FALSE(TransformationAddConstantComposite(100, 7, {10, 10}, false)
                   .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
