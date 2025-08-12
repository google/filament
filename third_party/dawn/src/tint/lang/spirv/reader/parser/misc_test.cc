// Copyright 2024 The Dawn & Tint Authors
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

TEST_F(SpirvParserTest, Misc_OpNop) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpNop
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
)");
}

TEST_F(SpirvParserTest, Misc_OpUndefInFunction) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_2 = OpConstant %uint 2
%_arr_uint_uint_2 = OpTypeArray %uint %uint_2
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
 %_struct_13 = OpTypeStruct %bool %uint %int %float
    %ep_type = OpTypeFunction %void
         %11 = OpTypeFunction %int
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpUndef %bool
          %2 = OpUndef %uint
          %3 = OpUndef %int
          %4 = OpUndef %float
          %5 = OpUndef %_arr_uint_uint_2
          %6 = OpUndef %mat2v2float
          %7 = OpUndef %v2uint
          %8 = OpUndef %v2int
          %9 = OpUndef %v2float
         %10 = OpUndef %_struct_13
               OpReturn
               OpFunctionEnd
          %m = OpFunction %int None %11
         %12 = OpLabel
         %13 = OpUndef %int
               OpReturnValue %13
               OpFunctionEnd

)",
              R"(
tint_symbol_4 = struct @align(4) {
  tint_symbol:bool @offset(0)
  tint_symbol_1:u32 @offset(4)
  tint_symbol_2:i32 @offset(8)
  tint_symbol_3:f32 @offset(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
%2 = func():i32 {
  $B2: {
    ret 0i
  }
}
)");
}

TEST_F(SpirvParserTest, Misc_OpUndefBeforeFunction) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_2 = OpConstant %uint 2
%_arr_uint_uint_2 = OpTypeArray %uint %uint_2
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
 %_struct_13 = OpTypeStruct %bool %uint %int %float
          %1 = OpUndef %bool
          %2 = OpUndef %uint
          %3 = OpUndef %int
          %4 = OpUndef %float
          %5 = OpUndef %_arr_uint_uint_2
          %6 = OpUndef %mat2v2float
          %7 = OpUndef %v2uint
          %8 = OpUndef %v2int
          %9 = OpUndef %v2float
         %10 = OpUndef %_struct_13
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
         %11 = OpCopyObject %bool %1
         %12 = OpCopyObject %uint %2
         %13 = OpCopyObject %int %3
         %14 = OpCopyObject %float %4
         %15 = OpCopyObject %_arr_uint_uint_2 %5
         %16 = OpCopyObject %mat2v2float %6
         %17 = OpCopyObject %v2uint %7
         %18 = OpCopyObject %v2int %8
         %19 = OpCopyObject %v2float %9
         %20 = OpCopyObject %_struct_13 %10
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_4 = struct @align(4) {
  tint_symbol:bool @offset(0)
  tint_symbol_1:u32 @offset(4)
  tint_symbol_2:i32 @offset(8)
  tint_symbol_3:f32 @offset(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = let false
    %3:u32 = let 0u
    %4:i32 = let 0i
    %5:f32 = let 0.0f
    %6:array<u32, 2> = let array<u32, 2>(0u)
    %7:mat2x2<f32> = let mat2x2<f32>(vec2<f32>(0.0f))
    %8:vec2<u32> = let vec2<u32>(0u)
    %9:vec2<i32> = let vec2<i32>(0i)
    %10:vec2<f32> = let vec2<f32>(0.0f)
    %11:tint_symbol_4 = let tint_symbol_4(false, 0u, 0i, 0.0f)
    ret
  }
)");
}

TEST_F(SpirvParserTest, Misc_OpCopyObject) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_2 = OpConstant %uint 2
%_arr_uint_uint_2 = OpTypeArray %uint %uint_2
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
 %_struct_13 = OpTypeStruct %bool %uint %int %float
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpUndef %bool
          %2 = OpUndef %uint
          %3 = OpUndef %int
          %4 = OpUndef %float
          %5 = OpUndef %_arr_uint_uint_2
          %6 = OpUndef %mat2v2float
          %7 = OpUndef %v2uint
          %8 = OpUndef %v2int
          %9 = OpUndef %v2float
         %10 = OpUndef %_struct_13
         %11 = OpCopyObject %bool %1
         %12 = OpCopyObject %uint %2
         %13 = OpCopyObject %int %3
         %14 = OpCopyObject %float %4
         %15 = OpCopyObject %_arr_uint_uint_2 %5
         %16 = OpCopyObject %mat2v2float %6
         %17 = OpCopyObject %v2uint %7
         %18 = OpCopyObject %v2int %8
         %19 = OpCopyObject %v2float %9
         %20 = OpCopyObject %_struct_13 %10
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_4 = struct @align(4) {
  tint_symbol:bool @offset(0)
  tint_symbol_1:u32 @offset(4)
  tint_symbol_2:i32 @offset(8)
  tint_symbol_3:f32 @offset(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = let false
    %3:u32 = let 0u
    %4:i32 = let 0i
    %5:f32 = let 0.0f
    %6:array<u32, 2> = let array<u32, 2>(0u)
    %7:mat2x2<f32> = let mat2x2<f32>(vec2<f32>(0.0f))
    %8:vec2<u32> = let vec2<u32>(0u)
    %9:vec2<i32> = let vec2<i32>(0i)
    %10:vec2<f32> = let vec2<f32>(0.0f)
    %11:tint_symbol_4 = let tint_symbol_4(false, 0u, 0i, 0.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, OpUnreachable_TopLevel) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpUnreachable
               OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(SpirvParserTest, OpUnreachable_InsideIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
    %ep_type = OpTypeFunction %void
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpBranchConditional %true %20 %99
         %20 = OpLabel
               OpUnreachable
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        unreachable
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, OpUnreachable_InsideLoop) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
    %ep_type = OpTypeFunction %void
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %30 %30
         %30 = OpLabel
               OpUnreachable
         %80 = OpLabel
               OpBranch %20
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:bool = or true, true
        if %2 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            unreachable
          }
          $B5: {  # false
            unreachable
          }
        }
        unreachable
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, OpUnreachable_InNonVoidFunction) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
    %ep_type = OpTypeFunction %void
     %boolfn = OpTypeFunction %bool
        %200 = OpFunction %bool None %boolfn
        %210 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %11 = OpFunctionCall %bool %200
               OpReturn
               OpFunctionEnd
  )",
              R"(
%1 = func():bool {
  $B1: {
    unreachable
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

TEST_F(SpirvParserTest, OpTerminateInvocation) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_terminate_invocation"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpTerminateInvocation
               OpFunctionEnd)",
              R"(
%main = @fragment func():void {
  $B1: {
    discard
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, OpKill_TopLevel) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpKill
               OpFunctionEnd)",
              R"(
%main = @fragment func():void {
  $B1: {
    discard
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, OpKill_InsideIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
       %void = OpTypeVoid
       %bool = OpTypeBool
    %ep_type = OpTypeFunction %void
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpBranchConditional %true %20 %99
         %20 = OpLabel
               OpKill
         %99 = OpLabel
               OpKill
               OpFunctionEnd
  )",
              R"(
%main = @fragment func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        discard
        ret
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    discard
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, OpKill_InsideLoop) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
       %void = OpTypeVoid
       %bool = OpTypeBool
    %ep_type = OpTypeFunction %void
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %30 %30
         %30 = OpLabel
               OpKill
         %80 = OpLabel
               OpBranch %20
         %99 = OpLabel
               OpKill
               OpFunctionEnd
  )",
              R"(
%main = @fragment func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:bool = or true, true
        if %2 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            discard
            ret
          }
          $B5: {  # false
            unreachable
          }
        }
        unreachable
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    discard
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, OpKill_InNonVoidFunction) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
       %void = OpTypeVoid
       %bool = OpTypeBool
    %ep_type = OpTypeFunction %void
     %boolfn = OpTypeFunction %bool
        %200 = OpFunction %bool None %boolfn
        %210 = OpLabel
               OpKill
               OpFunctionEnd
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %11 = OpFunctionCall %bool %200
               OpReturn
               OpFunctionEnd
  )",
              R"(
%1 = func():bool {
  $B1: {
    discard
    ret false
  }
}
%main = @fragment func():void {
  $B2: {
    %3:bool = call %1
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
