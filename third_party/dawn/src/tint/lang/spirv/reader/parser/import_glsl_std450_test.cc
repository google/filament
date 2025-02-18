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

std::string Preamble() {
    return R"(
  OpCapability Shader
  %glsl = OpExtInstImport "GLSL.std.450"
  OpMemoryModel Logical GLSL450
  OpEntryPoint GLCompute %100 "main"
  OpExecutionMode %100 LocalSize 1 1 1

  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %v2int = OpTypeVector %int 2
  %v2uint = OpTypeVector %uint 2
  %v2float = OpTypeVector %float 2
  %v3float = OpTypeVector %float 3
  %v4float = OpTypeVector %float 4
  %mat2v2float = OpTypeMatrix %v2float 2
  %mat3v3float = OpTypeMatrix %v3float 3
  %mat4v4float = OpTypeMatrix %v4float 4

  %ptr_function_float = OpTypePointer Function %float
  %ptr_function_v2float = OpTypePointer Function %v2float

  %ptr_function_int = OpTypePointer Function %int
  %ptr_function_v2int = OpTypePointer Function %v2int
  %ptr_function_uint = OpTypePointer Function %uint
  %ptr_function_v2uint = OpTypePointer Function %v2uint

  %int_10 = OpConstant %int 10
  %int_20 = OpConstant %int 20

  %uint_10 = OpConstant %uint 10
  %uint_20 = OpConstant %uint 20

  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %float_70 = OpConstant %float 70

  %v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20

  %v2int_20_10 = OpConstantComposite %v2int %int_20 %int_10
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10

  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
  %v2float_70_60 = OpConstantComposite %v2float %float_70 %float_60
  %v3float_50_60_70 = OpConstantComposite %v3float %float_50 %float_60 %float_70
  %v4float_50_50_50_50 = OpConstantComposite %v4float %float_50 %float_50 %float_50 %float_50

  %mat2v2float_50_60 = OpConstantComposite %mat2v2float %v2float_50_60 %v2float_50_60
  %mat3v3float_50_60_70 = OpConstantComposite %mat3v3float %v3float_50_60_70 %v3float_50_60_70 %v3float_50_60_70
  %mat4v4float_50_50_50_50 = OpConstantComposite %mat4v4float %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50

  %100 = OpFunction %void None %voidfn
  %entry = OpLabel
)";
}

TEST_F(SpirvParserTest, GlslStd450_MatrixInverse_mat2x2) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %mat2v2float %glsl MatrixInverse %mat2v2float_50_60
     %2 = OpCopyObject %mat2v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f32> = spirv.inverse mat2x2<f32>(vec2<f32>(50.0f, 60.0f))
    %3:mat2x2<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_MatrixInverse_mat3x3) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %mat3v3float %glsl MatrixInverse %mat3v3float_50_60_70
     %2 = OpCopyObject %mat3v3float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x3<f32> = spirv.inverse mat3x3<f32>(vec3<f32>(50.0f, 60.0f, 70.0f))
    %3:mat3x3<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_MatrixInverse_mat4x4) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %mat4v4float %glsl MatrixInverse %mat4v4float_50_50_50_50
     %2 = OpCopyObject %mat4v4float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat4x4<f32> = spirv.inverse mat4x4<f32>(vec4<f32>(50.0f))
    %3:mat4x4<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, GlslStd450_MatrixInverse_MultipleInScope) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %mat2v2float %glsl MatrixInverse %mat2v2float_50_60
     %2 = OpExtInst %mat2v2float %glsl MatrixInverse %mat2v2float_50_60
     %3 = OpCopyObject %mat2v2float %1
     %4 = OpCopyObject %mat2v2float %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f32> = spirv.inverse mat2x2<f32>(vec2<f32>(50.0f, 60.0f))
    %3:mat2x2<f32> = spirv.inverse mat2x2<f32>(vec2<f32>(50.0f, 60.0f))
    %4:mat2x2<f32> = let %2
    %5:mat2x2<f32> = let %3
    ret
  }
}
)");
}

struct GlslStd450Params {
    std::string spv_name;
    std::string ir_name;
};
[[maybe_unused]] inline std::ostream& operator<<(std::ostream& out, GlslStd450Params c) {
    out << c.spv_name;
    return out;
}

using GlslStd450OneParamTest = SpirvParserTestWithParam<GlslStd450Params>;

TEST_P(GlslStd450OneParamTest, UnsignedToUnsigned) {
    auto params = GetParam();
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  params.spv_name + R"( %uint_10
     %2 = OpExtInst %v2uint %glsl )" +
                  params.spv_name + R"( %v2uint_10_20
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 10u
    %3:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<u32>(10u, 20u)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450OneParamTest, UnsignedToSigned) {
    auto params = GetParam();
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  params.spv_name + R"( %uint_10
     %2 = OpExtInst %v2int %glsl )" +
                  params.spv_name + R"( %v2uint_10_20
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 10u
    %3:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<u32>(10u, 20u)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450OneParamTest, SignedToUnsigned) {
    auto params = GetParam();
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  params.spv_name + R"( %int_10
     %2 = OpExtInst %v2uint %glsl )" +
                  params.spv_name + R"( %v2int_10_20
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 10i
    %3:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<i32>(10i, 20i)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450OneParamTest, SignedToSigned) {
    auto params = GetParam();
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  params.spv_name + R"( %int_10
     %2 = OpExtInst %v2int %glsl )" +
                  params.spv_name + R"( %v2int_10_20
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 10i
    %3:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<i32>(10i, 20i)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParser,
                         GlslStd450OneParamTest,
                         ::testing::Values(GlslStd450Params{"SAbs", "abs"},
                                           GlslStd450Params{"FindSMsb", "findSMsb"},
                                           GlslStd450Params{"FindUMsb", "findUMsb"}));

using GlslStd450TwoParamTest = SpirvParserTestWithParam<GlslStd450Params>;

TEST_P(GlslStd450TwoParamTest, UnsignedToUnsigned) {
    auto params = GetParam();

    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  params.spv_name + R"( %uint_10 %uint_10
     %2 = OpExtInst %v2uint %glsl )" +
                  params.spv_name + R"( %v2uint_10_20 %v2uint_20_10
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 10u, 10u
    %3:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<u32>(10u, 20u), vec2<u32>(20u, 10u)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450TwoParamTest, SignedToSigned) {
    auto params = GetParam();

    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  params.spv_name + R"( %int_10 %int_10
     %2 = OpExtInst %v2int %glsl )" +
                  params.spv_name + R"( %v2int_10_20 %v2int_20_10
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 10i, 10i
    %3:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<i32>(10i, 20i), vec2<i32>(20i, 10i)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450TwoParamTest, MixedToUnsigned) {
    auto params = GetParam();

    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  params.spv_name + R"( %int_10 %uint_10
     %2 = OpExtInst %v2uint %glsl )" +
                  params.spv_name + R"( %v2int_10_20 %v2uint_20_10
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 10i, 10u
    %3:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<i32>(10i, 20i), vec2<u32>(20u, 10u)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450TwoParamTest, MixedToSigned) {
    auto params = GetParam();

    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  params.spv_name + R"( %uint_10 %int_10
     %2 = OpExtInst %v2int %glsl )" +
                  params.spv_name + R"( %v2uint_10_20 %v2int_20_10
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 10u, 10i
    %3:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<u32>(10u, 20u), vec2<i32>(20i, 10i)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParser,
                         GlslStd450TwoParamTest,
                         ::testing::Values(GlslStd450Params{"SMax", "smax"},
                                           GlslStd450Params{"SMin", "smin"},
                                           GlslStd450Params{"UMax", "umax"},
                                           GlslStd450Params{"UMin", "umin"}));

using GlslStd450ThreeParamTest = SpirvParserTestWithParam<GlslStd450Params>;

TEST_P(GlslStd450ThreeParamTest, UnsignedToUnsigned) {
    auto params = GetParam();
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  params.spv_name + R"( %uint_10 %uint_10 %uint_10
     %2 = OpExtInst %v2uint %glsl )" +
                  params.spv_name + R"( %v2uint_10_20 %v2uint_20_10 %v2uint_20_10
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 10u, 10u, 10u
    %3:vec2<u32> = spirv.)" +
                  params.ir_name +
                  R"(<u32> vec2<u32>(10u, 20u), vec2<u32>(20u, 10u), vec2<u32>(20u, 10u)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450ThreeParamTest, SignedToSigned) {
    auto params = GetParam();
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  params.spv_name + R"( %int_10 %int_10 %int_10
     %2 = OpExtInst %v2int %glsl )" +
                  params.spv_name + R"( %v2int_10_20 %v2int_20_10 %v2int_20_10
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 10i, 10i, 10i
    %3:vec2<i32> = spirv.)" +
                  params.ir_name +
                  R"(<i32> vec2<i32>(10i, 20i), vec2<i32>(20i, 10i), vec2<i32>(20i, 10i)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450ThreeParamTest, MixedToUnsigned) {
    auto params = GetParam();
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  params.spv_name + R"( %int_10 %uint_10 %int_10
     %2 = OpExtInst %v2uint %glsl )" +
                  params.spv_name + R"( %v2int_10_20 %v2uint_20_10 %v2int_10_20
     %3 = OpCopyObject %uint %1
     %4 = OpCopyObject %v2uint %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 10i, 10u, 10i
    %3:vec2<u32> = spirv.)" +
                  params.ir_name +
                  R"(<u32> vec2<i32>(10i, 20i), vec2<u32>(20u, 10u), vec2<i32>(10i, 20i)
    %4:u32 = let %2
    %5:vec2<u32> = let %3
    ret
  }
}
)");
}

TEST_P(GlslStd450ThreeParamTest, MixedToSigned) {
    auto params = GetParam();
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  params.spv_name + R"( %uint_10 %int_10 %uint_10
     %2 = OpExtInst %v2int %glsl )" +
                  params.spv_name + R"( %v2uint_10_20 %v2int_20_10 %v2uint_10_20
     %3 = OpCopyObject %int %1
     %4 = OpCopyObject %v2int %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 10u, 10i, 10u
    %3:vec2<i32> = spirv.)" +
                  params.ir_name +
                  R"(<i32> vec2<u32>(10u, 20u), vec2<i32>(20i, 10i), vec2<u32>(10u, 20u)
    %4:i32 = let %2
    %5:vec2<i32> = let %3
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParser,
                         GlslStd450ThreeParamTest,
                         ::testing::Values(GlslStd450Params{"SClamp", "sclamp"},
                                           GlslStd450Params{"UClamp", "uclamp"}));

TEST_F(SpirvParserTest, FindILsb) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl FindILsb %int_10
     %2 = OpExtInst %v2uint %glsl FindILsb %v2int_10_20
     %3 = OpExtInst %int %glsl FindILsb %uint_10
     %4 = OpExtInst %v2int %glsl FindILsb %v2uint_10_20
     %5 = OpExtInst %v2uint %glsl FindILsb %v2uint_10_20
     %6 = OpExtInst %v2int %glsl FindILsb %v2int_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.findILsb<u32> 10i
    %3:vec2<u32> = spirv.findILsb<u32> vec2<i32>(10i, 20i)
    %4:i32 = spirv.findILsb<i32> 10u
    %5:vec2<i32> = spirv.findILsb<i32> vec2<u32>(10u, 20u)
    %6:vec2<u32> = spirv.findILsb<u32> vec2<u32>(10u, 20u)
    %7:vec2<i32> = spirv.findILsb<i32> vec2<i32>(10i, 20i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Refract_Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl Refract %float_50 %float_60 %float_70
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.refract 50.0f, 60.0f, 70.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Refract_Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl Refract %v2float_50_60 %v2float_60_50 %float_70
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.refract vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f), 70.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FaceForward_Scalar) {
    EXPECT_IR(Preamble() + R"(
     %99 = OpFAdd %float %float_50 %float_50 ; normal operand has only one use
     %1 = OpExtInst %float %glsl FaceForward %99 %float_60 %float_70
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = add 50.0f, 50.0f
    %3:f32 = spirv.faceForward %2, 60.0f, 70.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FaceForward_Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl FaceForward %v2float_50_60 %v2float_60_50 %v2float_70_60
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.faceForward vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f), vec2<f32>(70.0f, 60.0f)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Reflect_Scalar) {
    EXPECT_IR(Preamble() + R"(
     %98 = OpFAdd %float %float_50 %float_50 ; has only one use
     %99 = OpFAdd %float %float_60 %float_60 ; has only one use
     %1 = OpExtInst %float %glsl Reflect %98 %99
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = add 50.0f, 50.0f
    %3:f32 = add 60.0f, 60.0f
    %4:f32 = spirv.reflect %2, %3
    %5:f32 = let %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Reflect_Vector) {
    EXPECT_IR(Preamble() + R"(
     %98 = OpFAdd %v2float %v2float_50_60 %v2float_50_60
     %99 = OpFAdd %v2float %v2float_60_50 %v2float_60_50
     %1 = OpExtInst %v2float %glsl Reflect %98 %99
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = add vec2<f32>(50.0f, 60.0f), vec2<f32>(50.0f, 60.0f)
    %3:vec2<f32> = add vec2<f32>(60.0f, 50.0f), vec2<f32>(60.0f, 50.0f)
    %4:vec2<f32> = spirv.reflect %2, %3
    %5:vec2<f32> = let %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Ldexp_ScalarUnsigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl Ldexp %float_50 %uint_10
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.ldexp 50.0f, 10u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Ldexp_VectorUnsigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl Ldexp %v2float_50_60 %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.ldexp vec2<f32>(50.0f, 60.0f), vec2<u32>(10u, 20u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Modf_Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpVariable %ptr_function_float Function
     %2 = OpExtInst %float %glsl Modf %float_50 %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var
    %3:f32 = spirv.modf 50.0f, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Modf_Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpVariable %ptr_function_v2float Function
     %2 = OpExtInst %v2float %glsl Modf %v2float_50_60 %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<f32>, read_write> = var
    %3:vec2<f32> = spirv.modf vec2<f32>(50.0f, 60.0f), %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Frexp_ScalarSigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpVariable %ptr_function_int Function
     %2 = OpExtInst %float %glsl Frexp %float_50 %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    %3:f32 = spirv.frexp 50.0f, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Frexp_ScalarUnSigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpVariable %ptr_function_uint Function
     %2 = OpExtInst %float %glsl Frexp %float_50 %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var
    %3:f32 = spirv.frexp 50.0f, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Frexp_VectorUnSigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpVariable %ptr_function_v2uint Function
     %2 = OpExtInst %v2float %glsl Frexp %v2float_50_60 %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<u32>, read_write> = var
    %3:vec2<f32> = spirv.frexp vec2<f32>(50.0f, 60.0f), %2
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
