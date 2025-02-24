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

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir {
namespace {

using IR_ValueTest = IRTestHelper;
using IR_ValueDeathTest = IR_ValueTest;

TEST_F(IR_ValueTest, ReplaceAllUsesWith_Value) {
    auto* val_old = b.InstructionResult(ty.i32());
    auto* val_new = b.InstructionResult(ty.i32());
    auto* inst = b.Add(mod.Types().i32(), val_old, 1_i);
    EXPECT_EQ(inst->LHS(), val_old);
    val_old->ReplaceAllUsesWith(val_new);
    EXPECT_EQ(inst->LHS(), val_new);
}

TEST_F(IR_ValueTest, ReplaceAllUsesWith_Fn) {
    auto* val_old = b.InstructionResult(ty.i32());
    auto* val_new = b.InstructionResult(ty.i32());
    auto* inst = b.Add(mod.Types().i32(), val_old, 1_i);
    EXPECT_EQ(inst->LHS(), val_old);
    val_old->ReplaceAllUsesWith([&](Usage use) {
        EXPECT_EQ(use.instruction, inst);
        EXPECT_EQ(use.operand_index, 0u);
        return val_new;
    });
    EXPECT_EQ(inst->LHS(), val_new);
}

TEST_F(IR_ValueTest, Destroy) {
    auto* val = b.InstructionResult(ty.i32());
    EXPECT_TRUE(val->Alive());
    val->Destroy();
    EXPECT_FALSE(val->Alive());
}

TEST_F(IR_ValueTest, Usages) {
    auto* i1 = b.Construct(ty.i32(), 1_i);
    auto* i2 = b.Construct(ty.i32(), 2_i);
    auto* i3 = b.Construct(ty.i32(), 3_i);

    auto* target = b.Let(ty.i32())->Result(0);

    target->AddUsage(Usage{i2, 3});
    target->AddUsage(Usage{i2, 2});
    target->AddUsage(Usage{i1, 1});
    target->AddUsage(Usage{i3, 1});

    auto usages = target->UsagesSorted();
    ASSERT_EQ(usages.Length(), 4u);
    EXPECT_EQ(usages[0].instruction, i1);
    EXPECT_EQ(usages[0].operand_index, 1u);
    EXPECT_EQ(usages[1].instruction, i2);
    EXPECT_EQ(usages[1].operand_index, 2u);
    EXPECT_EQ(usages[2].instruction, i2);
    EXPECT_EQ(usages[2].operand_index, 3u);
    EXPECT_EQ(usages[3].instruction, i3);
    EXPECT_EQ(usages[3].operand_index, 1u);
}

TEST_F(IR_ValueDeathTest, Destroy_HasSource) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            auto* val = b.Add(mod.Types().i32(), 1_i, 2_i)->Result(0);
            val->Destroy();
        },
        "internal compiler error");
}

TEST_F(IR_ValueTest, UsageComparison) {
    auto* i1 = b.Construct(ty.i32(), 1_i);
    auto* i2 = b.Construct(ty.i32(), 2_i);

    Usage r{nullptr, 0};
    Usage s{nullptr, 0};
    Usage t{nullptr, 1};
    Usage u{i1, 2};
    Usage v{i1, 3};
    Usage w{i2, 4};
    Usage x{i2, 4};

    EXPECT_EQ(r, s);
    EXPECT_EQ(w, x);
    EXPECT_FALSE(s == t);
    EXPECT_FALSE(s == t);
    EXPECT_FALSE(w == v);

    EXPECT_LT(s, t);

    // Nulls at the end
    EXPECT_LT(u, s);

    EXPECT_LT(u, v);
    EXPECT_LT(v, w);
}

}  // namespace
}  // namespace tint::core::ir
