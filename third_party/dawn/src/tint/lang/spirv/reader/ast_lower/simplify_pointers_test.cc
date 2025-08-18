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

#include "src/tint/lang/spirv/reader/ast_lower/simplify_pointers.h"

#include "src/tint/lang/spirv/reader/ast_lower/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_lower/unshadow.h"

namespace tint::ast::transform {
namespace {

using SimplifyPointersTest = TransformTest;

TEST_F(SimplifyPointersTest, EmptyModule) {
    auto* src = "";

    EXPECT_FALSE(ShouldRun<SimplifyPointers>(src));
}

TEST_F(SimplifyPointersTest, FoldPointer) {
    auto* src = R"(
fn f() {
  var v : i32;
  let p : ptr<function, i32> = &v;
  let x : i32 = *p;
}
)";

    auto* expect = R"(
fn f() {
  var v : i32;
  let x : i32 = v;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, UnusedPointerParam) {
    auto* src = R"(
fn f(p : ptr<function, i32>) {
}
)";

    auto* expect = src;

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, ReferencedPointerParam) {
    auto* src = R"(
fn f(p : ptr<function, i32>) {
  let x = p;
}
)";

    auto* expect = R"(
fn f(p : ptr<function, i32>) {
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, AddressOfDeref) {
    auto* src = R"(
fn f() {
  var v : i32;
  let p : ptr<function, i32> = &(v);
  let x : ptr<function, i32> = &(*(p));
  let y : ptr<function, i32> = &(*(&(*(p))));
  let z : ptr<function, i32> = &(*(&(*(&(*(&(*(p))))))));
  var a = *x;
  var b = *y;
  var c = *z;
}
)";

    auto* expect = R"(
fn f() {
  var v : i32;
  var a = v;
  var b = v;
  var c = v;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, DerefAddressOf) {
    auto* src = R"(
fn f() {
  var v : i32;
  let x : i32 = *(&(v));
  let y : i32 = *(&(*(&(v))));
  let z : i32 = *(&(*(&(*(&(*(&(v))))))));
}
)";

    auto* expect = R"(
fn f() {
  var v : i32;
  let x : i32 = v;
  let y : i32 = v;
  let z : i32 = v;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, PointerDerefIndex) {
    auto* src = R"(
fn f() {
  var a : array<f32, 2>;
  let p = &a;
  let v = (*p)[1];
}
)";

    auto* expect = R"(
fn f() {
  var a : array<f32, 2>;
  let v = a[1];
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, PointerIndex) {
    auto* src = R"(
fn f() {
  var a : array<f32, 2>;
  let p = &a;
  let v = p[1];
}
)";

    auto* expect = R"(
fn f() {
  var a : array<f32, 2>;
  let v = a[1];
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, SimpleChain) {
    auto* src = R"(
fn f() {
  var a : array<f32, 2>;
  let ap : ptr<function, array<f32, 2>> = &a;
  let vp : ptr<function, f32> = &(*ap)[1];
  let v : f32 = *vp;
}
)";

    auto* expect = R"(
fn f() {
  var a : array<f32, 2>;
  let v : f32 = a[1];
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, SimpleChain_ViaPointerIndex) {
    auto* src = R"(
fn f() {
  var a : array<f32, 2>;
  let ap : ptr<function, array<f32, 2>> = &a;
  let vp : ptr<function, f32> = &ap[1];
  let v : f32 = *vp;
}
)";

    auto* expect = R"(
fn f() {
  var a : array<f32, 2>;
  let v : f32 = a[1];
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, ComplexChain) {
    auto* src = R"(
fn f() {
  var a : array<mat4x4<f32>, 4>;
  let ap : ptr<function, array<mat4x4<f32>, 4>> = &a;
  let mp : ptr<function, mat4x4<f32>> = &(*ap)[3];
  let vp : ptr<function, vec4<f32>> = &(*mp)[2];
  let v : vec4<f32> = *vp;
}
)";

    auto* expect = R"(
fn f() {
  var a : array<mat4x4<f32>, 4>;
  let v : vec4<f32> = a[3][2];
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, ComplexChain_ViaPointerIndex) {
    auto* src = R"(
fn f() {
  var a : array<mat4x4<f32>, 4>;
  let ap : ptr<function, array<mat4x4<f32>, 4>> = &a;
  let mp : ptr<function, mat4x4<f32>> = &ap[3];
  let vp : ptr<function, vec4<f32>> = &mp[2];
  let v : vec4<f32> = *vp;
}
)";

    auto* expect = R"(
fn f() {
  var a : array<mat4x4<f32>, 4>;
  let v : vec4<f32> = a[3][2];
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, SavedVars) {
    auto* src = R"(
struct S {
  i : i32,
};

fn arr() {
  var a : array<S, 2>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p : ptr<function, i32> = &a[i + j].i;
  i = 2;
  *p = 4;
}

fn matrix() {
  var m : mat3x3<f32>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p : ptr<function, vec3<f32>> = &m[i + j];
  i = 2;
  *p = vec3<f32>(4.0, 5.0, 6.0);
}
)";

    auto* expect = R"(
struct S {
  i : i32,
}

fn arr() {
  var a : array<S, 2>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p_save = (i + j);
  i = 2;
  a[p_save].i = 4;
}

fn matrix() {
  var m : mat3x3<f32>;
  var i : i32 = 0;
  var j : i32 = 0;
  let p_save_1 = (i + j);
  i = 2;
  m[p_save_1] = vec3<f32>(4.0, 5.0, 6.0);
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, DontSaveLiterals) {
    auto* src = R"(
fn f() {
  var arr : array<i32, 2>;
  let p1 : ptr<function, i32> = &arr[1];
  *p1 = 4;
}
)";

    auto* expect = R"(
fn f() {
  var arr : array<i32, 2>;
  arr[1] = 4;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, SavedVarsChain) {
    auto* src = R"(
fn f() {
  var arr : array<array<i32, 2>, 2>;
  let i : i32 = 0;
  let j : i32 = 1;
  let p : ptr<function, array<i32, 2>> = &arr[i];
  let q : ptr<function, i32> = &(*p)[j];
  *q = 12;
}
)";

    auto* expect = R"(
fn f() {
  var arr : array<array<i32, 2>, 2>;
  let i : i32 = 0;
  let j : i32 = 1;
  let p_save = i;
  let q_save = j;
  arr[p_save][q_save] = 12;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, ForLoopInit) {
    auto* src = R"(
fn foo() -> i32 {
  return 1;
}

@fragment
fn main() {
  var arr = array<f32, 4>();
  for (let a = &arr[foo()]; ;) {
    let x = *a;
    break;
  }
}
)";

    auto* expect = R"(
fn foo() -> i32 {
  return 1;
}

@fragment
fn main() {
  var arr = array<f32, 4>();
  let a_save = foo();
  for(; ; ) {
    let x = arr[a_save];
    break;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, MultiSavedVarsInSinglePtrLetExpr) {
    auto* src = R"(
fn x() -> i32 {
  return 1;
}

fn y() -> i32 {
  return 1;
}

fn z() -> i32 {
  return 1;
}

struct Inner {
  a : array<i32, 2>,
};

struct Outer {
  a : array<Inner, 2>,
};

fn f() {
  var arr : array<Outer, 2>;
  let p : ptr<function, i32> = &arr[x()].a[y()].a[z()];
  *p = 1;
  *p = 2;
}
)";

    auto* expect = R"(
fn x() -> i32 {
  return 1;
}

fn y() -> i32 {
  return 1;
}

fn z() -> i32 {
  return 1;
}

struct Inner {
  a : array<i32, 2>,
}

struct Outer {
  a : array<Inner, 2>,
}

fn f() {
  var arr : array<Outer, 2>;
  let p_save = x();
  let p_save_1 = y();
  let p_save_2 = z();
  arr[p_save].a[p_save_1].a[p_save_2] = 1;
  arr[p_save].a[p_save_1].a[p_save_2] = 2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, ShadowPointer) {
    auto* src = R"(
var<private> a : array<i32, 2>;

@compute @workgroup_size(1)
fn main() {
  let x = &a;
  var a : i32 = (*x)[0];
  {
    var a : i32 = (*x)[1];
  }
}
)";

    auto* expect = R"(
var<private> a : array<i32, 2>;

@compute @workgroup_size(1)
fn main() {
  var a_1 : i32 = a[0];
  {
    var a_2 : i32 = a[1];
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyPointersTest, SwizzleFromPointer) {
    auto* src = R"(
fn f() {
  var a : vec4f;
  let p : ptr<function, vec4f> = &a;
  let v : vec2f = p.yw;
}
)";

    auto* expect = R"(
fn f() {
  var a : vec4f;
  let v : vec2f = a.yw;
}
)";

    auto got = Run<Unshadow, SimplifyPointers>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
