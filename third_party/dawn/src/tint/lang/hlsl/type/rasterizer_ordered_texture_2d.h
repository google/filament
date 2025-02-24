// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_HLSL_TYPE_RASTERIZER_ORDERED_TEXTURE_2D_H_
#define SRC_TINT_LANG_HLSL_TYPE_RASTERIZER_ORDERED_TEXTURE_2D_H_

#include <string>

#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/texture.h"

namespace tint::hlsl::type {

/// RasterizerOrderedTexture2D represents a 2D ROV texture
// See https://learn.microsoft.com/en-us/windows/win32/direct3d11/rasterizer-order-views
class RasterizerOrderedTexture2D final
    : public Castable<RasterizerOrderedTexture2D, core::type::Texture> {
  public:
    /// Constructor
    /// @param format the texel format
    /// @param subtype the texture subtype
    explicit RasterizerOrderedTexture2D(core::TexelFormat format, const Type* subtype);

    /// @returns the texel format
    core::TexelFormat TexelFormat() const { return texel_format_; }

    /// @returns the storage subtype
    const core::type::Type* Type() const { return subtype_; }

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the friendly name for this type
    std::string FriendlyName() const override;

    /// @param format the storage texture image format
    /// @param type_mgr the Manager used to build the returned type
    /// @returns the storage texture subtype for the given TexelFormat
    static core::type::Type* SubtypeFor(core::TexelFormat format, core::type::Manager& type_mgr);

    /// @param ctx the clone context
    /// @returns a clone of this type
    RasterizerOrderedTexture2D* Clone(core::type::CloneContext& ctx) const override;

  private:
    // const core::type::Type* const store_type_;
    core::TexelFormat const texel_format_;
    const core::type::Type* const subtype_;
};

}  // namespace tint::hlsl::type

#endif  // SRC_TINT_LANG_HLSL_TYPE_RASTERIZER_ORDERED_TEXTURE_2D_H_
