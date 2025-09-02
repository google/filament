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

#ifndef SRC_TINT_API_COMMON_BINDING_POINT_H_
#define SRC_TINT_API_COMMON_BINDING_POINT_H_

#include <stdint.h>

#include <functional>

#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/reflection.h"
#include "src/tint/utils/rtti/traits.h"

namespace tint {

/// BindingPoint holds a group and binding index.
struct BindingPoint {
    /// The `@group` part of the binding point
    uint32_t group = 0;
    /// The `@binding` part of the binding point
    uint32_t binding = 0;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(BindingPoint, group, binding);

    /// @returns the hash code of the BindingPoint
    tint::HashCode HashCode() const { return tint::Hash(group, binding); }

    /// Equality operator
    /// @param rhs the BindingPoint to compare against
    /// @returns true if this BindingPoint is equal to `rhs`
    bool operator==(const BindingPoint& rhs) const {
        return group == rhs.group && binding == rhs.binding;
    }

    /// Inequality operator
    /// @param rhs the BindingPoint to compare against
    /// @returns true if this BindingPoint is not equal to `rhs`
    bool operator!=(const BindingPoint& rhs) const { return !(*this == rhs); }

    /// Less-than operator
    /// @param rhs the BindingPoint to compare against
    /// @returns true if this BindingPoint comes before @p rhs
    bool operator<(const BindingPoint& rhs) const {
        if (group < rhs.group) {
            return true;
        }
        if (group > rhs.group) {
            return false;
        }
        return binding < rhs.binding;
    }
};

/// Prints the BindingPoint @p bp to @p o
/// @param o the stream to write to
/// @param bp the BindingPoint
/// @return the stream so calls can be chained
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& o, const BindingPoint& bp) {
    return o << "[group: " << bp.group << ", binding: " << bp.binding << "]";
}

}  // namespace tint

namespace std {

/// Custom std::hash specialization for tint::BindingPoint so BindingPoints can be used as keys for
/// std::unordered_map and std::unordered_set.
template <>
class hash<tint::BindingPoint> {
  public:
    /// @param binding_point the binding point to create a hash for
    /// @return the hash value
    size_t operator()(const tint::BindingPoint& binding_point) const {
        return (static_cast<size_t>(binding_point.group) << 16) |
               static_cast<size_t>(binding_point.binding);
    }
};

}  // namespace std

#endif  // SRC_TINT_API_COMMON_BINDING_POINT_H_
