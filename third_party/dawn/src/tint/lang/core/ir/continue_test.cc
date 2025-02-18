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

#include "src/tint/lang/core/ir/continue.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_ContinueTest = IRTestHelper;
using IR_ContinueDeathTest = IR_ContinueTest;

TEST_F(IR_ContinueTest, Usage) {
    auto* loop = b.Loop();
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);

    auto* brk = b.Continue(loop, arg1, arg2);

    EXPECT_THAT(arg1->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{brk, 0u}));
    EXPECT_THAT(arg2->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{brk, 1u}));
}

TEST_F(IR_ContinueTest, Results) {
    auto* loop = b.Loop();
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);

    auto* brk = b.Continue(loop, arg1, arg2);

    EXPECT_TRUE(brk->Results().IsEmpty());
}

TEST_F(IR_ContinueDeathTest, Fail_NullLoop) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            b.Continue(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_ContinueTest, Clone) {
    auto* loop = b.Loop();
    auto* cont = b.Continue(loop);

    auto* new_loop = clone_ctx.Clone(loop);
    clone_ctx.Replace(loop, new_loop);

    auto* new_c = clone_ctx.Clone(cont);

    EXPECT_NE(cont, new_c);
    EXPECT_EQ(new_loop, new_c->Loop());
    EXPECT_TRUE(new_c->Args().IsEmpty());
}

TEST_F(IR_ContinueTest, CloneWithArgs) {
    auto* loop = b.Loop();
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);

    auto* cont = b.Continue(loop, arg1, arg2);

    auto* new_c = clone_ctx.Clone(cont);

    auto args = new_c->Args();
    EXPECT_EQ(2u, args.Length());

    auto* val0 = args[0]->As<Constant>()->Value();
    EXPECT_EQ(1_u, val0->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    auto* val1 = args[1]->As<Constant>()->Value();
    EXPECT_EQ(2_u, val1->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

}  // namespace
}  // namespace tint::core::ir
