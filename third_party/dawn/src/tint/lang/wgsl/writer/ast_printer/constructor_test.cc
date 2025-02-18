// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

using ::testing::HasSubstr;

namespace tint::wgsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using WgslASTPrinterTest_Constructor = TestHelper;

TEST_F(WgslASTPrinterTest_Constructor, Bool) {
    WrapInFunction(Expr(false));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("false"));
}

TEST_F(WgslASTPrinterTest_Constructor, Int) {
    WrapInFunction(Expr(-12345_i));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("-12345"));
}

TEST_F(WgslASTPrinterTest_Constructor, UInt) {
    WrapInFunction(Expr(56779_u));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("56779u"));
}

TEST_F(WgslASTPrinterTest_Constructor, F32) {
    // Use a number close to 1<<30 but whose decimal representation ends in 0.
    WrapInFunction(Expr(f32((1 << 30) - 4)));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("1073741824.0f"));
}

TEST_F(WgslASTPrinterTest_Constructor, F16) {
    Enable(wgsl::Extension::kF16);

    // Use a number close to 1<<16 but whose decimal representation ends in 0.
    WrapInFunction(Expr(f16((1 << 15) - 8)));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("32752.0h"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_F32) {
    WrapInFunction(Call<f32>(Expr(-1.2e-5_f)));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("f32(-0.00001200000042445026f)"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_F16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Call<f16>(Expr(-1.2e-5_h)));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("f16(-0.00001198053359985352h)"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_Bool) {
    WrapInFunction(Call<bool>(true));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("bool(true)"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_Int) {
    WrapInFunction(Call<i32>(-12345_i));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("i32(-12345i)"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_Uint) {
    WrapInFunction(Call<u32>(12345_u));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("u32(12345u)"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_Vec_F32) {
    WrapInFunction(Call<vec3<f32>>(1_f, 2_f, 3_f));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("vec3<f32>(1.0f, 2.0f, 3.0f)"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_Vec_F16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Call<vec3<f16>>(1_h, 2_h, 3_h));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("vec3<f16>(1.0h, 2.0h, 3.0h)"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_Mat_F32) {
    WrapInFunction(
        Call<mat2x3<f32>>(Call<vec3<f32>>(1_f, 2_f, 3_f), Call<vec3<f32>>(3_f, 4_f, 5_f)));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("mat2x3<f32>(vec3<f32>(1.0f, 2.0f, 3.0f), "
                                        "vec3<f32>(3.0f, 4.0f, 5.0f))"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_Mat_F16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(
        Call<mat2x3<f16>>(Call<vec3<f16>>(1_h, 2_h, 3_h), Call<vec3<f16>>(3_h, 4_h, 5_h)));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("mat2x3<f16>(vec3<f16>(1.0h, 2.0h, 3.0h), "
                                        "vec3<f16>(3.0h, 4.0h, 5.0h))"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_Array) {
    WrapInFunction(Call(ty.array<vec3<f32>, 3>(), Call<vec3<f32>>(1_f, 2_f, 3_f),
                        Call<vec3<f32>>(4_f, 5_f, 6_f), Call<vec3<f32>>(7_f, 8_f, 9_f)));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(),
                HasSubstr("array<vec3<f32>, 3u>(vec3<f32>(1.0f, 2.0f, 3.0f), "
                          "vec3<f32>(4.0f, 5.0f, 6.0f), vec3<f32>(7.0f, 8.0f, 9.0f))"));
}

TEST_F(WgslASTPrinterTest_Constructor, Type_ImplicitArray) {
    WrapInFunction(Call(ty.array<Infer>(), Call<vec3<f32>>(1_f, 2_f, 3_f),
                        Call<vec3<f32>>(4_f, 5_f, 6_f), Call<vec3<f32>>(7_f, 8_f, 9_f)));

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(),
                HasSubstr("array(vec3<f32>(1.0f, 2.0f, 3.0f), "
                          "vec3<f32>(4.0f, 5.0f, 6.0f), vec3<f32>(7.0f, 8.0f, 9.0f))"));
}

}  // namespace
}  // namespace tint::wgsl::writer
