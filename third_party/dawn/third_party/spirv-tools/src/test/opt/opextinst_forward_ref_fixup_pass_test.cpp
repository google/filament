// Copyright (c) 2024 Google LLC
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

#include "spirv-tools/optimizer.hpp"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using OpExtInstForwardRefFixupPassTest = PassTest<::testing::Test>;

TEST_F(OpExtInstForwardRefFixupPassTest, NoChangeWithougExtendedInstructions) {
  const std::string kTest = R"(
; CHECK-NOT:   SomeOpcode
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd;
  )";
  const auto result =
      SinglePassRunAndMatch<OpExtInstWithForwardReferenceFixupPass>(
          kTest, /* do_validation= */ true);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithoutChange);
}

TEST_F(OpExtInstForwardRefFixupPassTest, NoForwardRef_NoChange) {
  const std::string kTest = R"(OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%3 = OpString "/usr/local/google/home/nathangauer/projects/DirectXShaderCompiler/repro.hlsl"
%4 = OpString "// RUN: %dxc -T cs_6_0 %s -E main -spirv -fspv-target-env=vulkan1.1 -fspv-debug=vulkan-with-source | FileCheck %s

[numthreads(1, 1, 1)]
void main() {
}
"
%5 = OpString "main"
%6 = OpString ""
%7 = OpString "3f3d3740"
%8 = OpString " -E main -T cs_6_0 -spirv -fspv-target-env=vulkan1.1 -fspv-debug=vulkan-with-source -Qembed_debug"
OpName %main "main"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint_3 = OpConstant %uint 3
%uint_1 = OpConstant %uint 1
%uint_4 = OpConstant %uint 4
%uint_5 = OpConstant %uint 5
%15 = OpTypeFunction %void
%16 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
%17 = OpExtInst %void %1 DebugSource %3 %4
%18 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %17 %uint_5
%19 = OpExtInst %void %1 DebugFunction %5 %16 %17 %uint_4 %uint_1 %18 %6 %uint_3 %uint_4
%20 = OpExtInst %void %1 DebugEntryPoint %19 %18 %7 %8
%main = OpFunction %void None %15
%21 = OpLabel
%22 = OpExtInst %void %1 DebugFunctionDefinition %19 %main
%23 = OpExtInst %void %1 DebugLine %17 %uint_5 %uint_5 %uint_1 %uint_1
OpReturn
OpFunctionEnd
)";
  SinglePassRunAndCheck<OpExtInstWithForwardReferenceFixupPass>(
      kTest, kTest, /* skip_nop= */ false);
}

TEST_F(OpExtInstForwardRefFixupPassTest,
       NoForwardRef_ReplaceOpExtInstWithForwardWithOpExtInst) {
  const std::string kTest = R"(
                       OpCapability Shader
                       OpExtension "SPV_KHR_non_semantic_info"
                       OpExtension "SPV_KHR_relaxed_extended_instruction"
; CHECK-NOT:           OpExtension "SPV_KHR_relaxed_extended_instruction"
                  %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
                       OpMemoryModel Logical GLSL450
                       OpEntryPoint GLCompute %main "main"
                       OpExecutionMode %main LocalSize 1 1 1
                  %3 = OpString "/usr/local/google/home/nathangauer/projects/DirectXShaderCompiler/repro.hlsl"
                  %4 = OpString "// RUN: %dxc -T cs_6_0 %s -E main -spirv -fspv-target-env=vulkan1.1 -fspv-debug=vulkan-with-source | FileCheck %s

[numthreads(1, 1, 1)]
void main() {
}
"
                  %5 = OpString "main"
                  %6 = OpString ""
                  %7 = OpString "3f3d3740"
                  %8 = OpString " -E main -T cs_6_0 -spirv -fspv-target-env=vulkan1.1 -fspv-debug=vulkan-with-source -Qembed_debug"
                       OpName %main "main"
               %void = OpTypeVoid
               %uint = OpTypeInt 32 0
             %uint_3 = OpConstant %uint 3
             %uint_1 = OpConstant %uint 1
             %uint_4 = OpConstant %uint 4
             %uint_5 = OpConstant %uint 5
                 %20 = OpTypeFunction %void
                 %10 = OpExtInstWithForwardRefsKHR %void %1 DebugTypeFunction %uint_3 %void
                 %12 = OpExtInstWithForwardRefsKHR %void %1 DebugSource %3 %4
                 %13 = OpExtInstWithForwardRefsKHR %void %1 DebugCompilationUnit %uint_1 %uint_4 %12 %uint_5
                 %17 = OpExtInstWithForwardRefsKHR %void %1 DebugFunction %5 %10 %12 %uint_4 %uint_1 %13 %6 %uint_3 %uint_4
                 %18 = OpExtInstWithForwardRefsKHR %void %1 DebugEntryPoint %17 %13 %7 %8
;  CHECK-NOT: {{.*}} = OpExtInstWithForwardRefsKHR %void %1 DebugTypeFunction %uint_3 %void
;  CHECK-NOT: {{.*}} = OpExtInstWithForwardRefsKHR %void %1 DebugSource {{.*}} {{.*}}
;  CHECK-NOT: {{.*}} = OpExtInstWithForwardRefsKHR %void %1 DebugCompilationUnit %uint_1 %uint_4 {{.*}} %uint_5
;  CHECK-NOT: {{.*}} = OpExtInstWithForwardRefsKHR %void %1 DebugFunction {{.*}} {{.*}} {{.*}} %uint_4 %uint_1 {{.*}} {{.*}} %uint_3 %uint_4
;  CHECK-NOT: {{.*}} = OpExtInstWithForwardRefsKHR %void %1 DebugEntryPoint {{.*}} {{.*}} {{.*}} {{.*}}
;  CHECK:     {{.*}} = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
;  CHECK:     {{.*}} = OpExtInst %void %1 DebugSource {{.*}} {{.*}}
;  CHECK:     {{.*}} = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 {{.*}} %uint_5
;  CHECK:     {{.*}} = OpExtInst %void %1 DebugFunction {{.*}} {{.*}} {{.*}} %uint_4 %uint_1 {{.*}} {{.*}} %uint_3 %uint_4
;  CHECK:     {{.*}} = OpExtInst %void %1 DebugEntryPoint {{.*}} {{.*}} {{.*}} {{.*}}
               %main = OpFunction %void None %20
                 %21 = OpLabel
                 %22 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
                 %23 = OpExtInst %void %1 DebugLine %12 %uint_5 %uint_5 %uint_1 %uint_1
; CHECK:      {{.*}} = OpExtInst %void %1 DebugFunctionDefinition {{.*}} %main
; CHECK:      {{.*}} = OpExtInst %void %1 DebugLine {{.*}} %uint_5 %uint_5 %uint_1 %uint_1
                       OpReturn
                       OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<OpExtInstWithForwardReferenceFixupPass>(
          kTest, /* do_validation= */ true);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

TEST_F(OpExtInstForwardRefFixupPassTest, ForwardRefs_NoChange) {
  const std::string kTest = R"(OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
OpExtension "SPV_KHR_relaxed_extended_instruction"
%1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%3 = OpString "/usr/local/google/home/nathangauer/projects/DirectXShaderCompiler/repro.hlsl"
%4 = OpString "// RUN: %dxc -T cs_6_0 %s -E main -spirv -fspv-target-env=vulkan1.1 -fspv-debug=vulkan-with-source | FileCheck %s

class A {
  void foo() {
  }
};

[numthreads(1, 1, 1)]
void main() {
  A a;
  a.foo();
}
"
%5 = OpString "A"
%6 = OpString "A.foo"
%7 = OpString ""
%8 = OpString "this"
%9 = OpString "main"
%10 = OpString "a"
%11 = OpString "d59ae9c2"
%12 = OpString " -E main -T cs_6_0 -spirv -fspv-target-env=vulkan1.1 -fspv-debug=vulkan-with-source -Vd -Qembed_debug"
OpName %main "main"
OpName %A "A"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%uint_4 = OpConstant %uint 4
%uint_5 = OpConstant %uint 5
%uint_0 = OpConstant %uint 0
%uint_3 = OpConstant %uint 3
%uint_7 = OpConstant %uint 7
%uint_288 = OpConstant %uint 288
%uint_9 = OpConstant %uint 9
%uint_13 = OpConstant %uint 13
%uint_10 = OpConstant %uint 10
%26 = OpTypeFunction %void
%uint_12 = OpConstant %uint 12
%A = OpTypeStruct
%_ptr_Function_A = OpTypePointer Function %A
%uint_11 = OpConstant %uint 11
%30 = OpExtInst %void %1 DebugExpression
%31 = OpExtInst %void %1 DebugSource %3 %4
%32 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %31 %uint_5
%33 = OpExtInstWithForwardRefsKHR %void %1 DebugTypeComposite %5 %uint_0 %31 %uint_3 %uint_7 %32 %5 %uint_0 %uint_3 %34
%35 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void %33
%34 = OpExtInst %void %1 DebugFunction %6 %35 %31 %uint_4 %uint_3 %33 %7 %uint_3 %uint_4
%36 = OpExtInst %void %1 DebugLocalVariable %8 %33 %31 %uint_4 %uint_3 %34 %uint_288 %uint_1
%37 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
%38 = OpExtInst %void %1 DebugFunction %9 %37 %31 %uint_9 %uint_1 %32 %7 %uint_3 %uint_9
%39 = OpExtInst %void %1 DebugLexicalBlock %31 %uint_9 %uint_13 %38
%40 = OpExtInst %void %1 DebugLocalVariable %10 %33 %31 %uint_10 %uint_5 %39 %uint_4
%41 = OpExtInst %void %1 DebugEntryPoint %38 %32 %11 %12
%42 = OpExtInst %void %1 DebugInlinedAt %uint_11 %39
%main = OpFunction %void None %26
%43 = OpLabel
%44 = OpVariable %_ptr_Function_A Function
%45 = OpExtInst %void %1 DebugFunctionDefinition %38 %main
%57 = OpExtInst %void %1 DebugScope %39
%47 = OpExtInst %void %1 DebugLine %31 %uint_10 %uint_10 %uint_3 %uint_5
%48 = OpExtInst %void %1 DebugDeclare %40 %44 %30
%58 = OpExtInst %void %1 DebugScope %34 %42
%50 = OpExtInst %void %1 DebugLine %31 %uint_4 %uint_5 %uint_3 %uint_3
%51 = OpExtInst %void %1 DebugDeclare %36 %44 %30
%59 = OpExtInst %void %1 DebugNoScope
%53 = OpExtInst %void %1 DebugLine %31 %uint_12 %uint_12 %uint_1 %uint_1
OpReturn
OpFunctionEnd
)";
  SinglePassRunAndCheck<OpExtInstWithForwardReferenceFixupPass>(
      kTest, kTest, /* skip_nop= */ false);
}

TEST_F(OpExtInstForwardRefFixupPassTest,
       ForwardRefs_ReplaceOpExtInstWithOpExtInstWithForwardRefs) {
  const std::string kTest = R"(
                        OpCapability Shader
                        OpExtension "SPV_KHR_non_semantic_info"
; CHECK:                OpExtension "SPV_KHR_relaxed_extended_instruction"
                   %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
                        OpMemoryModel Logical GLSL450
                        OpEntryPoint GLCompute %main "main"
                        OpExecutionMode %main LocalSize 1 1 1
                   %3 = OpString "/usr/local/google/home/nathangauer/projects/DirectXShaderCompiler/repro.hlsl"
                   %4 = OpString "// RUN: %dxc -T cs_6_0 %s -E main -spirv -fspv-target-env=vulkan1.1 -fspv-debug=vulkan-with-source | FileCheck %s

class A {
  void foo() {
  }
};

[numthreads(1, 1, 1)]
void main() {
  A a;
  a.foo();
}
"
                   %5 = OpString "A"
                   %6 = OpString "A.foo"
                   %7 = OpString ""
                   %8 = OpString "this"
                   %9 = OpString "main"
                  %10 = OpString "a"
                  %11 = OpString "d59ae9c2"
                  %12 = OpString " -E main -T cs_6_0 -spirv -fspv-target-env=vulkan1.1 -fspv-debug=vulkan-with-source -Vd -Qembed_debug"
                        OpName %main "main"
                        OpName %A "A"
                %void = OpTypeVoid
                %uint = OpTypeInt 32 0
              %uint_1 = OpConstant %uint 1
              %uint_4 = OpConstant %uint 4
              %uint_5 = OpConstant %uint 5
              %uint_0 = OpConstant %uint 0
              %uint_3 = OpConstant %uint 3
              %uint_7 = OpConstant %uint 7
            %uint_288 = OpConstant %uint 288
              %uint_9 = OpConstant %uint 9
             %uint_13 = OpConstant %uint 13
             %uint_10 = OpConstant %uint 10
                  %40 = OpTypeFunction %void
             %uint_12 = OpConstant %uint 12
                   %A = OpTypeStruct
         %_ptr_Function_A = OpTypePointer Function %A
             %uint_11 = OpConstant %uint 11
                  %15 = OpExtInst %void %1 DebugExpression
                  %16 = OpExtInst %void %1 DebugSource %3 %4
                  %17 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %16 %uint_5
                  %21 = OpExtInst %void %1 DebugTypeComposite %5 %uint_0 %16 %uint_3 %uint_7 %17 %5 %uint_0 %uint_3 %25
                  %26 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void %21
                  %25 = OpExtInst %void %1 DebugFunction %6 %26 %16 %uint_4 %uint_3 %21 %7 %uint_3 %uint_4
                  %27 = OpExtInst %void %1 DebugLocalVariable %8 %21 %16 %uint_4 %uint_3 %25 %uint_288 %uint_1
                  %29 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
                  %30 = OpExtInst %void %1 DebugFunction %9 %29 %16 %uint_9 %uint_1 %17 %7 %uint_3 %uint_9
                  %32 = OpExtInst %void %1 DebugLexicalBlock %16 %uint_9 %uint_13 %30
                  %34 = OpExtInst %void %1 DebugLocalVariable %10 %21 %16 %uint_10 %uint_5 %32 %uint_4
                  %36 = OpExtInst %void %1 DebugEntryPoint %30 %17 %11 %12
                  %37 = OpExtInst %void %1 DebugInlinedAt %uint_11 %32
; CHECK:       {{.*}} = OpExtInst %void %1 DebugExpression
; CHECK:       {{.*}} = OpExtInst %void %1 DebugSource
; CHECK:       {{.*}} = OpExtInst %void %1 DebugCompilationUnit
; CHECK:       {{.*}} = OpExtInstWithForwardRefsKHR %void {{.*}} DebugTypeComposite
; CHECK-NOT:   {{.*}} = OpExtInst %void {{.*}} DebugTypeComposite
; CHECK:       {{.*}} = OpExtInst %void %1 DebugTypeFunction
; CHECK:       {{.*}} = OpExtInst %void %1 DebugFunction
; CHECK:       {{.*}} = OpExtInst %void %1 DebugLocalVariable
; CHECK:       {{.*}} = OpExtInst %void %1 DebugTypeFunction
; CHECK:       {{.*}} = OpExtInst %void %1 DebugFunction
; CHECK:       {{.*}} = OpExtInst %void %1 DebugLexicalBlock
; CHECK:       {{.*}} = OpExtInst %void %1 DebugLocalVariable
; CHECK:       {{.*}} = OpExtInst %void %1 DebugEntryPoint
; CHECK:       {{.*}} = OpExtInst %void %1 DebugInlinedAt
                %main = OpFunction %void None %40
                  %43 = OpLabel
                  %44 = OpVariable %_ptr_Function_A Function
                  %45 = OpExtInst %void %1 DebugFunctionDefinition %30 %main
                  %51 = OpExtInst %void %1 DebugScope %32
                  %46 = OpExtInst %void %1 DebugLine %16 %uint_10 %uint_10 %uint_3 %uint_5
                  %47 = OpExtInst %void %1 DebugDeclare %34 %44 %15
                  %52 = OpExtInst %void %1 DebugScope %25 %37
                  %48 = OpExtInst %void %1 DebugLine %16 %uint_4 %uint_5 %uint_3 %uint_3
                  %49 = OpExtInst %void %1 DebugDeclare %27 %44 %15
                  %53 = OpExtInst %void %1 DebugNoScope
                  %50 = OpExtInst %void %1 DebugLine %16 %uint_12 %uint_12 %uint_1 %uint_1
; CHECK:       {{.*}} = OpExtInst %void %1 DebugFunctionDefinition
; CHECK:       {{.*}} = OpExtInst %void %1 DebugScope
; CHECK:       {{.*}} = OpExtInst %void %1 DebugLine
; CHECK:       {{.*}} = OpExtInst %void %1 DebugDeclare
; CHECK:       {{.*}} = OpExtInst %void %1 DebugScope
; CHECK:       {{.*}} = OpExtInst %void %1 DebugLine
; CHECK:       {{.*}} = OpExtInst %void %1 DebugDeclare
; CHECK:       {{.*}} = OpExtInst %void %1 DebugNoScope
; CHECK:       {{.*}} = OpExtInst %void %1 DebugLine
                        OpReturn
                        OpFunctionEnd
  )";
  const auto result =
      SinglePassRunAndMatch<OpExtInstWithForwardReferenceFixupPass>(
          kTest, /* do_validation= */ true);
  EXPECT_EQ(std::get<1>(result), Pass::Status::SuccessWithChange);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
