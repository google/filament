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

#include "src/tint/lang/wgsl/ast/transform/direct_variable_access.h"

#include <memory>
#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/utils/text/string.h"

namespace tint::ast::transform {
namespace {

/// @returns a DataMap with DirectVariableAccess::Config::transform_private enabled.
static DataMap EnablePrivate() {
    DirectVariableAccess::Options opts;
    opts.transform_private = true;

    DataMap inputs;
    inputs.Add<DirectVariableAccess::Config>(opts);
    return inputs;
}

/// @returns a DataMap with DirectVariableAccess::Config::transform_function enabled.
static DataMap EnableFunction() {
    DirectVariableAccess::Options opts;
    opts.transform_function = true;

    DataMap inputs;
    inputs.Add<DirectVariableAccess::Config>(opts);
    return inputs;
}

////////////////////////////////////////////////////////////////////////////////
// ShouldRun tests
////////////////////////////////////////////////////////////////////////////////
namespace should_run {

using DirectVariableAccessShouldRunTest = TransformTest;

TEST_F(DirectVariableAccessShouldRunTest, EmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<DirectVariableAccess>(src));
}

}  // namespace should_run

////////////////////////////////////////////////////////////////////////////////
// remove uncalled
////////////////////////////////////////////////////////////////////////////////
namespace remove_uncalled {

using DirectVariableAccessRemoveUncalledTest = TransformTest;

TEST_F(DirectVariableAccessRemoveUncalledTest, PtrUniform) {
    auto* src = R"(
var<private> keep_me = 42;

fn u(pre : i32, p : ptr<uniform, i32>, post : i32) -> i32 {
  return *(p);
}

)";

    auto* expect = R"(
var<private> keep_me = 42;
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessRemoveUncalledTest, PtrStorage) {
    auto* src = R"(
var<private> keep_me = 42;

fn s(pre : i32, p : ptr<storage, i32>, post : i32) -> i32 {
  return *(p);
}
)";

    auto* expect = R"(
var<private> keep_me = 42;
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessRemoveUncalledTest, PtrWorkgroup) {
    auto* src = R"(
var<private> keep_me = 42;

fn w(pre : i32, p : ptr<workgroup, i32>, post : i32) -> i32 {
  return *(p);
}

)";

    auto* expect = R"(
var<private> keep_me = 42;
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessRemoveUncalledTest, PtrPrivate_Disabled) {
    auto* src = R"(
var<private> keep_me = 42;

fn f(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}
)";

    auto* expect = src;

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessRemoveUncalledTest, PtrPrivate_Enabled) {
    auto* src = R"(
var<private> keep_me = 42;
)";

    auto* expect = src;

    auto got = Run<DirectVariableAccess>(src, EnablePrivate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessRemoveUncalledTest, PtrFunction_Disabled) {
    auto* src = R"(
var<private> keep_me = 42;

fn f(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *(p);
}
)";

    auto* expect = src;

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessRemoveUncalledTest, PtrFunction_Enabled) {
    auto* src = R"(
var<private> keep_me = 42;
)";

    auto* expect = src;

    auto got = Run<DirectVariableAccess>(src, EnableFunction());

    EXPECT_EQ(expect, str(got));
}

}  // namespace remove_uncalled

////////////////////////////////////////////////////////////////////////////////
// pointer chains
////////////////////////////////////////////////////////////////////////////////
namespace pointer_chains_tests {

using DirectVariableAccessPtrChainsTest = TransformTest;

TEST_F(DirectVariableAccessPtrChainsTest, ConstantIndices) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 8>, 8>, 8>;

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p0 = &U;
  let p1 = &(*p0)[1];
  let p2 = &(*p1)[1+1];
  let p3 = &(*p2)[2*2 - 1];
  a(10, p3, 20);
}

fn c(p : ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>>) {
  let p0 = p;
  let p1 = &(*p0)[1];
  let p2 = &(*p1)[1+1];
  let p3 = &(*p2)[2*2 - 1];
  a(10, p3, 20);
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 8>, 8>, 8>;

alias U_X_X_X = array<u32, 3u>;

fn a_U_X_X_X(pre : i32, p : U_X_X_X, post : i32) -> vec4<i32> {
  return U[p[0]][p[1]][p[2]];
}

fn b() {
  let p0 = &(U);
  let p1 = &((*(p0))[1]);
  let p2 = &((*(p1))[(1 + 1)]);
  let p3 = &((*(p2))[((2 * 2) - 1)]);
  a_U_X_X_X(10, U_X_X_X(1, 2, 3), 20);
}

fn c_U() {
  let p0 = &(U);
  let p1 = &(U[1]);
  let p2 = &(U[1][2]);
  let p3 = &(U[1][2][3]);
  a_U_X_X_X(10, U_X_X_X(1, 2, 3), 20);
}

fn d() {
  c_U();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPtrChainsTest, ConstantIndices_ViaPointerIndex) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 8>, 8>, 8>;

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p0 = &U;
  let p1 = &p0[1];
  let p2 = &p1[1+1];
  let p3 = &p2[2*2 - 1];
  a(10, p3, 20);
}

fn c(p : ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>>) {
  let p0 = p;
  let p1 = &p0[1];
  let p2 = &p1[1+1];
  let p3 = &p2[2*2 - 1];
  a(10, p3, 20);
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 8>, 8>, 8>;

alias U_X_X_X = array<u32, 3u>;

fn a_U_X_X_X(pre : i32, p : U_X_X_X, post : i32) -> vec4<i32> {
  return U[p[0]][p[1]][p[2]];
}

fn b() {
  let p0 = &(U);
  let p1 = &(p0[1]);
  let p2 = &(p1[(1 + 1)]);
  let p3 = &(p2[((2 * 2) - 1)]);
  a_U_X_X_X(10, U_X_X_X(1, 2, 3), 20);
}

fn c_U() {
  let p0 = &(U);
  let p1 = &(U[1]);
  let p2 = &(U[1][2]);
  let p3 = &(U[1][2][3]);
  a_U_X_X_X(10, U_X_X_X(1, 2, 3), 20);
}

fn d() {
  c_U();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPtrChainsTest, HoistIndices) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 8>, 8>, 8>;

var<private> i : i32;
fn first() -> i32 {
  i++;
  return i;
}
fn second() -> i32 {
  i++;
  return i;
}
fn third() -> i32 {
  i++;
  return i;
}

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p0 = &U;
  let p1 = &(*p0)[first()];
  let p2 = &(*p1)[second()][third()];
  a(10, p2, 20);
}

fn c(p : ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>>) {
  let p0 = &U;
  let p1 = &(*p0)[first()];
  let p2 = &(*p1)[second()][third()];
  a(10, p2, 20);
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 8>, 8>, 8>;

var<private> i : i32;

fn first() -> i32 {
  i++;
  return i;
}

fn second() -> i32 {
  i++;
  return i;
}

fn third() -> i32 {
  i++;
  return i;
}

alias U_X_X_X = array<u32, 3u>;

fn a_U_X_X_X(pre : i32, p : U_X_X_X, post : i32) -> vec4<i32> {
  return U[p[0]][p[1]][p[2]];
}

fn b() {
  let p0 = &(U);
  let ptr_index_save = first();
  let p1 = &((*(p0))[ptr_index_save]);
  let ptr_index_save_1 = second();
  let ptr_index_save_2 = third();
  let p2 = &((*(p1))[ptr_index_save_1][ptr_index_save_2]);
  a_U_X_X_X(10, U_X_X_X(u32(ptr_index_save), u32(ptr_index_save_1), u32(ptr_index_save_2)), 20);
}

fn c_U() {
  let p0 = &(U);
  let ptr_index_save_3 = first();
  let p1 = &((*(p0))[ptr_index_save_3]);
  let ptr_index_save_4 = second();
  let ptr_index_save_5 = third();
  let p2 = &((*(p1))[ptr_index_save_4][ptr_index_save_5]);
  a_U_X_X_X(10, U_X_X_X(u32(ptr_index_save_3), u32(ptr_index_save_4), u32(ptr_index_save_5)), 20);
}

fn d() {
  c_U();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPtrChainsTest, HoistInForLoopInit) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;
fn first() -> i32 {
  i++;
  return i;
}
fn second() -> i32 {
  i++;
  return i;
}

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  for (let p1 = &U[first()]; true; ) {
    a(10, &(*p1)[second()], 20);
  }
}

fn c(p : ptr<uniform, array<array<vec4<i32>, 8>, 8>>) {
  for (let p1 = &(*p)[first()]; true; ) {
    a(10, &(*p1)[second()], 20);
  }
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;

fn first() -> i32 {
  i++;
  return i;
}

fn second() -> i32 {
  i++;
  return i;
}

alias U_X_X = array<u32, 2u>;

fn a_U_X_X(pre : i32, p : U_X_X, post : i32) -> vec4<i32> {
  return U[p[0]][p[1]];
}

fn b() {
  {
    let ptr_index_save = first();
    let p1 = &(U[ptr_index_save]);
    loop {
      if (!(true)) {
        break;
      }
      {
        a_U_X_X(10, U_X_X(u32(ptr_index_save), u32(second())), 20);
      }
    }
  }
}

fn c_U() {
  {
    let ptr_index_save_1 = first();
    let p1 = &(U[ptr_index_save_1]);
    loop {
      if (!(true)) {
        break;
      }
      {
        a_U_X_X(10, U_X_X(u32(ptr_index_save_1), u32(second())), 20);
      }
    }
  }
}

fn d() {
  c_U();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPtrChainsTest, HoistInForLoopCond) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;
fn first() -> i32 {
  i++;
  return i;
}
fn second() -> i32 {
  i++;
  return i;
}

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p = &U[first()][second()];
  for (; a(10, p, 20).x < 4; ) {
    let body = 1;
  }
}

fn c(p : ptr<uniform, array<array<vec4<i32>, 8>, 8>>) {
  let p2 = &(*p)[first()][second()];
  for (; a(10, p2, 20).x < 4; ) {
    let body = 1;
  }
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;

fn first() -> i32 {
  i++;
  return i;
}

fn second() -> i32 {
  i++;
  return i;
}

alias U_X_X = array<u32, 2u>;

fn a_U_X_X(pre : i32, p : U_X_X, post : i32) -> vec4<i32> {
  return U[p[0]][p[1]];
}

fn b() {
  let ptr_index_save = first();
  let ptr_index_save_1 = second();
  let p = &(U[ptr_index_save][ptr_index_save_1]);
  for(; (a_U_X_X(10, U_X_X(u32(ptr_index_save), u32(ptr_index_save_1)), 20).x < 4); ) {
    let body = 1;
  }
}

fn c_U() {
  let ptr_index_save_2 = first();
  let ptr_index_save_3 = second();
  let p2 = &(U[ptr_index_save_2][ptr_index_save_3]);
  for(; (a_U_X_X(10, U_X_X(u32(ptr_index_save_2), u32(ptr_index_save_3)), 20).x < 4); ) {
    let body = 1;
  }
}

fn d() {
  c_U();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPtrChainsTest, HoistInForLoopCont) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;
fn first() -> i32 {
  i++;
  return i;
}
fn second() -> i32 {
  i++;
  return i;
}

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p = &U[first()][second()];
  for (var i = 0; i < 3; a(10, p, 20)) {
    i++;
  }
}

fn c(p : ptr<uniform, array<array<vec4<i32>, 8>, 8>>) {
  let p2 = &(*p)[first()][second()];
  for (var i = 0; i < 3; a(10, p2, 20)) {
    i++;
  }
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;

fn first() -> i32 {
  i++;
  return i;
}

fn second() -> i32 {
  i++;
  return i;
}

alias U_X_X = array<u32, 2u>;

fn a_U_X_X(pre : i32, p : U_X_X, post : i32) -> vec4<i32> {
  return U[p[0]][p[1]];
}

fn b() {
  let ptr_index_save = first();
  let ptr_index_save_1 = second();
  let p = &(U[ptr_index_save][ptr_index_save_1]);
  for(var i = 0; (i < 3); a_U_X_X(10, U_X_X(u32(ptr_index_save), u32(ptr_index_save_1)), 20)) {
    i++;
  }
}

fn c_U() {
  let ptr_index_save_2 = first();
  let ptr_index_save_3 = second();
  let p2 = &(U[ptr_index_save_2][ptr_index_save_3]);
  for(var i = 0; (i < 3); a_U_X_X(10, U_X_X(u32(ptr_index_save_2), u32(ptr_index_save_3)), 20)) {
    i++;
  }
}

fn d() {
  c_U();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPtrChainsTest, HoistInWhileCond) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;
fn first() -> i32 {
  i++;
  return i;
}
fn second() -> i32 {
  i++;
  return i;
}

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p = &U[first()][second()];
  while (a(10, p, 20).x < 4) {
    let body = 1;
  }
}

fn c(p : ptr<uniform, array<array<vec4<i32>, 8>, 8>>) {
  let p2 = &(*p)[first()][second()];
  while (a(10, p2, 20).x < 4) {
    let body = 1;
  }
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;

fn first() -> i32 {
  i++;
  return i;
}

fn second() -> i32 {
  i++;
  return i;
}

alias U_X_X = array<u32, 2u>;

fn a_U_X_X(pre : i32, p : U_X_X, post : i32) -> vec4<i32> {
  return U[p[0]][p[1]];
}

fn b() {
  let ptr_index_save = first();
  let ptr_index_save_1 = second();
  let p = &(U[ptr_index_save][ptr_index_save_1]);
  while((a_U_X_X(10, U_X_X(u32(ptr_index_save), u32(ptr_index_save_1)), 20).x < 4)) {
    let body = 1;
  }
}

fn c_U() {
  let ptr_index_save_2 = first();
  let ptr_index_save_3 = second();
  let p2 = &(U[ptr_index_save_2][ptr_index_save_3]);
  while((a_U_X_X(10, U_X_X(u32(ptr_index_save_2), u32(ptr_index_save_3)), 20).x < 4)) {
    let body = 1;
  }
}

fn d() {
  c_U();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace pointer_chains_tests

////////////////////////////////////////////////////////////////////////////////
// 'uniform' address space
////////////////////////////////////////////////////////////////////////////////
namespace uniform_as_tests {

using DirectVariableAccessUniformASTest = TransformTest;

TEST_F(DirectVariableAccessUniformASTest, Param_ptr_i32_read) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : i32;

fn a(pre : i32, p : ptr<uniform, i32>, post : i32) -> i32 {
  return *p;
}

fn b() {
  a(10, &U, 20);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<uniform> U : i32;

fn a_U(pre : i32, post : i32) -> i32 {
  return U;
}

fn b() {
  a_U(10, 20);
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessUniformASTest, Param_ptr_vec4i32_Via_array_DynamicRead) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<vec4<i32>, 8>;

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let I = 3;
  a(10, &U[I], 20);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<uniform> U : array<vec4<i32>, 8>;

alias U_X = array<u32, 1u>;

fn a_U_X(pre : i32, p : U_X, post : i32) -> vec4<i32> {
  return U[p[0]];
}

fn b() {
  let I = 3;
  a_U_X(10, U_X(u32(I)), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessUniformASTest, Param_ptr_vec4i32_Via_array_DynamicRead_ViaPointerDot) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<vec4<i32>, 8>;

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  var I = vec2<i32>(3, 3);
  let p = &I;
  a(10, &U[p.x], 20);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<uniform> U : array<vec4<i32>, 8>;

alias U_X = array<u32, 1u>;

fn a_U_X(pre : i32, p : U_X, post : i32) -> vec4<i32> {
  return U[p[0]];
}

fn b() {
  var I = vec2<i32>(3, 3);
  let p = &(I);
  a_U_X(10, U_X(u32(p.x)), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessUniformASTest, CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

@group(0) @binding(0) var<uniform> U : Outer;

fn f0(p : ptr<uniform, vec4<f32>>) -> f32 {
  return (*p).x;
}

fn f1(p : ptr<uniform, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&(*p)[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &(*p)[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of U
    res += f0(&U.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of U
    let p_vec = &U.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<uniform, Inner>) -> f32 {
  let p_mat = &(*p).mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<uniform, InnerArr>, p1 : ptr<uniform, mat3x4<f32>>) -> f32 {
  let p0_inner = &(*p0)[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<uniform, Outer>) -> f32 {
  return f3(&(*p).arr, &U.mat);
}

fn b() {
  f4(&U);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
}

@group(0) @binding(0) var<uniform> U : Outer;

alias U_mat_X = array<u32, 1u>;

fn f0_U_mat_X(p : U_mat_X) -> f32 {
  return U.mat[p[0]].x;
}

alias U_arr_X_mat_X = array<u32, 2u>;

fn f0_U_arr_X_mat_X(p : U_arr_X_mat_X) -> f32 {
  return U.arr[p[0]].mat[p[1]].x;
}

fn f1_U_mat() -> f32 {
  var res : f32;
  {
    res += f0_U_mat_X(U_mat_X(1));
  }
  {
    let p_vec = &(U.mat[1]);
    res += f0_U_mat_X(U_mat_X(1));
  }
  {
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(U.arr[2].mat[1]);
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(2, 1));
  }
  return res;
}

alias U_arr_X_mat = array<u32, 1u>;

fn f1_U_arr_X_mat(p : U_arr_X_mat) -> f32 {
  var res : f32;
  {
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(p[0u], 1));
  }
  {
    let p_vec = &(U.arr[p[0]].mat[1]);
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(p[0u], 1));
  }
  {
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(U.arr[2].mat[1]);
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(2, 1));
  }
  return res;
}

alias U_arr_X = array<u32, 1u>;

fn f2_U_arr_X(p : U_arr_X) -> f32 {
  let p_mat = &(U.arr[p[0]].mat);
  return f1_U_arr_X_mat(U_arr_X_mat(p[0u]));
}

fn f3_U_arr_U_mat() -> f32 {
  let p0_inner = &(U.arr[3]);
  return (f2_U_arr_X(U_arr_X(3)) + f1_U_mat());
}

fn f4_U() -> f32 {
  return f3_U_arr_U_mat();
}

fn b() {
  f4_U();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessUniformASTest, CallChaining_ViaPointerDotOrIndex) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

@group(0) @binding(0) var<uniform> U : Outer;

fn f0(p : ptr<uniform, vec4<f32>>) -> f32 {
  return p.x;
}

fn f1(p : ptr<uniform, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&p[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &p[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of U
    res += f0(&U.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of U
    let p_vec = &U.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<uniform, Inner>) -> f32 {
  let p_mat = &p.mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<uniform, InnerArr>, p1 : ptr<uniform, mat3x4<f32>>) -> f32 {
  let p0_inner = &(*p0)[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<uniform, Outer>) -> f32 {
  return f3(&p.arr, &U.mat);
}

fn b() {
  f4(&U);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
}

@group(0) @binding(0) var<uniform> U : Outer;

alias U_mat_X = array<u32, 1u>;

fn f0_U_mat_X(p : U_mat_X) -> f32 {
  return (&(U.mat[p[0]])).x;
}

alias U_arr_X_mat_X = array<u32, 2u>;

fn f0_U_arr_X_mat_X(p : U_arr_X_mat_X) -> f32 {
  return (&(U.arr[p[0]].mat[p[1]])).x;
}

fn f1_U_mat() -> f32 {
  var res : f32;
  {
    res += f0_U_mat_X(U_mat_X(1));
  }
  {
    let p_vec = &(U.mat[1]);
    res += f0_U_mat_X(U_mat_X(1));
  }
  {
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(U.arr[2].mat[1]);
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(2, 1));
  }
  return res;
}

alias U_arr_X_mat = array<u32, 1u>;

fn f1_U_arr_X_mat(p : U_arr_X_mat) -> f32 {
  var res : f32;
  {
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(p[0u], 1));
  }
  {
    let p_vec = &(U.arr[p[0]].mat[1]);
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(p[0u], 1));
  }
  {
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(U.arr[2].mat[1]);
    res += f0_U_arr_X_mat_X(U_arr_X_mat_X(2, 1));
  }
  return res;
}

alias U_arr_X = array<u32, 1u>;

fn f2_U_arr_X(p : U_arr_X) -> f32 {
  let p_mat = &(U.arr[p[0]].mat);
  return f1_U_arr_X_mat(U_arr_X_mat(p[0u]));
}

fn f3_U_arr_U_mat() -> f32 {
  let p0_inner = &(U.arr[3]);
  return (f2_U_arr_X(U_arr_X(3)) + f1_U_mat());
}

fn f4_U() -> f32 {
  return f3_U_arr_U_mat();
}

fn b() {
  f4_U();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessUniformASTest, CallChaining2) {
    auto* src = R"(
alias T3 = vec4i;
alias T2 = array<T3, 5>;
alias T1 = array<T2, 5>;
alias T = array<T1, 5>;

@binding(0) @group(0) var<uniform> input : T;

fn f2(p : ptr<uniform, T2>) -> T3 {
  return (*p)[3];
}

fn f1(p : ptr<uniform, T1>) -> T3 {
  return f2(&(*p)[2]);
}

fn f0(p : ptr<uniform, T>) -> T3 {
  return f1(&(*p)[1]);
}

@compute @workgroup_size(1)
fn main() {
  f0(&input);
}
)";

    auto* expect =
        R"(
alias T3 = vec4i;

alias T2 = array<T3, 5>;

alias T1 = array<T2, 5>;

alias T = array<T1, 5>;

@binding(0) @group(0) var<uniform> input : T;

alias input_X_X = array<u32, 2u>;

fn f2_input_X_X(p : input_X_X) -> T3 {
  return input[p[0]][p[1]][3];
}

alias input_X = array<u32, 1u>;

fn f1_input_X(p : input_X) -> T3 {
  return f2_input_X_X(input_X_X(p[0u], 2));
}

fn f0_input() -> T3 {
  return f1_input_X(input_X(1));
}

@compute @workgroup_size(1)
fn main() {
  f0_input();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace uniform_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'storage' address space
////////////////////////////////////////////////////////////////////////////////
namespace storage_as_tests {

using DirectVariableAccessStorageASTest = TransformTest;

TEST_F(DirectVariableAccessStorageASTest, Param_ptr_i32_Via_struct_read) {
    auto* src = R"(
struct str {
  i : i32,
};

@group(0) @binding(0) var<storage> S : str;

fn a(pre : i32, p : ptr<storage, i32>, post : i32) -> i32 {
  return *p;
}

fn b() {
  a(10, &S.i, 20);
}
)";

    auto* expect = R"(
struct str {
  i : i32,
}

@group(0) @binding(0) var<storage> S : str;

fn a_S_i(pre : i32, post : i32) -> i32 {
  return S.i;
}

fn b() {
  a_S_i(10, 20);
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessStorageASTest, Param_ptr_arr_i32_Via_struct_write) {
    auto* src = R"(
struct str {
  arr : array<i32, 4>,
};

@group(0) @binding(0) var<storage, read_write> S : str;

fn a(pre : i32, p : ptr<storage, array<i32, 4>, read_write>, post : i32) {
  *p = array<i32, 4>();
}

fn b() {
  a(10, &S.arr, 20);
}
)";

    auto* expect = R"(
struct str {
  arr : array<i32, 4>,
}

@group(0) @binding(0) var<storage, read_write> S : str;

fn a_S_arr(pre : i32, post : i32) {
  S.arr = array<i32, 4>();
}

fn b() {
  a_S_arr(10, 20);
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessStorageASTest, Param_ptr_vec4i32_Via_array_DynamicWrite) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> S : array<vec4<i32>, 8>;

fn a(pre : i32, p : ptr<storage, vec4<i32>, read_write>, post : i32) {
  *p = vec4<i32>();
}

fn b() {
  let I = 3;
  a(10, &S[I], 20);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<storage, read_write> S : array<vec4<i32>, 8>;

alias S_X = array<u32, 1u>;

fn a_S_X(pre : i32, p : S_X, post : i32) {
  S[p[0]] = vec4<i32>();
}

fn b() {
  let I = 3;
  a_S_X(10, S_X(u32(I)), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessStorageASTest, CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

@group(0) @binding(0) var<storage> S : Outer;

fn f0(p : ptr<storage, vec4<f32>>) -> f32 {
  return (*p).x;
}

fn f1(p : ptr<storage, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&(*p)[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &(*p)[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of S
    res += f0(&S.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of S
    let p_vec = &S.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<storage, Inner>) -> f32 {
  let p_mat = &(*p).mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<storage, InnerArr>, p1 : ptr<storage, mat3x4<f32>>) -> f32 {
  let p0_inner = &(*p0)[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<storage, Outer>) -> f32 {
  return f3(&(*p).arr, &S.mat);
}

fn b() {
  f4(&S);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
}

@group(0) @binding(0) var<storage> S : Outer;

alias S_mat_X = array<u32, 1u>;

fn f0_S_mat_X(p : S_mat_X) -> f32 {
  return S.mat[p[0]].x;
}

alias S_arr_X_mat_X = array<u32, 2u>;

fn f0_S_arr_X_mat_X(p : S_arr_X_mat_X) -> f32 {
  return S.arr[p[0]].mat[p[1]].x;
}

fn f1_S_mat() -> f32 {
  var res : f32;
  {
    res += f0_S_mat_X(S_mat_X(1));
  }
  {
    let p_vec = &(S.mat[1]);
    res += f0_S_mat_X(S_mat_X(1));
  }
  {
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(S.arr[2].mat[1]);
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(2, 1));
  }
  return res;
}

alias S_arr_X_mat = array<u32, 1u>;

fn f1_S_arr_X_mat(p : S_arr_X_mat) -> f32 {
  var res : f32;
  {
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(p[0u], 1));
  }
  {
    let p_vec = &(S.arr[p[0]].mat[1]);
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(p[0u], 1));
  }
  {
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(S.arr[2].mat[1]);
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(2, 1));
  }
  return res;
}

alias S_arr_X = array<u32, 1u>;

fn f2_S_arr_X(p : S_arr_X) -> f32 {
  let p_mat = &(S.arr[p[0]].mat);
  return f1_S_arr_X_mat(S_arr_X_mat(p[0u]));
}

fn f3_S_arr_S_mat() -> f32 {
  let p0_inner = &(S.arr[3]);
  return (f2_S_arr_X(S_arr_X(3)) + f1_S_mat());
}

fn f4_S() -> f32 {
  return f3_S_arr_S_mat();
}

fn b() {
  f4_S();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessStorageASTest, CallChaining_ViaPointerDotOrIndex) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

@group(0) @binding(0) var<storage> S : Outer;

fn f0(p : ptr<storage, vec4<f32>>) -> f32 {
  return p.x;
}

fn f1(p : ptr<storage, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&p[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &p[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of S
    res += f0(&S.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of S
    let p_vec = &S.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<storage, Inner>) -> f32 {
  let p_mat = &p.mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<storage, InnerArr>, p1 : ptr<storage, mat3x4<f32>>) -> f32 {
  let p0_inner = &p0[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<storage, Outer>) -> f32 {
  return f3(&p.arr, &S.mat);
}

fn b() {
  f4(&S);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
}

@group(0) @binding(0) var<storage> S : Outer;

alias S_mat_X = array<u32, 1u>;

fn f0_S_mat_X(p : S_mat_X) -> f32 {
  return (&(S.mat[p[0]])).x;
}

alias S_arr_X_mat_X = array<u32, 2u>;

fn f0_S_arr_X_mat_X(p : S_arr_X_mat_X) -> f32 {
  return (&(S.arr[p[0]].mat[p[1]])).x;
}

fn f1_S_mat() -> f32 {
  var res : f32;
  {
    res += f0_S_mat_X(S_mat_X(1));
  }
  {
    let p_vec = &(S.mat[1]);
    res += f0_S_mat_X(S_mat_X(1));
  }
  {
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(S.arr[2].mat[1]);
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(2, 1));
  }
  return res;
}

alias S_arr_X_mat = array<u32, 1u>;

fn f1_S_arr_X_mat(p : S_arr_X_mat) -> f32 {
  var res : f32;
  {
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(p[0u], 1));
  }
  {
    let p_vec = &(S.arr[p[0]].mat[1]);
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(p[0u], 1));
  }
  {
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(S.arr[2].mat[1]);
    res += f0_S_arr_X_mat_X(S_arr_X_mat_X(2, 1));
  }
  return res;
}

alias S_arr_X = array<u32, 1u>;

fn f2_S_arr_X(p : S_arr_X) -> f32 {
  let p_mat = &(S.arr[p[0]].mat);
  return f1_S_arr_X_mat(S_arr_X_mat(p[0u]));
}

fn f3_S_arr_S_mat() -> f32 {
  let p0_inner = &(S.arr[3]);
  return (f2_S_arr_X(S_arr_X(3)) + f1_S_mat());
}

fn f4_S() -> f32 {
  return f3_S_arr_S_mat();
}

fn b() {
  f4_S();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessStorageASTest, CallChaining2) {
    auto* src = R"(
alias T3 = vec4i;
alias T2 = array<T3, 5>;
alias T1 = array<T2, 5>;
alias T = array<T1, 5>;

@binding(0) @group(0) var<storage> input : T;

fn f2(p : ptr<storage, T2>) -> T3 {
  return (*p)[3];
}

fn f1(p : ptr<storage, T1>) -> T3 {
  return f2(&(*p)[2]);
}

fn f0(p : ptr<storage, T>) -> T3 {
  return f1(&(*p)[1]);
}

@compute @workgroup_size(1)
fn main() {
  f0(&input);
}
)";

    auto* expect =
        R"(
alias T3 = vec4i;

alias T2 = array<T3, 5>;

alias T1 = array<T2, 5>;

alias T = array<T1, 5>;

@binding(0) @group(0) var<storage> input : T;

alias input_X_X = array<u32, 2u>;

fn f2_input_X_X(p : input_X_X) -> T3 {
  return input[p[0]][p[1]][3];
}

alias input_X = array<u32, 1u>;

fn f1_input_X(p : input_X) -> T3 {
  return f2_input_X_X(input_X_X(p[0u], 2));
}

fn f0_input() -> T3 {
  return f1_input_X(input_X(1));
}

@compute @workgroup_size(1)
fn main() {
  f0_input();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace storage_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'workgroup' address space
////////////////////////////////////////////////////////////////////////////////
namespace workgroup_as_tests {

using DirectVariableAccessWorkgroupASTest = TransformTest;

TEST_F(DirectVariableAccessWorkgroupASTest, Param_ptr_vec4i32_Via_array_StaticRead) {
    auto* src = R"(
var<workgroup> W : array<vec4<i32>, 8>;

fn a(pre : i32, p : ptr<workgroup, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  a(10, &W[3], 20);
}
)";

    auto* expect = R"(
var<workgroup> W : array<vec4<i32>, 8>;

alias W_X = array<u32, 1u>;

fn a_W_X(pre : i32, p : W_X, post : i32) -> vec4<i32> {
  return W[p[0]];
}

fn b() {
  a_W_X(10, W_X(3), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessWorkgroupASTest, Param_ptr_vec4i32_Via_array_StaticWrite) {
    auto* src = R"(
var<workgroup> W : array<vec4<i32>, 8>;

fn a(pre : i32, p : ptr<workgroup, vec4<i32>>, post : i32) {
  *p = vec4<i32>();
}

fn b() {
  a(10, &W[3], 20);
}
)";

    auto* expect = R"(
var<workgroup> W : array<vec4<i32>, 8>;

alias W_X = array<u32, 1u>;

fn a_W_X(pre : i32, p : W_X, post : i32) {
  W[p[0]] = vec4<i32>();
}

fn b() {
  a_W_X(10, W_X(3), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessWorkgroupASTest, CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

var<workgroup> W : Outer;

fn f0(p : ptr<workgroup, vec4<f32>>) -> f32 {
  return (*p).x;
}

fn f1(p : ptr<workgroup, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&(*p)[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &(*p)[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of W
    res += f0(&W.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of W
    let p_vec = &W.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<workgroup, Inner>) -> f32 {
  let p_mat = &(*p).mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<workgroup, InnerArr>, p1 : ptr<workgroup, mat3x4<f32>>) -> f32 {
  let p0_inner = &(*p0)[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<workgroup, Outer>) -> f32 {
  return f3(&(*p).arr, &W.mat);
}

fn b() {
  f4(&W);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
}

var<workgroup> W : Outer;

alias W_mat_X = array<u32, 1u>;

fn f0_W_mat_X(p : W_mat_X) -> f32 {
  return W.mat[p[0]].x;
}

alias W_arr_X_mat_X = array<u32, 2u>;

fn f0_W_arr_X_mat_X(p : W_arr_X_mat_X) -> f32 {
  return W.arr[p[0]].mat[p[1]].x;
}

fn f1_W_mat() -> f32 {
  var res : f32;
  {
    res += f0_W_mat_X(W_mat_X(1));
  }
  {
    let p_vec = &(W.mat[1]);
    res += f0_W_mat_X(W_mat_X(1));
  }
  {
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(W.arr[2].mat[1]);
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(2, 1));
  }
  return res;
}

alias W_arr_X_mat = array<u32, 1u>;

fn f1_W_arr_X_mat(p : W_arr_X_mat) -> f32 {
  var res : f32;
  {
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(p[0u], 1));
  }
  {
    let p_vec = &(W.arr[p[0]].mat[1]);
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(p[0u], 1));
  }
  {
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(W.arr[2].mat[1]);
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(2, 1));
  }
  return res;
}

alias W_arr_X = array<u32, 1u>;

fn f2_W_arr_X(p : W_arr_X) -> f32 {
  let p_mat = &(W.arr[p[0]].mat);
  return f1_W_arr_X_mat(W_arr_X_mat(p[0u]));
}

fn f3_W_arr_W_mat() -> f32 {
  let p0_inner = &(W.arr[3]);
  return (f2_W_arr_X(W_arr_X(3)) + f1_W_mat());
}

fn f4_W() -> f32 {
  return f3_W_arr_W_mat();
}

fn b() {
  f4_W();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessWorkgroupASTest, CallChaining_ViaPointerDotOrIndex) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

var<workgroup> W : Outer;

fn f0(p : ptr<workgroup, vec4<f32>>) -> f32 {
  return p.x;
}

fn f1(p : ptr<workgroup, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&p[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &p[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of W
    res += f0(&W.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of W
    let p_vec = &W.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<workgroup, Inner>) -> f32 {
  let p_mat = &p.mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<workgroup, InnerArr>, p1 : ptr<workgroup, mat3x4<f32>>) -> f32 {
  let p0_inner = &p0[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<workgroup, Outer>) -> f32 {
  return f3(&p.arr, &W.mat);
}

fn b() {
  f4(&W);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
}

var<workgroup> W : Outer;

alias W_mat_X = array<u32, 1u>;

fn f0_W_mat_X(p : W_mat_X) -> f32 {
  return (&(W.mat[p[0]])).x;
}

alias W_arr_X_mat_X = array<u32, 2u>;

fn f0_W_arr_X_mat_X(p : W_arr_X_mat_X) -> f32 {
  return (&(W.arr[p[0]].mat[p[1]])).x;
}

fn f1_W_mat() -> f32 {
  var res : f32;
  {
    res += f0_W_mat_X(W_mat_X(1));
  }
  {
    let p_vec = &(W.mat[1]);
    res += f0_W_mat_X(W_mat_X(1));
  }
  {
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(W.arr[2].mat[1]);
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(2, 1));
  }
  return res;
}

alias W_arr_X_mat = array<u32, 1u>;

fn f1_W_arr_X_mat(p : W_arr_X_mat) -> f32 {
  var res : f32;
  {
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(p[0u], 1));
  }
  {
    let p_vec = &(W.arr[p[0]].mat[1]);
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(p[0u], 1));
  }
  {
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(2, 1));
  }
  {
    let p_vec = &(W.arr[2].mat[1]);
    res += f0_W_arr_X_mat_X(W_arr_X_mat_X(2, 1));
  }
  return res;
}

alias W_arr_X = array<u32, 1u>;

fn f2_W_arr_X(p : W_arr_X) -> f32 {
  let p_mat = &(W.arr[p[0]].mat);
  return f1_W_arr_X_mat(W_arr_X_mat(p[0u]));
}

fn f3_W_arr_W_mat() -> f32 {
  let p0_inner = &(W.arr[3]);
  return (f2_W_arr_X(W_arr_X(3)) + f1_W_mat());
}

fn f4_W() -> f32 {
  return f3_W_arr_W_mat();
}

fn b() {
  f4_W();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessWorkgroupASTest, CallChaining2) {
    auto* src = R"(
alias T3 = vec4i;
alias T2 = array<T3, 5>;
alias T1 = array<T2, 5>;
alias T = array<T1, 5>;

@binding(0) @group(0) var<storage, read> input : T;

fn f2(p : ptr<workgroup, T2>) -> T3 {
  return (*p)[3];
}

fn f1(p : ptr<workgroup, T1>) -> T3 {
  return f2(&(*p)[2]);
}

fn f0(p : ptr<workgroup, T>) -> T3 {
  return f1(&(*p)[1]);
}

var<workgroup> W : T;
@compute @workgroup_size(1)
fn main() {
  W = input;
  f0(&W);
}
)";

    auto* expect =
        R"(
alias T3 = vec4i;

alias T2 = array<T3, 5>;

alias T1 = array<T2, 5>;

alias T = array<T1, 5>;

@binding(0) @group(0) var<storage, read> input : T;

alias W_X_X = array<u32, 2u>;

fn f2_W_X_X(p : W_X_X) -> T3 {
  return W[p[0]][p[1]][3];
}

alias W_X = array<u32, 1u>;

fn f1_W_X(p : W_X) -> T3 {
  return f2_W_X_X(W_X_X(p[0u], 2));
}

fn f0_W() -> T3 {
  return f1_W_X(W_X(1));
}

var<workgroup> W : T;

@compute @workgroup_size(1)
fn main() {
  W = input;
  f0_W();
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace workgroup_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'private' address space
////////////////////////////////////////////////////////////////////////////////
namespace private_as_tests {

using DirectVariableAccessPrivateASTest = TransformTest;

TEST_F(DirectVariableAccessPrivateASTest, Enabled_Param_ptr_i32_read) {
    auto* src = R"(
fn a(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}

var<private> P : i32;

fn b() {
  a(10, &(P), 20);
}
)";

    auto* expect = R"(
fn a_F(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}

var<private> P : i32;

fn b() {
  a_F(10, &(P), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnablePrivate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPrivateASTest, Enabled_Param_ptr_i32_write) {
    auto* src = R"(
fn a(pre : i32, p : ptr<private, i32>, post : i32) {
  *(p) = 42;
}

var<private> P : i32;

fn b() {
  a(10, &(P), 20);
}
)";

    auto* expect = R"(
fn a_F(pre : i32, p : ptr<private, i32>, post : i32) {
  *(p) = 42;
}

var<private> P : i32;

fn b() {
  a_F(10, &(P), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnablePrivate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPrivateASTest, Enabled_Param_ptr_i32_Via_struct_read) {
    auto* src = R"(
struct str {
  i : i32,
};

fn a(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *p;
}

var<private> P : str;

fn b() {
  a(10, &P.i, 20);
}
)";

    auto* expect = R"(
struct str {
  i : i32,
}

fn a_F_i(pre : i32, p : ptr<private, str>, post : i32) -> i32 {
  return (*(p)).i;
}

var<private> P : str;

fn b() {
  a_F_i(10, &(P), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnablePrivate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPrivateASTest, Disabled_Param_ptr_i32_Via_struct_read) {
    auto* src = R"(
struct str {
  i : i32,
}

fn a(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}

var<private> P : str;

fn b() {
  a(10, &(P.i), 20);
}
)";

    auto* expect = src;

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPrivateASTest, Enabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* src = R"(
struct str {
  arr : array<i32, 4>,
};

fn a(pre : i32, p : ptr<private, array<i32, 4>>, post : i32) {
  *p = array<i32, 4>();
}

var<private> P : str;

fn b() {
  a(10, &P.arr, 20);
}
)";

    auto* expect = R"(
struct str {
  arr : array<i32, 4>,
}

fn a_F_arr(pre : i32, p : ptr<private, str>, post : i32) {
  (*(p)).arr = array<i32, 4>();
}

var<private> P : str;

fn b() {
  a_F_arr(10, &(P), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnablePrivate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPrivateASTest, Disabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* src = R"(
struct str {
  arr : array<i32, 4>,
}

fn a(pre : i32, p : ptr<private, array<i32, 4>>, post : i32) {
  *(p) = array<i32, 4>();
}

var<private> P : str;

fn b() {
  a(10, &(P.arr), 20);
}
)";

    auto* expect = src;

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPrivateASTest, Enabled_Param_ptr_i32_mixed) {
    auto* src = R"(
struct str {
  i : i32,
};

fn a(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *p;
}

var<private> Pi : i32;
var<private> Ps : str;
var<private> Pa : array<i32, 4>;

fn b() {
  a(10, &Pi, 20);
  a(30, &Ps.i, 40);
  a(50, &Pa[2], 60);
}
)";

    auto* expect = R"(
struct str {
  i : i32,
}

fn a_F(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}

fn a_F_i(pre : i32, p : ptr<private, str>, post : i32) -> i32 {
  return (*(p)).i;
}

alias F_X = array<u32, 1u>;

fn a_F_X(pre : i32, p_base : ptr<private, array<i32, 4u>>, p_indices : F_X, post : i32) -> i32 {
  return (*(p_base))[p_indices[0]];
}

var<private> Pi : i32;

var<private> Ps : str;

var<private> Pa : array<i32, 4>;

alias F_X_1 = array<u32, 1u>;

fn b() {
  a_F(10, &(Pi), 20);
  a_F_i(30, &(Ps), 40);
  a_F_X(50, &(Pa), F_X_1(2), 60);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnablePrivate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPrivateASTest, Disabled_Param_ptr_i32_mixed) {
    auto* src = R"(
struct str {
  i : i32,
}

fn a(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}

var<private> Pi : i32;

var<private> Ps : str;

var<private> Pa : array<i32, 4>;

fn b() {
  a(10, &(Pi), 20);
  a(10, &(Ps.i), 20);
  a(10, &(Pa[2]), 20);
}
)";

    auto* expect = src;

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPrivateASTest, Enabled_CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

var<private> P : Outer;

fn f0(p : ptr<private, vec4<f32>>) -> f32 {
  return (*p).x;
}

fn f1(p : ptr<private, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&(*p)[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &(*p)[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of P
    res += f0(&P.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of P
    let p_vec = &P.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<private, Inner>) -> f32 {
  let p_mat = &(*p).mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<private, InnerArr>, p1 : ptr<private, mat3x4<f32>>) -> f32 {
  let p0_inner = &(*p0)[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<private, Outer>) -> f32 {
  return f3(&(*p).arr, &P.mat);
}

fn b() {
  f4(&P);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
}

var<private> P : Outer;

alias F_mat_X = array<u32, 1u>;

fn f0_F_mat_X(p_base : ptr<private, Outer>, p_indices : F_mat_X) -> f32 {
  return (*(p_base)).mat[p_indices[0]].x;
}

alias F_arr_X_mat_X = array<u32, 2u>;

fn f0_F_arr_X_mat_X(p_base : ptr<private, Outer>, p_indices : F_arr_X_mat_X) -> f32 {
  return (*(p_base)).arr[p_indices[0]].mat[p_indices[1]].x;
}

alias F_mat_X_1 = array<u32, 1u>;

alias F_arr_X_mat_X_1 = array<u32, 2u>;

fn f1_F_mat(p : ptr<private, Outer>) -> f32 {
  var res : f32;
  {
    res += f0_F_mat_X(p, F_mat_X_1(1));
  }
  {
    let p_vec = &((*(p)).mat[1]);
    res += f0_F_mat_X(p, F_mat_X_1(1));
  }
  {
    res += f0_F_arr_X_mat_X(&(P), F_arr_X_mat_X_1(2, 1));
  }
  {
    let p_vec = &(P.arr[2].mat[1]);
    res += f0_F_arr_X_mat_X(&(P), F_arr_X_mat_X_1(2, 1));
  }
  return res;
}

alias F_arr_X_mat = array<u32, 1u>;

alias F_arr_X_mat_X_2 = array<u32, 2u>;

fn f1_F_arr_X_mat(p_base : ptr<private, Outer>, p_indices : F_arr_X_mat) -> f32 {
  var res : f32;
  {
    res += f0_F_arr_X_mat_X(p_base, F_arr_X_mat_X_2(p_indices[0u], 1));
  }
  {
    let p_vec = &((*(p_base)).arr[p_indices[0]].mat[1]);
    res += f0_F_arr_X_mat_X(p_base, F_arr_X_mat_X_2(p_indices[0u], 1));
  }
  {
    res += f0_F_arr_X_mat_X(&(P), F_arr_X_mat_X_1(2, 1));
  }
  {
    let p_vec = &(P.arr[2].mat[1]);
    res += f0_F_arr_X_mat_X(&(P), F_arr_X_mat_X_1(2, 1));
  }
  return res;
}

alias F_arr_X = array<u32, 1u>;

alias F_arr_X_mat_1 = array<u32, 1u>;

fn f2_F_arr_X(p_base : ptr<private, Outer>, p_indices : F_arr_X) -> f32 {
  let p_mat = &((*(p_base)).arr[p_indices[0]].mat);
  return f1_F_arr_X_mat(p_base, F_arr_X_mat_1(p_indices[0u]));
}

alias F_arr_X_1 = array<u32, 1u>;

fn f3_F_arr_F_mat(p0 : ptr<private, Outer>, p1 : ptr<private, Outer>) -> f32 {
  let p0_inner = &((*(p0)).arr[3]);
  return (f2_F_arr_X(p0, F_arr_X_1(3)) + f1_F_mat(p1));
}

fn f4_F(p : ptr<private, Outer>) -> f32 {
  return f3_F_arr_F_mat(p, &(P));
}

fn b() {
  f4_F(&(P));
}
)";

    auto got = Run<DirectVariableAccess>(src, EnablePrivate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPrivateASTest, Enabled_CallChaining2) {
    auto* src = R"(
alias T3 = vec4i;
alias T2 = array<T3, 5>;
alias T1 = array<T2, 5>;
alias T = array<T1, 5>;

fn f2(p : ptr<private, T2>) -> T3 {
  return (*p)[3];
}

fn f1(p : ptr<private, T1>) -> T3 {
  return f2(&(*p)[2]);
}

fn f0(p : ptr<private, T>) -> T3 {
  return f1(&(*p)[1]);
}

var<private> P : T;

@compute @workgroup_size(1)
fn main() {
  f0(&P);
}
)";

    auto* expect =
        R"(
alias T3 = vec4i;

alias T2 = array<T3, 5>;

alias T1 = array<T2, 5>;

alias T = array<T1, 5>;

alias F_X_X = array<u32, 2u>;

fn f2_F_X_X(p_base : ptr<private, array<array<array<vec4<i32>, 5u>, 5u>, 5u>>, p_indices : F_X_X) -> T3 {
  return (*(p_base))[p_indices[0]][p_indices[1]][3];
}

alias F_X = array<u32, 1u>;

alias F_X_X_1 = array<u32, 2u>;

fn f1_F_X(p_base : ptr<private, array<array<array<vec4<i32>, 5u>, 5u>, 5u>>, p_indices : F_X) -> T3 {
  return f2_F_X_X(p_base, F_X_X_1(p_indices[0u], 2));
}

alias F_X_1 = array<u32, 1u>;

fn f0_F(p : ptr<private, array<array<array<vec4<i32>, 5u>, 5u>, 5u>>) -> T3 {
  return f1_F_X(p, F_X_1(1));
}

var<private> P : T;

@compute @workgroup_size(1)
fn main() {
  f0_F(&(P));
}
)";

    auto got = Run<DirectVariableAccess>(src, EnablePrivate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessPrivateASTest, Disabled_CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
}

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
}

var<private> P : Outer;

fn f0(p : ptr<private, vec4<f32>>) -> f32 {
  return (*(p)).x;
}

fn f1(p : ptr<private, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    res += f0(&((*(p))[1]));
  }
  {
    let p_vec = &((*(p))[1]);
    res += f0(p_vec);
  }
  {
    res += f0(&(P.arr[2].mat[1]));
  }
  {
    let p_vec = &(P.arr[2].mat[1]);
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<private, Inner>) -> f32 {
  let p_mat = &((*(p)).mat);
  return f1(p_mat);
}

fn f3(p0 : ptr<private, InnerArr>, p1 : ptr<private, mat3x4<f32>>) -> f32 {
  let p0_inner = &((*(p0))[3]);
  return (f2(p0_inner) + f1(p1));
}

fn f4(p : ptr<private, Outer>) -> f32 {
  return f3(&((*(p)).arr), &(P.mat));
}

fn b() {
  f4(&(P));
}
)";

    auto* expect = src;

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace private_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'function' address space
////////////////////////////////////////////////////////////////////////////////
namespace function_as_tests {

using DirectVariableAccessFunctionASTest = TransformTest;

TEST_F(DirectVariableAccessFunctionASTest, Enabled_LocalPtr) {
    auto* src = R"(
fn f() {
  var v : i32;
  let p : ptr<function, i32> = &(v);
  var x : i32 = *(p);
}
)";

    auto* expect = src;  // Nothing changes

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessFunctionASTest, Enabled_Param_ptr_i32_read) {
    auto* src = R"(
fn a(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *(p);
}

fn b() {
  var F : i32;
  a(10, &(F), 20);
}
)";

    auto* expect = R"(
fn a_F(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *(p);
}

fn b() {
  var F : i32;
  a_F(10, &(F), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnableFunction());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessFunctionASTest, Enabled_Param_ptr_i32_write) {
    auto* src = R"(
fn a(pre : i32, p : ptr<function, i32>, post : i32) {
  *(p) = 42;
}

fn b() {
  var F : i32;
  a(10, &(F), 20);
}
)";

    auto* expect = R"(
fn a_F(pre : i32, p : ptr<function, i32>, post : i32) {
  *(p) = 42;
}

fn b() {
  var F : i32;
  a_F(10, &(F), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnableFunction());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessFunctionASTest, Enabled_Param_ptr_i32_Via_struct_read) {
    auto* src = R"(
struct str {
  i : i32,
};

fn a(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *p;
}

fn b() {
  var F : str;
  a(10, &F.i, 20);
}
)";

    auto* expect = R"(
struct str {
  i : i32,
}

fn a_F_i(pre : i32, p : ptr<function, str>, post : i32) -> i32 {
  return (*(p)).i;
}

fn b() {
  var F : str;
  a_F_i(10, &(F), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnableFunction());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessFunctionASTest, Enabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* src = R"(
struct str {
  arr : array<i32, 4>,
};

fn a(pre : i32, p : ptr<function, array<i32, 4>>, post : i32) {
  *p = array<i32, 4>();
}

fn b() {
  var F : str;
  a(10, &F.arr, 20);
}
)";

    auto* expect = R"(
struct str {
  arr : array<i32, 4>,
}

fn a_F_arr(pre : i32, p : ptr<function, str>, post : i32) {
  (*(p)).arr = array<i32, 4>();
}

fn b() {
  var F : str;
  a_F_arr(10, &(F), 20);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnableFunction());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessFunctionASTest, Enabled_Param_ptr_i32_mixed) {
    auto* src = R"(
struct str {
  i : i32,
};

fn a(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *p;
}

fn b() {
  var Fi : i32;
  var Fs : str;
  var Fa : array<i32, 4>;

  a(10, &Fi, 20);
  a(30, &Fs.i, 40);
  a(50, &Fa[2], 60);
}
)";

    auto* expect = R"(
struct str {
  i : i32,
}

fn a_F(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *(p);
}

fn a_F_i(pre : i32, p : ptr<function, str>, post : i32) -> i32 {
  return (*(p)).i;
}

alias F_X = array<u32, 1u>;

fn a_F_X(pre : i32, p_base : ptr<function, array<i32, 4u>>, p_indices : F_X, post : i32) -> i32 {
  return (*(p_base))[p_indices[0]];
}

alias F_X_1 = array<u32, 1u>;

fn b() {
  var Fi : i32;
  var Fs : str;
  var Fa : array<i32, 4>;
  a_F(10, &(Fi), 20);
  a_F_i(30, &(Fs), 40);
  a_F_X(50, &(Fa), F_X_1(2), 60);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnableFunction());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessFunctionASTest, Enabled_CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

fn f0(p : ptr<function, vec4<f32>>) -> f32 {
  return (*p).x;
}

fn f1(p : ptr<function, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&(*p)[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &(*p)[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<function, Inner>) -> f32 {
  let p_mat = &(*p).mat;
  return f1(p_mat);
}

fn f3(p : ptr<function, InnerArr>) -> f32 {
  let p_inner = &(*p)[3];
  return f2(p_inner);
}

fn f4(p : ptr<function, Outer>) -> f32 {
  return f3(&(*p).arr);
}

fn b() {
  var S : Outer;
  f4(&S);
}
)";

    auto* expect =
        R"(
struct Inner {
  mat : mat3x4<f32>,
}

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
}

fn f0(p : ptr<function, vec4<f32>>) -> f32 {
  return (*(p)).x;
}

fn f1(p : ptr<function, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    res += f0(&((*(p))[1]));
  }
  {
    let p_vec = &((*(p))[1]);
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<function, Inner>) -> f32 {
  let p_mat = &((*(p)).mat);
  return f1(p_mat);
}

fn f3(p : ptr<function, InnerArr>) -> f32 {
  let p_inner = &((*(p))[3]);
  return f2(p_inner);
}

fn f4(p : ptr<function, Outer>) -> f32 {
  return f3(&((*(p)).arr));
}

fn b() {
  var S : Outer;
  f4(&(S));
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessFunctionASTest, Enabled_CallChaining2) {
    auto* src = R"(
alias T3 = vec4i;
alias T2 = array<T3, 5>;
alias T1 = array<T2, 5>;
alias T = array<T1, 5>;

fn f2(p : ptr<function, T2>) -> T3 {
  return (*p)[3];
}

fn f1(p : ptr<function, T1>) -> T3 {
  return f2(&(*p)[2]);
}

fn f0(p : ptr<function, T>) -> T3 {
  return f1(&(*p)[1]);
}

@compute @workgroup_size(1)
fn main() {
  var v : T;
  f0(&v);
}
)";

    auto* expect =
        R"(
alias T3 = vec4i;

alias T2 = array<T3, 5>;

alias T1 = array<T2, 5>;

alias T = array<T1, 5>;

alias F_X_X = array<u32, 2u>;

fn f2_F_X_X(p_base : ptr<function, array<array<array<vec4<i32>, 5u>, 5u>, 5u>>, p_indices : F_X_X) -> T3 {
  return (*(p_base))[p_indices[0]][p_indices[1]][3];
}

alias F_X = array<u32, 1u>;

alias F_X_X_1 = array<u32, 2u>;

fn f1_F_X(p_base : ptr<function, array<array<array<vec4<i32>, 5u>, 5u>, 5u>>, p_indices : F_X) -> T3 {
  return f2_F_X_X(p_base, F_X_X_1(p_indices[0u], 2));
}

alias F_X_1 = array<u32, 1u>;

fn f0_F(p : ptr<function, array<array<array<vec4<i32>, 5u>, 5u>, 5u>>) -> T3 {
  return f1_F_X(p, F_X_1(1));
}

@compute @workgroup_size(1)
fn main() {
  var v : T;
  f0_F(&(v));
}
)";

    auto got = Run<DirectVariableAccess>(src, EnableFunction());

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessFunctionASTest, Disabled_Param_ptr_i32_Via_struct_read) {
    auto* src = R"(
struct str {
  i : i32,
}

fn a(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *(p);
}

fn b() {
  var F : str;
  a(10, &(F.i), 20);
}
)";

    auto* expect = src;

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessFunctionASTest, Disabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* src = R"(
struct str {
  arr : array<i32, 4>,
}

fn a(pre : i32, p : ptr<function, array<i32, 4>>, post : i32) {
  *(p) = array<i32, 4>();
}

fn b() {
  var F : str;
  a(10, &(F.arr), 20);
}
)";

    auto* expect = src;

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessFunctionASTest, PointerForwarding_NoUse) {
    auto* src = R"(
fn a(p : ptr<function, i32>) -> i32 {
  return *p;
}

fn b(p : ptr<function, i32>) -> i32 {
  return a(p);
}
)";

    auto* expect =
        R"(
fn a_F(p : ptr<function, i32>) -> i32 {
  return *(p);
}

fn b(p : ptr<function, i32>) -> i32 {
  return a_F(p);
}
)";

    auto got = Run<DirectVariableAccess>(src, EnableFunction());

    EXPECT_EQ(expect, str(got));
}

}  // namespace function_as_tests

////////////////////////////////////////////////////////////////////////////////
// complex tests
////////////////////////////////////////////////////////////////////////////////
namespace complex_tests {

using DirectVariableAccessComplexTest = TransformTest;

TEST_F(DirectVariableAccessComplexTest, Param_ptr_mixed_vec4i32_ViaMultiple) {
    auto* src = R"(
struct str {
  i : vec4<i32>,
};

@group(0) @binding(0) var<uniform> U     : vec4<i32>;
@group(0) @binding(1) var<uniform> U_str   : str;
@group(0) @binding(2) var<uniform> U_arr   : array<vec4<i32>, 8>;
@group(0) @binding(3) var<uniform> U_arr_arr : array<array<vec4<i32>, 8>, 4>;

@group(1) @binding(0) var<storage> S     : vec4<i32>;
@group(1) @binding(1) var<storage> S_str   : str;
@group(1) @binding(2) var<storage> S_arr   : array<vec4<i32>, 8>;
@group(1) @binding(3) var<storage> S_arr_arr : array<array<vec4<i32>, 8>, 4>;

          var<workgroup> W     : vec4<i32>;
          var<workgroup> W_str   : str;
          var<workgroup> W_arr   : array<vec4<i32>, 8>;
          var<workgroup> W_arr_arr : array<array<vec4<i32>, 8>, 4>;

fn fn_u(p : ptr<uniform, vec4<i32>>) -> vec4<i32> {
  return *p;
}

fn fn_s(p : ptr<storage, vec4<i32>>) -> vec4<i32> {
  return *p;
}

fn fn_w(p : ptr<workgroup, vec4<i32>>) -> vec4<i32> {
  return *p;
}

fn b() {
  let I = 3;
  let J = 4;

  let u           = fn_u(&U);
  let u_str       = fn_u(&U_str.i);
  let u_arr0      = fn_u(&U_arr[0]);
  let u_arr1      = fn_u(&U_arr[1]);
  let u_arrI      = fn_u(&U_arr[I]);
  let u_arr1_arr0 = fn_u(&U_arr_arr[1][0]);
  let u_arr2_arrI = fn_u(&U_arr_arr[2][I]);
  let u_arrI_arr2 = fn_u(&U_arr_arr[I][2]);
  let u_arrI_arrJ = fn_u(&U_arr_arr[I][J]);

  let s           = fn_s(&S);
  let s_str       = fn_s(&S_str.i);
  let s_arr0      = fn_s(&S_arr[0]);
  let s_arr1      = fn_s(&S_arr[1]);
  let s_arrI      = fn_s(&S_arr[I]);
  let s_arr1_arr0 = fn_s(&S_arr_arr[1][0]);
  let s_arr2_arrI = fn_s(&S_arr_arr[2][I]);
  let s_arrI_arr2 = fn_s(&S_arr_arr[I][2]);
  let s_arrI_arrJ = fn_s(&S_arr_arr[I][J]);

  let w           = fn_w(&W);
  let w_str       = fn_w(&W_str.i);
  let w_arr0      = fn_w(&W_arr[0]);
  let w_arr1      = fn_w(&W_arr[1]);
  let w_arrI      = fn_w(&W_arr[I]);
  let w_arr1_arr0 = fn_w(&W_arr_arr[1][0]);
  let w_arr2_arrI = fn_w(&W_arr_arr[2][I]);
  let w_arrI_arr2 = fn_w(&W_arr_arr[I][2]);
  let w_arrI_arrJ = fn_w(&W_arr_arr[I][J]);
}
)";

    auto* expect = R"(
struct str {
  i : vec4<i32>,
}

@group(0) @binding(0) var<uniform> U : vec4<i32>;

@group(0) @binding(1) var<uniform> U_str : str;

@group(0) @binding(2) var<uniform> U_arr : array<vec4<i32>, 8>;

@group(0) @binding(3) var<uniform> U_arr_arr : array<array<vec4<i32>, 8>, 4>;

@group(1) @binding(0) var<storage> S : vec4<i32>;

@group(1) @binding(1) var<storage> S_str : str;

@group(1) @binding(2) var<storage> S_arr : array<vec4<i32>, 8>;

@group(1) @binding(3) var<storage> S_arr_arr : array<array<vec4<i32>, 8>, 4>;

var<workgroup> W : vec4<i32>;

var<workgroup> W_str : str;

var<workgroup> W_arr : array<vec4<i32>, 8>;

var<workgroup> W_arr_arr : array<array<vec4<i32>, 8>, 4>;

fn fn_u_U() -> vec4<i32> {
  return U;
}

fn fn_u_U_str_i() -> vec4<i32> {
  return U_str.i;
}

alias U_arr_X = array<u32, 1u>;

fn fn_u_U_arr_X(p : U_arr_X) -> vec4<i32> {
  return U_arr[p[0]];
}

alias U_arr_arr_X_X = array<u32, 2u>;

fn fn_u_U_arr_arr_X_X(p : U_arr_arr_X_X) -> vec4<i32> {
  return U_arr_arr[p[0]][p[1]];
}

fn fn_s_S() -> vec4<i32> {
  return S;
}

fn fn_s_S_str_i() -> vec4<i32> {
  return S_str.i;
}

alias S_arr_X = array<u32, 1u>;

fn fn_s_S_arr_X(p : S_arr_X) -> vec4<i32> {
  return S_arr[p[0]];
}

alias S_arr_arr_X_X = array<u32, 2u>;

fn fn_s_S_arr_arr_X_X(p : S_arr_arr_X_X) -> vec4<i32> {
  return S_arr_arr[p[0]][p[1]];
}

fn fn_w_W() -> vec4<i32> {
  return W;
}

fn fn_w_W_str_i() -> vec4<i32> {
  return W_str.i;
}

alias W_arr_X = array<u32, 1u>;

fn fn_w_W_arr_X(p : W_arr_X) -> vec4<i32> {
  return W_arr[p[0]];
}

alias W_arr_arr_X_X = array<u32, 2u>;

fn fn_w_W_arr_arr_X_X(p : W_arr_arr_X_X) -> vec4<i32> {
  return W_arr_arr[p[0]][p[1]];
}

fn b() {
  let I = 3;
  let J = 4;
  let u = fn_u_U();
  let u_str = fn_u_U_str_i();
  let u_arr0 = fn_u_U_arr_X(U_arr_X(0));
  let u_arr1 = fn_u_U_arr_X(U_arr_X(1));
  let u_arrI = fn_u_U_arr_X(U_arr_X(u32(I)));
  let u_arr1_arr0 = fn_u_U_arr_arr_X_X(U_arr_arr_X_X(1, 0));
  let u_arr2_arrI = fn_u_U_arr_arr_X_X(U_arr_arr_X_X(2, u32(I)));
  let u_arrI_arr2 = fn_u_U_arr_arr_X_X(U_arr_arr_X_X(u32(I), 2));
  let u_arrI_arrJ = fn_u_U_arr_arr_X_X(U_arr_arr_X_X(u32(I), u32(J)));
  let s = fn_s_S();
  let s_str = fn_s_S_str_i();
  let s_arr0 = fn_s_S_arr_X(S_arr_X(0));
  let s_arr1 = fn_s_S_arr_X(S_arr_X(1));
  let s_arrI = fn_s_S_arr_X(S_arr_X(u32(I)));
  let s_arr1_arr0 = fn_s_S_arr_arr_X_X(S_arr_arr_X_X(1, 0));
  let s_arr2_arrI = fn_s_S_arr_arr_X_X(S_arr_arr_X_X(2, u32(I)));
  let s_arrI_arr2 = fn_s_S_arr_arr_X_X(S_arr_arr_X_X(u32(I), 2));
  let s_arrI_arrJ = fn_s_S_arr_arr_X_X(S_arr_arr_X_X(u32(I), u32(J)));
  let w = fn_w_W();
  let w_str = fn_w_W_str_i();
  let w_arr0 = fn_w_W_arr_X(W_arr_X(0));
  let w_arr1 = fn_w_W_arr_X(W_arr_X(1));
  let w_arrI = fn_w_W_arr_X(W_arr_X(u32(I)));
  let w_arr1_arr0 = fn_w_W_arr_arr_X_X(W_arr_arr_X_X(1, 0));
  let w_arr2_arrI = fn_w_W_arr_arr_X_X(W_arr_arr_X_X(2, u32(I)));
  let w_arrI_arr2 = fn_w_W_arr_arr_X_X(W_arr_arr_X_X(u32(I), 2));
  let w_arrI_arrJ = fn_w_W_arr_arr_X_X(W_arr_arr_X_X(u32(I), u32(J)));
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessComplexTest, Indexing) {
    auto* src = R"(
@group(0) @binding(0) var<storage> S : array<array<array<array<i32, 9>, 9>, 9>, 50>;

fn a(i : i32) -> i32 { return i; }

fn b(p : ptr<storage, array<array<array<i32, 9>, 9>, 9>>) -> i32 {
  return (*p) [ a( (*p)[0][1][2]    )]
              [ a( (*p)[a(3)][4][5] )]
              [ a( (*p)[6][a(7)][8] )];
}

fn c() {
  let v = b(&S[42]);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<storage> S : array<array<array<array<i32, 9>, 9>, 9>, 50>;

fn a(i : i32) -> i32 {
  return i;
}

alias S_X = array<u32, 1u>;

fn b_S_X(p : S_X) -> i32 {
  return S[p[0]][a(S[p[0]][0][1][2])][a(S[p[0]][a(3)][4][5])][a(S[p[0]][6][a(7)][8])];
}

fn c() {
  let v = b_S_X(S_X(42));
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessComplexTest, IndexingInPtrCall) {
    auto* src = R"(
@group(0) @binding(0) var<storage> S : array<array<array<array<i32, 9>, 9>, 9>, 50>;

fn a(pre : i32, i : ptr<storage, i32>, post : i32) -> i32 {
  return *i;
}

fn b(p : ptr<storage, array<array<array<i32, 9>, 9>, 9>>) -> i32 {
  return a(10, &(*p)[ a( 20, &(*p)[0][1][2], 30 )]
                    [ a( 40, &(*p)[3][4][5], 50 )]
                    [ a( 60, &(*p)[6][7][8], 70 )], 80);
}

fn c() {
  let v = b(&S[42]);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<storage> S : array<array<array<array<i32, 9>, 9>, 9>, 50>;

alias S_X_X_X_X = array<u32, 4u>;

fn a_S_X_X_X_X(pre : i32, i : S_X_X_X_X, post : i32) -> i32 {
  return S[i[0]][i[1]][i[2]][i[3]];
}

alias S_X = array<u32, 1u>;

fn b_S_X(p : S_X) -> i32 {
  return a_S_X_X_X_X(10, S_X_X_X_X(p[0u], u32(a_S_X_X_X_X(20, S_X_X_X_X(p[0u], 0, 1, 2), 30)), u32(a_S_X_X_X_X(40, S_X_X_X_X(p[0u], 3, 4, 5), 50)), u32(a_S_X_X_X_X(60, S_X_X_X_X(p[0u], 6, 7, 8), 70))), 80);
}

fn c() {
  let v = b_S_X(S_X(42));
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DirectVariableAccessComplexTest, IndexingDualPointers) {
    auto* src = R"(
@group(0) @binding(0) var<storage> S : array<array<array<i32, 9>, 9>, 50>;
@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 9>, 9>, 50>;

fn a(i : i32) -> i32 { return i; }

fn b(s : ptr<storage, array<array<i32, 9>, 9>>,
     u : ptr<uniform, array<array<vec4<i32>, 9>, 9>>) -> i32 {
  return (*s) [ a( (*u)[0][1].x    )]
              [ a( (*u)[a(3)][4].y )];
}

fn c() {
  let v = b(&S[42], &U[24]);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<storage> S : array<array<array<i32, 9>, 9>, 50>;

@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 9>, 9>, 50>;

fn a(i : i32) -> i32 {
  return i;
}

alias S_X = array<u32, 1u>;

alias U_X = array<u32, 1u>;

fn b_S_X_U_X(s : S_X, u : U_X) -> i32 {
  return S[s[0]][a(U[u[0]][0][1].x)][a(U[u[0]][a(3)][4].y)];
}

fn c() {
  let v = b_S_X_U_X(S_X(42), U_X(24));
}
)";

    auto got = Run<DirectVariableAccess>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace complex_tests

}  // namespace
}  // namespace tint::ast::transform
