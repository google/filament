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

#include "source/fuzz/transformation_load.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationLoadTest, BasicTest) {
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
          %7 = OpTypeFloat 32
          %8 = OpTypeStruct %6 %7
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %9
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 2
         %23 = OpConstant %6 1
         %24 = OpConstant %7 1
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Private %7
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %60 = OpConstantNull %50
         %61 = OpUndef %51
         %52 = OpVariable %50 Private
         %53 = OpVariable %51 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function ; irrelevant
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9 ; irrelevant
         %13 = OpLabel
         %46 = OpCopyObject %9 %11 ; irrelevant
         %16 = OpAccessChain %15 %11 %14 ; irrelevant
               OpReturnValue %21
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
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      27);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      11);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      46);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      16);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      52);

  transformation_context.GetFactManager()->AddFactBlockIsDead(36);

  // Variables with pointee types:
  //  52 - ptr_to(7)
  //  53 - ptr_to(6)
  //  20 - ptr_to(8)
  //  27 - ptr_to(8) - irrelevant

  // Access chains with pointee type:
  //  22 - ptr_to(6)
  //  26 - ptr_to(6)
  //  30 - ptr_to(6)
  //  33 - ptr_to(6)
  //  38 - ptr_to(6)
  //  40 - ptr_to(6)
  //  43 - ptr_to(6)
  //  16 - ptr_to(6) - irrelevant

  // Copied object with pointee type:
  //  44 - ptr_to(8)
  //  45 - ptr_to(6)
  //  46 - ptr_to(8) - irrelevant

  // Function parameters with pointee type:
  //  11 - ptr_to(8) - irrelevant

  // Pointers that cannot be used:
  //  60 - null
  //  61 - undefined

  // Bad: id is not fresh
  ASSERT_FALSE(TransformationLoad(
                   33, 33, MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
                   .IsApplicable(context.get(), transformation_context));
  // Bad: attempt to load from 11 from outside its function
  ASSERT_FALSE(TransformationLoad(
                   100, 11, MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: pointer is not available
  ASSERT_FALSE(TransformationLoad(
                   100, 33, MakeInstructionDescriptor(45, SpvOpCopyObject, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: attempt to insert before OpVariable
  ASSERT_FALSE(TransformationLoad(
                   100, 27, MakeInstructionDescriptor(27, SpvOpVariable, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: pointer id does not exist
  ASSERT_FALSE(
      TransformationLoad(100, 1000,
                         MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: pointer id exists but does not have a type
  ASSERT_FALSE(TransformationLoad(
                   100, 5, MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: pointer id exists and has a type, but is not a pointer
  ASSERT_FALSE(TransformationLoad(
                   100, 24, MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: attempt to load from null pointer
  ASSERT_FALSE(TransformationLoad(
                   100, 60, MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: attempt to load from undefined pointer
  ASSERT_FALSE(TransformationLoad(
                   100, 61, MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
                   .IsApplicable(context.get(), transformation_context));
  // Bad: %40 is not available at the program point
  ASSERT_FALSE(
      TransformationLoad(100, 40, MakeInstructionDescriptor(37, SpvOpReturn, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: The described instruction does not exist
  ASSERT_FALSE(TransformationLoad(
                   100, 33, MakeInstructionDescriptor(1000, SpvOpReturn, 0))
                   .IsApplicable(context.get(), transformation_context));

  {
    TransformationLoad transformation(
        100, 33, MakeInstructionDescriptor(38, SpvOpAccessChain, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    TransformationLoad transformation(
        101, 46, MakeInstructionDescriptor(16, SpvOpReturnValue, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    TransformationLoad transformation(
        102, 16, MakeInstructionDescriptor(16, SpvOpReturnValue, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    TransformationLoad transformation(
        103, 40, MakeInstructionDescriptor(43, SpvOpAccessChain, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
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
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeStruct %6 %7
          %9 = OpTypePointer Function %8
         %10 = OpTypeFunction %6 %9
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 2
         %23 = OpConstant %6 1
         %24 = OpConstant %7 1
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Private %7
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %60 = OpConstantNull %50
         %61 = OpUndef %51
         %52 = OpVariable %50 Private
         %53 = OpVariable %51 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function ; irrelevant
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
        %100 = OpLoad %6 %33
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
        %103 = OpLoad %6 %40
         %43 = OpAccessChain %15 %20 %14
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9 ; irrelevant
         %13 = OpLabel
         %46 = OpCopyObject %9 %11 ; irrelevant
         %16 = OpAccessChain %15 %11 %14 ; irrelevant
        %101 = OpLoad %8 %46
        %102 = OpLoad %6 %16
               OpReturnValue %21
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
