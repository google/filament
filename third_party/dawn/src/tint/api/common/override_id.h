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

#ifndef SRC_TINT_API_COMMON_OVERRIDE_ID_H_
#define SRC_TINT_API_COMMON_OVERRIDE_ID_H_

#include <stdint.h>
#include <functional>

#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/reflection.h"

namespace tint {

/// OverrideId is a numerical identifier for an override variable, unique per program.
struct OverrideId {
    /// The identifier value
    uint16_t value = 0;

    /// Reflect the fields of this struct so that it can be used by tint::ForeachField()
    TINT_REFLECT(OverrideId, value);

    /// @returns the hash code of the OverrideId
    tint::HashCode HashCode() const { return Hash(value); }
};

/// Equality operator for OverrideId
/// @param lhs the OverrideId on the left of the '=' operator
/// @param rhs the OverrideId on the right of the '=' operator
/// @returns true if `lhs` is equal to `rhs`
inline bool operator==(OverrideId lhs, OverrideId rhs) {
    return lhs.value == rhs.value;
}

/// Less-than operator for OverrideId
/// @param lhs the OverrideId on the left of the '<' operator
/// @param rhs the OverrideId on the right of the '<' operator
/// @returns true if `lhs` comes before `rhs`
inline bool operator<(OverrideId lhs, OverrideId rhs) {
    return lhs.value < rhs.value;
}

}  // namespace tint

namespace std {

/// Custom std::hash specialization for tint::OverrideId.
template <>
class hash<tint::OverrideId> {
  public:
    /// @param id the override identifier
    /// @return the hash of the override identifier
    inline size_t operator()(tint::OverrideId id) const {
        return std::hash<decltype(tint::OverrideId::value)>()(id.value);
    }
};

}  // namespace std

#endif  // SRC_TINT_API_COMMON_OVERRIDE_ID_H_
