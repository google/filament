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

#include "src/tint/lang/wgsl/ast/transform/demote_to_helper.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using DemoteToHelperTest = TransformTest;

TEST_F(DemoteToHelperTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<DemoteToHelper>(src));
}

TEST_F(DemoteToHelperTest, ShouldRunNoDiscard) {
    auto* src = R"(
@group(0) @binding(0)
var<storage, read_write> v : f32;

@fragment
fn foo() {
  v = 42;
}
)";

    EXPECT_FALSE(ShouldRun<DemoteToHelper>(src));
}

TEST_F(DemoteToHelperTest, ShouldRunDiscardInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0)
var<storage, read_write> v : f32;

@fragment
fn foo() {
  discard;
  v = 42;
}
)";

    EXPECT_TRUE(ShouldRun<DemoteToHelper>(src));
}

TEST_F(DemoteToHelperTest, ShouldRunDiscardInHelper) {
    auto* src = R"(
@group(0) @binding(0)
var<storage, read_write> v : f32;

fn bar() {
  discard;
}

@fragment
fn foo() {
  bar();
  v = 42;
}
)";

    EXPECT_TRUE(ShouldRun<DemoteToHelper>(src));
}

TEST_F(DemoteToHelperTest, EmptyModule) {
    auto* src = R"()";

    auto* expect = src;

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, WriteInEntryPoint_DiscardInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  v = ret.x;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    v = ret.x;
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, WriteInEntryPoint_DiscardInHelper) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar() {
  discard;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    bar();
  }
  let ret = textureSample(t, s, coord);
  v = ret.x;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar() {
  tint_discarded = true;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    bar();
  }
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    v = ret.x;
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, WriteInHelper_DiscardInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar(coord : vec2<f32>) {
  let ret = textureSample(t, s, coord);
  v = ret.x;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  bar(coord);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar(coord : vec2<f32>) {
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    v = ret.x;
  }
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  bar(coord);
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, WriteInHelper_DiscardInHelper) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar(in : f32, coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  v = ret.x;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  bar(in, coord);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar(in : f32, coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    v = ret.x;
  }
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  bar(in, coord);
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, WriteInEntryPoint_NoDiscard) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  let ret = textureSample(t, s, coord);
  v = ret.x;
}
)";

    auto* expect = src;

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that write via sugared pointer also discards
TEST_F(DemoteToHelperTest, WriteInEntryPoint_DiscardInEntryPoint_ViaPointerDot) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : vec4<f32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  let p = &v;
  p.x = ret.x;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : vec4<f32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  let p = &(v);
  if (!(tint_discarded)) {
    p.x = ret.x;
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that no additional discards are inserted when the function unconditionally returns in a
// nested block.
TEST_F(DemoteToHelperTest, EntryPointReturn_NestedInBlock) {
    auto* src = R"(
@fragment
fn foo() {
  {
    discard;
    return;
  }
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@fragment
fn foo() {
  {
    tint_discarded = true;
    if (tint_discarded) {
      discard;
    }
    return;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that a discard statement is inserted before every return statement in an entry point that
// contains a discard.
TEST_F(DemoteToHelperTest, EntryPointReturns_DiscardInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) f32 {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  if (in < 1.0) {
    return ret.x;
  }
  return 2.0;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) f32 {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  if ((in < 1.0)) {
    if (tint_discarded) {
      discard;
    }
    return ret.x;
  }
  if (tint_discarded) {
    discard;
  }
  return 2.0;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that a discard statement is inserted before every return statement in an entry point that
// calls a function that contains a discard.
TEST_F(DemoteToHelperTest, EntryPointReturns_DiscardInHelper) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar() {
  discard;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) f32 {
  if (in == 0.0) {
    bar();
  }
  let ret = textureSample(t, s, coord);
  if (in < 1.0) {
    return ret.x;
  }
  return 2.0;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar() {
  tint_discarded = true;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) f32 {
  if ((in == 0.0)) {
    bar();
  }
  let ret = textureSample(t, s, coord);
  if ((in < 1.0)) {
    if (tint_discarded) {
      discard;
    }
    return ret.x;
  }
  if (tint_discarded) {
    discard;
  }
  return 2.0;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that no return statements are modified in an entry point that does not discard.
TEST_F(DemoteToHelperTest, EntryPointReturns_NoDiscard) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar() {
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) f32 {
  if ((in == 0.0)) {
    bar();
  }
  let ret = textureSample(t, s, coord);
  if ((in < 1.0)) {
    return ret.x;
  }
  return 2.0;
}
)";

    auto* expect = src;

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that only functions that are part of a shader that discards are transformed.
// Functions in non-discarding stages should not have their writes masked, and non-discarding entry
// points should not have their return statements replaced.
TEST_F(DemoteToHelperTest, MultipleShaders) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v1 : f32;

@group(0) @binding(3) var<storage, read_write> v2 : f32;

fn bar_discard(in : f32, coord : vec2<f32>) -> f32 {
  let ret = textureSample(t, s, coord);
  v1 = ret.x * 2.0;
  return ret.y * 2.0;
}

fn bar_no_discard(in : f32, coord : vec2<f32>) -> f32 {
  let ret = textureSample(t, s, coord);
  v1 = ret.x * 2.0;
  return ret.y * 2.0;
}

@fragment
fn foo_discard(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = bar_discard(in, coord);
  v2 = ret;
}

@fragment
fn foo_no_discard(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  let ret = bar_no_discard(in, coord);
  if (in == 0.0) {
    return;
  }
  v2 = ret;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v1 : f32;

@group(0) @binding(3) var<storage, read_write> v2 : f32;

fn bar_discard(in : f32, coord : vec2<f32>) -> f32 {
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    v1 = (ret.x * 2.0);
  }
  return (ret.y * 2.0);
}

fn bar_no_discard(in : f32, coord : vec2<f32>) -> f32 {
  let ret = textureSample(t, s, coord);
  v1 = (ret.x * 2.0);
  return (ret.y * 2.0);
}

@fragment
fn foo_discard(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = bar_discard(in, coord);
  if (!(tint_discarded)) {
    v2 = ret;
  }
  if (tint_discarded) {
    discard;
  }
}

@fragment
fn foo_no_discard(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  let ret = bar_no_discard(in, coord);
  if ((in == 0.0)) {
    return;
  }
  v2 = ret;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that we do not mask writes to invocation-private address spaces.
TEST_F(DemoteToHelperTest, InvocationPrivateWrites) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

var<private> vp : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  var vf : f32;
  vf = ret.x;
  vp = ret.y;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

var<private> vp : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  var vf : f32;
  vf = ret.x;
  vp = ret.y;
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that we do not mask writes to invocation-private address spaces via a sugared pointer write
TEST_F(DemoteToHelperTest, InvocationPrivateWritesViaPointerDot) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

var<private> vp : vec4<f32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  var vf : f32;
  vf = ret.x;
  let p = &vp;
  p.x = ret.x;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

var<private> vp : vec4<f32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  var vf : f32;
  vf = ret.x;
  let p = &(vp);
  p.x = ret.x;
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, TextureStoreInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var t2 : texture_storage_2d<rgba8unorm, write>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  textureStore(t2, vec2<u32>(coord), ret);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var t2 : texture_storage_2d<rgba8unorm, write>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    textureStore(t2, vec2<u32>(coord), ret);
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, TextureStoreInHelper) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var t2 : texture_storage_2d<rgba8unorm, write>;

fn bar(coord : vec2<f32>, value : vec4<f32>) {
  textureStore(t2, vec2<u32>(coord), value);
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  bar(coord, ret);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var t2 : texture_storage_2d<rgba8unorm, write>;

fn bar(coord : vec2<f32>, value : vec4<f32>) {
  if (!(tint_discarded)) {
    textureStore(t2, vec2<u32>(coord), value);
  }
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  bar(coord, ret);
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, TextureStore_NoDiscard) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var t2 : texture_storage_2d<rgba8unorm, write>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  let ret = textureSample(t, s, coord);
  textureStore(t2, vec2<u32>(coord), ret);
}
)";

    auto* expect = src;

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicStoreInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  atomicStore(&a, i32(ret.x));
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    atomicStore(&(a), i32(ret.x));
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicStoreInHelper) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

fn bar(value : vec4<f32>) {
  atomicStore(&a, i32(value.x));
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  bar(ret);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

fn bar(value : vec4<f32>) {
  if (!(tint_discarded)) {
    atomicStore(&(a), i32(value.x));
  }
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  bar(ret);
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicStore_NoDiscard) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  let ret = textureSample(t, s, coord);
  atomicStore(&(a), i32(ret.x));
}
)";

    auto* expect = src;

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicBuiltinExpression) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if (in == 0.0) {
    discard;
  }
  let v = i32(textureSample(t, s, coord).x);
  let result = v + atomicAdd(&a, v);
  return result;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let v = i32(textureSample(t, s, coord).x);
  var tint_symbol : i32;
  if (!(tint_discarded)) {
    tint_symbol = atomicAdd(&(a), v);
  }
  let result = (v + tint_symbol);
  if (tint_discarded) {
    discard;
  }
  return result;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicBuiltinExpression_InForLoopContinuing) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if (in == 0.0) {
    discard;
  }
  var result = 0;
  for (var i = 0; i < 10; i = atomicAdd(&a, 1)) {
    result += i;
  }
  result += i32(textureSample(t, s, coord).x);
  return result;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  var result = 0;
  {
    var i = 0;
    loop {
      if (!((i < 10))) {
        break;
      }
      {
        result += i;
      }

      continuing {
        var tint_symbol : i32;
        if (!(tint_discarded)) {
          tint_symbol = atomicAdd(&(a), 1);
        }
        i = tint_symbol;
      }
    }
  }
  result += i32(textureSample(t, s, coord).x);
  if (tint_discarded) {
    discard;
  }
  return result;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicCompareExchangeWeak) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if (in == 0.0) {
    discard;
  }
  var result = 0;
  if (!atomicCompareExchangeWeak(&a, i32(in), 42).exchanged) {
    let xchg = atomicCompareExchangeWeak(&a, i32(in), 42);
    result = xchg.old_value;
  }
  result += i32(textureSample(t, s, coord).x);
  return result;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  var result = 0;
  var tint_symbol : __atomic_compare_exchange_result_i32;
  if (!(tint_discarded)) {
    tint_symbol = atomicCompareExchangeWeak(&(a), i32(in), 42);
  }
  if (!(tint_symbol.exchanged)) {
    var tint_symbol_1 : __atomic_compare_exchange_result_i32;
    if (!(tint_discarded)) {
      tint_symbol_1 = atomicCompareExchangeWeak(&(a), i32(in), 42);
    }
    let xchg = tint_symbol_1;
    result = xchg.old_value;
  }
  result += i32(textureSample(t, s, coord).x);
  if (tint_discarded) {
    discard;
  }
  return result;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that no masking is generated for calls to `atomicLoad()`.
TEST_F(DemoteToHelperTest, AtomicLoad) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if (in == 0.0) {
    discard;
  }
  let v = i32(textureSample(t, s, coord).x);
  let result = v + atomicLoad(&a);
  return result;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let v = i32(textureSample(t, s, coord).x);
  let result = (v + atomicLoad(&(a)));
  if (tint_discarded) {
    discard;
  }
  return result;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, PhonyAssignment) {
    auto* src = R"(
@fragment
fn foo(@location(0) in : f32) {
  if (in == 0.0) {
    discard;
  }
  _ = in;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@fragment
fn foo(@location(0) in : f32) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  _ = in;
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, Assignment_HoistExplicitDerivative) {
    auto* src = R"(
@group(0) @binding(0)
var<storage, read_write> output : array<f32, 4>;

@fragment
fn foo(@location(0) in : f32) {
  if (in == 0.0) {
    discard;
  }
  output[u32(in)] = dpdx(in);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var<storage, read_write> output : array<f32, 4>;

@fragment
fn foo(@location(0) in : f32) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let tint_symbol : f32 = dpdx(in);
  if (!(tint_discarded)) {
    output[u32(in)] = tint_symbol;
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, Assignment_HoistImplicitDerivative) {
    auto* src = R"(
@group(0) @binding(0)
var<storage, read_write> output : array<vec4f, 4>;

@group(0) @binding(1)
var t : texture_2d<f32>;

@group(0) @binding(2)
var s : sampler;

@fragment
fn foo(@interpolate(flat) @location(0) in : u32) {
  if (in == 0) {
    discard;
  }
  output[in] = textureSample(t, s, vec2());
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var<storage, read_write> output : array<vec4f, 4>;

@group(0) @binding(1) var t : texture_2d<f32>;

@group(0) @binding(2) var s : sampler;

@fragment
fn foo(@interpolate(flat) @location(0) in : u32) {
  if ((in == 0)) {
    tint_discarded = true;
  }
  let tint_symbol : vec4<f32> = textureSample(t, s, vec2());
  if (!(tint_discarded)) {
    output[in] = tint_symbol;
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
