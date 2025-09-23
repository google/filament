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

#include "gmock/gmock.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

struct ResolverVariableTest : public resolver::TestHelper, public testing::Test {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function-scope 'var'
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, LocalVar_NoInitializer) {
    // struct S { i : i32; }
    // alias A = S;
    // fn F(){
    //   var i : i32;
    //   var u : u32;
    //   var f : f32;
    //   var h : f16;
    //   var b : bool;
    //   var s : S;
    //   var a : A;
    // }

    Enable(wgsl::Extension::kF16);

    auto* S = Structure("S", Vector{Member("i", ty.i32())});
    auto* A = Alias("A", ty.Of(S));

    auto* i = Var("i", ty.i32());
    auto* u = Var("u", ty.u32());
    auto* f = Var("f", ty.f32());
    auto* h = Var("h", ty.f16());
    auto* b = Var("b", ty.bool_());
    auto* s = Var("s", ty.Of(S));
    auto* a = Var("a", ty.Of(A));

    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(i),
             Decl(u),
             Decl(f),
             Decl(h),
             Decl(b),
             Decl(s),
             Decl(a),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    // `var` declarations are always of reference type
    ASSERT_TRUE(TypeOf(i)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(u)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(f)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(h)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(b)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(s)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(a)->Is<core::type::Reference>());

    EXPECT_TRUE(TypeOf(i)->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(u)->As<core::type::Reference>()->StoreType()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(f)->As<core::type::Reference>()->StoreType()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(h)->As<core::type::Reference>()->StoreType()->Is<core::type::F16>());
    EXPECT_TRUE(TypeOf(b)->As<core::type::Reference>()->StoreType()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(s)->As<core::type::Reference>()->StoreType()->Is<core::type::Struct>());
    EXPECT_TRUE(TypeOf(a)->As<core::type::Reference>()->StoreType()->Is<core::type::Struct>());

    EXPECT_EQ(Sem().Get(i)->Initializer(), nullptr);
    EXPECT_EQ(Sem().Get(u)->Initializer(), nullptr);
    EXPECT_EQ(Sem().Get(f)->Initializer(), nullptr);
    EXPECT_EQ(Sem().Get(h)->Initializer(), nullptr);
    EXPECT_EQ(Sem().Get(b)->Initializer(), nullptr);
    EXPECT_EQ(Sem().Get(s)->Initializer(), nullptr);
    EXPECT_EQ(Sem().Get(a)->Initializer(), nullptr);
}

TEST_F(ResolverVariableTest, LocalVar_WithInitializer) {
    // struct S { i : i32; }
    // alias A = S;
    // fn F(){
    //   var i : i32 = 1i;
    //   var u : u32 = 1u;
    //   var f : f32 = 1.f;
    //   var h : f16 = 1.h;
    //   var b : bool = true;
    //   var s : S = S(1);
    //   var a : A = A(1);
    // }

    Enable(wgsl::Extension::kF16);

    auto* S = Structure("S", Vector{Member("i", ty.i32())});
    auto* A = Alias("A", ty.Of(S));

    auto* i_c = Expr(1_i);
    auto* u_c = Expr(1_u);
    auto* f_c = Expr(1_f);
    auto* h_c = Expr(1_h);
    auto* b_c = Expr(true);
    auto* s_c = Call(ty.Of(S), Expr(1_i));
    auto* a_c = Call(ty.Of(A), Expr(1_i));

    auto* i = Var("i", ty.i32(), i_c);
    auto* u = Var("u", ty.u32(), u_c);
    auto* f = Var("f", ty.f32(), f_c);
    auto* h = Var("h", ty.f16(), h_c);
    auto* b = Var("b", ty.bool_(), b_c);
    auto* s = Var("s", ty.Of(S), s_c);
    auto* a = Var("a", ty.Of(A), a_c);

    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(i),
             Decl(u),
             Decl(f),
             Decl(h),
             Decl(b),
             Decl(s),
             Decl(a),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    // `var` declarations are always of reference type
    ASSERT_TRUE(TypeOf(i)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(u)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(f)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(h)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(b)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(s)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(a)->Is<core::type::Reference>());

    EXPECT_EQ(TypeOf(i)->As<core::type::Reference>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(u)->As<core::type::Reference>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(f)->As<core::type::Reference>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(b)->As<core::type::Reference>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(s)->As<core::type::Reference>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(a)->As<core::type::Reference>()->Access(), core::Access::kReadWrite);

    EXPECT_TRUE(TypeOf(i)->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(u)->As<core::type::Reference>()->StoreType()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(f)->As<core::type::Reference>()->StoreType()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(h)->As<core::type::Reference>()->StoreType()->Is<core::type::F16>());
    EXPECT_TRUE(TypeOf(b)->As<core::type::Reference>()->StoreType()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(s)->As<core::type::Reference>()->StoreType()->Is<core::type::Struct>());
    EXPECT_TRUE(TypeOf(a)->As<core::type::Reference>()->StoreType()->Is<core::type::Struct>());

    EXPECT_EQ(Sem().Get(i)->Initializer()->Declaration(), i_c);
    EXPECT_EQ(Sem().Get(u)->Initializer()->Declaration(), u_c);
    EXPECT_EQ(Sem().Get(f)->Initializer()->Declaration(), f_c);
    EXPECT_EQ(Sem().Get(h)->Initializer()->Declaration(), h_c);
    EXPECT_EQ(Sem().Get(b)->Initializer()->Declaration(), b_c);
    EXPECT_EQ(Sem().Get(s)->Initializer()->Declaration(), s_c);
    EXPECT_EQ(Sem().Get(a)->Initializer()->Declaration(), a_c);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsAlias) {
    // type a = i32;
    //
    // fn F() {
    //   var a = false;
    // }

    auto* t = Alias("a", ty.i32());
    auto* v = Var("a", Expr(false));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(v)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(v);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsStruct) {
    // struct a {
    //   m : i32;
    // };
    //
    // fn F() {
    //   var a = true;
    // }

    auto* t = Structure("a", Vector{Member("m", ty.i32())});
    auto* v = Var("a", Expr(false));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(v)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(v);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsFunction) {
    // fn a() {
    //   var a = true;
    // }

    auto* v = Var("a", Expr(false));
    auto* f = Func("a", tint::Empty, ty.void_(), Vector{Decl(v)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* func = Sem().Get(f);
    ASSERT_NE(func, nullptr);

    auto* local = Sem().Get<sem::LocalVariable>(v);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), func);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsGlobalVar) {
    // var<private> a : i32;
    //
    // fn F() {
    //   var a = a;
    // }

    auto* g = GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);
    auto* v = Var("a", Expr("a"));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(v)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(v);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);

    auto* user_v =
        Sem().GetVal(local->Declaration()->initializer)->UnwrapLoad()->As<sem::VariableUser>();
    ASSERT_NE(user_v, nullptr);
    EXPECT_EQ(user_v->Variable(), global);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsGlobalConst) {
    // const a : i32 = 1i;
    //
    // fn X() {
    //   var a = (a == 123);
    // }

    auto* g = GlobalConst("a", ty.i32(), Expr(1_i));
    auto* v = Var("a", Expr("a"));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(v)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(v);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);

    auto* user_v = Sem().Get<sem::VariableUser>(local->Declaration()->initializer);
    ASSERT_NE(user_v, nullptr);
    EXPECT_EQ(user_v->Variable(), global);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsLocalVar) {
    // fn F() {
    //   var a : i32 = 1i; // x
    //   {
    //     var a = a; // y
    //   }
    // }

    auto* x = Var("a", ty.i32(), Expr(1_i));
    auto* y = Var("a", Expr("a"));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(x), Block(Decl(y))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_x = Sem().Get<sem::LocalVariable>(x);
    auto* local_y = Sem().Get<sem::LocalVariable>(y);

    ASSERT_NE(local_x, nullptr);
    ASSERT_NE(local_y, nullptr);
    EXPECT_EQ(local_y->Shadows(), local_x);

    auto* user_y =
        Sem().GetVal(local_y->Declaration()->initializer)->UnwrapLoad()->As<sem::VariableUser>();
    ASSERT_NE(user_y, nullptr);
    EXPECT_EQ(user_y->Variable(), local_x);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsLocalConst) {
    // fn F() {
    //   const a : i32 = 1i;
    //   {
    //     var a = (a == 123);
    //   }
    // }

    auto* c = Const("a", ty.i32(), Expr(1_i));
    auto* v = Var("a", Expr("a"));
    Func("X", tint::Empty, ty.void_(), Vector{Decl(c), Block(Decl(v))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_c = Sem().Get<sem::LocalVariable>(c);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);

    ASSERT_NE(local_c, nullptr);
    ASSERT_NE(local_v, nullptr);
    EXPECT_EQ(local_v->Shadows(), local_c);

    auto* user_v = Sem().Get<sem::VariableUser>(local_v->Declaration()->initializer);
    ASSERT_NE(user_v, nullptr);
    EXPECT_EQ(user_v->Variable(), local_c);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsLocalLet) {
    // fn F() {
    //   let a : i32 = 1i;
    //   {
    //     var a = (a == 123);
    //   }
    // }

    auto* l = Let("a", ty.i32(), Expr(1_i));
    auto* v = Var("a", Expr("a"));
    Func("X", tint::Empty, ty.void_(), Vector{Decl(l), Block(Decl(v))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_l = Sem().Get<sem::LocalVariable>(l);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);

    ASSERT_NE(local_l, nullptr);
    ASSERT_NE(local_v, nullptr);
    EXPECT_EQ(local_v->Shadows(), local_l);

    auto* user_v = Sem().Get<sem::VariableUser>(local_v->Declaration()->initializer);
    ASSERT_NE(user_v, nullptr);
    EXPECT_EQ(user_v->Variable(), local_l);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsParam) {
    // fn F(a : i32) {
    //   {
    //     var a = a;
    //   }
    // }

    auto* p = Param("a", ty.i32());
    auto* v = Var("a", Expr("a"));
    Func("X", Vector{p}, ty.void_(), Vector{Block(Decl(v))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* param = Sem().Get(p);
    auto* local = Sem().Get<sem::LocalVariable>(v);

    ASSERT_NE(param, nullptr);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), param);

    auto* user_v = Sem().Get<sem::VariableUser>(local->Declaration()->initializer);
    ASSERT_NE(user_v, nullptr);
    EXPECT_EQ(user_v->Variable(), param);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// 'let' declaration
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, LocalLet) {
    // struct S { i : i32; }
    // fn F(){
    //   var v : i32;
    //   let i : i32 = 1i;
    //   let u : u32 = 1u;
    //   let f : f32 = 1.f;
    //   let h : h32 = 1.h;
    //   let b : bool = true;
    //   let s : S = S(1);
    //   let a : A = A(1);
    //   let p : pointer<function, i32> = &v;
    // }

    Enable(wgsl::Extension::kF16);

    auto* S = Structure("S", Vector{Member("i", ty.i32())});
    auto* A = Alias("A", ty.Of(S));
    auto* v = Var("v", ty.i32());

    auto* i_c = Expr(1_i);
    auto* u_c = Expr(1_u);
    auto* f_c = Expr(1_f);
    auto* h_c = Expr(1_h);
    auto* b_c = Expr(true);
    auto* s_c = Call(ty.Of(S), Expr(1_i));
    auto* a_c = Call(ty.Of(A), Expr(1_i));
    auto* p_c = AddressOf(v);

    auto* i = Let("i", ty.i32(), i_c);
    auto* u = Let("u", ty.u32(), u_c);
    auto* f = Let("f", ty.f32(), f_c);
    auto* h = Let("h", ty.f16(), h_c);
    auto* b = Let("b", ty.bool_(), b_c);
    auto* s = Let("s", ty.Of(S), s_c);
    auto* a = Let("a", ty.Of(A), a_c);
    auto* p = Let("p", ty.ptr<function, i32>(), p_c);

    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(v),
             Decl(i),
             Decl(u),
             Decl(f),
             Decl(h),
             Decl(b),
             Decl(s),
             Decl(a),
             Decl(p),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    // `let` declarations are always of the storage type
    ASSERT_TRUE(TypeOf(i)->Is<core::type::I32>());
    ASSERT_TRUE(TypeOf(u)->Is<core::type::U32>());
    ASSERT_TRUE(TypeOf(f)->Is<core::type::F32>());
    ASSERT_TRUE(TypeOf(h)->Is<core::type::F16>());
    ASSERT_TRUE(TypeOf(b)->Is<core::type::Bool>());
    ASSERT_TRUE(TypeOf(s)->Is<core::type::Struct>());
    ASSERT_TRUE(TypeOf(a)->Is<core::type::Struct>());
    ASSERT_TRUE(TypeOf(p)->Is<core::type::Pointer>());
    ASSERT_TRUE(TypeOf(p)->As<core::type::Pointer>()->StoreType()->Is<core::type::I32>());

    EXPECT_EQ(Sem().Get(i)->Initializer()->Declaration(), i_c);
    EXPECT_EQ(Sem().Get(u)->Initializer()->Declaration(), u_c);
    EXPECT_EQ(Sem().Get(f)->Initializer()->Declaration(), f_c);
    EXPECT_EQ(Sem().Get(h)->Initializer()->Declaration(), h_c);
    EXPECT_EQ(Sem().Get(b)->Initializer()->Declaration(), b_c);
    EXPECT_EQ(Sem().Get(s)->Initializer()->Declaration(), s_c);
    EXPECT_EQ(Sem().Get(a)->Initializer()->Declaration(), a_c);
    EXPECT_EQ(Sem().Get(p)->Initializer()->Declaration(), p_c);
}

TEST_F(ResolverVariableTest, LocalLet_InheritsAccessFromOriginatingVariable) {
    // struct Inner {
    //    arr: array<i32, 4>;
    // }
    // struct S {
    //    inner: Inner;
    // }
    // @group(0) @binding(0) var<storage, read_write> s : S;
    // fn f() {
    //   let p = &s.inner.arr[3];
    // }
    auto* inner = Structure("Inner", Vector{Member("arr", ty.array<i32, 4>())});
    auto* buf = Structure("S", Vector{Member("inner", ty.Of(inner))});
    auto* storage = GlobalVar("s", ty.Of(buf), core::AddressSpace::kStorage,
                              core::Access::kReadWrite, Binding(0_a), Group(0_a));

    auto* expr = IndexAccessor(MemberAccessor(MemberAccessor(storage, "inner"), "arr"), 3_i);
    auto* ptr = Let("p", AddressOf(expr));

    WrapInFunction(ptr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(expr)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(ptr)->Is<core::type::Pointer>());

    EXPECT_EQ(TypeOf(expr)->As<core::type::Reference>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(ptr)->As<core::type::Pointer>()->Access(), core::Access::kReadWrite);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsAlias) {
    // type a = i32;
    //
    // fn F() {
    //   let a = true;
    // }

    auto* t = Alias("a", ty.i32());
    auto* l = Let("a", Expr(false));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(l);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsStruct) {
    // struct a {
    //   m : i32;
    // };
    //
    // fn F() {
    //   let a = false;
    // }

    auto* t = Structure("a", Vector{Member("m", ty.i32())});
    auto* l = Let("a", Expr(false));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(l);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsFunction) {
    // fn a() {
    //   let a = false;
    // }

    auto* l = Let("a", Expr(false));
    auto* fb = Func("a", tint::Empty, ty.void_(), Vector{Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* func = Sem().Get(fb);
    ASSERT_NE(func, nullptr);

    auto* local = Sem().Get<sem::LocalVariable>(l);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), func);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsGlobalVar) {
    // var<private> a : i32;
    //
    // fn F() {
    //   let a = a;
    // }

    auto* g = GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);
    auto* l = Let("a", Expr("a"));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(l);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);

    auto* user =
        Sem().GetVal(local->Declaration()->initializer)->UnwrapLoad()->As<sem::VariableUser>();
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), global);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsGlobalConst) {
    // const a : i32 = 1i;
    //
    // fn F() {
    //   let a = a;
    // }

    auto* g = GlobalConst("a", ty.i32(), Expr(1_i));
    auto* l = Let("a", Expr("a"));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(l);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);

    auto* user = Sem().Get<sem::VariableUser>(local->Declaration()->initializer);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), global);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsLocalVar) {
    // fn F() {
    //   var a : i32 = 1i;
    //   {
    //     let a = a;
    //   }
    // }

    auto* v = Var("a", ty.i32(), Expr(1_i));
    auto* l = Let("a", Expr("a"));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(v), Block(Decl(l))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_l = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_l, nullptr);
    EXPECT_EQ(local_l->Shadows(), local_v);

    auto* user =
        Sem().GetVal(local_l->Declaration()->initializer)->UnwrapLoad()->As<sem::VariableUser>();
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), local_v);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsLocalConst) {
    // fn X() {
    //   const a : i32 = 1i; // x
    //   {
    //     let a = a; // y
    //   }
    // }

    auto* x = Const("a", ty.i32(), Expr(1_i));
    auto* y = Let("a", Expr("a"));
    Func("X", tint::Empty, ty.void_(), Vector{Decl(x), Block(Decl(y))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_x = Sem().Get<sem::LocalVariable>(x);
    auto* local_y = Sem().Get<sem::LocalVariable>(y);

    ASSERT_NE(local_x, nullptr);
    ASSERT_NE(local_y, nullptr);
    EXPECT_EQ(local_y->Shadows(), local_x);

    auto* user = Sem().Get<sem::VariableUser>(local_y->Declaration()->initializer);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), local_x);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsLocalLet) {
    // fn X() {
    //   let a : i32 = 1i; // x
    //   {
    //     let a = a; // y
    //   }
    // }

    auto* x = Let("a", ty.i32(), Expr(1_i));
    auto* y = Let("a", Expr("a"));
    Func("X", tint::Empty, ty.void_(), Vector{Decl(x), Block(Decl(y))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_x = Sem().Get<sem::LocalVariable>(x);
    auto* local_y = Sem().Get<sem::LocalVariable>(y);

    ASSERT_NE(local_x, nullptr);
    ASSERT_NE(local_y, nullptr);
    EXPECT_EQ(local_y->Shadows(), local_x);

    auto* user = Sem().Get<sem::VariableUser>(local_y->Declaration()->initializer);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), local_x);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsParam) {
    // fn F(a : i32) {
    //   {
    //     let a = a;
    //   }
    // }

    auto* p = Param("a", ty.i32());
    auto* l = Let("a", Expr("a"));
    Func("X", Vector{p}, ty.void_(), Vector{Block(Decl(l))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* param = Sem().Get(p);
    auto* local = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(param, nullptr);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), param);

    auto* user = Sem().Get<sem::VariableUser>(local->Declaration()->initializer);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), param);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function-scope const
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, LocalConst_ShadowsAlias) {
    // type a = i32;
    //
    // fn F() {
    //   const a = true;
    // }

    auto* t = Alias("a", ty.i32());
    auto* c = Const("a", Expr(false));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(c)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(c);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsStruct) {
    // struct a {
    //   m : i32;
    // };
    //
    // fn F() {
    //   const a = false;
    // }

    auto* t = Structure("a", Vector{Member("m", ty.i32())});
    auto* c = Const("a", Expr(false));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(c)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(c);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsFunction) {
    // fn a() {
    //   const a = false;
    // }

    auto* c = Const("a", Expr(false));
    auto* fb = Func("a", tint::Empty, ty.void_(), Vector{Decl(c)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* func = Sem().Get(fb);
    ASSERT_NE(func, nullptr);

    auto* local = Sem().Get<sem::LocalVariable>(c);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), func);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsGlobalVar) {
    // var<private> a : i32;
    //
    // fn F() {
    //   const a = 1i;
    // }

    auto* g = GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);
    auto* c = Const("a", Expr(1_i));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(c)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(c);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsGlobalConst) {
    // const a : i32 = 1i;
    //
    // fn F() {
    //   const a = a;
    // }

    auto* g = GlobalConst("a", ty.i32(), Expr(1_i));
    auto* c = Const("a", Expr("a"));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(c)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(c);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);

    auto* user = Sem().Get<sem::VariableUser>(local->Declaration()->initializer);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), global);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsLocalVar) {
    // fn F() {
    //   var a = 1i;
    //   {
    //     const a = 1i;
    //   }
    // }

    auto* v = Var("a", ty.i32(), Expr(1_i));
    auto* c = Const("a", Expr(1_i));
    Func("F", tint::Empty, ty.void_(), Vector{Decl(v), Block(Decl(c))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_c = Sem().Get<sem::LocalVariable>(c);

    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_c, nullptr);
    EXPECT_EQ(local_c->Shadows(), local_v);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsLocalConst) {
    // fn X() {
    //   const a = 1i; // x
    //   {
    //     const a = a; // y
    //   }
    // }

    auto* x = Const("a", ty.i32(), Expr(1_i));
    auto* y = Const("a", Expr("a"));
    Func("X", tint::Empty, ty.void_(), Vector{Decl(x), Block(Decl(y))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_x = Sem().Get<sem::LocalVariable>(x);
    auto* local_y = Sem().Get<sem::LocalVariable>(y);

    ASSERT_NE(local_x, nullptr);
    ASSERT_NE(local_y, nullptr);
    EXPECT_EQ(local_y->Shadows(), local_x);

    auto* user = Sem().Get<sem::VariableUser>(local_y->Declaration()->initializer);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), local_x);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsLocalLet) {
    // fn X() {
    //   let a = 1i; // x
    //   {
    //     const a = 1i; // y
    //   }
    // }

    auto* l = Let("a", ty.i32(), Expr(1_i));
    auto* c = Const("a", Expr(1_i));
    Func("X", tint::Empty, ty.void_(), Vector{Decl(l), Block(Decl(c))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_l = Sem().Get<sem::LocalVariable>(l);
    auto* local_c = Sem().Get<sem::LocalVariable>(c);

    ASSERT_NE(local_l, nullptr);
    ASSERT_NE(local_c, nullptr);
    EXPECT_EQ(local_c->Shadows(), local_l);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsParam) {
    // fn F(a : i32) {
    //   {
    //     const a = 1i;
    //   }
    // }

    auto* p = Param("a", ty.i32());
    auto* c = Const("a", Expr(1_i));
    Func("X", Vector{p}, ty.void_(), Vector{Block(Decl(c))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* param = Sem().Get(p);
    auto* local = Sem().Get<sem::LocalVariable>(c);

    ASSERT_NE(param, nullptr);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), param);
}

TEST_F(ResolverVariableTest, LocalConst_ExplicitType_Decls) {
    Structure("S", Vector{Member("m", ty.u32())});

    auto* c_i32 = Const("a", ty.i32(), Expr(0_i));
    auto* c_u32 = Const("b", ty.u32(), Expr(0_u));
    auto* c_f32 = Const("c", ty.f32(), Expr(0_f));
    auto* c_vi32 = Const("d", ty.vec3<i32>(), Call<vec3<i32>>());
    auto* c_vu32 = Const("e", ty.vec3<u32>(), Call<vec3<u32>>());
    auto* c_vf32 = Const("f", ty.vec3<f32>(), Call<vec3<f32>>());
    auto* c_mf32 = Const("g", ty.mat3x3<f32>(), Call<mat3x3<f32>>());
    auto* c_s = Const("h", ty("S"), Call("S"));

    WrapInFunction(c_i32, c_u32, c_f32, c_vi32, c_vu32, c_vf32, c_mf32, c_s);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(c_i32)->Declaration(), c_i32);
    EXPECT_EQ(Sem().Get(c_u32)->Declaration(), c_u32);
    EXPECT_EQ(Sem().Get(c_f32)->Declaration(), c_f32);
    EXPECT_EQ(Sem().Get(c_vi32)->Declaration(), c_vi32);
    EXPECT_EQ(Sem().Get(c_vu32)->Declaration(), c_vu32);
    EXPECT_EQ(Sem().Get(c_vf32)->Declaration(), c_vf32);
    EXPECT_EQ(Sem().Get(c_mf32)->Declaration(), c_mf32);
    EXPECT_EQ(Sem().Get(c_s)->Declaration(), c_s);

    ASSERT_TRUE(TypeOf(c_i32)->Is<core::type::I32>());
    ASSERT_TRUE(TypeOf(c_u32)->Is<core::type::U32>());
    ASSERT_TRUE(TypeOf(c_f32)->Is<core::type::F32>());
    ASSERT_TRUE(TypeOf(c_vi32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vu32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vf32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_mf32)->Is<core::type::Matrix>());
    ASSERT_TRUE(TypeOf(c_s)->Is<core::type::Struct>());

    EXPECT_TRUE(Sem().Get(c_i32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_u32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_f32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vi32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vu32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_mf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_s)->ConstantValue()->AllZero());
}

TEST_F(ResolverVariableTest, LocalConst_ImplicitType_Decls) {
    Structure("S", Vector{Member("m", ty.u32())});

    auto* c_i32 = Const("a", Expr(0_i));
    auto* c_u32 = Const("b", Expr(0_u));
    auto* c_f32 = Const("c", Expr(0_f));
    auto* c_ai = Const("d", Expr(0_a));
    auto* c_af = Const("e", Expr(0._a));
    auto* c_vi32 = Const("f", Call<vec3<i32>>());
    auto* c_vu32 = Const("g", Call<vec3<u32>>());
    auto* c_vf32 = Const("h", Call<vec3<f32>>());
    auto* c_vai = Const("i", Call<vec3<Infer>>(0_a));
    auto* c_vaf = Const("j", Call<vec3<Infer>>(0._a));
    auto* c_mf32 = Const("k", Call<mat3x3<f32>>());
    auto* c_maf32 = Const("l", Call<mat3x3<Infer>>(          //
                                   Call<vec3<Infer>>(0._a),  //
                                   Call<vec3<Infer>>(0._a),  //
                                   Call<vec3<Infer>>(0._a)));
    auto* c_s = Const("m", Call("S"));

    WrapInFunction(c_i32, c_u32, c_f32, c_ai, c_af, c_vi32, c_vu32, c_vf32, c_vai, c_vaf, c_mf32,
                   c_maf32, c_s);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(c_i32)->Declaration(), c_i32);
    EXPECT_EQ(Sem().Get(c_u32)->Declaration(), c_u32);
    EXPECT_EQ(Sem().Get(c_f32)->Declaration(), c_f32);
    EXPECT_EQ(Sem().Get(c_ai)->Declaration(), c_ai);
    EXPECT_EQ(Sem().Get(c_af)->Declaration(), c_af);
    EXPECT_EQ(Sem().Get(c_vi32)->Declaration(), c_vi32);
    EXPECT_EQ(Sem().Get(c_vu32)->Declaration(), c_vu32);
    EXPECT_EQ(Sem().Get(c_vf32)->Declaration(), c_vf32);
    EXPECT_EQ(Sem().Get(c_vai)->Declaration(), c_vai);
    EXPECT_EQ(Sem().Get(c_vaf)->Declaration(), c_vaf);
    EXPECT_EQ(Sem().Get(c_mf32)->Declaration(), c_mf32);
    EXPECT_EQ(Sem().Get(c_maf32)->Declaration(), c_maf32);
    EXPECT_EQ(Sem().Get(c_s)->Declaration(), c_s);

    ASSERT_TRUE(TypeOf(c_i32)->Is<core::type::I32>());
    ASSERT_TRUE(TypeOf(c_u32)->Is<core::type::U32>());
    ASSERT_TRUE(TypeOf(c_f32)->Is<core::type::F32>());
    ASSERT_TRUE(TypeOf(c_ai)->Is<core::type::AbstractInt>());
    ASSERT_TRUE(TypeOf(c_af)->Is<core::type::AbstractFloat>());
    ASSERT_TRUE(TypeOf(c_vi32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vu32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vf32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vai)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vaf)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_mf32)->Is<core::type::Matrix>());
    ASSERT_TRUE(TypeOf(c_maf32)->Is<core::type::Matrix>());
    ASSERT_TRUE(TypeOf(c_s)->Is<core::type::Struct>());

    EXPECT_TRUE(Sem().Get(c_i32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_u32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_f32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_ai)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_af)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vi32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vu32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vai)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vaf)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_mf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_maf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_s)->ConstantValue()->AllZero());
}

TEST_F(ResolverVariableTest, LocalConst_PropagateConstValue) {
    auto* a = Const("a", Expr(42_i));
    auto* b = Const("b", Expr("a"));
    auto* c = Const("c", Expr("b"));

    WrapInFunction(a, b, c);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(c)->Is<core::type::I32>());

    EXPECT_EQ(Sem().Get(c)->ConstantValue()->ValueAs<i32>(), 42_i);
}

TEST_F(ResolverVariableTest, LocalConst_ConstEval) {
    auto* c = Const("c", Div(Mul(Add(1_i, 2_i), 3_i), 3_i));

    WrapInFunction(c);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(c)->Is<core::type::I32>());

    EXPECT_EQ(Sem().Get(c)->ConstantValue()->ValueAs<i32>(), 3_i);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Module-scope 'var'
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, GlobalVar_AddressSpace) {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

    Enable(wgsl::Extension::kChromiumExperimentalImmediate);

    auto* buf = Structure("S", Vector{Member("m", ty.i32())});
    auto* private_ = GlobalVar("p", ty.i32(), core::AddressSpace::kPrivate);
    auto* workgroup = GlobalVar("w", ty.i32(), core::AddressSpace::kWorkgroup);
    auto* immediate = GlobalVar("pc", ty.i32(), core::AddressSpace::kImmediate);
    auto* uniform =
        GlobalVar("ub", ty.Of(buf), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));
    auto* storage =
        GlobalVar("sb", ty.Of(buf), core::AddressSpace::kStorage, Binding(1_a), Group(0_a));
    auto* handle = GlobalVar("h", ty.depth_texture(core::type::TextureDimension::k2d), Binding(2_a),
                             Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(private_)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(workgroup)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(immediate)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(uniform)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(storage)->Is<core::type::Reference>());
    ASSERT_TRUE(TypeOf(handle)->Is<core::type::Reference>());

    EXPECT_EQ(TypeOf(private_)->As<core::type::Reference>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(workgroup)->As<core::type::Reference>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(immediate)->As<core::type::Reference>()->Access(), core::Access::kRead);
    EXPECT_EQ(TypeOf(uniform)->As<core::type::Reference>()->Access(), core::Access::kRead);
    EXPECT_EQ(TypeOf(storage)->As<core::type::Reference>()->Access(), core::Access::kRead);
    EXPECT_EQ(TypeOf(handle)->As<core::type::Reference>()->Access(), core::Access::kRead);
}

TEST_F(ResolverVariableTest, GlobalVar_ExplicitAddressSpace) {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

    auto* buf = Structure("S", Vector{Member("m", ty.i32())});
    auto* storage = GlobalVar("sb", ty.Of(buf), core::AddressSpace::kStorage,
                              core::Access::kReadWrite, Binding(1_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(storage)->Is<core::type::Reference>());

    EXPECT_EQ(TypeOf(storage)->As<core::type::Reference>()->Access(), core::Access::kReadWrite);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Module-scope const
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, GlobalConst_ExplicitType_Decls) {
    auto* c_i32 = GlobalConst("a", ty.i32(), Expr(0_i));
    auto* c_u32 = GlobalConst("b", ty.u32(), Expr(0_u));
    auto* c_f32 = GlobalConst("c", ty.f32(), Expr(0_f));
    auto* c_vi32 = GlobalConst("d", ty.vec3<i32>(), Call<vec3<i32>>());
    auto* c_vu32 = GlobalConst("e", ty.vec3<u32>(), Call<vec3<u32>>());
    auto* c_vf32 = GlobalConst("f", ty.vec3<f32>(), Call<vec3<f32>>());
    auto* c_mf32 = GlobalConst("g", ty.mat3x3<f32>(), Call<mat3x3<f32>>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(c_i32)->Declaration(), c_i32);
    EXPECT_EQ(Sem().Get(c_u32)->Declaration(), c_u32);
    EXPECT_EQ(Sem().Get(c_f32)->Declaration(), c_f32);
    EXPECT_EQ(Sem().Get(c_vi32)->Declaration(), c_vi32);
    EXPECT_EQ(Sem().Get(c_vu32)->Declaration(), c_vu32);
    EXPECT_EQ(Sem().Get(c_vf32)->Declaration(), c_vf32);
    EXPECT_EQ(Sem().Get(c_mf32)->Declaration(), c_mf32);

    ASSERT_TRUE(TypeOf(c_i32)->Is<core::type::I32>());
    ASSERT_TRUE(TypeOf(c_u32)->Is<core::type::U32>());
    ASSERT_TRUE(TypeOf(c_f32)->Is<core::type::F32>());
    ASSERT_TRUE(TypeOf(c_vi32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vu32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vf32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_mf32)->Is<core::type::Matrix>());

    EXPECT_TRUE(Sem().Get(c_i32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_u32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_f32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vi32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vu32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_mf32)->ConstantValue()->AllZero());
}

TEST_F(ResolverVariableTest, GlobalConst_ImplicitType_Decls) {
    auto* c_i32 = GlobalConst("a", Expr(0_i));
    auto* c_u32 = GlobalConst("b", Expr(0_u));
    auto* c_f32 = GlobalConst("c", Expr(0_f));
    auto* c_ai = GlobalConst("d", Expr(0_a));
    auto* c_af = GlobalConst("e", Expr(0._a));
    auto* c_vi32 = GlobalConst("f", Call<vec3<i32>>());
    auto* c_vu32 = GlobalConst("g", Call<vec3<u32>>());
    auto* c_vf32 = GlobalConst("h", Call<vec3<f32>>());
    auto* c_vai = GlobalConst("i", Call<vec3<Infer>>(0_a));
    auto* c_vaf = GlobalConst("j", Call<vec3<Infer>>(0._a));
    auto* c_mf32 = GlobalConst("k", Call<mat3x3<f32>>());
    auto* c_maf32 = GlobalConst("l", Call<mat3x3<Infer>>(          //
                                         Call<vec3<Infer>>(0._a),  //
                                         Call<vec3<Infer>>(0._a),  //
                                         Call<vec3<Infer>>(0._a)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(c_i32)->Declaration(), c_i32);
    EXPECT_EQ(Sem().Get(c_u32)->Declaration(), c_u32);
    EXPECT_EQ(Sem().Get(c_f32)->Declaration(), c_f32);
    EXPECT_EQ(Sem().Get(c_ai)->Declaration(), c_ai);
    EXPECT_EQ(Sem().Get(c_af)->Declaration(), c_af);
    EXPECT_EQ(Sem().Get(c_vi32)->Declaration(), c_vi32);
    EXPECT_EQ(Sem().Get(c_vu32)->Declaration(), c_vu32);
    EXPECT_EQ(Sem().Get(c_vf32)->Declaration(), c_vf32);
    EXPECT_EQ(Sem().Get(c_vai)->Declaration(), c_vai);
    EXPECT_EQ(Sem().Get(c_vaf)->Declaration(), c_vaf);
    EXPECT_EQ(Sem().Get(c_mf32)->Declaration(), c_mf32);
    EXPECT_EQ(Sem().Get(c_maf32)->Declaration(), c_maf32);

    ASSERT_TRUE(TypeOf(c_i32)->Is<core::type::I32>());
    ASSERT_TRUE(TypeOf(c_u32)->Is<core::type::U32>());
    ASSERT_TRUE(TypeOf(c_f32)->Is<core::type::F32>());
    ASSERT_TRUE(TypeOf(c_ai)->Is<core::type::AbstractInt>());
    ASSERT_TRUE(TypeOf(c_af)->Is<core::type::AbstractFloat>());
    ASSERT_TRUE(TypeOf(c_vi32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vu32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vf32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vai)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_vaf)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(c_mf32)->Is<core::type::Matrix>());
    ASSERT_TRUE(TypeOf(c_maf32)->Is<core::type::Matrix>());

    EXPECT_TRUE(Sem().Get(c_i32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_u32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_f32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_ai)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_af)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vi32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vu32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vai)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vaf)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_mf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_maf32)->ConstantValue()->AllZero());
}

TEST_F(ResolverVariableTest, GlobalConst_PropagateConstValue) {
    GlobalConst("b", Expr("a"));
    auto* c = GlobalConst("c", Expr("b"));
    GlobalConst("a", Expr(42_i));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(c)->Is<core::type::I32>());

    EXPECT_EQ(Sem().Get(c)->ConstantValue()->ValueAs<i32>(), 42_i);
}

TEST_F(ResolverVariableTest, GlobalConst_ConstEval) {
    auto* c = GlobalConst("c", Div(Mul(Add(1_i, 2_i), 3_i), 3_i));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(c)->Is<core::type::I32>());

    EXPECT_EQ(Sem().Get(c)->ConstantValue()->ValueAs<i32>(), 3_i);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function parameter
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, Param_ShadowsFunction) {
    // fn a(a : bool) {
    // }

    auto* p = Param("a", ty.bool_());
    auto* f = Func("a", Vector{p}, ty.void_(), tint::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* func = Sem().Get(f);
    auto* param = Sem().Get(p);

    ASSERT_NE(func, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), func);
}

TEST_F(ResolverVariableTest, Param_ShadowsGlobalVar) {
    // var<private> a : i32;
    //
    // fn F(a : bool) {
    // }

    auto* g = GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);
    auto* p = Param("a", ty.bool_());
    Func("F", Vector{p}, ty.void_(), tint::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* param = Sem().Get(p);

    ASSERT_NE(global, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), global);
}

TEST_F(ResolverVariableTest, Param_ShadowsGlobalConst) {
    // const a : i32 = 1i;
    //
    // fn F(a : bool) {
    // }

    auto* g = GlobalConst("a", ty.i32(), Expr(1_i));
    auto* p = Param("a", ty.bool_());
    Func("F", Vector{p}, ty.void_(), tint::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* param = Sem().Get(p);

    ASSERT_NE(global, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), global);
}

TEST_F(ResolverVariableTest, Param_ShadowsAlias) {
    // type a = i32;
    //
    // fn F(a : a) {
    // }

    auto* a = Alias("a", ty.i32());
    auto* p = Param("a", ty("a"));
    Func("F", Vector{p}, ty.void_(), tint::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* alias = Sem().Get(a);
    auto* param = Sem().Get(p);

    ASSERT_NE(alias, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), alias);
    EXPECT_EQ(param->Type(), alias);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Templated identifier uses
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, GlobalVar_UseTemplatedIdent) {
    // var<private> a : i32;
    //
    // fn F() {
    //   let l = a<i32>;
    // }

    GlobalVar(Source{{56, 78}}, "a", ty.i32(), core::AddressSpace::kPrivate);
    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("l", Expr(Ident(Source{{12, 34}}, "a", "i32")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: variable 'a' does not take template arguments
56:78 note: 'var a' declared here)");
}

TEST_F(ResolverVariableTest, GlobalConst_UseTemplatedIdent) {
    // const a = 1i;
    //
    // fn F() {
    //   let l = a<i32>;
    // }

    GlobalConst(Source{{56, 78}}, "a", Expr(1_i));
    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("l", Expr(Ident(Source{{12, 34}}, "a", "i32")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: variable 'a' does not take template arguments
56:78 note: 'const a' declared here)");
}

TEST_F(ResolverVariableTest, GlobalOverride_UseTemplatedIdent) {
    // override a = 1i;
    //
    // fn F() {
    //   let l = a<i32>;
    // }

    Override(Source{{56, 78}}, "a", Expr(1_i));
    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("l", Expr(Ident(Source{{12, 34}}, "a", "i32")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: variable 'a' does not take template arguments
56:78 note: 'override a' declared here)");
}

TEST_F(ResolverVariableTest, Param_UseTemplatedIdent) {
    // fn F(a : i32) {
    //   let l = a<i32>;
    // }

    Func("F", Vector{Param(Source{{56, 78}}, "a", ty.i32())}, ty.void_(),
         Vector{
             Decl(Let("l", Expr(Ident(Source{{12, 34}}, "a", "i32")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: variable 'a' does not take template arguments
56:78 note: parameter 'a' declared here)");
}

TEST_F(ResolverVariableTest, LocalVar_UseTemplatedIdent) {
    // fn F() {
    //   var a : i32;
    //   let l = a<i32>;
    // }

    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(Var(Source{{56, 78}}, "a", ty.i32())),
             Decl(Let("l", Expr(Ident(Source{{12, 34}}, "a", "i32")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: variable 'a' does not take template arguments
56:78 note: 'var a' declared here)");
}

TEST_F(ResolverVariableTest, Let_UseTemplatedIdent) {
    // fn F() {
    //   let a = 1i;
    //   let l = a<i32>;
    // }

    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(Let(Source{{56, 78}}, "a", Expr(1_i))),
             Decl(Let("l", Expr(Ident(Source{{12, 34}}, "a", "i32")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: variable 'a' does not take template arguments
56:78 note: 'let a' declared here)");
}

TEST_F(ResolverVariableTest, ScalarI8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.i8(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(error: i8 cannot be used as the type of a var)");
}

TEST_F(ResolverVariableTest, ScalarU8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.u8(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(error: u8 cannot be used as the type of a var)");
}

TEST_F(ResolverVariableTest, Vec2I8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.vec2<i8>(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(error: vector element type must be 'bool', 'f32', 'f16', 'i32' or 'u32')");
}

TEST_F(ResolverVariableTest, Vec2U8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.vec2<u8>(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(error: vector element type must be 'bool', 'f32', 'f16', 'i32' or 'u32')");
}

TEST_F(ResolverVariableTest, Vec3I8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.vec2<i8>(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(error: vector element type must be 'bool', 'f32', 'f16', 'i32' or 'u32')");
}

TEST_F(ResolverVariableTest, Vec3U8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.vec2<u8>(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(error: vector element type must be 'bool', 'f32', 'f16', 'i32' or 'u32')");
}

TEST_F(ResolverVariableTest, Vec4I8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.vec2<i8>(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(error: vector element type must be 'bool', 'f32', 'f16', 'i32' or 'u32')");
}

TEST_F(ResolverVariableTest, Vec4U8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.vec2<u8>(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(error: vector element type must be 'bool', 'f32', 'f16', 'i32' or 'u32')");
}

TEST_F(ResolverVariableTest, ArrayI8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.array<i8, 4>(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(error: i8 cannot be used as an element type of an array)");
}

TEST_F(ResolverVariableTest, ArrayU8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.array<u8, 4>(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(error: u8 cannot be used as an element type of an array)");
}

TEST_F(ResolverVariableTest, MatrixI8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.mat4x4<i8>(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(error: matrix element type must be 'f32' or 'f16')");
}

TEST_F(ResolverVariableTest, MatrixU8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    GlobalVar("v", ty.mat2x2<u8>(), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(error: matrix element type must be 'f32' or 'f16')");
}

}  // namespace
}  // namespace tint::resolver
