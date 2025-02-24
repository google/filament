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

#include "src/tint/lang/wgsl/ast/transform/remove_phonies.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using RemovePhoniesTest = TransformTest;

TEST_F(RemovePhoniesTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<RemovePhonies>(src));
}

TEST_F(RemovePhoniesTest, ShouldRunHasPhony) {
    auto* src = R"(
fn f() {
  _ = 1;
}
)";

    EXPECT_TRUE(ShouldRun<RemovePhonies>(src));
}

TEST_F(RemovePhoniesTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<RemovePhonies>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemovePhoniesTest, NoSideEffects) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn f() {
  var v : i32;
  _ = &v;
  _ = 1;
  _ = 1 + 2;
  _ = t;
  _ = u32(3.0);
  _ = f32(i32(4u));
  _ = vec2<f32>(5.0);
  _ = vec3<i32>(6, 7, 8);
  _ = mat2x2<f32>(9.0, 10.0, 11.0, 12.0);
  _ = atan2(1.0, 2.0);
  _ = clamp(1.0, 2.0, 3.0);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn f() {
  var v : i32;
}
)";

    auto got = Run<RemovePhonies>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemovePhoniesTest, SingleSideEffects) {
    auto* src = R"(
fn neg(a : i32) -> i32 {
  return -(a);
}

fn add(a : i32, b : i32) -> i32 {
  return (a + b);
}

fn f() {
  _ = neg(1);
  _ = add(2, 3);
  _ = add(neg(4), neg(5));
  _ = u32(neg(6));
  _ = f32(add(7, 8));
  _ = vec2<f32>(f32(neg(9)));
  _ = vec3<i32>(1, neg(10), 3);
  _ = mat2x2<f32>(1.0, f32(add(11, 12)), 3.0, 4.0);
}
)";

    auto* expect = R"(
fn neg(a : i32) -> i32 {
  return -(a);
}

fn add(a : i32, b : i32) -> i32 {
  return (a + b);
}

fn f() {
  neg(1);
  add(2, 3);
  add(neg(4), neg(5));
  neg(6);
  add(7, 8);
  neg(9);
  neg(10);
  add(11, 12);
}
)";

    auto got = Run<RemovePhonies>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemovePhoniesTest, SingleSideEffectsMustUse) {
    auto* src = R"(
@must_use
fn neg(a : i32) -> i32 {
  return -(a);
}

@must_use
fn add(a : i32, b : i32) -> i32 {
  return (a + b);
}

fn f() {
  let tint_phony = 42;
  _ = neg(1);
  _ = 100 + add(2, 3) + 200;
  _ = add(neg(4), neg(5)) + 6;
  _ = u32(neg(6));
  _ = f32(add(7, 8));
  _ = vec2<f32>(f32(neg(9)));
  _ = vec3<i32>(1, neg(10), 3);
  _ = mat2x2<f32>(1.0, f32(add(11, 12)), 3.0, 4.0);
}
)";

    auto* expect = R"(
@must_use
fn neg(a : i32) -> i32 {
  return -(a);
}

@must_use
fn add(a : i32, b : i32) -> i32 {
  return (a + b);
}

fn f() {
  let tint_phony = 42;
  let tint_phony_1 = neg(1);
  let tint_phony_2 = add(2, 3);
  let tint_phony_3 = add(neg(4), neg(5));
  let tint_phony_4 = neg(6);
  let tint_phony_5 = add(7, 8);
  let tint_phony_6 = neg(9);
  let tint_phony_7 = neg(10);
  let tint_phony_8 = add(11, 12);
}
)";

    auto got = Run<RemovePhonies>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemovePhoniesTest, SingleSideEffects_OutOfOrder) {
    auto* src = R"(
fn f() {
  _ = neg(1);
  _ = add(2, 3);
  _ = add(neg(4), neg(5));
  _ = u32(neg(6));
  _ = f32(add(7, 8));
  _ = vec2<f32>(f32(neg(9)));
  _ = vec3<i32>(1, neg(10), 3);
  _ = mat2x2<f32>(1.0, f32(add(11, 12)), 3.0, 4.0);
}

fn add(a : i32, b : i32) -> i32 {
  return (a + b);
}

fn neg(a : i32) -> i32 {
  return -(a);
}
)";

    auto* expect = R"(
fn f() {
  neg(1);
  add(2, 3);
  add(neg(4), neg(5));
  neg(6);
  add(7, 8);
  neg(9);
  neg(10);
  add(11, 12);
}

fn add(a : i32, b : i32) -> i32 {
  return (a + b);
}

fn neg(a : i32) -> i32 {
  return -(a);
}
)";

    auto got = Run<RemovePhonies>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemovePhoniesTest, MultipleSideEffects) {
    auto* src = R"(
fn neg(a : i32) -> i32 {
  return -(a);
}

@must_use
fn add(a : i32, b : i32) -> i32 {
  return (a + b);
}

fn xor(a : u32, b : u32) -> u32 {
  return (a ^ b);
}

fn f() {
  _ = (1 + add(2 + add(3, 4), 5)) * add(6, 7) * neg(8);
  _ = add(9, neg(10)) + neg(11);
  _ = xor(12u, 13u) + xor(14u, 15u);
  _ = neg(16) / neg(17) + add(18, 19);
}
)";

    auto* expect = R"(
fn neg(a : i32) -> i32 {
  return -(a);
}

@must_use
fn add(a : i32, b : i32) -> i32 {
  return (a + b);
}

fn xor(a : u32, b : u32) -> u32 {
  return (a ^ b);
}

fn phony_sink(p0 : i32, p1 : i32, p2 : i32) {
}

fn phony_sink_1(p0 : i32, p1 : i32) {
}

fn phony_sink_2(p0 : u32, p1 : u32) {
}

fn f() {
  phony_sink(add((2 + add(3, 4)), 5), add(6, 7), neg(8));
  phony_sink_1(add(9, neg(10)), neg(11));
  phony_sink_2(xor(12u, 13u), xor(14u, 15u));
  phony_sink(neg(16), neg(17), add(18, 19));
}
)";

    auto got = Run<RemovePhonies>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemovePhoniesTest, MultipleSideEffects_OutOfOrder) {
    auto* src = R"(
fn f() {
  _ = (1 + add(2 + add(3, 4), 5)) * add(6, 7) * neg(8);
  _ = add(9, neg(10)) + neg(11);
  _ = xor(12u, 13u) + xor(14u, 15u);
  _ = neg(16) / neg(17) + add(18, 19);
}

fn neg(a : i32) -> i32 {
  return -(a);
}

fn add(a : i32, b : i32) -> i32 {
  return (a + b);
}

fn xor(a : u32, b : u32) -> u32 {
  return (a ^ b);
}
)";

    auto* expect = R"(
fn phony_sink(p0 : i32, p1 : i32, p2 : i32) {
}

fn phony_sink_1(p0 : i32, p1 : i32) {
}

fn phony_sink_2(p0 : u32, p1 : u32) {
}

fn f() {
  phony_sink(add((2 + add(3, 4)), 5), add(6, 7), neg(8));
  phony_sink_1(add(9, neg(10)), neg(11));
  phony_sink_2(xor(12u, 13u), xor(14u, 15u));
  phony_sink(neg(16), neg(17), add(18, 19));
}

fn neg(a : i32) -> i32 {
  return -(a);
}

fn add(a : i32, b : i32) -> i32 {
  return (a + b);
}

fn xor(a : u32, b : u32) -> u32 {
  return (a ^ b);
}
)";

    auto got = Run<RemovePhonies>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemovePhoniesTest, ForLoop) {
    auto* src = R"(
struct S {
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read_write> s : S;

fn x() -> i32 {
  return 0;
}

fn y() -> i32 {
  return 0;
}

fn z() -> i32 {
  return 0;
}

fn f() {
  for (_ = &s.arr; ;_ = &s.arr) {
    break;
  }
  for (_ = x(); ;_ = y() + z()) {
    break;
  }
}
)";

    auto* expect = R"(
struct S {
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn x() -> i32 {
  return 0;
}

fn y() -> i32 {
  return 0;
}

fn z() -> i32 {
  return 0;
}

fn phony_sink(p0 : i32, p1 : i32) {
}

fn f() {
  for(; ; ) {
    break;
  }
  for(x(); ; phony_sink(y(), z())) {
    break;
  }
}
)";

    auto got = Run<RemovePhonies>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemovePhoniesTest, ForLoop_OutOfOrder) {
    auto* src = R"(
fn f() {
  for (_ = &s.arr; ;_ = &s.arr) {
    break;
  }
  for (_ = x(); ;_ = y() + z()) {
    break;
  }
}

fn x() -> i32 {
  return 0;
}

fn y() -> i32 {
  return 0;
}

fn z() -> i32 {
  return 0;
}

struct S {
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read_write> s : S;
)";

    auto* expect = R"(
fn phony_sink(p0 : i32, p1 : i32) {
}

fn f() {
  for(; ; ) {
    break;
  }
  for(x(); ; phony_sink(y(), z())) {
    break;
  }
}

fn x() -> i32 {
  return 0;
}

fn y() -> i32 {
  return 0;
}

fn z() -> i32 {
  return 0;
}

struct S {
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read_write> s : S;
)";

    auto got = Run<RemovePhonies>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemovePhoniesTest, ConstShortCircuit) {
    auto* src = R"(
fn a(v : i32) -> i32 {
  return v;
}

fn b() {
  _ = false && (a(4294967295) < a(a(4294967295)));
}
)";

    auto* expect = R"(
fn a(v : i32) -> i32 {
  return v;
}

fn b() {
}
)";

    auto got = Run<RemovePhonies>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
