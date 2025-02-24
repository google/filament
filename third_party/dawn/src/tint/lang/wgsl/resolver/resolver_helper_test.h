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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_RESOLVER_HELPER_TEST_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_RESOLVER_HELPER_TEST_H_

#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <variant>

#include "gtest/gtest.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/sem/array.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/rtti/traits.h"

namespace tint::resolver {

/// Helper class for testing
class TestHelper : public ProgramBuilder {
  public:
    /// Constructor
    TestHelper();

    /// Destructor
    ~TestHelper();

    /// @return a pointer to the Resolver
    Resolver* r() const { return resolver_.get(); }

    /// @return a pointer to the validator
    const Validator* v() const { return resolver_->GetValidatorForTesting(); }

    /// Returns the statement that holds the given expression.
    /// @param expr the ast::Expression
    /// @return the ast::Statement of the ast::Expression, or nullptr if the
    /// expression is not owned by a statement.
    const ast::Statement* StmtOf(const ast::Expression* expr) {
        auto* sem_stmt = Sem().Get(expr)->Stmt();
        return sem_stmt ? sem_stmt->Declaration() : nullptr;
    }

    /// Returns the BlockStatement that holds the given statement.
    /// @param stmt the ast::Statement
    /// @return the ast::BlockStatement that holds the ast::Statement, or nullptr
    /// if the statement is not owned by a BlockStatement.
    const ast::BlockStatement* BlockOf(const ast::Statement* stmt) {
        auto* sem_stmt = Sem().Get(stmt);
        return sem_stmt ? sem_stmt->Block()->Declaration() : nullptr;
    }

    /// Returns the BlockStatement that holds the given expression.
    /// @param expr the ast::Expression
    /// @return the ast::Statement of the ast::Expression, or nullptr if the
    /// expression is not indirectly owned by a BlockStatement.
    const ast::BlockStatement* BlockOf(const ast::Expression* expr) {
        auto* sem_stmt = Sem().Get(expr)->Stmt();
        return sem_stmt ? sem_stmt->Block()->Declaration() : nullptr;
    }

    /// Returns the semantic variable for the given identifier expression.
    /// @param expr the identifier expression
    /// @return the resolved sem::Variable of the identifier, or nullptr if
    /// the expression did not resolve to a variable.
    const sem::Variable* VarOf(const ast::Expression* expr) {
        if (auto* sem = Sem().GetVal(expr)) {
            if (auto* var_user = As<sem::VariableUser>(sem->UnwrapLoad())) {
                return var_user->Variable();
            }
        }
        return nullptr;
    }

    /// Checks that all the users of the given variable are as expected
    /// @param var the variable to check
    /// @param expected_users the expected users of the variable
    /// @return true if all users are as expected
    bool CheckVarUsers(const ast::Variable* var, VectorRef<const ast::Expression*> expected_users) {
        auto var_users = Sem().Get(var)->Users();
        if (var_users.Length() != expected_users.Length()) {
            return false;
        }
        for (size_t i = 0; i < var_users.Length(); i++) {
            if (var_users[i]->Declaration() != expected_users[i]) {
                return false;
            }
        }
        return true;
    }

    /// @param type a type
    /// @returns the name for `type` that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(ast::Type type) { return type->identifier->symbol.Name(); }

    /// @param type a type
    /// @returns the name for `type` that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const core::type::Type* type) { return type->FriendlyName(); }

  protected:
    std::unique_ptr<Resolver> resolver_;
};

class ResolverTest : public TestHelper, public testing::Test {};
using ResolverDeathTest = ResolverTest;

template <typename T>
class ResolverTestWithParam : public TestHelper, public testing::TestWithParam<T> {};

namespace builder {

template <typename TO, int ID = 0>
struct alias {};

template <typename TO>
using alias1 = alias<TO, 1>;

template <typename TO>
using alias2 = alias<TO, 2>;

template <typename TO>
using alias3 = alias<TO, 3>;

/// Scalar represents a scalar value
struct Scalar {
    /// The possible types of a scalar value.
    using Value =
        std::variant<core::i32, core::u32, core::f32, core::f16, core::AInt, core::AFloat, bool>;

    /// Constructor
    /// @param val the value used to construct this Scalar
    template <typename T>
    Scalar(T&& val) : value(std::forward<T>(val)) {}  // NOLINT

    /// @returns the Value
    operator Value&() { return value; }

    /// Equality operator
    /// @param other the other Scalar
    /// @return true if this Scalar is equal to @p other
    bool operator==(const Scalar& other) const { return value == other.value; }

    /// Inequality operator
    /// @param other the other Scalar
    /// @return true if this Scalar is not equal to @p other
    bool operator!=(const Scalar& other) const { return value != other.value; }

    /// The scalar value
    Value value;
};

/// @param out the stream to write to
/// @param s the Scalar
/// @returns @p out so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
STREAM& operator<<(STREAM& out, const Scalar& s) {
    std::visit([&](auto&& v) { out << v; }, s.value);
    return out;
}

/// @return  current variant value in @p s cast to type `T`
template <typename T>
T As(const Scalar& s) {
    return std::visit([](auto&& v) { return static_cast<T>(v); }, s.value);
}

using ast_type_func_ptr = ast::Type (*)(ProgramBuilder& b);
using ast_expr_func_ptr = const ast::Expression* (*)(ProgramBuilder& b, VectorRef<Scalar> args);
using ast_expr_from_double_func_ptr = const ast::Expression* (*)(ProgramBuilder& b, double v);
using sem_type_func_ptr = const core::type::Type* (*)(ProgramBuilder& b);
using type_name_func_ptr = std::string (*)();

struct UnspecializedElementType {};

/// Base template for DataType, specialized below.
template <typename T>
struct DataType {
    /// The element type
    using ElementType = UnspecializedElementType;
};

/// Helper that represents no-type. Returns nullptr for all static methods.
template <>
struct DataType<void> {
    /// The element type
    using ElementType = void;

    /// @return nullptr
    static inline ast::Type AST(ProgramBuilder&) { return {}; }
    /// @return nullptr
    static inline const core::type::Type* Sem(ProgramBuilder&) { return nullptr; }
};

/// Helper for building bool types and expressions
template <>
struct DataType<bool> {
    /// The element type
    using ElementType = bool;

    /// false as bool is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST bool type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.bool_(); }
    /// @param b the ProgramBuilder
    /// @return the semantic bool type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        return b.create<core::type::Bool>();
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the boolean value to init with
    /// @return a new AST expression of the bool type
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> args) {
        return b.Expr(std::get<bool>(args[0].value));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to bool.
    /// @return a new AST expression of the bool type
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "bool"; }
};

/// Helper for building i32 types and expressions
template <>
struct DataType<core::i32> {
    /// The element type
    using ElementType = core::i32;

    /// false as i32 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST i32 type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.i32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic i32 type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        return b.create<core::type::I32>();
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the i32 value to init with
    /// @return a new AST i32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> args) {
        return b.Expr(std::get<core::i32>(args[0].value));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to i32.
    /// @return a new AST i32 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "i32"; }
};

/// Helper for building u32 types and expressions
template <>
struct DataType<core::u32> {
    /// The element type
    using ElementType = core::u32;

    /// false as u32 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST u32 type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.u32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic u32 type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        return b.create<core::type::U32>();
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the u32 value to init with
    /// @return a new AST u32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> args) {
        return b.Expr(std::get<core::u32>(args[0].value));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to u32.
    /// @return a new AST u32 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "u32"; }
};

/// Helper for building f32 types and expressions
template <>
struct DataType<core::f32> {
    /// The element type
    using ElementType = core::f32;

    /// false as f32 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST f32 type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.f32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic f32 type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        return b.create<core::type::F32>();
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the f32 value to init with
    /// @return a new AST f32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> args) {
        return b.Expr(std::get<core::f32>(args[0].value));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to f32.
    /// @return a new AST f32 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<core::f32>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "f32"; }
};

/// Helper for building f16 types and expressions
template <>
struct DataType<core::f16> {
    /// The element type
    using ElementType = core::f16;

    /// false as f16 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST f16 type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.f16(); }
    /// @param b the ProgramBuilder
    /// @return the semantic f16 type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        return b.create<core::type::F16>();
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the f16 value to init with
    /// @return a new AST f16 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> args) {
        return b.Expr(std::get<core::f16>(args[0].value));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to f16.
    /// @return a new AST f16 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "f16"; }
};

/// Helper for building abstract float types and expressions
template <>
struct DataType<core::AFloat> {
    /// The element type
    using ElementType = core::AFloat;

    /// false as AFloat is not a composite type
    static constexpr bool is_composite = false;

    /// @returns nullptr, as abstract floats are un-typeable
    static inline ast::Type AST(ProgramBuilder&) { return {}; }
    /// @param b the ProgramBuilder
    /// @return the semantic abstract-float type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        return b.create<core::type::AbstractFloat>();
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the abstract-float value to init with
    /// @return a new AST abstract-float literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> args) {
        return b.Expr(std::get<core::AFloat>(args[0].value));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to AFloat.
    /// @return a new AST abstract-float literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "abstract-float"; }
};

/// Helper for building abstract integer types and expressions
template <>
struct DataType<core::AInt> {
    /// The element type
    using ElementType = core::AInt;

    /// false as AFloat is not a composite type
    static constexpr bool is_composite = false;

    /// @returns nullptr, as abstract integers are un-typeable
    static inline ast::Type AST(ProgramBuilder&) { return {}; }
    /// @param b the ProgramBuilder
    /// @return the semantic abstract-int type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        return b.create<core::type::AbstractInt>();
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the abstract-int value to init with
    /// @return a new AST abstract-int literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> args) {
        return b.Expr(std::get<core::AInt>(args[0].value));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to AInt.
    /// @return a new AST abstract-int literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "abstract-int"; }
};

/// Helper for building vector types and expressions
template <uint32_t N, typename T>
struct DataType<core::fluent_types::vec<N, T>> {
    /// The element type
    using ElementType = T;

    /// true as vectors are a composite type
    static constexpr bool is_composite = true;

    /// @param b the ProgramBuilder
    /// @return a new AST vector type
    static inline ast::Type AST(ProgramBuilder& b) {
        if (ast::IsInferOrAbstract<T>) {
            return b.ty.vec<core::fluent_types::Infer, N>();
        } else {
            return b.ty.vec(DataType<T>::AST(b), N);
        }
    }
    /// @param b the ProgramBuilder
    /// @return the semantic vector type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        return b.create<core::type::Vector>(DataType<T>::Sem(b), N);
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// @return a new AST vector value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> args) {
        return b.Call(AST(b), ExprArgs(b, std::move(args)));
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// @return the list of expressions that are used to construct the vector
    static inline auto ExprArgs(ProgramBuilder& b, VectorRef<Scalar> args) {
        const bool one_value = args.Length() == 1;
        Vector<const ast::Expression*, N> r;
        for (size_t i = 0; i < N; ++i) {
            r.Push(DataType<T>::Expr(b, Vector<Scalar, 1>{one_value ? args[0] : args[i]}));
        }
        return r;
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST vector value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() {
        return "vec" + std::to_string(N) + "<" + DataType<T>::Name() + ">";
    }
};

/// Helper for building matrix types and expressions
template <uint32_t N, uint32_t M, typename T>
struct DataType<core::fluent_types::mat<N, M, T>> {
    /// The element type
    using ElementType = T;

    /// true as matrices are a composite type
    static constexpr bool is_composite = true;

    /// @param b the ProgramBuilder
    /// @return a new AST matrix type
    static inline ast::Type AST(ProgramBuilder& b) {
        if (ast::IsInferOrAbstract<T>) {
            return b.ty.mat<core::fluent_types::Infer, N, M>();
        } else {
            return b.ty.mat(DataType<T>::AST(b), N, M);
        }
    }
    /// @param b the ProgramBuilder
    /// @return the semantic matrix type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        auto* column_type = b.create<core::type::Vector>(DataType<T>::Sem(b), M);
        return b.create<core::type::Matrix>(column_type, N);
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N*M with values of type T to initialize with
    /// @return a new AST matrix value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> args) {
        return b.Call(AST(b), ExprArgs(b, std::move(args)));
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N*M with values of type T to initialize with
    /// @return a new AST matrix value expression
    static inline auto ExprArgs(ProgramBuilder& b, VectorRef<Scalar> args) {
        const bool one_value = args.Length() == 1;
        size_t next = 0;
        Vector<const ast::Expression*, N> r;
        for (uint32_t i = 0; i < N; ++i) {
            if (one_value) {
                r.Push(
                    DataType<core::fluent_types::vec<M, T>>::Expr(b, Vector<Scalar, 1>{args[0]}));
            } else {
                Vector<Scalar, M> v;
                for (size_t j = 0; j < M; ++j) {
                    v.Push(args[next++]);
                }
                r.Push(DataType<core::fluent_types::vec<M, T>>::Expr(b, std::move(v)));
            }
        }
        return r;
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST matrix value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() {
        return "mat" + std::to_string(N) + "x" + std::to_string(M) + "<" + DataType<T>::Name() +
               ">";
    }
};

/// Helper for building alias types and expressions
template <typename T, int ID>
struct DataType<alias<T, ID>> {
    /// The element type
    using ElementType = typename DataType<T>::ElementType;

    /// true if the aliased type is a composite type
    static constexpr bool is_composite = DataType<T>::is_composite;

    /// @param b the ProgramBuilder
    /// @return a new AST alias type
    static inline ast::Type AST(ProgramBuilder& b) {
        auto name = b.Symbols().Register("alias_" + std::to_string(ID));
        if (!b.AST().LookupType(name)) {
            auto type = DataType<T>::AST(b);
            b.AST().AddTypeDecl(b.ty.alias(name, type));
        }
        return b.ty(name);
    }

    /// @param b the ProgramBuilder
    /// @return the semantic aliased type
    static inline const core::type::Type* Sem(ProgramBuilder& b) { return DataType<T>::Sem(b); }

    /// @param b the ProgramBuilder
    /// @param args the value nested elements will be initialized with
    /// @return a new AST expression of the alias type
    template <bool IS_COMPOSITE = is_composite>
    static inline tint::traits::EnableIf<!IS_COMPOSITE, const ast::Expression*> Expr(
        ProgramBuilder& b,
        VectorRef<Scalar> args) {
        // Cast
        return b.Call(AST(b), DataType<T>::Expr(b, std::move(args)));
    }

    /// @param b the ProgramBuilder
    /// @param args the value nested elements will be initialized with
    /// @return a new AST expression of the alias type
    template <bool IS_COMPOSITE = is_composite>
    static inline tint::traits::EnableIf<IS_COMPOSITE, const ast::Expression*> Expr(
        ProgramBuilder& b,
        VectorRef<Scalar> args) {
        // Construct
        return b.Call(AST(b), DataType<T>::ExprArgs(b, std::move(args)));
    }

    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST expression of the alias type
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }

    /// @returns the WGSL name for the type
    static inline std::string Name() { return "alias_" + std::to_string(ID); }
};

/// Helper for building pointer types and expressions
template <typename T>
struct DataType<
    core::fluent_types::ptr<core::AddressSpace::kPrivate, T, core::Access::kUndefined>> {
    /// The element type
    using ElementType = typename DataType<T>::ElementType;

    /// true if the pointer type is a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST alias type
    static inline ast::Type AST(ProgramBuilder& b) {
        return b.ty.ptr<core::AddressSpace::kPrivate, T>();
    }

    /// @param b the ProgramBuilder
    /// @return the semantic aliased type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        return b.create<core::type::Pointer>(core::AddressSpace::kPrivate, DataType<T>::Sem(b),
                                             core::Access::kReadWrite);
    }

    /// @param b the ProgramBuilder
    /// @return a new AST expression of the pointer type
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> /*unused*/) {
        auto sym = b.Symbols().New("global_for_ptr");
        b.GlobalVar(sym, DataType<T>::AST(b), core::AddressSpace::kPrivate);
        return b.AddressOf(sym);
    }

    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST expression of the pointer type
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }

    /// @returns the WGSL name for the type
    static inline std::string Name() { return "ptr<" + DataType<T>::Name() + ">"; }
};

/// Helper for building array types and expressions
template <typename T, uint32_t N>
struct DataType<core::fluent_types::array<T, N>> {
    /// The element type
    using ElementType = typename DataType<T>::ElementType;

    /// true as arrays are a composite type
    static constexpr bool is_composite = true;

    /// @param b the ProgramBuilder
    /// @return a new AST array type
    static inline ast::Type AST(ProgramBuilder& b) {
        if (auto ast = DataType<T>::AST(b)) {
            return b.ty.array(ast, core::u32(N));
        }
        return b.ty.array<core::fluent_types::Infer>();
    }
    /// @param b the ProgramBuilder
    /// @return the semantic array type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        auto* el = DataType<T>::Sem(b);
        const core::type::ArrayCount* count = nullptr;
        if (N == 0) {
            count = b.create<core::type::RuntimeArrayCount>();
        } else {
            count = b.create<core::type::ConstantArrayCount>(N);
        }
        return b.create<sem::Array>(
            /* element */ el,
            /* count */ count,
            /* align */ el->Align(),
            /* size */ N * el->Size(),
            /* stride */ el->Align(),
            /* implicit_stride */ el->Align());
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// with
    /// @return a new AST array value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, VectorRef<Scalar> args) {
        return b.Call(AST(b), ExprArgs(b, std::move(args)));
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// @return the list of expressions that are used to construct the array
    static inline auto ExprArgs(ProgramBuilder& b, VectorRef<Scalar> args) {
        const bool one_value = args.Length() == 1;
        Vector<const ast::Expression*, N> r;
        for (uint32_t i = 0; i < N; i++) {
            r.Push(DataType<T>::Expr(b, Vector<Scalar, 1>{one_value ? args[0] : args[i]}));
        }
        return r;
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST array value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() {
        return "array<" + DataType<T>::Name() + ", " + std::to_string(N) + ">";
    }
};

/// Helper for building atomic types and expressions
template <typename T>
struct DataType<core::fluent_types::atomic<T>> {
    /// The element type
    using ElementType = typename DataType<T>::ElementType;

    /// true as atomics are a composite type
    static constexpr bool is_composite = true;

    /// @param b the ProgramBuilder
    /// @return a new AST atomic type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.atomic(DataType<T>::AST(b)); }

    /// @param b the ProgramBuilder
    /// @return the semantic atomic type
    static inline const core::type::Type* Sem(ProgramBuilder& b) {
        return b.Types().atomic(DataType<T>::Sem(b));
    }

    /// @returns the WGSL name for the type
    static inline std::string Name() { return "atomic<" + DataType<T>::Name() + ">"; }
};

/// Struct of all creation pointer types
struct CreatePtrs {
    /// ast node type create function
    ast_type_func_ptr ast;
    /// ast expression type create function
    ast_expr_func_ptr expr;
    /// ast expression type create function from double arg
    ast_expr_from_double_func_ptr expr_from_double;
    /// sem type create function
    sem_type_func_ptr sem;
    /// type name function
    type_name_func_ptr name;
};

/// @param o the std::ostream to write to
/// @param ptrs the CreatePtrs
/// @return the std::ostream so calls can be chained
inline std::ostream& operator<<(std::ostream& o, const CreatePtrs& ptrs) {
    return o << (ptrs.name ? ptrs.name() : "<unknown>");
}

/// Returns a CreatePtrs struct instance with all creation pointer types for
/// type `T`
template <typename T>
constexpr CreatePtrs CreatePtrsFor() {
    return {DataType<T>::AST, DataType<T>::Expr, DataType<T>::ExprFromDouble, DataType<T>::Sem,
            DataType<T>::Name};
}

/// True if DataType<T> is specialized for T, false otherwise.
template <typename T>
const bool IsDataTypeSpecializedFor =
    !std::is_same_v<typename DataType<T>::ElementType, UnspecializedElementType>;

/// Value is used to create Values with a Scalar vector initializer.
struct Value {
    /// Creates a Value for type T initialized with `args`
    /// @param args the scalar args
    /// @returns Value
    template <typename T>
    static Value Create(VectorRef<Scalar> args) {
        static_assert(IsDataTypeSpecializedFor<T>, "No DataType<T> specialization exists");
        using EL_TY = typename builder::DataType<T>::ElementType;
        return Value{
            std::move(args),          //
            CreatePtrsFor<T>(),       //
            core::IsAbstract<EL_TY>,  //
            core::IsIntegral<EL_TY>,  //
            core::FriendlyName<EL_TY>(),
        };
    }

    /// Creates an `ast::Expression` for the type T passing in previously stored args
    /// @param b the ProgramBuilder
    /// @returns an expression node
    const ast::Expression* Expr(ProgramBuilder& b) const { return (*create_ptrs.expr)(b, args); }

    /// Prints this value to the output stream
    /// @param o the output stream
    /// @returns input argument `o`
    std::ostream& Print(std::ostream& o) const {
        o << type_name << "(";
        for (auto& a : args) {
            o << a;
            if (&a != &args.Back()) {
                o << ", ";
            }
        }
        o << ")";
        return o;
    }

    /// The arguments used to construct the value
    Vector<Scalar, 4> args;
    /// CreatePtrs for value's type used to create an expression with `args`
    builder::CreatePtrs create_ptrs;
    /// True if the element type is abstract
    bool is_abstract = false;
    /// True if the element type is an integer
    bool is_integral = false;
    /// The name of the type.
    const char* type_name = "<invalid>";
};

/// Prints Value to ostream
inline std::ostream& operator<<(std::ostream& o, const Value& value) {
    return value.Print(o);
}

/// True if T is Value, false otherwise
template <typename T>
constexpr bool IsValue = std::is_same_v<T, Value>;

/// Creates a Value of DataType<T> from a scalar `v`
template <typename T>
Value Val(T v) {
    static_assert(tint::traits::IsTypeIn<T, Scalar::Value>, "v must be a Number of bool");
    return Value::Create<T>(Vector<Scalar, 1>{v});
}

/// Creates a Value of DataType<vec<N, T>> from N scalar `args`
template <typename... Ts>
Value Vec(Ts... args) {
    using FirstT = std::tuple_element_t<0, std::tuple<Ts...>>;
    static_assert(sizeof...(args) >= 2 && sizeof...(args) <= 4, "Invalid vector size");
    static_assert(std::conjunction_v<std::is_same<FirstT, Ts>...>,
                  "Vector args must all be the same type");
    constexpr size_t N = sizeof...(args);
    Vector<Scalar, sizeof...(args)> v{args...};
    return Value::Create<core::fluent_types::vec<N, FirstT>>(std::move(v));
}

/// Creates a Value of DataType<array<N, T>> from N scalar `args`
template <typename... Ts>
Value Array(Ts... args) {
    using FirstT = std::tuple_element_t<0, std::tuple<Ts...>>;
    static_assert(std::conjunction_v<std::is_same<FirstT, Ts>...>,
                  "Array args must all be the same type");
    constexpr size_t N = sizeof...(args);
    Vector<Scalar, sizeof...(args)> v{args...};
    return Value::Create<core::fluent_types::array<FirstT, N>>(std::move(v));
}

/// Creates a Value of DataType<mat<C,R,T> from C*R scalar `args`
template <size_t C, size_t R, typename T>
Value Mat(const T (&m_in)[C][R]) {
    Vector<Scalar, C * R> m;
    for (uint32_t i = 0; i < C; ++i) {
        for (size_t j = 0; j < R; ++j) {
            m.Push(m_in[i][j]);
        }
    }
    return Value::Create<core::fluent_types::mat<C, R, T>>(std::move(m));
}

/// Creates a Value of DataType<mat<2,R,T> from column vectors `c0` and `c1`
template <typename T, size_t R>
Value Mat(const T (&c0)[R], const T (&c1)[R]) {
    constexpr size_t C = 2;
    Vector<Scalar, C * R> m;
    for (auto v : c0) {
        m.Push(v);
    }
    for (auto v : c1) {
        m.Push(v);
    }
    return Value::Create<core::fluent_types::mat<C, R, T>>(std::move(m));
}

/// Creates a Value of DataType<mat<3,R,T> from column vectors `c0`, `c1`, and `c2`
template <typename T, size_t R>
Value Mat(const T (&c0)[R], const T (&c1)[R], const T (&c2)[R]) {
    constexpr size_t C = 3;
    Vector<Scalar, C * R> m;
    for (auto v : c0) {
        m.Push(v);
    }
    for (auto v : c1) {
        m.Push(v);
    }
    for (auto v : c2) {
        m.Push(v);
    }
    return Value::Create<core::fluent_types::mat<C, R, T>>(std::move(m));
}

/// Creates a Value of DataType<mat<4,R,T> from column vectors `c0`, `c1`, `c2`, and `c3`
template <typename T, size_t R>
Value Mat(const T (&c0)[R], const T (&c1)[R], const T (&c2)[R], const T (&c3)[R]) {
    constexpr size_t C = 4;
    Vector<Scalar, C * R> m;
    for (auto v : c0) {
        m.Push(v);
    }
    for (auto v : c1) {
        m.Push(v);
    }
    for (auto v : c2) {
        m.Push(v);
    }
    for (auto v : c3) {
        m.Push(v);
    }
    return Value::Create<core::fluent_types::mat<C, R, T>>(std::move(m));
}
}  // namespace builder
}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_RESOLVER_HELPER_TEST_H_
