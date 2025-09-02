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

#ifndef SRC_TINT_LANG_WGSL_AST_VARIABLE_H_
#define SRC_TINT_LANG_WGSL_AST_VARIABLE_H_

#include <utility>
#include <vector>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/wgsl/ast/attribute.h"
#include "src/tint/lang/wgsl/ast/binding_attribute.h"
#include "src/tint/lang/wgsl/ast/expression.h"
#include "src/tint/lang/wgsl/ast/group_attribute.h"
#include "src/tint/lang/wgsl/ast/input_attachment_index_attribute.h"
#include "src/tint/lang/wgsl/ast/node.h"
#include "src/tint/lang/wgsl/ast/type.h"

// Forward declarations
namespace tint::ast {
class Identifier;
class LocationAttribute;
}  // namespace tint::ast

namespace tint::ast {

/// Variable is the base class for Var, Let, Const, Override and Parameter.
///
/// An instance of this class represents one of five constructs in WGSL: "var"  declaration, "let"
/// declaration, "override" declaration, "const" declaration, or formal parameter to a function.
///
/// @see https://www.w3.org/TR/WGSL/#value-decls
class Variable : public Castable<Variable, Node> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the variable source
    /// @param name The struct member name
    /// @param type the declared variable type
    /// @param initializer the initializer expression
    /// @param attributes the variable attributes
    Variable(GenerationID pid,
             NodeID nid,
             const Source& src,
             const Identifier* name,
             Type type,
             const Expression* initializer,
             VectorRef<const Attribute*> attributes);

    /// Destructor
    ~Variable() override;

    /// @returns true if the variable has both group and binding attributes
    bool HasBindingPoint() const {
        return HasAttribute<BindingAttribute>(attributes) &&
               HasAttribute<GroupAttribute>(attributes);
    }

    /// @returns true if the variable has an input_attachment_index attribute
    bool HasInputAttachmentIndex() const {
        return HasAttribute<InputAttachmentIndexAttribute>(attributes);
    }

    /// @returns the kind of the variable, which can be used in diagnostics
    ///          e.g. "var", "let", "const", etc
    virtual const char* Kind() const = 0;

    /// The variable name
    const Identifier* const name;

    /// The declared variable type. This is null if the type is inferred, e.g.:
    ///   let f = 1.0;
    ///   var i = 1;
    const Type type;

    /// The initializer expression or nullptr if none set
    const Expression* const initializer;

    /// The attributes attached to this variable
    const tint::Vector<const Attribute*, 2> attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_VARIABLE_H_
