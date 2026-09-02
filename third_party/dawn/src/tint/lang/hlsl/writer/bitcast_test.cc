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

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/hlsl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, BitcastIdentityNumeric) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", 1_i);
        b.Let("bc", b.Bitcast<i32>(b.Load(a)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void main() {
  int a = int(1);
  int bc = a;
}

)");
}

TEST_F(HlslWriterTest, BitcastIdentityVec) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<f32>>(1_f, 2_f));
        b.Let("bc", b.Bitcast<vec2<f32>>(b.Load(a)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void main() {
  float2 a = float2(1.0f, 2.0f);
  float2 bc = a;
}

)");
}

TEST_F(HlslWriterTest, BitcastToFloat) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", 1_i);
        b.Let("bc", b.Bitcast<f32>(b.Load(a)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void main() {
  int a = int(1);
  float bc = asfloat(a);
}

)");
}

TEST_F(HlslWriterTest, BitcastToInt) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", 1_u);
        b.Let("bc", b.Bitcast<i32>(b.Load(a)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void main() {
  uint a = 1u;
  int bc = asint(a);
}

)");
}

TEST_F(HlslWriterTest, BitcastToUint) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", 1_i);
        b.Let("bc", b.Bitcast<u32>(b.Load(a)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void main() {
  int a = int(1);
  uint bc = asuint(a);
}

)");
}

TEST_F(HlslWriterTest, BitcastFromVec2F16) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<f16>>(1_h, 2_h));
        auto* z = b.Load(a);
        b.Let("b", b.Bitcast<i32>(z));
        b.Let("c", b.Bitcast<f32>(z));
        b.Let("d", b.Bitcast<u32>(z));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
uint tint_bitcast_from_f16_2(vector<float16_t, 2> src) {
  uint2 v = (uint2(asuint16(src)) & (65535u).xx);
  uint2 v_1 = (v << uint2(0u, 16u));
  return (v_1.x | v_1.y);
}

float tint_bitcast_from_f16_1(vector<float16_t, 2> src) {
  uint2 v_2 = (uint2(asuint16(src)) & (65535u).xx);
  uint2 v_3 = (v_2 << uint2(0u, 16u));
  return asfloat((v_3.x | v_3.y));
}

int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 v_4 = (uint2(asuint16(src)) & (65535u).xx);
  uint2 v_5 = (v_4 << uint2(0u, 16u));
  return asint((v_5.x | v_5.y));
}

void main() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  vector<float16_t, 2> v_6 = a;
  int b = tint_bitcast_from_f16(v_6);
  float c = tint_bitcast_from_f16_1(v_6);
  uint d = tint_bitcast_from_f16_2(v_6);
}

)");
}

TEST_F(HlslWriterTest, BitcastToVec2F16) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", 1_i);
        b.Let("b", b.Bitcast<vec2<f16>>(b.Load(a)));

        auto* c = b.Var("c", 1_f);
        b.Let("d", b.Bitcast<vec2<f16>>(b.Load(c)));

        auto* e = b.Var("e", 1_u);
        b.Let("f", b.Bitcast<vec2<f16>>(b.Load(e)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
vector<float16_t, 2> tint_bitcast_to_f16_2(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 2> tint_bitcast_to_f16_1(float src) {
  uint v = asuint(src);
  uint2 v_2 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_2 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 2> tint_bitcast_to_f16(int src) {
  uint v = asuint(src);
  uint2 v_3 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_3 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

void main() {
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
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        auto* z = b.Load(a);
        b.Let("b", b.Bitcast<vec2<i32>>(z));
        b.Let("c", b.Bitcast<vec2<f32>>(z));
        b.Let("d", b.Bitcast<vec2<u32>>(z));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
uint2 tint_bitcast_from_f16_2(vector<float16_t, 4> src) {
  uint4 v = (uint4(asuint16(src)) & (65535u).xxxx);
  uint4 v_1 = (v << uint4(0u, 16u, 0u, 16u));
  return uint2((v_1.x | v_1.y), (v_1.z | v_1.w));
}

float2 tint_bitcast_from_f16_1(vector<float16_t, 4> src) {
  uint4 v_2 = (uint4(asuint16(src)) & (65535u).xxxx);
  uint4 v_3 = (v_2 << uint4(0u, 16u, 0u, 16u));
  return asfloat(uint2((v_3.x | v_3.y), (v_3.z | v_3.w)));
}

int2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 v_4 = (uint4(asuint16(src)) & (65535u).xxxx);
  uint4 v_5 = (v_4 << uint4(0u, 16u, 0u, 16u));
  return asint(uint2((v_5.x | v_5.y), (v_5.z | v_5.w)));
}

void main() {
  vector<float16_t, 4> a = vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(4.0h));
  vector<float16_t, 4> v_6 = a;
  int2 b = tint_bitcast_from_f16(v_6);
  float2 c = tint_bitcast_from_f16_1(v_6);
  uint2 d = tint_bitcast_from_f16_2(v_6);
}

)");
}

TEST_F(HlslWriterTest, BitcastToVec4F16) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<i32>>(1_i, 2_i));
        b.Let("b", b.Bitcast<vec4<f16>>(b.Load(a)));

        auto* c = b.Var("c", b.Construct<vec2<f32>>(1_f, 2_f));
        b.Let("d", b.Bitcast<vec4<f16>>(b.Load(c)));

        auto* e = b.Var("e", b.Construct<vec2<u32>>(1_u, 2_u));
        b.Let("f", b.Bitcast<vec4<f16>>(b.Load(e)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
vector<float16_t, 4> tint_bitcast_to_f16_2(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16_1(float2 src) {
  uint2 v = asuint(src);
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16(int2 src) {
  uint2 v = asuint(src);
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

void main() {
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
