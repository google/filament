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

TEST_F(SpirvParserTest, ArrayLength_FromVar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myvar "myvar"
               OpMemberName %struct 0 "first"
               OpMemberName %struct 1 "rtarr"
               OpDecorate %myvar DescriptorSet 0
               OpDecorate %myvar Binding 0
               OpDecorate %struct Block
               OpDecorate %arr ArrayStride 4
               OpMemberDecorate %struct 0 Offset 0
               OpMemberDecorate %struct 1 Offset 4
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
        %arr = OpTypeRuntimeArray %u32
     %struct = OpTypeStruct %u32 %arr
 %ptr_struct = OpTypePointer StorageBuffer %struct
      %myvar = OpVariable %ptr_struct StorageBuffer
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpArrayLength %u32 %myvar 1
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol = struct @align(4) {
  first:u32 @offset(0)
  rtarr:array<u32> @offset(4)
}

$B1: {  # root
  %myvar:ptr<storage, tint_symbol, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<u32>, read_write> = access %myvar, 1u
    %4:u32 = arrayLength %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ArrayLength_FromCopyObject) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myvar "myvar"
               OpMemberName %struct 0 "first"
               OpMemberName %struct 1 "rtarr"
               OpDecorate %myvar DescriptorSet 0
               OpDecorate %myvar Binding 0
               OpDecorate %struct Block
               OpDecorate %arr ArrayStride 4
               OpMemberDecorate %struct 0 Offset 0
               OpMemberDecorate %struct 1 Offset 4
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
        %arr = OpTypeRuntimeArray %u32
     %struct = OpTypeStruct %u32 %arr
 %ptr_struct = OpTypePointer StorageBuffer %struct
      %myvar = OpVariable %ptr_struct StorageBuffer
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %2 = OpCopyObject %ptr_struct %myvar
          %1 = OpArrayLength %u32 %2 1
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol = struct @align(4) {
  first:u32 @offset(0)
  rtarr:array<u32> @offset(4)
}

$B1: {  # root
  %myvar:ptr<storage, tint_symbol, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, tint_symbol, read_write> = let %myvar
    %4:ptr<storage, array<u32>, read_write> = access %3, 1u
    %5:u32 = arrayLength %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ArrayLength_FromAccessChain) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myvar "myvar"
               OpMemberName %struct 0 "first"
               OpMemberName %struct 1 "rtarr"
               OpDecorate %myvar DescriptorSet 0
               OpDecorate %myvar Binding 0
               OpDecorate %struct Block
               OpDecorate %arr ArrayStride 4
               OpMemberDecorate %struct 0 Offset 0
               OpMemberDecorate %struct 1 Offset 4
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
        %arr = OpTypeRuntimeArray %u32
     %struct = OpTypeStruct %u32 %arr
 %ptr_struct = OpTypePointer StorageBuffer %struct
      %myvar = OpVariable %ptr_struct StorageBuffer
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %2 = OpAccessChain %ptr_struct %myvar ; no indices
          %1 = OpArrayLength %u32 %2 1
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol = struct @align(4) {
  first:u32 @offset(0)
  rtarr:array<u32> @offset(4)
}

$B1: {  # root
  %myvar:ptr<storage, tint_symbol, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<u32>, read_write> = access %myvar, 1u
    %4:u32 = arrayLength %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Load_Scalar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %u32_ptr Function
       %load = OpLoad %u32 %var
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var undef
    %3:u32 = load %2
    ret
  }
)");
}

TEST_F(SpirvParserTest, Load_Vector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
  %vec4u_ptr = OpTypePointer Function %vec4u
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %vec4u_ptr Function
       %load = OpLoad %vec4u %var
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:vec4<u32> = load %2
    ret
  }
)");
}

TEST_F(SpirvParserTest, Load_VectorComponent) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
    %u32_ptr = OpTypePointer Function %u32
  %vec4u_ptr = OpTypePointer Function %vec4u
      %u32_2 = OpConstant %u32 2
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %vec4u_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_2
       %load = OpLoad %u32 %access
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:ptr<function, u32, read_write> = access %2, 2u
    %4:u32 = load %3
    ret
  }
)");
}

TEST_F(SpirvParserTest, Load_Matrix) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %mat3x4f = OpTypeMatrix %vec4f 3
%mat3x4f_ptr = OpTypePointer Function %mat3x4f
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %mat3x4f_ptr Function
       %load = OpLoad %mat3x4f %var
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, mat3x4<f32>, read_write> = var undef
    %3:mat3x4<f32> = load %2
    ret
  }
)");
}

TEST_F(SpirvParserTest, Load_MatrixColumn) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %mat3x4f = OpTypeMatrix %vec4f 3
  %vec4f_ptr = OpTypePointer Function %vec4f
%mat3x4f_ptr = OpTypePointer Function %mat3x4f
      %u32_2 = OpConstant %u32 2
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %mat3x4f_ptr Function
     %access = OpAccessChain %vec4f_ptr %var %u32_2
       %load = OpLoad %vec4f %access
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, mat3x4<f32>, read_write> = var undef
    %3:ptr<function, vec4<f32>, read_write> = access %2, 2u
    %4:vec4<f32> = load %3
    ret
  }
)");
}

TEST_F(SpirvParserTest, Load_Array) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_4 = OpConstant %u32 4
        %arr = OpTypeArray %u32 %u32_4
    %arr_ptr = OpTypePointer Function %arr
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %arr_ptr Function
       %load = OpLoad %arr %var
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, array<u32, 4>, read_write> = var undef
    %3:array<u32, 4> = load %2
    ret
  }
)");
}

TEST_F(SpirvParserTest, Load_ArrayElement) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_2 = OpConstant %u32 2
      %u32_4 = OpConstant %u32 4
        %arr = OpTypeArray %u32 %u32_4
    %u32_ptr = OpTypePointer Function %u32
    %arr_ptr = OpTypePointer Function %arr
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %arr_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_2
       %load = OpLoad %u32 %access
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, array<u32, 4>, read_write> = var undef
    %3:ptr<function, u32, read_write> = access %2, 2u
    %4:u32 = load %3
    ret
  }
)");
}

TEST_F(SpirvParserTest, Store_RuntimeArrayElement) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %sg "sg"
               OpName %main "main"
               OpName %S "S"
               OpMemberName %S 0 "a"
               OpDecorate %sg DescriptorSet 0
               OpDecorate %sg Binding 1
               OpDecorate %S Block
               OpDecorate %arr ArrayStride 4
               OpMemberDecorate %S 0 Offset 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_4 = OpConstant %int 4
     %uint_0 = OpConstant %uint 0
        %arr = OpTypeRuntimeArray %uint
          %S = OpTypeStruct %arr
          %5 = OpTypeFunction %void
      %ptr_s = OpTypePointer StorageBuffer %S
         %sg = OpVariable %ptr_s StorageBuffer
   %ptr_uint = OpTypePointer StorageBuffer %uint
       %main = OpFunction %void None %5
          %8 = OpLabel
         %12 = OpAccessChain %ptr_uint %sg %uint_0 %int_4
               OpStore %12 %uint_0
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:array<u32> @offset(0)
}

$B1: {  # root
  %sg:ptr<storage, S, read_write> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %sg, 0u, 4i
    store %3, 0u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Load_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %str = OpTypeStruct %u32 %u32
    %str_ptr = OpTypePointer Function %str
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %str_ptr Function
       %load = OpLoad %str %var
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:u32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, tint_symbol_2, read_write> = var undef
    %3:tint_symbol_2 = load %2
    ret
  }
)");
}

TEST_F(SpirvParserTest, Load_StructMember) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_1 = OpConstant %u32 1
        %str = OpTypeStruct %u32 %u32
    %u32_ptr = OpTypePointer Function %u32
    %str_ptr = OpTypePointer Function %str
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %str_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_1
       %load = OpLoad %u32 %access
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:u32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, tint_symbol_2, read_write> = var undef
    %3:ptr<function, u32, read_write> = access %2, 1u
    %4:u32 = load %3
    ret
  }
)");
}

TEST_F(SpirvParserTest, Store_Scalar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
     %u32_42 = OpConstant %u32 42
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %u32_ptr Function
               OpStore %var %u32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var undef
    store %2, 42u
    ret
  }
)");
}

TEST_F(SpirvParserTest, Store_Vector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
       %null = OpConstantNull %vec4u
  %vec4u_ptr = OpTypePointer Function %vec4u
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %vec4u_ptr Function
               OpStore %var %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    store %2, vec4<u32>(0u)
    ret
  }
)");
}

TEST_F(SpirvParserTest, Store_VectorComponent) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
     %u32_42 = OpConstant %u32 42
      %vec4u = OpTypeVector %u32 4
    %u32_ptr = OpTypePointer Function %u32
  %vec4u_ptr = OpTypePointer Function %vec4u
      %u32_2 = OpConstant %u32 2
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %vec4u_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_2
               OpStore %access %u32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:ptr<function, u32, read_write> = access %2, 2u
    store %3, 42u
    ret
  }
)");
}

TEST_F(SpirvParserTest, Store_Matrix) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %mat3x4f = OpTypeMatrix %vec4f 3
       %null = OpConstantNull %mat3x4f
%mat3x4f_ptr = OpTypePointer Function %mat3x4f
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %mat3x4f_ptr Function
               OpStore %var %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, mat3x4<f32>, read_write> = var undef
    store %2, mat3x4<f32>(vec4<f32>(0.0f))
    ret
  }
)");
}

TEST_F(SpirvParserTest, Store_MatrixColumn) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %mat3x4f = OpTypeMatrix %vec4f 3
       %null = OpConstantNull %vec4f
  %vec4f_ptr = OpTypePointer Function %vec4f
%mat3x4f_ptr = OpTypePointer Function %mat3x4f
      %u32_2 = OpConstant %u32 2
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %mat3x4f_ptr Function
     %access = OpAccessChain %vec4f_ptr %var %u32_2
               OpStore %access %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, mat3x4<f32>, read_write> = var undef
    %3:ptr<function, vec4<f32>, read_write> = access %2, 2u
    store %3, vec4<f32>(0.0f)
    ret
  }
)");
}

TEST_F(SpirvParserTest, Store_Array) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_4 = OpConstant %u32 4
        %arr = OpTypeArray %u32 %u32_4
       %null = OpConstantNull %arr
    %arr_ptr = OpTypePointer Function %arr
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %arr_ptr Function
               OpStore %var %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, array<u32, 4>, read_write> = var undef
    store %2, array<u32, 4>(0u)
    ret
  }
)");
}

TEST_F(SpirvParserTest, Store_ArrayElement) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_2 = OpConstant %u32 2
      %u32_4 = OpConstant %u32 4
     %u32_42 = OpConstant %u32 42
        %arr = OpTypeArray %u32 %u32_4
    %u32_ptr = OpTypePointer Function %u32
    %arr_ptr = OpTypePointer Function %arr
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %arr_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_2
               OpStore %access %u32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, array<u32, 4>, read_write> = var undef
    %3:ptr<function, u32, read_write> = access %2, 2u
    store %3, 42u
    ret
  }
)");
}

TEST_F(SpirvParserTest, Store_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %str = OpTypeStruct %u32 %u32
       %null = OpConstantNull %str
    %str_ptr = OpTypePointer Function %str
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %str_ptr Function
               OpStore %var %null
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:u32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, tint_symbol_2, read_write> = var undef
    store %2, tint_symbol_2(0u)
    ret
  }
)");
}

TEST_F(SpirvParserTest, Store_StructMember) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_1 = OpConstant %u32 1
     %u32_42 = OpConstant %u32 42
        %str = OpTypeStruct %u32 %u32
    %u32_ptr = OpTypePointer Function %u32
    %str_ptr = OpTypePointer Function %str
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %str_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_1
               OpStore %access %u32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:u32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, tint_symbol_2, read_write> = var undef
    %3:ptr<function, u32, read_write> = access %2, 1u
    store %3, 42u
    ret
  }
)");
}

TEST_F(SpirvParserTest, CopyMemory_Scalar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %src_var "src_var"
               OpName %dst_var "dst_var"
       %void = OpTypeVoid
       %u32  = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
   %src_var  = OpVariable %u32_ptr Function
   %dst_var  = OpVariable %u32_ptr Function
               OpCopyMemory %dst_var %src_var
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %src_var:ptr<function, u32, read_write> = var undef
    %dst_var:ptr<function, u32, read_write> = var undef
    %4:u32 = load %src_var
    store %dst_var, %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CopyMemory_Vector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %src_vec "src_vec"
               OpName %dst_vec "dst_vec"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
  %vec4u_ptr = OpTypePointer Function %vec4u
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
    %src_vec = OpVariable %vec4u_ptr Function
    %dst_vec = OpVariable %vec4u_ptr Function
               OpCopyMemory %dst_vec %src_vec
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %src_vec:ptr<function, vec4<u32>, read_write> = var undef
    %dst_vec:ptr<function, vec4<u32>, read_write> = var undef
    %4:vec4<u32> = load %src_vec
    store %dst_vec, %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CopyMemory_VectorComponent) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %src_vec "src_vec"
               OpName %dst_vec "dst_vec"
               OpName %src "src"
               OpName %dst "dst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
    %u32_ptr = OpTypePointer Function %u32
  %vec4u_ptr = OpTypePointer Function %vec4u
      %u32_2 = OpConstant %u32 2
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
    %src_vec = OpVariable %vec4u_ptr Function
    %dst_vec = OpVariable %vec4u_ptr Function
        %src = OpAccessChain %u32_ptr %src_vec %u32_2
        %dst = OpAccessChain %u32_ptr %dst_vec %u32_2
               OpCopyMemory %dst %src
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %src_vec:ptr<function, vec4<u32>, read_write> = var undef
    %dst_vec:ptr<function, vec4<u32>, read_write> = var undef
    %src:ptr<function, u32, read_write> = access %src_vec, 2u
    %dst:ptr<function, u32, read_write> = access %dst_vec, 2u
    %6:u32 = load %src
    store %dst, %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CopyMemory_Matrix) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %src_mat "src_mat"
               OpName %dst_mat "dst_mat"
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %mat3x4f = OpTypeMatrix %vec4f 3
%mat3x4f_ptr = OpTypePointer Function %mat3x4f
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
    %src_mat = OpVariable %mat3x4f_ptr Function
    %dst_mat = OpVariable %mat3x4f_ptr Function
               OpCopyMemory %dst_mat %src_mat
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %src_mat:ptr<function, mat3x4<f32>, read_write> = var undef
    %dst_mat:ptr<function, mat3x4<f32>, read_write> = var undef
    %4:mat3x4<f32> = load %src_mat
    store %dst_mat, %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CopyMemory_MatrixColumn) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %src_mat "src_mat"
               OpName %mat_dst "mat_dst"
               OpName %src "src"
               OpName %dst "dst"
      %void  = OpTypeVoid
      %f32   = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %mat3x4f = OpTypeMatrix %vec4f 3
  %vec4f_ptr = OpTypePointer Function %vec4f
%mat3x4f_ptr = OpTypePointer Function %mat3x4f
      %u32   = OpTypeInt 32 0
      %u32_2 = OpConstant %u32 2
    %ep_type = OpTypeFunction %void
      %main  = OpFunction %void None %ep_type
 %main_start = OpLabel
    %src_mat = OpVariable %mat3x4f_ptr Function
    %mat_dst = OpVariable %mat3x4f_ptr Function
    %src     = OpAccessChain %vec4f_ptr %src_mat %u32_2
    %dst     = OpAccessChain %vec4f_ptr %mat_dst %u32_2
               OpCopyMemory %dst %src
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %src_mat:ptr<function, mat3x4<f32>, read_write> = var undef
    %mat_dst:ptr<function, mat3x4<f32>, read_write> = var undef
    %src:ptr<function, vec4<f32>, read_write> = access %src_mat, 2u
    %dst:ptr<function, vec4<f32>, read_write> = access %mat_dst, 2u
    %6:vec4<f32> = load %src
    store %dst, %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CopyMemory_Array) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %src_arr "src_arr"
               OpName %dst_arr "dst_arr"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_4 = OpConstant %u32 4
        %arr = OpTypeArray %u32 %u32_4
    %arr_ptr = OpTypePointer Function %arr
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
    %src_arr = OpVariable %arr_ptr Function
    %dst_arr = OpVariable %arr_ptr Function
               OpCopyMemory %dst_arr %src_arr
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %src_arr:ptr<function, array<u32, 4>, read_write> = var undef
    %dst_arr:ptr<function, array<u32, 4>, read_write> = var undef
    %4:array<u32, 4> = load %src_arr
    store %dst_arr, %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CopyMemory_ArrayElement) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %src_arr "src_arr"
               OpName %dst_arr "dst_arr"
               OpName %src "src"
               OpName %dst "dst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_2 = OpConstant %u32 2
      %u32_4 = OpConstant %u32 4
        %arr = OpTypeArray %u32 %u32_4
    %u32_ptr = OpTypePointer Function %u32
    %arr_ptr = OpTypePointer Function %arr
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
    %src_arr = OpVariable %arr_ptr Function
    %dst_arr = OpVariable %arr_ptr Function
        %src = OpAccessChain %u32_ptr %src_arr %u32_2
        %dst = OpAccessChain %u32_ptr %dst_arr %u32_2
               OpCopyMemory %dst %src
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %src_arr:ptr<function, array<u32, 4>, read_write> = var undef
    %dst_arr:ptr<function, array<u32, 4>, read_write> = var undef
    %src:ptr<function, u32, read_write> = access %src_arr, 2u
    %dst:ptr<function, u32, read_write> = access %dst_arr, 2u
    %6:u32 = load %src
    store %dst, %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CopyMemory_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %src_str "src_str"
               OpName %dst_str "dst_str"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %str = OpTypeStruct %u32 %u32
    %str_ptr = OpTypePointer Function %str
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
    %src_str = OpVariable %str_ptr Function
    %dst_str = OpVariable %str_ptr Function
               OpCopyMemory %dst_str %src_str
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:u32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %src_str:ptr<function, tint_symbol_2, read_write> = var undef
    %dst_str:ptr<function, tint_symbol_2, read_write> = var undef
    %4:tint_symbol_2 = load %src_str
    store %dst_str, %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CopyMemory_StructMember) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %src_str "src_str"
               OpName %dst_str "dst_str"
               OpName %src "src"
               OpName %dst "dst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_1 = OpConstant %u32 1
        %str = OpTypeStruct %u32 %u32
    %u32_ptr = OpTypePointer Function %u32
    %str_ptr = OpTypePointer Function %str
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
    %src_str = OpVariable %str_ptr Function
    %dst_str = OpVariable %str_ptr Function
        %src = OpAccessChain %u32_ptr %src_str %u32_1
        %dst = OpAccessChain %u32_ptr %dst_str %u32_1
               OpCopyMemory %dst %src
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:u32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %src_str:ptr<function, tint_symbol_2, read_write> = var undef
    %dst_str:ptr<function, tint_symbol_2, read_write> = var undef
    %src:ptr<function, u32, read_write> = access %src_str, 1u
    %dst:ptr<function, u32, read_write> = access %dst_str, 1u
    %6:u32 = load %src
    store %dst, %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Access_Nested_SingleAccessInstruction) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
     %u32_42 = OpConstant %u32 42
  %arr_inner = OpTypeArray %u32 %u32_4
        %str = OpTypeStruct %arr_inner %arr_inner %arr_inner %arr_inner
  %arr_outer = OpTypeArray %str %u32_4
    %u32_ptr = OpTypePointer Function %u32
%arr_outer_ptr = OpTypePointer Function %arr_outer
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %arr_outer_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_1 %u32_2 %u32_3
       %load = OpLoad %u32 %access
               OpStore %access %u32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
  tint_symbol:array<u32, 4> @offset(0)
  tint_symbol_1:array<u32, 4> @offset(16)
  tint_symbol_2:array<u32, 4> @offset(32)
  tint_symbol_3:array<u32, 4> @offset(48)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, array<tint_symbol_4, 4>, read_write> = var undef
    %3:ptr<function, u32, read_write> = access %2, 1u, 2u, 3u
    %4:u32 = load %3
    store %3, 42u
    ret
  }
)");
}

TEST_F(SpirvParserTest, Access_Nested_SeparateAccessInstructions) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
     %u32_42 = OpConstant %u32 42
  %arr_inner = OpTypeArray %u32 %u32_4
        %str = OpTypeStruct %arr_inner %arr_inner %arr_inner %arr_inner
  %arr_outer = OpTypeArray %str %u32_4
    %u32_ptr = OpTypePointer Function %u32
%arr_inner_ptr = OpTypePointer Function %arr_inner
    %str_ptr = OpTypePointer Function %str
%arr_outer_ptr = OpTypePointer Function %arr_outer
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %arr_outer_ptr Function
   %access_1 = OpAccessChain %str_ptr %var %u32_1
   %access_2 = OpAccessChain %arr_inner_ptr %access_1 %u32_2
   %access_3 = OpAccessChain %u32_ptr %access_2 %u32_3
       %load = OpLoad %u32 %access_3
               OpStore %access_3 %u32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
  tint_symbol:array<u32, 4> @offset(0)
  tint_symbol_1:array<u32, 4> @offset(16)
  tint_symbol_2:array<u32, 4> @offset(32)
  tint_symbol_3:array<u32, 4> @offset(48)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, array<tint_symbol_4, 4>, read_write> = var undef
    %3:ptr<function, tint_symbol_4, read_write> = access %2, 1u
    %4:ptr<function, array<u32, 4>, read_write> = access %3, 2u
    %5:ptr<function, u32, read_write> = access %4, 3u
    %6:u32 = load %5
    store %5, 42u
    ret
  }
)");
}

TEST_F(SpirvParserTest, Access_NoIndices) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %u32_ptr Function
   %access_1 = OpAccessChain %u32_ptr %var
   %access_2 = OpAccessChain %u32_ptr %access_1
       %load = OpLoad %u32 %access_2
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var undef
    %3:u32 = load %2
    ret
  }
)");
}

TEST_F(SpirvParserTest, Access_SignedIndices) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
      %i32_1 = OpConstant %i32 1
      %u32_2 = OpConstant %u32 2
      %i32_3 = OpConstant %i32 3
      %u32_4 = OpConstant %u32 4
     %u32_42 = OpConstant %u32 42
  %arr_inner = OpTypeArray %u32 %u32_4
        %str = OpTypeStruct %arr_inner %arr_inner %arr_inner %arr_inner
  %arr_outer = OpTypeArray %str %u32_4
    %u32_ptr = OpTypePointer Function %u32
%arr_outer_ptr = OpTypePointer Function %arr_outer
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %arr_outer_ptr Function
     %access = OpAccessChain %u32_ptr %var %i32_1 %u32_2 %i32_3
       %load = OpLoad %u32 %access
               OpStore %access %u32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
  tint_symbol:array<u32, 4> @offset(0)
  tint_symbol_1:array<u32, 4> @offset(16)
  tint_symbol_2:array<u32, 4> @offset(32)
  tint_symbol_3:array<u32, 4> @offset(48)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, array<tint_symbol_4, 4>, read_write> = var undef
    %3:ptr<function, u32, read_write> = access %2, 1i, 2u, 3i
    %4:u32 = load %3
    store %3, 42u
    ret
  }
)");
}

TEST_F(SpirvParserTest, InBoundsAccessChain) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
     %u32_42 = OpConstant %u32 42
  %arr_inner = OpTypeArray %u32 %u32_4
        %str = OpTypeStruct %arr_inner %arr_inner %arr_inner %arr_inner
  %arr_outer = OpTypeArray %str %u32_4
    %u32_ptr = OpTypePointer Function %u32
%arr_outer_ptr = OpTypePointer Function %arr_outer
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %arr_outer_ptr Function
     %access = OpInBoundsAccessChain %u32_ptr %var %u32_1 %u32_2 %u32_3
       %load = OpLoad %u32 %access
               OpStore %access %u32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
  tint_symbol:array<u32, 4> @offset(0)
  tint_symbol_1:array<u32, 4> @offset(16)
  tint_symbol_2:array<u32, 4> @offset(32)
  tint_symbol_3:array<u32, 4> @offset(48)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, array<tint_symbol_4, 4>, read_write> = var undef
    %3:ptr<function, u32, read_write> = access %2, 1u, 2u, 3u
    %4:u32 = load %3
    store %3, 42u
    ret
  }
)");
}

TEST_F(SpirvParserTest, StorageBufferAccessMode) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %ro_var NonWritable
               OpDecorate %ro_var DescriptorSet 1
               OpDecorate %ro_var Binding 2
               OpDecorate %rw_var DescriptorSet 1
               OpDecorate %rw_var Binding 3
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %str = OpTypeStruct %u32
    %u32_ptr = OpTypePointer StorageBuffer %u32
    %str_ptr = OpTypePointer StorageBuffer %str
    %ep_type = OpTypeFunction %void
      %u32_0 = OpConstant %u32 0
     %ro_var = OpVariable %str_ptr StorageBuffer
     %rw_var = OpVariable %str_ptr StorageBuffer
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
  %ro_access = OpAccessChain %u32_ptr %ro_var %u32_0
  %rw_access = OpAccessChain %u32_ptr %rw_var %u32_0
       %load = OpLoad %u32 %ro_access
               OpStore %rw_access %load
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read> = var undef @binding_point(1, 2)
  %2:ptr<storage, tint_symbol_1, read_write> = var undef @binding_point(1, 3)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<storage, u32, read> = access %1, 0u
    %5:ptr<storage, u32, read_write> = access %2, 0u
    %6:u32 = load %4
    store %5, %6
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
