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

#ifndef SRC_TINT_LANG_WGSL_SEM_BUILTIN_ENUM_EXPRESSION_H_
#define SRC_TINT_LANG_WGSL_SEM_BUILTIN_ENUM_EXPRESSION_H_

#include "src/tint/lang/wgsl/sem/expression.h"

// Forward declarations
namespace tint::core::type {
class Type;
}  // namespace tint::core::type

namespace tint::sem {

/// Base class for BuiltinEnumExpression.
/// Useful for Is() queries.
class BuiltinEnumExpressionBase : public Castable<BuiltinEnumExpressionBase, Expression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param statement the statement that owns this expression
    BuiltinEnumExpressionBase(const ast::Expression* declaration, const Statement* statement);

    /// Destructor
    ~BuiltinEnumExpressionBase() override;
};

/// BuiltinEnumExpression holds the semantic information for expression nodes that resolve to a
/// builtin enumerator value.
template <typename ENUM>
class BuiltinEnumExpression
    : public Castable<BuiltinEnumExpression<ENUM>, BuiltinEnumExpressionBase> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param statement the statement that owns this expression
    /// @param value the enumerator value
    BuiltinEnumExpression(const ast::Expression* declaration,
                          const Statement* statement,
                          ENUM value)
        : BuiltinEnumExpression<ENUM>::Base(declaration, statement), value_(value) {}

    /// Destructor
    ~BuiltinEnumExpression() override = default;

    /// @return the enumerator value
    ENUM Value() const { return value_; }

  private:
    const ENUM value_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_BUILTIN_ENUM_EXPRESSION_H_
