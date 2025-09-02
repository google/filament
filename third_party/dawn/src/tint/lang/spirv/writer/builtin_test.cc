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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/builtin_structs.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer {
namespace {

/// A parameterized builtin function test case.
struct BuiltinTestCase {
    /// The element type to test.
    TestElementType type;
    /// The builtin function.
    enum core::BuiltinFn function;
    /// The expected SPIR-V instruction string.
    std::string spirv_inst;
};

// Tests for builtins with the signature: T = func(T)
using Builtin_1arg = SpirvWriterTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_1arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(MakeScalarType(params.type), params.function, MakeScalarValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
TEST_P(Builtin_1arg, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    Builtin_1arg,
    testing::Values(BuiltinTestCase{kF32, core::BuiltinFn::kAbs, "FAbs"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kAbs, "FAbs"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kAcos, "Acos"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kAcos, "Acos"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kAsinh, "Asinh"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kAsinh, "Asinh"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kAcos, "Acos"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kAcos, "Acos"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kAsinh, "Asinh"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kAsinh, "Asinh"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kAtan, "Atan"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kAtan, "Atan"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kAtanh, "Atanh"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kAtanh, "Atanh"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kCeil, "Ceil"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kCeil, "Ceil"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kCos, "Cos"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kCos, "Cos"},
                    BuiltinTestCase{kI32, core::BuiltinFn::kCountOneBits, "OpBitCount"},
                    BuiltinTestCase{kU32, core::BuiltinFn::kCountOneBits, "OpBitCount"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kDpdx, "OpDPdx"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kDpdxCoarse, "OpDPdxCoarse"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kDpdxFine, "OpDPdxFine"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kDpdy, "OpDPdy"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kDpdyCoarse, "OpDPdyCoarse"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kDpdyFine, "OpDPdyFine"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kDegrees, "Degrees"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kDegrees, "Degrees"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kExp, "Exp"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kExp, "Exp"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kExp2, "Exp2"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kExp2, "Exp2"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kFloor, "Floor"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kFloor, "Floor"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kFract, "Fract"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kFract, "Fract"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kFwidth, "OpFwidth"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kFwidthCoarse, "OpFwidthCoarse"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kFwidthFine, "OpFwidthFine"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kInverseSqrt, "InverseSqrt"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kInverseSqrt, "InverseSqrt"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kLog, "Log"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kLog, "Log"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kLog2, "Log2"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kLog2, "Log2"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kQuantizeToF16, "OpQuantizeToF16"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kRadians, "Radians"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kRadians, "Radians"},
                    BuiltinTestCase{kI32, core::BuiltinFn::kReverseBits, "OpBitReverse"},
                    BuiltinTestCase{kU32, core::BuiltinFn::kReverseBits, "OpBitReverse"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kRound, "RoundEven"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kRound, "RoundEven"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kSign, "FSign"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kSign, "FSign"},
                    BuiltinTestCase{kI32, core::BuiltinFn::kSign, "SSign"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kSin, "Sin"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kSin, "Sin"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kSqrt, "Sqrt"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kSqrt, "Sqrt"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kTan, "Tan"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kTan, "Tan"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kTrunc, "Trunc"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kTrunc, "Trunc"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kCosh, "Cosh"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kCosh, "Cosh"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kSinh, "Sinh"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kSinh, "Sinh"},
                    BuiltinTestCase{kF32, core::BuiltinFn::kTanh, "Tanh"},
                    BuiltinTestCase{kF16, core::BuiltinFn::kTanh, "Tanh"}));

// Test that abs of an unsigned value just folds away.
TEST_F(SpirvWriterTest, Builtin_Abs_u32) {
    auto* func = b.Function("foo", MakeScalarType(kU32));
    b.Append(func->Block(), [&] {
        auto* arg = MakeScalarValue(kU32);
        auto* result = b.Call(MakeScalarType(kU32), core::BuiltinFn::kAbs, arg);
        b.Return(func, result);
        mod.SetName(arg, "arg");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %uint None %3
          %4 = OpLabel
               OpReturnValue %arg
               OpFunctionEnd
)");
}

// Test that abs of a signed integer is implemented as max(x,-x);
TEST_F(SpirvWriterTest, Builtin_Abs_i32) {
    auto* func = b.Function("foo", MakeScalarType(kI32));
    b.Append(func->Block(), [&] {
        auto* arg = MakeScalarValue(kI32);
        auto* result = b.Call(MakeScalarType(kI32), core::BuiltinFn::kAbs, arg);
        b.Return(func, result);
        mod.SetName(arg, "arg");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %6 = OpBitcast %uint %arg
          %8 = OpNot %uint %6
          %9 = OpIAdd %uint %8 %uint_1
         %11 = OpBitcast %int %9
         %12 = OpExtInst %int %13 SMax %arg %11
)");
}

TEST_F(SpirvWriterTest, Builtin_Abs_vec2u) {
    auto* func = b.Function("foo", MakeVectorType(kU32));
    b.Append(func->Block(), [&] {
        auto* arg = MakeVectorValue(kU32);
        auto* result = b.Call(MakeVectorType(kU32), core::BuiltinFn::kAbs, arg);
        b.Return(func, result);
        mod.SetName(arg, "arg");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %v2uint None %4
          %5 = OpLabel
               OpReturnValue %arg
               OpFunctionEnd
)");
}

// Test that all of a scalar just folds away.
TEST_F(SpirvWriterTest, Builtin_All_Scalar) {
    auto* arg = b.FunctionParam("arg", ty.bool_());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), core::BuiltinFn::kAll, arg);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpReturnValue %arg");
}

TEST_F(SpirvWriterTest, Builtin_All_Vector) {
    auto* arg = b.FunctionParam("arg", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), core::BuiltinFn::kAll, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAll %bool %arg");
}

// Test that any of a scalar just folds away.
TEST_F(SpirvWriterTest, Builtin_Any_Scalar) {
    auto* arg = b.FunctionParam("arg", ty.bool_());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), core::BuiltinFn::kAny, arg);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpReturnValue %arg");
}

TEST_F(SpirvWriterTest, Builtin_Any_Vector) {
    auto* arg = b.FunctionParam("arg", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), core::BuiltinFn::kAny, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAny %bool %arg");
}

TEST_F(SpirvWriterTest, Builtin_Determinant_Mat4x4f) {
    auto* arg = b.FunctionParam("arg", ty.mat4x4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kDeterminant, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %9 Determinant %arg");
}

TEST_F(SpirvWriterTest, Builtin_Determinant_Mat3x3h) {
    auto* arg = b.FunctionParam("arg", ty.mat3x3<f16>());
    auto* func = b.Function("foo", ty.f16());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f16(), core::BuiltinFn::kDeterminant, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %half %9 Determinant %arg");
}

TEST_F(SpirvWriterTest, Builtin_Frexp_F32) {
    auto* str = core::type::CreateFrexpResult(ty, mod.symbols, ty.f32());
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, core::BuiltinFn::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_f32 %9 FrexpStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Frexp_F16) {
    auto* str = core::type::CreateFrexpResult(ty, mod.symbols, ty.f16());
    auto* arg = b.FunctionParam("arg", ty.f16());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, core::BuiltinFn::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_f16 %9 FrexpStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Frexp_Vec2f) {
    auto* str = core::type::CreateFrexpResult(ty, mod.symbols, ty.vec2<f32>());
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, core::BuiltinFn::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_vec2_f32 %11 FrexpStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Frexp_Vec3h) {
    auto* str = core::type::CreateFrexpResult(ty, mod.symbols, ty.vec3<f16>());
    auto* arg = b.FunctionParam("arg", ty.vec3<f16>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, core::BuiltinFn::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_vec3_f16 %11 FrexpStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Length_vec4f) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kLength, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %8 Length %arg");
}

TEST_F(SpirvWriterTest, Builtin_Modf_F32) {
    auto* str = core::type::CreateModfResult(ty, mod.symbols, ty.f32());
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, core::BuiltinFn::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_f32 %8 ModfStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Modf_F16) {
    auto* str = core::type::CreateModfResult(ty, mod.symbols, ty.f16());
    auto* arg = b.FunctionParam("arg", ty.f16());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, core::BuiltinFn::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_f16 %8 ModfStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Modf_Vec2f) {
    auto* str = core::type::CreateModfResult(ty, mod.symbols, ty.vec2<f32>());
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, core::BuiltinFn::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_vec2_f32 %9 ModfStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Modf_Vec3h) {
    auto* str = core::type::CreateModfResult(ty, mod.symbols, ty.vec3<f16>());
    auto* arg = b.FunctionParam("arg", ty.vec3<f16>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, core::BuiltinFn::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_vec3_f16 %9 ModfStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Normalize_vec4f) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kNormalize, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %8 Normalize %arg");
}

TEST_F(SpirvWriterTest, Builtin_Transpose_Mat2x3f) {
    auto* arg = b.FunctionParam("arg", ty.mat2x3<f32>());
    auto* func = b.Function("foo", ty.mat3x2<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.mat3x2<f32>(), core::BuiltinFn::kTranspose, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpTranspose %mat3v2float %arg");
}

TEST_F(SpirvWriterTest, Builtin_Transpose_Mat4x4f) {
    auto* arg = b.FunctionParam("arg", ty.mat4x4<f32>());
    auto* func = b.Function("foo", ty.mat4x4<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.mat4x4<f32>(), core::BuiltinFn::kTranspose, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpTranspose %mat4v4float %arg");
}

TEST_F(SpirvWriterTest, Builtin_Transpose_Mat4x3h) {
    auto* arg = b.FunctionParam("arg", ty.mat4x3<f16>());
    auto* func = b.Function("foo", ty.mat3x4<f16>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.mat3x4<f16>(), core::BuiltinFn::kTranspose, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpTranspose %mat3v4half %arg");
}

TEST_F(SpirvWriterTest, Builtin_Transpose_Mat2x2h) {
    auto* arg = b.FunctionParam("arg", ty.mat2x2<f16>());
    auto* func = b.Function("foo", ty.mat2x2<f16>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.mat2x2<f16>(), core::BuiltinFn::kTranspose, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpTranspose %mat2v2half %arg");
}

TEST_F(SpirvWriterTest, Builtin_Pack2X16Float) {
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kPack2X16Float, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %uint %9 PackHalf2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Pack2X16Snorm) {
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kPack2X16Snorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %uint %9 PackSnorm2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Pack2X16Unorm) {
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kPack2X16Unorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %uint %9 PackUnorm2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Pack4X8Snorm) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kPack4X8Snorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %uint %9 PackSnorm4x8 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Pack4X8Unorm) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kPack4X8Unorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %uint %9 PackUnorm4x8 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Unpack2X16Float) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Float, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v2float %9 UnpackHalf2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Unpack2X16Snorm) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Snorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v2float %9 UnpackSnorm2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Unpack2X16Unorm) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Unorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v2float %9 UnpackUnorm2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Unpack4X8Snorm) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kUnpack4X8Snorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %9 UnpackSnorm4x8 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Unpack4X8Unorm) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kUnpack4X8Unorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %9 UnpackUnorm4x8 %arg");
}

TEST_F(SpirvWriterTest, Builtin_CountLeadingZeros_U32) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kCountLeadingZeros, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %6 = OpULessThanEqual %bool %arg %uint_65535
          %9 = OpSelect %uint %6 %uint_16 %uint_0
         %12 = OpShiftLeftLogical %uint %arg %9
         %13 = OpULessThanEqual %bool %12 %uint_16777215
         %15 = OpSelect %uint %13 %uint_8 %uint_0
         %17 = OpShiftLeftLogical %uint %12 %15
         %18 = OpULessThanEqual %bool %17 %uint_268435455
         %20 = OpSelect %uint %18 %uint_4 %uint_0
         %22 = OpShiftLeftLogical %uint %17 %20
         %23 = OpULessThanEqual %bool %22 %uint_1073741823
         %25 = OpSelect %uint %23 %uint_2 %uint_0
         %27 = OpShiftLeftLogical %uint %22 %25
         %28 = OpULessThanEqual %bool %27 %uint_2147483647
         %30 = OpSelect %uint %28 %uint_1 %uint_0
         %32 = OpIEqual %bool %27 %uint_0
         %33 = OpSelect %uint %32 %uint_1 %uint_0
         %34 = OpBitwiseOr %uint %30 %33
         %35 = OpBitwiseOr %uint %25 %34
         %36 = OpBitwiseOr %uint %20 %35
         %37 = OpBitwiseOr %uint %15 %36
         %38 = OpBitwiseOr %uint %9 %37
     %result = OpIAdd %uint %38 %33
)");
}

TEST_F(SpirvWriterTest, Builtin_CountLeadingZeros_I32) {
    auto* arg = b.FunctionParam("arg", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kCountLeadingZeros, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %7 = OpBitcast %uint %arg
          %8 = OpULessThanEqual %bool %7 %uint_65535
         %11 = OpSelect %uint %8 %uint_16 %uint_0
         %14 = OpShiftLeftLogical %uint %7 %11
         %15 = OpULessThanEqual %bool %14 %uint_16777215
         %17 = OpSelect %uint %15 %uint_8 %uint_0
         %19 = OpShiftLeftLogical %uint %14 %17
         %20 = OpULessThanEqual %bool %19 %uint_268435455
         %22 = OpSelect %uint %20 %uint_4 %uint_0
         %24 = OpShiftLeftLogical %uint %19 %22
         %25 = OpULessThanEqual %bool %24 %uint_1073741823
         %27 = OpSelect %uint %25 %uint_2 %uint_0
         %29 = OpShiftLeftLogical %uint %24 %27
         %30 = OpULessThanEqual %bool %29 %uint_2147483647
         %32 = OpSelect %uint %30 %uint_1 %uint_0
         %34 = OpIEqual %bool %29 %uint_0
         %35 = OpSelect %uint %34 %uint_1 %uint_0
         %36 = OpBitwiseOr %uint %32 %35
         %37 = OpBitwiseOr %uint %27 %36
         %38 = OpBitwiseOr %uint %22 %37
         %39 = OpBitwiseOr %uint %17 %38
         %40 = OpBitwiseOr %uint %11 %39
         %41 = OpIAdd %uint %40 %35
     %result = OpBitcast %int %41
)");
}

TEST_F(SpirvWriterTest, Builtin_CountLeadingZeros_Vec2U32) {
    auto* arg = b.FunctionParam("arg", ty.vec2<u32>());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kCountLeadingZeros, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%8 = OpConstantComposite %v2uint %uint_65535 %uint_65535");
    EXPECT_INST("%13 = OpConstantComposite %v2uint %uint_16 %uint_16");
    EXPECT_INST("%15 = OpConstantNull %v2uint");
    EXPECT_INST("%18 = OpConstantComposite %v2uint %uint_16777215 %uint_16777215");
    EXPECT_INST("%21 = OpConstantComposite %v2uint %uint_8 %uint_8");
    EXPECT_INST("%25 = OpConstantComposite %v2uint %uint_268435455 %uint_268435455");
    EXPECT_INST("%28 = OpConstantComposite %v2uint %uint_4 %uint_4");
    EXPECT_INST("%32 = OpConstantComposite %v2uint %uint_1073741823 %uint_1073741823");
    EXPECT_INST("%35 = OpConstantComposite %v2uint %uint_2 %uint_2");
    EXPECT_INST("%39 = OpConstantComposite %v2uint %uint_2147483647 %uint_2147483647");
    EXPECT_INST("%42 = OpConstantComposite %v2uint %uint_1 %uint_1");
    EXPECT_INST(R"(
          %7 = OpULessThanEqual %v2bool %arg %8
         %12 = OpSelect %v2uint %7 %13 %15
         %16 = OpShiftLeftLogical %v2uint %arg %12
         %17 = OpULessThanEqual %v2bool %16 %18
         %20 = OpSelect %v2uint %17 %21 %15
         %23 = OpShiftLeftLogical %v2uint %16 %20
         %24 = OpULessThanEqual %v2bool %23 %25
         %27 = OpSelect %v2uint %24 %28 %15
         %30 = OpShiftLeftLogical %v2uint %23 %27
         %31 = OpULessThanEqual %v2bool %30 %32
         %34 = OpSelect %v2uint %31 %35 %15
         %37 = OpShiftLeftLogical %v2uint %30 %34
         %38 = OpULessThanEqual %v2bool %37 %39
         %41 = OpSelect %v2uint %38 %42 %15
         %44 = OpIEqual %v2bool %37 %15
         %45 = OpSelect %v2uint %44 %42 %15
         %46 = OpBitwiseOr %v2uint %41 %45
         %47 = OpBitwiseOr %v2uint %34 %46
         %48 = OpBitwiseOr %v2uint %27 %47
         %49 = OpBitwiseOr %v2uint %20 %48
         %50 = OpBitwiseOr %v2uint %12 %49
     %result = OpIAdd %v2uint %50 %45
)");
}

TEST_F(SpirvWriterTest, Builtin_CountTrailingZeros_U32) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kCountTrailingZeros, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %6 = OpBitwiseAnd %uint %arg %uint_65535
          %8 = OpIEqual %bool %6 %uint_0
         %11 = OpSelect %uint %8 %uint_16 %uint_0
         %13 = OpShiftRightLogical %uint %arg %11
         %14 = OpBitwiseAnd %uint %13 %uint_255
         %16 = OpIEqual %bool %14 %uint_0
         %17 = OpSelect %uint %16 %uint_8 %uint_0
         %19 = OpShiftRightLogical %uint %13 %17
         %20 = OpBitwiseAnd %uint %19 %uint_15
         %22 = OpIEqual %bool %20 %uint_0
         %23 = OpSelect %uint %22 %uint_4 %uint_0
         %25 = OpShiftRightLogical %uint %19 %23
         %26 = OpBitwiseAnd %uint %25 %uint_3
         %28 = OpIEqual %bool %26 %uint_0
         %29 = OpSelect %uint %28 %uint_2 %uint_0
         %31 = OpShiftRightLogical %uint %25 %29
         %32 = OpBitwiseAnd %uint %31 %uint_1
         %34 = OpIEqual %bool %32 %uint_0
         %35 = OpSelect %uint %34 %uint_1 %uint_0
         %36 = OpIEqual %bool %31 %uint_0
         %37 = OpSelect %uint %36 %uint_1 %uint_0
         %38 = OpBitwiseOr %uint %29 %35
         %39 = OpBitwiseOr %uint %23 %38
         %40 = OpBitwiseOr %uint %17 %39
         %41 = OpBitwiseOr %uint %11 %40
     %result = OpIAdd %uint %41 %37
)");
}

TEST_F(SpirvWriterTest, Builtin_CountTrailingZeros_I32) {
    auto* arg = b.FunctionParam("arg", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kCountTrailingZeros, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %7 = OpBitcast %uint %arg
          %8 = OpBitwiseAnd %uint %7 %uint_65535
         %10 = OpIEqual %bool %8 %uint_0
         %13 = OpSelect %uint %10 %uint_16 %uint_0
         %15 = OpShiftRightLogical %uint %7 %13
         %16 = OpBitwiseAnd %uint %15 %uint_255
         %18 = OpIEqual %bool %16 %uint_0
         %19 = OpSelect %uint %18 %uint_8 %uint_0
         %21 = OpShiftRightLogical %uint %15 %19
         %22 = OpBitwiseAnd %uint %21 %uint_15
         %24 = OpIEqual %bool %22 %uint_0
         %25 = OpSelect %uint %24 %uint_4 %uint_0
         %27 = OpShiftRightLogical %uint %21 %25
         %28 = OpBitwiseAnd %uint %27 %uint_3
         %30 = OpIEqual %bool %28 %uint_0
         %31 = OpSelect %uint %30 %uint_2 %uint_0
         %33 = OpShiftRightLogical %uint %27 %31
         %34 = OpBitwiseAnd %uint %33 %uint_1
         %36 = OpIEqual %bool %34 %uint_0
         %37 = OpSelect %uint %36 %uint_1 %uint_0
         %38 = OpIEqual %bool %33 %uint_0
         %39 = OpSelect %uint %38 %uint_1 %uint_0
         %40 = OpBitwiseOr %uint %31 %37
         %41 = OpBitwiseOr %uint %25 %40
         %42 = OpBitwiseOr %uint %19 %41
         %43 = OpBitwiseOr %uint %13 %42
         %44 = OpIAdd %uint %43 %39
     %result = OpBitcast %int %44
)");
}

TEST_F(SpirvWriterTest, Builtin_CountTrailingZeros_Vec2U32) {
    auto* arg = b.FunctionParam("arg", ty.vec2<u32>());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kCountTrailingZeros, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%8 = OpConstantComposite %v2uint %uint_65535 %uint_65535");
    EXPECT_INST("%11 = OpConstantNull %v2uint");
    EXPECT_INST("%15 = OpConstantComposite %v2uint %uint_16 %uint_16");
    EXPECT_INST("%19 = OpConstantComposite %v2uint %uint_255 %uint_255");
    EXPECT_INST("%23 = OpConstantComposite %v2uint %uint_8 %uint_8");
    EXPECT_INST("%27 = OpConstantComposite %v2uint %uint_15 %uint_15");
    EXPECT_INST("%31 = OpConstantComposite %v2uint %uint_4 %uint_4");
    EXPECT_INST("%35 = OpConstantComposite %v2uint %uint_3 %uint_3");
    EXPECT_INST("%39 = OpConstantComposite %v2uint %uint_2 %uint_2");
    EXPECT_INST("%43 = OpConstantComposite %v2uint %uint_1 %uint_1");
    EXPECT_INST(R"(
          %7 = OpBitwiseAnd %v2uint %arg %8
         %10 = OpIEqual %v2bool %7 %11
         %14 = OpSelect %v2uint %10 %15 %11
         %17 = OpShiftRightLogical %v2uint %arg %14
         %18 = OpBitwiseAnd %v2uint %17 %19
         %21 = OpIEqual %v2bool %18 %11
         %22 = OpSelect %v2uint %21 %23 %11
         %25 = OpShiftRightLogical %v2uint %17 %22
         %26 = OpBitwiseAnd %v2uint %25 %27
         %29 = OpIEqual %v2bool %26 %11
         %30 = OpSelect %v2uint %29 %31 %11
         %33 = OpShiftRightLogical %v2uint %25 %30
         %34 = OpBitwiseAnd %v2uint %33 %35
         %37 = OpIEqual %v2bool %34 %11
         %38 = OpSelect %v2uint %37 %39 %11
         %41 = OpShiftRightLogical %v2uint %33 %38
         %42 = OpBitwiseAnd %v2uint %41 %43
         %45 = OpIEqual %v2bool %42 %11
         %46 = OpSelect %v2uint %45 %43 %11
         %47 = OpIEqual %v2bool %41 %11
         %48 = OpSelect %v2uint %47 %43 %11
         %49 = OpBitwiseOr %v2uint %38 %46
         %50 = OpBitwiseOr %v2uint %30 %49
         %51 = OpBitwiseOr %v2uint %22 %50
         %52 = OpBitwiseOr %v2uint %14 %51
     %result = OpIAdd %v2uint %52 %48
)");
}

TEST_F(SpirvWriterTest, Builtin_FirstLeadingBit_U32) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kFirstLeadingBit, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %6 = OpBitwiseAnd %uint %arg %uint_4294901760
          %8 = OpIEqual %bool %6 %uint_0
         %11 = OpSelect %uint %8 %uint_0 %uint_16
         %13 = OpShiftRightLogical %uint %arg %11
         %14 = OpBitwiseAnd %uint %13 %uint_65280
         %16 = OpIEqual %bool %14 %uint_0
         %17 = OpSelect %uint %16 %uint_0 %uint_8
         %19 = OpShiftRightLogical %uint %13 %17
         %20 = OpBitwiseAnd %uint %19 %uint_240
         %22 = OpIEqual %bool %20 %uint_0
         %23 = OpSelect %uint %22 %uint_0 %uint_4
         %25 = OpShiftRightLogical %uint %19 %23
         %26 = OpBitwiseAnd %uint %25 %uint_12
         %28 = OpIEqual %bool %26 %uint_0
         %29 = OpSelect %uint %28 %uint_0 %uint_2
         %31 = OpShiftRightLogical %uint %25 %29
         %32 = OpBitwiseAnd %uint %31 %uint_2
         %33 = OpIEqual %bool %32 %uint_0
         %34 = OpSelect %uint %33 %uint_0 %uint_1
         %36 = OpBitwiseOr %uint %29 %34
         %37 = OpBitwiseOr %uint %23 %36
         %38 = OpBitwiseOr %uint %17 %37
         %39 = OpBitwiseOr %uint %11 %38
         %40 = OpIEqual %bool %31 %uint_0
     %result = OpSelect %uint %40 %uint_4294967295 %39
)");
}

TEST_F(SpirvWriterTest, Builtin_FirstLeadingBit_I32) {
    auto* arg = b.FunctionParam("arg", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kFirstLeadingBit, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %7 = OpBitcast %uint %arg
          %8 = OpNot %uint %7
          %9 = OpULessThan %bool %7 %uint_2147483648
         %12 = OpSelect %uint %9 %7 %8
         %13 = OpBitwiseAnd %uint %12 %uint_4294901760
         %15 = OpIEqual %bool %13 %uint_0
         %17 = OpSelect %uint %15 %uint_0 %uint_16
         %19 = OpShiftRightLogical %uint %12 %17
         %20 = OpBitwiseAnd %uint %19 %uint_65280
         %22 = OpIEqual %bool %20 %uint_0
         %23 = OpSelect %uint %22 %uint_0 %uint_8
         %25 = OpShiftRightLogical %uint %19 %23
         %26 = OpBitwiseAnd %uint %25 %uint_240
         %28 = OpIEqual %bool %26 %uint_0
         %29 = OpSelect %uint %28 %uint_0 %uint_4
         %31 = OpShiftRightLogical %uint %25 %29
         %32 = OpBitwiseAnd %uint %31 %uint_12
         %34 = OpIEqual %bool %32 %uint_0
         %35 = OpSelect %uint %34 %uint_0 %uint_2
         %37 = OpShiftRightLogical %uint %31 %35
         %38 = OpBitwiseAnd %uint %37 %uint_2
         %39 = OpIEqual %bool %38 %uint_0
         %40 = OpSelect %uint %39 %uint_0 %uint_1
         %42 = OpBitwiseOr %uint %35 %40
         %43 = OpBitwiseOr %uint %29 %42
         %44 = OpBitwiseOr %uint %23 %43
         %45 = OpBitwiseOr %uint %17 %44
         %46 = OpIEqual %bool %37 %uint_0
         %47 = OpSelect %uint %46 %uint_4294967295 %45
     %result = OpBitcast %int %47
)");
}

TEST_F(SpirvWriterTest, Builtin_FirstLeadingBit_Vec2U32) {
    auto* arg = b.FunctionParam("arg", ty.vec2<u32>());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kFirstLeadingBit, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%8 = OpConstantComposite %v2uint %uint_4294901760 %uint_4294901760");
    EXPECT_INST("%11 = OpConstantNull %v2uint");
    EXPECT_INST("%15 = OpConstantComposite %v2uint %uint_16 %uint_16");
    EXPECT_INST("%19 = OpConstantComposite %v2uint %uint_65280 %uint_65280");
    EXPECT_INST("%23 = OpConstantComposite %v2uint %uint_8 %uint_8");
    EXPECT_INST("%27 = OpConstantComposite %v2uint %uint_240 %uint_240");
    EXPECT_INST("%31 = OpConstantComposite %v2uint %uint_4 %uint_4");
    EXPECT_INST("%35 = OpConstantComposite %v2uint %uint_12 %uint_12");
    EXPECT_INST("%39 = OpConstantComposite %v2uint %uint_2 %uint_2");
    EXPECT_INST("%45 = OpConstantComposite %v2uint %uint_1 %uint_1");
    EXPECT_INST("%53 = OpConstantComposite %v2uint %uint_4294967295 %uint_4294967295");
    EXPECT_INST(R"(
          %7 = OpBitwiseAnd %v2uint %arg %8
         %10 = OpIEqual %v2bool %7 %11
         %14 = OpSelect %v2uint %10 %11 %15
         %17 = OpShiftRightLogical %v2uint %arg %14
         %18 = OpBitwiseAnd %v2uint %17 %19
         %21 = OpIEqual %v2bool %18 %11
         %22 = OpSelect %v2uint %21 %11 %23
         %25 = OpShiftRightLogical %v2uint %17 %22
         %26 = OpBitwiseAnd %v2uint %25 %27
         %29 = OpIEqual %v2bool %26 %11
         %30 = OpSelect %v2uint %29 %11 %31
         %33 = OpShiftRightLogical %v2uint %25 %30
         %34 = OpBitwiseAnd %v2uint %33 %35
         %37 = OpIEqual %v2bool %34 %11
         %38 = OpSelect %v2uint %37 %11 %39
         %41 = OpShiftRightLogical %v2uint %33 %38
         %42 = OpBitwiseAnd %v2uint %41 %39
         %43 = OpIEqual %v2bool %42 %11
         %44 = OpSelect %v2uint %43 %11 %45
         %47 = OpBitwiseOr %v2uint %38 %44
         %48 = OpBitwiseOr %v2uint %30 %47
         %49 = OpBitwiseOr %v2uint %22 %48
         %50 = OpBitwiseOr %v2uint %14 %49
         %51 = OpIEqual %v2bool %41 %11
     %result = OpSelect %v2uint %51 %53 %50
)");
}

TEST_F(SpirvWriterTest, Builtin_FirstTrailingBit_U32) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kFirstTrailingBit, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %6 = OpBitwiseAnd %uint %arg %uint_65535
          %8 = OpIEqual %bool %6 %uint_0
         %11 = OpSelect %uint %8 %uint_16 %uint_0
         %13 = OpShiftRightLogical %uint %arg %11
         %14 = OpBitwiseAnd %uint %13 %uint_255
         %16 = OpIEqual %bool %14 %uint_0
         %17 = OpSelect %uint %16 %uint_8 %uint_0
         %19 = OpShiftRightLogical %uint %13 %17
         %20 = OpBitwiseAnd %uint %19 %uint_15
         %22 = OpIEqual %bool %20 %uint_0
         %23 = OpSelect %uint %22 %uint_4 %uint_0
         %25 = OpShiftRightLogical %uint %19 %23
         %26 = OpBitwiseAnd %uint %25 %uint_3
         %28 = OpIEqual %bool %26 %uint_0
         %29 = OpSelect %uint %28 %uint_2 %uint_0
         %31 = OpShiftRightLogical %uint %25 %29
         %32 = OpBitwiseAnd %uint %31 %uint_1
         %34 = OpIEqual %bool %32 %uint_0
         %35 = OpSelect %uint %34 %uint_1 %uint_0
         %36 = OpBitwiseOr %uint %29 %35
         %37 = OpBitwiseOr %uint %23 %36
         %38 = OpBitwiseOr %uint %17 %37
         %39 = OpBitwiseOr %uint %11 %38
         %40 = OpIEqual %bool %31 %uint_0
     %result = OpSelect %uint %40 %uint_4294967295 %39
)");
}

TEST_F(SpirvWriterTest, Builtin_FirstTrailingBit_I32) {
    auto* arg = b.FunctionParam("arg", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kFirstTrailingBit, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %7 = OpBitcast %uint %arg
          %8 = OpBitwiseAnd %uint %7 %uint_65535
         %10 = OpIEqual %bool %8 %uint_0
         %13 = OpSelect %uint %10 %uint_16 %uint_0
         %15 = OpShiftRightLogical %uint %7 %13
         %16 = OpBitwiseAnd %uint %15 %uint_255
         %18 = OpIEqual %bool %16 %uint_0
         %19 = OpSelect %uint %18 %uint_8 %uint_0
         %21 = OpShiftRightLogical %uint %15 %19
         %22 = OpBitwiseAnd %uint %21 %uint_15
         %24 = OpIEqual %bool %22 %uint_0
         %25 = OpSelect %uint %24 %uint_4 %uint_0
         %27 = OpShiftRightLogical %uint %21 %25
         %28 = OpBitwiseAnd %uint %27 %uint_3
         %30 = OpIEqual %bool %28 %uint_0
         %31 = OpSelect %uint %30 %uint_2 %uint_0
         %33 = OpShiftRightLogical %uint %27 %31
         %34 = OpBitwiseAnd %uint %33 %uint_1
         %36 = OpIEqual %bool %34 %uint_0
         %37 = OpSelect %uint %36 %uint_1 %uint_0
         %38 = OpBitwiseOr %uint %31 %37
         %39 = OpBitwiseOr %uint %25 %38
         %40 = OpBitwiseOr %uint %19 %39
         %41 = OpBitwiseOr %uint %13 %40
         %42 = OpIEqual %bool %33 %uint_0
         %43 = OpSelect %uint %42 %uint_4294967295 %41
     %result = OpBitcast %int %43
)");
}

TEST_F(SpirvWriterTest, Builtin_FirstTrailingBit_Vec2U32) {
    auto* arg = b.FunctionParam("arg", ty.vec2<u32>());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kFirstTrailingBit, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%8 = OpConstantComposite %v2uint %uint_65535 %uint_65535");
    EXPECT_INST("%11 = OpConstantNull %v2uint");
    EXPECT_INST("%15 = OpConstantComposite %v2uint %uint_16 %uint_16");
    EXPECT_INST("%19 = OpConstantComposite %v2uint %uint_255 %uint_255");
    EXPECT_INST("%23 = OpConstantComposite %v2uint %uint_8 %uint_8");
    EXPECT_INST("%27 = OpConstantComposite %v2uint %uint_15 %uint_15");
    EXPECT_INST("%31 = OpConstantComposite %v2uint %uint_4 %uint_4");
    EXPECT_INST("%35 = OpConstantComposite %v2uint %uint_3 %uint_3");
    EXPECT_INST("%39 = OpConstantComposite %v2uint %uint_2 %uint_2");
    EXPECT_INST("%43 = OpConstantComposite %v2uint %uint_1 %uint_1");
    EXPECT_INST("%53 = OpConstantComposite %v2uint %uint_4294967295 %uint_4294967295");
    EXPECT_INST(R"(
          %7 = OpBitwiseAnd %v2uint %arg %8
         %10 = OpIEqual %v2bool %7 %11
         %14 = OpSelect %v2uint %10 %15 %11
         %17 = OpShiftRightLogical %v2uint %arg %14
         %18 = OpBitwiseAnd %v2uint %17 %19
         %21 = OpIEqual %v2bool %18 %11
         %22 = OpSelect %v2uint %21 %23 %11
         %25 = OpShiftRightLogical %v2uint %17 %22
         %26 = OpBitwiseAnd %v2uint %25 %27
         %29 = OpIEqual %v2bool %26 %11
         %30 = OpSelect %v2uint %29 %31 %11
         %33 = OpShiftRightLogical %v2uint %25 %30
         %34 = OpBitwiseAnd %v2uint %33 %35
         %37 = OpIEqual %v2bool %34 %11
         %38 = OpSelect %v2uint %37 %39 %11
         %41 = OpShiftRightLogical %v2uint %33 %38
         %42 = OpBitwiseAnd %v2uint %41 %43
         %45 = OpIEqual %v2bool %42 %11
         %46 = OpSelect %v2uint %45 %43 %11
         %47 = OpBitwiseOr %v2uint %38 %46
         %48 = OpBitwiseOr %v2uint %30 %47
         %49 = OpBitwiseOr %v2uint %22 %48
         %50 = OpBitwiseOr %v2uint %14 %49
         %51 = OpIEqual %v2bool %41 %11
     %result = OpSelect %v2uint %51 %53 %50
)");
}

TEST_F(SpirvWriterTest, Builtin_Saturate_F32) {
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kSaturate, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %7 NClamp %arg %float_0 %float_1");
}

TEST_F(SpirvWriterTest, Builtin_Saturate_Vec4h) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f16>());
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f16>(), core::BuiltinFn::kSaturate, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%9 = OpConstantNull %v4half");
    EXPECT_INST(
        "%10 = OpConstantComposite %v4half %half_0x1p_0 %half_0x1p_0 %half_0x1p_0 %half_0x1p_0");
    EXPECT_INST("%result = OpExtInst %v4half %8 NClamp %arg %9 %10");
}

// Tests for builtins with the signature: T = func(T, T)
using Builtin_2arg = SpirvWriterTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_2arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(MakeScalarType(params.type), params.function, MakeScalarValue(params.type),
               MakeScalarValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
TEST_P(Builtin_2arg, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type),
               MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         Builtin_2arg,
                         testing::Values(BuiltinTestCase{kF32, core::BuiltinFn::kAtan2, "Atan2"},
                                         BuiltinTestCase{kF32, core::BuiltinFn::kMax, "NMax"},
                                         BuiltinTestCase{kI32, core::BuiltinFn::kMax, "SMax"},
                                         BuiltinTestCase{kU32, core::BuiltinFn::kMax, "UMax"},
                                         BuiltinTestCase{kF32, core::BuiltinFn::kMin, "NMin"},
                                         BuiltinTestCase{kI32, core::BuiltinFn::kMin, "SMin"},
                                         BuiltinTestCase{kU32, core::BuiltinFn::kMin, "UMin"},
                                         BuiltinTestCase{kF32, core::BuiltinFn::kPow, "Pow"},
                                         BuiltinTestCase{kF16, core::BuiltinFn::kPow, "Pow"},
                                         BuiltinTestCase{kF32, core::BuiltinFn::kStep, "Step"},
                                         BuiltinTestCase{kF16, core::BuiltinFn::kStep, "Step"}));

TEST_F(SpirvWriterTest, Builtin_Cross_vec3f) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec3<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec3<f32>());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f32>(), core::BuiltinFn::kCross, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v3float %9 Cross %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Distance_vec2f) {
    auto* arg1 = b.FunctionParam("arg1", MakeVectorType(kF32));
    auto* arg2 = b.FunctionParam("arg2", MakeVectorType(kF32));
    auto* func = b.Function("foo", MakeScalarType(kF32));
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(MakeScalarType(kF32), core::BuiltinFn::kDistance, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %9 Distance %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Distance_vec3h) {
    auto* arg1 = b.FunctionParam("arg1", MakeVectorType(kF16));
    auto* arg2 = b.FunctionParam("arg2", MakeVectorType(kF16));
    auto* func = b.Function("foo", MakeScalarType(kF16));
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(MakeScalarType(kF16), core::BuiltinFn::kDistance, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %half %9 Distance %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Dot_vec4f) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kDot, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpDot %float %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Dot_vec2i) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec2<i32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kDot, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %8 = OpCompositeExtract %int %arg1 0
          %9 = OpCompositeExtract %int %arg2 0
         %11 = OpBitcast %uint %8
         %12 = OpBitcast %uint %9
         %13 = OpIMul %uint %11 %12
         %14 = OpBitcast %int %13
         %15 = OpCompositeExtract %int %arg1 1
         %16 = OpCompositeExtract %int %arg2 1
         %17 = OpBitcast %uint %15
         %18 = OpBitcast %uint %16
         %19 = OpIMul %uint %17 %18
         %20 = OpBitcast %int %19
         %21 = OpBitcast %uint %14
         %22 = OpBitcast %uint %20
         %23 = OpIAdd %uint %21 %22
         %24 = OpBitcast %int %23
)");
}

TEST_F(SpirvWriterTest, Builtin_Dot_vec4u) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<u32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kDot, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %8 = OpCompositeExtract %uint %arg1 0
          %9 = OpCompositeExtract %uint %arg2 0
         %10 = OpIMul %uint %8 %9
         %11 = OpCompositeExtract %uint %arg1 1
         %12 = OpCompositeExtract %uint %arg2 1
         %13 = OpIMul %uint %11 %12
         %14 = OpIAdd %uint %10 %13
         %15 = OpCompositeExtract %uint %arg1 2
         %16 = OpCompositeExtract %uint %arg2 2
         %17 = OpIMul %uint %15 %16
         %18 = OpIAdd %uint %14 %17
         %19 = OpCompositeExtract %uint %arg1 3
         %20 = OpCompositeExtract %uint %arg2 3
         %21 = OpIMul %uint %19 %20
     %result = OpIAdd %uint %18 %21
)");
}

TEST_F(SpirvWriterTest, Builtin_Ldexp_F32) {
    auto* arg1 = b.FunctionParam("arg1", ty.f32());
    auto* arg2 = b.FunctionParam("arg2", ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kLdexp, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %9 Ldexp %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Ldexp_F16) {
    auto* arg1 = b.FunctionParam("arg1", ty.f16());
    auto* arg2 = b.FunctionParam("arg2", ty.i32());
    auto* func = b.Function("foo", ty.f16());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f16(), core::BuiltinFn::kLdexp, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %half %9 Ldexp %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Ldexp_Vec2_F32) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec2<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), core::BuiltinFn::kLdexp, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v2float %11 Ldexp %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Ldexp_Vec3_F16) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec3<f16>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec3<i32>());
    auto* func = b.Function("foo", ty.vec3<f16>());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f16>(), core::BuiltinFn::kLdexp, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v3half %11 Ldexp %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Reflect_F32) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec3<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec3<f32>());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f32>(), core::BuiltinFn::kReflect, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v3float %9 Reflect %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Reflect_F16) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f16>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f16>());
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f16>(), core::BuiltinFn::kReflect, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4half %9 Reflect %arg1 %arg2");
}

// Tests for builtins with the signature: T = func(T, T, T)
using Builtin_3arg = SpirvWriterTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_3arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(MakeScalarType(params.type), params.function, MakeScalarValue(params.type),
               MakeScalarValue(params.type), MakeScalarValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
TEST_P(Builtin_3arg, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type),
               MakeVectorValue(params.type), MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         Builtin_3arg,
                         testing::Values(BuiltinTestCase{kF32, core::BuiltinFn::kClamp, "NClamp"},
                                         BuiltinTestCase{kF32, core::BuiltinFn::kFma, "Fma"},
                                         BuiltinTestCase{kF16, core::BuiltinFn::kFma, "Fma"},
                                         BuiltinTestCase{kF32, core::BuiltinFn::kMix, "Mix"},
                                         BuiltinTestCase{kF16, core::BuiltinFn::kMix, "Mix"}));

TEST_F(SpirvWriterTest, Builtin_Clamp_Scalar_I32) {
    auto* value = b.FunctionParam("value", ty.i32());
    auto* low = b.FunctionParam("low", ty.i32());
    auto* high = b.FunctionParam("high", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({value, low, high});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kClamp, value, low, high);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %8 = OpExtInst %int %9 SMax %value %low
     %result = OpExtInst %int %9 SMin %8 %high
)");
}

TEST_F(SpirvWriterTest, Builtin_Clamp_Scalar_U32) {
    auto* value = b.FunctionParam("value", ty.u32());
    auto* low = b.FunctionParam("low", ty.u32());
    auto* high = b.FunctionParam("high", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({value, low, high});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kClamp, value, low, high);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %8 = OpExtInst %uint %9 UMax %value %low
     %result = OpExtInst %uint %9 UMin %8 %high
)");
}

TEST_F(SpirvWriterTest, Builtin_Clamp_Vector_I32) {
    auto* value = b.FunctionParam("value", ty.vec4<i32>());
    auto* low = b.FunctionParam("low", ty.vec4<i32>());
    auto* high = b.FunctionParam("high", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({value, low, high});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), core::BuiltinFn::kClamp, value, low, high);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpExtInst %v4int %10 SMax %value %low
     %result = OpExtInst %v4int %10 SMin %9 %high
)");
}

TEST_F(SpirvWriterTest, Builtin_Clamp_Vector_U32) {
    auto* value = b.FunctionParam("value", ty.vec2<u32>());
    auto* low = b.FunctionParam("low", ty.vec2<u32>());
    auto* high = b.FunctionParam("high", ty.vec2<u32>());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({value, low, high});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kClamp, value, low, high);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpExtInst %v2uint %10 UMax %value %low
     %result = OpExtInst %v2uint %10 UMin %9 %high
)");
}

TEST_F(SpirvWriterTest, Builtin_ExtractBits_Scalar_I32) {
    auto* arg = b.FunctionParam("arg", ty.i32());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg, offset, count});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kExtractBits, arg, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpExtInst %uint %10 UMin %offset %uint_32
         %12 = OpISub %uint %uint_32 %9
         %13 = OpExtInst %uint %10 UMin %count %12
     %result = OpBitFieldSExtract %int %arg %9 %13
)");
}

TEST_F(SpirvWriterTest, Builtin_Smoothstep_F32) {
    auto* value = b.FunctionParam("value", ty.f32());
    auto* low = b.FunctionParam("low", ty.f32());
    auto* high = b.FunctionParam("high", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value, low, high});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kSmoothstep, value, low, high);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %8 = OpFSub %float %high %value
          %9 = OpFSub %float %low %value
         %10 = OpFDiv %float %8 %9
         %11 = OpExtInst %float %12 NClamp %10 %float_0 %float_1
         %15 = OpFMul %float %float_2 %11
         %17 = OpFSub %float %float_3 %15
         %19 = OpFMul %float %11 %17
     %result = OpFMul %float %11 %19
)");
}

TEST_F(SpirvWriterTest, Builtin_Smoothstep_F16) {
    auto* value = b.FunctionParam("value", ty.f16());
    auto* low = b.FunctionParam("low", ty.f16());
    auto* high = b.FunctionParam("high", ty.f16());
    auto* func = b.Function("foo", ty.f16());
    func->SetParams({value, low, high});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f16(), core::BuiltinFn::kSmoothstep, value, low, high);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %8 = OpFSub %half %high %value
          %9 = OpFSub %half %low %value
         %10 = OpFDiv %half %8 %9
         %11 = OpExtInst %half %12 NClamp %10 %half_0x0p_0 %half_0x1p_0
         %15 = OpFMul %half %half_0x1p_1 %11
         %17 = OpFSub %half %half_0x1_8p_1 %15
         %19 = OpFMul %half %11 %17
     %result = OpFMul %half %11 %19
)");
}
TEST_F(SpirvWriterTest, Builtin_ExtractBits_Scalar_U32) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg, offset, count});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kExtractBits, arg, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %8 = OpExtInst %uint %9 UMin %offset %uint_32
         %11 = OpISub %uint %uint_32 %8
         %12 = OpExtInst %uint %9 UMin %count %11
     %result = OpBitFieldUExtract %uint %arg %8 %12
)");
}

TEST_F(SpirvWriterTest, Builtin_ExtractBits_Vector_I32) {
    auto* arg = b.FunctionParam("arg", ty.vec4<i32>());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({arg, offset, count});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), core::BuiltinFn::kExtractBits, arg, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %10 = OpExtInst %uint %11 UMin %offset %uint_32
         %13 = OpISub %uint %uint_32 %10
         %14 = OpExtInst %uint %11 UMin %count %13
     %result = OpBitFieldSExtract %v4int %arg %10 %14
)");
}

TEST_F(SpirvWriterTest, Builtin_ExtractBits_Vector_U32) {
    auto* arg = b.FunctionParam("arg", ty.vec2<u32>());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({arg, offset, count});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kExtractBits, arg, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpExtInst %uint %10 UMin %offset %uint_32
         %12 = OpISub %uint %uint_32 %9
         %13 = OpExtInst %uint %10 UMin %count %12
     %result = OpBitFieldUExtract %v2uint %arg %9 %13
)");
}

TEST_F(SpirvWriterTest, Builtin_InsertBits_Scalar_I32) {
    auto* arg = b.FunctionParam("arg", ty.i32());
    auto* newbits = b.FunctionParam("newbits", ty.i32());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg, newbits, offset, count});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kInsertBits, arg, newbits, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %10 = OpExtInst %uint %11 UMin %offset %uint_32
         %13 = OpISub %uint %uint_32 %10
         %14 = OpExtInst %uint %11 UMin %count %13
     %result = OpBitFieldInsert %int %arg %newbits %10 %14
)");
}

TEST_F(SpirvWriterTest, Builtin_InsertBits_Scalar_U32) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* newbits = b.FunctionParam("newbits", ty.u32());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg, newbits, offset, count});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kInsertBits, arg, newbits, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpExtInst %uint %10 UMin %offset %uint_32
         %12 = OpISub %uint %uint_32 %9
         %13 = OpExtInst %uint %10 UMin %count %12
     %result = OpBitFieldInsert %uint %arg %newbits %9 %13
)");
}

TEST_F(SpirvWriterTest, Builtin_InsertBits_Vector_I32) {
    auto* arg = b.FunctionParam("arg", ty.vec4<i32>());
    auto* newbits = b.FunctionParam("newbits", ty.vec4<i32>());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({arg, newbits, offset, count});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<i32>(), core::BuiltinFn::kInsertBits, arg, newbits, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %11 = OpExtInst %uint %12 UMin %offset %uint_32
         %14 = OpISub %uint %uint_32 %11
         %15 = OpExtInst %uint %12 UMin %count %14
     %result = OpBitFieldInsert %v4int %arg %newbits %11 %15
)");
}

TEST_F(SpirvWriterTest, Builtin_InsertBits_Vector_U32) {
    auto* arg = b.FunctionParam("arg", ty.vec2<u32>());
    auto* newbits = b.FunctionParam("newbits", ty.vec2<u32>());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({arg, newbits, offset, count});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec2<u32>(), core::BuiltinFn::kInsertBits, arg, newbits, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %10 = OpExtInst %uint %11 UMin %offset %uint_32
         %13 = OpISub %uint %uint_32 %10
         %14 = OpExtInst %uint %11 UMin %count %13
     %result = OpBitFieldInsert %v2uint %arg %newbits %10 %14
)");
}

TEST_F(SpirvWriterTest, Builtin_FaceForward_F32) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec3<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec3<f32>());
    auto* arg3 = b.FunctionParam("arg3", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec3<f32>());
    func->SetParams({arg1, arg2, arg3});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f32>(), core::BuiltinFn::kFaceForward, arg1, arg2, arg3);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v3float %10 FaceForward %arg1 %arg2 %arg3");
}

TEST_F(SpirvWriterTest, Builtin_FaceForward_F16) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f16>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f16>());
    auto* arg3 = b.FunctionParam("arg3", ty.vec4<f16>());
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({arg1, arg2, arg3});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f16>(), core::BuiltinFn::kFaceForward, arg1, arg2, arg3);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4half %10 FaceForward %arg1 %arg2 %arg3");
}

TEST_F(SpirvWriterTest, Builtin_Mix_VectorOperands_ScalarFactor) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* factor = b.FunctionParam("factor", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg1, arg2, factor});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kMix, arg1, arg2, factor);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%9 = OpCompositeConstruct %v4float %factor %factor %factor %factor");
    EXPECT_INST("%result = OpExtInst %v4float %11 FMix %arg1 %arg2 %9");
}

TEST_F(SpirvWriterTest, Builtin_Mix_VectorOperands_VectorFactor) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* factor = b.FunctionParam("factor", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg1, arg2, factor});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kMix, arg1, arg2, factor);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %10 FMix %arg1 %arg2 %factor");
}

TEST_F(SpirvWriterTest, Builtin_Refract_F32) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* i = b.FunctionParam("i", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg1, arg2, i});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kRefract, arg1, arg2, i);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %10 Refract %arg1 %arg2 %i");
}

TEST_F(SpirvWriterTest, Builtin_Refract_F16) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f16>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f16>());
    auto* i = b.FunctionParam("i", ty.f16());
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({arg1, arg2, i});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f16>(), core::BuiltinFn::kRefract, arg1, arg2, i);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4half %10 Refract %arg1 %arg2 %i");
}

TEST_F(SpirvWriterTest, Builtin_Select_ScalarCondition_ScalarOperands) {
    auto* argf = b.FunctionParam("argf", ty.i32());
    auto* argt = b.FunctionParam("argt", ty.i32());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kSelect, argf, argt, cond);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpSelect %int %cond %argt %argf");
}

TEST_F(SpirvWriterTest, Builtin_Select_VectorCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), core::BuiltinFn::kSelect, argf, argt, cond);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpSelect %v4int %cond %argt %argf");
}

TEST_F(SpirvWriterTest, Builtin_Select_ScalarCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), core::BuiltinFn::kSelect, argf, argt, cond);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%11 = OpCompositeConstruct %v4bool %cond %cond %cond %cond");
    EXPECT_INST("%result = OpSelect %v4int %11 %argt %argf");
}

TEST_F(SpirvWriterTest, Builtin_StorageBarrier) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpControlBarrier %uint_2 %uint_2 %uint_72");
}

TEST_F(SpirvWriterTest, Builtin_StorageBarrier_VulkanMemoryModel) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    Options opts{};
    opts.use_vulkan_memory_model = true;
    ASSERT_TRUE(Generate(opts)) << Error() << output_;
    EXPECT_INST("OpControlBarrier %uint_2 %uint_2 %uint_24648");
}

TEST_F(SpirvWriterTest, Builtin_TextureBarrier) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpControlBarrier %uint_2 %uint_2 %uint_2056");
}

TEST_F(SpirvWriterTest, Builtin_TextureBarrier_Vulkan) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    Options opts{};
    opts.use_vulkan_memory_model = true;
    ASSERT_TRUE(Generate(opts)) << Error() << output_;
    EXPECT_INST("OpControlBarrier %uint_2 %uint_2 %uint_26632");
}

TEST_F(SpirvWriterTest, Builtin_WorkgroupBarrier) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpControlBarrier %uint_2 %uint_2 %uint_264");
}

TEST_F(SpirvWriterTest, Builtin_WorkgroupBarrier_VulkanMemoryModel) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    Options opts{};
    opts.use_vulkan_memory_model = true;
    ASSERT_TRUE(Generate(opts)) << Error() << output_;
    EXPECT_INST("OpControlBarrier %uint_2 %uint_2 %uint_24840");
}

TEST_F(SpirvWriterTest, Builtin_SubgroupBallot) {
    auto* func = b.Function("foo", ty.vec4<u32>());
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<u32>(), core::BuiltinFn::kSubgroupBallot, true);
        mod.SetName(result, "result");
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpCapability GroupNonUniformBallot");
    EXPECT_INST("%result = OpGroupNonUniformBallot %v4uint %uint_3 %true");
}

TEST_F(SpirvWriterTest, Builtin_SubgroupBroadcastValueF32) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kSubgroupBroadcast, 1_f, 0_u);
        mod.SetName(result, "result");
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpCapability GroupNonUniformBallot");
    EXPECT_INST("%result = OpGroupNonUniformBroadcast %float %uint_3 %float_1 %uint_0");
}

TEST_F(SpirvWriterTest, Builtin_SubgroupBroadcastValueI32) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kSubgroupBroadcast, 1_i, 0_u);
        mod.SetName(result, "result");
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpCapability GroupNonUniformBallot");
    EXPECT_INST("%result = OpGroupNonUniformBroadcast %int %uint_3 %int_1 %uint_0");
}

TEST_F(SpirvWriterTest, Builtin_SubgroupBroadcastValueU32) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kSubgroupBroadcast, 1_u, 0_u);
        mod.SetName(result, "result");
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpCapability GroupNonUniformBallot");
    EXPECT_INST("%result = OpGroupNonUniformBroadcast %uint %uint_3 %uint_1 %uint_0");
}

TEST_F(SpirvWriterTest, Builtin_ArrayLength) {
    auto* var = b.Var("var", ty.ptr(storage, ty.runtime_array(ty.i32())));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* ptr = b.Let("ptr", var);
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, ptr);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpVariable %_ptr_StorageBuffer_var_block_tint_explicit_layout StorageBuffer");
    EXPECT_INST("%result = OpArrayLength %uint %1 0");
}

TEST_F(SpirvWriterTest, Builtin_ArrayLength_WithStruct) {
    auto* arr = ty.runtime_array(ty.i32());
    auto* str = ty.Struct(mod.symbols.New("Buffer"), {
                                                         {mod.symbols.New("a"), ty.i32()},
                                                         {mod.symbols.New("b"), ty.i32()},
                                                         {mod.symbols.New("arr"), arr},
                                                     });
    auto* var = b.Var("var", ty.ptr(storage, str));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* ptr = b.Let("ptr", b.Access(ty.ptr(storage, arr), var, 2_u));
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, ptr);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%var = OpVariable %_ptr_StorageBuffer_Buffer_tint_explicit_layout StorageBuffer");
    EXPECT_INST("%result = OpArrayLength %uint %var 2");
}

////////////////////////////////////////////////////////////////////////////////
// DP4A builtins
////////////////////////////////////////////////////////////////////////////////

TEST_F(SpirvWriterTest, Builtin_Dot4I8Packed) {
    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* arg2 = b.FunctionParam("arg2", ty.u32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kDot4I8Packed, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpCapability DotProduct
               OpCapability DotProductInput4x8BitPacked
               OpExtension "SPV_KHR_integer_dot_product"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %unused_entry_point "unused_entry_point"
               OpExecutionMode %unused_entry_point LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %arg1 "arg1"                  ; id %4
               OpName %arg2 "arg2"                  ; id %5
               OpName %result "result"              ; id %8
               OpName %unused_entry_point "unused_entry_point"  ; id %9

               ; Types, variables and constants
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
          %6 = OpTypeFunction %int %uint %uint
       %void = OpTypeVoid
         %11 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %int None %6
       %arg1 = OpFunctionParameter %uint
       %arg2 = OpFunctionParameter %uint
          %7 = OpLabel
     %result = OpSDot %int %arg1 %arg2 PackedVectorFormat4x8Bit
               OpReturnValue %result
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Builtin_Dot4U8Packed) {
    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* arg2 = b.FunctionParam("arg2", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kDot4U8Packed, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpCapability DotProduct
               OpCapability DotProductInput4x8BitPacked
               OpExtension "SPV_KHR_integer_dot_product"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %unused_entry_point "unused_entry_point"
               OpExecutionMode %unused_entry_point LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %arg1 "arg1"                  ; id %3
               OpName %arg2 "arg2"                  ; id %4
               OpName %result "result"              ; id %7
               OpName %unused_entry_point "unused_entry_point"  ; id %8

               ; Types, variables and constants
       %uint = OpTypeInt 32 0
          %5 = OpTypeFunction %uint %uint %uint
       %void = OpTypeVoid
         %10 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %uint None %5
       %arg1 = OpFunctionParameter %uint
       %arg2 = OpFunctionParameter %uint
          %6 = OpLabel
     %result = OpUDot %uint %arg1 %arg2 PackedVectorFormat4x8Bit
               OpReturnValue %result
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
