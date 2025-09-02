// Copyright 2023 The Dawn & Tint Authors
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

TEST_F(SpirvParserTest, ComputeShader) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, LocalSize) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 3 4 5
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(3u, 4u, 5u) func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, WorkgroupSize_Constant) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_3 = OpConstant %uint 3
     %uint_5 = OpConstant %uint 5
     %uint_7 = OpConstant %uint 7
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_3 %uint_5 %uint_7
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(3u, 5u, 7u) func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, WorkgroupSize_SpecConstant_Mixed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_3 = OpSpecConstant %uint 3
     %uint_5 = OpConstant %uint 5
     %uint_7 = OpSpecConstant %uint 7
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %uint_3 %uint_5 %uint_7
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = override 3u
  %2:u32 = override 7u
}

%main = @compute @workgroup_size(%1, 5u, %2) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, WorkgroupSize_SpecConstant) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_3 = OpSpecConstant %uint 3
     %uint_5 = OpSpecConstant %uint 5
     %uint_7 = OpSpecConstant %uint 7
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %uint_3 %uint_5 %uint_7
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = override 3u
  %2:u32 = override 5u
  %3:u32 = override 7u
}

%main = @compute @workgroup_size(%1, %2, %3) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FragmentShader) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FragmentShader_DepthReplacing) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %depth
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main DepthReplacing
               OpDecorate %depth BuiltIn FragDepth
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
     %f32_42 = OpConstant %f32 42.0
%_ptr_Output_f32 = OpTypePointer Output %f32
      %depth = OpVariable %_ptr_Output_f32 Output
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpStore %depth %f32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<__out, f32, read_write> = var undef @builtin(frag_depth)
}

%main = @fragment func():void {
  $B2: {
    undef = phony %1
    store %1, 42.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VertexShader_PositionUnused_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %1
               OpDecorate %str Block
               OpMemberDecorate %str 0 BuiltIn Position
       %void = OpTypeVoid
      %float = OpTypeFloat 32
    %ep_type = OpTypeFunction %void
    %v4float = OpTypeVector %float 4
        %str = OpTypeStruct %v4float
    %str_ptr = OpTypePointer Output %str
          %1 = OpVariable %str_ptr Output
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %1:ptr<__out, tint_symbol_1, read_write> = var undef
}

%main = @vertex func():void {
  $B2: {
    undef = phony %1
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VertexShader_PositionUsed_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %1
               OpDecorate %str Block
               OpMemberDecorate %str 0 BuiltIn Position
       %void = OpTypeVoid
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
    %ep_type = OpTypeFunction %void
    %v4float = OpTypeVector %float 4
        %str = OpTypeStruct %v4float
    %str_ptr = OpTypePointer Output %str
%ptr_v4float = OpTypePointer Output %v4float
     %uint_0 = OpConstant %uint 0
        %one = OpConstant %float 1
        %two = OpConstant %float 2
      %three = OpConstant %float 3
       %four = OpConstant %float 4
          %3 = OpConstantComposite %v4float %one %two %three %four
          %1 = OpVariable %str_ptr Output
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %2 = OpAccessChain %ptr_v4float %1 %uint_0
               OpStore %2 %3
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %1:ptr<__out, tint_symbol_1, read_write> = var undef
}

%main = @vertex func():void {
  $B2: {
    undef = phony %1
    %3:ptr<__out, vec4<f32>, read_write> = access %1, 0u
    store %3, vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VertexShader_PositionUnused) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %position
               OpName %position "position"
               OpDecorate %position BuiltIn Position
       %void = OpTypeVoid
      %float = OpTypeFloat 32
    %ep_type = OpTypeFunction %void
    %v4float = OpTypeVector %float 4
        %ptr = OpTypePointer Output %v4float
   %position = OpVariable %ptr Output
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
}

%main = @vertex func():void {
  $B2: {
    undef = phony %position
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VertexShader_PositionUsed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %position
               OpName %position "position"
               OpDecorate %position BuiltIn Position
       %void = OpTypeVoid
      %float = OpTypeFloat 32
    %ep_type = OpTypeFunction %void
    %v4float = OpTypeVector %float 4
        %ptr = OpTypePointer Output %v4float
         %f1 = OpConstant %float 1
         %f2 = OpConstant %float 2
         %f3 = OpConstant %float 3
         %f4 = OpConstant %float 4
         %v4 = OpConstantComposite %v4float %f1 %f2 %f3 %f4
   %position = OpVariable %ptr Output
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpStore %position %v4
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
}

%main = @vertex func():void {
  $B2: {
    undef = phony %position
    store %position, vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VertexShader_PositionUsed_Transitive) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %position
               OpName %position "position"
               OpDecorate %position BuiltIn Position
       %void = OpTypeVoid
      %float = OpTypeFloat 32
    %ep_type = OpTypeFunction %void
    %v4float = OpTypeVector %float 4
        %ptr = OpTypePointer Output %v4float
         %f1 = OpConstant %float 1
         %f2 = OpConstant %float 2
         %f3 = OpConstant %float 3
         %f4 = OpConstant %float 4
         %v4 = OpConstantComposite %v4float %f1 %f2 %f3 %f4
   %position = OpVariable %ptr Output
          %c = OpFunction %void None %ep_type
         %c1 = OpLabel
               OpStore %position %v4
               OpReturn
               OpFunctionEnd
          %b = OpFunction %void None %ep_type
         %b1 = OpLabel
         %b2 = OpFunctionCall %void %c
               OpReturn
               OpFunctionEnd
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
         %a1 = OpFunctionCall %void %b
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %position:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)
}

%2 = func():void {
  $B2: {
    store %position, vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
    ret
  }
}
%3 = func():void {
  $B3: {
    %4:void = call %2
    ret
  }
}
%main = @vertex func():void {
  $B4: {
    undef = phony %position
    %6:void = call %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, MultipleEntryPoints) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %foo "foo"
               OpEntryPoint GLCompute %bar "bar"
               OpExecutionMode %foo LocalSize 3 4 5
               OpExecutionMode %bar LocalSize 6 7 8
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void

        %foo = OpFunction %void None %ep_type
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

        %bar = OpFunction %void None %ep_type
  %bar_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%foo = @compute @workgroup_size(3u, 4u, 5u) func():void {
  $B1: {
    ret
  }
}
%bar = @compute @workgroup_size(6u, 7u, 8u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionCall) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
  %func_type = OpTypeFunction %void

        %foo = OpFunction %void None %func_type
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %func_type
 %main_start = OpLabel
          %1 = OpFunctionCall %void %foo
               OpReturn
               OpFunctionEnd
)",
              R"(
%1 = func():void {
  $B1: {
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:void = call %1
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionCall_ForwardReference) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
  %func_type = OpTypeFunction %void

       %main = OpFunction %void None %func_type
 %main_start = OpLabel
          %1 = OpFunctionCall %void %foo
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %void None %func_type
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = call %3
    ret
  }
}
%3 = func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionCall_WithParam) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
   %foo_type = OpTypeFunction %void %bool
  %main_type = OpTypeFunction %void

        %foo = OpFunction %void None %foo_type
      %param = OpFunctionParameter %bool
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %main_type
 %main_start = OpLabel
          %1 = OpFunctionCall %void %foo %true
          %2 = OpFunctionCall %void %foo %false
               OpReturn
               OpFunctionEnd
)",
              R"(
%1 = func(%2:bool):void {
  $B1: {
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:void = call %1, true
    %5:void = call %1, false
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionCall_Chained_WithParam) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
   %foo_type = OpTypeFunction %void %bool
  %main_type = OpTypeFunction %void

        %bar = OpFunction %void None %foo_type
  %bar_param = OpFunctionParameter %bool
  %bar_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %void None %foo_type
  %foo_param = OpFunctionParameter %bool
  %foo_start = OpLabel
          %3 = OpFunctionCall %void %bar %foo_param
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %main_type
 %main_start = OpLabel
          %1 = OpFunctionCall %void %foo %true
          %2 = OpFunctionCall %void %foo %false
               OpReturn
               OpFunctionEnd
)",
              R"(
%1 = func(%2:bool):void {
  $B1: {
    ret
  }
}
%3 = func(%4:bool):void {
  $B2: {
    %5:void = call %1, %4
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:void = call %3, true
    %8:void = call %3, false
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionCall_WithMultipleParams) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
   %foo_type = OpTypeFunction %void %bool %bool
  %main_type = OpTypeFunction %void

        %foo = OpFunction %void None %foo_type
    %param_1 = OpFunctionParameter %bool
    %param_2 = OpFunctionParameter %bool
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %main_type
 %main_start = OpLabel
          %1 = OpFunctionCall %void %foo %true %false
               OpReturn
               OpFunctionEnd
)",
              R"(
%1 = func(%2:bool, %3:bool):void {
  $B1: {
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:void = call %1, true, false
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionCall_ReturnValue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
   %foo_type = OpTypeFunction %bool
  %main_type = OpTypeFunction %void

        %foo = OpFunction %bool None %foo_type
  %foo_start = OpLabel
               OpReturnValue %true
               OpFunctionEnd

       %main = OpFunction %void None %main_type
 %main_start = OpLabel
          %1 = OpFunctionCall %bool %foo
               OpReturn
               OpFunctionEnd
)",
              R"(
%1 = func():bool {
  $B1: {
    ret true
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = call %1
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionCall_ReturnValueChain) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
    %fn_type = OpTypeFunction %bool
  %main_type = OpTypeFunction %void

        %bar = OpFunction %bool None %fn_type
  %bar_start = OpLabel
               OpReturnValue %true
               OpFunctionEnd

        %foo = OpFunction %bool None %fn_type
  %foo_start = OpLabel
       %call = OpFunctionCall %bool %bar
               OpReturnValue %call
               OpFunctionEnd

       %main = OpFunction %void None %main_type
 %main_start = OpLabel
          %1 = OpFunctionCall %bool %foo
               OpReturn
               OpFunctionEnd
)",
              R"(
%1 = func():bool {
  $B1: {
    ret true
  }
}
%2 = func():bool {
  $B2: {
    %3:bool = call %1
    ret %3
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %5:bool = call %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionCall_ParamAndReturnValue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
   %foo_type = OpTypeFunction %bool %bool
  %main_type = OpTypeFunction %void

        %foo = OpFunction %bool None %foo_type
      %param = OpFunctionParameter %bool
  %foo_start = OpLabel
               OpReturnValue %param
               OpFunctionEnd

       %main = OpFunction %void None %main_type
 %main_start = OpLabel
          %1 = OpFunctionCall %bool %foo %true
               OpReturn
               OpFunctionEnd
)",
              R"(
%1 = func(%2:bool):bool {
  $B1: {
    ret %2
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = call %1, true
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
