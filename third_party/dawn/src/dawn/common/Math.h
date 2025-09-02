// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_MATH_H_
#define SRC_DAWN_COMMON_MATH_H_

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <limits>
#include <optional>

#include "dawn/common/Assert.h"
#include "dawn/common/Platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

#if DAWN_COMPILER_IS(MSVC)
#include <intrin.h>
#endif

namespace dawn {

// The following are not valid for 0
uint32_t Log2(uint32_t value);
uint32_t Log2(uint64_t value);
bool IsPowerOfTwo(uint64_t n);
uint64_t RoundUp(uint64_t n, uint64_t m);

constexpr uint32_t ConstexprLog2(uint64_t v) {
    return v <= 1 ? 0 : 1 + ConstexprLog2(v / 2);
}

constexpr uint32_t ConstexprLog2Ceil(uint64_t v) {
    return v <= 1 ? 0 : ConstexprLog2(v - 1) + 1;
}

inline uint32_t Log2Ceil(uint32_t v) {
    return v <= 1 ? 0 : Log2(v - 1) + 1;
}

inline uint32_t Log2Ceil(uint64_t v) {
    return v <= 1 ? 0 : Log2(v - 1) + 1;
}

uint64_t NextPowerOfTwo(uint64_t n);
bool IsPtrAligned(const void* ptr, size_t alignment);
bool IsAligned(uint32_t value, size_t alignment);

template <typename T>
T Align(T value, size_t alignment) {
    DAWN_ASSERT(value <= std::numeric_limits<T>::max() - (alignment - 1));
    DAWN_ASSERT(IsPowerOfTwo(alignment));
    DAWN_ASSERT(alignment != 0);
    T alignmentT = static_cast<T>(alignment);
    return (value + (alignmentT - 1)) & ~(alignmentT - 1);
}

template <typename T>
T AlignDown(T value, size_t alignment) {
    DAWN_ASSERT(IsPowerOfTwo(alignment));
    DAWN_ASSERT(alignment != 0);
    T alignmentT = static_cast<T>(alignment);
    return value & ~(alignmentT - 1);
}

template <typename T, size_t Alignment>
constexpr size_t AlignSizeof() {
    static_assert(Alignment != 0 && (Alignment & (Alignment - 1)) == 0,
                  "Alignment must be a valid power of 2.");
    static_assert(sizeof(T) <= std::numeric_limits<size_t>::max() - (Alignment - 1));
    return (sizeof(T) + (Alignment - 1)) & ~(Alignment - 1);
}

// Returns an aligned size for an n-sized array of T elements. If the size would overflow, returns
// nullopt instead.
template <typename T, size_t Alignment>
std::optional<size_t> AlignSizeofN(uint64_t n) {
    constexpr uint64_t kMaxCountWithoutOverflows =
        (std::numeric_limits<size_t>::max() - Alignment + 1) / sizeof(T);
    if (n > kMaxCountWithoutOverflows) {
        return std::nullopt;
    }
    return Align(sizeof(T) * n, Alignment);
}

template <typename T>
DAWN_FORCE_INLINE T* AlignPtr(T* ptr, size_t alignment) {
    DAWN_ASSERT(IsPowerOfTwo(alignment));
    DAWN_ASSERT(alignment != 0);
    return reinterpret_cast<T*>((reinterpret_cast<size_t>(ptr) + (alignment - 1)) &
                                ~(alignment - 1));
}

template <typename T, partition_alloc::internal::RawPtrTraits Traits>
DAWN_FORCE_INLINE T* AlignPtr(raw_ptr<T, Traits> ptr, size_t alignment) {
    return AlignPtr(ptr.get(), alignment);
}

template <typename T>
DAWN_FORCE_INLINE const T* AlignPtr(const T* ptr, size_t alignment) {
    DAWN_ASSERT(IsPowerOfTwo(alignment));
    DAWN_ASSERT(alignment != 0);
    return reinterpret_cast<const T*>((reinterpret_cast<size_t>(ptr) + (alignment - 1)) &
                                      ~(alignment - 1));
}

uint16_t Float32ToFloat16(float fp32);
float Float16ToFloat32(uint16_t fp16);
bool IsFloat16NaN(uint16_t fp16);

template <typename T>
T FloatToUnorm(float value) {
    return static_cast<T>(value * static_cast<float>(std::numeric_limits<T>::max()));
}

float SRGBToLinear(float srgb);

template <typename T1, typename T2>
    requires(sizeof(T1) == sizeof(T2))
constexpr bool IsSubset(T1 subset, T2 set) {
    T2 bitsAlsoInSet = subset & set;
    return bitsAlsoInSet == subset;
}

template <typename T>
constexpr T Max(T a, T b) {
    return (a > b) ? a : b;
}

template <typename T, typename... Args>
constexpr T Max(T first, Args... rest) {
    return Max(first, Max(rest...));
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_MATH_H_
