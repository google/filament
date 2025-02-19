// Copyright (c) 2017 Google Inc.
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

#include <string>

#include "gmock/gmock.h"
#include "source/opt/build_module.h"
#include "source/opt/value_number_table.h"
#include "test/opt/pass_fixture.h"

namespace spvtools {
namespace opt {
namespace {

using ::testing::HasSubstr;
using ::testing::MatchesRegex;
using ValueTableTest = PassTest<::testing::Test>;

TEST_F(ValueTableTest, SameInstructionSameValue) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Function %5
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %8 = OpVariable %6 Function
          %9 = OpLoad %5 %8
         %10 = OpFAdd %5 %9 %9
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst = context->get_def_use_mgr()->GetDef(10);
  EXPECT_EQ(vtable.GetValueNumber(inst), vtable.GetValueNumber(inst));
}

TEST_F(ValueTableTest, DifferentInstructionSameValue) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Function %5
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %8 = OpVariable %6 Function
          %9 = OpLoad %5 %8
         %10 = OpFAdd %5 %9 %9
         %11 = OpFAdd %5 %9 %9
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(10);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(11);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, SameValueDifferentBlock) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Function %5
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %8 = OpVariable %6 Function
          %9 = OpLoad %5 %8
         %10 = OpFAdd %5 %9 %9
               OpBranch %11
         %11 = OpLabel
         %12 = OpFAdd %5 %9 %9
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(10);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(12);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, DifferentValue) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Function %5
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %8 = OpVariable %6 Function
          %9 = OpLoad %5 %8
         %10 = OpFAdd %5 %9 %9
         %11 = OpFAdd %5 %9 %10
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(10);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(11);
  EXPECT_NE(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, DifferentValueDifferentBlock) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Function %5
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %8 = OpVariable %6 Function
          %9 = OpLoad %5 %8
         %10 = OpFAdd %5 %9 %9
               OpBranch %11
         %11 = OpLabel
         %12 = OpFAdd %5 %9 %10
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(10);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(12);
  EXPECT_NE(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, SameLoad) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Function %5
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %8 = OpVariable %6 Function
          %9 = OpLoad %5 %8
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst = context->get_def_use_mgr()->GetDef(9);
  EXPECT_EQ(vtable.GetValueNumber(inst), vtable.GetValueNumber(inst));
}

// Two different loads, even from the same memory, must given different value
// numbers if the memory is not read-only.
TEST_F(ValueTableTest, DifferentFunctionLoad) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Function %5
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %8 = OpVariable %6 Function
          %9 = OpLoad %5 %8
          %10 = OpLoad %5 %8
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(9);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(10);
  EXPECT_NE(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, DifferentUniformLoad) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Uniform %5
          %8 = OpVariable %6 Uniform
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %9 = OpLoad %5 %8
          %10 = OpLoad %5 %8
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(9);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(10);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, DifferentInputLoad) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Input %5
          %8 = OpVariable %6 Input
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %9 = OpLoad %5 %8
          %10 = OpLoad %5 %8
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(9);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(10);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, DifferentUniformConstantLoad) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer UniformConstant %5
          %8 = OpVariable %6 UniformConstant
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %9 = OpLoad %5 %8
          %10 = OpLoad %5 %8
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(9);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(10);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, DifferentPushConstantLoad) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer PushConstant %5
          %8 = OpVariable %6 PushConstant
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %9 = OpLoad %5 %8
          %10 = OpLoad %5 %8
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(9);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(10);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, SameCall) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypeFunction %5
          %7 = OpTypePointer Function %5
          %8 = OpVariable %7 Private
          %2 = OpFunction %3 None %4
          %9 = OpLabel
         %10 = OpFunctionCall %5 %11
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %5 None %6
         %12 = OpLabel
         %13 = OpLoad %5 %8
               OpReturnValue %13
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst = context->get_def_use_mgr()->GetDef(10);
  EXPECT_EQ(vtable.GetValueNumber(inst), vtable.GetValueNumber(inst));
}

// Function calls should be given a new value number, even if they are the same.
TEST_F(ValueTableTest, DifferentCall) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypeFunction %5
          %7 = OpTypePointer Function %5
          %8 = OpVariable %7 Private
          %2 = OpFunction %3 None %4
          %9 = OpLabel
         %10 = OpFunctionCall %5 %11
         %12 = OpFunctionCall %5 %11
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %5 None %6
         %13 = OpLabel
         %14 = OpLoad %5 %8
               OpReturnValue %14
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(10);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(12);
  EXPECT_NE(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

// It is possible to have two instruction that compute the same numerical value,
// but with different types.  They should have different value numbers.
TEST_F(ValueTableTest, DifferentTypes) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeInt 32 0
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %5
          %2 = OpFunction %3 None %4
          %8 = OpLabel
          %9 = OpVariable %7 Function
         %10 = OpLoad %5 %9
         %11 = OpIAdd %5 %10 %10
         %12 = OpIAdd %6 %10 %10
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(11);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(12);
  EXPECT_NE(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, CopyObject) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Function %5
          %2 = OpFunction %3 None %4
          %7 = OpLabel
          %8 = OpVariable %6 Function
          %9 = OpLoad %5 %8
         %10 = OpCopyObject %5 %9
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(9);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(10);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, CopyObjectWitDecoration) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
               OpDecorate %3 NonUniformEXT
          %4 = OpTypeVoid
          %5 = OpTypeFunction %4
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %2 = OpFunction %4 None %5
          %8 = OpLabel
          %9 = OpVariable %7 Function
         %10 = OpLoad %6 %9
          %3 = OpCopyObject %6 %10
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(10);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(3);
  EXPECT_NE(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

// Test that a phi where the operands have the same value assigned that value
// to the result of the phi.
TEST_F(ValueTableTest, PhiTest1) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Uniform %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %9 = OpVariable %6 Uniform
          %2 = OpFunction %3 None %4
         %10 = OpLabel
               OpBranchConditional %8 %11 %12
         %11 = OpLabel
         %13 = OpLoad %5 %9
               OpBranch %14
         %12 = OpLabel
         %15 = OpLoad %5 %9
               OpBranch %14
         %14 = OpLabel
         %16 = OpPhi %5 %13 %11 %15 %12
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(13);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(15);
  Instruction* phi = context->get_def_use_mgr()->GetDef(16);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(phi));
}

TEST_F(ValueTableTest, PhiTest1WithDecoration) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
               OpDecorate %3 NonUniformEXT
          %4 = OpTypeVoid
          %5 = OpTypeFunction %5
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Uniform %6
          %8 = OpTypeBool
          %9 = OpConstantTrue %8
          %10 = OpVariable %7 Uniform
          %2 = OpFunction %4 None %5
         %11 = OpLabel
               OpBranchConditional %9 %12 %13
         %12 = OpLabel
         %14 = OpLoad %6 %10
               OpBranch %15
         %13 = OpLabel
         %16 = OpLoad %6 %10
               OpBranch %15
         %15 = OpLabel
         %3 = OpPhi %6 %14 %12 %16 %13
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(14);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(16);
  Instruction* phi = context->get_def_use_mgr()->GetDef(3);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
  EXPECT_NE(vtable.GetValueNumber(inst1), vtable.GetValueNumber(phi));
}

// When the values for the inputs to a phi do not match, then the phi should
// have its own value number.
TEST_F(ValueTableTest, PhiTest2) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Uniform %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %9 = OpVariable %6 Uniform
         %10 = OpVariable %6 Uniform
          %2 = OpFunction %3 None %4
         %11 = OpLabel
               OpBranchConditional %8 %12 %13
         %12 = OpLabel
         %14 = OpLoad %5 %9
               OpBranch %15
         %13 = OpLabel
         %16 = OpLoad %5 %10
               OpBranch %15
         %15 = OpLabel
         %17 = OpPhi %14 %12 %16 %13
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(14);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(16);
  Instruction* phi = context->get_def_use_mgr()->GetDef(17);
  EXPECT_NE(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
  EXPECT_NE(vtable.GetValueNumber(inst1), vtable.GetValueNumber(phi));
  EXPECT_NE(vtable.GetValueNumber(inst2), vtable.GetValueNumber(phi));
}

// Test that a phi node in a loop header gets a new value because one of its
// inputs comes from later in the loop.
TEST_F(ValueTableTest, PhiLoopTest) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Uniform %5
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %9 = OpVariable %6 Uniform
         %10 = OpVariable %6 Uniform
          %2 = OpFunction %3 None %4
         %11 = OpLabel
         %12 = OpLoad %5 %9
               OpSelectionMerge %13 None
               OpBranchConditional %8 %14 %13
         %14 = OpLabel
         %15 = OpPhi %5 %12 %11 %16 %14
         %16 = OpLoad %5 %9
               OpLoopMerge %17 %14 None
               OpBranchConditional %8 %14 %17
         %17 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %18 = OpPhi %5 %12 %11 %16 %17
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(12);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(16);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));

  Instruction* phi1 = context->get_def_use_mgr()->GetDef(15);
  EXPECT_NE(vtable.GetValueNumber(inst1), vtable.GetValueNumber(phi1));

  Instruction* phi2 = context->get_def_use_mgr()->GetDef(18);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(phi2));
  EXPECT_NE(vtable.GetValueNumber(phi1), vtable.GetValueNumber(phi2));
}

// Test to make sure that OpPhi instructions with no in operands are handled
// correctly.
TEST_F(ValueTableTest, EmptyPhiTest) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
          %2 = OpFunction %void None %4
          %7 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %true %9 %8
          %9 = OpLabel
               OpKill
          %8 = OpLabel
         %10 = OpPhi %bool
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst = context->get_def_use_mgr()->GetDef(10);
  vtable.GetValueNumber(inst);
}

TEST_F(ValueTableTest, RedundantSampledImageLoad) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_FragColor
               OpExecutionMode %main OriginLowerLeft
               OpSource GLSL 330
               OpName %main "main"
               OpName %tex0 "tex0"
               OpName %gl_FragColor "gl_FragColor"
               OpDecorate %tex0 Location 0
               OpDecorate %tex0 DescriptorSet 0
               OpDecorate %tex0 Binding 0
               OpDecorate %gl_FragColor Location 0
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %9 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %10 = OpTypeSampledImage %9
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
       %tex0 = OpVariable %_ptr_UniformConstant_10 UniformConstant
%_ptr_Output_v4float = OpTypePointer Output %v4float
         %13 = OpConstantNull %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
         %14 = OpUndef %v4float
       %main = OpFunction %void None %6
         %15 = OpLabel
         %16 = OpLoad %10 %tex0
         %17 = OpImageSampleProjImplicitLod %v4float %16 %13
         %18 = OpImageSampleProjImplicitLod %v4float %16 %13
         %19 = OpFAdd %v4float %18 %17
               OpStore %gl_FragColor %19
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* load1 = context->get_def_use_mgr()->GetDef(17);
  Instruction* load2 = context->get_def_use_mgr()->GetDef(18);
  EXPECT_EQ(vtable.GetValueNumber(load1), vtable.GetValueNumber(load2));
}

TEST_F(ValueTableTest, DifferentDebugLocalVariableSameValue) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
          %2 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %3 "main"
               OpExecutionMode %3 OriginUpperLeft
               OpSource GLSL 430
          %4 = OpString "test"
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 32
          %9 = OpExtInst %5 %2 DebugSource %4
         %10 = OpExtInst %5 %2 DebugCompilationUnit 1 4 %9 HLSL
         %11 = OpExtInst %5 %2 DebugTypeBasic %4 %8 Float
         %12 = OpExtInst %5 %2 DebugLocalVariable %4 %11 %9 0 0 %10 FlagIsLocal
         %13 = OpExtInst %5 %2 DebugLocalVariable %4 %11 %9 0 0 %10 FlagIsLocal
          %3 = OpFunction %5 None %6
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(12);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(13);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, DifferentDebugValueSameValue) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
          %2 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %3 "main"
               OpExecutionMode %3 OriginUpperLeft
               OpSource GLSL 430
          %4 = OpString "test"
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 32
          %9 = OpExtInst %5 %2 DebugSource %4
         %10 = OpExtInst %5 %2 DebugCompilationUnit 1 4 %9 HLSL
         %11 = OpExtInst %5 %2 DebugTypeBasic %4 %8 Float
         %12 = OpExtInst %5 %2 DebugLocalVariable %4 %11 %9 0 0 %10 FlagIsLocal
         %13 = OpExtInst %5 %2 DebugExpression
          %3 = OpFunction %5 None %6
         %14 = OpLabel
         %15 = OpExtInst %5 %2 DebugValue %12 %8 %13
         %16 = OpExtInst %5 %2 DebugValue %12 %8 %13
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(15);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(16);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

TEST_F(ValueTableTest, DifferentDebugDeclareSameValue) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
          %2 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %3 "main"
               OpExecutionMode %3 OriginUpperLeft
               OpSource GLSL 430
          %4 = OpString "test"
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
    %uint_32 = OpConstant %uint 32
         %10 = OpExtInst %void %2 DebugSource %4
         %11 = OpExtInst %void %2 DebugCompilationUnit 1 4 %10 HLSL
         %12 = OpExtInst %void %2 DebugTypeBasic %4 %uint_32 Float
         %13 = OpExtInst %void %2 DebugLocalVariable %4 %12 %10 0 0 %11 FlagIsLocal
         %14 = OpExtInst %void %2 DebugExpression
          %3 = OpFunction %void None %6
         %15 = OpLabel
         %16 = OpVariable %_ptr_Function_uint Function
         %17 = OpExtInst %void %2 DebugDeclare %13 %16 %14
         %18 = OpExtInst %void %2 DebugDeclare %13 %16 %14
               OpReturn
               OpFunctionEnd
  )";
  auto context = BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);
  ValueNumberTable vtable(context.get());
  Instruction* inst1 = context->get_def_use_mgr()->GetDef(17);
  Instruction* inst2 = context->get_def_use_mgr()->GetDef(18);
  EXPECT_EQ(vtable.GetValueNumber(inst1), vtable.GetValueNumber(inst2));
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
