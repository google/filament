// Copyright (c) 2020 André Perez Maselco
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

#include "source/fuzz/transformation_replace_linear_algebra_instruction.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationReplaceLinearAlgebraInstructionTest, IsApplicable) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %22 "main"
               OpExecutionMode %22 OriginUpperLeft
               OpSource ESSL 310
               OpName %22 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpConstant %4 1
          %9 = OpConstant %4 2
         %10 = OpConstant %4 3
         %11 = OpConstant %4 4
         %12 = OpConstant %4 5
         %13 = OpConstant %4 6
         %14 = OpConstant %4 7
         %15 = OpConstant %4 8
         %16 = OpConstantComposite %5 %8 %9
         %17 = OpConstantComposite %5 %10 %11
         %18 = OpConstantComposite %6 %8 %9 %10
         %19 = OpConstantComposite %6 %11 %12 %13
         %20 = OpConstantComposite %7 %8 %9 %10 %11
         %21 = OpConstantComposite %7 %12 %13 %14 %15
         %22 = OpFunction %2 None %3
         %23 = OpLabel
         %24 = OpDot %4 %16 %17
         %25 = OpDot %4 %18 %19
         %26 = OpDot %4 %20 %21
         %27 = OpVectorTimesScalar %5 %16 %8
         %28 = OpVectorTimesScalar %6 %18 %9
         %29 = OpVectorTimesScalar %7 %20 %10
         %30 = OpCopyObject %4 %24
         %31 = OpFAdd %4 %8 %9
         %32 = OpFMul %4 %10 %11
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
  // Tests linear algebra instructions.
  auto instruction_descriptor =
      MakeInstructionDescriptor(24, spv::Op::OpDot, 0);
  auto transformation = TransformationReplaceLinearAlgebraInstruction(
      {33, 34, 35, 36, 37, 38}, instruction_descriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor =
      MakeInstructionDescriptor(27, spv::Op::OpVectorTimesScalar, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {33, 34, 35, 36}, instruction_descriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests non-linear algebra instructions.
  instruction_descriptor =
      MakeInstructionDescriptor(30, spv::Op::OpCopyObject, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {33, 34, 35, 36, 37, 38}, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor = MakeInstructionDescriptor(31, spv::Op::OpFAdd, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {33, 34, 35, 36, 37}, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor = MakeInstructionDescriptor(32, spv::Op::OpFMul, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {33, 34, 35, 36}, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests number of fresh ids is different than necessary.
  instruction_descriptor = MakeInstructionDescriptor(25, spv::Op::OpDot, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {33, 34, 35, 36}, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor =
      MakeInstructionDescriptor(28, spv::Op::OpVectorTimesScalar, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {33, 34, 35, 36, 37, 38, 39}, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests non-fresh ids.
  instruction_descriptor = MakeInstructionDescriptor(26, spv::Op::OpDot, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {33, 34, 5, 36, 37, 8, 39, 40, 1, 42, 3, 44, 45, 46},
      instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instruction_descriptor =
      MakeInstructionDescriptor(29, spv::Op::OpVectorTimesScalar, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {33, 34, 35, 36, 7, 38, 9, 40}, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationReplaceLinearAlgebraInstructionTest, ReplaceOpTranspose) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %54 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; Constant matrices
         %45 = OpConstantComposite %8 %33 %34
         %46 = OpConstantComposite %9 %33 %34 %35
         %47 = OpConstantComposite %10 %33 %34 %35 %36
         %48 = OpConstantComposite %11 %37 %38
         %49 = OpConstantComposite %12 %37 %38 %39
         %50 = OpConstantComposite %13 %37 %38 %39 %40
         %51 = OpConstantComposite %14 %41 %42
         %52 = OpConstantComposite %15 %41 %42 %43
         %53 = OpConstantComposite %16 %41 %42 %43 %44

; main function
         %54 = OpFunction %2 None %3
         %55 = OpLabel

; Transposing a 2x2 matrix
         %56 = OpTranspose %8 %45

; Transposing a 2x3 matrix
         %57 = OpTranspose %11 %46

; Transposing a 2x4 matrix
         %58 = OpTranspose %14 %47

; Transposing a 3x2 matrix
         %59 = OpTranspose %9 %48

; Transposing a 3x3 matrix
         %60 = OpTranspose %12 %49

; Transposing a 3x4 matrix
         %61 = OpTranspose %15 %50

; Transposing a 4x2 matrix
         %62 = OpTranspose %10 %51

; Transposing a 4x3 matrix
         %63 = OpTranspose %13 %52

; Transposing a 4x4 matrix
         %64 = OpTranspose %16 %53
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto instruction_descriptor =
      MakeInstructionDescriptor(56, spv::Op::OpTranspose, 0);
  auto transformation = TransformationReplaceLinearAlgebraInstruction(
      {65, 66, 67, 68, 69, 70, 71, 72, 73, 74}, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(57, spv::Op::OpTranspose, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(58, spv::Op::OpTranspose, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105,
       106},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(59, spv::Op::OpTranspose, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
       121},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(60, spv::Op::OpTranspose, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132,
       133, 134, 135, 136, 137, 138, 139, 140, 141, 142},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %54 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; Constant matrices
         %45 = OpConstantComposite %8 %33 %34
         %46 = OpConstantComposite %9 %33 %34 %35
         %47 = OpConstantComposite %10 %33 %34 %35 %36
         %48 = OpConstantComposite %11 %37 %38
         %49 = OpConstantComposite %12 %37 %38 %39
         %50 = OpConstantComposite %13 %37 %38 %39 %40
         %51 = OpConstantComposite %14 %41 %42
         %52 = OpConstantComposite %15 %41 %42 %43
         %53 = OpConstantComposite %16 %41 %42 %43 %44

; main function
         %54 = OpFunction %2 None %3
         %55 = OpLabel

; Transposing a 2x2 matrix
         %65 = OpCompositeExtract %5 %45 0
         %66 = OpCompositeExtract %4 %65 0
         %67 = OpCompositeExtract %5 %45 1
         %68 = OpCompositeExtract %4 %67 0
         %69 = OpCompositeConstruct %5 %66 %68
         %70 = OpCompositeExtract %5 %45 0
         %71 = OpCompositeExtract %4 %70 1
         %72 = OpCompositeExtract %5 %45 1
         %73 = OpCompositeExtract %4 %72 1
         %74 = OpCompositeConstruct %5 %71 %73
         %56 = OpCompositeConstruct %8 %69 %74

; Transposing a 2x3 matrix
         %75 = OpCompositeExtract %5 %46 0
         %76 = OpCompositeExtract %4 %75 0
         %77 = OpCompositeExtract %5 %46 1
         %78 = OpCompositeExtract %4 %77 0
         %79 = OpCompositeExtract %5 %46 2
         %80 = OpCompositeExtract %4 %79 0
         %81 = OpCompositeConstruct %6 %76 %78 %80
         %82 = OpCompositeExtract %5 %46 0
         %83 = OpCompositeExtract %4 %82 1
         %84 = OpCompositeExtract %5 %46 1
         %85 = OpCompositeExtract %4 %84 1
         %86 = OpCompositeExtract %5 %46 2
         %87 = OpCompositeExtract %4 %86 1
         %88 = OpCompositeConstruct %6 %83 %85 %87
         %57 = OpCompositeConstruct %11 %81 %88

; Transposing a 2x4 matrix
         %89 = OpCompositeExtract %5 %47 0
         %90 = OpCompositeExtract %4 %89 0
         %91 = OpCompositeExtract %5 %47 1
         %92 = OpCompositeExtract %4 %91 0
         %93 = OpCompositeExtract %5 %47 2
         %94 = OpCompositeExtract %4 %93 0
         %95 = OpCompositeExtract %5 %47 3
         %96 = OpCompositeExtract %4 %95 0
         %97 = OpCompositeConstruct %7 %90 %92 %94 %96
         %98 = OpCompositeExtract %5 %47 0
         %99 = OpCompositeExtract %4 %98 1
        %100 = OpCompositeExtract %5 %47 1
        %101 = OpCompositeExtract %4 %100 1
        %102 = OpCompositeExtract %5 %47 2
        %103 = OpCompositeExtract %4 %102 1
        %104 = OpCompositeExtract %5 %47 3
        %105 = OpCompositeExtract %4 %104 1
        %106 = OpCompositeConstruct %7 %99 %101 %103 %105
         %58 = OpCompositeConstruct %14 %97 %106

; Transposing a 3x2 matrix
        %107 = OpCompositeExtract %6 %48 0
        %108 = OpCompositeExtract %4 %107 0
        %109 = OpCompositeExtract %6 %48 1
        %110 = OpCompositeExtract %4 %109 0
        %111 = OpCompositeConstruct %5 %108 %110
        %112 = OpCompositeExtract %6 %48 0
        %113 = OpCompositeExtract %4 %112 1
        %114 = OpCompositeExtract %6 %48 1
        %115 = OpCompositeExtract %4 %114 1
        %116 = OpCompositeConstruct %5 %113 %115
        %117 = OpCompositeExtract %6 %48 0
        %118 = OpCompositeExtract %4 %117 2
        %119 = OpCompositeExtract %6 %48 1
        %120 = OpCompositeExtract %4 %119 2
        %121 = OpCompositeConstruct %5 %118 %120
         %59 = OpCompositeConstruct %9 %111 %116 %121

; Transposing a 3x3 matrix
        %122 = OpCompositeExtract %6 %49 0
        %123 = OpCompositeExtract %4 %122 0
        %124 = OpCompositeExtract %6 %49 1
        %125 = OpCompositeExtract %4 %124 0
        %126 = OpCompositeExtract %6 %49 2
        %127 = OpCompositeExtract %4 %126 0
        %128 = OpCompositeConstruct %6 %123 %125 %127
        %129 = OpCompositeExtract %6 %49 0
        %130 = OpCompositeExtract %4 %129 1
        %131 = OpCompositeExtract %6 %49 1
        %132 = OpCompositeExtract %4 %131 1
        %133 = OpCompositeExtract %6 %49 2
        %134 = OpCompositeExtract %4 %133 1
        %135 = OpCompositeConstruct %6 %130 %132 %134
        %136 = OpCompositeExtract %6 %49 0
        %137 = OpCompositeExtract %4 %136 2
        %138 = OpCompositeExtract %6 %49 1
        %139 = OpCompositeExtract %4 %138 2
        %140 = OpCompositeExtract %6 %49 2
        %141 = OpCompositeExtract %4 %140 2
        %142 = OpCompositeConstruct %6 %137 %139 %141
         %60 = OpCompositeConstruct %12 %128 %135 %142

; Transposing a 3x4 matrix
         %61 = OpTranspose %15 %50

; Transposing a 4x2 matrix
         %62 = OpTranspose %10 %51

; Transposing a 4x3 matrix
         %63 = OpTranspose %13 %52

; Transposing a 4x4 matrix
         %64 = OpTranspose %16 %53
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

TEST(TransformationReplaceLinearAlgebraInstructionTest,
     ReplaceOpVectorTimesScalar) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %15 "main"
               OpExecutionMode %15 OriginUpperLeft
               OpSource ESSL 310
               OpName %15 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpConstant %4 1
          %9 = OpConstant %4 2
         %10 = OpConstant %4 3
         %11 = OpConstant %4 4
         %12 = OpConstantComposite %5 %8 %9
         %13 = OpConstantComposite %6 %8 %9 %10
         %14 = OpConstantComposite %7 %8 %9 %10 %11
         %15 = OpFunction %2 None %3
         %16 = OpLabel
         %17 = OpVectorTimesScalar %5 %12 %8
         %18 = OpVectorTimesScalar %6 %13 %9
         %19 = OpVectorTimesScalar %7 %14 %10
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto instruction_descriptor =
      MakeInstructionDescriptor(17, spv::Op::OpVectorTimesScalar, 0);
  auto transformation = TransformationReplaceLinearAlgebraInstruction(
      {20, 21, 22, 23}, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(18, spv::Op::OpVectorTimesScalar, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {24, 25, 26, 27, 28, 29}, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(19, spv::Op::OpVectorTimesScalar, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {30, 31, 32, 33, 34, 35, 36, 37}, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %15 "main"
               OpExecutionMode %15 OriginUpperLeft
               OpSource ESSL 310
               OpName %15 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpConstant %4 1
          %9 = OpConstant %4 2
         %10 = OpConstant %4 3
         %11 = OpConstant %4 4
         %12 = OpConstantComposite %5 %8 %9
         %13 = OpConstantComposite %6 %8 %9 %10
         %14 = OpConstantComposite %7 %8 %9 %10 %11
         %15 = OpFunction %2 None %3
         %16 = OpLabel
         %20 = OpCompositeExtract %4 %12 0
         %21 = OpFMul %4 %20 %8
         %22 = OpCompositeExtract %4 %12 1
         %23 = OpFMul %4 %22 %8
         %17 = OpCompositeConstruct %5 %21 %23
         %24 = OpCompositeExtract %4 %13 0
         %25 = OpFMul %4 %24 %9
         %26 = OpCompositeExtract %4 %13 1
         %27 = OpFMul %4 %26 %9
         %28 = OpCompositeExtract %4 %13 2
         %29 = OpFMul %4 %28 %9
         %18 = OpCompositeConstruct %6 %25 %27 %29
         %30 = OpCompositeExtract %4 %14 0
         %31 = OpFMul %4 %30 %10
         %32 = OpCompositeExtract %4 %14 1
         %33 = OpFMul %4 %32 %10
         %34 = OpCompositeExtract %4 %14 2
         %35 = OpFMul %4 %34 %10
         %36 = OpCompositeExtract %4 %14 3
         %37 = OpFMul %4 %36 %10
         %19 = OpCompositeConstruct %7 %31 %33 %35 %37
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

TEST(TransformationReplaceLinearAlgebraInstructionTest,
     ReplaceOpMatrixTimesScalar) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %54 "main"
               OpExecutionMode %54 OriginUpperLeft

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; Constant matrices
         %45 = OpConstantComposite %8 %33 %34
         %46 = OpConstantComposite %9 %33 %34 %35
         %47 = OpConstantComposite %10 %33 %34 %35 %36
         %48 = OpConstantComposite %11 %37 %38
         %49 = OpConstantComposite %12 %37 %38 %39
         %50 = OpConstantComposite %13 %37 %38 %39 %40
         %51 = OpConstantComposite %14 %41 %42
         %52 = OpConstantComposite %15 %41 %42 %43
         %53 = OpConstantComposite %16 %41 %42 %43 %44

; main function
         %54 = OpFunction %2 None %3
         %55 = OpLabel

; Multiplying 2-row matrices by scalar
         %56 = OpMatrixTimesScalar %8 %45 %17
         %57 = OpMatrixTimesScalar %9 %46 %18
         %58 = OpMatrixTimesScalar %10 %47 %19

; Multiplying 3-row matrices by scalar
         %59 = OpMatrixTimesScalar %11 %48 %21
         %60 = OpMatrixTimesScalar %12 %49 %22
         %61 = OpMatrixTimesScalar %13 %50 %23

; Multiplying 4-row matrices by scalar
         %62 = OpMatrixTimesScalar %14 %51 %24
         %63 = OpMatrixTimesScalar %15 %52 %25
         %64 = OpMatrixTimesScalar %16 %53 %26
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto instruction_descriptor =
      MakeInstructionDescriptor(56, spv::Op::OpMatrixTimesScalar, 0);
  auto transformation = TransformationReplaceLinearAlgebraInstruction(
      {65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76}, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(57, spv::Op::OpMatrixTimesScalar, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(58, spv::Op::OpMatrixTimesScalar, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106,
       107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %54 "main"
               OpExecutionMode %54 OriginUpperLeft

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; Constant matrices
         %45 = OpConstantComposite %8 %33 %34
         %46 = OpConstantComposite %9 %33 %34 %35
         %47 = OpConstantComposite %10 %33 %34 %35 %36
         %48 = OpConstantComposite %11 %37 %38
         %49 = OpConstantComposite %12 %37 %38 %39
         %50 = OpConstantComposite %13 %37 %38 %39 %40
         %51 = OpConstantComposite %14 %41 %42
         %52 = OpConstantComposite %15 %41 %42 %43
         %53 = OpConstantComposite %16 %41 %42 %43 %44

; main function
         %54 = OpFunction %2 None %3
         %55 = OpLabel

; Multiplying 2x2 matrix by scalar
         %65 = OpCompositeExtract %5 %45 0
         %66 = OpCompositeExtract %4 %65 0
         %67 = OpFMul %4 %66 %17
         %68 = OpCompositeExtract %4 %65 1
         %69 = OpFMul %4 %68 %17
         %70 = OpCompositeConstruct %5 %67 %69
         %71 = OpCompositeExtract %5 %45 1
         %72 = OpCompositeExtract %4 %71 0
         %73 = OpFMul %4 %72 %17
         %74 = OpCompositeExtract %4 %71 1
         %75 = OpFMul %4 %74 %17
         %76 = OpCompositeConstruct %5 %73 %75
         %56 = OpCompositeConstruct %8 %70 %76

; Multiplying 2x3 matrix by scalar
         %77 = OpCompositeExtract %5 %46 0
         %78 = OpCompositeExtract %4 %77 0
         %79 = OpFMul %4 %78 %18
         %80 = OpCompositeExtract %4 %77 1
         %81 = OpFMul %4 %80 %18
         %82 = OpCompositeConstruct %5 %79 %81
         %83 = OpCompositeExtract %5 %46 1
         %84 = OpCompositeExtract %4 %83 0
         %85 = OpFMul %4 %84 %18
         %86 = OpCompositeExtract %4 %83 1
         %87 = OpFMul %4 %86 %18
         %88 = OpCompositeConstruct %5 %85 %87
         %89 = OpCompositeExtract %5 %46 2
         %90 = OpCompositeExtract %4 %89 0
         %91 = OpFMul %4 %90 %18
         %92 = OpCompositeExtract %4 %89 1
         %93 = OpFMul %4 %92 %18
         %94 = OpCompositeConstruct %5 %91 %93
         %57 = OpCompositeConstruct %9 %82 %88 %94

; Multiplying 2x4 matrix by scalar
         %95 = OpCompositeExtract %5 %47 0
         %96 = OpCompositeExtract %4 %95 0
         %97 = OpFMul %4 %96 %19
         %98 = OpCompositeExtract %4 %95 1
         %99 = OpFMul %4 %98 %19
        %100 = OpCompositeConstruct %5 %97 %99
        %101 = OpCompositeExtract %5 %47 1
        %102 = OpCompositeExtract %4 %101 0
        %103 = OpFMul %4 %102 %19
        %104 = OpCompositeExtract %4 %101 1
        %105 = OpFMul %4 %104 %19
        %106 = OpCompositeConstruct %5 %103 %105
        %107 = OpCompositeExtract %5 %47 2
        %108 = OpCompositeExtract %4 %107 0
        %109 = OpFMul %4 %108 %19
        %110 = OpCompositeExtract %4 %107 1
        %111 = OpFMul %4 %110 %19
        %112 = OpCompositeConstruct %5 %109 %111
        %113 = OpCompositeExtract %5 %47 3
        %114 = OpCompositeExtract %4 %113 0
        %115 = OpFMul %4 %114 %19
        %116 = OpCompositeExtract %4 %113 1
        %117 = OpFMul %4 %116 %19
        %118 = OpCompositeConstruct %5 %115 %117
         %58 = OpCompositeConstruct %10 %100 %106 %112 %118

; Multiplying 3-row matrices by scalar
         %59 = OpMatrixTimesScalar %11 %48 %21
         %60 = OpMatrixTimesScalar %12 %49 %22
         %61 = OpMatrixTimesScalar %13 %50 %23

; Multiplying 4-row matrices by scalar
         %62 = OpMatrixTimesScalar %14 %51 %24
         %63 = OpMatrixTimesScalar %15 %52 %25
         %64 = OpMatrixTimesScalar %16 %53 %26
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

TEST(TransformationReplaceLinearAlgebraInstructionTest,
     ReplaceOpVectorTimesMatrix) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %54 "main"
               OpExecutionMode %54 OriginUpperLeft
               OpSource ESSL 310
               OpName %54 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; Constant matrices
         %45 = OpConstantComposite %8 %33 %34
         %46 = OpConstantComposite %9 %33 %34 %35
         %47 = OpConstantComposite %10 %33 %34 %35 %36
         %48 = OpConstantComposite %11 %37 %38
         %49 = OpConstantComposite %12 %37 %38 %39
         %50 = OpConstantComposite %13 %37 %38 %39 %40
         %51 = OpConstantComposite %14 %41 %42
         %52 = OpConstantComposite %15 %41 %42 %43
         %53 = OpConstantComposite %16 %41 %42 %43 %44

; main function
         %54 = OpFunction %2 None %3
         %55 = OpLabel

; Multiplying 2-dimensional vector by 2x2 matrix
         %56 = OpVectorTimesMatrix %5 %33 %45

; Multiplying 2-dimensional vector by 2x3 matrix
         %57 = OpVectorTimesMatrix %6 %34 %46

; Multiplying 2-dimensional vector by 2x4 matrix
         %58 = OpVectorTimesMatrix %7 %35 %47

; Multiplying 3-dimensional vector by 3x2 matrix
         %59 = OpVectorTimesMatrix %5 %37 %48

; Multiplying 3-dimensional vector by 3x3 matrix
         %60 = OpVectorTimesMatrix %6 %38 %49

; Multiplying 3-dimensional vector by 3x4 matrix
         %61 = OpVectorTimesMatrix %7 %39 %50

; Multiplying 4-dimensional vector by 4x2 matrix
         %62 = OpVectorTimesMatrix %5 %41 %51

; Multiplying 4-dimensional vector by 4x3 matrix
         %63 = OpVectorTimesMatrix %6 %42 %52

; Multiplying 4-dimensional vector by 4x4 matrix
         %64 = OpVectorTimesMatrix %7 %43 %53
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto instruction_descriptor =
      MakeInstructionDescriptor(56, spv::Op::OpVectorTimesMatrix, 0);
  auto transformation = TransformationReplaceLinearAlgebraInstruction(
      {65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(57, spv::Op::OpVectorTimesMatrix, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {79, 80, 81, 82, 83, 84, 85, 86, 87, 88,
       89, 90, 91, 92, 93, 94, 95, 96, 97, 98},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(58, spv::Op::OpVectorTimesMatrix, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
       112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(59, spv::Op::OpVectorTimesMatrix, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135,
       136, 137, 138, 139, 140, 141, 142, 143, 144, 145},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %54 "main"
               OpExecutionMode %54 OriginUpperLeft
               OpSource ESSL 310
               OpName %54 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; Constant matrices
         %45 = OpConstantComposite %8 %33 %34
         %46 = OpConstantComposite %9 %33 %34 %35
         %47 = OpConstantComposite %10 %33 %34 %35 %36
         %48 = OpConstantComposite %11 %37 %38
         %49 = OpConstantComposite %12 %37 %38 %39
         %50 = OpConstantComposite %13 %37 %38 %39 %40
         %51 = OpConstantComposite %14 %41 %42
         %52 = OpConstantComposite %15 %41 %42 %43
         %53 = OpConstantComposite %16 %41 %42 %43 %44

; main function
         %54 = OpFunction %2 None %3
         %55 = OpLabel

; Multiplying 2-dimensional vector by 2x2 matrix
         %65 = OpCompositeExtract %4 %33 0
         %66 = OpCompositeExtract %4 %33 1
         %67 = OpCompositeExtract %5 %45 0
         %68 = OpCompositeExtract %4 %67 0
         %69 = OpFMul %4 %65 %68
         %70 = OpCompositeExtract %4 %67 1
         %71 = OpFMul %4 %66 %70
         %72 = OpFAdd %4 %69 %71
         %73 = OpCompositeExtract %5 %45 1
         %74 = OpCompositeExtract %4 %73 0
         %75 = OpFMul %4 %65 %74
         %76 = OpCompositeExtract %4 %73 1
         %77 = OpFMul %4 %66 %76
         %78 = OpFAdd %4 %75 %77
         %56 = OpCompositeConstruct %5 %72 %78

; Multiplying 2-dimensional vector by 2x3 matrix
         %79 = OpCompositeExtract %4 %34 0
         %80 = OpCompositeExtract %4 %34 1
         %81 = OpCompositeExtract %5 %46 0
         %82 = OpCompositeExtract %4 %81 0
         %83 = OpFMul %4 %79 %82
         %84 = OpCompositeExtract %4 %81 1
         %85 = OpFMul %4 %80 %84
         %86 = OpFAdd %4 %83 %85
         %87 = OpCompositeExtract %5 %46 1
         %88 = OpCompositeExtract %4 %87 0
         %89 = OpFMul %4 %79 %88
         %90 = OpCompositeExtract %4 %87 1
         %91 = OpFMul %4 %80 %90
         %92 = OpFAdd %4 %89 %91
         %93 = OpCompositeExtract %5 %46 2
         %94 = OpCompositeExtract %4 %93 0
         %95 = OpFMul %4 %79 %94
         %96 = OpCompositeExtract %4 %93 1
         %97 = OpFMul %4 %80 %96
         %98 = OpFAdd %4 %95 %97
         %57 = OpCompositeConstruct %6 %86 %92 %98

; Multiplying 2-dimensional vector by 2x4 matrix
         %99 = OpCompositeExtract %4 %35 0
        %100 = OpCompositeExtract %4 %35 1
        %101 = OpCompositeExtract %5 %47 0
        %102 = OpCompositeExtract %4 %101 0
        %103 = OpFMul %4 %99 %102
        %104 = OpCompositeExtract %4 %101 1
        %105 = OpFMul %4 %100 %104
        %106 = OpFAdd %4 %103 %105
        %107 = OpCompositeExtract %5 %47 1
        %108 = OpCompositeExtract %4 %107 0
        %109 = OpFMul %4 %99 %108
        %110 = OpCompositeExtract %4 %107 1
        %111 = OpFMul %4 %100 %110
        %112 = OpFAdd %4 %109 %111
        %113 = OpCompositeExtract %5 %47 2
        %114 = OpCompositeExtract %4 %113 0
        %115 = OpFMul %4 %99 %114
        %116 = OpCompositeExtract %4 %113 1
        %117 = OpFMul %4 %100 %116
        %118 = OpFAdd %4 %115 %117
        %119 = OpCompositeExtract %5 %47 3
        %120 = OpCompositeExtract %4 %119 0
        %121 = OpFMul %4 %99 %120
        %122 = OpCompositeExtract %4 %119 1
        %123 = OpFMul %4 %100 %122
        %124 = OpFAdd %4 %121 %123
         %58 = OpCompositeConstruct %7 %106 %112 %118 %124

; Multiplying 3-dimensional vector by 3x2 matrix
        %125 = OpCompositeExtract %4 %37 0
        %126 = OpCompositeExtract %4 %37 1
        %127 = OpCompositeExtract %4 %37 2
        %128 = OpCompositeExtract %6 %48 0
        %129 = OpCompositeExtract %4 %128 0
        %130 = OpFMul %4 %125 %129
        %131 = OpCompositeExtract %4 %128 1
        %132 = OpFMul %4 %126 %131
        %133 = OpCompositeExtract %4 %128 2
        %134 = OpFMul %4 %127 %133
        %135 = OpFAdd %4 %130 %132
        %136 = OpFAdd %4 %134 %135
        %137 = OpCompositeExtract %6 %48 1
        %138 = OpCompositeExtract %4 %137 0
        %139 = OpFMul %4 %125 %138
        %140 = OpCompositeExtract %4 %137 1
        %141 = OpFMul %4 %126 %140
        %142 = OpCompositeExtract %4 %137 2
        %143 = OpFMul %4 %127 %142
        %144 = OpFAdd %4 %139 %141
        %145 = OpFAdd %4 %143 %144
         %59 = OpCompositeConstruct %5 %136 %145

; Multiplying 3-dimensional vector by 3x3 matrix
         %60 = OpVectorTimesMatrix %6 %38 %49

; Multiplying 3-dimensional vector by 3x4 matrix
         %61 = OpVectorTimesMatrix %7 %39 %50

; Multiplying 4-dimensional vector by 4x2 matrix
         %62 = OpVectorTimesMatrix %5 %41 %51

; Multiplying 4-dimensional vector by 4x3 matrix
         %63 = OpVectorTimesMatrix %6 %42 %52

; Multiplying 4-dimensional vector by 4x4 matrix
         %64 = OpVectorTimesMatrix %7 %43 %53
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

TEST(TransformationReplaceLinearAlgebraInstructionTest,
     ReplaceOpMatrixTimesVector) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %54 "main"
               OpExecutionMode %54 OriginUpperLeft
               OpSource ESSL 310
               OpName %54 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; Constant matrices
         %45 = OpConstantComposite %8 %33 %34
         %46 = OpConstantComposite %9 %33 %34 %35
         %47 = OpConstantComposite %10 %33 %34 %35 %36
         %48 = OpConstantComposite %11 %37 %38
         %49 = OpConstantComposite %12 %37 %38 %39
         %50 = OpConstantComposite %13 %37 %38 %39 %40
         %51 = OpConstantComposite %14 %41 %42
         %52 = OpConstantComposite %15 %41 %42 %43
         %53 = OpConstantComposite %16 %41 %42 %43 %44

; main function
         %54 = OpFunction %2 None %3
         %55 = OpLabel

; Multiplying 2x2 matrix by 2-dimensional vector
         %56 = OpMatrixTimesVector %5 %45 %33

; Multiplying 3x2 matrix by 2-dimensional vector
         %57 = OpMatrixTimesVector %6 %48 %34

; Multiplying 4x2 matrix by 2-dimensional vector
         %58 = OpMatrixTimesVector %7 %51 %35

; Multiplying 2x3 matrix by 3-dimensional vector
         %59 = OpMatrixTimesVector %5 %46 %37

; Multiplying 3x3 matrix by 3-dimensional vector
         %60 = OpMatrixTimesVector %6 %49 %38

; Multiplying 4x3 matrix by 3-dimensional vector
         %61 = OpMatrixTimesVector %7 %52 %39

; Multiplying 2x4 matrix by 4-dimensional vector
         %62 = OpMatrixTimesVector %5 %47 %41

; Multiplying 3x4 matrix by 4-dimensional vector
         %63 = OpMatrixTimesVector %6 %50 %42

; Multiplying 4x4 matrix by 4-dimensional vector
         %64 = OpMatrixTimesVector %7 %53 %43
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto instruction_descriptor =
      MakeInstructionDescriptor(56, spv::Op::OpMatrixTimesVector, 0);
  auto transformation = TransformationReplaceLinearAlgebraInstruction(
      {65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(57, spv::Op::OpMatrixTimesVector, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
       97},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(58, spv::Op::OpMatrixTimesVector, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
       110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(59, spv::Op::OpMatrixTimesVector, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132,
       133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %54 "main"
               OpExecutionMode %54 OriginUpperLeft
               OpSource ESSL 310
               OpName %54 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; Constant matrices
         %45 = OpConstantComposite %8 %33 %34
         %46 = OpConstantComposite %9 %33 %34 %35
         %47 = OpConstantComposite %10 %33 %34 %35 %36
         %48 = OpConstantComposite %11 %37 %38
         %49 = OpConstantComposite %12 %37 %38 %39
         %50 = OpConstantComposite %13 %37 %38 %39 %40
         %51 = OpConstantComposite %14 %41 %42
         %52 = OpConstantComposite %15 %41 %42 %43
         %53 = OpConstantComposite %16 %41 %42 %43 %44

; main function
         %54 = OpFunction %2 None %3
         %55 = OpLabel

; Multiplying 2x2 matrix by 2-dimensional vector
         %65 = OpCompositeExtract %5 %45 0
         %66 = OpCompositeExtract %5 %45 1
         %67 = OpCompositeExtract %4 %33 0
         %68 = OpCompositeExtract %4 %33 1
         %69 = OpCompositeExtract %4 %65 0
         %70 = OpFMul %4 %69 %67
         %71 = OpCompositeExtract %4 %66 0
         %72 = OpFMul %4 %71 %68
         %73 = OpFAdd %4 %70 %72
         %74 = OpCompositeExtract %4 %65 1
         %75 = OpFMul %4 %74 %67
         %76 = OpCompositeExtract %4 %66 1
         %77 = OpFMul %4 %76 %68
         %78 = OpFAdd %4 %75 %77
         %56 = OpCompositeConstruct %5 %73 %78

; Multiplying 3x2 matrix by 2-dimensional vector
         %79 = OpCompositeExtract %6 %48 0
         %80 = OpCompositeExtract %6 %48 1
         %81 = OpCompositeExtract %4 %34 0
         %82 = OpCompositeExtract %4 %34 1
         %83 = OpCompositeExtract %4 %79 0
         %84 = OpFMul %4 %83 %81
         %85 = OpCompositeExtract %4 %80 0
         %86 = OpFMul %4 %85 %82
         %87 = OpFAdd %4 %84 %86
         %88 = OpCompositeExtract %4 %79 1
         %89 = OpFMul %4 %88 %81
         %90 = OpCompositeExtract %4 %80 1
         %91 = OpFMul %4 %90 %82
         %92 = OpFAdd %4 %89 %91
         %93 = OpCompositeExtract %4 %79 2
         %94 = OpFMul %4 %93 %81
         %95 = OpCompositeExtract %4 %80 2
         %96 = OpFMul %4 %95 %82
         %97 = OpFAdd %4 %94 %96
         %57 = OpCompositeConstruct %6 %87 %92 %97

; Multiplying 4x2 matrix by 2-dimensional vector
         %98 = OpCompositeExtract %7 %51 0
         %99 = OpCompositeExtract %7 %51 1
        %100 = OpCompositeExtract %4 %35 0
        %101 = OpCompositeExtract %4 %35 1
        %102 = OpCompositeExtract %4 %98 0
        %103 = OpFMul %4 %102 %100
        %104 = OpCompositeExtract %4 %99 0
        %105 = OpFMul %4 %104 %101
        %106 = OpFAdd %4 %103 %105
        %107 = OpCompositeExtract %4 %98 1
        %108 = OpFMul %4 %107 %100
        %109 = OpCompositeExtract %4 %99 1
        %110 = OpFMul %4 %109 %101
        %111 = OpFAdd %4 %108 %110
        %112 = OpCompositeExtract %4 %98 2
        %113 = OpFMul %4 %112 %100
        %114 = OpCompositeExtract %4 %99 2
        %115 = OpFMul %4 %114 %101
        %116 = OpFAdd %4 %113 %115
        %117 = OpCompositeExtract %4 %98 3
        %118 = OpFMul %4 %117 %100
        %119 = OpCompositeExtract %4 %99 3
        %120 = OpFMul %4 %119 %101
        %121 = OpFAdd %4 %118 %120
         %58 = OpCompositeConstruct %7 %106 %111 %116 %121

; Multiplying 2x3 matrix by 3-dimensional vector
        %122 = OpCompositeExtract %5 %46 0
        %123 = OpCompositeExtract %5 %46 1
        %124 = OpCompositeExtract %5 %46 2
        %125 = OpCompositeExtract %4 %37 0
        %126 = OpCompositeExtract %4 %37 1
        %127 = OpCompositeExtract %4 %37 2
        %128 = OpCompositeExtract %4 %122 0
        %129 = OpFMul %4 %128 %125
        %130 = OpCompositeExtract %4 %123 0
        %131 = OpFMul %4 %130 %126
        %132 = OpCompositeExtract %4 %124 0
        %133 = OpFMul %4 %132 %127
        %134 = OpFAdd %4 %129 %131
        %135 = OpFAdd %4 %133 %134
        %136 = OpCompositeExtract %4 %122 1
        %137 = OpFMul %4 %136 %125
        %138 = OpCompositeExtract %4 %123 1
        %139 = OpFMul %4 %138 %126
        %140 = OpCompositeExtract %4 %124 1
        %141 = OpFMul %4 %140 %127
        %142 = OpFAdd %4 %137 %139
        %143 = OpFAdd %4 %141 %142
         %59 = OpCompositeConstruct %5 %135 %143

; Multiplying 3x3 matrix by 3-dimensional vector
         %60 = OpMatrixTimesVector %6 %49 %38

; Multiplying 4x3 matrix by 3-dimensional vector
         %61 = OpMatrixTimesVector %7 %52 %39

; Multiplying 2x4 matrix by 4-dimensional vector
         %62 = OpMatrixTimesVector %5 %47 %41

; Multiplying 3x4 matrix by 4-dimensional vector
         %63 = OpMatrixTimesVector %6 %50 %42

; Multiplying 4x4 matrix by 4-dimensional vector
         %64 = OpMatrixTimesVector %7 %53 %43
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

TEST(TransformationReplaceLinearAlgebraInstructionTest,
     ReplaceOpMatrixTimesMatrix) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %54 "main"
               OpExecutionMode %54 OriginUpperLeft
               OpSource ESSL 310
               OpName %54 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; Constant matrices
         %45 = OpConstantComposite %8 %33 %34
         %46 = OpConstantComposite %9 %33 %34 %35
         %47 = OpConstantComposite %10 %33 %34 %35 %36
         %48 = OpConstantComposite %11 %37 %38
         %49 = OpConstantComposite %12 %37 %38 %39
         %50 = OpConstantComposite %13 %37 %38 %39 %40
         %51 = OpConstantComposite %14 %41 %42
         %52 = OpConstantComposite %15 %41 %42 %43
         %53 = OpConstantComposite %16 %41 %42 %43 %44

; main function
         %54 = OpFunction %2 None %3
         %55 = OpLabel

; Multiplying 2x2 matrix by 2x2 matrix
         %56 = OpMatrixTimesMatrix %8 %45 %45

; Multiplying 2x2 matrix by 2x3 matrix
         %57 = OpMatrixTimesMatrix %9 %45 %46

; Multiplying 2x2 matrix by 2x4 matrix
         %58 = OpMatrixTimesMatrix %10 %45 %47

; Multiplying 2x3 matrix by 3x2 matrix
         %59 = OpMatrixTimesMatrix %8 %46 %48

; Multiplying 2x3 matrix by 3x3 matrix
         %60 = OpMatrixTimesMatrix %9 %46 %49

; Multiplying 2x3 matrix by 3x4 matrix
         %61 = OpMatrixTimesMatrix %10 %46 %50

; Multiplying 2x4 matrix by 4x2 matrix
         %62 = OpMatrixTimesMatrix %8 %47 %51

; Multiplying 2x4 matrix by 4x3 matrix
         %63 = OpMatrixTimesMatrix %9 %47 %52

; Multiplying 2x4 matrix by 4x4 matrix
         %64 = OpMatrixTimesMatrix %10 %47 %53

; Multiplying 3x2 matrix by 2x2 matrix
         %65 = OpMatrixTimesMatrix %11 %48 %45

; Multiplying 3x2 matrix by 2x3 matrix
         %66 = OpMatrixTimesMatrix %12 %48 %46

; Multiplying 3x2 matrix by 2x4 matrix
         %67 = OpMatrixTimesMatrix %13 %48 %47

; Multiplying 3x3 matrix by 3x2 matrix
         %68 = OpMatrixTimesMatrix %11 %49 %48

; Multiplying 3x3 matrix by 3x3 matrix
         %69 = OpMatrixTimesMatrix %12 %49 %49

; Multiplying 3x3 matrix by 3x4 matrix
         %70 = OpMatrixTimesMatrix %13 %49 %50

; Multiplying 3x4 matrix by 4x2 matrix
         %71 = OpMatrixTimesMatrix %11 %50 %51

; Multiplying 3x4 matrix by 4x3 matrix
         %72 = OpMatrixTimesMatrix %12 %50 %52

; Multiplying 3x4 matrix by 4x4 matrix
         %73 = OpMatrixTimesMatrix %13 %50 %53

; Multiplying 4x2 matrix by 2x2 matrix
         %74 = OpMatrixTimesMatrix %14 %51 %45

; Multiplying 4x2 matrix by 2x3 matrix
         %75 = OpMatrixTimesMatrix %15 %51 %46

; Multiplying 4x2 matrix by 2x4 matrix
         %76 = OpMatrixTimesMatrix %16 %51 %47

; Multiplying 4x3 matrix by 3x2 matrix
         %77 = OpMatrixTimesMatrix %14 %52 %48

; Multiplying 4x3 matrix by 3x3 matrix
         %78 = OpMatrixTimesMatrix %15 %52 %49

; Multiplying 4x3 matrix by 3x4 matrix
         %79 = OpMatrixTimesMatrix %16 %52 %50

; Multiplying 4x4 matrix by 4x2 matrix
         %80 = OpMatrixTimesMatrix %14 %53 %51

; Multiplying 4x4 matrix by 4x3 matrix
         %81 = OpMatrixTimesMatrix %15 %53 %52

; Multiplying 4x4 matrix by 4x4 matrix
         %82 = OpMatrixTimesMatrix %16 %53 %53
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto instruction_descriptor =
      MakeInstructionDescriptor(56, spv::Op::OpMatrixTimesMatrix, 0);
  auto transformation = TransformationReplaceLinearAlgebraInstruction(
      {83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,
       97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
       111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(57, spv::Op::OpMatrixTimesMatrix, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
       135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146,
       147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
       159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170,
       171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(58, spv::Op::OpMatrixTimesMatrix, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196,
       197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
       211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
       225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238,
       239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252,
       253, 254, 255, 256, 257, 258, 259, 260, 261, 262},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %54 "main"
               OpExecutionMode %54 OriginUpperLeft
               OpSource ESSL 310
               OpName %54 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; Constant matrices
         %45 = OpConstantComposite %8 %33 %34
         %46 = OpConstantComposite %9 %33 %34 %35
         %47 = OpConstantComposite %10 %33 %34 %35 %36
         %48 = OpConstantComposite %11 %37 %38
         %49 = OpConstantComposite %12 %37 %38 %39
         %50 = OpConstantComposite %13 %37 %38 %39 %40
         %51 = OpConstantComposite %14 %41 %42
         %52 = OpConstantComposite %15 %41 %42 %43
         %53 = OpConstantComposite %16 %41 %42 %43 %44

; main function
         %54 = OpFunction %2 None %3
         %55 = OpLabel

; Multiplying 2x2 matrix by 2x2 matrix
         %83 = OpCompositeExtract %5 %45 0 ; matrix 2 column 0
         %84 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
         %85 = OpCompositeExtract %4 %84 0 ; matrix 1 row 0 column 0
         %86 = OpCompositeExtract %4 %83 0 ; matrix 2 row 0 column 0
         %87 = OpFMul %4 %85 %86
         %88 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
         %89 = OpCompositeExtract %4 %88 0 ; matrix 1 row 0 column 1
         %90 = OpCompositeExtract %4 %83 1 ; matrix 2 row 1 column 0
         %91 = OpFMul %4 %89 %90
         %92 = OpFAdd %4 %87 %91
         %93 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
         %94 = OpCompositeExtract %4 %93 1 ; matrix 1 row 1 column 0
         %95 = OpCompositeExtract %4 %83 0 ; matrix 2 row 0 column 0
         %96 = OpFMul %4 %94 %95
         %97 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
         %98 = OpCompositeExtract %4 %97 1 ; matrix 1 row 1 column 1
         %99 = OpCompositeExtract %4 %83 1 ; matrix 2 row 1 column 0
        %100 = OpFMul %4 %98 %99
        %101 = OpFAdd %4 %96 %100
        %102 = OpCompositeConstruct %5 %92 %101 ; resulting matrix column 0
        %103 = OpCompositeExtract %5 %45 1 ; matrix 2 column 1
        %104 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %105 = OpCompositeExtract %4 %104 0 ; matrix 1 row 0 column 0
        %106 = OpCompositeExtract %4 %103 0 ; matrix 2 row 0 column 1
        %107 = OpFMul %4 %105 %106
        %108 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %109 = OpCompositeExtract %4 %108 0 ; matrix 1 row 0 column 1
        %110 = OpCompositeExtract %4 %103 1 ; matrix 2 row 1 column 1
        %111 = OpFMul %4 %109 %110
        %112 = OpFAdd %4 %107 %111
        %113 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %114 = OpCompositeExtract %4 %113 1 ; matrix 1 row 1 column 0
        %115 = OpCompositeExtract %4 %103 0 ; matrix 2 row 0 column 1
        %116 = OpFMul %4 %114 %115
        %117 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %118 = OpCompositeExtract %4 %117 1 ; matrix 1 row 1 column 1
        %119 = OpCompositeExtract %4 %103 1 ; matrix 2 row 1 column 1
        %120 = OpFMul %4 %118 %119
        %121 = OpFAdd %4 %116 %120
        %122 = OpCompositeConstruct %5 %112 %121 ; resulting matrix column 1
         %56 = OpCompositeConstruct %8 %102 %122 ; resulting matrix

; Multiplying 2x2 matrix by 2x3 matrix
        %123 = OpCompositeExtract %5 %46 0 ; matrix 2 column 0
        %124 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %125 = OpCompositeExtract %4 %124 0 ; matrix 1 row 0 column 0
        %126 = OpCompositeExtract %4 %123 0 ; matrix 2 row 0 column 0
        %127 = OpFMul %4 %125 %126
        %128 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %129 = OpCompositeExtract %4 %128 0 ; matrix 1 row 0 column 1
        %130 = OpCompositeExtract %4 %123 1 ; matrix 2 row 1 column 0
        %131 = OpFMul %4 %129 %130
        %132 = OpFAdd %4 %127 %131
        %133 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %134 = OpCompositeExtract %4 %133 1 ; matrix 1 row 1 column 0
        %135 = OpCompositeExtract %4 %123 0 ; matrix 2 row 0 column 0
        %136 = OpFMul %4 %134 %135
        %137 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %138 = OpCompositeExtract %4 %137 1 ; matrix 1 row 1 column 1
        %139 = OpCompositeExtract %4 %123 1 ; matrix 2 row 1 column 0
        %140 = OpFMul %4 %138 %139
        %141 = OpFAdd %4 %136 %140
        %142 = OpCompositeConstruct %5 %132 %141 ; resulting matrix column 0
        %143 = OpCompositeExtract %5 %46 1 ; matrix 2 column 1
        %144 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %145 = OpCompositeExtract %4 %144 0 ; matrix 1 row 0 column 0
        %146 = OpCompositeExtract %4 %143 0 ; matrix 2 row 0 column 1
        %147 = OpFMul %4 %145 %146
        %148 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %149 = OpCompositeExtract %4 %148 0 ; matrix 1 row 0 column 1
        %150 = OpCompositeExtract %4 %143 1 ; matrix 2 row 1 column 1
        %151 = OpFMul %4 %149 %150
        %152 = OpFAdd %4 %147 %151
        %153 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %154 = OpCompositeExtract %4 %153 1 ; matrix 1 row 1 column 0
        %155 = OpCompositeExtract %4 %143 0 ; matrix 2 row 0 column 1
        %156 = OpFMul %4 %154 %155
        %157 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %158 = OpCompositeExtract %4 %157 1 ; matrix 1 row 1 column 1
        %159 = OpCompositeExtract %4 %143 1 ; matrix 2 row 1 column 1
        %160 = OpFMul %4 %158 %159
        %161 = OpFAdd %4 %156 %160
        %162 = OpCompositeConstruct %5 %152 %161 ; resulting matrix column 1
        %163 = OpCompositeExtract %5 %46 2 ; matrix 2 column 2
        %164 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %165 = OpCompositeExtract %4 %164 0 ; matrix 1 row 0 column 0
        %166 = OpCompositeExtract %4 %163 0 ; matrix 2 row 0 column 2
        %167 = OpFMul %4 %165 %166
        %168 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %169 = OpCompositeExtract %4 %168 0 ; matrix 1 row 0 column 1
        %170 = OpCompositeExtract %4 %163 1 ; matrix 2 row 1 column 2
        %171 = OpFMul %4 %169 %170
        %172 = OpFAdd %4 %167 %171
        %173 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %174 = OpCompositeExtract %4 %173 1 ; matrix 1 row 1 column 0
        %175 = OpCompositeExtract %4 %163 0 ; matrix 2 row 0 column 2
        %176 = OpFMul %4 %174 %175
        %177 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %178 = OpCompositeExtract %4 %177 1 ; matrix 1 row 1 column 1
        %179 = OpCompositeExtract %4 %163 1 ; matrix 2 row 1 column 2
        %180 = OpFMul %4 %178 %179
        %181 = OpFAdd %4 %176 %180
        %182 = OpCompositeConstruct %5 %172 %181 ; resulting matrix column 2
         %57 = OpCompositeConstruct %9 %142 %162 %182

; Multiplying 2x2 matrix by 2x4 matrix
        %183 = OpCompositeExtract %5 %47 0 ; matrix 2 column 0
        %184 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %185 = OpCompositeExtract %4 %184 0 ; matrix 1 row 0 column 0
        %186 = OpCompositeExtract %4 %183 0 ; matrix 2 row 0 column 0
        %187 = OpFMul %4 %185 %186
        %188 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %189 = OpCompositeExtract %4 %188 0 ; matrix 1 row 0 column 1
        %190 = OpCompositeExtract %4 %183 1 ; matrix 2 row 1 column 0
        %191 = OpFMul %4 %189 %190
        %192 = OpFAdd %4 %187 %191
        %193 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %194 = OpCompositeExtract %4 %193 1 ; matrix 1 row 1 column 0
        %195 = OpCompositeExtract %4 %183 0 ; matrix 2 row 0 column 0
        %196 = OpFMul %4 %194 %195
        %197 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %198 = OpCompositeExtract %4 %197 1 ; matrix 1 row 1 column 1
        %199 = OpCompositeExtract %4 %183 1 ; matrix 2 row 1 column 0
        %200 = OpFMul %4 %198 %199
        %201 = OpFAdd %4 %196 %200
        %202 = OpCompositeConstruct %5 %192 %201 ; resulting matrix column 0
        %203 = OpCompositeExtract %5 %47 1 ; matrix 2 column 1
        %204 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %205 = OpCompositeExtract %4 %204 0 ; matrix 1 row 0 column 0
        %206 = OpCompositeExtract %4 %203 0 ; matrix 2 row 0 column 1
        %207 = OpFMul %4 %205 %206
        %208 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %209 = OpCompositeExtract %4 %208 0 ; matrix 1 row 0 column 1
        %210 = OpCompositeExtract %4 %203 1 ; matrix 2 row 1 column 1
        %211 = OpFMul %4 %209 %210
        %212 = OpFAdd %4 %207 %211
        %213 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %214 = OpCompositeExtract %4 %213 1 ; matrix 1 row 1 column 0
        %215 = OpCompositeExtract %4 %203 0 ; matrix 2 row 0 column 1
        %216 = OpFMul %4 %214 %215
        %217 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %218 = OpCompositeExtract %4 %217 1 ; matrix 1 row 1 column 1
        %219 = OpCompositeExtract %4 %203 1 ; matrix 2 row 1 column 1
        %220 = OpFMul %4 %218 %219
        %221 = OpFAdd %4 %216 %220
        %222 = OpCompositeConstruct %5 %212 %221 ; resulting matrix column 1
        %223 = OpCompositeExtract %5 %47 2 ; matrix 2 column 2
        %224 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %225 = OpCompositeExtract %4 %224 0 ; matrix 1 row 0 column 0
        %226 = OpCompositeExtract %4 %223 0 ; matrix 2 row 0 column 2
        %227 = OpFMul %4 %225 %226
        %228 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %229 = OpCompositeExtract %4 %228 0 ; matrix 1 row 0 column 1
        %230 = OpCompositeExtract %4 %223 1 ; matrix 2 row 1 column 2
        %231 = OpFMul %4 %229 %230
        %232 = OpFAdd %4 %227 %231
        %233 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %234 = OpCompositeExtract %4 %233 1 ; matrix 1 row 1 column 0
        %235 = OpCompositeExtract %4 %223 0 ; matrix 2 row 0 column 2
        %236 = OpFMul %4 %234 %235
        %237 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %238 = OpCompositeExtract %4 %237 1 ; matrix 1 row 1 column 1
        %239 = OpCompositeExtract %4 %223 1 ; matrix 2 row 1 column 2
        %240 = OpFMul %4 %238 %239
        %241 = OpFAdd %4 %236 %240
        %242 = OpCompositeConstruct %5 %232 %241 ; resulting matrix column 2
        %243 = OpCompositeExtract %5 %47 3 ; matrix 2 column 3
        %244 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %245 = OpCompositeExtract %4 %244 0 ; matrix 1 row 0 column 0
        %246 = OpCompositeExtract %4 %243 0 ; matrix 2 row 0 column 3
        %247 = OpFMul %4 %245 %246
        %248 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %249 = OpCompositeExtract %4 %248 0 ; matrix 1 row 0 column 1
        %250 = OpCompositeExtract %4 %243 1 ; matrix 2 row 1 column 3
        %251 = OpFMul %4 %249 %250
        %252 = OpFAdd %4 %247 %251
        %253 = OpCompositeExtract %5 %45 0 ; matrix 1 column 0
        %254 = OpCompositeExtract %4 %253 1 ; matrix 1 row 1 column 0
        %255 = OpCompositeExtract %4 %243 0 ; matrix 2 row 0 column 3
        %256 = OpFMul %4 %254 %255
        %257 = OpCompositeExtract %5 %45 1 ; matrix 1 column 1
        %258 = OpCompositeExtract %4 %257 1 ; matrix 1 row 1 column 1
        %259 = OpCompositeExtract %4 %243 1 ; matrix 2 row 1 column 3
        %260 = OpFMul %4 %258 %259
        %261 = OpFAdd %4 %256 %260
        %262 = OpCompositeConstruct %5 %252 %261 ; resulting matrix column 3
         %58 = OpCompositeConstruct %10 %202 %222 %242 %262

; Multiplying 2x3 matrix by 3x2 matrix
         %59 = OpMatrixTimesMatrix %8 %46 %48

; Multiplying 2x3 matrix by 3x3 matrix
         %60 = OpMatrixTimesMatrix %9 %46 %49

; Multiplying 2x3 matrix by 3x4 matrix
         %61 = OpMatrixTimesMatrix %10 %46 %50

; Multiplying 2x4 matrix by 4x2 matrix
         %62 = OpMatrixTimesMatrix %8 %47 %51

; Multiplying 2x4 matrix by 4x3 matrix
         %63 = OpMatrixTimesMatrix %9 %47 %52

; Multiplying 2x4 matrix by 4x4 matrix
         %64 = OpMatrixTimesMatrix %10 %47 %53

; Multiplying 3x2 matrix by 2x2 matrix
         %65 = OpMatrixTimesMatrix %11 %48 %45

; Multiplying 3x2 matrix by 2x3 matrix
         %66 = OpMatrixTimesMatrix %12 %48 %46

; Multiplying 3x2 matrix by 2x4 matrix
         %67 = OpMatrixTimesMatrix %13 %48 %47

; Multiplying 3x3 matrix by 3x2 matrix
         %68 = OpMatrixTimesMatrix %11 %49 %48

; Multiplying 3x3 matrix by 3x3 matrix
         %69 = OpMatrixTimesMatrix %12 %49 %49

; Multiplying 3x3 matrix by 3x4 matrix
         %70 = OpMatrixTimesMatrix %13 %49 %50

; Multiplying 3x4 matrix by 4x2 matrix
         %71 = OpMatrixTimesMatrix %11 %50 %51

; Multiplying 3x4 matrix by 4x3 matrix
         %72 = OpMatrixTimesMatrix %12 %50 %52

; Multiplying 3x4 matrix by 4x4 matrix
         %73 = OpMatrixTimesMatrix %13 %50 %53

; Multiplying 4x2 matrix by 2x2 matrix
         %74 = OpMatrixTimesMatrix %14 %51 %45

; Multiplying 4x2 matrix by 2x3 matrix
         %75 = OpMatrixTimesMatrix %15 %51 %46

; Multiplying 4x2 matrix by 2x4 matrix
         %76 = OpMatrixTimesMatrix %16 %51 %47

; Multiplying 4x3 matrix by 3x2 matrix
         %77 = OpMatrixTimesMatrix %14 %52 %48

; Multiplying 4x3 matrix by 3x3 matrix
         %78 = OpMatrixTimesMatrix %15 %52 %49

; Multiplying 4x3 matrix by 3x4 matrix
         %79 = OpMatrixTimesMatrix %16 %52 %50

; Multiplying 4x4 matrix by 4x2 matrix
         %80 = OpMatrixTimesMatrix %14 %53 %51

; Multiplying 4x4 matrix by 4x3 matrix
         %81 = OpMatrixTimesMatrix %15 %53 %52

; Multiplying 4x4 matrix by 4x4 matrix
         %82 = OpMatrixTimesMatrix %16 %53 %53
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

TEST(TransformationReplaceLinearAlgebraInstructionTest, ReplaceOpOuterProduct) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %45 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; main function
         %45 = OpFunction %2 None %3
         %46 = OpLabel

; Multiplying 2-dimensional vector by 2-dimensional vector
         %47 = OpOuterProduct %8 %33 %34

; Multiplying 2-dimensional vector by 3-dimensional vector
         %48 = OpOuterProduct %9 %35 %37

; Multiplying 2-dimensional vector by 4-dimensional vector
         %49 = OpOuterProduct %10 %36 %41

; Multiplying 3-dimensional vector by 2-dimensional vector
         %50 = OpOuterProduct %11 %37 %33

; Multiplying 3-dimensional vector by 3-dimensional vector
         %51 = OpOuterProduct %12 %38 %39

; Multiplying 3-dimensional vector by 4-dimensional vector
         %52 = OpOuterProduct %13 %40 %41

; Multiplying 4-dimensional vector by 2-dimensional vector
         %53 = OpOuterProduct %14 %41 %33

; Multiplying 4-dimensional vector by 3-dimensional vector
         %54 = OpOuterProduct %15 %42 %37

; Multiplying 4-dimensional vector by 4-dimensional vector
         %55 = OpOuterProduct %16 %43 %44
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto instruction_descriptor =
      MakeInstructionDescriptor(47, spv::Op::OpOuterProduct, 0);
  auto transformation = TransformationReplaceLinearAlgebraInstruction(
      {56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67}, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(48, spv::Op::OpOuterProduct, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(49, spv::Op::OpOuterProduct, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {86, 87, 88,  89,  90,  91,  92,  93,  94,  95,  96,  97,
       98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor =
      MakeInstructionDescriptor(50, spv::Op::OpOuterProduct, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123,
       124, 125},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %45 "main"

; Types
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpTypeMatrix %5 2
          %9 = OpTypeMatrix %5 3
         %10 = OpTypeMatrix %5 4
         %11 = OpTypeMatrix %6 2
         %12 = OpTypeMatrix %6 3
         %13 = OpTypeMatrix %6 4
         %14 = OpTypeMatrix %7 2
         %15 = OpTypeMatrix %7 3
         %16 = OpTypeMatrix %7 4

; Constant scalars
         %17 = OpConstant %4 1
         %18 = OpConstant %4 2
         %19 = OpConstant %4 3
         %20 = OpConstant %4 4
         %21 = OpConstant %4 5
         %22 = OpConstant %4 6
         %23 = OpConstant %4 7
         %24 = OpConstant %4 8
         %25 = OpConstant %4 9
         %26 = OpConstant %4 10
         %27 = OpConstant %4 11
         %28 = OpConstant %4 12
         %29 = OpConstant %4 13
         %30 = OpConstant %4 14
         %31 = OpConstant %4 15
         %32 = OpConstant %4 16

; Constant vectors
         %33 = OpConstantComposite %5 %17 %18
         %34 = OpConstantComposite %5 %19 %20
         %35 = OpConstantComposite %5 %21 %22
         %36 = OpConstantComposite %5 %23 %24
         %37 = OpConstantComposite %6 %17 %18 %19
         %38 = OpConstantComposite %6 %20 %21 %22
         %39 = OpConstantComposite %6 %23 %24 %25
         %40 = OpConstantComposite %6 %26 %27 %28
         %41 = OpConstantComposite %7 %17 %18 %19 %20
         %42 = OpConstantComposite %7 %21 %22 %23 %24
         %43 = OpConstantComposite %7 %25 %26 %27 %28
         %44 = OpConstantComposite %7 %29 %30 %31 %32

; main function
         %45 = OpFunction %2 None %3
         %46 = OpLabel

; Multiplying 2-dimensional vector by 2-dimensional vector
         %56 = OpCompositeExtract %4 %34 0
         %57 = OpCompositeExtract %4 %33 0
         %58 = OpFMul %4 %56 %57
         %59 = OpCompositeExtract %4 %33 1
         %60 = OpFMul %4 %56 %59
         %61 = OpCompositeConstruct %5 %58 %60
         %62 = OpCompositeExtract %4 %34 1
         %63 = OpCompositeExtract %4 %33 0
         %64 = OpFMul %4 %62 %63
         %65 = OpCompositeExtract %4 %33 1
         %66 = OpFMul %4 %62 %65
         %67 = OpCompositeConstruct %5 %64 %66
         %47 = OpCompositeConstruct %8 %61 %67

; Multiplying 2-dimensional vector by 3-dimensional vector
         %68 = OpCompositeExtract %4 %37 0
         %69 = OpCompositeExtract %4 %35 0
         %70 = OpFMul %4 %68 %69
         %71 = OpCompositeExtract %4 %35 1
         %72 = OpFMul %4 %68 %71
         %73 = OpCompositeConstruct %5 %70 %72
         %74 = OpCompositeExtract %4 %37 1
         %75 = OpCompositeExtract %4 %35 0
         %76 = OpFMul %4 %74 %75
         %77 = OpCompositeExtract %4 %35 1
         %78 = OpFMul %4 %74 %77
         %79 = OpCompositeConstruct %5 %76 %78
         %80 = OpCompositeExtract %4 %37 2
         %81 = OpCompositeExtract %4 %35 0
         %82 = OpFMul %4 %80 %81
         %83 = OpCompositeExtract %4 %35 1
         %84 = OpFMul %4 %80 %83
         %85 = OpCompositeConstruct %5 %82 %84
         %48 = OpCompositeConstruct %9 %73 %79 %85

; Multiplying 2-dimensional vector by 4-dimensional vector
         %86 = OpCompositeExtract %4 %41 0
         %87 = OpCompositeExtract %4 %36 0
         %88 = OpFMul %4 %86 %87
         %89 = OpCompositeExtract %4 %36 1
         %90 = OpFMul %4 %86 %89
         %91 = OpCompositeConstruct %5 %88 %90
         %92 = OpCompositeExtract %4 %41 1
         %93 = OpCompositeExtract %4 %36 0
         %94 = OpFMul %4 %92 %93
         %95 = OpCompositeExtract %4 %36 1
         %96 = OpFMul %4 %92 %95
         %97 = OpCompositeConstruct %5 %94 %96
         %98 = OpCompositeExtract %4 %41 2
         %99 = OpCompositeExtract %4 %36 0
        %100 = OpFMul %4 %98 %99
        %101 = OpCompositeExtract %4 %36 1
        %102 = OpFMul %4 %98 %101
        %103 = OpCompositeConstruct %5 %100 %102
        %104 = OpCompositeExtract %4 %41 3
        %105 = OpCompositeExtract %4 %36 0
        %106 = OpFMul %4 %104 %105
        %107 = OpCompositeExtract %4 %36 1
        %108 = OpFMul %4 %104 %107
        %109 = OpCompositeConstruct %5 %106 %108
         %49 = OpCompositeConstruct %10 %91 %97 %103 %109

; Multiplying 3-dimensional vector by 2-dimensional vector
        %110 = OpCompositeExtract %4 %33 0
        %111 = OpCompositeExtract %4 %37 0
        %112 = OpFMul %4 %110 %111
        %113 = OpCompositeExtract %4 %37 1
        %114 = OpFMul %4 %110 %113
        %115 = OpCompositeExtract %4 %37 2
        %116 = OpFMul %4 %110 %115
        %117 = OpCompositeConstruct %6 %112 %114 %116
        %118 = OpCompositeExtract %4 %33 1
        %119 = OpCompositeExtract %4 %37 0
        %120 = OpFMul %4 %118 %119
        %121 = OpCompositeExtract %4 %37 1
        %122 = OpFMul %4 %118 %121
        %123 = OpCompositeExtract %4 %37 2
        %124 = OpFMul %4 %118 %123
        %125 = OpCompositeConstruct %6 %120 %122 %124
         %50 = OpCompositeConstruct %11 %117 %125

; Multiplying 3-dimensional vector by 3-dimensional vector
         %51 = OpOuterProduct %12 %38 %39

; Multiplying 3-dimensional vector by 4-dimensional vector
         %52 = OpOuterProduct %13 %40 %41

; Multiplying 4-dimensional vector by 2-dimensional vector
         %53 = OpOuterProduct %14 %41 %33

; Multiplying 4-dimensional vector by 3-dimensional vector
         %54 = OpOuterProduct %15 %42 %37

; Multiplying 4-dimensional vector by 4-dimensional vector
         %55 = OpOuterProduct %16 %43 %44
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

TEST(TransformationReplaceLinearAlgebraInstructionTest, ReplaceOpDot) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %22 "main"
               OpExecutionMode %22 OriginUpperLeft
               OpSource ESSL 310
               OpName %22 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpConstant %4 1
          %9 = OpConstant %4 2
         %10 = OpConstant %4 3
         %11 = OpConstant %4 4
         %12 = OpConstant %4 5
         %13 = OpConstant %4 6
         %14 = OpConstant %4 7
         %15 = OpConstant %4 8
         %16 = OpConstantComposite %5 %8 %9
         %17 = OpConstantComposite %5 %10 %11
         %18 = OpConstantComposite %6 %8 %9 %10
         %19 = OpConstantComposite %6 %11 %12 %13
         %20 = OpConstantComposite %7 %8 %9 %10 %11
         %21 = OpConstantComposite %7 %12 %13 %14 %15
         %22 = OpFunction %2 None %3
         %23 = OpLabel
         %24 = OpDot %4 %16 %17
         %25 = OpDot %4 %18 %19
         %26 = OpDot %4 %20 %21
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_5;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  auto instruction_descriptor =
      MakeInstructionDescriptor(24, spv::Op::OpDot, 0);
  auto transformation = TransformationReplaceLinearAlgebraInstruction(
      {27, 28, 29, 30, 31, 32}, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor = MakeInstructionDescriptor(25, spv::Op::OpDot, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {33, 34, 35, 36, 37, 38, 39, 40, 41, 42}, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instruction_descriptor = MakeInstructionDescriptor(26, spv::Op::OpDot, 0);
  transformation = TransformationReplaceLinearAlgebraInstruction(
      {43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56},
      instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %22 "main"
               OpExecutionMode %22 OriginUpperLeft
               OpSource ESSL 310
               OpName %22 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeFloat 32
          %5 = OpTypeVector %4 2
          %6 = OpTypeVector %4 3
          %7 = OpTypeVector %4 4
          %8 = OpConstant %4 1
          %9 = OpConstant %4 2
         %10 = OpConstant %4 3
         %11 = OpConstant %4 4
         %12 = OpConstant %4 5
         %13 = OpConstant %4 6
         %14 = OpConstant %4 7
         %15 = OpConstant %4 8
         %16 = OpConstantComposite %5 %8 %9
         %17 = OpConstantComposite %5 %10 %11
         %18 = OpConstantComposite %6 %8 %9 %10
         %19 = OpConstantComposite %6 %11 %12 %13
         %20 = OpConstantComposite %7 %8 %9 %10 %11
         %21 = OpConstantComposite %7 %12 %13 %14 %15
         %22 = OpFunction %2 None %3
         %23 = OpLabel
         %27 = OpCompositeExtract %4 %16 0
         %28 = OpCompositeExtract %4 %17 0
         %29 = OpFMul %4 %27 %28
         %30 = OpCompositeExtract %4 %16 1
         %31 = OpCompositeExtract %4 %17 1
         %32 = OpFMul %4 %30 %31
         %24 = OpFAdd %4 %29 %32
         %33 = OpCompositeExtract %4 %18 0
         %34 = OpCompositeExtract %4 %19 0
         %35 = OpFMul %4 %33 %34
         %36 = OpCompositeExtract %4 %18 1
         %37 = OpCompositeExtract %4 %19 1
         %38 = OpFMul %4 %36 %37
         %39 = OpCompositeExtract %4 %18 2
         %40 = OpCompositeExtract %4 %19 2
         %41 = OpFMul %4 %39 %40
         %42 = OpFAdd %4 %35 %38
         %25 = OpFAdd %4 %41 %42
         %43 = OpCompositeExtract %4 %20 0
         %44 = OpCompositeExtract %4 %21 0
         %45 = OpFMul %4 %43 %44
         %46 = OpCompositeExtract %4 %20 1
         %47 = OpCompositeExtract %4 %21 1
         %48 = OpFMul %4 %46 %47
         %49 = OpCompositeExtract %4 %20 2
         %50 = OpCompositeExtract %4 %21 2
         %51 = OpFMul %4 %49 %50
         %52 = OpCompositeExtract %4 %20 3
         %53 = OpCompositeExtract %4 %21 3
         %54 = OpFMul %4 %52 %53
         %55 = OpFAdd %4 %45 %48
         %56 = OpFAdd %4 %51 %55
         %26 = OpFAdd %4 %54 %56
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
