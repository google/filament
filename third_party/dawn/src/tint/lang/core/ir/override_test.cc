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

#include "src/tint/lang/core/ir/var.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_OverrideTest = IRTestHelper;
using IR_OverrideDeathTest = IR_OverrideTest;

TEST_F(IR_OverrideDeathTest, Fail_NullType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            b.Override(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_OverrideTest, Results) {
    auto* var = b.Override(ty.i32());
    EXPECT_EQ(var->Results().Length(), 1u);
    EXPECT_TRUE(var->Result()->Is<InstructionResult>());
    EXPECT_EQ(var->Result()->Instruction(), var);
}

TEST_F(IR_OverrideTest, Initializer_Usage) {
    Module mod;
    Builder b{mod};
    auto* var = b.Override(ty.f32());
    auto* init = b.Constant(1_f);
    var->SetInitializer(init);

    EXPECT_THAT(init->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{var, 0u}));
    var->SetInitializer(nullptr);
    EXPECT_FALSE(init->IsUsed());
}

TEST_F(IR_OverrideTest, Clone) {
    auto* v = b.Override(ty.f32());
    v->SetInitializer(b.Constant(4_f));
    v->SetOverrideId(OverrideId{2});

    auto* new_v = clone_ctx.Clone(v);

    EXPECT_NE(v, new_v);
    ASSERT_NE(nullptr, new_v->Result());
    EXPECT_NE(v->Result(), new_v->Result());
    EXPECT_EQ(new_v->Result()->Type(), mod.Types().f32());

    ASSERT_NE(nullptr, new_v->Initializer());
    auto new_val = new_v->Initializer()->As<Constant>()->Value();
    ASSERT_TRUE(new_val->Is<core::constant::Scalar<f32>>());
    EXPECT_FLOAT_EQ(4_f, new_val->As<core::constant::Scalar<f32>>()->ValueAs<f32>());
    EXPECT_EQ(OverrideId{2u}, new_v->OverrideId());
}

TEST_F(IR_OverrideTest, CloneWithName) {
    auto* v = b.Override("v", b.Constant(1_f));
    auto* new_v = clone_ctx.Clone(v);

    EXPECT_EQ(std::string("v"), mod.NameOf(new_v).Name());
}

}  // namespace
}  // namespace tint::core::ir
