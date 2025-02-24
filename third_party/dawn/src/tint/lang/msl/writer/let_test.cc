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

#include "src/tint/lang/msl/writer/helper_test.h"

namespace tint::msl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(MslWriterTest, LetU32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", 42_u);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  uint const l = 42u;
}
)");
}

TEST_F(MslWriterTest, LetDuplicate) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l1", 42_u);
        b.Let("l2", 42_u);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  uint const l1 = 42u;
  uint const l2 = 42u;
}
)");
}

TEST_F(MslWriterTest, LetF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", 42.0_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float const l = 42.0f;
}
)");
}

TEST_F(MslWriterTest, LetI32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", 42_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int const l = 42;
}
)");
}

TEST_F(MslWriterTest, LetF16) {
    // Enable F16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", 42_h);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half const l = 42.0h;
}
)");
}

TEST_F(MslWriterTest, LetVec3F32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite<vec3<f32>>(1_f, 2_f, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3 const l = float3(1.0f, 2.0f, 3.0f);
}
)");
}

TEST_F(MslWriterTest, LetVec3F16) {
    // Enable f16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite<vec3<f16>>(1_h, 2_h, 3_h));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half3 const l = half3(1.0h, 2.0h, 3.0h);
}
)");
}

TEST_F(MslWriterTest, LetMat2x3F32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite<mat2x3<f32>>(b.Composite<vec3<f32>>(1_f, 2_f, 3_f),
                                            b.Composite<vec3<f32>>(4_f, 5_f, 6_f)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float2x3 const l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}
)");
}

TEST_F(MslWriterTest, LetMat2x3F16) {
    // Enable f16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite<mat2x3<f16>>(b.Composite<vec3<f16>>(1_h, 2_h, 3_h),
                                            b.Composite<vec3<f16>>(4_h, 5_h, 6_h)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half2x3 const l = half2x3(half3(1.0h, 2.0h, 3.0h), half3(4.0h, 5.0h, 6.0h));
}
)");
}

TEST_F(MslWriterTest, LetArrF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite<array<f32, 3>>(1_f, 2_f, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<float, 3> const l = tint_array<float, 3>{1.0f, 2.0f, 3.0f};
}
)");
}

TEST_F(MslWriterTest, LetArrVec2Bool) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite<array<vec2<bool>, 3>>(b.Composite<vec2<bool>>(true, false),
                                                     b.Composite<vec2<bool>>(false, true),
                                                     b.Composite<vec2<bool>>(true, false)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<bool2, 3> const l = tint_array<bool2, 3>{bool2(true, false), bool2(false, true), bool2(true, false)};
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
