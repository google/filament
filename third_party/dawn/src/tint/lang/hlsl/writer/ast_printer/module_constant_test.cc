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

#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"

namespace tint::hlsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using HlslASTPrinterTest_ModuleConstant = TestHelper;

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_AInt) {
    auto* var = GlobalConst("G", Expr(1_a));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  int l = 1;
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_AFloat) {
    auto* var = GlobalConst("G", Expr(1._a));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  float l = 1.0f;
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_i32) {
    auto* var = GlobalConst("G", Expr(1_i));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  int l = 1;
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_u32) {
    auto* var = GlobalConst("G", Expr(1_u));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  uint l = 1u;
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_f32) {
    auto* var = GlobalConst("G", Expr(1_f));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  float l = 1.0f;
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_f16) {
    Enable(wgsl::Extension::kF16);

    auto* var = GlobalConst("G", Expr(1_h));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  float16_t l = float16_t(1.0h);
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_vec3_AInt) {
    auto* var = GlobalConst("G", Call<vec3<Infer>>(1_a, 2_a, 3_a));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  int3 l = int3(1, 2, 3);
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_vec3_AFloat) {
    auto* var = GlobalConst("G", Call<vec3<Infer>>(1._a, 2._a, 3._a));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  float3 l = float3(1.0f, 2.0f, 3.0f);
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_vec3_f32) {
    auto* var = GlobalConst("G", Call<vec3<f32>>(1_f, 2_f, 3_f));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  float3 l = float3(1.0f, 2.0f, 3.0f);
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_vec3_f16) {
    Enable(wgsl::Extension::kF16);

    auto* var = GlobalConst("G", Call<vec3<f16>>(1_h, 2_h, 3_h));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  vector<float16_t, 3> l = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_mat2x3_AFloat) {
    auto* var = GlobalConst("G", Call<mat2x3<Infer>>(1._a, 2._a, 3._a, 4._a, 5._a, 6._a));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  float2x3 l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_mat2x3_f32) {
    auto* var = GlobalConst("G", Call<mat2x3<f32>>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  float2x3 l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_mat2x3_f16) {
    Enable(wgsl::Extension::kF16);

    auto* var = GlobalConst("G", Call<mat2x3<f16>>(1_h, 2_h, 3_h, 4_h, 5_h, 6_h));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  matrix<float16_t, 2, 3> l = matrix<float16_t, 2, 3>(vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 3>(float16_t(4.0h), float16_t(5.0h), float16_t(6.0h)));
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_arr_f32) {
    auto* var = GlobalConst("G", Call<array<f32, 3>>(1_f, 2_f, 3_f));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  float l[3] = {1.0f, 2.0f, 3.0f};
}
)");
}

TEST_F(HlslASTPrinterTest_ModuleConstant, Emit_GlobalConst_arr_vec2_bool) {
    auto* var = GlobalConst("G", Call<array<vec2<bool>, 3>>(         //
                                     Call<vec2<bool>>(true, false),  //
                                     Call<vec2<bool>>(false, true),  //
                                     Call<vec2<bool>>(true, true)));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(void f() {
  bool2 l[3] = {bool2(true, false), bool2(false, true), (true).xx};
}
)");
}

}  // namespace
}  // namespace tint::hlsl::writer
