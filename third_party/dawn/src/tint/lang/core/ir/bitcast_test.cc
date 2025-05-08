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
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir {
namespace {

using IR_BitcastTest = IRTestHelper;
using IR_BitcastDeathTest = IR_BitcastTest;

TEST_F(IR_BitcastTest, Bitcast) {
    auto* inst = b.Bitcast(mod.Types().i32(), 4_i);

    ASSERT_TRUE(inst->Is<ir::Bitcast>());
    ASSERT_NE(inst->Result()->Type(), nullptr);

    auto args = inst->Args();
    ASSERT_EQ(args.Length(), 1u);
    ASSERT_TRUE(args[0]->Is<Constant>());
    auto val = args[0]->As<Constant>()->Value();
    ASSERT_TRUE(val->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, val->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BitcastTest, Result) {
    auto* a = b.Bitcast(mod.Types().i32(), 4_i);

    EXPECT_EQ(a->Results().Length(), 1u);
    EXPECT_TRUE(a->Result()->Is<InstructionResult>());
    EXPECT_EQ(a, a->Result()->Instruction());
}

TEST_F(IR_BitcastTest, Bitcast_Usage) {
    auto* inst = b.Bitcast(mod.Types().i32(), 4_i);

    auto args = inst->Args();
    ASSERT_EQ(args.Length(), 1u);
    ASSERT_NE(args[0], nullptr);
    EXPECT_THAT(args[0]->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 0u}));
}

TEST_F(IR_BitcastDeathTest, Fail_NullType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            b.Bitcast(nullptr, 1_i);
        },
        "internal compiler error");
}

TEST_F(IR_BitcastTest, Clone) {
    auto* inst = b.Bitcast(mod.Types().i32(), 4_i);

    auto* n = clone_ctx.Clone(inst);

    EXPECT_NE(inst, n);

    EXPECT_EQ(mod.Types().i32(), n->Result()->Type());

    auto new_val = n->Val()->As<Constant>()->Value();
    ASSERT_TRUE(new_val->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, new_val->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

}  // namespace
}  // namespace tint::core::ir
