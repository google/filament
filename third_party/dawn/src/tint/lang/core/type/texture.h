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

#ifndef SRC_TINT_LANG_CORE_TYPE_TEXTURE_H_
#define SRC_TINT_LANG_CORE_TYPE_TEXTURE_H_

#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::core::type {

/// A texture type.
class Texture : public Castable<Texture, Type> {
  public:
    /// Constructor
    /// @param hash the unique hash of the node
    /// @param dim the dimensionality of the texture
    Texture(size_t hash, TextureDimension dim);
    /// Destructor
    ~Texture() override;

    /// @returns the texture dimension
    TextureDimension Dim() const { return dim_; }

  private:
    TextureDimension const dim_;
};

/// @param dim the type::TextureDimension to query
/// @return true if the given type::TextureDimension is an array texture
bool IsTextureArray(core::type::TextureDimension dim);

/// Returns the number of axes in the coordinate used for accessing
/// the texture, where an access is one of: sampling, fetching, load,
/// or store.
///  None -> 0
///  1D -> 1
///  2D, 2DArray -> 2
///  3D, Cube, CubeArray -> 3
/// Note: To sample a cube texture, the coordinate has 3 dimensions,
/// but textureDimensions on a cube or cube array returns a 2-element
/// size, representing the (x,y) size of each cube face, in texels.
/// @param dim the type::TextureDimension to query
/// @return number of dimensions in a coordinate for the dimensionality
int NumCoordinateAxes(core::type::TextureDimension dim);

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_TEXTURE_H_
