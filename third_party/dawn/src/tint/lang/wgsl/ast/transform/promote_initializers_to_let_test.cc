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

#include "src/tint/lang/wgsl/ast/transform/promote_initializers_to_let.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using PromoteInitializersToLetTest = TransformTest;

TEST_F(PromoteInitializersToLetTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, BasicConstArray) {
    auto* src = R"(
fn f() {
  const f0 = 1.0;
  const f1 = 2.0;
  const f2 = 3.0;
  const f3 = 4.0;
  var i = array<f32, 4u>(f0, f1, f2, f3)[2];
}
)";

    EXPECT_FALSE(ShouldRun<PromoteInitializersToLet>(src));
}

TEST_F(PromoteInitializersToLetTest, BasicRuntimeArray) {
    auto* src = R"(
fn f() {
  var f0 = 1.0;
  var f1 = 2.0;
  var f2 = 3.0;
  var f3 = 4.0;
  var i = array<f32, 4u>(f0, f1, f2, f3)[2];
}
)";

    auto* expect = R"(
fn f() {
  var f0 = 1.0;
  var f1 = 2.0;
  var f2 = 3.0;
  var f3 = 4.0;
  let tint_symbol : array<f32, 4u> = array<f32, 4u>(f0, f1, f2, f3);
  var i = tint_symbol[2];
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, BasicConstStruct) {
    auto* src = R"(
struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
};

fn f() {
  var x = S(1, 2.0, vec3<f32>()).b;
}
)";

    EXPECT_FALSE(ShouldRun<PromoteInitializersToLet>(src));
}

TEST_F(PromoteInitializersToLetTest, BasicRuntimeStruct) {
    auto* src = R"(
struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
};

fn f() {
  let runtime_value = 1;
  var x = S(runtime_value, 2.0, vec3<f32>()).b;
}
)";

    auto* expect = R"(
struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
}

fn f() {
  let runtime_value = 1;
  let tint_symbol : S = S(runtime_value, 2.0, vec3<f32>());
  var x = tint_symbol.b;
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, BasicStruct_OutOfOrder) {
    auto* src = R"(
fn f() {
  let runtime_value = 1;
  var x = S(runtime_value, 2.0, vec3<f32>()).b;
}

struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
};
)";

    auto* expect = R"(
fn f() {
  let runtime_value = 1;
  let tint_symbol : S = S(runtime_value, 2.0, vec3<f32>());
  var x = tint_symbol.b;
}

struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstBasicArray) {
    auto* src = R"(
const f0 = 1.0;

const f1 = 2.0;

const C = array<f32, 2u>(f0, f1);

fn f() {
  var f0 = 100.0;
  var f1 = 100.0;
  var i = C[1]; // Not hoisted, as the final const value is not an array
}
)";

    EXPECT_FALSE(ShouldRun<PromoteInitializersToLet>(src));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstBasicArray_OutOfOrder) {
    auto* src = R"(
fn f() {
  var f0 = 100.0;
  var f1 = 100.0;
  var i = C[1];
}

const C = array<f32, 2u>(f0, f1);

const f0 = 1.0;

const f1 = 2.0;
)";

    EXPECT_FALSE(ShouldRun<PromoteInitializersToLet>(src));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayDynamicIndex) {
    auto* src = R"(
const TRI_VERTICES = array(
  vec4(0., 0., 0., 1.),
  vec4(0., 1., 0., 1.),
  vec4(1., 1., 0., 1.),
);

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
  // note: TRI_VERTICES requires a materialize before the dynamic index.
  return TRI_VERTICES[in_vertex_index];
}
)";

    auto* expect = R"(
const TRI_VERTICES = array(vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0), vec4(1.0, 1.0, 0.0, 1.0));

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index : u32) -> @builtin(position) vec4<f32> {
  let tint_symbol : array<vec4<f32>, 3u> = TRI_VERTICES;
  return tint_symbol[in_vertex_index];
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstBasicArray) {
    auto* src = R"(
fn f() {
  const f0 = 1.0;
  const f1 = 2.0;
  const C = array<f32, 2u>(f0, f1);
  var i = C[1];  // Not hoisted, as the final const value is not an array
}
)";

    EXPECT_FALSE(ShouldRun<PromoteInitializersToLet>(src));
}

TEST_F(PromoteInitializersToLetTest, LocalConstBasicArrayRuntimeIndex) {
    auto* src = R"(
fn f() {
  const f0 = 1.0;
  const f1 = 2.0;
  const C = array<f32, 2u>(f0, f1);
  let runtime_value = 1;
  var i = C[runtime_value];
}
)";

    auto* expect = R"(
fn f() {
  const f0 = 1.0;
  const f1 = 2.0;
  const C = array<f32, 2u>(f0, f1);
  let runtime_value = 1;
  let tint_symbol : array<f32, 2u> = C;
  var i = tint_symbol[runtime_value];
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInForLoopInit) {
    auto* src = R"(
fn f() {
  var insert_after = 1;
  for(var i = array<f32, 4u>(0.0, 1.0, 2.0, 3.0)[insert_after]; ; ) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var insert_after = 1;
  {
    let tint_symbol : array<f32, 4u> = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
    var i = tint_symbol[insert_after];
    loop {
      {
        break;
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInForLoopInit) {
    auto* src = R"(
fn f() {
  const arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  let runtime_value = 1;
  var insert_after = 1;
  for(var i = arr[runtime_value]; ; ) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  const arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  let runtime_value = 1;
  var insert_after = 1;
  {
    let tint_symbol : array<f32, 4u> = arr;
    var i = tint_symbol[runtime_value];
    loop {
      {
        break;
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayInForLoopInit) {
    auto* src = R"(
const arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);

fn f() {
  let runtime_value = 1;
  var insert_after = 1;
  for(var i = arr[runtime_value]; ; ) {
    break;
  }
}
)";

    auto* expect = R"(
const arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);

fn f() {
  let runtime_value = 1;
  var insert_after = 1;
  {
    let tint_symbol : array<f32, 4u> = arr;
    var i = tint_symbol[runtime_value];
    loop {
      {
        break;
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, StructInForLoopInit) {
    auto* src = R"(
struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
};

fn get_b_runtime(s : S) -> f32 {
  return s.b;
}

fn f() {
  var insert_after = 1;
  for(var x = get_b_runtime(S(1, 2.0, vec3<f32>())); ; ) {
    break;
  }
}
)";

    auto* expect = R"(
struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
}

fn get_b_runtime(s : S) -> f32 {
  return s.b;
}

fn f() {
  var insert_after = 1;
  {
    let tint_symbol : S = S(1, 2.0, vec3<f32>());
    var x = get_b_runtime(tint_symbol);
    loop {
      {
        break;
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, StructInForLoopInit_OutOfOrder) {
    auto* src = R"(
fn f() {
  var insert_after = 1;
  for(var x = get_b_runtime(S(1, 2.0, vec3<f32>())); ; ) {
    break;
  }
}

fn get_b_runtime(s : S) -> f32 {
  return s.b;
}

struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
};
)";

    auto* expect = R"(
fn f() {
  var insert_after = 1;
  {
    let tint_symbol : S = S(1, 2.0, vec3<f32>());
    var x = get_b_runtime(tint_symbol);
    loop {
      {
        break;
      }
    }
  }
}

fn get_b_runtime(s : S) -> f32 {
  return s.b;
}

struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInForLoopCond) {
    auto* src = R"(
fn f() {
  var f = 1.0;
  for(; f == array<f32, 1u>(f)[0]; f = f + 1.0) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  var f = 1.0;
  loop {
    let tint_symbol : array<f32, 1u> = array<f32, 1u>(f);
    if (!((f == tint_symbol[0]))) {
      break;
    }
    {
      var marker = 1;
    }

    continuing {
      f = (f + 1.0);
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInForLoopCond) {
    auto* src = R"(
fn f() {
  let runtime_value = 0;
  const f = 1.0;
  const arr = array<f32, 1u>(f);
  for(var i = f; i == arr[runtime_value]; i = i + 1.0) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  let runtime_value = 0;
  const f = 1.0;
  const arr = array<f32, 1u>(f);
  {
    var i = f;
    loop {
      let tint_symbol : array<f32, 1u> = arr;
      if (!((i == tint_symbol[runtime_value]))) {
        break;
      }
      {
        var marker = 1;
      }

      continuing {
        i = (i + 1.0);
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayInForLoopCond) {
    auto* src = R"(
const f = 1.0;

const arr = array<f32, 1u>(f);

fn F() {
  let runtime_value = 0;
  for(var i = f; i == arr[runtime_value]; i = i + 1.0) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
const f = 1.0;

const arr = array<f32, 1u>(f);

fn F() {
  let runtime_value = 0;
  {
    var i = f;
    loop {
      let tint_symbol : array<f32, 1u> = arr;
      if (!((i == tint_symbol[runtime_value]))) {
        break;
      }
      {
        var marker = 1;
      }

      continuing {
        i = (i + 1.0);
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInForLoopCont) {
    auto* src = R"(
fn f() {
  let runtime_value = 0;
  var f = 0.0;
  for(; f < 10.0; f = f + array<f32, 1u>(1.0)[runtime_value]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  let runtime_value = 0;
  var f = 0.0;
  loop {
    if (!((f < 10.0))) {
      break;
    }
    {
      var marker = 1;
    }

    continuing {
      let tint_symbol : array<f32, 1u> = array<f32, 1u>(1.0);
      f = (f + tint_symbol[runtime_value]);
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInForLoopCont) {
    auto* src = R"(
fn f() {
  let runtime_value = 0;
  const arr = array<f32, 1u>(1.0);
  var f = 0.0;
  for(; f < 10.0; f = f + arr[runtime_value]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  let runtime_value = 0;
  const arr = array<f32, 1u>(1.0);
  var f = 0.0;
  loop {
    if (!((f < 10.0))) {
      break;
    }
    {
      var marker = 1;
    }

    continuing {
      let tint_symbol : array<f32, 1u> = arr;
      f = (f + tint_symbol[runtime_value]);
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayInForLoopCont) {
    auto* src = R"(
const arr = array<f32, 1u>(1.0);

fn f() {
  let runtime_value = 0;
  var f = 0.0;
  for(; f < 10.0; f = f + arr[runtime_value]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
const arr = array<f32, 1u>(1.0);

fn f() {
  let runtime_value = 0;
  var f = 0.0;
  loop {
    if (!((f < 10.0))) {
      break;
    }
    {
      var marker = 1;
    }

    continuing {
      let tint_symbol : array<f32, 1u> = arr;
      f = (f + tint_symbol[runtime_value]);
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInForLoopInitCondCont) {
    auto* src = R"(
fn f() {
  let runtime_value = 0;
  for(var f = array<f32, 1u>(0.0)[runtime_value];
      f < array<f32, 1u>(1.0)[runtime_value];
      f = f + array<f32, 1u>(2.0)[runtime_value]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  let runtime_value = 0;
  {
    let tint_symbol : array<f32, 1u> = array<f32, 1u>(0.0);
    var f = tint_symbol[runtime_value];
    loop {
      let tint_symbol_1 : array<f32, 1u> = array<f32, 1u>(1.0);
      if (!((f < tint_symbol_1[runtime_value]))) {
        break;
      }
      {
        var marker = 1;
      }

      continuing {
        let tint_symbol_2 : array<f32, 1u> = array<f32, 1u>(2.0);
        f = (f + tint_symbol_2[runtime_value]);
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInForLoopInitCondCont) {
    auto* src = R"(
fn f() {
  let runtime_value = 0;
  const arr_a = array<f32, 1u>(0.0);
  const arr_b = array<f32, 1u>(1.0);
  const arr_c = array<f32, 1u>(2.0);
  for(var f = arr_a[runtime_value]; f < arr_b[runtime_value]; f = f + arr_c[runtime_value]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  let runtime_value = 0;
  const arr_a = array<f32, 1u>(0.0);
  const arr_b = array<f32, 1u>(1.0);
  const arr_c = array<f32, 1u>(2.0);
  {
    let tint_symbol : array<f32, 1u> = arr_a;
    var f = tint_symbol[runtime_value];
    loop {
      let tint_symbol_1 : array<f32, 1u> = arr_b;
      if (!((f < tint_symbol_1[runtime_value]))) {
        break;
      }
      {
        var marker = 1;
      }

      continuing {
        let tint_symbol_2 : array<f32, 1u> = arr_c;
        f = (f + tint_symbol_2[runtime_value]);
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInElseIf) {
    auto* src = R"(
fn f() {
  var f = 1.0;
  if (true) {
    var marker = 0;
  } else if (f == array<f32, 2u>(f, f)[0]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  var f = 1.0;
  if (true) {
    var marker = 0;
  } else {
    let tint_symbol : array<f32, 2u> = array<f32, 2u>(f, f);
    if ((f == tint_symbol[0])) {
      var marker = 1;
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInElseIfChain) {
    auto* src = R"(
fn f() {
  var f = 1.0;
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else if (f == array<f32, 2u>(f, f)[0]) {
    var marker = 2;
  } else if (f == array<f32, 2u>(f, f)[1]) {
    var marker = 3;
  } else if (true) {
    var marker = 4;
  } else {
    var marker = 5;
  }
}
)";

    auto* expect = R"(
fn f() {
  var f = 1.0;
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else {
    let tint_symbol : array<f32, 2u> = array<f32, 2u>(f, f);
    if ((f == tint_symbol[0])) {
      var marker = 2;
    } else {
      let tint_symbol_1 : array<f32, 2u> = array<f32, 2u>(f, f);
      if ((f == tint_symbol_1[1])) {
        var marker = 3;
      } else if (true) {
        var marker = 4;
      } else {
        var marker = 5;
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInElseIfChain) {
    auto* src = R"(
fn f() {
  let runtime_value = 0;
  const f = 1.0;
  const arr = array<f32, 2u>(f, f);
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else if (f == arr[runtime_value]) {
    var marker = 2;
  } else if (f == arr[runtime_value + 1]) {
    var marker = 3;
  } else if (true) {
    var marker = 4;
  } else {
    var marker = 5;
  }
}
)";

    auto* expect = R"(
fn f() {
  let runtime_value = 0;
  const f = 1.0;
  const arr = array<f32, 2u>(f, f);
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else {
    let tint_symbol : array<f32, 2u> = arr;
    if ((f == tint_symbol[runtime_value])) {
      var marker = 2;
    } else {
      let tint_symbol_1 : array<f32, 2u> = arr;
      if ((f == tint_symbol_1[(runtime_value + 1)])) {
        var marker = 3;
      } else if (true) {
        var marker = 4;
      } else {
        var marker = 5;
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayInElseIfChain) {
    auto* src = R"(
const f = 1.0;

const arr = array<f32, 2u>(f, f);

fn F() {
  let runtime_value = 0;
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else if (f == arr[runtime_value]) {
    var marker = 2;
  } else if (f == arr[runtime_value + 1]) {
    var marker = 3;
  } else if (true) {
    var marker = 4;
  } else {
    var marker = 5;
  }
}
)";

    auto* expect = R"(
const f = 1.0;

const arr = array<f32, 2u>(f, f);

fn F() {
  let runtime_value = 0;
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else {
    let tint_symbol : array<f32, 2u> = arr;
    if ((f == tint_symbol[runtime_value])) {
      var marker = 2;
    } else {
      let tint_symbol_1 : array<f32, 2u> = arr;
      if ((f == tint_symbol_1[(runtime_value + 1)])) {
        var marker = 3;
      } else if (true) {
        var marker = 4;
      } else {
        var marker = 5;
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInArrayArrayConstIndex) {
    auto* src = R"(
fn f() {
  var i = array<array<f32, 2u>, 2u>(array<f32, 2u>(1.0, 2.0), array<f32, 2u>(3.0, 4.0))[0][1];
}
)";

    EXPECT_FALSE(ShouldRun<PromoteInitializersToLet>(src));
}

TEST_F(PromoteInitializersToLetTest, ArrayInArrayArrayRuntimeIndex) {
    auto* src = R"(
fn f() {
  let runtime_value = 1;
  var i = array<array<f32, 2u>, 2u>(array<f32, 2u>(1.0, 2.0), array<f32, 2u>(3.0, 4.0))[runtime_value][runtime_value + 1];
}
)";

    auto* expect = R"(
fn f() {
  let runtime_value = 1;
  let tint_symbol : array<array<f32, 2u>, 2u> = array<array<f32, 2u>, 2u>(array<f32, 2u>(1.0, 2.0), array<f32, 2u>(3.0, 4.0));
  var i = tint_symbol[runtime_value][(runtime_value + 1)];
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInArrayArrayConstIndex) {
    auto* src = R"(
fn f() {
  const arr_0 = array<f32, 2u>(1.0, 2.0);
  const arr_1 = array<f32, 2u>(3.0, 4.0);
  const arr_2 = array<array<f32, 2u>, 2u>(arr_0, arr_1);
  var i = arr_2[0][1];
}
)";

    EXPECT_FALSE(ShouldRun<PromoteInitializersToLet>(src));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInArrayArrayRuntimeIndex) {
    auto* src = R"(
fn f() {
  let runtime_value = 1;
  const arr_0 = array<f32, 2u>(1.0, 2.0);
  const arr_1 = array<f32, 2u>(3.0, 4.0);
  const arr_2 = array<array<f32, 2u>, 2u>(arr_0, arr_1);
  var i = arr_2[runtime_value][runtime_value + 1];
}
)";

    auto* expect = R"(
fn f() {
  let runtime_value = 1;
  const arr_0 = array<f32, 2u>(1.0, 2.0);
  const arr_1 = array<f32, 2u>(3.0, 4.0);
  const arr_2 = array<array<f32, 2u>, 2u>(arr_0, arr_1);
  let tint_symbol : array<array<f32, 2u>, 2u> = arr_2;
  var i = tint_symbol[runtime_value][(runtime_value + 1)];
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayInArrayArray) {
    auto* src = R"(
const arr_0 = array<f32, 2u>(1.0, 2.0);

const arr_1 = array<f32, 2u>(3.0, 4.0);

const arr_2 = array<array<f32, 2u>, 2u>(arr_0, arr_1);

fn f() {
  let runtime_value = 1;
  var i = arr_2[runtime_value][runtime_value + 1];
}
)";

    auto* expect = R"(
const arr_0 = array<f32, 2u>(1.0, 2.0);

const arr_1 = array<f32, 2u>(3.0, 4.0);

const arr_2 = array<array<f32, 2u>, 2u>(arr_0, arr_1);

fn f() {
  let runtime_value = 1;
  let tint_symbol : array<array<f32, 2u>, 2u> = arr_2;
  var i = tint_symbol[runtime_value][(runtime_value + 1)];
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, StructNested) {
    auto* src = R"(
struct S1 {
  a : i32,
};

struct S2 {
  a : i32,
  b : S1,
  c : i32,
};

struct S3 {
  a : S2,
};

fn get_a(s : S3) -> S2 {
  return s.a;
}

fn f() {
  var x = get_a(S3(S2(1, S1(2), 3))).b.a;
}
)";

    auto* expect = R"(
struct S1 {
  a : i32,
}

struct S2 {
  a : i32,
  b : S1,
  c : i32,
}

struct S3 {
  a : S2,
}

fn get_a(s : S3) -> S2 {
  return s.a;
}

fn f() {
  let tint_symbol : S3 = S3(S2(1, S1(2), 3));
  var x = get_a(tint_symbol).b.a;
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, Mixed) {
    auto* src = R"(
struct S1 {
  a : i32,
};

struct S2 {
  a : array<S1, 3u>,
};

fn get_a(s : S2) -> array<S1, 3u> {
  return s.a;
}

fn f() {
  var x = get_a(S2(array<S1, 3u>(S1(1), S1(2), S1(3))))[1].a;
}
)";

    auto* expect = R"(
struct S1 {
  a : i32,
}

struct S2 {
  a : array<S1, 3u>,
}

fn get_a(s : S2) -> array<S1, 3u> {
  return s.a;
}

fn f() {
  let tint_symbol : S2 = S2(array<S1, 3u>(S1(1), S1(2), S1(3)));
  var x = get_a(tint_symbol)[1].a;
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, Mixed_OutOfOrder) {
    auto* src = R"(
fn f() {
  var x = get_a(S2(array<S1, 3u>(S1(1), S1(2), S1(3))))[1].a;
}

fn get_a(s : S2) -> array<S1, 3u> {
  return s.a;
}

struct S2 {
  a : array<S1, 3u>,
};

struct S1 {
  a : i32,
};
)";

    auto* expect = R"(
fn f() {
  let tint_symbol : S2 = S2(array<S1, 3u>(S1(1), S1(2), S1(3)));
  var x = get_a(tint_symbol)[1].a;
}

fn get_a(s : S2) -> array<S1, 3u> {
  return s.a;
}

struct S2 {
  a : array<S1, 3u>,
}

struct S1 {
  a : i32,
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, NoChangeOnVarDecl) {
    auto* src = R"(
alias F = f32;

fn f() {
  var local_arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  var local_str = F(3.0);
}

const module_arr : array<f32, 4u> = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);

const module_str : F = F(2.0);
)";

    auto* expect = src;

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, NoChangeOnVarDecl_OutOfOrder) {
    auto* src = R"(
fn f() {
  var local_arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  var local_str = F(3.0);
}

const module_str : F = F(2.0);

alias F = f32;

const module_arr : array<f32, 4u> = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
)";

    auto* expect = src;

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ForLoopShadowing) {
    auto* src = R"(
fn X() {
  var i = 10;
  for(var f = 0; f < 10; f = f + array<i32, 1u>(i)[0]) {
      var i = 20;
  }
}

fn Y() {
  var i = 10;
  for(var f = 0; f < array<i32, 1u>(i)[0]; f = f + 1) {
      var i = 20;
  }
}

fn Z() {
  var i = 10;
  for(var f = array<i32, 1u>(i)[0]; f < 10; f = f + 1) {
      var i = 20;
  }
}
)";

    auto* expect = R"(
fn X() {
  var i = 10;
  {
    var f = 0;
    loop {
      if (!((f < 10))) {
        break;
      }
      {
        var i = 20;
      }

      continuing {
        let tint_symbol : array<i32, 1u> = array<i32, 1u>(i);
        f = (f + tint_symbol[0]);
      }
    }
  }
}

fn Y() {
  var i = 10;
  {
    var f = 0;
    loop {
      let tint_symbol_1 : array<i32, 1u> = array<i32, 1u>(i);
      if (!((f < tint_symbol_1[0]))) {
        break;
      }
      {
        var i = 20;
      }

      continuing {
        f = (f + 1);
      }
    }
  }
}

fn Z() {
  var i = 10;
  {
    let tint_symbol_2 : array<i32, 1u> = array<i32, 1u>(i);
    var f = tint_symbol_2[0];
    loop {
      if (!((f < 10))) {
        break;
      }
      {
        var i = 20;
      }

      continuing {
        f = (f + 1);
      }
    }
  }
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, AssignAbstractArray) {
    // Test that hoisting an array with abstract type still materializes to the correct type.
    auto* src = R"(
fn f() {
  var arr : array<f32, 4>;
  arr = array(1, 2, 3, 4);
}
)";

    auto* expect = R"(
fn f() {
  var arr : array<f32, 4>;
  let tint_symbol : array<f32, 4u> = array(1, 2, 3, 4);
  arr = tint_symbol;
}
)";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, AssignAbstractArray_ToPhony) {
    // Test that we do not try to hoist an abstract array expression that is the RHS of a phony
    // assignment, as its type will not be materialized.
    auto* src = R"(
fn f() {
  _ = array(1, 2, 3, 4);
}
)";

    EXPECT_FALSE(ShouldRun<PromoteInitializersToLet>(src));
}

TEST_F(PromoteInitializersToLetTest, Bug2241) {
    auto* src = R"(
fn f () {
  let v = false && array<i32, array(array(6))[0][0]>()[0] == 0;
}
)";
    EXPECT_FALSE(ShouldRun<PromoteInitializersToLet>(src));
}

}  // namespace
}  // namespace tint::ast::transform
