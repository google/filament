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

#include "src/tint/lang/wgsl/ast/transform/fold_constants.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using FoldConstantsTest = TransformTest;

TEST_F(FoldConstantsTest, NoFolding) {
    auto* src = R"(
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Expression) {
    auto* src = R"(
fn foo() -> i32 {
  return (1 + (2 * 4)) - (3 * 5);
}
)";

    auto* expect = R"(
fn foo() -> i32 {
  return -6i;
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Abstract) {
    auto* src = R"(
fn foo() {
  const v = vec4(1, 2, 3, vec3(0)[1 + 1 - 0]);
}
)";

    auto* expect = R"(
fn foo() {
  const v = vec4(1, 2, 3, 0);
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, IndexExpression) {
    auto* src = R"(
struct S {
  a : array<i32>,
}

@group(0) @binding(0) var<storage> s : S;

fn foo() -> i32 {
  return s.a[(3 * 5) - (1 + (2 * 4))];
}
)";

    auto* expect = R"(
struct S {
  a : array<i32>,
}

@group(0i) @binding(0i) var<storage> s : S;

fn foo() -> i32 {
  return s.a[6i];
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, IndexAccessorToConst) {
    auto* src = R"(
const a : array<i32, 4> = array(0, 1, 2, 3);

fn foo() -> i32 {
  return a[3];
}
)";

    auto* expect = R"(
const a : array<i32, 4i> = array<i32, 4u>(0i, 1i, 2i, 3i);

fn foo() -> i32 {
  return 3i;
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, SplatParam) {
    auto* src = R"(
fn foo() -> i32 {
  let v = vec3(2 + 5 - 4);
  return v.x;
}
)";

    auto* expect = R"(
fn foo() -> i32 {
  let v = vec3<i32>(3i);
  return v.x;
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, CallParam) {
    auto* src = R"(
fn foo() -> i32 {
  let v = vec3(2 + 5 - 4, 5 * 9, -2 + -3);
  return v.x;
}
)";

    auto* expect = R"(
fn foo() -> i32 {
  let v = vec3<i32>(3i, 45i, -5i);
  return v.x;
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, LHSIndexExpressionOnAssign) {
    auto* src = R"(
fn foo() -> i32 {
  var v = vec4(0);
  v[1 * 2 + (3 - 2)] = 1;

  return v[1];
}
)";

    auto* expect = R"(
fn foo() -> i32 {
  var v = vec4<i32>();
  v[3i] = 1i;
  return v[1i];
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, LHSIndexExpressionOnCompoundAssign) {
    auto* src = R"(
fn foo() -> i32 {
  var v = vec4(0);
  v[1 * 2 + (3 - 2)] += 1;

  return v[1];
}
)";

    auto* expect = R"(
fn foo() -> i32 {
  var v = vec4<i32>();
  v[3i] += 1i;
  return v[1i];
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, LHSIndexExpressionOnIncrement) {
    auto* src = R"(
fn foo() -> i32 {
  var v = vec4(0);
  v[1 * 2 + (3 - 2)]++;

  return v[1];
}
)";

    auto* expect = R"(
fn foo() -> i32 {
  var v = vec4<i32>();
  v[3i]++;
  return v[1i];
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Attribute) {
    auto* src = R"(
struct A {
  @location(1 + 2)
  v : i32,
}

@group(0) @binding(0) var<storage> a : A;

fn foo() -> i32 {
  return a.v;
}
)";

    auto* expect = R"(
struct A {
  @location(3i)
  v : i32,
}

@group(0i) @binding(0i) var<storage> a : A;

fn foo() -> i32 {
  return a.v;
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, ShortCircut) {
    auto* src = R"(
fn foo() -> bool {
  return false && (1 + 2) == 3;
}
)";

    auto* expect = R"(
fn foo() -> bool {
  return false;
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, BreakIf) {
    auto* src = R"(
fn foo() {
  loop {

    continuing {
      break if 4 > 3;
    }
  }
}
)";

    auto* expect = R"(
fn foo() {
  loop {

    continuing {
      break if true;
    }
  }
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, Switch) {
    auto* src = R"(
fn foo() {
  switch(1 + 2 + (3 * 5)) {
    case 1 + 2, 3 + 4: {
      break;
    }
    default: {
      break;
    }
  }
}
)";

    auto* expect = R"(
fn foo() {
  switch(18i) {
    case 3i, 7i: {
      break;
    }
    default: {
      break;
    }
  }
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, If) {
    auto* src = R"(
fn foo() {
  if 4 - 2 > 3 {
    return;
  }
}
)";

    auto* expect = R"(
fn foo() {
  if (false) {
    return;
  }
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, For) {
    auto* src = R"(
fn foo() {
  for(var i = 2 + (3 * 4); i < 9 + (5 * 10); i = i + (2 * 3)) {
  }
}
)";

    auto* expect = R"(
fn foo() {
  for(var i = 14i; (i < 59i); i = (i + 6i)) {
  }
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, While) {
    auto* src = R"(
fn foo() {
  while(2 > 4) {
  }
}
)";

    auto* expect = R"(
fn foo() {
  while(false) {
  }
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, ConstAssert) {
    auto* src = R"(
const_assert 2 < (3 + 5);
)";

    auto* expect = R"(
const_assert true;
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldConstantsTest, InternalStruct) {
    auto* src = R"(
fn foo() {
  let a = modf(1.5);
}
)";

    auto* expect = R"(
fn foo() {
  let a = __modf_result_f32(0.5f, 1.0f);
}
)";

    auto got = Run<FoldConstants>(src);
    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
