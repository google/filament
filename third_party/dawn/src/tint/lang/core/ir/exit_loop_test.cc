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

#include "src/tint/lang/core/ir/exit_loop.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_ExitLoopTest = IRTestHelper;

TEST_F(IR_ExitLoopTest, Usage) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* loop = b.Loop();
    auto* e = b.ExitLoop(loop, arg1, arg2);

    EXPECT_THAT(arg1->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{e, 0u}));
    EXPECT_THAT(arg2->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{e, 1u}));
    EXPECT_EQ(loop->Result(0), nullptr);
}

TEST_F(IR_ExitLoopTest, Destroy) {
    auto* loop = b.Loop();
    auto* exit = b.ExitLoop(loop);
    EXPECT_THAT(loop->Exits(), testing::UnorderedElementsAre(exit));
    exit->Destroy();
    EXPECT_TRUE(loop->Exits().IsEmpty());
    EXPECT_FALSE(exit->Alive());
}

TEST_F(IR_ExitLoopTest, Clone) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* loop = b.Loop();
    auto* e = b.ExitLoop(loop, arg1, arg2);

    auto* new_loop = clone_ctx.Clone(loop);
    auto* new_exit = clone_ctx.Clone(e);

    EXPECT_NE(e, new_exit);
    EXPECT_EQ(new_loop, new_exit->Loop());

    auto args = new_exit->Args();
    ASSERT_EQ(2u, args.Length());

    auto new_arg1 = args[0]->As<Constant>()->Value();
    ASSERT_TRUE(new_arg1->Is<core::constant::Scalar<u32>>());
    EXPECT_EQ(1_u, new_arg1->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    auto new_arg2 = args[1]->As<Constant>()->Value();
    ASSERT_TRUE(new_arg2->Is<core::constant::Scalar<u32>>());
    EXPECT_EQ(2_u, new_arg2->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

TEST_F(IR_ExitLoopTest, CloneNoArgs) {
    auto* loop = b.Loop();
    auto* e = b.ExitLoop(loop);

    auto* new_loop = clone_ctx.Clone(loop);
    auto* new_exit = clone_ctx.Clone(e);

    EXPECT_EQ(new_loop, new_exit->Loop());
    EXPECT_TRUE(new_exit->Args().IsEmpty());
}

}  // namespace
}  // namespace tint::core::ir
