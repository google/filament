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

#ifndef SRC_TINT_LANG_WGSL_AST_INTERNAL_ATTRIBUTE_H_
#define SRC_TINT_LANG_WGSL_AST_INTERNAL_ATTRIBUTE_H_

#include <string>

#include "src/tint/lang/wgsl/ast/attribute.h"
#include "src/tint/utils/containers/vector.h"

// Forward declarations
namespace tint::ast {
class IdentifierExpression;
}  // namespace tint::ast

namespace tint::ast {

/// An attribute used to indicate that a function is tint-internal.
/// These attributes are not produced by generators, but instead are usually
/// created by transforms for consumption by a particular backend.
class InternalAttribute : public Castable<InternalAttribute, Attribute> {
  public:
    /// Constructor
    /// @param generation_id the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param deps a list of identifiers that this attribute is dependent on
    InternalAttribute(GenerationID generation_id,
                      NodeID nid,
                      VectorRef<const IdentifierExpression*> deps);

    /// Destructor
    ~InternalAttribute() override;

    /// @return a short description of the internal attribute which will be
    /// displayed in WGSL as `@internal(<name>)` (but is not parsable).
    virtual std::string InternalName() const = 0;

    /// @returns the WGSL name for the attribute
    std::string Name() const override;

    /// A list of identifiers that this attribute is dependent on
    const tint::Vector<const IdentifierExpression*, 1> dependencies;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_INTERNAL_ATTRIBUTE_H_
