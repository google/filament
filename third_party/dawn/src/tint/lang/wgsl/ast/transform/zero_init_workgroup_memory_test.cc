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

#include "src/tint/lang/wgsl/ast/transform/zero_init_workgroup_memory.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using ZeroInitWorkgroupMemoryTest = TransformTest;

TEST_F(ZeroInitWorkgroupMemoryTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<ZeroInitWorkgroupMemory>(src));
}

TEST_F(ZeroInitWorkgroupMemoryTest, ShouldRunHasNoWorkgroupVars) {
    auto* src = R"(
var<private> v : i32;
)";

    EXPECT_FALSE(ShouldRun<ZeroInitWorkgroupMemory>(src));
}

TEST_F(ZeroInitWorkgroupMemoryTest, ShouldRunHasWorkgroupVars) {
    auto* src = R"(
var<workgroup> a : i32;
)";

    EXPECT_TRUE(ShouldRun<ZeroInitWorkgroupMemory>(src));
}

TEST_F(ZeroInitWorkgroupMemoryTest, EmptyModule) {
    auto* src = "";
    auto* expect = src;

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, NoWorkgroupVars) {
    auto* src = R"(
var<private> v : i32;

fn f() {
  v = 1;
}
)";
    auto* expect = src;

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, UnreferencedWorkgroupVars) {
    auto* src = R"(
var<workgroup> a : i32;

var<workgroup> b : i32;

var<workgroup> c : i32;

fn unreferenced() {
  b = c;
}

@compute @workgroup_size(1)
fn f() {
}
)";
    auto* expect = src;

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, UnreferencedWorkgroupVars_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
}

fn unreferenced() {
  b = c;
}

var<workgroup> a : i32;

var<workgroup> b : i32;

var<workgroup> c : i32;
)";
    auto* expect = src;

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_ExistingLocalIndex) {
    auto* src = R"(
var<workgroup> v : i32;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = v; // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    v = i32();
  }
  workgroupBarrier();
}

var<workgroup> v : i32;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  tint_zero_workgroup_memory(local_idx);
  _ = v;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_ExistingLocalIndex_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = v; // Initialization should be inserted above this statement
}

var<workgroup> v : i32;
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    v = i32();
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  tint_zero_workgroup_memory(local_idx);
  _ = v;
}

var<workgroup> v : i32;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_ExistingLocalIndexInStruct) {
    auto* src = R"(
var<workgroup> v : i32;

struct Params {
  @builtin(local_invocation_index) local_idx : u32,
};

@compute @workgroup_size(1)
fn f(params : Params) {
  _ = v; // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    v = i32();
  }
  workgroupBarrier();
}

var<workgroup> v : i32;

struct Params {
  @builtin(local_invocation_index)
  local_idx : u32,
}

@compute @workgroup_size(1)
fn f(params : Params) {
  tint_zero_workgroup_memory(params.local_idx);
  _ = v;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_ExistingLocalIndexInStruct_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f(params : Params) {
  _ = v; // Initialization should be inserted above this statement
}

struct Params {
  @builtin(local_invocation_index) local_idx : u32,
};

var<workgroup> v : i32;
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    v = i32();
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f(params : Params) {
  tint_zero_workgroup_memory(params.local_idx);
  _ = v;
}

struct Params {
  @builtin(local_invocation_index)
  local_idx : u32,
}

var<workgroup> v : i32;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_InjectedLocalIndex) {
    auto* src = R"(
var<workgroup> v : i32;

@compute @workgroup_size(1)
fn f() {
  _ = v; // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    v = i32();
  }
  workgroupBarrier();
}

var<workgroup> v : i32;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  _ = v;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_InjectedLocalIndex_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
  _ = v; // Initialization should be inserted above this statement
}

var<workgroup> v : i32;
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    v = i32();
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  _ = v;
}

var<workgroup> v : i32;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex_Size1) {
    auto* src = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    a = i32();
    b.x = i32();
  }
  for(var idx : u32 = local_idx_1; (idx < 8u); idx = (idx + 1u)) {
    let i : u32 = idx;
    b.y[i] = i32();
  }
  for(var idx_1 : u32 = local_idx_1; (idx_1 < 32u); idx_1 = (idx_1 + 1u)) {
    let i_1 : u32 = idx_1;
    c[i_1].x = i32();
  }
  for(var idx_2 : u32 = local_idx_1; (idx_2 < 256u); idx_2 = (idx_2 + 1u)) {
    let i_2 : u32 = (idx_2 / 8u);
    let i : u32 = (idx_2 % 8u);
    c[i_2].y[i] = i32();
  }
  workgroupBarrier();
}

struct S {
  x : i32,
  y : array<i32, 8>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  tint_zero_workgroup_memory(local_idx);
  _ = a;
  _ = b;
  _ = c;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex_Size1_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
};
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    a = i32();
    b.x = i32();
  }
  for(var idx : u32 = local_idx_1; (idx < 8u); idx = (idx + 1u)) {
    let i : u32 = idx;
    b.y[i] = i32();
  }
  for(var idx_1 : u32 = local_idx_1; (idx_1 < 32u); idx_1 = (idx_1 + 1u)) {
    let i_1 : u32 = idx_1;
    c[i_1].x = i32();
  }
  for(var idx_2 : u32 = local_idx_1; (idx_2 < 256u); idx_2 = (idx_2 + 1u)) {
    let i_2 : u32 = (idx_2 / 8u);
    let i : u32 = (idx_2 % 8u);
    c[i_2].y[i] = i32();
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  tint_zero_workgroup_memory(local_idx);
  _ = a;
  _ = b;
  _ = c;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex_Size_2_3) {
    auto* src = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(2, 3)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    a = i32();
    b.x = i32();
  }
  for(var idx : u32 = local_idx_1; (idx < 8u); idx = (idx + 6u)) {
    let i : u32 = idx;
    b.y[i] = i32();
  }
  for(var idx_1 : u32 = local_idx_1; (idx_1 < 32u); idx_1 = (idx_1 + 6u)) {
    let i_1 : u32 = idx_1;
    c[i_1].x = i32();
  }
  for(var idx_2 : u32 = local_idx_1; (idx_2 < 256u); idx_2 = (idx_2 + 6u)) {
    let i_2 : u32 = (idx_2 / 8u);
    let i : u32 = (idx_2 % 8u);
    c[i_2].y[i] = i32();
  }
  workgroupBarrier();
}

struct S {
  x : i32,
  y : array<i32, 8>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(2, 3)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  tint_zero_workgroup_memory(local_idx);
  _ = a;
  _ = b;
  _ = c;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex_Size_2_3_X) {
    auto* src = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@id(1) override X : i32;

@compute @workgroup_size(2, 3, X)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}
)";
    auto* expect =
        R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  for(var idx : u32 = local_idx_1; (idx < 1u); idx = (idx + (u32(X) * 6u))) {
    a = i32();
    b.x = i32();
  }
  for(var idx_1 : u32 = local_idx_1; (idx_1 < 8u); idx_1 = (idx_1 + (u32(X) * 6u))) {
    let i : u32 = idx_1;
    b.y[i] = i32();
  }
  for(var idx_2 : u32 = local_idx_1; (idx_2 < 32u); idx_2 = (idx_2 + (u32(X) * 6u))) {
    let i_1 : u32 = idx_2;
    c[i_1].x = i32();
  }
  for(var idx_3 : u32 = local_idx_1; (idx_3 < 256u); idx_3 = (idx_3 + (u32(X) * 6u))) {
    let i_2 : u32 = (idx_3 / 8u);
    let i : u32 = (idx_3 % 8u);
    c[i_2].y[i] = i32();
  }
  workgroupBarrier();
}

struct S {
  x : i32,
  y : array<i32, 8>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@id(1) override X : i32;

@compute @workgroup_size(2, 3, X)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  tint_zero_workgroup_memory(local_idx);
  _ = a;
  _ = b;
  _ = c;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex_Size_5u_X_10u) {
    auto* src = R"(
struct S {
  x : array<array<i32, 8>, 10>,
  y : array<i32, 8>,
  z : array<array<array<i32, 8>, 10>, 20>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@id(1) override X : u32;

@compute @workgroup_size(5u, X, 10u)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}
)";
    auto* expect =
        R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  for(var idx : u32 = local_idx_1; (idx < 1u); idx = (idx + (X * 50u))) {
    a = i32();
  }
  for(var idx_1 : u32 = local_idx_1; (idx_1 < 8u); idx_1 = (idx_1 + (X * 50u))) {
    let i_1 : u32 = idx_1;
    b.y[i_1] = i32();
  }
  for(var idx_2 : u32 = local_idx_1; (idx_2 < 80u); idx_2 = (idx_2 + (X * 50u))) {
    let i : u32 = (idx_2 / 8u);
    let i_1 : u32 = (idx_2 % 8u);
    b.x[i][i_1] = i32();
  }
  for(var idx_3 : u32 = local_idx_1; (idx_3 < 256u); idx_3 = (idx_3 + (X * 50u))) {
    let i_4 : u32 = (idx_3 / 8u);
    let i_1 : u32 = (idx_3 % 8u);
    c[i_4].y[i_1] = i32();
  }
  for(var idx_4 : u32 = local_idx_1; (idx_4 < 1600u); idx_4 = (idx_4 + (X * 50u))) {
    let i_2 : u32 = (idx_4 / 80u);
    let i : u32 = ((idx_4 % 80u) / 8u);
    let i_1 : u32 = (idx_4 % 8u);
    b.z[i_2][i][i_1] = i32();
  }
  for(var idx_5 : u32 = local_idx_1; (idx_5 < 2560u); idx_5 = (idx_5 + (X * 50u))) {
    let i_3 : u32 = (idx_5 / 80u);
    let i : u32 = ((idx_5 % 80u) / 8u);
    let i_1 : u32 = (idx_5 % 8u);
    c[i_3].x[i][i_1] = i32();
  }
  for(var idx_6 : u32 = local_idx_1; (idx_6 < 51200u); idx_6 = (idx_6 + (X * 50u))) {
    let i_5 : u32 = (idx_6 / 1600u);
    let i_2 : u32 = ((idx_6 % 1600u) / 80u);
    let i : u32 = ((idx_6 % 80u) / 8u);
    let i_1 : u32 = (idx_6 % 8u);
    c[i_5].z[i_2][i][i_1] = i32();
  }
  workgroupBarrier();
}

struct S {
  x : array<array<i32, 8>, 10>,
  y : array<i32, 8>,
  z : array<array<array<i32, 8>, 10>, 20>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@id(1) override X : u32;

@compute @workgroup_size(5u, X, 10u)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  tint_zero_workgroup_memory(local_idx);
  _ = a;
  _ = b;
  _ = c;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_InjectedLocalIndex) {
    auto* src = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_id) local_invocation_id : vec3<u32>) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    a = i32();
    b.x = i32();
  }
  for(var idx : u32 = local_idx; (idx < 8u); idx = (idx + 1u)) {
    let i : u32 = idx;
    b.y[i] = i32();
  }
  for(var idx_1 : u32 = local_idx; (idx_1 < 32u); idx_1 = (idx_1 + 1u)) {
    let i_1 : u32 = idx_1;
    c[i_1].x = i32();
  }
  for(var idx_2 : u32 = local_idx; (idx_2 < 256u); idx_2 = (idx_2 + 1u)) {
    let i_2 : u32 = (idx_2 / 8u);
    let i : u32 = (idx_2 % 8u);
    c[i_2].y[i] = i32();
  }
  workgroupBarrier();
}

struct S {
  x : i32,
  y : array<i32, 8>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_id) local_invocation_id : vec3<u32>, @builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  _ = a;
  _ = b;
  _ = c;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_InjectedLocalIndex_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_id) local_invocation_id : vec3<u32>) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
};
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    a = i32();
    b.x = i32();
  }
  for(var idx : u32 = local_idx; (idx < 8u); idx = (idx + 1u)) {
    let i : u32 = idx;
    b.y[i] = i32();
  }
  for(var idx_1 : u32 = local_idx; (idx_1 < 32u); idx_1 = (idx_1 + 1u)) {
    let i_1 : u32 = idx_1;
    c[i_1].x = i32();
  }
  for(var idx_2 : u32 = local_idx; (idx_2 < 256u); idx_2 = (idx_2 + 1u)) {
    let i_2 : u32 = (idx_2 / 8u);
    let i : u32 = (idx_2 % 8u);
    c[i_2].y[i] = i32();
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_id) local_invocation_id : vec3<u32>, @builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  _ = a;
  _ = b;
  _ = c;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_MultipleEntryPoints) {
    auto* src = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f1() {
  _ = a; // Initialization should be inserted above this statement
  _ = c;
}

@compute @workgroup_size(1, 2, 3)
fn f2(@builtin(local_invocation_id) local_invocation_id : vec3<u32>) {
  _ = b; // Initialization should be inserted above this statement
}

@compute @workgroup_size(4, 5, 6)
fn f3() {
  _ = c; // Initialization should be inserted above this statement
  _ = a;
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    a = i32();
  }
  for(var idx : u32 = local_idx; (idx < 32u); idx = (idx + 1u)) {
    let i : u32 = idx;
    c[i].x = i32();
  }
  for(var idx_1 : u32 = local_idx; (idx_1 < 256u); idx_1 = (idx_1 + 1u)) {
    let i_1 : u32 = (idx_1 / 8u);
    let i_2 : u32 = (idx_1 % 8u);
    c[i_1].y[i_2] = i32();
  }
  workgroupBarrier();
}

fn tint_zero_workgroup_memory_1(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    b.x = i32();
  }
  for(var idx_2 : u32 = local_idx_1; (idx_2 < 8u); idx_2 = (idx_2 + 6u)) {
    let i_3 : u32 = idx_2;
    b.y[i_3] = i32();
  }
  workgroupBarrier();
}

fn tint_zero_workgroup_memory_2(local_idx_2 : u32) {
  if ((local_idx_2 < 1u)) {
    a = i32();
  }
  if ((local_idx_2 < 32u)) {
    let i_4 : u32 = local_idx_2;
    c[i_4].x = i32();
  }
  for(var idx_3 : u32 = local_idx_2; (idx_3 < 256u); idx_3 = (idx_3 + 120u)) {
    let i_5 : u32 = (idx_3 / 8u);
    let i_6 : u32 = (idx_3 % 8u);
    c[i_5].y[i_6] = i32();
  }
  workgroupBarrier();
}

struct S {
  x : i32,
  y : array<i32, 8>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f1(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  _ = a;
  _ = c;
}

@compute @workgroup_size(1, 2, 3)
fn f2(@builtin(local_invocation_id) local_invocation_id : vec3<u32>, @builtin(local_invocation_index) local_invocation_index_1 : u32) {
  tint_zero_workgroup_memory_1(local_invocation_index_1);
  _ = b;
}

@compute @workgroup_size(4, 5, 6)
fn f3(@builtin(local_invocation_index) local_invocation_index_2 : u32) {
  tint_zero_workgroup_memory_2(local_invocation_index_2);
  _ = c;
  _ = a;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_MultipleEntryPoints_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f1() {
  _ = a; // Initialization should be inserted above this statement
  _ = c;
}

@compute @workgroup_size(1, 2, 3)
fn f2(@builtin(local_invocation_id) local_invocation_id : vec3<u32>) {
  _ = b; // Initialization should be inserted above this statement
}

@compute @workgroup_size(4, 5, 6)
fn f3() {
  _ = c; // Initialization should be inserted above this statement
  _ = a;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
};
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    a = i32();
  }
  for(var idx : u32 = local_idx; (idx < 32u); idx = (idx + 1u)) {
    let i : u32 = idx;
    c[i].x = i32();
  }
  for(var idx_1 : u32 = local_idx; (idx_1 < 256u); idx_1 = (idx_1 + 1u)) {
    let i_1 : u32 = (idx_1 / 8u);
    let i_2 : u32 = (idx_1 % 8u);
    c[i_1].y[i_2] = i32();
  }
  workgroupBarrier();
}

fn tint_zero_workgroup_memory_1(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    b.x = i32();
  }
  for(var idx_2 : u32 = local_idx_1; (idx_2 < 8u); idx_2 = (idx_2 + 6u)) {
    let i_3 : u32 = idx_2;
    b.y[i_3] = i32();
  }
  workgroupBarrier();
}

fn tint_zero_workgroup_memory_2(local_idx_2 : u32) {
  if ((local_idx_2 < 1u)) {
    a = i32();
  }
  if ((local_idx_2 < 32u)) {
    let i_4 : u32 = local_idx_2;
    c[i_4].x = i32();
  }
  for(var idx_3 : u32 = local_idx_2; (idx_3 < 256u); idx_3 = (idx_3 + 120u)) {
    let i_5 : u32 = (idx_3 / 8u);
    let i_6 : u32 = (idx_3 % 8u);
    c[i_5].y[i_6] = i32();
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f1(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  _ = a;
  _ = c;
}

@compute @workgroup_size(1, 2, 3)
fn f2(@builtin(local_invocation_id) local_invocation_id : vec3<u32>, @builtin(local_invocation_index) local_invocation_index_1 : u32) {
  tint_zero_workgroup_memory_1(local_invocation_index_1);
  _ = b;
}

@compute @workgroup_size(4, 5, 6)
fn f3(@builtin(local_invocation_index) local_invocation_index_2 : u32) {
  tint_zero_workgroup_memory_2(local_invocation_index_2);
  _ = c;
  _ = a;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, TransitiveUsage) {
    auto* src = R"(
var<workgroup> v : i32;

fn use_v() {
  _ = v;
}

fn call_use_v() {
  use_v();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  call_use_v(); // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    v = i32();
  }
  workgroupBarrier();
}

var<workgroup> v : i32;

fn use_v() {
  _ = v;
}

fn call_use_v() {
  use_v();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  tint_zero_workgroup_memory(local_idx);
  call_use_v();
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, TransitiveUsage_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  call_use_v(); // Initialization should be inserted above this statement
}

fn call_use_v() {
  use_v();
}

fn use_v() {
  _ = v;
}

var<workgroup> v : i32;
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx_1 : u32) {
  if ((local_idx_1 < 1u)) {
    v = i32();
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  tint_zero_workgroup_memory(local_idx);
  call_use_v();
}

fn call_use_v() {
  use_v();
}

fn use_v() {
  _ = v;
}

var<workgroup> v : i32;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupAtomics) {
    auto* src = R"(
var<workgroup> i : atomic<i32>;
var<workgroup> u : atomic<u32>;

@compute @workgroup_size(1)
fn f() {
  atomicLoad(&(i)); // Initialization should be inserted above this statement
  atomicLoad(&(u));
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    atomicStore(&(i), i32());
    atomicStore(&(u), u32());
  }
  workgroupBarrier();
}

var<workgroup> i : atomic<i32>;

var<workgroup> u : atomic<u32>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  atomicLoad(&(i));
  atomicLoad(&(u));
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
  atomicLoad(&(i)); // Initialization should be inserted above this statement
  atomicLoad(&(u));
}

var<workgroup> i : atomic<i32>;
var<workgroup> u : atomic<u32>;
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    atomicStore(&(i), i32());
    atomicStore(&(u), u32());
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  atomicLoad(&(i));
  atomicLoad(&(u));
}

var<workgroup> i : atomic<i32>;

var<workgroup> u : atomic<u32>;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupStructOfAtomics) {
    auto* src = R"(
struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
};

var<workgroup> w : S;

@compute @workgroup_size(1)
fn f() {
  _ = w.a; // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    w.a = i32();
    atomicStore(&(w.i), i32());
    w.b = f32();
    atomicStore(&(w.u), u32());
    w.c = u32();
  }
  workgroupBarrier();
}

struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
}

var<workgroup> w : S;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  _ = w.a;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupStructOfAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
  _ = w.a; // Initialization should be inserted above this statement
}

var<workgroup> w : S;

struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
};
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    w.a = i32();
    atomicStore(&(w.i), i32());
    w.b = f32();
    atomicStore(&(w.u), u32());
    w.c = u32();
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  _ = w.a;
}

var<workgroup> w : S;

struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArrayOfAtomics) {
    auto* src = R"(
var<workgroup> w : array<atomic<u32>, 4>;

@compute @workgroup_size(1)
fn f() {
  atomicLoad(&w[0]); // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  for(var idx : u32 = local_idx; (idx < 4u); idx = (idx + 1u)) {
    let i : u32 = idx;
    atomicStore(&(w[i]), u32());
  }
  workgroupBarrier();
}

var<workgroup> w : array<atomic<u32>, 4>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  atomicLoad(&(w[0]));
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArrayOfAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
  atomicLoad(&w[0]); // Initialization should be inserted above this statement
}

var<workgroup> w : array<atomic<u32>, 4>;
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  for(var idx : u32 = local_idx; (idx < 4u); idx = (idx + 1u)) {
    let i : u32 = idx;
    atomicStore(&(w[i]), u32());
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  atomicLoad(&(w[0]));
}

var<workgroup> w : array<atomic<u32>, 4>;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArrayOfStructOfAtomics) {
    auto* src = R"(
struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
};

var<workgroup> w : array<S, 4>;

@compute @workgroup_size(1)
fn f() {
  _ = w[0].a; // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  for(var idx : u32 = local_idx; (idx < 4u); idx = (idx + 1u)) {
    let i_1 : u32 = idx;
    w[i_1].a = i32();
    atomicStore(&(w[i_1].i), i32());
    w[i_1].b = f32();
    atomicStore(&(w[i_1].u), u32());
    w[i_1].c = u32();
  }
  workgroupBarrier();
}

struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
}

var<workgroup> w : array<S, 4>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  _ = w[0].a;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArrayOfStructOfAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
  _ = w[0].a; // Initialization should be inserted above this statement
}

var<workgroup> w : array<S, 4>;

struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
};
)";
    auto* expect = R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  for(var idx : u32 = local_idx; (idx < 4u); idx = (idx + 1u)) {
    let i_1 : u32 = idx;
    w[i_1].a = i32();
    atomicStore(&(w[i_1].i), i32());
    w[i_1].b = f32();
    atomicStore(&(w[i_1].u), u32());
    w[i_1].c = u32();
  }
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  tint_zero_workgroup_memory(local_invocation_index);
  _ = w[0].a;
}

var<workgroup> w : array<S, 4>;

struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, ArrayWithOverrideCount) {
    auto* src =
        R"(override O = 123;
alias A = array<i32, O*2>;

var<workgroup> W : A;

@compute @workgroup_size(1)
fn main() {
    let p : ptr<workgroup, A> = &W;
    (*p)[0] = 42;
}
)";

    auto* expect =
        R"(error: array size is an override-expression, when expected a constant-expression.
Was the SubstituteOverride transform run?)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, AliasTypeWithParamName) {
    auto* src =
        R"(
var<workgroup> W : mat2x2<f32>;

@compute @workgroup_size(1) fn F(@builtin(local_invocation_index) mat2x2 : u32) {
  W[0]+=0;
}
)";

    auto* expect =
        R"(
fn tint_zero_workgroup_memory(local_idx : u32) {
  if ((local_idx < 1u)) {
    W = mat2x2<f32>();
  }
  workgroupBarrier();
}

var<workgroup> W : mat2x2<f32>;

@compute @workgroup_size(1)
fn F(@builtin(local_invocation_index) mat2x2 : u32) {
  tint_zero_workgroup_memory(mat2x2);
  W[0] += 0;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
