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

#include "src/tint/lang/wgsl/ast/transform/expand_compound_assignment.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using ExpandCompoundAssignmentTest = TransformTest;

TEST_F(ExpandCompoundAssignmentTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<ExpandCompoundAssignment>(src));
}

TEST_F(ExpandCompoundAssignmentTest, ShouldRunHasCompoundAssignment) {
    auto* src = R"(
fn foo() {
  var v : i32;
  v += 1;
}
)";

    EXPECT_TRUE(ShouldRun<ExpandCompoundAssignment>(src));
}

TEST_F(ExpandCompoundAssignmentTest, ShouldRunHasIncrementDecrement) {
    auto* src = R"(
fn foo() {
  var v : i32;
  v++;
}
)";

    EXPECT_TRUE(ShouldRun<ExpandCompoundAssignment>(src));
}

TEST_F(ExpandCompoundAssignmentTest, Basic) {
    auto* src = R"(
fn main() {
  var v : i32;
  v += 1;
}
)";

    auto* expect = R"(
fn main() {
  var v : i32;
  v = (v + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsPointer) {
    auto* src = R"(
fn main() {
  var v : i32;
  let p = &v;
  *p += 1;
}
)";

    auto* expect = R"(
fn main() {
  var v : i32;
  let p = &(v);
  let tint_symbol = &(*(p));
  *(tint_symbol) = (*(tint_symbol) + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsStructMember) {
    auto* src = R"(
struct S {
  m : f32,
}

fn main() {
  var s : S;
  s.m += 1.0;
}
)";

    auto* expect = R"(
struct S {
  m : f32,
}

fn main() {
  var s : S;
  s.m = (s.m + 1.0);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsArrayElement) {
    auto* src = R"(
var<private> a : array<i32, 4>;

fn idx() -> i32 {
  a[1] = 42;
  return 1;
}

fn main() {
  a[idx()] += 1;
}
)";

    auto* expect = R"(
var<private> a : array<i32, 4>;

fn idx() -> i32 {
  a[1] = 42;
  return 1;
}

fn main() {
  let tint_symbol = &(a[idx()]);
  *(tint_symbol) = (*(tint_symbol) + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsVectorComponent_ArrayAccessor) {
    auto* src = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  v[idx()] += 1;
}
)";

    auto* expect = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  let tint_symbol = &(v);
  let tint_symbol_1 = idx();
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsVectorComponent_MemberAccessor) {
    auto* src = R"(
fn main() {
  var v : vec4<i32>;
  v.y += 1;
}
)";

    auto* expect = R"(
fn main() {
  var v : vec4<i32>;
  v.y = (v.y + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsArrayOfVectorComponent_MemberAccessor_ViaArrayIndex) {
    auto* src = R"(
fn main() {
  var v : array<vec4<i32>, 3>;
  v[0].y += 1;
}
)";

    auto* expect = R"(
fn main() {
  var v : array<vec4<i32>, 3>;
  let tint_symbol = &(v[0]);
  (*(tint_symbol)).y = ((*(tint_symbol)).y + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsVectorComponent_MemberAccessor_ViaDerefPointerDot) {
    auto* src = R"(
fn main() {
  var v : vec4<i32>;
  let p = &v;
  (*p).y += 1;
}
)";

    // TODO(crbug.com/tint/2115): we currently needlessly hoist pointer-deref to another pointer.
    auto* expect = R"(
fn main() {
  var v : vec4<i32>;
  let p = &(v);
  let tint_symbol = &(*(p));
  (*(tint_symbol)).y = ((*(tint_symbol)).y + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsVectorComponent_MemberAccessor_ViaPointerDot) {
    auto* src = R"(
fn main() {
  var v : vec4<i32>;
  let p = &v;
  p.y += 1;
}
)";

    auto* expect = R"(
fn main() {
  var v : vec4<i32>;
  let p = &(v);
  p.y = (p.y + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsVectorComponent_MemberAccessor_ViaDerefPointerIndex) {
    auto* src = R"(
fn main() {
  var v : vec4<i32>;
  let p = &v;
  (*p)[0] += 1;
}
)";

    // TODO(crbug.com/tint/2115): we currently needlessly hoist pointer-deref to another pointer.
    auto* expect = R"(
fn main() {
  var v : vec4<i32>;
  let p = &(v);
  let tint_symbol = &(*(p));
  let tint_symbol_1 = 0;
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsVectorComponent_MemberAccessor_ViaPointerIndex) {
    auto* src = R"(
fn main() {
  var v : vec4<i32>;
  let p = &v;
  p[0] += 1;
}
)";

    auto* expect = R"(
fn main() {
  var v : vec4<i32>;
  let p = &(v);
  let tint_symbol = p;
  let tint_symbol_1 = 0;
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsMatrixColumn) {
    auto* src = R"(
var<private> m : mat4x4<f32>;

fn idx() -> i32 {
  m[0].y = 42.0;
  return 1;
}

fn main() {
  m[idx()] += 1.0;
}
)";

    auto* expect = R"(
var<private> m : mat4x4<f32>;

fn idx() -> i32 {
  m[0].y = 42.0;
  return 1;
}

fn main() {
  let tint_symbol = &(m[idx()]);
  *(tint_symbol) = (*(tint_symbol) + 1.0);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsMatrixElement) {
    auto* src = R"(
var<private> m : mat4x4<f32>;

fn idx1() -> i32 {
  m[0].y = 42.0;
  return 1;
}

fn idx2() -> i32 {
  m[1].z = 42.0;
  return 1;
}

fn main() {
  m[idx1()][idx2()] += 1.0;
}
)";

    auto* expect = R"(
var<private> m : mat4x4<f32>;

fn idx1() -> i32 {
  m[0].y = 42.0;
  return 1;
}

fn idx2() -> i32 {
  m[1].z = 42.0;
  return 1;
}

fn main() {
  let tint_symbol = &(m[idx1()]);
  let tint_symbol_1 = idx2();
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1.0);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, LhsMultipleSideEffects) {
    auto* src = R"(
struct S {
  a : array<vec4<f32>, 3>,
}

@group(0) @binding(0) var<storage, read_write> buffer : array<S>;

var<private> p : i32;

fn idx1() -> i32 {
  p += 1;
  return 3;
}

fn idx2() -> i32 {
  p *= 3;
  return 2;
}

fn idx3() -> i32 {
  p -= 2;
  return 1;
}

fn main() {
  buffer[idx1()].a[idx2()][idx3()] += 1.0;
}
)";

    auto* expect = R"(
struct S {
  a : array<vec4<f32>, 3>,
}

@group(0) @binding(0) var<storage, read_write> buffer : array<S>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn idx3() -> i32 {
  p = (p - 2);
  return 1;
}

fn main() {
  let tint_symbol = &(buffer[idx1()].a[idx2()]);
  let tint_symbol_1 = idx3();
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1.0);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, ForLoopInit) {
    auto* src = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  for (a[idx1()][idx2()] += 1; ; ) {
    break;
  }
}
)";

    auto* expect = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  {
    let tint_symbol = &(a[idx1()]);
    let tint_symbol_1 = idx2();
    (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
    loop {
      {
        break;
      }
    }
  }
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, ForLoopCont) {
    auto* src = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  for (; ; a[idx1()][idx2()] += 1) {
    break;
  }
}
)";

    auto* expect = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  loop {
    {
      break;
    }

    continuing {
      let tint_symbol = &(a[idx1()]);
      let tint_symbol_1 = idx2();
      (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
    }
  }
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_I32) {
    auto* src = R"(
fn main() {
  var v : i32;
  v++;
}
)";

    auto* expect = R"(
fn main() {
  var v : i32;
  v = (v + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_U32) {
    auto* src = R"(
fn main() {
  var v : u32;
  v++;
}
)";

    auto* expect = R"(
fn main() {
  var v : u32;
  v = (v + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Decrement_I32) {
    auto* src = R"(
fn main() {
  var v : i32;
  v--;
}
)";

    auto* expect = R"(
fn main() {
  var v : i32;
  v = (v - 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Decrement_U32) {
    auto* src = R"(
fn main() {
  var v : u32;
  v--;
}
)";

    auto* expect = R"(
fn main() {
  var v : u32;
  v = (v - 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsPointer) {
    auto* src = R"(
fn main() {
  var v : i32;
  let p = &v;
  *p++;
}
)";

    auto* expect = R"(
fn main() {
  var v : i32;
  let p = &(v);
  let tint_symbol = &(*(p));
  *(tint_symbol) = (*(tint_symbol) + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsStructMember) {
    auto* src = R"(
struct S {
  m : i32,
}

fn main() {
  var s : S;
  s.m++;
}
)";

    auto* expect = R"(
struct S {
  m : i32,
}

fn main() {
  var s : S;
  s.m = (s.m + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsArrayElement) {
    auto* src = R"(
var<private> a : array<i32, 4>;

fn idx() -> i32 {
  a[1] = 42;
  return 1;
}

fn main() {
  a[idx()]++;
}
)";

    auto* expect = R"(
var<private> a : array<i32, 4>;

fn idx() -> i32 {
  a[1] = 42;
  return 1;
}

fn main() {
  let tint_symbol = &(a[idx()]);
  *(tint_symbol) = (*(tint_symbol) + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsVectorComponent_ArrayAccessor) {
    auto* src = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  v[idx()]++;
}
)";

    auto* expect = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  let tint_symbol = &(v);
  let tint_symbol_1 = idx();
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest,
       Increment_LhsVectorComponent_ArrayAccessor_ViaDerefPointerIndex) {
    auto* src = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  let p = &v;
  (*p)[idx()]++;
}
)";

    auto* expect = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  let p = &(v);
  let tint_symbol = &(*(p));
  let tint_symbol_1 = idx();
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsVectorComponent_ArrayAccessor_ViaPointerIndex) {
    auto* src = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  let p = &v;
  p[idx()]++;
}
)";

    auto* expect = R"(
var<private> v : vec4<i32>;

fn idx() -> i32 {
  v.y = 42;
  return 1;
}

fn main() {
  let p = &(v);
  let tint_symbol = p;
  let tint_symbol_1 = idx();
  (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsVectorComponent_MemberAccessor) {
    auto* src = R"(
fn main() {
  var v : vec4<i32>;
  v.y++;
}
)";

    auto* expect = R"(
fn main() {
  var v : vec4<i32>;
  v.y = (v.y + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest,
       Increment_LhsVectorComponent_MemberAccessor_ViaDerefPointerDot) {
    auto* src = R"(
fn main() {
  var v : vec4<i32>;
  let p = &v;
  (*p).y++;
}
)";

    // TODO(crbug.com/tint/2115): we currently needlessly hoist pointer-deref to another pointer.
    auto* expect = R"(
fn main() {
  var v : vec4<i32>;
  let p = &(v);
  let tint_symbol = &(*(p));
  (*(tint_symbol)).y = ((*(tint_symbol)).y + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_LhsVectorComponent_MemberAccessor_ViaPointerDot) {
    auto* src = R"(
fn main() {
  var v : vec4<i32>;
  let p = &v;
  p.y++;
}
)";

    auto* expect = R"(
fn main() {
  var v : vec4<i32>;
  let p = &(v);
  p.y = (p.y + 1);
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ExpandCompoundAssignmentTest, Increment_ForLoopCont) {
    auto* src = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  for (; ; a[idx1()][idx2()]++) {
    break;
  }
}
)";

    auto* expect = R"(
var<private> a : array<vec4<i32>, 4>;

var<private> p : i32;

fn idx1() -> i32 {
  p = (p + 1);
  return 3;
}

fn idx2() -> i32 {
  p = (p * 3);
  return 2;
}

fn main() {
  loop {
    {
      break;
    }

    continuing {
      let tint_symbol = &(a[idx1()]);
      let tint_symbol_1 = idx2();
      (*(tint_symbol))[tint_symbol_1] = ((*(tint_symbol))[tint_symbol_1] + 1);
    }
  }
}
)";

    auto got = Run<ExpandCompoundAssignment>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
