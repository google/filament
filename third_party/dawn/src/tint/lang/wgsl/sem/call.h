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

#ifndef SRC_TINT_LANG_WGSL_SEM_CALL_H_
#define SRC_TINT_LANG_WGSL_SEM_CALL_H_

#include <vector>

#include "src/tint/lang/wgsl/ast/call_expression.h"
#include "src/tint/lang/wgsl/sem/builtin_fn.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::sem {

/// Call is the base class for semantic nodes that hold semantic information for
/// ast::CallExpression nodes.
class Call final : public Castable<Call, ValueExpression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param target the call target
    /// @param stage the earliest evaluation stage for the expression
    /// @param arguments the call arguments
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of this expression
    /// @param has_side_effects whether this expression may have side effects
    Call(const ast::CallExpression* declaration,
         const CallTarget* target,
         core::EvaluationStage stage,
         VectorRef<const sem::ValueExpression*> arguments,
         const Statement* statement,
         const core::constant::Value* constant,
         bool has_side_effects);

    /// Destructor
    ~Call() override;

    /// @return the target of the call
    const CallTarget* Target() const { return target_; }

    /// @return the call arguments
    const auto& Arguments() const { return arguments_; }

    /// @returns the AST node
    const ast::CallExpression* Declaration() const {
        return static_cast<const ast::CallExpression*>(declaration_);
    }

  private:
    CallTarget const* const target_;
    tint::Vector<const sem::ValueExpression*, 8> arguments_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_CALL_H_
