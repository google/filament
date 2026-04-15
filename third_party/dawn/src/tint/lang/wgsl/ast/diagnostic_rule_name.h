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

#ifndef SRC_TINT_LANG_WGSL_AST_DIAGNOSTIC_RULE_NAME_H_
#define SRC_TINT_LANG_WGSL_AST_DIAGNOSTIC_RULE_NAME_H_

#include <string>

#include "src/tint/lang/wgsl/ast/node.h"

// Forward declarations
namespace tint::ast {
class Identifier;
}  // namespace tint::ast

namespace tint::ast {

/// A diagnostic rule name used for diagnostic directives and attributes.
class DiagnosticRuleName final : public Castable<DiagnosticRuleName, Node> {
  public:
    /// Constructor
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param name the rule name
    DiagnosticRuleName(NodeID nid, const Source& src, const Identifier* name);

    /// Constructor
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param category the rule category.
    /// @param name the rule name
    DiagnosticRuleName(NodeID nid,
                       const Source& src,
                       const Identifier* category,
                       const Identifier* name);

    ~DiagnosticRuleName() override;

    /// @return the full name of this diagnostic rule, either as `name` or `category.name`.
    std::string String() const;

    /// The diagnostic rule category (category.name)
    Identifier const* const category = nullptr;

    /// The diagnostic rule name.
    Identifier const* const name;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_DIAGNOSTIC_RULE_NAME_H_
