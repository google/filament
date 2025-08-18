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

#ifndef SRC_TINT_UTILS_MATH_MATH_H_
#define SRC_TINT_UTILS_MATH_MATH_H_

#include <stdint.h>

#include <string>
#include <type_traits>

namespace tint {

/// @param alignment the next multiple to round `value` to
/// @param value the value to round to the next multiple of `alignment`
/// @return `value` rounded to the next multiple of `alignment`, or `value` if `alignment` is 0
/// @note `alignment` must be positive.
template <typename T>
inline constexpr T RoundUp(T alignment, T value) {
    return alignment != 0 ? ((value + alignment - 1) / alignment) * alignment : value;
}

/// @param value the value to check whether it is a power-of-two
/// @returns true if `value` is a power-of-two
/// @note `value` must be positive if `T` is signed
template <typename T>
inline constexpr bool IsPowerOfTwo(T value) {
    return (value & (value - 1)) == 0;
}

/// @param value the input value
/// @returns the base-2 logarithm of @p value
inline constexpr uint32_t Log2(uint64_t value) {
#if defined(__clang__) || defined(__GNUC__)
    return 63 - static_cast<uint32_t>(__builtin_clzll(value));
#elif defined(_MSC_VER) && !defined(__clang__) && __cplusplus >= 202002L  // MSVC and C++20+
    // note: std::is_constant_evaluated() added in C++20
    //       required here as _BitScanReverse64 is not constexpr
    if (!std::is_constant_evaluated()) {
        // NOLINTNEXTLINE(runtime/int)
        if constexpr (sizeof(unsigned long) == 8) {  // 64-bit
            // NOLINTNEXTLINE(runtime/int)
            unsigned long first_bit_index = 0;
            _BitScanReverse64(&first_bit_index, value);
            return first_bit_index;
        } else {  // 32-bit
            // NOLINTNEXTLINE(runtime/int)
            unsigned long first_bit_index = 0;
            if (_BitScanReverse(&first_bit_index, value >> 32)) {
                return first_bit_index + 32;
            }
            _BitScanReverse(&first_bit_index, value & 0xffffffff);
            return first_bit_index;
        }
    }
#endif

    // Non intrinsic (slow) path. Supports constexpr evaluation.
    for (uint64_t clz = 0; clz < 64; clz++) {
        uint64_t bit = 63 - clz;
        if (value & (static_cast<uint64_t>(1u) << bit)) {
            return static_cast<uint32_t>(bit);
        }
    }
    return 64;
}

/// @param value the input value
/// @returns the next power of two number greater or equal to @p value
inline constexpr uint64_t NextPowerOfTwo(uint64_t value) {
    if (value <= 1) {
        return 1;
    } else {
        return static_cast<uint64_t>(1) << (Log2(value - 1) + 1);
    }
}

/// @param value the input value
/// @returns the largest power of two that `value` is a multiple of
template <typename T>
inline std::enable_if_t<std::is_unsigned<T>::value, T> MaxAlignOf(T value) {
    T pot = 1;
    while (value && ((value & 1u) == 0)) {
        pot <<= 1;
        value >>= 1;
    }
    return pot;
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_MATH_MATH_H_
