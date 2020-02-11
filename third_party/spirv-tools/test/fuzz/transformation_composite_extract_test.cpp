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

#include "source/fuzz/transformation_composite_extract.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationCompositeExtractTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpName %17 "FunnyPoint"
               OpMemberName %17 0 "x"
               OpMemberName %17 1 "y"
               OpMemberName %17 2 "z"
               OpName %19 "p"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %12 = OpTypeBool
         %16 = OpTypeFloat 32
         %17 = OpTypeStruct %16 %16 %6
         %81 = OpTypeStruct %17 %16
         %18 = OpTypePointer Function %17
         %20 = OpConstant %6 0
         %23 = OpTypePointer Function %16
         %26 = OpConstant %6 1
         %30 = OpConstant %6 2
         %80 = OpUndef %16
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %19 = OpVariable %18 Function
          %9 = OpLoad %6 %8
         %11 = OpLoad %6 %10
        %100 = OpCompositeConstruct %17 %80 %80 %26
        %104 = OpCompositeConstruct %81 %100 %80
         %13 = OpIEqual %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %25
         %14 = OpLabel
         %21 = OpLoad %6 %8
         %22 = OpConvertSToF %16 %21
        %101 = OpCompositeConstruct %17 %22 %80 %30
         %24 = OpAccessChain %23 %19 %20
               OpStore %24 %22
               OpBranch %15
         %25 = OpLabel
         %27 = OpLoad %6 %10
         %28 = OpConvertSToF %16 %27
        %102 = OpCompositeConstruct %17 %80 %28 %27
         %29 = OpAccessChain %23 %19 %26
               OpStore %29 %28
               OpBranch %15
         %15 = OpLabel
         %31 = OpAccessChain %23 %19 %20
         %32 = OpLoad %16 %31
         %33 = OpAccessChain %23 %19 %26
         %34 = OpLoad %16 %33
        %103 = OpCompositeConstruct %17 %34 %32 %9
         %35 = OpFAdd %16 %32 %34
         %36 = OpConvertFToS %6 %35
         %37 = OpAccessChain %7 %19 %30
               OpStore %37 %36
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // Instruction does not exist.
  ASSERT_FALSE(TransformationCompositeExtract(
                   MakeInstructionDescriptor(36, SpvOpIAdd, 0), 200, 101, {0})
                   .IsApplicable(context.get(), fact_manager));

  // Id for composite is not a composite.
  ASSERT_FALSE(TransformationCompositeExtract(
                   MakeInstructionDescriptor(36, SpvOpIAdd, 0), 200, 27, {})
                   .IsApplicable(context.get(), fact_manager));

  // Composite does not dominate instruction being inserted before.
  ASSERT_FALSE(
      TransformationCompositeExtract(
          MakeInstructionDescriptor(37, SpvOpAccessChain, 0), 200, 101, {0})
          .IsApplicable(context.get(), fact_manager));

  // Too many indices for extraction from struct composite.
  ASSERT_FALSE(
      TransformationCompositeExtract(
          MakeInstructionDescriptor(24, SpvOpAccessChain, 0), 200, 101, {0, 0})
          .IsApplicable(context.get(), fact_manager));

  // Too many indices for extraction from struct composite.
  ASSERT_FALSE(
      TransformationCompositeExtract(
          MakeInstructionDescriptor(13, SpvOpIEqual, 0), 200, 104, {0, 0, 0})
          .IsApplicable(context.get(), fact_manager));

  // Out of bounds index for extraction from struct composite.
  ASSERT_FALSE(
      TransformationCompositeExtract(
          MakeInstructionDescriptor(13, SpvOpIEqual, 0), 200, 104, {0, 3})
          .IsApplicable(context.get(), fact_manager));

  // Result id already used.
  ASSERT_FALSE(TransformationCompositeExtract(
                   MakeInstructionDescriptor(35, SpvOpFAdd, 0), 80, 103, {0})
                   .IsApplicable(context.get(), fact_manager));

  TransformationCompositeExtract transformation_1(
      MakeInstructionDescriptor(36, SpvOpConvertFToS, 0), 201, 100, {2});
  ASSERT_TRUE(transformation_1.IsApplicable(context.get(), fact_manager));
  transformation_1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  TransformationCompositeExtract transformation_2(
      MakeInstructionDescriptor(37, SpvOpAccessChain, 0), 202, 104, {0, 2});
  ASSERT_TRUE(transformation_2.IsApplicable(context.get(), fact_manager));
  transformation_2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  TransformationCompositeExtract transformation_3(
      MakeInstructionDescriptor(29, SpvOpAccessChain, 0), 203, 104, {0});
  ASSERT_TRUE(transformation_3.IsApplicable(context.get(), fact_manager));
  transformation_3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  TransformationCompositeExtract transformation_4(
      MakeInstructionDescriptor(24, SpvOpStore, 0), 204, 101, {0});
  ASSERT_TRUE(transformation_4.IsApplicable(context.get(), fact_manager));
  transformation_4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  TransformationCompositeExtract transformation_5(
      MakeInstructionDescriptor(29, SpvOpBranch, 0), 205, 102, {2});
  ASSERT_TRUE(transformation_5.IsApplicable(context.get(), fact_manager));
  transformation_5.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  TransformationCompositeExtract transformation_6(
      MakeInstructionDescriptor(37, SpvOpReturn, 0), 206, 103, {1});
  ASSERT_TRUE(transformation_6.IsApplicable(context.get(), fact_manager));
  transformation_6.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(201, {}),
                                        MakeDataDescriptor(100, {2}),
                                        context.get()));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(202, {}),
                                        MakeDataDescriptor(104, {0, 2}),
                                        context.get()));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(203, {}),
                                        MakeDataDescriptor(104, {0}),
                                        context.get()));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(204, {}),
                                        MakeDataDescriptor(101, {0}),
                                        context.get()));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(205, {}),
                                        MakeDataDescriptor(102, {2}),
                                        context.get()));
  ASSERT_TRUE(fact_manager.IsSynonymous(MakeDataDescriptor(206, {}),
                                        MakeDataDescriptor(103, {1}),
                                        context.get()));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "a"
               OpName %10 "b"
               OpName %17 "FunnyPoint"
               OpMemberName %17 0 "x"
               OpMemberName %17 1 "y"
               OpMemberName %17 2 "z"
               OpName %19 "p"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
         %12 = OpTypeBool
         %16 = OpTypeFloat 32
         %17 = OpTypeStruct %16 %16 %6
         %81 = OpTypeStruct %17 %16
         %18 = OpTypePointer Function %17
         %20 = OpConstant %6 0
         %23 = OpTypePointer Function %16
         %26 = OpConstant %6 1
         %30 = OpConstant %6 2
         %80 = OpUndef %16
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
         %19 = OpVariable %18 Function
          %9 = OpLoad %6 %8
         %11 = OpLoad %6 %10
        %100 = OpCompositeConstruct %17 %80 %80 %26
        %104 = OpCompositeConstruct %81 %100 %80
         %13 = OpIEqual %12 %9 %11
               OpSelectionMerge %15 None
               OpBranchConditional %13 %14 %25
         %14 = OpLabel
         %21 = OpLoad %6 %8
         %22 = OpConvertSToF %16 %21
        %101 = OpCompositeConstruct %17 %22 %80 %30
         %24 = OpAccessChain %23 %19 %20
        %204 = OpCompositeExtract %16 %101 0
               OpStore %24 %22
               OpBranch %15
         %25 = OpLabel
         %27 = OpLoad %6 %10
         %28 = OpConvertSToF %16 %27
        %102 = OpCompositeConstruct %17 %80 %28 %27
        %203 = OpCompositeExtract %17 %104 0
         %29 = OpAccessChain %23 %19 %26
               OpStore %29 %28
        %205 = OpCompositeExtract %6 %102 2
               OpBranch %15
         %15 = OpLabel
         %31 = OpAccessChain %23 %19 %20
         %32 = OpLoad %16 %31
         %33 = OpAccessChain %23 %19 %26
         %34 = OpLoad %16 %33
        %103 = OpCompositeConstruct %17 %34 %32 %9
         %35 = OpFAdd %16 %32 %34
        %201 = OpCompositeExtract %6 %100 2
         %36 = OpConvertFToS %6 %35
        %202 = OpCompositeExtract %6 %104 0 2
         %37 = OpAccessChain %7 %19 %30
               OpStore %37 %36
        %206 = OpCompositeExtract %16 %103 1
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationCompositeExtractTest, IllegalInsertionPoints) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %51 %27
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %25 "buf"
               OpMemberName %25 0 "value"
               OpName %27 ""
               OpName %51 "color"
               OpMemberDecorate %25 0 Offset 0
               OpDecorate %25 Block
               OpDecorate %27 DescriptorSet 0
               OpDecorate %27 Binding 0
               OpDecorate %51 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
         %10 = OpConstant %6 0.300000012
         %11 = OpConstant %6 0.400000006
         %12 = OpConstant %6 0.5
         %13 = OpConstant %6 1
         %14 = OpConstantComposite %7 %10 %11 %12 %13
         %15 = OpTypeInt 32 1
         %18 = OpConstant %15 0
         %25 = OpTypeStruct %6
         %26 = OpTypePointer Uniform %25
         %27 = OpVariable %26 Uniform
         %28 = OpTypePointer Uniform %6
         %32 = OpTypeBool
        %103 = OpConstantTrue %32
         %34 = OpConstant %6 0.100000001
         %48 = OpConstant %15 1
         %50 = OpTypePointer Output %7
         %51 = OpVariable %50 Output
        %100 = OpTypePointer Function %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %101 = OpVariable %100 Function
        %102 = OpVariable %100 Function
               OpBranch %19
         %19 = OpLabel
         %60 = OpPhi %7 %14 %5 %58 %20
         %59 = OpPhi %15 %18 %5 %49 %20
         %29 = OpAccessChain %28 %27 %18
         %30 = OpLoad %6 %29
         %31 = OpConvertFToS %15 %30
         %33 = OpSLessThan %32 %59 %31
               OpLoopMerge %21 %20 None
               OpBranchConditional %33 %20 %21
         %20 = OpLabel
         %39 = OpCompositeExtract %6 %60 0
         %40 = OpFAdd %6 %39 %34
         %55 = OpCompositeInsert %7 %40 %60 0
         %44 = OpCompositeExtract %6 %60 1
         %45 = OpFSub %6 %44 %34
         %58 = OpCompositeInsert %7 %45 %55 1
         %49 = OpIAdd %15 %59 %48
               OpBranch %19
         %21 = OpLabel
               OpStore %51 %60
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
               OpBranch %105
        %105 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // Cannot insert before the OpVariables of a function.
  ASSERT_FALSE(
      TransformationCompositeExtract(
          MakeInstructionDescriptor(101, SpvOpVariable, 0), 200, 14, {0})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      TransformationCompositeExtract(
          MakeInstructionDescriptor(101, SpvOpVariable, 1), 200, 14, {1})
          .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(
      TransformationCompositeExtract(
          MakeInstructionDescriptor(102, SpvOpVariable, 0), 200, 14, {1})
          .IsApplicable(context.get(), fact_manager));
  // OK to insert right after the OpVariables.
  ASSERT_FALSE(TransformationCompositeExtract(
                   MakeInstructionDescriptor(102, SpvOpBranch, 1), 200, 14, {1})
                   .IsApplicable(context.get(), fact_manager));

  // Cannot insert before the OpPhis of a block.
  ASSERT_FALSE(TransformationCompositeExtract(
                   MakeInstructionDescriptor(60, SpvOpPhi, 0), 200, 14, {2})
                   .IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(TransformationCompositeExtract(
                   MakeInstructionDescriptor(59, SpvOpPhi, 0), 200, 14, {3})
                   .IsApplicable(context.get(), fact_manager));
  // OK to insert after the OpPhis.
  ASSERT_TRUE(
      TransformationCompositeExtract(
          MakeInstructionDescriptor(59, SpvOpAccessChain, 0), 200, 14, {3})
          .IsApplicable(context.get(), fact_manager));

  // Cannot insert before OpLoopMerge
  ASSERT_FALSE(TransformationCompositeExtract(
                   MakeInstructionDescriptor(33, SpvOpBranchConditional, 0),
                   200, 14, {3})
                   .IsApplicable(context.get(), fact_manager));

  // Cannot insert before OpSelectionMerge
  ASSERT_FALSE(TransformationCompositeExtract(
                   MakeInstructionDescriptor(21, SpvOpBranchConditional, 0),
                   200, 14, {2})
                   .IsApplicable(context.get(), fact_manager));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
