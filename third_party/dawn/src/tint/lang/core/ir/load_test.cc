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
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_LoadTest = IRTestHelper;

TEST_F(IR_LoadTest, Create) {
    auto* store_type = ty.i32();
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* inst = b.Load(var);

    ASSERT_TRUE(inst->Is<Load>());
    ASSERT_EQ(inst->From(), var->Result(0));
    EXPECT_EQ(inst->Result(0)->Type(), store_type);

    auto* result = inst->From()->As<InstructionResult>();
    ASSERT_NE(result, nullptr);
    ASSERT_TRUE(result->Instruction()->Is<ir::Var>());
    EXPECT_EQ(result->Instruction(), var);
}

TEST_F(IR_LoadTest, Usage) {
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* inst = b.Load(var);

    ASSERT_NE(inst->From(), nullptr);
    EXPECT_THAT(inst->From()->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 0u}));
}

TEST_F(IR_LoadTest, Results) {
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* inst = b.Load(var);

    EXPECT_EQ(inst->Results().Length(), 1u);
    EXPECT_TRUE(inst->Result(0)->Is<InstructionResult>());
    EXPECT_EQ(inst->Result(0)->Instruction(), inst);
}

TEST_F(IR_LoadTest, Clone) {
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* inst = b.Load(var);

    auto* new_var = clone_ctx.Clone(var);
    auto* new_inst = clone_ctx.Clone(inst);

    EXPECT_NE(inst, new_inst);
    EXPECT_NE(nullptr, new_inst->Result(0));
    EXPECT_NE(inst->Result(0), new_inst->Result(0));

    EXPECT_EQ(new_var->Result(0), new_inst->From());
}

}  // namespace
}  // namespace tint::core::ir
