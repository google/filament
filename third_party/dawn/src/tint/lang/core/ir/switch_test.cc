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

#include "src/tint/lang/core/ir/switch.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using IR_SwitchTest = IRTestHelper;

TEST_F(IR_SwitchTest, Usage) {
    auto* cond = b.Constant(true);
    auto* switch_ = b.Switch(cond);
    EXPECT_THAT(cond->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{switch_, 0u}));
}

TEST_F(IR_SwitchTest, Results) {
    auto* cond = b.Constant(true);
    auto* switch_ = b.Switch(cond);
    EXPECT_TRUE(switch_->Results().IsEmpty());
}

TEST_F(IR_SwitchTest, Parent) {
    auto* switch_ = b.Switch(1_i);
    b.DefaultCase(switch_);
    EXPECT_THAT(switch_->Cases().Front().block->Parent(), switch_);
}

TEST_F(IR_SwitchTest, Clone) {
    auto* switch_ = b.Switch(1_i);
    b.Case(switch_, {nullptr, b.Constant(2_i)});
    b.Case(switch_, {b.Constant(3_i)});

    auto* new_switch = clone_ctx.Clone(switch_);

    EXPECT_NE(switch_, new_switch);

    auto new_cond = new_switch->Condition()->As<Constant>()->Value();
    ASSERT_TRUE(new_cond->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(1_i, new_cond->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    auto& cases = new_switch->Cases();
    ASSERT_EQ(2u, cases.Length());

    {
        auto& case1 = cases[0];
        EXPECT_NE(nullptr, case1.block);
        EXPECT_NE(switch_->Cases()[0].block, case1.block);
        EXPECT_EQ(case1.block->Parent(), new_switch);

        ASSERT_EQ(2u, case1.selectors.Length());
        EXPECT_EQ(nullptr, case1.selectors[0].val);
        auto val = case1.selectors[1].val->Value();
        ASSERT_TRUE(val->Is<core::constant::Scalar<i32>>());
        EXPECT_EQ(2_i, val->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
    }

    {
        auto& case2 = cases[1];
        EXPECT_NE(nullptr, case2.block);
        EXPECT_NE(switch_->Cases()[1].block, case2.block);
        EXPECT_EQ(case2.block->Parent(), new_switch);

        ASSERT_EQ(1u, case2.selectors.Length());
        auto val = case2.selectors[0].val->Value();
        ASSERT_TRUE(val->Is<core::constant::Scalar<i32>>());
        EXPECT_EQ(3_i, val->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
    }
}

TEST_F(IR_SwitchTest, CloneWithExits) {
    Switch* new_switch = nullptr;
    {
        auto* switch_ = b.Switch(1_i);

        auto* blk = b.Case(switch_, {b.Constant(3_i)});
        b.Append(blk, [&] { b.ExitSwitch(switch_); });
        new_switch = clone_ctx.Clone(switch_);
    }

    auto& case_ = new_switch->Cases().Front();
    ASSERT_TRUE(case_.block->Front()->Is<ExitSwitch>());
    EXPECT_EQ(new_switch, case_.block->Front()->As<ExitSwitch>()->Switch());
}

TEST_F(IR_SwitchTest, CloneWithResults) {
    Switch* new_switch = nullptr;
    auto* r0 = b.InstructionResult(ty.i32());
    auto* r1 = b.InstructionResult(ty.f32());
    {
        auto* switch_ = b.Switch(1_i);
        switch_->SetResults(Vector{r0, r1});

        auto* blk = b.Case(switch_, Vector{b.Constant(3_i)});
        b.Append(blk, [&] { b.ExitSwitch(switch_, b.Constant(42_i), b.Constant(42_f)); });
        new_switch = clone_ctx.Clone(switch_);
    }

    ASSERT_EQ(2u, new_switch->Results().Length());
    auto* new_r0 = new_switch->Results()[0]->As<InstructionResult>();
    ASSERT_NE(new_r0, nullptr);
    ASSERT_NE(new_r0, r0);
    EXPECT_EQ(new_r0->Type(), ty.i32());
    auto* new_r1 = new_switch->Results()[1]->As<InstructionResult>();
    ASSERT_NE(new_r1, nullptr);
    ASSERT_NE(new_r1, r1);
    EXPECT_EQ(new_r1->Type(), ty.f32());
}

}  // namespace
}  // namespace tint::core::ir
