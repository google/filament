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

#include "src/tint/lang/core/ir/swizzle.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir {
namespace {

using IR_SwizzleTest = IRTestHelper;
using IR_SwizzleDeathTest = IR_SwizzleTest;

TEST_F(IR_SwizzleTest, SetsUsage) {
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* a = b.Swizzle(mod.Types().i32(), var, {1u});

    EXPECT_THAT(var->Result(0)->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{a, 0u}));
}

TEST_F(IR_SwizzleTest, Results) {
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* a = b.Swizzle(mod.Types().i32(), var, {1u});

    EXPECT_EQ(a->Results().Length(), 1u);
    EXPECT_TRUE(a->Result(0)->Is<InstructionResult>());
    EXPECT_EQ(a->Result(0)->Instruction(), a);
}

TEST_F(IR_SwizzleDeathTest, Fail_NullType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            auto* var = b.Var(mod.Types().ptr<function, i32>());
            b.Swizzle(nullptr, var, {1u});
        },
        "internal compiler error");
}

TEST_F(IR_SwizzleDeathTest, Fail_EmptyIndices) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            auto* var = b.Var(mod.Types().ptr<function, i32>());
            b.Swizzle(mod.Types().i32(), var, tint::Empty);
        },
        "internal compiler error");
}

TEST_F(IR_SwizzleDeathTest, Fail_TooManyIndices) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            auto* var = b.Var(mod.Types().ptr<function, i32>());
            b.Swizzle(mod.Types().i32(), var, {1u, 1u, 1u, 1u, 1u});
        },
        "internal compiler error");
}

TEST_F(IR_SwizzleDeathTest, Fail_IndexOutOfRange) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            auto* var = b.Var(mod.Types().ptr<function, i32>());
            b.Swizzle(mod.Types().i32(), var, {4u});
        },
        "internal compiler error");
}

TEST_F(IR_SwizzleTest, Clone) {
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* s = b.Swizzle(mod.Types().i32(), var, {2u});

    auto* new_var = clone_ctx.Clone(var);
    auto* new_s = clone_ctx.Clone(s);

    EXPECT_NE(s, new_s);
    EXPECT_NE(nullptr, new_s->Result(0));
    EXPECT_NE(s->Result(0), new_s->Result(0));

    EXPECT_EQ(new_var->Result(0), new_s->Object());

    EXPECT_EQ(1u, new_s->Indices().Length());
    EXPECT_EQ(2u, new_s->Indices().Front());
}

}  // namespace
}  // namespace tint::core::ir
