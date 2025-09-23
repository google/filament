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

#include "src/tint/lang/hlsl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, ConstructF32) {
    auto* f = b.Function("a", ty.f32());
    b.Append(f->Block(), [&] {
        auto* v = b.Let("v", 2_f);
        b.Return(f, b.Construct(ty.f32(), v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float a() {
  float v = 2.0f;
  return float(v);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstructF16) {
    auto* f = b.Function("a", ty.f16());
    b.Append(f->Block(), [&] {
        auto* v = b.Let("v", 2_h);
        b.Return(f, b.Construct(ty.f16(), v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float16_t a() {
  float16_t v = float16_t(2.0h);
  return float16_t(v);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstructBool) {
    auto* f = b.Function("a", ty.bool_());
    b.Append(f->Block(), [&] {
        auto* v = b.Let("v", false);
        b.Return(f, b.Construct(ty.bool_(), v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
bool a() {
  bool v = false;
  return bool(v);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstructI32) {
    auto* f = b.Function("a", ty.i32());
    b.Append(f->Block(), [&] {
        auto* v = b.Let("v", 2_i);
        b.Return(f, b.Construct(ty.i32(), v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
int a() {
  int v = int(2);
  return int(v);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstructU32) {
    auto* f = b.Function("a", ty.u32());
    b.Append(f->Block(), [&] {
        auto* v = b.Let("v", 2_u);
        b.Return(f, b.Construct(ty.u32(), v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
uint a() {
  uint v = 2u;
  return uint(v);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstructMatrix) {
    auto* f = b.Function("a", ty.mat2x2<f32>());
    b.Append(f->Block(), [&] {
        auto* v = b.Let("v", 2_f);
        b.Return(f, b.Construct(ty.mat2x2<f32>(), v, v, v, v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float2x2 a() {
  float v = 2.0f;
  float2 v_1 = float2(v, v);
  return float2x2(v_1, float2(v, v));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstructVecSingleScalarF32) {
    auto* f = b.Function("a", ty.vec3<f32>());
    b.Append(f->Block(), [&] {
        auto* v = b.Let("v", 2_f);
        b.Return(f, b.Construct(ty.vec3<f32>(), v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float3 a() {
  float v = 2.0f;
  return float3((v).xxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstructVecSingleScalarF16) {
    auto* f = b.Function("a", ty.vec3<f16>());
    b.Append(f->Block(), [&] {
        auto* v = b.Let("v", 2_h);
        b.Return(f, b.Construct(ty.vec3<f16>(), v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
vector<float16_t, 3> a() {
  float16_t v = float16_t(2.0h);
  return vector<float16_t, 3>((v).xxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstructVecSingleScalarBool) {
    auto* f = b.Function("a", ty.vec3<bool>());
    b.Append(f->Block(), [&] {
        auto* v = b.Let("v", true);
        b.Return(f, b.Construct(ty.vec3<bool>(), v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
bool3 a() {
  bool v = true;
  return bool3((v).xxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstructArray) {
    auto* f = b.ComputeFunction("a");

    b.Append(f->Block(), [&] {
        b.Var("v", b.Construct(ty.array<vec3<f32>, 3>(), b.Composite(ty.vec3<f32>(), 1_f, 2_f, 3_f),
                               b.Composite(ty.vec3<f32>(), 4_f, 5_f, 6_f),
                               b.Composite(ty.vec3<f32>(), 7_f, 8_f, 9_f)));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float3 v[3] = {float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)};
}

)");
}

TEST_F(HlslWriterTest, ConstructStruct) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), ty.vec3<i32>(), 2u, 8u, 16u, 16u,
                                         core::IOAttributes{}),
    };
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    auto* f = b.Function("a", strct);
    b.Append(f->Block(), [&] {
        b.Return(f, b.Construct(strct, 1_i, 2_f, b.Composite(ty.vec3<i32>(), 3_i, 4_i, 5_i)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct S {
  int a;
  float b;
  int3 c;
};


S a() {
  S v = {int(1), 2.0f, int3(int(3), int(4), int(5))};
  return v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
