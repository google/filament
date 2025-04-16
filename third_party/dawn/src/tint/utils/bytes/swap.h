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

#ifndef SRC_TINT_UTILS_BYTES_SWAP_H_
#define SRC_TINT_UTILS_BYTES_SWAP_H_

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>

#include "src/tint/utils/macros/compiler.h"


namespace tint::bytes {

/// @returns the input value with all bytes reversed
/// @param value the input value, can be any type that passes std::is_integral
///              (this includes non-obvious types like wchar_t)
/// TODO(394825124): Once ranges from C++20 is available this code can be
///                  rewritten to avoid needing to disable warnings.
TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
template <typename T>
[[nodiscard]] inline T Swap(T value) {
    static_assert(std::is_integral_v<T>);
    uint8_t bytes[sizeof(T)];
    memcpy(bytes, &value, sizeof(T));
    for (size_t i = 0; i < sizeof(T) / 2; i++) {
        std::swap(bytes[i], bytes[sizeof(T) - i - 1]);
    }
    T out;
    memcpy(&out, bytes, sizeof(T));
    return out;
}
TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

}  // namespace tint::bytes


#endif  // SRC_TINT_UTILS_BYTES_SWAP_H_
