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

#include "source/fuzz/transformation_vector_shuffle.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationVectorShuffleTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypeVector %6 2
         %10 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %12 = OpConstantComposite %7 %10 %11
        %112 = OpUndef %7
         %13 = OpTypeVector %6 3
         %16 = OpConstantComposite %13 %10 %11 %10
         %17 = OpTypeVector %6 4
         %20 = OpConstantComposite %17 %10 %11 %10 %11
         %21 = OpTypeInt 32 1
         %22 = OpTypeVector %21 2
         %25 = OpConstant %21 1
         %26 = OpConstant %21 0
         %27 = OpConstantComposite %22 %25 %26
         %28 = OpTypeVector %21 3
         %31 = OpConstantComposite %28 %25 %26 %25
         %32 = OpTypeVector %21 4
         %33 = OpTypePointer Function %32
         %35 = OpConstantComposite %32 %25 %26 %25 %26
         %36 = OpTypeInt 32 0
         %37 = OpTypeVector %36 2
         %40 = OpConstant %36 1
         %41 = OpConstant %36 0
         %42 = OpConstantComposite %37 %40 %41
         %43 = OpTypeVector %36 3
         %46 = OpConstantComposite %43 %40 %41 %40
         %47 = OpTypeVector %36 4
         %50 = OpConstantComposite %47 %40 %41 %40 %41
         %51 = OpTypeFloat 32
         %55 = OpConstant %51 1
         %56 = OpConstant %51 0
         %58 = OpTypeVector %51 3
         %61 = OpConstantComposite %58 %55 %56 %55
         %62 = OpTypeVector %51 4
         %65 = OpConstantComposite %62 %55 %56 %55 %56
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %100 None
               OpBranchConditional %10 %101 %102
        %101 = OpLabel
        %103 = OpCompositeConstruct %62 %55 %55 %55 %56
               OpBranch %100
        %102 = OpLabel
               OpBranch %100
        %100 = OpLabel
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
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(10, {}), MakeDataDescriptor(12, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(11, {}), MakeDataDescriptor(12, {1}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(10, {}), MakeDataDescriptor(16, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(11, {}), MakeDataDescriptor(16, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(10, {}), MakeDataDescriptor(16, {2}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(10, {}), MakeDataDescriptor(20, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(11, {}), MakeDataDescriptor(20, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(10, {}), MakeDataDescriptor(20, {2}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(11, {}), MakeDataDescriptor(20, {3}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(25, {}), MakeDataDescriptor(27, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(26, {}), MakeDataDescriptor(27, {1}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(25, {}), MakeDataDescriptor(31, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(26, {}), MakeDataDescriptor(31, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(25, {}), MakeDataDescriptor(31, {2}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(25, {}), MakeDataDescriptor(35, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(26, {}), MakeDataDescriptor(35, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(25, {}), MakeDataDescriptor(35, {2}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(26, {}), MakeDataDescriptor(35, {3}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(40, {}), MakeDataDescriptor(42, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(41, {}), MakeDataDescriptor(42, {1}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(40, {}), MakeDataDescriptor(46, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(41, {}), MakeDataDescriptor(46, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(40, {}), MakeDataDescriptor(46, {2}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(40, {}), MakeDataDescriptor(50, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(41, {}), MakeDataDescriptor(50, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(40, {}), MakeDataDescriptor(50, {2}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(41, {}), MakeDataDescriptor(50, {3}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(55, {}), MakeDataDescriptor(61, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(56, {}), MakeDataDescriptor(61, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(55, {}), MakeDataDescriptor(61, {2}));

  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(55, {}), MakeDataDescriptor(65, {0}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(56, {}), MakeDataDescriptor(65, {1}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(55, {}), MakeDataDescriptor(65, {2}));
  transformation_context.GetFactManager()->AddFactDataSynonym(
      MakeDataDescriptor(56, {}), MakeDataDescriptor(65, {3}));

  // %103 does not dominate the return instruction.
  ASSERT_FALSE(TransformationVectorShuffle(
                   MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 103, 65,
                   {3, 5, 7})
                   .IsApplicable(context.get(), transformation_context));

  // Illegal to shuffle a bvec2 and a vec3
  ASSERT_FALSE(TransformationVectorShuffle(
                   MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 112, 61,
                   {0, 2, 4})
                   .IsApplicable(context.get(), transformation_context));

  // Illegal to shuffle an ivec2 and a uvec4
  ASSERT_FALSE(TransformationVectorShuffle(
                   MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 27, 50,
                   {1, 3, 5})
                   .IsApplicable(context.get(), transformation_context));

  // Vector 1 does not exist
  ASSERT_FALSE(TransformationVectorShuffle(
                   MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 300, 50,
                   {1, 3, 5})
                   .IsApplicable(context.get(), transformation_context));

  // Vector 2 does not exist
  ASSERT_FALSE(TransformationVectorShuffle(
                   MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 27, 300,
                   {1, 3, 5})
                   .IsApplicable(context.get(), transformation_context));

  // Index out of range
  ASSERT_FALSE(
      TransformationVectorShuffle(
          MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 12, 112, {0, 20})
          .IsApplicable(context.get(), transformation_context));

  // Too many indices
  ASSERT_FALSE(TransformationVectorShuffle(
                   MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 12, 112,
                   {0, 1, 0, 1, 0, 1, 0, 1})
                   .IsApplicable(context.get(), transformation_context));

  // Too few indices
  ASSERT_FALSE(
      TransformationVectorShuffle(
          MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 12, 112, {})
          .IsApplicable(context.get(), transformation_context));

  // Too few indices again
  ASSERT_FALSE(
      TransformationVectorShuffle(
          MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 12, 112, {0})
          .IsApplicable(context.get(), transformation_context));

  // Indices define unknown type: we do not have vec2
  ASSERT_FALSE(
      TransformationVectorShuffle(
          MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 65, 65, {0, 1})
          .IsApplicable(context.get(), transformation_context));

  // The instruction to insert before does not exist
  ASSERT_FALSE(TransformationVectorShuffle(
                   MakeInstructionDescriptor(100, SpvOpCompositeConstruct, 1),
                   201, 20, 12, {0xFFFFFFFF, 3, 5})
                   .IsApplicable(context.get(), transformation_context));

  // The 'fresh' id is already in use
  ASSERT_FALSE(
      TransformationVectorShuffle(
          MakeInstructionDescriptor(100, SpvOpReturn, 0), 12, 12, 112, {})
          .IsApplicable(context.get(), transformation_context));

  protobufs::DataDescriptor temp_dd;

  TransformationVectorShuffle transformation1(
      MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 12, 112, {1, 0});
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  temp_dd = MakeDataDescriptor(200, {0});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(11, {}), temp_dd));
  temp_dd = MakeDataDescriptor(200, {1});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(10, {}), temp_dd));

  TransformationVectorShuffle transformation2(
      MakeInstructionDescriptor(100, SpvOpReturn, 0), 201, 20, 12,
      {0xFFFFFFFF, 3, 5});
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  temp_dd = MakeDataDescriptor(201, {1});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(11, {}), temp_dd));
  temp_dd = MakeDataDescriptor(201, {2});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(11, {}), temp_dd));

  TransformationVectorShuffle transformation3(
      MakeInstructionDescriptor(100, SpvOpReturn, 0), 202, 27, 35, {5, 4, 1});
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);
  temp_dd = MakeDataDescriptor(202, {0});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(26, {}), temp_dd));
  temp_dd = MakeDataDescriptor(202, {1});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(25, {}), temp_dd));
  temp_dd = MakeDataDescriptor(202, {2});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(26, {}), temp_dd));

  TransformationVectorShuffle transformation4(
      MakeInstructionDescriptor(100, SpvOpReturn, 0), 203, 42, 46, {0, 1});
  ASSERT_TRUE(
      transformation4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation4, context.get(),
                        &transformation_context);
  temp_dd = MakeDataDescriptor(203, {0});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {}), temp_dd));
  temp_dd = MakeDataDescriptor(203, {1});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(41, {}), temp_dd));

  TransformationVectorShuffle transformation5(
      MakeInstructionDescriptor(100, SpvOpReturn, 0), 204, 42, 46, {2, 3, 4});
  ASSERT_TRUE(
      transformation5.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation5, context.get(),
                        &transformation_context);
  temp_dd = MakeDataDescriptor(204, {0});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {}), temp_dd));
  temp_dd = MakeDataDescriptor(204, {1});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(41, {}), temp_dd));
  temp_dd = MakeDataDescriptor(204, {2});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {}), temp_dd));

  TransformationVectorShuffle transformation6(
      MakeInstructionDescriptor(100, SpvOpReturn, 0), 205, 42, 42,
      {0, 1, 2, 3});
  ASSERT_TRUE(
      transformation6.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation6, context.get(),
                        &transformation_context);
  temp_dd = MakeDataDescriptor(205, {0});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {}), temp_dd));
  temp_dd = MakeDataDescriptor(205, {1});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(41, {}), temp_dd));
  temp_dd = MakeDataDescriptor(205, {2});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {}), temp_dd));
  temp_dd = MakeDataDescriptor(205, {3});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(41, {}), temp_dd));

  // swizzle vec4 from vec4 and vec4 using some undefs
  TransformationVectorShuffle transformation7(
      MakeInstructionDescriptor(100, SpvOpReturn, 0), 206, 65, 65,
      {0xFFFFFFFF, 3, 6, 0xFFFFFFFF});
  ASSERT_TRUE(
      transformation7.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation7, context.get(),
                        &transformation_context);
  temp_dd = MakeDataDescriptor(206, {1});
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(56, {}), temp_dd));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypeVector %6 2
         %10 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %12 = OpConstantComposite %7 %10 %11
        %112 = OpUndef %7
         %13 = OpTypeVector %6 3
         %16 = OpConstantComposite %13 %10 %11 %10
         %17 = OpTypeVector %6 4
         %20 = OpConstantComposite %17 %10 %11 %10 %11
         %21 = OpTypeInt 32 1
         %22 = OpTypeVector %21 2
         %25 = OpConstant %21 1
         %26 = OpConstant %21 0
         %27 = OpConstantComposite %22 %25 %26
         %28 = OpTypeVector %21 3
         %31 = OpConstantComposite %28 %25 %26 %25
         %32 = OpTypeVector %21 4
         %33 = OpTypePointer Function %32
         %35 = OpConstantComposite %32 %25 %26 %25 %26
         %36 = OpTypeInt 32 0
         %37 = OpTypeVector %36 2
         %40 = OpConstant %36 1
         %41 = OpConstant %36 0
         %42 = OpConstantComposite %37 %40 %41
         %43 = OpTypeVector %36 3
         %46 = OpConstantComposite %43 %40 %41 %40
         %47 = OpTypeVector %36 4
         %50 = OpConstantComposite %47 %40 %41 %40 %41
         %51 = OpTypeFloat 32
         %55 = OpConstant %51 1
         %56 = OpConstant %51 0
         %58 = OpTypeVector %51 3
         %61 = OpConstantComposite %58 %55 %56 %55
         %62 = OpTypeVector %51 4
         %65 = OpConstantComposite %62 %55 %56 %55 %56
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %100 None
               OpBranchConditional %10 %101 %102
        %101 = OpLabel
        %103 = OpCompositeConstruct %62 %55 %55 %55 %56
               OpBranch %100
        %102 = OpLabel
               OpBranch %100
        %100 = OpLabel
        %200 = OpVectorShuffle %7 %12 %112 1 0
        %201 = OpVectorShuffle %13 %20 %12 0xFFFFFFFF 3 5
        %202 = OpVectorShuffle %28 %27 %35 5 4 1
        %203 = OpVectorShuffle %37 %42 %46 0 1
        %204 = OpVectorShuffle %43 %42 %46 2 3 4
        %205 = OpVectorShuffle %47 %42 %42 0 1 2 3
        %206 = OpVectorShuffle %62 %65 %65 0xFFFFFFFF 3 6 0xFFFFFFFF
               OpReturn
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationVectorShuffleTest, IllegalInsertionPoints) {
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
        %150 = OpTypeVector %6 2
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
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Cannot insert before the OpVariables of a function.
  ASSERT_FALSE(
      TransformationVectorShuffle(
          MakeInstructionDescriptor(101, SpvOpVariable, 0), 200, 14, 14, {0, 1})
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationVectorShuffle(
          MakeInstructionDescriptor(101, SpvOpVariable, 1), 200, 14, 14, {1, 2})
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationVectorShuffle(
          MakeInstructionDescriptor(102, SpvOpVariable, 0), 200, 14, 14, {1, 2})
          .IsApplicable(context.get(), transformation_context));
  // OK to insert right after the OpVariables.
  ASSERT_FALSE(
      TransformationVectorShuffle(
          MakeInstructionDescriptor(102, SpvOpBranch, 1), 200, 14, 14, {1, 1})
          .IsApplicable(context.get(), transformation_context));

  // Cannot insert before the OpPhis of a block.
  ASSERT_FALSE(
      TransformationVectorShuffle(MakeInstructionDescriptor(60, SpvOpPhi, 0),
                                  200, 14, 14, {2, 0})
          .IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      TransformationVectorShuffle(MakeInstructionDescriptor(59, SpvOpPhi, 0),
                                  200, 14, 14, {3, 0})
          .IsApplicable(context.get(), transformation_context));
  // OK to insert after the OpPhis.
  ASSERT_TRUE(TransformationVectorShuffle(
                  MakeInstructionDescriptor(59, SpvOpAccessChain, 0), 200, 14,
                  14, {3, 4})
                  .IsApplicable(context.get(), transformation_context));

  // Cannot insert before OpLoopMerge
  ASSERT_FALSE(TransformationVectorShuffle(
                   MakeInstructionDescriptor(33, SpvOpBranchConditional, 0),
                   200, 14, 14, {3})
                   .IsApplicable(context.get(), transformation_context));

  // Cannot insert before OpSelectionMerge
  ASSERT_FALSE(TransformationVectorShuffle(
                   MakeInstructionDescriptor(21, SpvOpBranchConditional, 0),
                   200, 14, 14, {2})
                   .IsApplicable(context.get(), transformation_context));
}

TEST(TransformationVectorShuffleTest, HandlesIrrelevantIds1) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypeVector %6 2
         %10 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %12 = OpConstantComposite %7 %10 %11
        %112 = OpConstantComposite %7 %11 %10
         %13 = OpTypeVector %6 3
         %16 = OpConstantComposite %13 %10 %11 %10
         %17 = OpTypeVector %6 4
         %20 = OpConstantComposite %17 %10 %11 %10 %11
         %21 = OpTypeInt 32 1
         %22 = OpTypeVector %21 2
         %25 = OpConstant %21 1
         %26 = OpConstant %21 0
         %27 = OpConstantComposite %22 %25 %26
         %28 = OpTypeVector %21 3
         %31 = OpConstantComposite %28 %25 %26 %25
         %32 = OpTypeVector %21 4
         %33 = OpTypePointer Function %32
         %35 = OpConstantComposite %32 %25 %26 %25 %26
         %36 = OpTypeInt 32 0
         %37 = OpTypeVector %36 2
         %40 = OpConstant %36 1
         %41 = OpConstant %36 0
         %42 = OpConstantComposite %37 %40 %41
         %43 = OpTypeVector %36 3
         %46 = OpConstantComposite %43 %40 %41 %40
         %47 = OpTypeVector %36 4
         %50 = OpConstantComposite %47 %40 %41 %40 %41
         %51 = OpTypeFloat 32
         %55 = OpConstant %51 1
         %56 = OpConstant %51 0
         %58 = OpTypeVector %51 3
         %61 = OpConstantComposite %58 %55 %56 %55
         %62 = OpTypeVector %51 4
         %65 = OpConstantComposite %62 %55 %56 %55 %56
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %100 None
               OpBranchConditional %10 %101 %102
        %101 = OpLabel
        %103 = OpCompositeConstruct %62 %55 %55 %55 %56
               OpBranch %100
        %102 = OpLabel
               OpBranch %100
        %100 = OpLabel
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
  TransformationVectorShuffle transformation(
      MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 12, 112, {2, 0});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(12, {0}), MakeDataDescriptor(200, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(112, {0}), MakeDataDescriptor(200, {0})));
}

TEST(TransformationVectorShuffleTest, HandlesIrrelevantIds2) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeBool
          %7 = OpTypeVector %6 2
         %10 = OpConstantTrue %6
         %11 = OpConstantFalse %6
         %12 = OpConstantComposite %7 %10 %11
        %112 = OpConstantComposite %7 %11 %10
         %13 = OpTypeVector %6 3
         %16 = OpConstantComposite %13 %10 %11 %10
         %17 = OpTypeVector %6 4
         %20 = OpConstantComposite %17 %10 %11 %10 %11
         %21 = OpTypeInt 32 1
         %22 = OpTypeVector %21 2
         %25 = OpConstant %21 1
         %26 = OpConstant %21 0
         %27 = OpConstantComposite %22 %25 %26
         %28 = OpTypeVector %21 3
         %31 = OpConstantComposite %28 %25 %26 %25
         %32 = OpTypeVector %21 4
         %33 = OpTypePointer Function %32
         %35 = OpConstantComposite %32 %25 %26 %25 %26
         %36 = OpTypeInt 32 0
         %37 = OpTypeVector %36 2
         %40 = OpConstant %36 1
         %41 = OpConstant %36 0
         %42 = OpConstantComposite %37 %40 %41
         %43 = OpTypeVector %36 3
         %46 = OpConstantComposite %43 %40 %41 %40
         %47 = OpTypeVector %36 4
         %50 = OpConstantComposite %47 %40 %41 %40 %41
         %51 = OpTypeFloat 32
         %55 = OpConstant %51 1
         %56 = OpConstant %51 0
         %58 = OpTypeVector %51 3
         %61 = OpConstantComposite %58 %55 %56 %55
         %62 = OpTypeVector %51 4
         %65 = OpConstantComposite %62 %55 %56 %55 %56
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %100 None
               OpBranchConditional %10 %101 %102
        %101 = OpLabel
        %103 = OpCompositeConstruct %62 %55 %55 %55 %56
               OpBranch %100
        %102 = OpLabel
               OpBranch %100
        %100 = OpLabel
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
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(112);
  TransformationVectorShuffle transformation(
      MakeInstructionDescriptor(100, SpvOpReturn, 0), 200, 12, 112, {2, 0});
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  // Because %12 is not irrelevant, we get a synonym between it and %200[1].
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(12, {0}), MakeDataDescriptor(200, {1})));
  // Because %112 is irrelevant, we do not get a synonym between it and %200[0].
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(112, {0}), MakeDataDescriptor(200, {0})));
}

TEST(TransformationVectorShuffleTest, HandlesIrrelevantIds3) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeVector %6 2
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 0
         %11 = OpConstant %6 1
         %12 = OpConstantComposite %7 %10 %11
         %40 = OpConstantComposite %7 %10 %11
         %13 = OpTypeBool
         %14 = OpConstantFalse %13
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
               OpStore %9 %12
               OpSelectionMerge %16 None
               OpBranchConditional %14 %15 %16
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
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
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(40);
  transformation_context.GetFactManager()->AddFactBlockIsDead(15);

  TransformationVectorShuffle transformation1(
      MakeInstructionDescriptor(15, SpvOpBranch, 0), 200, 12, 12, {0, 3});
  ASSERT_TRUE(
      transformation1.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation1, context.get(),
                        &transformation_context);
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(200, {0}), MakeDataDescriptor(12, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(200, {1}), MakeDataDescriptor(12, {1})));

  TransformationVectorShuffle transformation2(
      MakeInstructionDescriptor(16, SpvOpReturn, 0), 201, 12, 40, {0, 1});
  ASSERT_TRUE(
      transformation2.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(201, {0}), MakeDataDescriptor(12, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(201, {1}), MakeDataDescriptor(12, {1})));

  TransformationVectorShuffle transformation3(
      MakeInstructionDescriptor(16, SpvOpReturn, 0), 202, 40, 12, {2, 3});
  ASSERT_TRUE(
      transformation3.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(202, {0}), MakeDataDescriptor(12, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(202, {1}), MakeDataDescriptor(12, {1})));

  TransformationVectorShuffle transformation4(
      MakeInstructionDescriptor(16, SpvOpReturn, 0), 203, 40, 12, {0, 3});
  ASSERT_TRUE(
      transformation4.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation4, context.get(),
                        &transformation_context);
  // Because %40 is irrelevant we do not get a synonym between it and %203[0].
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(203, {0}), MakeDataDescriptor(40, {0})));
  // Because %12 is *not* irrelevant we do get a synonym between it and %203[1].
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(203, {1}), MakeDataDescriptor(12, {1})));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
