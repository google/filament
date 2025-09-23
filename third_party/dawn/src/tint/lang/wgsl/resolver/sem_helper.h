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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_SEM_HELPER_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_SEM_HELPER_H_

#include <string>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/dependency_graph.h"
#include "src/tint/lang/wgsl/sem/builtin_enum_expression.h"
#include "src/tint/lang/wgsl/sem/function_expression.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::resolver {

/// Helper class to retrieve sem information.
class SemHelper {
  public:
    /// Constructor
    /// @param builder the program builder
    explicit SemHelper(ProgramBuilder* builder);
    ~SemHelper();

    /// Get is a helper for obtaining the semantic node for the given AST node.
    /// Raises an ICE and returns `nullptr` if there is no semantic node associated with the AST
    /// node.
    /// @param ast the ast node to get the sem for
    /// @returns the sem node for @p ast
    template <typename SEM = sem::Info::InferFromAST, typename AST = ast::Node>
    auto* Get(const AST* ast) const {
        using T = sem::Info::GetResultType<SEM, AST>;
        auto* sem = builder_->Sem().Get(ast);
        if (DAWN_UNLIKELY(!sem)) {
            TINT_ICE() << "AST node '" << ast->TypeInfo().name << "' had no semantic info\n"
                       << "At: " << ast->source << "\n"
                       << "Pointer: " << ast;
        }
        return const_cast<T*>(As<T>(sem));
    }

    /// GetVal is a helper for obtaining the semantic sem::ValueExpression for the given AST
    /// expression. Raises an error diagnostic and returns `nullptr` if the semantic node is not a
    /// sem::ValueExpression.
    /// @param ast the ast node to get the sem for
    /// @returns the sem node for @p ast
    template <typename AST = ast::Node>
    auto* GetVal(const AST* ast) const {
        return AsValueExpression(Get(ast));
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to sem::ValueExpression if the cast
    /// is successful, otherwise an error diagnostic is raised.
    sem::ValueExpression* AsValueExpression(sem::Expression* expr) const {
        if (DAWN_LIKELY(expr)) {
            if (auto* val_expr = expr->As<sem::ValueExpression>(); DAWN_LIKELY(val_expr)) {
                return val_expr;
            }
            ErrorExpectedValueExpr(expr);
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to type::Type if the cast is
    /// successful, otherwise an error diagnostic is raised.
    sem::TypeExpression* AsTypeExpression(sem::Expression* expr) const;

    /// GetType is a helper for obtaining the semantic type for the given AST expression.
    /// Raises an error diagnostic and returns `nullptr` if the semantic node is not a
    /// sem::TypeExpression
    /// @param ast the ast node to get the sem for
    /// @returns the sem node for @p ast
    const core::type::Type* GetType(const ast::Expression* ast) const {
        auto* expr = AsTypeExpression(Get(ast));
        if (DAWN_LIKELY(expr)) {
            return expr->Type();
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to sem::Function if the cast is
    /// successful, otherwise an error diagnostic is raised.
    sem::FunctionExpression* AsFunctionExpression(sem::Expression* expr) const {
        if (DAWN_LIKELY(expr)) {
            auto* fn_expr = expr->As<sem::FunctionExpression>();
            if (DAWN_LIKELY(fn_expr)) {
                return fn_expr;
            }
            ErrorUnexpectedExprKind(expr, "function");
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to
    /// sem::BuiltinEnumExpression<core::AddressSpace> if the cast is successful, otherwise an
    /// error diagnostic is raised.
    sem::BuiltinEnumExpression<core::AddressSpace>* AsAddressSpace(sem::Expression* expr) const {
        if (DAWN_LIKELY(expr)) {
            auto* enum_expr = expr->As<sem::BuiltinEnumExpression<core::AddressSpace>>();
            if (DAWN_LIKELY(enum_expr)) {
                return enum_expr;
            }
            ErrorUnexpectedExprKind(expr, "address space", core::kAddressSpaceStrings);
        }
        return nullptr;
    }

    /// GetAddressSpace is a helper for obtaining the address space for the given AST expression.
    /// Raises an error diagnostic and returns core::AddressSpace::kUndefined if the semantic node
    /// is not a sem::BuiltinEnumExpression<core::AddressSpace>
    /// @param ast the ast node to get the address space
    /// @returns the sem node for @p ast
    core::AddressSpace GetAddressSpace(const ast::Expression* ast) const {
        auto* expr = AsAddressSpace(Get(ast));
        if (DAWN_LIKELY(expr)) {
            return expr->Value();
        }
        return core::AddressSpace::kUndefined;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to
    /// sem::BuiltinEnumExpression<core::type::TexelFormat> if the cast is successful, otherwise an
    /// error diagnostic is raised.
    sem::BuiltinEnumExpression<core::TexelFormat>* AsTexelFormat(sem::Expression* expr) const {
        if (DAWN_LIKELY(expr)) {
            auto* enum_expr = expr->As<sem::BuiltinEnumExpression<core::TexelFormat>>();
            if (DAWN_LIKELY(enum_expr)) {
                return enum_expr;
            }
            ErrorUnexpectedExprKind(expr, "texel format", core::kTexelFormatStrings);
        }
        return nullptr;
    }

    /// GetTexelFormat is a helper for obtaining the texel format for the given AST expression.
    /// Raises an error diagnostic and returns core::TexelFormat::kUndefined if the semantic node
    /// is not a sem::BuiltinEnumExpression<core::TexelFormat>
    /// @param ast the ast node to get the texel format
    /// @returns the sem node for @p ast
    core::TexelFormat GetTexelFormat(const ast::Expression* ast) const {
        auto* expr = AsTexelFormat(Get(ast));
        if (DAWN_LIKELY(expr)) {
            return expr->Value();
        }
        return core::TexelFormat::kUndefined;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to
    /// sem::BuiltinEnumExpression<core::Access> if the cast is successful, otherwise an error
    /// diagnostic is raised.
    sem::BuiltinEnumExpression<core::Access>* AsAccess(sem::Expression* expr) const {
        if (DAWN_LIKELY(expr)) {
            auto* enum_expr = expr->As<sem::BuiltinEnumExpression<core::Access>>();
            if (DAWN_LIKELY(enum_expr)) {
                return enum_expr;
            }
            ErrorUnexpectedExprKind(expr, "access", core::kAccessStrings);
        }
        return nullptr;
    }

    /// GetAccess is a helper for obtaining the access mode for the given AST expression.
    /// Raises an error diagnostic and returns core::Access::kUndefined if the semantic node
    /// is not a sem::BuiltinEnumExpression<core::Access>
    /// @param ast the ast node to get the access mode
    /// @returns the sem node for @p ast
    core::Access GetAccess(const ast::Expression* ast) const {
        auto* expr = AsAccess(Get(ast));
        if (DAWN_LIKELY(expr)) {
            return expr->Value();
        }
        return core::Access::kUndefined;
    }

    /// @returns the resolved type of the ast::Expression @p expr
    /// @param expr the expression
    core::type::Type* TypeOf(const ast::Expression* expr) const;

    /// @returns the type name of the given semantic type, unwrapping references.
    /// @param ty the type to look up
    std::string TypeNameOf(const core::type::Type* ty) const;

    /// @returns the type name of the given semantic type, without unwrapping references.
    /// @param ty the type to look up
    std::string RawTypeNameOf(const core::type::Type* ty) const;

    /// Raises an error diagnostic that the expression @p got was expected to be a
    /// sem::ValueExpression, but the expression evaluated to something different.
    /// @param expr the expression
    void ErrorExpectedValueExpr(const sem::Expression* expr) const;

    /// Raises an error diagnostic that the identifier @p got was not of the kind @p wanted.
    /// @param ident the identifier
    /// @param wanted the expected identifier kind
    /// @param suggestions suggested valid identifiers
    void ErrorUnexpectedIdent(const ast::Identifier* ident,
                              std::string_view wanted,
                              tint::Slice<const std::string_view> suggestions = Empty) const;

    /// Raises an error diagnostic that the expression @p got was not of the kind @p wanted.
    /// @param expr the expression
    /// @param wanted the expected expression kind
    /// @param suggestions suggested valid identifiers
    void ErrorUnexpectedExprKind(const sem::Expression* expr,
                                 std::string_view wanted,
                                 tint::Slice<const std::string_view> suggestions = Empty) const;

    /// If @p node is a module-scope type, variable or function declaration, then appends a note
    /// diagnostic where this declaration was declared, otherwise the function does nothing.
    /// @param node the AST node.
    void NoteDeclarationSource(const ast::Node* node) const;

    /// @param expr the expression to describe
    /// @return a string that describes @p expr. Useful for diagnostics.
    StyledText Describe(const sem::Expression* expr) const;

  private:
    /// @returns a new error diagnostics
    diag::Diagnostic& AddError(const Source& source) const;

    /// @returns a new warning diagnostics
    diag::Diagnostic& AddWarning(const Source& source) const;

    /// @returns a new note diagnostics
    diag::Diagnostic& AddNote(const Source& source) const;

    ProgramBuilder* builder_;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_SEM_HELPER_H_
