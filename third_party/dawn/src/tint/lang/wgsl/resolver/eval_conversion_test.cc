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

#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/wgsl/resolver/eval_test.h"
#include "src/tint/lang/wgsl/sem/materialize.h"

namespace tint::core::constant::test {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

enum class Kind {
    kScalar,
    kVector,
};

static std::ostream& operator<<(std::ostream& o, const Kind& k) {
    switch (k) {
        case Kind::kScalar:
            return o << "scalar";
        case Kind::kVector:
            return o << "vector";
    }
    return o << "<unknown>";
}

struct Case {
    Value input;
    Value expected;
    builder::CreatePtrs type;
    bool unrepresentable = false;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    if (c.unrepresentable) {
        o << "[unrepresentable] input: " << c.input;
    } else {
        o << "input: " << c.input << ", expected: " << c.expected;
    }
    return o << ", type: " << c.type;
}

template <typename TO, typename FROM>
Case Success(FROM input, TO expected) {
    return {Val(input), Val(expected), builder::CreatePtrsFor<TO>()};
}

template <typename TO, typename FROM>
Case Unrepresentable(FROM input) {
    return {builder::Val(input), builder::Val(0_i), builder::CreatePtrsFor<TO>(),
            /* unrepresentable */ true};
}

using ConstEvalConvTest = ConstEvalTestWithParam<std::tuple<Kind, Case>>;

TEST_P(ConstEvalConvTest, Test) {
    const auto& kind = std::get<0>(GetParam());
    const auto& input = std::get<1>(GetParam()).input;
    const auto& expected = std::get<1>(GetParam()).expected;
    const auto& type = std::get<1>(GetParam()).type;
    const auto unrepresentable = std::get<1>(GetParam()).unrepresentable;

    auto* input_val = input.Expr(*this);
    auto* expr = Call(type.ast(*this), input_val);
    if (kind == Kind::kVector) {
        expr = Call<vec3<Infer>>(expr);
    }
    WrapInFunction(expr);

    auto* target_sem_ty = type.sem(*this);
    if (kind == Kind::kVector) {
        target_sem_ty = create<core::type::Vector>(target_sem_ty, 3u);
    }

    if (unrepresentable) {
        ASSERT_FALSE(r()->Resolve());
        EXPECT_THAT(r()->error(), testing::HasSubstr("cannot be represented as"));
    } else {
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        auto* sem = Sem().Get(expr);
        ASSERT_NE(sem, nullptr);
        EXPECT_TYPE(sem->Type(), target_sem_ty);
        ASSERT_NE(sem->ConstantValue(), nullptr);
        EXPECT_TYPE(sem->ConstantValue()->Type(), target_sem_ty);

        auto expected_values = expected.args;
        if (kind == Kind::kVector) {
            expected_values.Push(expected_values[0]);
            expected_values.Push(expected_values[0]);
        }
        auto got_values = ScalarsFrom(sem->ConstantValue());
        EXPECT_EQ(expected_values, got_values);
    }
}
INSTANTIATE_TEST_SUITE_P(
    ScalarAndVector,
    ConstEvalConvTest,
    testing::Combine(testing::Values(Kind::kScalar, Kind::kVector),
                     testing::ValuesIn({
                         // TODO(crbug.com/tint/1502): Add f16 tests
                         // i32 -> u32
                         Success(0_i, 0_u),
                         Success(1_i, 1_u),
                         Success(-1_i, 0xffffffff_u),
                         Success(2_i, 2_u),
                         Success(-2_i, 0xfffffffe_u),
                         // i32 -> f32
                         Success(0_i, 0_f),
                         Success(1_i, 1_f),
                         Success(-1_i, -1_f),
                         Success(2_i, 2_f),
                         Success(-2_i, -2_f),
                         // i32 -> bool
                         Success(0_i, false),
                         Success(1_i, true),
                         Success(-1_i, true),
                         Success(2_i, true),
                         Success(-2_i, true),
                         // u32 -> i32
                         Success(0_u, 0_i),
                         Success(1_u, 1_i),
                         Success(0xffffffff_u, -1_i),
                         Success(2_u, 2_i),
                         Success(0xfffffffe_u, -2_i),
                         // u32 -> f32
                         Success(0_u, 0_f),
                         Success(1_u, 1_f),
                         Success(2_u, 2_f),
                         Success(0xffffffff_u, 0xffffffff_f),
                         // u32 -> bool
                         Success(0_u, false),
                         Success(1_u, true),
                         Success(2_u, true),
                         Success(0xffffffff_u, true),
                         // f32 -> i32
                         Success(0_f, 0_i),
                         Success(1_f, 1_i),
                         Success(1.9_f, 1_i),
                         Success(-1.9_f, -1_i),
                         Success(2_f, 2_i),
                         Success(1e20_f, i32(tint::core::kMaxI32WhichIsAlsoF32)),
                         Success(-1e20_f, i32::Lowest()),
                         // f32 -> u32
                         Success(0_f, 0_i),
                         Success(1_f, 1_i),
                         Success(1.9_f, 1_i),
                         Success(-1_f, u32::Lowest()),
                         Success(2_f, 2_i),
                         Success(1e20_f, u32(tint::core::kMaxU32WhichIsAlsoF32)),
                         Success(-1e20_f, u32::Lowest()),
                         // f32 -> bool
                         Success(0_f, false),
                         Success(0.1_f, true),
                         Success(0.9_f, true),
                         Success(1_f, true),
                         Success(-1_f, true),
                         Success(2_f, true),
                         Success(1e20_f, true),
                         Success(-1e20_f, true),
                         // abstract-int -> i32
                         Success(0_a, 0_i),
                         Success(1_a, 1_i),
                         Success(-1_a, -1_i),
                         Success(0x7fffffff_a, i32::Highest()),
                         Success(-0x80000000_a, i32::Lowest()),
                         Unrepresentable<i32>(0x80000000_a),
                         // abstract-int -> u32
                         Success(0_a, 0_u),
                         Success(1_a, 1_u),
                         Success(0xffffffff_a, 0xffffffff_u),
                         Unrepresentable<u32>(0x100000000_a),
                         Unrepresentable<u32>(-1_a),
                         // abstract-int -> f32
                         Success(0_a, 0_f),
                         Success(1_a, 1_f),
                         Success(0xffffffff_a, 0xffffffff_f),
                         Success(0x100000000_a, 0x100000000_f),
                         Success(-0x100000000_a, -0x100000000_f),
                         Success(0x7fffffffffffffff_a, 0x7fffffffffffffff_f),
                         Success(-0x7fffffffffffffff_a, -0x7fffffffffffffff_f),
                         // abstract-int -> bool
                         Success(0_a, false),
                         Success(1_a, true),
                         Success(0xffffffff_a, true),
                         Success(0x100000000_a, true),
                         Success(-0x100000000_a, true),
                         Success(0x7fffffffffffffff_a, true),
                         Success(-0x7fffffffffffffff_a, true),
                         // abstract-float -> i32
                         Success(0.0_a, 0_i),
                         Success(1.0_a, 1_i),
                         Success(1.9_a, 1_i),
                         Success(-1.0_a, -1_i),
                         Success(-1.9_a, -1_i),
                         Success(AFloat(0x7fffffff), i32::Highest()),
                         Success(-AFloat(0x80000000), i32::Lowest()),
                         Success(AFloat(0x80000000), i32::Highest()),  // overflow
                         Success(-AFloat(0x80000001), i32::Lowest()),  // underflow
                         // abstract-float -> u32
                         Success(0.0_a, 0_u),
                         Success(1.0_a, 1_u),
                         Success(1.9_a, 1_u),
                         Success(AFloat(0xffffffff), 0xffffffff_u),
                         Success(AFloat(0x100000000), u32::Highest()),  // overflow
                         Success(AFloat(-1), u32::Lowest()),            // underflow
                         // abstract-float -> f32
                         Success(0.0_a, 0_f),
                         Success(1.0_a, 1_f),
                         Success(AFloat(0xffffffff), 0xffffffff_f),
                         Success(AFloat(0x100000000), 0x100000000_f),
                         Success(-AFloat(0x100000000), -0x100000000_f),
                         Unrepresentable<f32>(1e40_a),
                         Unrepresentable<f32>(-1e40_a),
                         // abstract-float -> bool
                         Success(0.0_a, false),
                         Success(0.1_a, true),
                         Success(1.0_a, true),
                         Success(AFloat(0xffffffff), true),
                         Success(AFloat(0x100000000), true),
                         Success(-AFloat(0x100000000), true),
                         Success(1e40_a, true),
                         Success(-1e40_a, true),
                     })));

TEST_F(ConstEvalTest, Vec3_Convert_f32_to_i32) {
    auto* expr = Call<vec3<i32>>(Call<vec3<f32>>(1.1_f, 2.2_f, 3.3_f));
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

TEST_F(ConstEvalTest, Vec3_Convert_u32_to_f32) {
    auto* expr = Call<vec3<f32>>(Call<vec3<u32>>(10_u, 20_u, 30_u));
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
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 10.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 20.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 30.f);
}

TEST_F(ConstEvalTest, Vec3_Convert_f16_to_i32) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<i32>>(Call<vec3<f16>>(1.1_h, 2.2_h, 3.3_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(), 1_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), 2_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(), 3_i);
}

TEST_F(ConstEvalTest, Vec3_Convert_u32_to_f16) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<f16>>(Call<vec3<u32>>(10_u, 20_u, 30_u));
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
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 10.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), 20.f);

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 30.f);
}

TEST_F(ConstEvalTest, Vec3_Convert_Large_f32_to_i32) {
    auto* expr = Call<vec3<i32>>(Call<vec3<f32>>(1e10_f, -1e20_f, 1e30_f));
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
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(),
              i32(tint::core::kMaxI32WhichIsAlsoF32));

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), i32::Lowest());

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(),
              i32(tint::core::kMaxI32WhichIsAlsoF32));
}

TEST_F(ConstEvalTest, Vec3_Convert_Large_f32_to_u32) {
    auto* expr = Call<vec3<u32>>(Call<vec3<f32>>(1e10_f, -1e20_f, 1e30_f));
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
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AInt>(),
              u32(tint::core::kMaxU32WhichIsAlsoF32));

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AInt>(), u32::Lowest());

    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AInt>(),
              u32(tint::core::kMaxU32WhichIsAlsoF32));
}

TEST_F(ConstEvalTest, Vec3_Convert_Large_f32_to_f16) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<vec3<f16>>(Source{{12, 34}}, Call<vec3<f32>>(1e10_f, 0_f, 0_f));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: value 10000000000.0 cannot be represented as 'f16'");
}

TEST_F(ConstEvalTest, Vec3_Convert_Small_f32_to_f16) {
    Enable(wgsl::Extension::kF16);

    // These values are chosen to force underflow when converting to f16.
    // When the result is zero, the sign bit is *not* guaranteed to be preserved.
    // - IEEE 754 does not require it.
    // - C++14 does not require it.
    auto* expr = Call<vec3<f16>>(Call<vec3<f32>>(1e-20_f, -2e-30_f, 3e-40_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<core::type::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->Type()->Is<core::type::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->ValueAs<AFloat>(), 0.0);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->ValueAs<AFloat>(), -0.0);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->ValueAs<AFloat>(), 0.0);
}

TEST_F(ConstEvalTest, StructAbstractSplat_to_StructDifferentTypes) {
    // fn f() {
    //   const c = modf(4.0);
    //   var v = c;
    // }
    auto* expr_c = Call(core::BuiltinFn::kModf, 0_a);
    auto* materialized = Expr("c");
    WrapInFunction(Decl(Const("c", expr_c)), Decl(Var("v", materialized)));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* c = Sem().Get(expr_c);
    ASSERT_NE(c, nullptr);
    EXPECT_TRUE(c->ConstantValue()->Is<constant::Splat>());
    EXPECT_TRUE(c->ConstantValue()->AnyZero());
    EXPECT_TRUE(c->ConstantValue()->AllZero());

    EXPECT_TRUE(c->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(c->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(c->ConstantValue()->Index(0)->Type()->Is<core::type::AbstractFloat>());
    EXPECT_EQ(c->ConstantValue()->Index(0)->ValueAs<AFloat>(), 0_f);

    EXPECT_TRUE(c->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(c->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(c->ConstantValue()->Index(1)->Type()->Is<core::type::AbstractFloat>());
    EXPECT_EQ(c->ConstantValue()->Index(1)->ValueAs<AFloat>(), 0_a);

    auto* v = Sem().GetVal(materialized);
    ASSERT_NE(v, nullptr);
    EXPECT_TRUE(v->Is<sem::Materialize>());
    EXPECT_TRUE(v->ConstantValue()->Is<constant::Splat>());
    EXPECT_TRUE(v->ConstantValue()->AnyZero());
    EXPECT_TRUE(v->ConstantValue()->AllZero());

    EXPECT_TRUE(v->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(v->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(v->ConstantValue()->Index(0)->Type()->Is<core::type::F32>());
    EXPECT_EQ(v->ConstantValue()->Index(0)->ValueAs<f32>(), 0_f);

    EXPECT_TRUE(v->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(v->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(v->ConstantValue()->Index(1)->Type()->Is<core::type::F32>());
    EXPECT_EQ(v->ConstantValue()->Index(1)->ValueAs<f32>(), 0_f);
}

}  // namespace
}  // namespace tint::core::constant::test
