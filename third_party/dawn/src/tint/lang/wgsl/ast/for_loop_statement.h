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

#ifndef SRC_TINT_LANG_WGSL_AST_FOR_LOOP_STATEMENT_H_
#define SRC_TINT_LANG_WGSL_AST_FOR_LOOP_STATEMENT_H_

#include "src/tint/lang/wgsl/ast/block_statement.h"

namespace tint::ast {

class Expression;

/// A for loop statement
class ForLoopStatement final : public Castable<ForLoopStatement, Statement> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the for loop statement source
    /// @param initializer the optional loop initializer statement
    /// @param condition the optional loop condition expression
    /// @param continuing the optional continuing statement
    /// @param body the loop body
    /// @param attributes the while statement attributes
    ForLoopStatement(GenerationID pid,
                     NodeID nid,
                     const Source& source,
                     const Statement* initializer,
                     const Expression* condition,
                     const Statement* continuing,
                     const BlockStatement* body,
                     VectorRef<const ast::Attribute*> attributes);

    /// Destructor
    ~ForLoopStatement() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const ForLoopStatement* Clone(CloneContext& ctx) const override;

    /// The initializer statement
    const Statement* const initializer;

    /// The condition expression
    const Expression* const condition;

    /// The continuing statement
    const Statement* const continuing;

    /// The loop body block
    const BlockStatement* const body;

    /// The attribute list
    const tint::Vector<const Attribute*, 1> attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_FOR_LOOP_STATEMENT_H_
