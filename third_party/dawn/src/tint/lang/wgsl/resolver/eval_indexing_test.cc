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

#include "src/tint/lang/wgsl/resolver/eval_test.h"

namespace tint::core::constant::test {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(ConstEvalTest, Vec3_Index) {
    auto* expr = IndexAccessor(Call<vec3<i32>>(1_i, 2_i, 3_i), 2_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<core::type::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->ValueAs<i32>(), 3_i);
}

TEST_F(ConstEvalTest, Vec3_Index_OOB_High) {
    auto* expr = IndexAccessor(Call<vec3<i32>>(1_i, 2_i, 3_i), Expr(Source{{12, 34}}, 3_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index 3 out of bounds [0..2]");
}

TEST_F(ConstEvalTest, Vec3_Index_OOB_Low) {
    auto* expr = IndexAccessor(Call<vec3<i32>>(1_i, 2_i, 3_i), Expr(Source{{12, 34}}, -3_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index -3 out of bounds [0..2]");
}

TEST_F(ConstEvalTest, Vec3_Index_Via_Ptr_OOB_High) {
    auto* a = Var("a", ty.vec3(ty.i32()));
    auto* p = Let("p", ty.ptr(function, ty.vec3(ty.i32())), AddressOf(a));
    auto* expr = IndexAccessor("p", Expr(Source{{12, 34}}, 12_i));
    WrapInFunction(a, p, expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index 12 out of bounds [0..2]");
}

TEST_F(ConstEvalTest, Vec3_Index_Via_Ptr_OOB_Low) {
    auto* a = Var("a", ty.vec3(ty.i32()));
    auto* p = Let("p", ty.ptr(function, ty.vec3(ty.i32())), AddressOf(a));
    auto* expr = IndexAccessor("p", Expr(Source{{12, 34}}, -3_i));
    WrapInFunction(a, p, expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index -3 out of bounds [0..2]");
}

namespace Swizzle {
struct Case {
    Value input;
    const char* swizzle;
    Value expected;
};

static Case C(Value input, const char* swizzle, Value expected) {
    return Case{std::move(input), swizzle, std::move(expected)};
}

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    return o << "input: " << c.input << ", swizzle: " << c.swizzle << ", expected: " << c.expected;
}

using ConstEvalSwizzleTest = ConstEvalTestWithParam<Case>;
TEST_P(ConstEvalSwizzleTest, Test) {
    Enable(wgsl::Extension::kF16);
    auto& param = GetParam();
    auto* expr = MemberAccessor(param.input.Expr(*this), param.swizzle);
    auto* a = Const("a", expr);
    WrapInFunction(a);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    CheckConstant(sem->ConstantValue(), param.expected);
}
template <typename T>
std::vector<Case> SwizzleCases() {
    return {
        C(Vec(T(0), T(1), T(2)), "xyz", Vec(T(0), T(1), T(2))),
        C(Vec(T(0), T(1), T(2)), "xzy", Vec(T(0), T(2), T(1))),
        C(Vec(T(0), T(1), T(2)), "yxz", Vec(T(1), T(0), T(2))),
        C(Vec(T(0), T(1), T(2)), "yzx", Vec(T(1), T(2), T(0))),
        C(Vec(T(0), T(1), T(2)), "zxy", Vec(T(2), T(0), T(1))),
        C(Vec(T(0), T(1), T(2)), "zyx", Vec(T(2), T(1), T(0))),
        C(Vec(T(0), T(1), T(2)), "xy", Vec(T(0), T(1))),
        C(Vec(T(0), T(1), T(2)), "xz", Vec(T(0), T(2))),
        C(Vec(T(0), T(1), T(2)), "yx", Vec(T(1), T(0))),
        C(Vec(T(0), T(1), T(2)), "yz", Vec(T(1), T(2))),
        C(Vec(T(0), T(1), T(2)), "zx", Vec(T(2), T(0))),
        C(Vec(T(0), T(1), T(2)), "zy", Vec(T(2), T(1))),
        C(Vec(T(0), T(1), T(2)), "xxxx", Vec(T(0), T(0), T(0), T(0))),
        C(Vec(T(0), T(1), T(2)), "yyyy", Vec(T(1), T(1), T(1), T(1))),
        C(Vec(T(0), T(1), T(2)), "zzzz", Vec(T(2), T(2), T(2), T(2))),
        C(Vec(T(0), T(1), T(2)), "xxx", Vec(T(0), T(0), T(0))),
        C(Vec(T(0), T(1), T(2)), "yyy", Vec(T(1), T(1), T(1))),
        C(Vec(T(0), T(1), T(2)), "zzz", Vec(T(2), T(2), T(2))),
        C(Vec(T(0), T(1), T(2)), "xx", Vec(T(0), T(0))),
        C(Vec(T(0), T(1), T(2)), "yy", Vec(T(1), T(1))),
        C(Vec(T(0), T(1), T(2)), "zz", Vec(T(2), T(2))),
        C(Vec(T(0), T(1), T(2)), "x", Val(T(0))),
        C(Vec(T(0), T(1), T(2)), "y", Val(T(1))),
        C(Vec(T(0), T(1), T(2)), "z", Val(T(2))),
    };
}
INSTANTIATE_TEST_SUITE_P(Swizzle,
                         ConstEvalSwizzleTest,
                         testing::ValuesIn(Concat(SwizzleCases<AInt>(),    //
                                                  SwizzleCases<AFloat>(),  //
                                                  SwizzleCases<f32>(),     //
                                                  SwizzleCases<f16>(),     //
                                                  SwizzleCases<i32>(),     //
                                                  SwizzleCases<u32>(),     //
                                                  SwizzleCases<bool>()     //
                                                  )));
}  // namespace Swizzle

TEST_F(ConstEvalTest, Vec3_Swizzle_Scalar) {
    auto* expr = MemberAccessor(Call<vec3<i32>>(1_i, 2_i, 3_i), "y");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<core::type::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->ValueAs<i32>(), 2_i);
}

TEST_F(ConstEvalTest, Vec3_Swizzle_Vector) {
    auto* expr = MemberAccessor(Call<vec3<i32>>(1_i, 2_i, 3_i), "zx");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f32>(), 3._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f32>(), 1._a);
}

TEST_F(ConstEvalTest, Vec3_Swizzle_Chain) {
    auto* expr =  // (1, 2, 3) -> (2, 3, 1) -> (3, 2) -> 2
        MemberAccessor(MemberAccessor(MemberAccessor(Call<vec3<i32>>(1_i, 2_i, 3_i), "gbr"), "yx"),
                       "y");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<core::type::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->ValueAs<i32>(), 2_i);
}

TEST_F(ConstEvalTest, Mat3x2_Index) {
    auto* expr =
        IndexAccessor(Call<mat3x2<f32>>(Call<vec2<f32>>(1._a, 2._a), Call<vec2<f32>>(3._a, 4._a),
                                        Call<vec2<f32>>(5._a, 6._a)),
                      2_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f32>(), 5._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f32>(), 6._a);
}

TEST_F(ConstEvalTest, Mat3x2_Index_OOB_High) {
    auto* expr =
        IndexAccessor(Call<mat3x2<f32>>(Call<vec2<f32>>(1._a, 2._a), Call<vec2<f32>>(3._a, 4._a),
                                        Call<vec2<f32>>(5._a, 6._a)),
                      Expr(Source{{12, 34}}, 3_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index 3 out of bounds [0..2]");
}

TEST_F(ConstEvalTest, Mat3x2_Index_OOB_Low) {
    auto* expr =
        IndexAccessor(Call<mat3x2<f32>>(Call<vec2<f32>>(1._a, 2._a), Call<vec2<f32>>(3._a, 4._a),
                                        Call<vec2<f32>>(5._a, 6._a)),
                      Expr(Source{{12, 34}}, -3_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index -3 out of bounds [0..2]");
}

TEST_F(ConstEvalTest, Array_vec3_f32_Index) {
    auto* expr = IndexAccessor(Call<array<vec3<f32>, 2>>(           //
                                   Call<vec3<f32>>(1_f, 2_f, 3_f),  //
                                   Call<vec3<f32>>(4_f, 5_f, 6_f)),
                               1_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f32>(), 4_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f32>(), 5_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f32>(), 6_f);
}

TEST_F(ConstEvalTest, Array_vec3_f32_Index_OOB_High) {
    auto* expr = IndexAccessor(Call<array<vec3<f32>, 2>>(           //
                                   Call<vec3<f32>>(1_f, 2_f, 3_f),  //
                                   Call<vec3<f32>>(4_f, 5_f, 6_f)),
                               Expr(Source{{12, 34}}, 2_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index 2 out of bounds [0..1]");
}

TEST_F(ConstEvalTest, Array_vec3_f32_Index_OOB_Low) {
    auto* expr = IndexAccessor(Call<array<vec3<f32>, 2>>(           //
                                   Call<vec3<f32>>(1_f, 2_f, 3_f),  //
                                   Call<vec3<f32>>(4_f, 5_f, 6_f)),
                               Expr(Source{{12, 34}}, -2_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index -2 out of bounds [0..1]");
}

TEST_F(ConstEvalTest, RuntimeArray_vec3_f32_Index_OOB_Low) {
    auto* sb = GlobalVar("sb", ty.array<vec3<f32>>(), Group(0_a), Binding(0_a),
                         core::AddressSpace::kStorage);
    auto* expr = IndexAccessor(sb, Expr(Source{{12, 34}}, -2_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index -2 out of bounds");
}

TEST_F(ConstEvalTest, BindingArray_Index_OOB_Low) {
    GlobalVar(
        "a", Binding(0_a), Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));
    auto* acc = IndexAccessor("a", Expr(Source{{12, 34}}, -1_i));
    auto* call = Call("textureDimensions", acc);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index -1 out of bounds [0..3]");
}

TEST_F(ConstEvalTest, BindingArray_Index_OOB_High) {
    GlobalVar(
        "a", Binding(0_a), Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));
    auto* acc = IndexAccessor("a", Expr(Source{{12, 34}}, 4_i));
    auto* call = Call("textureDimensions", acc);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index 4 out of bounds [0..3]");
}

TEST_F(ConstEvalTest, ChainedIndex) {
    auto* arr_expr = Call<array<mat2x3<f32>, 2>>(           //
        Call<mat2x3<f32>>(Call<vec3<f32>>(1_f, 2_f, 3_f),   //
                          Call<vec3<f32>>(4_f, 5_f, 6_f)),  //
        Call<mat2x3<f32>>(Call<vec3<f32>>(7_f, 0_f, 9_f),   //
                          Call<vec3<f32>>(10_f, 11_f, 12_f)));

    auto* mat_expr = IndexAccessor(arr_expr, 1_i);  // arr[1]
    auto* vec_expr = IndexAccessor(mat_expr, 0_i);  // arr[1][0]
    auto* f32_expr = IndexAccessor(vec_expr, 2_i);  // arr[1][0][2]
    WrapInFunction(f32_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* mat = Sem().Get(mat_expr);
        EXPECT_NE(mat, nullptr);
        auto* ty = mat->Type()->As<core::type::Matrix>();
        ASSERT_NE(mat->Type(), nullptr);
        EXPECT_TRUE(ty->ColumnType()->Is<core::type::Vector>());
        EXPECT_EQ(ty->Columns(), 2u);
        EXPECT_EQ(ty->Rows(), 3u);
        EXPECT_EQ(mat->ConstantValue()->Type(), mat->Type());
        EXPECT_TRUE(mat->ConstantValue()->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->AllZero());

        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(0)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(0)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(0)->ValueAs<f32>(), 7_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(1)->AnyZero());
        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(1)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(1)->ValueAs<f32>(), 0_f);

        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(2)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(2)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(2)->ValueAs<f32>(), 9_f);

        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(0)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(0)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(0)->ValueAs<f32>(), 10_f);

        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(1)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(1)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(1)->ValueAs<f32>(), 11_f);

        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(2)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(2)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(2)->ValueAs<f32>(), 12_f);
    }
    {
        auto* vec = Sem().Get(vec_expr);
        EXPECT_NE(vec, nullptr);
        auto* ty = vec->Type()->As<core::type::Vector>();
        ASSERT_NE(vec->Type(), nullptr);
        EXPECT_TRUE(ty->Type()->Is<core::type::F32>());
        EXPECT_EQ(ty->Width(), 3u);
        EXPECT_EQ(vec->ConstantValue()->Type(), vec->Type());
        EXPECT_TRUE(vec->ConstantValue()->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->AllZero());

        EXPECT_FALSE(vec->ConstantValue()->Index(0)->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->Index(0)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(0)->ValueAs<f32>(), 7_f);

        EXPECT_TRUE(vec->ConstantValue()->Index(1)->AnyZero());
        EXPECT_TRUE(vec->ConstantValue()->Index(1)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(1)->ValueAs<f32>(), 0_f);

        EXPECT_FALSE(vec->ConstantValue()->Index(2)->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->Index(2)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(2)->ValueAs<f32>(), 9_f);
    }
    {
        auto* f = Sem().Get(f32_expr);
        EXPECT_NE(f, nullptr);
        EXPECT_TRUE(f->Type()->Is<core::type::F32>());
        EXPECT_EQ(f->ConstantValue()->Type(), f->Type());
        EXPECT_FALSE(f->ConstantValue()->AnyZero());
        EXPECT_FALSE(f->ConstantValue()->AllZero());
        EXPECT_EQ(f->ConstantValue()->ValueAs<f32>(), 9_f);
    }
}
}  // namespace
}  // namespace tint::core::constant::test
