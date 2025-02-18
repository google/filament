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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/hlsl/writer/helper_test.h"

#include "gtest/gtest.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, BitcastIdentityNumeric) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", 1_i);
        b.Let("bc", b.Bitcast<i32>(b.Load(a)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int a = int(1);
  int bc = a;
}

)");
}

TEST_F(HlslWriterTest, BitcastIdentityVec) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<f32>>(1_f, 2_f));
        b.Let("bc", b.Bitcast<vec2<f32>>(b.Load(a)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float2 a = float2(1.0f, 2.0f);
  float2 bc = a;
}

)");
}

TEST_F(HlslWriterTest, BitcastToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", 1_i);
        b.Let("bc", b.Bitcast<f32>(b.Load(a)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int a = int(1);
  float bc = asfloat(a);
}

)");
}

TEST_F(HlslWriterTest, BitcastToInt) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", 1_u);
        b.Let("bc", b.Bitcast<i32>(b.Load(a)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint a = 1u;
  int bc = asint(a);
}

)");
}

TEST_F(HlslWriterTest, BitcastToUint) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", 1_i);
        b.Let("bc", b.Bitcast<u32>(b.Load(a)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int a = int(1);
  uint bc = asuint(a);
}

)");
}

TEST_F(HlslWriterTest, BitcastFromVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<f16>>(1_h, 2_h));
        auto* z = b.Load(a);
        b.Let("b", b.Bitcast<i32>(z));
        b.Let("c", b.Bitcast<f32>(z));
        b.Let("d", b.Bitcast<u32>(z));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
uint tint_bitcast_from_f16_2(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return ((r.x & 65535u) | ((r.y & 65535u) << 16u));
}

float tint_bitcast_from_f16_1(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asfloat(((r.x & 65535u) | ((r.y & 65535u) << 16u)));
}

int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asint(((r.x & 65535u) | ((r.y & 65535u) << 16u)));
}

void foo() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  vector<float16_t, 2> v = a;
  int b = tint_bitcast_from_f16(v);
  float c = tint_bitcast_from_f16_1(v);
  uint d = tint_bitcast_from_f16_2(v);
}

)");
}

TEST_F(HlslWriterTest, BitcastToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", 1_i);
        b.Let("b", b.Bitcast<vec2<f16>>(b.Load(a)));

        auto* c = b.Var("c", 1_f);
        b.Let("d", b.Bitcast<vec2<f16>>(b.Load(c)));

        auto* e = b.Var("e", 1_u);
        b.Let("f", b.Bitcast<vec2<f16>>(b.Load(e)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
vector<float16_t, 2> tint_bitcast_to_f16_2(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

vector<float16_t, 2> tint_bitcast_to_f16_1(float src) {
  uint v = asuint(src);
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_2 = float16_t(t_low);
  return vector<float16_t, 2>(v_2, float16_t(t_high));
}

vector<float16_t, 2> tint_bitcast_to_f16(int src) {
  uint v = asuint(src);
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_3 = float16_t(t_low);
  return vector<float16_t, 2>(v_3, float16_t(t_high));
}

void foo() {
  int a = int(1);
  vector<float16_t, 2> b = tint_bitcast_to_f16(a);
  float c = 1.0f;
  vector<float16_t, 2> d = tint_bitcast_to_f16_1(c);
  uint e = 1u;
  vector<float16_t, 2> f = tint_bitcast_to_f16_2(e);
}

)");
}

TEST_F(HlslWriterTest, BitcastFromVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        auto* z = b.Load(a);
        b.Let("b", b.Bitcast<vec2<i32>>(z));
        b.Let("c", b.Bitcast<vec2<f32>>(z));
        b.Let("d", b.Bitcast<vec2<u32>>(z));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
uint2 tint_bitcast_from_f16_2(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return uint2(((r.x & 65535u) | ((r.y & 65535u) << 16u)), ((r.z & 65535u) | ((r.w & 65535u) << 16u)));
}

float2 tint_bitcast_from_f16_1(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asfloat(uint2(((r.x & 65535u) | ((r.y & 65535u) << 16u)), ((r.z & 65535u) | ((r.w & 65535u) << 16u))));
}

int2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asint(uint2(((r.x & 65535u) | ((r.y & 65535u) << 16u)), ((r.z & 65535u) | ((r.w & 65535u) << 16u))));
}

void foo() {
  vector<float16_t, 4> a = vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(4.0h));
  vector<float16_t, 4> v = a;
  int2 b = tint_bitcast_from_f16(v);
  float2 c = tint_bitcast_from_f16_1(v);
  uint2 d = tint_bitcast_from_f16_2(v);
}

)");
}

TEST_F(HlslWriterTest, BitcastToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<i32>>(1_i, 2_i));
        b.Let("b", b.Bitcast<vec4<f16>>(b.Load(a)));

        auto* c = b.Var("c", b.Construct<vec2<f32>>(1_f, 2_f));
        b.Let("d", b.Bitcast<vec4<f16>>(b.Load(c)));

        auto* e = b.Var("e", b.Construct<vec2<u32>>(1_u, 2_u));
        b.Let("f", b.Bitcast<vec4<f16>>(b.Load(e)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
vector<float16_t, 4> tint_bitcast_to_f16_2(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

vector<float16_t, 4> tint_bitcast_to_f16_1(float2 src) {
  uint2 v = asuint(src);
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_4 = float16_t(t_low.x);
  float16_t v_5 = float16_t(t_high.x);
  float16_t v_6 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_4, v_5, v_6, float16_t(t_high.y));
}

vector<float16_t, 4> tint_bitcast_to_f16(int2 src) {
  uint2 v = asuint(src);
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_7 = float16_t(t_low.x);
  float16_t v_8 = float16_t(t_high.x);
  float16_t v_9 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_7, v_8, v_9, float16_t(t_high.y));
}

void foo() {
  int2 a = int2(int(1), int(2));
  vector<float16_t, 4> b = tint_bitcast_to_f16(a);
  float2 c = float2(1.0f, 2.0f);
  vector<float16_t, 4> d = tint_bitcast_to_f16_1(c);
  uint2 e = uint2(1u, 2u);
  vector<float16_t, 4> f = tint_bitcast_to_f16_2(e);
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
