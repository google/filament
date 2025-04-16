// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/spirv/reader/parser/helper_test.h"

namespace tint::spirv::reader {
namespace {

TEST_F(SpirvParserTest, Name_Set) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %1 "my_name"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
    %ep_type = OpTypeFunction %void
        %one = OpConstant %i32 1

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %my_name:i32 = let 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Name_IgnoreEmptyName) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %1 ""
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
    %ep_type = OpTypeFunction %void
        %one = OpConstant %i32 1

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Name_Duplicate) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %1 "vanilla"
               OpName %2 "vanilla"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
    %ep_type = OpTypeFunction %void
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
          %2 = OpCopyObject %i32 %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %vanilla:i32 = let 1i
    %vanilla_1:i32 = let 2i  # %vanilla_1: 'vanilla'
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Name_MemberName) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "Desert"
               OpMemberName %S 0 "strawberry"
               OpMemberName %S 1 "vanilla"
               OpMemberName %S 2 "chocolate"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
          %S = OpTypeStruct %i32 %i32 %i32
        %one = OpConstant %i32 1
          %d = OpConstantComposite %S %one %one %one
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %2 = OpCopyObject %S %d
               OpReturn
               OpFunctionEnd
)",
              R"(
Desert = struct @align(4) {
  strawberry:i32 @offset(0)
  vanilla:i32 @offset(4)
  chocolate:i32 @offset(8)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:Desert = let Desert(1i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Name_DuplicateStructNames) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "Desert"
               OpName %D "Desert"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %f32 = OpTypeFloat 32
          %S = OpTypeStruct %i32 %i32 %i32
          %D = OpTypeStruct %i32 %i32 %f32
        %one = OpConstant %i32 1
        %two = OpConstant %f32 2
          %s = OpConstantComposite %S %one %one %one
          %d = OpConstantComposite %D %one %one %two
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %2 = OpCopyObject %S %s
          %3 = OpCopyObject %D %d
               OpReturn
               OpFunctionEnd
)",
              R"(
Desert = struct @align(4) {
  tint_symbol:i32 @offset(0)
  tint_symbol_1:i32 @offset(4)
  tint_symbol_2:i32 @offset(8)
}

Desert_1 = struct @align(4) {
  tint_symbol_3:i32 @offset(0)
  tint_symbol_4:i32 @offset(4)
  tint_symbol_5:f32 @offset(8)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:Desert = let Desert(1i)
    %3:Desert_1 = let Desert_1(1i, 1i, 2.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Name_IgnoreEmptyMemberName) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "Desert"
               OpMemberName %S 0 "strawberry"
               OpMemberName %S 1 ""
               OpMemberName %S 2 "chocolate"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
          %S = OpTypeStruct %i32 %i32 %i32
        %one = OpConstant %i32 1
          %d = OpConstantComposite %S %one %one %one
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %2 = OpCopyObject %S %d
               OpReturn
               OpFunctionEnd
)",
              R"(
Desert = struct @align(4) {
  strawberry:i32 @offset(0)
  tint_symbol:i32 @offset(4)
  chocolate:i32 @offset(8)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:Desert = let Desert(1i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Name_StructSkipFirstMember) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "Desert"
               OpMemberName %S 1 "strawberry"
               OpMemberName %S 2 "chocolate"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
          %S = OpTypeStruct %i32 %i32 %i32
        %one = OpConstant %i32 1
          %d = OpConstantComposite %S %one %one %one
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %2 = OpCopyObject %S %d
               OpReturn
               OpFunctionEnd
)",
              R"(
Desert = struct @align(4) {
  tint_symbol:i32 @offset(0)
  strawberry:i32 @offset(4)
  chocolate:i32 @offset(8)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:Desert = let Desert(1i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Name_StructSkipMember) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "Desert"
               OpMemberName %S 0 "strawberry"
               OpMemberName %S 2 "chocolate"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
          %S = OpTypeStruct %i32 %i32 %i32
        %one = OpConstant %i32 1
          %d = OpConstantComposite %S %one %one %one
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %2 = OpCopyObject %S %d
               OpReturn
               OpFunctionEnd
)",
              R"(
Desert = struct @align(4) {
  strawberry:i32 @offset(0)
  tint_symbol:i32 @offset(4)
  chocolate:i32 @offset(8)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:Desert = let Desert(1i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Name_StructSkipLast) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "Desert"
               OpMemberName %S 0 "strawberry"
               OpMemberName %S 1 "chocolate"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
          %S = OpTypeStruct %i32 %i32 %i32
        %one = OpConstant %i32 1
          %d = OpConstantComposite %S %one %one %one
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %2 = OpCopyObject %S %d
               OpReturn
               OpFunctionEnd
)",
              R"(
Desert = struct @align(4) {
  strawberry:i32 @offset(0)
  chocolate:i32 @offset(4)
  tint_symbol:i32 @offset(8)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:Desert = let Desert(1i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Name_FunctionName) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %foo "my_function"
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void

        %foo = OpFunction %void None %ep_type
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%my_function = func():void {
  $B1: {
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Name_FunctionParamName) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %foo "my_function"
               OpName %bar "param1"
               OpName %baz "terrapin"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %i32 %i32 %i32

        %foo = OpFunction %void None %fn_type
        %bar = OpFunctionParameter %i32
        %baz = OpFunctionParameter %i32
          %1 = OpFunctionParameter %i32
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%my_function = func(%param1:i32, %terrapin:i32, %4:i32):void {
  $B1: {
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
