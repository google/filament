/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_UTILS_BITSET_H
#define TNT_UTILS_BITSET_H

#include <utils/algorithm.h>
#include <utils/compiler.h>

#include <algorithm> // for std::fill
#include <iterator>
#include <limits>
#include <type_traits>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(__ARM_NEON)
#   if defined(__ARM_ACLE) && defined(__aarch64__)
#       include <arm_neon.h>
#       define TNT_UTILS_BITSET_USE_NEON 1
#   endif
#endif

namespace utils {

/*
 * This bitset<> class is different from std::bitset<> in that it allows us to control
 * the exact storage size. This is useful for small bitset (e.g. < 64, on 64-bits machines).
 * It also allows for lexicographical compares (i.e. sorting).
 */

template<typename T, size_t N = 1,
        typename = std::enable_if_t<std::is_integral_v<T> &&
                                           std::is_unsigned_v<T>>>
class UTILS_PUBLIC bitset {
public:
    static constexpr size_t BITS_PER_WORD = sizeof(T) * 8;
    static constexpr size_t BIT_COUNT = BITS_PER_WORD * N;
    static constexpr size_t WORD_COUNT = N;
    using container_type = T;

private:
    // Naturally aligned on purpose: types containing a bitset live in pool allocators (e.g.
    // HandleAllocator) that only guarantee alignof(std::max_align_t), which can be 8.
    // The NEON paths therefore use vld1q/vst1q, which don't require 16-byte alignment.
    T storage[N];

#if defined(TNT_UTILS_BITSET_USE_NEON)
    static uint8x16_t load(T const* s, size_t i) noexcept {
        return vld1q_u8(reinterpret_cast<uint8_t const*>(s) + i * 16);
    }
    static void store(T* s, size_t i, uint8x16_t v) noexcept {
        vst1q_u8(reinterpret_cast<uint8_t*>(s) + i * 16, v);
    }
#endif

public:
    bitset() noexcept {
        std::fill(std::begin(storage), std::end(storage), 0);
    }

    template<typename U, typename = typename std::enable_if_t<N == 1, U>>
    explicit bitset(U value) noexcept {
        storage[0] = value;
    }

    T getBitsAt(size_t n) const noexcept {
        assert(n<N);
        return storage[n];
    }

    T& getBitsAt(size_t n) noexcept {
        assert(n<N);
        return storage[n];
    }

    T getValue() const noexcept {
        static_assert(N == 1, "bitfield must only have one storage word");
        return storage[0];
    }

    void setValue(T value) noexcept {
        static_assert(N == 1, "bitfield must only have one storage word");
        storage[0] = value;
    }

    template<typename F>
    void forEachSetBit(F exec) const noexcept {
        for (size_t i = 0; i < N; i++) {
            T v = storage[i];
            while (v) {
                T k = utils::ctz(v);
                v &= ~(T(1) << k);
                exec(size_t(k + BITS_PER_WORD * i));
            }
        }
    }

    size_t firstSetBit() const noexcept {
        for (size_t i = 0; i < N; i++) {
            if (T v = storage[i]) {
                T k = utils::ctz(v);
                return size_t(k + BITS_PER_WORD * i);
            }
        }
        return std::numeric_limits<size_t>::max();
    }

    size_t size() const noexcept { return N * BITS_PER_WORD; }

    bool empty() const noexcept { return none(); }

    bool test(size_t const bit) const noexcept { return operator[](bit); }

    void set(size_t const b) noexcept {
        assert(b / BITS_PER_WORD < N);
        storage[b / BITS_PER_WORD] |= T(1) << (b % BITS_PER_WORD);
    }

    void set(size_t const b, bool value) noexcept {
        assert(b / BITS_PER_WORD < N);
        storage[b / BITS_PER_WORD] &= ~(T(1) << (b % BITS_PER_WORD));
        storage[b / BITS_PER_WORD] |= T(value) << (b % BITS_PER_WORD);
    }

    void unset(size_t const b) noexcept {
        assert(b / BITS_PER_WORD < N);
        storage[b / BITS_PER_WORD] &= ~(T(1) << (b % BITS_PER_WORD));
    }

    void flip(size_t const b) noexcept {
        assert(b / BITS_PER_WORD < N);
        storage[b / BITS_PER_WORD] ^= T(1) << (b % BITS_PER_WORD);
    }

    void reset() noexcept {
        std::fill(std::begin(storage), std::end(storage), 0);
    }

    void clear() noexcept {
        reset();
    }

    bool operator[](size_t const b) const noexcept {
        assert(b / BITS_PER_WORD < N);
        return bool(storage[b / BITS_PER_WORD] & (T(1) << (b % BITS_PER_WORD)));
    }

    size_t count() const noexcept {
#if defined(TNT_UTILS_BITSET_USE_NEON)
        if (BIT_COUNT % 128 == 0 && BIT_COUNT / 128 < 31) {
            // Use NEON for bitset multiple of 128 bits.
            // The intermediate computation can't handle more than 31*128 bits because
            // intermediate counts must be 8 bits.
            uint8x16_t counts = vcntq_u8(load(storage, 0));
            for (size_t i = 1; i < BIT_COUNT / 128; ++i) {
                counts += vcntq_u8(load(storage, i));
            }
            return vaddlvq_u8(counts);
        } else
#endif
        {
            size_t r = utils::popcount(storage[0]);
            for (size_t i = 1; i < N; ++i) {
                r += utils::popcount(storage[i]);
            }
            return r;
        }
    }

    bool any() const noexcept {
#if defined(TNT_UTILS_BITSET_USE_NEON)
        if (BIT_COUNT % 128 == 0) {
            uint8x16_t r = load(storage, 0);
            for (size_t i = 1; i < BIT_COUNT / 128; ++i) {
                r |= load(storage, i);
            }
            return vmaxvq_u8(r) != 0;
        } else
#endif
        {
            T r = storage[0];
            for (size_t i = 1; i < N; ++i) {
                r |= storage[i];
            }
            return bool(r);
        }
    }

    bool none() const noexcept {
        return !any();
    }

    bool all() const noexcept {
#if defined(TNT_UTILS_BITSET_USE_NEON)
        if (BIT_COUNT % 128 == 0) {
            uint8x16_t r = load(storage, 0);
            for (size_t i = 1; i < BIT_COUNT / 128; ++i) {
                r &= load(storage, i);
            }
            return vminvq_u8(r) == 0xFF;
        } else
#endif
        {
            T r = storage[0];
            for (size_t i = 1; i < N; ++i) {
                r &= storage[i];
            }
            return T(~r) == T(0);
        }
    }

    bool operator!=(const bitset& b) const noexcept {
#if defined(TNT_UTILS_BITSET_USE_NEON)
        if (BIT_COUNT % 128 == 0) {
            uint8x16_t r = load(storage, 0) ^ load(b.storage, 0);
            for (size_t i = 1; i < BIT_COUNT / 128; ++i) {
                r |= load(storage, i) ^ load(b.storage, i);
            }
            return vmaxvq_u8(r) != 0;
        } else
#endif
        {
            T r = storage[0] ^ b.storage[0];
            for (size_t i = 1; i < N; ++i) {
                r |= storage[i] ^ b.storage[i];
            }
            return bool(r);
        }
    }

    bool operator==(const bitset& b) const noexcept {
        return !operator!=(b);
    }

    bitset& operator&=(const bitset& b) noexcept {
#if defined(TNT_UTILS_BITSET_USE_NEON)
        if (BIT_COUNT % 128 == 0) {
            for (size_t i = 0; i < BIT_COUNT / 128; ++i) {
                store(storage, i, load(storage, i) & load(b.storage, i));
            }
        } else
#endif
        {
            for (size_t i = 0; i < N; ++i) {
                storage[i] &= b.storage[i];
            }
        }
        return *this;
    }

    bitset& operator|=(const bitset& b) noexcept {
#if defined(TNT_UTILS_BITSET_USE_NEON)
        if (BIT_COUNT % 128 == 0) {
            for (size_t i = 0; i < BIT_COUNT / 128; ++i) {
                store(storage, i, load(storage, i) | load(b.storage, i));
            }
        } else
#endif
        {
            for (size_t i = 0; i < N; ++i) {
                storage[i] |= b.storage[i];
            }
        }
        return *this;
    }

    bitset& operator^=(const bitset& b) noexcept {
#if defined(TNT_UTILS_BITSET_USE_NEON)
        if (BIT_COUNT % 128 == 0) {
            for (size_t i = 0; i < BIT_COUNT / 128; ++i) {
                store(storage, i, load(storage, i) ^ load(b.storage, i));
            }
        } else
#endif
        {
            for (size_t i = 0; i < N; ++i) {
                storage[i] ^= b.storage[i];
            }
        }
        return *this;
    }

    bitset operator~() const noexcept {
        bitset r;
#if defined(TNT_UTILS_BITSET_USE_NEON)
        if (BIT_COUNT % 128 == 0) {
            for (size_t i = 0; i < BIT_COUNT / 128; ++i) {
                store(r.storage, i, ~load(storage, i));
            }
        } else
#endif
        {
            for (size_t i = 0; i < N; ++i) {
                r.storage[i] = ~storage[i];
            }
        }
        return r;
    }

    // Shifts all bits toward higher bit indices. Bits shifted past BIT_COUNT are discarded and
    // vacated bits are filled with zeros. Shifting by BIT_COUNT or more clears the bitset.
    bitset& operator<<=(size_t const n) noexcept {
        shiftLeft(storage, storage, n);
        return *this;
    }

    // Shifts all bits toward lower bit indices. Bits shifted below index 0 are discarded and
    // vacated bits are filled with zeros. Shifting by BIT_COUNT or more clears the bitset.
    bitset& operator>>=(size_t const n) noexcept {
        shiftRight(storage, storage, n);
        return *this;
    }

    bitset operator<<(size_t const n) const noexcept {
        bitset r{ Uninitialized{} };
        shiftLeft(r.storage, storage, n);
        return r;
    }

    bitset operator>>(size_t const n) const noexcept {
        bitset r{ Uninitialized{} };
        shiftRight(r.storage, storage, n);
        return r;
    }

private:
    struct Uninitialized {};
    explicit bitset(Uninitialized) noexcept {}

    enum class ShiftDirection { LEFT, RIGHT };

    // 128-bit bitsets are shifted as a single __uint128_t, which compiles to a few native
    // instructions. Little-endian byte order is required so that storage[0] maps onto the low
    // bits of the integer. The sizeof(void*) == 8 check is intentional and is not implied by
    // __SIZEOF_INT128__: some 32-bit targets such as wasm32 also provide __int128 but lower its
    // shifts to library calls, so they are better served by the generic word loop.
#if defined(__SIZEOF_INT128__) && defined(__BYTE_ORDER__) && \
        defined(__ORDER_LITTLE_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    static constexpr bool USE_UINT128_SHIFT =
            sizeof(void*) == 8 && N * sizeof(T) == sizeof(__uint128_t);

    template<ShiftDirection DIRECTION>
    static void shift128(T* dst, T const* src, size_t const n) noexcept {
        __uint128_t v;
        memcpy(&v, src, sizeof(v));
        if constexpr (DIRECTION == ShiftDirection::LEFT) {
            v = (n < BIT_COUNT) ? (v << n) : 0;
        } else {
            v = (n < BIT_COUNT) ? (v >> n) : 0;
        }
        memcpy(dst, &v, sizeof(v));
    }
#else
    static constexpr bool USE_UINT128_SHIFT = false;

    // Declared but never defined or instantiated. It only needs to exist so that the discarded
    // USE_UINT128_SHIFT branches in shiftLeft() and shiftRight() still parse.
    template<ShiftDirection DIRECTION>
    static void shift128(T* dst, T const* src, size_t n) noexcept;
#endif

    static void shiftLeft(T* dst, T const* src, size_t const n) noexcept {
        if constexpr (N == 1) {
            dst[0] = (n < BIT_COUNT) ? T(src[0] << n) : T(0);
        } else if constexpr (USE_UINT128_SHIFT) {
            shift128<ShiftDirection::LEFT>(dst, src, n);
        } else {
            if (n >= BIT_COUNT) {
                std::fill_n(dst, N, T(0));
                return;
            }
            size_t const ws = n / BITS_PER_WORD;
            size_t const bs = n % BITS_PER_WORD;
            size_t const rbs = BITS_PER_WORD - 1 - bs;
            // Iterate from the most significant word down so in-place shifts (dst == src) read
            // source words before overwriting them. Shifting low right in two steps avoids
            // undefined behavior when bs == 0 without introducing a branch.
            T high = src[N - 1 - ws];
            for (size_t i = N - 1; i > ws; --i) {
                T const low = src[i - 1 - ws];
                dst[i] = T(high << bs) | T(T(low >> 1) >> rbs);
                high = low;
            }
            dst[ws] = T(high << bs);
            std::fill_n(dst, ws, T(0));
        }
    }

    static void shiftRight(T* dst, T const* src, size_t const n) noexcept {
        if constexpr (N == 1) {
            dst[0] = (n < BIT_COUNT) ? T(src[0] >> n) : T(0);
        } else if constexpr (USE_UINT128_SHIFT) {
            shift128<ShiftDirection::RIGHT>(dst, src, n);
        } else {
            if (n >= BIT_COUNT) {
                std::fill_n(dst, N, T(0));
                return;
            }
            size_t const ws = n / BITS_PER_WORD;
            size_t const bs = n % BITS_PER_WORD;
            size_t const lbs = BITS_PER_WORD - 1 - bs;
            size_t const limit = N - 1 - ws;
            // Iterate from the least significant word up so in-place shifts (dst == src) read
            // source words before overwriting them. Shifting high left in two steps avoids
            // undefined behavior when bs == 0 without introducing a branch.
            T low = src[ws];
            for (size_t i = 0; i < limit; ++i) {
                T const high = src[i + ws + 1];
                dst[i] = T(low >> bs) | T(T(high << 1) << lbs);
                low = high;
            }
            dst[limit] = T(low >> bs);
            std::fill_n(dst + limit + 1, ws, T(0));
        }
    }
    friend bool operator<(bitset const& lhs, bitset const& rhs) noexcept {
        return std::lexicographical_compare(
                std::begin(lhs.storage), std::end(lhs.storage),
                std::begin(rhs.storage), std::end(rhs.storage)
        );
    }

    friend bitset operator&(const bitset& lhs, const bitset& rhs) noexcept {
        return bitset(lhs) &= rhs;
    }

    friend bitset operator|(const bitset& lhs, const bitset& rhs) noexcept {
        return bitset(lhs) |= rhs;
    }

    friend bitset operator^(const bitset& lhs, const bitset& rhs) noexcept {
        return bitset(lhs) ^= rhs;
    }
};

using bitset8 = bitset<uint8_t>;
using bitset32 = bitset<uint32_t>;
using bitset64 = bitset<uint64_t>;
using bitset128 = bitset<uint64_t, 2>;
using bitset256 = bitset<uint64_t, 4>;

static_assert(sizeof(bitset8) == 1, "bitset8 isn't 8 bits!");
static_assert(sizeof(bitset32) == 4, "bitset32 isn't 32 bits!");
static_assert(sizeof(bitset64) == 8, "bitset64 isn't 64 bits!");
static_assert(sizeof(bitset128) == 16, "bitset128 isn't 128 bits!");
static_assert(sizeof(bitset256) == 32, "bitset256 isn't 256 bits!");
static_assert(alignof(bitset128) == alignof(uint64_t), "bitset128 must not be over-aligned");
static_assert(alignof(bitset256) == alignof(uint64_t), "bitset256 must not be over-aligned");

static_assert(std::is_trivially_copyable_v<bitset8>, "bitset8 must be trivially copyable");
static_assert(std::is_trivially_copyable_v<bitset64>, "bitset64 must be trivially copyable");
static_assert(std::is_trivially_copyable_v<bitset128>, "bitset128 must be trivially copyable");
static_assert(std::is_trivially_copyable_v<bitset256>, "bitset256 must be trivially copyable");

} // namespace utils

#endif // TNT_UTILS_BITSET_H
