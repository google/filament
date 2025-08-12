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

#ifndef SRC_TINT_LANG_WGSL_SEM_SAMPLER_TEXTURE_PAIR_H_
#define SRC_TINT_LANG_WGSL_SEM_SAMPLER_TEXTURE_PAIR_H_

#include <cstdint>
#include <functional>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/utils/reflection.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::sem {

/// Mapping of a sampler to a texture it samples.
struct SamplerTexturePair {
    /// group & binding values for a sampler.
    BindingPoint sampler_binding_point;
    /// group & binding values for a texture samepled by the sampler.
    BindingPoint texture_binding_point;

    /// Equality operator
    /// @param rhs the SamplerTexturePair to compare against
    /// @returns true if this SamplerTexturePair is equal to `rhs`
    inline bool operator==(const SamplerTexturePair& rhs) const {
        return sampler_binding_point == rhs.sampler_binding_point &&
               texture_binding_point == rhs.texture_binding_point;
    }

    /// Inequality operator
    /// @param rhs the SamplerTexturePair to compare against
    /// @returns true if this SamplerTexturePair is not equal to `rhs`
    inline bool operator!=(const SamplerTexturePair& rhs) const { return !(*this == rhs); }

    /// Less than operator
    /// @param rhs the SamplerTexturePair to compare against
    /// @returns true if this SamplerTexturePair is less then `rhs`
    inline bool operator<(const SamplerTexturePair& rhs) const {
        if (sampler_binding_point == rhs.sampler_binding_point) {
            return texture_binding_point < rhs.texture_binding_point;
        }
        return sampler_binding_point < rhs.sampler_binding_point;
    }

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(SamplerTexturePair, sampler_binding_point, texture_binding_point);
};

/// Prints the SamplerTexturePair @p stp to @p o
/// @param o the stream to write to
/// @param stp the SamplerTexturePair
/// @return the stream so calls can be chained
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& o, const SamplerTexturePair& stp) {
    return o << "[sampler: " << stp.sampler_binding_point
             << ", texture: " << stp.sampler_binding_point << "]";
}

}  // namespace tint::sem

namespace std {

/// Custom std::hash specialization for tint::sem::SamplerTexturePair so
/// SamplerTexturePairs be used as keys for std::unordered_map and
/// std::unordered_set.
template <>
class hash<tint::sem::SamplerTexturePair> {
  public:
    /// @param stp the texture pair to create a hash for
    /// @return the hash value
    inline std::size_t operator()(const tint::sem::SamplerTexturePair& stp) const {
        return Hash(stp.sampler_binding_point, stp.texture_binding_point);
    }
};

}  // namespace std

#endif  // SRC_TINT_LANG_WGSL_SEM_SAMPLER_TEXTURE_PAIR_H_
