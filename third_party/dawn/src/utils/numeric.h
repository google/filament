// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_UTILS_NUMERIC_H_
#define SRC_UTILS_NUMERIC_H_

#include <limits>
#include <type_traits>

namespace dawn {
template <typename T>
bool inline IsDoubleValueRepresentable(double value) {
    if constexpr (std::is_same_v<T, float> || std::is_integral_v<T>) {
        // Following WebIDL 3.3.6.[EnforceRange] for integral
        // Following WebIDL 3.2.5.float for float
        // TODO(crbug.com/1396194): now follows what blink does but may need revisit.
        constexpr double kLowest = static_cast<double>(std::numeric_limits<T>::lowest());
        constexpr double kMax = static_cast<double>(std::numeric_limits<T>::max());
        return kLowest <= value && value <= kMax;
    } else {
        static_assert(std::is_same_v<T, float> || std::is_integral_v<T>, "Unsupported type");
    }
}

inline bool IsDoubleValueRepresentableAsF16(double value) {
    constexpr double kLowestF16 = -65504.0;
    constexpr double kMaxF16 = 65504.0;
    return kLowestF16 <= value && value <= kMaxF16;
}
}  // namespace dawn

#endif  // SRC_UTILS_NUMERIC_H_
