// Copyright 2023 The Dawn & Tint Authors
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
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::resolver {
namespace {

enum class Def {
    kAccess,
    kAddressSpace,
    kBuiltinFunction,
    kBuiltinType,
    kFunction,
    kParameter,
    kStruct,
    kTexelFormat,
    kTypeAlias,
    kVariable,
    kTextureFilterable,
    kSamplerFiltering,
};

std::ostream& operator<<(std::ostream& out, Def def) {
    switch (def) {
        case Def::kAccess:
            return out << "Def::kAccess";
        case Def::kAddressSpace:
            return out << "Def::kAddressSpace";
        case Def::kBuiltinFunction:
            return out << "Def::kBuiltinFunction";
        case Def::kBuiltinType:
            return out << "Def::kBuiltinType";
        case Def::kFunction:
            return out << "Def::kFunction";
        case Def::kParameter:
            return out << "Def::kParameter";
        case Def::kStruct:
            return out << "Def::kStruct";
        case Def::kTexelFormat:
            return out << "Def::kTexelFormat";
        case Def::kTypeAlias:
            return out << "Def::kTypeAlias";
        case Def::kVariable:
            return out << "Def::kVariable";
        case Def::kTextureFilterable:
            return out << "Def::kTextureFilterable";
        case Def::kSamplerFiltering:
            return out << "Def::kSamplerFiltering";
    }
    return out << "<unknown>";
}

enum class Use {
    kAccess,
    kAddressSpace,
    kBinaryOp,
    kCallExpr,
    kCallStmt,
    kFunctionReturnType,
    kMemberType,
    kTexelFormat,
    kValueExpression,
    kVariableType,
    kUnaryOp,
    kTextureFilterable,
    kSamplerFiltering,
};

std::ostream& operator<<(std::ostream& out, Use use) {
    switch (use) {
        case Use::kAccess:
            return out << "Use::kAccess";
        case Use::kAddressSpace:
            return out << "Use::kAddressSpace";
        case Use::kBinaryOp:
            return out << "Use::kBinaryOp";
        case Use::kCallExpr:
            return out << "Use::kCallExpr";
        case Use::kCallStmt:
            return out << "Use::kCallStmt";
        case Use::kFunctionReturnType:
            return out << "Use::kFunctionReturnType";
        case Use::kMemberType:
            return out << "Use::kMemberType";
        case Use::kTexelFormat:
            return out << "Use::kTexelFormat";
        case Use::kValueExpression:
            return out << "Use::kValueExpression";
        case Use::kVariableType:
            return out << "Use::kVariableType";
        case Use::kUnaryOp:
            return out << "Use::kUnaryOp";
        case Use::kTextureFilterable:
            return out << "Use::kTextureFilterable";
        case Use::kSamplerFiltering:
            return out << "Use::kSamplerFiltering";
    }
    return out << "<unknown>";
}

struct Case {
    Def def;
    Use use;
    const char* error;
};

std::ostream& operator<<(std::ostream& out, Case c) {
    return out << "{" << c.def << ", " << c.use << "}";
}

static const char* kPass = "<pass>";

static const Source kDefSource{Source::Range{{1, 2}, {3, 4}}};
static const Source kUseSource{Source::Range{{5, 6}, {7, 8}}};

using ResolverExpressionKindTest = ResolverTestWithParam<Case>;

TEST_P(ResolverExpressionKindTest, Test) {
    Symbol sym;
    std::function<void(const sem::Expression*)> check_expr;

    Vector<const ast::Parameter*, 2> fn_params;
    Vector<const ast::Statement*, 2> fn_stmts;
    Vector<const ast::Attribute*, 2> fn_attrs;

    switch (GetParam().def) {
        case Def::kAccess: {
            sym = Sym("write");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<core::Access>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), core::Access::kWrite);
            };
            break;
        }
        case Def::kAddressSpace: {
            sym = Sym("workgroup");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<core::AddressSpace>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), core::AddressSpace::kWorkgroup);
            };
            break;
        }
        case Def::kBuiltinFunction: {
            sym = Sym("workgroupBarrier");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* fn_expr = expr->As<sem::BuiltinEnumExpression<wgsl::BuiltinFn>>();
                ASSERT_NE(fn_expr, nullptr);
                EXPECT_EQ(fn_expr->Value(), wgsl::BuiltinFn::kWorkgroupBarrier);
            };
            break;
        }
        case Def::kBuiltinType: {
            sym = Sym("vec4f");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* ty_expr = expr->As<sem::TypeExpression>();
                ASSERT_NE(ty_expr, nullptr);
                EXPECT_TRUE(ty_expr->Type()->Is<core::type::Vector>());
            };
            break;
        }
        case Def::kFunction: {
            sym = Sym("FUNCTION");
            auto* fn = Func(kDefSource, sym, tint::Empty, ty.i32(), Return(1_i));
            check_expr = [fn](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* fn_expr = expr->As<sem::FunctionExpression>();
                ASSERT_NE(fn_expr, nullptr);
                EXPECT_EQ(fn_expr->Function()->Declaration(), fn);
            };
            break;
        }
        case Def::kParameter: {
            sym = Sym("PARAMETER");
            auto* param = Param(kDefSource, sym, ty.i32());
            fn_params.Push(param);
            check_expr = [param](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* user = expr->As<sem::VariableUser>();
                ASSERT_NE(user, nullptr);
                EXPECT_EQ(user->Variable()->Declaration(), param);
            };
            break;
        }
        case Def::kStruct: {
            sym = Sym("STRUCT");
            auto* s = Structure(kDefSource, sym, Vector{Member("m", ty.i32())});
            check_expr = [s](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* ty_expr = expr->As<sem::TypeExpression>();
                ASSERT_NE(ty_expr, nullptr);
                auto* got = ty_expr->Type()->As<sem::Struct>();
                ASSERT_NE(got, nullptr);
                EXPECT_EQ(got->Declaration(), s);
            };
            break;
        }
        case Def::kTexelFormat: {
            sym = Sym("rgba8unorm");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<core::TexelFormat>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), core::TexelFormat::kRgba8Unorm);
            };
            break;
        }
        case Def::kTypeAlias: {
            sym = Sym("ALIAS");
            Alias(kDefSource, sym, ty.i32());
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* ty_expr = expr->As<sem::TypeExpression>();
                ASSERT_NE(ty_expr, nullptr);
                EXPECT_TRUE(ty_expr->Type()->Is<core::type::I32>());
            };
            break;
        }
        case Def::kVariable: {
            sym = Sym("VARIABLE");
            auto* c = GlobalConst(kDefSource, sym, Expr(1_i));
            check_expr = [c](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* var_expr = expr->As<sem::VariableUser>();
                ASSERT_NE(var_expr, nullptr);
                EXPECT_EQ(var_expr->Variable()->Declaration(), c);
            };
            break;
        }
        case Def::kTextureFilterable: {
            sym = Sym("filterable");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<core::TextureFilterable>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), core::TextureFilterable::kFilterable);
            };
            break;
        }
        case Def::kSamplerFiltering: {
            sym = Sym("filtering");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<core::SamplerFiltering>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), core::SamplerFiltering::kFiltering);
            };
            break;
        }
    }

    auto* ident = Ident(kUseSource, sym);
    auto* expr = Expr(ident);
    switch (GetParam().use) {
        case Use::kAccess:
            GlobalVar("v", ty.AsType("texture_storage_2d", "rgba8unorm", expr), Group(0_u),
                      Binding(0_u));
            break;
        case Use::kAddressSpace:
            Func(Symbols().New(), Vector{Param("p", ty.AsType("ptr", expr, ty.f32()))}, ty.void_(),
                 tint::Empty);
            break;
        case Use::kCallExpr:
            fn_stmts.Push(Decl(Var("v", Call(expr))));
            break;
        case Use::kCallStmt:
            fn_stmts.Push(CallStmt(Call(expr)));
            break;
        case Use::kBinaryOp:
            fn_stmts.Push(Decl(Var("v", Mul(1_a, expr))));
            break;
        case Use::kFunctionReturnType:
            Func(Symbols().New(), tint::Empty, ty.AsType(expr), Return(Call(sym)));
            break;
        case Use::kMemberType:
            Structure(Symbols().New(), Vector{Member("m", ty.AsType(expr))});
            break;
        case Use::kTexelFormat:
            GlobalVar(Symbols().New(), ty.AsType("texture_storage_2d", ty.AsType(expr), "write"),
                      Group(0_u), Binding(0_u));
            break;
        case Use::kValueExpression:
            fn_stmts.Push(Decl(Var("v", expr)));
            break;
        case Use::kVariableType:
            fn_stmts.Push(Decl(Var("v", ty.AsType(expr))));
            break;
        case Use::kUnaryOp:
            fn_stmts.Push(Assign(Phony(), Negation(expr)));
            break;
        case Use::kTextureFilterable:
            GlobalVar(Symbols().New(), ty.AsType("texture_2d", ty.f32(), ty.AsType(expr)),
                      Group(0_u), Binding(0_u));
            break;
        case Use::kSamplerFiltering:
            GlobalVar(Symbols().New(), ty.AsType("sampler", ty.AsType(expr)), Group(0_u),
                      Binding(0_u));
            break;
    }

    if (!fn_params.IsEmpty() || !fn_stmts.IsEmpty()) {
        Func(Symbols().New(), std::move(fn_params), ty.void_(), std::move(fn_stmts),
             std::move(fn_attrs));
    }

    if (GetParam().error == kPass) {
        EXPECT_TRUE(r()->Resolve());
        EXPECT_EQ(r()->error(), "");
        check_expr(Sem().Get(expr));
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), GetParam().error);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ,
    ResolverExpressionKindTest,
    testing::ValuesIn(std::vector<Case>{
        {Def::kAccess, Use::kAccess, kPass},
        {Def::kAccess, Use::kAddressSpace,
         R"(5:6 error: cannot use access enumerant 'write' as address space)"},
        {Def::kAccess, Use::kBinaryOp,
         R"(5:6 error: cannot use access enumerant 'write' as value)"},
        {Def::kAccess, Use::kCallExpr,
         R"(5:6 error: cannot use access enumerant 'write' as call target)"},
        {Def::kAccess, Use::kCallStmt,
         R"(5:6 error: cannot use access enumerant 'write' as call target)"},
        {Def::kAccess, Use::kFunctionReturnType,
         R"(5:6 error: cannot use access enumerant 'write' as type)"},
        {Def::kAccess, Use::kMemberType,
         R"(5:6 error: cannot use access enumerant 'write' as type)"},
        {Def::kAccess, Use::kTexelFormat,
         R"(5:6 error: cannot use access enumerant 'write' as texel format)"},
        {Def::kAccess, Use::kValueExpression,
         R"(5:6 error: cannot use access enumerant 'write' as value)"},
        {Def::kAccess, Use::kVariableType,
         R"(5:6 error: cannot use access enumerant 'write' as type)"},
        {Def::kAccess, Use::kUnaryOp, R"(5:6 error: cannot use access enumerant 'write' as value)"},
        {Def::kAccess, Use::kTextureFilterable,
         R"(5:6 error: cannot use access enumerant 'write' as texture filterable)"},
        {Def::kAccess, Use::kSamplerFiltering,
         R"(5:6 error: cannot use access enumerant 'write' as sampler filtering)"},

        {Def::kAddressSpace, Use::kAccess,
         R"(5:6 error: cannot use address space 'workgroup' as access)"},
        {Def::kAddressSpace, Use::kAddressSpace, kPass},
        {Def::kAddressSpace, Use::kBinaryOp,
         R"(5:6 error: cannot use address space 'workgroup' as value)"},
        {Def::kAddressSpace, Use::kCallExpr,
         R"(5:6 error: cannot use address space 'workgroup' as call target)"},
        {Def::kAddressSpace, Use::kCallStmt,
         R"(5:6 error: cannot use address space 'workgroup' as call target)"},
        {Def::kAddressSpace, Use::kFunctionReturnType,
         R"(5:6 error: cannot use address space 'workgroup' as type)"},
        {Def::kAddressSpace, Use::kMemberType,
         R"(5:6 error: cannot use address space 'workgroup' as type)"},
        {Def::kAddressSpace, Use::kTexelFormat,
         R"(5:6 error: cannot use address space 'workgroup' as texel format)"},
        {Def::kAddressSpace, Use::kValueExpression,
         R"(5:6 error: cannot use address space 'workgroup' as value)"},
        {Def::kAddressSpace, Use::kVariableType,
         R"(5:6 error: cannot use address space 'workgroup' as type)"},
        {Def::kAddressSpace, Use::kUnaryOp,
         R"(5:6 error: cannot use address space 'workgroup' as value)"},
        {Def::kAddressSpace, Use::kTextureFilterable,
         R"(5:6 error: cannot use address space 'workgroup' as texture filterable)"},
        {Def::kAddressSpace, Use::kSamplerFiltering,
         R"(5:6 error: cannot use address space 'workgroup' as sampler filtering)"},

        {Def::kBuiltinFunction, Use::kAccess,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as access)"},
        {Def::kBuiltinFunction, Use::kAddressSpace,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as address space)"},
        {Def::kBuiltinFunction, Use::kBinaryOp,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as value
7:8 note: are you missing '()'?)"},
        {Def::kBuiltinFunction, Use::kCallStmt, kPass},
        {Def::kBuiltinFunction, Use::kFunctionReturnType,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as type)"},
        {Def::kBuiltinFunction, Use::kMemberType,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as type)"},
        {Def::kBuiltinFunction, Use::kTexelFormat,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as texel format)"},
        {Def::kBuiltinFunction, Use::kValueExpression,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as value
7:8 note: are you missing '()'?)"},
        {Def::kBuiltinFunction, Use::kVariableType,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as type)"},
        {Def::kBuiltinFunction, Use::kUnaryOp,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as value
7:8 note: are you missing '()'?)"},
        {Def::kBuiltinFunction, Use::kTextureFilterable,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as texture filterable)"},
        {Def::kBuiltinFunction, Use::kSamplerFiltering,
         R"(5:6 error: cannot use builtin function 'workgroupBarrier' as sampler filtering)"},

        {Def::kBuiltinType, Use::kAccess, R"(5:6 error: cannot use type 'vec4<f32>' as access)"},
        {Def::kBuiltinType, Use::kAddressSpace,
         R"(5:6 error: cannot use type 'vec4<f32>' as address space)"},
        {Def::kBuiltinType, Use::kBinaryOp,
         R"(5:6 error: cannot use type 'vec4<f32>' as value
7:8 note: are you missing '()'?)"},
        {Def::kBuiltinType, Use::kCallExpr, kPass},
        {Def::kBuiltinType, Use::kFunctionReturnType, kPass},
        {Def::kBuiltinType, Use::kMemberType, kPass},
        {Def::kBuiltinType, Use::kTexelFormat,
         R"(5:6 error: cannot use type 'vec4<f32>' as texel format)"},
        {Def::kBuiltinType, Use::kValueExpression,
         R"(5:6 error: cannot use type 'vec4<f32>' as value
7:8 note: are you missing '()'?)"},
        {Def::kBuiltinType, Use::kVariableType, kPass},
        {Def::kBuiltinType, Use::kUnaryOp,
         R"(5:6 error: cannot use type 'vec4<f32>' as value
7:8 note: are you missing '()'?)"},
        {Def::kBuiltinType, Use::kTextureFilterable,
         R"(5:6 error: cannot use type 'vec4<f32>' as texture filterable)"},
        {Def::kBuiltinType, Use::kSamplerFiltering,
         R"(5:6 error: cannot use type 'vec4<f32>' as sampler filtering)"},

        {Def::kFunction, Use::kAccess, R"(5:6 error: cannot use function 'FUNCTION' as access
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kAddressSpace,
         R"(5:6 error: cannot use function 'FUNCTION' as address space
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kBinaryOp, R"(5:6 error: cannot use function 'FUNCTION' as value
1:2 note: function 'FUNCTION' declared here
7:8 note: are you missing '()'?)"},
        {Def::kFunction, Use::kCallExpr, kPass},
        {Def::kFunction, Use::kCallStmt, kPass},
        {Def::kFunction, Use::kFunctionReturnType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kMemberType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kTexelFormat,
         R"(5:6 error: cannot use function 'FUNCTION' as texel format
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kValueExpression,
         R"(5:6 error: cannot use function 'FUNCTION' as value
1:2 note: function 'FUNCTION' declared here
7:8 note: are you missing '()'?)"},
        {Def::kFunction, Use::kVariableType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kUnaryOp, R"(5:6 error: cannot use function 'FUNCTION' as value
1:2 note: function 'FUNCTION' declared here
7:8 note: are you missing '()'?)"},
        {Def::kFunction, Use::kTextureFilterable,
         R"(5:6 error: cannot use function 'FUNCTION' as texture filterable
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kSamplerFiltering,
         R"(5:6 error: cannot use function 'FUNCTION' as sampler filtering
1:2 note: function 'FUNCTION' declared here)"},

        {Def::kParameter, Use::kBinaryOp, kPass},
        {Def::kParameter, Use::kCallStmt,
         R"(5:6 error: cannot use parameter 'PARAMETER' as call target
1:2 note: parameter 'PARAMETER' declared here)"},
        {Def::kParameter, Use::kCallExpr,
         R"(5:6 error: cannot use parameter 'PARAMETER' as call target
1:2 note: parameter 'PARAMETER' declared here)"},
        {Def::kParameter, Use::kValueExpression, kPass},
        {Def::kParameter, Use::kVariableType,
         R"(5:6 error: cannot use parameter 'PARAMETER' as type
1:2 note: parameter 'PARAMETER' declared here)"},
        {Def::kParameter, Use::kUnaryOp, kPass},

        {Def::kStruct, Use::kAccess, R"(5:6 error: cannot use type 'STRUCT' as access
1:2 note: 'struct STRUCT' declared here)"},
        {Def::kStruct, Use::kAddressSpace,
         R"(5:6 error: cannot use type 'STRUCT' as address space
1:2 note: 'struct STRUCT' declared here)"},
        {Def::kStruct, Use::kBinaryOp, R"(5:6 error: cannot use type 'STRUCT' as value
1:2 note: 'struct STRUCT' declared here
7:8 note: are you missing '()'?)"},
        {Def::kStruct, Use::kFunctionReturnType, kPass},
        {Def::kStruct, Use::kMemberType, kPass},
        {Def::kStruct, Use::kTexelFormat, R"(5:6 error: cannot use type 'STRUCT' as texel format
1:2 note: 'struct STRUCT' declared here)"},
        {Def::kStruct, Use::kValueExpression,
         R"(5:6 error: cannot use type 'STRUCT' as value
1:2 note: 'struct STRUCT' declared here
7:8 note: are you missing '()'?)"},
        {Def::kStruct, Use::kVariableType, kPass},
        {Def::kStruct, Use::kUnaryOp,
         R"(5:6 error: cannot use type 'STRUCT' as value
1:2 note: 'struct STRUCT' declared here
7:8 note: are you missing '()'?)"},
        {Def::kStruct, Use::kTextureFilterable,
         R"(5:6 error: cannot use type 'STRUCT' as texture filterable
1:2 note: 'struct STRUCT' declared here)"},
        {Def::kStruct, Use::kSamplerFiltering,
         R"(5:6 error: cannot use type 'STRUCT' as sampler filtering
1:2 note: 'struct STRUCT' declared here)"},

        {Def::kTexelFormat, Use::kAccess,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as access)"},
        {Def::kTexelFormat, Use::kAddressSpace,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as address space)"},
        {Def::kTexelFormat, Use::kBinaryOp,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as value)"},
        {Def::kTexelFormat, Use::kCallExpr,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as call target)"},
        {Def::kTexelFormat, Use::kCallStmt,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as call target)"},
        {Def::kTexelFormat, Use::kFunctionReturnType,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as type)"},
        {Def::kTexelFormat, Use::kMemberType,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as type)"},
        {Def::kTexelFormat, Use::kTexelFormat, kPass},
        {Def::kTexelFormat, Use::kValueExpression,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as value)"},
        {Def::kTexelFormat, Use::kVariableType,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as type)"},
        {Def::kTexelFormat, Use::kUnaryOp,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as value)"},
        {Def::kTexelFormat, Use::kTextureFilterable,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as texture filterable)"},
        {Def::kTexelFormat, Use::kSamplerFiltering,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as sampler filtering)"},

        {Def::kTypeAlias, Use::kAccess, R"(5:6 error: cannot use type 'i32' as access)"},
        {Def::kTypeAlias, Use::kAddressSpace,
         R"(5:6 error: cannot use type 'i32' as address space)"},
        {Def::kTypeAlias, Use::kBinaryOp,
         R"(5:6 error: cannot use type 'i32' as value
7:8 note: are you missing '()'?)"},
        {Def::kTypeAlias, Use::kCallExpr, kPass},
        {Def::kTypeAlias, Use::kFunctionReturnType, kPass},
        {Def::kTypeAlias, Use::kMemberType, kPass},
        {Def::kTypeAlias, Use::kTexelFormat, R"(5:6 error: cannot use type 'i32' as texel format)"},
        {Def::kTypeAlias, Use::kValueExpression,
         R"(5:6 error: cannot use type 'i32' as value
7:8 note: are you missing '()'?)"},
        {Def::kTypeAlias, Use::kVariableType, kPass},
        {Def::kTypeAlias, Use::kUnaryOp,
         R"(5:6 error: cannot use type 'i32' as value
7:8 note: are you missing '()'?)"},
        {Def::kTypeAlias, Use::kTextureFilterable,
         R"(5:6 error: cannot use type 'i32' as texture filterable)"},
        {Def::kTypeAlias, Use::kSamplerFiltering,
         R"(5:6 error: cannot use type 'i32' as sampler filtering)"},

        {Def::kVariable, Use::kAccess, R"(5:6 error: cannot use 'const VARIABLE' as access
1:2 note: 'const VARIABLE' declared here)"},
        {Def::kVariable, Use::kAddressSpace,
         R"(5:6 error: cannot use 'const VARIABLE' as address space
1:2 note: 'const VARIABLE' declared here)"},
        {Def::kVariable, Use::kBinaryOp, kPass},
        {Def::kVariable, Use::kCallStmt,
         R"(5:6 error: cannot use 'const VARIABLE' as call target
1:2 note: 'const VARIABLE' declared here)"},
        {Def::kVariable, Use::kCallExpr,
         R"(5:6 error: cannot use 'const VARIABLE' as call target
1:2 note: 'const VARIABLE' declared here)"},
        {Def::kVariable, Use::kFunctionReturnType,
         R"(5:6 error: cannot use 'const VARIABLE' as type
1:2 note: 'const VARIABLE' declared here)"},
        {Def::kVariable, Use::kMemberType,
         R"(5:6 error: cannot use 'const VARIABLE' as type
1:2 note: 'const VARIABLE' declared here)"},
        {Def::kVariable, Use::kTexelFormat,
         R"(5:6 error: cannot use 'const VARIABLE' as texel format
1:2 note: 'const VARIABLE' declared here)"},
        {Def::kVariable, Use::kValueExpression, kPass},
        {Def::kVariable, Use::kVariableType,
         R"(5:6 error: cannot use 'const VARIABLE' as type
1:2 note: 'const VARIABLE' declared here)"},
        {Def::kVariable, Use::kUnaryOp, kPass},
        {Def::kVariable, Use::kTextureFilterable,
         R"(5:6 error: cannot use 'const VARIABLE' as texture filterable
1:2 note: 'const VARIABLE' declared here)"},
        {Def::kVariable, Use::kSamplerFiltering,
         R"(5:6 error: cannot use 'const VARIABLE' as sampler filtering
1:2 note: 'const VARIABLE' declared here)"},

        {Def::kTextureFilterable, Use::kAccess,
         R"(5:6 error: cannot use texture filterable 'filterable' as access)"},
        {Def::kTextureFilterable, Use::kAddressSpace,
         R"(5:6 error: cannot use texture filterable 'filterable' as address space)"},
        {Def::kTextureFilterable, Use::kBinaryOp,
         R"(5:6 error: cannot use texture filterable 'filterable' as value)"},
        {Def::kTextureFilterable, Use::kCallStmt,
         R"(5:6 error: cannot use texture filterable 'filterable' as call target)"},
        {Def::kTextureFilterable, Use::kCallExpr,
         R"(5:6 error: cannot use texture filterable 'filterable' as call target)"},
        {Def::kTextureFilterable, Use::kFunctionReturnType,
         R"(5:6 error: cannot use texture filterable 'filterable' as type)"},
        {Def::kTextureFilterable, Use::kMemberType,
         R"(5:6 error: cannot use texture filterable 'filterable' as type)"},
        {Def::kTextureFilterable, Use::kTexelFormat,
         R"(5:6 error: cannot use texture filterable 'filterable' as texel format)"},
        {Def::kTextureFilterable, Use::kValueExpression,
         R"(5:6 error: cannot use texture filterable 'filterable' as value)"},
        {Def::kTextureFilterable, Use::kVariableType,
         R"(5:6 error: cannot use texture filterable 'filterable' as type)"},
        {Def::kTextureFilterable, Use::kUnaryOp,
         R"(5:6 error: cannot use texture filterable 'filterable' as value)"},
        {Def::kTextureFilterable, Use::kTextureFilterable, kPass},
        {Def::kTextureFilterable, Use::kSamplerFiltering,
         R"(5:6 error: cannot use texture filterable 'filterable' as sampler filtering)"},

        {Def::kSamplerFiltering, Use::kAccess,
         R"(5:6 error: cannot use sampler filtering 'filtering' as access)"},
        {Def::kSamplerFiltering, Use::kAddressSpace,
         R"(5:6 error: cannot use sampler filtering 'filtering' as address space)"},
        {Def::kSamplerFiltering, Use::kBinaryOp,
         R"(5:6 error: cannot use sampler filtering 'filtering' as value)"},
        {Def::kSamplerFiltering, Use::kCallStmt,
         R"(5:6 error: cannot use sampler filtering 'filtering' as call target)"},
        {Def::kSamplerFiltering, Use::kCallExpr,
         R"(5:6 error: cannot use sampler filtering 'filtering' as call target)"},
        {Def::kSamplerFiltering, Use::kFunctionReturnType,
         R"(5:6 error: cannot use sampler filtering 'filtering' as type)"},
        {Def::kSamplerFiltering, Use::kMemberType,
         R"(5:6 error: cannot use sampler filtering 'filtering' as type)"},
        {Def::kSamplerFiltering, Use::kTexelFormat,
         R"(5:6 error: cannot use sampler filtering 'filtering' as texel format)"},
        {Def::kSamplerFiltering, Use::kValueExpression,
         R"(5:6 error: cannot use sampler filtering 'filtering' as value)"},
        {Def::kSamplerFiltering, Use::kVariableType,
         R"(5:6 error: cannot use sampler filtering 'filtering' as type)"},
        {Def::kSamplerFiltering, Use::kUnaryOp,
         R"(5:6 error: cannot use sampler filtering 'filtering' as value)"},
        {Def::kSamplerFiltering, Use::kTextureFilterable,
         R"(5:6 error: cannot use sampler filtering 'filtering' as texture filterable)"},
        {Def::kSamplerFiltering, Use::kSamplerFiltering, kPass},
    }));

}  // namespace
}  // namespace tint::resolver
