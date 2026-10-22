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

#include "src/tint/lang/core/number.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/memory/bitcast.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::core {
namespace {

constexpr uint16_t kF16Nan = 0x7e00u;
constexpr uint16_t kF16PosInf = 0x7c00u;
constexpr uint16_t kF16NegInf = 0xfc00u;

constexpr uint16_t kF16SignMask = 0x8000u;
constexpr uint16_t kF16ExponentMask = 0x7c00u;
constexpr uint16_t kF16MantissaMask = 0x03ffu;

constexpr uint32_t kF16MantissaBits = 10;
constexpr uint32_t kF16ExponentBias = 15;

constexpr uint32_t kF32SignMask = 0x80000000u;
constexpr uint32_t kF32ExponentMask = 0x7f800000u;
constexpr uint32_t kF32MantissaMask = 0x007fffffu;

constexpr uint32_t kF32MantissaBits = 23;
constexpr uint32_t kF32ExponentBias = 127;

constexpr uint32_t kMaxF32BiasedExpForF16NormalNumber = 142;
constexpr uint32_t kMinF32BiasedExpForF16NormalNumber = 113;
constexpr uint32_t kMaxF32BiasedExpForF16SubnormalNumber = 112;
constexpr uint32_t kMinF32BiasedExpForF16SubnormalNumber = 103;

}  // namespace

f16::type f16::Quantize(f16::type value) {
    if (value > kHighestValue) {
        return std::numeric_limits<f16::type>::infinity();
    }
    if (value < kLowestValue) {
        return -std::numeric_limits<f16::type>::infinity();
    }

    // Below value must be within the finite range of a f16.
    // Assert we use binary32 (i.e. float) as underlying type, which has 4 bytes.
    static_assert(std::is_same<f16::type, float>());

    uint32_t u32 = tint::Bitcast<uint32_t>(value);
    if ((u32 & ~kF32SignMask) == 0) {
        return value;  // +/- zero
    }
    if ((u32 & kF32ExponentMask) == kF32ExponentMask) {  // exponent all 1's
        return value;                                    // inf or nan
    }

    // We are now going to quantize a f32 number into subnormal f16 and store the result value back
    // into a f32 variable. Notice that all subnormal f16 values are just normal f32 values. Below
    // will show that we can do this quantization by just masking out 13 or more lowest mantissa
    // bits of the original f32 number.
    //
    // Note:
    // * f32 has 1 sign bit, 8 exponent bits for biased exponent (i.e. unbiased exponent + 127), and
    //   23 mantissa bits. Binary form: s_eeeeeeee_mmmmmmmmmmmmmmmmmmmmmmm
    //
    // * f16 has 1 sign bit, 5 exponent bits for biased exponent (i.e. unbiased exponent + 15), and
    //   10 mantissa bits. Binary form: s_eeeee_mmmmmmmmmm
    //
    // The largest finite f16 number has a biased exponent of 11110 in binary, or 30 decimal, and so
    // an unbiased exponent of 30 - 15 = 15.
    //
    // The smallest finite f16 number has a biased exponent of 00001 in binary, or 1 decimal, and so
    // a unbiased exponent of 1 - 15 = -14.
    //
    // We may follow the argument below:
    // 1. All normal or subnormal f16 values, range from 0x1.p-24 to 0x1.ffcp15, are exactly
    // representable by a normal f32 number.
    //   1.1. We can denote the set of all f32 number that are exact representation of finite f16
    //   values by `R`.
    //   1.2. We can do the quantization by mapping a normal f32 value v (in the f16 finite range)
    //   to a certain f32 number v' in the set R, which is the largest (by the meaning of absolute
    //   value) one among all values in R that are no larger than v.
    //
    // 2. We can decide whether a given normal f32 number v is in the set R, by looking at its
    // mantissa bits and biased exponent `e`. Recall that biased exponent e is unbiased exponent +
    // 127, and in the range of 1 to 254 for normal f32 number.
    //   2.1. If e >= 143, i.e. abs(v) >= 2^16 > f16::kHighestValue = 0x1.ffcp15, v is larger than
    //   any finite f16 value and can not be in set R. 2.2. If 142 >= e >= 113, or
    //   f16::kHighestValue >= abs(v) >= f16::kSmallestValue = 2^-14, v falls in the range of normal
    //   f16 values. In this case, v is in the set R iff the lowest 13 mantissa bits are all 0. (See
    //   below for proof)
    //     2.2.1. If we let v' be v with lowest 13 mantissa bits masked to 0, v' will be in set R
    //     and the largest one in set R that no larger than v. Such v' is the quantized value of v.
    //   2.3. If 112 >= e >= 103, i.e. 2^-14 > abs(v) >= f16::kSmallestSubnormalValue = 2^-24, v
    //   falls in the range of subnormal f16 values. In this case, v is in the set R iff the lowest
    //   126-e mantissa bits are all 0. Notice that 126-e is in range 14 to 23, inclusive. (See
    //   below for proof)
    //     2.3.1. If we let v' be v with lowest 126-e mantissa bits masked to 0, v' will be in set R
    //     and the largest on in set R that no larger than v. Such v' is the quantized value of v.
    //   2.4. If 2^-24 > abs(v) > 0, i.e. 103 > e, v is smaller than any finite f16 value and not
    //   equal to 0.0, thus can not be in set R.
    //   2.5. If abs(v) = 0, v is in set R and is just +-0.0.
    //
    // Proof for 2.2
    // -------------
    // Any normal f16 number, in binary form, s_eeeee_mmmmmmmmmm, has value
    //
    //      (s == 0 ? 1 : -1) * (1 + uint(mmmmm_mmmmm) * (2^-10)) * 2^(uint(eeeee) - 15)
    //
    // in which unit(bbbbb) means interprete binary pattern "bbbbb" as unsigned binary number,
    // and we have 1 <= uint(eeeee) <= 30.
    //
    // This value is equal to a normal f32 number with binary
    //      s_EEEEEEEE_mmmmmmmmmm0000000000000
    //
    // where uint(EEEEEEEE) = uint(eeeee) + 112, so that unbiased exponent is kept unchanged
    //
    //      uint(EEEEEEEE) - 127 = uint(eeeee) - 15
    //
    // and its value is
    //         (s == 0 ? 1 : -1) *
    //            (1 + uint(mmmmm_mmmmm_00000_00000_000) * (2^-23)) * 2^(uint(EEEEEEEE) - 127)
    //      == (s == 0 ? 1 : -1) *
    //            (1 + uint(mmmmm_mmmmm)                 * (2^-10)) * 2^(uint(eeeee) - 15)
    //
    // Notice that uint(EEEEEEEE) is in range [113, 142], showing that it is a normal f32 number.
    // So we proved that any normal f16 number can be exactly representd by a normal f32 number
    // with biased exponent in range [113, 142] and the lowest 13 mantissa bits 0.
    //
    // On the other hand, since mantissa bits mmmmmmmmmm are arbitrary, the value of any f32
    // that has a biased exponent in range [113, 142] and lowest 13 mantissa bits zero is equal
    // to a normal f16 value. Hence we prove 2.2.
    //
    // Proof for 2.3
    // -------------
    // Any subnormal f16 number has a binary form of s_00000_mmmmmmmmmm, and its value is
    //
    //    (s == 0 ? 1 : -1) * uint(mmmmmmmmmm) * (2^-10) * (2^-14)
    // == (s == 0 ? 1 : -1) * uint(mmmmmmmmmm) * (2^-24).
    //
    // We discuss the bit pattern of mantissa bits mmmmmmmmmm.
    //   Case 1: mantissa bits have no leading zero bit, s_00000_1mmmmmmmmm
    //      In this case the value is
    //              (s == 0 ? 1 : -1) *      uint(1mmmm_mmmmm)                 * (2^-10)  * (2^-14)
    //          ==  (s == 0 ? 1 : -1) * (    uint(1_mmmmm_mmmm)                * (2^-9))  * (2^-15)
    //          ==  (s == 0 ? 1 : -1) * (1 + uint(mmmmm_mmmm)                  * (2^-9))  * (2^-15)
    //          ==  (s == 0 ? 1 : -1) * (1 + uint(mmmmm_mmmm0_00000_00000_000) * (2^-23)) * (2^-15)
    //
    //      which is equal to the value of the normal f32 number
    //
    //          s_EEEEEEEE_mmmmm_mmmm0_00000_00000_000
    //
    //      where uint(EEEEEEEE) == -15 + 127 = 112. Hence we proved that any subnormal f16 number
    //      with no leading zero mantissa bit can be exactly represented by a f32 number with
    //      biased exponent 112 and the lowest 14 mantissa bits zero, and the value of any f32
    //      number with biased exponent 112 and the lowest 14 mantissa bits zero is equal to a
    //      subnormal f16 number with no leading zero mantissa bit.
    //
    //   Case 2: mantissa bits has 1 leading zero bit, s_00000_01mmmmmmmm
    //      In this case the value is
    //              (s == 0 ? 1 : -1) *      uint(01mmm_mmmmm)                 * (2^-10)  * (2^-14)
    //          ==  (s == 0 ? 1 : -1) * (    uint(01_mmmmm_mmm)                * (2^-8))  * (2^-16)
    //          ==  (s == 0 ? 1 : -1) * (1 + uint(mmmmm_mmm)                   * (2^-8))  * (2^-16)
    //          ==  (s == 0 ? 1 : -1) * (1 + uint(mmmmm_mmm00_00000_00000_000) * (2^-23)) * (2^-16)
    //
    //      which is equal to the value of normal f32 number
    //
    //          s_EEEEEEEE_mmmmm_mmm00_00000_00000_000
    //
    //      where uint(EEEEEEEE) = -16 + 127 = 111. Hence we proved that any subnormal f16 number
    //      with 1 leading zero mantissa bit can be exactly represented by a f32 number with
    //      biased exponent 111 and the lowest 15 mantissa bits zero, and the value of any f32
    //      number with biased exponent 111 and the lowest 15 mantissa bits zero is equal to a
    //      subnormal f16 number with 1 leading zero mantissa bit.
    //
    //   Case 3 to case 8: ......
    //
    //   Case 9: mantissa bits has 8 leading zero bits, s_00000_000000001m
    //      In this case the value is
    //              (s == 0 ? 1 : -1) *      uint(00000_0001m)                 * (2^-10)  * (2^-14)
    //          ==  (s == 0 ? 1 : -1) * (    uint(000000001_m)                 * (2^-1))  * (2^-23)
    //          ==  (s == 0 ? 1 : -1) * (1 + uint(m)                           * (2^-1))  * (2^-23)
    //          ==  (s == 0 ? 1 : -1) * (1 + uint(m0000_00000_00000_00000_000) * (2^-23)) * (2^-23)
    //
    //      which is equal to the value of normal f32 number
    //
    //          s_EEEEEEEE_m0000_00000_00000_00000_000
    //
    //      where uint(EEEEEEEE) = -23 + 127 = 104. Hence we proved that any subnormal f16 number
    //      with 8 leading zero mantissa bit can be exactly represented by a f32 number with
    //      biased exponent 104 and the lowest 22 mantissa bits zero, and the value of any f32
    //      number with biased exponent 104 and the lowest 22 mantissa bits zero are equal to a
    //      subnormal f16 number with 8 leading zero mantissa bit.
    //
    //   Case 10: mantissa bits has 9 leading zero bits, s_00000_0000000001
    //      In this case the value is just +-2^-24 == +-0x1.0p-24,
    //      the f32 number has biased exponent 103 and all 23 mantissa bits zero.
    //
    //   Case 11: mantissa bits has 10 leading zero bits, s_00000_0000000000, just 0.0
    //
    // Concluding all these case, we proved that any subnormal f16 number with N leading zero
    // mantissa bit can be exactly represented by a f32 number with biased exponent 112 - N and the
    // lowest 14 + N mantissa bits zero, and the value of any f32 number with biased exponent
    // 112 - N (= e) and the lowest 14 + N (= 126 - e) mantissa bits zero are equal to a subnormal
    // f16 number with N leading zero mantissa bits. N is in range [0, 9], so the f32 number's
    // biased exponent e is in range [103, 112], or unbiased exponent in [-24, -15].

    float abs_value = std::fabs(value);
    if (abs_value >= kSmallestValue) {
        // Value falls in the normal f16 range, quantize it to a normal f16 value by masking out
        // lowest 13 mantissa bits.
        u32 = u32 & ~((1u << (kF32MantissaBits - kF16MantissaBits)) - 1);
    } else if (abs_value >= kSmallestSubnormalValue) {
        // Value should be quantized to a subnormal f16 value.

        // Get the biased exponent `e` of f32 value, e.g. value 127 representing exponent 2^0.
        uint32_t biased_exponent_original = (u32 & kF32ExponentMask) >> kF32MantissaBits;
        // Since we ensure that kSmallestValue = 0x1f-14 > abs(value) >= kSmallestSubnormalValue =
        // 0x1f-24, value will have a unbiased exponent in range -24 to -15 (inclusive), and the
        // corresponding biased exponent in f32 is in range 103 to 112 (inclusive).
        TINT_ASSERT((kMinF32BiasedExpForF16SubnormalNumber <= biased_exponent_original) &&
                    (biased_exponent_original <= kMaxF32BiasedExpForF16SubnormalNumber));

        // As we have proved, masking out the lowest 126-e mantissa bits of input value will result
        // in a valid subnormal f16 value, which is exactly the required quantization result.
        uint32_t discard_bits = 126 - biased_exponent_original;  // In range 14 to 23 (inclusive)
        TINT_ASSERT((14 <= discard_bits) && (discard_bits <= kF32MantissaBits));
        uint32_t discard_mask = (1u << discard_bits) - 1;
        u32 = u32 & ~discard_mask;
    } else {
        // value is too small that it can't even be represented as subnormal f16 number. Quantize
        // to zero.
        return value > 0 ? 0.0 : -0.0;
    }

    return tint::Bitcast<f16::type>(u32);
}

uint16_t f16::BitsRepresentation() const {
    // Assert we use binary32 (i.e. float) as underlying type, which has 4 bytes.
    static_assert(std::is_same<f16::type, float>());

    // The stored value in f16 object must be already quantized, so it should be either NaN, +/-
    // Inf, or exactly representable by normal or subnormal f16.

    if (std::isnan(value)) {
        return kF16Nan;
    }

    if (std::isinf(value)) {
        return value > 0 ? kF16PosInf : kF16NegInf;
    }

    // Now quantized_value must be a finite f16 exactly-representable value.
    // The following table shows exponent cases for all finite f16 exactly-representable value.
    // ---------------------------------------------------------------------------
    // |  Value category  |  Unbiased exp  |  F16 biased exp  |  F32 biased exp  |
    // |------------------|----------------|------------------|------------------|
    // |     +/- zero     |        \       |         0        |         0        |
    // |  Subnormal f16   |   [-24, -15]   |         0        |    [103, 112]    |
    // |    Normal f16    |   [-14,  15]   |      [1, 30]     |    [113, 142]    |
    // ---------------------------------------------------------------------------

    uint32_t f32_bit_pattern = tint::Bitcast<uint32_t>(value);
    uint32_t f32_biased_exponent = (f32_bit_pattern & kF32ExponentMask) >> kF32MantissaBits;
    uint32_t f32_mantissa = f32_bit_pattern & kF32MantissaMask;

    uint16_t f16_sign_part = static_cast<uint16_t>((f32_bit_pattern & kF32SignMask) >> 16);
    TINT_ASSERT((f16_sign_part & ~kF16SignMask) == 0);

    if ((f32_bit_pattern & ~kF32SignMask) == 0) {
        // +/- zero
        return f16_sign_part;
    }

    if ((kMinF32BiasedExpForF16NormalNumber <= f32_biased_exponent) &&
        (f32_biased_exponent <= kMaxF32BiasedExpForF16NormalNumber)) {
        // Normal f16
        uint32_t f16_biased_exponent = f32_biased_exponent - kF32ExponentBias + kF16ExponentBias;
        uint16_t f16_exp_part = static_cast<uint16_t>(f16_biased_exponent << kF16MantissaBits);
        uint16_t f16_mantissa_part =
            static_cast<uint16_t>(f32_mantissa >> (kF32MantissaBits - kF16MantissaBits));

        TINT_ASSERT((f16_exp_part & ~kF16ExponentMask) == 0);
        TINT_ASSERT((f16_mantissa_part & ~kF16MantissaMask) == 0);

        return f16_sign_part | f16_exp_part | f16_mantissa_part;
    }

    if ((kMinF32BiasedExpForF16SubnormalNumber <= f32_biased_exponent) &&
        (f32_biased_exponent <= kMaxF32BiasedExpForF16SubnormalNumber)) {
        // Subnormal f16
        // The resulting exp bits are always 0, and the mantissa bits should be handled specially.
        uint16_t f16_exp_part = 0;
        // The resulting subnormal f16 will have only 1 valid mantissa bit if the unbiased exponent
        // of value is of the minimum, i.e. -24; and have all 10 mantissa bits valid if the unbiased
        // exponent of value is of the maximum, i.e. -15.
        uint32_t f16_valid_mantissa_bits =
            f32_biased_exponent - kMinF32BiasedExpForF16SubnormalNumber + 1;
        // The resulting f16 mantissa part comes from right-shifting the f32 mantissa bits with
        // leading 1 added.
        uint16_t f16_mantissa_part =
            static_cast<uint16_t>((f32_mantissa | (kF32MantissaMask + 1)) >>
                                  (kF32MantissaBits + 1 - f16_valid_mantissa_bits));

        TINT_ASSERT((1 <= f16_valid_mantissa_bits) &&
                    (f16_valid_mantissa_bits <= kF16MantissaBits));
        TINT_ASSERT((f16_mantissa_part & ~((1u << f16_valid_mantissa_bits) - 1)) == 0);
        TINT_ASSERT((f16_mantissa_part != 0));

        return f16_sign_part | f16_exp_part | f16_mantissa_part;
    }

    // Neither zero, subnormal f16 or normal f16, shall never hit.
    TINT_UNREACHABLE();
}

// static
core::Number<core::detail::NumberKindF16> f16::FromBits(uint16_t bits) {
    // Assert we use binary32 (i.e. float) as underlying type, which has 4 bytes.
    static_assert(std::is_same<f16::type, float>());

    if (bits == kF16PosInf) {
        return f16(std::numeric_limits<f16::type>::infinity());
    }
    if (bits == kF16NegInf) {
        return f16(-std::numeric_limits<f16::type>::infinity());
    }

    auto f16_sign_bit = uint32_t(bits & kF16SignMask);
    // If none of the other bits are set we have a 0. If only the sign bit is set we have a -0.
    if ((bits & ~kF16SignMask) == 0) {
        return f16(f16_sign_bit > 0 ? -0.f : 0.f);
    }

    auto f16_mantissa = uint32_t(bits & kF16MantissaMask);
    auto f16_biased_exponent = uint32_t(bits & kF16ExponentMask);

    // F16 NaN has all expoennt bits set and at least one mantissa bit set
    if (((f16_biased_exponent & kF16ExponentMask) == kF16ExponentMask) && f16_mantissa != 0) {
        return f16(std::numeric_limits<f16::type>::quiet_NaN());
    }

    // Shift the exponent over to be a regular number.
    f16_biased_exponent >>= kF16MantissaBits;

    // Add the F32 bias and remove the F16 bias.
    uint32_t f32_biased_exponent = f16_biased_exponent + kF32ExponentBias - kF16ExponentBias;

    if (f16_biased_exponent == 0) {
        // Subnormal number
        //
        // All subnormal F16 values can be represented as normal F32 values. Shift the mantissa and
        // set the exponent as if this was a normal f16 value.

        // While the first F16 exponent bit is not set
        constexpr uint32_t kF16FirstExponentBit = 0x0400;
        while ((f16_mantissa & kF16FirstExponentBit) == 0) {
            // Shift the mantissa to the left
            f16_mantissa <<= 1;
            // Decrease the biased exponent to counter the shift
            f32_biased_exponent -= 1;
        }

        // Remove the first exponent bit from the mantissa value
        f16_mantissa &= ~kF16FirstExponentBit;
        // Increase the exponent to deal with the masked off value.
        f32_biased_exponent += 1;
    }

    // The mantissa bits are shifted over the difference in mantissa size to be in the F32 location.
    uint32_t f32_mantissa = f16_mantissa << (kF32MantissaBits - kF16MantissaBits);

    // Shift the exponent to the F32 exponent position before the mantissa.
    f32_biased_exponent <<= kF32MantissaBits;

    // Shift the sign bit over to the f32 sign bit position
    uint32_t f32_sign_bit = f16_sign_bit << 16;

    // Combine values together into the F32 value as a uint32_t.
    uint32_t val = f32_sign_bit | f32_biased_exponent | f32_mantissa;

    // Bitcast to a F32 and then store into the F16 Number
    return f16(tint::Bitcast<f16::type>(val));
}

}  // namespace tint::core
