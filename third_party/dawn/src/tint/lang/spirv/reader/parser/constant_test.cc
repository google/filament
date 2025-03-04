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

TEST_F(SpirvParserTest, Constant_Bool) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
       %null = OpConstantNull %bool
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %bool

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %bool
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %void %foo %true
          %2 = OpFunctionCall %void %foo %false
          %3 = OpFunctionCall %void %foo %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:void = call %2, true
    %6:void = call %2, false
    %7:void = call %2, false
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_I32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
      %i32_0 = OpConstant %i32 0
      %i32_1 = OpConstant %i32 1
     %i32_n1 = OpConstant %i32 -1
    %i32_max = OpConstant %i32 2147483647
    %i32_min = OpConstant %i32 -2147483648
   %i32_null = OpConstantNull %i32
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %i32

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %i32
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %void %foo %i32_0
          %2 = OpFunctionCall %void %foo %i32_1
          %3 = OpFunctionCall %void %foo %i32_n1
          %4 = OpFunctionCall %void %foo %i32_max
          %5 = OpFunctionCall %void %foo %i32_min
          %6 = OpFunctionCall %void %foo %i32_null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:void = call %2, 0i
    %6:void = call %2, 1i
    %7:void = call %2, -1i
    %8:void = call %2, 2147483647i
    %9:void = call %2, -2147483648i
    %10:void = call %2, 0i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_U32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_0 = OpConstant %u32 0
      %u32_1 = OpConstant %u32 1
    %u32_max = OpConstant %u32 4294967295
    %u32_null = OpConstantNull %u32
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %u32

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %u32
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %void %foo %u32_0
          %2 = OpFunctionCall %void %foo %u32_1
          %3 = OpFunctionCall %void %foo %u32_max
          %4 = OpFunctionCall %void %foo %u32_null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:void = call %2, 0u
    %6:void = call %2, 1u
    %7:void = call %2, 4294967295u
    %8:void = call %2, 0u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_F16) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %f16 = OpTypeFloat 16
      %f16_0 = OpConstant %f16 0
      %f16_1 = OpConstant %f16 1
    %f16_max = OpConstant %f16 0x1.ffcp+15
    %f16_min = OpConstant %f16 -0x1.ffcp+15
 %f16_denorm = OpConstant %f16 0x0.004p-14
   %f16_null = OpConstantNull %f16
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %f16

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %f16
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %void %foo %f16_0
          %2 = OpFunctionCall %void %foo %f16_1
          %3 = OpFunctionCall %void %foo %f16_max
          %4 = OpFunctionCall %void %foo %f16_min
          %5 = OpFunctionCall %void %foo %f16_denorm
          %6 = OpFunctionCall %void %foo %f16_null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:void = call %2, 0.0h
    %6:void = call %2, 1.0h
    %7:void = call %2, 65504.0h
    %8:void = call %2, -65504.0h
    %9:void = call %2, 0.00000005960464477539h
    %10:void = call %2, 0.0h
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_F32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %f32_0 = OpConstant %f32 0
      %f32_1 = OpConstant %f32 1
    %f32_max = OpConstant %f32 0x1.fffffep+127
    %f32_min = OpConstant %f32 -0x1.fffffep+127
 %f32_denorm = OpConstant %f32 0x0.000002p-126
    %f32_null = OpConstantNull %f32
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %f32

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %f32
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %void %foo %f32_0
          %2 = OpFunctionCall %void %foo %f32_1
          %3 = OpFunctionCall %void %foo %f32_max
          %4 = OpFunctionCall %void %foo %f32_min
          %5 = OpFunctionCall %void %foo %f32_denorm
          %6 = OpFunctionCall %void %foo %f32_null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:void = call %2, 0.0f
    %6:void = call %2, 1.0f
    %7:void = call %2, 340282346638528859811704183484516925440.0f
    %8:void = call %2, -340282346638528859811704183484516925440.0f
    %9:void = call %2, 1.40129846e-45f
    %10:void = call %2, 0.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_Vec2Bool) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
      %vec2b = OpTypeVector %bool 2
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
%vec2b_const = OpConstantComposite %vec2b %true %false
       %null = OpConstantNull %vec2b
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %vec2b %vec2b

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %vec2b None %fn_type
      %param = OpFunctionParameter %vec2b
  %foo_start = OpLabel
               OpReturnValue %param
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %vec2b %foo %vec2b_const
          %2 = OpFunctionCall %vec2b %foo %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:vec2<bool> = call %2, vec2<bool>(true, false)
    %6:vec2<bool> = call %2, vec2<bool>(false)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_Vec3I32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
      %vec3i = OpTypeVector %i32 3
      %i32_0 = OpConstant %i32 0
      %i32_1 = OpConstant %i32 1
     %i32_n1 = OpConstant %i32 -1
%vec3i_const = OpConstantComposite %vec3i %i32_0 %i32_1 %i32_n1
       %null = OpConstantNull %vec3i
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %vec3i %vec3i

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %vec3i None %fn_type
      %param = OpFunctionParameter %vec3i
  %foo_start = OpLabel
               OpReturnValue %param
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %vec3i %foo %vec3i_const
          %2 = OpFunctionCall %vec3i %foo %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:vec3<i32> = call %2, vec3<i32>(0i, 1i, -1i)
    %6:vec3<i32> = call %2, vec3<i32>(0i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_Vec4F32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
      %f32_0 = OpConstant %f32 0
      %f32_1 = OpConstant %f32 1
    %f32_max = OpConstant %f32 0x1.fffffep+127
    %f32_min = OpConstant %f32 -0x1.fffffep+127
%vec4f_const = OpConstantComposite %vec4f %f32_0 %f32_1 %f32_max %f32_min
       %null = OpConstantNull %vec4f
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %vec4f %vec4f

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %vec4f None %fn_type
      %param = OpFunctionParameter %vec4f
  %foo_start = OpLabel
               OpReturnValue %param
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %vec4f %foo %vec4f_const
          %2 = OpFunctionCall %vec4f %foo %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:vec4<f32> = call %2, vec4<f32>(0.0f, 1.0f, 340282346638528859811704183484516925440.0f, -340282346638528859811704183484516925440.0f)
    %6:vec4<f32> = call %2, vec4<f32>(0.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_Mat2x4F32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %mat2x4f = OpTypeMatrix %vec4f 2
      %f32_0 = OpConstant %f32 0
      %f32_1 = OpConstant %f32 1
%vec4f_const_0 = OpConstantComposite %vec4f %f32_0 %f32_0 %f32_0 %f32_0
%vec4f_const_1 = OpConstantComposite %vec4f %f32_1 %f32_1 %f32_1 %f32_1
%mat2x4f_const = OpConstantComposite %mat2x4f %vec4f_const_0 %vec4f_const_1
       %null = OpConstantNull %mat2x4f
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %mat2x4f %mat2x4f

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %mat2x4f None %fn_type
      %param = OpFunctionParameter %mat2x4f
  %foo_start = OpLabel
               OpReturnValue %param
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %mat2x4f %foo %mat2x4f_const
          %2 = OpFunctionCall %mat2x4f %foo %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:mat2x4<f32> = call %2, mat2x4<f32>(vec4<f32>(0.0f), vec4<f32>(1.0f))
    %6:mat2x4<f32> = call %2, mat2x4<f32>(vec4<f32>(0.0f))
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_Mat3x2F16) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %f16 = OpTypeFloat 16
      %vec2h = OpTypeVector %f16 2
    %mat3x2h = OpTypeMatrix %vec2h 3
      %f16_0 = OpConstant %f16 0
      %f16_1 = OpConstant %f16 1
    %f16_max = OpConstant %f16 0x1.ffcp+15
%vec2h_const_0 = OpConstantComposite %vec2h %f16_0 %f16_0
%vec2h_const_1 = OpConstantComposite %vec2h %f16_1 %f16_1
%vec2h_const_max = OpConstantComposite %vec2h %f16_max %f16_max
%mat3x2h_const = OpConstantComposite %mat3x2h %vec2h_const_0 %vec2h_const_1 %vec2h_const_max
       %null = OpConstantNull %mat3x2h
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %mat3x2h %mat3x2h

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %mat3x2h None %fn_type
      %param = OpFunctionParameter %mat3x2h
  %foo_start = OpLabel
               OpReturnValue %param
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %mat3x2h %foo %mat3x2h_const
          %2 = OpFunctionCall %mat3x2h %foo %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:mat3x2<f16> = call %2, mat3x2<f16>(vec2<f16>(0.0h), vec2<f16>(1.0h), vec2<f16>(65504.0h))
    %6:mat3x2<f16> = call %2, mat3x2<f16>(vec2<f16>(0.0h))
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_Array_I32_4) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
      %i32_4 = OpConstant %i32 4
        %arr = OpTypeArray %i32 %i32_4
      %i32_0 = OpConstant %i32 0
      %i32_1 = OpConstant %i32 1
     %i32_n1 = OpConstant %i32 -1
    %i32_max = OpConstant %i32 2147483647
  %arr_const = OpConstantComposite %arr %i32_0 %i32_1 %i32_n1 %i32_max
       %null = OpConstantNull %arr
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %arr %arr

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %arr None %fn_type
      %param = OpFunctionParameter %arr
  %foo_start = OpLabel
               OpReturnValue %param
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %arr %foo %arr_const
          %2 = OpFunctionCall %arr %foo %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:array<i32, 4> = call %2, array<i32, 4>(0i, 1i, -1i, 2147483647i)
    %6:array<i32, 4> = call %2, array<i32, 4>(0i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_Array_Array_F32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
      %i32_2 = OpConstant %i32 2
      %i32_4 = OpConstant %i32 4
        %f32 = OpTypeFloat 32
      %f32_0 = OpConstant %f32 0
      %f32_1 = OpConstant %f32 1
    %f32_max = OpConstant %f32 0x1.fffffep+127
    %f32_min = OpConstant %f32 -0x1.fffffep+127
      %inner = OpTypeArray %f32 %i32_4
      %outer = OpTypeArray %inner %i32_2
%inner_const_0 = OpConstantComposite %inner %f32_0 %f32_1 %f32_max %f32_min
%inner_const_1 = OpConstantComposite %inner %f32_min %f32_max %f32_1 %f32_0
  %outer_const = OpConstantComposite %outer %inner_const_0 %inner_const_1
       %null = OpConstantNull %outer
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %outer %outer

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %outer None %fn_type
      %param = OpFunctionParameter %outer
  %foo_start = OpLabel
               OpReturnValue %param
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %outer %foo %outer_const
          %2 = OpFunctionCall %outer %foo %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:array<array<f32, 4>, 2> = call %2, array<array<f32, 4>, 2>(array<f32, 4>(0.0f, 1.0f, 340282346638528859811704183484516925440.0f, -340282346638528859811704183484516925440.0f), array<f32, 4>(-340282346638528859811704183484516925440.0f, 340282346638528859811704183484516925440.0f, 1.0f, 0.0f))
    %6:array<array<f32, 4>, 2> = call %2, array<array<f32, 4>, 2>(array<f32, 4>(0.0f))
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %f32 = OpTypeFloat 32
        %str = OpTypeStruct %i32 %f32
     %i32_42 = OpConstant %i32 42
     %f32_n1 = OpConstant %f32 -1
  %str_const = OpConstantComposite %str %i32_42 %f32_n1
       %null = OpConstantNull %str
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %str %str

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %str None %fn_type
      %param = OpFunctionParameter %str
  %foo_start = OpLabel
               OpReturnValue %param
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %str %foo %str_const
          %2 = OpFunctionCall %str %foo %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:tint_symbol_2 = call %2, tint_symbol_2(42i, -1.0f)
    %6:tint_symbol_2 = call %2, tint_symbol_2(0i, 0.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_Struct_Nested) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %f32 = OpTypeFloat 32
      %i32_2 = OpConstant %i32 2
      %inner = OpTypeStruct %i32 %f32
        %arr = OpTypeArray %inner %i32_2
      %outer = OpTypeStruct %arr %arr
     %i32_42 = OpConstant %i32 42
     %i32_n1 = OpConstant %i32 -1
     %f32_n1 = OpConstant %f32 -1
     %f32_42 = OpConstant %f32 42
%inner_const_0 = OpConstantComposite %inner %i32_42 %f32_n1
%inner_const_1 = OpConstantComposite %inner %i32_n1 %f32_42
  %arr_const_0 = OpConstantComposite %arr %inner_const_0 %inner_const_1
  %arr_const_1 = OpConstantComposite %arr %inner_const_1 %inner_const_0
  %outer_const = OpConstantComposite %outer %arr_const_0 %arr_const_1
       %null = OpConstantNull %outer
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %outer %outer

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %outer None %fn_type
      %param = OpFunctionParameter %outer
  %foo_start = OpLabel
               OpReturnValue %param
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %outer %foo %outer_const
          %2 = OpFunctionCall %outer %foo %null
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void {
  $B3: {
    %5:tint_symbol_5 = call %2, tint_symbol_5(array<tint_symbol_2, 2>(tint_symbol_2(42i, -1.0f), tint_symbol_2(-1i, 42.0f)), array<tint_symbol_2, 2>(tint_symbol_2(-1i, 42.0f), tint_symbol_2(42i, -1.0f)))
    %6:tint_symbol_5 = call %2, tint_symbol_5(array<tint_symbol_2, 2>(tint_symbol_2(0i, 0.0f)))
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
