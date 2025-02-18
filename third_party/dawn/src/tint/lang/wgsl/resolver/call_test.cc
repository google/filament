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
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

template <typename T, int ID = 0>
using alias = builder::alias<T, ID>;

template <typename T>
using alias1 = builder::alias1<T>;

template <typename T>
using alias2 = builder::alias2<T>;

template <typename T>
using alias3 = builder::alias3<T>;

using ResolverCallTest = ResolverTest;

struct Params {
    builder::ast_expr_from_double_func_ptr create_value;
    builder::ast_type_func_ptr create_type;
};

template <typename T>
constexpr Params ParamsFor() {
    return Params{builder::DataType<T>::ExprFromDouble, builder::DataType<T>::AST};
}

static constexpr Params all_param_types[] = {
    ParamsFor<bool>(),         //
    ParamsFor<u32>(),          //
    ParamsFor<i32>(),          //
    ParamsFor<f32>(),          //
    ParamsFor<f16>(),          //
    ParamsFor<vec3<bool>>(),   //
    ParamsFor<vec3<i32>>(),    //
    ParamsFor<vec3<u32>>(),    //
    ParamsFor<vec3<f32>>(),    //
    ParamsFor<mat3x3<f32>>(),  //
    ParamsFor<mat2x3<f32>>(),  //
    ParamsFor<mat3x2<f32>>()   //
};

TEST_F(ResolverCallTest, Valid) {
    Enable(wgsl::Extension::kF16);

    Vector<const ast::Parameter*, 4> params;
    Vector<const ast::Expression*, 4> args;
    for (auto& p : all_param_types) {
        params.Push(Param(Sym(), p.create_type(*this)));
        args.Push(p.create_value(*this, 0));
    }

    auto* func = Func("foo", std::move(params), ty.f32(), Vector{Return(1.23_f)});
    auto* call_expr = Call("foo", std::move(args));
    WrapInFunction(call_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(call_expr);
    EXPECT_NE(call, nullptr);
    EXPECT_EQ(call->Target(), Sem().Get(func));
}

TEST_F(ResolverCallTest, OutOfOrder) {
    auto* call_expr = Call("b");
    Func("a", tint::Empty, ty.void_(), Vector{CallStmt(call_expr)});
    auto* b = Func("b", tint::Empty, ty.void_(), tint::Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(call_expr);
    EXPECT_NE(call, nullptr);
    EXPECT_EQ(call->Target(), Sem().Get(b));
}

}  // namespace
}  // namespace tint::resolver
