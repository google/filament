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

#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_enum_class_bitmasks.h>

#include <cmath>
#include <limits>
#include <vector>

#include "dawn/common/Math.h"

namespace wgpu {

enum class TestEnum {
    A = 0x1,
    B = 0x2,
    C = 0x4,
};

template <>
struct IsWGPUBitmask<TestEnum> {
    static constexpr bool enable = true;
};

}  // namespace wgpu

namespace dawn {
namespace {

// Tests for ScanForward
TEST(Math, ScanForward) {
    // Test extrema
    ASSERT_EQ(ScanForward(1), 0u);
    ASSERT_EQ(ScanForward(0x80000000), 31u);

    // Test with more than one bit set.
    ASSERT_EQ(ScanForward(256), 8u);
    ASSERT_EQ(ScanForward(256 + 32), 5u);
    ASSERT_EQ(ScanForward(1024 + 256 + 32), 5u);
}

// Tests for Log2
TEST(Math, Log2) {
    // Test extrema
    ASSERT_EQ(Log2(1u), 0u);
    ASSERT_EQ(Log2(0xFFFFFFFFu), 31u);
    ASSERT_EQ(Log2(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFF)), 63u);

    static_assert(ConstexprLog2(1u) == 0u);
    static_assert(ConstexprLog2(0xFFFFFFFFu) == 31u);
    static_assert(ConstexprLog2(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFF)) == 63u);

    // Test boundary between two logs
    ASSERT_EQ(Log2(0x80000000u), 31u);
    ASSERT_EQ(Log2(0x7FFFFFFFu), 30u);
    ASSERT_EQ(Log2(static_cast<uint64_t>(0x8000000000000000)), 63u);
    ASSERT_EQ(Log2(static_cast<uint64_t>(0x7FFFFFFFFFFFFFFF)), 62u);

    static_assert(ConstexprLog2(0x80000000u) == 31u);
    static_assert(ConstexprLog2(0x7FFFFFFFu) == 30u);
    static_assert(ConstexprLog2(static_cast<uint64_t>(0x8000000000000000)) == 63u);
    static_assert(ConstexprLog2(static_cast<uint64_t>(0x7FFFFFFFFFFFFFFF)) == 62u);

    ASSERT_EQ(Log2(16u), 4u);
    ASSERT_EQ(Log2(15u), 3u);

    static_assert(ConstexprLog2(16u) == 4u);
    static_assert(ConstexprLog2(15u) == 3u);
}

// Tests for Log2Ceil
TEST(Math, Log2Ceil) {
    // Test extrema
    ASSERT_EQ(Log2Ceil(1u), 0u);
    ASSERT_EQ(Log2Ceil(0xFFFFFFFFu), 32u);
    ASSERT_EQ(Log2Ceil(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFF)), 64u);

    static_assert(ConstexprLog2Ceil(1u) == 0u);
    static_assert(ConstexprLog2Ceil(0xFFFFFFFFu) == 32u);
    static_assert(ConstexprLog2Ceil(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFF)) == 64u);

    // Test boundary between two logs
    ASSERT_EQ(Log2Ceil(0x80000001u), 32u);
    ASSERT_EQ(Log2Ceil(0x80000000u), 31u);
    ASSERT_EQ(Log2Ceil(0x7FFFFFFFu), 31u);
    ASSERT_EQ(Log2Ceil(static_cast<uint64_t>(0x8000000000000001)), 64u);
    ASSERT_EQ(Log2Ceil(static_cast<uint64_t>(0x8000000000000000)), 63u);
    ASSERT_EQ(Log2Ceil(static_cast<uint64_t>(0x7FFFFFFFFFFFFFFF)), 63u);

    static_assert(ConstexprLog2Ceil(0x80000001u) == 32u);
    static_assert(ConstexprLog2Ceil(0x80000000u) == 31u);
    static_assert(ConstexprLog2Ceil(0x7FFFFFFFu) == 31u);
    static_assert(ConstexprLog2Ceil(static_cast<uint64_t>(0x8000000000000001)) == 64u);
    static_assert(ConstexprLog2Ceil(static_cast<uint64_t>(0x8000000000000000)) == 63u);
    static_assert(ConstexprLog2Ceil(static_cast<uint64_t>(0x7FFFFFFFFFFFFFFF)) == 63u);

    ASSERT_EQ(Log2Ceil(17u), 5u);
    ASSERT_EQ(Log2Ceil(16u), 4u);
    ASSERT_EQ(Log2Ceil(15u), 4u);

    static_assert(ConstexprLog2Ceil(17u) == 5u);
    static_assert(ConstexprLog2Ceil(16u) == 4u);
    static_assert(ConstexprLog2Ceil(15u) == 4u);
}

// Tests for IsPowerOfTwo
TEST(Math, IsPowerOfTwo) {
    ASSERT_TRUE(IsPowerOfTwo(1));
    ASSERT_TRUE(IsPowerOfTwo(2));
    ASSERT_FALSE(IsPowerOfTwo(3));

    ASSERT_TRUE(IsPowerOfTwo(0x8000000));
    ASSERT_FALSE(IsPowerOfTwo(0x8000400));
}

// Tests for NextPowerOfTwo
TEST(Math, NextPowerOfTwo) {
    // Test extrema
    ASSERT_EQ(NextPowerOfTwo(0), 1ull);
    ASSERT_EQ(NextPowerOfTwo(0x7FFFFFFFFFFFFFFF), 0x8000000000000000);

    // Test boundary between powers-of-two.
    ASSERT_EQ(NextPowerOfTwo(31), 32ull);
    ASSERT_EQ(NextPowerOfTwo(33), 64ull);

    ASSERT_EQ(NextPowerOfTwo(32), 32ull);
}

// Tests for AlignPtr
TEST(Math, AlignPtr) {
    constexpr size_t kTestAlignment = 8;

    char buffer[kTestAlignment * 4];

    for (size_t i = 0; i < 2 * kTestAlignment; ++i) {
        char* unaligned = &buffer[i];
        char* aligned = AlignPtr(unaligned, kTestAlignment);

        ASSERT_GE(aligned - unaligned, 0);
        ASSERT_LT(static_cast<size_t>(aligned - unaligned), kTestAlignment);
        ASSERT_EQ(reinterpret_cast<uintptr_t>(aligned) & (kTestAlignment - 1), 0u);
    }
}

// Tests for Align
TEST(Math, Align) {
    // 0 aligns to 0
    ASSERT_EQ(Align(0u, 4), 0u);
    ASSERT_EQ(Align(0u, 256), 0u);
    ASSERT_EQ(Align(0u, 512), 0u);

    // Multiples align to self
    ASSERT_EQ(Align(8u, 8), 8u);
    ASSERT_EQ(Align(16u, 8), 16u);
    ASSERT_EQ(Align(24u, 8), 24u);
    ASSERT_EQ(Align(256u, 256), 256u);
    ASSERT_EQ(Align(512u, 256), 512u);
    ASSERT_EQ(Align(768u, 256), 768u);

    // Alignment with 1 is self
    for (uint32_t i = 0; i < 128; ++i) {
        ASSERT_EQ(Align(i, 1), i);
    }

    // Everything in the range (align, 2*align] aligns to 2*align
    for (uint32_t i = 1; i <= 64; ++i) {
        ASSERT_EQ(Align(64 + i, 64), 128u);
    }

    // Test extrema
    ASSERT_EQ(Align(static_cast<uint64_t>(0xFFFFFFFF), 4), 0x100000000u);
    ASSERT_EQ(Align(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFF), 1), 0xFFFFFFFFFFFFFFFFull);
}

// Tests for AlignDown
TEST(Math, AlignDown) {
    // 0 aligns to 0
    ASSERT_EQ(AlignDown(0u, 4), 0u);
    ASSERT_EQ(AlignDown(0u, 256), 0u);
    ASSERT_EQ(AlignDown(0u, 512), 0u);

    // Multiples align to self
    ASSERT_EQ(AlignDown(8u, 8), 8u);
    ASSERT_EQ(AlignDown(16u, 8), 16u);
    ASSERT_EQ(AlignDown(24u, 8), 24u);
    ASSERT_EQ(AlignDown(256u, 256), 256u);
    ASSERT_EQ(AlignDown(512u, 256), 512u);
    ASSERT_EQ(AlignDown(768u, 256), 768u);

    // Alignment with 1 is self
    for (uint32_t i = 0; i < 128; ++i) {
        ASSERT_EQ(AlignDown(i, 1), i);
    }

    // Everything in the range (align, 2*align - 1) aligns down to align
    for (uint32_t i = 1; i < 64; ++i) {
        ASSERT_EQ(AlignDown(64 + i, 64), 64u);
    }

    // Test extrema
    ASSERT_EQ(AlignDown(static_cast<uint64_t>(0xFFFFFFFF), 4), 0xFFFFFFFC);
    ASSERT_EQ(AlignDown(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFF), 1), 0xFFFFFFFFFFFFFFFFull);
}

TEST(Math, AlignSizeof) {
    // Basic types should align to self if alignment is a divisor.
    ASSERT_EQ((AlignSizeof<uint8_t, 1>()), 1u);

    ASSERT_EQ((AlignSizeof<uint16_t, 1>()), 2u);
    ASSERT_EQ((AlignSizeof<uint16_t, 2>()), 2u);

    ASSERT_EQ((AlignSizeof<uint32_t, 1>()), 4u);
    ASSERT_EQ((AlignSizeof<uint32_t, 2>()), 4u);
    ASSERT_EQ((AlignSizeof<uint32_t, 4>()), 4u);

    ASSERT_EQ((AlignSizeof<uint64_t, 1>()), 8u);
    ASSERT_EQ((AlignSizeof<uint64_t, 2>()), 8u);
    ASSERT_EQ((AlignSizeof<uint64_t, 4>()), 8u);
    ASSERT_EQ((AlignSizeof<uint64_t, 8>()), 8u);

    // Everything in range (align, 2*align] aligns to 2*align.
    ASSERT_EQ((AlignSizeof<char[5], 4>()), 8u);
    ASSERT_EQ((AlignSizeof<char[6], 4>()), 8u);
    ASSERT_EQ((AlignSizeof<char[7], 4>()), 8u);
    ASSERT_EQ((AlignSizeof<char[8], 4>()), 8u);
}

TEST(Math, AlignSizeofN) {
    // Everything in range (align, 2*align] aligns to 2*align.
    ASSERT_EQ(*(AlignSizeofN<char, 4>(5)), 8u);
    ASSERT_EQ(*(AlignSizeofN<char, 4>(6)), 8u);
    ASSERT_EQ(*(AlignSizeofN<char, 4>(7)), 8u);
    ASSERT_EQ(*(AlignSizeofN<char, 4>(8)), 8u);

    // Extremes should return nullopt.
    ASSERT_EQ((AlignSizeofN<char, 4>(std::numeric_limits<size_t>::max())), std::nullopt);
    ASSERT_EQ((AlignSizeofN<char, 4>(std::numeric_limits<uint64_t>::max())), std::nullopt);
}

// Tests for IsPtrAligned
TEST(Math, IsPtrAligned) {
    constexpr size_t kTestAlignment = 8;

    char buffer[kTestAlignment * 4];

    for (size_t i = 0; i < 2 * kTestAlignment; ++i) {
        char* unaligned = &buffer[i];
        char* aligned = AlignPtr(unaligned, kTestAlignment);

        ASSERT_EQ(IsPtrAligned(unaligned, kTestAlignment), unaligned == aligned);
    }
}

// Tests for IsAligned
TEST(Math, IsAligned) {
    // 0 is aligned
    ASSERT_TRUE(IsAligned(0, 4));
    ASSERT_TRUE(IsAligned(0, 256));
    ASSERT_TRUE(IsAligned(0, 512));

    // Multiples are aligned
    ASSERT_TRUE(IsAligned(8, 8));
    ASSERT_TRUE(IsAligned(16, 8));
    ASSERT_TRUE(IsAligned(24, 8));
    ASSERT_TRUE(IsAligned(256, 256));
    ASSERT_TRUE(IsAligned(512, 256));
    ASSERT_TRUE(IsAligned(768, 256));

    // Alignment with 1 is always aligned
    for (uint32_t i = 0; i < 128; ++i) {
        ASSERT_TRUE(IsAligned(i, 1));
    }

    // Everything in the range (align, 2*align) is not aligned
    for (uint32_t i = 1; i < 64; ++i) {
        ASSERT_FALSE(IsAligned(64 + i, 64));
    }
}

// Tests for float32 to float16 conversion
TEST(Math, Float32ToFloat16) {
    ASSERT_EQ(Float32ToFloat16(0.0f), 0x0000);
    ASSERT_EQ(Float32ToFloat16(-0.0f), 0x8000);

    ASSERT_EQ(Float32ToFloat16(INFINITY), 0x7C00);
    ASSERT_EQ(Float32ToFloat16(-INFINITY), 0xFC00);

    // Check that NaN is converted to a value in one of the float16 NaN ranges
    uint16_t nan16 = Float32ToFloat16(NAN);
    ASSERT_TRUE(nan16 > 0xFC00 || (nan16 < 0x8000 && nan16 > 0x7C00));

    ASSERT_EQ(Float32ToFloat16(1.0f), 0x3C00);
}

// Tests for IsFloat16NaN
TEST(Math, IsFloat16NaN) {
    ASSERT_FALSE(IsFloat16NaN(0u));
    ASSERT_FALSE(IsFloat16NaN(0u));
    ASSERT_FALSE(IsFloat16NaN(Float32ToFloat16(1.0f)));
    ASSERT_FALSE(IsFloat16NaN(Float32ToFloat16(INFINITY)));
    ASSERT_FALSE(IsFloat16NaN(Float32ToFloat16(-INFINITY)));

    ASSERT_TRUE(IsFloat16NaN(Float32ToFloat16(INFINITY) + 1));
    ASSERT_TRUE(IsFloat16NaN(Float32ToFloat16(-INFINITY) + 1));
    ASSERT_TRUE(IsFloat16NaN(0x7FFF));
    ASSERT_TRUE(IsFloat16NaN(0xFFFF));
}

// Tests for FloatToUnorm
TEST(Math, FloatToUnorm) {
    std::vector<float> kTestFloatValues = {0.0f, 0.4f, 0.5f, 1.0f};
    std::vector<unsigned char> kExpectedCharValues = {0, 102, 127, 255};
    std::vector<uint8_t> kExpectedUint8Values = {0, 102, 127, 255};
    std::vector<uint16_t> kExpectedUint16Values = {0, 26214, 32767, 65535};
    for (size_t i = 0; i < kTestFloatValues.size(); i++) {
        ASSERT_EQ(FloatToUnorm<unsigned char>(kTestFloatValues[i]), kExpectedCharValues[i]);
        ASSERT_EQ(FloatToUnorm<uint8_t>(kTestFloatValues[i]), kExpectedUint8Values[i]);
        ASSERT_EQ(FloatToUnorm<uint16_t>(kTestFloatValues[i]), kExpectedUint16Values[i]);
    }
}

// Tests for SRGBToLinear
TEST(Math, SRGBToLinear) {
    ASSERT_EQ(SRGBToLinear(0.0f), 0.0f);
    ASSERT_EQ(SRGBToLinear(1.0f), 1.0f);

    ASSERT_EQ(SRGBToLinear(-1.0f), 0.0f);
    ASSERT_EQ(SRGBToLinear(2.0f), 1.0f);

    ASSERT_FLOAT_EQ(SRGBToLinear(0.5f), 0.21404114f);
}

// Tests for RoundUp
TEST(Math, RoundUp) {
    ASSERT_EQ(RoundUp(2, 2), 2u);
    ASSERT_EQ(RoundUp(2, 4), 4u);
    ASSERT_EQ(RoundUp(6, 2), 6u);
    ASSERT_EQ(RoundUp(8, 4), 8u);
    ASSERT_EQ(RoundUp(12, 6), 12u);

    ASSERT_EQ(RoundUp(3, 3), 3u);
    ASSERT_EQ(RoundUp(3, 5), 5u);
    ASSERT_EQ(RoundUp(5, 3), 6u);
    ASSERT_EQ(RoundUp(9, 5), 10u);

    // Test extrema
    ASSERT_EQ(RoundUp(0x7FFFFFFFFFFFFFFFull, 0x8000000000000000ull), 0x8000000000000000ull);
    ASSERT_EQ(RoundUp(1, 1), 1u);
}

// Tests for IsSubset
TEST(Math, IsSubset) {
    // single value is a subset
    ASSERT_TRUE(IsSubset(0b100, 0b101));
    ASSERT_FALSE(IsSubset(0b010, 0b101));
    ASSERT_TRUE(IsSubset(0b001, 0b101));

    // empty set is a subset
    ASSERT_TRUE(IsSubset(0b000, 0b101));

    // equal-to is a subset
    ASSERT_TRUE(IsSubset(0b101, 0b101));

    // superset is not a subset
    ASSERT_FALSE(IsSubset(0b111, 0b101));

    // only empty is a subset of empty
    ASSERT_FALSE(IsSubset(0b100, 0b000));
    ASSERT_FALSE(IsSubset(0b010, 0b000));
    ASSERT_FALSE(IsSubset(0b001, 0b000));
    ASSERT_TRUE(IsSubset(0b000, 0b000));

    // Test with enums
    ASSERT_TRUE(IsSubset(wgpu::TestEnum::A, wgpu::TestEnum::A));
    ASSERT_TRUE(IsSubset(wgpu::TestEnum::A, wgpu::TestEnum::A | wgpu::TestEnum::B));
    ASSERT_FALSE(IsSubset(wgpu::TestEnum::C, wgpu::TestEnum::A | wgpu::TestEnum::B));
    ASSERT_FALSE(IsSubset(wgpu::TestEnum::A | wgpu::TestEnum::C, wgpu::TestEnum::A));
}

}  // anonymous namespace
}  // namespace dawn
