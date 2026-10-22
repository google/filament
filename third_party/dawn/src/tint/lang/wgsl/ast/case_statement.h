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

#ifndef SRC_TINT_LANG_WGSL_AST_CASE_STATEMENT_H_
#define SRC_TINT_LANG_WGSL_AST_CASE_STATEMENT_H_

#include <vector>

#include "src/tint/lang/wgsl/ast/block_statement.h"
#include "src/tint/lang/wgsl/ast/case_selector.h"

namespace tint::ast {

/// A case statement
class CaseStatement final : public Castable<CaseStatement, Statement> {
  public:
    /// Constructor
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param selectors the case selectors
    /// @param body the case body
    CaseStatement(NodeID nid,
                  const Source& src,
                  VectorRef<const CaseSelector*> selectors,
                  const BlockStatement* body);

    /// Destructor
    ~CaseStatement() override;

    /// @returns true if this item contains a default selector
    bool ContainsDefault() const;

    /// The case selectors, empty if none set
    const tint::Vector<const CaseSelector*, 4> selectors;

    /// The case body
    const BlockStatement* const body;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_CASE_STATEMENT_H_
