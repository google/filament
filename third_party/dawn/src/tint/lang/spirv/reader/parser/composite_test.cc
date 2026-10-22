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

TEST_F(SpirvParserTest, Matrix_ColMajor) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %str 0 ColMajor
               OpMemberDecorate %str 0 Offset 0
               OpMemberDecorate %str 0 MatrixStride 48
               OpDecorate %str Block
               OpDecorate %var DescriptorSet 1
               OpDecorate %var Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec3f = OpTypeVector %f32 3
     %mat4x3 = OpTypeMatrix %vec3f 4
        %str = OpTypeStruct %mat4x3
        %ptr = OpTypePointer Uniform %str
    %ep_type = OpTypeFunction %void
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(16) {
  tint_symbol:mat4x3<f32> @offset(0) @size(192), @matrix_stride(48)
}

$B1: {  # root
  %1:ptr<uniform, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Matrix_RowMajor) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %str 0 RowMajor
               OpMemberDecorate %str 0 Offset 0
               OpMemberDecorate %str 0 MatrixStride 48
               OpDecorate %str Block
               OpDecorate %var DescriptorSet 1
               OpDecorate %var Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec3f = OpTypeVector %f32 3
     %mat4x3 = OpTypeMatrix %vec3f 4
        %str = OpTypeStruct %mat4x3
        %ptr = OpTypePointer Uniform %str
    %ep_type = OpTypeFunction %void
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(16) {
  tint_symbol:mat4x3<f32> @offset(0) @size(144), @row_major, @matrix_stride(48)
}

$B1: {  # root
  %1:ptr<uniform, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ArrayOfMatrix_ColMajor) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %str 0 ColMajor
               OpMemberDecorate %str 0 Offset 0
               OpMemberDecorate %str 0 MatrixStride 48
               OpDecorate %arr_mat4x3 ArrayStride 192
               OpDecorate %str Block
               OpDecorate %var DescriptorSet 1
               OpDecorate %var Binding 2
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
        %f32 = OpTypeFloat 32
      %vec3f = OpTypeVector %f32 3
     %mat4x3 = OpTypeMatrix %vec3f 4
 %arr_mat4x3 = OpTypeArray %mat4x3 %uint_4
        %str = OpTypeStruct %arr_mat4x3
        %ptr = OpTypePointer Uniform %str
    %ep_type = OpTypeFunction %void
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(16) {
  tint_symbol:spirv.explicit_layout_array<mat4x3<f32>, 4, stride=192> @offset(0), @matrix_stride(48)
}

$B1: {  # root
  %1:ptr<uniform, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ArrayOfMatrix_RowMajor) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %str 0 RowMajor
               OpMemberDecorate %str 0 Offset 0
               OpMemberDecorate %str 0 MatrixStride 48
               OpDecorate %arr_mat4x3 ArrayStride 144
               OpDecorate %str Block
               OpDecorate %var DescriptorSet 1
               OpDecorate %var Binding 2
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
        %f32 = OpTypeFloat 32
      %vec3f = OpTypeVector %f32 3
     %mat4x3 = OpTypeMatrix %vec3f 4
 %arr_mat4x3 = OpTypeArray %mat4x3 %uint_4
        %str = OpTypeStruct %arr_mat4x3
        %ptr = OpTypePointer Uniform %str
    %ep_type = OpTypeFunction %void
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(16) {
  tint_symbol:spirv.explicit_layout_array<mat4x3<f32>, 4, stride=144> @offset(0), @row_major, @matrix_stride(48)
}

$B1: {  # root
  %1:ptr<uniform, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, RuntimeArrayOfMatrix_ColMajor) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %str 0 ColMajor
               OpMemberDecorate %str 0 Offset 0
               OpMemberDecorate %str 0 MatrixStride 48
               OpDecorate %arr_mat4x3 ArrayStride 192
               OpDecorate %str BufferBlock
               OpDecorate %var DescriptorSet 1
               OpDecorate %var Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec3f = OpTypeVector %f32 3
     %mat4x3 = OpTypeMatrix %vec3f 4
 %arr_mat4x3 = OpTypeRuntimeArray %mat4x3
        %str = OpTypeStruct %arr_mat4x3
        %ptr = OpTypePointer Uniform %str
    %ep_type = OpTypeFunction %void
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(16) {
  tint_symbol:spirv.explicit_layout_array<mat4x3<f32>, stride=192> @offset(0), @matrix_stride(48)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read_write> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, RuntimeArrayOfMatrix_RowMajor) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %str 0 RowMajor
               OpMemberDecorate %str 0 Offset 0
               OpMemberDecorate %str 0 MatrixStride 48
               OpDecorate %arr_mat4x3 ArrayStride 144
               OpDecorate %str BufferBlock
               OpDecorate %var DescriptorSet 1
               OpDecorate %var Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec3f = OpTypeVector %f32 3
     %mat4x3 = OpTypeMatrix %vec3f 4
 %arr_mat4x3 = OpTypeRuntimeArray %mat4x3
        %str = OpTypeStruct %arr_mat4x3
        %ptr = OpTypePointer Uniform %str
    %ep_type = OpTypeFunction %void
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(16) {
  tint_symbol:spirv.explicit_layout_array<mat4x3<f32>, stride=144> @offset(0), @row_major, @matrix_stride(48)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read_write> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, StructMemberRelaxedPrecision) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %str 0 RelaxedPrecision
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %str Block
               OpDecorate %var DescriptorSet 1
               OpDecorate %var Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %str = OpTypeStruct %f32
        %ptr = OpTypePointer Uniform %str
    %ep_type = OpTypeFunction %void
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:f32 @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, StructMemberNonWritable) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %str 0 NonWritable
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %str Block
               OpDecorate %var DescriptorSet 1
               OpDecorate %var Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %str = OpTypeStruct %f32
        %ptr = OpTypePointer Uniform %str
    %ep_type = OpTypeFunction %void
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:f32 @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, StructMemberNonReadable) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %str 0 NonReadable
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %str Block
               OpDecorate %var DescriptorSet 1
               OpDecorate %var Binding 2
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %str = OpTypeStruct %f32
        %ptr = OpTypePointer Uniform %str
    %ep_type = OpTypeFunction %void
        %var = OpVariable %ptr Uniform
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:f32 @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeConstruct_Vector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %vec4u
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %vec4u None %fn_type
  %foo_start = OpLabel
        %vec = OpCompositeConstruct %vec4u %u32_1 %u32_2 %u32_3 %u32_4
               OpReturnValue %vec
               OpFunctionEnd
)",
              R"(
%2 = func():vec4<u32> {
  $B2: {
    %3:vec4<u32> = construct 1u, 2u, 3u, 4u
    ret %3
  }
)");
}

TEST_F(SpirvParserTest, CompositeConstruct_Matrix) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec3f = OpTypeVector %f32 3
     %mat4x3 = OpTypeMatrix %vec3f 4
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %mat4x3
      %f32_1 = OpConstant %f32 1.0
      %f32_2 = OpConstant %f32 2.0
      %f32_3 = OpConstant %f32 3.0
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %mat4x3 None %fn_type
  %foo_start = OpLabel
      %col_0 = OpCompositeConstruct %vec3f %f32_1 %f32_2 %f32_3
      %col_1 = OpCompositeConstruct %vec3f %f32_2 %f32_3 %f32_1
      %col_2 = OpCompositeConstruct %vec3f %f32_3 %f32_2 %f32_1
      %col_3 = OpCompositeConstruct %vec3f %f32_3 %f32_3 %f32_3
        %mat = OpCompositeConstruct %mat4x3 %col_0 %col_1 %col_2 %col_3
               OpReturnValue %mat
               OpFunctionEnd
)",
              R"(
%2 = func():mat4x3<f32> {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:vec3<f32> = construct 2.0f, 3.0f, 1.0f
    %5:vec3<f32> = construct 3.0f, 2.0f, 1.0f
    %6:vec3<f32> = construct 3.0f, 3.0f, 3.0f
    %7:mat4x3<f32> = construct %3, %4, %5, %6
    ret %7
  }
)");
}

TEST_F(SpirvParserTest, CompositeConstruct_Array) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %ep_type = OpTypeFunction %void
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
     %arr_ty = OpTypeArray %u32 %u32_4
    %fn_type = OpTypeFunction %arr_ty
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %arr_ty None %fn_type
  %foo_start = OpLabel
        %arr = OpCompositeConstruct %arr_ty %u32_1 %u32_2 %u32_3 %u32_4
               OpReturnValue %arr
               OpFunctionEnd
)",
              R"(
%2 = func():array<u32, 4> {
  $B2: {
    %3:array<u32, 4> = construct 1u, 2u, 3u, 4u
    ret %3
  }
)");
}

TEST_F(SpirvParserTest, CompositeConstruct_Array_ArrayStride_EqualsElementSize) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %arr_ty ArrayStride 4
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %ep_type = OpTypeFunction %void
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
     %arr_ty = OpTypeArray %u32 %u32_4
    %fn_type = OpTypeFunction %arr_ty
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %arr_ty None %fn_type
  %foo_start = OpLabel
        %arr = OpCompositeConstruct %arr_ty %u32_1 %u32_2 %u32_3 %u32_4
               OpReturnValue %arr
               OpFunctionEnd
)",
              R"(
%2 = func():array<u32, 4> {
  $B2: {
    %3:array<u32, 4> = construct 1u, 2u, 3u, 4u
    ret %3
  }
)");
}

TEST_F(SpirvParserTest, CompositeConstruct_Array_ArrayStride_EqualsElementSize_ArrayVec3) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %arr_ty ArrayStride 12
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %ep_type = OpTypeFunction %void
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
      %vec3u = OpTypeVector %u32 3
     %arr_ty = OpTypeArray %vec3u %u32_4
        %ptr = OpTypePointer Private %arr_ty
         %vs = OpVariable %ptr Private
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %2 = OpCopyObject %ptr %vs
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, spirv.explicit_layout_array<vec3<u32>, 4, stride=12>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, spirv.explicit_layout_array<vec3<u32>, 4, stride=12>, read_write> = let %1
    ret
  }
}
)");
}

TEST_F(SpirvParserTest,
       CompositeConstruct_Array_ArrayStride_EqualsElementSize_ArrayVec3_MatchTint) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %arr_ty ArrayStride 16
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %ep_type = OpTypeFunction %void
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
      %vec3u = OpTypeVector %u32 3
     %arr_ty = OpTypeArray %vec3u %u32_4
        %ptr = OpTypePointer Private %arr_ty
         %vs = OpVariable %ptr Private
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %2 = OpCopyObject %ptr %vs
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, array<vec3<u32>, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, array<vec3<u32>, 4>, read_write> = let %1
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeConstruct_Array_ArrayStride) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %arr_ty ArrayStride 16
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %ep_type = OpTypeFunction %void
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
     %arr_ty = OpTypeArray %u32 %u32_4
    %fn_type = OpTypeFunction %arr_ty
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %arr_ty None %fn_type
  %foo_start = OpLabel
        %arr = OpCompositeConstruct %arr_ty %u32_1 %u32_2 %u32_3 %u32_4
               OpReturnValue %arr
               OpFunctionEnd
)",
              R"(
%2 = func():spirv.explicit_layout_array<u32, 4, stride=16> {
  $B2: {
    %3:spirv.explicit_layout_array<u32, 4, stride=16> = construct 1u, 2u, 3u, 4u
    ret %3
  }
)");
}

TEST_F(SpirvParserTest, CompositeConstruct_ArrayOfVec) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
    %ep_type = OpTypeFunction %void
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
     %arr_ty = OpTypeArray %vec4u %u32_4
    %fn_type = OpTypeFunction %arr_ty
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

       %foo = OpFunction %arr_ty None %fn_type
 %foo_start = OpLabel
       %el_0 = OpCompositeConstruct %vec4u %u32_1 %u32_2 %u32_3 %u32_4
       %el_1 = OpCompositeConstruct %vec4u %u32_2 %u32_3 %u32_4 %u32_1
       %el_2 = OpCompositeConstruct %vec4u %u32_3 %u32_4 %u32_1 %u32_2
       %el_3 = OpCompositeConstruct %vec4u %u32_4 %u32_1 %u32_2 %u32_3
        %arr = OpCompositeConstruct %arr_ty %el_0 %el_1 %el_2 %el_3
               OpReturnValue %arr
               OpFunctionEnd
)",
              R"(
%2 = func():array<vec4<u32>, 4> {
  $B2: {
    %3:vec4<u32> = construct 1u, 2u, 3u, 4u
    %4:vec4<u32> = construct 2u, 3u, 4u, 1u
    %5:vec4<u32> = construct 3u, 4u, 1u, 2u
    %6:vec4<u32> = construct 4u, 1u, 2u, 3u
    %7:array<vec4<u32>, 4> = construct %3, %4, %5, %6
    ret %7
  }
)");
}

TEST_F(SpirvParserTest, CompositeConstruct_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
    %ep_type = OpTypeFunction %void
      %u32_1 = OpConstant %u32 1
      %u32_2 = OpConstant %u32 2
      %u32_3 = OpConstant %u32 3
      %u32_4 = OpConstant %u32 4
     %arr_ty = OpTypeArray %vec4u %u32_2
     %str_ty = OpTypeStruct %u32 %vec4u %arr_ty
    %fn_type = OpTypeFunction %str_ty
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %str_ty None %fn_type
  %foo_start = OpLabel
       %el_0 = OpCompositeConstruct %vec4u %u32_1 %u32_2 %u32_3 %u32_4
       %el_1 = OpCompositeConstruct %vec4u %u32_2 %u32_3 %u32_4 %u32_1
       %el_2 = OpCompositeConstruct %vec4u %u32_3 %u32_4 %u32_1 %u32_2
        %arr = OpCompositeConstruct %arr_ty %el_1 %el_2
        %str = OpCompositeConstruct %str_ty %u32_4 %el_0 %arr
               OpReturnValue %str
               OpFunctionEnd
)",
              R"(
tint_symbol_3 = struct @align(16) {
  tint_symbol:u32 @offset(0)
  tint_symbol_1:vec4<u32> @offset(16)
  tint_symbol_2:array<vec4<u32>, 2> @offset(32)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
%2 = func():tint_symbol_3 {
  $B2: {
    %3:vec4<u32> = construct 1u, 2u, 3u, 4u
    %4:vec4<u32> = construct 2u, 3u, 4u, 1u
    %5:vec4<u32> = construct 3u, 4u, 1u, 2u
    %6:array<vec4<u32>, 2> = construct %4, %5
    %7:tint_symbol_3 = construct 4u, %3, %6
    ret %7
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeExtract_Vector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %u32 %vec4u
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %u32 None %fn_type
        %vec = OpFunctionParameter %vec4u
  %foo_start = OpLabel
    %extract = OpCompositeExtract %u32 %vec 2
               OpReturnValue %extract
               OpFunctionEnd
)",
              R"(
%2 = func(%3:vec4<u32>):u32 {
  $B2: {
    %4:u32 = access %3, 2u
    ret %4
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeExtract_MatrixColumn) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %f32 = OpTypeFloat 32
      %vec3f = OpTypeVector %f32 3
     %mat4x3 = OpTypeMatrix %vec3f 4
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %vec3f %mat4x3
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %vec3f None %fn_type
        %mat = OpFunctionParameter %mat4x3
  %foo_start = OpLabel
    %extract = OpCompositeExtract %vec3f %mat 2
               OpReturnValue %extract
               OpFunctionEnd
)",
              R"(
%2 = func(%3:mat4x3<f32>):vec3<f32> {
  $B2: {
    %4:vec3<f32> = access %3, 2u
    ret %4
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeExtract_MatrixElement) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %f32 = OpTypeFloat 32
      %vec3f = OpTypeVector %f32 3
     %mat4x3 = OpTypeMatrix %vec3f 4
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %f32 %mat4x3
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %f32 None %fn_type
        %mat = OpFunctionParameter %mat4x3
  %foo_start = OpLabel
    %extract = OpCompositeExtract %f32 %mat 2 1
               OpReturnValue %extract
               OpFunctionEnd
)",
              R"(
%2 = func(%3:mat4x3<f32>):f32 {
  $B2: {
    %4:f32 = access %3, 2u, 1u
    ret %4
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeExtract_Array) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_4 = OpConstant %u32 4
     %arr_ty = OpTypeArray %u32 %u32_4
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %u32 %arr_ty
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %u32 None %fn_type
        %arr = OpFunctionParameter %arr_ty
  %foo_start = OpLabel
    %extract = OpCompositeExtract %u32 %arr 2
               OpReturnValue %extract
               OpFunctionEnd
)",
              R"(
%2 = func(%3:array<u32, 4>):u32 {
  $B2: {
    %4:u32 = access %3, 2u
    ret %4
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeExtract_ArrayOfVec) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec3u = OpTypeVector %u32 3
      %u32_4 = OpConstant %u32 4
     %arr_ty = OpTypeArray %vec3u %u32_4
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %u32 %arr_ty
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %u32 None %fn_type
        %arr = OpFunctionParameter %arr_ty
  %foo_start = OpLabel
    %extract = OpCompositeExtract %u32 %arr 1 2
               OpReturnValue %extract
               OpFunctionEnd
)",
              R"(
%2 = func(%3:array<vec3<u32>, 4>):u32 {
  $B2: {
    %4:u32 = access %3, 1u, 2u
    ret %4
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeExtract_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
     %str_ty = OpTypeStruct %u32 %u32
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %u32 %str_ty
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %u32 None %fn_type
        %str = OpFunctionParameter %str_ty
  %foo_start = OpLabel
    %extract = OpCompositeExtract %u32 %str 1
               OpReturnValue %extract
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:u32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
%2 = func(%3:tint_symbol_2):u32 {
  $B2: {
    %4:u32 = access %3, 1u
    ret %4
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeInsert_Vector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Function %u32
      %vec4u = OpTypeVector %u32 4
  %vec4u_ptr = OpTypePointer Function %vec4u
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_label = OpLabel
   %comp_var = OpVariable %vec4u_ptr Function
 %insert_var = OpVariable %u32_ptr Function
  %composite = OpLoad %vec4u %comp_var
     %insert = OpLoad %u32 %insert_var
     %result = OpCompositeInsert %vec4u %insert %composite 2
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:ptr<function, u32, read_write> = var undef
    %4:vec4<u32> = load %2
    %5:u32 = load %3
    %6:ptr<function, vec4<u32>, read_write> = var %4
    %7:ptr<function, u32, read_write> = access %6, 2u
    store %7, %5
    %8:vec4<u32> = load %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeInsert_MatrixColumn) {
    EXPECT_IR(R"(
              OpCapability Shader
              OpMemoryModel Logical GLSL450
              OpEntryPoint GLCompute %main "main"
              OpExecutionMode %main LocalSize 1 1 1
      %void = OpTypeVoid
       %f32 = OpTypeFloat 32
   %f32_ptr = OpTypePointer Function %f32
     %vec4f = OpTypeVector %f32 4
 %vec4f_ptr = OpTypePointer Function %vec4f
   %mat2x4f = OpTypeMatrix %vec4f 2
%mat2x4f_ptr = OpTypePointer Function %mat2x4f
   %ep_type = OpTypeFunction %void
      %main = OpFunction %void None %ep_type
%main_label = OpLabel
  %comp_var = OpVariable %mat2x4f_ptr Function
%insert_var = OpVariable %vec4f_ptr Function
 %composite = OpLoad %mat2x4f %comp_var
    %insert = OpLoad %vec4f %insert_var
    %result = OpCompositeInsert %mat2x4f %insert %composite 1
              OpReturn
              OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, mat2x4<f32>, read_write> = var undef
    %3:ptr<function, vec4<f32>, read_write> = var undef
    %4:mat2x4<f32> = load %2
    %5:vec4<f32> = load %3
    %6:ptr<function, mat2x4<f32>, read_write> = var %4
    %7:ptr<function, vec4<f32>, read_write> = access %6, 1u
    store %7, %5
    %8:mat2x4<f32> = load %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeInsert_MatrixElement) {
    EXPECT_IR(R"(
              OpCapability Shader
              OpMemoryModel Logical GLSL450
              OpEntryPoint GLCompute %main "main"
              OpExecutionMode %main LocalSize 1 1 1
      %void = OpTypeVoid
       %f32 = OpTypeFloat 32
   %f32_ptr = OpTypePointer Function %f32
     %vec4f = OpTypeVector %f32 4
 %vec4f_ptr = OpTypePointer Function %vec4f
   %mat2x4f = OpTypeMatrix %vec4f 2
%mat2x4f_ptr = OpTypePointer Function %mat2x4f
   %ep_type = OpTypeFunction %void
      %main = OpFunction %void None %ep_type
%main_label = OpLabel
  %comp_var = OpVariable %mat2x4f_ptr Function
%insert_var = OpVariable %f32_ptr Function
 %composite = OpLoad %mat2x4f %comp_var
    %insert = OpLoad %f32 %insert_var
    %result = OpCompositeInsert %mat2x4f %insert %composite 0 2
              OpReturn
              OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, mat2x4<f32>, read_write> = var undef
    %3:ptr<function, f32, read_write> = var undef
    %4:mat2x4<f32> = load %2
    %5:f32 = load %3
    %6:ptr<function, mat2x4<f32>, read_write> = var %4
    %7:ptr<function, f32, read_write> = access %6, 0u, 2u
    store %7, %5
    %8:mat2x4<f32> = load %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeInsert_Array) {
    EXPECT_IR(R"(
              OpCapability Shader
              OpMemoryModel Logical GLSL450
              OpEntryPoint GLCompute %main "main"
              OpExecutionMode %main LocalSize 1 1 1
      %void = OpTypeVoid
       %f32 = OpTypeFloat 32
      %uint = OpTypeInt 32 0
   %f32_ptr = OpTypePointer Function %f32
   %const_3 = OpConstant %uint 3
   %arr_f32 = OpTypeArray %f32 %const_3
%arr_f32_ptr = OpTypePointer Function %arr_f32
   %ep_type = OpTypeFunction %void
      %main = OpFunction %void None %ep_type
%main_label = OpLabel
  %comp_var = OpVariable %arr_f32_ptr Function
%insert_var = OpVariable %f32_ptr Function
 %composite = OpLoad %arr_f32 %comp_var
    %insert = OpLoad %f32 %insert_var
    %result = OpCompositeInsert %arr_f32 %insert %composite 2
              OpReturn
              OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, array<f32, 3>, read_write> = var undef
    %3:ptr<function, f32, read_write> = var undef
    %4:array<f32, 3> = load %2
    %5:f32 = load %3
    %6:ptr<function, array<f32, 3>, read_write> = var %4
    %7:ptr<function, f32, read_write> = access %6, 2u
    store %7, %5
    %8:array<f32, 3> = load %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeInsert_ArrayOfVec) {
    EXPECT_IR(R"(
              OpCapability Shader
              OpMemoryModel Logical GLSL450
              OpEntryPoint GLCompute %main "main"
              OpExecutionMode %main LocalSize 1 1 1
      %void = OpTypeVoid
       %f32 = OpTypeFloat 32
      %uint = OpTypeInt 32 0
   %f32_ptr = OpTypePointer Function %f32
     %vec3f = OpTypeVector %f32 3
 %vec3f_ptr = OpTypePointer Function %vec3f
   %const_4 = OpConstant %uint 4
 %arr_vec3f = OpTypeArray %vec3f %const_4
%arr_vec3f_ptr = OpTypePointer Function %arr_vec3f
   %ep_type = OpTypeFunction %void
      %main = OpFunction %void None %ep_type
%main_label = OpLabel
  %comp_var = OpVariable %arr_vec3f_ptr Function
%insert_var = OpVariable %vec3f_ptr Function
 %composite = OpLoad %arr_vec3f %comp_var
    %insert = OpLoad %vec3f %insert_var
    %result = OpCompositeInsert %arr_vec3f %insert %composite 1
              OpReturn
              OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, array<vec3<f32>, 4>, read_write> = var undef
    %3:ptr<function, vec3<f32>, read_write> = var undef
    %4:array<vec3<f32>, 4> = load %2
    %5:vec3<f32> = load %3
    %6:ptr<function, array<vec3<f32>, 4>, read_write> = var %4
    %7:ptr<function, vec3<f32>, read_write> = access %6, 1u
    store %7, %5
    %8:array<vec3<f32>, 4> = load %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, CompositeInsert_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
    %f32_ptr = OpTypePointer Function %f32
      %vec2f = OpTypeVector %f32 2
     %struct = OpTypeStruct %f32 %vec2f
%struct_ptr = OpTypePointer Function %struct
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_label = OpLabel
   %comp_var = OpVariable %struct_ptr Function
 %insert_var = OpVariable %f32_ptr Function
  %composite = OpLoad %struct %comp_var
     %insert = OpLoad %f32 %insert_var
     %result = OpCompositeInsert %struct %insert %composite 0
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(8) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:vec2<f32> @offset(8)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, tint_symbol_2, read_write> = var undef
    %3:ptr<function, f32, read_write> = var undef
    %4:tint_symbol_2 = load %2
    %5:f32 = load %3
    %6:ptr<function, tint_symbol_2, read_write> = var %4
    %7:ptr<function, f32, read_write> = access %6, 0u
    store %7, %5
    %8:tint_symbol_2 = load %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorInsertDynamic_VectorComponent) {
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
  %component = OpConstant %u32 1
      %index = OpConstant %u32 1
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %vec = OpVariable %vec4u_ptr Function
        %tmp = OpLoad %vec4u %vec
     %result = OpVectorInsertDynamic %vec4u %tmp %component %index
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:vec4<u32> = load %2
    %4:ptr<function, vec4<u32>, read_write> = var %3
    store_vector_element %4, 1u, 1u
    %5:vec4<u32> = load %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_AlternateVectors) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v4u32 = OpTypeVector %u32 4
  %v4u32_ptr = OpTypePointer Function %v4u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
       %vec1 = OpVariable %v4u32_ptr Function
       %vec2 = OpVariable %v4u32_ptr Function
       %tmp1 = OpLoad %v4u32 %vec1
       %tmp2 = OpLoad %v4u32 %vec2
       %shuf = OpVectorShuffle %v4u32 %tmp1 %tmp2 0 5 2 7
     %result = OpCopyObject %v4u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:ptr<function, vec4<u32>, read_write> = var undef
    %4:vec4<u32> = load %2
    %5:vec4<u32> = load %3
    %6:u32 = swizzle %4, x
    %7:u32 = swizzle %5, y
    %8:u32 = swizzle %4, z
    %9:u32 = swizzle %5, w
    %10:vec4<u32> = construct %6, %7, %8, %9
    %11:vec4<u32> = let %10
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_TwoFromA_Then_TwoFromB) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v4u32 = OpTypeVector %u32 4
  %v4u32_ptr = OpTypePointer Function %v4u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
       %vec1 = OpVariable %v4u32_ptr Function
       %vec2 = OpVariable %v4u32_ptr Function
       %tmp1 = OpLoad %v4u32 %vec1
       %tmp2 = OpLoad %v4u32 %vec2
       %shuf = OpVectorShuffle %v4u32 %tmp1 %tmp2 0 2 5 7
     %result = OpCopyObject %v4u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:ptr<function, vec4<u32>, read_write> = var undef
    %4:vec4<u32> = load %2
    %5:vec4<u32> = load %3
    %6:vec2<u32> = swizzle %4, xz
    %7:vec2<u32> = swizzle %5, yw
    %8:vec4<u32> = construct %6, %7
    %9:vec4<u32> = let %8
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_OneFromA_Then_TwoFromB_Then_OneFromA) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v4u32 = OpTypeVector %u32 4
  %v4u32_ptr = OpTypePointer Function %v4u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
       %vec1 = OpVariable %v4u32_ptr Function
       %vec2 = OpVariable %v4u32_ptr Function
       %tmp1 = OpLoad %v4u32 %vec1
       %tmp2 = OpLoad %v4u32 %vec2
       %shuf = OpVectorShuffle %v4u32 %tmp1 %tmp2 0 5 6 3
     %result = OpCopyObject %v4u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:ptr<function, vec4<u32>, read_write> = var undef
    %4:vec4<u32> = load %2
    %5:vec4<u32> = load %3
    %6:u32 = swizzle %4, x
    %7:vec2<u32> = swizzle %5, yz
    %8:u32 = swizzle %4, w
    %9:vec4<u32> = construct %6, %7, %8
    %10:vec4<u32> = let %9
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_BothVectorsToBigger) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v2u32 = OpTypeVector %u32 2
      %v3u32 = OpTypeVector %u32 3
      %v4u32 = OpTypeVector %u32 4
  %v2u32_ptr = OpTypePointer Function %v2u32
  %v3u32_ptr = OpTypePointer Function %v3u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
       %vec1 = OpVariable %v2u32_ptr Function
       %vec2 = OpVariable %v3u32_ptr Function
       %tmp1 = OpLoad %v2u32 %vec1
       %tmp2 = OpLoad %v3u32 %vec2
       %shuf = OpVectorShuffle %v4u32 %tmp1 %tmp2 0 2 1 4
     %result = OpCopyObject %v4u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<u32>, read_write> = var undef
    %3:ptr<function, vec3<u32>, read_write> = var undef
    %4:vec2<u32> = load %2
    %5:vec3<u32> = load %3
    %6:u32 = swizzle %4, x
    %7:u32 = swizzle %5, x
    %8:u32 = swizzle %4, y
    %9:u32 = swizzle %5, z
    %10:vec4<u32> = construct %6, %7, %8, %9
    %11:vec4<u32> = let %10
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_BothVectorsToSmaller) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v2u32 = OpTypeVector %u32 2
      %v3u32 = OpTypeVector %u32 3
      %v4u32 = OpTypeVector %u32 4
  %v3u32_ptr = OpTypePointer Function %v3u32
  %v4u32_ptr = OpTypePointer Function %v4u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
       %vec1 = OpVariable %v3u32_ptr Function
       %vec2 = OpVariable %v4u32_ptr Function
       %tmp1 = OpLoad %v3u32 %vec1
       %tmp2 = OpLoad %v4u32 %vec2
       %shuf = OpVectorShuffle %v2u32 %tmp1 %tmp2 0 4
     %result = OpCopyObject %v2u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec3<u32>, read_write> = var undef
    %3:ptr<function, vec4<u32>, read_write> = var undef
    %4:vec3<u32> = load %2
    %5:vec4<u32> = load %3
    %6:u32 = swizzle %4, x
    %7:u32 = swizzle %5, y
    %8:vec2<u32> = construct %6, %7
    %9:vec2<u32> = let %8
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_BothVectors_UndefinedIndex) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v4u32 = OpTypeVector %u32 4
  %v4u32_ptr = OpTypePointer Function %v4u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
       %vecA = OpVariable %v4u32_ptr Function
       %vecB = OpVariable %v4u32_ptr Function
       %tmpA = OpLoad %v4u32 %vecA
       %tmpB = OpLoad %v4u32 %vecB
       %shuf = OpVectorShuffle %v4u32 %tmpA %tmpB 0 4294967295 6 3
     %result = OpCopyObject %v4u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:ptr<function, vec4<u32>, read_write> = var undef
    %4:vec4<u32> = load %2
    %5:vec4<u32> = load %3
    %6:vec2<u32> = swizzle %4, xx
    %7:u32 = swizzle %5, z
    %8:u32 = swizzle %4, w
    %9:vec4<u32> = construct %6, %7, %8
    %10:vec4<u32> = let %9
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_MixedDimensions_234) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v2u32 = OpTypeVector %u32 2
      %v3u32 = OpTypeVector %u32 3
      %v4u32 = OpTypeVector %u32 4
  %v2u32_ptr = OpTypePointer Function %v2u32
  %v3u32_ptr = OpTypePointer Function %v3u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
       %vec1 = OpVariable %v2u32_ptr Function
       %vec2 = OpVariable %v3u32_ptr Function
       %tmp1 = OpLoad %v2u32 %vec1
       %tmp2 = OpLoad %v3u32 %vec2
       %shuf = OpVectorShuffle %v4u32 %tmp1 %tmp2 0 3 4 1
     %result = OpCopyObject %v4u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<u32>, read_write> = var undef
    %3:ptr<function, vec3<u32>, read_write> = var undef
    %4:vec2<u32> = load %2
    %5:vec3<u32> = load %3
    %6:u32 = swizzle %4, x
    %7:vec2<u32> = swizzle %5, yz
    %8:u32 = swizzle %4, y
    %9:vec4<u32> = construct %6, %7, %8
    %10:vec4<u32> = let %9
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_AllFromA) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v4u32 = OpTypeVector %u32 4
  %v4u32_ptr = OpTypePointer Function %v4u32
      %v2u32 = OpTypeVector %u32 2
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %vec = OpVariable %v4u32_ptr Function
        %tmp = OpLoad %v4u32 %vec
       %shuf = OpVectorShuffle %v2u32 %tmp %tmp 0 2
     %result = OpCopyObject %v2u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:vec4<u32> = load %2
    %4:vec2<u32> = swizzle %3, xz
    %5:vec2<u32> = let %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_AllFromB) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v4u32 = OpTypeVector %u32 4
  %v4u32_ptr = OpTypePointer Function %v4u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
       %vec1 = OpVariable %v4u32_ptr Function
       %vec2 = OpVariable %v4u32_ptr Function
       %tmp1 = OpLoad %v4u32 %vec1
       %tmp2 = OpLoad %v4u32 %vec2
       %shuf = OpVectorShuffle %v4u32 %tmp1 %tmp2 4 5 6 7
     %result = OpCopyObject %v4u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:ptr<function, vec4<u32>, read_write> = var undef
    %4:vec4<u32> = load %2
    %5:vec4<u32> = load %3
    %6:vec4<u32> = swizzle %5, xyzw
    %7:vec4<u32> = let %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_Swizzle_SmallerResult) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v4u32 = OpTypeVector %u32 4
      %v2u32 = OpTypeVector %u32 2
  %v4u32_ptr = OpTypePointer Function %v4u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
       %vec  = OpVariable %v4u32_ptr Function
       %tmp  = OpLoad %v4u32 %vec
       %shuf = OpVectorShuffle %v2u32 %tmp %tmp 2 0
     %result = OpCopyObject %v2u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:vec4<u32> = load %2
    %4:vec2<u32> = swizzle %3, zx
    %5:vec2<u32> = let %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_Swizzle_BiggerResult) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v2u32 = OpTypeVector %u32 2
      %v4u32 = OpTypeVector %u32 4
  %v2u32_ptr = OpTypePointer Function %v2u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
       %vec  = OpVariable %v2u32_ptr Function
       %tmp  = OpLoad %v2u32 %vec
       %shuf = OpVectorShuffle %v4u32 %tmp %tmp 3 2 3 2
     %result = OpCopyObject %v4u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<u32>, read_write> = var undef
    %3:vec2<u32> = load %2
    %4:vec4<u32> = swizzle %3, yxyx
    %5:vec4<u32> = let %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VectorShuffle_Swizzle_OneVectorUndef) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %v2u32 = OpTypeVector %u32 2
      %v4u32 = OpTypeVector %u32 4
  %v2u32_ptr = OpTypePointer Function %v2u32
    %ep_type = OpTypeFunction %void

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %vec = OpVariable %v2u32_ptr Function
        %tmp = OpLoad %v2u32 %vec
     %undef4 = OpUndef %v4u32
       %shuf = OpVectorShuffle %v2u32 %undef4 %tmp 4 5
     %result = OpCopyObject %v2u32 %shuf
               OpReturn
               OpFunctionEnd
    )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<u32>, read_write> = var undef
    %3:vec2<u32> = load %2
    %4:vec2<u32> = swizzle %3, xy
    %5:vec2<u32> = let %4
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
