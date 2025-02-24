// Copyright (c) 2020 Vasyl Teliman
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

#include "source/fuzz/transformation_mutate_pointer.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationMutatePointerTest, BasicTest) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
         %34 = OpConstant %7 0
         %36 = OpConstant %6 0
         %14 = OpTypeVector %7 3
         %35 = OpConstantComposite %14 %34 %34 %34
         %15 = OpTypeMatrix %14 2
          %8 = OpConstant %6 5
          %9 = OpTypeArray %7 %8
         %37 = OpConstantComposite %9 %34 %34 %34 %34 %34
         %11 = OpTypeStruct
         %38 = OpConstantComposite %11
         %39 = OpConstantComposite %15 %35 %35
         %31 = OpTypePointer Function %14
         %10 = OpTypeStruct %7 %6 %9 %11 %15 %14
         %40 = OpConstantComposite %10 %34 %36 %37 %38 %39 %35
         %13 = OpTypePointer Function %10
         %16 = OpTypePointer Private %10
         %17 = OpTypePointer Workgroup %10
         %18 = OpTypeStruct %16
         %19 = OpTypePointer Private %18
         %20 = OpVariable %16 Private
         %21 = OpVariable %17 Workgroup
         %22 = OpVariable %19 Private
         %23 = OpTypePointer Output %6
         %24 = OpVariable %23 Output
         %27 = OpTypeFunction %2 %13
         %33 = OpConstantNull %16
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %28 = OpFunction %2 None %27
         %29 = OpFunctionParameter %13
         %30 = OpLabel
         %25 = OpVariable %13 Function
         %26 = OpAccessChain %31 %25 %8
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
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(35);
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(39);

  const auto insert_before =
      MakeInstructionDescriptor(26, spv::Op::OpReturn, 0);

  // 20 is not a fresh id.
  ASSERT_FALSE(TransformationMutatePointer(20, 20, insert_before)
                   .IsApplicable(context.get(), transformation_context));

  // |insert_before| instruction descriptor is invalid.
  ASSERT_FALSE(TransformationMutatePointer(
                   20, 70, MakeInstructionDescriptor(26, spv::Op::OpStore, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Can't insert OpLoad before OpVariable.
  ASSERT_FALSE(
      TransformationMutatePointer(
          20, 70, MakeInstructionDescriptor(26, spv::Op::OpVariable, 0))
          .IsApplicable(context.get(), transformation_context));

  // |pointer_id| doesn't exist in the module.
  ASSERT_FALSE(TransformationMutatePointer(70, 70, insert_before)
                   .IsApplicable(context.get(), transformation_context));

  // |pointer_id| doesn't have a type id.
  ASSERT_FALSE(TransformationMutatePointer(11, 70, insert_before)
                   .IsApplicable(context.get(), transformation_context));

  // |pointer_id| is a result id of OpConstantNull.
  ASSERT_FALSE(TransformationMutatePointer(33, 70, insert_before)
                   .IsApplicable(context.get(), transformation_context));

  // |pointer_id| is not a pointer instruction.
  ASSERT_FALSE(TransformationMutatePointer(8, 70, insert_before)
                   .IsApplicable(context.get(), transformation_context));

  // |pointer_id| has invalid storage class
  ASSERT_FALSE(TransformationMutatePointer(24, 70, insert_before)
                   .IsApplicable(context.get(), transformation_context));

  // |pointer_id|'s pointee contains non-scalar and non-composite constituents.
  ASSERT_FALSE(TransformationMutatePointer(22, 70, insert_before)
                   .IsApplicable(context.get(), transformation_context));

  // There is no irrelevant zero constant to insert into the |pointer_id|.
  ASSERT_FALSE(TransformationMutatePointer(20, 70, insert_before)
                   .IsApplicable(context.get(), transformation_context));

  // |pointer_id| is not available before |insert_before|.
  ASSERT_FALSE(
      TransformationMutatePointer(
          26, 70, MakeInstructionDescriptor(26, spv::Op::OpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(40);

  uint32_t fresh_id = 70;
  uint32_t pointer_ids[] = {
      20,  // Mutate Private variable.
      21,  // Mutate Workgroup variable.
      25,  // Mutate Function variable.
      29,  // Mutate function parameter.
      26,  // Mutate OpAccessChain.
  };

  for (auto pointer_id : pointer_ids) {
    TransformationMutatePointer transformation(pointer_id, fresh_id++,
                                               insert_before);
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  std::string after_transformation = R"(
               OpCapability Shader
               OpCapability VariablePointers
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
         %34 = OpConstant %7 0
         %36 = OpConstant %6 0
         %14 = OpTypeVector %7 3
         %35 = OpConstantComposite %14 %34 %34 %34
         %15 = OpTypeMatrix %14 2
          %8 = OpConstant %6 5
          %9 = OpTypeArray %7 %8
         %37 = OpConstantComposite %9 %34 %34 %34 %34 %34
         %11 = OpTypeStruct
         %38 = OpConstantComposite %11
         %39 = OpConstantComposite %15 %35 %35
         %31 = OpTypePointer Function %14
         %10 = OpTypeStruct %7 %6 %9 %11 %15 %14
         %40 = OpConstantComposite %10 %34 %36 %37 %38 %39 %35
         %13 = OpTypePointer Function %10
         %16 = OpTypePointer Private %10
         %17 = OpTypePointer Workgroup %10
         %18 = OpTypeStruct %16
         %19 = OpTypePointer Private %18
         %20 = OpVariable %16 Private
         %21 = OpVariable %17 Workgroup
         %22 = OpVariable %19 Private
         %23 = OpTypePointer Output %6
         %24 = OpVariable %23 Output
         %27 = OpTypeFunction %2 %13
         %33 = OpConstantNull %16
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %28 = OpFunction %2 None %27
         %29 = OpFunctionParameter %13
         %30 = OpLabel
         %25 = OpVariable %13 Function
         %26 = OpAccessChain %31 %25 %8

         ; modified Private variable
         %70 = OpLoad %10 %20
               OpStore %20 %40
               OpStore %20 %70

         ; modified Workgroup variable
         %71 = OpLoad %10 %21
               OpStore %21 %40
               OpStore %21 %71

         ; modified Function variable
         %72 = OpLoad %10 %25
               OpStore %25 %40
               OpStore %25 %72

         ; modified function parameter
         %73 = OpLoad %10 %29
               OpStore %29 %40
               OpStore %29 %73

         ; modified OpAccessChain
         %74 = OpLoad %14 %26
               OpStore %26 %35
               OpStore %26 %74

               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationMutatePointerTest, HandlesUnreachableBlocks) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 0
          %8 = OpTypePointer Function %6
         %11 = OpTypePointer Private %6
         %12 = OpVariable %11 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
               OpReturn
         %10 = OpLabel
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
  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(7);

  ASSERT_FALSE(
      context->GetDominatorAnalysis(context->GetFunction(4))->IsReachable(10));

  const auto insert_before =
      MakeInstructionDescriptor(10, spv::Op::OpReturn, 0);

  // Can mutate a global variable in an unreachable block.
  TransformationMutatePointer transformation(12, 50, insert_before);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
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
          %6 = OpTypeInt 32 1
          %7 = OpConstant %6 0
          %8 = OpTypePointer Function %6
         %11 = OpTypePointer Private %6
         %12 = OpVariable %11 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
               OpReturn
         %10 = OpLabel
         %50 = OpLoad %6 %12
               OpStore %12 %7
               OpStore %12 %50
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
