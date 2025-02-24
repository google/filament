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

#include "source/fuzz/transformation_composite_construct.h"

#include "gtest/gtest.h"
#include "source/fuzz/data_descriptor.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationCompositeConstructTest, ConstructArrays) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %11 "floats"
               OpName %22 "x"
               OpName %39 "vecs"
               OpName %49 "bools"
               OpName %60 "many_uvec3s"
               OpDecorate %60 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 2
          %9 = OpTypeArray %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpTypeInt 32 1
         %13 = OpConstant %12 0
         %14 = OpConstant %6 1
         %15 = OpTypePointer Function %6
         %17 = OpConstant %12 1
         %18 = OpConstant %6 2
         %20 = OpTypeVector %6 2
         %21 = OpTypePointer Function %20
         %32 = OpTypeBool
         %36 = OpConstant %7 3
         %37 = OpTypeArray %20 %36
         %38 = OpTypePointer Private %37
         %39 = OpVariable %38 Private
         %40 = OpConstant %6 3
         %41 = OpConstantComposite %20 %40 %40
         %42 = OpTypePointer Private %20
         %44 = OpConstant %12 2
         %47 = OpTypeArray %32 %36
         %48 = OpTypePointer Function %47
         %50 = OpConstantTrue %32
         %51 = OpTypePointer Function %32
         %56 = OpTypeVector %7 3
         %57 = OpTypeArray %56 %8
         %58 = OpTypeArray %57 %8
         %59 = OpTypePointer Function %58
         %61 = OpConstant %7 4
         %62 = OpConstantComposite %56 %61 %61 %61
         %63 = OpTypePointer Function %56
         %65 = OpConstant %7 5
         %66 = OpConstantComposite %56 %65 %65 %65
         %67 = OpConstant %7 6
         %68 = OpConstantComposite %56 %67 %67 %67
         %69 = OpConstantComposite %57 %66 %68
        %100 = OpUndef %57
         %70 = OpTypePointer Function %57
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %22 = OpVariable %21 Function
         %49 = OpVariable %48 Function
         %60 = OpVariable %59 Function
         %16 = OpAccessChain %15 %11 %13
               OpStore %16 %14
         %19 = OpAccessChain %15 %11 %17
               OpStore %19 %18
         %23 = OpAccessChain %15 %11 %13
         %24 = OpLoad %6 %23
         %25 = OpAccessChain %15 %11 %17
         %26 = OpLoad %6 %25
         %27 = OpCompositeConstruct %20 %24 %26
               OpStore %22 %27
         %28 = OpAccessChain %15 %11 %13
         %29 = OpLoad %6 %28
         %30 = OpAccessChain %15 %11 %17
         %31 = OpLoad %6 %30
         %33 = OpFOrdGreaterThan %32 %29 %31
               OpSelectionMerge %35 None
               OpBranchConditional %33 %34 %35
         %34 = OpLabel
         %43 = OpAccessChain %42 %39 %17
               OpStore %43 %41
         %45 = OpLoad %20 %22
         %46 = OpAccessChain %42 %39 %44
               OpStore %46 %45
               OpBranch %35
         %35 = OpLabel
         %52 = OpAccessChain %51 %49 %13
               OpStore %52 %50
         %53 = OpAccessChain %51 %49 %13
         %54 = OpLoad %32 %53
         %55 = OpAccessChain %51 %49 %17
               OpStore %55 %54
         %64 = OpAccessChain %63 %60 %13 %13
               OpStore %64 %62
         %71 = OpAccessChain %70 %60 %17
               OpStore %71 %69
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
  // Make a vec2[3]
  TransformationCompositeConstruct make_vec2_array_length_3(
      37, {41, 45, 27},
      MakeInstructionDescriptor(46, spv::Op::OpAccessChain, 0), 200);
  // Bad: there are too many components
  TransformationCompositeConstruct make_vec2_array_length_3_bad(
      37, {41, 45, 27, 27},
      MakeInstructionDescriptor(46, spv::Op::OpAccessChain, 0), 200);
  // The first component does not correspond to an instruction with a result
  // type so this check should return false.
  TransformationCompositeConstruct make_vec2_array_length_3_nores(
      37, {2, 45, 27}, MakeInstructionDescriptor(46, spv::Op::OpAccessChain, 0),
      200);
  ASSERT_TRUE(make_vec2_array_length_3.IsApplicable(context.get(),
                                                    transformation_context));
  ASSERT_FALSE(make_vec2_array_length_3_bad.IsApplicable(
      context.get(), transformation_context));
  ASSERT_FALSE(make_vec2_array_length_3_nores.IsApplicable(
      context.get(), transformation_context));
  ASSERT_EQ(nullptr, context->get_def_use_mgr()->GetDef(200));
  ASSERT_EQ(nullptr, context->get_instr_block(200));
  uint32_t num_uses_of_41_before = context->get_def_use_mgr()->NumUses(41);
  uint32_t num_uses_of_45_before = context->get_def_use_mgr()->NumUses(45);
  uint32_t num_uses_of_27_before = context->get_def_use_mgr()->NumUses(27);
  ApplyAndCheckFreshIds(make_vec2_array_length_3, context.get(),
                        &transformation_context);
  ASSERT_EQ(spv::Op::OpCompositeConstruct,
            context->get_def_use_mgr()->GetDef(200)->opcode());
  ASSERT_EQ(34, context->get_instr_block(200)->id());
  ASSERT_EQ(num_uses_of_41_before + 1, context->get_def_use_mgr()->NumUses(41));
  ASSERT_EQ(num_uses_of_45_before + 1, context->get_def_use_mgr()->NumUses(45));
  ASSERT_EQ(num_uses_of_27_before + 1, context->get_def_use_mgr()->NumUses(27));
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(41, {}), MakeDataDescriptor(200, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(45, {}), MakeDataDescriptor(200, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(27, {}), MakeDataDescriptor(200, {2})));

  // Make a float[2]
  TransformationCompositeConstruct make_float_array_length_2(
      9, {24, 40}, MakeInstructionDescriptor(71, spv::Op::OpStore, 0), 201);
  // Bad: %41 does not have type float
  TransformationCompositeConstruct make_float_array_length_2_bad(
      9, {41, 40}, MakeInstructionDescriptor(71, spv::Op::OpStore, 0), 201);
  ASSERT_TRUE(make_float_array_length_2.IsApplicable(context.get(),
                                                     transformation_context));
  ASSERT_FALSE(make_float_array_length_2_bad.IsApplicable(
      context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_float_array_length_2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(24, {}), MakeDataDescriptor(201, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(40, {}), MakeDataDescriptor(201, {1})));

  // Make a bool[3]
  TransformationCompositeConstruct make_bool_array_length_3(
      47, {33, 50, 50},
      MakeInstructionDescriptor(33, spv::Op::OpSelectionMerge, 0), 202);
  // Bad: %54 is not available at the desired program point.
  TransformationCompositeConstruct make_bool_array_length_3_bad(
      47, {33, 54, 50},
      MakeInstructionDescriptor(33, spv::Op::OpSelectionMerge, 0), 202);
  ASSERT_TRUE(make_bool_array_length_3.IsApplicable(context.get(),
                                                    transformation_context));
  ASSERT_FALSE(make_bool_array_length_3_bad.IsApplicable(
      context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_bool_array_length_3, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(33, {}), MakeDataDescriptor(202, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {}), MakeDataDescriptor(202, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(50, {}), MakeDataDescriptor(202, {2})));

  // make a uvec3[2][2]
  TransformationCompositeConstruct make_uvec3_array_length_2_2(
      58, {69, 100}, MakeInstructionDescriptor(64, spv::Op::OpStore, 0), 203);
  // Bad: Skip count 100 is too large.
  TransformationCompositeConstruct make_uvec3_array_length_2_2_bad(
      58, {33, 54}, MakeInstructionDescriptor(64, spv::Op::OpStore, 100), 203);
  ASSERT_TRUE(make_uvec3_array_length_2_2.IsApplicable(context.get(),
                                                       transformation_context));
  ASSERT_FALSE(make_uvec3_array_length_2_2_bad.IsApplicable(
      context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_uvec3_array_length_2_2, context.get(),
                        &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(69, {}), MakeDataDescriptor(203, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {}), MakeDataDescriptor(203, {1})));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %11 "floats"
               OpName %22 "x"
               OpName %39 "vecs"
               OpName %49 "bools"
               OpName %60 "many_uvec3s"
               OpDecorate %60 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 2
          %9 = OpTypeArray %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpTypeInt 32 1
         %13 = OpConstant %12 0
         %14 = OpConstant %6 1
         %15 = OpTypePointer Function %6
         %17 = OpConstant %12 1
         %18 = OpConstant %6 2
         %20 = OpTypeVector %6 2
         %21 = OpTypePointer Function %20
         %32 = OpTypeBool
         %36 = OpConstant %7 3
         %37 = OpTypeArray %20 %36
         %38 = OpTypePointer Private %37
         %39 = OpVariable %38 Private
         %40 = OpConstant %6 3
         %41 = OpConstantComposite %20 %40 %40
         %42 = OpTypePointer Private %20
         %44 = OpConstant %12 2
         %47 = OpTypeArray %32 %36
         %48 = OpTypePointer Function %47
         %50 = OpConstantTrue %32
         %51 = OpTypePointer Function %32
         %56 = OpTypeVector %7 3
         %57 = OpTypeArray %56 %8
         %58 = OpTypeArray %57 %8
         %59 = OpTypePointer Function %58
         %61 = OpConstant %7 4
         %62 = OpConstantComposite %56 %61 %61 %61
         %63 = OpTypePointer Function %56
         %65 = OpConstant %7 5
         %66 = OpConstantComposite %56 %65 %65 %65
         %67 = OpConstant %7 6
         %68 = OpConstantComposite %56 %67 %67 %67
         %69 = OpConstantComposite %57 %66 %68
        %100 = OpUndef %57
         %70 = OpTypePointer Function %57
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %22 = OpVariable %21 Function
         %49 = OpVariable %48 Function
         %60 = OpVariable %59 Function
         %16 = OpAccessChain %15 %11 %13
               OpStore %16 %14
         %19 = OpAccessChain %15 %11 %17
               OpStore %19 %18
         %23 = OpAccessChain %15 %11 %13
         %24 = OpLoad %6 %23
         %25 = OpAccessChain %15 %11 %17
         %26 = OpLoad %6 %25
         %27 = OpCompositeConstruct %20 %24 %26
               OpStore %22 %27
         %28 = OpAccessChain %15 %11 %13
         %29 = OpLoad %6 %28
         %30 = OpAccessChain %15 %11 %17
         %31 = OpLoad %6 %30
         %33 = OpFOrdGreaterThan %32 %29 %31
        %202 = OpCompositeConstruct %47 %33 %50 %50
               OpSelectionMerge %35 None
               OpBranchConditional %33 %34 %35
         %34 = OpLabel
         %43 = OpAccessChain %42 %39 %17
               OpStore %43 %41
         %45 = OpLoad %20 %22
        %200 = OpCompositeConstruct %37 %41 %45 %27
         %46 = OpAccessChain %42 %39 %44
               OpStore %46 %45
               OpBranch %35
         %35 = OpLabel
         %52 = OpAccessChain %51 %49 %13
               OpStore %52 %50
         %53 = OpAccessChain %51 %49 %13
         %54 = OpLoad %32 %53
         %55 = OpAccessChain %51 %49 %17
               OpStore %55 %54
         %64 = OpAccessChain %63 %60 %13 %13
        %203 = OpCompositeConstruct %58 %69 %100
               OpStore %64 %62
         %71 = OpAccessChain %70 %60 %17
        %201 = OpCompositeConstruct %9 %24 %40
               OpStore %71 %69
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationCompositeConstructTest, ConstructMatrices) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "v1"
               OpName %12 "v2"
               OpName %14 "v3"
               OpName %19 "v4"
               OpName %26 "v5"
               OpName %29 "v6"
               OpName %34 "m34"
               OpName %37 "m43"
               OpName %43 "vecs"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 3
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstantComposite %7 %10 %10 %10
         %17 = OpTypeVector %6 4
         %18 = OpTypePointer Function %17
         %21 = OpConstant %6 2
         %32 = OpTypeMatrix %17 3
         %33 = OpTypePointer Private %32
         %34 = OpVariable %33 Private
         %35 = OpTypeMatrix %7 4
         %36 = OpTypePointer Private %35
         %37 = OpVariable %36 Private
         %38 = OpTypeVector %6 2
         %39 = OpTypeInt 32 0
         %40 = OpConstant %39 3
         %41 = OpTypeArray %38 %40
         %42 = OpTypePointer Private %41
         %43 = OpVariable %42 Private
        %100 = OpUndef %7
        %101 = OpUndef %17
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %12 = OpVariable %8 Function
         %14 = OpVariable %8 Function
         %19 = OpVariable %18 Function
         %26 = OpVariable %18 Function
         %29 = OpVariable %18 Function
               OpStore %9 %11
         %13 = OpLoad %7 %9
               OpStore %12 %13
         %15 = OpLoad %7 %12
         %16 = OpVectorShuffle %7 %15 %15 2 1 0
               OpStore %14 %16
         %20 = OpLoad %7 %14
         %22 = OpCompositeExtract %6 %20 0
         %23 = OpCompositeExtract %6 %20 1
         %24 = OpCompositeExtract %6 %20 2
         %25 = OpCompositeConstruct %17 %22 %23 %24 %21
               OpStore %19 %25
         %27 = OpLoad %17 %19
         %28 = OpVectorShuffle %17 %27 %27 3 2 1 0
               OpStore %26 %28
         %30 = OpLoad %7 %9
         %31 = OpVectorShuffle %17 %30 %30 0 0 1 1
               OpStore %29 %31
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
  // make a mat3x4
  TransformationCompositeConstruct make_mat34(
      32, {25, 28, 31}, MakeInstructionDescriptor(31, spv::Op::OpReturn, 0),
      200);
  // Bad: %35 is mat4x3, not mat3x4.
  TransformationCompositeConstruct make_mat34_bad(
      35, {25, 28, 31}, MakeInstructionDescriptor(31, spv::Op::OpReturn, 0),
      200);
  // The first component does not correspond to an instruction with a result
  // type so this check should return false.
  TransformationCompositeConstruct make_mat34_nores(
      32, {2, 28, 31}, MakeInstructionDescriptor(31, spv::Op::OpReturn, 0),
      200);
  ASSERT_TRUE(make_mat34.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_mat34_bad.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_mat34_nores.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_mat34, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(25, {}), MakeDataDescriptor(200, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(28, {}), MakeDataDescriptor(200, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(31, {}), MakeDataDescriptor(200, {2})));

  // make a mat4x3
  TransformationCompositeConstruct make_mat43(
      35, {11, 13, 16, 100}, MakeInstructionDescriptor(31, spv::Op::OpStore, 0),
      201);
  // Bad: %25 does not match the matrix's column type.
  TransformationCompositeConstruct make_mat43_bad(
      35, {25, 13, 16, 100}, MakeInstructionDescriptor(31, spv::Op::OpStore, 0),
      201);
  ASSERT_TRUE(make_mat43.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_mat43_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_mat43, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(11, {}), MakeDataDescriptor(201, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(13, {}), MakeDataDescriptor(201, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(16, {}), MakeDataDescriptor(201, {2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {}), MakeDataDescriptor(201, {3})));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "v1"
               OpName %12 "v2"
               OpName %14 "v3"
               OpName %19 "v4"
               OpName %26 "v5"
               OpName %29 "v6"
               OpName %34 "m34"
               OpName %37 "m43"
               OpName %43 "vecs"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 3
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstantComposite %7 %10 %10 %10
         %17 = OpTypeVector %6 4
         %18 = OpTypePointer Function %17
         %21 = OpConstant %6 2
         %32 = OpTypeMatrix %17 3
         %33 = OpTypePointer Private %32
         %34 = OpVariable %33 Private
         %35 = OpTypeMatrix %7 4
         %36 = OpTypePointer Private %35
         %37 = OpVariable %36 Private
         %38 = OpTypeVector %6 2
         %39 = OpTypeInt 32 0
         %40 = OpConstant %39 3
         %41 = OpTypeArray %38 %40
         %42 = OpTypePointer Private %41
         %43 = OpVariable %42 Private
        %100 = OpUndef %7
        %101 = OpUndef %17
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %12 = OpVariable %8 Function
         %14 = OpVariable %8 Function
         %19 = OpVariable %18 Function
         %26 = OpVariable %18 Function
         %29 = OpVariable %18 Function
               OpStore %9 %11
         %13 = OpLoad %7 %9
               OpStore %12 %13
         %15 = OpLoad %7 %12
         %16 = OpVectorShuffle %7 %15 %15 2 1 0
               OpStore %14 %16
         %20 = OpLoad %7 %14
         %22 = OpCompositeExtract %6 %20 0
         %23 = OpCompositeExtract %6 %20 1
         %24 = OpCompositeExtract %6 %20 2
         %25 = OpCompositeConstruct %17 %22 %23 %24 %21
               OpStore %19 %25
         %27 = OpLoad %17 %19
         %28 = OpVectorShuffle %17 %27 %27 3 2 1 0
               OpStore %26 %28
         %30 = OpLoad %7 %9
         %31 = OpVectorShuffle %17 %30 %30 0 0 1 1
        %201 = OpCompositeConstruct %35 %11 %13 %16 %100
               OpStore %29 %31
        %200 = OpCompositeConstruct %32 %25 %28 %31
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationCompositeConstructTest, ConstructStructs) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "Inner"
               OpMemberName %9 0 "a"
               OpMemberName %9 1 "b"
               OpName %11 "i1"
               OpName %22 "i2"
               OpName %33 "Outer"
               OpMemberName %33 0 "c"
               OpMemberName %33 1 "d"
               OpMemberName %33 2 "e"
               OpName %35 "o"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
          %8 = OpTypeInt 32 1
          %9 = OpTypeStruct %7 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %8 0
         %13 = OpConstant %6 2
         %14 = OpTypeInt 32 0
         %15 = OpConstant %14 0
         %16 = OpTypePointer Function %6
         %18 = OpConstant %8 1
         %19 = OpConstant %8 3
         %20 = OpTypePointer Function %8
         %23 = OpTypePointer Function %7
         %31 = OpConstant %14 2
         %32 = OpTypeArray %9 %31
         %33 = OpTypeStruct %32 %9 %6
         %34 = OpTypePointer Function %33
         %36 = OpConstant %6 1
         %37 = OpConstantComposite %7 %36 %13
         %38 = OpConstant %8 2
         %39 = OpConstantComposite %9 %37 %38
         %40 = OpConstant %6 3
         %41 = OpConstant %6 4
         %42 = OpConstantComposite %7 %40 %41
         %56 = OpConstant %6 5
        %100 = OpUndef %9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %22 = OpVariable %10 Function
         %35 = OpVariable %34 Function
         %17 = OpAccessChain %16 %11 %12 %15
               OpStore %17 %13
         %21 = OpAccessChain %20 %11 %18
               OpStore %21 %19
         %24 = OpAccessChain %23 %11 %12
         %25 = OpLoad %7 %24
         %26 = OpAccessChain %23 %22 %12
               OpStore %26 %25
         %27 = OpAccessChain %20 %11 %18
         %28 = OpLoad %8 %27
         %29 = OpIAdd %8 %28 %18
         %30 = OpAccessChain %20 %22 %18
               OpStore %30 %29
         %43 = OpAccessChain %20 %11 %18
         %44 = OpLoad %8 %43
         %45 = OpCompositeConstruct %9 %42 %44
         %46 = OpCompositeConstruct %32 %39 %45
         %47 = OpLoad %9 %22
         %48 = OpCompositeConstruct %33 %46 %47 %40
               OpStore %35 %48
         %49 = OpLoad %9 %11
         %50 = OpAccessChain %10 %35 %12 %12
               OpStore %50 %49
         %51 = OpLoad %9 %22
         %52 = OpAccessChain %10 %35 %12 %18
               OpStore %52 %51
         %53 = OpAccessChain %10 %35 %12 %12
         %54 = OpLoad %9 %53
         %55 = OpAccessChain %10 %35 %18
               OpStore %55 %54
         %57 = OpAccessChain %16 %35 %38
               OpStore %57 %56
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
  // make an Inner
  TransformationCompositeConstruct make_inner(
      9, {25, 19}, MakeInstructionDescriptor(57, spv::Op::OpAccessChain, 0),
      200);
  // Bad: Too few fields to make the struct.
  TransformationCompositeConstruct make_inner_bad(
      9, {25}, MakeInstructionDescriptor(57, spv::Op::OpAccessChain, 0), 200);
  // The first component does not correspond to an instruction with a result
  // type so this check should return false.
  TransformationCompositeConstruct make_inner_nores(
      9, {2, 19}, MakeInstructionDescriptor(57, spv::Op::OpAccessChain, 0),
      200);
  ASSERT_TRUE(make_inner.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_inner_bad.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_inner_nores.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_inner, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(25, {}), MakeDataDescriptor(200, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(19, {}), MakeDataDescriptor(200, {1})));

  // make an Outer
  TransformationCompositeConstruct make_outer(
      33, {46, 200, 56},
      MakeInstructionDescriptor(200, spv::Op::OpAccessChain, 0), 201);
  // Bad: %200 is not available at the desired program point.
  TransformationCompositeConstruct make_outer_bad(
      33, {46, 200, 56},
      MakeInstructionDescriptor(200, spv::Op::OpCompositeConstruct, 0), 201);
  ASSERT_TRUE(make_outer.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_outer_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_outer, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(46, {}), MakeDataDescriptor(201, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(200, {}), MakeDataDescriptor(201, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(56, {}), MakeDataDescriptor(201, {2})));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "Inner"
               OpMemberName %9 0 "a"
               OpMemberName %9 1 "b"
               OpName %11 "i1"
               OpName %22 "i2"
               OpName %33 "Outer"
               OpMemberName %33 0 "c"
               OpMemberName %33 1 "d"
               OpMemberName %33 2 "e"
               OpName %35 "o"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
          %8 = OpTypeInt 32 1
          %9 = OpTypeStruct %7 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %8 0
         %13 = OpConstant %6 2
         %14 = OpTypeInt 32 0
         %15 = OpConstant %14 0
         %16 = OpTypePointer Function %6
         %18 = OpConstant %8 1
         %19 = OpConstant %8 3
         %20 = OpTypePointer Function %8
         %23 = OpTypePointer Function %7
         %31 = OpConstant %14 2
         %32 = OpTypeArray %9 %31
         %33 = OpTypeStruct %32 %9 %6
         %34 = OpTypePointer Function %33
         %36 = OpConstant %6 1
         %37 = OpConstantComposite %7 %36 %13
         %38 = OpConstant %8 2
         %39 = OpConstantComposite %9 %37 %38
         %40 = OpConstant %6 3
         %41 = OpConstant %6 4
         %42 = OpConstantComposite %7 %40 %41
         %56 = OpConstant %6 5
        %100 = OpUndef %9
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %22 = OpVariable %10 Function
         %35 = OpVariable %34 Function
         %17 = OpAccessChain %16 %11 %12 %15
               OpStore %17 %13
         %21 = OpAccessChain %20 %11 %18
               OpStore %21 %19
         %24 = OpAccessChain %23 %11 %12
         %25 = OpLoad %7 %24
         %26 = OpAccessChain %23 %22 %12
               OpStore %26 %25
         %27 = OpAccessChain %20 %11 %18
         %28 = OpLoad %8 %27
         %29 = OpIAdd %8 %28 %18
         %30 = OpAccessChain %20 %22 %18
               OpStore %30 %29
         %43 = OpAccessChain %20 %11 %18
         %44 = OpLoad %8 %43
         %45 = OpCompositeConstruct %9 %42 %44
         %46 = OpCompositeConstruct %32 %39 %45
         %47 = OpLoad %9 %22
         %48 = OpCompositeConstruct %33 %46 %47 %40
               OpStore %35 %48
         %49 = OpLoad %9 %11
         %50 = OpAccessChain %10 %35 %12 %12
               OpStore %50 %49
         %51 = OpLoad %9 %22
         %52 = OpAccessChain %10 %35 %12 %18
               OpStore %52 %51
         %53 = OpAccessChain %10 %35 %12 %12
         %54 = OpLoad %9 %53
         %55 = OpAccessChain %10 %35 %18
               OpStore %55 %54
        %200 = OpCompositeConstruct %9 %25 %19
        %201 = OpCompositeConstruct %33 %46 %200 %56
         %57 = OpAccessChain %16 %35 %38
               OpStore %57 %56
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationCompositeConstructTest, ConstructVectors) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "v2"
               OpName %27 "v3"
               OpName %46 "v4"
               OpName %53 "iv2"
               OpName %61 "uv3"
               OpName %72 "bv4"
               OpName %88 "uv2"
               OpName %95 "bv3"
               OpName %104 "bv2"
               OpName %116 "iv3"
               OpName %124 "iv4"
               OpName %133 "uv4"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpConstantComposite %7 %10 %11
         %13 = OpTypeInt 32 0
         %14 = OpConstant %13 0
         %15 = OpTypePointer Function %6
         %18 = OpConstant %13 1
         %21 = OpTypeBool
         %25 = OpTypeVector %6 3
         %26 = OpTypePointer Function %25
         %33 = OpConstant %6 3
         %34 = OpConstant %6 -0.756802499
         %38 = OpConstant %13 2
         %44 = OpTypeVector %6 4
         %45 = OpTypePointer Function %44
         %50 = OpTypeInt 32 1
         %51 = OpTypeVector %50 2
         %52 = OpTypePointer Function %51
         %57 = OpTypePointer Function %50
         %59 = OpTypeVector %13 3
         %60 = OpTypePointer Function %59
         %65 = OpConstant %13 3
         %67 = OpTypePointer Function %13
         %70 = OpTypeVector %21 4
         %71 = OpTypePointer Function %70
         %73 = OpConstantTrue %21
         %74 = OpTypePointer Function %21
         %86 = OpTypeVector %13 2
         %87 = OpTypePointer Function %86
         %93 = OpTypeVector %21 3
         %94 = OpTypePointer Function %93
        %102 = OpTypeVector %21 2
        %103 = OpTypePointer Function %102
        %111 = OpConstantFalse %21
        %114 = OpTypeVector %50 3
        %115 = OpTypePointer Function %114
        %117 = OpConstant %50 3
        %122 = OpTypeVector %50 4
        %123 = OpTypePointer Function %122
        %131 = OpTypeVector %13 4
        %132 = OpTypePointer Function %131
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %27 = OpVariable %26 Function
         %46 = OpVariable %45 Function
         %53 = OpVariable %52 Function
         %61 = OpVariable %60 Function
         %72 = OpVariable %71 Function
         %88 = OpVariable %87 Function
         %95 = OpVariable %94 Function
        %104 = OpVariable %103 Function
        %116 = OpVariable %115 Function
        %124 = OpVariable %123 Function
        %133 = OpVariable %132 Function
               OpStore %9 %12
         %16 = OpAccessChain %15 %9 %14
         %17 = OpLoad %6 %16
         %19 = OpAccessChain %15 %9 %18
         %20 = OpLoad %6 %19
         %22 = OpFOrdGreaterThan %21 %17 %20
               OpSelectionMerge %24 None
               OpBranchConditional %22 %23 %101
         %23 = OpLabel
         %28 = OpAccessChain %15 %9 %14
         %29 = OpLoad %6 %28
         %30 = OpAccessChain %15 %9 %18
         %31 = OpLoad %6 %30
         %32 = OpFAdd %6 %29 %31
         %35 = OpCompositeConstruct %25 %32 %33 %34
               OpStore %27 %35
         %36 = OpAccessChain %15 %27 %14
         %37 = OpLoad %6 %36
         %39 = OpAccessChain %15 %27 %38
         %40 = OpLoad %6 %39
         %41 = OpFOrdLessThan %21 %37 %40
               OpSelectionMerge %43 None
               OpBranchConditional %41 %42 %69
         %42 = OpLabel
         %47 = OpAccessChain %15 %9 %18
         %48 = OpLoad %6 %47
         %49 = OpAccessChain %15 %46 %14
               OpStore %49 %48
         %54 = OpAccessChain %15 %27 %38
         %55 = OpLoad %6 %54
         %56 = OpConvertFToS %50 %55
         %58 = OpAccessChain %57 %53 %14
               OpStore %58 %56
         %62 = OpAccessChain %15 %46 %14
         %63 = OpLoad %6 %62
         %64 = OpConvertFToU %13 %63
         %66 = OpIAdd %13 %64 %65
         %68 = OpAccessChain %67 %61 %14
               OpStore %68 %66
               OpBranch %43
         %69 = OpLabel
         %75 = OpAccessChain %74 %72 %14
               OpStore %75 %73
         %76 = OpAccessChain %74 %72 %14
         %77 = OpLoad %21 %76
         %78 = OpLogicalNot %21 %77
         %79 = OpAccessChain %74 %72 %18
               OpStore %79 %78
         %80 = OpAccessChain %74 %72 %14
         %81 = OpLoad %21 %80
         %82 = OpAccessChain %74 %72 %18
         %83 = OpLoad %21 %82
         %84 = OpLogicalAnd %21 %81 %83
         %85 = OpAccessChain %74 %72 %38
               OpStore %85 %84
         %89 = OpAccessChain %67 %88 %14
         %90 = OpLoad %13 %89
         %91 = OpINotEqual %21 %90 %14
         %92 = OpAccessChain %74 %72 %65
               OpStore %92 %91
               OpBranch %43
         %43 = OpLabel
         %96 = OpLoad %70 %72
         %97 = OpCompositeExtract %21 %96 0
         %98 = OpCompositeExtract %21 %96 1
         %99 = OpCompositeExtract %21 %96 2
        %100 = OpCompositeConstruct %93 %97 %98 %99
               OpStore %95 %100
               OpBranch %24
        %101 = OpLabel
        %105 = OpAccessChain %67 %88 %14
        %106 = OpLoad %13 %105
        %107 = OpINotEqual %21 %106 %14
        %108 = OpCompositeConstruct %102 %107 %107
               OpStore %104 %108
               OpBranch %24
         %24 = OpLabel
        %109 = OpAccessChain %74 %104 %18
        %110 = OpLoad %21 %109
        %112 = OpLogicalOr %21 %110 %111
        %113 = OpAccessChain %74 %104 %14
               OpStore %113 %112
        %118 = OpAccessChain %57 %116 %14
               OpStore %118 %117
        %119 = OpAccessChain %57 %116 %14
        %120 = OpLoad %50 %119
        %121 = OpAccessChain %57 %53 %18
               OpStore %121 %120
        %125 = OpAccessChain %57 %116 %14
        %126 = OpLoad %50 %125
        %127 = OpAccessChain %57 %53 %18
        %128 = OpLoad %50 %127
        %129 = OpIAdd %50 %126 %128
        %130 = OpAccessChain %57 %124 %65
               OpStore %130 %129
        %134 = OpAccessChain %57 %116 %14
        %135 = OpLoad %50 %134
        %136 = OpBitcast %13 %135
        %137 = OpAccessChain %67 %133 %14
               OpStore %137 %136
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
  TransformationCompositeConstruct make_vec2(
      7, {17, 11}, MakeInstructionDescriptor(100, spv::Op::OpStore, 0), 200);
  // Bad: not enough data for a vec2
  TransformationCompositeConstruct make_vec2_bad(
      7, {11}, MakeInstructionDescriptor(100, spv::Op::OpStore, 0), 200);
  ASSERT_TRUE(make_vec2.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_vec2_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_vec2, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(17, {}), MakeDataDescriptor(200, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(11, {}), MakeDataDescriptor(200, {1})));

  TransformationCompositeConstruct make_vec3(
      25, {12, 32},
      MakeInstructionDescriptor(35, spv::Op::OpCompositeConstruct, 0), 201);
  // Bad: too much data for a vec3
  TransformationCompositeConstruct make_vec3_bad(
      25, {12, 32, 32},
      MakeInstructionDescriptor(35, spv::Op::OpCompositeConstruct, 0), 201);
  ASSERT_TRUE(make_vec3.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_vec3_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_vec3, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(12, {0}), MakeDataDescriptor(201, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(12, {1}), MakeDataDescriptor(201, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(32, {}), MakeDataDescriptor(201, {2})));

  TransformationCompositeConstruct make_vec4(
      44, {32, 32, 10, 11},
      MakeInstructionDescriptor(75, spv::Op::OpAccessChain, 0), 202);
  // Bad: id 48 is not available at the insertion points
  TransformationCompositeConstruct make_vec4_bad(
      44, {48, 32, 10, 11},
      MakeInstructionDescriptor(75, spv::Op::OpAccessChain, 0), 202);
  ASSERT_TRUE(make_vec4.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_vec4_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_vec4, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(32, {}), MakeDataDescriptor(202, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(32, {}), MakeDataDescriptor(202, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(10, {}), MakeDataDescriptor(202, {2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(11, {}), MakeDataDescriptor(202, {3})));

  TransformationCompositeConstruct make_ivec2(
      51, {126, 120}, MakeInstructionDescriptor(128, spv::Op::OpLoad, 0), 203);
  // Bad: if 128 is not available at the instruction that defines 128
  TransformationCompositeConstruct make_ivec2_bad(
      51, {128, 120}, MakeInstructionDescriptor(128, spv::Op::OpLoad, 0), 203);
  ASSERT_TRUE(make_ivec2.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_ivec2_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_ivec2, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(126, {}), MakeDataDescriptor(203, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(120, {}), MakeDataDescriptor(203, {1})));

  TransformationCompositeConstruct make_ivec3(
      114, {56, 117, 56},
      MakeInstructionDescriptor(66, spv::Op::OpAccessChain, 0), 204);
  // Bad because 1300 is not an id
  TransformationCompositeConstruct make_ivec3_bad(
      114, {56, 117, 1300},
      MakeInstructionDescriptor(66, spv::Op::OpAccessChain, 0), 204);
  ASSERT_TRUE(make_ivec3.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_ivec3_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_ivec3, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(56, {}), MakeDataDescriptor(204, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(117, {}), MakeDataDescriptor(204, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(56, {}), MakeDataDescriptor(204, {2})));

  TransformationCompositeConstruct make_ivec4(
      122, {56, 117, 117, 117},
      MakeInstructionDescriptor(66, spv::Op::OpIAdd, 0), 205);
  // Bad because 86 is the wrong type.
  TransformationCompositeConstruct make_ivec4_bad(
      86, {56, 117, 117, 117},
      MakeInstructionDescriptor(66, spv::Op::OpIAdd, 0), 205);
  ASSERT_TRUE(make_ivec4.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_ivec4_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_ivec4, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(56, {}), MakeDataDescriptor(205, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(117, {}), MakeDataDescriptor(205, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(117, {}), MakeDataDescriptor(205, {2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(117, {}), MakeDataDescriptor(205, {3})));

  TransformationCompositeConstruct make_uvec2(
      86, {18, 38}, MakeInstructionDescriptor(133, spv::Op::OpAccessChain, 0),
      206);
  TransformationCompositeConstruct make_uvec2_bad(
      86, {18, 38}, MakeInstructionDescriptor(133, spv::Op::OpAccessChain, 200),
      206);
  ASSERT_TRUE(make_uvec2.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_uvec2_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_uvec2, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(18, {}), MakeDataDescriptor(206, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(38, {}), MakeDataDescriptor(206, {1})));

  TransformationCompositeConstruct make_uvec3(
      59, {14, 18, 136}, MakeInstructionDescriptor(137, spv::Op::OpReturn, 0),
      207);
  // Bad because 1300 is not an id
  TransformationCompositeConstruct make_uvec3_bad(
      59, {14, 18, 1300}, MakeInstructionDescriptor(137, spv::Op::OpReturn, 0),
      207);
  ASSERT_TRUE(make_uvec3.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_uvec3_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_uvec3, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(14, {}), MakeDataDescriptor(207, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(18, {}), MakeDataDescriptor(207, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(136, {}), MakeDataDescriptor(207, {2})));

  TransformationCompositeConstruct make_uvec4(
      131, {14, 18, 136, 136},
      MakeInstructionDescriptor(137, spv::Op::OpAccessChain, 0), 208);
  // Bad because 86 is the wrong type.
  TransformationCompositeConstruct make_uvec4_bad(
      86, {14, 18, 136, 136},
      MakeInstructionDescriptor(137, spv::Op::OpAccessChain, 0), 208);
  ASSERT_TRUE(make_uvec4.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_uvec4_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_uvec4, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(14, {}), MakeDataDescriptor(208, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(18, {}), MakeDataDescriptor(208, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(136, {}), MakeDataDescriptor(208, {2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(136, {}), MakeDataDescriptor(208, {3})));

  TransformationCompositeConstruct make_bvec2(
      102,
      {
          111,
          41,
      },
      MakeInstructionDescriptor(75, spv::Op::OpAccessChain, 0), 209);
  // Bad because 0 is not a valid base instruction id
  TransformationCompositeConstruct make_bvec2_bad(
      102,
      {
          111,
          41,
      },
      MakeInstructionDescriptor(0, spv::Op::OpExtInstImport, 0), 209);
  ASSERT_TRUE(make_bvec2.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_bvec2_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_bvec2, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(111, {}), MakeDataDescriptor(209, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(41, {}), MakeDataDescriptor(209, {1})));

  TransformationCompositeConstruct make_bvec3(
      93, {108, 73}, MakeInstructionDescriptor(108, spv::Op::OpStore, 0), 210);
  // Bad because there are too many components for a bvec3
  TransformationCompositeConstruct make_bvec3_bad(
      93, {108, 108}, MakeInstructionDescriptor(108, spv::Op::OpStore, 0), 210);
  ASSERT_TRUE(make_bvec3.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_bvec3_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_bvec3, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(108, {0}), MakeDataDescriptor(210, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(108, {1}), MakeDataDescriptor(210, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(73, {}), MakeDataDescriptor(210, {2})));

  TransformationCompositeConstruct make_bvec4(
      70, {108, 108}, MakeInstructionDescriptor(108, spv::Op::OpBranch, 0),
      211);
  // Bad because 21 is a type, not a result id
  TransformationCompositeConstruct make_bvec4_bad(
      70, {21, 108}, MakeInstructionDescriptor(108, spv::Op::OpBranch, 0), 211);
  ASSERT_TRUE(make_bvec4.IsApplicable(context.get(), transformation_context));
  ASSERT_FALSE(
      make_bvec4_bad.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(make_bvec4, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(108, {0}), MakeDataDescriptor(211, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(108, {1}), MakeDataDescriptor(211, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(108, {0}), MakeDataDescriptor(211, {2})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(108, {1}), MakeDataDescriptor(211, {3})));

  std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "v2"
               OpName %27 "v3"
               OpName %46 "v4"
               OpName %53 "iv2"
               OpName %61 "uv3"
               OpName %72 "bv4"
               OpName %88 "uv2"
               OpName %95 "bv3"
               OpName %104 "bv2"
               OpName %116 "iv3"
               OpName %124 "iv4"
               OpName %133 "uv4"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstant %6 2
         %12 = OpConstantComposite %7 %10 %11
         %13 = OpTypeInt 32 0
         %14 = OpConstant %13 0
         %15 = OpTypePointer Function %6
         %18 = OpConstant %13 1
         %21 = OpTypeBool
         %25 = OpTypeVector %6 3
         %26 = OpTypePointer Function %25
         %33 = OpConstant %6 3
         %34 = OpConstant %6 -0.756802499
         %38 = OpConstant %13 2
         %44 = OpTypeVector %6 4
         %45 = OpTypePointer Function %44
         %50 = OpTypeInt 32 1
         %51 = OpTypeVector %50 2
         %52 = OpTypePointer Function %51
         %57 = OpTypePointer Function %50
         %59 = OpTypeVector %13 3
         %60 = OpTypePointer Function %59
         %65 = OpConstant %13 3
         %67 = OpTypePointer Function %13
         %70 = OpTypeVector %21 4
         %71 = OpTypePointer Function %70
         %73 = OpConstantTrue %21
         %74 = OpTypePointer Function %21
         %86 = OpTypeVector %13 2
         %87 = OpTypePointer Function %86
         %93 = OpTypeVector %21 3
         %94 = OpTypePointer Function %93
        %102 = OpTypeVector %21 2
        %103 = OpTypePointer Function %102
        %111 = OpConstantFalse %21
        %114 = OpTypeVector %50 3
        %115 = OpTypePointer Function %114
        %117 = OpConstant %50 3
        %122 = OpTypeVector %50 4
        %123 = OpTypePointer Function %122
        %131 = OpTypeVector %13 4
        %132 = OpTypePointer Function %131
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %27 = OpVariable %26 Function
         %46 = OpVariable %45 Function
         %53 = OpVariable %52 Function
         %61 = OpVariable %60 Function
         %72 = OpVariable %71 Function
         %88 = OpVariable %87 Function
         %95 = OpVariable %94 Function
        %104 = OpVariable %103 Function
        %116 = OpVariable %115 Function
        %124 = OpVariable %123 Function
        %133 = OpVariable %132 Function
               OpStore %9 %12
        %206 = OpCompositeConstruct %86 %18 %38
         %16 = OpAccessChain %15 %9 %14
         %17 = OpLoad %6 %16
         %19 = OpAccessChain %15 %9 %18
         %20 = OpLoad %6 %19
         %22 = OpFOrdGreaterThan %21 %17 %20
               OpSelectionMerge %24 None
               OpBranchConditional %22 %23 %101
         %23 = OpLabel
         %28 = OpAccessChain %15 %9 %14
         %29 = OpLoad %6 %28
         %30 = OpAccessChain %15 %9 %18
         %31 = OpLoad %6 %30
         %32 = OpFAdd %6 %29 %31
        %201 = OpCompositeConstruct %25 %12 %32
         %35 = OpCompositeConstruct %25 %32 %33 %34
               OpStore %27 %35
         %36 = OpAccessChain %15 %27 %14
         %37 = OpLoad %6 %36
         %39 = OpAccessChain %15 %27 %38
         %40 = OpLoad %6 %39
         %41 = OpFOrdLessThan %21 %37 %40
               OpSelectionMerge %43 None
               OpBranchConditional %41 %42 %69
         %42 = OpLabel
         %47 = OpAccessChain %15 %9 %18
         %48 = OpLoad %6 %47
         %49 = OpAccessChain %15 %46 %14
               OpStore %49 %48
         %54 = OpAccessChain %15 %27 %38
         %55 = OpLoad %6 %54
         %56 = OpConvertFToS %50 %55
         %58 = OpAccessChain %57 %53 %14
               OpStore %58 %56
         %62 = OpAccessChain %15 %46 %14
         %63 = OpLoad %6 %62
         %64 = OpConvertFToU %13 %63
        %205 = OpCompositeConstruct %122 %56 %117 %117 %117
         %66 = OpIAdd %13 %64 %65
        %204 = OpCompositeConstruct %114 %56 %117 %56
         %68 = OpAccessChain %67 %61 %14
               OpStore %68 %66
               OpBranch %43
         %69 = OpLabel
        %202 = OpCompositeConstruct %44 %32 %32 %10 %11
        %209 = OpCompositeConstruct %102 %111 %41
         %75 = OpAccessChain %74 %72 %14
               OpStore %75 %73
         %76 = OpAccessChain %74 %72 %14
         %77 = OpLoad %21 %76
         %78 = OpLogicalNot %21 %77
         %79 = OpAccessChain %74 %72 %18
               OpStore %79 %78
         %80 = OpAccessChain %74 %72 %14
         %81 = OpLoad %21 %80
         %82 = OpAccessChain %74 %72 %18
         %83 = OpLoad %21 %82
         %84 = OpLogicalAnd %21 %81 %83
         %85 = OpAccessChain %74 %72 %38
               OpStore %85 %84
         %89 = OpAccessChain %67 %88 %14
         %90 = OpLoad %13 %89
         %91 = OpINotEqual %21 %90 %14
         %92 = OpAccessChain %74 %72 %65
               OpStore %92 %91
               OpBranch %43
         %43 = OpLabel
         %96 = OpLoad %70 %72
         %97 = OpCompositeExtract %21 %96 0
         %98 = OpCompositeExtract %21 %96 1
         %99 = OpCompositeExtract %21 %96 2
        %100 = OpCompositeConstruct %93 %97 %98 %99
        %200 = OpCompositeConstruct %7 %17 %11
               OpStore %95 %100
               OpBranch %24
        %101 = OpLabel
        %105 = OpAccessChain %67 %88 %14
        %106 = OpLoad %13 %105
        %107 = OpINotEqual %21 %106 %14
        %108 = OpCompositeConstruct %102 %107 %107
        %210 = OpCompositeConstruct %93 %108 %73
               OpStore %104 %108
        %211 = OpCompositeConstruct %70 %108 %108
               OpBranch %24
         %24 = OpLabel
        %109 = OpAccessChain %74 %104 %18
        %110 = OpLoad %21 %109
        %112 = OpLogicalOr %21 %110 %111
        %113 = OpAccessChain %74 %104 %14
               OpStore %113 %112
        %118 = OpAccessChain %57 %116 %14
               OpStore %118 %117
        %119 = OpAccessChain %57 %116 %14
        %120 = OpLoad %50 %119
        %121 = OpAccessChain %57 %53 %18
               OpStore %121 %120
        %125 = OpAccessChain %57 %116 %14
        %126 = OpLoad %50 %125
        %127 = OpAccessChain %57 %53 %18
        %203 = OpCompositeConstruct %51 %126 %120
        %128 = OpLoad %50 %127
        %129 = OpIAdd %50 %126 %128
        %130 = OpAccessChain %57 %124 %65
               OpStore %130 %129
        %134 = OpAccessChain %57 %116 %14
        %135 = OpLoad %50 %134
        %136 = OpBitcast %13 %135
        %208 = OpCompositeConstruct %131 %14 %18 %136 %136
        %137 = OpAccessChain %67 %133 %14
               OpStore %137 %136
        %207 = OpCompositeConstruct %59 %14 %18 %136
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationCompositeConstructTest, AddSynonymsForRelevantIds) {
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
          %7 = OpTypeVector %6 3
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstantComposite %7 %10 %10 %10
         %17 = OpTypeVector %6 4
         %18 = OpTypePointer Function %17
         %21 = OpConstant %6 2
         %32 = OpTypeMatrix %17 3
         %33 = OpTypePointer Private %32
         %34 = OpVariable %33 Private
         %35 = OpTypeMatrix %7 4
         %36 = OpTypePointer Private %35
         %37 = OpVariable %36 Private
         %38 = OpTypeVector %6 2
         %39 = OpTypeInt 32 0
         %40 = OpConstant %39 3
         %41 = OpTypeArray %38 %40
         %42 = OpTypePointer Private %41
         %43 = OpVariable %42 Private
        %100 = OpUndef %7
        %101 = OpUndef %17
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %12 = OpVariable %8 Function
         %14 = OpVariable %8 Function
         %19 = OpVariable %18 Function
         %26 = OpVariable %18 Function
         %29 = OpVariable %18 Function
               OpStore %9 %11
         %13 = OpLoad %7 %9
               OpStore %12 %13
         %15 = OpLoad %7 %12
         %16 = OpVectorShuffle %7 %15 %15 2 1 0
               OpStore %14 %16
         %20 = OpLoad %7 %14
         %22 = OpCompositeExtract %6 %20 0
         %23 = OpCompositeExtract %6 %20 1
         %24 = OpCompositeExtract %6 %20 2
         %25 = OpCompositeConstruct %17 %22 %23 %24 %21
               OpStore %19 %25
         %27 = OpLoad %17 %19
         %28 = OpVectorShuffle %17 %27 %27 3 2 1 0
               OpStore %26 %28
         %30 = OpLoad %7 %9
         %31 = OpVectorShuffle %17 %30 %30 0 0 1 1
               OpStore %29 %31
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
  TransformationCompositeConstruct transformation(
      32, {25, 28, 31}, MakeInstructionDescriptor(31, spv::Op::OpReturn, 0),
      200);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(25, {}), MakeDataDescriptor(200, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(28, {}), MakeDataDescriptor(200, {1})));
}

TEST(TransformationCompositeConstructTest, DontAddSynonymsForIrrelevantIds) {
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
          %7 = OpTypeVector %6 3
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 1
         %11 = OpConstantComposite %7 %10 %10 %10
         %17 = OpTypeVector %6 4
         %18 = OpTypePointer Function %17
         %21 = OpConstant %6 2
         %32 = OpTypeMatrix %17 3
         %33 = OpTypePointer Private %32
         %34 = OpVariable %33 Private
         %35 = OpTypeMatrix %7 4
         %36 = OpTypePointer Private %35
         %37 = OpVariable %36 Private
         %38 = OpTypeVector %6 2
         %39 = OpTypeInt 32 0
         %40 = OpConstant %39 3
         %41 = OpTypeArray %38 %40
         %42 = OpTypePointer Private %41
         %43 = OpVariable %42 Private
        %100 = OpUndef %7
        %101 = OpUndef %17
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
         %12 = OpVariable %8 Function
         %14 = OpVariable %8 Function
         %19 = OpVariable %18 Function
         %26 = OpVariable %18 Function
         %29 = OpVariable %18 Function
               OpStore %9 %11
         %13 = OpLoad %7 %9
               OpStore %12 %13
         %15 = OpLoad %7 %12
         %16 = OpVectorShuffle %7 %15 %15 2 1 0
               OpStore %14 %16
         %20 = OpLoad %7 %14
         %22 = OpCompositeExtract %6 %20 0
         %23 = OpCompositeExtract %6 %20 1
         %24 = OpCompositeExtract %6 %20 2
         %25 = OpCompositeConstruct %17 %22 %23 %24 %21
               OpStore %19 %25
         %27 = OpLoad %17 %19
         %28 = OpVectorShuffle %17 %27 %27 3 2 1 0
               OpStore %26 %28
         %30 = OpLoad %7 %9
         %31 = OpVectorShuffle %17 %30 %30 0 0 1 1
               OpStore %29 %31
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
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(25);

  TransformationCompositeConstruct transformation(
      32, {25, 28, 31}, MakeInstructionDescriptor(31, spv::Op::OpReturn, 0),
      200);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(25, {}), MakeDataDescriptor(200, {0})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(28, {}), MakeDataDescriptor(200, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(31, {}), MakeDataDescriptor(200, {2})));
}

TEST(TransformationCompositeConstructTest, DontAddSynonymsInDeadBlock) {
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

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  transformation_context.GetFactManager()->AddFactBlockIsDead(15);

  TransformationCompositeConstruct transformation(
      7, {10, 11}, MakeInstructionDescriptor(15, spv::Op::OpBranch, 0), 100);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {0}), MakeDataDescriptor(10, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {1}), MakeDataDescriptor(11, {})));
}

TEST(TransformationCompositeConstructTest, OneIrrelevantComponent) {
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
          %7 = OpTypeStruct %6 %6 %6
          %8 = OpConstant %6 42
          %9 = OpConstant %6 50
         %10 = OpConstant %6 51
          %4 = OpFunction %2 None %3
          %5 = OpLabel
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
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(8);

  TransformationCompositeConstruct transformation(
      7, {8, 9, 10}, MakeInstructionDescriptor(5, spv::Op::OpReturn, 0), 100);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {0}), MakeDataDescriptor(8, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {1}), MakeDataDescriptor(9, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {2}), MakeDataDescriptor(10, {})));
}

TEST(TransformationCompositeConstructTest, IrrelevantVec2ThenFloat) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
          %8 = OpTypeVector %6 3
          %9 = OpConstant %6 0
         %11 = OpConstant %6 1
         %12 = OpConstant %6 2
         %10 = OpConstantComposite %7 %11 %12
          %4 = OpFunction %2 None %3
          %5 = OpLabel
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

  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(10);

  TransformationCompositeConstruct transformation(
      8, {10, 9}, MakeInstructionDescriptor(5, spv::Op::OpReturn, 0), 100);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {0}), MakeDataDescriptor(10, {0})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {1}), MakeDataDescriptor(10, {1})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {2}), MakeDataDescriptor(9, {})));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(100, {1}), MakeDataDescriptor(9, {})));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
