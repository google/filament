// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/if.h"
#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_IfTest = IRTestHelper;
using IR_IfDeathTest = IR_IfTest;

TEST_F(IR_IfTest, Usage) {
    auto* cond = b.Constant(true);
    auto* if_ = b.If(cond);
    EXPECT_THAT(cond->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{if_, 0u}));
}

TEST_F(IR_IfTest, Result) {
    auto* if_ = b.If(b.Constant(true));

    EXPECT_TRUE(if_->Results().IsEmpty());
}

TEST_F(IR_IfTest, Parent) {
    auto* cond = b.Constant(true);
    auto* if_ = b.If(cond);
    EXPECT_EQ(if_->True()->Parent(), if_);
    EXPECT_EQ(if_->False()->Parent(), if_);
}

TEST_F(IR_IfDeathTest, Fail_NullTrueBlock) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            mod.CreateInstruction<If>(b.Constant(false), nullptr, b.Block());
        },
        "internal compiler error");
}

TEST_F(IR_IfDeathTest, Fail_NullFalseBlock) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            mod.CreateInstruction<If>(b.Constant(false), b.Block(), nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_IfTest, Clone) {
    auto* if_ = b.If(b.Constant(true));
    auto* new_if = clone_ctx.Clone(if_);

    EXPECT_NE(if_, new_if);

    auto new_cond = new_if->Condition()->As<Constant>()->Value();
    ASSERT_TRUE(new_cond->Is<core::constant::Scalar<bool>>());
    EXPECT_TRUE(new_cond->As<core::constant::Scalar<bool>>()->ValueAs<bool>());

    EXPECT_NE(nullptr, new_if->True());
    EXPECT_NE(nullptr, new_if->False());
    EXPECT_NE(if_->True(), new_if->True());
    EXPECT_NE(if_->False(), new_if->False());
}

TEST_F(IR_IfTest, CloneWithExits) {
    If* new_if = nullptr;
    {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        new_if = clone_ctx.Clone(if_);
    }

    ASSERT_EQ(1u, new_if->True()->Length());
    EXPECT_TRUE(new_if->True()->Front()->Is<ExitIf>());
    EXPECT_EQ(new_if, new_if->True()->Front()->As<ExitIf>()->If());
}

TEST_F(IR_IfTest, CloneWithResults) {
    If* new_if = nullptr;
    auto* r0 = b.InstructionResult(ty.i32());
    auto* r1 = b.InstructionResult(ty.f32());
    {
        auto* if_ = b.If(true);
        if_->SetResults(Vector{r0, r1});
        b.Append(if_->True(), [&] { b.ExitIf(if_, b.Constant(42_i), b.Constant(42_f)); });
        new_if = clone_ctx.Clone(if_);
    }

    ASSERT_EQ(2u, new_if->Results().Length());
    auto* new_r0 = new_if->Results()[0]->As<InstructionResult>();
    ASSERT_NE(new_r0, nullptr);
    ASSERT_NE(new_r0, r0);
    EXPECT_EQ(new_r0->Type(), ty.i32());
    auto* new_r1 = new_if->Results()[1]->As<InstructionResult>();
    ASSERT_NE(new_r1, nullptr);
    ASSERT_NE(new_r1, r1);
    EXPECT_EQ(new_r1->Type(), ty.f32());
}

}  // namespace
}  // namespace tint::core::ir
