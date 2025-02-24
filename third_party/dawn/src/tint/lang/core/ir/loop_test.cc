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

#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_LoopTest = IRTestHelper;
using IR_LoopDeathTest = IR_LoopTest;

TEST_F(IR_LoopTest, Parent) {
    auto* loop = b.Loop();
    EXPECT_EQ(loop->Initializer()->Parent(), loop);
    EXPECT_EQ(loop->Body()->Parent(), loop);
    EXPECT_EQ(loop->Continuing()->Parent(), loop);
}

TEST_F(IR_LoopTest, Result) {
    auto* loop = b.Loop();
    EXPECT_TRUE(loop->Results().IsEmpty());
}

TEST_F(IR_LoopDeathTest, Fail_NullInitializerBlock) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            mod.CreateInstruction<Loop>(nullptr, b.MultiInBlock(), b.MultiInBlock());
        },
        "internal compiler error");
}

TEST_F(IR_LoopDeathTest, Fail_NullBodyBlock) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            mod.CreateInstruction<Loop>(b.Block(), nullptr, b.MultiInBlock());
        },
        "internal compiler error");
}

TEST_F(IR_LoopDeathTest, Fail_NullContinuingBlock) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            mod.CreateInstruction<Loop>(b.Block(), b.MultiInBlock(), nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_LoopTest, Clone) {
    auto* loop = b.Loop();
    auto* new_loop = clone_ctx.Clone(loop);

    EXPECT_NE(loop, new_loop);
    EXPECT_TRUE(new_loop->Results().IsEmpty());
    EXPECT_EQ(0u, new_loop->Exits().Count());
    EXPECT_NE(nullptr, new_loop->Initializer());
    EXPECT_NE(loop->Initializer(), new_loop->Initializer());

    EXPECT_NE(nullptr, new_loop->Body());
    EXPECT_NE(loop->Body(), new_loop->Body());

    EXPECT_NE(nullptr, new_loop->Continuing());
    EXPECT_NE(loop->Continuing(), new_loop->Continuing());
}

TEST_F(IR_LoopTest, CloneWithExits) {
    Loop* new_loop = nullptr;
    {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* if_ = b.If(true);
            b.Append(if_->True(), [&] { b.Continue(loop); });
            b.Append(if_->False(), [&] { b.ExitLoop(loop); });
            b.Append(loop->Continuing(), [&] { b.BreakIf(loop, false); });

            b.NextIteration(loop);
        });
        new_loop = clone_ctx.Clone(loop);
    }

    ASSERT_EQ(2u, new_loop->Body()->Length());
    EXPECT_TRUE(new_loop->Body()->Front()->Is<If>());

    auto* new_if = new_loop->Body()->Front()->As<If>();
    ASSERT_EQ(1u, new_if->True()->Length());
    EXPECT_TRUE(new_if->True()->Front()->Is<Continue>());
    EXPECT_EQ(new_loop, new_if->True()->Front()->As<Continue>()->Loop());

    ASSERT_EQ(1u, new_if->False()->Length());
    EXPECT_TRUE(new_if->False()->Front()->Is<ExitLoop>());
    EXPECT_EQ(new_loop, new_if->False()->Front()->As<ExitLoop>()->Loop());

    ASSERT_EQ(1u, new_loop->Continuing()->Length());
    EXPECT_TRUE(new_loop->Continuing()->Front()->Is<BreakIf>());
    EXPECT_EQ(new_loop, new_loop->Continuing()->Front()->As<BreakIf>()->Loop());

    EXPECT_TRUE(new_loop->Body()->Back()->Is<NextIteration>());
    EXPECT_EQ(new_loop, new_loop->Body()->Back()->As<NextIteration>()->Loop());
}

TEST_F(IR_LoopTest, CloneWithResults) {
    Loop* new_loop = nullptr;
    auto* r0 = b.InstructionResult(ty.i32());
    auto* r1 = b.InstructionResult(ty.f32());
    {
        auto* loop = b.Loop();
        loop->SetResults(Vector{r0, r1});
        b.Append(loop->Body(), [&] { b.ExitLoop(loop, b.Constant(42_i), b.Constant(42_f)); });
        new_loop = clone_ctx.Clone(loop);
    }

    ASSERT_EQ(2u, new_loop->Results().Length());
    auto* new_r0 = new_loop->Results()[0]->As<InstructionResult>();
    ASSERT_NE(new_r0, nullptr);
    ASSERT_NE(new_r0, r0);
    EXPECT_EQ(new_r0->Type(), ty.i32());
    auto* new_r1 = new_loop->Results()[1]->As<InstructionResult>();
    ASSERT_NE(new_r1, nullptr);
    ASSERT_NE(new_r1, r1);
    EXPECT_EQ(new_r1->Type(), ty.f32());
}

}  // namespace
}  // namespace tint::core::ir
