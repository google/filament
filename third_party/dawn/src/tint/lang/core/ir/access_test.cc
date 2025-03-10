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

#include "src/tint/lang/core/ir/access.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir {
namespace {

using IR_AccessTest = IRTestHelper;
using IR_AccessDeathTest = IR_AccessTest;

TEST_F(IR_AccessTest, SetsUsage) {
    auto* type = ty.ptr<function, i32>();
    auto* var = b.Var(type);
    auto* idx = b.Constant(u32(1));
    auto* a = b.Access(ty.i32(), var, idx);

    EXPECT_THAT(var->Result(0)->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{a, 0u}));
    EXPECT_THAT(idx->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{a, 1u}));
}

TEST_F(IR_AccessTest, Result) {
    auto* type = ty.ptr<function, i32>();
    auto* var = b.Var(type);
    auto* idx = b.Constant(u32(1));
    auto* a = b.Access(ty.i32(), var, idx);

    EXPECT_EQ(a->Results().Length(), 1u);

    EXPECT_TRUE(a->Result(0)->Is<InstructionResult>());
    EXPECT_EQ(a, a->Result(0)->Instruction());
}

TEST_F(IR_AccessDeathTest, Fail_NullType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            auto* ty = (mod.Types().ptr<function, i32>());
            auto* var = b.Var(ty);
            b.Access(nullptr, var, u32(1));
        },
        "internal compiler error");
}

TEST_F(IR_AccessTest, Clone) {
    auto* type = ty.ptr<function, i32>();
    auto* var = b.Var(type);
    auto* idx1 = b.Constant(u32(1));
    auto* idx2 = b.Constant(u32(2));
    auto* a = b.Access(type, var, idx1, idx2);

    auto* new_a = clone_ctx.Clone(a);

    EXPECT_NE(a, new_a);

    EXPECT_NE(a->Result(0), new_a->Result(0));
    EXPECT_EQ(type, new_a->Result(0)->Type());

    EXPECT_NE(nullptr, new_a->Object());
    EXPECT_EQ(a->Object(), new_a->Object());

    auto indices = new_a->Indices();
    EXPECT_EQ(2u, indices.Length());

    auto* val0 = indices[0]->As<Constant>()->Value();
    EXPECT_EQ(1_u, val0->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    auto* val1 = indices[1]->As<Constant>()->Value();
    EXPECT_EQ(2_u, val1->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

TEST_F(IR_AccessTest, CloneNoIndices) {
    auto* type = ty.ptr<function, i32>();
    auto* var = b.Var(type);
    auto* a = b.Access(type, var);

    auto* new_a = clone_ctx.Clone(a);

    auto indices = new_a->Indices();
    EXPECT_EQ(0u, indices.Length());
}

}  // namespace
}  // namespace tint::core::ir
