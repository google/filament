// Copyright 2020 The Dawn & Tint Authors
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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/439062058): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#include "src/tint/utils/strconv/float_to_string.h"

#include <cmath>
#include <cstring>
#include <iomanip>

#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::strconv {

namespace {

template <typename T>
struct Traits;

template <>
struct Traits<float> {
    using uint_t = uint32_t;
    static constexpr int kExponentBias = 127;
    static constexpr uint_t kExponentMask = 0x7f800000;
    static constexpr uint_t kMantissaMask = 0x007fffff;
    static constexpr uint_t kSignMask = 0x80000000;
    static constexpr int kMantissaBits = 23;
};

template <>
struct Traits<double> {
    using uint_t = uint64_t;
    static constexpr int kExponentBias = 1023;
    static constexpr uint_t kExponentMask = 0x7ff0000000000000;
    static constexpr uint_t kMantissaMask = 0x000fffffffffffff;
    static constexpr uint_t kSignMask = 0x8000000000000000;
    static constexpr int kMantissaBits = 52;
};

template <typename F>
std::string ToString(F f) {
    StringStream s;
    s << f;
    return s.str();
}

template <typename F>
std::string ToBitPreservingString(F f) {
    using T = Traits<F>;
    using uint_t = typename T::uint_t;

    // For the NaN case, avoid handling the number as a floating point value.
    // Some machines will modify the top bit in the mantissa of a NaN.

    std::stringstream ss;

    typename T::uint_t float_bits = 0u;
    static_assert(sizeof(float_bits) == sizeof(f));
    std::memcpy(&float_bits, &f, sizeof(float_bits));

    // Handle the sign.
    if (float_bits & T::kSignMask) {
        // If `f` is -0.0 print -0.0.
        ss << '-';
        // Strip sign bit.
        float_bits = float_bits & (~T::kSignMask);
    }

    switch (std::fpclassify(f)) {
        case FP_ZERO:
        case FP_NORMAL:
            std::memcpy(&f, &float_bits, sizeof(float_bits));
            ss << ToString(f);
            break;

        default: {
            // Infinity, NaN, and Subnormal
            // TODO(dneto): It's unclear how Infinity and NaN should be handled.
            // See https://github.com/gpuweb/gpuweb/issues/1769

            // std::hexfloat prints 'nan' and 'inf' instead of an explicit representation like we
            // want. Split it out manually.
            int mantissa_nibbles = (T::kMantissaBits + 3) / 4;

            const int biased_exponent =
                static_cast<int>((float_bits & T::kExponentMask) >> T::kMantissaBits);
            int exponent = biased_exponent - T::kExponentBias;
            uint_t mantissa = float_bits & T::kMantissaMask;

            ss << "0x";

            if (exponent == T::kExponentBias + 1) {
                if (mantissa == 0) {
                    //  Infinity case.
                    ss << "1p+" << exponent;
                } else {
                    // NaN case.
                    // Emit the mantissa bits as if they are left-justified after the binary point.
                    // This is what SPIRV-Tools hex float emitter does, and it's a justifiable
                    // choice independent of the bit width of the mantissa.
                    mantissa <<= (4 - (T::kMantissaBits % 4));
                    // Remove trailing zeroes, for tidiness.
                    while (0 == (0xf & mantissa)) {
                        mantissa >>= 4;
                        mantissa_nibbles--;
                    }
                    ss << "1." << std::hex << std::setfill('0') << std::setw(mantissa_nibbles)
                       << mantissa << "p+" << std::dec << exponent;
                }
            } else {
                // Subnormal, and not zero.
                TINT_ASSERT(mantissa != 0);
                const auto kTopBit = static_cast<uint_t>(1u) << T::kMantissaBits;

                // Shift left until we get 1.x
                while (0 == (kTopBit & mantissa)) {
                    mantissa <<= 1;
                    exponent--;
                }
                // Emit the leading 1, and remove it from the mantissa.
                ss << "1";
                mantissa = mantissa ^ kTopBit;
                exponent++;

                // Left-justify mantissa to whole nibble.
                mantissa <<= (4 - (T::kMantissaBits % 4));

                // Emit the fractional part.
                if (mantissa) {
                    // Remove trailing zeroes, for tidiness
                    while (0 == (0xf & mantissa)) {
                        mantissa >>= 4;
                        mantissa_nibbles--;
                    }
                    ss << "." << std::hex << std::setfill('0') << std::setw(mantissa_nibbles)
                       << mantissa;
                }
                // Emit the exponent
                ss << "p" << std::showpos << std::dec << exponent;
            }
        }
    }
    return ss.str();
}

}  // namespace

std::string FloatToString(float f) {
    return ToString(f);
}

std::string FloatToBitPreservingString(float f) {
    return ToBitPreservingString(f);
}

std::string DoubleToString(double f) {
    return ToString(f);
}

std::string DoubleToBitPreservingString(double f) {
    return ToBitPreservingString(f);
}

}  // namespace tint::strconv
