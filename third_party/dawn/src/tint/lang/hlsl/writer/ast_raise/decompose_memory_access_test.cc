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

#include "src/tint/lang/hlsl/writer/ast_raise/decompose_memory_access.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"

namespace tint::hlsl::writer {
namespace {

using DecomposeMemoryAccessTest = ast::transform::TransformTest;

TEST_F(DecomposeMemoryAccessTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<DecomposeMemoryAccess>(src));
}

TEST_F(DecomposeMemoryAccessTest, ShouldRunStorageBuffer) {
    auto* src = R"(
struct Buffer {
  i : i32,
};
@group(0) @binding(0) var<storage, read_write> sb : Buffer;
)";

    EXPECT_TRUE(ShouldRun<DecomposeMemoryAccess>(src));
}

TEST_F(DecomposeMemoryAccessTest, ShouldRunUniformBuffer) {
    auto* src = R"(
struct Buffer {
  i : i32,
};
@group(0) @binding(0) var<uniform> ub : Buffer;
)";

    EXPECT_TRUE(ShouldRun<DecomposeMemoryAccess>(src));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicLoad) {
    auto* src = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_vec3_f16 : array<vec3<f16>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = sb.scalar_f32;
  var scalar_i32 : i32 = sb.scalar_i32;
  var scalar_u32 : u32 = sb.scalar_u32;
  var scalar_f16 : f16 = sb.scalar_f16;
  var vec2_f32 : vec2<f32> = sb.vec2_f32;
  var vec2_i32 : vec2<i32> = sb.vec2_i32;
  var vec2_u32 : vec2<u32> = sb.vec2_u32;
  var vec2_f16 : vec2<f16> = sb.vec2_f16;
  var vec3_f32 : vec3<f32> = sb.vec3_f32;
  var vec3_i32 : vec3<i32> = sb.vec3_i32;
  var vec3_u32 : vec3<u32> = sb.vec3_u32;
  var vec3_f16 : vec3<f16> = sb.vec3_f16;
  var vec4_f32 : vec4<f32> = sb.vec4_f32;
  var vec4_i32 : vec4<i32> = sb.vec4_i32;
  var vec4_u32 : vec4<u32> = sb.vec4_u32;
  var vec4_f16 : vec4<f16> = sb.vec4_f16;
  var mat2x2_f32 : mat2x2<f32> = sb.mat2x2_f32;
  var mat2x3_f32 : mat2x3<f32> = sb.mat2x3_f32;
  var mat2x4_f32 : mat2x4<f32> = sb.mat2x4_f32;
  var mat3x2_f32 : mat3x2<f32> = sb.mat3x2_f32;
  var mat3x3_f32 : mat3x3<f32> = sb.mat3x3_f32;
  var mat3x4_f32 : mat3x4<f32> = sb.mat3x4_f32;
  var mat4x2_f32 : mat4x2<f32> = sb.mat4x2_f32;
  var mat4x3_f32 : mat4x3<f32> = sb.mat4x3_f32;
  var mat4x4_f32 : mat4x4<f32> = sb.mat4x4_f32;
  var mat2x2_f16 : mat2x2<f16> = sb.mat2x2_f16;
  var mat2x3_f16 : mat2x3<f16> = sb.mat2x3_f16;
  var mat2x4_f16 : mat2x4<f16> = sb.mat2x4_f16;
  var mat3x2_f16 : mat3x2<f16> = sb.mat3x2_f16;
  var mat3x3_f16 : mat3x3<f16> = sb.mat3x3_f16;
  var mat3x4_f16 : mat3x4<f16> = sb.mat3x4_f16;
  var mat4x2_f16 : mat4x2<f16> = sb.mat4x2_f16;
  var mat4x3_f16 : mat4x3<f16> = sb.mat4x3_f16;
  var mat4x4_f16 : mat4x4<f16> = sb.mat4x4_f16;
  var arr2_vec3_f32 : array<vec3<f32>, 2> = sb.arr2_vec3_f32;
  var arr2_vec3_f16 : array<vec3<f16>, 2> = sb.arr2_vec3_f16;
}
)";

    auto* expect = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_vec3_f16 : array<vec3<f16>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load(offset : u32) -> f32

@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_1(offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_2(offset : u32) -> u32

@internal(intrinsic_load_storage_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_3(offset : u32) -> f16

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_4(offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_5(offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_6(offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_7(offset : u32) -> vec2<f16>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_8(offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_9(offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_10(offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_11(offset : u32) -> vec3<f16>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_12(offset : u32) -> vec4<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_13(offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_14(offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_15(offset : u32) -> vec4<f16>

fn sb_load_16(offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(sb_load_4((offset + 0u)), sb_load_4((offset + 8u)));
}

fn sb_load_17(offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(sb_load_8((offset + 0u)), sb_load_8((offset + 16u)));
}

fn sb_load_18(offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(sb_load_12((offset + 0u)), sb_load_12((offset + 16u)));
}

fn sb_load_19(offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(sb_load_4((offset + 0u)), sb_load_4((offset + 8u)), sb_load_4((offset + 16u)));
}

fn sb_load_20(offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(sb_load_8((offset + 0u)), sb_load_8((offset + 16u)), sb_load_8((offset + 32u)));
}

fn sb_load_21(offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(sb_load_12((offset + 0u)), sb_load_12((offset + 16u)), sb_load_12((offset + 32u)));
}

fn sb_load_22(offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(sb_load_4((offset + 0u)), sb_load_4((offset + 8u)), sb_load_4((offset + 16u)), sb_load_4((offset + 24u)));
}

fn sb_load_23(offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(sb_load_8((offset + 0u)), sb_load_8((offset + 16u)), sb_load_8((offset + 32u)), sb_load_8((offset + 48u)));
}

fn sb_load_24(offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(sb_load_12((offset + 0u)), sb_load_12((offset + 16u)), sb_load_12((offset + 32u)), sb_load_12((offset + 48u)));
}

fn sb_load_25(offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(sb_load_7((offset + 0u)), sb_load_7((offset + 4u)));
}

fn sb_load_26(offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(sb_load_11((offset + 0u)), sb_load_11((offset + 8u)));
}

fn sb_load_27(offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(sb_load_15((offset + 0u)), sb_load_15((offset + 8u)));
}

fn sb_load_28(offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(sb_load_7((offset + 0u)), sb_load_7((offset + 4u)), sb_load_7((offset + 8u)));
}

fn sb_load_29(offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(sb_load_11((offset + 0u)), sb_load_11((offset + 8u)), sb_load_11((offset + 16u)));
}

fn sb_load_30(offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(sb_load_15((offset + 0u)), sb_load_15((offset + 8u)), sb_load_15((offset + 16u)));
}

fn sb_load_31(offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(sb_load_7((offset + 0u)), sb_load_7((offset + 4u)), sb_load_7((offset + 8u)), sb_load_7((offset + 12u)));
}

fn sb_load_32(offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(sb_load_11((offset + 0u)), sb_load_11((offset + 8u)), sb_load_11((offset + 16u)), sb_load_11((offset + 24u)));
}

fn sb_load_33(offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(sb_load_15((offset + 0u)), sb_load_15((offset + 8u)), sb_load_15((offset + 16u)), sb_load_15((offset + 24u)));
}

fn sb_load_34(offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = sb_load_8((offset + (i * 16u)));
  }
  return arr;
}

fn sb_load_35(offset : u32) -> array<vec3<f16>, 2u> {
  var arr_1 : array<vec3<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = sb_load_11((offset + (i_1 * 8u)));
  }
  return arr_1;
}

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = sb_load(0u);
  var scalar_i32 : i32 = sb_load_1(4u);
  var scalar_u32 : u32 = sb_load_2(8u);
  var scalar_f16 : f16 = sb_load_3(12u);
  var vec2_f32 : vec2<f32> = sb_load_4(16u);
  var vec2_i32 : vec2<i32> = sb_load_5(24u);
  var vec2_u32 : vec2<u32> = sb_load_6(32u);
  var vec2_f16 : vec2<f16> = sb_load_7(40u);
  var vec3_f32 : vec3<f32> = sb_load_8(48u);
  var vec3_i32 : vec3<i32> = sb_load_9(64u);
  var vec3_u32 : vec3<u32> = sb_load_10(80u);
  var vec3_f16 : vec3<f16> = sb_load_11(96u);
  var vec4_f32 : vec4<f32> = sb_load_12(112u);
  var vec4_i32 : vec4<i32> = sb_load_13(128u);
  var vec4_u32 : vec4<u32> = sb_load_14(144u);
  var vec4_f16 : vec4<f16> = sb_load_15(160u);
  var mat2x2_f32 : mat2x2<f32> = sb_load_16(168u);
  var mat2x3_f32 : mat2x3<f32> = sb_load_17(192u);
  var mat2x4_f32 : mat2x4<f32> = sb_load_18(224u);
  var mat3x2_f32 : mat3x2<f32> = sb_load_19(256u);
  var mat3x3_f32 : mat3x3<f32> = sb_load_20(288u);
  var mat3x4_f32 : mat3x4<f32> = sb_load_21(336u);
  var mat4x2_f32 : mat4x2<f32> = sb_load_22(384u);
  var mat4x3_f32 : mat4x3<f32> = sb_load_23(416u);
  var mat4x4_f32 : mat4x4<f32> = sb_load_24(480u);
  var mat2x2_f16 : mat2x2<f16> = sb_load_25(544u);
  var mat2x3_f16 : mat2x3<f16> = sb_load_26(552u);
  var mat2x4_f16 : mat2x4<f16> = sb_load_27(568u);
  var mat3x2_f16 : mat3x2<f16> = sb_load_28(584u);
  var mat3x3_f16 : mat3x3<f16> = sb_load_29(600u);
  var mat3x4_f16 : mat3x4<f16> = sb_load_30(624u);
  var mat4x2_f16 : mat4x2<f16> = sb_load_31(648u);
  var mat4x3_f16 : mat4x3<f16> = sb_load_32(664u);
  var mat4x4_f16 : mat4x4<f16> = sb_load_33(696u);
  var arr2_vec3_f32 : array<vec3<f32>, 2> = sb_load_34(736u);
  var arr2_vec3_f16 : array<vec3<f16>, 2> = sb_load_35(768u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicLoad_OutOfOrder) {
    auto* src = R"(
enable f16;

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = sb.scalar_f32;
  var scalar_i32 : i32 = sb.scalar_i32;
  var scalar_u32 : u32 = sb.scalar_u32;
  var scalar_f16 : f16 = sb.scalar_f16;
  var vec2_f32 : vec2<f32> = sb.vec2_f32;
  var vec2_i32 : vec2<i32> = sb.vec2_i32;
  var vec2_u32 : vec2<u32> = sb.vec2_u32;
  var vec2_f16 : vec2<f16> = sb.vec2_f16;
  var vec3_f32 : vec3<f32> = sb.vec3_f32;
  var vec3_i32 : vec3<i32> = sb.vec3_i32;
  var vec3_u32 : vec3<u32> = sb.vec3_u32;
  var vec3_f16 : vec3<f16> = sb.vec3_f16;
  var vec4_f32 : vec4<f32> = sb.vec4_f32;
  var vec4_i32 : vec4<i32> = sb.vec4_i32;
  var vec4_u32 : vec4<u32> = sb.vec4_u32;
  var vec4_f16 : vec4<f16> = sb.vec4_f16;
  var mat2x2_f32 : mat2x2<f32> = sb.mat2x2_f32;
  var mat2x3_f32 : mat2x3<f32> = sb.mat2x3_f32;
  var mat2x4_f32 : mat2x4<f32> = sb.mat2x4_f32;
  var mat3x2_f32 : mat3x2<f32> = sb.mat3x2_f32;
  var mat3x3_f32 : mat3x3<f32> = sb.mat3x3_f32;
  var mat3x4_f32 : mat3x4<f32> = sb.mat3x4_f32;
  var mat4x2_f32 : mat4x2<f32> = sb.mat4x2_f32;
  var mat4x3_f32 : mat4x3<f32> = sb.mat4x3_f32;
  var mat4x4_f32 : mat4x4<f32> = sb.mat4x4_f32;
  var mat2x2_f16 : mat2x2<f16> = sb.mat2x2_f16;
  var mat2x3_f16 : mat2x3<f16> = sb.mat2x3_f16;
  var mat2x4_f16 : mat2x4<f16> = sb.mat2x4_f16;
  var mat3x2_f16 : mat3x2<f16> = sb.mat3x2_f16;
  var mat3x3_f16 : mat3x3<f16> = sb.mat3x3_f16;
  var mat3x4_f16 : mat3x4<f16> = sb.mat3x4_f16;
  var mat4x2_f16 : mat4x2<f16> = sb.mat4x2_f16;
  var mat4x3_f16 : mat4x3<f16> = sb.mat4x3_f16;
  var mat4x4_f16 : mat4x4<f16> = sb.mat4x4_f16;
  var arr2_vec3_f32 : array<vec3<f32>, 2> = sb.arr2_vec3_f32;
  var arr2_vec3_f16 : array<vec3<f16>, 2> = sb.arr2_vec3_f16;
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_vec3_f16 : array<vec3<f16>, 2>,
};
)";

    auto* expect = R"(
enable f16;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load(offset : u32) -> f32

@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_1(offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_2(offset : u32) -> u32

@internal(intrinsic_load_storage_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_3(offset : u32) -> f16

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_4(offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_5(offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_6(offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_7(offset : u32) -> vec2<f16>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_8(offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_9(offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_10(offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_11(offset : u32) -> vec3<f16>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_12(offset : u32) -> vec4<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_13(offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_14(offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_15(offset : u32) -> vec4<f16>

fn sb_load_16(offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(sb_load_4((offset + 0u)), sb_load_4((offset + 8u)));
}

fn sb_load_17(offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(sb_load_8((offset + 0u)), sb_load_8((offset + 16u)));
}

fn sb_load_18(offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(sb_load_12((offset + 0u)), sb_load_12((offset + 16u)));
}

fn sb_load_19(offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(sb_load_4((offset + 0u)), sb_load_4((offset + 8u)), sb_load_4((offset + 16u)));
}

fn sb_load_20(offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(sb_load_8((offset + 0u)), sb_load_8((offset + 16u)), sb_load_8((offset + 32u)));
}

fn sb_load_21(offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(sb_load_12((offset + 0u)), sb_load_12((offset + 16u)), sb_load_12((offset + 32u)));
}

fn sb_load_22(offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(sb_load_4((offset + 0u)), sb_load_4((offset + 8u)), sb_load_4((offset + 16u)), sb_load_4((offset + 24u)));
}

fn sb_load_23(offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(sb_load_8((offset + 0u)), sb_load_8((offset + 16u)), sb_load_8((offset + 32u)), sb_load_8((offset + 48u)));
}

fn sb_load_24(offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(sb_load_12((offset + 0u)), sb_load_12((offset + 16u)), sb_load_12((offset + 32u)), sb_load_12((offset + 48u)));
}

fn sb_load_25(offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(sb_load_7((offset + 0u)), sb_load_7((offset + 4u)));
}

fn sb_load_26(offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(sb_load_11((offset + 0u)), sb_load_11((offset + 8u)));
}

fn sb_load_27(offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(sb_load_15((offset + 0u)), sb_load_15((offset + 8u)));
}

fn sb_load_28(offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(sb_load_7((offset + 0u)), sb_load_7((offset + 4u)), sb_load_7((offset + 8u)));
}

fn sb_load_29(offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(sb_load_11((offset + 0u)), sb_load_11((offset + 8u)), sb_load_11((offset + 16u)));
}

fn sb_load_30(offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(sb_load_15((offset + 0u)), sb_load_15((offset + 8u)), sb_load_15((offset + 16u)));
}

fn sb_load_31(offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(sb_load_7((offset + 0u)), sb_load_7((offset + 4u)), sb_load_7((offset + 8u)), sb_load_7((offset + 12u)));
}

fn sb_load_32(offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(sb_load_11((offset + 0u)), sb_load_11((offset + 8u)), sb_load_11((offset + 16u)), sb_load_11((offset + 24u)));
}

fn sb_load_33(offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(sb_load_15((offset + 0u)), sb_load_15((offset + 8u)), sb_load_15((offset + 16u)), sb_load_15((offset + 24u)));
}

fn sb_load_34(offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = sb_load_8((offset + (i * 16u)));
  }
  return arr;
}

fn sb_load_35(offset : u32) -> array<vec3<f16>, 2u> {
  var arr_1 : array<vec3<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = sb_load_11((offset + (i_1 * 8u)));
  }
  return arr_1;
}

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = sb_load(0u);
  var scalar_i32 : i32 = sb_load_1(4u);
  var scalar_u32 : u32 = sb_load_2(8u);
  var scalar_f16 : f16 = sb_load_3(12u);
  var vec2_f32 : vec2<f32> = sb_load_4(16u);
  var vec2_i32 : vec2<i32> = sb_load_5(24u);
  var vec2_u32 : vec2<u32> = sb_load_6(32u);
  var vec2_f16 : vec2<f16> = sb_load_7(40u);
  var vec3_f32 : vec3<f32> = sb_load_8(48u);
  var vec3_i32 : vec3<i32> = sb_load_9(64u);
  var vec3_u32 : vec3<u32> = sb_load_10(80u);
  var vec3_f16 : vec3<f16> = sb_load_11(96u);
  var vec4_f32 : vec4<f32> = sb_load_12(112u);
  var vec4_i32 : vec4<i32> = sb_load_13(128u);
  var vec4_u32 : vec4<u32> = sb_load_14(144u);
  var vec4_f16 : vec4<f16> = sb_load_15(160u);
  var mat2x2_f32 : mat2x2<f32> = sb_load_16(168u);
  var mat2x3_f32 : mat2x3<f32> = sb_load_17(192u);
  var mat2x4_f32 : mat2x4<f32> = sb_load_18(224u);
  var mat3x2_f32 : mat3x2<f32> = sb_load_19(256u);
  var mat3x3_f32 : mat3x3<f32> = sb_load_20(288u);
  var mat3x4_f32 : mat3x4<f32> = sb_load_21(336u);
  var mat4x2_f32 : mat4x2<f32> = sb_load_22(384u);
  var mat4x3_f32 : mat4x3<f32> = sb_load_23(416u);
  var mat4x4_f32 : mat4x4<f32> = sb_load_24(480u);
  var mat2x2_f16 : mat2x2<f16> = sb_load_25(544u);
  var mat2x3_f16 : mat2x3<f16> = sb_load_26(552u);
  var mat2x4_f16 : mat2x4<f16> = sb_load_27(568u);
  var mat3x2_f16 : mat3x2<f16> = sb_load_28(584u);
  var mat3x3_f16 : mat3x3<f16> = sb_load_29(600u);
  var mat3x4_f16 : mat3x4<f16> = sb_load_30(624u);
  var mat4x2_f16 : mat4x2<f16> = sb_load_31(648u);
  var mat4x3_f16 : mat4x3<f16> = sb_load_32(664u);
  var mat4x4_f16 : mat4x4<f16> = sb_load_33(696u);
  var arr2_vec3_f32 : array<vec3<f32>, 2> = sb_load_34(736u);
  var arr2_vec3_f16 : array<vec3<f16>, 2> = sb_load_35(768u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_vec3_f16 : array<vec3<f16>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, UB_BasicLoad) {
    auto* src = R"(
enable f16;

struct UB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};

@group(0) @binding(0) var<uniform> ub : UB;

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = ub.scalar_f32;
  var scalar_i32 : i32 = ub.scalar_i32;
  var scalar_u32 : u32 = ub.scalar_u32;
  var scalar_f16 : f16 = ub.scalar_f16;
  var vec2_f32 : vec2<f32> = ub.vec2_f32;
  var vec2_i32 : vec2<i32> = ub.vec2_i32;
  var vec2_u32 : vec2<u32> = ub.vec2_u32;
  var vec2_f16 : vec2<f16> = ub.vec2_f16;
  var vec3_f32 : vec3<f32> = ub.vec3_f32;
  var vec3_i32 : vec3<i32> = ub.vec3_i32;
  var vec3_u32 : vec3<u32> = ub.vec3_u32;
  var vec3_f16 : vec3<f16> = ub.vec3_f16;
  var vec4_f32 : vec4<f32> = ub.vec4_f32;
  var vec4_i32 : vec4<i32> = ub.vec4_i32;
  var vec4_u32 : vec4<u32> = ub.vec4_u32;
  var vec4_f16 : vec4<f16> = ub.vec4_f16;
  var mat2x2_f32 : mat2x2<f32> = ub.mat2x2_f32;
  var mat2x3_f32 : mat2x3<f32> = ub.mat2x3_f32;
  var mat2x4_f32 : mat2x4<f32> = ub.mat2x4_f32;
  var mat3x2_f32 : mat3x2<f32> = ub.mat3x2_f32;
  var mat3x3_f32 : mat3x3<f32> = ub.mat3x3_f32;
  var mat3x4_f32 : mat3x4<f32> = ub.mat3x4_f32;
  var mat4x2_f32 : mat4x2<f32> = ub.mat4x2_f32;
  var mat4x3_f32 : mat4x3<f32> = ub.mat4x3_f32;
  var mat4x4_f32 : mat4x4<f32> = ub.mat4x4_f32;
  var mat2x2_f16 : mat2x2<f16> = ub.mat2x2_f16;
  var mat2x3_f16 : mat2x3<f16> = ub.mat2x3_f16;
  var mat2x4_f16 : mat2x4<f16> = ub.mat2x4_f16;
  var mat3x2_f16 : mat3x2<f16> = ub.mat3x2_f16;
  var mat3x3_f16 : mat3x3<f16> = ub.mat3x3_f16;
  var mat3x4_f16 : mat3x4<f16> = ub.mat3x4_f16;
  var mat4x2_f16 : mat4x2<f16> = ub.mat4x2_f16;
  var mat4x3_f16 : mat4x3<f16> = ub.mat4x3_f16;
  var mat4x4_f16 : mat4x4<f16> = ub.mat4x4_f16;
  var arr2_vec3_f32 : array<vec3<f32>, 2> = ub.arr2_vec3_f32;
  var arr2_mat4x2_f16 : array<mat4x2<f16>, 2> = ub.arr2_mat4x2_f16;
}
)";

    auto* expect = R"(
enable f16;

struct UB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}

@group(0) @binding(0) var<uniform> ub : UB;

@internal(intrinsic_load_uniform_f32) @internal(disable_validation__function_has_no_body)
fn ub_load(offset : u32) -> f32

@internal(intrinsic_load_uniform_i32) @internal(disable_validation__function_has_no_body)
fn ub_load_1(offset : u32) -> i32

@internal(intrinsic_load_uniform_u32) @internal(disable_validation__function_has_no_body)
fn ub_load_2(offset : u32) -> u32

@internal(intrinsic_load_uniform_f16) @internal(disable_validation__function_has_no_body)
fn ub_load_3(offset : u32) -> f16

@internal(intrinsic_load_uniform_vec2_f32) @internal(disable_validation__function_has_no_body)
fn ub_load_4(offset : u32) -> vec2<f32>

@internal(intrinsic_load_uniform_vec2_i32) @internal(disable_validation__function_has_no_body)
fn ub_load_5(offset : u32) -> vec2<i32>

@internal(intrinsic_load_uniform_vec2_u32) @internal(disable_validation__function_has_no_body)
fn ub_load_6(offset : u32) -> vec2<u32>

@internal(intrinsic_load_uniform_vec2_f16) @internal(disable_validation__function_has_no_body)
fn ub_load_7(offset : u32) -> vec2<f16>

@internal(intrinsic_load_uniform_vec3_f32) @internal(disable_validation__function_has_no_body)
fn ub_load_8(offset : u32) -> vec3<f32>

@internal(intrinsic_load_uniform_vec3_i32) @internal(disable_validation__function_has_no_body)
fn ub_load_9(offset : u32) -> vec3<i32>

@internal(intrinsic_load_uniform_vec3_u32) @internal(disable_validation__function_has_no_body)
fn ub_load_10(offset : u32) -> vec3<u32>

@internal(intrinsic_load_uniform_vec3_f16) @internal(disable_validation__function_has_no_body)
fn ub_load_11(offset : u32) -> vec3<f16>

@internal(intrinsic_load_uniform_vec4_f32) @internal(disable_validation__function_has_no_body)
fn ub_load_12(offset : u32) -> vec4<f32>

@internal(intrinsic_load_uniform_vec4_i32) @internal(disable_validation__function_has_no_body)
fn ub_load_13(offset : u32) -> vec4<i32>

@internal(intrinsic_load_uniform_vec4_u32) @internal(disable_validation__function_has_no_body)
fn ub_load_14(offset : u32) -> vec4<u32>

@internal(intrinsic_load_uniform_vec4_f16) @internal(disable_validation__function_has_no_body)
fn ub_load_15(offset : u32) -> vec4<f16>

fn ub_load_16(offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(ub_load_4((offset + 0u)), ub_load_4((offset + 8u)));
}

fn ub_load_17(offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(ub_load_8((offset + 0u)), ub_load_8((offset + 16u)));
}

fn ub_load_18(offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(ub_load_12((offset + 0u)), ub_load_12((offset + 16u)));
}

fn ub_load_19(offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(ub_load_4((offset + 0u)), ub_load_4((offset + 8u)), ub_load_4((offset + 16u)));
}

fn ub_load_20(offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(ub_load_8((offset + 0u)), ub_load_8((offset + 16u)), ub_load_8((offset + 32u)));
}

fn ub_load_21(offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(ub_load_12((offset + 0u)), ub_load_12((offset + 16u)), ub_load_12((offset + 32u)));
}

fn ub_load_22(offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(ub_load_4((offset + 0u)), ub_load_4((offset + 8u)), ub_load_4((offset + 16u)), ub_load_4((offset + 24u)));
}

fn ub_load_23(offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(ub_load_8((offset + 0u)), ub_load_8((offset + 16u)), ub_load_8((offset + 32u)), ub_load_8((offset + 48u)));
}

fn ub_load_24(offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(ub_load_12((offset + 0u)), ub_load_12((offset + 16u)), ub_load_12((offset + 32u)), ub_load_12((offset + 48u)));
}

fn ub_load_25(offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(ub_load_7((offset + 0u)), ub_load_7((offset + 4u)));
}

fn ub_load_26(offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(ub_load_11((offset + 0u)), ub_load_11((offset + 8u)));
}

fn ub_load_27(offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(ub_load_15((offset + 0u)), ub_load_15((offset + 8u)));
}

fn ub_load_28(offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(ub_load_7((offset + 0u)), ub_load_7((offset + 4u)), ub_load_7((offset + 8u)));
}

fn ub_load_29(offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(ub_load_11((offset + 0u)), ub_load_11((offset + 8u)), ub_load_11((offset + 16u)));
}

fn ub_load_30(offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(ub_load_15((offset + 0u)), ub_load_15((offset + 8u)), ub_load_15((offset + 16u)));
}

fn ub_load_31(offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(ub_load_7((offset + 0u)), ub_load_7((offset + 4u)), ub_load_7((offset + 8u)), ub_load_7((offset + 12u)));
}

fn ub_load_32(offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(ub_load_11((offset + 0u)), ub_load_11((offset + 8u)), ub_load_11((offset + 16u)), ub_load_11((offset + 24u)));
}

fn ub_load_33(offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(ub_load_15((offset + 0u)), ub_load_15((offset + 8u)), ub_load_15((offset + 16u)), ub_load_15((offset + 24u)));
}

fn ub_load_34(offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = ub_load_8((offset + (i * 16u)));
  }
  return arr;
}

fn ub_load_35(offset : u32) -> array<mat4x2<f16>, 2u> {
  var arr_1 : array<mat4x2<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = ub_load_31((offset + (i_1 * 16u)));
  }
  return arr_1;
}

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = ub_load(0u);
  var scalar_i32 : i32 = ub_load_1(4u);
  var scalar_u32 : u32 = ub_load_2(8u);
  var scalar_f16 : f16 = ub_load_3(12u);
  var vec2_f32 : vec2<f32> = ub_load_4(16u);
  var vec2_i32 : vec2<i32> = ub_load_5(24u);
  var vec2_u32 : vec2<u32> = ub_load_6(32u);
  var vec2_f16 : vec2<f16> = ub_load_7(40u);
  var vec3_f32 : vec3<f32> = ub_load_8(48u);
  var vec3_i32 : vec3<i32> = ub_load_9(64u);
  var vec3_u32 : vec3<u32> = ub_load_10(80u);
  var vec3_f16 : vec3<f16> = ub_load_11(96u);
  var vec4_f32 : vec4<f32> = ub_load_12(112u);
  var vec4_i32 : vec4<i32> = ub_load_13(128u);
  var vec4_u32 : vec4<u32> = ub_load_14(144u);
  var vec4_f16 : vec4<f16> = ub_load_15(160u);
  var mat2x2_f32 : mat2x2<f32> = ub_load_16(168u);
  var mat2x3_f32 : mat2x3<f32> = ub_load_17(192u);
  var mat2x4_f32 : mat2x4<f32> = ub_load_18(224u);
  var mat3x2_f32 : mat3x2<f32> = ub_load_19(256u);
  var mat3x3_f32 : mat3x3<f32> = ub_load_20(288u);
  var mat3x4_f32 : mat3x4<f32> = ub_load_21(336u);
  var mat4x2_f32 : mat4x2<f32> = ub_load_22(384u);
  var mat4x3_f32 : mat4x3<f32> = ub_load_23(416u);
  var mat4x4_f32 : mat4x4<f32> = ub_load_24(480u);
  var mat2x2_f16 : mat2x2<f16> = ub_load_25(544u);
  var mat2x3_f16 : mat2x3<f16> = ub_load_26(552u);
  var mat2x4_f16 : mat2x4<f16> = ub_load_27(568u);
  var mat3x2_f16 : mat3x2<f16> = ub_load_28(584u);
  var mat3x3_f16 : mat3x3<f16> = ub_load_29(600u);
  var mat3x4_f16 : mat3x4<f16> = ub_load_30(624u);
  var mat4x2_f16 : mat4x2<f16> = ub_load_31(648u);
  var mat4x3_f16 : mat4x3<f16> = ub_load_32(664u);
  var mat4x4_f16 : mat4x4<f16> = ub_load_33(696u);
  var arr2_vec3_f32 : array<vec3<f32>, 2> = ub_load_34(736u);
  var arr2_mat4x2_f16 : array<mat4x2<f16>, 2> = ub_load_35(768u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, UB_BasicLoad_OutOfOrder) {
    auto* src = R"(
enable f16;

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = ub.scalar_f32;
  var scalar_i32 : i32 = ub.scalar_i32;
  var scalar_u32 : u32 = ub.scalar_u32;
  var scalar_f16 : f16 = ub.scalar_f16;
  var vec2_f32 : vec2<f32> = ub.vec2_f32;
  var vec2_i32 : vec2<i32> = ub.vec2_i32;
  var vec2_u32 : vec2<u32> = ub.vec2_u32;
  var vec2_f16 : vec2<f16> = ub.vec2_f16;
  var vec3_f32 : vec3<f32> = ub.vec3_f32;
  var vec3_i32 : vec3<i32> = ub.vec3_i32;
  var vec3_u32 : vec3<u32> = ub.vec3_u32;
  var vec3_f16 : vec3<f16> = ub.vec3_f16;
  var vec4_f32 : vec4<f32> = ub.vec4_f32;
  var vec4_i32 : vec4<i32> = ub.vec4_i32;
  var vec4_u32 : vec4<u32> = ub.vec4_u32;
  var vec4_f16 : vec4<f16> = ub.vec4_f16;
  var mat2x2_f32 : mat2x2<f32> = ub.mat2x2_f32;
  var mat2x3_f32 : mat2x3<f32> = ub.mat2x3_f32;
  var mat2x4_f32 : mat2x4<f32> = ub.mat2x4_f32;
  var mat3x2_f32 : mat3x2<f32> = ub.mat3x2_f32;
  var mat3x3_f32 : mat3x3<f32> = ub.mat3x3_f32;
  var mat3x4_f32 : mat3x4<f32> = ub.mat3x4_f32;
  var mat4x2_f32 : mat4x2<f32> = ub.mat4x2_f32;
  var mat4x3_f32 : mat4x3<f32> = ub.mat4x3_f32;
  var mat4x4_f32 : mat4x4<f32> = ub.mat4x4_f32;
  var mat2x2_f16 : mat2x2<f16> = ub.mat2x2_f16;
  var mat2x3_f16 : mat2x3<f16> = ub.mat2x3_f16;
  var mat2x4_f16 : mat2x4<f16> = ub.mat2x4_f16;
  var mat3x2_f16 : mat3x2<f16> = ub.mat3x2_f16;
  var mat3x3_f16 : mat3x3<f16> = ub.mat3x3_f16;
  var mat3x4_f16 : mat3x4<f16> = ub.mat3x4_f16;
  var mat4x2_f16 : mat4x2<f16> = ub.mat4x2_f16;
  var mat4x3_f16 : mat4x3<f16> = ub.mat4x3_f16;
  var mat4x4_f16 : mat4x4<f16> = ub.mat4x4_f16;
  var arr2_vec3_f32 : array<vec3<f32>, 2> = ub.arr2_vec3_f32;
  var arr2_mat4x2_f16 : array<mat4x2<f16>, 2> = ub.arr2_mat4x2_f16;
}

@group(0) @binding(0) var<uniform> ub : UB;

struct UB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};
)";

    auto* expect = R"(
enable f16;

@internal(intrinsic_load_uniform_f32) @internal(disable_validation__function_has_no_body)
fn ub_load(offset : u32) -> f32

@internal(intrinsic_load_uniform_i32) @internal(disable_validation__function_has_no_body)
fn ub_load_1(offset : u32) -> i32

@internal(intrinsic_load_uniform_u32) @internal(disable_validation__function_has_no_body)
fn ub_load_2(offset : u32) -> u32

@internal(intrinsic_load_uniform_f16) @internal(disable_validation__function_has_no_body)
fn ub_load_3(offset : u32) -> f16

@internal(intrinsic_load_uniform_vec2_f32) @internal(disable_validation__function_has_no_body)
fn ub_load_4(offset : u32) -> vec2<f32>

@internal(intrinsic_load_uniform_vec2_i32) @internal(disable_validation__function_has_no_body)
fn ub_load_5(offset : u32) -> vec2<i32>

@internal(intrinsic_load_uniform_vec2_u32) @internal(disable_validation__function_has_no_body)
fn ub_load_6(offset : u32) -> vec2<u32>

@internal(intrinsic_load_uniform_vec2_f16) @internal(disable_validation__function_has_no_body)
fn ub_load_7(offset : u32) -> vec2<f16>

@internal(intrinsic_load_uniform_vec3_f32) @internal(disable_validation__function_has_no_body)
fn ub_load_8(offset : u32) -> vec3<f32>

@internal(intrinsic_load_uniform_vec3_i32) @internal(disable_validation__function_has_no_body)
fn ub_load_9(offset : u32) -> vec3<i32>

@internal(intrinsic_load_uniform_vec3_u32) @internal(disable_validation__function_has_no_body)
fn ub_load_10(offset : u32) -> vec3<u32>

@internal(intrinsic_load_uniform_vec3_f16) @internal(disable_validation__function_has_no_body)
fn ub_load_11(offset : u32) -> vec3<f16>

@internal(intrinsic_load_uniform_vec4_f32) @internal(disable_validation__function_has_no_body)
fn ub_load_12(offset : u32) -> vec4<f32>

@internal(intrinsic_load_uniform_vec4_i32) @internal(disable_validation__function_has_no_body)
fn ub_load_13(offset : u32) -> vec4<i32>

@internal(intrinsic_load_uniform_vec4_u32) @internal(disable_validation__function_has_no_body)
fn ub_load_14(offset : u32) -> vec4<u32>

@internal(intrinsic_load_uniform_vec4_f16) @internal(disable_validation__function_has_no_body)
fn ub_load_15(offset : u32) -> vec4<f16>

fn ub_load_16(offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(ub_load_4((offset + 0u)), ub_load_4((offset + 8u)));
}

fn ub_load_17(offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(ub_load_8((offset + 0u)), ub_load_8((offset + 16u)));
}

fn ub_load_18(offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(ub_load_12((offset + 0u)), ub_load_12((offset + 16u)));
}

fn ub_load_19(offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(ub_load_4((offset + 0u)), ub_load_4((offset + 8u)), ub_load_4((offset + 16u)));
}

fn ub_load_20(offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(ub_load_8((offset + 0u)), ub_load_8((offset + 16u)), ub_load_8((offset + 32u)));
}

fn ub_load_21(offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(ub_load_12((offset + 0u)), ub_load_12((offset + 16u)), ub_load_12((offset + 32u)));
}

fn ub_load_22(offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(ub_load_4((offset + 0u)), ub_load_4((offset + 8u)), ub_load_4((offset + 16u)), ub_load_4((offset + 24u)));
}

fn ub_load_23(offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(ub_load_8((offset + 0u)), ub_load_8((offset + 16u)), ub_load_8((offset + 32u)), ub_load_8((offset + 48u)));
}

fn ub_load_24(offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(ub_load_12((offset + 0u)), ub_load_12((offset + 16u)), ub_load_12((offset + 32u)), ub_load_12((offset + 48u)));
}

fn ub_load_25(offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(ub_load_7((offset + 0u)), ub_load_7((offset + 4u)));
}

fn ub_load_26(offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(ub_load_11((offset + 0u)), ub_load_11((offset + 8u)));
}

fn ub_load_27(offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(ub_load_15((offset + 0u)), ub_load_15((offset + 8u)));
}

fn ub_load_28(offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(ub_load_7((offset + 0u)), ub_load_7((offset + 4u)), ub_load_7((offset + 8u)));
}

fn ub_load_29(offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(ub_load_11((offset + 0u)), ub_load_11((offset + 8u)), ub_load_11((offset + 16u)));
}

fn ub_load_30(offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(ub_load_15((offset + 0u)), ub_load_15((offset + 8u)), ub_load_15((offset + 16u)));
}

fn ub_load_31(offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(ub_load_7((offset + 0u)), ub_load_7((offset + 4u)), ub_load_7((offset + 8u)), ub_load_7((offset + 12u)));
}

fn ub_load_32(offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(ub_load_11((offset + 0u)), ub_load_11((offset + 8u)), ub_load_11((offset + 16u)), ub_load_11((offset + 24u)));
}

fn ub_load_33(offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(ub_load_15((offset + 0u)), ub_load_15((offset + 8u)), ub_load_15((offset + 16u)), ub_load_15((offset + 24u)));
}

fn ub_load_34(offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = ub_load_8((offset + (i * 16u)));
  }
  return arr;
}

fn ub_load_35(offset : u32) -> array<mat4x2<f16>, 2u> {
  var arr_1 : array<mat4x2<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = ub_load_31((offset + (i_1 * 16u)));
  }
  return arr_1;
}

@compute @workgroup_size(1)
fn main() {
  var scalar_f32 : f32 = ub_load(0u);
  var scalar_i32 : i32 = ub_load_1(4u);
  var scalar_u32 : u32 = ub_load_2(8u);
  var scalar_f16 : f16 = ub_load_3(12u);
  var vec2_f32 : vec2<f32> = ub_load_4(16u);
  var vec2_i32 : vec2<i32> = ub_load_5(24u);
  var vec2_u32 : vec2<u32> = ub_load_6(32u);
  var vec2_f16 : vec2<f16> = ub_load_7(40u);
  var vec3_f32 : vec3<f32> = ub_load_8(48u);
  var vec3_i32 : vec3<i32> = ub_load_9(64u);
  var vec3_u32 : vec3<u32> = ub_load_10(80u);
  var vec3_f16 : vec3<f16> = ub_load_11(96u);
  var vec4_f32 : vec4<f32> = ub_load_12(112u);
  var vec4_i32 : vec4<i32> = ub_load_13(128u);
  var vec4_u32 : vec4<u32> = ub_load_14(144u);
  var vec4_f16 : vec4<f16> = ub_load_15(160u);
  var mat2x2_f32 : mat2x2<f32> = ub_load_16(168u);
  var mat2x3_f32 : mat2x3<f32> = ub_load_17(192u);
  var mat2x4_f32 : mat2x4<f32> = ub_load_18(224u);
  var mat3x2_f32 : mat3x2<f32> = ub_load_19(256u);
  var mat3x3_f32 : mat3x3<f32> = ub_load_20(288u);
  var mat3x4_f32 : mat3x4<f32> = ub_load_21(336u);
  var mat4x2_f32 : mat4x2<f32> = ub_load_22(384u);
  var mat4x3_f32 : mat4x3<f32> = ub_load_23(416u);
  var mat4x4_f32 : mat4x4<f32> = ub_load_24(480u);
  var mat2x2_f16 : mat2x2<f16> = ub_load_25(544u);
  var mat2x3_f16 : mat2x3<f16> = ub_load_26(552u);
  var mat2x4_f16 : mat2x4<f16> = ub_load_27(568u);
  var mat3x2_f16 : mat3x2<f16> = ub_load_28(584u);
  var mat3x3_f16 : mat3x3<f16> = ub_load_29(600u);
  var mat3x4_f16 : mat3x4<f16> = ub_load_30(624u);
  var mat4x2_f16 : mat4x2<f16> = ub_load_31(648u);
  var mat4x3_f16 : mat4x3<f16> = ub_load_32(664u);
  var mat4x4_f16 : mat4x4<f16> = ub_load_33(696u);
  var arr2_vec3_f32 : array<vec3<f32>, 2> = ub_load_34(736u);
  var arr2_mat4x2_f16 : array<mat4x2<f16>, 2> = ub_load_35(768u);
}

@group(0) @binding(0) var<uniform> ub : UB;

struct UB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicStore) {
    auto* src = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  sb.scalar_f32 = f32();
  sb.scalar_i32 = i32();
  sb.scalar_u32 = u32();
  sb.scalar_f16 = f16();
  sb.vec2_f32 = vec2<f32>();
  sb.vec2_i32 = vec2<i32>();
  sb.vec2_u32 = vec2<u32>();
  sb.vec2_f16 = vec2<f16>();
  sb.vec3_f32 = vec3<f32>();
  sb.vec3_i32 = vec3<i32>();
  sb.vec3_u32 = vec3<u32>();
  sb.vec3_f16 = vec3<f16>();
  sb.vec4_f32 = vec4<f32>();
  sb.vec4_i32 = vec4<i32>();
  sb.vec4_u32 = vec4<u32>();
  sb.vec4_f16 = vec4<f16>();
  sb.mat2x2_f32 = mat2x2<f32>();
  sb.mat2x3_f32 = mat2x3<f32>();
  sb.mat2x4_f32 = mat2x4<f32>();
  sb.mat3x2_f32 = mat3x2<f32>();
  sb.mat3x3_f32 = mat3x3<f32>();
  sb.mat3x4_f32 = mat3x4<f32>();
  sb.mat4x2_f32 = mat4x2<f32>();
  sb.mat4x3_f32 = mat4x3<f32>();
  sb.mat4x4_f32 = mat4x4<f32>();
  sb.mat2x2_f16 = mat2x2<f16>();
  sb.mat2x3_f16 = mat2x3<f16>();
  sb.mat2x4_f16 = mat2x4<f16>();
  sb.mat3x2_f16 = mat3x2<f16>();
  sb.mat3x3_f16 = mat3x3<f16>();
  sb.mat3x4_f16 = mat3x4<f16>();
  sb.mat4x2_f16 = mat4x2<f16>();
  sb.mat4x3_f16 = mat4x3<f16>();
  sb.mat4x4_f16 = mat4x4<f16>();
  sb.arr2_vec3_f32 = array<vec3<f32>, 2>();
  sb.arr2_mat4x2_f16 = array<mat4x2<f16>, 2>();
}
)";

    auto* expect = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_store(offset : u32, value : f32)

@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_1(offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_2(offset : u32, value : u32)

@internal(intrinsic_store_storage_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_3(offset : u32, value : f16)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_4(offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_5(offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_6(offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_7(offset : u32, value : vec2<f16>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_8(offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_9(offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_10(offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_11(offset : u32, value : vec3<f16>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_12(offset : u32, value : vec4<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_13(offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_14(offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_15(offset : u32, value : vec4<f16>)

fn sb_store_16(offset : u32, value : mat2x2<f32>) {
  sb_store_4((offset + 0u), value[0u]);
  sb_store_4((offset + 8u), value[1u]);
}

fn sb_store_17(offset : u32, value : mat2x3<f32>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 16u), value[1u]);
}

fn sb_store_18(offset : u32, value : mat2x4<f32>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 16u), value[1u]);
}

fn sb_store_19(offset : u32, value : mat3x2<f32>) {
  sb_store_4((offset + 0u), value[0u]);
  sb_store_4((offset + 8u), value[1u]);
  sb_store_4((offset + 16u), value[2u]);
}

fn sb_store_20(offset : u32, value : mat3x3<f32>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 16u), value[1u]);
  sb_store_8((offset + 32u), value[2u]);
}

fn sb_store_21(offset : u32, value : mat3x4<f32>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 16u), value[1u]);
  sb_store_12((offset + 32u), value[2u]);
}

fn sb_store_22(offset : u32, value : mat4x2<f32>) {
  sb_store_4((offset + 0u), value[0u]);
  sb_store_4((offset + 8u), value[1u]);
  sb_store_4((offset + 16u), value[2u]);
  sb_store_4((offset + 24u), value[3u]);
}

fn sb_store_23(offset : u32, value : mat4x3<f32>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 16u), value[1u]);
  sb_store_8((offset + 32u), value[2u]);
  sb_store_8((offset + 48u), value[3u]);
}

fn sb_store_24(offset : u32, value : mat4x4<f32>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 16u), value[1u]);
  sb_store_12((offset + 32u), value[2u]);
  sb_store_12((offset + 48u), value[3u]);
}

fn sb_store_25(offset : u32, value : mat2x2<f16>) {
  sb_store_7((offset + 0u), value[0u]);
  sb_store_7((offset + 4u), value[1u]);
}

fn sb_store_26(offset : u32, value : mat2x3<f16>) {
  sb_store_11((offset + 0u), value[0u]);
  sb_store_11((offset + 8u), value[1u]);
}

fn sb_store_27(offset : u32, value : mat2x4<f16>) {
  sb_store_15((offset + 0u), value[0u]);
  sb_store_15((offset + 8u), value[1u]);
}

fn sb_store_28(offset : u32, value : mat3x2<f16>) {
  sb_store_7((offset + 0u), value[0u]);
  sb_store_7((offset + 4u), value[1u]);
  sb_store_7((offset + 8u), value[2u]);
}

fn sb_store_29(offset : u32, value : mat3x3<f16>) {
  sb_store_11((offset + 0u), value[0u]);
  sb_store_11((offset + 8u), value[1u]);
  sb_store_11((offset + 16u), value[2u]);
}

fn sb_store_30(offset : u32, value : mat3x4<f16>) {
  sb_store_15((offset + 0u), value[0u]);
  sb_store_15((offset + 8u), value[1u]);
  sb_store_15((offset + 16u), value[2u]);
}

fn sb_store_31(offset : u32, value : mat4x2<f16>) {
  sb_store_7((offset + 0u), value[0u]);
  sb_store_7((offset + 4u), value[1u]);
  sb_store_7((offset + 8u), value[2u]);
  sb_store_7((offset + 12u), value[3u]);
}

fn sb_store_32(offset : u32, value : mat4x3<f16>) {
  sb_store_11((offset + 0u), value[0u]);
  sb_store_11((offset + 8u), value[1u]);
  sb_store_11((offset + 16u), value[2u]);
  sb_store_11((offset + 24u), value[3u]);
}

fn sb_store_33(offset : u32, value : mat4x4<f16>) {
  sb_store_15((offset + 0u), value[0u]);
  sb_store_15((offset + 8u), value[1u]);
  sb_store_15((offset + 16u), value[2u]);
  sb_store_15((offset + 24u), value[3u]);
}

fn sb_store_34(offset : u32, value : array<vec3<f32>, 2u>) {
  var array_1 = value;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    sb_store_8((offset + (i * 16u)), array_1[i]);
  }
}

fn sb_store_35(offset : u32, value : array<mat4x2<f16>, 2u>) {
  var array_2 = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    sb_store_31((offset + (i_1 * 16u)), array_2[i_1]);
  }
}

@compute @workgroup_size(1)
fn main() {
  sb_store(0u, f32());
  sb_store_1(4u, i32());
  sb_store_2(8u, u32());
  sb_store_3(12u, f16());
  sb_store_4(16u, vec2<f32>());
  sb_store_5(24u, vec2<i32>());
  sb_store_6(32u, vec2<u32>());
  sb_store_7(40u, vec2<f16>());
  sb_store_8(48u, vec3<f32>());
  sb_store_9(64u, vec3<i32>());
  sb_store_10(80u, vec3<u32>());
  sb_store_11(96u, vec3<f16>());
  sb_store_12(112u, vec4<f32>());
  sb_store_13(128u, vec4<i32>());
  sb_store_14(144u, vec4<u32>());
  sb_store_15(160u, vec4<f16>());
  sb_store_16(168u, mat2x2<f32>());
  sb_store_17(192u, mat2x3<f32>());
  sb_store_18(224u, mat2x4<f32>());
  sb_store_19(256u, mat3x2<f32>());
  sb_store_20(288u, mat3x3<f32>());
  sb_store_21(336u, mat3x4<f32>());
  sb_store_22(384u, mat4x2<f32>());
  sb_store_23(416u, mat4x3<f32>());
  sb_store_24(480u, mat4x4<f32>());
  sb_store_25(544u, mat2x2<f16>());
  sb_store_26(552u, mat2x3<f16>());
  sb_store_27(568u, mat2x4<f16>());
  sb_store_28(584u, mat3x2<f16>());
  sb_store_29(600u, mat3x3<f16>());
  sb_store_30(624u, mat3x4<f16>());
  sb_store_31(648u, mat4x2<f16>());
  sb_store_32(664u, mat4x3<f16>());
  sb_store_33(696u, mat4x4<f16>());
  sb_store_34(736u, array<vec3<f32>, 2>());
  sb_store_35(768u, array<mat4x2<f16>, 2>());
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, SB_BasicStore_OutOfOrder) {
    auto* src = R"(
enable f16;

@compute @workgroup_size(1)
fn main() {
  sb.scalar_f32 = f32();
  sb.scalar_i32 = i32();
  sb.scalar_u32 = u32();
  sb.scalar_f16 = f16();
  sb.vec2_f32 = vec2<f32>();
  sb.vec2_i32 = vec2<i32>();
  sb.vec2_u32 = vec2<u32>();
  sb.vec2_f16 = vec2<f16>();
  sb.vec3_f32 = vec3<f32>();
  sb.vec3_i32 = vec3<i32>();
  sb.vec3_u32 = vec3<u32>();
  sb.vec3_f16 = vec3<f16>();
  sb.vec4_f32 = vec4<f32>();
  sb.vec4_i32 = vec4<i32>();
  sb.vec4_u32 = vec4<u32>();
  sb.vec4_f16 = vec4<f16>();
  sb.mat2x2_f32 = mat2x2<f32>();
  sb.mat2x3_f32 = mat2x3<f32>();
  sb.mat2x4_f32 = mat2x4<f32>();
  sb.mat3x2_f32 = mat3x2<f32>();
  sb.mat3x3_f32 = mat3x3<f32>();
  sb.mat3x4_f32 = mat3x4<f32>();
  sb.mat4x2_f32 = mat4x2<f32>();
  sb.mat4x3_f32 = mat4x3<f32>();
  sb.mat4x4_f32 = mat4x4<f32>();
  sb.mat2x2_f16 = mat2x2<f16>();
  sb.mat2x3_f16 = mat2x3<f16>();
  sb.mat2x4_f16 = mat2x4<f16>();
  sb.mat3x2_f16 = mat3x2<f16>();
  sb.mat3x3_f16 = mat3x3<f16>();
  sb.mat3x4_f16 = mat3x4<f16>();
  sb.mat4x2_f16 = mat4x2<f16>();
  sb.mat4x3_f16 = mat4x3<f16>();
  sb.mat4x4_f16 = mat4x4<f16>();
  sb.arr2_vec3_f32 = array<vec3<f32>, 2>();
  sb.arr2_mat4x2_f16 = array<mat4x2<f16>, 2>();
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};
)";

    auto* expect = R"(
enable f16;

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_store(offset : u32, value : f32)

@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_1(offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_2(offset : u32, value : u32)

@internal(intrinsic_store_storage_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_3(offset : u32, value : f16)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_4(offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_5(offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_6(offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_7(offset : u32, value : vec2<f16>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_8(offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_9(offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_10(offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_11(offset : u32, value : vec3<f16>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_12(offset : u32, value : vec4<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_13(offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_14(offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_15(offset : u32, value : vec4<f16>)

fn sb_store_16(offset : u32, value : mat2x2<f32>) {
  sb_store_4((offset + 0u), value[0u]);
  sb_store_4((offset + 8u), value[1u]);
}

fn sb_store_17(offset : u32, value : mat2x3<f32>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 16u), value[1u]);
}

fn sb_store_18(offset : u32, value : mat2x4<f32>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 16u), value[1u]);
}

fn sb_store_19(offset : u32, value : mat3x2<f32>) {
  sb_store_4((offset + 0u), value[0u]);
  sb_store_4((offset + 8u), value[1u]);
  sb_store_4((offset + 16u), value[2u]);
}

fn sb_store_20(offset : u32, value : mat3x3<f32>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 16u), value[1u]);
  sb_store_8((offset + 32u), value[2u]);
}

fn sb_store_21(offset : u32, value : mat3x4<f32>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 16u), value[1u]);
  sb_store_12((offset + 32u), value[2u]);
}

fn sb_store_22(offset : u32, value : mat4x2<f32>) {
  sb_store_4((offset + 0u), value[0u]);
  sb_store_4((offset + 8u), value[1u]);
  sb_store_4((offset + 16u), value[2u]);
  sb_store_4((offset + 24u), value[3u]);
}

fn sb_store_23(offset : u32, value : mat4x3<f32>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 16u), value[1u]);
  sb_store_8((offset + 32u), value[2u]);
  sb_store_8((offset + 48u), value[3u]);
}

fn sb_store_24(offset : u32, value : mat4x4<f32>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 16u), value[1u]);
  sb_store_12((offset + 32u), value[2u]);
  sb_store_12((offset + 48u), value[3u]);
}

fn sb_store_25(offset : u32, value : mat2x2<f16>) {
  sb_store_7((offset + 0u), value[0u]);
  sb_store_7((offset + 4u), value[1u]);
}

fn sb_store_26(offset : u32, value : mat2x3<f16>) {
  sb_store_11((offset + 0u), value[0u]);
  sb_store_11((offset + 8u), value[1u]);
}

fn sb_store_27(offset : u32, value : mat2x4<f16>) {
  sb_store_15((offset + 0u), value[0u]);
  sb_store_15((offset + 8u), value[1u]);
}

fn sb_store_28(offset : u32, value : mat3x2<f16>) {
  sb_store_7((offset + 0u), value[0u]);
  sb_store_7((offset + 4u), value[1u]);
  sb_store_7((offset + 8u), value[2u]);
}

fn sb_store_29(offset : u32, value : mat3x3<f16>) {
  sb_store_11((offset + 0u), value[0u]);
  sb_store_11((offset + 8u), value[1u]);
  sb_store_11((offset + 16u), value[2u]);
}

fn sb_store_30(offset : u32, value : mat3x4<f16>) {
  sb_store_15((offset + 0u), value[0u]);
  sb_store_15((offset + 8u), value[1u]);
  sb_store_15((offset + 16u), value[2u]);
}

fn sb_store_31(offset : u32, value : mat4x2<f16>) {
  sb_store_7((offset + 0u), value[0u]);
  sb_store_7((offset + 4u), value[1u]);
  sb_store_7((offset + 8u), value[2u]);
  sb_store_7((offset + 12u), value[3u]);
}

fn sb_store_32(offset : u32, value : mat4x3<f16>) {
  sb_store_11((offset + 0u), value[0u]);
  sb_store_11((offset + 8u), value[1u]);
  sb_store_11((offset + 16u), value[2u]);
  sb_store_11((offset + 24u), value[3u]);
}

fn sb_store_33(offset : u32, value : mat4x4<f16>) {
  sb_store_15((offset + 0u), value[0u]);
  sb_store_15((offset + 8u), value[1u]);
  sb_store_15((offset + 16u), value[2u]);
  sb_store_15((offset + 24u), value[3u]);
}

fn sb_store_34(offset : u32, value : array<vec3<f32>, 2u>) {
  var array_1 = value;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    sb_store_8((offset + (i * 16u)), array_1[i]);
  }
}

fn sb_store_35(offset : u32, value : array<mat4x2<f16>, 2u>) {
  var array_2 = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    sb_store_31((offset + (i_1 * 16u)), array_2[i_1]);
  }
}

@compute @workgroup_size(1)
fn main() {
  sb_store(0u, f32());
  sb_store_1(4u, i32());
  sb_store_2(8u, u32());
  sb_store_3(12u, f16());
  sb_store_4(16u, vec2<f32>());
  sb_store_5(24u, vec2<i32>());
  sb_store_6(32u, vec2<u32>());
  sb_store_7(40u, vec2<f16>());
  sb_store_8(48u, vec3<f32>());
  sb_store_9(64u, vec3<i32>());
  sb_store_10(80u, vec3<u32>());
  sb_store_11(96u, vec3<f16>());
  sb_store_12(112u, vec4<f32>());
  sb_store_13(128u, vec4<i32>());
  sb_store_14(144u, vec4<u32>());
  sb_store_15(160u, vec4<f16>());
  sb_store_16(168u, mat2x2<f32>());
  sb_store_17(192u, mat2x3<f32>());
  sb_store_18(224u, mat2x4<f32>());
  sb_store_19(256u, mat3x2<f32>());
  sb_store_20(288u, mat3x3<f32>());
  sb_store_21(336u, mat3x4<f32>());
  sb_store_22(384u, mat4x2<f32>());
  sb_store_23(416u, mat4x3<f32>());
  sb_store_24(480u, mat4x4<f32>());
  sb_store_25(544u, mat2x2<f16>());
  sb_store_26(552u, mat2x3<f16>());
  sb_store_27(568u, mat2x4<f16>());
  sb_store_28(584u, mat3x2<f16>());
  sb_store_29(600u, mat3x3<f16>());
  sb_store_30(624u, mat3x4<f16>());
  sb_store_31(648u, mat4x2<f16>());
  sb_store_32(664u, mat4x3<f16>());
  sb_store_33(696u, mat4x4<f16>());
  sb_store_34(736u, array<vec3<f32>, 2>());
  sb_store_35(768u, array<mat4x2<f16>, 2>());
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, LoadStructure) {
    auto* src = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var x : SB = sb;
}
)";

    auto* expect = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_1(offset : u32) -> f32

@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_2(offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_3(offset : u32) -> u32

@internal(intrinsic_load_storage_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_4(offset : u32) -> f16

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_5(offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_6(offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_7(offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_8(offset : u32) -> vec2<f16>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_9(offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_10(offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_11(offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_12(offset : u32) -> vec3<f16>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_13(offset : u32) -> vec4<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_14(offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_15(offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_16(offset : u32) -> vec4<f16>

fn sb_load_17(offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(sb_load_5((offset + 0u)), sb_load_5((offset + 8u)));
}

fn sb_load_18(offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(sb_load_9((offset + 0u)), sb_load_9((offset + 16u)));
}

fn sb_load_19(offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(sb_load_13((offset + 0u)), sb_load_13((offset + 16u)));
}

fn sb_load_20(offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(sb_load_5((offset + 0u)), sb_load_5((offset + 8u)), sb_load_5((offset + 16u)));
}

fn sb_load_21(offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(sb_load_9((offset + 0u)), sb_load_9((offset + 16u)), sb_load_9((offset + 32u)));
}

fn sb_load_22(offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(sb_load_13((offset + 0u)), sb_load_13((offset + 16u)), sb_load_13((offset + 32u)));
}

fn sb_load_23(offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(sb_load_5((offset + 0u)), sb_load_5((offset + 8u)), sb_load_5((offset + 16u)), sb_load_5((offset + 24u)));
}

fn sb_load_24(offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(sb_load_9((offset + 0u)), sb_load_9((offset + 16u)), sb_load_9((offset + 32u)), sb_load_9((offset + 48u)));
}

fn sb_load_25(offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(sb_load_13((offset + 0u)), sb_load_13((offset + 16u)), sb_load_13((offset + 32u)), sb_load_13((offset + 48u)));
}

fn sb_load_26(offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(sb_load_8((offset + 0u)), sb_load_8((offset + 4u)));
}

fn sb_load_27(offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(sb_load_12((offset + 0u)), sb_load_12((offset + 8u)));
}

fn sb_load_28(offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(sb_load_16((offset + 0u)), sb_load_16((offset + 8u)));
}

fn sb_load_29(offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(sb_load_8((offset + 0u)), sb_load_8((offset + 4u)), sb_load_8((offset + 8u)));
}

fn sb_load_30(offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(sb_load_12((offset + 0u)), sb_load_12((offset + 8u)), sb_load_12((offset + 16u)));
}

fn sb_load_31(offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(sb_load_16((offset + 0u)), sb_load_16((offset + 8u)), sb_load_16((offset + 16u)));
}

fn sb_load_32(offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(sb_load_8((offset + 0u)), sb_load_8((offset + 4u)), sb_load_8((offset + 8u)), sb_load_8((offset + 12u)));
}

fn sb_load_33(offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(sb_load_12((offset + 0u)), sb_load_12((offset + 8u)), sb_load_12((offset + 16u)), sb_load_12((offset + 24u)));
}

fn sb_load_34(offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(sb_load_16((offset + 0u)), sb_load_16((offset + 8u)), sb_load_16((offset + 16u)), sb_load_16((offset + 24u)));
}

fn sb_load_35(offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = sb_load_9((offset + (i * 16u)));
  }
  return arr;
}

fn sb_load_36(offset : u32) -> array<mat4x2<f16>, 2u> {
  var arr_1 : array<mat4x2<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = sb_load_32((offset + (i_1 * 16u)));
  }
  return arr_1;
}

fn sb_load(offset : u32) -> SB {
  return SB(sb_load_1((offset + 0u)), sb_load_2((offset + 4u)), sb_load_3((offset + 8u)), sb_load_4((offset + 12u)), sb_load_5((offset + 16u)), sb_load_6((offset + 24u)), sb_load_7((offset + 32u)), sb_load_8((offset + 40u)), sb_load_9((offset + 48u)), sb_load_10((offset + 64u)), sb_load_11((offset + 80u)), sb_load_12((offset + 96u)), sb_load_13((offset + 112u)), sb_load_14((offset + 128u)), sb_load_15((offset + 144u)), sb_load_16((offset + 160u)), sb_load_17((offset + 168u)), sb_load_18((offset + 192u)), sb_load_19((offset + 224u)), sb_load_20((offset + 256u)), sb_load_21((offset + 288u)), sb_load_22((offset + 336u)), sb_load_23((offset + 384u)), sb_load_24((offset + 416u)), sb_load_25((offset + 480u)), sb_load_26((offset + 544u)), sb_load_27((offset + 552u)), sb_load_28((offset + 568u)), sb_load_29((offset + 584u)), sb_load_30((offset + 600u)), sb_load_31((offset + 624u)), sb_load_32((offset + 648u)), sb_load_33((offset + 664u)), sb_load_34((offset + 696u)), sb_load_35((offset + 736u)), sb_load_36((offset + 768u)));
}

@compute @workgroup_size(1)
fn main() {
  var x : SB = sb_load(0u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, LoadStructure_OutOfOrder) {
    auto* src = R"(
enable f16;

@compute @workgroup_size(1)
fn main() {
  var x : SB = sb;
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};
)";

    auto* expect = R"(
enable f16;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_1(offset : u32) -> f32

@internal(intrinsic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_2(offset : u32) -> i32

@internal(intrinsic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_3(offset : u32) -> u32

@internal(intrinsic_load_storage_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_4(offset : u32) -> f16

@internal(intrinsic_load_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_5(offset : u32) -> vec2<f32>

@internal(intrinsic_load_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_6(offset : u32) -> vec2<i32>

@internal(intrinsic_load_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_7(offset : u32) -> vec2<u32>

@internal(intrinsic_load_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_8(offset : u32) -> vec2<f16>

@internal(intrinsic_load_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_9(offset : u32) -> vec3<f32>

@internal(intrinsic_load_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_10(offset : u32) -> vec3<i32>

@internal(intrinsic_load_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_11(offset : u32) -> vec3<u32>

@internal(intrinsic_load_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_12(offset : u32) -> vec3<f16>

@internal(intrinsic_load_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn sb_load_13(offset : u32) -> vec4<f32>

@internal(intrinsic_load_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn sb_load_14(offset : u32) -> vec4<i32>

@internal(intrinsic_load_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn sb_load_15(offset : u32) -> vec4<u32>

@internal(intrinsic_load_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn sb_load_16(offset : u32) -> vec4<f16>

fn sb_load_17(offset : u32) -> mat2x2<f32> {
  return mat2x2<f32>(sb_load_5((offset + 0u)), sb_load_5((offset + 8u)));
}

fn sb_load_18(offset : u32) -> mat2x3<f32> {
  return mat2x3<f32>(sb_load_9((offset + 0u)), sb_load_9((offset + 16u)));
}

fn sb_load_19(offset : u32) -> mat2x4<f32> {
  return mat2x4<f32>(sb_load_13((offset + 0u)), sb_load_13((offset + 16u)));
}

fn sb_load_20(offset : u32) -> mat3x2<f32> {
  return mat3x2<f32>(sb_load_5((offset + 0u)), sb_load_5((offset + 8u)), sb_load_5((offset + 16u)));
}

fn sb_load_21(offset : u32) -> mat3x3<f32> {
  return mat3x3<f32>(sb_load_9((offset + 0u)), sb_load_9((offset + 16u)), sb_load_9((offset + 32u)));
}

fn sb_load_22(offset : u32) -> mat3x4<f32> {
  return mat3x4<f32>(sb_load_13((offset + 0u)), sb_load_13((offset + 16u)), sb_load_13((offset + 32u)));
}

fn sb_load_23(offset : u32) -> mat4x2<f32> {
  return mat4x2<f32>(sb_load_5((offset + 0u)), sb_load_5((offset + 8u)), sb_load_5((offset + 16u)), sb_load_5((offset + 24u)));
}

fn sb_load_24(offset : u32) -> mat4x3<f32> {
  return mat4x3<f32>(sb_load_9((offset + 0u)), sb_load_9((offset + 16u)), sb_load_9((offset + 32u)), sb_load_9((offset + 48u)));
}

fn sb_load_25(offset : u32) -> mat4x4<f32> {
  return mat4x4<f32>(sb_load_13((offset + 0u)), sb_load_13((offset + 16u)), sb_load_13((offset + 32u)), sb_load_13((offset + 48u)));
}

fn sb_load_26(offset : u32) -> mat2x2<f16> {
  return mat2x2<f16>(sb_load_8((offset + 0u)), sb_load_8((offset + 4u)));
}

fn sb_load_27(offset : u32) -> mat2x3<f16> {
  return mat2x3<f16>(sb_load_12((offset + 0u)), sb_load_12((offset + 8u)));
}

fn sb_load_28(offset : u32) -> mat2x4<f16> {
  return mat2x4<f16>(sb_load_16((offset + 0u)), sb_load_16((offset + 8u)));
}

fn sb_load_29(offset : u32) -> mat3x2<f16> {
  return mat3x2<f16>(sb_load_8((offset + 0u)), sb_load_8((offset + 4u)), sb_load_8((offset + 8u)));
}

fn sb_load_30(offset : u32) -> mat3x3<f16> {
  return mat3x3<f16>(sb_load_12((offset + 0u)), sb_load_12((offset + 8u)), sb_load_12((offset + 16u)));
}

fn sb_load_31(offset : u32) -> mat3x4<f16> {
  return mat3x4<f16>(sb_load_16((offset + 0u)), sb_load_16((offset + 8u)), sb_load_16((offset + 16u)));
}

fn sb_load_32(offset : u32) -> mat4x2<f16> {
  return mat4x2<f16>(sb_load_8((offset + 0u)), sb_load_8((offset + 4u)), sb_load_8((offset + 8u)), sb_load_8((offset + 12u)));
}

fn sb_load_33(offset : u32) -> mat4x3<f16> {
  return mat4x3<f16>(sb_load_12((offset + 0u)), sb_load_12((offset + 8u)), sb_load_12((offset + 16u)), sb_load_12((offset + 24u)));
}

fn sb_load_34(offset : u32) -> mat4x4<f16> {
  return mat4x4<f16>(sb_load_16((offset + 0u)), sb_load_16((offset + 8u)), sb_load_16((offset + 16u)), sb_load_16((offset + 24u)));
}

fn sb_load_35(offset : u32) -> array<vec3<f32>, 2u> {
  var arr : array<vec3<f32>, 2u>;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    arr[i] = sb_load_9((offset + (i * 16u)));
  }
  return arr;
}

fn sb_load_36(offset : u32) -> array<mat4x2<f16>, 2u> {
  var arr_1 : array<mat4x2<f16>, 2u>;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    arr_1[i_1] = sb_load_32((offset + (i_1 * 16u)));
  }
  return arr_1;
}

fn sb_load(offset : u32) -> SB {
  return SB(sb_load_1((offset + 0u)), sb_load_2((offset + 4u)), sb_load_3((offset + 8u)), sb_load_4((offset + 12u)), sb_load_5((offset + 16u)), sb_load_6((offset + 24u)), sb_load_7((offset + 32u)), sb_load_8((offset + 40u)), sb_load_9((offset + 48u)), sb_load_10((offset + 64u)), sb_load_11((offset + 80u)), sb_load_12((offset + 96u)), sb_load_13((offset + 112u)), sb_load_14((offset + 128u)), sb_load_15((offset + 144u)), sb_load_16((offset + 160u)), sb_load_17((offset + 168u)), sb_load_18((offset + 192u)), sb_load_19((offset + 224u)), sb_load_20((offset + 256u)), sb_load_21((offset + 288u)), sb_load_22((offset + 336u)), sb_load_23((offset + 384u)), sb_load_24((offset + 416u)), sb_load_25((offset + 480u)), sb_load_26((offset + 544u)), sb_load_27((offset + 552u)), sb_load_28((offset + 568u)), sb_load_29((offset + 584u)), sb_load_30((offset + 600u)), sb_load_31((offset + 624u)), sb_load_32((offset + 648u)), sb_load_33((offset + 664u)), sb_load_34((offset + 696u)), sb_load_35((offset + 736u)), sb_load_36((offset + 768u)));
}

@compute @workgroup_size(1)
fn main() {
  var x : SB = sb_load(0u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StoreStructure) {
    auto* src = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  sb = SB();
}
)";

    auto* expect = R"(
enable f16;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_1(offset : u32, value : f32)

@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_2(offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_3(offset : u32, value : u32)

@internal(intrinsic_store_storage_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_4(offset : u32, value : f16)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_5(offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_6(offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_7(offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_8(offset : u32, value : vec2<f16>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_9(offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_10(offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_11(offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_12(offset : u32, value : vec3<f16>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_13(offset : u32, value : vec4<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_14(offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_15(offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_16(offset : u32, value : vec4<f16>)

fn sb_store_17(offset : u32, value : mat2x2<f32>) {
  sb_store_5((offset + 0u), value[0u]);
  sb_store_5((offset + 8u), value[1u]);
}

fn sb_store_18(offset : u32, value : mat2x3<f32>) {
  sb_store_9((offset + 0u), value[0u]);
  sb_store_9((offset + 16u), value[1u]);
}

fn sb_store_19(offset : u32, value : mat2x4<f32>) {
  sb_store_13((offset + 0u), value[0u]);
  sb_store_13((offset + 16u), value[1u]);
}

fn sb_store_20(offset : u32, value : mat3x2<f32>) {
  sb_store_5((offset + 0u), value[0u]);
  sb_store_5((offset + 8u), value[1u]);
  sb_store_5((offset + 16u), value[2u]);
}

fn sb_store_21(offset : u32, value : mat3x3<f32>) {
  sb_store_9((offset + 0u), value[0u]);
  sb_store_9((offset + 16u), value[1u]);
  sb_store_9((offset + 32u), value[2u]);
}

fn sb_store_22(offset : u32, value : mat3x4<f32>) {
  sb_store_13((offset + 0u), value[0u]);
  sb_store_13((offset + 16u), value[1u]);
  sb_store_13((offset + 32u), value[2u]);
}

fn sb_store_23(offset : u32, value : mat4x2<f32>) {
  sb_store_5((offset + 0u), value[0u]);
  sb_store_5((offset + 8u), value[1u]);
  sb_store_5((offset + 16u), value[2u]);
  sb_store_5((offset + 24u), value[3u]);
}

fn sb_store_24(offset : u32, value : mat4x3<f32>) {
  sb_store_9((offset + 0u), value[0u]);
  sb_store_9((offset + 16u), value[1u]);
  sb_store_9((offset + 32u), value[2u]);
  sb_store_9((offset + 48u), value[3u]);
}

fn sb_store_25(offset : u32, value : mat4x4<f32>) {
  sb_store_13((offset + 0u), value[0u]);
  sb_store_13((offset + 16u), value[1u]);
  sb_store_13((offset + 32u), value[2u]);
  sb_store_13((offset + 48u), value[3u]);
}

fn sb_store_26(offset : u32, value : mat2x2<f16>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 4u), value[1u]);
}

fn sb_store_27(offset : u32, value : mat2x3<f16>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 8u), value[1u]);
}

fn sb_store_28(offset : u32, value : mat2x4<f16>) {
  sb_store_16((offset + 0u), value[0u]);
  sb_store_16((offset + 8u), value[1u]);
}

fn sb_store_29(offset : u32, value : mat3x2<f16>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 4u), value[1u]);
  sb_store_8((offset + 8u), value[2u]);
}

fn sb_store_30(offset : u32, value : mat3x3<f16>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 8u), value[1u]);
  sb_store_12((offset + 16u), value[2u]);
}

fn sb_store_31(offset : u32, value : mat3x4<f16>) {
  sb_store_16((offset + 0u), value[0u]);
  sb_store_16((offset + 8u), value[1u]);
  sb_store_16((offset + 16u), value[2u]);
}

fn sb_store_32(offset : u32, value : mat4x2<f16>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 4u), value[1u]);
  sb_store_8((offset + 8u), value[2u]);
  sb_store_8((offset + 12u), value[3u]);
}

fn sb_store_33(offset : u32, value : mat4x3<f16>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 8u), value[1u]);
  sb_store_12((offset + 16u), value[2u]);
  sb_store_12((offset + 24u), value[3u]);
}

fn sb_store_34(offset : u32, value : mat4x4<f16>) {
  sb_store_16((offset + 0u), value[0u]);
  sb_store_16((offset + 8u), value[1u]);
  sb_store_16((offset + 16u), value[2u]);
  sb_store_16((offset + 24u), value[3u]);
}

fn sb_store_35(offset : u32, value : array<vec3<f32>, 2u>) {
  var array_1 = value;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    sb_store_9((offset + (i * 16u)), array_1[i]);
  }
}

fn sb_store_36(offset : u32, value : array<mat4x2<f16>, 2u>) {
  var array_2 = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    sb_store_32((offset + (i_1 * 16u)), array_2[i_1]);
  }
}

fn sb_store(offset : u32, value : SB) {
  sb_store_1((offset + 0u), value.scalar_f32);
  sb_store_2((offset + 4u), value.scalar_i32);
  sb_store_3((offset + 8u), value.scalar_u32);
  sb_store_4((offset + 12u), value.scalar_f16);
  sb_store_5((offset + 16u), value.vec2_f32);
  sb_store_6((offset + 24u), value.vec2_i32);
  sb_store_7((offset + 32u), value.vec2_u32);
  sb_store_8((offset + 40u), value.vec2_f16);
  sb_store_9((offset + 48u), value.vec3_f32);
  sb_store_10((offset + 64u), value.vec3_i32);
  sb_store_11((offset + 80u), value.vec3_u32);
  sb_store_12((offset + 96u), value.vec3_f16);
  sb_store_13((offset + 112u), value.vec4_f32);
  sb_store_14((offset + 128u), value.vec4_i32);
  sb_store_15((offset + 144u), value.vec4_u32);
  sb_store_16((offset + 160u), value.vec4_f16);
  sb_store_17((offset + 168u), value.mat2x2_f32);
  sb_store_18((offset + 192u), value.mat2x3_f32);
  sb_store_19((offset + 224u), value.mat2x4_f32);
  sb_store_20((offset + 256u), value.mat3x2_f32);
  sb_store_21((offset + 288u), value.mat3x3_f32);
  sb_store_22((offset + 336u), value.mat3x4_f32);
  sb_store_23((offset + 384u), value.mat4x2_f32);
  sb_store_24((offset + 416u), value.mat4x3_f32);
  sb_store_25((offset + 480u), value.mat4x4_f32);
  sb_store_26((offset + 544u), value.mat2x2_f16);
  sb_store_27((offset + 552u), value.mat2x3_f16);
  sb_store_28((offset + 568u), value.mat2x4_f16);
  sb_store_29((offset + 584u), value.mat3x2_f16);
  sb_store_30((offset + 600u), value.mat3x3_f16);
  sb_store_31((offset + 624u), value.mat3x4_f16);
  sb_store_32((offset + 648u), value.mat4x2_f16);
  sb_store_33((offset + 664u), value.mat4x3_f16);
  sb_store_34((offset + 696u), value.mat4x4_f16);
  sb_store_35((offset + 736u), value.arr2_vec3_f32);
  sb_store_36((offset + 768u), value.arr2_mat4x2_f16);
}

@compute @workgroup_size(1)
fn main() {
  sb_store(0u, SB());
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StoreStructure_OutOfOrder) {
    auto* src = R"(
enable f16;

@compute @workgroup_size(1)
fn main() {
  sb = SB();
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
};
)";

    auto* expect = R"(
enable f16;

@internal(intrinsic_store_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_1(offset : u32, value : f32)

@internal(intrinsic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_2(offset : u32, value : i32)

@internal(intrinsic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_3(offset : u32, value : u32)

@internal(intrinsic_store_storage_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_4(offset : u32, value : f16)

@internal(intrinsic_store_storage_vec2_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_5(offset : u32, value : vec2<f32>)

@internal(intrinsic_store_storage_vec2_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_6(offset : u32, value : vec2<i32>)

@internal(intrinsic_store_storage_vec2_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_7(offset : u32, value : vec2<u32>)

@internal(intrinsic_store_storage_vec2_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_8(offset : u32, value : vec2<f16>)

@internal(intrinsic_store_storage_vec3_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_9(offset : u32, value : vec3<f32>)

@internal(intrinsic_store_storage_vec3_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_10(offset : u32, value : vec3<i32>)

@internal(intrinsic_store_storage_vec3_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_11(offset : u32, value : vec3<u32>)

@internal(intrinsic_store_storage_vec3_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_12(offset : u32, value : vec3<f16>)

@internal(intrinsic_store_storage_vec4_f32) @internal(disable_validation__function_has_no_body)
fn sb_store_13(offset : u32, value : vec4<f32>)

@internal(intrinsic_store_storage_vec4_i32) @internal(disable_validation__function_has_no_body)
fn sb_store_14(offset : u32, value : vec4<i32>)

@internal(intrinsic_store_storage_vec4_u32) @internal(disable_validation__function_has_no_body)
fn sb_store_15(offset : u32, value : vec4<u32>)

@internal(intrinsic_store_storage_vec4_f16) @internal(disable_validation__function_has_no_body)
fn sb_store_16(offset : u32, value : vec4<f16>)

fn sb_store_17(offset : u32, value : mat2x2<f32>) {
  sb_store_5((offset + 0u), value[0u]);
  sb_store_5((offset + 8u), value[1u]);
}

fn sb_store_18(offset : u32, value : mat2x3<f32>) {
  sb_store_9((offset + 0u), value[0u]);
  sb_store_9((offset + 16u), value[1u]);
}

fn sb_store_19(offset : u32, value : mat2x4<f32>) {
  sb_store_13((offset + 0u), value[0u]);
  sb_store_13((offset + 16u), value[1u]);
}

fn sb_store_20(offset : u32, value : mat3x2<f32>) {
  sb_store_5((offset + 0u), value[0u]);
  sb_store_5((offset + 8u), value[1u]);
  sb_store_5((offset + 16u), value[2u]);
}

fn sb_store_21(offset : u32, value : mat3x3<f32>) {
  sb_store_9((offset + 0u), value[0u]);
  sb_store_9((offset + 16u), value[1u]);
  sb_store_9((offset + 32u), value[2u]);
}

fn sb_store_22(offset : u32, value : mat3x4<f32>) {
  sb_store_13((offset + 0u), value[0u]);
  sb_store_13((offset + 16u), value[1u]);
  sb_store_13((offset + 32u), value[2u]);
}

fn sb_store_23(offset : u32, value : mat4x2<f32>) {
  sb_store_5((offset + 0u), value[0u]);
  sb_store_5((offset + 8u), value[1u]);
  sb_store_5((offset + 16u), value[2u]);
  sb_store_5((offset + 24u), value[3u]);
}

fn sb_store_24(offset : u32, value : mat4x3<f32>) {
  sb_store_9((offset + 0u), value[0u]);
  sb_store_9((offset + 16u), value[1u]);
  sb_store_9((offset + 32u), value[2u]);
  sb_store_9((offset + 48u), value[3u]);
}

fn sb_store_25(offset : u32, value : mat4x4<f32>) {
  sb_store_13((offset + 0u), value[0u]);
  sb_store_13((offset + 16u), value[1u]);
  sb_store_13((offset + 32u), value[2u]);
  sb_store_13((offset + 48u), value[3u]);
}

fn sb_store_26(offset : u32, value : mat2x2<f16>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 4u), value[1u]);
}

fn sb_store_27(offset : u32, value : mat2x3<f16>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 8u), value[1u]);
}

fn sb_store_28(offset : u32, value : mat2x4<f16>) {
  sb_store_16((offset + 0u), value[0u]);
  sb_store_16((offset + 8u), value[1u]);
}

fn sb_store_29(offset : u32, value : mat3x2<f16>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 4u), value[1u]);
  sb_store_8((offset + 8u), value[2u]);
}

fn sb_store_30(offset : u32, value : mat3x3<f16>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 8u), value[1u]);
  sb_store_12((offset + 16u), value[2u]);
}

fn sb_store_31(offset : u32, value : mat3x4<f16>) {
  sb_store_16((offset + 0u), value[0u]);
  sb_store_16((offset + 8u), value[1u]);
  sb_store_16((offset + 16u), value[2u]);
}

fn sb_store_32(offset : u32, value : mat4x2<f16>) {
  sb_store_8((offset + 0u), value[0u]);
  sb_store_8((offset + 4u), value[1u]);
  sb_store_8((offset + 8u), value[2u]);
  sb_store_8((offset + 12u), value[3u]);
}

fn sb_store_33(offset : u32, value : mat4x3<f16>) {
  sb_store_12((offset + 0u), value[0u]);
  sb_store_12((offset + 8u), value[1u]);
  sb_store_12((offset + 16u), value[2u]);
  sb_store_12((offset + 24u), value[3u]);
}

fn sb_store_34(offset : u32, value : mat4x4<f16>) {
  sb_store_16((offset + 0u), value[0u]);
  sb_store_16((offset + 8u), value[1u]);
  sb_store_16((offset + 16u), value[2u]);
  sb_store_16((offset + 24u), value[3u]);
}

fn sb_store_35(offset : u32, value : array<vec3<f32>, 2u>) {
  var array_1 = value;
  for(var i = 0u; (i < 2u); i = (i + 1u)) {
    sb_store_9((offset + (i * 16u)), array_1[i]);
  }
}

fn sb_store_36(offset : u32, value : array<mat4x2<f16>, 2u>) {
  var array_2 = value;
  for(var i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
    sb_store_32((offset + (i_1 * 16u)), array_2[i_1]);
  }
}

fn sb_store(offset : u32, value : SB) {
  sb_store_1((offset + 0u), value.scalar_f32);
  sb_store_2((offset + 4u), value.scalar_i32);
  sb_store_3((offset + 8u), value.scalar_u32);
  sb_store_4((offset + 12u), value.scalar_f16);
  sb_store_5((offset + 16u), value.vec2_f32);
  sb_store_6((offset + 24u), value.vec2_i32);
  sb_store_7((offset + 32u), value.vec2_u32);
  sb_store_8((offset + 40u), value.vec2_f16);
  sb_store_9((offset + 48u), value.vec3_f32);
  sb_store_10((offset + 64u), value.vec3_i32);
  sb_store_11((offset + 80u), value.vec3_u32);
  sb_store_12((offset + 96u), value.vec3_f16);
  sb_store_13((offset + 112u), value.vec4_f32);
  sb_store_14((offset + 128u), value.vec4_i32);
  sb_store_15((offset + 144u), value.vec4_u32);
  sb_store_16((offset + 160u), value.vec4_f16);
  sb_store_17((offset + 168u), value.mat2x2_f32);
  sb_store_18((offset + 192u), value.mat2x3_f32);
  sb_store_19((offset + 224u), value.mat2x4_f32);
  sb_store_20((offset + 256u), value.mat3x2_f32);
  sb_store_21((offset + 288u), value.mat3x3_f32);
  sb_store_22((offset + 336u), value.mat3x4_f32);
  sb_store_23((offset + 384u), value.mat4x2_f32);
  sb_store_24((offset + 416u), value.mat4x3_f32);
  sb_store_25((offset + 480u), value.mat4x4_f32);
  sb_store_26((offset + 544u), value.mat2x2_f16);
  sb_store_27((offset + 552u), value.mat2x3_f16);
  sb_store_28((offset + 568u), value.mat2x4_f16);
  sb_store_29((offset + 584u), value.mat3x2_f16);
  sb_store_30((offset + 600u), value.mat3x3_f16);
  sb_store_31((offset + 624u), value.mat3x4_f16);
  sb_store_32((offset + 648u), value.mat4x2_f16);
  sb_store_33((offset + 664u), value.mat4x3_f16);
  sb_store_34((offset + 696u), value.mat4x4_f16);
  sb_store_35((offset + 736u), value.arr2_vec3_f32);
  sb_store_36((offset + 768u), value.arr2_mat4x2_f16);
}

@compute @workgroup_size(1)
fn main() {
  sb_store(0u, SB());
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  scalar_f16 : f16,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec2_f16 : vec2<f16>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec3_f16 : vec3<f16>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  vec4_f16 : vec4<f16>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  mat2x2_f16 : mat2x2<f16>,
  mat2x3_f16 : mat2x3<f16>,
  mat2x4_f16 : mat2x4<f16>,
  mat3x2_f16 : mat3x2<f16>,
  mat3x3_f16 : mat3x3<f16>,
  mat3x4_f16 : mat3x4<f16>,
  mat4x2_f16 : mat4x2<f16>,
  mat4x3_f16 : mat4x3<f16>,
  mat4x4_f16 : mat4x4<f16>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  arr2_mat4x2_f16 : array<mat4x2<f16>, 2>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexStaticAccessChain) {
    auto* src = R"(
// sizeof(S1) == 32
// alignof(S1) == 16
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};

// sizeof(S2) == 116
// alignof(S2) == 16
struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
};

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var x : f32 = sb.b[4].b[1].b.z;
}
)";

    // sb.b[4].b[1].b.z
    //    ^  ^ ^  ^ ^ ^
    //    |  | |  | | |
    //  128  | |688 | 712
    //       | |    |
    //     640 656  704

    auto* expect = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
}

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load(offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var x : f32 = sb_load(712u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexStaticAccessChain_ViaPointerDot) {
    auto* src = R"(
// sizeof(S1) == 32
// alignof(S1) == 16
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};

// sizeof(S2) == 116
// alignof(S2) == 16
struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
};

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  let p = &sb;
  var x : f32 = (*p).b[4].b[1].b.z;
}
)";

    // sb.b[4].b[1].b.z
    //    ^  ^ ^  ^ ^ ^
    //    |  | |  | | |
    //  128  | |688 | 712
    //       | |    |
    //     640 656  704

    auto* expect = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
}

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load(offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var x : f32 = sb_load(712u);
}
)";

    auto got = Run<ast::transform::SimplifyPointers, DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexStaticAccessChain_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var x : f32 = sb.b[4].b[1].b.z;
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
};

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
};

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};
)";

    // sb.b[4].b[1].b.z
    //    ^  ^ ^  ^ ^ ^
    //    |  | |  | | |
    //  128  | |688 | 712
    //       | |    |
    //     640 656  704

    auto* expect = R"(
@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load(offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var x : f32 = sb_load(712u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
}

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
}

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexDynamicAccessChain) {
    auto* src = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
};

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb.b[i].b[j].b[k];
}
)";

    auto* expect = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
}

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load(offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb_load((((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexDynamicAccessChain_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb.b[i].b[j].b[k];
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : array<S2>
};

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
};

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};
)";

    auto* expect = R"(
@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load(offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb_load((((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : array<S2>,
}

struct S2 {
  a : i32,
  b : array<S1, 3>,
  c : i32,
}

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexDynamicAccessChainWithAliases) {
    auto* src = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};

alias A1 = S1;

alias A1_Array = array<S1, 3>;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
};

alias A2 = S2;

alias A2_Array = array<S2>;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb.b[i].b[j].b[k];
}
)";

    auto* expect = R"(
struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}

alias A1 = S1;

alias A1_Array = array<S1, 3>;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
}

alias A2 = S2;

alias A2_Array = array<S2>;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load(offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb_load((((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, ComplexDynamicAccessChainWithAliases_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb.b[i].b[j].b[k];
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
};

alias A2_Array = array<S2>;

alias A2 = S2;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
};

alias A1 = S1;

alias A1_Array = array<S1, 3>;

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
};
)";

    auto* expect = R"(
@internal(intrinsic_load_storage_f32) @internal(disable_validation__function_has_no_body)
fn sb_load(offset : u32) -> f32

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 4;
  var j : u32 = 1u;
  var k : i32 = 2;
  var x : f32 = sb_load((((((128u + (128u * u32(i))) + 16u) + (32u * j)) + 16u) + (4u * u32(k))));
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  @size(128)
  a : i32,
  b : A2_Array,
}

alias A2_Array = array<S2>;

alias A2 = S2;

struct S2 {
  a : i32,
  b : A1_Array,
  c : i32,
}

alias A1 = S1;

alias A1_Array = array<S1, 3>;

struct S1 {
  a : i32,
  b : vec3<f32>,
  c : i32,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StorageBufferAtomics) {
    auto* src = R"(
struct SB {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
};

@group(0) @binding(0) var<storage, read_write> sb : SB;

@compute @workgroup_size(1)
fn main() {
  atomicStore(&sb.a, 123);
  atomicLoad(&sb.a);
  atomicAdd(&sb.a, 123);
  atomicSub(&sb.a, 123);
  atomicMax(&sb.a, 123);
  atomicMin(&sb.a, 123);
  atomicAnd(&sb.a, 123);
  atomicOr(&sb.a, 123);
  atomicXor(&sb.a, 123);
  atomicExchange(&sb.a, 123);
  atomicCompareExchangeWeak(&sb.a, 123, 345);

  atomicStore(&sb.b, 123u);
  atomicLoad(&sb.b);
  atomicAdd(&sb.b, 123u);
  atomicSub(&sb.b, 123u);
  atomicMax(&sb.b, 123u);
  atomicMin(&sb.b, 123u);
  atomicAnd(&sb.b, 123u);
  atomicOr(&sb.b, 123u);
  atomicXor(&sb.b, 123u);
  atomicExchange(&sb.b, 123u);
  atomicCompareExchangeWeak(&sb.b, 123u, 345u);
}
)";

    auto* expect = R"(
struct SB {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

@internal(intrinsic_atomic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicStore(offset : u32, param_1 : i32)

@internal(intrinsic_atomic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicLoad(offset : u32) -> i32

@internal(intrinsic_atomic_add_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicAdd(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_sub_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicSub(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_max_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicMax(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_min_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicMin(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_and_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicAnd(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_or_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicOr(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_xor_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicXor(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_exchange_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicExchange(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_compare_exchange_weak_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicCompareExchangeWeak(offset : u32, param_1 : i32, param_2 : i32) -> __atomic_compare_exchange_result_i32

@internal(intrinsic_atomic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicStore_1(offset : u32, param_1 : u32)

@internal(intrinsic_atomic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicLoad_1(offset : u32) -> u32

@internal(intrinsic_atomic_add_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicAdd_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_sub_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicSub_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_max_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicMax_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_min_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicMin_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_and_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicAnd_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_or_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicOr_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_xor_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicXor_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_exchange_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicExchange_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_compare_exchange_weak_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicCompareExchangeWeak_1(offset : u32, param_1 : u32, param_2 : u32) -> __atomic_compare_exchange_result_u32

@compute @workgroup_size(1)
fn main() {
  sbatomicStore(16u, 123);
  sbatomicLoad(16u);
  sbatomicAdd(16u, 123);
  sbatomicSub(16u, 123);
  sbatomicMax(16u, 123);
  sbatomicMin(16u, 123);
  sbatomicAnd(16u, 123);
  sbatomicOr(16u, 123);
  sbatomicXor(16u, 123);
  sbatomicExchange(16u, 123);
  sbatomicCompareExchangeWeak(16u, 123, 345);
  sbatomicStore_1(20u, 123u);
  sbatomicLoad_1(20u);
  sbatomicAdd_1(20u, 123u);
  sbatomicSub_1(20u, 123u);
  sbatomicMax_1(20u, 123u);
  sbatomicMin_1(20u, 123u);
  sbatomicAnd_1(20u, 123u);
  sbatomicOr_1(20u, 123u);
  sbatomicXor_1(20u, 123u);
  sbatomicExchange_1(20u, 123u);
  sbatomicCompareExchangeWeak_1(20u, 123u, 345u);
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, StorageBufferAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  atomicStore(&sb.a, 123);
  atomicLoad(&sb.a);
  atomicAdd(&sb.a, 123);
  atomicSub(&sb.a, 123);
  atomicMax(&sb.a, 123);
  atomicMin(&sb.a, 123);
  atomicAnd(&sb.a, 123);
  atomicOr(&sb.a, 123);
  atomicXor(&sb.a, 123);
  atomicExchange(&sb.a, 123);
  atomicCompareExchangeWeak(&sb.a, 123, 345);

  atomicStore(&sb.b, 123u);
  atomicLoad(&sb.b);
  atomicAdd(&sb.b, 123u);
  atomicSub(&sb.b, 123u);
  atomicMax(&sb.b, 123u);
  atomicMin(&sb.b, 123u);
  atomicAnd(&sb.b, 123u);
  atomicOr(&sb.b, 123u);
  atomicXor(&sb.b, 123u);
  atomicExchange(&sb.b, 123u);
  atomicCompareExchangeWeak(&sb.b, 123u, 345u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
};
)";

    auto* expect = R"(
@internal(intrinsic_atomic_store_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicStore(offset : u32, param_1 : i32)

@internal(intrinsic_atomic_load_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicLoad(offset : u32) -> i32

@internal(intrinsic_atomic_add_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicAdd(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_sub_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicSub(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_max_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicMax(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_min_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicMin(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_and_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicAnd(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_or_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicOr(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_xor_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicXor(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_exchange_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicExchange(offset : u32, param_1 : i32) -> i32

@internal(intrinsic_atomic_compare_exchange_weak_storage_i32) @internal(disable_validation__function_has_no_body)
fn sbatomicCompareExchangeWeak(offset : u32, param_1 : i32, param_2 : i32) -> __atomic_compare_exchange_result_i32

@internal(intrinsic_atomic_store_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicStore_1(offset : u32, param_1 : u32)

@internal(intrinsic_atomic_load_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicLoad_1(offset : u32) -> u32

@internal(intrinsic_atomic_add_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicAdd_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_sub_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicSub_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_max_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicMax_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_min_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicMin_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_and_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicAnd_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_or_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicOr_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_xor_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicXor_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_exchange_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicExchange_1(offset : u32, param_1 : u32) -> u32

@internal(intrinsic_atomic_compare_exchange_weak_storage_u32) @internal(disable_validation__function_has_no_body)
fn sbatomicCompareExchangeWeak_1(offset : u32, param_1 : u32, param_2 : u32) -> __atomic_compare_exchange_result_u32

@compute @workgroup_size(1)
fn main() {
  sbatomicStore(16u, 123);
  sbatomicLoad(16u);
  sbatomicAdd(16u, 123);
  sbatomicSub(16u, 123);
  sbatomicMax(16u, 123);
  sbatomicMin(16u, 123);
  sbatomicAnd(16u, 123);
  sbatomicOr(16u, 123);
  sbatomicXor(16u, 123);
  sbatomicExchange(16u, 123);
  sbatomicCompareExchangeWeak(16u, 123, 345);
  sbatomicStore_1(20u, 123u);
  sbatomicLoad_1(20u);
  sbatomicAdd_1(20u, 123u);
  sbatomicSub_1(20u, 123u);
  sbatomicMax_1(20u, 123u);
  sbatomicMin_1(20u, 123u);
  sbatomicAnd_1(20u, 123u);
  sbatomicOr_1(20u, 123u);
  sbatomicXor_1(20u, 123u);
  sbatomicExchange_1(20u, 123u);
  sbatomicCompareExchangeWeak_1(20u, 123u, 345u);
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

struct SB {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
}
)";

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, WorkgroupBufferAtomics) {
    auto* src = R"(
struct S {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
}

var<workgroup> w : S;

@compute @workgroup_size(1)
fn main() {
  atomicStore(&(w.a), 123);
  atomicLoad(&(w.a));
  atomicAdd(&(w.a), 123);
  atomicSub(&(w.a), 123);
  atomicMax(&(w.a), 123);
  atomicMin(&(w.a), 123);
  atomicAnd(&(w.a), 123);
  atomicOr(&(w.a), 123);
  atomicXor(&(w.a), 123);
  atomicExchange(&(w.a), 123);
  atomicCompareExchangeWeak(&(w.a), 123, 345);
  atomicStore(&(w.b), 123u);
  atomicLoad(&(w.b));
  atomicAdd(&(w.b), 123u);
  atomicSub(&(w.b), 123u);
  atomicMax(&(w.b), 123u);
  atomicMin(&(w.b), 123u);
  atomicAnd(&(w.b), 123u);
  atomicOr(&(w.b), 123u);
  atomicXor(&(w.b), 123u);
  atomicExchange(&(w.b), 123u);
  atomicCompareExchangeWeak(&(w.b), 123u, 345u);
}
)";

    auto* expect = src;

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeMemoryAccessTest, WorkgroupBufferAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  atomicStore(&(w.a), 123);
  atomicLoad(&(w.a));
  atomicAdd(&(w.a), 123);
  atomicSub(&(w.a), 123);
  atomicMax(&(w.a), 123);
  atomicMin(&(w.a), 123);
  atomicAnd(&(w.a), 123);
  atomicOr(&(w.a), 123);
  atomicXor(&(w.a), 123);
  atomicExchange(&(w.a), 123);
  atomicCompareExchangeWeak(&(w.a), 123, 345);
  atomicStore(&(w.b), 123u);
  atomicLoad(&(w.b));
  atomicAdd(&(w.b), 123u);
  atomicSub(&(w.b), 123u);
  atomicMax(&(w.b), 123u);
  atomicMin(&(w.b), 123u);
  atomicAnd(&(w.b), 123u);
  atomicOr(&(w.b), 123u);
  atomicXor(&(w.b), 123u);
  atomicExchange(&(w.b), 123u);
  atomicCompareExchangeWeak(&(w.b), 123u, 345u);
}

var<workgroup> w : S;

struct S {
  padding : vec4<f32>,
  a : atomic<i32>,
  b : atomic<u32>,
}
)";

    auto* expect = src;

    auto got = Run<DecomposeMemoryAccess>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::hlsl::writer
