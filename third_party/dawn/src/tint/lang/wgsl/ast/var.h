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

#ifndef SRC_TINT_LANG_WGSL_AST_VAR_H_
#define SRC_TINT_LANG_WGSL_AST_VAR_H_

#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/variable.h"

namespace tint::ast {

/// A "var" declaration is a name for typed storage.
///
/// Examples:
///
/// ```
///  // Declared outside a function, i.e. at module scope, requires
///  // a address space.
///  var<workgroup> width : i32;     // no initializer
///  var<private> height : i32 = 3;  // with initializer
///
///  // A variable declared inside a function doesn't take a address space,
///  // and maps to SPIR-V Function storage.
///  var computed_depth : i32;
///  var area : i32 = compute_area(width, height);
/// ```
///
/// @see https://www.w3.org/TR/WGSL/#var-decls
class Var final : public Castable<Var, Variable> {
  public:
    /// Create a 'var' variable
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the variable source
    /// @param name the variable name
    /// @param type the declared variable type
    /// @param declared_address_space the declared address space
    /// @param declared_access the declared access control
    /// @param initializer the initializer expression
    /// @param attributes the variable attributes
    Var(GenerationID pid,
        NodeID nid,
        const Source& source,
        const Identifier* name,
        Type type,
        const Expression* declared_address_space,
        const Expression* declared_access,
        const Expression* initializer,
        VectorRef<const Attribute*> attributes);

    /// Destructor
    ~Var() override;

    /// @returns "var"
    const char* Kind() const override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Var* Clone(CloneContext& ctx) const override;

    /// The declared address space
    const Expression* const declared_address_space = nullptr;

    /// The declared access control
    const Expression* const declared_access = nullptr;
};

/// A list of `var` declarations
using VarList = std::vector<const Var*>;

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_VAR_H_
