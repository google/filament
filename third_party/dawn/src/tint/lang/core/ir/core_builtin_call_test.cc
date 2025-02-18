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
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_CoreBuiltinCallTest = IRTestHelper;
using IR_CoreBuiltinCallDeathTest = IR_CoreBuiltinCallTest;

TEST_F(IR_CoreBuiltinCallTest, Usage) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* builtin = b.Call(mod.Types().f32(), core::BuiltinFn::kAbs, arg1, arg2);

    EXPECT_THAT(arg1->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{builtin, 0u}));
    EXPECT_THAT(arg2->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{builtin, 1u}));
}

TEST_F(IR_CoreBuiltinCallTest, Result) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* builtin = b.Call(mod.Types().f32(), core::BuiltinFn::kAbs, arg1, arg2);

    EXPECT_EQ(builtin->Results().Length(), 1u);
    EXPECT_TRUE(builtin->Result(0)->Is<InstructionResult>());
    EXPECT_EQ(builtin->Result(0)->Instruction(), builtin);
}

TEST_F(IR_CoreBuiltinCallDeathTest, Fail_NullType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            b.Call(static_cast<type::Type*>(nullptr), core::BuiltinFn::kAbs);
        },
        "internal compiler error");
}

TEST_F(IR_CoreBuiltinCallDeathTest, Fail_NoneFunction) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            b.Call(mod.Types().f32(), core::BuiltinFn::kNone);
        },
        "internal compiler error");
}

TEST_F(IR_CoreBuiltinCallTest, Clone) {
    auto* builtin = b.Call(mod.Types().f32(), core::BuiltinFn::kAbs, 1_u, 2_u);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(0), new_b->Result(0));
    EXPECT_EQ(mod.Types().f32(), new_b->Result(0)->Type());

    EXPECT_EQ(core::BuiltinFn::kAbs, new_b->Func());

    auto args = new_b->Args();
    EXPECT_EQ(2u, args.Length());

    auto* val0 = args[0]->As<Constant>()->Value();
    EXPECT_EQ(1_u, val0->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    auto* val1 = args[1]->As<Constant>()->Value();
    EXPECT_EQ(2_u, val1->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

TEST_F(IR_CoreBuiltinCallTest, CloneNoArgs) {
    auto* builtin = b.Call(mod.Types().f32(), core::BuiltinFn::kAbs);

    auto* new_b = clone_ctx.Clone(builtin);
    EXPECT_NE(builtin->Result(0), new_b->Result(0));
    EXPECT_EQ(mod.Types().f32(), new_b->Result(0)->Type());

    EXPECT_EQ(core::BuiltinFn::kAbs, new_b->Func());

    auto args = new_b->Args();
    EXPECT_TRUE(args.IsEmpty());
}

}  // namespace
}  // namespace tint::core::ir
