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

using IR_LoadVectorElementTest = IRTestHelper;

TEST_F(IR_LoadVectorElementTest, Create) {
    auto* from = b.Var(ty.ptr<private_, vec3<i32>>());
    auto* inst = b.LoadVectorElement(from, 2_i);

    ASSERT_TRUE(inst->Is<LoadVectorElement>());
    ASSERT_EQ(inst->From(), from->Result(0));

    ASSERT_TRUE(inst->Index()->Is<Constant>());
    auto index = inst->Index()->As<Constant>()->Value();
    ASSERT_TRUE(index->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, index->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_LoadVectorElementTest, Usage) {
    auto* from = b.Var(ty.ptr<private_, vec3<i32>>());
    auto* inst = b.LoadVectorElement(from, 2_i);

    ASSERT_NE(inst->From(), nullptr);
    EXPECT_THAT(inst->From()->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 0u}));

    ASSERT_NE(inst->Index(), nullptr);
    EXPECT_THAT(inst->Index()->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 1u}));
}

TEST_F(IR_LoadVectorElementTest, Result) {
    auto* from = b.Var(ty.ptr<private_, vec3<i32>>());
    auto* inst = b.LoadVectorElement(from, 2_i);

    EXPECT_EQ(inst->Results().Length(), 1u);
}

TEST_F(IR_LoadVectorElementTest, Clone) {
    auto* from = b.Var(ty.ptr<private_, vec3<i32>>());
    auto* inst = b.LoadVectorElement(from, 2_i);

    auto* new_from = clone_ctx.Clone(from);
    auto* new_inst = clone_ctx.Clone(inst);

    EXPECT_NE(inst, new_inst);
    EXPECT_NE(nullptr, new_inst->Result(0));
    EXPECT_NE(inst->Result(0), new_inst->Result(0));

    EXPECT_EQ(new_from->Result(0), new_inst->From());

    auto new_idx = new_inst->Index()->As<Constant>()->Value();
    ASSERT_TRUE(new_idx->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, new_idx->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

}  // namespace
}  // namespace tint::core::ir
