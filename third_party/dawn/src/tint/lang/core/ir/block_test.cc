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
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_BlockTest = IRTestHelper;

TEST_F(IR_BlockTest, Terminator_Empty) {
    auto* blk = b.Block();
    EXPECT_EQ(blk->Terminator(), nullptr);
}

TEST_F(IR_BlockTest, Terminator_None) {
    auto* blk = b.Block();
    blk->Append(b.Add(mod.Types().i32(), 1_u, 2_u));
    EXPECT_EQ(blk->Terminator(), nullptr);
}

TEST_F(IR_BlockTest, Terminator_BreakIf) {
    auto* blk = b.Block();
    auto* loop = b.Loop();
    blk->Append(b.BreakIf(loop, true));
    EXPECT_NE(blk->Terminator(), nullptr);
}

TEST_F(IR_BlockTest, Terminator_Continue) {
    auto* blk = b.Block();
    auto* loop = b.Loop();
    blk->Append(b.Continue(loop));
    EXPECT_NE(blk->Terminator(), nullptr);
}

TEST_F(IR_BlockTest, Terminator_ExitIf) {
    auto* blk = b.Block();
    auto* if_ = b.If(true);
    blk->Append(b.ExitIf(if_));
    EXPECT_NE(blk->Terminator(), nullptr);
}

TEST_F(IR_BlockTest, Terminator_ExitLoop) {
    auto* blk = b.Block();
    auto* loop = b.Loop();
    blk->Append(b.ExitLoop(loop));
    EXPECT_NE(blk->Terminator(), nullptr);
}

TEST_F(IR_BlockTest, Terminator_ExitSwitch) {
    auto* blk = b.Block();
    auto* s = b.Switch(1_u);
    blk->Append(b.ExitSwitch(s));
    EXPECT_NE(blk->Terminator(), nullptr);
}

TEST_F(IR_BlockTest, Terminator_NextIteration) {
    auto* blk = b.Block();
    auto* loop = b.Loop();
    blk->Append(b.NextIteration(loop));
    EXPECT_NE(blk->Terminator(), nullptr);
}

TEST_F(IR_BlockTest, Terminator_Return) {
    auto* f = b.Function("myFunc", mod.Types().void_());

    auto* blk = b.Block();
    blk->Append(b.Return(f));
    EXPECT_NE(blk->Terminator(), nullptr);
}

TEST_F(IR_BlockTest, Append) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* inst3 = b.Loop();

    auto* blk = b.Block();
    EXPECT_EQ(blk->Append(inst1), inst1);
    EXPECT_EQ(blk->Append(inst2), inst2);
    EXPECT_EQ(blk->Append(inst3), inst3);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    ASSERT_EQ(inst3->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(3u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    inst = inst->next;

    ASSERT_EQ(inst, inst3);
    ASSERT_EQ(inst->prev, inst2);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Prepend) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* inst3 = b.Loop();

    auto* blk = b.Block();
    EXPECT_EQ(blk->Prepend(inst3), inst3);
    EXPECT_EQ(blk->Prepend(inst2), inst2);
    EXPECT_EQ(blk->Prepend(inst1), inst1);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    ASSERT_EQ(inst3->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(3u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    inst = inst->next;

    ASSERT_EQ(inst, inst3);
    ASSERT_EQ(inst->prev, inst2);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, InsertBefore_AtStart) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();

    auto* blk = b.Block();
    blk->Append(inst2);
    blk->InsertBefore(inst2, inst1);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(2u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, InsertBefore_Middle) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* inst3 = b.Loop();

    auto* blk = b.Block();
    blk->Append(inst1);
    blk->Append(inst3);
    blk->InsertBefore(inst3, inst2);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    ASSERT_EQ(inst3->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(3u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    inst = inst->next;

    ASSERT_EQ(inst, inst3);
    ASSERT_EQ(inst->prev, inst2);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, InsertAfter_AtEnd) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();

    auto* blk = b.Block();
    blk->Append(inst1);
    blk->InsertAfter(inst1, inst2);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(2u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, InsertAfter_Middle) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* inst3 = b.Loop();

    auto* blk = b.Block();
    blk->Append(inst1);
    blk->Append(inst3);
    blk->InsertAfter(inst1, inst2);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    ASSERT_EQ(inst3->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(3u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    inst = inst->next;

    ASSERT_EQ(inst, inst3);
    ASSERT_EQ(inst->prev, inst2);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Replace_Middle) {
    auto* blk = b.Block();
    auto* inst1 = blk->Append(b.Loop());
    auto* inst4 = blk->Append(b.Loop());
    auto* inst3 = blk->Append(b.Loop());

    auto* inst2 = b.Loop();
    blk->Replace(inst4, inst2);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    ASSERT_EQ(inst3->Block(), blk);
    EXPECT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(3u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    inst = inst->next;

    ASSERT_EQ(inst, inst3);
    ASSERT_EQ(inst->prev, inst2);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Replace_Start) {
    auto* blk = b.Block();
    auto* inst4 = blk->Append(b.Loop());
    auto* inst2 = blk->Append(b.Loop());

    auto* inst1 = b.Loop();
    blk->Replace(inst4, inst1);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    EXPECT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(2u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Replace_End) {
    auto* blk = b.Block();
    auto* inst1 = blk->Append(b.Loop());
    auto* inst4 = blk->Append(b.Loop());

    auto* inst2 = b.Loop();
    blk->Replace(inst4, inst2);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    EXPECT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(2u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Replace_OnlyNode) {
    auto* blk = b.Block();
    auto* inst4 = blk->Append(b.Loop());

    auto* inst1 = b.Loop();
    blk->Replace(inst4, inst1);

    ASSERT_EQ(inst1->Block(), blk);
    EXPECT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(1u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Remove_Middle) {
    auto* blk = b.Block();
    auto* inst1 = blk->Append(b.Loop());
    auto* inst4 = blk->Append(b.Loop());
    auto* inst2 = blk->Append(b.Loop());
    blk->Remove(inst4);

    ASSERT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(2u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Remove_Start) {
    auto* blk = b.Block();
    auto* inst4 = blk->Append(b.Loop());
    auto* inst1 = blk->Append(b.Loop());
    blk->Remove(inst4);

    ASSERT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(1u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Remove_End) {
    auto* blk = b.Block();
    auto* inst1 = blk->Append(b.Loop());
    auto* inst4 = blk->Append(b.Loop());
    blk->Remove(inst4);

    ASSERT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(1u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Remove_OnlyNode) {
    auto* blk = b.Block();
    auto* inst4 = blk->Append(b.Loop());
    blk->Remove(inst4);

    ASSERT_EQ(inst4->Block(), nullptr);

    EXPECT_TRUE(blk->IsEmpty());
    EXPECT_EQ(0u, blk->Length());
}

}  // namespace
}  // namespace tint::core::ir
