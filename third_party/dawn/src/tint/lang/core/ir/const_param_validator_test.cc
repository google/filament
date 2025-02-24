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

#include <string>

#include "gtest/gtest.h"

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/const_param_validator.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/diagnostic/source.h"

// These unit tests are used for internal development. CTS validation does a more complete job of
// testing all expectations for const and override parameters.

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_ConstParamValidatorTest : public IRTestHelper {};

TEST_F(IR_ConstParamValidatorTest, CorrectDomainQuantizeF16) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kQuantizeToF16, 65504_f);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %2:f32 = quantizeToF16 65504.0f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainQuantizeF16) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kQuantizeToF16, 65505_f);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %2:f32 = quantizeToF16 65505.0f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              "5:7 error: value 65505.0 cannot be represented as 'f16'");
}

TEST_F(IR_ConstParamValidatorTest, CorrectDomainSubgroupsShuffleXor) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", 777_f);
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kSubgroupShuffleXor, e, 127_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %a:f32 = let 777.0f
    %3:f32 = subgroupShuffleXor %a, 127u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainSubgroupsShuffleXor) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", 777_f);
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kSubgroupShuffleXor, e, 128_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %a:f32 = let 777.0f
    %3:f32 = subgroupShuffleXor %a, 128u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(5:7 error: The mask argument of subgroupShuffleXor must be less than 128)");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainSubgroupsShuffleDown) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", 777_f);
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kSubgroupShuffleDown, e, 128_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %a:f32 = let 777.0f
    %3:f32 = subgroupShuffleDown %a, 128u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(5:7 error: The delta argument of subgroupShuffleDown must be less than 128)");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainSubgroupsShuffle) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", 777_f);
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kSubgroupShuffle, e, 128_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %a:f32 = let 777.0f
    %3:f32 = subgroupShuffle %a, 128u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(5:7 error: The sourceLaneIndex argument of subgroupShuffle must be less than 128)");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainSubgroupsShuffle_SignedHigh) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", 777_f);
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kSubgroupShuffle, e, 128_i);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %a:f32 = let 777.0f
    %3:f32 = subgroupShuffle %a, 128i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(5:7 error: The sourceLaneIndex argument of subgroupShuffle must be less than 128)");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainSubgroupsShuffle_SignedLow) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", 777_f);
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kSubgroupShuffle, e, -128_i);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %a:f32 = let 777.0f
    %3:f32 = subgroupShuffle %a, -128i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(5:7 error: The sourceLaneIndex argument of subgroupShuffle must be greater than or equal to zero)");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainkExtractBits) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", 123_u);
        auto* call_func = b.Call(ty.u32(), core::BuiltinFn::kExtractBits, e, 13_u, 23_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %a:u32 = let 123u
    %3:u32 = extractBits %a, 13u, 23u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              "5:7 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'");
}

TEST_F(IR_ConstParamValidatorTest, CorrectDomainkExtractBits) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", 123_u);
        auto* call_func = b.Call(ty.u32(), core::BuiltinFn::kExtractBits, e, 13_u, 13_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %a:u32 = let 123u
    %3:u32 = extractBits %a, 13u, 13u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainkExtractBits_Vec) {
    auto* func = b.Function("foo", ty.vec4(ty.i32()));
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", b.Splat(ty.vec4<i32>(), -3_i));
        auto* call_func = b.Call(ty.vec4<i32>(), core::BuiltinFn::kExtractBits, e, 13_u, 23_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():vec4<i32> {
  $B1: {
    %a:vec4<i32> = let vec4<i32>(-3i)
    %3:vec4<i32> = extractBits %a, 13u, 23u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              "5:7 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainkInsertBits_Vec) {
    auto* func = b.Function("foo", ty.vec4(ty.u32()));
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", b.Splat(ty.vec4<u32>(), 3_u));
        auto* newBits = b.Let("b", b.Splat(ty.vec4<u32>(), 4_u));
        auto* call_func =
            b.Call(ty.vec4<u32>(), core::BuiltinFn::kInsertBits, e, newBits, 13_u, 23_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():vec4<u32> {
  $B1: {
    %a:vec4<u32> = let vec4<u32>(3u)
    %b:vec4<u32> = let vec4<u32>(4u)
    %4:vec4<u32> = insertBits %a, %b, 13u, 23u
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              "5:7 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainkInsertBits) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", 3_u);
        auto* newBits = b.Let("b", 4_u);
        auto* call_func = b.Call(ty.u32(), core::BuiltinFn::kInsertBits, e, newBits, 13_u, 23_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %a:u32 = let 3u
    %b:u32 = let 4u
    %4:u32 = insertBits %a, %b, 13u, 23u
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              "5:7 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'");
}

TEST_F(IR_ConstParamValidatorTest, CorrectDomainkInsertBits) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e = b.Let("a", 3_u);
        auto* newBits = b.Let("b", 4_u);
        auto* call_func = b.Call(ty.u32(), core::BuiltinFn::kInsertBits, e, newBits, 7_u, 7_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %a:u32 = let 3u
    %b:u32 = let 4u
    %4:u32 = insertBits %a, %b, 7u, 7u
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainClamp) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* val = b.Let("a", 3_f);
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kClamp, val, 2_f, 1_f);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %a:f32 = let 3.0f
    %3:f32 = clamp %a, 2.0f, 1.0f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              "5:7 error: clamp called with 'low' (2.0) greater than 'high' (1.0)");
}

TEST_F(IR_ConstParamValidatorTest, CorrectDomainClamp) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* val = b.Let("a", 3_f);
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kClamp, val, 1_f, 2_f);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %a:f32 = let 3.0f
    %3:f32 = clamp %a, 1.0f, 2.0f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainClamp_Vec) {
    auto* func = b.Function("foo", ty.vec4(ty.f16()));
    b.Append(func->Block(), [&] {
        auto* e = b.Let("b", b.Splat(ty.vec4<f16>(), 4_h));
        auto* low = b.Splat(ty.vec4<f16>(), 4_h);
        auto* high = b.Splat(ty.vec4<f16>(), 2_h);
        auto* call_func = b.Call(ty.vec4(ty.f16()), core::BuiltinFn::kClamp, e, low, high);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():vec4<f16> {
  $B1: {
    %b:vec4<f16> = let vec4<f16>(4.0h)
    %3:vec4<f16> = clamp %b, vec4<f16>(4.0h), vec4<f16>(2.0h)
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              "5:7 error: clamp called with 'low' (4.0) greater than 'high' (2.0)");
}

TEST_F(IR_ConstParamValidatorTest, CorrectDomainSmoothstep) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* val = b.Let("a", 3_f);
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kSmoothstep, val, 1_f, 2_f);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %a:f32 = let 3.0f
    %3:f32 = smoothstep %a, 1.0f, 2.0f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainSmoothstep_Vec) {
    auto* func = b.Function("foo", ty.vec4(ty.f16()));
    b.Append(func->Block(), [&] {
        auto* e = b.Let("b", b.Splat(ty.vec4<f16>(), 4_h));
        auto* edge0 = b.Splat(ty.vec4<f16>(), 3_h);
        auto* edge1 = b.Splat(ty.vec4<f16>(), 3_h);
        auto* call_func = b.Call(ty.vec4(ty.f16()), core::BuiltinFn::kSmoothstep, e, edge0, edge1);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():vec4<f16> {
  $B1: {
    %b:vec4<f16> = let vec4<f16>(4.0h)
    %3:vec4<f16> = smoothstep %b, vec4<f16>(3.0h), vec4<f16>(3.0h)
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              "5:7 error: smoothstep called with 'low' (3.0) equal to 'high' (3.0)");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainLdexp_Vec) {
    auto* func = b.Function("foo", ty.vec4(ty.f32()));
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", b.Splat(ty.vec4<f32>(), 4_f));
        auto* e2 = b.Splat(ty.vec4<i32>(), 267_i);
        auto* call_func = b.Call(ty.vec4(ty.f32()), core::BuiltinFn::kLdexp, e1, e2);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():vec4<f32> {
  $B1: {
    %b:vec4<f32> = let vec4<f32>(4.0f)
    %3:vec4<f32> = ldexp %b, vec4<i32>(267i)
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), "5:7 error: e2 must be less than or equal to 128");
}

TEST_F(IR_ConstParamValidatorTest, CorrectDomainLdexp_Vec) {
    auto* func = b.Function("foo", ty.vec4(ty.f16()));
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", b.Splat(ty.vec4<f16>(), 4_h));
        auto* e2 = b.Splat(ty.vec4<i32>(), 10_i);
        auto* call_func = b.Call(ty.vec4(ty.f16()), core::BuiltinFn::kLdexp, e1, e2);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():vec4<f16> {
  $B1: {
    %b:vec4<f16> = let vec4<f16>(4.0h)
    %3:vec4<f16> = ldexp %b, vec4<i32>(10i)
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainLdexp) {
    auto* func = b.Function("foo", ty.f16());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", 16.0_h);
        auto* call_func = b.Call(ty.f16(), core::BuiltinFn::kLdexp, e1, 17_i);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():f16 {
  $B1: {
    %b:f16 = let 16.0h
    %3:f16 = ldexp %b, 17i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), "5:7 error: e2 must be less than or equal to 16");
}

TEST_F(IR_ConstParamValidatorTest, CorrectDomainLdexp) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", 16.0_f);
        auto* call_func = b.Call(ty.f32(), core::BuiltinFn::kLdexp, e1, 17_i);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %b:f32 = let 16.0f
    %3:f32 = ldexp %b, 17i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectPack2x16Float) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e = b.Splat(ty.vec2<f32>(), 65505_f);
        auto* call_func = b.Call(ty.u32(), core::BuiltinFn::kPack2X16Float, e);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %2:u32 = pack2x16float vec2<f32>(65505.0f)
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              "5:7 error: value 65505.0 cannot be represented as 'f16'");
}

TEST_F(IR_ConstParamValidatorTest, CorrectPack2x16Float) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e = b.Splat(ty.vec2<f32>(), 4.0_f);
        auto* call_func = b.Call(ty.u32(), core::BuiltinFn::kPack2X16Float, e);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %2:u32 = pack2x16float vec2<f32>(4.0f)
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, CorrectDomainShiftLeft) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", 16_u);
        auto* call_func = b.Binary(core::BinaryOp::kShiftLeft, ty.u32(), e1, 17_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %b:u32 = let 16u
    %3:u32 = shl %b, 17u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainShiftLeft) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", 16_u);
        auto* call_func = b.Binary(core::BinaryOp::kShiftLeft, ty.u32(), e1, 32_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %b:u32 = let 16u
    %3:u32 = shl %b, 32u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        "5:7 error: shift left value must be less than the bit width of the lhs, which is 32");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainShiftRight_Vec) {
    auto* func = b.Function("foo", ty.vec4(ty.i32()));
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", b.Splat(ty.vec4<i32>(), 4_i));
        auto* e2 = b.Splat(ty.vec4<u32>(), 33_u);
        auto* call_func = b.Binary(core::BinaryOp::kShiftLeft, ty.vec4(ty.i32()), e1, e2);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():vec4<i32> {
  $B1: {
    %b:vec4<i32> = let vec4<i32>(4i)
    %3:vec4<i32> = shl %b, vec4<u32>(33u)
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        "5:7 error: shift left value must be less than the bit width of the lhs, which is 32");
}

TEST_F(IR_ConstParamValidatorTest, CorrectDomainDiv) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", 16_u);
        auto* call_func = b.Binary(core::BinaryOp::kDivide, ty.u32(), e1, 17_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %b:u32 = let 16u
    %3:u32 = div %b, 17u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainDiv) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", 16_u);
        auto* call_func = b.Binary(core::BinaryOp::kDivide, ty.u32(), e1, 0_u);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %b:u32 = let 16u
    %3:u32 = div %b, 0u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), "5:7 error: integer division by zero is invalid");
}

TEST_F(IR_ConstParamValidatorTest, IncorrectDomainModulo_Vec) {
    auto* func = b.Function("foo", ty.vec4(ty.i32()));
    b.Append(func->Block(), [&] {
        auto* e1 = b.Let("b", b.Splat(ty.vec4<i32>(), 4_i));
        auto* e2 = b.Splat(ty.vec4<i32>(), 0_i);
        auto* call_func = b.Binary(core::BinaryOp::kModulo, ty.vec4(ty.i32()), e1, e2);
        b.ir.SetSource(call_func, Source{{5, 7}});
        b.Return(func, call_func->Result(0));
    });
    ASSERT_EQ(ir::Validate(mod), Success);

    auto* src = R"(
%foo = func():vec4<i32> {
  $B1: {
    %b:vec4<i32> = let vec4<i32>(4i)
    %3:vec4<i32> = mod %b, vec4<i32>(0i)
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto res = ir::ValidateConstParam(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), "5:7 error: integer division by zero is invalid");
}

}  // namespace
}  // namespace tint::core::ir
