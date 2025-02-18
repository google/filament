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

#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_MultiInBlockTest = IRTestHelper;
using IR_MultiInBlockDeathTest = IR_MultiInBlockTest;

TEST_F(IR_MultiInBlockDeathTest, Fail_NullInboundBranch) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};

            auto* blk = b.MultiInBlock();
            blk->AddInboundSiblingBranch(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_MultiInBlockTest, CloneInto) {
    auto* loop = b.Loop();

    auto* blk = b.MultiInBlock();
    auto* add = b.Add(mod.Types().i32(), 1_i, 2_i);
    blk->Append(add);
    auto* param1 = b.BlockParam(mod.Types().i32());
    auto* param2 = b.BlockParam(mod.Types().f32());
    blk->SetParams({param1, param2});
    blk->SetParent(loop);

    auto* terminate = b.TerminateInvocation();
    blk->AddInboundSiblingBranch(terminate);

    auto* new_blk = b.MultiInBlock();
    blk->CloneInto(clone_ctx, new_blk);

    EXPECT_EQ(0u, new_blk->InboundSiblingBranches().Length());

    EXPECT_EQ(2u, new_blk->Params().Length());
    EXPECT_EQ(mod.Types().i32(), new_blk->Params()[0]->Type());
    EXPECT_EQ(mod.Types().f32(), new_blk->Params()[1]->Type());

    EXPECT_EQ(nullptr, new_blk->Parent());

    EXPECT_EQ(1u, new_blk->Length());
    EXPECT_NE(add, new_blk->Front());
    EXPECT_TRUE(new_blk->Front()->Is<Binary>());
    EXPECT_EQ(BinaryOp::kAdd, new_blk->Front()->As<Binary>()->Op());

    // Check parameter ownership is correct.
    EXPECT_EQ(param1->Block(), blk);
    EXPECT_EQ(param2->Block(), blk);
    EXPECT_EQ(new_blk->Params()[0]->Block(), new_blk);
    EXPECT_EQ(new_blk->Params()[1]->Block(), new_blk);
}

TEST_F(IR_MultiInBlockTest, CloneEmpty) {
    auto* blk = b.MultiInBlock();
    auto* new_blk = b.MultiInBlock();
    blk->CloneInto(clone_ctx, new_blk);

    EXPECT_EQ(0u, new_blk->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, new_blk->Params().Length());
}

TEST_F(IR_MultiInBlockTest, Parameters) {
    auto* blk = b.MultiInBlock();

    auto* param1 = b.BlockParam("a", mod.Types().i32());
    auto* param2 = b.BlockParam("b", mod.Types().f32());
    auto* param3 = b.BlockParam("b", mod.Types().f32());

    blk->SetParams({param1, param2});
    EXPECT_EQ(param1->Block(), blk);
    EXPECT_EQ(param2->Block(), blk);
    EXPECT_EQ(param3->Block(), nullptr);

    blk->SetParams({param1, param3});
    EXPECT_EQ(param1->Block(), blk);
    EXPECT_EQ(param2->Block(), nullptr);
    EXPECT_EQ(param3->Block(), blk);
}

}  // namespace
}  // namespace tint::core::ir
