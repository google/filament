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

#include "src/tint/lang/hlsl/writer/ast_raise/calculate_array_length.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"

namespace tint::hlsl::writer {
namespace {

using CalculateArrayLengthTest = ast::transform::TransformTest;
using Unshadow = ast::transform::Unshadow;
using SimplifyPointers = ast::transform::SimplifyPointers;

TEST_F(CalculateArrayLengthTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<CalculateArrayLength>(src));
}

TEST_F(CalculateArrayLengthTest, ShouldRunNoArrayLength) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
}
)";

    EXPECT_FALSE(ShouldRun<CalculateArrayLength>(src));
}

TEST_F(CalculateArrayLengthTest, ShouldRunWithArrayLength) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb.arr);
}
)";

    EXPECT_TRUE(ShouldRun<CalculateArrayLength>(src));
}

TEST_F(CalculateArrayLengthTest, BasicArray) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> sb : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<i32>, read>, result : ptr<function, u32>)

@group(0) @binding(0) var<storage, read> sb : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb), &(tint_symbol_1));
  let tint_symbol_2 : u32 = (tint_symbol_1 / 4u);
  var len : u32 = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, BasicInStruct) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb.arr);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read>, result : ptr<function, u32>)

struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var len : u32 = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, BasicInStruct_ViaPointerDot) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  let p = &sb;
  var len : u32 = arrayLength(&p.arr);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read>, result : ptr<function, u32>)

struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var len : u32 = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, ArrayOfStruct) {
    auto* src = R"(
struct S {
  f : f32,
}

@group(0) @binding(0) var<storage, read> arr : array<S>;

@compute @workgroup_size(1)
fn main() {
  let len = arrayLength(&arr);
}
)";
    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<S>, read>, result : ptr<function, u32>)

struct S {
  f : f32,
}

@group(0) @binding(0) var<storage, read> arr : array<S>;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(arr), &(tint_symbol_1));
  let tint_symbol_2 : u32 = (tint_symbol_1 / 4u);
  let len = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, ArrayOfArrayOfStruct) {
    auto* src = R"(
struct S {
  f : f32,
}

@group(0) @binding(0) var<storage, read> arr : array<array<S, 4>>;

@compute @workgroup_size(1)
fn main() {
  let len = arrayLength(&arr);
}
)";
    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<array<S, 4u>>, read>, result : ptr<function, u32>)

struct S {
  f : f32,
}

@group(0) @binding(0) var<storage, read> arr : array<array<S, 4>>;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(arr), &(tint_symbol_1));
  let tint_symbol_2 : u32 = (tint_symbol_1 / 16u);
  let len = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, InSameBlock) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> sb : array<i32>;;

@compute @workgroup_size(1)
fn main() {
  var a : u32 = arrayLength(&sb);
  var b : u32 = arrayLength(&sb);
  var c : u32 = arrayLength(&sb);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<i32>, read>, result : ptr<function, u32>)

@group(0) @binding(0) var<storage, read> sb : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb), &(tint_symbol_1));
  let tint_symbol_2 : u32 = (tint_symbol_1 / 4u);
  var a : u32 = tint_symbol_2;
  var b : u32 = tint_symbol_2;
  var c : u32 = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, InSameBlock_Struct) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var a : u32 = arrayLength(&sb.arr);
  var b : u32 = arrayLength(&sb.arr);
  var c : u32 = arrayLength(&sb.arr);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read>, result : ptr<function, u32>)

struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var a : u32 = tint_symbol_2;
  var b : u32 = tint_symbol_2;
  var c : u32 = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, Nested) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  if (true) {
    var len : u32 = arrayLength(&sb.arr);
  } else {
    if (true) {
      var len : u32 = arrayLength(&sb.arr);
    }
  }
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read>, result : ptr<function, u32>)

struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  if (true) {
    var tint_symbol_1 : u32 = 0u;
    tint_symbol(&(sb), &(tint_symbol_1));
    let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
    var len : u32 = tint_symbol_2;
  } else {
    if (true) {
      var tint_symbol_3 : u32 = 0u;
      tint_symbol(&(sb), &(tint_symbol_3));
      let tint_symbol_4 : u32 = ((tint_symbol_3 - 4u) / 4u);
      var len : u32 = tint_symbol_4;
    }
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, MultipleStorageBuffers) {
    auto* src = R"(
struct SB1 {
  x : i32,
  arr1 : array<i32>,
};

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
};

@group(0) @binding(0) var<storage, read> sb1 : SB1;

@group(0) @binding(1) var<storage, read> sb2 : SB2;

@group(0) @binding(2) var<storage, read> sb3 : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var len1 : u32 = arrayLength(&(sb1.arr1));
  var len2 : u32 = arrayLength(&(sb2.arr2));
  var len3 : u32 = arrayLength(&sb3);
  var x : u32 = (len1 + len2 + len3);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB1, read>, result : ptr<function, u32>)

@internal(intrinsic_buffer_size)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB2, read>, result : ptr<function, u32>)

@internal(intrinsic_buffer_size)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<i32>, read>, result : ptr<function, u32>)

struct SB1 {
  x : i32,
  arr1 : array<i32>,
}

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
}

@group(0) @binding(0) var<storage, read> sb1 : SB1;

@group(0) @binding(1) var<storage, read> sb2 : SB2;

@group(0) @binding(2) var<storage, read> sb3 : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb1), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var tint_symbol_4 : u32 = 0u;
  tint_symbol_3(&(sb2), &(tint_symbol_4));
  let tint_symbol_5 : u32 = ((tint_symbol_4 - 16u) / 16u);
  var tint_symbol_7 : u32 = 0u;
  tint_symbol_6(&(sb3), &(tint_symbol_7));
  let tint_symbol_8 : u32 = (tint_symbol_7 / 4u);
  var len1 : u32 = tint_symbol_2;
  var len2 : u32 = tint_symbol_5;
  var len3 : u32 = tint_symbol_8;
  var x : u32 = ((len1 + len2) + len3);
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, Shadowing) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> a : SB;
@group(0) @binding(1) var<storage, read> b : SB;

@compute @workgroup_size(1)
fn main() {
  let x = &a;
  var a : u32 = arrayLength(&a.arr);
  {
    var b : u32 = arrayLength(&((*x).arr));
  }
}
)";

    auto* expect =
        R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read>, result : ptr<function, u32>)

struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> a : SB;

@group(0) @binding(1) var<storage, read> b : SB;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(a), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var a_1 : u32 = tint_symbol_2;
  {
    var tint_symbol_3 : u32 = 0u;
    tint_symbol(&(a), &(tint_symbol_3));
    let tint_symbol_4 : u32 = ((tint_symbol_3 - 4u) / 4u);
    var b_1 : u32 = tint_symbol_4;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var len1 : u32 = arrayLength(&(sb1.arr1));
  var len2 : u32 = arrayLength(&(sb2.arr2));
  var len3 : u32 = arrayLength(&sb3);
  var x : u32 = (len1 + len2 + len3);
}

@group(0) @binding(0) var<storage, read> sb1 : SB1;

struct SB1 {
  x : i32,
  arr1 : array<i32>,
};

@group(0) @binding(1) var<storage, read> sb2 : SB2;

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
};

@group(0) @binding(2) var<storage, read> sb3 : array<i32>;
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB1, read>, result : ptr<function, u32>)

@internal(intrinsic_buffer_size)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB2, read>, result : ptr<function, u32>)

@internal(intrinsic_buffer_size)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<i32>, read>, result : ptr<function, u32>)

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb1), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var tint_symbol_4 : u32 = 0u;
  tint_symbol_3(&(sb2), &(tint_symbol_4));
  let tint_symbol_5 : u32 = ((tint_symbol_4 - 16u) / 16u);
  var tint_symbol_7 : u32 = 0u;
  tint_symbol_6(&(sb3), &(tint_symbol_7));
  let tint_symbol_8 : u32 = (tint_symbol_7 / 4u);
  var len1 : u32 = tint_symbol_2;
  var len2 : u32 = tint_symbol_5;
  var len3 : u32 = tint_symbol_8;
  var x : u32 = ((len1 + len2) + len3);
}

@group(0) @binding(0) var<storage, read> sb1 : SB1;

struct SB1 {
  x : i32,
  arr1 : array<i32>,
}

@group(0) @binding(1) var<storage, read> sb2 : SB2;

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
}

@group(0) @binding(2) var<storage, read> sb3 : array<i32>;
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::hlsl::writer
