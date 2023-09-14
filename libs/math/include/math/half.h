/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_MATH_HALF_H
#define TNT_MATH_HALF_H

#include <math/compiler.h>

#include <limits>
#include <type_traits>

#include <stddef.h>
#include <stdint.h>

namespace filament {
namespace math {

template<unsigned S, unsigned E, unsigned M>
class fp {
    static_assert(S + E + M <= 16, "we only support 16-bits max custom floats");

    using TYPE = uint16_t; // this should be dynamic

    static constexpr unsigned S_SHIFT = E + M;
    static constexpr unsigned E_SHIFT = M;
    static constexpr unsigned M_SHIFT = 0;
    static constexpr unsigned S_MASK = ((1u << S) - 1u) << S_SHIFT;
    static constexpr unsigned E_MASK = ((1u << E) - 1u) << E_SHIFT;
    static constexpr unsigned M_MASK = ((1u << M) - 1u) << M_SHIFT;

    struct fp32 {
        explicit constexpr fp32(float f) noexcept : fp(f) { } // NOLINT
        explicit constexpr fp32(uint32_t b) noexcept : bits(b) { } // NOLINT
        constexpr void setS(unsigned int s) noexcept {
            bits = uint32_t((bits & 0x7FFFFFFFu) | (s << 31u));
        }
        constexpr unsigned int getS() const noexcept { return bits >> 31u; }
        constexpr unsigned int getE() const noexcept { return (bits >> 23u) & 0xFFu; }
        constexpr unsigned int getM() const noexcept { return bits & 0x7FFFFFu; }
        union {
            uint32_t bits;
            float fp;
        };
    };

public:
    static constexpr fp fromf(float f) noexcept {
        fp out;
        if (S == 0 && f < 0.0f) {
            return out;
        }

        fp32 in(f);
        unsigned int sign = in.getS();
        in.setS(0);
        if (MATH_UNLIKELY(in.getE() == 0xFF)) { // inf or nan
            out.setE((1u << E) - 1u);
            out.setM(in.getM() ? (1u << (M - 1u)) : 0);
        } else {
            constexpr fp32 infinity(((1u << E) - 1u) << 23u);       // fp infinity in fp32 position
            constexpr fp32 magic(((1u << (E - 1u)) - 1u) << 23u);   // exponent offset
            in.bits &= ~((1u << (22 - M)) - 1u);                    // erase extra mantissa bits
            in.bits += 1u << (22 - M);                              // rounding
            in.fp *= magic.fp;                      // add exponent offset
            in.bits = in.bits < infinity.bits ? in.bits : infinity.bits;
            out.bits = uint16_t(in.bits >> (23 - M));
        }
        out.setS(sign);
        return out;
    }

    static constexpr float tof(fp in) noexcept {
        constexpr fp32 magic ((0xFE - ((1u << (E - 1u)) - 1u)) << 23u);
        constexpr fp32 infnan((0x80 + ((1u << (E - 1u)) - 1u)) << 23u);
        fp32 out((in.bits & ((1u << (E + M)) - 1u)) << (23u - M));
        out.fp *= magic.fp;
        if (out.fp >= infnan.fp) {
            out.bits |= 0xFFu << 23u;
        }
        out.bits |= (in.bits & S_MASK) << (31u - S_SHIFT);
        return out.fp;
    }

    TYPE bits{};
    static constexpr size_t getBitCount() noexcept { return S + E + M; }
    constexpr fp() noexcept = default;
    explicit constexpr fp(TYPE bits) noexcept : bits(bits) { }
    constexpr void setS(unsigned int s) noexcept { bits = TYPE((bits & ~S_MASK) | (s << S_SHIFT)); }
    constexpr void setE(unsigned int s) noexcept { bits = TYPE((bits & ~E_MASK) | (s << E_SHIFT)); }
    constexpr void setM(unsigned int s) noexcept { bits = TYPE((bits & ~M_MASK) | (s << M_SHIFT)); }
    constexpr unsigned int getS() const noexcept { return (bits & S_MASK) >> S_SHIFT; }
    constexpr unsigned int getE() const noexcept { return (bits & E_MASK) >> E_SHIFT; }
    constexpr unsigned int getM() const noexcept { return (bits & M_MASK) >> M_SHIFT; }
};

/*
 * half-float
 *
 *  1   5       10
 * +-+------+------------+
 * |s|eee.ee|mm.mmmm.mmmm|
 * +-+------+------------+
 *
 * minimum (denormal) value: 2^-24 = 5.96e-8
 * minimum (normal) value:   2^-14 = 6.10e-5
 * maximum value:            (2 - 2^-10) * 2^15 = 65504
 *
 * Integers between 0 and 2048 can be represented exactly
 */

#ifdef __ARM_NEON

using half = __fp16;

inline constexpr  uint16_t getBits(half const& h) noexcept {
    return MAKE_CONSTEXPR(reinterpret_cast<uint16_t const&>(h));
}

inline constexpr half makeHalf(uint16_t bits) noexcept {
    return MAKE_CONSTEXPR(reinterpret_cast<half const&>(bits));
}

#else

class half {
    using fp16 = fp<1, 5, 10>;

public:
    half() = default;
    constexpr half(float v) noexcept : mBits(fp16::fromf(v)) { } // NOLINT
    constexpr operator float() const noexcept { return fp16::tof(mBits); } // NOLINT

private:
    // these are friends, not members (and they're not "private")
    friend constexpr uint16_t getBits(half const& h) noexcept { return h.mBits.bits; }
    friend constexpr inline half makeHalf(uint16_t bits) noexcept;

    enum Binary { binary };
    explicit constexpr half(Binary, uint16_t bits) noexcept : mBits(bits) { }

    fp16 mBits;
};

constexpr inline half makeHalf(uint16_t bits) noexcept {
    return half(half::binary, bits);
}

#endif // __ARM_NEON

inline constexpr half operator""_h(long double v) {
    return half( static_cast<float>(v) );
}

template<> struct is_arithmetic<filament::math::half> : public std::true_type {};

} // namespace math
} // namespace filament

namespace std {

template<> struct is_floating_point<filament::math::half> : public std::true_type {};

// note: this shouldn't be needed (is_floating_point<> is enough) but some version of msvc need it
// This stopped working with MSVC 2019 16.4, so we specialize our own version of is_arithmetic in
// the math::filament namespace (see above).
template<> struct is_arithmetic<filament::math::half> : public std::true_type {};

template<>
class numeric_limits<filament::math::half> {
public:
    typedef filament::math::half type;

    static constexpr const bool is_specialized = true;
    static constexpr const bool is_signed = true;
    static constexpr const bool is_integer = false;
    static constexpr const bool is_exact = false;
    static constexpr const bool has_infinity = true;
    static constexpr const bool has_quiet_NaN = true;
    static constexpr const bool has_signaling_NaN = false;
    static constexpr const float_denorm_style has_denorm = denorm_absent;
    static constexpr const bool has_denorm_loss = true;
    static constexpr const bool is_iec559 = false;
    static constexpr const bool is_bounded = true;
    static constexpr const bool is_modulo = false;
    static constexpr const bool traps = false;
    static constexpr const bool tinyness_before = false;
    static constexpr const float_round_style round_style = round_indeterminate;

    static constexpr const int digits = 11;
    static constexpr const int digits10 = 3;
    static constexpr const int max_digits10 = 5;
    static constexpr const int radix = 2;
    static constexpr const int min_exponent = -13;
    static constexpr const int min_exponent10 = -4;
    static constexpr const int max_exponent = 16;
    static constexpr const int max_exponent10 = 4;

    inline static constexpr type round_error() noexcept { return filament::math::makeHalf(0x3800); }
    inline static constexpr type min() noexcept { return filament::math::makeHalf(0x0400); }
    inline static constexpr type max() noexcept { return filament::math::makeHalf(0x7bff); }
    inline static constexpr type lowest() noexcept { return filament::math::makeHalf(0xfbff); }
    inline static constexpr type epsilon() noexcept { return filament::math::makeHalf(0x1400); }
    inline static constexpr type infinity() noexcept { return filament::math::makeHalf(0x7c00); }
    inline static constexpr type quiet_NaN() noexcept { return filament::math::makeHalf(0x7fff); }
    inline static constexpr type denorm_min() noexcept { return filament::math::makeHalf(0x0001); }
    inline static constexpr type signaling_NaN() noexcept { return filament::math::makeHalf(0x7dff); }
};

} // namespace std

#endif // TNT_MATH_HALF_H
