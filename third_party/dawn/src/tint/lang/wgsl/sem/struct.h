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

#ifndef SRC_TINT_LANG_WGSL_SEM_STRUCT_H_
#define SRC_TINT_LANG_WGSL_SEM_STRUCT_H_

#include <optional>

#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/wgsl/ast/struct.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/symbol/symbol.h"

// Forward declarations
namespace tint::ast {
class StructMember;
}  // namespace tint::ast
namespace tint::sem {
class StructMember;
}  // namespace tint::sem
namespace tint::core::type {
class StructMember;
}  // namespace tint::core::type

namespace tint::sem {

/// Struct holds the semantic information for structures.
/// Unlike core::type::Struct, sem::Struct has an AST declaration node.
class Struct final : public Castable<Struct, core::type::Struct> {
  public:
    /// Constructor
    /// @param declaration the AST structure declaration
    /// @param name the name of the structure
    /// @param members the structure members
    /// @param align the byte alignment of the structure
    /// @param size the byte size of the structure
    /// @param size_no_padding size of the members without the end of structure alignment padding
    Struct(const ast::Struct* declaration,
           Symbol name,
           VectorRef<const StructMember*> members,
           uint32_t align,
           uint32_t size,
           uint32_t size_no_padding);

    /// Destructor
    ~Struct() override;

    /// @returns the struct
    const ast::Struct* Declaration() const { return declaration_; }

    /// @returns the members of the structure
    VectorRef<const StructMember*> Members() const {
        return Base::Members().ReinterpretCast<const StructMember*>();
    }

  private:
    ast::Struct const* const declaration_;
};

/// StructMember holds the semantic information for structure members.
/// Unlike core::type::StructMember, sem::StructMember has an AST declaration node.
class StructMember final : public Castable<StructMember, core::type::StructMember> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param name the name of the structure member
    /// @param type the type of the member
    /// @param index the index of the member in the structure
    /// @param offset the byte offset from the base of the structure
    /// @param align the byte alignment of the member
    /// @param size the byte size of the member
    /// @param attributes the optional attributes
    StructMember(const ast::StructMember* declaration,
                 Symbol name,
                 const core::type::Type* type,
                 uint32_t index,
                 uint32_t offset,
                 uint32_t align,
                 uint32_t size,
                 const core::IOAttributes& attributes);

    /// Destructor
    ~StructMember() override;

    /// @returns the AST declaration node
    const ast::StructMember* Declaration() const { return declaration_; }

    /// @returns the structure that owns this member
    const sem::Struct* Struct() const { return static_cast<const sem::Struct*>(Base::Struct()); }

  private:
    const ast::StructMember* const declaration_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_STRUCT_H_
