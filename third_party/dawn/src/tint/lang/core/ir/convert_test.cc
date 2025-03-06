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

#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_ConvertTest = IRTestHelper;
using IR_ConvertDeathTest = IR_ConvertTest;

TEST_F(IR_ConvertDeathTest, Fail_NullToType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            b.Convert(nullptr, 1_u);
        },
        "internal compiler error");
}

TEST_F(IR_ConvertTest, Results) {
    auto* c = b.Convert(mod.Types().i32(), 1_u);

    EXPECT_EQ(c->Results().Length(), 1u);
    EXPECT_TRUE(c->Result(0)->Is<InstructionResult>());
    EXPECT_EQ(c->Result(0)->Instruction(), c);
}

TEST_F(IR_ConvertTest, Clone) {
    auto* c = b.Convert(mod.Types().f32(), 1_u);

    auto* new_c = clone_ctx.Clone(c);

    EXPECT_NE(c, new_c);
    EXPECT_NE(c->Result(0), new_c->Result(0));
    EXPECT_EQ(mod.Types().f32(), new_c->Result(0)->Type());

    auto args = new_c->Args();
    EXPECT_EQ(1u, args.Length());

    auto* val0 = args[0]->As<Constant>()->Value();
    EXPECT_EQ(1_u, val0->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

}  // namespace
}  // namespace tint::core::ir
