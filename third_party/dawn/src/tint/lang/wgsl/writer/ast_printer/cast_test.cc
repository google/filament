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
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::wgsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, EmitExpression_Cast_Scalar_F32_From_I32) {
    auto* cast = Call<f32>(1_i);
    WrapInFunction(cast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, cast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f32(1i)");
}

TEST_F(WgslASTPrinterTest, EmitExpression_Cast_Scalar_F16_From_I32) {
    Enable(wgsl::Extension::kF16);

    auto* cast = Call<f16>(1_i);
    WrapInFunction(cast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, cast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f16(1i)");
}

TEST_F(WgslASTPrinterTest, EmitExpression_Cast_Vector_F32_From_I32) {
    auto* cast = Call<vec3<f32>>(Call<vec3<i32>>(1_i, 2_i, 3_i));
    WrapInFunction(cast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, cast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "vec3<f32>(vec3<i32>(1i, 2i, 3i))");
}

TEST_F(WgslASTPrinterTest, EmitExpression_Cast_Vector_F16_From_I32) {
    Enable(wgsl::Extension::kF16);

    auto* cast = Call<vec3<f16>>(Call<vec3<i32>>(1_i, 2_i, 3_i));
    WrapInFunction(cast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, cast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "vec3<f16>(vec3<i32>(1i, 2i, 3i))");
}

}  // namespace
}  // namespace tint::wgsl::writer
