// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/signed_integer_polyfill.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {
namespace {

using IR_SignedIntegerPolyfillTest = core::ir::transform::TransformTest;

TEST_F(IR_SignedIntegerPolyfillTest, Negation_F32) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Negation<f32>(value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:f32):f32 {
  $B1: {
    %3:f32 = negation %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{.signed_negation = true};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Negation_I32_Scalar) {
    auto* value = b.FunctionParam<i32>("value");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Negation<i32>(value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:i32):i32 {
  $B1: {
    %3:i32 = negation %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value:i32):i32 {
  $B1: {
    %3:u32 = bitcast %value
    %4:u32 = complement %3
    %5:u32 = add %4, 1u
    %6:i32 = bitcast %5
    ret %6
  }
}
)";
    SignedIntegerPolyfillConfig cfg{.signed_negation = true};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Disabled_Negation_I32_Scalar) {
    auto* value = b.FunctionParam<i32>("value");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Negation<i32>(value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:i32):i32 {
  $B1: {
    %3:i32 = negation %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Negation_I32_Vector) {
    auto* value = b.FunctionParam<vec4<i32>>("value");
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Negation<vec4<i32>>(value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:vec4<i32>):vec4<i32> {
  $B1: {
    %3:vec4<i32> = negation %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%value:vec4<i32>):vec4<i32> {
  $B1: {
    %3:vec4<u32> = bitcast %value
    %4:vec4<u32> = complement %3
    %5:vec4<u32> = add %4, vec4<u32>(1u)
    %6:vec4<i32> = bitcast %5
    ret %6
  }
}
)";

    SignedIntegerPolyfillConfig cfg{.signed_negation = true};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Disabled_Negation_I32_Vector) {
    auto* value = b.FunctionParam<vec4<i32>>("value");
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Negation<vec4<i32>>(value);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%value:vec4<i32>):vec4<i32> {
  $B1: {
    %3:vec4<i32> = negation %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, IntAdd_Scalar) {
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

    SignedIntegerPolyfillConfig cfg{.signed_arithmetic = true};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Disabled_IntAdd_Scalar) {
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

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, IntMul_Scalar) {
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

    SignedIntegerPolyfillConfig cfg{.signed_arithmetic = true};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Disabled_IntMul_Scalar) {
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

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, IntSub_Scalar) {
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

    SignedIntegerPolyfillConfig cfg{.signed_arithmetic = true};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Disabled_IntSub_Scalar) {
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

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, IntAdd_Vector) {
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

    SignedIntegerPolyfillConfig cfg{.signed_arithmetic = true};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Disabled_IntAdd_Vector) {
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

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, IntAdd_ScalarVector) {
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

    SignedIntegerPolyfillConfig cfg{.signed_arithmetic = true};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Disabled_IntAdd_ScalarVector) {
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

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, IntAdd_VectorScalar) {
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

    SignedIntegerPolyfillConfig cfg{.signed_arithmetic = true};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Disabled_IntAdd_VectorScalar) {
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

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, IntShift_Scalar) {
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

    SignedIntegerPolyfillConfig cfg{.signed_shiftleft = true};
    Run(SignedIntegerPolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Disabled_IntShift_Scalar) {
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

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{};
    Run(SignedIntegerPolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, IntShift_Vector) {
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

    SignedIntegerPolyfillConfig cfg{.signed_shiftleft = true};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SignedIntegerPolyfillTest, Disabled_IntShift_Vector) {
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

    auto* expect = src;
    SignedIntegerPolyfillConfig cfg{};
    Run(SignedIntegerPolyfill, cfg);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
