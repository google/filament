// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/ast_lower/fold_trivial_lets.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::spirv::reader {
namespace {

using FoldTrivialLetsTest = ast::transform::TransformTest;

TEST_F(FoldTrivialLetsTest, Fold_IdentInitializer_AssignRHS) {
    auto* src = R"(
fn f() {
  var v = 42;
  let x = v;
  v = (x + 1);
}
)";

    auto* expect = R"(
fn f() {
  var v = 42;
  v = (v + 1);
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_IdentInitializer_IfCondition) {
    auto* src = R"(
fn f() {
  var v = 42;
  let x = v;
  if (x > 0) {
    v = 0;
  }
}
)";

    auto* expect = R"(
fn f() {
  var v = 42;
  if ((v > 0)) {
    v = 0;
  }
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, NoFold_IdentInitializer_StoreBeforeUse) {
    auto* src = R"(
fn f() {
  var v = 42;
  let x = v;
  v = 0;
  v = (x + 1);
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, NoFold_IdentInitializer_SideEffectsInUseExpression) {
    auto* src = R"(
var<private> v = 42;

fn g() -> i32 {
  v = 0;
  return 1;
}

fn f() {
  let x = v;
  v = (g() + x);
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_IdentInitializer_MultiUse) {
    auto* src = R"(
fn f() {
  var v = 42;
  let x = v;
  v = (x + x);
}
)";

    auto* expect = R"(
fn f() {
  var v = 42;
  v = (v + v);
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_IdentInitializer_MultiUse_OnlySomeInlineable) {
    auto* src = R"(
fn f() {
  var v = 42;
  let x = v;
  v = (x + x);
  if (x > 0) {
    v = 0;
  }
}
)";

    auto* expect = R"(
fn f() {
  var v = 42;
  let x = v;
  v = (v + v);
  if ((x > 0)) {
    v = 0;
  }
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_ComplexInitializer_SingleUse) {
    auto* src = R"(
fn f(idx : i32) {
  var v = array<vec4i, 4>();
  let x = v[idx].y;
  v[0].x = (x + 1);
}
)";

    auto* expect = R"(
fn f(idx : i32) {
  var v = array<vec4i, 4>();
  v[0].x = (v[idx].y + 1);
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, NoFold_ComplexInitializer_SingleUseWithSideEffects) {
    auto* src = R"(
var<private> i = 0;

fn bar() -> i32 {
  i++;
  return i;
}

fn f() -> i32 {
  var v = array<vec4i, 4>();
  let x = v[bar()].y;
  return (i + x);
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, NoFold_ComplexInitializer_MultiUse) {
    auto* src = R"(
fn f(idx : i32) {
  var v = array<vec4i, 4>();
  let x = v[idx].y;
  v[0].x = (x + x);
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_ComplexInitializer_SingleUseViaSimpleLet) {
    auto* src = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let x = ((a * b) + c);
  let y = x;
  return y;
}
)";

    auto* expect = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let y = ((a * b) + c);
  return y;
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_ComplexInitializer_SingleUseViaSimpleLetUsedTwice) {
    auto* src = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let x = (a * b) + c;
  let y = x;
  let z = y + y;
  return z;
}
)";

    auto* expect = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let x = ((a * b) + c);
  let z = (x + x);
  return z;
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialLetsTest, Fold_ComplexInitializer_MultiUseUseDifferentLets) {
    auto* src = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let x = (a * b) + c;
  let y = x;
  let z = x + y;
  return z;
}
)";

    auto* expect = R"(
fn f(a : i32, b : i32, c : i32) -> i32 {
  let x = ((a * b) + c);
  let z = (x + x);
  return z;
}
)";

    auto got = Run<FoldTrivialLets>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::spirv::reader
