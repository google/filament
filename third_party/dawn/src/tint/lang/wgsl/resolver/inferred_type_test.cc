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
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/array.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <typename T>
using alias = builder::alias<T>;

struct ResolverInferredTypeTest : public resolver::TestHelper, public testing::Test {};

struct Params {
    // builder::ast_expr_func_ptr_default_arg create_value;
    builder::ast_expr_from_double_func_ptr create_value;
    builder::sem_type_func_ptr create_expected_type;
};

template <typename T>
constexpr Params ParamsFor() {
    // return Params{builder::CreateExprWithDefaultArg<T>(), DataType<T>::Sem};
    return Params{DataType<T>::ExprFromDouble, DataType<T>::Sem};
}

Params all_cases[] = {
    ParamsFor<bool>(),                //
    ParamsFor<u32>(),                 //
    ParamsFor<i32>(),                 //
    ParamsFor<f32>(),                 //
    ParamsFor<vec3<bool>>(),          //
    ParamsFor<vec3<i32>>(),           //
    ParamsFor<vec3<u32>>(),           //
    ParamsFor<vec3<f32>>(),           //
    ParamsFor<mat3x3<f32>>(),         //
    ParamsFor<alias<bool>>(),         //
    ParamsFor<alias<u32>>(),          //
    ParamsFor<alias<i32>>(),          //
    ParamsFor<alias<f32>>(),          //
    ParamsFor<alias<vec3<bool>>>(),   //
    ParamsFor<alias<vec3<i32>>>(),    //
    ParamsFor<alias<vec3<u32>>>(),    //
    ParamsFor<alias<vec3<f32>>>(),    //
    ParamsFor<alias<mat3x3<f32>>>(),  //
};

using ResolverInferredTypeParamTest = ResolverTestWithParam<Params>;

TEST_P(ResolverInferredTypeParamTest, GlobalConst_Pass) {
    auto& params = GetParam();

    auto* expected_type = params.create_expected_type(*this);

    // const a = <type initializer>;
    auto* ctor_expr = params.create_value(*this, 0);
    auto* a = GlobalConst("a", ctor_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(a), expected_type);
}

TEST_P(ResolverInferredTypeParamTest, GlobalVar_Pass) {
    auto& params = GetParam();

    auto* expected_type = params.create_expected_type(*this);

    // var a = <type initializer>;
    auto* ctor_expr = params.create_value(*this, 0);
    auto* var = GlobalVar("a", core::AddressSpace::kPrivate, ctor_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

TEST_P(ResolverInferredTypeParamTest, LocalLet_Pass) {
    auto& params = GetParam();

    auto* expected_type = params.create_expected_type(*this);

    // let a = <type initializer>;
    auto* ctor_expr = params.create_value(*this, 0);
    auto* var = Let("a", ctor_expr);
    WrapInFunction(var);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(var), expected_type);
}

TEST_P(ResolverInferredTypeParamTest, LocalVar_Pass) {
    auto& params = GetParam();

    auto* expected_type = params.create_expected_type(*this);

    // var a = <type initializer>;
    auto* ctor_expr = params.create_value(*this, 0);
    auto* var = Var("a", core::AddressSpace::kFunction, ctor_expr);
    WrapInFunction(var);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

INSTANTIATE_TEST_SUITE_P(ResolverTest, ResolverInferredTypeParamTest, testing::ValuesIn(all_cases));

TEST_F(ResolverInferredTypeTest, InferArray_Pass) {
    auto type = ty.array<u32, 10>();
    auto* expected_type =
        create<sem::Array>(create<core::type::U32>(), create<core::type::ConstantArrayCount>(10u),
                           4u, 4u * 10u, 4u, 4u);

    auto* ctor_expr = Call(type);
    auto* var = Var("a", core::AddressSpace::kFunction, ctor_expr);
    WrapInFunction(var);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

TEST_F(ResolverInferredTypeTest, InferStruct_Pass) {
    auto* member = Member("x", ty.i32());
    auto* str = Structure("S", Vector{member});

    auto* expected_type = create<sem::Struct>(
        str, str->name->symbol,
        Vector{create<sem::StructMember>(member, member->name->symbol, create<core::type::I32>(),
                                         0u, 0u, 0u, 4u, core::IOAttributes{})},
        0u, 4u, 4u);

    auto* ctor_expr = Call(ty.Of(str));

    auto* var = Var("a", core::AddressSpace::kFunction, ctor_expr);
    WrapInFunction(var);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

}  // namespace
}  // namespace tint::resolver
