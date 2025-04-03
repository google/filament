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
#include <type_traits>

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

template <typename destType, typename sourceType>
destType BitCast(const sourceType& source) {
    static_assert(sizeof(destType) == sizeof(sourceType), "BitCast: cannot lose precision.");
    destType output;
    std::memcpy(&output, &source, sizeof(destType));
    return output;
}

uint16_t Float32ToFloat16(float fp32);
float Float16ToFloat32(uint16_t fp16);
bool IsFloat16NaN(uint16_t fp16);

template <typename T>
T FloatToUnorm(float value) {
    return static_cast<T>(value * static_cast<float>(std::numeric_limits<T>::max()));
}

float SRGBToLinear(float srgb);

template <typename T1,
          typename T2,
          typename Enable = typename std::enable_if<sizeof(T1) == sizeof(T2)>::type>
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

// The following functions are defined in the header so they may be inlined.

// Count the 1 bits.
#if DAWN_COMPILER_IS(MSVC) && !DAWN_COMPILER_IS(CLANG)
#if defined(_M_IX86) || defined(_M_X64)
namespace priv {
// Check POPCNT instruction support and cache the result.
// https://docs.microsoft.com/en-us/cpp/intrinsics/popcnt16-popcnt-popcnt64#remarks
static const bool kHasPopcnt = [] {
    int info[4];
    __cpuid(&info[0], 1);
    return static_cast<bool>(info[2] & 0x800000);
}();
}  // namespace priv

// Polyfills for x86/x64 CPUs without POPCNT.
// https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
inline uint32_t BitCountPolyfill(uint32_t bits) {
    bits = bits - ((bits >> 1) & 0x55555555);
    bits = (bits & 0x33333333) + ((bits >> 2) & 0x33333333);
    bits = ((bits + (bits >> 4) & 0x0F0F0F0F) * 0x01010101) >> 24;
    return bits;
}

inline uint32_t BitCountPolyfill(uint64_t bits) {
    bits = bits - ((bits >> 1) & 0x5555555555555555ull);
    bits = (bits & 0x3333333333333333ull) + ((bits >> 2) & 0x3333333333333333ull);
    bits = ((bits + (bits >> 4) & 0x0F0F0F0F0F0F0F0Full) * 0x0101010101010101ull) >> 56;
    return static_cast<uint32_t>(bits);
}

inline uint32_t BitCount(uint32_t bits) {
    if (priv::kHasPopcnt) {
        return __popcnt(bits);
    }
    return BitCountPolyfill(bits);
}

inline uint32_t BitCount(uint64_t bits) {
    if (priv::kHasPopcnt) {
#if defined(_M_X64)
        return static_cast<uint32_t>(__popcnt64(bits));
#else   // x86
        return __popcnt(static_cast<uint32_t>(bits >> 32)) +
               __popcnt(static_cast<uint32_t>(bits)));
#endif  // defined(_M_X64)
    }
    return BitCountPolyfill(bits);
}

#elif defined(_M_ARM) || defined(_M_ARM64)

// MSVC's _CountOneBits* intrinsics are not defined for ARM64, moreover they do not use dedicated
// NEON instructions.

inline uint32_t BitCount(uint32_t bits) {
    // cast bits to 8x8 datatype and use VCNT on it
    const uint8x8_t vsum = vcnt_u8(vcreate_u8(static_cast<uint64_t>(bits)));

    // pairwise sums: 8x8 -> 16x4 -> 32x2
    return vget_lane_u32(vpaddl_u16(vpaddl_u8(vsum)), 0);
}

inline uint32_t BitCount(uint64_t bits) {
    // cast bits to 8x8 datatype and use VCNT on it
    const uint8x8_t vsum = vcnt_u8(vcreate_u8(bits));

    // pairwise sums: 8x8 -> 16x4 -> 32x2 -> 64x1
    return vget_lane_u64(vpaddl_u32(vpaddl_u16(vpaddl_u8(vsum))), 0);
}
#endif  // defined(_M_IX86) || defined(_M_X64)
#endif  // DAWN_COMPILER_IS(MSVC) && !DAWN_COMPILER_IS(CLANG)

#if DAWN_PLATFORM_IS(POSIX) || DAWN_COMPILER_IS(CLANG) || DAWN_COMPILER_IS(GCC)
inline uint32_t BitCount(uint32_t bits) {
    return __builtin_popcount(bits);
}

inline uint32_t BitCount(uint64_t bits) {
    return __builtin_popcountll(bits);
}
#endif  // DAWN_PLATFORM_IS(POSIX) || DAWN_COMPILER_IS(CLANG) || DAWN_COMPILER_IS(GCC)

inline uint32_t BitCount(uint8_t bits) {
    return BitCount(static_cast<uint32_t>(bits));
}

inline uint32_t BitCount(uint16_t bits) {
    return BitCount(static_cast<uint32_t>(bits));
}

#if DAWN_COMPILER_IS(MSVC)
// Return the index of the least significant bit set. Indexing is such that bit 0 is the least
// significant bit. Implemented for different bit widths on different platforms.
inline uint32_t ScanForward(uint32_t bits) {
    DAWN_ASSERT(bits != 0u);
    // NOLINTNEXTLINE(runtime/int)
    unsigned long firstBitIndex = 0ul;
    uint8_t ret = _BitScanForward(&firstBitIndex, bits);
    DAWN_ASSERT(ret != 0u);
    return static_cast<uint32_t>(firstBitIndex);
}

inline uint32_t ScanForward(uint64_t bits) {
    DAWN_ASSERT(bits != 0u);
    // NOLINTNEXTLINE(runtime/int)
    unsigned long firstBitIndex = 0ul;
#if DAWN_PLATFORM_IS(64_BIT)
    uint8_t ret = _BitScanForward64(&firstBitIndex, bits);
#else
    uint8_t ret;
    if (static_cast<uint32_t>(bits) == 0) {
        ret = _BitScanForward(&firstBitIndex, static_cast<uint32_t>(bits >> 32));
        firstBitIndex += 32ul;
    } else {
        ret = _BitScanForward(&firstBitIndex, static_cast<uint32_t>(bits));
    }
#endif  // DAWN_PLATFORM_IS(64_BIT)
    DAWN_ASSERT(ret != 0u);
    return firstBitIndex;
}

// Return the index of the most significant bit set. Indexing is such that bit 0 is the least
// significant bit.
inline uint32_t ScanReverse(uint32_t bits) {
    DAWN_ASSERT(bits != 0u);
    // NOLINTNEXTLINE(runtime/int)
    unsigned long lastBitIndex = 0ul;
    uint8_t ret = _BitScanReverse(&lastBitIndex, bits);
    DAWN_ASSERT(ret != 0u);
    return lastBitIndex;
}

inline uint32_t ScanReverse(uint64_t bits) {
    DAWN_ASSERT(bits != 0u);
    // NOLINTNEXTLINE(runtime/int)
    unsigned long lastBitIndex = 0ul;
#if DAWN_PLATFORM_IS(64_BIT)
    uint8_t ret = _BitScanReverse64(&lastBitIndex, bits);
#else
    uint8_t ret;
    if (static_cast<uint32_t>(bits >> 32) == 0) {
        ret = _BitScanReverse(&lastBitIndex, static_cast<uint32_t>(bits));
    } else {
        ret = _BitScanReverse(&lastBitIndex, static_cast<uint32_t>(bits >> 32));
        lastBitIndex += 32ul;
    }
#endif  // DAWN_PLATFORM_IS(64_BIT)
    DAWN_ASSERT(ret != 0u);
    return lastBitIndex;
}
#else  // DAWN_COMPILER_IS(MSVC)

inline uint32_t ScanForward(uint32_t bits) {
    DAWN_ASSERT(bits != 0u);
    return static_cast<uint32_t>(__builtin_ctz(bits));
}

inline uint32_t ScanForward(uint64_t bits) {
    DAWN_ASSERT(bits != 0u);
#if DAWN_PLATFORM_IS(64_BIT)
    return static_cast<uint32_t>(__builtin_ctzll(bits));
#else
    return static_cast<uint32_t>(static_cast<uint32_t>(bits) == 0
                                     ? __builtin_ctz(static_cast<uint32_t>(bits >> 32)) + 32
                                     : __builtin_ctz(static_cast<uint32_t>(bits)));
#endif  // DAWN_PLATFORM_IS(64_BIT)
}

inline uint32_t ScanReverse(uint32_t bits) {
    DAWN_ASSERT(bits != 0u);
    return static_cast<uint32_t>((sizeof(uint32_t) * CHAR_BIT) - 1 - __builtin_clz(bits));
}

inline uint32_t ScanReverse(uint64_t bits) {
    DAWN_ASSERT(bits != 0u);
#if DAWN_PLATFORM_IS(64_BIT)
    return static_cast<uint32_t>((sizeof(uint64_t) * CHAR_BIT) - 1 - __builtin_clzll(bits));
#else
    if (static_cast<uint32_t>(bits >> 32) == 0) {
        return (sizeof(uint32_t) * CHAR_BIT) - 1 - __builtin_clz(static_cast<uint32_t>(bits));
    } else {
        return (sizeof(uint32_t) * CHAR_BIT) - 1 -
               __builtin_clz(static_cast<uint32_t>(bits >> 32)) + 32;
    }
#endif  // DAWN_PLATFORM_IS(64_BIT)
}

#endif  // !DAWN_COMPILER_IS(MSVC)

inline uint32_t ScanForward(uint8_t bits) {
    return ScanForward(static_cast<uint32_t>(bits));
}

inline uint32_t ScanForward(uint16_t bits) {
    return ScanForward(static_cast<uint32_t>(bits));
}

inline uint32_t ScanReverse(uint8_t bits) {
    return ScanReverse(static_cast<uint32_t>(bits));
}

inline uint32_t ScanReverse(uint16_t bits) {
    return ScanReverse(static_cast<uint32_t>(bits));
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_MATH_H_
