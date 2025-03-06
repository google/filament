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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_HOIST_TO_DECL_BEFORE_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_HOIST_TO_DECL_BEFORE_H_

#include <functional>
#include <memory>

#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"

namespace tint::ast::transform {

/// Utility class that can be used to hoist expressions before other
/// expressions, possibly converting 'for-loop's to 'loop's and 'else-if's to
// 'else {if}'s.
class HoistToDeclBefore {
  public:
    /// Constructor
    /// @param ctx the clone context
    explicit HoistToDeclBefore(program::CloneContext& ctx);

    /// Destructor
    ~HoistToDeclBefore();

    /// StmtBuilder is a builder of an AST statement
    using StmtBuilder = std::function<const Statement*()>;

    /// VariableKind is either a var, let or const
    enum class VariableKind {
        kVar,
        kLet,
        kConst,
    };

    /// Hoists @p expr to a `let` or `var` with optional `decl_name`, inserting it
    /// before @p before_expr.
    /// @param before_expr expression to insert `expr` before
    /// @param expr expression to hoist
    /// @param kind variable kind to hoist to
    /// @param decl_name optional name to use for the variable/constant name
    /// @return true on success
    bool Add(const sem::ValueExpression* before_expr,
             const Expression* expr,
             VariableKind kind,
             const char* decl_name = "");

    /// Inserts @p stmt before @p before_stmt, possibly converting 'for-loop's to 'loop's if
    /// necessary.
    /// @warning If the container of @p before_stmt is cloned multiple times, then the resolver will
    /// ICE as the same statement cannot be shared.
    /// @param before_stmt statement to insert @p stmt before
    /// @param stmt statement to insert
    /// @return true on success
    bool InsertBefore(const sem::Statement* before_stmt, const Statement* stmt);

    /// Inserts the returned statement of @p builder before @p before_stmt, possibly converting
    /// 'for-loop's to 'loop's if necessary.
    /// @note If the container of @p before_stmt is cloned multiple times, then @p builder will be
    /// called for each clone.
    /// @param before_stmt the preceding statement that the statement of @p builder will be inserted
    /// before
    /// @param builder the statement builder used to create the new statement
    /// @return true on success
    bool InsertBefore(const sem::Statement* before_stmt, const StmtBuilder& builder);

    /// Replaces the statement @p what with the statement @p stmt, possibly converting 'for-loop's
    /// to 'loop's if necessary.
    /// @param what the statement to replace
    /// @param with the replacement statement
    /// @return true on success
    bool Replace(const sem::Statement* what, const Statement* with);

    /// Replaces the statement @p what with the statement returned by @p stmt, possibly converting
    /// 'for-loop's to 'loop's if necessary.
    /// @param what the statement to replace
    /// @param with the replacement statement builder
    /// @return true on success
    bool Replace(const sem::Statement* what, const StmtBuilder& with);

    /// Use to signal that we plan on hoisting a decl before `before_expr`. This
    /// will convert 'for-loop's to 'loop's and 'else-if's to 'else {if}'s if
    /// needed.
    /// @param before_expr expression we would hoist a decl before
    /// @return true on success
    bool Prepare(const sem::ValueExpression* before_expr);

  private:
    struct State;
    std::unique_ptr<State> state_;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_HOIST_TO_DECL_BEFORE_H_
