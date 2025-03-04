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

#include "src/tint/lang/core/constant/eval_test.h"

namespace tint::core::constant::test {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(ConstEvalTest, Scalar_AFloat) {
    auto* expr = Expr(99.0_a);
    auto* a = Const("a", expr);
    WrapInFunction(a);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<core::type::AbstractFloat>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->ValueAs<AFloat>(), 99.0f);
}

TEST_F(ConstEvalTest, Scalar_AInt) {
    auto* expr = Expr(99_a);
    auto* a = Const("a", expr);
    WrapInFunction(a);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<core::type::AbstractInt>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->ValueAs<AInt>(), 99);
}

TEST_F(ConstEvalTest, Scalar_i32) {
    auto* expr = Expr(99_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<core::type::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->ValueAs<AInt>(), 99);
}

TEST_F(ConstEvalTest, Scalar_u32) {
    auto* expr = Expr(99_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<core::type::U32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->ValueAs<AInt>(), 99u);
}

TEST_F(ConstEvalTest, Scalar_f32) {
    auto* expr = Expr(9.9_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<core::type::F32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->ValueAs<AFloat>().value, 9.9f);
}

TEST_F(ConstEvalTest, Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Expr(9.9_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<core::type::F16>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    // 9.9 is not exactly representable by f16, and should be quantized to 9.8984375
    EXPECT_EQ(sem->ConstantValue()->ValueAs<AFloat>(), 9.8984375f);
}

TEST_F(ConstEvalTest, Scalar_bool) {
    auto* expr = Expr(true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<core::type::Bool>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->ValueAs<bool>(), true);
}

namespace ZeroInit {
struct Case {
    builder::ast_type_func_ptr type;
};
template <typename T>
Case C() {
    return Case{builder::DataType<T>::AST};
}
using ConstEvalZeroInitTest = ConstEvalTestWithParam<Case>;
TEST_P(ConstEvalZeroInitTest, Test) {
    Enable(wgsl::Extension::kF16);
    auto& param = GetParam();
    auto ty = param.type(*this);
    auto* expr = Call(ty);
    auto* a = Const("a", expr);
    WrapInFunction(a);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);

    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    if (sem->Type()->Is<core::type::Scalar>()) {
        EXPECT_EQ(sem->ConstantValue()->Index(0), nullptr);
        EXPECT_EQ(sem->ConstantValue()->ValueAs<f32>(), 0.0f);
    } else if (auto* vec = sem->Type()->As<core::type::Vector>()) {
        for (size_t i = 0; i < vec->Width(); ++i) {
            EXPECT_TRUE(sem->ConstantValue()->Index(i)->AnyZero());
            EXPECT_TRUE(sem->ConstantValue()->Index(i)->AllZero());
            EXPECT_EQ(sem->ConstantValue()->Index(i)->ValueAs<f32>(), 0.0f);
        }
    } else if (auto* mat = sem->Type()->As<core::type::Matrix>()) {
        for (size_t i = 0; i < mat->Columns(); ++i) {
            EXPECT_TRUE(sem->ConstantValue()->Index(i)->AnyZero());
            EXPECT_TRUE(sem->ConstantValue()->Index(i)->AllZero());
            for (size_t j = 0; j < mat->Rows(); ++j) {
                EXPECT_TRUE(sem->ConstantValue()->Index(i)->Index(j)->AnyZero());
                EXPECT_TRUE(sem->ConstantValue()->Index(i)->Index(j)->AllZero());
                EXPECT_EQ(sem->ConstantValue()->Index(i)->Index(j)->ValueAs<f32>(), 0.0f);
            }
        }
    } else if (auto* arr = sem->Type()->As<core::type::Array>()) {
        for (size_t i = 0; i < *(arr->ConstantCount()); ++i) {
            EXPECT_TRUE(sem->ConstantValue()->Index(i)->AnyZero());
            EXPECT_TRUE(sem->ConstantValue()->Index(i)->AllZero());
            EXPECT_EQ(sem->ConstantValue()->Index(i)->ValueAs<f32>(), 0.0f);
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ZeroInit,
                         ConstEvalZeroInitTest,
                         testing::ValuesIn({
                             C<u32>(),
                             C<i32>(),
                             C<f32>(),
                             C<f16>(),
                             C<bool>(),
                             C<vec2<AInt>>(),
                             C<vec3<AInt>>(),
                             C<vec4<AInt>>(),
                             C<vec3<u32>>(),
                             C<vec3<i32>>(),
                             C<vec3<f32>>(),
                             C<vec3<f16>>(),
                             C<mat2x2<f32>>(),
                             C<mat2x2<f16>>(),
                             C<array<u32, 3>>(),
                             C<array<i32, 3>>(),
                             C<array<f32, 3>>(),
                             C<array<f16, 3>>(),
                             C<array<bool, 3>>(),
                         }));

}  // namespace ZeroInit

TEST_F(ConstEvalTest, Vec3_ZeroInit_i32) {
    auto* expr = Call<vec3<i32>>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 0);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 0);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(), 0);
}

TEST_F(ConstEvalTest, Vec3_ZeroInit_u32) {
    auto* expr = Call<vec3<u32>>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 0u);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 0u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(), 0u);
}

TEST_F(ConstEvalTest, Vec3_ZeroInit_f32) {
    auto* expr = Call<vec3<f32>>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 0._a);
}

TEST_F(ConstEvalTest, Vec3_ZeroInit_f16) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<f16>>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 0._a);
}

TEST_F(ConstEvalTest, Vec3_ZeroInit_bool) {
    auto* expr = Call<vec3<bool>>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<bool>(), false);
}

TEST_F(ConstEvalTest, Vec3_Splat_i32) {
    auto* expr = Call<vec3<i32>>(99_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 99);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 99);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(), 99);
}

TEST_F(ConstEvalTest, Vec3_Splat_u32) {
    auto* expr = Call<vec3<u32>>(99_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 99u);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 99u);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(), 99u);
}

TEST_F(ConstEvalTest, Vec3_Splat_f32) {
    auto* expr = Call<vec3<f32>>(9.9_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 9.9f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 9.9f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 9.9f);
}

TEST_F(ConstEvalTest, Vec3_Splat_f16) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<f16>>(9.9_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    // 9.9 is not exactly representable by f16, and should be quantized to 9.8984375

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 9.8984375f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 9.8984375f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 9.8984375f);
}

TEST_F(ConstEvalTest, Vec3_Splat_bool) {
    auto* expr = Call<vec3<bool>>(true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<bool>(), true);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<bool>(), true);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<bool>(), true);
}

TEST_F(ConstEvalTest, Vec3_FullConstruct_AInt) {
    auto* expr = Call<vec3<Infer>>(1_a, 2_a, 3_a);
    auto* a = Const("a", expr);
    WrapInFunction(a);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::AbstractInt>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 1);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 2);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(), 3);
}

TEST_F(ConstEvalTest, Vec3_FullConstruct_AFloat) {
    auto* expr = Call<vec3<Infer>>(1.0_a, 2.0_a, 3.0_a);
    auto* a = Const("a", expr);
    WrapInFunction(a);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::AbstractFloat>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 1.0f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 2.0f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 3.0f);
}

TEST_F(ConstEvalTest, Vec3_FullConstruct_i32) {
    auto* expr = Call<vec3<i32>>(1_i, 2_i, 3_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 1);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 2);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(), 3);
}

TEST_F(ConstEvalTest, Vec3_FullConstruct_u32) {
    auto* expr = Call<vec3<u32>>(1_u, 2_u, 3_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 1);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 2);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(), 3);
}

TEST_F(ConstEvalTest, Vec3_FullConstruct_f32) {
    auto* expr = Call<vec3<f32>>(1_f, 2_f, 3_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 1.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 2.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 3.f);
}

TEST_F(ConstEvalTest, Vec3_FullConstruct_f16) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<f16>>(1_h, 2_h, 3_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 1.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 2.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 3.f);
}

TEST_F(ConstEvalTest, Vec3_FullConstruct_bool) {
    auto* expr = Call<vec3<bool>>(true, false, true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<bool>(), false);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<bool>(), true);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_i32) {
    auto* expr = Call<vec3<i32>>(1_i, Call<vec2<i32>>(2_i, 3_i));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 1);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 2);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(), 3);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_u32) {
    auto* expr = Call<vec3<u32>>(Call<vec2<u32>>(1_u, 2_u), 3_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 1);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 2);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(), 3);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_f32) {
    auto* expr = Call<vec3<f32>>(1_f, Call<vec2<f32>>(2_f, 3_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 1.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 2.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 3.f);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_f32_all_10) {
    auto* expr = Call<vec3<f32>>(10_f, Call<vec2<f32>>(10_f, 10_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f32>(), 10_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f32>(), 10_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f32>(), 10_f);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_f32_all_positive_0) {
    auto* expr = Call<vec3<f32>>(0_f, Call<vec2<f32>>(0_f, 0_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f32>(), 0_f);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_f32_all_negative_0) {
    auto* expr = Call<vec3<f32>>(Call<vec2<f32>>(-0_f, -0_f), -0_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f32>(), -0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f32>(), -0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f32>(), -0_f);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_f32_mixed_sign_0) {
    auto* expr = Call<vec3<f32>>(0_f, Call<vec2<f32>>(-0_f, 0_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f32>(), -0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f32>(), 0_f);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_f16) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<f16>>(1_h, Call<vec2<f16>>(2_h, 3_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 1.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 2.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 3.f);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_f16_all_10) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<f16>>(10_h, Call<vec2<f16>>(10_h, 10_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f16>(), 10_h);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f16>(), 10_h);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f16>(), 10_h);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_f16_all_positive_0) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<f16>>(0_h, Call<vec2<f16>>(0_h, 0_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f16>(), 0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f16>(), 0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f16>(), 0_h);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_f16_all_negative_0) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<f16>>(Call<vec2<f16>>(-0_h, -0_h), -0_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f16>(), -0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f16>(), -0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f16>(), -0_h);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_f16_mixed_sign_0) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<f16>>(0_h, Call<vec2<f16>>(-0_h, 0_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f16>(), 0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f16>(), -0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f16>(), 0_h);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_bool) {
    auto* expr = Call<vec3<bool>>(Call<vec2<bool>>(true, false), true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<bool>(), false);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<bool>(), true);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_all_true) {
    auto* expr = Call<vec3<bool>>(true, Call<vec2<bool>>(true, true));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<bool>(), true);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<bool>(), true);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<bool>(), true);
}

TEST_F(ConstEvalTest, Vec3_MixConstruct_all_false) {
    auto* expr = Call<vec3<bool>>(false, Call<vec2<bool>>(false, false));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<bool>(), false);
}

TEST_F(ConstEvalTest, Mat2x3_ZeroInit_f32) {
    auto* expr = Call<mat2x3<f32>>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<core::type::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->Type()->Is<core::type::F32>());
    EXPECT_EQ(mat->Columns(), 2u);
    EXPECT_EQ(mat->Rows(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->ValueAs<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->ValueAs<f32>(), 0._f);
}

TEST_F(ConstEvalTest, Mat2x3_ZeroInit_f16) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<mat2x3<f16>>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<core::type::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->Type()->Is<core::type::F16>());
    EXPECT_EQ(mat->Columns(), 2u);
    EXPECT_EQ(mat->Rows(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->ValueAs<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->ValueAs<f16>(), 0._h);
}

TEST_F(ConstEvalTest, Mat3x2_Construct_Scalars_af) {
    auto* expr = Call<mat3x2<Infer>>(1.0_a, 2.0_a, 3.0_a, 4.0_a, 5.0_a, 6.0_a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<core::type::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->Type()->Is<core::type::F32>());
    EXPECT_EQ(mat->Columns(), 3u);
    EXPECT_EQ(mat->Rows(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<AFloat>(), 1._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<AFloat>(), 2._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<AFloat>(), 3._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<AFloat>(), 4._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->ValueAs<AFloat>(), 5._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->ValueAs<AFloat>(), 6._a);
}

TEST_F(ConstEvalTest, Mat3x2_Construct_Columns_af) {
    auto* expr = Call<mat<3, 2, Infer>>(  //
        Call<vec2<Infer>>(1.0_a, 2.0_a),  //
        Call<vec2<Infer>>(3.0_a, 4.0_a),  //
        Call<vec2<Infer>>(5.0_a, 6.0_a));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<core::type::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->Type()->Is<core::type::F32>());
    EXPECT_EQ(mat->Columns(), 3u);
    EXPECT_EQ(mat->Rows(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<AFloat>(), 1._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<AFloat>(), 2._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<AFloat>(), 3._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<AFloat>(), 4._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->ValueAs<AFloat>(), 5._a);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->ValueAs<AFloat>(), 6._a);
}

TEST_F(ConstEvalTest, Array_i32_Zero) {
    auto* expr = Call<array<i32, 4>>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->ValueAs<i32>(), 0_i);
}

TEST_F(ConstEvalTest, Array_f32_Zero) {
    auto* expr = Call<array<f32, 4>>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::F32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->ValueAs<f32>(), 0_f);
}

TEST_F(ConstEvalTest, Array_vec3_f32_Zero) {
    auto* expr = Call<array<vec3<f32>, 2>>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::Vector>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->ValueAs<f32>(), 0_f);
}

TEST_F(ConstEvalTest, Array_Struct_f32_Zero) {
    Structure("S", Vector{
                       Member("m1", ty.f32()),
                       Member("m2", ty.f32()),
                   });
    auto* expr = Call(ty.array(ty("S"), 2_u));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    ASSERT_EQ(arr->ConstantCount(), 2u);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::Struct>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<f32>(), 0_f);
}

TEST_F(ConstEvalTest, Array_i32_Elements) {
    auto* expr = Call<array<i32, 4>>(10_i, 20_i, 30_i, 40_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    ASSERT_EQ(arr->ConstantCount(), 4u);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<i32>(), 10_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<i32>(), 20_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<i32>(), 30_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->ValueAs<i32>(), 40_i);
}

TEST_F(ConstEvalTest, Array_Infer_i32_i32) {
    auto* expr = Call<array<Infer>>(10_i, 20_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    ASSERT_EQ(arr->ConstantCount(), 2u);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<i32>(), 10_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<i32>(), 20_i);
}

TEST_F(ConstEvalTest, Array_Infer_ai_ai) {
    auto* expr = Call<array<Infer>>(10_a, 20_a);
    GlobalConst("C", expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    ASSERT_EQ(arr->ConstantCount(), 2u);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::AbstractInt>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 10_a);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 20_a);
}

TEST_F(ConstEvalTest, Array_Infer_af_af) {
    auto* expr = Call<array<Infer>>(10.0_a, 20.0_a);
    GlobalConst("C", expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    ASSERT_EQ(arr->ConstantCount(), 2u);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::AbstractFloat>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 10_a);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 20_a);
}

TEST_F(ConstEvalTest, Array_Infer_af_ai) {
    auto* expr = Call<array<Infer>>(10.0_a, 20_a);
    GlobalConst("C", expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    ASSERT_EQ(arr->ConstantCount(), 2u);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::AbstractFloat>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 10_a);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 20_a);
}

TEST_F(ConstEvalTest, Array_Infer_ai_af) {
    auto* expr = Call<array<Infer>>(10_a, 20.0_a);
    GlobalConst("C", expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    ASSERT_EQ(arr->ConstantCount(), 2u);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::AbstractFloat>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 10_a);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 20_a);
}

namespace ArrayInit {
struct Case {
    Value input;
};
static Case C(Value input) {
    return Case{std::move(input)};
}
static std::ostream& operator<<(std::ostream& o, const Case& c) {
    return o << "input: " << c.input;
}

using ResolverConstEvalArrayInitTest = ConstEvalTestWithParam<Case>;
TEST_P(ResolverConstEvalArrayInitTest, Test) {
    Enable(wgsl::Extension::kF16);
    auto& param = GetParam();
    auto* expr = param.input.Expr(*this);
    auto* a = Const("a", expr);
    WrapInFunction(a);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().GetVal(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);

    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    // Constant values should match input values
    CheckConstant(sem->ConstantValue(), param.input);
}
template <typename T>
std::vector<Case> ArrayInitCases() {
    return {
        C(Array(T(0))),              //
        C(Array(T(0))),              //
        C(Array(T(0), T(1))),        //
        C(Array(T(0), T(1), T(2))),  //
        C(Array(T(2), T(1), T(0))),  //
        C(Array(T(2), T(0), T(1))),  //
    };
}
INSTANTIATE_TEST_SUITE_P(  //
    ArrayInit,
    ResolverConstEvalArrayInitTest,
    testing::ValuesIn(Concat(ArrayInitCases<AInt>(),    //
                             ArrayInitCases<AFloat>(),  //
                             ArrayInitCases<i32>(),     //
                             ArrayInitCases<u32>(),     //
                             ArrayInitCases<f32>(),     //
                             ArrayInitCases<f16>(),     //
                             ArrayInitCases<bool>())));
}  // namespace ArrayInit

TEST_F(ConstEvalTest, ArrayInit_Nested_f32) {
    auto inner_ty = [&] { return ty.array<f32, 2>(); };
    auto outer_ty = ty.array(inner_ty(), Expr(3_i));

    auto* expr = Call(outer_ty,                    //
                      Call(inner_ty(), 1_f, 2_f),  //
                      Call(inner_ty(), 3_f, 4_f),  //
                      Call(inner_ty(), 5_f, 6_f));

    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* outer_arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(outer_arr, nullptr);
    EXPECT_TRUE(outer_arr->ElemType()->Is<core::type::Array>());
    EXPECT_TRUE(outer_arr->ElemType()->As<core::type::Array>()->ElemType()->Is<core::type::F32>());

    auto* arr = sem->ConstantValue();
    EXPECT_FALSE(arr->AnyZero());
    EXPECT_FALSE(arr->AllZero());

    EXPECT_FALSE(arr->Index(0)->AnyZero());
    EXPECT_FALSE(arr->Index(0)->AllZero());
    EXPECT_FALSE(arr->Index(1)->AnyZero());
    EXPECT_FALSE(arr->Index(1)->AllZero());
    EXPECT_FALSE(arr->Index(2)->AnyZero());
    EXPECT_FALSE(arr->Index(2)->AllZero());

    EXPECT_EQ(arr->Index(0)->Index(0)->ValueAs<f32>(), 1.0f);
    EXPECT_EQ(arr->Index(0)->Index(1)->ValueAs<f32>(), 2.0f);
    EXPECT_EQ(arr->Index(1)->Index(0)->ValueAs<f32>(), 3.0f);
    EXPECT_EQ(arr->Index(1)->Index(1)->ValueAs<f32>(), 4.0f);
    EXPECT_EQ(arr->Index(2)->Index(0)->ValueAs<f32>(), 5.0f);
    EXPECT_EQ(arr->Index(2)->Index(1)->ValueAs<f32>(), 6.0f);
}

TEST_F(ConstEvalTest, Array_f32_Elements) {
    auto* expr = Call<array<f32, 4>>(10_f, 20_f, 30_f, 40_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::F32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<f32>(), 10_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<f32>(), 20_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f32>(), 30_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->ValueAs<f32>(), 40_f);
}

TEST_F(ConstEvalTest, Array_vec3_f32_Elements) {
    auto* expr =
        Call<array<vec3<f32>, 2>>(Call<vec3<f32>>(1_f, 2_f, 3_f), Call<vec3<f32>>(4_f, 5_f, 6_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::Vector>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<f32>(), 1_f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<f32>(), 2_f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->ValueAs<f32>(), 3_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<f32>(), 4_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<f32>(), 5_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->ValueAs<f32>(), 6_f);
}

TEST_F(ConstEvalTest, Array_Struct_f32_Elements) {
    Structure("S", Vector{
                       Member("m1", ty.f32()),
                       Member("m2", ty.f32()),
                   });
    auto* expr = Call(ty.array(ty("S"), 2_u),  //
                      Call("S", 1_f, 2_f),     //
                      Call("S", 3_f, 4_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<core::type::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<core::type::Struct>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<f32>(), 1_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<f32>(), 2_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<f32>(), 3_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<f32>(), 4_f);
}

TEST_F(ConstEvalTest, Struct_ZeroInit) {
    Enable(wgsl::Extension::kF16);
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.u32()),
                                 Member("c", ty.f32()),
                                 Member("d", ty.f16()),
                                 Member("e", ty.bool_()),
                             });

    auto* expr = Call(ty.Of(s));
    auto* a = Const("a", expr);
    WrapInFunction(a);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 5u);

    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<core::type::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<i32>(), 0);
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<core::type::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<u32>(), 0u);
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<core::type::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f32>(), 0.0f);
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<core::type::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->ValueAs<f16>(), 0.0f);
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<core::type::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->ValueAs<bool>(), false);

    for (size_t i = 0; i < str->Members().Length(); ++i) {
        EXPECT_TRUE(sem->ConstantValue()->Index(i)->AnyZero());
        EXPECT_TRUE(sem->ConstantValue()->Index(i)->AllZero());
    }
}

TEST_F(ConstEvalTest, Struct_Nested_ZeroInit) {
    Enable(wgsl::Extension::kF16);
    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.i32()),
                                         Member("b", ty.u32()),
                                         Member("c", ty.f32()),
                                         Member("d", ty.f16()),
                                         Member("e", ty.bool_()),
                                     });
    auto* s = Structure("s",  //
                        Vector{
                            Member("inner", ty.Of(inner)),
                        });

    auto* expr = Call(ty.Of(s));
    auto* a = Const("a", expr);
    WrapInFunction(a);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 1u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    auto* inner_struct = sem->ConstantValue()->Index(0);
    EXPECT_TRUE(inner_struct->AnyZero());
    EXPECT_TRUE(inner_struct->AllZero());

    EXPECT_TRUE(inner_struct->Index(0)->Type()->Is<core::type::I32>());
    EXPECT_EQ(inner_struct->Index(0)->ValueAs<i32>(), 0);
    EXPECT_TRUE(inner_struct->Index(1)->Type()->Is<core::type::U32>());
    EXPECT_EQ(inner_struct->Index(1)->ValueAs<u32>(), 0u);
    EXPECT_TRUE(inner_struct->Index(2)->Type()->Is<core::type::F32>());
    EXPECT_EQ(inner_struct->Index(2)->ValueAs<f32>(), 0.0f);
    EXPECT_TRUE(inner_struct->Index(3)->Type()->Is<core::type::F16>());
    EXPECT_EQ(inner_struct->Index(3)->ValueAs<f16>(), 0.0f);
    EXPECT_TRUE(inner_struct->Index(4)->Type()->Is<core::type::Bool>());
    EXPECT_EQ(inner_struct->Index(4)->ValueAs<bool>(), false);

    for (size_t i = 0; i < str->Members().Length(); ++i) {
        EXPECT_TRUE(inner_struct->Index(i)->AnyZero());
        EXPECT_TRUE(inner_struct->Index(i)->AllZero());
    }
}

TEST_F(ConstEvalTest, Struct_I32s_ZeroInit) {
    Structure("S", Vector{Member("m1", ty.i32()), Member("m2", ty.i32()), Member("m3", ty.i32())});
    auto* expr = Call("S");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 3u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<core::type::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<core::type::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<core::type::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<i32>(), 0_i);
}

TEST_F(ConstEvalTest, Struct_MixedScalars_ZeroInit) {
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{
                       Member("m1", ty.i32()),
                       Member("m2", ty.u32()),
                       Member("m3", ty.f32()),
                       Member("m4", ty.f16()),
                       Member("m5", ty.bool_()),
                   });
    auto* expr = Call("S");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<core::type::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<core::type::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<u32>(), 0_u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<core::type::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<core::type::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->ValueAs<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<core::type::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->ValueAs<bool>(), false);
}

TEST_F(ConstEvalTest, Struct_VectorF32s_ZeroInit) {
    Structure("S", Vector{
                       Member("m1", ty.vec3<f32>()),
                       Member("m2", ty.vec3<f32>()),
                       Member("m3", ty.vec3<f32>()),
                   });
    auto* expr = Call("S");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 3u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(0)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->ValueAs<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(1)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->ValueAs<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(2)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->ValueAs<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->ValueAs<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(2)->ValueAs<f32>(), 0._f);
}

TEST_F(ConstEvalTest, Struct_MixedVectors_ZeroInit) {
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{
                       Member("m1", ty.vec2<i32>()),
                       Member("m2", ty.vec3<u32>()),
                       Member("m3", ty.vec4<f32>()),
                       Member("m4", ty.vec3<f16>()),
                       Member("m5", ty.vec2<bool>()),
                   });
    auto* expr = Call("S");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(0)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<i32>(), 0_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(1)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->ValueAs<u32>(), 0_u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(2)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->ValueAs<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->ValueAs<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(2)->ValueAs<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(3)->ValueAs<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(3)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(0)->ValueAs<f16>(), 0._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(1)->ValueAs<f16>(), 0._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(2)->ValueAs<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(4)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(0)->ValueAs<bool>(), false);
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(1)->ValueAs<bool>(), false);
}

TEST_F(ConstEvalTest, Struct_Struct_ZeroInit) {
    Structure("Inner", Vector{
                           Member("m1", ty.i32()),
                           Member("m2", ty.u32()),
                           Member("m3", ty.f32()),
                       });

    Structure("Outer", Vector{
                           Member("m1", ty("Inner")),
                           Member("m2", ty("Inner")),
                       });
    auto* expr = Call("Outer");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 2u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<core::type::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<i32>(), 0_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<core::type::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<i32>(), 0_i);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->ValueAs<f32>(), 0_f);
}

TEST_F(ConstEvalTest, Struct_MixedScalars_Construct) {
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{
                       Member("m1", ty.i32()),
                       Member("m2", ty.u32()),
                       Member("m3", ty.f32()),
                       Member("m4", ty.f16()),
                       Member("m5", ty.bool_()),
                   });
    auto* expr = Call("S", 1_i, 2_u, 3_f, 4_h, false);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<core::type::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<i32>(), 1_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<core::type::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<u32>(), 2_u);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<core::type::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<f32>(), 3._f);

    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<core::type::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->ValueAs<f16>(), 4._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<core::type::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->ValueAs<bool>(), false);
}

TEST_F(ConstEvalTest, Struct_MixedVectors_Construct) {
    Enable(wgsl::Extension::kF16);

    Structure("S", Vector{
                       Member("m1", ty.vec2<i32>()),
                       Member("m2", ty.vec3<u32>()),
                       Member("m3", ty.vec4<f32>()),
                       Member("m4", ty.vec3<f16>()),
                       Member("m5", ty.vec2<bool>()),
                   });
    auto* expr = Call("S", Call<vec2<i32>>(1_i), Call<vec3<u32>>(2_u), Call<vec4<f32>>(3_f),
                      Call<vec3<f16>>(4_h), Call<vec2<bool>>(false));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(0)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<i32>(), 1_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<i32>(), 1_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(1)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<u32>(), 2_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<u32>(), 2_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->ValueAs<u32>(), 2_u);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(2)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->ValueAs<f32>(), 3._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->ValueAs<f32>(), 3._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(2)->ValueAs<f32>(), 3._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(3)->ValueAs<f32>(), 3._f);

    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(3)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(0)->ValueAs<f16>(), 4._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(1)->ValueAs<f16>(), 4._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(2)->ValueAs<f16>(), 4._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(sem->ConstantValue()
                    ->Index(4)
                    ->Type()
                    ->As<core::type::Vector>()
                    ->Type()
                    ->Is<core::type::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(0)->ValueAs<bool>(), false);
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(1)->ValueAs<bool>(), false);
}

TEST_F(ConstEvalTest, Struct_Struct_Construct) {
    Structure("Inner", Vector{
                           Member("m1", ty.i32()),
                           Member("m2", ty.u32()),
                           Member("m3", ty.f32()),
                       });

    Structure("Outer", Vector{
                           Member("m1", ty("Inner")),
                           Member("m2", ty("Inner")),
                       });
    auto* expr = Call("Outer",  //
                      Call("Inner", 1_i, 2_u, 3_f), Call("Inner", 4_i, 0_u, 6_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 2u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<core::type::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<i32>(), 1_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<u32>(), 2_u);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->ValueAs<f32>(), 3_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<core::type::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<i32>(), 4_i);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->ValueAs<f32>(), 6_f);
}

TEST_F(ConstEvalTest, Struct_Array_Construct) {
    Structure("S", Vector{
                       Member("m1", ty.array<i32, 2>()),
                       Member("m2", ty.array<f32, 3>()),
                   });
    auto* expr = Call("S",  //
                      Call<array<i32, 2>>(1_i, 2_i), Call<array<f32, 3>>(1_f, 2_f, 3_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<core::type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 2u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<core::type::Array>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->ValueAs<i32>(), 1_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->ValueAs<u32>(), 2_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<core::type::Array>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->ValueAs<i32>(), 1_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->ValueAs<u32>(), 2_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->ValueAs<f32>(), 3_f);
}

}  // namespace
}  // namespace tint::core::constant::test
