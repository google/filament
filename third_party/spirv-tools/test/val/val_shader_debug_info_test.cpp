// Copyright (c) 2026 The Khronos Group Inc.
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

// Tests displaying ShaderDebugInfo in validation error messages

#include <string>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;

using ValidateShaderDebugInfo = spvtest::ValidateBase<bool>;

TEST_F(ValidateShaderDebugInfo, DebugLineSingleLine) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
         %19 = OpString "#version 450
layout(set = 0, binding = 1, std430) buffer SSBO {
    vec4 data;
};

void main() {
    data = vec4(0.0);
}"
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %_ Binding 1
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_0 = OpConstant %uint 0
         %18 = OpExtInst %void %1 DebugSource %2 %19
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %SSBO = OpTypeStruct %v4float
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
    %uint_12 = OpConstant %uint 12
          %_ = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
         %50 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
     %uint_7 = OpConstant %uint 7
       %main = OpFunction %void None %5
         %15 = OpLabel
         %54 = OpExtInst %void %1 DebugLine %18 %uint_7 %uint_7 %uint_0 %uint_0
         ;; invalid here
         %53 = OpAccessChain %_ptr_StorageBuffer_v4float %_ %int_0 %int_0 %int_0
               OpStore %53 %50
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_1);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(  --> a.comp:7:0
  |
7 |     data = vec4(0.0);
  |)"));
}

TEST_F(ValidateShaderDebugInfo, DebugNoLine) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
         %19 = OpString "#version 450
layout(set = 0, binding = 1, std430) buffer SSBO {
    vec4 data;
};

void main() {
    data = vec4(0.0);
}"
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %_ Binding 1
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_0 = OpConstant %uint 0
         %18 = OpExtInst %void %1 DebugSource %2 %19
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %SSBO = OpTypeStruct %v4float
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
    %uint_12 = OpConstant %uint 12
          %_ = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
         %50 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
     %uint_7 = OpConstant %uint 7
       %main = OpFunction %void None %5
         %15 = OpLabel
         %54 = OpExtInst %void %1 DebugLine %18 %uint_7 %uint_7 %uint_0 %uint_0
         %55 = OpExtInst %void %1 DebugNoLine
         %53 = OpAccessChain %_ptr_StorageBuffer_v4float %_ %int_0 %int_0 %int_0
         %56 = OpExtInst %void %1 DebugLine %18 %uint_7 %uint_7 %uint_0 %uint_0
               OpStore %53 %50
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_1);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(), Not(HasSubstr("a.comp")));
}

TEST_F(ValidateShaderDebugInfo, DebugLineMultieLine) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
         %19 = OpString "#version 450
layout(set = 0, binding = 1, std430) buffer SSBO {
    vec4 data;
};

void main() {
    data = vec4(0.0);
}"
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %_ Binding 1
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_0 = OpConstant %uint 0
         %18 = OpExtInst %void %1 DebugSource %2 %19
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %SSBO = OpTypeStruct %v4float
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
    %uint_12 = OpConstant %uint 12
          %_ = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
         %50 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
     %uint_6 = OpConstant %uint 6
     %uint_8 = OpConstant %uint 8
       %main = OpFunction %void None %5
         %15 = OpLabel
         %54 = OpExtInst %void %1 DebugLine %18 %uint_6 %uint_8 %uint_0 %uint_0
         ;; invalid here
         %53 = OpAccessChain %_ptr_StorageBuffer_v4float %_ %int_0 %int_0 %int_0
               OpStore %53 %50
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_1);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(  --> a.comp:6:0
  |
6 | void main() {
7 |     data = vec4(0.0);
8 | }
  |)"));
}

TEST_F(ValidateShaderDebugInfo, DebugSourceContinued) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
         %text = OpString "#version 450
layout(set = 0, binding = 1, std430) buffer SSBO {
    vec4 data;
};

"
          %text1 = OpString "void main() {"
          %text2 = OpString "
    data = vec4"
          %text3 = OpString "(0.0);
}"
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %_ Binding 1
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_0 = OpConstant %uint 0

   %d_source = OpExtInst %void %1 DebugSource %2 %text
         %d1 = OpExtInst %void %1 DebugSourceContinued %text1
         %d2 = OpExtInst %void %1 DebugSourceContinued %text2
         %d3 = OpExtInst %void %1 DebugSourceContinued %text3

         %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %SSBO = OpTypeStruct %v4float
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
    %uint_12 = OpConstant %uint 12
          %_ = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
         %50 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
     %uint_6 = OpConstant %uint 6
     %uint_8 = OpConstant %uint 8
       %main = OpFunction %void None %5
         %15 = OpLabel
         %54 = OpExtInst %void %1 DebugLine %d_source %uint_6 %uint_8 %uint_0 %uint_0
         ;; invalid here
         %53 = OpAccessChain %_ptr_StorageBuffer_v4float %_ %int_0 %int_0 %int_0
               OpStore %53 %50
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_1);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(  --> a.comp:6:0
  |
6 | void main() {
7 |     data = vec4(0.0);
8 | }
  |)"));
}

// Make sure we can handle various instructions where there DebugInfo is unused
TEST_F(ValidateShaderDebugInfo, UnusedOpFunction) {
  const std::string str = R"(
     OpCapability Shader
     OpCapability Linkage
     OpExtension "SPV_KHR_non_semantic_info"
%d = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
     OpMemoryModel Logical GLSL450
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%4 = OpFunction %1 None %2
%5 = OpLabel
     OpReturn
     OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpFunction Function Type <id> '3[%uint]' is not a function type."));
}

TEST_F(ValidateShaderDebugInfo, VariableBasic) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %x
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450
layout(set = 0, binding = 1, std430) buffer SSBO {
    vec4 data;
} x;

void main() {
    x.data = vec4(0.0);
}"
         %25 = OpString "float"
         %31 = OpString "data"
         %34 = OpString "SSBO"
         %40 = OpString "x"
         %46 = OpString "int"
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %x Binding 1
               OpDecorate %x DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %20 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_6 %uint_0 %20 %16 %uint_3 %uint_6
      %float = OpTypeFloat 32
         %26 = OpExtInst %void %1 DebugTypeBasic %25 %uint_32 %uint_3 %uint_0
    %v4float = OpTypeVector %float 4
         %28 = OpExtInst %void %1 DebugTypeVector %26 %uint_4
       %SSBO = OpTypeStruct %v4float
    %uint_10 = OpConstant %uint 10
         %30 = OpExtInst %void %1 DebugTypeMember %31 %28 %18 %uint_3 %uint_10 %uint_0 %uint_0 %uint_3
         %33 = OpExtInst %void %1 DebugTypeComposite %34 %uint_1 %18 %uint_2 %uint_0 %20 %34 %uint_0 %uint_3 %30
%_ptr_StorageBuffer_SSBO = OpTypePointer Uniform %SSBO
    %uint_12 = OpConstant %uint 12
         %37 = OpExtInst %void %1 DebugTypePointer %33 %uint_12 %uint_0
          %x = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
     %uint_8 = OpConstant %uint 8
         %39 = OpExtInst %void %1 DebugGlobalVariable %40 %33 %18 %uint_2 %uint_0 %20 %40 %x %uint_8
    %float_0 = OpConstant %float 0
         %50 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
       %main = OpFunction %void None %5
         %15 = OpLabel
         %53 = OpAccessChain %_ptr_StorageBuffer_v4float %x %uint_0
               OpStore %53 %50
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(  --> a.comp:2:0
  |
2 | layout(set = 0, binding = 1, std430) buffer SSBO {
  |)"));
}

// Test if no DebugGlobalVariable is found, things still work
TEST_F(ValidateShaderDebugInfo, NoDebugGlobalVariable) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %x
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %19 = OpString "BAD"
         %25 = OpString "float"
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %x Binding 1
               OpDecorate %x DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %20 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
      %float = OpTypeFloat 32
         %26 = OpExtInst %void %1 DebugTypeBasic %25 %uint_32 %uint_3 %uint_0
    %v4float = OpTypeVector %float 4
         %28 = OpExtInst %void %1 DebugTypeVector %26 %uint_4
       %SSBO = OpTypeStruct %v4float
    %uint_10 = OpConstant %uint 10
%_ptr_StorageBuffer_SSBO = OpTypePointer Uniform %SSBO
    %uint_12 = OpConstant %uint 12
          %x = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer
     %uint_8 = OpConstant %uint 8
    %float_0 = OpConstant %float 0
         %50 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
       %main = OpFunction %void None %5
         %15 = OpLabel
         %53 = OpAccessChain %_ptr_StorageBuffer_v4float %x %uint_0
               OpStore %53 %50
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), Not(HasSubstr("a.comp")));
}

TEST_F(ValidateShaderDebugInfo, DebugLocalVariable) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450

void main() {
    uint x = 0;
    int y = int(x);
}"
         %32 = OpString "x"
         %37 = OpString "int"
         %43 = OpString "y"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %20 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_3 %uint_0 %20 %16 %uint_3 %uint_3
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_7 = OpConstant %uint 7
         %29 = OpExtInst %void %1 DebugTypePointer %9 %uint_7 %uint_0
         %31 = OpExtInst %void %1 DebugLocalVariable %32 %9 %18 %uint_4 %uint_0 %17 %uint_4
         %34 = OpExtInst %void %1 DebugExpression
        %int = OpTypeInt 32 1
         %38 = OpExtInst %void %1 DebugTypeBasic %37 %uint_32 %uint_4 %uint_0
%_ptr_Function_int = OpTypePointer Function %int
         %40 = OpExtInst %void %1 DebugTypePointer %38 %uint_7 %uint_0
     %uint_5 = OpConstant %uint 5
         %42 = OpExtInst %void %1 DebugLocalVariable %43 %38 %18 %uint_5 %uint_0 %17 %uint_4
       %main = OpFunction %void None %5
         %15 = OpLabel
          %x = OpVariable %_ptr_Function_uint Uniform
          %y = OpVariable %_ptr_Function_int Function
         %25 = OpExtInst %void %1 DebugScope %17
         %26 = OpExtInst %void %1 DebugLine %18 %uint_3 %uint_3 %uint_0 %uint_0
         %24 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
         %35 = OpExtInst %void %1 DebugLine %18 %uint_4 %uint_4 %uint_0 %uint_0
         %33 = OpExtInst %void %1 DebugDeclare %31 %x %34
               OpStore %x %uint_0
         %46 = OpExtInst %void %1 DebugLine %18 %uint_5 %uint_5 %uint_0 %uint_0
         %45 = OpExtInst %void %1 DebugDeclare %42 %y %34
         %47 = OpLoad %uint %x
         %48 = OpBitcast %int %47
               OpStore %y %48
         %49 = OpExtInst %void %1 DebugLine %18 %uint_6 %uint_6 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(  --> a.comp:4:0
  |
4 |     uint x = 0;
  |)"));
}

// Test if no DebugLocalVariable is found, things still work
TEST_F(ValidateShaderDebugInfo, NoDebugLocalVariable) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450

void main() {
    uint x = 0;
    int y = int(x);
}"
         %32 = OpString "x"
         %37 = OpString "int"
         %43 = OpString "y"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %20 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_3 %uint_0 %20 %16 %uint_3 %uint_3
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_7 = OpConstant %uint 7
         %29 = OpExtInst %void %1 DebugTypePointer %9 %uint_7 %uint_0
         %31 = OpExtInst %void %1 DebugLocalVariable %32 %9 %18 %uint_4 %uint_0 %17 %uint_4
         %34 = OpExtInst %void %1 DebugExpression
        %int = OpTypeInt 32 1
         %38 = OpExtInst %void %1 DebugTypeBasic %37 %uint_32 %uint_4 %uint_0
%_ptr_Function_int = OpTypePointer Function %int
         %40 = OpExtInst %void %1 DebugTypePointer %38 %uint_7 %uint_0
     %uint_5 = OpConstant %uint 5
         %42 = OpExtInst %void %1 DebugLocalVariable %43 %38 %18 %uint_5 %uint_0 %17 %uint_4
       %main = OpFunction %void None %5
         %15 = OpLabel
          %x = OpVariable %_ptr_Function_uint Uniform
          %y = OpVariable %_ptr_Function_int Function
         %25 = OpExtInst %void %1 DebugScope %17
         %26 = OpExtInst %void %1 DebugLine %18 %uint_3 %uint_3 %uint_0 %uint_0
         %24 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
         %35 = OpExtInst %void %1 DebugLine %18 %uint_4 %uint_4 %uint_0 %uint_0
               OpStore %x %uint_0
         %46 = OpExtInst %void %1 DebugLine %18 %uint_5 %uint_5 %uint_0 %uint_0
         %47 = OpLoad %uint %x
         %48 = OpBitcast %int %47
               OpStore %y %48
         %49 = OpExtInst %void %1 DebugLine %18 %uint_6 %uint_6 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), Not(HasSubstr("a.comp")));
}

TEST_F(ValidateShaderDebugInfo, FunctionCall) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %24 = OpString "foo"
         %27 = OpString "#version 450

int foo(int z) {
    return z * 3;
}

void main() {
    uint x = 0;
    int y = foo(x);
}"
         %33 = OpString "z"
         %38 = OpString "main"
         %54 = OpString "x"
         %60 = OpString "y"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
     %int_3 = OpConstant %int 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_int = OpTypePointer Function %int
     %uint_7 = OpConstant %uint 7
         %18 = OpExtInst %void %1 DebugTypePointer %9 %uint_7 %uint_0
         %19 = OpTypeFunction %int %_ptr_Function_int
         %20 = OpExtInst %void %1 DebugTypeFunction %uint_3 %9 %9
         %26 = OpExtInst %void %1 DebugSource %2 %27
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %28 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %26 %uint_2
         %25 = OpExtInst %void %1 DebugFunction %24 %20 %26 %uint_3 %uint_0 %28 %24 %uint_3 %uint_3
         %32 = OpExtInst %void %1 DebugLocalVariable %33 %9 %26 %uint_3 %uint_0 %25 %uint_4 %uint_1
         %35 = OpExtInst %void %1 DebugExpression
         %39 = OpExtInst %void %1 DebugFunction %38 %6 %26 %uint_7 %uint_0 %28 %38 %uint_3 %uint_7
     %uint_5 = OpConstant %uint 5
     %uint_8 = OpConstant %uint 8
         %53 = OpExtInst %void %1 DebugLocalVariable %54 %9 %26 %uint_8 %uint_0 %39 %uint_4
     %uint_9 = OpConstant %uint 9
         %59 = OpExtInst %void %1 DebugLocalVariable %60 %9 %26 %uint_9 %uint_0 %39 %uint_4
    %uint_10 = OpConstant %uint 10
       %main = OpFunction %void None %5
         %15 = OpLabel
          %x = OpVariable %_ptr_Function_uint Function
          %y = OpVariable %_ptr_Function_uint Function
      %param = OpVariable %_ptr_Function_uint Function
         %50 = OpExtInst %void %1 DebugScope %39
         %51 = OpExtInst %void %1 DebugLine %26 %uint_7 %uint_7 %uint_0 %uint_0
         %49 = OpExtInst %void %1 DebugFunctionDefinition %39 %main
         %57 = OpExtInst %void %1 DebugLine %26 %uint_8 %uint_8 %uint_0 %uint_0
         %56 = OpExtInst %void %1 DebugDeclare %53 %x %35
               OpStore %x %uint_0
         %63 = OpExtInst %void %1 DebugLine %26 %uint_9 %uint_9 %uint_0 %uint_0
         %62 = OpExtInst %void %1 DebugDeclare %59 %y %35
         %65 = OpLoad %uint %x
               OpStore %param %65
         %66 = OpFunctionCall %int %foo_u1_ %param
               OpStore %y %66
               OpReturn
               OpFunctionEnd
    %foo_u1_ = OpFunction %int None %19
          %z = OpFunctionParameter %_ptr_Function_int
         %23 = OpLabel
         %36 = OpExtInst %void %1 DebugScope %25
         %37 = OpExtInst %void %1 DebugLine %26 %uint_3 %uint_3 %uint_0 %uint_0
         %34 = OpExtInst %void %1 DebugDeclare %32 %z %35
         %40 = OpExtInst %void %1 DebugFunctionDefinition %25 %foo_u1_
         %42 = OpExtInst %void %1 DebugLine %26 %uint_4 %uint_4 %uint_0 %uint_0
         %41 = OpLoad %int %z
         %43 = OpIMul %int %41 %int_3
               OpReturnValue %43
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(  --> a.comp:9:0
  |
9 |     int y = foo(x);
  |
  --> a.comp:3:0
  |
3 | int foo(int z) {
  |)"));
}

// If we can only find the caller, not the callee
TEST_F(ValidateShaderDebugInfo, FunctionCallNoCallee) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %24 = OpString "foo"
         %27 = OpString "#version 450

int foo(int z) {
    return z * 3;
}

void main() {
    uint x = 0;
    int y = foo(x);
}"
         %33 = OpString "z"
         %38 = OpString "main"
         %54 = OpString "x"
         %60 = OpString "y"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
     %int_3 = OpConstant %int 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_int = OpTypePointer Function %int
     %uint_7 = OpConstant %uint 7
         %18 = OpExtInst %void %1 DebugTypePointer %9 %uint_7 %uint_0
         %19 = OpTypeFunction %int %_ptr_Function_int
         %20 = OpExtInst %void %1 DebugTypeFunction %uint_3 %9 %9
         %26 = OpExtInst %void %1 DebugSource %2 %27
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %28 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %26 %uint_2
         %25 = OpExtInst %void %1 DebugFunction %24 %20 %26 %uint_3 %uint_0 %28 %24 %uint_3 %uint_3
         %32 = OpExtInst %void %1 DebugLocalVariable %33 %9 %26 %uint_3 %uint_0 %25 %uint_4 %uint_1
         %35 = OpExtInst %void %1 DebugExpression
         %39 = OpExtInst %void %1 DebugFunction %38 %6 %26 %uint_7 %uint_0 %28 %38 %uint_3 %uint_7
     %uint_5 = OpConstant %uint 5
     %uint_8 = OpConstant %uint 8
         %53 = OpExtInst %void %1 DebugLocalVariable %54 %9 %26 %uint_8 %uint_0 %39 %uint_4
     %uint_9 = OpConstant %uint 9
         %59 = OpExtInst %void %1 DebugLocalVariable %60 %9 %26 %uint_9 %uint_0 %39 %uint_4
    %uint_10 = OpConstant %uint 10
       %main = OpFunction %void None %5
         %15 = OpLabel
          %x = OpVariable %_ptr_Function_uint Function
          %y = OpVariable %_ptr_Function_uint Function
      %param = OpVariable %_ptr_Function_uint Function
         %50 = OpExtInst %void %1 DebugScope %39
         %51 = OpExtInst %void %1 DebugLine %26 %uint_7 %uint_7 %uint_0 %uint_0
         %49 = OpExtInst %void %1 DebugFunctionDefinition %39 %main
         %57 = OpExtInst %void %1 DebugLine %26 %uint_8 %uint_8 %uint_0 %uint_0
         %56 = OpExtInst %void %1 DebugDeclare %53 %x %35
               OpStore %x %uint_0
         %63 = OpExtInst %void %1 DebugLine %26 %uint_9 %uint_9 %uint_0 %uint_0
         %62 = OpExtInst %void %1 DebugDeclare %59 %y %35
         %65 = OpLoad %uint %x
               OpStore %param %65
         %66 = OpFunctionCall %int %foo_u1_ %param
               OpStore %y %66
               OpReturn
               OpFunctionEnd
    %foo_u1_ = OpFunction %int None %19
          %z = OpFunctionParameter %_ptr_Function_int
         %23 = OpLabel
         %41 = OpLoad %int %z
         %43 = OpIMul %int %41 %int_3
               OpReturnValue %43
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(  --> a.comp:9:0
  |
9 |     int y = foo(x);
  |)"));
}

TEST_F(ValidateShaderDebugInfo, FunctionCallAcrossFile) {
  const std::string str = R"(

               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
%file_name_0 = OpString "file_a.comp"
%file_name_1 = OpString "file_b.comp"
          %8 = OpString "uint"
         %24 = OpString "foo"
      %file_0 = OpString "
int foo(int z) {
    return z * 3;
}
"
    %file_1 = OpString "#version 450
void main() {
    uint x = 0;
    int y = foo(
                 x
               );
}"
         %33 = OpString "z"
         %38 = OpString "main"
         %54 = OpString "x"
         %60 = OpString "y"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_32 = OpConstant %uint 32
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %uint_4 = OpConstant %uint 4
     %uint_6 = OpConstant %uint 6
     %uint_7 = OpConstant %uint 7
     %int_3 = OpConstant %int 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_int = OpTypePointer Function %int
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
         %18 = OpExtInst %void %1 DebugTypePointer %9 %uint_7 %uint_0
         %19 = OpTypeFunction %int %_ptr_Function_int
         %20 = OpExtInst %void %1 DebugTypeFunction %uint_3 %9 %9
  %source_0 = OpExtInst %void %1 DebugSource %file_name_0 %file_0
  %source_1 = OpExtInst %void %1 DebugSource %file_name_1 %file_1
         %28 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %source_0 %uint_2
         %25 = OpExtInst %void %1 DebugFunction %24 %20 %source_0 %uint_2 %uint_0 %28 %24 %uint_3 %uint_3
         %39 = OpExtInst %void %1 DebugFunction %38 %6 %source_1 %uint_2 %uint_0 %28 %38 %uint_3 %uint_7
       %main = OpFunction %void None %5
         %15 = OpLabel
          %x = OpVariable %_ptr_Function_uint Function
          %y = OpVariable %_ptr_Function_uint Function
      %param = OpVariable %_ptr_Function_uint Function
         %51 = OpExtInst %void %1 DebugLine %source_1 %uint_2 %uint_2 %uint_0 %uint_0
         %49 = OpExtInst %void %1 DebugFunctionDefinition %39 %main
         %57 = OpExtInst %void %1 DebugLine %source_1 %uint_3 %uint_3 %uint_0 %uint_0
               OpStore %x %uint_0
         %63 = OpExtInst %void %1 DebugLine %source_1 %uint_4 %uint_6 %uint_0 %uint_0
         %65 = OpLoad %uint %x
               OpStore %param %65
         %66 = OpFunctionCall %int %foo_u1_ %param
               OpStore %y %66
               OpReturn
               OpFunctionEnd
    %foo_u1_ = OpFunction %int None %19
          %z = OpFunctionParameter %_ptr_Function_int
         %23 = OpLabel
         %37 = OpExtInst %void %1 DebugLine %source_0 %uint_2 %uint_2 %uint_0 %uint_0
         %40 = OpExtInst %void %1 DebugFunctionDefinition %25 %foo_u1_
         %42 = OpExtInst %void %1 DebugLine %source_0 %uint_3 %uint_3 %uint_0 %uint_0
         %41 = OpLoad %int %z
         %43 = OpIMul %int %41 %int_3
               OpReturnValue %43
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(  --> file_b.comp:4:0
  |
4 |     int y = foo(
5 |                  x
6 |                );
  |
  --> file_a.comp:2:0
  |
2 | int foo(int z) {
  |)"));
}

TEST_F(ValidateShaderDebugInfo, ExecutionMode) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 0 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450

void main() {
}"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %20 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_3 %uint_0 %20 %16 %uint_3 %uint_3
       %main = OpFunction %void None %5
         %15 = OpLabel
         %25 = OpExtInst %void %1 DebugScope %17
         %26 = OpExtInst %void %1 DebugLine %18 %uint_3 %uint_3 %uint_0 %uint_0
         %24 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(--> a.comp:3:0
  |
3 | void main() {
  |)"));
}

TEST_F(ValidateShaderDebugInfo, EntryPoint) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450

void main() {
}"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %20 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_3 %uint_0 %20 %16 %uint_3 %uint_3
       %main = OpFunction %uint None %5
         %15 = OpLabel
         %25 = OpExtInst %void %1 DebugScope %17
         %26 = OpExtInst %void %1 DebugLine %18 %uint_3 %uint_3 %uint_0 %uint_0
         %24 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
               OpReturnValue %uint_1
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(--> a.comp:3:0
  |
3 | void main() {
  |)"));
}

TEST_F(ValidateShaderDebugInfo, ReturnValue) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %20 = OpString "foo"
         %23 = OpString "#version 450

uint foo() {
   return 4;
}

void main() {
   int a = 0;
   uint b = foo();
}"
         %28 = OpString "main"
         %44 = OpString "int"
         %50 = OpString "a"
         %60 = OpString "b"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %16 = OpTypeFunction %uint
         %17 = OpExtInst %void %1 DebugTypeFunction %uint_3 %9
         %22 = OpExtInst %void %1 DebugSource %2 %23
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %int_4 = OpConstant %int 4
     %uint_2 = OpConstant %uint 2
         %24 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %22 %uint_2
         %21 = OpExtInst %void %1 DebugFunction %20 %17 %22 %uint_3 %uint_0 %24 %20 %uint_3 %uint_3
     %uint_7 = OpConstant %uint 7
         %29 = OpExtInst %void %1 DebugFunction %28 %6 %22 %uint_7 %uint_0 %24 %28 %uint_3 %uint_7
     %uint_5 = OpConstant %uint 5
         %45 = OpExtInst %void %1 DebugTypeBasic %44 %uint_32 %uint_4 %uint_0
%_ptr_Function_int = OpTypePointer Function %int
         %47 = OpExtInst %void %1 DebugTypePointer %45 %uint_7 %uint_0
     %uint_8 = OpConstant %uint 8
         %49 = OpExtInst %void %1 DebugLocalVariable %50 %45 %22 %uint_8 %uint_0 %29 %uint_4
         %53 = OpExtInst %void %1 DebugExpression
      %int_0 = OpConstant %int 0
%_ptr_Function_uint = OpTypePointer Function %uint
         %57 = OpExtInst %void %1 DebugTypePointer %9 %uint_7 %uint_0
     %uint_9 = OpConstant %uint 9
         %59 = OpExtInst %void %1 DebugLocalVariable %60 %9 %22 %uint_9 %uint_0 %29 %uint_4
    %uint_10 = OpConstant %uint 10
       %main = OpFunction %void None %5
         %15 = OpLabel
          %a = OpVariable %_ptr_Function_int Function
          %b = OpVariable %_ptr_Function_uint Function
         %41 = OpExtInst %void %1 DebugScope %29
         %42 = OpExtInst %void %1 DebugLine %22 %uint_7 %uint_7 %uint_0 %uint_0
         %40 = OpExtInst %void %1 DebugFunctionDefinition %29 %main
         %54 = OpExtInst %void %1 DebugLine %22 %uint_8 %uint_8 %uint_0 %uint_0
         %52 = OpExtInst %void %1 DebugDeclare %49 %a %53
               OpStore %a %int_0
         %63 = OpExtInst %void %1 DebugLine %22 %uint_9 %uint_9 %uint_0 %uint_0
         %62 = OpExtInst %void %1 DebugDeclare %59 %b %53
         %64 = OpFunctionCall %uint %foo_
               OpStore %b %64
         %65 = OpExtInst %void %1 DebugLine %22 %uint_10 %uint_10 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
       %foo_ = OpFunction %uint None %16
         %19 = OpLabel
         %32 = OpExtInst %void %1 DebugScope %21
         %33 = OpExtInst %void %1 DebugLine %22 %uint_3 %uint_3 %uint_0 %uint_0
         %31 = OpExtInst %void %1 DebugFunctionDefinition %21 %foo_
         %34 = OpExtInst %void %1 DebugLine %22 %uint_4 %uint_4 %uint_0 %uint_0
               OpReturnValue %int_4
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(--> a.comp:4:0
  |
4 |    return 4;
  |
  --> a.comp:3:0
  |
3 | uint foo() {
  |)"));
}

TEST_F(ValidateShaderDebugInfo, ReturnValueVoid) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %20 = OpString "foo"
         %23 = OpString "#version 450

uint foo() {
   return 4;
}

void main() {
   int a = 0;
   uint b = foo();
}"
         %28 = OpString "main"
         %44 = OpString "int"
         %50 = OpString "a"
         %60 = OpString "b"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %16 = OpTypeFunction %uint
         %17 = OpExtInst %void %1 DebugTypeFunction %uint_3 %9
         %22 = OpExtInst %void %1 DebugSource %2 %23
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %int_4 = OpConstant %int 4
     %uint_2 = OpConstant %uint 2
         %24 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %22 %uint_2
         %21 = OpExtInst %void %1 DebugFunction %20 %17 %22 %uint_3 %uint_0 %24 %20 %uint_3 %uint_3
     %uint_7 = OpConstant %uint 7
         %29 = OpExtInst %void %1 DebugFunction %28 %6 %22 %uint_7 %uint_0 %24 %28 %uint_3 %uint_7
     %uint_5 = OpConstant %uint 5
         %45 = OpExtInst %void %1 DebugTypeBasic %44 %uint_32 %uint_4 %uint_0
%_ptr_Function_int = OpTypePointer Function %int
         %47 = OpExtInst %void %1 DebugTypePointer %45 %uint_7 %uint_0
     %uint_8 = OpConstant %uint 8
         %49 = OpExtInst %void %1 DebugLocalVariable %50 %45 %22 %uint_8 %uint_0 %29 %uint_4
         %53 = OpExtInst %void %1 DebugExpression
      %int_0 = OpConstant %int 0
%_ptr_Function_uint = OpTypePointer Function %uint
         %57 = OpExtInst %void %1 DebugTypePointer %9 %uint_7 %uint_0
     %uint_9 = OpConstant %uint 9
         %59 = OpExtInst %void %1 DebugLocalVariable %60 %9 %22 %uint_9 %uint_0 %29 %uint_4
    %uint_10 = OpConstant %uint 10
       %main = OpFunction %void None %5
         %15 = OpLabel
          %a = OpVariable %_ptr_Function_int Function
          %b = OpVariable %_ptr_Function_uint Function
         %41 = OpExtInst %void %1 DebugScope %29
         %42 = OpExtInst %void %1 DebugLine %22 %uint_7 %uint_7 %uint_0 %uint_0
         %40 = OpExtInst %void %1 DebugFunctionDefinition %29 %main
         %54 = OpExtInst %void %1 DebugLine %22 %uint_8 %uint_8 %uint_0 %uint_0
         %52 = OpExtInst %void %1 DebugDeclare %49 %a %53
               OpStore %a %int_0
         %63 = OpExtInst %void %1 DebugLine %22 %uint_9 %uint_9 %uint_0 %uint_0
         %62 = OpExtInst %void %1 DebugDeclare %59 %b %53
         %64 = OpFunctionCall %uint %foo_
               OpStore %b %64
         %65 = OpExtInst %void %1 DebugLine %22 %uint_10 %uint_10 %uint_0 %uint_0
               OpReturnValue %int_0
               OpFunctionEnd
       %foo_ = OpFunction %uint None %16
         %19 = OpLabel
         %32 = OpExtInst %void %1 DebugScope %21
         %33 = OpExtInst %void %1 DebugLine %22 %uint_3 %uint_3 %uint_0 %uint_0
         %31 = OpExtInst %void %1 DebugFunctionDefinition %21 %foo_
         %34 = OpExtInst %void %1 DebugLine %22 %uint_4 %uint_4 %uint_0 %uint_0
               OpReturnValue %int_4
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(--> a.comp:10:0
   |
10 | }
   |
  --> a.comp:7:0
  |
7 | void main() {
  |)"));
}

TEST_F(ValidateShaderDebugInfo, FunctionParameter) {
  const std::string str = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %24 = OpString "foo"
         %27 = OpString "#version 450

uint foo(uint a) {
   return a * 4;
}

void main() {
   int a = 0;
   uint b = foo(a);
}"
         %33 = OpString "a"
         %38 = OpString "main"
         %53 = OpString "int"
         %65 = OpString "b"
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_7 = OpConstant %uint 7
         %18 = OpExtInst %void %1 DebugTypePointer %9 %uint_7 %uint_0
         %19 = OpTypeFunction %uint %_ptr_Function_uint
         %20 = OpExtInst %void %1 DebugTypeFunction %uint_3 %9 %9
         %26 = OpExtInst %void %1 DebugSource %2 %27
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %28 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %26 %uint_2
         %25 = OpExtInst %void %1 DebugFunction %24 %20 %26 %uint_3 %uint_0 %28 %24 %uint_3 %uint_3
         %32 = OpExtInst %void %1 DebugLocalVariable %33 %9 %26 %uint_3 %uint_0 %25 %uint_4 %uint_1
         %35 = OpExtInst %void %1 DebugExpression
         %39 = OpExtInst %void %1 DebugFunction %38 %6 %26 %uint_7 %uint_0 %28 %38 %uint_3 %uint_7
     %uint_5 = OpConstant %uint 5
        %int = OpTypeInt 32 1
         %54 = OpExtInst %void %1 DebugTypeBasic %53 %uint_32 %uint_4 %uint_0
%_ptr_Function_int = OpTypePointer Function %int
         %56 = OpExtInst %void %1 DebugTypePointer %54 %uint_7 %uint_0
     %uint_8 = OpConstant %uint 8
         %58 = OpExtInst %void %1 DebugLocalVariable %33 %54 %26 %uint_8 %uint_0 %39 %uint_4
      %int_0 = OpConstant %int 0
     %uint_9 = OpConstant %uint 9
         %64 = OpExtInst %void %1 DebugLocalVariable %65 %9 %26 %uint_9 %uint_0 %39 %uint_4
    %uint_10 = OpConstant %uint 10
       %main = OpFunction %void None %5
         %15 = OpLabel
        %a_0 = OpVariable %_ptr_Function_int Function
          %b = OpVariable %_ptr_Function_uint Function
      %param = OpVariable %_ptr_Function_uint Function
         %50 = OpExtInst %void %1 DebugScope %39
         %51 = OpExtInst %void %1 DebugLine %26 %uint_7 %uint_7 %uint_0 %uint_0
         %49 = OpExtInst %void %1 DebugFunctionDefinition %39 %main
         %61 = OpExtInst %void %1 DebugLine %26 %uint_8 %uint_8 %uint_0 %uint_0
         %60 = OpExtInst %void %1 DebugDeclare %58 %a_0 %35
               OpStore %a_0 %int_0
         %68 = OpExtInst %void %1 DebugLine %26 %uint_9 %uint_9 %uint_0 %uint_0
         %67 = OpExtInst %void %1 DebugDeclare %64 %b %35
         %69 = OpLoad %int %a_0
         %70 = OpBitcast %uint %69
               OpStore %param %70
         %72 = OpFunctionCall %uint %foo_u1_ %param
               OpStore %b %72
         %73 = OpExtInst %void %1 DebugLine %26 %uint_10 %uint_10 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
    %foo_u1_ = OpFunction %uint None %19
          %a = OpFunctionParameter %_ptr_Function_int
         %23 = OpLabel
         %36 = OpExtInst %void %1 DebugScope %25
         %37 = OpExtInst %void %1 DebugLine %26 %uint_3 %uint_3 %uint_0 %uint_0
         %34 = OpExtInst %void %1 DebugDeclare %32 %a %35
         %40 = OpExtInst %void %1 DebugFunctionDefinition %25 %foo_u1_
         %42 = OpExtInst %void %1 DebugLine %26 %uint_4 %uint_4 %uint_0 %uint_0
         %41 = OpLoad %uint %a
         %43 = OpIMul %uint %41 %uint_4
               OpReturnValue %43
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str(), SPV_ENV_VULKAN_1_3);
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr(R"(--> a.comp:3:0
  |
3 | uint foo(uint a) {
  |)"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
