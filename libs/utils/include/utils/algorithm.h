/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_UTILS_ALGORITHM_H
#define TNT_UTILS_ALGORITHM_H

#include <utils/compiler.h>

#include <functional>

#include <limits.h>
#include <stdint.h>

namespace utils {

namespace details {
template<typename T>
constexpr inline T popcount(T v) noexcept {
    static_assert(sizeof(T) * 8 <= 128, "details::popcount() only support up to 128 bits");
    constexpr T ONES = ~T(0);
    v = v - ((v >> 1) & ONES / 3);
    v = (v & ONES / 15 * 3) + ((v >> 2) & ONES / 15 * 3);
    v = (v + (v >> 4)) & ONES / 255 * 15;
    return (T) (v * (ONES / 255)) >> (sizeof(T) - 1) * CHAR_BIT;
}

template<typename T>
constexpr inline T clz(T x) noexcept {
    static_assert(sizeof(T) <= sizeof(uint64_t), "details::clz() only support up to 64 bits");
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    if (sizeof(T) * 8 > 32) { // if() only needed to quash compiler warnings
        x |= (x >> 32);
    }
    return (sizeof(T) * CHAR_BIT) - details::popcount(x);
}

template<typename T>
constexpr inline T ctz(T x) noexcept {
    static_assert(sizeof(T) <= sizeof(uint64_t), "details::ctz() only support up to 64 bits");
    T c = sizeof(T) * 8;
    x &= -signed(x);
    if (x) c--;
    if (sizeof(T) * 8 > 32) { // if() only needed to quash compiler warnings
        if (x & 0x00000000FFFFFFFF) c -= 32;
    }
    if (x & 0x0000FFFF) c -= 16;
    if (x & 0x00FF00FF) c -= 8;
    if (x & 0x0F0F0F0F) c -= 4;
    if (x & 0x33333333) c -= 2;
    if (x & 0x55555555) c -= 1;
    return c;
}
} // namespace details

constexpr inline UTILS_PUBLIC UTILS_PURE
unsigned int UTILS_ALWAYS_INLINE clz(unsigned int x) noexcept {
#if __has_builtin(__builtin_clz)
    return __builtin_clz(x);
#else
    return details::clz(x);
#endif
}

constexpr inline UTILS_PUBLIC UTILS_PURE
unsigned long UTILS_ALWAYS_INLINE clz(unsigned long x) noexcept {
#if __has_builtin(__builtin_clzl)
    return __builtin_clzl(x);
#else
    return details::clz(x);
#endif
}

constexpr inline UTILS_PUBLIC UTILS_PURE
unsigned long long UTILS_ALWAYS_INLINE clz(unsigned long long x) noexcept {
#if __has_builtin(__builtin_clzll)
    return __builtin_clzll(x);
#else
    return details::clz(x);
#endif
}

constexpr inline UTILS_PUBLIC UTILS_PURE
unsigned int UTILS_ALWAYS_INLINE ctz(unsigned int x) noexcept {
#if __has_builtin(__builtin_ctz)
    return __builtin_ctz(x);
#else
    return details::ctz(x);
#endif
}

constexpr inline UTILS_PUBLIC UTILS_PURE
unsigned long UTILS_ALWAYS_INLINE ctz(unsigned long x) noexcept {
#if __has_builtin(__builtin_ctzl)
    return __builtin_ctzl(x);
#else
    return details::ctz(x);
#endif
}

constexpr inline UTILS_PUBLIC UTILS_PURE
unsigned long long UTILS_ALWAYS_INLINE ctz(unsigned long long x) noexcept {
#if __has_builtin(__builtin_ctzll)
    return __builtin_ctzll(x);
#else
    return details::ctz(x);
#endif
}

constexpr inline UTILS_PUBLIC UTILS_PURE
unsigned int UTILS_ALWAYS_INLINE popcount(unsigned int x) noexcept {
#if __has_builtin(__builtin_popcount)
    return __builtin_popcount(x);
#else
    return details::popcount(x);
#endif
}

constexpr inline UTILS_PUBLIC UTILS_PURE
unsigned long UTILS_ALWAYS_INLINE popcount(unsigned long x) noexcept {
#if __has_builtin(__builtin_popcountl)
    return __builtin_popcountl(x);
#else
    return details::popcount(x);
#endif
}

constexpr inline UTILS_PUBLIC UTILS_PURE
unsigned long long UTILS_ALWAYS_INLINE popcount(unsigned long long x) noexcept {
#if __has_builtin(__builtin_popcountll)
    return __builtin_popcountll(x);
#else
    return details::popcount(x);
#endif
}

constexpr inline UTILS_PUBLIC UTILS_PURE
uint8_t UTILS_ALWAYS_INLINE popcount(uint8_t x) noexcept {
    return (uint8_t)popcount((unsigned int)x);
}

template<typename T,
        typename = std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>>
constexpr inline UTILS_PUBLIC UTILS_PURE
T log2i(T x) noexcept {
    return (sizeof(x) * 8 - 1u) - clz(x);
}

/*
 * branch-less version of std::lower_bound and std::upper_bound.
 * These versions are intended to be fully inlined, which only happens when the size
 * of the array is known at compile time. This code also performs better if the
 * array is a power-of-two in size.
 *
 * These code works even if the conditions above are not met, and becomes a less-branches
 * algorithm instead of a branch-less one!
 */

template<typename RandomAccessIterator, typename T, typename COMPARE = typename std::less<T>>
inline UTILS_PUBLIC
RandomAccessIterator lower_bound(
        RandomAccessIterator first, RandomAccessIterator last, const T& value,
        COMPARE comp = std::less<T>(),
        bool assume_power_of_two = false) {
    size_t len = last - first;

    if (!assume_power_of_two) {
        // handle non power-of-two sized arrays. If it's POT, the next line is a no-op
        // and gets optimized out if the size is known at compile time.
        len = 1u << (31 - clz(uint32_t(len)));     // next power of two length / 2
        size_t difference = (last - first) - len;
        // If len was already a POT, then difference will be 0.
        // We need to explicitly check this case to avoid dereferencing past the end of the array
        first += !difference || comp(first[len], value) ? difference : 0;
    }

    while (len) {
        // The number of repetitions here doesn't affect the result. We manually unroll the loop
        // twice, to guarantee we have at least two iterations without branches (for the case
        // where the size is not known at compile time
        first += comp(first[len>>=1], value) ? len : 0;
        first += comp(first[len>>=1], value) ? len : 0;
    }
    first += comp(*first, value);
    return first;
}

template<typename RandomAccessIterator, typename T, typename COMPARE = typename std::less<T>>
inline UTILS_PUBLIC
RandomAccessIterator upper_bound(
        RandomAccessIterator first, RandomAccessIterator last,
        const T& value, COMPARE comp = std::less<T>(),
        bool assume_power_of_two = false) {
    size_t len = last - first;

    if (!assume_power_of_two) {
        // handle non power-of-two sized arrays. If it's POT, the next line is a no-op
        // and gets optimized out if the size is known at compile time.
        len = 1u << (31 - clz(uint32_t(len)));     // next power of two length / 2
        size_t difference = (last - first) - len;
        // If len was already a POT, then difference will be 0.
        // We need to explicitly check this case to avoid dereferencing past the end of the array
        first += !difference || comp(value, first[len]) ? 0 : difference;
    }

    while (len) {
        // The number of repetitions here doesn't affect the result. We manually unroll the loop
        // twice, to guarantee we have at least two iterations without branches (for the case
        // where the size is not known at compile time
        first += !comp(value, first[len>>=1]) ? len : 0;
        first += !comp(value, first[len>>=1]) ? len : 0;
    }
    first += !comp(value, *first);
    return first;
}

template<typename RandomAccessIterator, typename COMPARE>
inline UTILS_PUBLIC
RandomAccessIterator partition_point(
        RandomAccessIterator first, RandomAccessIterator last, COMPARE pred,
        bool assume_power_of_two = false) {
    size_t len = last - first;

    if (!assume_power_of_two) {
        // handle non power-of-two sized arrays. If it's POT, the next line is a no-op
        // and gets optimized out if the size is known at compile time.
        len = 1u << (31 - clz(uint32_t(len)));     // next power of two length / 2
        first += pred(first[len]) ? ((last - first) - len) : 0;
    }

    while (len) {
        // The number of repetitions here doesn't affect the result. We manually unroll the loop
        // twice, to guarantee we have at least two iterations without branches (for the case
        // where the size is not known at compile time
        first += pred(first[len>>=1]) ? len : 0;
        first += pred(first[len>>=1]) ? len : 0;
    }
    first += pred(*first);
    return first;
}

template <class To, class From>
typename std::enable_if_t<
    (sizeof(To) == sizeof(From)) &&
    std::is_trivially_copyable<From>::value,
    To>
// constexpr support needs compiler magic
bit_cast(const From &src) noexcept {
    return reinterpret_cast<const To&>(src);
}

} // namespace utils

#endif // TNT_UTILS_ALGORITHM_H
