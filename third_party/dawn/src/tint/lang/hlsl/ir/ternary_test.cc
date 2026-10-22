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

#include "src/tint/lang/hlsl/ir/ternary.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::ir {
namespace {

using HlslIRTest = core::ir::IRTestHelper;
using HlslIRDeathTest = HlslIRTest;

TEST_F(HlslIRTest, SetsUsage) {
    auto* true_ = b.Constant(u32(1));
    auto* false_ = b.Constant(u32(2));
    auto* cmp = b.Constant(true);

    Vector<core::ir::Value*, 3> args = {false_, true_, cmp};
    auto* t = b.ir.CreateInstruction<Ternary>(b.InstructionResult(ty.u32()), args);

    EXPECT_THAT(false_->UsagesUnsorted(), testing::UnorderedElementsAre(core::ir::Usage{t, 0u}));
    EXPECT_THAT(true_->UsagesUnsorted(), testing::UnorderedElementsAre(core::ir::Usage{t, 1u}));
    EXPECT_THAT(cmp->UsagesUnsorted(), testing::UnorderedElementsAre(core::ir::Usage{t, 2u}));
}

TEST_F(HlslIRTest, Result) {
    auto* true_ = b.Constant(u32(1));
    auto* false_ = b.Constant(u32(2));
    auto* cmp = b.Constant(true);

    Vector<core::ir::Value*, 3> args = {false_, true_, cmp};
    auto* t = b.ir.CreateInstruction<Ternary>(b.InstructionResult(ty.u32()), args);

    EXPECT_EQ(t->Results().Length(), 1u);

    EXPECT_TRUE(t->Result()->Is<core::ir::InstructionResult>());
    EXPECT_EQ(t, t->Result()->Instruction());
}

TEST_F(HlslIRTest, Clone) {
    auto* true_ = b.Constant(u32(1));
    auto* false_ = b.Constant(u32(2));
    auto* cmp = b.Constant(true);

    Vector<core::ir::Value*, 3> args = {false_, true_, cmp};
    auto* t = b.ir.CreateInstruction<Ternary>(b.InstructionResult(ty.u32()), args);

    auto* new_t = clone_ctx.Clone(t);

    EXPECT_NE(t, new_t);

    EXPECT_NE(t->Result(), new_t->Result());
    EXPECT_EQ(ty.u32(), new_t->Result()->Type());

    EXPECT_NE(nullptr, new_t->True());
    EXPECT_EQ(t->True(), new_t->True());

    EXPECT_NE(nullptr, new_t->False());
    EXPECT_EQ(t->False(), new_t->False());

    EXPECT_NE(nullptr, new_t->Cmp());
    EXPECT_EQ(t->Cmp(), new_t->Cmp());
}

}  // namespace
}  // namespace tint::hlsl::ir
