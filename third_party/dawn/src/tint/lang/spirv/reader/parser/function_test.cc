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
    store %1, 42.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, DISABLED_VertexShader) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @vertex func():void {
  $B1: {
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
       %call = OpFunctionCall %bool %foo
               OpReturnValue %call
               OpFunctionEnd

       %main = OpFunction %void None %main_type
 %main_start = OpLabel
          %1 = OpFunctionCall %bool %bar
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
    %3:bool = call %2
    ret %3
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %5:bool = call %1
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
