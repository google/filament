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

#ifndef SRC_TINT_LANG_CORE_TYPE_TEXTURE_DIMENSION_H_
#define SRC_TINT_LANG_CORE_TYPE_TEXTURE_DIMENSION_H_

#include "src/tint/utils/rtti/traits.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::core::type {

/// The dimensionality of the texture
enum class TextureDimension : uint8_t {
    /// 1 dimensional texture
    k1d,
    /// 2 dimensional texture
    k2d,
    /// 2 dimensional array texture
    k2dArray,
    /// 3 dimensional texture
    k3d,
    /// cube texture
    kCube,
    /// cube array texture
    kCubeArray,
    /// Invalid texture
    kNone,
};

/// @param dim the enum value
/// @returns the string for the given enum value
std::string_view ToString(enum type::TextureDimension dim);

/// @param out the stream to write to
/// @param dim the type::TextureDimension
/// @return the stream so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, core::type::TextureDimension dim) {
    return out << ToString(dim);
}

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_TEXTURE_DIMENSION_H_
