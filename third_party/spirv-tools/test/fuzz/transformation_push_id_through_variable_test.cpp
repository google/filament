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

#include "source/fuzz/transformation_push_id_through_variable.h"

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(TransformationPushIdThroughVariableTest, IsApplicable) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
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
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
        %100 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Tests the reference shader validity.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  // Tests |value_synonym_id| and |variable_id| are fresh ids.
  uint32_t value_id = 21;
  uint32_t value_synonym_id = 62;
  uint32_t variable_id = 63;
  uint32_t initializer_id = 23;
  uint32_t variable_storage_class = SpvStorageClassPrivate;
  auto instruction_descriptor =
      MakeInstructionDescriptor(95, SpvOpReturnValue, 0);
  auto transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests |value_synonym_id| and |variable_id| are non-fresh ids.
  value_id = 80;
  value_synonym_id = 60;
  variable_id = 61;
  initializer_id = 80;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(38, SpvOpAccessChain, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // The instruction to insert before is not defined.
  value_id = 80;
  value_synonym_id = 62;
  variable_id = 63;
  initializer_id = 80;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(64, SpvOpAccessChain, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Attempting to insert the store and load instructions
  // before an OpVariable instruction.
  value_id = 24;
  value_synonym_id = 62;
  variable_id = 63;
  initializer_id = 24;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(27, SpvOpVariable, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // The block containing instruction descriptor must be reachable.
  value_id = 80;
  value_synonym_id = 62;
  variable_id = 63;
  initializer_id = 80;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(100, SpvOpUnreachable, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests value instruction not available.
  value_id = 64;
  value_synonym_id = 62;
  variable_id = 63;
  initializer_id = 23;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(95, SpvOpReturnValue, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests pointer type not available.
  value_id = 80;
  value_synonym_id = 62;
  variable_id = 63;
  initializer_id = 80;
  variable_storage_class = SpvStorageClassPrivate;
  instruction_descriptor = MakeInstructionDescriptor(95, SpvOpReturnValue, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Tests not a private nor function storage class.
  value_id = 93;
  value_synonym_id = 62;
  variable_id = 63;
  initializer_id = 93;
  variable_storage_class = SpvStorageClassInput;
  instruction_descriptor = MakeInstructionDescriptor(95, SpvOpReturnValue, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
#ifndef NDEBUG
  ASSERT_DEATH(
      transformation.IsApplicable(context.get(), transformation_context),
      "The variable storage class must be private or function");
#endif

  // Tests value instruction not available before instruction.
  value_id = 95;
  value_synonym_id = 62;
  variable_id = 63;
  initializer_id = 80;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(40, SpvOpAccessChain, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Variable initializer is not constant.
  value_id = 95;
  value_synonym_id = 62;
  variable_id = 63;
  initializer_id = 95;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(40, SpvOpAccessChain, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));

  // Variable initializer has wrong type.
  value_id = 95;
  value_synonym_id = 62;
  variable_id = 63;
  initializer_id = 93;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(40, SpvOpAccessChain, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_FALSE(
      transformation.IsApplicable(context.get(), transformation_context));
}

TEST(TransformationPushIdThroughVariableTest, Apply) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
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
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
               OpStore %53 %21
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  uint32_t value_id = 80;
  uint32_t value_synonym_id = 100;
  uint32_t variable_id = 101;
  uint32_t initializer_id = 80;
  uint32_t variable_storage_class = SpvStorageClassFunction;
  auto instruction_descriptor =
      MakeInstructionDescriptor(38, SpvOpAccessChain, 0);
  auto transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  value_id = 21;
  value_synonym_id = 102;
  variable_id = 103;
  initializer_id = 21;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(38, SpvOpAccessChain, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  value_id = 95;
  value_synonym_id = 104;
  variable_id = 105;
  initializer_id = 80;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(95, SpvOpReturnValue, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  value_id = 80;
  value_synonym_id = 106;
  variable_id = 107;
  initializer_id = 80;
  variable_storage_class = SpvStorageClassFunction;
  instruction_descriptor = MakeInstructionDescriptor(95, SpvOpReturnValue, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  value_id = 21;
  value_synonym_id = 108;
  variable_id = 109;
  initializer_id = 21;
  variable_storage_class = SpvStorageClassPrivate;
  instruction_descriptor = MakeInstructionDescriptor(95, SpvOpReturnValue, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  value_id = 23;
  value_synonym_id = 110;
  variable_id = 111;
  initializer_id = 21;
  variable_storage_class = SpvStorageClassPrivate;
  instruction_descriptor = MakeInstructionDescriptor(27, SpvOpStore, 0);
  transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);

  std::string variant_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53 %109 %111
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
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
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
        %109 = OpVariable %51 Private %21
        %111 = OpVariable %51 Private %21
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %103 = OpVariable %15 Function %21
        %101 = OpVariable %9 Function %80
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
               OpStore %111 %23
        %110 = OpLoad %6 %111
               OpStore %53 %21
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
               OpStore %101 %80
        %100 = OpLoad %8 %101
               OpStore %103 %21
        %102 = OpLoad %6 %103
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
        %107 = OpVariable %9 Function %80
        %105 = OpVariable %9 Function %80
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpStore %105 %95
        %104 = OpLoad %8 %105
               OpStore %107 %80
        %106 = OpLoad %8 %107
               OpStore %109 %21
        %108 = OpLoad %6 %109
               OpReturnValue %21
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, variant_shader, context.get()));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(80, {}), MakeDataDescriptor(100, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(21, {}), MakeDataDescriptor(102, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(95, {}), MakeDataDescriptor(104, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(80, {}), MakeDataDescriptor(106, {})));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(21, {}), MakeDataDescriptor(108, {})));
}

TEST(TransformationPushIdThroughVariableTest, AddSynonymsForRelevantIds) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
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
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
        %100 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Tests the reference shader validity.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  uint32_t value_id = 21;
  uint32_t value_synonym_id = 62;
  uint32_t variable_id = 63;
  uint32_t initializer_id = 23;
  uint32_t variable_storage_class = SpvStorageClassPrivate;
  auto instruction_descriptor =
      MakeInstructionDescriptor(95, SpvOpReturnValue, 0);
  auto transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_TRUE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(21, {}), MakeDataDescriptor(62, {})));
}

TEST(TransformationPushIdThroughVariableTest, DontAddSynonymsForIrrelevantIds) {
  std::string reference_shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %52 %53
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpDecorate %92 BuiltIn FragCoord
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
         %80 = OpConstantComposite %8 %21 %24
         %90 = OpTypeVector %7 4
         %91 = OpTypePointer Input %90
         %92 = OpVariable %91 Input
         %93 = OpConstantComposite %90 %24 %24 %24 %24
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %20 = OpVariable %9 Function
         %27 = OpVariable %9 Function
         %22 = OpAccessChain %15 %20 %14
         %44 = OpCopyObject %9 %20
         %26 = OpAccessChain %25 %20 %23
         %29 = OpFunctionCall %6 %12 %27
         %30 = OpAccessChain %15 %20 %14
         %45 = OpCopyObject %15 %30
         %81 = OpCopyObject %9 %27
         %33 = OpAccessChain %15 %20 %14
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %15 %20 %14
         %40 = OpAccessChain %15 %20 %14
         %43 = OpAccessChain %15 %20 %14
         %82 = OpCopyObject %9 %27
               OpBranch %37
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %6 None %10
         %11 = OpFunctionParameter %9
         %13 = OpLabel
         %46 = OpCopyObject %9 %11
         %16 = OpAccessChain %15 %11 %14
         %95 = OpCopyObject %8 %80
               OpReturnValue %21
        %100 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_4;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Tests the reference shader validity.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  transformation_context.GetFactManager()->AddFactIdIsIrrelevant(21);

  uint32_t value_id = 21;
  uint32_t value_synonym_id = 62;
  uint32_t variable_id = 63;
  uint32_t initializer_id = 23;
  uint32_t variable_storage_class = SpvStorageClassPrivate;
  auto instruction_descriptor =
      MakeInstructionDescriptor(95, SpvOpReturnValue, 0);
  auto transformation = TransformationPushIdThroughVariable(
      value_id, value_synonym_id, variable_id, variable_storage_class,
      initializer_id, instruction_descriptor);
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(21, {}), MakeDataDescriptor(62, {})));
}

TEST(TransformationPushIdThroughVariableTest, DontAddSynonymsInDeadBlocks) {
  std::string reference_shader = R"(
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
         %50 = OpTypePointer Function %13
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
  const auto context =
      BuildModule(env, consumer, reference_shader, kFuzzAssembleOption);

  spvtools::ValidatorOptions validator_options;
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Tests the reference shader validity.
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));

  transformation_context.GetFactManager()->AddFactBlockIsDead(15);
  auto transformation = TransformationPushIdThroughVariable(
      14, 100, 101, SpvStorageClassFunction, 14,
      MakeInstructionDescriptor(15, SpvOpBranch, 0));
  ASSERT_TRUE(
      transformation.IsApplicable(context.get(), transformation_context));
  ApplyAndCheckFreshIds(transformation, context.get(), &transformation_context);
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  ASSERT_FALSE(transformation_context.GetFactManager()->IsSynonymous(
      MakeDataDescriptor(14, {}), MakeDataDescriptor(100, {})));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
