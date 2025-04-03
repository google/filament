// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/multiplanar_external_texture.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using MultiplanarExternalTextureTest = TransformTest;

TEST_F(MultiplanarExternalTextureTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 2}}}});

    EXPECT_FALSE(ShouldRun<MultiplanarExternalTexture>(src, data));
}

TEST_F(MultiplanarExternalTextureTest, ShouldRunHasExternalTextureAlias) {
    auto* src = R"(
alias ET = texture_external;
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 2}}}});

    EXPECT_TRUE(ShouldRun<MultiplanarExternalTexture>(src, data));
}
TEST_F(MultiplanarExternalTextureTest, ShouldRunHasExternalTextureGlobal) {
    auto* src = R"(
@group(0) @binding(0) var ext_tex : texture_external;
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 2}}}});

    EXPECT_TRUE(ShouldRun<MultiplanarExternalTexture>(src, data));
}

TEST_F(MultiplanarExternalTextureTest, ShouldRunHasExternalTextureParam) {
    auto* src = R"(
fn f(ext_tex : texture_external) {}
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 2}}}});

    EXPECT_TRUE(ShouldRun<MultiplanarExternalTexture>(src, data));
}

// Running the transform without passing in data for the new bindings should result in an error.
TEST_F(MultiplanarExternalTextureTest, ErrorNoPassedData_SampleBaseClampToEdge) {
    auto* src = R"(
@group(0) @binding(0) var s : sampler;
@group(0) @binding(1) var ext_tex : texture_external;

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return textureSampleBaseClampToEdge(ext_tex, s, coord.xy);
}
)";
    auto* expect =
        "error: missing new binding point data for "
        "tint::ast::transform::MultiplanarExternalTexture";

    auto got = Run<MultiplanarExternalTexture>(src);
    EXPECT_EQ(expect, str(got));
}

// Running the transform with incorrect binding data should result in an error.
TEST_F(MultiplanarExternalTextureTest, ErrorIncorrectBindingPont_SampleBaseClampToEdge) {
    auto* src = R"(
@group(0) @binding(0) var s : sampler;
@group(0) @binding(1) var ext_tex : texture_external;

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return textureSampleBaseClampToEdge(ext_tex, s, coord.xy);
}
)";

    auto* expect = R"(error: missing new binding points for texture_external at binding {0,1})";

    DataMap data;
    // This bindings map specifies 0,0 as the location of the texture_external,
    // which is incorrect.
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 2}}}});
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the transform works with a textureDimensions call.
TEST_F(MultiplanarExternalTextureTest, Dimensions) {
    auto* src = R"(
@group(0) @binding(0) var ext_tex : texture_external;

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  var dim : vec2<u32>;
  dim = textureDimensions(ext_tex);
  return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(1) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(2) var<uniform> ext_tex_params : ExternalTextureParams;

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  var dim : vec2<u32>;
  dim = (ext_tex_params.apparentSize + vec2<u32>(1));
  return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 2}}}});
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

TEST_F(MultiplanarExternalTextureTest, Collisions) {
    auto* src = R"(
@group(0) @binding(0) var myTexture: texture_external;

@fragment
fn fragmentMain() -> @location(0) vec4f {
  let result = textureLoad(myTexture, vec2u(1, 1));
  return vec4f(1);
})";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@internal(disable_validation__binding_point_collision) @group(0) @binding(1) var ext_tex_plane_1 : texture_2d<f32>;

@internal(disable_validation__binding_point_collision) @group(0) @binding(0) var<uniform> ext_tex_params : ExternalTextureParams;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var myTexture : texture_2d<f32>;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureLoadExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, coord : vec2<u32>, params : ExternalTextureParams) -> vec4<f32> {
  let clampedCoords = min(vec2<u32>(coord), params.apparentSize);
  let plane0_clamped = vec2<u32>(round((params.loadTransform * vec3<f32>(vec2<f32>(clampedCoords), 1))));
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureLoad(plane0, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = vec2<u32>((vec2<f32>(plane0_clamped) * params.plane1CoordFactor));
    color = vec4<f32>((vec4<f32>(textureLoad(plane0, plane0_clamped, 0).r, textureLoad(plane1, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

@fragment
fn fragmentMain() -> @location(0) vec4f {
  let result = textureLoadExternal(myTexture, ext_tex_plane_1, vec2u(1, 1), ext_tex_params);
  return vec4f(1);
}
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 0}}}},
        /* allow collisions */ true);
    auto got = Run<MultiplanarExternalTexture>(src, data);

    EXPECT_EQ(expect, str(got));
}

// Tests that the transform works with a textureDimensions call.
TEST_F(MultiplanarExternalTextureTest, Dimensions_OutOfOrder) {
    auto* src = R"(
@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  var dim : vec2<u32>;
  dim = textureDimensions(ext_tex);
  return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}

@group(0) @binding(0) var ext_tex : texture_external;
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(1) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(2) var<uniform> ext_tex_params : ExternalTextureParams;

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  var dim : vec2<u32>;
  dim = (ext_tex_params.apparentSize + vec2<u32>(1));
  return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 2}}}});
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Test that the transform works with a textureSampleBaseClampToEdge call.
TEST_F(MultiplanarExternalTextureTest, BasicTextureSampleBaseClampToEdge) {
    auto* src = R"(
@group(0) @binding(0) var s : sampler;
@group(0) @binding(1) var ext_tex : texture_external;

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return textureSampleBaseClampToEdge(ext_tex, s, coord.xy);
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

@group(0) @binding(0) var s : sampler;

@group(0) @binding(1) var ext_tex : texture_2d<f32>;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return textureSampleExternal(ext_tex, ext_tex_plane_1, s, coord.xy, ext_tex_params);
}
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 1}, {{0, 2}, {0, 3}}}});
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Test that the transform works with a textureSampleBaseClampToEdge call.
TEST_F(MultiplanarExternalTextureTest, BasicTextureSampleBaseClampToEdge_OutOfOrder) {
    auto* src = R"(
@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return textureSampleBaseClampToEdge(ext_tex, s, coord.xy);
}

@group(0) @binding(1) var ext_tex : texture_external;
@group(0) @binding(0) var s : sampler;
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return textureSampleExternal(ext_tex, ext_tex_plane_1, s, coord.xy, ext_tex_params);
}

@group(0) @binding(1) var ext_tex : texture_2d<f32>;

@group(0) @binding(0) var s : sampler;
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 1}, {{0, 2}, {0, 3}}}});
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the transform works with a textureLoad call.
TEST_F(MultiplanarExternalTextureTest, BasicTextureLoad) {
    auto* src = R"(
@group(0) @binding(0) var ext_tex : texture_external;

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  var val_signed = textureLoad(ext_tex, vec2<i32>(1));
  var val_unsigned = textureLoad(ext_tex, vec2<u32>(1));
  return val_signed + val_unsigned;
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(1) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(2) var<uniform> ext_tex_params : ExternalTextureParams;

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureLoadExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, coord : vec2<i32>, params : ExternalTextureParams) -> vec4<f32> {
  let clampedCoords = min(vec2<u32>(coord), params.apparentSize);
  let plane0_clamped = vec2<u32>(round((params.loadTransform * vec3<f32>(vec2<f32>(clampedCoords), 1))));
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureLoad(plane0, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = vec2<u32>((vec2<f32>(plane0_clamped) * params.plane1CoordFactor));
    color = vec4<f32>((vec4<f32>(textureLoad(plane0, plane0_clamped, 0).r, textureLoad(plane1, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn textureLoadExternal_1(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, coord : vec2<u32>, params : ExternalTextureParams) -> vec4<f32> {
  let clampedCoords = min(vec2<u32>(coord), params.apparentSize);
  let plane0_clamped = vec2<u32>(round((params.loadTransform * vec3<f32>(vec2<f32>(clampedCoords), 1))));
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureLoad(plane0, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = vec2<u32>((vec2<f32>(plane0_clamped) * params.plane1CoordFactor));
    color = vec4<f32>((vec4<f32>(textureLoad(plane0, plane0_clamped, 0).r, textureLoad(plane1, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  var val_signed = textureLoadExternal(ext_tex, ext_tex_plane_1, vec2<i32>(1), ext_tex_params);
  var val_unsigned = textureLoadExternal_1(ext_tex, ext_tex_plane_1, vec2<u32>(1), ext_tex_params);
  return (val_signed + val_unsigned);
}
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 2}}}});
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the transform works with a textureLoad call.
TEST_F(MultiplanarExternalTextureTest, BasicTextureLoad_OutOfOrder) {
    auto* src = R"(
@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  var val_signed = textureLoad(ext_tex, vec2<i32>(1));
  var val_unsigned = textureLoad(ext_tex, vec2<u32>(1));
  return val_signed + val_unsigned;
}

@group(0) @binding(0) var ext_tex : texture_external;
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(1) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(2) var<uniform> ext_tex_params : ExternalTextureParams;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureLoadExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, coord : vec2<i32>, params : ExternalTextureParams) -> vec4<f32> {
  let clampedCoords = min(vec2<u32>(coord), params.apparentSize);
  let plane0_clamped = vec2<u32>(round((params.loadTransform * vec3<f32>(vec2<f32>(clampedCoords), 1))));
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureLoad(plane0, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = vec2<u32>((vec2<f32>(plane0_clamped) * params.plane1CoordFactor));
    color = vec4<f32>((vec4<f32>(textureLoad(plane0, plane0_clamped, 0).r, textureLoad(plane1, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn textureLoadExternal_1(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, coord : vec2<u32>, params : ExternalTextureParams) -> vec4<f32> {
  let clampedCoords = min(vec2<u32>(coord), params.apparentSize);
  let plane0_clamped = vec2<u32>(round((params.loadTransform * vec3<f32>(vec2<f32>(clampedCoords), 1))));
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureLoad(plane0, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = vec2<u32>((vec2<f32>(plane0_clamped) * params.plane1CoordFactor));
    color = vec4<f32>((vec4<f32>(textureLoad(plane0, plane0_clamped, 0).r, textureLoad(plane1, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  var val_signed = textureLoadExternal(ext_tex, ext_tex_plane_1, vec2<i32>(1), ext_tex_params);
  var val_unsigned = textureLoadExternal_1(ext_tex, ext_tex_plane_1, vec2<u32>(1), ext_tex_params);
  return (val_signed + val_unsigned);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 2}}}});
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the transform works with both a textureSampleBaseClampToEdge and textureLoad call.
TEST_F(MultiplanarExternalTextureTest, TextureSampleBaseClampToEdgeAndTextureLoad) {
    auto* src = R"(
@group(0) @binding(0) var s : sampler;
@group(0) @binding(1) var ext_tex : texture_external;

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return textureSampleBaseClampToEdge(ext_tex, s, coord.xy) + textureLoad(ext_tex, vec2<i32>(1, 1));
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

@group(0) @binding(0) var s : sampler;

@group(0) @binding(1) var ext_tex : texture_2d<f32>;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn textureLoadExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, coord : vec2<i32>, params : ExternalTextureParams) -> vec4<f32> {
  let clampedCoords = min(vec2<u32>(coord), params.apparentSize);
  let plane0_clamped = vec2<u32>(round((params.loadTransform * vec3<f32>(vec2<f32>(clampedCoords), 1))));
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureLoad(plane0, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = vec2<u32>((vec2<f32>(plane0_clamped) * params.plane1CoordFactor));
    color = vec4<f32>((vec4<f32>(textureLoad(plane0, plane0_clamped, 0).r, textureLoad(plane1, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return (textureSampleExternal(ext_tex, ext_tex_plane_1, s, coord.xy, ext_tex_params) + textureLoadExternal(ext_tex, ext_tex_plane_1, vec2<i32>(1, 1), ext_tex_params));
}
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 1}, {{0, 2}, {0, 3}}}});
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the transform works with both a textureSampleBaseClampToEdge and textureLoad call.
TEST_F(MultiplanarExternalTextureTest, TextureSampleBaseClampToEdgeAndTextureLoad_OutOfOrder) {
    auto* src = R"(
@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return textureSampleBaseClampToEdge(ext_tex, s, coord.xy) + textureLoad(ext_tex, vec2<i32>(1, 1));
}

@group(0) @binding(0) var s : sampler;
@group(0) @binding(1) var ext_tex : texture_external;
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn textureLoadExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, coord : vec2<i32>, params : ExternalTextureParams) -> vec4<f32> {
  let clampedCoords = min(vec2<u32>(coord), params.apparentSize);
  let plane0_clamped = vec2<u32>(round((params.loadTransform * vec3<f32>(vec2<f32>(clampedCoords), 1))));
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureLoad(plane0, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = vec2<u32>((vec2<f32>(plane0_clamped) * params.plane1CoordFactor));
    color = vec4<f32>((vec4<f32>(textureLoad(plane0, plane0_clamped, 0).r, textureLoad(plane1, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return (textureSampleExternal(ext_tex, ext_tex_plane_1, s, coord.xy, ext_tex_params) + textureLoadExternal(ext_tex, ext_tex_plane_1, vec2<i32>(1, 1), ext_tex_params));
}

@group(0) @binding(0) var s : sampler;

@group(0) @binding(1) var ext_tex : texture_2d<f32>;
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 1}, {{0, 2}, {0, 3}}}});
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the transform works with many instances of texture_external.
TEST_F(MultiplanarExternalTextureTest, ManyTextureSampleBaseClampToEdge) {
    auto* src = R"(
@group(0) @binding(0) var s : sampler;
@group(0) @binding(1) var ext_tex : texture_external;
@group(0) @binding(2) var ext_tex_1 : texture_external;
@group(0) @binding(3) var ext_tex_2 : texture_external;
@group(1) @binding(0) var ext_tex_3 : texture_external;

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return textureSampleBaseClampToEdge(ext_tex, s, coord.xy) +
         textureSampleBaseClampToEdge(ext_tex_1, s, coord.xy) +
         textureSampleBaseClampToEdge(ext_tex_2, s, coord.xy) +
         textureSampleBaseClampToEdge(ext_tex_3, s, coord.xy);
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(4) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(5) var<uniform> ext_tex_params : ExternalTextureParams;

@group(0) @binding(6) var ext_tex_plane_1_1 : texture_2d<f32>;

@group(0) @binding(7) var<uniform> ext_tex_params_1 : ExternalTextureParams;

@group(0) @binding(8) var ext_tex_plane_1_2 : texture_2d<f32>;

@group(0) @binding(9) var<uniform> ext_tex_params_2 : ExternalTextureParams;

@group(1) @binding(1) var ext_tex_plane_1_3 : texture_2d<f32>;

@group(1) @binding(2) var<uniform> ext_tex_params_3 : ExternalTextureParams;

@group(0) @binding(0) var s : sampler;

@group(0) @binding(1) var ext_tex : texture_2d<f32>;

@group(0) @binding(2) var ext_tex_1 : texture_2d<f32>;

@group(0) @binding(3) var ext_tex_2 : texture_2d<f32>;

@group(1) @binding(0) var ext_tex_3 : texture_2d<f32>;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

@fragment
fn main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
  return (((textureSampleExternal(ext_tex, ext_tex_plane_1, s, coord.xy, ext_tex_params) + textureSampleExternal(ext_tex_1, ext_tex_plane_1_1, s, coord.xy, ext_tex_params_1)) + textureSampleExternal(ext_tex_2, ext_tex_plane_1_2, s, coord.xy, ext_tex_params_2)) + textureSampleExternal(ext_tex_3, ext_tex_plane_1_3, s, coord.xy, ext_tex_params_3));
}
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{
            {{0, 1}, {{0, 4}, {0, 5}}},
            {{0, 2}, {{0, 6}, {0, 7}}},
            {{0, 3}, {{0, 8}, {0, 9}}},
            {{1, 0}, {{1, 1}, {1, 2}}},
        });
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the texture_external passed as a function parameter produces the
// correct output.
TEST_F(MultiplanarExternalTextureTest, ExternalTexturePassedAsParam) {
    auto* src = R"(
fn f(t : texture_external, s : sampler) {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) var ext_tex : texture_external;
@group(0) @binding(1) var smp : sampler;

@fragment
fn main() {
  f(ext_tex, smp);
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn f(t : texture_2d<f32>, ext_tex_plane_1_1 : texture_2d<f32>, ext_tex_params_1 : ExternalTextureParams, s : sampler) {
  _ = textureSampleExternal(t, ext_tex_plane_1_1, s, vec2<f32>(1.0, 2.0), ext_tex_params_1);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

@group(0) @binding(1) var smp : sampler;

@fragment
fn main() {
  f(ext_tex, ext_tex_plane_1, ext_tex_params, smp);
}
)";
    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{
            {{0, 0}, {{0, 2}, {0, 3}}},
        });
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the texture_external passed as a function parameter produces the
// correct output.
TEST_F(MultiplanarExternalTextureTest, ExternalTexturePassedAsParam_OutOfOrder) {
    auto* src = R"(
@fragment
fn main() {
  f(ext_tex, smp);
}

fn f(t : texture_external, s : sampler) {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) var ext_tex : texture_external;
@group(0) @binding(1) var smp : sampler;
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

@fragment
fn main() {
  f(ext_tex, ext_tex_plane_1, ext_tex_params, smp);
}

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn f(t : texture_2d<f32>, ext_tex_plane_1_1 : texture_2d<f32>, ext_tex_params_1 : ExternalTextureParams, s : sampler) {
  _ = textureSampleExternal(t, ext_tex_plane_1_1, s, vec2<f32>(1.0, 2.0), ext_tex_params_1);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

@group(0) @binding(1) var smp : sampler;
)";
    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{
            {{0, 0}, {{0, 2}, {0, 3}}},
        });
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the texture_external passed as a parameter not in the first
// position produces the correct output.
TEST_F(MultiplanarExternalTextureTest, ExternalTexturePassedAsSecondParam) {
    auto* src = R"(
fn f(s : sampler, t : texture_external) {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) var ext_tex : texture_external;
@group(0) @binding(1) var smp : sampler;

@fragment
fn main() {
  f(smp, ext_tex);
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn f(s : sampler, t : texture_2d<f32>, ext_tex_plane_1_1 : texture_2d<f32>, ext_tex_params_1 : ExternalTextureParams) {
  _ = textureSampleExternal(t, ext_tex_plane_1_1, s, vec2<f32>(1.0, 2.0), ext_tex_params_1);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

@group(0) @binding(1) var smp : sampler;

@fragment
fn main() {
  f(smp, ext_tex, ext_tex_plane_1, ext_tex_params);
}
)";
    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{
            {{0, 0}, {{0, 2}, {0, 3}}},
        });
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that multiple texture_external params passed to a function produces the
// correct output.
TEST_F(MultiplanarExternalTextureTest, ExternalTexturePassedAsParamMultiple) {
    auto* src = R"(
fn f(t : texture_external, s : sampler, t2 : texture_external) {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(1.0, 2.0));
  _ = textureSampleBaseClampToEdge(t2, s, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) var ext_tex : texture_external;
@group(0) @binding(1) var smp : sampler;
@group(0) @binding(2) var ext_tex2 : texture_external;

@fragment
fn main() {
  f(ext_tex, smp, ext_tex2);
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(3) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(4) var<uniform> ext_tex_params : ExternalTextureParams;

@group(0) @binding(5) var ext_tex_plane_1_1 : texture_2d<f32>;

@group(0) @binding(6) var<uniform> ext_tex_params_1 : ExternalTextureParams;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn f(t : texture_2d<f32>, ext_tex_plane_1_2 : texture_2d<f32>, ext_tex_params_2 : ExternalTextureParams, s : sampler, t2 : texture_2d<f32>, ext_tex_plane_1_3 : texture_2d<f32>, ext_tex_params_3 : ExternalTextureParams) {
  _ = textureSampleExternal(t, ext_tex_plane_1_2, s, vec2<f32>(1.0, 2.0), ext_tex_params_2);
  _ = textureSampleExternal(t2, ext_tex_plane_1_3, s, vec2<f32>(1.0, 2.0), ext_tex_params_3);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

@group(0) @binding(1) var smp : sampler;

@group(0) @binding(2) var ext_tex2 : texture_2d<f32>;

@fragment
fn main() {
  f(ext_tex, ext_tex_plane_1, ext_tex_params, smp, ext_tex2, ext_tex_plane_1_1, ext_tex_params_1);
}
)";
    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{
            {{0, 0}, {{0, 3}, {0, 4}}},
            {{0, 2}, {{0, 5}, {0, 6}}},
        });
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that multiple texture_external params passed to a function produces the
// correct output.
TEST_F(MultiplanarExternalTextureTest, ExternalTexturePassedAsParamMultiple_OutOfOrder) {
    auto* src = R"(
@fragment
fn main() {
  f(ext_tex, smp, ext_tex2);
}

fn f(t : texture_external, s : sampler, t2 : texture_external) {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(1.0, 2.0));
  _ = textureSampleBaseClampToEdge(t2, s, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) var ext_tex : texture_external;
@group(0) @binding(1) var smp : sampler;
@group(0) @binding(2) var ext_tex2 : texture_external;

)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(3) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(4) var<uniform> ext_tex_params : ExternalTextureParams;

@group(0) @binding(5) var ext_tex_plane_1_1 : texture_2d<f32>;

@group(0) @binding(6) var<uniform> ext_tex_params_1 : ExternalTextureParams;

@fragment
fn main() {
  f(ext_tex, ext_tex_plane_1, ext_tex_params, smp, ext_tex2, ext_tex_plane_1_1, ext_tex_params_1);
}

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn f(t : texture_2d<f32>, ext_tex_plane_1_2 : texture_2d<f32>, ext_tex_params_2 : ExternalTextureParams, s : sampler, t2 : texture_2d<f32>, ext_tex_plane_1_3 : texture_2d<f32>, ext_tex_params_3 : ExternalTextureParams) {
  _ = textureSampleExternal(t, ext_tex_plane_1_2, s, vec2<f32>(1.0, 2.0), ext_tex_params_2);
  _ = textureSampleExternal(t2, ext_tex_plane_1_3, s, vec2<f32>(1.0, 2.0), ext_tex_params_3);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

@group(0) @binding(1) var smp : sampler;

@group(0) @binding(2) var ext_tex2 : texture_2d<f32>;
)";
    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{
            {{0, 0}, {{0, 3}, {0, 4}}},
            {{0, 2}, {{0, 5}, {0, 6}}},
        });
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the texture_external passed to as a parameter to multiple
// functions produces the correct output.
TEST_F(MultiplanarExternalTextureTest, ExternalTexturePassedAsParamNested) {
    auto* src = R"(
fn nested(t : texture_external, s : sampler) {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(1.0, 2.0));
}

fn f(t : texture_external, s : sampler) {
  nested(t, s);
}

@group(0) @binding(0) var ext_tex : texture_external;
@group(0) @binding(1) var smp : sampler;

@fragment
fn main() {
  f(ext_tex, smp);
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn nested(t : texture_2d<f32>, ext_tex_plane_1_1 : texture_2d<f32>, ext_tex_params_1 : ExternalTextureParams, s : sampler) {
  _ = textureSampleExternal(t, ext_tex_plane_1_1, s, vec2<f32>(1.0, 2.0), ext_tex_params_1);
}

fn f(t : texture_2d<f32>, ext_tex_plane_1_2 : texture_2d<f32>, ext_tex_params_2 : ExternalTextureParams, s : sampler) {
  nested(t, ext_tex_plane_1_2, ext_tex_params_2, s);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

@group(0) @binding(1) var smp : sampler;

@fragment
fn main() {
  f(ext_tex, ext_tex_plane_1, ext_tex_params, smp);
}
)";
    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{
            {{0, 0}, {{0, 2}, {0, 3}}},
        });
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the texture_external passed to as a parameter to multiple functions produces the
// correct output.
TEST_F(MultiplanarExternalTextureTest, ExternalTexturePassedAsParamNested_OutOfOrder) {
    auto* src = R"(
fn nested(t : texture_external, s : sampler) {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(1.0, 2.0));
}

fn f(t : texture_external, s : sampler) {
  nested(t, s);
}

@group(0) @binding(0) var ext_tex : texture_external;
@group(0) @binding(1) var smp : sampler;

@fragment
fn main() {
  f(ext_tex, smp);
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn nested(t : texture_2d<f32>, ext_tex_plane_1_1 : texture_2d<f32>, ext_tex_params_1 : ExternalTextureParams, s : sampler) {
  _ = textureSampleExternal(t, ext_tex_plane_1_1, s, vec2<f32>(1.0, 2.0), ext_tex_params_1);
}

fn f(t : texture_2d<f32>, ext_tex_plane_1_2 : texture_2d<f32>, ext_tex_params_2 : ExternalTextureParams, s : sampler) {
  nested(t, ext_tex_plane_1_2, ext_tex_params_2, s);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

@group(0) @binding(1) var smp : sampler;

@fragment
fn main() {
  f(ext_tex, ext_tex_plane_1, ext_tex_params, smp);
}
)";
    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{
            {{0, 0}, {{0, 2}, {0, 3}}},
        });
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the transform works with a function using an external texture,
// even if there's no external texture declared at module scope.
TEST_F(MultiplanarExternalTextureTest, ExternalTexturePassedAsParamWithoutGlobalDecl) {
    auto* src = R"(
fn f(ext_tex : texture_external) -> vec2<u32> {
  return textureDimensions(ext_tex);
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

fn f(ext_tex : texture_2d<f32>, ext_tex_plane_1 : texture_2d<f32>, ext_tex_params : ExternalTextureParams) -> vec2<u32> {
  return (ext_tex_params.apparentSize + vec2<u32>(1));
}
)";

    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{{{0, 0}, {{0, 1}, {0, 2}}}});
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the transform handles aliases to external textures
TEST_F(MultiplanarExternalTextureTest, ExternalTextureAlias) {
    auto* src = R"(
alias ET = texture_external;

fn f(t : ET, s : sampler) {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) var ext_tex : ET;
@group(0) @binding(1) var smp : sampler;

@fragment
fn main() {
  f(ext_tex, smp);
}
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

alias ET = texture_external;

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn f(t : texture_2d<f32>, ext_tex_plane_1_1 : texture_2d<f32>, ext_tex_params_1 : ExternalTextureParams, s : sampler) {
  _ = textureSampleExternal(t, ext_tex_plane_1_1, s, vec2<f32>(1.0, 2.0), ext_tex_params_1);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

@group(0) @binding(1) var smp : sampler;

@fragment
fn main() {
  f(ext_tex, ext_tex_plane_1, ext_tex_params, smp);
}
)";
    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{
            {{0, 0}, {{0, 2}, {0, 3}}},
        });
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Tests that the transform handles aliases to external textures
TEST_F(MultiplanarExternalTextureTest, ExternalTextureAlias_OutOfOrder) {
    auto* src = R"(
@fragment
fn main() {
  f(ext_tex, smp);
}

fn f(t : ET, s : sampler) {
  _ = textureSampleBaseClampToEdge(t, s, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) var ext_tex : ET;
@group(0) @binding(1) var smp : sampler;

alias ET = texture_external;
)";

    auto* expect = R"(
struct GammaTransferParams {
  G : f32,
  A : f32,
  B : f32,
  C : f32,
  D : f32,
  E : f32,
  F : f32,
  padding : u32,
}

struct ExternalTextureParams {
  numPlanes : u32,
  doYuvToRgbConversionOnly : u32,
  yuvToRgbConversionMatrix : mat3x4<f32>,
  gammaDecodeParams : GammaTransferParams,
  gammaEncodeParams : GammaTransferParams,
  gamutConversionMatrix : mat3x3<f32>,
  sampleTransform : mat3x2<f32>,
  loadTransform : mat3x2<f32>,
  samplePlane0RectMin : vec2<f32>,
  samplePlane0RectMax : vec2<f32>,
  samplePlane1RectMin : vec2<f32>,
  samplePlane1RectMax : vec2<f32>,
  apparentSize : vec2<u32>,
  plane1CoordFactor : vec2<f32>,
}

@group(0) @binding(2) var ext_tex_plane_1 : texture_2d<f32>;

@group(0) @binding(3) var<uniform> ext_tex_params : ExternalTextureParams;

@fragment
fn main() {
  f(ext_tex, ext_tex_plane_1, ext_tex_params, smp);
}

fn gammaCorrection(v : vec3<f32>, params : GammaTransferParams) -> vec3<f32> {
  let cond = (abs(v) < vec3<f32>(params.D));
  let t = (sign(v) * ((params.C * abs(v)) + params.F));
  let f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3<f32>(params.G)) + params.E));
  return select(f, t, cond);
}

fn textureSampleExternal(plane0 : texture_2d<f32>, plane1 : texture_2d<f32>, smp : sampler, coord : vec2<f32>, params : ExternalTextureParams) -> vec4<f32> {
  let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
  let plane0_clamped = clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
  var color : vec4<f32>;
  if ((params.numPlanes == 1)) {
    color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
  } else {
    let plane1_clamped = clamp(modifiedCoords, params.samplePlane1RectMin, params.samplePlane1RectMax);
    color = vec4<f32>((vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r, textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) * params.yuvToRgbConversionMatrix), 1);
  }
  if ((params.doYuvToRgbConversionOnly == 0)) {
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

fn f(t : texture_2d<f32>, ext_tex_plane_1_1 : texture_2d<f32>, ext_tex_params_1 : ExternalTextureParams, s : sampler) {
  _ = textureSampleExternal(t, ext_tex_plane_1_1, s, vec2<f32>(1.0, 2.0), ext_tex_params_1);
}

@group(0) @binding(0) var ext_tex : texture_2d<f32>;

@group(0) @binding(1) var smp : sampler;

alias ET = texture_external;
)";
    DataMap data;
    data.Add<MultiplanarExternalTexture::NewBindingPoints>(
        tint::transform::multiplanar::BindingsMap{
            {{0, 0}, {{0, 2}, {0, 3}}},
        });
    auto got = Run<MultiplanarExternalTexture>(src, data);
    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
