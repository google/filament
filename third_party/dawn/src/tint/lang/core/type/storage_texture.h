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

#ifndef SRC_TINT_LANG_CORE_TYPE_STORAGE_TEXTURE_H_
#define SRC_TINT_LANG_CORE_TYPE_STORAGE_TEXTURE_H_

#include <string>

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"

// Forward declarations
namespace tint::core::type {
class Manager;
}  // namespace tint::core::type

namespace tint::core::type {

/// A storage texture type.
class StorageTexture final : public Castable<StorageTexture, Texture> {
  public:
    /// Constructor
    /// @param dim the dimensionality of the texture
    /// @param format the texel format of the texture
    /// @param access the access control type of the texture
    /// @param subtype the storage subtype. Use SubtypeFor() to calculate this.
    StorageTexture(TextureDimension dim,
                   core::TexelFormat format,
                   core::Access access,
                   const Type* subtype);

    /// Destructor
    ~StorageTexture() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the storage subtype
    const type::Type* Type() const { return subtype_; }

    /// @returns the texel format
    core::TexelFormat TexelFormat() const { return texel_format_; }

    /// @returns the access control
    core::Access Access() const { return access_; }

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @param format the storage texture image format
    /// @param type_mgr the Manager used to build the returned type
    /// @returns the storage texture subtype for the given TexelFormat
    static type::Type* SubtypeFor(core::TexelFormat format, Manager& type_mgr);

    /// @param ctx the clone context
    /// @returns a clone of this type
    StorageTexture* Clone(CloneContext& ctx) const override;

  private:
    core::TexelFormat const texel_format_;
    core::Access const access_;
    const type::Type* const subtype_;
};

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_STORAGE_TEXTURE_H_
