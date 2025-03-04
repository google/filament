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

}  // namespace
}  // namespace tint::spirv::reader
