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

#include "gmock/gmock.h"
#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using WrapOpKillTest = PassTest<::testing::Test>;

TEST_F(WrapOpKillTest, SingleOpKill) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment [[main:%\w+]]
; CHECK: [[main]] = OpFunction
; CHECK: OpFunctionCall %void [[orig_kill:%\w+]]
; CHECK: [[orig_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK-NEXT: OpReturn
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpKill
; CHECK-NEXT: OpFunctionEnd
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpName %main "main"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %5
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %true %13 %10
         %13 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %14 = OpFunctionCall %void %kill_
               OpBranch %9
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
      %kill_ = OpFunction %void None %5
         %15 = OpLabel
               OpKill
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

TEST_F(WrapOpKillTest, MultipleOpKillInSameFunc) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment [[main:%\w+]]
; CHECK: [[main]] = OpFunction
; CHECK: OpFunctionCall %void [[orig_kill:%\w+]]
; CHECK: [[orig_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpSelectionMerge
; CHECK-NEXT: OpBranchConditional
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK-NEXT: OpReturn
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill]]
; CHECK-NEXT: OpReturn
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpKill
; CHECK-NEXT: OpFunctionEnd
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpName %main "main"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %5
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %true %13 %10
         %13 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %14 = OpFunctionCall %void %kill_
               OpBranch %9
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
      %kill_ = OpFunction %void None %5
         %15 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %true %17 %18
         %17 = OpLabel
               OpKill
         %18 = OpLabel
               OpKill
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

TEST_F(WrapOpKillTest, MultipleOpKillInDifferentFunc) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment [[main:%\w+]]
; CHECK: [[main]] = OpFunction
; CHECK: OpFunctionCall %void [[orig_kill1:%\w+]]
; CHECK-NEXT: OpFunctionCall %void [[orig_kill2:%\w+]]
; CHECK: [[orig_kill1]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK-NEXT: OpReturn
; CHECK: [[orig_kill2]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill]]
; CHECK-NEXT: OpReturn
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpKill
; CHECK-NEXT: OpFunctionEnd
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpName %main "main"
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %4
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpLoopMerge %9 %10 None
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %true %12 %9
         %12 = OpLabel
               OpBranch %10
         %10 = OpLabel
         %13 = OpFunctionCall %void %14
         %15 = OpFunctionCall %void %16
               OpBranch %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
         %14 = OpFunction %void None %4
         %17 = OpLabel
               OpKill
               OpFunctionEnd
         %16 = OpFunction %void None %4
         %18 = OpLabel
               OpKill
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

TEST_F(WrapOpKillTest, SingleOpTerminateInvocation) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment [[main:%\w+]]
; CHECK: [[main]] = OpFunction
; CHECK: OpFunctionCall %void [[orig_kill:%\w+]]
; CHECK: [[orig_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK-NEXT: OpReturn
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpTerminateInvocation
; CHECK-NEXT: OpFunctionEnd
               OpCapability Shader
               OpExtension "SPV_KHR_terminate_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpName %main "main"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %5
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %true %13 %10
         %13 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %14 = OpFunctionCall %void %kill_
               OpBranch %9
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
      %kill_ = OpFunction %void None %5
         %15 = OpLabel
               OpTerminateInvocation
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

TEST_F(WrapOpKillTest, MultipleTerminateInvocationInSameFunc) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment [[main:%\w+]]
; CHECK: [[main]] = OpFunction
; CHECK: OpFunctionCall %void [[orig_kill:%\w+]]
; CHECK: [[orig_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpSelectionMerge
; CHECK-NEXT: OpBranchConditional
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK-NEXT: OpReturn
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill]]
; CHECK-NEXT: OpReturn
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpTerminateInvocation
; CHECK-NEXT: OpFunctionEnd
               OpCapability Shader
               OpExtension "SPV_KHR_terminate_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpName %main "main"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %5
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %true %13 %10
         %13 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %14 = OpFunctionCall %void %kill_
               OpBranch %9
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
      %kill_ = OpFunction %void None %5
         %15 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %true %17 %18
         %17 = OpLabel
               OpTerminateInvocation
         %18 = OpLabel
               OpTerminateInvocation
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

TEST_F(WrapOpKillTest, MultipleOpTerminateInvocationDifferentFunc) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment [[main:%\w+]]
; CHECK: [[main]] = OpFunction
; CHECK: OpFunctionCall %void [[orig_kill1:%\w+]]
; CHECK-NEXT: OpFunctionCall %void [[orig_kill2:%\w+]]
; CHECK: [[orig_kill1]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK-NEXT: OpReturn
; CHECK: [[orig_kill2]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill]]
; CHECK-NEXT: OpReturn
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpTerminateInvocation
; CHECK-NEXT: OpFunctionEnd
               OpCapability Shader
               OpExtension "SPV_KHR_terminate_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpName %main "main"
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %4
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpLoopMerge %9 %10 None
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %true %12 %9
         %12 = OpLabel
               OpBranch %10
         %10 = OpLabel
         %13 = OpFunctionCall %void %14
         %15 = OpFunctionCall %void %16
               OpBranch %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
         %14 = OpFunction %void None %4
         %17 = OpLabel
               OpTerminateInvocation
               OpFunctionEnd
         %16 = OpFunction %void None %4
         %18 = OpLabel
               OpTerminateInvocation
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

TEST_F(WrapOpKillTest, KillAndTerminateInvocationSameFunc) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment [[main:%\w+]]
; CHECK: [[main]] = OpFunction
; CHECK: OpFunctionCall %void [[orig_kill:%\w+]]
; CHECK: [[orig_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpSelectionMerge
; CHECK-NEXT: OpBranchConditional
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK-NEXT: OpReturn
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_terminate:%\w+]]
; CHECK-NEXT: OpReturn
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpKill
; CHECK-NEXT: OpFunctionEnd
; CHECK-NEXT: [[new_terminate]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpTerminateInvocation
; CHECK-NEXT: OpFunctionEnd
               OpCapability Shader
               OpExtension "SPV_KHR_terminate_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpName %main "main"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %5
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %true %13 %10
         %13 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %14 = OpFunctionCall %void %kill_
               OpBranch %9
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
      %kill_ = OpFunction %void None %5
         %15 = OpLabel
               OpSelectionMerge %16 None
               OpBranchConditional %true %17 %18
         %17 = OpLabel
               OpKill
         %18 = OpLabel
               OpTerminateInvocation
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

TEST_F(WrapOpKillTest, KillAndTerminateInvocationDifferentFunc) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment [[main:%\w+]]
; CHECK: [[main]] = OpFunction
; CHECK: OpFunctionCall %void [[orig_kill1:%\w+]]
; CHECK-NEXT: OpFunctionCall %void [[orig_kill2:%\w+]]
; CHECK: [[orig_kill1]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_terminate:%\w+]]
; CHECK-NEXT: OpReturn
; CHECK: [[orig_kill2]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK-NEXT: OpReturn
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpKill
; CHECK-NEXT: OpFunctionEnd
; CHECK-NEXT: [[new_terminate]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpTerminateInvocation
; CHECK-NEXT: OpFunctionEnd
               OpCapability Shader
               OpExtension "SPV_KHR_terminate_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpName %main "main"
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %4
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpLoopMerge %9 %10 None
               OpBranch %11
         %11 = OpLabel
               OpBranchConditional %true %12 %9
         %12 = OpLabel
               OpBranch %10
         %10 = OpLabel
         %13 = OpFunctionCall %void %14
         %15 = OpFunctionCall %void %16
               OpBranch %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
         %14 = OpFunction %void None %4
         %17 = OpLabel
               OpTerminateInvocation
               OpFunctionEnd
         %16 = OpFunction %void None %4
         %18 = OpLabel
               OpKill
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

TEST_F(WrapOpKillTest, FuncWithReturnValue) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment [[main:%\w+]]
; CHECK: [[main]] = OpFunction
; CHECK: OpFunctionCall %int [[orig_kill:%\w+]]
; CHECK: [[orig_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK-NEXT: [[undef:%\w+]] = OpUndef %int
; CHECK-NEXT: OpReturnValue [[undef]]
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpKill
; CHECK-NEXT: OpFunctionEnd
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpName %main "main"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
        %int = OpTypeInt 32 1
  %func_type = OpTypeFunction %int
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %5
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %true %13 %10
         %13 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %14 = OpFunctionCall %int %kill_
               OpBranch %9
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
      %kill_ = OpFunction %int None %func_type
         %15 = OpLabel
               OpKill
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

TEST_F(WrapOpKillTest, IdBoundOverflow1) {
  const std::string text = R"(
OpCapability GeometryStreams
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%bool = OpTypeBool
%true = OpConstantTrue %bool
%main = OpFunction %2 None %3
%8 = OpLabel
OpBranch %9
%9 = OpLabel
OpLoopMerge %10 %11 None
OpBranch %12
%12 = OpLabel
OpBranchConditional %true %13 %10
%13 = OpLabel
OpBranch %11
%11 = OpLabel
%14 = OpFunctionCall %void %kill_
OpBranch %9
%10 = OpLabel
OpReturn
OpFunctionEnd
%kill_ = OpFunction %2 Pure|Const %3
%4194302 = OpLabel
OpKill
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<WrapOpKill>(text, true);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

TEST_F(WrapOpKillTest, IdBoundOverflow2) {
  const std::string text = R"(
OpCapability GeometryStreams
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%bool = OpTypeBool
%true = OpConstantTrue %bool
%main = OpFunction %2 None %3
%8 = OpLabel
OpBranch %9
%9 = OpLabel
OpLoopMerge %10 %11 None
OpBranch %12
%12 = OpLabel
OpBranchConditional %true %13 %10
%13 = OpLabel
OpBranch %11
%11 = OpLabel
%14 = OpFunctionCall %void %kill_
OpBranch %9
%10 = OpLabel
OpReturn
OpFunctionEnd
%kill_ = OpFunction %2 Pure|Const %3
%4194301 = OpLabel
OpKill
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<WrapOpKill>(text, true);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

TEST_F(WrapOpKillTest, IdBoundOverflow3) {
  const std::string text = R"(
OpCapability GeometryStreams
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%bool = OpTypeBool
%true = OpConstantTrue %bool
%main = OpFunction %2 None %3
%8 = OpLabel
OpBranch %9
%9 = OpLabel
OpLoopMerge %10 %11 None
OpBranch %12
%12 = OpLabel
OpBranchConditional %true %13 %10
%13 = OpLabel
OpBranch %11
%11 = OpLabel
%14 = OpFunctionCall %void %kill_
OpBranch %9
%10 = OpLabel
OpReturn
OpFunctionEnd
%kill_ = OpFunction %2 Pure|Const %3
%4194300 = OpLabel
OpKill
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<WrapOpKill>(text, true);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

TEST_F(WrapOpKillTest, IdBoundOverflow4) {
  const std::string text = R"(
OpCapability DerivativeControl
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %2 Location 539091968
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%bool = OpTypeBool
%true = OpConstantTrue %bool
%main = OpFunction %2 None %3
%8 = OpLabel
OpBranch %9
%9 = OpLabel
OpLoopMerge %10 %11 None
OpBranch %12
%12 = OpLabel
OpBranchConditional %true %13 %10
%13 = OpLabel
OpBranch %11
%11 = OpLabel
%14 = OpFunctionCall %void %kill_
OpBranch %9
%10 = OpLabel
OpReturn
OpFunctionEnd
%kill_ = OpFunction %2 Inline|Pure|Const %3
%4194302 = OpLabel
OpKill
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<WrapOpKill>(text, true);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

TEST_F(WrapOpKillTest, IdBoundOverflow5) {
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main"
               OpExecutionMode %1 OriginUpperLeft
               OpDecorate %void Location 539091968
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
  %_struct_5 = OpTypeStruct %float %float
  %_struct_6 = OpTypeStruct %_struct_5
%_ptr_Function__struct_6 = OpTypePointer Function %_struct_6
%_ptr_Output_float = OpTypePointer Output %float
          %9 = OpTypeFunction %_struct_5 %_ptr_Function__struct_6
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
          %1 = OpFunction %void None %3
         %12 = OpLabel
         %13 = OpVariable %_ptr_Function__struct_6 Function
               OpBranch %14
         %14 = OpLabel
               OpLoopMerge %15 %16 None
               OpBranch %17
         %17 = OpLabel
               OpBranchConditional %true %18 %15
         %18 = OpLabel
               OpBranch %16
         %16 = OpLabel
         %19 = OpFunctionCall %void %20
         %21 = OpFunctionCall %_struct_5 %22 %13
               OpBranch %14
         %15 = OpLabel
               OpReturn
               OpFunctionEnd
         %20 = OpFunction %void Inline|Pure|Const %3
         %23 = OpLabel
         %24 = OpVariable %_ptr_Function__struct_6 Function
         %25 = OpFunctionCall %_struct_5 %26 %24
               OpKill
               OpFunctionEnd
         %26 = OpFunction %_struct_5 None %9
         %27 = OpLabel
               OpUnreachable
               OpFunctionEnd
         %22 = OpFunction %_struct_5 Inline %9
    %4194295 = OpLabel
               OpKill
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<WrapOpKill>(text, true);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

TEST_F(WrapOpKillTest, SkipEntryPoint) {
  const std::string text = R"(
OpCapability GeometryStreams
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main"
OpExecutionMode %4 OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%4 = OpFunction %2 Pure|Const %3
%5 = OpLabel
OpKill
OpFunctionEnd
)";

  auto result = SinglePassRunToBinary<WrapOpKill>(text, true);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

TEST_F(WrapOpKillTest, SkipFunctionNotInContinue) {
  const std::string text = R"(
OpCapability GeometryStreams
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%bool = OpTypeBool
%true = OpConstantTrue %bool
%main = OpFunction %2 None %3
%6 = OpLabel
%7 = OpFunctionCall %void %4
OpReturn
OpFunctionEnd
%4 = OpFunction %2 Pure|Const %3
%5 = OpLabel
OpKill
OpFunctionEnd
)";

  auto result = SinglePassRunToBinary<WrapOpKill>(text, true);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

TEST_F(WrapOpKillTest, SetParentBlock) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%undef = OpUndef %bool
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpBranch %loop
%loop = OpLabel
OpLoopMerge %merge %continue None
OpBranchConditional %undef %merge %continue
%continue = OpLabel
%call = OpFunctionCall %void %kill_func
OpBranch %loop
%merge = OpLabel
OpReturn
OpFunctionEnd
%kill_func = OpFunction %void None %void_fn
%kill_entry = OpLabel
OpKill
OpFunctionEnd
)";

  auto result = SinglePassRunToBinary<WrapOpKill>(text, true);
  EXPECT_EQ(Pass::Status::SuccessWithChange, std::get<1>(result));
  result = SinglePassRunToBinary<WrapOpKill>(text, true);
  EXPECT_EQ(Pass::Status::SuccessWithChange, std::get<1>(result));
}

TEST_F(WrapOpKillTest, KillInSingleBlockLoop) {
  const std::string text = R"(
; CHECK: OpFunction %void
; CHECK: OpFunction %void
; CHECK-NOT: OpKill
; CHECK: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK-NOT: OpKill
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpKill
; CHECK-NEXT: OpFunctionEnd
              OpCapability Shader
              OpCapability Linkage
              OpMemoryModel Logical GLSL450
      %void = OpTypeVoid
      %bool = OpTypeBool
     %undef = OpUndef %bool
   %void_fn = OpTypeFunction %void
      %main = OpFunction %void None %void_fn
%main_entry = OpLabel
              OpBranch %loop
      %loop = OpLabel
      %call = OpFunctionCall %void %sub
              OpLoopMerge %exit %loop None
              OpBranchConditional %undef %loop %exit
      %exit = OpLabel
              OpReturn
              OpFunctionEnd
       %sub = OpFunction %void None %void_fn
 %sub_entry = OpLabel
              OpSelectionMerge %ret None
              OpBranchConditional %undef %kill %ret
      %kill = OpLabel
              OpKill
       %ret = OpLabel
              OpReturn
              OpFunctionEnd
)";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

TEST_F(WrapOpKillTest, DebugInfoSimple) {
  const std::string text = R"(
; CHECK: OpEntryPoint Fragment [[main:%\w+]]
; CHECK: [[main]] = OpFunction
; CHECK: OpFunctionCall %void [[orig_kill:%\w+]]
; CHECK: [[orig_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: {{%\d+}} = OpExtInst %void [[ext:%\d+]] DebugScope
; CHECK-NEXT: OpLine [[file:%\d+]] 100 200
; CHECK-NEXT: OpFunctionCall %void [[new_kill:%\w+]]
; CHECK:      {{%\d+}} = OpExtInst %void [[ext]] DebugNoScope
; CHECK-NEXT: OpReturn
; CHECK: [[new_kill]] = OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpKill
; CHECK-NEXT: OpFunctionEnd
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
          %2 = OpString "File name"
               OpSource GLSL 330
               OpName %main "main"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
          %3 = OpExtInst %void %1 DebugSource %2
          %4 = OpExtInst %void %1 DebugCompilationUnit 0 0 %3 GLSL
       %main = OpFunction %void None %5
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %10 %11 None
               OpBranch %12
         %12 = OpLabel
               OpBranchConditional %true %13 %10
         %13 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %14 = OpFunctionCall %void %kill_
               OpBranch %9
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
      %kill_ = OpFunction %void None %5
         %15 = OpLabel
         %16 = OpExtInst %void %1 DebugScope %4
               OpLine %2 100 200
               OpKill
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<WrapOpKill>(text, true);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
