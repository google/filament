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
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_StoreTest = IRTestHelper;

TEST_F(IR_StoreTest, CreateStore) {
    auto* to = b.Var(ty.ptr<private_, i32>());
    auto* inst = b.Store(to, 4_i);

    ASSERT_TRUE(inst->Is<Store>());
    ASSERT_EQ(inst->To(), to->Result(0));

    ASSERT_TRUE(inst->From()->Is<Constant>());
    auto lhs = inst->From()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_StoreTest, Usage) {
    auto* to = b.Var(ty.ptr<private_, i32>());
    auto* inst = b.Store(to, 4_i);

    ASSERT_NE(inst->To(), nullptr);
    EXPECT_THAT(inst->To()->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 0u}));

    ASSERT_NE(inst->From(), nullptr);
    EXPECT_THAT(inst->From()->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 1u}));
}

TEST_F(IR_StoreTest, Result) {
    auto* to = b.Var(ty.ptr<private_, i32>());
    auto* inst = b.Store(to, 4_i);

    EXPECT_TRUE(inst->Results().IsEmpty());
}

TEST_F(IR_StoreTest, Clone) {
    auto* v = b.Var("a", mod.Types().ptr<private_, i32>());
    auto* s = b.Store(v, b.Constant(1_i));

    auto* new_v = clone_ctx.Clone(v);
    auto* new_s = clone_ctx.Clone(s);

    EXPECT_NE(s, new_s);
    EXPECT_EQ(new_v->Result(0), new_s->To());

    auto new_from = new_s->From()->As<Constant>()->Value();
    ASSERT_TRUE(new_from->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(1_i, new_from->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

}  // namespace
}  // namespace tint::core::ir
