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

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

using IR_OperandInstructionTest = IRTestHelper;

TEST_F(IR_OperandInstructionTest, Destroy) {
    auto* block = b.Block();
    auto* inst = b.Add(ty.i32(), 1_i, 2_i);
    block->Append(inst);
    auto* lhs = inst->LHS();
    auto* rhs = inst->RHS();
    EXPECT_EQ(inst->Block(), block);
    EXPECT_THAT(lhs->UsagesUnsorted(), testing::ElementsAre(Usage{inst, 0u}));
    EXPECT_THAT(rhs->UsagesUnsorted(), testing::ElementsAre(Usage{inst, 1u}));
    EXPECT_TRUE(inst->Result(0)->Alive());

    inst->Destroy();

    EXPECT_EQ(inst->Block(), nullptr);
    EXPECT_FALSE(lhs->IsUsed());
    EXPECT_FALSE(rhs->IsUsed());
    EXPECT_FALSE(inst->Result(0)->Alive());
}

TEST_F(IR_OperandInstructionTest, ClearOperands_WithNullOperand) {
    auto* block = b.Block();
    // The var initializer is a nullptr
    auto* inst = b.Var(ty.ptr<private_, f32>());
    block->Append(inst);

    inst->Destroy();
    EXPECT_EQ(inst->Block(), nullptr);
    EXPECT_FALSE(inst->Result(0)->Alive());
}

TEST_F(IR_OperandInstructionTest, SetOperands_WithNullOperand) {
    auto* inst = b.Var(ty.ptr<private_, f32>());
    Vector<Value*, 1> ops;
    ops.Push(nullptr);

    inst->SetOperands(ops);
}

}  // namespace
}  // namespace tint::core::ir
