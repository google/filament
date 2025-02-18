// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/robustness.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {

static std::ostream& operator<<(std::ostream& out, Robustness::Action action) {
    switch (action) {
        case Robustness::Action::kIgnore:
            return out << "ignore";
        case Robustness::Action::kClamp:
            return out << "clamp";
        case Robustness::Action::kPredicate:
            return out << "predicate";
    }
    return out << "unknown";
}

namespace {

DataMap Config(Robustness::Action action, bool disable_runtime_sized_array_index_clamping = false) {
    Robustness::Config cfg;
    cfg.value_action = action;
    cfg.texture_action = action;
    cfg.function_action = action;
    cfg.private_action = action;
    cfg.push_constant_action = action;
    cfg.storage_action = action;
    cfg.uniform_action = action;
    cfg.workgroup_action = action;
    cfg.disable_runtime_sized_array_index_clamping = disable_runtime_sized_array_index_clamping;
    DataMap data;
    data.Add<Robustness::Config>(cfg);
    return data;
}

const char* Expect(Robustness::Action action,
                   const char* expect_ignore,
                   const char* expect_clamp,
                   const char* expect_predicate) {
    switch (action) {
        case Robustness::Action::kIgnore:
            return expect_ignore;
        case Robustness::Action::kClamp:
            return expect_clamp;
        case Robustness::Action::kPredicate:
            return expect_predicate;
    }
    return "<invalid action>";
}

using RobustnessTest = TransformTestWithParam<Robustness::Action>;

////////////////////////////////////////////////////////////////////////////////
// Constant sized array
////////////////////////////////////////////////////////////////////////////////

TEST_P(RobustnessTest, Read_ConstantSizedArrayVal_IndexWithLiteral) {
    auto* src = R"(
fn f() {
  var b : f32 = array<f32, 3>()[1i];
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayVal_IndexWithConst) {
    auto* src = R"(
const c : u32 = 1u;

fn f() {
  let b : f32 = array<f32, 3>()[c];
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayVal_IndexWithLet) {
    auto* src = R"(
fn f() {
  let l : u32 = 1u;
  let b : f32 = array<f32, 3>()[l];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
fn f() {
  let l : u32 = 1u;
  let b : f32 = array<f32, 3>()[min(l, 2u)];
}
)",
                          /* predicate */ R"(
fn f() {
  let l : u32 = 1u;
  let index = l;
  let predicate = (u32(index) <= 2u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = array<f32, 3>()[index];
  }
  let b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayVal_IndexWithRuntimeArrayIndex) {
    auto* src = R"(
var<private> i : u32;

fn f() {
  let a = array<f32, 3>();
  let b = array<i32, 5>();
  var c : f32 = a[b[i]];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> i : u32;

fn f() {
  let a = array<f32, 3>();
  let b = array<i32, 5>();
  var c : f32 = a[min(u32(b[min(i, 4u)]), 2u)];
}
)",
                          /* predicate */ R"(
var<private> i : u32;

fn f() {
  let a = array<f32, 3>();
  let b = array<i32, 5>();
  let index = i;
  let predicate = (u32(index) <= 4u);
  var predicated_expr : i32;
  if (predicate) {
    predicated_expr = b[index];
  }
  let index_1 = predicated_expr;
  let predicate_1 = (u32(index_1) <= 2u);
  var predicated_expr_1 : f32;
  if (predicate_1) {
    predicated_expr_1 = a[index_1];
  }
  var c : f32 = predicated_expr_1;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayVal_IndexWithRuntimeExpression) {
    auto* src = R"(
var<private> c : i32;

fn f() {
  var b : f32 = array<f32, 3>()[((c + 2) - 3)];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> c : i32;

fn f() {
  var b : f32 = array<f32, 3>()[min(u32(((c + 2) - 3)), 2u)];
}
)",
                          /* predicate */ R"(
var<private> c : i32;

fn f() {
  let index = ((c + 2) - 3);
  let predicate = (u32(index) <= 2u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = array<f32, 3>()[index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_NestedConstantSizedArraysVal_IndexWithRuntimeExpressions) {
    auto* src = R"(
var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  let a = array<array<array<f32, 1>, 2>, 3>();
  var r = a[x][y][z];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  let a = array<array<array<f32, 1>, 2>, 3>();
  var r = a[min(u32(x), 2u)][min(u32(y), 1u)][min(u32(z), 0u)];
}
)",
                          /* predicate */ R"(
var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  let a = array<array<array<f32, 1>, 2>, 3>();
  let index = x;
  let predicate = (u32(index) <= 2u);
  var predicated_expr : array<array<f32, 1u>, 2u>;
  if (predicate) {
    predicated_expr = a[index];
  }
  let index_1 = y;
  let predicate_1 = (u32(index_1) <= 1u);
  var predicated_expr_1 : array<f32, 1u>;
  if (predicate_1) {
    predicated_expr_1 = predicated_expr[index_1];
  }
  let index_2 = z;
  let predicate_2 = (u32(index_2) <= 0u);
  var predicated_expr_2 : f32;
  if (predicate_2) {
    predicated_expr_2 = predicated_expr_1[index_2];
  }
  var r = predicated_expr_2;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayVal_IndexWithOverride) {
    auto* src = R"(
@id(1300) override idx : i32;

fn f() {
  let a = array<f32, 4>();
  var b : f32 = a[idx];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@id(1300) override idx : i32;

fn f() {
  let a = array<f32, 4>();
  var b : f32 = a[min(u32(idx), 3u)];
}
)",
                          /* predicate */ R"(
@id(1300) override idx : i32;

fn f() {
  let a = array<f32, 4>();
  let index = idx;
  let predicate = (u32(index) <= 3u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a[index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));
    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayRef_IndexWithLiteral) {
    auto* src = R"(
var<private> a : array<f32, 3>;

fn f() {
  var b : f32 = a[1i];
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayRef_IndexWithConst) {
    auto* src = R"(
var<private> a : array<f32, 3>;

const c : u32 = 1u;

fn f() {
  let b : f32 = a[c];
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayRef_IndexWithLet) {
    auto* src = R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let b : f32 = a[l];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let b : f32 = a[min(l, 2u)];
}
)",
                          /* predicate */ R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let index = l;
  let predicate = (u32(index) <= 2u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a[index];
  }
  let b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayRef_IndexWithRuntimeArrayIndex) {
    auto* src = R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  var c : f32 = a[b[i]];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  var c : f32 = a[min(u32(b[min(i, 4u)]), 2u)];
}
)",
                          /* predicate */ R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  let index = i;
  let predicate = (u32(index) <= 4u);
  var predicated_expr : i32;
  if (predicate) {
    predicated_expr = b[index];
  }
  let index_1 = predicated_expr;
  let predicate_1 = (u32(index_1) <= 2u);
  var predicated_expr_1 : f32;
  if (predicate_1) {
    predicated_expr_1 = a[index_1];
  }
  var c : f32 = predicated_expr_1;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayRef_IndexWithRuntimeArrayIndexViaPointerIndex) {
    auto* src = R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  let p1 = &(a);
  let p2 = &(b);
  var c : f32 = p1[p2[i]];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  let p1 = &(a);
  let p2 = &(b);
  var c : f32 = p1[min(u32(p2[min(i, 4u)]), 2u)];
}
)",
                          /* predicate */ R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  let p1 = &(a);
  let p2 = &(b);
  let index = i;
  let predicate = (u32(index) <= 4u);
  var predicated_expr : i32;
  if (predicate) {
    predicated_expr = p2[index];
  }
  let index_1 = predicated_expr;
  let predicate_1 = (u32(index_1) <= 2u);
  var predicated_expr_1 : f32;
  if (predicate_1) {
    predicated_expr_1 = p1[index_1];
  }
  var c : f32 = predicated_expr_1;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayRef_IndexWithRuntimeExpression) {
    auto* src = R"(
var<private> a : array<f32, 3>;

var<private> c : i32;

fn f() {
  var b : f32 = a[((c + 2) - 3)];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<f32, 3>;

var<private> c : i32;

fn f() {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)];
}
)",
                          /* predicate */ R"(
var<private> a : array<f32, 3>;

var<private> c : i32;

fn f() {
  let index = ((c + 2) - 3);
  let predicate = (u32(index) <= 2u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a[index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_NestedConstantSizedArraysRef_IndexWithRuntimeExpressions) {
    auto* src = R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  var r = a[x][y][z];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  var r = a[min(u32(x), 2u)][min(u32(y), 1u)][min(u32(z), 0u)];
}
)",
                          /* predicate */ R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  let index = x;
  let predicate = (u32(index) <= 2u);
  let index_1 = y;
  let predicate_1 = (predicate & (u32(index_1) <= 1u));
  let index_2 = z;
  let predicate_2 = (predicate_1 & (u32(index_2) <= 0u));
  var predicated_expr : f32;
  if (predicate_2) {
    predicated_expr = a[index][index_1][index_2];
  }
  var r = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayRef_IndexWithOverride) {
    auto* src = R"(
@id(1300) override idx : i32;

fn f() {
  var a : array<f32, 4>;
  var b : f32 = a[idx];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@id(1300) override idx : i32;

fn f() {
  var a : array<f32, 4>;
  var b : f32 = a[min(u32(idx), 3u)];
}
)",
                          /* predicate */ R"(
@id(1300) override idx : i32;

fn f() {
  var a : array<f32, 4>;
  let index = idx;
  let predicate = (u32(index) <= 3u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a[index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));
    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayPtr_IndexWithLet) {
    auto* src = R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let p = &(a[l]);
  let f : f32 = *(p);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let p = &(a[min(l, 2u)]);
  let f : f32 = *(p);
}
)",
                          /* predicate */ R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let index = l;
  let predicate = (u32(index) <= 2u);
  let p = &(a[index]);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = *(p);
  }
  let f : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_ConstantSizedArrayPtr_IndexWithRuntimeArrayIndex) {
    auto* src = R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  let pa = &(a);
  let pb = &(b);
  let p0 = &((*(pb))[i]);
  let p1 = &(a[*(p0)]);
  var x : f32 = *(p1);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  let pa = &(a);
  let pb = &(b);
  let p0 = &((*(pb))[min(i, 4u)]);
  let p1 = &(a[min(u32(*(p0)), 2u)]);
  var x : f32 = *(p1);
}
)",
                          /* predicate */ R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  let pa = &(a);
  let pb = &(b);
  let index = i;
  let predicate = (u32(index) <= 4u);
  let p0 = &((*(pb))[index]);
  var predicated_expr : i32;
  if (predicate) {
    predicated_expr = *(p0);
  }
  let index_1 = predicated_expr;
  let predicate_1 = (u32(index_1) <= 2u);
  let p1 = &(a[index_1]);
  var predicated_expr_1 : f32;
  if (predicate_1) {
    predicated_expr_1 = *(p1);
  }
  var x : f32 = predicated_expr_1;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_NestedConstantSizedArraysPtr_IndexWithRuntimeExpressions) {
    auto* src = R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  let p0 = &(a);
  let p1 = &((*(p0))[x]);
  let p2 = &((*(p1))[y]);
  let p3 = &((*(p2))[z]);
  var r = *(p3);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  let p0 = &(a);
  let p1 = &((*(p0))[min(u32(x), 2u)]);
  let p2 = &((*(p1))[min(u32(y), 1u)]);
  let p3 = &((*(p2))[min(u32(z), 0u)]);
  var r = *(p3);
}
)",
                          /* predicate */ R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  let p0 = &(a);
  let index = x;
  let predicate = (u32(index) <= 2u);
  let p1 = &((*(p0))[index]);
  let index_1 = y;
  let predicate_1 = (predicate & (u32(index_1) <= 1u));
  let p2 = &((*(p1))[index_1]);
  let index_2 = z;
  let predicate_2 = (predicate_1 & (u32(index_2) <= 0u));
  let p3 = &((*(p2))[index_2]);
  var predicated_expr : f32;
  if (predicate_2) {
    predicated_expr = *(p3);
  }
  var r = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_NestedConstantSizedArrays_MixedAccess) {
    auto* src = R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

const y = 1;

override z : i32;

fn f() {
  let p0 = &(a);
  let p1 = &((*(p0))[x]);
  let p2 = &((*(p1))[y]);
  let p3 = &((*(p2))[z]);
  var r = *(p3);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

const y = 1;

override z : i32;

fn f() {
  let p0 = &(a);
  let p1 = &((*(p0))[min(u32(x), 2u)]);
  let p2 = &((*(p1))[y]);
  let p3 = &((*(p2))[min(u32(z), 0u)]);
  var r = *(p3);
}
)",
                          /* predicate */ R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

const y = 1;

override z : i32;

fn f() {
  let p0 = &(a);
  let index = x;
  let predicate = (u32(index) <= 2u);
  let p1 = &((*(p0))[index]);
  let p2 = &((*(p1))[y]);
  let index_1 = z;
  let predicate_1 = (predicate & (u32(index_1) <= 0u));
  let p3 = &((*(p2))[index_1]);
  var predicated_expr : f32;
  if (predicate_1) {
    predicated_expr = *(p3);
  }
  var r = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Assign_ConstantSizedArray_IndexWithLet) {
    auto* src = R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  a[l] = 42.0f;
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  a[min(l, 2u)] = 42.0f;
}
)",
                          /* predicate */ R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let index = l;
  let predicate = (u32(index) <= 2u);
  if (predicate) {
    a[index] = 42.0f;
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Assign_ConstantSizedArrayPtr_IndexWithLet) {
    auto* src = R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let p = &(a[l]);
  *(p) = 42.0f;
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let p = &(a[min(l, 2u)]);
  *(p) = 42.0f;
}
)",
                          /* predicate */ R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let index = l;
  let predicate = (u32(index) <= 2u);
  let p = &(a[index]);
  if (predicate) {
    *(p) = 42.0f;
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Assign_ConstantSizedArrayPtr_IndexWithRuntimeArrayIndex) {
    auto* src = R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  let pa = &(a);
  let pb = &(b);
  let p0 = &((*(pb))[i]);
  let p1 = &(a[*(p0)]);
  *(p1) = 42.0f;
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  let pa = &(a);
  let pb = &(b);
  let p0 = &((*(pb))[min(i, 4u)]);
  let p1 = &(a[min(u32(*(p0)), 2u)]);
  *(p1) = 42.0f;
}
)",
                          /* predicate */ R"(
var<private> a : array<f32, 3>;

var<private> b : array<i32, 5>;

var<private> i : u32;

fn f() {
  let pa = &(a);
  let pb = &(b);
  let index = i;
  let predicate = (u32(index) <= 4u);
  let p0 = &((*(pb))[index]);
  var predicated_expr : i32;
  if (predicate) {
    predicated_expr = *(p0);
  }
  let index_1 = predicated_expr;
  let predicate_1 = (u32(index_1) <= 2u);
  let p1 = &(a[index_1]);
  if (predicate_1) {
    *(p1) = 42.0f;
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Assign_NestedConstantSizedArraysPtr_IndexWithRuntimeExpressions) {
    auto* src = R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  let p0 = &(a);
  let p1 = &((*(p0))[x]);
  let p2 = &((*(p1))[y]);
  let p3 = &((*(p2))[z]);
  *(p3) = 42.0f;
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  let p0 = &(a);
  let p1 = &((*(p0))[min(u32(x), 2u)]);
  let p2 = &((*(p1))[min(u32(y), 1u)]);
  let p3 = &((*(p2))[min(u32(z), 0u)]);
  *(p3) = 42.0f;
}
)",
                          /* predicate */ R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

var<private> y : i32;

var<private> z : i32;

fn f() {
  let p0 = &(a);
  let index = x;
  let predicate = (u32(index) <= 2u);
  let p1 = &((*(p0))[index]);
  let index_1 = y;
  let predicate_1 = (predicate & (u32(index_1) <= 1u));
  let p2 = &((*(p1))[index_1]);
  let index_2 = z;
  let predicate_2 = (predicate_1 & (u32(index_2) <= 0u));
  let p3 = &((*(p2))[index_2]);
  if (predicate_2) {
    *(p3) = 42.0f;
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Assign_NestedConstantSizedArrays_MixedAccess) {
    auto* src = R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

const y = 1;

override z : i32;

fn f() {
  let p0 = &(a);
  let p1 = &((*(p0))[x]);
  let p2 = &((*(p1))[y]);
  let p3 = &((*(p2))[z]);
  *(p3) = 42.0f;
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

const y = 1;

override z : i32;

fn f() {
  let p0 = &(a);
  let p1 = &((*(p0))[min(u32(x), 2u)]);
  let p2 = &((*(p1))[y]);
  let p3 = &((*(p2))[min(u32(z), 0u)]);
  *(p3) = 42.0f;
}
)",
                          /* predicate */ R"(
var<private> a : array<array<array<f32, 1>, 2>, 3>;

var<private> x : i32;

const y = 1;

override z : i32;

fn f() {
  let p0 = &(a);
  let index = x;
  let predicate = (u32(index) <= 2u);
  let p1 = &((*(p0))[index]);
  let p2 = &((*(p1))[y]);
  let index_1 = z;
  let predicate_1 = (predicate & (u32(index_1) <= 0u));
  let p3 = &((*(p2))[index_1]);
  if (predicate_1) {
    *(p3) = 42.0f;
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, CompoundAssign_ConstantSizedArray_IndexWithLet) {
    auto* src = R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  a[l] += 42.0f;
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  a[min(l, 2u)] += 42.0f;
}
)",
                          /* predicate */ R"(
var<private> a : array<f32, 3>;

fn f() {
  let l : u32 = 1u;
  let index = l;
  let predicate = (u32(index) <= 2u);
  if (predicate) {
    a[index] += 42.0f;
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Increment_ConstantSizedArray_IndexWithLet) {
    auto* src = R"(
var<private> a : array<i32, 3>;

fn f() {
  let l : u32 = 1u;
  a[l]++;
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<i32, 3>;

fn f() {
  let l : u32 = 1u;
  a[min(l, 2u)]++;
}
)",
                          /* predicate */ R"(
var<private> a : array<i32, 3>;

fn f() {
  let l : u32 = 1u;
  let index = l;
  let predicate = (u32(index) <= 2u);
  if (predicate) {
    a[index]++;
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// Runtime sized array
////////////////////////////////////////////////////////////////////////////////

TEST_P(RobustnessTest, Read_RuntimeArray_IndexWithLiteral) {
    auto* src = R"(
struct S {
  a : f32,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

fn f() {
  var d : f32 = s.b[25];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
struct S {
  a : f32,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

fn f() {
  var d : f32 = s.b[min(u32(25), (arrayLength(&(s.b)) - 1u))];
}
)",
                          /* predicate */ R"(
struct S {
  a : f32,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

fn f() {
  let index = 25;
  let predicate = (u32(index) <= (arrayLength(&(s.b)) - 1u));
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = s.b[index];
  }
  var d : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// Vector
////////////////////////////////////////////////////////////////////////////////

TEST_P(RobustnessTest, Read_Vector_IndexWithLiteral) {
    auto* src = R"(
var<private> a : vec3<f32>;

fn f() {
  var b : f32 = a[1i];
}
)";

    auto* expect = src;
    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_Vector_IndexWithConst) {
    auto* src = R"(
var<private> a : vec3<f32>;

fn f() {
  const i = 1;
  var b : f32 = a[i];
}
)";

    auto* expect = src;
    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_Vector_IndexWithLet) {
    auto* src = R"(
fn f() {
  let i = 99;
  let v = vec4<f32>()[i];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
fn f() {
  let i = 99;
  let v = vec4<f32>()[min(u32(i), 3u)];
}
)",
                          /* predicate */ R"(
fn f() {
  let i = 99;
  let index = i;
  let predicate = (u32(index) <= 3u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = vec4<f32>()[index];
  }
  let v = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_Vector_IndexWithRuntimeExpression) {
    auto* src = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[((c + 2) - 3)];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)];
}
)",
                          /* predicate */ R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  let index = ((c + 2) - 3);
  let predicate = (u32(index) <= 2u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a[index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_Vector_IndexWithRuntimeExpression_ViaPointerIndex) {
    auto* src = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  let p = &(a);
  var b : f32 = p[((c + 2) - 3)];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  let p = &(a);
  var b : f32 = p[min(u32(((c + 2) - 3)), 2u)];
}
)",
                          /* predicate */ R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  let p = &(a);
  let index = ((c + 2) - 3);
  let predicate = (u32(index) <= 2u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = p[index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_Vector_SwizzleIndexWithGlobalVar) {
    auto* src = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a.xy[c];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a.xy[min(u32(c), 1u)];
}
)",
                          /* predicate */ R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  let index = c;
  let predicate = (u32(index) <= 1u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a.xy[index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_Vector_SwizzleIndexWithRuntimeExpression) {
    auto* src = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a.xy[((c + 2) - 3)];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a.xy[min(u32(((c + 2) - 3)), 1u)];
}
)",
                          /* predicate */ R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  let index = ((c + 2) - 3);
  let predicate = (u32(index) <= 1u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a.xy[index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_Vector_SwizzleIndexWithRuntimeExpression_ViaPointerDot) {
    auto* src = R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  let p = &(a);
  var b : f32 = p.xy[((c + 2) - 3)];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  let p = &(a);
  var b : f32 = p.xy[min(u32(((c + 2) - 3)), 1u)];
}
)",
                          /* predicate */ R"(
var<private> a : vec3<f32>;

var<private> c : i32;

fn f() {
  let p = &(a);
  let index = ((c + 2) - 3);
  let predicate = (u32(index) <= 1u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = p.xy[index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_Vector_IndexWithOverride) {
    auto* src = R"(
@id(1300) override idx : i32;

fn f() {
  var a : vec3<f32>;
  var b : f32 = a[idx];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@id(1300) override idx : i32;

fn f() {
  var a : vec3<f32>;
  var b : f32 = a[min(u32(idx), 2u)];
}
)",
                          /* predicate */ R"(
@id(1300) override idx : i32;

fn f() {
  var a : vec3<f32>;
  let index = idx;
  let predicate = (u32(index) <= 2u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a[index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));
    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// Matrix
////////////////////////////////////////////////////////////////////////////////

TEST_P(RobustnessTest, Read_MatrixRef_IndexingWithLiterals) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

fn f() {
  var b : f32 = a[2i][1i];
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_MatrixRef_IndexWithRuntimeExpressionThenLiteral) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[((c + 2) - 3)][1];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : mat3x2<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[min(u32(((c + 2) - 3)), 2u)][1];
}
)",
                          /* predicate */ R"(
var<private> a : mat3x2<f32>;

var<private> c : i32;

fn f() {
  let index = ((c + 2) - 3);
  let predicate = (u32(index) <= 2u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a[index][1];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_MatrixRef_IndexWithLiteralThenRuntimeExpression) {
    auto* src = R"(
var<private> a : mat3x2<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[1][((c + 2) - 3)];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : mat3x2<f32>;

var<private> c : i32;

fn f() {
  var b : f32 = a[1][min(u32(((c + 2) - 3)), 1u)];
}
)",
                          /* predicate */ R"(
var<private> a : mat3x2<f32>;

var<private> c : i32;

fn f() {
  let index = ((c + 2) - 3);
  let predicate = (u32(index) <= 1u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a[1][index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_MatrixRef_IndexWithOverrideThenLiteral) {
    auto* src = R"(
@id(1300) override idx : i32;

fn f() {
  var a : mat3x2<f32>;
  var b : f32 = a[idx][1];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@id(1300) override idx : i32;

fn f() {
  var a : mat3x2<f32>;
  var b : f32 = a[min(u32(idx), 2u)][1];
}
)",
                          /* predicate */ R"(
@id(1300) override idx : i32;

fn f() {
  var a : mat3x2<f32>;
  let index = idx;
  let predicate = (u32(index) <= 2u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a[index][1];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));
    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_MatrixRef_IndexWithLetThenSwizzle) {
    auto* src = R"(
fn f() {
  let i = 1;
  var m = mat3x2<f32>();
  var v = m[i].yx;
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
fn f() {
  let i = 1;
  var m = mat3x2<f32>();
  var v = m[min(u32(i), 2u)].yx;
}
)",
                          /* predicate */ R"(
fn f() {
  let i = 1;
  var m = mat3x2<f32>();
  let index = i;
  let predicate = (u32(index) <= 2u);
  var predicated_expr : vec2<f32>;
  if (predicate) {
    predicated_expr = m[index];
  }
  var v = predicated_expr.yx;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));
    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_MatrixRef_IndexWithLiteralThenOverride) {
    auto* src = R"(
@id(1300) override idx : i32;

fn f() {
  var a : mat3x2<f32>;
  var b : f32 = a[1][idx];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@id(1300) override idx : i32;

fn f() {
  var a : mat3x2<f32>;
  var b : f32 = a[1][min(u32(idx), 1u)];
}
)",
                          /* predicate */ R"(
@id(1300) override idx : i32;

fn f() {
  var a : mat3x2<f32>;
  let index = idx;
  let predicate = (u32(index) <= 1u);
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = a[1][index];
  }
  var b : f32 = predicated_expr;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));
    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Assign_Matrix_IndexWithLet) {
    auto* src = R"(
var<private> m : mat3x4f;

fn f() {
  let c = 1;
  m[c] = vec4f(1);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> m : mat3x4f;

fn f() {
  let c = 1;
  m[min(u32(c), 2u)] = vec4f(1);
}
)",
                          /* predicate */ R"(
var<private> m : mat3x4f;

fn f() {
  let c = 1;
  let index = c;
  let predicate = (u32(index) <= 2u);
  if (predicate) {
    m[index] = vec4f(1);
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, CompoundAssign_Matrix_IndexWithLet) {
    auto* src = R"(
var<private> m : mat3x4f;

fn f() {
  let c = 1;
  m[c] += vec4f(1);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> m : mat3x4f;

fn f() {
  let c = 1;
  m[min(u32(c), 2u)] += vec4f(1);
}
)",
                          /* predicate */ R"(
var<private> m : mat3x4f;

fn f() {
  let c = 1;
  let index = c;
  let predicate = (u32(index) <= 2u);
  if (predicate) {
    m[index] += vec4f(1);
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// Texture builtin calls.
////////////////////////////////////////////////////////////////////////////////

TEST_P(RobustnessTest, TextureDimensions) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn dimensions() {
  let l = textureDimensions(t);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureDimensions_Level) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn dimensions_signed(level : i32) {
  let l = textureDimensions(t, level);
}

fn dimensions_unsigned(level : u32) {
  let l = textureDimensions(t, level);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  dimensions_signed(i32(non_uniform.x));
  dimensions_unsigned(u32(non_uniform.x));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn dimensions_signed(level : i32) {
  let level_idx = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureDimensions(t, level_idx);
}

fn dimensions_unsigned(level : u32) {
  let level_idx_1 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureDimensions(t, level_idx_1);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  dimensions_signed(i32(non_uniform.x));
  dimensions_unsigned(u32(non_uniform.x));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn dimensions_signed(level : i32) {
  let level_idx = u32(level);
  let num_levels = textureNumLevels(t);
  var predicated_value : vec2<u32>;
  if ((level_idx < num_levels)) {
    predicated_value = textureDimensions(t, level_idx);
  }
  let l = predicated_value;
}

fn dimensions_unsigned(level : u32) {
  let level_idx_1 = u32(level);
  let num_levels_1 = textureNumLevels(t);
  var predicated_value_1 : vec2<u32>;
  if ((level_idx_1 < num_levels_1)) {
    predicated_value_1 = textureDimensions(t, level_idx_1);
  }
  let l = predicated_value_1;
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  dimensions_signed(i32(non_uniform.x));
  dimensions_unsigned(u32(non_uniform.x));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureGather) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn gather(coords : vec2f) {
  let l = textureGather(0, t, s, coords);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureGather_Array) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d_array<f32>;

@group(0) @binding(1) var s : sampler;

fn gather_signed(coords : vec2f, array : i32) {
  let l = textureGather(1, t, s, coords, array);
}

fn gather_unsigned(coords : vec2f, array : u32) {
  let l = textureGather(1, t, s, coords, array);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  gather_signed(non_uniform.xy, i32(non_uniform.x));
  gather_unsigned(non_uniform.xy, u32(non_uniform.x));
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureGatherCompare) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d;

@group(0) @binding(1) var s : sampler_comparison;

fn gather(coords : vec2f, depth_ref : f32) {
  let l = textureGatherCompare(t, s, coords, depth_ref);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureGatherCompare_Array) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d_array;

@group(0) @binding(1) var s : sampler_comparison;

fn gather_signed(coords : vec2f, array : i32, depth_ref : f32) {
  let l = textureGatherCompare(t, s, coords, array, depth_ref);
}

fn gather_unsigned(coords : vec2f, array : u32, depth_ref : f32) {
  let l = textureGatherCompare(t, s, coords, array, depth_ref);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  gather_signed(non_uniform.xy, i32(non_uniform.x), non_uniform.x);
  gather_unsigned(non_uniform.xy, u32(non_uniform.x), non_uniform.x);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_1D) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_1d<f32>;

fn load_signed(coords : i32, level : i32) {
  let l = textureLoad(t, coords, level);
}

fn load_unsigned(coords : u32, level : u32) {
  let l = textureLoad(t, coords, level);
}

fn load_mixed(coords : i32, level : u32) {
  let l = textureLoad(t, coords, level);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(i32(non_uniform.x), i32(non_uniform.x));
  load_unsigned(u32(non_uniform.x), u32(non_uniform.x));
  load_mixed(i32(non_uniform.x), u32(non_uniform.x));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_1d<f32>;

fn load_signed(coords : i32, level : i32) {
  let level_idx = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, clamp(coords, 0, i32((textureDimensions(t, level_idx) - 1))), level_idx);
}

fn load_unsigned(coords : u32, level : u32) {
  let level_idx_1 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, min(coords, (textureDimensions(t, level_idx_1) - 1)), level_idx_1);
}

fn load_mixed(coords : i32, level : u32) {
  let level_idx_2 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, clamp(coords, 0, i32((textureDimensions(t, level_idx_2) - 1))), level_idx_2);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(i32(non_uniform.x), i32(non_uniform.x));
  load_unsigned(u32(non_uniform.x), u32(non_uniform.x));
  load_mixed(i32(non_uniform.x), u32(non_uniform.x));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_1d<f32>;

fn load_signed(coords : i32, level : i32) {
  let level_idx = u32(level);
  let num_levels = textureNumLevels(t);
  let coords_1 = u32(coords);
  var predicated_value : vec4<f32>;
  if (((level_idx < num_levels) & all((coords_1 < textureDimensions(t, min(level_idx, (num_levels - 1))))))) {
    predicated_value = textureLoad(t, coords_1, level_idx);
  }
  let l = predicated_value;
}

fn load_unsigned(coords : u32, level : u32) {
  let level_idx_1 = u32(level);
  let num_levels_1 = textureNumLevels(t);
  let coords_2 = u32(coords);
  var predicated_value_1 : vec4<f32>;
  if (((level_idx_1 < num_levels_1) & all((coords_2 < textureDimensions(t, min(level_idx_1, (num_levels_1 - 1))))))) {
    predicated_value_1 = textureLoad(t, coords_2, level_idx_1);
  }
  let l = predicated_value_1;
}

fn load_mixed(coords : i32, level : u32) {
  let level_idx_2 = u32(level);
  let num_levels_2 = textureNumLevels(t);
  let coords_3 = u32(coords);
  var predicated_value_2 : vec4<f32>;
  if (((level_idx_2 < num_levels_2) & all((coords_3 < textureDimensions(t, min(level_idx_2, (num_levels_2 - 1))))))) {
    predicated_value_2 = textureLoad(t, coords_3, level_idx_2);
  }
  let l = predicated_value_2;
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(i32(non_uniform.x), i32(non_uniform.x));
  load_unsigned(u32(non_uniform.x), u32(non_uniform.x));
  load_mixed(i32(non_uniform.x), u32(non_uniform.x));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_2D) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn load_signed(coords : vec2i, level : i32) {
  let l = textureLoad(t, coords, level);
}

fn load_unsigned(coords : vec2u, level : u32) {
  let l = textureLoad(t, coords, level);
}

fn load_mixed(coords : vec2u, level : i32) {
  let l = textureLoad(t, coords, level);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x));
  load_mixed(vec2u(non_uniform.xy), i32(non_uniform.x));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn load_signed(coords : vec2i, level : i32) {
  let level_idx = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, clamp(coords, vec2(0), vec2<i32>((textureDimensions(t, level_idx) - vec2(1)))), level_idx);
}

fn load_unsigned(coords : vec2u, level : u32) {
  let level_idx_1 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, min(coords, (textureDimensions(t, level_idx_1) - vec2(1))), level_idx_1);
}

fn load_mixed(coords : vec2u, level : i32) {
  let level_idx_2 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, min(coords, (textureDimensions(t, level_idx_2) - vec2(1))), level_idx_2);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x));
  load_mixed(vec2u(non_uniform.xy), i32(non_uniform.x));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn load_signed(coords : vec2i, level : i32) {
  let level_idx = u32(level);
  let num_levels = textureNumLevels(t);
  let coords_1 = vec2<u32>(coords);
  var predicated_value : vec4<f32>;
  if (((level_idx < num_levels) & all((coords_1 < textureDimensions(t, min(level_idx, (num_levels - 1))))))) {
    predicated_value = textureLoad(t, coords_1, level_idx);
  }
  let l = predicated_value;
}

fn load_unsigned(coords : vec2u, level : u32) {
  let level_idx_1 = u32(level);
  let num_levels_1 = textureNumLevels(t);
  let coords_2 = vec2<u32>(coords);
  var predicated_value_1 : vec4<f32>;
  if (((level_idx_1 < num_levels_1) & all((coords_2 < textureDimensions(t, min(level_idx_1, (num_levels_1 - 1))))))) {
    predicated_value_1 = textureLoad(t, coords_2, level_idx_1);
  }
  let l = predicated_value_1;
}

fn load_mixed(coords : vec2u, level : i32) {
  let level_idx_2 = u32(level);
  let num_levels_2 = textureNumLevels(t);
  let coords_3 = vec2<u32>(coords);
  var predicated_value_2 : vec4<f32>;
  if (((level_idx_2 < num_levels_2) & all((coords_3 < textureDimensions(t, min(level_idx_2, (num_levels_2 - 1))))))) {
    predicated_value_2 = textureLoad(t, coords_3, level_idx_2);
  }
  let l = predicated_value_2;
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x));
  load_mixed(vec2u(non_uniform.xy), i32(non_uniform.x));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_2DArray) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d_array<f32>;

fn load_signed(coords : vec2i, array : i32, level : i32) {
  let l = textureLoad(t, coords, array, level);
}

fn load_unsigned(coords : vec2u, array : u32, level : u32) {
  let l = textureLoad(t, coords, array, level);
}

fn load_mixed(coords : vec2u, array : i32, level : u32) {
  let l = textureLoad(t, coords, array, level);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x), u32(non_uniform.x));
  load_mixed(vec2u(non_uniform.xy), i32(non_uniform.x), u32(non_uniform.x));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_2d_array<f32>;

fn load_signed(coords : vec2i, array : i32, level : i32) {
  let level_idx = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, clamp(coords, vec2(0), vec2<i32>((textureDimensions(t, level_idx) - vec2(1)))), clamp(array, 0, i32((textureNumLayers(t) - 1))), level_idx);
}

fn load_unsigned(coords : vec2u, array : u32, level : u32) {
  let level_idx_1 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, min(coords, (textureDimensions(t, level_idx_1) - vec2(1))), min(array, (textureNumLayers(t) - 1)), level_idx_1);
}

fn load_mixed(coords : vec2u, array : i32, level : u32) {
  let level_idx_2 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, min(coords, (textureDimensions(t, level_idx_2) - vec2(1))), clamp(array, 0, i32((textureNumLayers(t) - 1))), level_idx_2);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x), u32(non_uniform.x));
  load_mixed(vec2u(non_uniform.xy), i32(non_uniform.x), u32(non_uniform.x));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_2d_array<f32>;

fn load_signed(coords : vec2i, array : i32, level : i32) {
  let level_idx = u32(level);
  let num_levels = textureNumLevels(t);
  let coords_1 = vec2<u32>(coords);
  let array_idx = u32(array);
  var predicated_value : vec4<f32>;
  if ((((level_idx < num_levels) & all((coords_1 < textureDimensions(t, min(level_idx, (num_levels - 1)))))) & (array_idx < textureNumLayers(t)))) {
    predicated_value = textureLoad(t, coords_1, array_idx, level_idx);
  }
  let l = predicated_value;
}

fn load_unsigned(coords : vec2u, array : u32, level : u32) {
  let level_idx_1 = u32(level);
  let num_levels_1 = textureNumLevels(t);
  let coords_2 = vec2<u32>(coords);
  let array_idx_1 = u32(array);
  var predicated_value_1 : vec4<f32>;
  if ((((level_idx_1 < num_levels_1) & all((coords_2 < textureDimensions(t, min(level_idx_1, (num_levels_1 - 1)))))) & (array_idx_1 < textureNumLayers(t)))) {
    predicated_value_1 = textureLoad(t, coords_2, array_idx_1, level_idx_1);
  }
  let l = predicated_value_1;
}

fn load_mixed(coords : vec2u, array : i32, level : u32) {
  let level_idx_2 = u32(level);
  let num_levels_2 = textureNumLevels(t);
  let coords_3 = vec2<u32>(coords);
  let array_idx_2 = u32(array);
  var predicated_value_2 : vec4<f32>;
  if ((((level_idx_2 < num_levels_2) & all((coords_3 < textureDimensions(t, min(level_idx_2, (num_levels_2 - 1)))))) & (array_idx_2 < textureNumLayers(t)))) {
    predicated_value_2 = textureLoad(t, coords_3, array_idx_2, level_idx_2);
  }
  let l = predicated_value_2;
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x), u32(non_uniform.x));
  load_mixed(vec2u(non_uniform.xy), i32(non_uniform.x), u32(non_uniform.x));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_3D) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_3d<f32>;

fn load_signed(coords : vec3i, level : i32) {
  let l = textureLoad(t, coords, level);
}

fn load_unsigned(coords : vec3u, level : u32) {
  let l = textureLoad(t, coords, level);
}

fn load_mixed(coords : vec3u, level : i32) {
  let l = textureLoad(t, coords, level);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec3i(non_uniform.xyz), i32(non_uniform.x));
  load_unsigned(vec3u(non_uniform.xyz), u32(non_uniform.x));
  load_mixed(vec3u(non_uniform.xyz), i32(non_uniform.x));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_3d<f32>;

fn load_signed(coords : vec3i, level : i32) {
  let level_idx = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, clamp(coords, vec3(0), vec3<i32>((textureDimensions(t, level_idx) - vec3(1)))), level_idx);
}

fn load_unsigned(coords : vec3u, level : u32) {
  let level_idx_1 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, min(coords, (textureDimensions(t, level_idx_1) - vec3(1))), level_idx_1);
}

fn load_mixed(coords : vec3u, level : i32) {
  let level_idx_2 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, min(coords, (textureDimensions(t, level_idx_2) - vec3(1))), level_idx_2);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec3i(non_uniform.xyz), i32(non_uniform.x));
  load_unsigned(vec3u(non_uniform.xyz), u32(non_uniform.x));
  load_mixed(vec3u(non_uniform.xyz), i32(non_uniform.x));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_3d<f32>;

fn load_signed(coords : vec3i, level : i32) {
  let level_idx = u32(level);
  let num_levels = textureNumLevels(t);
  let coords_1 = vec3<u32>(coords);
  var predicated_value : vec4<f32>;
  if (((level_idx < num_levels) & all((coords_1 < textureDimensions(t, min(level_idx, (num_levels - 1))))))) {
    predicated_value = textureLoad(t, coords_1, level_idx);
  }
  let l = predicated_value;
}

fn load_unsigned(coords : vec3u, level : u32) {
  let level_idx_1 = u32(level);
  let num_levels_1 = textureNumLevels(t);
  let coords_2 = vec3<u32>(coords);
  var predicated_value_1 : vec4<f32>;
  if (((level_idx_1 < num_levels_1) & all((coords_2 < textureDimensions(t, min(level_idx_1, (num_levels_1 - 1))))))) {
    predicated_value_1 = textureLoad(t, coords_2, level_idx_1);
  }
  let l = predicated_value_1;
}

fn load_mixed(coords : vec3u, level : i32) {
  let level_idx_2 = u32(level);
  let num_levels_2 = textureNumLevels(t);
  let coords_3 = vec3<u32>(coords);
  var predicated_value_2 : vec4<f32>;
  if (((level_idx_2 < num_levels_2) & all((coords_3 < textureDimensions(t, min(level_idx_2, (num_levels_2 - 1))))))) {
    predicated_value_2 = textureLoad(t, coords_3, level_idx_2);
  }
  let l = predicated_value_2;
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec3i(non_uniform.xyz), i32(non_uniform.x));
  load_unsigned(vec3u(non_uniform.xyz), u32(non_uniform.x));
  load_mixed(vec3u(non_uniform.xyz), i32(non_uniform.x));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_Multisampled2D) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_multisampled_2d<f32>;

fn load_signed(coords : vec2i, sample : i32) {
  let l = textureLoad(t, coords, sample);
}

fn load_unsigned(coords : vec2u, sample : u32) {
  let l = textureLoad(t, coords, sample);
}

fn load_mixed(coords : vec2i, sample : u32) {
  let l = textureLoad(t, coords, sample);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x));
  load_mixed(vec2i(non_uniform.xy), u32(non_uniform.x));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_multisampled_2d<f32>;

fn load_signed(coords : vec2i, sample : i32) {
  let l = textureLoad(t, clamp(coords, vec2(0), vec2<i32>((textureDimensions(t) - vec2(1)))), sample);
}

fn load_unsigned(coords : vec2u, sample : u32) {
  let l = textureLoad(t, min(coords, (textureDimensions(t) - vec2(1))), sample);
}

fn load_mixed(coords : vec2i, sample : u32) {
  let l = textureLoad(t, clamp(coords, vec2(0), vec2<i32>((textureDimensions(t) - vec2(1)))), sample);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x));
  load_mixed(vec2i(non_uniform.xy), u32(non_uniform.x));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_multisampled_2d<f32>;

fn load_signed(coords : vec2i, sample : i32) {
  let coords_1 = vec2<u32>(coords);
  var predicated_value : vec4<f32>;
  if (all((coords_1 < textureDimensions(t)))) {
    predicated_value = textureLoad(t, coords_1, sample);
  }
  let l = predicated_value;
}

fn load_unsigned(coords : vec2u, sample : u32) {
  let coords_2 = vec2<u32>(coords);
  var predicated_value_1 : vec4<f32>;
  if (all((coords_2 < textureDimensions(t)))) {
    predicated_value_1 = textureLoad(t, coords_2, sample);
  }
  let l = predicated_value_1;
}

fn load_mixed(coords : vec2i, sample : u32) {
  let coords_3 = vec2<u32>(coords);
  var predicated_value_2 : vec4<f32>;
  if (all((coords_3 < textureDimensions(t)))) {
    predicated_value_2 = textureLoad(t, coords_3, sample);
  }
  let l = predicated_value_2;
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x));
  load_mixed(vec2i(non_uniform.xy), u32(non_uniform.x));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_Depth2D) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d;

fn load_signed(coords : vec2i, level : i32) {
  let l = textureLoad(t, coords, level);
}

fn load_unsigned(coords : vec2u, level : u32) {
  let l = textureLoad(t, coords, level);
}

fn load_mixed(coords : vec2i, level : u32) {
  let l = textureLoad(t, coords, level);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x));
  load_mixed(vec2i(non_uniform.xy), u32(non_uniform.x));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_depth_2d;

fn load_signed(coords : vec2i, level : i32) {
  let level_idx = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, clamp(coords, vec2(0), vec2<i32>((textureDimensions(t, level_idx) - vec2(1)))), level_idx);
}

fn load_unsigned(coords : vec2u, level : u32) {
  let level_idx_1 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, min(coords, (textureDimensions(t, level_idx_1) - vec2(1))), level_idx_1);
}

fn load_mixed(coords : vec2i, level : u32) {
  let level_idx_2 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, clamp(coords, vec2(0), vec2<i32>((textureDimensions(t, level_idx_2) - vec2(1)))), level_idx_2);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x));
  load_mixed(vec2i(non_uniform.xy), u32(non_uniform.x));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_depth_2d;

fn load_signed(coords : vec2i, level : i32) {
  let level_idx = u32(level);
  let num_levels = textureNumLevels(t);
  let coords_1 = vec2<u32>(coords);
  var predicated_value : f32;
  if (((level_idx < num_levels) & all((coords_1 < textureDimensions(t, min(level_idx, (num_levels - 1))))))) {
    predicated_value = textureLoad(t, coords_1, level_idx);
  }
  let l = predicated_value;
}

fn load_unsigned(coords : vec2u, level : u32) {
  let level_idx_1 = u32(level);
  let num_levels_1 = textureNumLevels(t);
  let coords_2 = vec2<u32>(coords);
  var predicated_value_1 : f32;
  if (((level_idx_1 < num_levels_1) & all((coords_2 < textureDimensions(t, min(level_idx_1, (num_levels_1 - 1))))))) {
    predicated_value_1 = textureLoad(t, coords_2, level_idx_1);
  }
  let l = predicated_value_1;
}

fn load_mixed(coords : vec2i, level : u32) {
  let level_idx_2 = u32(level);
  let num_levels_2 = textureNumLevels(t);
  let coords_3 = vec2<u32>(coords);
  var predicated_value_2 : f32;
  if (((level_idx_2 < num_levels_2) & all((coords_3 < textureDimensions(t, min(level_idx_2, (num_levels_2 - 1))))))) {
    predicated_value_2 = textureLoad(t, coords_3, level_idx_2);
  }
  let l = predicated_value_2;
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x));
  load_mixed(vec2i(non_uniform.xy), u32(non_uniform.x));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_Depth2DArray) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d_array;

fn load_signed(coords : vec2i, array : i32, level : i32) {
  let l = textureLoad(t, coords, array, level);
}

fn load_unsigned(coords : vec2u, array : u32, level : u32) {
  let l = textureLoad(t, coords, array, level);
}

fn load_mixed(coords : vec2u, array : i32, level : u32) {
  let l = textureLoad(t, coords, array, level);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x), u32(non_uniform.x));
  load_mixed(vec2u(non_uniform.xy), i32(non_uniform.x), u32(non_uniform.x));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_depth_2d_array;

fn load_signed(coords : vec2i, array : i32, level : i32) {
  let level_idx = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, clamp(coords, vec2(0), vec2<i32>((textureDimensions(t, level_idx) - vec2(1)))), clamp(array, 0, i32((textureNumLayers(t) - 1))), level_idx);
}

fn load_unsigned(coords : vec2u, array : u32, level : u32) {
  let level_idx_1 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, min(coords, (textureDimensions(t, level_idx_1) - vec2(1))), min(array, (textureNumLayers(t) - 1)), level_idx_1);
}

fn load_mixed(coords : vec2u, array : i32, level : u32) {
  let level_idx_2 = min(u32(level), (textureNumLevels(t) - 1));
  let l = textureLoad(t, min(coords, (textureDimensions(t, level_idx_2) - vec2(1))), clamp(array, 0, i32((textureNumLayers(t) - 1))), level_idx_2);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x), u32(non_uniform.x));
  load_mixed(vec2u(non_uniform.xy), i32(non_uniform.x), u32(non_uniform.x));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_depth_2d_array;

fn load_signed(coords : vec2i, array : i32, level : i32) {
  let level_idx = u32(level);
  let num_levels = textureNumLevels(t);
  let coords_1 = vec2<u32>(coords);
  let array_idx = u32(array);
  var predicated_value : f32;
  if ((((level_idx < num_levels) & all((coords_1 < textureDimensions(t, min(level_idx, (num_levels - 1)))))) & (array_idx < textureNumLayers(t)))) {
    predicated_value = textureLoad(t, coords_1, array_idx, level_idx);
  }
  let l = predicated_value;
}

fn load_unsigned(coords : vec2u, array : u32, level : u32) {
  let level_idx_1 = u32(level);
  let num_levels_1 = textureNumLevels(t);
  let coords_2 = vec2<u32>(coords);
  let array_idx_1 = u32(array);
  var predicated_value_1 : f32;
  if ((((level_idx_1 < num_levels_1) & all((coords_2 < textureDimensions(t, min(level_idx_1, (num_levels_1 - 1)))))) & (array_idx_1 < textureNumLayers(t)))) {
    predicated_value_1 = textureLoad(t, coords_2, array_idx_1, level_idx_1);
  }
  let l = predicated_value_1;
}

fn load_mixed(coords : vec2u, array : i32, level : u32) {
  let level_idx_2 = u32(level);
  let num_levels_2 = textureNumLevels(t);
  let coords_3 = vec2<u32>(coords);
  let array_idx_2 = u32(array);
  var predicated_value_2 : f32;
  if ((((level_idx_2 < num_levels_2) & all((coords_3 < textureDimensions(t, min(level_idx_2, (num_levels_2 - 1)))))) & (array_idx_2 < textureNumLayers(t)))) {
    predicated_value_2 = textureLoad(t, coords_3, array_idx_2, level_idx_2);
  }
  let l = predicated_value_2;
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy), i32(non_uniform.x), i32(non_uniform.x));
  load_unsigned(vec2u(non_uniform.xy), u32(non_uniform.x), u32(non_uniform.x));
  load_mixed(vec2u(non_uniform.xy), i32(non_uniform.x), u32(non_uniform.x));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_External) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_external;

fn load_signed(coords : vec2i) {
  let l = textureLoad(t, coords);
}

fn load_unsigned(coords : vec2u) {
  let l = textureLoad(t, coords);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy));
  load_unsigned(vec2u(non_uniform.xy));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_external;

fn load_signed(coords : vec2i) {
  let l = textureLoad(t, clamp(coords, vec2(0), vec2<i32>((textureDimensions(t) - vec2(1)))));
}

fn load_unsigned(coords : vec2u) {
  let l = textureLoad(t, min(coords, (textureDimensions(t) - vec2(1))));
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy));
  load_unsigned(vec2u(non_uniform.xy));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_external;

fn load_signed(coords : vec2i) {
  let coords_1 = vec2<u32>(coords);
  var predicated_value : vec4<f32>;
  if (all((coords_1 < textureDimensions(t)))) {
    predicated_value = textureLoad(t, coords_1);
  }
  let l = predicated_value;
}

fn load_unsigned(coords : vec2u) {
  let coords_2 = vec2<u32>(coords);
  var predicated_value_1 : vec4<f32>;
  if (all((coords_2 < textureDimensions(t)))) {
    predicated_value_1 = textureLoad(t, coords_2);
  }
  let l = predicated_value_1;
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  load_signed(vec2i(non_uniform.xy));
  load_unsigned(vec2u(non_uniform.xy));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureNumLayers) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d_array;

fn num_layers(coords : vec2f, depth_ref : f32) {
  let l = textureNumLayers(t);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureNumLevels) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d;

fn num_levels(coords : vec2f, depth_ref : f32) {
  let l = textureNumLevels(t);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureNumSamples) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_multisampled_2d;

fn num_levels(coords : vec2f, depth_ref : f32) {
  let l = textureNumSamples(t);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSample) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn sample(coords : vec2f) {
  let l = textureSample(t, s, coords);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSample_Offset) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn sample(coords : vec2f) {
  const offset = vec2i(1);
  let l = textureSample(t, s, coords, offset);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSample_ArrayIndex) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d_array<f32>;

@group(0) @binding(1) var s : sampler;

fn sample_signed(coords : vec2f, array : i32) {
  let l = textureSample(t, s, coords, array);
}

fn sample_unsigned(coords : vec2f, array : u32) {
  let l = textureSample(t, s, coords, array);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  sample_signed(non_uniform.xy, i32(non_uniform.x));
  sample_unsigned(non_uniform.xy, u32(non_uniform.x));
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleBias) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn sample_bias(coords : vec2f, bias : f32) {
  let l = textureSampleBias(t, s, coords, bias);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleBias_Offset) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn sample_bias(coords : vec2f, bias : f32) {
  const offset = vec2i(1);
  let l = textureSampleBias(t, s, coords, bias, offset);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleBias_ArrayIndex) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d_array<f32>;

@group(0) @binding(1) var s : sampler;

fn sample_bias_signed(coords : vec2f, array : i32, bias : f32) {
  let l = textureSampleBias(t, s, coords, array, bias);
}

fn sample_bias_unsigned(coords : vec2f, array : u32, bias : f32) {
  let l = textureSampleBias(t, s, coords, array, bias);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  sample_bias_signed(non_uniform.xy, i32(non_uniform.x), non_uniform.x);
  sample_bias_unsigned(non_uniform.xy, u32(non_uniform.x), non_uniform.x);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleCompare) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d;

@group(0) @binding(1) var s : sampler_comparison;

fn sample_compare(coords : vec2f, depth_ref : f32) {
  let l = textureSampleCompare(t, s, coords, depth_ref);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleCompare_Offset) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d;

@group(0) @binding(1) var s : sampler_comparison;

fn sample_compare(coords : vec2f, depth_ref : f32) {
  const offset = vec2i(1);
  let l = textureSampleCompare(t, s, coords, depth_ref, offset);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleCompare_ArrayIndex) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d_array;

@group(0) @binding(1) var s : sampler_comparison;

fn sample_compare_signed(coords : vec2f, array : i32, depth_ref : f32) {
  let l = textureSampleCompare(t, s, coords, array, depth_ref);
}

fn sample_compare_unsigned(coords : vec2f, array : u32, depth_ref : f32) {
  let l = textureSampleCompare(t, s, coords, array, depth_ref);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  sample_compare_signed(non_uniform.xy, i32(non_uniform.x), non_uniform.x);
  sample_compare_unsigned(non_uniform.xy, u32(non_uniform.x), non_uniform.x);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleCompareLevel) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d;

@group(0) @binding(1) var s : sampler_comparison;

fn sample_compare_level(coords : vec2f, depth_ref : f32) {
  let l = textureSampleCompareLevel(t, s, coords, depth_ref);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleCompareLevel_Offset) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d;

@group(0) @binding(1) var s : sampler_comparison;

fn sample_compare_level(coords : vec2f, depth_ref : f32) {
  const offset = vec2i(1);
  let l = textureSampleCompareLevel(t, s, coords, depth_ref, offset);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleCompareLevel_ArrayIndex) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_depth_2d_array;

@group(0) @binding(1) var s : sampler_comparison;

fn sample_compare_level_signed(coords : vec2f, array : i32, depth_ref : f32) {
  let l = textureSampleCompareLevel(t, s, coords, array, depth_ref);
}

fn sample_compare_level_unsigned(coords : vec2f, array : u32, depth_ref : f32) {
  let l = textureSampleCompareLevel(t, s, coords, array, depth_ref);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  sample_compare_level_signed(non_uniform.xy, i32(non_uniform.x), non_uniform.x);
  sample_compare_level_unsigned(non_uniform.xy, u32(non_uniform.x), non_uniform.x);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleGrad) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn sample_compare_level(coords : vec2f, ddx : vec2f, ddy : vec2f) {
  let l = textureSampleGrad(t, s, coords, ddx, ddy);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleGrad_Offset) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn sample_compare_level(coords : vec2f, ddx : vec2f, ddy : vec2f) {
  const offset = vec2i(1);
  let l = textureSampleGrad(t, s, coords, ddx, ddy, offset);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleGrad_ArrayIndex) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d_array<f32>;

@group(0) @binding(1) var s : sampler;

fn sample_grad_signed(coords : vec2f, array : i32, ddx : vec2f, ddy : vec2f) {
  let l = textureSampleGrad(t, s, coords, array, ddx, ddy);
}

fn sample_grad_unsigned(coords : vec2f, array : u32, ddx : vec2f, ddy : vec2f) {
  let l = textureSampleGrad(t, s, coords, array, ddx, ddy);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  sample_grad_signed(non_uniform.xy, i32(non_uniform.x), non_uniform.xy, non_uniform.xy);
  sample_grad_unsigned(non_uniform.xy, u32(non_uniform.x), non_uniform.xy, non_uniform.xy);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleLevel) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn sample_compare_level(coords : vec2f, level : f32) {
  let l = textureSampleLevel(t, s, coords, level);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleLevel_Offset) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn sample_compare_level(coords : vec2f, level : f32) {
  const offset = vec2i(1);
  let l = textureSampleLevel(t, s, coords, level, offset);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleLevel_ArrayIndex) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d_array<f32>;

@group(0) @binding(1) var s : sampler;

fn sample_compare_level_signed(coords : vec2f, array : i32, level : f32) {
  let l = textureSampleLevel(t, s, coords, array, level);
}

fn sample_compare_level_unsigned(coords : vec2f, array : u32, level : f32) {
  let l = textureSampleLevel(t, s, coords, array, level);
}

@fragment
fn main(@builtin(position) non_uniform : vec4f) {
  sample_compare_level_signed(non_uniform.xy, i32(non_uniform.x), non_uniform.x);
  sample_compare_level_unsigned(non_uniform.xy, u32(non_uniform.x), non_uniform.x);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureSampleBaseClampToEdge) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_external;

@group(0) @binding(1) var s : sampler;

fn sample_base_clamp_to_edge(coords : vec2f) {
  let l = textureSampleBaseClampToEdge(t, s, coords);
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// Other
////////////////////////////////////////////////////////////////////////////////
TEST_P(RobustnessTest, ShadowedVariable) {
    auto* src = R"(
fn f() {
  var a : array<f32, 3>;
  var i : u32;
  {
    var a : array<f32, 5>;
    var b : f32 = a[i];
  }
  var c : f32 = a[i];
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
fn f() {
  var a : array<f32, 3>;
  var i : u32;
  {
    var a : array<f32, 5>;
    var b : f32 = a[min(i, 4u)];
  }
  var c : f32 = a[min(i, 2u)];
}
)",
                          /* predicate */ R"(
fn f() {
  var a : array<f32, 3>;
  var i : u32;
  {
    var a : array<f32, 5>;
    let index = i;
    let predicate = (u32(index) <= 4u);
    var predicated_expr : f32;
    if (predicate) {
      predicated_expr = a[index];
    }
    var b : f32 = predicated_expr;
  }
  let index_1 = i;
  let predicate_1 = (u32(index_1) <= 2u);
  var predicated_expr_1 : f32;
  if (predicate_1) {
    predicated_expr_1 = a[index_1];
  }
  var c : f32 = predicated_expr_1;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));
    EXPECT_EQ(expect, str(got));
}

// Check that existing use of min() and arrayLength() do not get renamed.
TEST_P(RobustnessTest, DontRenameSymbols) {
    auto* src = R"(
struct S {
  a : f32,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

const c : u32 = 1u;

fn f() {
  let b : f32 = s.b[c];
  let x : i32 = min(1, 2);
  let y : u32 = arrayLength(&(s.b));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
struct S {
  a : f32,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

const c : u32 = 1u;

fn f() {
  let b : f32 = s.b[min(c, (arrayLength(&(s.b)) - 1u))];
  let x : i32 = min(1, 2);
  let y : u32 = arrayLength(&(s.b));
}
)",
                          /* predicate */ R"(
struct S {
  a : f32,
  b : array<f32>,
}

@group(0) @binding(0) var<storage, read> s : S;

const c : u32 = 1u;

fn f() {
  let index = c;
  let predicate = (u32(index) <= (arrayLength(&(s.b)) - 1u));
  var predicated_expr : f32;
  if (predicate) {
    predicated_expr = s.b[index];
  }
  let b : f32 = predicated_expr;
  let x : i32 = min(1, 2);
  let y : u32 = arrayLength(&(s.b));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, WorkgroupOverrideCount) {
    auto* src = R"(
override N = 123;

var<workgroup> w : array<f32, N>;

fn f() {
  var b : f32 = w[1i];
}
)";

    auto* expect =
        Expect(GetParam(),
               /* ignore */ src,
               /* clamp */
               R"(error: array size is an override-expression, when expected a constant-expression.
Was the SubstituteOverride transform run?)",
               /* predicate */
               R"(error: array size is an override-expression, when expected a constant-expression.
Was the SubstituteOverride transform run?)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// atomic predication
////////////////////////////////////////////////////////////////////////////////

TEST_P(RobustnessTest, AtomicLoad) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  let i = 0;
  let r = atomicLoad(&(a[i]));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  let i = 0;
  let r = atomicLoad(&(a[min(u32(i), 3u)]));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  var predicated_value : i32;
  if (predicate) {
    predicated_value = atomicLoad(&(a[index]));
  }
  let r = predicated_value;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, AtomicAnd) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  let i = 0;
  atomicAnd(&(a[i]), 10);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  let i = 0;
  atomicAnd(&(a[min(u32(i), 3u)]), 10);
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  if (predicate) {
    atomicAnd(&(a[index]), 10);
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// workgroupUniformLoad
////////////////////////////////////////////////////////////////////////////////

TEST_P(RobustnessTest, WorkgroupUniformLoad) {
    auto* src = R"(
var<workgroup> a : array<u32, 32>;

@compute @workgroup_size(1)
fn f() {
  let i = 1;
  let p = &(a[i]);
  let v = workgroupUniformLoad(p);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<workgroup> a : array<u32, 32>;

@compute @workgroup_size(1)
fn f() {
  let i = 1;
  let p = &(a[min(u32(i), 31u)]);
  let v = workgroupUniformLoad(p);
}
)",
                          /* predicate */ R"(
var<workgroup> a : array<u32, 32>;

@compute @workgroup_size(1)
fn f() {
  let i = 1;
  let index = i;
  let predicate = (u32(index) <= 31u);
  let p = &(a[index]);
  var predicated_value : u32;
  if (predicate) {
    predicated_value = workgroupUniformLoad(p);
  } else {
    workgroupBarrier();
  }
  let v = predicated_value;
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// Usage in loops
////////////////////////////////////////////////////////////////////////////////

TEST_P(RobustnessTest, ArrayVal_ForLoopInit) {
    auto* src = R"(
fn f() {
  let a = array<i32, 3>();
  var v = 1;
  for(var i = a[v]; (i < 3); i++) {
    var in_loop = 42;
  }
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
fn f() {
  let a = array<i32, 3>();
  var v = 1;
  for(var i = a[min(u32(v), 2u)]; (i < 3); i++) {
    var in_loop = 42;
  }
}
)",
                          /* predicate */ R"(
fn f() {
  let a = array<i32, 3>();
  var v = 1;
  {
    let index = v;
    let predicate = (u32(index) <= 2u);
    var predicated_expr : i32;
    if (predicate) {
      predicated_expr = a[index];
    }
    var i = predicated_expr;
    loop {
      if (!((i < 3))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        i++;
      }
    }
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, ArrayVal_ForLoopCond) {
    auto* src = R"(
fn f() {
  let a = array<i32, 3>();
  var v = 1;
  for(var i = 0; (i < a[v]); i++) {
    var in_loop = 42;
  }
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
fn f() {
  let a = array<i32, 3>();
  var v = 1;
  for(var i = 0; (i < a[min(u32(v), 2u)]); i++) {
    var in_loop = 42;
  }
}
)",
                          /* predicate */ R"(
fn f() {
  let a = array<i32, 3>();
  var v = 1;
  {
    var i = 0;
    loop {
      let index = v;
      let predicate = (u32(index) <= 2u);
      var predicated_expr : i32;
      if (predicate) {
        predicated_expr = a[index];
      }
      if (!((i < predicated_expr))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        i++;
      }
    }
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, ArrayVal_ForLoopCont) {
    auto* src = R"(
fn f() {
  let a = array<i32, 3>();
  for(var i = 0; (i < 5); i = a[i]) {
    var in_loop = 42;
  }
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
fn f() {
  let a = array<i32, 3>();
  for(var i = 0; (i < 5); i = a[min(u32(i), 2u)]) {
    var in_loop = 42;
  }
}
)",
                          /* predicate */ R"(
fn f() {
  let a = array<i32, 3>();
  {
    var i = 0;
    loop {
      if (!((i < 5))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        let index = i;
        let predicate = (u32(index) <= 2u);
        var predicated_expr : i32;
        if (predicate) {
          predicated_expr = a[index];
        }
        i = predicated_expr;
      }
    }
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_ForLoopInit) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<i32>;

fn f() {
  var coords = vec2(1);
  var level = 1;
  for(var i = textureLoad(t, coords, level).x; (i < 3); i++) {
    var in_loop = 42;
  }
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_2d<i32>;

fn f() {
  var coords = vec2(1);
  var level = 1;
  {
    let level_idx = min(u32(level), (textureNumLevels(t) - 1));
    var i = textureLoad(t, clamp(coords, vec2(0), vec2<i32>((textureDimensions(t, level_idx) - vec2(1)))), level_idx).x;
    loop {
      if (!((i < 3))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        i++;
      }
    }
  }
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_2d<i32>;

fn f() {
  var coords = vec2(1);
  var level = 1;
  {
    let level_idx = u32(level);
    let num_levels = textureNumLevels(t);
    let coords_1 = vec2<u32>(coords);
    var predicated_value : vec4<i32>;
    if (((level_idx < num_levels) & all((coords_1 < textureDimensions(t, min(level_idx, (num_levels - 1))))))) {
      predicated_value = textureLoad(t, coords_1, level_idx);
    }
    var i = predicated_value.x;
    loop {
      if (!((i < 3))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        i++;
      }
    }
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_ForLoopCond) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<i32>;

fn f() {
  var coords = vec2(1);
  var level = 1;
  for(var i = 0; (i < textureLoad(t, coords, level).x); i++) {
    var in_loop = 42;
  }
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_2d<i32>;

fn f() {
  var coords = vec2(1);
  var level = 1;
  {
    var i = 0;
    loop {
      let level_idx = min(u32(level), (textureNumLevels(t) - 1));
      if (!((i < textureLoad(t, clamp(coords, vec2(0), vec2<i32>((textureDimensions(t, level_idx) - vec2(1)))), level_idx).x))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        i++;
      }
    }
  }
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_2d<i32>;

fn f() {
  var coords = vec2(1);
  var level = 1;
  {
    var i = 0;
    loop {
      let level_idx = u32(level);
      let num_levels = textureNumLevels(t);
      let coords_1 = vec2<u32>(coords);
      var predicated_value : vec4<i32>;
      if (((level_idx < num_levels) & all((coords_1 < textureDimensions(t, min(level_idx, (num_levels - 1))))))) {
        predicated_value = textureLoad(t, coords_1, level_idx);
      }
      if (!((i < predicated_value.x))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        i++;
      }
    }
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, TextureLoad_ForLoopCont) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<i32>;

fn f() {
  var level = 1;
  for(var i = 0; (i < 5); i = textureLoad(t, vec2(i), i).x) {
    var in_loop = 42;
  }
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var t : texture_2d<i32>;

fn f() {
  var level = 1;
  {
    var i = 0;
    loop {
      if (!((i < 5))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        let level_idx = min(u32(i), (textureNumLevels(t) - 1));
        i = textureLoad(t, clamp(vec2(i), vec2(0), vec2<i32>((textureDimensions(t, level_idx) - vec2(1)))), level_idx).x;
      }
    }
  }
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var t : texture_2d<i32>;

fn f() {
  var level = 1;
  {
    var i = 0;
    loop {
      if (!((i < 5))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        let level_idx = u32(i);
        let num_levels = textureNumLevels(t);
        let coords = vec2<u32>(vec2(i));
        var predicated_value : vec4<i32>;
        if (((level_idx < num_levels) & all((coords < textureDimensions(t, min(level_idx, (num_levels - 1))))))) {
          predicated_value = textureLoad(t, coords, level_idx);
        }
        i = predicated_value.x;
      }
    }
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, AtomicXor_ForLoopInit) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  var i = 0;
  for(atomicXor(&(a[i]), 4); (i < 3); i++) {
    var in_loop = 42;
  }
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  var i = 0;
  for(atomicXor(&(a[min(u32(i), 3u)]), 4); (i < 3); i++) {
    var in_loop = 42;
  }
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  var i = 0;
  {
    let index = i;
    let predicate = (u32(index) <= 3u);
    if (predicate) {
      atomicXor(&(a[index]), 4);
    }
    loop {
      if (!((i < 3))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        i++;
      }
    }
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, AtomicXor_ForLoopCond) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  for(var i = 0; (i < atomicXor(&(a[i]), 4)); i++) {
    var in_loop = 42;
  }
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  for(var i = 0; (i < atomicXor(&(a[min(u32(i), 3u)]), 4)); i++) {
    var in_loop = 42;
  }
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  {
    var i = 0;
    loop {
      let index = i;
      let predicate = (u32(index) <= 3u);
      var predicated_value : i32;
      if (predicate) {
        predicated_value = atomicXor(&(a[index]), 4);
      }
      if (!((i < predicated_value))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        i++;
      }
    }
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, AtomicXor_ForLoopCont) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  for(var i = 0; (i < 5); i = atomicXor(&(a[i]), 4)) {
    var in_loop = 42;
  }
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  for(var i = 0; (i < 5); i = atomicXor(&(a[min(u32(i), 3u)]), 4)) {
    var in_loop = 42;
  }
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<atomic<i32>, 4>;

fn f() {
  {
    var i = 0;
    loop {
      if (!((i < 5))) {
        break;
      }
      {
        var in_loop = 42;
      }

      continuing {
        let index = i;
        let predicate = (u32(index) <= 3u);
        var predicated_value : i32;
        if (predicate) {
          predicated_value = atomicXor(&(a[index]), 4);
        }
        i = predicated_value;
      }
    }
  }
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// User pointer parameters
////////////////////////////////////////////////////////////////////////////////

TEST_P(RobustnessTest, Read_PrivatePointerParameter_IndexWithConstant) {
    auto* src = R"(
var<private> a : array<i32, 4>;

fn x(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return ((pre + *(p)) + post);
}

fn y(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return x(pre, p, post);
}

fn z() {
  y(1, &(a[1]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ src,
                          /* predicate */ R"(
var<private> a : array<i32, 4>;

fn x(pre : i32, p : ptr<private, i32>, p_predicate : bool, post : i32) -> i32 {
  var predicated_expr : i32;
  if (p_predicate) {
    predicated_expr = *(p);
  }
  return ((pre + predicated_expr) + post);
}

fn y(pre : i32, p : ptr<private, i32>, p_predicate_1 : bool, post : i32) -> i32 {
  return x(pre, p, p_predicate_1, post);
}

fn z() {
  y(1, &(a[1]), true, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_WorkgroupPointerParameter_IndexWithConstant) {
    auto* src = R"(
var<workgroup> a : array<i32, 4>;

fn x(pre : i32, p : ptr<workgroup, i32>, post : i32) -> i32 {
  return ((pre + *(p)) + post);
}

fn y(pre : i32, p : ptr<workgroup, i32>, post : i32) -> i32 {
  return x(pre, p, post);
}

fn z() {
  y(1, &(a[1]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ src,
                          /* predicate */ R"(
var<workgroup> a : array<i32, 4>;

fn x(pre : i32, p : ptr<workgroup, i32>, p_predicate : bool, post : i32) -> i32 {
  var predicated_expr : i32;
  if (p_predicate) {
    predicated_expr = *(p);
  }
  return ((pre + predicated_expr) + post);
}

fn y(pre : i32, p : ptr<workgroup, i32>, p_predicate_1 : bool, post : i32) -> i32 {
  return x(pre, p, p_predicate_1, post);
}

fn z() {
  y(1, &(a[1]), true, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_UniformPointerParameter_IndexWithConstant) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<vec4i, 4>;

fn x(pre : vec4i, p : ptr<uniform, vec4i>, post : vec4i) -> vec4i {
  return ((pre + *(p)) + post);
}

fn y(pre : vec4i, p : ptr<uniform, vec4i>, post : vec4i) -> vec4i {
  return x(pre, p, post);
}

fn z() {
  y(vec4(1), &(a[1]), vec4(2));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ src,
                          /* predicate */ R"(
@group(0) @binding(0) var<uniform> a : array<vec4i, 4>;

fn x(pre : vec4i, p : ptr<uniform, vec4i>, p_predicate : bool, post : vec4i) -> vec4i {
  var predicated_expr : vec4<i32>;
  if (p_predicate) {
    predicated_expr = *(p);
  }
  return ((pre + predicated_expr) + post);
}

fn y(pre : vec4i, p : ptr<uniform, vec4i>, p_predicate_1 : bool, post : vec4i) -> vec4i {
  return x(pre, p, p_predicate_1, post);
}

fn z() {
  y(vec4(1), &(a[1]), true, vec4(2));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_StoragePointerParameter_IndexWithConstant) {
    auto* src = R"(
@group(0) @binding(0) var<storage> a : array<vec4i, 4>;

fn x(pre : vec4i, p : ptr<storage, vec4i>, post : vec4i) -> vec4i {
  return ((pre + *(p)) + post);
}

fn y(pre : vec4i, p : ptr<storage, vec4i>, post : vec4i) -> vec4i {
  return x(pre, p, post);
}

fn z() {
  y(vec4(1), &(a[1]), vec4(2));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ src,
                          /* predicate */ R"(
@group(0) @binding(0) var<storage> a : array<vec4i, 4>;

fn x(pre : vec4i, p : ptr<storage, vec4i>, p_predicate : bool, post : vec4i) -> vec4i {
  var predicated_expr : vec4<i32>;
  if (p_predicate) {
    predicated_expr = *(p);
  }
  return ((pre + predicated_expr) + post);
}

fn y(pre : vec4i, p : ptr<storage, vec4i>, p_predicate_1 : bool, post : vec4i) -> vec4i {
  return x(pre, p, p_predicate_1, post);
}

fn z() {
  y(vec4(1), &(a[1]), true, vec4(2));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_FunctionPointerParameter_IndexWithConstant) {
    auto* src = R"(
fn x(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return ((pre + *(p)) + post);
}

fn y(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return x(pre, p, post);
}

fn z() {
  var a : array<i32, 4>;
  y(1, &(a[1]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ src,
                          /* predicate */ R"(
fn x(pre : i32, p : ptr<function, i32>, p_predicate : bool, post : i32) -> i32 {
  var predicated_expr : i32;
  if (p_predicate) {
    predicated_expr = *(p);
  }
  return ((pre + predicated_expr) + post);
}

fn y(pre : i32, p : ptr<function, i32>, p_predicate_1 : bool, post : i32) -> i32 {
  return x(pre, p, p_predicate_1, post);
}

fn z() {
  var a : array<i32, 4>;
  y(1, &(a[1]), true, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_PrivatePointerParameter_IndexWithLet) {
    auto* src = R"(
var<private> a : array<i32, 4>;

fn x(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return ((pre + *(p)) + post);
}

fn y(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return x(pre, p, post);
}

fn z() {
  let i = 0;
  y(1, &(a[i]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<i32, 4>;

fn x(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return ((pre + *(p)) + post);
}

fn y(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return x(pre, p, post);
}

fn z() {
  let i = 0;
  y(1, &(a[min(u32(i), 3u)]), 2);
}
)",
                          /* predicate */ R"(
var<private> a : array<i32, 4>;

fn x(pre : i32, p : ptr<private, i32>, p_predicate : bool, post : i32) -> i32 {
  var predicated_expr : i32;
  if (p_predicate) {
    predicated_expr = *(p);
  }
  return ((pre + predicated_expr) + post);
}

fn y(pre : i32, p : ptr<private, i32>, p_predicate_1 : bool, post : i32) -> i32 {
  return x(pre, p, p_predicate_1, post);
}

fn z() {
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  y(1, &(a[index]), predicate, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_WorkgroupPointerParameter_IndexWithLet) {
    auto* src = R"(
var<workgroup> a : array<i32, 4>;

fn x(pre : i32, p : ptr<workgroup, i32>, post : i32) -> i32 {
  return ((pre + *(p)) + post);
}

fn y(pre : i32, p : ptr<workgroup, i32>, post : i32) -> i32 {
  return x(pre, p, post);
}

fn z() {
  let i = 0;
  y(1, &(a[i]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<workgroup> a : array<i32, 4>;

fn x(pre : i32, p : ptr<workgroup, i32>, post : i32) -> i32 {
  return ((pre + *(p)) + post);
}

fn y(pre : i32, p : ptr<workgroup, i32>, post : i32) -> i32 {
  return x(pre, p, post);
}

fn z() {
  let i = 0;
  y(1, &(a[min(u32(i), 3u)]), 2);
}
)",
                          /* predicate */ R"(
var<workgroup> a : array<i32, 4>;

fn x(pre : i32, p : ptr<workgroup, i32>, p_predicate : bool, post : i32) -> i32 {
  var predicated_expr : i32;
  if (p_predicate) {
    predicated_expr = *(p);
  }
  return ((pre + predicated_expr) + post);
}

fn y(pre : i32, p : ptr<workgroup, i32>, p_predicate_1 : bool, post : i32) -> i32 {
  return x(pre, p, p_predicate_1, post);
}

fn z() {
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  y(1, &(a[index]), predicate, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_UniformPointerParameter_IndexWithLet) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<vec4i, 4>;

fn x(pre : vec4i, p : ptr<uniform, vec4i>, post : vec4i) -> vec4i {
  return ((pre + *(p)) + post);
}

fn y(pre : vec4i, p : ptr<uniform, vec4i>, post : vec4i) -> vec4i {
  return x(pre, p, post);
}

fn z() {
  let i = 0;
  y(vec4(1), &(a[i]), vec4(2));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var<uniform> a : array<vec4i, 4>;

fn x(pre : vec4i, p : ptr<uniform, vec4i>, post : vec4i) -> vec4i {
  return ((pre + *(p)) + post);
}

fn y(pre : vec4i, p : ptr<uniform, vec4i>, post : vec4i) -> vec4i {
  return x(pre, p, post);
}

fn z() {
  let i = 0;
  y(vec4(1), &(a[min(u32(i), 3u)]), vec4(2));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var<uniform> a : array<vec4i, 4>;

fn x(pre : vec4i, p : ptr<uniform, vec4i>, p_predicate : bool, post : vec4i) -> vec4i {
  var predicated_expr : vec4<i32>;
  if (p_predicate) {
    predicated_expr = *(p);
  }
  return ((pre + predicated_expr) + post);
}

fn y(pre : vec4i, p : ptr<uniform, vec4i>, p_predicate_1 : bool, post : vec4i) -> vec4i {
  return x(pre, p, p_predicate_1, post);
}

fn z() {
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  y(vec4(1), &(a[index]), predicate, vec4(2));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_StoragePointerParameter_IndexWithLet) {
    auto* src = R"(
@group(0) @binding(0) var<storage> a : array<vec4i, 4>;

fn x(pre : vec4i, p : ptr<storage, vec4i>, post : vec4i) -> vec4i {
  return ((pre + *(p)) + post);
}

fn y(pre : vec4i, p : ptr<storage, vec4i>, post : vec4i) -> vec4i {
  return x(pre, p, post);
}

fn z() {
  let i = 0;
  y(vec4(1), &(a[i]), vec4(2));
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var<storage> a : array<vec4i, 4>;

fn x(pre : vec4i, p : ptr<storage, vec4i>, post : vec4i) -> vec4i {
  return ((pre + *(p)) + post);
}

fn y(pre : vec4i, p : ptr<storage, vec4i>, post : vec4i) -> vec4i {
  return x(pre, p, post);
}

fn z() {
  let i = 0;
  y(vec4(1), &(a[min(u32(i), 3u)]), vec4(2));
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var<storage> a : array<vec4i, 4>;

fn x(pre : vec4i, p : ptr<storage, vec4i>, p_predicate : bool, post : vec4i) -> vec4i {
  var predicated_expr : vec4<i32>;
  if (p_predicate) {
    predicated_expr = *(p);
  }
  return ((pre + predicated_expr) + post);
}

fn y(pre : vec4i, p : ptr<storage, vec4i>, p_predicate_1 : bool, post : vec4i) -> vec4i {
  return x(pre, p, p_predicate_1, post);
}

fn z() {
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  y(vec4(1), &(a[index]), predicate, vec4(2));
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_FunctionPointerParameter_IndexWithLet) {
    auto* src = R"(
fn x(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return ((pre + *(p)) + post);
}

fn y(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return x(pre, p, post);
}

fn z() {
  var a : array<i32, 4>;
  let i = 0;
  y(1, &(a[i]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
fn x(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return ((pre + *(p)) + post);
}

fn y(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return x(pre, p, post);
}

fn z() {
  var a : array<i32, 4>;
  let i = 0;
  y(1, &(a[min(u32(i), 3u)]), 2);
}
)",
                          /* predicate */ R"(
fn x(pre : i32, p : ptr<function, i32>, p_predicate : bool, post : i32) -> i32 {
  var predicated_expr : i32;
  if (p_predicate) {
    predicated_expr = *(p);
  }
  return ((pre + predicated_expr) + post);
}

fn y(pre : i32, p : ptr<function, i32>, p_predicate_1 : bool, post : i32) -> i32 {
  return x(pre, p, p_predicate_1, post);
}

fn z() {
  var a : array<i32, 4>;
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  y(1, &(a[index]), predicate, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Write_PrivatePointerParameter_IndexWithConstant) {
    auto* src = R"(
var<private> a : array<i32, 4>;

fn x(pre : i32, p : ptr<private, i32>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<private, i32>, post : i32) {
  x(pre, p, post);
}

fn z() {
  y(1, &(a[1]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ src,
                          /* predicate */ R"(
var<private> a : array<i32, 4>;

fn x(pre : i32, p : ptr<private, i32>, p_predicate : bool, post : i32) {
  if (p_predicate) {
    *(p) = (pre + post);
  }
}

fn y(pre : i32, p : ptr<private, i32>, p_predicate_1 : bool, post : i32) {
  x(pre, p, p_predicate_1, post);
}

fn z() {
  y(1, &(a[1]), true, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Write_WorkgroupPointerParameter_IndexWithConstant) {
    auto* src = R"(
var<workgroup> a : array<i32, 4>;

fn x(pre : i32, p : ptr<workgroup, i32>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<workgroup, i32>, post : i32) {
  x(pre, p, post);
}

fn z() {
  y(1, &(a[1]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ src,
                          /* predicate */ R"(
var<workgroup> a : array<i32, 4>;

fn x(pre : i32, p : ptr<workgroup, i32>, p_predicate : bool, post : i32) {
  if (p_predicate) {
    *(p) = (pre + post);
  }
}

fn y(pre : i32, p : ptr<workgroup, i32>, p_predicate_1 : bool, post : i32) {
  x(pre, p, p_predicate_1, post);
}

fn z() {
  y(1, &(a[1]), true, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Write_StoragePointerParameter_IndexWithConstant) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> a : array<i32, 4>;

fn x(pre : i32, p : ptr<storage, i32, read_write>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<storage, i32, read_write>, post : i32) {
  x(pre, p, post);
}

fn z() {
  y(1, &(a[1]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ src,
                          /* predicate */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<i32, 4>;

fn x(pre : i32, p : ptr<storage, i32, read_write>, p_predicate : bool, post : i32) {
  if (p_predicate) {
    *(p) = (pre + post);
  }
}

fn y(pre : i32, p : ptr<storage, i32, read_write>, p_predicate_1 : bool, post : i32) {
  x(pre, p, p_predicate_1, post);
}

fn z() {
  y(1, &(a[1]), true, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Write_FunctionPointerParameter_IndexWithConstant) {
    auto* src = R"(
fn x(pre : i32, p : ptr<function, i32>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<function, i32>, post : i32) {
  x(pre, p, post);
}

fn z() {
  var a : array<i32, 4>;
  y(1, &(a[1]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ src,
                          /* predicate */ R"(
fn x(pre : i32, p : ptr<function, i32>, p_predicate : bool, post : i32) {
  if (p_predicate) {
    *(p) = (pre + post);
  }
}

fn y(pre : i32, p : ptr<function, i32>, p_predicate_1 : bool, post : i32) {
  x(pre, p, p_predicate_1, post);
}

fn z() {
  var a : array<i32, 4>;
  y(1, &(a[1]), true, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Write_PrivatePointerParameter_IndexWithLet) {
    auto* src = R"(
var<private> a : array<i32, 4>;

fn x(pre : i32, p : ptr<private, i32>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<private, i32>, post : i32) {
  x(pre, p, post);
}

fn z() {
  let i = 0;
  y(1, &(a[i]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<private> a : array<i32, 4>;

fn x(pre : i32, p : ptr<private, i32>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<private, i32>, post : i32) {
  x(pre, p, post);
}

fn z() {
  let i = 0;
  y(1, &(a[min(u32(i), 3u)]), 2);
}
)",
                          /* predicate */ R"(
var<private> a : array<i32, 4>;

fn x(pre : i32, p : ptr<private, i32>, p_predicate : bool, post : i32) {
  if (p_predicate) {
    *(p) = (pre + post);
  }
}

fn y(pre : i32, p : ptr<private, i32>, p_predicate_1 : bool, post : i32) {
  x(pre, p, p_predicate_1, post);
}

fn z() {
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  y(1, &(a[index]), predicate, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Write_WorkgroupPointerParameter_IndexWithLet) {
    auto* src = R"(
var<workgroup> a : array<i32, 4>;

fn x(pre : i32, p : ptr<workgroup, i32>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<workgroup, i32>, post : i32) {
  x(pre, p, post);
}

fn z() {
  let i = 0;
  y(1, &(a[i]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
var<workgroup> a : array<i32, 4>;

fn x(pre : i32, p : ptr<workgroup, i32>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<workgroup, i32>, post : i32) {
  x(pre, p, post);
}

fn z() {
  let i = 0;
  y(1, &(a[min(u32(i), 3u)]), 2);
}
)",
                          /* predicate */ R"(
var<workgroup> a : array<i32, 4>;

fn x(pre : i32, p : ptr<workgroup, i32>, p_predicate : bool, post : i32) {
  if (p_predicate) {
    *(p) = (pre + post);
  }
}

fn y(pre : i32, p : ptr<workgroup, i32>, p_predicate_1 : bool, post : i32) {
  x(pre, p, p_predicate_1, post);
}

fn z() {
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  y(1, &(a[index]), predicate, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Write_StoragePointerParameter_IndexWithLet) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> a : array<i32, 4>;

fn x(pre : i32, p : ptr<storage, i32, read_write>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<storage, i32, read_write>, post : i32) {
  x(pre, p, post);
}

fn z() {
  let i = 0;
  y(1, &(a[i]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<i32, 4>;

fn x(pre : i32, p : ptr<storage, i32, read_write>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<storage, i32, read_write>, post : i32) {
  x(pre, p, post);
}

fn z() {
  let i = 0;
  y(1, &(a[min(u32(i), 3u)]), 2);
}
)",
                          /* predicate */ R"(
@group(0) @binding(0) var<storage, read_write> a : array<i32, 4>;

fn x(pre : i32, p : ptr<storage, i32, read_write>, p_predicate : bool, post : i32) {
  if (p_predicate) {
    *(p) = (pre + post);
  }
}

fn y(pre : i32, p : ptr<storage, i32, read_write>, p_predicate_1 : bool, post : i32) {
  x(pre, p, p_predicate_1, post);
}

fn z() {
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  y(1, &(a[index]), predicate, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Write_FunctionPointerParameter_IndexWithLet) {
    auto* src = R"(
fn x(pre : i32, p : ptr<function, i32>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<function, i32>, post : i32) {
  x(pre, p, post);
}

fn z() {
  var a : array<i32, 4>;
  let i = 0;
  y(1, &(a[i]), 2);
}
)";

    auto* expect = Expect(GetParam(),
                          /* ignore */ src,
                          /* clamp */ R"(
fn x(pre : i32, p : ptr<function, i32>, post : i32) {
  *(p) = (pre + post);
}

fn y(pre : i32, p : ptr<function, i32>, post : i32) {
  x(pre, p, post);
}

fn z() {
  var a : array<i32, 4>;
  let i = 0;
  y(1, &(a[min(u32(i), 3u)]), 2);
}
)",
                          /* predicate */ R"(
fn x(pre : i32, p : ptr<function, i32>, p_predicate : bool, post : i32) {
  if (p_predicate) {
    *(p) = (pre + post);
  }
}

fn y(pre : i32, p : ptr<function, i32>, p_predicate_1 : bool, post : i32) {
  x(pre, p, p_predicate_1, post);
}

fn z() {
  var a : array<i32, 4>;
  let i = 0;
  let index = i;
  let predicate = (u32(index) <= 3u);
  y(1, &(a[index]), predicate, 2);
}
)");

    auto got = Run<Robustness>(src, Config(GetParam()));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// disable_runtime_sized_array_index_clamping == true
////////////////////////////////////////////////////////////////////////////////

TEST_P(RobustnessTest, Read_disable_unsized_array_index_clamping_i32) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> s : array<f32>;

fn f() {
  let i = 25i;
  var d : f32 = s[i];
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<storage, read> s : array<f32>;

fn f() {
  let i = 25i;
  var d : f32 = s[u32(i)];
}
)";

    auto got = Run<Robustness>(src, Config(GetParam(), true));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_disable_unsized_array_index_clamping_u32) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> s : array<f32>;

fn f() {
  let i = 25u;
  var d : f32 = s[i];
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam(), true));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_disable_unsized_array_index_clamping_abstract_int) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> s : array<f32>;

fn f() {
  var d : f32 = s[25];
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<storage, read> s : array<f32>;

fn f() {
  var d : f32 = s[u32(25)];
}
)";

    auto got = Run<Robustness>(src, Config(GetParam(), true));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Read_disable_unsized_array_index_clamping_abstract_int_ViaPointerIndex) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> s : array<f32>;

fn f() {
  let p = &(s);
  var d : f32 = p[25];
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<storage, read> s : array<f32>;

fn f() {
  let p = &(s);
  var d : f32 = p[u32(25)];
}
)";

    auto got = Run<Robustness>(src, Config(GetParam(), true));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Assign_disable_unsized_array_index_clamping_i32) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> s : array<f32>;

fn f() {
  let i = 25i;
  s[i] = 0.5f;
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<storage, read_write> s : array<f32>;

fn f() {
  let i = 25i;
  s[u32(i)] = 0.5f;
}
)";

    auto got = Run<Robustness>(src, Config(GetParam(), true));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Assign_disable_unsized_array_index_clamping_u32) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> s : array<f32>;

fn f() {
  let i = 25u;
  s[i] = 0.5f;
}
)";

    auto* expect = src;

    auto got = Run<Robustness>(src, Config(GetParam(), true));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Assign_disable_unsized_array_index_clamping_abstract_int) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> s : array<f32>;

fn f() {
  s[25] = 0.5f;
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<storage, read_write> s : array<f32>;

fn f() {
  s[u32(25)] = 0.5f;
}
)";

    auto got = Run<Robustness>(src, Config(GetParam(), true));

    EXPECT_EQ(expect, str(got));
}

TEST_P(RobustnessTest, Assign_disable_unsized_array_index_clamping_abstract_int_ViaPointerIndex) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> s : array<f32>;

fn f() {
  let p = &(s);
  p[25] = 0.5f;
}
)";

    auto* expect = R"(
@group(0) @binding(0) var<storage, read_write> s : array<f32>;

fn f() {
  let p = &(s);
  p[u32(25)] = 0.5f;
}
)";

    auto got = Run<Robustness>(src, Config(GetParam(), true));

    EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(,
                         RobustnessTest,
                         testing::Values(Robustness::Action::kIgnore,
                                         Robustness::Action::kClamp,
                                         Robustness::Action::kPredicate));
}  // namespace
}  // namespace tint::ast::transform
