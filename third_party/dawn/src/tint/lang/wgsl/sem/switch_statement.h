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

#ifndef SRC_TINT_LANG_WGSL_SEM_SWITCH_STATEMENT_H_
#define SRC_TINT_LANG_WGSL_SEM_SWITCH_STATEMENT_H_

#include <vector>

#include "src/tint/lang/wgsl/sem/block_statement.h"

// Forward declarations
namespace tint::ast {
class CaseStatement;
class CaseSelector;
class SwitchStatement;
}  // namespace tint::ast
namespace tint::core::constant {
class Value;
}  // namespace tint::core::constant
namespace tint::sem {
class CaseStatement;
class CaseSelector;
class ValueExpression;
}  // namespace tint::sem

namespace tint::sem {

/// Holds semantic information about an switch statement
class SwitchStatement final : public Castable<SwitchStatement, CompoundStatement> {
  public:
    /// Constructor
    /// @param declaration the AST node for this switch statement
    /// @param parent the owning statement
    /// @param function the owning function
    SwitchStatement(const ast::SwitchStatement* declaration,
                    const CompoundStatement* parent,
                    const sem::Function* function);

    /// Destructor
    ~SwitchStatement() override;

    /// @return the AST node for this statement
    const ast::SwitchStatement* Declaration() const;

    /// @returns the case statements for this switch
    std::vector<const CaseStatement*>& Cases() { return cases_; }

    /// @returns the case statements for this switch
    const std::vector<const CaseStatement*>& Cases() const { return cases_; }

  private:
    std::vector<const CaseStatement*> cases_;
};

/// Holds semantic information about a switch case statement
class CaseStatement final : public Castable<CaseStatement, CompoundStatement> {
  public:
    /// Constructor
    /// @param declaration the AST node for this case statement
    /// @param parent the owning statement
    /// @param function the owning function
    CaseStatement(const ast::CaseStatement* declaration,
                  const CompoundStatement* parent,
                  const sem::Function* function);

    /// Destructor
    ~CaseStatement() override;

    /// @return the AST node for this statement
    const ast::CaseStatement* Declaration() const;

    /// @param body the case body block statement
    void SetBlock(const BlockStatement* body) { body_ = body; }

    /// @returns the case body block statement
    const BlockStatement* Body() const { return body_; }

    /// @returns the selectors for the case
    std::vector<const CaseSelector*>& Selectors() { return selectors_; }

    /// @returns the selectors for the case
    const std::vector<const CaseSelector*>& Selectors() const { return selectors_; }

  private:
    const BlockStatement* body_ = nullptr;
    std::vector<const CaseSelector*> selectors_;
};

/// Holds semantic information about a switch case selector
class CaseSelector final : public Castable<CaseSelector, Node> {
  public:
    /// Constructor
    /// @param decl the selector declaration
    /// @param val the case selector value, nullptr for a default selector
    explicit CaseSelector(const ast::CaseSelector* decl,
                          const core::constant::Value* val = nullptr);

    /// Destructor
    ~CaseSelector() override;

    /// @returns true if this is a default selector
    bool IsDefault() const { return val_ == nullptr; }

    /// @returns the case selector declaration
    const ast::CaseSelector* Declaration() const;

    /// @returns the selector constant value, or nullptr if this is the default selector
    const core::constant::Value* Value() const { return val_; }

  private:
    const ast::CaseSelector* const decl_;
    const core::constant::Value* const val_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_SWITCH_STATEMENT_H_
