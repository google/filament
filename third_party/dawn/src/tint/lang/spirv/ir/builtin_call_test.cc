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

#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::spirv::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
                                              //
using IR_SpirvBuiltinCallTest = core::ir::IRTestHelper;

TEST_F(IR_SpirvBuiltinCallTest, Clone) {
    auto* builtin = b.Call<BuiltinCall>(mod.Types().f32(), BuiltinFn::kArrayLength, 1_u, 2_u);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(0), new_b->Result(0));
    EXPECT_EQ(mod.Types().f32(), new_b->Result(0)->Type());

    EXPECT_EQ(BuiltinFn::kArrayLength, new_b->Func());
    EXPECT_TRUE(new_b->ExplicitTemplateParams().IsEmpty());

    auto args = new_b->Args();
    EXPECT_EQ(2u, args.Length());

    auto* val0 = args[0]->As<core::ir::Constant>()->Value();
    EXPECT_EQ(1_u, val0->As<core::constant::Scalar<core::u32>>()->ValueAs<core::u32>());

    auto* val1 = args[1]->As<core::ir::Constant>()->Value();
    EXPECT_EQ(2_u, val1->As<core::constant::Scalar<core::u32>>()->ValueAs<core::u32>());
}

TEST_F(IR_SpirvBuiltinCallTest, CloneWithExplicitParams) {
    auto* builtin = b.Call<BuiltinCall>(mod.Types().f32(), BuiltinFn::kArrayLength, 1_u, 2_u);
    builtin->SetExplicitTemplateParams(
        Vector<const core::type::Type*, 2>{mod.Types().f32(), mod.Types().i32()});

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(0), new_b->Result(0));
    EXPECT_EQ(mod.Types().f32(), new_b->Result(0)->Type());

    EXPECT_EQ(BuiltinFn::kArrayLength, new_b->Func());

    auto args = new_b->Args();
    EXPECT_EQ(2u, args.Length());

    auto* val0 = args[0]->As<core::ir::Constant>()->Value();
    EXPECT_EQ(1_u, val0->As<core::constant::Scalar<core::u32>>()->ValueAs<core::u32>());

    auto* val1 = args[1]->As<core::ir::Constant>()->Value();
    EXPECT_EQ(2_u, val1->As<core::constant::Scalar<core::u32>>()->ValueAs<core::u32>());

    auto new_explicit = new_b->ExplicitTemplateParams();
    ASSERT_EQ(2u, new_explicit.Length());

    EXPECT_EQ(mod.Types().f32(), new_explicit[0]);
    EXPECT_EQ(mod.Types().i32(), new_explicit[1]);
}

TEST_F(IR_SpirvBuiltinCallTest, CloneNoArgs) {
    auto* builtin = b.Call<BuiltinCall>(mod.Types().f32(), BuiltinFn::kArrayLength);

    auto* new_b = clone_ctx.Clone(builtin);
    EXPECT_NE(builtin->Result(0), new_b->Result(0));
    EXPECT_EQ(mod.Types().f32(), new_b->Result(0)->Type());

    EXPECT_EQ(BuiltinFn::kArrayLength, new_b->Func());
    EXPECT_TRUE(new_b->ExplicitTemplateParams().IsEmpty());

    auto args = new_b->Args();
    EXPECT_TRUE(args.IsEmpty());
}

}  // namespace
}  // namespace tint::spirv::ir
