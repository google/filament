// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/raise/binary_polyfill.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

using MslWriter_BinaryPolyfillTest = core::ir::transform::TransformTest;

TEST_F(MslWriter_BinaryPolyfillTest, FMod_Scalar) {
    auto* lhs = b.FunctionParam<f32>("lhs");
    auto* rhs = b.FunctionParam<f32>("rhs");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Modulo<f32>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:f32, %rhs:f32):f32 {
  $B1: {
    %4:f32 = mod %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:f32, %rhs:f32):f32 {
  $B1: {
    %4:f32 = msl.fmod %lhs, %rhs
    ret %4
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, FMod_Vector) {
    auto* lhs = b.FunctionParam<vec4<f16>>("lhs");
    auto* rhs = b.FunctionParam<vec4<f16>>("rhs");
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Modulo<vec4<f16>>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:vec4<f16>, %rhs:vec4<f16>):vec4<f16> {
  $B1: {
    %4:vec4<f16> = mod %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:vec4<f16>, %rhs:vec4<f16>):vec4<f16> {
  $B1: {
    %4:vec4<f16> = msl.fmod %lhs, %rhs
    ret %4
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, NoModify_I32And) {
    auto* lhs = b.FunctionParam<i32>("lhs");
    auto* rhs = b.FunctionParam<i32>("rhs");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.And<i32>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %4:i32 = and %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, BoolAnd_Scalar) {
    auto* lhs = b.FunctionParam<bool>("lhs");
    auto* rhs = b.FunctionParam<bool>("rhs");
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.And<bool>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:bool, %rhs:bool):bool {
  $B1: {
    %4:bool = and %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:bool, %rhs:bool):bool {
  $B1: {
    %4:u32 = convert %lhs
    %5:u32 = convert %rhs
    %6:u32 = and %4, %5
    %7:bool = convert %6
    ret %7
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, BoolAnd_Vector) {
    auto* lhs = b.FunctionParam<vec4<bool>>("lhs");
    auto* rhs = b.FunctionParam<vec4<bool>>("rhs");
    auto* func = b.Function("foo", ty.vec4<bool>());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.And<vec4<bool>>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:vec4<bool>, %rhs:vec4<bool>):vec4<bool> {
  $B1: {
    %4:vec4<bool> = and %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:vec4<bool>, %rhs:vec4<bool>):vec4<bool> {
  $B1: {
    %4:vec4<u32> = convert %lhs
    %5:vec4<u32> = convert %rhs
    %6:vec4<u32> = and %4, %5
    %7:vec4<bool> = convert %6
    ret %7
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, BoolOr_Scalar) {
    auto* lhs = b.FunctionParam<bool>("lhs");
    auto* rhs = b.FunctionParam<bool>("rhs");
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Or<bool>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:bool, %rhs:bool):bool {
  $B1: {
    %4:bool = or %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:bool, %rhs:bool):bool {
  $B1: {
    %4:u32 = convert %lhs
    %5:u32 = convert %rhs
    %6:u32 = or %4, %5
    %7:bool = convert %6
    ret %7
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, BoolOr_Vector) {
    auto* lhs = b.FunctionParam<vec4<bool>>("lhs");
    auto* rhs = b.FunctionParam<vec4<bool>>("rhs");
    auto* func = b.Function("foo", ty.vec4<bool>());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Or<vec4<bool>>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:vec4<bool>, %rhs:vec4<bool>):vec4<bool> {
  $B1: {
    %4:vec4<bool> = or %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:vec4<bool>, %rhs:vec4<bool>):vec4<bool> {
  $B1: {
    %4:vec4<u32> = convert %lhs
    %5:vec4<u32> = convert %rhs
    %6:vec4<u32> = or %4, %5
    %7:vec4<bool> = convert %6
    ret %7
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, IntAdd_Scalar) {
    auto* lhs = b.FunctionParam<i32>("lhs");
    auto* rhs = b.FunctionParam<i32>("rhs");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Add<i32>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %4:i32 = add %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %4:u32 = bitcast %lhs
    %5:u32 = bitcast %rhs
    %6:u32 = add %4, %5
    %7:i32 = bitcast %6
    ret %7
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, IntMul_Scalar) {
    auto* lhs = b.FunctionParam<i32>("lhs");
    auto* rhs = b.FunctionParam<i32>("rhs");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Multiply<i32>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %4:i32 = mul %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %4:u32 = bitcast %lhs
    %5:u32 = bitcast %rhs
    %6:u32 = mul %4, %5
    %7:i32 = bitcast %6
    ret %7
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, IntSub_Scalar) {
    auto* lhs = b.FunctionParam<i32>("lhs");
    auto* rhs = b.FunctionParam<i32>("rhs");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Subtract<i32>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %4:i32 = sub %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %4:u32 = bitcast %lhs
    %5:u32 = bitcast %rhs
    %6:u32 = sub %4, %5
    %7:i32 = bitcast %6
    ret %7
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, IntAdd_Vector) {
    auto* lhs = b.FunctionParam<vec4<i32>>("lhs");
    auto* rhs = b.FunctionParam<vec4<i32>>("rhs");
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Add<vec4<i32>>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:vec4<i32>, %rhs:vec4<i32>):vec4<i32> {
  $B1: {
    %4:vec4<i32> = add %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:vec4<i32>, %rhs:vec4<i32>):vec4<i32> {
  $B1: {
    %4:vec4<u32> = bitcast %lhs
    %5:vec4<u32> = bitcast %rhs
    %6:vec4<u32> = add %4, %5
    %7:vec4<i32> = bitcast %6
    ret %7
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, IntAdd_ScalarVector) {
    auto* lhs = b.FunctionParam<i32>("lhs");
    auto* rhs = b.FunctionParam<vec4<i32>>("rhs");
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Add<vec4<i32>>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:i32, %rhs:vec4<i32>):vec4<i32> {
  $B1: {
    %4:vec4<i32> = add %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:vec4<i32>):vec4<i32> {
  $B1: {
    %4:u32 = bitcast %lhs
    %5:vec4<u32> = bitcast %rhs
    %6:vec4<u32> = add %4, %5
    %7:vec4<i32> = bitcast %6
    ret %7
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, IntAdd_VectorScalar) {
    auto* lhs = b.FunctionParam<vec4<i32>>("lhs");
    auto* rhs = b.FunctionParam<i32>("rhs");
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.Add<vec4<i32>>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:vec4<i32>, %rhs:i32):vec4<i32> {
  $B1: {
    %4:vec4<i32> = add %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:vec4<i32>, %rhs:i32):vec4<i32> {
  $B1: {
    %4:vec4<u32> = bitcast %lhs
    %5:u32 = bitcast %rhs
    %6:vec4<u32> = add %4, %5
    %7:vec4<i32> = bitcast %6
    ret %7
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, IntShift_Scalar) {
    auto* lhs = b.FunctionParam<i32>("lhs");
    auto* rhs = b.FunctionParam<u32>("rhs");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.ShiftLeft<i32>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:i32, %rhs:u32):i32 {
  $B1: {
    %4:i32 = shl %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:u32):i32 {
  $B1: {
    %4:u32 = bitcast %lhs
    %5:u32 = shl %4, %rhs
    %6:i32 = bitcast %5
    ret %6
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BinaryPolyfillTest, IntShift_Vector) {
    auto* lhs = b.FunctionParam<vec4<i32>>("lhs");
    auto* rhs = b.FunctionParam<vec4<u32>>("rhs");
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({lhs, rhs});
    b.Append(func->Block(), [&] {
        auto* result = b.ShiftLeft<vec4<i32>>(lhs, rhs);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%lhs:vec4<i32>, %rhs:vec4<u32>):vec4<i32> {
  $B1: {
    %4:vec4<i32> = shl %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%lhs:vec4<i32>, %rhs:vec4<u32>):vec4<i32> {
  $B1: {
    %4:vec4<u32> = bitcast %lhs
    %5:vec4<u32> = shl %4, %rhs
    %6:vec4<i32> = bitcast %5
    ret %6
  }
}
)";

    Run(BinaryPolyfill);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
