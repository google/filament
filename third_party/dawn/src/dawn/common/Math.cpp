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

#include "dawn/common/Math.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "dawn/common/Assert.h"
#include "dawn/common/Platform.h"

#if DAWN_COMPILER_IS(MSVC)
#include <intrin.h>
#endif

namespace dawn {

uint32_t ScanForward(uint32_t bits) {
    DAWN_ASSERT(bits != 0);
#if DAWN_COMPILER_IS(MSVC)
    // NOLINTNEXTLINE(runtime/int)
    unsigned long firstBitIndex = 0ul;
    unsigned char ret = _BitScanForward(&firstBitIndex, bits);
    DAWN_ASSERT(ret != 0);
    return firstBitIndex;
#else
    return static_cast<uint32_t>(__builtin_ctz(bits));
#endif
}

uint32_t Log2(uint32_t value) {
    DAWN_ASSERT(value != 0);
#if DAWN_COMPILER_IS(MSVC)
    // NOLINTNEXTLINE(runtime/int)
    unsigned long firstBitIndex = 0ul;
    unsigned char ret = _BitScanReverse(&firstBitIndex, value);
    DAWN_ASSERT(ret != 0);
    return firstBitIndex;
#else
    return 31 - static_cast<uint32_t>(__builtin_clz(value));
#endif
}

uint32_t Log2(uint64_t value) {
    DAWN_ASSERT(value != 0);
#if DAWN_COMPILER_IS(MSVC)
#if DAWN_PLATFORM_IS(64_BIT)
    // NOLINTNEXTLINE(runtime/int)
    unsigned long firstBitIndex = 0ul;
    unsigned char ret = _BitScanReverse64(&firstBitIndex, value);
    DAWN_ASSERT(ret != 0);
    return firstBitIndex;
#else   // DAWN_PLATFORM_IS(64_BIT)
    // NOLINTNEXTLINE(runtime/int)
    unsigned long firstBitIndex = 0ul;
    if (_BitScanReverse(&firstBitIndex, value >> 32)) {
        return firstBitIndex + 32;
    }
    unsigned char ret = _BitScanReverse(&firstBitIndex, value & 0xFFFFFFFF);
    DAWN_ASSERT(ret != 0);
    return firstBitIndex;
#endif  // DAWN_PLATFORM_IS(64_BIT)
#else   // DAWN_COMPILER_IS(MSVC)
    return 63 - static_cast<uint32_t>(__builtin_clzll(value));
#endif  // DAWN_COMPILER_IS(MSVC)
}

uint64_t NextPowerOfTwo(uint64_t n) {
    if (n <= 1) {
        return 1;
    }

    return 1ull << (Log2(n - 1) + 1);
}

bool IsPowerOfTwo(uint64_t n) {
    DAWN_ASSERT(n != 0);
    return (n & (n - 1)) == 0;
}

bool IsPtrAligned(const void* ptr, size_t alignment) {
    DAWN_ASSERT(IsPowerOfTwo(alignment));
    DAWN_ASSERT(alignment != 0);
    return (reinterpret_cast<size_t>(ptr) & (alignment - 1)) == 0;
}

bool IsAligned(uint32_t value, size_t alignment) {
    DAWN_ASSERT(alignment <= UINT32_MAX);
    DAWN_ASSERT(IsPowerOfTwo(alignment));
    DAWN_ASSERT(alignment != 0);
    uint32_t alignment32 = static_cast<uint32_t>(alignment);
    return (value & (alignment32 - 1)) == 0;
}

uint16_t Float32ToFloat16(float fp32) {
    uint32_t fp32i = BitCast<uint32_t>(fp32);
    uint32_t sign16 = (fp32i & 0x80000000) >> 16;
    uint32_t mantissaAndExponent = fp32i & 0x7FFFFFFF;

    if (mantissaAndExponent > 0x7F800000) {  // NaN
        return 0x7FFF;
    } else if (mantissaAndExponent > 0x47FFEFFF) {  // Infinity
        return static_cast<uint16_t>(sign16 | 0x7C00);
    } else if (mantissaAndExponent < 0x38800000) {  // Denormal
        uint32_t mantissa = (mantissaAndExponent & 0x007FFFFF) | 0x00800000;
        int32_t exponent = 113 - (mantissaAndExponent >> 23);

        if (exponent < 24) {
            mantissaAndExponent = mantissa >> exponent;
        } else {
            mantissaAndExponent = 0;
        }

        return static_cast<uint16_t>(
            sign16 | (mantissaAndExponent + 0x00000FFF + ((mantissaAndExponent >> 13) & 1)) >> 13);
    } else {
        return static_cast<uint16_t>(sign16 | (mantissaAndExponent + 0xC8000000 + 0x00000FFF +
                                               ((mantissaAndExponent >> 13) & 1)) >>
                                                  13);
    }
}

float Float16ToFloat32(uint16_t fp16) {
    uint32_t tmp = (fp16 & 0x7fff) << 13 | (fp16 & 0x8000) << 16;
    float tmp2 = *reinterpret_cast<float*>(&tmp);
    return pow(2, 127 - 15) * tmp2;
}

bool IsFloat16NaN(uint16_t fp16) {
    return (fp16 & 0x7FFF) > 0x7C00;
}

// Based on the Khronos Data Format Specification 1.2 Section 13.3 sRGB transfer functions
float SRGBToLinear(float srgb) {
    // sRGB is always used in unsigned normalized formats so clamp to [0.0, 1.0]
    if (srgb <= 0.0f) {
        return 0.0f;
    } else if (srgb > 1.0f) {
        return 1.0f;
    }

    if (srgb < 0.04045f) {
        return srgb / 12.92f;
    } else {
        return std::pow((srgb + 0.055f) / 1.055f, 2.4f);
    }
}

uint64_t RoundUp(uint64_t n, uint64_t m) {
    DAWN_ASSERT(m > 0);
    DAWN_ASSERT(n > 0);
    DAWN_ASSERT(m <= std::numeric_limits<uint64_t>::max() - n);
    return ((n + m - 1) / m) * m;
}

}  // namespace dawn
