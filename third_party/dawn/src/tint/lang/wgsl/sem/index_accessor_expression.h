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

#ifndef SRC_TINT_LANG_WGSL_SEM_INDEX_ACCESSOR_EXPRESSION_H_
#define SRC_TINT_LANG_WGSL_SEM_INDEX_ACCESSOR_EXPRESSION_H_

#include <vector>

#include "src/tint/lang/wgsl/ast/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/accessor_expression.h"

namespace tint::sem {

/// IndexAccessorExpression holds the semantic information for a ast::IndexAccessorExpression node.
class IndexAccessorExpression final : public Castable<IndexAccessorExpression, AccessorExpression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param stage the earliest evaluation stage for the expression
    /// @param object the object expression that is being indexed
    /// @param index the index expression
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param has_side_effects whether this expression may have side effects
    /// @param root_ident the (optional) root identifier for this expression
    IndexAccessorExpression(const ast::IndexAccessorExpression* declaration,
                            const core::type::Type* type,
                            core::EvaluationStage stage,
                            const ValueExpression* object,
                            const ValueExpression* index,
                            const Statement* statement,
                            const core::constant::Value* constant,
                            bool has_side_effects,
                            const Variable* root_ident = nullptr);

    /// Destructor
    ~IndexAccessorExpression() override;

    /// @returns the AST node
    const ast::IndexAccessorExpression* Declaration() const {
        return static_cast<const ast::IndexAccessorExpression*>(declaration_);
    }

    /// @returns the index expression
    ValueExpression const* Index() const { return index_; }

  private:
    ValueExpression const* const index_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_INDEX_ACCESSOR_EXPRESSION_H_
