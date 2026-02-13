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

TEST_F(SpirvParserTest, Dot) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %ep_type = OpTypeFunction %void
   %float_50 = OpConstant %float 50
   %float_60 = OpConstant %float 60
%v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
%v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpDot %float %v2float_50_60 %v2float_60_50
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = dot vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Scalar_UnsignedToUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %uint %uint_10
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_count<u32> 10u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Scalar_UnsignedToSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_10 = OpConstant %uint 10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %int %uint_10
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_count<i32> 10u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Scalar_SignedToUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %uint %int_20
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_count<u32> 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Scalar_SignedToSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %int %int_20
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_count<i32> 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Vector_UnsignedToUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %v2_uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2_uint_10_20 = OpConstantComposite %v2_uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %v2_uint %v2_uint_10_20
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_count<u32> vec2<u32>(10u, 20u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Vector_UnsignedToSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2_int = OpTypeVector %int 2
    %v2_uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2_uint_10_20 = OpConstantComposite %v2_uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %v2_int %v2_uint_10_20
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_count<i32> vec2<u32>(10u, 20u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Vector_SignedToUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2_int = OpTypeVector %int 2
    %v2_uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
%v2_int_10_20 = OpConstantComposite %v2_int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %v2_uint %v2_int_10_20
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_count<u32> vec2<i32>(10i, 20i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Vector_SignedToSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %v2_int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
%v2_int_10_20 = OpConstantComposite %v2_int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %v2_int %v2_int_10_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_count<i32> vec2<i32>(10i, 20i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_Int_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %int %int_30 %int_40 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_insert 30i, 40i, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_Int_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %int %int_30 %int_40 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_insert 30i, 40i, 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_IntVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %v2int = OpTypeVector %int 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
%v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
%v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %v2int %v2int_30_40 %v2int_40_30 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_insert vec2<i32>(30i, 40i), vec2<i32>(40i, 30i), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_IntVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
%v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
%v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %v2int %v2int_30_40 %v2int_40_30 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_insert vec2<i32>(30i, 40i), vec2<i32>(40i, 30i), 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_Uint_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %uint %uint_10 %uint_20 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_insert 10u, 20u, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_Uint_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %uint %uint_10 %uint_20 %int_30 %int_40
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_insert 10u, 20u, 30i, 40i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_UintVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
%v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %v2uint %v2uint_10_20 %v2uint_20_10 %uint_10 %uint_20
     OpReturn
     OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_insert vec2<u32>(10u, 20u), vec2<u32>(20u, 10u), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_UintVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
%v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %v2uint %v2uint_10_20 %v2uint_20_10 %int_30 %int_40
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_insert vec2<u32>(10u, 20u), vec2<u32>(20u, 10u), 30i, 40i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_UintVector_SignedOffsetAndUnsignedCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
%v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %v2uint %v2uint_10_20 %v2uint_20_10 %int_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_insert vec2<u32>(10u, 20u), vec2<u32>(20u, 10u), 10i, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_Int_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %int %int_10 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_s_extract 10i, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_Int_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %int %int_10 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_s_extract 10i, 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_IntVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2int %v2int_10_20 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i, 20i), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_IntVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
%v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2int %v2int_10_20 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i, 20i), 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_IntVector_SignedOffsetAndUnsignedCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_20 = OpConstant %uint 20
%v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2int %v2int_10_20 %int_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i, 20i), 10i, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_Uint_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %uint %uint_10 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_s_extract 10u, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_Uint_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_10 = OpConstant %uint 10
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %uint %uint_10 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_s_extract 10u, 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_UintVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2uint %v2uint_10_20 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u, 20u), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_UintVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2uint %v2uint_10_20 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u, 20u), 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_UintVector_SignedOffsetAndUnsignedCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2uint %v2uint_10_20 %int_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u, 20u), 10i, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_Uint_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %uint %uint_10 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_u_extract 10u, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_Uint_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_10 = OpConstant %uint 10
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %uint %uint_10 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_u_extract 10u, 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_UintVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2uint %v2uint_10_20 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u, 20u), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_UintVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2uint %v2uint_10_20 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u, 20u), 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_UintVector_UnsignedOffsetAndSignedCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2uint %v2uint_10_20 %uint_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u, 20u), 10u, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_Int_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %int_10 = OpConstant %int 10
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %int %int_10 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_u_extract 10i, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_Int_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %int %int_10 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_u_extract 10i, 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_IntVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2int %v2int_10_20 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i, 20i), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_IntVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
 %v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2int %v2int_10_20 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i, 20i), 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_IntVector_UnsignedOffsetAndSignedCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
 %v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2int %v2int_10_20 %uint_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i, 20i), 10u, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitReverse_Uint) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitReverse %uint %uint_10
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = reverseBits 10u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitReverse_Int) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitReverse %int %int_10
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = reverseBits 10i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitReverse_UintVector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
 %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitReverse %v2uint %v2uint_10_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = reverseBits vec2<u32>(10u, 20u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitReverse_IntVector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
 %v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitReverse %v2int %v2int_10_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = reverseBits vec2<i32>(10i, 20i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, All) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
 %v2bool_true_false = OpConstantComposite %v2bool %true %false
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpAll %bool %v2bool_true_false
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = all vec2<bool>(true, false)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Any) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
 %v2bool_true_false = OpConstantComposite %v2bool %true %false
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpAny %bool %v2bool_true_false
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = any vec2<bool>(true, false)
    ret
  }
}
)");
}

struct BuiltinData {
    const std::string spirv;
    const std::string ir;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.spirv << "," << data.ir;
    return out;
}

using SpirvParser_DerivativeTest = SpirvParserTestWithParam<BuiltinData>;

TEST_P(SpirvParser_DerivativeTest, Scalar) {
    auto& builtin = GetParam();

    EXPECT_IR(R"(
               OpCapability DerivativeControl
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
       %void = OpTypeVoid
      %float = OpTypeFloat 32
   %float_50 = OpConstant %float 50
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
         %1 = )" + builtin.spirv +
                  R"( %float %float_50
     OpReturn
     OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:f32 = )" + builtin.ir +
                  R"( 50.0f
    ret
  }
}
)");
}

TEST_P(SpirvParser_DerivativeTest, Vector) {
    auto& builtin = GetParam();

    EXPECT_IR(R"(
                OpCapability DerivativeControl
                OpCapability Shader
                OpMemoryModel Logical GLSL450
                OpEntryPoint Fragment %main "main"
                OpExecutionMode %main OriginUpperLeft
        %void = OpTypeVoid
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
   %float_50 = OpConstant %float 50
   %float_60 = OpConstant %float 60
 %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
     %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
       %entry = OpLabel
          %1 = )" +
                  builtin.spirv + R"( %v2float %v2float_50_60
      OpReturn
      OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:vec2<f32> = )" +
                  builtin.ir + R"( vec2<f32>(50.0f, 60.0f)
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParserTest,
                         SpirvParser_DerivativeTest,
                         testing::Values(BuiltinData{"OpDPdx", "dpdx"},
                                         BuiltinData{"OpDPdy", "dpdy"},
                                         BuiltinData{"OpFwidth", "fwidth"},
                                         BuiltinData{"OpDPdxFine", "dpdxFine"},
                                         BuiltinData{"OpDPdyFine", "dpdyFine"},
                                         BuiltinData{"OpFwidthFine", "fwidthFine"},
                                         BuiltinData{"OpDPdxCoarse", "dpdxCoarse"},
                                         BuiltinData{"OpDPdyCoarse", "dpdyCoarse"},
                                         BuiltinData{"OpFwidthCoarse", "fwidthCoarse"}));

TEST_F(SpirvParserTest, Transpose_2x2) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
      %float = OpTypeFloat 32
        %one = OpConstant %float 1
        %two = OpConstant %float 2
      %three = OpConstant %float 3
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
  %m2v2float = OpTypeMatrix %v2float 2
  %m2v3float = OpTypeMatrix %v3float 2
  %m3v2float = OpTypeMatrix %v2float 3
      %v2one = OpConstantComposite %v2float %one %one
      %v2two = OpConstantComposite %v2float %two %two
    %v3three = OpConstantComposite %v3float %three %three %three
%m2v2_one_two = OpConstantComposite %m2v2float %v2one %v2two
%m3v2_two_one_two = OpConstantComposite %m3v2float %v2two %v2one %v2two
%m2v3_threee_three = OpConstantComposite %m2v3float %v3three %v3three
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpTranspose %m2v2float %m2v2_one_two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f32> = transpose mat2x2<f32>(vec2<f32>(1.0f), vec2<f32>(2.0f))
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Transpose_2x3) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
      %float = OpTypeFloat 32
        %one = OpConstant %float 1
        %two = OpConstant %float 2
      %three = OpConstant %float 3
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
  %m2v2float = OpTypeMatrix %v2float 2
  %m2v3float = OpTypeMatrix %v3float 2
  %m3v2float = OpTypeMatrix %v2float 3
      %v2one = OpConstantComposite %v2float %one %one
      %v2two = OpConstantComposite %v2float %two %two
    %v3three = OpConstantComposite %v3float %three %three %three
%m2v2_one_two = OpConstantComposite %m2v2float %v2one %v2two
%m3v2_two_one_two = OpConstantComposite %m3v2float %v2two %v2one %v2two
%m2v3_three_three = OpConstantComposite %m2v3float %v3three %v3three
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpTranspose %m3v2float %m2v3_three_three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x2<f32> = transpose mat2x3<f32>(vec3<f32>(3.0f))
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Transpose_3x2) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
      %float = OpTypeFloat 32
        %one = OpConstant %float 1
        %two = OpConstant %float 2
      %three = OpConstant %float 3
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
  %m2v2float = OpTypeMatrix %v2float 2
  %m2v3float = OpTypeMatrix %v3float 2
  %m3v2float = OpTypeMatrix %v2float 3
      %v2one = OpConstantComposite %v2float %one %one
      %v2two = OpConstantComposite %v2float %two %two
    %v3three = OpConstantComposite %v3float %three %three %three
%m2v2_one_two = OpConstantComposite %m2v2float %v2one %v2two
%m3v2_two_one_two = OpConstantComposite %m3v2float %v2two %v2one %v2two
%m2v3_threee_three = OpConstantComposite %m2v3float %v3three %v3three
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpTranspose %m2v3float %m3v2_two_one_two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x3<f32> = transpose mat3x2<f32>(vec2<f32>(2.0f), vec2<f32>(1.0f), vec2<f32>(2.0f))
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, SelectScalar) {
    EXPECT_IR(R"(
             OpCapability Shader
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
     %void = OpTypeVoid
    %float = OpTypeFloat 32
    %bool = OpTypeBool
  %ep_type = OpTypeFunction %void
 %float_50 = OpConstant %float 50
 %float_60 = OpConstant %float 60
   %true   = OpConstantTrue %bool
     %main = OpFunction %void None %ep_type
    %entry = OpLabel
        %1 = OpSelect %float %true %float_50 %float_60
             OpReturn
             OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.select true, 50.0f, 60.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, SelectVector) {
    EXPECT_IR(R"(
             OpCapability Shader
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
     %void = OpTypeVoid
    %float = OpTypeFloat 32
    %bool = OpTypeBool
  %v2float = OpTypeVector %float 2
   %v2bool = OpTypeVector %bool 2
  %ep_type = OpTypeFunction %void
 %float_50 = OpConstant %float 50
 %float_60 = OpConstant %float 60
%v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
%v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
   %true   = OpConstantTrue %bool
  %false   = OpConstantFalse %bool
%true_false_vec2 = OpConstantComposite %v2bool %true %false
     %main = OpFunction %void None %ep_type
    %entry = OpLabel
        %1 = OpSelect %v2float %true_false_vec2 %v2float_50_60 %v2float_60_50
             OpReturn
             OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.select vec2<bool>(true, false), vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorExtractDynamic) {
    EXPECT_IR(R"(
       OpCapability Shader
       OpMemoryModel Logical GLSL450
       OpEntryPoint GLCompute %main "main"
       OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%float = OpTypeFloat 32
%uint = OpTypeInt 32 0
%v4float = OpTypeVector %float 4
%ep_type = OpTypeFunction %void
%float_1 = OpConstant %float 1.0
%float_2 = OpConstant %float 2.0
%float_3 = OpConstant %float 3.0
%float_4 = OpConstant %float 4.0
%uint_2  = OpConstant %uint 2
%vec = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%main = OpFunction %void None %ep_type
%entry = OpLabel
  %1 = OpVectorExtractDynamic %float %vec %uint_2
       OpReturn
       OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = access vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), 2u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, OuterProductVec2Vec3) {
    EXPECT_IR(R"(
         OpCapability Shader
         OpMemoryModel Logical GLSL450
         OpEntryPoint GLCompute %main "main"
         OpExecutionMode %main LocalSize 1 1 1

 %void = OpTypeVoid
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v3float = OpTypeVector %float 3
%mat2x3float = OpTypeMatrix %v3float 2
%ep_type = OpTypeFunction %void

%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5

%vec2 = OpConstantComposite %v2float %float_1 %float_2
%vec3 = OpConstantComposite %v3float %float_3 %float_4 %float_5

 %main = OpFunction %void None %ep_type
%entry = OpLabel
    %1 = OpOuterProduct %mat2x3float %vec3 %vec2
         OpReturn
         OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x3<f32> = spirv.outer_product vec3<f32>(3.0f, 4.0f, 5.0f), vec2<f32>(1.0f, 2.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, OuterProductVec3Vec2) {
    EXPECT_IR(R"(
         OpCapability Shader
         OpMemoryModel Logical GLSL450
         OpEntryPoint GLCompute %main "main"
         OpExecutionMode %main LocalSize 1 1 1

 %void = OpTypeVoid
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%v3float = OpTypeVector %float 3
%mat3x2float = OpTypeMatrix %v2float 3
%ep_type = OpTypeFunction %void

%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5

%vec2 = OpConstantComposite %v2float %float_1 %float_2
%vec3 = OpConstantComposite %v3float %float_3 %float_4 %float_5

 %main = OpFunction %void None %ep_type
%entry = OpLabel
    %1 = OpOuterProduct %mat3x2float %vec2 %vec3
         OpReturn
         OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x2<f32> = spirv.outer_product vec2<f32>(1.0f, 2.0f), vec3<f32>(3.0f, 4.0f, 5.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, NonUniformAll) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformVote
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformAll %bool %uint_3 %true
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = subgroupAll true
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformAny) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformVote
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformAny %bool %uint_3 %true
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = subgroupAny true
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformElect) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformVote
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformElect %bool %uint_3
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = subgroupElect
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBroadcast_Constant_BoolScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBroadcast %bool %uint_3 %true %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_broadcast 3u, true, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBroadcast_Constant_BoolVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3bool = OpTypeVector %bool 3
         %12 = OpConstantComposite %v3bool %true %false %true
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBroadcast %v3bool %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_broadcast 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBroadcast_Constant_NumericScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBroadcast %uint %uint_3 %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_broadcast 3u, 3u, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBroadcast_Constant_NumericVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBroadcast %v3uint %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_broadcast 3u, vec3<u32>(1u, 3u, 1u), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBroadcast_NonConstant_NumericVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %7 = OpCopyObject %v3uint %12
          %8 = OpGroupNonUniformBroadcast %v3uint %uint_3 %7 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = let vec3<u32>(1u, 3u, 1u)
    %3:vec3<u32> = spirv.group_non_uniform_broadcast 3u, %2, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBroadcastFirst_Constant_BoolScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBroadcastFirst %bool %uint_3 %true
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_broadcast_first 3u, true
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBroadcastFirst_Constant_BoolVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
     %v3bool = OpTypeVector %bool 3
         %12 = OpConstantComposite %v3bool %true %false %true
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBroadcastFirst %v3bool %uint_3 %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_broadcast_first 3u, vec3<bool>(true, false, true)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBroadcastFirst_Constant_NumericScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBroadcastFirst %uint %uint_3 %uint_3
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_broadcast_first 3u, 3u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBroadcastFirst_Constant_NumericVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBroadcastFirst %v3uint %uint_3 %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_broadcast_first 3u, vec3<u32>(1u, 3u, 1u)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBallot) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
     %v4uint = OpTypeVector %uint 4
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBallot %v4uint %uint_3 %true
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec4<u32> = subgroupBallot true
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformQuadBroadcast_Constant_BoolScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformQuad
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformQuadBroadcast %bool %uint_3 %true %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_quad_broadcast 3u, true, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformQuadBroadcast_Constant_BoolVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformQuad
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3bool = OpTypeVector %bool 3
         %12 = OpConstantComposite %v3bool %true %false %true
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformQuadBroadcast %v3bool %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_quad_broadcast 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformQuadBroadcast_Constant_NumericScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformQuad
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformQuadBroadcast %uint %uint_3 %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_quad_broadcast 3u, 3u, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformQuadBroadcast_Constant_NumericVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformQuad
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformQuadBroadcast %v3uint %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_quad_broadcast 3u, vec3<u32>(1u, 3u, 1u), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformQuadSwap_Constant_BoolScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformQuad
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformQuadSwap %bool %uint_3 %true %uint_0
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_quad_swap 3u, true, 0u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformQuadSwap_Constant_BoolVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformQuad
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3bool = OpTypeVector %bool 3
         %12 = OpConstantComposite %v3bool %true %false %true
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformQuadSwap %v3bool %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_quad_swap 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformQuadSwap_Constant_NumericScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformQuad
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformQuadSwap %uint %uint_3 %uint_1 %uint_2
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_quad_swap 3u, 1u, 2u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformQuadSwap_Constant_NumericVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformQuad
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformQuadSwap %v3uint %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_quad_swap 3u, vec3<u32>(1u, 3u, 1u), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffle_Constant_BoolScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffle
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffle %bool %uint_3 %true %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_shuffle 3u, true, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffle_NonConstant_BoolScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffle
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %7 = OpCopyObject %bool %true
          %8 = OpGroupNonUniformShuffle %bool %uint_3 %7 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = let true
    %3:bool = spirv.group_non_uniform_shuffle 3u, %2, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffle_Constant_BoolVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffle
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3bool = OpTypeVector %bool 3
         %12 = OpConstantComposite %v3bool %true %false %true
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffle %v3bool %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_shuffle 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffle_Constant_NumericScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffle
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffle %uint %uint_3 %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_shuffle 3u, 3u, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffle_Constant_NumericVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffle
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffle %v3uint %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_shuffle 3u, vec3<u32>(1u, 3u, 1u), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleXor_Constant_BoolScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffle
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleXor %bool %uint_3 %true %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_shuffle_xor 3u, true, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleXor_Constant_BoolVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffle
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3bool = OpTypeVector %bool 3
         %12 = OpConstantComposite %v3bool %true %false %true
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleXor %v3bool %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_shuffle_xor 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleXor_Constant_NumericScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffle
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleXor %uint %uint_3 %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_shuffle_xor 3u, 3u, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleXor_Constant_NumericVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffle
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleXor %v3uint %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_shuffle_xor 3u, vec3<u32>(1u, 3u, 1u), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleDown_Constant_BoolScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffleRelative
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleDown %bool %uint_3 %true %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_shuffle_down 3u, true, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleDown_Constant_BoolVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffleRelative
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3bool = OpTypeVector %bool 3
         %12 = OpConstantComposite %v3bool %true %false %true
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleDown %v3bool %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_shuffle_down 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleDown_Constant_NumericScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffleRelative
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleDown %uint %uint_3 %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_shuffle_down 3u, 3u, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleDown_Constant_NumericVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffleRelative
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleDown %v3uint %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_shuffle_down 3u, vec3<u32>(1u, 3u, 1u), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleUp_Constant_BoolScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffleRelative
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleUp %bool %uint_3 %true %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_shuffle_up 3u, true, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleUp_Constant_BoolVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffleRelative
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3bool = OpTypeVector %bool 3
         %12 = OpConstantComposite %v3bool %true %false %true
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleUp %v3bool %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_shuffle_up 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleUp_Constant_NumericScalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffleRelative
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleUp %uint %uint_3 %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_shuffle_up 3u, 3u, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformShuffleUp_Constant_NumericVector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformShuffleRelative
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformShuffleUp %v3uint %uint_3 %12 %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_shuffle_up 3u, vec3<u32>(1u, 3u, 1u), 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformSMin_Scalar_i32) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformSMin %int %uint_3 Reduce %int_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.group_non_uniform_s_min 3u, 0u, 1i
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformSMin_Vector_i32) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformSMin %v3int %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = spirv.group_non_uniform_s_min 3u, 0u, vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformSMin_Scalar_u32) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformSMin %uint %uint_3 Reduce %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_s_min 3u, 0u, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformSMin_Vector_u32) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformSMin %v3uint %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_s_min 3u, 0u, vec3<u32>(1u, 3u, 1u)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformUMin_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformUMin %uint %uint_3 Reduce %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupMin 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformUMin_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformUMin %v3uint %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = subgroupMin vec3<u32>(1u, 3u, 1u)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFMin_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFMin %float %uint_3 Reduce %float_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = subgroupMin 1.0f
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFMin_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %12 = OpConstantComposite %v3float %float_1 %float_3 %float_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFMin %v3float %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = subgroupMin vec3<f32>(1.0f, 3.0f, 1.0f)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformSMax_Scalar_i32) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformSMax %int %uint_3 Reduce %int_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.group_non_uniform_s_max 3u, 0u, 1i
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformSMax_Vector_i32) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformSMax %v3int %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = spirv.group_non_uniform_s_max 3u, 0u, vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformSMax_Scalar_u32) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformSMax %uint %uint_3 Reduce %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_s_max 3u, 0u, 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformSMax_Vector_u32) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformSMax %v3uint %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_s_max 3u, 0u, vec3<u32>(1u, 3u, 1u)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformUMax_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformUMax %uint %uint_3 Reduce %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupMax 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformUMax_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
         %12 = OpConstantComposite %v3uint %uint_1 %uint_3 %uint_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformUMax %v3uint %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = subgroupMax vec3<u32>(1u, 3u, 1u)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFMax_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFMax %float %uint_3 Reduce %float_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = subgroupMax 1.0f
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFMax_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %12 = OpConstantComposite %v3float %float_1 %float_3 %float_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFMax %v3float %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = subgroupMax vec3<f32>(1.0f, 3.0f, 1.0f)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIAdd_Reduce_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIAdd %uint %uint_3 Reduce %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupAdd 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIAdd_Reduce_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIAdd %v3int %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupAdd vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFAdd_Reduce_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFAdd %float %uint_3 Reduce %float_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = subgroupAdd 1.0f
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFAdd_Reduce_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %12 = OpConstantComposite %v3float %float_1 %float_3 %float_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFAdd %v3float %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = subgroupAdd vec3<f32>(1.0f, 3.0f, 1.0f)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIAdd_InclusiveScan_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIAdd %uint %uint_3 InclusiveScan %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupInclusiveAdd 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIAdd_InclusiveScan_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIAdd %v3int %uint_3 InclusiveScan %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupInclusiveAdd vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFAdd_InclusiveScan_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFAdd %float %uint_3 InclusiveScan %float_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = subgroupInclusiveAdd 1.0f
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFAdd_InclusiveScan_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %12 = OpConstantComposite %v3float %float_1 %float_3 %float_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFAdd %v3float %uint_3 InclusiveScan %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = subgroupInclusiveAdd vec3<f32>(1.0f, 3.0f, 1.0f)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIAdd_ExclusiveScan_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIAdd %uint %uint_3 ExclusiveScan %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupExclusiveAdd 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIAdd_ExclusiveScan_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIAdd %v3int %uint_3 ExclusiveScan %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupExclusiveAdd vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFAdd_ExclusiveScan_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFAdd %float %uint_3 ExclusiveScan %float_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = subgroupExclusiveAdd 1.0f
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFAdd_ExclusiveScan_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %12 = OpConstantComposite %v3float %float_1 %float_3 %float_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFAdd %v3float %uint_3 ExclusiveScan %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = subgroupExclusiveAdd vec3<f32>(1.0f, 3.0f, 1.0f)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIMul_Reduce_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIMul %uint %uint_3 Reduce %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupMul 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIMul_Reduce_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIMul %v3int %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupMul vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFMul_Reduce_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFMul %float %uint_3 Reduce %float_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = subgroupMul 1.0f
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFMul_Reduce_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %12 = OpConstantComposite %v3float %float_1 %float_3 %float_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFMul %v3float %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = subgroupMul vec3<f32>(1.0f, 3.0f, 1.0f)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIMul_InclusiveScan_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIMul %uint %uint_3 InclusiveScan %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupInclusiveMul 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIMul_InclusiveScan_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIMul %v3int %uint_3 InclusiveScan %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupInclusiveMul vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFMul_InclusiveScan_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFMul %float %uint_3 InclusiveScan %float_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = subgroupInclusiveMul 1.0f
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFMul_InclusiveScan_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %12 = OpConstantComposite %v3float %float_1 %float_3 %float_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFMul %v3float %uint_3 InclusiveScan %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = subgroupInclusiveMul vec3<f32>(1.0f, 3.0f, 1.0f)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIMul_ExclusiveScan_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIMul %uint %uint_3 ExclusiveScan %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupExclusiveMul 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformIMul_ExclusiveScan_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformIMul %v3int %uint_3 ExclusiveScan %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupExclusiveMul vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFMul_ExclusiveScan_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFMul %float %uint_3 ExclusiveScan %float_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = subgroupExclusiveMul 1.0f
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformFMul_ExclusiveScan_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %12 = OpConstantComposite %v3float %float_1 %float_3 %float_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformFMul %v3float %uint_3 ExclusiveScan %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = subgroupExclusiveMul vec3<f32>(1.0f, 3.0f, 1.0f)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBitwiseAnd_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBitwiseAnd %uint %uint_3 Reduce %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupAnd 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBitwiseAnd_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBitwiseAnd %v3int %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupAnd vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBitwiseOr_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBitwiseOr %uint %uint_3 Reduce %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupOr 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBitwiseOr_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBitwiseOr %v3int %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupOr vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBitwiseXor_Scalar) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBitwiseXor %uint %uint_3 Reduce %uint_1
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupXor 1u
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

TEST_F(SpirvParserTest, NonUniformBitwiseXor_Vector) {
    EXPECT_IR_SPV(R"(
               OpCapability Shader
               OpCapability GroupNonUniformArithmetic
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %v3int = OpTypeVector %int 3
         %12 = OpConstantComposite %v3int %int_1 %int_3 %int_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %main = OpFunction %void None %23
         %24 = OpLabel
          %8 = OpGroupNonUniformBitwiseXor %v3int %uint_3 Reduce %12
               OpReturn
               OpFunctionEnd
)",
                  R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupXor vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)",
                  SPV_ENV_VULKAN_1_1);
}

}  // namespace
}  // namespace tint::spirv::reader
