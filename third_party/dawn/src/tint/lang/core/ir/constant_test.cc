// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/value.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir {
namespace {

using IR_ConstantTest = IRTestHelper;
using IR_ConstantDeathTest = IR_ConstantTest;

TEST_F(IR_ConstantTest, f32) {
    StringStream str;

    auto* c = b.Constant(1.2_f);
    EXPECT_EQ(1.2_f, c->Value()->As<core::constant::Scalar<f32>>()->ValueAs<f32>());

    EXPECT_TRUE(c->Value()->Is<core::constant::Scalar<f32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f16>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<u32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, f16) {
    StringStream str;

    auto* c = b.Constant(1.1_h);
    EXPECT_EQ(1.1_h, c->Value()->As<core::constant::Scalar<f16>>()->ValueAs<f16>());

    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f32>>());
    EXPECT_TRUE(c->Value()->Is<core::constant::Scalar<f16>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<u32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, i32) {
    StringStream str;

    auto* c = b.Constant(1_i);
    EXPECT_EQ(1_i, c->Value()->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f16>>());
    EXPECT_TRUE(c->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<u32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, i8) {
    StringStream str;

    auto* c = b.Constant(i8(1));
    EXPECT_EQ(i8(1), c->Value()->As<core::constant::Scalar<i8>>()->ValueAs<i8>());

    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f16>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_TRUE(c->Value()->Is<core::constant::Scalar<i8>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<u32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<u8>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, u32) {
    StringStream str;

    auto* c = b.Constant(2_u);
    EXPECT_EQ(2_u, c->Value()->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f16>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_TRUE(c->Value()->Is<core::constant::Scalar<u32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, u64) {
    StringStream str;

    auto* c = b.Constant(u64(2));
    EXPECT_EQ(u64(2), c->Value()->As<core::constant::Scalar<u64>>()->ValueAs<u64>());

    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f16>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<i8>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<u32>>());
    EXPECT_TRUE(c->Value()->Is<core::constant::Scalar<u64>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<u8>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, u8) {
    StringStream str;

    auto* c = b.Constant(u8(2));
    EXPECT_EQ(u8(2), c->Value()->As<core::constant::Scalar<u8>>()->ValueAs<u8>());

    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f16>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<i8>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<u32>>());
    EXPECT_TRUE(c->Value()->Is<core::constant::Scalar<u8>>());
    EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, bool) {
    {
        StringStream str;

        auto* c = b.Constant(false);
        EXPECT_FALSE(c->Value()->As<core::constant::Scalar<bool>>()->ValueAs<bool>());
    }

    {
        StringStream str;
        auto c = b.Constant(true);
        EXPECT_TRUE(c->Value()->As<core::constant::Scalar<bool>>()->ValueAs<bool>());

        EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f32>>());
        EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<f16>>());
        EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<i32>>());
        EXPECT_FALSE(c->Value()->Is<core::constant::Scalar<u32>>());
        EXPECT_TRUE(c->Value()->Is<core::constant::Scalar<bool>>());
    }
}

TEST_F(IR_ConstantDeathTest, Fail_NullValue) {
    EXPECT_DEATH_IF_SUPPORTED({ Constant c(nullptr); }, "internal compiler error");
}

TEST_F(IR_ConstantDeathTest, Fail_Builder_NullValue) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            b.Constant(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_ConstantTest, Clone) {
    auto* c = b.Constant(2_u);
    auto* new_c = clone_ctx.Clone(c);

    EXPECT_EQ(c, new_c);
}

}  // namespace
}  // namespace tint::core::ir
