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

#include "src/tint/lang/hlsl/writer/ast_raise/localize_struct_array_assignment.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::hlsl::writer {
namespace {

using LocalizeStructArrayAssignmentTest = ast::transform::TransformTest;
using Unshadow = ast::transform::Unshadow;
using SimplifyPointers = ast::transform::SimplifyPointers;

TEST_F(LocalizeStructArrayAssignmentTest, EmptyModule) {
    auto* src = R"()";
    EXPECT_FALSE(ShouldRun<LocalizeStructArrayAssignment>(src));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructArray) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct OuterS {
  a1 : array<InnerS, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.a1[uniforms.i] = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<InnerS, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructArray_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.a1[uniforms.i] = v;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct OuterS {
  a1 : array<InnerS, 8>,
};

struct InnerS {
  v : i32,
};

struct Uniforms {
  i : u32,
};
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct OuterS {
  a1 : array<InnerS, 8>,
}

struct InnerS {
  v : i32,
}

struct Uniforms {
  i : u32,
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructStructArray) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct S1 {
  a : array<InnerS, 8>,
};

struct OuterS {
  s2 : S1,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.s2.a[uniforms.i] = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct S1 {
  a : array<InnerS, 8>,
}

struct OuterS {
  s2 : S1,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.s2.a);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructStructArray_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.s2.a[uniforms.i] = v;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct OuterS {
  s2 : S1,
};

struct S1 {
  a : array<InnerS, 8>,
};

struct InnerS {
  v : i32,
};

struct Uniforms {
  i : u32,
};
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.s2.a);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct OuterS {
  s2 : S1,
}

struct S1 {
  a : array<InnerS, 8>,
}

struct InnerS {
  v : i32,
}

struct Uniforms {
  i : u32,
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructArrayArray) {
    auto* src = R"(
struct Uniforms {
  i : u32,
  j : u32,
};

struct InnerS {
  v : i32,
};

struct OuterS {
  a1 : array<array<InnerS, 8>, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.a1[uniforms.i][uniforms.j] = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
  j : u32,
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<array<InnerS, 8>, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i][uniforms.j] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructArrayStruct) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct S1 {
  s2 : InnerS,
};

struct OuterS {
  a1 : array<S1, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.a1[uniforms.i].s2 = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct S1 {
  s2 : InnerS,
}

struct OuterS {
  a1 : array<S1, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i].s2 = v;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructArrayStructArray) {
    auto* src = R"(
struct Uniforms {
  i : u32,
  j : u32,
};

struct InnerS {
  v : i32,
};

struct S1 {
  a2 : array<InnerS, 8>,
};

struct OuterS {
  a1 : array<S1, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  s.a1[uniforms.i].a2[uniforms.j] = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
  j : u32,
}

struct InnerS {
  v : i32,
}

struct S1 {
  a2 : array<InnerS, 8>,
}

struct OuterS {
  a1 : array<S1, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  {
    let tint_symbol = &(s.a1);
    var tint_symbol_1 = *(tint_symbol);
    let tint_symbol_2 = &(tint_symbol_1[uniforms.i].a2);
    var tint_symbol_3 = *(tint_symbol_2);
    tint_symbol_3[uniforms.j] = v;
    *(tint_symbol_2) = tint_symbol_3;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, IndexingWithSideEffectFunc) {
    auto* src = R"(
struct Uniforms {
  i : u32,
  j : u32,
};

struct InnerS {
  v : i32,
};

struct S1 {
  a2 : array<InnerS, 8>,
};

struct OuterS {
  a1 : array<S1, 8>,
};

var<private> nextIndex : u32;
fn getNextIndex() -> u32 {
  nextIndex = nextIndex + 1u;
  return nextIndex;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  s.a1[getNextIndex()].a2[uniforms.j] = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
  j : u32,
}

struct InnerS {
  v : i32,
}

struct S1 {
  a2 : array<InnerS, 8>,
}

struct OuterS {
  a1 : array<S1, 8>,
}

var<private> nextIndex : u32;

fn getNextIndex() -> u32 {
  nextIndex = (nextIndex + 1u);
  return nextIndex;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  {
    let tint_symbol = &(s.a1);
    var tint_symbol_1 = *(tint_symbol);
    let tint_symbol_2 = &(tint_symbol_1[getNextIndex()].a2);
    var tint_symbol_3 = *(tint_symbol_2);
    tint_symbol_3[uniforms.j] = v;
    *(tint_symbol_2) = tint_symbol_3;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, IndexingWithSideEffectFunc_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  s.a1[getNextIndex()].a2[uniforms.j] = v;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct Uniforms {
  i : u32,
  j : u32,
};

var<private> nextIndex : u32;
fn getNextIndex() -> u32 {
  nextIndex = nextIndex + 1u;
  return nextIndex;
}

struct OuterS {
  a1 : array<S1, 8>,
};

struct S1 {
  a2 : array<InnerS, 8>,
};

struct InnerS {
  v : i32,
};
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  {
    let tint_symbol = &(s.a1);
    var tint_symbol_1 = *(tint_symbol);
    let tint_symbol_2 = &(tint_symbol_1[getNextIndex()].a2);
    var tint_symbol_3 = *(tint_symbol_2);
    tint_symbol_3[uniforms.j] = v;
    *(tint_symbol_2) = tint_symbol_3;
    *(tint_symbol) = tint_symbol_1;
  }
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct Uniforms {
  i : u32,
  j : u32,
}

var<private> nextIndex : u32;

fn getNextIndex() -> u32 {
  nextIndex = (nextIndex + 1u);
  return nextIndex;
}

struct OuterS {
  a1 : array<S1, 8>,
}

struct S1 {
  a2 : array<InnerS, 8>,
}

struct InnerS {
  v : i32,
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, ViaPointerArg) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};
struct InnerS {
  v : i32,
};
struct OuterS {
  a1 : array<InnerS, 8>,
};
@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  (*p).a1[uniforms.i] = v;
}

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&s1);
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<InnerS, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  {
    let tint_symbol = &((*(p)).a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&(s1));
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, ViaPointerArg_PointerDot) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};
struct InnerS {
  v : i32,
};
struct OuterS {
  a1 : array<InnerS, 8>,
};
@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  p.a1[uniforms.i] = v;
}

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&s1);
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<InnerS, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  {
    let tint_symbol = &((*(p)).a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&(s1));
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, ViaPointerArg_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&s1);
}

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  (*p).a1[uniforms.i] = v;
}

struct InnerS {
  v : i32,
};
struct OuterS {
  a1 : array<InnerS, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct Uniforms {
  i : u32,
};
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&(s1));
}

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  {
    let tint_symbol = &((*(p)).a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<InnerS, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct Uniforms {
  i : u32,
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, ViaPointerVar) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct OuterS {
  a1 : array<InnerS, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, InnerS>, v : InnerS) {
  *(p) = v;
}

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  let p = &(s1.a1[uniforms.i]);
  *(p) = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<InnerS, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, InnerS>, v : InnerS) {
  *(p) = v;
}

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  let p_save = uniforms.i;
  {
    let tint_symbol = &(s1.a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[p_save] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, VectorAssignment) {
    auto* src = R"(
struct Uniforms {
  i : u32,
}

struct OuterS {
  a1 : array<u32, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(i : u32) -> u32 {
  return (i + 1u);
}

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  var v : vec3<f32>;
  v[s1.a1[uniforms.i]] = 1.0;
  v[f(s1.a1[uniforms.i])] = 1.0;
}
)";

    // Transform does nothing here as we're not actually assigning to the array in
    // the struct.
    EXPECT_FALSE(ShouldRun<LocalizeStructArrayAssignment>(src));
}

TEST_F(LocalizeStructArrayAssignmentTest, ArrayStructArray) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct OuterS {
  a1 : array<InnerS, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : array<OuterS, 2>;
  s1[uniforms.i].a1[uniforms.i] = v;
}
)";

    // Transform does nothing as the struct-of-array is in an array, which FXC has no problem with.
    EXPECT_FALSE(ShouldRun<LocalizeStructArrayAssignment>(src));
}

TEST_F(LocalizeStructArrayAssignmentTest, ArrayStructArray_ViaPointerDerefIndex) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct OuterS {
  a1 : array<InnerS, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : array<OuterS, 2>;
  let p = &s1;
  (*p)[uniforms.i].a1[uniforms.i] = v;
}
)";

    // Transform does nothing as the struct-of-array is in an array, which FXC has no problem with.
    EXPECT_FALSE(ShouldRun<LocalizeStructArrayAssignment>(src));
}

TEST_F(LocalizeStructArrayAssignmentTest, ArrayStructArray_ViaPointerIndex) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct OuterS {
  a1 : array<InnerS, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : array<OuterS, 2>;
  let p = &s1;
  p[uniforms.i].a1[uniforms.i] = v;
}
)";

    // Transform does nothing as the struct-of-array is in an array, which FXC has no problem with.
    EXPECT_FALSE(ShouldRun<LocalizeStructArrayAssignment>(src));
}

}  // namespace
}  // namespace tint::hlsl::writer
