// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class ResolverRootIdentifierTest : public ResolverTest {};

TEST_F(ResolverRootIdentifierTest, GlobalPrivateVar) {
    auto* a = GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate);
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalWorkgroupVar) {
    auto* a = GlobalVar("a", ty.f32(), core::AddressSpace::kWorkgroup);
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalStorageVar) {
    auto* a = GlobalVar("a", ty.f32(), core::AddressSpace::kStorage, Group(0_a), Binding(0_a));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalUniformVar) {
    auto* a = GlobalVar("a", ty.f32(), core::AddressSpace::kUniform, Group(0_a), Binding(0_a));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalTextureVar) {
    auto* a = GlobalVar("a", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                        core::AddressSpace::kUndefined, Group(0_a), Binding(0_a));
    auto* expr = Expr(a);
    WrapInFunction(Call("textureDimensions", expr));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalOverride) {
    auto* a = Override("a", ty.f32(), Expr(1_f));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalConst) {
    auto* a = GlobalConst("a", ty.f32(), Expr(1_f));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, FunctionVar) {
    auto* a = Var("a", ty.f32());
    auto* expr = Expr(a);
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, FunctionLet) {
    auto* a = Let("a", ty.f32(), Expr(1_f));
    auto* expr = Expr(a);
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, Parameter) {
    auto* a = Param("a", ty.f32());
    auto* expr = Expr(a);
    Func("foo", Vector{a}, ty.void_(), Vector{WrapInStatement(expr)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, PointerParameter) {
    // fn foo(a : ptr<function, f32>)
    // {
    //   let b = a;
    // }
    auto* param = Param("a", ty.ptr<function, f32>());
    auto* expr_param = Expr(param);
    auto* let = Let("b", expr_param);
    auto* expr_let = Expr("b");
    Func("foo", Vector{param}, ty.void_(), Vector{WrapInStatement(let), WrapInStatement(expr_let)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_param = Sem().Get(param);
    EXPECT_EQ(Sem().GetVal(expr_param)->RootIdentifier(), sem_param);
    EXPECT_EQ(Sem().GetVal(expr_let)->RootIdentifier(), sem_param);
}

TEST_F(ResolverRootIdentifierTest, VarCopyVar) {
    // {
    //   var a : f32;
    //   var b = a;
    // }
    auto* a = Var("a", ty.f32());
    auto* expr_a = Expr(a);
    auto* b = Var("b", ty.f32(), expr_a);
    auto* expr_b = Expr(b);
    WrapInFunction(a, b, expr_b);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    auto* sem_b = Sem().Get(b);
    EXPECT_EQ(Sem().GetVal(expr_a)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().GetVal(expr_b)->RootIdentifier(), sem_b);
}

TEST_F(ResolverRootIdentifierTest, LetCopyVar) {
    // {
    //   var a : f32;
    //   let b = a;
    // }
    auto* a = Var("a", ty.f32());
    auto* expr_a = Expr(a);
    auto* b = Let("b", ty.f32(), expr_a);
    auto* expr_b = Expr(b);
    WrapInFunction(a, b, expr_b);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    auto* sem_b = Sem().Get(b);
    EXPECT_EQ(Sem().GetVal(expr_a)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().GetVal(expr_b)->RootIdentifier(), sem_b);
}

TEST_F(ResolverRootIdentifierTest, ThroughIndexAccessor) {
    // var<private> a : array<f32, 4u>;
    // {
    //   a[2i]
    // }
    auto* a = GlobalVar("a", ty.array<f32, 4>(), core::AddressSpace::kPrivate);
    auto* expr = IndexAccessor(a, 2_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, ThroughMemberAccessor) {
    // struct S { f : f32 }
    // var<private> a : S;
    // {
    //   a.f
    // }
    auto* S = Structure("S", Vector{Member("f", ty.f32())});
    auto* a = GlobalVar("a", ty.Of(S), core::AddressSpace::kPrivate);
    auto* expr = MemberAccessor(a, "f");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, ThroughPointers) {
    // var<private> a : f32;
    // {
    //   let a_ptr1 = &*&a;
    //   let a_ptr2 = &*a_ptr1;
    // }
    auto* a = GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate);
    auto* address_of_1 = AddressOf(a);
    auto* deref_1 = Deref(address_of_1);
    auto* address_of_2 = AddressOf(deref_1);
    auto* a_ptr1 = Let("a_ptr1", address_of_2);
    auto* deref_2 = Deref(a_ptr1);
    auto* address_of_3 = AddressOf(deref_2);
    auto* a_ptr2 = Let("a_ptr2", address_of_3);
    WrapInFunction(a_ptr1, a_ptr2);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(address_of_1)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().Get(address_of_2)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().Get(address_of_3)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().Get(deref_1)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().Get(deref_2)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, Literal) {
    auto* expr = Expr(1_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), nullptr);
}

TEST_F(ResolverRootIdentifierTest, FunctionReturnValue) {
    auto* expr = Call("min", 1_f, 2_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), nullptr);
}

TEST_F(ResolverRootIdentifierTest, BinaryExpression) {
    auto* a = Var("a", ty.f32());
    auto* expr = Add(a, Expr(1_f));
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), nullptr);
}

TEST_F(ResolverRootIdentifierTest, UnaryExpression) {
    auto* a = Var("a", ty.f32());
    auto* expr = create<ast::UnaryOpExpression>(core::UnaryOp::kNegation, Expr(a));
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), nullptr);
}

}  // namespace
}  // namespace tint::resolver
