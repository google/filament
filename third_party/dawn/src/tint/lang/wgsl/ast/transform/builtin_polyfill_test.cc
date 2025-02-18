// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/builtin_polyfill.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/direct_variable_access.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using Level = BuiltinPolyfill::Level;

using BuiltinPolyfillTest = TransformTest;

TEST_F(BuiltinPolyfillTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
}

TEST_F(BuiltinPolyfillTest, ShouldRun_OverrideCall) {
    auto* src = R"(
override x = 42.123;
override y = saturate(x);
)";

    DataMap data;
    BuiltinPolyfill::Builtins builtins;
    builtins.saturate = true;
    data.Add<BuiltinPolyfill::Config>(builtins);

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, data));
}

TEST_F(BuiltinPolyfillTest, ShouldRun_OverrideBinary) {
    auto* src = R"(
override v = 10i;
override x = 20i / v;
)";

    DataMap data;
    BuiltinPolyfill::Builtins builtins;
    builtins.int_div_mod = true;
    data.Add<BuiltinPolyfill::Config>(builtins);

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, data));
}

////////////////////////////////////////////////////////////////////////////////
// acosh
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillAcosh(Level level) {
    BuiltinPolyfill::Builtins builtins;
    builtins.acosh = level;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunAcosh) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  _ = acosh(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillAcosh(Level::kNone)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillAcosh(Level::kRangeCheck)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillAcosh(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, Acosh_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : f32 = acosh(1.0);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillAcosh(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, Acosh_Full_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : f32 = acosh(v);
}
)";

    auto* expect = R"(
fn tint_acosh(x : f32) -> f32 {
  return log((x + sqrt(((x * x) - 1))));
}

fn f() {
  let v = 1.0;
  let r : f32 = tint_acosh(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAcosh(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Acosh_Full_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : vec3<f32> = acosh(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_acosh(x : vec3<f32>) -> vec3<f32> {
  return log((x + sqrt(((x * x) - 1))));
}

fn f() {
  let v = 1.0;
  let r : vec3<f32> = tint_acosh(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAcosh(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Acosh_Range_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : f32 = acosh(v);
}
)";

    auto* expect = R"(
fn tint_acosh(x : f32) -> f32 {
  return select(acosh(x), 0.0, (x < 1.0));
}

fn f() {
  let v = 1.0;
  let r : f32 = tint_acosh(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAcosh(Level::kRangeCheck));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Acosh_Range_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : vec3<f32> = acosh(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_acosh(x : vec3<f32>) -> vec3<f32> {
  return select(acosh(x), vec3<f32>(0.0), (x < vec3<f32>(1.0)));
}

fn f() {
  let v = 1.0;
  let r : vec3<f32> = tint_acosh(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAcosh(Level::kRangeCheck));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// asinh
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillSinh() {
    BuiltinPolyfill::Builtins builtins;
    builtins.asinh = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunAsinh) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  _ = asinh(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillSinh()));
}

TEST_F(BuiltinPolyfillTest, Asinh_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : f32 = asinh(1.0);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillSinh()));
}

TEST_F(BuiltinPolyfillTest, Asinh_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : f32 = asinh(v);
}
)";

    auto* expect = R"(
fn tint_sinh(x : f32) -> f32 {
  return log((x + sqrt(((x * x) + 1))));
}

fn f() {
  let v = 1.0;
  let r : f32 = tint_sinh(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSinh());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Asinh_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : vec3<f32> = asinh(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_sinh(x : vec3<f32>) -> vec3<f32> {
  return log((x + sqrt(((x * x) + 1))));
}

fn f() {
  let v = 1.0;
  let r : vec3<f32> = tint_sinh(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSinh());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// atanh
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillAtanh(Level level) {
    BuiltinPolyfill::Builtins builtins;
    builtins.atanh = level;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunAtanh) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  _ = atanh(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillAtanh(Level::kNone)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillAtanh(Level::kRangeCheck)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillAtanh(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, Atanh_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : f32 = atanh(0.23);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillAtanh(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, Atanh_Full_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : f32 = atanh(v);
}
)";

    auto* expect = R"(
fn tint_atanh(x : f32) -> f32 {
  return (log(((1 + x) / (1 - x))) * 0.5);
}

fn f() {
  let v = 1.0;
  let r : f32 = tint_atanh(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAtanh(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Atanh_Full_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : vec3<f32> = atanh(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_atanh(x : vec3<f32>) -> vec3<f32> {
  return (log(((1 + x) / (1 - x))) * 0.5);
}

fn f() {
  let v = 1.0;
  let r : vec3<f32> = tint_atanh(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAtanh(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Atanh_Range_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : f32 = atanh(v);
}
)";

    auto* expect = R"(
fn tint_atanh(x : f32) -> f32 {
  return select(atanh(x), 0.0, (x >= 1.0));
}

fn f() {
  let v = 1.0;
  let r : f32 = tint_atanh(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAtanh(Level::kRangeCheck));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Atanh_Range_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : vec3<f32> = atanh(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_atanh(x : vec3<f32>) -> vec3<f32> {
  return select(atanh(x), vec3<f32>(0.0), (x >= vec3<f32>(1.0)));
}

fn f() {
  let v = 1.0;
  let r : vec3<f32> = tint_atanh(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAtanh(Level::kRangeCheck));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// bgra8unorm
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillBgra8unorm() {
    BuiltinPolyfill::Builtins builtins;
    builtins.bgra8unorm = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunBgra8unorm_StorageTextureVar) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_storage_3d<bgra8unorm, write>;
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillBgra8unorm()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunBgra8unorm_StorageTextureParam) {
    auto* src = R"(
fn f(tex : texture_storage_3d<bgra8unorm, write>) {
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillBgra8unorm()));
}

TEST_F(BuiltinPolyfillTest, Bgra8unorm_StorageTextureVar) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_storage_3d<bgra8unorm, write>;
)";

    auto* expect = R"(
@group(0) @binding(0) var tex : texture_storage_3d<rgba8unorm, write>;
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBgra8unorm());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Bgra8unorm_StorageTextureParam) {
    auto* src = R"(
fn f(tex : texture_storage_3d<bgra8unorm, write>) {
}
)";

    auto* expect = R"(
fn f(tex : texture_storage_3d<rgba8unorm, write>) {
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBgra8unorm());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Bgra8unorm_TextureLoad) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_storage_2d<bgra8unorm, read>;

fn f(coords : vec2<i32>) -> vec4<f32> {
  return textureLoad(tex, coords);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var tex : texture_storage_2d<rgba8unorm, read>;

fn f(coords : vec2<i32>) -> vec4<f32> {
  return textureLoad(tex, coords).bgra;
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBgra8unorm());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Bgra8unorm_TextureStore) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_storage_2d<bgra8unorm, write>;

fn f(coords : vec2<i32>, value : vec4<f32>) {
  textureStore(tex, coords, value);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var tex : texture_storage_2d<rgba8unorm, write>;

fn f(coords : vec2<i32>, value : vec4<f32>) {
  textureStore(tex, coords, value.bgra);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBgra8unorm());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Bgra8unorm_TextureStore_Param) {
    auto* src = R"(
fn f(tex : texture_storage_2d<bgra8unorm, write>, coords : vec2<i32>, value : vec4<f32>) {
  textureStore(tex, coords, value);
}
)";

    auto* expect = R"(
fn f(tex : texture_storage_2d<rgba8unorm, write>, coords : vec2<i32>, value : vec4<f32>) {
  textureStore(tex, coords, value.bgra);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBgra8unorm());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Bgra8unorm_TextureStore_WithAtanh) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_storage_2d<bgra8unorm, write>;

fn f(coords : vec2<i32>, value : vec4<f32>) {
  textureStore(tex, coords, atanh(value));
}
)";

    auto* expect = R"(
fn tint_atanh(x : vec4<f32>) -> vec4<f32> {
  return (log(((1 + x) / (1 - x))) * 0.5);
}

@group(0) @binding(0) var tex : texture_storage_2d<rgba8unorm, write>;

fn f(coords : vec2<i32>, value : vec4<f32>) {
  textureStore(tex, coords, tint_atanh(value).bgra);
}
)";

    BuiltinPolyfill::Builtins builtins;
    builtins.atanh = BuiltinPolyfill::Level::kFull;
    builtins.bgra8unorm = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);

    auto got = Run<BuiltinPolyfill>(src, std::move(data));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Bgra8unorm_TextureLoadAndStore) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_storage_2d<bgra8unorm, read_write>;

fn f(coords : vec2<i32>) {
  textureStore(tex, coords, textureLoad(tex, coords));
}
)";

    auto* expect = R"(
@group(0) @binding(0) var tex : texture_storage_2d<rgba8unorm, read_write>;

fn f(coords : vec2<i32>) {
  textureStore(tex, coords, textureLoad(tex, coords).bgra.bgra);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBgra8unorm());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// bitshiftModulo
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillBitshiftModulo() {
    BuiltinPolyfill::Builtins builtins;
    builtins.bitshift_modulo = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunBitshiftModulo_shl_scalar) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = 1i << v;
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillBitshiftModulo()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunBitshiftModulo_shl_vector) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = vec3(1i) << vec3(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillBitshiftModulo()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunBitshiftModulo_shr_scalar) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = 1i >> v;
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillBitshiftModulo()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunBitshiftModulo_shr_vector) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = vec3(1i) >> vec3(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillBitshiftModulo()));
}

TEST_F(BuiltinPolyfillTest, BitshiftModulo_shl_scalar) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = 1i << v;
}
)";

    auto* expect = R"(
fn f() {
  let v = 15u;
  let r = (1i << (v & 31));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBitshiftModulo());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, BitshiftModulo_shl_vector) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = vec3(1i) << vec3(v);
}
)";

    auto* expect = R"(
fn f() {
  let v = 15u;
  let r = (vec3(1i) << (vec3(v) & vec3<u32>(31)));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBitshiftModulo());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, BitshiftModulo_shr_scalar) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = 1i >> v;
}
)";

    auto* expect = R"(
fn f() {
  let v = 15u;
  let r = (1i >> (v & 31));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBitshiftModulo());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, BitshiftModulo_shr_vector) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = vec3(1i) >> vec3(v);
}
)";

    auto* expect = R"(
fn f() {
  let v = 15u;
  let r = (vec3(1i) >> (vec3(v) & vec3<u32>(31)));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBitshiftModulo());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// clampInteger
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillClampInteger() {
    BuiltinPolyfill::Builtins builtins;
    builtins.clamp_int = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunClampInteger_i32) {
    auto* src = R"(
fn f() {
  let v = 1i;
  _ = clamp(v, 2i, 3i);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillClampInteger()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunClampInteger_u32) {
    auto* src = R"(
fn f() {
  let v = 1u;
  _ = clamp(v, 2u, 3u);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillClampInteger()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunClampInteger_f32) {
    auto* src = R"(
fn f() {
  let v = 1f;
  _ = clamp(v, 2f, 3f);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillClampInteger()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunClampInteger_f16) {
    auto* src = R"(
enable f16;

fn f() {
  let v = 1h;
  _ = clamp(v, 2h, 3h);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillClampInteger()));
}

TEST_F(BuiltinPolyfillTest, ClampInteger_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = clamp(1i, 2i, 3i);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillClampInteger()));
}

TEST_F(BuiltinPolyfillTest, ClampInteger_i32) {
    auto* src = R"(
fn f() {
  let v = 1i;
  let r : i32 = clamp(v, 2i, 3i);
}
)";

    auto* expect = R"(
fn tint_clamp(e : i32, low : i32, high : i32) -> i32 {
  return min(max(e, low), high);
}

fn f() {
  let v = 1i;
  let r : i32 = tint_clamp(v, 2i, 3i);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillClampInteger());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ClampInteger_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1i;
  let r : vec3<i32> = clamp(vec3(v), vec3(2i), vec3(3i));
}
)";

    auto* expect =
        R"(
fn tint_clamp(e : vec3<i32>, low : vec3<i32>, high : vec3<i32>) -> vec3<i32> {
  return min(max(e, low), high);
}

fn f() {
  let v = 1i;
  let r : vec3<i32> = tint_clamp(vec3(v), vec3(2i), vec3(3i));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillClampInteger());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ClampInteger_u32) {
    auto* src = R"(
fn f() {
  let r : u32 = clamp(1u, 2u, 3u);
}
)";

    auto* expect = R"(
fn f() {
  let r : u32 = clamp(1u, 2u, 3u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillClampInteger());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ClampInteger_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 1u;
  let r : vec3<u32> = clamp(vec3(v), vec3(2u), vec3(3u));
}
)";

    auto* expect =
        R"(
fn tint_clamp(e : vec3<u32>, low : vec3<u32>, high : vec3<u32>) -> vec3<u32> {
  return min(max(e, low), high);
}

fn f() {
  let v = 1u;
  let r : vec3<u32> = tint_clamp(vec3(v), vec3(2u), vec3(3u));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillClampInteger());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// conv_f32_to_iu32
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillConvF32ToIU32() {
    BuiltinPolyfill::Builtins builtins;
    builtins.conv_f32_to_iu32 = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunConvF32ToI32) {
    auto* src = R"(
fn f() {
  let f = 42.0;
  _ = i32(f);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillConvF32ToIU32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunConvF32ToU32) {
    auto* src = R"(
fn f() {
  let f = 42.0;
  _ = u32(f);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillConvF32ToIU32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunConvVec3F32ToVec3I32) {
    auto* src = R"(
fn f() {
  let f = vec3(42.0);
  _ = vec3<i32>(f);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillConvF32ToIU32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunConvVec3F32ToVec3U32) {
    auto* src = R"(
fn f() {
  let f = vec3(42.0);
  _ = vec3<u32>(f);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillConvF32ToIU32()));
}

TEST_F(BuiltinPolyfillTest, ConvF32ToI32) {
    auto* src = R"(
fn f() {
  let f = 42.0;
  _ = i32(f);
}
)";
    auto* expect = R"(
fn tint_ftoi(v : f32) -> i32 {
  return select(2147483647, select(i32(v), -2147483648, (v < -2147483648.0)), (v <= 2147483520.0));
}

fn f() {
  let f = 42.0;
  _ = tint_ftoi(f);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillConvF32ToIU32());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ConvF32ToU32) {
    auto* src = R"(
fn f() {
  let f = 42.0;
  _ = u32(f);
}
)";
    auto* expect = R"(
fn tint_ftou(v : f32) -> u32 {
  return select(4294967295, select(u32(v), 0, (v < 0.0)), (v <= 4294967040.0));
}

fn f() {
  let f = 42.0;
  _ = tint_ftou(f);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillConvF32ToIU32());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ConvVec3F32ToVec3I32) {
    auto* src = R"(
fn f() {
  let f = vec3(42.0);
  _ = vec3<i32>(f);
}
)";
    auto* expect = R"(
fn tint_ftoi(v : vec3<f32>) -> vec3<i32> {
  return select(vec3(2147483647), select(vec3<i32>(v), vec3(-2147483648), (v < vec3(-2147483648.0))), (v <= vec3(2147483520.0)));
}

fn f() {
  let f = vec3(42.0);
  _ = tint_ftoi(f);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillConvF32ToIU32());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ConvVec3F32ToVec3U32) {
    auto* src = R"(
fn f() {
  let f = vec3(42.0);
  _ = vec3<u32>(f);
}
)";
    auto* expect = R"(
fn tint_ftou(v : vec3<f32>) -> vec3<u32> {
  return select(vec3(4294967295), select(vec3<u32>(v), vec3(0), (v < vec3(0.0))), (v <= vec3(4294967040.0)));
}

fn f() {
  let f = vec3(42.0);
  _ = tint_ftou(f);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillConvF32ToIU32());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// countLeadingZeros
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillCountLeadingZeros() {
    BuiltinPolyfill::Builtins builtins;
    builtins.count_leading_zeros = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunCountLeadingZeros) {
    auto* src = R"(
fn f() {
  let v = 15;
  _ = countLeadingZeros(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillCountLeadingZeros()));
}

TEST_F(BuiltinPolyfillTest, CountLeadingZeros_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = countLeadingZeros(15i);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillCountLeadingZeros()));
}

TEST_F(BuiltinPolyfillTest, CountLeadingZeros_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : i32 = countLeadingZeros(v);
}
)";

    auto* expect = R"(
fn tint_count_leading_zeros(v : i32) -> i32 {
  var x = u32(v);
  let b16 = select(0u, 16u, (x <= 65535u));
  x = (x << b16);
  let b8 = select(0u, 8u, (x <= 16777215u));
  x = (x << b8);
  let b4 = select(0u, 4u, (x <= 268435455u));
  x = (x << b4);
  let b2 = select(0u, 2u, (x <= 1073741823u));
  x = (x << b2);
  let b1 = select(0u, 1u, (x <= 2147483647u));
  let is_zero = select(0u, 1u, (x == 0u));
  return i32((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15i;
  let r : i32 = tint_count_leading_zeros(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountLeadingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountLeadingZeros_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : u32 = countLeadingZeros(v);
}
)";

    auto* expect = R"(
fn tint_count_leading_zeros(v : u32) -> u32 {
  var x = u32(v);
  let b16 = select(0u, 16u, (x <= 65535u));
  x = (x << b16);
  let b8 = select(0u, 8u, (x <= 16777215u));
  x = (x << b8);
  let b4 = select(0u, 4u, (x <= 268435455u));
  x = (x << b4);
  let b2 = select(0u, 2u, (x <= 1073741823u));
  x = (x << b2);
  let b1 = select(0u, 1u, (x <= 2147483647u));
  let is_zero = select(0u, 1u, (x == 0u));
  return u32((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15u;
  let r : u32 = tint_count_leading_zeros(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountLeadingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountLeadingZeros_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : vec3<i32> = countLeadingZeros(vec3<i32>(v));
}
)";

    auto* expect = R"(
fn tint_count_leading_zeros(v : vec3<i32>) -> vec3<i32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(0u), vec3<u32>(16u), (x <= vec3<u32>(65535u)));
  x = (x << b16);
  let b8 = select(vec3<u32>(0u), vec3<u32>(8u), (x <= vec3<u32>(16777215u)));
  x = (x << b8);
  let b4 = select(vec3<u32>(0u), vec3<u32>(4u), (x <= vec3<u32>(268435455u)));
  x = (x << b4);
  let b2 = select(vec3<u32>(0u), vec3<u32>(2u), (x <= vec3<u32>(1073741823u)));
  x = (x << b2);
  let b1 = select(vec3<u32>(0u), vec3<u32>(1u), (x <= vec3<u32>(2147483647u)));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(1u), (x == vec3<u32>(0u)));
  return vec3<i32>((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15i;
  let r : vec3<i32> = tint_count_leading_zeros(vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountLeadingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountLeadingZeros_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : vec3<u32> = countLeadingZeros(vec3<u32>(v));
}
)";

    auto* expect = R"(
fn tint_count_leading_zeros(v : vec3<u32>) -> vec3<u32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(0u), vec3<u32>(16u), (x <= vec3<u32>(65535u)));
  x = (x << b16);
  let b8 = select(vec3<u32>(0u), vec3<u32>(8u), (x <= vec3<u32>(16777215u)));
  x = (x << b8);
  let b4 = select(vec3<u32>(0u), vec3<u32>(4u), (x <= vec3<u32>(268435455u)));
  x = (x << b4);
  let b2 = select(vec3<u32>(0u), vec3<u32>(2u), (x <= vec3<u32>(1073741823u)));
  x = (x << b2);
  let b1 = select(vec3<u32>(0u), vec3<u32>(1u), (x <= vec3<u32>(2147483647u)));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(1u), (x == vec3<u32>(0u)));
  return vec3<u32>((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15u;
  let r : vec3<u32> = tint_count_leading_zeros(vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountLeadingZeros());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// countTrailingZeros
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillCountTrailingZeros() {
    BuiltinPolyfill::Builtins builtins;
    builtins.count_trailing_zeros = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunCountTrailingZeros) {
    auto* src = R"(
fn f() {
  let v = 15;
  _ = countTrailingZeros(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillCountTrailingZeros()));
}

TEST_F(BuiltinPolyfillTest, CountTrailingZeros_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = countTrailingZeros(15i);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillCountTrailingZeros()));
}

TEST_F(BuiltinPolyfillTest, CountTrailingZeros_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : i32 = countTrailingZeros(v);
}
)";

    auto* expect = R"(
fn tint_count_trailing_zeros(v : i32) -> i32 {
  var x = u32(v);
  let b16 = select(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  let b8 = select(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  let b4 = select(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  let b2 = select(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  let b1 = select(1u, 0u, bool((x & 1u)));
  let is_zero = select(0u, 1u, (x == 0u));
  return i32((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15i;
  let r : i32 = tint_count_trailing_zeros(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountTrailingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountTrailingZeros_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : u32 = countTrailingZeros(v);
}
)";

    auto* expect = R"(
fn tint_count_trailing_zeros(v : u32) -> u32 {
  var x = u32(v);
  let b16 = select(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  let b8 = select(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  let b4 = select(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  let b2 = select(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  let b1 = select(1u, 0u, bool((x & 1u)));
  let is_zero = select(0u, 1u, (x == 0u));
  return u32((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15u;
  let r : u32 = tint_count_trailing_zeros(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountTrailingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountTrailingZeros_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : vec3<i32> = countTrailingZeros(vec3<i32>(v));
}
)";

    auto* expect = R"(
fn tint_count_trailing_zeros(v : vec3<i32>) -> vec3<i32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(16u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(65535u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(8u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(255u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(4u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(15u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(2u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(3u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(1u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(1u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(1u), (x == vec3<u32>(0u)));
  return vec3<i32>((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15i;
  let r : vec3<i32> = tint_count_trailing_zeros(vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountTrailingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountTrailingZeros_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : vec3<u32> = countTrailingZeros(vec3<u32>(v));
}
)";

    auto* expect = R"(
fn tint_count_trailing_zeros(v : vec3<u32>) -> vec3<u32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(16u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(65535u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(8u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(255u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(4u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(15u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(2u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(3u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(1u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(1u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(1u), (x == vec3<u32>(0u)));
  return vec3<u32>((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15u;
  let r : vec3<u32> = tint_count_trailing_zeros(vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountTrailingZeros());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// extractBits
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillExtractBits(Level level) {
    BuiltinPolyfill::Builtins builtins;
    builtins.extract_bits = level;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunExtractBits) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  _ = extractBits(v, 5u, 6u);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillExtractBits(Level::kNone)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillExtractBits(Level::kClampParameters)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = countTrailingZeros(15i);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Full_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : i32 = extractBits(v, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : i32, offset : u32, count : u32) -> i32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  let shl = (32u - e);
  let shr = (shl + s);
  let shl_result = select(i32(), (v << shl), (shl < 32u));
  return select(((shl_result >> 31u) >> 1u), (shl_result >> shr), (shr < 32u));
}

fn f() {
  let v = 1234i;
  let r : i32 = tint_extract_bits(v, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Full_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : u32 = extractBits(v, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : u32, offset : u32, count : u32) -> u32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  let shl = (32u - e);
  let shr = (shl + s);
  let shl_result = select(u32(), (v << shl), (shl < 32u));
  return select(((shl_result >> 31u) >> 1u), (shl_result >> shr), (shr < 32u));
}

fn f() {
  let v = 1234u;
  let r : u32 = tint_extract_bits(v, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Full_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : vec3<i32> = extractBits(vec3<i32>(v), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : vec3<i32>, offset : u32, count : u32) -> vec3<i32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  let shl = (32u - e);
  let shr = (shl + s);
  let shl_result = select(vec3<i32>(), (v << vec3<u32>(shl)), (shl < 32u));
  return select(((shl_result >> vec3<u32>(31u)) >> vec3<u32>(1u)), (shl_result >> vec3<u32>(shr)), (shr < 32u));
}

fn f() {
  let v = 1234i;
  let r : vec3<i32> = tint_extract_bits(vec3<i32>(v), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Full_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : vec3<u32> = extractBits(vec3<u32>(v), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : vec3<u32>, offset : u32, count : u32) -> vec3<u32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  let shl = (32u - e);
  let shr = (shl + s);
  let shl_result = select(vec3<u32>(), (v << vec3<u32>(shl)), (shl < 32u));
  return select(((shl_result >> vec3<u32>(31u)) >> vec3<u32>(1u)), (shl_result >> vec3<u32>(shr)), (shr < 32u));
}

fn f() {
  let v = 1234u;
  let r : vec3<u32> = tint_extract_bits(vec3<u32>(v), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Clamp_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : i32 = extractBits(v, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : i32, offset : u32, count : u32) -> i32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return extractBits(v, s, (e - s));
}

fn f() {
  let v = 1234i;
  let r : i32 = tint_extract_bits(v, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Clamp_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : u32 = extractBits(v, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : u32, offset : u32, count : u32) -> u32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return extractBits(v, s, (e - s));
}

fn f() {
  let v = 1234u;
  let r : u32 = tint_extract_bits(v, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Clamp_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : vec3<i32> = extractBits(vec3<i32>(v), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : vec3<i32>, offset : u32, count : u32) -> vec3<i32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return extractBits(v, s, (e - s));
}

fn f() {
  let v = 1234i;
  let r : vec3<i32> = tint_extract_bits(vec3<i32>(v), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Clamp_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : vec3<u32> = extractBits(vec3<u32>(v), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : vec3<u32>, offset : u32, count : u32) -> vec3<u32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return extractBits(v, s, (e - s));
}

fn f() {
  let v = 1234u;
  let r : vec3<u32> = tint_extract_bits(vec3<u32>(v), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// firstLeadingBit
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillFirstLeadingBit() {
    BuiltinPolyfill::Builtins builtins;
    builtins.first_leading_bit = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunFirstLeadingBit) {
    auto* src = R"(
fn f() {
  let v = 15i;
  _ = firstLeadingBit(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillFirstLeadingBit()));
}

TEST_F(BuiltinPolyfillTest, FirstLeadingBit_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = firstLeadingBit(15i);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillFirstLeadingBit()));
}

TEST_F(BuiltinPolyfillTest, FirstLeadingBit_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : i32 = firstLeadingBit(v);
}
)";

    auto* expect = R"(
fn tint_first_leading_bit(v : i32) -> i32 {
  var x = select(u32(v), u32(~(v)), (v < 0i));
  let b16 = select(0u, 16u, bool((x & 4294901760u)));
  x = (x >> b16);
  let b8 = select(0u, 8u, bool((x & 65280u)));
  x = (x >> b8);
  let b4 = select(0u, 4u, bool((x & 240u)));
  x = (x >> b4);
  let b2 = select(0u, 2u, bool((x & 12u)));
  x = (x >> b2);
  let b1 = select(0u, 1u, bool((x & 2u)));
  let is_zero = select(0u, 4294967295u, (x == 0u));
  return i32((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15i;
  let r : i32 = tint_first_leading_bit(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstLeadingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstLeadingBit_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : u32 = firstLeadingBit(v);
}
)";

    auto* expect = R"(
fn tint_first_leading_bit(v : u32) -> u32 {
  var x = v;
  let b16 = select(0u, 16u, bool((x & 4294901760u)));
  x = (x >> b16);
  let b8 = select(0u, 8u, bool((x & 65280u)));
  x = (x >> b8);
  let b4 = select(0u, 4u, bool((x & 240u)));
  x = (x >> b4);
  let b2 = select(0u, 2u, bool((x & 12u)));
  x = (x >> b2);
  let b1 = select(0u, 1u, bool((x & 2u)));
  let is_zero = select(0u, 4294967295u, (x == 0u));
  return u32((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15u;
  let r : u32 = tint_first_leading_bit(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstLeadingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstLeadingBit_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : vec3<i32> = firstLeadingBit(vec3<i32>(v));
}
)";

    auto* expect = R"(
fn tint_first_leading_bit(v : vec3<i32>) -> vec3<i32> {
  var x = select(vec3<u32>(v), vec3<u32>(~(v)), (v < vec3<i32>(0i)));
  let b16 = select(vec3<u32>(0u), vec3<u32>(16u), vec3<bool>((x & vec3<u32>(4294901760u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(0u), vec3<u32>(8u), vec3<bool>((x & vec3<u32>(65280u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(0u), vec3<u32>(4u), vec3<bool>((x & vec3<u32>(240u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(0u), vec3<u32>(2u), vec3<bool>((x & vec3<u32>(12u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(0u), vec3<u32>(1u), vec3<bool>((x & vec3<u32>(2u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(4294967295u), (x == vec3<u32>(0u)));
  return vec3<i32>((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15i;
  let r : vec3<i32> = tint_first_leading_bit(vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstLeadingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstLeadingBit_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : vec3<u32> = firstLeadingBit(vec3<u32>(v));
}
)";

    auto* expect = R"(
fn tint_first_leading_bit(v : vec3<u32>) -> vec3<u32> {
  var x = v;
  let b16 = select(vec3<u32>(0u), vec3<u32>(16u), vec3<bool>((x & vec3<u32>(4294901760u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(0u), vec3<u32>(8u), vec3<bool>((x & vec3<u32>(65280u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(0u), vec3<u32>(4u), vec3<bool>((x & vec3<u32>(240u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(0u), vec3<u32>(2u), vec3<bool>((x & vec3<u32>(12u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(0u), vec3<u32>(1u), vec3<bool>((x & vec3<u32>(2u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(4294967295u), (x == vec3<u32>(0u)));
  return vec3<u32>((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15u;
  let r : vec3<u32> = tint_first_leading_bit(vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstLeadingBit());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// firstTrailingBit
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillFirstTrailingBit() {
    BuiltinPolyfill::Builtins builtins;
    builtins.first_trailing_bit = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunFirstTrailingBit) {
    auto* src = R"(
fn f() {
  let v = 15i;
  _ = firstTrailingBit(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillFirstTrailingBit()));
}

TEST_F(BuiltinPolyfillTest, FirstTrailingBit_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = firstTrailingBit(15i);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillFirstTrailingBit()));
}

TEST_F(BuiltinPolyfillTest, FirstTrailingBit_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : i32 = firstTrailingBit(v);
}
)";

    auto* expect = R"(
fn tint_first_trailing_bit(v : i32) -> i32 {
  var x = u32(v);
  let b16 = select(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  let b8 = select(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  let b4 = select(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  let b2 = select(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  let b1 = select(1u, 0u, bool((x & 1u)));
  let is_zero = select(0u, 4294967295u, (x == 0u));
  return i32((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15i;
  let r : i32 = tint_first_trailing_bit(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstTrailingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstTrailingBit_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : u32 = firstTrailingBit(v);
}
)";

    auto* expect = R"(
fn tint_first_trailing_bit(v : u32) -> u32 {
  var x = u32(v);
  let b16 = select(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  let b8 = select(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  let b4 = select(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  let b2 = select(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  let b1 = select(1u, 0u, bool((x & 1u)));
  let is_zero = select(0u, 4294967295u, (x == 0u));
  return u32((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15u;
  let r : u32 = tint_first_trailing_bit(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstTrailingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstTrailingBit_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : vec3<i32> = firstTrailingBit(vec3<i32>(v));
}
)";

    auto* expect = R"(
fn tint_first_trailing_bit(v : vec3<i32>) -> vec3<i32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(16u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(65535u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(8u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(255u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(4u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(15u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(2u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(3u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(1u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(1u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(4294967295u), (x == vec3<u32>(0u)));
  return vec3<i32>((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15i;
  let r : vec3<i32> = tint_first_trailing_bit(vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstTrailingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstTrailingBit_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : vec3<u32> = firstTrailingBit(vec3<u32>(v));
}
)";

    auto* expect = R"(
fn tint_first_trailing_bit(v : vec3<u32>) -> vec3<u32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(16u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(65535u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(8u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(255u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(4u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(15u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(2u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(3u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(1u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(1u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(4294967295u), (x == vec3<u32>(0u)));
  return vec3<u32>((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15u;
  let r : vec3<u32> = tint_first_trailing_bit(vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstTrailingBit());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// fwidthFine
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillFwidthFine() {
    BuiltinPolyfill::Builtins builtins;
    builtins.fwidth_fine = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunFwidthFine) {
    auto* src = R"(
fn f() {
  let v = 0.5f;
  _ = fwidthFine(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillFwidthFine()));
}

TEST_F(BuiltinPolyfillTest, FwidthFine_f32) {
    auto* src = R"(
fn f() {
  let v = 0.5f;
  let r : f32 = fwidthFine(v);
}
)";

    auto* expect = R"(
fn tint_fwidth_fine(v : f32) -> f32 {
  return (abs(dpdxFine(v)) + abs(dpdyFine(v)));
}

fn f() {
  let v = 0.5f;
  let r : f32 = tint_fwidth_fine(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFwidthFine());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FwidthFine_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 0.5f;
  let r : vec3<f32> = fwidthFine(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_fwidth_fine(v : vec3<f32>) -> vec3<f32> {
  return (abs(dpdxFine(v)) + abs(dpdyFine(v)));
}

fn f() {
  let v = 0.5f;
  let r : vec3<f32> = tint_fwidth_fine(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFwidthFine());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// insertBits
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillInsertBits(Level level) {
    BuiltinPolyfill::Builtins builtins;
    builtins.insert_bits = level;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunInsertBits) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  _ = insertBits(v, 5678, 5u, 6u);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillInsertBits(Level::kNone)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillInsertBits(Level::kClampParameters)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, InsertBits_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = insertBits(1234i, 5678i, 5u, 6u);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Full_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : i32 = insertBits(v, 5678, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : i32, n : i32, offset : u32, count : u32) -> i32 {
  let e = (offset + count);
  let mask = ((select(0u, (1u << offset), (offset < 32u)) - 1u) ^ (select(0u, (1u << e), (e < 32u)) - 1u));
  return ((select(i32(), (n << offset), (offset < 32u)) & i32(mask)) | (v & i32(~(mask))));
}

fn f() {
  let v = 1234i;
  let r : i32 = tint_insert_bits(v, 5678, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Full_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : u32 = insertBits(v, 5678u, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : u32, n : u32, offset : u32, count : u32) -> u32 {
  let e = (offset + count);
  let mask = ((select(0u, (1u << offset), (offset < 32u)) - 1u) ^ (select(0u, (1u << e), (e < 32u)) - 1u));
  return ((select(u32(), (n << offset), (offset < 32u)) & mask) | (v & ~(mask)));
}

fn f() {
  let v = 1234u;
  let r : u32 = tint_insert_bits(v, 5678u, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Full_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : vec3<i32> = insertBits(vec3<i32>(v), vec3<i32>(5678), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : vec3<i32>, n : vec3<i32>, offset : u32, count : u32) -> vec3<i32> {
  let e = (offset + count);
  let mask = ((select(0u, (1u << offset), (offset < 32u)) - 1u) ^ (select(0u, (1u << e), (e < 32u)) - 1u));
  return ((select(vec3<i32>(), (n << vec3<u32>(offset)), (offset < 32u)) & vec3<i32>(i32(mask))) | (v & vec3<i32>(i32(~(mask)))));
}

fn f() {
  let v = 1234i;
  let r : vec3<i32> = tint_insert_bits(vec3<i32>(v), vec3<i32>(5678), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Full_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : vec3<u32> = insertBits(vec3<u32>(v), vec3<u32>(5678u), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : vec3<u32>, n : vec3<u32>, offset : u32, count : u32) -> vec3<u32> {
  let e = (offset + count);
  let mask = ((select(0u, (1u << offset), (offset < 32u)) - 1u) ^ (select(0u, (1u << e), (e < 32u)) - 1u));
  return ((select(vec3<u32>(), (n << vec3<u32>(offset)), (offset < 32u)) & vec3<u32>(mask)) | (v & vec3<u32>(~(mask))));
}

fn f() {
  let v = 1234u;
  let r : vec3<u32> = tint_insert_bits(vec3<u32>(v), vec3<u32>(5678u), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Clamp_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : i32 = insertBits(v, 5678, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : i32, n : i32, offset : u32, count : u32) -> i32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return insertBits(v, n, s, (e - s));
}

fn f() {
  let v = 1234i;
  let r : i32 = tint_insert_bits(v, 5678, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Clamp_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : u32 = insertBits(v, 5678u, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : u32, n : u32, offset : u32, count : u32) -> u32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return insertBits(v, n, s, (e - s));
}

fn f() {
  let v = 1234u;
  let r : u32 = tint_insert_bits(v, 5678u, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Clamp_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : vec3<i32> = insertBits(vec3<i32>(v), vec3<i32>(5678), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : vec3<i32>, n : vec3<i32>, offset : u32, count : u32) -> vec3<i32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return insertBits(v, n, s, (e - s));
}

fn f() {
  let v = 1234i;
  let r : vec3<i32> = tint_insert_bits(vec3<i32>(v), vec3<i32>(5678), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Clamp_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : vec3<u32> = insertBits(vec3<u32>(v), vec3<u32>(5678u), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : vec3<u32>, n : vec3<u32>, offset : u32, count : u32) -> vec3<u32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return insertBits(v, n, s, (e - s));
}

fn f() {
  let v = 1234u;
  let r : vec3<u32> = tint_insert_bits(vec3<u32>(v), vec3<u32>(5678u), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// precise_float_mod
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillPreciseFloatMod() {
    BuiltinPolyfill::Builtins builtins;
    builtins.precise_float_mod = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunPreciseFloatMod) {
    auto* src = R"(
fn f() {
  let v = 10f;
  let x = 20f % v;
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillPreciseFloatMod()));
}

TEST_F(BuiltinPolyfillTest, PreciseFloatMod_af_f32) {
    auto* src = R"(
fn f() {
  let v = 10f;
  let x = 20.0 % v;
}
)";

    auto* expect = R"(
fn tint_float_mod(lhs : f32, rhs : f32) -> f32 {
  return (lhs - (trunc((lhs / rhs)) * rhs));
}

fn f() {
  let v = 10.0f;
  let x = tint_float_mod(20.0, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPreciseFloatMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, PreciseFloatMod_f32_af) {
    auto* src = R"(
fn f() {
  let v = 10.0;
  let x = 20f % v;
}
)";

    auto* expect = R"(
fn tint_float_mod(lhs : f32, rhs : f32) -> f32 {
  return (lhs - (trunc((lhs / rhs)) * rhs));
}

fn f() {
  let v = 10.0;
  let x = tint_float_mod(20.0f, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPreciseFloatMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, PreciseFloatMod_f32_f32) {
    auto* src = R"(
fn f() {
  let v = 10f;
  let x = 20f % v;
}
)";

    auto* expect = R"(
fn tint_float_mod(lhs : f32, rhs : f32) -> f32 {
  return (lhs - (trunc((lhs / rhs)) * rhs));
}

fn f() {
  let v = 10.0f;
  let x = tint_float_mod(20.0f, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPreciseFloatMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, PreciseFloatMod_Overloads) {
    auto* src = R"(
fn f() {
  let v = 10f;
  let x = 20f % v;
  let w = 10i;
  let y = 20i % w;
  let u = 10u;
  let z = 20u % u;
}
)";

    auto* expect = R"(
fn tint_float_mod(lhs : f32, rhs : f32) -> f32 {
  return (lhs - (trunc((lhs / rhs)) * rhs));
}

fn f() {
  let v = 10.0f;
  let x = tint_float_mod(20.0f, v);
  let w = 10i;
  let y = (20i % w);
  let u = 10u;
  let z = (20u % u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPreciseFloatMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, PreciseFloatMod_vec3_af_f32) {
    auto* src = R"(
fn f() {
  let v = 10f;
  let x = vec3(20.0) % v;
}
)";

    auto* expect = R"(
fn tint_float_mod(lhs : vec3<f32>, rhs : f32) -> vec3<f32> {
  let r = vec3<f32>(rhs);
  return (lhs - (trunc((lhs / r)) * r));
}

fn f() {
  let v = 10.0f;
  let x = tint_float_mod(vec3(20.0), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPreciseFloatMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, PreciseFloatMod_vec3_f32_af) {
    auto* src = R"(
fn f() {
  let v = 10.0;
  let x = vec3(20f) % v;
}
)";

    auto* expect = R"(
fn tint_float_mod(lhs : vec3<f32>, rhs : f32) -> vec3<f32> {
  let r = vec3<f32>(rhs);
  return (lhs - (trunc((lhs / r)) * r));
}

fn f() {
  let v = 10.0;
  let x = tint_float_mod(vec3(20.0f), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPreciseFloatMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, PreciseFloatMod_vec3_f32_f32) {
    auto* src = R"(
fn f() {
  let v = 10f;
  let x = vec3(20f) % v;
}
)";

    auto* expect = R"(
fn tint_float_mod(lhs : vec3<f32>, rhs : f32) -> vec3<f32> {
  let r = vec3<f32>(rhs);
  return (lhs - (trunc((lhs / r)) * r));
}

fn f() {
  let v = 10.0f;
  let x = tint_float_mod(vec3(20.0f), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPreciseFloatMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, PreciseFloatMod_vec3_f32_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 10f;
  let x = vec3<f32>(20f) % vec3<f32>(v);
}
)";

    auto* expect = R"(
fn tint_float_mod(lhs : vec3<f32>, rhs : vec3<f32>) -> vec3<f32> {
  return (lhs - (trunc((lhs / rhs)) * rhs));
}

fn f() {
  let v = 10.0f;
  let x = tint_float_mod(vec3<f32>(20.0f), vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPreciseFloatMod());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// int_div_mod
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillIntDivMod() {
    BuiltinPolyfill::Builtins builtins;
    builtins.int_div_mod = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunIntDiv) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i / v;
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillIntDivMod()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunIntMod) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i % v;
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillIntDivMod()));
}

TEST_F(BuiltinPolyfillTest, IntDiv_ai_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20 / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : i32, rhs : i32) -> i32 {
  return (lhs / select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))));
}

fn f() {
  let v = 10i;
  let x = tint_div(20, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_ai_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20 % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : i32, rhs : i32) -> i32 {
  let rhs_or_one = select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))));
  if (any(((u32((lhs | rhs_or_one)) & 2147483648u) != 0u))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

fn f() {
  let v = 10i;
  let x = tint_mod(20, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_i32_ai) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = v / 20;
}
)";

    auto* expect = R"(
fn tint_div(lhs : i32, rhs : i32) -> i32 {
  return (lhs / select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))));
}

fn f() {
  let v = 10i;
  let x = tint_div(v, 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_i32_ai) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = v % 20;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : i32, rhs : i32) -> i32 {
  let rhs_or_one = select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))));
  if (any(((u32((lhs | rhs_or_one)) & 2147483648u) != 0u))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

fn f() {
  let v = 10i;
  let x = tint_mod(v, 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_i32_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : i32, rhs : i32) -> i32 {
  return (lhs / select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))));
}

fn f() {
  let v = 10i;
  let x = tint_div(20i, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_i32_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : i32, rhs : i32) -> i32 {
  let rhs_or_one = select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))));
  if (any(((u32((lhs | rhs_or_one)) & 2147483648u) != 0u))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

fn f() {
  let v = 10i;
  let x = tint_mod(20i, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_ai_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20 / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : u32, rhs : u32) -> u32 {
  return (lhs / select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_div(20, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_ai_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20 % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : u32, rhs : u32) -> u32 {
  return (lhs % select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_mod(20, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_u32_ai) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = v / 20;
}
)";

    auto* expect = R"(
fn tint_div(lhs : u32, rhs : u32) -> u32 {
  return (lhs / select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_div(v, 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_u32_ai) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = v % 20;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : u32, rhs : u32) -> u32 {
  return (lhs % select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_mod(v, 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_u32_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20u / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : u32, rhs : u32) -> u32 {
  return (lhs / select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_div(20u, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_u32_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20u % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : u32, rhs : u32) -> u32 {
  return (lhs % select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_mod(20u, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_ai_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3(20) / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  return (lhs / select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(vec3(20), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_ai_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3(20) % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  let rhs_or_one = select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1)))));
  if (any(((vec3<u32>((lhs | rhs_or_one)) & vec3<u32>(2147483648u)) != vec3<u32>(0u)))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

fn f() {
  let v = 10i;
  let x = tint_mod(vec3(20), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_i32_ai) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3(v) / 20;
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  return (lhs / select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(vec3(v), 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_i32_ai) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3(v) % 20;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  let rhs_or_one = select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1)))));
  if (any(((vec3<u32>((lhs | rhs_or_one)) & vec3<u32>(2147483648u)) != vec3<u32>(0u)))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

fn f() {
  let v = 10i;
  let x = tint_mod(vec3(v), 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_i32_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3<i32>(20i) / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  return (lhs / select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(vec3<i32>(20i), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_i32_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3<i32>(20i) % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  let rhs_or_one = select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1)))));
  if (any(((vec3<u32>((lhs | rhs_or_one)) & vec3<u32>(2147483648u)) != vec3<u32>(0u)))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

fn f() {
  let v = 10i;
  let x = tint_mod(vec3<i32>(20i), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_u32_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = vec3<u32>(20u) / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<u32>, rhs : u32) -> vec3<u32> {
  let r = vec3<u32>(rhs);
  return (lhs / select(r, vec3(1), (r == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_div(vec3<u32>(20u), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_u32_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = vec3<u32>(20u) % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<u32>, rhs : u32) -> vec3<u32> {
  let r = vec3<u32>(rhs);
  return (lhs % select(r, vec3(1), (r == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_mod(vec3<u32>(20u), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_ai_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20 / vec3(v);
}
)";

    auto* expect = R"(
fn tint_div(lhs : i32, rhs : vec3<i32>) -> vec3<i32> {
  let l = vec3<i32>(lhs);
  return (l / select(rhs, vec3(1), ((rhs == vec3(0)) | ((l == vec3(-2147483648)) & (rhs == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(20, vec3(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_ai_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20 % vec3(v);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : i32, rhs : vec3<i32>) -> vec3<i32> {
  let l = vec3<i32>(lhs);
  let rhs_or_one = select(rhs, vec3(1), ((rhs == vec3(0)) | ((l == vec3(-2147483648)) & (rhs == vec3(-1)))));
  if (any(((vec3<u32>((l | rhs_or_one)) & vec3<u32>(2147483648u)) != vec3<u32>(0u)))) {
    return (l - ((l / rhs_or_one) * rhs_or_one));
  } else {
    return (l % rhs_or_one);
  }
}

fn f() {
  let v = 10i;
  let x = tint_mod(20, vec3(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_i32_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i / vec3<i32>(v);
}
)";

    auto* expect = R"(
fn tint_div(lhs : i32, rhs : vec3<i32>) -> vec3<i32> {
  let l = vec3<i32>(lhs);
  return (l / select(rhs, vec3(1), ((rhs == vec3(0)) | ((l == vec3(-2147483648)) & (rhs == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(20i, vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_i32_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i % vec3<i32>(v);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : i32, rhs : vec3<i32>) -> vec3<i32> {
  let l = vec3<i32>(lhs);
  let rhs_or_one = select(rhs, vec3(1), ((rhs == vec3(0)) | ((l == vec3(-2147483648)) & (rhs == vec3(-1)))));
  if (any(((vec3<u32>((l | rhs_or_one)) & vec3<u32>(2147483648u)) != vec3<u32>(0u)))) {
    return (l - ((l / rhs_or_one) * rhs_or_one));
  } else {
    return (l % rhs_or_one);
  }
}

fn f() {
  let v = 10i;
  let x = tint_mod(20i, vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_u32_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20u / vec3<u32>(v);
}
)";

    auto* expect = R"(
fn tint_div(lhs : u32, rhs : vec3<u32>) -> vec3<u32> {
  let l = vec3<u32>(lhs);
  return (l / select(rhs, vec3(1), (rhs == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_div(20u, vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_u32_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20u % vec3<u32>(v);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : u32, rhs : vec3<u32>) -> vec3<u32> {
  let l = vec3<u32>(lhs);
  return (l % select(rhs, vec3(1), (rhs == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_mod(20u, vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_i32_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3<i32>(20i) / vec3<i32>(v);
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<i32>, rhs : vec3<i32>) -> vec3<i32> {
  return (lhs / select(rhs, vec3(1), ((rhs == vec3(0)) | ((lhs == vec3(-2147483648)) & (rhs == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(vec3<i32>(20i), vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_i32_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3<i32>(20i) % vec3<i32>(v);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<i32>, rhs : vec3<i32>) -> vec3<i32> {
  let rhs_or_one = select(rhs, vec3(1), ((rhs == vec3(0)) | ((lhs == vec3(-2147483648)) & (rhs == vec3(-1)))));
  if (any(((vec3<u32>((lhs | rhs_or_one)) & vec3<u32>(2147483648u)) != vec3<u32>(0u)))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

fn f() {
  let v = 10i;
  let x = tint_mod(vec3<i32>(20i), vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_u32_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = vec3<u32>(20u) / vec3<u32>(v);
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<u32>, rhs : vec3<u32>) -> vec3<u32> {
  return (lhs / select(rhs, vec3(1), (rhs == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_div(vec3<u32>(20u), vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_u32_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = vec3<u32>(20u) % vec3<u32>(v);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<u32>, rhs : vec3<u32>) -> vec3<u32> {
  return (lhs % select(rhs, vec3(1), (rhs == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_mod(vec3<u32>(20u), vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// reflect for vec2<f32>
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillReflectVec2F32() {
    BuiltinPolyfill::Builtins builtins;
    builtins.reflect_vec2_f32 = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunReflect_vec2_f32) {
    auto* src = R"(
fn f() {
  let e1 = vec2<f32>(1.0f);
  let e2 = vec2<f32>(1.0f);
  let x = reflect(e1, e2);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillReflectVec2F32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunReflect_vec2_f16) {
    auto* src = R"(
enable f16;

fn f() {
  let e1 = vec2<f16>(1.0h);
  let e2 = vec2<f16>(1.0h);
  let x = reflect(e1, e2);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillReflectVec2F32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunReflect_vec3_f32) {
    auto* src = R"(
fn f() {
  let e1 = vec3<f32>(1.0f);
  let e2 = vec3<f32>(1.0f);
  let x = reflect(e1, e2);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillReflectVec2F32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunReflect_vec3_f16) {
    auto* src = R"(
enable f16;

fn f() {
  let e1 = vec3<f16>(1.0h);
  let e2 = vec3<f16>(1.0h);
  let x = reflect(e1, e2);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillReflectVec2F32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunReflect_vec4_f32) {
    auto* src = R"(
fn f() {
  let e1 = vec3<f32>(1.0f);
  let e2 = vec3<f32>(1.0f);
  let x = reflect(e1, e2);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillReflectVec2F32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunReflect_vec4_f16) {
    auto* src = R"(
enable f16;

fn f() {
  let e1 = vec3<f16>(1.0h);
  let e2 = vec3<f16>(1.0h);
  let x = reflect(e1, e2);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillReflectVec2F32()));
}

TEST_F(BuiltinPolyfillTest, Reflect_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : vec2<f32> = reflect(vec2<f32>(1.0), vec2<f32>(1.0));
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillReflectVec2F32()));
}

TEST_F(BuiltinPolyfillTest, Reflect_vec2_f32) {
    auto* src = R"(
fn f() {
  let v = 0.5f;
  let r : vec2<f32> = reflect(vec2<f32>(v), vec2<f32>(v));
}
)";

    auto* expect = R"(
fn tint_reflect(e1 : vec2<f32>, e2 : vec2<f32>) -> vec2<f32> {
  let factor = (-2.0 * dot(e1, e2));
  return (e1 + (factor * e2));
}

fn f() {
  let v = 0.5f;
  let r : vec2<f32> = tint_reflect(vec2<f32>(v), vec2<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillReflectVec2F32());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Reflect_multiple_types) {
    auto* src = R"(
enable f16;

fn f() {
  let in_f32 = 0.5f;
  let out_f32_vec2 : vec2<f32> = reflect(vec2<f32>(in_f32), vec2<f32>(in_f32));
  let out_f32_vec3 : vec3<f32> = reflect(vec3<f32>(in_f32), vec3<f32>(in_f32));
  let out_f32_vec4 : vec4<f32> = reflect(vec4<f32>(in_f32), vec4<f32>(in_f32));
  let in_f16 = 0.5h;
  let out_f16_vec2 : vec2<f16> = reflect(vec2<f16>(in_f16), vec2<f16>(in_f16));
  let out_f16_vec3 : vec3<f16> = reflect(vec3<f16>(in_f16), vec3<f16>(in_f16));
  let out_f16_vec4 : vec4<f16> = reflect(vec4<f16>(in_f16), vec4<f16>(in_f16));
}
)";

    auto* expect = R"(
enable f16;

fn tint_reflect(e1 : vec2<f32>, e2 : vec2<f32>) -> vec2<f32> {
  let factor = (-2.0 * dot(e1, e2));
  return (e1 + (factor * e2));
}

fn f() {
  let in_f32 = 0.5f;
  let out_f32_vec2 : vec2<f32> = tint_reflect(vec2<f32>(in_f32), vec2<f32>(in_f32));
  let out_f32_vec3 : vec3<f32> = reflect(vec3<f32>(in_f32), vec3<f32>(in_f32));
  let out_f32_vec4 : vec4<f32> = reflect(vec4<f32>(in_f32), vec4<f32>(in_f32));
  let in_f16 = 0.5h;
  let out_f16_vec2 : vec2<f16> = reflect(vec2<f16>(in_f16), vec2<f16>(in_f16));
  let out_f16_vec3 : vec3<f16> = reflect(vec3<f16>(in_f16), vec3<f16>(in_f16));
  let out_f16_vec4 : vec4<f16> = reflect(vec4<f16>(in_f16), vec4<f16>(in_f16));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillReflectVec2F32());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// saturate
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillSaturate() {
    BuiltinPolyfill::Builtins builtins;
    builtins.saturate = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunSaturate) {
    auto* src = R"(
fn f() {
  let v = 0.5f;
  _ = saturate(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillSaturate()));
}

TEST_F(BuiltinPolyfillTest, Saturate_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : f32 = saturate(0.5);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillSaturate()));
}

TEST_F(BuiltinPolyfillTest, Saturate_f32) {
    auto* src = R"(
fn f() {
  let v = 0.5f;
  let r : f32 = saturate(v);
}
)";

    auto* expect = R"(
fn tint_saturate(v : f32) -> f32 {
  return clamp(v, f32(0), f32(1));
}

fn f() {
  let v = 0.5f;
  let r : f32 = tint_saturate(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSaturate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Saturate_f16) {
    auto* src = R"(
enable f16;

fn f() {
  let v = 0.5h;
  let r : f16 = saturate(v);
}
)";

    auto* expect = R"(
enable f16;

fn tint_saturate(v : f16) -> f16 {
  return clamp(v, f16(0), f16(1));
}

fn f() {
  let v = 0.5h;
  let r : f16 = tint_saturate(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSaturate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Saturate_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 0.5f;
  let r : vec3<f32> = saturate(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_saturate(v : vec3<f32>) -> vec3<f32> {
  return clamp(v, vec3<f32>(0), vec3<f32>(1));
}

fn f() {
  let v = 0.5f;
  let r : vec3<f32> = tint_saturate(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSaturate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Saturate_vec3_f16) {
    auto* src = R"(
enable f16;

fn f() {
  let v = 0.5h;
  let r : vec3<f16> = saturate(vec3<f16>(v));
}
)";

    auto* expect = R"(
enable f16;

fn tint_saturate(v : vec3<f16>) -> vec3<f16> {
  return clamp(v, vec3<f16>(0), vec3<f16>(1));
}

fn f() {
  let v = 0.5h;
  let r : vec3<f16> = tint_saturate(vec3<f16>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSaturate());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// sign_int
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillSignInt() {
    BuiltinPolyfill::Builtins builtins;
    builtins.sign_int = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunSign_i32) {
    auto* src = R"(
fn f() {
  let v = 1i;
  _ = sign(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillSignInt()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunSign_f32) {
    auto* src = R"(
fn f() {
  let v = 1f;
  _ = sign(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillSignInt()));
}

TEST_F(BuiltinPolyfillTest, SignInt_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = sign(1i);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillSignInt()));
}

TEST_F(BuiltinPolyfillTest, SignInt_i32) {
    auto* src = R"(
fn f() {
  let v = 1i;
  let r : i32 = sign(v);
}
)";

    auto* expect = R"(
fn tint_sign(v : i32) -> i32 {
  return select(select(-1, 1, (v > 0)), 0, (v == 0));
}

fn f() {
  let v = 1i;
  let r : i32 = tint_sign(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSignInt());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, SignInt_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1i;
  let r : vec3<i32> = sign(vec3<i32>(v));
}
)";

    auto* expect = R"(
fn tint_sign(v : vec3<i32>) -> vec3<i32> {
  return select(select(vec3(-1), vec3(1), (v > vec3(0))), vec3(0), (v == vec3(0)));
}

fn f() {
  let v = 1i;
  let r : vec3<i32> = tint_sign(vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSignInt());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// textureSampleBaseClampToEdge
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillTextureSampleBaseClampToEdge_2d_f32() {
    BuiltinPolyfill::Builtins builtins;
    builtins.texture_sample_base_clamp_to_edge_2d_f32 = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunTextureSampleBaseClampToEdge_2d_f32) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;
@group(0) @binding(1) var s : sampler;

fn f() {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(0.5));
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillTextureSampleBaseClampToEdge_2d_f32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunTextureSampleBaseClampToEdge_external) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_external;
@group(0) @binding(1) var s : sampler;

fn f() {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(0.5));
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillTextureSampleBaseClampToEdge_2d_f32()));
}

TEST_F(BuiltinPolyfillTest, TextureSampleBaseClampToEdge_2d_f32_f32) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;
@group(0) @binding(1) var s : sampler;

fn f() {
  let r = textureSampleBaseClampToEdge(t, s, vec2<f32>(0.5));
}
)";

    auto* expect = R"(
fn tint_textureSampleBaseClampToEdge(t : texture_2d<f32>, s : sampler, coord : vec2<f32>) -> vec4<f32> {
  let dims = vec2<f32>(textureDimensions(t, 0));
  let half_texel = (vec2<f32>(0.5) / dims);
  let clamped = clamp(coord, half_texel, (1 - half_texel));
  return textureSampleLevel(t, s, clamped, 0);
}

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn f() {
  let r = tint_textureSampleBaseClampToEdge(t, s, vec2<f32>(0.5));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillTextureSampleBaseClampToEdge_2d_f32());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// workgroupUniformLoad
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillWorkgroupUniformLoad() {
    BuiltinPolyfill::Builtins builtins;
    builtins.workgroup_uniform_load = true;

    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);

    return data;
}

DataMap polyfillWorkgroupUniformLoadWithDirectVariableAccess() {
    DataMap data;

    BuiltinPolyfill::Builtins builtins;
    builtins.workgroup_uniform_load = true;
    data.Add<BuiltinPolyfill::Config>(builtins);

    DirectVariableAccess::Options options;
    data.Add<DirectVariableAccess::Config>(options);

    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunWorkgroupUniformLoad) {
    auto* src = R"(
var<workgroup> v : i32;

fn f() {
  _ = workgroupUniformLoad(&v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillWorkgroupUniformLoad()));
}

TEST_F(BuiltinPolyfillTest, WorkgroupUniformLoad_i32) {
    auto* src = R"(
var<workgroup> v : i32;

fn f() {
  let r = workgroupUniformLoad(&v);
}
)";

    auto* expect = R"(
fn tint_workgroupUniformLoad(p : ptr<workgroup, i32>) -> i32 {
  workgroupBarrier();
  let result = *(p);
  workgroupBarrier();
  return result;
}

var<workgroup> v : i32;

fn f() {
  let r = tint_workgroupUniformLoad(&(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillWorkgroupUniformLoad());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, WorkgroupUniformLoad_ComplexType) {
    auto* src = R"(
struct Inner {
  b : bool,
  v : vec4<i32>,
  m : mat3x3<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

var<workgroup> v : Outer;

fn f() {
  let r = workgroupUniformLoad(&v);
}
)";

    auto* expect = R"(
fn tint_workgroupUniformLoad(p : ptr<workgroup, Outer>) -> Outer {
  workgroupBarrier();
  let result = *(p);
  workgroupBarrier();
  return result;
}

struct Inner {
  b : bool,
  v : vec4<i32>,
  m : mat3x3<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

var<workgroup> v : Outer;

fn f() {
  let r = tint_workgroupUniformLoad(&(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillWorkgroupUniformLoad());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, WorkgroupUniformLoad_AvoidDuplicateEnables) {
    auto* src = R"(
var<workgroup> a : i32;
var<workgroup> b : u32;
var<workgroup> c : f32;

fn f() {
  let ra = workgroupUniformLoad(&a);
  let rb = workgroupUniformLoad(&b);
  let rc = workgroupUniformLoad(&c);
}
)";

    auto* expect = R"(
fn tint_workgroupUniformLoad(p : ptr<workgroup, i32>) -> i32 {
  workgroupBarrier();
  let result = *(p);
  workgroupBarrier();
  return result;
}

fn tint_workgroupUniformLoad_1(p : ptr<workgroup, u32>) -> u32 {
  workgroupBarrier();
  let result = *(p);
  workgroupBarrier();
  return result;
}

fn tint_workgroupUniformLoad_2(p : ptr<workgroup, f32>) -> f32 {
  workgroupBarrier();
  let result = *(p);
  workgroupBarrier();
  return result;
}

var<workgroup> a : i32;

var<workgroup> b : u32;

var<workgroup> c : f32;

fn f() {
  let ra = tint_workgroupUniformLoad(&(a));
  let rb = tint_workgroupUniformLoad_1(&(b));
  let rc = tint_workgroupUniformLoad_2(&(c));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillWorkgroupUniformLoad());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, WorkgroupUniformLoad_DirectVariableAccess) {
    auto* src = R"(
var<workgroup> v : i32;
var<workgroup> v2 : i32;

fn f() {
  let r = workgroupUniformLoad(&v);
  let s = workgroupUniformLoad(&v2);
}
)";

    auto* expect = R"(
fn tint_workgroupUniformLoad_v() -> i32 {
  workgroupBarrier();
  let result = v;
  workgroupBarrier();
  return result;
}

fn tint_workgroupUniformLoad_v2() -> i32 {
  workgroupBarrier();
  let result = v2;
  workgroupBarrier();
  return result;
}

var<workgroup> v : i32;

var<workgroup> v2 : i32;

fn f() {
  let r = tint_workgroupUniformLoad_v();
  let s = tint_workgroupUniformLoad_v2();
}
)";

    auto got = Run<BuiltinPolyfill, DirectVariableAccess>(
        src, polyfillWorkgroupUniformLoadWithDirectVariableAccess());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// Built-in functions in packed_4x8_integer_dot_product
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillPacked4x8IntegerDotProduct() {
    BuiltinPolyfill::Builtins builtins;
    builtins.dot_4x8_packed = true;
    builtins.pack_unpack_4x8 = true;
    builtins.pack_4xu8_clamp = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, Dot4I8Packed) {
    auto* src = R"(
fn f() {
  let v1 = 0x01020304u;
  let v2 = 0xF1F2F3F4u;
  _ = dot4I8Packed(v1, v2);
}
)";

    auto* expect = R"(
fn tint_dot4_i8_packed(a : u32, b : u32) -> i32 {
  const n = vec4<u32>(24, 16, 8, 0);
  let a_i8 = (bitcast<vec4<i32>>((vec4<u32>(a) << n)) >> vec4<u32>(24));
  let b_i8 = (bitcast<vec4<i32>>((vec4<u32>(b) << n)) >> vec4<u32>(24));
  return dot(a_i8, b_i8);
}

fn f() {
  let v1 = 16909060u;
  let v2 = 4059231220u;
  _ = tint_dot4_i8_packed(v1, v2);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPacked4x8IntegerDotProduct());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Dot4U8Packed) {
    auto* src = R"(
fn f() {
  let v1 = 0x01020304u;
  let v2 = 0xF1F2F3F4u;
  _ = dot4U8Packed(v1, v2);
}
)";

    auto* expect = R"(
fn tint_dot4_u8_packed(a : u32, b : u32) -> u32 {
  const n = vec4<u32>(24, 16, 8, 0);
  let a_u8 = ((vec4<u32>(a) >> n) & vec4<u32>(255));
  let b_u8 = ((vec4<u32>(b) >> n) & vec4<u32>(255));
  return dot(a_u8, b_u8);
}

fn f() {
  let v1 = 16909060u;
  let v2 = 4059231220u;
  _ = tint_dot4_u8_packed(v1, v2);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPacked4x8IntegerDotProduct());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Pack4xI8) {
    auto* src = R"(
fn f() {
  let v1 = vec4i(127, 128, -128, -129);
  _ = pack4xI8(v1);
}
)";

    auto* expect = R"(
fn tint_pack_4xi8(a : vec4<i32>) -> u32 {
  const n = vec4<u32>(0, 8, 16, 24);
  let a_u32 = bitcast<vec4<u32>>(a);
  let a_u8 = ((a_u32 & vec4<u32>(255)) << n);
  return dot(a_u8, vec4<u32>(1));
}

fn f() {
  let v1 = vec4i(127, 128, -(128), -(129));
  _ = tint_pack_4xi8(v1);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPacked4x8IntegerDotProduct());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Pack4xU8) {
    auto* src = R"(
fn f() {
  let v1 = vec4u(0, 254, 255, 256);
  _ = pack4xU8(v1);
}
)";

    auto* expect = R"(
fn tint_pack_4xu8(a : vec4<u32>) -> u32 {
  const n = vec4<u32>(0, 8, 16, 24);
  let a_u8 = ((a & vec4<u32>(255)) << n);
  return dot(a_u8, vec4<u32>(1));
}

fn f() {
  let v1 = vec4u(0, 254, 255, 256);
  _ = tint_pack_4xu8(v1);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPacked4x8IntegerDotProduct());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Pack4xI8Clamp) {
    auto* src = R"(
fn f() {
  let v1 = vec4i(127, 128, -128, -129);
  _ = pack4xI8Clamp(v1);
}
)";

    auto* expect = R"(
fn tint_pack_4xi8_clamp(a : vec4<i32>) -> u32 {
  const n = vec4<u32>(0, 8, 16, 24);
  let a_clamp = clamp(a, vec4<i32>(-128), vec4<i32>(127));
  let a_u32 = bitcast<vec4<u32>>(a_clamp);
  let a_u8 = ((a_u32 & vec4<u32>(255)) << n);
  return dot(a_u8, vec4<u32>(1));
}

fn f() {
  let v1 = vec4i(127, 128, -(128), -(129));
  _ = tint_pack_4xi8_clamp(v1);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPacked4x8IntegerDotProduct());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Pack4xU8Clamp) {
    auto* src = R"(
fn f() {
  let v1 = vec4u(0, 254, 255, 256);
  _ = pack4xU8Clamp(v1);
}
)";

    auto* expect = R"(
fn tint_pack_4xu8_clamp(a : vec4<u32>) -> u32 {
  const n = vec4<u32>(0, 8, 16, 24);
  let a_clamp = clamp(a, vec4<u32>(0), vec4<u32>(255));
  let a_u8 = vec4<u32>((a_clamp << n));
  return dot(a_u8, vec4<u32>(1));
}

fn f() {
  let v1 = vec4u(0, 254, 255, 256);
  _ = tint_pack_4xu8_clamp(v1);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPacked4x8IntegerDotProduct());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Unpack4xI8) {
    auto* src = R"(
fn f() {
  let v1 = u32(0x01FF02FE);
  _ = unpack4xI8(v1);
}
)";

    auto* expect = R"(
fn tint_unpack_4xi8(a : u32) -> vec4<i32> {
  const n = vec4<u32>(24, 16, 8, 0);
  let a_vec4u = vec4<u32>(a);
  let a_vec4i = bitcast<vec4<i32>>((a_vec4u << n));
  return (a_vec4i >> vec4<u32>(24));
}

fn f() {
  let v1 = u32(33489662);
  _ = tint_unpack_4xi8(v1);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPacked4x8IntegerDotProduct());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Unpack4xU8) {
    auto* src = R"(
fn f() {
  let v1 = u32(0xFF01FE02);
  _ = unpack4xU8(v1);
}
)";

    auto* expect = R"(
fn tint_unpack_4xu8(a : u32) -> vec4<u32> {
  const n = vec4<u32>(0, 8, 16, 24);
  let a_vec4u = (vec4<u32>(a) >> n);
  return (a_vec4u & vec4<u32>(255));
}

fn f() {
  let v1 = u32(4278320642);
  _ = tint_unpack_4xu8(v1);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillPacked4x8IntegerDotProduct());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// Polyfill combinations
////////////////////////////////////////////////////////////////////////////////

TEST_F(BuiltinPolyfillTest, BitshiftAndModulo) {
    auto* src = R"(
fn f(x : i32, y : u32, z : u32) {
    let l = x << (y % z);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : u32, rhs : u32) -> u32 {
  return (lhs % select(rhs, 1, (rhs == 0)));
}

fn f(x : i32, y : u32, z : u32) {
  let l = (x << (tint_mod(y, z) & 31));
}
)";

    BuiltinPolyfill::Builtins builtins;
    builtins.bitshift_modulo = true;
    builtins.int_div_mod = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);

    auto got = Run<BuiltinPolyfill>(src, std::move(data));

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
