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

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverIncrementDecrementValidationTest = ResolverTest;

TEST_F(ResolverIncrementDecrementValidationTest, Increment_Signed) {
    // var a : i32 = 2;
    // a++;
    auto* var = Var("a", ty.i32(), Expr(2_i));
    WrapInFunction(var, Increment(Source{{12, 34}}, "a"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIncrementDecrementValidationTest, Decrement_Signed) {
    // var a : i32 = 2;
    // a--;
    auto* var = Var("a", ty.i32(), Expr(2_i));
    WrapInFunction(var, Decrement(Source{{12, 34}}, "a"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIncrementDecrementValidationTest, Increment_Unsigned) {
    // var a : u32 = 2u;
    // a++;
    auto* var = Var("a", ty.u32(), Expr(2_u));
    WrapInFunction(var, Increment(Source{{12, 34}}, "a"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIncrementDecrementValidationTest, Decrement_Unsigned) {
    // var a : u32 = 2u;
    // a--;
    auto* var = Var("a", ty.u32(), Expr(2_u));
    WrapInFunction(var, Decrement(Source{{12, 34}}, "a"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIncrementDecrementValidationTest, ThroughPointer) {
    // var a : i32;
    // let b : ptr<function,i32> = &a;
    // *b++;
    auto* var_a = Var("a", ty.i32(), core::AddressSpace::kFunction);
    auto* var_b = Let("b", ty.ptr<function, i32>(), AddressOf(Expr("a")));
    WrapInFunction(var_a, var_b, Increment(Source{{12, 34}}, Deref("b")));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIncrementDecrementValidationTest, ThroughArray) {
    // var a : array<i32, 4u>;
    // a[1i]++;
    auto* var_a = Var("a", ty.array<i32, 4>());
    WrapInFunction(var_a, Increment(Source{{12, 34}}, IndexAccessor("a", 1_i)));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIncrementDecrementValidationTest, ThroughVector_Index) {
    // var a : vec4<i32>;
    // a[1i]++;
    auto* var_a = Var("a", ty.vec4(ty.i32()));
    WrapInFunction(var_a, Increment(Source{{12, 34}}, IndexAccessor("a", 1_i)));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIncrementDecrementValidationTest, ThroughVector_Member) {
    // var a : vec4<i32>;
    // a.y++;
    auto* var_a = Var("a", ty.vec4(ty.i32()));
    WrapInFunction(var_a, Increment(Source{{12, 34}}, MemberAccessor("a", "y")));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIncrementDecrementValidationTest, Float) {
    // var a : f32 = 2.0;
    // a++;
    auto* var = Var("a", ty.f32(), Expr(2_f));
    auto* inc = Increment(Expr(Source{{12, 34}}, "a"));
    WrapInFunction(var, inc);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: increment statement can only be applied to an "
              "integer scalar");
}

TEST_F(ResolverIncrementDecrementValidationTest, Vector) {
    // var a : vec4<f32>;
    // a++;
    auto* var = Var("a", ty.vec4<i32>());
    auto* inc = Increment(Expr(Source{{12, 34}}, "a"));
    WrapInFunction(var, inc);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: increment statement can only be applied to an "
              "integer scalar");
}

TEST_F(ResolverIncrementDecrementValidationTest, Atomic) {
    // var<workgroup> a : atomic<i32>;
    // a++;
    GlobalVar(Source{{12, 34}}, "a", ty.atomic(ty.i32()), core::AddressSpace::kWorkgroup);
    WrapInFunction(Increment(Expr(Source{{56, 78}}, "a")));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: increment statement can only be applied to an "
              "integer scalar");
}

TEST_F(ResolverIncrementDecrementValidationTest, Literal) {
    // 1++;
    WrapInFunction(Increment(Expr(Source{{56, 78}}, 1_i)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: cannot modify value of type 'i32'");
}

TEST_F(ResolverIncrementDecrementValidationTest, Constant) {
    // let a = 1;
    // a++;
    auto* a = Let(Source{{12, 34}}, "a", Expr(1_i));
    WrapInFunction(a, Increment(Expr(Source{{56, 78}}, "a")));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: cannot modify 'let'
12:34 note: 'let a' declared here)");
}

TEST_F(ResolverIncrementDecrementValidationTest, Parameter) {
    // fn func(a : i32)
    // {
    //   a++;
    // }
    auto* a = Param(Source{{12, 34}}, "a", ty.i32());
    Func("func", Vector{a}, ty.void_(),
         Vector{
             Increment(Expr(Source{{56, 78}}, "a")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: cannot modify function parameter
12:34 note: parameter 'a' declared here)");
}

TEST_F(ResolverIncrementDecrementValidationTest, ReturnValue) {
    // fn func() -> i32 {
    //   return 0;
    // }
    // {
    //   a++;
    // }
    Func("func", tint::Empty, ty.i32(),
         Vector{
             Return(0_i),
         });
    WrapInFunction(Increment(Call(Source{{56, 78}}, "func")));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: cannot modify value of type 'i32')");
}

TEST_F(ResolverIncrementDecrementValidationTest, ReadOnlyBuffer) {
    // @group(0) @binding(0) var<storage,read> a : i32;
    // {
    //   a++;
    // }
    GlobalVar(Source{{12, 34}}, "a", ty.i32(), core::AddressSpace::kStorage, core::Access::kRead,
              Group(0_a), Binding(0_a));
    WrapInFunction(Increment(Source{{56, 78}}, "a"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: cannot modify read-only type 'ref<storage, i32, read>'");
}

TEST_F(ResolverIncrementDecrementValidationTest, Phony) {
    // _++;
    WrapInFunction(Increment(Phony(Source{{56, 78}})));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: cannot modify value of type 'void'");
}

TEST_F(ResolverIncrementDecrementValidationTest, InForLoopInit) {
    // var a : i32 = 2;
    // for (a++; ; ) {
    //   break;
    // }
    auto* a = Var("a", ty.i32(), Expr(2_i));
    auto* loop = For(Increment(Source{{56, 78}}, "a"), nullptr, nullptr, Block(Break()));
    WrapInFunction(a, loop);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIncrementDecrementValidationTest, InForLoopCont) {
    // var a : i32 = 2;
    // for (; ; a++) {
    //   break;
    // }
    auto* a = Var("a", ty.i32(), Expr(2_i));
    auto* loop = For(nullptr, nullptr, Increment(Source{{56, 78}}, "a"), Block(Break()));
    WrapInFunction(a, loop);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint::resolver
