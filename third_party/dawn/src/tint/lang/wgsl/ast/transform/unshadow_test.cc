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

#include "src/tint/lang/wgsl/ast/transform/unshadow.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using UnshadowTest = TransformTest;

TEST_F(UnshadowTest, EmptyModule) {
    auto* src = "";

    EXPECT_FALSE(ShouldRun<Unshadow>(src));
}

TEST_F(UnshadowTest, Noop) {
    auto* src = R"(
var<private> a : i32;

const b : i32 = 1;

fn F(c : i32) {
  var d : i32;
  let e : i32 = 1;
  const f : i32 = 2;
  {
    var g : i32;
    let h : i32 = 1;
    const i : i32 = 2;
  }
}
)";

    EXPECT_FALSE(ShouldRun<Unshadow>(src));
}

TEST_F(UnshadowTest, LocalShadowsAlias) {
    auto* src = R"(
alias a = i32;

fn X() {
  var a = false;
}

fn Y() {
  let a = true;
}

fn Z() {
  const a = true;
}
)";

    auto* expect = R"(
alias a = i32;

fn X() {
  var a_1 = false;
}

fn Y() {
  let a_2 = true;
}

fn Z() {
  const a_3 = true;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsAlias_OutOfOrder) {
    auto* src = R"(
fn X() {
  var a = false;
}

fn Y() {
  let a = true;
}

fn Z() {
  const a = true;
}

alias a = i32;
)";

    auto* expect = R"(
fn X() {
  var a_1 = false;
}

fn Y() {
  let a_2 = true;
}

fn Z() {
  const a_3 = true;
}

alias a = i32;
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsStruct) {
    auto* src = R"(
struct a {
  m : i32,
};

fn X() {
  var a = true;
}

fn Y() {
  let a = false;
}

fn Z() {
  const a = false;
}
)";

    auto* expect = R"(
struct a {
  m : i32,
}

fn X() {
  var a_1 = true;
}

fn Y() {
  let a_2 = false;
}

fn Z() {
  const a_3 = false;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsStruct_OutOfOrder) {
    auto* src = R"(
fn X() {
  var a = true;
}

fn Y() {
  let a = false;
}

fn Z() {
  const a = false;
}

struct a {
  m : i32,
};

)";

    auto* expect = R"(
fn X() {
  var a_1 = true;
}

fn Y() {
  let a_2 = false;
}

fn Z() {
  const a_3 = false;
}

struct a {
  m : i32,
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsFunction) {
    auto* src = R"(
fn a() {
  var a = true;
  var b = false;
  var c = true;
}

fn b() {
  let a = true;
  let b = false;
  let c = true;
}

fn c() {
  const a = true;
  const b = false;
  const c = true;
}
)";

    auto* expect = R"(
fn a() {
  var a_1 = true;
  var b_1 = false;
  var c_1 = true;
}

fn b() {
  let a_2 = true;
  let b_2 = false;
  let c_2 = true;
}

fn c() {
  const a_3 = true;
  const b_3 = false;
  const c_3 = true;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsFunction_OutOfOrder) {
    auto* src = R"(
fn b() {
  let a = true;
  let b = false;
  let c = true;
}

fn a() {
  var a = true;
  var b = false;
  var c = true;
}

fn c() {
  const a = true;
  const b = false;
  const c = true;
}
)";

    auto* expect = R"(
fn b() {
  let a_1 = true;
  let b_1 = false;
  let c_1 = true;
}

fn a() {
  var a_2 = true;
  var b_2 = false;
  var c_2 = true;
}

fn c() {
  const a_3 = true;
  const b_3 = false;
  const c_3 = true;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsGlobalVar) {
    auto* src = R"(
var<private> a : i32;

fn X() {
  var a = (a == 123);
}

fn Y() {
  let a = (a == 321);
}

fn Z() {
  const a = 321;
}
)";

    auto* expect = R"(
var<private> a : i32;

fn X() {
  var a_1 = (a == 123);
}

fn Y() {
  let a_2 = (a == 321);
}

fn Z() {
  const a_3 = 321;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsGlobalVar_OutOfOrder) {
    auto* src = R"(
fn X() {
  var a = (a == 123);
}

fn Y() {
  let a = (a == 321);
}

fn Z() {
  const a = 321;
}

var<private> a : i32;
)";

    auto* expect = R"(
fn X() {
  var a_1 = (a == 123);
}

fn Y() {
  let a_2 = (a == 321);
}

fn Z() {
  const a_3 = 321;
}

var<private> a : i32;
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsGlobalConst) {
    auto* src = R"(
const a : i32 = 1;

fn X() {
  var a = (a == 123);
}

fn Y() {
  let a = (a == 321);
}

fn Z() {
  const a = 321;
}
)";

    auto* expect = R"(
const a : i32 = 1;

fn X() {
  var a_1 = (a == 123);
}

fn Y() {
  let a_2 = (a == 321);
}

fn Z() {
  const a_3 = 321;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsGlobalConst_OutOfOrder) {
    auto* src = R"(
fn X() {
  var a = (a == 123);
}

fn Y() {
  let a = (a == 321);
}

fn Z() {
  const a = a;
}

const a : i32 = 1;
)";

    auto* expect = R"(
fn X() {
  var a_1 = (a == 123);
}

fn Y() {
  let a_2 = (a == 321);
}

fn Z() {
  const a_3 = a;
}

const a : i32 = 1;
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsLocalVar) {
    auto* src = R"(
fn X() {
  var a : i32;
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
  {
    const a = 321;
  }
}
)";

    auto* expect = R"(
fn X() {
  var a : i32;
  {
    var a_1 = (a == 123);
  }
  {
    let a_2 = (a == 321);
  }
  {
    const a_3 = 321;
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsLocalLet) {
    auto* src = R"(
fn X() {
  let a = 1;
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
  {
    const a = 321;
  }
}
)";

    auto* expect = R"(
fn X() {
  let a = 1;
  {
    var a_1 = (a == 123);
  }
  {
    let a_2 = (a == 321);
  }
  {
    const a_3 = 321;
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsLocalConst) {
    auto* src = R"(
fn X() {
  const a = 1;
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
  {
    const a = a;
  }
}
)";

    auto* expect = R"(
fn X() {
  const a = 1;
  {
    var a_1 = (a == 123);
  }
  {
    let a_2 = (a == 321);
  }
  {
    const a_3 = a;
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, LocalShadowsParam) {
    auto* src = R"(
fn F(a : i32) {
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
  {
    const a = 321;
  }
}
)";

    auto* expect = R"(
fn F(a : i32) {
  {
    var a_1 = (a == 123);
  }
  {
    let a_2 = (a == 321);
  }
  {
    const a_3 = 321;
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsFunction) {
    auto* src = R"(
fn a(a : i32) {
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
  {
    const a = 321;
  }
}
)";

    auto* expect = R"(
fn a(a_1 : i32) {
  {
    var a_2 = (a_1 == 123);
  }
  {
    let a_3 = (a_1 == 321);
  }
  {
    const a_4 = 321;
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsGlobalVar) {
    auto* src = R"(
var<private> a : i32;

fn F(a : bool) {
}
)";

    auto* expect = R"(
var<private> a : i32;

fn F(a_1 : bool) {
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsGlobalConst) {
    auto* src = R"(
const a : i32 = 1;

fn F(a : bool) {
}
)";

    auto* expect = R"(
const a : i32 = 1;

fn F(a_1 : bool) {
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsGlobalConst_OutOfOrder) {
    auto* src = R"(
fn F(a : bool) {
}

const a : i32 = 1;
)";

    auto* expect = R"(
fn F(a_1 : bool) {
}

const a : i32 = 1;
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsAlias) {
    auto* src = R"(
alias a = i32;

fn F(a : a) {
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
}
)";

    auto* expect = R"(
alias a = i32;

fn F(a_1 : a) {
  {
    var a_2 = (a_1 == 123);
  }
  {
    let a_3 = (a_1 == 321);
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, ParamShadowsAlias_OutOfOrder) {
    auto* src = R"(
fn F(a : a) {
  {
    var a = (a == 123);
  }
  {
    let a = (a == 321);
  }
}

alias a = i32;
)";

    auto* expect = R"(
fn F(a_1 : a) {
  {
    var a_2 = (a_1 == 123);
  }
  {
    let a_3 = (a_1 == 321);
  }
}

alias a = i32;
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, RenamedVarHasUsers) {
    auto* src = R"(
fn F() {
  var a : bool;
  {
    var a : i32;
    var b = a + 1;
  }
}
)";

    auto* expect = R"(
fn F() {
  var a : bool;
  {
    var a_1 : i32;
    var b = (a_1 + 1);
  }
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(UnshadowTest, RenamedAbstractConstHasUsers) {
    auto* src = R"(
fn v() {
  const v = 1;
  let x = v;
}
)";

    auto* expect = R"(
fn v() {
  const v_1 = 1;
  let x = v_1;
}
)";

    auto got = Run<Unshadow>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
