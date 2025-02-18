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

#include "src/tint/lang/core/ir/user_call.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_UserCallTest = IRTestHelper;
using IR_UserCallDeathTest = IR_UserCallTest;

TEST_F(IR_UserCallTest, Usage) {
    auto* func = b.Function("myfunc", mod.Types().void_());
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* e = b.Call(mod.Types().void_(), func, Vector{arg1, arg2});
    EXPECT_THAT(func->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{e, 0u}));
    EXPECT_THAT(arg1->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{e, 1u}));
    EXPECT_THAT(arg2->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{e, 2u}));
}

TEST_F(IR_UserCallTest, Results) {
    auto* func = b.Function("myfunc", mod.Types().void_());
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* e = b.Call(mod.Types().void_(), func, Vector{arg1, arg2});

    EXPECT_EQ(e->Results().Length(), 1u);
    EXPECT_TRUE(e->Result(0)->Is<InstructionResult>());
    EXPECT_EQ(e->Result(0)->Instruction(), e);
}

TEST_F(IR_UserCallDeathTest, Fail_NullType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            b.Call(static_cast<type::Type*>(nullptr), b.Function("myfunc", mod.Types().void_()));
        },
        "internal compiler error");
}

TEST_F(IR_UserCallTest, Clone) {
    auto* func = b.Function("myfunc", mod.Types().void_());
    auto* e = b.Call(mod.Types().void_(), func, Vector{b.Constant(1_u), b.Constant(2_u)});

    auto* new_func = clone_ctx.Clone(func);
    auto* new_e = clone_ctx.Clone(e);

    EXPECT_NE(e, new_e);
    EXPECT_NE(nullptr, new_e->Result(0));
    EXPECT_NE(e->Result(0), new_e->Result(0));

    EXPECT_EQ(new_func, new_e->Target());

    auto args = new_e->Args();
    EXPECT_EQ(2u, args.Length());

    auto new_arg1 = args[0]->As<Constant>()->Value();
    ASSERT_TRUE(new_arg1->Is<core::constant::Scalar<u32>>());
    EXPECT_EQ(1_u, new_arg1->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    auto new_arg2 = args[1]->As<Constant>()->Value();
    ASSERT_TRUE(new_arg2->Is<core::constant::Scalar<u32>>());
    EXPECT_EQ(2_u, new_arg2->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

TEST_F(IR_UserCallTest, CloneWithoutArgs) {
    auto* func = b.Function("myfunc", mod.Types().void_());
    auto* e = b.Call(mod.Types().void_(), func);

    auto* new_e = clone_ctx.Clone(e);

    EXPECT_EQ(0u, new_e->Args().Length());
}

}  // namespace
}  // namespace tint::core::ir
