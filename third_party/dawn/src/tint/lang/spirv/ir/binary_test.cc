// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/ir/binary.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::spirv::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using IR_SpirvBinaryTest = core::ir::IRTestHelper;

TEST_F(IR_SpirvBinaryTest, Clone) {
    auto* ty = mod.Types().subgroup_matrix_result(mod.Types().i8(), 2_u, 2_u);

    auto* lhs = b.Construct(ty, 2_i);
    auto* rhs = b.Construct(ty, 4_i);
    auto* inst = b.Binary<spirv::ir::Binary>(core::BinaryOp::kAdd, ty, lhs, rhs);

    auto* c = clone_ctx.Clone(inst);

    EXPECT_NE(inst, c);

    EXPECT_EQ(ty, c->Result()->Type());
    EXPECT_EQ(core::BinaryOp::kAdd, c->Op());

    auto new_lhs =
        c->LHS()->As<core::ir::InstructionResult>()->Instruction()->As<core::ir::Construct>();
    EXPECT_EQ(ty, new_lhs->Result()->Type());

    auto new_rhs =
        c->RHS()->As<core::ir::InstructionResult>()->Instruction()->As<core::ir::Construct>();
    EXPECT_EQ(ty, new_rhs->Result()->Type());
}

TEST_F(IR_SpirvBinaryTest, MatchOverloadFromDialect) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* ty = mod.Types().subgroup_matrix_left(mod.Types().i32(), 2_u, 2_u);
        auto* lhs = b.Construct(ty, 2_i);
        auto* rhs = b.Construct(ty, 4_i);
        b.Let("r", b.Binary<spirv::ir::Binary>(core::BinaryOp::kAdd, ty, lhs, rhs));
        b.Return(func);
    });

    core::ir::Capabilities caps;
    auto res = core::ir::Validate(mod, caps);
    EXPECT_EQ(res, Success) << res.Failure();
}

}  // namespace
}  // namespace tint::spirv::ir
