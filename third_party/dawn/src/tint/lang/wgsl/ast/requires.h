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

#ifndef SRC_TINT_LANG_WGSL_AST_REQUIRES_H_
#define SRC_TINT_LANG_WGSL_AST_REQUIRES_H_

#include <string>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/node.h"
#include "src/tint/lang/wgsl/enums.h"
#include "src/tint/utils/containers/unique_vector.h"

namespace tint::ast {

/// A "requires" directive. Example:
/// ```
///   // Require a language feature named "foo"
///   requires foo;
/// ```
class Requires final : public Castable<Requires, Node> {
  public:
    /// A unique list of WGSL language features
    using LanguageFeatures = UniqueVector<wgsl::LanguageFeature, 4>;

    /// Create a requires directive
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param feats the language features being required by this directive
    Requires(GenerationID pid, NodeID nid, const Source& src, LanguageFeatures feats);

    /// Destructor
    ~Requires() override;

    /// Clones this node and all transitive child nodes using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Requires* Clone(CloneContext& ctx) const override;

    /// The features being required by this directive.
    const LanguageFeatures features;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_REQUIRES_H_
