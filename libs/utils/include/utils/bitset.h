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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <algorithm> // for std::fill
#include <iterator>
#include <limits>
#include <type_traits>

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
    T storage[N];

public:
    static constexpr T BITS_PER_WORD = sizeof(T) * 8;
    static constexpr T BIT_COUNT = BITS_PER_WORD * N;
    static constexpr T WORLD_COUNT = N;
    using container_type = T;

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
                v &= ~(T(1) << k);
                return size_t(k + BITS_PER_WORD * i);
            }
        }
        return std::numeric_limits<size_t>::max();
    }

    size_t size() const noexcept { return N * BITS_PER_WORD; }

    bool empty() const noexcept { return none(); }

    bool test(size_t bit) const noexcept { return operator[](bit); }

    void set(size_t b) noexcept {
        assert(b / BITS_PER_WORD < N);
        storage[b / BITS_PER_WORD] |= T(1) << (b % BITS_PER_WORD);
    }

    void set(size_t b, bool value) noexcept {
        assert(b / BITS_PER_WORD < N);
        storage[b / BITS_PER_WORD] &= ~(T(1) << (b % BITS_PER_WORD));
        storage[b / BITS_PER_WORD] |= T(value) << (b % BITS_PER_WORD);
    }

    void unset(size_t b) noexcept {
        assert(b / BITS_PER_WORD < N);
        storage[b / BITS_PER_WORD] &= ~(T(1) << (b % BITS_PER_WORD));
    }

    void flip(size_t b) noexcept {
        assert(b / BITS_PER_WORD < N);
        storage[b / BITS_PER_WORD] ^= T(1) << (b % BITS_PER_WORD);
    }

    void reset() noexcept {
        std::fill(std::begin(storage), std::end(storage), 0);
    }

    void clear() noexcept {
        reset();
    }

    bool operator[](size_t b) const noexcept {
        assert(b / BITS_PER_WORD < N);
        return bool(storage[b / BITS_PER_WORD] & (T(1) << (b % BITS_PER_WORD)));
    }

    size_t count() const noexcept {
#if defined(TNT_UTILS_BITSET_USE_NEON)
        if (BIT_COUNT % 128 == 0 && BIT_COUNT / 128 < 31) {
            // Use NEON for bitset multiple of 128 bits.
            // The intermediate computation can't handle more than 31*128 bits because
            // intermediate counts must be 8 bits.
            uint8x16_t const* const p = (uint8x16_t const*) storage;
            uint8x16_t counts = vcntq_u8(p[0]);
            for (size_t i = 1; i < BIT_COUNT / 128; ++i) {
                counts += vcntq_u8(p[i]);
            }
            return vaddlvq_u8(counts);
        } else
#endif
        {
            T r = utils::popcount(storage[0]);
            for (size_t i = 1; i < N; ++i) {
                r += utils::popcount(storage[i]);
            }
            return r;
        }
    }

    bool any() const noexcept {
#if defined(TNT_UTILS_BITSET_USE_NEON)
        if (BIT_COUNT % 128 == 0) {
            uint64x2_t const* const p = (uint64x2_t const*) storage;
            uint64x2_t r = p[0];
            for (size_t i = 1; i < BIT_COUNT / 128; ++i) {
                r |= p[i];
            }
            return bool(r[0] | r[1]);
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
            uint64x2_t const* const p = (uint64x2_t const*) storage;
            uint64x2_t r = p[0];
            for (size_t i = 1; i < BIT_COUNT / 128; ++i) {
                r &= p[i];
            }
            return T(~(r[0] & r[1])) == T(0);
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
            bitset temp(*this ^ b);
            uint64x2_t const* const p = (uint64x2_t const*) temp.storage;
            uint64x2_t r = p[0];
            for (size_t i = 1; i < BIT_COUNT / 128; ++i) {
                r |= p[i];
            }
            return bool(r[0] | r[1]);
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
            uint8x16_t* const p = (uint8x16_t*) storage;
            uint8x16_t const* const q = (uint8x16_t const*) b.storage;
            for (size_t i = 0; i < BIT_COUNT / 128; ++i) {
                p[i] &= q[i];
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
            uint8x16_t* const p = (uint8x16_t*) storage;
            uint8x16_t const* const q = (uint8x16_t const*) b.storage;
            for (size_t i = 0; i < BIT_COUNT / 128; ++i) {
                p[i] |= q[i];
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
            uint8x16_t* const p = (uint8x16_t*) storage;
            uint8x16_t const* const q = (uint8x16_t const*) b.storage;
            for (size_t i = 0; i < BIT_COUNT / 128; ++i) {
                p[i] ^= q[i];
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
            uint8x16_t* const p = (uint8x16_t*) r.storage;
            uint8x16_t const* const q = (uint8x16_t const*) storage;
            for (size_t i = 0; i < BIT_COUNT / 128; ++i) {
                p[i] = ~q[i];
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

private:
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

} // namespace utils

#endif // TNT_UTILS_BITSET_H
