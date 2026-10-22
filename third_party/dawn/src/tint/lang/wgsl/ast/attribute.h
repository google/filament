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

#ifndef SRC_TINT_LANG_WGSL_AST_ATTRIBUTE_H_
#define SRC_TINT_LANG_WGSL_AST_ATTRIBUTE_H_

#include <string>

#include "src/tint/lang/wgsl/ast/node.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::ast {

/// The base class for all attributes
class Attribute : public Castable<Attribute, Node> {
  public:
    ~Attribute() override;

    /// @returns the WGSL name for the attribute
    virtual std::string Name() const = 0;

  protected:
    /// Constructor
    /// @param nid the unique node identifier
    /// @param src the source of this node
    Attribute(NodeID nid, const Source& src) : Base(nid, src) {}
};

/// @param attributes the list of attributes to search
/// @returns true if `attributes` includes a attribute of type `T`
template <typename... Ts>
bool HasAttribute(VectorRef<const Attribute*> attributes) {
    for (auto* attr : attributes) {
        if (attr->IsAnyOf<Ts...>()) {
            return true;
        }
    }
    return false;
}

/// @param attributes the list of attributes to search
/// @returns a pointer to `T` from `attributes` if found, otherwise nullptr.
template <typename T>
const T* GetAttribute(VectorRef<const Attribute*> attributes) {
    for (auto* attr : attributes) {
        if (attr->Is<T>()) {
            return attr->As<T>();
        }
    }
    return nullptr;
}

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_ATTRIBUTE_H_
