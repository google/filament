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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/msl/writer/helper_test.h"
#include "src/tint/utils/text/string.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::msl::writer {
namespace {

TEST_F(MslWriterTest, Constant_Bool_True) {
    auto* c = b.Constant(true);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  bool const a = true;
}
)");
}

TEST_F(MslWriterTest, Constant_Bool_False) {
    auto* c = b.Constant(false);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  bool const a = false;
}
)");
}

TEST_F(MslWriterTest, Constant_i32) {
    auto* c = b.Constant(-12345_i);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int const a = -12345;
}
)");
}

TEST_F(MslWriterTest, Constant_u32) {
    auto* c = b.Constant(12345_u);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  uint const a = 12345u;
}
)");
}

TEST_F(MslWriterTest, Constant_u64) {
    auto* c = b.Constant(u64(UINT64_MAX));
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    // Use `Print()` as u64 types are only support after certain transforms have run.
    ASSERT_TRUE(Print()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  ulong const a = 18446744073709551615ul;
}
)");
}

TEST_F(MslWriterTest, Constant_F32) {
    auto* c = b.Constant(f32((1 << 30) - 4));
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float const a = 1073741824.0f;
}
)");
}

TEST_F(MslWriterTest, Constant_F16) {
    auto* c = b.Constant(f16((1 << 15) - 8));
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half const a = 32752.0h;
}
)");
}

TEST_F(MslWriterTest, Constant_Vector_Splat) {
    auto* c = b.Splat<vec3<f32>>(1.5_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3 const a = float3(1.5f);
}
)");
}

TEST_F(MslWriterTest, Constant_Vector_Composite) {
    auto* c = b.Composite<vec3<f32>>(1.5_f, 1.0_f, 1.5_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3 const a = float3(1.5f, 1.0f, 1.5f);
}
)");
}

TEST_F(MslWriterTest, Constant_Vector_Composite_AnyZero) {
    auto* c = b.Composite<vec3<f32>>(1.0_f, 0.0_f, 1.5_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3 const a = float3(1.0f, 0.0f, 1.5f);
}
)");
}

TEST_F(MslWriterTest, Constant_Vector_Composite_AllZero) {
    auto* c = b.Composite<vec3<f32>>(0.0_f, 0.0_f, 0.0_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3 const a = float3(0.0f);
}
)");
}

TEST_F(MslWriterTest, Constant_Matrix_Splat) {
    auto* c = b.Splat<mat3x2<f32>>(b.Splat<vec2<f32>>(1.5_f));
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3x2 const a = float3x2(float2(1.5f), float2(1.5f), float2(1.5f));
}
)");
}

TEST_F(MslWriterTest, Constant_Matrix_Composite) {
    auto* c = b.Composite<mat3x2<f32>>(        //
        b.Composite<vec2<f32>>(1.5_f, 1.0_f),  //
        b.Composite<vec2<f32>>(1.5_f, 2.0_f),  //
        b.Composite<vec2<f32>>(2.5_f, 3.5_f));
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3x2 const a = float3x2(float2(1.5f, 1.0f), float2(1.5f, 2.0f), float2(2.5f, 3.5f));
}
)");
}

TEST_F(MslWriterTest, Constant_Matrix_Composite_AnyZero) {
    auto* c = b.Composite<mat2x2<f32>>(        //
        b.Composite<vec2<f32>>(1.0_f, 0.0_f),  //
        b.Composite<vec2<f32>>(1.5_f, 2.5_f));
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float2x2 const a = float2x2(float2(1.0f, 0.0f), float2(1.5f, 2.5f));
}
)");
}

TEST_F(MslWriterTest, Constant_Matrix_Composite_AllZero) {
    auto* c = b.Composite<mat3x2<f32>>(        //
        b.Composite<vec2<f32>>(0.0_f, 0.0_f),  //
        b.Composite<vec2<f32>>(0.0_f, 0.0_f),  //
        b.Composite<vec2<f32>>(0.0_f, 0.0_f));
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3x2 const a = float3x2(float2(0.0f), float2(0.0f), float2(0.0f));
}
)");
}

TEST_F(MslWriterTest, Constant_Array_Splat) {
    auto* c = b.Splat<array<f32, 3>>(1.5_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<float, 3> const a = tint_array<float, 3>{1.5f, 1.5f, 1.5f};
}
)");
}

TEST_F(MslWriterTest, Constant_Array_Composite) {
    auto* c = b.Composite<array<f32, 3>>(1.5_f, 1.0_f, 2.0_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<float, 3> const a = tint_array<float, 3>{1.5f, 1.0f, 2.0f};
}
)");
}

TEST_F(MslWriterTest, Constant_Array_Composite_AnyZero) {
    auto* c = b.Composite<array<f32, 2>>(1.0_f, 0.0_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<float, 2> const a = tint_array<float, 2>{1.0f, 0.0f};
}
)");
}

TEST_F(MslWriterTest, Constant_Array_Composite_AllZero) {
    auto* c = b.Composite<array<f32, 3>>(0.0_f, 0.0_f, 0.0_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<float, 3> const a = tint_array<float, 3>{};
}
)");
}

TEST_F(MslWriterTest, Constant_Struct_Splat) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.f32()},
                                                  {mod.symbols.Register("b"), ty.f32()},
                                              });
    auto* c = b.Splat(s, 1.5_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct S {
  float a;
  float b;
};

void foo() {
  S const a = S{.a=1.5f, .b=1.5f};
}
)");
}

TEST_F(MslWriterTest, Constant_Struct_Composite) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.f32()},
                                                  {mod.symbols.Register("b"), ty.f32()},
                                              });
    auto* c = b.Composite(s, 1.5_f, 1.0_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct S {
  float a;
  float b;
};

void foo() {
  S const a = S{.a=1.5f, .b=1.0f};
}
)");
}

TEST_F(MslWriterTest, Constant_Struct_Composite_AnyZero) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.f32()},
                                                  {mod.symbols.Register("b"), ty.f32()},
                                              });
    auto* c = b.Composite(s, 1.0_f, 0.0_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct S {
  float a;
  float b;
};

void foo() {
  S const a = S{.a=1.0f, .b=0.0f};
}
)");
}

TEST_F(MslWriterTest, Constant_Struct_Composite_AllZero) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.f32()},
                                                  {mod.symbols.Register("b"), ty.f32()},
                                              });
    auto* c = b.Composite(s, 0.0_f, 0.0_f);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct S {
  float a;
  float b;
};

void foo() {
  S const a = S{};
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
