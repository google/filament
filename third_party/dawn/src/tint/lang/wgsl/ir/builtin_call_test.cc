// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ir/builtin_call.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::wgsl::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_WgslBuiltinCallTest = core::ir::IRTestHelper;

TEST_F(IR_WgslBuiltinCallTest, Clone) {
    auto* builtin = b.Call<BuiltinCall>(mod.Types().void_(), BuiltinFn::kAbs, 1_i);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(), new_b->Result());
    EXPECT_EQ(mod.Types().void_(), new_b->Result()->Type());

    EXPECT_EQ(BuiltinFn::kAbs, new_b->Func());

    auto args = new_b->Args();
    EXPECT_EQ(1u, args.size());
    auto* val0 = args[0]->As<core::ir::Constant>()->Value();
    EXPECT_EQ(1_i, val0->As<core::constant::Scalar<core::i32>>()->ValueAs<core::i32>());
}

TEST_F(IR_WgslBuiltinCallTest, CloneWithExplicitParams) {
    auto* builtin = b.Call<BuiltinCall>(mod.Types().void_(), BuiltinFn::kAbs, 1_i);
    builtin->SetExplicitTemplateParams(Vector{mod.Types().i32()});

    auto* new_b = clone_ctx.Clone(builtin);
    EXPECT_NE(builtin->Result(), new_b->Result());
    EXPECT_EQ(mod.Types().void_(), new_b->Result()->Type());

    EXPECT_EQ(BuiltinFn::kAbs, new_b->Func());
    EXPECT_THAT(new_b->ExplicitTemplateParams(), testing::ElementsAre(mod.Types().i32()));
}

}  // namespace
}  // namespace tint::wgsl::ir
