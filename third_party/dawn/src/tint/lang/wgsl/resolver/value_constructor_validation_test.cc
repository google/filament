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
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/array.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ::testing::HasSubstr;

// Helpers and typedefs
using builder::alias;
using builder::alias1;
using builder::alias2;
using builder::alias3;
using builder::CreatePtrs;
using builder::CreatePtrsFor;
using builder::DataType;

class ResolverValueConstructorValidationTest : public resolver::TestHelper, public testing::Test {};

namespace InferTypeTest {
struct Params {
    builder::ast_type_func_ptr create_rhs_ast_type;
    builder::ast_expr_from_double_func_ptr create_rhs_ast_value;
    builder::sem_type_func_ptr create_rhs_sem_type;
};

template <typename T>
constexpr Params ParamsFor() {
    return Params{DataType<T>::AST, DataType<T>::ExprFromDouble, DataType<T>::Sem};
}

TEST_F(ResolverValueConstructorValidationTest, InferTypeTest_Simple) {
    // var a = 1i;
    // var b = a;
    auto* a = Var("a", Expr(1_i));
    auto* b = Var("b", Expr("a"));
    auto* a_ident = Expr("a");
    auto* b_ident = Expr("b");

    WrapInFunction(a, b, Assign(a_ident, "a"), Assign(b_ident, "b"));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_TRUE(TypeOf(a_ident)->Is<core::type::Reference>());
    EXPECT_TRUE(TypeOf(a_ident)->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
    EXPECT_EQ(TypeOf(a_ident)->As<core::type::Reference>()->AddressSpace(),
              core::AddressSpace::kFunction);
    ASSERT_TRUE(TypeOf(b_ident)->Is<core::type::Reference>());
    EXPECT_TRUE(TypeOf(b_ident)->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
    EXPECT_EQ(TypeOf(b_ident)->As<core::type::Reference>()->AddressSpace(),
              core::AddressSpace::kFunction);
}

using InferTypeTest_FromConstructorExpression = ResolverTestWithParam<Params>;
TEST_P(InferTypeTest_FromConstructorExpression, All) {
    // e.g. for vec3<f32>
    // {
    //   var a = vec3<f32>(0.0, 0.0, 0.0)
    // }
    auto& params = GetParam();

    Enable(wgsl::Extension::kF16);

    auto* initializer_expr = params.create_rhs_ast_value(*this, 0);

    auto* a = Var("a", initializer_expr);
    // Self-assign 'a' to force the expression to be resolved so we can test its
    // type below
    auto* a_ident = Expr("a");
    WrapInFunction(Decl(a), Assign(a_ident, "a"));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* got = TypeOf(a_ident);
    auto* expected = create<core::type::Reference>(
        core::AddressSpace::kFunction, params.create_rhs_sem_type(*this), core::Access::kReadWrite);
    ASSERT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                             << "expected: " << FriendlyName(expected) << "\n";
}

static constexpr Params from_constructor_expression_cases[] = {
    ParamsFor<bool>(),
    ParamsFor<i32>(),
    ParamsFor<u32>(),
    ParamsFor<f32>(),
    ParamsFor<f16>(),
    ParamsFor<vec3<i32>>(),
    ParamsFor<vec3<u32>>(),
    ParamsFor<vec3<f32>>(),
    ParamsFor<vec3<f16>>(),
    ParamsFor<mat3x3<f32>>(),
    ParamsFor<mat3x3<f16>>(),
    ParamsFor<alias<bool>>(),
    ParamsFor<alias<i32>>(),
    ParamsFor<alias<u32>>(),
    ParamsFor<alias<f32>>(),
    ParamsFor<alias<f16>>(),
    ParamsFor<alias<vec3<i32>>>(),
    ParamsFor<alias<vec3<u32>>>(),
    ParamsFor<alias<vec3<f32>>>(),
    ParamsFor<alias<vec3<f16>>>(),
    ParamsFor<alias<mat3x3<f32>>>(),
    ParamsFor<alias<mat3x3<f16>>>(),
};
INSTANTIATE_TEST_SUITE_P(ResolverValueConstructorValidationTest,
                         InferTypeTest_FromConstructorExpression,
                         testing::ValuesIn(from_constructor_expression_cases));

using InferTypeTest_FromArithmeticExpression = ResolverTestWithParam<Params>;
TEST_P(InferTypeTest_FromArithmeticExpression, All) {
    // e.g. for vec3<f32>
    // {
    //   var a = vec3<f32>(2.0, 2.0, 2.0) * 3.0;
    // }
    auto& params = GetParam();

    auto* arith_lhs_expr = params.create_rhs_ast_value(*this, 2);
    auto* arith_rhs_expr = params.create_rhs_ast_value(*this, 3);
    auto* initializer_expr = Mul(arith_lhs_expr, arith_rhs_expr);

    auto* a = Var("a", initializer_expr);
    // Self-assign 'a' to force the expression to be resolved so we can test its
    // type below
    auto* a_ident = Expr("a");
    WrapInFunction(Decl(a), Assign(a_ident, "a"));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* got = TypeOf(a_ident);
    auto* expected = create<core::type::Reference>(
        core::AddressSpace::kFunction, params.create_rhs_sem_type(*this), core::Access::kReadWrite);
    ASSERT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                             << "expected: " << FriendlyName(expected) << "\n";
}
static constexpr Params from_arithmetic_expression_cases[] = {
    ParamsFor<i32>(),
    ParamsFor<u32>(),
    ParamsFor<f32>(),
    ParamsFor<vec3<f32>>(),
    ParamsFor<mat3x3<f32>>(),
    ParamsFor<alias<i32>>(),
    ParamsFor<alias<u32>>(),
    ParamsFor<alias<f32>>(),
    ParamsFor<alias<vec3<f32>>>(),
    ParamsFor<alias<mat3x3<f32>>>(),
};
INSTANTIATE_TEST_SUITE_P(ResolverValueConstructorValidationTest,
                         InferTypeTest_FromArithmeticExpression,
                         testing::ValuesIn(from_arithmetic_expression_cases));

using InferTypeTest_FromCallExpression = ResolverTestWithParam<Params>;
TEST_P(InferTypeTest_FromCallExpression, All) {
    // e.g. for vec3<f32>
    //
    // fn foo() -> vec3<f32> {
    //   return vec3<f32>();
    // }
    //
    // fn bar()
    // {
    //   var a = foo();
    // }
    auto& params = GetParam();

    Enable(wgsl::Extension::kF16);

    Func("foo", tint::Empty, params.create_rhs_ast_type(*this),
         Vector{Return(Call(params.create_rhs_ast_type(*this)))}, {});

    auto* a = Var("a", Call("foo"));
    // Self-assign 'a' to force the expression to be resolved so we can test its
    // type below
    auto* a_ident = Expr("a");
    WrapInFunction(Decl(a), Assign(a_ident, "a"));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* got = TypeOf(a_ident);
    auto* expected = create<core::type::Reference>(
        core::AddressSpace::kFunction, params.create_rhs_sem_type(*this), core::Access::kReadWrite);
    ASSERT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                             << "expected: " << FriendlyName(expected) << "\n";
}
static constexpr Params from_call_expression_cases[] = {
    ParamsFor<bool>(),
    ParamsFor<i32>(),
    ParamsFor<u32>(),
    ParamsFor<f32>(),
    ParamsFor<f16>(),
    ParamsFor<vec3<i32>>(),
    ParamsFor<vec3<u32>>(),
    ParamsFor<vec3<f32>>(),
    ParamsFor<vec3<f16>>(),
    ParamsFor<mat3x3<f32>>(),
    ParamsFor<mat3x3<f16>>(),
    ParamsFor<alias<bool>>(),
    ParamsFor<alias<i32>>(),
    ParamsFor<alias<u32>>(),
    ParamsFor<alias<f32>>(),
    ParamsFor<alias<f16>>(),
    ParamsFor<alias<vec3<i32>>>(),
    ParamsFor<alias<vec3<u32>>>(),
    ParamsFor<alias<vec3<f32>>>(),
    ParamsFor<alias<vec3<f16>>>(),
    ParamsFor<alias<mat3x3<f32>>>(),
    ParamsFor<alias<mat3x3<f16>>>(),
};
INSTANTIATE_TEST_SUITE_P(ResolverValueConstructorValidationTest,
                         InferTypeTest_FromCallExpression,
                         testing::ValuesIn(from_call_expression_cases));

}  // namespace InferTypeTest

namespace ConversionConstructTest {
enum class Kind {
    Construct,
    Conversion,
};

struct Params {
    Kind kind;
    builder::ast_type_func_ptr lhs_type;
    builder::ast_type_func_ptr rhs_type;
    builder::ast_expr_from_double_func_ptr rhs_value_expr;
};

template <typename LhsType, typename RhsType>
constexpr Params ParamsFor(Kind kind) {
    return Params{kind, DataType<LhsType>::AST, DataType<RhsType>::AST,
                  DataType<RhsType>::ExprFromDouble};
}

static constexpr Params valid_cases[] = {
    // Identity
    ParamsFor<bool, bool>(Kind::Construct),                //
    ParamsFor<i32, i32>(Kind::Construct),                  //
    ParamsFor<u32, u32>(Kind::Construct),                  //
    ParamsFor<f32, f32>(Kind::Construct),                  //
    ParamsFor<f16, f16>(Kind::Construct),                  //
    ParamsFor<vec3<bool>, vec3<bool>>(Kind::Construct),    //
    ParamsFor<vec3<i32>, vec3<i32>>(Kind::Construct),      //
    ParamsFor<vec3<u32>, vec3<u32>>(Kind::Construct),      //
    ParamsFor<vec3<f32>, vec3<f32>>(Kind::Construct),      //
    ParamsFor<vec3<f16>, vec3<f16>>(Kind::Construct),      //
    ParamsFor<mat3x3<f32>, mat3x3<f32>>(Kind::Construct),  //
    ParamsFor<mat2x3<f32>, mat2x3<f32>>(Kind::Construct),  //
    ParamsFor<mat3x2<f32>, mat3x2<f32>>(Kind::Construct),  //
    ParamsFor<mat3x3<f16>, mat3x3<f16>>(Kind::Construct),  //
    ParamsFor<mat2x3<f16>, mat2x3<f16>>(Kind::Construct),  //
    ParamsFor<mat3x2<f16>, mat3x2<f16>>(Kind::Construct),  //

    // Splat
    ParamsFor<vec3<bool>, bool>(Kind::Construct),  //
    ParamsFor<vec3<i32>, i32>(Kind::Construct),    //
    ParamsFor<vec3<u32>, u32>(Kind::Construct),    //
    ParamsFor<vec3<f32>, f32>(Kind::Construct),    //
    ParamsFor<vec3<f16>, f16>(Kind::Construct),    //

    // Conversion
    ParamsFor<bool, u32>(Kind::Conversion),  //
    ParamsFor<bool, i32>(Kind::Conversion),  //
    ParamsFor<bool, f32>(Kind::Conversion),  //
    ParamsFor<bool, f16>(Kind::Conversion),  //

    ParamsFor<i32, bool>(Kind::Conversion),  //
    ParamsFor<i32, u32>(Kind::Conversion),   //
    ParamsFor<i32, f32>(Kind::Conversion),   //
    ParamsFor<i32, f16>(Kind::Conversion),   //

    ParamsFor<u32, bool>(Kind::Conversion),  //
    ParamsFor<u32, i32>(Kind::Conversion),   //
    ParamsFor<u32, f32>(Kind::Conversion),   //
    ParamsFor<u32, f16>(Kind::Conversion),   //

    ParamsFor<f32, bool>(Kind::Conversion),  //
    ParamsFor<f32, u32>(Kind::Conversion),   //
    ParamsFor<f32, i32>(Kind::Conversion),   //
    ParamsFor<f32, f16>(Kind::Conversion),   //

    ParamsFor<f16, bool>(Kind::Conversion),  //
    ParamsFor<f16, u32>(Kind::Conversion),   //
    ParamsFor<f16, i32>(Kind::Conversion),   //
    ParamsFor<f16, f32>(Kind::Conversion),   //

    ParamsFor<vec3<bool>, vec3<u32>>(Kind::Conversion),  //
    ParamsFor<vec3<bool>, vec3<i32>>(Kind::Conversion),  //
    ParamsFor<vec3<bool>, vec3<f32>>(Kind::Conversion),  //
    ParamsFor<vec3<bool>, vec3<f16>>(Kind::Conversion),  //

    ParamsFor<vec3<i32>, vec3<bool>>(Kind::Conversion),  //
    ParamsFor<vec3<i32>, vec3<u32>>(Kind::Conversion),   //
    ParamsFor<vec3<i32>, vec3<f32>>(Kind::Conversion),   //
    ParamsFor<vec3<i32>, vec3<f16>>(Kind::Conversion),   //

    ParamsFor<vec3<u32>, vec3<bool>>(Kind::Conversion),  //
    ParamsFor<vec3<u32>, vec3<i32>>(Kind::Conversion),   //
    ParamsFor<vec3<u32>, vec3<f32>>(Kind::Conversion),   //
    ParamsFor<vec3<u32>, vec3<f16>>(Kind::Conversion),   //

    ParamsFor<vec3<f32>, vec3<bool>>(Kind::Conversion),  //
    ParamsFor<vec3<f32>, vec3<u32>>(Kind::Conversion),   //
    ParamsFor<vec3<f32>, vec3<i32>>(Kind::Conversion),   //
    ParamsFor<vec3<f32>, vec3<f16>>(Kind::Conversion),   //

    ParamsFor<vec3<f16>, vec3<bool>>(Kind::Conversion),  //
    ParamsFor<vec3<f16>, vec3<u32>>(Kind::Conversion),   //
    ParamsFor<vec3<f16>, vec3<i32>>(Kind::Conversion),   //
    ParamsFor<vec3<f16>, vec3<f32>>(Kind::Conversion),   //

    ParamsFor<mat3x3<f16>, mat3x3<f32>>(Kind::Conversion),  //
    ParamsFor<mat2x3<f16>, mat2x3<f32>>(Kind::Conversion),  //
    ParamsFor<mat3x2<f16>, mat3x2<f32>>(Kind::Conversion),  //

    ParamsFor<mat3x3<f32>, mat3x3<f16>>(Kind::Conversion),  //
    ParamsFor<mat2x3<f32>, mat2x3<f16>>(Kind::Conversion),  //
    ParamsFor<mat3x2<f32>, mat3x2<f16>>(Kind::Conversion),  //
};

using ConversionConstructorValidTest = ResolverTestWithParam<Params>;
TEST_P(ConversionConstructorValidTest, All) {
    auto& params = GetParam();

    Enable(wgsl::Extension::kF16);

    // var a : <lhs_type1> = <lhs_type2>(<rhs_type>(<rhs_value_expr>));
    auto lhs_type1 = params.lhs_type(*this);
    auto lhs_type2 = params.lhs_type(*this);
    auto rhs_type = params.rhs_type(*this);
    auto* rhs_value_expr = params.rhs_value_expr(*this, 0);

    StringStream ss;
    ss << FriendlyName(lhs_type1) << " = " << FriendlyName(lhs_type2) << "("
       << FriendlyName(rhs_type) << "(<rhs value expr>))";
    SCOPED_TRACE(ss.str());

    auto* arg = Call(rhs_type, rhs_value_expr);
    auto* tc = Call(lhs_type2, arg);
    auto* a = Var("a", lhs_type1, tc);

    // Self-assign 'a' to force the expression to be resolved so we can test its
    // type below
    auto* a_ident = Expr("a");
    WrapInFunction(Decl(a), Assign(a_ident, "a"));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    switch (params.kind) {
        case Kind::Construct: {
            auto* ctor = call->Target()->As<sem::ValueConstructor>();
            ASSERT_NE(ctor, nullptr);
            EXPECT_EQ(call->Type(), ctor->ReturnType());
            ASSERT_EQ(ctor->Parameters().Length(), 1u);
            EXPECT_EQ(ctor->Parameters()[0]->Type(), TypeOf(arg));
            break;
        }
        case Kind::Conversion: {
            auto* conv = call->Target()->As<sem::ValueConversion>();
            ASSERT_NE(conv, nullptr);
            EXPECT_EQ(call->Type(), conv->ReturnType());
            ASSERT_EQ(conv->Parameters().Length(), 1u);
            EXPECT_EQ(conv->Parameters()[0]->Type(), TypeOf(arg));
            break;
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverValueConstructorValidationTest,
                         ConversionConstructorValidTest,
                         testing::ValuesIn(valid_cases));

constexpr CreatePtrs all_types[] = {
    CreatePtrsFor<bool>(),         //
    CreatePtrsFor<u32>(),          //
    CreatePtrsFor<i32>(),          //
    CreatePtrsFor<f32>(),          //
    CreatePtrsFor<f16>(),          //
    CreatePtrsFor<vec3<bool>>(),   //
    CreatePtrsFor<vec3<i32>>(),    //
    CreatePtrsFor<vec3<u32>>(),    //
    CreatePtrsFor<vec3<f32>>(),    //
    CreatePtrsFor<vec3<f16>>(),    //
    CreatePtrsFor<mat3x3<i32>>(),  //
    CreatePtrsFor<mat3x3<u32>>(),  //
    CreatePtrsFor<mat3x3<f32>>(),  //
    CreatePtrsFor<mat3x3<f16>>(),  //
    CreatePtrsFor<mat2x3<i32>>(),  //
    CreatePtrsFor<mat2x3<u32>>(),  //
    CreatePtrsFor<mat2x3<f32>>(),  //
    CreatePtrsFor<mat2x3<f16>>(),  //
    CreatePtrsFor<mat3x2<i32>>(),  //
    CreatePtrsFor<mat3x2<u32>>(),  //
    CreatePtrsFor<mat3x2<f32>>(),  //
    CreatePtrsFor<mat3x2<f16>>(),  //
};

using ConversionConstructorInvalidTest = ResolverTestWithParam<std::tuple<CreatePtrs,  // lhs
                                                                          CreatePtrs   // rhs
                                                                          >>;
TEST_P(ConversionConstructorInvalidTest, All) {
    auto& params = GetParam();

    auto& lhs_params = std::get<0>(params);
    auto& rhs_params = std::get<1>(params);

    // Skip test for valid cases
    for (auto& v : valid_cases) {
        if (v.lhs_type == lhs_params.ast && v.rhs_type == rhs_params.ast &&
            v.rhs_value_expr == rhs_params.expr_from_double) {
            return;
        }
    }
    // Skip non-conversions
    if (lhs_params.ast == rhs_params.ast) {
        return;
    }

    // var a : <lhs_type1> = <lhs_type2>(<rhs_type>(<rhs_value_expr>));
    auto lhs_type1 = lhs_params.ast(*this);
    auto lhs_type2 = lhs_params.ast(*this);
    auto rhs_type = rhs_params.ast(*this);
    auto* rhs_value_expr = rhs_params.expr_from_double(*this, 0);

    StringStream ss;
    ss << FriendlyName(lhs_type1) << " = " << FriendlyName(lhs_type2) << "("
       << FriendlyName(rhs_type) << "(<rhs value expr>))";
    SCOPED_TRACE(ss.str());

    Enable(wgsl::Extension::kF16);

    auto* a = Var("a", lhs_type1, Call(lhs_type2, Call(rhs_type, rhs_value_expr)));

    // Self-assign 'a' to force the expression to be resolved so we can test its
    // type below
    auto* a_ident = Expr("a");
    WrapInFunction(Decl(a), Assign(a_ident, "a"));

    ASSERT_FALSE(r()->Resolve());
}
INSTANTIATE_TEST_SUITE_P(ResolverValueConstructorValidationTest,
                         ConversionConstructorInvalidTest,
                         testing::Combine(testing::ValuesIn(all_types),
                                          testing::ValuesIn(all_types)));

TEST_F(ResolverValueConstructorValidationTest, ConversionConstructorInvalid_TooManyConstructors) {
    auto* a = Var("a", ty.f32(), Call(Source{{12, 34}}, ty.f32(), Expr(1_f), Expr(2_f)));
    WrapInFunction(a);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'f32(f32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, ConversionConstructorInvalid_InvalidConstructor) {
    auto* a = Var("a", ty.f32(), Call(Source{{12, 34}}, ty.f32(), Call<array<f32, 4>>()));
    WrapInFunction(a);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'f32(array<f32, 4>)"));
}

TEST_F(ResolverValueConstructorValidationTest, ConversionConstructorInvalid_ConstructI8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    auto* a = Var("a", ty.i8(), Call(Source{{12, 34}}, ty.i8()));
    WrapInFunction(a);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: type is not constructible"));
}

TEST_F(ResolverValueConstructorValidationTest, ConversionConstructorInvalid_ConstructU8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    auto* a = Var("a", ty.u8(), Call(Source{{12, 34}}, ty.u8()));
    WrapInFunction(a);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: type is not constructible"));
}

TEST_F(ResolverValueConstructorValidationTest,
       ConversionConstructorInvalid_ConstructI8WithoutExtension) {
    auto* a = Var("a", ty.i8(), Call(Source{{12, 34}}, ty.i8()));
    WrapInFunction(a);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("error: 'i8' type used without 'chromium_experimental_subgroup_matrix' "
                          "extension enabled"));
}

TEST_F(ResolverValueConstructorValidationTest,
       ConversionConstructorInvalid_ConstructU8WithoutExtension) {
    auto* a = Var("a", ty.u8(), Call(Source{{12, 34}}, ty.u8()));
    WrapInFunction(a);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("error: 'u8' type used without 'chromium_experimental_subgroup_matrix' "
                          "extension enabled"));
}

}  // namespace ConversionConstructTest

namespace ArrayConstructor {

TEST_F(ResolverValueConstructorValidationTest, Array_ZeroValue_Pass) {
    // array<u32, 10u>();
    auto* tc = Call<array<u32, 10>>();
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 0u);
}

TEST_F(ResolverValueConstructorValidationTest, Array_U32U32U32) {
    // array<u32, 3u>(0u, 10u, 20u);
    auto* tc = Call<array<u32, 3>>(0_u, 10_u, 20_u);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::U32>());
}

TEST_F(ResolverValueConstructorValidationTest, InferredArray_U32U32U32) {
    // array(0u, 10u, 20u);
    auto* tc = Call<array<Infer>>(Source{{12, 34}}, 0_u, 10_u, 20_u);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::U32>());
}

TEST_F(ResolverValueConstructorValidationTest, Array_U32AIU32) {
    // array<u32, 3u>(0u, 10, 20u);
    auto* tc = Call<array<u32, 3>>(0_u, 10_a, 20_u);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::U32>());
}

TEST_F(ResolverValueConstructorValidationTest, InferredArray_U32AIU32) {
    // array(0u, 10u, 20u);
    auto* tc = Call<array<Infer>>(Source{{12, 34}}, 0_u, 10_a, 20_u);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::U32>());
}

TEST_F(ResolverValueConstructorValidationTest, ArrayU32_AIAIAI) {
    // array<u32, 3u>(0, 10, 20);
    auto* tc = Call<array<u32, 3>>(0_a, 10_a, 20_a);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::U32>());
}

TEST_F(ResolverValueConstructorValidationTest, InferredArray_AIAIAI) {
    // const c = array(0, 10, 20);
    auto* tc = Call<array<Infer>>(Source{{12, 34}}, 0_a, 10_a, 20_a);
    WrapInFunction(Decl(Const("C", tc)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::AbstractInt>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::AbstractInt>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::AbstractInt>());
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayU32_VecI32_VecAI) {
    // array(vec2(10i), vec2(20));
    auto* tc = Call<array<Infer>>(Source{{12, 34}},         //
                                  Call<vec2<Infer>>(20_i),  //
                                  Call<vec2<Infer>>(20_a));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    ASSERT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(
        ctor->Parameters()[0]->Type()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    ASSERT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(
        ctor->Parameters()[1]->Type()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayU32_VecAI_VecF32) {
    // array(vec2(20), vec2(10f));
    auto* tc = Call<array<Infer>>(Source{{12, 34}},         //
                                  Call<vec2<Infer>>(20_a),  //
                                  Call<vec2<Infer>>(20_f));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    EXPECT_TRUE(call->Type()->Is<sem::Array>());
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    ASSERT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(
        ctor->Parameters()[0]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    ASSERT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(
        ctor->Parameters()[1]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
}

TEST_F(ResolverValueConstructorValidationTest, ArrayArgumentTypeMismatch_U32F32) {
    // array<u32, 3u>(0u, 1.0f, 20u);
    auto* tc = Call<array<u32, 3>>(0_u, Expr(Source{{12, 34}}, 1_f), 20_u);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: 'f32' cannot be used to construct an array of 'u32')");
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayArgumentTypeMismatch_U32F32) {
    // array(0u, 1.0f, 20u);
    auto* tc = Call<array<Infer>>(Source{{12, 34}}, 0_u, 1_f, 20_u);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot infer common array element type from constructor arguments
note: argument 0 is of type 'u32'
note: argument 1 is of type 'f32')");
}

TEST_F(ResolverValueConstructorValidationTest, ArrayArgumentTypeMismatch_F32I32) {
    // array<f32, 1u>(1i);
    auto* tc = Call<array<f32, 1>>(Expr(Source{{12, 34}}, 1_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: 'i32' cannot be used to construct an array of 'f32')");
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayArgumentTypeMismatch_F32I32) {
    // array(1f, 1i);
    auto* tc = Call<array<Infer>>(Source{{12, 34}}, 1_f, 1_i);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot infer common array element type from constructor arguments
note: argument 0 is of type 'f32'
note: argument 1 is of type 'i32')");
}

TEST_F(ResolverValueConstructorValidationTest, ArrayArgumentTypeMismatch_U32I32) {
    // array<u32, 1u>(1i, 0u, 0u, 0u, 0u, 0u);
    auto* tc = Call<array<u32, 1>>(Expr(Source{{12, 34}}, 1_i), 0_u, 0_u, 0_u, 0_u);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: 'i32' cannot be used to construct an array of 'u32')");
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayArgumentTypeMismatch_U32I32) {
    // array(1i, 0u, 0u, 0u, 0u, 0u);
    auto* tc = Call<array<Infer>>(Source{{12, 34}}, 1_i, 0_u, 0_u, 0_u, 0_u);
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot infer common array element type from constructor arguments
note: argument 0 is of type 'i32'
note: argument 1 is of type 'u32')");
}

TEST_F(ResolverValueConstructorValidationTest, ArrayArgumentTypeMismatch_I32Vec2) {
    // array<i32, 3u>(1i, vec2<i32>());
    auto* tc = Call<array<i32, 3>>(1_i, Call<vec2<i32>>(Source{{12, 34}}));
    WrapInFunction(tc);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: 'vec2<i32>' cannot be used to construct an array of 'i32')");
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayArgumentTypeMismatch_I32Vec2) {
    // array(1i, vec2<i32>());
    auto* tc = Call<array<Infer>>(Source{{12, 34}}, 1_i, Call<vec2<i32>>());
    WrapInFunction(tc);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot infer common array element type from constructor arguments
note: argument 0 is of type 'i32'
note: argument 1 is of type 'vec2<i32>')");
}

TEST_F(ResolverValueConstructorValidationTest, ArrayArgumentTypeMismatch_Vec3i32_Vec3u32) {
    // array<vec3<i32>, 2u>(vec3<u32>(), vec3<u32>());
    auto* t = Call<array<vec3<i32>, 2>>(Call<vec3<u32>>(Source{{12, 34}}), Call<vec3<u32>>());
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: 'vec3<u32>' cannot be used to construct an array of 'vec3<i32>')");
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayArgumentTypeMismatch_Vec3i32_Vec3u32) {
    // array(vec3<i32>(), vec3<u32>());
    auto* t = Call<array<Infer>>(Source{{12, 34}}, Call<vec3<i32>>(), Call<vec3<u32>>());
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot infer common array element type from constructor arguments
note: argument 0 is of type 'vec3<i32>'
note: argument 1 is of type 'vec3<u32>')");
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayArgumentTypeMismatch_Vec3i32_Vec3AF) {
    // array(vec3<i32>(), vec3(1.0));
    auto* t = Call<array<Infer>>(Source{{12, 34}}, Call<vec3<i32>>(), Call("vec3", 1._a));
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot infer common array element type from constructor arguments
note: argument 0 is of type 'vec3<i32>'
note: argument 1 is of type 'vec3<abstract-float>')");
}

TEST_F(ResolverValueConstructorValidationTest, ArrayArgumentTypeMismatch_Vec3i32_Vec3bool) {
    // array<vec3<i32>, 2u>(vec3<i32>(), vec3<bool>());
    auto* t = Call<array<vec3<i32>, 2>>(Call<vec3<i32>>(), Call<vec3<bool>>());
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: 'vec3<bool>' cannot be used to construct an array of 'vec3<i32>')");
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayArgumentTypeMismatch_Vec3i32_Vec3bool) {
    // array(vec3<i32>(), vec3<bool>());
    auto* t = Call<array<Infer>>(Source{{12, 34}}, Call<vec3<i32>>(), Call<vec3<bool>>());
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot infer common array element type from constructor arguments
note: argument 0 is of type 'vec3<i32>'
note: argument 1 is of type 'vec3<bool>')");
}

TEST_F(ResolverValueConstructorValidationTest, ArrayOfArray_SubElemSizeMismatch) {
    // array<array<i32, 2u>, 2u>(array<i32, 3u>(), array<i32, 2u>());
    auto* t = Call<array<array<i32, 2>, 2>>(Source{{12, 34}}, Call<array<i32, 3>>(),
                                            Call<array<i32, 2>>());
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: 'array<i32, 3>' cannot be used to construct an array of 'array<i32, 2>')");
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayOfArray_SubElemSizeMismatch) {
    // array<array<i32, 2u>, 2u>(array<i32, 3u>(), array<i32, 2u>());
    auto* t = Call<array<Infer>>(Source{{12, 34}}, Call<array<i32, 3>>(), Call<array<i32, 2>>());
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot infer common array element type from constructor arguments
note: argument 0 is of type 'array<i32, 3>'
note: argument 1 is of type 'array<i32, 2>')");
}

TEST_F(ResolverValueConstructorValidationTest, ArrayOfArray_SubElemTypeMismatch) {
    // array<array<i32, 2u>, 2u>(array<i32, 2u>(), array<u32, 2u>());
    auto* t = Call<array<array<i32, 2>, 2>>(Source{{12, 34}}, Call<array<i32, 2>>(),
                                            Call<array<u32, 2>>());
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: 'array<u32, 2>' cannot be used to construct an array of 'array<i32, 2>')");
}

TEST_F(ResolverValueConstructorValidationTest, InferredArrayOfArray_SubElemTypeMismatch) {
    // array<array<i32, 2u>, 2u>(array<i32, 2u>(), array<u32, 2u>());
    auto* t = Call<array<Infer>>(Source{{12, 34}}, Call<array<i32, 2>>(), Call<array<u32, 2>>());
    WrapInFunction(t);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot infer common array element type from constructor arguments
note: argument 0 is of type 'array<i32, 2>'
note: argument 1 is of type 'array<u32, 2>')");
}

TEST_F(ResolverValueConstructorValidationTest, Array_TooFewElements) {
    // array<i32, 4u>(1i, 2i, 3i);
    SetSource(Source::Location({12, 34}));
    auto* tc = Call<array<i32, 4>>(Expr(1_i), Expr(2_i), Expr(3_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: array constructor has too few elements: expected 4, found 3");
}

TEST_F(ResolverValueConstructorValidationTest, Array_TooManyElements) {
    // array<i32, 4u>(1i, 2i, 3i, 4i, 5i);
    SetSource(Source::Location({12, 34}));
    auto* tc = Call<array<i32, 4>>(Expr(1_i), Expr(2_i), Expr(3_i), Expr(4_i), Expr(5_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: array constructor has too many "
              "elements: expected 4, "
              "found 5");
}

TEST_F(ResolverValueConstructorValidationTest, Array_ExcessiveNumberOfElements) {
    // array<i32, 40000u>(0i, 2i, 3i, ..., 39999i);
    SetSource(Source::Location({12, 34}));
    Vector<const ast::IntLiteralExpression*, 40000> elements;
    for (uint32_t i = 0; i < 40000; i++) {
        elements.Push(Expr(i32(i)));
    }
    auto* tc = Call<array<i32, 40000>>(std::move(elements));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: array constructor has excessive number of elements (>32767)");
}

TEST_F(ResolverValueConstructorValidationTest, Array_Runtime) {
    // array<i32>(1i);
    auto* tc = Call<array<i32>>(Source{{12, 34}}, Expr(1_i));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot construct a runtime-sized array");
}

TEST_F(ResolverValueConstructorValidationTest, Array_RuntimeZeroValue) {
    // array<i32>();
    auto* tc = Call<array<i32>>(Source{{12, 34}});
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot construct a runtime-sized array");
}

}  // namespace ArrayConstructor

namespace ScalarConstructor {

TEST_F(ResolverValueConstructorValidationTest, I32_Success) {
    auto* expr = Call<i32>(Expr(123_i));
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<core::type::I32>());

    auto* call = Sem().Get<sem::Call>(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::I32>());
}

TEST_F(ResolverValueConstructorValidationTest, U32_Success) {
    auto* expr = Call<u32>(Expr(123_u));
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<core::type::U32>());

    auto* call = Sem().Get<sem::Call>(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::U32>());
}

TEST_F(ResolverValueConstructorValidationTest, F32_Success) {
    auto* expr = Call<f32>(Expr(1.23_f));
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<core::type::F32>());

    auto* call = Sem().Get<sem::Call>(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::F32>());
}

TEST_F(ResolverValueConstructorValidationTest, F16_Success) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<f16>(Expr(1.5_h));
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<core::type::F16>());

    auto* call = Sem().Get<sem::Call>(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::F16>());
}

TEST_F(ResolverValueConstructorValidationTest, Convert_f32_to_i32_Success) {
    auto* expr = Call<i32>(1.23_f);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<core::type::I32>());

    auto* call = Sem().Get<sem::Call>(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::F32>());
}

TEST_F(ResolverValueConstructorValidationTest, Convert_i32_to_u32_Success) {
    auto* expr = Call<u32>(123_i);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<core::type::U32>());

    auto* call = Sem().Get<sem::Call>(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::I32>());
}

TEST_F(ResolverValueConstructorValidationTest, Convert_u32_to_f16_Success) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<f16>(123_u);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<core::type::F16>());

    auto* call = Sem().Get<sem::Call>(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::U32>());
}

TEST_F(ResolverValueConstructorValidationTest, Convert_f16_to_f32_Success) {
    Enable(wgsl::Extension::kF16);

    auto* expr = Call<f32>(123_h);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<core::type::F32>());

    auto* call = Sem().Get<sem::Call>(expr);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::F16>());
}

}  // namespace ScalarConstructor

namespace VectorConstructor {

TEST_F(ResolverValueConstructorValidationTest, Vec2F32_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec2<f32>>(Source{{12, 34}}, 1_i, 2_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<f32>(i32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2F16_Error_ScalarArgumentTypeMismatch) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Call<vec2<f16>>(Source{{12, 34}}, 1_h, 2_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<f16>(f16, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2U32_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec2<u32>>(Source{{12, 34}}, 1_u, 2_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<u32>(u32, i32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2I32_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec2<i32>>(Source{{12, 34}}, 1_u, 2_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<i32>(u32, i32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2Bool_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec2<bool>>(Source{{12, 34}}, true, 1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<bool>(bool, i32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2_Error_Vec3ArgumentCardinalityTooLarge) {
    WrapInFunction(Call<vec2<f32>>(Source{{12, 34}}, Call<vec3<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<f32>(vec3<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2_Error_Vec4ArgumentCardinalityTooLarge) {
    WrapInFunction(Call<vec2<f32>>(Source{{12, 34}}, Call<vec4<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<f32>(vec4<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2_Error_TooManyArgumentsScalar) {
    WrapInFunction(Call<vec2<f32>>(Source{{12, 34}}, 1_f, 2_f, 3_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<f32>(f32, f32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2_Error_TooManyArgumentsVector) {
    WrapInFunction(Call<vec2<f32>>(Source{{12, 34}}, Call<vec2<f32>>(), Call<vec2<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec2<f32>(vec2<f32>, vec2<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2_Error_TooManyArgumentsVectorAndScalar) {
    WrapInFunction(Call<vec2<f32>>(Source{{12, 34}}, Call<vec2<f32>>(), 1_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<f32>(vec2<f32>, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2_Error_InvalidArgumentType) {
    WrapInFunction(Call<vec2<f32>>(Source{{12, 34}}, Call<mat2x2<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<f32>(mat2x2<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec2_Success_ZeroValue) {
    auto* tc = Call<vec2<f32>>();
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 2u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 0u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec2F32_Success_Scalar) {
    auto* tc = Call<vec2<f32>>(1_f, 1_f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 2u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::F32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::F32>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec2F16_Success_Scalar) {
    Enable(wgsl::Extension::kF16);

    auto* tc = Call<vec2<f16>>(1_h, 1_h);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 2u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::F16>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::F16>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec2U32_Success_Scalar) {
    auto* tc = Call<vec2<u32>>(1_u, 1_u);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 2u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::U32>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec2I32_Success_Scalar) {
    auto* tc = Call<vec2<i32>>(1_i, 1_i);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 2u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec2Bool_Success_Scalar) {
    auto* tc = Call<vec2<bool>>(true, false);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 2u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::Bool>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec2_Success_Identity) {
    auto* tc = Call<vec2<f32>>(Call<vec2<f32>>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 2u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec2_Success_Vec2TypeConversion) {
    auto* tc = Call<vec2<f32>>(Call<vec2<i32>>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 2u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec3F32_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec3<f32>>(Source{{12, 34}}, 1_f, 2_f, 3_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<f32>(f32, f32, i32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3F16_Error_ScalarArgumentTypeMismatch) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Call<vec3<f16>>(Source{{12, 34}}, 1_h, 2_h, 3_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<f16>(f16, f16, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3U32_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec3<u32>>(Source{{12, 34}}, 1_u, 2_i, 3_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<u32>(u32, i32, u32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3I32_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec3<i32>>(Source{{12, 34}}, 1_i, 2_u, 3_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<i32>(i32, u32, i32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3Bool_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec3<bool>>(Source{{12, 34}}, false, 1_i, true));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec3<bool>(bool, i32, bool)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Error_Vec4ArgumentCardinalityTooLarge) {
    WrapInFunction(Call<vec3<f32>>(Source{{12, 34}}, Call<vec4<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<f32>(vec4<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Error_TooFewArgumentsScalar) {
    WrapInFunction(Call<vec3<f32>>(Source{{12, 34}}, 1_f, 2_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<f32>(f32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Error_TooManyArgumentsScalar) {
    WrapInFunction(Call<vec3<f32>>(Source{{12, 34}}, 1_f, 2_f, 3_f, 4_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec3<f32>(f32, f32, f32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Error_TooFewArgumentsVec2) {
    WrapInFunction(Call<vec3<f32>>(Source{{12, 34}}, Call<vec2<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<f32>(vec2<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Error_TooManyArgumentsVec2) {
    WrapInFunction(Call<vec3<f32>>(Source{{12, 34}}, Call<vec2<f32>>(), Call<vec2<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec3<f32>(vec2<f32>, vec2<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Error_TooManyArgumentsVec2AndScalar) {
    WrapInFunction(Call<vec3<f32>>(Source{{12, 34}}, Call<vec2<f32>>(), 1_f, 1_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec3<f32>(vec2<f32>, f32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Error_TooManyArgumentsVec3) {
    WrapInFunction(Call<vec3<f32>>(Source{{12, 34}}, Call<vec3<f32>>(), 1_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<f32>(vec3<f32>, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Error_InvalidArgumentType) {
    WrapInFunction(Call<vec3<f32>>(Source{{12, 34}}, Call<mat2x2<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<f32>(mat2x2<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Success_ZeroValue) {
    auto* tc = Call<vec3<f32>>();
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 0u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec3F32_Success_Scalar) {
    auto* tc = Call<vec3<f32>>(1_f, 1_f, 1_f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::F32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::F32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::F32>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec3F16_Success_Scalar) {
    Enable(wgsl::Extension::kF16);

    auto* tc = Call<vec3<f16>>(1_h, 1_h, 1_h);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::F16>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::F16>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::F16>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec3U32_Success_Scalar) {
    auto* tc = Call<vec3<u32>>(1_u, 1_u, 1_u);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::U32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::U32>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec3I32_Success_Scalar) {
    auto* tc = Call<vec3<i32>>(1_i, 1_i, 1_i);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::I32>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec3Bool_Success_Scalar) {
    auto* tc = Call<vec3<bool>>(true, false, true);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::Bool>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Success_Vec2AndScalar) {
    auto* tc = Call<vec3<f32>>(Call<vec2<f32>>(), 1_f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::F32>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Success_ScalarAndVec2) {
    auto* tc = Call<vec3<f32>>(1_f, Call<vec2<f32>>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::F32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::Vector>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Success_Identity) {
    auto* tc = Call<vec3<f32>>(Call<vec3<f32>>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec3_Success_Vec3TypeConversion) {
    auto* tc = Call<vec3<f32>>(Call<vec3<i32>>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 3u);

    auto* call = Sem().Get<sem::Call>(tc);
    ASSERT_NE(call, nullptr);
    auto* ctor = call->Target()->As<sem::ValueConversion>();
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(call->Type(), ctor->ReturnType());
    ASSERT_EQ(ctor->Parameters().Length(), 1u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
}

TEST_F(ResolverValueConstructorValidationTest, Vec4F32_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, 1_f, 1_f, 1_i, 1_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec4<f32>(f32, f32, i32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4F16_Error_ScalarArgumentTypeMismatch) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Call<vec4<f16>>(Source{{12, 34}}, 1_h, 1_h, 1_f, 1_h));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec4<f16>(f16, f16, f32, f16)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4U32_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec4<u32>>(Source{{12, 34}}, 1_u, 1_u, 1_i, 1_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec4<u32>(u32, u32, i32, u32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4I32_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec4<i32>>(Source{{12, 34}}, 1_i, 1_i, 1_u, 1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec4<i32>(i32, i32, u32, i32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4Bool_Error_ScalarArgumentTypeMismatch) {
    WrapInFunction(Call<vec4<bool>>(Source{{12, 34}}, true, false, 1_i, true));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec4<bool>(bool, bool, i32, bool)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooFewArgumentsScalar) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, 1_f, 2_f, 3_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec4<f32>(f32, f32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooManyArgumentsScalar) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, 1_f, 2_f, 3_f, 4_f, 5_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec4<f32>(f32, f32, f32, f32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooFewArgumentsVec2AndScalar) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, Call<vec2<f32>>(), 1_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec4<f32>(vec2<f32>, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooManyArgumentsVec2AndScalars) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, Call<vec2<f32>>(), 1_f, 2_f, 3_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr(
            "12:34 error: no matching constructor for 'vec4<f32>(vec2<f32>, f32, f32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooManyArgumentsVec2Vec2Scalar) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, Call<vec2<f32>>(), Call<vec2<f32>>(), 1_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr(
            "12:34 error: no matching constructor for 'vec4<f32>(vec2<f32>, vec2<f32>, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooManyArgumentsVec2Vec2Vec2) {
    WrapInFunction(
        Call<vec4<f32>>(Source{{12, 34}}, Call<vec2<f32>>(), Call<vec2<f32>>(), Call<vec2<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for "
                                        "'vec4<f32>(vec2<f32>, vec2<f32>, vec2<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooFewArgumentsVec3) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, Call<vec3<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec4<f32>(vec3<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooManyArgumentsVec3AndScalars) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, Call<vec3<f32>>(), 1_f, 2_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec4<f32>(vec3<f32>, f32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooManyArgumentsVec3AndVec2) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, Call<vec3<f32>>(), Call<vec2<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec4<f32>(vec3<f32>, vec2<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooManyArgumentsVec2AndVec3) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, Call<vec2<f32>>(), Call<vec3<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec4<f32>(vec2<f32>, vec3<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_TooManyArgumentsVec3AndVec3) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, Call<vec3<f32>>(), Call<vec3<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'vec4<f32>(vec3<f32>, vec3<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Error_InvalidArgumentType) {
    WrapInFunction(Call<vec4<f32>>(Source{{12, 34}}, Call<mat2x2<f32>>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec4<f32>(mat2x2<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Success_ZeroValue) {
    auto* tc = Call<vec4<f32>>();
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4F32_Success_Scalar) {
    auto* tc = Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4F16_Success_Scalar) {
    Enable(wgsl::Extension::kF16);

    auto* tc = Call<vec4<f16>>(1_h, 1_h, 1_h, 1_h);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4U32_Success_Scalar) {
    auto* tc = Call<vec4<u32>>(1_u, 1_u, 1_u, 1_u);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4I32_Success_Scalar) {
    auto* tc = Call<vec4<i32>>(1_i, 1_i, 1_i, 1_i);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4Bool_Success_Scalar) {
    auto* tc = Call<vec4<bool>>(true, false, true, false);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Success_Vec2ScalarScalar) {
    auto* tc = Call<vec4<f32>>(Call<vec2<f32>>(), 1_f, 1_f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Success_ScalarVec2Scalar) {
    auto* tc = Call<vec4<f32>>(1_f, Call<vec2<f32>>(), 1_f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Success_ScalarScalarVec2) {
    auto* tc = Call<vec4<f32>>(1_f, 1_f, Call<vec2<f32>>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Success_Vec2AndVec2) {
    auto* tc = Call<vec4<f32>>(Call<vec2<f32>>(), Call<vec2<f32>>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Success_Vec3AndScalar) {
    auto* tc = Call<vec4<f32>>(Call<vec3<f32>>(), 1_f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Success_ScalarAndVec3) {
    auto* tc = Call<vec4<f32>>(1_f, Call<vec3<f32>>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Success_Identity) {
    auto* tc = Call<vec4<f32>>(Call<vec4<f32>>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vec4_Success_Vec4TypeConversion) {
    auto* tc = Call<vec4<f32>>(Call<vec4<i32>>());
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, NestedVectorConstructors_InnerError) {
    WrapInFunction(Call<vec4<f32>>(Call<vec4<f32>>(1_f, 1_f,  //
                                                   Call<vec3<f32>>(Source{{12, 34}}, 1_f, 1_f)),
                                   1_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<f32>(f32, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, NestedVectorConstructors_Success) {
    auto* tc = Call<vec4<f32>>(Call<vec3<f32>>(Call<vec2<f32>>(1_f, 1_f), 1_f), 1_f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(tc), nullptr);
    ASSERT_TRUE(TypeOf(tc)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(tc)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(TypeOf(tc)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, Vector_Alias_Argument_Error) {
    auto* alias = Alias("UnsignedInt", ty.u32());
    GlobalVar("uint_var", ty.Of(alias), core::AddressSpace::kPrivate);

    auto* tc = Call<vec2<f32>>(Source{{12, 34}}, "uint_var");
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<f32>(u32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vector_Alias_Argument_Success) {
    auto* f32_alias = Alias("Float32", ty.f32());
    auto* vec2_alias = Alias("VectorFloat2", ty.vec2<f32>());
    GlobalVar("my_f32", ty.Of(f32_alias), core::AddressSpace::kPrivate);
    GlobalVar("my_vec2", ty.Of(vec2_alias), core::AddressSpace::kPrivate);

    auto* tc = Call<vec3<f32>>("my_vec2", "my_f32");
    WrapInFunction(tc);
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValueConstructorValidationTest, Vector_ElementTypeAlias_Error) {
    auto* f32_alias = Alias("Float32", ty.f32());

    // vec2<Float32>(1.0f, 1u)
    auto vec_type = ty.vec(ty.Of(f32_alias), 2);
    WrapInFunction(Call(Source{{12, 34}}, vec_type, 1_f, 1_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec2<f32>(f32, u32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vector_ElementTypeAlias_Success) {
    auto* f32_alias = Alias("Float32", ty.f32());

    // vec2<Float32>(1.0f, 1.0f)
    auto vec_type = ty.vec(ty.Of(f32_alias), 2);
    auto* tc = Call(Source{{12, 34}}, vec_type, 1_f, 1_f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValueConstructorValidationTest, Vector_ArgumentElementTypeAlias_Error) {
    auto* f32_alias = Alias("Float32", ty.f32());

    // vec3<u32>(vec<Float32>(), 1.0f)
    auto vec_type = ty.vec(ty.Of(f32_alias), 2);
    WrapInFunction(Call<vec3<u32>>(Source{{12, 34}}, Call(vec_type), 1_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching constructor for 'vec3<u32>(vec2<f32>, f32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, Vector_ArgumentElementTypeAlias_Success) {
    auto* f32_alias = Alias("Float32", ty.f32());

    // vec3<f32>(vec<Float32>(), 1.0f)
    auto vec_type = ty.vec(ty.Of(f32_alias), 2);
    auto* tc = Call<vec3<f32>>(Call(Source{{12, 34}}, vec_type), 1_f);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValueConstructorValidationTest, InferVec2ElementTypeFromScalars) {
    Enable(wgsl::Extension::kF16);

    auto* vec2_bool = Call<vec2<Infer>>(true, false);
    auto* vec2_i32 = Call<vec2<Infer>>(1_i, 2_i);
    auto* vec2_u32 = Call<vec2<Infer>>(1_u, 2_u);
    auto* vec2_f32 = Call<vec2<Infer>>(1_f, 2_f);
    auto* vec2_f16 = Call<vec2<Infer>>(1_h, 2_h);
    WrapInFunction(vec2_bool, vec2_i32, vec2_u32, vec2_f32, vec2_f16);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec2_bool)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec2_i32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec2_u32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec2_f32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec2_f16)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(vec2_bool)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(vec2_i32)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(vec2_u32)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(vec2_f32)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(vec2_f16)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(vec2_bool)->As<core::type::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_i32)->As<core::type::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_u32)->As<core::type::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_f32)->As<core::type::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_f16)->As<core::type::Vector>()->Width(), 2u);
}

TEST_F(ResolverValueConstructorValidationTest, InferVec2ElementTypeFromVec2) {
    Enable(wgsl::Extension::kF16);

    auto* vec2_bool = Call<vec2<Infer>>(Call<vec2<bool>>(true, false));
    auto* vec2_i32 = Call<vec2<Infer>>(Call<vec2<i32>>(1_i, 2_i));
    auto* vec2_u32 = Call<vec2<Infer>>(Call<vec2<u32>>(1_u, 2_u));
    auto* vec2_f32 = Call<vec2<Infer>>(Call<vec2<f32>>(1_f, 2_f));
    auto* vec2_f16 = Call<vec2<Infer>>(Call<vec2<f16>>(1_h, 2_h));
    WrapInFunction(vec2_bool, vec2_i32, vec2_u32, vec2_f32, vec2_f16);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec2_bool)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec2_i32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec2_u32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec2_f32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec2_f16)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(vec2_bool)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(vec2_i32)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(vec2_u32)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(vec2_f32)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(vec2_f16)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(vec2_bool)->As<core::type::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_i32)->As<core::type::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_u32)->As<core::type::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_f32)->As<core::type::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(vec2_f16)->As<core::type::Vector>()->Width(), 2u);
}

TEST_F(ResolverValueConstructorValidationTest, InferVec3ElementTypeFromScalars) {
    Enable(wgsl::Extension::kF16);

    auto* vec3_bool = Call<vec3<Infer>>(true, false, true);
    auto* vec3_i32 = Call<vec3<Infer>>(1_i, 2_i, 3_i);
    auto* vec3_u32 = Call<vec3<Infer>>(1_u, 2_u, 3_u);
    auto* vec3_f32 = Call<vec3<Infer>>(1_f, 2_f, 3_f);
    auto* vec3_f16 = Call<vec3<Infer>>(1_h, 2_h, 3_h);
    WrapInFunction(vec3_bool, vec3_i32, vec3_u32, vec3_f32, vec3_f16);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec3_bool)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_i32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_u32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_f32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_f16)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(vec3_bool)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(vec3_i32)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(vec3_u32)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(vec3_f32)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(vec3_f16)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(vec3_bool)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_i32)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_u32)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_f32)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_f16)->As<core::type::Vector>()->Width(), 3u);
}

TEST_F(ResolverValueConstructorValidationTest, InferVec3ElementTypeFromVec3) {
    Enable(wgsl::Extension::kF16);

    auto* vec3_bool = Call<vec3<Infer>>(Call<vec3<bool>>(true, false, true));
    auto* vec3_i32 = Call<vec3<Infer>>(Call<vec3<i32>>(1_i, 2_i, 3_i));
    auto* vec3_u32 = Call<vec3<Infer>>(Call<vec3<u32>>(1_u, 2_u, 3_u));
    auto* vec3_f32 = Call<vec3<Infer>>(Call<vec3<f32>>(1_f, 2_f, 3_f));
    auto* vec3_f16 = Call<vec3<Infer>>(Call<vec3<f16>>(1_h, 2_h, 3_h));
    WrapInFunction(vec3_bool, vec3_i32, vec3_u32, vec3_f32, vec3_f16);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec3_bool)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_i32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_u32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_f32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_f16)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(vec3_bool)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(vec3_i32)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(vec3_u32)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(vec3_f32)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(vec3_f16)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(vec3_bool)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_i32)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_u32)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_f32)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_f16)->As<core::type::Vector>()->Width(), 3u);
}

TEST_F(ResolverValueConstructorValidationTest, InferVec3ElementTypeFromScalarAndVec2) {
    Enable(wgsl::Extension::kF16);

    auto* vec3_bool = Call<vec3<Infer>>(true, Call<vec2<bool>>(false, true));
    auto* vec3_i32 = Call<vec3<Infer>>(1_i, Call<vec2<i32>>(2_i, 3_i));
    auto* vec3_u32 = Call<vec3<Infer>>(1_u, Call<vec2<u32>>(2_u, 3_u));
    auto* vec3_f32 = Call<vec3<Infer>>(1_f, Call<vec2<f32>>(2_f, 3_f));
    auto* vec3_f16 = Call<vec3<Infer>>(1_h, Call<vec2<f16>>(2_h, 3_h));
    WrapInFunction(vec3_bool, vec3_i32, vec3_u32, vec3_f32, vec3_f16);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec3_bool)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_i32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_u32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_f32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec3_f16)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(vec3_bool)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(vec3_i32)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(vec3_u32)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(vec3_f32)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(vec3_f16)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(vec3_bool)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_i32)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_u32)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_f32)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(vec3_f16)->As<core::type::Vector>()->Width(), 3u);
}

TEST_F(ResolverValueConstructorValidationTest, InferVec4ElementTypeFromScalars) {
    Enable(wgsl::Extension::kF16);

    auto* vec4_bool = Call<vec4<Infer>>(true, false, true, false);
    auto* vec4_i32 = Call<vec4<Infer>>(1_i, 2_i, 3_i, 4_i);
    auto* vec4_u32 = Call<vec4<Infer>>(1_u, 2_u, 3_u, 4_u);
    auto* vec4_f32 = Call<vec4<Infer>>(1_f, 2_f, 3_f, 4_f);
    auto* vec4_f16 = Call<vec4<Infer>>(1_h, 2_h, 3_h, 4_h);
    WrapInFunction(vec4_bool, vec4_i32, vec4_u32, vec4_f32, vec4_f16);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec4_bool)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_i32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_u32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f16)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(vec4_bool)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(vec4_i32)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(vec4_u32)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(vec4_f32)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(vec4_f16)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(vec4_bool)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_i32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_u32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f16)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, InferVec4ElementTypeFromVec4) {
    Enable(wgsl::Extension::kF16);

    auto* vec4_bool = Call<vec4<Infer>>(Call<vec4<bool>>(true, false, true, false));
    auto* vec4_i32 = Call<vec4<Infer>>(Call<vec4<i32>>(1_i, 2_i, 3_i, 4_i));
    auto* vec4_u32 = Call<vec4<Infer>>(Call<vec4<u32>>(1_u, 2_u, 3_u, 4_u));
    auto* vec4_f32 = Call<vec4<Infer>>(Call<vec4<f32>>(1_f, 2_f, 3_f, 4_f));
    auto* vec4_f16 = Call<vec4<Infer>>(Call<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
    WrapInFunction(vec4_bool, vec4_i32, vec4_u32, vec4_f32, vec4_f16);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec4_bool)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_i32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_u32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f16)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(vec4_bool)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(vec4_i32)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(vec4_u32)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(vec4_f32)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(vec4_f16)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(vec4_bool)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_i32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_u32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f16)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, InferVec4ElementTypeFromScalarAndVec3) {
    Enable(wgsl::Extension::kF16);

    auto* vec4_bool = Call<vec4<Infer>>(true, Call<vec3<bool>>(false, true, false));
    auto* vec4_i32 = Call<vec4<Infer>>(1_i, Call<vec3<i32>>(2_i, 3_i, 4_i));
    auto* vec4_u32 = Call<vec4<Infer>>(1_u, Call<vec3<u32>>(2_u, 3_u, 4_u));
    auto* vec4_f32 = Call<vec4<Infer>>(1_f, Call<vec3<f32>>(2_f, 3_f, 4_f));
    auto* vec4_f16 = Call<vec4<Infer>>(1_h, Call<vec3<f16>>(2_h, 3_h, 4_h));
    WrapInFunction(vec4_bool, vec4_i32, vec4_u32, vec4_f32, vec4_f16);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec4_bool)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_i32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_u32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f16)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(vec4_bool)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(vec4_i32)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(vec4_u32)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(vec4_f32)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(vec4_f16)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(vec4_bool)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_i32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_u32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f16)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, InferVec4ElementTypeFromVec2AndVec2) {
    Enable(wgsl::Extension::kF16);

    auto* vec4_bool =
        Call<vec4<Infer>>(Call<vec2<bool>>(true, false), Call<vec2<bool>>(true, false));
    auto* vec4_i32 = Call<vec4<Infer>>(Call<vec2<i32>>(1_i, 2_i), Call<vec2<i32>>(3_i, 4_i));
    auto* vec4_u32 = Call<vec4<Infer>>(Call<vec2<u32>>(1_u, 2_u), Call<vec2<u32>>(3_u, 4_u));
    auto* vec4_f32 = Call<vec4<Infer>>(Call<vec2<f32>>(1_f, 2_f), Call<vec2<f32>>(3_f, 4_f));
    auto* vec4_f16 = Call<vec4<Infer>>(Call<vec2<f16>>(1_h, 2_h), Call<vec2<f16>>(3_h, 4_h));
    WrapInFunction(vec4_bool, vec4_i32, vec4_u32, vec4_f32, vec4_f16);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(vec4_bool)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_i32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_u32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f32)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(vec4_f16)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(vec4_bool)->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_TRUE(TypeOf(vec4_i32)->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_TRUE(TypeOf(vec4_u32)->As<core::type::Vector>()->Type()->Is<core::type::U32>());
    EXPECT_TRUE(TypeOf(vec4_f32)->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_TRUE(TypeOf(vec4_f16)->As<core::type::Vector>()->Type()->Is<core::type::F16>());
    EXPECT_EQ(TypeOf(vec4_bool)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_i32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_u32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f32)->As<core::type::Vector>()->Width(), 4u);
    EXPECT_EQ(TypeOf(vec4_f16)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, InferVecNoArgs) {
    auto* v2 = Call<vec2<Infer>>();
    auto* v3 = Call<vec3<Infer>>();
    auto* v4 = Call<vec4<Infer>>();

    GlobalConst("v2", v2);
    GlobalConst("v3", v3);
    GlobalConst("v4", v4);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(v2)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(v3)->Is<core::type::Vector>());
    ASSERT_TRUE(TypeOf(v4)->Is<core::type::Vector>());
    EXPECT_TRUE(TypeOf(v2)->As<core::type::Vector>()->Type()->Is<core::type::AbstractInt>());
    EXPECT_TRUE(TypeOf(v3)->As<core::type::Vector>()->Type()->Is<core::type::AbstractInt>());
    EXPECT_TRUE(TypeOf(v4)->As<core::type::Vector>()->Type()->Is<core::type::AbstractInt>());
    EXPECT_EQ(TypeOf(v2)->As<core::type::Vector>()->Width(), 2u);
    EXPECT_EQ(TypeOf(v3)->As<core::type::Vector>()->Width(), 3u);
    EXPECT_EQ(TypeOf(v4)->As<core::type::Vector>()->Width(), 4u);
}

TEST_F(ResolverValueConstructorValidationTest, CannotInferVec2ElementTypeFromScalarsMismatch) {
    WrapInFunction(Call(Source{{1, 1}}, "vec2",     //
                        Expr(Source{{1, 2}}, 1_i),  //
                        Expr(Source{{1, 3}}, 2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("1:1 error: no matching constructor for 'vec2(i32, u32)'"));
}

TEST_F(ResolverValueConstructorValidationTest, CannotInferVec3ElementTypeFromScalarsMismatch) {
    WrapInFunction(Call(Source{{1, 1}}, "vec3",     //
                        Expr(Source{{1, 2}}, 1_i),  //
                        Expr(Source{{1, 3}}, 2_u),  //
                        Expr(Source{{1, 4}}, 3_i)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("1:1 error: no matching constructor for 'vec3(i32, u32, i32)'"));
}

TEST_F(ResolverValueConstructorValidationTest,
       CannotInferVec3ElementTypeFromScalarAndVec2Mismatch) {
    WrapInFunction(Call(Source{{1, 1}}, "vec3",     //
                        Expr(Source{{1, 2}}, 1_i),  //
                        Call(Source{{1, 3}}, ty.vec2<f32>(), 2_f, 3_f)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("1:1 error: no matching constructor for 'vec3(i32, vec2<f32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, CannotInferVec4ElementTypeFromScalarsMismatch) {
    WrapInFunction(Call(Source{{1, 1}}, "vec4",     //
                        Expr(Source{{1, 2}}, 1_i),  //
                        Expr(Source{{1, 3}}, 2_i),  //
                        Expr(Source{{1, 4}}, 3_f),  //
                        Expr(Source{{1, 5}}, 4_i)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("1:1 error: no matching constructor for 'vec4(i32, i32, f32, i32)'"));
}

TEST_F(ResolverValueConstructorValidationTest,
       CannotInferVec4ElementTypeFromScalarAndVec3Mismatch) {
    WrapInFunction(Call(Source{{1, 1}}, "vec4",     //
                        Expr(Source{{1, 2}}, 1_i),  //
                        Call(Source{{1, 3}}, ty.vec3<u32>(), 2_u, 3_u, 4_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("1:1 error: no matching constructor for 'vec4(i32, vec3<u32>)'"));
}

TEST_F(ResolverValueConstructorValidationTest, CannotInferVec4ElementTypeFromVec2AndVec2Mismatch) {
    WrapInFunction(Call(Source{{1, 1}}, "vec4",                          //
                        Call(Source{{1, 2}}, ty.vec2<i32>(), 3_i, 4_i),  //
                        Call(Source{{1, 3}}, ty.vec2<u32>(), 3_u, 4_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("1:1 error: no matching constructor for 'vec4(vec2<i32>, vec2<u32>)'"));
}

}  // namespace VectorConstructor

namespace MatrixConstructor {

struct MatrixParams {
    using name_func_ptr = std::string (*)();

    uint32_t rows;
    uint32_t columns;
    name_func_ptr get_element_type_name;
    builder::ast_type_func_ptr create_element_ast_type;
    builder::ast_expr_from_double_func_ptr create_element_ast_value;
    builder::ast_type_func_ptr create_column_ast_type;
    builder::ast_type_func_ptr create_mat_ast_type;
};

template <typename T, uint32_t R, uint32_t C>
constexpr MatrixParams MatrixParamsFor() {
    return MatrixParams{
        R,
        C,
        DataType<T>::Name,
        DataType<T>::AST,
        DataType<T>::ExprFromDouble,
        DataType<vec<R, T>>::AST,
        DataType<mat<C, R, T>>::AST,
    };
}

static std::string MatrixStr(const MatrixParams& param) {
    return "mat" + std::to_string(param.columns) + "x" + std::to_string(param.rows) + "<" +
           param.get_element_type_name() + ">";
}

using MatrixConstructorTest = ResolverTestWithParam<MatrixParams>;

TEST_P(MatrixConstructorTest, ColumnConstructor_Error_TooFewArguments) {
    // matNxM<f32>(vecM<f32>(), ...); with N - 1 arguments
    // matNxM<f16>(vecM<f16>(), ...); with N - 1 arguments

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    const std::string element_type_name = param.get_element_type_name();
    StringStream args_tys;
    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns - 1; i++) {
        ast::Type vec_type = param.create_column_ast_type(*this);
        args.Push(Call(vec_type));
        if (i > 0) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<" + element_type_name + ">";
    }

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(Source{{12, 34}}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for '" +
                                        MatrixStr(param) + "(" + args_tys.str() + ")'"));
}

TEST_P(MatrixConstructorTest, ElementConstructor_Error_TooFewArguments) {
    // matNxM<f32>(f32,...,f32); with N*M - 1 arguments
    // matNxM<f16>(f16,...,f16); with N*M - 1 arguments

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    const std::string element_type_name = param.get_element_type_name();
    StringStream args_tys;
    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns * param.rows - 1; i++) {
        args.Push(Call(param.create_element_ast_type(*this)));
        if (i > 0) {
            args_tys << ", ";
        }
        args_tys << element_type_name;
    }

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(Source{{12, 34}}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for '" +
                                        MatrixStr(param) + "(" + args_tys.str() + ")'"));
}

TEST_P(MatrixConstructorTest, ColumnConstructor_Error_TooManyArguments) {
    // matNxM<f32>(vecM<f32>(), ...); with N + 1 arguments
    // matNxM<f16>(vecM<f16>(), ...); with N + 1 arguments

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    const std::string element_type_name = param.get_element_type_name();
    StringStream args_tys;
    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns + 1; i++) {
        ast::Type vec_type = param.create_column_ast_type(*this);
        args.Push(Call(vec_type));
        if (i > 0) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<" + element_type_name + ">";
    }

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(Source{{12, 34}}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for '" +
                                        MatrixStr(param) + "(" + args_tys.str() + ")'"));
}

TEST_P(MatrixConstructorTest, ElementConstructor_Error_TooManyArguments) {
    // matNxM<f32>(f32,...,f32); with N*M + 1 arguments
    // matNxM<f16>(f16,...,f16); with N*M + 1 arguments

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    const std::string element_type_name = param.get_element_type_name();
    StringStream args_tys;
    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns * param.rows + 1; i++) {
        args.Push(Call(param.create_element_ast_type(*this)));
        if (i > 0) {
            args_tys << ", ";
        }
        args_tys << element_type_name;
    }

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(Source{{12, 34}}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for '" +
                                        MatrixStr(param) + "(" + args_tys.str() + ")'"));
}

TEST_P(MatrixConstructorTest, ColumnConstructor_Error_InvalidArgumentType) {
    // matNxM<f32>(vec<u32>, vec<u32>, ...); N arguments
    // matNxM<f16>(vec<u32>, vec<u32>, ...); N arguments

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    StringStream args_tys;
    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        auto vec_type = ty.vec<u32>(param.rows);
        args.Push(Call(vec_type));
        if (i > 0) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<u32>";
    }

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(Source{{12, 34}}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for '" +
                                        MatrixStr(param) + "(" + args_tys.str() + ")'"));
}

TEST_P(MatrixConstructorTest, ElementConstructor_Error_InvalidArgumentType) {
    // matNxM<f32>(u32, u32, ...); N*M arguments
    // matNxM<f16>(u32, u32, ...); N*M arguments

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    StringStream args_tys;
    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        args.Push(Expr(1_u));
        if (i > 0) {
            args_tys << ", ";
        }
        args_tys << "u32";
    }

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(Source{{12, 34}}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for '" +
                                        MatrixStr(param) + "(" + args_tys.str() + ")'"));
}

TEST_P(MatrixConstructorTest, ColumnConstructor_Error_TooFewRowsInVectorArgument) {
    // matNxM<f32>(vecM<f32>(),...,vecM-1<f32>());
    // matNxM<f16>(vecM<f16>(),...,vecM-1<f32>());

    const auto param = GetParam();

    // Skip the test if parameters would have resulted in an invalid vec1 type.
    if (param.rows == 2) {
        return;
    }

    Enable(wgsl::Extension::kF16);

    const std::string element_type_name = param.get_element_type_name();
    StringStream args_tys;
    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        ast::Type valid_vec_type = param.create_column_ast_type(*this);
        args.Push(Call(valid_vec_type));
        if (i > 0) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<" + element_type_name + ">";
    }
    const uint32_t kInvalidLoc = 2 * (param.columns - 1);
    auto invalid_vec_type = ty.vec(param.create_element_ast_type(*this), param.rows - 1);
    args.Push(Call(Source{{12, kInvalidLoc}}, invalid_vec_type));
    args_tys << ", vec" << (param.rows - 1) << "<" + element_type_name + ">";

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(Source{{12, 34}}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for '" +
                                        MatrixStr(param) + "(" + args_tys.str() + ")'"));
}

TEST_P(MatrixConstructorTest, ColumnConstructor_Error_TooManyRowsInVectorArgument) {
    // matNxM<f32>(vecM<f32>(),...,vecM+1<f32>());
    // matNxM<f16>(vecM<f16>(),...,vecM+1<f16>());

    const auto param = GetParam();

    // Skip the test if parameters would have resulted in an invalid vec5 type.
    if (param.rows == 4) {
        return;
    }

    Enable(wgsl::Extension::kF16);

    const std::string element_type_name = param.get_element_type_name();
    StringStream args_tys;
    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        ast::Type valid_vec_type = param.create_column_ast_type(*this);
        args.Push(Call(valid_vec_type));
        if (i > 0) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<" + element_type_name + ">";
    }
    auto invalid_vec_type = ty.vec(param.create_element_ast_type(*this), param.rows + 1);
    args.Push(Call(invalid_vec_type));
    args_tys << ", vec" << (param.rows + 1) << "<" + element_type_name + ">";

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(Source{{12, 34}}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for '" +
                                        MatrixStr(param) + "(" + args_tys.str() + ")'"));
}

TEST_P(MatrixConstructorTest, ZeroValue_Success) {
    // matNxM<f32>();
    // matNxM<f16>();

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(Source{{12, 40}}, matrix_type);
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, WithColumns_Success) {
    // matNxM<f32>(vecM<f32>(), ...); with N arguments
    // matNxM<f16>(vecM<f16>(), ...); with N arguments

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    Vector<const ast::Expression*, 4> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        ast::Type vec_type = param.create_column_ast_type(*this);
        args.Push(Call(vec_type));
    }

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, WithElements_Success) {
    // matNxM<f32>(f32,...,f32); with N*M arguments
    // matNxM<f16>(f16,...,f16); with N*M arguments

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    Vector<const ast::Expression*, 16> args;
    for (uint32_t i = 0; i < param.columns * param.rows; i++) {
        args.Push(Call(param.create_element_ast_type(*this)));
    }

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, ElementTypeAlias_Error) {
    // matNxM<Float32>(vecM<u32>(), ...); with N arguments
    // matNxM<Float16>(vecM<u32>(), ...); with N arguments

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    auto* elem_type_alias = Alias("ElemType", param.create_element_ast_type(*this));

    StringStream args_tys;
    Vector<const ast::Expression*, 4> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        auto vec_type = ty.vec(ty.u32(), param.rows);
        args.Push(Call(vec_type));
        if (i > 0) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<u32>";
    }

    auto matrix_type = ty.mat(ty.Of(elem_type_alias), param.columns, param.rows);
    auto* tc = Call(Source{{12, 34}}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for '" +
                                        MatrixStr(param) + "(" + args_tys.str() + ")'"));
}

TEST_P(MatrixConstructorTest, ElementTypeAlias_Success) {
    // matNxM<Float32>(vecM<f32>(), ...); with N arguments
    // matNxM<Float16>(vecM<f16>(), ...); with N arguments

    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    auto* elem_type_alias = Alias("ElemType", param.create_element_ast_type(*this));

    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        ast::Type vec_type = param.create_column_ast_type(*this);
        args.Push(Call(vec_type));
    }

    auto matrix_type = ty.mat(ty.Of(elem_type_alias), param.columns, param.rows);
    auto* tc = Call(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValueConstructorValidationTest, MatrixConstructor_ArgumentTypeAlias_Error) {
    auto* alias = Alias("VectorUnsigned2", ty.vec2<u32>());
    auto* tc = Call(Source{{12, 34}}, ty.mat2x2<f32>(), Call(ty.Of(alias)), Call<vec2<f32>>());
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching constructor for 'mat2x2<f32>(vec2<u32>, vec2<f32>)'"));
}

TEST_P(MatrixConstructorTest, ArgumentTypeAlias_Success) {
    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    ast::Type vec_type = param.create_column_ast_type(*this);
    auto* vec_alias = Alias("ColVectorAlias", vec_type);

    Vector<const ast::Expression*, 4> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        args.Push(Call(ty.Of(vec_alias)));
    }

    auto* tc = Call(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, ArgumentElementTypeAlias_Error) {
    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* u32_type_alias = Alias("UnsignedInt", ty.u32());

    StringStream args_tys;
    Vector<const ast::Expression*, 4> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        auto vec_type = ty.vec(ty.Of(u32_type_alias), param.rows);
        args.Push(Call(vec_type));
        if (i > 0) {
            args_tys << ", ";
        }
        args_tys << "vec" << param.rows << "<u32>";
    }

    auto* tc = Call(Source{{12, 34}}, matrix_type, std::move(args));
    WrapInFunction(tc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: no matching constructor for '" +
                                        MatrixStr(param) + "(" + args_tys.str() + ")'"));
}

TEST_P(MatrixConstructorTest, ArgumentElementTypeAlias_Success) {
    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    auto* elem_type_alias = Alias("ElemType", param.create_element_ast_type(*this));

    Vector<const ast::Expression*, 4> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        auto vec_type = ty.vec(ty.Of(elem_type_alias), param.rows);
        args.Push(Call(vec_type));
    }

    ast::Type matrix_type = param.create_mat_ast_type(*this);
    auto* tc = Call(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, InferElementTypeFromVectors) {
    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        args.Push(Call(param.create_column_ast_type(*this)));
    }

    auto matrix_type = ty.mat<Infer>(param.columns, param.rows);
    auto* tc = Call(Source{}, matrix_type, std::move(args));
    WrapInFunction(tc);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, InferElementTypeFromScalars) {
    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.rows * param.columns; i++) {
        args.Push(param.create_element_ast_value(*this, static_cast<double>(i)));
    }

    auto matrix_type = ty.mat<Infer>(param.columns, param.rows);
    WrapInFunction(Call(Source{{12, 34}}, matrix_type, std::move(args)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, CannotInferElementTypeFromVectors_Mismatch) {
    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    StringStream err;
    err << "12:34 error: no matching constructor for 'mat" << param.columns << "x" << param.rows
        << "(";

    Vector<const ast::Expression*, 8> args;
    for (uint32_t i = 0; i < param.columns; i++) {
        if (i > 0) {
            err << ", ";
        }
        if (i == 1) {
            // Odd one out
            args.Push(Call(ty.vec<i32>(param.rows)));
            err << "vec" << param.rows << "<i32>";
        } else {
            args.Push(Call(param.create_column_ast_type(*this)));
            err << "vec" << param.rows << "<" + param.get_element_type_name() + ">";
        }
    }

    auto matrix_type = ty.mat<Infer>(param.columns, param.rows);
    WrapInFunction(Call(Source{{12, 34}}, matrix_type, std::move(args)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr(err.str()));
}

TEST_P(MatrixConstructorTest, CannotInferElementTypeFromScalars_Mismatch) {
    const auto param = GetParam();

    Enable(wgsl::Extension::kF16);

    StringStream err;
    err << "12:34 error: no matching constructor for 'mat" << param.columns << "x" << param.rows
        << "(";

    Vector<const ast::Expression*, 16> args;
    for (uint32_t i = 0; i < param.rows * param.columns; i++) {
        if (i > 0) {
            err << ", ";
        }
        if (i == 3) {
            args.Push(Expr(static_cast<i32>(i)));  // The odd one out
            err << "i32";
        } else {
            args.Push(param.create_element_ast_value(*this, static_cast<double>(i)));
            err << param.get_element_type_name();
        }
    }

    err << ")'";

    auto matrix_type = ty.mat<Infer>(param.columns, param.rows);
    WrapInFunction(Call(Source{{12, 34}}, matrix_type, std::move(args)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), HasSubstr(err.str()));
}

INSTANTIATE_TEST_SUITE_P(ResolverValueConstructorValidationTest,
                         MatrixConstructorTest,
                         testing::Values(MatrixParamsFor<f32, 2, 2>(),
                                         MatrixParamsFor<f32, 3, 2>(),
                                         MatrixParamsFor<f32, 4, 2>(),
                                         MatrixParamsFor<f32, 2, 3>(),
                                         MatrixParamsFor<f32, 3, 3>(),
                                         MatrixParamsFor<f32, 4, 3>(),
                                         MatrixParamsFor<f32, 2, 4>(),
                                         MatrixParamsFor<f32, 3, 4>(),
                                         MatrixParamsFor<f32, 4, 4>(),
                                         MatrixParamsFor<f16, 2, 2>(),
                                         MatrixParamsFor<f16, 3, 2>(),
                                         MatrixParamsFor<f16, 4, 2>(),
                                         MatrixParamsFor<f16, 2, 3>(),
                                         MatrixParamsFor<f16, 3, 3>(),
                                         MatrixParamsFor<f16, 4, 3>(),
                                         MatrixParamsFor<f16, 2, 4>(),
                                         MatrixParamsFor<f16, 3, 4>(),
                                         MatrixParamsFor<f16, 4, 4>()));
}  // namespace MatrixConstructor

namespace StructConstructor {
using builder::CreatePtrs;
using builder::CreatePtrsFor;

constexpr CreatePtrs all_types[] = {
    CreatePtrsFor<bool>(),         //
    CreatePtrsFor<u32>(),          //
    CreatePtrsFor<i32>(),          //
    CreatePtrsFor<f32>(),          //
    CreatePtrsFor<f16>(),          //
    CreatePtrsFor<vec4<bool>>(),   //
    CreatePtrsFor<vec2<i32>>(),    //
    CreatePtrsFor<vec3<u32>>(),    //
    CreatePtrsFor<vec4<f32>>(),    //
    CreatePtrsFor<vec4<f16>>(),    //
    CreatePtrsFor<mat2x2<f32>>(),  //
    CreatePtrsFor<mat3x3<f32>>(),  //
    CreatePtrsFor<mat4x4<f32>>(),  //
    CreatePtrsFor<mat2x2<f16>>(),  //
    CreatePtrsFor<mat3x3<f16>>(),  //
    CreatePtrsFor<mat4x4<f16>>()   //
};

auto number_of_members = testing::Values(2u, 32u, 64u);

using StructConstructorInputsTest =
    ResolverTestWithParam<std::tuple<CreatePtrs,  // struct member type
                                     uint32_t>>;  // number of struct members
TEST_P(StructConstructorInputsTest, TooFew) {
    auto& param = GetParam();
    auto& str_params = std::get<0>(param);
    uint32_t N = std::get<1>(param);

    Enable(wgsl::Extension::kF16);

    Vector<const ast::StructMember*, 16> members;
    Vector<const ast::Expression*, 16> values;
    for (uint32_t i = 0; i < N; i++) {
        ast::Type struct_type = str_params.ast(*this);
        members.Push(Member("member_" + std::to_string(i), struct_type));
        if (i < N - 1) {
            auto* ctor_value_expr = str_params.expr_from_double(*this, 0);
            values.Push(ctor_value_expr);
        }
    }
    auto* s = Structure("s", members);
    auto* tc = Call(Source{{12, 34}}, ty.Of(s), values);
    WrapInFunction(tc);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: structure constructor has too few inputs: expected " +
                                std::to_string(N) + ", found " + std::to_string(N - 1));
}

TEST_P(StructConstructorInputsTest, TooMany) {
    auto& param = GetParam();
    auto& str_params = std::get<0>(param);
    uint32_t N = std::get<1>(param);

    Enable(wgsl::Extension::kF16);

    Vector<const ast::StructMember*, 16> members;
    Vector<const ast::Expression*, 8> values;
    for (uint32_t i = 0; i < N + 1; i++) {
        if (i < N) {
            ast::Type struct_type = str_params.ast(*this);
            members.Push(Member("member_" + std::to_string(i), struct_type));
        }
        auto* ctor_value_expr = str_params.expr_from_double(*this, 0);
        values.Push(ctor_value_expr);
    }
    auto* s = Structure("s", members);
    auto* tc = Call(Source{{12, 34}}, ty.Of(s), values);
    WrapInFunction(tc);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: structure constructor has too many inputs: expected " +
                                std::to_string(N) + ", found " + std::to_string(N + 1));
}

INSTANTIATE_TEST_SUITE_P(ResolverValueConstructorValidationTest,
                         StructConstructorInputsTest,
                         testing::Combine(testing::ValuesIn(all_types), number_of_members));
using StructConstructorTypeTest =
    ResolverTestWithParam<std::tuple<CreatePtrs,  // struct member type
                                     CreatePtrs,  // constructor value type
                                     uint32_t>>;  // number of struct members
TEST_P(StructConstructorTypeTest, AllTypes) {
    auto& param = GetParam();
    auto& str_params = std::get<0>(param);
    auto& ctor_params = std::get<1>(param);
    uint32_t N = std::get<2>(param);

    Enable(wgsl::Extension::kF16);

    if (str_params.ast == ctor_params.ast) {
        return;
    }

    Vector<const ast::StructMember*, 16> members;
    Vector<const ast::Expression*, 8> values;
    // make the last value of the constructor to have a different type
    uint32_t constructor_value_with_different_type = N - 1;
    for (uint32_t i = 0; i < N; i++) {
        ast::Type struct_type = str_params.ast(*this);
        members.Push(Member("member_" + std::to_string(i), struct_type));
        auto* ctor_value_expr = (i == constructor_value_with_different_type)
                                    ? ctor_params.expr_from_double(*this, 0)
                                    : str_params.expr_from_double(*this, 0);
        values.Push(ctor_value_expr);
    }
    auto* s = Structure("s", members);
    auto* tc = Call(ty.Of(s), values);
    WrapInFunction(tc);

    StringStream err;
    err << "error: type in structure constructor does not match struct member ";
    err << "type: expected '" << str_params.name() << "', found '" << ctor_params.name() << "'";
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), err.str());
}

INSTANTIATE_TEST_SUITE_P(ResolverValueConstructorValidationTest,
                         StructConstructorTypeTest,
                         testing::Combine(testing::ValuesIn(all_types),
                                          testing::ValuesIn(all_types),
                                          number_of_members));

TEST_F(ResolverValueConstructorValidationTest, Struct_Nested) {
    auto* inner_m = Member("m", ty.i32());
    auto* inner_s = Structure("inner_s", Vector{inner_m});

    auto* m0 = Member("m0", ty.i32());
    auto* m1 = Member("m1", ty.Of(inner_s));
    auto* m2 = Member("m2", ty.i32());
    auto* s = Structure("s", Vector{m0, m1, m2});

    auto* tc = Call(Source{{12, 34}}, ty.Of(s), 1_i, 1_i, 1_i);
    WrapInFunction(tc);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: type in structure constructor does not match struct member "
              "type: expected 'inner_s', found 'i32'");
}

TEST_F(ResolverValueConstructorValidationTest, Struct) {
    auto* m = Member("m", ty.i32());
    auto* s = Structure("MyInputs", Vector{m});
    auto* tc = Call(Source{{12, 34}}, ty.Of(s));
    WrapInFunction(tc);
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValueConstructorValidationTest, Struct_Empty) {
    auto* str = Structure("S", Vector{
                                   Member("a", ty.i32()),
                                   Member("b", ty.f32()),
                                   Member("c", ty.vec3<i32>()),
                               });

    WrapInFunction(Call(ty.Of(str)));
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}
}  // namespace StructConstructor

TEST_F(ResolverValueConstructorValidationTest, NonConstructibleType_Atomic) {
    WrapInFunction(Assign(Phony(), Call(Source{{12, 34}}, ty.atomic(ty.i32()))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: type is not constructible");
}

TEST_F(ResolverValueConstructorValidationTest, NonConstructibleType_AtomicArray) {
    WrapInFunction(Assign(Phony(), Call(Source{{12, 34}}, ty.array(ty.atomic(ty.i32()), 4_i))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array constructor has non-constructible element type");
}

TEST_F(ResolverValueConstructorValidationTest, NonConstructibleType_AtomicStructMember) {
    auto* str = Structure("S", Vector{Member("a", ty.atomic(ty.i32()))});
    WrapInFunction(Assign(Phony(), Call(Source{{12, 34}}, ty.Of(str))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: structure constructor has non-constructible type");
}

TEST_F(ResolverValueConstructorValidationTest, NonConstructibleType_Sampler) {
    WrapInFunction(
        Assign(Phony(), Call(Source{{12, 34}}, ty.sampler(core::type::SamplerKind::kSampler))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: type is not constructible");
}

TEST_F(ResolverValueConstructorValidationTest, BuilinTypeConstructorAsStatement) {
    WrapInFunction(CallStmt(Call<vec2<f32>>(Source{{12, 34}}, 1_f, 2_f)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: value constructor evaluated but not used");
}

TEST_F(ResolverValueConstructorValidationTest, StructConstructorAsStatement) {
    Structure("S", Vector{Member("m", ty.i32())});
    WrapInFunction(CallStmt(Call(Source{{12, 34}}, "S", 1_a)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: value constructor evaluated but not used");
}

TEST_F(ResolverValueConstructorValidationTest, AliasConstructorAsStatement) {
    Alias("A", ty.i32());
    WrapInFunction(CallStmt(Call(Source{{12, 34}}, "A", 1_i)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: value constructor evaluated but not used");
}

TEST_F(ResolverValueConstructorValidationTest, BuilinTypeConversionAsStatement) {
    WrapInFunction(CallStmt(Call(Source{{12, 34}}, ty.f32(), 1_i)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: value conversion evaluated but not used");
}

TEST_F(ResolverValueConstructorValidationTest, AliasConversionAsStatement) {
    Alias("A", ty.i32());
    WrapInFunction(CallStmt(Call(Source{{12, 34}}, "A", 1_f)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: value conversion evaluated but not used");
}

}  // namespace
}  // namespace tint::resolver
