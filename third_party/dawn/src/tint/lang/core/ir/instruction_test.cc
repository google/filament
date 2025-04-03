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

#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/module.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir {
namespace {

using IR_InstructionTest = IRTestHelper;
using IR_InstructionDeathTest = IR_InstructionTest;

TEST_F(IR_InstructionTest, InsertBefore) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* blk = b.Block();
    blk->Append(inst2);
    inst1->InsertBefore(inst2);
    EXPECT_EQ(2u, blk->Length());
    EXPECT_EQ(inst1->Block(), blk);
}

TEST_F(IR_InstructionDeathTest, Fail_InsertBeforeNullptr) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            inst1->InsertBefore(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_InstructionDeathTest, Fail_InsertBeforeNotInserted) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            inst1->InsertBefore(inst2);
        },
        "internal compiler error");
}

TEST_F(IR_InstructionTest, InsertAfter) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* blk = b.Block();
    blk->Append(inst2);
    inst1->InsertAfter(inst2);
    EXPECT_EQ(2u, blk->Length());
    EXPECT_EQ(inst1->Block(), blk);
}

TEST_F(IR_InstructionDeathTest, Fail_InsertAfterNullptr) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            inst1->InsertAfter(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_InstructionDeathTest, Fail_InsertAfterNotInserted) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            inst1->InsertAfter(inst2);
        },
        "internal compiler error");
}

TEST_F(IR_InstructionTest, ReplaceWith) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* blk = b.Block();
    blk->Append(inst2);
    inst2->ReplaceWith(inst1);
    EXPECT_EQ(1u, blk->Length());
    EXPECT_EQ(inst1->Block(), blk);
    EXPECT_EQ(inst2->Block(), nullptr);
}

TEST_F(IR_InstructionDeathTest, Fail_ReplaceWithNullptr) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk = b.Block();
            blk->Append(inst1);
            inst1->ReplaceWith(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_InstructionDeathTest, Fail_ReplaceWithNotInserted) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            inst1->ReplaceWith(inst2);
        },
        "internal compiler error");
}

TEST_F(IR_InstructionDeathTest, Fail_Result_ZeroResults) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};

            auto* inst = b.Loop();
            inst->Result();
        },
        "internal compiler error");
}

TEST_F(IR_InstructionDeathTest, Fail_Result_MultipleResults) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};

            auto* inst = b.Loop();
            inst->SetResults(Vector{b.InstructionResult<u32>(), b.InstructionResult<u32>()});
            inst->Result();
        },
        "internal compiler error");
}

TEST_F(IR_InstructionTest, Remove) {
    auto* inst1 = b.Loop();
    auto* blk = b.Block();
    blk->Append(inst1);
    EXPECT_EQ(1u, blk->Length());

    inst1->Remove();
    EXPECT_EQ(0u, blk->Length());
    EXPECT_EQ(inst1->Block(), nullptr);
}

TEST_F(IR_InstructionDeathTest, Fail_RemoveNotInserted) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            inst1->Remove();
        },
        "internal compiler error");
}

TEST_F(IR_InstructionTest, DetachResult) {
    auto* inst = b.Let("foo", 42_u);
    auto* result = inst->Result();
    EXPECT_EQ(result->Instruction(), inst);

    auto* detached = inst->DetachResult();
    EXPECT_EQ(detached, result);
    EXPECT_EQ(detached->Instruction(), nullptr);
    EXPECT_EQ(inst->Results().Length(), 0u);
}

TEST_F(IR_InstructionTest, Comparison) {
    auto* x = b.Let("foo", 42_u);
    auto* y = b.Let("foo", 42_u);

    EXPECT_EQ(*x, *x);
    EXPECT_FALSE(*x == *y);

    EXPECT_TRUE((*x) < (*y));
}

}  // namespace
}  // namespace tint::core::ir
