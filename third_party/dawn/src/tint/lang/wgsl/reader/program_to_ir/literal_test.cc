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

#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/ir_program_test.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::wgsl::reader {
namespace {

core::ir::Value* GlobalVarInitializer(core::ir::Module& m) {
    if (m.root_block->IsEmpty()) {
        ADD_FAILURE() << "m.root_block has no instruction";
        return nullptr;
    }

    auto instr = m.root_block->Instructions();
    auto* var = instr->As<core::ir::Var>();
    if (!var) {
        ADD_FAILURE() << "m.root_block.instructions[0] was not a var";
        return nullptr;
    }
    return var->Initializer();
}

using namespace tint::core::number_suffixes;  // NOLINT

using ProgramToIRLiteralTest = helpers::IRProgramTest;

TEST_F(ProgramToIRLiteralTest, EmitLiteral_Bool_True) {
    auto* expr = Expr(true);
    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<core::ir::Constant>(init));
    auto* val = init->As<core::ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<core::constant::Scalar<bool>>());
    EXPECT_TRUE(val->As<core::constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_Bool_False) {
    auto* expr = Expr(false);
    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<core::ir::Constant>(init));
    auto* val = init->As<core::ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<core::constant::Scalar<bool>>());
    EXPECT_FALSE(val->As<core::constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_Bool_Deduped) {
    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate, Expr(true));
    GlobalVar("b", ty.bool_(), core::AddressSpace::kPrivate, Expr(false));
    GlobalVar("c", ty.bool_(), core::AddressSpace::kPrivate, Expr(true));
    GlobalVar("d", ty.bool_(), core::AddressSpace::kPrivate, Expr(false));

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto itr = m.Get().root_block->begin();
    auto* var_a = (*itr)->As<core::ir::Var>();
    ++itr;

    ASSERT_NE(var_a, nullptr);
    auto* var_b = (*itr)->As<core::ir::Var>();
    ++itr;

    ASSERT_NE(var_b, nullptr);
    auto* var_c = (*itr)->As<core::ir::Var>();
    ++itr;

    ASSERT_NE(var_c, nullptr);
    auto* var_d = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_d, nullptr);

    ASSERT_EQ(var_a->Initializer(), var_c->Initializer());
    ASSERT_EQ(var_b->Initializer(), var_d->Initializer());
    ASSERT_NE(var_a->Initializer(), var_b->Initializer());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_F32) {
    auto* expr = Expr(1.2_f);
    GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<core::ir::Constant>(init));
    auto* val = init->As<core::ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<core::constant::Scalar<f32>>());
    EXPECT_EQ(1.2_f, val->As<core::constant::Scalar<f32>>()->ValueAs<f32>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_F32_Deduped) {
    GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate, Expr(1.2_f));
    GlobalVar("b", ty.f32(), core::AddressSpace::kPrivate, Expr(1.25_f));
    GlobalVar("c", ty.f32(), core::AddressSpace::kPrivate, Expr(1.2_f));

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto itr = m.Get().root_block->begin();
    auto* var_a = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_a, nullptr);
    ++itr;

    auto* var_b = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_b, nullptr);
    ++itr;

    auto* var_c = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_c, nullptr);

    ASSERT_EQ(var_a->Initializer(), var_c->Initializer());
    ASSERT_NE(var_a->Initializer(), var_b->Initializer());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_F16) {
    Enable(wgsl::Extension::kF16);
    auto* expr = Expr(1.2_h);
    GlobalVar("a", ty.f16(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<core::ir::Constant>(init));
    auto* val = init->As<core::ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<core::constant::Scalar<f16>>());
    EXPECT_EQ(1.2_h, val->As<core::constant::Scalar<f16>>()->ValueAs<f32>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_F16_Deduped) {
    Enable(wgsl::Extension::kF16);
    GlobalVar("a", ty.f16(), core::AddressSpace::kPrivate, Expr(1.2_h));
    GlobalVar("b", ty.f16(), core::AddressSpace::kPrivate, Expr(1.25_h));
    GlobalVar("c", ty.f16(), core::AddressSpace::kPrivate, Expr(1.2_h));

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto itr = m.Get().root_block->begin();
    auto* var_a = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_a, nullptr);
    ++itr;

    auto* var_b = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_b, nullptr);
    ++itr;

    auto* var_c = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_c, nullptr);

    ASSERT_EQ(var_a->Initializer(), var_c->Initializer());
    ASSERT_NE(var_a->Initializer(), var_b->Initializer());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_I32) {
    auto* expr = Expr(-2_i);
    GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<core::ir::Constant>(init));
    auto* val = init->As<core::ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(-2_i, val->As<core::constant::Scalar<i32>>()->ValueAs<f32>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_I32_Deduped) {
    GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate, Expr(-2_i));
    GlobalVar("b", ty.i32(), core::AddressSpace::kPrivate, Expr(2_i));
    GlobalVar("c", ty.i32(), core::AddressSpace::kPrivate, Expr(-2_i));

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto itr = m.Get().root_block->begin();
    auto* var_a = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_a, nullptr);
    ++itr;

    auto* var_b = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_b, nullptr);
    ++itr;

    auto* var_c = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_c, nullptr);

    ASSERT_EQ(var_a->Initializer(), var_c->Initializer());
    ASSERT_NE(var_a->Initializer(), var_b->Initializer());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_U32) {
    auto* expr = Expr(2_u);
    GlobalVar("a", ty.u32(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<core::ir::Constant>(init));
    auto* val = init->As<core::ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<core::constant::Scalar<u32>>());
    EXPECT_EQ(2_u, val->As<core::constant::Scalar<u32>>()->ValueAs<f32>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_U32_Deduped) {
    GlobalVar("a", ty.u32(), core::AddressSpace::kPrivate, Expr(2_u));
    GlobalVar("b", ty.u32(), core::AddressSpace::kPrivate, Expr(3_u));
    GlobalVar("c", ty.u32(), core::AddressSpace::kPrivate, Expr(2_u));

    auto m = Build();
    ASSERT_EQ(m, Success);

    auto itr = m.Get().root_block->begin();
    auto* var_a = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_a, nullptr);
    ++itr;

    auto* var_b = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_b, nullptr);
    ++itr;

    auto* var_c = (*itr)->As<core::ir::Var>();
    ASSERT_NE(var_c, nullptr);

    ASSERT_EQ(var_a->Initializer(), var_c->Initializer());
    ASSERT_NE(var_a->Initializer(), var_b->Initializer());
}

}  // namespace
}  // namespace tint::wgsl::reader
