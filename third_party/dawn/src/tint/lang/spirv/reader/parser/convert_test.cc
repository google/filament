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

TEST_F(SpirvParserTest, ConvertFToS_ScalarSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
        %two = OpConstant %float 2
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertFToS %int %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.convert_f_to_s<i32> 2.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertFToS_ScalarUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
        %two = OpConstant %float 2
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertFToS %uint %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.convert_f_to_s<u32> 2.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertFToS_VectorSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
        %two = OpConstant %float 2
     %v2_two = OpConstantComposite %v2float %two %two
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertFToS %v2int %v2_two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.convert_f_to_s<i32> vec2<f32>(2.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertFToS_VectorUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
        %two = OpConstant %float 2
     %v2_two = OpConstantComposite %v2float %two %two
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertFToS %v2uint %v2_two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.convert_f_to_s<u32> vec2<f32>(2.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertFToU_ScalarUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
        %two = OpConstant %float 2
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertFToU %uint %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = convert 2.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertFToU_VectorUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
        %two = OpConstant %float 2
     %v2_two = OpConstantComposite %v2float %two %two
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertFToU %v2uint %v2_two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = convert vec2<f32>(2.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertSToF_ScalarSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
        %two = OpConstant %int 2
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertSToF %float %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_s_to_f<f32> 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertSToF_ScalarUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
        %two = OpConstant %uint 2
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertSToF %float %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_s_to_f<f32> 2u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertSToF_VectorSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
        %two = OpConstant %int 2
     %v2_two = OpConstantComposite %v2int %two %two
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertSToF %v2float %v2_two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_s_to_f<f32> vec2<i32>(2i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertSToF_VectorUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
        %two = OpConstant %uint 2
     %v2_two = OpConstantComposite %v2uint %two %two
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertSToF %v2float %v2_two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_s_to_f<f32> vec2<u32>(2u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertUToF_ScalarSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
        %two = OpConstant %int 2
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertUToF %float %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_u_to_f<f32> 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertUToF_ScalarUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
        %two = OpConstant %uint 2
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertUToF %float %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_u_to_f<f32> 2u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertUToF_VectorSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
        %two = OpConstant %int 2
     %v2_two = OpConstantComposite %v2int %two %two
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertUToF %v2float %v2_two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_u_to_f<f32> vec2<i32>(2i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ConvertUToF_VectorUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
        %two = OpConstant %uint 2
     %v2_two = OpConstantComposite %v2uint %two %two
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = OpConvertUToF %v2float %v2_two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_u_to_f<f32> vec2<u32>(2u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FConvert_ScalarF32ToF16) {
    EXPECT_IR(R"(
             OpCapability Shader
             OpCapability Float16
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
     %void    = OpTypeVoid
     %float16 = OpTypeFloat 16
     %float32 = OpTypeFloat 32
     %two     = OpConstant %float32 2
     %void_fn = OpTypeFunction %void

     %main    = OpFunction %void None %void_fn
     %main_start = OpLabel
             %1 = OpFConvert %float16 %two
             OpReturn
     OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f16 = convert 2.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FConvert_ScalarF16ToF32) {
    EXPECT_IR(R"(
             OpCapability Shader
             OpCapability Float16
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
     %void    = OpTypeVoid
     %float16 = OpTypeFloat 16
     %float32 = OpTypeFloat 32
     %two     = OpConstant %float16 2
     %void_fn = OpTypeFunction %void

     %main    = OpFunction %void None %void_fn
     %main_start = OpLabel
             %1 = OpFConvert %float32 %two
             OpReturn
     OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = convert 2.0h
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FConvert_VectorF32ToF16) {
    EXPECT_IR(R"(
             OpCapability Shader
             OpCapability Float16
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
     %void       = OpTypeVoid
     %float16    = OpTypeFloat 16
     %float32    = OpTypeFloat 32
     %v2float16  = OpTypeVector %float16 2
     %v2float32  = OpTypeVector %float32 2
     %two        = OpConstant %float32 2
     %v2_two     = OpConstantComposite %v2float32 %two %two
     %void_fn    = OpTypeFunction %void

     %main       = OpFunction %void None %void_fn
     %main_start = OpLabel
             %1 = OpFConvert %v2float16 %v2_two
             OpReturn
     OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f16> = convert vec2<f32>(2.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FConvert_VectorF16ToF32) {
    EXPECT_IR(R"(
             OpCapability Shader
             OpCapability Float16
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
     %void       = OpTypeVoid
     %float16    = OpTypeFloat 16
     %float32    = OpTypeFloat 32
     %v2float16  = OpTypeVector %float16 2
     %v2float32  = OpTypeVector %float32 2
     %two       = OpConstant %float16 2
     %v2_two    = OpConstantComposite %v2float16 %two %two
     %void_fn    = OpTypeFunction %void

     %main       = OpFunction %void None %void_fn
     %main_start = OpLabel
             %1 = OpFConvert %v2float32 %v2_two
             OpReturn
     OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = convert vec2<f16>(2.0h)
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
