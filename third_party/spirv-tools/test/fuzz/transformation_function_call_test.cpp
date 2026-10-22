// Copyright (c) 2020 Google LLC
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

#include "source/fuzz/transformation_function_call.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationFunctionCallTest, BasicTest) {
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
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %6 %7
         %12 = OpTypeFloat 32
         %13 = OpTypePointer Function %12
         %14 = OpTypeFunction %6 %7 %13
         %27 = OpConstant %6 1
         %50 = OpConstant %12 1
         %57 = OpTypeBool
         %58 = OpConstantFalse %57
        %204 = OpUndef %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %61 = OpVariable %7 Function
         %62 = OpVariable %7 Function
         %65 = OpVariable %13 Function
         %66 = OpVariable %7 Function
         %68 = OpVariable %13 Function
         %71 = OpVariable %7 Function
         %72 = OpVariable %13 Function
         %73 = OpVariable %7 Function
         %75 = OpVariable %13 Function
         %78 = OpVariable %7 Function
         %98 = OpAccessChain %7 %71
         %99 = OpCopyObject %7 %71
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
               OpBranch %60
         %60 = OpLabel
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %26 = OpLoad %6 %9
         %28 = OpIAdd %6 %26 %27
               OpSelectionMerge %97 None
               OpBranchConditional %58 %96 %97
         %96 = OpLabel
               OpBranch %97
         %97 = OpLabel
               OpReturnValue %28
               OpFunctionEnd
         %17 = OpFunction %6 None %14
         %15 = OpFunctionParameter %7
         %16 = OpFunctionParameter %13
         %18 = OpLabel
         %31 = OpVariable %7 Function
         %32 = OpLoad %6 %15
               OpStore %31 %32
         %33 = OpFunctionCall %6 %10 %31
               OpReturnValue %33
               OpFunctionEnd
         %21 = OpFunction %6 None %14
         %19 = OpFunctionParameter %7
         %20 = OpFunctionParameter %13
         %22 = OpLabel
         %36 = OpLoad %6 %19
         %37 = OpLoad %12 %20
         %38 = OpConvertFToS %6 %37
         %39 = OpIAdd %6 %36 %38
               OpReturnValue %39
               OpFunctionEnd
         %24 = OpFunction %6 None %8
         %23 = OpFunctionParameter %7
         %25 = OpLabel
         %44 = OpVariable %7 Function
         %46 = OpVariable %13 Function
         %51 = OpVariable %7 Function
         %52 = OpVariable %13 Function
         %42 = OpLoad %6 %23
         %43 = OpConvertSToF %12 %42
         %45 = OpLoad %6 %23
               OpStore %44 %45
               OpStore %46 %43
         %47 = OpFunctionCall %6 %17 %44 %46
         %48 = OpLoad %6 %23
         %49 = OpIAdd %6 %48 %27
               OpStore %51 %49
               OpStore %52 %50
         %53 = OpFunctionCall %6 %17 %51 %52
         %54 = OpIAdd %6 %47 %53
               OpReturnValue %54
               OpFunctionEnd
        %200 = OpFunction %6 None %14
        %201 = OpFunctionParameter %7
        %202 = OpFunctionParameter %13
        %203 = OpLabel
               OpSelectionMerge %206 None
               OpBranchConditional %58 %205 %206
        %205 = OpLabel
               OpBranch %206
        %206 = OpLabel
               OpReturnValue %204
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
  transformation_context.GetFactManager()->AddFactBlockIsDead(59);
  transformation_context.GetFactManager()->AddFactBlockIsDead(11);
  transformation_context.GetFactManager()->AddFactBlockIsDead(18);
  transformation_context.GetFactManager()->AddFactBlockIsDead(25);
  transformation_context.GetFactManager()->AddFactBlockIsDead(96);
  transformation_context.GetFactManager()->AddFactBlockIsDead(205);
  transformation_context.GetFactManager()->AddFactFunctionIsLivesafe(21);
  transformation_context.GetFactManager()->AddFactFunctionIsLivesafe(200);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      71);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      72);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      19);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      20);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      23);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      44);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      46);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      51);
  transformation_context.GetFactManager()->AddFactValueOfPointeeIsIrrelevant(
      52);

  // Livesafe functions with argument types: 21(7, 13), 200(7, 13)
  // Non-livesafe functions with argument types: 4(), 10(7), 17(7, 13), 24(7)
  // Call graph edges:
  //    17 -> 10
  //    24 -> 17

  // Bad transformations
  // Too many arguments
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 21, {71, 72, 71},
                   MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
                   .IsApplicable(context.get(), transformation_context));
  // Too few arguments
  ASSERT_FALSE(
      TransformationFunctionCall(
          100, 21, {71}, MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
          .IsApplicable(context.get(), transformation_context));
  // Arguments are the wrong way around (types do not match)
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 21, {72, 71},
                   MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
                   .IsApplicable(context.get(), transformation_context));
  // 21 is not an appropriate argument
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 21, {21, 72},
                   MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
                   .IsApplicable(context.get(), transformation_context));
  // 300 does not exist
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 21, {300, 72},
                   MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
                   .IsApplicable(context.get(), transformation_context));
  // 71 is not a function
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 71, {71, 72},
                   MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
                   .IsApplicable(context.get(), transformation_context));
  // 500 does not exist
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 500, {71, 72},
                   MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
                   .IsApplicable(context.get(), transformation_context));
  // Id is not fresh
  ASSERT_FALSE(
      TransformationFunctionCall(
          21, 21, {71, 72}, MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
          .IsApplicable(context.get(), transformation_context));
  // Access chain as pointer parameter
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 21, {98, 72},
                   MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
                   .IsApplicable(context.get(), transformation_context));
  // Copied object as pointer parameter
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 21, {99, 72},
                   MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
                   .IsApplicable(context.get(), transformation_context));
  // Non-livesafe called from original live block
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 10, {71},
                   MakeInstructionDescriptor(99, spv::Op::OpSelectionMerge, 0))
                   .IsApplicable(context.get(), transformation_context));
  // Non-livesafe called from livesafe function
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 10, {19},
                   MakeInstructionDescriptor(38, spv::Op::OpConvertFToS, 0))
                   .IsApplicable(context.get(), transformation_context));
  // Livesafe function called with pointer to non-arbitrary local variable
  ASSERT_FALSE(TransformationFunctionCall(
                   100, 21, {61, 72},
                   MakeInstructionDescriptor(38, spv::Op::OpConvertFToS, 0))
                   .IsApplicable(context.get(), transformation_context));
  // Direct recursion
  ASSERT_FALSE(
      TransformationFunctionCall(
          100, 4, {}, MakeInstructionDescriptor(59, spv::Op::OpBranch, 0))
          .IsApplicable(context.get(), transformation_context));
  // Indirect recursion
  ASSERT_FALSE(
      TransformationFunctionCall(
          100, 24, {9}, MakeInstructionDescriptor(96, spv::Op::OpBranch, 0))
          .IsApplicable(context.get(), transformation_context));
  // Parameter 23 is not available at the call site
  ASSERT_FALSE(
      TransformationFunctionCall(
          104, 10, {23}, MakeInstructionDescriptor(205, spv::Op::OpBranch, 0))
          .IsApplicable(context.get(), transformation_context));

  // Good transformations
  {
    // Livesafe called from dead block: fine
    TransformationFunctionCall transformation(
        100, 21, {71, 72}, MakeInstructionDescriptor(59, spv::Op::OpBranch, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
  {
    // Livesafe called from original live block: fine
    TransformationFunctionCall transformation(
        101, 21, {71, 72},
        MakeInstructionDescriptor(98, spv::Op::OpAccessChain, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
  {
    // Livesafe called from livesafe function: fine
    TransformationFunctionCall transformation(
        102, 200, {19, 20}, MakeInstructionDescriptor(36, spv::Op::OpLoad, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
  {
    // Dead called from dead block in injected function: fine
    TransformationFunctionCall transformation(
        103, 10, {23}, MakeInstructionDescriptor(45, spv::Op::OpLoad, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
  {
    // Non-livesafe called from dead block in livesafe function: OK
    TransformationFunctionCall transformation(
        104, 10, {201}, MakeInstructionDescriptor(205, spv::Op::OpBranch, 0));
    ASSERT_TRUE(
        transformation.IsApplicable(context.get(), transformation_context));
    ApplyAndCheckFreshIds(transformation, context.get(),
                          &transformation_context);
    ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(
        context.get(), validator_options, kConsoleMessageConsumer));
  }
  {
    // Livesafe called from dead block with non-arbitrary parameter
    TransformationFunctionCall transformation(
        105, 21, {62, 65}, MakeInstructionDescriptor(59, spv::Op::OpBranch, 0));
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
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %6 %7
         %12 = OpTypeFloat 32
         %13 = OpTypePointer Function %12
         %14 = OpTypeFunction %6 %7 %13
         %27 = OpConstant %6 1
         %50 = OpConstant %12 1
         %57 = OpTypeBool
         %58 = OpConstantFalse %57
        %204 = OpUndef %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %61 = OpVariable %7 Function
         %62 = OpVariable %7 Function
         %65 = OpVariable %13 Function
         %66 = OpVariable %7 Function
         %68 = OpVariable %13 Function
         %71 = OpVariable %7 Function
         %72 = OpVariable %13 Function
         %73 = OpVariable %7 Function
         %75 = OpVariable %13 Function
         %78 = OpVariable %7 Function
        %101 = OpFunctionCall %6 %21 %71 %72
         %98 = OpAccessChain %7 %71
         %99 = OpCopyObject %7 %71
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
        %100 = OpFunctionCall %6 %21 %71 %72
        %105 = OpFunctionCall %6 %21 %62 %65
               OpBranch %60
         %60 = OpLabel
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %6 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
         %26 = OpLoad %6 %9
         %28 = OpIAdd %6 %26 %27
               OpSelectionMerge %97 None
               OpBranchConditional %58 %96 %97
         %96 = OpLabel
               OpBranch %97
         %97 = OpLabel
               OpReturnValue %28
               OpFunctionEnd
         %17 = OpFunction %6 None %14
         %15 = OpFunctionParameter %7
         %16 = OpFunctionParameter %13
         %18 = OpLabel
         %31 = OpVariable %7 Function
         %32 = OpLoad %6 %15
               OpStore %31 %32
         %33 = OpFunctionCall %6 %10 %31
               OpReturnValue %33
               OpFunctionEnd
         %21 = OpFunction %6 None %14
         %19 = OpFunctionParameter %7
         %20 = OpFunctionParameter %13
         %22 = OpLabel
        %102 = OpFunctionCall %6 %200 %19 %20
         %36 = OpLoad %6 %19
         %37 = OpLoad %12 %20
         %38 = OpConvertFToS %6 %37
         %39 = OpIAdd %6 %36 %38
               OpReturnValue %39
               OpFunctionEnd
         %24 = OpFunction %6 None %8
         %23 = OpFunctionParameter %7
         %25 = OpLabel
         %44 = OpVariable %7 Function
         %46 = OpVariable %13 Function
         %51 = OpVariable %7 Function
         %52 = OpVariable %13 Function
         %42 = OpLoad %6 %23
         %43 = OpConvertSToF %12 %42
        %103 = OpFunctionCall %6 %10 %23
         %45 = OpLoad %6 %23
               OpStore %44 %45
               OpStore %46 %43
         %47 = OpFunctionCall %6 %17 %44 %46
         %48 = OpLoad %6 %23
         %49 = OpIAdd %6 %48 %27
               OpStore %51 %49
               OpStore %52 %50
         %53 = OpFunctionCall %6 %17 %51 %52
         %54 = OpIAdd %6 %47 %53
               OpReturnValue %54
               OpFunctionEnd
        %200 = OpFunction %6 None %14
        %201 = OpFunctionParameter %7
        %202 = OpFunctionParameter %13
        %203 = OpLabel
               OpSelectionMerge %206 None
               OpBranchConditional %58 %205 %206
        %205 = OpLabel
        %104 = OpFunctionCall %6 %10 %201
               OpBranch %206
        %206 = OpLabel
               OpReturnValue %204
               OpFunctionEnd
  )";
  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationFunctionCallTest, DoNotInvokeEntryPoint) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %3
         %11 = OpLabel
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
  transformation_context.GetFactManager()->AddFactBlockIsDead(11);

  // 4 is an entry point, so it is not legal for it to be the target of a call.
  ASSERT_FALSE(
      TransformationFunctionCall(
          100, 4, {}, MakeInstructionDescriptor(11, spv::Op::OpReturn, 0))
          .IsApplicable(context.get(), transformation_context));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
