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

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/utils/containers/slice.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverEvaluationStageTest = ResolverTest;

TEST_F(ResolverEvaluationStageTest, Literal_i32) {
    auto* expr = Expr(123_i);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Literal_f32) {
    auto* expr = Expr(123_f);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Vector_Init) {
    auto* expr = Call<vec3<f32>>();
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Vector_Init_Const_Const) {
    // const f = 1.f;
    // vec2<f32>(f, f);
    auto* f = Const("f", Expr(1_f));
    auto* expr = Call<vec2<f32>>(f, f);
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Vector_Init_Runtime_Runtime) {
    // var f = 1.f;
    // vec2<f32>(f, f);
    auto* f = Var("f", Expr(1_f));
    auto* expr = Call<vec2<f32>>(f, f);
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Vector_Conv_Const) {
    // const f = 1.f;
    // vec2<u32>(vec2<f32>(f));
    auto* f = Const("f", Expr(1_f));
    auto* expr = Call<vec2<u32>>(Call<vec2<f32>>(f));
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Vector_Conv_Runtime) {
    // var f = 1.f;
    // vec2<u32>(vec2<f32>(f));
    auto* f = Var("f", Expr(1_f));
    auto* expr = Call<vec2<u32>>(Call<vec2<f32>>(f));
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Matrix_Init) {
    auto* expr = Call<mat2x2<f32>>();
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Array_Init) {
    auto* expr = Call<array<f32, 3>>();
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Array_Init_Const_Const) {
    // const f = 1.f;
    // array<f32, 2>(f, f);
    auto* f = Const("f", Expr(1_f));
    auto* expr = Call<array<f32, 2>>(f, f);
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Array_Init_Const_Override) {
    // const f1 = 1.f;
    // override f2 = 2.f;
    // array<f32, 2>(f1, f2);
    auto* f1 = Const("f1", Expr(1_f));
    auto* f2 = Override("f2", Expr(2_f));
    auto* expr = Call<array<f32, 2>>(f1, f2);
    WrapInFunction(f1, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f1)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(f2)->Stage(), core::EvaluationStage::kOverride);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kOverride);
}

TEST_F(ResolverEvaluationStageTest, Array_Init_Override_Runtime) {
    // override f1 = 1.f;
    // var f2 = 2.f;
    // array<f32, 2>(f1, f2);
    auto* f1 = Override("f1", Expr(1_f));
    auto* f2 = Var("f2", Expr(2_f));
    auto* expr = Call<array<f32, 2>>(f1, f2);
    WrapInFunction(f2, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f1)->Stage(), core::EvaluationStage::kOverride);
    EXPECT_EQ(Sem().Get(f2)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Array_Init_Const_Runtime) {
    // const f1 = 1.f;
    // var f2 = 2.f;
    // array<f32, 2>(f1, f2);
    auto* f1 = Const("f1", Expr(1_f));
    auto* f2 = Var("f2", Expr(2_f));
    auto* expr = Call<array<f32, 2>>(f1, f2);
    WrapInFunction(f1, f2, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f1)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(f2)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Array_Init_Runtime_Runtime) {
    // var f = 1.f;
    // array<f32, 2>(f, f);
    auto* f = Var("f", Expr(1_f));
    auto* expr = Call<array<f32, 2>>(f, f);
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, IndexAccessor_Const_Const) {
    // const vec = vec4<f32>();
    // const idx = 1_i;
    // vec[idx]
    auto* vec = Const("vec", Call<vec4<f32>>());
    auto* idx = Const("idx", Expr(1_i));
    auto* expr = IndexAccessor(vec, idx);
    WrapInFunction(vec, idx, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(idx)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, IndexAccessor_Runtime_Const) {
    // var vec = vec4<f32>();
    // const idx = 1_i;
    // vec[idx]
    auto* vec = Var("vec", Call<vec4<f32>>());
    auto* idx = Const("idx", Expr(1_i));
    auto* expr = IndexAccessor(vec, idx);
    WrapInFunction(vec, idx, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(idx)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, IndexAccessor_Const_Override) {
    // const vec = vec4<f32>();
    // override idx = 1_i;
    // vec[idx]
    auto* vec = Const("vec", Call<vec4<f32>>());
    auto* idx = Override("idx", Expr(1_i));
    auto* expr = IndexAccessor(vec, idx);
    WrapInFunction(vec, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(idx)->Stage(), core::EvaluationStage::kOverride);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kOverride);
}

TEST_F(ResolverEvaluationStageTest, IndexAccessor_Const_Runtime) {
    // const vec = vec4<f32>();
    // let idx = 1_i;
    // vec[idx]
    auto* vec = Const("vec", Call<vec4<f32>>());
    auto* idx = Let("idx", Expr(1_i));
    auto* expr = IndexAccessor(vec, idx);
    WrapInFunction(vec, idx, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(idx)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Swizzle_Const) {
    // const vec = S();
    // vec.m
    auto* vec = Const("vec", Call<vec4<f32>>());
    auto* expr = MemberAccessor(vec, "xz");
    WrapInFunction(vec, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Swizzle_Runtime) {
    // var vec = S();
    // vec.m
    auto* vec = Var("vec", Call<vec4<f32>>());
    auto* expr = MemberAccessor(vec, "rg");
    WrapInFunction(vec, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, MemberAccessor_Const) {
    // struct S { m : i32 };
    // const str = S();
    // str.m
    Structure("S", Vector{Member("m", ty.i32())});
    auto* str = Const("str", Call("S"));
    auto* expr = MemberAccessor(str, "m");
    WrapInFunction(str, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(str)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, MemberAccessor_Runtime) {
    // struct S { m : i32 };
    // var str = S();
    // str.m
    Structure("S", Vector{Member("m", ty.i32())});
    auto* str = Var("str", Call("S"));
    auto* expr = MemberAccessor(str, "m");
    WrapInFunction(str, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(str)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Binary_Runtime) {
    // let one = 1;
    // let result = (one == 1) && (one == 1);
    auto* one = Let("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal("one", 1_a);
    auto* binary = LogicalAnd(lhs, rhs);
    auto* result = Let("result", binary);
    WrapInFunction(one, result);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(lhs)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(rhs)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(binary)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Binary_Const) {
    // const one = 1;
    // const result = (one == 1) && (one == 1);
    auto* one = Const("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal("one", 1_a);
    auto* binary = LogicalAnd(lhs, rhs);
    auto* result = Const("result", binary);
    WrapInFunction(one, result);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(lhs)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(rhs)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(binary)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Binary_NotEvaluated) {
    // const one = 1;
    // const result = (one == 0) && (one == 1);
    auto* one = Const("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal("one", 1_a);
    auto* binary = LogicalAnd(lhs, rhs);
    auto* result = Const("result", binary);
    WrapInFunction(one, result);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(lhs)->Stage(), core::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(rhs)->Stage(), core::EvaluationStage::kNotEvaluated);
    EXPECT_EQ(Sem().Get(binary)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, FnCall_Runtime) {
    // fn f() -> bool { return true; }
    // let l = false
    // let result = l && f();
    Func("f", Empty, ty.bool_(), Vector{Return(true)});
    auto* let = Let("l", Expr(false));
    auto* lhs = Expr(let);
    auto* rhs = Call("f");
    auto* binary = LogicalAnd(lhs, rhs);
    auto* result = Let("result", binary);
    WrapInFunction(let, result);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(rhs)->Stage(), core::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(binary)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, FnCall_NotEvaluated) {
    // fn f() -> bool { return true; }
    // let result = false && f();
    Func("f", Empty, ty.bool_(), Vector{Return(true)});
    auto* rhs = Call("f");
    auto* lhs = Expr(false);
    auto* binary = LogicalAnd(lhs, rhs);
    auto* result = Let("result", binary);
    WrapInFunction(result);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(rhs)->Stage(), core::EvaluationStage::kNotEvaluated);
    EXPECT_EQ(Sem().Get(binary)->Stage(), core::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, NestedFnCall_NotEvaluated) {
    // fn f(b : bool) -> bool { return b; }
    // let result = false && f(f(f(1 == 0)));
    Func("f", Vector{Param("b", ty.bool_())}, ty.bool_(), Vector{Return("b")});
    auto* cmp = Equal(0_i, 1_i);
    auto* rhs_0 = Call("f", cmp);
    auto* rhs_1 = Call("f", rhs_0);
    auto* rhs_2 = Call("f", rhs_1);
    auto* lhs = Expr(false);
    auto* binary = LogicalAnd(lhs, rhs_2);
    auto* result = Let("result", binary);
    WrapInFunction(result);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(cmp)->Stage(), core::EvaluationStage::kNotEvaluated);
    EXPECT_EQ(Sem().Get(rhs_0)->Stage(), core::EvaluationStage::kNotEvaluated);
    EXPECT_EQ(Sem().Get(rhs_1)->Stage(), core::EvaluationStage::kNotEvaluated);
    EXPECT_EQ(Sem().Get(rhs_2)->Stage(), core::EvaluationStage::kNotEvaluated);
    EXPECT_EQ(Sem().Get(binary)->Stage(), core::EvaluationStage::kConstant);
}

}  // namespace
}  // namespace tint::resolver
