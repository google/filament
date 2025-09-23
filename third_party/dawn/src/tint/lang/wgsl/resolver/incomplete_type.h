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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_INCOMPLETE_TYPE_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_INCOMPLETE_TYPE_H_

#include <string>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::resolver {

/// Represents a expression that resolved to the name of a type without explicit template arguments.
/// This is a placeholder type that on successful resolving, is replaced with the type built using
/// the inferred template arguments.
/// For example, given the expression `vec3(1i, 2i, 3i)`:
/// * The IdentifierExpression for `vec3` is resolved first, to an IncompleteType as it does not
///   know the vector element type.
/// * Next, the CallExpression replaces the IncompleteType with a core::type::Vector once it can
///   infer the element type from the call's arguments.
class IncompleteType : public Castable<IncompleteType, core::type::Type> {
  public:
    /// Constructor
    /// @param b the incomplete builtin type
    explicit IncompleteType(core::BuiltinType b);

    /// Destructor
    ~IncompleteType() override;

    /// The incomplete builtin type
    const core::BuiltinType builtin = core::BuiltinType::kUndefined;

    /// @copydoc core::type::Type::FriendlyName
    std::string FriendlyName() const override;

    /// @copydoc core::type::Type::Size
    uint32_t Size() const override;

    /// @copydoc core::type::Type::Align
    uint32_t Align() const override;

    /// @copydoc core::type::Type::Clone
    core::type::Type* Clone(core::type::CloneContext& ctx) const override;

    /// @copydoc core::type::UniqueNode::Equals
    bool Equals(const UniqueNode& other) const override;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_INCOMPLETE_TYPE_H_
