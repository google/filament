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

  // Bad: id is not fresh
  ASSERT_FALSE(
      TransformationLoad(33, 33, false, 0, 0,
                         MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));
  // Bad: attempt to load from 11 from outside its function
  ASSERT_FALSE(
      TransformationLoad(100, 11, false, 0, 0,
                         MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: pointer is not available
  ASSERT_FALSE(
      TransformationLoad(100, 33, false, 0, 0,
                         MakeInstructionDescriptor(45, SpvOpCopyObject, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: attempt to insert before OpVariable
  ASSERT_FALSE(
      TransformationLoad(100, 27, false, 0, 0,
                         MakeInstructionDescriptor(27, SpvOpVariable, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: pointer id does not exist
  ASSERT_FALSE(
      TransformationLoad(100, 1000, false, 0, 0,
                         MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: pointer id exists but does not have a type
  ASSERT_FALSE(
      TransformationLoad(100, 5, false, 0, 0,
                         MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: pointer id exists and has a type, but is not a pointer
  ASSERT_FALSE(
      TransformationLoad(100, 24, false, 0, 0,
                         MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: attempt to load from null pointer
  ASSERT_FALSE(
      TransformationLoad(100, 60, false, 0, 0,
                         MakeInstructionDescriptor(38, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: %40 is not available at the program point
  ASSERT_FALSE(TransformationLoad(100, 40, false, 0, 0,
                                  MakeInstructionDescriptor(37, SpvOpReturn, 0))
                   .IsApplicable(context.get(), transformation_context));

  // Bad: The described instruction does not exist
  ASSERT_FALSE(
      TransformationLoad(100, 33, false, 0, 0,
                         MakeInstructionDescriptor(1000, SpvOpReturn, 0))
          .IsApplicable(context.get(), transformation_context));

  {
    TransformationLoad transformation(
        100, 33, false, 0, 0,
        MakeInstructionDescriptor(38, SpvOpAccessChain, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    TransformationLoad transformation(
        101, 46, false, 0, 0,
        MakeInstructionDescriptor(16, SpvOpReturnValue, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    TransformationLoad transformation(
        102, 16, false, 0, 0,
        MakeInstructionDescriptor(16, SpvOpReturnValue, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  {
    TransformationLoad transformation(
        103, 40, false, 0, 0,
        MakeInstructionDescriptor(43, SpvOpAccessChain, 0));
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

TEST(TransformationLoadTest, AtomicLoadTestCase) {
  const std::string shader = R"(
               OpCapability Shader
               OpCapability Int8
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 8 1
          %9 = OpTypeInt 32 0
         %26 = OpTypeFloat 32
          %8 = OpTypeStruct %6
         %10 = OpTypePointer StorageBuffer %8
         %11 = OpVariable %10 StorageBuffer
         %19 = OpConstant %26 0
         %18 = OpConstant %9 1
         %12 = OpConstant %6 0
         %13 = OpTypePointer StorageBuffer %6
         %15 = OpConstant %6 4
         %16 = OpConstant %6 7
         %17 = OpConstant %7 4
         %20 = OpConstant %9 64
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %14 = OpAccessChain %13 %11 %12
         %24 = OpAccessChain %13 %11 %12
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

  // Bad: id is not fresh.
  ASSERT_FALSE(
      TransformationLoad(14, 14, true, 15, 20,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: id 100 of memory scope instruction does not exist.
  ASSERT_FALSE(
      TransformationLoad(21, 14, true, 100, 20,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));
  // Bad: id 100 of memory semantics instruction does not exist.
  ASSERT_FALSE(
      TransformationLoad(21, 14, true, 15, 100,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));
  // Bad: memory scope should be |OpConstant| opcode.
  ASSERT_FALSE(
      TransformationLoad(21, 14, true, 5, 20,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));
  // Bad: memory semantics should be |OpConstant| opcode.
  ASSERT_FALSE(
      TransformationLoad(21, 14, true, 15, 5,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: The memory scope instruction must have an Integer operand.
  ASSERT_FALSE(
      TransformationLoad(21, 14, true, 15, 19,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));
  // Bad: The memory memory semantics instruction must have an Integer operand.
  ASSERT_FALSE(
      TransformationLoad(21, 14, true, 19, 20,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: Integer size of the memory scope must be equal to 32 bits.
  ASSERT_FALSE(
      TransformationLoad(21, 14, true, 17, 20,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: Integer size of memory semantics must be equal to 32 bits.
  ASSERT_FALSE(
      TransformationLoad(21, 14, true, 15, 17,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: memory scope value must be 4 (SpvScopeInvocation).
  ASSERT_FALSE(
      TransformationLoad(21, 14, true, 16, 20,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: memory semantics value must be either:
  // 64 (SpvMemorySemanticsUniformMemoryMask)
  // 256 (SpvMemorySemanticsWorkgroupMemoryMask)
  ASSERT_FALSE(
      TransformationLoad(21, 14, true, 15, 16,
                         MakeInstructionDescriptor(24, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: The described instruction does not exist
  ASSERT_FALSE(
      TransformationLoad(21, 14, false, 15, 20,
                         MakeInstructionDescriptor(150, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Successful transformations.
  {
    TransformationLoad transformation(
        21, 14, true, 15, 20,
        MakeInstructionDescriptor(24, SpvOpAccessChain, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  const std::string after_transformation = R"(
               OpCapability Shader
               OpCapability Int8
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 320
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 8 1
          %9 = OpTypeInt 32 0
         %26 = OpTypeFloat 32
          %8 = OpTypeStruct %6
         %10 = OpTypePointer StorageBuffer %8
         %11 = OpVariable %10 StorageBuffer
         %19 = OpConstant %26 0
         %18 = OpConstant %9 1
         %12 = OpConstant %6 0
         %13 = OpTypePointer StorageBuffer %6
         %15 = OpConstant %6 4
         %16 = OpConstant %6 7
         %17 = OpConstant %7 4
         %20 = OpConstant %9 64
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %14 = OpAccessChain %13 %11 %12
         %21 = OpAtomicLoad %6 %14 %15 %20
         %24 = OpAccessChain %13 %11 %12
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationLoadTest, AtomicLoadTestCaseForWorkgroupMemory) {
  std::string shader = R"(
               OpCapability Shader
               OpCapability Int8
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %26 = OpTypeFloat 32
          %27 = OpTypeInt 8 1
          %7 = OpTypeInt 32 0 ; 0 means unsigned
          %8 = OpConstant %7 0
         %17 = OpConstant %27 4
         %19 = OpConstant %26 0
          %9 = OpTypePointer Function %6
         %13 = OpTypeStruct %6
         %12 = OpTypePointer Workgroup %13
         %11 = OpVariable %12 Workgroup
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 4
         %23 = OpConstant %6 256
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Workgroup %6
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %53 = OpVariable %51 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %50 %11 %14
         %40 = OpAccessChain %50 %11 %14
               OpBranch %37
         %37 = OpLabel
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

  // Bad: Can't insert OpAccessChain before the id 23 of memory scope.
  ASSERT_FALSE(
      TransformationLoad(60, 38, true, 21, 23,
                         MakeInstructionDescriptor(23, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Bad: Can't insert OpAccessChain before the id 23 of memory semantics.
  ASSERT_FALSE(
      TransformationLoad(60, 38, true, 21, 23,
                         MakeInstructionDescriptor(21, SpvOpAccessChain, 0))
          .IsApplicable(context.get(), transformation_context));

  // Successful transformations.
  {
    TransformationLoad transformation(
        60, 38, true, 21, 23,
        MakeInstructionDescriptor(40, SpvOpAccessChain, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }

  std::string after_transformation = R"(
               OpCapability Shader
               OpCapability Int8
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %26 = OpTypeFloat 32
          %27 = OpTypeInt 8 1
          %7 = OpTypeInt 32 0 ; 0 means unsigned
          %8 = OpConstant %7 0
         %17 = OpConstant %27 4
         %19 = OpConstant %26 0
          %9 = OpTypePointer Function %6
         %13 = OpTypeStruct %6
         %12 = OpTypePointer Workgroup %13
         %11 = OpVariable %12 Workgroup
         %14 = OpConstant %6 0
         %15 = OpTypePointer Function %6
         %51 = OpTypePointer Private %6
         %21 = OpConstant %6 4
         %23 = OpConstant %6 256
         %25 = OpTypePointer Function %7
         %50 = OpTypePointer Workgroup %6
         %34 = OpTypeBool
         %35 = OpConstantFalse %34
         %53 = OpVariable %51 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %50 %11 %14
         %60 = OpAtomicLoad %6 %38 %21 %23
         %40 = OpAccessChain %50 %11 %14
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
