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

#ifndef SRC_TINT_LANG_WGSL_SEM_MATERIALIZE_H_
#define SRC_TINT_LANG_WGSL_SEM_MATERIALIZE_H_

#include "src/tint/lang/wgsl/sem/value_expression.h"

namespace tint::sem {

/// Materialize is a semantic expression which represents the materialization of a value of an
/// abstract numeric type to a value of a concrete type.
/// Abstract numeric materialization is implicit in WGSL, so the Materialize semantic node shares
/// the same AST node as the inner semantic node.
/// Abstract numerics types may only be used by compile-time expressions, so a Materialize semantic
/// node must have a valid Constant value.
class Materialize final : public Castable<Materialize, ValueExpression> {
  public:
    /// Constructor
    /// @param expr the inner expression, being materialized
    /// @param statement the statement that owns this expression
    /// @param type concrete type to materialize to
    /// @param constant the constant value of this expression or nullptr
    Materialize(const ValueExpression* expr,
                const Statement* statement,
                const core::type::Type* type,
                const core::constant::Value* constant);

    /// Destructor
    ~Materialize() override;

    /// @return the expression being materialized
    const ValueExpression* Expr() const { return expr_; }

  private:
    ValueExpression const* const expr_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_MATERIALIZE_H_
