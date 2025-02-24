// Copyright 2021 The Dawn & Tint Authors
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

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverIndexAccessorTest = ResolverTest;

TEST_F(ResolverIndexAccessorTest, Matrix_F32) {
    GlobalVar("my_var", ty.mat2x3<f32>(), core::AddressSpace::kPrivate);
    auto* acc = IndexAccessor("my_var", Expr(Source{{12, 34}}, 1_f));
    WrapInFunction(acc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverIndexAccessorTest, Matrix_Dynamic_Ref) {
    GlobalVar("my_var", ty.mat2x3<f32>(), core::AddressSpace::kPrivate);
    auto* idx = Var("idx", ty.i32(), Call<i32>());
    auto* acc = IndexAccessor("my_var", idx);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Matrix_BothDimensions_Dynamic_Ref) {
    GlobalVar("my_var", ty.mat4x4<f32>(), core::AddressSpace::kPrivate);
    auto* idx = Var("idx", ty.u32(), Expr(3_u));
    auto* idy = Var("idy", ty.u32(), Expr(2_u));
    auto* acc = IndexAccessor(IndexAccessor("my_var", idx), idy);
    WrapInFunction(Decl(idx), Decl(idy), acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Matrix_Dynamic) {
    GlobalConst("my_const", ty.mat2x3<f32>(), Call<mat2x3<f32>>());
    auto* idx = Var("idx", ty.i32(), Call<i32>());
    auto* acc = IndexAccessor("my_const", idx);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Matrix_XDimension_Dynamic) {
    GlobalConst("my_const", ty.mat4x4<f32>(), Call<mat4x4<f32>>());
    auto* idx = Var("idx", ty.u32(), Expr(3_u));
    auto* acc = IndexAccessor("my_const", idx);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");
}

TEST_F(ResolverIndexAccessorTest, Matrix_BothDimension_Dynamic) {
    GlobalConst("my_const", ty.mat4x4<f32>(), Call<mat4x4<f32>>());
    auto* idx = Var("idx", ty.u32(), Expr(3_u));
    auto* idy = Var("idy", ty.u32(), Expr(2_u));
    auto* acc = IndexAccessor(IndexAccessor("my_const", idx), idy);
    WrapInFunction(Decl(idx), Decl(idy), acc);

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");
}

TEST_F(ResolverIndexAccessorTest, Matrix) {
    GlobalVar("my_var", ty.mat2x3<f32>(), core::AddressSpace::kPrivate);

    auto* acc = IndexAccessor("my_var", 1_i);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    ASSERT_TRUE(TypeOf(acc)->Is<core::type::Vector>());
    EXPECT_EQ(TypeOf(acc)->As<core::type::Vector>()->Width(), 3u);

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Matrix_BothDimensions) {
    GlobalVar("my_var", ty.mat2x3<f32>(), core::AddressSpace::kPrivate);

    auto* acc = IndexAccessor(IndexAccessor("my_var", 0_i), 1_i);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    EXPECT_TRUE(TypeOf(acc)->Is<core::type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Vector_F32) {
    GlobalVar("my_var", ty.vec3<f32>(), core::AddressSpace::kPrivate);
    auto* acc = IndexAccessor("my_var", Expr(Source{{12, 34}}, 2_f));
    WrapInFunction(acc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverIndexAccessorTest, Vector_Dynamic_Ref) {
    GlobalVar("my_var", ty.vec3<f32>(), core::AddressSpace::kPrivate);
    auto* idx = Var("idx", ty.i32(), Expr(2_i));
    auto* acc = IndexAccessor("my_var", idx);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Vector_Dynamic) {
    GlobalConst("my_const", ty.vec3<f32>(), Call<vec3<f32>>());
    auto* idx = Var("idx", ty.i32(), Expr(2_i));
    auto* acc = IndexAccessor("my_const", idx);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverIndexAccessorTest, Vector) {
    GlobalConst("my_const", ty.vec3<f32>(), Call<vec3<f32>>());

    auto* acc = IndexAccessor("my_const", 2_i);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    EXPECT_TRUE(TypeOf(acc)->Is<core::type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_i32) {
    GlobalVar("my_var", ty.array<f32, 3>(), core::AddressSpace::kPrivate);
    auto* acc = IndexAccessor("my_var", 2_i);
    WrapInFunction(acc);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(acc)->Is<core::type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_u32) {
    GlobalVar("my_var", ty.array<f32, 3>(), core::AddressSpace::kPrivate);
    auto* acc = IndexAccessor("my_var", 2_u);
    WrapInFunction(acc);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(acc)->Is<core::type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_AInt) {
    GlobalVar("my_var", ty.array<f32, 3>(), core::AddressSpace::kPrivate);
    auto* acc = IndexAccessor("my_var", 2_a);
    WrapInFunction(acc);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(acc)->Is<core::type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Alias_Array) {
    auto* aary = Alias("myarrty", ty.array<f32, 3>());

    GlobalVar("my_var", ty.Of(aary), core::AddressSpace::kPrivate);

    auto* acc = IndexAccessor("my_var", 2_i);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    EXPECT_TRUE(TypeOf(acc)->Is<core::type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Array_Constant) {
    GlobalConst("my_const", ty.array<f32, 3>(), Call<array<f32, 3>>());

    auto* acc = IndexAccessor("my_const", 2_i);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    EXPECT_TRUE(TypeOf(acc)->Is<core::type::F32>());
}

TEST_F(ResolverIndexAccessorTest, Array_Dynamic_I32) {
    // let a : array<f32, 3> = 0;
    // var idx : i32 = 0;
    // var f : f32 = a[idx];
    auto* a = Let("a", ty.array<f32, 3>(), Call<array<f32, 3>>());
    auto* idx = Var("idx", ty.i32(), Call<i32>());
    auto* acc = IndexAccessor("a", Expr(Source{{12, 34}}, idx));
    auto* f = Var("f", ty.f32(), acc);
    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             Decl(a),
             Decl(idx),
             Decl(f),
         });

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_F32) {
    // let a : array<f32, 3>;
    // var f : f32 = a[2.0f];
    auto* a = Let("a", ty.array<f32, 3>(), Call<array<f32, 3>>());
    auto* f = Var("a_2", ty.f32(), IndexAccessor("a", Expr(Source{{12, 34}}, 2_f)));
    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             Decl(a),
             Decl(f),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_I32) {
    // let a : array<f32, 3>;
    // var f : f32 = a[2i];
    auto* a = Let("a", ty.array<f32, 3>(), Call<array<f32, 3>>());
    auto* acc = IndexAccessor("a", 2_i);
    auto* f = Var("a_2", ty.f32(), acc);
    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             Decl(a),
             Decl(f),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Expr_Deref_FuncGoodParent) {
    // fn func(p: ptr<function, vec4<f32>>) -> f32 {
    //     let idx: u32 = u32();
    //     let x: f32 = (*p)[idx];
    //     return x;
    // }
    auto* p = Param("p", ty.ptr<function, vec4<f32>>());
    auto* idx = Let("idx", ty.u32(), Call<u32>());
    auto* star_p = Deref(p);
    auto* acc = IndexAccessor(Source{{12, 34}}, star_p, idx);
    auto* x = Var("x", ty.f32(), acc);
    Func("func", Vector{p}, ty.f32(), Vector{Decl(idx), Decl(x), Return(x)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Expr_ImplicitDeref_FuncGoodParent) {
    // fn func(p: ptr<function, vec4<f32>>) -> f32 {
    //     let idx: u32 = u32();
    //     let x: f32 = p[idx];
    //     return x;
    // }
    auto* p = Param("p", ty.ptr<function, vec4<f32>>());
    auto* idx = Let("idx", ty.u32(), Call<u32>());
    auto* acc = IndexAccessor(Source{{12, 34}}, p, idx);
    auto* x = Var("x", ty.f32(), acc);
    Func("func", Vector{p}, ty.f32(), Vector{Decl(idx), Decl(x), Return(x)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Expr_Deref_FuncBadParent) {
    // fn func(p: ptr<function, vec4<f32>>) -> f32 {
    //     let idx: u32 = u32();
    //     let x: f32 = *p[idx];
    //     return x;
    // }
    auto* p = Param("p", ty.ptr<function, vec4<f32>>());
    auto* idx = Let("idx", ty.u32(), Call<u32>());
    auto* accessor_expr = IndexAccessor(Source{{12, 34}}, p, idx);
    auto* star_p = Deref(accessor_expr);
    auto* x = Var("x", ty.f32(), star_p);
    Func("func", Vector{p}, ty.f32(), Vector{Decl(idx), Decl(x), Return(x)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot dereference expression of type 'f32'");
}

TEST_F(ResolverIndexAccessorTest, Expr_Deref_BadParent) {
    // var param: vec4<f32>
    // let x: f32 = *(&param)[0];
    auto* param = Var("param", ty.vec4<f32>());
    auto* idx = Var("idx", ty.u32(), Call<u32>());
    auto* addressOf_expr = AddressOf(param);
    auto* accessor_expr = IndexAccessor(Source{{12, 34}}, addressOf_expr, idx);
    auto* star_p = Deref(accessor_expr);
    auto* x = Var("x", ty.f32(), star_p);
    WrapInFunction(param, idx, x);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot dereference expression of type 'f32'");
}

}  // namespace
}  // namespace tint::resolver
