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

#ifndef SRC_TINT_LANG_WGSL_AST_OVERRIDE_H_
#define SRC_TINT_LANG_WGSL_AST_OVERRIDE_H_

#include <string>

#include "src/tint/lang/wgsl/ast/variable.h"

namespace tint::ast {

/// An "override" declaration - a name for a pipeline-overridable constant.
/// Examples:
///
/// ```
///   override radius : i32 = 2;       // Can be overridden by name.
///   @id(5) override width : i32 = 2; // Can be overridden by ID.
///   override scale : f32;            // No default - must be overridden.
/// ```
/// @see https://www.w3.org/TR/WGSL/#override-decls
class Override final : public Castable<Override, Variable> {
  public:
    /// Create an 'override' pipeline-overridable constant.
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the variable source
    /// @param name the variable name
    /// @param type the declared variable type
    /// @param initializer the initializer expression
    /// @param attributes the variable attributes
    Override(GenerationID pid,
             NodeID nid,
             const Source& source,
             const Identifier* name,
             Type type,
             const Expression* initializer,
             VectorRef<const Attribute*> attributes);

    /// Destructor
    ~Override() override;

    /// @returns "override"
    const char* Kind() const override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Override* Clone(CloneContext& ctx) const override;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_OVERRIDE_H_
