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

#include "gmock/gmock.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using ::testing::HasSubstr;
using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverCompoundAssignmentValidationTest = ResolverTest;

TEST_F(ResolverCompoundAssignmentValidationTest, CompatibleTypes) {
    // var a : i32 = 2;
    // a += 2
    auto* var = Var("a", ty.i32(), Expr(2_i));
    WrapInFunction(var, CompoundAssign(Source{{12, 34}}, "a", 2_i, core::BinaryOp::kAdd));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, CompatibleTypesThroughAlias) {
    // alias myint = i32;
    // var a : myint = 2;
    // a += 2
    auto* myint = Alias("myint", ty.i32());
    auto* var = Var("a", ty.Of(myint), Expr(2_i));
    WrapInFunction(var, CompoundAssign(Source{{12, 34}}, "a", 2_i, core::BinaryOp::kAdd));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, CompatibleTypesAssignThroughPointer) {
    // var a : i32;
    // let b : ptr<function,i32> = &a;
    // *b += 2;
    auto* var_a = Var("a", ty.i32(), core::AddressSpace::kFunction, Expr(2_i));
    auto* var_b = Let("b", ty.ptr<function, i32>(), AddressOf(Expr("a")));
    WrapInFunction(var_a, var_b,
                   CompoundAssign(Source{{12, 34}}, Deref("b"), 2_i, core::BinaryOp::kAdd));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, IncompatibleTypes) {
    // {
    //   var a : i32 = 2;
    //   a += 2.3;
    // }

    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", 2.3_f, core::BinaryOp::kAdd);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching overload for 'operator += (i32, f32)'"));
}

TEST_F(ResolverCompoundAssignmentValidationTest, IncompatibleOp) {
    // {
    //   var a : f32 = 1.0;
    //   a |= 2.0;
    // }

    auto* var = Var("a", ty.f32(), Expr(1_f));

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", 2_f, core::BinaryOp::kOr);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching overload for 'operator |= (f32, f32)'"));
}

TEST_F(ResolverCompoundAssignmentValidationTest, VectorScalar_Pass) {
    // {
    //   var a : vec4<f32>;
    //   a += 1.0;
    // }

    auto* var = Var("a", ty.vec4<f32>());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", 1_f, core::BinaryOp::kAdd);
    WrapInFunction(var, assign);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, ScalarVector_Fail) {
    // {
    //   var a : f32;
    //   a += vec4<f32>();
    // }

    auto* var = Var("a", ty.f32());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", Call<vec4<f32>>(), core::BinaryOp::kAdd);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'vec4<f32>' to 'f32'");
}

TEST_F(ResolverCompoundAssignmentValidationTest, MatrixScalar_Pass) {
    // {
    //   var a : mat4x4<f32>;
    //   a *= 2.0;
    // }

    auto* var = Var("a", ty.mat4x4<f32>());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", 2_f, core::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, ScalarMatrix_Fail) {
    // {
    //   var a : f32;
    //   a *= mat4x4();
    // }

    auto* var = Var("a", ty.f32());

    auto* assign =
        CompoundAssign(Source{{12, 34}}, "a", Call<mat4x4<f32>>(), core::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'mat4x4<f32>' to 'f32'");
}

TEST_F(ResolverCompoundAssignmentValidationTest, VectorMatrix_Pass) {
    // {
    //   var a : vec4<f32>;
    //   a *= mat4x4();
    // }

    auto* var = Var("a", ty.vec4<f32>());

    auto* assign =
        CompoundAssign(Source{{12, 34}}, "a", Call<mat4x4<f32>>(), core::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, VectorMatrix_ColumnMismatch) {
    // {
    //   var a : vec4<f32>;
    //   a *= mat4x2();
    // }

    auto* var = Var("a", ty.vec4<f32>());

    auto* assign =
        CompoundAssign(Source{{12, 34}}, "a", Call<mat4x2<f32>>(), core::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching overload for 'operator *= (vec4<f32>, mat4x2<f32>)'"));
}

TEST_F(ResolverCompoundAssignmentValidationTest, VectorMatrix_ResultMismatch) {
    // {
    //   var a : vec4<f32>;
    //   a *= mat2x4();
    // }

    auto* var = Var("a", ty.vec4<f32>());

    auto* assign =
        CompoundAssign(Source{{12, 34}}, "a", Call<mat2x4<f32>>(), core::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'vec2<f32>' to 'vec4<f32>'");
}

TEST_F(ResolverCompoundAssignmentValidationTest, MatrixVector_Fail) {
    // {
    //   var a : mat4x4<f32>;
    //   a *= vec4();
    // }

    auto* var = Var("a", ty.mat4x4<f32>());

    auto* assign =
        CompoundAssign(Source{{12, 34}}, "a", Call<vec4<f32>>(), core::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'vec4<f32>' to 'mat4x4<f32>'");
}

TEST_F(ResolverCompoundAssignmentValidationTest, Phony) {
    // {
    //   _ += 1i;
    // }
    WrapInFunction(CompoundAssign(Source{{56, 78}}, Phony(), 1_i, core::BinaryOp::kAdd));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("56:78 error: no matching overload for 'operator += (void, i32)'"));
}

TEST_F(ResolverCompoundAssignmentValidationTest, ReadOnlyBuffer) {
    // @group(0) @binding(0) var<storage,read> a : i32;
    // {
    //   a += 1i;
    // }
    GlobalVar(Source{{12, 34}}, "a", ty.i32(), core::AddressSpace::kStorage, core::Access::kRead,
              Group(0_a), Binding(0_a));
    WrapInFunction(CompoundAssign(Source{{56, 78}}, "a", 1_i, core::BinaryOp::kAdd));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: cannot store into a read-only type 'ref<storage, i32, read>'");
}

TEST_F(ResolverCompoundAssignmentValidationTest, LhsLet) {
    // let a = 1i;
    // a += 1i;
    auto* a = Let(Source{{12, 34}}, "a", Expr(1_i));
    WrapInFunction(a, CompoundAssign(Expr(Source{{56, 78}}, "a"), 1_i, core::BinaryOp::kAdd));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: cannot assign to 'let a'
56:78 note: 'let' variables are immutable
12:34 note: 'let a' declared here)");
}

TEST_F(ResolverCompoundAssignmentValidationTest, LhsLiteral) {
    // 1i += 1i;
    WrapInFunction(CompoundAssign(Expr(Source{{56, 78}}, 1_i), 1_i, core::BinaryOp::kAdd));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: cannot assign to value of type 'i32')");
}

TEST_F(ResolverCompoundAssignmentValidationTest, LhsAtomic) {
    // var<workgroup> a : atomic<i32>;
    // a += a;
    GlobalVar(Source{{12, 34}}, "a", ty.atomic(ty.i32()), core::AddressSpace::kWorkgroup);
    WrapInFunction(CompoundAssign(Source{{56, 78}}, "a", "a", core::BinaryOp::kAdd));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("error: no matching overload for 'operator += (atomic<i32>, atomic<i32>)'"));
}

}  // namespace
}  // namespace tint::resolver
