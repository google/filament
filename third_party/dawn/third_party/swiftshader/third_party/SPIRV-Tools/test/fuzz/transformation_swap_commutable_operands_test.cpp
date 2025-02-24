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

#include "source/fuzz/transformation_swap_commutable_operands.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationSwapCommutableOperandsTest, IsApplicableTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 2
          %9 = OpTypeArray %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 1
         %13 = OpConstant %6 2
         %14 = OpConstantComposite %9 %12 %13
         %15 = OpTypePointer Function %6
         %17 = OpConstant %6 0
         %29 = OpTypeFloat 32
         %30 = OpTypeArray %29 %8
         %31 = OpTypePointer Function %30
         %33 = OpConstant %29 1
         %34 = OpConstant %29 2
         %35 = OpConstantComposite %30 %33 %34
         %36 = OpTypePointer Function %29
         %49 = OpTypeVector %29 3
         %50 = OpTypeArray %49 %8
         %51 = OpTypePointer Function %50
         %53 = OpConstant %29 3
         %54 = OpConstantComposite %49 %33 %34 %53
         %55 = OpConstant %29 4
         %56 = OpConstant %29 5
         %57 = OpConstant %29 6
         %58 = OpConstantComposite %49 %55 %56 %57
         %59 = OpConstantComposite %50 %54 %58
         %61 = OpTypePointer Function %49
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %16 = OpVariable %15 Function
         %23 = OpVariable %15 Function
         %32 = OpVariable %31 Function
         %37 = OpVariable %36 Function
         %43 = OpVariable %36 Function
         %52 = OpVariable %51 Function
         %60 = OpVariable %36 Function
               OpStore %11 %14
         %18 = OpAccessChain %15 %11 %17
         %19 = OpLoad %6 %18
         %20 = OpAccessChain %15 %11 %12
         %21 = OpLoad %6 %20
         %22 = OpIAdd %6 %19 %21
               OpStore %16 %22
         %24 = OpAccessChain %15 %11 %17
         %25 = OpLoad %6 %24
         %26 = OpAccessChain %15 %11 %12
         %27 = OpLoad %6 %26
         %28 = OpIMul %6 %25 %27
               OpStore %23 %28
               OpStore %32 %35
         %38 = OpAccessChain %36 %32 %17
         %39 = OpLoad %29 %38
         %40 = OpAccessChain %36 %32 %12
         %41 = OpLoad %29 %40
         %42 = OpFAdd %29 %39 %41
               OpStore %37 %42
         %44 = OpAccessChain %36 %32 %17
         %45 = OpLoad %29 %44
         %46 = OpAccessChain %36 %32 %12
         %47 = OpLoad %29 %46
         %48 = OpFMul %29 %45 %47
               OpStore %43 %48
               OpStore %52 %59
         %62 = OpAccessChain %61 %52 %17
         %63 = OpLoad %49 %62
         %64 = OpAccessChain %61 %52 %12
         %65 = OpLoad %49 %64
         %66 = OpDot %29 %63 %65
               OpStore %60 %66
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
  // Tests existing commutative instructions
  auto instructionDescriptor =
      MakeInstructionDescriptor(22, spv::Op::OpIAdd, 0);
  auto transformation =
      TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(28, spv::Op::OpIMul, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(42, spv::Op::OpFAdd, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(48, spv::Op::OpFMul, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(66, spv::Op::OpDot, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests existing non-commutative instructions
  instructionDescriptor =
      MakeInstructionDescriptor(1, spv::Op::OpExtInstImport, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(5, spv::Op::OpLabel, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(8, spv::Op::OpConstant, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(11, spv::Op::OpVariable, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor =
      MakeInstructionDescriptor(14, spv::Op::OpConstantComposite, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests the base instruction id not existing
  instructionDescriptor =
      MakeInstructionDescriptor(67, spv::Op::OpIAddCarry, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(68, spv::Op::OpIEqual, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor =
      MakeInstructionDescriptor(69, spv::Op::OpINotEqual, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor =
      MakeInstructionDescriptor(70, spv::Op::OpFOrdEqual, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(71, spv::Op::OpPtrEqual, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests there being no instruction with the desired opcode after the base
  // instruction id
  instructionDescriptor = MakeInstructionDescriptor(24, spv::Op::OpIAdd, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(38, spv::Op::OpIMul, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(45, spv::Op::OpFAdd, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(66, spv::Op::OpFMul, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests there being an instruction with the desired opcode after the base
  // instruction id, but the skip count associated with the instruction
  // descriptor being so high.
  instructionDescriptor = MakeInstructionDescriptor(11, spv::Op::OpIAdd, 100);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(16, spv::Op::OpIMul, 100);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(23, spv::Op::OpFAdd, 100);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(32, spv::Op::OpFMul, 100);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  instructionDescriptor = MakeInstructionDescriptor(37, spv::Op::OpDot, 100);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationSwapCommutableOperandsTest, ApplyTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 2
          %9 = OpTypeArray %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 1
         %13 = OpConstant %6 2
         %14 = OpConstantComposite %9 %12 %13
         %15 = OpTypePointer Function %6
         %17 = OpConstant %6 0
         %29 = OpTypeFloat 32
         %30 = OpTypeArray %29 %8
         %31 = OpTypePointer Function %30
         %33 = OpConstant %29 1
         %34 = OpConstant %29 2
         %35 = OpConstantComposite %30 %33 %34
         %36 = OpTypePointer Function %29
         %49 = OpTypeVector %29 3
         %50 = OpTypeArray %49 %8
         %51 = OpTypePointer Function %50
         %53 = OpConstant %29 3
         %54 = OpConstantComposite %49 %33 %34 %53
         %55 = OpConstant %29 4
         %56 = OpConstant %29 5
         %57 = OpConstant %29 6
         %58 = OpConstantComposite %49 %55 %56 %57
         %59 = OpConstantComposite %50 %54 %58
         %61 = OpTypePointer Function %49
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %16 = OpVariable %15 Function
         %23 = OpVariable %15 Function
         %32 = OpVariable %31 Function
         %37 = OpVariable %36 Function
         %43 = OpVariable %36 Function
         %52 = OpVariable %51 Function
         %60 = OpVariable %36 Function
               OpStore %11 %14
         %18 = OpAccessChain %15 %11 %17
         %19 = OpLoad %6 %18
         %20 = OpAccessChain %15 %11 %12
         %21 = OpLoad %6 %20
         %22 = OpIAdd %6 %19 %21
               OpStore %16 %22
         %24 = OpAccessChain %15 %11 %17
         %25 = OpLoad %6 %24
         %26 = OpAccessChain %15 %11 %12
         %27 = OpLoad %6 %26
         %28 = OpIMul %6 %25 %27
               OpStore %23 %28
               OpStore %32 %35
         %38 = OpAccessChain %36 %32 %17
         %39 = OpLoad %29 %38
         %40 = OpAccessChain %36 %32 %12
         %41 = OpLoad %29 %40
         %42 = OpFAdd %29 %39 %41
               OpStore %37 %42
         %44 = OpAccessChain %36 %32 %17
         %45 = OpLoad %29 %44
         %46 = OpAccessChain %36 %32 %12
         %47 = OpLoad %29 %46
         %48 = OpFMul %29 %45 %47
               OpStore %43 %48
               OpStore %52 %59
         %62 = OpAccessChain %61 %52 %17
         %63 = OpLoad %49 %62
         %64 = OpAccessChain %61 %52 %12
         %65 = OpLoad %49 %64
         %66 = OpDot %29 %63 %65
               OpStore %60 %66
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
  auto instructionDescriptor =
      MakeInstructionDescriptor(22, spv::Op::OpIAdd, 0);
  auto transformation =
      TransformationSwapCommutableOperands(instructionDescriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instructionDescriptor = MakeInstructionDescriptor(28, spv::Op::OpIMul, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instructionDescriptor = MakeInstructionDescriptor(42, spv::Op::OpFAdd, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instructionDescriptor = MakeInstructionDescriptor(48, spv::Op::OpFMul, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  instructionDescriptor = MakeInstructionDescriptor(66, spv::Op::OpDot, 0);
  transformation = TransformationSwapCommutableOperands(instructionDescriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variantShader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 2
          %9 = OpTypeArray %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 1
         %13 = OpConstant %6 2
         %14 = OpConstantComposite %9 %12 %13
         %15 = OpTypePointer Function %6
         %17 = OpConstant %6 0
         %29 = OpTypeFloat 32
         %30 = OpTypeArray %29 %8
         %31 = OpTypePointer Function %30
         %33 = OpConstant %29 1
         %34 = OpConstant %29 2
         %35 = OpConstantComposite %30 %33 %34
         %36 = OpTypePointer Function %29
         %49 = OpTypeVector %29 3
         %50 = OpTypeArray %49 %8
         %51 = OpTypePointer Function %50
         %53 = OpConstant %29 3
         %54 = OpConstantComposite %49 %33 %34 %53
         %55 = OpConstant %29 4
         %56 = OpConstant %29 5
         %57 = OpConstant %29 6
         %58 = OpConstantComposite %49 %55 %56 %57
         %59 = OpConstantComposite %50 %54 %58
         %61 = OpTypePointer Function %49
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %16 = OpVariable %15 Function
         %23 = OpVariable %15 Function
         %32 = OpVariable %31 Function
         %37 = OpVariable %36 Function
         %43 = OpVariable %36 Function
         %52 = OpVariable %51 Function
         %60 = OpVariable %36 Function
               OpStore %11 %14
         %18 = OpAccessChain %15 %11 %17
         %19 = OpLoad %6 %18
         %20 = OpAccessChain %15 %11 %12
         %21 = OpLoad %6 %20
         %22 = OpIAdd %6 %21 %19
               OpStore %16 %22
         %24 = OpAccessChain %15 %11 %17
         %25 = OpLoad %6 %24
         %26 = OpAccessChain %15 %11 %12
         %27 = OpLoad %6 %26
         %28 = OpIMul %6 %27 %25
               OpStore %23 %28
               OpStore %32 %35
         %38 = OpAccessChain %36 %32 %17
         %39 = OpLoad %29 %38
         %40 = OpAccessChain %36 %32 %12
         %41 = OpLoad %29 %40
         %42 = OpFAdd %29 %41 %39
               OpStore %37 %42
         %44 = OpAccessChain %36 %32 %17
         %45 = OpLoad %29 %44
         %46 = OpAccessChain %36 %32 %12
         %47 = OpLoad %29 %46
         %48 = OpFMul %29 %47 %45
               OpStore %43 %48
               OpStore %52 %59
         %62 = OpAccessChain %61 %52 %17
         %63 = OpLoad %49 %62
         %64 = OpAccessChain %61 %52 %12
         %65 = OpLoad %49 %64
         %66 = OpDot %29 %65 %63
               OpStore %60 %66
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, variantShader, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
