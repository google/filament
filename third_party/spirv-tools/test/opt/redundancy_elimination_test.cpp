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
#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using ::testing::HasSubstr;
using ::testing::MatchesRegex;
using RedundancyEliminationTest = PassTest<::testing::Test>;

// Test that it can get a simple case of local redundancy elimination.
// The rest of the test check for extra functionality.
TEST_F(RedundancyEliminationTest, RemoveRedundantLocalAdd) {
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
; CHECK: OpFAdd
; CHECK-NOT: OpFAdd
         %11 = OpFAdd %5 %9 %9
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<RedundancyEliminationPass>(text, false);
}

// Remove a redundant add across basic blocks.
TEST_F(RedundancyEliminationTest, RemoveRedundantAdd) {
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
; CHECK: OpFAdd
; CHECK-NOT: OpFAdd
         %12 = OpFAdd %5 %9 %9
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<RedundancyEliminationPass>(text, false);
}

// Remove a redundant add going through a multiple basic blocks.
TEST_F(RedundancyEliminationTest, RemoveRedundantAddDiamond) {
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
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
         %10 = OpVariable %6 Function
         %11 = OpLoad %5 %10
         %12 = OpFAdd %5 %11 %11
; CHECK: OpFAdd
; CHECK-NOT: OpFAdd
               OpBranchConditional %8 %13 %14
         %13 = OpLabel
               OpBranch %15
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
         %16 = OpFAdd %5 %11 %11
               OpReturn
               OpFunctionEnd

  )";
  SinglePassRunAndMatch<RedundancyEliminationPass>(text, false);
}

// Remove a redundant add in a side node.
TEST_F(RedundancyEliminationTest, RemoveRedundantAddInSideNode) {
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
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
         %10 = OpVariable %6 Function
         %11 = OpLoad %5 %10
         %12 = OpFAdd %5 %11 %11
; CHECK: OpFAdd
; CHECK-NOT: OpFAdd
               OpBranchConditional %8 %13 %14
         %13 = OpLabel
               OpBranch %15
         %14 = OpLabel
         %16 = OpFAdd %5 %11 %11
               OpBranch %15
         %15 = OpLabel
               OpReturn
               OpFunctionEnd

  )";
  SinglePassRunAndMatch<RedundancyEliminationPass>(text, false);
}

// Remove a redundant add whose value is in the result of a phi node.
TEST_F(RedundancyEliminationTest, RemoveRedundantAddWithPhi) {
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
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
         %10 = OpVariable %6 Function
         %11 = OpLoad %5 %10
               OpBranchConditional %8 %13 %14
         %13 = OpLabel
         %add1 = OpFAdd %5 %11 %11
; CHECK: OpFAdd
               OpBranch %15
         %14 = OpLabel
         %add2 = OpFAdd %5 %11 %11
; CHECK: OpFAdd
               OpBranch %15
         %15 = OpLabel
; CHECK: OpPhi
          %phi = OpPhi %5 %add1 %13 %add2 %14
; CHECK-NOT: OpFAdd
         %16 = OpFAdd %5 %11 %11
               OpReturn
               OpFunctionEnd

  )";
  SinglePassRunAndMatch<RedundancyEliminationPass>(text, false);
}

// Keep the add because it is redundant on some paths, but not all paths.
TEST_F(RedundancyEliminationTest, KeepPartiallyRedundantAdd) {
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
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
         %10 = OpVariable %6 Function
         %11 = OpLoad %5 %10
               OpBranchConditional %8 %13 %14
         %13 = OpLabel
        %add = OpFAdd %5 %11 %11
               OpBranch %15
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
         %16 = OpFAdd %5 %11 %11
               OpReturn
               OpFunctionEnd

  )";
  auto result = SinglePassRunAndDisassemble<RedundancyEliminationPass>(
      text, /* skip_nop = */ true, /* do_validation = */ false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

// Keep the add.  Even if it is redundant on all paths, there is no single id
// whose definition dominates the add and contains the same value.
TEST_F(RedundancyEliminationTest, KeepRedundantAddWithoutPhi) {
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
          %7 = OpTypeBool
          %8 = OpConstantTrue %7
          %2 = OpFunction %3 None %4
          %9 = OpLabel
         %10 = OpVariable %6 Function
         %11 = OpLoad %5 %10
               OpBranchConditional %8 %13 %14
         %13 = OpLabel
         %add1 = OpFAdd %5 %11 %11
               OpBranch %15
         %14 = OpLabel
         %add2 = OpFAdd %5 %11 %11
               OpBranch %15
         %15 = OpLabel
         %16 = OpFAdd %5 %11 %11
               OpReturn
               OpFunctionEnd

  )";
  auto result = SinglePassRunAndDisassemble<RedundancyEliminationPass>(
      text, /* skip_nop = */ true, /* do_validation = */ false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

// Test that it can get a simple case of local redundancy elimination
// when it has OpenCL.DebugInfo.100 instructions.
TEST_F(RedundancyEliminationTest, OpenCLDebugInfo100) {
  // When three redundant DebugValues exist, only one DebugValue must remain.
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %3 "main"
               OpExecutionMode %3 OriginUpperLeft
               OpSource GLSL 430
          %4 = OpString "ps.hlsl"
          %5 = OpString "float"
          %6 = OpString "s0"
          %7 = OpString "main"
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
    %uint_32 = OpConstant %uint 32
%_ptr_Function_float = OpTypePointer Function %float
         %15 = OpExtInst %void %1 DebugExpression
         %16 = OpExtInst %void %1 DebugSource %4
         %17 = OpExtInst %void %1 DebugCompilationUnit 1 4 %16 HLSL
         %18 = OpExtInst %void %1 DebugTypeBasic %5 %uint_32 Float
         %19 = OpExtInst %void %1 DebugTypeVector %18 4
         %20 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %19
         %21 = OpExtInst %void %1 DebugFunction %7 %20 %16 4 1 %17 %7 FlagIsProtected|FlagIsPrivate 4 %3
; CHECK:     [[dbg_local_var:%\w+]] = OpExtInst %void {{%\w+}} DebugLocalVariable
         %22 = OpExtInst %void %1 DebugLocalVariable %6 %19 %16 0 0 %21 FlagIsLocal
         %14 = OpExtInst %void %1 DebugLocalVariable %6 %19 %16 0 0 %21 FlagIsLocal
          %3 = OpFunction %void None %9
         %23 = OpLabel
         %24 = OpExtInst %void %1 DebugScope %21
         %25 = OpVariable %_ptr_Function_float Function
         %26 = OpLoad %float %25
               OpLine %4 0 0
; Two `OpFAdd %float %26 %26` are the same. One must be removed.
; After removing one `OpFAdd %float %26 %26`, two DebugValues are the same.
; One must be removed.
;
; CHECK:      OpLine {{%\w+}} 0 0
; CHECK-NEXT: [[add:%\w+]] = OpFAdd %float [[value:%\w+]]
; CHECK-NEXT: DebugValue [[dbg_local_var]] [[add]]
; CHECK-NEXT: OpLine {{%\w+}} 1 0
; CHECK-NEXT: OpFAdd %float [[add]] [[value]]
; CHECK-NEXT: OpReturn
         %27 = OpFAdd %float %26 %26
         %28 = OpExtInst %void %1 DebugValue %22 %27 %15 %uint_0
               OpLine %4 1 0
         %29 = OpFAdd %float %26 %26
         %30 = OpExtInst %void %1 DebugValue %14 %29 %15 %uint_0
         %31 = OpExtInst %void %1 DebugValue %22 %29 %15 %uint_0
         %32 = OpFAdd %float %29 %26
         %33 = OpFAdd %float %27 %26
               OpReturn
               OpFunctionEnd
  )";
  SinglePassRunAndMatch<RedundancyEliminationPass>(text, false);
}

TEST_F(RedundancyEliminationTest, FunctionDeclaration) {
  // Make sure the pass works with a function declaration that is called.
  const std::string text = R"(OpCapability Addresses
OpCapability Linkage
OpCapability Kernel
OpCapability Int8
%1 = OpExtInstImport "OpenCL.std"
OpMemoryModel Physical64 OpenCL
OpEntryPoint Kernel %2 "_Z23julia__1166_kernel_77094Bool"
OpExecutionMode %2 ContractionOff
OpSource Unknown 0
OpDecorate %3 LinkageAttributes "julia_error_7712" Import
%void = OpTypeVoid
%5 = OpTypeFunction %void
%3 = OpFunction %void None %5
OpFunctionEnd
%2 = OpFunction %void None %5
%6 = OpLabel
%7 = OpFunctionCall %void %3
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<RedundancyEliminationPass>(text, text, false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools