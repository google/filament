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

#include "src/tint/lang/wgsl/sem/load.h"
#include "gmock/gmock.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverLoadTest = ResolverTest;

TEST_F(ResolverLoadTest, VarInitializer) {
    // var ref = 1i;
    // var v = ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Var("v", ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::I32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::I32>());
}

TEST_F(ResolverLoadTest, LetInitializer) {
    // var ref = 1i;
    // let l = ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Let("l", ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::I32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::I32>());
}

TEST_F(ResolverLoadTest, Assignment) {
    // var ref = 1i;
    // var v : i32;
    // v = ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Var("v", ty.i32()),     //
                   Assign("v", ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::I32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::I32>());
}

TEST_F(ResolverLoadTest, CompoundAssignment) {
    // var ref = 1i;
    // var v : i32;
    // v += ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Var("v", ty.i32()),     //
                   CompoundAssign("v", ident, core::BinaryOp::kAdd));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::I32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::I32>());
}

TEST_F(ResolverLoadTest, UnaryOp) {
    // var ref = 1i;
    // var v = -ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Var("v", Negation(ident)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::I32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::I32>());
}

TEST_F(ResolverLoadTest, UnaryOp_NoLoad) {
    // var ref = 1i;
    // let v = &ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Let("v", AddressOf(ident)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* var_user = Sem().Get<sem::VariableUser>(ident);
    ASSERT_NE(var_user, nullptr);
    EXPECT_TRUE(var_user->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(var_user->Type()->UnwrapRef()->Is<core::type::I32>());
}

TEST_F(ResolverLoadTest, BinaryOp) {
    // var ref = 1i;
    // var v = ref * 1i;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Var("v", Mul(ident, 1_i)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::I32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::I32>());
}

TEST_F(ResolverLoadTest, Index) {
    // var ref = 1i;
    // var v = array<i32, 3>(1i, 2i, 3i)[ref];
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   IndexAccessor(Call<array<i32, 3>>(1_i, 2_i, 3_i), ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::I32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::I32>());
}

TEST_F(ResolverLoadTest, MultiComponentSwizzle) {
    // var ref = vec4(1);
    // var v = ref.xyz;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Call<vec4<i32>>(1_i)),  //
                   Var("v", MemberAccessor(ident, "xyz")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::Vector>());
}

TEST_F(ResolverLoadTest, MultiComponentSwizzle_FromPointer) {
    // var ref = vec4(1);
    // let ptr = &ref;
    // var v = ptr.xyz;
    auto* ident = Expr("ptr");
    WrapInFunction(Var("ref", Call<vec4<i32>>(1_i)),  //
                   Let("ptr", AddressOf("ref")),      //
                   Var("v", MemberAccessor(ident, "xyz")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Pointer>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapPtr()->Is<core::type::Vector>());
}

TEST_F(ResolverLoadTest, Bitcast) {
    // var ref = 1f;
    // var v = bitcast<i32>(ref);
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_f)),  //
                   Bitcast<i32>(ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::F32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::F32>());
}

TEST_F(ResolverLoadTest, BuiltinArg) {
    // var ref = 1f;
    // var v = abs(ref);
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_f)),  //
                   Call("abs", ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::F32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::F32>());
}

TEST_F(ResolverLoadTest, FunctionArg) {
    // fn f(x : f32) {}
    // var ref = 1f;
    // f(ref);
    Func("f", Vector{Param("x", ty.f32())}, ty.void_(), tint::Empty);
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_f)),  //
                   CallStmt(Call("f", ident)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::F32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::F32>());
}

TEST_F(ResolverLoadTest, FunctionArg_Handles) {
    // @group(0) @binding(0) var t : texture_2d<f32>;
    // @group(0) @binding(1) var s : sampler;
    // fn f(tp : texture_2d<f32>, sp : sampler) -> vec4<f32> {
    //   return textureSampleLevel(tp, sp, vec2(), 0);
    // }
    // f(t, s);
    GlobalVar("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              Vector{Group(0_a), Binding(0_a)});
    GlobalVar("s", ty.sampler(core::type::SamplerKind::kSampler), Vector{Group(0_a), Binding(1_a)});
    Func("f",
         Vector{
             Param("tp", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())),
             Param("sp", ty.sampler(core::type::SamplerKind::kSampler)),
         },
         ty.vec4<f32>(),
         Vector{
             Return(Call("textureSampleLevel", "tp", "sp", Call<vec2<f32>>(), 0_a)),
         });
    auto* t_ident = Expr("t");
    auto* s_ident = Expr("s");
    WrapInFunction(CallStmt(Call("f", t_ident, s_ident)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* load = Sem().Get<sem::Load>(t_ident);
        ASSERT_NE(load, nullptr);
        EXPECT_TRUE(load->Type()->Is<core::type::SampledTexture>());
        EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
        EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::SampledTexture>());
    }
    {
        auto* load = Sem().Get<sem::Load>(s_ident);
        ASSERT_NE(load, nullptr);
        EXPECT_TRUE(load->Type()->Is<core::type::Sampler>());
        EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
        EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::Sampler>());
    }
}

TEST_F(ResolverLoadTest, FunctionReturn) {
    // var ref = 1f;
    // return ref;
    auto* ident = Expr("ref");
    Func("f", tint::Empty, ty.f32(),
         Vector{
             Decl(Var("ref", Expr(1_f))),
             Return(ident),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::F32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::F32>());
}

TEST_F(ResolverLoadTest, IfCond) {
    // var ref = false;
    // if (ref) {}
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(false)),  //
                   If(ident, Block()));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::Bool>());
}

TEST_F(ResolverLoadTest, Switch) {
    // var ref = 1i;
    // switch (ref) {
    //   default:
    // }
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Switch(ident, DefaultCase()));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::I32>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::I32>());
}

TEST_F(ResolverLoadTest, BreakIfCond) {
    // var ref = false;
    // loop {
    //   continuing {
    //     break if (ref);
    //   }
    // }
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(false)),  //
                   Loop(Block(), Block(BreakIf(ident))));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::Bool>());
}

TEST_F(ResolverLoadTest, ForCond) {
    // var ref = false;
    // for (; ref; ) {}
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(false)),  //
                   For(nullptr, ident, nullptr, Block()));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::Bool>());
}

TEST_F(ResolverLoadTest, WhileCond) {
    // var ref = false;
    // while (ref) {}
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(false)),  //
                   While(ident, Block()));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(load->Source()->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(load->Source()->Type()->UnwrapRef()->Is<core::type::Bool>());
}

TEST_F(ResolverLoadTest, AddressOf) {
    // var ref = 1i;
    // let l = &ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Let("l", AddressOf(ident)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* no_load = Sem().GetVal(ident);
    ASSERT_NE(no_load, nullptr);
    EXPECT_TRUE(no_load->Type()->Is<core::type::Reference>());  // No load
}

}  // namespace
}  // namespace tint::resolver
