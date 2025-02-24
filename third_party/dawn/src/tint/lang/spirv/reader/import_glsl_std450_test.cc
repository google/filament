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

#include "src/tint/lang/spirv/reader/helper_test.h"

namespace tint::spirv::reader {
namespace {

std::string Preamble() {
    return R"(
  OpCapability Shader
  OpCapability Float16
  %glsl = OpExtInstImport "GLSL.std.450"
  OpMemoryModel Logical GLSL450
  OpEntryPoint GLCompute %100 "main"
  OpExecutionMode %100 LocalSize 1 1 1

  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32
  %half = OpTypeFloat 16

  %uint_10 = OpConstant %uint 10
  %uint_15 = OpConstant %uint 15
  %uint_20 = OpConstant %uint 20
  %int_30 = OpConstant %int 30
  %int_35 = OpConstant %int 35
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %float_70 = OpConstant %float 70
  %half_50 = OpConstant %half 50
  %half_60 = OpConstant %half 60
  %half_70 = OpConstant %half 70

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2
  %v3float = OpTypeVector %float 3
  %v4float = OpTypeVector %float 4
  %v2half = OpTypeVector %half 2
  %v3half = OpTypeVector %half 3
  %v4half = OpTypeVector %half 4
  %mat2v2float = OpTypeMatrix %v2float 2
  %mat3v3float = OpTypeMatrix %v3float 3
  %mat4v4float = OpTypeMatrix %v4float 4
  %mat2v2half = OpTypeMatrix %v2half 2
  %mat3v3half = OpTypeMatrix %v3half 3
  %mat4v4half = OpTypeMatrix %v4half 4

  %modf_result_type = OpTypeStruct %float %float
  %modf_v2_result_type = OpTypeStruct %v2float %v2float
  %ptr_function_modf_result_type = OpTypePointer Function %modf_result_type

  %frexp_result_type_unsigned = OpTypeStruct %float %uint
  %frexp_result_type_signed = OpTypeStruct %float %int
  %frexp_v2_result_type_unsigned = OpTypeStruct %v2float %v2uint
  %frexp_v2_result_type_signed = OpTypeStruct %v2float %v2int
  %ptr_function_frexp_result_type_unsigned = OpTypePointer Function %frexp_result_type_unsigned

  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2uint_15_15 = OpConstantComposite %v2uint %uint_15 %uint_15
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2int_35_35 = OpConstantComposite %v2int %int_35 %int_35
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
  %v2float_70_70 = OpConstantComposite %v2float %float_70 %float_70
  %v2half_50_60 = OpConstantComposite %v2half %half_50 %half_60

  %v3float_50_60_70 = OpConstantComposite %v3float %float_50 %float_60 %float_70
  %v3float_60_70_50 = OpConstantComposite %v3float %float_60 %float_70 %float_50
  %v3half_50_60_70 = OpConstantComposite %v3half %half_50 %half_60 %half_70

  %v4float_50_50_50_50 = OpConstantComposite %v4float %float_50 %float_50 %float_50 %float_50
  %v4half_50_50_50_50 = OpConstantComposite %v4half %half_50 %half_50 %half_50 %half_50

  %mat2v2float_50_60 = OpConstantComposite %mat2v2float %v2float_50_60 %v2float_50_60
  %mat3v3float_50_60_70 = OpConstantComposite %mat3v3float %v3float_50_60_70 %v3float_50_60_70 %v3float_50_60_70
  %mat4v4float_50_50_50_50 = OpConstantComposite %mat4v4float %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50

  %mat2v2half_50_60 = OpConstantComposite %mat2v2half %v2half_50_60 %v2half_50_60
  %mat3v3half_50_60_70 = OpConstantComposite %mat3v3half %v3half_50_60_70 %v3half_50_60_70 %v3half_50_60_70
  %mat4v4half_50_50_50_50 = OpConstantComposite %mat4v4half %v4half_50_50_50_50 %v4half_50_50_50_50 %v4half_50_50_50_50 %v4half_50_50_50_50

  %100 = OpFunction %void None %voidfn
  %entry = OpLabel
)";
}

struct GlslStd450Case {
    std::string opcode;
    std::string wgsl_func;
};
inline std::ostream& operator<<(std::ostream& out, GlslStd450Case c) {
    out << "GlslStd450Case(" << c.opcode << " " << c.wgsl_func << ")";
    return out;
}

// Nomenclature:
// Float = scalar float
// Floating = scalar float or vector-of-float
// Float3 = 3-element vector of float
// Int = scalar signed int
// Inting = scalar int or vector-of-int
// Uint = scalar unsigned int
// Uinting = scalar unsigned or vector-of-unsigned

using SpirvReaderTest_GlslStd450_Float_Floating = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Float_FloatingFloating = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Floating_Floating = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Floating_FloatingFloating =
    SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Floating_FloatingFloatingFloating =
    SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Floating_FloatingInting = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Float3_Float3Float3 = SpirvReaderTestWithParam<GlslStd450Case>;

using SpirvReaderTest_GlslStd450_Inting_Inting = SpirvReaderTestWithParam<GlslStd450Case>;
using SpirvReaderTest_GlslStd450_Uinting_Uinting = SpirvReaderTestWithParam<GlslStd450Case>;

TEST_P(SpirvReaderTest_GlslStd450_Float_Floating, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",

              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float_Floating, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %v2float_50_60
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f)
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float_FloatingFloating, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50 %float_60
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f, 60.0f
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float_FloatingFloating, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %v2float_50_60 %v2float_60_50
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f)
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_Floating, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f
    %3:f32 = let %2
    ret
  }
})");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_Floating, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode + R"( %v2float_50_60
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloating, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50 %float_60
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f, 60.0f
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloating, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode + R"( %v2float_50_60 %v2float_60_50
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloatingFloating, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50 %float_60 %float_70
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f, 60.0f, 70.0f
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingFloatingFloating, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode +
                  R"( %v2float_50_60 %v2float_60_50 %v2float_70_70
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f), vec2<f32>(70.0f)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingInting, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                  GetParam().opcode + R"( %float_50 %int_30
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = )" + GetParam().wgsl_func +
                  R"( 50.0f, 30i
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Floating_FloatingInting, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                  GetParam().opcode +
                  R"( %v2float_50_60 %v2int_30_40
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<f32>(50.0f, 60.0f), vec2<i32>(30i, 40i)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Float3_Float3Float3, SpirvParser) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v3float %glsl )" +
                  GetParam().opcode +
                  R"( %v3float_50_60_70 %v3float_60_70_50
     %2 = OpCopyObject %v3float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = )" +
                  GetParam().wgsl_func +
                  R"( vec3<f32>(50.0f, 60.0f, 70.0f), vec3<f32>(60.0f, 70.0f, 50.0f)
    %3:vec3<f32> = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Float_Floating,
                         ::testing::Values(GlslStd450Case{"Length", "length"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Float_FloatingFloating,
                         ::testing::Values(GlslStd450Case{"Distance", "distance"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Floating_Floating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"Acos", "acos"},                //
                             {"Acosh", "acosh"},              //
                             {"Asin", "asin"},                //
                             {"Asinh", "asinh"},              //
                             {"Atan", "atan"},                //
                             {"Atanh", "atanh"},              //
                             {"Ceil", "ceil"},                //
                             {"Cos", "cos"},                  //
                             {"Cosh", "cosh"},                //
                             {"Degrees", "degrees"},          //
                             {"Exp", "exp"},                  //
                             {"Exp2", "exp2"},                //
                             {"FAbs", "abs"},                 //
                             {"FSign", "sign"},               //
                             {"Floor", "floor"},              //
                             {"Fract", "fract"},              //
                             {"InverseSqrt", "inverseSqrt"},  //
                             {"Log", "log"},                  //
                             {"Log2", "log2"},                //
                             {"Radians", "radians"},          //
                             {"Round", "round"},              //
                             {"RoundEven", "round"},          //
                             {"Sin", "sin"},                  //
                             {"Sinh", "sinh"},                //
                             {"Sqrt", "sqrt"},                //
                             {"Tan", "tan"},                  //
                             {"Tanh", "tanh"},                //
                             {"Trunc", "trunc"},              //
                         }));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Floating_FloatingFloating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"Atan2", "atan2"},
                             {"NMax", "max"},
                             {"NMin", "min"},
                             {"FMax", "max"},  // WGSL max promises more for NaN
                             {"FMin", "min"},  // WGSL min promises more for NaN
                             {"Pow", "pow"},
                             {"Step", "step"},
                         }));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Floating_FloatingInting,
                         ::testing::Values(GlslStd450Case{"Ldexp", "ldexp"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Float3_Float3Float3,
                         ::testing::Values(GlslStd450Case{"Cross", "cross"}));

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Floating_FloatingFloatingFloating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"NClamp", "clamp"},
                             {"FClamp", "clamp"},  // WGSL FClamp promises more for NaN
                             {"Fma", "fma"},
                             {"FMix", "mix"},
                             {"SmoothStep", "smoothstep"}}));

TEST_P(SpirvReaderTest_GlslStd450_Inting_Inting, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                  GetParam().opcode +
                  R"( %int_30
     %2 = OpCopyObject %int %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + GetParam().wgsl_func +
                  R"( 30i
    %3:i32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Inting_Inting, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                  GetParam().opcode +
                  R"( %v2int_30_40
     %2 = OpCopyObject %v2int %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<i32>(30i, 40i)
    %3:vec2<i32> = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Inting_Inting,
                         ::testing::Values(GlslStd450Case{"FindILsb", "firstTrailingBit"},
                                           GlslStd450Case{"FindSMsb", "firstLeadingBit"}));

TEST_P(SpirvReaderTest_GlslStd450_Uinting_Uinting, Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                  GetParam().opcode +
                  R"( %uint_10
     %2 = OpCopyObject %uint %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + GetParam().wgsl_func +
                  R"( 10u
    %3:u32 = let %2
    ret
  }
}
)");
}

TEST_P(SpirvReaderTest_GlslStd450_Uinting_Uinting, Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                  GetParam().opcode +
                  R"( %v2uint_10_20
     %2 = OpCopyObject %v2uint %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = )" +
                  GetParam().wgsl_func +
                  R"( vec2<u32>(10u, 20u)
    %3:vec2<u32> = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_Uinting_Uinting,
                         ::testing::Values(GlslStd450Case{"FindILsb", "firstTrailingBit"},
                                           GlslStd450Case{"FindUMsb", "firstLeadingBit"}));

// Test Normalize.  WGSL does not have a scalar form of the normalize builtin.
// So we have to test it separately, as it does not fit the patterns tested
// above.

TEST_F(SpirvReaderTest, Normalize_Scalar) {
    // Scalar normalize maps to sign.
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %float %glsl Normalize %float_50
     %2 = OpCopyObject %float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = sign 50.0f
    %3:f32 = let %2
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Normalize_Vector2) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v2float %glsl Normalize %v2float_50_60
     %2 = OpCopyObject %v2float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = normalize vec2<f32>(50.0f, 60.0f)
    %3:vec2<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Normalize_Vector3) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v3float %glsl Normalize %v3float_50_60_70
     %2 = OpCopyObject %v3float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<f32> = normalize vec3<f32>(50.0f, 60.0f, 70.0f)
    %3:vec3<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Normalize_Vector4) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %v4float %glsl Normalize %v4float_50_50_50_50
     %2 = OpCopyObject %v4float %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec4<f32> = normalize vec4<f32>(50.0f)
    %3:vec4<f32> = let %2
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, DISABLED_RectifyOperandsAndResult_FindUMsb) {
    // Check signedness conversion of arguments and results.
    //   SPIR-V signed arg -> cast arg to unsigned
    //      signed result -> cast result to signed
    //      unsigned result -> keep it
    //
    //   SPIR-V unsigned arg -> keep it
    //      signed result -> cast result to signed
    //      unsigned result -> keep it
    EXPECT_IR(Preamble() + R"(
     ; signed arg
     ;    signed result
     %1 = OpExtInst %int %glsl FindUMsb %int_30
     %2 = OpExtInst %v2int %glsl FindUMsb %v2int_30_40

     ; signed arg
     ;    unsigned result
     %3 = OpExtInst %uint %glsl FindUMsb %int_30
     %4 = OpExtInst %v2uint %glsl FindUMsb %v2int_30_40

     ; unsigned arg
     ;    signed result
     %5 = OpExtInst %int %glsl FindUMsb %uint_10
     %6 = OpExtInst %v2int %glsl FindUMsb %v2uint_10_20

     ; unsigned arg
     ;    unsigned result
     %7 = OpExtInst %uint %glsl FindUMsb %uint_10
     %8 = OpExtInst %v2uint %glsl FindUMsb %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    let x_1 = bitcast<i32>(firstLeadingBit(bitcast<u32>(i1)));
    let x_2 = bitcast<vec2i>(firstLeadingBit(bitcast<vec2u>(v2i1)));
    let x_3 = firstLeadingBit(bitcast<u32>(i1));
    let x_4 = firstLeadingBit(bitcast<vec2u>(v2i1));
    let x_5 = bitcast<i32>(firstLeadingBit(u1));
    let x_6 = bitcast<vec2i>(firstLeadingBit(v2u1));
    let x_7 = firstLeadingBit(u1);
    let x_8 = firstLeadingBit(v2u1);
  }
}
)");
}

struct DataPackingCase {
    std::string opcode;
    std::string wgsl_func;
    uint32_t vec_size;
};

inline std::ostream& operator<<(std::ostream& out, DataPackingCase c) {
    out << "DataPacking(" << c.opcode << ")";
    return out;
}

using SpirvReaderTest_GlslStd450_DataPacking = SpirvReaderTestWithParam<DataPackingCase>;

TEST_P(SpirvReaderTest_GlslStd450_DataPacking, Valid) {
    auto param = GetParam();
    EXPECT_IR(Preamble() + R"(
  %1 = OpExtInst %uint %glsl )" +
                  param.opcode +
                  (param.vec_size == 2 ? " %v2float_50_60" : " %v4float_50_50_50_50") + R"(
  %2 = OpCopyObject %uint %1
  OpReturn
  OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + param.wgsl_func +
                  " vec" + std::to_string(param.vec_size) + "<f32>(50.0f" +
                  (param.vec_size == 4 ? "" : ", 60.0f") + R"()
    %3:u32 = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_DataPacking,
                         ::testing::ValuesIn(std::vector<DataPackingCase>{
                             {"PackSnorm4x8", "pack4x8snorm", 4},
                             {"PackUnorm4x8", "pack4x8unorm", 4},
                             {"PackSnorm2x16", "pack2x16snorm", 2},
                             {"PackUnorm2x16", "pack2x16unorm", 2},
                             {"PackHalf2x16", "pack2x16float", 2}}));

using SpirvReaderTest_GlslStd450_DataUnpacking = SpirvReaderTestWithParam<DataPackingCase>;

TEST_P(SpirvReaderTest_GlslStd450_DataUnpacking, Valid) {
    auto param = GetParam();
    auto type = param.vec_size == 2 ? "%v2float" : "%v4float";
    auto wgsl_type = "vec" + std::to_string(param.vec_size) + "<f32>";

    EXPECT_IR(Preamble() + R"(
  %1 = OpExtInst )" +
                  type + std::string(" %glsl ") + param.opcode + R"( %uint_10
  %2 = OpCopyObject )" +
                  type + R"( %1
  OpReturn
  OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:)" + wgsl_type +
                  " = " + param.wgsl_func +
                  R"( 10u
    %3:)" + wgsl_type +
                  R"( = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReaderTest_GlslStd450_DataUnpacking,
                         ::testing::ValuesIn(std::vector<DataPackingCase>{
                             {"UnpackSnorm4x8", "unpack4x8snorm", 4},
                             {"UnpackUnorm4x8", "unpack4x8unorm", 4},
                             {"UnpackSnorm2x16", "unpack2x16snorm", 2},
                             {"UnpackUnorm2x16", "unpack2x16unorm", 2},
                             {"UnpackHalf2x16", "unpack2x16float", 2}}));

struct DeterminantData {
    std::string in;
    std::string out;
    std::string ty;
    std::string ty_name;
};

[[maybe_unused]] inline std::ostream& operator<<(std::ostream& out, DeterminantData c) {
    out << "Determinant(" << c.in << ")";
    return out;
}

using SpirvReaderTest_GlslStd450_Determinant = SpirvReaderTestWithParam<DeterminantData>;

TEST_P(SpirvReaderTest_GlslStd450_Determinant, Test) {
    auto param = GetParam();

    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %)" +
                  param.ty_name + R"( %glsl Determinant %)" + param.in + R"(
     %2 = OpCopyObject %)" +
                  param.ty_name + R"( %1
     OpReturn
     OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:)" + param.ty +
                  R"( = determinant )" + param.out + R"(
    %3:)" + param.ty +
                  R"( = let %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReaderTest_GlslStd450_Determinant,
    ::testing::Values(
        DeterminantData{"mat2v2float_50_60", "mat2x2<f32>(vec2<f32>(50.0f, 60.0f))", "f32",
                        "float"},
        DeterminantData{"mat3v3float_50_60_70", "mat3x3<f32>(vec3<f32>(50.0f, 60.0f, 70.0f))",
                        "f32", "float"},
        DeterminantData{"mat4v4float_50_50_50_50", "mat4x4<f32>(vec4<f32>(50.0f))", "f32", "float"},
        DeterminantData{"mat2v2half_50_60", "mat2x2<f16>(vec2<f16>(50.0h, 60.0h))", "f16", "half"},
        DeterminantData{"mat3v3half_50_60_70", "mat3x3<f16>(vec3<f16>(50.0h, 60.0h, 70.0h))", "f16",
                        "half"},
        DeterminantData{"mat4v4half_50_50_50_50", "mat4x4<f16>(vec4<f16>(50.0h))", "f16", "half"}));

TEST_F(SpirvReaderTest, ModfStruct_Store) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpVariable %ptr_function_modf_result_type Function
     %2 = OpExtInst %modf_result_type %glsl ModfStruct %float_50
     OpStore %1 %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:f32 @offset(4)
}

__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, tint_symbol_2, read_write> = var
    %3:__modf_result_f32 = modf 50.0f
    %4:f32 = access %3, 0u
    %5:f32 = access %3, 1u
    %6:tint_symbol_2 = construct %4, %5
    store %2, %6
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ModfStruct_Scalar) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %modf_result_type %glsl ModfStruct %float_50
     %2 = OpCompositeExtract %float %1 0
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:f32 @offset(4)
}

__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:__modf_result_f32 = modf 50.0f
    %3:f32 = access %2, 0u
    %4:f32 = access %2, 1u
    %5:tint_symbol_2 = construct %3, %4
    %6:f32 = access %5, 0u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ModfStruct_Vector) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %modf_v2_result_type %glsl ModfStruct %v2float_50_60
     %2 = OpCompositeExtract %v2float %1 0
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(8) {
  tint_symbol:vec2<f32> @offset(0)
  tint_symbol_1:vec2<f32> @offset(8)
}

__modf_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  whole:vec2<f32> @offset(8)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:__modf_result_vec2_f32 = modf vec2<f32>(50.0f, 60.0f)
    %3:vec2<f32> = access %2, 0u
    %4:vec2<f32> = access %2, 1u
    %5:tint_symbol_2 = construct %3, %4
    %6:vec2<f32> = access %5, 0u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, FrexpStruct_Store) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpVariable %ptr_function_frexp_result_type_unsigned Function
     %2 = OpExtInst %frexp_result_type_unsigned %glsl FrexpStruct %float_50
     OpStore %1 %2
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, tint_symbol_2, read_write> = var
    %3:__frexp_result_f32 = frexp 50.0f
    %4:f32 = access %3, 0u
    %5:i32 = access %3, 1u
    %6:u32 = bitcast %5
    %7:tint_symbol_2 = construct %4, %6
    store %2, %7
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, FrexpStruct_ScalarUnsigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %frexp_result_type_unsigned %glsl FrexpStruct %float_50
     %2 = OpCompositeExtract %float %1 0
     %3 = OpCompositeExtract %uint %1 1
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:u32 @offset(4)
}

__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:__frexp_result_f32 = frexp 50.0f
    %3:f32 = access %2, 0u
    %4:i32 = access %2, 1u
    %5:u32 = bitcast %4
    %6:tint_symbol_2 = construct %3, %5
    %7:f32 = access %6, 0u
    %8:u32 = access %6, 1u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, FrexpStruct_ScalarSigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %frexp_result_type_signed %glsl FrexpStruct %float_50
     %2 = OpCompositeExtract %float %1 0
     %3 = OpCompositeExtract %int %1 1
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:f32 @offset(0)
  tint_symbol_1:i32 @offset(4)
}

__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:__frexp_result_f32 = frexp 50.0f
    %3:f32 = access %2, 0u
    %4:i32 = access %2, 1u
    %5:tint_symbol_2 = construct %3, %4
    %6:f32 = access %5, 0u
    %7:i32 = access %5, 1u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, FrexpStruct_VectorUnsigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %frexp_v2_result_type_unsigned %glsl FrexpStruct %v2float_50_60
     %2 = OpCompositeExtract %v2float %1 0
     %3 = OpCompositeExtract %v2uint %1 1
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(8) {
  tint_symbol:vec2<f32> @offset(0)
  tint_symbol_1:vec2<u32> @offset(8)
}

__frexp_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  exp:vec2<i32> @offset(8)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:__frexp_result_vec2_f32 = frexp vec2<f32>(50.0f, 60.0f)
    %3:vec2<f32> = access %2, 0u
    %4:vec2<i32> = access %2, 1u
    %5:vec2<u32> = bitcast %4
    %6:tint_symbol_2 = construct %3, %5
    %7:vec2<f32> = access %6, 0u
    %8:vec2<u32> = access %6, 1u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, FrexpStruct_VectorSigned) {
    EXPECT_IR(Preamble() + R"(
     %1 = OpExtInst %frexp_v2_result_type_signed %glsl FrexpStruct %v2float_50_60
     %2 = OpCompositeExtract %v2float %1 0
     %3 = OpCompositeExtract %v2int %1 1
     OpReturn
     OpFunctionEnd
  )",
              R"(
tint_symbol_2 = struct @align(8) {
  tint_symbol:vec2<f32> @offset(0)
  tint_symbol_1:vec2<i32> @offset(8)
}

__frexp_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  exp:vec2<i32> @offset(8)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:__frexp_result_vec2_f32 = frexp vec2<f32>(50.0f, 60.0f)
    %3:vec2<f32> = access %2, 0u
    %4:vec2<i32> = access %2, 1u
    %5:tint_symbol_2 = construct %3, %4
    %6:vec2<f32> = access %5, 0u
    %7:vec2<i32> = access %5, 1u
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
