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

#ifndef SRC_TINT_LANG_WGSL_SEM_VALUE_EXPRESSION_H_
#define SRC_TINT_LANG_WGSL_SEM_VALUE_EXPRESSION_H_

#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/evaluation_stage.h"
#include "src/tint/lang/wgsl/sem/behavior.h"
#include "src/tint/lang/wgsl/sem/expression.h"

// Forward declarations
namespace tint::sem {
class Statement;
class Variable;
}  // namespace tint::sem

namespace tint::sem {

/// ValueExpression holds the semantic information for expression nodes.
class ValueExpression : public Castable<ValueExpression, Expression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param stage the earliest evaluation stage for the expression
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param has_side_effects true if this expression may have side-effects
    /// @param root_ident the (optional) root identifier for this expression
    ValueExpression(const ast::Expression* declaration,
                    const core::type::Type* type,
                    core::EvaluationStage stage,
                    const Statement* statement,
                    const core::constant::Value* constant,
                    bool has_side_effects,
                    const Variable* root_ident = nullptr);

    /// Destructor
    ~ValueExpression() override;

    /// @return the resolved type of the expression
    const core::type::Type* Type() const { return type_; }

    /// @return the earliest evaluation stage for the expression
    core::EvaluationStage Stage() const { return stage_; }

    /// @return the constant value of this expression
    const core::constant::Value* ConstantValue() const { return constant_; }

    /// Returns the variable or parameter that this expression derives from.
    /// For reference and pointer expressions, this will either be the originating
    /// variable or a function parameter. For other types of expressions, it will
    /// either be the parameter or constant declaration, or nullptr.
    /// @return the root identifier of this expression, or nullptr
    const Variable* RootIdentifier() const { return root_identifier_; }

    /// @return the behaviors of this statement
    const sem::Behaviors& Behaviors() const { return behaviors_; }

    /// @return the behaviors of this statement
    sem::Behaviors& Behaviors() { return behaviors_; }

    /// @return true of this expression may have side effects
    bool HasSideEffects() const { return has_side_effects_; }

    /// @return the inner expression node if this is a Materialize, otherwise this.
    const ValueExpression* UnwrapMaterialize() const;

    /// @return the inner reference expression if this is a Load, otherwise this.
    const ValueExpression* UnwrapLoad() const;

    /// @return the inner expression node if this is a Materialize or Load, otherwise this.
    const ValueExpression* Unwrap() const;

  protected:
    /// The root identifier for this semantic expression, or nullptr
    const Variable* root_identifier_;

  private:
    const core::type::Type* const type_;
    const core::EvaluationStage stage_;
    const core::constant::Value* const constant_;
    sem::Behaviors behaviors_{sem::Behavior::kNext};
    const bool has_side_effects_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_VALUE_EXPRESSION_H_
