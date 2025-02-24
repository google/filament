// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_SWITCH_STATEMENT_H_
#define SRC_TINT_LANG_WGSL_AST_SWITCH_STATEMENT_H_

#include "src/tint/lang/wgsl/ast/case_statement.h"
#include "src/tint/lang/wgsl/ast/expression.h"

namespace tint::ast {

/// A switch statement
class SwitchStatement final : public Castable<SwitchStatement, Statement> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param condition the switch condition
    /// @param body the switch body
    /// @param stmt_attributes the switch statement attributes
    /// @param body_attributes the switch body attributes
    SwitchStatement(GenerationID pid,
                    NodeID nid,
                    const Source& src,
                    const Expression* condition,
                    VectorRef<const CaseStatement*> body,
                    VectorRef<const Attribute*> stmt_attributes,
                    VectorRef<const Attribute*> body_attributes);

    /// Destructor
    ~SwitchStatement() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const SwitchStatement* Clone(CloneContext& ctx) const override;

    /// The switch condition or nullptr if none set
    const Expression* const condition;

    /// The Switch body
    const tint::Vector<const CaseStatement*, 4> body;
    SwitchStatement(const SwitchStatement&) = delete;

    /// The attribute list for the statement
    const tint::Vector<const Attribute*, 1> attributes;

    /// The attribute list for the body
    const tint::Vector<const Attribute*, 1> body_attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_SWITCH_STATEMENT_H_
