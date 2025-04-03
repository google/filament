// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/resolver/resolver.h"

#include <tuple>

#include "gmock/gmock.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/builtin_texture_helper_test.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/float_literal_expression.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/array.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::resolver {
namespace {

using ::testing::ElementsAre;
using ::testing::HasSubstr;

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <typename T, int ID = 0>
using alias = builder::alias<T, ID>;
template <typename T>
using alias1 = builder::alias1<T>;
template <typename T>
using alias2 = builder::alias2<T>;
template <typename T>
using alias3 = builder::alias3<T>;
using Op = core::BinaryOp;

TEST_F(ResolverTest, Stmt_Assign) {
    auto* v = Var("v", ty.f32());
    auto* lhs = Expr("v");
    auto* rhs = Expr(2.3_f);

    auto* assign = Assign(lhs, rhs);
    WrapInFunction(v, assign);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(lhs), nullptr);
    ASSERT_NE(TypeOf(rhs), nullptr);

    EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(rhs)->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(lhs), assign);
    EXPECT_EQ(StmtOf(rhs), assign);
}

TEST_F(ResolverTest, Stmt_Case) {
    auto* v = Var("v", ty.f32());
    auto* lhs = Expr("v");
    auto* rhs = Expr(2.3_f);

    auto* assign = Assign(lhs, rhs);
    auto* block = Block(assign);
    auto* sel = CaseSelector(3_i);
    auto* cse = Case(sel, block);
    auto* def = DefaultCase();
    auto* cond_var = Var("c", ty.i32());
    auto* sw = Switch(cond_var, cse, def);
    WrapInFunction(v, cond_var, sw);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(lhs), nullptr);
    ASSERT_NE(TypeOf(rhs), nullptr);
    EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(rhs)->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(lhs), assign);
    EXPECT_EQ(StmtOf(rhs), assign);
    EXPECT_EQ(BlockOf(assign), block);
    auto* sem = Sem().Get(sw);
    ASSERT_EQ(sem->Cases().size(), 2u);
    EXPECT_EQ(sem->Cases()[0]->Declaration(), cse);
    ASSERT_EQ(sem->Cases()[0]->Selectors().size(), 1u);
    EXPECT_EQ(sem->Cases()[1]->Selectors().size(), 1u);
}

TEST_F(ResolverTest, Stmt_Case_AddressOf_Invalid) {
    auto* cond_var = Var("i", ty.i32());
    WrapInFunction(cond_var, Switch("i", Case(CaseSelector(AddressOf(1_a)), Block())));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: cannot take the address of value of type 'abstract-int'");
}

TEST_F(ResolverTest, Stmt_Block) {
    auto* v = Var("v", ty.f32());
    auto* lhs = Expr("v");
    auto* rhs = Expr(2.3_f);

    auto* assign = Assign(lhs, rhs);
    auto* block = Block(assign);
    WrapInFunction(v, block);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(lhs), nullptr);
    ASSERT_NE(TypeOf(rhs), nullptr);
    EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(rhs)->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(lhs), assign);
    EXPECT_EQ(StmtOf(rhs), assign);
    EXPECT_EQ(BlockOf(lhs), block);
    EXPECT_EQ(BlockOf(rhs), block);
    EXPECT_EQ(BlockOf(assign), block);
}

TEST_F(ResolverTest, Stmt_If) {
    auto* v = Var("v", ty.f32());
    auto* else_lhs = Expr("v");
    auto* else_rhs = Expr(2.3_f);

    auto* else_body = Block(Assign(else_lhs, else_rhs));

    auto* else_cond = Expr(true);
    auto* else_stmt = If(else_cond, else_body);

    auto* lhs = Expr("v");
    auto* rhs = Expr(2.3_f);

    auto* assign = Assign(lhs, rhs);
    auto* body = Block(assign);
    auto* cond = Expr(true);
    auto* stmt = If(cond, body, Else(else_stmt));
    WrapInFunction(v, stmt);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(stmt->condition), nullptr);
    ASSERT_NE(TypeOf(else_lhs), nullptr);
    ASSERT_NE(TypeOf(else_rhs), nullptr);
    ASSERT_NE(TypeOf(lhs), nullptr);
    ASSERT_NE(TypeOf(rhs), nullptr);
    EXPECT_TRUE(TypeOf(stmt->condition)->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(else_lhs)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(else_rhs)->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(rhs)->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(lhs), assign);
    EXPECT_EQ(StmtOf(rhs), assign);
    EXPECT_EQ(StmtOf(cond), stmt);
    EXPECT_EQ(StmtOf(else_cond), else_stmt);
    EXPECT_EQ(BlockOf(lhs), body);
    EXPECT_EQ(BlockOf(rhs), body);
    EXPECT_EQ(BlockOf(else_lhs), else_body);
    EXPECT_EQ(BlockOf(else_rhs), else_body);
}

TEST_F(ResolverTest, Stmt_Loop) {
    auto* v = Var("v", ty.f32());
    auto* body_lhs = Expr("v");
    auto* body_rhs = Expr(2.3_f);

    auto* body = Block(Assign(body_lhs, body_rhs), Break());
    auto* continuing_lhs = Expr("v");
    auto* continuing_rhs = Expr(2.3_f);

    auto* break_if = BreakIf(false);
    auto* continuing = Block(Assign(continuing_lhs, continuing_rhs), break_if);
    auto* stmt = Loop(body, continuing);
    WrapInFunction(v, stmt);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(body_lhs), nullptr);
    ASSERT_NE(TypeOf(body_rhs), nullptr);
    ASSERT_NE(TypeOf(continuing_lhs), nullptr);
    ASSERT_NE(TypeOf(continuing_rhs), nullptr);
    EXPECT_TRUE(TypeOf(body_lhs)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(body_rhs)->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(continuing_lhs)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(continuing_rhs)->Is<core::type::F32>());
    EXPECT_EQ(BlockOf(body_lhs), body);
    EXPECT_EQ(BlockOf(body_rhs), body);
    EXPECT_EQ(BlockOf(continuing_lhs), continuing);
    EXPECT_EQ(BlockOf(continuing_rhs), continuing);
    EXPECT_EQ(BlockOf(break_if), continuing);
}

TEST_F(ResolverTest, Stmt_Return) {
    auto* cond = Expr(2_i);

    auto* ret = Return(cond);
    Func("test", tint::Empty, ty.i32(), Vector{ret}, tint::Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(cond), nullptr);
    EXPECT_TRUE(TypeOf(cond)->Is<core::type::I32>());
}

TEST_F(ResolverTest, Stmt_Return_WithoutValue) {
    auto* ret = Return();
    WrapInFunction(ret);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_Switch) {
    auto* v = Var("v", ty.f32());
    auto* lhs = Expr("v");
    auto* rhs = Expr(2.3_f);
    auto* case_block = Block(Assign(lhs, rhs));
    auto* stmt = Switch(Expr(2_i), Case(CaseSelector(3_i), case_block), DefaultCase());
    WrapInFunction(v, stmt);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(stmt->condition), nullptr);
    ASSERT_NE(TypeOf(lhs), nullptr);
    ASSERT_NE(TypeOf(rhs), nullptr);

    EXPECT_TRUE(TypeOf(stmt->condition)->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(rhs)->Is<core::type::F32>());
    EXPECT_EQ(BlockOf(lhs), case_block);
    EXPECT_EQ(BlockOf(rhs), case_block);
}

TEST_F(ResolverTest, Stmt_Call) {
    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             Return(),
         });

    auto* expr = Call("my_func");

    auto* call = CallStmt(expr);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<core::type::Void>());
    EXPECT_EQ(StmtOf(expr), call);
}

TEST_F(ResolverTest, Stmt_VariableDecl) {
    auto* var = Var("my_var", ty.i32(), Expr(2_i));
    auto* init = var->initializer;

    auto* decl = Decl(var);
    WrapInFunction(decl);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(init), nullptr);
    EXPECT_TRUE(TypeOf(init)->Is<core::type::I32>());
}

TEST_F(ResolverTest, Stmt_VariableDecl_Alias) {
    auto* my_int = Alias("MyInt", ty.i32());
    auto* var = Var("my_var", ty.Of(my_int), Expr(2_i));
    auto* init = var->initializer;

    auto* decl = Decl(var);
    WrapInFunction(decl);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(init), nullptr);
    EXPECT_TRUE(TypeOf(init)->Is<core::type::I32>());
}

TEST_F(ResolverTest, Stmt_VariableDecl_ModuleScope) {
    auto* init = Expr(2_i);
    GlobalVar("my_var", ty.i32(), core::AddressSpace::kPrivate, init);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(init), nullptr);
    EXPECT_TRUE(TypeOf(init)->Is<core::type::I32>());
    EXPECT_EQ(StmtOf(init), nullptr);
}

TEST_F(ResolverTest, Stmt_VariableDecl_OuterScopeAfterInnerScope) {
    // fn func_i32() {
    //   {
    //     var foo : i32 = 2;
    //     var bar : i32 = foo;
    //   }
    //   var foo : f32 = 2.0;
    //   var bar : f32 = foo;
    // }

    // Declare i32 "foo" inside a block
    auto* foo_i32 = Var("foo", ty.i32(), Expr(2_i));
    auto* foo_i32_init = foo_i32->initializer;
    auto* foo_i32_decl = Decl(foo_i32);

    // Reference "foo" inside the block
    auto* bar_i32 = Var("bar", ty.i32(), Expr("foo"));
    auto* bar_i32_init = bar_i32->initializer;
    auto* bar_i32_decl = Decl(bar_i32);

    auto* inner = Block(foo_i32_decl, bar_i32_decl);

    // Declare f32 "foo" at function scope
    auto* foo_f32 = Var("foo", ty.f32(), Expr(2_f));
    auto* foo_f32_init = foo_f32->initializer;
    auto* foo_f32_decl = Decl(foo_f32);

    // Reference "foo" at function scope
    auto* bar_f32 = Var("bar", ty.f32(), Expr("foo"));
    auto* bar_f32_init = bar_f32->initializer;
    auto* bar_f32_decl = Decl(bar_f32);

    Func("func", tint::Empty, ty.void_(), Vector{inner, foo_f32_decl, bar_f32_decl});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(foo_i32_init), nullptr);
    EXPECT_TRUE(TypeOf(foo_i32_init)->Is<core::type::I32>());
    ASSERT_NE(TypeOf(foo_f32_init), nullptr);
    EXPECT_TRUE(TypeOf(foo_f32_init)->Is<core::type::F32>());
    ASSERT_NE(TypeOf(bar_i32_init), nullptr);
    EXPECT_TRUE(TypeOf(bar_i32_init)->UnwrapRef()->Is<core::type::I32>());
    ASSERT_NE(TypeOf(bar_f32_init), nullptr);
    EXPECT_TRUE(TypeOf(bar_f32_init)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(foo_i32_init), foo_i32_decl);
    EXPECT_EQ(StmtOf(bar_i32_init), bar_i32_decl);
    EXPECT_EQ(StmtOf(foo_f32_init), foo_f32_decl);
    EXPECT_EQ(StmtOf(bar_f32_init), bar_f32_decl);
    EXPECT_TRUE(CheckVarUsers(foo_i32, Vector{bar_i32->initializer}));
    EXPECT_TRUE(CheckVarUsers(foo_f32, Vector{bar_f32->initializer}));
    ASSERT_NE(VarOf(bar_i32->initializer), nullptr);
    EXPECT_EQ(VarOf(bar_i32->initializer)->Declaration(), foo_i32);
    ASSERT_NE(VarOf(bar_f32->initializer), nullptr);
    EXPECT_EQ(VarOf(bar_f32->initializer)->Declaration(), foo_f32);
}

TEST_F(ResolverTest, Stmt_VariableDecl_ModuleScopeAfterFunctionScope) {
    // fn func_i32() {
    //   var foo : i32 = 2;
    // }
    // var foo : f32 = 2.0;
    // fn func_f32() {
    //   var bar : f32 = foo;
    // }

    // Declare i32 "foo" inside a function
    auto* fn_i32 = Var("foo", ty.i32(), Expr(2_i));
    auto* fn_i32_init = fn_i32->initializer;
    auto* fn_i32_decl = Decl(fn_i32);
    Func("func_i32", tint::Empty, ty.void_(), Vector{fn_i32_decl});

    // Declare f32 "foo" at module scope
    auto* mod_f32 = Var("foo", ty.f32(), core::AddressSpace::kPrivate, Expr(2_f));
    auto* mod_init = mod_f32->initializer;
    AST().AddGlobalVariable(mod_f32);

    // Reference "foo" in another function
    auto* fn_f32 = Var("bar", ty.f32(), Expr("foo"));
    auto* fn_f32_init = fn_f32->initializer;
    auto* fn_f32_decl = Decl(fn_f32);
    Func("func_f32", tint::Empty, ty.void_(), Vector{fn_f32_decl});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(mod_init), nullptr);
    EXPECT_TRUE(TypeOf(mod_init)->Is<core::type::F32>());
    ASSERT_NE(TypeOf(fn_i32_init), nullptr);
    EXPECT_TRUE(TypeOf(fn_i32_init)->Is<core::type::I32>());
    ASSERT_NE(TypeOf(fn_f32_init), nullptr);
    EXPECT_TRUE(TypeOf(fn_f32_init)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(fn_i32_init), fn_i32_decl);
    EXPECT_EQ(StmtOf(mod_init), nullptr);
    EXPECT_EQ(StmtOf(fn_f32_init), fn_f32_decl);
    EXPECT_TRUE(CheckVarUsers(fn_i32, tint::Empty));
    EXPECT_TRUE(CheckVarUsers(mod_f32, Vector{fn_f32->initializer}));
    ASSERT_NE(VarOf(fn_f32->initializer), nullptr);
    EXPECT_EQ(VarOf(fn_f32->initializer)->Declaration(), mod_f32);
}

TEST_F(ResolverTest, ArraySize_UnsignedLiteral) {
    // var<private> a : array<f32, 10u>;
    auto* a = GlobalVar("a", ty.array(ty.f32(), Expr(10_u)), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(a), nullptr);
    auto* ref = TypeOf(a)->As<core::type::Reference>();
    ASSERT_NE(ref, nullptr);
    auto* ary = ref->StoreType()->As<sem::Array>();
    EXPECT_EQ(ary->Count(), create<core::type::ConstantArrayCount>(10u));
}

TEST_F(ResolverTest, ArraySize_SignedLiteral) {
    // var<private> a : array<f32, 10i>;
    auto* a = GlobalVar("a", ty.array(ty.f32(), Expr(10_i)), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(a), nullptr);
    auto* ref = TypeOf(a)->As<core::type::Reference>();
    ASSERT_NE(ref, nullptr);
    auto* ary = ref->StoreType()->As<sem::Array>();
    EXPECT_EQ(ary->Count(), create<core::type::ConstantArrayCount>(10u));
}

TEST_F(ResolverTest, ArraySize_UnsignedConst) {
    // const size = 10u;
    // var<private> a : array<f32, size>;
    GlobalConst("size", Expr(10_u));
    auto* a = GlobalVar("a", ty.array(ty.f32(), Expr("size")), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(a), nullptr);
    auto* ref = TypeOf(a)->As<core::type::Reference>();
    ASSERT_NE(ref, nullptr);
    auto* ary = ref->StoreType()->As<sem::Array>();
    EXPECT_EQ(ary->Count(), create<core::type::ConstantArrayCount>(10u));
}

TEST_F(ResolverTest, ArraySize_SignedConst) {
    // const size = 0;
    // var<private> a : array<f32, size>;
    GlobalConst("size", Expr(10_i));
    auto* a = GlobalVar("a", ty.array(ty.f32(), Expr("size")), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(a), nullptr);
    auto* ref = TypeOf(a)->As<core::type::Reference>();
    ASSERT_NE(ref, nullptr);
    auto* ary = ref->StoreType()->As<sem::Array>();
    EXPECT_EQ(ary->Count(), create<core::type::ConstantArrayCount>(10u));
}

TEST_F(ResolverTest, ArraySize_NamedOverride) {
    // override size = 10i;
    // var<workgroup> a : array<f32, size>;
    auto* override = Override("size", Expr(10_i));
    auto* a = GlobalVar("a", ty.array(ty.f32(), Expr("size")), core::AddressSpace::kWorkgroup);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(a), nullptr);
    auto* ref = TypeOf(a)->As<core::type::Reference>();
    ASSERT_NE(ref, nullptr);
    auto* ary = ref->StoreType()->As<sem::Array>();
    auto* sem_override = Sem().Get(override);
    ASSERT_NE(sem_override, nullptr);
    EXPECT_EQ(ary->Count(), create<sem::NamedOverrideArrayCount>(sem_override));
}

TEST_F(ResolverTest, ArraySize_NamedOverride_Equivalence) {
    // override size = 10i;
    // var<workgroup> a : array<f32, size>;
    // var<workgroup> b : array<f32, size>;
    auto* override = Override("size", Expr(10_i));
    auto* a = GlobalVar("a", ty.array(ty.f32(), Expr("size")), core::AddressSpace::kWorkgroup);
    auto* b = GlobalVar("b", ty.array(ty.f32(), Expr("size")), core::AddressSpace::kWorkgroup);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(a), nullptr);
    auto* ref_a = TypeOf(a)->As<core::type::Reference>();
    ASSERT_NE(ref_a, nullptr);
    auto* ary_a = ref_a->StoreType()->As<sem::Array>();

    ASSERT_NE(TypeOf(b), nullptr);
    auto* ref_b = TypeOf(b)->As<core::type::Reference>();
    ASSERT_NE(ref_b, nullptr);
    auto* ary_b = ref_b->StoreType()->As<sem::Array>();

    auto* sem_override = Sem().Get(override);
    ASSERT_NE(sem_override, nullptr);
    EXPECT_EQ(ary_a->Count(), create<sem::NamedOverrideArrayCount>(sem_override));
    EXPECT_EQ(ary_b->Count(), create<sem::NamedOverrideArrayCount>(sem_override));
    EXPECT_EQ(ary_a, ary_b);
}

TEST_F(ResolverTest, ArraySize_UnnamedOverride) {
    // override size = 10i;
    // var<workgroup> a : array<f32, size*2>;
    auto* override = Override("size", Expr(10_i));
    auto* cnt = Mul("size", 2_a);
    auto* a = GlobalVar("a", ty.array(ty.f32(), cnt), core::AddressSpace::kWorkgroup);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(a), nullptr);
    auto* ref = TypeOf(a)->As<core::type::Reference>();
    ASSERT_NE(ref, nullptr);
    auto* ary = ref->StoreType()->As<sem::Array>();
    auto* sem_override = Sem().Get(override);
    ASSERT_NE(sem_override, nullptr);
    EXPECT_EQ(ary->Count(), create<sem::UnnamedOverrideArrayCount>(Sem().Get(cnt)));
}

TEST_F(ResolverTest, ArraySize_UnamedOverride_Equivalence) {
    // override size = 10i;
    // var<workgroup> a : array<f32, size>;
    // var<workgroup> b : array<f32, size>;
    auto* override = Override("size", Expr(10_i));
    auto* a_cnt = Mul("size", 2_a);
    auto* b_cnt = Mul("size", 2_a);
    auto* a = GlobalVar("a", ty.array(ty.f32(), a_cnt), core::AddressSpace::kWorkgroup);
    auto* b = GlobalVar("b", ty.array(ty.f32(), b_cnt), core::AddressSpace::kWorkgroup);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(a), nullptr);
    auto* ref_a = TypeOf(a)->As<core::type::Reference>();
    ASSERT_NE(ref_a, nullptr);
    auto* ary_a = ref_a->StoreType()->As<sem::Array>();

    ASSERT_NE(TypeOf(b), nullptr);
    auto* ref_b = TypeOf(b)->As<core::type::Reference>();
    ASSERT_NE(ref_b, nullptr);
    auto* ary_b = ref_b->StoreType()->As<sem::Array>();

    auto* sem_override = Sem().Get(override);
    ASSERT_NE(sem_override, nullptr);
    EXPECT_EQ(ary_a->Count(), create<sem::UnnamedOverrideArrayCount>(Sem().Get(a_cnt)));
    EXPECT_EQ(ary_b->Count(), create<sem::UnnamedOverrideArrayCount>(Sem().Get(b_cnt)));
    EXPECT_NE(ary_a, ary_b);
}

TEST_F(ResolverTest, Expr_Bitcast) {
    GlobalVar("name", ty.f32(), core::AddressSpace::kPrivate);

    auto* bitcast = Bitcast<f32>(Expr("name"));
    WrapInFunction(bitcast);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(bitcast), nullptr);
    EXPECT_TRUE(TypeOf(bitcast)->Is<core::type::F32>());
}

TEST_F(ResolverTest, Expr_Call) {
    Func("my_func", tint::Empty, ty.f32(), Vector{Return(0_f)});

    auto* call = Call("my_func");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<core::type::F32>());
}

TEST_F(ResolverTest, Expr_Call_InBinaryOp) {
    Func("func", tint::Empty, ty.f32(), Vector{Return(0_f)});

    auto* expr = Add(Call("func"), Call("func"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<core::type::F32>());
}

TEST_F(ResolverTest, Expr_Call_WithParams) {
    Func("my_func", Vector{Param(Sym(), ty.f32())}, ty.f32(),
         Vector{
             Return(1.2_f),
         });

    auto* param = Expr(2.4_f);

    auto* call = Call("my_func", param);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(param), nullptr);
    EXPECT_TRUE(TypeOf(param)->Is<core::type::F32>());
}

TEST_F(ResolverTest, Expr_Call_Builtin) {
    auto* call = Call("round", 2.4_f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<core::type::F32>());
}

TEST_F(ResolverTest, Expr_Cast) {
    GlobalVar("name", ty.f32(), core::AddressSpace::kPrivate);

    auto* cast = Call<f32>("name");
    WrapInFunction(cast);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(cast), nullptr);
    EXPECT_TRUE(TypeOf(cast)->Is<core::type::F32>());
}

TEST_F(ResolverTest, Expr_Initializer_Scalar) {
    auto* s = Expr(1_f);
    WrapInFunction(s);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(s), nullptr);
    EXPECT_TRUE(TypeOf(s)->Is<core::type::F32>());
}

TEST_F(ResolverTest, Expr_Initializer_Type_Vec2) {
    auto* tc = Call<vec2<f32>>(1_f, 1_f);
    WrapInFunction(tc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 2u);
}

TEST_F(ResolverTest, Expr_Initializer_Type_Vec3) {
    auto* tc = Call<vec3<f32>>(1_f, 1_f, 1_f);
    WrapInFunction(tc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);
}

TEST_F(ResolverTest, Expr_Initializer_Type_Vec4) {
    auto* tc = Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f);
    WrapInFunction(tc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverTest, Expr_Identifier_GlobalVariable) {
    auto* my_var = GlobalVar("my_var", ty.f32(), core::AddressSpace::kPrivate);

    auto* ident = Expr("my_var");
    WrapInFunction(ident);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(ident), nullptr);
    EXPECT_TRUE(TypeOf(ident)->Is<core::type::F32>());
    EXPECT_TRUE(CheckVarUsers(my_var, Vector{ident}));
    ASSERT_NE(VarOf(ident), nullptr);
    EXPECT_EQ(VarOf(ident)->Declaration(), my_var);
}

TEST_F(ResolverTest, Expr_Identifier_GlobalConst) {
    auto* my_var = GlobalConst("my_var", ty.f32(), Call<f32>());

    auto* ident = Expr("my_var");
    WrapInFunction(ident);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(ident), nullptr);
    EXPECT_TRUE(TypeOf(ident)->Is<core::type::F32>());
    EXPECT_TRUE(CheckVarUsers(my_var, Vector{ident}));
    ASSERT_NE(VarOf(ident), nullptr);
    EXPECT_EQ(VarOf(ident)->Declaration(), my_var);
}

TEST_F(ResolverTest, Expr_Identifier_FunctionVariable_Const) {
    auto* my_var_a = Expr("my_var");
    auto* var = Let("my_var", ty.f32(), Call<f32>());
    auto* decl = Decl(Var("b", ty.f32(), my_var_a));

    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
             decl,
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(my_var_a), nullptr);
    EXPECT_TRUE(TypeOf(my_var_a)->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(my_var_a), decl);
    EXPECT_TRUE(CheckVarUsers(var, Vector{my_var_a}));
    ASSERT_NE(VarOf(my_var_a), nullptr);
    EXPECT_EQ(VarOf(my_var_a)->Declaration(), var);
}

TEST_F(ResolverTest, IndexAccessor_Dynamic_Ref_F32) {
    // var a : array<bool, 10u> = 0;
    // var idx : f32 = f32();
    // var f : f32 = a[idx];
    auto* a = Var("a", ty.array<bool, 10>(), Call<array<bool, 10>>());
    auto* idx = Var("idx", ty.f32(), Call<f32>());
    auto* f = Var("f", ty.f32(), IndexAccessor("a", Expr(Source{{12, 34}}, idx)));
    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             Decl(a),
             Decl(idx),
             Decl(f),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverTest, Expr_Identifier_FunctionVariable) {
    auto* my_var_a = Expr("my_var");
    auto* my_var_b = Expr("my_var");
    auto* assign = Assign(my_var_a, my_var_b);

    auto* var = Var("my_var", ty.f32());

    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
             assign,
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(my_var_a), nullptr);
    ASSERT_TRUE(TypeOf(my_var_a)->Is<core::type::Reference>());
    EXPECT_TRUE(TypeOf(my_var_a)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(my_var_a), assign);
    ASSERT_NE(TypeOf(my_var_b), nullptr);
    EXPECT_TRUE(TypeOf(my_var_b)->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(my_var_b), assign);
    EXPECT_TRUE(CheckVarUsers(var, Vector{my_var_a, my_var_b}));
    ASSERT_NE(VarOf(my_var_a), nullptr);
    EXPECT_EQ(VarOf(my_var_a)->Declaration(), var);
    ASSERT_NE(VarOf(my_var_b), nullptr);
    EXPECT_EQ(VarOf(my_var_b)->Declaration(), var);
}

TEST_F(ResolverTest, Expr_Identifier_Function_Ptr) {
    auto* v = Expr("v");
    auto* p = Expr("p");
    auto* v_decl = Decl(Var("v", ty.f32()));
    auto* p_decl = Decl(Let("p", ty.ptr<function, f32>(), AddressOf(v)));
    auto* assign = Assign(Deref(p), 1.23_f);
    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             v_decl,
             p_decl,
             assign,
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(v), nullptr);
    ASSERT_TRUE(TypeOf(v)->Is<core::type::Reference>());
    EXPECT_TRUE(TypeOf(v)->UnwrapRef()->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(v), p_decl);
    ASSERT_NE(TypeOf(p), nullptr);
    ASSERT_TRUE(TypeOf(p)->Is<core::type::Pointer>());
    EXPECT_TRUE(TypeOf(p)->UnwrapPtr()->Is<core::type::F32>());
    EXPECT_EQ(StmtOf(p), assign);
}

TEST_F(ResolverTest, Expr_Call_Function) {
    Func("my_func", tint::Empty, ty.f32(),
         Vector{
             Return(0_f),
         });

    auto* call = Call("my_func");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<core::type::F32>());
}

TEST_F(ResolverTest, Expr_Identifier_Unknown) {
    auto* a = Expr("a");
    WrapInFunction(a);

    EXPECT_FALSE(r()->Resolve());
}

TEST_F(ResolverTest, Function_Parameters) {
    auto* param_a = Param("a", ty.f32());
    auto* param_b = Param("b", ty.i32());
    auto* param_c = Param("c", ty.u32());

    auto* func = Func("my_func",
                      Vector{
                          param_a,
                          param_b,
                          param_c,
                      },
                      ty.void_(), tint::Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);
    EXPECT_EQ(func_sem->Parameters().Length(), 3u);
    EXPECT_TRUE(func_sem->Parameters()[0]->Type()->Is<core::type::F32>());
    EXPECT_TRUE(func_sem->Parameters()[1]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(func_sem->Parameters()[2]->Type()->Is<core::type::U32>());
    EXPECT_EQ(func_sem->Parameters()[0]->Declaration(), param_a);
    EXPECT_EQ(func_sem->Parameters()[1]->Declaration(), param_b);
    EXPECT_EQ(func_sem->Parameters()[2]->Declaration(), param_c);
    EXPECT_TRUE(func_sem->ReturnType()->Is<core::type::Void>());
}

TEST_F(ResolverTest, Function_Parameters_Locations) {
    auto* param_a = Param("a", ty.f32(), Vector{Location(3_a)});
    auto* param_b = Param("b", ty.u32(), Vector{Builtin(core::BuiltinValue::kVertexIndex)});
    auto* param_c = Param("c", ty.u32(), Vector{Location(1_a)});

    GlobalVar("my_vec", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    auto* func = Func("my_func",
                      Vector{
                          param_a,
                          param_b,
                          param_c,
                      },
                      ty.vec4<f32>(),
                      Vector{
                          Return("my_vec"),
                      },
                      Vector{
                          Stage(ast::PipelineStage::kVertex),
                      },
                      Vector{
                          Builtin(core::BuiltinValue::kPosition),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);
    EXPECT_EQ(func_sem->Parameters().Length(), 3u);
    EXPECT_EQ(3u, func_sem->Parameters()[0]->Attributes().location);
    EXPECT_FALSE(func_sem->Parameters()[1]->Attributes().location.has_value());
    EXPECT_EQ(1u, func_sem->Parameters()[2]->Attributes().location);
}

TEST_F(ResolverTest, Function_RegisterInputOutputVariables) {
    auto* s = Structure("S", Vector{Member("m", ty.u32())});

    auto* sb_var = GlobalVar("sb_var", ty.Of(s), core::AddressSpace::kStorage,
                             core::Access::kReadWrite, Binding(0_a), Group(0_a));
    auto* wg_var = GlobalVar("wg_var", ty.f32(), core::AddressSpace::kWorkgroup);
    auto* priv_var = GlobalVar("priv_var", ty.f32(), core::AddressSpace::kPrivate);

    auto* func = Func("my_func", tint::Empty, ty.void_(),
                      Vector{
                          Assign("wg_var", "wg_var"),
                          Assign("sb_var", "sb_var"),
                          Assign("priv_var", "priv_var"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);
    EXPECT_EQ(func_sem->Parameters().Length(), 0u);
    EXPECT_TRUE(func_sem->ReturnType()->Is<core::type::Void>());

    const auto& vars = func_sem->TransitivelyReferencedGlobals();
    ASSERT_EQ(vars.Length(), 3u);
    EXPECT_EQ(vars[0]->Declaration(), wg_var);
    EXPECT_EQ(vars[1]->Declaration(), sb_var);
    EXPECT_EQ(vars[2]->Declaration(), priv_var);
}

TEST_F(ResolverTest, Function_ReturnType_Location) {
    auto* func = Func("my_func", tint::Empty, ty.f32(),
                      Vector{
                          Return(1_f),
                      },
                      Vector{
                          Stage(ast::PipelineStage::kFragment),
                      },
                      Vector{
                          Location(2_a),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(func);
    ASSERT_NE(nullptr, sem);
    EXPECT_EQ(2u, sem->ReturnLocation());
}

TEST_F(ResolverTest, Function_ReturnType_NoLocation) {
    GlobalVar("my_vec", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    auto* func = Func("my_func", tint::Empty, ty.vec4<f32>(),
                      Vector{
                          Return("my_vec"),
                      },
                      Vector{
                          Stage(ast::PipelineStage::kVertex),
                      },
                      Vector{
                          Builtin(core::BuiltinValue::kPosition),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(func);
    ASSERT_NE(nullptr, sem);
    EXPECT_FALSE(sem->ReturnLocation());
}

TEST_F(ResolverTest, Function_RegisterInputOutputVariables_SubFunction) {
    auto* s = Structure("S", Vector{Member("m", ty.u32())});

    auto* sb_var = GlobalVar("sb_var", ty.Of(s), core::AddressSpace::kStorage,
                             core::Access::kReadWrite, Binding(0_a), Group(0_a));
    auto* wg_var = GlobalVar("wg_var", ty.f32(), core::AddressSpace::kWorkgroup);
    auto* priv_var = GlobalVar("priv_var", ty.f32(), core::AddressSpace::kPrivate);

    Func("my_func", tint::Empty, ty.f32(),
         Vector{Assign("wg_var", "wg_var"), Assign("sb_var", "sb_var"),
                Assign("priv_var", "priv_var"), Return(0_f)});

    auto* func2 = Func("func", tint::Empty, ty.void_(),
                       Vector{
                           WrapInStatement(Call("my_func")),
                       },
                       tint::Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func2_sem = Sem().Get(func2);
    ASSERT_NE(func2_sem, nullptr);
    EXPECT_EQ(func2_sem->Parameters().Length(), 0u);

    const auto& vars = func2_sem->TransitivelyReferencedGlobals();
    ASSERT_EQ(vars.Length(), 3u);
    EXPECT_EQ(vars[0]->Declaration(), wg_var);
    EXPECT_EQ(vars[1]->Declaration(), sb_var);
    EXPECT_EQ(vars[2]->Declaration(), priv_var);
}

TEST_F(ResolverTest, Function_NotRegisterFunctionVariable) {
    auto* func = Func("my_func", tint::Empty, ty.void_(),
                      Vector{
                          Decl(Var("var", ty.f32())),
                          Assign("var", 1_f),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);

    EXPECT_EQ(func_sem->TransitivelyReferencedGlobals().Length(), 0u);
    EXPECT_TRUE(func_sem->ReturnType()->Is<core::type::Void>());
}

TEST_F(ResolverTest, Function_NotRegisterFunctionConstant) {
    auto* func = Func("my_func", tint::Empty, ty.void_(),
                      Vector{
                          Decl(Let("var", ty.f32(), Call<f32>())),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);

    EXPECT_EQ(func_sem->TransitivelyReferencedGlobals().Length(), 0u);
    EXPECT_TRUE(func_sem->ReturnType()->Is<core::type::Void>());
}

TEST_F(ResolverTest, Function_NotRegisterFunctionParams) {
    auto* func = Func("my_func", Vector{Param("var", ty.f32())}, ty.void_(), tint::Empty);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);

    EXPECT_EQ(func_sem->TransitivelyReferencedGlobals().Length(), 0u);
    EXPECT_TRUE(func_sem->ReturnType()->Is<core::type::Void>());
}

TEST_F(ResolverTest, Function_CallSites) {
    auto* foo = Func("foo", tint::Empty, ty.void_(), tint::Empty);

    auto* call_1 = Call("foo");
    auto* call_2 = Call("foo");
    auto* bar = Func("bar", tint::Empty, ty.void_(),
                     Vector{
                         CallStmt(call_1),
                         CallStmt(call_2),
                     });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* foo_sem = Sem().Get(foo);
    ASSERT_NE(foo_sem, nullptr);
    ASSERT_EQ(foo_sem->CallSites().Length(), 2u);
    EXPECT_EQ(foo_sem->CallSites()[0]->Declaration(), call_1);
    EXPECT_EQ(foo_sem->CallSites()[1]->Declaration(), call_2);

    auto* bar_sem = Sem().Get(bar);
    ASSERT_NE(bar_sem, nullptr);
    EXPECT_EQ(bar_sem->CallSites().Length(), 0u);
}

TEST_F(ResolverTest, Function_WorkgroupSize_NotSet) {
    // @compute @workgroup_size(1)
    // fn main() {}
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);

    EXPECT_EQ(func_sem->WorkgroupSize()[0], 1u);
    EXPECT_EQ(func_sem->WorkgroupSize()[1], 1u);
    EXPECT_EQ(func_sem->WorkgroupSize()[2], 1u);
}

TEST_F(ResolverTest, Function_WorkgroupSize_Literals) {
    // @compute @workgroup_size(8, 2, 3)
    // fn main() {}
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(8_i, 2_i, 3_i),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);

    EXPECT_EQ(func_sem->WorkgroupSize()[0], 8u);
    EXPECT_EQ(func_sem->WorkgroupSize()[1], 2u);
    EXPECT_EQ(func_sem->WorkgroupSize()[2], 3u);
}

TEST_F(ResolverTest, Function_WorkgroupSize_ViaConst) {
    // const width = 16i;
    // const height = 8i;
    // const depth = 2i;
    // @compute @workgroup_size(width, height, depth)
    // fn main() {}
    GlobalConst("width", ty.i32(), Expr(16_i));
    GlobalConst("height", ty.i32(), Expr(8_i));
    GlobalConst("depth", ty.i32(), Expr(2_i));
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize("width", "height", "depth"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);

    EXPECT_EQ(func_sem->WorkgroupSize()[0], 16u);
    EXPECT_EQ(func_sem->WorkgroupSize()[1], 8u);
    EXPECT_EQ(func_sem->WorkgroupSize()[2], 2u);
}

TEST_F(ResolverTest, Function_WorkgroupSize_ViaConst_NestedInitializer) {
    // const width = i32(i32(i32(8i)));
    // const height = i32(i32(i32(4i)));
    // @compute @workgroup_size(width, height)
    // fn main() {}
    GlobalConst("width", ty.i32(), Call<i32>(Call<i32>(Call<i32>(8_i))));
    GlobalConst("height", ty.i32(), Call<i32>(Call<i32>(Call<i32>(4_i))));
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize("width", "height"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);

    EXPECT_EQ(func_sem->WorkgroupSize()[0], 8u);
    EXPECT_EQ(func_sem->WorkgroupSize()[1], 4u);
    EXPECT_EQ(func_sem->WorkgroupSize()[2], 1u);
}

TEST_F(ResolverTest, Function_WorkgroupSize_OverridableConsts) {
    // @id(0) override width = 16i;
    // @id(1) override height = 8i;
    // @id(2) override depth = 2i;
    // @compute @workgroup_size(width, height, depth)
    // fn main() {}
    Override("width", ty.i32(), Expr(16_i), Id(0_a));
    Override("height", ty.i32(), Expr(8_i), Id(1_a));
    Override("depth", ty.i32(), Expr(2_i), Id(2_a));
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize("width", "height", "depth"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);

    EXPECT_EQ(func_sem->WorkgroupSize()[0], std::nullopt);
    EXPECT_EQ(func_sem->WorkgroupSize()[1], std::nullopt);
    EXPECT_EQ(func_sem->WorkgroupSize()[2], std::nullopt);
}

TEST_F(ResolverTest, Function_WorkgroupSize_OverridableConsts_NoInit) {
    // @id(0) override width : i32;
    // @id(1) override height : i32;
    // @id(2) override depth : i32;
    // @compute @workgroup_size(width, height, depth)
    // fn main() {}
    Override("width", ty.i32(), Id(0_a));
    Override("height", ty.i32(), Id(1_a));
    Override("depth", ty.i32(), Id(2_a));
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize("width", "height", "depth"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);

    EXPECT_EQ(func_sem->WorkgroupSize()[0], std::nullopt);
    EXPECT_EQ(func_sem->WorkgroupSize()[1], std::nullopt);
    EXPECT_EQ(func_sem->WorkgroupSize()[2], std::nullopt);
}

TEST_F(ResolverTest, Function_WorkgroupSize_Mixed) {
    // @id(1) override height = 2i;
    // const depth = 3i;
    // @compute @workgroup_size(8, height, depth)
    // fn main() {}
    Override("height", ty.i32(), Expr(2_i), Id(0_a));
    GlobalConst("depth", ty.i32(), Expr(3_i));
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(8_i, "height", "depth"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* func_sem = Sem().Get(func);
    ASSERT_NE(func_sem, nullptr);

    EXPECT_EQ(func_sem->WorkgroupSize()[0], 8u);
    EXPECT_EQ(func_sem->WorkgroupSize()[1], std::nullopt);
    EXPECT_EQ(func_sem->WorkgroupSize()[2], 3u);
}

TEST_F(ResolverTest, Expr_MemberAccessor_Type) {
    auto* mem = MemberAccessor(Ident(Source{{12, 34}}, "f32"), "member");
    WrapInFunction(mem);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot use type 'f32' as value
12:34 note: are you missing '()'?)");
}

TEST_F(ResolverTest, Expr_MemberAccessor_NonCompoundType) {
    GlobalConst("depth", ty.i32(), Expr(3_i));
    auto* mem = MemberAccessor(Ident(Source{{12, 34}}, "depth"), "x");
    WrapInFunction(mem);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot index into expression of type 'i32')");
}

TEST_F(ResolverTest, Expr_MemberAccessor_Struct) {
    auto* st =
        Structure("S", Vector{Member("first_member", ty.i32()), Member("second_member", ty.f32())});
    GlobalVar("my_struct", ty.Of(st), core::AddressSpace::kPrivate);

    auto* mem = MemberAccessor("my_struct", "second_member");
    WrapInFunction(mem);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(mem), nullptr);
    EXPECT_TRUE(TypeOf(mem)->Is<core::type::F32>());
    auto* sma = Sem().Get(mem)->UnwrapLoad()->As<sem::StructMemberAccess>();
    ASSERT_NE(sma, nullptr);
    EXPECT_TRUE(sma->Member()->Type()->Is<core::type::F32>());
    EXPECT_EQ(sma->Object()->Declaration(), mem->object);
    EXPECT_EQ(sma->Member()->Index(), 1u);
    EXPECT_EQ(sma->Member()->Name().Name(), "second_member");
}

TEST_F(ResolverTest, Expr_MemberAccessor_Struct_Alias) {
    auto* st =
        Structure("S", Vector{Member("first_member", ty.i32()), Member("second_member", ty.f32())});
    auto* alias = Alias("alias", ty.Of(st));
    GlobalVar("my_struct", ty.Of(alias), core::AddressSpace::kPrivate);

    auto* mem = MemberAccessor("my_struct", "second_member");
    WrapInFunction(mem);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(mem), nullptr);
    EXPECT_TRUE(TypeOf(mem)->Is<core::type::F32>());
    auto* sma = Sem().Get(mem)->UnwrapLoad()->As<sem::StructMemberAccess>();
    ASSERT_NE(sma, nullptr);
    EXPECT_EQ(sma->Object()->Declaration(), mem->object);
    EXPECT_TRUE(sma->Member()->Type()->Is<core::type::F32>());
    EXPECT_EQ(sma->Member()->Index(), 1u);
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle) {
    GlobalVar("my_vec", ty.vec4<f32>(), core::AddressSpace::kPrivate);

    auto* mem = MemberAccessor("my_vec", "xzyw");
    WrapInFunction(mem);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(mem), nullptr);
    ASSERT_TRUE(TypeOf(mem)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(mem)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(mem)->As<core::type::Vector>()->Width(), 4u);
    auto* sma = Sem().Get(mem)->As<sem::Swizzle>();
    ASSERT_NE(sma, nullptr);
    EXPECT_EQ(sma->Object()->Declaration(), mem->object);
    EXPECT_THAT(sma->Indices(), ElementsAre(0, 2, 1, 3));
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle_SingleElement) {
    GlobalVar("my_vec", ty.vec3<f32>(), core::AddressSpace::kPrivate);

    auto* mem = MemberAccessor("my_vec", "b");
    WrapInFunction(mem);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(mem), nullptr);
    ASSERT_TRUE(TypeOf(mem)->Is<core::type::F32>());
    auto* sma = Sem().Get(mem)->UnwrapLoad()->As<sem::Swizzle>();
    ASSERT_NE(sma, nullptr);
    EXPECT_EQ(sma->Object()->Declaration(), mem->object);
    EXPECT_THAT(sma->Indices(), ElementsAre(2));
}

TEST_F(ResolverTest, Expr_Accessor_MultiLevel) {
    // struct b {
    //   vec4<f32> foo
    // }
    // struct A {
    //   array<b, 3u> mem
    // }
    // var c : A
    // c.mem[0].foo.yx
    //   -> vec2<f32>
    //
    // fn f() {
    //   c.mem[0].foo
    // }
    //

    auto* stB = Structure("B", Vector{Member("foo", ty.vec4<f32>())});
    auto* stA = Structure("A", Vector{Member("mem", ty.array(ty.Of(stB), 3_i))});
    GlobalVar("c", ty.Of(stA), core::AddressSpace::kPrivate);

    auto* mem =
        MemberAccessor(MemberAccessor(IndexAccessor(MemberAccessor("c", "mem"), 0_i), "foo"), "yx");
    WrapInFunction(mem);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(mem), nullptr);
    ASSERT_TRUE(TypeOf(mem)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(mem)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(mem)->As<core::type::Vector>()->Width(), 2u);
    ASSERT_TRUE(Sem().Get(mem)->Is<sem::Swizzle>());
}

TEST_F(ResolverTest, Expr_MemberAccessor_InBinaryOp) {
    auto* st =
        Structure("S", Vector{Member("first_member", ty.f32()), Member("second_member", ty.f32())});
    GlobalVar("my_struct", ty.Of(st), core::AddressSpace::kPrivate);

    auto* expr = Add(MemberAccessor("my_struct", "first_member"),
                     MemberAccessor("my_struct", "second_member"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<core::type::F32>());
}

namespace ExprBinaryTest {

template <typename T, int ID>
struct Aliased {
    using type = alias<T, ID>;
};

template <int N, typename T, int ID>
struct Aliased<vec<N, T>, ID> {
    using type = vec<N, alias<T, ID>>;
};

template <int N, int M, typename T, int ID>
struct Aliased<mat<N, M, T>, ID> {
    using type = mat<N, M, alias<T, ID>>;
};

struct Params {
    core::BinaryOp op;
    builder::ast_type_func_ptr create_lhs_type;
    builder::ast_type_func_ptr create_rhs_type;
    builder::ast_type_func_ptr create_lhs_alias_type;
    builder::ast_type_func_ptr create_rhs_alias_type;
    builder::sem_type_func_ptr create_result_type;
};

template <typename LHS, typename RHS, typename RES>
constexpr Params ParamsFor(core::BinaryOp op) {
    return Params{op,
                  DataType<LHS>::AST,
                  DataType<RHS>::AST,
                  DataType<typename Aliased<LHS, 0>::type>::AST,
                  DataType<typename Aliased<RHS, 1>::type>::AST,
                  DataType<RES>::Sem};
}

static constexpr core::BinaryOp all_ops[] = {
    core::BinaryOp::kAnd,
    core::BinaryOp::kOr,
    core::BinaryOp::kXor,
    core::BinaryOp::kLogicalAnd,
    core::BinaryOp::kLogicalOr,
    core::BinaryOp::kEqual,
    core::BinaryOp::kNotEqual,
    core::BinaryOp::kLessThan,
    core::BinaryOp::kGreaterThan,
    core::BinaryOp::kLessThanEqual,
    core::BinaryOp::kGreaterThanEqual,
    core::BinaryOp::kShiftLeft,
    core::BinaryOp::kShiftRight,
    core::BinaryOp::kAdd,
    core::BinaryOp::kSubtract,
    core::BinaryOp::kMultiply,
    core::BinaryOp::kDivide,
    core::BinaryOp::kModulo,
};

static constexpr builder::ast_type_func_ptr all_create_type_funcs[] = {
    DataType<bool>::AST,         //
    DataType<u32>::AST,          //
    DataType<i32>::AST,          //
    DataType<f32>::AST,          //
    DataType<vec3<bool>>::AST,   //
    DataType<vec3<i32>>::AST,    //
    DataType<vec3<u32>>::AST,    //
    DataType<vec3<f32>>::AST,    //
    DataType<mat3x3<f32>>::AST,  //
    DataType<mat2x3<f32>>::AST,  //
    DataType<mat3x2<f32>>::AST   //
};

// A list of all valid test cases for 'lhs op rhs', except that for vecN and
// matNxN, we only test N=3.
static constexpr Params all_valid_cases[] = {
    // Logical expressions
    // https://gpuweb.github.io/gpuweb/wgsl.html#logical-expr

    // Binary logical expressions
    ParamsFor<bool, bool, bool>(Op::kLogicalAnd),
    ParamsFor<bool, bool, bool>(Op::kLogicalOr),

    ParamsFor<bool, bool, bool>(Op::kAnd),
    ParamsFor<bool, bool, bool>(Op::kOr),
    ParamsFor<vec3<bool>, vec3<bool>, vec3<bool>>(Op::kAnd),
    ParamsFor<vec3<bool>, vec3<bool>, vec3<bool>>(Op::kOr),

    // Arithmetic expressions
    // https://gpuweb.github.io/gpuweb/wgsl.html#arithmetic-expr

    // Binary arithmetic expressions over scalars
    ParamsFor<i32, i32, i32>(Op::kAdd),
    ParamsFor<i32, i32, i32>(Op::kSubtract),
    ParamsFor<i32, i32, i32>(Op::kMultiply),
    ParamsFor<i32, i32, i32>(Op::kDivide),
    ParamsFor<i32, i32, i32>(Op::kModulo),

    ParamsFor<u32, u32, u32>(Op::kAdd),
    ParamsFor<u32, u32, u32>(Op::kSubtract),
    ParamsFor<u32, u32, u32>(Op::kMultiply),
    ParamsFor<u32, u32, u32>(Op::kDivide),
    ParamsFor<u32, u32, u32>(Op::kModulo),

    ParamsFor<f32, f32, f32>(Op::kAdd),
    ParamsFor<f32, f32, f32>(Op::kSubtract),
    ParamsFor<f32, f32, f32>(Op::kMultiply),
    ParamsFor<f32, f32, f32>(Op::kDivide),
    ParamsFor<f32, f32, f32>(Op::kModulo),

    // Binary arithmetic expressions over vectors
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kAdd),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kSubtract),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kMultiply),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kDivide),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kModulo),

    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kAdd),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kSubtract),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kMultiply),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kDivide),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kModulo),

    ParamsFor<vec3<f32>, vec3<f32>, vec3<f32>>(Op::kAdd),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<f32>>(Op::kSubtract),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<f32>>(Op::kMultiply),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<f32>>(Op::kDivide),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<f32>>(Op::kModulo),

    // Binary arithmetic expressions with mixed scalar and vector operands
    ParamsFor<vec3<i32>, i32, vec3<i32>>(Op::kAdd),
    ParamsFor<vec3<i32>, i32, vec3<i32>>(Op::kSubtract),
    ParamsFor<vec3<i32>, i32, vec3<i32>>(Op::kMultiply),
    ParamsFor<vec3<i32>, i32, vec3<i32>>(Op::kDivide),
    ParamsFor<vec3<i32>, i32, vec3<i32>>(Op::kModulo),

    ParamsFor<i32, vec3<i32>, vec3<i32>>(Op::kAdd),
    ParamsFor<i32, vec3<i32>, vec3<i32>>(Op::kSubtract),
    ParamsFor<i32, vec3<i32>, vec3<i32>>(Op::kMultiply),
    ParamsFor<i32, vec3<i32>, vec3<i32>>(Op::kDivide),
    ParamsFor<i32, vec3<i32>, vec3<i32>>(Op::kModulo),

    ParamsFor<vec3<u32>, u32, vec3<u32>>(Op::kAdd),
    ParamsFor<vec3<u32>, u32, vec3<u32>>(Op::kSubtract),
    ParamsFor<vec3<u32>, u32, vec3<u32>>(Op::kMultiply),
    ParamsFor<vec3<u32>, u32, vec3<u32>>(Op::kDivide),
    ParamsFor<vec3<u32>, u32, vec3<u32>>(Op::kModulo),

    ParamsFor<u32, vec3<u32>, vec3<u32>>(Op::kAdd),
    ParamsFor<u32, vec3<u32>, vec3<u32>>(Op::kSubtract),
    ParamsFor<u32, vec3<u32>, vec3<u32>>(Op::kMultiply),
    ParamsFor<u32, vec3<u32>, vec3<u32>>(Op::kDivide),
    ParamsFor<u32, vec3<u32>, vec3<u32>>(Op::kModulo),

    ParamsFor<vec3<f32>, f32, vec3<f32>>(Op::kAdd),
    ParamsFor<vec3<f32>, f32, vec3<f32>>(Op::kSubtract),
    ParamsFor<vec3<f32>, f32, vec3<f32>>(Op::kMultiply),
    ParamsFor<vec3<f32>, f32, vec3<f32>>(Op::kDivide),
    ParamsFor<vec3<f32>, f32, vec3<f32>>(Op::kModulo),

    ParamsFor<f32, vec3<f32>, vec3<f32>>(Op::kAdd),
    ParamsFor<f32, vec3<f32>, vec3<f32>>(Op::kSubtract),
    ParamsFor<f32, vec3<f32>, vec3<f32>>(Op::kMultiply),
    ParamsFor<f32, vec3<f32>, vec3<f32>>(Op::kDivide),
    ParamsFor<f32, vec3<f32>, vec3<f32>>(Op::kModulo),

    // Matrix arithmetic
    ParamsFor<mat2x3<f32>, f32, mat2x3<f32>>(Op::kMultiply),
    ParamsFor<mat3x2<f32>, f32, mat3x2<f32>>(Op::kMultiply),
    ParamsFor<mat3x3<f32>, f32, mat3x3<f32>>(Op::kMultiply),

    ParamsFor<f32, mat2x3<f32>, mat2x3<f32>>(Op::kMultiply),
    ParamsFor<f32, mat3x2<f32>, mat3x2<f32>>(Op::kMultiply),
    ParamsFor<f32, mat3x3<f32>, mat3x3<f32>>(Op::kMultiply),

    ParamsFor<vec3<f32>, mat2x3<f32>, vec2<f32>>(Op::kMultiply),
    ParamsFor<vec2<f32>, mat3x2<f32>, vec3<f32>>(Op::kMultiply),
    ParamsFor<vec3<f32>, mat3x3<f32>, vec3<f32>>(Op::kMultiply),

    ParamsFor<mat3x2<f32>, vec3<f32>, vec2<f32>>(Op::kMultiply),
    ParamsFor<mat2x3<f32>, vec2<f32>, vec3<f32>>(Op::kMultiply),
    ParamsFor<mat3x3<f32>, vec3<f32>, vec3<f32>>(Op::kMultiply),

    ParamsFor<mat2x3<f32>, mat3x2<f32>, mat3x3<f32>>(Op::kMultiply),
    ParamsFor<mat3x2<f32>, mat2x3<f32>, mat2x2<f32>>(Op::kMultiply),
    ParamsFor<mat3x2<f32>, mat3x3<f32>, mat3x2<f32>>(Op::kMultiply),
    ParamsFor<mat3x3<f32>, mat3x3<f32>, mat3x3<f32>>(Op::kMultiply),
    ParamsFor<mat3x3<f32>, mat2x3<f32>, mat2x3<f32>>(Op::kMultiply),

    ParamsFor<mat2x3<f32>, mat2x3<f32>, mat2x3<f32>>(Op::kAdd),
    ParamsFor<mat3x2<f32>, mat3x2<f32>, mat3x2<f32>>(Op::kAdd),
    ParamsFor<mat3x3<f32>, mat3x3<f32>, mat3x3<f32>>(Op::kAdd),

    ParamsFor<mat2x3<f32>, mat2x3<f32>, mat2x3<f32>>(Op::kSubtract),
    ParamsFor<mat3x2<f32>, mat3x2<f32>, mat3x2<f32>>(Op::kSubtract),
    ParamsFor<mat3x3<f32>, mat3x3<f32>, mat3x3<f32>>(Op::kSubtract),

    // Comparison expressions
    // https://gpuweb.github.io/gpuweb/wgsl.html#comparison-expr

    // Comparisons over scalars
    ParamsFor<bool, bool, bool>(Op::kEqual),
    ParamsFor<bool, bool, bool>(Op::kNotEqual),

    ParamsFor<i32, i32, bool>(Op::kEqual),
    ParamsFor<i32, i32, bool>(Op::kNotEqual),
    ParamsFor<i32, i32, bool>(Op::kLessThan),
    ParamsFor<i32, i32, bool>(Op::kLessThanEqual),
    ParamsFor<i32, i32, bool>(Op::kGreaterThan),
    ParamsFor<i32, i32, bool>(Op::kGreaterThanEqual),

    ParamsFor<u32, u32, bool>(Op::kEqual),
    ParamsFor<u32, u32, bool>(Op::kNotEqual),
    ParamsFor<u32, u32, bool>(Op::kLessThan),
    ParamsFor<u32, u32, bool>(Op::kLessThanEqual),
    ParamsFor<u32, u32, bool>(Op::kGreaterThan),
    ParamsFor<u32, u32, bool>(Op::kGreaterThanEqual),

    ParamsFor<f32, f32, bool>(Op::kEqual),
    ParamsFor<f32, f32, bool>(Op::kNotEqual),
    ParamsFor<f32, f32, bool>(Op::kLessThan),
    ParamsFor<f32, f32, bool>(Op::kLessThanEqual),
    ParamsFor<f32, f32, bool>(Op::kGreaterThan),
    ParamsFor<f32, f32, bool>(Op::kGreaterThanEqual),

    // Comparisons over vectors
    ParamsFor<vec3<bool>, vec3<bool>, vec3<bool>>(Op::kEqual),
    ParamsFor<vec3<bool>, vec3<bool>, vec3<bool>>(Op::kNotEqual),

    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kEqual),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kNotEqual),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kLessThan),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kLessThanEqual),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kGreaterThan),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kGreaterThanEqual),

    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kEqual),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kNotEqual),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kLessThan),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kLessThanEqual),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kGreaterThan),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kGreaterThanEqual),

    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kEqual),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kNotEqual),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kLessThan),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kLessThanEqual),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kGreaterThan),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kGreaterThanEqual),

    // Binary bitwise operations
    ParamsFor<i32, i32, i32>(Op::kOr),
    ParamsFor<i32, i32, i32>(Op::kAnd),
    ParamsFor<i32, i32, i32>(Op::kXor),

    ParamsFor<u32, u32, u32>(Op::kOr),
    ParamsFor<u32, u32, u32>(Op::kAnd),
    ParamsFor<u32, u32, u32>(Op::kXor),

    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kOr),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kAnd),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kXor),

    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kOr),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kAnd),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kXor),

    // Bit shift expressions
    ParamsFor<i32, u32, i32>(Op::kShiftLeft),
    ParamsFor<vec3<i32>, vec3<u32>, vec3<i32>>(Op::kShiftLeft),

    ParamsFor<u32, u32, u32>(Op::kShiftLeft),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kShiftLeft),

    ParamsFor<i32, u32, i32>(Op::kShiftRight),
    ParamsFor<vec3<i32>, vec3<u32>, vec3<i32>>(Op::kShiftRight),

    ParamsFor<u32, u32, u32>(Op::kShiftRight),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kShiftRight),
};

using Expr_Binary_Test_Valid = ResolverTestWithParam<Params>;
TEST_P(Expr_Binary_Test_Valid, All) {
    auto& params = GetParam();

    ast::Type lhs_type = params.create_lhs_type(*this);
    ast::Type rhs_type = params.create_rhs_type(*this);
    auto* result_type = params.create_result_type(*this);

    StringStream ss;
    ss << FriendlyName(lhs_type) << " " << params.op << " " << FriendlyName(rhs_type);
    SCOPED_TRACE(ss.str());

    GlobalVar("lhs", lhs_type, core::AddressSpace::kPrivate);
    GlobalVar("rhs", rhs_type, core::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(params.op, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr) == result_type);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest, Expr_Binary_Test_Valid, testing::ValuesIn(all_valid_cases));

enum class BinaryExprSide { Left, Right, Both };
using Expr_Binary_Test_WithAlias_Valid = ResolverTestWithParam<std::tuple<Params, BinaryExprSide>>;
TEST_P(Expr_Binary_Test_WithAlias_Valid, All) {
    const Params& params = std::get<0>(GetParam());
    BinaryExprSide side = std::get<1>(GetParam());

    auto* create_lhs_type = (side == BinaryExprSide::Left || side == BinaryExprSide::Both)
                                ? params.create_lhs_alias_type
                                : params.create_lhs_type;
    auto* create_rhs_type = (side == BinaryExprSide::Right || side == BinaryExprSide::Both)
                                ? params.create_rhs_alias_type
                                : params.create_rhs_type;

    ast::Type lhs_type = create_lhs_type(*this);
    ast::Type rhs_type = create_rhs_type(*this);

    StringStream ss;
    ss << FriendlyName(lhs_type) << " " << params.op << " " << FriendlyName(rhs_type);

    ss << ", After aliasing: " << FriendlyName(lhs_type) << " " << params.op << " "
       << FriendlyName(rhs_type);
    SCOPED_TRACE(ss.str());

    GlobalVar("lhs", lhs_type, core::AddressSpace::kPrivate);
    GlobalVar("rhs", rhs_type, core::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(params.op, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(expr), nullptr);
    // TODO(amaiorano): Bring this back once we have a way to get the canonical
    // type
    // auto* *result_type = params.create_result_type(*this);
    // ASSERT_TRUE(TypeOf(expr) == result_type);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_Test_WithAlias_Valid,
                         testing::Combine(testing::ValuesIn(all_valid_cases),
                                          testing::Values(BinaryExprSide::Left,
                                                          BinaryExprSide::Right,
                                                          BinaryExprSide::Both)));

// This test works by taking the cartesian product of all possible
// (type * type * op), and processing only the triplets that are not found in
// the `all_valid_cases` table.
using Expr_Binary_Test_Invalid = ResolverTestWithParam<
    std::tuple<builder::ast_type_func_ptr, builder::ast_type_func_ptr, core::BinaryOp>>;
TEST_P(Expr_Binary_Test_Invalid, All) {
    const builder::ast_type_func_ptr& lhs_create_type_func = std::get<0>(GetParam());
    const builder::ast_type_func_ptr& rhs_create_type_func = std::get<1>(GetParam());
    const core::BinaryOp op = std::get<2>(GetParam());

    // Skip if valid case
    // TODO(amaiorano): replace linear lookup with O(1) if too slow
    for (auto& c : all_valid_cases) {
        if (c.create_lhs_type == lhs_create_type_func &&
            c.create_rhs_type == rhs_create_type_func && c.op == op) {
            return;
        }
    }

    ast::Type lhs_type = lhs_create_type_func(*this);
    ast::Type rhs_type = rhs_create_type_func(*this);

    StringStream ss;
    ss << FriendlyName(lhs_type) << " " << op << " " << FriendlyName(rhs_type);
    SCOPED_TRACE(ss.str());

    GlobalVar("lhs", lhs_type, core::AddressSpace::kPrivate);
    GlobalVar("rhs", rhs_type, core::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(Source{{12, 34}}, op, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching overload for 'operator "));
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_Test_Invalid,
                         testing::Combine(testing::ValuesIn(all_create_type_funcs),
                                          testing::ValuesIn(all_create_type_funcs),
                                          testing::ValuesIn(all_ops)));

using Expr_Binary_Test_Invalid_VectorMatrixMultiply =
    ResolverTestWithParam<std::tuple<bool, uint32_t, uint32_t, uint32_t>>;
TEST_P(Expr_Binary_Test_Invalid_VectorMatrixMultiply, All) {
    bool vec_by_mat = std::get<0>(GetParam());
    uint32_t vec_size = std::get<1>(GetParam());
    uint32_t mat_rows = std::get<2>(GetParam());
    uint32_t mat_cols = std::get<3>(GetParam());

    ast::Type lhs_type;
    ast::Type rhs_type;
    const core::type::Type* result_type = nullptr;
    bool is_valid_expr;

    if (vec_by_mat) {
        lhs_type = ty.vec<f32>(vec_size);
        rhs_type = ty.mat<f32>(mat_cols, mat_rows);
        result_type = create<core::type::Vector>(create<core::type::F32>(), mat_cols);
        is_valid_expr = vec_size == mat_rows;
    } else {
        lhs_type = ty.mat<f32>(mat_cols, mat_rows);
        rhs_type = ty.vec<f32>(vec_size);
        result_type = create<core::type::Vector>(create<core::type::F32>(), mat_rows);
        is_valid_expr = vec_size == mat_cols;
    }

    GlobalVar("lhs", lhs_type, core::AddressSpace::kPrivate);
    GlobalVar("rhs", rhs_type, core::AddressSpace::kPrivate);

    auto* expr = Mul(Source{{12, 34}}, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    if (is_valid_expr) {
        ASSERT_TRUE(r()->Resolve()) << r()->error();
        ASSERT_TRUE(TypeOf(expr) == result_type);
    } else {
        ASSERT_FALSE(r()->Resolve());
        EXPECT_THAT(r()->error(), HasSubstr("no matching overload for 'operator *"));
    }
}
auto all_dimension_values = testing::Values(2u, 3u, 4u);
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_Test_Invalid_VectorMatrixMultiply,
                         testing::Combine(testing::Values(true, false),
                                          all_dimension_values,
                                          all_dimension_values,
                                          all_dimension_values));

using Expr_Binary_Test_Invalid_MatrixMatrixMultiply =
    ResolverTestWithParam<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>>;
TEST_P(Expr_Binary_Test_Invalid_MatrixMatrixMultiply, All) {
    uint32_t lhs_mat_rows = std::get<0>(GetParam());
    uint32_t lhs_mat_cols = std::get<1>(GetParam());
    uint32_t rhs_mat_rows = std::get<2>(GetParam());
    uint32_t rhs_mat_cols = std::get<3>(GetParam());

    auto lhs_type = ty.mat<f32>(lhs_mat_cols, lhs_mat_rows);
    auto rhs_type = ty.mat<f32>(rhs_mat_cols, rhs_mat_rows);

    auto* f32 = create<core::type::F32>();
    auto* col = create<core::type::Vector>(f32, lhs_mat_rows);
    auto* result_type = create<core::type::Matrix>(col, rhs_mat_cols);

    GlobalVar("lhs", lhs_type, core::AddressSpace::kPrivate);
    GlobalVar("rhs", rhs_type, core::AddressSpace::kPrivate);

    auto* expr = Mul(Source{{12, 34}}, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    bool is_valid_expr = lhs_mat_cols == rhs_mat_rows;
    if (is_valid_expr) {
        ASSERT_TRUE(r()->Resolve()) << r()->error();
        ASSERT_TRUE(TypeOf(expr) == result_type);
    } else {
        ASSERT_FALSE(r()->Resolve());
        EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching overload for 'operator * "));
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_Test_Invalid_MatrixMatrixMultiply,
                         testing::Combine(all_dimension_values,
                                          all_dimension_values,
                                          all_dimension_values,
                                          all_dimension_values));

}  // namespace ExprBinaryTest

using UnaryOpExpressionTest = ResolverTestWithParam<core::UnaryOp>;
TEST_P(UnaryOpExpressionTest, Expr_UnaryOp) {
    auto op = GetParam();

    if (op == core::UnaryOp::kNot) {
        GlobalVar("ident", ty.vec4<bool>(), core::AddressSpace::kPrivate);
    } else if (op == core::UnaryOp::kNegation || op == core::UnaryOp::kComplement) {
        GlobalVar("ident", ty.vec4<i32>(), core::AddressSpace::kPrivate);
    } else {
        GlobalVar("ident", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    }
    auto* der = create<ast::UnaryOpExpression>(op, Expr("ident"));
    WrapInFunction(der);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(der), nullptr);
    ASSERT_TRUE(TypeOf(der)->Is<core::type::Vector>());
    if (op == core::UnaryOp::kNot) {
        EXPECT_TRUE(TypeOf(der)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    } else if (op == core::UnaryOp::kNegation || op == core::UnaryOp::kComplement) {
        EXPECT_TRUE(TypeOf(der)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    } else {
        EXPECT_TRUE(TypeOf(der)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    }
    EXPECT_EQ(TypeOf(der)->As<core::type::Vector>()->Width(), 4u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         UnaryOpExpressionTest,
                         testing::Values(core::UnaryOp::kComplement,
                                         core::UnaryOp::kNegation,
                                         core::UnaryOp::kNot));

TEST_F(ResolverTest, AddressSpace_SetsIfMissing) {
    auto* var = Var("var", ty.i32());

    auto* stmt = Decl(var);
    Func("func", tint::Empty, ty.void_(), Vector{stmt});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(var)->AddressSpace(), core::AddressSpace::kFunction);
}

TEST_F(ResolverTest, AddressSpace_SetForSampler) {
    auto t = ty.sampler(core::type::SamplerKind::kSampler);
    auto* var = GlobalVar("var", t, Binding(0_a), Group(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(var)->AddressSpace(), core::AddressSpace::kHandle);
}

TEST_F(ResolverTest, AddressSpace_SetForTexture) {
    auto t = ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32());
    auto* var = GlobalVar("var", t, Binding(0_a), Group(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(var)->AddressSpace(), core::AddressSpace::kHandle);
}

TEST_F(ResolverTest, AddressSpace_DoesNotSetOnConst) {
    auto* var = Let("var", ty.i32(), Call<i32>());
    auto* stmt = Decl(var);
    Func("func", tint::Empty, ty.void_(), Vector{stmt});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(var)->AddressSpace(), core::AddressSpace::kUndefined);
}

TEST_F(ResolverTest, Access_SetForStorageBuffer) {
    // struct S { x : i32 };
    // var<storage> g : S;
    auto* s = Structure("S", Vector{Member(Source{{12, 34}}, "x", ty.i32())});
    auto* var = GlobalVar(Source{{56, 78}}, "g", ty.Of(s), core::AddressSpace::kStorage,
                          Binding(0_a), Group(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(var)->Access(), core::Access::kRead);
}

TEST_F(ResolverTest, BindingPoint_SetForResources) {
    // @group(1) @binding(2) var s1 : sampler;
    // @group(3) @binding(4) var s2 : sampler;
    auto* s1 =
        GlobalVar(Sym(), ty.sampler(core::type::SamplerKind::kSampler), Group(1_a), Binding(2_a));
    auto* s2 =
        GlobalVar(Sym(), ty.sampler(core::type::SamplerKind::kSampler), Group(3_a), Binding(4_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get<sem::GlobalVariable>(s1)->Attributes().binding_point,
              (BindingPoint{1u, 2u}));
    EXPECT_EQ(Sem().Get<sem::GlobalVariable>(s2)->Attributes().binding_point,
              (BindingPoint{3u, 4u}));
}

TEST_F(ResolverTest, Function_EntryPoints_StageAttribute) {
    // fn b() {}
    // fn c() { b(); }
    // fn a() { c(); }
    // fn ep_1() { a(); b(); }
    // fn ep_2() { c();}
    //
    // c -> {ep_1, ep_2}
    // a -> {ep_1}
    // b -> {ep_1, ep_2}
    // ep_1 -> {}
    // ep_2 -> {}

    GlobalVar("first", ty.f32(), core::AddressSpace::kPrivate);
    GlobalVar("second", ty.f32(), core::AddressSpace::kPrivate);
    GlobalVar("call_a", ty.f32(), core::AddressSpace::kPrivate);
    GlobalVar("call_b", ty.f32(), core::AddressSpace::kPrivate);
    GlobalVar("call_c", ty.f32(), core::AddressSpace::kPrivate);

    auto* func_b = Func("b", tint::Empty, ty.f32(),
                        Vector{
                            Return(0_f),
                        });
    auto* func_c = Func("c", tint::Empty, ty.f32(),
                        Vector{
                            Assign("second", Call("b")),
                            Return(0_f),
                        });

    auto* func_a = Func("a", tint::Empty, ty.f32(),
                        Vector{
                            Assign("first", Call("c")),
                            Return(0_f),
                        });

    auto* ep_1 = Func("ep_1", tint::Empty, ty.void_(),
                      Vector{
                          Assign("call_a", Call("a")),
                          Assign("call_b", Call("b")),
                      },
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(1_i),
                      });

    auto* ep_2 = Func("ep_2", tint::Empty, ty.void_(),
                      Vector{
                          Assign("call_c", Call("c")),
                      },
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(1_i),
                      });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* func_b_sem = Sem().Get(func_b);
    auto* func_a_sem = Sem().Get(func_a);
    auto* func_c_sem = Sem().Get(func_c);
    auto* ep_1_sem = Sem().Get(ep_1);
    auto* ep_2_sem = Sem().Get(ep_2);
    ASSERT_NE(func_b_sem, nullptr);
    ASSERT_NE(func_a_sem, nullptr);
    ASSERT_NE(func_c_sem, nullptr);
    ASSERT_NE(ep_1_sem, nullptr);
    ASSERT_NE(ep_2_sem, nullptr);

    EXPECT_EQ(func_b_sem->Parameters().Length(), 0u);
    EXPECT_EQ(func_a_sem->Parameters().Length(), 0u);
    EXPECT_EQ(func_c_sem->Parameters().Length(), 0u);

    const auto& b_eps = func_b_sem->CallGraphEntryPoints();
    ASSERT_EQ(2u, b_eps.Length());
    EXPECT_EQ(Symbols().Register("ep_1"), b_eps[0]->Declaration()->name->symbol);
    EXPECT_EQ(Symbols().Register("ep_2"), b_eps[1]->Declaration()->name->symbol);

    const auto& a_eps = func_a_sem->CallGraphEntryPoints();
    ASSERT_EQ(1u, a_eps.Length());
    EXPECT_EQ(Symbols().Register("ep_1"), a_eps[0]->Declaration()->name->symbol);

    const auto& c_eps = func_c_sem->CallGraphEntryPoints();
    ASSERT_EQ(2u, c_eps.Length());
    EXPECT_EQ(Symbols().Register("ep_1"), c_eps[0]->Declaration()->name->symbol);
    EXPECT_EQ(Symbols().Register("ep_2"), c_eps[1]->Declaration()->name->symbol);

    const auto& ep_1_eps = ep_1_sem->CallGraphEntryPoints();
    ASSERT_EQ(1u, ep_1_eps.Length());
    EXPECT_EQ(Symbols().Register("ep_1"), ep_1_eps[0]->Declaration()->name->symbol);

    const auto& ep_2_eps = ep_2_sem->CallGraphEntryPoints();
    ASSERT_EQ(1u, ep_2_eps.Length());
    EXPECT_EQ(Symbols().Register("ep_2"), ep_2_eps[0]->Declaration()->name->symbol);
}

// Check for linear-time traversal of functions reachable from entry points.
// See: crbug.com/tint/245
TEST_F(ResolverTest, Function_EntryPoints_LinearTime) {
    // fn lNa() { }
    // fn lNb() { }
    // ...
    // fn l2a() { l3a(); l3b(); }
    // fn l2b() { l3a(); l3b(); }
    // fn l1a() { l2a(); l2b(); }
    // fn l1b() { l2a(); l2b(); }
    // fn main() { l1a(); l1b(); }

    static constexpr int levels = 64;

    auto fn_a = [](int level) { return "l" + std::to_string(level + 1) + "a"; };
    auto fn_b = [](int level) { return "l" + std::to_string(level + 1) + "b"; };

    Func(fn_a(levels), tint::Empty, ty.void_(), tint::Empty);
    Func(fn_b(levels), tint::Empty, ty.void_(), tint::Empty);

    for (int i = levels - 1; i >= 0; i--) {
        Func(fn_a(i), tint::Empty, ty.void_(),
             Vector{
                 CallStmt(Call(fn_a(i + 1))),
                 CallStmt(Call(fn_b(i + 1))),
             },
             tint::Empty);
        Func(fn_b(i), tint::Empty, ty.void_(),
             Vector{
                 CallStmt(Call(fn_a(i + 1))),
                 CallStmt(Call(fn_b(i + 1))),
             },
             tint::Empty);
    }

    Func("main", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call(fn_a(0))),
             CallStmt(Call(fn_b(0))),
         },
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Test for crbug.com/tint/728
TEST_F(ResolverTest, ASTNodesAreReached) {
    Structure("A", Vector{Member("x", ty.array<f32, 4>(Vector{Stride(4)}))});
    Structure("B", Vector{Member("x", ty.array<f32, 4>(Vector{Stride(4)}))});
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverDeathTest, ASTNodeNotReached) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.Ident("ident");
            Resolver(&b, {}).Resolve();
        },
        "internal compiler error: AST node 'tint::ast::Identifier' was not reached by the "
        "resolver");
}

TEST_F(ResolverDeathTest, ASTNodeReachedTwice) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            auto* expr = b.Expr(1_i);
            b.GlobalVar("a", b.ty.i32(), core::AddressSpace::kPrivate, expr);
            b.GlobalVar("b", b.ty.i32(), core::AddressSpace::kPrivate, expr);
            Resolver(&b, {}).Resolve();
        },
        "internal compiler error: AST node 'tint::ast::IntLiteralExpression' was encountered twice "
        "in the same AST of a Program");
}

TEST_F(ResolverTest, UnaryOp_Not) {
    GlobalVar("ident", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    auto* der =
        create<ast::UnaryOpExpression>(core::UnaryOp::kNot, Expr(Source{{12, 34}}, "ident"));
    WrapInFunction(der);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("error: no matching overload for 'operator ! (vec4<f32>)"));
}

TEST_F(ResolverTest, UnaryOp_Complement) {
    GlobalVar("ident", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    auto* der =
        create<ast::UnaryOpExpression>(core::UnaryOp::kComplement, Expr(Source{{12, 34}}, "ident"));
    WrapInFunction(der);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("error: no matching overload for 'operator ~ (vec4<f32>)"));
}

TEST_F(ResolverTest, UnaryOp_Negation) {
    GlobalVar("ident", ty.u32(), core::AddressSpace::kPrivate);
    auto* der =
        create<ast::UnaryOpExpression>(core::UnaryOp::kNegation, Expr(Source{{12, 34}}, "ident"));
    WrapInFunction(der);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("error: no matching overload for 'operator - (u32)"));
}

TEST_F(ResolverTest, TextureSampler_Bug1715) {  // crbug.com/tint/1715
    // @binding(0) @group(0) var s: sampler;
    // @binding(1) @group(0) var t: texture_2d<f32>;
    // @binding(2) @group(0) var<uniform> c: vec2<f32>;
    //
    // @fragment
    // fn main() -> @location(0) vec4<f32> {
    //     return helper(&s, &t);
    // }
    //
    // fn helper(sl: ptr<function, sampler>, tl: ptr<function, texture_2d<f32>>) -> vec4<f32> {
    //     return textureSampleLevel(*tl, *sl, c, 0.0);
    // }
    GlobalVar("s", ty.sampler(core::type::SamplerKind::kSampler), Group(0_a), Binding(0_a));
    GlobalVar("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Group(0_a),
              Binding(1_a));
    GlobalVar("c", ty.vec2<f32>(), core::AddressSpace::kUniform, Group(0_a), Binding(2_a));

    Func("main", tint::Empty, ty.vec4<f32>(),
         Vector{
             Return(Call("helper", AddressOf("s"), AddressOf("t"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_u),
         });

    Func("helper",
         Vector{
             Param("sl", ty.ptr<function>(ty.sampler(core::type::SamplerKind::kSampler))),
             Param("tl", ty.ptr<function>(
                             ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()))),
         },
         ty.vec4<f32>(),
         Vector{
             Return(Call("textureSampleLevel", Deref("tl"), Deref("sl"), "c", 0.0_a)),
         });

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: pointer can not be formed to handle type sampler");
}

TEST_F(ResolverTest, ModuleDependencyOrderedDeclarations) {
    auto* f0 = Func("f0", tint::Empty, ty.void_(), tint::Empty);
    auto* v0 = GlobalVar("v0", ty.i32(), core::AddressSpace::kPrivate);
    auto* a0 = Alias("a0", ty.i32());
    auto* s0 = Structure("s0", Vector{Member("m", ty.i32())});
    auto* f1 = Func("f1", tint::Empty, ty.void_(), tint::Empty);
    auto* v1 = GlobalVar("v1", ty.i32(), core::AddressSpace::kPrivate);
    auto* a1 = Alias("a1", ty.i32());
    auto* s1 = Structure("s1", Vector{Member("m", ty.i32())});
    auto* f2 = Func("f2", tint::Empty, ty.void_(), tint::Empty);
    auto* v2 = GlobalVar("v2", ty.i32(), core::AddressSpace::kPrivate);
    auto* a2 = Alias("a2", ty.i32());
    auto* s2 = Structure("s2", Vector{Member("m", ty.i32())});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(Sem().Module(), nullptr);
    EXPECT_THAT(Sem().Module()->DependencyOrderedDeclarations(),
                ElementsAre(f0, v0, a0, s0, f1, v1, a1, s1, f2, v2, a2, s2));
}

constexpr size_t kMaxExpressionDepth = 512U;

TEST_F(ResolverTest, MaxExpressionDepth_Pass) {
    auto* b = Var("b", ty.i32());
    const ast::Expression* chain = nullptr;
    for (size_t i = 0; i < kMaxExpressionDepth; ++i) {
        chain = Add(chain ? chain : Expr("b"), Expr("b"));
    }
    auto* a = Let("a", chain);
    WrapInFunction(b, a);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, MaxExpressionDepth_Fail) {
    auto* b = Var("b", ty.i32());
    const ast::Expression* chain = nullptr;
    for (size_t i = 0; i < kMaxExpressionDepth + 1; ++i) {
        chain = Add(chain ? chain : Expr("b"), Expr("b"));
    }
    auto* a = Let("a", chain);
    WrapInFunction(b, a);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("error: reached max expression depth of " +
                                        std::to_string(kMaxExpressionDepth)));
}

// Windows debug builds have significantly smaller stack than other builds, and these tests will
// stack overflow.
#if !defined(NDEBUG)

TEST_F(ResolverTest, ScopeDepth_NestedBlocks) {
    const ast::Statement* stmt = Return();
    for (uint32_t i = 0; i < 150; i++) {
        stmt = Block(Source{{i, 1}}, stmt);
    }
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "23:1 error: statement nesting depth / chaining length exceeds limit of 127");
}

TEST_F(ResolverTest, ScopeDepth_NestedIf) {
    const ast::Statement* stmt = Return();
    for (uint32_t i = 0; i < 150; i++) {
        stmt = If(Source{{i, 1}}, false, Block(Source{{i, 2}}, stmt));
    }
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "86:1 error: statement nesting depth / chaining length exceeds limit of 127");
}

TEST_F(ResolverTest, ScopeDepth_IfElseChain) {
    const ast::Statement* stmt = nullptr;
    for (uint32_t i = 0; i < 150; i++) {
        stmt = If(Source{{i, 1}}, false, Block(Source{{i, 2}}), Else(stmt));
    }
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "24:2 error: statement nesting depth / chaining length exceeds limit of 127");
}

#endif  // !defined(NDEBUG)

const size_t kMaxNumStructMembers = 16383;

TEST_F(ResolverTest, MaxNumStructMembers_Valid) {
    Vector<const ast::StructMember*, 0> members;
    members.Reserve(kMaxNumStructMembers);
    for (size_t i = 0; i < kMaxNumStructMembers; ++i) {
        members.Push(Member("m" + std::to_string(i), ty.i32()));
    }
    Structure("S", std::move(members));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, MaxNumStructMembers_Invalid) {
    Vector<const ast::StructMember*, 0> members;
    members.Reserve(kMaxNumStructMembers + 1);
    for (size_t i = 0; i < kMaxNumStructMembers + 1; ++i) {
        members.Push(Member("m" + std::to_string(i), ty.i32()));
    }
    Structure(Source{{12, 34}}, "S", std::move(members));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'struct S' has 16384 members, maximum is 16383");
}

TEST_F(ResolverTest, MaxNumStructMembers_WithIgnoreStructMemberLimit_Valid) {
    Vector<const ast::StructMember*, 0> members;
    members.Reserve(kMaxNumStructMembers);
    for (size_t i = 0; i < kMaxNumStructMembers; ++i) {
        members.Push(Member("m" + std::to_string(i), ty.i32()));
    }

    // Add 10 more members, but we set the limit to be ignored on the struct
    for (size_t i = 0; i < 10; ++i) {
        members.Push(Member("ignored" + std::to_string(i), ty.i32()));
    }

    Structure("S", std::move(members),
              Vector{Disable(ast::DisabledValidation::kIgnoreStructMemberLimit)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

uint32_t kMaxNestDepthOfCompositeType = 255;

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_Structs_Valid) {
    auto* s = Structure("S", Vector{Member("m", ty.i32())});
    uint32_t depth = 1;  // Depth of struct
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth;
    for (uint32_t i = 0; i < iterations; ++i) {
        s = Structure("S" + std::to_string(i), Vector{Member("m", ty.Of(s))});
    }
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_Structs_Invalid) {
    auto* s = Structure("S", Vector{Member("m", ty.i32())});
    uint32_t depth = 1;  // Depth of struct
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth + 1;
    for (uint32_t i = 0; i < iterations; ++i) {
        auto source = i == iterations - 1 ? Source{{12, 34}} : Source{{0, i}};
        s = Structure(source, "S" + std::to_string(i), Vector{Member("m", ty.Of(s))});
    }
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'struct S254' has nesting depth of 256, maximum is 255");
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_StructsWithVector_Valid) {
    auto* s = Structure("S", Vector{Member("m", ty.vec3<i32>())});
    uint32_t depth = 2;  // Despth of struct + vector
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth;
    for (uint32_t i = 0; i < iterations; ++i) {
        s = Structure("S" + std::to_string(i), Vector{Member("m", ty.Of(s))});
    }
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_StructsWithVector_Invalid) {
    auto* s = Structure("S", Vector{Member("m", ty.vec3<i32>())});
    uint32_t depth = 2;  // Despth of struct + vector
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth + 1;
    for (uint32_t i = 0; i < iterations; ++i) {
        auto source = i == iterations - 1 ? Source{{12, 34}} : Source{{0, i}};
        s = Structure(source, "S" + std::to_string(i), Vector{Member("m", ty.Of(s))});
    }
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'struct S253' has nesting depth of 256, maximum is 255");
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_StructsWithMatrix_Valid) {
    auto* s = Structure("S", Vector{Member("m", ty.mat3x3<f32>())});
    uint32_t depth = 3;  // Depth of struct + matrix
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth;
    for (uint32_t i = 0; i < iterations; ++i) {
        s = Structure("S" + std::to_string(i), Vector{Member("m", ty.Of(s))});
    }
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_StructsWithMatrix_Invalid) {
    auto* s = Structure("S", Vector{Member("m", ty.mat3x3<f32>())});
    uint32_t depth = 3;  // Depth of struct + matrix
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth + 1;
    for (uint32_t i = 0; i < iterations; ++i) {
        auto source = i == iterations - 1 ? Source{{12, 34}} : Source{{0, i}};
        s = Structure(source, "S" + std::to_string(i), Vector{Member("m", ty.Of(s))});
    }
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'struct S252' has nesting depth of 256, maximum is 255");
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_Arrays_Valid) {
    auto a = ty.array(ty.i32(), 10_u);
    uint32_t depth = 1;  // Depth of array
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth;
    for (uint32_t i = 0; i < iterations; ++i) {
        a = ty.array(a, 1_u);
    }
    Alias("a", a);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_Arrays_Invalid) {
    auto a = ty.array(Source{{99, 88}}, ty.i32(), 10_u);
    uint32_t depth = 1;  // Depth of array
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth + 1;
    for (uint32_t i = 0; i < iterations; ++i) {
        auto source = (i == iterations - 1) ? Source{{12, 34}} : Source{{0, i}};
        a = ty.array(source, a, 1_u);
    }
    Alias("a", a);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array has nesting depth of 256, maximum is 255");
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_ArraysOfVector_Valid) {
    auto a = ty.array<vec3<i32>, 10>();
    uint32_t depth = 2;  // Depth of array + vector
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth;
    for (uint32_t i = 0; i < iterations; ++i) {
        a = ty.array(a, 1_u);
    }
    Alias("a", a);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_ArraysOfVector_Invalid) {
    auto a = ty.array(Source{{99, 88}}, ty.vec3<i32>(), 10_u);
    uint32_t depth = 2;  // Depth of array + vector
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth + 1;
    for (uint32_t i = 0; i < iterations; ++i) {
        auto source = (i == iterations - 1) ? Source{{12, 34}} : Source{{0, i}};
        a = ty.array(source, a, 1_u);
    }
    Alias("a", a);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array has nesting depth of 256, maximum is 255");
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_ArraysOfMatrix_Valid) {
    auto a = ty.array(ty.mat3x3<f32>(), 10_u);
    uint32_t depth = 3;  // Depth of array + matrix
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth;
    for (uint32_t i = 0; i < iterations; ++i) {
        a = ty.array(a, 1_u);
    }
    Alias("a", a);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_ArraysOfMatrix_Invalid) {
    auto a = ty.array(ty.mat3x3<f32>(), 10_u);
    uint32_t depth = 3;  // Depth of array + matrix
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth + 1;
    for (uint32_t i = 0; i < iterations; ++i) {
        auto source = (i == iterations - 1) ? Source{{12, 34}} : Source{{0, i}};
        a = ty.array(source, a, 1_u);
    }
    Alias("a", a);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array has nesting depth of 256, maximum is 255");
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_StructsOfArray_Valid) {
    auto a = ty.array(ty.mat3x3<f32>(), 10_u);
    auto* s = Structure("S", Vector{Member("m", a)});
    uint32_t depth = 4;  // Depth of struct + array + matrix
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth;
    for (uint32_t i = 0; i < iterations; ++i) {
        s = Structure("S" + std::to_string(i), Vector{Member("m", ty.Of(s))});
    }
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_StructsOfArray_Invalid) {
    auto a = ty.array(ty.mat3x3<f32>(), 10_u);
    auto* s = Structure("S", Vector{Member("m", a)});
    uint32_t depth = 4;  // Depth of struct + array + matrix
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth + 1;
    for (uint32_t i = 0; i < iterations; ++i) {
        auto source = i == iterations - 1 ? Source{{12, 34}} : Source{{0, i}};
        s = Structure(source, "S" + std::to_string(i), Vector{Member("m", ty.Of(s))});
    }
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'struct S251' has nesting depth of 256, maximum is 255");
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_ArraysOfStruct_Valid) {
    auto* s = Structure("S", Vector{Member("m", ty.mat3x3<f32>())});
    auto a = ty.array(ty.Of(s), 10_u);
    uint32_t depth = 4;  // Depth of array + struct + matrix
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth;
    for (uint32_t i = 0; i < iterations; ++i) {
        a = ty.array(a, 1_u);
    }
    Alias("a", a);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, MaxNestDepthOfCompositeType_ArraysOfStruct_Invalid) {
    auto* s = Structure("S", Vector{Member("m", ty.mat3x3<f32>())});
    auto a = ty.array(ty.Of(s), 10_u);
    uint32_t depth = 4;  // Depth of array + struct + matrix
    uint32_t iterations = kMaxNestDepthOfCompositeType - depth + 1;
    for (uint32_t i = 0; i < iterations; ++i) {
        auto source = (i == iterations - 1) ? Source{{12, 34}} : Source{{0, i}};
        a = ty.array(source, a, 1_u);
    }
    Alias("a", a);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array has nesting depth of 256, maximum is 255");
}

TEST_F(ResolverTest, PointerToHandleTextureParameter) {
    Func("helper",
         Vector{
             Param("sl", ty.ptr<function>(
                             Source{{12, 34}},
                             ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()))),
         },
         ty.void_(), {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: pointer can not be formed to handle type texture_1d<f32>");
}

TEST_F(ResolverTest, PointerToHandleTextureReturn) {
    Func("helper", {},
         ty.ptr<function>(Source{{12, 34}},
                          ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32())),
         {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: pointer can not be formed to handle type texture_1d<f32>");
}

TEST_F(ResolverTest, PointerToHandleSamplerParameter) {
    Func("helper",
         Vector{
             Param("sl", ty.ptr<function>(Source{{12, 34}},
                                          ty.sampler(core::type::SamplerKind::kSampler))),
         },
         ty.void_(), {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: pointer can not be formed to handle type sampler");
}

TEST_F(ResolverTest, PointerToHandleTextureParameterAlias) {
    auto* my_ty = Alias(
        "MyTy", ty.ptr<private_>(Source{{12, 34}},
                                 ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32())));
    Func("helper",
         Vector{
             Param("sl", ty.Of(my_ty)),
         },
         ty.void_(), {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: pointer can not be formed to handle type texture_1d<f32>");
}

TEST_F(ResolverTest, PointerToHandleSamplerParameterAlias) {
    auto* my_ty = Alias(
        "MyTy", ty.ptr<private_>(Source{{12, 34}}, ty.sampler(core::type::SamplerKind::kSampler)));
    Func("helper",
         Vector{
             Param("sl", ty.Of(my_ty)),
         },
         ty.void_(), {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: pointer can not be formed to handle type sampler");
}

TEST_F(ResolverTest, PointerToHandleTextureVar) {
    GlobalVar("s",
              ty.ptr<private_>(Source{{12, 34}},
                               ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32())),
              core::AddressSpace::kPrivate, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: pointer can not be formed to handle type texture_1d<f32>");
}

TEST_F(ResolverTest, PointerToHandleSamplerVar) {
    GlobalVar("s",
              ty.ptr<private_>(Source{{12, 34}}, ty.sampler(core::type::SamplerKind::kSampler)),
              Group(0_a), core::AddressSpace::kPrivate, Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: pointer can not be formed to handle type sampler");
}

TEST_F(ResolverTest, PointerToHandleTextureVarAlias) {
    auto* my_ty = Alias(
        "MyTy", ty.ptr<private_>(Source{{12, 34}},
                                 ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32())));
    GlobalVar("s", ty.Of(my_ty), core::AddressSpace::kPrivate, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: pointer can not be formed to handle type texture_1d<f32>");
}

TEST_F(ResolverTest, PointerToHandleSamplerVarAlias) {
    auto* my_ty = Alias(
        "MyTy", ty.ptr<private_>(Source{{12, 34}}, ty.sampler(core::type::SamplerKind::kSampler)));

    GlobalVar("s", ty.Of(my_ty), Group(0_a), core::AddressSpace::kPrivate, Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: pointer can not be formed to handle type sampler");
}

TEST_F(ResolverTest, PointerToHandleTextureAlias) {
    Alias("MyTy",
          ty.ptr<private_>(Source{{12, 34}},
                           ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32())));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: pointer can not be formed to handle type texture_1d<f32>");
}

TEST_F(ResolverTest, PointerToHandleSamplerAlias) {
    Alias("MyTy",
          ty.ptr<private_>(Source{{12, 34}}, ty.sampler(core::type::SamplerKind::kSampler)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: pointer can not be formed to handle type sampler");
}

}  // namespace
}  // namespace tint::resolver
